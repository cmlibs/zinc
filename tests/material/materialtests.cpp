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
#include "opencmiss/zinc/changemanager.hpp"
#include "opencmiss/zinc/fieldimage.hpp"
#include "opencmiss/zinc/material.hpp"

#include "test_resources.h"

namespace {

int getNumberOfMaterials(Materialmodule& materialmodule)
{
	int count = 0;
	Materialiterator iter = materialmodule.createMaterialiterator();
	Material material;
	while ((material = iter.next()).isValid())
		++count;
	return count;
}

}

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

	cmzn_context_id context = cmzn_materialmodule_get_context(materialmodule);
	EXPECT_EQ(zinc.context, context);
	cmzn_context_destroy(&context);

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

	Context context = materialmodule.getContext();
	EXPECT_EQ(zinc.context, context);

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

TEST(ZincMaterialmodule, defaultMaterialsAndGraphics)
{
	ZincTestSetupCpp zinc;
	Material tmpMaterial;

	Materialmodule materialmodule = zinc.context.getMaterialmodule();
	EXPECT_TRUE(materialmodule.isValid());

	EXPECT_EQ(2, getNumberOfMaterials(materialmodule));

	Material defaultMaterial = materialmodule.getDefaultMaterial();
	EXPECT_TRUE(defaultMaterial.isValid());
	tmpMaterial = materialmodule.findMaterialByName("default");
	EXPECT_EQ(tmpMaterial, defaultMaterial);

	Material defaultSelectedMaterial = materialmodule.getDefaultSelectedMaterial();
	EXPECT_TRUE(defaultSelectedMaterial.isValid());
	tmpMaterial = materialmodule.findMaterialByName("default_selected");
	EXPECT_EQ(tmpMaterial, defaultSelectedMaterial);

	Material defaultSurfaceMaterial = materialmodule.getDefaultSurfaceMaterial();
	EXPECT_FALSE(defaultSurfaceMaterial.isValid());
	EXPECT_EQ(RESULT_OK, materialmodule.defineStandardMaterials());
	Material whiteMaterial = materialmodule.findMaterialByName("white");
	EXPECT_TRUE(whiteMaterial.isValid());
	EXPECT_EQ(RESULT_OK, materialmodule.setDefaultSurfaceMaterial(whiteMaterial));
	defaultSurfaceMaterial = materialmodule.getDefaultSurfaceMaterial();
	EXPECT_EQ(whiteMaterial, defaultSurfaceMaterial);

	EXPECT_EQ(20, getNumberOfMaterials(materialmodule));

	GraphicsContours contours = zinc.scene.createGraphicsContours();
	EXPECT_TRUE(contours.isValid());
	tmpMaterial = contours.getMaterial();
	EXPECT_EQ(defaultSurfaceMaterial, tmpMaterial);
	tmpMaterial = contours.getSelectedMaterial();
	EXPECT_EQ(defaultSelectedMaterial, tmpMaterial);

	GraphicsLines lines = zinc.scene.createGraphicsLines();
	EXPECT_TRUE(lines.isValid());
	tmpMaterial = lines.getMaterial();
	EXPECT_EQ(defaultMaterial, tmpMaterial);
	tmpMaterial = lines.getSelectedMaterial();
	EXPECT_EQ(defaultSelectedMaterial, tmpMaterial);

	GraphicsPoints points = zinc.scene.createGraphicsPoints();
	EXPECT_TRUE(points.isValid());
	tmpMaterial = points.getMaterial();
	EXPECT_EQ(defaultMaterial, tmpMaterial);
	tmpMaterial = points.getSelectedMaterial();
	EXPECT_EQ(defaultSelectedMaterial, tmpMaterial);

	GraphicsStreamlines streamlines = zinc.scene.createGraphicsStreamlines();
	EXPECT_TRUE(streamlines.isValid());
	tmpMaterial = streamlines.getMaterial();
	EXPECT_EQ(defaultMaterial, tmpMaterial);
	tmpMaterial = streamlines.getSelectedMaterial();
	EXPECT_EQ(defaultSelectedMaterial, tmpMaterial);

	GraphicsSurfaces surfaces = zinc.scene.createGraphicsSurfaces();
	EXPECT_TRUE(surfaces.isValid());
	tmpMaterial = surfaces.getMaterial();
	EXPECT_EQ(defaultSurfaceMaterial, tmpMaterial);
	tmpMaterial = surfaces.getSelectedMaterial();
	EXPECT_EQ(defaultSelectedMaterial, tmpMaterial);
}

TEST(ZincMaterialmodule, writeReadDescription)
{
	ZincTestSetupCpp zinc;

	// create image fields in root and child region for material textures
	FieldImage image1 = zinc.fm.createFieldImage();
	EXPECT_TRUE(image1.isValid());
	EXPECT_EQ(RESULT_OK, image1.setName("image_blockcolours"));
	EXPECT_EQ(RESULT_OK, image1.readFile(TestResources::getLocation(TestResources::FIELDIMAGE_BLOCKCOLOURS_RESOURCE)));
	Region child = zinc.root_region.createChild("child");
	EXPECT_TRUE(child.isValid());
	Fieldmodule childFm = child.getFieldmodule();
	FieldImage image2 = childFm.createFieldImage();
	EXPECT_TRUE(image2.isValid());
	EXPECT_EQ(RESULT_OK, image2.setName("image_gray"));
	EXPECT_EQ(RESULT_OK, image2.readFile(TestResources::getLocation(TestResources::TESTIMAGE_GRAY_JPG_RESOURCE)));

	Materialmodule materialmodule = zinc.context.getMaterialmodule();
	EXPECT_TRUE(materialmodule.isValid());
	EXPECT_TRUE(materialmodule.findMaterialByName("default").isValid());
	EXPECT_FALSE(materialmodule.findMaterialByName("green").isValid());
	EXPECT_EQ(2, getNumberOfMaterials(materialmodule));

	EXPECT_EQ(RESULT_OK, materialmodule.defineStandardMaterials());
	Material green = materialmodule.findMaterialByName("green");
	EXPECT_TRUE(materialmodule.findMaterialByName("green").isValid());
	EXPECT_EQ(20, getNumberOfMaterials(materialmodule));

	// create and serialise a bogus material with different parameters for all attributes to test for mismatches
	Material bogusMaterial;
	const double bogusAmbient[3]  = { 0.11, 0.22, 0.33 };
	const double bogusDiffuse[3]  = { 0.44, 0.55, 0.66 };
	const double bogusEmission[3] = { 0.77, 0.88, 0.99 };
	const double bogusSpecular[3] = { 0.15, 0.24, 0.33 };
	const double bogusAlpha = 0.678;
	const double bogusShininess = 0.345;
	{
		ChangeManager<Materialmodule> changeMaterial(materialmodule);
		bogusMaterial = materialmodule.createMaterial();
		EXPECT_EQ(RESULT_OK, bogusMaterial.setName("bogus"));
		EXPECT_EQ(RESULT_OK, bogusMaterial.setAttributeReal3(Material::ATTRIBUTE_AMBIENT, bogusAmbient));
		EXPECT_EQ(RESULT_OK, bogusMaterial.setAttributeReal3(Material::ATTRIBUTE_DIFFUSE, bogusDiffuse));
		EXPECT_EQ(RESULT_OK, bogusMaterial.setAttributeReal3(Material::ATTRIBUTE_EMISSION, bogusEmission));
		EXPECT_EQ(RESULT_OK, bogusMaterial.setAttributeReal3(Material::ATTRIBUTE_SPECULAR, bogusSpecular));
		EXPECT_EQ(RESULT_OK, bogusMaterial.setAttributeReal(Material::ATTRIBUTE_ALPHA, bogusAlpha));
		EXPECT_EQ(RESULT_OK, bogusMaterial.setAttributeReal(Material::ATTRIBUTE_SHININESS, bogusShininess));
		EXPECT_EQ(RESULT_OK, bogusMaterial.setManaged(true));
		EXPECT_EQ(RESULT_OK, bogusMaterial.setTextureField(1, image1));
		EXPECT_EQ(RESULT_OK, bogusMaterial.setTextureField(3, image2));
	}
	EXPECT_EQ(21, getNumberOfMaterials(materialmodule));

	char* jsonString = materialmodule.writeDescription();
	EXPECT_NE(nullptr, jsonString);

	Context context2("test2");
	Materialmodule materialmodule2 = context2.getMaterialmodule();
	EXPECT_EQ(2, getNumberOfMaterials(materialmodule2));
	Region rootRegion2 = context2.getDefaultRegion();
	Fieldmodule fm2 = rootRegion2.getFieldmodule();
	// create image fields in root and child region for material textures - to find in readDescription
	image1 = fm2.createFieldImage();
	EXPECT_TRUE(image1.isValid());
	EXPECT_EQ(RESULT_OK, image1.setName("image_blockcolours"));
	EXPECT_EQ(RESULT_OK, image1.readFile(TestResources::getLocation(TestResources::FIELDIMAGE_BLOCKCOLOURS_RESOURCE)));
	Region child2 = rootRegion2.createChild("child");
	EXPECT_TRUE(child2.isValid());
	Fieldmodule childFm2 = child2.getFieldmodule();
	image2 = childFm2.createFieldImage();
	EXPECT_TRUE(image2.isValid());
	EXPECT_EQ(RESULT_OK, image2.setName("image_gray"));
	EXPECT_EQ(RESULT_OK, image2.readFile(TestResources::getLocation(TestResources::TESTIMAGE_GRAY_JPG_RESOURCE)));

	EXPECT_EQ(RESULT_OK, materialmodule2.readDescription(jsonString));
	EXPECT_EQ(21, getNumberOfMaterials(materialmodule2));

	char* jsonString2 = materialmodule2.writeDescription();
	EXPECT_STREQ(jsonString, jsonString2);

	// test bogus material correctly serialised
	Material bogusMaterial2 = materialmodule2.findMaterialByName("bogus");
	EXPECT_TRUE(bogusMaterial2.isValid());
	double ambient[3], diffuse[3], emission[3], specular[3];
	EXPECT_EQ(RESULT_OK, bogusMaterial.getAttributeReal3(Material::ATTRIBUTE_AMBIENT, ambient));
	EXPECT_EQ(RESULT_OK, bogusMaterial.getAttributeReal3(Material::ATTRIBUTE_DIFFUSE, diffuse));
	EXPECT_EQ(RESULT_OK, bogusMaterial.getAttributeReal3(Material::ATTRIBUTE_EMISSION, emission));
	EXPECT_EQ(RESULT_OK, bogusMaterial.getAttributeReal3(Material::ATTRIBUTE_SPECULAR, specular));
	for (int i = 0; i < 3; ++i)
	{
		EXPECT_DOUBLE_EQ(bogusAmbient[i], ambient[i]);
		EXPECT_DOUBLE_EQ(bogusDiffuse[i], diffuse[i]);
		EXPECT_DOUBLE_EQ(bogusEmission[i], emission[i]);
		EXPECT_DOUBLE_EQ(bogusSpecular[i], specular[i]);
	}
	EXPECT_DOUBLE_EQ(bogusAlpha, bogusMaterial.getAttributeReal(Material::ATTRIBUTE_ALPHA));
	EXPECT_DOUBLE_EQ(bogusShininess, bogusMaterial.getAttributeReal(Material::ATTRIBUTE_SHININESS));

	// test reading without finding the subregion or image
	Context context3("test3");
	Materialmodule materialmodule3 = context3.getMaterialmodule();
	EXPECT_EQ(RESULT_ERROR_NOT_FOUND, materialmodule3.readDescription(jsonString));

	// test reading without a Default region
	Context context4("test4");
	EXPECT_EQ(RESULT_OK, context4.setDefaultRegion(Region()));
	Materialmodule materialmodule4 = context4.getMaterialmodule();
	EXPECT_EQ(RESULT_ERROR_NOT_FOUND, materialmodule4.readDescription(jsonString));
}

TEST(ZincMaterialiterator, iteration)
{
	ZincTestSetupCpp zinc;

	Materialmodule materialmodule = zinc.context.getMaterialmodule();
	EXPECT_TRUE(materialmodule.isValid());

	Material xxx = materialmodule.createMaterial();
	EXPECT_TRUE(xxx.isValid());
	EXPECT_EQ(RESULT_OK, xxx.setName("xxx"));

	Material zzz = materialmodule.createMaterial();
	EXPECT_TRUE(zzz.isValid());
	EXPECT_EQ(RESULT_OK, zzz.setName("zzz"));

	Material aaa = materialmodule.createMaterial();
	EXPECT_TRUE(aaa.isValid());
	EXPECT_EQ(RESULT_OK, aaa.setName("aaa"));

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

class MaterialmodulecallbackRecordChange : public Materialmodulecallback
{
	Materialmoduleevent lastMaterialmoduleevent;

	virtual void operator()(const Materialmoduleevent &materialmoduleevent)
	{
		this->lastMaterialmoduleevent = materialmoduleevent;
	}

public:
	MaterialmodulecallbackRecordChange() :
		Materialmodulecallback()
	{ }

	void clearLastEvent()
	{
		this->lastMaterialmoduleevent = Materialmoduleevent();
	}

	const Materialmoduleevent &getLastEvent() const
	{
		return this->lastMaterialmoduleevent;
	}
};

TEST(ZincMaterialmodulenotifier, changeCallback)
{
	ZincTestSetupCpp zinc;
	int result;

	Materialmodule mm = zinc.context.getMaterialmodule();
	EXPECT_TRUE(mm.isValid());

	Materialmodulenotifier materialmodulenotifier = mm.createMaterialmodulenotifier();
	EXPECT_TRUE(materialmodulenotifier.isValid());

	const char *materialName = "bob";
	MaterialmodulecallbackRecordChange recordChange;
	EXPECT_EQ(CMZN_OK, materialmodulenotifier.setCallback(recordChange));

	Material material = mm.createMaterial();
	EXPECT_TRUE(material.isValid());
	result = recordChange.getLastEvent().getSummaryMaterialChangeFlags();
	EXPECT_EQ(Material::CHANGE_FLAG_ADD, result);
	result = recordChange.getLastEvent().getMaterialChangeFlags(material);
	EXPECT_EQ(Material::CHANGE_FLAG_ADD, result);
	recordChange.clearLastEvent();

	EXPECT_EQ(CMZN_OK, material.setName(materialName));
	result = recordChange.getLastEvent().getSummaryMaterialChangeFlags();
	EXPECT_EQ(Material::CHANGE_FLAG_IDENTIFIER, result);
	result = recordChange.getLastEvent().getMaterialChangeFlags(material);
	EXPECT_EQ(Material::CHANGE_FLAG_IDENTIFIER, result);
	recordChange.clearLastEvent();

	EXPECT_EQ(CMZN_OK, material.setAttributeReal(Material::ATTRIBUTE_ALPHA, 0.5));
	result = recordChange.getLastEvent().getSummaryMaterialChangeFlags();
	EXPECT_EQ(Material::CHANGE_FLAG_DEFINITION | Material::CHANGE_FLAG_FULL_RESULT, result);
	result = recordChange.getLastEvent().getMaterialChangeFlags(material);
	EXPECT_EQ(Material::CHANGE_FLAG_DEFINITION | Material::CHANGE_FLAG_FULL_RESULT, result);
	recordChange.clearLastEvent();

	const double diffuseColour[3] = { 0.2, 0.4, 0.6 };
	EXPECT_EQ(CMZN_OK, material.setAttributeReal3(Material::ATTRIBUTE_DIFFUSE, diffuseColour));
	result = recordChange.getLastEvent().getSummaryMaterialChangeFlags();
	EXPECT_EQ(Material::CHANGE_FLAG_DEFINITION | Material::CHANGE_FLAG_FULL_RESULT, result);
	result = recordChange.getLastEvent().getMaterialChangeFlags(material);
	EXPECT_EQ(Material::CHANGE_FLAG_DEFINITION | Material::CHANGE_FLAG_FULL_RESULT, result);
	recordChange.clearLastEvent();

	EXPECT_EQ(CMZN_OK, material.setManaged(true));
	result = recordChange.getLastEvent().getSummaryMaterialChangeFlags();
	EXPECT_EQ(Material::CHANGE_FLAG_DEFINITION, result);
	result = recordChange.getLastEvent().getMaterialChangeFlags(material);
	EXPECT_EQ(Material::CHANGE_FLAG_DEFINITION, result);
	recordChange.clearLastEvent();

	const char *materialName2 = "fred";
	// add another material while unmanaging above material
	{
		ChangeManager<Materialmodule> materialChange(mm);

		Material material2 = mm.createMaterial();
		EXPECT_TRUE(material2.isValid());
		EXPECT_EQ(CMZN_OK, material2.setName(materialName2));
		EXPECT_EQ(CMZN_OK, material2.setManaged(true));  // so it stays around
		EXPECT_EQ(CMZN_OK, material.setManaged(false));
	}
	Material material2 = mm.findMaterialByName(materialName2);
	EXPECT_TRUE(material2.isValid());
	result = recordChange.getLastEvent().getSummaryMaterialChangeFlags();
	EXPECT_EQ(Material::CHANGE_FLAG_ADD | Material::CHANGE_FLAG_DEFINITION, result);
	result = recordChange.getLastEvent().getMaterialChangeFlags(material);
	EXPECT_EQ(Material::CHANGE_FLAG_DEFINITION, result);
	result = recordChange.getLastEvent().getMaterialChangeFlags(material2);
	EXPECT_EQ(Material::CHANGE_FLAG_ADD, result);
	recordChange.clearLastEvent();

	// destroy by removing last reference
	material = Material();
	result = recordChange.getLastEvent().getSummaryMaterialChangeFlags();
	EXPECT_EQ(Material::CHANGE_FLAG_REMOVE, result);
	recordChange.clearLastEvent();

	EXPECT_EQ(CMZN_OK, materialmodulenotifier.clearCallback());
}
