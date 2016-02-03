message(STATUS "Installing: opencmiss.zinc package for Python virtual environment...")
set(PIP_EXEC bin/pip)
if (WIN32)
    set(PIP_EXEC Scripts/pip.exe)
endif()
execute_process(
    COMMAND ${PIP_EXEC} install --upgrade "@CMAKE_CURRENT_BINARY_DIR@"
    WORKING_DIRECTORY "@PYTHON_BINDINGS_INSTALL_DIR@"
    RESULT_VARIABLE _RES
)    
#if (NOT _RES EQUAL 0)
#    message(FATAL_ERROR "Installing python bindings into virtual environment at @PYTHON_BINDINGS_INSTALL_DIR@ failed\n 
#        Error code: ${_RES}:
#        Output: ${_OUT}
#        Error: ${_ERR}")
#endif()