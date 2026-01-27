cmake_minimum_required(VERSION 3.23)
project(DeskPet VERSION 0.0.1 LANGUAGES CXX C)

add_subdirectory(SysLib)
add_subdirectory(PetAI)
add_subdirectory(CliPet)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
