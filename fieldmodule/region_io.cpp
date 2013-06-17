
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/region.h>
#include <zinc/status.h>
#include <zinc/stream.h>

TEST(region_file_input, invalid_args)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);

	Cmiss_stream_information_id si = Cmiss_region_create_stream_information(
		root_region);
	EXPECT_NE(static_cast<Cmiss_stream_information *>(0), si);

	Cmiss_stream_resource_id sr = Cmiss_stream_information_create_resource_file(
		si, "region_input.exregion");
	EXPECT_NE(static_cast<Cmiss_stream_resource *>(0), sr);

	int result = Cmiss_region_read(root_region, si);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_stream_resource_destroy(&sr);
	Cmiss_stream_information_destroy(&si);

	Cmiss_region_id plate_region = Cmiss_region_find_child_by_name(
		root_region, "plate");
	EXPECT_NE(static_cast<Cmiss_region *>(0), plate_region);
	Cmiss_region_destroy(&plate_region);

	Cmiss_region_id tetrahedron_region = Cmiss_region_find_child_by_name(
		root_region, "tetrahedron");
	EXPECT_NE(static_cast<Cmiss_region *>(0), tetrahedron_region);

	Cmiss_region_id starburst_region = Cmiss_region_find_child_by_name(
		tetrahedron_region, "starburst");
	EXPECT_NE(static_cast<Cmiss_region *>(0), starburst_region);

	Cmiss_region_destroy(&starburst_region);
	Cmiss_region_destroy(&tetrahedron_region);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

