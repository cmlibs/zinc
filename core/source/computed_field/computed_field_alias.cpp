/*****************************************************************************//**
 * FILE : computed_field_alias.cpp
 * 
 * Implements a cmiss field which is an alias for another field, commonly from a
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
//-- extern "C" {
#include <stdlib.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_alias.h"
//-- }
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_set.h"
//-- extern "C" {
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
#include "general/message.h"
//-- }

/*
Module types
------------
*/

class Computed_field_alias_package : public Computed_field_type_package
{
public:
	Cmiss_region *root_region;

	Computed_field_alias_package(Cmiss_region *root_region)
	  : root_region(root_region)
	{
		ACCESS(Cmiss_region)(root_region);
	}
	
	~Computed_field_alias_package()
	{
		DEACCESS(Cmiss_region)(&root_region);
	}
};

namespace {

char computed_field_alias_type_string[] = "alias";

class Computed_field_alias : public Computed_field_core
{
public:
	void *other_field_manager_callback_id;

	Computed_field_alias() : Computed_field_core(),
		other_field_manager_callback_id(NULL)
	{
	}

	virtual bool attach_to_field(Computed_field *parent)
	{
		if (Computed_field_core::attach_to_field(parent))
		{
			check_alias_from_other_manager();
			return true;
		}
		return false;
	}

	~Computed_field_alias()
	{
		if (other_field_manager_callback_id)
		{
			if (field && (field->number_of_source_fields > 0) && field->source_fields && original_field())
			{
				if (original_field()->manager)
					MANAGER_DEREGISTER(Computed_field)(other_field_manager_callback_id,
						original_field()->manager);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"~Computed_field_alias.  Computed_field source_fields removed before core. Can't get manager of aliased field to end callbacks.");
			}
		}
	}

private:
	inline Computed_field *original_field(void) { return field->source_fields[0]; }

	static void other_field_manager_change(
		MANAGER_MESSAGE(Computed_field) *message, void *alias_field_core_void);

	void check_alias_from_other_manager(void);

	Computed_field_core* copy()
	{
		Computed_field_alias* core = new Computed_field_alias();
		return (core);
	};

	const char* get_type_string()
	{
		return (computed_field_alias_type_string);
	}

	int compare(Computed_field_core* other_field);

	virtual FieldValueCache *createValueCache(Cmiss_field_cache& parentCache)
	{
		RealFieldValueCache *valueCache = new RealFieldValueCache(field->number_of_components);
		Cmiss_region_id otherRegion = Computed_field_get_region(getSourceField(0));
		if (otherRegion != Computed_field_get_region(field))
		{
			// @TODO: share extraCache with other alias fields in cache referencing otherRegion
			valueCache->createExtraCache(parentCache, otherRegion);
		}
		return valueCache;
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	virtual enum FieldAssignmentResult assign(Cmiss_field_cache& /*cache*/, RealFieldValueCache& /*valueCache*/);

	void field_is_managed(void)
	{
		check_alias_from_other_manager();
	}
};

/***************************************************************************//**
 * Callback for changes in the field manager owning original_field.
 * If this field depends on the change, propagate to this manager as a change to
 * this field.
 */
void Computed_field_alias::other_field_manager_change(
	struct MANAGER_MESSAGE(Computed_field) *message, void *alias_field_core_void)
{
	Computed_field_alias *alias_field_core =
		reinterpret_cast<Computed_field_alias *>(alias_field_core_void);
	Computed_field *field;
	
	if (message && alias_field_core && (field = alias_field_core->field) &&
		(field->number_of_source_fields > 0) && field->source_fields)
	{
		int change = MANAGER_MESSAGE_GET_OBJECT_CHANGE(Computed_field)(message,
			alias_field_core->original_field());
		if (change & MANAGER_CHANGE_RESULT(Computed_field))
		{
			Computed_field_dependency_changed(field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_alias::other_field_manager_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Computed_field_alias::other_field_manager_change */

/***************************************************************************//**
 * If original_field is from a different manager to this field, request
 * manager messages to propagate changes to this manager.
 */
void Computed_field_alias::check_alias_from_other_manager(void)
{
	ENTER(Computed_field_alias::check_alias_from_other_manager);
	if (!other_field_manager_callback_id)
	{
		if (field && (field->number_of_source_fields > 0) && field->source_fields &&
			original_field() && original_field()->manager)
		{
			if (field->manager && (field->manager != original_field()->manager))
			{
				// alias from another region: set up manager callbacks
				other_field_manager_callback_id = MANAGER_REGISTER(Computed_field)(
					other_field_manager_change, (void *)this, original_field()->manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_alias::check_alias_from_other_manager.  Invalid source_fields array.");
		}
	}
	LEAVE;
} /* Computed_field_alias::check_alias_from_other_manager */

/***************************************************************************//**
 * Compare the type specific data.
 */
int Computed_field_alias::compare(Computed_field_core *other_core)
{
	int return_code;

	ENTER(Computed_field_alias::compare);
	if (field && dynamic_cast<Computed_field_alias*>(other_core))
	{
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_alias::compare */

int Computed_field_alias::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	Cmiss_field_cache *extraCache = valueCache.getExtraCache();
	RealFieldValueCache *sourceCache = 0;
	if (extraCache)
	{
		extraCache->setLocation(cache.cloneLocation());
		extraCache->setRequestedDerivatives(cache.getRequestedDerivatives());
		sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(*extraCache));
	}
	else
	{
		sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	}
	if (sourceCache)
	{
		valueCache.copyValues(*sourceCache);
		return 1;
	}
	return 0;
}

/***************************************************************************//**
 * Sets values of the original field at the supplied location. 
 */
enum FieldAssignmentResult Computed_field_alias::assign(Cmiss_field_cache& cache, RealFieldValueCache& valueCache)
{
	Cmiss_field_cache *extraCache = valueCache.getExtraCache();
	RealFieldValueCache *sourceCache = 0;
	if (extraCache)
	{
		extraCache->setLocation(cache.cloneLocation());
		sourceCache = RealFieldValueCache::cast(getSourceField(0)->getValueCache(*extraCache));
	}
	else
	{
		sourceCache = RealFieldValueCache::cast(getSourceField(0)->getValueCache(cache));
	}
	sourceCache->setValues(valueCache.values);
	return getSourceField(0)->assign(extraCache ? *extraCache : cache, *sourceCache);
}

/***************************************************************************//**
 * Writes type-specific details of the field to the console. 
 */
int Computed_field_alias::list()
{
	char *field_name;
	int return_code;
	
	ENTER(List_Computed_field_alias);
	if (field)
	{
		display_message(INFORMATION_MESSAGE, "    Original field : ");
		if (original_field()->manager != field->manager)
		{
			char *path = Cmiss_region_get_path(Computed_field_get_region(original_field()));
			display_message(INFORMATION_MESSAGE, "%s", path);
			DEALLOCATE(path);
		}
		if (GET_NAME(Computed_field)(original_field(), &field_name))
		{
			make_valid_token(&field_name);
			display_message(INFORMATION_MESSAGE, "%s\n", field_name);
			DEALLOCATE(field_name);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_alias.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_alias */

/***************************************************************************//**
 * Returns allocated command string for reproducing this field. Includes type.
 */
char *Computed_field_alias::get_command_string()
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_alias::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string, computed_field_alias_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (original_field()->manager != field->manager)
		{
			char *path = Cmiss_region_get_path(Computed_field_get_region(original_field()));
			append_string(&command_string, path, &error);
			DEALLOCATE(path);
		}
		if (GET_NAME(Computed_field)(original_field(), &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_alias::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_alias::get_command_string */

} //namespace

Computed_field *Cmiss_field_module_create_alias(Cmiss_field_module_id field_module,
	Computed_field *original_field)
{
	Cmiss_field_id field = 0;
	// @TODO Generalise to non-numeric types by adding createValueCache and modifying evaluate methods
	if (original_field && original_field->isNumerical())
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/false, original_field->number_of_components,
			/*number_of_source_fields*/1, &original_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_alias());
	}
	return (field);
}

