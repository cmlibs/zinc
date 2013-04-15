/*******************************************************************************
FILE : time_keeper.c

LAST MODIFIED : 21 January 2003

DESCRIPTION :
This object defines a relationship between a bunch of time objects, keeps them
in sync and allows control such as play, rewind and fast forward.
This is intended to be multithreaded......
==============================================================================*/
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
 * Portions created by the Initial Developer are Copyright (C) 2005
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
#include <math.h>
#include <stdio.h>

#include "zinc/timekeeper.h"
#include "general/debug.h"
#include "general/list_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "general/time.h"
#include "general/message.h"
#include "time/time.h"
#include "time/time_keeper.hpp"
#include "time/time_private.h"
#include "general/enumerator_conversion.hpp"

Cmiss_time_keeper::Cmiss_time_keeper():
	name(NULL),
	time(0.0),
	time_object_info_list(NULL),
	minimum(0.0), maximum(0.0),
	speed(0.0),
	access_count(1)
{
}

Cmiss_time_keeper::~Cmiss_time_keeper()
{
	struct Time_object_info *object_info = this->time_object_info_list;
	struct Time_object_info *next = 0;
	while(object_info)
	{
		if (object_info->time_object)
		{
			Time_object_set_time_keeper(object_info->time_object,
				(struct Cmiss_time_keeper *)NULL);
			object_info->time_object = (struct Time_object *)NULL;
		}
		next = object_info->next;
		DEALLOCATE(object_info);
		object_info = next;
	}

	if (name)
	{
		DEALLOCATE(name);
	}
}

bool Cmiss_time_keeper::setName(const char *name_in)
{
	char *new_name = duplicate_string(name_in);
	if (!new_name)
		return false;
	if (name)
		DEALLOCATE(name);
	name = new_name;
	return true;
}

char *Cmiss_time_keeper::getName()
{
	char *name_out = NULL;
	if (name)
	{
		name_out = duplicate_string(name);
	}

	return name_out;
}

int Cmiss_time_keeper::addTimeObject(struct Time_object *time_object)
{
	int return_code = 0;
	struct Time_object_info *object_info, *previous;

	if (time_object)
	{
		if (!Time_object_get_time_keeper(time_object))
		{
			if(ALLOCATE(object_info, struct Time_object_info, 1))
			{
				object_info->time_object = time_object;
				Time_object_set_current_time_privileged(time_object, time);
				Time_object_notify_clients_privileged(time_object);
				object_info->next = (struct Time_object_info *)NULL;
				if(time_object_info_list)
				{
					previous = time_object_info_list;
					while(previous->next)
					{
						previous = previous->next;
					}
					previous->next = object_info;
				}
				else
				{
					time_object_info_list = object_info;
				}
				Time_object_set_time_keeper(time_object, this);
				return_code=1;
			}

		}
	}

	return (return_code);
}

int Cmiss_time_keeper::removeTimeObject(struct Time_object *time_object)
{
	int return_code = 0;
	struct Time_object_info *object_info, *previous = NULL;

	if (time_object)
	{
		if (this == Time_object_get_time_keeper(time_object))
		{
			return_code = 0;
			object_info = time_object_info_list;
			while(!return_code && object_info)
			{
				if (object_info->time_object == time_object)
				{
					if (object_info == time_object_info_list)
					{
						time_object_info_list = object_info->next;
					}
					else
					{
						previous->next = object_info->next;
					}
					Time_object_set_time_keeper(object_info->time_object, NULL);
					DEALLOCATE(object_info);
					return_code = 1;
				}
				else
				{
					previous = object_info;
					object_info = object_info->next;
				}
			}
		}
	}
	else
	{
		return_code=0;
	}

	return (return_code);
}

double Cmiss_time_keeper::getTime()
{
	return time;
}

double Cmiss_time_keeper::getMinimum()
{
	return minimum;
}

int Cmiss_time_keeper::setMinimum(double minimum_in)
{
	minimum = minimum_in;

	return 1;
}

double Cmiss_time_keeper::getMaximum()
{
	return maximum;
}

int Cmiss_time_keeper::setMaximum(double maximum_in)
{
	maximum = maximum_in;

	return 1;
}

int Cmiss_time_keeper::setTime(double new_time)
{
	static int recursive_check = 0;
	struct Time_object_info *object_info;

	if(!recursive_check)
	{
		recursive_check = 1;
		time = new_time;
		/* Update the times in all the clients and then get them to call their clients */
		object_info = time_object_info_list;
		while(object_info)
		{
			Time_object_set_current_time_privileged(object_info->time_object, new_time);
			object_info = object_info->next;
		}
		object_info = time_object_info_list;
		while(object_info)
		{
			Time_object_notify_clients_privileged(object_info->time_object);
			object_info = object_info->next;
		}
		recursive_check = 0;
	}
	return 1;
}

int Cmiss_time_keeper::hasTimeObject()
{
	 if (time_object_info_list)
	 {
		 return 1;
	 }

	return 0;
}

struct Time_object_info *Cmiss_time_keeper::getObjectInfo()
{
	return time_object_info_list;
}

void Cmiss_time_keeper::setTimeQuiet(double new_time)
{
	time = new_time;
}

int DESTROY(Cmiss_time_keeper)(struct Cmiss_time_keeper **time_keeper_address)
{
	int return_code = 0;

	if (time_keeper_address && (*time_keeper_address))
	{
		delete *time_keeper_address;
		*time_keeper_address = NULL;
		return_code = 1;
	}

	return (return_code);
}

DECLARE_OBJECT_FUNCTIONS(Cmiss_time_keeper)

Cmiss_time_notifier_id Cmiss_time_keeper_create_notifier_regular(
	Cmiss_time_keeper_id time_keeper, double update_frequency, double time_offset)
{
	Cmiss_time_notifier_id time_notifier = NULL;
	if (time_keeper)
	{
		time_notifier = Time_object_create_regular(update_frequency, time_offset);
		if (time_notifier)
		{
			if (!time_keeper->addTimeObject(time_notifier))
			{
				Cmiss_time_notifier_destroy(&time_notifier);
			}
		}
	}
	return time_notifier;
}

Cmiss_time_keeper_id Cmiss_time_keeper_access(Cmiss_time_keeper_id time_keeper)
{
	if (time_keeper)
	{
		return time_keeper->access();
	}
	return NULL;
}

int Cmiss_time_keeper_destroy(Cmiss_time_keeper_id *time_keeper_address)
{
	return (DEACCESS(Cmiss_time_keeper)(time_keeper_address));
}

double Cmiss_time_keeper_get_attribute_real(Cmiss_time_keeper_id time_keeper,
	enum Cmiss_time_keeper_attribute attribute)
{
	double value = 0.0;
	if (time_keeper)
	{
		switch (attribute)
		{
			case CMISS_TIME_KEEPER_ATTRIBUTE_TIME:
			{
				value = time_keeper->getTime();
			} break;
			case CMISS_TIME_KEEPER_ATTRIBUTE_MINIMUM_TIME:
			{
				value = time_keeper->getMinimum();
			}	break;
			case CMISS_TIME_KEEPER_ATTRIBUTE_MAXIMUM_TIME:
			{
				value = time_keeper->getMaximum();
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_time_keeper_get_attribute_real.  Invalid attribute");
			} break;
		}
	}
	return ((double)value);
}

int Cmiss_time_keeper_set_attribute_real(Cmiss_time_keeper_id time_keeper,
	enum Cmiss_time_keeper_attribute attribute, double value)
{
	int return_code = 0;
	if (time_keeper)
	{
		return_code = 1;
		switch (attribute)
		{
			case CMISS_TIME_KEEPER_ATTRIBUTE_TIME:
			{
				return_code = time_keeper->setTime(value);
			} break;
			case CMISS_TIME_KEEPER_ATTRIBUTE_MINIMUM_TIME:
			{
				return_code = time_keeper->setMinimum(value);
			}	break;
			case CMISS_TIME_KEEPER_ATTRIBUTE_MAXIMUM_TIME:
			{
				return_code = time_keeper->setMaximum(value);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_time_keeper_set_attribute_real.  Invalid attribute");
				return_code = 0;
			} break;
		}
	}
	return return_code;
}

class Cmiss_time_keeper_attribute_conversion
{
public:
	static const char *to_string(enum Cmiss_time_keeper_attribute attribute)
	{
		const char *enum_string = 0;
		switch (attribute)
		{
			case CMISS_TIME_KEEPER_ATTRIBUTE_TIME:
				enum_string = "TIME";
				break;
			case CMISS_TIME_KEEPER_ATTRIBUTE_MINIMUM_TIME:
				enum_string = "MINIMUM_TIME";
				break;
			case CMISS_TIME_KEEPER_ATTRIBUTE_MAXIMUM_TIME:
				enum_string = "MAXIMUM_TIME";
				break;
			default:
				break;
		}
		return enum_string;
	}
};

enum Cmiss_time_keeper_attribute Cmiss_time_keeper_attribute_enum_from_string(
	const char *string)
{
	return string_to_enum<enum Cmiss_time_keeper_attribute,
		Cmiss_time_keeper_attribute_conversion>(string);
}

char *Cmiss_time_keeper_attribute_enum_to_string(enum Cmiss_time_keeper_attribute attribute)
{
	const char *attribute_string = Cmiss_time_keeper_attribute_conversion::to_string(attribute);
	return (attribute_string ? duplicate_string(attribute_string) : 0);
}

int Cmiss_time_keeper_add_time_notifier(Cmiss_time_keeper_id time_keeper,
	Cmiss_time_notifier_id time_notifier)
{
	int return_code = 0;
	if (time_keeper && time_notifier)
	{
		return_code = time_keeper->addTimeObject(time_notifier);
	}
	return return_code;
}

int Cmiss_time_keeper_remove_time_notifier(Cmiss_time_keeper_id time_keeper,
	Cmiss_time_notifier_id time_notifier)
{
	int return_code = 0;
	if (time_keeper && time_notifier)
	{
		return_code = time_keeper->removeTimeObject(time_notifier);
	}
	return return_code;
}
