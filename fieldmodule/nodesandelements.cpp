
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
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);

	Cmiss_region_id cube_region = Cmiss_region_create_child(root_region, "cube");
	Cmiss_stream_information_id cube_si = Cmiss_region_create_stream_information(
		cube_region);
	EXPECT_NE(static_cast<Cmiss_stream_information *>(0), cube_si);

	Cmiss_stream_resource_id cube_sr = Cmiss_stream_information_create_resource_file(
		cube_si, TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE));
	EXPECT_NE(static_cast<Cmiss_stream_resource *>(0), cube_sr);

	int result = 0;
	EXPECT_EQ(CMISS_OK, result = Cmiss_region_read(cube_region, cube_si));

	Cmiss_stream_resource_destroy(&cube_sr);
	Cmiss_stream_information_destroy(&cube_si);

	Cmiss_field_module_id cubeFM = Cmiss_region_get_field_module(cube_region);
	EXPECT_NE(static_cast<Cmiss_field_module *>(0), cubeFM);

	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(cubeFM, "cmiss_nodes");
	EXPECT_NE(static_cast<Cmiss_nodeset *>(0), nodeset);

	Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, 1);
	EXPECT_NE(static_cast<Cmiss_node *>(0), node);

	EXPECT_EQ(CMISS_ERROR_GENERAL, result = Cmiss_node_set_identifier(node, 3));
	EXPECT_EQ(CMISS_OK, result = Cmiss_node_set_identifier(node, 9));

	Cmiss_mesh_id mesh = Cmiss_field_module_find_mesh_by_dimension(cubeFM, 3);
	EXPECT_NE(static_cast<Cmiss_mesh *>(0), mesh);

	Cmiss_element_id element = Cmiss_mesh_find_element_by_identifier(mesh, 1);
	EXPECT_NE(static_cast<Cmiss_element *>(0), element);

	EXPECT_EQ(CMISS_ERROR_GENERAL, result = Cmiss_element_set_identifier(element, 1));
	EXPECT_EQ(CMISS_OK, result = Cmiss_element_set_identifier(element, 2));

	Cmiss_element_destroy(&element);
	Cmiss_mesh_destroy(&mesh);
	Cmiss_field_module_destroy(&cubeFM);
	Cmiss_nodeset_destroy(&nodeset);
	Cmiss_node_destroy(&node);

	Cmiss_region_destroy(&cube_region);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}
