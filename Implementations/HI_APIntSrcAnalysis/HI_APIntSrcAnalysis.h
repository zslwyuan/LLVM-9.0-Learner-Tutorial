#ifndef _HI_APIntSrcAnalysis
#define _HI_APIntSrcAnalysis
// related headers should be included.
#include "HI_SysExec.h"
#include "HI_print.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Type.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <ios>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>
#include <utility>
#include <vector>

using namespace clang;

// According the official template of Clang, this is a frontend factory with function
// createASTConsumer(), which will generator a AST consumer. We can first create a rewriter and pass
// the reference of the rewriter to the factory. Finally,  we can pass the rewriter reference to the
// inner visitor. rewriter  -> factory -> frontend-action -> ASTconsumer -> Visitor

//                         declare a rewriter
//                               |  pass the reference to
//                  create       V
// frontend Factory ----->   FrontEnd Action
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
    HI_APIntSrcAnalysis_Visitor(CompilerInstance &_CI, Rewriter &R, std::string _parselog_name)
        : CI(_CI), TheRewriter(R), parselog_name(_parselog_name)
    {
        parseLog = new llvm::raw_fd_ostream(_parselog_name.c_str(), ErrInfo, llvm::sys::fs::F_None);
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
            if (Else)
                TheRewriter.InsertText(Else->getBeginLoc(), "// the 'else' part\n", true, true);
        }
        parseLog->flush();
        return true;
    }

    // Visit the declaration of Variables and detect templates among them
    virtual bool VisitVarDecl(VarDecl *VD)
    {
        *parseLog << "find VarDecl: VarName: [" << VD->getNameAsString() << "] DeclKind:["
                  << VD->getDeclKindName() << "] Type: [" << VD->getType().getAsString()
                  << "] at Loc: [" << VD->getBeginLoc().printToString(CI.getSourceManager())
                  << "]\n";
        *parseLog << "    ---  detailed information of the type\n";
        printTypeInfo(VD->getType().getTypePtr());
        if (isAPInt(VD))
        {
            TheRewriter.InsertText(VD->getBeginLoc(), "// " + VD->getNameAsString() +
                                                          " is ap int type (" + getAPIntName(VD) +
                                                          ").\n");
        }
        parseLog->flush();
        return true;
    }

    // Toy: access functions in AST
    bool VisitFunctionDecl(FunctionDecl *f)
    {
        // Only function definitions (with bodies), not declarations.
        if (f->hasBody())
        {
            Stmt *FuncBody = f->getBody(); // Type name as string
            QualType QT = f->getReturnType();
            std::string TypeStr = QT.getAsString(); // Function name
            DeclarationName DeclName = f->getNameInfo().getName();
            std::string FuncName = DeclName.getAsString();

            // Add comment before
            std::stringstream SSBefore;
            SSBefore << "// Begin function " << FuncName << " returning " << TypeStr << "\n";

            if (f->isReferenced())
                SSBefore << "// is referenced\n";
            else
                SSBefore << "// is not referenced\n";

            SourceLocation ST = f->getSourceRange().getBegin();
            TheRewriter.InsertText(ST, SSBefore.str(), true, true);

            // And after
            std::stringstream SSAfter;
            SSAfter << "\n// End function " << FuncName;
            ST = FuncBody->getEndLoc().getLocWithOffset(1);
            TheRewriter.InsertText(ST, SSAfter.str(), true, true);
        }
        return true;
    }

    // print the detailed information of the type
    void printTypeInfo(const Type *T);

    // check whether it is a template structure like XXXX<X>
    bool isAPInt(VarDecl *VD);

    // get tht template name
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
    std::string parselog_name;
};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class HI_APIntSrcAnalysis_ASTConsumer : public ASTConsumer
{
  public:
    HI_APIntSrcAnalysis_ASTConsumer(CompilerInstance &_CI, Rewriter &R, std::string _parselog_name)
        : Visitor(_CI, R, _parselog_name), CI(_CI), parselog_name(_parselog_name)
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
    std::string parselog_name;

    /// Timer

    struct timeval tv_begin;
    struct timeval tv_end;
};

// For each source file provided to the tool, a new FrontendAction is created.
class HI_APIntSrcAnalysis_FrontendAction : public ASTFrontendAction
{
  public:
    HI_APIntSrcAnalysis_FrontendAction(const char *_parselog_name, Rewriter &R,
                                       const char *_outputCode_name)
        : parselog_name(_parselog_name), TheRewriter(R), outputCode_name(_outputCode_name)
    {
    }
    void EndSourceFileAction() override
    {
        SourceManager &SM = TheRewriter.getSourceMgr();
        llvm::errs() << "** EndSourceFileAction for: "
                     << SM.getFileEntryForID(SM.getMainFileID())->getName()
                     << "\n"; // Now emit the rewritten buffer.
        outputCode =
            new llvm::raw_fd_ostream(outputCode_name.c_str(), ErrInfo, llvm::sys::fs::F_None);
        TheRewriter.getEditBuffer(SM.getMainFileID()).write(*outputCode);
        outputCode->flush();
        delete outputCode;
    }
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
    {
        llvm::errs() << "** Creating AST consumer for: " << file << "\n";
        TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        return llvm::make_unique<HI_APIntSrcAnalysis_ASTConsumer>(CI, TheRewriter, parselog_name);
    }

  private:
    Rewriter &TheRewriter;
    std::string parselog_name;
    std::string outputCode_name;
    raw_ostream *outputCode;
    std::error_code ErrInfo;
};

// We need a factory to produce such a frontend action
template <typename T>
std::unique_ptr<tooling::FrontendActionFactory>
HI_rewrite_newFrontendActionFactory(const char *_parseLog_name, Rewriter &R,
                                    const char *_outputCode_name)
{
    class SimpleFrontendActionFactory : public tooling::FrontendActionFactory
    {
      public:
        SimpleFrontendActionFactory(const char *_parseLog_name, Rewriter &R,
                                    const char *_outputCode_name)
            : parseLog_name(_parseLog_name), TheRewriter(R), outputCode_name(_outputCode_name)
        {
        }
        FrontendAction *create() override
        {
            return new T(parseLog_name.c_str(), TheRewriter, outputCode_name.c_str());
        }
        std::string parseLog_name;
        std::string outputCode_name;
        Rewriter &TheRewriter;
    };

    return std::unique_ptr<tooling::FrontendActionFactory>(
        new SimpleFrontendActionFactory(_parseLog_name, R, _outputCode_name));
}

#endif
