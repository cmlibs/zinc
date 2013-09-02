
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/element.h>
#include <zinc/field.h>
#include <zinc/fieldmodule.h>
#include <zinc/fieldfiniteelement.h>
#include <zinc/node.h>
#include <zinc/region.h>
#include <zinc/status.h>
#include <zinc/stream.h>

#include "zinctestsetup.hpp"

#include "test_resources.h"


TEST(nodes_elements_identifier, set_identifier)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);

	cmzn_region_id cube_region = cmzn_region_create_child(root_region, "cube");
	cmzn_stream_information_id cube_si = cmzn_region_create_stream_information(
		cube_region);
	EXPECT_NE(static_cast<cmzn_stream_information *>(0), cube_si);

	cmzn_stream_resource_id cube_sr = cmzn_stream_information_create_resource_file(
		cube_si, TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE));
	EXPECT_NE(static_cast<cmzn_stream_resource *>(0), cube_sr);

	int result = 0;
	EXPECT_EQ(CMZN_OK, result = cmzn_region_read(cube_region, cube_si));

	cmzn_stream_resource_destroy(&cube_sr);
	cmzn_stream_information_destroy(&cube_si);

	cmzn_field_module_id cubeFM = cmzn_region_get_field_module(cube_region);
	EXPECT_NE(static_cast<cmzn_field_module *>(0), cubeFM);

	cmzn_nodeset_id nodeset = cmzn_field_module_find_nodeset_by_name(cubeFM, "nodes");
	EXPECT_NE(static_cast<cmzn_nodeset *>(0), nodeset);

	cmzn_node_id node = cmzn_nodeset_find_node_by_identifier(nodeset, 1);
	EXPECT_NE(static_cast<cmzn_node *>(0), node);

	EXPECT_EQ(CMZN_ERROR_GENERAL, result = cmzn_node_set_identifier(node, 3));
	EXPECT_EQ(CMZN_OK, result = cmzn_node_set_identifier(node, 9));

	cmzn_mesh_id mesh = cmzn_field_module_find_mesh_by_dimension(cubeFM, 3);
	EXPECT_NE(static_cast<cmzn_mesh *>(0), mesh);

	cmzn_element_id element = cmzn_mesh_find_element_by_identifier(mesh, 1);
	EXPECT_NE(static_cast<cmzn_element *>(0), element);

	EXPECT_EQ(CMZN_ERROR_GENERAL, result = cmzn_element_set_identifier(element, 1));
	EXPECT_EQ(CMZN_OK, result = cmzn_element_set_identifier(element, 2));

	cmzn_element_destroy(&element);
	cmzn_mesh_destroy(&mesh);
	cmzn_field_module_destroy(&cubeFM);
	cmzn_nodeset_destroy(&nodeset);
	cmzn_node_destroy(&node);

	cmzn_region_destroy(&cube_region);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
}
