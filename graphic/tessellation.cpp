
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/tessellation.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/tessellation.hpp"

TEST(Cmiss_tessellation_module_api, valid_args)
{
	ZincTestSetup zinc;

	Cmiss_tessellation_module_id tm = Cmiss_graphics_module_get_tessellation_module(zinc.gm);
	EXPECT_NE(static_cast<Cmiss_tessellation_module *>(0), tm);

	int result = Cmiss_tessellation_module_begin_change(tm);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_tessellation_id tessellation = Cmiss_tessellation_module_get_default_tessellation(tm);
	EXPECT_NE(static_cast<Cmiss_tessellation *>(0), tessellation);
	int value;
	result = Cmiss_tessellation_get_minimum_divisions(tessellation, 1, &value);
	EXPECT_EQ(1, result);
	EXPECT_EQ(1, value);
	result = Cmiss_tessellation_get_refinement_factors(tessellation, 1, &value);
	EXPECT_EQ(1, result);
	EXPECT_EQ(4, value);
	result = Cmiss_tessellation_get_circle_divisions(tessellation);
	EXPECT_EQ(6, result);
	Cmiss_tessellation_destroy(&tessellation);

	// following should destroy default tessellation as not managed and not used
	// otherwise it isn't possible to create a tessellation named "default" below
	result = Cmiss_tessellation_module_set_default_tessellation(tm, 0);
	EXPECT_EQ(CMISS_OK, result);

	tessellation = Cmiss_tessellation_module_get_default_points_tessellation(tm);
	EXPECT_NE(static_cast<Cmiss_tessellation *>(0), tessellation);
	result = Cmiss_tessellation_get_minimum_divisions(tessellation, 1, &value);
	EXPECT_EQ(1, result);
	EXPECT_EQ(1, value);
	result = Cmiss_tessellation_get_refinement_factors(tessellation, 1, &value);
	EXPECT_EQ(1, result);
	EXPECT_EQ(1, value);
	result = Cmiss_tessellation_get_circle_divisions(tessellation);
	EXPECT_EQ(6, result);
	Cmiss_tessellation_destroy(&tessellation);

	tessellation = Cmiss_tessellation_module_create_tessellation(tm);
	EXPECT_NE(static_cast<Cmiss_tessellation *>(0), tessellation);

	result = Cmiss_tessellation_set_name(tessellation, "default");
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_tessellation_module_end_change(tm);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_tessellation_module_set_default_tessellation(tm, tessellation);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_tessellation_set_managed(tessellation, 1);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_tessellation_id temp_tessellation = Cmiss_tessellation_module_get_default_tessellation(tm);
	EXPECT_EQ(tessellation, temp_tessellation);
	Cmiss_tessellation_destroy(&temp_tessellation);

	temp_tessellation = Cmiss_tessellation_module_find_tessellation_by_name(tm, "default");
	EXPECT_EQ(tessellation, temp_tessellation);
	Cmiss_tessellation_destroy(&temp_tessellation);

	Cmiss_tessellation_destroy(&tessellation);

	tessellation = Cmiss_tessellation_module_get_default_tessellation(tm);
	EXPECT_NE(static_cast<Cmiss_tessellation *>(0), tessellation);
	Cmiss_tessellation_destroy(&tessellation);

	Cmiss_tessellation_module_destroy(&tm);
}

TEST(Cmiss_tessellation_module_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	TessellationModule tm = zinc.gm.getTessellationModule();
	EXPECT_TRUE(tm.isValid());

	int result = tm.beginChange();
	EXPECT_EQ(CMISS_OK, result);

	Tessellation tessellation = tm.getDefaultTessellation();
	EXPECT_TRUE(tessellation.isValid());
	int value;
	result = tessellation.getMinimumDivisions(1, &value);
	EXPECT_EQ(1, result);
	EXPECT_EQ(1, value);
	result = tessellation.getRefinementFactors(1, &value);
	EXPECT_EQ(1, result);
	EXPECT_EQ(4, value);
	result = tessellation.getCircleDivisions();
	EXPECT_EQ(6, result);

	// following should destroy default tessellation as not managed and not used
	// otherwise it isn't possible to create a tessellation named "default" below
	Tessellation noTessellation;
	tessellation = noTessellation;
	result = tm.setDefaultTessellation(noTessellation);
	EXPECT_EQ(CMISS_OK, result);

	tessellation = tm.getDefaultPointsTessellation();
	EXPECT_TRUE(tessellation.isValid());
	result = tessellation.getMinimumDivisions(1, &value);
	EXPECT_EQ(1, result);
	EXPECT_EQ(1, value);
	result = tessellation.getRefinementFactors(1, &value);
	EXPECT_EQ(1, result);
	EXPECT_EQ(1, value);
	result = tessellation.getCircleDivisions();
	EXPECT_EQ(6, result);

	tessellation = tm.createTessellation();
	EXPECT_TRUE(tessellation.isValid());

	result = tessellation.setName("default");
	EXPECT_EQ(CMISS_OK, result);

	result = tm.endChange();
	EXPECT_EQ(CMISS_OK, result);

	result = tm.setDefaultTessellation( tessellation);
	EXPECT_EQ(CMISS_OK, result);

	result = tessellation.setManaged(1);
	EXPECT_EQ(CMISS_OK, result);

	Tessellation tempTessellation = tm.getDefaultTessellation();
	EXPECT_EQ(tessellation.getId(), tempTessellation.getId());

	tempTessellation = tm.findTessellationByName("default");
	EXPECT_EQ(tessellation.getId(), tempTessellation.getId());

	tessellation = tm.getDefaultTessellation();
	EXPECT_TRUE(tessellation.isValid());
}

TEST(Cmiss_tessellation_api, valid_args)
{
	ZincTestSetup zinc;

	Cmiss_tessellation_module_id tm = Cmiss_graphics_module_get_tessellation_module(zinc.gm);
	EXPECT_NE(static_cast<Cmiss_tessellation_module *>(0), tm);

	int result = Cmiss_tessellation_module_begin_change(tm);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_tessellation_id tessellation = Cmiss_tessellation_module_create_tessellation(tm);
	EXPECT_NE(static_cast<Cmiss_tessellation *>(0), tessellation);

	result = Cmiss_tessellation_set_name(tessellation, "default");
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_tessellation_module_end_change(tm);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_tessellation_set_circle_divisions(tessellation, 10);
	EXPECT_EQ(CMISS_OK, result);
	result = Cmiss_tessellation_get_circle_divisions(tessellation);
	EXPECT_EQ(10, result);

	result = Cmiss_tessellation_set_managed(tessellation, 1);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_tessellation_is_managed(tessellation);
	EXPECT_EQ(1, result);

	int inValues[3], outValues[3];
	inValues[0] = 4;
	inValues[1] = 4;
	inValues[2] = 4;

	result = Cmiss_tessellation_set_minimum_divisions(tessellation, 3, &inValues[0]);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_tessellation_get_minimum_divisions(tessellation, 3, &outValues[0]);
	EXPECT_EQ(3, result);

	result = Cmiss_tessellation_set_refinement_factors(tessellation, 3, &inValues[0]);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_tessellation_get_refinement_factors(tessellation, 3, &outValues[0]);
	EXPECT_EQ(3, result);

	Cmiss_tessellation_destroy(&tessellation);

	Cmiss_tessellation_module_destroy(&tm);
}

TEST(Cmiss_tessellation_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	TessellationModule tm = zinc.gm.getTessellationModule();
	EXPECT_TRUE(tm.isValid());

	int result = tm.beginChange();
	EXPECT_EQ(CMISS_OK, result);

	Tessellation tessellation = tm.createTessellation();
	EXPECT_TRUE(tessellation.isValid());

	result = tessellation.setName("default");
	EXPECT_EQ(CMISS_OK, result);

	result = tm.endChange();
	EXPECT_EQ(CMISS_OK, result);

	result = tessellation.setCircleDivisions(10);
	EXPECT_EQ(CMISS_OK, result);
	result = tessellation.getCircleDivisions();
	EXPECT_EQ(10, result);

	result = tessellation.setManaged(true);
	EXPECT_EQ(CMISS_OK, result);

	EXPECT_TRUE(tessellation.isManaged());

	int inValues[3], outValues[3];
	inValues[0] = 4;
	inValues[1] = 4;
	inValues[2] = 4;

	result = tessellation.setMinimumDivisions(3, &inValues[0]);
	EXPECT_EQ(CMISS_OK, result);

	result = tessellation.getMinimumDivisions(3, &outValues[0]);
	EXPECT_EQ(3, result);

	result = tessellation.setRefinementFactors(3, &inValues[0]);
	EXPECT_EQ(CMISS_OK, result);

	result = tessellation.getRefinementFactors(3, &outValues[0]);
	EXPECT_EQ(3, result);
}
