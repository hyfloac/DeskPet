cmake_minimum_required(VERSION 3.23)
project(CliPet VERSION 0.0.1 LANGUAGES CXX C)

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

# Setup the executable with the source files.
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Create a "folder" for the sources in Visual Studio
GenVsFilters(SOURCES)
# Create a "folder" for the headers in Visual Studio
GenVsFilters(HEADERS)

# Set the sources as part of the private interface.
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

# Set the headers as part of the private interface.
target_sources(${PROJECT_NAME} PRIVATE FILE_SET "HEADERS" BASE_DIRS "include" FILES ${HEADERS})

# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SysLib> $<TARGET_FILE_DIR:${PROJECT_NAME}>)
# Copy the DLL for the PetAI to the output directory.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:PetAI> $<TARGET_FILE_DIR:${PROJECT_NAME}>)
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_LINKER_FILE:PetAI> $<TARGET_FILE_DIR:${PROJECT_NAME}>)

# Link the required libraries.
target_link_libraries(${PROJECT_NAME} SysLib PetAI)

# Set the include directory.
target_include_directories(${PROJECT_NAME} PUBLIC include)
# Set the source directory.
target_include_directories(${PROJECT_NAME} PRIVATE src)

# Set the compiler flags we will use.
SetCompileFlags(${PROJECT_NAME} PUBLIC PRIVATE ${BUILD_SHARED_LIBS})

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    FILE_SET HEADERS
)
