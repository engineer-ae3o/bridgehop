cmake_minimum_required(VERSION 4.0)

set(CMAKE_PROJECT_NAME BridgeHop)
set(LIB_ELF_NAME bridgehop)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(${CMAKE_PROJECT_NAME})
enable_language(C CXX)

add_library(${LIB_ELF_NAME})
target_include_directories(${LIB_ELF_NAME} PUBLIC active/ passive/ utils/ third-party/etl/include/)

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

# Process all active components
add_subdirectory(./utils/)
add_subdirectory(./active/)
add_subdirectory(./passive/)

if (false)
add_custom_command(
    TARGET ${LIB_ELF_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "CREATE $<TARGET_FILE:${LIB_ELF_NAME}>.tmp.a" > combine.mri
    COMMAND ${CMAKE_COMMAND} -E echo "ADDLIB $<TARGET_FILE:${LIB_ELF_NAME}>" >> combine.mri
    COMMAND ${CMAKE_COMMAND} -E echo "ADDLIB $<TARGET_FILE:active>" >> combine.mri
    COMMAND ${CMAKE_COMMAND} -E echo "ADDLIB $<TARGET_FILE:passive>" >> combine.mri
    COMMAND ${CMAKE_COMMAND} -E echo "ADDLIB $<TARGET_FILE:utils>" >> combine.mri
    COMMAND ${CMAKE_COMMAND} -E echo "SAVE" >> combine.mri
    COMMAND ${CMAKE_COMMAND} -E echo "END" >> combine.mri
    COMMAND ${CMAKE_AR} -M < combine.mri
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${LIB_ELF_NAME}>.tmp.a $<TARGET_FILE:${LIB_ELF_NAME}>
    COMMAND ${CMAKE_COMMAND} -E remove $<TARGET_FILE:${LIB_ELF_NAME}>.tmp.a combine.mri
    COMMENT "Bundling static dependencies into lib${LIB_ELF_NAME}.a..."
    COMMAND ${CMAKE_SIZE} "lib${LIB_ELF_NAME}.a"
    VERBATIM
)
endif()

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
