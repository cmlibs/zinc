# OpenCMISS-Zinc Library Unit Tests
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
# Find dependent packages
find_package(GTest ${GTEST_VERSION} REQUIRED)

list(INSERT CMAKE_MODULE_PATH  0 "${CMAKE_CURRENT_SOURCE_DIR}")
# Test for pthread requirement, and OS X 10.9
include(GTestChecks)

set(API_TESTS)

# Any tests to include must append the test name
# to the API_TESTS list.  Any source files for the
# test must be set to <test name>_SRC.
include(context/tests.cmake)
include(fieldio/tests.cmake)
include(fieldmodule/tests.cmake)
include(glyph/tests.cmake)
include(graphics/tests.cmake)
include(material/tests.cmake)
include(optimisation/tests.cmake)
include(region/tests.cmake)
include(selection/tests.cmake)
include(spectrum/tests.cmake)
include(time/tests.cmake)
include(image/tests.cmake)
include(logger/tests.cmake)

set(TEST_RESOURCE_HEADER ${CMAKE_CURRENT_BINARY_DIR}/test_resources.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_resources.h.cmake
	${TEST_RESOURCE_HEADER})

foreach(DEF ${ZINC_DEFINITIONS} ${PLATFORM_DEFS})
	add_definitions(-D${DEF})
endforeach()

get_library_path(PATH_DEF "${Zinc_BINARY_DIR}/core")
foreach( TEST ${API_TESTS} )
	set( CURRENT_TEST APITest_${TEST} )
	add_executable(${CURRENT_TEST} ${${TEST}_SRC} ${TEST_RESOURCE_HEADER})
	target_link_libraries(${CURRENT_TEST} gtest_main zinc)
	target_include_directories(${CURRENT_TEST} PRIVATE 
	    ${ZINC_API_INCLUDE_DIR} 
	    ${CMAKE_CURRENT_SOURCE_DIR} 
	    ${CMAKE_CURRENT_BINARY_DIR}
	)
	add_test(NAME ${CURRENT_TEST} COMMAND ${CURRENT_TEST})
	set_tests_properties(${CURRENT_TEST} PROPERTIES
		TIMEOUT 30
		ENVIRONMENT "${PATH_DEF}"
	)
endforeach()

