#include "HI_ConstantDivisorOpt.h"
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
#include "llvm/Transforms/Utils/Local.h"
#include <ios>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace llvm;

// implemented based on https://reviews.llvm.org/D26819

bool HI_ConstantDivisorOpt::runOnFunction(
    llvm::Function &F) // The runOnModule declaration will overide the virtual one in ModulePass,
                       // which will be executed for each Module.
{
    print_status("Running HI_ConstantDivisorOpt pass.");
    std::set<long long> power2;
    generatedI.clear();
    op2Cnt.clear();

    for (int i = 0, pow = 1; i < 64; i++, pow *= 2)
        power2.insert(pow);

    if (F.getName().find("llvm.") != std::string::npos)
        return false;

    bool changed = 0;
    std::vector<Instruction *> DivIs;
    for (BasicBlock &B : F)
    {
        for (auto &I : B)
        {
            if (I.getOpcode() == Instruction::SDiv || I.getOpcode() == Instruction::UDiv)
            {
                DivIs.push_back(&I);
            }
        }
    }

    for (Instruction *DivI : DivIs)
    {
        if (ConstantInt *Divisor = dyn_cast<ConstantInt>(DivI->getOperand(1)))
        {
            IRBuilder<> Builder(DivI);
            Value *Quotient = NULL;
            if (DivI->getOpcode() == Instruction::SDiv)
            {
                Quotient = generateSignedDivisionByConstant(DivI->getOperand(0), Divisor, Builder);
            }
            else
            {
                Quotient =
                    generateUnsignedDivisionByConstant(DivI->getOperand(0), Divisor, Builder);
            }

            assert(Quotient && "failed to generate code for division by a constant");
            DivI->replaceAllUsesWith(Quotient);
            DivI->eraseFromParent();

            changed = true;
        }
    }

    DivisionOptLog->flush();
    return changed;
}

char HI_ConstantDivisorOpt::ID =
    0; // the ID for pass should be initialized but the value does not matter, since LLVM uses the
       // address of this variable as label instead of its value.

void HI_ConstantDivisorOpt::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.setPreservesCFG();
}

/// Generate code for signed division by a constant. Implementation follows
/// TargetLowering::BuildSDIV that replaces division with multiplication
/// by a magic number.
Value *HI_ConstantDivisorOpt::generateSignedDivisionByConstant(Value *Dividend,
                                                               ConstantInt *Divisor,
                                                               IRBuilder<> &Builder)
{
    unsigned BitWidth = Dividend->getType()->getIntegerBitWidth();
    ConstantInt *Shift;
    ConstantInt *Magics;

    APInt d = Divisor->getValue();
    APInt::ms magics = d.magic();

    Shift = Builder.getInt(APInt(BitWidth, BitWidth - 1, true));
    Magics = Builder.getInt(APInt(BitWidth, magics.m.getSExtValue(), true));

    // Multiply the numerator (operand 0) by the magic value
    Value *Q = Builder.CreateMul(Dividend, Magics);

    // If d > 0 and m < 0, add the numerator
    if (d.isStrictlyPositive() && magics.m.isNegative())
    {
        Q = Builder.CreateAdd(Q, Dividend);
    }

    // If d < 0 and m > 0, subtract the numerator.
    if (d.isNegative() && magics.m.isStrictlyPositive())
    {
        Q = Builder.CreateSub(Q, Dividend);
    }

    // Shift right algebraic if shift value is nonzero
    if (magics.s > 0)
    {
        ConstantInt *MagicsShift = Builder.getInt(APInt(BitWidth, magics.s, true));
        Q = Builder.CreateAShr(Q, MagicsShift);
    }

    // Extract the sign bit and add it to the quotient
    Value *T = Builder.CreateLShr(Q, Shift);
    Q = Builder.CreateAdd(Q, T);

    return Q;
}

/// Generate code for unsigned division by a constant. Implementation follows
/// TargetLowering::BuildUDIV that replaces division with multiplication
/// by a magic number.
Value *HI_ConstantDivisorOpt::generateUnsignedDivisionByConstant(Value *Dividend,
                                                                 ConstantInt *Divisor,
                                                                 IRBuilder<> &Builder)
{
    unsigned BitWidth = Dividend->getType()->getIntegerBitWidth();

    APInt d = Divisor->getValue();
    APInt::mu magics = d.magicu();

    Value *Q = Dividend;

    // If the divisor is even, we can avoid using the expensive fixup by shifting
    // the divided value upfront.
    if (magics.a != 0 && !d[0])
    {
        unsigned s = d.countTrailingZeros();
        ConstantInt *Shift = Builder.getInt(APInt(BitWidth, s, false));
        Q = Builder.CreateLShr(Q, Shift);

        // Get magic number for the shifted divisor.
        magics = d.lshr(s).magicu(s);
        assert(magics.a == 0 && "Should use cheap fixup now");
    }

    // Multiply the numerator (operand 0) by the magic value
    ConstantInt *Magics = Builder.getInt(APInt(BitWidth, magics.m.getZExtValue(), false));
    Q = Builder.CreateMul(Q, Magics);

    if (magics.a == 0)
    {
        assert(magics.s < d.getBitWidth() && "We shouldn't generate an undefined shift!");
        ConstantInt *Shift = Builder.getInt(APInt(BitWidth, magics.s, false));
        Q = Builder.CreateLShr(Q, Shift);
    }
    else
    {
        Value *NPQ = Builder.CreateSub(Dividend, Q);
        ConstantInt *One = Builder.getInt(APInt(BitWidth, 1, false));
        NPQ = Builder.CreateLShr(NPQ, One);
        NPQ = Builder.CreateAdd(NPQ, Q);
        ConstantInt *Shift = Builder.getInt(APInt(BitWidth, magics.s - 1, false));
        Q = Builder.CreateLShr(NPQ, Shift);
    }

    return Q;
}
