add_executable(benchmark
    benchmark.cpp
)

target_link_libraries(benchmark
    erasure_coding_crust
    ec-cpp
)
