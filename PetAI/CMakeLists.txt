cmake_minimum_required(VERSION 3.23)
project(PetAI VERSION 0.0.1 LANGUAGES CXX C)

# Used to disable some warnings.
include(CheckCCompilerFlag)

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
set(PETAI_SOURCE_FILES ${SOURCES_BASE} ${HEADERS_BASE} ${HEADERS_PRIVATE})

# Setup the Dynamic and Static libraries with the source files.
add_library("${PROJECT_NAME}Shared" SHARED ${PETAI_SOURCE_FILES})
add_library("${PROJECT_NAME}Static" STATIC ${PETAI_SOURCE_FILES})

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
target_sources("${PROJECT_NAME}Shared" PRIVATE ${SOURCES})
target_sources("${PROJECT_NAME}Static" PRIVATE ${SOURCES})

# Set the headers as part of the public interface.
target_sources("${PROJECT_NAME}Shared" PUBLIC FILE_SET "HEADERS" BASE_DIRS "include" FILES ${HEADERS})
target_sources("${PROJECT_NAME}Static" PUBLIC FILE_SET "HEADERS" BASE_DIRS "include" FILES ${HEADERS})

# Set the private headers as part of the private interface.
target_sources("${PROJECT_NAME}Shared" PRIVATE FILE_SET "headers_private_${PROJECT_NAME}" TYPE "HEADERS" BASE_DIRS "private" FILES ${PRIVATE_HEADERS})
target_sources("${PROJECT_NAME}Static" PRIVATE FILE_SET "headers_private_${PROJECT_NAME}" TYPE "HEADERS" BASE_DIRS "private" FILES ${PRIVATE_HEADERS})

# Copy the DLL for the PetAI to the output directory of Win32CliPet.
add_custom_command(TARGET ${PROJECT_NAME}Shared POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${PROJECT_NAME}Shared> $<TARGET_FILE_DIR:Win32CliPet>)
# add_custom_command(TARGET ${PROJECT_NAME}Shared POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_LINKER_FILE:${PROJECT_NAME}Shared> $<TARGET_FILE_DIR:Win32CliPet>)

# Link the required libraries.
target_link_libraries("${PROJECT_NAME}Shared" SysLib)
target_link_libraries("${PROJECT_NAME}Static" SysLib)

# Set the include directory.
target_include_directories("${PROJECT_NAME}Shared" PUBLIC include)
target_include_directories("${PROJECT_NAME}Static" PUBLIC include)
# Set the private include directory.
target_include_directories("${PROJECT_NAME}Shared" PUBLIC private)
target_include_directories("${PROJECT_NAME}Static" PUBLIC private)
# Set the source directory.
target_include_directories("${PROJECT_NAME}Shared" PRIVATE src)
target_include_directories("${PROJECT_NAME}Static" PRIVATE src)

# Set C++20
target_compile_features("${PROJECT_NAME}Shared" PUBLIC cxx_std_20)
target_compile_features("${PROJECT_NAME}Static" PUBLIC cxx_std_20)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        # using clang with clang-cl front end
    elseif(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
        # using clang with regular front end
    endif()
endif()

# Set a macro for __declspec(dllexport) for PetAI
target_compile_definitions("${PROJECT_NAME}Shared" PRIVATE -DPET_AI_BUILD_SHARED)
target_compile_definitions("${PROJECT_NAME}Static" PRIVATE -DPET_AI_BUILD_STATIC)

if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    # Disable exceptions and ignore some CRT warnings
    target_compile_definitions("${PROJECT_NAME}Shared" PRIVATE -D_CRT_SECURE_NO_WARNINGS -D_HAS_EXCEPTIONS=1)
    target_compile_definitions("${PROJECT_NAME}Static" PRIVATE -D_CRT_SECURE_NO_WARNINGS -D_HAS_EXCEPTIONS=1)

    # Set the runtime library to be dynamic for the shared library and static for the static library. 
    set_target_properties("${PROJECT_NAME}Shared" PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    set_target_properties("${PROJECT_NAME}Static" PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    
    # Set some debug PDB flags.
    target_compile_options("${PROJECT_NAME}Shared" PRIVATE "$<$<NOT:$<CONFIG:Debug>>:/Zi>")
    target_link_options("${PROJECT_NAME}Shared" PRIVATE "$<$<NOT:$<CONFIG:Debug>>:/DEBUG>")
    target_link_options("${PROJECT_NAME}Shared" PRIVATE "$<$<NOT:$<CONFIG:Debug>>:/OPT:REF>")
    target_link_options("${PROJECT_NAME}Shared" PRIVATE "$<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>")
    
    # Set some debug PDB flags.
    target_compile_options("${PROJECT_NAME}Static" PRIVATE "$<$<NOT:$<CONFIG:Debug>>:/Zi>")
    target_link_options("${PROJECT_NAME}Static" PRIVATE "$<$<NOT:$<CONFIG:Debug>>:/DEBUG>")
    target_link_options("${PROJECT_NAME}Static" PRIVATE "$<$<NOT:$<CONFIG:Debug>>:/OPT:REF>")
    target_link_options("${PROJECT_NAME}Static" PRIVATE "$<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>")
endif()

check_c_compiler_flag(/wd5030 HAS_UNRECOGNIZED_ATTRIBUTES_WARNING)
check_c_compiler_flag(/wd4251 HAS_DLL_INTERFACE_WARNING)

if(HAS_UNRECOGNIZED_ATTRIBUTES_WARNING)
    target_compile_options("${PROJECT_NAME}Shared" PRIVATE /wd5030)
endif()

if(HAS_DLL_INTERFACE_WARNING)
    target_compile_options("${PROJECT_NAME}Shared" PRIVATE /wd4251)
endif()

install(
    TARGETS "${PROJECT_NAME}Shared" "${PROJECT_NAME}Static"
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    FILE_SET HEADERS
)
