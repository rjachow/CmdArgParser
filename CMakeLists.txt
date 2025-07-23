cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(CmdArgParser CXX)

enable_testing()

add_executable(CmdArgParser
    main.cpp
    src/CmdArgParser.hpp
    )

add_subdirectory(test)
add_subdirectory(lib/googletest)
