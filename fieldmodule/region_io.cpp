
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
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);

	cmzn_stream_information_id si = cmzn_region_create_stream_information(
		root_region);
	EXPECT_NE(static_cast<cmzn_stream_information *>(0), si);

	cmzn_stream_resource_id sr = cmzn_stream_information_create_resource_file(
		si, TestResources::getLocation(TestResources::FIELDMODULE_REGION_INPUT_RESOURCE));
	EXPECT_NE(static_cast<cmzn_stream_resource *>(0), sr);

	int result = cmzn_region_read(root_region, si);
	EXPECT_EQ(CMISS_OK, result);

	cmzn_stream_resource_destroy(&sr);
	cmzn_stream_information_destroy(&si);

	cmzn_region_id plate_region = cmzn_region_find_child_by_name(
		root_region, "plate");
	EXPECT_NE(static_cast<cmzn_region *>(0), plate_region);
	cmzn_region_destroy(&plate_region);

	cmzn_region_id tetrahedron_region = cmzn_region_find_child_by_name(
		root_region, "tetrahedron");
	EXPECT_NE(static_cast<cmzn_region *>(0), tetrahedron_region);

	cmzn_region_id starburst_region = cmzn_region_find_child_by_name(
		tetrahedron_region, "starburst");
	EXPECT_NE(static_cast<cmzn_region *>(0), starburst_region);

	cmzn_region_destroy(&starburst_region);
	cmzn_region_destroy(&tetrahedron_region);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
}

TEST(issue3614, read_embedded_nodes)
{
	ZincTestSetup zinc;

	int result = cmzn_region_read_file(zinc.root_region,
		TestResources::getLocation(TestResources::FIELDMODULE_EMBEDDING_ISSUE3614_RESOURCE));
	EXPECT_EQ(CMISS_OK, result);

	cmzn_field_id coordinates = cmzn_field_module_find_field_by_name(zinc.fm, "coordinates");
	EXPECT_NE(static_cast<cmzn_field_id>(0), coordinates);
	cmzn_field_id hostLocation = cmzn_field_module_find_field_by_name(zinc.fm, "host_location");
	EXPECT_NE(static_cast<cmzn_field_id>(0), hostLocation);
	EXPECT_EQ(CMISS_FIELD_VALUE_TYPE_MESH_LOCATION, cmzn_field_get_value_type(hostLocation));
	cmzn_field_id hostCoordinates = cmzn_field_module_create_embedded(zinc.fm, coordinates, hostLocation);
	EXPECT_NE(static_cast<cmzn_field_id>(0), hostLocation);

	cmzn_field_cache_id cache = cmzn_field_module_create_cache(zinc.fm);

	cmzn_nodeset_id nodeset = cmzn_field_module_find_nodeset_by_domain_type(zinc.fm, CMISS_FIELD_DOMAIN_NODES);
	EXPECT_NE(static_cast<cmzn_nodeset_id>(0), nodeset);
	cmzn_node_id node = cmzn_nodeset_find_node_by_identifier(nodeset, 1003);
	EXPECT_NE(static_cast<cmzn_node_id>(0), node);
	cmzn_field_cache_set_node(cache, node);
	double x[3] = { 0.0, 0.0, 0.0 };
	result = cmzn_field_evaluate_real(hostCoordinates, cache, 3, x);
	EXPECT_EQ(CMISS_OK, result);
	ASSERT_DOUBLE_EQ(0.25, x[0]);
	ASSERT_DOUBLE_EQ(0.75, x[1]);
	ASSERT_DOUBLE_EQ(0.0, x[2]);

	cmzn_node_destroy(&node);
	cmzn_nodeset_destroy(&nodeset);
	cmzn_field_cache_destroy(&cache);
	cmzn_field_destroy(&hostCoordinates);
	cmzn_field_destroy(&hostLocation);
	cmzn_field_destroy(&coordinates);
}

TEST(exdata_and_exnodes_file, invalid_args)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);

	cmzn_region_id node_region = cmzn_region_create_child(root_region, "node");
	cmzn_stream_information_id node_si = cmzn_region_create_stream_information(
		node_region);
	EXPECT_NE(static_cast<cmzn_stream_information *>(0), node_si);

	cmzn_stream_resource_id node_sr = cmzn_stream_information_create_resource_file(
		node_si, TestResources::getLocation(TestResources::FIELDMODULE_EXNODE_RESOURCE));
	EXPECT_NE(static_cast<cmzn_stream_resource *>(0), node_sr);

	int result = cmzn_region_read(node_region, node_si);
	EXPECT_EQ(CMISS_OK, result);

	cmzn_stream_resource_destroy(&node_sr);
	cmzn_stream_information_destroy(&node_si);

	cmzn_field_module_id node_fm = cmzn_region_get_field_module(node_region);
	EXPECT_NE(static_cast<cmzn_field_module *>(0), node_fm);

	cmzn_nodeset_id nodeset = cmzn_field_module_find_nodeset_by_name(node_fm, "nodes");
	EXPECT_NE(static_cast<cmzn_nodeset *>(0), nodeset);

	int numberOfNodes = cmzn_nodeset_get_size(nodeset);
	EXPECT_EQ(16, numberOfNodes);

	cmzn_nodeset_destroy(&nodeset);
	cmzn_field_module_destroy(&node_fm);

	cmzn_region_id data_region = cmzn_region_create_child(root_region, "data");
	cmzn_stream_information_id data_si = cmzn_region_create_stream_information(
		data_region);
	EXPECT_NE(static_cast<cmzn_stream_information *>(0), data_si);

	cmzn_stream_resource_id data_sr = cmzn_stream_information_create_resource_file(
		data_si, TestResources::getLocation(TestResources::FIELDMODULE_EXNODE_RESOURCE));
	EXPECT_NE(static_cast<cmzn_stream_resource *>(0), data_sr);

	cmzn_stream_information_region_id data_region_si = cmzn_stream_information_cast_region(
		data_si);
	EXPECT_NE(static_cast<cmzn_stream_information_region *>(0), data_region_si);

	cmzn_stream_information_region_set_resource_domain_type(data_region_si,
		data_sr,	CMISS_FIELD_DOMAIN_DATA);

	EXPECT_NE(static_cast<cmzn_stream_resource *>(0), data_sr);

	result = cmzn_region_read(data_region, data_si);
	EXPECT_EQ(CMISS_OK, result);

	cmzn_stream_resource_destroy(&data_sr);
	cmzn_stream_information_destroy(&data_si);

	data_si = cmzn_stream_information_region_base_cast(data_region_si);
	cmzn_stream_information_destroy(&data_si);

	cmzn_field_module_id data_fm = cmzn_region_get_field_module(data_region);
	EXPECT_NE(static_cast<cmzn_field_module *>(0), data_fm);

	cmzn_nodeset_id dataset = cmzn_field_module_find_nodeset_by_name(data_fm, "datapoints");
	EXPECT_NE(static_cast<cmzn_nodeset *>(0), dataset);

	numberOfNodes = cmzn_nodeset_get_size(dataset);
	EXPECT_EQ(16, numberOfNodes);

	data_si = cmzn_region_create_stream_information(data_region);
	data_sr = cmzn_stream_information_create_resource_memory(data_si);

	result = cmzn_region_write(data_region, data_si);
	EXPECT_EQ(CMISS_OK, result);

	cmzn_stream_resource_memory_id memeory_sr = cmzn_stream_resource_cast_memory(
		data_sr);

	char *memory_buffer;
	unsigned int size = 0;

	result = cmzn_stream_resource_memory_get_buffer(memeory_sr, (void**)&memory_buffer, &size);
	EXPECT_EQ(CMISS_OK, result);

	char *temp_char = strstr ( memory_buffer, "!#nodeset datapoints");
	EXPECT_NE(static_cast<char *>(0), temp_char);

	cmzn_nodeset_destroy(&dataset);
	cmzn_field_module_destroy(&data_fm);

	cmzn_region_id new_data_region = cmzn_region_create_child(root_region, "new_data");
	cmzn_stream_information_id new_data_si = cmzn_region_create_stream_information(
		new_data_region);
	EXPECT_NE(static_cast<cmzn_stream_information *>(0), new_data_si);

	cmzn_stream_resource_id new_data_sr = cmzn_stream_information_create_resource_memory_buffer(
		new_data_si, memory_buffer, size);
	EXPECT_NE(static_cast<cmzn_stream_resource *>(0), new_data_sr);

	result = cmzn_region_read(new_data_region, new_data_si);
	EXPECT_EQ(CMISS_OK, result);

	cmzn_field_module_id new_data_fm = cmzn_region_get_field_module(new_data_region);
	EXPECT_NE(static_cast<cmzn_field_module *>(0), new_data_fm);

	cmzn_nodeset_id new_dataset = cmzn_field_module_find_nodeset_by_name(new_data_fm, "datapoints");
	EXPECT_NE(static_cast<cmzn_nodeset *>(0), new_dataset);

	numberOfNodes = cmzn_nodeset_get_size(new_dataset);
	EXPECT_EQ(16, numberOfNodes);

	cmzn_stream_resource_destroy(&new_data_sr);
	cmzn_stream_information_destroy(&new_data_si);

	cmzn_nodeset_destroy(&new_dataset);
	cmzn_field_module_destroy(&new_data_fm);

	cmzn_stream_resource_destroy(&data_sr);
	cmzn_stream_information_destroy(&data_si);

	data_sr = cmzn_stream_resource_memory_base_cast(memeory_sr);
	cmzn_stream_resource_destroy(&data_sr);

	cmzn_region_destroy(&new_data_region);
	cmzn_region_destroy(&data_region);
	cmzn_region_destroy(&node_region);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
}


TEST(element_dimension_file, invalid_args)
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

	int result = cmzn_region_read(cube_region, cube_si);
	EXPECT_EQ(CMISS_OK, result);

	cmzn_stream_resource_destroy(&cube_sr);
	cmzn_stream_information_destroy(&cube_si);

	cmzn_stream_information_id output_si = cmzn_region_create_stream_information(cube_region);
	cmzn_stream_resource_id output_sr = cmzn_stream_information_create_resource_memory(output_si);

	cmzn_stream_information_region_id output_region_si = cmzn_stream_information_cast_region(
		output_si);
	EXPECT_NE(static_cast<cmzn_stream_information_region *>(0), output_region_si);

	cmzn_stream_information_region_set_resource_domain_type(output_region_si,
		output_sr,	CMISS_FIELD_DOMAIN_MESH_1D|CMISS_FIELD_DOMAIN_MESH_2D);

	result = cmzn_region_write(cube_region, output_si);
	EXPECT_EQ(CMISS_OK, result);

	cmzn_stream_resource_memory_id memeory_sr = cmzn_stream_resource_cast_memory(
		output_sr);

	char *memory_buffer;
	unsigned int size = 0;

	result = cmzn_stream_resource_memory_get_buffer(memeory_sr, (void**)&memory_buffer, &size);
	EXPECT_EQ(CMISS_OK, result);

	char *temp_char = strstr ( memory_buffer, "Dimension=1");
	EXPECT_NE(static_cast<char *>(0), temp_char);

	temp_char = strstr ( memory_buffer, "Dimension=2");
	EXPECT_NE(static_cast<char *>(0), temp_char);

	temp_char = strstr ( memory_buffer, "Dimension=3");
	EXPECT_EQ(static_cast<char *>(0), temp_char);

	cmzn_stream_resource_destroy(&output_sr);
	cmzn_stream_information_destroy(&output_si);

	output_sr = cmzn_stream_resource_memory_base_cast(memeory_sr);
	cmzn_stream_resource_destroy(&output_sr);

	output_si = cmzn_stream_information_region_base_cast(output_region_si);
	cmzn_stream_information_destroy(&output_si);

	cmzn_region_destroy(&cube_region);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
}
