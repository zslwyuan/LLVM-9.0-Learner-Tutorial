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
    Evaluate the latency of the block
    (1) iterate the instructions in the block
    (2) check the CP to the instruction's predecessors and find the maximum one to update its CP
    (3) get the maximum CP among instructions and take it as the CP of block
*/
HI_NoDirectiveTimingResourceEvaluation::timingBase HI_NoDirectiveTimingResourceEvaluation::BlockLatencyResourceEvaluation(BasicBlock *B)
{
    *Evaluating_log << "---- Evaluating Block Latency for Block: " << B->getName() <<":\n";

    if (BlockLatency.find(B) != BlockLatency.end()) 
    {
        *Evaluating_log << "---- Done evaluation of Block Latency for Block: " << B->getName() <<" and the latency is "<< BlockLatency[B] <<"\n";
        return BlockLatency[B]; // if B is evaluated, return directly.
    }
    
    // A container records the critical path from the block entry to specific instruction
    std::map<Instruction*, timingBase> cur_InstructionCriticalPath; 

    // iterate the instructions in the block
    timingBase max_critical_path(0,0,1,clock_period);
    timingBase origin_path(0,0,1,clock_period);
    resourceBase resourceAccmulator(0,0,0,clock_period);
    
    // (1) iterate the instructions in the block
    if (B->getInstList().size()>1) // ignore block with only branch instruction
    {
        for (Instruction &rI : *B)
        {
            Instruction* I = &rI;
            timingBase tmp_I_latency = getInstructionLatency(I);
            cur_InstructionCriticalPath[I] = origin_path + tmp_I_latency;
            bool Chained = 0;

            // (2) check the CP to the instruction's predecessors and find the maximum one to update its CP
            //     but Store/Load operation should be scheduled specially due to the limited number of BRAM ports            
            if ( I->getOpcode()==Instruction::Load || I->getOpcode()==Instruction::Store )
            {
                *Evaluating_log << "------- A Memory Access Instruction: " << *I <<" is found, do the scheduling for it\n";
                timingBase tmp_path(0,0,1,clock_period); // get the CP to the address
                if (auto Address_I = dyn_cast<Instruction>(I->getOperand(0)))
                {
                    if (BlockContain(B, Address_I) && cur_InstructionCriticalPath.find(Address_I) != cur_InstructionCriticalPath.end()) 
                    {
                        tmp_path = cur_InstructionCriticalPath[Address_I];         
                    }                 
                }           
                else
                {
                    assert(false && "For Store/Load instruction, the analysis should not arrive this branch.\n");
                }
                cur_InstructionCriticalPath[I] = scheduleBRAMAccess(I, B, tmp_path);                
            }
            else
            {
                
                // for other instructions, we find the latest-finished operand
                for (User::op_iterator I_tmp = I->op_begin(), I_Pred_end = I->op_end(); I_tmp != I_Pred_end; ++I_tmp)// update the critical path to I by checking its predecessors' critical path
                {
                    if (auto I_Pred = dyn_cast<Instruction>(I_tmp))
                    {
                        // ensure that the predecessor is in the block and before I, considering some predecessors 
                        // may be located behind the instruction itself (not in cur_InstructionCriticalPath yet) in some loop structures 
                        if (BlockContain(B, I_Pred) && cur_InstructionCriticalPath.find(I_Pred) != cur_InstructionCriticalPath.end()) 
                        {
                            if (canChainOrNot(I_Pred,I))
                            {
                                // TODO: may need to rethink the machanism carefully
                                // *Evaluating_log << "        --------- Evaluated Instruction critical path for Instruction: <<" << *I << " which can be chained.\n";
                                if ( cur_InstructionCriticalPath[I_Pred]  > cur_InstructionCriticalPath[I] ) //addition chained with multiplication
                                    cur_InstructionCriticalPath[I] = cur_InstructionCriticalPath[I_Pred] ;
                                Chained = 1;
                            }
                            else
                            {
                                // *Evaluating_log << "        --------- Evaluated Instruction critical path for Instruction: <<" << *I << " which cannot be chained.\n";
                                if ( cur_InstructionCriticalPath[I_Pred] + tmp_I_latency > cur_InstructionCriticalPath[I] ) //update the critical path
                                    cur_InstructionCriticalPath[I] = cur_InstructionCriticalPath[I_Pred] + tmp_I_latency;        
                            }

                        }                 
                    }           
                }

                // accmulate the cost of resource
                if (!Chained)
                    resourceAccmulator = resourceAccmulator + getInstructionResource(I);
            }
            


            // (3) get the maximum CP among instructions and take it as the CP of block
            if (cur_InstructionCriticalPath[I] > max_critical_path) max_critical_path = cur_InstructionCriticalPath[I];
            *Evaluating_log << "--------- Evaluated Instruction critical path for Instruction: <<" << *I <<">> and its CP is :"<< cur_InstructionCriticalPath[I] << " the resource cost is: " << (Chained?(resourceBase(0,0,0,clock_period)):(getInstructionResource(I)) );
            if (Chained)
                *Evaluating_log << "(Chained))";
            else            
                *Evaluating_log << "(Not Chained)";            
            
            *Evaluating_log << "\n";
            Evaluating_log->flush();
        }
    }

    InstructionCriticalPath_inBlock[B] = cur_InstructionCriticalPath;

    BlockLatency[B] = max_critical_path;
    BlockResource[B] = resourceAccmulator;
    *Evaluating_log << "---- Done evaluation of Block Latency for Block: " << B->getName() <<" and the latency is "<< BlockLatency[B] << " and the cost of resource is : " << resourceAccmulator <<"\n";
    BlockEvaluated.insert(B);
    return max_critical_path*1;
}

/*
    mainly used to ensure that the predecessor is in the block 
*/
bool HI_NoDirectiveTimingResourceEvaluation::BlockContain(BasicBlock *B, Instruction *I)
{
    return I->getParent() == B;
}

