cmake_minimum_required(VERSION 3.23)

find_package(GTest REQUIRED)

add_executable(SysLibTests SysLibTests.cpp)
target_link_libraries(SysLibTests PRIVATE SysLib GTest::gtest_main)
target_include_directories(SysLibTests PRIVATE
    $<TARGET_PROPERTY:SysLib,INTERFACE_INCLUDE_DIRECTORIES>
)

add_test(NAME SysLibTests COMMAND SysLibTests)
