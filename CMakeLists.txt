# OpenCMISS-Zinc Library Unit Tests
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	)

SET(API_TESTS)
SET(TESTIMAGE_GRAY_JPG_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources/testimage_gray.jpg")

# Any tests to include must append the test name
# to the API_TESTS list.  Any source files for the
# test must be set to <test name>_SRC.
INCLUDE(fieldmodule/tests.cmake)
INCLUDE(glyph/tests.cmake)
INCLUDE(graphics/tests.cmake)
INCLUDE(material/tests.cmake)
INCLUDE(optimisation/tests.cmake)
INCLUDE(region/tests.cmake)
INCLUDE(selection/tests.cmake)
INCLUDE(spectrum/tests.cmake)
INCLUDE(time/tests.cmake)
INCLUDE(image/tests.cmake)

SET(TEST_RESOURCE_HEADER ${CMAKE_CURRENT_BINARY_DIR}/test_resources.h)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/test_resources.h.cmake
	${TEST_RESOURCE_HEADER})

FOREACH(DEF ${ZINC_DEFINITIONS} ${PLATFORM_DEFS})
	ADD_DEFINITIONS(-D${DEF})
ENDFOREACH()
FOREACH( TEST ${API_TESTS} )
	SET( CURRENT_TEST APITest_${TEST} )
	ADD_EXECUTABLE(${CURRENT_TEST} ${${TEST}_SRC} ${TEST_RESOURCE_HEADER})
	TARGET_LINK_LIBRARIES(${CURRENT_TEST} ${GTEST_MAIN_LIBRARIES} ${GTEST_LIBRARIES} ${ZINC_LIBRARIES})
	ADD_TEST(${CURRENT_TEST} ${CURRENT_TEST})
ENDFOREACH()

