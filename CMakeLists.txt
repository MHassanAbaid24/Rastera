cmake_minimum_required(VERSION 3.20)
project(Rastera LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(LLVM 18.1 REQUIRED CONFIG)
find_package(BISON 3.8 REQUIRED)
find_package(FLEX 2.6 REQUIRED)

# Front end: Bison and Flex generate C++ sources into the build directory.
bison_target(Parser src/parse/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
             DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp)
flex_target(Lexer src/parse/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
add_flex_bison_dependency(Lexer Parser)

# Runtime library linked into every compiled Rastera program.
add_library(rastera_rt STATIC runtime/rastera_rt.c)
target_compile_options(rastera_rt PRIVATE -Wall -Wextra)

add_executable(rasterc
    src/main.cpp
    src/sema/sema.cpp
    src/codegen/codegen.cpp
    src/backend/backend.cpp
    ${BISON_Parser_OUTPUTS}
    ${FLEX_Lexer_OUTPUTS}
)
target_include_directories(rasterc PRIVATE src ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(rasterc SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
target_compile_definitions(rasterc PRIVATE
    ${LLVM_DEFINITIONS_LIST}
    RASTERA_RUNTIME_LIB="$<TARGET_FILE:rastera_rt>")
target_compile_options(rasterc PRIVATE -Wall -Wextra)

llvm_map_components_to_libnames(LLVM_LIBS core support target targetparser mc nativecodegen)
target_link_libraries(rasterc PRIVATE ${LLVM_LIBS})
add_dependencies(rasterc rastera_rt)
