
add_library(ec-cpp
    ./ec-cpp.cpp
)

target_include_directories(ec-cpp PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../include>
  $<INSTALL_INTERFACE:include>
  )

install(
    TARGETS ec-cpp EXPORT erasure_coding_crust_targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ec-cpp
)
