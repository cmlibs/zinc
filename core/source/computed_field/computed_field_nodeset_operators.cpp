/***************************************************************************//**
 * FILE : computed_field_nodeset_operators.cpp
 *
 * Implementation of field operators that act on a nodeset.
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
#include <cmath>
#include <iostream>
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_nodeset_operators.hpp"
#include "computed_field/field_module.hpp"
#include "mesh/cmiss_node_private.hpp"
#include "zinc/fieldnodesetoperators.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_set.h"
#include "region/cmiss_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"
#include "finite_element/finite_element_region.h"
using namespace std;

class Computed_field_nodeset_operators_package : public Computed_field_type_package
{
};

namespace {

const char computed_field_nodeset_operator_type_string[] = "nodeset_operator";

class Computed_field_nodeset_operator : public Computed_field_core
{
protected:
	cmzn_nodeset_id nodeset;

public:
	Computed_field_nodeset_operator(cmzn_nodeset_id nodeset_in) :
		Computed_field_core(),
		nodeset(cmzn_nodeset_access(nodeset_in))
	{
	}

	virtual ~Computed_field_nodeset_operator()
	{
		cmzn_nodeset_destroy(&nodeset);
	}

	virtual void inherit_source_field_attributes()
	{
		if (field)
		{
			Computed_field_set_coordinate_system_from_sources(field);
		}
	}

	const char *get_type_string()
	{
		return(computed_field_nodeset_operator_type_string);
	}

	cmzn_nodeset_id get_nodeset()
	{
		return nodeset;
	}

	virtual FieldValueCache *createValueCache(cmzn_field_cache& parentCache)
	{
		RealFieldValueCache *valueCache = new RealFieldValueCache(field->number_of_components);
		valueCache->createExtraCache(parentCache, Computed_field_get_region(field));
		return valueCache;
	}

	virtual bool is_defined_at_location(cmzn_field_cache& cache);

	int list();

	char* get_command_string();
};

bool Computed_field_nodeset_operator::is_defined_at_location(cmzn_field_cache& cache)
{
	// Checks if source field is defined at a node in nodeset
	FieldValueCache &inValueCache = *(field->getValueCache(cache));
	cmzn_field_cache& extraCache = *(inValueCache.getExtraCache());
	extraCache.setTime(cache.getTime());
	int return_code = 0;
	cmzn_node_iterator_id iterator = cmzn_nodeset_create_node_iterator(nodeset);
	cmzn_node_id node = 0;
	while (0 != (node = cmzn_node_iterator_next_non_access(iterator)))
	{
		extraCache.setNode(node);
		if (getSourceField(0)->core->is_defined_at_location(extraCache))
		{
			return_code = 1;
			break;
		}
	}
	cmzn_node_iterator_destroy(&iterator);
	return return_code == 1;
}

/** Lists a description of the nodeset_operator arguments */
int Computed_field_nodeset_operator::list()
{
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    field : %s\n",
			field->source_fields[0]->name);
		char *nodeset_name = cmzn_nodeset_get_name(nodeset);
		display_message(INFORMATION_MESSAGE,"    nodeset : %s\n", nodeset_name);
		DEALLOCATE(nodeset_name);
		return 1;
	}
	return 0;
}

/** Returns allocated command string for reproducing field. Includes type. */
char *Computed_field_nodeset_operator::get_command_string()
{
	char *command_string = 0;
	if (field)
	{
		int error = 0;
		append_string(&command_string, get_type_string(), &error);
		append_string(&command_string, " field ", &error);
		append_string(&command_string, field->source_fields[0]->name, &error);
		char *nodeset_name = cmzn_nodeset_get_name(nodeset);
		append_string(&command_string, " nodeset ", &error);
		make_valid_token(&nodeset_name);
		append_string(&command_string, nodeset_name, &error);
		DEALLOCATE(nodeset_name);
	}
	return (command_string);
}


const char computed_field_nodeset_sum_type_string[] = "nodeset_sum";

class Computed_field_nodeset_sum : public Computed_field_nodeset_operator
{
public:
	Computed_field_nodeset_sum(cmzn_nodeset_id nodeset_in) :
		Computed_field_nodeset_operator(nodeset_in)
	{
	}

	Computed_field_core *copy()
	{
		return new Computed_field_nodeset_sum(nodeset);
	}

	const char *get_type_string()
	{
		return (computed_field_nodeset_sum_type_string);
	}

	int compare(Computed_field_core* other_core)
	{
		Computed_field_nodeset_sum *other =
			dynamic_cast<Computed_field_nodeset_sum*>(other_core);
		if (other)
			return cmzn_nodeset_match(nodeset, other->get_nodeset());
		return 0;
	}

	int evaluate(cmzn_field_cache& cache, FieldValueCache& inValueCache)
	{
		evaluate_sum(cache, inValueCache);
		return 1;
	}

protected:
	/** @return  number_of_terms summed. 0 is not an error for nodeset_sum, but is for nodeset_mean */
	int evaluate_sum(cmzn_field_cache& cache, FieldValueCache& inValueCache);
};

int Computed_field_nodeset_sum::evaluate_sum(cmzn_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	cmzn_field_cache& extraCache = *(inValueCache.getExtraCache());
	extraCache.setTime(cache.getTime());
	int number_of_terms = 0;
	const int number_of_components = field->number_of_components;
	FE_value *values = valueCache.values;
	cmzn_field_id sourceField = getSourceField(0);
	int i;
	for (i = 0; i < number_of_components; i++)
	{
		values[i] = 0;
	}
	cmzn_node_iterator_id iterator = cmzn_nodeset_create_node_iterator(nodeset);
	cmzn_node_id node = 0;
	while (0 != (node = cmzn_node_iterator_next_non_access(iterator)))
	{
		extraCache.setNode(node);
		RealFieldValueCache* sourceValueCache = static_cast<RealFieldValueCache*>(sourceField->evaluate(extraCache));
		if (sourceValueCache)
		{
			for (i = 0 ; i < number_of_components ; i++)
			{
				values[i] += sourceValueCache->values[i];
			}
			++number_of_terms;
		}
	}
	cmzn_node_iterator_destroy(&iterator);
	valueCache.derivatives_valid = 0;
	return number_of_terms;
}

const char computed_field_nodeset_mean_type_string[] = "nodeset_mean";

class Computed_field_nodeset_mean : public Computed_field_nodeset_sum
{
public:
	Computed_field_nodeset_mean(cmzn_nodeset_id nodeset_in) :
		Computed_field_nodeset_sum(nodeset_in)
	{
	}

	Computed_field_core *copy()
	{
		return new Computed_field_nodeset_mean(nodeset);
	}

	const char *get_type_string()
	{
		return (computed_field_nodeset_mean_type_string);
	}

	int compare(Computed_field_core* other_core)
	{
		Computed_field_nodeset_sum *other =
			dynamic_cast<Computed_field_nodeset_mean*>(other_core);
		if (other)
			return cmzn_nodeset_match(nodeset, other->get_nodeset());
		return 0;
	}

	int evaluate(cmzn_field_cache& cache, FieldValueCache& inValueCache);

};

int Computed_field_nodeset_mean::evaluate(cmzn_field_cache& cache, FieldValueCache& inValueCache)
{
	int number_of_terms = evaluate_sum(cache, inValueCache);
	if (number_of_terms > 0)
	{
		RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
		FE_value scaling = 1.0 / (FE_value)number_of_terms;
		for (int i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] *= scaling;
		}
		return 1;
	}
	return 0;
}

const char computed_field_nodeset_sum_squares_type_string[] = "nodeset_sum_squares";

class Computed_field_nodeset_sum_squares : public Computed_field_nodeset_operator
{
public:
	Computed_field_nodeset_sum_squares(cmzn_nodeset_id nodeset_in) :
		Computed_field_nodeset_operator(nodeset_in)
	{
	}

	Computed_field_core *copy()
	{
		return new Computed_field_nodeset_sum_squares(nodeset);
	}

	const char *get_type_string()
	{
		return (computed_field_nodeset_sum_squares_type_string);
	}

	int compare(Computed_field_core* other_core)
	{
		Computed_field_nodeset_sum_squares *other =
			dynamic_cast<Computed_field_nodeset_sum_squares*>(other_core);
		if (other)
			return cmzn_nodeset_match(nodeset, other->get_nodeset());
		return 0;
	}

	virtual int supports_sum_square_terms() const
	{
		return 1;
	}

	virtual int get_number_of_sum_square_terms(cmzn_field_cache& cache) const;

	int evaluate_sum_square_terms(cmzn_field_cache& cache, RealFieldValueCache& valueCache,
		int number_of_values, FE_value *values);

	int evaluate(cmzn_field_cache& cache, FieldValueCache& inValueCache)
	{
		evaluate_sum_squares(cache, inValueCache);
		return 1;
	}

protected:
	/** @return  number_of_terms summed. 0 is not an error for nodeset_sum_squares, but is for nodeset_mean_squares */
	int evaluate_sum_squares(cmzn_field_cache& cache, FieldValueCache& inValueCache);
};

int Computed_field_nodeset_sum_squares::get_number_of_sum_square_terms(
	cmzn_field_cache& cache) const
{
	int number_of_terms = 0;
	cmzn_field_id sourceField = field->source_fields[0];
	cmzn_node_iterator_id iterator = cmzn_nodeset_create_node_iterator(nodeset);
	cmzn_node_id node = 0;
	while (0 != (node = cmzn_node_iterator_next_non_access(iterator)))
	{
		cache.setNode(node);
		if (sourceField->core->is_defined_at_location(cache))
		{
			++number_of_terms;
		}
	}
	cmzn_node_iterator_destroy(&iterator);
	return number_of_terms;
}

int Computed_field_nodeset_sum_squares::evaluate_sum_square_terms(
	cmzn_field_cache& cache, RealFieldValueCache& valueCache, int number_of_values, FE_value *values)
{
	cmzn_field_cache& extraCache = *(valueCache.getExtraCache());
	extraCache.setTime(cache.getTime());
	int return_code = 1;
	int number_of_terms = 0;
	const int number_of_components = field->number_of_components;
	const int max_terms = number_of_values / number_of_components;
	FE_value *value = values;
	cmzn_field_id sourceField = getSourceField(0);
	int i;
	cmzn_node_iterator_id iterator = cmzn_nodeset_create_node_iterator(nodeset);
	cmzn_node_id node = 0;
	while (0 != (node = cmzn_node_iterator_next_non_access(iterator)))
	{
		extraCache.setNode(node);
		RealFieldValueCache* sourceValueCache = static_cast<RealFieldValueCache*>(sourceField->evaluate(extraCache));
		if (sourceValueCache)
		{
			if (number_of_terms >= max_terms)
			{
				return_code = 0;
				break;
			}
			for (i = 0 ; i < number_of_components ; i++)
			{
				*value = sourceValueCache->values[i];
				++value;
			}
			++number_of_terms;
		}
	}
	cmzn_node_iterator_destroy(&iterator);
	if (number_of_terms*number_of_components != number_of_values)
	{
		return_code = 0;
	}
	return return_code;
}

int Computed_field_nodeset_sum_squares::evaluate_sum_squares(cmzn_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	cmzn_field_cache& extraCache = *(inValueCache.getExtraCache());
	extraCache.setTime(cache.getTime());
	int number_of_terms = 0;
	const int number_of_components = field->number_of_components;
	FE_value *values = valueCache.values;
	cmzn_field_id sourceField = getSourceField(0);
	int i;
	for (i = 0; i < number_of_components; i++)
	{
		values[i] = 0;
	}
	cmzn_node_iterator_id iterator = cmzn_nodeset_create_node_iterator(nodeset);
	cmzn_node_id node = 0;
	while (0 != (node = cmzn_node_iterator_next_non_access(iterator)))
	{
		extraCache.setNode(node);
		RealFieldValueCache* sourceValueCache = static_cast<RealFieldValueCache*>(sourceField->evaluate(extraCache));
		if (sourceValueCache)
		{
			for (i = 0 ; i < number_of_components ; i++)
			{
				values[i] += sourceValueCache->values[i]*sourceValueCache->values[i];
			}
			++number_of_terms;
		}
	}
	cmzn_node_iterator_destroy(&iterator);
	valueCache.derivatives_valid = 0;
	return number_of_terms;
}

const char computed_field_nodeset_mean_squares_type_string[] = "nodeset_mean_squares";

class Computed_field_nodeset_mean_squares : public Computed_field_nodeset_sum_squares
{
public:
	Computed_field_nodeset_mean_squares(cmzn_nodeset_id nodeset_in) :
		Computed_field_nodeset_sum_squares(nodeset_in)
	{
	}

	Computed_field_core *copy()
	{
		return new Computed_field_nodeset_mean_squares(nodeset);
	}

	const char *get_type_string()
	{
		return (computed_field_nodeset_mean_squares_type_string);
	}

	int compare(Computed_field_core* other_core)
	{
		Computed_field_nodeset_mean_squares *other =
			dynamic_cast<Computed_field_nodeset_mean_squares*>(other_core);
		if (other)
			return cmzn_nodeset_match(nodeset, other->get_nodeset());
		return 0;
	}

	int evaluate_sum_square_terms(cmzn_field_cache& cache, RealFieldValueCache& valueCache,
		int number_of_values, FE_value *values);

	int evaluate(cmzn_field_cache& cache, FieldValueCache& inValueCache);

};

int Computed_field_nodeset_mean_squares::evaluate_sum_square_terms(
	cmzn_field_cache& cache, RealFieldValueCache& valueCache, int number_of_values, FE_value *values)
{
	int return_code = evaluate_sum_square_terms(cache, valueCache, number_of_values, values);
	if (return_code)
	{
		int number_of_terms = number_of_values / field->number_of_components;
		if (number_of_terms > 0)
		{
			FE_value scaling = 1.0 / sqrt((FE_value)number_of_terms);
			for (int i = 0 ; i < number_of_values ; i++)
			{
				values[i] *= scaling;
			}
		}
		else
		{
			return_code = 0;
		}
	}
	return (return_code);
}

int Computed_field_nodeset_mean_squares::evaluate(cmzn_field_cache& cache, FieldValueCache& inValueCache)
{
	int number_of_terms = evaluate_sum_squares(cache, inValueCache);
	if (number_of_terms > 0)
	{
		RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
		FE_value scaling = 1.0 / (FE_value)number_of_terms;
		for (int i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] *= scaling;
		}
		return 1;
	}
	return 0;
}

const char computed_field_nodeset_minimum_type_string[] = "nodeset_min";

class Computed_field_nodeset_minimum : public Computed_field_nodeset_operator
{
public:
	Computed_field_nodeset_minimum(cmzn_nodeset_id nodeset_in) :
		Computed_field_nodeset_operator(nodeset_in)
	{
	}

	Computed_field_core *copy()
	{
		return new Computed_field_nodeset_minimum(nodeset);
	}

	const char *get_type_string()
	{
		return (computed_field_nodeset_minimum_type_string);
	}

	int compare(Computed_field_core* other_core)
	{
		Computed_field_nodeset_minimum *other =
			dynamic_cast<Computed_field_nodeset_minimum*>(other_core);
		if (other)
			return cmzn_nodeset_match(nodeset, other->get_nodeset());
		return 0;
	}

	int evaluate(cmzn_field_cache& cache, FieldValueCache& inValueCache);

};

int Computed_field_nodeset_minimum::evaluate(cmzn_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	cmzn_field_cache& extraCache = *(inValueCache.getExtraCache());
	extraCache.setTime(cache.getTime());
	cmzn_field_id sourceField = getSourceField(0);

	bool initialise = true;
	cmzn_node_iterator_id iterator = cmzn_nodeset_create_node_iterator(nodeset);
	int node_count = 0;
	cmzn_node_id node = 0;
	while (0 != (node = cmzn_node_iterator_next_non_access(iterator)))
	{
		node_count++;
		extraCache.setNode(node);
		RealFieldValueCache* sourceValueCache = static_cast<RealFieldValueCache*>(sourceField->evaluate(extraCache));
		if (sourceValueCache)
		{
			for (int i = 0 ; i < field->number_of_components ; i++)
			{
				if (initialise)
				{
					valueCache.values[i] = sourceValueCache->values[i];
				}
				else if (sourceValueCache->values[i] < valueCache.values[i])
				{
					valueCache.values[i] = sourceValueCache->values[i];
				}
			}
			if (initialise)
			{
				initialise = false;
			}
		}
	}
	cmzn_node_iterator_destroy(&iterator);

	if (node_count > 0)
	{
		return 1;
	}

	return 0;
}

const char computed_field_nodeset_maximum_type_string[] = "nodeset_max";

class Computed_field_nodeset_maximum : public Computed_field_nodeset_operator
{
public:
	Computed_field_nodeset_maximum(cmzn_nodeset_id nodeset_in) :
		Computed_field_nodeset_operator(nodeset_in)
	{
	}

	Computed_field_core *copy()
	{
		return new Computed_field_nodeset_maximum(nodeset);
	}

	const char *get_type_string()
	{
		return (computed_field_nodeset_maximum_type_string);
	}

	int compare(Computed_field_core* other_core)
	{
		Computed_field_nodeset_maximum *other =
			dynamic_cast<Computed_field_nodeset_maximum*>(other_core);
		if (other)
			return cmzn_nodeset_match(nodeset, other->get_nodeset());
		return 0;
	}

	int evaluate(cmzn_field_cache& cache, FieldValueCache& inValueCache);

};

int Computed_field_nodeset_maximum::evaluate(cmzn_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	cmzn_field_cache& extraCache = *(inValueCache.getExtraCache());
	extraCache.setTime(cache.getTime());
	cmzn_field_id sourceField = getSourceField(0);

	bool initialise = true;
	cmzn_node_iterator_id iterator = cmzn_nodeset_create_node_iterator(nodeset);
	int node_count = 0;
	cmzn_node_id node = 0;
	while (0 != (node = cmzn_node_iterator_next_non_access(iterator)))
	{
		node_count++;
		extraCache.setNode(node);
		RealFieldValueCache* sourceValueCache = static_cast<RealFieldValueCache*>(sourceField->evaluate(extraCache));
		if (sourceValueCache)
		{
			for (int i = 0 ; i < field->number_of_components ; i++)
			{
				if (initialise)
				{
					valueCache.values[i] = sourceValueCache->values[i];
				}
				else if (sourceValueCache->values[i] > valueCache.values[i])
				{
					valueCache.values[i] = sourceValueCache->values[i];
				}
			}
			if (initialise)
			{
				initialise = false;
			}
		}
	}
	cmzn_node_iterator_destroy(&iterator);

	if (node_count > 0)
	{
		return 1;
	}

	return 0;
}

} //namespace

cmzn_field_id cmzn_field_module_create_nodeset_sum(
	cmzn_field_module_id field_module, cmzn_field_id source_field,
	cmzn_nodeset_id nodeset)
{
	cmzn_field_id field = 0;
	if (source_field && source_field->isNumerical() && nodeset &&
		(cmzn_field_module_get_master_region_internal(field_module) ==
			cmzn_nodeset_get_master_region_internal(nodeset)))
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_nodeset_sum(nodeset));
	}
	return field;
}

cmzn_field_id cmzn_field_module_create_nodeset_mean(
	cmzn_field_module_id field_module, cmzn_field_id source_field,
	cmzn_nodeset_id nodeset)
{
	cmzn_field_id field = 0;
	if (source_field && source_field->isNumerical() && nodeset &&
		(cmzn_field_module_get_master_region_internal(field_module) ==
			cmzn_nodeset_get_master_region_internal(nodeset)))
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_nodeset_mean(nodeset));
	}
	return field;
}

cmzn_field_id cmzn_field_module_create_nodeset_sum_squares(
	cmzn_field_module_id field_module, cmzn_field_id source_field,
	cmzn_nodeset_id nodeset)
{
	cmzn_field_id field = 0;
	if (source_field && source_field->isNumerical() && nodeset &&
		(cmzn_field_module_get_master_region_internal(field_module) ==
			cmzn_nodeset_get_master_region_internal(nodeset)))
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_nodeset_sum_squares(nodeset));
	}
	return field;
}

cmzn_field_id cmzn_field_module_create_nodeset_mean_squares(
	cmzn_field_module_id field_module, cmzn_field_id source_field,
	cmzn_nodeset_id nodeset)
{
	cmzn_field_id field = 0;
	if (source_field && source_field->isNumerical() && nodeset &&
		(cmzn_field_module_get_master_region_internal(field_module) ==
			cmzn_nodeset_get_master_region_internal(nodeset)))
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_nodeset_mean_squares(nodeset));
	}
	return field;
}

cmzn_field_id cmzn_field_module_create_nodeset_minimum(
	cmzn_field_module_id field_module, cmzn_field_id source_field,
	cmzn_nodeset_id nodeset)
{
	cmzn_field_id field = 0;
	if (source_field && source_field->isNumerical() && nodeset &&
		(cmzn_field_module_get_master_region_internal(field_module) ==
			cmzn_nodeset_get_master_region_internal(nodeset)))
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_nodeset_minimum(nodeset));
	}
	return field;
}

cmzn_field_id cmzn_field_module_create_nodeset_maximum(
	cmzn_field_module_id field_module, cmzn_field_id source_field,
	cmzn_nodeset_id nodeset)
{
	cmzn_field_id field = 0;
	if (source_field && source_field->isNumerical() && nodeset &&
		(cmzn_field_module_get_master_region_internal(field_module) ==
			cmzn_nodeset_get_master_region_internal(nodeset)))
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_nodeset_maximum(nodeset));
	}
	return field;
}

