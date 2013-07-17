
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/region.h>
#include <zinc/fieldmodule.h>
#include <zinc/field.h>
#include <zinc/fieldfibres.h>
#include <zinc/fieldconstant.h>

TEST(Cmiss_field_module_create_fibre_axes, invalid_args)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id fm = Cmiss_region_get_field_module(root_region);

	EXPECT_NE(static_cast<Cmiss_field_module *>(0), fm);

	Cmiss_field_id f0 = Cmiss_field_module_create_fibre_axes(0, 0, 0);
	EXPECT_EQ(0, f0);

	Cmiss_field_id f1 = Cmiss_field_module_create_fibre_axes(fm, 0, 0);
	EXPECT_EQ(0, f1);

	double values[] = {3.0, 2.0, 1.0, 7.0};
	Cmiss_field_id f2 = Cmiss_field_module_create_constant(fm, 3, values);
	Cmiss_field_id f3 = Cmiss_field_module_create_constant(fm, 4, values);

	Cmiss_field_id f4 = Cmiss_field_module_create_fibre_axes(fm, f2, f3);
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

