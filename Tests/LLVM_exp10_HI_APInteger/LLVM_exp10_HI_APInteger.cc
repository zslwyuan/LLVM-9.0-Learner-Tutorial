#include "LLVM_exp10_HI_APInteger.h"
#include <iostream>
#include <sstream>
#include <string>
#include<cstdlib>
#include <sstream>
#include <string>
#include <fstream>
#include "HI_SysExec.h"
#include "HI_print.h"

void ReplaceAll(std::string& strSource, const std::string& strOld, const std::string& strNew)
{
    int nPos = 0;
    while ((nPos = strSource.find(strOld, nPos)) != strSource.npos)
    {
        strSource.replace(nPos, strOld.length(), strNew);
        nPos += strNew.length();
    }
} 

void pathAdvice()
{
    std::cout<< "===============================================================================" << std::endl;
    std::cout<< "if undefined reference occurs, please check whether the following include paths are required." << std::endl;
    std::string line;
    std::string cmd_str = "clang++ ../testcase/test.c  -v 2> ciinfor";
    print_cmd(cmd_str.c_str());
    sysexec(cmd_str.c_str()); 
    std::ifstream infile("ciinfor");
    while (std::getline(infile, line))
    {
        if (line.find("#include <...> search starts here")!=std::string::npos)
        {
            while (std::getline(infile, line))
            {
                if (line.find("End of search list.")!=std::string::npos)
                {                                        
                    break;
                }
                else
                {
                    ReplaceAll(line," ","");
                    ReplaceAll(line,"\n","");                        
                  //  CI.getHeaderSearchOpts().AddPath(line,frontend::ExternCSystem,false,true);
                    line = "Potential Path : " + line;
                    print_info(line.c_str());
                }
                
            }
            break;
        }
    }
    std::cout<< "===============================================================================" << std::endl;
}




static llvm::cl::OptionCategory StatSampleCategory("Stat Sample");
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;

int main(int argc, const char **argv) {

    pathAdvice();

    // parse the command-line args passed to your code
    CommonOptionsParser op(argc, argv, StatSampleCategory);     

    Rewriter rewriter;
    HI_APIntSrcTrans_Creator *creator = new HI_APIntSrcTrans_Creator(&rewriter);   
    // create a new Clang Tool instance (a LibTooling environment)
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    std::cout<<"tag 1" << std::endl;
    // run the Clang Tool, creating a new FrontendAction (explained below)
    int result = Tool.run(HI_Rewrite_newFrontendActionFactory<HI_APIntSrcTrans_Creator>(creator,nullptr).get());
    std::cout<<"tag 2" << std::endl;
    // print out the rewritten source code ("rewriter" is a global var.)
    rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(llvm::errs());
    return 0;
}





// using namespace clang;
// // By implementing RecursiveASTVisitor, we can specify which AST nodes
// // we're interested in by overriding relevant methods.
// class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor>
// {
// public:
//     MyASTVisitor(Rewriter &R)
//         : TheRewriter(R)
//     {}

//     bool VisitStmt(Stmt *s) {
//         // Only care about If statements.
//         if (isa<IfStmt>(s)) {
//             IfStmt *IfStatement = cast<IfStmt>(s);
//             Stmt *Then = IfStatement->getThen();

//             TheRewriter.InsertText(Then->getBeginLoc(),
//                                    "// the 'if' part\n",
//                                    true, true);

//             Stmt *Else = IfStatement->getElse();
//             if (Else)
//                 TheRewriter.InsertText(Else->getBeginLoc(),
//                                        "// the 'else' part\n",
//                                        true, true);
//         }

//         return true;
//     }

//     bool VisitFunctionDecl(FunctionDecl *f) {
//         // Only function definitions (with bodies), not declarations.
//         if (f->hasBody()) {
//             Stmt *FuncBody = f->getBody();

//             // Type name as string
//             QualType QT = f->getReturnType();
//             std::string TypeStr = QT.getAsString();

//             // Function name
//             DeclarationName DeclName = f->getNameInfo().getName();
//             std::string FuncName = DeclName.getAsString();

//             // Add comment before
//             std::stringstream SSBefore("");
//             SSBefore << "// Begin function " << FuncName << " returning "
//                      << TypeStr << "\n";
//             SourceLocation ST = f->getSourceRange().getBegin();
//             TheRewriter.InsertText(ST, SSBefore.str(), true, true);

//             // And after
//             std::stringstream SSAfter("");
//             SSAfter << "\n// End function " << FuncName << "\n";
//             ST = FuncBody->getEndLoc().getLocWithOffset(1);
//             TheRewriter.InsertText(ST, SSAfter.str(), true, true);
//         }

//         return true;
//     }

// private:
//     void AddBraces(Stmt *s);

//     Rewriter &TheRewriter;
// };


// // Implementation of the ASTConsumer interface for reading an AST produced
// // by the Clang parser.
// class MyASTConsumer : public ASTConsumer
// {
// public:
//     MyASTConsumer(Rewriter &R)
//         : Visitor(R)
//     {}

//     // Override the method that gets called for each parsed top-level
//     // declaration.
//     virtual bool HandleTopLevelDecl(DeclGroupRef DR) 
//     {
//         for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b)
//         {
//             // Traverse the declaration using our AST visitor.
//             Visitor.TraverseDecl(*b);
//         }
//         return true;
//     }

// private:
//     MyASTVisitor Visitor;
// };


// std::string transform(std::string fileName) 
// {

//     std::string cmd_str = "clang++ ../testcase/test.c  -v 2> ciinfor";
//     print_cmd(cmd_str.c_str());
//     bool result = sysexec(cmd_str.c_str()); 
//     std::ifstream infile("ciinfor");

//     CompilerInstance compilerInstance;
//     compilerInstance.createDiagnostics();

//     CompilerInvocation & invocation = compilerInstance.getInvocation();

//     // Initialize target info with the default triple for our platform.
//     auto TO = std::make_shared<TargetOptions>();
//     TO->Triple = llvm::sys::getDefaultTargetTriple();
//     TargetInfo* targetInfo =
//         TargetInfo::CreateTargetInfo(compilerInstance.getDiagnostics(), TO);
//     compilerInstance.setTarget(targetInfo);

//     compilerInstance.createFileManager();
//     auto& fileManager = compilerInstance.getFileManager();

//     compilerInstance.createSourceManager(fileManager);
//     auto& sourceManager = compilerInstance.getSourceManager();

//     LangOptions langOpts; 
//     langOpts.GNUMode = 1;  
//     langOpts.CXXExceptions = 1;  
//     langOpts.RTTI = 1;  
//     langOpts.Bool = 1;   // <-- Note the Bool option here !
//     langOpts.CPlusPlus = 1;  
    


//         std::string line;
//         while (std::getline(infile, line))
//         {
//             if (line.find("#include <...> search starts here")!=std::string::npos)
//             {
//                 while (std::getline(infile, line))
//                 {
//                     if (line.find("End of search list.")!=std::string::npos)
//                     {                                        
//                         break;
//                     }
//                     else
//                     {
//                         ReplaceAll(line," ","");
//                         ReplaceAll(line,"\n","");
//                         compilerInstance.getHeaderSearchOpts().AddPath(line,frontend::ExternCSystem,false,true);
//                     }
                    
//                 }
//                 break;
//             }
//         }


//     PreprocessorOptions &PPOpts = compilerInstance.getPreprocessorOpts();


//     std::cout << TO->Triple<< std::endl; 
//     std::cout << (llvm::Triple(TO->Triple)).getOSName().str() << std::endl;
//     invocation.setLangDefaults(langOpts, 
//                                 clang::InputKind(),
//                                 llvm::Triple(TO->Triple), 
//                                 PPOpts,
//                                 clang::LangStandard::lang_c11); 


//     /*

//     static void setLangDefaults(LangOptions &Opts, InputKind IK,
//                     const llvm::Triple &T, PreprocessorOptions &PPOpts,
//                     LangStandard::Kind LangStd = LangStandard::lang_unspecified);

//     */
        
//     compilerInstance.createPreprocessor(TU_Complete);
//     compilerInstance.createASTContext();

//     // A Rewriter helps us manage the code rewriting task.
//     auto rewriter = clang::Rewriter(sourceManager, compilerInstance.getLangOpts());

//     // Set the main file handled by the source manager to the input file.
//     const FileEntry* inputFile = fileManager.getFile(fileName);
//     sourceManager.setMainFileID(
//         sourceManager.createFileID(inputFile, SourceLocation(), SrcMgr::C_User));
//     compilerInstance.getDiagnosticClient().BeginSourceFile(
//         compilerInstance.getLangOpts(), &compilerInstance.getPreprocessor());

//     // Create an AST consumer instance which is going to get called by
//     // ParseAST.
//     MyASTConsumer consumer(rewriter);

//     // Parse the file to AST, registering our consumer as the AST consumer.
//     clang::ParseAST(
//         compilerInstance.getPreprocessor(), 
//         &consumer, 
//         compilerInstance.getASTContext());
        
//     // At this point the rewriter's buffer should be full with the rewritten
//     // file contents.
//     const RewriteBuffer* buffer = rewriter.getRewriteBufferFor(sourceManager.getMainFileID());

//     return std::string(buffer->begin(), buffer->end());
// }

// int main()
// {
//     std::cout << transform("../testcase/test.c") << std::endl;
//     // using clang::CompilerInstance;
//     // using clang::TargetOptions;
//     // using clang::TargetInfo;
//     // using clang::FileEntry;
//     // using clang::Token;
//     // using clang::ASTContext;
//     // using clang::ASTConsumer;
//     // using clang::Parser;
//     // using clang::DiagnosticOptions;
//     // using clang::TextDiagnosticPrinter;

//     // CompilerInstance ci;
//     // DiagnosticOptions diagnosticOptions;
//     // ci.createDiagnostics();

//     // std::shared_ptr<clang::TargetOptions> pto = std::make_shared<clang::TargetOptions>();
//     // pto->Triple = llvm::sys::getDefaultTargetTriple();
//     // TargetInfo *pti = TargetInfo::CreateTargetInfo(ci.getDiagnostics(), pto);
//     // ci.setTarget(pti);

//     // ci.createFileManager();
//     // ci.createSourceManager(ci.getFileManager());
//     // ci.createPreprocessor(clang::TU_Complete);
//     // PreprocessorOptions &PO = (ci.getPreprocessorOpts());
//     // PO.UsePredefines = false;

//     // ci.setASTConsumer(llvm::make_unique<ASTConsumer>());

//     // ci.createASTContext();
//     // ci.createSema(clang::TU_Complete, NULL);

//     // const FileEntry *pFile = ci.getFileManager().getFile("test.c");
//     // ci.getSourceManager().setMainFileID( ci.getSourceManager().createFileID( pFile, clang::SourceLocation(), clang::SrcMgr::C_User));
//     // clang::ParseAST(ci.getSema());
//     // ci.getASTContext().Idents.PrintStats();

//     return 0;
// }