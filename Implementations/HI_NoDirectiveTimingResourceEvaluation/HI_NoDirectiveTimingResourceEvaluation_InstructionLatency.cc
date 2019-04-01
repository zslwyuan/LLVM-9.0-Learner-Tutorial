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
// HI_NoDirectiveTimingResourceEvaluation::timingBase 
HI_NoDirectiveTimingResourceEvaluation::timingBase HI_NoDirectiveTimingResourceEvaluation::getInstructionLatency(Instruction *I)
{
    timingBase result(0,0,1,clock_period);

    ////////////////////////////// Cast Operations /////////////////////////
    if (PtrToIntInst *PTI = dyn_cast<PtrToIntInst>(I))
    {   
        return result;
    }
    else if (IntToPtrInst *ITP = dyn_cast<IntToPtrInst>(I))
    {   
        return result;
    }
    else if (FPToUIInst *FTU = dyn_cast<FPToUIInst>(I))
    {
        result = get_inst_info_result("fptoui",FTU->getType()->getIntegerBitWidth(),FTU->getType()->getIntegerBitWidth(),clock_period_str);
        return result;
    }
    else if (FPToSIInst *FTS = dyn_cast<FPToSIInst>(I))
    {
        result = get_inst_info_result("fptosi",FTS->getType()->getIntegerBitWidth(),FTS->getType()->getIntegerBitWidth(),clock_period_str);
        return result;
    }
    else if (UIToFPInst *UTF = dyn_cast<UIToFPInst>(I))
    {
        Value *op1 = UTF->getOperand(1);
        result = get_inst_info_result("uitofp",op1->getType()->getIntegerBitWidth(),op1->getType()->getIntegerBitWidth(),clock_period_str);
        return result;
    }
    else if (SIToFPInst *STF = dyn_cast<SIToFPInst>(I))
    {
        Value *op1 = STF->getOperand(1);
        result = get_inst_info_result("sitofp",op1->getType()->getIntegerBitWidth(),op1->getType()->getIntegerBitWidth(),clock_period_str);
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
                result = get_inst_info_result("shl",op1->getType()->getIntegerBitWidth(),SHLI->getType()->getIntegerBitWidth(),clock_period_str);
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
            result = get_inst_info_result("lshr",op1->getType()->getIntegerBitWidth(),LSHRI->getType()->getIntegerBitWidth(),clock_period_str);;
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
            result = get_inst_info_result("ashr",op1->getType()->getIntegerBitWidth(),ASHRI->getType()->getIntegerBitWidth(),clock_period_str);;
            return result;
        }        
    }
    else if (BinaryOperator *BinO = dyn_cast<BinaryOperator>(I))
    {
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

        result = get_inst_info_result(opcode_str,oprandBitWidth,resBitWidth,clock_period_str);;
        return result;
    }

    else if (ICmpInst *ICI = dyn_cast<ICmpInst>(I))
    {
        int oprandBitWidth;
        int resBitWidth;
        Value *op1 = ICI->getOperand(1);
        oprandBitWidth = op1->getType()->getIntegerBitWidth();
        resBitWidth = oprandBitWidth;
        result = get_inst_info_result("icmp",oprandBitWidth,resBitWidth,clock_period_str);;
        return result;
    }
    else if (FCmpInst *FI = dyn_cast<FCmpInst>(I))
    {
        result = get_inst_info_result("fcmp",-1,-1,clock_period_str);;
        return result;
    }
    ////////////////////////////// Memory Operations /////////////////////////
    else if (StoreInst *SI = dyn_cast<StoreInst>(I))
    {
        result = get_inst_info_result("store",-1,-1,clock_period_str);
        return result;
    }
    else if (LoadInst *LI = dyn_cast<LoadInst>(I))
    {
        result = get_inst_info_result("load",-1,-1,clock_period_str);
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
        result.latency = 1;
        return result;
    }
    else if (CallInst *CI = dyn_cast<CallInst>(I))
    {
        *Evaluating_log << " Going into subfunction: " << CI->getCalledFunction()->getName() <<"\n";
        result = getFunctionLatency(CI->getCalledFunction());
        return result;
    }
    else if (BranchInst *BI = dyn_cast<BranchInst>(I))
    {
        result = get_inst_info_result("br",-1,-1,clock_period_str);
        return result;
    }
    else if (ReturnInst *RI = dyn_cast<ReturnInst>(I))
    {
        result = get_inst_info_result("ret",-1,-1,clock_period_str);
        return result;
    }
    else if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(I))
    {
        result = get_inst_info_result("getelementptr",-1,-1,clock_period_str);
        return result;
    }


    result.latency = 1;
    return result;
}