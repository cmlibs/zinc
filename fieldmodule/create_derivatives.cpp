
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/region.h>
#include <zinc/fieldmodule.h>
#include <zinc/field.h>
#include <zinc/fieldderivatives.h>
#include <zinc/fieldconstant.h>
#include <zinc/node.h>
#include <zinc/element.h>
#include <zinc/fieldvectoroperators.h>

#include "test_resources.h"


TEST(Cmiss_field_module_create_derivative, invalid_args)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id fm = Cmiss_region_get_field_module(root_region);

	EXPECT_NE(static_cast<Cmiss_field_module *>(0), fm);

	Cmiss_field_id f0 = Cmiss_field_module_create_derivative(0, 0, 0);
	EXPECT_EQ(0, f0);

	Cmiss_field_id f1 = Cmiss_field_module_create_derivative(fm, 0, 2);
	EXPECT_EQ(0, f1);

	double values[] = {6.0, 1.0, 2.5};
	Cmiss_field_id f2 = Cmiss_field_module_create_constant(fm, 3, values);

	Cmiss_field_id f3 = Cmiss_field_module_create_derivative(fm, f2, -1);
	EXPECT_EQ(0, f3);

	Cmiss_field_destroy(&f0);
	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_destroy(&f3);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_field_module_create_derivative, valid_args)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id fm = Cmiss_region_get_field_module(root_region);

	Cmiss_region_read_file(root_region, TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE));

	Cmiss_field_id f1 = Cmiss_field_module_find_field_by_name(fm, "coordinates");
	EXPECT_NE(static_cast<Cmiss_field *>(0), f1);

	Cmiss_field_id ft = Cmiss_field_module_find_field_by_name(fm, "xi");
	EXPECT_NE(static_cast<Cmiss_field *>(0), ft);

	Cmiss_field_id f2 = Cmiss_field_module_create_derivative(fm, f1, 1);
	EXPECT_NE(static_cast<Cmiss_field *>(0), f2);

	Cmiss_field_cache_id fc = Cmiss_field_module_create_cache(fm);

	Cmiss_mesh_id mesh = Cmiss_field_module_find_mesh_by_dimension(fm, 3);
	EXPECT_NE(static_cast<Cmiss_mesh *>(0), mesh);

	Cmiss_element_id el = Cmiss_mesh_find_element_by_identifier(mesh, 1);
	EXPECT_NE(static_cast<Cmiss_element *>(0), el);

	double chart_coordinates[] = {0.6, 0.2, 0.45};
	int result = Cmiss_field_cache_set_mesh_location(fc, el, 3, chart_coordinates);
	EXPECT_EQ(CMISS_OK, result);

	double outvalues[3];
	result = Cmiss_field_evaluate_real(f2, fc, 3, outvalues);
	EXPECT_EQ(CMISS_OK, result);
	EXPECT_EQ(1.0, outvalues[0]);
	EXPECT_EQ(0.0, outvalues[1]);
	EXPECT_EQ(0.0, outvalues[2]);

	Cmiss_element_destroy(&el);
	Cmiss_mesh_destroy(&mesh);
	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_destroy(&ft);
	Cmiss_field_cache_destroy(&fc);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_field_module_create_curl, invalid_args)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id fm = Cmiss_region_get_field_module(root_region);

	EXPECT_NE(static_cast<Cmiss_field_module *>(0), fm);

	Cmiss_field_id f0 = Cmiss_field_module_create_curl(0, 0, 0);
	EXPECT_EQ(0, f0);

	Cmiss_field_id f1 = Cmiss_field_module_create_curl(fm, 0, 0);
	EXPECT_EQ(0, f1);

	double values[] = {6.0, 1.0, 2.5};
	Cmiss_field_id f2 = Cmiss_field_module_create_constant(fm, 3, values);

	Cmiss_field_id f3 = Cmiss_field_module_create_curl(fm, f2, 0);
	EXPECT_EQ(0, f3);

	Cmiss_field_destroy(&f0);
	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_destroy(&f3);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_field_module_create_curl, valid_args)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id fm = Cmiss_region_get_field_module(root_region);

	Cmiss_region_read_file(root_region, TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE));

	Cmiss_field_id f1 = Cmiss_field_module_find_field_by_name(fm, "coordinates");
	EXPECT_NE(static_cast<Cmiss_field *>(0), f1);

	double values[] = {2.0, 3.0, 5.0};
	Cmiss_field_id c1 = Cmiss_field_module_create_constant(fm, 3, values);

	Cmiss_field_id f2 = Cmiss_field_module_create_curl(fm, c1, f1);
	EXPECT_NE(static_cast<Cmiss_field *>(0), f2);

	Cmiss_field_cache_id fc = Cmiss_field_module_create_cache(fm);

	Cmiss_mesh_id mesh = Cmiss_field_module_find_mesh_by_dimension(fm, 3);
	EXPECT_NE(static_cast<Cmiss_mesh *>(0), mesh);

	Cmiss_element_id el = Cmiss_mesh_find_element_by_identifier(mesh, 1);
	EXPECT_NE(static_cast<Cmiss_element *>(0), el);

	double chart_coordinates[] = {0.6, 0.2, 0.45};
	int result = Cmiss_field_cache_set_mesh_location(fc, el, 3, chart_coordinates);
	EXPECT_EQ(CMISS_OK, result);

	double outvalues[3];
	result = Cmiss_field_evaluate_real(f2, fc, 3, outvalues);
	EXPECT_EQ(CMISS_OK, result);
	EXPECT_EQ(0.0, outvalues[0]);
	EXPECT_EQ(0.0, outvalues[1]);
	EXPECT_EQ(0.0, outvalues[2]);

	Cmiss_element_destroy(&el);
	Cmiss_mesh_destroy(&mesh);
	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&c1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_cache_destroy(&fc);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_field_module_create_divergence, invalid_args)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id fm = Cmiss_region_get_field_module(root_region);

	EXPECT_NE(static_cast<Cmiss_field_module *>(0), fm);

	Cmiss_field_id f0 = Cmiss_field_module_create_divergence(0, 0, 0);
	EXPECT_EQ(0, f0);

	Cmiss_field_id f1 = Cmiss_field_module_create_divergence(fm, 0, 0);
	EXPECT_EQ(0, f1);

	double values[] = {6.0, 1.0, 2.5};
	Cmiss_field_id f2 = Cmiss_field_module_create_constant(fm, 3, values);

	Cmiss_field_id f3 = Cmiss_field_module_create_divergence(fm, f2, 0);
	EXPECT_EQ(0, f3);

	Cmiss_field_destroy(&f0);
	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_destroy(&f3);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_field_module_create_divergence, valid_args)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id fm = Cmiss_region_get_field_module(root_region);

	Cmiss_region_read_file(root_region, TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE));

	Cmiss_field_id f1 = Cmiss_field_module_find_field_by_name(fm, "coordinates");
	EXPECT_NE(static_cast<Cmiss_field *>(0), f1);

	double values[] = {2.0, 3.0, 5.0};
	Cmiss_field_id c1 = Cmiss_field_module_create_constant(fm, 3, values);

	Cmiss_field_id f2 = Cmiss_field_module_create_divergence(fm, c1, f1);
	EXPECT_NE(static_cast<Cmiss_field *>(0), f2);

	Cmiss_field_cache_id fc = Cmiss_field_module_create_cache(fm);

	Cmiss_mesh_id mesh = Cmiss_field_module_find_mesh_by_dimension(fm, 3);
	EXPECT_NE(static_cast<Cmiss_mesh *>(0), mesh);

	Cmiss_element_id el = Cmiss_mesh_find_element_by_identifier(mesh, 1);
	EXPECT_NE(static_cast<Cmiss_element *>(0), el);

	double chart_coordinates[] = {0.6, 0.2, 0.45};
	int result = Cmiss_field_cache_set_mesh_location(fc, el, 3, chart_coordinates);
	EXPECT_EQ(CMISS_OK, result);

	double outvalues[1];
	result = Cmiss_field_evaluate_real(f2, fc, 1, outvalues);
	EXPECT_EQ(CMISS_OK, result);
	EXPECT_EQ(0.0, outvalues[0]);

	Cmiss_element_destroy(&el);
	Cmiss_mesh_destroy(&mesh);
	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&c1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_cache_destroy(&fc);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_field_module_create_divergence, grad_mag)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id fm = Cmiss_region_get_field_module(root_region);

	Cmiss_region_read_file(root_region, TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE));

	Cmiss_field_id f1 = Cmiss_field_module_find_field_by_name(fm, "coordinates");
	EXPECT_NE(static_cast<Cmiss_field *>(0), f1);

	Cmiss_field_id c1 = Cmiss_field_module_create_magnitude(fm, f1);

	Cmiss_field_id c2 = Cmiss_field_module_create_gradient(fm, c1, f1);

	Cmiss_field_id f2 = Cmiss_field_module_create_divergence(fm, c2, f1);
	EXPECT_NE(static_cast<Cmiss_field *>(0), f2);

	Cmiss_field_cache_id fc = Cmiss_field_module_create_cache(fm);

	Cmiss_mesh_id mesh = Cmiss_field_module_find_mesh_by_dimension(fm, 3);
	EXPECT_NE(static_cast<Cmiss_mesh *>(0), mesh);

	Cmiss_element_id el = Cmiss_mesh_find_element_by_identifier(mesh, 1);
	EXPECT_NE(static_cast<Cmiss_element *>(0), el);

	double chart_coordinates[] = {0.6, 0.2, 0.45};
	int result = Cmiss_field_cache_set_mesh_location(fc, el, 3, chart_coordinates);
	EXPECT_EQ(CMISS_OK, result);

	double outvalues[1];
	result = Cmiss_field_evaluate_real(f2, fc, 1, outvalues);
	EXPECT_NE(CMISS_OK, result);
	//EXPECT_EQ(0.0, outvalues[0]);

	Cmiss_element_destroy(&el);
	Cmiss_mesh_destroy(&mesh);
	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&c1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_destroy(&c2);
	Cmiss_field_cache_destroy(&fc);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_field_module_create_gradient, invalid_args)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id fm = Cmiss_region_get_field_module(root_region);

	EXPECT_NE(static_cast<Cmiss_field_module *>(0), fm);

	Cmiss_field_id f0 = Cmiss_field_module_create_gradient(0, 0, 0);
	EXPECT_EQ(0, f0);

	Cmiss_field_id f1 = Cmiss_field_module_create_gradient(fm, 0, 0);
	EXPECT_EQ(0, f1);

	double values[] = {6.0, 1.0, 2.5};
	Cmiss_field_id f2 = Cmiss_field_module_create_constant(fm, 3, values);

	Cmiss_field_id f3 = Cmiss_field_module_create_gradient(fm, f2, 0);
	EXPECT_EQ(0, f3);

	Cmiss_field_id f4 = Cmiss_field_module_create_gradient(fm, 0, f2);
	EXPECT_EQ(0, f4);

	Cmiss_field_destroy(&f0);
	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_destroy(&f3);
	Cmiss_field_destroy(&f4);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_field_module_create_gradient, valid_args)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id fm = Cmiss_region_get_field_module(root_region);

	Cmiss_region_read_file(root_region, TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE));

	Cmiss_field_id f1 = Cmiss_field_module_find_field_by_name(fm, "coordinates");
	EXPECT_NE(static_cast<Cmiss_field *>(0), f1);

	Cmiss_field_id mag = Cmiss_field_module_create_magnitude(fm, f1);

	Cmiss_field_id f2 = Cmiss_field_module_create_gradient(fm, mag, f1);
	EXPECT_NE(static_cast<Cmiss_field *>(0), f2);

	Cmiss_field_cache_id fc = Cmiss_field_module_create_cache(fm);

	Cmiss_mesh_id mesh = Cmiss_field_module_find_mesh_by_dimension(fm, 3);
	EXPECT_NE(static_cast<Cmiss_mesh *>(0), mesh);

	Cmiss_element_id el = Cmiss_mesh_find_element_by_identifier(mesh, 1);
	EXPECT_NE(static_cast<Cmiss_element *>(0), el);

	double chart_coordinates[] = {0.0, 1.0, 0.0};
	int result = Cmiss_field_cache_set_mesh_location(fc, el, 3, chart_coordinates);
	EXPECT_EQ(CMISS_OK, result);

	double outvalues[3];
	result = Cmiss_field_evaluate_real(f2, fc, 3, outvalues);
	EXPECT_EQ(CMISS_OK, result);
	EXPECT_EQ(0.0, outvalues[0]);
	EXPECT_EQ(1.0, outvalues[1]);
	EXPECT_EQ(0.0, outvalues[2]);

	Cmiss_element_destroy(&el);
	Cmiss_mesh_destroy(&mesh);
	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&mag);
	Cmiss_field_destroy(&f2);
	Cmiss_field_cache_destroy(&fc);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

