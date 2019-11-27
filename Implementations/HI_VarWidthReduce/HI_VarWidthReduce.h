#ifndef _HI_VarWidthReduce
#define _HI_VarWidthReduce
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
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Transforms/Utils/Local.h"
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

#include "llvm/IR/Instructions.h"
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

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Analysis/LazyValueInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "HI_print.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>
#include <map>
#include <set>
#include <vector>
#include <sstream>
#include "llvm/Analysis/ScalarEvolution.h"
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
#include "llvm/Analysis/LoopInfo.h"
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
#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <memory>
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
#include <utility>
#include <sys/time.h>

using namespace llvm;

class HI_VarWidthReduce : public FunctionPass {
public:
    HI_VarWidthReduce(const char* VarWidthChangeLog_Name, bool DEBUG=0 ) : FunctionPass(ID), DEBUG(DEBUG)
    {
        Instruction_Counter = 0;
        Function_Counter = 0;
        BasicBlock_Counter = 0;
        Loop_Counter = 0;
        callCounter = 0;
        VarWidthChangeLog = new raw_fd_ostream(VarWidthChangeLog_Name, ErrInfo, sys::fs::F_None);
        tmp_stream = new raw_string_ostream(tmp_stream_str);
        UnsignedRanges.clear();
        SignedRanges.clear();
    } // define a pass, which can be inherited from ModulePass, LoopPass, FunctionPass and etc.

    ~HI_VarWidthReduce()
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
        VarWidthChangeLog->flush();
        delete VarWidthChangeLog;
        tmp_stream->flush(); delete tmp_stream;
    }

    virtual bool doInitialization(Module &M)
    {
        print_status("Initilizing HI_VarWidthReduce pass.");  
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
        Instruction_BitNeeded.clear();

        return false;
    }

    // virtual bool doFinalization(Module &M)
    // {
    //     return false;
    // }

    void getAnalysisUsage(AnalysisUsage &AU) const;
    virtual bool runOnFunction(Function &M);
    static char ID;
    bool DEBUG;
    // Determine the range for a particular SCEV, but bypass the operands generated from PtrToInt Instruction, considering the actual 
    // implementation in HLS
    const ConstantRange HI_getSignedRangeRef(const SCEV *S);

    const ConstantRange HI_getUnsignedRangeRef(const SCEV *S);

    // check whether we should bypass the PtrToInt Instruction
    bool bypassPTI(const SCEV *S);

    // cache constant range for those evaluated SCEVs
    const ConstantRange &setSignedRange(const SCEV *S, ConstantRange CR);

    const ConstantRange &setUnsignedRange(const SCEV *S, ConstantRange CR);

    unsigned int HI_getBidwith(Value *I);

    unsigned int bitNeededFor(ConstantRange CR);

    void ReplaceUses_withNewOperand_newBW(Instruction *from, Value *to);

    void ReplaceUses_withNewOperand_oriBW(Instruction *from, Value *to);
    
    // Analysis: check the value range of the instructions in the source code and determine the bitwidth
    void Bitwidth_Analysis(Function *F);    

    // Forward Process: check the bitwidth of operands and output of an instruction, trunc/ext the operands, update the bitwidth of the instruction
    bool InsturctionUpdate_WidthCast(Function *F);

    // Forward Process of BinaryOperator: check the bitwidth of operands and output of an instruction, trunc/ext the operands, update the bitwidth of the instruction
    void BOI_WidthCast(BinaryOperator *BOI);

    // Forward Process of ICmpInst: check the bitwidth of operands and output of an instruction, trunc/ext the operands, update the bitwidth of the instruction
    void ICMP_WidthCast(ICmpInst *ICMP_I);

    // Forward Process of PHI: check the bitwidth of operands and output of an instruction, trunc/ext the operands, update the bitwidth of the instruction
    void PHI_WidthCast(PHINode *PHI_I);

    // Check Redundancy: Some instructions could be truncated to be an operand, but itself is actually updated with the same bitwidth with the truncation.
    bool RedundantCastRemove(Function *F);

    // Validation Check: Check whether there is any binary operation with operands in different types.
    void VarWidthReduce_Validation(Function *F);

    // copy the metadata from one to another
    void CopyInstMetadata(Instruction *oldI, Instruction *newI);


        
    int callCounter;
    int Instruction_Counter;
    int Function_Counter;
    int BasicBlock_Counter;
    int Loop_Counter;
    unsigned int changed_id = 0;

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
    DenseMap<const SCEV *, ConstantRange> SignedRanges;
    DenseMap<const SCEV *, ConstantRange> UnsignedRanges;
    std::map<Instruction *, unsigned int> Instruction_BitNeeded;

    std::map<Instruction*, bool> I2NeedSign;

    std::error_code ErrInfo;
    raw_ostream *VarWidthChangeLog;

    raw_string_ostream *tmp_stream;
    std::string tmp_stream_str;

    LazyValueInfo* LazyI;
    ScalarEvolution  *SE;

    
/// Timer

    struct timeval tv_begin;
    struct timeval tv_end;

};



#endif