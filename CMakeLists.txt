cmake_minimum_required(VERSION 4.0)

project(BridgeHopSimulator)
enable_language(CXX)

if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/host_gcc.cmake)
endif()

find_program(CCACHE ccache)
if (CCACHE)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Build flags for the different build modes
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -ggdb -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-Os -flto -DNDEBUG")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")

# Build flags
set(CMAKE_C_FLAGS "-Wall -Wduplicated-branches -Wstrict-aliasing=2 -Wduplicated-cond -Werror \
                   -Wlogical-op -Wextra -Wshadow -Wno-conversion -Wwrite-strings -Wformat=2 \
                   -fstrict-overflow -fstrict-aliasing -Wdouble-promotion -fstack-usage \
                   -Wnull-dereference -Wcast-align -Wunused-variable -Wunused-parameter")

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions -fno-rtti")

# Process the BridgeHop library CMake component
add_subdirectory(bridgehop/)

# Build the project executable
set(LIB_ELF_NAME bridgehop-simulator)
add_executable(${LIB_ELF_NAME} host/main.cpp)
target_link_libraries(${LIB_ELF_NAME} PRIVATE bridgehop)

set_target_properties(${LIB_ELF_NAME} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS ON
)

# Targets for static analysis and formatting
find_program(CPPCHECK cppcheck)
find_program(CLANG_FORMAT clang-format)

if (CPPCHECK)
add_custom_target(cppcheck
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/.cppcheck
    COMMAND ${CPPCHECK}
        --enable=all
        --inline-suppr
        --cppcheck-build-dir=${CMAKE_BINARY_DIR}/.cppcheck
        --suppressions-list=${CMAKE_SOURCE_DIR}/cppcheck_suppressions.txt
        --error-exitcode=1
        --check-level=exhaustive
        -D__GNUC__
        --project=${CMAKE_BINARY_DIR}/compile_commands.json
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
