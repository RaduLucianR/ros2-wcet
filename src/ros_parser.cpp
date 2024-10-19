#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <iostream>

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;
using namespace clang::ast_matchers;

int numFunctions = 0;
int numVar = 0;
int numClass = 0;
int numConstr = 0;
static cl::OptionCategory ROSParser("my-tool options");


class NodeClassMatcher : public MatchFinder::MatchCallback {
public:
    virtual void run(const MatchFinder::MatchResult &Result) {
        if (const CXXRecordDecl *classDecl = Result.Nodes.getNodeAs<CXXRecordDecl>("derivedClass")) {
            std::cout << "Class: " << classDecl->getNameAsString() << std::endl;

            // Iterate over the declarations in the class
            for (const auto *decl : classDecl->decls()) {
                // Check if the declaration is a member function
                if (const auto *methodDecl = llvm::dyn_cast<CXXMethodDecl>(decl)) {
                    if (
                        methodDecl->getAccess() == AS_public &&                // Check if member function is public
                        !llvm::isa<CXXConstructorDecl>(methodDecl) &&          // Check if member function is NOT constructor
                        !llvm::isa<CXXDestructorDecl>(methodDecl) &&           // Check if member function is NOT destructor
                        methodDecl->getNameAsString().rfind("operator", 0) > 0 // Check if member function is NOT operator
                    ) {       
                        std::cout << "  Public Member Function: " << methodDecl->getNameAsString() << endl;

                        for (const auto *param : methodDecl->parameters()) {
                            std::cout << "    " << param->getType().getAsString() << " "; // Get argument type

                            if (!param->getNameAsString().empty()) {
                                std::cout << param->getNameAsString() << endl; // Get argument name
                            }
                        }
                    }
                }
            }
        }
    }
};

int main(int argc, const char **argv) {
    // parse the command-line args passed to your code
    auto CliParser = CommonOptionsParser::create(argc, argv, ROSParser);
    if (!CliParser) {
        llvm::errs() << CliParser.takeError();
        return 1;
    }

    CommonOptionsParser& op = CliParser.get();      
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());

    // Match all classes that...
    DeclarationMatcher classMatcher = cxxRecordDecl(
        isDerivedFrom("rclcpp::Node"), // ...extend the "Super" class
        isExpansionInMainFile()        // ...are in the given file
    ).bind("derivedClass");

    NodeClassMatcher nodeClassMatcher;
    MatchFinder Finder;

    Finder.addMatcher(classMatcher, &nodeClassMatcher);

    return Tool.run(newFrontendActionFactory(&Finder).get());
}