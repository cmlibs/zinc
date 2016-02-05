if (NOT PYTHON_BINDINGS_INSTALL_DIR)
    set(PYTHON_BINDINGS_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/python)
endif()
# Install step for virtual environment (if given)
if (PYTHON_BINDINGS_INSTALL_DIR_IS_VIRTUALENV)
    # The binary directories for the python environments are different on windows (for what reason exactly?)
    # So we need different subpaths
    set(VENV_BINDIR bin/)
    if (WIN32)
        set(VENV_BINDIR Scripts/)
    endif()
    # Convention between manage and iron CMake scripts: On multiconfig-environments, the 
    # installation directories have the build type path element inside the PYTHON_BINDINGS_INSTALL_DIR
    if (HAVE_MULTICONFIG_ENV)
        set(VENV_BINDIR $<LOWER_CASE:$<CONFIG>>/${VENV_BINDIR})
    endif()
    # We need a native path to pass to the pip program
    file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>" NATIVE_CMAKE_CURRENT_BINARY_DIR)
    # This target takes care to install the python package generated in the build tree to the specified virtual
    # environment.
    add_custom_target(install_venv
        DEPENDS collect_python_binding_files
        COMMAND ${VENV_BINDIR}/pip install --upgrade "${NATIVE_CMAKE_CURRENT_BINARY_DIR}"
        WORKING_DIRECTORY "${PYTHON_BINDINGS_INSTALL_DIR}"
        COMMENT "Installing: opencmiss.iron package for Python virtual environment ..."
    )
    install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" --build . --target install_venv --config \${CMAKE_INSTALL_CONFIG_NAME} WORKING_DIRECTORY \"${Iron_BINARY_DIR}\")")
       
    # Add python interface tests
    set(PYTHON_TESTS import_tests region_tests
        graphics_tests field_tests logger_tests 
        sceneviewer_tests imageprocessing_tests
    )
    foreach(TESTDIR ${PYTHON_TESTS})
        string(REPLACE "_" "" TESTNAME ${TESTDIR})
        add_test(NAME python_bindings_${TESTNAME}
            COMMAND ${VENV_BINDIR}/python ${CMAKE_CURRENT_SOURCE_DIR}/tests/${TESTDIR}/${TESTNAME}.py
            WORKING_DIRECTORY "${PYTHON_BINDINGS_INSTALL_DIR}"
        )
    endforeach()
    
    # Also add a test to see if simple importing of the libraries works
    add_test(NAME python_bindings_import
        COMMAND ${VENV_BINDIR}/python -c "from opencmiss.zinc.context import Context"
        WORKING_DIRECTORY "${PYTHON_BINDINGS_INSTALL_DIR}")
else()
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/opencmiss
            DESTINATION ${PYTHON_BINDINGS_INSTALL_DIR}
            FILES_MATCHING PATTERN ${ZINC_SHARED_OBJECT_GLOB})        
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/opencmiss
            DESTINATION ${PYTHON_BINDINGS_INSTALL_DIR}
            FILES_MATCHING PATTERN "*.py")            
endif()