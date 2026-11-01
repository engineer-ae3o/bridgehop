cmake_minimum_required(VERSION 4.0)

set(SRCS
    src/node.cpp
)

set(INC
    ../
    ../third-party/etl/include
)

# Create library
add_library(utils STATIC ${SRCS})
target_include_directories(utils PUBLIC ${INC})

set_target_properties(utils PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS ON
)
