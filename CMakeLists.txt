cmake_minimum_required(VERSION 3.12)
project(heap_timer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(heap_timer INTERFACE)
target_include_directories(heap_timer INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

option(HEAP_TIMER_BUILD_TESTS "Build unit tests and benchmark binary" ON)

if(HEAP_TIMER_BUILD_TESTS)
    add_executable(heap_timer_test test.cpp)
    target_link_libraries(heap_timer_test PRIVATE heap_timer)
    target_compile_options(heap_timer_test PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra>
    )

    enable_testing()
    add_test(
        NAME heap_timer_unit
        COMMAND $<TARGET_FILE:heap_timer_test>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()
