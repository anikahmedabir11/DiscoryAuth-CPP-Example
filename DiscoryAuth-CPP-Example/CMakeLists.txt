cmake_minimum_required(VERSION 3.15)
project(DiscoryAuth_CPP_Example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

add_executable(DiscoryAuth_CPP_Example
    src/main.cpp
    src/discoryauth.hpp
)

if (WIN32)
    target_link_libraries(DiscoryAuth_CPP_Example winhttp)
endif()
