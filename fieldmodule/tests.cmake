# OpenCMISS-Zinc Library Unit Tests
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SET(CURRENT_TEST fieldmodule)
LIST(APPEND API_TESTS ${CURRENT_TEST})
SET(${CURRENT_TEST}_SRC
	${CURRENT_TEST}/create_composite.cpp
	${CURRENT_TEST}/create_derivatives.cpp
	${CURRENT_TEST}/create_if.cpp
	${CURRENT_TEST}/create_nodeset_operators.cpp
	${CURRENT_TEST}/create_vectoroperators.cpp
	${CURRENT_TEST}/region_io.cpp
	${CURRENT_TEST}/create_image_processing.cpp
	${CURRENT_TEST}/create_fibre_axes.cpp
	${CURRENT_TEST}/fieldconstant.cpp
	${CURRENT_TEST}/fieldimage.cpp
	${CURRENT_TEST}/fielditerator.cpp
	${CURRENT_TEST}/fieldlogicaloperators.cpp
	${CURRENT_TEST}/fieldmeshoperators.cpp
	${CURRENT_TEST}/fieldmodulenotifier.cpp
	${CURRENT_TEST}/finiteelement.cpp
	${CURRENT_TEST}/nodesandelements.cpp
	${CURRENT_TEST}/timesequence.cpp
	)

SET(FIELDMODULE_EXNODE_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/nodes.exnode")
SET(FIELDMODULE_CUBE_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/cube.exformat")
SET(FIELDMODULE_CUBE_GRID_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/cube_grid.exelem")
SET(FIELDMODULE_REGION_INPUT_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/region_input.exregion")
SET(FIELDMODULE_EMBEDDING_ISSUE3614_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/embedding_issue3614.exregion")
SET(FIELDIMAGE_BLOCKCOLOURS_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/blockcolours.png")
SET(FIELDMODULE_TWO_CUBES_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/two_cubes.exformat")
SET(HEART_EXNODE_GZ "${CMAKE_CURRENT_LIST_DIR}/heart.exnode.gz")
SET(HEART_EXELEM_GZ "${CMAKE_CURRENT_LIST_DIR}/heart.exelem.gz")
SET(FIELDMODULE_ALLSHAPES_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/allshapes.exformat")
SET(FIELDMODULE_CUBE_XYZP_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/cube_xyzp.exformat")
