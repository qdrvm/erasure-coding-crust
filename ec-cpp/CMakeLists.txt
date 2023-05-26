if (CMAKE_BUILD_TYPE STREQUAL "Release")
  set(path_prefix "${CMAKE_BINARY_DIR}/ec-cpp")
else ()
  set(path_prefix "${CMAKE_BINARY_DIR}/ec-cpp")
endif ()

if (BUILD_SHARED_LIBS)
  set(lib_prefix ${CMAKE_SHARED_LIBRARY_PREFIX})
  set(lib_suffix ${CMAKE_SHARED_LIBRARY_SUFFIX})
else ()
  set(lib_prefix ${CMAKE_STATIC_LIBRARY_PREFIX})
  set(lib_suffix ${CMAKE_STATIC_LIBRARY_SUFFIX})
endif ()


if (CMAKE_BUILD_TYPE STREQUAL "Release")
  set(lib_ec_cpp ${path_prefix}/${lib_prefix}ec-cpp${lib_suffix})
else ()
  set(lib_ec_cpp ${path_prefix}/${lib_prefix}ec-cpp${CMAKE_DEBUG_POSTFIX}${lib_suffix})
endif ()
message(STATUS "[ec-cpp] library: ${lib_ec_cpp}")

set(include_path ${PROJECT_SOURCE_DIR}/include)
set(ec_cpp_h_dir ${include_path}/ec-cpp)

add_library(ec-cpp
    ./ec-cpp.cpp
)

# if we build static lib
if (NOT BUILD_SHARED_LIBS)
  if (APPLE)
    # on apple we need to link Security
    find_library(Security Security)
    find_package_handle_standard_args(ec-cpp
        REQUIRED_VARS Security
        )
    set_target_properties(ec-cpp PROPERTIES
        INTERFACE_LINK_LIBRARIES ${Security}
        )
  elseif (UNIX)
    # on Linux we need to link pthread
    target_link_libraries(ec-cpp INTERFACE
        pthread
        -Wl,--no-as-needed
        dl
        )
  else ()
    message(WARNING "You're building static lib, it may not link. Come here and fix.")
  endif ()
endif ()

set_target_properties(ec-cpp PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${include_path}
    IMPORTED_LOCATION ${lib_ec_cpp}
    )

file(MAKE_DIRECTORY ${ec_cpp_h_dir})

### setup install task
include(GNUInstallDirs)

install(
    DIRECTORY ${ec_cpp_h_dir}
    TYPE INCLUDE
)
install(
    FILES ${lib_ec_cpp}
    TYPE LIB
)

install(
    FILES ${PROJECT_SOURCE_DIR}/cmake/erasure_coding_crustConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/erasure_coding_crust
)
