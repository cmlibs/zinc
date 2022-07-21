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
	${CURRENT_TEST}/elementbasis.cpp
	${CURRENT_TEST}/fieldapply.cpp
	${CURRENT_TEST}/fieldarithmeticoperators.cpp
	${CURRENT_TEST}/fieldassignment.cpp
	${CURRENT_TEST}/fieldconditional.cpp
	${CURRENT_TEST}/fieldconstant.cpp
	${CURRENT_TEST}/fieldimage.cpp
	${CURRENT_TEST}/fielditerator.cpp
	${CURRENT_TEST}/fieldlogicaloperators.cpp
	${CURRENT_TEST}/fieldmeshoperators.cpp
	${CURRENT_TEST}/fieldmoduledescription.cpp
	${CURRENT_TEST}/fieldmodulenotifier.cpp
	${CURRENT_TEST}/fieldparameterstests.cpp
	${CURRENT_TEST}/field_operator_derivatives.cpp
	${CURRENT_TEST}/fieldrange.cpp
	${CURRENT_TEST}/fieldsmoothing.cpp
	${CURRENT_TEST}/fieldtests.cpp
	${CURRENT_TEST}/finiteelement.cpp
	${CURRENT_TEST}/nodesandelements.cpp
	${CURRENT_TEST}/numerical_operators.cpp
	${CURRENT_TEST}/timesequence.cpp
	utilities/fileio.cpp
	)

SET(FIELDMODULE_EXNODE_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/nodes.exnode")
SET(FIELDMODULE_CUBE_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/cube.exformat")
SET(FIELDMODULE_CUBE_GRID_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/cube_grid.exelem")
SET(FIELDMODULE_CUBE_TRICUBIC_DEFORMED_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/cube_tricubic_deformed.exfile")
SET(FIELDMODULE_REGION_INPUT_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/data/region_input.exf")
SET(FIELDMODULE_EMBEDDING_ISSUE3614_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/embedding_issue3614.exregion")
SET(FIELDIMAGE_BLOCKCOLOURS_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/blockcolours.png")
SET(FIELDMODULE_TWO_CUBES_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/two_cubes.exformat")
SET(HEART_EXNODE_GZ "${CMAKE_CURRENT_LIST_DIR}/heart.exnode.gz")
SET(HEART_EXELEM_GZ "${CMAKE_CURRENT_LIST_DIR}/heart.exelem.gz")
SET(FIELDMODULE_ALLSHAPES_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/allshapes.exformat")
SET(FIELDMODULE_CUBE_XYZP_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/cube_xyzp.exformat")
SET(FIELDMODULE_CUBESQUARELINE_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/cubesquareline.exformat")
SET(TESTIMAGE_GRAY_JPG_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources/testimage_gray.jpg")
SET(FIELDMODULE_DESCRIPTION_JSON_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/fieldmodule_description.json")
SET(FIELDMODULE_HEART_SURFACE_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/heart_surface.exfile")
SET(FIELDMODULE_PLATE_600X300_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/plate_600x300.exfile")
SET(FIELDMODULE_EX2_PART_SURFACES_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/data/part_surfaces.ex2")
SET(FIELDMODULE_EX2_TWO_CUBES_HERMITE_NOCROSS_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/data/two_cubes_hermite_nocross.ex2")
SET(FIELDMODULE_EX2_CYLINDER_TEXTURE_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/data/cylinder_texture.ex2")
SET(FIELDMODULE_EX2_CUBE_NODE_ELEMENT_GRID_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/data/cube_node_element_grid.exf")
SET(FIELDMODULE_CUBE_TRIQUADRATIC_DELTA_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/data/cube_triquadratic_delta.ex2")
SET(FIELDMODULE_EX2_ELEMENT_BASES_3D_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/data/element_bases_3d.ex2")
SET(FIELDMODULE_EX2_NON_NUMERIC_COORDINATE_SYSTEM_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/data/non_numeric_coordinate_system.exf")
SET(FIELDMODULE_EX3_EMBED_HOST_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/data/embed_host.exf")
SET(FIELDMODULE_EX3_EMBED_NETWORK_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/data/embed_network.exf")
SET(FIELDMODULE_EX3_FIELDRANGE_CURVE_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/data/fieldrange_curve.exf")
