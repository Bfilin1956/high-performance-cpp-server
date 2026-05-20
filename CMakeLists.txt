cmake_minimum_required(VERSION 3.12)
project(atta1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "src/*.cpp"
        "src/*.h"
        "include/*.h"
        "include/*.hpp"
)

add_executable(atta1 ${SOURCES}
        include/attaServer/protocol/ProtocolHeader.h
        include/attaServer/PoolAllocator.h)
target_include_directories(atta1 PRIVATE include)

find_package(Boost REQUIRED)
find_package(fmt CONFIG REQUIRED)
target_link_libraries(atta1 PRIVATE Boost::headers fmt::fmt)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()