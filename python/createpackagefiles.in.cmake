# Write the version of PyZinc into the package file __init__.py
FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${OPENCMISS_PYTHON_MODULE}/__init__.py" "# OpenCMISS Python package initialisation file.\n")
FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${ZINC_PYTHON_MODULE}/__init__.py" "# Zinc Python package initialisation file.\n\n__version__ = \"${ZINC_VERSION}\"\n\n")
