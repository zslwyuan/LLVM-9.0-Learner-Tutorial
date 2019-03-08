#ifndef _HI_APIntSrcAnalysis
#define _HI_APIntSrcAnalysis
// related headers should be included.
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>
#include "HI_print.h"
#include "HI_SysExec.h"
#include "clang/AST/Type.h"
#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Parse/ParseAST.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>
#include <map>
#include <set>
#include <vector>
#include <sstream>
#include "llvm/Support/raw_ostream.h"

using namespace clang;




//                         declare a rewriter
//                               |  pass the pointer to
//                  call         V
// frontend action  --->   the creator
//         |                     |  create / pass the rewriter
//         |   Src Code          V
//         ------------->   AST consumer
//                               |
//                               |  generate AST
//                               V
//                            Visitor (visit the nodes in AST and do the rewritting)


class HI_APIntSrcAnalysis_Visitor : public RecursiveASTVisitor<HI_APIntSrcAnalysis_Visitor> 
{ 
public: 
    HI_APIntSrcAnalysis_Visitor(CompilerInstance &_CI, Rewriter &R) : CI(_CI), TheRewriter(R) 
    {
        parseLog = new llvm::raw_fd_ostream("parseLog", ErrInfo, llvm::sys::fs::F_None);
    } 

    ~HI_APIntSrcAnalysis_Visitor() 
    {
        parseLog->flush();
        delete parseLog;
    } 
    
    
    bool VisitStmt(Stmt *s) 
    { 
        // Only care about If statements. 
        if (isa<IfStmt>(s)) 
        { 
            IfStmt *IfStatement = cast<IfStmt>(s); 
            Stmt *Then = IfStatement->getThen(); 
            TheRewriter.InsertText(Then->getBeginLoc(), "// the 'if' part\n", true, true); 
            Stmt *Else = IfStatement->getElse(); 
            if (Else) TheRewriter.InsertText(Else->getBeginLoc(), "// the 'else' part\n", true, true); 
        } 
        parseLog->flush();
        return true; 
    } 

    virtual bool VisitVarDecl(VarDecl *VD) 
    { 
        *parseLog << "find VarDecl: VarName: ["  << VD->getNameAsString() << "] DeclKind:[" 
            << VD->getDeclKindName()<< "] Type: ["<< VD->getType().getAsString() << "] at Loc: [" 
            << VD->getBeginLoc().printToString(CI.getSourceManager()) <<"]\n";
        *parseLog << "    ---  detailed information of the type\n";
        printTypeInfo(VD->getType().getTypePtr());
        if (isAPInt(VD))
        {
            TheRewriter.InsertText(VD->getBeginLoc(), "// "+VD->getNameAsString() +" is ap int type ("+getAPIntName(VD)+").\n");
        }
        parseLog->flush();
        return true;
    }

    bool VisitFunctionDecl(FunctionDecl *f) 
    { 
        // Only function definitions (with bodies), not declarations. 
        if (f->hasBody()) 
        { 
            Stmt *FuncBody = f->getBody(); // Type name as string 
            QualType QT = f->getReturnType(); 
            std::string TypeStr = QT.getAsString(); // Function name 
            DeclarationName DeclName = f->getNameInfo().getName(); 
            std::string FuncName = DeclName.getAsString(); // Add comment before 
            std::stringstream SSBefore; SSBefore << "// Begin function " << FuncName << " returning " << TypeStr << "\n"; 
            SourceLocation ST = f->getSourceRange().getBegin(); 
            TheRewriter.InsertText(ST, SSBefore.str(), true, true); // And after 
            std::stringstream SSAfter; 
            SSAfter << "\n// End function " << FuncName; 
            ST = FuncBody->getEndLoc().getLocWithOffset(1); 
            TheRewriter.InsertText(ST, SSAfter.str(), true, true); 
        } 
        return true; 
    } 

    void printTypeInfo(const Type *T);


    bool isAPInt(VarDecl *VD);

    std::string getAPIntName(VarDecl *VD);

    PrintingPolicy PP()
    {
        return PrintingPolicy(CI.getLangOpts());
    }

private: 
    Rewriter &TheRewriter; 
    CompilerInstance &CI;
    std::error_code ErrInfo;
    raw_ostream *parseLog;

}; // Implementation of the ASTConsumer interface for reading an AST produced // by the Clang parser. 

class HI_APIntSrcAnalysis_ASTConsumer : public ASTConsumer 
{ 
  public: 
    HI_APIntSrcAnalysis_ASTConsumer(CompilerInstance &_CI,Rewriter &R) : Visitor(_CI,R),CI(_CI)
    {

    } // Override the method that gets called for each parsed top-level // declaration. 
    bool HandleTopLevelDecl(DeclGroupRef DR) override 
    { 
        for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) 
        { 
            // Traverse the declaration using our AST visitor. 
            Visitor.TraverseDecl(*b); //(*b)->dump(); 
        } 
        return true; 
    } 

private: 
    HI_APIntSrcAnalysis_Visitor Visitor; 
    CompilerInstance &CI;
}; 

// For each source file provided to the tool, a new FrontendAction is created. 

class HI_APIntSrcAnalysis_FrontendAction : public ASTFrontendAction 
{ 
    public: HI_APIntSrcAnalysis_FrontendAction() {} 
    void EndSourceFileAction() override 
    { 
        SourceManager &SM = TheRewriter.getSourceMgr(); 
        llvm::errs() << "** EndSourceFileAction for: " << SM.getFileEntryForID(SM.getMainFileID())->getName() << "\n";  // Now emit the rewritten buffer. 
        TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs()); 
    } 
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override 
    { 
        llvm::errs() << "** Creating AST consumer for: " << file << "\n"; 
        TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts()); 
        return llvm::make_unique<HI_APIntSrcAnalysis_ASTConsumer>(CI,TheRewriter); 
    } 
    
private: 
    Rewriter TheRewriter; 
};




// According the official template of Clang, this is a creater with newASTConsumer(), which
// will generator a AST consumer. We can first create a rewriter and pass the pointer of the
// rewriter to the creator. Finally,  we can pass the rewriter pointer to the inner visitor.
// rewriter ptr -> creator -> frontend-action -> ASTconsumer -> Visitor

// template <typename T>
// class HI_Rewrite_FrontendActionFactory : public tooling::FrontendActionFactory 
// {
// private:
//     std::string outputName;

// public:
//     HI_Rewrite_FrontendActionFactory(const char *rewriteout_name) 
//     {
//         outputName = rewriteout_name;
//     }
//     FrontendAction *create() override { return new T(outputName.c_str()); }
// };


// class HI_APIntSrcAnalysis_FrontendAction : public ASTFrontendAction 
// { 
// public: 
//     HI_APIntSrcAnalysis_FrontendAction(const char *rewriteout_name) 
//     {
//         rewriteout = new llvm::raw_fd_ostream(rewriteout_name, ErrInfo, llvm::sys::fs::F_None);
//     } 
    
//     void EndSourceFileAction() override 
//     { 
//         SourceManager &SM = TheRewriter.getSourceMgr(); 
//         llvm::errs() << "** EndSourceFileAction for: " << SM.getFileEntryForID(SM.getMainFileID())->getName() << "\n"; // Now emit the rewritten buffer. 
//         TheRewriter.getEditBuffer(SM.getMainFileID()).write(*rewriteout); 
//     } 

//     void ExecuteAction() override
//     {
//       CompilerInstance &CI = getCompilerInstance();
//       if (!CI.hasPreprocessor())
//         return;

//       // FIXME: Move the truncation aspect of this into Sema, we delayed this till
//       // here so the source manager would be initialized.
//       if (hasCodeCompletionSupport() &&
//           !CI.getFrontendOpts().CodeCompletionAt.FileName.empty())
//         CI.createCodeCompletionConsumer();

//       // Use a code completion consumer?
//       CodeCompleteConsumer *CompletionConsumer = nullptr;
//       if (CI.hasCodeCompletionConsumer())
//         CompletionConsumer = &CI.getCodeCompletionConsumer();

//       if (!CI.hasSema())
//         CI.createSema(getTranslationUnitKind(), CompletionConsumer);

//       llvm::errs() << "** Begin to parse AST\n"; 
//       // Sema &S = CI.getSema();
//       // ASTConsumer *Consumer = (&S.getASTConsumer());
//       // if (!Consumer )
//       //   llvm::errs() << "** No Consumer Found\n"; 
//       CI.getASTConsumer();
//       clang::ParseAST(CI.getSema(), CI.getFrontendOpts().ShowStats,
//               CI.getFrontendOpts().SkipFunctionBodies);
//       llvm::errs() << "** End parsing AST\n"; 
//     }


//     std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override 
//     { 
//         llvm::errs() << "** Creating AST consumer for: " << file << "\n"; 
//         TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts()); 
//         return llvm::make_unique<HI_APIntSrcAnalysis_ASTConsumer>(&CI,&TheRewriter); 
//     } 

// private:
//     std::error_code ErrInfo;
//     raw_ostream *rewriteout; 
//     Rewriter TheRewriter;
// };

#endif
