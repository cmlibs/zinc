/***************************************************************************//**
 * FILE : timekeeper.hpp
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is libZinc.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2012
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#ifndef __ZN_TIME_KEEPER_HPP__
#define __ZN_TIME_KEEPER_HPP__

#include "zinc/timekeeper.h"
#include "zinc/timenotifier.hpp"

namespace zinc
{

class TimeKeeper
{
protected:
	Cmiss_time_keeper_id id;

public:

	TimeKeeper() : id(0)
	{   }

	// takes ownership of C handle, responsibility for destroying it
	explicit TimeKeeper(Cmiss_time_keeper_id in_time_keeper_id) :
		id(in_time_keeper_id)
	{  }

	TimeKeeper(const TimeKeeper& timeKeeper) :
		id(Cmiss_time_keeper_access(timeKeeper.id))
	{  }

	TimeKeeper& operator=(const TimeKeeper& timeKeeper)
	{
		Cmiss_time_keeper_id temp_id = Cmiss_time_keeper_access(timeKeeper.id);
		if (0 != id)
		{
			Cmiss_time_keeper_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~TimeKeeper()
	{
		if (0 != id)
		{
			Cmiss_time_keeper_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Cmiss_time_keeper_id getId()
	{
		return id;
	}

	enum Attribute
	{
		ATTRIBUTE_INVALID = CMISS_TIME_KEEPER_ATTRIBUTE_INVALID,
		ATTRIBUTE_TIME = CMISS_TIME_KEEPER_ATTRIBUTE_TIME,
		ATTRIBUTE_MINIMUM_TIME = CMISS_TIME_KEEPER_ATTRIBUTE_MINIMUM_TIME,
		ATTRIBUTE_MAXIMUM_TIME = CMISS_TIME_KEEPER_ATTRIBUTE_MAXIMUM_TIME,
		ATTRIBUTE_SPEED = CMISS_TIME_KEEPER_ATTRIBUTE_SPEED
	};

	int getAttributeReal(Attribute attribute)
	{
		return Cmiss_time_keeper_get_attribute_real(id,
			static_cast<Cmiss_time_keeper_attribute>(attribute));
	}

	int setAttributeReal(Attribute attribute, double value)
	{
		return Cmiss_time_keeper_set_attribute_real(id,
			static_cast<Cmiss_time_keeper_attribute>(attribute), value);
	}

	TimeNotifier createNotifierRegular(double updateFrequency, double timeOffset)
	{
		return TimeNotifier(Cmiss_time_keeper_create_notifier_regular(
			id, updateFrequency, timeOffset));
	}

	int addTimeNotifier(TimeNotifier timeNotifier)
	{
		return Cmiss_time_keeper_add_time_notifier(id, timeNotifier.getId());
	}

	int removeTimeNotifier(TimeNotifier timeNotifier)
	{
		return Cmiss_time_keeper_remove_time_notifier(id, timeNotifier.getId());
	}

};

}  // namespace zinc

#endif /* __ZN_TIME_KEEPER_HPP__ */
