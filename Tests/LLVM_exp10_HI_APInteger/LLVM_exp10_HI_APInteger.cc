#include <iostream>

#include "llvm/Support/Host.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"

#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Parse/Parser.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include <iostream>
#include <sstream>
#include <string>
#include<cstdlib>


using namespace clang;
// By implementing RecursiveASTVisitor, we can specify which AST nodes
// we're interested in by overriding relevant methods.
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor>
{
public:
    MyASTVisitor(Rewriter &R)
        : TheRewriter(R)
    {}

    bool VisitStmt(Stmt *s) {
        // Only care about If statements.
        if (isa<IfStmt>(s)) {
            IfStmt *IfStatement = cast<IfStmt>(s);
            Stmt *Then = IfStatement->getThen();

            TheRewriter.InsertText(Then->getBeginLoc(),
                                   "// the 'if' part\n",
                                   true, true);

            Stmt *Else = IfStatement->getElse();
            if (Else)
                TheRewriter.InsertText(Else->getBeginLoc(),
                                       "// the 'else' part\n",
                                       true, true);
        }

        return true;
    }

    bool VisitFunctionDecl(FunctionDecl *f) {
        // Only function definitions (with bodies), not declarations.
        if (f->hasBody()) {
            Stmt *FuncBody = f->getBody();

            // Type name as string
            QualType QT = f->getReturnType();
            std::string TypeStr = QT.getAsString();

            // Function name
            DeclarationName DeclName = f->getNameInfo().getName();
            std::string FuncName = DeclName.getAsString();

            // Add comment before
            std::stringstream SSBefore("");
            SSBefore << "// Begin function " << FuncName << " returning "
                     << TypeStr << "\n";
            SourceLocation ST = f->getSourceRange().getBegin();
            TheRewriter.InsertText(ST, SSBefore.str(), true, true);

            // And after
            std::stringstream SSAfter("");
            SSAfter << "\n// End function " << FuncName << "\n";
            ST = FuncBody->getEndLoc().getLocWithOffset(1);
            TheRewriter.InsertText(ST, SSAfter.str(), true, true);
        }

        return true;
    }

private:
    void AddBraces(Stmt *s);

    Rewriter &TheRewriter;
};


// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class MyASTConsumer : public ASTConsumer
{
public:
    MyASTConsumer(Rewriter &R)
        : Visitor(R)
    {}

    // Override the method that gets called for each parsed top-level
    // declaration.
    virtual bool HandleTopLevelDecl(DeclGroupRef DR) {
        for (DeclGroupRef::iterator b = DR.begin(), e = DR.end();
             b != e; ++b)
            // Traverse the declaration using our AST visitor.
            Visitor.TraverseDecl(*b);
        return true;
    }

private:
    MyASTVisitor Visitor;
};


std::string transform(std::string fileName) {

  CompilerInstance compilerInstance;
  compilerInstance.createDiagnostics();

  CompilerInvocation & invocation = compilerInstance.getInvocation();

  // Initialize target info with the default triple for our platform.
  auto TO = std::make_shared<TargetOptions>();
  TO->Triple = llvm::sys::getDefaultTargetTriple();
  TargetInfo* targetInfo =
      TargetInfo::CreateTargetInfo(compilerInstance.getDiagnostics(), TO);
  compilerInstance.setTarget(targetInfo);

  compilerInstance.createFileManager();
  auto& fileManager = compilerInstance.getFileManager();

  compilerInstance.createSourceManager(fileManager);
  auto& sourceManager = compilerInstance.getSourceManager();

  LangOptions langOpts; 
  langOpts.GNUMode = 1;  
  langOpts.CXXExceptions = 1;  
  langOpts.RTTI = 1;  
  langOpts.Bool = 1;   // <-- Note the Bool option here !
  langOpts.CPlusPlus = 1;  
  PreprocessorOptions &PPOpts = compilerInstance.getPreprocessorOpts();

  invocation.setLangDefaults(langOpts, 
                             clang::InputKind(),
                             llvm::Triple(TO->Triple), 
                             PPOpts,
                             clang::LangStandard::lang_c11); 


/*

  static void setLangDefaults(LangOptions &Opts, InputKind IK,
                   const llvm::Triple &T, PreprocessorOptions &PPOpts,
                   LangStandard::Kind LangStd = LangStandard::lang_unspecified);

*/

  compilerInstance.createPreprocessor(TU_Module);
  compilerInstance.createASTContext();

  // A Rewriter helps us manage the code rewriting task.
  auto rewriter = clang::Rewriter(sourceManager, compilerInstance.getLangOpts());

  // Set the main file handled by the source manager to the input file.
  const FileEntry* inputFile = fileManager.getFile(fileName);
  sourceManager.setMainFileID(
      sourceManager.createFileID(inputFile, SourceLocation(), SrcMgr::C_User));
  compilerInstance.getDiagnosticClient().BeginSourceFile(
      compilerInstance.getLangOpts(), &compilerInstance.getPreprocessor());

  // Create an AST consumer instance which is going to get called by
  // ParseAST.
  MyASTConsumer consumer(rewriter);

  // Parse the file to AST, registering our consumer as the AST consumer.
  clang::ParseAST(
    compilerInstance.getPreprocessor(), 
    &consumer, 
    compilerInstance.getASTContext());

  // At this point the rewriter's buffer should be full with the rewritten
  // file contents.
  const RewriteBuffer* buffer = rewriter.getRewriteBufferFor(sourceManager.getMainFileID());

  return std::string(buffer->begin(), buffer->end());
}

int main()
{
    std::cout << transform("test.c") << std::endl;
    // using clang::CompilerInstance;
    // using clang::TargetOptions;
    // using clang::TargetInfo;
    // using clang::FileEntry;
    // using clang::Token;
    // using clang::ASTContext;
    // using clang::ASTConsumer;
    // using clang::Parser;
    // using clang::DiagnosticOptions;
    // using clang::TextDiagnosticPrinter;

    // CompilerInstance ci;
    // DiagnosticOptions diagnosticOptions;
    // ci.createDiagnostics();

    // std::shared_ptr<clang::TargetOptions> pto = std::make_shared<clang::TargetOptions>();
    // pto->Triple = llvm::sys::getDefaultTargetTriple();
    // TargetInfo *pti = TargetInfo::CreateTargetInfo(ci.getDiagnostics(), pto);
    // ci.setTarget(pti);

    // ci.createFileManager();
    // ci.createSourceManager(ci.getFileManager());
    // ci.createPreprocessor(clang::TU_Complete);
    // PreprocessorOptions &PO = (ci.getPreprocessorOpts());
    // PO.UsePredefines = false;

    // ci.setASTConsumer(llvm::make_unique<ASTConsumer>());

    // ci.createASTContext();
    // ci.createSema(clang::TU_Complete, NULL);

    // const FileEntry *pFile = ci.getFileManager().getFile("test.c");
    // ci.getSourceManager().setMainFileID( ci.getSourceManager().createFileID( pFile, clang::SourceLocation(), clang::SrcMgr::C_User));
    // clang::ParseAST(ci.getSema());
    // ci.getASTContext().Idents.PrintStats();

    return 0;
}