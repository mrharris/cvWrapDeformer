cmake_minimum_required(VERSION 2.6)
project(cvWrapDeformer)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)

SET(SOURCE_FILES
    "pluginMain.cpp"
    "cvWrapCmd.cpp"
	"cvWrapCmd.h"
    "cvWrapDeformer.cpp"
    "cvWrapDeformer.h")

find_package(Maya REQUIRED)

include_directories(${MAYA_INCLUDE_DIR})
link_directories(${MAYA_LIBRARY_DIR})
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${MAYA_LIBRARIES})

MAYA_PLUGIN(${PROJECT_NAME})

# cd build
# cmake -G "Visual Studio 14 2015 Win64" -DMAYA_VERSION=2017 ../src
# cmake --build . --config Release