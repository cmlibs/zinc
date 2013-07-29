
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

TEST(exdata_and_exnodes_file, invalid_args)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);

	Cmiss_region_id node_region = Cmiss_region_create_child(root_region, "node");
	Cmiss_stream_information_id node_si = Cmiss_region_create_stream_information(
		node_region);
	EXPECT_NE(static_cast<Cmiss_stream_information *>(0), node_si);

	Cmiss_stream_resource_id node_sr = Cmiss_stream_information_create_resource_file(
		node_si, TestResources::getLocation(TestResources::FIELDMODULE_EXNODE_RESOURCE));
	EXPECT_NE(static_cast<Cmiss_stream_resource *>(0), node_sr);

	int result = Cmiss_region_read(node_region, node_si);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_stream_resource_destroy(&node_sr);
	Cmiss_stream_information_destroy(&node_si);

	Cmiss_field_module_id node_fm = Cmiss_region_get_field_module(node_region);
	EXPECT_NE(static_cast<Cmiss_field_module *>(0), node_fm);

	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(node_fm, "cmiss_nodes");
	EXPECT_NE(static_cast<Cmiss_nodeset *>(0), nodeset);

	int numberOfNodes = Cmiss_nodeset_get_size(nodeset);
	EXPECT_EQ(16, numberOfNodes);

	Cmiss_nodeset_destroy(&nodeset);
	Cmiss_field_module_destroy(&node_fm);

	Cmiss_region_id data_region = Cmiss_region_create_child(root_region, "data");
	Cmiss_stream_information_id data_si = Cmiss_region_create_stream_information(
		data_region);
	EXPECT_NE(static_cast<Cmiss_stream_information *>(0), data_si);

	Cmiss_stream_resource_id data_sr = Cmiss_stream_information_create_resource_file(
		data_si, TestResources::getLocation(TestResources::FIELDMODULE_EXNODE_RESOURCE));
	EXPECT_NE(static_cast<Cmiss_stream_resource *>(0), data_sr);

	Cmiss_stream_information_region_id data_region_si = Cmiss_stream_information_cast_region(
		data_si);
	EXPECT_NE(static_cast<Cmiss_stream_information_region *>(0), data_region_si);

	Cmiss_stream_information_region_set_resource_domain_type(data_region_si,
		data_sr,	CMISS_FIELD_DOMAIN_DATA);

	EXPECT_NE(static_cast<Cmiss_stream_resource *>(0), data_sr);

	result = Cmiss_region_read(data_region, data_si);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_stream_resource_destroy(&data_sr);
	Cmiss_stream_information_destroy(&data_si);

	data_si = Cmiss_stream_information_region_base_cast(data_region_si);
	Cmiss_stream_information_destroy(&data_si);

	Cmiss_field_module_id data_fm = Cmiss_region_get_field_module(data_region);
	EXPECT_NE(static_cast<Cmiss_field_module *>(0), data_fm);

	Cmiss_nodeset_id dataset = Cmiss_field_module_find_nodeset_by_name(data_fm, "cmiss_data");
	EXPECT_NE(static_cast<Cmiss_nodeset *>(0), dataset);

	numberOfNodes = Cmiss_nodeset_get_size(dataset);
	EXPECT_EQ(16, numberOfNodes);

	data_si = Cmiss_region_create_stream_information(data_region);
	data_sr = Cmiss_stream_information_create_resource_memory(data_si);

	result = Cmiss_region_write(data_region, data_si);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_stream_resource_memory_id memeory_sr = Cmiss_stream_resource_cast_memory(
		data_sr);

	char *memory_buffer;
	unsigned int size = 0;

	result = Cmiss_stream_resource_memory_get_buffer(memeory_sr, (void**)&memory_buffer, &size);
	EXPECT_EQ(CMISS_OK, result);

	char *temp_char = strstr ( memory_buffer, "!#nodeset datapoints");
	EXPECT_NE(static_cast<char *>(0), temp_char);

	Cmiss_nodeset_destroy(&dataset);
	Cmiss_field_module_destroy(&data_fm);

	Cmiss_region_id new_data_region = Cmiss_region_create_child(root_region, "new_data");
	Cmiss_stream_information_id new_data_si = Cmiss_region_create_stream_information(
		new_data_region);
	EXPECT_NE(static_cast<Cmiss_stream_information *>(0), new_data_si);

	Cmiss_stream_resource_id new_data_sr = Cmiss_stream_information_create_resource_memory_buffer(
		new_data_si, memory_buffer, size);
	EXPECT_NE(static_cast<Cmiss_stream_resource *>(0), new_data_sr);

	result = Cmiss_region_read(new_data_region, new_data_si);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_field_module_id new_data_fm = Cmiss_region_get_field_module(new_data_region);
	EXPECT_NE(static_cast<Cmiss_field_module *>(0), new_data_fm);

	Cmiss_nodeset_id new_dataset = Cmiss_field_module_find_nodeset_by_name(new_data_fm, "cmiss_data");
	EXPECT_NE(static_cast<Cmiss_nodeset *>(0), new_dataset);

	numberOfNodes = Cmiss_nodeset_get_size(new_dataset);
	EXPECT_EQ(16, numberOfNodes);

	Cmiss_stream_resource_destroy(&new_data_sr);
	Cmiss_stream_information_destroy(&new_data_si);

	Cmiss_nodeset_destroy(&new_dataset);
	Cmiss_field_module_destroy(&new_data_fm);

	Cmiss_stream_resource_destroy(&data_sr);
	Cmiss_stream_information_destroy(&data_si);

	data_sr = Cmiss_stream_resource_memory_base_cast(memeory_sr);
	Cmiss_stream_resource_destroy(&data_sr);

	Cmiss_region_destroy(&new_data_region);
	Cmiss_region_destroy(&data_region);
	Cmiss_region_destroy(&node_region);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}
