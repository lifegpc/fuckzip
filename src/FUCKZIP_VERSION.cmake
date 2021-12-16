
set(FUCKZIP_VERSION_MAJOR 1)
set(FUCKZIP_VERSION_MINOR 0)
set(FUCKZIP_VERSION_MICRO 0)
set(FUCKZIP_VERSION_REV 0)
set(FUCKZIP_VERSION ${FUCKZIP_VERSION_MAJOR}.${FUCKZIP_VERSION_MINOR}.${FUCKZIP_VERSION_MICRO}.${FUCKZIP_VERSION_REV})
if (WIN32)
    message(STATUS "Generate \"${CMAKE_CURRENT_BINARY_DIR}/fuckzip.rc\"")
    configure_file("${CMAKE_CURRENT_LIST_DIR}/fuckzip.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/fuckzip.rc")
endif()
message(STATUS "Generate \"${CMAKE_CURRENT_BINARY_DIR}/fuckzip_version.h\"")
configure_file("${CMAKE_CURRENT_LIST_DIR}/fuckzip_version.h.in" "${CMAKE_CURRENT_BINARY_DIR}/fuckzip_version.h")