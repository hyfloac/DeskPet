cmake_minimum_required(VERSION 3.23)
project(SysLib VERSION 0.0.1 LANGUAGES CXX C)

# Recursively collect all .cpp and .c files.
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c")
# Collect all .cpp and .c files in the root source directory. 
file(GLOB SOURCES_BASE "src/*.cpp" "src/*.c")
# Recursively collect all .hpp, .h, and .inl files in the include directory.
file(GLOB_RECURSE HEADERS "include/*.hpp" "include/*.h" "include/*.inl")
# Collect all .hpp, .h, and .inl files in the root include directory. 
file(GLOB HEADERS_BASE "include/*.hpp" "include/*.h" "include/*.inl")
# Recursively collect all .hpp, .h, and .inl files in the private directory.
file(GLOB_RECURSE PRIVATE_HEADERS "private/*.hpp" "private/*.h" "private/*.inl")

# Store all of the required files in a variable. 
# This is mostly for shared/static libraries.
set(SYSLIB_SOURCE_FILES ${SOURCES_BASE} ${HEADERS_BASE} ${HEADERS_PRIVATE})

# Setup the Static library with the source files.
add_library(${PROJECT_NAME} STATIC ${SYSLIB_SOURCE_FILES})

# Create a "folder" for the headers in Visual Studio
foreach(_source IN ITEMS ${HEADERS})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "/" "\\" _source_dir_corrected "${CMAKE_SOURCE_DIR}")
    string(REPLACE "/" "\\" _source_path "${_source_path}")
    string(REPLACE "${_source_dir_corrected}\\${PROJECT_NAME}" "" _group_path "${_source_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

# Create a "folder" for the private headers in Visual Studio
foreach(_source IN ITEMS ${PRIVATE_HEADERS})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "/" "\\" _source_dir_corrected "${CMAKE_SOURCE_DIR}")
    string(REPLACE "/" "\\" _source_path "${_source_path}")
    string(REPLACE "${_source_dir_corrected}\\${PROJECT_NAME}" "" _group_path "${_source_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

# Create a "folder" for the sources in Visual Studio
foreach(_source IN ITEMS ${SOURCES})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "/" "\\" _source_dir_corrected "${CMAKE_SOURCE_DIR}")
    string(REPLACE "/" "\\" _source_path "${_source_path}")
    string(REPLACE "${_source_dir_corrected}\\${PROJECT_NAME}" "" _group_path "${_source_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

# Set the sources as part of the private interface.
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
# Set the headers as part of the public interface.
target_sources(${PROJECT_NAME} PUBLIC FILE_SET "HEADERS" BASE_DIRS "include" FILES ${HEADERS})
# Set the private headers as part of the private interface.
target_sources(${PROJECT_NAME} PRIVATE FILE_SET "headers_private_${PROJECT_NAME}" TYPE "HEADERS" BASE_DIRS "private" FILES ${PRIVATE_HEADERS})

# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${PROJECT_NAME}> $<TARGET_FILE_DIR:Win32CliPet>)

# Set the include directory.
target_include_directories(${PROJECT_NAME} PUBLIC include)
# Set the private include directory.
target_include_directories(${PROJECT_NAME} PRIVATE private)
# Set the source directory.
target_include_directories(${PROJECT_NAME} PRIVATE src)

# Set C++20
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        # using clang with clang-cl front end
    elseif(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
        # using clang with regular front end
    endif()
endif()

if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    # Disable exceptions and ignore some CRT warnings
    target_compile_definitions(${PROJECT_NAME} PRIVATE -D_CRT_SECURE_NO_WARNINGS -D_HAS_EXCEPTIONS=1)

    # Set the runtime library to be dynamic.
    set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    
    # Set some debug PDB flags.
    target_compile_options(${PROJECT_NAME} PRIVATE "$<$<NOT:$<CONFIG:Debug>>:/Zi>")
    target_link_options(${PROJECT_NAME} PRIVATE "$<$<NOT:$<CONFIG:Debug>>:/DEBUG>")
    target_link_options(${PROJECT_NAME} PRIVATE "$<$<NOT:$<CONFIG:Debug>>:/OPT:REF>")
    target_link_options(${PROJECT_NAME} PRIVATE "$<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

install(
    TARGETS ${PROJECT_NAME} 
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    FILE_SET HEADERS
)
