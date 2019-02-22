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
        ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest/src/gtest-all.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googlemock/src/gmock-all.cc
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

target_link_libraries(${PROJECT_NAME} curl)
if (WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
else (WIN32)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} Threads::Threads)
endif (WIN32)

enable_testing()
add_test(${PROJECT_NAME} ${PROJECT_NAME})
