message(STATUS "==========================")
message(STATUS "SPHINX_FOUND: ${SPHINX_FOUND}")
message(STATUS "Sphinx_FOUND: ${Sphinx_FOUND}")
if(SPHINX_FOUND)
  set(SPHINX_DOCS_TARGET project_docs)

  if(NOT DEFINED SPHINX_THEME)
    set(SPHINX_THEME theme)
  endif()

  if(NOT DEFINED SPHINX_THEME_DIR)
    set(SPHINX_THEME_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
  endif()

  string(TIMESTAMP SPHINX_YEAR "%Y")

  # configured documentation tools and intermediate build results
  set(BINARY_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/_build")

  # Sphinx cache with pickled ReST documents
  set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")

  # HTML output directory
  set(SPHINX_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/html")

  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/conf.in.py"
    "${BINARY_BUILD_DIR}/conf.py"
    @ONLY)

  add_custom_target(${SPHINX_DOCS_TARGET}
    COMMAND ${SPHINX_EXECUTABLE}
        -q -b html
        -c "${BINARY_BUILD_DIR}"
        -d "${SPHINX_CACHE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${SPHINX_HTML_DIR}"
    COMMENT "Building HTML documentation with Sphinx")

  add_custom_target(serve_docs
    DEPENDS ${SPHINX_DOCS_TARGET}
    COMMAND ${Python_EXECUTABLE} -m http.server
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
    COMMENT "Serving documentation.")
endif()

if(SPHINX_FOUND)
  add_custom_target(docs
    DEPENDS ${SPHINX_DOCS_TARGET}
    COMMENT "Generating documentation")
  add_custom_target(clean_docs
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/html
    COMMENT "Cleaning documentation")
endif()

