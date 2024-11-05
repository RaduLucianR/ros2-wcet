#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Frontend/CompilerInstance.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Error.h>
#include <memory>
#include <string>
#include <system_error>

using namespace clang;
using namespace clang::tooling;

// Define a new command-line option category for our tool
static llvm::cl::OptionCategory VarRenamerCategory("var-renamer options");

class VarRenamerVisitor : public RecursiveASTVisitor<VarRenamerVisitor> {
public:
    explicit VarRenamerVisitor(Rewriter &R) : TheRewriter(R) {}

    bool VisitDeclRefExpr(DeclRefExpr *E) {
        llvm::outs() << "Found variable: " << E->getNameInfo().getAsString() << "\n";
        
        if (E->getNameInfo().getAsString() == "a") {
            SourceLocation loc = E->getLocation();
            // Replace 'a' with 'b'
            TheRewriter.ReplaceText(loc, 1, "v");
            llvm::outs() << "Replaced 'a' with 'b' at location: " << loc.printToString(TheRewriter.getSourceMgr()) << "\n";
        }
        return true;
    }

private:
    Rewriter &TheRewriter;
};

class VarRenamerASTConsumer : public ASTConsumer {
public:
    explicit VarRenamerASTConsumer(Rewriter &R) : Visitor(R) {}

    bool HandleTopLevelDecl(DeclGroupRef DR) override {
        for (auto *D : DR) {
            Visitor.TraverseDecl(D);
        }
        return true;
    }

private:
    VarRenamerVisitor Visitor;
};

class VarRenamerFrontendAction : public ASTFrontendAction {
public:
    VarRenamerFrontendAction() = default;

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
        TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        return std::make_unique<VarRenamerASTConsumer>(TheRewriter);
    }

    void EndSourceFileAction() override {
        // Get the location of the main file
        auto &SourceMgr = TheRewriter.getSourceMgr();
        auto MainFileID = SourceMgr.getMainFileID();
        SourceLocation StartLoc = SourceMgr.getLocForStartOfFile(MainFileID);
        
        // Get the filename from the SourceLocation
        llvm::StringRef Filename = SourceMgr.getFilename(StartLoc);

        // Create an output file stream
        std::error_code EC;
        llvm::raw_fd_ostream out(Filename.str(), EC, llvm::sys::fs::FA_Write);
        
        if (EC) {
            llvm::errs() << "Could not open file for writing: " << EC.message() << "\n";
            return;
        }

        // Write the modified content back to the file
        TheRewriter.getEditBuffer(MainFileID).write(out);
        out.close();
    }

private:
    Rewriter TheRewriter;
};

int main(int argc, const char **argv) {
    auto CliParser = CommonOptionsParser::create(argc, argv, VarRenamerCategory);

    if (!CliParser) {
        llvm::errs() << "Error creating CommonOptionsParser: " << toString(CliParser.takeError()) << "\n";
        return 1;
    }

    CommonOptionsParser &OptionsParser = *CliParser;

    // Initialize the ClangTool
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

    // Run the tool
    return Tool.run(newFrontendActionFactory<VarRenamerFrontendAction>().get());
}
