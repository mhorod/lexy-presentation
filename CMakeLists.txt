cmake_minimum_required(VERSION 3.16)
project(lexy-presentation)

# Download the library as dependency
include(FetchContent)
FetchContent_Declare(lexy URL https://lexy.foonathan.net/download/lexy-src.zip)
FetchContent_MakeAvailable(lexy)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ipv4_address example
add_executable(ipv4_address ipv4_address.cpp)
target_link_libraries(ipv4_address PRIVATE foonathan::lexy)

# calculator example
add_executable(calculator calculator.cpp)
target_link_libraries(calculator PRIVATE foonathan::lexy)

# mini_lambda example
add_executable(mini_lambda mini_lambda.cpp)
target_link_libraries(mini_lambda PRIVATE foonathan::lexy)