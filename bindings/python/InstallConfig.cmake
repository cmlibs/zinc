# Install step for virtual environment (if given)
if (EXISTS "${INSTALL_TO_VIRTUALENV}")
    # The binary directories for the python environments are different on windows (for what reason exactly?)
    # So we need different subpaths
    set(VENV_BINDIR bin)
    if (WIN32)
        set(VENV_BINDIR Scripts)
    endif()
    # Convention between manage and iron CMake scripts: On multiconfig-environments, the 
    # installation directories have the build type path element inside the INSTALL_TO_VIRTUALENV
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
        WORKING_DIRECTORY "${INSTALL_TO_VIRTUALENV}"
        COMMENT "Installing: opencmiss.iron package for Python virtual environment ..."
    )
    install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" --build . --target install_venv --config \${CMAKE_INSTALL_CONFIG_NAME} WORKING_DIRECTORY \"${Iron_BINARY_DIR}\")"
        COMPONENT VirtualEnv)
endif()

# Always install to install tree - packaging requires this even if virtualenv is used.
#install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/opencmiss
#        DESTINATION python/$<CONFIG>
#        COMPONENT PythonBindings
#        FILES_MATCHING PATTERN ${ZINC_SHARED_OBJECT_GLOB}
#)
install(TARGETS ${SWIG_MODULE_TARGETS}
    DESTINATION python/$<CONFIG>/opencmiss/zinc
    COMPONENT PythonBindings
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/setup.py
        DESTINATION python/$<CONFIG>
        COMPONENT PythonBindings
)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/opencmiss
        DESTINATION python/$<CONFIG>
        COMPONENT PythonBindings
        FILES_MATCHING PATTERN "*.py"
)
install(FILES "${BASE_PYTHON_PACKAGE_DIR}/README.txt"
    DESTINATION python/$<CONFIG> 
    COMPONENT PythonBindings)

