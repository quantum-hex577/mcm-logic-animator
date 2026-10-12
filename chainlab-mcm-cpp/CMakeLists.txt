cmake_minimum_required(VERSION 3.16)
project(ChainLabMCM LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(chainlab_mcm src/mcm.cpp)
target_include_directories(chainlab_mcm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(chainlab src/main.cpp)
target_link_libraries(chainlab PRIVATE chainlab_mcm)

add_executable(chainlab_tests tests/tests.cpp)
target_link_libraries(chainlab_tests PRIVATE chainlab_mcm)

enable_testing()
add_test(NAME chainlab_regression COMMAND chainlab_tests)
