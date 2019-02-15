#ifndef _HI_DEPENDENCELIST
#define _HI_DEPENDENCELIST
// related headers should be included.
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_print.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>
#include <map>
#include <set>
#include <vector>

using namespace llvm;

class HI_DependenceList : public FunctionPass {
public:
    HI_DependenceList(const char* Instruction_out_file, const char* dependence_out_file ) : FunctionPass(ID) 
    {
        Instruction_Counter = 0;
        Function_Counter = 0;
        BasicBlock_Counter = 0;
        Loop_Counter = 0;
        callCounter = 0;
        Instruction_out = new raw_fd_ostream(Instruction_out_file, ErrInfo, sys::fs::F_None);
        Dependence_out = new raw_fd_ostream(dependence_out_file, ErrInfo, sys::fs::F_None);
    } // define a pass, which can be inherited from ModulePass, LoopPass, FunctionPass and etc.

    ~HI_DependenceList()
    {
        for (auto it : Instruction2User_id) 
        {   
            delete it.second;
        }   
        for (auto it : Blcok2InstructionList_id) 
        {   
            delete it.second;
        }
        for (auto it : Block_Successors) 
        {   
            delete it.second;
        }
        for (auto it : Block_Predecessors) 
        {   
            delete it.second;
        }
        Instruction_out->flush(); delete Instruction_out;
        Dependence_out->flush(); delete Dependence_out;
    }

    virtual bool doInitialization(Module &M)
    {
        print_status("Initilizing HI_DependenceList pass.");  
        for (auto it : Instruction2Pre_id) 
        {   
            delete it.second;
        } 
        for (auto it : Instruction2User_id) 
        {   
            delete it.second;
        }   
        for (auto it : Blcok2InstructionList_id) 
        {   
            delete it.second;
        }
        for (auto it : Block_Successors) 
        {   
            delete it.second;
        }
        for (auto it : Block_Predecessors) 
        {   
            delete it.second;
        }
        Function_id.clear();
        Instruction_id.clear();
        InstructionsNameSet.clear();
        BasicBlock_id.clear();
        Loop_id.clear();
        Block_Successors.clear();
        Block_Predecessors.clear();
        
        Instruction2Blcok_id.clear();
        Instruction2User_id.clear();
        Instruction2Pre_id.clear();
        Blcok2InstructionList_id.clear();

        return false;
    }

    virtual bool doFinalization(Module &M)
    {
        InstructionsNameSet.clear();
        *Instruction_out << "\n\n\n\n\n=================== Instruction Set and Example ======================\n\n";
        for (auto &F : M)
        for (auto &B : F)
        for (auto &I : B)
        {
            if (InstructionsNameSet.find(std::string(I.getOpcodeName())) == InstructionsNameSet.end())
            {
                *Instruction_out<<"I-ID:"<< Instruction_id[&I] << "-->" << I.getOpcodeName()  << "\n";
                InstructionsNameSet.insert(std::string(I.getOpcodeName()));
            }            
        }
        return false;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const;
    virtual bool runOnFunction(Function &M);
    static char ID;

    // find all the successors of the block and map them
    void checkSuccessorsOfBlock(BasicBlock *B);

    // find all the predecessors of the block and map them
    void checkPredecessorsOfBlock(BasicBlock *B);

    // find the dependence between instructions and map them
    void checkInstructionDependence(Instruction *I);

    int callCounter;
    int Instruction_Counter;
    int Function_Counter;
    int BasicBlock_Counter;
    int Loop_Counter;

    Function* TargeFunction;

    std::map<Function*, int> Function_id;
    std::map<Instruction*, int> Instruction_id;
    std::set<std::string> InstructionsNameSet;
    std::map<BasicBlock*, int> BasicBlock_id;
    std::map<Loop*, int> Loop_id;
    std::map<BasicBlock*, std::vector<BasicBlock*>*> Block_Successors;
    std::map<BasicBlock*, std::vector<BasicBlock*>*> Block_Predecessors;
    
    std::map<int, int> Instruction2Blcok_id;
    std::map<int, std::vector<int>*> Instruction2User_id;
    std::map<int, std::vector<int>*> Instruction2Pre_id;
    std::map<int, std::vector<int>*> Blcok2InstructionList_id;

    std::error_code ErrInfo;
    raw_ostream *Instruction_out;
    raw_ostream *Dependence_out;

};



#endif
