# OpenCMISS-Zinc Library Unit Tests
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SET(CURRENT_TEST fieldio)
LIST(APPEND API_TESTS ${CURRENT_TEST})
SET(${CURRENT_TEST}_SRC
	${CURRENT_TEST}/fieldml_input.cpp
	)

SET(FIELDIO_FIELDML_CUBE_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/cube.fieldml")
SET(FIELDIO_FIELDML_TETMESH_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/tetmesh.fieldml")
SET(FIELDIO_FIELDML_WHEEL_DIRECT_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/wheel_direct.fieldml")
