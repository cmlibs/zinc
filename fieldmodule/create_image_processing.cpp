
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/region.h>
#include <zinc/fieldmodule.h>
#include <zinc/field.h>
#include <zinc/fieldcomposite.h>
#include <zinc/fieldconstant.h>
#include <zinc/fieldimageprocessing.h>
#include <zinc/fieldimage.h>
#include <zinc/stream.h>

#include "zinctestsetup.hpp"
#include "test_resources.h"

TEST(Cmiss_field_module_create_connected_threshold_image_filter, invalid_args)
{
	const double values[] = { 0.3, 0.1, 0.7 };
	ZincTestSetup zinc;

	Cmiss_field_id f1 = Cmiss_field_module_create_curvature_anisotropic_diffusion_image_filter(zinc.fm, 0, 0.2, 1.0, 1);
	EXPECT_EQ((Cmiss_field_id)0, f1);

	Cmiss_field_id f2 = Cmiss_field_module_create_connected_threshold_image_filter(zinc.fm, 0, 0.2, 1.0, 1.0, 1, 3, values);
	EXPECT_EQ((Cmiss_field_id)0, f2);

}

TEST(Cmiss_field_module_create_curvature_anisotropic_diffusion_image_filter, valid_args)
{
	ZincTestSetup zinc;

	// Create empty image field
	Cmiss_field_id f1 = Cmiss_field_module_create_image(zinc.fm, 0, 0);
	EXPECT_NE(static_cast<Cmiss_field_id>(0), f1);

	Cmiss_field_image_id im = Cmiss_field_cast_image(f1);
	Cmiss_stream_information_id si = Cmiss_field_image_create_stream_information(im);
	EXPECT_NE(static_cast<Cmiss_stream_information_id>(0), si);

	Cmiss_stream_resource_id sr = Cmiss_stream_information_create_resource_file(si, TestResources::getLocation(TestResources::TESTIMAGE_GRAY_JPG_RESOURCE));

	EXPECT_EQ(CMISS_OK, Cmiss_field_image_read(im, si));

	Cmiss_field_id f2 = Cmiss_field_module_create_curvature_anisotropic_diffusion_image_filter(zinc.fm, Cmiss_field_image_base_cast(im), 0.1, 1.0, 1);
	EXPECT_NE((Cmiss_field_id)0, f2);

//	Cmiss_field_id f2 = Cmiss_field_module_create_connected_threshold_image_filter(zinc.fm, f1, 0.2, 1.0, 1.0, 1, 3, values);
//	EXPECT_NE((Cmiss_field_id)0, f2);

	Cmiss_field_id xi = Cmiss_field_module_find_field_by_name(zinc.fm, "Cmiss_temp_image_domain");
	EXPECT_NE((Cmiss_field_id)0, xi);

	Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(zinc.fm);

	double location[] = { 0.7, 0.2};
	double value = 0.0;

	EXPECT_EQ(CMISS_OK, Cmiss_field_cache_set_field_real(cache, xi, 2, location));
//	EXPECT_EQ(CMISS_OK, Cmiss_field_evaluate_real(Cmiss_field_image_base_cast(im), cache, 1, &value));

	EXPECT_EQ(CMISS_OK, Cmiss_field_evaluate_real(f2, cache, 1, &value));
	EXPECT_NEAR(0.211765, value, 1e-6);

	Cmiss_field_cache_destroy(&cache);
	Cmiss_field_destroy(&xi);
	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
	Cmiss_stream_resource_destroy(&sr);
	Cmiss_stream_information_destroy(&si);
	Cmiss_field_image_destroy(&im);

}

TEST(Cmiss_field_module_create_connected_threshold_image_filter, valid_args)
{
	ZincTestSetup zinc;
	double location[] = { 0.7, 0.2};

	// Create empty image field
	Cmiss_field_id f1 = Cmiss_field_module_create_image(zinc.fm, 0, 0);
	EXPECT_NE(static_cast<Cmiss_field_id>(0), f1);

	Cmiss_field_image_id im = Cmiss_field_cast_image(f1);
	Cmiss_stream_information_id si = Cmiss_field_image_create_stream_information(im);
	EXPECT_NE(static_cast<Cmiss_stream_information_id>(0), si);

	Cmiss_stream_resource_id sr = Cmiss_stream_information_create_resource_file(si, TestResources::getLocation(TestResources::TESTIMAGE_GRAY_JPG_RESOURCE));

	EXPECT_EQ(CMISS_OK, Cmiss_field_image_read(im, si));

	Cmiss_field_id f2 = Cmiss_field_module_create_connected_threshold_image_filter(zinc.fm, Cmiss_field_image_base_cast(im), 0.2, 0.22, 0.33, 1, 2, location);
	EXPECT_NE((Cmiss_field_id)0, f2);

	Cmiss_field_id xi = Cmiss_field_module_find_field_by_name(zinc.fm, "Cmiss_temp_image_domain");
	EXPECT_NE((Cmiss_field_id)0, xi);

	Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(zinc.fm);

	double value = 0.0;
	EXPECT_EQ(CMISS_OK, Cmiss_field_cache_set_field_real(cache, xi, 2, location));
//	EXPECT_EQ(CMISS_OK, Cmiss_field_evaluate_real(Cmiss_field_image_base_cast(im), cache, 1, &value));

	EXPECT_EQ(CMISS_OK, Cmiss_field_evaluate_real(f2, cache, 1, &value));
	EXPECT_NEAR(0.33, value, 1e-6);

	Cmiss_field_cache_destroy(&cache);
	Cmiss_field_destroy(&xi);
	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
	Cmiss_stream_resource_destroy(&sr);
	Cmiss_stream_information_destroy(&si);
	Cmiss_field_image_destroy(&im);

}


