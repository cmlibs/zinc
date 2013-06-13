
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/graphicsmaterial.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/graphicsmaterial.hpp"

TEST(Cmiss_graphics_material_module_api, valid_args)
{
	ZincTestSetup zinc;

	Cmiss_graphics_material_module_id materialmodule = Cmiss_graphics_module_get_material_module(zinc.gm);
	EXPECT_NE(static_cast<Cmiss_graphics_material_module *>(0), materialmodule);

	int result = Cmiss_graphics_material_module_begin_change(materialmodule);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_graphics_material_id material = Cmiss_graphics_material_module_create_material(materialmodule);
	EXPECT_NE(static_cast<Cmiss_graphics_material *>(0), material);

	result = Cmiss_graphics_material_set_name(material, "temp");
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_material_set_attribute_integer(material,
		CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_IS_MANAGED, 1);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_material_module_end_change(materialmodule);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_material_module_define_standard_materials(materialmodule);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_material_module_set_default_material(materialmodule, material);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_material_module_set_default_selected_material(materialmodule, material);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_graphics_material_destroy(&material);

	material = Cmiss_graphics_material_module_find_material_by_name(materialmodule, "temp");
	EXPECT_NE(static_cast<Cmiss_graphics_material *>(0), material);

	Cmiss_graphics_material_destroy(&material);

	material = Cmiss_graphics_material_module_get_default_material(materialmodule);
	EXPECT_NE(static_cast<Cmiss_graphics_material *>(0), material);

	Cmiss_graphics_material_destroy(&material);

	material = Cmiss_graphics_material_module_get_default_selected_material(materialmodule);
	EXPECT_NE(static_cast<Cmiss_graphics_material *>(0), material);

	Cmiss_graphics_material_destroy(&material);

	Cmiss_graphics_material_module_destroy(&materialmodule);
}

TEST(Cmiss_graphics_material_module_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsMaterialModule materialmodule = zinc.gm.getGraphicsMaterialModule();
	EXPECT_EQ(true, materialmodule.isValid());

	int result = materialmodule.beginChange();
	EXPECT_EQ(CMISS_OK, result);

	GraphicsMaterial material = materialmodule.createGraphicsMaterial();
	EXPECT_EQ(true, material.isValid());

	result = material.setName("temp");
	EXPECT_EQ(CMISS_OK, result);

	result = material.setAttributeInteger(
		material.ATTRIBUTE_IS_MANAGED, 1);
	EXPECT_EQ(CMISS_OK, result);

	result = materialmodule.endChange();
	EXPECT_EQ(CMISS_OK, result);

	result = materialmodule.defineStandardMaterials();
	EXPECT_EQ(CMISS_OK, result);

	result = materialmodule.setDefaultGraphicsMaterial(material);
	EXPECT_EQ(CMISS_OK, result);

	material = materialmodule.findGraphicsMaterialByName("temp");
	EXPECT_EQ(true, material.isValid());

	material = materialmodule.getDefaultGraphicsMaterial();
	EXPECT_EQ(true, material.isValid());

	material = materialmodule.getDefaultSelectedGraphicsMaterial();
	EXPECT_EQ(true, material.isValid());
}

TEST(Cmiss_graphics_material_api, valid_args)
{
	ZincTestSetup zinc;

	Cmiss_graphics_material_module_id materialmodule = Cmiss_graphics_module_get_material_module(zinc.gm);
	EXPECT_NE(static_cast<Cmiss_graphics_material_module *>(0), materialmodule);

	int result = Cmiss_graphics_material_module_begin_change(materialmodule);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_graphics_material_id material = Cmiss_graphics_material_module_create_material(materialmodule);
	EXPECT_NE(static_cast<Cmiss_graphics_material *>(0), material);

	result = Cmiss_graphics_material_set_name(material, "temp");
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_material_module_end_change(materialmodule);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_material_set_attribute_integer(material,
		CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_IS_MANAGED, 1);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_material_get_attribute_integer(material,
		CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_IS_MANAGED);
	EXPECT_EQ(1, result);

	double inValue = 0.7, outValue = 0.0;

	result = Cmiss_graphics_material_set_attribute_real(material,
		CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_ALPHA, inValue);
	EXPECT_EQ(CMISS_OK, result);

	outValue = Cmiss_graphics_material_get_attribute_real(material,
		CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_ALPHA);
	EXPECT_EQ(0.7, outValue);

	inValue = 1.0;

	result = Cmiss_graphics_material_set_attribute_real(material,
		CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_SHININESS, inValue);
	EXPECT_EQ(CMISS_OK, result);

	outValue = Cmiss_graphics_material_get_attribute_real(material,
		CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_SHININESS);
	EXPECT_EQ(1.0, outValue);

	double inValues[3], outValues[3];
	inValues[0] = 0.4;
	inValues[1] = 0.5;
	inValues[2] = 0.6;
	outValues[0] = 0.0;
	outValues[1] = 0.0;
	outValues[2] = 0.0;

	result = Cmiss_graphics_material_set_attribute_real3(material,
		CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_AMBIENT, &(inValues[0]));
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_material_get_attribute_real3(material,
		CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_AMBIENT, &outValues[0]);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_material_set_attribute_real3(material,
		CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_DIFFUSE, &inValues[0]);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_material_get_attribute_real3(material,
		CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_DIFFUSE, &outValues[0]);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_material_set_attribute_real3(material,
		CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_EMISSION, &inValues[0]);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_material_get_attribute_real3(material,
		CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_EMISSION, &outValues[0]);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_material_set_attribute_real3(material,
		CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_SPECULAR, &inValues[0]);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_material_get_attribute_real3(material,
		CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_SPECULAR, &outValues[0]);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_graphics_material_destroy(&material);

	Cmiss_graphics_material_module_destroy(&materialmodule);
}

TEST(Cmiss_graphics_material_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsMaterialModule materialmodule = zinc.gm.getGraphicsMaterialModule();
	EXPECT_EQ(true, materialmodule.isValid());

	int result = materialmodule.beginChange();
	EXPECT_EQ(CMISS_OK, result);

	GraphicsMaterial material = materialmodule.createGraphicsMaterial();
	EXPECT_EQ(true, material.isValid());

	result = material.setName("temp");
	EXPECT_EQ(CMISS_OK, result);

	result = materialmodule.endChange();
	EXPECT_EQ(CMISS_OK, result);

	result = material.setAttributeInteger(
		material.ATTRIBUTE_IS_MANAGED, 1);
	EXPECT_EQ(CMISS_OK, result);

	result = material.getAttributeInteger(
		material.ATTRIBUTE_IS_MANAGED);
	EXPECT_EQ(1, result);

	double inValue = 0.7, outValue = 0.0;

	result = material.setAttributeReal(
		material.ATTRIBUTE_ALPHA, inValue);
	EXPECT_EQ(CMISS_OK, result);

	outValue = material.getAttributeReal(
		material.ATTRIBUTE_ALPHA);
	EXPECT_EQ(0.7, outValue);

	inValue = 1.0;

	result = material.setAttributeReal(
		material.ATTRIBUTE_SHININESS, inValue);
	EXPECT_EQ(CMISS_OK, result);

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
	EXPECT_EQ(CMISS_OK, result);

	result = material.getAttributeReal3(
		material.ATTRIBUTE_AMBIENT, &outValues[0]);
	EXPECT_EQ(CMISS_OK, result);

	result = material.setAttributeReal3(
		material.ATTRIBUTE_DIFFUSE, &inValues[0]);
	EXPECT_EQ(CMISS_OK, result);

	result = material.getAttributeReal3(
		material.ATTRIBUTE_DIFFUSE, &outValues[0]);
	EXPECT_EQ(CMISS_OK, result);

	result = material.setAttributeReal3(
		material.ATTRIBUTE_EMISSION, &inValues[0]);
	EXPECT_EQ(CMISS_OK, result);

	result = material.getAttributeReal3(
		material.ATTRIBUTE_EMISSION, &outValues[0]);
	EXPECT_EQ(CMISS_OK, result);

	result = material.setAttributeReal3(
		material.ATTRIBUTE_SPECULAR, &inValues[0]);
	EXPECT_EQ(CMISS_OK, result);

	result = material.getAttributeReal3(
		material.ATTRIBUTE_SPECULAR, &outValues[0]);
	EXPECT_EQ(CMISS_OK, result);

}

