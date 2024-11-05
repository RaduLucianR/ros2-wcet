#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Lex/Lexer.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;
using namespace clang::ast_matchers;

static cl::OptionCategory ROSParser("rosparser options");

class TimerDefMatcher : public MatchFinder::MatchCallback {
public:
    TimerDefMatcher(Rewriter &Rewrite) : Rewrite(Rewrite), Initialized(false) {

    }

    void run(const MatchFinder::MatchResult &Result) override {
        if (!Initialized) {
            // Initialize the Rewriter with SourceManager and LangOpts from MatchResult
            Rewrite.setSourceMgr(Result.Context->getSourceManager(), Result.Context->getLangOpts());
            Initialized = true;
        }

        const CXXMemberCallExpr *callExpr = Result.Nodes.getNodeAs<CXXMemberCallExpr>("createWallTimerCall");
        if (!callExpr) return;

        auto &SourceMgr = Rewrite.getSourceMgr();
        const Expr *arg1 = callExpr->getArg(0);
        const Expr *arg2 = callExpr->getArg(1);

        // Get the source code text for arg1 and arg2
        std::string arg1Text = Lexer::getSourceText(
            CharSourceRange::getTokenRange(arg1->getSourceRange()),
            SourceMgr, Result.Context->getLangOpts()
        ).str();
        std::string arg2Text = Lexer::getSourceText(
            CharSourceRange::getTokenRange(arg2->getSourceRange()),
            SourceMgr, Result.Context->getLangOpts()
        ).str();

        // Define the new argument string using arg1 and arg2 text
        // Add nullptr for callback group i.e. don't add it to a cb group
        // Add false for autostart option i.e. block it from autostarting

        // TODO: There must be a way to enable the timer at runtime 
        // but I can't find it in the source code or documentation.
        // Find a better way than to do a find-and-replace of the
        // timer declaration with the correct flag disabled :)
        std::string newArgs = arg1Text + ", " + arg2Text + ", nullptr, false";
        clang::SourceLocation callStartLocation = callExpr->getArg(0)->getBeginLoc();

        // Replace the entire argument list
        CharSourceRange argRange = CharSourceRange::getTokenRange(
            callStartLocation,
            callExpr->getEndLoc().getLocWithOffset(-1)
        );

        Rewrite.ReplaceText(argRange, newArgs);

        // ############# Write to file ################## 
        auto MainFileID = SourceMgr.getMainFileID();
        SourceLocation StartLoc = SourceMgr.getLocForStartOfFile(MainFileID);
        
        // Get the filename from the SourceLocation
        llvm::StringRef Filename = SourceMgr.getFilename(StartLoc);

        // Save a copy of the file, so we can revert changes later
        copyFile(Filename.str(), getDestinationPath(Filename.str()));

        // Create an output file stream
        std::error_code EC;
        llvm::raw_fd_ostream out(Filename.str(), EC, llvm::sys::fs::FA_Write);
        
        if (EC) {
            llvm::errs() << "Could not open file for writing: " << EC.message() << "\n";
            return;
        }

        // Write the modified content back to the file
        Rewrite.getEditBuffer(MainFileID).write(out);
        out.close();

        std::cout << "Modified timer located at: " << callStartLocation.printToString(SourceMgr) << std::endl;
    }

private:
    Rewriter &Rewrite;
    bool Initialized;

    // TODO: Maybe move these 2 functions to a separate file
    bool copyFile(const std::string& source, const std::string& destination) {
        std::ifstream src(source, std::ios::binary);
        if (!src) {
            std::cerr << "Error opening source file: " << source << std::endl;
            return false;
        }

        std::ofstream dest(destination, std::ios::binary);
        if (!dest) {
            std::cerr << "Error opening destination file: " << destination << std::endl;
            return false;
        }

        dest << src.rdbuf(); // Copy content from source to destination
        return true;
    }

    std::string getDestinationPath(const std::string& source) {
        // Get the filename from the source path
        std::filesystem::path sourcePath(source);
        std::string filename = sourcePath.filename().string();

        // Construct the destination path
        std::string homeDir = getenv("HOME"); // Get the home directory
        std::string destinationDir = homeDir + "/.ros2wcet/save_modified_files/";
        
        // Create the destination directory if it doesn't exist
        std::filesystem::create_directories(destinationDir);

        return destinationDir + filename; // Combine the directory and filename
    }
};

int main(int argc, const char **argv) {
    auto CliParser = CommonOptionsParser::create(argc, argv, ROSParser);

    if (!CliParser) {
        llvm::errs() << CliParser.takeError();
        return 1;
    }

    CommonOptionsParser& op = CliParser.get();      
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());

    StatementMatcher timerMatcher = cxxMemberCallExpr(
        callee(cxxMethodDecl(hasName("create_wall_timer"))) // Match calls to create_wall_timer
    ).bind("createWallTimerCall");

    Rewriter Rewrite;
    TimerDefMatcher timerDefMatcher(Rewrite);
    MatchFinder Finder;

    Finder.addMatcher(timerMatcher, &timerDefMatcher);

    return Tool.run(newFrontendActionFactory(&Finder).get());
}