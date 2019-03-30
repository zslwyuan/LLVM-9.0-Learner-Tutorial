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
    if (PtrToIntInst *tmpI = dyn_cast<PtrToIntInst>(I))
    {   
        return result;
    }
    else if (IntToPtrInst *tmpI = dyn_cast<IntToPtrInst>(I))
    {   
        return result;
    }
    else if (ShlOperator *tmpI = dyn_cast<ShlOperator>(I))
    {   
        Value *op1 = tmpI->getOperand(1);
         
        if (Constant *tmpop = dyn_cast<Constant>(op1))
            return result;
            {
                result = get_inst_info_result("shl",op1->getType()->getIntegerBitWidth(),tmpI->getType()->getIntegerBitWidth(),clock_period_str);
                return result;
            }
    }
    else if (LShrOperator *tmpI = dyn_cast<LShrOperator>(I))
    {   
        Value *op1 = tmpI->getOperand(1);
         
        if (Constant *tmpop = dyn_cast<Constant>(op1))
            return result;
        else
        {
            result = get_inst_info_result("lshr",op1->getType()->getIntegerBitWidth(),tmpI->getType()->getIntegerBitWidth(),clock_period_str);;
            return result;
        }        
    }
    else if (AShrOperator *tmpI = dyn_cast<AShrOperator>(I))
    {   
        Value *op1 = tmpI->getOperand(1);
         
        if (Constant *tmpop = dyn_cast<Constant>(op1))
            return result;
        else
        {
            result = get_inst_info_result("ashr",op1->getType()->getIntegerBitWidth(),tmpI->getType()->getIntegerBitWidth(),clock_period_str);;
            return result;
        }        
    }
    else if (ZExtInst *tmpI = dyn_cast<ZExtInst>(I))  // such operation like trunc/ext will not cost extra timing on FPGA
    {   
        return result;
    }
    else if (SExtInst *tmpI = dyn_cast<SExtInst>(I))
    {   
        return result;
    }
    else if (TruncInst *tmpI = dyn_cast<TruncInst>(I))
    {   
        return result;
    }
    else if (BinaryOperator *BinO = dyn_cast<BinaryOperator>(I))
    {
        std::string opcodeInput;
        int oprandBitWidth;
        int resBitWidth;
        std::string opcode_str(BinO->getOpcodeName());
        std::transform(opcode_str.begin(), opcode_str.end(), opcode_str.begin(), ::tolower);
        Value *op1 = tmpI->getOperand(1);
        if (BinO->getType()->isIntegerTy())
        {
            oprandBitWidth = op1->getType()->getIntegerBitWidth();
            resBitWidth = tmpI->getType()->getIntegerBitWidth();
        }
        else
        {
            oprandBitWidth = -1;
            resBitWidth = -1;
        }
        result = get_inst_info_result(opcode_str,oprandBitWidth,resBitWidth,clock_period_str);;
        return result;
    }
    else if (PHINode *tmpI = dyn_cast<PHINode>(I))
    {   
        int num_Block = tmpI->getNumOperands();
        for (int i=0;i<num_Block;i++)
        {
            BasicBlock *tmpB = tmpI->getIncomingBlock(i);
            if (tmpB == tmpI->getParent())
                return result;
        }        
        result.latency = 1;
        return result;
    }
    else if (CallInst *tmpI = dyn_cast<CallInst>(I))
    {
        *Evaluating_log << " Going into subfunction: " << tmpI->getCalledFunction()->getName() <<"\n";
        result = getFunctionLatency(tmpI->getCalledFunction());
        return result;
    }
    result.latency = 1;
    return result;
}