cmake_minimum_required(VERSION 3.22)
project(notifier_cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

set(EIGEN3_INCLUDE_DIR "libs/eigen-3.4.0")
include_directories(${EIGEN3_INCLUDE_DIR})

set(JSON_INCLUDE_DIR "libs/json-3.11.3")
include_directories(${JSON_INCLUDE_DIR})

set(CATCH2_INCLUDE_DIR "libs/catch2-3.11.0")
include_directories(${CATCH2_INCLUDE_DIR})

set(BIUS_BASE_SOURCES
        src/printer.h
        src/tools.h
        src/tools.cpp
        src/notifier.h
        src/notifier.cpp
)

set(BIUS_APP_SOURCES
        src/app.h
        src/app.cpp
)

set(BIUS_TEST_SOURCES
        tests/sample_test.cpp
)

add_executable(notifier_cpp
        main.cpp

        ${BIUS_BASE_SOURCES}
        ${BIUS_APP_SOURCES}
)

add_executable(notifier_tests
        libs/catch2-3.11.0/catch_amalgamated.hpp
        libs/catch2-3.11.0/catch_amalgamated.cpp

        ${BIUS_BASE_SOURCES}
        ${BIUS_APP_SOURCES}
        ${BIUS_TEST_SOURCES}
)
