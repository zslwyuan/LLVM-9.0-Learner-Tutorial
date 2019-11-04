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
#include "HI_Mul2Shl.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>

using namespace llvm;
 
bool HI_Mul2Shl::runOnFunction(llvm::Function &F) // The runOnModule declaration will overide the virtual one in ModulePass, which will be executed for each Module.
{
    print_status("Running HI_Mul2Shl pass."); 
    std::set<long long> power2;
    for (int i=0, pow=1;i<64;i++,pow*=2)
        power2.insert(pow);

    if (F.getName().find("llvm.") != std::string::npos)
        return false;
        
    bool changed = 0;
    for (BasicBlock &B : F) 
    {
        bool action = 1;
        while (action)
        {
            action = 0;
            for (Instruction &I: B) 
            {
                if (I.getOpcode() != Instruction::Mul)
                    continue;
                ConstantInt *CL = nullptr, *CR = nullptr, *ConstForMul;
                Value *VarForMul;
                
                CL = dyn_cast<ConstantInt>(I.getOperand(0));
                CR = dyn_cast<ConstantInt>(I.getOperand(1));
                if (!CL && !CR) 
                    continue;
                else if (CL)
                {
                    ConstForMul = CL;
                    VarForMul = I.getOperand(1);
                }
                else 
                {
                    ConstForMul = CR;
                    VarForMul = I.getOperand(0);
                }

                long long constValue = ConstForMul->getValue().getSExtValue();
                for (int i=1, pow=2;i<64;i++,pow*=2)
                {
                    if (pow/2 < constValue && pow > constValue )
                    {
                        int addVal = constValue - pow/2;
                        int subVal = pow - constValue;
                        if (power2.find(addVal) != power2.end())
                        {
                            changed = 1;
                            action = 1;
                            if (DEBUG) *Mul2ShlLog << "\n\nbefore replacement:\n\n" << B;
                            IRBuilder<> Builder(&I);
                            std::string oriName = I.getName();
                            Value* Shl0 = Builder.CreateShl(VarForMul, i-1,  oriName+".shl.0");
                            Value* Shl1 = Builder.CreateShl(VarForMul, log2int(addVal),  oriName+".shl.1");
                            Value* Add_For_Mul = Builder.CreateAdd(Shl0, Shl1,  oriName+".mul.add");
                            I.replaceAllUsesWith(Add_For_Mul);
                            I.eraseFromParent();
                            if (DEBUG) *Mul2ShlLog << "\n\nafter replacement:\n\n" << B;
                        }
                        else if (power2.find(subVal) != power2.end())
                        {
                            changed = 1;
                            action = 1;
                            if (DEBUG) *Mul2ShlLog << "\n\nbefore replacement:\n\n" << B;
                            IRBuilder<> Builder(&I);
                            std::string oriName = I.getName();
                            Value* Shl0 = Builder.CreateShl(VarForMul, i,  oriName+".shl.0");
                            Value* Shl1 = Builder.CreateShl(VarForMul, log2int(subVal),  oriName+".shl.1");
                            Value* Sub_For_Mul = Builder.CreateSub(Shl0, Shl1,  oriName+".mul.sub");                            
                            I.replaceAllUsesWith(Sub_For_Mul);
                            I.eraseFromParent();
                            if (DEBUG) *Mul2ShlLog << "\n\nafter replacement:\n\n" << B;
                        }
                        break;
                    }
                }

                if (action)
                    break;
                    
            }
        }

    }
    Mul2ShlLog->flush(); 
    return changed;
}



char HI_Mul2Shl::ID = 0;  // the ID for pass should be initialized but the value does not matter, since LLVM uses the address of this variable as label instead of its value.

void HI_Mul2Shl::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.setPreservesCFG();
}

int HI_Mul2Shl::log2int(long long val)
{
    for (int i=0, pow=1;i<64;i++,pow*=2)
    {
        if (pow==val)
            return i;
    }
    assert(false && "should not arrive here.");
}
