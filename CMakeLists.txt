cmake_minimum_required (VERSION 3.13)

project(ArcnetAnalyzer)

add_definitions( -DLOGIC2 )

# enable generation of compile_commands.json, helpful for IDEs to locate include files.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# custom CMake Modules are located in the cmake directory.
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_BUILD_TYPE Debug)

include(ExternalAnalyzerSDK)

set(SOURCES 
src/ArcnetAnalyzer.cpp
src/ArcnetAnalyzer.h
src/ArcnetAnalyzerResults.cpp
src/ArcnetAnalyzerResults.h
src/ArcnetAnalyzerSettings.cpp
src/ArcnetAnalyzerSettings.h
src/ArcnetSimulationDataGenerator.cpp
src/ArcnetSimulationDataGenerator.h
)

add_analyzer_plugin(${PROJECT_NAME} SOURCES ${SOURCES})
