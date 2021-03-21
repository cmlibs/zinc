
# OpenCMISS-Zinc Library
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(ZINC_BUILD_PYTHON_BINDINGS)
    find_package(Python COMPONENTS Interpreter Development)

    if(Python_Interpreter_FOUND AND Python_Development_FOUND)
        add_subdirectory(python)
    else()
        message(WARNING "Python bindings requested but a Python interpreter and library were not found.")
    endif()
endif()

