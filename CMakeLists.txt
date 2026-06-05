cmake_minimum_required(VERSION 3.14)

project(slimboy VERSION 1.0.0 LANGUAGES CXX)

add_library(slimboy INTERFACE)

target_include_directories(slimboy
    INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_compile_features(gb_core INTERFACE cxx_std_20)