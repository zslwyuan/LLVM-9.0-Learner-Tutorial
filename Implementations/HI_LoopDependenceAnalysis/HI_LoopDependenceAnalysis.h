#ifndef _HI_HI_LoopDependenceAnalysis
#define _HI_HI_LoopDependenceAnalysis
// related headers should be included.
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_print.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IRReader/IRReader.h"
#include <bits/stl_map.h>
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/LoopVersioning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
using namespace llvm;

#include "llvm/Pass.h"
#include "llvm/IR/Type.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/CFG.h"
#include "llvm-c/Target.h"

#include <vector>
#include <deque>
#include <set>
#include <map>
#include <algorithm>
#include <ostream>
#include <fstream> 
#include <iostream>
#include <sys/time.h>

using namespace llvm;

/* A single dependence */
struct DependenceDirection
{
	char direction; // <,=,>, or *
	int distance; //if exists, nonzero
	
	DependenceDirection(): direction('*'), distance(0) {}
};

/*
 A class that represents a dependence matrix for a loop.  This is the union
 of all valid direction vectors of all dependences.
 */
class DependenceMatrix
{
	//implement, feel free to change any data structures
public:
	// DependenceMatrix();
	// DependenceMatrix(int n);
	// void add(const std::vector<DependenceDirection>& vec);
	// //more ?
	// void clear();	
	// void print(	std::ostream& out);
	// bool isTrivial(); //return true if empty (no dependence) or full (all dependencies)
};

class HI_LoopDependenceAnalysis : public LoopPass
{
	friend struct MemOp;
	std::ofstream file;
public:
	static char ID;

	HI_LoopDependenceAnalysis(const char *dep_filename) : LoopPass(ID)
	{
		file.open(dep_filename);
	}

	~HI_LoopDependenceAnalysis()
	{
	}

	virtual bool runOnLoop(Loop *L, LPPassManager &);

	virtual void getAnalysisUsage(AnalysisUsage &AU) const;

private:
	// Various analyses that we use...
	AliasAnalysis *AA; // Current AliasAnalysis information
	LoopInfo *LI; // Current LoopInfo
	std::vector<PHINode*> iVars;
	std::map<PHINode*, int> iVarsIndex;
	std::vector<Loop *> nestedLoops;
	DependenceMatrix depMatrix;

	//return true if L is a perfect nest of loops with nice iteration variables
	//fills in the induction variables in order and the body of the loop nest
	bool isPerfectNest(Loop *L, BasicBlock*& body);

	//return true if b has no memory operations
	bool hasNoMemoryOps(BasicBlock *b);

	//compute the dependency matrix for the inner block
	//implement this
	void computeDependencies(BasicBlock*& body);

    int NumLoopNests = 0;
    int NumPerfectLoopNests = 0;
    int NumIndependentMemOps = 0;
    int NumAmbiguousDependentMemOps = 0;
    int NumDirectionalDependentMemOps = 0;
    int NumInterestingLoops = 0;

    
/// Timer

    struct timeval tv_begin;
    struct timeval tv_end;
};




#endif