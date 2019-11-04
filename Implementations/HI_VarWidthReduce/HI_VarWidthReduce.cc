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



/*
    1.Analysis: check the value range of the instructions in the source code and determine the bitwidth (select the maximum bitwidth among input and output)
    2.Forward Process: check the bitwidth of operands and output of an instruction, trunc/ext the operands, update the bitwidth of the instruction
    3.Check Redundancy: Some instructions could be truncated to be an operand, but itself is actually updated with the same bitwidth with the truncation.
    4.Validation Check: Check whether there is any binary operation with operands in different types.
*/
bool HI_VarWidthReduce::runOnFunction(Function &F) // The runOnModule declaration will overide the virtual one in ModulePass, which will be executed for each Module.
{
    print_status("Running HI_VarWidthReduce pass.");  
    SignedRanges.clear();
    UnsignedRanges.clear();
    if (F.getName().find("llvm.")!=std::string::npos)
    {
        return false;
    }
    else
    {
        if (DEBUG) *VarWidthChangeLog << "\n============================================================\nstart to process function: ["
                                                                                                        << F.getName() << "]\n";
    }
    
    const DataLayout &DL = F.getParent()->getDataLayout();
    SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    
    if (Function_id.find(&F)==Function_id.end())  // traverse functions and assign function ID
    {
        Function_id[&F] = ++Function_Counter;
    }
    bool changed = 0;

    // 1.Analysis: check the value range of the instructions in the source code and determine the bitwidth
    Bitwidth_Analysis(&F);

    // 2.Forward Process: check the bitwidth of operands and output of an instruction, trunc/ext the operands, update the bitwidth of the instruction
    changed |= InsturctionUpdate_WidthCast(&F);

    // 3.Check Redundancy: Some instructions could be truncated to be an operand, but itself is actually updated with the same bitwidth with the truncation.
    changed |= RedundantCastRemove(&F);

    // 4.Validation Check: Check whether there is any binary operation with operands in different types.
    VarWidthReduce_Validation(&F);


    if (changed)    
        if (DEBUG) *VarWidthChangeLog << "THE IR CODE IS CHANGED\n";    
    else    
        if (DEBUG) *VarWidthChangeLog << "THE IR CODE IS NOT CHANGED\n";
    VarWidthChangeLog -> flush();
    return changed;
}


char HI_VarWidthReduce::ID = 0;  // the ID for pass should be initialized but the value does not matter, since LLVM uses the address of this variable as label instead of its value.

void HI_VarWidthReduce::getAnalysisUsage(AnalysisUsage &AU) const {
    // AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    // AU.addRequired<TargetTransformInfoWrapperPass>();
    // AU.addRequired<LoopInfoWrapperPass>();
    // AU.addRequired<LazyValueInfoWrapperPass>();    
    // AU.setPreservesCFG();
}


// Analysis: check the value range of the instructions in the source code and determine the bitwidth
void HI_VarWidthReduce::Bitwidth_Analysis(Function *F) 
{
    const DataLayout &DL = F->getParent()->getDataLayout();
    SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    for (BasicBlock &B : *F) 
    {
        for (Instruction &I: B) 
        {
            if (I.getType()->isIntegerTy())
            {
                Instruction_id[&I] = ++Instruction_Counter;
                KnownBits tmp_KB = computeKnownBits(&I,DL); 
                const SCEV *tmp_S = SE->getSCEV(&I);

                // LLVM-Provided Value Range Evaluation (may be wrong with HLS because it takes array entries as memory address,
                // but in HLS, array entries are just memory ports. Operations, e.g. addition, with memory port are just to get the right address)
                bool isUnsignedInst = I.getOpcode() == Instruction::UDiv || I.getOpcode() == Instruction::URem;


                if (I.mayReadFromMemory()) // if the instruction is actually a load instruction, the bitwidth should be the bitwidth of memory bitwidth
                {
                    Instruction_BitNeeded[&I] = I.getType()->getIntegerBitWidth();
                    I2NeedSign[&I] = 1;
                    if (DEBUG) *VarWidthChangeLog << "        ----  this could be a load inst.\n";
                }
                else  // otherwise, extract the bitwidth from the value range
                {
                    ConstantRange tmp_CR1 = isUnsignedInst ? SE->getUnsignedRange(tmp_S) : SE->getSignedRange(tmp_S);
                    

                    // HI Value Range Evaluation, take the array entries as ZERO offsets and will not effect the result of the value range of address   
                    
                    ConstantRange tmp_CR2 = isUnsignedInst ? ConstantRange(APInt( HI_getUnsignedRangeRef(tmp_S).getBitWidth(),0), HI_getUnsignedRangeRef(tmp_S).getUpper()) :  HI_getSignedRangeRef(tmp_S);

                    I2NeedSign[&I] = !isUnsignedInst && tmp_CR2.getLower().isNegative();

                    if (DEBUG) *VarWidthChangeLog << I << " isUnsigned:" << isUnsignedInst << " ---- Ori-CR: "<<tmp_CR1    
                                        << "(" << tmp_CR1.getUnsignedMin().getSExtValue() << "->" << tmp_CR1.getUnsignedMax().getSExtValue() << ")"
                                        << "(bw=" << I.getType()->getIntegerBitWidth() <<") ---- HI-CR:"<<tmp_CR2 << "(bw=" ;

                    if (ICmpInst *Icmp_I = dyn_cast<ICmpInst>(&I))
                    {
                        Instruction_BitNeeded[&I] = 1;
                        for (int i =0;i<Icmp_I->getNumOperands();i++)
                        {
                            if (HI_getBidwith(Icmp_I->getOperand(i)) > Instruction_BitNeeded[&I] )
                            {
                                Instruction_BitNeeded[&I] = HI_getBidwith(Icmp_I->getOperand(i)) ;
                            }
                        }
                    }
                    else
                    {
                        // select the maximum bitwidth among input and output
                        Instruction_BitNeeded[&I] = bitNeededFor(tmp_CR2);
                        for (int i =0;i<I.getNumOperands();i++)
                        {
                            if (HI_getBidwith(I.getOperand(i)) > Instruction_BitNeeded[&I] )
                            {
                                Instruction_BitNeeded[&I] = HI_getBidwith(I.getOperand(i)) ;
                            }
                        }
                    }
                }                
                 if (DEBUG) *VarWidthChangeLog << Instruction_BitNeeded[&I] <<")\n";
                if (DEBUG) *VarWidthChangeLog << "\n\n\n";
                VarWidthChangeLog->flush();
            }
        }    
        if (DEBUG) *VarWidthChangeLog << "\n";
    }
}

unsigned int HI_VarWidthReduce::HI_getBidwith(Value *I)
{
    if (I->getType()->isIntOrPtrTy()||I->getType()->isIntegerTy()||I->getType()->isIntOrIntVectorTy())
    {
        const SCEV *tmp_S = SE->getSCEV(I);

        bool isUnsignedInst = false; 
        if (auto InstI = dyn_cast<Instruction>(I))
        {
            isUnsignedInst = InstI->getOpcode() == Instruction::UDiv || InstI->getOpcode() == Instruction::URem;
        }

        ConstantRange tmp_CR2 = isUnsignedInst ? ConstantRange(APInt( HI_getUnsignedRangeRef(tmp_S).getBitWidth(),0), HI_getUnsignedRangeRef(tmp_S).getUpper()) : HI_getSignedRangeRef(tmp_S);
        return bitNeededFor(tmp_CR2);
    }
    else
    {
        if (I->getType()->isFloatTy())
            return 32;
        if (I->getType()->isDoubleTy())
            return 64;
        llvm::errs() << "unknownTy: " << *I << "\n";
        assert(false && "unknown type");
    }
    
}

// Forward Process: check the bitwidth of operands and output of an instruction, trunc/ext the operands, update the bitwidth of the instruction
bool HI_VarWidthReduce::InsturctionUpdate_WidthCast(Function *F) 
{
    const DataLayout &DL = F->getParent()->getDataLayout();
    // SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    bool changed = 0;
    for (BasicBlock &B : *F) 
    {
        bool take_action = 1;
      //  while(take_action)
        {
            take_action = 0;
            std::vector<Instruction*> InstInBlock;
            for (auto &I : B)
                InstInBlock.push_back(&I);
            for (auto I: InstInBlock)  // TODO: try to improve the way to handle instructions if you want to remove some of them
            {
                if (Instruction_id.find(I) != Instruction_id.end())
                {
                    Instruction_id.erase(I);
                    if (DEBUG) *VarWidthChangeLog <<"\n\n\n find target instrction " <<*I->getType() <<":" << I ;
                    //VarWidthChangeLog->flush(); 

                    // bypass cast operations
                    if (CastInst *CastI = dyn_cast<CastInst>(I))
                        continue;

                    
                    if (I->getType()->isIntegerTy())
                    {
                        changed = 1;
                        if (DEBUG) *VarWidthChangeLog <<"\n" <<*I->getType() <<":" << *I ;
                        if (DEBUG) *VarWidthChangeLog << "------- under processing (targetBW="<<Instruction_BitNeeded[I]<<", curBW="<< (cast<IntegerType>(I->getType()))->getBitWidth()<<") ";   
                        //VarWidthChangeLog->flush(); 

                        // bypass load instructions
                        if (I->mayReadFromMemory())
                        {
                            if (DEBUG) *VarWidthChangeLog << "                         ------->  this could be a load inst (bypass).\n";
                            continue;
                        }
                        
                        // for comp instruction, we just need to ensure that, the two operands are in the same type
                        if (ICmpInst* ICMP_I = dyn_cast<ICmpInst>(I))
                        {                            
                            if (cast<IntegerType>(ICMP_I->getOperand(0)->getType())->getIntegerBitWidth() == cast<IntegerType>(ICMP_I->getOperand(1)->getType())->getIntegerBitWidth())
                            {
                                if (DEBUG) *VarWidthChangeLog << "\n                         -------> Inst: " << I << "  ---needs no update req="<< Instruction_BitNeeded[I] << " user width=" <<(cast<IntegerType>(I->getType()))->getBitWidth() << " \n";  
                                continue;
                            }
                        }

                        // check whether all the elements (input+output) in the same type.
                        if (Instruction_BitNeeded[I] == (cast<IntegerType>(I->getType()))->getBitWidth())
                        {
                            bool neq = 0;
                            for (int i = 0; i < I->getNumOperands(); ++i)
                            {
                                neq |= Instruction_BitNeeded[I] != (cast<IntegerType>(I->getOperand(i)->getType()))->getBitWidth();
                            }
                            if (!neq)
                            {
                                if (DEBUG) *VarWidthChangeLog << "\n                         -------> Inst: " << I << "  ---needs no update req="<< Instruction_BitNeeded[I] << " user width=" <<(cast<IntegerType>(I->getType()))->getBitWidth() << " \n";  
                                continue;
                            }
                        }                      
                        


                        //VarWidthChangeLog->flush(); 
                        
                        // process different operations with corresponding consideration/procedure
                        if (ICmpInst* ICMP_I = dyn_cast<ICmpInst>(I))
                        {
                            if (DEBUG) *VarWidthChangeLog << "          "<< *ICMP_I << " is a ICmpInst\n ";
                            ICMP_WidthCast(ICMP_I);
                            take_action = 1;
                            changed = 1;    
                     //       break;
                        }
                        else if (llvm::PHINode* PHI_I = dyn_cast<llvm::PHINode>(I))
                        {
                            if (DEBUG) *VarWidthChangeLog << "          "<< *PHI_I << " is a PHINode\n ";
                            PHI_WidthCast(PHI_I);
                            take_action = 1;
                            changed = 1;    
                     //       break;
                        }
                        else if (BinaryOperator* BOI = dyn_cast<BinaryOperator>(I))
                        {
                            if (DEBUG) *VarWidthChangeLog << "          "<< *BOI << " is a BinaryOperator\n ";
                            BOI_WidthCast(BOI);
                            take_action = 1;
                            changed = 1;    
                         //   break;
                        }
                        // else if (CallInst* Call_I = dyn_cast<CallInst>(&I))
                        // {
                        //     if (DEBUG) *VarWidthChangeLog << "          "<< *Call_I << " is a CallInst\n ";
                        //     CallInst_WidthCast(Call_I);
                        //     take_action = 1;
                        //     changed = 1;    
                        //     break;
                        // }
                        else
                        {
                            if (DEBUG) *VarWidthChangeLog << "and it is not a operator which HI_VarWidthReduce can be applied.(bypass)\n";
                        }                        
                    }
                }
                else
                {
                    // if (DEBUG) *VarWidthChangeLog <<"\n\n\n find non-target instrction " <<*I.getType() <<":" << I ;
                    //VarWidthChangeLog->flush(); 
                }
            }
        }    
        if (DEBUG) *VarWidthChangeLog << "\n";
        //VarWidthChangeLog->flush(); 
    }
    return changed;
}


// Check Redundancy: Some instructions could be truncated to be an operand, but itself is actually updated with the same bitwidth with the truncation.
bool HI_VarWidthReduce::RedundantCastRemove(Function *F)
{
    bool changed = 0;
    if (DEBUG) *VarWidthChangeLog << "==============================================\n==============================================\n\n\n\n\n\n";
    for (BasicBlock &B : *F) 
    {
        bool rmflag = 1;
        while (rmflag)
        {
            rmflag = 0;
            for (Instruction &I: B) 
            {
                if (DEBUG) *VarWidthChangeLog << "                         ------->checking redunctan CastI: " << I  <<"\n";
                if (CastInst * CastI = dyn_cast<CastInst>(&I))         
                {
                    if (CastI->getOpcode()!=Instruction::Trunc && CastI->getOpcode()!=Instruction::ZExt && CastI->getOpcode()!=Instruction::SExt)
                    {
                        // Cast Instrctions are more than TRUNC/EXT
                        continue;
                    }
                    // If bitwidth(A)==bitwidth(B) in TRUNT/EXT A to B, then it is not necessary to do the instruction
                    if (CastI->getType()->getIntegerBitWidth() == I.getOperand(0)->getType()->getIntegerBitWidth())
                    {
                        if (DEBUG) *VarWidthChangeLog << "                         ------->remove redunctan CastI: " << *CastI  <<"\n";
                        if (DEBUG) *VarWidthChangeLog << "                         ------->replace CastI with its operand 0: " << *I.getOperand(0)  <<"\n";
                        //VarWidthChangeLog->flush(); 
                        ReplaceUses_withNewOperand_newBW(&I,I.getOperand(0));
                        I.eraseFromParent();
                        rmflag = 1;
                        changed = 1;
                        break;
                    }
                }            
            }   
        } 
    }
    return changed;
}


// Validation Check: Check whether there is any binary operation with operands in different types.
void HI_VarWidthReduce::VarWidthReduce_Validation(Function *F)
{
    // In some other passes in LLVM, if a insturction has operands in different types, errors could be generated
    for (BasicBlock &B : *F) 
    {
        bool take_action = 1;
     //   while(take_action)
        {
            take_action = 0;
            std::vector<Instruction*> integerInsts;
            for (Instruction &I: B) 
            {
                if (DEBUG) *VarWidthChangeLog << "checking Instruction width: " << I << " ";
                if (I.getType()->isIntegerTy())
                {
                    integerInsts.push_back(&I);
                }
            }
            for (Instruction *I: integerInsts) 
            {
                if (DEBUG) *VarWidthChangeLog << "checking Instruction width: " << I << " ";
                if (I->getType()->isIntegerTy())
                {
                    const SCEV *tmp_S = SE->getSCEV(I);
                    bool isUnsignedInst = I->getOpcode() == Instruction::UDiv || I->getOpcode() == Instruction::URem;
                    ConstantRange tmp_CR1 = isUnsignedInst ? SE->getUnsignedRange(tmp_S) : SE->getSignedRange(tmp_S);
                    
                    if (DEBUG) *VarWidthChangeLog << "CR-bw=" << tmp_CR1.getBitWidth() << " type-bw="<<I->getType()->getIntegerBitWidth() <<"\n";
                    if (tmp_CR1.getBitWidth() != I->getType()->getIntegerBitWidth())
                        if (DEBUG) *VarWidthChangeLog << "Bit width error!!!\n";
                    if (TruncInst *TI = dyn_cast<TruncInst>(I))
                    {
                        if (TI->getDestTy()->getIntegerBitWidth() > TI->getSrcTy()->getIntegerBitWidth())
                        {
                            if (DEBUG) *VarWidthChangeLog << "  should not be a truncI. correct it\n";
                                            Type *NewTy_OP = IntegerType::get(I->getType()->getContext(), Instruction_BitNeeded[I]);
                            ConstantRange tmp_CR = isUnsignedInst ? ConstantRange(APInt( HI_getUnsignedRangeRef(tmp_S).getBitWidth(),0), HI_getUnsignedRangeRef(tmp_S).getUpper()) : HI_getSignedRangeRef(tmp_S);

                            Value *ResultPtr;
                            IRBuilder<> Builder( TI->getNextNode());
                            std::string regNameS = "bcast"+std::to_string(changed_id);
                            changed_id++; 
                            if (I2NeedSign[I])
                            {                                                
                                ResultPtr = Builder.CreateSExtOrTrunc(TI->getOperand(0), TI->getDestTy(),regNameS.c_str()); // process the operand with SExtOrTrunc if it is signed.
                            }
                            else
                            {
                                ResultPtr = Builder.CreateZExtOrTrunc(TI->getOperand(0), TI->getDestTy(),regNameS.c_str()); // process the operand with ZExtOrTrunc if it is unsigned.                                           
                            }                            
                            // re-create the instruction to update the type(bitwidth) of it, otherwise, although the operans are changed, the output of instrcution will be remained.
                            if (DEBUG) *VarWidthChangeLog << "                         ------->  new_BOI = "<<*ResultPtr<<"\n";
                            ReplaceUses_withNewOperand_newBW(I, ResultPtr) ;
                            if (DEBUG) *VarWidthChangeLog << "                         ------->  accomplish replacement of original instruction in uses.\n";
                            //VarWidthChangeLog->flush(); 
                            I->eraseFromParent();
                            if (DEBUG) *VarWidthChangeLog << "                         ------->  accomplish erasing of original instruction.\n";
                            //VarWidthChangeLog->flush(); 
                            take_action = 1;
                           // break;
                        }
                        else if (TI->getDestTy()->getIntegerBitWidth() == TI->getSrcTy()->getIntegerBitWidth())
                        {
                            ReplaceUses_withNewOperand_newBW(I, TI->getOperand(0)) ;
                            if (DEBUG) *VarWidthChangeLog << "                         ------->  accomplish replacement of original instruction in uses.\n";
                            //VarWidthChangeLog->flush(); 
                            I->eraseFromParent();
                            if (DEBUG) *VarWidthChangeLog << "                         ------->  accomplish erasing of original instruction.\n";
                            take_action = 1;
                           // break;
                        }
                    } 
                    else if (ZExtInst *ZI = dyn_cast<ZExtInst>(I))
                    {
                        if (ZI->getDestTy()->getIntegerBitWidth() < ZI->getSrcTy()->getIntegerBitWidth())
                        {
                            if (DEBUG) *VarWidthChangeLog << "  should not be a truncI. correct it\n";
                                            Type *NewTy_OP = IntegerType::get(I->getType()->getContext(), Instruction_BitNeeded[I]);

                            
                            ConstantRange tmp_CR = isUnsignedInst ? ConstantRange(APInt( HI_getUnsignedRangeRef(tmp_S).getBitWidth(),0), HI_getUnsignedRangeRef(tmp_S).getUpper()): HI_getSignedRangeRef(tmp_S);

                            Value *ResultPtr;
                            IRBuilder<> Builder( ZI->getNextNode());
                            std::string regNameS = "bcast"+std::to_string(changed_id);
                            changed_id++; 
                            if (I2NeedSign[I])
                            {                                                
                                ResultPtr = Builder.CreateSExtOrTrunc(ZI->getOperand(0), ZI->getDestTy(),regNameS.c_str()); // process the operand with SExtOrTrunc if it is signed.
                            }
                            else
                            {
                                ResultPtr = Builder.CreateZExtOrTrunc(ZI->getOperand(0), ZI->getDestTy(),regNameS.c_str()); // process the operand with ZExtOrTrunc if it is unsigned.                                           
                            }                            
                            // re-create the instruction to update the type(bitwidth) of it, otherwise, although the operans are changed, the output of instrcution will be remained.
                            if (DEBUG) *VarWidthChangeLog << "                         ------->  new_BOI = "<<*ResultPtr<<"\n";
                            ReplaceUses_withNewOperand_newBW(I, ResultPtr) ;
                            if (DEBUG) *VarWidthChangeLog << "                         ------->  accomplish replacement of original instruction in uses.\n";
                            //VarWidthChangeLog->flush(); 
                            I->eraseFromParent();
                            if (DEBUG) *VarWidthChangeLog << "                         ------->  accomplish erasing of original instruction.\n";
                            //VarWidthChangeLog->flush(); 
                            take_action = 1;
                           // break;
                        }                        
                        else if (ZI->getDestTy()->getIntegerBitWidth() == ZI->getSrcTy()->getIntegerBitWidth())
                        {
                            ReplaceUses_withNewOperand_newBW(I, ZI->getOperand(0)) ;
                            if (DEBUG) *VarWidthChangeLog << "                         ------->  accomplish replacement of original instruction in uses.\n";
                            //VarWidthChangeLog->flush(); 
                            I->eraseFromParent();
                            if (DEBUG) *VarWidthChangeLog << "                         ------->  accomplish erasing of original instruction.\n";
                            take_action = 1;
                            //break;
                        }
                    }
                    else if (SExtInst *SI = dyn_cast<SExtInst>(I))
                    {
                        if (SI->getDestTy()->getIntegerBitWidth() < SI->getSrcTy()->getIntegerBitWidth())
                        {
                            if (DEBUG) *VarWidthChangeLog << "  should not be a truncI. correct it\n";
                                            Type *NewTy_OP = IntegerType::get(I->getType()->getContext(), Instruction_BitNeeded[I]);
                            ConstantRange tmp_CR = isUnsignedInst ? ConstantRange(APInt( HI_getUnsignedRangeRef(tmp_S).getBitWidth(),0), HI_getUnsignedRangeRef(tmp_S).getUpper()) : HI_getSignedRangeRef(tmp_S);

                            Value *ResultPtr;
                            IRBuilder<> Builder( SI->getNextNode());
                            std::string regNameS = "bcast"+std::to_string(changed_id);
                            changed_id++; 
                            if (I2NeedSign[I])
                            {                                                
                                ResultPtr = Builder.CreateSExtOrTrunc(SI->getOperand(0), SI->getDestTy(),regNameS.c_str()); // process the operand with SExtOrTrunc if it is signed.
                            }
                            else
                            {
                                ResultPtr = Builder.CreateZExtOrTrunc(SI->getOperand(0), SI->getDestTy(),regNameS.c_str()); // process the operand with ZExtOrTrunc if it is unsigned.                                           
                            }                            
                            // re-create the instruction to update the type(bitwidth) of it, otherwise, although the operans are changed, the output of instrcution will be remained.
                            if (DEBUG) *VarWidthChangeLog << "                         ------->  new_BOI = "<<*ResultPtr<<"\n";
                            ReplaceUses_withNewOperand_newBW(I, ResultPtr) ;
                            if (DEBUG) *VarWidthChangeLog << "                         ------->  accomplish replacement of original instruction in uses.\n";
                            //VarWidthChangeLog->flush(); 
                            I->eraseFromParent();
                            if (DEBUG) *VarWidthChangeLog << "                         ------->  accomplish erasing of original instruction.\n";
                            //VarWidthChangeLog->flush(); 
                            take_action = 1;
                           // break;
                        }                        
                        else if (SI->getDestTy()->getIntegerBitWidth() == SI->getSrcTy()->getIntegerBitWidth())
                        {
                            ReplaceUses_withNewOperand_newBW(I, SI->getOperand(0)) ;
                            if (DEBUG) *VarWidthChangeLog << "                         ------->  accomplish replacement of original instruction in uses.\n";
                            //VarWidthChangeLog->flush(); 
                            I->eraseFromParent();
                            if (DEBUG) *VarWidthChangeLog << "                         ------->  accomplish erasing of original instruction.\n";
                            take_action = 1;
                          //  break;
                        }
                    }
                }
                else
                {
                    if (DEBUG) *VarWidthChangeLog << "is not an integer type.\n ";
                }
            }
        }    
        if (DEBUG) *VarWidthChangeLog << "\n";
        //VarWidthChangeLog->flush(); 
    }

    if (DEBUG) *VarWidthChangeLog << "==============================================\n==============================================\n\n\n\n\n\n";
    for (BasicBlock &B : *F) 
    {
        if (DEBUG) *VarWidthChangeLog << B.getName() <<"\n";
        for (Instruction &I: B) 
        {
            if (DEBUG) *VarWidthChangeLog << "   " << I<<"\n";  
        }    
        if (DEBUG) *VarWidthChangeLog << "-------------------\n";
    }

    //VarWidthChangeLog->flush(); 
    return;
}


// The replaceAllUsesWith function requires that the new use in the user has the same type with the original use.
// Therefore, a new function to replace uses of an instruction is implemented
void HI_VarWidthReduce::ReplaceUses_withNewOperand_newBW(Instruction *from, Value *to) 
{
    if (DEBUG) *VarWidthChangeLog << "            ------  replacing  " << *from << " in its user\n";
    while (!from->use_empty()) 
    {
        User* tmp_user = from->use_begin()->getUser();
        if (DEBUG) *VarWidthChangeLog << "            ------  replacing the original inst in " << *from->use_begin()->getUser() << " with " << *to <<"\n";

        // we should not replace the argument with new argument with different BW, it may go wrong to match the definition of the function
        // therefore, we need to bitcast the value "to" to fit the bitwidth defined in the function definition
        if (auto Call_I = dyn_cast<CallInst>(tmp_user))
        {
            const SCEV *tmp_S = SE->getSCEV(to);

            bool isUnsignedInst = false; 
            if (auto InstI = dyn_cast<Instruction>(to))
            {
                isUnsignedInst = InstI->getOpcode() == Instruction::UDiv || InstI->getOpcode() == Instruction::URem;
            }
            ConstantRange tmp_CR = isUnsignedInst ? ConstantRange(APInt( HI_getUnsignedRangeRef(tmp_S).getBitWidth(),0), HI_getUnsignedRangeRef(tmp_S).getUpper()) : HI_getSignedRangeRef(tmp_S);

            Value *ResultPtr;
            IRBuilder<> Builder( Call_I);
            std::string regNameS = "bcast"+std::to_string(changed_id);
            changed_id++; 
            if (tmp_CR.getLower().isNegative())
            {                                                
                ResultPtr = Builder.CreateSExtOrTrunc(to, from->use_begin()->get()->getType(),regNameS.c_str()); // process the operand with SExtOrTrunc if it is signed.
            }
            else
            {
                ResultPtr = Builder.CreateZExtOrTrunc(to, from->use_begin()->get()->getType(),regNameS.c_str()); // process the operand with ZExtOrTrunc if it is unsigned.                                           
            }
            from->use_begin()->set(ResultPtr);      
        }
        else
        {
            from->use_begin()->set(to);
        }      
        
        if (DEBUG) *VarWidthChangeLog << "            ------  new user => " << *tmp_user << "\n";
        if (DEBUG) *VarWidthChangeLog << "            ------  from->getNumUses() "<< from->getNumUses() << "\n";
    }
}


// compute the bitwidth needed for the specific constant range
unsigned int HI_VarWidthReduce::bitNeededFor(ConstantRange CR)
{
    if (CR.isFullSet())
        return CR.getBitWidth();
    if (CR.getLower().isNonNegative())
    {
        // do no consider the leading zero, if the range is non-negative
        unsigned int lowerNeedBits = CR.getLower().getActiveBits();
        unsigned int upperNeedBits = CR.getUpper().getActiveBits();

        if (lowerNeedBits > upperNeedBits) 
            return lowerNeedBits;
        else
            return upperNeedBits;
    }
    else
    {
        // consider the leading zero/ones, if the range is negative
        unsigned int lowerNeedBits = CR.getLower().getMinSignedBits();
        unsigned int upperNeedBits = CR.getUpper().getMinSignedBits();

        if (lowerNeedBits > upperNeedBits) 
            return lowerNeedBits;
        else
            return upperNeedBits;
    }   
}


// Forward Process of BinaryOperator: check the bitwidth of operands and output of an instruction, trunc/ext the operands, update the bitwidth of the instruction
void HI_VarWidthReduce::BOI_WidthCast(BinaryOperator *BOI)
{
    Instruction &I = *(cast<Instruction>(BOI));
    

    Value *ResultPtr = &I;

    if (DEBUG) *VarWidthChangeLog << "and its operands are:\n";
    //VarWidthChangeLog->flush(); 

    // check whether an instruction involve PTI operation
    for (int i = 0; i < I.getNumOperands(); ++i)
    {
        if (PtrToIntInst *PTI_I = dyn_cast<PtrToIntInst>(I.getOperand(i))) 
        {
            // if this instruction involve operands from pointer, 
            // we meed to ensure the operands have the same width of the pointer
            Instruction_BitNeeded[&I] = (cast<IntegerType>(I.getType()))->getBitWidth();
        }
    }

    for (int i = 0; i < I.getNumOperands(); ++i) // check the operands to see whether a TRUNC/EXT is necessary
    {
        if (DEBUG) *VarWidthChangeLog << "                         ------->  op#"<<i <<"==>"<<*I.getOperand(i)<<"\n";
        //VarWidthChangeLog->flush(); 
        if (ConstantInt *C_I = dyn_cast<ConstantInt>(I.getOperand(i)))                                
        {
            if (DEBUG) *VarWidthChangeLog << "                         ------->  op#"<<i<<" "<<*C_I<<" is a constant.\n";
            //VarWidthChangeLog->flush(); 
            Type *NewTy_C = IntegerType::get(I.getType()->getContext(), Instruction_BitNeeded[&I]);
            Constant *New_C = ConstantInt::get(NewTy_C,*C_I->getValue().getRawData());
            if (DEBUG) *VarWidthChangeLog << "                         ------->  update"<<I<<" to ";
            //VarWidthChangeLog->flush(); 
            I.setOperand(i,New_C);
            if (DEBUG) *VarWidthChangeLog <<I<<"\n";
        }
        else
        {
            if (Instruction *Op_I = dyn_cast<Instruction>(I.getOperand(i)))
            {
                if (DEBUG) *VarWidthChangeLog << "                         ------->  op#"<<i<<" "<<*Op_I<<" is an instruction\n";
                //VarWidthChangeLog->flush(); 
                Instruction *InsertPoint = nullptr;
                if (Op_I->getOpcode() == Instruction::PHI)
                    InsertPoint = Op_I->getParent()->getFirstNonPHI();
                else
                    InsertPoint = Op_I->getNextNode();
                IRBuilder<> Builder(InsertPoint);

                std::string regNameS = "bcast"+std::to_string(changed_id);
                changed_id++; 
                // create a net type with specific bitwidth                  
                Type *NewTy_OP = IntegerType::get(I.getType()->getContext(), Instruction_BitNeeded[&I]);
                if (I2NeedSign[&I])
                {                                                
                    ResultPtr = Builder.CreateSExtOrTrunc(Op_I, NewTy_OP,regNameS.c_str()); // process the operand with SExtOrTrunc if it is signed.
                }
                else
                {
                    ResultPtr = Builder.CreateZExtOrTrunc(Op_I, NewTy_OP,regNameS.c_str()); // process the operand with ZExtOrTrunc if it is unsigned.                                           
                }
                if (DEBUG) *VarWidthChangeLog << "                         ------->  update"<<I<<" to ";
                //VarWidthChangeLog->flush(); 
                I.setOperand(i,ResultPtr);
                if (DEBUG) *VarWidthChangeLog <<I<<"\n";
            }

        }
        //VarWidthChangeLog->flush();                                 
    }
    //VarWidthChangeLog->flush(); 
    if (DEBUG) *VarWidthChangeLog << "                         ------->  op0 type = "<<*BOI->getOperand(0)->getType()<<"\n";
    if (DEBUG) *VarWidthChangeLog << "                         ------->  op1 type = "<<*BOI->getOperand(1)->getType()<<"\n";
    
    // re-create the instruction to update the type(bitwidth) of it, otherwise, although the operans are changed, the output of instrcution will be remained.
    std::string regNameS = "new"+std::to_string(changed_id);
    BinaryOperator *newBOI = BinaryOperator::Create(BOI->getOpcode(), BOI->getOperand(0), BOI->getOperand(1), "HI."+BOI->getName()+regNameS,BOI); 
    if (DEBUG) *VarWidthChangeLog << "                         ------->  new_BOI = "<<*newBOI<<"\n";
    // BOI->replaceAllUsesWith(newBOI) ;
    ReplaceUses_withNewOperand_newBW(BOI, newBOI) ;
    if (DEBUG) *VarWidthChangeLog << "                         ------->  accomplish replacement of original instruction in uses.\n";
    //VarWidthChangeLog->flush(); 
    I.eraseFromParent();
    if (DEBUG) *VarWidthChangeLog << "                         ------->  accomplish erasing of original instruction.\n";
    //VarWidthChangeLog->flush(); 
}



// Forward Process of ICmpInst: check the bitwidth of operands and output of an instruction, trunc/ext the operands, update the bitwidth of the instruction
void HI_VarWidthReduce::ICMP_WidthCast(ICmpInst *ICMP_I)
{
    Instruction &I = *(cast<Instruction>(ICMP_I));


    Value *ResultPtr = &I;

    if (DEBUG) *VarWidthChangeLog << "and its operands are:\n";
    //VarWidthChangeLog->flush(); 

    // check whether an instruction involve PTI operation
    for (int i = 0; i < I.getNumOperands(); ++i)
    {
        if (PtrToIntInst *PTI_I = dyn_cast<PtrToIntInst>(I.getOperand(i))) 
        {
            // if this instruction involve operands from pointer, 
            // we meed to ensure the operands have the same width of the pointer
            Instruction_BitNeeded[&I] = (cast<IntegerType>(I.getType()))->getBitWidth();
        }
    }

    for (int i = 0; i < I.getNumOperands(); ++i) // check the operands to see whether a TRUNC/EXT is necessary
    {
        if (DEBUG) *VarWidthChangeLog << "                         ------->  op#"<<i <<"==>"<<*I.getOperand(i)<<"\n";
        //VarWidthChangeLog->flush(); 
        if (ConstantInt *C_I = dyn_cast<ConstantInt>(I.getOperand(i)))                                
        {
            if (DEBUG) *VarWidthChangeLog << "                         ------->  op#"<<i<<" "<<*C_I<<" is a constant.\n";
            //VarWidthChangeLog->flush(); 
            //  if (C_I->getBitWidth()!= Instruction_BitNeeded[User_I])
            Type *NewTy_C = IntegerType::get(I.getType()->getContext(), Instruction_BitNeeded[&I]);
            Constant *New_C = ConstantInt::get(NewTy_C,*C_I->getValue().getRawData());
            if (DEBUG) *VarWidthChangeLog << "                         ------->  update"<<I<<" to ";
            //VarWidthChangeLog->flush(); 
            I.setOperand(i,New_C);
            if (DEBUG) *VarWidthChangeLog <<I<<"\n";
        }
        else
        {
            if (Instruction *Op_I = dyn_cast<Instruction>(I.getOperand(i)))
            {
                if (DEBUG) *VarWidthChangeLog << "                         ------->  op#"<<i<<" "<<*Op_I<<" is an instruction\n";
                //VarWidthChangeLog->flush(); 
                Instruction *InsertPoint = nullptr;
                if (Op_I->getOpcode() == Instruction::PHI)
                    InsertPoint = Op_I->getParent()->getFirstNonPHI();
                else
                    InsertPoint = Op_I->getNextNode();
                IRBuilder<> Builder(InsertPoint);
                std::string regNameS = "bcast"+std::to_string(changed_id);
                changed_id++;                   

                // create a net type with specific bitwidth
                Type *NewTy_OP = IntegerType::get(I.getType()->getContext(), Instruction_BitNeeded[&I]);
                if (I2NeedSign[&I])
                {                                                
                    ResultPtr = Builder.CreateSExtOrTrunc(Op_I, NewTy_OP,regNameS.c_str());// process the operand with SExtOrTrunc if it is signed.
                }
                else
                {
                    ResultPtr = Builder.CreateZExtOrTrunc(Op_I, NewTy_OP,regNameS.c_str());// process the operand with ZExtOrTrunc if it is unsigned.                                            
                }
                if (DEBUG) *VarWidthChangeLog << "                         ------->  update"<<I<<" to ";
                //VarWidthChangeLog->flush(); 
                I.setOperand(i,ResultPtr);
                if (DEBUG) *VarWidthChangeLog <<I<<"\n";
            }

        }
        //VarWidthChangeLog->flush();                                 
    }
    //VarWidthChangeLog->flush(); 
    if (DEBUG) *VarWidthChangeLog << "                         ------->  op0 type = "<<*ICMP_I->getOperand(0)->getType()<<"\n";
    if (DEBUG) *VarWidthChangeLog << "                         ------->  op1 type = "<<*ICMP_I->getOperand(1)->getType()<<"\n";
    
    // re-create the instruction to update the type(bitwidth) of it, otherwise, although the operans are changed, the output of instrcution will be remained.
    std::string regNameS = "new"+std::to_string(changed_id);
    ICmpInst *newCMP = new ICmpInst(
        ICMP_I,  ///< Where to insert
        ICMP_I->getPredicate(),  ///< The predicate to use for the comparison
        ICMP_I->getOperand(0),      ///< The left-hand-side of the expression
        ICMP_I->getOperand(1),      ///< The right-hand-side of the expression
        "HI."+ICMP_I->getName()+regNameS  ///< Name of the instruction
    ); 
    if (DEBUG) *VarWidthChangeLog << "                         ------->  new_CMP = "<<*newCMP<<"\n";
    // BOI->replaceAllUsesWith(newBOI) ;
    ReplaceUses_withNewOperand_newBW(ICMP_I, newCMP) ;
    if (DEBUG) *VarWidthChangeLog << "                         ------->  accomplish replacement of original instruction in uses.\n";
    //VarWidthChangeLog->flush(); 
    I.eraseFromParent();
    if (DEBUG) *VarWidthChangeLog << "                         ------->  accomplish erasing of original instruction.\n";
    //VarWidthChangeLog->flush(); 
}


// Forward Process of PHI: check the bitwidth of operands and output of an instruction, trunc/ext the operands, update the bitwidth of the instruction
void HI_VarWidthReduce::PHI_WidthCast(llvm::PHINode *PHI_I)
{
    Instruction &I = *(cast<Instruction>(PHI_I));


    Value *ResultPtr = &I;
    if (DEBUG) *VarWidthChangeLog << "and its operands are:\n";
    //VarWidthChangeLog->flush(); 

    // check whether an instruction involve PTI operation
    for (int i = 0; i < I.getNumOperands(); ++i)
    {
        if (PtrToIntInst *PTI_I = dyn_cast<PtrToIntInst>(I.getOperand(i))) 
        {
            // if this instruction involve operands from pointer, 
            // we meed to ensure the operands have the same width of the pointer
            Instruction_BitNeeded[&I] = (cast<IntegerType>(I.getType()))->getBitWidth();
        }
    }

    for (int i = 0; i < I.getNumOperands(); ++i) // check the operands to see whether a TRUNC/EXT is necessary
    {
        if (DEBUG) *VarWidthChangeLog << "                         ------->  op#"<<i <<"==>"<<*I.getOperand(i)<<"\n";
        //VarWidthChangeLog->flush(); 
        if (ConstantInt *C_I = dyn_cast<ConstantInt>(I.getOperand(i)))                                
        {
            if (DEBUG) *VarWidthChangeLog << "                         ------->  op#"<<i<<" "<<*C_I<<" is a constant.\n";
            //VarWidthChangeLog->flush(); 
            //  if (C_I->getBitWidth()!= Instruction_BitNeeded[User_I])
            Type *NewTy_C = IntegerType::get(I.getType()->getContext(), Instruction_BitNeeded[&I]);
            Constant *New_C = ConstantInt::get(NewTy_C,*C_I->getValue().getRawData());
            if (DEBUG) *VarWidthChangeLog << "                         ------->  update"<<I<<" to ";
            //VarWidthChangeLog->flush(); 
            I.setOperand(i,New_C);
            if (DEBUG) *VarWidthChangeLog <<I<<"\n";
        }
        else
        {
            if (Instruction *Op_I = dyn_cast<Instruction>(I.getOperand(i)))
            {
                if (DEBUG) *VarWidthChangeLog << "                         ------->  op#"<<i<<" "<<*Op_I<<" is an instruction\n";
                //VarWidthChangeLog->flush(); 
                Instruction *InsertPoint = nullptr;
                if (Op_I->getOpcode() == Instruction::PHI)
                    InsertPoint = Op_I->getParent()->getFirstNonPHI();
                else
                    InsertPoint = Op_I->getNextNode();
                IRBuilder<> Builder(InsertPoint);
                std::string regNameS = "bcast"+std::to_string(changed_id);
                changed_id++;       
                // create a net type with specific bitwidth            
                Type *NewTy_OP = IntegerType::get(I.getType()->getContext(), Instruction_BitNeeded[&I]);
                if (I2NeedSign[&I])
                {                                                
                    ResultPtr = Builder.CreateSExtOrTrunc(Op_I, NewTy_OP,regNameS.c_str());// process the operand with SExtOrTrunc if it is signed.
                }
                else
                {
                    ResultPtr = Builder.CreateZExtOrTrunc(Op_I, NewTy_OP,regNameS.c_str()); // process the operand with ZExtOrTrunc if it is unsigned.                                            
                }
                if (DEBUG) *VarWidthChangeLog << "                         ------->  update"<<I<<" to ";
                //VarWidthChangeLog->flush(); 
                I.setOperand(i,ResultPtr);
                if (DEBUG) *VarWidthChangeLog <<I<<"\n";
            }

        }
        //VarWidthChangeLog->flush();                                 
    }
    //VarWidthChangeLog->flush(); 
    if (DEBUG) *VarWidthChangeLog << "                         ------->  op0 type = "<<*PHI_I->getOperand(0)->getType()<<"\n";
    if (DEBUG) *VarWidthChangeLog << "                         ------->  op1 type = "<<*PHI_I->getOperand(1)->getType()<<"\n";
    Type *NewTy_PHI = IntegerType::get(I.getType()->getContext(), Instruction_BitNeeded[&I]);
    
    
    // re-create the instruction to update the type(bitwidth) of it, otherwise, although the operans are changed, the output of instrcution will be remained.
    std::string regNameS = "new"+std::to_string(changed_id);
    PHINode *new_PHI = PHINode::Create(NewTy_PHI, 0, "HI."+PHI_I->getName()+regNameS,PHI_I);
    if (new_PHI->getName().find("HI.k.016new1")!=std::string::npos)
    {
        if (new_PHI->getType()->isIntOrIntVectorTy())
        {
            llvm::errs() << "isIntOrIntVectorTy\n";
        }
        if (new_PHI->getType()->isIntegerTy())
        {
            llvm::errs() << "isIntegerTy\n";
        }
    }

    for (int i = 0; i < I.getNumOperands(); ++i)
    {
        new_PHI->addIncoming(PHI_I->getIncomingValue(i),PHI_I->getIncomingBlock(i));
    }
    if (DEBUG) *VarWidthChangeLog << "                         ------->  new_PHI_I = "<<*new_PHI<<"\n";
    
    // BOI->replaceAllUsesWith(newBOI) ;
    ReplaceUses_withNewOperand_newBW(PHI_I, new_PHI) ;
    if (DEBUG) *VarWidthChangeLog << "                         ------->  accomplish replacement of original instruction in uses.\n";
    //VarWidthChangeLog->flush(); 
    I.eraseFromParent();
    if (DEBUG) *VarWidthChangeLog << "                         ------->  accomplish erasing of original instruction.\n";
    //VarWidthChangeLog->flush(); 
}

// Determine the range for a particular SCEV, but bypass the operands generated from PtrToInt Instruction, considering the actual 
// implementation in HLS
const ConstantRange HI_VarWidthReduce::HI_getSignedRangeRef(const SCEV *S) 
{

    if (DEBUG) *VarWidthChangeLog << "        ------  HI_getSignedRangeRef handling SECV: " << *S->getType() << "\n";
    ConstantRange tmp_CR1 = SE->getSignedRange(S);
    if (!tmp_CR1.isFullSet())
    {
        if (DEBUG) *VarWidthChangeLog << "        ------  HI_getSignedRangeRef: it is not full-set " << tmp_CR1 << "\n";
        return tmp_CR1;
    }
    DenseMap<const SCEV *, ConstantRange> &Cache = SignedRanges;
    if (DEBUG) *VarWidthChangeLog << "        ------  handling full-set SECV: " << *S->getType() << "\n";//  << "--> " << *S << "\n";
    VarWidthChangeLog->flush();
    
    if (const SCEVUnknown *U = dyn_cast<SCEVUnknown>(S)) 
    {
        
        if (LoadInst *LI = dyn_cast<LoadInst>(U->getValue()))
        {   
            if (DEBUG) *VarWidthChangeLog << "        ------  SCEVUnknown Load\n";
            VarWidthChangeLog->flush();
          //  return setSignedRange(S, std::move(SE->getSignedRange(U)));
        }        
    }

    // See if we've computed this range already.
    DenseMap<const SCEV *, ConstantRange>::iterator I = Cache.find(S);
    if (I != Cache.end())
        return I->second;

    if (const SCEVConstant *C = dyn_cast<SCEVConstant>(S))
        return setSignedRange(C, ConstantRange(C->getAPInt()));

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

        // if (DEBUG) *VarWidthChangeLog << "        ------  Add:" << *Add << "\n"; 

        ConstantRange X = HI_getSignedRangeRef(Add->getOperand(0));
        const SCEVConstant *tmp_const = dyn_cast<SCEVConstant>(Add->getOperand(0));
        const SCEVUnknown *tmp_unknow = dyn_cast<SCEVUnknown>(Add->getOperand(0));
        // if (DEBUG) *VarWidthChangeLog << "        ------  Add operand#" << 0 /* << " ==> " << *Add->getOperand(0)*/  << "\n";
        if (tmp_const)
            if (DEBUG) *VarWidthChangeLog << "        ------  bitwidth: " << tmp_const->getAPInt().getBitWidth() << "\n";
        else if (tmp_unknow)
            if (DEBUG) *VarWidthChangeLog << "        ------  bitwidth: " << tmp_unknow->getValue()->getType()->getIntegerBitWidth() << "\n";

        if (DEBUG) *VarWidthChangeLog << "        ------  range bitwidth: " << X.getBitWidth() << "\n";
        VarWidthChangeLog->flush();

        for (unsigned i = 1, e = Add->getNumOperands(); i != e; ++i)
            if (bypassPTI(Add->getOperand(i))) 
                continue;
            else
            {
                if (DEBUG) *VarWidthChangeLog << "        ------  Add operand#" << i /* << " ==> " << *Add->getOperand(i)*/  << "\n"; 
                const SCEVConstant *tmp_const = dyn_cast<SCEVConstant>(Add->getOperand(i));
                const SCEVUnknown *tmp_unknow = dyn_cast<SCEVUnknown>(Add->getOperand(i));
                if (tmp_const)
                    if (DEBUG) *VarWidthChangeLog << "        ------  bitwidth: " << tmp_const->getAPInt().getBitWidth() << "\n";
                else if (tmp_unknow)
                    if (DEBUG) *VarWidthChangeLog << "        ------  bitwidth: " << tmp_unknow->getValue()->getType()->getIntegerBitWidth() << "\n";
        
                ConstantRange Y = HI_getSignedRangeRef(Add->getOperand(i));
                if (DEBUG) *VarWidthChangeLog << "        ------  range bitwidth: " << Y.getBitWidth() << "\n";
                VarWidthChangeLog->flush();
                X = X.add(Y);
            }
        if (DEBUG) *VarWidthChangeLog << "            ------  handling full-set SECV new range: " << X << "\n";
        return setSignedRange(Add, ConservativeResult.intersectWith(X));
    }

    if (const SCEVMulExpr *Mul = dyn_cast<SCEVMulExpr>(S)) 
    {
        // if (DEBUG) *VarWidthChangeLog << "        ------  Mul:" << *Mul << "\n";
        ConstantRange X = HI_getSignedRangeRef(Mul->getOperand(0));
        for (unsigned i = 1, e = Mul->getNumOperands(); i != e; ++i)
            if (bypassPTI(Mul->getOperand(i))) 
                continue; 
            else
                X = X.multiply(HI_getSignedRangeRef(Mul->getOperand(i)));
          if (DEBUG) *VarWidthChangeLog << "            ------  handling full-set SECV new range: " << X << "\n";
        return setSignedRange(Mul, ConservativeResult.intersectWith(X));
    }

    if (const SCEVSMaxExpr *SMax = dyn_cast<SCEVSMaxExpr>(S)) 
    {
        if (DEBUG) *VarWidthChangeLog << "        ------  SMax\n";
        ConstantRange X = HI_getSignedRangeRef(SMax->getOperand(0));
        for (unsigned i = 1, e = SMax->getNumOperands(); i != e; ++i)
            if (bypassPTI(SMax->getOperand(i))) 
                continue;
            else
                X = X.smax(HI_getSignedRangeRef(SMax->getOperand(i)));
        if (DEBUG) *VarWidthChangeLog << "          ------  handling full-set SECV new range: " << X << "\n";
        return setSignedRange(SMax, ConservativeResult.intersectWith(X));
    }

    if (const SCEVUMaxExpr *UMax = dyn_cast<SCEVUMaxExpr>(S)) 
    {
        if (DEBUG) *VarWidthChangeLog << "        ------  UMax\n";
        ConstantRange X = HI_getSignedRangeRef(UMax->getOperand(0));
        for (unsigned i = 1, e = UMax->getNumOperands(); i != e; ++i)
            if (bypassPTI(UMax->getOperand(i))) 
                continue;
            else
                X = X.umax(HI_getSignedRangeRef(UMax->getOperand(i)));
        if (DEBUG) *VarWidthChangeLog << "          ------  handling full-set SECV new range: " << X << "\n";
        return setSignedRange(UMax, ConservativeResult.intersectWith(X));
    }

    if (const SCEVUDivExpr *UDiv = dyn_cast<SCEVUDivExpr>(S)) 
    {
        if (DEBUG) *VarWidthChangeLog << "        ------  UDiv\n";
        if (bypassPTI(UDiv->getLHS())) return ConservativeResult;
        if (bypassPTI(UDiv->getRHS())) return ConservativeResult;
        ConstantRange X = HI_getSignedRangeRef(UDiv->getLHS());
        ConstantRange Y = HI_getSignedRangeRef(UDiv->getRHS());
        return setSignedRange(UDiv,
                        ConservativeResult.intersectWith(X.udiv(Y)));
    }

    if (const SCEVZeroExtendExpr *ZExt = dyn_cast<SCEVZeroExtendExpr>(S)) 
    {
        if (DEBUG) *VarWidthChangeLog << "        ------  ZExt\n";
        if (bypassPTI(ZExt->getOperand())) return ConservativeResult;
        ConstantRange X = HI_getSignedRangeRef(ZExt->getOperand());
        return setSignedRange(ZExt,
                        ConservativeResult.intersectWith(X.zeroExtend(BitWidth)));
    }

    if (const SCEVSignExtendExpr *SExt = dyn_cast<SCEVSignExtendExpr>(S)) 
    {
        if (DEBUG) *VarWidthChangeLog << "        ------  SExt\n";
        if (bypassPTI(SExt->getOperand())) return ConservativeResult;
        ConstantRange X = HI_getSignedRangeRef(SExt->getOperand());
        return setSignedRange(SExt,
                        ConservativeResult.intersectWith(X.signExtend(BitWidth)));
    }

    if (const SCEVTruncateExpr *Trunc = dyn_cast<SCEVTruncateExpr>(S)) 
    {
        if (DEBUG) *VarWidthChangeLog << "        ------  Trunc\n";
        if (bypassPTI(Trunc->getOperand())) return ConservativeResult;
        ConstantRange X = HI_getSignedRangeRef(Trunc->getOperand());
        return setSignedRange(Trunc,
                        ConservativeResult.intersectWith(X.truncate(BitWidth)));
    }

    if (const SCEVAddRecExpr *AddRec = dyn_cast<SCEVAddRecExpr>(S)) 
    {
        if (DEBUG) *VarWidthChangeLog << "        ------  SCEVAddRecExpr\n";
        return setSignedRange(S, std::move(SE->getSignedRange(AddRec)));
    }

    if (const SCEVUnknown *U = dyn_cast<SCEVUnknown>(S)) 
    {
        if (DEBUG) *VarWidthChangeLog << "        ------  SCEVUnknown\n";
        if (PtrToIntInst *PTI = dyn_cast<PtrToIntInst>(U->getValue()))
        {   

        }
        //return setSignedRange(S, std::move(SE->getSignedRange(U)));
    }
    if (DEBUG) *VarWidthChangeLog << "        ------  Out of Scope\n";
    return setSignedRange(S, std::move(ConservativeResult));
}

// cache constant range for those evaluated SCEVs
const ConstantRange &HI_VarWidthReduce::setSignedRange(const SCEV *S,  ConstantRange CR) 
{
    DenseMap<const SCEV *, ConstantRange> &Cache = SignedRanges;

    auto Pair = Cache.try_emplace(S, std::move(CR));
    if (!Pair.second)
        Pair.first->second = std::move(CR);
    return Pair.first->second;
}



const ConstantRange HI_VarWidthReduce::HI_getUnsignedRangeRef(const SCEV *S) 
{

    if (DEBUG) *VarWidthChangeLog << "        ------  HI_getUnsignedRangeRef handling SECV: " << *S->getType() << "\n";
    ConstantRange tmp_CR1 = SE->getUnsignedRange(S);
    if (!tmp_CR1.isFullSet())
    {
        if (DEBUG) *VarWidthChangeLog << "        ------  HI_getUnsignedRangeRef: it is not full-set " << tmp_CR1 << "\n";
        return tmp_CR1;
    }
    DenseMap<const SCEV *, ConstantRange> &Cache = SignedRanges;
    if (DEBUG) *VarWidthChangeLog << "        ------  handling full-set SECV: " << *S->getType() << "\n";

    
    if (const SCEVUnknown *U = dyn_cast<SCEVUnknown>(S)) 
    {
        
        if (LoadInst *LI = dyn_cast<LoadInst>(U->getValue()))
        {   
            if (DEBUG) *VarWidthChangeLog << "        ------  SCEVUnknown Load\n";
            VarWidthChangeLog->flush();
           // return setSignedRange(S, std::move(SE->getUnsignedRange(U)));
        }        
    }

    // See if we've computed this range already.
    DenseMap<const SCEV *, ConstantRange>::iterator I = Cache.find(S);
    if (I != Cache.end())
        return I->second;

    if (const SCEVConstant *C = dyn_cast<SCEVConstant>(S))
        return setUnsignedRange(C, ConstantRange(C->getAPInt()));

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
        if (DEBUG) *VarWidthChangeLog << "        ------  Add\n";
        ConstantRange X = HI_getUnsignedRangeRef(Add->getOperand(0));
        for (unsigned i = 1, e = Add->getNumOperands(); i != e; ++i)
            if (bypassPTI(Add->getOperand(i))) 
                continue;
            else
                X = X.add(HI_getUnsignedRangeRef(Add->getOperand(i)));
        if (DEBUG) *VarWidthChangeLog << "            ------  handling full-set SECV new range: " << X << "\n";
        return setUnsignedRange(Add, ConservativeResult.intersectWith(X));
    }

    if (const SCEVMulExpr *Mul = dyn_cast<SCEVMulExpr>(S)) 
    {
        if (DEBUG) *VarWidthChangeLog << "        ------  Mul\n";
        ConstantRange X = HI_getUnsignedRangeRef(Mul->getOperand(0));
        for (unsigned i = 1, e = Mul->getNumOperands(); i != e; ++i)
            if (bypassPTI(Mul->getOperand(i))) 
                continue; 
            else
                X = X.multiply(HI_getUnsignedRangeRef(Mul->getOperand(i)));
          if (DEBUG) *VarWidthChangeLog << "            ------  handling full-set SECV new range: " << X << "\n";
        return setUnsignedRange(Mul, ConservativeResult.intersectWith(X));
    }

    if (const SCEVSMaxExpr *SMax = dyn_cast<SCEVSMaxExpr>(S)) 
    {
        if (DEBUG) *VarWidthChangeLog << "        ------  SMax\n";
        ConstantRange X = HI_getUnsignedRangeRef(SMax->getOperand(0));
        for (unsigned i = 1, e = SMax->getNumOperands(); i != e; ++i)
            if (bypassPTI(SMax->getOperand(i))) 
                continue;
            else
                X = X.smax(HI_getUnsignedRangeRef(SMax->getOperand(i)));
        if (DEBUG) *VarWidthChangeLog << "          ------  handling full-set SECV new range: " << X << "\n";
        return setUnsignedRange(SMax, ConservativeResult.intersectWith(X));
    }

    if (const SCEVUMaxExpr *UMax = dyn_cast<SCEVUMaxExpr>(S)) 
    {
        if (DEBUG) *VarWidthChangeLog << "        ------  UMax\n";
        ConstantRange X = HI_getUnsignedRangeRef(UMax->getOperand(0));
        for (unsigned i = 1, e = UMax->getNumOperands(); i != e; ++i)
            if (bypassPTI(UMax->getOperand(i))) 
                continue;
            else
                X = X.umax(HI_getUnsignedRangeRef(UMax->getOperand(i)));
        if (DEBUG) *VarWidthChangeLog << "          ------  handling full-set SECV new range: " << X << "\n";
        return setUnsignedRange(UMax, ConservativeResult.intersectWith(X));
    }

    if (const SCEVUDivExpr *UDiv = dyn_cast<SCEVUDivExpr>(S)) 
    {
        if (DEBUG) *VarWidthChangeLog << "        ------  UDiv\n";
        if (bypassPTI(UDiv->getLHS())) return ConservativeResult;
        if (bypassPTI(UDiv->getRHS())) return ConservativeResult;
        ConstantRange X = HI_getUnsignedRangeRef(UDiv->getLHS());
        ConstantRange Y = HI_getUnsignedRangeRef(UDiv->getRHS());
        return setUnsignedRange(UDiv,
                        ConservativeResult.intersectWith(X.udiv(Y)));
    }

    if (const SCEVZeroExtendExpr *ZExt = dyn_cast<SCEVZeroExtendExpr>(S)) 
    {
        if (DEBUG) *VarWidthChangeLog << "        ------  ZExt\n";
        if (bypassPTI(ZExt->getOperand())) return ConservativeResult;
        ConstantRange X = HI_getUnsignedRangeRef(ZExt->getOperand());
        return setUnsignedRange(ZExt,
                        ConservativeResult.intersectWith(X.zeroExtend(BitWidth)));
    }

    if (const SCEVSignExtendExpr *SExt = dyn_cast<SCEVSignExtendExpr>(S)) 
    {
        if (DEBUG) *VarWidthChangeLog << "        ------  SExt\n";
        if (bypassPTI(SExt->getOperand())) return ConservativeResult;
        ConstantRange X = HI_getUnsignedRangeRef(SExt->getOperand());
        return setUnsignedRange(SExt,
                        ConservativeResult.intersectWith(X.signExtend(BitWidth)));
    }

    if (const SCEVTruncateExpr *Trunc = dyn_cast<SCEVTruncateExpr>(S)) 
    {
        if (DEBUG) *VarWidthChangeLog << "        ------  Trunc\n";
        if (bypassPTI(Trunc->getOperand())) return ConservativeResult;
        ConstantRange X = HI_getUnsignedRangeRef(Trunc->getOperand());
        return setUnsignedRange(Trunc,
                        ConservativeResult.intersectWith(X.truncate(BitWidth)));
    }

    if (const SCEVAddRecExpr *AddRec = dyn_cast<SCEVAddRecExpr>(S)) 
    {
        if (DEBUG) *VarWidthChangeLog << "        ------  SCEVAddRecExpr\n";
        return setUnsignedRange(S, std::move(SE->getUnsignedRange(AddRec)));
    }

    if (const SCEVUnknown *U = dyn_cast<SCEVUnknown>(S)) 
    {
        if (DEBUG) *VarWidthChangeLog << "        ------  SCEVUnknown\n";
        if (PtrToIntInst *PTI = dyn_cast<PtrToIntInst>(U->getValue()))
        {   

        }
        //return setUnsignedRange(S, std::move(SE->getUnsignedRange(U)));
    }
    if (DEBUG) *VarWidthChangeLog << "        ------  Out of Scope\n";
    return setUnsignedRange(S, std::move(ConservativeResult));
}

// cache constant range for those evaluated SCEVs
const ConstantRange &HI_VarWidthReduce::setUnsignedRange(const SCEV *S,  ConstantRange CR) 
{
    DenseMap<const SCEV *, ConstantRange> &Cache = UnsignedRanges;

    auto Pair = Cache.try_emplace(S, std::move(CR));
    if (!Pair.second)
        Pair.first->second = std::move(CR);
    return Pair.first->second;
}


// check whether we should bypass the PtrToInt Instruction
bool HI_VarWidthReduce::bypassPTI(const SCEV *S)
{
    if (const SCEVUnknown *U = dyn_cast<SCEVUnknown>(S))
        if (PtrToIntInst *PTI = dyn_cast<PtrToIntInst>(U->getValue()))
        {
            if (DEBUG) *VarWidthChangeLog << "            ------  bypassing range evaluation for PtrToIntInst: " << *U->getValue() << "\n";
            return true;
        }
    return false;
}


