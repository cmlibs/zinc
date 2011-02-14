/***************************************************************************//**
 * FILE : computed_field_sub_group.hpp
 * 
 * Implements region sub object groups, e.g. node group, element group.
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

#if !defined (COMPUTED_FIELD_SUB_GROUP_HPP)
#define COMPUTED_FIELD_SUB_GROUP_HPP
extern "C" {
#include <stdlib.h>
#include "api/cmiss_field_sub_group.h"
#include "computed_field/computed_field.h"
}
#include "computed_field/computed_field_group_base.hpp"
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
#include "user_interface/message.h"
}
#include <map>
#include <iterator>

/***************************************************************************//**
 * Change details for simple object groups where a single change status is
 * sufficient.
 */
struct Cmiss_field_sub_group_change_detail : public Cmiss_field_group_base_change_detail
{
private:
	Cmiss_field_group_change_type change;

public:
	Cmiss_field_sub_group_change_detail() :
		change(CMISS_FIELD_GROUP_NO_CHANGE)
	{
	}

	void clear()
	{
		change = CMISS_FIELD_GROUP_NO_CHANGE;
	}

	Cmiss_field_group_change_type getChange() const
	{
		return change;
	}

	Cmiss_field_group_change_type getLocalChange() const
	{
		return change;
	}

	/** Inform object(s) have been added */
	void changeAdd()
	{
		switch (change)
		{
		case CMISS_FIELD_GROUP_NO_CHANGE:
			change = CMISS_FIELD_GROUP_ADD;
			break;
		case CMISS_FIELD_GROUP_CLEAR:
		case CMISS_FIELD_GROUP_REMOVE:
			change = CMISS_FIELD_GROUP_REPLACE;
			break;
		default:
			// do nothing
			break;
		}
	}

	/** Inform object(s) have been removed (clear handled separately) */
	void changeRemove()
	{
		switch (change)
		{
		case CMISS_FIELD_GROUP_NO_CHANGE:
			change = CMISS_FIELD_GROUP_REMOVE;
			break;
		case CMISS_FIELD_GROUP_ADD:
			change = CMISS_FIELD_GROUP_REPLACE;
			break;
		default:
			// do nothing
			break;
		}
	}

	/** Inform group has been cleared, but wasn't before */
	void changeClear()
	{
		change = CMISS_FIELD_GROUP_CLEAR;
	}
};

class Computed_field_sub_group : public Computed_field_group_base
{
public:

	Computed_field_sub_group() :
		Computed_field_group_base()
	{
	}

	const char* get_type_string()
	{
		return ("sub_group_object");
	}
};

namespace {

	template <typename T>
	class Computed_field_sub_group_object : public Computed_field_sub_group
	{
	public:

		Computed_field_sub_group_object() :
			Computed_field_sub_group(),
			object_map()
		{
			object_pos = object_map.begin();
		}

		~Computed_field_sub_group_object()
		{
		}

		inline int add_object(int identifier, T object)
		{
			if (object_map.insert(std::make_pair(identifier,object)).second)
			{
				change_detail.changeAdd();
				update();
				return 1;
			}
			return 0;
		};

		inline int remove_object(int identifier)
		{
			if (object_map.find(identifier) != object_map.end())
			{
				object_map.erase(identifier);
				if (0 == object_map.size())
				{
					change_detail.changeClear();
				}
				else
				{
					change_detail.changeRemove();
				}
				update();
				return 1;
			}
			return 0;
		};

		inline T get_object(int identifier)
		{
			T return_object = NULL;
			if (object_map.find(identifier) != object_map.end())
				return_object = object_map.find(identifier)->second;

			return return_object;
		}

		virtual int clear()
		{
			if (object_map.size())
			{
				object_map.clear();
				change_detail.changeClear();
				update();
			}
			return 1;
		};

		int get_object_selected(int identifier,T object)
		{
			int return_code = 0;
			if (object_map.find(identifier) != object_map.end())
			{
				return_code = 1;
			}
			else
			{
				return_code = 0;
			}

			return (return_code);
		};

		virtual int isEmpty() const
		{
			return object_map.empty();
		}

		int isIdentifierInList(int identifier)
		{
			return (!(object_map.empty()) && (object_map.find(identifier) != object_map.end()));
		}

		T getFirstObject()
		{
			T return_object = NULL;
			object_pos = object_map.begin();
			if (object_pos != object_map.end())
			{
				return_object = object_pos->second;
			}
			return return_object;
		}

		T getNextObject()
		{
			T return_object = NULL;
			if (object_pos != object_map.end())
			{
				object_pos++;
				if (object_pos != object_map.end())
				{
					return_object = object_pos->second;
				}
			}
			return return_object;
		}

		virtual Cmiss_field_change_detail *extract_change_detail()
		{
			if (change_detail.getChange() == CMISS_FIELD_GROUP_NO_CHANGE)
				return NULL;
			Cmiss_field_sub_group_change_detail *prior_change_detail =
				new Cmiss_field_sub_group_change_detail();
			*prior_change_detail = change_detail;
			change_detail.clear();
			return prior_change_detail;
		}

		virtual const Cmiss_field_change_detail *get_change_detail() const
		{
			return &change_detail;
		}

	private:

		std::map<int, T> object_map;
		Cmiss_field_sub_group_change_detail change_detail;
		typename std::map<int, T>::iterator object_pos;

		Computed_field_core* copy()
		{
			Computed_field_sub_group_object *core = new Computed_field_sub_group_object();
			core->object_map = this->object_map;
			return (core);
		};

		int compare(Computed_field_core* other_field)
		{
			int return_code;

			ENTER(Computed_field_sub_group_object::compare);
			if (field && dynamic_cast<Computed_field_sub_group_object<T>*>(other_field))
			{
				return_code = 1;
			}
			else
			{
				return_code = 0;
			}
			LEAVE;

			return (return_code);
		}

		int evaluate_cache_at_location(Field_location* location)
		{
			USE_PARAMETER(location);
			return 1;
		};

		int list()
		{
			return 1;
		};

		char* get_command_string()
		{
			return NULL;
		};

		inline void update()
		{
			Computed_field_changed(field);
		}

	};

}

template <typename ObjectType, typename FieldType>
Computed_field_sub_group_object<ObjectType> *Computed_field_sub_group_object_core_cast(
	FieldType object_group_field)
{
	return (static_cast<Computed_field_sub_group_object<ObjectType>*>(
		reinterpret_cast<Computed_field*>(object_group_field)->core));
}

Cmiss_node_id Cmiss_field_node_group_get_first_node(
	Cmiss_field_node_group_id node_group);

Cmiss_node_id Cmiss_field_node_group_get_next_node(
	Cmiss_field_node_group_id node_group);

Cmiss_element_id Cmiss_field_element_group_get_first_element(
	Cmiss_field_element_group_id element_group);

Cmiss_element_id Cmiss_field_element_group_get_next_element(
	Cmiss_field_element_group_id element_group);

#endif /* COMPUTED_FIELD_SUB_GROUP_HPP */

