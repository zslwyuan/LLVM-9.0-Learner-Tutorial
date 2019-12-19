#ifndef _HI_ArrayAccessPattern
#define _HI_ArrayAccessPattern
// related headers should be included.
#include "HI_print.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Value.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Local.h"
#include <ios>
#include <map>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "llvm/ADT/None.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Twine.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/AtomicOrdering.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include <algorithm>

#include "HI_print.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/EquivalenceClasses.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/ADT/None.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/ScopeExit.h"
#include "llvm/ADT/Sequence.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/LazyValueInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/ConstantRange.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Use.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/KnownBits.h"
#include "llvm/Support/SaveAndRestore.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Scalar.h"
#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ios>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Optional.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/KnownBits.h"

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/ConstantRange.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Pass.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Compiler.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <sys/time.h>
#include <utility>

using namespace llvm;

class HI_ArrayAccessPattern : public FunctionPass
{
  public:
    HI_ArrayAccessPattern(const char *ArrayLog_Name, std::string _top_function_name)
        : FunctionPass(ID), top_function_name(_top_function_name)
    {
        ArrayLog = new raw_fd_ostream(ArrayLog_Name, ErrInfo, sys::fs::F_None);
        tmp_stream = new raw_string_ostream(tmp_stream_str);
    } // define a pass, which can be inherited from ModulePass, LoopPass, FunctionPass and etc.

    ~HI_ArrayAccessPattern()
    {
        ArrayLog->flush();
        delete ArrayLog;
        tmp_stream->flush();
        delete tmp_stream;
    }

    virtual bool doInitialization(Module &M)
    {
        print_status("Initilizing HI_ArrayAccessPattern pass.");
        return false;
    }

    // virtual bool doFinalization(Module &M)
    // {
    //     return false;
    // }

    void getAnalysisUsage(AnalysisUsage &AU) const;
    virtual bool runOnFunction(Function &M);
    static char ID;

    class ArrayInfo;
    class HI_AccessInfo;

    // find the array access in the function F and trace the accesses to them
    void findMemoryAccessin(Function *F);

    // find out which instrctuins are related to the array, going through PtrToInt, Add, IntToPtr,
    // Store, Load instructions
    void TraceAccessForTarget(Value *cur_node);

    // check the memory access in the function
    void TraceMemoryAccessinFunction(Function &F);

    bool ArrayAccessOffset(Instruction *I, ScalarEvolution *SE, bool isTopFunction);

    void findMemoryDeclarationin(Function *F, bool isTopFunction);

    std::string demangleFunctionName(std::string mangled_name);

    const SCEV *findTheActualStartValue(const SCEVAddRecExpr *S);

    HI_AccessInfo getAccessInfoFor(Value *target, int initial_offset);

    int getPartitionFor(HI_AccessInfo access, int partition_factor, int partition_dimension);

    class ArrayInfo
    {
      public:
        int dim_size[10];
        int sub_element_num[10];
        int num_dims;
        bool isArgument = 0;
        Type *elementType;
        Value *target;
        ArrayInfo()
        {
            num_dims = -1;
        }
        ArrayInfo(const ArrayInfo &input)
        {
            elementType = input.elementType;
            num_dims = input.num_dims;
            target = input.target;
            isArgument = input.isArgument;
            for (int i = 0; i < num_dims; i++)
                dim_size[i] = input.dim_size[i];
            for (int i = 0; i < num_dims; i++)
                sub_element_num[i] = input.sub_element_num[i];
        }
        ArrayInfo &operator=(const ArrayInfo &input)
        {
            elementType = input.elementType;
            num_dims = input.num_dims;
            target = input.target;
            isArgument = input.isArgument;
            for (int i = 0; i < num_dims; i++)
                dim_size[i] = input.dim_size[i];
            for (int i = 0; i < num_dims; i++)
                sub_element_num[i] = input.sub_element_num[i];
        }
    };

    class HI_AccessInfo
    {
      public:
        int dim_size[10];
        int sub_element_num[10];
        int index[10];
        int num_dims;
        bool isArgument = 0;
        Type *elementType;
        Value *target;
        HI_AccessInfo()
        {
            num_dims = -1;
        }
        HI_AccessInfo(const HI_AccessInfo &input)
        {
            elementType = input.elementType;
            num_dims = input.num_dims;
            target = input.target;
            isArgument = input.isArgument;
            for (int i = 0; i < 10; i++)
            {
                dim_size[i] = -1;
                sub_element_num[i] = -1;
                index[i] = -1;
            }
            for (int i = 0; i < num_dims; i++)
                dim_size[i] = input.dim_size[i];
            for (int i = 0; i < num_dims; i++)
                sub_element_num[i] = input.sub_element_num[i];
            for (int i = 0; i < num_dims; i++)
                index[i] = input.index[i];
        }
        HI_AccessInfo(const ArrayInfo &input)
        {
            elementType = input.elementType;
            num_dims = input.num_dims;
            target = input.target;
            isArgument = input.isArgument;
            for (int i = 0; i < 10; i++)
            {
                dim_size[i] = -1;
                sub_element_num[i] = -1;
                index[i] = -1;
            }
            for (int i = 0; i < num_dims; i++)
                dim_size[i] = input.dim_size[i];
            for (int i = 0; i < num_dims; i++)
                sub_element_num[i] = input.sub_element_num[i];
        }
        HI_AccessInfo &operator=(const HI_AccessInfo &input)
        {
            elementType = input.elementType;
            num_dims = input.num_dims;
            target = input.target;
            isArgument = input.isArgument;
            for (int i = 0; i < 10; i++)
            {
                dim_size[i] = -1;
                sub_element_num[i] = -1;
                index[i] = -1;
            }
            for (int i = 0; i < num_dims; i++)
                dim_size[i] = input.dim_size[i];
            for (int i = 0; i < num_dims; i++)
                sub_element_num[i] = input.sub_element_num[i];
            for (int i = 0; i < num_dims; i++)
                index[i] = input.index[i];
        }
        HI_AccessInfo &operator=(const ArrayInfo &input)
        {
            elementType = input.elementType;
            num_dims = input.num_dims;
            target = input.target;
            isArgument = input.isArgument;
            for (int i = 0; i < 10; i++)
            {
                dim_size[i] = -1;
                sub_element_num[i] = -1;
                index[i] = -1;
            }
            for (int i = 0; i < num_dims; i++)
                dim_size[i] = input.dim_size[i];
            for (int i = 0; i < num_dims; i++)
                sub_element_num[i] = input.sub_element_num[i];
        }
    };

    friend raw_ostream &operator<<(raw_ostream &stream, const ArrayInfo &tb)
    {
        stream << "ArrayInfo for: <<" << *tb.target << ">> [ele_Type= " << *tb.elementType
               << ", num_dims=" << tb.num_dims << ", ";
        for (int i = 0; i < tb.num_dims; i++)
        {
            stream << "dim-" << i << "-size=" << tb.dim_size[i] << ", ";
        }

        for (int i = 0; i < tb.num_dims; i++)
        {
            stream << "dim-" << i << "-subnum=" << tb.sub_element_num[i] << ", ";
        }
        stream << "] ";
        // timing="<<tb.timing<<"] ";
        return stream;
    }

    friend raw_ostream &operator<<(raw_ostream &stream, const HI_AccessInfo &tb)
    {
        stream << "HI_AccessInfo for: <<" << *tb.target << ">> [ele_Type= " << *tb.elementType
               << ", num_dims=" << tb.num_dims << ", ";
        for (int i = 0; i < tb.num_dims; i++)
        {
            stream << "dim-" << i << "-size=" << tb.dim_size[i] << ", ";
        }

        for (int i = 0; i < tb.num_dims; i++)
        {
            stream << "dim-" << i << "-subnum=" << tb.sub_element_num[i] << ", ";
        }

        for (int i = 0; i < tb.num_dims; i++)
        {
            stream << "dim-" << i << "-index=" << tb.index[i] << ", ";
        }

        stream << "representation: " << tb.target->getName() << "[";

        for (int i = tb.num_dims - 1; i > 0; i--)
        {
            stream << tb.index[i] << "][";
        }
        stream << tb.index[0] << "]   ";
        stream << "] ";
        // timing="<<tb.timing<<"] ";
        return stream;
    }

    std::map<Value *, ArrayInfo> Target2ArrayInfo;

    std::map<Instruction *, HI_AccessInfo> Inst2AccessInfo;

    ArrayInfo getArrayInfo(Value *target);

    std::string top_function_name;

    Function *TargeFunction;

    std::set<Value *> ArrayValueVisited;
    std::set<Instruction *> Inst_AccessRelated;

    std::error_code ErrInfo;
    raw_ostream *ArrayLog;

    raw_string_ostream *tmp_stream;
    std::string tmp_stream_str;

    /// Timer

    struct timeval tv_begin;
    struct timeval tv_end;
};

#endif
