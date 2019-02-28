#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_print.h"
#include "HI_VarWidthReduce.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>

using namespace llvm;

bool HI_VarWidthReduce::runOnFunction(Function &F) // The runOnModule declaration will overide the virtual one in ModulePass, which will be executed for each Module.
{
    const DataLayout &DL = F.getParent()->getDataLayout();
    SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    // LazyValueInfo* LazyI = &getAnalysis<LazyValueInfoWrapperPass>().getLVI();
    
    if (Function_id.find(&F)==Function_id.end())  // traverse functions and assign function ID
    {
        Function_id[&F] = ++Function_Counter;
    }
    bool changed = 0;

    // Analysis
    for (BasicBlock &B : F) 
    {
        for (Instruction &I: B) 
        {
            if (I.getType()->isIntegerTy())
            {
                Instruction_id[&I] = ++Instruction_Counter;
                KnownBits tmp_KB = computeKnownBits(&I,DL); 
                const SCEV *tmp_S = SE->getSCEV(&I);
                ConstantRange tmp_CR1 = SE->getSignedRange(tmp_S);
                ConstantRange tmp_CR2 = HI_getSignedRangeRef(tmp_S);
                *VarWidthChangeLog << I << "---- Ori-CR: "<<tmp_CR1 << "(bw=" << I.getType()->getIntegerBitWidth() <<") ---- HI-CR:"<<tmp_CR2 << "(bw=" ;
                if (I.mayReadFromMemory())
                {
                    Instruction_BitNeeded[&I] = I.getType()->getIntegerBitWidth();
                    *VarWidthChangeLog << "        ----  this could be a load inst.\n";
                }
                else
                {
                    Instruction_BitNeeded[&I] = bitNeededFor(tmp_CR2) ;
                }                
                 *VarWidthChangeLog << Instruction_BitNeeded[&I] <<")\n";
                *VarWidthChangeLog << "\n\n\n";
            }
        }    
        *VarWidthChangeLog << "\n";
    }


    // Forward Process
   // return false;
    for (BasicBlock &B : F) 
    {
        bool take_action = 1;
        while(take_action)
        {
            take_action = 0;
            for (Instruction &I: B) 
            {
                if (Instruction_id.find(&I) != Instruction_id.end())
                {
                    Instruction_id.erase(&I);
                    *VarWidthChangeLog <<"\n\n\n find target instrction " <<*I.getType() <<":" << I ;
                    VarWidthChangeLog->flush(); 


                    if (PtrToIntInst *PTI = dyn_cast<PtrToIntInst>(&I))
                        continue;
                    if (IntToPtrInst *ITP = dyn_cast<IntToPtrInst>(&I))
                        continue;

                    
                    if (I.getType()->isIntegerTy())
                    {
                        changed = 1;
                        *VarWidthChangeLog <<"\n" <<*I.getType() <<":" << I ;
                        *VarWidthChangeLog << "------- under processing (targetBW="<<Instruction_BitNeeded[&I]<<", curBW="<< (cast<IntegerType>(I.getType()))->getBitWidth()<<") ";   
                        VarWidthChangeLog->flush(); 
                        const SCEV *tmp_S = SE->getSCEV(&I);
                        ConstantRange tmp_CR = HI_getSignedRangeRef(tmp_S);
                        
                        if (ICmpInst* ICMP_I = dyn_cast<ICmpInst>(&I))
                        {
                            if (cast<IntegerType>(ICMP_I->getOperand(0)->getType())->getIntegerBitWidth() == cast<IntegerType>(ICMP_I->getOperand(1)->getType())->getIntegerBitWidth())
                            {
                                *VarWidthChangeLog << "\n                         -------> Inst: " << I << "  ---needs no update req="<< Instruction_BitNeeded[&I] << " user width=" <<(cast<IntegerType>(I.getType()))->getBitWidth() << " \n";  
                                continue;
                            }
                        }
                        else
                        {
                            if (Instruction_BitNeeded[&I] == (cast<IntegerType>(I.getType()))->getBitWidth())
                            {
                                *VarWidthChangeLog << "\n                         -------> Inst: " << I << "  ---needs no update req="<< Instruction_BitNeeded[&I] << " user width=" <<(cast<IntegerType>(I.getType()))->getBitWidth() << " \n";  
                                continue;
                            }
                        }
                        

                        Value *ResultPtr = &I;


                        if (I.mayReadFromMemory())
                        {
                            *VarWidthChangeLog << "                         ------->  this could be a load inst (bypass).\n";
                            continue;
                        }
                        VarWidthChangeLog->flush(); 
                        
                        if (BinaryOperator* BOI = dyn_cast<BinaryOperator>(&I))
                        {
                            *VarWidthChangeLog << "and its operands are:\n";
                            VarWidthChangeLog->flush(); 
                            for (int i = 0; i < I.getNumOperands(); ++i)
                            {
                                if (PtrToIntInst *PTI_I = dyn_cast<PtrToIntInst>(I.getOperand(i)))
                                {
                                    Instruction_BitNeeded[&I] = (cast<IntegerType>(I.getType()))->getBitWidth();
                                }
                            }
                            for (int i = 0; i < I.getNumOperands(); ++i)
                            {
                                *VarWidthChangeLog << "                         ------->  op#"<<i <<"==>"<<*I.getOperand(i)<<"\n";
                                VarWidthChangeLog->flush(); 
                                if (ConstantInt *C_I = dyn_cast<ConstantInt>(I.getOperand(i)))                                
                                {
                                    *VarWidthChangeLog << "                         ------->  op#"<<i<<" "<<*C_I<<" is a constant.\n";
                                    VarWidthChangeLog->flush(); 
                                    //  if (C_I->getBitWidth()!= Instruction_BitNeeded[User_I])
                                    Type *NewTy_C = IntegerType::get(I.getType()->getContext(), Instruction_BitNeeded[&I]);
                                    Constant *New_C = ConstantInt::get(NewTy_C,*C_I->getValue().getRawData());
                                    *VarWidthChangeLog << "                         ------->  update"<<I<<" to ";
                                    VarWidthChangeLog->flush(); 
                                    I.setOperand(i,New_C);
                                    *VarWidthChangeLog <<I<<"\n";
                                }
                                else
                                {
                                    if (Instruction *Op_I = dyn_cast<Instruction>(I.getOperand(i)))
                                    {
                                        *VarWidthChangeLog << "                         ------->  op#"<<i<<" "<<*Op_I<<" is an instruction\n";
                                        VarWidthChangeLog->flush(); 
                                        IRBuilder<> Builder( Op_I->getNextNode());
                                        std::string regNameS = "bcast"+std::to_string(changed_id);
                                        changed_id++;                   
                                        Type *NewTy_OP = IntegerType::get(I.getType()->getContext(), Instruction_BitNeeded[&I]);
                                        if (tmp_CR.getLower().isNegative())
                                        {                                                
                                            ResultPtr = Builder.CreateSExtOrTrunc(Op_I, NewTy_OP,regNameS.c_str());
                                        }
                                        else
                                        {
                                            ResultPtr = Builder.CreateZExtOrTrunc(Op_I, NewTy_OP,regNameS.c_str());                                            
                                        }
                                        *VarWidthChangeLog << "                         ------->  update"<<I<<" to ";
                                        VarWidthChangeLog->flush(); 
                                        I.setOperand(i,ResultPtr);
                                        *VarWidthChangeLog <<I<<"\n";
                                    }

                                }
                                VarWidthChangeLog->flush();                                 
                            }
                            VarWidthChangeLog->flush(); 
                            *VarWidthChangeLog << "                         ------->  op0 type = "<<*BOI->getOperand(0)->getType()<<"\n";
                            *VarWidthChangeLog << "                         ------->  op1 type = "<<*BOI->getOperand(1)->getType()<<"\n";
                            
                            std::string regNameS = "new"+std::to_string(changed_id);
                            BinaryOperator *newBOI = BinaryOperator::Create(BOI->getOpcode(), BOI->getOperand(0), BOI->getOperand(1), "HI."+BOI->getName()+regNameS,BOI); 
                            *VarWidthChangeLog << "                         ------->  new_BOI = "<<*newBOI<<"\n";
                            // BOI->replaceAllUsesWith(newBOI) ;
                            ReplaceUsesUnsafe(BOI, newBOI) ;
                            *VarWidthChangeLog << "                         ------->  accomplish replacement of original instruction in uses.\n";
                            VarWidthChangeLog->flush(); 
                            I.eraseFromParent();
                            *VarWidthChangeLog << "                         ------->  accomplish erasing of original instruction.\n";
                            VarWidthChangeLog->flush(); 
                            take_action = 1;
                            changed = 1;    
                            break;
                        }
                        else if (ICmpInst* ICMP_I = dyn_cast<ICmpInst>(&I))
                        {
                            *VarWidthChangeLog << "and its operands are:\n";
                            VarWidthChangeLog->flush(); 
                            for (int i = 0; i < I.getNumOperands(); ++i)
                            {
                                if (PtrToIntInst *PTI_I = dyn_cast<PtrToIntInst>(I.getOperand(i)))
                                {
                                    Instruction_BitNeeded[&I] = (cast<IntegerType>(I.getType()))->getBitWidth();
                                }
                            }
                            for (int i = 0; i < I.getNumOperands(); ++i)
                            {
                                *VarWidthChangeLog << "                         ------->  op#"<<i <<"==>"<<*I.getOperand(i)<<"\n";
                                VarWidthChangeLog->flush(); 
                                if (ConstantInt *C_I = dyn_cast<ConstantInt>(I.getOperand(i)))                                
                                {
                                    *VarWidthChangeLog << "                         ------->  op#"<<i<<" "<<*C_I<<" is a constant.\n";
                                    VarWidthChangeLog->flush(); 
                                    //  if (C_I->getBitWidth()!= Instruction_BitNeeded[User_I])
                                    Type *NewTy_C = IntegerType::get(I.getType()->getContext(), Instruction_BitNeeded[&I]);
                                    Constant *New_C = ConstantInt::get(NewTy_C,*C_I->getValue().getRawData());
                                    *VarWidthChangeLog << "                         ------->  update"<<I<<" to ";
                                    VarWidthChangeLog->flush(); 
                                    I.setOperand(i,New_C);
                                    *VarWidthChangeLog <<I<<"\n";
                                }
                                else
                                {
                                    if (Instruction *Op_I = dyn_cast<Instruction>(I.getOperand(i)))
                                    {
                                        *VarWidthChangeLog << "                         ------->  op#"<<i<<" "<<*Op_I<<" is an instruction\n";
                                        VarWidthChangeLog->flush(); 
                                        IRBuilder<> Builder( Op_I->getNextNode());
                                        std::string regNameS = "bcast"+std::to_string(changed_id);
                                        changed_id++;                   
                                        Type *NewTy_OP = IntegerType::get(I.getType()->getContext(), Instruction_BitNeeded[&I]);
                                        if (tmp_CR.getLower().isNegative())
                                        {                                                
                                            ResultPtr = Builder.CreateSExtOrTrunc(Op_I, NewTy_OP,regNameS.c_str());
                                        }
                                        else
                                        {
                                            ResultPtr = Builder.CreateZExtOrTrunc(Op_I, NewTy_OP,regNameS.c_str());                                            
                                        }
                                        *VarWidthChangeLog << "                         ------->  update"<<I<<" to ";
                                        VarWidthChangeLog->flush(); 
                                        I.setOperand(i,ResultPtr);
                                        *VarWidthChangeLog <<I<<"\n";
                                    }

                                }
                                VarWidthChangeLog->flush();                                 
                            }
                            VarWidthChangeLog->flush(); 
                            *VarWidthChangeLog << "                         ------->  op0 type = "<<*ICMP_I->getOperand(0)->getType()<<"\n";
                            *VarWidthChangeLog << "                         ------->  op1 type = "<<*ICMP_I->getOperand(1)->getType()<<"\n";
                            
                            std::string regNameS = "new"+std::to_string(changed_id);
                            ICmpInst *newCMP = new ICmpInst(
                                ICMP_I,  ///< Where to insert
                                ICMP_I->getPredicate(),  ///< The predicate to use for the comparison
                                ICMP_I->getOperand(0),      ///< The left-hand-side of the expression
                                ICMP_I->getOperand(1),      ///< The right-hand-side of the expression
                                "HI."+ICMP_I->getName()+regNameS  ///< Name of the instruction
                            ); 
                            *VarWidthChangeLog << "                         ------->  new_CMP = "<<*newCMP<<"\n";
                            // BOI->replaceAllUsesWith(newBOI) ;
                            ReplaceUsesUnsafe(ICMP_I, newCMP) ;
                            *VarWidthChangeLog << "                         ------->  accomplish replacement of original instruction in uses.\n";
                            VarWidthChangeLog->flush(); 
                            I.eraseFromParent();
                            *VarWidthChangeLog << "                         ------->  accomplish erasing of original instruction.\n";
                            VarWidthChangeLog->flush(); 
                            take_action = 1;
                            changed = 1;    
                            break;
                        }
                        else if (PHINode* PHI_I = dyn_cast<PHINode>(&I))
                        {
                            *VarWidthChangeLog << "and its operands are:\n";
                            VarWidthChangeLog->flush(); 
                            for (int i = 0; i < I.getNumOperands(); ++i)
                            {
                                *VarWidthChangeLog << "                         ------->  op#"<<i <<"==>"<<*I.getOperand(i)<<"\n";
                                VarWidthChangeLog->flush(); 
                                if (ConstantInt *C_I = dyn_cast<ConstantInt>(I.getOperand(i)))                                
                                {
                                    *VarWidthChangeLog << "                         ------->  op#"<<i<<" "<<*C_I<<" is a constant.\n";
                                    VarWidthChangeLog->flush(); 
                                    //  if (C_I->getBitWidth()!= Instruction_BitNeeded[User_I])
                                    Type *NewTy_C = IntegerType::get(I.getType()->getContext(), Instruction_BitNeeded[&I]);
                                    Constant *New_C = ConstantInt::get(NewTy_C,*C_I->getValue().getRawData());
                                    *VarWidthChangeLog << "                         ------->  update"<<I<<" to ";
                                    VarWidthChangeLog->flush(); 
                                    I.setOperand(i,New_C);
                                    *VarWidthChangeLog <<I<<"\n";
                                }
                                else
                                {
                                    if (Instruction *Op_I = dyn_cast<Instruction>(I.getOperand(i)))
                                    {
                                        *VarWidthChangeLog << "                         ------->  op#"<<i<<" "<<*Op_I<<" is an instruction\n";
                                        VarWidthChangeLog->flush(); 
                                        IRBuilder<> Builder( Op_I->getNextNode());
                                        std::string regNameS = "bcast"+std::to_string(changed_id);
                                        changed_id++;                   
                                        Type *NewTy_OP = IntegerType::get(I.getType()->getContext(), Instruction_BitNeeded[&I]);
                                        if (tmp_CR.getLower().isNegative())
                                        {                                                
                                            ResultPtr = Builder.CreateSExtOrTrunc(Op_I, NewTy_OP,regNameS.c_str());
                                        }
                                        else
                                        {
                                            ResultPtr = Builder.CreateZExtOrTrunc(Op_I, NewTy_OP,regNameS.c_str());                                            
                                        }
                                        *VarWidthChangeLog << "                         ------->  update"<<I<<" to ";
                                        VarWidthChangeLog->flush(); 
                                        I.setOperand(i,ResultPtr);
                                        *VarWidthChangeLog <<I<<"\n";
                                    }

                                }
                                VarWidthChangeLog->flush();                                 
                            }
                            VarWidthChangeLog->flush(); 
                            *VarWidthChangeLog << "                         ------->  op0 type = "<<*PHI_I->getOperand(0)->getType()<<"\n";
                            *VarWidthChangeLog << "                         ------->  op1 type = "<<*PHI_I->getOperand(1)->getType()<<"\n";
                            Type *NewTy_PHI = IntegerType::get(I.getType()->getContext(), Instruction_BitNeeded[&I]);
                            std::string regNameS = "new"+std::to_string(changed_id);
                            PHINode *new_PHI = PHINode::Create(NewTy_PHI, 0, "HI."+PHI_I->getName()+regNameS,PHI_I);
                            for (int i = 0; i < I.getNumOperands(); ++i)
                            {
                                new_PHI->addIncoming(PHI_I->getIncomingValue(i),PHI_I->getIncomingBlock(i));
                            }
                            *VarWidthChangeLog << "                         ------->  new_PHI_I = "<<*new_PHI<<"\n";
                            
                            // BOI->replaceAllUsesWith(newBOI) ;
                            ReplaceUsesUnsafe(PHI_I, new_PHI) ;
                            *VarWidthChangeLog << "                         ------->  accomplish replacement of original instruction in uses.\n";
                            VarWidthChangeLog->flush(); 
                            I.eraseFromParent();
                            *VarWidthChangeLog << "                         ------->  accomplish erasing of original instruction.\n";
                            VarWidthChangeLog->flush(); 
                            take_action = 1;
                            changed = 1;    
                            break;
                        }
                        else
                        {
                            *VarWidthChangeLog << "and it is not a binary operator.(bypass)\n";
                        }
                        
                    }

                }
                else
                {
                    *VarWidthChangeLog <<"\n\n\n find non-target instrction " <<*I.getType() <<":" << I ;
                    VarWidthChangeLog->flush(); 
                }
            }
        }    
        *VarWidthChangeLog << "\n";
        VarWidthChangeLog->flush(); 
    }


    *VarWidthChangeLog << "==============================================\n==============================================\n\n\n\n\n\n";
    for (BasicBlock &B : F) 
    {
        bool rmflag = 1;
        while (rmflag)
        {
            rmflag = 0;
            for (Instruction &I: B) 
            {
                *VarWidthChangeLog << "                         ------->checking redunctan CastI: " << I  <<"\n";
                if (CastInst * CastI = dyn_cast<CastInst>(&I))         
                {
                    if (CastI->getOpcode()!=Instruction::Trunc && CastI->getOpcode()!=Instruction::ZExt && CastI->getOpcode()!=Instruction::SExt)
                    {
                        continue;
                    }
                    if (CastI->getType()->getIntegerBitWidth() == I.getOperand(0)->getType()->getIntegerBitWidth())
                    {
                        *VarWidthChangeLog << "                         ------->remove redunctan CastI: " << *CastI  <<"\n";
                        *VarWidthChangeLog << "                         ------->replace CastI with its operand 0: " << *I.getOperand(0)  <<"\n";
                        VarWidthChangeLog->flush(); 
                        ReplaceUsesUnsafe(&I,I.getOperand(0));
                       // I.replaceAllUsesWith(I.getOperand(0));
                        I.eraseFromParent();
                        rmflag = 1;
                        break;
                    }
                }            
            }   
        }
 
    }




    // Forward Process
   // return false;
    for (BasicBlock &B : F) 
    {
        bool take_action = 1;
        while(take_action)
        {
            take_action = 0;
            for (Instruction &I: B) 
            {
                *VarWidthChangeLog << "checking Instruction width: " << I << " ";
                if (I.getType()->isIntegerTy())
                {
                    const SCEV *tmp_S = SE->getSCEV(&I);
                    ConstantRange tmp_CR1 = SE->getSignedRange(tmp_S);
                    *VarWidthChangeLog << "CR-bw=" << tmp_CR1.getBitWidth() << " type-bw="<<I.getType()->getIntegerBitWidth() <<"\n";
                    if (tmp_CR1.getBitWidth() != I.getType()->getIntegerBitWidth())
                        *VarWidthChangeLog << "Bit width error!!!\n";
                }
                else
                {
                    *VarWidthChangeLog << "is not an integer type.\n ";
                }
  

            }
        }    
        *VarWidthChangeLog << "\n";
        VarWidthChangeLog->flush(); 
    }





    *VarWidthChangeLog << "==============================================\n==============================================\n\n\n\n\n\n";
    for (BasicBlock &B : F) 
    {
        *VarWidthChangeLog << B.getName() <<"\n";
        for (Instruction &I: B) 
        {
            *VarWidthChangeLog << "   " << I<<"\n";            
            
        }    
        *VarWidthChangeLog << "-------------------\n";
    }

    VarWidthChangeLog->flush(); 


    if (changed)
    {
        *VarWidthChangeLog << "THE IR CODE IS CHANGED\n";
    }
    else
    {
        *VarWidthChangeLog << "THE IR CODE IS NOT CHANGED\n";
    }
    return changed;
}

void HI_VarWidthReduce::ReplaceUsesUnsafe(Instruction *from, Value *to) 
{
    *VarWidthChangeLog << "            ------  replacing  " << *from << " in its user\n";
    while (!from->use_empty()) 
    {
        // for (auto it = from->use_begin(), ie = from->use_end(); it!=ie; ++it )
        // {
        //     if (User *U_to = dyn_cast<User>(to))
        //     {
        //         if (U_to==it->getUser())
        //             continue;
        //         *VarWidthChangeLog << "            ------  replacing the original inst in " << *it->getUser() << " with " << *to <<"\n";
        //     }
        //     it->set(to);
        //     break;
        // }
        User* tmp_user = from->use_begin()->getUser();
        *VarWidthChangeLog << "            ------  replacing the original inst in " << *from->use_begin()->getUser() << " with " << *to <<"\n";
        from->use_begin()->set(to);
        *VarWidthChangeLog << "            ------  new user => " << *tmp_user << "\n";
        *VarWidthChangeLog << "            ------  from->getNumUses() "<< from->getNumUses() << "\n";
    }
    //from->eraseFromParent();
}

char HI_VarWidthReduce::ID = 0;  // the ID for pass should be initialized but the value does not matter, since LLVM uses the address of this variable as label instead of its value.

void HI_VarWidthReduce::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    // AU.addRequired<LazyValueInfoWrapperPass>();    
    // AU.setPreservesCFG();
}



/// Determine the range for a particular SCEV.  If SignHint is
/// HINT_RANGE_UNSIGNED (resp. HINT_RANGE_SIGNED) then getRange prefers ranges
/// with a "cleaner" unsigned (resp. signed) representation.
const ConstantRange HI_VarWidthReduce::HI_getSignedRangeRef(const SCEV *S) 
{

    *VarWidthChangeLog << "        ------  HI_getSignedRangeRef handling SECV: " << *S->getType() << "\n";
    ConstantRange tmp_CR1 = SE->getSignedRange(S);
    if (!tmp_CR1.isFullSet())
    {
        *VarWidthChangeLog << "        ------  HI_getSignedRangeRef: it is not full-set " << tmp_CR1 << "\n";
        return tmp_CR1;
    }
    DenseMap<const SCEV *, ConstantRange> &Cache = SignedRanges;
    *VarWidthChangeLog << "        ------  handling full-set SECV: " << *S->getType() << "\n";
    // See if we've computed this range already.
    DenseMap<const SCEV *, ConstantRange>::iterator I = Cache.find(S);
    if (I != Cache.end())
        return I->second;

    if (const SCEVConstant *C = dyn_cast<SCEVConstant>(S))
        return setRange(C, ConstantRange(C->getAPInt()));

    unsigned BitWidth = SE->getTypeSizeInBits(S->getType());
    ConstantRange ConservativeResult(BitWidth, /*isFullSet=*/true);

    // If the value has known zeros, the maximum value will have those known zeros
    // as well.
    uint32_t TZ = SE->GetMinTrailingZeros(S);
    if (TZ != 0) 
    {
        ConservativeResult = ConstantRange(
            APInt::getSignedMinValue(BitWidth),
            APInt::getSignedMaxValue(BitWidth).ashr(TZ).shl(TZ) + 1);
    }

    if (const SCEVAddExpr *Add = dyn_cast<SCEVAddExpr>(S)) 
    {
        *VarWidthChangeLog << "        ------  Add\n";
        ConstantRange X = HI_getSignedRangeRef(Add->getOperand(0));
        for (unsigned i = 1, e = Add->getNumOperands(); i != e; ++i)
            if (bypassPTI(Add->getOperand(i))) 
                continue;
            else
                X = X.add(HI_getSignedRangeRef(Add->getOperand(i)));
        *VarWidthChangeLog << "            ------  handling full-set SECV new range: " << X << "\n";
        return setRange(Add, ConservativeResult.intersectWith(X));
    }

    if (const SCEVMulExpr *Mul = dyn_cast<SCEVMulExpr>(S)) 
    {
        *VarWidthChangeLog << "        ------  Mul\n";
        ConstantRange X = HI_getSignedRangeRef(Mul->getOperand(0));
        for (unsigned i = 1, e = Mul->getNumOperands(); i != e; ++i)
            if (bypassPTI(Mul->getOperand(i))) 
                continue; 
            else
                X = X.multiply(HI_getSignedRangeRef(Mul->getOperand(i)));
          *VarWidthChangeLog << "            ------  handling full-set SECV new range: " << X << "\n";
        return setRange(Mul, ConservativeResult.intersectWith(X));
    }

    if (const SCEVSMaxExpr *SMax = dyn_cast<SCEVSMaxExpr>(S)) 
    {
        *VarWidthChangeLog << "        ------  SMax\n";
        ConstantRange X = HI_getSignedRangeRef(SMax->getOperand(0));
        for (unsigned i = 1, e = SMax->getNumOperands(); i != e; ++i)
            if (bypassPTI(SMax->getOperand(i))) 
                continue;
            else
                X = X.smax(HI_getSignedRangeRef(SMax->getOperand(i)));
        *VarWidthChangeLog << "          ------  handling full-set SECV new range: " << X << "\n";
        return setRange(SMax, ConservativeResult.intersectWith(X));
    }

    if (const SCEVUMaxExpr *UMax = dyn_cast<SCEVUMaxExpr>(S)) 
    {
        *VarWidthChangeLog << "        ------  UMax\n";
        ConstantRange X = HI_getSignedRangeRef(UMax->getOperand(0));
        for (unsigned i = 1, e = UMax->getNumOperands(); i != e; ++i)
            if (bypassPTI(UMax->getOperand(i))) 
                continue;
            else
                X = X.umax(HI_getSignedRangeRef(UMax->getOperand(i)));
        *VarWidthChangeLog << "          ------  handling full-set SECV new range: " << X << "\n";
        return setRange(UMax, ConservativeResult.intersectWith(X));
    }

    if (const SCEVUDivExpr *UDiv = dyn_cast<SCEVUDivExpr>(S)) 
    {
        *VarWidthChangeLog << "        ------  UDiv\n";
        if (bypassPTI(UDiv->getLHS())) return ConservativeResult;
        if (bypassPTI(UDiv->getRHS())) return ConservativeResult;
        ConstantRange X = HI_getSignedRangeRef(UDiv->getLHS());
        ConstantRange Y = HI_getSignedRangeRef(UDiv->getRHS());
        return setRange(UDiv,
                        ConservativeResult.intersectWith(X.udiv(Y)));
    }

    if (const SCEVZeroExtendExpr *ZExt = dyn_cast<SCEVZeroExtendExpr>(S)) 
    {
        *VarWidthChangeLog << "        ------  ZExt\n";
        if (bypassPTI(ZExt->getOperand())) return ConservativeResult;
        ConstantRange X = HI_getSignedRangeRef(ZExt->getOperand());
        return setRange(ZExt,
                        ConservativeResult.intersectWith(X.zeroExtend(BitWidth)));
    }

    if (const SCEVSignExtendExpr *SExt = dyn_cast<SCEVSignExtendExpr>(S)) 
    {
        *VarWidthChangeLog << "        ------  SExt\n";
        if (bypassPTI(SExt->getOperand())) return ConservativeResult;
        ConstantRange X = HI_getSignedRangeRef(SExt->getOperand());
        return setRange(SExt,
                        ConservativeResult.intersectWith(X.signExtend(BitWidth)));
    }

    if (const SCEVTruncateExpr *Trunc = dyn_cast<SCEVTruncateExpr>(S)) 
    {
        *VarWidthChangeLog << "        ------  Trunc\n";
        if (bypassPTI(Trunc->getOperand())) return ConservativeResult;
        ConstantRange X = HI_getSignedRangeRef(Trunc->getOperand());
        return setRange(Trunc,
                        ConservativeResult.intersectWith(X.truncate(BitWidth)));
    }

    if (const SCEVAddRecExpr *AddRec = dyn_cast<SCEVAddRecExpr>(S)) 
    {
        *VarWidthChangeLog << "        ------  SCEVAddRecExpr\n";
    }

    if (const SCEVUnknown *U = dyn_cast<SCEVUnknown>(S)) 
    {
        *VarWidthChangeLog << "        ------  SCEVUnknown\n";
        if (PtrToIntInst *PTI = dyn_cast<PtrToIntInst>(U->getValue()))
        {   

        }

    }
    *VarWidthChangeLog << "        ------  Out of Scope\n";
    return setRange(S, std::move(ConservativeResult));
}


const ConstantRange &HI_VarWidthReduce::setRange(const SCEV *S,  ConstantRange CR) 
{
    DenseMap<const SCEV *, ConstantRange> &Cache = SignedRanges;

    auto Pair = Cache.try_emplace(S, std::move(CR));
    if (!Pair.second)
        Pair.first->second = std::move(CR);
    return Pair.first->second;
}

bool HI_VarWidthReduce::bypassPTI(const SCEV *S)
{
    if (const SCEVUnknown *U = dyn_cast<SCEVUnknown>(S))
        if (PtrToIntInst *PTI = dyn_cast<PtrToIntInst>(U->getValue()))
        {
            *VarWidthChangeLog << "            ------  bypassing range evaluation for PtrToIntInst: " << *U->getValue() << "\n";
            return true;
        }
    return false;
}

unsigned int HI_VarWidthReduce::bitNeededFor(ConstantRange CR)
{
    if (CR.isFullSet())
        return CR.getBitWidth();
    if (CR.getLower().isNonNegative())
    {
        unsigned int lowerNeedBits = CR.getLower().getActiveBits();
        unsigned int upperNeedBits = CR.getUpper().getActiveBits();

        if (lowerNeedBits > upperNeedBits) 
            return lowerNeedBits;
        else
            return upperNeedBits;
    }
    else
    {
        unsigned int lowerNeedBits = CR.getLower().getMinSignedBits();
        unsigned int upperNeedBits = CR.getUpper().getMinSignedBits();

        if (lowerNeedBits > upperNeedBits) 
            return lowerNeedBits;
        else
            return upperNeedBits;
    }
    

}



// bool HI_VarWidthReduce::runOnFunction(Function &F) // The runOnModule declaration will overide the virtual one in ModulePass, which will be executed for each Module.
// {
//     const DataLayout &DL = F.getParent()->getDataLayout();
//     SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
//     LazyValueInfo* LazyI = &getAnalysis<LazyValueInfoWrapperPass>().getLVI();
    
//     if (Function_id.find(&F)==Function_id.end())  // traverse functions and assign function ID
//     {
//         Function_id[&F] = ++Function_Counter;
//     }
//     bool changed = 0;

//     // Analysis
//     for (BasicBlock &B : F) 
//     {
//         for (Instruction &I: B) 
//         {
//             if (I.getType()->isIntegerTy())
//             {
//                 Instruction_id[&I] = ++Instruction_Counter;
//                 KnownBits tmp_KB = computeKnownBits(&I,DL); 
//                 const SCEV *tmp_S = SE->getSCEV(&I);
//                 ConstantRange tmp_CR1 = SE->getSignedRange(tmp_S);
//                 ConstantRange tmp_CR2 = HI_getSignedRangeRef(tmp_S);
//                 *VarWidthChangeLog << I << "---- Ori-CR: "<<tmp_CR1 << "(bw=" << I.getType()->getIntegerBitWidth() <<") ---- HI-CR:"<<tmp_CR2 << "(bw=" << bitNeededFor(tmp_CR2) <<")\n";
//                 if (I.mayReadFromMemory())
//                 {
//                     Instruction_BitNeeded[&I] = I.getType()->getIntegerBitWidth();
//                     *VarWidthChangeLog << "        ----  this could be a load inst.\n";
//                 }
//                 else
//                 {
//                     for (int i = 0;i < I.getNumOperands(); i++)
//                     {
//                         *VarWidthChangeLog << "        ----  evaluating Op#"<<i<<"\n";
//                         const SCEV *sub_S = SE->getSCEV(I.getOperand(i));
//                         unsigned int bit_tmp = 0;
//                         if (Instruction* op_I= dyn_cast<Instruction>(I.getOperand(i)))
//                         {
//                             if (op_I->mayReadFromMemory())
//                             {
//                                 bit_tmp = bitNeededFor(SE->getSignedRange(sub_S));
//                             }
//                             else
//                             {
//                                 bit_tmp = bitNeededFor(HI_getSignedRangeRef(sub_S));
//                             }
                            
//                         }
//                         else
//                         {
//                             bit_tmp = bitNeededFor(HI_getSignedRangeRef(sub_S));
//                         }                  
                        
//                         if (Instruction_BitNeeded[&I] < bit_tmp) Instruction_BitNeeded[&I] = bit_tmp;
//                         *VarWidthChangeLog << "        ----  evaluating Op#"<<i<<" <----"<<bit_tmp<<"\n";
//                     }
//                 }
                
//                 *VarWidthChangeLog << "\n\n\n";
//             }
//         }    
//         *VarWidthChangeLog << "\n";
//     }


//     // Forward Process
//     unsigned int changed_id = 0;
//     for (BasicBlock &B : F) 
//     {
//         for (Instruction &I: B) 
//         {
//             if (Instruction_id.find(&I) != Instruction_id.end())
//             {
//                 if (CmpInst *cmpI = dyn_cast<CmpInst>(&I))
//                     continue;
//                 if (PtrToIntInst *PTI = dyn_cast<PtrToIntInst>(&I))
//                     continue;
//                 if (IntToPtrInst *ITP = dyn_cast<IntToPtrInst>(&I))
//                     continue;

                
//                 if (I.getType()->isIntegerTy())
//                 {
//                     changed = 1;
//                     *VarWidthChangeLog <<"\n\n\n" << I << "------- under processing (targetBW="<<Instruction_BitNeeded[&I]<<", curBW="<< (cast<IntegerType>(I.getType()))->getBitWidth()<<") "<< "and its users are:\n";   

//                     for (User *U : I.users())
//                     {
//                         if (Instruction *User_I = dyn_cast<Instruction>(U))
//                         {
//                             if (Instruction_id.find(User_I) != Instruction_id.end())                            
//                                 *VarWidthChangeLog << "                         -------> User: " << *User_I << "  is an target instruction " << "\n"; 
//                             else
//                                 *VarWidthChangeLog << "                         -------> User: " << *User_I << "  is an non-target instruction " << "\n";           
                            
//                         }
//                         else
//                         {
//                             *VarWidthChangeLog << "                         -------> User: " << *U << "  is not an instruction " << "\n";  
//                         }
//                     } 
//                     bool User_Modifed = 1;
                    
//                     while   (User_Modifed)  
//                     {
//                         User_Modifed = 0;
//                         for (User *U : I.users())
//                         {
//                             if (Instruction *User_I = dyn_cast<Instruction>(U))
//                             {
//                                 *VarWidthChangeLog << "                         -------> processing User: " << *User_I;  
                            
//                                 if (Instruction_id.find(User_I) == Instruction_id.end())
//                                 {
//                                     *VarWidthChangeLog << " but it is not the targets. (bypass) \n";  
//                                     continue;
//                                 }
//                                 if (CmpInst *cmpI = dyn_cast<CmpInst>(User_I))
//                                 {
//                                     *VarWidthChangeLog << " but it is comparison instruction. (bypass) \n"; 
//                                     continue;
//                                 }
//                                 if (PtrToIntInst *PTI = dyn_cast<PtrToIntInst>(User_I))
//                                 {
//                                     *VarWidthChangeLog << " but it is PtrToInt instruction. (bypass) \n"; 
//                                     continue;
//                                 }
//                                 if (IntToPtrInst *ITP = dyn_cast<IntToPtrInst>(User_I))
//                                 {
//                                     *VarWidthChangeLog << " but it is IntToPtr instruction. (bypass) \n"; 
//                                     continue;
//                                 }
//                                 *VarWidthChangeLog << " and NumOperand="<<User_I->getNumOperands()<<"\n"; 
//                                 for (int i = 0; i < User_I->getNumOperands(); ++i)
//                                 {
//                                     if (Instruction *OP_I = dyn_cast<Instruction>(User_I->getOperand(i)))                                
//                                     {
//                                         if (OP_I==&I)
//                                         {
//                                             *VarWidthChangeLog << "                         -------> User: " << *User_I << "  -- Op#= " << i << "\n";  
//                                             const SCEV *tmp_S = SE->getSCEV(User_I);
//                                             ConstantRange tmp_CR2 = HI_getSignedRangeRef(tmp_S);
//                                             Type *NewTy = IntegerType::get(User_I->getType()->getContext(), Instruction_BitNeeded[User_I]);
//                                             if (Instruction_BitNeeded[User_I] == (cast<IntegerType>(User_I->getType()))->getBitWidth())
//                                             {
//                                                 *VarWidthChangeLog << "                         -------> User: " << *User_I << "  ---needs no update req="<< Instruction_BitNeeded[User_I] << " user width=" <<(cast<IntegerType>(User_I->getType()))->getBitWidth() << " \n";  
//                                                 break;
//                                             }
//                                             Value *ResultPtr = &I;
//                                             IRBuilder<> Builder(I.getNextNode());
//                                             std::string regNameS = "bcast"+std::to_string(changed_id);
//                                             changed_id++;
//                                             if (Instruction_BitNeeded[&I] != (cast<IntegerType>(I.getType()))->getBitWidth())
//                                             {
//                                                 if (tmp_CR2.getLower().isNegative())
//                                                 {                                                
//                                                     ResultPtr = Builder.CreateSExtOrTrunc(&I, NewTy,regNameS.c_str());
//                                                 }
//                                                 else
//                                                 {
//                                                     ResultPtr = Builder.CreateZExtOrTrunc(&I, NewTy,regNameS.c_str());                                            
//                                                 }
//                                             }
//                                             else
//                                             {
//                                                 *VarWidthChangeLog <<  "                         ------->  "<< I << " processed \n"; 
//                                             }
                                            
//                                             User_I->setOperand(i,ResultPtr);
//                                             *VarWidthChangeLog <<  "                         -------> User: " << *User_I << "  -- Op#= " << i << " replaced by "<< *ResultPtr << " \n";
//                                             changed = 1;
//                                             User_Modifed = 1;
//                                             break;
//                                         }                                    
//                                     }
//                                     VarWidthChangeLog->flush(); 
//                                 }
//                                 for (int i = 0; i < User_I->getNumOperands(); ++i)
//                                 {
//                                     if (ConstantInt *C_I = dyn_cast<ConstantInt>(User_I->getOperand(i)))                                
//                                     {
//                                       //  if (C_I->getBitWidth()!= Instruction_BitNeeded[User_I])
//                                         Type *NewTy = IntegerType::get(C_I->getType()->getContext(), Instruction_BitNeeded[User_I]);
//                                         Constant *New_C = ConstantInt::get(NewTy,*C_I->getValue().getRawData());
//                                         User_I->setOperand(i,New_C);
//                                        // C_I->erasefr();
//                                         *VarWidthChangeLog <<  "                         -------> User: " << *User_I << "  -- Op#= " << i << "  is a constant\n";
//                                     }
//                                 }
//                             }
//                         }
//                     }     
//                 }
//             }
//         }    
//         *VarWidthChangeLog << "\n";
//     }

//     *VarWidthChangeLog << "==============================================\n";
//     for (BasicBlock &B : F) 
//     {
//         for (Instruction &I: B) 
//         {
//             *VarWidthChangeLog << "   " << I<<"\n";            
            
//         }    
//         *VarWidthChangeLog << "-------------------\n";
//     }

//     VarWidthChangeLog->flush(); 
//     return changed;
// }
