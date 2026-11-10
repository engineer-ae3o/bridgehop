cmake_minimum_required(VERSION 4.0)

enable_language(CXX)
set(LIB_ELF_NAME bridgehop)

add_library(${LIB_ELF_NAME} INTERFACE)
target_include_directories(${LIB_ELF_NAME} INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/.>
    $<INSTALL_INTERFACE:.>
)

set_target_properties(${LIB_ELF_NAME} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS ON
)
