cmake_minimum_required(VERSION 3.21)

project(lirs_cache
    VERSION 0.1.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(lirs_cache)

add_subdirectory(cmake/flags)
add_subdirectory(cmake/third_party)

target_sources(lirs_cache
    PRIVATE
        source/main.cpp
        source/lirs_cache.cpp
)

target_link_libraries(lirs_cache
    PRIVATE
        flags
)