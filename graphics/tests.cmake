# OpenCMISS-Zinc Library Unit Tests
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SET(CURRENT_TEST graphics)
LIST(APPEND API_TESTS ${CURRENT_TEST})
SET(${CURRENT_TEST}_SRC
    ${CURRENT_TEST}/contours.cpp
    ${CURRENT_TEST}/convert_to_finite_elements.cpp
    ${CURRENT_TEST}/font.cpp
    ${CURRENT_TEST}/graphics.cpp
    ${CURRENT_TEST}/light.cpp
    ${CURRENT_TEST}/scene.cpp
    ${CURRENT_TEST}/scenefilter.cpp
    ${CURRENT_TEST}/scenepicker.cpp
    ${CURRENT_TEST}/sceneviewer.cpp
    ${CURRENT_TEST}/streamlines.cpp
    ${CURRENT_TEST}/tessellation.cpp
    )

SET(SCENEVIEWER_DESCRIPTION_JSON_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/sceneviewer_description.json")
SET(TESSELLATION_DESCRIPTION_JSON_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/tessellation_description.json")
