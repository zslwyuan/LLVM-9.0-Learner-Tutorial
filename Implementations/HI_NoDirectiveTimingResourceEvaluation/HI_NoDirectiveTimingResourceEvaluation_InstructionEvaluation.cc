#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_print.h"
#include "HI_NoDirectiveTimingResourceEvaluation.h"
#include "polly/PolyhedralInfo.h"

#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>

using namespace llvm;



/*
    mainly used to get the latency of an instruction 
*/
HI_NoDirectiveTimingResourceEvaluation::timingBase HI_NoDirectiveTimingResourceEvaluation::getInstructionLatency(Instruction *I)
{
    timingBase result(0,0,1,clock_period);

    ////////////////////////////// Cast Operations /////////////////////////
    if (PtrToIntInst *PTI = dyn_cast<PtrToIntInst>(I)) // such operation like trunc/ext will not cost extra timing on FPGA
    {   
        return result;
    }
    else if (IntToPtrInst *ITP = dyn_cast<IntToPtrInst>(I))
    {   
        return result;
    }
    else if (FPToUIInst *FTU = dyn_cast<FPToUIInst>(I))
    {
        result = get_inst_TimingInfo_result("fptoui",FTU->getType()->getIntegerBitWidth(),FTU->getType()->getIntegerBitWidth(),clock_period_str);
        return result;
    }
    else if (FPToSIInst *FTS = dyn_cast<FPToSIInst>(I))
    {
        result = get_inst_TimingInfo_result("fptosi",FTS->getType()->getIntegerBitWidth(),FTS->getType()->getIntegerBitWidth(),clock_period_str);
        return result;
    }
    else if (UIToFPInst *UTF = dyn_cast<UIToFPInst>(I))
    {
        Value *op1 = UTF->getOperand(1);
        result = get_inst_TimingInfo_result("uitofp",op1->getType()->getIntegerBitWidth(),op1->getType()->getIntegerBitWidth(),clock_period_str);
        return result;
    }
    else if (SIToFPInst *STF = dyn_cast<SIToFPInst>(I))
    {
        Value *op1 = STF->getOperand(1);
        result = get_inst_TimingInfo_result("sitofp",op1->getType()->getIntegerBitWidth(),op1->getType()->getIntegerBitWidth(),clock_period_str);
        return result;
    }
    else if (ZExtInst *ZEXTI = dyn_cast<ZExtInst>(I))  // such operation like trunc/ext will not cost extra timing on FPGA
    {   
        return result;
    }
    else if (SExtInst *SEXTI = dyn_cast<SExtInst>(I))
    {   
        return result;
    }
    else if (TruncInst *TI = dyn_cast<TruncInst>(I))
    {   
        return result;
    }
    else if (BitCastInst *BI = dyn_cast<BitCastInst>(I))
    {   
        return result;
    }

    ////////////////////////////// Binary Operations /////////////////////////
    else if (ShlOperator *SHLI = dyn_cast<ShlOperator>(I))
    {   
        Value *op1 = SHLI->getOperand(1);
         
        if (Constant *tmpop = dyn_cast<Constant>(op1))
            return result;
            {
                result = get_inst_TimingInfo_result("shl",op1->getType()->getIntegerBitWidth(),SHLI->getType()->getIntegerBitWidth(),clock_period_str);
                return result;
            }
    }
    else if (LShrOperator *LSHRI = dyn_cast<LShrOperator>(I))
    {   
        Value *op1 = LSHRI->getOperand(1);
         
        if (Constant *tmpop = dyn_cast<Constant>(op1))
            return result;
        else
        {
            result = get_inst_TimingInfo_result("lshr",op1->getType()->getIntegerBitWidth(),LSHRI->getType()->getIntegerBitWidth(),clock_period_str);;
            return result;
        }        
    }
    else if (AShrOperator *ASHRI = dyn_cast<AShrOperator>(I))
    {   
        Value *op1 = ASHRI->getOperand(1);
         
        if (Constant *tmpop = dyn_cast<Constant>(op1)) 
            return result;
        else
        {
            result = get_inst_TimingInfo_result("ashr",op1->getType()->getIntegerBitWidth(),ASHRI->getType()->getIntegerBitWidth(),clock_period_str);;
            return result;
        }        
    }
    else if (BinaryOperator *BinO = dyn_cast<BinaryOperator>(I))
    {
        // for binary operator, we need to consider whether it is a operator for integer or floating-point value
        std::string opcodeInput;
        int oprandBitWidth;
        int resBitWidth;
        std::string opcode_str(BinO->getOpcodeName());
        std::transform(opcode_str.begin(), opcode_str.end(), opcode_str.begin(), ::tolower);
        Value *op1 = BinO->getOperand(1);
        if (BinO->getType()->isIntegerTy())
        {
            oprandBitWidth = op1->getType()->getIntegerBitWidth();
            resBitWidth = BinO->getType()->getIntegerBitWidth();
        }
        else
        {
            oprandBitWidth = -1;
            resBitWidth = -1;
            // for floating operator, we need to consider whether it is a operator for float value or double value
            if (BinO->getType()->isDoubleTy() && opcode_str[0]=='f')            
                opcode_str[0]='d';            
        }

        // check Add for IntToPtr
        if (BinO->getOpcode()==Instruction::Add)
        {
            for (auto it = BinO->use_begin(),ie = BinO->use_end();it!=ie;++it)
            {
                User* tmp_user = it->getUser();
                if (Instruction *UserI = dyn_cast<Instruction>(tmp_user))
                {
                    if (UserI->getOpcode()==Instruction::IntToPtr)
                        return result;
                }
            }            
        }

        result = get_inst_TimingInfo_result(opcode_str,oprandBitWidth,resBitWidth,clock_period_str);;
        return result;
    }

    else if (ICmpInst *ICI = dyn_cast<ICmpInst>(I))
    {
        int oprandBitWidth;
        int resBitWidth;
        Value *op1 = ICI->getOperand(1);
        oprandBitWidth = op1->getType()->getIntegerBitWidth();
        resBitWidth = oprandBitWidth;
        result = get_inst_TimingInfo_result("icmp",oprandBitWidth,resBitWidth,clock_period_str);;
        return result;
    }
    else if (FCmpInst *FI = dyn_cast<FCmpInst>(I))
    {
        result = get_inst_TimingInfo_result("fcmp",-1,-1,clock_period_str);;
        return result;
    }
    ////////////////////////////// Memory Operations /////////////////////////
    else if (StoreInst *SI = dyn_cast<StoreInst>(I))
    {
        result = get_inst_TimingInfo_result("store",-1,-1,clock_period_str);
        return result;
    }
    else if (LoadInst *LI = dyn_cast<LoadInst>(I))
    {
        result = get_inst_TimingInfo_result("load",-1,-1,clock_period_str);
        return result;
    }
    else if (AllocaInst *AI = dyn_cast<AllocaInst>(I))
    {
        return result;
    }

    ////////////////////////////// Control Operations /////////////////////////
    else if (PHINode *PHI = dyn_cast<PHINode>(I))
    {   
        int num_Block = PHI->getNumOperands();
        for (int i=0;i<num_Block;i++)
        {
            BasicBlock *tmpB = PHI->getIncomingBlock(i);
            if (tmpB == PHI->getParent())
                return result;
        }        
        result.latency = 0;
        result.timing = 0.1;
        return result;
    }
    else if (CallInst *CI = dyn_cast<CallInst>(I))
    {
        *Evaluating_log << " Going into subfunction: " << CI->getCalledFunction()->getName() <<"\n";
        result = analyzeFunction(CI->getCalledFunction());
        return result;
    }
    else if (BranchInst *BI = dyn_cast<BranchInst>(I))
    {
        result = get_inst_TimingInfo_result("br",-1,-1,clock_period_str);
        return result;
    }
    else if (ReturnInst *RI = dyn_cast<ReturnInst>(I))
    {
        result = get_inst_TimingInfo_result("ret",-1,-1,clock_period_str);
        return result;
    }
    else if (SelectInst *SeI = dyn_cast<SelectInst>(I))
    {
        result.timing=1.0;
        return result;
    }
    else if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(I))
    {
        result = get_inst_TimingInfo_result("getelementptr",-1,-1,clock_period_str);
        return result;
    }


    result.latency = 1;
    return result;
}



// check whether the two operations can be chained
bool HI_NoDirectiveTimingResourceEvaluation::canChainOrNot(Instruction *PredI,Instruction *I)
{
    // *Evaluating_log << "        --------- checking Instruction canChainOrNot: <<" << *I << "\n";
    if (isMACpossible(PredI,I))
    {
        // *Evaluating_log << "        --------- checking Instruction " << *I << " can be chained as MAC\n";
        return true;
    }
    if (isAMApossible(PredI,I))
    {
        // *Evaluating_log << "        --------- checking Instruction " << *I << " can be chained as MAC\n";
        return true;
    }
    return false;
}



// check whether the two operations can be chained into MAC operation
bool HI_NoDirectiveTimingResourceEvaluation::isMACpossible(Instruction *PredI,Instruction *I)
{
    if (I->getOpcode()==Instruction::Add)
    {
        if (PredI->getOpcode()==Instruction::Mul)
        {
            Value *op0 = (PredI->getOperand(0));
            Value *op1 = (PredI->getOperand(1));
            // *Evaluating_log << "        --------- checking Instruction " << *I << " for being chained as MAC, getActualUsersNum=" << getActualUsersNum(PredI,0) << "\n";

            if (op0 && op1 && getActualUsersNum(PredI,0)<2)
            {
                // *Evaluating_log << "        --------- checking Instruction " << *I << " for being chained as MAC, op0BW=" << getOriginalBitwidth(op0) << "op1BW=" << getOriginalBitwidth(op1) << "IBW=" << I->getType()->getIntegerBitWidth() << "\n";
                return (getOriginalBitwidth(op0)<=18) && (getOriginalBitwidth(op1)<=18) && (I->getType()->getIntegerBitWidth()<=48);
            }
        }
        else if (PredI->getOpcode()==Instruction::Trunc ||  PredI->getOpcode()==Instruction::SExt || PredI->getOpcode()==Instruction::ZExt)
        {
            Instruction *Pred_Pred_I = dyn_cast<Instruction>(PredI->getOperand(0));
            if (Pred_Pred_I)
            {
                if (Pred_Pred_I->getOpcode()==Instruction::Mul)
                {
                    Value *op0 = (Pred_Pred_I->getOperand(0));
                    Value *op1 = (Pred_Pred_I->getOperand(1));
                    if (op0 && op1 && getActualUsersNum(Pred_Pred_I,0)<2)
                    {
                        return (getOriginalBitwidth(op0)<=18) && (getOriginalBitwidth(op1)<=18) && (I->getType()->getIntegerBitWidth()<=48);
                    }
                }
            }
        }
    }    
    return false;
}


bool HI_NoDirectiveTimingResourceEvaluation::isAMApossible(Instruction *PredI,Instruction *I)
{
    // for the GEP MAA, consider to transform it into AMA
    if (I->getOpcode()==Instruction::Add)
    {
        if (PredI->getOpcode()==Instruction::Add)
        {
            Value *op0 = (PredI->getOperand(0));
            Value *op1 = (PredI->getOperand(1));
            if (auto Pred_Pred_I = dyn_cast<Instruction>(op0))
            {
                if (Pred_Pred_I->getOpcode()==Instruction::Mul)
                {
                    if (auto Pred_Pred_I_const = dyn_cast<ConstantInt>(Pred_Pred_I->getOperand(1)))
                    {
                        if (auto I_const = dyn_cast<ConstantInt>(I->getOperand(1)))
                        {
                            long long add_const = (I_const->getValue().getSExtValue());
                            long long mul_const = (Pred_Pred_I_const->getValue().getSExtValue());
                            if (add_const % mul_const == 0)
                            {
                                return (getOriginalBitwidth(op0)<=18) && (getOriginalBitwidth(op1)<=18) && (Pred_Pred_I->getType()->getIntegerBitWidth()<=48);
                            }
                        }
                    }
                }
            }
        }
    }    
    return false;
}



// Trace back to get the bitwidth of an operand, bypassing truct/zext/sext
int HI_NoDirectiveTimingResourceEvaluation::getOriginalBitwidth(Value *Val)
{
    if (Instruction *I = dyn_cast<Instruction>(Val))
    {
        if (I->getOpcode()==Instruction::Trunc ||  I->getOpcode()==Instruction::SExt || I->getOpcode()==Instruction::ZExt)
            return getOriginalBitwidth((I->getOperand(0)));
        else 
            return I->getType()->getIntegerBitWidth();
    }

    else 
        return Val->getType()->getIntegerBitWidth();
}



// Trace forward to get the bitwidth of an operand, bypassing truct/zext/sext
int HI_NoDirectiveTimingResourceEvaluation::getActualUsersNum(Instruction *I, int dep)
{
    std::string cur_opcode = I->getOpcodeName();
    // *Evaluating_log << "        --------- getActualUsersNum tracing " << *I << " at dep=" << dep << ".\n";
    if (dep==0 || I->getOpcode()==Instruction::Trunc ||  I->getOpcode()==Instruction::SExt || I->getOpcode()==Instruction::ZExt)
    {
        int num=0;
        for (auto it=I->use_begin(),ie=I->use_end();it!=ie;++it)
        {
            User* tmp_user = it->getUser();
            if (Instruction *tmpI = dyn_cast<Instruction>(tmp_user))
                num += getActualUsersNum(tmpI,dep+1);
        }
        return num;
    }
    else 
    {
        return 1;
    }        
}



HI_NoDirectiveTimingResourceEvaluation::resourceBase HI_NoDirectiveTimingResourceEvaluation::getInstructionResource(Instruction *I)
{
    resourceBase result(0,0,0,clock_period);

    ////////////////////////////// Cast Operations /////////////////////////
    if (PtrToIntInst *PTI = dyn_cast<PtrToIntInst>(I)) // such operation like trunc/ext will not cost extra timing on FPGA
    {   
        return result;
    }
    else if (IntToPtrInst *ITP = dyn_cast<IntToPtrInst>(I))
    {   
        return result;
    }
    else if (FPToUIInst *FTU = dyn_cast<FPToUIInst>(I))
    {
        result = get_inst_ResourceInfo_result("fptoui",FTU->getType()->getIntegerBitWidth(),FTU->getType()->getIntegerBitWidth(),clock_period_str);
        return result;
    }
    else if (FPToSIInst *FTS = dyn_cast<FPToSIInst>(I))
    {
        result = get_inst_ResourceInfo_result("fptosi",FTS->getType()->getIntegerBitWidth(),FTS->getType()->getIntegerBitWidth(),clock_period_str);
        return result;
    }
    else if (UIToFPInst *UTF = dyn_cast<UIToFPInst>(I))
    {
        Value *op1 = UTF->getOperand(1);
        result = get_inst_ResourceInfo_result("uitofp",op1->getType()->getIntegerBitWidth(),op1->getType()->getIntegerBitWidth(),clock_period_str);
        return result;
    }
    else if (SIToFPInst *STF = dyn_cast<SIToFPInst>(I))
    {
        Value *op1 = STF->getOperand(1);
        result = get_inst_ResourceInfo_result("sitofp",op1->getType()->getIntegerBitWidth(),op1->getType()->getIntegerBitWidth(),clock_period_str);
        return result;
    }
    else if (ZExtInst *ZEXTI = dyn_cast<ZExtInst>(I))  // such operation like trunc/ext will not cost extra timing on FPGA
    {   
        return result;
    }
    else if (SExtInst *SEXTI = dyn_cast<SExtInst>(I))
    {   
        return result;
    }
    else if (TruncInst *TI = dyn_cast<TruncInst>(I))
    {   
        return result;
    }
    else if (BitCastInst *BI = dyn_cast<BitCastInst>(I))
    {   
        return result;
    }

    ////////////////////////////// Binary Operations /////////////////////////
    else if (ShlOperator *SHLI = dyn_cast<ShlOperator>(I))
    {   
        Value *op1 = SHLI->getOperand(1);
         
        if (Constant *tmpop = dyn_cast<Constant>(op1))
            return result;
            {
                result = get_inst_ResourceInfo_result("shl",op1->getType()->getIntegerBitWidth(),SHLI->getType()->getIntegerBitWidth(),clock_period_str);
                return result;
            }
    }
    else if (LShrOperator *LSHRI = dyn_cast<LShrOperator>(I))
    {   
        Value *op1 = LSHRI->getOperand(1);
         
        if (Constant *tmpop = dyn_cast<Constant>(op1))
            return result;
        else
        {
            result = get_inst_ResourceInfo_result("lshr",op1->getType()->getIntegerBitWidth(),LSHRI->getType()->getIntegerBitWidth(),clock_period_str);;
            return result;
        }        
    }
    else if (AShrOperator *ASHRI = dyn_cast<AShrOperator>(I))
    {   
        Value *op1 = ASHRI->getOperand(1);
         
        if (Constant *tmpop = dyn_cast<Constant>(op1)) 
            return result;
        else
        {
            result = get_inst_ResourceInfo_result("ashr",op1->getType()->getIntegerBitWidth(),ASHRI->getType()->getIntegerBitWidth(),clock_period_str);;
            return result;
        }        
    }
    else if (BinaryOperator *BinO = dyn_cast<BinaryOperator>(I))
    {
        // for binary operator, we need to consider whether it is a operator for integer or floating-point value
        std::string opcodeInput;
        int oprandBitWidth;
        int resBitWidth;
        std::string opcode_str(BinO->getOpcodeName());
        std::transform(opcode_str.begin(), opcode_str.end(), opcode_str.begin(), ::tolower);
        Value *op1 = BinO->getOperand(1);
        if (BinO->getType()->isIntegerTy())
        {
            oprandBitWidth = op1->getType()->getIntegerBitWidth();
            resBitWidth = BinO->getType()->getIntegerBitWidth();
        }
        else
        {
            oprandBitWidth = -1;
            resBitWidth = -1;
            // for floating operator, we need to consider whether it is a operator for float value or double value
            if (BinO->getType()->isDoubleTy() && opcode_str[0]=='f')            
                opcode_str[0]='d';            
        }

        // check Add for IntToPtr
        if (BinO->getOpcode()==Instruction::Add)
        {
            for (auto it = BinO->use_begin(),ie = BinO->use_end();it!=ie;++it)
            {
                User* tmp_user = it->getUser();
                if (Instruction *UserI = dyn_cast<Instruction>(tmp_user))
                {
                    if (UserI->getOpcode()==Instruction::IntToPtr)
                        return result;
                }
            }            
        }

        result = get_inst_ResourceInfo_result(opcode_str,oprandBitWidth,resBitWidth,clock_period_str);;
        return result;
    }

    else if (ICmpInst *ICI = dyn_cast<ICmpInst>(I))
    {
        int oprandBitWidth;
        int resBitWidth;
        Value *op1 = ICI->getOperand(1);
        oprandBitWidth = op1->getType()->getIntegerBitWidth();
        resBitWidth = oprandBitWidth;
        result = get_inst_ResourceInfo_result("icmp",oprandBitWidth,resBitWidth,clock_period_str);;
        return result;
    }
    else if (FCmpInst *FI = dyn_cast<FCmpInst>(I))
    {
        result = get_inst_ResourceInfo_result("fcmp",-1,-1,clock_period_str);;
        return result;
    }
    ////////////////////////////// Memory Operations /////////////////////////
    else if (StoreInst *SI = dyn_cast<StoreInst>(I))
    {
        result = get_inst_ResourceInfo_result("store",-1,-1,clock_period_str);
        return result;
    }
    else if (LoadInst *LI = dyn_cast<LoadInst>(I))
    {
        result = get_inst_ResourceInfo_result("load",-1,-1,clock_period_str);
        return result;
    }
    else if (AllocaInst *AI = dyn_cast<AllocaInst>(I))
    {
        return result;
    }

    ////////////////////////////// Control Operations /////////////////////////
    else if (PHINode *PHI = dyn_cast<PHINode>(I))
    {   
        return result;
    }
    else if (CallInst *CI = dyn_cast<CallInst>(I))
    {
        *Evaluating_log << " Going into subfunction: " << CI->getCalledFunction()->getName() <<"\n";
        result = getFunctionResource(CI->getCalledFunction());
        return result;
    }
    else if (BranchInst *BI = dyn_cast<BranchInst>(I))
    {
        return result;
    }
    else if (ReturnInst *RI = dyn_cast<ReturnInst>(I))
    {
        return result;
    }
    else if (SelectInst *SeI = dyn_cast<SelectInst>(I))
    {
        return result;
    }
    else if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(I))
    {
        return result;
    }
    return result;
}




// evaluate the number of FF needed by the instruction
HI_NoDirectiveTimingResourceEvaluation::resourceBase HI_NoDirectiveTimingResourceEvaluation::FF_Evaluate(std::map<Instruction*, timingBase> &cur_InstructionCriticalPath, Instruction* I)
{
    resourceBase res(0,0,0,clock_period);

    // Handle Load/Store for FF calculation since usually we have lower the GEP to mul/add/inttoptr/ptrtoint operations
    if (auto storeI = dyn_cast<StoreInst>(I))
    {
        if (auto l0_pred = dyn_cast<IntToPtrInst>(storeI->getOperand(1)))
        {
            if (auto l1_pred = dyn_cast<AddOperator>(l0_pred->getOperand(0)))
            {
                if (auto l2_pred = dyn_cast<Instruction>(l1_pred->getOperand(1)))
                {

                    // check whether we should consider the FF cost by this instruction l2_pred
                    if (Instruction_FFAssigned.find(l2_pred) != Instruction_FFAssigned.end())
                        return res;

                    if (BlockContain(I->getParent(), l2_pred))
                    {
                        if (cur_InstructionCriticalPath.find(l2_pred) != cur_InstructionCriticalPath.end())
                            if (cur_InstructionCriticalPath[l2_pred].latency  == (cur_InstructionCriticalPath[I].latency - getInstructionLatency(I).latency))
                                return res;
                    }
                    
                    // For ZExt/SExt Instruction, we do not need to consider those constant bits
                    int minBW = l2_pred->getType()->getIntegerBitWidth();
                    if (auto zext_I = dyn_cast<ZExtInst>(l2_pred))
                        minBW = zext_I->getSrcTy()->getIntegerBitWidth();
                    if (auto sext_I = dyn_cast<SExtInst>(l2_pred))
                        minBW = sext_I->getSrcTy()->getIntegerBitWidth(); 
                    res.FF = minBW;
                    Instruction_FFAssigned.insert(l2_pred);                   
                }
            }
            else
            {
                print_warning("WARNING: The PRE-predecessor of store instruction should be AddOperator.");
            }
        }
        else
        {
            print_warning("WARNING: The predecessor of store instruction should be IntToPtrInst.");
        }

        if (auto I_Pred = dyn_cast<Instruction>(storeI->getOperand(0)))
        {
            if (I_Pred->getType()->isIntegerTy() )
            {
                int minBW = I_Pred->getType()->getIntegerBitWidth();
                
                // For ZExt/SExt Instruction, we do not need to consider those constant bits
                if (auto zext_I = dyn_cast<ZExtInst>(I_Pred))
                {                    
                    Instruction* ori_I = byPassUnregisterOp(zext_I);
                    if (Instruction_FFAssigned.find(ori_I) == Instruction_FFAssigned.end())
                    {
                        minBW = zext_I->getSrcTy()->getIntegerBitWidth();
                        Instruction_FFAssigned.insert(ori_I);
                    }
                    
                    // if (auto )
                    // res = FF_Evaluate(cur_InstructionCriticalPath,static_cast<Instruction>());
                }
                if (auto sext_I = dyn_cast<SExtInst>(I_Pred))
                {
                    Instruction* ori_I = byPassUnregisterOp(sext_I);
                    if (Instruction_FFAssigned.find(ori_I) == Instruction_FFAssigned.end())
                    {
                        minBW = sext_I->getSrcTy()->getIntegerBitWidth();
                        Instruction_FFAssigned.insert(ori_I);
                    }                 
                }
                    

                res.FF += minBW;
                
                Instruction_FFAssigned.insert(I_Pred);
            }
        }


        return res;
    }

    // Handle Load/Store for FF calculation since usually we have lower the GEP to mul/add/inttoptr/ptrtoint operations
    if (auto loadI = dyn_cast<LoadInst>(I))
    {
        if (auto l0_pred = dyn_cast<IntToPtrInst>(loadI->getOperand(0)))
        {
            if (auto l1_pred = dyn_cast<AddOperator>(l0_pred->getOperand(0)))
            {
                if (auto l2_pred = dyn_cast<Instruction>(l1_pred->getOperand(1)))
                {                  

                    // check whether we should consider the FF cost by this instruction l2_pred
                    if (Instruction_FFAssigned.find(l2_pred) != Instruction_FFAssigned.end())
                        return res;

                    if (BlockContain(I->getParent(), l2_pred))
                    {
                        if (cur_InstructionCriticalPath.find(l2_pred) != cur_InstructionCriticalPath.end())
                            if (cur_InstructionCriticalPath[l2_pred].latency  == (cur_InstructionCriticalPath[I].latency - getInstructionLatency(I).latency))
                                return res;
                    }
                    
                    // For ZExt/SExt Instruction, we do not need to consider those constant bits
                    int minBW = l2_pred->getType()->getIntegerBitWidth();
                    if (auto zext_I = dyn_cast<ZExtInst>(l2_pred))
                        minBW = zext_I->getSrcTy()->getIntegerBitWidth();
                    if (auto sext_I = dyn_cast<SExtInst>(l2_pred))
                        minBW = sext_I->getSrcTy()->getIntegerBitWidth(); 
                    res.FF = minBW;
                    Instruction_FFAssigned.insert(l2_pred);                   
                }
            }
            else
            {
                print_warning("WARNING: The PRE-predecessor of load instruction should be AddOperator.");
            }
        }
        else
        {
            print_warning("WARNING: The predecessor of load instruction should be IntToPtrInst.");
        }



        return res;
    }

    // Handle Load/Store for FF calculation since usually we have lower the GEP to mul/add/inttoptr/ptrtoint operations
    for (auto use_IT=I->use_begin(), use_IE=I->use_end(); use_IT!=use_IE; ++use_IT)
    {
        if (auto int2ptr = dyn_cast<IntToPtrInst>(use_IT->getUser()))
        {
            return res;
        }
    }


    for (User::op_iterator I_tmp = I->op_begin(), I_Pred_end = I->op_end(); I_tmp != I_Pred_end; ++I_tmp)
    {
        if (auto I_Pred = dyn_cast<Instruction>(I_tmp))
        {
            // check whether we should consider the FF cost by this instruction I
            if (Instruction_FFAssigned.find(I_Pred) != Instruction_FFAssigned.end())
            {
                continue;
            }                

            if (BlockContain(I->getParent(), I_Pred))
            {
                // may be the operand is operated later, especially for phi insturction in loop
                if (cur_InstructionCriticalPath.find(I_Pred) != cur_InstructionCriticalPath.end())
                    if (cur_InstructionCriticalPath[I_Pred].latency == (cur_InstructionCriticalPath[I].latency - getInstructionLatency(I).latency))
                        continue;
            }           

            // calculate the FF needed to store the immediate result
            if (I_Pred->getType()->isIntegerTy() )
            {
                int minBW = I_Pred->getType()->getIntegerBitWidth();
                
                // For ZExt/SExt Instruction, we do not need to consider those constant bits
                if (auto zext_I = dyn_cast<ZExtInst>(I_Pred))
                {                    
                    Instruction* ori_I = byPassUnregisterOp(zext_I);
                    if (Instruction_FFAssigned.find(ori_I) != Instruction_FFAssigned.end())
                    {
                        continue;
                    }
                    else
                    {
                        minBW = zext_I->getSrcTy()->getIntegerBitWidth();
                        Instruction_FFAssigned.insert(ori_I);
                    }
                    
                    // if (auto )
                    // res = FF_Evaluate(cur_InstructionCriticalPath,static_cast<Instruction>());
                }
                if (auto sext_I = dyn_cast<SExtInst>(I_Pred))
                {
                    Instruction* ori_I = byPassUnregisterOp(sext_I);
                    if (Instruction_FFAssigned.find(ori_I) != Instruction_FFAssigned.end())
                    {
                        continue;
                    }
                    else
                    {
                        minBW = sext_I->getSrcTy()->getIntegerBitWidth();
                        Instruction_FFAssigned.insert(ori_I);
                    }                 
                }
                    

                res.FF += minBW;
                
                Instruction_FFAssigned.insert(I_Pred);
            }
            else if (I_Pred->getType()->isFloatTy() )
            {
                res.FF += 32;
                Instruction_FFAssigned.insert(I_Pred);
            }
            else if (I_Pred->getType()->isDoubleTy() )
            {
                res.FF += 64;
                Instruction_FFAssigned.insert(I_Pred);
            }
        }           
    }
    return res;
}



// evaluate the number of LUT needed by the PHI instruction
HI_NoDirectiveTimingResourceEvaluation::resourceBase HI_NoDirectiveTimingResourceEvaluation::IndexVar_LUT(std::map<Instruction*, timingBase> &cur_InstructionCriticalPath, Instruction* I)
{
    resourceBase res(0,0,0,clock_period);

    if (auto PHI_I = dyn_cast<PHINode>(I))
    {
        for (User::op_iterator I_tmp = I->op_begin(), I_Pred_end = I->op_end(); I_tmp != I_Pred_end; ++I_tmp)
        {
            if (auto I_Pred = dyn_cast<Instruction>(I_tmp))
            {
                if (BlockContain(I->getParent(), I_Pred))
                {
                    // may be the operand is operated later, especially for phi insturction in loop
                    if (cur_InstructionCriticalPath.find(I_Pred) == cur_InstructionCriticalPath.end())
                    {
                        res.LUT = 9; // for invar PHI with two input
                    }
                }   
            }           
        }
    }
    return res;
}


// evaluate the number of LUT needed by the BRAM MUXs
HI_NoDirectiveTimingResourceEvaluation::resourceBase HI_NoDirectiveTimingResourceEvaluation::BRAM_MUX_Evaluate()
{
    resourceBase res(0,0,0,clock_period);
    int LUT_total = 0;
       
    for (auto Val_IT : target2LastAccessCycleInBlock)
    {
        int access_counter_for_value = 0;
        *Evaluating_log << " The access to target: [" << Val_IT.first->getName() <<"] includes:\n";
        for (auto B2Cycles : Val_IT.second)
        {
            access_counter_for_value += B2Cycles.second.size();
            *Evaluating_log << " in block: [" << B2Cycles.first->getName() <<"] cycles: ";
            for (auto C_tmp : B2Cycles.second)
                *Evaluating_log << " --- " << C_tmp <<" ";
            *Evaluating_log << " \n";
            Evaluating_log->flush();
        }
        *Evaluating_log << " \n\n";
        // This analysis is based on observation
        if (access_counter_for_value <= 2)
        {
            LUT_total += 0;
        }
        else if (access_counter_for_value <= 4)
        {
            LUT_total += 6;
        }
        else 
        {
            LUT_total += (access_counter_for_value + 2);
        }
    }

    res.LUT = LUT_total * 5;
    return res;
}


// trace back to find the original operator, bypassing SExt and ZExt operations
Instruction* HI_NoDirectiveTimingResourceEvaluation::byPassUnregisterOp(Instruction* cur_I)
{
                
    // For ZExt/SExt Instruction, we do not need to consider those constant bits
    if (/*cur_I->getOpcode() == Instruction::Trunc ||*/ cur_I->getOpcode() == Instruction::ZExt || cur_I->getOpcode() == Instruction::SExt )
    {
        if (auto next_I = dyn_cast<Instruction>(cur_I->getOperand(0)))
        {
            return byPassUnregisterOp(next_I);
        }
        else
        {
            assert(false && "Predecessor of bitcast operator should be found.\n");
        }
    }
    // else if ( cur_I->getOpcode() == Instruction::PHI )
    // {
    //     int constant_cnt = 0;
    //     Instruction *I_incoming;
    //     for (int i = 0 ; i < cur_I->getNumOperands(); ++i )
    //     {
    //         if (auto const_val = dyn_cast<Constant>(cur_I->getOperand(i)))
    //         {
    //             constant_cnt ++;
    //         }
    //         else
    //         {
    //             if (auto op_I = dyn_cast<Instruction>(cur_I->getOperand(i)))
    //             {
    //                 I_incoming = op_I;
    //             }
    //         }            
    //     }
    //     if (constant_cnt == 1)
    //     {
    //         return byPassUnregisterOp(I_incoming);
    //     }
    //     else
    //     {
    //         return cur_I;
    //     }
        
    // }
    else
    {
        return cur_I;
    }    
}


// trace back to find the original operator, bypassing SExt and ZExt operations
Instruction* HI_NoDirectiveTimingResourceEvaluation::byPassBitcastOp(Instruction* cur_I)
{
                
    // For ZExt/SExt Instruction, we do not need to consider those constant bits
    if (cur_I->getOpcode() == Instruction::Trunc || cur_I->getOpcode() == Instruction::ZExt || cur_I->getOpcode() == Instruction::SExt )
    {
        if (auto next_I = dyn_cast<Instruction>(cur_I->getOperand(0)))
        {
            return byPassBitcastOp(next_I);
        }
        else
        {
            assert(false && "Predecessor of bitcast operator should be found.\n");
        }
    }
    else
    {
        return cur_I;
    }    
}

