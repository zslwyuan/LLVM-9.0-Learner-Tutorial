#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_print.h"
#include "HI_MuxInsertionArrayPartition.h"

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
bool HI_MuxInsertionArrayPartition::MuxInsert(BasicBlock *B)
{
    if (DEBUG) *BRAM_log << "---- Evaluating MuxInsert for Block: " << B->getName() <<":\n";

    bool action = 1;
    bool changed = 0;
    while (action)
    {
        action = 0;
        // (1) iterate the instructions in the block
        if (B->getInstList().size()>1) // ignore block with only branch instruction
        {
            std::vector<Instruction*> LoadInsts;
            for (Instruction &rI : *B)
            {
                Instruction* I = &rI;
                if ( I->getOpcode()==Instruction::Load /* || I->getOpcode()==Instruction::Store */)
                {
                    LoadInsts.push_back(I);
                }
            }
            for (Instruction *I : LoadInsts)
            {

                // handle memory access instruction, because for BRAM access
                // apart from waiting for the address is ready, we need to 
                // wait until the BRAM ports are available
                // we assume that only Load instructions will need mux
                if ( I->getOpcode()==Instruction::Load /* || I->getOpcode()==Instruction::Store */)
                {
                    if (muxed_load.find(I) != muxed_load.end())
                        continue;
                    
                    HI_AccessInfo cur_access_info = getAccessInfoForAccessInst(I);
                    std::vector<partition_info> target_partitions;

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

                    if (target_partitions.size()>1)
                    {
                        if (target_partitions.size()>32)
                        {
                            muxWithMoreThan32 = 1;
                            std::string tmp_loop_name = LI->getLoopFor(B)->getHeader()->getParent()->getName();
                            tmp_loop_name += "-";
                            tmp_loop_name += LI->getLoopFor(B)->getHeader()->getName();
                            muxTooMuchLoopIRName = tmp_loop_name;
                            return true;
                        }
                        if (DEBUG) *BRAM_log << "----------- A Memory Access Instruction: " << *I <<" is found,\n-----------  information fot this access is:  " 
                                        << getAccessInfoForAccessInst(I) 
                                        << "\n-----------  the access is to partition num=" << target_partitions.size() << ""
                                        << "\n-----------  do the mux insertion for it\n";
                        IRBuilder<> Builder(I->getNextNode());
                        // Create the arguments vector from the my argument list
                        SmallVector<llvm::Type *,5> ArgTys;
                        SmallVector<Value*,2> ArgsForMux;
                        
                        ArgTys.push_back(I->getType());
                        ArgTys.push_back(llvm::Type::getInt32Ty(B->getContext()));

                        // Just get a void return type
                        Type *RetTy = I->getType();


                        // Create a new function with MyArgs as arguments
                        std::string funcName = "HIPartitionMux" + std::to_string(target_partitions.size()) + "_"+std::to_string(mux_cnt++);
                        Value *newF_val = B->getParent()->getParent()->getOrInsertFunction(funcName, FunctionType::get(RetTy, ArgTys, false)).getCallee();
                        Function *newF = dyn_cast<Function>(newF_val);
                        assert(newF && "the Function has to been created successfully.");
                        if (DEBUG) *BRAM_log << "----------- create mux function for it : " << newF->getName() << "\n";
                        

                        ArgsForMux.push_back(I);
                        ArgsForMux.push_back(ConstantInt::get(llvm::Type::getInt32Ty(B->getContext()), target_partitions.size()));

                        Value *newMux = Builder.CreateCall(newF_val, ArgsForMux);
                        if (DEBUG) *BRAM_log << "----------- insert mux function call: " << *newMux << " and relace the original use of the Load\n";
                        // BRAM_log->flush();
                        HI_takeOverAllUsesWith(I, newMux);
                        muxed_load.insert(I);
                        if (DEBUG) *BRAM_log << "\n\nafter mux insertion: " << *B << "\n\n";

                        action = 1;
                        changed = 1;
                        continue;

                    }

                }                   
                if (DEBUG) *BRAM_log << "\n";
                // BRAM_log->flush();
            }
        }
    }

    return changed;
}

void HI_MuxInsertionArrayPartition::HI_takeOverAllUsesWith(Value *Old, Value *New) 
{
    bool replaced = 1;
    while (replaced)
    {
        replaced = 0;
        for (auto tmp_use = Old->use_begin(), tmp_use_end = Old->use_end(); tmp_use!=tmp_use_end; tmp_use++ )
        {
            Use &U = *tmp_use;
            if (DEBUG) *BRAM_log << "----------- checking user : [" << *U.getUser() << "] of " << *Old << "\n";
            if (U.getUser() != New)
            {
                
                replaced = 1;
                U.set(New);
                if (DEBUG) *BRAM_log << "----------- replaced use : [" << *U.getUser() << "]\n";
                break;
            }                
            if (DEBUG) *BRAM_log << "\n";
        }
    }
}
