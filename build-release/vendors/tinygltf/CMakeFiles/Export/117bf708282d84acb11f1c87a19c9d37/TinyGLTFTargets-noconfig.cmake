#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "tinygltf::tinygltf" for configuration ""
set_property(TARGET tinygltf::tinygltf APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(tinygltf::tinygltf PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libtinygltf.a"
  )

list(APPEND _cmake_import_check_targets tinygltf::tinygltf )
list(APPEND _cmake_import_check_files_for_tinygltf::tinygltf "${_IMPORT_PREFIX}/lib/libtinygltf.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
