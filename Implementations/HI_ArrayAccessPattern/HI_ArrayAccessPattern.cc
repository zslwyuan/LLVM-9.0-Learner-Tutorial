#include "HI_ArrayAccessPattern.h"
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

bool HI_ArrayAccessPattern::runOnFunction(
    Function &F) // The runOnModule declaration will overide the virtual one in ModulePass, which
                 // will be executed for each Module.
{
    ScalarEvolution &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();

    std::string demangled_name = demangleFunctionName(F.getName());
    bool isTopFunction = top_function_name == demangled_name;

    TraceMemoryAccessinFunction(F);
    findMemoryDeclarationin(&F, isTopFunction);

    for (auto &B : F)
    {
        for (auto &I : B)
        {
            ArrayAccessOffset(&I, &SE, isTopFunction);
        }
    }
    return false;
}

char HI_ArrayAccessPattern::ID =
    0; // the ID for pass should be initialized but the value does not matter, since LLVM uses the
       // address of this variable as label instead of its value.

void HI_ArrayAccessPattern::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.setPreservesCFG();
}

// check whether the instruction is Multiplication suitable for LSR
// If suitable, process it
bool HI_ArrayAccessPattern::ArrayAccessOffset(Instruction *I, ScalarEvolution *SE,
                                              bool isTopFunction)
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

    auto ITP_I = dyn_cast<IntToPtrInst>(I->use_begin()->getUser());

    if (!ITP_I)
        return false;

    // 1.  get the incremental value by using SCEV
    const SCEV *tmp_S = SE->getSCEV(I);
    const SCEVAddRecExpr *SARE = dyn_cast<SCEVAddRecExpr>(tmp_S);
    if (SARE)
    {
        if (SARE->isAffine())
        {
            *ArrayLog << *I << " --> is add rec Affine Add: " << *SARE << " it operand (0) "
                      << *SARE->getOperand(0) << " it operand (1) " << *SARE->getOperand(1) << "\n";
            *ArrayLog << " -----> intial offset expression: " << *findTheActualStartValue(SARE)
                      << "\n";
            ArrayLog->flush();
            const SCEV *initial_expr_tmp = findTheActualStartValue(SARE);
            int initial_const = -1;
            Value *target = nullptr;
            if (auto initial_expr_add = dyn_cast<SCEVAddExpr>(initial_expr_tmp))
            {
                for (int i = 0; i < initial_expr_add->getNumOperands(); i++)
                {
                    if (const SCEVConstant *start_V =
                            dyn_cast<SCEVConstant>(initial_expr_add->getOperand(i)))
                    {
                        initial_const = start_V->getAPInt().getSExtValue();
                        *ArrayLog << " -----> intial offset const: " << initial_const << "\n";
                        ArrayLog->flush();
                    }
                    else
                    {
                        if (const SCEVUnknown *array_value_scev =
                                dyn_cast<SCEVUnknown>(initial_expr_add->getOperand(i)))
                        {
                            *ArrayLog << " -----> access target: " << *array_value_scev->getValue()
                                      << "\n";
                            if (auto tmp_PTI_I =
                                    dyn_cast<PtrToIntInst>(array_value_scev->getValue()))
                            {
                                target = tmp_PTI_I->getOperand(0);
                            }
                            else
                            {
                                assert(target && "There should be an PtrToInt Instruction for the "
                                                 "addition operation.\n");
                            }
                            *ArrayLog << " -----> access target info: " << Target2ArrayInfo[target]
                                      << "\n";
                            ArrayLog->flush();
                        }
                        else
                        {
                            assert(false && "The access target should be found.\n");
                        }
                    }
                }
                assert(initial_const >= 0 && "the initial offset should be found.\n");
                assert(target && "the target array should be found.\n");
                Inst2AccessInfo[I] = getAccessInfoFor(target, initial_const);
                *ArrayLog << " -----> access info with array index: " << Inst2AccessInfo[I]
                          << "\n\n\n";
                ArrayLog->flush();
            }
            else if (auto initial_expr_unknown = dyn_cast<SCEVUnknown>(initial_expr_tmp))
            {

                initial_const = 0;
                *ArrayLog << " -----> intial offset const: " << initial_const << "\n";
                *ArrayLog << " -----> access target: " << *initial_expr_unknown->getValue() << "\n";
                if (auto tmp_PTI_I = dyn_cast<PtrToIntInst>(initial_expr_unknown->getValue()))
                {
                    target = tmp_PTI_I->getOperand(0);
                }
                else
                {
                    assert(target &&
                           "There should be an PtrToInt Instruction for the addition operation.\n");
                }

                *ArrayLog << " -----> access target info: " << Target2ArrayInfo[target] << "\n";
                ArrayLog->flush();

                assert(initial_const >= 0 && "the initial offset should be found.\n");
                assert(target && "the target array should be found.\n");
                Inst2AccessInfo[I] = getAccessInfoFor(target, initial_const);
                *ArrayLog << " -----> access info with array index: " << Inst2AccessInfo[I]
                          << "\n\n\n";
                ArrayLog->flush();
            }
        }
    }
    return false;
}

HI_ArrayAccessPattern::HI_AccessInfo HI_ArrayAccessPattern::getAccessInfoFor(Value *target,
                                                                             int initial_offset)
{
    HI_AccessInfo res(Target2ArrayInfo[target]);
    for (int i = 0; i < res.num_dims; i++)
    {
        res.index[i] = (initial_offset / res.sub_element_num[i]) % res.dim_size[i];
    }

    return res;
}

const SCEV *HI_ArrayAccessPattern::findTheActualStartValue(const SCEVAddRecExpr *S)
{
    if (const SCEVAddExpr *start_V = dyn_cast<SCEVAddExpr>(S->getOperand(0)))
    {
        return start_V;
    }
    else if (const SCEVUnknown *start_V_unknown = dyn_cast<SCEVUnknown>(S->getOperand(0)))
    {
        return start_V_unknown;
    }
    else
    {
        if (const SCEVAddRecExpr *SARE = dyn_cast<SCEVAddRecExpr>(S->getOperand(0)))
            findTheActualStartValue(SARE);
    }
}

// check the memory access in the function
void HI_ArrayAccessPattern::TraceMemoryAccessinFunction(Function &F)
{
    if (F.getName().find("llvm.") != std::string::npos) // bypass the "llvm.xxx" functions..
        return;
    findMemoryAccessin(&F);
}

// find the array access in the function F and trace the accesses to them
void HI_ArrayAccessPattern::findMemoryAccessin(Function *F)
{
    *ArrayLog << "checking the Memory Access information in Function: " << F->getName() << "\n";
    ArrayValueVisited.clear();

    // for general function in HLS, arrays in functions are usually declared with alloca instruction
    for (auto &B : *F)
    {
        for (auto &I : B)
        {
            if (IntToPtrInst *ITP_I = dyn_cast<IntToPtrInst>(&I))
            {
                *ArrayLog << "find a IntToPtrInst: [" << *ITP_I << "] backtrace to its operands.\n";
                TraceAccessForTarget(ITP_I);
            }
        }
    }
    *ArrayLog << "-------------------------------------------------"
              << "\n\n\n\n";
    ArrayLog->flush();
}

// find out which instrctuins are related to the array, going through PtrToInt, Add, IntToPtr,
// Store, Load instructions
void HI_ArrayAccessPattern::TraceAccessForTarget(Value *cur_node)
{
    *ArrayLog << "looking for the operands of " << *cur_node << "\n";
    if (Instruction *tmpI = dyn_cast<Instruction>(cur_node))
    {
        Inst_AccessRelated.insert(tmpI);
    }
    else
    {
        return;
    }

    Instruction *curI = dyn_cast<Instruction>(cur_node);
    ArrayLog->flush();

    // we are doing DFS now
    if (ArrayValueVisited.find(cur_node) != ArrayValueVisited.end())
        return;

    ArrayValueVisited.insert(cur_node);

    // Trace the uses of the pointer value or integer generaed by PtrToInt
    for (int i = 0; i < curI->getNumOperands(); ++i)
    {
        Value *tmp_op = curI->getOperand(i);
        TraceAccessForTarget(tmp_op);
    }
    ArrayValueVisited.erase(cur_node);
}

HI_ArrayAccessPattern::ArrayInfo HI_ArrayAccessPattern::getArrayInfo(Value *target)
{

    PointerType *ptr_type = dyn_cast<PointerType>(target->getType());
    *ArrayLog << "\n\nchecking type : " << *ptr_type << " and its ElementType is: ["
              << *ptr_type->getElementType() << "]\n";
    Type *tmp_type = ptr_type->getElementType();
    int total_ele = 1;
    int tmp_dim_size[10];
    int num_dims = 0;
    while (auto array_T = dyn_cast<ArrayType>(tmp_type))
    {
        *ArrayLog << "----- element type of : " << *tmp_type << " is "
                  << *(array_T->getElementType()) << " and the number of its elements is "
                  << (array_T->getNumElements()) << "\n";
        total_ele *= (array_T->getNumElements());
        tmp_dim_size[num_dims] = (array_T->getNumElements());
        num_dims++;
        tmp_type = array_T->getElementType();
    }

    ArrayInfo res_array_info;
    res_array_info.num_dims = num_dims;
    for (int i = 0; i < num_dims; i++)
    {
        res_array_info.dim_size[i] = tmp_dim_size[num_dims - i - 1];
    }

    res_array_info.sub_element_num[0] = 1;
    for (int i = 1; i < num_dims; i++)
    {
        res_array_info.sub_element_num[i] =
            res_array_info.sub_element_num[i - 1] * res_array_info.dim_size[i - 1];
    }

    if (auto arg_v = dyn_cast<Argument>(target))
    {
        res_array_info.sub_element_num[num_dims] =
            res_array_info.sub_element_num[num_dims - 1] * res_array_info.dim_size[num_dims - 1];
        res_array_info.dim_size[num_dims] = 100000000; // set to nearly infinite
        res_array_info.num_dims++;
        res_array_info.isArgument = 1;
    }

    res_array_info.elementType = tmp_type;
    res_array_info.target = target;
    return res_array_info;
}

// find the array declaration in the function F and trace the accesses to them
void HI_ArrayAccessPattern::findMemoryDeclarationin(Function *F, bool isTopFunction)
{
    *ArrayLog << "\n\nchecking the BRAM information in Function: " << F->getName() << "\n";

    // for top function in HLS, arrays in interface may involve BRAM
    if (isTopFunction)
    {
        *ArrayLog << " is Top function "
                  << "\n";
        for (auto it = F->arg_begin(), ie = F->arg_end(); it != ie; ++it)
        {
            if (it->getType()->isPointerTy())
            {
                PointerType *tmp_PtrType = dyn_cast<PointerType>(it->getType());
                if (tmp_PtrType->getElementType()->isArrayTy())
                {
                    Target2ArrayInfo[it] = getArrayInfo(it);
                    *ArrayLog << Target2ArrayInfo[it] << "\n";
                }
            }
        }
    }
    else
    {
        *ArrayLog << " is not top function "
                  << "\n";
    }

    // for general function in HLS, arrays in functions are usually declared with alloca instruction
    for (auto &B : *F)
    {
        for (auto &I : B)
        {
            if (AllocaInst *allocI = dyn_cast<AllocaInst>(&I))
            {
                Target2ArrayInfo[allocI] = getArrayInfo(allocI);
                *ArrayLog << Target2ArrayInfo[allocI] << "\n";
            }
        }
    }
    ArrayLog->flush();
}

std::string HI_ArrayAccessPattern::demangleFunctionName(std::string mangled_name)
{
    std::string demangled_name;

    // demangle the function
    if (mangled_name.find("_Z") == std::string::npos)
        demangled_name = mangled_name;
    else
    {
        std::stringstream iss(mangled_name);
        iss.ignore(1, '_');
        iss.ignore(1, 'Z');
        int len;
        iss >> len;
        while (len--)
        {
            char tc;
            iss >> tc;
            demangled_name += tc;
        }
    }
    return demangled_name;
}

int HI_ArrayAccessPattern::getPartitionFor(HI_ArrayAccessPattern::HI_AccessInfo access,
                                           int partition_factor, int partition_dimension)
{
    int res = -1;
    res = access.index[partition_dimension - 1] % partition_factor;

    return res;
}