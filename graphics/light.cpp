/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/light.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/light.hpp"

TEST(cmzn_lightmodule_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_lightmodule_id lm = cmzn_context_get_lightmodule(zinc.context);
	EXPECT_NE(static_cast<cmzn_lightmodule *>(0), lm);

	int result = cmzn_lightmodule_begin_change(lm);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_light_id light = cmzn_lightmodule_get_default_light(lm);
	EXPECT_NE(static_cast<cmzn_light *>(0), light);
	double float_values[3] = {0.0, 0.0, 0.0};
	result = cmzn_light_get_colour(light, &float_values[0]);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_DOUBLE_EQ(0.9, float_values[0]);
	EXPECT_DOUBLE_EQ(0.9, float_values[1]);
	EXPECT_DOUBLE_EQ(0.9, float_values[2]);

	result = cmzn_light_get_direction(light, &float_values[0]);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_DOUBLE_EQ(0.0, float_values[0]);
	EXPECT_DOUBLE_EQ(-0.5, float_values[1]);
	EXPECT_DOUBLE_EQ(-1.0, float_values[2]);

	result = cmzn_light_get_position(light, &float_values[0]);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_DOUBLE_EQ(0.0, float_values[0]);
	EXPECT_DOUBLE_EQ(0.0, float_values[1]);
	EXPECT_DOUBLE_EQ(0.0, float_values[2]);

	enum cmzn_light_type light_type = cmzn_light_get_type(light);
	EXPECT_EQ(CMZN_LIGHT_TYPE_DIRECTIONAL, light_type);
	EXPECT_DOUBLE_EQ(cmzn_light_get_constant_attenuation(light), 1.0);
	EXPECT_DOUBLE_EQ(cmzn_light_get_linear_attenuation(light), 0.0);
	EXPECT_DOUBLE_EQ(cmzn_light_get_quadratic_attenuation(light), 0.0);
	EXPECT_DOUBLE_EQ(cmzn_light_get_spot_cutoff(light), 90.0);
	EXPECT_DOUBLE_EQ(cmzn_light_get_spot_exponent(light), 0.0);
	EXPECT_EQ(cmzn_light_is_enabled(light), 1);
	EXPECT_EQ(cmzn_light_destroy(&light), result);

	light = cmzn_lightmodule_get_default_ambient_light(lm);
	EXPECT_NE(static_cast<cmzn_light *>(0), light);
	result = cmzn_light_get_colour(light, &float_values[0]);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_DOUBLE_EQ(0.1, float_values[0]);
	EXPECT_DOUBLE_EQ(0.1, float_values[1]);
	EXPECT_DOUBLE_EQ(0.1, float_values[2]);
	light_type = cmzn_light_get_type(light);
	EXPECT_EQ(CMZN_LIGHT_TYPE_AMBIENT, light_type);
	enum cmzn_light_render_viewer_mode viewer_mode = cmzn_light_get_render_viewer_mode(light);
	EXPECT_EQ(CMZN_LIGHT_RENDER_VIEWER_MODE_INFINITE, viewer_mode);
	enum cmzn_light_render_side render_side = cmzn_light_get_render_side(light);
	EXPECT_EQ(CMZN_LIGHT_RENDER_SIDE_DOUBLE, render_side);
	EXPECT_EQ(cmzn_light_destroy(&light), result);

	// following should destroy default light as not managed and not used
	// otherwise it isn't possible to create a light named "default" below
	result = cmzn_lightmodule_set_default_light(lm, 0);
	EXPECT_EQ(CMZN_OK, result);

	light = cmzn_lightmodule_create_light(lm);
	EXPECT_NE(static_cast<cmzn_light *>(0), light);

	result = cmzn_light_set_name(light, "default");
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_lightmodule_end_change(lm);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_lightmodule_set_default_light(lm, light);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_light_set_managed(light, 1);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_light_id temp_light = cmzn_lightmodule_get_default_light(lm);
	EXPECT_EQ(light, temp_light);
	cmzn_light_destroy(&temp_light);

	temp_light = cmzn_lightmodule_find_light_by_name(lm, "default");
	EXPECT_EQ(light, temp_light);
	cmzn_light_destroy(&temp_light);

	cmzn_light_destroy(&light);

	light = cmzn_lightmodule_get_default_light(lm);
	EXPECT_NE(static_cast<cmzn_light *>(0), light);
	cmzn_light_destroy(&light);

	cmzn_lightmodule_destroy(&lm);
}

TEST(cmzn_lightmodule_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	Lightmodule lm = zinc.context.getLightmodule();
	EXPECT_TRUE(lm.isValid());

	int result = lm.beginChange();
	EXPECT_EQ(OK, result);

	Light light = lm.getDefaultLight();
	EXPECT_TRUE(light.isValid());

	double float_values[3] = {0.0, 0.0, 0.0};
	result = light.getColour(&float_values[0]);
	EXPECT_EQ(OK, result);
	EXPECT_DOUBLE_EQ(0.9, float_values[0]);
	EXPECT_DOUBLE_EQ(0.9, float_values[1]);
	EXPECT_DOUBLE_EQ(0.9, float_values[2]);

	result = light.getDirection(&float_values[0]);
	EXPECT_EQ(OK, result);
	EXPECT_DOUBLE_EQ(0.0, float_values[0]);
	EXPECT_DOUBLE_EQ(-0.5, float_values[1]);
	EXPECT_DOUBLE_EQ(-1.0, float_values[2]);

	result = light.getPosition(&float_values[0]);
	EXPECT_EQ(OK, result);
	EXPECT_DOUBLE_EQ(0.0, float_values[0]);
	EXPECT_DOUBLE_EQ(0.0, float_values[1]);
	EXPECT_DOUBLE_EQ(0.0, float_values[2]);

	enum Light::Type lightType = light.getType();
	EXPECT_EQ(Light::TYPE_DIRECTIONAL, lightType);
	EXPECT_DOUBLE_EQ(light.getConstantAttenuation(), 1.0);
	EXPECT_DOUBLE_EQ(light.getLinearAttenuation(), 0.0);
	EXPECT_DOUBLE_EQ(light.getQuadraticAttenuation(), 0.0);
	EXPECT_DOUBLE_EQ(light.getSpotCutoff(), 90.0);
	EXPECT_DOUBLE_EQ(light.getSpotExponent(), 0.0);
	EXPECT_EQ(light.isEnabled(), 1);

	light = lm.getDefaultAmbientLight();
	EXPECT_TRUE(light.isValid());
	result = light.getColour( &float_values[0]);
	EXPECT_EQ(OK, result);
	EXPECT_DOUBLE_EQ(0.1, float_values[0]);
	EXPECT_DOUBLE_EQ(0.1, float_values[1]);
	EXPECT_DOUBLE_EQ(0.1, float_values[2]);
	lightType = light.getType();
	EXPECT_EQ(Light::TYPE_AMBIENT, lightType);
	enum Light::RenderViewerMode viewerMode = light.getRenderViewerMode();
	EXPECT_EQ(Light::RENDER_VIEWER_MODE_INFINITE, viewerMode);
	enum Light::RenderSide renderSide = light.getRenderSide();
	EXPECT_EQ(Light::RENDER_SIDE_DOUBLE, renderSide);

	// following should destroy default light as not managed and not used
	// otherwise it isn't possible to create a light named "default" below
	light = Light();
	result = lm.setDefaultLight(light);
	EXPECT_EQ(OK, result);

	light = lm.createLight();
	EXPECT_TRUE(light.isValid());

	result = light.setName("default");
	EXPECT_EQ(OK, result);

	result = lm.endChange();
	EXPECT_EQ(OK, result);

	result = lm.setDefaultLight( light);
	EXPECT_EQ(OK, result);

	result = light.setManaged(1);
	EXPECT_EQ(OK, result);

	Light tempLight = lm.getDefaultLight();
	EXPECT_EQ(light.getId(), tempLight.getId());

	tempLight = lm.findLightByName("default");
	EXPECT_EQ(light.getId(), tempLight.getId());

	light = lm.getDefaultLight();
	EXPECT_TRUE(light.isValid());
}

TEST(cmzn_light_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_lightmodule_id lm = cmzn_context_get_lightmodule(zinc.context);
	EXPECT_NE(static_cast<cmzn_lightmodule *>(0), lm);

	int result = cmzn_lightmodule_begin_change(lm);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_light_id light = cmzn_lightmodule_create_light(lm);
	EXPECT_NE(static_cast<cmzn_light *>(0), light);

	result = cmzn_light_set_name(light, "default");
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_lightmodule_end_change(lm);
	EXPECT_EQ(CMZN_OK, result);

	double inValues[3] = {0.2, 0.3, 0.4};
	double outValues[3] = {0.0, 0.0, 0.0};

	result = cmzn_light_set_colour(light, &(inValues[0]));
	EXPECT_EQ(CMZN_OK, result);
	result = cmzn_light_get_colour(light, &(outValues[0]));
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_DOUBLE_EQ(0.2, outValues[0]);
	EXPECT_DOUBLE_EQ(0.3, outValues[1]);
	EXPECT_DOUBLE_EQ(0.4, outValues[2]);

	result = cmzn_light_set_direction(light, &(inValues[0]));
	EXPECT_EQ(CMZN_OK, result);
	result = cmzn_light_get_direction(light, &(outValues[0]));
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_DOUBLE_EQ(0.2, outValues[0]);
	EXPECT_DOUBLE_EQ(0.3, outValues[1]);
	EXPECT_DOUBLE_EQ(0.4, outValues[2]);

	result = cmzn_light_set_position(light, &(inValues[0]));
	EXPECT_EQ(CMZN_OK, result);
	result = cmzn_light_get_position(light, &(outValues[0]));
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_DOUBLE_EQ(0.2, outValues[0]);
	EXPECT_DOUBLE_EQ(0.3, outValues[1]);
	EXPECT_DOUBLE_EQ(0.4, outValues[2]);

	result = cmzn_light_set_managed(light, 1);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_light_is_managed(light);
	EXPECT_EQ(1, result);

	EXPECT_EQ(CMZN_OK, cmzn_light_set_constant_attenuation(light,0.5));
	EXPECT_EQ(CMZN_OK, cmzn_light_set_linear_attenuation(light, 1.0));
	EXPECT_EQ(CMZN_OK, cmzn_light_set_quadratic_attenuation(light, 1.0));
	EXPECT_EQ(CMZN_OK, cmzn_light_set_spot_cutoff(light, 70.0));
	EXPECT_EQ(CMZN_OK, cmzn_light_set_spot_exponent(light, 1.0));
	EXPECT_EQ(CMZN_OK, cmzn_light_set_enabled(light, 0));

	EXPECT_DOUBLE_EQ(cmzn_light_get_constant_attenuation(light), 0.5);
	EXPECT_DOUBLE_EQ(cmzn_light_get_linear_attenuation(light), 1.0);
	EXPECT_DOUBLE_EQ(cmzn_light_get_quadratic_attenuation(light), 1.0);
	EXPECT_DOUBLE_EQ(cmzn_light_get_spot_cutoff(light), 70.0);
	EXPECT_DOUBLE_EQ(cmzn_light_get_spot_exponent(light), 1.0);
	EXPECT_EQ(cmzn_light_is_enabled(light), 0);

	EXPECT_EQ(CMZN_OK, cmzn_light_set_type(light, CMZN_LIGHT_TYPE_AMBIENT));
	EXPECT_EQ(cmzn_light_get_type(light), CMZN_LIGHT_TYPE_AMBIENT);

	EXPECT_EQ(CMZN_OK, cmzn_light_set_render_viewer_mode(light, CMZN_LIGHT_RENDER_VIEWER_MODE_LOCAL));
	EXPECT_EQ(cmzn_light_get_render_viewer_mode(light), CMZN_LIGHT_RENDER_VIEWER_MODE_LOCAL);

	EXPECT_EQ(CMZN_OK, cmzn_light_set_render_side(light, CMZN_LIGHT_RENDER_SIDE_SINGLE));
	EXPECT_EQ( cmzn_light_get_render_side(light), CMZN_LIGHT_RENDER_SIDE_SINGLE);

	cmzn_light_destroy(&light);

	cmzn_lightmodule_destroy(&lm);
}

TEST(cmzn_light_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	Lightmodule lm = zinc.context.getLightmodule();
	EXPECT_TRUE(lm.isValid());

	int result = lm.beginChange();
	EXPECT_EQ(OK, result);

	Light light = lm.createLight();
	EXPECT_TRUE(light.isValid());

	result = light.setName("default");
	EXPECT_EQ(OK, result);

	result = lm.endChange();
	EXPECT_EQ(OK, result);


	double inValues[3] = {0.2, 0.3, 0.4};
	double outValues[3] = {0.0, 0.0, 0.0};

	result = light.setColour(&(inValues[0]));
	EXPECT_EQ(OK, result);
	result = light.getColour(&(outValues[0]));
	EXPECT_EQ(OK, result);
	EXPECT_DOUBLE_EQ(0.2, outValues[0]);
	EXPECT_DOUBLE_EQ(0.3, outValues[1]);
	EXPECT_DOUBLE_EQ(0.4, outValues[2]);

	result = light.setDirection(&(inValues[0]));
	EXPECT_EQ(OK, result);
	result = light.getDirection(&(outValues[0]));
	EXPECT_EQ(OK, result);
	EXPECT_DOUBLE_EQ(0.2, outValues[0]);
	EXPECT_DOUBLE_EQ(0.3, outValues[1]);
	EXPECT_DOUBLE_EQ(0.4, outValues[2]);

	result = light.setPosition(&(inValues[0]));
	EXPECT_EQ(OK, result);
	result = light.getPosition(&(outValues[0]));
	EXPECT_EQ(OK, result);
	EXPECT_DOUBLE_EQ(0.2, outValues[0]);
	EXPECT_DOUBLE_EQ(0.3, outValues[1]);
	EXPECT_DOUBLE_EQ(0.4, outValues[2]);

	result = light.setManaged(0);
	EXPECT_EQ(OK, result);

	result = light.isManaged();
	EXPECT_EQ(0, result);

	EXPECT_EQ(OK, light.setConstantAttenuation(0.5));
	EXPECT_EQ(OK, light.setLinearAttenuation(1.0));
	EXPECT_EQ(OK, light.setQuadraticAttenuation(1.0));
	EXPECT_EQ(OK, light.setSpotCutoff(70.0));
	EXPECT_EQ(OK, light.setSpotExponent(1.0));
	EXPECT_EQ(OK, light.setEnabled(0));

	EXPECT_DOUBLE_EQ(light.getLinearAttenuation(), 1.0);
	EXPECT_DOUBLE_EQ(light.getQuadraticAttenuation(), 1.0);
//	EXPECT_DOUBLE_EQ(light.getConstantAttenuation(), 0.5);
	EXPECT_DOUBLE_EQ(light.getSpotCutoff(), 70.0);
	EXPECT_DOUBLE_EQ(light.getSpotExponent(), 1.0);
	EXPECT_EQ(light.isEnabled(), 0);

	EXPECT_EQ(OK, light.setType(Light::TYPE_AMBIENT));
	EXPECT_EQ(light.getType(), Light::TYPE_AMBIENT);

	EXPECT_EQ(OK, light.setRenderViewerMode(Light::RENDER_VIEWER_MODE_LOCAL));
	EXPECT_EQ(light.getRenderViewerMode(), Light::RENDER_VIEWER_MODE_LOCAL);

	EXPECT_EQ(OK, light.setRenderSide(Light::RENDER_SIDE_SINGLE));
	EXPECT_EQ( light.getRenderSide(), Light::RENDER_SIDE_SINGLE);
}

TEST(ZincLightiterator, iteration)
{
	ZincTestSetupCpp zinc;

	Lightmodule lm = zinc.context.getLightmodule();
	EXPECT_TRUE(lm.isValid());
	Light defaultLight = lm.getDefaultLight();
	EXPECT_TRUE(defaultLight.isValid());
	Light defaultAmbientLight = lm.getDefaultAmbientLight();
	EXPECT_TRUE(defaultAmbientLight.isValid());

	Light zzz = lm.createLight();
	EXPECT_TRUE(zzz.isValid());
	EXPECT_EQ(OK, zzz.setName("zzz"));
	char *name = zzz.getName();
	EXPECT_STREQ("zzz", name);
	cmzn_deallocate(name);
	name = 0;

	Light aaa = lm.createLight();
	EXPECT_TRUE(aaa.isValid());
	EXPECT_EQ(OK, aaa.setName("aaa"));

	Light aab = lm.createLight();
	EXPECT_TRUE(aab.isValid());
	EXPECT_EQ(OK, aab.setName("aab"));

	Lightiterator iter = lm.createLightiterator();
	EXPECT_TRUE(iter.isValid());
	Light g;
	EXPECT_EQ(aaa, g = iter.next());
	EXPECT_EQ(aab, g = iter.next());
	EXPECT_EQ(defaultLight, g = iter.next());
	EXPECT_EQ(defaultAmbientLight, g = iter.next());
	EXPECT_EQ(zzz, g = iter.next());
	g = iter.next();
	EXPECT_FALSE(g.isValid());
}
