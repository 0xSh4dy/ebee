cmake_minimum_required(VERSION 3.14)
project(ebee)

find_program(LLVM_CONFIG_EXECUTABLE llvm-config-18 REQUIRED)

if(NOT LLVM_CONFIG_EXECUTABLE)
    message(FATAL_ERROR "llvm-config not found")
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/include)
    include_directories(${CMAKE_SOURCE_DIR}/include)
else()
    message(WARNING "Include directory does not exist: ${CMAKE_SOURCE_DIR}/include")
endif()

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cc)

if(NOT SOURCE_FILES)
    message(FATAL_ERROR "No source files found in src directory.")
endif()

execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --cxxflags
    OUTPUT_VARIABLE LLVM_CXXFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
    OUTPUT_VARIABLE LLVM_LDFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs core orcjit native
    OUTPUT_VARIABLE LLVM_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_library(ebeelib ${SOURCE_FILES})
add_executable(ebee ${SOURCE_FILES})
separate_arguments(LLVM_CXXFLAGS)
separate_arguments(LLVM_LDFLAGS)
separate_arguments(LLVM_LIBS)

target_compile_options(${PROJECT_NAME} PRIVATE ${LLVM_CXXFLAGS})
target_compile_options(${PROJECT_NAME} PRIVATE -fexceptions)
target_compile_options(ebeelib PRIVATE -fexceptions)
target_link_options(${PROJECT_NAME} PRIVATE ${LLVM_LDFLAGS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LLVM_LIBS})


include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)
file(GLOB TEST_SOURCES "tests/*.cc")

add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests
    gtest_main ebeelib ${LLVM_LIBS}
)
gtest_discover_tests(tests)