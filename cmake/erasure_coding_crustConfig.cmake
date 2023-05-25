include(GNUInstallDirs)

# Compute the installation prefix relative to this file.
get_filename_component(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

set(shared_lib_name ${CMAKE_SHARED_LIBRARY_PREFIX}erasure_coding_crust${CMAKE_SHARED_LIBRARY_SUFFIX})
set(shared_lib_path ${_IMPORT_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${shared_lib_name})
set(static_lib_name ${CMAKE_STATIC_LIBRARY_PREFIX}erasure_coding_crust${CMAKE_STATIC_LIBRARY_SUFFIX})
set(static_lib_path ${_IMPORT_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${static_lib_name})
if(EXISTS ${shared_lib_path})
    set(lib_path ${shared_lib_path})
elseif(EXISTS ${static_lib_path})
    set(lib_path ${static_lib_path})
else()
    message(FATAL_ERROR "erasure_coding_crust library (${shared_lib_name} or ${static_lib_name}) not found in ${_IMPORT_PREFIX}/${CMAKE_INSTALL_LIBDIR}!")
endif()

set(shared_lib_name_ec_cpp ${CMAKE_SHARED_LIBRARY_PREFIX}ec-cpp${CMAKE_SHARED_LIBRARY_SUFFIX})
set(shared_lib_path_ec_cpp ${_IMPORT_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${shared_lib_name_ec_cpp})
set(static_lib_name_ec_cpp ${CMAKE_STATIC_LIBRARY_PREFIX}ec-cpp${CMAKE_STATIC_LIBRARY_SUFFIX})
set(static_lib_path_ec_cpp ${_IMPORT_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${static_lib_name_ec_cpp})
if(EXISTS ${shared_lib_path_ec_cpp})
    set(lib_path_ec_cpp ${shared_lib_path_ec_cpp})
elseif(EXISTS ${static_lib_path_ec_cpp})
    set(lib_path_ec_cpp ${static_lib_path_ec_cpp})
else()
    message(FATAL_ERROR "ec-cpp library (${shared_lib_name_ec_cpp} or ${static_lib_name_ec_cpp}) not found in ${_IMPORT_PREFIX}/${CMAKE_INSTALL_LIBDIR}!")
endif()

set(include_path ${_IMPORT_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
set(include_path_ec_cpp ${_IMPORT_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
if(NOT EXISTS ${include_path})
    message(FATAL_ERROR "erasure_coding_crust headers not found in ${include_path}!")
endif()

if(NOT TARGET erasure_coding_crust::ec-cpp)
    add_library(erasure_coding_crust::ec-cpp STATIC IMPORTED GLOBAL)
    set_target_properties(erasure_coding_crust::ec-cpp PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${include_path_ec_cpp}
        IMPORTED_LOCATION ${lib_path_ec_cpp}
        )
endif()

if(NOT TARGET erasure_coding_crust::erasure_coding_crust)
    add_library(erasure_coding_crust::erasure_coding_crust STATIC IMPORTED GLOBAL)

    if(EXISTS ${_IMPORT_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${static_lib_name})
        if (APPLE)
            # on apple we need to link Security
            find_library(Security Security)
            find_package_handle_standard_args(erasure_coding_crust
                REQUIRED_VARS Security
                )
            set_target_properties(erasure_coding_crust::erasure_coding_crust PROPERTIES
                INTERFACE_LINK_LIBRARIES ${Security}
                )
        elseif (UNIX)
            # on Linux we need to link pthread
            target_link_libraries(erasure_coding_crust::erasure_coding_crust INTERFACE
                pthread
                -Wl,--no-as-needed
                dl
                )
        else ()
            message(ERROR "You've built static lib, it may not link on this platform. Come here and fix.")
        endif ()
    endif()


    set_target_properties(erasure_coding_crust::erasure_coding_crust PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${include_path}
        IMPORTED_LOCATION ${lib_path}
        )
endif()

unset(shared_lib_name)
unset(static_lib_name)
unset(shared_lib_path)
unset(static_lib_path)
unset(lib_path)
unset(shared_lib_name_ec_cpp)
unset(static_lib_name_ec_cpp)
unset(shared_lib_path_ec_cpp)
unset(static_lib_path_ec_cpp)
unset(lib_path_ec_cpp)
unset(include_path)
unset(include_path_ec_cpp)

check_required_components(erasure_coding_crust)
