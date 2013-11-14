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
	cmzn_fieldmoduleevent_id lastEvent;

	RecordChange() :
		lastEvent(0)
	{
	}

	~RecordChange()
	{
		cmzn_fieldmoduleevent_destroy(&lastEvent);
	}
};

void fieldmoduleCallback(cmzn_fieldmoduleevent_id event,
	void *recordChangeVoid)
{
	RecordChange *recordChange = reinterpret_cast<RecordChange*>(recordChangeVoid);
	if (recordChange->lastEvent)
		cmzn_fieldmoduleevent_destroy(&recordChange->lastEvent);
	recordChange->lastEvent = cmzn_fieldmoduleevent_access(event);
}

TEST(cmzn_fieldmodulenotifier, change_callback)
{
	ZincTestSetup zinc;
	int result;

	cmzn_fieldmodulenotifier_id notifier = cmzn_fieldmodule_create_fieldmodulenotifier(zinc.fm);
	EXPECT_NE(static_cast<cmzn_fieldmodulenotifier_id>(0), notifier);

	RecordChange recordChange;
	EXPECT_EQ(CMZN_OK, result = cmzn_fieldmodulenotifier_set_callback(notifier,
		fieldmoduleCallback, static_cast<void*>(&recordChange)));

	void *tmp = cmzn_fieldmodulenotifier_get_callback_user_data(notifier);
	EXPECT_EQ(static_cast<void*>(&recordChange), tmp);

	const double value1 = 2.0;
	cmzn_field_id joe = cmzn_fieldmodule_create_field_constant(zinc.fm, 1, &value1);
	EXPECT_NE((cmzn_field_id)0, joe);
	EXPECT_EQ(CMZN_FIELDMODULEEVENT_CHANGE_FLAG_ADD, result = cmzn_fieldmoduleevent_get_change_flags(recordChange.lastEvent));

	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_name(joe, "joe"));
	EXPECT_EQ(CMZN_FIELDMODULEEVENT_CHANGE_FLAG_IDENTIFIER, result = cmzn_fieldmoduleevent_get_change_flags(recordChange.lastEvent));

	cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(zinc.fm);
	double value2 = 4.5;
	EXPECT_EQ(CMZN_OK, result = cmzn_field_assign_real(joe, cache, 1, &value2));
	EXPECT_EQ(CMZN_FIELDMODULEEVENT_CHANGE_FLAG_DEFINITION, result = cmzn_fieldmoduleevent_get_change_flags(recordChange.lastEvent));

	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_managed(joe, true));
	EXPECT_EQ(CMZN_FIELDMODULEEVENT_CHANGE_FLAG_METADATA, result = cmzn_fieldmoduleevent_get_change_flags(recordChange.lastEvent));

	cmzn_field_id fred = cmzn_fieldmodule_create_field_magnitude(zinc.fm, joe);
	EXPECT_NE((cmzn_field_id)0, fred);
	EXPECT_EQ(CMZN_FIELDMODULEEVENT_CHANGE_FLAG_ADD, result = cmzn_fieldmoduleevent_get_change_flags(recordChange.lastEvent));
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_ADD, result = cmzn_fieldmoduleevent_get_field_change_flags(recordChange.lastEvent, fred));
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_NONE, result = cmzn_fieldmoduleevent_get_field_change_flags(recordChange.lastEvent, joe));

	EXPECT_EQ(CMZN_OK, result = cmzn_field_assign_real(joe, cache, 1, &value1));
	EXPECT_EQ(CMZN_FIELDMODULEEVENT_CHANGE_FLAG_DEFINITION | CMZN_FIELDMODULEEVENT_CHANGE_FLAG_DEPENDENCY,
		result = cmzn_fieldmoduleevent_get_change_flags(recordChange.lastEvent));
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_DEFINITION, result = cmzn_fieldmoduleevent_get_field_change_flags(recordChange.lastEvent, joe));
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_DEPENDENCY, result = cmzn_fieldmoduleevent_get_field_change_flags(recordChange.lastEvent, fred));

	EXPECT_EQ(CMZN_OK, cmzn_field_set_managed(joe, false));
	cmzn_field_destroy(&joe);
	cmzn_field_destroy(&fred);
	EXPECT_EQ(CMZN_FIELDMODULEEVENT_CHANGE_FLAG_REMOVE, result = cmzn_fieldmoduleevent_get_change_flags(recordChange.lastEvent));

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

	cmzn_fieldmodulenotifier_id notifier = cmzn_fieldmodule_create_fieldmodulenotifier(fm);
	EXPECT_NE(static_cast<cmzn_fieldmodulenotifier_id>(0), notifier);

	RecordChange recordChange;
	EXPECT_EQ(CMZN_OK, result = cmzn_fieldmodulenotifier_set_callback(notifier,
		fieldmoduleCallback, static_cast<void*>(&recordChange)));

	cmzn_region_remove_child(zinc.root_region, region);
	EXPECT_EQ(CMZN_FIELDMODULEEVENT_CHANGE_FLAG_NONE, result = cmzn_fieldmoduleevent_get_change_flags(recordChange.lastEvent));

	cmzn_fieldmodule_destroy(&fm);
	cmzn_region_destroy(&region);
	EXPECT_EQ(CMZN_FIELDMODULEEVENT_CHANGE_FLAG_FINAL, result = cmzn_fieldmoduleevent_get_change_flags(recordChange.lastEvent));

	EXPECT_EQ(CMZN_OK, result = cmzn_fieldmodulenotifier_destroy(&notifier));
}

class FieldmodulecallbackRecordChange : public Fieldmodulecallback
{
public:
	Fieldmoduleevent lastEvent;

	FieldmodulecallbackRecordChange()
	{ }

	virtual void operator()(const Fieldmoduleevent &event)
	{
		this->lastEvent = event;
	}
};

TEST(ZincFieldmodulenotifier, changeCallback)
{
	ZincTestSetupCpp zinc;
	int result;

	Fieldmodulenotifier notifier = zinc.fm.createFieldmodulenotifier();
	EXPECT_TRUE(notifier.isValid());

	FieldmodulecallbackRecordChange recordChange;
	EXPECT_EQ(CMZN_OK, result = notifier.setCallback(recordChange));

	const double value1 = 2.0;
	Field joe = zinc.fm.createFieldConstant(1, &value1);
	EXPECT_TRUE(joe.isValid());
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_ADD, result = recordChange.lastEvent.getChangeFlags());

	EXPECT_EQ(CMZN_OK, result = joe.setName("joe"));
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_IDENTIFIER, result = recordChange.lastEvent.getChangeFlags());

	Fieldcache cache = zinc.fm.createFieldcache();
	double value2 = 4.5;
	EXPECT_EQ(CMZN_OK, result = joe.assignReal(cache, 1, &value2));
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_DEFINITION, result = recordChange.lastEvent.getChangeFlags());

	EXPECT_EQ(CMZN_OK, result = joe.setManaged(true));
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_METADATA, result = recordChange.lastEvent.getChangeFlags());

	Field fred = zinc.fm.createFieldMagnitude(joe);
	EXPECT_TRUE(fred.isValid());
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_ADD, result = recordChange.lastEvent.getChangeFlags());
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_ADD, result = recordChange.lastEvent.getFieldChangeFlags(fred));
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_NONE, result = recordChange.lastEvent.getFieldChangeFlags(joe));

	EXPECT_EQ(CMZN_OK, result = joe.assignReal(cache, 1, &value1));
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_DEFINITION | Fieldmoduleevent::CHANGE_FLAG_DEPENDENCY,
		result = recordChange.lastEvent.getChangeFlags());
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_DEFINITION, result = recordChange.lastEvent.getFieldChangeFlags(joe));
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_DEPENDENCY, result = recordChange.lastEvent.getFieldChangeFlags(fred));

	EXPECT_EQ(CMZN_OK, result = joe.setManaged(false));
	Field noField;
	joe = noField;
	fred = noField;
	EXPECT_EQ(Fieldmoduleevent::CHANGE_FLAG_REMOVE, result = recordChange.lastEvent.getChangeFlags());

	EXPECT_EQ(CMZN_OK, result = notifier.clearCallback());
}
