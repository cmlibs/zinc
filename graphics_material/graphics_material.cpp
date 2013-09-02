
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/graphicsmaterial.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/graphicsmaterial.hpp"

TEST(cmzn_graphics_material_module_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_graphics_material_module_id materialmodule = cmzn_graphics_module_get_material_module(zinc.gm);
	EXPECT_NE(static_cast<cmzn_graphics_material_module *>(0), materialmodule);

	int result = cmzn_graphics_material_module_begin_change(materialmodule);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_graphics_material_id material = cmzn_graphics_material_module_create_material(materialmodule);
	EXPECT_NE(static_cast<cmzn_graphics_material *>(0), material);

	result = cmzn_graphics_material_set_name(material, "temp");
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_graphics_material_set_managed(material, 1);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_graphics_material_module_end_change(materialmodule);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_graphics_material_module_define_standard_materials(materialmodule);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_graphics_material_module_set_default_material(materialmodule, material);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_graphics_material_module_set_default_selected_material(materialmodule, material);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_graphics_material_destroy(&material);

	material = cmzn_graphics_material_module_find_material_by_name(materialmodule, "temp");
	EXPECT_NE(static_cast<cmzn_graphics_material *>(0), material);

	cmzn_graphics_material_destroy(&material);

	material = cmzn_graphics_material_module_get_default_material(materialmodule);
	EXPECT_NE(static_cast<cmzn_graphics_material *>(0), material);

	cmzn_graphics_material_destroy(&material);

	material = cmzn_graphics_material_module_get_default_selected_material(materialmodule);
	EXPECT_NE(static_cast<cmzn_graphics_material *>(0), material);

	cmzn_graphics_material_destroy(&material);

	cmzn_graphics_material_module_destroy(&materialmodule);
}

TEST(cmzn_graphics_material_module_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsMaterialModule materialmodule = zinc.gm.getMaterialModule();
	EXPECT_TRUE(materialmodule.isValid());

	int result = materialmodule.beginChange();
	EXPECT_EQ(CMZN_OK, result);

	GraphicsMaterial material = materialmodule.createMaterial();
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

TEST(cmzn_graphics_material_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_graphics_material_module_id materialmodule = cmzn_graphics_module_get_material_module(zinc.gm);
	EXPECT_NE(static_cast<cmzn_graphics_material_module *>(0), materialmodule);

	int result = cmzn_graphics_material_module_begin_change(materialmodule);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_graphics_material_id material = cmzn_graphics_material_module_create_material(materialmodule);
	EXPECT_NE(static_cast<cmzn_graphics_material *>(0), material);

	result = cmzn_graphics_material_set_name(material, "temp");
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_graphics_material_module_end_change(materialmodule);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_graphics_material_set_managed(material, 1);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_graphics_material_is_managed(material);
	EXPECT_EQ(1, result);

	double inValue = 0.7, outValue = 0.0;

	result = cmzn_graphics_material_set_attribute_real(material,
		CMZN_GRAPHICS_MATERIAL_ATTRIBUTE_ALPHA, inValue);
	EXPECT_EQ(CMZN_OK, result);

	outValue = cmzn_graphics_material_get_attribute_real(material,
		CMZN_GRAPHICS_MATERIAL_ATTRIBUTE_ALPHA);
	EXPECT_EQ(0.7, outValue);

	inValue = 1.0;

	result = cmzn_graphics_material_set_attribute_real(material,
		CMZN_GRAPHICS_MATERIAL_ATTRIBUTE_SHININESS, inValue);
	EXPECT_EQ(CMZN_OK, result);

	outValue = cmzn_graphics_material_get_attribute_real(material,
		CMZN_GRAPHICS_MATERIAL_ATTRIBUTE_SHININESS);
	EXPECT_EQ(1.0, outValue);

	double inValues[3], outValues[3];
	inValues[0] = 0.4;
	inValues[1] = 0.5;
	inValues[2] = 0.6;
	outValues[0] = 0.0;
	outValues[1] = 0.0;
	outValues[2] = 0.0;

	result = cmzn_graphics_material_set_attribute_real3(material,
		CMZN_GRAPHICS_MATERIAL_ATTRIBUTE_AMBIENT, &(inValues[0]));
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_graphics_material_get_attribute_real3(material,
		CMZN_GRAPHICS_MATERIAL_ATTRIBUTE_AMBIENT, &outValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_graphics_material_set_attribute_real3(material,
		CMZN_GRAPHICS_MATERIAL_ATTRIBUTE_DIFFUSE, &inValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_graphics_material_get_attribute_real3(material,
		CMZN_GRAPHICS_MATERIAL_ATTRIBUTE_DIFFUSE, &outValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_graphics_material_set_attribute_real3(material,
		CMZN_GRAPHICS_MATERIAL_ATTRIBUTE_EMISSION, &inValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_graphics_material_get_attribute_real3(material,
		CMZN_GRAPHICS_MATERIAL_ATTRIBUTE_EMISSION, &outValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_graphics_material_set_attribute_real3(material,
		CMZN_GRAPHICS_MATERIAL_ATTRIBUTE_SPECULAR, &inValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_graphics_material_get_attribute_real3(material,
		CMZN_GRAPHICS_MATERIAL_ATTRIBUTE_SPECULAR, &outValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_graphics_material_destroy(&material);

	cmzn_graphics_material_module_destroy(&materialmodule);
}

TEST(cmzn_graphics_material_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsMaterialModule materialmodule = zinc.gm.getMaterialModule();
	EXPECT_TRUE(materialmodule.isValid());

	int result = materialmodule.beginChange();
	EXPECT_EQ(CMZN_OK, result);

	GraphicsMaterial material = materialmodule.createMaterial();
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

