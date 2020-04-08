
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(HEAPALLOC)

AUX_SOURCE_DIRECTORY(./ SRC_LIST)

add_executable(test ${SRC_LIST})
target_compile_definitions(test PRIVATE _HEAP_64)

