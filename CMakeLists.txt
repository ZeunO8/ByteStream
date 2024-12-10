cmake_minimum_required(VERSION 3.30)
project(ByteStream VERSION 1.0.1)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_library(ByteStream STATIC src/ByteStream.cpp)

function(create_test TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} ByteStream)
    add_test(NAME ${TEST_NAME} COMMAND ${CMAKE_BINARY_DIR}/${TEST_NAME})
endfunction()

include(CTest)
enable_testing()
create_test(Simple tests/Simple.cpp)
