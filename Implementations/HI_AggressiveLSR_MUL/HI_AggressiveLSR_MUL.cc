#include "HI_AggressiveLSR_MUL.h"
#include "HI_print.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
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

bool HI_AggressiveLSR_MUL::runOnFunction(
    Function &F) // The runOnModule declaration will overide the virtual one in ModulePass, which
                 // will be executed for each Module.
{
    print_status("Running HI_AggressiveLSR_MUL pass.");
    ScalarEvolution &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    bool changed = false;
    bool ActionTaken = true;
    ValueVisited.clear();
    Inst_AccessRelated.clear();
    TraceMemoryAccessinFunction(F);
    while (ActionTaken)
    {
        ActionTaken = false;
        for (auto &B : F)
        {
            for (auto &I : B)
            {
                ActionTaken = LSR_Mul(&I, &SE);
                LSR_Add(&I, &SE);
                changed |= ActionTaken;
                if (ActionTaken)
                    break;
            }
            if (ActionTaken)
                break;
        }
    }

    // return false;
    return changed;
}

char HI_AggressiveLSR_MUL::ID =
    0; // the ID for pass should be initialized but the value does not matter, since LLVM uses the
       // address of this variable as label instead of its value.

void HI_AggressiveLSR_MUL::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.setPreservesCFG();
}

// check whether the instruction is Multiplication suitable for LSR
// If suitable, process it
bool HI_AggressiveLSR_MUL::LSR_Mul(Instruction *I, ScalarEvolution *SE)
{
    /*
    1.  get the incremental value by using SCEV
    2.  insert a new PHI (carefully select the initial constant)
    3.  replace multiplication with addition
    */
    if (I->getOpcode() != Instruction::Mul)
        return false;
    if (Inst_AccessRelated.find(I) == Inst_AccessRelated.end())
        return false;

    Instruction *Incr_I = find_Incremental_op(I);
    ConstantInt *Mul_Const_V = find_Constant_op(I);

    if (!Incr_I)
        return false;
    if (!Mul_Const_V)
        return false;

    // 1.  get the incremental value by using SCEV
    const SCEV *tmp_S = SE->getSCEV(I);
    const SCEVAddRecExpr *SARE = dyn_cast<SCEVAddRecExpr>(tmp_S);
    if (SARE)
    {
        if (SARE->isAffine())
        {
            if (DEBUG)
                *AggrLSRLog << *I << " --> is add rec Mul: " << *SARE << " it operand (0) "
                            << *SARE->getOperand(0) << " it operand (1) " << *SARE->getOperand(1)
                            << "\n";
            if (const SCEVConstant *start_V = dyn_cast<SCEVConstant>(SARE->getOperand(0)))
            {
                if (const SCEVConstant *step_V = dyn_cast<SCEVConstant>(SARE->getOperand(1)))
                {
                    int start_val = start_V->getAPInt().getSExtValue();
                    int step_val = step_V->getAPInt().getSExtValue();
                    APInt start_val_APInt = start_V->getAPInt();
                    APInt step_val_APInt = step_V->getAPInt();
                    if (DEBUG)
                        *AggrLSRLog << *I << " --> is being replaced. start_val_APInt="
                                    << start_val_APInt.getSExtValue()
                                    << " step_val_APInt =" << step_val_APInt.getSExtValue() << "\n";
                    AggrLSRLog->flush();
                    LSR_Process(I, start_val_APInt, step_val_APInt);
                    return true;
                }
            }
        }
    }
    return false;
}

// check whether the instruction is Multiplication suitable for LSR
// If suitable, process it
bool HI_AggressiveLSR_MUL::LSR_Add(Instruction *I, ScalarEvolution *SE)
{
    /*
    1.  get the incremental value by using SCEV
    2.  insert a new PHI (carefully select the initial constant)
    3.  replace multiplication with addition
    */
    if (I->getOpcode() != Instruction::Add)
        return false;
    if (Inst_AccessRelated.find(I) == Inst_AccessRelated.end())
        return false;

    // 1.  get the incremental value by using SCEV
    const SCEV *tmp_S = SE->getSCEV(I);
    const SCEVAddRecExpr *SARE = dyn_cast<SCEVAddRecExpr>(tmp_S);
    if (SARE)
    {
        if (SARE->isAffine())
        {
            if (DEBUG)
                *AggrLSRLog << *I << " --> is add rec Affine Add: " << *SARE << " it operand (0) "
                            << *SARE->getOperand(0) << " it operand (1) " << *SARE->getOperand(1)
                            << "\n";
            if (const SCEVConstant *start_V = dyn_cast<SCEVConstant>(SARE->getOperand(0)))
            {
                if (const SCEVConstant *step_V = dyn_cast<SCEVConstant>(SARE->getOperand(1)))
                {
                    // int start_val = start_V->getAPInt().getSExtValue();
                    // int step_val = step_V->getAPInt().getSExtValue();
                    // APInt start_val_APInt = start_V->getAPInt();
                    // APInt step_val_APInt = step_V->getAPInt();
                    // LSR_Process(I, start_val_APInt, step_val_APInt);
                    // return true;
                }
            }
        }
        if (SARE->isQuadratic())
        {
            if (DEBUG)
                *AggrLSRLog << *I << " --> is add rec Quadratic Add: " << *SARE
                            << " it operand (0) " << *SARE->getOperand(0) << " it operand (1) "
                            << *SARE->getOperand(1) << " it operand (2) " << *SARE->getOperand(2)
                            << "\n";
            if (const SCEVConstant *start_V = dyn_cast<SCEVConstant>(SARE->getOperand(0)))
            {
                if (const SCEVConstant *step_V = dyn_cast<SCEVConstant>(SARE->getOperand(1)))
                {
                    // int start_val = start_V->getAPInt().getSExtValue();
                    // int step_val = step_V->getAPInt().getSExtValue();
                    // APInt start_val_APInt = start_V->getAPInt();
                    // APInt step_val_APInt = step_V->getAPInt();
                    // LSR_Process(I, start_val_APInt, step_val_APInt);
                    // return true;
                }
            }
        }
    }
    return false;
}

// replace the original MUL with PHI and Add operator
void HI_AggressiveLSR_MUL::LSR_Process(Instruction *Mul_I, APInt start_val, APInt step_val)
{
    /*
    1.  get the incremental value by using SCEV
    2.  insert a new PHI (carefully select the initial constant)
    3.  replace multiplication with addition
    */
    Instruction *Inst_I = find_Incremental_op(Mul_I);
    PHINode *PHI_I = byPassBack_BitcastOp_findPHINode(Inst_I);
    if (DEBUG)
        *AggrLSRLog << "find the PHINode: [" << *PHI_I << "] for Mul: [" << *Mul_I << "]\n";

    std::string LSR_PHI_Name = Mul_I->getName();
    LSR_PHI_Name += ".PHI";
    std::string LSR_Add_Name = Mul_I->getName();
    LSR_Add_Name += ".Add";
    IRBuilder<> Builder(Mul_I->getParent()->getFirstNonPHI());
    // BasicBlock *CurBlock = Mul_I->getParent();

    // 2.  insert a new PHI (carefully select the initial constant)
    PHINode *PHI_I_for_LSR_Mul = Builder.CreatePHI(Mul_I->getType(), 2, LSR_PHI_Name);
    Constant *step_Value = ConstantInt::get(Mul_I->getType(), step_val);
    Constant *start_Value = ConstantInt::get(Mul_I->getType(), start_val - step_val);

    Value *Add_I_for_LSR_Mul = Builder.CreateAdd(PHI_I_for_LSR_Mul, step_Value, LSR_Add_Name);

    for (int i = 0; i < PHI_I->getNumOperands(); i++)
    {
        if (auto con_val = dyn_cast<ConstantInt>(PHI_I->getOperand(i)))
        {
            PHI_I_for_LSR_Mul->addIncoming(start_Value, PHI_I->getIncomingBlock(i));
        }
    }

    for (int i = 0; i < PHI_I->getNumOperands(); i++)
    {
        if (auto opI_val = dyn_cast<Instruction>(PHI_I->getOperand(i)))
        {
            PHI_I_for_LSR_Mul->addIncoming(Add_I_for_LSR_Mul, PHI_I->getIncomingBlock(i));
        }
    }

    if (DEBUG)
        *AggrLSRLog << "create the LSR PHINode: [" << *PHI_I_for_LSR_Mul << "] for Mul: [" << *Mul_I
                    << "]\n";
    if (DEBUG)
        *AggrLSRLog << "create the LSR Add: [" << *Add_I_for_LSR_Mul << "] for Mul: [" << *Mul_I
                    << "]\n\n\n";

    // 3.  replace multiplication with addition
    Mul_I->replaceAllUsesWith(Add_I_for_LSR_Mul);
    Mul_I->eraseFromParent();

    AggrLSRLog->flush();
}

// find the instruction operand of the Mul operation
Instruction *HI_AggressiveLSR_MUL::find_Incremental_op(Instruction *Mul_I)
{
    for (int i = 0; i < Mul_I->getNumOperands(); i++)
    {
        if (auto res_I = dyn_cast<Instruction>(Mul_I->getOperand(i)))
            return res_I;
    }
    return nullptr;
}

// find the constant operand of the Mul operation
ConstantInt *HI_AggressiveLSR_MUL::find_Constant_op(Instruction *Mul_I)
{
    for (int i = 0; i < Mul_I->getNumOperands(); i++)
    {
        if (auto res_I = dyn_cast<ConstantInt>(Mul_I->getOperand(i)))
            return res_I;
    }
    return nullptr;
}

// check the memory access in the function
void HI_AggressiveLSR_MUL::TraceMemoryAccessinFunction(Function &F)
{
    if (F.getName().find("llvm.") != std::string::npos) // bypass the "llvm.xxx" functions..
        return;
    findMemoryAccessin(&F);
}

// find the array access in the function F and trace the accesses to them
void HI_AggressiveLSR_MUL::findMemoryAccessin(Function *F)
{
    if (DEBUG)
        *AggrLSRLog << "checking the Memory Access information in Function: " << F->getName()
                    << "\n";
    ValueVisited.clear();

    // for general function in HLS, arrays in functions are usually declared with alloca instruction
    for (auto &B : *F)
    {
        for (auto &I : B)
        {
            if (IntToPtrInst *ITP_I = dyn_cast<IntToPtrInst>(&I))
            {
                if (DEBUG)
                    *AggrLSRLog << "find a IntToPtrInst: [" << *ITP_I
                                << "] backtrace to its operands.\n";
                TraceAccessForTarget(ITP_I);
            }
        }
    }
    if (DEBUG)
        *AggrLSRLog << "-------------------------------------------------"
                    << "\n\n\n\n";
    AggrLSRLog->flush();
}

// find out which instrctuins are related to the array, going through PtrToInt, Add, IntToPtr,
// Store, Load instructions
void HI_AggressiveLSR_MUL::TraceAccessForTarget(Value *cur_node)
{
    if (DEBUG)
        *AggrLSRLog << "looking for the operands of " << *cur_node << "\n";
    if (Instruction *tmpI = dyn_cast<Instruction>(cur_node))
    {
        Inst_AccessRelated.insert(tmpI);
    }
    else
    {
        return;
    }

    Instruction *curI = dyn_cast<Instruction>(cur_node);
    AggrLSRLog->flush();

    // we are doing DFS now
    if (ValueVisited.find(cur_node) != ValueVisited.end())
        return;

    ValueVisited.insert(cur_node);

    // Trace the uses of the pointer value or integer generaed by PtrToInt
    for (int i = 0; i < curI->getNumOperands(); ++i)
    {
        Value *tmp_op = curI->getOperand(i);
        TraceAccessForTarget(tmp_op);
    }
    ValueVisited.erase(cur_node);
}

// trace back to find the original PHI operator, bypassing SExt and ZExt operations
// according to which, we can generate new PHI node for the MUL operation
PHINode *HI_AggressiveLSR_MUL::byPassBack_BitcastOp_findPHINode(Value *cur_I_value)
{
    auto cur_I = dyn_cast<Instruction>(cur_I_value);
    assert(cur_I && "This should be an instruction.\n");
    // For ZExt/SExt Instruction, we do not need to consider those constant bits
    if (cur_I->getOpcode() == Instruction::ZExt || cur_I->getOpcode() == Instruction::SExt)
    {
        if (auto next_I = dyn_cast<Instruction>(cur_I->getOperand(0)))
        {
            return byPassBack_BitcastOp_findPHINode(next_I);
        }
        else
        {
            assert(false && "Predecessor of bitcast operator should be found.\n");
        }
    }
    else
    {
        if (auto BOI = dyn_cast<BinaryOperator>(cur_I))
        {
            auto next_I0 = dyn_cast<Instruction>(cur_I->getOperand(0));
            auto next_I1 = dyn_cast<Instruction>(cur_I->getOperand(1));
            if (next_I0 && !next_I1)
                return byPassBack_BitcastOp_findPHINode(next_I0);
            if (next_I1 && !next_I0)
                return byPassBack_BitcastOp_findPHINode(next_I1);

            llvm::errs() << *cur_I << "\n";
            assert(false && "Fail to find the PHI for this instruction.\n");
            return nullptr;
        }
        else if (auto PHI_I = dyn_cast<PHINode>(cur_I))
        {
            return PHI_I;
        }
        else
        {
            return nullptr;
        }
    }
}
