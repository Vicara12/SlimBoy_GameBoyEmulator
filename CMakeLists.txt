cmake_minimum_required(VERSION 3.14)

project(gb_core VERSION 1.0.0 LANGUAGES CXX)

add_library(gb_core INTERFACE)

target_include_directories(gb_core
    INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_compile_features(gb_core INTERFACE cxx_std_20)