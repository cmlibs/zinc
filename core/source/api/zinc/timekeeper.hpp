/***************************************************************************//**
 * FILE : timekeeper.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_TIMEKEEPER_HPP__
#define CMZN_TIMEKEEPER_HPP__

#include "zinc/timekeeper.h"
#include "zinc/timenotifier.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class TimeKeeper
{
protected:
	cmzn_time_keeper_id id;

public:

	TimeKeeper() : id(0)
	{   }

	// takes ownership of C handle, responsibility for destroying it
	explicit TimeKeeper(cmzn_time_keeper_id in_time_keeper_id) :
		id(in_time_keeper_id)
	{  }

	TimeKeeper(const TimeKeeper& timeKeeper) :
		id(cmzn_time_keeper_access(timeKeeper.id))
	{  }

	TimeKeeper& operator=(const TimeKeeper& timeKeeper)
	{
		cmzn_time_keeper_id temp_id = cmzn_time_keeper_access(timeKeeper.id);
		if (0 != id)
		{
			cmzn_time_keeper_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~TimeKeeper()
	{
		if (0 != id)
		{
			cmzn_time_keeper_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_time_keeper_id getId()
	{
		return id;
	}

	enum Attribute
	{
		ATTRIBUTE_INVALID = CMZN_TIME_KEEPER_ATTRIBUTE_INVALID,
		ATTRIBUTE_TIME = CMZN_TIME_KEEPER_ATTRIBUTE_TIME,
		ATTRIBUTE_MINIMUM_TIME = CMZN_TIME_KEEPER_ATTRIBUTE_MINIMUM_TIME,
		ATTRIBUTE_MAXIMUM_TIME = CMZN_TIME_KEEPER_ATTRIBUTE_MAXIMUM_TIME
	};

	double getAttributeReal(Attribute attribute)
	{
		return cmzn_time_keeper_get_attribute_real(id,
			static_cast<cmzn_time_keeper_attribute>(attribute));
	}

	int setAttributeReal(Attribute attribute, double value)
	{
		return cmzn_time_keeper_set_attribute_real(id,
			static_cast<cmzn_time_keeper_attribute>(attribute), value);
	}

	TimeNotifier createNotifierRegular(double updateFrequency, double timeOffset)
	{
		return TimeNotifier(cmzn_time_keeper_create_notifier_regular(
			id, updateFrequency, timeOffset));
	}

	int addTimeNotifier(TimeNotifier timeNotifier)
	{
		return cmzn_time_keeper_add_time_notifier(id, timeNotifier.getId());
	}

	int removeTimeNotifier(TimeNotifier timeNotifier)
	{
		return cmzn_time_keeper_remove_time_notifier(id, timeNotifier.getId());
	}

};

}  // namespace Zinc
}

#endif
