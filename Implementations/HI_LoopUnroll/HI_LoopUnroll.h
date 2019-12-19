#ifndef _HI_LoopUnroll
#define _HI_LoopUnroll
// related headers should be included.
#include "HI_print.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
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
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Value.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/LoopVersioning.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include <bits/stl_map.h>
#include <ios>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "llvm-c/Target.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Scalar.h"

#include "HI_print.h"
#include <ios>
#include <map>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "llvm/Analysis/CodeMetrics.h"
#include <sys/time.h>

using namespace llvm;

class HI_LoopUnroll : public LoopPass
{
  public:
    static char ID; // Pass ID, replacement for typeid

    int OptLevel;

    /// If false, use a cost model to determine whether unrolling of a loop is
    /// profitable. If true, only loops that explicitly request unrolling via
    /// metadata are considered. All other loops are skipped.
    bool OnlyWhenForced;

    Optional<unsigned> ProvidedCount;
    Optional<unsigned> ProvidedThreshold;
    Optional<bool> ProvidedAllowPartial;
    Optional<bool> ProvidedRuntime;
    Optional<bool> ProvidedUpperBound;
    Optional<bool> ProvidedAllowPeeling;

    std::map<std::string, std::string> &IRLoop2LoopLabel;
    std::map<std::string, int> &LoopLabel2UnrollFactor;

    HI_LoopUnroll(std::map<std::string, std::string> &_IRLoop2LoopLabel,
                  std::map<std::string, int> &_LoopLabel2UnrollFactor, int OptLevel = 2,
                  bool OnlyWhenForced = false, Optional<unsigned> Threshold = None,
                  Optional<unsigned> Count = None, Optional<bool> AllowPartial = None,
                  Optional<bool> Runtime = None, Optional<bool> UpperBound = None,
                  Optional<bool> AllowPeeling = None)
        : LoopPass(ID), OptLevel(OptLevel), OnlyWhenForced(OnlyWhenForced),
          ProvidedCount(std::move(Count)), ProvidedThreshold(Threshold),
          ProvidedAllowPartial(AllowPartial), ProvidedRuntime(Runtime),
          ProvidedUpperBound(UpperBound), ProvidedAllowPeeling(AllowPeeling),
          IRLoop2LoopLabel(_IRLoop2LoopLabel), LoopLabel2UnrollFactor(_LoopLabel2UnrollFactor)
    {

        LoopUnrollLog = new raw_fd_ostream("LoopUnrollLog", ErrInfo, sys::fs::F_None);
    }

    ~HI_LoopUnroll()
    {
        LoopUnrollLog->flush();
        delete LoopUnrollLog;
    }

    virtual bool runOnLoop(Loop *L, LPPassManager &LPM);

    LoopUnrollResult
    tryToUnrollLoop(Loop *L, DominatorTree &DT, LoopInfo *LI, ScalarEvolution &SE,
                    const TargetTransformInfo &TTI, AssumptionCache &AC,
                    OptimizationRemarkEmitter &ORE, bool PreserveLCSSA, int OptLevel,
                    bool OnlyWhenForced, int ProvidedCount, Optional<unsigned> ProvidedThreshold,
                    Optional<bool> ProvidedAllowPartial, Optional<bool> ProvidedRuntime,
                    Optional<bool> ProvidedUpperBound, Optional<bool> ProvidedAllowPeeling);

    void getAnalysisUsage(AnalysisUsage &AU) const override
    {
        AU.addRequired<AssumptionCacheTracker>();
        AU.addRequired<TargetTransformInfoWrapperPass>();
        // FIXME: Loop passes are required to preserve domtree, and for now we just
        // recreate dom info if anything gets unrolled.
        getLoopAnalysisUsage(AU);
    }

    unsigned int UnrollMaxUpperBound = 8;

    std::error_code ErrInfo;
    raw_ostream *LoopUnrollLog;

    /// Timer

    struct timeval tv_begin;
    struct timeval tv_end;
};

// class HI_LoopUnroll : public FunctionPass {
// public:
//     HI_LoopUnroll(const char* LoopUnrollLog_Name ) : FunctionPass(ID)
//     {

//         LoopUnrollLog = new raw_fd_ostream(LoopUnrollLog_Name, ErrInfo, sys::fs::F_None);
//         tmp_stream = new raw_string_ostream(tmp_stream_str);
//     } // define a pass, which can be inherited from ModulePass, LoopPass, FunctionPass and etc.

//     ~HI_LoopUnroll()
//     {
//         LoopUnrollLog->flush(); delete LoopUnrollLog;
//         tmp_stream->flush(); delete tmp_stream;
//     }

//     virtual bool doInitialization(Module &M)
//     {
//         print_status("Initilizing HI_LoopUnroll pass.");
//         return false;
//     }

//     // virtual bool doFinalization(Module &M)
//     // {
//     //     return false;
//     // }

//     void getAnalysisUsage(AnalysisUsage &AU) const;
//     virtual bool runOnFunction(Function &M);
//     static char ID;

//     std::error_code ErrInfo;
//     raw_ostream *LoopUnrollLog;

//     raw_string_ostream *tmp_stream;
//     std::string tmp_stream_str;

// };

#endif
