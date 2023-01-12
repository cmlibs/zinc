# OpenCMISS-Zinc Library Unit Tests
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
# Find dependent packages
find_package(GoogleTest ${GTEST_VERSION} REQUIRED)
include(GoogleTest)

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

foreach(DEF ${ZINC_DEFINITIONS} ${PLATFORM_DEFS})
	add_definitions(-D${DEF})
endforeach()

include(test_resources)

set(_ZINC_LINK_LIBRARY ${ZINC_SHARED_TARGET_NAME})
if(NOT ZINC_SHARED_TARGET_NAME AND TARGET ${ZINC_STATIC_TARGET_NAME})
  set(_ZINC_LINK_LIBRARY ${ZINC_STATIC_TARGET_NAME})
endif()

set(TEST_LIST)
foreach( TEST ${API_TESTS} )
	set( CURRENT_TEST APITest_${TEST} )
	add_executable(${CURRENT_TEST} ${${TEST}_SRC} ${TEST_RESOURCE_HEADER})
    target_link_libraries(${CURRENT_TEST} GTest::gtest_main ${_ZINC_LINK_LIBRARY} testresources)
	target_include_directories(${CURRENT_TEST} PRIVATE 
	    ${ZINC_API_INCLUDE_DIR} 
	    ${CMAKE_CURRENT_SOURCE_DIR} 
	    ${CMAKE_CURRENT_BINARY_DIR}
	)
	add_test(NAME TEST_${CURRENT_TEST} COMMAND ${CURRENT_TEST})
  if(DEFINED ZINC_SHARED_TARGET_NAME AND TARGET ${ZINC_SHARED_TARGET_NAME} AND MSVC)
    set(_TEST_PROPERTIES "PATH=$<TARGET_FILE_DIR:${ZINC_SHARED_TARGET_NAME}>")
    set_tests_properties(TEST_${CURRENT_TEST} PROPERTIES ENVIRONMENT "${_TEST_PROPERTIES}")
    #gtest_discover_tests(${CURRENT_TEST} PROPERTIES ENVIRONMENT "${_TEST_PROPERTIES}")
    add_custom_command(TARGET ${CURRENT_TEST} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${ZINC_SHARED_TARGET_NAME}> $<$<CONFIG:Debug,RelWithDebInfo>:$<TARGET_PDB_FILE:${ZINC_SHARED_TARGET_NAME}>> $<TARGET_FILE_DIR:${CURRENT_TEST}>
      COMMENT "Conditionally copy shared libraries to test directory.")
  endif()

  list(APPEND TEST_LIST ${CURRENT_TEST})
endforeach()

if(IS_MULTI_CONFIG_ENV)
  foreach(_TEST ${TEST_LIST})
    string(TOLOWER ${_TEST} _TEST_LOWERCASE)
    add_custom_target(run_${_TEST_LOWERCASE}
      COMMAND ${CMAKE_CTEST_COMMAND} -R TEST_{_TEST} --output-on-failure
      DEPENDS ${_TEST}
      COMMENT "Run test ${_TEST}.")
    set_target_properties(run_${_TEST_LOWERCASE} PROPERTIES FOLDER RunIndividualTests)
  endforeach()
endif()

if(ZINC_MEMCHECK_WITH_VALGRIND)
  add_custom_target(prepare_memcheck
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/valgrind_memcheck
    COMMENT "Prepare for valgrind memcheck tests")

  add_custom_target(valgrind_memcheck
    COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/valgrindmemcheck.py ${CMAKE_CURRENT_BINARY_DIR} ${TEST_LIST}
    DEPENDS ${TEST_LIST} prepare_memcheck
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/valgrind_memcheck
    COMMENT "Running memcheck tests")
endif()

if(ZINC_LLVM_COVERAGE)
  set(RAW_PROFILE_FILENAME "${CMAKE_CURRENT_BINARY_DIR}/zinc-%m.profraw")
  set(RAW_GATHER_PROFILE_FILENAME "${CMAKE_CURRENT_BINARY_DIR}/zinc-*.profraw")
  set(DATA_PROFILE_FILENAME "${CMAKE_CURRENT_BINARY_DIR}/zinc.profdata")

  unset(_CMD_STRING)
  foreach(_TEST ${TEST_LIST})
    list(APPEND _CMD_STRING COMMAND LLVM_PROFILE_FILE=${RAW_PROFILE_FILENAME} ./${_TEST} > /dev/null 2> /dev/null)
  endforeach()

  # Clean away old profile files
  add_custom_target(clean_llvm_coverage
    COMMAND ${FIND_EXE} ${CMAKE_CURRENT_BINARY_DIR} -type f -name '*.profraw' -print | xargs /bin/rm -f
    COMMAND ${FIND_EXE} ${CMAKE_CURRENT_BINARY_DIR} -type f -name '*.profdata' -print | xargs /bin/rm -f
    COMMENT "Clean old LLVM coverage data")

  add_custom_target(prepare_llvm_coverage
    ${_CMD_STRING}
    COMMAND ${LLVM_PROFDATA_EXE} merge -sparse ${RAW_GATHER_PROFILE_FILENAME} -o ${DATA_PROFILE_FILENAME}
    DEPENDS clean_llvm_coverage ${TEST_LIST}
    COMMENT "Prepare for LLVM coverage tests")

  # Unfortunately we cannot get an output in html format at this time as it is not supported at version 8.
  add_custom_target(llvm_coverage_terminal
    COMMAND ${LLVM_COV_EXE} report $<TARGET_FILE:zinc> -instr-profile=${DATA_PROFILE_FILENAME} -format=text -use-color
    DEPENDS prepare_llvm_coverage
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Report LLVM coverage results in the terminal")

  add_custom_target(llvm_coverage_html
    COMMAND ${LLVM_COV_EXE} show $<TARGET_FILE:zinc> -instr-profile=${DATA_PROFILE_FILENAME} -format=html -Xdemangler c++filt -Xdemangler -n -o coverage_report --show-expansions --show-branches=count
    DEPENDS prepare_llvm_coverage
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Report LLVM coverage results in HTML")
endif()
