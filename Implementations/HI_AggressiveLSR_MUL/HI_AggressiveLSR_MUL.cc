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
#include "HI_AggressiveLSR_MUL.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>

using namespace llvm;
 
bool HI_AggressiveLSR_MUL::runOnFunction(Function &F) // The runOnModule declaration will overide the virtual one in ModulePass, which will be executed for each Module.
{
    DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    bool changed = false;

    printDominatorTree(DT);
    processedBlock.clear();
    

    // 1. loop to find the lowest node in Dominator Tree, which should not be preocessed previously, to process
    while (1)
    {   
        BasicBlock* cur_block = findUnprocessedLowestBlock(DT);
        assert(cur_block && "A unprocessed block should be found until the root is reached.\n");

        // 2. mark the block processed
        processedBlock.insert(cur_block);
        if (DT.getRoot() == cur_block)
            break;

        // 3. obtain the instruction independent with those PHI nodes
        std::vector<Instruction*> InstructionVec_forBackward = getInstructions_PhiIndependent(cur_block);

        // 4. move those specific instructions to dominator block
        for (auto node = GraphTraits<DominatorTree *>::nodes_begin(&DT); node != GraphTraits<DominatorTree *>::nodes_end(&DT); ++node) 
        {
            BasicBlock *BB = node->getBlock();
            if (BB==cur_block)
            {               
                assert(node->getIDom() && "This node should have its dominator.\n");
                BasicBlock *domB = node->getIDom()->getBlock();
                for (Instruction *I : InstructionVec_forBackward)
                {
                    changed|=transferInstructionTo(I, domB);
                }
                printFunction(&F);
                BackwardLog->flush();
                break;
            }
        }

    }

    BackwardLog->flush(); 
    return changed;
}



char HI_AggressiveLSR_MUL::ID = 0;  // the ID for pass should be initialized but the value does not matter, since LLVM uses the address of this variable as label instead of its value.

void HI_AggressiveLSR_MUL::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.setPreservesCFG();
}

// print out the graph of dominator tree
void HI_AggressiveLSR_MUL::printDominatorTree(DominatorTree &DT)
{
    *BackwardLog << "\n\n\n\n\n Printing Dominator Graph:\n";
    for (auto node = GraphTraits<DominatorTree *>::nodes_begin(&DT); node != GraphTraits<DominatorTree *>::nodes_end(&DT); ++node) 
    {
        BasicBlock *BB = node->getBlock();
        if (node->getIDom())
        {
            BasicBlock *PreBB = node->getIDom()->getBlock();
            *BackwardLog << "Block: [" << PreBB->getName() <<"] dominates Block: [" << BB->getName() <<"].\n";
        }            
    }
    BackwardLog->flush();
}

// find the lowest unprocessed block in the dominator tree
BasicBlock* HI_AggressiveLSR_MUL::findUnprocessedLowestBlock(DominatorTree &DT)
{
    *BackwardLog << "findUnprocessedLowestBlock:\n";
    for (auto node = GraphTraits<DominatorTree *>::nodes_begin(&DT); node != GraphTraits<DominatorTree *>::nodes_end(&DT); ++node) 
    {
        BasicBlock *BB = node->getBlock();
        if (processedBlock.find(BB)!=processedBlock.end())
            continue;
        bool hasUnprocessedChild = 0;
        for (auto childNode : node->getChildren())
        {
            BasicBlock *childBB = childNode->getBlock();
            if (processedBlock.find(childBB)==processedBlock.end())
            {
                hasUnprocessedChild = 1;
                break;
            }
        }
        if (!hasUnprocessedChild)
            return BB;
    }
    BackwardLog->flush();
    return nullptr;
}

// find the instructions independent with the PHI nodes
std::vector<Instruction*> HI_AggressiveLSR_MUL::getInstructions_PhiIndependent(BasicBlock *cur_block)
{
    *BackwardLog << "\ngetInstructions_PhiIndependent\n";
    isInstruction_PHI_Independent.clear();

    // mark those instruction dependent on the PHINodes
    for (auto &I : *cur_block)
    {
        if (auto PHI_I = dyn_cast<PHINode>(&I))
        {
            isInstruction_PHI_Independent.insert(&I);
        }
        else
        {
            auto cur_opcode = I.getOpcode();
            if (cur_opcode == Instruction::Ret || cur_opcode == Instruction::Br || cur_opcode == Instruction::Switch || 
                cur_opcode == Instruction::IndirectBr || cur_opcode == Instruction::Invoke || cur_opcode == Instruction::Resume || 
                cur_opcode == Instruction::Unreachable || cur_opcode == Instruction::CleanupRet || cur_opcode == Instruction::CatchRet || 
                cur_opcode == Instruction::CatchSwitch )
            {
                isInstruction_PHI_Independent.insert(&I);
            }
            if (auto callI = dyn_cast<CallInst>(&I))
            {
                for (int i = 0; i<callI->getNumArgOperands(); i++)
                {
                    if (auto tmpI = dyn_cast<Instruction>(callI->getArgOperand(i)))
                    {
                        if (tmpI->getParent()==cur_block)
                        {
                            if (isInstruction_PHI_Independent.find(tmpI)!=isInstruction_PHI_Independent.end())
                            {
                                isInstruction_PHI_Independent.insert(&I);
                            }
                        }
                    }
                }
                continue;
            }
            for (int i = 0; i<I.getNumOperands(); i++)
            {
                if (auto tmpI = dyn_cast<Instruction>(I.getOperand(i)))
                {
                    if (tmpI->getParent()==cur_block)
                    {
                        if (isInstruction_PHI_Independent.find(tmpI)!=isInstruction_PHI_Independent.end())
                        {
                            isInstruction_PHI_Independent.insert(&I);
                        }
                    }
                }
            }
        }        
    }

    std::vector<Instruction*> res;
    *BackwardLog << "\n\n\n\n\n In block: " << cur_block->getName() << ", the following instructions are independent with the PHI";
    for (auto &I : *cur_block)
    {
        if (isInstruction_PHI_Independent.find(&I)==isInstruction_PHI_Independent.end())
        {
            *BackwardLog << I << "\n";         
            res.push_back(&I);
        }
    }
    BackwardLog->flush();
    return res;
}

// move the Instruction to another block
bool HI_AggressiveLSR_MUL::transferInstructionTo(Instruction* I, BasicBlock *To_B)
{
    *BackwardLog << "moving instruction: " << *I<< " from block: " << I->getParent()->getName() << " to block: " << To_B->getName() <<"\n" ;
    Instruction* BlockEnd_I = To_B->getTerminator();
    I->moveBefore(BlockEnd_I);
    return true;
}

void HI_AggressiveLSR_MUL::printFunction(Function* F)
{
    *BackwardLog << "\n\nThe Content of Function: " << F->getName() << " is \n" <<*F;
    *BackwardLog << "\n";
}