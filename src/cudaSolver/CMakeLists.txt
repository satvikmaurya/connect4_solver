add_library(cudaSolver cudaSolver.cu)

target_include_directories(cudaSolver PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}")