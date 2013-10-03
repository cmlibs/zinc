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

class Timenotifier
{
protected:
	cmzn_timenotifier_id id;

public:

	Timenotifier() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Timenotifier(cmzn_timenotifier_id in_timenotifier_id) :
		id(in_timenotifier_id)
	{  }

	Timenotifier(const Timenotifier& timenotifier) :
		id(cmzn_timenotifier_access(timenotifier.id))
	{  }

	Timenotifier& operator=(const Timenotifier& timenotifier)
	{
		cmzn_timenotifier_id temp_id = cmzn_timenotifier_access(timenotifier.id);
		if (0 != id)
		{
			cmzn_timenotifier_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Timenotifier()
	{
		if (0 != id)
		{
			cmzn_timenotifier_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_timenotifier_id getId()
	{
		return id;
	}
/*
	int addCallback(cmzn_timenotifier_callback callback, void *user_data)
	{
		return cmzn_timenotifier_add_callback(id, callback, user_data);
	}

	int removeCallback(cmzn_timenotifier_callback callback, void *user_data)
	{
		return cmzn_timenotifier_remove_callback(id, callback, user_data);
	}
*/

	double getTime()
	{
		return cmzn_timenotifier_get_time(id);
	}

	int setFrequency(double frequency)
	{
		return cmzn_timenotifier_regular_set_frequency(id, frequency);
	}

	int setOffset(double timeOffset)
	{
		return cmzn_timenotifier_regular_set_offset(id, timeOffset);
	}

};

}  // namespace Zinc
}

#endif
