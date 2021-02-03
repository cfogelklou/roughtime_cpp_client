cmake_minimum_required(VERSION 3.5)
project(roughtime_cpp_test)

set(CMAKE_CXX_STANDARD 11)

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

set(GTEST_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest/src/gtest-all.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googlemock/src/gmock-all.cc
)


file(GLOB_RECURSE LIBSODIUM_CRYPTO_SIGN_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/libsodium/src/libsodium/crypto_sign/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/libsodium/src/libsodium/crypto_sign/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/libsodium/src/libsodium/crypto_core/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/libsodium/src/libsodium/crypto_core/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/libsodium/src/libsodium/crypto_hash/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/libsodium/src/libsodium/crypto_hash/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/libsodium/src/libsodium/crypto_verify/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/libsodium/src/libsodium/crypto_verify/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/libsodium/src/libsodium/sodium/utils.c
)

set(SOURCE_FILES
        ${LOCAL_SRC}

        ${LIBSODIUM_CRYPTO_SIGN_SRC}

        ${GTEST_SRC}
)

list(REMOVE_DUPLICATES SOURCE_FILES)

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

find_package(Threads REQUIRED)

# target_link_libraries(${PROJECT_NAME} curl)
if (WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
else (WIN32)
    target_link_libraries(${PROJECT_NAME} Threads::Threads)
endif (WIN32)

enable_testing()
add_test(${PROJECT_NAME} ${PROJECT_NAME})
