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

class Timenotifierevent
{
protected:
	cmzn_timenotifierevent_id id;

public:

	Timenotifierevent() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Timenotifierevent(cmzn_timenotifierevent_id in_timenotifier_event_id) :
		id(in_timenotifier_event_id)
	{  }

	Timenotifierevent(const Timenotifierevent& timenotifierEvent) :
		id(cmzn_timenotifierevent_access(timenotifierEvent.id))
	{  }

	Timenotifierevent& operator=(const Timenotifierevent& timenotifierEvent)
	{
		cmzn_timenotifierevent_id temp_id = cmzn_timenotifierevent_access(timenotifierEvent.id);
		if (0 != id)
		{
			cmzn_timenotifierevent_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Timenotifierevent()
	{
		if (0 != id)
		{
			cmzn_timenotifierevent_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_timenotifierevent_id getId()
	{
		return id;
	}

	double getTime() const
	{
		return cmzn_timenotifierevent_get_time(id);
	}

};

class Timenotifiercallback
{
friend class Timenotifier;
private:
	Timenotifiercallback(Timenotifiercallback&); // not implemented
	Timenotifiercallback& operator=(Timenotifiercallback&); // not implemented

	static void C_callback(cmzn_timenotifierevent_id timenotifierevent_id, void *callbackVoid)
	{
		Timenotifierevent timenotifierevent(cmzn_timenotifierevent_access(timenotifierevent_id));
		Timenotifiercallback *callback = reinterpret_cast<Timenotifiercallback *>(callbackVoid);
		(*callback)(timenotifierevent);
	}

  virtual void operator()(const Timenotifierevent &timenotifierevent) = 0;

protected:
  Timenotifiercallback()
	{ }

public:
	virtual ~Timenotifiercallback()
	{ }
};

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

	int setCallback(Timenotifiercallback& callback)
	{
		return cmzn_timenotifier_set_callback(id, callback.C_callback, static_cast<void*>(&callback));
	}

	int clearCallback()
	{
		return cmzn_timenotifier_clear_callback(id);
	}

};

}  // namespace Zinc
}

#endif
