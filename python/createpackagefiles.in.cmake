# Write the version of PyZinc into the package file __init__.py
FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${OPENCMISS_PYTHON_PACKAGE}/__init__.py" "
# OpenCMISS Python package initialisation file.
from pkgutil import extend_path
__path__ = extend_path(__path__, __name__)

")
FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${OPENCMISS_PYTHON_PACKAGE}/${ZINC_PYTHON_PACKAGE}/__init__.py" "
# Zinc Python package initialisation file.
__version__ = \"${ZINC_VERSION}${ZINC_DEVELOPER_VERSION}\"

")
