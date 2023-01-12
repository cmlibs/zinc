/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/core.h>
#include <opencmiss/zinc/field.h>
#include <opencmiss/zinc/fieldconstant.h>
#include <opencmiss/zinc/fieldimage.h>

#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldimage.hpp>
#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

#include "test_resources.h"

TEST(cmzn_field_image, create_evaluate)
{
	ZincTestSetup zinc;

	cmzn_field_id f1 = cmzn_fieldmodule_create_field_image(zinc.fm);
	EXPECT_NE(static_cast<cmzn_field_id>(0), f1);

	cmzn_field_image_id im = cmzn_field_cast_image(f1);
	EXPECT_NE(static_cast<cmzn_field_image_id>(0), im);

	int result;
    EXPECT_EQ(CMZN_OK, result = cmzn_field_image_read_file(im, resourcePath("blockcolours.png").c_str()));
	int numberOfComponents = cmzn_field_get_number_of_components(f1);
	EXPECT_EQ(3, numberOfComponents);

	cmzn_field_id xi = cmzn_field_image_get_domain_field(im);
	EXPECT_NE(static_cast<cmzn_field_id>(0), f1);
	char *name = cmzn_field_get_name(xi);
	EXPECT_STREQ("xi", name);
	cmzn_deallocate(name);

	cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(zinc.fm);
	EXPECT_NE(static_cast<cmzn_fieldcache_id>(0), cache);
	double outRGB[3];
	const double xi1[3] = { 0.2, 0.8, 0.0 };
	const double expectedRGB1[3] = { 1.0, 128.0/255.0, 0.0 };
	EXPECT_EQ(CMZN_OK, result = cmzn_fieldcache_set_field_real(cache, xi, 2, xi1));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_evaluate_real(f1, cache, 3, outRGB));
	EXPECT_EQ(expectedRGB1[0], outRGB[0]);
	EXPECT_EQ(expectedRGB1[1], outRGB[1]);
	EXPECT_EQ(expectedRGB1[2], outRGB[2]);
	const double xi2[3] = { 0.75, 0.2, 0.0 };
	const double expectedRGB2[3] = { 192.0/255.0, 192.0/255.0, 192.0/255.0 };
	EXPECT_EQ(CMZN_OK, result = cmzn_fieldcache_set_field_real(cache, xi, 2, xi2));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_evaluate_real(f1, cache, 3, outRGB));
	EXPECT_EQ(expectedRGB2[0], outRGB[0]);
	EXPECT_EQ(expectedRGB2[1], outRGB[1]);
	EXPECT_EQ(expectedRGB2[2], outRGB[2]);
	cmzn_fieldcache_destroy(&cache);

	double width = cmzn_field_image_get_texture_coordinate_width(im);
	ASSERT_DOUBLE_EQ(1.0, width);
	double height = cmzn_field_image_get_texture_coordinate_height(im);
	ASSERT_DOUBLE_EQ(1.0, height);
	double depth = cmzn_field_image_get_texture_coordinate_depth(im);
	ASSERT_DOUBLE_EQ(1.0, depth);
	double double_sizes[3] = {};
	cmzn_field_image_get_texture_coordinate_sizes(im, 3, &double_sizes[0]);
	ASSERT_DOUBLE_EQ(1.0, double_sizes[0]);
	ASSERT_DOUBLE_EQ(1.0, double_sizes[1]);
	ASSERT_DOUBLE_EQ(1.0, double_sizes[2]);

	// test setting domain field and evaluation of image_from_source

	const double tempValues3[3] = { 0.0, 1.0, 2.0 };
	cmzn_field_id altDomainField = cmzn_fieldmodule_create_field_constant(zinc.fm, 3, tempValues3);
	cmzn_field_set_name(altDomainField, "altDomainField");
	EXPECT_EQ(CMZN_OK, result = cmzn_field_image_set_domain_field(im, altDomainField));
	cmzn_field_id f2 = cmzn_fieldmodule_create_field_image_from_source(zinc.fm, f1);
	EXPECT_NE(static_cast<cmzn_field_id>(0), f2);

	// check resolution is same as source field
	cmzn_field_image_id im2 = cmzn_field_cast_image(f2);
	int width_texels = cmzn_field_image_get_width_in_pixels(im2);
	EXPECT_EQ(32, width_texels);
	int height_texels = cmzn_field_image_get_height_in_pixels(im2);
	EXPECT_EQ(32, height_texels);
	int depth_texels = cmzn_field_image_get_depth_in_pixels(im2);
	EXPECT_EQ(1, depth_texels);
	int int_sizes[3] = {};
	cmzn_field_image_get_size_in_pixels(im2, 3, &int_sizes[0]);
	EXPECT_EQ(32, int_sizes[0]);
	EXPECT_EQ(32, int_sizes[1]);
	EXPECT_EQ(1, int_sizes[2]);
	cmzn_field_image_destroy(&im2);

	cache = cmzn_fieldmodule_create_fieldcache(zinc.fm);
	EXPECT_NE(static_cast<cmzn_fieldcache_id>(0), cache);
	EXPECT_EQ(CMZN_OK, result = cmzn_fieldcache_set_field_real(cache, altDomainField, 3, xi1));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_evaluate_real(f2, cache, 3, outRGB));
	EXPECT_EQ(expectedRGB1[0], outRGB[0]);
	EXPECT_EQ(expectedRGB1[1], outRGB[1]);
	EXPECT_EQ(expectedRGB1[2], outRGB[2]);
	cmzn_fieldcache_destroy(&cache);

	cmzn_field_destroy(&f2);
	cmzn_field_destroy(&altDomainField);
	cmzn_field_destroy(&xi);
	cmzn_field_image_destroy(&im);
	cmzn_field_destroy(&f1);
}

TEST(ZincFieldImage, create_evaluate)
{
	ZincTestSetupCpp zinc;

	FieldImage im = zinc.fm.createFieldImage();
	EXPECT_TRUE(im.isValid());

	// test casting
	FieldImage castImage = im.castImage();
	EXPECT_TRUE(castImage.isValid());

	int result;
    EXPECT_EQ(OK, result = im.readFile(resourcePath("blockcolours.png").c_str()));
	int numberOfComponents = im.getNumberOfComponents();
	EXPECT_EQ(3, numberOfComponents);

	Field xi = im.getDomainField();
	EXPECT_TRUE(xi.isValid());
	char *name = xi.getName();
	EXPECT_STREQ("xi", name);
	cmzn_deallocate(name);

	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());
	double outRGB[3];
	const double xi1[3] = { 0.2, 0.8, 0.0 };
	const double expectedRGB1[3] = { 1.0, 128.0/255.0, 0.0 };
	EXPECT_EQ(OK, result = cache.setFieldReal(xi, 2, xi1));
	EXPECT_EQ(OK, result = im.evaluateReal(cache, 3, outRGB));
	EXPECT_EQ(expectedRGB1[0], outRGB[0]);
	EXPECT_EQ(expectedRGB1[1], outRGB[1]);
	EXPECT_EQ(expectedRGB1[2], outRGB[2]);
	const double xi2[3] = { 0.75, 0.2, 0.0 };
	const double expectedRGB2[3] = { 192.0/255.0, 192.0/255.0, 192.0/255.0 };
	EXPECT_EQ(OK, result = cache.setFieldReal(xi, 2, xi2));
	EXPECT_EQ(OK, result = im.evaluateReal(cache, 3, outRGB));
	EXPECT_EQ(expectedRGB2[0], outRGB[0]);
	EXPECT_EQ(expectedRGB2[1], outRGB[1]);
	EXPECT_EQ(expectedRGB2[2], outRGB[2]);

	double width = im.getTextureCoordinateWidth();
	ASSERT_DOUBLE_EQ(1.0, width);
	double height = im.getTextureCoordinateHeight();
	ASSERT_DOUBLE_EQ(1.0, height);
	double depth = im.getTextureCoordinateDepth();
	ASSERT_DOUBLE_EQ(1.0, depth);
	double double_sizes[3] = {};
	im.getTextureCoordinateSizes(3, &double_sizes[0]);
	ASSERT_DOUBLE_EQ(1.0, double_sizes[0]);
	ASSERT_DOUBLE_EQ(1.0, double_sizes[1]);
	ASSERT_DOUBLE_EQ(1.0, double_sizes[2]);

	// test setting domain field and evaluation of imageFromSource

	const double tempValues3[3] = { 0.0, 1.0, 2.0 };
	FieldConstant altDomainField = zinc.fm.createFieldConstant(3, tempValues3);
	altDomainField.setName("altDomainField");
	EXPECT_EQ(OK, result = im.setDomainField(altDomainField));
	FieldImage im2 = zinc.fm.createFieldImageFromSource(im);
	EXPECT_TRUE(im2.isValid());

	// check resolution is same as source field
	int width_texels = im2.getWidthInPixels();
	EXPECT_EQ(32, width_texels);
	int height_texels = im2.getHeightInPixels();
	EXPECT_EQ(32, height_texels);
	int depth_texels = im2.getDepthInPixels();
	EXPECT_EQ(1, depth_texels);
	int int_sizes[3] = {};
	im2.getSizeInPixels(3, &int_sizes[0]);
	EXPECT_EQ(32, int_sizes[0]);
	EXPECT_EQ(32, int_sizes[1]);
	EXPECT_EQ(1, int_sizes[2]);

	cache = zinc.fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());
	EXPECT_EQ(OK, result = cache.setFieldReal(altDomainField, 3, xi1));
	EXPECT_EQ(OK, result = im2.evaluateReal(cache, 3, outRGB));
	EXPECT_EQ(expectedRGB1[0], outRGB[0]);
	EXPECT_EQ(expectedRGB1[1], outRGB[1]);
	EXPECT_EQ(expectedRGB1[2], outRGB[2]);
}

TEST(cmzn_field_image, enumerations)
{
	ZincTestSetup zinc;

	cmzn_field_id f1 = cmzn_fieldmodule_create_field_image(zinc.fm);
	EXPECT_NE(static_cast<cmzn_field_id>(0), f1);

	cmzn_field_image_id im = cmzn_field_cast_image(f1);
	EXPECT_NE(static_cast<cmzn_field_image_id>(0), im);

	int result;
    EXPECT_EQ(CMZN_OK, result = cmzn_field_image_read_file(im, resourcePath("blockcolours.png").c_str()));

	EXPECT_EQ(CMZN_FIELD_IMAGE_COMBINE_MODE_DECAL, result = cmzn_field_image_get_combine_mode(im));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_image_set_combine_mode(im, CMZN_FIELD_IMAGE_COMBINE_MODE_BLEND));
	EXPECT_EQ(CMZN_FIELD_IMAGE_COMBINE_MODE_BLEND, result = cmzn_field_image_get_combine_mode(im));

	EXPECT_EQ(CMZN_FIELD_IMAGE_FILTER_MODE_NEAREST, result = cmzn_field_image_get_filter_mode(im));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_image_set_filter_mode(im,
		CMZN_FIELD_IMAGE_FILTER_MODE_NEAREST_MIPMAP_NEAREST));
	EXPECT_EQ(CMZN_FIELD_IMAGE_FILTER_MODE_NEAREST_MIPMAP_NEAREST, result = cmzn_field_image_get_filter_mode(im));

	EXPECT_EQ(CMZN_FIELD_IMAGE_HARDWARE_COMPRESSION_MODE_UNCOMPRESSED,
		result = cmzn_field_image_get_hardware_compression_mode(im));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_image_set_hardware_compression_mode(im,
		CMZN_FIELD_IMAGE_HARDWARE_COMPRESSION_MODE_AUTOMATIC));
	EXPECT_EQ(CMZN_FIELD_IMAGE_HARDWARE_COMPRESSION_MODE_AUTOMATIC,
		result = cmzn_field_image_get_hardware_compression_mode(im));

	EXPECT_EQ(CMZN_FIELD_IMAGE_WRAP_MODE_REPEAT, result = cmzn_field_image_get_wrap_mode(im));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_image_set_wrap_mode(im, CMZN_FIELD_IMAGE_WRAP_MODE_EDGE_CLAMP));
	EXPECT_EQ(CMZN_FIELD_IMAGE_WRAP_MODE_EDGE_CLAMP, result = cmzn_field_image_get_wrap_mode(im));

	cmzn_field_image_destroy(&im);
	cmzn_field_destroy(&f1);
}

TEST(ZincFieldImage, enumerations)
{
	ZincTestSetupCpp zinc;

	FieldImage im = zinc.fm.createFieldImage();
	EXPECT_TRUE(im.isValid());

	int result;
    EXPECT_EQ(OK, result = im.readFile(resourcePath("blockcolours.png").c_str()));

	EXPECT_EQ(FieldImage::COMBINE_MODE_DECAL, im.getCombineMode());
	EXPECT_EQ(OK, result = im.setCombineMode(FieldImage::COMBINE_MODE_BLEND));
	EXPECT_EQ(FieldImage::COMBINE_MODE_BLEND, im.getCombineMode());

	EXPECT_EQ(FieldImage::FILTER_MODE_NEAREST, im.getFilterMode());
	EXPECT_EQ(OK, result = im.setFilterMode(FieldImage::FILTER_MODE_NEAREST_MIPMAP_NEAREST));
	EXPECT_EQ(FieldImage::FILTER_MODE_NEAREST_MIPMAP_NEAREST, im.getFilterMode());

	EXPECT_EQ(FieldImage::HARDWARE_COMPRESSION_MODE_UNCOMPRESSED, im.getHardwareCompressionMode());
	EXPECT_EQ(OK, result = im.setHardwareCompressionMode(FieldImage::HARDWARE_COMPRESSION_MODE_AUTOMATIC));
	EXPECT_EQ(FieldImage::HARDWARE_COMPRESSION_MODE_AUTOMATIC, im.getHardwareCompressionMode());

	EXPECT_EQ(FieldImage::WRAP_MODE_REPEAT, im.getWrapMode());
	EXPECT_EQ(OK, result = im.setWrapMode(FieldImage::WRAP_MODE_EDGE_CLAMP));
	EXPECT_EQ(FieldImage::WRAP_MODE_EDGE_CLAMP, im.getWrapMode());
}
