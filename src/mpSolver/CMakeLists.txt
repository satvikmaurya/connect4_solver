add_library(mpSolver mpSolver.cpp boardMp.cpp)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    message(STATUS "found openmp")
    target_compile_options(mpSolver PRIVATE -fopenmp)
    target_link_libraries(mpSolver PUBLIC OpenMP::OpenMP_CXX)
endif()

target_include_directories(mpSolver PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}")
