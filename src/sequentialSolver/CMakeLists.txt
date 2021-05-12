add_library(sequentialSolver STATIC sequentialSolver.cpp boardSeq.cpp)

target_include_directories(sequentialSolver PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}")