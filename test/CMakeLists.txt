# https://docs.hunter.sh/en/latest/packages/pkg/GTest.html
hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)

hunter_add_package(Microsoft.GSL)
find_package(Microsoft.GSL CONFIG REQUIRED)

erasure_coding_add_test(ec_test
        erasure_coding/reconstruct.cpp
    )
target_link_libraries(ec_test
    erasure_coding_crust
    )
