# LLVM-9.0-Learner-Tutorial

A blog for LLVM(v9.0.0) beginner, step by step, with detailed documents and comments. (Actually I also implemented [a similar tutorial project for **LLVM-11**](https://github.com/zslwyuan/Light-HLS/tree/llvm11support)). This blog records the way I learn LLVM and finnaly accomplish a complete project with it. Therefore, I will keep updating this blog according to the progress of my project. Currently, my target is to implement a source code optimizer. I am happy to discuss and any suggestion will be welcome ^_^ (tliang@ust.hk). ~If this blog is useful for you, a STAR will be encouragement to me. LOL~ 

The learner note leads to our project for FPGA HLS, **[Light-HLS](https://github.com/zslwyuan/Light-HLS)**, presented in ICCAD 2019. If you are interested in analyzing LLVM from the perspective of project, you can have a look at  **[Light-HLS](https://github.com/zslwyuan/Light-HLS)**. Please note that Light-HLS targets at FPGAs.If Light-HLS helps for your works, please cite [our paper in ICCAD 2019](https://ieeexplore.ieee.org/document/8942136) ^_^: 

    T. Liang, J. Zhao, L. Feng, S. Sinha and W. Zhang, "Hi-ClockFlow: Multi-Clock Dataflow Automation and Throughput Optimization in High-Level Synthesis," 2019 IEEE/ACM International Conference on Computer-Aided Design (ICCAD), Westminster, CO, USA, 2019, pp. 1-6. doi: 10.1109/ICCAD45719.2019.8942136


LLVM is very interesting tool but it is also upsetting for beginners (e.g. me). In this blog, I begin with the simplest pass construction and invocation, trying to explain the usage of some functions, types and variables. A complete project will be built gradually. I just hope this github can help some people like me to understand and handle LLVM better.  PS: **Clang** provides user with **Python-APIs**, hence some people may use python to handle/analyze/transform the source code. An example project can be found **[here](https://github.com/zslwyuan/Hi-DMM)**.

Please note that all the experiments are built with **LLVM-9.0.0**.  In this blog, I tend to make the passes standalone, which means that they can be called in a main source code, but do not implement in the way which requires users compile the pass into dynamic library, re-make the LLVM source code and invoke the pass by "opt" command in the terminal. I think it might not be flexible, convenient and portable for development and testing. **Visual Studio Code** is the suggested editor since you can trace the functions and variables in LLVM very easily with VScode.


## Category:


**[How to use this blog](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial#how-to-use-this-blog)**

**[List of Experiments I conducted](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial#list-of-experiments-i-conducted)**

**[Further development](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial#further-development)**


***

 
## [How to use this blog](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial#how-to-use-this-blog)

1. download the blog (entire project) and install LLVM-9.0.0. If you get problems during install LLVM, I suggest you to use **[the LLVM package with my install shell script](https://github.com/zslwyuan/LLVM-9-for-Light-HLS)**.
2. basic functiones and passes are implemented in the directory **["Implementations"](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations)**. Nearly all the directories have their own README file to explain the directory.
3. experiments are tested in the directory **["Test"](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests)**.
4. by making a "build" directory and using CMake in each experiment directory (e.g. **[this one](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp5_SimpleTimingAnalysis/)**), executable can be generated and tried. (hint: cmake .. & make) 
5. for user's convenience, I prepare some scripts for example, **BuildAllFiles.sh**, which will build all the projects, **CleanBuiltFiles.sh**, which will clean all the built files to shrink the size of the directories, and **Build.sh** in test directory, which will just build one test project. All these scripts can be run directly.
6. looking into the source code with detailed comments, reader can trace the headers and functions to understand how the experiment work.




***



**usage example**

      When built, most test executables can be used like below but please check the source code for confirmation.
      (1) ./LLVM_expXXXXX  <C/C++ FILE> <top_function_name>   
      or
      (2) ./LLVM_expXXXXX  <C/C++ FILE>

      for comprehensive evaluation test:
      (1) ./LLVM_expXXXXX  <C/C++ FILE> <top_function_name> <configuration_file> [DEBUG]


***

## [List of Experiments I conducted](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial#list-of-experiments-i-conducted):

**Experiment 0**: Find all the functions' names in the program. (Analysis Pass) **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_FindFunctions)** **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp0_find_functions)**

      -- keywords: installation of LLVM, basic pass template

**Experiment 1**: Get the dependence graph in a function and plot it. (Analysis Pass) **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_DependenceList)** **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp1_dependence_list)**

      -- keywords: dependence types, graph, iterator, passmanager, iterator, types in LLVM 

**Experiment 2**: Utilize existing LLVM passes in your project  (Analysis Pass)  **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp2_loop_processing)**

      -- keywords: invoke different passes in your application, createXXXXPass (or new xxxxPass), 

**Experiment 3**: Basic Loop information analysis  (Analysis Pass)  **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_LoopInFormationCollect)** **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp3_loop_info_extraction)**

      -- keywords: co-operation/dependence between Pass, Basic function for loop analysis

**Experiment 4**: Invoke polyhedral Information Pass  (Analysis Pass)  **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_Polly_Info)** **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp4_polly_info)**

      -- keywords: compilation with polly libraries, pass dependence

**Experiment 5**: Build a ASAP scheduler to schedule the instructions for parallel machine (e.g. FPGA) and evaluate latency (Analysis Pass) **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_SimpleTimingEvaluation)** **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp5_SimpleTimingAnalysis)**

      -- keywords: information processing, DFS traverse, comprehensively used the Function/Loop/Block information, basic scheduler implementation
      -- PS: Experiment 5a is just used to collect involved instructions in a IR file.


**Experiment 6**: Analyze the GEP lowering transformation pass (Passed implemented in previous experimented are analysis passes.)  **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp6_GEP_Transformation)** **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp6_GEP_Transformation)**

      -- SeparateConstOffsetFromGEP, GEP Lowering, Transformation Pass, DataLayout


**Experiment 7**: Implementation a transformation pass to safely remove duplicated instrcutions (Transformation Pass) **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_DuplicateInstRm)** **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp7_DuplicateInstRemove)**

      -- keywords: erase an instruction, remove duplicatied instruction, eraseFromParent, replaceAllUsesWith

**Experiment 8**: By using SCEV, Successfully obtain the range of the targe value and then implement bitwidth optimization (IR insertion/operand replacement/IR removal/reduncdant instruction check T\_T....) (Transformation Pass)    **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_VarWidthReduce)** **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp8_VarWidthReduce)**

      -- keywords: SCEV, value range, bitwidth optimization, Usage of IR Builder, change type/width of value


**Experiment 9**: Build a GEP operation transformation pass for HLS  (Transformation Pass)  **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_SeparateConstOffsetFromGEP)**  **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp6_GEP_Transformation)**

      --- HLS, ByteAlignment->ElementAlignment, SeparateConstOffsetFromGEP, GEP Lowering, Transformation Pass, DataLayout
      --- WARNING: Currently, array of struct is not supported. In the future, we need to transform array of struct to seperate array.


**Experiment 10**: Hack on Clang to detect arbitary precision integer ap\_int<X> in source code for VivadoHLS and mark them in the source code (Front-end Operation: AST Consumer) **[ACTION CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_APIntSrcAnalysis)**

      --- Front-End Operation, AST Consumer, Visit Nodes in AST, Clang, Template Detection


**Experiment 11**: (Patch-Applied) Hack on Clang to properly map arbitary precision integer ap\_int<X> in source code for VivadoHLS into iX in IR generation  (Front-end Operation: CodeGen) **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_expAPINT_test)**. Since a patch for LLVM-9 necessary for this tutorial is still under the review of LLVM, **[complete source code of LLVM for this tutorial](https://github.com/zslwyuan/LLVM-9-for-Light-HLS)** can be downloaded via github in case that you cannot make the patch work properly. Example of declaring arbitrary precision integer variables is shown [here](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/App/2dloop2darray_pl_apint).

      --- CodeGen Operation, AST Consumer, Visit Nodes in AST, Clang
      --- The original way I implement is too ugly and I applied the patch which Erich Keane shared via maillist clang-cfe.
      --- The patch, under review: https://reviews.llvm.org/D59105 , is developed by Erich Keane, Compiler Engineer, Intel Corporation. (Thanks Erich!^_^)


**Experiment 12**: Based on the library collected from VivadoHLS synthesis result, a Pass for more accurate evaluation of timing and resource is implemented. (Analysis Pass) **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_NoDirectiveTimingResourceEvaluation)**   **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp12_HI_NoDirectiveTimingResourceEvaluation)**

      --- HLS Timing/Resource Library for most instructions, Timing/Resource Evaluation, Operation Chaining for DSP Utilization
	
**Experiment 12(a)**: Based on DominatorTree push instructions back to dominant ancestor to improve parallelism and reduce redundant instructions. (Transformation Pass) **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_IntstructionMoveBackward)**  **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp12_HI_NoDirectiveTimingResourceEvaluation)**

      --- DominatorTree algorithm, parallelism, redudant instruction removal

**Experiment 12(c)**: Since LLVM-provided loop strength reduction (LSR) pass limits itself according to the Target ISA addressing mode, an aggressive LSR pass is implemented for HLS to reduce the cost of DSPs on FPGA. (Transformation Pass) **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_AggressiveLSR_MUL)**  **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp12_HI_NoDirectiveTimingResourceEvaluation)**

      --- Loop Strength Reduction, Analysis based on Scalar Evolution, AddRecExpr, (Aggressive: may not stable for all the situations 0-0, but it works fine temporarily)
      --- Thank Momchil Velikov for his detailed reply about my inquiry about LSR pass

**Experiment 13**: Before involving HLS directives (array partitioning, loop pipelining/unroll), we need to check the memory access pattern of arrays in the source code (Analysis Pass) **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_ArrayAccessPattern)** 

      --- SCEV, Scalar Evolution Interpretation (AddRec, Add, Unknown, Constant), Array Access Pattern

**Experiment 13c**: map IR code back to source code based on DWARF debug information in IR metadata (Instruction=>line in source code; basic block/loop/function=>range in source code) **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_IR2SourceCode)**  **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp13d_HI_LoopUnroll)** 

      --- DWARF Debug Information for the mapping from IR code to Source Code

**Experiment 13d**: unroll the specifc loops according to the label set in the source code and the setting in the configuration file (Transformation Pass) **[PASS CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_LoopUnroll)**  **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_exp13d_HI_LoopUnroll)** 

      --- Loop Unrolling, Debug Information from Source Code

**Experiment 14**: (WIP) Based on the IR file, evaluate the resource and timing of the Window/Buffer/Filter in HLS 

      --- IR transformation, Timing/Resource Evaluation for special structures on FPGA
	
      
**Experiment 15**: A flow of high-level synthesis including source code processing, implementation of HLS directives, IR optimization for FPGA implementation, timing scheduling and resource binding. It consists of a series of front-end IR processing and back-end ([HI\_WithDirectiveTimingResourceEvaluation pass](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_WithDirectiveTimingResourceEvaluation) is used to evaluate the timing and resource of the application, considering the effect of HLS directives.) **[TEST CODE](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/Light_HLS_Top)** (for more information for this flow, please refer to **[Light-HLS](https://github.com/zslwyuan/Light-HLS)**)

      --- Clang Source Code Transformation, IR optimization for FPGA HLS, implementation of HLS directives, timing scheduling and resource binding.

***

## [Further development](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial#further-development)

If you want to do your own works based on this project, following hints might be useful.

1. user can add their passes according to the examples in the directory  **["Implementations"](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations)**. 
2. Note for development: (a) remember to define unique marco for header files (like #ifndef _HI_HI_POLLY_INFO);  (b) Modify the CMakelists.txt file in the 4 directories: **the pass directory([example](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/HI_SimpleTimingEvaluation/CMakeLists.txt)), Implementation directory([example](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Implementations/CMakeLists.txt)), LLVM_Learner_Libs directory([example](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/tree/master/Tests/LLVM_Learner_Libs/CMakeLists.txt)) and the test directory([example](https://github.com/zslwyuan/LLVM-9.0-Learner-Tutorial/blob/master/Tests/LLVM_exp7_DuplicateInstRemove/CMakeLists.txt))**. The modification should add subdirectory and consider the including path/library name.

## Good Good Study Day Day Up \(^o^)/~
