cmake_minimum_required(VERSION 3.23)
project(Win32CliPet VERSION 0.0.1 LANGUAGES CXX C)

include(CheckCCompilerFlag)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c")
file(GLOB SOURCES_BASE "src/*.cpp" "src/*.c")
file(GLOB_RECURSE HEADERS "include/*.hpp" "include/*.h" "include/*.inl")
file(GLOB HEADERS_BASE "include/*.hpp" "include/*.h" "include/*.inl")

set(WIN32CLIPET_SOURCE_FILES ${SOURCES_BASE} ${HEADERS_BASE})

# Setup the Dynamic and Static libraries with the source files.
add_executable(${PROJECT_NAME} ${WIN32CLIPET_SOURCE_FILES})

foreach(_source IN ITEMS ${HEADERS})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "/" "\\" _source_dir_corrected "${CMAKE_SOURCE_DIR}")
    string(REPLACE "/" "\\" _source_path "${_source_path}")
    string(REPLACE "${_source_dir_corrected}\\${PROJECT_NAME}" "" _group_path "${_source_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

foreach(_source IN ITEMS ${SOURCES})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "/" "\\" _source_dir_corrected "${CMAKE_SOURCE_DIR}")
    string(REPLACE "/" "\\" _source_path "${_source_path}")
    string(REPLACE "${_source_dir_corrected}\\${PROJECT_NAME}" "" _group_path "${_source_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

target_sources(${PROJECT_NAME} PUBLIC FILE_SET "HEADERS" BASE_DIRS "include" FILES ${HEADERS})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SysLib> $<TARGET_FILE_DIR:${PROJECT_NAME}>)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:PetAIShared> $<TARGET_FILE_DIR:${PROJECT_NAME}>)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_LINKER_FILE:PetAIShared> $<TARGET_FILE_DIR:${PROJECT_NAME}>)

target_link_libraries(${PROJECT_NAME} SysLib PetAIShared)

# Set the include directory.
target_include_directories(${PROJECT_NAME} PUBLIC include)
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

target_compile_definitions(${PROJECT_NAME} PRIVATE -DPET_AI_IMPORT_SHARED)

if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    # Disable exceptions and ignore some CRT warnings
    target_compile_definitions(${PROJECT_NAME} PRIVATE -D_CRT_SECURE_NO_WARNINGS -D_HAS_EXCEPTIONS=1)

    set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    
    target_compile_options(${PROJECT_NAME} PRIVATE "$<$<NOT:$<CONFIG:Debug>>:/Zi>")
    target_link_options(${PROJECT_NAME} PRIVATE "$<$<NOT:$<CONFIG:Debug>>:/DEBUG>")
    target_link_options(${PROJECT_NAME} PRIVATE "$<$<NOT:$<CONFIG:Debug>>:/OPT:REF>")
    target_link_options(${PROJECT_NAME} PRIVATE "$<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>")
endif()

check_c_compiler_flag(/wd5030 HAS_UNRECOGNIZED_ATTRIBUTES_WARNING)
check_c_compiler_flag(/wd4251 HAS_DLL_INTERFACE_WARNING)

if(HAS_UNRECOGNIZED_ATTRIBUTES_WARNING)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd5030)
endif()

if(HAS_DLL_INTERFACE_WARNING)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4251)
endif()

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    FILE_SET HEADERS
)
