cmake_minimum_required(VERSION 3.14)

project(voiceRecognizer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_PARALLEL_LEVEL 8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)

#--------------------------------
add_executable(${PROJECT_NAME}
    main.cpp
    worker.h
    worker.cpp
)
target_link_libraries(${PROJECT_NAME} PUBLIC
        recognizer
)
#--------------------------------
