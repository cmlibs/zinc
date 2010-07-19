/*****************************************************************************//**
 * FILE : computed_field_sub_group.cpp
 * 
 * Implements a cmiss field which is an group for another field, commonly from a
 * different region to make it available locally.
 *
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

#if !defined (COMPUTED_FIELD_SUB_GROUP_TEMPLATE_HPP)
#define COMPUTED_FIELD_SUB_GROUP_TEMPLATE_HPP
extern "C" {
#include <stdlib.h>
#include "computed_field/computed_field.h"
}
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
#include "user_interface/message.h"
}
#include <map>

namespace {

	char computed_field_sub_group_object_type_string[] = "sub_group_object";

	template <typename T>
	class Computed_field_sub_group_object : public Computed_field_core
	{
	public:

		Computed_field_sub_group_object() : Computed_field_core(), object_map()
		{
		}

		~Computed_field_sub_group_object()
		{
		}

		inline int add_object(int identifier, T object)
		{
			object_map.insert(std::make_pair(identifier,object));
			return 1;
		};

		inline int remove_object(int identifier, T object)
		{

			if (object_map.find(identifier) != object_map.end())
			{
				object_map.erase(identifier);
			}
			return 1;
		};

		inline int clear()
		{
			object_map.clear();
			return 1;
		};

		int get_object_selected(int identifier,T object)
		{
			int return_code = 0;

			if (object_map.find(identifier) != object_map.end() &&
				object_map.find(identifier)->second == object)
			{
				return_code = 1;
			}
			else
			{
				return_code = 0;
			}

			return (return_code);
		};

		int isIdentifierInList(int identifier)
		{
			return (!(object_map.empty()) && (object_map.find(identifier) != object_map.end()));
		}

	private:

		std::map<int, T> object_map;

		Computed_field_core* copy()
		{
			Computed_field_sub_group_object *core = new Computed_field_sub_group_object();
			core->object_map = this->object_map;
			return (core);
		};

		char* get_type_string()
		{
			return (computed_field_sub_group_object_type_string);
		}

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

	};

}

template <typename ObjectType, typename FieldType>
Computed_field_sub_group_object<ObjectType> *Computed_field_sub_group_object_core_cast(
	FieldType object_group_field)
{
	return (static_cast<Computed_field_sub_group_object<ObjectType>*>(
		reinterpret_cast<Computed_field*>(object_group_field)->core));
}
#endif /* COMPUTED_FIELD_SUB_GROUP_TEMPLATE_HPP */

