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
    TimerDefMatcher() : Initialized(false) {

    }

    void run(const MatchFinder::MatchResult &Result) override {
        if (!Initialized) {
            Initialized = true;
        }

        const CXXMemberCallExpr *callExpr = Result.Nodes.getNodeAs<CXXMemberCallExpr>("createWallTimerCall");
        if (!callExpr) return;

        SourceManager &srcMgr = Result.Context->getSourceManager();

        const Expr *period = callExpr->getArg(0);
        const Expr *callback = callExpr->getArg(1);

        std::string periodString = Lexer::getSourceText(
            CharSourceRange::getTokenRange(period->getSourceRange()),
            srcMgr, Result.Context->getLangOpts()
        ).str();

        std::string callbackString = Lexer::getSourceText(
            CharSourceRange::getTokenRange(callback->getSourceRange()),
            srcMgr, Result.Context->getLangOpts()
        ).str();

        std::cout << "Found timer callback with period " << periodString << " and name: " << callbackString << std::endl;
    }

private:
    bool Initialized;
};

class SubscribtionDefMatcher : public MatchFinder::MatchCallback {
public:
    SubscribtionDefMatcher() : Initialized(false) {

    }

    void run(const MatchFinder::MatchResult &Result) override {
        if (!Initialized) {
            Initialized = true;
        }

        const CXXMemberCallExpr *callExpr = Result.Nodes.getNodeAs<CXXMemberCallExpr>("createSubscriptionCall");
        if (!callExpr) return;

        SourceManager &srcMgr = Result.Context->getSourceManager();
        const SourceLocation loc = callExpr->getBeginLoc();
        std::string fileName = srcMgr.getFilename(loc).str();

        if (fileName == "") {

        }

        const Expr *topic = callExpr->getArg(0);
        const Expr *qos = callExpr->getArg(1);
        const Expr *callback = callExpr->getArg(2);

        std::string topicString = Lexer::getSourceText(
            CharSourceRange::getTokenRange(topic->getSourceRange()),
            srcMgr, Result.Context->getLangOpts()
        ).str();

        std::string callbackString = Lexer::getSourceText(
            CharSourceRange::getTokenRange(callback->getSourceRange()),
            srcMgr, Result.Context->getLangOpts()
        ).str();

        std::cout << "Found subscription callback on topic " << topicString << " and name: " << callbackString << std::endl;
    }

private:
    bool Initialized;
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

    StatementMatcher subscriptionMatcher = cxxMemberCallExpr(
        callee(cxxMethodDecl(hasName("create_subscription"))) // Match calls to create_subscriptions
    ).bind("createSubscriptionCall");

    TimerDefMatcher timerDefMatcher;
    SubscribtionDefMatcher subDefMatcher;
    MatchFinder Finder;

    Finder.addMatcher(timerMatcher, &timerDefMatcher);
    Finder.addMatcher(subscriptionMatcher, &subDefMatcher);

    return Tool.run(newFrontendActionFactory(&Finder).get());
}