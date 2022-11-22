cmake_minimum_required(VERSION 3.16)
project(lexy-presentation)

# Download the library as dependency
include(FetchContent)
FetchContent_Declare(lexy URL https://lexy.foonathan.net/download/lexy-src.zip)
FetchContent_MakeAvailable(lexy)

# Set output to bin folder
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 0_basic_api
add_executable(0_basic_api examples/0_basic_api.cpp)
target_link_libraries(0_basic_api PRIVATE foonathan::lexy)

# ipv4_address example
add_executable(1_ipv4_address examples/1_ipv4_address.cpp)
target_link_libraries(1_ipv4_address PRIVATE foonathan::lexy)

# 2_advanced_api example
add_executable(2_advanced_api examples/2_advanced_api.cpp)
target_link_libraries(2_advanced_api PRIVATE foonathan::lexy)

# calculator example
add_executable(3_calculator examples/3_calculator.cpp)
target_link_libraries(3_calculator PRIVATE foonathan::lexy)

# mini_lambda example
add_executable(4_mini_lambda examples/4_mini_lambda.cpp)
target_link_libraries(4_mini_lambda PRIVATE foonathan::lexy)

# 5_simple_ethernet_frame example
add_executable(5_simple_ethernet_frame examples/5_simple_ethernet_frame.cpp)
target_link_libraries(5_simple_ethernet_frame PRIVATE foonathan::lexy)