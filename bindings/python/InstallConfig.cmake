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
    COMPONENT PythonBindings
)

# Install step for virtual environment (if given)
if (ZINC_USE_VIRTUALENV)
    # Variables required for OCPythonBindingsVirtualEnv.
    set(VIRTUALENV_INSTALL_PREFIX ${ZINC_VIRTUALENV_INSTALL_PREFIX})
    set(PYTHON_PACKAGE_CURRENT_NAME zinc)
    # Virtual environment creation target.
    include(OCPythonBindingsVirtualEnv)
endif ()

