
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/context.h>
#include <zinc/core.h>
#include <zinc/field.h>
#include <zinc/fieldcache.h>
#include <zinc/fieldcomposite.h>
#include <zinc/fieldconstant.h>
#include <zinc/fieldimage.h>
#include <zinc/fieldimageprocessing.h>
#include <zinc/fieldmodule.h>
#include <zinc/region.h>
#include <zinc/status.h>Ä
#include <zinc/stream.h>

#include "zinctestsetup.hpp"
#include "test_resources.h"

TEST(cmzn_fieldmodule_create_field_imagefilter_connected_threshold, invalid_args)
{
	const double values[] = { 0.3, 0.1, 0.7 };
	ZincTestSetup zinc;

	cmzn_field_id f1 = cmzn_fieldmodule_create_field_imagefilter_curvature_anisotropic_diffusion(zinc.fm, 0, 0.2, 1.0, 1);
	EXPECT_EQ((cmzn_field_id)0, f1);

	cmzn_field_id f2 = cmzn_fieldmodule_create_field_imagefilter_connected_threshold(zinc.fm, 0, 0.2, 1.0, 1.0, 1, 3, values);
	EXPECT_EQ((cmzn_field_id)0, f2);

}

TEST(cmzn_fieldmodule_create_field_imagefilter_curvature_anisotropic_diffusion, valid_args)
{
	ZincTestSetup zinc;

	// Create empty image field
	cmzn_field_id f1 = cmzn_fieldmodule_create_field_image(zinc.fm);
	EXPECT_NE(static_cast<cmzn_field_id>(0), f1);

	cmzn_field_image_id im = cmzn_field_cast_image(f1);
	cmzn_streaminformation_id si = cmzn_field_image_create_streaminformation(im);
	EXPECT_NE(static_cast<cmzn_streaminformation_id>(0), si);

	cmzn_streamresource_id sr = cmzn_streaminformation_create_streamresource_file(si, TestResources::getLocation(TestResources::TESTIMAGE_GRAY_JPG_RESOURCE));

	EXPECT_EQ(CMZN_OK, cmzn_field_image_read(im, si));

	cmzn_field_id f2 = cmzn_fieldmodule_create_field_imagefilter_curvature_anisotropic_diffusion(zinc.fm, cmzn_field_image_base_cast(im), 0.1, 1.0, 1);
	EXPECT_NE((cmzn_field_id)0, f2);

//	cmzn_field_id f2 = cmzn_fieldmodule_create_field_connected_threshold_image_filter(zinc.fm, f1, 0.2, 1.0, 1.0, 1, 3, values);
//	EXPECT_NE((cmzn_field_id)0, f2);

	cmzn_field_id xi = cmzn_field_image_get_domain_field(im);
	EXPECT_NE((cmzn_field_id)0, xi);

	cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(zinc.fm);

	double location[] = { 0.7, 0.2};
	double value = 0.0;

	EXPECT_EQ(CMZN_OK, cmzn_fieldcache_set_field_real(cache, xi, 2, location));
//	EXPECT_EQ(CMZN_OK, cmzn_field_evaluate_real(cmzn_field_image_base_cast(im), cache, 1, &value));

	EXPECT_EQ(CMZN_OK, cmzn_field_evaluate_real(f2, cache, 1, &value));
	EXPECT_NEAR(0.211765, value, 1e-6);

	cmzn_fieldcache_destroy(&cache);
	cmzn_field_destroy(&xi);
	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
	cmzn_streamresource_destroy(&sr);
	cmzn_streaminformation_destroy(&si);
	cmzn_field_image_destroy(&im);

}

TEST(cmzn_fieldmodule_create_field_imagefilter_connected_threshold, valid_args)
{
	ZincTestSetup zinc;
	double location[] = { 0.7, 0.2};

	// Create empty image field
	cmzn_field_id f1 = cmzn_fieldmodule_create_field_image(zinc.fm);
	EXPECT_NE(static_cast<cmzn_field_id>(0), f1);

	cmzn_field_image_id im = cmzn_field_cast_image(f1);
	cmzn_streaminformation_id si = cmzn_field_image_create_streaminformation(im);
	EXPECT_NE(static_cast<cmzn_streaminformation_id>(0), si);

	cmzn_streamresource_id sr = cmzn_streaminformation_create_streamresource_file(si, TestResources::getLocation(TestResources::TESTIMAGE_GRAY_JPG_RESOURCE));

	EXPECT_EQ(CMZN_OK, cmzn_field_image_read(im, si));

	cmzn_field_id f2 = cmzn_fieldmodule_create_field_imagefilter_connected_threshold(zinc.fm, cmzn_field_image_base_cast(im), 0.2, 0.22, 0.33, 1, 2, location);
	EXPECT_NE((cmzn_field_id)0, f2);

	cmzn_field_id xi = cmzn_field_image_get_domain_field(im);
	EXPECT_NE((cmzn_field_id)0, xi);

	cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(zinc.fm);

	double value = 0.0;
	EXPECT_EQ(CMZN_OK, cmzn_fieldcache_set_field_real(cache, xi, 2, location));
//	EXPECT_EQ(CMZN_OK, cmzn_field_evaluate_real(cmzn_field_image_base_cast(im), cache, 1, &value));

	EXPECT_EQ(CMZN_OK, cmzn_field_evaluate_real(f2, cache, 1, &value));
	EXPECT_NEAR(0.33, value, 1e-6);

	cmzn_fieldcache_destroy(&cache);
	cmzn_field_destroy(&xi);
	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
	cmzn_streamresource_destroy(&sr);
	cmzn_streaminformation_destroy(&si);
	cmzn_field_image_destroy(&im);
}
