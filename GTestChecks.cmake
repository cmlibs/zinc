
INCLUDE(CheckCXXSourceCompiles)

SET(gtest_default "

#include <gtest/gtest.h>

TEST(gtest_test, defualt)
{
    EXPECT_EQ(0, 1);
}

"
)


CHECK_CXX_SOURCE_COMPILES("${gtest_default}" GTEST_SUCCESS)

IF(NOT GTEST_SUCCESS)
    # Try with thread library
    FIND_PACKAGE(Threads)
    SET(CMAKE_REQUIRED_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
    CHECK_CXX_SOURCE_COMPILES("${gtest_default}" GTEST_SUCCESS)
    SET(GTEST_THREAD_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
ENDIF()

