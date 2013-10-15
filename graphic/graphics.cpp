
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/region.h>
#include <zinc/fieldmodule.h>
#include <zinc/scene.h>
#include <zinc/field.h>
#include <zinc/fieldconstant.h>
#include <zinc/graphics.h>
#include <zinc/spectrum.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/graphics.hpp"
#include "zinc/fieldconstant.hpp"
#include "zinc/font.hpp"

TEST(cmzn_graphics_api, set_use_element_type)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_contours(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	int result = cmzn_graphics_set_domain_type(gr, CMZN_FIELD_DOMAIN_MESH_2D);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_graphics_destroy(&gr);
}

TEST(cmzn_graphics_api, exterior)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_surfaces(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	EXPECT_FALSE(cmzn_graphics_is_exterior(gr));
	int result = cmzn_graphics_set_exterior(gr, true);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_TRUE(cmzn_graphics_is_exterior(gr));

	cmzn_graphics_destroy(&gr);
}

TEST(ZincGraphics, exterior)
{
	ZincTestSetupCpp zinc;

	Graphics gr = zinc.scene.createGraphicsSurfaces();
	EXPECT_TRUE(gr.isValid());

	EXPECT_FALSE(gr.isExterior());
	int result = gr.setExterior(true);
	EXPECT_EQ(OK, result);
	EXPECT_TRUE(gr.isExterior());
}

TEST(cmzn_graphics_api, face)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_lines(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	EXPECT_EQ(CMZN_ELEMENT_FACE_ALL, cmzn_graphics_get_face(gr));
	int result = cmzn_graphics_set_face(gr, CMZN_ELEMENT_FACE_XI2_0);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_EQ(CMZN_ELEMENT_FACE_XI2_0, cmzn_graphics_get_face(gr));

	cmzn_graphics_destroy(&gr);
}

TEST(cmzn_graphics_api, coordinate_field)
{
	ZincTestSetup zinc;

	const double values[] = { 1.0, 2.0, 3.0 };
	cmzn_field_id coordinate_field = cmzn_fieldmodule_create_field_constant(zinc.fm,
		sizeof(values)/sizeof(double), values);
	EXPECT_NE(static_cast<cmzn_field *>(0), coordinate_field);

	cmzn_graphics_id gr = cmzn_scene_create_graphics_points(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_domain_type(gr, CMZN_FIELD_DOMAIN_NODES));

	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_coordinate_field(gr, coordinate_field));

	// coordinate field cannot have more than 3 components
	const double values4[] = { 1.0, 2.0, 3.0, 4.0 };
	cmzn_field_id bad_coordinate_field = cmzn_fieldmodule_create_field_constant(zinc.fm,
		sizeof(values4)/sizeof(double), values4);
	EXPECT_NE(static_cast<cmzn_field *>(0), bad_coordinate_field);
	// previous coordinate field should be left unchanged
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphics_set_coordinate_field(gr, bad_coordinate_field));
	cmzn_field_destroy(&bad_coordinate_field);

	cmzn_field_id temp_coordinate_field = cmzn_graphics_get_coordinate_field(gr);
	EXPECT_EQ(coordinate_field, temp_coordinate_field);
	cmzn_field_destroy(&temp_coordinate_field);
	cmzn_field_destroy(&coordinate_field);

	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_coordinate_field(gr, 0));
	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphics_get_coordinate_field(gr));

	// check coordinate_field removed as no longer used
	cmzn_fielditerator_id iter = cmzn_fieldmodule_create_fielditerator(zinc.fm);
	EXPECT_NE(static_cast<cmzn_fielditerator *>(0), iter);
	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_fielditerator_next(iter));
	cmzn_fielditerator_destroy(&iter);

	cmzn_graphics_destroy(&gr);
}

TEST(cmzn_graphics, coordinate_system)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics(zinc.scene, CMZN_GRAPHICS_POINTS);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	enum cmzn_scene_coordinate_system coordinate_system = cmzn_graphics_get_coordinate_system(gr);
	EXPECT_EQ(CMZN_SCENE_COORDINATE_SYSTEM_LOCAL, coordinate_system);

	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_coordinate_system(gr, CMZN_SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_LEFT));
	coordinate_system = cmzn_graphics_get_coordinate_system(gr);
	EXPECT_EQ(CMZN_SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_LEFT, coordinate_system);

	cmzn_graphics_destroy(&gr);
}

TEST(ZincGraphics, CoordinateSystem)
{
	ZincTestSetupCpp zinc;

	Graphics gr = zinc.scene.createGraphics(Graphics::POINTS);
	EXPECT_TRUE(gr.isValid());

	SceneCoordinateSystem coordinateSystem = gr.getCoordinateSystem();
	EXPECT_EQ(SCENE_COORDINATE_SYSTEM_LOCAL, coordinateSystem);

	EXPECT_EQ(OK, gr.setCoordinateSystem(SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_LEFT));
	coordinateSystem = gr.getCoordinateSystem();
	EXPECT_EQ(SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_LEFT, coordinateSystem);
}

TEST(cmzn_graphics_api, data_field)
{
	ZincTestSetup zinc;

	double values[] = { 1.0, 2.0, 3.0 };
	cmzn_field_id data_field = cmzn_fieldmodule_create_field_constant(zinc.fm,
		sizeof(values)/sizeof(double), values);
	EXPECT_NE(static_cast<cmzn_field *>(0), data_field);

	cmzn_graphics_id gr = cmzn_scene_create_graphics_points(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_data_field(gr, data_field));

	cmzn_field_id temp_data_field = cmzn_graphics_get_data_field(gr);
	EXPECT_EQ(temp_data_field, data_field);
	cmzn_field_destroy(&temp_data_field);
	cmzn_field_destroy(&data_field);

	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_data_field(gr, 0));
	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphics_get_data_field(gr));

	// check data_field removed as no longer used
	cmzn_fielditerator_id iter = cmzn_fieldmodule_create_fielditerator(zinc.fm);
	EXPECT_NE(static_cast<cmzn_fielditerator *>(0), iter);
	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_fielditerator_next(iter));
	cmzn_fielditerator_destroy(&iter);

	cmzn_graphics_destroy(&gr);
}

TEST(cmzn_graphics_api, material)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics(zinc.scene, CMZN_GRAPHICS_LINES);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	cmzn_graphics_material_module_id material_module = cmzn_context_get_material_module(zinc.context);
	cmzn_graphics_material_id default_material = cmzn_graphics_material_module_get_default_material(material_module);
	cmzn_graphics_material_id temp_material = cmzn_graphics_get_material(gr);
	EXPECT_EQ(default_material, temp_material);
	cmzn_graphics_material_destroy(&temp_material);
	cmzn_graphics_material_destroy(&default_material);

	cmzn_graphics_material_id material = cmzn_graphics_material_module_create_material(material_module);
	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_material(gr, material));
	temp_material = cmzn_graphics_get_material(gr);
	EXPECT_EQ(material, temp_material);
	cmzn_graphics_material_destroy(&temp_material);
	cmzn_graphics_material_destroy(&material);

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphics_set_material(gr, 0));

	cmzn_graphics_material_module_destroy(&material_module);
	cmzn_graphics_destroy(&gr);
}

TEST(cmzn_graphics_api, material_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsLines gr = zinc.scene.createGraphicsLines();
	EXPECT_TRUE(gr.isValid());

	GraphicsMaterialModule materialModule = zinc.context.getMaterialModule();
	GraphicsMaterial defaultMaterial = materialModule.getDefaultMaterial();
	GraphicsMaterial tempMaterial = gr.getMaterial();
	EXPECT_EQ(defaultMaterial.getId(), tempMaterial.getId());

	GraphicsMaterial material = materialModule.createMaterial();
	EXPECT_EQ(OK, gr.setMaterial(material));
	tempMaterial = gr.getMaterial();
	EXPECT_EQ(material.getId(), tempMaterial.getId());

	GraphicsMaterial noMaterial;
	EXPECT_EQ(ERROR_ARGUMENT, gr.setMaterial(noMaterial));
}

TEST(cmzn_graphics, render_line_width)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics(zinc.scene, CMZN_GRAPHICS_LINES);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	const double inWidth = 2.0;
	double outWidth;
	ASSERT_DOUBLE_EQ(1.0, outWidth = cmzn_graphics_get_render_line_width(gr));

	int result;
	ASSERT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_graphics_set_render_line_width(static_cast<cmzn_graphics_id>(0), 2.0));
	ASSERT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_graphics_set_render_line_width(gr, 0.0));

	ASSERT_EQ(CMZN_OK, result = cmzn_graphics_set_render_line_width(gr, inWidth));
	ASSERT_DOUBLE_EQ(inWidth, outWidth = cmzn_graphics_get_render_line_width(gr));

	cmzn_graphics_destroy(&gr);
}

TEST(ZincGraphics, RenderLineWidth)
{
	ZincTestSetupCpp zinc;

	GraphicsLines gr = zinc.scene.createGraphicsLines();
	EXPECT_TRUE(gr.isValid());

	const double inWidth = 2.0;
	double outWidth;
	ASSERT_DOUBLE_EQ(1.0, outWidth = gr.getRenderLineWidth());

	int result;
	ASSERT_EQ(ERROR_ARGUMENT, result = gr.setRenderLineWidth(0.0));

	ASSERT_EQ(OK, result = gr.setRenderLineWidth(inWidth));
	ASSERT_EQ(inWidth, outWidth = gr.getRenderLineWidth());
}

TEST(cmzn_graphics, render_point_size)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics(zinc.scene, CMZN_GRAPHICS_POINTS);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	const double inSize = 2.0;
	double outSize;
	ASSERT_DOUBLE_EQ(1.0, outSize = cmzn_graphics_get_render_point_size(gr));

	int result;
	ASSERT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_graphics_set_render_point_size(static_cast<cmzn_graphics_id>(0), 2.0));
	ASSERT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_graphics_set_render_point_size(gr, 0.0));

	ASSERT_EQ(CMZN_OK, result = cmzn_graphics_set_render_point_size(gr, inSize));
	ASSERT_DOUBLE_EQ(inSize, outSize = cmzn_graphics_get_render_point_size(gr));

	cmzn_graphics_destroy(&gr);
}

TEST(ZincGraphics, RenderPointSize)
{
	ZincTestSetupCpp zinc;

	GraphicsPoints gr = zinc.scene.createGraphicsPoints();
	EXPECT_TRUE(gr.isValid());

	const double inSize = 2.0;
	double outSize;
	ASSERT_DOUBLE_EQ(1.0, outSize = gr.getRenderPointSize());

	int result;
	ASSERT_EQ(ERROR_ARGUMENT, result = gr.setRenderPointSize(0.0));

	ASSERT_EQ(OK, result = gr.setRenderPointSize(inSize));
	ASSERT_EQ(inSize, outSize = gr.getRenderPointSize());
}

TEST(cmzn_graphics, select_mode)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics(zinc.scene, CMZN_GRAPHICS_SURFACES);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	cmzn_graphics_select_mode selectMode;
	ASSERT_EQ(CMZN_GRAPHICS_SELECT_ON, selectMode = cmzn_graphics_get_select_mode(gr));

	int result;
	ASSERT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_graphics_set_select_mode(static_cast<cmzn_graphics_id>(0), CMZN_GRAPHICS_DRAW_SELECTED));
	ASSERT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_graphics_set_select_mode(gr, CMZN_GRAPHICS_SELECT_MODE_INVALID));

	ASSERT_EQ(CMZN_OK, result = cmzn_graphics_set_select_mode(gr, CMZN_GRAPHICS_DRAW_SELECTED));
	ASSERT_EQ(CMZN_GRAPHICS_DRAW_SELECTED, selectMode = cmzn_graphics_get_select_mode(gr));

	cmzn_graphics_destroy(&gr);
}

TEST(ZincGraphics, selectMode)
{
	ZincTestSetupCpp zinc;

	GraphicsSurfaces gr = zinc.scene.createGraphicsSurfaces();
	EXPECT_TRUE(gr.isValid());

	Graphics::SelectMode selectMode;
	ASSERT_EQ(Graphics::SELECT_ON, selectMode = gr.getSelectMode());

	int result;
	ASSERT_EQ(ERROR_ARGUMENT, result = gr.setSelectMode(Graphics::SELECT_MODE_INVALID));

	ASSERT_EQ(OK, result = gr.setSelectMode(Graphics::DRAW_SELECTED));
	ASSERT_EQ(Graphics::DRAW_SELECTED, selectMode = gr.getSelectMode());
}

TEST(cmzn_graphics_api, selected_material)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics(zinc.scene, CMZN_GRAPHICS_LINES);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	cmzn_graphics_material_module_id material_module = cmzn_context_get_material_module(zinc.context);
	cmzn_graphics_material_id default_selected_material = cmzn_graphics_material_module_get_default_selected_material(material_module);
	cmzn_graphics_material_id temp_selected_material = cmzn_graphics_get_selected_material(gr);
	EXPECT_EQ(default_selected_material, temp_selected_material);
	cmzn_graphics_material_destroy(&temp_selected_material);
	cmzn_graphics_material_destroy(&default_selected_material);

	cmzn_graphics_material_id selected_material = cmzn_graphics_material_module_create_material(material_module);
	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_selected_material(gr, selected_material));
	temp_selected_material = cmzn_graphics_get_selected_material(gr);
	EXPECT_EQ(selected_material, temp_selected_material);
	cmzn_graphics_material_destroy(&temp_selected_material);
	cmzn_graphics_material_destroy(&selected_material);

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphics_set_selected_material(gr, 0));

	cmzn_graphics_material_module_destroy(&material_module);
	cmzn_graphics_destroy(&gr);
}

TEST(cmzn_graphics_api, selected_material_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsLines gr = zinc.scene.createGraphicsLines();
	EXPECT_TRUE(gr.isValid());

	GraphicsMaterialModule materialModule = zinc.context.getMaterialModule();
	GraphicsMaterial defaultSelectedMaterial = materialModule.getDefaultSelectedMaterial();
	GraphicsMaterial tempSelectedMaterial = gr.getSelectedMaterial();
	EXPECT_EQ(defaultSelectedMaterial.getId(), tempSelectedMaterial.getId());

	GraphicsMaterial selectedMaterial = materialModule.createMaterial();
	EXPECT_EQ(OK, gr.setSelectedMaterial(selectedMaterial));
	tempSelectedMaterial = gr.getSelectedMaterial();
	EXPECT_EQ(selectedMaterial.getId(), tempSelectedMaterial.getId());

	GraphicsMaterial noMaterial;
	EXPECT_EQ(ERROR_ARGUMENT, gr.setSelectedMaterial(noMaterial));
}

TEST(cmzn_graphics, name)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_surfaces(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	char *name = cmzn_graphics_get_name(gr);
	EXPECT_STREQ(static_cast<char *>(0), name);

	const char *nameBob = "Bob";
	int result;
	ASSERT_EQ(CMZN_OK, result = cmzn_graphics_set_name(gr, nameBob));
	name = cmzn_graphics_get_name(gr);
	EXPECT_STREQ(nameBob, name);
	cmzn_deallocate(name);

	ASSERT_EQ(CMZN_OK, result = cmzn_graphics_set_name(gr, static_cast<char *>(0)));
	name = cmzn_graphics_get_name(gr);
	EXPECT_STREQ(static_cast<char *>(0), name);

	cmzn_graphics_destroy(&gr);
}

TEST(ZincGraphics, name)
{
	ZincTestSetupCpp zinc;

	Graphics gr = zinc.scene.createGraphicsSurfaces();
	EXPECT_TRUE(gr.isValid());

	char *name = gr.getName();
	EXPECT_STREQ(static_cast<char *>(0), name);

	const char *nameBob = "Bob";
	int result;
	ASSERT_EQ(OK, result = gr.setName(nameBob));
	name = gr.getName();
	EXPECT_STREQ(nameBob, name);
	cmzn_deallocate(name);

	ASSERT_EQ(OK, result = result = gr.setName(static_cast<char *>(0)));
	name = gr.getName();
	EXPECT_STREQ(static_cast<char *>(0), name);
}

TEST(cmzn_graphics_api, spectrum)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_surfaces(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	cmzn_spectrummodule_id spectrummodule = cmzn_context_get_spectrummodule(zinc.context);
	EXPECT_NE(static_cast<cmzn_spectrummodule *>(0), spectrummodule);

	cmzn_spectrum_id spectrum = cmzn_spectrummodule_create_spectrum(spectrummodule);
	EXPECT_NE(static_cast<cmzn_spectrum *>(0), spectrum);

	cmzn_spectrummodule_destroy(&spectrummodule);

	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_spectrum(gr, spectrum));

	cmzn_spectrum_id temp_spectrum = cmzn_graphics_get_spectrum(gr);
	EXPECT_EQ(temp_spectrum, spectrum);
	cmzn_spectrum_destroy(&temp_spectrum);
	cmzn_spectrum_destroy(&spectrum);

	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_spectrum(gr, 0));
	EXPECT_EQ(static_cast<cmzn_spectrum *>(0), cmzn_graphics_get_spectrum(gr));

	cmzn_graphics_destroy(&gr);
}

TEST(cmzn_graphics_api, subgroup_field)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_points(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);
	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_domain_type(gr, CMZN_FIELD_DOMAIN_NODES));

	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphics_get_subgroup_field(gr));

	const double value = 1.0;
	cmzn_field_id subgroup_field = cmzn_fieldmodule_create_field_constant(zinc.fm, 1, &value);
	EXPECT_NE(static_cast<cmzn_field *>(0), subgroup_field);
	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_subgroup_field(gr, subgroup_field));

	// subgroup field must be scalar
	double values2[] = { 1.0, 2.0 };
	cmzn_field_id bad_subgroup_field = cmzn_fieldmodule_create_field_constant(zinc.fm,
		sizeof(values2)/sizeof(double), values2);
	EXPECT_NE(static_cast<cmzn_field *>(0), bad_subgroup_field);
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphics_set_subgroup_field(gr, bad_subgroup_field));
	cmzn_field_destroy(&bad_subgroup_field);

	// previous subgroup field should be left unchanged
	cmzn_field_id temp_subgroup_field = cmzn_graphics_get_subgroup_field(gr);
	EXPECT_EQ(subgroup_field, temp_subgroup_field);
	cmzn_field_destroy(&temp_subgroup_field);
	cmzn_field_destroy(&subgroup_field);

	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_subgroup_field(gr, 0));
	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphics_get_subgroup_field(gr));

	cmzn_graphics_destroy(&gr);
}

TEST(cmzn_graphics_api, subgroup_field_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsPoints gr = zinc.scene.createGraphicsPoints();
	EXPECT_TRUE(gr.isValid());
	EXPECT_EQ(OK, gr.setDomainType(Field::DOMAIN_NODES));

	Field tempSubgroupField = gr.getSubgroupField();
	EXPECT_FALSE(tempSubgroupField.isValid());

	const double value = 1.0;
	Field subgroupField = zinc.fm.createFieldConstant(1, &value);
	EXPECT_TRUE(subgroupField.isValid());
	EXPECT_EQ(OK, gr.setSubgroupField(subgroupField));

	// subgroup field must be scalar
	double values2[] = { 1.0, 2.0 };
	Field badSubgroupField = zinc.fm.createFieldConstant(2, values2);
	EXPECT_TRUE(badSubgroupField.isValid());
	EXPECT_EQ(ERROR_ARGUMENT, gr.setSubgroupField(badSubgroupField));

	// previous subgroup field should be left unchanged
	tempSubgroupField = gr.getSubgroupField();
	EXPECT_EQ(subgroupField.getId(), tempSubgroupField.getId());

	Field noField;
	EXPECT_EQ(OK, gr.setSubgroupField(noField)); // clear subgroup field
	tempSubgroupField = gr.getSubgroupField();
	EXPECT_FALSE(tempSubgroupField.isValid());
}

TEST(cmzn_graphics_api, tessellation)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_surfaces(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	cmzn_tessellationmodule_id tessellationmodule = cmzn_context_get_tessellationmodule(zinc.context);
	cmzn_tessellation_id default_tessellation = cmzn_tessellationmodule_get_default_tessellation(tessellationmodule);
	EXPECT_NE(static_cast<cmzn_tessellation_id>(0), default_tessellation);
	cmzn_tessellation_id temp_tessellation = cmzn_graphics_get_tessellation(gr);
	EXPECT_EQ(default_tessellation, temp_tessellation);
	cmzn_tessellation_destroy(&temp_tessellation);
	cmzn_tessellation_destroy(&default_tessellation);

	cmzn_tessellation_id tessellation = cmzn_tessellationmodule_create_tessellation(tessellationmodule);
	EXPECT_NE(static_cast<cmzn_tessellation_id>(0), tessellation);
	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_tessellation(gr, tessellation));
	temp_tessellation = cmzn_graphics_get_tessellation(gr);
	EXPECT_EQ(tessellation, temp_tessellation);
	cmzn_tessellation_destroy(&temp_tessellation);
	cmzn_tessellation_destroy(&tessellation);

	// can't remove tessellation
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphics_set_tessellation(gr, 0));

	cmzn_tessellationmodule_destroy(&tessellationmodule);
	cmzn_graphics_destroy(&gr);
}

TEST(cmzn_graphics_api, tessellation_cpp)
{
	ZincTestSetupCpp zinc;

	Graphics gr = zinc.scene.createGraphicsSurfaces();
	EXPECT_TRUE(gr.isValid());

	Tessellationmodule tessellationModule = zinc.context.getTessellationmodule();
	Tessellation defaultTessellation = tessellationModule.getDefaultTessellation();
	Tessellation tempTessellation = gr.getTessellation();
	EXPECT_EQ(defaultTessellation.getId(), tempTessellation.getId());

	Tessellation tessellation = tessellationModule.createTessellation();
	EXPECT_TRUE(tessellation.isValid());
	EXPECT_EQ(OK, gr.setTessellation(tessellation));
	tempTessellation = gr.getTessellation();
	EXPECT_EQ(tessellation.getId(), tempTessellation.getId());

	// can't remove tessellation
	Tessellation noTessellation;
	EXPECT_EQ(ERROR_ARGUMENT, gr.setTessellation(noTessellation));
}

TEST(cmzn_graphics_api, tessellation_field)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_contours(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphics_get_tessellation_field(gr));

	const double value = 1.0;
	cmzn_field_id tessellation_field = cmzn_fieldmodule_create_field_constant(zinc.fm, 1, &value);
	EXPECT_NE(static_cast<cmzn_field *>(0), tessellation_field);
	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_tessellation_field(gr, tessellation_field));

	cmzn_field_id temp_tessellation_field = cmzn_graphics_get_tessellation_field(gr);
	EXPECT_EQ(tessellation_field, temp_tessellation_field);
	cmzn_field_destroy(&temp_tessellation_field);
	cmzn_field_destroy(&tessellation_field);

	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_tessellation_field(gr, 0));
	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphics_get_tessellation_field(gr));

	cmzn_graphics_destroy(&gr);
}

TEST(cmzn_graphics_api, tessellation_field_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsContours gr = zinc.scene.createGraphicsContours();
	EXPECT_TRUE(gr.isValid());

	Field tempTessellationField = gr.getTessellationField();
	EXPECT_FALSE(tempTessellationField.isValid());

	const double value = 1.0;
	Field tessellationField = zinc.fm.createFieldConstant(1, &value);
	EXPECT_TRUE(tessellationField.isValid());
	EXPECT_EQ(OK, gr.setTessellationField(tessellationField));

	tempTessellationField = gr.getTessellationField();
	EXPECT_EQ(tessellationField.getId(), tempTessellationField.getId());

	Field noField;
	EXPECT_EQ(OK, gr.setTessellationField(noField)); // clear tessellation field
	tempTessellationField = gr.getTessellationField();
	EXPECT_FALSE(tempTessellationField.isValid());
}

TEST(cmzn_graphics_api, texture_coordinate_field)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_surfaces(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	const double values[] = { 1.0, 2.0, 3.0 };
	cmzn_field_id texture_coordinate_field = cmzn_fieldmodule_create_field_constant(zinc.fm,
		sizeof(values)/sizeof(double), values);
	EXPECT_NE(static_cast<cmzn_field *>(0), texture_coordinate_field);

	EXPECT_EQ((cmzn_field_id)0, cmzn_graphics_get_texture_coordinate_field(gr));
	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_texture_coordinate_field(gr, texture_coordinate_field));

	// coordinate field cannot have more than 3 components
	const double values4[] = { 1.0, 2.0, 3.0, 4.0 };
	cmzn_field_id bad_texture_coordinate_field = cmzn_fieldmodule_create_field_constant(zinc.fm,
		sizeof(values4)/sizeof(double), values4);
	EXPECT_NE(static_cast<cmzn_field *>(0), bad_texture_coordinate_field);
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphics_set_texture_coordinate_field(gr, bad_texture_coordinate_field));
	cmzn_field_destroy(&bad_texture_coordinate_field);

	// previous texture coordinate field should be left unchanged
	cmzn_field_id temp_texture_coordinate_field = cmzn_graphics_get_texture_coordinate_field(gr);
	EXPECT_EQ(texture_coordinate_field, temp_texture_coordinate_field);
	cmzn_field_destroy(&temp_texture_coordinate_field);
	cmzn_field_destroy(&texture_coordinate_field);

	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_texture_coordinate_field(gr, 0));
	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphics_get_texture_coordinate_field(gr));

	cmzn_graphics_destroy(&gr);
}

TEST(cmzn_graphics_api, texture_coordinate_field_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsSurfaces gr = zinc.scene.createGraphicsSurfaces();
	EXPECT_TRUE(gr.isValid());

	Field tempTextureCoordinateField = gr.getTextureCoordinateField();
	EXPECT_FALSE(tempTextureCoordinateField.isValid());

	const double values[] = { 1.0, 2.0, 3.0 };
	Field textureCoordinateField = zinc.fm.createFieldConstant(3, values);
	EXPECT_TRUE(textureCoordinateField.isValid());
	EXPECT_EQ(OK, gr.setTextureCoordinateField(textureCoordinateField));

	// coordinate field cannot have more than 3 components
	const double values4[] = { 1.0, 2.0, 3.0, 4.0 };
	Field badTextureCoordinateField = zinc.fm.createFieldConstant(4, values4);
	EXPECT_TRUE(badTextureCoordinateField.isValid());
	EXPECT_EQ(ERROR_ARGUMENT, gr.setTextureCoordinateField(badTextureCoordinateField));

	// previous texture coordinate field should be left unchanged
	tempTextureCoordinateField = gr.getTextureCoordinateField();
	EXPECT_EQ(textureCoordinateField.getId(), tempTextureCoordinateField.getId());

	Field noField;
	EXPECT_EQ(OK, gr.setTextureCoordinateField(noField)); // clear texture coordinate field
	tempTextureCoordinateField = gr.getTextureCoordinateField();
	EXPECT_FALSE(tempTextureCoordinateField.isValid());
}

TEST(cmzn_graphics_api, point_attributes_glyph)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_points(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	cmzn_graphicspointattributes_id pointattr = cmzn_graphics_get_graphicspointattributes(gr);
	EXPECT_NE(static_cast<cmzn_graphicspointattributes *>(0), pointattr);

	cmzn_glyph_id glyph = cmzn_glyphmodule_get_default_point_glyph(zinc.glyphmodule);
	EXPECT_NE((cmzn_glyph_id)0, glyph);
	cmzn_glyph_id temp_glyph = cmzn_graphicspointattributes_get_glyph(pointattr);
	EXPECT_EQ(glyph, temp_glyph);
	cmzn_glyph_destroy(&temp_glyph);
	cmzn_glyph_destroy(&glyph);
	EXPECT_EQ(CMZN_GLYPH_POINT, cmzn_graphicspointattributes_get_glyph_type(pointattr));

	glyph = cmzn_glyphmodule_find_glyph_by_name(zinc.glyphmodule, "sphere");
	EXPECT_NE((cmzn_glyph_id)0, glyph);
	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_set_glyph(pointattr, glyph));
	temp_glyph = cmzn_graphicspointattributes_get_glyph(pointattr);
	EXPECT_EQ(glyph, temp_glyph);
	cmzn_glyph_destroy(&temp_glyph);
	cmzn_glyph_destroy(&glyph);
	EXPECT_EQ(CMZN_GLYPH_SPHERE, cmzn_graphicspointattributes_get_glyph_type(pointattr));

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_set_glyph_type(pointattr, CMZN_GLYPH_TYPE_INVALID));
	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_set_glyph_type(pointattr, CMZN_GLYPH_CUBE_SOLID));
	EXPECT_EQ(CMZN_GLYPH_CUBE_SOLID, cmzn_graphicspointattributes_get_glyph_type(pointattr));

	EXPECT_EQ(CMZN_GLYPH_REPEAT_NONE, cmzn_graphicspointattributes_get_glyph_repeat_mode(pointattr));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_set_glyph_repeat_mode(0, CMZN_GLYPH_REPEAT_MIRROR));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_set_glyph_repeat_mode(pointattr, CMZN_GLYPH_REPEAT_MODE_INVALID));
	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_set_glyph_repeat_mode(pointattr, CMZN_GLYPH_REPEAT_MIRROR));
	EXPECT_EQ(CMZN_GLYPH_REPEAT_MIRROR, cmzn_graphicspointattributes_get_glyph_repeat_mode(pointattr));
	double fieldValues[] = { 0.3, 0.4, 0.5 };
	cmzn_field_id field = cmzn_fieldmodule_create_field_constant(zinc.fm, 3, fieldValues);
	EXPECT_NE(static_cast<cmzn_field *>(0), field);
	cmzn_field_id temp_field = 0;

	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphicspointattributes_get_orientation_scale_field(pointattr));
	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_set_orientation_scale_field(pointattr, field));
	temp_field = cmzn_graphicspointattributes_get_orientation_scale_field(pointattr);
	EXPECT_EQ(temp_field, field);
	cmzn_field_destroy(&temp_field);
	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_set_orientation_scale_field(pointattr, 0));
	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphicspointattributes_get_orientation_scale_field(pointattr));

	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphicspointattributes_get_signed_scale_field(pointattr));
	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_set_signed_scale_field(pointattr, field));
	temp_field = cmzn_graphicspointattributes_get_signed_scale_field(pointattr);
	EXPECT_EQ(temp_field, field);
	cmzn_field_destroy(&temp_field);
	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_set_signed_scale_field(pointattr, 0));
	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphicspointattributes_get_signed_scale_field(pointattr));

	cmzn_field_destroy(&field);

	const double values[] = { 0.5, 1.2 };
	double outputValues[3];

	// check default values = 0.0
	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_get_base_size(pointattr, 3, outputValues));
	EXPECT_EQ(0.0, outputValues[0]);
	EXPECT_EQ(0.0, outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_set_base_size(pointattr, 0, values));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_set_base_size(pointattr, 2, 0));
	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_set_base_size(pointattr, 2, values));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_get_base_size(pointattr, 0, outputValues));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_get_base_size(pointattr, 3, 0));
	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_get_base_size(pointattr, 3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[1], outputValues[2]);

	// check default values = 0.0
	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_get_glyph_offset(pointattr, 3, outputValues));
	EXPECT_EQ(0.0, outputValues[0]);
	EXPECT_EQ(0.0, outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_set_glyph_offset(pointattr, 0, values));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_set_glyph_offset(pointattr, 2, 0));
	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_set_glyph_offset(pointattr, 2, values));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_get_glyph_offset(pointattr, 0, outputValues));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_get_glyph_offset(pointattr, 3, 0));
	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_get_glyph_offset(pointattr, 3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);

	// check default values = 1.0
	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_get_scale_factors(pointattr, 3, outputValues));
	EXPECT_EQ(1.0, outputValues[0]);
	EXPECT_EQ(1.0, outputValues[1]);
	EXPECT_EQ(1.0, outputValues[2]);
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_set_scale_factors(pointattr, 0, values));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_set_scale_factors(pointattr, 2, 0));
	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_set_scale_factors(pointattr, 2, values));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_get_scale_factors(pointattr, 0, outputValues));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_get_scale_factors(pointattr, 3, 0));
	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_get_scale_factors(pointattr, 3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[1], outputValues[2]);

	cmzn_graphicspointattributes_destroy(&pointattr);
	cmzn_graphics_destroy(&gr);
}

TEST(cmzn_graphics_api, point_attributes_glyph_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsPoints gr = zinc.scene.createGraphicsPoints();
	EXPECT_TRUE(gr.isValid());
	// test can assign to base class handle
	Graphics tmp(gr);
	EXPECT_TRUE(tmp.isValid());

	Graphicspointattributes pointattr = gr.getGraphicspointattributes();
	EXPECT_TRUE(pointattr.isValid());

	Glyph glyph = zinc.glyphmodule.getDefaultPointGlyph();
	EXPECT_TRUE(glyph.isValid());
	Glyph tempGlyph = pointattr.getGlyph();
	EXPECT_EQ(glyph.getId(), tempGlyph.getId());
	EXPECT_EQ(Glyph::POINT, pointattr.getGlyphType());

	glyph = zinc.glyphmodule.findGlyphByName("sphere");
	EXPECT_TRUE(glyph.isValid());
	EXPECT_EQ(OK, pointattr.setGlyph(glyph));
	tempGlyph = pointattr.getGlyph();
	EXPECT_EQ(glyph.getId(), tempGlyph.getId());
	EXPECT_EQ(Glyph::SPHERE, pointattr.getGlyphType());

	EXPECT_EQ(ERROR_ARGUMENT, pointattr.setGlyphType(Glyph::TYPE_INVALID));
	EXPECT_EQ(OK, pointattr.setGlyphType(Glyph::CUBE_SOLID));
	EXPECT_EQ(Glyph::CUBE_SOLID, pointattr.getGlyphType());

	EXPECT_EQ(Glyph::REPEAT_NONE, pointattr.getGlyphRepeatMode());
	EXPECT_EQ(ERROR_ARGUMENT, pointattr.setGlyphRepeatMode(Glyph::REPEAT_MODE_INVALID));
	EXPECT_EQ(OK, pointattr.setGlyphRepeatMode(Glyph::REPEAT_MIRROR));
	EXPECT_EQ(Glyph::REPEAT_MIRROR, pointattr.getGlyphRepeatMode());

	double fieldValues[] = { 0.3, 0.4, 0.5 };
	Field field = zinc.fm.createFieldConstant(sizeof(fieldValues)/sizeof(double), fieldValues);
	EXPECT_TRUE(field.isValid());
	Field tempField;

	EXPECT_FALSE(pointattr.getOrientationScaleField().isValid());
	EXPECT_EQ(OK, pointattr.setOrientationScaleField(field));
	tempField = pointattr.getOrientationScaleField();
	EXPECT_EQ(tempField.getId(), field.getId());
	Field noField;
	EXPECT_EQ(OK, pointattr.setOrientationScaleField(noField)); // clear field
	EXPECT_FALSE(pointattr.getOrientationScaleField().isValid());

	EXPECT_FALSE(pointattr.getSignedScaleField().isValid());
	EXPECT_EQ(OK, pointattr.setSignedScaleField(field));
	tempField = pointattr.getSignedScaleField();
	EXPECT_EQ(tempField.getId(), field.getId());
	EXPECT_EQ(OK, pointattr.setSignedScaleField(noField)); // clear field
	EXPECT_FALSE(pointattr.getSignedScaleField().isValid());

	const double values[] = { 0.5, 1.2 };
	double outputValues[3];

	EXPECT_EQ(ERROR_ARGUMENT, pointattr.setBaseSize(0, values));
	EXPECT_EQ(ERROR_ARGUMENT, pointattr.setBaseSize(2, 0));
	EXPECT_EQ(OK, pointattr.setBaseSize(2, values));
	EXPECT_EQ(ERROR_ARGUMENT, pointattr.getBaseSize(0, outputValues));
	EXPECT_EQ(ERROR_ARGUMENT, pointattr.getBaseSize(3, 0));
	EXPECT_EQ(OK, pointattr.getBaseSize(3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[1], outputValues[2]);

	EXPECT_EQ(ERROR_ARGUMENT, pointattr.setGlyphOffset(0, values));
	EXPECT_EQ(ERROR_ARGUMENT, pointattr.setGlyphOffset(2, 0));
	EXPECT_EQ(OK, pointattr.setGlyphOffset(2, values));
	EXPECT_EQ(ERROR_ARGUMENT, pointattr.getGlyphOffset(0, outputValues));
	EXPECT_EQ(ERROR_ARGUMENT, pointattr.getGlyphOffset(3, 0));
	EXPECT_EQ(OK, pointattr.getGlyphOffset(3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);

	EXPECT_EQ(ERROR_ARGUMENT, pointattr.setScaleFactors(0, values));
	EXPECT_EQ(ERROR_ARGUMENT, pointattr.setScaleFactors(2, 0));
	EXPECT_EQ(OK, pointattr.setScaleFactors(2, values));
	EXPECT_EQ(ERROR_ARGUMENT, pointattr.getScaleFactors(0, outputValues));
	EXPECT_EQ(ERROR_ARGUMENT, pointattr.getScaleFactors(3, 0));
	EXPECT_EQ(OK, pointattr.getScaleFactors(3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[1], outputValues[2]);
}

TEST(cmzn_graphics_api, point_attributes_label)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_points(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	cmzn_graphicspointattributes_id pointattr = cmzn_graphics_get_graphicspointattributes(gr);
	EXPECT_NE(static_cast<cmzn_graphicspointattributes *>(0), pointattr);

	double values[] = { 1.0, 2.0, 3.0 };
	cmzn_field_id label_field = cmzn_fieldmodule_create_field_constant(zinc.fm,
		sizeof(values)/sizeof(double), values);
	EXPECT_NE(static_cast<cmzn_field *>(0), label_field);

	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_set_label_field(pointattr, label_field));

	cmzn_field_id temp_label_field = cmzn_graphicspointattributes_get_label_field(pointattr);
	EXPECT_EQ(temp_label_field, label_field);
	cmzn_field_destroy(&temp_label_field);
	cmzn_field_destroy(&label_field);

	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_set_label_field(pointattr, 0));
	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphicspointattributes_get_label_field(pointattr));

	double outputValues[3];
	// check default values = 0.0
	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_get_label_offset(pointattr, 3, outputValues));
	EXPECT_EQ(0.0, outputValues[0]);
	EXPECT_EQ(0.0, outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_set_label_offset(pointattr, 0, values));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_set_label_offset(pointattr, 2, 0));
	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_set_label_offset(pointattr, 2, values));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_get_label_offset(pointattr, 0, outputValues));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_get_label_offset(pointattr, 3, 0));
	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_get_label_offset(pointattr, 3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);

	// should start with a default font
	cmzn_font_id font = cmzn_graphicspointattributes_get_font(pointattr);
	EXPECT_NE(static_cast<cmzn_font *>(0), font);

	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_set_font(pointattr, 0));
	EXPECT_EQ(static_cast<cmzn_font *>(0), cmzn_graphicspointattributes_get_font(pointattr));

	EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_set_font(pointattr, font));

	const char *text = "ABC";
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_set_label_text(0, 1, text));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicspointattributes_set_label_text(pointattr, 0, text));
	EXPECT_EQ((char *)0, cmzn_graphicspointattributes_get_label_text(0, 1));
	EXPECT_EQ((char *)0, cmzn_graphicspointattributes_get_label_text(pointattr, 0));
	for (int labelNumber = 1; labelNumber <= 3; ++labelNumber)
	{
		char *outText;
		EXPECT_EQ((char *)0, cmzn_graphicspointattributes_get_label_text(pointattr, labelNumber));
		EXPECT_EQ(CMZN_OK, cmzn_graphicspointattributes_set_label_text(pointattr, labelNumber, text));
		outText = cmzn_graphicspointattributes_get_label_text(pointattr, labelNumber);
		EXPECT_STREQ(text, outText);
		cmzn_deallocate(outText);
	}
	cmzn_font_destroy(&font);
	cmzn_graphicspointattributes_destroy(&pointattr);
	cmzn_graphics_destroy(&gr);
}

TEST(cmzn_graphics_api, point_attributes_label_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsPoints gr = zinc.scene.createGraphicsPoints();
	EXPECT_TRUE(gr.isValid());

	Graphicspointattributes pointattr = gr.getGraphicspointattributes();
	EXPECT_TRUE(pointattr.isValid());

	double values[] = { 1.0, 2.0, 3.0 };
	Field labelField = zinc.fm.createFieldConstant(sizeof(values)/sizeof(double), values);
	EXPECT_TRUE(labelField.isValid());

	EXPECT_EQ(OK, pointattr.setLabelField(labelField));

	Field tempLabelField = pointattr.getLabelField();
	EXPECT_EQ(tempLabelField.getId(), labelField.getId());

	Field noField;
	EXPECT_EQ(OK, pointattr.setLabelField(noField)); // clear label field
	EXPECT_FALSE(pointattr.getLabelField().isValid());

	double outputValues[3];
	EXPECT_EQ(ERROR_ARGUMENT, pointattr.setLabelOffset(0, values));
	EXPECT_EQ(ERROR_ARGUMENT, pointattr.setLabelOffset(2, 0));
	EXPECT_EQ(OK, pointattr.setLabelOffset(2, values));
	EXPECT_EQ(ERROR_ARGUMENT, pointattr.getLabelOffset(0, outputValues));
	EXPECT_EQ(ERROR_ARGUMENT, pointattr.getLabelOffset(3, 0));
	EXPECT_EQ(OK, pointattr.getLabelOffset(3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);

	// should start with a default font
	Font font = pointattr.getFont();
	EXPECT_TRUE(font.isValid());

	Font noFont;
	EXPECT_EQ(OK, pointattr.setFont(noFont)); // clear font
	EXPECT_FALSE(pointattr.getFont().isValid());

	EXPECT_EQ(OK, pointattr.setFont(font));

	const char *text = "ABC";
	EXPECT_EQ(ERROR_ARGUMENT, pointattr.setLabelText(0, text));
	EXPECT_EQ((char *)0, pointattr.getLabelText(0));
	for (int labelNumber = 1; labelNumber <= 3; ++labelNumber)
	{
		char *outText;
		EXPECT_EQ((char *)0, pointattr.getLabelText(labelNumber));
		EXPECT_EQ(OK, pointattr.setLabelText(labelNumber, text));
		outText = pointattr.getLabelText(labelNumber);
		EXPECT_STREQ(text, outText);
		cmzn_deallocate(outText);
	}
}

TEST(cmzn_graphics, render_polygon_mode)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics(zinc.scene, CMZN_GRAPHICS_SURFACES);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	cmzn_graphics_render_polygon_mode renderPolygonMode;
	ASSERT_EQ(CMZN_GRAPHICS_RENDER_POLYGON_SHADED, renderPolygonMode = cmzn_graphics_get_render_polygon_mode(gr));

	int result;
	ASSERT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_graphics_set_render_polygon_mode(static_cast<cmzn_graphics_id>(0), CMZN_GRAPHICS_RENDER_POLYGON_SHADED));
	ASSERT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_graphics_set_render_polygon_mode(gr, CMZN_GRAPHICS_RENDER_POLYGON_MODE_INVALID));

	ASSERT_EQ(CMZN_OK, result = cmzn_graphics_set_render_polygon_mode(gr, CMZN_GRAPHICS_RENDER_POLYGON_WIREFRAME));
	ASSERT_EQ(CMZN_GRAPHICS_RENDER_POLYGON_WIREFRAME, renderPolygonMode = cmzn_graphics_get_render_polygon_mode(gr));

	cmzn_graphics_destroy(&gr);
}

TEST(ZincGraphics, RenderPolygonMode)
{
	ZincTestSetupCpp zinc;

	GraphicsSurfaces gr = zinc.scene.createGraphicsSurfaces();
	EXPECT_TRUE(gr.isValid());

	Graphics::RenderPolygonMode renderPolygonMode;
	ASSERT_EQ(Graphics::RENDER_POLYGON_SHADED, renderPolygonMode = gr.getRenderPolygonMode());

	int result;
	ASSERT_EQ(ERROR_ARGUMENT, result = gr.setRenderPolygonMode(Graphics::RENDER_POLYGON_MODE_INVALID));

	ASSERT_EQ(OK, result = gr.setRenderPolygonMode(Graphics::RENDER_POLYGON_WIREFRAME));
	ASSERT_EQ(Graphics::RENDER_POLYGON_WIREFRAME, renderPolygonMode = gr.getRenderPolygonMode());
}

TEST(cmzn_graphics_api, line_attributes)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics(zinc.scene, CMZN_GRAPHICS_LINES);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	cmzn_graphicslineattributes_id lineattr = cmzn_graphics_get_graphicslineattributes(gr);
	EXPECT_NE(static_cast<cmzn_graphicslineattributes *>(0), lineattr);
	EXPECT_EQ(CMZN_GRAPHICSLINEATTRIBUTES_SHAPE_LINE, cmzn_graphicslineattributes_get_shape(lineattr));
	EXPECT_EQ(CMZN_OK, cmzn_graphicslineattributes_set_shape(lineattr, CMZN_GRAPHICSLINEATTRIBUTES_SHAPE_CIRCLE_EXTRUSION));
	EXPECT_EQ(CMZN_GRAPHICSLINEATTRIBUTES_SHAPE_CIRCLE_EXTRUSION, cmzn_graphicslineattributes_get_shape(lineattr));

	double value = 1.0;
	cmzn_field_id orientation_scale_field = cmzn_fieldmodule_create_field_constant(zinc.fm, 1, &value);
	EXPECT_NE(static_cast<cmzn_field *>(0), orientation_scale_field);

	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphicslineattributes_get_orientation_scale_field(lineattr));
	EXPECT_EQ(CMZN_OK, cmzn_graphicslineattributes_set_orientation_scale_field(lineattr, orientation_scale_field));

	cmzn_field_id temp_orientation_scale_field = cmzn_graphicslineattributes_get_orientation_scale_field(lineattr);
	EXPECT_EQ(temp_orientation_scale_field, orientation_scale_field);
	cmzn_field_destroy(&temp_orientation_scale_field);
	cmzn_field_destroy(&orientation_scale_field);

	EXPECT_EQ(CMZN_OK, cmzn_graphicslineattributes_set_orientation_scale_field(lineattr, 0));
	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphicslineattributes_get_orientation_scale_field(lineattr));

	const double values[] = { 0.5, 1.2 };
	double outputValues[2];

	// check default values = 0.0
	EXPECT_EQ(CMZN_OK, cmzn_graphicslineattributes_get_base_size(lineattr, 2, outputValues));
	EXPECT_EQ(0.0, outputValues[0]);
	EXPECT_EQ(0.0, outputValues[1]);
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicslineattributes_set_base_size(lineattr, 0, values));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicslineattributes_set_base_size(lineattr, 2, 0));
	EXPECT_EQ(CMZN_OK, cmzn_graphicslineattributes_set_base_size(lineattr, 2, values));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicslineattributes_get_base_size(lineattr, 0, outputValues));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicslineattributes_get_base_size(lineattr, 2, 0));
	EXPECT_EQ(CMZN_OK, cmzn_graphicslineattributes_get_base_size(lineattr, 2, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[0], outputValues[1]); // lines/cylinders currently constrained to equal values

	// check default values = 1.0
	EXPECT_EQ(CMZN_OK, cmzn_graphicslineattributes_get_scale_factors(lineattr, 2, outputValues));
	EXPECT_EQ(1.0, outputValues[0]);
	EXPECT_EQ(1.0, outputValues[1]);
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicslineattributes_set_scale_factors(lineattr, 0, values));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicslineattributes_set_scale_factors(lineattr, 2, 0));
	EXPECT_EQ(CMZN_OK, cmzn_graphicslineattributes_set_scale_factors(lineattr, 2, values));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicslineattributes_get_scale_factors(lineattr, 0, outputValues));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicslineattributes_get_scale_factors(lineattr, 2, 0));
	EXPECT_EQ(CMZN_OK, cmzn_graphicslineattributes_get_scale_factors(lineattr, 2, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[0], outputValues[1]); // lines/cylinders currently constrained to equal values

	cmzn_graphicslineattributes_destroy(&lineattr);
	cmzn_graphics_destroy(&gr);
}

TEST(cmzn_graphics_api, line_attributes_cpp)
{
	ZincTestSetupCpp zinc;

	Graphics gr = zinc.scene.createGraphics(Graphics::LINES);
	EXPECT_TRUE(gr.isValid());

	Graphicslineattributes lineattr = gr.getGraphicslineattributes();
	EXPECT_TRUE(lineattr.isValid());
	EXPECT_EQ(Graphicslineattributes::SHAPE_LINE, lineattr.getShape());
	EXPECT_EQ(OK, lineattr.setShape(Graphicslineattributes::SHAPE_CIRCLE_EXTRUSION));
	EXPECT_EQ(Graphicslineattributes::SHAPE_CIRCLE_EXTRUSION, lineattr.getShape());

	double value = 1.0;
	Field orientationScaleField = zinc.fm.createFieldConstant(1, &value);
	EXPECT_TRUE(orientationScaleField.isValid());

	EXPECT_FALSE(lineattr.getOrientationScaleField().isValid());
	EXPECT_EQ(OK, lineattr.setOrientationScaleField(orientationScaleField));

	Field tempOrientationScaleField = lineattr.getOrientationScaleField();
	EXPECT_EQ(tempOrientationScaleField.getId(), orientationScaleField.getId());

	Field noField;
	EXPECT_EQ(OK, lineattr.setOrientationScaleField(noField)); // clear field
	EXPECT_FALSE(lineattr.getOrientationScaleField().isValid());

	const double values[] = { 0.5, 1.2 };
	double outputValues[2];

	EXPECT_EQ(ERROR_ARGUMENT, lineattr.setBaseSize(0, values));
	EXPECT_EQ(ERROR_ARGUMENT, lineattr.setBaseSize(2, 0));
	EXPECT_EQ(OK, lineattr.setBaseSize(2, values));
	EXPECT_EQ(ERROR_ARGUMENT, lineattr.getBaseSize(0, outputValues));
	EXPECT_EQ(ERROR_ARGUMENT, lineattr.getBaseSize(2, 0));
	EXPECT_EQ(OK, lineattr.getBaseSize(2, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[0], outputValues[1]); // lines/cylinders currently constrained to equal values

	EXPECT_EQ(ERROR_ARGUMENT, lineattr.setScaleFactors(0, values));
	EXPECT_EQ(ERROR_ARGUMENT, lineattr.setScaleFactors(2, 0));
	EXPECT_EQ(OK, lineattr.setScaleFactors(2, values));
	EXPECT_EQ(ERROR_ARGUMENT, lineattr.getScaleFactors(0, outputValues));
	EXPECT_EQ(ERROR_ARGUMENT, lineattr.getScaleFactors(2, 0));
	EXPECT_EQ(OK, lineattr.getScaleFactors(2, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[0], outputValues[1]); // lines/cylinders currently constrained to equal values
}

TEST(cmzn_graphics_api, visibility_flag)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_surfaces(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	EXPECT_TRUE(cmzn_graphics_get_visibility_flag(gr));
	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_visibility_flag(gr, false));
	EXPECT_FALSE(cmzn_graphics_get_visibility_flag(gr));

	cmzn_graphics_destroy(&gr);
}

TEST(cmzn_graphics_api, visibility_flag_cpp)
{
	ZincTestSetupCpp zinc;

	Graphics gr = zinc.scene.createGraphicsSurfaces();
	EXPECT_TRUE(gr.isValid());

	EXPECT_TRUE(gr.getVisibilityFlag());
	EXPECT_EQ(OK, gr.setVisibilityFlag(false));
	EXPECT_FALSE(gr.getVisibilityFlag());
}

TEST(cmzn_graphics_api, sampling_attributes)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_streamlines(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);

	cmzn_graphicssamplingattributes_id sampling = cmzn_graphics_get_graphicssamplingattributes(gr);
	EXPECT_NE(static_cast<cmzn_graphicssamplingattributes *>(0), sampling);

	EXPECT_EQ(CMZN_ELEMENT_POINT_SAMPLE_CELL_CENTRES, cmzn_graphicssamplingattributes_get_mode(sampling));
	EXPECT_EQ(CMZN_OK, cmzn_graphicssamplingattributes_set_mode(sampling, CMZN_ELEMENT_POINT_SAMPLE_CELL_POISSON));
	EXPECT_EQ(CMZN_ELEMENT_POINT_SAMPLE_CELL_POISSON, cmzn_graphicssamplingattributes_get_mode(sampling));

	double value = 1.0;
	cmzn_field_id density_field = cmzn_fieldmodule_create_field_constant(zinc.fm, 1, &value);
	EXPECT_NE(static_cast<cmzn_field *>(0), density_field);

	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphicssamplingattributes_get_density_field(sampling));
	EXPECT_EQ(CMZN_OK, cmzn_graphicssamplingattributes_set_density_field(sampling, density_field));

	cmzn_field_id temp_density_field = cmzn_graphicssamplingattributes_get_density_field(sampling);
	EXPECT_EQ(density_field, temp_density_field);
	cmzn_field_destroy(&temp_density_field);
	cmzn_field_destroy(&density_field);
	EXPECT_EQ(CMZN_OK, cmzn_graphicssamplingattributes_set_density_field(sampling, static_cast<cmzn_field_id>(0)));
	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphicssamplingattributes_get_density_field(sampling));

	EXPECT_EQ(CMZN_OK, cmzn_graphicssamplingattributes_set_mode(sampling, CMZN_ELEMENT_POINT_SAMPLE_SET_LOCATION));

	const double values[] = { 0.5, 0.20, 0.8 };
	double outputValues[3];
	// check default values = 0.0
	EXPECT_EQ(CMZN_OK, cmzn_graphicssamplingattributes_get_location(sampling, 3, outputValues));
	EXPECT_EQ(0.0, outputValues[0]);
	EXPECT_EQ(0.0, outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicssamplingattributes_set_location(sampling, 0, values));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicssamplingattributes_set_location(sampling, 2, 0));
	EXPECT_EQ(CMZN_OK, cmzn_graphicssamplingattributes_set_location(sampling, 3, values));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicssamplingattributes_get_location(sampling, 0, outputValues));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphicssamplingattributes_get_location(sampling, 3, 0));
	EXPECT_EQ(CMZN_OK, cmzn_graphicssamplingattributes_get_location(sampling, 3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[2], outputValues[2]);

	cmzn_graphicssamplingattributes_destroy(&sampling);
	cmzn_graphics_destroy(&gr);
}

TEST(cmzn_graphics_api, sampling_attributes_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsStreamlines gr = zinc.scene.createGraphicsStreamlines();
	EXPECT_TRUE(gr.isValid());

	Graphicsamplingattributes sampling = gr.getGraphicsamplingattributes();
	EXPECT_TRUE(sampling.isValid());

	EXPECT_EQ(Element::POINT_SAMPLE_CELL_CENTRES, sampling.getMode());
	EXPECT_EQ(OK, sampling.setMode(Element::POINT_SAMPLE_CELL_POISSON));
	EXPECT_EQ(Element::POINT_SAMPLE_CELL_POISSON, sampling.getMode());

	double value = 1.0;
	Field densityField = zinc.fm.createFieldConstant(1, &value);
	EXPECT_TRUE(densityField.isValid());

	Field tempField = sampling.getDensityField();
	EXPECT_FALSE(tempField.isValid());
	EXPECT_EQ(OK, sampling.setDensityField(densityField));
	tempField = sampling.getDensityField();
	EXPECT_EQ(densityField.getId(), tempField.getId());

	Field noField;
	EXPECT_EQ(OK, sampling.setDensityField(noField));
	tempField = sampling.getDensityField();
	EXPECT_FALSE(tempField.isValid());

	EXPECT_EQ(OK, sampling.setMode(Element::POINT_SAMPLE_SET_LOCATION));

	const double values[] = { 0.5, 0.20, 0.8 };
	double outputValues[3];
	// check default values = 0.0
	EXPECT_EQ(OK, sampling.getLocation(3, outputValues));
	EXPECT_EQ(0.0, outputValues[0]);
	EXPECT_EQ(0.0, outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);
	EXPECT_EQ(ERROR_ARGUMENT, sampling.setLocation(0, values));
	EXPECT_EQ(ERROR_ARGUMENT, sampling.setLocation(2, 0));
	EXPECT_EQ(OK, sampling.setLocation(3, values));
	EXPECT_EQ(ERROR_ARGUMENT, sampling.getLocation(0, outputValues));
	EXPECT_EQ(ERROR_ARGUMENT, sampling.getLocation(3, 0));
	EXPECT_EQ(OK, sampling.getLocation(3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[2], outputValues[2]);
}
