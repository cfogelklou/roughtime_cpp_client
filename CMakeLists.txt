##  Environment identifers
# From: https://cmake.org/Wiki/CMake_Useful_Variables
# UNIX
#   is TRUE on all UNIX-like OS's, including Apple OS X and CygWin
# WIN32
#   is TRUE on Windows. Prior to 2.8.4 this included CygWin
# APPLE
#   is TRUE on Apple systems. Note this does not imply the system is Mac OS X,
#   only that __APPLE__ is #defined in C/C++ header files. Obtain more specific
#   system information via CMAKE_SYSTEM_VERSION,
#   i.e. IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin"), then it's Mac OS X.
# MINGW
#   is TRUE when using the MinGW compiler in Windows
# MSYS
#   is TRUE when using the MSYS developer environment in Windows
# CYGWIN
#   is TRUE on Windows when using the CygWin version of cmake
# MSVC, MSVC_IDE, MSVC60, MSVC70, MSVC71, MSVC80, CMAKE_COMPILER_2005, MSVC90, MSVC10 (Visual Studio 2010)
#   Microsoft compiler

cmake_minimum_required(VERSION 3.5)
project(roughtime_cpp_test)

set(CMAKE_CXX_STANDARD 11)

add_definitions(-DSODIUM_STATIC -DDEBUG -D_CONSOLE)

file(GLOB LOCAL_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/test/*.hpp
)

set(GTEST_SRC
        ${POLESTAR_PAK_DIR}/googletest/googletest/src/gtest-all.cc
        ${POLESTAR_PAK_DIR}/googletest/googlemock/src/gmock-all.cc
)


set(SOURCE_FILES
        ${LOCAL_SRC}

        ${GTEST_SRC}
)

include_directories(
       .
       ${CMAKE_CURRENT_SOURCE_DIR}/libsodium/src/libsodium/include/sodium
       # Google test (the real thing)
       ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest
       ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest/include
       ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googlemock
       ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googlemock/include
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

if (WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
else (WIN32)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} Threads::Threads)
endif (WIN32)

enable_testing()
add_test(${PROJECT_NAME} ${PROJECT_NAME})
