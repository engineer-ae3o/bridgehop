cmake_minimum_required(VERSION 4.0)

# Add default toolchain file if one not added yet
if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/host_gcc.cmake)
endif()

find_program(CCACHE ccache)
if (CCACHE)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
endif()

project(BridgeHopSim)
enable_language(C CXX)

set(LIB_ELF_NAME bridgehop-sim)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

add_executable(${LIB_ELF_NAME} host/main.cpp)
target_include_directories(${LIB_ELF_NAME} PRIVATE bridgehop/)

target_compile_options(${LIB_ELF_NAME} PRIVATE
   "-Wall -Wstrict-aliasing=2 -Wduplicated-branches -Wduplicated-cond -Werror \
    -Wlogical-op -Wextra -Wshadow -Wno-conversion -Wwrite-strings -Wformat=2 \
    -fstrict-overflow -fstrict-aliasing -Wdouble-promotion -fstack-usage \
    -Wnull-dereference -Wcast-align -Wunused-variable -Wunused-parameter \
    -fno-exceptions -fno-rtti"
)

set_target_properties(${LIB_ELF_NAME} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS ON
)

add_subdirectory(bridgehop/)
target_link_libraries(${LIB_ELF_NAME} bridgehop)

# Targets for static analysis and formatting
find_program(CPPCHECK cppcheck)
find_program(CLANG_FORMAT clang-format)

if (CPPCHECK)
add_custom_target(cppcheck
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/.cppcheck
    COMMAND ${CPPCHECK} --enable=all
        --inline-suppr
        --cppcheck-build-dir=${CMAKE_BINARY_DIR}/.cppcheck
        --suppressions-list=${CMAKE_SOURCE_DIR}/cppcheck_suppressions.txt
        --error-exitcode=1
        --check-level=exhaustive
        -D__GNUC__
        --project=${CMAKE_BINARY_DIR}/compile_commands.json
        --file-filter=${CMAKE_SOURCE_DIR}/bridgehop/active/**
        --file-filter=${CMAKE_SOURCE_DIR}/bridgehop/passive/**
        --file-filter=${CMAKE_SOURCE_DIR}/bridgehop/utils/**
        --output-file=${CMAKE_BINARY_DIR}/cppcheck_report.txt
        --checkers-report=${CMAKE_BINARY_DIR}/cppcheck_checks.txt
    COMMENT "Running static analysis..."
)
else()
message(WARNING "Cppcheck not found. Static analysis will not be available for this target.")
endif()

if(CLANG_FORMAT)
add_custom_target(format
    COMMAND git ls-files -- '*.c' '*.h' '*.cpp' '*.hpp' '*.cc' '*.cxx' | xargs -r ${CLANG_FORMAT} -i
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Formatting version controlled files...."
)
else()
message(WARNING "clang-format not found. Formatting will not be availabe for this target.")
endif()
