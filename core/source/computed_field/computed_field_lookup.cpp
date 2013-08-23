/*******************************************************************************
FILE : computed_field_lookup.c

LAST MODIFIED : 25 July 2007

Defines fields for looking up values at given locations.
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
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_finite_element.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_time.h"
#include "region/cmiss_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"
#include "computed_field/computed_field_lookup.h"
#include "graphics/quaternion.hpp"

class Computed_field_lookup_package : public Computed_field_type_package
{
public:
	struct Cmiss_region *root_region;
};

namespace {

void Computed_field_nodal_lookup_FE_region_change(struct FE_region *fe_region,
	struct FE_region_changes *changes, void *computed_field_void);
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the node we are looking at changes generate a computed field change message.
==============================================================================*/

const char computed_field_nodal_lookup_type_string[] = "nodal_lookup";

class Computed_field_nodal_lookup : public Computed_field_core
{
public:
	FE_node *lookup_node;

	Computed_field_nodal_lookup(FE_node *lookup_node) :
		Computed_field_core(),
		lookup_node(ACCESS(FE_node)(lookup_node))
	{
	}

	virtual bool attach_to_field(Computed_field *parent)
	{
		if (Computed_field_core::attach_to_field(parent))
		{
			if (Cmiss_region_get_FE_region(Computed_field_get_region(parent->source_fields[0])) ==
				FE_node_get_FE_region(lookup_node))
			{
				FE_region *fe_region = FE_node_get_FE_region(lookup_node);
				if (!FE_region_contains_FE_node(fe_region, lookup_node))
					fe_region = FE_region_get_data_FE_region(fe_region);
				if (FE_region_add_callback(fe_region,
					Computed_field_nodal_lookup_FE_region_change,
					(void *)parent))
				{
					return true;
				}
			}
		}
		return false;
	}

	~Computed_field_nodal_lookup();

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_nodal_lookup_type_string);
	}

	int compare(Computed_field_core* other_field);

	int list();

	char* get_command_string();

	virtual bool is_defined_at_location(Cmiss_field_cache& cache);

	virtual FieldValueCache *createValueCache(Cmiss_field_cache& parentCache)
	{
		RealFieldValueCache *valueCache = new RealFieldValueCache(field->number_of_components);
		valueCache->createExtraCache(parentCache, Computed_field_get_region(field));
		// set node once as doesn't change
		valueCache->getExtraCache()->setNode(lookup_node);
		return valueCache;
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int has_multiple_times();
};

Computed_field_nodal_lookup::~Computed_field_nodal_lookup()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{

	ENTER(Computed_field_nodal_lookup::~Computed_field_nodal_lookup);
	if (field)
	{
		FE_region *fe_region = FE_node_get_FE_region(lookup_node);
		if (!FE_region_contains_FE_node(fe_region, lookup_node))
			fe_region = FE_region_get_data_FE_region(fe_region);
		FE_region_remove_callback(fe_region,
			Computed_field_nodal_lookup_FE_region_change, (void *)field);
		DEACCESS(FE_node)(&lookup_node);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_nodal_lookup::~Computed_field_nodal_lookup.  Invalid argument(s)");
	}
	LEAVE;

} /* Computed_field_nodal_lookup::~Computed_field_nodal_lookup */

Computed_field_core* Computed_field_nodal_lookup::copy()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_nodal_lookup* core =
		new Computed_field_nodal_lookup(lookup_node);

	return (core);
} /* Computed_field_compose::copy */

int Computed_field_nodal_lookup::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Compare the type specific data.
==============================================================================*/
{
	Computed_field_nodal_lookup* other;
	int return_code;

	ENTER(Computed_field_nodal_lookup::compare);
	if (field && (other = dynamic_cast<Computed_field_nodal_lookup*>(other_core)))
	{
		if (lookup_node == other->lookup_node)
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_nodal_lookup::compare */

bool Computed_field_nodal_lookup::is_defined_at_location(Cmiss_field_cache& cache)
{
	FieldValueCache &inValueCache = *(field->getValueCache(cache));
	Cmiss_field_cache& extraCache = *(inValueCache.getExtraCache());
	extraCache.setTime(cache.getTime());
	return getSourceField(0)->core->is_defined_at_location(extraCache);
}

int Computed_field_nodal_lookup::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
	Cmiss_field_cache& extraCache = *(valueCache.getExtraCache());
	extraCache.setTime(cache.getTime());
	RealFieldValueCache *sourceValueCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(extraCache));
	if (sourceValueCache)
	{
		valueCache.copyValuesZeroDerivatives(*sourceValueCache);
		return 1;
	}
	return 0;
}

int Computed_field_nodal_lookup::list()
{
	int return_code;

	ENTER(List_Computed_field_time_nodal_lookup);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    node : %d\n",
			get_FE_node_identifier(lookup_node));
		FE_region *fe_region = FE_node_get_FE_region(lookup_node);
		if (FE_region_contains_FE_node(fe_region, lookup_node))
		{
			display_message(INFORMATION_MESSAGE, "    nodeset: nodes\n");
		}
		else
		{
			display_message(INFORMATION_MESSAGE, "    nodeset: datapoints\n");
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_nodal_lookup.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_nodal_lookup */

char *Computed_field_nodal_lookup::get_command_string()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field.
==============================================================================*/
{
	char *command_string, *field_name, node_id[10];
	int error,node_number;

	ENTER(Computed_field_time_nodal_lookup::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_nodal_lookup_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		FE_region *fe_region = FE_node_get_FE_region(lookup_node);
		if (FE_region_contains_FE_node(fe_region, lookup_node))
		{
			append_string(&command_string, " nodeset nodes ", &error);
		}
		else
		{
			append_string(&command_string, " nodeset datapoints ", &error);
		}
		append_string(&command_string, " node ", &error);
		node_number = get_FE_node_identifier(lookup_node);
		sprintf(node_id,"%d",node_number);
		append_string(&command_string, " ", &error);
		append_string(&command_string, node_id, &error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_nodal_lookup::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_nodal_lookup::get_command_string */

int Computed_field_nodal_lookup::has_multiple_times ()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Always has multiple times.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_nodal_lookup::has_multiple_times);
	if (field)
	{
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_nodal_lookup::has_multiple_times.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_time_value::has_multiple_times */

void Computed_field_nodal_lookup_FE_region_change(struct FE_region *fe_region,
	struct FE_region_changes *changes, void *computed_field_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the node we are looking at changes generate a computed field change message.
==============================================================================*/
{
	Computed_field *field;
	Computed_field_nodal_lookup *core;
	int node_change;

	ENTER(Computed_field_FE_region_change);
	USE_PARAMETER(fe_region);
	if (changes && (field = (struct Computed_field *)computed_field_void) &&
		(core = dynamic_cast<Computed_field_nodal_lookup*>(field->core)))
	{
		/* I'm not sure if we could also check if we depend on an FE_field change
			and so reduce the total number of changes? */
		if (field->manager && CHANGE_LOG_QUERY(FE_node)(changes->fe_node_changes,
				core->lookup_node, &node_change))
		{
			if (node_change | CHANGE_LOG_OBJECT_CHANGED(FE_node))
			{
				Computed_field_dependency_changed(field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_FE_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Computed_field_FE_region_change */

} //namespace

struct Computed_field *Computed_field_create_nodal_lookup(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field, struct FE_node *lookup_node)
{
	Computed_field *field = NULL;
	if (source_field && source_field->isNumerical() && lookup_node &&
		(FE_node_get_FE_region(lookup_node) == Cmiss_region_get_FE_region(Cmiss_field_module_get_region_internal(field_module))))
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_nodal_lookup(lookup_node));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_nodal_lookup.  Invalid argument(s)");
	}

	return (field);
}

int Computed_field_get_type_nodal_lookup(struct Computed_field *field,
  struct Computed_field **source_field, struct FE_node **lookup_node)
{
	Computed_field_nodal_lookup* core;
	int return_code = 0;

	ENTER(Computed_field_get_type_nodal_lookup);
	if (field && (NULL != (core =
		dynamic_cast<Computed_field_nodal_lookup*>(field->core))) &&
		source_field && lookup_node)
	{
		*source_field = field->source_fields[0];
		*lookup_node = core->lookup_node;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_nodal_lookup.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_nodal_lookup */

namespace {

void Computed_field_quaternion_SLERP_FE_region_change(struct FE_region *fe_region,
	 struct FE_region_changes *changes, void *computed_field_void);
/*******************************************************************************
LAST MODIFIED : 10 Oct 2007

DESCRIPTION :
If the node we are looking at changes generate a computed field change message.
==============================================================================*/

const char computed_field_quaternion_SLERP_type_string[] = "quaternion_SLERP";

class Computed_field_quaternion_SLERP : public Computed_field_core
{
public:
	FE_node *nodal_lookup_node;

	Computed_field_quaternion_SLERP(
		FE_node *nodal_lookup_node) :
		Computed_field_core(),
		nodal_lookup_node(ACCESS(FE_node)(nodal_lookup_node))
	{
	};

	virtual bool attach_to_field(Computed_field *parent)
	{
		if (Computed_field_core::attach_to_field(parent))
		{
			FE_region *fe_region = FE_node_get_FE_region(nodal_lookup_node);
			if (!FE_region_contains_FE_node(fe_region, nodal_lookup_node))
				fe_region = FE_region_get_data_FE_region(fe_region);
			if (FE_region_add_callback(fe_region,
				Computed_field_quaternion_SLERP_FE_region_change,
				(void *)parent))
			{
				return true;
			}
		}
		return false;
	}

	~Computed_field_quaternion_SLERP();

private:

	 Computed_field_core *copy();

	 const char *get_type_string()
	 {
			return(computed_field_quaternion_SLERP_type_string);
	 }

	int compare(Computed_field_core* other_field);

	virtual bool is_defined_at_location(Cmiss_field_cache& cache);

	virtual FieldValueCache *createValueCache(Cmiss_field_cache& parentCache)
	{
		RealFieldValueCache *valueCache = new RealFieldValueCache(field->number_of_components);
		valueCache->createExtraCache(parentCache, Computed_field_get_region(field));
		// set node once as doesn't change
		valueCache->getExtraCache()->setNode(nodal_lookup_node);
		return valueCache;
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	int has_multiple_times();
};

Computed_field_quaternion_SLERP::~Computed_field_quaternion_SLERP()
/*******************************************************************************
LAST MODIFIED : 10 October 2007

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	ENTER(Computed_field_quaternion_SLERP::~Computed_field_quaternion_SLERP);
	if (field)
	{
		FE_region *fe_region = FE_node_get_FE_region(nodal_lookup_node);
		if (!FE_region_contains_FE_node(fe_region, nodal_lookup_node))
			fe_region = FE_region_get_data_FE_region(fe_region);
		FE_region_remove_callback(fe_region,
			Computed_field_quaternion_SLERP_FE_region_change,
			(void *)field);
		if (nodal_lookup_node)
		{
			DEACCESS(FE_node)(&(nodal_lookup_node));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_quaternion_SLERP::~Computed_field_quaternion_SLERP.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* Computed_field_quaternionSLERP::~Computed_field_quaternion_SLERP */

Computed_field_core* Computed_field_quaternion_SLERP::copy()
/*******************************************************************************
LAST MODIFIED : 18 October 2007

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_quaternion_SLERP* core = new Computed_field_quaternion_SLERP(
		nodal_lookup_node);

	return (core);
} /* Computed_field_quaternion_SLERP::copy */

int Computed_field_quaternion_SLERP::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 18 October 2007

DESCRIPTION :
Compare the type specific data.
==============================================================================*/
{
	Computed_field_quaternion_SLERP* other;
	int return_code;

	ENTER(Computed_field_quaternion_SLERP::compare);
	if (field && (other = dynamic_cast<Computed_field_quaternion_SLERP*>(other_core)))
	{
		if (nodal_lookup_node == other->nodal_lookup_node)
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_quaternion_SLERP::compare */

bool Computed_field_quaternion_SLERP::is_defined_at_location(Cmiss_field_cache& cache)
{
	FieldValueCache &inValueCache = *(field->getValueCache(cache));
	Cmiss_field_cache& extraCache = *(inValueCache.getExtraCache());
	extraCache.setTime(cache.getTime());
	return getSourceField(0)->core->is_defined_at_location(extraCache);
}

int Computed_field_quaternion_SLERP::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
	Cmiss_field_cache& extraCache = *(valueCache.getExtraCache());
	FE_value time = cache.getTime();
	//t is the normalised time scaled from 0 to 1
	FE_time_sequence *time_sequence = Computed_field_get_FE_node_field_FE_time_sequence(
		getSourceField(0), nodal_lookup_node);
	if (time_sequence)
	{
		FE_value xi, lower_time, upper_time;
		int time_index_one, time_index_two;
		FE_time_sequence_get_interpolation_for_time(
			time_sequence, time, &time_index_one,
			&time_index_two, &xi);
		FE_time_sequence_get_time_for_index(
			time_sequence, time_index_one, &lower_time);
		FE_time_sequence_get_time_for_index(
			time_sequence, time_index_two, &upper_time);
		FE_value normalised_t = xi;
		// get the starting quaternion
		extraCache.setTime(lower_time);
		RealFieldValueCache *sourceValueCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(extraCache));
		FE_value old_w = sourceValueCache->values[0];
		FE_value old_x = sourceValueCache->values[1];
		FE_value old_y = sourceValueCache->values[2];
		FE_value old_z = sourceValueCache->values[3];
		// get the last quaternion
		extraCache.setTime(upper_time);
		getSourceField(0)->evaluate(extraCache);
		FE_value w = sourceValueCache->values[0];
		FE_value x = sourceValueCache->values[1];
		FE_value y = sourceValueCache->values[2];
		FE_value z = sourceValueCache->values[3];

		Quaternion from(old_w, old_x, old_y, old_z);
		Quaternion to(w, x, y, z);
		Quaternion current;
		from.normalise();
		to.normalise();
		current.interpolated_with_SLERP(from, to, normalised_t);
		double quaternion_component[4];
		current.get(quaternion_component);
		for (int i = 0; i < 4; ++i)
		{
			valueCache.values[i] = quaternion_component[i];
		}
		return 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Computed_field_quaternion::evaluate.  time sequence is missing.");
	}
	return 0;
}

int Computed_field_quaternion_SLERP::list()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_quaternion_SLERP);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    node : %d\n",
			get_FE_node_identifier(nodal_lookup_node));
		FE_region *fe_region = FE_node_get_FE_region(nodal_lookup_node);
		if (FE_region_contains_FE_node(fe_region, nodal_lookup_node))
		{
			display_message(INFORMATION_MESSAGE, "    nodeset: nodes	\n");
		}
		else
		{
			display_message(INFORMATION_MESSAGE, "    nodeset: datapoints \n");
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_quaternion_SLERP.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_quaternion_SLERP */

char *Computed_field_quaternion_SLERP::get_command_string()
/*******************************************************************************
LAST MODIFIED : 5 October 2007

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, node_id[10];
	int error, node_number;

	ENTER(Computed_field_quaternion_SLERP::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_quaternion_SLERP_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			 make_valid_token(&field_name);
			 append_string(&command_string, field_name, &error);
			 DEALLOCATE(field_name);
		}
		FE_region *fe_region = FE_node_get_FE_region(nodal_lookup_node);
		if (FE_region_contains_FE_node(fe_region, nodal_lookup_node))
		{
			append_string(&command_string, " nodeset nodes ", &error);
		}
		else
		{
			append_string(&command_string, " nodeset datapoints ", &error);
		}
		append_string(&command_string, " node ", &error);
		node_number = get_FE_node_identifier(nodal_lookup_node);
		sprintf(node_id,"%d",node_number);
		append_string(&command_string, " ", &error);
		append_string(&command_string, node_id, &error);
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"Computed_field_quaternion_SLERP::get_command_string.  "
				"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_quaternion_SLERP::get_command_string */

int Computed_field_quaternion_SLERP::has_multiple_times()
/*******************************************************************************
LAST MODIFIED : 10 Oct 2007

DESCRIPTION :
Always has multiple times.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_quaternion_SLERP::has_multiple_times);
	if (field)
	{
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			 "Computed_field_quaternion_SLERP::has_multiple_times.  "
			 "Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_quaternion_SLERP::has_multiple_times */

void Computed_field_quaternion_SLERP_FE_region_change(struct FE_region *fe_region,
	 struct FE_region_changes *changes, void *computed_field_void)
/*******************************************************************************
LAST MODIFIED : 10 Oct 2007

DESCRIPTION :
If the node we are looking at changes generate a computed field change message.
==============================================================================*/
{
	Computed_field *field;
	Computed_field_quaternion_SLERP *core;
	int node_change;

	ENTER(Computed_field_FE_region_change);
	USE_PARAMETER(fe_region);
	if (changes && (field = (struct Computed_field *)computed_field_void) &&
		(core = dynamic_cast<Computed_field_quaternion_SLERP*>(field->core)))
	{
		/* I'm not sure if we could also check if we depend on an FE_field change
			and so reduce the total number of changes? */
		if (field->manager && CHANGE_LOG_QUERY(FE_node)(changes->fe_node_changes,
				core->nodal_lookup_node, &node_change))
		{
			if (node_change | CHANGE_LOG_OBJECT_CHANGED(FE_node))
			{
				Computed_field_dependency_changed(field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_FE_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Computed_field_FE_region_change */

} //namespace

/***************************************************************************//**
 * Create field that perfoms a quaternion SLERP interpolation where time
 * variation of quaternion parameters are held in a node.
 * <source_field> must have 4 components.
 */
struct Computed_field *Computed_field_create_quaternion_SLERP(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field,
	Cmiss_node_id quaternion_SLERP_node)
{
	Computed_field *field = NULL;
	if (source_field && (4 == source_field->number_of_components) &&
		quaternion_SLERP_node && (FE_node_get_FE_region(quaternion_SLERP_node) ==
			Cmiss_region_get_FE_region(Cmiss_field_module_get_region_internal(field_module))))
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_quaternion_SLERP(quaternion_SLERP_node));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_quaternion_SLERP.  Invalid argument(s)");
	}

	return (field);
}

int Computed_field_get_type_quaternion_SLERP(struct Computed_field *field,
	 struct Computed_field **quaternion_SLERP_field,
	 struct FE_node **lookup_node)
/*******************************************************************************
LAST MODIFIED : 10 October 2007

DESCRIPTION :
If the field is of type COMPUTED_FIELD_QUATERNION_SLERP, the function returns the source
<quaternion_SLERP_field>, <quaternion_SLERP_region>, and the <quaternion_SLERP_node_identifier>.
Note that nothing returned has been ACCESSed.
==============================================================================*/
{
	 Computed_field_quaternion_SLERP* core;
	 int return_code;

	ENTER(Computed_field_get_type_quatenions_SLERP);
	if (field && (core = dynamic_cast<Computed_field_quaternion_SLERP*>(field->core)))
	{
		 *quaternion_SLERP_field = field->source_fields[0];
		 *lookup_node = core->nodal_lookup_node;
		 return_code = 1;
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"Computed_field_get_type_nodal_lookup.  Invalid argument(s)");
		 return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_quaternion_SLERP */

