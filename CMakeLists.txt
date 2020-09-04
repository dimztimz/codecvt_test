cmake_minimum_required(VERSION 3.5)

project(codecvt_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(codecvt_test codecvt.cpp)
