cmake_minimum_required(VERSION 3.5)
project(roughtime_cpp_test)

set(CMAKE_CXX_STANDARD 11)

set(SOP_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sop)
include(${SOP_TOP_DIR}/sop_src/LibraryFiles.cmake)

add_definitions(-DSODIUM_STATIC -DDEBUG -D_CONSOLE -DNATIVE_LITTLE_ENDIAN)
#add_definitions(-DED25519_COMPAT)
if (WIN32)
elseif (APPLE)
    set(CMAKE_BUILD_TYPE Debug)
elseif (UNIX)
    set(CMAKE_BUILD_TYPE Debug)
endif ()

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

file(GLOB_RECURSE LIBSODIUM_CRYPTO_SIGN_SRC
  ${SOP_TOP_DIR}/ext/libsodium/src/libsodium/crypto_sign/*.c
  ${SOP_TOP_DIR}/ext/libsodium/src/libsodium/crypto_sign/*.h
  ${SOP_TOP_DIR}/ext/libsodium/src/libsodium/crypto_core/*.c
  ${SOP_TOP_DIR}/ext/libsodium/src/libsodium/crypto_core/*.h
  ${SOP_TOP_DIR}/ext/libsodium/src/libsodium/crypto_hash/*.c
  ${SOP_TOP_DIR}/ext/libsodium/src/libsodium/crypto_hash/*.h
  ${SOP_TOP_DIR}/ext/libsodium/src/libsodium/crypto_verify/*.c
  ${SOP_TOP_DIR}/ext/libsodium/src/libsodium/crypto_verify/*.h
  ${SOP_TOP_DIR}/ext/libsodium/src/libsodium/sodium/utils.c
)

set(SOURCE_FILES
        ${LOCAL_SRC}

        ${LIBSODIUM_SRC}

        ${SOP_SRC}

        ${GTEST_SRC}
)

list(REMOVE_DUPLICATES SOURCE_FILES)

include_directories(
       .
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

find_package(Threads REQUIRED)

# target_link_libraries(${PROJECT_NAME} curl)
if (WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
else (WIN32)
    target_link_libraries(${PROJECT_NAME} Threads::Threads)
endif (WIN32)

enable_testing()
add_test(${PROJECT_NAME} ${PROJECT_NAME})
