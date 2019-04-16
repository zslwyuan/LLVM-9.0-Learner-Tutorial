# LLVM-9.0-Learner-Tutorial

A blog for LLVM(v9.0.0) beginner, step by step, with detailed documents and comments. This blog records the way I learn LLVM and finnaly accomplish a complete project with it. Therefore, I will keep updating this blog according to the progress of my project. Currently, my target is to implement a source code optimizer. I am happy to discuss and any suggestion will be welcome ^_^ (tliang@ust.hk). ~If this blog is useful for you, a STAR will be encouragement to me. LOL~

LLVM is very interesting tool but it is also upsetting for beginners (e.g. me). In this blog, I begin with the simplest pass construction and invocation, trying to explain the usage of some functions, types and variables. A complete project will be built gradually. I just hope this github can help some people like me to understand and handle LLVM better.  PS: **Clang** provides user with **Python-APIs**, hence some people may use python to handle/analyze/transform the source code. An example project can be found **[here](https://github.com/zslwyuan/Hi-DMM)**.

Please note that all the experiments are built with **LLVM-9.0.0**. In this blog, I tend to make the passes standalone, which means that they can be called in a main source code, but do not implement in the way which requires users compile the pass into dynamic library, re-make the LLVM source code and invoke the pass by "opt" command in the terminal. I think it might not be flexible, convenient and portable for development and testing. **Visual Studio Code** is the suggested editor since you can trace the functions and variables in LLVM very easily with VScode.


## Category:


**[How to use this blog](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial#how-to-use-this-blog)**

**[List of Experiments I conducted](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial#list-of-experiments-i-conducted)**

**[Further development](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial#further-development)**


***

 
## [How to use this blog](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial#how-to-use-this-blog)

1. download the blog (entire project)
2. basic functiones and passes are implemented in the directory **["Implementations"](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations)**. Nearly all the directories have their own README file to explain the directory.
3. experiments are tested in the directory **["Test"](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests)**.
4. by making a "build" directory and using CMake in each experiment directory (e.g. **[this one](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp5_SimpleTimingAnalysis/)**), executable can be generated and tried. (hint: cmake .. & make) 
5. for user's convenience, I prepare some scripts for example, **BuildAllFiles.sh**, which will build all the projects, **CleanBuiltFiles.sh**, which will clean all the built files to shrink the size of the directories, and **Build.sh** in test directory, which will just build one test project. All these scripts can be run directly.
6. looking into the source code with detailed comments, reader can trace the headers and functions to understand how the experiment work.




***



**usage example**

      When built, most test executables can be used like below but please check the source code for confirmation.
      (1) ./LLVM\_expXXXXX  <C/C++ FILE> <top\_function\_name>   
      or
      (2) ./LLVM\_expXXXXX  <IR FILE>


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
	
**Experiment 12(a)**: Based on DominatorTree push instructions back to dominant ancestor to improve parallelism and reduce redundant instructions. (Transformation Pass) **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_IntstructionMoveBackward)** 

      --- DominatorTree algorithm, parallelism, redudant instruction removal

**Experiment 12(b)**: (WIP)Since LLVM-provided loop strength reduction (LSR) pass limits itself according to the Target ISA addressing mode, an aggressive LSR pass is implemented for HLS to reduce the cost of DSPs on FPGA. (Transformation Pass) 

      --- Loop Strength Reduction (Aggressive: may not stable for all the situations 0-0)

**Experiment 14**: (WIP) Based on the IR file, evaluate the resource and timing of the Window/Buffer/Filter in HLS 

      --- IR transformation, Timing/Resource Evaluation for special structures on FPGA
	
      


***

## [Further development](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial#further-development)

If you want to do your own works based on this project, following hints might be useful.

1. user can add their passes according to the examples in the directory  **["Implementations"](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations)**. 
2. Note for development: (a) remember to define unique marco for header files (like #ifndef _HI_HI_POLLY_INFO);  (b) Modify the CMakelists.txt file in the 4 directories: **the pass directory([example](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_SimpleTimingEvaluation/CMakeLists.txt)), Implementation directory([example](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/CMakeLists.txt)), LLVM_Learner_Libs directory([example](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_Learner_Libs/CMakeLists.txt)) and the test directory([example](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/blob/master/Tests/LLVM_exp7_DuplicateInstRemove/CMakeLists.txt))**. The modification should add subdirectory and consider the including path/library name.

## Good Good Study Day Day Up \(^o^)/~
