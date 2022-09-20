#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "VLCQt::Qml" for configuration "Release"
set_property(TARGET VLCQt::Qml APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(VLCQt::Qml PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/VLCQtQml.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/VLCQtQml.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS VLCQt::Qml )
list(APPEND _IMPORT_CHECK_FILES_FOR_VLCQt::Qml "${_IMPORT_PREFIX}/lib/VLCQtQml.lib" "${_IMPORT_PREFIX}/bin/VLCQtQml.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
