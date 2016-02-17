file(MAKE_DIRECTORY ${WD}/test_venv)
execute_process(
    COMMAND ${VENV_EXEC} --system-site-packages ${WD}/test_venv
    RESULT_VARIABLE RES
)
if (NOT RES EQUAL 0)
    message(FATAL_ERROR "Creating test virtual environment in ${WD}/venv_tmp failed")
endif()
set(VENV_BINDIR bin/)
if (WIN32)
    set(VENV_BINDIR Scripts/)
endif()
set(VENV_BIN "${WD}/test_venv/${VENV_BINDIR}")
execute_process(
    COMMAND ${VENV_BIN}/pip install --upgrade "${PACKAGE_DIR}"
    RESULT_VARIABLE RES
)
if (NOT RES EQUAL 0)
    message(FATAL_ERROR "Installing bindings package failed")
endif()
execute_process(
    COMMAND ${VENV_BIN}/python -c "from opencmiss.zinc.context import Context"
    RESULT_VARIABLE RES
)
if (NOT RES EQUAL 0)
    message(FATAL_ERROR "Importing opencmiss zinc package in virtual environment failed")
endif()
# Dont forget to clean up!
file(REMOVE_RECURSE ${WD}/test_venv)