#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "recognizer::recognizer" for configuration ""
set_property(TARGET recognizer::recognizer APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(recognizer::recognizer PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "/usr/lib/librecognizer.so"
  IMPORTED_SONAME_NOCONFIG "librecognizer.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS recognizer::recognizer )
list(APPEND _IMPORT_CHECK_FILES_FOR_recognizer::recognizer "/usr/lib/librecognizer.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
