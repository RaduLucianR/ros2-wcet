import clang.cindex
import json

def main():    
    index = clang.cindex.Index.create()
    path_cpp_file = '/home/radu/ros2-wcet/src/stress.cpp'
    compdb = clang.cindex.CompilationDatabase.fromDirectory("/home/radu/ros2-wcet/build")
    commands = compdb.getCompileCommands(path_cpp_file)

    file_args = []
    for command in commands:
        for argument in command.arguments:
            file_args.append(argument)

    translation_unit = index.parse(path_cpp_file, file_args)

    # # Collect variable names and line numbers of variable declarations
    # variables = []
    # var_decl_lines = []
    # for node in translation_unit.cursor.walk_preorder():
    #     # # Verify whether the declared variable is present in main method of source file
    #     # if (node.kind == clang.cindex.CursorKind.VAR_DECL and
    #     #     node.semantic_parent.kind == clang.cindex.CursorKind.FUNCTION_DECL and
    #     #         node.semantic_parent.spelling == 'main'):
    #     #     # Append variables and their declaration lines in arrays variables, var_decl_lines
    #     #     variables.append(node.spelling)
    #     #     var_decl_lines.append(node.extent.start.line)

    #     print(node.spelling) 

if __name__ == '__main__':
    main()
