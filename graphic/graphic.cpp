
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/region.h>
#include <zinc/fieldmodule.h>
#include <zinc/scene.h>
#include <zinc/field.h>
#include <zinc/fieldconstant.h>
#include <zinc/graphic.h>
#include <zinc/spectrum.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/graphic.hpp"
#include "zinc/fieldtypesconstant.hpp"
#include "zinc/font.hpp"

TEST(Cmiss_graphic_api, set_use_element_type)
{
	ZincTestSetup zinc;

	Cmiss_graphic_contours_id is = Cmiss_scene_create_graphic_contours(zinc.scene);
	EXPECT_NE(static_cast<Cmiss_graphic_contours *>(0), is);

	int result = Cmiss_graphic_set_domain_type(Cmiss_graphic_contours_base_cast(is), CMISS_FIELD_DOMAIN_ELEMENTS_2D);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_graphic_contours_destroy(&is);
}

TEST(Cmiss_graphic_api, exterior)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_graphic_surfaces_base_cast(Cmiss_scene_create_graphic_surfaces(zinc.scene));
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	EXPECT_EQ(0, Cmiss_graphic_get_exterior(gr));
	int result = Cmiss_graphic_set_exterior(gr, 1);
	EXPECT_EQ(CMISS_OK, result);
	EXPECT_EQ(1, Cmiss_graphic_get_exterior(gr));

	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, face)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_graphic_lines_base_cast(Cmiss_scene_create_graphic_lines(zinc.scene));
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	EXPECT_EQ(CMISS_ELEMENT_FACE_ALL, Cmiss_graphic_get_face(gr));
	int result = Cmiss_graphic_set_face(gr, CMISS_ELEMENT_FACE_XI2_0);
	EXPECT_EQ(CMISS_OK, result);
	EXPECT_EQ(CMISS_ELEMENT_FACE_XI2_0, Cmiss_graphic_get_face(gr));

	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, coordinate_field)
{
	ZincTestSetup zinc;

	const double values[] = { 1.0, 2.0, 3.0 };
	Cmiss_field_id coordinate_field = Cmiss_field_module_create_constant(zinc.fm,
		sizeof(values)/sizeof(double), values);
	EXPECT_NE(static_cast<Cmiss_field *>(0), coordinate_field);

	Cmiss_graphic_id gr = Cmiss_graphic_points_base_cast(Cmiss_scene_create_graphic_points(zinc.scene));
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_domain_type(gr, CMISS_FIELD_DOMAIN_NODES));

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_coordinate_field(gr, coordinate_field));

	// coordinate field cannot have more than 3 components
	const double values4[] = { 1.0, 2.0, 3.0, 4.0 };
	Cmiss_field_id bad_coordinate_field = Cmiss_field_module_create_constant(zinc.fm,
		sizeof(values4)/sizeof(double), values4);
	EXPECT_NE(static_cast<Cmiss_field *>(0), bad_coordinate_field);
	// previous coordinate field should be left unchanged
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_set_coordinate_field(gr, bad_coordinate_field));
	Cmiss_field_destroy(&bad_coordinate_field);

	Cmiss_field_id temp_coordinate_field = Cmiss_graphic_get_coordinate_field(gr);
	EXPECT_EQ(coordinate_field, temp_coordinate_field);
	Cmiss_field_destroy(&temp_coordinate_field);
	Cmiss_field_destroy(&coordinate_field);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_coordinate_field(gr, 0));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_get_coordinate_field(gr));

	// check coordinate_field removed as no longer used
	Cmiss_field_iterator_id iter = Cmiss_field_module_create_field_iterator(zinc.fm);
	EXPECT_NE(static_cast<Cmiss_field_iterator *>(0), iter);
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_field_iterator_next(iter));
	Cmiss_field_iterator_destroy(&iter);

	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, data_field)
{
	ZincTestSetup zinc;

	double values[] = { 1.0, 2.0, 3.0 };
	Cmiss_field_id data_field = Cmiss_field_module_create_constant(zinc.fm,
		sizeof(values)/sizeof(double), values);
	EXPECT_NE(static_cast<Cmiss_field *>(0), data_field);

	Cmiss_graphic_id gr = Cmiss_graphic_points_base_cast(Cmiss_scene_create_graphic_points(zinc.scene));
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_data_field(gr, data_field));

	Cmiss_field_id temp_data_field = Cmiss_graphic_get_data_field(gr);
	EXPECT_EQ(temp_data_field, data_field);
	Cmiss_field_destroy(&temp_data_field);
	Cmiss_field_destroy(&data_field);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_data_field(gr, 0));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_get_data_field(gr));

	// check data_field removed as no longer used
	Cmiss_field_iterator_id iter = Cmiss_field_module_create_field_iterator(zinc.fm);
	EXPECT_NE(static_cast<Cmiss_field_iterator *>(0), iter);
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_field_iterator_next(iter));
	Cmiss_field_iterator_destroy(&iter);

	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, material)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_scene_create_graphic(zinc.scene, CMISS_GRAPHIC_LINES);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	Cmiss_graphics_material_module_id material_module = Cmiss_graphics_module_get_material_module(zinc.gm);
	Cmiss_graphics_material_id default_material = Cmiss_graphics_material_module_get_default_material(material_module);
	Cmiss_graphics_material_id temp_material = Cmiss_graphic_get_material(gr);
	EXPECT_EQ(default_material, temp_material);
	Cmiss_graphics_material_destroy(&temp_material);
	Cmiss_graphics_material_destroy(&default_material);

	Cmiss_graphics_material_id material = Cmiss_graphics_material_module_create_material(material_module);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_material(gr, material));
	temp_material = Cmiss_graphic_get_material(gr);
	EXPECT_EQ(material, temp_material);
	Cmiss_graphics_material_destroy(&temp_material);
	Cmiss_graphics_material_destroy(&material);

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_set_material(gr, 0));

	Cmiss_graphics_material_module_destroy(&material_module);
	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, material_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicLines gr = zinc.scene.createGraphicLines();
	EXPECT_TRUE(gr.isValid());

	GraphicsMaterialModule materialModule = zinc.gm.getMaterialModule();
	GraphicsMaterial defaultMaterial = materialModule.getDefaultMaterial();
	GraphicsMaterial tempMaterial = gr.getMaterial();
	EXPECT_EQ(defaultMaterial.getId(), tempMaterial.getId());

	GraphicsMaterial material = materialModule.createMaterial();
	EXPECT_EQ(CMISS_OK, gr.setMaterial(material));
	tempMaterial = gr.getMaterial();
	EXPECT_EQ(material.getId(), tempMaterial.getId());

	GraphicsMaterial noMaterial;
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, gr.setMaterial(noMaterial));
}

TEST(Cmiss_graphic_api, selected_material)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_scene_create_graphic(zinc.scene, CMISS_GRAPHIC_LINES);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	Cmiss_graphics_material_module_id material_module = Cmiss_graphics_module_get_material_module(zinc.gm);
	Cmiss_graphics_material_id default_selected_material = Cmiss_graphics_material_module_get_default_selected_material(material_module);
	Cmiss_graphics_material_id temp_selected_material = Cmiss_graphic_get_selected_material(gr);
	EXPECT_EQ(default_selected_material, temp_selected_material);
	Cmiss_graphics_material_destroy(&temp_selected_material);
	Cmiss_graphics_material_destroy(&default_selected_material);

	Cmiss_graphics_material_id selected_material = Cmiss_graphics_material_module_create_material(material_module);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_selected_material(gr, selected_material));
	temp_selected_material = Cmiss_graphic_get_selected_material(gr);
	EXPECT_EQ(selected_material, temp_selected_material);
	Cmiss_graphics_material_destroy(&temp_selected_material);
	Cmiss_graphics_material_destroy(&selected_material);

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_set_selected_material(gr, 0));

	Cmiss_graphics_material_module_destroy(&material_module);
	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, selected_material_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicLines gr = zinc.scene.createGraphicLines();
	EXPECT_TRUE(gr.isValid());

	GraphicsMaterialModule materialModule = zinc.gm.getMaterialModule();
	GraphicsMaterial defaultSelectedMaterial = materialModule.getDefaultSelectedMaterial();
	GraphicsMaterial tempSelectedMaterial = gr.getSelectedMaterial();
	EXPECT_EQ(defaultSelectedMaterial.getId(), tempSelectedMaterial.getId());

	GraphicsMaterial selectedMaterial = materialModule.createMaterial();
	EXPECT_EQ(CMISS_OK, gr.setSelectedMaterial(selectedMaterial));
	tempSelectedMaterial = gr.getSelectedMaterial();
	EXPECT_EQ(selectedMaterial.getId(), tempSelectedMaterial.getId());

	GraphicsMaterial noMaterial;
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, gr.setSelectedMaterial(noMaterial));
}

TEST(Cmiss_graphic, name)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_graphic_surfaces_base_cast(Cmiss_scene_create_graphic_surfaces(zinc.scene));
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	char *name = Cmiss_graphic_get_name(gr);
	EXPECT_STREQ(static_cast<char *>(0), name);

	const char *nameBob = "Bob";
	int result;
	ASSERT_EQ(CMISS_OK, result = Cmiss_graphic_set_name(gr, nameBob));
	name = Cmiss_graphic_get_name(gr);
	EXPECT_STREQ(nameBob, name);
	Cmiss_deallocate(name);

	ASSERT_EQ(CMISS_OK, result = Cmiss_graphic_set_name(gr, static_cast<char *>(0)));
	name = Cmiss_graphic_get_name(gr);
	EXPECT_STREQ(static_cast<char *>(0), name);

	Cmiss_graphic_destroy(&gr);
}

TEST(ZincGraphic, name)
{
	ZincTestSetupCpp zinc;

	Graphic gr = zinc.scene.createGraphicSurfaces();
	EXPECT_TRUE(gr.isValid());

	char *name = gr.getName();
	EXPECT_STREQ(static_cast<char *>(0), name);

	const char *nameBob = "Bob";
	int result;
	ASSERT_EQ(CMISS_OK, result = gr.setName(nameBob));
	name = gr.getName();
	EXPECT_STREQ(nameBob, name);
	Cmiss_deallocate(name);

	ASSERT_EQ(CMISS_OK, result = result = gr.setName(static_cast<char *>(0)));
	name = gr.getName();
	EXPECT_STREQ(static_cast<char *>(0), name);
}

TEST(Cmiss_graphic_api, spectrum)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_graphic_surfaces_base_cast(Cmiss_scene_create_graphic_surfaces(zinc.scene));
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	Cmiss_spectrum_module_id spectrum_module = Cmiss_graphics_module_get_spectrum_module(zinc.gm);
	EXPECT_NE(static_cast<Cmiss_spectrum_module *>(0), spectrum_module);

	Cmiss_spectrum_id spectrum = Cmiss_spectrum_module_create_spectrum(spectrum_module);
	EXPECT_NE(static_cast<Cmiss_spectrum *>(0), spectrum);

	Cmiss_spectrum_module_destroy(&spectrum_module);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_spectrum(gr, spectrum));

	Cmiss_spectrum_id temp_spectrum = Cmiss_graphic_get_spectrum(gr);
	EXPECT_EQ(temp_spectrum, spectrum);
	Cmiss_spectrum_destroy(&temp_spectrum);
	Cmiss_spectrum_destroy(&spectrum);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_spectrum(gr, 0));
	EXPECT_EQ(static_cast<Cmiss_spectrum *>(0), Cmiss_graphic_get_spectrum(gr));

	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, subgroup_field)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_graphic_points_base_cast(Cmiss_scene_create_graphic_points(zinc.scene));
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_domain_type(gr, CMISS_FIELD_DOMAIN_NODES));

	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_get_subgroup_field(gr));

	const double value = 1.0;
	Cmiss_field_id subgroup_field = Cmiss_field_module_create_constant(zinc.fm, 1, &value);
	EXPECT_NE(static_cast<Cmiss_field *>(0), subgroup_field);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_subgroup_field(gr, subgroup_field));

	// subgroup field must be scalar
	double values2[] = { 1.0, 2.0 };
	Cmiss_field_id bad_subgroup_field = Cmiss_field_module_create_constant(zinc.fm,
		sizeof(values2)/sizeof(double), values2);
	EXPECT_NE(static_cast<Cmiss_field *>(0), bad_subgroup_field);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_set_subgroup_field(gr, bad_subgroup_field));
	Cmiss_field_destroy(&bad_subgroup_field);

	// previous subgroup field should be left unchanged
	Cmiss_field_id temp_subgroup_field = Cmiss_graphic_get_subgroup_field(gr);
	EXPECT_EQ(subgroup_field, temp_subgroup_field);
	Cmiss_field_destroy(&temp_subgroup_field);
	Cmiss_field_destroy(&subgroup_field);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_subgroup_field(gr, 0));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_get_subgroup_field(gr));

	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, subgroup_field_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicPoints gr = zinc.scene.createGraphicPoints();
	EXPECT_TRUE(gr.isValid());
	EXPECT_EQ(CMISS_OK, gr.setDomainType(Field::DOMAIN_NODES));

	Field tempSubgroupField = gr.getSubgroupField();
	EXPECT_FALSE(tempSubgroupField.isValid());

	const double value = 1.0;
	Field subgroupField = zinc.fm.createConstant(1, &value);
	EXPECT_TRUE(subgroupField.isValid());
	EXPECT_EQ(CMISS_OK, gr.setSubgroupField(subgroupField));

	// subgroup field must be scalar
	double values2[] = { 1.0, 2.0 };
	Field badSubgroupField = zinc.fm.createConstant(2, values2);
	EXPECT_TRUE(badSubgroupField.isValid());
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, gr.setSubgroupField(badSubgroupField));

	// previous subgroup field should be left unchanged
	tempSubgroupField = gr.getSubgroupField();
	EXPECT_EQ(subgroupField.getId(), tempSubgroupField.getId());

	Field noField;
	EXPECT_EQ(CMISS_OK, gr.setSubgroupField(noField)); // clear subgroup field
	tempSubgroupField = gr.getSubgroupField();
	EXPECT_FALSE(tempSubgroupField.isValid());
}

TEST(Cmiss_graphic_api, tessellation)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_graphic_surfaces_base_cast(Cmiss_scene_create_graphic_surfaces(zinc.scene));
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	Cmiss_tessellation_module_id tessellation_module = Cmiss_graphics_module_get_tessellation_module(zinc.gm);
	Cmiss_tessellation_id default_tessellation = Cmiss_tessellation_module_get_default_tessellation(tessellation_module);
	EXPECT_NE(static_cast<Cmiss_tessellation_id>(0), default_tessellation);
	Cmiss_tessellation_id temp_tessellation = Cmiss_graphic_get_tessellation(gr);
	EXPECT_EQ(default_tessellation, temp_tessellation);
	Cmiss_tessellation_destroy(&temp_tessellation);
	Cmiss_tessellation_destroy(&default_tessellation);

	Cmiss_tessellation_id tessellation = Cmiss_tessellation_module_create_tessellation(tessellation_module);
	EXPECT_NE(static_cast<Cmiss_tessellation_id>(0), tessellation);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_tessellation(gr, tessellation));
	temp_tessellation = Cmiss_graphic_get_tessellation(gr);
	EXPECT_EQ(tessellation, temp_tessellation);
	Cmiss_tessellation_destroy(&temp_tessellation);
	Cmiss_tessellation_destroy(&tessellation);

	// can't remove tessellation
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_set_tessellation(gr, 0));

	Cmiss_tessellation_module_destroy(&tessellation_module);
	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, tessellation_cpp)
{
	ZincTestSetupCpp zinc;

	Graphic gr = zinc.scene.createGraphicSurfaces();
	EXPECT_TRUE(gr.isValid());

	TessellationModule tessellationModule = zinc.gm.getTessellationModule();
	Tessellation defaultTessellation = tessellationModule.getDefaultTessellation();
	Tessellation tempTessellation = gr.getTessellation();
	EXPECT_EQ(defaultTessellation.getId(), tempTessellation.getId());

	Tessellation tessellation = tessellationModule.createTessellation();
	EXPECT_TRUE(tessellation.isValid());
	EXPECT_EQ(CMISS_OK, gr.setTessellation(tessellation));
	tempTessellation = gr.getTessellation();
	EXPECT_EQ(tessellation.getId(), tempTessellation.getId());

	// can't remove tessellation
	Tessellation noTessellation;
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, gr.setTessellation(noTessellation));
}

TEST(Cmiss_graphic_api, tessellation_field)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_graphic_contours_base_cast(Cmiss_scene_create_graphic_contours(zinc.scene));
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_get_tessellation_field(gr));

	const double value = 1.0;
	Cmiss_field_id tessellation_field = Cmiss_field_module_create_constant(zinc.fm, 1, &value);
	EXPECT_NE(static_cast<Cmiss_field *>(0), tessellation_field);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_tessellation_field(gr, tessellation_field));

	Cmiss_field_id temp_tessellation_field = Cmiss_graphic_get_tessellation_field(gr);
	EXPECT_EQ(tessellation_field, temp_tessellation_field);
	Cmiss_field_destroy(&temp_tessellation_field);
	Cmiss_field_destroy(&tessellation_field);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_tessellation_field(gr, 0));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_get_tessellation_field(gr));

	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, tessellation_field_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicContours gr = zinc.scene.createGraphicContours();
	EXPECT_TRUE(gr.isValid());

	Field tempTessellationField = gr.getTessellationField();
	EXPECT_FALSE(tempTessellationField.isValid());

	const double value = 1.0;
	Field tessellationField = zinc.fm.createConstant(1, &value);
	EXPECT_TRUE(tessellationField.isValid());
	EXPECT_EQ(CMISS_OK, gr.setTessellationField(tessellationField));

	tempTessellationField = gr.getTessellationField();
	EXPECT_EQ(tessellationField.getId(), tempTessellationField.getId());

	Field noField;
	EXPECT_EQ(CMISS_OK, gr.setTessellationField(noField)); // clear tessellation field
	tempTessellationField = gr.getTessellationField();
	EXPECT_FALSE(tempTessellationField.isValid());
}

TEST(Cmiss_graphic_api, texture_coordinate_field)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_graphic_surfaces_base_cast(Cmiss_scene_create_graphic_surfaces(zinc.scene));
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	const double values[] = { 1.0, 2.0, 3.0 };
	Cmiss_field_id texture_coordinate_field = Cmiss_field_module_create_constant(zinc.fm,
		sizeof(values)/sizeof(double), values);
	EXPECT_NE(static_cast<Cmiss_field *>(0), texture_coordinate_field);

	EXPECT_EQ((Cmiss_field_id)0, Cmiss_graphic_get_texture_coordinate_field(gr));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_texture_coordinate_field(gr, texture_coordinate_field));

	// coordinate field cannot have more than 3 components
	const double values4[] = { 1.0, 2.0, 3.0, 4.0 };
	Cmiss_field_id bad_texture_coordinate_field = Cmiss_field_module_create_constant(zinc.fm,
		sizeof(values4)/sizeof(double), values4);
	EXPECT_NE(static_cast<Cmiss_field *>(0), bad_texture_coordinate_field);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_set_texture_coordinate_field(gr, bad_texture_coordinate_field));
	Cmiss_field_destroy(&bad_texture_coordinate_field);

	// previous texture coordinate field should be left unchanged
	Cmiss_field_id temp_texture_coordinate_field = Cmiss_graphic_get_texture_coordinate_field(gr);
	EXPECT_EQ(texture_coordinate_field, temp_texture_coordinate_field);
	Cmiss_field_destroy(&temp_texture_coordinate_field);
	Cmiss_field_destroy(&texture_coordinate_field);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_texture_coordinate_field(gr, 0));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_get_texture_coordinate_field(gr));

	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, texture_coordinate_field_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicSurfaces gr = zinc.scene.createGraphicSurfaces();
	EXPECT_TRUE(gr.isValid());

	Field tempTextureCoordinateField = gr.getTextureCoordinateField();
	EXPECT_FALSE(tempTextureCoordinateField.isValid());

	const double values[] = { 1.0, 2.0, 3.0 };
	Field textureCoordinateField = zinc.fm.createConstant(3, values);
	EXPECT_TRUE(textureCoordinateField.isValid());
	EXPECT_EQ(CMISS_OK, gr.setTextureCoordinateField(textureCoordinateField));

	// coordinate field cannot have more than 3 components
	const double values4[] = { 1.0, 2.0, 3.0, 4.0 };
	Field badTextureCoordinateField = zinc.fm.createConstant(4, values4);
	EXPECT_TRUE(badTextureCoordinateField.isValid());
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, gr.setTextureCoordinateField(badTextureCoordinateField));

	// previous texture coordinate field should be left unchanged
	tempTextureCoordinateField = gr.getTextureCoordinateField();
	EXPECT_EQ(textureCoordinateField.getId(), tempTextureCoordinateField.getId());

	Field noField;
	EXPECT_EQ(CMISS_OK, gr.setTextureCoordinateField(noField)); // clear texture coordinate field
	tempTextureCoordinateField = gr.getTextureCoordinateField();
	EXPECT_FALSE(tempTextureCoordinateField.isValid());
}

TEST(Cmiss_graphic_api, point_attributes_glyph)
{
	ZincTestSetup zinc;

	Cmiss_graphic_points_id gr = Cmiss_scene_create_graphic_points(zinc.scene);
	EXPECT_NE(static_cast<Cmiss_graphic_points *>(0), gr);

	Cmiss_graphic_point_attributes_id pointattr =
		Cmiss_graphic_get_point_attributes(Cmiss_graphic_points_base_cast(gr));
	EXPECT_NE(static_cast<Cmiss_graphic_point_attributes *>(0), pointattr);

	Cmiss_glyph_id glyph = Cmiss_glyph_module_get_default_point_glyph(zinc.glyph_module);
	EXPECT_NE((Cmiss_glyph_id)0, glyph);
	Cmiss_glyph_id temp_glyph = Cmiss_graphic_point_attributes_get_glyph(pointattr);
	EXPECT_EQ(glyph, temp_glyph);
	Cmiss_glyph_destroy(&temp_glyph);
	Cmiss_glyph_destroy(&glyph);

	glyph = Cmiss_glyph_module_find_glyph_by_name(zinc.glyph_module, "sphere");
	EXPECT_NE((Cmiss_glyph_id)0, glyph);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_glyph(pointattr, glyph));
	temp_glyph = Cmiss_graphic_point_attributes_get_glyph(pointattr);
	EXPECT_EQ(glyph, temp_glyph);
	Cmiss_glyph_destroy(&temp_glyph);
	Cmiss_glyph_destroy(&glyph);

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_glyph_type(pointattr, CMISS_GRAPHICS_GLYPH_TYPE_INVALID));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_glyph_type(pointattr, CMISS_GRAPHICS_GLYPH_SPHERE));

	EXPECT_EQ(CMISS_GLYPH_REPEAT_NONE, Cmiss_graphic_point_attributes_get_glyph_repeat_mode(pointattr));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_glyph_repeat_mode(0, CMISS_GLYPH_REPEAT_MIRROR));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_glyph_repeat_mode(pointattr, CMISS_GLYPH_REPEAT_MODE_INVALID));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_glyph_repeat_mode(pointattr, CMISS_GLYPH_REPEAT_MIRROR));
	EXPECT_EQ(CMISS_GLYPH_REPEAT_MIRROR, Cmiss_graphic_point_attributes_get_glyph_repeat_mode(pointattr));
	double fieldValues[] = { 0.3, 0.4, 0.5 };
	Cmiss_field_id field = Cmiss_field_module_create_constant(zinc.fm, 3, fieldValues);
	EXPECT_NE(static_cast<Cmiss_field *>(0), field);
	Cmiss_field_id temp_field = 0;

	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_point_attributes_get_orientation_scale_field(pointattr));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_orientation_scale_field(pointattr, field));
	temp_field = Cmiss_graphic_point_attributes_get_orientation_scale_field(pointattr);
	EXPECT_EQ(temp_field, field);
	Cmiss_field_destroy(&temp_field);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_orientation_scale_field(pointattr, 0));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_point_attributes_get_orientation_scale_field(pointattr));

	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_point_attributes_get_signed_scale_field(pointattr));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_signed_scale_field(pointattr, field));
	temp_field = Cmiss_graphic_point_attributes_get_signed_scale_field(pointattr);
	EXPECT_EQ(temp_field, field);
	Cmiss_field_destroy(&temp_field);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_signed_scale_field(pointattr, 0));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_point_attributes_get_signed_scale_field(pointattr));

	Cmiss_field_destroy(&field);

	const double values[] = { 0.5, 1.2 };
	double outputValues[3];

	// check default values = 0.0
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_get_base_size(pointattr, 3, outputValues));
	EXPECT_EQ(0.0, outputValues[0]);
	EXPECT_EQ(0.0, outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_base_size(pointattr, 0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_base_size(pointattr, 2, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_base_size(pointattr, 2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_get_base_size(pointattr, 0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_get_base_size(pointattr, 3, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_get_base_size(pointattr, 3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[1], outputValues[2]);

	// check default values = 0.0
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_get_glyph_offset(pointattr, 3, outputValues));
	EXPECT_EQ(0.0, outputValues[0]);
	EXPECT_EQ(0.0, outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_glyph_offset(pointattr, 0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_glyph_offset(pointattr, 2, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_glyph_offset(pointattr, 2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_get_glyph_offset(pointattr, 0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_get_glyph_offset(pointattr, 3, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_get_glyph_offset(pointattr, 3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);

	// check default values = 1.0
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_get_scale_factors(pointattr, 3, outputValues));
	EXPECT_EQ(1.0, outputValues[0]);
	EXPECT_EQ(1.0, outputValues[1]);
	EXPECT_EQ(1.0, outputValues[2]);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_scale_factors(pointattr, 0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_scale_factors(pointattr, 2, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_scale_factors(pointattr, 2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_get_scale_factors(pointattr, 0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_get_scale_factors(pointattr, 3, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_get_scale_factors(pointattr, 3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[1], outputValues[2]);

	Cmiss_graphic_point_attributes_destroy(&pointattr);
	Cmiss_graphic_points_destroy(&gr);
}

TEST(Cmiss_graphic_api, point_attributes_glyph_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicPoints gr = zinc.scene.createGraphicPoints();
	EXPECT_TRUE(gr.isValid());
	// test can assign to base class handle
	Graphic tmp(gr);
	EXPECT_TRUE(tmp.isValid());

	GraphicPointAttributes pointattr = gr.getPointAttributes();
	EXPECT_TRUE(pointattr.isValid());

	Glyph glyph = zinc.glyphModule.getDefaultPointGlyph();
	EXPECT_TRUE(glyph.isValid());
	Glyph tempGlyph = pointattr.getGlyph();
	EXPECT_EQ(glyph.getId(), tempGlyph.getId());

	glyph = zinc.glyphModule.findGlyphByName("sphere");
	EXPECT_TRUE(glyph.isValid());
	EXPECT_EQ(CMISS_OK, pointattr.setGlyph(glyph));
	tempGlyph = pointattr.getGlyph();
	EXPECT_EQ(glyph.getId(), tempGlyph.getId());

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setGlyphType(Graphic::GLYPH_TYPE_INVALID));
	EXPECT_EQ(CMISS_OK, pointattr.setGlyphType(Graphic::GLYPH_TYPE_SPHERE));

	EXPECT_EQ(Glyph::REPEAT_NONE, pointattr.getGlyphRepeatMode());
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setGlyphRepeatMode(Glyph::REPEAT_MODE_INVALID));
	EXPECT_EQ(CMISS_OK, pointattr.setGlyphRepeatMode(Glyph::REPEAT_MIRROR));
	EXPECT_EQ(Glyph::REPEAT_MIRROR, pointattr.getGlyphRepeatMode());

	double fieldValues[] = { 0.3, 0.4, 0.5 };
	Field field = zinc.fm.createConstant(sizeof(fieldValues)/sizeof(double), fieldValues);
	EXPECT_TRUE(field.isValid());
	Field tempField;

	EXPECT_FALSE(pointattr.getOrientationScaleField().isValid());
	EXPECT_EQ(CMISS_OK, pointattr.setOrientationScaleField(field));
	tempField = pointattr.getOrientationScaleField();
	EXPECT_EQ(tempField.getId(), field.getId());
	Field noField;
	EXPECT_EQ(CMISS_OK, pointattr.setOrientationScaleField(noField)); // clear field
	EXPECT_FALSE(pointattr.getOrientationScaleField().isValid());

	EXPECT_FALSE(pointattr.getSignedScaleField().isValid());
	EXPECT_EQ(CMISS_OK, pointattr.setSignedScaleField(field));
	tempField = pointattr.getSignedScaleField();
	EXPECT_EQ(tempField.getId(), field.getId());
	EXPECT_EQ(CMISS_OK, pointattr.setSignedScaleField(noField)); // clear field
	EXPECT_FALSE(pointattr.getSignedScaleField().isValid());

	const double values[] = { 0.5, 1.2 };
	double outputValues[3];

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setBaseSize(0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setBaseSize(2, 0));
	EXPECT_EQ(CMISS_OK, pointattr.setBaseSize(2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.getBaseSize(0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.getBaseSize(3, 0));
	EXPECT_EQ(CMISS_OK, pointattr.getBaseSize(3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[1], outputValues[2]);

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setGlyphOffset(0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setGlyphOffset(2, 0));
	EXPECT_EQ(CMISS_OK, pointattr.setGlyphOffset(2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.getGlyphOffset(0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.getGlyphOffset(3, 0));
	EXPECT_EQ(CMISS_OK, pointattr.getGlyphOffset(3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setScaleFactors(0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setScaleFactors(2, 0));
	EXPECT_EQ(CMISS_OK, pointattr.setScaleFactors(2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.getScaleFactors(0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.getScaleFactors(3, 0));
	EXPECT_EQ(CMISS_OK, pointattr.getScaleFactors(3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[1], outputValues[2]);
}

TEST(Cmiss_graphic_api, point_attributes_label)
{
	ZincTestSetup zinc;

	Cmiss_graphic_points_id gr = Cmiss_scene_create_graphic_points(zinc.scene);
	EXPECT_NE(static_cast<Cmiss_graphic_points *>(0), gr);

	Cmiss_graphic_point_attributes_id pointattr = Cmiss_graphic_get_point_attributes(Cmiss_graphic_points_base_cast(gr));
	EXPECT_NE(static_cast<Cmiss_graphic_point_attributes *>(0), pointattr);

	double values[] = { 1.0, 2.0, 3.0 };
	Cmiss_field_id label_field = Cmiss_field_module_create_constant(zinc.fm,
		sizeof(values)/sizeof(double), values);
	EXPECT_NE(static_cast<Cmiss_field *>(0), label_field);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_label_field(pointattr, label_field));

	Cmiss_field_id temp_label_field = Cmiss_graphic_point_attributes_get_label_field(pointattr);
	EXPECT_EQ(temp_label_field, label_field);
	Cmiss_field_destroy(&temp_label_field);
	Cmiss_field_destroy(&label_field);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_label_field(pointattr, 0));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_point_attributes_get_label_field(pointattr));

	double outputValues[3];
	// check default values = 0.0
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_get_label_offset(pointattr, 3, outputValues));
	EXPECT_EQ(0.0, outputValues[0]);
	EXPECT_EQ(0.0, outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_label_offset(pointattr, 0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_label_offset(pointattr, 2, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_label_offset(pointattr, 2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_get_label_offset(pointattr, 0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_get_label_offset(pointattr, 3, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_get_label_offset(pointattr, 3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);

	// should start with a default font
	Cmiss_font_id font = Cmiss_graphic_point_attributes_get_font(pointattr);
	EXPECT_NE(static_cast<Cmiss_font *>(0), font);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_font(pointattr, 0));
	EXPECT_EQ(static_cast<Cmiss_font *>(0), Cmiss_graphic_point_attributes_get_font(pointattr));

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_font(pointattr, font));

	const char *text = "ABC";
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_label_text(0, 1, text));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_label_text(pointattr, 0, text));
	EXPECT_EQ((char *)0, Cmiss_graphic_point_attributes_get_label_text(0, 1));
	EXPECT_EQ((char *)0, Cmiss_graphic_point_attributes_get_label_text(pointattr, 0));
	for (int labelNumber = 1; labelNumber <= 3; ++labelNumber)
	{
		char *outText;
		EXPECT_EQ((char *)0, Cmiss_graphic_point_attributes_get_label_text(pointattr, labelNumber));
		EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_label_text(pointattr, labelNumber, text));
		outText = Cmiss_graphic_point_attributes_get_label_text(pointattr, labelNumber);
		EXPECT_STREQ(text, outText);
		Cmiss_deallocate(outText);
	}
	Cmiss_font_destroy(&font);
	Cmiss_graphic_point_attributes_destroy(&pointattr);
	Cmiss_graphic_points_destroy(&gr);
}

TEST(Cmiss_graphic_api, point_attributes_label_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicPoints gr = zinc.scene.createGraphicPoints();
	EXPECT_TRUE(gr.isValid());

	GraphicPointAttributes pointattr = gr.getPointAttributes();
	EXPECT_TRUE(pointattr.isValid());

	double values[] = { 1.0, 2.0, 3.0 };
	Field labelField = zinc.fm.createConstant(sizeof(values)/sizeof(double), values);
	EXPECT_TRUE(labelField.isValid());

	EXPECT_EQ(CMISS_OK, pointattr.setLabelField(labelField));

	Field tempLabelField = pointattr.getLabelField();
	EXPECT_EQ(tempLabelField.getId(), labelField.getId());

	Field noField;
	EXPECT_EQ(CMISS_OK, pointattr.setLabelField(noField)); // clear label field
	EXPECT_FALSE(pointattr.getLabelField().isValid());

	double outputValues[3];
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setLabelOffset(0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setLabelOffset(2, 0));
	EXPECT_EQ(CMISS_OK, pointattr.setLabelOffset(2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.getLabelOffset(0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.getLabelOffset(3, 0));
	EXPECT_EQ(CMISS_OK, pointattr.getLabelOffset(3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);

	// should start with a default font
	Font font = pointattr.getFont();
	EXPECT_TRUE(font.isValid());

	Font noFont;
	EXPECT_EQ(CMISS_OK, pointattr.setFont(noFont)); // clear font
	EXPECT_FALSE(pointattr.getFont().isValid());

	EXPECT_EQ(CMISS_OK, pointattr.setFont(font));

	const char *text = "ABC";
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setLabelText(0, text));
	EXPECT_EQ((char *)0, pointattr.getLabelText(0));
	for (int labelNumber = 1; labelNumber <= 3; ++labelNumber)
	{
		char *outText;
		EXPECT_EQ((char *)0, pointattr.getLabelText(labelNumber));
		EXPECT_EQ(CMISS_OK, pointattr.setLabelText(labelNumber, text));
		outText = pointattr.getLabelText(labelNumber);
		EXPECT_STREQ(text, outText);
		Cmiss_deallocate(outText);
	}
}

TEST(Cmiss_graphic, polygon_render_mode)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_scene_create_graphic(zinc.scene, CMISS_GRAPHIC_SURFACES);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	Cmiss_graphic_polygon_render_mode polygonRenderMode;
	ASSERT_EQ(CMISS_GRAPHIC_POLYGON_RENDER_SHADED, polygonRenderMode = Cmiss_graphic_get_polygon_render_mode(gr));

	int result;
	ASSERT_EQ(CMISS_ERROR_ARGUMENT, result = Cmiss_graphic_set_polygon_render_mode(static_cast<Cmiss_graphic_id>(0), CMISS_GRAPHIC_POLYGON_RENDER_SHADED));
	ASSERT_EQ(CMISS_ERROR_ARGUMENT, result = Cmiss_graphic_set_polygon_render_mode(gr, CMISS_GRAPHIC_POLYGON_RENDER_MODE_INVALID));

	ASSERT_EQ(CMISS_OK, result = Cmiss_graphic_set_polygon_render_mode(gr, CMISS_GRAPHIC_POLYGON_RENDER_WIREFRAME));
	ASSERT_EQ(CMISS_GRAPHIC_POLYGON_RENDER_WIREFRAME, polygonRenderMode = Cmiss_graphic_get_polygon_render_mode(gr));

	Cmiss_graphic_destroy(&gr);
}

TEST(ZincGraphic, PolygonRenderMode)
{
	ZincTestSetupCpp zinc;

	GraphicSurfaces gr = zinc.scene.createGraphicSurfaces();
	EXPECT_TRUE(gr.isValid());

	Graphic::PolygonRenderMode polygonRenderMode;
	ASSERT_EQ(Graphic::POLYGON_RENDER_SHADED, polygonRenderMode = gr.getPolygonRenderMode());

	int result;
	ASSERT_EQ(CMISS_ERROR_ARGUMENT, result = gr.setPolygonRenderMode(Graphic::POLYGON_RENDER_MODE_INVALID));

	ASSERT_EQ(CMISS_OK, result = gr.setPolygonRenderMode(Graphic::POLYGON_RENDER_WIREFRAME));
	ASSERT_EQ(Graphic::POLYGON_RENDER_WIREFRAME, polygonRenderMode = gr.getPolygonRenderMode());
}

TEST(Cmiss_graphic_api, line_attributes)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_scene_create_graphic(zinc.scene, CMISS_GRAPHIC_LINES);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	Cmiss_graphic_line_attributes_id lineattr = Cmiss_graphic_get_line_attributes(gr);
	EXPECT_NE(static_cast<Cmiss_graphic_line_attributes *>(0), lineattr);
	EXPECT_EQ(CMISS_GRAPHIC_LINE_ATTRIBUTES_SHAPE_LINE, Cmiss_graphic_line_attributes_get_shape(lineattr));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_line_attributes_set_shape(lineattr, CMISS_GRAPHIC_LINE_ATTRIBUTES_SHAPE_CIRCLE_EXTRUSION));
	EXPECT_EQ(CMISS_GRAPHIC_LINE_ATTRIBUTES_SHAPE_CIRCLE_EXTRUSION, Cmiss_graphic_line_attributes_get_shape(lineattr));

	double value = 1.0;
	Cmiss_field_id orientation_scale_field = Cmiss_field_module_create_constant(zinc.fm, 1, &value);
	EXPECT_NE(static_cast<Cmiss_field *>(0), orientation_scale_field);

	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_line_attributes_get_orientation_scale_field(lineattr));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_line_attributes_set_orientation_scale_field(lineattr, orientation_scale_field));

	Cmiss_field_id temp_orientation_scale_field = Cmiss_graphic_line_attributes_get_orientation_scale_field(lineattr);
	EXPECT_EQ(temp_orientation_scale_field, orientation_scale_field);
	Cmiss_field_destroy(&temp_orientation_scale_field);
	Cmiss_field_destroy(&orientation_scale_field);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_line_attributes_set_orientation_scale_field(lineattr, 0));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_line_attributes_get_orientation_scale_field(lineattr));

	const double values[] = { 0.5, 1.2 };
	double outputValues[2];

	// check default values = 0.0
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_line_attributes_get_base_size(lineattr, 2, outputValues));
	EXPECT_EQ(0.0, outputValues[0]);
	EXPECT_EQ(0.0, outputValues[1]);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_line_attributes_set_base_size(lineattr, 0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_line_attributes_set_base_size(lineattr, 2, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_line_attributes_set_base_size(lineattr, 2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_line_attributes_get_base_size(lineattr, 0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_line_attributes_get_base_size(lineattr, 2, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_line_attributes_get_base_size(lineattr, 2, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[0], outputValues[1]); // lines/cylinders currenetly constrained to equal values

	// check default values = 1.0
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_line_attributes_get_scale_factors(lineattr, 2, outputValues));
	EXPECT_EQ(1.0, outputValues[0]);
	EXPECT_EQ(1.0, outputValues[1]);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_line_attributes_set_scale_factors(lineattr, 0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_line_attributes_set_scale_factors(lineattr, 2, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_line_attributes_set_scale_factors(lineattr, 2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_line_attributes_get_scale_factors(lineattr, 0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_line_attributes_get_scale_factors(lineattr, 2, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_line_attributes_get_scale_factors(lineattr, 2, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[0], outputValues[1]); // lines/cylinders currenetly constrained to equal values

	Cmiss_graphic_line_attributes_destroy(&lineattr);
	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, line_attributes_cpp)
{
	ZincTestSetupCpp zinc;

	Graphic gr = zinc.scene.createGraphic(Graphic::GRAPHIC_LINES);
	EXPECT_TRUE(gr.isValid());

	GraphicLineAttributes lineattr = gr.getLineAttributes();
	EXPECT_TRUE(lineattr.isValid());
	EXPECT_EQ(GraphicLineAttributes::SHAPE_LINE, lineattr.getShape());
	EXPECT_EQ(CMISS_OK, lineattr.setShape(GraphicLineAttributes::SHAPE_CIRCLE_EXTRUSION));
	EXPECT_EQ(GraphicLineAttributes::SHAPE_CIRCLE_EXTRUSION, lineattr.getShape());

	double value = 1.0;
	Field orientationScaleField = zinc.fm.createConstant(1, &value);
	EXPECT_TRUE(orientationScaleField.isValid());

	EXPECT_FALSE(lineattr.getOrientationScaleField().isValid());
	EXPECT_EQ(CMISS_OK, lineattr.setOrientationScaleField(orientationScaleField));

	Field tempOrientationScaleField = lineattr.getOrientationScaleField();
	EXPECT_EQ(tempOrientationScaleField.getId(), orientationScaleField.getId());

	Field noField;
	EXPECT_EQ(CMISS_OK, lineattr.setOrientationScaleField(noField)); // clear field
	EXPECT_FALSE(lineattr.getOrientationScaleField().isValid());

	const double values[] = { 0.5, 1.2 };
	double outputValues[2];

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, lineattr.setBaseSize(0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, lineattr.setBaseSize(2, 0));
	EXPECT_EQ(CMISS_OK, lineattr.setBaseSize(2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, lineattr.getBaseSize(0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, lineattr.getBaseSize(2, 0));
	EXPECT_EQ(CMISS_OK, lineattr.getBaseSize(2, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[0], outputValues[1]); // lines/cylinders currently constrained to equal values

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, lineattr.setScaleFactors(0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, lineattr.setScaleFactors(2, 0));
	EXPECT_EQ(CMISS_OK, lineattr.setScaleFactors(2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, lineattr.getScaleFactors(0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, lineattr.getScaleFactors(2, 0));
	EXPECT_EQ(CMISS_OK, lineattr.getScaleFactors(2, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[0], outputValues[1]); // lines/cylinders currently constrained to equal values
}

TEST(Cmiss_graphic_api, visibility_flag)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_graphic_surfaces_base_cast(Cmiss_scene_create_graphic_surfaces(zinc.scene));
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	EXPECT_TRUE(Cmiss_graphic_get_visibility_flag(gr));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_visibility_flag(gr, false));
	EXPECT_FALSE(Cmiss_graphic_get_visibility_flag(gr));

	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, visibility_flag_cpp)
{
	ZincTestSetupCpp zinc;

	Graphic gr = zinc.scene.createGraphicSurfaces();
	EXPECT_TRUE(gr.isValid());

	EXPECT_TRUE(gr.getVisibilityFlag());
	EXPECT_EQ(CMISS_OK, gr.setVisibilityFlag(false));
	EXPECT_FALSE(gr.getVisibilityFlag());
}

TEST(Cmiss_graphic_api, sampling_attributes)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_graphic_streamlines_base_cast(Cmiss_scene_create_graphic_streamlines(zinc.scene));
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	Cmiss_graphic_sampling_attributes_id sampling = Cmiss_graphic_get_sampling_attributes(gr);
	EXPECT_NE(static_cast<Cmiss_graphic_sampling_attributes *>(0), sampling);

	EXPECT_EQ(CMISS_ELEMENT_POINT_SAMPLE_CELL_CENTRES, Cmiss_graphic_sampling_attributes_get_mode(sampling));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_sampling_attributes_set_mode(sampling, CMISS_ELEMENT_POINT_SAMPLE_CELL_POISSON));
	EXPECT_EQ(CMISS_ELEMENT_POINT_SAMPLE_CELL_POISSON, Cmiss_graphic_sampling_attributes_get_mode(sampling));

	double value = 1.0;
	Cmiss_field_id density_field = Cmiss_field_module_create_constant(zinc.fm, 1, &value);
	EXPECT_NE(static_cast<Cmiss_field *>(0), density_field);

	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_sampling_attributes_get_density_field(sampling));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_sampling_attributes_set_density_field(sampling, density_field));

	Cmiss_field_id temp_density_field = Cmiss_graphic_sampling_attributes_get_density_field(sampling);
	EXPECT_EQ(density_field, temp_density_field);
	Cmiss_field_destroy(&temp_density_field);
	Cmiss_field_destroy(&density_field);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_sampling_attributes_set_density_field(sampling, static_cast<Cmiss_field_id>(0)));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_sampling_attributes_get_density_field(sampling));

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_sampling_attributes_set_mode(sampling, CMISS_ELEMENT_POINT_SAMPLE_SET_LOCATION));

	const double values[] = { 0.5, 0.20, 0.8 };
	double outputValues[3];
	// check default values = 0.0
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_sampling_attributes_get_location(sampling, 3, outputValues));
	EXPECT_EQ(0.0, outputValues[0]);
	EXPECT_EQ(0.0, outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_sampling_attributes_set_location(sampling, 0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_sampling_attributes_set_location(sampling, 2, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_sampling_attributes_set_location(sampling, 3, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_sampling_attributes_get_location(sampling, 0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_sampling_attributes_get_location(sampling, 3, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_sampling_attributes_get_location(sampling, 3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[2], outputValues[2]);

	Cmiss_graphic_sampling_attributes_destroy(&sampling);
	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, sampling_attributes_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicStreamlines gr = zinc.scene.createGraphicStreamlines();
	EXPECT_TRUE(gr.isValid());

	GraphicSamplingAttributes sampling = gr.getSamplingAttributes();
	EXPECT_TRUE(sampling.isValid());

	EXPECT_EQ(Element::POINT_SAMPLE_CELL_CENTRES, sampling.getMode());
	EXPECT_EQ(CMISS_OK, sampling.setMode(Element::POINT_SAMPLE_CELL_POISSON));
	EXPECT_EQ(Element::POINT_SAMPLE_CELL_POISSON, sampling.getMode());

	double value = 1.0;
	Field densityField = zinc.fm.createConstant(1, &value);
	EXPECT_TRUE(densityField.isValid());

	Field tempField = sampling.getDensityField();
	EXPECT_FALSE(tempField.isValid());
	EXPECT_EQ(CMISS_OK, sampling.setDensityField(densityField));
	tempField = sampling.getDensityField();
	EXPECT_EQ(densityField.getId(), tempField.getId());

	Field noField;
	EXPECT_EQ(CMISS_OK, sampling.setDensityField(noField));
	tempField = sampling.getDensityField();
	EXPECT_FALSE(tempField.isValid());

	EXPECT_EQ(CMISS_OK, sampling.setMode(Element::POINT_SAMPLE_SET_LOCATION));

	const double values[] = { 0.5, 0.20, 0.8 };
	double outputValues[3];
	// check default values = 0.0
	EXPECT_EQ(CMISS_OK, sampling.getLocation(3, outputValues));
	EXPECT_EQ(0.0, outputValues[0]);
	EXPECT_EQ(0.0, outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, sampling.setLocation(0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, sampling.setLocation(2, 0));
	EXPECT_EQ(CMISS_OK, sampling.setLocation(3, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, sampling.getLocation(0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, sampling.getLocation(3, 0));
	EXPECT_EQ(CMISS_OK, sampling.getLocation(3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[2], outputValues[2]);
}
