/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/core.h>
#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldarithmeticoperators.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldfiniteelement.hpp>
#include <opencmiss/zinc/fieldgroup.hpp>
#include <opencmiss/zinc/streamregion.hpp>

#include "utilities/testenum.hpp"
#include "zinctestsetupcpp.hpp"
#include "test_resources.h"
#include <cmath>

TEST(ZincField, CoordinateSystemTypeEnum)
{
	const char *enumNames[8] = { nullptr, "RECTANGULAR_CARTESIAN", "CYLINDRICAL_POLAR",
		"SPHERICAL_POLAR", "PROLATE_SPHEROIDAL", "OBLATE_SPHEROIDAL", "FIBRE", "NOT_APPLICABLE" };
	testEnum(8, enumNames, Field::CoordinateSystemTypeEnumToString, Field::CoordinateSystemTypeEnumFromString);
}

TEST(ZincField, DomainTypeEnum)
{
	const char *enumNames[8] = { nullptr, "POINT", "NODES", "DATAPOINTS",
		"MESH1D", "MESH2D", "MESH3D", "MESH_HIGHEST_DIMENSION" };
	testEnumBitShift(8, enumNames, Field::DomainTypeEnumToString, Field::DomainTypeEnumFromString);
}

TEST(ZincField, automaticRenaming)
{
	ZincTestSetupCpp zinc;

	// test can rename a field to same name as another field if it has an
	// automatic name, which is changed to make way for the new field
	const double valuesIn1a[3] = { 1.0, 4.0, 9.0 };
	FieldConstant fieldConstant1a = zinc.fm.createFieldConstant(3, valuesIn1a);
	EXPECT_TRUE(fieldConstant1a.isValid());
	char *name = fieldConstant1a.getName();
	EXPECT_STREQ("temp1", name);
	cmzn_deallocate(name);
	FieldSqrt fieldSqrta = zinc.fm.createFieldSqrt(fieldConstant1a);
	EXPECT_TRUE(fieldSqrta.isValid());
	name = fieldSqrta.getName();
	EXPECT_STREQ("temp2", name);
	cmzn_deallocate(name);
	// explicitly set name temp1 so no longer automatic
	EXPECT_EQ(RESULT_OK, fieldSqrta.setName("temp1"));
	name = fieldSqrta.getName();
	EXPECT_STREQ("temp1", name);
	cmzn_deallocate(name);
	// fieldConstant1a had an automatic name so will be named something else
	name = fieldConstant1a.getName();
	EXPECT_STREQ("temp3", name);
	cmzn_deallocate(name);
	// can't reclaim original name as field of that name is not using an automatic name
	EXPECT_EQ(RESULT_ERROR_ALREADY_EXISTS, fieldConstant1a.setName("temp1"));
	name = fieldConstant1a.getName();
	EXPECT_STREQ("temp3", name);
	cmzn_deallocate(name);

	// test overriding when reading description
	char *description = zinc.fm.writeDescription();
	Region regionb = zinc.context.createRegion();
	Fieldmodule fmb = regionb.getFieldmodule();
	EXPECT_TRUE(fmb.isValid());
	const double valuesIn2b[3] = { 0.5, 1.6, 64.0 };
	FieldConstant fieldConstant2b = fmb.createFieldConstant(3, valuesIn2b);
	EXPECT_TRUE(fieldConstant2b.isValid());
	name = fieldConstant2b.getName();
	EXPECT_STREQ("temp1", name);
	cmzn_deallocate(name);
	EXPECT_EQ(RESULT_OK, fmb.readDescription(description));
	// test automatically-named field has been renamed
	name = fieldConstant2b.getName();
	EXPECT_STREQ("temp5", name);  // temp5 as temp4 is in use by field that will be renamed temp1
	cmzn_deallocate(name);
	Field fieldSqrtb = fmb.findFieldByName("temp1");
	FieldConstant fieldConstant1b = fmb.findFieldByName("temp3").castConstant();
	EXPECT_EQ(fieldConstant1b, fieldSqrtb.getSourceField(1));
	Fieldcache fieldcacheb = fmb.createFieldcache();
	double valuesOut[3];
	const double TOL = 1.0E-12;
	EXPECT_EQ(RESULT_OK, fieldConstant1b.evaluateReal(fieldcacheb, 3, valuesOut));
	for (int c = 0; c < 3; ++c)
		EXPECT_NEAR(valuesIn1a[c], valuesOut[c], TOL);
	EXPECT_EQ(RESULT_OK, fieldSqrtb.evaluateReal(fieldcacheb, 3, valuesOut));
	for (int c = 0; c < 3; ++c)
		EXPECT_NEAR(sqrt(valuesIn1a[c]), valuesOut[c], TOL);
	cmzn_deallocate(description);
}

TEST(ZincField, automaticRenamingFiniteElement)
{
	ZincTestSetupCpp zinc;

	// test can load a finite element field with same name as an automatically
	// named field by renaming it
	FieldFiniteElement temp1 = zinc.fm.createFieldFiniteElement(3);
	EXPECT_TRUE(temp1.isValid());
	char *name = temp1.getName();
	EXPECT_STREQ("temp1", name);
	cmzn_deallocate(name);
	EXPECT_EQ(RESULT_OK, temp1.setTypeCoordinate(true));
	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	Nodetemplate nodetemplate = nodes.createNodetemplate();
	EXPECT_EQ(RESULT_OK, nodetemplate.defineField(temp1));
	Node node1 = nodes.createNode(1, nodetemplate);
	Fieldcache fieldcache = zinc.fm.createFieldcache();
	EXPECT_TRUE(fieldcache.isValid());
	EXPECT_EQ(RESULT_OK, fieldcache.setNode(node1));
	const double coordinatesIn[3] = { 1.1, 2.2, 3.3 };
	EXPECT_EQ(RESULT_OK, temp1.assignReal(fieldcache, 3, coordinatesIn));
	FieldGroup bob = zinc.fm.createFieldGroup();
	EXPECT_TRUE(bob.isValid());
	EXPECT_EQ(RESULT_OK, bob.setName("bob"));
	//Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);

	StreaminformationRegion sir = zinc.root_region.createStreaminformationRegion();
	StreamresourceMemory srm = sir.createStreamresourceMemory();
	EXPECT_TRUE(srm.isValid());
	EXPECT_EQ(RESULT_OK, zinc.root_region.write(sir));
	const void *buffer = nullptr;
	unsigned int bufferLength = 0;
	EXPECT_EQ(RESULT_OK, srm.getBuffer(&buffer, &bufferLength));
	//const char *bufferText = static_cast<const char *>(buffer);

	Region regionb = zinc.context.createRegion();
	Fieldmodule fmb = regionb.getFieldmodule();
	EXPECT_TRUE(fmb.isValid());
	const double valuesInb[2] = { 0.5, 1.6 };
	FieldConstant fieldConstantb = fmb.createFieldConstant(2, valuesInb);
	EXPECT_TRUE(fieldConstantb.isValid());
	name = fieldConstantb.getName();
	EXPECT_STREQ("temp1", name);
	cmzn_deallocate(name);
	StreaminformationRegion sirb = regionb.createStreaminformationRegion();
	StreamresourceMemory srmb = sirb.createStreamresourceMemoryBuffer(buffer, bufferLength);
	EXPECT_TRUE(srmb.isValid());
	EXPECT_EQ(RESULT_OK, regionb.read(sirb));
	// test automatically-named field has been renamed
	name = fieldConstantb.getName();
	EXPECT_STREQ("temp2", name);
	cmzn_deallocate(name);
	FieldFiniteElement temp1b = fmb.findFieldByName("temp1").castFiniteElement();
	EXPECT_TRUE(temp1b.isValid());
	FieldGroup bobb = fmb.findFieldByName("bob").castGroup();
	EXPECT_TRUE(bob.isValid());

	// test can't read model if an incompatible field is using a name from the model
	EXPECT_EQ(RESULT_OK, temp1b.setName("coordinates"));
	EXPECT_EQ(RESULT_OK, fieldConstantb.setName("temp1"));
	EXPECT_EQ(RESULT_ERROR_INCOMPATIBLE_DATA, regionb.read(sirb));
	EXPECT_EQ(RESULT_OK, bobb.setName("fred"));
	EXPECT_EQ(RESULT_OK, fieldConstantb.setName("bob"));
	EXPECT_EQ(RESULT_ERROR_INCOMPATIBLE_DATA, regionb.read(sirb));
}

// test can only have NOT_APPLICABLE coordinate system for a non-numeric value type field
TEST(ZincField, nonNumericFieldCoordinateSystem)
{
	ZincTestSetupCpp zinc;

	FieldStringConstant stringConstant = zinc.fm.createFieldStringConstant("string_constant");
	EXPECT_TRUE(stringConstant.isValid());
	EXPECT_EQ(RESULT_OK, stringConstant.setName("string_constant"));
	EXPECT_EQ(Field::COORDINATE_SYSTEM_TYPE_NOT_APPLICABLE, stringConstant.getCoordinateSystemType());
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, stringConstant.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN));

	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	FieldStoredMeshLocation storedMeshLocation = zinc.fm.createFieldStoredMeshLocation(mesh3d);
	EXPECT_TRUE(storedMeshLocation.isValid());
	// set name to force merge later
	EXPECT_EQ(RESULT_OK, storedMeshLocation.setName("stored_mesh_location"));
	EXPECT_EQ(Field::COORDINATE_SYSTEM_TYPE_NOT_APPLICABLE, storedMeshLocation.getCoordinateSystemType());
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, storedMeshLocation.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN));

	FieldStoredString storedString = zinc.fm.createFieldStoredString();
	EXPECT_TRUE(storedString.isValid());
	// set name to force merge later
	EXPECT_EQ(RESULT_OK, storedString.setName("stored_string"));
	EXPECT_EQ(Field::COORDINATE_SYSTEM_TYPE_NOT_APPLICABLE, storedString.getCoordinateSystemType());
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, storedString.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN));

	const double zeroValues[3] = { 0.0, 0.0, 0.0 };
	FieldConstant zero = zinc.fm.createFieldConstant(3, zeroValues);
	EXPECT_TRUE(zero.isValid());
	FieldFindMeshLocation findMeshLocation = zinc.fm.createFieldFindMeshLocation(zero, zero, mesh3d);
	EXPECT_TRUE(findMeshLocation.isValid());
	EXPECT_EQ(RESULT_OK, findMeshLocation.setName("find_mesh_location"));
	EXPECT_EQ(Field::COORDINATE_SYSTEM_TYPE_NOT_APPLICABLE, findMeshLocation.getCoordinateSystemType());
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, findMeshLocation.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN));

	// check old EX files specifying coordinate system other than NON_APPLICABLE for non-numeric fields
	// can be read, but these field are read as coordinate system type NON_APPLICABLE
	Elementtemplate elementtemplate = mesh3d.createElementtemplate();
	EXPECT_EQ(RESULT_OK, elementtemplate.setElementShapeType(Element::SHAPE_TYPE_CUBE));
	Element element1 = mesh3d.createElement(1, elementtemplate);
	EXPECT_TRUE(element1.isValid());
	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_EX2_NON_NUMERIC_COORDINATE_SYSTEM_RESOURCE)));
	storedMeshLocation = zinc.fm.findFieldByName("stored_mesh_location").castStoredMeshLocation();
	EXPECT_TRUE(storedMeshLocation.isValid());
	EXPECT_EQ(Field::COORDINATE_SYSTEM_TYPE_NOT_APPLICABLE, storedMeshLocation.getCoordinateSystemType());

	storedString = zinc.fm.findFieldByName("stored_string").castStoredString();
	EXPECT_TRUE(storedString.isValid());
	EXPECT_EQ(Field::COORDINATE_SYSTEM_TYPE_NOT_APPLICABLE, storedString.getCoordinateSystemType());
}
