#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/SourceManager.h"

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;
using namespace clang::ast_matchers;

static cl::OptionCategory ROSParser("rosparser options");
nlohmann::json jsonArray;
std::string fileName = "";

class NodeClassMatcher : public MatchFinder::MatchCallback {
public:
    virtual void run(const MatchFinder::MatchResult &Result) {
        if (const CXXRecordDecl *classDecl = Result.Nodes.getNodeAs<CXXRecordDecl>("derivedClass")) {
            std::cout << "Class: " << classDecl->getNameAsString() << std::endl;

            /**
             * ######## Get name of the file in which the Node Class resides ########
             * TODO: Do this in a different way. It must be an easier way.
             * Right now we check the location of each Node class 
             * and we re-initialize the jsonArray if the current class 
             * is in a new file.
             * 
             * Instead, we should already know *somehow* that we are 
             * in a new file by this point, so the jsonArray should already
             * correspond to the new file.
             * 
             * This portion of code is necessary because each file that
             * contains Node Classes must have a JSON of its own.
             */
            SourceLocation loc = classDecl->getLocation();
            const SourceManager &sourceManager = Result.Context->getSourceManager();     

            if (fileName.compare(sourceManager.getFilename(loc).str()) != 0) {
                fileName = sourceManager.getFilename(loc).str();
                jsonArray = nlohmann::json::array();
            }
            // #######################################################################

            nlohmann::json nodeClass;
            nodeClass["name"] = classDecl->getNameAsString();
            nlohmann::json callbacksArray = nlohmann::json::array();

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
                        nlohmann::json callback;
                        callback["name"] = methodDecl->getNameAsString();
                        callback["param_count"] = 0;
                        nlohmann::json parametersArray = nlohmann::json::array();

                        for (const auto *param : methodDecl->parameters()) {
                            nlohmann::json parameter;
                            parameter["name"] = param->getNameAsString();
                            parameter["type"] = param->getType().getAsString();
                            parametersArray.push_back(parameter);

                            std::cout << "    " << param->getType().getAsString() << " "; // Get argument type
                            std::cout << param->getNameAsString() << endl;
                        }

                        callback["parameters"] = parametersArray;
                        callbacksArray.push_back(callback);
                    }
                }
            }

            nodeClass["callbacks"] = callbacksArray;
            jsonArray.push_back(nodeClass);

            // Write to file
            std::string jsonFile = fileName + ".json";
            ofstream Json(jsonFile); //TODO: close file!!!
            Json << jsonArray.dump(4);
        }
    }
};

int main(int argc, const char **argv) {
    // * TODO: Add option for destination directory of JSONs 
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