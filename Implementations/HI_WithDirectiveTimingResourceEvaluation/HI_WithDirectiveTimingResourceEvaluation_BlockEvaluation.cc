#include "HI_WithDirectiveTimingResourceEvaluation.h"
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
    (2) for the timing evaluation:
            a. check the CP to the instruction's predecessors and find the maximum one to update its
   CP b. get the maximum CP among instructions and take it as the CP of block c. for BRAM accesses,
   we need to consider the BRAM port num limitation to schedule the accessess (3) for the resource
   evaluation: a. check the reuse and operation chaining for LUT/DSP shared among instruction b. for
   the operands in different cycles, we need to insert FFs for them
*/
HI_WithDirectiveTimingResourceEvaluation::timingBase
HI_WithDirectiveTimingResourceEvaluation::BlockLatencyResourceEvaluation(BasicBlock *B)
{
    if (DEBUG)
        *Evaluating_log << "---- Evaluating Block Latency for Block: " << B->getName() << ":\n";

    if (BlockLatency.find(B) != BlockLatency.end())
    {
        if (DEBUG)
            *Evaluating_log << "---- Done evaluation of Block Latency for Block: " << B->getName()
                            << " and the latency is " << BlockLatency[B] << "\n";
        return BlockLatency[B] * 1; // if B is evaluated, return directly.
    }

    // A container records the critical path from the block entry to specific instruction
    std::map<Instruction *, timingBase> cur_InstructionCriticalPath;

    // initialize the timing and resource statistics
    timingBase max_critical_path(0, 0, 1, clock_period);
    timingBase origin_path(0, 0, 1, clock_period);

    resourceBase resourceAccmulator(0, 0, 0, clock_period);

    // (1) iterate the instructions in the block
    if (B->getInstList().size() > 1) // ignore block with only branch instruction
    {
        for (Instruction &rI : *B)
        {
            Instruction *I = &rI;
            timingBase tmp_I_latency = getInstructionLatency(I);
            cur_InstructionCriticalPath[I] = origin_path + tmp_I_latency;
            bool Chained = 0;
            bool PartialChained = 0;

            // (2) check the CP to the instruction's predecessors and find the maximum one to update
            // its CP
            //     but Store/Load operation should be scheduled specially due to the limited number
            //     of BRAM ports

            timingBase latest_timing(0, 0, 1, clock_period);

            timingBase partialChainedTiming;
            resourceBase partialChainedResource;

            // for general instructions, we find the latest-finished operand of them
            for (User::op_iterator I_tmp = I->op_begin(), I_Pred_end = I->op_end();
                 I_tmp != I_Pred_end; ++I_tmp) // update the critical path to I by checking its
                                               // predecessors' critical path
            {
                if (auto I_Pred = dyn_cast<Instruction>(I_tmp))
                {
                    // (1) ensure that the predecessor is in the block and before I
                    // (2) considering that some predecessors may be located behind the instruction
                    // itself (not in cur_InstructionCriticalPath yet) in some loop structures
                    if (BlockContain(B, I_Pred) && cur_InstructionCriticalPath.find(I_Pred) !=
                                                       cur_InstructionCriticalPath.end())
                    {
                        if (I->getOpcode() == Instruction::Add ||
                            I->getOpcode() == Instruction::Sub)
                        {
                            if (I_Pred->getOpcode() == Instruction::Add ||
                                I_Pred->getOpcode() == Instruction::Sub)
                            {
                                if (I_Pred->getType()->getIntegerBitWidth() == 32)
                                {
                                    int u = 0;
                                }
                            }
                        }

                        // check whether the instruction can be chained with the operand
                        // instructions to use DSPs
                        if (canCompleteChainOrNot(I_Pred, I))
                        {
                            // TODO: maybe we need to rethink the machanism carefully
                            // *Evaluating_log << "        --------- Evaluated Instruction critical
                            // path for Instruction: <<" << *I << " which can be chained.\n";
                            if (cur_InstructionCriticalPath[I_Pred] >
                                cur_InstructionCriticalPath[I]) // addition chained with
                                                                // multiplication
                            {
                                cur_InstructionCriticalPath[I] =
                                    cur_InstructionCriticalPath[I_Pred];
                                latest_timing = cur_InstructionCriticalPath[I_Pred];
                                Inst2LatestOperand[I] = I_Pred;
                            }
                            if (DEBUG)
                                *Evaluating_log << "  " << *I << " is completely chained with "
                                                << *I_Pred << "\n";
                            Chained = 1;
                            chained_I.insert(I);
                        }
                        else
                        {
                            // *Evaluating_log << "        --------- Evaluated Instruction critical
                            // path for Instruction: <<" << *I << " which cannot be chained.\n";
                            if (cur_InstructionCriticalPath[I_Pred] + tmp_I_latency >=
                                cur_InstructionCriticalPath[I]) // update the critical path
                            {
                                cur_InstructionCriticalPath[I] =
                                    cur_InstructionCriticalPath[I_Pred] + tmp_I_latency;
                                latest_timing = cur_InstructionCriticalPath[I_Pred];
                                Inst2LatestOperand[I] = I_Pred;
                            }
                        }
                    }
                }
            }

            if (Inst2LatestOperand.find(I) != Inst2LatestOperand.end())
            {
                auto I_Pred = Inst2LatestOperand[I];
                // currerntly some adders might be partially chained togather
                if (canPartitalChainOrNot(
                        I_Pred,
                        I)) //  && (cur_InstructionCriticalPath[I_Pred] + tmp_I_latency).latency ==
                            //  cur_InstructionCriticalPath[I].latency
                {
                    cur_InstructionCriticalPath[I] =
                        cur_InstructionCriticalPath[I_Pred] + getPartialTimingOverhead(I_Pred, I);
                    latest_timing = cur_InstructionCriticalPath[I_Pred];
                    partialChainedResource = getPartialResourceOverhead(Inst2LatestOperand[I], I);
                    PartialChained = 1;
                    if (DEBUG)
                        *Evaluating_log << "  " << *I << " is partially chained with " << *I_Pred
                                        << "\n";
                    chained_I.insert(I);
                    chained_I.insert(I_Pred);
                }
            }

            // for the access to pointer or callInst with pointer arguments, check previous access
            // to the target because we don't know whether there is any access in function, when
            // there is pointer interface therefore, we insert the following dependence: the IR in
            // the following patterns, there will be dependence between Load/Store and Call
            // instructions:

            // pattern 1:
            //      store XXX -> *ptr_A
            //      call func(*ptr_A)
            // the call operands of the call may not include the store instruction but there are
            // implict dependences

            // pattern 2:
            //      call func(*ptr_A)
            //      XXX = load *ptr_A
            //  the load operands may not include the call instruction but there are implict
            //  dependences.
            if (I->getOpcode() == Instruction::Load || I->getOpcode() == Instruction::Store ||
                I->getOpcode() == Instruction::Call)
            {
                bool bypassAnalysis = false;

                if (auto callI = dyn_cast<CallInst>(I))
                {
                    if (callI->getCalledFunction()->getName().find("llvm.") != std::string::npos ||
                        callI->getCalledFunction()->getName().find("HIPartitionMux") !=
                            std::string::npos)
                        bypassAnalysis = true;
                }

                if (!bypassAnalysis)
                {
                    std::vector<Value *> ptrInOperands;
                    if (DEBUG)
                        *BRAM_log << "   current access :  " << *I << "\n";
                    checkPtrInOperands(I, ptrInOperands);
                    for (auto tmp_ptr : ptrInOperands)
                    {
                        if (DEBUG)
                            *BRAM_log << "       checking pointer in args :  " << *tmp_ptr
                                      << " of targets: ";
                        if (DEBUG)
                        {
                            for (auto tmp_target : Value2Target[tmp_ptr])
                                *BRAM_log << *tmp_target << ",,";
                            *BRAM_log << "\n";
                        }

                        Instruction *latestPointerAccess = findLatestPointerAccess(
                            I, Value2Target[tmp_ptr], cur_InstructionCriticalPath);
                        if (latestPointerAccess)
                        {
                            if (DEBUG)
                                *BRAM_log
                                    << "           has pointer access :  " << *latestPointerAccess
                                    << "\n";
                            if (cur_InstructionCriticalPath.find(latestPointerAccess) !=
                                    cur_InstructionCriticalPath.end() &&
                                latestPointerAccess != I)
                            {
                                if (I->getOpcode() != Instruction::Call &&
                                    latestPointerAccess->getOpcode() !=
                                        Instruction::Call) // bypass this situation
                                    continue;
                                if (cur_InstructionCriticalPath[latestPointerAccess] +
                                        tmp_I_latency >
                                    cur_InstructionCriticalPath[I]) // update the critical path
                                {
                                    if (DEBUG)
                                        *BRAM_log << "   current access :  " << *I
                                                  << " limited by previous access: "
                                                  << *latestPointerAccess << "\n";
                                    cur_InstructionCriticalPath[I] =
                                        cur_InstructionCriticalPath[latestPointerAccess] +
                                        tmp_I_latency;
                                    latest_timing =
                                        cur_InstructionCriticalPath[latestPointerAccess];
                                    Inst2LatestOperand[I] = latestPointerAccess;
                                }
                            }
                        }
                    }
                }
            }

            // handle memory access instruction, because for BRAM access
            // apart from waiting for the address is ready, we need to
            // wait until the BRAM ports are available
            if (I->getOpcode() == Instruction::Load || I->getOpcode() == Instruction::Store)
            {

                HI_AccessInfo cur_access_info = getAccessInfoForAccessInst(I);
                std::vector<partition_info> target_partitions;

                // obtain the target partitions for the access for the scheduling
                if (cur_access_info.unpredictable)
                {
                    // if the access has unpredictable pattern, schedule the accesses for all the
                    // partition
                    target_partitions = getAllPartitionFor(I);
                    // assert(false && "unfinished.");
                }
                else
                {
                    // if the access has predictable pattern, schedule the accesses for the
                    // predicted partitions
                    target_partitions = getPartitionFor(I);
                }

                checkPartitionBenefit(target_partitions, I);

                Inst2Partitions[I] = target_partitions;

                timingBase latest_schedule_access(0, 0, 1, clock_period);

                // schedule the access for each potential target partition
                for (auto target_partition : target_partitions)
                {
                    if (DEBUG)
                    {
                        if (Inst2LatestOperand.find(I) != Inst2LatestOperand.end())
                        {
                            if (DEBUG)
                                *Evaluating_log
                                    << "----------- A Memory Access Instruction: " << *I
                                    << " is found,\n-----------  information fot this access is:  ";
                            if (DEBUG)
                                Evaluating_log->flush();
                            if (DEBUG)
                                *Evaluating_log << getAccessInfoForAccessInst(I);
                            if (DEBUG)
                                Evaluating_log->flush();
                            if (DEBUG)
                                *Evaluating_log << "\n-----------  the access is to partition #"
                                                << target_partition;
                            if (DEBUG)
                                Evaluating_log->flush();
                            if (DEBUG)
                                *Evaluating_log << "\n addressI:" << *Inst2LatestOperand[I]
                                                << " is ready at " << latest_timing;
                            if (DEBUG)
                                Evaluating_log->flush();
                            if (DEBUG)
                                *Evaluating_log << "\n-----------  do the scheduling for it\n";
                            ;
                        }
                        else
                        {
                            if (DEBUG)
                                *Evaluating_log
                                    << "----------- A Memory Access Instruction: " << *I
                                    << " is found,\n-----------  information fot this access is:  ";
                            if (DEBUG)
                                Evaluating_log->flush();
                            if (DEBUG)
                                *Evaluating_log << getAccessInfoForAccessInst(I);
                            if (DEBUG)
                                Evaluating_log->flush();
                            if (DEBUG)
                                *Evaluating_log << "\n-----------  the access is to partition #"
                                                << target_partition;
                            if (DEBUG)
                                Evaluating_log->flush();
                            if (DEBUG)
                                *Evaluating_log << "\n addressI is in previous block, is ready at "
                                                << latest_timing;
                            if (DEBUG)
                                Evaluating_log->flush();
                            if (DEBUG)
                                *Evaluating_log << "\n-----------  do the scheduling for it\n";
                            if (DEBUG)
                                Evaluating_log->flush();
                        }
                    }

                    if (DEBUG)
                        Evaluating_log->flush();

                    timingBase tmp_schedule =
                        scheduleBRAMAccess(I, B, latest_timing, target_partition);
                    if (tmp_schedule > latest_schedule_access)
                        latest_schedule_access = tmp_schedule;
                }
                cur_InstructionCriticalPath[I] = latest_schedule_access;
                AccessesList.push_back(I);
            }

            // for the operands in different cycles, we need to insert FFs for them
            // update the lifetime of the predecessors' result registers
            // by keeping updating the latest user of the result of the operand (I_Pred)
            for (User::op_iterator I_tmp = I->op_begin(), I_Pred_end = I->op_end();
                 I_tmp != I_Pred_end; ++I_tmp) // update the critical path to I by checking its
                                               // predecessors' critical path
            {
                if (auto I_Pred = dyn_cast<Instruction>(I_tmp))
                {
                    // update the latest user of the result of the operand (I_Pred)
                    // so we can know when a result reg can be released (ResultRelease).
                    updateResultRelease(
                        I, I_Pred,
                        (cur_InstructionCriticalPath[I] - getInstructionLatency(I)).latency);
                }
            }

            // accmulate the cost of resource
            if (!Chained)
            {
                // we need to specially process the cases for resource reuse
                // for other situations, we directly accumulate the resource cost.
                if (PartialChained)
                {
                    resourceAccmulator = resourceAccmulator + partialChainedResource;
                }
                else if (!checkAndTryRecordReuseOperatorDSP(
                             I,
                             (cur_InstructionCriticalPath[I] - getInstructionLatency(I)).latency))
                {
                    resourceAccmulator = resourceAccmulator + getInstructionResource(I);
                }
            }

            resourceBase FF_Num(0, 0, 0, clock_period);
            resourceBase PHI_LUT_Num(0, 0, 0, clock_period);

            // some load instructions reuse the register of previous load instructions
            FF_Num = FF_Evaluate(cur_InstructionCriticalPath, I);

            // if it is a PHINode, get the LUT it need
            PHI_LUT_Num = IndexVar_LUT(cur_InstructionCriticalPath, I);

            // cur_InstructionCriticalPath[I] = cur_InstructionCriticalPath[I] + PHI_LUT_Num;
            resourceAccmulator = resourceAccmulator + FF_Num + PHI_LUT_Num;

            // record where the instruction is scheduled
            Inst_Schedule[I] = std::pair<BasicBlock *, int>(
                B, (cur_InstructionCriticalPath[I] - getInstructionLatency(I)).latency);

            // (3) get the maximum CP among instructions and take it as the CP of block
            if (cur_InstructionCriticalPath[I] > max_critical_path)
                max_critical_path = cur_InstructionCriticalPath[I];
            if (DEBUG)
                *Evaluating_log
                    << "--------- Evaluated Instruction critical path for Instruction: <<" << *I
                    << ">> and its CP is :" << cur_InstructionCriticalPath[I]
                    << " the resource cost is: "
                    << (Chained ? (resourceBase(0, 0, 0, clock_period))
                                : (PartialChained ? partialChainedResource
                                                  : (getInstructionResource(I) + PHI_LUT_Num)))
                    << " + reg_FF: [" << FF_Num.FF << "] ";
            if (PHI_LUT_Num.LUT > 0)
            {
                if (DEBUG)
                    *Evaluating_log << "------ PHI_LUT_Num=" << PHI_LUT_Num.LUT;
            }

            if (Chained)
            {
                if (DEBUG)
                    *Evaluating_log << "(Chained))";
            }
            else
            {
                if (DEBUG)
                    *Evaluating_log << "(Not Chained)";
            }

            if (DEBUG)
                *Evaluating_log << "\n";
            if (DEBUG)
                Evaluating_log->flush();
        }
    }
    else
    {
        for (Instruction &rI : *B)
        {
            Instruction *I = &rI;
            Inst_Schedule[I] = std::pair<BasicBlock *, int>(B, 0);
        }
    }

    // record the use of the DSP for floating point.
    recordCostRescheduleFPDSPOperators_forBlock(B, max_critical_path.latency);

    InstructionCriticalPath_inBlock[B] = cur_InstructionCriticalPath;

    BlockLatency[B] = max_critical_path;
    BlockResource[B] = resourceAccmulator;
    if (DEBUG)
        *Evaluating_log << "---- Done evaluation of Block Latency for Block: " << B->getName()
                        << " and the latency is " << BlockLatency[B]
                        << " and the cost of resource is : " << resourceAccmulator << "\n";
    BlockEvaluated.insert(B);
    return max_critical_path * 1;
}

/*
    mainly used to ensure that the predecessor is in the block
*/
bool HI_WithDirectiveTimingResourceEvaluation::BlockContain(BasicBlock *B, Instruction *I)
{
    return I->getParent() == B;
}

// among the scheduled instructions, find the latest access to the specific target
// excluding the instruction "curI"
Instruction *HI_WithDirectiveTimingResourceEvaluation::findLatestPointerAccess(
    Instruction *curI, std::set<Value *> targets,
    std::map<Instruction *, timingBase> &cur_InstructionCriticalPath)
{
    timingBase latest_timing(0, 0, 1, clock_period);
    Instruction *res_I = nullptr;
    for (auto target : targets)
    {
        for (auto it : cur_InstructionCriticalPath)
        {
            if (auto I = dyn_cast<Instruction>(it.first))
            {
                if (I == curI)
                    continue;
                if (I->getOpcode() == Instruction::Load || I->getOpcode() == Instruction::Store ||
                    I->getOpcode() == Instruction::Call)
                {
                    if (auto callI = dyn_cast<CallInst>(I))
                    {
                        if (callI->getCalledFunction()->getName().find("llvm.") !=
                                std::string::npos ||
                            callI->getCalledFunction()->getName().find("HIPartitionMux") !=
                                std::string::npos)
                            continue;
                    }
                    if (it.second > latest_timing)
                    {
                        latest_timing = it.second;
                        res_I = I;
                    }
                }
            }
        }
    }

    return res_I;
}

/*
    get the latency of functions in the path to the instruction
*/
int HI_WithDirectiveTimingResourceEvaluation::getFunctionLatencyInPath(Instruction *I)
{
    int res = 0;

    if (CallInst *callI = dyn_cast<CallInst>(I))
    {
        res = FunctionLatency[callI->getCalledFunction()].latency;
    }

    if (Inst2LatestOperand.find(I) == Inst2LatestOperand.end())
        return res;

    Instruction *preI = Inst2LatestOperand[I];

    if (preI->getParent() != I->getParent())
        return res;

    return res + getFunctionLatencyInPath(preI);
}

// get the number of stage arrive the instruction
int HI_WithDirectiveTimingResourceEvaluation::getStageTo(Instruction *I)
{
    assert(Inst_Schedule.find(I) != Inst_Schedule.end());
    int instruction_latency = getInstructionLatency(I).latency;
    if (instruction_latency == -1)
        return Inst_Schedule[I].second + 1 - getFunctionLatencyInPath(I);
    else
        return Inst_Schedule[I].second + instruction_latency - getFunctionLatencyInPath(I);
}

// get the number of stage in the block
int HI_WithDirectiveTimingResourceEvaluation::getStageNumOfBlock(BasicBlock *B)
{
    if (DEBUG)
        *Evaluating_log << "checking BLOCK:\n" << *B << "\n";
    int max_stage_num = -1;
    if (BlockLatency[B].latency == 0 && BlockLatency[B].timing < 0.00001)
        return 0;
    for (auto &I : *B)
    {
        if (DEBUG)
            *Evaluating_log << "checking instruction: [" << I << "]\n";
        int stage_to_inst = getStageTo(&I);
        if (stage_to_inst > max_stage_num)
            max_stage_num = stage_to_inst;
    }
    if (DEBUG)
        *Evaluating_log << "checking BLOCK: " << B->getName() << " #stage=" << max_stage_num
                        << "\n";
    assert(max_stage_num >= 0);
    return max_stage_num;
}

// get the pointers in the operands
void HI_WithDirectiveTimingResourceEvaluation::checkPtrInOperands(
    Instruction *I, std::vector<Value *> &ptrInOperands)
{

    if (CallInst *callI = dyn_cast<CallInst>(I))
    {
        for (int i = 0; i < callI->getNumArgOperands(); i++)
        {
            if (callI->getArgOperand(i)->getType()->isPointerTy())
                ptrInOperands.push_back(callI->getArgOperand(i));
        }
    }
    else
    {
        for (int i = 0; i < I->getNumOperands(); i++)
        {
            if (I->getOperand(i)->getType()->isPointerTy())
                ptrInOperands.push_back(I->getOperand(i));
        }
    }
}

// check whether the DSP used for this instruction could be reused by the others
// reuse condition: operator / bitwidth / data source (partition) are the same
bool HI_WithDirectiveTimingResourceEvaluation::checkAndTryRecordReuseOperatorDSP(Instruction *I,
                                                                                 int timeslot)
{
    int DSPnum = getInstructionResource(I).DSP;
    if (DSPnum <= 0)
        return false;

    if (DEBUG)
        *Evaluating_log << "    check whether Instruction with DSPs [" << *I
                        << "] can reuse previous DSPs]\n";

    unsigned opcode = I->getOpcode();
    if (auto binOp = dyn_cast<BinaryOperator>(I))
    {
        auto LLoad = dyn_cast<LoadInst>(I->getOperand(0));
        auto RLoad = dyn_cast<LoadInst>(I->getOperand(1));

        if (I->getOpcode() == Instruction::FMul || I->getOpcode() == Instruction::FAdd ||
            I->getOpcode() == Instruction::FDiv || I->getOpcode() == Instruction::FSub)
        {
            // for floating point operations, we first pass all the possible DSP re-users
            // and analysis them after the analysis of total latency of blocks/loops/functions
            // BECAUSE it seems that floating-point operations will reuse DSP not matter what
            // are the operands.

            // we should found the maximum cost of each kind of FP operator among the basic blocks
            if (DEBUG)
                *Evaluating_log << "    [" << *I << "] is floating-point instruction\n";

            DSPReuseScheduleUnit schUnit(I, DSPnum, opcode, timeslot);
            Block2FPDSPReuseScheduleUnits[I->getParent()][InstToOpcodeString(I)].push_back(schUnit);
            return true;
        }
        else
        {
            // for fixed-point operations, we first mark down one of the possible DSP re-users.
            // and we will also record which operator might reuse the resource.

            // based on these record, at the end of the overall function evaluation,
            // we will check and allocate them after the analysis of total latency of
            // blocks/loops/functions BECAUSE it seems that fixed-point operations have specific
            // conditions to reuse DSP

            if (!LLoad || !RLoad)
            {
                // at least one of the operands is not load instruction

                if (DEBUG)
                    *Evaluating_log << "    Instruction with DSPs [" << *I
                                    << "] --> data are not from LoadInst]\n";
                if (clock_period <= 6.0001) // when the clock period is too low, the DSP might not
                                            // be reused due the the routing limitation
                    return false;
                else
                {
                    // initialize schUnit (a potential reuse DSP unit), with a mark:
                    // operands-are-not-simply-from-BRAM-partitions which means, when using "==" for
                    // it, it will not consider the effect of BRAM partitions
                    DSPReuseScheduleUnit schUnit(I, DSPnum, opcode, timeslot, I->getOperand(0),
                                                 I->getOperand(1));
                    if (DEBUG)
                        *Evaluating_log << "    check whether [" << schUnit
                                        << "] can reuse previous DSPs]\n";

                    // check whether the operator can reuse the DSPs from previous operaotor
                    bool reuse = false;
                    for (auto pre_schUnit : Block2IntDSPReuseScheduleUnits[I->getParent()])
                    {
                        if (DEBUG)
                            *Evaluating_log << "    checking potential reusee [" << pre_schUnit
                                            << "]]\n";

                        // the operator "==" is overrided
                        // it means "operator / bitwidth / data source (partition) are the same"
                        if (pre_schUnit == schUnit)
                        {
                            if (pre_schUnit.timeslot_inBlock != schUnit.timeslot_inBlock)
                            {
                                reuse = true;
                                if (DEBUG)
                                    *Evaluating_log << "    DSP used by: [" << *pre_schUnit.opI
                                                    << "] can be reused by [" << *I << "]\n";
                                break;
                            }
                        }
                    }

                    // although it can be reused, we still need to record it
                    // when handling loop-pipelining, some of them may not be reused.
                    Block2IntDSPReuseScheduleUnits[I->getParent()].push_back(schUnit);

                    return reuse;
                }
            }
            else
            {

                // since both operands are from load instruction.
                // and they are not from MUX, therefore,
                // they simply come from one partition respectively

                // initialize schUnit (a potential reuse DSP unit), with a mark:
                // operands-are-simply-from-BRAM-partitions which means, when using "==" for it, it
                // will consider the effect of BRAM partition
                DSPReuseScheduleUnit schUnit(I, DSPnum, opcode, timeslot, Inst2Partitions[LLoad][0],
                                             Inst2Partitions[RLoad][0]);
                if (DEBUG)
                    *Evaluating_log << "    check whether [" << schUnit
                                    << "] can reuse previous DSPs]\n";

                // check whether the operator can reuse the  DSPs from previous operaotor
                bool reuse = false;
                for (auto pre_schUnit : Block2IntDSPReuseScheduleUnits[I->getParent()])
                {
                    if (DEBUG)
                        *Evaluating_log << "    checking potential reusee [" << pre_schUnit
                                        << "]]\n";
                    if (pre_schUnit.opcode == schUnit.opcode)
                    {
                        // this region is just for debug.
                        if (DEBUG)
                        {
                            if (pre_schUnit.LPartition == schUnit.LPartition)
                            {
                                if (pre_schUnit.RPartition == schUnit.RPartition)
                                {
                                }
                                else
                                {
                                    if (DEBUG)
                                        *Evaluating_log << "           RPartitions are different\n";
                                }
                            }
                            else
                            {
                                if (DEBUG)
                                    *Evaluating_log << "           LPartitions are different\n";
                            }
                        }
                    }

                    if (pre_schUnit == schUnit)
                    {
                        if (pre_schUnit.timeslot_inBlock != schUnit.timeslot_inBlock)
                        {
                            reuse = true;
                            if (DEBUG)
                                *Evaluating_log << "    DSP used by: [" << *pre_schUnit.opI
                                                << "] can be reused by [" << *I << "]\n";
                            break;
                        }
                    }
                }

                // although it can be reused, we still need to record it
                // when handling loop-pipelining, some of them may not be reused.
                Block2IntDSPReuseScheduleUnits[I->getParent()].push_back(schUnit);

                return reuse;
            }
        }
    }
    else
    {
        return false;
    }
}

// for blocks, we need to re-check the resource cost  FOR FLOATING POINT OPERATOR since some of them
// might be reused.
void HI_WithDirectiveTimingResourceEvaluation::recordCostRescheduleFPDSPOperators_forBlock(
    BasicBlock *tmp_block, int block_latency)
{
    resourceBase res(0, 0, 0, clock_period);

    std::vector<std::string> checkopcodes = {"fmul", "fadd", "fdiv", "fsub",
                                             "dmul", "dadd", "ddiv", "dsub"};

    // Block2FPDSPReuseScheduleUnits[I->getParent()][opcode].push_back(schUnit);
    for (auto cur_opcode : checkopcodes)
    {
        int op_totalcnt = 0;

        if (Block2FPDSPReuseScheduleUnits.find(tmp_block) == Block2FPDSPReuseScheduleUnits.end())
        {
            Block2FPDSPOpCnt[tmp_block][cur_opcode] = 0;
            continue;
        }
        if (Block2FPDSPReuseScheduleUnits[tmp_block].find(cur_opcode) ==
            Block2FPDSPReuseScheduleUnits[tmp_block].end())
        {
            Block2FPDSPOpCnt[tmp_block][cur_opcode] = 0;
            continue;
        }

        op_totalcnt = Block2FPDSPReuseScheduleUnits[tmp_block][cur_opcode].size();

        int reuse_DSPModule = 1;
        if (op_totalcnt == 0)
            reuse_DSPModule = 0;
        if (op_totalcnt > block_latency)
            reuse_DSPModule = op_totalcnt / block_latency;

        if (DEBUG)
            *Evaluating_log << " for block: the amount of floating point operators (refI):"
                            << cur_opcode << " is " << reuse_DSPModule << " each cost =["
                            << checkFPOperatorCost(cur_opcode) << "]\n";

        Block2FPDSPOpCnt[tmp_block][cur_opcode] = reuse_DSPModule;
    }
}
