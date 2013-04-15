/***************************************************************************//**
 * time_keeper.hpp
 *
 * Declaration of time keeper classes and functions.
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
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2010
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

#ifndef TIME_KEEPER_HPP
#define TIME_KEEPER_HPP

#include "general/mystring.h"
#include "general/object.h"
#include "general/debug.h"

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_time_keeper);

#define Time_object Cmiss_time_notifier

struct Time_object;

struct Time_object_info
{
	struct Time_object *time_object;
	double next_callback_due;
	struct Time_object_info *next;
};

struct Cmiss_time_keeper
{
private:
	const char *name;
	double time;
	struct Time_object_info *time_object_info_list;
	double minimum, maximum, speed;

public:

	int access_count;

	Cmiss_time_keeper();

	virtual ~Cmiss_time_keeper();

	inline Cmiss_time_keeper *access()
	{
		++access_count;
		return this;
	}

	static inline int deaccess(Cmiss_time_keeper **time_keeper_address)
	{
		return DEACCESS(Cmiss_time_keeper)(time_keeper_address);
	}

	bool setName(const char *name_in);

	char *getName();

	int addTimeObject(struct Time_object *time_object);

	int removeTimeObject(struct Time_object *time_object);

	double getTime();

	double getMinimum();

	int setMinimum(double minimum_in);

	double getMaximum();

	int setMaximum(double maximum_in);

	int setTime(double new_time);

	int hasTimeObject();

	struct Time_object_info *getObjectInfo();

	void setTimeQuiet(double new_time);
};

enum Time_keeper_play_direction
{
	TIME_KEEPER_PLAY_FORWARD,
	TIME_KEEPER_PLAY_BACKWARD
};

#endif
