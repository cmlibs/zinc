/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/status.h>
#include <opencmiss/zinc/core.h>
#include <opencmiss/zinc/material.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "opencmiss/zinc/material.hpp"

TEST(cmzn_materialmodule_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_materialmodule_id materialmodule = cmzn_context_get_materialmodule(zinc.context);
	EXPECT_NE(static_cast<cmzn_materialmodule *>(0), materialmodule);

	int result = cmzn_materialmodule_begin_change(materialmodule);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_material_id material = cmzn_materialmodule_create_material(materialmodule);
	EXPECT_NE(static_cast<cmzn_material *>(0), material);

	result = cmzn_material_set_name(material, "temp");
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_material_set_managed(material, 1);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_materialmodule_end_change(materialmodule);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_materialmodule_define_standard_materials(materialmodule);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_materialmodule_set_default_material(materialmodule, material);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_materialmodule_set_default_selected_material(materialmodule, material);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_material_destroy(&material);

	material = cmzn_materialmodule_find_material_by_name(materialmodule, "temp");
	EXPECT_NE(static_cast<cmzn_material *>(0), material);

	cmzn_material_destroy(&material);

	material = cmzn_materialmodule_get_default_material(materialmodule);
	EXPECT_NE(static_cast<cmzn_material *>(0), material);

	cmzn_material_destroy(&material);

	material = cmzn_materialmodule_get_default_selected_material(materialmodule);
	EXPECT_NE(static_cast<cmzn_material *>(0), material);

	cmzn_material_destroy(&material);

	cmzn_materialmodule_destroy(&materialmodule);
}

TEST(cmzn_materialmodule_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	Materialmodule materialmodule = zinc.context.getMaterialmodule();
	EXPECT_TRUE(materialmodule.isValid());

	int result = materialmodule.beginChange();
	EXPECT_EQ(CMZN_OK, result);

	Material material = materialmodule.createMaterial();
	EXPECT_TRUE(material.isValid());

	result = material.setName("temp");
	EXPECT_EQ(CMZN_OK, result);

	result = material.setManaged(true);
	EXPECT_EQ(CMZN_OK, result);

	result = materialmodule.endChange();
	EXPECT_EQ(CMZN_OK, result);

	result = materialmodule.defineStandardMaterials();
	EXPECT_EQ(CMZN_OK, result);

	result = materialmodule.setDefaultMaterial(material);
	EXPECT_EQ(CMZN_OK, result);

	material = materialmodule.findMaterialByName("temp");
	EXPECT_TRUE(material.isValid());

	material = materialmodule.getDefaultMaterial();
	EXPECT_TRUE(material.isValid());

	material = materialmodule.getDefaultSelectedMaterial();
	EXPECT_TRUE(material.isValid());
}

TEST(cmzn_material_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_materialmodule_id materialmodule = cmzn_context_get_materialmodule(zinc.context);
	EXPECT_NE(static_cast<cmzn_materialmodule *>(0), materialmodule);

	int result = cmzn_materialmodule_begin_change(materialmodule);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_material_id material = cmzn_materialmodule_create_material(materialmodule);
	EXPECT_NE(static_cast<cmzn_material *>(0), material);

	result = cmzn_material_set_name(material, "temp");
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_materialmodule_end_change(materialmodule);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_material_set_managed(material, 1);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_material_is_managed(material);
	EXPECT_EQ(1, result);

	double inValue = 0.7, outValue = 0.0;

	result = cmzn_material_set_attribute_real(material,
		CMZN_MATERIAL_ATTRIBUTE_ALPHA, inValue);
	EXPECT_EQ(CMZN_OK, result);

	outValue = cmzn_material_get_attribute_real(material,
		CMZN_MATERIAL_ATTRIBUTE_ALPHA);
	EXPECT_EQ(0.7, outValue);

	inValue = 1.0;

	result = cmzn_material_set_attribute_real(material,
		CMZN_MATERIAL_ATTRIBUTE_SHININESS, inValue);
	EXPECT_EQ(CMZN_OK, result);

	outValue = cmzn_material_get_attribute_real(material,
		CMZN_MATERIAL_ATTRIBUTE_SHININESS);
	EXPECT_EQ(1.0, outValue);

	double inValues[3], outValues[3];
	inValues[0] = 0.4;
	inValues[1] = 0.5;
	inValues[2] = 0.6;
	outValues[0] = 0.0;
	outValues[1] = 0.0;
	outValues[2] = 0.0;

	result = cmzn_material_set_attribute_real3(material,
		CMZN_MATERIAL_ATTRIBUTE_AMBIENT, &(inValues[0]));
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_material_get_attribute_real3(material,
		CMZN_MATERIAL_ATTRIBUTE_AMBIENT, &outValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_material_set_attribute_real3(material,
		CMZN_MATERIAL_ATTRIBUTE_DIFFUSE, &inValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_material_get_attribute_real3(material,
		CMZN_MATERIAL_ATTRIBUTE_DIFFUSE, &outValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_material_set_attribute_real3(material,
		CMZN_MATERIAL_ATTRIBUTE_EMISSION, &inValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_material_get_attribute_real3(material,
		CMZN_MATERIAL_ATTRIBUTE_EMISSION, &outValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_material_set_attribute_real3(material,
		CMZN_MATERIAL_ATTRIBUTE_SPECULAR, &inValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_material_get_attribute_real3(material,
		CMZN_MATERIAL_ATTRIBUTE_SPECULAR, &outValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_material_destroy(&material);

	cmzn_materialmodule_destroy(&materialmodule);
}

TEST(cmzn_material_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	Materialmodule materialmodule = zinc.context.getMaterialmodule();
	EXPECT_TRUE(materialmodule.isValid());

	int result = materialmodule.beginChange();
	EXPECT_EQ(CMZN_OK, result);

	Material material = materialmodule.createMaterial();
	EXPECT_TRUE(material.isValid());

	result = material.setName("temp");
	EXPECT_EQ(CMZN_OK, result);

	result = materialmodule.endChange();
	EXPECT_EQ(CMZN_OK, result);

	result = material.setManaged(true);
	EXPECT_EQ(CMZN_OK, result);

	EXPECT_TRUE(material.isManaged());

	double inValue = 0.7, outValue = 0.0;

	result = material.setAttributeReal(
		material.ATTRIBUTE_ALPHA, inValue);
	EXPECT_EQ(CMZN_OK, result);

	outValue = material.getAttributeReal(
		material.ATTRIBUTE_ALPHA);
	EXPECT_EQ(0.7, outValue);

	inValue = 1.0;

	result = material.setAttributeReal(
		material.ATTRIBUTE_SHININESS, inValue);
	EXPECT_EQ(CMZN_OK, result);

	outValue = material.getAttributeReal(
		material.ATTRIBUTE_SHININESS);
	EXPECT_EQ(1.0, outValue);

	double inValues[3], outValues[3];
	inValues[0] = 0.4;
	inValues[1] = 0.5;
	inValues[2] = 0.6;
	outValues[0] = 0.0;
	outValues[1] = 0.0;
	outValues[2] = 0.0;

	result = material.setAttributeReal3(
		material.ATTRIBUTE_AMBIENT, &inValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = material.getAttributeReal3(
		material.ATTRIBUTE_AMBIENT, &outValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = material.setAttributeReal3(
		material.ATTRIBUTE_DIFFUSE, &inValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = material.getAttributeReal3(
		material.ATTRIBUTE_DIFFUSE, &outValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = material.setAttributeReal3(
		material.ATTRIBUTE_EMISSION, &inValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = material.getAttributeReal3(
		material.ATTRIBUTE_EMISSION, &outValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = material.setAttributeReal3(
		material.ATTRIBUTE_SPECULAR, &inValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = material.getAttributeReal3(
		material.ATTRIBUTE_SPECULAR, &outValues[0]);
	EXPECT_EQ(CMZN_OK, result);

}

TEST(ZincMaterial, unmanage)
{
	ZincTestSetupCpp zinc;

	Materialmodule materialmodule = zinc.context.getMaterialmodule();
	EXPECT_TRUE(materialmodule.isValid());

	Material material = materialmodule.createMaterial();
	EXPECT_TRUE(material.isValid());

	const char name[] = "temp";
	int result;
	EXPECT_EQ(CMZN_OK, result = material.setName(name));
	EXPECT_EQ(CMZN_OK, result = material.setManaged(true));

	// clear material so no local reference held
	material = Material();
	EXPECT_FALSE(material.isValid());

	material = materialmodule.findMaterialByName(name);
	EXPECT_TRUE(material.isValid());
	EXPECT_EQ(CMZN_OK, result = material.setManaged(false));
	material = Material();

	// material should be removed
	material = materialmodule.findMaterialByName(name);
	EXPECT_FALSE(material.isValid());
}

TEST(ZincMaterialiterator, iteration)
{
	ZincTestSetupCpp zinc;

	Materialmodule materialmodule = zinc.context.getMaterialmodule();
	EXPECT_TRUE(materialmodule.isValid());

	Material xxx = materialmodule.createMaterial();
	EXPECT_TRUE(xxx.isValid());
	EXPECT_EQ(OK, xxx.setName("xxx"));

	Material zzz = materialmodule.createMaterial();
	EXPECT_TRUE(zzz.isValid());
	EXPECT_EQ(OK, zzz.setName("zzz"));

	Material aaa = materialmodule.createMaterial();
	EXPECT_TRUE(aaa.isValid());
	EXPECT_EQ(OK, aaa.setName("aaa"));

	Material defaultMaterial = materialmodule.getDefaultMaterial();
	EXPECT_TRUE(defaultMaterial.isValid());
	Material defaultSelectedMaterial = materialmodule.getDefaultSelectedMaterial();
	EXPECT_TRUE(defaultSelectedMaterial.isValid());

	Materialiterator iter = materialmodule.createMaterialiterator();
	EXPECT_TRUE(iter.isValid());
	Material m;
	EXPECT_EQ(aaa, m = iter.next());
	EXPECT_EQ(defaultMaterial, m = iter.next());
	EXPECT_EQ(defaultSelectedMaterial, m = iter.next());
	EXPECT_EQ(xxx, m = iter.next());
	EXPECT_EQ(zzz, m = iter.next());
	EXPECT_FALSE((m = iter.next()).isValid());
}
