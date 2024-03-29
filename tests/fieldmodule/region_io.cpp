/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <cmlibs/zinc/core.h>
#include <cmlibs/zinc/context.h>
#include <cmlibs/zinc/element.h>
#include <cmlibs/zinc/field.h>
#include <cmlibs/zinc/fieldcache.h>
#include <cmlibs/zinc/fieldmodule.h>
#include <cmlibs/zinc/fieldfiniteelement.h>
#include <cmlibs/zinc/fieldnodesetoperators.h>
#include <cmlibs/zinc/mesh.h>
#include <cmlibs/zinc/node.h>
#include <cmlibs/zinc/nodeset.h>
#include <cmlibs/zinc/region.h>
#include <cmlibs/zinc/status.h>
#include <cmlibs/zinc/stream.h>
#include <cmlibs/zinc/streamregion.h>

#include "zinctestsetup.hpp"

#include <string>       // std::string
#include <iostream>     // std::cout, std::ostream, std::hex
#include <sstream>
#include <fstream>

#include "test_resources.h"

TEST(region_file_input, invalid_args)
{
    cmzn_context_id context = cmzn_context_create("test");
    cmzn_region_id root_region = cmzn_context_get_default_region(context);

    cmzn_streaminformation_id si = cmzn_region_create_streaminformation_region(
        root_region);
    EXPECT_NE(static_cast<cmzn_streaminformation *>(0), si);

    cmzn_streamresource_id sr = cmzn_streaminformation_create_streamresource_file(
        si, resourcePath("fieldmodule/region_input.exf").c_str());
    EXPECT_NE(static_cast<cmzn_streamresource *>(0), sr);

    cmzn_streaminformation_region_id si_region = cmzn_streaminformation_cast_region(
        si);
    EXPECT_NE(static_cast<cmzn_streaminformation_region *>(0), si_region);

    int result = cmzn_region_read(root_region, si_region);
    EXPECT_EQ(CMZN_OK, result);

    cmzn_streamresource_destroy(&sr);
    cmzn_streaminformation_region_destroy(&si_region);
    cmzn_streaminformation_destroy(&si);

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

TEST(region_file_output, invalid_args)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);

	cmzn_streaminformation_id si = cmzn_region_create_streaminformation_region(
		root_region);
	EXPECT_NE(static_cast<cmzn_streaminformation *>(0), si);

	cmzn_streamresource_id sr = cmzn_streaminformation_create_streamresource_file(
        si, resourcePath("fieldmodule/region_input.exf").c_str());
	EXPECT_NE(static_cast<cmzn_streamresource *>(0), sr);

	cmzn_streaminformation_region_id si_region = cmzn_streaminformation_cast_region(
		si);
	EXPECT_NE(static_cast<cmzn_streaminformation_region *>(0), si_region);

	int result = cmzn_region_read(root_region, si_region);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_streamresource_destroy(&sr);
	cmzn_streaminformation_region_destroy(&si_region);
	cmzn_streaminformation_destroy(&si);

	si = cmzn_region_create_streaminformation_region(
			root_region);
	EXPECT_NE(static_cast<cmzn_streaminformation *>(0), si);

	si_region = cmzn_streaminformation_cast_region(
		si);
	EXPECT_NE(static_cast<cmzn_streaminformation_region *>(0), si_region);

	sr = cmzn_streaminformation_create_streamresource_memory(si);
	EXPECT_NE(static_cast<cmzn_streamresource *>(0), sr);

	result = cmzn_streaminformation_region_set_resource_recursion_mode(si_region, sr,
		CMZN_STREAMINFORMATION_REGION_RECURSION_MODE_OFF);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_streamresource_memory_id memory_sr = cmzn_streamresource_cast_memory(
		sr);
	EXPECT_NE(static_cast<cmzn_streamresource_memory *>(0), memory_sr);

	result = cmzn_region_write(root_region, si_region);
	EXPECT_EQ(CMZN_OK, result);

	const char *memory_buffer;
	unsigned int size = 0;

	result = cmzn_streamresource_memory_get_buffer(memory_sr, (const void**)&memory_buffer, &size);
	EXPECT_EQ(CMZN_OK, result);

	const char *regionName = "/plate";
	const char *temp_char = strstr ( memory_buffer, regionName);
	EXPECT_EQ(static_cast<char *>(0), temp_char);

	cmzn_streamresource_destroy(&sr);
	sr = cmzn_streamresource_memory_base_cast(memory_sr);
	cmzn_streamresource_destroy(&sr);
	cmzn_streaminformation_region_destroy(&si_region);
	cmzn_streaminformation_destroy(&si);

    cmzn_region_id plate_region = cmzn_region_find_child_by_name(
        root_region, "plate");
    EXPECT_NE(static_cast<cmzn_region *>(0), plate_region);

    si = cmzn_region_create_streaminformation_region(
        plate_region);
    EXPECT_NE(static_cast<cmzn_streaminformation *>(0), si);

    si_region = cmzn_streaminformation_cast_region(si);
    EXPECT_NE(static_cast<cmzn_streaminformation_region *>(0), si_region);

    sr = cmzn_streaminformation_create_streamresource_memory(si);
    EXPECT_NE(static_cast<cmzn_streamresource *>(0), sr);

    memory_sr = cmzn_streamresource_cast_memory(sr);
    EXPECT_NE(static_cast<cmzn_streamresource_memory *>(0), memory_sr);

    cmzn_streamresource_id sr2 = cmzn_streaminformation_create_streamresource_memory(si);
    EXPECT_NE(static_cast<cmzn_streamresource *>(0), sr2);

    cmzn_streamresource_memory_id memory_sr2 = cmzn_streamresource_cast_memory(sr2);
    EXPECT_NE(static_cast<cmzn_streamresource_memory *>(0), memory_sr2);

    const char *fieldName = "temperature";

    result = cmzn_streaminformation_region_set_resource_field_names(si_region, sr, 1, &fieldName);
    EXPECT_EQ(CMZN_OK, result);

    result = cmzn_streaminformation_region_set_resource_group_name(si_region, sr2, "elevated");
    EXPECT_EQ(CMZN_OK, result);

    result = cmzn_region_write(plate_region, si_region);
    EXPECT_EQ(CMZN_OK, result);

    memory_buffer = 0;
    size = 0;

    result = cmzn_streamresource_memory_get_buffer(memory_sr, (const void**)&memory_buffer, &size);
    EXPECT_EQ(CMZN_OK, result);

    temp_char = strstr ( memory_buffer, fieldName);
    EXPECT_NE(static_cast<char *>(0), temp_char);

    temp_char = strstr ( memory_buffer, "coordinates");
    EXPECT_EQ(static_cast<char *>(0), temp_char);

    result = cmzn_streamresource_memory_get_buffer(memory_sr2, (const void**)&memory_buffer, &size);
    EXPECT_EQ(CMZN_OK, result);

    temp_char = strstr ( memory_buffer, "elevated");
    EXPECT_NE(static_cast<char *>(0), temp_char);

    temp_char = strstr ( memory_buffer, "Node: 1");
    EXPECT_EQ(static_cast<char *>(0), temp_char);

    cmzn_streamresource_destroy(&sr);
    sr = cmzn_streamresource_memory_base_cast(memory_sr);
    cmzn_streamresource_destroy(&sr);
    cmzn_streamresource_destroy(&sr2);
    sr2 = cmzn_streamresource_memory_base_cast(memory_sr2);
    cmzn_streamresource_destroy(&sr2);
    cmzn_streaminformation_region_destroy(&si_region);
    cmzn_streaminformation_destroy(&si);

    cmzn_region_destroy(&plate_region);

	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
}

TEST(issue3614, read_embedded_nodes)
{
    ZincTestSetup zinc;

    int result = cmzn_region_read_file(zinc.root_region,
        resourcePath("fieldmodule/embedding_issue3614.exregion").c_str());
    EXPECT_EQ(CMZN_OK, result);

    cmzn_field_id coordinates = cmzn_fieldmodule_find_field_by_name(zinc.fm, "coordinates");
    EXPECT_NE(static_cast<cmzn_field_id>(0), coordinates);
    cmzn_field_id hostLocation = cmzn_fieldmodule_find_field_by_name(zinc.fm, "host_location");
    EXPECT_NE(static_cast<cmzn_field_id>(0), hostLocation);
    cmzn_field_stored_mesh_location_id storedMeshLocation = cmzn_field_cast_stored_mesh_location(hostLocation);
    EXPECT_NE(static_cast<cmzn_field_stored_mesh_location_id>(0), storedMeshLocation);
    EXPECT_EQ(CMZN_FIELD_VALUE_TYPE_MESH_LOCATION, cmzn_field_get_value_type(hostLocation));
    cmzn_field_id hostCoordinates = cmzn_fieldmodule_create_field_embedded(zinc.fm, coordinates, hostLocation);
    EXPECT_NE(static_cast<cmzn_field_id>(0), hostCoordinates);

    cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(zinc.fm);

    cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(zinc.fm, CMZN_FIELD_DOMAIN_TYPE_NODES);
    EXPECT_NE(static_cast<cmzn_nodeset_id>(0), nodeset);
    cmzn_node_id node = cmzn_nodeset_find_node_by_identifier(nodeset, 7);
    EXPECT_NE(static_cast<cmzn_node_id>(0), node);
    result = cmzn_fieldcache_set_node(cache, node);
    EXPECT_EQ(CMZN_OK, result);
    double xi[2];
    cmzn_element_id element = cmzn_field_evaluate_mesh_location(hostLocation, cache, 2, xi);
    EXPECT_EQ(1, cmzn_element_get_identifier(element));
    ASSERT_DOUBLE_EQ(0.9, xi[0]);
    ASSERT_DOUBLE_EQ(0.75, xi[1]);

    double x[3] = { 0.0, 0.0, 0.0 };
    result = cmzn_field_evaluate_real(hostCoordinates, cache, 3, x);
    EXPECT_EQ(CMZN_OK, result);
    ASSERT_DOUBLE_EQ(0.9, x[0]);
    ASSERT_DOUBLE_EQ(0.75, x[1]);
    ASSERT_DOUBLE_EQ(0.0, x[2]);

    // test the element to embedded node map has been defined correctly:
    cmzn_field_id cmissNumber = cmzn_fieldmodule_find_field_by_name(zinc.fm, "cmiss_number");
    EXPECT_NE(static_cast<cmzn_field_id>(0), cmissNumber);
    cmzn_field_id sumCmissNumber = cmzn_fieldmodule_create_field_nodeset_sum(zinc.fm, cmissNumber, nodeset);
    EXPECT_NE(static_cast<cmzn_field_id>(0), sumCmissNumber);
    cmzn_field_nodeset_operator_id nodesetOperator = cmzn_field_cast_nodeset_operator(sumCmissNumber);
    EXPECT_NE(static_cast<cmzn_field_nodeset_operator_id>(0), nodesetOperator);
    EXPECT_EQ(CMZN_OK, cmzn_field_nodeset_operator_set_element_map_field(nodesetOperator, hostLocation));

    cmzn_fieldcache_id fieldcache = cmzn_fieldmodule_create_fieldcache(zinc.fm);
    EXPECT_EQ(CMZN_OK, cmzn_fieldcache_set_element(fieldcache, element));
    double sum;
    const double TOL = 1.0E-6;
    EXPECT_EQ(CMZN_OK, cmzn_field_evaluate_real(sumCmissNumber, fieldcache, 1, &sum));
    EXPECT_NEAR(26.0, sum, TOL);  // 5 + 6 + 7 + 8
    cmzn_nodeset_destroy_node(nodeset, node);
    EXPECT_EQ(CMZN_OK, cmzn_field_evaluate_real(sumCmissNumber, fieldcache, 1, &sum));
    EXPECT_NEAR(19.0, sum, TOL);  // 5 + 6 + 8

    cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(zinc.fm, 2);
    EXPECT_EQ(CMZN_OK, cmzn_mesh_destroy_element(mesh, element));
    // element is destroyed so evaluation fails
    EXPECT_EQ(CMZN_ERROR_GENERAL, cmzn_field_evaluate_real(sumCmissNumber, fieldcache, 1, &sum));

    cmzn_mesh_destroy(&mesh);
    cmzn_fieldcache_destroy(&fieldcache);
    cmzn_field_nodeset_operator_destroy(&nodesetOperator);
    cmzn_field_destroy(&sumCmissNumber);
    cmzn_field_destroy(&cmissNumber);
    cmzn_element_destroy(&element);
    cmzn_node_destroy(&node);
    cmzn_nodeset_destroy(&nodeset);
    cmzn_fieldcache_destroy(&cache);
    cmzn_field_destroy(&hostCoordinates);
    cmzn_field_stored_mesh_location_destroy(&storedMeshLocation);
    cmzn_field_destroy(&hostLocation);
    cmzn_field_destroy(&coordinates);
}

TEST(exdata_and_exnodes_file, invalid_args)
{
    cmzn_context_id context = cmzn_context_create("test");
    cmzn_region_id root_region = cmzn_context_get_default_region(context);

    cmzn_region_id node_region = cmzn_region_create_child(root_region, "node");
    cmzn_streaminformation_id node_si = cmzn_region_create_streaminformation_region(
        node_region);
    EXPECT_NE(static_cast<cmzn_streaminformation *>(0), node_si);

    cmzn_streaminformation_region_id node_si_region = cmzn_streaminformation_cast_region(
        node_si);
    EXPECT_NE(static_cast<cmzn_streaminformation_region *>(0), node_si_region);

    cmzn_streamresource_id node_sr = cmzn_streaminformation_create_streamresource_file(
        node_si, resourcePath("fieldmodule/nodes.exnode").c_str());
    EXPECT_NE(static_cast<cmzn_streamresource *>(0), node_sr);

    int result = cmzn_region_read(node_region, node_si_region);
    EXPECT_EQ(CMZN_OK, result);

    cmzn_streamresource_destroy(&node_sr);
    cmzn_streaminformation_region_destroy(&node_si_region);
    cmzn_streaminformation_destroy(&node_si);

    cmzn_fieldmodule_id node_fm = cmzn_region_get_fieldmodule(node_region);
    EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), node_fm);

    cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_name(node_fm, "nodes");
    EXPECT_NE(static_cast<cmzn_nodeset *>(0), nodeset);

    int numberOfNodes = cmzn_nodeset_get_size(nodeset);
    EXPECT_EQ(16, numberOfNodes);

    cmzn_nodeset_destroy(&nodeset);
    cmzn_fieldmodule_destroy(&node_fm);

    cmzn_region_id data_region = cmzn_region_create_child(root_region, "data");
    cmzn_streaminformation_id data_si = cmzn_region_create_streaminformation_region(
        data_region);
    EXPECT_NE(static_cast<cmzn_streaminformation *>(0), data_si);

    cmzn_streamresource_id data_sr = cmzn_streaminformation_create_streamresource_file(
        data_si, resourcePath("fieldmodule/nodes.exnode").c_str());
    EXPECT_NE(static_cast<cmzn_streamresource *>(0), data_sr);

    cmzn_streaminformation_region_id data_region_si = cmzn_streaminformation_cast_region(
        data_si);
    EXPECT_NE(static_cast<cmzn_streaminformation_region *>(0), data_region_si);

    cmzn_streaminformation_region_set_resource_domain_types(data_region_si,
        data_sr, CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS);

    EXPECT_NE(static_cast<cmzn_streamresource *>(0), data_sr);

    result = cmzn_region_read(data_region, data_region_si);
    EXPECT_EQ(CMZN_OK, result);

    cmzn_streamresource_destroy(&data_sr);
    cmzn_streaminformation_destroy(&data_si);

    cmzn_streaminformation_region_destroy(&data_region_si);

    cmzn_fieldmodule_id data_fm = cmzn_region_get_fieldmodule(data_region);
    EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), data_fm);

    cmzn_nodeset_id dataset = cmzn_fieldmodule_find_nodeset_by_name(data_fm, "datapoints");
    EXPECT_NE(static_cast<cmzn_nodeset *>(0), dataset);

    numberOfNodes = cmzn_nodeset_get_size(dataset);
    EXPECT_EQ(16, numberOfNodes);

    data_si = cmzn_region_create_streaminformation_region(data_region);
    data_sr = cmzn_streaminformation_create_streamresource_memory(data_si);

    data_region_si = cmzn_streaminformation_cast_region(
        data_si);
    EXPECT_NE(static_cast<cmzn_streaminformation_region *>(0), data_region_si);

    result = cmzn_region_write(data_region, data_region_si);
    EXPECT_EQ(CMZN_OK, result);

    cmzn_streaminformation_region_destroy(&data_region_si);

    cmzn_streamresource_memory_id memory_sr = cmzn_streamresource_cast_memory(
        data_sr);

    const char *memory_buffer;
    unsigned int size = 0;

    result = cmzn_streamresource_memory_get_buffer(memory_sr, (const void**)&memory_buffer, &size);
    EXPECT_EQ(CMZN_OK, result);

    const char *temp_char = strstr ( memory_buffer, "!#nodeset datapoints");
    EXPECT_NE(static_cast<char *>(0), temp_char);

    cmzn_nodeset_destroy(&dataset);
    cmzn_fieldmodule_destroy(&data_fm);

    cmzn_region_id new_data_region = cmzn_region_create_child(root_region, "new_data");
    cmzn_streaminformation_id new_data_si = cmzn_region_create_streaminformation_region(
        new_data_region);
    EXPECT_NE(static_cast<cmzn_streaminformation *>(0), new_data_si);

    cmzn_streamresource_id new_data_sr = cmzn_streaminformation_create_streamresource_memory_buffer(
        new_data_si, memory_buffer, size);
    EXPECT_NE(static_cast<cmzn_streamresource *>(0), new_data_sr);


    cmzn_streaminformation_region_id new_data_si_region = cmzn_streaminformation_cast_region(
        new_data_si);
    EXPECT_NE(static_cast<cmzn_streaminformation_region *>(0), new_data_si_region);

    result = cmzn_region_read(new_data_region, new_data_si_region);
    EXPECT_EQ(CMZN_OK, result);

    cmzn_fieldmodule_id new_data_fm = cmzn_region_get_fieldmodule(new_data_region);
    EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), new_data_fm);

    cmzn_nodeset_id new_dataset = cmzn_fieldmodule_find_nodeset_by_name(new_data_fm, "datapoints");
    EXPECT_NE(static_cast<cmzn_nodeset *>(0), new_dataset);

    numberOfNodes = cmzn_nodeset_get_size(new_dataset);
    EXPECT_EQ(16, numberOfNodes);

    cmzn_streamresource_destroy(&new_data_sr);
    cmzn_streaminformation_destroy(&new_data_si);
    cmzn_streaminformation_region_destroy(&new_data_si_region);

    cmzn_nodeset_destroy(&new_dataset);
    cmzn_fieldmodule_destroy(&new_data_fm);

    cmzn_streamresource_destroy(&data_sr);
    cmzn_streaminformation_destroy(&data_si);

    data_sr = cmzn_streamresource_memory_base_cast(memory_sr);
    cmzn_streamresource_destroy(&data_sr);

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
    cmzn_streaminformation_id cube_si = cmzn_region_create_streaminformation_region(
        cube_region);
    EXPECT_NE(static_cast<cmzn_streaminformation *>(0), cube_si);

    cmzn_streamresource_id cube_sr = cmzn_streaminformation_create_streamresource_file(
        cube_si, resourcePath("fieldmodule/cube.exformat").c_str());
    EXPECT_NE(static_cast<cmzn_streamresource *>(0), cube_sr);

    cmzn_streaminformation_region_id cube_si_region = cmzn_streaminformation_cast_region(
        cube_si);

    int result = cmzn_region_read(cube_region, cube_si_region);
    EXPECT_EQ(CMZN_OK, result);

    cmzn_streamresource_destroy(&cube_sr);
    cmzn_streaminformation_region_destroy(&cube_si_region);
    cmzn_streaminformation_destroy(&cube_si);

    cmzn_streaminformation_id output_si = cmzn_region_create_streaminformation_region(cube_region);
    cmzn_streamresource_id output_sr = cmzn_streaminformation_create_streamresource_memory(output_si);

    cmzn_streaminformation_region_id output_region_si = cmzn_streaminformation_cast_region(
        output_si);
    EXPECT_NE(static_cast<cmzn_streaminformation_region *>(0), output_region_si);

    cmzn_streaminformation_region_set_resource_domain_types(output_region_si,
        output_sr, CMZN_FIELD_DOMAIN_TYPE_MESH1D|CMZN_FIELD_DOMAIN_TYPE_MESH2D);

    result = cmzn_region_write(cube_region, output_region_si);
    EXPECT_EQ(CMZN_OK, result);

    cmzn_streamresource_memory_id memory_sr = cmzn_streamresource_cast_memory(
        output_sr);

    const char *memory_buffer;
    unsigned int size = 0;

    result = cmzn_streamresource_memory_get_buffer(memory_sr, (const void**)&memory_buffer, &size);
    EXPECT_EQ(CMZN_OK, result);

    const char *temp_char = strstr ( memory_buffer, "Dimension=1");
    EXPECT_NE(static_cast<char *>(0), temp_char);

    temp_char = strstr ( memory_buffer, "Dimension=2");
    EXPECT_NE(static_cast<char *>(0), temp_char);

    temp_char = strstr ( memory_buffer, "Dimension=3");
    EXPECT_EQ(static_cast<char *>(0), temp_char);

    cmzn_streamresource_destroy(&output_sr);
    cmzn_streaminformation_destroy(&output_si);

    output_sr = cmzn_streamresource_memory_base_cast(memory_sr);
    cmzn_streamresource_destroy(&output_sr);

    output_si = cmzn_streaminformation_region_base_cast(output_region_si);
    cmzn_streaminformation_destroy(&output_si);

    cmzn_region_destroy(&cube_region);
    cmzn_region_destroy(&root_region);
    cmzn_context_destroy(&context);
}

TEST(region_stream_gzip_input, invalid_args)
{
    cmzn_context_id context = cmzn_context_create("test");
    cmzn_region_id root_region = cmzn_context_get_default_region(context);
    std::ifstream exnodeFile(resourcePath("fieldmodule/heart.exnode.gz"), std::ifstream::binary);
    std::ifstream exelemFile(resourcePath("fieldmodule/heart.exelem.gz"), std::ifstream::binary);
    if (exnodeFile.is_open() && exelemFile.is_open())
    {
        exnodeFile.seekg (0, exnodeFile.end);
        const int exnodeSize = static_cast<int>(exnodeFile.tellg());
        char *memblock_exnode = new char [exnodeSize];
        exnodeFile.seekg (0, exnodeFile.beg);
        exnodeFile.read (memblock_exnode, exnodeSize);
        exnodeFile.close();
        exelemFile.seekg (0, exelemFile.end);
        const int exelemSize = static_cast<int>(exelemFile.tellg());
        char * memblock_exelem = new char [exelemSize];
        exelemFile.seekg (0, exelemFile.beg);
        exelemFile.read (memblock_exelem, exelemSize);
        exelemFile.close();
        cmzn_streaminformation_id si = cmzn_region_create_streaminformation_region(
            root_region);
        cmzn_streamresource_id sr_exnode = cmzn_streaminformation_create_streamresource_memory_buffer(
            si, (void *)memblock_exnode, exnodeSize);
        cmzn_streamresource_id sr_exelem = cmzn_streaminformation_create_streamresource_memory_buffer(
            si, (void *)memblock_exelem, exelemSize);
        cmzn_streaminformation_set_data_compression_type(si, CMZN_STREAMINFORMATION_DATA_COMPRESSION_TYPE_GZIP);
        cmzn_streaminformation_region_id si_region = cmzn_streaminformation_cast_region(
            si);
        int result = cmzn_region_read(root_region, si_region);
        EXPECT_EQ(CMZN_OK, result);
        cmzn_streamresource_destroy(&sr_exnode);
        cmzn_streamresource_destroy(&sr_exelem);
        cmzn_streaminformation_region_destroy(&si_region);
        cmzn_streaminformation_destroy(&si);

        if (memblock_exnode)
            delete[] memblock_exnode;
        if (memblock_exelem)
            delete[] memblock_exelem;
    }
    cmzn_region_destroy(&root_region);
    cmzn_context_destroy(&context);
}
