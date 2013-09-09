/***************************************************************************//**
 * FILE : timesequence.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_TIMESEQUENCE_HPP__
#define CMZN_TIMESEQUENCE_HPP__

#include "zinc/timesequence.h"

namespace OpenCMISS
{
namespace Zinc
{

class TimeSequence
{
protected:
	cmzn_time_sequence_id id;

public:

	TimeSequence() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit TimeSequence(cmzn_time_sequence_id in_time_sequence_id) :
		id(in_time_sequence_id)
	{  }

	TimeSequence(const TimeSequence& timeSequence) :
		id(cmzn_time_sequence_access(timeSequence.id))
	{  }

	TimeSequence& operator=(const TimeSequence& timeSequence)
	{
		cmzn_time_sequence_id temp_id = cmzn_time_sequence_access(timeSequence.id);
		if (0 != id)
		{
			cmzn_time_sequence_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~TimeSequence()
	{
		if (0 != id)
		{
			cmzn_time_sequence_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_time_sequence_id getId()
	{
		return id;
	}

	int setValue(int timeIndex, double time)
	{
		return cmzn_time_sequence_set_value(id, timeIndex, time);

	}

};

}  // namespace Zinc
}

#endif
