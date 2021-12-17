include(CheckSymbolExists)
include(GNUInstallDirs)

if (WIN32)
    check_symbol_exists(printf_s "stdio.h" HAVE_PRINTF_S)
    check_symbol_exists(sscanf_s "stdio.h" HAVE_SSCANF_S)
endif()

configure_file(fuckzip_config.h.in "${CMAKE_CURRENT_BINARY_DIR}/fuckzip_config.h")

set(
    FUCKZIP_FILES
    "${CMAKE_CURRENT_BINARY_DIR}/fuckzip_version.h"
    "${CMAKE_CURRENT_BINARY_DIR}/fuckzip_config.h"
    extract_zip.cpp
    extract_zip.h
    main.cpp
)

if (WIN32)
    list(APPEND FUCKZIP_FILES "${CMAKE_CURRENT_BINARY_DIR}/fuckzip.rc")
    set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/fuckzip.rc" PROPERTIES GENERATED TRUE)
endif()
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/fuckzip_version.h" PROPERTIES GENERATED TRUE)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("${CMAKE_BINARY_DIR}/utils")

add_custom_target(fuckzip-version ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_SOURCE_DIR}/FUCKZIP_VERSION.cmake")
add_executable(fuckzip "${FUCKZIP_FILES}")
add_dependencies(fuckzip fuckzip-version)
target_link_libraries(fuckzip LIBZIP::LIBZIP)
target_link_libraries(fuckzip utils)
if (TARGET getopt)
    target_link_libraries(fuckzip getopt)
endif()
install(TARGETS fuckzip)
