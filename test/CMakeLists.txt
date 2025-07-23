# Tests
add_executable(CmdArgParserTest
    CmdArgParserTest.cpp
    )
target_include_directories(CmdArgParserTest PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(CmdArgParserTest PRIVATE gtest_main gtest)
add_test(NAME CmdArgParserTest COMMAND CmdArgParserTest)
