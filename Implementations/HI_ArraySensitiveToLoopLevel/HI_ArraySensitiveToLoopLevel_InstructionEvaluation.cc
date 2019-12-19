#include "HI_ArraySensitiveToLoopLevel.h"
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
    mainly used to get the latency of an instruction
*/
// HI_ArraySensitiveToLoopLevel::timingBase
// HI_ArraySensitiveToLoopLevel::getInstructionLatency(Instruction *I)
// {
//     timingBase result(0,0,1,clock_period);

//     ////////////////////////////// Cast Operations /////////////////////////
//     if (PtrToIntInst *PTI = dyn_cast<PtrToIntInst>(I)) // such operation like trunc/ext will not
//     cost extra timing on FPGA
//     {
//         return result;
//     }
//     else if (IntToPtrInst *ITP = dyn_cast<IntToPtrInst>(I))
//     {
//         return result;
//     }
//     else if (FPToUIInst *FTU = dyn_cast<FPToUIInst>(I))
//     {
//         result =
//         get_inst_TimingInfo_result("fptoui",FTU->getType()->getIntegerBitWidth(),FTU->getType()->getIntegerBitWidth(),clock_period_str);
//         return result;
//     }
//     else if (FPToSIInst *FTS = dyn_cast<FPToSIInst>(I))
//     {
//         result =
//         get_inst_TimingInfo_result("fptosi",FTS->getType()->getIntegerBitWidth(),FTS->getType()->getIntegerBitWidth(),clock_period_str);
//         return result;
//     }
//     else if (UIToFPInst *UTF = dyn_cast<UIToFPInst>(I))
//     {
//         Value *op1 = UTF->getOperand(0);
//         result =
//         get_inst_TimingInfo_result("uitofp",op1->getType()->getIntegerBitWidth(),op1->getType()->getIntegerBitWidth(),clock_period_str);
//         return result;
//     }
//     else if (SIToFPInst *STF = dyn_cast<SIToFPInst>(I))
//     {
//         Value *op1 = STF->getOperand(0);
//         result =
//         get_inst_TimingInfo_result("sitofp",op1->getType()->getIntegerBitWidth(),op1->getType()->getIntegerBitWidth(),clock_period_str);
//         return result;
//     }
//     else if (ZExtInst *ZEXTI = dyn_cast<ZExtInst>(I))  // such operation like trunc/ext will not
//     cost extra timing on FPGA
//     {
//         return result;
//     }
//     else if (SExtInst *SEXTI = dyn_cast<SExtInst>(I))
//     {
//         return result;
//     }
//     else if (TruncInst *TI = dyn_cast<TruncInst>(I))
//     {
//         return result;
//     }
//     else if (BitCastInst *BI = dyn_cast<BitCastInst>(I))
//     {
//         return result;
//     }

//     ////////////////////////////// Binary Operations /////////////////////////
//     else if (ShlOperator *SHLI = dyn_cast<ShlOperator>(I))
//     {
//         Value *op1 = SHLI->getOperand(1);

//         if (Constant *tmpop = dyn_cast<Constant>(op1))
//             return result;
//             {
//                 result =
//                 get_inst_TimingInfo_result("shl",op1->getType()->getIntegerBitWidth(),SHLI->getType()->getIntegerBitWidth(),clock_period_str);
//                 return result;
//             }
//     }
//     else if (LShrOperator *LSHRI = dyn_cast<LShrOperator>(I))
//     {
//         Value *op1 = LSHRI->getOperand(1);

//         if (Constant *tmpop = dyn_cast<Constant>(op1))
//             return result;
//         else
//         {
//             result =
//             get_inst_TimingInfo_result("lshr",op1->getType()->getIntegerBitWidth(),LSHRI->getType()->getIntegerBitWidth(),clock_period_str);;
//             return result;
//         }
//     }
//     else if (AShrOperator *ASHRI = dyn_cast<AShrOperator>(I))
//     {
//         Value *op1 = ASHRI->getOperand(1);

//         if (Constant *tmpop = dyn_cast<Constant>(op1))
//             return result;
//         else
//         {
//             result =
//             get_inst_TimingInfo_result("ashr",op1->getType()->getIntegerBitWidth(),ASHRI->getType()->getIntegerBitWidth(),clock_period_str);;
//             return result;
//         }
//     }
//     else if (BinaryOperator *BinO = dyn_cast<BinaryOperator>(I))
//     {
//         // for binary operator, we need to consider whether it is a operator for integer or
//         floating-point value std::string opcodeInput; int oprandBitWidth; int resBitWidth;
//         std::string opcode_str(BinO->getOpcodeName());
//         std::transform(opcode_str.begin(), opcode_str.end(), opcode_str.begin(), ::tolower);
//         Value *op1 = BinO->getOperand(1);
//         if (BinO->getType()->isIntegerTy())
//         {
//             oprandBitWidth = op1->getType()->getIntegerBitWidth();
//             resBitWidth = BinO->getType()->getIntegerBitWidth();
//         }
//         else
//         {
//             oprandBitWidth = -1;
//             resBitWidth = -1;
//             // for floating operator, we need to consider whether it is a operator for float
//             value or double value if (BinO->getType()->isDoubleTy() && opcode_str[0]=='f')
//                 opcode_str[0]='d';
//         }

//         // check Add for IntToPtr
//         if (BinO->getOpcode()==Instruction::Add)
//         {
//             for (auto it = BinO->use_begin(),ie = BinO->use_end();it!=ie;++it)
//             {
//                 User* tmp_user = it->getUser();
//                 if (Instruction *UserI = dyn_cast<Instruction>(tmp_user))
//                 {
//                     if (UserI->getOpcode()==Instruction::IntToPtr)
//                         return result;
//                 }
//             }
//         }

//         result =
//         get_inst_TimingInfo_result(opcode_str,oprandBitWidth,resBitWidth,clock_period_str);;
//         return result;
//     }

//     else if (ICmpInst *ICI = dyn_cast<ICmpInst>(I))
//     {
//         int oprandBitWidth;
//         int resBitWidth;
//         Value *op1 = ICI->getOperand(1);
//         oprandBitWidth = op1->getType()->getIntegerBitWidth();
//         resBitWidth = oprandBitWidth;
//         result = get_inst_TimingInfo_result("icmp",oprandBitWidth,resBitWidth,clock_period_str);;
//         return result;
//     }
//     else if (FCmpInst *FI = dyn_cast<FCmpInst>(I))
//     {
//         result = get_inst_TimingInfo_result("fcmp",-1,-1,clock_period_str);;
//         return result;
//     }
//     ////////////////////////////// Memory Operations /////////////////////////
//     else if (StoreInst *SI = dyn_cast<StoreInst>(I))
//     {
//         result = get_inst_TimingInfo_result("store",-1,-1,clock_period_str);
//         result.strict_timing = true;
//         return result;
//     }
//     else if (LoadInst *LI = dyn_cast<LoadInst>(I))
//     {
//         result = get_inst_TimingInfo_result("load",-1,-1,clock_period_str);
//         return result;
//     }
//     else if (AllocaInst *AI = dyn_cast<AllocaInst>(I))
//     {
//         return result;
//     }

//     ////////////////////////////// Control Operations /////////////////////////
//     else if (PHINode *PHI = dyn_cast<PHINode>(I))
//     {
//         int num_Block = PHI->getNumOperands();
//         for (int i=0;i<num_Block;i++)
//         {
//             BasicBlock *tmpB = PHI->getIncomingBlock(i);
//             if (tmpB == PHI->getParent())
//                 return result;
//         }
//         result.latency = 0;
//         result.timing = 0.1;
//         return result;
//     }
//     else if (CallInst *CI = dyn_cast<CallInst>(I))
//     {
//         if (CI->getCalledFunction()->getName().find("HIPartitionMux") !=std::string::npos)
//         {
//             auto partitionConst = dyn_cast<ConstantInt>(CI->getArgOperand(1));
//             int partitionVal = partitionConst->getValue().getSExtValue();
//             if (DEBUG) *Evaluating_log << " handling mux with " << partitionVal<<" inputs.\n";
//             if (partitionVal == 2) result.timing = 0;
//             else if (partitionVal <= 4) result.timing = 1.95;
//             else if (partitionVal <= 8) result.timing = 2;
//             else if (partitionVal <= 16) result.timing = 2.19;
//             else if (partitionVal <= 32) result.timing = 2.73;
//             else if (partitionVal <= 64) result.timing = 3.35;
//             else
//             {
//                 result.timing = 4.1;
//                 print_warning("using undefined partition factor and the mux delay for it is
//                 unknown. set delay=4.1ns for it.");
//             }
//             return result;
//         }
//         if (DEBUG) *Evaluating_log << " Going into subfunction: " <<
//         CI->getCalledFunction()->getName() <<"\n"; result =
//         analyzeFunction(CI->getCalledFunction()); return result;
//     }
//     else if (BranchInst *BI = dyn_cast<BranchInst>(I))
//     {
//         result = get_inst_TimingInfo_result("br",-1,-1,clock_period_str);
//         return result;
//     }
//     else if (ReturnInst *RI = dyn_cast<ReturnInst>(I))
//     {
//         result = get_inst_TimingInfo_result("ret",-1,-1,clock_period_str);
//         return result;
//     }
//     else if (SelectInst *SeI = dyn_cast<SelectInst>(I))
//     {
//         result.timing=1.0;
//         return result;
//     }
//     else if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(I))
//     {
//         result = get_inst_TimingInfo_result("getelementptr",-1,-1,clock_period_str);
//         return result;
//     }
//     else
//     {
//         llvm::errs() << *I << "\n";
//         assert(false && "The instruction is not defined.");
//     }

//     result.latency = 1;
//     return result;
// }

// check whether the two operations can be chained
bool HI_ArraySensitiveToLoopLevel::canCompleteChainOrNot(Instruction *PredI, Instruction *I)
{
    // *Evaluating_log << "        --------- checking Instruction canCompleteChainOrNot: <<" << *I
    // << "\n";
    if (isMACpossible(PredI, I))
    {
        // *Evaluating_log << "        --------- checking Instruction " << *I << " can be chained as
        // MAC\n";
        return true;
    }
    if (isAMApossible(PredI, I))
    {
        // *Evaluating_log << "        --------- checking Instruction " << *I << " can be chained as
        // MAC\n";
        return true;
    }
    return false;
}

bool HI_ArraySensitiveToLoopLevel::canPartitalChainOrNot(Instruction *PredI, Instruction *I)
{
    if (chained_I.find(PredI) != chained_I.end())
        return false;
    if (isTernaryAddpossible(PredI, I))
    {
        // *Evaluating_log << "        --------- checking Instruction " << *I << " can be chained as
        // MAC\n";
        return true;
    }
    return false;
}

bool HI_ArraySensitiveToLoopLevel::isTernaryAddpossible(Instruction *PredI, Instruction *I)
{
    if (!PredI->hasOneUse())
        return false;
    if (I->getOpcode() == Instruction::Add || I->getOpcode() == Instruction::Sub)
    {
        if (PredI->getOpcode() == Instruction::Add || PredI->getOpcode() == Instruction::Sub)
        {
            if (checkInfoAvailability("tadd", PredI->getType()->getIntegerBitWidth(),
                                      PredI->getType()->getIntegerBitWidth(), clock_period_str))
            {
                return true;
            }
        }
    }

    return false;
}

// HI_ArraySensitiveToLoopLevel::timingBase
// HI_ArraySensitiveToLoopLevel::getPartialTimingOverhead(Instruction *PredI, Instruction *I)
// {
//     if (isTernaryAddpossible(PredI,I))
//     {
//         timingBase overallTiming = get_inst_TimingInfo_result( "tadd",
//         PredI->getType()->getIntegerBitWidth() , PredI->getType()->getIntegerBitWidth(),
//         clock_period_str); timingBase basicTiming = getInstructionLatency(PredI); return
//         timingBase(overallTiming.latency - basicTiming.latency,
//                             overallTiming.timing - basicTiming.timing,
//                             basicTiming.II,
//                             basicTiming.clock_period);//timingBase( int l,double t, int i, double
//                             p)
//     }
//     assert(false && "should not reach here.");
// }

// HI_ArraySensitiveToLoopLevel::resourceBase
// HI_ArraySensitiveToLoopLevel::getPartialResourceOverhead(Instruction *PredI, Instruction *I)
// {
//     if (isTernaryAddpossible(PredI,I))
//     {
//         resourceBase overallResource = get_inst_ResourceInfo_result( "tadd",
//         PredI->getType()->getIntegerBitWidth() , PredI->getType()->getIntegerBitWidth(),
//         clock_period_str); resourceBase basicResource = getInstructionResource(PredI); return
//         resourceBase(overallResource.DSP - basicResource.DSP,
//                             overallResource.FF - basicResource.FF,
//                             overallResource.LUT - basicResource.LUT,
//                             basicResource.clock_period);//resourceBase( int D, int F, int L,
//                             double C)
//     }
//     assert(false && "should not reach here.");
// }

// check whether the two operations can be chained into MAC operation
bool HI_ArraySensitiveToLoopLevel::isMACpossible(Instruction *PredI, Instruction *I)
{
    if (I->getOpcode() == Instruction::Add || I->getOpcode() == Instruction::Sub)
    {
        if (PredI->getOpcode() == Instruction::Mul)
        {
            Value *op0 = (PredI->getOperand(0));
            Value *op1 = (PredI->getOperand(1));
            // *Evaluating_log << "        --------- checking Instruction " << *I << " for being
            // chained as MAC, getActualUsersNum=" << getActualUsersNum(PredI,0) << "\n";

            if (op0 && op1 && getActualUsersNum(PredI, 0) < 2)
            {
                // *Evaluating_log << "        --------- checking Instruction " << *I << " for being
                // chained as MAC, op0BW=" << getOriginalBitwidth(op0) << "op1BW=" <<
                // getOriginalBitwidth(op1) << "IBW=" << I->getType()->getIntegerBitWidth() << "\n";
                return (getOriginalBitwidth(op0) <= 18) && (getOriginalBitwidth(op1) <= 18) &&
                       (I->getType()->getIntegerBitWidth() <= 48);
            }
        }
        else if (PredI->getOpcode() == Instruction::Trunc ||
                 PredI->getOpcode() == Instruction::SExt || PredI->getOpcode() == Instruction::ZExt)
        {
            Instruction *Pred_Pred_I = dyn_cast<Instruction>(PredI->getOperand(0));
            if (Pred_Pred_I)
            {
                if (Pred_Pred_I->getOpcode() == Instruction::Mul)
                {
                    Value *op0 = (Pred_Pred_I->getOperand(0));
                    Value *op1 = (Pred_Pred_I->getOperand(1));
                    if (op0 && op1 && getActualUsersNum(Pred_Pred_I, 0) < 2)
                    {
                        return (getOriginalBitwidth(op0) <= 18) &&
                               (getOriginalBitwidth(op1) <= 18) &&
                               (I->getType()->getIntegerBitWidth() <= 48);
                    }
                }
            }
        }
    }
    return false;
}

bool HI_ArraySensitiveToLoopLevel::isAMApossible(Instruction *PredI, Instruction *I)
{
    // for the GEP MAA, consider to transform it into AMA
    if (I->getOpcode() == Instruction::Add || I->getOpcode() == Instruction::Sub)
    {
        Instruction *ori_PredI = dyn_cast<Instruction>(byPassBitcastOp(PredI));
        if (!ori_PredI)
            return false;
        if (ori_PredI->getOpcode() == Instruction::Add ||
            ori_PredI->getOpcode() == Instruction::Sub)
        {
            if (auto Pred_Pred_I = dyn_cast<Instruction>(ori_PredI->getOperand(0)))
            {
                Instruction *ori_Pred_Pred_I = dyn_cast<Instruction>(byPassBitcastOp(Pred_Pred_I));
                if (!ori_Pred_Pred_I)
                    return false;
                if (ori_Pred_Pred_I->getOpcode() == Instruction::Mul)
                {
                    Value *op0 = (ori_Pred_Pred_I->getOperand(0));
                    Value *op1 = (ori_Pred_Pred_I->getOperand(1));
                    if (auto Pred_Pred_I_const =
                            dyn_cast<ConstantInt>(ori_Pred_Pred_I->getOperand(1)))
                    {
                        if (auto I_const = dyn_cast<ConstantInt>(I->getOperand(1)))
                        {
                            long long add_const = (I_const->getValue().getSExtValue());
                            long long mul_const = (Pred_Pred_I_const->getValue().getSExtValue());
                            if (add_const % mul_const == 0)
                            {
                                return (getOriginalBitwidth(op0) <= 18) &&
                                       (getOriginalBitwidth(op1) <= 18) &&
                                       (ori_Pred_Pred_I->getType()->getIntegerBitWidth() <= 48);
                            }
                        }
                    }
                    else
                    {
                        if (auto Pred_Pred_I_op1 =
                                dyn_cast<Instruction>(ori_Pred_Pred_I->getOperand(1)))
                        {
                            if (auto I_op1 = dyn_cast<Instruction>(I->getOperand(1)))
                            {
                                if (I_op1 == Pred_Pred_I_op1)
                                {
                                    return (getOriginalBitwidth(op0) <= 18) &&
                                           (getOriginalBitwidth(op1) <= 18) &&
                                           (ori_Pred_Pred_I->getType()->getIntegerBitWidth() <= 48);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

// Trace back to get the bitwidth of an operand, bypassing truct/zext/sext
int HI_ArraySensitiveToLoopLevel::getOriginalBitwidth(Value *Val)
{
    if (Instruction *I = dyn_cast<Instruction>(Val))
    {
        if (I->getOpcode() == Instruction::Trunc || I->getOpcode() == Instruction::SExt ||
            I->getOpcode() == Instruction::ZExt)
            return getOriginalBitwidth((I->getOperand(0)));
        else
            return I->getType()->getIntegerBitWidth();
    }

    else
        return Val->getType()->getIntegerBitWidth();
}

// Trace forward to get the bitwidth of an operand, bypassing truct/zext/sext
int HI_ArraySensitiveToLoopLevel::getActualUsersNum(Instruction *I, int dep)
{
    std::string cur_opcode = I->getOpcodeName();
    // *Evaluating_log << "        --------- getActualUsersNum tracing " << *I << " at dep=" << dep
    // << ".\n";
    if (dep == 0 || I->getOpcode() == Instruction::Trunc || I->getOpcode() == Instruction::SExt ||
        I->getOpcode() == Instruction::ZExt)
    {
        int num = 0;
        for (auto it = I->use_begin(), ie = I->use_end(); it != ie; ++it)
        {
            User *tmp_user = it->getUser();
            if (Instruction *tmpI = dyn_cast<Instruction>(tmp_user))
                num += getActualUsersNum(tmpI, dep + 1);
        }
        return num;
    }
    else
    {
        return 1;
    }
}

// HI_ArraySensitiveToLoopLevel::resourceBase
// HI_ArraySensitiveToLoopLevel::getInstructionResource(Instruction *I)
// {
//     resourceBase result(0,0,0,clock_period);

//     ////////////////////////////// Cast Operations /////////////////////////
//     if (PtrToIntInst *PTI = dyn_cast<PtrToIntInst>(I)) // such operation like trunc/ext will not
//     cost extra timing on FPGA
//     {
//         return result;
//     }
//     else if (IntToPtrInst *ITP = dyn_cast<IntToPtrInst>(I))
//     {
//         return result;
//     }
//     else if (FPToUIInst *FTU = dyn_cast<FPToUIInst>(I))
//     {
//         result =
//         get_inst_ResourceInfo_result("fptoui",FTU->getType()->getIntegerBitWidth(),FTU->getType()->getIntegerBitWidth(),clock_period_str);
//         return result;
//     }
//     else if (FPToSIInst *FTS = dyn_cast<FPToSIInst>(I))
//     {
//         result =
//         get_inst_ResourceInfo_result("fptosi",FTS->getType()->getIntegerBitWidth(),FTS->getType()->getIntegerBitWidth(),clock_period_str);
//         return result;
//     }
//     else if (UIToFPInst *UTF = dyn_cast<UIToFPInst>(I))
//     {
//         Value *op1 = UTF->getOperand(0);
//         result =
//         get_inst_ResourceInfo_result("uitofp",op1->getType()->getIntegerBitWidth(),op1->getType()->getIntegerBitWidth(),clock_period_str);
//         return result;
//     }
//     else if (SIToFPInst *STF = dyn_cast<SIToFPInst>(I))
//     {
//         Value *op1 = STF->getOperand(0);
//         result =
//         get_inst_ResourceInfo_result("sitofp",op1->getType()->getIntegerBitWidth(),op1->getType()->getIntegerBitWidth(),clock_period_str);
//         return result;
//     }
//     else if (ZExtInst *ZEXTI = dyn_cast<ZExtInst>(I))  // such operation like trunc/ext will not
//     cost extra timing on FPGA
//     {
//         return result;
//     }
//     else if (SExtInst *SEXTI = dyn_cast<SExtInst>(I))
//     {
//         return result;
//     }
//     else if (TruncInst *TI = dyn_cast<TruncInst>(I))
//     {
//         return result;
//     }
//     else if (BitCastInst *BI = dyn_cast<BitCastInst>(I))
//     {
//         return result;
//     }

//     ////////////////////////////// Binary Operations /////////////////////////
//     else if (ShlOperator *SHLI = dyn_cast<ShlOperator>(I))
//     {
//         Value *op1 = SHLI->getOperand(1);

//         if (Constant *tmpop = dyn_cast<Constant>(op1))
//             return result;
//             {
//                 result =
//                 get_inst_ResourceInfo_result("shl",op1->getType()->getIntegerBitWidth(),SHLI->getType()->getIntegerBitWidth(),clock_period_str);
//                 return result;
//             }
//     }
//     else if (LShrOperator *LSHRI = dyn_cast<LShrOperator>(I))
//     {
//         Value *op1 = LSHRI->getOperand(1);

//         if (Constant *tmpop = dyn_cast<Constant>(op1))
//             return result;
//         else
//         {
//             result =
//             get_inst_ResourceInfo_result("lshr",op1->getType()->getIntegerBitWidth(),LSHRI->getType()->getIntegerBitWidth(),clock_period_str);;
//             return result;
//         }
//     }
//     else if (AShrOperator *ASHRI = dyn_cast<AShrOperator>(I))
//     {
//         Value *op1 = ASHRI->getOperand(1);

//         if (Constant *tmpop = dyn_cast<Constant>(op1))
//             return result;
//         else
//         {
//             result =
//             get_inst_ResourceInfo_result("ashr",op1->getType()->getIntegerBitWidth(),ASHRI->getType()->getIntegerBitWidth(),clock_period_str);;
//             return result;
//         }
//     }
//     else if (BinaryOperator *BinO = dyn_cast<BinaryOperator>(I))
//     {
//         // for binary operator, we need to consider whether it is a operator for integer or
//         floating-point value std::string opcodeInput; int oprandBitWidth; int resBitWidth;
//         std::string opcode_str(BinO->getOpcodeName());
//         std::transform(opcode_str.begin(), opcode_str.end(), opcode_str.begin(), ::tolower);
//         Value *op1 = BinO->getOperand(1);
//         if (BinO->getType()->isIntegerTy())
//         {
//             oprandBitWidth = op1->getType()->getIntegerBitWidth();
//             resBitWidth = BinO->getType()->getIntegerBitWidth();
//         }
//         else
//         {
//             oprandBitWidth = -1;
//             resBitWidth = -1;
//             // for floating operator, we need to consider whether it is a operator for float
//             value or double value if (BinO->getType()->isDoubleTy() && opcode_str[0]=='f')
//                 opcode_str[0]='d';
//         }

//         // check Add for IntToPtr
//         if (BinO->getOpcode()==Instruction::Add)
//         {
//             for (auto it = BinO->use_begin(),ie = BinO->use_end();it!=ie;++it)
//             {
//                 User* tmp_user = it->getUser();
//                 if (Instruction *UserI = dyn_cast<Instruction>(tmp_user))
//                 {
//                     if (UserI->getOpcode()==Instruction::IntToPtr)
//                         return result;
//                 }
//             }
//         }

//         result =
//         get_inst_ResourceInfo_result(opcode_str,oprandBitWidth,resBitWidth,clock_period_str);;
//         return result;
//     }

//     else if (ICmpInst *ICI = dyn_cast<ICmpInst>(I))
//     {
//         int oprandBitWidth;
//         int resBitWidth;
//         Value *op1 = ICI->getOperand(1);
//         oprandBitWidth = op1->getType()->getIntegerBitWidth();
//         resBitWidth = oprandBitWidth;
//         result =
//         get_inst_ResourceInfo_result("icmp",oprandBitWidth,resBitWidth,clock_period_str);; return
//         result;
//     }
//     else if (FCmpInst *FI = dyn_cast<FCmpInst>(I))
//     {
//         result = get_inst_ResourceInfo_result("fcmp",-1,-1,clock_period_str);;
//         return result;
//     }
//     ////////////////////////////// Memory Operations /////////////////////////
//     else if (StoreInst *SI = dyn_cast<StoreInst>(I))
//     {
//         result = get_inst_ResourceInfo_result("store",-1,-1,clock_period_str);
//         return result;
//     }
//     else if (LoadInst *LI = dyn_cast<LoadInst>(I))
//     {
//         result = get_inst_ResourceInfo_result("load",-1,-1,clock_period_str);
//         return result;
//     }
//     else if (AllocaInst *AI = dyn_cast<AllocaInst>(I))
//     {
//         result = get_BRAM_Num_For(AI);
//         return result;
//     }

//     ////////////////////////////// Control Operations /////////////////////////
//     else if (PHINode *PHI = dyn_cast<PHINode>(I))
//     {
//         return result;
//     }
//     else if (CallInst *CI = dyn_cast<CallInst>(I))
//     {
//         if (DEBUG) *Evaluating_log << " Going into subfunction: " <<
//         CI->getCalledFunction()->getName() <<"\n"; result =
//         getFunctionResource(CI->getCalledFunction()); return result;
//     }
//     else if (BranchInst *BI = dyn_cast<BranchInst>(I))
//     {
//         return result;
//     }
//     else if (ReturnInst *RI = dyn_cast<ReturnInst>(I))
//     {
//         return result;
//     }
//     else if (SelectInst *SeI = dyn_cast<SelectInst>(I))
//     {
//         return result;
//     }
//     else if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(I))
//     {
//         return result;
//     }
//     else
//     {
//         llvm::errs() << *I << "\n";
//         assert(false && "The instruction is not defined.");
//     }
//     return result;
// }

// // evaluate the number of FF needed by the instruction
// HI_ArraySensitiveToLoopLevel::resourceBase
// HI_ArraySensitiveToLoopLevel::FF_Evaluate(std::map<Instruction*, timingBase>
// &cur_InstructionCriticalPath, Instruction* I)
// {
//     resourceBase res(0,0,0,clock_period);
//     if (DEBUG) *FF_log << "\n\nChecking FF needed by Instruction: [" << *I << "]\n";

//     // Handle Load/Store for FF calculation since usually we have lower the GEP to
//     mul/add/inttoptr/ptrtoint operations if (auto storeI = dyn_cast<StoreInst>(I))
//     {
//         if (DEBUG) *FF_log << "---- is a store instruction\n";

//         // consider the address instruction for store instruction
//         if (auto l0_pred = dyn_cast<IntToPtrInst>(storeI->getOperand(1)))
//         {
//             if (DEBUG) *FF_log << "---- checking the register for address\n";
//             if (DEBUG) *FF_log << "---- found the ITP instruction for it: " << *l0_pred <<"\n";
//             if (auto l1_pred = dyn_cast<AddOperator>(l0_pred->getOperand(0)))
//             {
//                 if (DEBUG) *FF_log << "---- found the Add instruction for its offset: " <<
//                 *l1_pred <<"\n"; for (int i = 0 ; i < l1_pred->getNumOperands(); i++)
//                 {
//                     if (isa<PtrToIntInst>(l1_pred->getOperand(i)))
//                         continue;

//                     if (auto l2_pred =
//                     dyn_cast<Instruction>(byPassBitcastOp(l1_pred->getOperand(i))))
//                     {
//                         if (DEBUG) *FF_log << "---- found the exact offset instruction for it: "
//                         << *l2_pred <<"\n";

//                         // check whether we should consider the FF cost by this instruction
//                         l2_pred if (Instruction_FFAssigned.find(l2_pred) !=
//                         Instruction_FFAssigned.end())
//                         {
//                             if (DEBUG) *FF_log << "---- which is registered.\n";
//                             return res;
//                         }

//                         if (BlockContain(I->getParent(), l2_pred))
//                         {
//                             if (cur_InstructionCriticalPath.find(l2_pred) !=
//                             cur_InstructionCriticalPath.end())
//                                 if (cur_InstructionCriticalPath[l2_pred].latency  ==
//                                 (cur_InstructionCriticalPath[I] -
//                                 getInstructionLatency(I)).latency)// WARNING: there are
//                                 instructions with negative latency in the libraries
//                                 {
//                                     if (DEBUG) *FF_log << "---- which needs no register.\n";
//                                     return res;
//                                 }
//                         }

//                         // For ZExt/SExt Instruction, we do not need to consider those constant
//                         bits int minBW = l2_pred->getType()->getIntegerBitWidth(); if (auto
//                         zext_I = dyn_cast<ZExtInst>(l2_pred))
//                         {
//                             minBW = zext_I->getSrcTy()->getIntegerBitWidth();
//                             if (DEBUG) *FF_log << "---- which involves extension operation and
//                             the src BW is " << minBW << "\n";
//                         }
//                         if (auto sext_I = dyn_cast<SExtInst>(l2_pred))
//                         {
//                             minBW = sext_I->getSrcTy()->getIntegerBitWidth();
//                             if (DEBUG) *FF_log << "---- which involves extension operation and
//                             the src BW is " << minBW << "\n";
//                         }
//                         res.FF = minBW;
//                         Instruction_FFAssigned.insert(l2_pred);
//                     }
//                 }

//             }
//             else
//             {
//                 print_warning("WARNING: The PRE-predecessor of store instruction should be
//                 AddOperator.");
//             }
//         }
//         else
//         {
//             print_warning("WARNING: The predecessor of store instruction should be
//             IntToPtrInst.");
//         }

//         // consider the data instruction for store instruction
//         if (auto I_Pred = dyn_cast<Instruction>(storeI->getOperand(0)))
//         {
//             if (DEBUG) *FF_log << "---- checking the register for data\n";

//             if (cur_InstructionCriticalPath.find(I_Pred) != cur_InstructionCriticalPath.end())
//             {
//                 if (checkLoadOpRegisterReusable(I_Pred,
//                 (cur_InstructionCriticalPath[I_Pred]-getInstructionLatency(I_Pred)).latency))
//                 {
//                     if (DEBUG) *FF_log << "---- reuse load instruction reg for it, bypass\n";
//                     return res;
//                 }
//             }

//             if (I_Pred->getType()->isIntegerTy() )
//             {
//                 int minBW = I_Pred->getType()->getIntegerBitWidth();

//                 // For ZExt/SExt Instruction, we do not need to consider those constant bits
//                 if (auto zext_I = dyn_cast<ZExtInst>(I_Pred))
//                 {
//                     Instruction* ori_I = byPassUnregisterOp(zext_I);
//                     if (cur_InstructionCriticalPath.find(ori_I) !=
//                     cur_InstructionCriticalPath.end())
//                     {
//                         if (checkLoadOpRegisterReusable(ori_I,
//                         (cur_InstructionCriticalPath[ori_I]-getInstructionLatency(ori_I)).latency))
//                         {
//                             if (DEBUG) *FF_log << "---- reuse load instruction reg for it,
//                             bypass\n"; return res;
//                         }
//                     }
//                     if (Instruction_FFAssigned.find(ori_I) == Instruction_FFAssigned.end())
//                     {

//                         minBW = zext_I->getSrcTy()->getIntegerBitWidth();
//                         if (DEBUG) *FF_log << "---- which involves extension operation and the
//                         src BW is " << minBW << "\n"; Instruction_FFAssigned.insert(ori_I);
//                     }
//                     else
//                     {
//                         if (DEBUG) *FF_log << "---- which is registered.\n";
//                     }

//                 }
//                 if (auto sext_I = dyn_cast<SExtInst>(I_Pred))
//                 {
//                     Instruction* ori_I = byPassUnregisterOp(sext_I);
//                     if (cur_InstructionCriticalPath.find(ori_I) !=
//                     cur_InstructionCriticalPath.end())
//                     {
//                         if (checkLoadOpRegisterReusable(ori_I,
//                         (cur_InstructionCriticalPath[ori_I]-getInstructionLatency(ori_I)).latency))
//                         {
//                             if (DEBUG) *FF_log << "---- reuse load instruction reg for it,
//                             bypass\n"; return res;
//                         }
//                     }

//                     if (Instruction_FFAssigned.find(ori_I) == Instruction_FFAssigned.end())
//                     {
//                         minBW = sext_I->getSrcTy()->getIntegerBitWidth();
//                         if (DEBUG) *FF_log << "---- which involves extension operation and the
//                         src BW is " << minBW << "\n"; Instruction_FFAssigned.insert(ori_I);
//                     }
//                     else
//                     {
//                         if (DEBUG) *FF_log << "---- which is registered.\n";
//                     }
//                 }

//                 if (cur_InstructionCriticalPath.find(I_Pred) !=
//                 cur_InstructionCriticalPath.end())
//                     if (cur_InstructionCriticalPath[I_Pred].latency  ==
//                     (cur_InstructionCriticalPath[I] - getInstructionLatency(I)).latency)//
//                     WARNING: there are instructions with negative latency in the libraries
//                     {
//                         if (DEBUG) *FF_log << "---- which needs no register.\n";
//                         return res;
//                     }

//                 res.FF += minBW;

//                 Instruction_FFAssigned.insert(I_Pred);
//             }
//         }

//         return res;
//     }

//     // Handle Load/Store for FF calculation since usually we have lower the GEP to
//     mul/add/inttoptr/ptrtoint operations if (auto loadI = dyn_cast<LoadInst>(I))
//     {
//         if (DEBUG) *FF_log << "---- is a load instruction\n";
//         // consider the address instruction for store instruction
//         if (auto l0_pred = dyn_cast<IntToPtrInst>(loadI->getOperand(0)))
//         {
//             if (DEBUG) *FF_log << "---- checking the register for address\n";
//             if (DEBUG) *FF_log << "---- found the ITP instruction for it: " << *l0_pred <<"\n";
//             if (auto l1_pred = dyn_cast<AddOperator>(l0_pred->getOperand(0)))
//             {
//                 if (DEBUG) *FF_log << "---- found the Add instruction for its offset: " <<
//                 *l1_pred <<"\n"; for (int i = 0 ; i < l1_pred->getNumOperands(); i++)
//                 {
//                     if (isa<PtrToIntInst>(l1_pred->getOperand(i)))
//                         continue;
//                     if (auto l2_pred =
//                     dyn_cast<Instruction>(byPassBitcastOp(l1_pred->getOperand(i))))
//                     {
//                         if (DEBUG) *FF_log << "---- found the exact offset instruction for it: "
//                         << *l2_pred <<"\n";

//                         // check whether we should consider the FF cost by this instruction
//                         l2_pred if (Instruction_FFAssigned.find(l2_pred) !=
//                         Instruction_FFAssigned.end())
//                         {
//                             if (DEBUG) *FF_log << "---- which is registered.\n";
//                             return res;
//                         }

//                         if (BlockContain(I->getParent(), l2_pred))
//                         {
//                             if (cur_InstructionCriticalPath.find(l2_pred) !=
//                             cur_InstructionCriticalPath.end())
//                             {
//                                 if (cur_InstructionCriticalPath[l2_pred].latency  ==
//                                 (cur_InstructionCriticalPath[I] -
//                                 getInstructionLatency(I)).latency)// WARNING: there are
//                                 instructions with negative latency in the libraries
//                                 {
//                                     if (DEBUG) *FF_log << "---- which needs no register.\n";
//                                     return res;
//                                 }
//                             }
//                         }

//                         // For ZExt/SExt Instruction, we do not need to consider those constant
//                         bits int minBW = l2_pred->getType()->getIntegerBitWidth(); if (auto
//                         zext_I = dyn_cast<ZExtInst>(l2_pred))
//                         {
//                             minBW = zext_I->getSrcTy()->getIntegerBitWidth();
//                             if (DEBUG) *FF_log << "---- which involves extension operation and
//                             the src BW is " << minBW << "\n";
//                         }
//                         if (auto sext_I = dyn_cast<SExtInst>(l2_pred))
//                         {
//                             minBW = sext_I->getSrcTy()->getIntegerBitWidth();
//                             if (DEBUG) *FF_log << "---- which involves extension operation and
//                             the src BW is " << minBW << "\n";
//                         }
//                         res.FF = minBW;
//                         Instruction_FFAssigned.insert(l2_pred);
//                     }
//                 }

//             }
//             else
//             {
//                 print_warning("WARNING: The PRE-predecessor of load instruction should be
//                 AddOperator.");
//             }
//         }
//         else
//         {
//             bool warnOut = 1;
//             if (auto tmp_arg = dyn_cast<Argument>(loadI->getOperand(0)))
//                 warnOut = 0;
//             if (auto tmp_alloca = dyn_cast<AllocaInst>(loadI->getOperand(0)))
//                 warnOut = 0;
//             if (warnOut)
//                 print_warning("WARNING: The predecessor of load instruction should be
//                 IntToPtrInst.");
//         }
//         return res;
//     }

//     // ignore the instruction if it is a PtrToInt instruction, since in FPGA, we do not need to
//     consider this instruction if (DEBUG) *FF_log << "---- is a non-memory-access instruction\n";
//     for (User::op_iterator I_tmp = I->op_begin(), I_Pred_end = I->op_end(); I_tmp != I_Pred_end;
//     ++I_tmp)
//     {
//         if (auto I_Pred = dyn_cast<PtrToIntInst>(I_tmp))
//         {
//             if (DEBUG) *FF_log << "---- is an helper instruction for array access, bypass\n";
//             return res;
//         }
//     }

//     // for other instructions, we need to check whether we need registers for their operands
//     for (User::op_iterator I_tmp = I->op_begin(), I_Pred_end = I->op_end(); I_tmp != I_Pred_end;
//     ++I_tmp)
//     {
//         if (auto I_Pred = dyn_cast<Instruction>(I_tmp))
//         {
//             if (DEBUG) *FF_log << "---- checking op: [" << *I_Pred << "]\n";
//             FF_log->flush();
//             // check whether we should consider the FF cost by this instruction I
//             if (Instruction_FFAssigned.find(I_Pred) != Instruction_FFAssigned.end())
//             {
//                 if (DEBUG) *FF_log << "---- op: [" << *I_Pred << "] is registered.\n";
//                 continue;
//             }

//             // try to reuse the load registers if they are released from previous accesses
//             if (cur_InstructionCriticalPath.find(I_Pred) != cur_InstructionCriticalPath.end())
//             {
//                 if (checkLoadOpRegisterReusable(I_Pred,
//                 (cur_InstructionCriticalPath[I_Pred]-getInstructionLatency(I_Pred)).latency))
//                 {
//                     if (DEBUG) *FF_log << "---- reuse load instruction reg for it, bypass\n";
//                     continue;
//                 }
//             }

//             if (BlockContain(I->getParent(), I_Pred))
//             {
//                 // may be the operand is operated later, especially for phi insturction in loop
//                 // for this situation, we may still need registers for the operands
//                 if (cur_InstructionCriticalPath.find(I_Pred) !=
//                 cur_InstructionCriticalPath.end())
//                     if (cur_InstructionCriticalPath[I_Pred].latency ==
//                     (cur_InstructionCriticalPath[I] - getInstructionLatency(I)).latency) //
//                     WARNING: there are instructions with negative latency in the libraries
//                     {
//                         if (DEBUG) *FF_log << "---- which needs no register.\n";
//                         continue;
//                     }
//             }

//             // calculate the FF needed to store the intermediate result
//             if (I_Pred->getType()->isIntegerTy() )
//             {
//                 int minBW = I_Pred->getType()->getIntegerBitWidth();

//                 // For ZExt/SExt Instruction, we do not need to consider those constant bits
//                 if (auto zext_I = dyn_cast<ZExtInst>(I_Pred))
//                 {
//                     Instruction* ori_I = byPassUnregisterOp(zext_I); //zext_I
//                     if (cur_InstructionCriticalPath.find(ori_I) !=
//                     cur_InstructionCriticalPath.end())
//                     {
//                         if (checkLoadOpRegisterReusable(ori_I,
//                         (cur_InstructionCriticalPath[ori_I]-getInstructionLatency(ori_I)).latency))
//                         {
//                             if (DEBUG) *FF_log << "---- reuse load instruction reg for it,
//                             bypass\n"; continue;
//                         }
//                     }
//                     if (Instruction_FFAssigned.find(ori_I) != Instruction_FFAssigned.end())
//                     {
//                         if (DEBUG) *FF_log << "---- ori_op: [" << *ori_I << "] is registered.\n";
//                         continue;
//                     }
//                     else
//                     {
//                         minBW = zext_I->getSrcTy()->getIntegerBitWidth();
//                         Instruction_FFAssigned.insert(ori_I);
//                         if (DEBUG) *FF_log << "---- which involves extension operation and the
//                         src BW is " << minBW << "\n";
//                     }
//                 }
//                 else if (auto sext_I = dyn_cast<SExtInst>(I_Pred))
//                 {
//                     Instruction* ori_I = byPassUnregisterOp(sext_I);
//                     if (cur_InstructionCriticalPath.find(ori_I) !=
//                     cur_InstructionCriticalPath.end())
//                     {
//                         if (checkLoadOpRegisterReusable(ori_I,
//                         (cur_InstructionCriticalPath[ori_I]-getInstructionLatency(ori_I)).latency))
//                         {
//                             if (DEBUG) *FF_log << "---- reuse load instruction reg for it,
//                             bypass\n"; continue;
//                         }
//                     }

//                     if (Instruction_FFAssigned.find(ori_I) != Instruction_FFAssigned.end())
//                     {
//                         if (DEBUG) *FF_log << "---- ori_op: [" << *ori_I << "] is registered.\n";
//                         continue;
//                     }
//                     else
//                     {
//                         minBW = sext_I->getSrcTy()->getIntegerBitWidth();
//                         Instruction_FFAssigned.insert(ori_I);
//                         if (DEBUG) *FF_log << "---- which involves extension operation and the
//                         src BW is " << minBW << "\n";
//                     }
//                 }

//                 if (DEBUG) *FF_log << "---- op or the ori_op of " <<*I_Pred << " register now.
//                 \n"; res.FF += minBW;

//                 Instruction_FFAssigned.insert(I_Pred);
//             }
//             else if (I_Pred->getType()->isFloatTy() )
//             {
//                 res.FF += 32;
//                 if (DEBUG) *FF_log << "---- ori_op: [" << *I_Pred << "] is a float variable and
//                 registered.\n"; Instruction_FFAssigned.insert(I_Pred);
//             }
//             else if (I_Pred->getType()->isDoubleTy() )
//             {
//                 res.FF += 64;
//                 if (DEBUG) *FF_log << "---- ori_op: [" << *I_Pred << "] is a double variable and
//                 registered.\n"; Instruction_FFAssigned.insert(I_Pred);
//             }
//         }
//     }

//     // in VivadoHLS, for PHI node, no matter whether the value is involved in other cycle or not,
//     it will be
//     // registered as phireg (refer to the verbose.rpt in Vivado)
//     if (auto PHI_I = dyn_cast<PHINode>(I))
//     {
//         if (DEBUG) *FF_log << "---- is PHI instruction\n";
//         if (Instruction_FFAssigned.find(PHI_I) == Instruction_FFAssigned.end())
//         {
//             if (PHI_I->getType()->isIntegerTy())
//             {
//                 int BW = PHI_I->getType()->getIntegerBitWidth();
//                 res.FF += BW;
//                 Instruction_FFAssigned.insert(PHI_I);
//                 if (DEBUG) *FF_log << "---- register anyway\n";
//             }
//         }
//         else
//         {
//             if (DEBUG) *FF_log << "---- is registered\n";
//         }
//     }

//     return res;
// }

// evaluate the number of LUT needed by the PHI instruction
HI_ArraySensitiveToLoopLevel::resourceBase HI_ArraySensitiveToLoopLevel::IndexVar_LUT(
    std::map<Instruction *, timingBase> &cur_InstructionCriticalPath, Instruction *I)
{
    resourceBase res(0, 0, 0, clock_period);

    if (auto PHI_I = dyn_cast<PHINode>(I))
    {
        for (User::op_iterator I_tmp = I->op_begin(), I_Pred_end = I->op_end(); I_tmp != I_Pred_end;
             ++I_tmp)
        {
            if (auto I_Pred = dyn_cast<Instruction>(I_tmp))
            {
                if (BlockContain(I->getParent(), I_Pred))
                {
                    // may be the operand is operated later, especially for phi insturction in loop
                    if (cur_InstructionCriticalPath.find(I_Pred) ==
                        cur_InstructionCriticalPath.end())
                    {
                        res.LUT = 9; // for invar PHI with two input
                    }
                }
            }
        }
    }
    return res;
}

// trace back to find the original operator, bypassing SExt and ZExt operations
Instruction *HI_ArraySensitiveToLoopLevel::byPassUnregisterOp(Instruction *cur_I)
{

    // For ZExt/SExt Instruction, we do not need to consider those constant bits
    if (/*cur_I->getOpcode() == Instruction::Trunc ||*/ cur_I->getOpcode() == Instruction::ZExt ||
        cur_I->getOpcode() == Instruction::SExt)
    {
        if (auto next_I = dyn_cast<Instruction>(cur_I->getOperand(0)))
        {
            return byPassUnregisterOp(next_I);
        }
        else
        {
            assert(false && "Predecessor of bitcast operator should be found.\n");
        }
    }
    // else if ( cur_I->getOpcode() == Instruction::PHI )
    // {
    //     int constant_cnt = 0;
    //     Instruction *I_incoming;
    //     for (int i = 0 ; i < cur_I->getNumOperands(); ++i )
    //     {
    //         if (auto const_val = dyn_cast<Constant>(cur_I->getOperand(i)))
    //         {
    //             constant_cnt ++;
    //         }
    //         else
    //         {
    //             if (auto op_I = dyn_cast<Instruction>(cur_I->getOperand(i)))
    //             {
    //                 I_incoming = op_I;
    //             }
    //         }
    //     }
    //     if (constant_cnt == 1)
    //     {
    //         return byPassUnregisterOp(I_incoming);
    //     }
    //     else
    //     {
    //         return cur_I;
    //     }

    // }
    else
    {
        return cur_I;
    }
}

// trace back to find the original operator, bypassing SExt and ZExt operations
Value *HI_ArraySensitiveToLoopLevel::byPassBitcastOp(Instruction *cur_I)
{

    // For ZExt/SExt Instruction, we do not need to consider those constant bits
    if (/*cur_I->getOpcode() == Instruction::Trunc || */ cur_I->getOpcode() == Instruction::ZExt ||
        cur_I->getOpcode() == Instruction::SExt)
    {
        if (auto next_I = dyn_cast<Instruction>(cur_I->getOperand(0)))
        {
            return byPassBitcastOp(next_I);
        }
        else
        {
            if (auto next_Arg = dyn_cast<Argument>(cur_I->getOperand(0)))
            {
                return next_Arg;
            }
            assert(false && "Predecessor of bitcast operator should be found.\n");
        }
    }
    else
    {
        return cur_I;
    }
}

// trace back to find the original operator, bypassing SExt and ZExt operations
Value *HI_ArraySensitiveToLoopLevel::byPassBitcastOp(Value *cur_I_value)
{
    auto cur_I = dyn_cast<Instruction>(cur_I_value);

    if (auto arg = dyn_cast<Argument>(cur_I_value))
        return cur_I_value;
    if (!cur_I)
    {
        return cur_I_value;
        llvm::errs() << *cur_I_value << "\n";
    }
    assert(cur_I && "This should be an instruction.\n");
    // For ZExt/SExt Instruction, we do not need to consider those constant bits
    if (/*cur_I->getOpcode() == Instruction::Trunc || */ cur_I->getOpcode() == Instruction::ZExt ||
        cur_I->getOpcode() == Instruction::SExt)
    {
        if (auto next_I = dyn_cast<Instruction>(cur_I->getOperand(0)))
        {
            return byPassBitcastOp(next_I);
        }
        else
        {
            if (auto next_Arg = dyn_cast<Argument>(cur_I->getOperand(0)))
            {
                return next_Arg;
            }
            assert(false && "Predecessor of bitcast operator should be found.\n");
        }
    }
    else
    {
        return cur_I;
    }
}

// for load instructions, HLS will reuse the register for the data
bool HI_ArraySensitiveToLoopLevel::checkLoadOpRegisterReusable(Instruction *Load_I, int time_point)
{
    if (Load_I->getOpcode() != Instruction::Load)
        return false;

    if (DEBUG)
        *FF_log << "\n\ncheckLoadOpRegisterReusable for instruction: [" << *Load_I
                << "] at cycle in the block: " << time_point << "\n";
    // currently, the situation for a load instruction with different target array is ignored.
    if (Access2TargetMap[Load_I].size() > 1)
    {
        if (DEBUG)
            *FF_log << "---- the load has multiple potential target array, bypass it.\n";
        return false;
    }

    for (auto Access_I : AccessesList)
    {
        if (auto tmp_load_I = dyn_cast<LoadInst>(Access_I))
        {
            if (Access_I == Load_I)
                continue;

            if (DEBUG)
                *FF_log << "---- checking candidate Instruction: " << *tmp_load_I << "\n";

            if (Instruction_FFAssigned.find(tmp_load_I) == Instruction_FFAssigned.end())
            {
                if (DEBUG)
                    *FF_log << "---- no register used for it, bypass the candidate.\n";
                continue;
            }

            // the result register has been reused, bypass it
            if (I_RegReused.find(tmp_load_I) != I_RegReused.end())
            {
                if (DEBUG)
                    *FF_log << "---- the register is reused, bypass it.\n";
                continue;
            }

            // currently, the situation for a load instruction with different target array is
            // ignored.
            if (Access2TargetMap[tmp_load_I].size() > 1)
            {
                if (DEBUG)
                    *FF_log
                        << "---- the candidate has multiple potential target array, bypass it.\n";
                continue;
            }

            // find a load instruction which has the same target array
            if (Access2TargetMap[tmp_load_I][0] == Access2TargetMap[Load_I][0])
            {
                if (RegRelease_Schedule.find(tmp_load_I) == RegRelease_Schedule.end())
                {
                    if (DEBUG)
                        *FF_log << "---- no lifetime information for the instruction, bypass it.\n";
                    continue;
                }

                // check the lifetime of the previous load instruction register
                BasicBlock *tmpB = RegRelease_Schedule[tmp_load_I].first;
                int last_time_point = RegRelease_Schedule[tmp_load_I].second;
                if (tmpB != Load_I->getParent())
                {
                    if (DEBUG)
                        *FF_log << "---- the candidate is in different block, reuse it.\n";
                    I_RegReused.insert(tmp_load_I);
                    return true;
                }
                else if (time_point >= last_time_point)
                {
                    I_RegReused.insert(tmp_load_I);
                    if (DEBUG)
                        *FF_log << "---- the candidate is out of its lifetime, reuse it.\n";
                    return true;
                }
                else
                {
                    if (DEBUG)
                        *FF_log << "---- the candidate is not reusable: in Block ["
                                << tmpB->getName() << "]  at cycle : " << last_time_point << "\n";
                }
            }
        }
    }
    return false;
}

// update the latest user of the the specific user, based on which we can determine the lifetime of
// a register
void HI_ArraySensitiveToLoopLevel::updateResultRelease(Instruction *I, Instruction *I_Pred,
                                                       int time_point)
{
    if (RegRelease_Schedule.find(I_Pred) == RegRelease_Schedule.end())
    {
        RegRelease_Schedule[I_Pred] = std::pair<BasicBlock *, int>(I->getParent(), time_point);
    }
    else
    {
        BasicBlock *tmpB = RegRelease_Schedule[I_Pred].first;
        int last_time_point = RegRelease_Schedule[I_Pred].second;
        if (tmpB != I->getParent())
        {
            RegRelease_Schedule[I_Pred] = std::pair<BasicBlock *, int>(I->getParent(), time_point);
        }
        else if (time_point > last_time_point)
        {
            RegRelease_Schedule[I_Pred] = std::pair<BasicBlock *, int>(I->getParent(), time_point);
        }
    }
    return;
}

// get the resource cost of FP operator with opcode string
HI_ArraySensitiveToLoopLevel::resourceBase
HI_ArraySensitiveToLoopLevel::checkFPOperatorCost(std::string opcode_str)
{

    resourceBase result(0, 0, 0, clock_period);
    int oprandBitWidth;
    int resBitWidth;

    std::transform(opcode_str.begin(), opcode_str.end(), opcode_str.begin(), ::tolower);

    oprandBitWidth = -1;
    resBitWidth = -1;

    result =
        get_inst_ResourceInfo_result(opcode_str, oprandBitWidth, resBitWidth, clock_period_str);
    ;
    return result;
}

// transform instruction to its opcode string
std::string HI_ArraySensitiveToLoopLevel::InstToOpcodeString(Instruction *I)
{
    if (BinaryOperator *BinO = dyn_cast<BinaryOperator>(I))
    {
        // for binary operator, we need to consider whether it is a operator for integer or
        // floating-point value
        std::string opcodeInput;
        int oprandBitWidth;
        int resBitWidth;
        std::string opcode_str(BinO->getOpcodeName());
        std::transform(opcode_str.begin(), opcode_str.end(), opcode_str.begin(), ::tolower);
        Value *op1 = BinO->getOperand(1);
        if (BinO->getType()->isIntegerTy())
        {
            assert(false && "undefined branch.");
            oprandBitWidth = op1->getType()->getIntegerBitWidth();
            resBitWidth = BinO->getType()->getIntegerBitWidth();
        }
        else
        {
            oprandBitWidth = -1;
            resBitWidth = -1;
            // for floating operator, we need to consider whether it is a operator for float value
            // or double value
            if (BinO->getType()->isDoubleTy() && opcode_str[0] == 'f')
                opcode_str[0] = 'd';
            return opcode_str;
        }
    }
    assert(false && "should not reach here.");
}