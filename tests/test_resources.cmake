include(GenerateExportHeader)

set(TESTS_RESOURCE_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/resources)
set(TESTS_OUTPUT_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/outputs)

file(MAKE_DIRECTORY ${TESTS_OUTPUT_LOCATION})

set(TEST_RESOURCE_HEADER ${CMAKE_CURRENT_BINARY_DIR}/test_resources.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_resources.in.h ${TEST_RESOURCE_HEADER})

set(TEST_EXPORTDEFINITIONS_H "${CMAKE_CURRENT_BINARY_DIR}/test_exportdefinitions.h")

add_library(testresources OBJECT
  ${CMAKE_CURRENT_SOURCE_DIR}/test_resources.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/test_resources.h
  ${TEST_EXPORTDEFINITIONS_H}
)
target_include_directories(testresources PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
)
generate_export_header(testresources EXPORT_FILE_NAME ${TEST_EXPORTDEFINITIONS_H} BASE_NAME TEST)

