This directory stores all the tests, which have their main functions and can generate standalone executable.

Please note that the Passes in the Implementaions directory will be first compiled into libraries in LLVM_Learner_Libs, for later linking.


***

## [List of Experiments I conducted](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial#list-of-experiments-i-conducted):

**Experiment 0**: Find all the functions' names in the program. (Analysis Pass) **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_FindFunctions)**

      -- keywords: installation of LLVM, basic pass template

**Experiment 1**: Get the dependence graph in a function and plot it. (Analysis Pass) **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_DependenceList)**

      -- keywords: dependence types, graph, iterator, passmanager, iterator, types in LLVM 

**Experiment 2**: Utilize existing LLVM passes in your project  (Analysis Pass)  **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp2_loop_processing)**

      -- keywords: invoke different passes in your application, createXXXXPass (or new xxxxPass), 

**Experiment 3**: Basic Loop information analysis  (Analysis Pass)  **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_LoopInFormationCollect)**

      -- keywords: co-operation/dependence between Pass, Basic function for loop analysis

**Experiment 4**: Invoke polyhedral Information Pass  (Analysis Pass)  **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_Polly_Info)**

      -- keywords: compilation with polly libraries, pass dependence

**Experiment 5**: Build a ASAP scheduler to schedule the instructions for parallel machine (e.g. FPGA) and evaluate latency (Analysis Pass) **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_SimpleTimingEvaluation)**

      -- keywords: information processing, DFS traverse, comprehensively used the Function/Loop/Block information, basic scheduler implementation
      -- PS: Experiment 5a is just used to collect involved instructions in a IR file.


**Experiment 6**: Analyze the GEP lowering transformation pass (Passed implemented in previous experimented are analysis passes.)  **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp6_GEP_Transformation)**

      -- SeparateConstOffsetFromGEP, GEP Lowering, Transformation Pass, DataLayout


**Experiment 7**: Implementation a transformation pass to safely remove duplicated instrcutions (Transformation Pass) **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_DuplicateInstRm)**

      -- keywords: erase an instruction, remove duplicatied instruction, eraseFromParent, replaceAllUsesWith

**Experiment 8**: By using SCEV, Successfully obtain the range of the targe value and then implement bitwidth optimization (IR insertion/operand replacement/IR removal/reduncdant instruction check T\_T....) (Transformation Pass)    **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_VarWidthReduce)** 

      -- keywords: SCEV, value range, bitwidth optimization, Usage of IR Builder, change type/width of value


**Experiment 9**: Build a GEP operation transformation pass for HLS  (Transformation Pass)  **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_SeparateConstOffsetFromGEP)**

      --- HLS, ByteAlignment->ElementAlignment, SeparateConstOffsetFromGEP, GEP Lowering, Transformation Pass, DataLayout


**Experiment 10**: Hack on Clang to detect arbitary precision integer ap\_int<X> in source code for VivadoHLS and mark them in the source code (Front-end Operation: AST Consumer) **[ACTION CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_APIntSrcAnalysis)**

      --- Front-End Operation, AST Consumer, Visit Nodes in AST, Clang, Template Detection


**Experiment 11**: (Patch-Applied) Hack on Clang to properly map arbitary precision integer ap\_int<X> in source code for VivadoHLS into iX in IR generation  (Front-end Operation: CodeGen) **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_expAPINT_test)**

      --- CodeGen Operation, AST Consumer, Visit Nodes in AST, Clang
      --- The original way I implement is too ugly and I applied the patch which Erich Keane shared via maillist clang-cfe.
      --- The patch, under review: https://reviews.llvm.org/D59105 , is developed by Erich Keane, Compiler Engineer, Intel Corporation. (Thanks Erich!^_^)

**Experiment 12**: (WIP) Based on the library collected from VivadoHLS synthesis result, a Pass for more accurate evaluation of timing and resource is implemented. (Analysis Pass) **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_NoDirectiveTimingResourceEvaluation)** 

      --- HLS Timing/Resource Library for most instructions, Timing/Resource Evaluation, Operation Chaining for DSP Utilization
	

**Experiment 13**: (WIP) Based on the IR file, evaluate the resource and timing of the Window/Buffer/Filter in HLS 

      --- IR transformation, Timing/Resource Evaluation for special structures on FPGA
	
