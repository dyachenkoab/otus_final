cmake_minimum_required(VERSION 3.14)

project(voiceRecognizer LANGUAGES CXX)

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

include_directories("/home/user4/Download/kaldi")
include_directories("${GST_INCLUDE_DIRS}")

set(API_SOURCES
    "${CMAKE_SOURCE_DIR}/vosk-api/src/kaldi_recognizer.cc"
    "${CMAKE_SOURCE_DIR}/vosk-api/src/language_model.cc"
    "${CMAKE_SOURCE_DIR}/vosk-api/src/model.cc"
    "${CMAKE_SOURCE_DIR}/vosk-api/src/spk_model.cc"
    "${CMAKE_SOURCE_DIR}/vosk-api/src/vosk_api.cc"
)

add_library(vosk STATIC ${API_SOURCES})


set(PROJECT_SOURCES
        main.cpp
        recognizer.c
)

add_executable(${PROJECT_NAME}
          ${PROJECT_SOURCES}
)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/config $<TARGET_FILE_DIR:${PROJECT_NAME}>)

include_directories("${CMAKE_SOURCE_DIR}/vosk-api/src")
include_directories(/home/user4/Download/kaldi/src)
include_directories(/home/user4/Download/kaldi/tools/OpenBLAS)
include_directories(/home/user4/Download/kaldi/tools/openfst/include)
include_directories(/home/user4/Download/kaldi/tools/openfst/lib)

add_definitions(-DFST_NO_DYNAMIC_LINKING )

target_link_libraries(${PROJECT_NAME} vosk -ldl)

target_link_libraries(${PROJECT_NAME}
    ${GST_LIBRARIES}
    ${GST_APP_LIBRARIES}
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
    -lgfortran -lpthread -lm
)
