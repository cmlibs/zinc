
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(Zinc_Tests)

# Unit Testing
ENABLE_TESTING()

# Find dependent packages
FIND_PACKAGE(GTest REQUIRED)
FIND_PACKAGE(Zinc REQUIRED)

INCLUDE_DIRECTORIES(
    ${GTEST_INCLUDE_DIRS}
    ${ZINC_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

SET(API_TESTS)

# Any tests to include must append the test name
# to the API_TESTS list.  Any source files for the
# test must be set to <test name>_SRC.
INCLUDE(fieldmodule/tests.cmake)
INCLUDE(graphic/tests.cmake)
INCLUDE(fieldimage/tests.cmake)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/test_resources.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/test_resources.h)

FOREACH(DEF ${ZINC_DEFINITIONS} ${PLATFORM_DEFS})
        ADD_DEFINITIONS(-D${DEF})
ENDFOREACH()
FOREACH( TEST ${API_TESTS} )
        ADD_EXECUTABLE(APITestEXE${TEST} ${${TEST}_SRC})
        TARGET_LINK_LIBRARIES(APITestEXE${TEST} ${GTEST_MAIN_LIBRARIES} ${GTEST_LIBRARIES} ${ZINC_LIBRARIES})
        ADD_TEST(APITest_${TEST} APITestEXE${TEST})
ENDFOREACH()

