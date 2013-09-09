/***************************************************************************//**
 * FILE : timenotifier.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_TIMENOTIFIER_HPP__
#define CMZN_TIMENOTIFIER_HPP__

#include "zinc/timenotifier.h"

namespace OpenCMISS
{
namespace Zinc
{
class TimeNotifier;
/*
typedef int (*Time_notifier_callback)(cmzn_time_notifier_id time_notifier,
	double current_time, void *user_data);
*/
class TimeNotifier
{
protected:
	cmzn_time_notifier_id id;

public:

	TimeNotifier() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit TimeNotifier(cmzn_time_notifier_id in_time_notifier_id) :
		id(in_time_notifier_id)
	{  }

	TimeNotifier(const TimeNotifier& time_notifier) :
		id(cmzn_time_notifier_access(time_notifier.id))
	{  }

	TimeNotifier& operator=(const TimeNotifier& time_notifierNotifier)
	{
		cmzn_time_notifier_id temp_id = cmzn_time_notifier_access(time_notifierNotifier.id);
		if (0 != id)
		{
			cmzn_time_notifier_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~TimeNotifier()
	{
		if (0 != id)
		{
			cmzn_time_notifier_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_time_notifier_id getId()
	{
		return id;
	}
/*
	int addCallback(Time_notifier_callback callback, void *user_data)
	{
		return cmzn_time_notifier_add_callback(id, callback, user_data);
	}

	int removeCallback(Time_notifier_callback callback, void *user_data)
	{
		return cmzn_time_notifier_remove_callback(id, callback, user_data);
	}
*/
	int setFrequency(double frequency)
	{
		return cmzn_time_notifier_regular_set_frequency(id, frequency);
	}

	int setOffset(double timeOffset)
	{
		return cmzn_time_notifier_regular_set_offset(id, timeOffset);
	}

};

}  // namespace Zinc
}

#endif
