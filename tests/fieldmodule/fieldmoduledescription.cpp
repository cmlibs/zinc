/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/core.h>
#include <opencmiss/zinc/context.h>
#include <opencmiss/zinc/element.h>
#include <opencmiss/zinc/field.h>
#include <opencmiss/zinc/fieldcache.h>
#include <opencmiss/zinc/fieldmodule.h>
#include <opencmiss/zinc/fieldarithmeticoperators.h>
#include <opencmiss/zinc/fieldcomposite.h>
#include <opencmiss/zinc/fieldfiniteelement.h>
#include <opencmiss/zinc/node.h>
#include <opencmiss/zinc/region.h>
#include <opencmiss/zinc/status.h>
#include <opencmiss/zinc/stream.h>
#include <opencmiss/zinc/streamregion.h>

#include "zinctestsetup.hpp"

#include <string>       // std::string
#include <iostream>     // std::cout, std::ostream, std::hex
#include <sstream>
#include <fstream>

#include "test_resources.h"

TEST(fieldmodule_description, write)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);

	cmzn_streaminformation_id si = cmzn_region_create_streaminformation_region(
		root_region);
	EXPECT_NE(static_cast<cmzn_streaminformation *>(0), si);

	cmzn_streamresource_id sr = cmzn_streaminformation_create_streamresource_file(
		si, TestResources::getLocation(TestResources::FIELDMODULE_REGION_INPUT_RESOURCE));
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

	cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(tetrahedron_region);
	EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), fieldmodule);

	cmzn_field_id coordinatesField = cmzn_fieldmodule_find_field_by_name(fieldmodule, "coordinates");
	EXPECT_NE(static_cast<cmzn_field *>(0), coordinatesField);

	cmzn_field_id temperatureField = cmzn_fieldmodule_find_field_by_name(fieldmodule, "temperature");
	EXPECT_NE(static_cast<cmzn_field *>(0), temperatureField);

	int indexes[2] = {2,1};

	cmzn_field_id componentField = cmzn_fieldmodule_create_field_component_multiple(
		fieldmodule, coordinatesField, 2, indexes);
	cmzn_field_set_managed(componentField, true);
	cmzn_field_set_name(componentField, "component");
	EXPECT_NE(static_cast<cmzn_field *>(0), componentField);

	cmzn_field_id firstComponentField = cmzn_fieldmodule_create_field_component(
		fieldmodule, coordinatesField, 1);
	EXPECT_NE(static_cast<cmzn_field *>(0), firstComponentField);
	cmzn_field_set_managed(firstComponentField, false);
	cmzn_field_set_name(firstComponentField, "firstComponent");

	cmzn_field_id addField = cmzn_fieldmodule_create_field_add(
		fieldmodule, firstComponentField, temperatureField);
	EXPECT_NE(static_cast<cmzn_field *>(0), addField);
	cmzn_field_set_managed(addField, true);
	cmzn_field_set_name(addField, "add");

	cmzn_field_id sourceFields[2] = {coordinatesField, temperatureField};

	cmzn_field_id concatenateField = cmzn_fieldmodule_create_field_concatenate(
		fieldmodule, 2, sourceFields);
	cmzn_field_set_managed(concatenateField, true);
	cmzn_field_set_name(concatenateField, "concatenate");
	EXPECT_NE(static_cast<cmzn_field *>(0), concatenateField);

	char *description_string = cmzn_fieldmodule_write_description(fieldmodule);
	EXPECT_NE(static_cast<char *>(0), description_string);

	printf("%s", description_string);

	cmzn_field_destroy(&firstComponentField);
	cmzn_field_destroy(&addField);
	cmzn_field_destroy(&concatenateField);
	cmzn_field_destroy(&componentField);
	cmzn_field_destroy(&temperatureField);
	cmzn_field_destroy(&coordinatesField);
	cmzn_fieldmodule_destroy(&fieldmodule);

	cmzn_region_destroy(&tetrahedron_region);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
}


TEST(fieldmodule_description, read)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);

	cmzn_streaminformation_id si = cmzn_region_create_streaminformation_region(
		root_region);
	EXPECT_NE(static_cast<cmzn_streaminformation *>(0), si);

	cmzn_streamresource_id sr = cmzn_streaminformation_create_streamresource_file(
		si, TestResources::getLocation(TestResources::FIELDMODULE_REGION_INPUT_RESOURCE));
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

	cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(tetrahedron_region);
	EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), fieldmodule);

	void *buffer = 0;
	long length;
	FILE * f = fopen (TestResources::getLocation(TestResources::FIELDMODULE_DESCRIPTION_JSON_RESOURCE), "rb");
	if (f)
	{
		fseek (f, 0, SEEK_END);
		length = ftell (f);
		fseek (f, 0, SEEK_SET);
		buffer = malloc (length);
		if (buffer)
		{
			fread (buffer, 1, length, f);
		}
		fclose (f);
	}

	EXPECT_TRUE(buffer != 0);
	EXPECT_EQ(CMZN_OK, cmzn_fieldmodule_read_description(fieldmodule, (char *)buffer));
	free(buffer);

	cmzn_field_id coordinatesField = cmzn_fieldmodule_find_field_by_name(fieldmodule, "coordinates");
	EXPECT_NE(static_cast<cmzn_field *>(0), coordinatesField);

	cmzn_field_id temperatureField = cmzn_fieldmodule_find_field_by_name(fieldmodule, "temperature");
	EXPECT_NE(static_cast<cmzn_field *>(0), temperatureField);

	cmzn_field_id componentField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "component");
	EXPECT_NE(static_cast<cmzn_field *>(0), componentField);
	EXPECT_TRUE(cmzn_field_is_managed(componentField));
	EXPECT_EQ(1, cmzn_field_get_number_of_source_fields(componentField));
	EXPECT_EQ(2, cmzn_field_get_number_of_components(componentField));
	cmzn_field_id temp = cmzn_field_get_source_field(componentField, 1);
	EXPECT_EQ(temp, coordinatesField);
	cmzn_field_destroy(&temp);

	cmzn_field_component_id fieldComponent = cmzn_field_cast_component(componentField);
	EXPECT_NE(static_cast<cmzn_field_component *>(0), fieldComponent);
	EXPECT_EQ(2, cmzn_field_component_get_source_component_index(fieldComponent, 1));
	EXPECT_EQ(1, cmzn_field_component_get_source_component_index(fieldComponent, 2));
	cmzn_field_component_destroy(&fieldComponent);

	cmzn_field_id firstComponentField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "firstComponent");
	EXPECT_NE(static_cast<cmzn_field *>(0), firstComponentField);
	EXPECT_TRUE(cmzn_field_is_managed(firstComponentField));
	EXPECT_EQ(1, cmzn_field_get_number_of_source_fields(firstComponentField));
	EXPECT_EQ(1, cmzn_field_get_number_of_components(firstComponentField));
	temp = cmzn_field_get_source_field(firstComponentField, 1);
	EXPECT_EQ(temp, coordinatesField);
	cmzn_field_destroy(&temp);

	cmzn_field_id addField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "add");
	EXPECT_NE(static_cast<cmzn_field *>(0), addField);
	EXPECT_TRUE(cmzn_field_is_managed(addField));
	EXPECT_EQ(2, cmzn_field_get_number_of_source_fields(addField));
	EXPECT_EQ(1, cmzn_field_get_number_of_components(addField));
	temp = cmzn_field_get_source_field(addField, 1);
	EXPECT_EQ(temp, firstComponentField);
	cmzn_field_destroy(&temp);
	temp = cmzn_field_get_source_field(addField, 2);
	EXPECT_EQ(temp, temperatureField);
	cmzn_field_destroy(&temp);

	cmzn_field_id concatenateField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "concatenate");
	EXPECT_NE(static_cast<cmzn_field *>(0), concatenateField);
	EXPECT_TRUE(cmzn_field_is_managed(concatenateField));
	EXPECT_EQ(2, cmzn_field_get_number_of_source_fields(concatenateField));
	EXPECT_EQ(4, cmzn_field_get_number_of_components(concatenateField));
	temp = cmzn_field_get_source_field(concatenateField, 1);
	EXPECT_EQ(temp, coordinatesField);
	cmzn_field_destroy(&temp);
	temp = cmzn_field_get_source_field(concatenateField, 2);
	EXPECT_EQ(temp, temperatureField);
	cmzn_field_destroy(&temp);

	cmzn_field_destroy(&firstComponentField);
	cmzn_field_destroy(&addField);
	cmzn_field_destroy(&concatenateField);
	cmzn_field_destroy(&componentField);
	cmzn_field_destroy(&temperatureField);
	cmzn_field_destroy(&coordinatesField);
	cmzn_fieldmodule_destroy(&fieldmodule);

	cmzn_region_destroy(&tetrahedron_region);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
}
