set(include_path ${PROJECT_SOURCE_DIR}/include)
set(ec_cpp_h_dir ${include_path}/ec-cpp)

add_library(ec-cpp 
    ./ec-cpp.cpp
)

set_target_properties(ec-cpp PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${include_path}
    )

file(MAKE_DIRECTORY ${ec_cpp_h_dir})

install(
    FILES ${PROJECT_SOURCE_DIR}/cmake/erasure_coding_crustConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/erasure_coding_crust
)

unset(include_path)
unset(ec_cpp_h_dir)
