# OpenCMISS-Zinc Library Unit Tests
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SET(CURRENT_TEST spectrum)
LIST(APPEND API_TESTS ${CURRENT_TEST})
SET(${CURRENT_TEST}_SRC
    ${CURRENT_TEST}/spectrum.cpp
    utilities/fileio.cpp
    )

SET(SPECTRUM_DESCRIPTION_JSON_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/spectrum_description.json")
