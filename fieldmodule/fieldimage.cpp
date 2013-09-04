#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/core.h>
#include <zinc/field.h>
#include <zinc/fieldconstant.h>
#include <zinc/fieldimage.h>

#include <zinc/field.hpp>
#include <zinc/fieldcache.hpp>
#include <zinc/fieldconstant.hpp>
#include <zinc/fieldimage.hpp>
#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

#include "test_resources.h"

TEST(cmzn_field_image, create_evaluate)
{
	ZincTestSetup zinc;

	cmzn_field_id f1 = cmzn_field_module_create_image(zinc.fm);
	EXPECT_NE(static_cast<cmzn_field_id>(0), f1);

	cmzn_field_image_id im = cmzn_field_cast_image(f1);
	EXPECT_NE(static_cast<cmzn_field_image_id>(0), im);

	int result;
	EXPECT_EQ(CMZN_OK, result = cmzn_field_image_read_file(im, TestResources::getLocation(TestResources::FIELDIMAGE_BLOCKCOLOURS_RESOURCE)));
	int numberOfComponents = cmzn_field_get_number_of_components(f1);
	EXPECT_EQ(3, numberOfComponents);

	cmzn_field_id xi = cmzn_field_image_get_domain_field(im);
	EXPECT_NE(static_cast<cmzn_field_id>(0), f1);
	char *name = cmzn_field_get_name(xi);
	EXPECT_STREQ("xi", name);
	cmzn_deallocate(name);

	cmzn_field_cache_id cache = cmzn_field_module_create_cache(zinc.fm);
	EXPECT_NE(static_cast<cmzn_field_cache_id>(0), cache);
	double outRGB[3];
	const double xi1[3] = { 0.2, 0.8, 0.0 };
	const double expectedRGB1[3] = { 1.0, 128.0/255.0, 0.0 };
	EXPECT_EQ(CMZN_OK, result = cmzn_field_cache_set_field_real(cache, xi, 2, xi1));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_evaluate_real(f1, cache, 3, outRGB));
	EXPECT_EQ(expectedRGB1[0], outRGB[0]);
	EXPECT_EQ(expectedRGB1[1], outRGB[1]);
	EXPECT_EQ(expectedRGB1[2], outRGB[2]);
	const double xi2[3] = { 0.75, 0.2, 0.0 };
	const double expectedRGB2[3] = { 192.0/255.0, 192.0/255.0, 192.0/255.0 };
	EXPECT_EQ(CMZN_OK, result = cmzn_field_cache_set_field_real(cache, xi, 2, xi2));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_evaluate_real(f1, cache, 3, outRGB));
	EXPECT_EQ(expectedRGB2[0], outRGB[0]);
	EXPECT_EQ(expectedRGB2[1], outRGB[1]);
	EXPECT_EQ(expectedRGB2[2], outRGB[2]);
	cmzn_field_cache_destroy(&cache);

	double width = cmzn_field_image_get_attribute_real(im, CMZN_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_WIDTH);
	ASSERT_DOUBLE_EQ(1.0, width);
	double height = cmzn_field_image_get_attribute_real(im, CMZN_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_HEIGHT);
	ASSERT_DOUBLE_EQ(1.0, height);
	double depth = cmzn_field_image_get_attribute_real(im, CMZN_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_DEPTH);
	ASSERT_DOUBLE_EQ(0.0, depth);

	// test setting domain field and evaluation of image_from_source

	const double tempValues3[3] = { 0.0, 1.0, 2.0 };
	cmzn_field_id altDomainField = cmzn_field_module_create_constant(zinc.fm, 3, tempValues3);
	cmzn_field_set_name(altDomainField, "altDomainField");
	EXPECT_EQ(CMZN_OK, result = cmzn_field_image_set_domain_field(im, altDomainField));
	cmzn_field_id f2 = cmzn_field_module_create_image_from_source(zinc.fm, f1);
	EXPECT_NE(static_cast<cmzn_field_id>(0), f2);

	// check resolution is same as source field
	cmzn_field_image_id im2 = cmzn_field_cast_image(f2);
	int width_texels = cmzn_field_image_get_attribute_integer(im, CMZN_FIELD_IMAGE_ATTRIBUTE_RAW_WIDTH_PIXELS);
	EXPECT_EQ(32, width_texels);
	int height_texels = cmzn_field_image_get_attribute_integer(im, CMZN_FIELD_IMAGE_ATTRIBUTE_RAW_HEIGHT_PIXELS);
	EXPECT_EQ(32, height_texels);
	int depth_texels = cmzn_field_image_get_attribute_integer(im, CMZN_FIELD_IMAGE_ATTRIBUTE_RAW_DEPTH_PIXELS);
	EXPECT_EQ(1, depth_texels);
	cmzn_field_image_destroy(&im2);

	cache = cmzn_field_module_create_cache(zinc.fm);
	EXPECT_NE(static_cast<cmzn_field_cache_id>(0), cache);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_cache_set_field_real(cache, altDomainField, 3, xi1));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_evaluate_real(f2, cache, 3, outRGB));
	EXPECT_EQ(expectedRGB1[0], outRGB[0]);
	EXPECT_EQ(expectedRGB1[1], outRGB[1]);
	EXPECT_EQ(expectedRGB1[2], outRGB[2]);
	cmzn_field_cache_destroy(&cache);

	cmzn_field_destroy(&f2);
	cmzn_field_destroy(&altDomainField);
	cmzn_field_destroy(&xi);
	cmzn_field_image_destroy(&im);
	cmzn_field_destroy(&f1);
}

TEST(ZincFieldImage, create_evaluate)
{
	ZincTestSetupCpp zinc;

	FieldImage im = zinc.fm.createImage();
	EXPECT_TRUE(im.isValid());

	int result;
	EXPECT_EQ(OK, result = im.readFile(TestResources::getLocation(TestResources::FIELDIMAGE_BLOCKCOLOURS_RESOURCE)));
	int numberOfComponents = im.getNumberOfComponents();
	EXPECT_EQ(3, numberOfComponents);

	Field xi = im.getDomainField();
	EXPECT_TRUE(xi.isValid());
	char *name = xi.getName();
	EXPECT_STREQ("xi", name);
	cmzn_deallocate(name);

	FieldCache cache = zinc.fm.createCache();
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

	double width = im.getAttributeReal(FieldImage::IMAGE_ATTRIBUTE_PHYSICAL_WIDTH);
	ASSERT_DOUBLE_EQ(1.0, width);
	double height = im.getAttributeReal(FieldImage::IMAGE_ATTRIBUTE_PHYSICAL_HEIGHT);
	ASSERT_DOUBLE_EQ(1.0, height);
	double depth = im.getAttributeReal(FieldImage::IMAGE_ATTRIBUTE_PHYSICAL_DEPTH);
	ASSERT_DOUBLE_EQ(0.0, depth);

	// test setting domain field and evaluation of imageFromSource

	const double tempValues3[3] = { 0.0, 1.0, 2.0 };
	FieldConstant altDomainField = zinc.fm.createConstant(3, tempValues3);
	altDomainField.setName("altDomainField");
	EXPECT_EQ(OK, result = im.setDomainField(altDomainField));
	FieldImage im2 = zinc.fm.createImageFromSource(im);
	EXPECT_TRUE(im2.isValid());

	// check resolution is same as source field
	int width_texels = im2.getAttributeInteger(FieldImage::IMAGE_ATTRIBUTE_RAW_WIDTH_PIXELS);
	EXPECT_EQ(32, width_texels);
	int height_texels = im2.getAttributeInteger(FieldImage::IMAGE_ATTRIBUTE_RAW_HEIGHT_PIXELS);
	EXPECT_EQ(32, height_texels);
	int depth_texels = im2.getAttributeInteger(FieldImage::IMAGE_ATTRIBUTE_RAW_DEPTH_PIXELS);
	EXPECT_EQ(1, depth_texels);

	cache = zinc.fm.createCache();
	EXPECT_TRUE(cache.isValid());
	EXPECT_EQ(OK, result = cache.setFieldReal(altDomainField, 3, xi1));
	EXPECT_EQ(OK, result = im2.evaluateReal(cache, 3, outRGB));
	EXPECT_EQ(expectedRGB1[0], outRGB[0]);
	EXPECT_EQ(expectedRGB1[1], outRGB[1]);
	EXPECT_EQ(expectedRGB1[2], outRGB[2]);
}
