cmake_minimum_required(VERSION 3.16)

project(joltgrep VERSION 0.0.1 DESCRIPTION "grep C++ implementation") 

option(BUILD_TESTS "build and run tests" OFF) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE sources      src/*.cpp)

find_package(absl REQUIRED)
find_package(re2 REQUIRED)

add_executable(joltgrep ${sources})

target_compile_options(joltgrep PRIVATE -Wall -O2)

if (BUILD_TESTS)
    target_compile_definitions(joltgrep PRIVATE TESTS_BUILD)
    target_include_directories(joltgrep PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)

    file(GLOB_RECURSE tests_sources     tests/*.cpp)
    target_sources(joltgrep PRIVATE ${tests_sources})
endif()

target_link_libraries(joltgrep re2::re2)

target_include_directories(joltgrep PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/include)

set_target_properties(joltgrep PROPERTIES RUNTIME_OUTPUT_DIRECTORY "bin")
install(TARGETS joltgrep DESTINATION build)
set(CPACK_PACKAGE_NAME "joltgrep")
set(CPACK_PACKAGE_VERSION "0.0.1")
set(CPACK_MONOLITHIC_INSTALL 1)
    
include(CPack)

