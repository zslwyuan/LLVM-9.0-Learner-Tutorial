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
    if (isMACpossible(PredI,I))
        return true;
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
            if (op0 && op1 && getActualUsersNum(PredI,0)<2)
            {
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
    if (dep==0 || I->getOpcode()==Instruction::Trunc ||  I->getOpcode()==Instruction::SExt || I->getOpcode()==Instruction::ZExt)
    {
        int num=0;
        for (auto it=I->use_begin(),ie=I->use_end();it!=ie;++it)
        {
            User* tmp_user = it->getUser();
            if (Instruction *tmpI = dyn_cast<Instruction>(tmp_user))
                num += getActualUsersNum(tmpI,dep+1);
        }
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