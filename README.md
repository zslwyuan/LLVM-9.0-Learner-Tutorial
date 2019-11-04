# LLVM-Learner-Note

A blog for LLVM(v9.0.0) beginner, step by step, with detailed documents and comments. This blog records the way I learn LLVM and finnaly accomplish a complete project with it. Therefore, I will keep updating this blog according to the progress of my project. I am happy to discuss and any suggestion will be welcome ^_^ (tliang@ust.hk). 

LLVM is very interesting tool but it is also upsetting for beginners (e.g. me). In this blog, I begin with the simplest pass construction and invocation, trying to explain the usage of some functions, types and variables. A complete project will be built gradually. I just hope this github can help some people like me to understand and handle LLVM better.  PS: **Clang** provides user with **Python-APIs**, hence some people may use python to handle/analyze/transform the source code. An example project can be found **[here](https://github.com/zslwyuan/Hi-DMM)**.

Please note that all the experiments are built with **LLVM-9.0.0**. In this blog, I tend to make the passes standalone, which means that they can be called in a main source code, but do not implement in the way which requires users compile the pass into dynamic library, re-make the LLVM source code and invoke the pass by "opt" command in the terminal. I think it might not be flexible, convenient and portable for development and testing. **Visual Studio Code** is the suggested editor since you can trace the functions and variables in LLVM very easily with VScode.


## Category:


**[How to use this blog](https://github.com/zslwyuan/LLVM-Learner-Note#how-to-use-this-blog)**

**[List of Experiments I conducted](https://github.com/zslwyuan/LLVM-Learner-Note#list-of-experiments-i-conducted)**

**[Further development](https://github.com/zslwyuan/LLVM-Learner-Note#further-development)**

 
***


## [How to use this blog](https://github.com/zslwyuan/LLVM-Learner-Note#how-to-use-this-blog)

1. download the blog (entire project)
2. basic functiones and passes are implemented in the directory **["Implementations"](https://github.com/zslwyuan/LLVM-Learner-Note/tree/master/Implementations)**. Nearly all the directories have their own README file to explain the directory.
3. experiments are tested in the directory **["Test"](https://github.com/zslwyuan/LLVM-Learner-Note/tree/master/Tests)**.
4. by making a "build" directory and using CMake in each experiment directory (e.g. **[this one](https://github.com/zslwyuan/LLVM-Learner-Note/tree/master/Tests/LLVM_exp5_LoopSimpleAnalysis/)**), executable can be generated and tried. (hint: cmake .. & make) 
5. for user's convenience, I prepare some sripts for example, **BuildAllFiles.sh**, which will build all the projects, **CleanBuiltFiles.sh**, which will clean all the built files to shrink the size of the directories, and **Build.sh** in test directory, which will just build one test project. All these scripts can be run directly.
6. looking into the source code with detailed comments, reader can trace the headers and functions to understand how the experiment work.


## [List of Experiments I conducted](https://github.com/zslwyuan/LLVM-Learner-Note#list-of-experiments-i-conducted):


**Experiment 0**: Find all the functions' names in the program.

      -- keywords: installation of LLVM, basic pass template

**Experiment 1**: Get the dependence graph in a function and plot it.

      -- keywords: dependence types, graph, iterator, passmanager, iterator, types in LLVM

**Experiment 2**: Utilize existing LLVM passes in your project 

      -- keywords: invoke different passes in your application, createXXXXPass (or new xxxxPass), 

**Experiment 3**: Basic Loop information analysis

      -- keywords: co-operation/dependence between Pass, Basic function for loop analysis

**Experiment 4**: Invoke polyhedral Information Pass

      -- keywords: compilation with polly libraries, pass dependence

**Experiment 5**: Build a ASAP scheduler to schedule the instructions for parallel machine (e.g. FPGA) and evaluate latency

      -- keywords: information processing, DFS traverse, comprehensively used the Function/Loop/Block information, basic scheduler implementation
      -- PS: Experiment 5a is just used to collect involved instructions in a IR file.


**Experiment 6**: Analyze the GEP lowering transformation pass (Passed implemented in previous experimented are analysis passes.)

      -- SeparateConstOffsetFromGEP, GEP Lowering, Transformation Pass, DataLayout


**Experiment 7**: Implementation a transformation pass to safely remove duplicated instrcutions  (Passed implemented in previous experimented are analysis passes.)

      -- keywords: erase an instruction, remove duplicatied instruction, eraseFromParent, replaceAllUsesWith

**Experiment 8**: By using SCEV, Successfully obtain the range of the targe value and then implement bitwidth optimization (IR insertion/operand replacement/IR removal/reduncdant instruction check T_T....) (Passed implemented in previous experimented are analysis passes.)

      -- keywords: SCEV, value range, bitwidth optimization, Usage of IR Builder, change type/width of value



**Experiment 9**: Build a GEP operation transformation pass for HLS  (Passed implemented in previous experimented are analysis passes.)

      --- HLS, ByteAlignment->ElementAlignment, SeparateConstOffsetFromGEP, GEP Lowering, Transformation Pass, DataLayout

**usage example**

      When built, most test executables can be used like below but please check the source code for confirmation.
      (1) ./LLVM\_expXXXXX  <C/C++ FILE> <top\_function\_name>   
      or
      (2) ./LLVM\_expXXXXX  <IR FILE>

## [Further development](https://github.com/zslwyuan/LLVM-Learner-Note#further-development)

If you want to do your own works based on this project, following hints might be useful.

1. user can add their passes according to the examples in the directory  **["Implementations"](https://github.com/zslwyuan/LLVM-Learner-Note/tree/master/Implementations)**. 
2. Note for development: (a) remember to define unique marco for header files (like #ifndef _HI_HI_POLLY_INFO);  (b) Modify the CMakelists.txt file in the 4 directories: **the pass directory([example](https://github.com/zslwyuan/LLVM-Learner-Note/tree/master/Implementations/HI_SimpleTimingEvaluation/CMakeLists.txt)), Implementation directory([example](https://github.com/zslwyuan/LLVM-Learner-Note/tree/master/Implementations/CMakeLists.txt)), LLVM_Learner_Libs directory([example](https://github.com/zslwyuan/LLVM-Learner-Note/tree/master/Tests/LLVM_Learner_Libs/CMakeLists.txt)) and the test directory([example](https://github.com/zslwyuan/LLVM-Learner-Note/tree/master/Tests/LLVM_exp5_LoopSimpleAnalysis/CMakeLists.txt))**. The modification should add subdirectory and consider the including path/library name.

## Good Good Study Day Day Up \(^o^)/~
