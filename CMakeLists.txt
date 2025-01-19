cmake_minimum_required(VERSION 3.16)

project(joltgrep VERSION 0.0.1 DESCRIPTION "grep C++ implementation") 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

file(GLOB_RECURSE sources      src/*.cpp)

find_package(absl REQUIRED)
find_package(re2 REQUIRED)

add_executable(joltgrep ${sources})

target_compile_options(joltgrep PUBLIC -Wall)

target_link_libraries(joltgrep re2::re2)

set_target_properties(joltgrep PROPERTIES RUNTIME_OUTPUT_DIRECTORY "bin")
install(TARGETS joltgrep DESTINATION build)
set(CPACK_PACKAGE_NAME "joltgrep")
set(CPACK_PACKAGE_VERSION "0.0.1")
set(CPACK_MONOLITHIC_INSTALL 1)
    
include(CPack)

