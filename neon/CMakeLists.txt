cmake_minimum_required(VERSION 3.10.2)

project(demo LANGUAGES C CXX ASM)

add_definitions("-Wall")

# platform
set( CMAKE_SYSTEM_NAME Android )

# flags
# add_definitions( -march=armv8.2-a+fp16 -mfloat-abi=softfp -mfpu=neon)
add_definitions( -march=armv8.2-a+fp16)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# file(GLOB MNN_AArch64_SRC ${CMAKE_CURRENT_LIST_DIR}/*.[sS])
# add_library(MNNARM64 OBJECT ${MNN_AArch64_SRC})

# set_property(SOURCE MNNAxByClampBroadcastC4.s APPEND PROPERTY COMPILE_OPTIONS "-x" "assembler-with-cpp")
# set_property(SOURCE MNNAxByClampBroadcastC4.S PROPERTY LANGUAGE C)

add_executable(
    test
    main.cpp
    ./boxfilter/boxfilter.cpp
)

target_link_libraries(
    test
    android
    log
)