cmake_minimum_required(VERSION 3.5)

project(codecvt_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(codecvt_test codecvt.cpp)
if (MSVC)
	target_compile_options(codecvt_test PRIVATE "/utf-8")
endif()
