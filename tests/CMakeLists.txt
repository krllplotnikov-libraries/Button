cmake_minimum_required(VERSION 3.16)
project(ButtonTest C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(MicroTest 1.0 REQUIRED)

add_executable(TestButton
	TestButton.c
	../src/Button.c
)

target_include_directories(TestButton PRIVATE
	../src
)

target_compile_options(TestButton PRIVATE -Wall -Wextra --coverage)
target_link_options(TestButton PRIVATE --coverage)
target_link_libraries(TestButton PRIVATE MicroTest::MicroTest m)
