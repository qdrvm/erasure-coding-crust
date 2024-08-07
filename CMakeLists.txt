cmake_minimum_required(VERSION 3.12)

find_program(CCACHE_FOUND ccache)
if (CCACHE_FOUND)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif (CCACHE_FOUND)

option(TESTING "Enable tests" OFF)
option(BENCHMARK "Enable benchmark" OFF)

if (TESTING OR BENCHMARK)
  include("cmake/3rdparty/Hunter/init.cmake")
  HunterGate(
      URL https://github.com/qdrvm/hunter/archive/refs/tags/v0.25.3-qdrvm19.zip
      SHA1 eed8b8333c14f25176d4af4fb26256981fd1b527
  )
endif ()

project(novelpoly-reed-solomon-crust C CXX)

include(FindPackageHandleStandardArgs)
include(cmake/functions.cmake)

include(cmake/rust.cmake)

if (NOT EXISTS "${CMAKE_TOOLCHAIN_FILE}")
  # https://cgold.readthedocs.io/en/latest/tutorials/toolchain/globals/cxx-standard.html#summary
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
  set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
endif ()

add_subdirectory(ec-cpp)
if (TESTING)
  enable_testing()
  add_subdirectory(test)
endif ()

if (BENCHMARK)
  add_subdirectory(benchmark)
endif ()


# TODO: Consider changing to CMake 3.23 FILE_SETs
install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

export(EXPORT erasure_coding_crust_targets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/erasure_coding_crust/erasure_coding_crustTargets.cmake"
  NAMESPACE erasure_coding_crust::
)
configure_file(cmake/erasure_coding_crustConfig.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/erasure_coding_crust/erasure_coding_crustConfig.cmake"
  COPYONLY
)

set(ConfigPackageLocation lib/cmake/erasure_coding_crust)
install(EXPORT erasure_coding_crust_targets
  FILE
    erasure_coding_crustTargets.cmake
  NAMESPACE
    erasure_coding_crust::
  DESTINATION
    ${ConfigPackageLocation}
)
install(
  FILES
    cmake/erasure_coding_crustConfig.cmake
  DESTINATION
    ${ConfigPackageLocation}
)
