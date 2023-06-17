cmake_minimum_required(VERSION 3.14)

set(PROJECT_VERSION 0.1)
project(recognizer VERSION ${PROJECT_VERSION} LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_PARALLEL_LEVEL 8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DHAVE_OPENBLAS)
add_definitions(-DQT_NO_KEYWORDS)


#include(FetchContent) # once in the project to include the module

#FetchContent_Declare(kaldi
#                     GIT_REPOSITORY https://github.com/kaldi-asr/kaldi
#                     FetchContent_MakeAvailable(kaldi))

find_package(PkgConfig REQUIRED)
if ( NOT (PKGCONFIG_FOUND) )
      message(FATAL_ERROR "Please Install PPkgConfig: CMake will Exit")
      endif()

        pkg_check_modules(GST REQUIRED gstreamer-1.0>=1.8)
        pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
        pkg_search_module(GLIB REQUIRED glib-2.0)

      if ( NOT (GST_FOUND) )
            message(FATAL_ERROR "Please Install Gstreamer Dev: CMake will Exit")
        endif()
      if ( NOT (GST_APP_FOUND) )
            message(FATAL_ERROR "Please Install Gstreamer App: CMake will Exit")
        endif()
      if ( NOT (GLIB_FOUND) )
            message(FATAL_ERROR "Please Install GLIB: CMake will Exit")
        endif()


include_directories("/home/user4/Download/kaldi")
include_directories("/home/user4/Download/kaldi")
include_directories("${GST_INCLUDE_DIRS}")

set(API_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/vosk-api/src/kaldi_recognizer.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/vosk-api/src/language_model.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/vosk-api/src/model.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/vosk-api/src/spk_model.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/vosk-api/src/vosk_api.cc"
)

add_library(vosk STATIC ${API_SOURCES})

find_package(Threads REQUIRED)

add_library(${PROJECT_NAME} SHARED)
target_sources(${PROJECT_NAME}
    PRIVATE
    listener.h
    listener.cpp
    recognizer.cpp
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -pedantic
    -Werror
)

target_compile_options(vosk PRIVATE
     -fPIC
)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/config $<TARGET_FILE_DIR:${PROJECT_NAME}>/..)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/vosk-api/src")
include_directories(/home/user4/Download/kaldi/src)
include_directories(/home/user4/Download/kaldi/tools/OpenBLAS)
include_directories(/home/user4/Download/kaldi/tools/openfst/include)
include_directories(/home/user4/Download/kaldi/tools/openfst/lib)

add_definitions(-DFST_NO_DYNAMIC_LINKING )

target_link_libraries(${PROJECT_NAME} PRIVATE vosk -ldl)

target_link_libraries(${PROJECT_NAME} PUBLIC
    ${GST_LIBRARIES}
    ${GST_APP_LIBRARIES}
    Threads::Threads
   /home/user4/Download/kaldi/src/lib/libkaldi-online2.so
   /home/user4/Download/kaldi/src/lib/libkaldi-decoder.so
   /home/user4/Download/kaldi/src/lib/libkaldi-ivector.so
   /home/user4/Download/kaldi/src/lib/libkaldi-gmm.so
   /home/user4/Download/kaldi/src/lib/libkaldi-nnet3.so
   /home/user4/Download/kaldi/src/lib/libkaldi-tree.so
   /home/user4/Download/kaldi/src/lib/libkaldi-feat.so
   /home/user4/Download/kaldi/src/lib/libkaldi-lat.so
   /home/user4/Download/kaldi/src/lib/libkaldi-lm.so
   /home/user4/Download/kaldi/src/lib/libkaldi-hmm.so
   /home/user4/Download/kaldi/src/lib/libkaldi-transform.so
   /home/user4/Download/kaldi/src/lib/libkaldi-cudamatrix.so
   /home/user4/Download/kaldi/src/lib/libkaldi-matrix.so
   /home/user4/Download/kaldi/src/lib/libkaldi-fstext.so
   /home/user4/Download/kaldi/src/lib/libkaldi-util.so
   /home/user4/Download/kaldi/src/lib/libkaldi-base.so
   /home/user4/Download/kaldi/tools/OpenBLAS/libopenblas.so
   /home/user4/Download/kaldi/tools/openfst/lib/libfst.so
   /home/user4/Download/kaldi/tools/openfst/lib/libfstngram.so
    -lgfortran -lm
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC
        # where top-level project will look for the library's public headers
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # where external projects will look for the library's public headers
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

set(public_header include/recognizer.h)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${public_header}")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        LIBRARY DESTINATION "/usr/lib"
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)


# generate and install export file
install(EXPORT "${PROJECT_NAME}Targets"
    FILE "${PROJECT_NAME}Targets.cmake"
    NAMESPACE recognizer::
    DESTINATION cmake
)

# generate the version file for the config file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY AnyNewerVersion
)
# create config file
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION cmake
)
# install config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION cmake
)
# generate the export targets for the build tree
export(EXPORT "${PROJECT_NAME}Targets"
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Targets.cmake"
    NAMESPACE recognizer::
)

