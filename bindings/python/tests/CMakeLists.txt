set(PYTHONTESTS )

# Add python interface tests
set(PYTHON_TESTDIRS import_tests region_tests
    graphics_tests field_tests logger_tests 
    sceneviewer_tests imageprocessing_tests
)


foreach(TESTDIR ${PYTHON_TESTDIRS})
    string(REPLACE "_" "" TESTNAME ${TESTDIR})
    add_test(NAME Python_Bindings_${TESTNAME}
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${TESTDIR}/${TESTNAME}.py
    )
    list(APPEND PYTHONTESTS Python_Bindings_${TESTNAME})
    
endforeach()

# Also add a test to see if simple importing of the libraries works
add_test(NAME Python_Bindings_Import
    COMMAND ${PYTHON_EXECUTABLE} -c "from opencmiss.zinc.context import Context"
)
list(APPEND PYTHONTESTS Python_Bindings_Import)

# Common test settings
GET_LD_PATH(LD_PATH "${Zinc_BINARY_DIR}/core")
# Set the correct python path to have it find the opencmiss package
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/../$<CONFIG>" NATIVE_PYTHON_BINDINGS_DIR)
foreach(_TEST ${PYTHONTESTS})
    set_tests_properties(${_TEST} PROPERTIES
        TIMEOUT 30
        ENVIRONMENT "PYTHONPATH=${NATIVE_PYTHON_BINDINGS_DIR};${LD_VARNAME}=${LD_PATH}"
    )
endforeach()
    