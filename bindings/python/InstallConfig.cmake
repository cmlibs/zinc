# Always install to install tree - packaging requires this even if virtualenv is used.
#install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/opencmiss
#        DESTINATION python/$<CONFIG>
#        COMPONENT PythonBindings
#        FILES_MATCHING PATTERN ${ZINC_SHARED_OBJECT_GLOB}
#)


install(TARGETS ${SWIG_MODULE_TARGETS} ${ZINC_SHARED_TARGET}
    DESTINATION ${PYTHON_MODULE_TARGETS_DESTINATION_PREFIX}
    COMPONENT PythonBindings
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/setup.py
    DESTINATION ${PYTHON_DESTINATION_PREFIX}
    COMPONENT PythonBindings
)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/opencmiss
    DESTINATION ${PYTHON_DESTINATION_PREFIX}
    COMPONENT PythonBindings
    FILES_MATCHING PATTERN "*.py"
)
install(FILES "${BASE_PYTHON_PACKAGE_DIR}/README.txt"
    DESTINATION ${PYTHON_DESTINATION_PREFIX}
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

