cmake_minimum_required(VERSION 3.28)
project(ButonTest)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")

find_package(GTest)

#add_library(ButtonLibrary SHARED ../src/Button.c)
#target_include_directories(ButtonLibrary PUBLIC ../src)

include_directories(../src)
set(SOURCE_FILES ../src/Button.c)

add_executable(ButonTest testRunner.cpp ${SOURCE_FILES})
target_link_libraries(ButonTest ${GTEST_LIBRARIES})

#target_link_libraries(ButonTest ButtonLibrary ${GTEST_LIBRARIES})
