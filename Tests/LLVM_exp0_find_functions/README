The HI_FindFunction pass is mainlyed tested by LLVM_exp0_find_functions. 

HI_FindFunction Pass is just a HelloWorld-level pass, showing the template of the construction of Pass in LLVM. 
Detailed explanation can be found in the source code's comments.
The test code will replace the function name in the IR code with the demangled ones.

The test can be run with the following command:

    ./LLVM_expXXXXX  <C/C++ FILE>

    

LLVM installation hint:

1. I build the following directory hierarchy:
   ./LLVM
   ./LLVM/LLVM_src
   ./LLVM/LLVM_build

2. Enter the LLVM_src directory and downloand LLVM-9.0 by following command:
   git clone https://github.com/llvm/llvm-project.git

3. Enter the LLVM_build directory and first do the CMAKE jobs by the following command:
   cmake -G "Unix Makefiles" -DLLVM_ENABLE_PROJECTS="clang;lldb;compiler-rt;lld;polly;debuginfo-tests" ../LLVM_src/llvm/

4. Then build LLVM by:
   make -j8

5. Then install LLVM by:
   sudo make install

ps: There could be some dependences you need to solve but fortunately LLVM will give you the information during installation.
