find_program(Sphinx_EXECUTABLE NAMES ${SPHINX_PREFERRED_NAMES} sphinx-build sphinx-build2
    HINTS
    $ENV{SPHINX_DIR}
    PATH_SUFFIXES bin Scripts
    DOC "Sphinx documentation generator"
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Sphinx DEFAULT_MSG
    Sphinx_EXECUTABLE
)

mark_as_advanced(Sphinx_EXECUTABLE)
