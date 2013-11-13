#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/field.h>
#include <zinc/fieldcache.h>
#include <zinc/fieldconstant.h>
#include <zinc/fieldvectoroperators.h>
#include <zinc/status.h>

#include <zinc/field.hpp>
#include <zinc/fieldcache.hpp>
#include <zinc/fieldconstant.hpp>
#include <zinc/fieldvectoroperators.hpp>
#include <zinc/status.hpp>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

struct RecordChange
{
	int changeFlags;
};

void fieldmoduleCallback(cmzn_fieldmoduleevent_id fieldmoduleevent,
	void *recordChangeVoid)
{
	RecordChange *recordChange = reinterpret_cast<RecordChange*>(recordChangeVoid);
	recordChange->changeFlags = cmzn_fieldmoduleevent_get_change_flags(fieldmoduleevent);
}

TEST(cmzn_fieldmodulenotifier, change_callback)
{
	ZincTestSetup zinc;
	int result;

	cmzn_fieldmodulenotifier_id notifier = cmzn_fieldmodule_create_notifier(zinc.fm);
	EXPECT_NE(static_cast<cmzn_fieldmodulenotifier_id>(0), notifier);

	RecordChange recordChange;
	recordChange.changeFlags = CMZN_FIELDMODULEEVENT_CHANGE_FLAG_NONE;
	EXPECT_EQ(CMZN_OK, result = cmzn_fieldmodulenotifier_set_callback(notifier,
		fieldmoduleCallback, static_cast<void*>(&recordChange)));

	void *tmp = cmzn_fieldmodulenotifier_get_callback_user_data(notifier);
	EXPECT_EQ(static_cast<void*>(&recordChange), tmp);

	const double value1 = 2.0;
	cmzn_field_id joe = cmzn_fieldmodule_create_field_constant(zinc.fm, 1, &value1);
	EXPECT_NE((cmzn_field_id)0, joe);
	EXPECT_EQ(CMZN_FIELDMODULEEVENT_CHANGE_FLAG_ADD, recordChange.changeFlags);

	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_name(joe, "joe"));
	EXPECT_EQ(CMZN_FIELDMODULEEVENT_CHANGE_FLAG_IDENTIFIER, recordChange.changeFlags);

	cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(zinc.fm);
	double value2 = 4.5;
	EXPECT_EQ(CMZN_OK, result = cmzn_field_assign_real(joe, cache, 1, &value2));
	EXPECT_EQ(CMZN_FIELDMODULEEVENT_CHANGE_FLAG_DEFINITION, recordChange.changeFlags);

	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_managed(joe, true));
	EXPECT_EQ(CMZN_FIELDMODULEEVENT_CHANGE_FLAG_METADATA, recordChange.changeFlags);

	cmzn_field_id fred = cmzn_fieldmodule_create_field_magnitude(zinc.fm, joe);
	EXPECT_NE((cmzn_field_id)0, fred);
	EXPECT_EQ(CMZN_FIELDMODULEEVENT_CHANGE_FLAG_ADD, recordChange.changeFlags);

	EXPECT_EQ(CMZN_OK, result = cmzn_field_assign_real(joe, cache, 1, &value1));
	EXPECT_EQ(CMZN_FIELDMODULEEVENT_CHANGE_FLAG_DEFINITION | CMZN_FIELDMODULEEVENT_CHANGE_FLAG_DEPENDENCY, recordChange.changeFlags);

	EXPECT_EQ(CMZN_OK, cmzn_field_set_managed(joe, false));
	cmzn_field_destroy(&joe);
	cmzn_field_destroy(&fred);
	EXPECT_EQ(CMZN_FIELDMODULEEVENT_CHANGE_FLAG_REMOVE, recordChange.changeFlags);

	cmzn_fieldcache_destroy(&cache);
	EXPECT_EQ(CMZN_OK, result = cmzn_fieldmodulenotifier_clear_callback(notifier));
	EXPECT_EQ(CMZN_OK, result = cmzn_fieldmodulenotifier_destroy(&notifier));
}

// check selection callbacks safely handle destruction of scene
TEST(cmzn_fieldmodulenotifier, destroy_region_final_callback)
{
	ZincTestSetup zinc;
	int result;

	cmzn_region_id region = cmzn_region_create_child(zinc.root_region, "child");
	EXPECT_NE(static_cast<cmzn_region_id>(0), region);
	cmzn_fieldmodule_id fm = cmzn_region_get_fieldmodule(region);

	cmzn_fieldmodulenotifier_id notifier = cmzn_fieldmodule_create_notifier(fm);
	EXPECT_NE(static_cast<cmzn_fieldmodulenotifier_id>(0), notifier);

	RecordChange recordChange;
	recordChange.changeFlags = CMZN_FIELDMODULEEVENT_CHANGE_FLAG_NONE;
	EXPECT_EQ(CMZN_OK, result = cmzn_fieldmodulenotifier_set_callback(notifier,
		fieldmoduleCallback, static_cast<void*>(&recordChange)));

	cmzn_region_remove_child(zinc.root_region, region);
	EXPECT_EQ(CMZN_FIELDMODULEEVENT_CHANGE_FLAG_NONE, recordChange.changeFlags);

	cmzn_fieldmodule_destroy(&fm);
	cmzn_region_destroy(&region);
	EXPECT_EQ(CMZN_FIELDMODULEEVENT_CHANGE_FLAG_FINAL, recordChange.changeFlags);

	EXPECT_EQ(CMZN_OK, result = cmzn_fieldmodulenotifier_destroy(&notifier));
}

class FieldmodulecallbackRecordChange : public Fieldmodulecallback
{
public:
	int changeFlags;

	FieldmodulecallbackRecordChange() :
		changeFlags(Fieldmoduleevent::CHANGE_FLAG_NONE)
	{ }

	virtual void operator()(const Fieldmoduleevent &event)
	{
		this->changeFlags = event.getChangeFlags();
	}
};

TEST(ZincFieldmodulenotifier, changeCallback)
{
	ZincTestSetupCpp zinc;
	int result;

	Fieldmodulenotifier notifier = zinc.fm.createNotifier();
	EXPECT_TRUE(notifier.isValid());

	FieldmodulecallbackRecordChange recordChange;
	recordChange.changeFlags = CMZN_FIELDMODULEEVENT_CHANGE_FLAG_NONE;
	EXPECT_EQ(CMZN_OK, result = notifier.setCallback(recordChange));

	const double value1 = 2.0;
	Field joe = zinc.fm.createFieldConstant(1, &value1);
	EXPECT_TRUE(joe.isValid());
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_ADD, recordChange.changeFlags);

	EXPECT_EQ(CMZN_OK, result = joe.setName("joe"));
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_IDENTIFIER, recordChange.changeFlags);

	Fieldcache cache = zinc.fm.createFieldcache();
	double value2 = 4.5;
	EXPECT_EQ(CMZN_OK, result = joe.assignReal(cache, 1, &value2));
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_DEFINITION, recordChange.changeFlags);

	EXPECT_EQ(CMZN_OK, result = joe.setManaged(true));
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_METADATA, recordChange.changeFlags);

	Field fred = zinc.fm.createFieldMagnitude(joe);
	EXPECT_TRUE(fred.isValid());
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_ADD, recordChange.changeFlags);

	EXPECT_EQ(CMZN_OK, result = joe.assignReal(cache, 1, &value1));
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_DEFINITION | Fieldmoduleevent::CHANGE_FLAG_DEPENDENCY, recordChange.changeFlags);

	EXPECT_EQ(CMZN_OK, result = joe.setManaged(false));
	Field noField;
	joe = noField;
	fred = noField;
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_REMOVE, recordChange.changeFlags);

	EXPECT_EQ(CMZN_OK, result = notifier.clearCallback());
}
