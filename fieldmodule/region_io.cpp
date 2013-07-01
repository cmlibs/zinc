
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/field.h>
#include <zinc/fieldmodule.h>
#include <zinc/fieldfiniteelement.h>
#include <zinc/node.h>
#include <zinc/region.h>
#include <zinc/status.h>
#include <zinc/stream.h>

#include "zinctestsetup.hpp"

#include "test_resources.h"

TEST(region_file_input, invalid_args)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);

	Cmiss_stream_information_id si = Cmiss_region_create_stream_information(
		root_region);
	EXPECT_NE(static_cast<Cmiss_stream_information *>(0), si);

	Cmiss_stream_resource_id sr = Cmiss_stream_information_create_resource_file(
		si, TestResources::getLocation(TestResources::FIELDMODULE_REGION_INPUT_RESOURCE));
	EXPECT_NE(static_cast<Cmiss_stream_resource *>(0), sr);

	int result = Cmiss_region_read(root_region, si);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_stream_resource_destroy(&sr);
	Cmiss_stream_information_destroy(&si);

	Cmiss_region_id plate_region = Cmiss_region_find_child_by_name(
		root_region, "plate");
	EXPECT_NE(static_cast<Cmiss_region *>(0), plate_region);
	Cmiss_region_destroy(&plate_region);

	Cmiss_region_id tetrahedron_region = Cmiss_region_find_child_by_name(
		root_region, "tetrahedron");
	EXPECT_NE(static_cast<Cmiss_region *>(0), tetrahedron_region);

	Cmiss_region_id starburst_region = Cmiss_region_find_child_by_name(
		tetrahedron_region, "starburst");
	EXPECT_NE(static_cast<Cmiss_region *>(0), starburst_region);

	Cmiss_region_destroy(&starburst_region);
	Cmiss_region_destroy(&tetrahedron_region);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(issue3614, read_embedded_nodes)
{
	ZincTestSetup zinc;

	int result = Cmiss_region_read_file(zinc.root_region,
		TestResources::getLocation(TestResources::FIELDMODULE_EMBEDDING_ISSUE3614_RESOURCE));
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_field_id coordinates = Cmiss_field_module_find_field_by_name(zinc.fm, "coordinates");
	EXPECT_NE(static_cast<Cmiss_field_id>(0), coordinates);
	Cmiss_field_id hostLocation = Cmiss_field_module_find_field_by_name(zinc.fm, "host_location");
	EXPECT_NE(static_cast<Cmiss_field_id>(0), hostLocation);
	EXPECT_EQ(CMISS_FIELD_VALUE_TYPE_MESH_LOCATION, Cmiss_field_get_value_type(hostLocation));
	Cmiss_field_id hostCoordinates = Cmiss_field_module_create_embedded(zinc.fm, coordinates, hostLocation);
	EXPECT_NE(static_cast<Cmiss_field_id>(0), hostLocation);

	Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(zinc.fm);

	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_domain_type(zinc.fm, CMISS_FIELD_DOMAIN_NODES);
	EXPECT_NE(static_cast<Cmiss_nodeset_id>(0), nodeset);
	Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, 1003);
	EXPECT_NE(static_cast<Cmiss_node_id>(0), node);
	Cmiss_field_cache_set_node(cache, node);
	double x[3] = { 0.0, 0.0, 0.0 };
	result = Cmiss_field_evaluate_real(hostCoordinates, cache, 3, x);
	EXPECT_EQ(CMISS_OK, result);
	ASSERT_DOUBLE_EQ(0.25, x[0]);
	ASSERT_DOUBLE_EQ(0.75, x[1]);
	ASSERT_DOUBLE_EQ(0.0, x[2]);

	Cmiss_node_destroy(&node);
	Cmiss_nodeset_destroy(&nodeset);
	Cmiss_field_cache_destroy(&cache);
	Cmiss_field_destroy(&hostCoordinates);
	Cmiss_field_destroy(&hostLocation);
	Cmiss_field_destroy(&coordinates);
}
