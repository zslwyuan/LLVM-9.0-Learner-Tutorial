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
#include "HI_IR2SourceCode.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>
#include "HI_StringProcess.h"

using namespace llvm;
 
bool HI_IR2SourceCode::runOnModule(Module &M) // The runOnModule declaration will overide the virtual one in ModulePass, which will be executed for each Module.
{
    if (DEBUG) *IR2Src_Log << M;
    if (DEBUG) *IR2Src_Log << "\n\n============================================================\n\n\n";

    auto subprogramSet = getAllSubprogram(M);
    for (auto &F : M)
    {
        if (F.getName().find("llvm.")!=std::string::npos)
            continue;
        if (DEBUG) *IR2Src_Log << "processing IR code of Function : [" << F.getName() << "]\n";
        auto &LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
        auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(F).getSE();
        traceInstructionSourceCode(F);
        traceFunctionSourceCode(F);
    }

    for (auto subprogram : subprogramSet)
    {
        if (DEBUG) *IR2Src_Log << "     Processing SubProgram: " << subprogram->getName() << " \n";
        for (auto &F : M)
        {
            if (F.getName().find("llvm.")!=std::string::npos)
                continue;
            if (DEBUG) *IR2Src_Log << "\n\n============================================================\n";
            if (DEBUG) *IR2Src_Log << "============================================================\n";
            if (DEBUG) *IR2Src_Log << "============================================================\n";

            if (DEBUG) *IR2Src_Log << "processing IR code of Function : [" << F.getName() << "]\n";

            if (DEBUG) *IR2Src_Log << "============================================================\n";
            if (DEBUG) *IR2Src_Log << "============================================================\n";
            if (DEBUG) *IR2Src_Log << "============================================================\n";

            auto &LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
            auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(F).getSE();

            Block2Line.clear();
            Block2Line_forLoop.clear();

            traceBasicBlockSourceCode(LI, F, subprogram);
            if (DEBUG) *IR2Src_Log << "\n\n============================================================\n\n\n";

            traceLoopSourceCode(LI, SE, F, subprogram);   
            if (DEBUG) *IR2Src_Log << "\n\n============================================================\n\n\n";

        }

        if (DEBUG) *IR2Src_Log << "\n\n============================================================\n\n\n";
        if (DEBUG) *IR2Src_Log << "mappingLoopIR2LoopLabel\n\n============================================================\n\n\n";
        mappingLoopIR2LoopLabel(subprogram);
        if (DEBUG) *IR2Src_Log << "\n\n============================================================\n\n\n";

        IR2Src_Log->flush();
    }
   

    for (auto funcName2beginline_pair : IRFunc2BeginLine)
    {
        if (DEBUG) *IR2Src_Log << "printing function: [" << funcName2beginline_pair.first << "] 's possible beginlines:\n";
        for (auto line : funcName2beginline_pair.second)
        {
            if (DEBUG) *IR2Src_Log << "      line--->" << line << "\n";
        }
        if (DEBUG) *IR2Src_Log << "\n";
    }



    for (auto itLine : Loop2Line)
    {
        std::string tmp_loop_name = itLine.first->getParent()->getName();
        tmp_loop_name += "-";
        tmp_loop_name += itLine.first->getName();
        if (IRLoop2LoopLabel.find(tmp_loop_name) == IRLoop2LoopLabel.end())
        {
            print_warning(tmp_loop_name + " is not mapped to loop label in source code.\n");
        }
    }
    IR2Src_Log->flush();
    return false;
}



char HI_IR2SourceCode::ID = 0;  // the ID for pass should be initialized but the value does not matter, since LLVM uses the address of this variable as label instead of its value.

void HI_IR2SourceCode::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.setPreservesCFG();
}

void HI_IR2SourceCode::traceInstructionSourceCode(Function &F)
{
    for (auto &B : F)
    {
        for(auto &I : B) 
        {
            if (DEBUG) *IR2Src_Log << "Instruction: " << I;
            SmallVector<std::pair<unsigned, MDNode *>, 4> I_MDs;
            I.getAllMetadata(I_MDs);
            for (auto &MD : I_MDs) 
            {
                if (MDNode *N = MD.second) 
                {
                    if (DEBUG) *IR2Src_Log << " ==>  Details: " << *N;
                    if (auto DILoc = dyn_cast<DILocation>(N) )
                    {
                        // if  (DILoc->getInlinedAt() != nullptr)
                        // {
                        //     continue;
                        //     DILoc = dyn_cast<DILocation>(DILoc->getInlinedAt());
                        // }                        
                        if (DEBUG) *IR2Src_Log << " ==>  SourceRange: " << DILoc->getDirectory() << "/" << DILoc->getFilename() << ":" <<DILoc->getLine();
                    }
                }
            }
            if (DEBUG) *IR2Src_Log << "\n";
        } 
    }
}


void HI_IR2SourceCode::traceBasicBlockSourceCode(LoopInfo &LI, Function &F, DISubprogram* subprog)
{
    for (auto &B : F)
    {
        if (DEBUG) *IR2Src_Log << "BasicBlock: " << B.getName() << ": SourceRange = ";
        std::string path = "";
        int begin_line = 1000000;
        int end_line = 1;
        int begin_line_for_loop = 1000000;
        int end_line_for_loop = 1;
        for(auto &I : B) 
        {

            SmallVector<std::pair<unsigned, MDNode *>, 4> I_MDs;
            I.getAllMetadata(I_MDs);
            for (auto &MD : I_MDs) 
            {
                if (MDNode *N = MD.second) 
                {
                    if (auto DILoc = dyn_cast<DILocation>(N) )
                    {

                        if (subprog != DILoc->getScope()->getSubprogram())
                            continue;

                        if (path == "")
                        {
                            path = DILoc->getDirectory().str() + "/" + DILoc->getFilename().str();
                        }

                        if (auto callI = dyn_cast<CallInst>(&I))
                        {
                            if (callI->getCalledFunction()->getName().str().find("llvm.") != std::string::npos)
                            {
                                break;
                            }
                        }

                        if (DILoc->getLine() > end_line)
                            end_line = DILoc->getLine();

                        if (DILoc->getLine() < begin_line && DILoc->getLine()>0)
                            begin_line = DILoc->getLine();

                        if (I.getOpcode() == Instruction::ICmp)
                        {
                            if (DILoc->getLine() > end_line_for_loop)
                                end_line_for_loop = DILoc->getLine();

                            if (DILoc->getLine() < begin_line_for_loop && DILoc->getLine()>0)
                                begin_line_for_loop = DILoc->getLine();
                        }
                    }
                }
            }
        } 
        Block2Line[&B] = std::pair<int, int> (begin_line, end_line);
        Block2Line_forLoop[&B] = std::pair<int, int> (begin_line_for_loop, end_line_for_loop);
        Block2Path[&B] = path;
        if (DEBUG) *IR2Src_Log << path << ":" << begin_line << "--" << end_line << "\n";
        if (DEBUG) *IR2Src_Log << "LineCheckForLoop:" << begin_line_for_loop << "--" << end_line_for_loop << "\n";
        if (DEBUG) *IR2Src_Log << "BasicBlock: " << B.getName() << ": for subProg: " << subprog->getName() << "\n";
        for(auto &I : B) 
        {
            if (DEBUG) *IR2Src_Log << "  " << I << ":  line:";
            SmallVector<std::pair<unsigned, MDNode *>, 4> I_MDs;
            I.getAllMetadata(I_MDs);
            for (auto &MD : I_MDs) 
            {
                if (MDNode *N = MD.second) 
                {
                    if (auto DILoc = dyn_cast<DILocation>(N) )
                    {

                        if (DEBUG) *IR2Src_Log << DILoc->getLine();
                        auto scopeFuncName = DILoc->getScope()->getSubprogram()->getName();
                        if (DEBUG) *IR2Src_Log << "  scopeFuncName:" << scopeFuncName;               
                    }                    
                }
            }
            if (DEBUG) *IR2Src_Log << "\n";
        } 
        if (DEBUG) *IR2Src_Log << "\n\n";
    }   
}

void HI_IR2SourceCode::traceLoopSourceCode(LoopInfo &LI, ScalarEvolution &SE, Function &F, DISubprogram* subprog)
{
    for (auto cur_Loop : LI.getLoopsInPreorder())
    {
        if (DEBUG) *IR2Src_Log << "Loop: " << cur_Loop->getName() << " cur_Loop->getStartLoc()=line: " << cur_Loop->getStartLoc().getLine() << " includes basic blocks:\n   ";
        std::string path = "";
        int begin_line = 1000000;
        int end_line = 1;

        std::set<BasicBlock*> Loop_Blocks;
        for (auto B : cur_Loop->getBlocks())
        {
            Loop_Blocks.insert(B);
        }

        int tmp_begin_line = 0;
        if (DEBUG) *IR2Src_Log << *cur_Loop << "\n";
        for (auto &I : *cur_Loop->getHeader())
        {
            if (auto PHI_I = dyn_cast<PHINode>(&I))
            {
                for (int i=0;i<PHI_I->getNumIncomingValues();i++)
                {
                    if (auto in_I = dyn_cast<Instruction>(PHI_I->getIncomingValue(i)))
                    {
                        if (cur_Loop->contains(in_I->getParent()))
                        {
                          //  auto Br_I = dyn_cast<BranchInst>(in_I->getParent()->getTerminator());
                            // if (Br_I->getName().find("indvars") != std::string::npos)
                            if (DEBUG) *IR2Src_Log << "  check feedback Instruction: " << *in_I << ": SCEV:" <<  *SE.getSCEV(in_I) << "\n";
                            const SCEVAddRecExpr* tmp_S = dyn_cast<SCEVAddRecExpr>(bypassExtTruntSCEV(SE.getSCEV(in_I)));
                            if (tmp_S)
                            {
                                if (tmp_S->getLoop() == cur_Loop)
                                {
                                    const SCEVConstant *constS = dyn_cast<SCEVConstant>(bypassExtTruntSCEV(tmp_S->getStepRecurrence(SE)));
                                    if (constS)
                                    {
                                        if (constS->getAPInt().getSExtValue() == 1)
                                        {
                                            SmallVector<std::pair<unsigned, MDNode *>, 4> I_MDs;
                                            in_I->getAllMetadata(I_MDs);
                                            for (auto &MD : I_MDs) 
                                            {
                                                if (MDNode *N = MD.second) 
                                                {
                                                    if (auto DILoc = dyn_cast<DILocation>(N) )
                                                    {
                                                        if (DEBUG) *IR2Src_Log << "  recognize feedback Instruction: " << *in_I << ":  line:";
                                                        if (DEBUG) *IR2Src_Log << DILoc->getLine();
                                                        auto scopeFuncName = DILoc->getScope()->getSubprogram()->getName();
                                                        if (DEBUG) *IR2Src_Log << "  scopeFuncName:" << scopeFuncName << "\n"; 
                                                        if (tmp_begin_line < DILoc->getLine() ) 
                                                        {                                            
                                                            tmp_begin_line = DILoc->getLine();             
                                                            if (DEBUG) *IR2Src_Log << "   update tmp_begin_line to " << tmp_begin_line << "\n"; 
                                                        }
                                                    }                    
                                                }
                                            }
                                        }
                                    }                                    
                                }
                            }
                        }
                    }
                }
            }
        }
        if (!tmp_begin_line)
        {
            for (auto &I : *cur_Loop->getHeader())
            {
                if (auto PHI_I = dyn_cast<PHINode>(&I))
                {
                    for (int i=0;i<PHI_I->getNumIncomingValues();i++)
                    {
                        if (auto in_I = dyn_cast<Instruction>(byPassBitcastOp(PHI_I->getIncomingValue(i))))
                        {
                            if (cur_Loop->contains(in_I->getParent()))
                            {
                            //  auto Br_I = dyn_cast<BranchInst>(in_I->getParent()->getTerminator());
                                // if (Br_I->getName().find("indvars") != std::string::npos)
                                if (DEBUG) *IR2Src_Log << "  check feedback Instruction: " << *in_I << ": SCEV:" <<  *SE.getSCEV(in_I) << "\n";
                                if (useForCmp(in_I))
                                {
                                    SmallVector<std::pair<unsigned, MDNode *>, 4> I_MDs;
                                    in_I->getAllMetadata(I_MDs);
                                    for (auto &MD : I_MDs) 
                                    {
                                        if (MDNode *N = MD.second) 
                                        {
                                            if (auto DILoc = dyn_cast<DILocation>(N) )
                                            {
                                                if (DEBUG) *IR2Src_Log << "  recognize feedback Instruction: " << *in_I << ":  line:";
                                                if (DEBUG) *IR2Src_Log << DILoc->getLine();
                                                auto scopeFuncName = DILoc->getScope()->getSubprogram()->getName();
                                                if (DEBUG) *IR2Src_Log << "  scopeFuncName:" << scopeFuncName << "\n"; 
                                                if (tmp_begin_line < DILoc->getLine() ) 
                                                {                                            
                                                    tmp_begin_line = DILoc->getLine();             
                                                    if (DEBUG) *IR2Src_Log << "   update tmp_begin_line to " << tmp_begin_line << "\n"; 
                                                }
                                            }                    
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }


        if (tmp_begin_line>0)
            begin_line = tmp_begin_line;

        for (auto B : cur_Loop->getBlocks())
        {
            
            // if (cur_Loop != LI.getLoopFor(B))
            //     continue;
            if (DEBUG) *IR2Src_Log << B->getName() << ": " << Block2Line[B].first << "--" << Block2Line[B].second << "\n";
            if (path == "")
            {
                path = Block2Path[B];
            }

            if (Block2Line[B].second > end_line)
                end_line = Block2Line[B].second;

            // if (Block2Line_forLoop[B].first < begin_line)
            //     begin_line = Block2Line_forLoop[B].first;
        }
        if (DEBUG) *IR2Src_Log << "\n";
        if (DEBUG) *IR2Src_Log << "====SourceRang: " << path << ":" << begin_line << "--" << end_line << "\n\n";
        Loop2Line[cur_Loop->getHeader()] = std::pair<int, int>(begin_line, end_line);
        Loop2Path[cur_Loop->getHeader()] = path;

        bool duplicated = 0;
        for (auto tmp_line : IRFunc2BeginLine[demangleFunctionName(F.getName())])
        {
            if (tmp_line == begin_line)
            {
                duplicated = 1;
                break;
            }
        }

        if (!duplicated)
            IRFunc2BeginLine[demangleFunctionName(F.getName())].push_back(begin_line);

        std::string tmp_loop_name = cur_Loop->getHeader()->getParent()->getName();
        tmp_loop_name += "-";
        tmp_loop_name += cur_Loop->getHeader()->getName();
        
        IRLoop2OriginTripCount[tmp_loop_name] = SE.getSmallConstantMaxTripCount(cur_Loop);
    }

}


void HI_IR2SourceCode::traceFunctionSourceCode(Function &F)
{
    SmallVector<std::pair<unsigned, MDNode *>, 4> MDs;

    if (DEBUG) *IR2Src_Log << "Function: " << F.getName() << " includes basic blocks:\n   ";
    std::string path = "";
    int begin_line = 1000000;
    int end_line = 1;
    
    for (auto &B : F)
    {
        for(auto &I : B) 
        {
            SmallVector<std::pair<unsigned, MDNode *>, 4> I_MDs;
            I.getAllMetadata(I_MDs);
            for (auto &MD : I_MDs) 
            {
                if (MDNode *N = MD.second) 
                {
                    if (auto DILoc = dyn_cast<DILocation>(N) )
                    {
                        if (DILoc->getLine() > end_line)
                            end_line = DILoc->getLine();

                        if (DILoc->getLine() < begin_line && DILoc->getLine()>0)
                            begin_line = DILoc->getLine();
                    }
                }
            }
            if (DEBUG) *IR2Src_Log << "\n";
        } 
    }

    F.getAllMetadata(MDs);
    for (auto &MD : MDs) 
    {
        if (MDNode *N = MD.second) 
        {
            if (DEBUG) *IR2Src_Log << "Total instructions in function " << F.getName() << " - " << *N << "\n";
            if (auto DFLoc = dyn_cast<DISubprogram>(N) )
            {
                bool duplicated = 0;
                for (auto tmp_line : IRFunc2BeginLine[demangleFunctionName(F.getName())])
                {
                    if (tmp_line == DFLoc->getLine())
                    {
                        duplicated = 1;
                        break;
                    }
                }
                if (duplicated)
                    continue;

                IRFunc2BeginLine[demangleFunctionName(F.getName())].push_back(DFLoc->getLine());
                if (begin_line > DFLoc->getLine() && DFLoc->getLine()>0)
                    begin_line = DFLoc->getLine();
                
            }
        }
    }
    if (DEBUG) *IR2Src_Log << "\n";
    if (DEBUG) *IR2Src_Log << "    SourceRang: " << path << ":" << begin_line << "--" << end_line << "\n\n";

    Function2Line[&F] = std::pair<int, int>(begin_line, end_line);
    
    bool duplicated = 0;
    for (auto tmp_line : IRFunc2BeginLine[demangleFunctionName(F.getName())])
    {
        if (tmp_line == begin_line)
        {
            duplicated = 1;
            break;
        }
    }

    if (!duplicated)
        IRFunc2BeginLine[demangleFunctionName(F.getName())].push_back(begin_line);
    
    Function2Path[&F] = path;
}

void HI_IR2SourceCode::traceModuleMetadata(Module &M)
{
    SmallVector<std::pair<unsigned, MDNode *>, 4> MDs;

    
    // for (auto &MD : M.meta()) 
    // {
    //     if (MDNode *N = MD) 
    //     {
    //         if (DEBUG) *IR2Src_Log << "Total instructions in function " << F.getName() << " - " << *N << "\n";
    //         if (auto DFLoc = dyn_cast<DISubprogram>(N) )
    //         {
    //             if (DEBUG) *IR2Src_Log << "     DebugInfo: : " << DFLoc->getDirectory() << "/" << DFLoc->getFilename() << ":" << DFLoc->getLine() << "\n";
    //         }
    //         else if (auto DLLoc = dyn_cast<DILabel>(N))
    //         {
    //             if (DEBUG) *IR2Src_Log << "     LoopLable: : " << DLLoc->getFile() << ":" << DLLoc->getLine() << "\n";
    //         }
    //     }
    // }
}

void HI_IR2SourceCode::mappingLoopIR2LoopLabel(DISubprogram* subprogram)
{
    auto DFLoc = subprogram;

    if (DEBUG) *IR2Src_Log << "     DebugInfo: : " << DFLoc->getDirectory() << "/" << DFLoc->getFilename() << ":" << DFLoc->getLine() << "\n";
    for (auto RMD : DFLoc->getRetainedNodes())
    {                
        if (auto DLLoc = dyn_cast<DILabel>(RMD))
        {
            if (DEBUG) *IR2Src_Log << "           LoopLable: " << DLLoc->getName() <<" : " << DLLoc->getFile()->getDirectory() << "/" << DLLoc->getFile()->getName() << ":" << DLLoc->getLine() << "\n";
            std::string tmp_path(DFLoc->getDirectory());
            tmp_path +=  "/";
            tmp_path += DFLoc->getFilename();
            std::string tmp_label = DLLoc->getName();
            bool find = 0;
            for (auto itLine : Loop2Line)
            {
                if (itLine.second.first == DLLoc->getLine() && Loop2Path[itLine.first] == tmp_path)
                {
                    if (!find)
                    {
                        if (DEBUG) *IR2Src_Log << "               mapping to IR loop: " << itLine.first->getName() << "in Function : " << itLine.first->getParent()->getName() << "\n";
                        std::string tmp_loop_name = itLine.first->getParent()->getName();
                        tmp_loop_name += "-";
                        tmp_loop_name += itLine.first->getName();
                        IRLoop2LoopLabel[tmp_loop_name] = DLLoc->getName();
                        find = 1;
                    }
                    else
                    {
                        std::string tmp_loop_name = itLine.first->getParent()->getName();
                        tmp_loop_name += "-";
                        tmp_loop_name += itLine.first->getName();
                        llvm::errs() << "Loop 1 (now): " << tmp_loop_name << "\n";
                        for (auto irloop_label : IRLoop2LoopLabel)
                        {
                            if (irloop_label.second == DLLoc->getName())
                            {
                                tmp_loop_name = irloop_label.first;
                                break;
                            }
                        }
                        llvm::errs() << "Loop 2 (previous): " << tmp_loop_name << "\n";
                        assert(false && "They are mapped to the same loop label. One IR loop <==> One label.");
                    }
                    
                }
            }
            IR2Src_Log->flush();
            if (!find)
            {
                print_warning("Mapping for each loop label should be found but the loop with label=" 
                    + tmp_label 
                    + " does not get mapped. This might be caused by the dead loop elimination when doing IR optimization and prediction.");
            }
            // assert(find && "mapping for each loop label should be found!!!");
        }                        
    }                    
}

std::set<DISubprogram*> HI_IR2SourceCode::getAllSubprogram(llvm::Module &M)
{
    std::set<DISubprogram*> res;
    for (auto &F : M)
    {
        if (F.getName().find("llvm.")!=std::string::npos)
            continue;

        auto &LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
        auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(F).getSE();

        for (auto &B : F)
        {
            for(auto &I : B) 
            {
                SmallVector<std::pair<unsigned, MDNode *>, 4> I_MDs;
                I.getAllMetadata(I_MDs);
                for (auto &MD : I_MDs) 
                {
                    if (MDNode *N = MD.second) 
                    {
                        if (auto DILoc = dyn_cast<DILocation>(N) )
                        {
                            if (DILoc->getScope())
                            {
                                if (DILoc->getScope()->getSubprogram())
                                {
                                    if (DILoc->getScope()->getSubprogram()->getName() != "")
                                    {
                                        if (res.find(DILoc->getScope()->getSubprogram())==res.end())
                                        {
                                            if (DEBUG) *IR2Src_Log << "     find (sub) function in original source code: " << DILoc->getScope()->getSubprogram()->getName() << " \n";
                                            res.insert(DILoc->getScope()->getSubprogram());
                                        }
                                    }

                                }
                            }
                        }
                    }
                }
            } 
        }
    }
    return res;
}

// check whether the instruction is used by a ICmp instruction
bool HI_IR2SourceCode::useForCmp(llvm::Instruction *I)
{
    for (auto tmpUser : I->users())
    {
        auto tmpI = dyn_cast<Instruction>(tmpUser);
        if (auto tmp_icmp = dyn_cast<ICmpInst>(tmpI))
        {
            if (tmp_icmp)
            return true;
        }
        else if (Instruction::isCast(tmpI->getOpcode()))
        {
            if (useForCmp(tmpI))
                return true;
        }
    }
    return false;
}

const SCEV* HI_IR2SourceCode::bypassExtTruntSCEV(const SCEV* inputS)
{
    if (auto castSCEV = dyn_cast<SCEVCastExpr>(inputS))
        return bypassExtTruntSCEV(castSCEV->getOperand());
    else
        return inputS;
}

// trace back to find the original operator, bypassing SExt and ZExt operations
llvm::Value* HI_IR2SourceCode::byPassBitcastOp(llvm::Value* cur_V)
{
    if (auto cur_I = dyn_cast<Instruction>(cur_V))
    {
        if (cur_I->getOpcode() == Instruction::Trunc ||cur_I->getOpcode() == Instruction::ZExt || cur_I->getOpcode() == Instruction::SExt )
        {
            if (auto next_I = dyn_cast<Instruction>(cur_I->getOperand(0)))
            {
                return byPassBitcastOp(next_I);
            }
        }
    }
    return cur_V;
}