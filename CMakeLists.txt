cmake_minimum_required(VERSION 3.14)
project(ebee)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_program(LLVM_CONFIG_EXECUTABLE llvm-config-18 REQUIRED)
if(NOT LLVM_CONFIG_EXECUTABLE)
    message(FATAL_ERROR "llvm-config not found")
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
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs core orcjit native support
    OUTPUT_VARIABLE LLVM_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

separate_arguments(LLVM_CXXFLAGS)
separate_arguments(LLVM_LDFLAGS)
separate_arguments(LLVM_LIBS)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cc)
if(NOT SOURCE_FILES)
    message(FATAL_ERROR "No source files found in src directory.")
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/include)
    include_directories(${CMAKE_SOURCE_DIR}/include)
else()
    message(WARNING "Include directory does not exist: ${CMAKE_SOURCE_DIR}/include")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBBPF REQUIRED libbpf)

find_package(ZLIB REQUIRED)

add_library(ebeelib ${SOURCE_FILES})
target_compile_options(ebeelib PRIVATE -fvisibility=hidden)
target_include_directories(ebeelib PRIVATE ${LIBBPF_INCLUDE_DIRS})
target_compile_options(ebeelib PRIVATE ${LLVM_CXXFLAGS} -fexceptions -fvisibility=hidden)

# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--strip-all")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ")

target_link_libraries(ebeelib PRIVATE ${LLVM_LIBS} ${LIBBPF_LIBRARIES} ZLIB::ZLIB)
target_link_options(ebeelib PRIVATE ${LLVM_LDFLAGS})

add_executable(ebee ${SOURCE_FILES})
target_link_libraries(ebee PRIVATE ebeelib)

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
target_link_libraries(tests PRIVATE gtest_main ebeelib ${LLVM_LIBS} ZLIB::ZLIB)
gtest_discover_tests(tests)

file(GLOB_RECURSE EBPF_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/ebpf/*.c)

set(EBPF_OUTPUT_DIR ${CMAKE_BINARY_DIR}/ebpf)
file(MAKE_DIRECTORY ${EBPF_OUTPUT_DIR})

foreach(ebpf_src ${EBPF_SOURCES})
    get_filename_component(src_name ${ebpf_src} NAME_WE)
    set(output_obj ${EBPF_OUTPUT_DIR}/${src_name}.o)

    add_custom_command(
        OUTPUT ${output_obj}
        COMMAND clang
            -target bpf
            -O2
            -g
            -D__TARGET_ARCH_x86
            -I ${CMAKE_SOURCE_DIR}/include
            -c ${ebpf_src}
            -o ${output_obj}
        DEPENDS ${ebpf_src}
        COMMENT "Compiling eBPF program: ${src_name}.c -> ${src_name}.o"
        VERBATIM
    )

    list(APPEND EBPF_OBJECTS ${output_obj})
endforeach()

add_custom_target(ebpf_objs ALL DEPENDS ${EBPF_OBJECTS})