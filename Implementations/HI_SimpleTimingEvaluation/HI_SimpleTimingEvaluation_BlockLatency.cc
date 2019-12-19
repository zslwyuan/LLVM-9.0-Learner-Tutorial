#include "HI_SimpleTimingEvaluation.h"
#include "HI_print.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

#include <ios>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace llvm;

/*
    Evaluate the latency of the block
    (1) iterate the instructions in the block
    (2) check the CP to the instruction's predecessors and find the maximum one to update its CP
    (3) get the maximum CP among instructions and take it as the CP of block
*/
double HI_SimpleTimingEvaluation::BlockLatencyEvaluation(BasicBlock *B)
{
    *Evaluating_log << "---- Evaluating Block Latency for Block: " << B->getName() << ":\n";

    if (BlockLatency.find(B) != BlockLatency.end())
    {
        *Evaluating_log << "---- Done evaluation of Block Latency for Block: " << B->getName()
                        << " and the latency is " << BlockLatency[B] << "\n";
        return BlockLatency[B]; // if B is evaluated, return directly.
    }

    // A container records the critical path from the block entry to specific instruction
    std::map<Instruction *, double> cur_InstructionCriticalPath;

    // iterate the instructions in the block
    double max_critical_path = 0.0;

    // (1) iterate the instructions in the block
    for (Instruction &rI : *B)
    {
        Instruction *I = &rI;
        double tmp_I_latency = getInstructionLatency(I);
        cur_InstructionCriticalPath[I] = tmp_I_latency;

        // (2) check the CP to the instruction's predecessors and find the maximum one to update its
        // CP
        for (User::op_iterator I_tmp = I->op_begin(), I_Pred_end = I->op_end(); I_tmp != I_Pred_end;
             ++I_tmp) // update the critical path to I by checking its predecessors' critical path
        {
            if (auto I_Pred = dyn_cast<Instruction>(I_tmp))
            {
                // ensure that the predecessor is in the block and before I, considering some
                // predecessors may be located behind the instruction itself (not in
                // cur_InstructionCriticalPath yet) in some loop structures
                if (BlockContain(B, I_Pred) &&
                    cur_InstructionCriticalPath.find(I_Pred) != cur_InstructionCriticalPath.end())
                {
                    if (cur_InstructionCriticalPath[I_Pred] + tmp_I_latency >
                        cur_InstructionCriticalPath[I]) // update the critical path
                        cur_InstructionCriticalPath[I] =
                            cur_InstructionCriticalPath[I_Pred] + tmp_I_latency;
                }
            }
        }

        // (3) get the maximum CP among instructions and take it as the CP of block
        if (cur_InstructionCriticalPath[I] > max_critical_path)
            max_critical_path = cur_InstructionCriticalPath[I];
        *Evaluating_log << "--------- Evaluated Instruction critical path for Instruction: <<" << *I
                        << ">> and its CP is :" << cur_InstructionCriticalPath[I] << "\n";
    }
    InstructionCriticalPath_inBlock[B] = cur_InstructionCriticalPath;

    BlockLatency[B] = max_critical_path;
    *Evaluating_log << "---- Done evaluation of Block Latency for Block: " << B->getName()
                    << " and the latency is " << BlockLatency[B] << "\n";
    BlockEvaluated.insert(B);
    return max_critical_path;
}

/*
    mainly used to ensure that the predecessor is in the block
*/
bool HI_SimpleTimingEvaluation::BlockContain(BasicBlock *B, Instruction *I)
{
    return I->getParent() == B;
}

/*
    mainly used to get the latency of an instruction
*/
double HI_SimpleTimingEvaluation::getInstructionLatency(Instruction *I)
{
    if (PtrToIntInst *tmpI = dyn_cast<PtrToIntInst>(I))
    {
        return 0.0;
    }

    if (IntToPtrInst *tmpI = dyn_cast<IntToPtrInst>(I))
    {
        return 0.0;
    }

    if (ShlOperator *tmpI = dyn_cast<ShlOperator>(I))
    {
        Value *op1 = tmpI->getOperand(1);

        if (Constant *tmpop = dyn_cast<Constant>(op1))
            return 0.0;
    }

    if (LShrOperator *tmpI = dyn_cast<LShrOperator>(I))
    {
        Value *op1 = tmpI->getOperand(1);

        if (Constant *tmpop = dyn_cast<Constant>(op1))
            return 0.0;
    }

    // such operation like trunc/ext will not cost extra timing on FPGA
    if (ZExtInst *tmpI = dyn_cast<ZExtInst>(I))
    {
        return 0.0;
    }

    if (SExtInst *tmpI = dyn_cast<SExtInst>(I))
    {
        return 0.0;
    }

    if (TruncInst *tmpI = dyn_cast<TruncInst>(I))
    {
        return 0.0;
    }

    if (PHINode *tmpI = dyn_cast<PHINode>(I))
    {
        int num_Block = tmpI->getNumOperands();
        for (int i = 0; i < num_Block; i++)
        {
            BasicBlock *tmpB = tmpI->getIncomingBlock(i);
            if (tmpB == tmpI->getParent())
                return 0.0;
        }
    }

    if (CallInst *tmpI = dyn_cast<CallInst>(I))
    {
        *Evaluating_log << " Going into subfunction: " << tmpI->getCalledFunction()->getName()
                        << "\n";
        return getFunctionLatency(tmpI->getCalledFunction());
    }
    return 1.0;
}