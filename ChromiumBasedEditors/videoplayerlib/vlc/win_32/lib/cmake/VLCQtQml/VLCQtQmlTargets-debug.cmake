#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "VLCQt::Qml" for configuration "Debug"
set_property(TARGET VLCQt::Qml APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(VLCQt::Qml PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/VLCQtQmld.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/VLCQtQmld.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS VLCQt::Qml )
list(APPEND _IMPORT_CHECK_FILES_FOR_VLCQt::Qml "${_IMPORT_PREFIX}/lib/VLCQtQmld.lib" "${_IMPORT_PREFIX}/bin/VLCQtQmld.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
