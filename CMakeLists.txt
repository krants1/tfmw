cmake_minimum_required(VERSION 3.5)

project(TFMW)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_COTIRE "ENABLE_COTIRE(not MSVC)" ON)
option(ENABLE_PROTOBUF "ENABLE_PROTOBUF(not MINGW)" OFF)
option(ENABLE_GOOGLE_TESTS "ENABLE_GOOGLE_TESTS" OFF)
option(ENABLE_CLANG "ENABLE_CLANG(CYGWIN only)" OFF)
option(ENABLE_CCACHE "ENABLE_CCACHE(not MSVC)" OFF)

if (NOT MSVC AND ENABLE_COTIRE)
    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
    include(cotire)
endif ()

message(STATUS "PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

set(SOURCE_FILES main.cpp)

if (NOT MINGW AND ENABLE_PROTOBUF)
    add_definitions(-D_USE_PROTOBUF_)
    list(APPEND SOURCE_FILES "proto/examples/tcp.cpp")
endif ()

list(APPEND SOURCE_FILES 
		"t/examples/tcp.cpp"				
        "t/examples/log.cpp"
        "t/examples/threads.cpp"
        "t/examples/time_utils.cpp"
        "t/examples/ini_files.cpp"
        "t/examples/packet.cpp"
        "t/examples/counters.cpp")

message(STATUS "SOURCE_FILES: ${SOURCE_FILES}")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
set(PROJECT_APP_SAMPLE TApplication)
add_executable(${PROJECT_APP_SAMPLE} application/main.cpp)

if (CYGWIN AND ENABLE_CLANG)
    set(CMAKE_C_COMPILER "/usr/bin/clang")
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
endif ()
message(STATUS "CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER_VERSION: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "CMAKE_VERSION: ${CMAKE_VERSION}")

if (UNIX)
    SET(CMAKE_CXX_FLAGS -pthread)
endif ()

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
    add_definitions(-D_WIN32_WINNT=0x0501)
else ()
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif ()

if (NOT MSVC AND ENABLE_CCACHE)
    find_program(CCACHE_FOUND ccache)
    if (CCACHE_FOUND)
        message(STATUS "CCACHE_FOUND: ${CCACHE_FOUND}")
        foreach (LANG C CXX)
            if (NOT DEFINED CMAKE_${LANG}_COMPILER_LAUNCHER AND NOT CMAKE_${LANG}_COMPILER MATCHES ".*/ccache$")
                message(STATUS "Enabling ccache for ${LANG}")
                set(CMAKE_${LANG}_COMPILER_LAUNCHER ${CCACHE_FOUND} CACHE STRING "")
            endif ()
            message(STATUS "Enabling ccache for: CMAKE_${LANG}_COMPILER_LAUNCHER")
        endforeach ()
    endif (CCACHE_FOUND)
endif ()

if (ENABLE_PROTOBUF)
    find_package(Protobuf REQUIRED)
    if (PROTOBUF_FOUND)
        message(STATUS "Protobuf_VERSION: ${Protobuf_VERSION}")
        message(STATUS "Protobuf_INCLUDE_DIRS: ${PROTOBUF_INCLUDE_DIRS}")
        message(STATUS "Protobuf_LIBRARIES: ${PROTOBUF_LIBRARIES}")
        message(STATUS "Protobuf_PROTOC_EXECUTABLE: ${Protobuf_PROTOC_EXECUTABLE}")

        target_link_libraries(${PROJECT_NAME} ${PROTOBUF_LIBRARIES})
        include_directories(${PROTOBUF_INCLUDE_DIRS})
    endif ()
endif ()

if (CYGWIN)
endif ()

if (MINGW)
    target_link_libraries(${PROJECT_NAME} ws2_32)
    target_link_libraries(${PROJECT_APP_SAMPLE} ws2_32)

    set(Boost_INCLUDE_DIR "D:/boost/boost_1_64_0_MinGW_/include/boost-1_64")
    set(Boost_LIBRARY "D:/boost/boost_1_64_0_MinGW_/lib")
    set(BOOST_ROOT "D:/boost/boost_1_64_0_MinGW_")
    include_directories(${Boost_INCLUDE_DIR})
endif ()

if (MSVC)
    message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
    message(STATUS "VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")
endif ()

find_package(Boost COMPONENTS signals serialization filesystem thread system REQUIRED)
if (Boost_FOUND)
    message(STATUS "Boost_VERSION: ${Boost_VERSION}")
    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
    target_link_libraries(${PROJECT_APP_SAMPLE} ${Boost_LIBRARIES})
    include_directories(${Boost_INCLUDE_DIR})
    #if (MSVC) include_directories(${Boost_INCLUDE_DIR}) endif ()
endif ()

if (ENABLE_GOOGLE_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    set(PROJECT_TESTS Tests)
	set(TEST_FILES "tests/packet.cpp")
    add_executable(${PROJECT_TESTS} ${TEST_FILES})
    target_link_libraries(${PROJECT_TESTS} PRIVATE GTest::GTest GTest::Main ${Boost_LIBRARIES})
    add_test(${PROJECT_TESTS} ${TEST_FILES})
endif ()

if (NOT MSVC AND ENABLE_COTIRE)
    message(STATUS "ENABLE_COTIRE")
    cotire(${PROJECT_NAME})
    cotire(${PROJECT_APP_SAMPLE})

    # cotire sets the following properties
    get_target_property(_unitySource ${PROJECT_NAME} COTIRE_CXX_UNITY_SOURCE)
    get_target_property(_prefixHeader ${PROJECT_NAME} COTIRE_CXX_PREFIX_HEADER)
    get_target_property(_precompiledHeader ${PROJECT_NAME} COTIRE_CXX_PRECOMPILED_HEADER)
    get_target_property(_unityTargetName ${PROJECT_NAME} COTIRE_UNITY_TARGET_NAME)

    if (_unitySource)
        message(STATUS "${PROJECT_NAME} unity source: ${_unitySource}")
    endif ()
    if (_prefixHeader)
        message(STATUS "${PROJECT_NAME} prefix header: ${_prefixHeader}")
    endif ()
    if (_precompiledHeader)
        message(STATUS "${PROJECT_NAME} precompiled header: ${_precompiledHeader}")
    endif ()
    if (TARGET ${_unityTargetName})
        message(STATUS "${PROJECT_NAME} unity target: ${_unityTargetName}")
    endif ()
endif ()

