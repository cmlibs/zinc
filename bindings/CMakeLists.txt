
# OpenCMISS-Zinc Library
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


find_package(PythonLibs QUIET)
find_package(PythonInterp QUIET)

if(PYTHONLIBS_FOUND AND PYTHONINTERP_FOUND)
    add_subdirectory(python)
endif()

