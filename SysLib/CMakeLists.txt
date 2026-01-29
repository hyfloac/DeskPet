cmake_minimum_required(VERSION 3.23)
project(SysLib VERSION 0.0.1 LANGUAGES CXX C)

# We use this to check for some compiler flags, mostly to disable warnings.
include(CheckCCompilerFlag)
# This is a helper utility for generating the folder layout in VS.
include(GenVsFilters)
# This sets the compile flags we will use.
include(SetCompileFlags)

# Recursively collect all .cpp and .c files.
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c")
# Recursively collect all .hpp, .h, and .inl files in the include directory.
file(GLOB_RECURSE HEADERS "include/*.hpp" "include/*.h" "include/*.inl")
# Recursively collect all .hpp, .h, and .inl files in the private directory.
file(GLOB_RECURSE PRIVATE_HEADERS "private/*.hpp" "private/*.h" "private/*.inl")

# Setup the Static library with the source files.
add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS} ${PRIVATE_HEADERS})

# Create a "folder" for the sources in Visual Studio
GenVsFilters(SOURCES)
# Create a "folder" for the headers in Visual Studio
GenVsFilters(HEADERS)
# Create a "folder" for the private headers in Visual Studio
GenVsFilters(PRIVATE_HEADERS)

# Set the sources as part of the private interface.
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
# Set the headers as part of the public interface.
target_sources(${PROJECT_NAME} PUBLIC FILE_SET "HEADERS" BASE_DIRS "include" FILES ${HEADERS})
# Set the private headers as part of the private interface.
target_sources(${PROJECT_NAME} PRIVATE FILE_SET "headers_private_${PROJECT_NAME}" TYPE "HEADERS" BASE_DIRS "private" FILES ${PRIVATE_HEADERS})

# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${PROJECT_NAME}> $<TARGET_FILE_DIR:Win32CliPet>)

# Set the source directory.
target_include_directories(${PROJECT_NAME} PRIVATE src)
# Set the include directory.
target_include_directories(${PROJECT_NAME} PUBLIC include)
# Set the private include directory.
target_include_directories(${PROJECT_NAME} PRIVATE private)

# Set the compiler flags we will use.
SetCompileFlags(${PROJECT_NAME} PUBLIC PRIVATE OFF)

install(
    TARGETS ${PROJECT_NAME} 
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    FILE_SET HEADERS
)
