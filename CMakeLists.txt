cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(banking)

add_library(banking STATIC banking/Account.cpp banking/Transaction.cpp)
target_include_directories(banking PUBLIC banking)

option(BUILD_TESTS "Build tests" OFF)
option(ENABLE_COVERAGE "Enable coverage" OFF)

if(ENABLE_COVERAGE)
    target_compile_options(banking PRIVATE --coverage -O0)
    target_link_options(banking PRIVATE --coverage)
endif()

if(BUILD_TESTS)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(check tests/test.cpp)
    target_link_libraries(check banking gtest_main gmock_main)
    add_test(NAME check COMMAND check)

    if(ENABLE_COVERAGE)
        target_compile_options(check PRIVATE --coverage -O0)
        target_link_options(check PRIVATE --coverage)
    endif()
endif()
