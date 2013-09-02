/***************************************************************************//**
 * FILE : time.hpp
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
#ifndef CMZN_TIMENOTIFIER_HPP__
#define CMZN_TIMENOTIFIER_HPP__

#include "zinc/timenotifier.h"

namespace zinc
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

}  // namespace zinc

#endif
