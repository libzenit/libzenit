#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "libzen::zen" for configuration "Debug"
set_property(TARGET libzen::zen APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(libzen::zen PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libzen.a"
  )

list(APPEND _cmake_import_check_targets libzen::zen )
list(APPEND _cmake_import_check_files_for_libzen::zen "${_IMPORT_PREFIX}/lib/libzen.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
