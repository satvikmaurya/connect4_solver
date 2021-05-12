cmake_minimum_required(VERSION 3.10)

# set the project name
project(ConnectFour LANGUAGES CXX)

# specify the C++ 17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(connectFourAssets)
add_subdirectory(sequentialSolver)
add_subdirectory(mpSolver)
find_package(CUDA)
if(CUDA_FOUND)
    add_definitions(-DCUDA_FOUND)
    enable_language(CUDA)
    add_subdirectory(cudaSolver)
endif()

add_executable(app main.cpp)

add_library(gameTreeSearch gameTreeSearchSolver.hpp)
target_link_libraries(gameTreeSearch sequentialSolver)
target_link_libraries(gameTreeSearch mpSolver)
target_link_libraries(app PUBLIC sequentialSolver)
target_link_libraries(app PUBLIC mpSolver)
if(CUDA_FOUND)
    target_link_libraries(app PUBLIC cudaSolver -lcublas)
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --default-stream per-thread")
endif()
target_link_libraries(app PUBLIC connectFourAssets)


target_include_directories(app PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/connectFourAssets"
                          "${PROJECT_SOURCE_DIR}/sequentialSolver"
                          "${PROJECT_SOURCE_DIR}/mpSolver")

if(CUDA_FOUND)
    target_include_directories(app PUBLIC
                               "${PROJECT_SOURCE_DIR}/cudaSolver")
endif()

target_compile_features(app PRIVATE cxx_std_17)
target_compile_options(app PRIVATE -Wall)
