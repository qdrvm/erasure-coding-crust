find_program(RUSTC rustc REQUIRED)
find_program(CARGO cargo REQUIRED)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  set(path_prefix "${CMAKE_BINARY_DIR}/release")
  set(release_option "--release")
  message(STATUS "CMAKE_BUILD_TYPE=Release, adding ${release_option}")
else ()
  set(path_prefix "${CMAKE_BINARY_DIR}/debug")
endif ()


if (BUILD_SHARED_LIBS)
  set(lib ${path_prefix}/${CMAKE_SHARED_LIBRARY_PREFIX}erasure_coding_crust${CMAKE_SHARED_LIBRARY_SUFFIX})
else ()
  set(lib ${path_prefix}/${CMAKE_STATIC_LIBRARY_PREFIX}erasure_coding_crust${CMAKE_STATIC_LIBRARY_SUFFIX})
endif ()
message(STATUS "[erasure_coding] library: ${lib}")


set(include_path ${PROJECT_SOURCE_DIR}/include)
set(erasure_coding_h_dir ${include_path}/erasure_coding)

### setup tasks
add_custom_target(
    cargo_build
    ALL
    COMMAND cargo build --target-dir ${CMAKE_BINARY_DIR} ${release_option}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_library(erasure_coding_crust STATIC IMPORTED GLOBAL)

# if we build static lib
if (NOT BUILD_SHARED_LIBS)
  if (APPLE)
    # on apple we need to link Security
    find_library(Security Security)
    find_package_handle_standard_args(erasure_coding_crust
        REQUIRED_VARS Security
        )
    set_target_properties(erasure_coding_crust PROPERTIES
        INTERFACE_LINK_LIBRARIES ${Security}
        )
  elseif (UNIX)
    # on Linux we need to link pthread
    target_link_libraries(erasure_coding_crust INTERFACE
        pthread
        -Wl,--no-as-needed
        dl
        )
  else ()
    message(WARNING "You're building static lib, it may not link. Come here and fix.")
  endif ()
endif ()

set_target_properties(erasure_coding_crust PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${include_path}
    IMPORTED_LOCATION ${lib}
    )
add_dependencies(erasure_coding_crust cargo_build)

file(MAKE_DIRECTORY ${erasure_coding_h_dir})

### add tests
add_test(
    NAME cargo_test
    COMMAND cargo test
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

### setup install task
include(GNUInstallDirs)

install(
    DIRECTORY ${erasure_coding_h_dir}
    TYPE INCLUDE
)
install(
    FILES ${lib}
    TYPE LIB
)

install(
    FILES ${PROJECT_SOURCE_DIR}/cmake/erasure_coding_crustConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/erasure_coding_crust
)
