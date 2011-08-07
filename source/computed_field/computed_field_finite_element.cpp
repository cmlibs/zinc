/*******************************************************************************
FILE : computed_field_finite_element.cpp

LAST MODIFIED : 24 August 2006

DESCRIPTION :
Implements a number of basic component wise operations on computed fields.
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
extern "C" {
#include <math.h>
#include "api/cmiss_field_module.h"
#include "api/cmiss_field_finite_element.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_find_xi.h"
}
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "computed_field/computed_field_set.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_discretization.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_region_private.h"
#include "finite_element/finite_element_time.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_finite_element.h"
}
#include "mesh/cmiss_element_private.hpp"

#if defined (DEBUG_CODE)
/* SAB This field is useful for debugging when things don't clean up properly
	but has to be used carefully, especially as operations such as caching
	accesses the node or element being considered so you get effects like 
	the first point evaluated in an element having a count one less than 
	all the others */
#define COMPUTED_FIELD_ACCESS_COUNT
#endif /* defined (DEBUG_CODE) */

/*
Module types
------------
*/

class Computed_field_finite_element_package : public Computed_field_type_package
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
}; /* Computed_field_finite_element_package */

namespace {

char computed_field_finite_element_type_string[] = "finite_element";

class Computed_field_finite_element : public Computed_field_core
{
public:
	FE_field* fe_field;

private:
	FE_element_field_values* fe_element_field_values;
	/* need pointer to fe_field_manager so can call MANAGED_OBJECT_NOT_IN_USE in
		 Computed_field_finite_element::not_in_use */

	/* Keep a cache of FE_element_field_values as calculation is expensive */
	LIST(FE_element_field_values) *field_values_cache;

public:
	Computed_field_finite_element(FE_field *fe_field) : Computed_field_core(),
		fe_field(ACCESS(FE_field)(fe_field))
	{
		FE_field_add_wrapper(fe_field);
		fe_element_field_values = (FE_element_field_values*)NULL;
		field_values_cache = (LIST(FE_element_field_values) *)NULL;
	};

	virtual ~Computed_field_finite_element();

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_finite_element_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int clear_cache();

	int is_defined_at_location(Field_location* location);

	int has_multiple_times();

	int has_numerical_components();

	int not_in_use();

	int calculate_FE_element_field_values_for_element(
		int calculate_derivatives, struct FE_element *element,
		FE_value time, struct FE_element *top_level_element);

	int set_values_at_location(Field_location* location, FE_value *values);

	virtual int set_mesh_location_value(Field_location* location, Cmiss_element_id element, FE_value *xi);

	int set_string_at_location(Field_location* location, const char *string_value);

	virtual int make_string_cache(int component_number = -1);

	virtual Cmiss_element_id get_mesh_location_value(FE_value *xi) const;

	int get_native_discretization_in_element(
		struct FE_element *element,int *number_in_xi);

	int find_element_xi(
		FE_value *values, int number_of_values, struct FE_element **element,
		FE_value *xi, int element_dimension, double time,
		struct Cmiss_region *search_region);

	virtual bool is_non_linear() const
	{
		return FE_field_uses_non_linear_basis(fe_field);
	}

	virtual int set_name(const char *name)
	{
		return FE_region_set_FE_field_name(FE_field_get_FE_region(fe_field), fe_field, name);
	};

	virtual int get_attribute_integer(enum Cmiss_field_attribute attribute) const
	{
		if (attribute == CMISS_FIELD_ATTRIBUTE_IS_COORDINATE)
			return (get_FE_field_CM_field_type(fe_field) == CM_COORDINATE_FIELD);
		return 0;
	}

	virtual int set_attribute_integer(enum Cmiss_field_attribute attribute, int value)
	{
		// Note that CM_field_type is an enum with 3 states
		// so can't be COORDINATE and ANATOMICAL at the same time.
		if (attribute == CMISS_FIELD_ATTRIBUTE_IS_COORDINATE)
		{
			CM_field_type cm_field_type = get_FE_field_CM_field_type(fe_field);
			if (value)
			{
				if (cm_field_type != CM_COORDINATE_FIELD)
					set_FE_field_CM_field_type(fe_field, CM_COORDINATE_FIELD);
			}
			else
			{
				if (cm_field_type == CM_COORDINATE_FIELD)
					set_FE_field_CM_field_type(fe_field, CM_GENERAL_FIELD);
			}
			return 1;
		}
		return 0;
	}

	virtual Cmiss_field_value_type get_value_type() const
	{
		enum Value_type fe_value_type = get_FE_field_value_type(fe_field);
		Cmiss_field_value_type value_type = CMISS_FIELD_VALUE_TYPE_INVALID;
		switch (fe_value_type)
		{
			case ELEMENT_XI_VALUE:
				value_type = CMISS_FIELD_VALUE_TYPE_MESH_LOCATION;
				break;
			case STRING_VALUE:
			case URL_VALUE:
				value_type = CMISS_FIELD_VALUE_TYPE_STRING;
				break;
			case DOUBLE_VALUE:
			case FE_VALUE_VALUE:
			case FLT_VALUE:
			case INT_VALUE:
			case SHORT_VALUE:
				value_type = CMISS_FIELD_VALUE_TYPE_REAL;
				break;
			default:
				break;
		}
		return value_type;
	}

};

Computed_field_finite_element::~Computed_field_finite_element()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{

	ENTER(Computed_field_finite_element::~Computed_field_finite_element);
	if (field)
	{
		if (fe_field)
		{
			/* The following logic only removes the FE_field when it is not in
			 * use, which should be ensured in normal use by
			 * MANAGED_OBJECT_NOT_IN_USE(Computed_field).
			 * There are complications due to the merge process used when reading
			 * fields from file which appears to leave some FE_fields temporarily
			 * not in their owning FE_region when this is called.
			 * Also gfx define field commands create and destroy temporary
			 * finite_element field wrappers & we don't want to clean up the
			 * FE_field until the last wrapper is destroyed.
			 */
			int number_of_remaining_wrappers = FE_field_remove_wrapper(fe_field);
			if (0 == number_of_remaining_wrappers)
			{
				struct FE_region *fe_region = FE_field_get_FE_region(fe_field);
				if (fe_region && FE_region_contains_FE_field(fe_region, fe_field) &&
					(!FE_region_is_FE_field_in_use(fe_region, fe_field)))
				{
					if (!FE_region_remove_FE_field(fe_region, fe_field))
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_finite_element::~Computed_field_finite_element.  "
							"Destroying computed field before FE_field.");
					}
				}
			}
			DEACCESS(FE_field)(&(fe_field));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::~Computed_field_finite_element.  "
			"Invalid arguments.");
	}
	LEAVE;

} /* Computed_field_finite_element::~Computed_field_finite_element */

Computed_field_core* Computed_field_finite_element::copy()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_finite_element* core =
		new Computed_field_finite_element(fe_field);

	return (core);
} /* Computed_field_finite_element::copy */

int Computed_field_finite_element::clear_cache()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_finite_element::clear_cache_type_specific);
	if (field)
	{
		if(fe_element_field_values)
		{
			clear_FE_element_field_values(fe_element_field_values);
		}
		if (field_values_cache)
		{
			DESTROY(LIST(FE_element_field_values))(&field_values_cache);
		}
		/* These are owned by the list */
		fe_element_field_values = (FE_element_field_values *)NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::clear_cache_type_specific(.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_finite_element::clear_cache_type_specific( */

int Computed_field_finite_element::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	Computed_field_finite_element* other;
	int return_code;

	ENTER(Computed_field_finite_element::compare);
	if (field && (other = dynamic_cast<Computed_field_finite_element*>(other_core)))
	{
		return_code = (fe_field == other->fe_field);
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_finite_element::compare */

int Computed_field_finite_element::is_defined_at_location(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_finite_element::is_defined_at_location);
	if (field && location && fe_field)
	{
		return_code = 0;

		Field_element_xi_location *element_xi_location;
		Field_node_location *node_location;
		element_xi_location = dynamic_cast<Field_element_xi_location*>(location);
		if (element_xi_location)
		{
			FE_element* element = element_xi_location->get_element();

			return_code = FE_field_is_defined_in_element(fe_field,element);
		}
		else if (NULL != (node_location = dynamic_cast<Field_node_location*>(location)))
		{
			FE_node *node = node_location->get_node();

			return_code = FE_field_is_defined_at_node(fe_field,node);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::is_defined_at_location.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_finite_element::is_defined_at_location */

int Computed_field_finite_element::has_multiple_times()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Check the fe_field
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_finite_element::has_multiple_times);
	if (field)
	{
		return_code=FE_field_has_multiple_times(fe_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::has_multiple_times.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_finite_element::has_multiple_times */

int Computed_field_finite_element::has_numerical_components()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_finite_element::has_numerical_components);
	if (field)
	{
		return_code=Value_type_is_numeric_simple(
			get_FE_field_value_type(fe_field));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::has_numerical_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_finite_element::has_numerical_components */

int Computed_field_finite_element::not_in_use()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
The FE_field must also not be in use.
==============================================================================*/
{
	int return_code;
	struct FE_region *fe_region;

	ENTER(Computed_field_finite_element::not_in_use);
	if (field)
	{
		/* check the fe_field can be destroyed */
		fe_region = FE_field_get_FE_region(fe_field);
		if (fe_region)
		{
			/* ask owning FE_region if fe_field is used in nodes and elements */
			if (FE_region_is_FE_field_in_use(fe_region, fe_field))
			{
				return_code = 0;
			}
			else
			{
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_finite_element::not_in_use.  Missing FE_region");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::not_in_use.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_finite_element::not_in_use */

int Computed_field_finite_element::calculate_FE_element_field_values_for_element(
	int calculate_derivatives,
	struct FE_element *element,FE_value time,struct FE_element *top_level_element)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Establishes the FE_element_field values necessary for evaluating <field>, which
must be of type COMPUTED_FIELD_FINITE_ELEMENT. <calculate_derivatives> flag
controls whether basis functions for derivatives are also evaluated. If <field>
is already set up in the correct way, does nothing.
==============================================================================*/
{
	int need_to_add_to_list, need_update, return_code;

	ENTER(Computed_field_finite_element::calculate_FE_element_field_values_for_element);
	if (field&&element)
	{
		return_code=1;
		/* clear the cache if values already cached for a node */
		if (field->node)
		{
			Computed_field_clear_cache(field);
		}
		/* ensure we have FE_element_field_values for element, with
			 derivatives_calculated if requested */
		if ((!fe_element_field_values)||
			(!FE_element_field_values_are_for_element_and_time(
				fe_element_field_values,element,time,top_level_element))||
			(calculate_derivatives&&
				(!FE_element_field_values_have_derivatives_calculated(fe_element_field_values))))
		{
			need_update = 0;
			need_to_add_to_list = 0;
			if (!field_values_cache)
			{
				field_values_cache = CREATE(LIST(FE_element_field_values))();
			}
			if (!(fe_element_field_values = FIND_BY_IDENTIFIER_IN_LIST(
						FE_element_field_values, element)(element, field_values_cache)))
			{
				need_update = 1;
				fe_element_field_values = CREATE(FE_element_field_values)();
				if (fe_element_field_values)
				{
					need_to_add_to_list = 1;
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				if ((!FE_element_field_values_are_for_element_and_time(
						 fe_element_field_values,element,time,top_level_element))||
					(calculate_derivatives&&
						(!FE_element_field_values_have_derivatives_calculated(fe_element_field_values))))
				{
					need_update = 1;
					clear_FE_element_field_values(fe_element_field_values);
				}
			}
			if (return_code && need_update)
			{
				/* note that FE_element_field_values accesses the element */
				if (calculate_FE_element_field_values(element,fe_field,
						time,calculate_derivatives,fe_element_field_values,
						top_level_element))
				{
					if (need_to_add_to_list)
					{
						/* Set a cache size limit */
						if (1000 < NUMBER_IN_LIST(FE_element_field_values)(field_values_cache))
						{
							REMOVE_ALL_OBJECTS_FROM_LIST(FE_element_field_values)
								(field_values_cache);
						}
						return_code = ADD_OBJECT_TO_LIST(FE_element_field_values)(
							fe_element_field_values, field_values_cache);
					}
				}
				else
				{
					/* clear element to indicate that values are clear */
					clear_FE_element_field_values(fe_element_field_values);
					return_code=0;
				}
			 
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_finite_element::calculate_FE_element_field_values_for_element.  "
				"Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::calculate_FE_element_field_values_for_element.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_finite_element::calculate_FE_element_field_values_for_element */

int Computed_field_finite_element::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	enum Value_type value_type;
	int error, i, *int_values, return_code;

	ENTER(Computed_field_finite_element::evaluate_cache_at_location);
	if (field && location)
	{
		Field_element_xi_location *element_xi_location;
		Field_node_location *node_location;
		element_xi_location = dynamic_cast<Field_element_xi_location*>(location);
		if (element_xi_location)
		{
			FE_element* element = element_xi_location->get_element();
 			FE_element* top_level_element = element_xi_location->get_top_level_element();
			FE_value time = element_xi_location->get_time();
			const FE_value* xi = element_xi_location->get_xi();
			int number_of_derivatives = location->get_number_of_derivatives();

			/* 1. Precalculate any source fields that this field depends on.
				For type COMPUTED_FIELD_FINITE_ELEMENT, this means getting
				FE_element_field_values.*/
			return_code=calculate_FE_element_field_values_for_element(
				(0<number_of_derivatives),element,time,top_level_element);
			if (return_code)
			{
				/* 2. Calculate the field */
				value_type=get_FE_field_value_type(fe_field);
				/* component number -1 = calculate all components */
				switch (value_type)
				{
					case FE_VALUE_VALUE:
					case SHORT_VALUE:
					{
						if (number_of_derivatives)
						{
							return_code=calculate_FE_element_field(-1,
								fe_element_field_values,xi,field->values,
								field->derivatives);
							field->derivatives_valid = (0<number_of_derivatives);
						}
						else
						{
							return_code=calculate_FE_element_field(-1,
								fe_element_field_values,xi,field->values,
								(FE_value *)NULL);
						}
					} break;
					case INT_VALUE:
					{
						/* no derivatives for this value_type */
						field->derivatives_valid=0;
						if (number_of_derivatives)
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_finite_element::evaluate_cache_at_location.  "
								"Derivatives not defined for integer fields");
							return_code=0;
						}
						else
						{
							if (ALLOCATE(int_values,int,field->number_of_components))
							{
								return_code=calculate_FE_element_field_int_values(-1,
									fe_element_field_values,xi,int_values);
								for (i=0;i<field->number_of_components;i++)
								{
									field->values[i]=(FE_value)int_values[i];
								}
								DEALLOCATE(int_values);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Computed_field_finite_element::evaluate_cache_at_location.  "
									"Not enough memory for int_values");
								return_code=0;
							}
						}
					} break;
					case STRING_VALUE:
					{
						return_code = calculate_FE_element_field_as_string(-1,
							fe_element_field_values,xi,&(field->string_cache));
						field->values_valid = 0;
						field->derivatives_valid = 0;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_finite_element::evaluate_cache_at_location.  "
							"Unsupported value type %s in finite_element field",
							Value_type_string(value_type));
						return_code=0;
					} break;
				}
			}
		}
		else if (NULL != (node_location =	dynamic_cast<Field_node_location*>(location)))
		{
			double double_value;
			float float_value;
			int int_value;
			short short_value;

			FE_node *node = node_location->get_node();
			FE_value time = node_location->get_time();

			/* not very efficient - should cache FE_node_field or similar */
			return_code = 1;
			value_type=get_FE_field_value_type(fe_field);
			error = 0;
			for (i=0;(i<field->number_of_components)&&return_code;i++)
			{
				switch (value_type)
				{
					case DOUBLE_VALUE:
					{
						return_code=get_FE_nodal_double_value(node,
							fe_field,/*component_number*/i, /*version_number*/0,
							/*nodal_value_type*/FE_NODAL_VALUE,time,&double_value);
						field->values[i] = (FE_value)double_value;
					} break;
					case FE_VALUE_VALUE:
					{
						return_code=get_FE_nodal_FE_value_value(node,
							fe_field,/*component_number*/i, /*version_number*/0,
							/*nodal_value_type*/FE_NODAL_VALUE,time,&(field->values[i]));
					} break;
					case FLT_VALUE:
					{
						return_code=get_FE_nodal_float_value(node,fe_field,/*component_number*/i, 
							/*version_number*/0,/*nodal_value_type*/FE_NODAL_VALUE,
							time,&float_value);
						field->values[i] = (FE_value)float_value;
					} break;
					case INT_VALUE:
					{
						return_code=get_FE_nodal_int_value(node,fe_field,/*component_number*/i, 
							/*version_number*/0,/*nodal_value_type*/FE_NODAL_VALUE,
							time,&int_value);
						field->values[i] = (FE_value)int_value;
					} break;
					case SHORT_VALUE:
					{
						return_code=get_FE_nodal_short_value(node,fe_field,/*component_number*/i, 
							/*version_number*/0,/*nodal_value_type*/FE_NODAL_VALUE,
							time,&short_value);
						field->values[i] = (FE_value)short_value;
					} break;
					case ELEMENT_XI_VALUE:
					{
						// don't cache the value as it is relatively cheap to extract from node
						return_code = FE_field_is_defined_at_node(fe_field, node);
						field->values_valid = 0;
					} break;
  					case STRING_VALUE:
					case URL_VALUE:
					{
						// can only have 1 component
						return_code = get_FE_nodal_value_as_string(node,fe_field,
							/*component_number*/i,
							/*version_number*/0,/*nodal_value_type*/FE_NODAL_VALUE,
							time, &(field->string_cache));
						field->values_valid = 0;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_finite_element::evaluate_cache_at_location.  "
							"Unsupported value type %s in finite_element field",
							Value_type_string(value_type));
						return_code=0;
					} break;
				}
				/* No derivatives at node (at least at the moment!) */
				field->derivatives_valid = 0;
			}
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::evaluate_cache_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_finite_element::evaluate_cache_at_location */

int Computed_field_finite_element::set_values_at_location(
	Field_location* location, FE_value *values)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Sets the <values> of the computed <field> over the <element>.
==============================================================================*/
{
	enum Value_type value_type;
	FE_value *grid_values;
	int element_dimension, i, j, k, grid_map_number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],
		indices[MAXIMUM_ELEMENT_XI_DIMENSIONS], *grid_int_values, offset,
		return_code = 0;
	struct FE_element_shape *element_shape;
#if defined (OLD_CODE)
	enum Value_type value_type;
	int grid_map_number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],
		i,*int_values,j,k,number_of_points,offset,return_code;
#endif /* defined (OLD_CODE) */

	ENTER(Computed_field_finite_element::set_values_at_location);
	if (field && location && values)
	{
		// avoid setting values in field if only assigning to cache
		if (location->get_assign_to_cache())
		{
			return 1;
		}

		if (location->get_time() != 0)
		{
			display_message(WARNING_MESSAGE,
				"Computed_field_finite_element::set_values_at_location.  "
				"This function is not implemented for time.");
			return_code = 0;
		}

		Field_element_xi_location *element_xi_location;
		Field_node_location *node_location;
		element_xi_location = dynamic_cast<Field_element_xi_location*>(location);
		if (element_xi_location)
		{
			FE_element* element = element_xi_location->get_element();
			const FE_value* xi = element_xi_location->get_xi();

			element_dimension = get_FE_element_dimension(element);
			if (FE_element_field_is_grid_based(element,fe_field))
			{
				return_code=1;
				for (k = 0 ; (k < field->number_of_components) && return_code ; k++)
				{
					/* ignore non-grid-based components */
					if (get_FE_element_field_component_grid_map_number_in_xi(element,
							fe_field, /*component_number*/k, grid_map_number_in_xi))
					{
						if (get_FE_element_shape(element, &element_shape) &&
							FE_element_shape_get_indices_for_xi_location_in_cell_corners(
								element_shape, grid_map_number_in_xi, xi, indices))
						{
							offset = indices[element_dimension - 1];
							for (i = element_dimension - 2 ; i >= 0 ; i--)
							{
								offset = offset * (grid_map_number_in_xi[i] + 1) + indices[i];
							}
							value_type=get_FE_field_value_type(fe_field);
							switch (value_type)
							{
								case FE_VALUE_VALUE:
								{
									if (get_FE_element_field_component_grid_FE_value_values(element,
											fe_field, k, &grid_values))
									{
										grid_values[offset] = values[k];
										if (!set_FE_element_field_component_grid_FE_value_values(
												 element, fe_field, k, grid_values))
										{
											display_message(ERROR_MESSAGE,
												"Computed_field_finite_element::set_values_at_location.  "
												"Unable to set finite element grid FE_value values");
											return_code=0;
										}
										DEALLOCATE(grid_values);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"Computed_field_finite_element::set_values_at_location.  "
											"Unable to get old grid FE_value values");
										return_code=0;
									}
								} break;
								case INT_VALUE:
								{
									if (get_FE_element_field_component_grid_int_values(element,
											fe_field, k, &grid_int_values))
									{
										grid_int_values[offset] = (int)values[k];
										if (!set_FE_element_field_component_grid_int_values(
												 element, fe_field, k, grid_int_values))
										{
											display_message(ERROR_MESSAGE,
												"Computed_field_finite_element::set_values_at_location.  "
												"Unable to set finite element grid FE_value values");
											return_code=0;
										}
										DEALLOCATE(grid_int_values);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"Computed_field_finite_element::set_values_at_location.  "
											"Unable to get old grid FE_value values");
										return_code=0;
									}
								} break;
								default:
								{
									display_message(ERROR_MESSAGE,
										"Computed_field_finite_element::set_values_at_location.  "
										"Unsupported value_type for finite_element field");
									return_code=0;
								} break;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_finite_element::set_values_at_location.  "
								"Element locations do not coincide with grid");
							return_code = 0;
						}
					}
				}
			}
			else if (FE_element_field_is_standard_node_based(element, fe_field))
			{
				return_code=1;
			
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_finite_element::set_values_at_location.  "
					"Failed for field %s.",field->name,
					Computed_field_get_type_string(field));
			}
		}
		else if (NULL != (node_location =	dynamic_cast<Field_node_location*>(location)))
		{
			double double_value;
			float float_value;
			int int_value;
			short short_value;

			FE_node *node = node_location->get_node();
			FE_value time = node_location->get_time();

			return_code = 1;
			value_type=get_FE_field_value_type(fe_field);
			for (i=0;(i<field->number_of_components)&&return_code;i++)
			{
				/* set values all versions; to set values for selected version only,
					use COMPUTED_FIELD_NODE_VALUE instead */
				k=get_FE_node_field_component_number_of_versions(node,
					fe_field,i);
				for (j=0;(j<k)&&return_code;j++)
				{
					switch (value_type)
					{
						case DOUBLE_VALUE:
						{
							double_value=(double)values[i];
							return_code=set_FE_nodal_double_value(node,fe_field,/*component_number*/i, 
								j,/*nodal_value_type*/FE_NODAL_VALUE,time,double_value);
						} break;
						case FE_VALUE_VALUE:
						{
							return_code=set_FE_nodal_FE_value_value(node,fe_field,/*component_number*/i, 
								j,/*nodal_value_type*/FE_NODAL_VALUE,time,values[i]);
						} break;
						case FLT_VALUE:
						{
							float_value=(float)values[i];
							return_code=set_FE_nodal_float_value(node,fe_field,/*component_number*/i, 
								j,/*nodal_value_type*/FE_NODAL_VALUE,time,float_value);
						} break;
						case INT_VALUE:
						{
							int_value=(int)floor(values[i]+0.5);
							return_code=set_FE_nodal_int_value(node,fe_field,/*component_number*/i, 
								j,/*nodal_value_type*/FE_NODAL_VALUE,time,int_value);
						} break;
						case SHORT_VALUE:
						{
							short_value=(short)floor(values[i]+0.5);
							return_code=set_FE_nodal_short_value(node,fe_field,/*component_number*/i, 
								j,/*nodal_value_type*/FE_NODAL_VALUE,time,short_value);
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_finite_element::set_values_at_location.  "
								"Could not set finite_element field %s at node",field->name);
							return_code=0;
						} break;
					}
				}
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_finite_element::set_values_at_location.  "
					"Could not set finite_element field %s at node",field->name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_finite_element::set_values_at_location.  "
				"Location type is unknown or not implemented.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::set_values_at_location.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_finite_element::set_values_at_location */

int Computed_field_finite_element::set_mesh_location_value(Field_location* location, Cmiss_element_id element, FE_value *xi)
{
	int return_code;
	Field_node_location *node_location = dynamic_cast<Field_node_location*>(location);
	if (node_location &&
		(get_FE_field_value_type(fe_field) == ELEMENT_XI_VALUE) &&
		(get_FE_field_FE_field_type(fe_field) == GENERAL_FE_FIELD))
	{
		return_code = set_FE_nodal_element_xi_value(node_location->get_node(), fe_field,
			/*component_number*/0, /*version*/0, FE_NODAL_VALUE, element, xi);
	}
	else
	{
		return_code = 0;
	}
	return (return_code);
};

int Computed_field_finite_element::set_string_at_location(
	Field_location* location, const char *string_value)
{
	int return_code;
	Field_node_location *node_location = dynamic_cast<Field_node_location*>(location);
	if (node_location &&
		(get_FE_field_value_type(fe_field) == STRING_VALUE) &&
		(get_FE_field_FE_field_type(fe_field) == GENERAL_FE_FIELD))
	{
		return_code = set_FE_nodal_string_value(node_location->get_node(),
			fe_field, /*component_number*/0, /*version*/0,
			FE_NODAL_VALUE, const_cast<char *>(string_value));
	}
	else
	{
		return_code = 0;
	}
	return (return_code);
}

int Computed_field_finite_element::make_string_cache(int component_number)
{
	if (!field)
		return 0;
	if (field->string_cache)
		return 1;
	if (get_FE_field_value_type(fe_field) != ELEMENT_XI_VALUE)
	{
		return Computed_field_core::make_string_cache(component_number);
	}
	field->string_component = -1;
	return get_FE_nodal_value_as_string(field->node, fe_field,
		/*component_number*/0, /*version_number*/0,
		/*nodal_value_type*/FE_NODAL_VALUE,  field->time,
		&(field->string_cache));
}

Cmiss_element_id Computed_field_finite_element::get_mesh_location_value(FE_value *xi) const
{
	Cmiss_element_id element = 0;
	// can only have 1 component; can only be evaluated at node so assume node location
	get_FE_nodal_element_xi_value(field->node, fe_field, /*component_number*/0,
		/*version_number*/0, FE_NODAL_VALUE, &element, xi);
	return element;
}

int Computed_field_finite_element::get_native_discretization_in_element(
	struct FE_element *element,int *number_in_xi)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the <field> is grid-based in <element>, returns in
<number_in_xi> the numbers of finite difference cells in each xi-direction of
<element>. Note that this number is one less than the number of grid points in
each direction. <number_in_xi> should be allocated with at least as much space
as the number of dimensions in <element>, but is assumed to have no more than
MAXIMUM_ELEMENT_XI_DIMENSIONS so that
int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS] can be passed to this function.
Returns 0 with no errors if the field is not grid-based.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_native_discretization_in_element);
	if (field&&element&&number_in_xi&&
		(MAXIMUM_ELEMENT_XI_DIMENSIONS>=get_FE_element_dimension(element)))
	{
		if (FE_element_field_is_grid_based(element,fe_field))
		{
			/* use only first component */
			return_code=get_FE_element_field_component_grid_map_number_in_xi(element,
				fe_field, /*component_number*/0, number_in_xi);
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_native_discretization_in_element.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_native_discretization_in_element */

int Computed_field_finite_element::find_element_xi(
	FE_value *values, int number_of_values, struct FE_element **element,
	FE_value *xi, int element_dimension, double time,
	struct Cmiss_region *search_region)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_find_element_xi);
	if (field && values && (number_of_values == field->number_of_components) &&
		element && xi && search_region)
	{
		return_code = Computed_field_perform_find_element_xi(field,
			values, number_of_values, time, element, xi, element_dimension, search_region,
			/*find_nearest_element*/0);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_find_element_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_find_element_xi */

int Computed_field_finite_element::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	char *field_name;
	int return_code;

	ENTER(List_Computed_field_finite_element);
	if (field)
	{
		return_code=GET_NAME(FE_field)(fe_field,&field_name);
		if (return_code)
		{
			display_message(INFORMATION_MESSAGE,"    fe_field : %s\n",field_name);
			DEALLOCATE(field_name);
		}
		display_message(INFORMATION_MESSAGE,"    CM field type : %s\n",
			ENUMERATOR_STRING(CM_field_type)(get_FE_field_CM_field_type(fe_field)));
		display_message(INFORMATION_MESSAGE,"    Value type : %s\n",
			Value_type_string(get_FE_field_value_type(fe_field)));
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_finite_element.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_finite_element */

char *Computed_field_finite_element::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *component_name, temp_string[40];
	int error, i, number_of_components;

	ENTER(Computed_field_finite_element::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_finite_element_type_string, &error);
		number_of_components = get_FE_field_number_of_components(fe_field);
		sprintf(temp_string, " number_of_components %d ", number_of_components);
		append_string(&command_string, temp_string, &error);
		append_string(&command_string, ENUMERATOR_STRING(CM_field_type)(
			get_FE_field_CM_field_type(fe_field)), &error);
		append_string(&command_string, " ", &error);
		append_string(&command_string,
			Value_type_string(get_FE_field_value_type(fe_field)), &error);
		append_string(&command_string, " component_names", &error);
		for (i = 0; i < number_of_components; i++)
		{
			component_name = get_FE_field_component_name(fe_field, i);
			if (component_name)
			{
				make_valid_token(&component_name);
				append_string(&command_string, " ", &error);
				append_string(&command_string, component_name, &error);
				DEALLOCATE(component_name);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_finite_element::get_command_string */

} //namespace

inline Computed_field *Computed_field_cast(
	Cmiss_field_finite_element *finite_element_field)
{
	return (reinterpret_cast<Computed_field*>(finite_element_field));
}

inline Computed_field_finite_element *Computed_field_finite_element_core_cast(
	Cmiss_field_finite_element *finite_element_field)
{
	return (static_cast<Computed_field_finite_element*>(
		reinterpret_cast<Computed_field*>(finite_element_field)->core));
}

static struct Computed_field *Computed_field_create_finite_element_internal(
	struct Cmiss_field_module *field_module, struct FE_field *fe_field)
{
	char **component_names;
	int i, number_of_components, return_code;
	struct Computed_field *field = NULL;

	ENTER(Computed_field_create_finite_element_internal);
	if (field_module && fe_field)
	{
		return_code = 1;
		Cmiss_region *region = Cmiss_field_module_get_region(field_module);
		FE_region *fe_region = Cmiss_region_get_FE_region(region);
		FE_region *master_fe_region = NULL;
		FE_region_get_ultimate_master_FE_region(fe_region, &master_fe_region);
		if (FE_field_get_FE_region(fe_field) != master_fe_region)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_create_finite_element_internal.  Region mismatch");
			return_code = 0;
		}
		number_of_components = get_FE_field_number_of_components(fe_field);
		/* 1. make dynamic allocations for any new type-specific data */
		if (ALLOCATE(component_names, char *, number_of_components))
		{
			for (i = 0 ; i < number_of_components; i++)
			{
				if (!(component_names[i]=get_FE_field_component_name(fe_field,i)))
				{
					return_code = 0;
				}
			}
		}
		else
		{
			return_code = 0;
		}
		if (return_code)
		{
			field = Computed_field_create_generic(field_module,
				/*check_source_field_regions*/true, number_of_components,
				/*number_of_source_fields*/0, NULL,
				/*number_of_source_values*/0, NULL,
				new Computed_field_finite_element(fe_field));
			if (field)
			{
				// following should be a function call
				field->component_names = component_names;
			}
			else
			{
				return_code = 0;
			}
		}
		if ((!return_code) && (component_names))
		{
			for (i = 0 ; i < number_of_components ; i++)
			{
				DEALLOCATE(component_names[i]);
			}
			DEALLOCATE(component_names);
		}
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_finite_element_internal.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (field);
}

Cmiss_field_id Cmiss_field_module_create_finite_element(
	Cmiss_field_module_id field_module, int number_of_components)
{
	Computed_field *field = NULL;
	if (field_module && (0 < number_of_components))
	{
		// cache changes to ensure FE_field not automatically wrapped already
		Cmiss_field_module_begin_change(field_module);
		FE_region *fe_region = Cmiss_region_get_FE_region(Cmiss_field_module_get_region_internal(field_module));
		// ensure FE_field and Computed_field have same name
		char *field_name = Cmiss_field_module_get_field_name(field_module);
		bool no_default_name = (0 == field_name);
		if (no_default_name)
		{
			field_name = Cmiss_field_module_get_unique_field_name(field_module);
			Cmiss_field_module_set_field_name(field_module, field_name);
		}
		FE_field *fe_field = FE_region_get_FE_field_with_general_properties(
			fe_region, field_name, FE_VALUE_VALUE, number_of_components);
		if (fe_field)
		{
			Coordinate_system coordinate_system = Cmiss_field_module_get_coordinate_system(field_module);
			set_FE_field_coordinate_system(fe_field, &coordinate_system);
			field = Computed_field_create_generic(field_module,
				/*check_source_field_regions*/false, number_of_components,
				/*number_of_source_fields*/0, NULL,
				/*number_of_source_values*/0, NULL,
				new Computed_field_finite_element(fe_field));
		}
		DEALLOCATE(field_name);
		if (no_default_name)
		{
			Cmiss_field_module_set_field_name(field_module, /*field_name*/0);
		}
		Cmiss_field_module_end_change(field_module);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_finite_element.  Invalid argument(s)");
	}
	return (field);
}

Cmiss_field_finite_element_id Cmiss_field_cast_finite_element(Cmiss_field_id field)
{
	if (field)
	{
		Computed_field_finite_element* core =
			dynamic_cast<Computed_field_finite_element*>(field->core);
		if (core &&
			(get_FE_field_FE_field_type(core->fe_field) == GENERAL_FE_FIELD) &&
			(get_FE_field_value_type(core->fe_field) == FE_VALUE_VALUE))
		{
			Cmiss_field_access(field);
			return (reinterpret_cast<Cmiss_field_finite_element_id>(field));
		}
	}
	return 0;
}

int Cmiss_field_finite_element_destroy(
	Cmiss_field_finite_element_id *finite_element_field_address)
{
	return Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id *>(finite_element_field_address));
}

int Computed_field_is_type_finite_element(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_finite_element);
	if (field)
	{
		if (dynamic_cast<Computed_field_finite_element*>(field->core))
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
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_finite_element.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_finite_element */

int Computed_field_get_type_finite_element(struct Computed_field *field,
	struct FE_field **fe_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_FINITE_ELEMENT, the FE_field being
"wrapped" by it is returned - otherwise an error is reported.
==============================================================================*/
{
	Computed_field_finite_element* core;
	int return_code;

	ENTER(Computed_field_get_type_finite_element);
	if (field&&(core=dynamic_cast<Computed_field_finite_element*>(field->core)))
	{
		*fe_field=core->fe_field;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_finite_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_finite_element */

int define_Computed_field_type_finite_element(struct Parse_state *state,
	void *field_modify_void,void *computed_field_finite_element_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Creates FE_fields with the given name, coordinate_system, value_type,
cm_field_type, number_of_components and component_names.
The actual finite_element wrapper is not made here but in response to the
FE_field being made and/or modified.
==============================================================================*/
{
	char **component_names,**temp_component_names;
	const char *current_token;
	const char *cm_field_type_string,**valid_strings,*value_type_string;
	enum CM_field_type cm_field_type;
	enum Value_type value_type;
	int i,number_of_components,number_of_valid_strings,
		original_number_of_components,return_code;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;

	ENTER(define_Computed_field_type_finite_element);
	USE_PARAMETER(computed_field_finite_element_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code = 1;
		Cmiss_field_id existing_field = field_modify->get_field();
		struct FE_field *existing_fe_field = (struct FE_field *)NULL;
		if (existing_field && Computed_field_is_type_finite_element(existing_field))
		{
			return_code =
				Computed_field_get_type_finite_element(field_modify->get_field(), &existing_fe_field);
		}
		if (return_code)
		{
			if (existing_fe_field)
			{
				number_of_components=
					get_FE_field_number_of_components(existing_fe_field);
				cm_field_type = get_FE_field_CM_field_type(existing_fe_field);
				value_type = get_FE_field_value_type(existing_fe_field);
			}
			else
			{
				/* default to real, scalar general field */
				number_of_components = 1;
				cm_field_type = CM_GENERAL_FIELD;
				value_type = FE_VALUE_VALUE;
			}
			if (return_code)
			{
				cm_field_type_string = ENUMERATOR_STRING(CM_field_type)(cm_field_type);
				value_type_string = Value_type_string(value_type);
			}
			if (ALLOCATE(component_names, char *, number_of_components))
			{
				for (i = 0; i < number_of_components; i++)
				{
					if (existing_fe_field)
					{
						component_names[i] =
							get_FE_field_component_name(existing_fe_field, i);
					}
					else
					{
						char temp[20];
						sprintf(temp, "%d", i+1);
						component_names[i] = duplicate_string(temp);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_finite_element.  Not enough memory");
				return_code = 0;
			}
			original_number_of_components = number_of_components;
			/* try to handle help first */
			if (return_code && (current_token = state->current_token))
			{
				if (!(strcmp(PARSER_HELP_STRING, current_token) &&
					strcmp(PARSER_RECURSIVE_HELP_STRING, current_token)))
				{
					option_table = CREATE(Option_table)();
					/* cm_field_type */
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(CM_field_type)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(CM_field_type) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&cm_field_type_string);
					DEALLOCATE(valid_strings);
					/* component_names */
					Option_table_add_entry(option_table,"component_names",component_names,
						&original_number_of_components,set_names);
					/* value_type */
					valid_strings=Value_type_get_valid_strings_simple(
						&number_of_valid_strings);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&value_type_string);
					DEALLOCATE(valid_strings);
					/* number_of_components */
					Option_table_add_entry(option_table,"number_of_components",
						&number_of_components,NULL,set_int_positive);
					return_code=Option_table_multi_parse(option_table,state);
					DESTROY(Option_table)(&option_table);
				}
			}
			/* parse the number_of_components */
			current_token=state->current_token;
			if (return_code && current_token)
			{
				/* ... only if the "number_of_components" token is next */
				if (fuzzy_string_compare(current_token,"number_of_components"))
				{
					option_table=CREATE(Option_table)();
					Option_table_add_entry(option_table,"number_of_components",
						&number_of_components,NULL,set_int_positive);
					return_code=Option_table_parse(option_table,state);
					if (return_code)
					{
						if (number_of_components != original_number_of_components)
						{
							/* deallocate any names that will no longer be with the field */
							for (i=number_of_components;i<original_number_of_components;i++)
							{
								if (component_names[i])
								{
									DEALLOCATE(component_names[i]);
								}
							}
							if (REALLOCATE(temp_component_names,component_names,char *,
								number_of_components))
							{
								component_names=temp_component_names;
								/* clear any new names */
								for (i=original_number_of_components;i<number_of_components;i++)
								{
									component_names[i]=(char *)NULL;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"define_Computed_field_type_finite_element.  "
									"Not enough memory");
								number_of_components=original_number_of_components;
								return_code=0;
							}
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			/* parse the remainder */
			if (return_code && state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* cm_field_type */
				valid_strings = ENUMERATOR_GET_VALID_STRINGS(CM_field_type)(
					&number_of_valid_strings,
					(ENUMERATOR_CONDITIONAL_FUNCTION(CM_field_type) *)NULL,
					(void *)NULL);
				Option_table_add_enumerator(option_table, number_of_valid_strings,
					valid_strings, &cm_field_type_string);
				DEALLOCATE(valid_strings);
				/* component_names */
				Option_table_add_entry(option_table, "component_names", component_names,
					&number_of_components, set_names);
				/* value_type */
				valid_strings = Value_type_get_valid_strings_simple(
					&number_of_valid_strings);
				Option_table_add_enumerator(option_table, number_of_valid_strings,
					valid_strings, &value_type_string);
				DEALLOCATE(valid_strings);
				return_code = Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			if (return_code)
			{
				value_type = Value_type_from_string(value_type_string);
				STRING_TO_ENUMERATOR(CM_field_type)(cm_field_type_string, &cm_field_type);
				Cmiss_field_module *field_module = field_modify->get_field_module();
				// cache changes to ensure FE_field not automatically wrapped already
				Cmiss_field_module_begin_change(field_module);
				char *field_name = Cmiss_field_module_get_field_name(field_modify->get_field_module());
				FE_field *fe_field = FE_region_get_FE_field_with_general_properties(
					Cmiss_region_get_FE_region(Cmiss_field_module_get_region_internal(field_module)),
					field_name, value_type, number_of_components);
				Coordinate_system coordinate_system = Cmiss_field_module_get_coordinate_system(field_module);
				if (fe_field &&
					set_FE_field_CM_field_type(fe_field, cm_field_type) &&
					set_FE_field_coordinate_system(fe_field, &coordinate_system))
				{
					if (component_names)
					{
						for (i=0;i<number_of_components;i++)
						{
							if (component_names[i])
							{
								set_FE_field_component_name(fe_field, i, component_names[i]);
							}
						}
					}
					return_code = field_modify->update_field_and_deaccess(
						Computed_field_create_finite_element_internal(field_module, fe_field));
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx define field finite_element.  Cannot change value type or number of components of existing field");
					return_code = 0;
				}
				DEALLOCATE(field_name);
				Cmiss_field_module_end_change(field_module);
			}
			/* clean up the component_names array */
			if (component_names)
			{
				for (i=0;i<number_of_components;i++)
				{
					if (component_names[i])
					{
						DEALLOCATE(component_names[i]);
					}
				}
				DEALLOCATE(component_names);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_finite_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_finite_element */

namespace {

char computed_field_cmiss_number_type_string[] = "cmiss_number";

class Computed_field_cmiss_number : public Computed_field_core
{
public:
	Computed_field_cmiss_number() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_cmiss_number();
	}

	const char *get_type_string()
	{
		return(computed_field_cmiss_number_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_cmiss_number*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

int Computed_field_cmiss_number::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache in the element.
==============================================================================*/
{
	int element_dimension, i, return_code;
	struct CM_element_information cm_identifier;

	ENTER(Computed_field_cmiss_number::evaluate_cache_at_location);
	if (field && location)
	{
		Field_element_xi_location *element_xi_location;
		Field_node_location *node_location;
		element_xi_location = dynamic_cast<Field_element_xi_location*>(location);
		if (element_xi_location)
		{
			FE_element* element = element_xi_location->get_element();

			/* simply convert the element number into an FE_value */
			if (get_FE_element_identifier(element, &cm_identifier))
			{
				field->values[0] = cm_identifier.number;
				return_code = 1;
			}
			else
			{
				field->values[0] = 0;
				return_code = 0;
			}
			/* derivatives are always zero for this type, hence always calculated */
			element_dimension = get_FE_element_dimension(element);
			for (i = 0; i < element_dimension; i++)
			{
				field->derivatives[i] = 0.0;
			}
			field->derivatives_valid = 1;
		}
		else if (NULL != (node_location = dynamic_cast<Field_node_location*>(location)))
		{
			FE_node *node = node_location->get_node();

			/* simply convert the node number into an FE_value */
			field->values[0]=(FE_value)get_FE_node_identifier(node);
			field->derivatives_valid = 0;
			return_code = 1;
		}
		else
		{
			// Location type unknown or not implemented
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cmiss_number::evaluate_cache_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cmiss_number::evaluate_cache_at_location */

int Computed_field_cmiss_number::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_cmiss_number);
	if (field)
	{
		/* no extra parameters */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_cmiss_number.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_cmiss_number */

char *Computed_field_cmiss_number::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string;

	ENTER(Computed_field_cmiss_number::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		/* no command options */
		command_string = duplicate_string(computed_field_cmiss_number_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cmiss_number::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_cmiss_number::get_command_string */

} //namespace

int Computed_field_is_type_cmiss_number(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_cmiss_number);
	if (field)
	{
		if (dynamic_cast<Computed_field_cmiss_number*>(field->core))
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
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_cmiss_number.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_cmiss_number */

struct Computed_field *Computed_field_create_cmiss_number(
	struct Cmiss_field_module *field_module)
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/true,
		/*number_of_components*/1,
		/*number_of_source_fields*/0, NULL,
		/*number_of_source_values*/0, NULL,
		new Computed_field_cmiss_number());

	return (field);
}

int define_Computed_field_type_cmiss_number(struct Parse_state *state,
	void *field_modify_void,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CMISS_NUMBER.
==============================================================================*/
{
	Computed_field_modify_data *field_modify;
	int return_code;

	ENTER(define_Computed_field_type_cmiss_number);
	USE_PARAMETER(dummy_void);
	if (state && (field_modify = (Computed_field_modify_data *)field_modify_void))
	{
		if (!state->current_token)
		{
			return_code = field_modify->update_field_and_deaccess(
				Computed_field_create_cmiss_number(field_modify->get_field_module()));
		}
		else
		{
			if (strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
			{
				display_message(ERROR_MESSAGE,
					"Unknown option <%s>",state->current_token);
				display_parse_state_location(state);
			}
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_cmiss_number.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_cmiss_number */

#if defined (COMPUTED_FIELD_ACCESS_COUNT)
namespace {

char computed_field_access_count_type_string[] = "access_count";

class Computed_field_access_count : public Computed_field_core
{
public:
	Computed_field_access_count() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_access_count();
	}

	const char *get_type_string()
	{
		return(computed_field_access_count_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_access_count*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

int Computed_field_access_count::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_access_count::evaluate_cache_at_location);
	if (field && location)
	{
		Field_element_xi_location *element_xi_location;
		Field_node_location *node_location;

		element_xi_location = 
			dynamic_cast<Field_element_xi_location*>(location);
		if (element_xi_location != 0)
		{
			FE_element* element = element_xi_location->get_element();			
			field->values[0] = (FE_value)FE_element_get_access_count(element);
		}
		else if (0 != (node_location = 
			dynamic_cast<Field_node_location*>(location)))
		{
			FE_node *node = node_location->get_node();
			field->values[0] = (FE_value)FE_node_get_access_count(node);
		}
		else
		{
			field->values[0] = 0;
		}
	  /* no derivatives for this type */
	  field->derivatives_valid=0;
	  return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_access_count::evaluate_cache_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_access_count::evaluate_cache_at_location */

int Computed_field_access_count::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_access_count);
	if (field)
	{
		/* no extra parameters */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_access_count.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_access_count */

char *Computed_field_access_count::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string;

	ENTER(Computed_field_access_count::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		/* no command options */
		command_string = duplicate_string(computed_field_access_count_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_access_count::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_access_count::get_command_string */

} //namespace

/*****************************************************************************//**
 * Creates a field which returns the element or node access count as its value.
 * 
 * @experimental
 * @param field_module  Region field module which will own new field.
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_access_count(
	struct Cmiss_field_module *field_module)
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/true,
		/*number_of_components*/1,
		/*number_of_source_fields*/0, NULL,
		/*number_of_source_values*/0, NULL,
		new Computed_field_access_count());

	return (field);
}

int define_Computed_field_type_access_count(struct Parse_state *state,
	void *field_modify_void,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_ACCESS_COUNT.
==============================================================================*/
{
	int return_code;
	Computed_field_modify_data *field_modify;

	ENTER(define_Computed_field_type_access_count);
	USE_PARAMETER(dummy_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		if (!state->current_token)
		{
			return_code = field_modify->update_field_and_deaccess(
				Computed_field_create_access_count(field_modify->get_field_module()));
		}
		else
		{
			if (strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
			{
				display_message(ERROR_MESSAGE,
					"Unknown option <%s>",state->current_token);
				display_parse_state_location(state);
			}
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_access_count.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_access_count */
#endif /* defined (COMPUTED_FIELD_ACCESS_COUNT) */

namespace {

char computed_field_node_value_type_string[] = "node_value";

class Computed_field_node_value : public Computed_field_core
{
public:
	struct FE_field *fe_field;
	enum FE_nodal_value_type nodal_value_type;
	int version_number;

	Computed_field_node_value(FE_field *fe_field,
		enum FE_nodal_value_type nodal_value_type, int version_number) : 
		Computed_field_core(), fe_field(ACCESS(FE_field)(fe_field)),
		nodal_value_type(nodal_value_type), version_number(version_number)
	{
	};
			
	virtual ~Computed_field_node_value();

	virtual void inherit_source_field_attributes()
	{
		if (field)
		{
			Computed_field_set_coordinate_system(field,
				get_FE_field_coordinate_system(fe_field));
		}
	}
			
private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_node_value_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int is_defined_at_location(Field_location* location);

	int has_numerical_components();

	int set_values_at_location(Field_location* location, FE_value *values);

	int has_multiple_times();
};

Computed_field_node_value::~Computed_field_node_value()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{

	ENTER(Computed_field_node_value::~Computed_field_node_value);
	if (field)
	{
		if (fe_field)
		{
			DEACCESS(FE_field)(&(fe_field));
		}
		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value::~Computed_field_node_value.  "
			"Invalid arguments.");
	}
	LEAVE;

} /* Computed_field_node_value::~Computed_field_node_value */

Computed_field_core* Computed_field_node_value::copy()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_node_value* core =
		new Computed_field_node_value(fe_field, nodal_value_type, version_number);

	return (core);
} /* Computed_field_node_value::copy */

int Computed_field_node_value::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	Computed_field_node_value* other;
	int return_code;

	ENTER(Computed_field_node_value::compare);
	if (field && (other = dynamic_cast<Computed_field_node_value*>(other_core)))
	{
		return_code = ((fe_field == other->fe_field)
			&& (nodal_value_type == other->nodal_value_type)
			&& (version_number == other->version_number));
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_node_value::compare */

int Computed_field_node_value::is_defined_at_location(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Check the fe_field
==============================================================================*/
{
	int comp_no,return_code = 0;

	ENTER(Computed_field_node_value_is_defined_at_node);
	if (field && location)
	{
		Field_node_location *node_location = dynamic_cast<Field_node_location*>(location);
		if (node_location)
		{
			FE_node *node = node_location->get_node();
			
			if (FE_field_is_defined_at_node(fe_field,node))
			{
				/* must ensure at least one component of version_number,
					nodal_value_type defined at node */
				return_code=0;
				for (comp_no=0;(comp_no<field->number_of_components)&&(!return_code);
					  comp_no++)
				{
					if (FE_nodal_value_version_exists(node,fe_field,/*component_number*/comp_no, 
							version_number,nodal_value_type))
					{
						return_code=1;
					}
				}
			}
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value_is_defined_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_node_value_is_defined_at_node */

int Computed_field_node_value::has_numerical_components()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_node_value::has_numerical_components);
	if (field)
	{
		return_code=Value_type_is_numeric_simple(
			get_FE_field_value_type(fe_field));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value::has_numerical_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_node_value::has_numerical_components */

int Computed_field_node_value::evaluate_cache_at_location(
	Field_location *location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	double double_value;
	enum Value_type value_type;
	float float_value;
	int i, int_value, return_code;
	short short_value;

	ENTER(Computed_field_node_value::evaluate_cache_at_location);
	if (field && location)
	{
		Field_node_location *node_location =	dynamic_cast<Field_node_location*>(location);

		if (node_location)
		{
			FE_node *node = node_location->get_node();
			FE_value time = node_location->get_time();
	
			return_code = 1;
			value_type=get_FE_field_value_type(fe_field);
			for (i=0;(i<field->number_of_components)&&return_code;i++)
			{
				if (FE_nodal_value_version_exists(node,fe_field,/*component_number*/i, 
						version_number,nodal_value_type))
				{
					switch (value_type)
					{
						case DOUBLE_VALUE:
						{
							return_code=get_FE_nodal_double_value(node,
								fe_field,/*component_number*/i, version_number,
								nodal_value_type,time,&double_value);
							field->values[i] = (FE_value)double_value;
						} break;
						case FE_VALUE_VALUE:
						{
							return_code=get_FE_nodal_FE_value_value(node,
								fe_field,/*component_number*/i, version_number,
								nodal_value_type,time,&(field->values[i]));
						} break;
						case FLT_VALUE:
						{
							return_code=get_FE_nodal_float_value(node,
								fe_field,/*component_number*/i, version_number,
								nodal_value_type,time,&float_value);
							field->values[i] = (FE_value)float_value;
						} break;
						case INT_VALUE:
						{
							return_code=get_FE_nodal_int_value(node,fe_field,/*component_number*/i, 
								version_number,nodal_value_type,time,&int_value);
							field->values[i] = (FE_value)int_value;
						} break;
						case SHORT_VALUE:
						{
							return_code=get_FE_nodal_short_value(node,fe_field,/*component_number*/i, 
								version_number,nodal_value_type,time,&short_value);
							field->values[i] = (FE_value)short_value;
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_node_value::evaluate_cache_at_location.  "
								"Unsupported value type %s in node_value field",
								Value_type_string(value_type));
							return_code=0;
						} break;
					}
				}
				else
				{
					/* use 0 for all undefined components */
					field->values[i]=0.0;
				}
			}
		}
		else
		{
			// Only valid for Field_node_location type
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value::evaluate_cache_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_node_value::evaluate_cache_at_location */

int Computed_field_node_value::set_values_at_location(
	Field_location* location, FE_value *values)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Sets the <values> of the computed <field> at <node>.
==============================================================================*/
{
	double double_value;
	enum Value_type value_type;
	float float_value;
	int i,int_value,return_code;

	ENTER(Computed_field_node_value_set_values_at_node);
	if (field&&location&&values)
	{
		// avoid setting values in field if only assigning to cache
		if (location->get_assign_to_cache())
		{
			return 1;
		}

		Field_node_location *node_location = dynamic_cast<Field_node_location*>(location);

		if (node_location)
		{
			FE_node *node = node_location->get_node();
			FE_value time = node_location->get_time();
	
			return_code=1;

			value_type=get_FE_field_value_type(fe_field);
			for (i=0;(i<field->number_of_components)&&return_code;i++)
			{
				/* only set nodal value/versions that exist */
				if (FE_nodal_value_version_exists(node,fe_field,/*component_number*/i, 
						version_number,nodal_value_type))
				{
					switch (value_type)
					{
						case DOUBLE_VALUE:
						{
							double_value=(double)values[i];
							return_code=set_FE_nodal_double_value(node,fe_field,/*component_number*/i, 
								version_number,nodal_value_type,time,double_value);
						} break;
						case FE_VALUE_VALUE:
						{
							return_code=set_FE_nodal_FE_value_value(node,fe_field,/*component_number*/i, 
								version_number,nodal_value_type,time,values[i]);
						} break;
						case FLT_VALUE:
						{
							float_value=(float)values[i];
							return_code=set_FE_nodal_float_value(node,fe_field,/*component_number*/i, 
								version_number,nodal_value_type,time,float_value);
						} break;
						case INT_VALUE:
						{
							int_value=(int)floor(values[i]+0.5);
							return_code=set_FE_nodal_float_value(node,fe_field,/*component_number*/i, 
								version_number,nodal_value_type,time,int_value);
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_set_values_at_location.  "
								"Could not set finite_element field %s at node",field->name);
							return_code=0;
						} break;
					}
				}
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_values_at_location.  "
					"Could not set node_value field at node",field->name);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_node_value::set_values_at_location.  "
				"Only valid for Field_node_location type.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value::set_values_at_location.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_node_value::set_values_at_location */


int Computed_field_node_value::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	char *field_name;
	int return_code;

	ENTER(List_Computed_field_node_value);
	if (field)
	{
		return_code=GET_NAME(FE_field)(fe_field,&field_name);
		if (return_code)
		{
			display_message(INFORMATION_MESSAGE,"    fe_field : %s\n",field_name);
			display_message(INFORMATION_MESSAGE,"    nodal value type : %s\n",
				ENUMERATOR_STRING(FE_nodal_value_type)(nodal_value_type));
			display_message(INFORMATION_MESSAGE,"    version : %d\n",
				version_number+1);
			DEALLOCATE(field_name);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_node_value.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_node_value */

char *Computed_field_node_value::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error;

	ENTER(Computed_field_node_value::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_node_value_type_string, &error);
		append_string(&command_string, " fe_field ", &error);
		if (GET_NAME(FE_field)(fe_field, &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " ", &error);
		append_string(&command_string,
			ENUMERATOR_STRING(FE_nodal_value_type)(nodal_value_type), &error);
		sprintf(temp_string, " version %d", version_number + 1);
		append_string(&command_string, temp_string, &error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_node_value::get_command_string */

int Computed_field_node_value::has_multiple_times()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Check the fe_field
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_node_value::has_multiple_times);
	if (field)
	{
		return_code=FE_field_has_multiple_times(fe_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value::has_multiple_times.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_node_value::has_multiple_times */

} //namespace

struct Computed_field *Computed_field_create_node_value(
	struct Cmiss_field_module *field_module,
	struct FE_field *fe_field,enum FE_nodal_value_type nodal_value_type,
	int version_number)
{
	char **component_names;
	int i, number_of_components, return_code;
	struct Computed_field *field = NULL;

	ENTER(Computed_field_create_node_value);
	if (fe_field)
	{
		return_code=1;
		number_of_components = get_FE_field_number_of_components(fe_field);
		/* 1. make dynamic allocations for any new type-specific data */
		if (ALLOCATE(component_names, char *, number_of_components))
		{
			for (i = 0 ; i < number_of_components; i++)
			{
				if (!(component_names[i]=get_FE_field_component_name(fe_field,i)))
				{
					return_code = 0;
				}
			}
		}
		else
		{
			return_code = 0;
		}
		if (return_code)
		{
			field = Computed_field_create_generic(field_module,
				/*check_source_field_regions*/true, number_of_components,
				/*number_of_source_fields*/0, NULL,
				/*number_of_source_values*/0, NULL,
				new Computed_field_node_value(
					fe_field, nodal_value_type, version_number));
			if (field)
			{
				// following should be a function call
				field->component_names = component_names;
			}
			else
			{
				return_code = 0;
			}
		}
		if ((!return_code) && (component_names))
		{
			for (i = 0 ; i < number_of_components ; i++)
			{
				DEALLOCATE(component_names[i]);
			}
			DEALLOCATE(component_names);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_node_value.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* Computed_field_create_node_value */

int Computed_field_get_type_node_value(struct Computed_field *field,
	struct FE_field **fe_field,enum FE_nodal_value_type *nodal_value_type,
	int *version_number)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_NODE_VALUE, the FE_field being
"wrapped" by it is returned - otherwise an error is reported.
==============================================================================*/
{
	Computed_field_node_value* core;
	int return_code;

	ENTER(Computed_field_get_type_node_value);
	if (field&&(core = dynamic_cast<Computed_field_node_value*>(field->core)))
	{
		*fe_field=core->fe_field;
		*nodal_value_type=core->nodal_value_type;
		*version_number=core->version_number;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_node_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_node_value */

int define_Computed_field_type_node_value(struct Parse_state *state,
	void *field_modify_void,void *computed_field_finite_element_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_NODE_VALUE (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	const char *current_token;
	const char *nodal_value_type_string;
	enum FE_nodal_value_type nodal_value_type;
	int return_code,version_number;
	static const char *nodal_value_type_strings[] =
	{
	  "value",
	  "d/ds1",
	  "d/ds2",
	  "d/ds3",
	  "d2/ds1ds2",
	  "d2/ds1ds3",
	  "d2/ds2ds3",
	  "d3/ds1ds2ds3"
	};
	Computed_field_modify_data *field_modify;
	struct FE_field *fe_field;
	struct Option_table *option_table;

	ENTER(define_Computed_field_type_node_value);
	USE_PARAMETER(computed_field_finite_element_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		fe_field=(struct FE_field *)NULL;
		nodal_value_type=FE_NODAL_UNKNOWN;
		/* user enters version number starting at 1; field stores it as 0 */
		version_number=1;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_node_value_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code=Computed_field_get_type_node_value(field_modify->get_field(),&fe_field,
				&nodal_value_type,&version_number);
			version_number++;
		}
		if (return_code)
		{
			/* have ACCESS/DEACCESS because set_FE_field does */
			if (fe_field)
			{
				ACCESS(FE_field)(fe_field);
			}
			/* try to handle help first */
			current_token=state->current_token;
			if (current_token)
			{
				if (!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
				{
					option_table=CREATE(Option_table)();
					/* fe_field */
					Option_table_add_set_FE_field_from_FE_region(
						option_table, "fe_field" ,&fe_field,
					  Cmiss_region_get_FE_region(field_modify->get_region()));
					/* nodal_value_type */
					nodal_value_type_string=nodal_value_type_strings[0];
					Option_table_add_enumerator(option_table,
					  sizeof(nodal_value_type_strings)/sizeof(char *),
					  nodal_value_type_strings,&nodal_value_type_string);
					/* version_number */
					Option_table_add_entry(option_table,"version", &version_number,
					  NULL, set_int_positive);
					return_code=Option_table_multi_parse(option_table,state);
					DESTROY(Option_table)(&option_table);
				}
			}
			/* parse the fe_field if the "fe_field" token is next */
			if (return_code)
			{
				current_token=state->current_token;
				if (current_token && fuzzy_string_compare(current_token,"fe_field"))
				{
					option_table=CREATE(Option_table)();
					/* fe_field */
					Option_table_add_set_FE_field_from_FE_region(
						option_table, "fe_field" ,&fe_field,
					  Cmiss_region_get_FE_region(field_modify->get_region()));
					return_code=Option_table_parse(option_table,state);
					if (return_code)
					{
						if (!fe_field)
						{
							display_parse_state_location(state);
							display_message(ERROR_MESSAGE,"Missing or invalid fe_field");
							return_code=0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
				else
				{
					display_parse_state_location(state);
					display_message(ERROR_MESSAGE,
						"Must specify fe_field before other options");
					return_code=0;
				}
			}
			/* parse the value_type/version number */
			if (return_code&&state->current_token)
			{
			  option_table=CREATE(Option_table)();
			  /* nodal_value_type */
			  nodal_value_type_string=nodal_value_type_strings[0];
			  Option_table_add_enumerator(option_table,
				 sizeof(nodal_value_type_strings)/sizeof(char *),
				 nodal_value_type_strings,&nodal_value_type_string);
			  /* version_number */
			  Option_table_add_entry(option_table,"version", &version_number,
				 NULL, set_int_positive);
			  return_code=Option_table_multi_parse(option_table,state);
			  DESTROY(Option_table)(&option_table);
			}
			if (return_code)
			{
				if (nodal_value_type_string == nodal_value_type_strings[0])
				{
					nodal_value_type = FE_NODAL_VALUE;
				}
				else if (nodal_value_type_string == nodal_value_type_strings[1])
				{
					nodal_value_type = FE_NODAL_D_DS1;
				}
				else if (nodal_value_type_string == nodal_value_type_strings[2])
				{
					nodal_value_type = FE_NODAL_D_DS2;
				}
				else if (nodal_value_type_string == nodal_value_type_strings[3])
				{
					nodal_value_type = FE_NODAL_D_DS3;
				}
				else if (nodal_value_type_string == nodal_value_type_strings[4])
				{
					nodal_value_type = FE_NODAL_D2_DS1DS2;
				}
				else if (nodal_value_type_string == nodal_value_type_strings[5])
				{
					nodal_value_type = FE_NODAL_D2_DS1DS3;
				}
				else if (nodal_value_type_string == nodal_value_type_strings[6])
				{
					nodal_value_type = FE_NODAL_D2_DS2DS3;
				}
				else if (nodal_value_type_string == nodal_value_type_strings[7])
				{
					nodal_value_type = FE_NODAL_D3_DS1DS2DS3;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_node_value.  "
						"Unknown nodal value string.");
					return_code = 0;
				}
			}
			if (return_code)
			{
				/* user enters version number starting at 1; field stores it as 0 */
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_node_value(field_modify->get_field_module(),
						fe_field, nodal_value_type, version_number-1));
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_node_value.  Failed");
				}
			}
			if (fe_field)
			{
				DEACCESS(FE_field)(&fe_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_node_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_node_value */

namespace {

char computed_field_embedded_type_string[] = "embedded";

class Computed_field_embedded : public Computed_field_core
{
public:
	Computed_field_embedded() :
		Computed_field_core()
	{
	};

	virtual void inherit_source_field_attributes()
	{
		if (field)
		{
			Computed_field_set_coordinate_system_from_sources(field);
		}
	}

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_embedded_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int is_defined_at_location(Field_location* location);

	int has_numerical_components();

};

Computed_field_core* Computed_field_embedded::copy()
{
	return new Computed_field_embedded();
}

int Computed_field_embedded::compare(Computed_field_core *other_core)
{
	return (field && (0 != dynamic_cast<Computed_field_embedded*>(other_core)));
}

int Computed_field_embedded::is_defined_at_location(Field_location* location)
{
	return evaluate_cache_at_location(location);
}

int Computed_field_embedded::has_numerical_components()
{
	return (field && Computed_field_has_numerical_components(
		field->source_fields[0],(void *)NULL));
}

int Computed_field_embedded::evaluate_cache_at_location(Field_location* location)
{
	int return_code = 0;
	if (field && location)
	{
		if (Computed_field_evaluate_cache_at_location(field->source_fields[1], location))
		{
			FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
			Cmiss_element_id element = field->source_fields[1]->core->get_mesh_location_value(xi);
			if (element)
			{
				Field_element_xi_location element_xi_location(element, xi, location->get_time());
				if (Computed_field_evaluate_cache_at_location(
					field->source_fields[0], &element_xi_location))
				{
					for (int i = 0; i < field->number_of_components; i++)
					{
						field->values[i] = field->source_fields[0]->values[i];
					}
					return_code = 1;
				}
			}
		}
	}
	return (return_code);
}

int Computed_field_embedded::list()
{
	int return_code;
	if (field)
	{
		display_message(INFORMATION_MESSAGE, "    embedded location field : %s\n", field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE, "    source field : %s\n", field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_embedded.  Invalid arguments.");
		return_code = 0;
	}
	return (return_code);
}

/** Returns allocated command string for reproducing field. Includes type. */
char *Computed_field_embedded::get_command_string()
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_embedded::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_embedded_type_string, &error);
		append_string(&command_string, " element_xi ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_embedded::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
}

} //namespace

Cmiss_field_id Cmiss_field_create_embedded(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field,
	Cmiss_field_id embedded_location_field)
{
	struct Computed_field *field = 0;
	if (field_module && embedded_location_field && source_field &&
		(CMISS_FIELD_VALUE_TYPE_MESH_LOCATION ==
			Cmiss_field_get_value_type(embedded_location_field)) &&
		Computed_field_has_numerical_components(source_field, NULL))
	{
		Cmiss_field_id source_fields[2];
		source_fields[0] = source_field;
		source_fields[1] = embedded_location_field;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_embedded());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_embedded.  Invalid argument(s)");
	}
	return (field);
}

/** If the field is of type COMPUTED_FIELD_EMBEDDED, returns the fields it uses. */
int Computed_field_get_type_embedded(struct Computed_field *field,
	struct Computed_field **source_field_address,
	struct Computed_field **embedded_location_field_address)
{
	int return_code = 0;
	if (field && (0 != dynamic_cast<Computed_field_embedded*>(field->core)))
	{
		*source_field_address = field->source_fields[0];
		*embedded_location_field_address = field->source_fields[1];
		return_code = 1;
	}
	return (return_code);
}

int define_Computed_field_type_embedded(struct Parse_state *state,
	void *field_modify_void, void *computed_field_finite_element_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_EMBEDDED (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	int return_code;

	ENTER(define_Computed_field_type_embedded);
	Computed_field_modify_data *field_modify = (Computed_field_modify_data *)field_modify_void;
	USE_PARAMETER(computed_field_finite_element_package_void);
	if (state && field_modify)
	{
		return_code = 1;
		Cmiss_field_id source_field = 0;
		Cmiss_field_id embedded_location_field = 0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_embedded_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			Computed_field_get_type_embedded(field_modify->get_field(),
				&source_field, &embedded_location_field);
			Cmiss_field_access(source_field);
			Cmiss_field_access(embedded_location_field);
		}
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"An embedded field returns the value of a source field at the "
			"location given by the element_xi field - a field whose value is "
			"a location in a mesh.");
		/* embedded_location_field */
		struct Set_Computed_field_conditional_data set_embedded_location_field_data;
		set_embedded_location_field_data.conditional_function = Computed_field_has_value_type_mesh_location;
		set_embedded_location_field_data.conditional_function_user_data = (void *)NULL;
		set_embedded_location_field_data.computed_field_manager = field_modify->get_field_manager();
		Option_table_add_entry(option_table, "element_xi", &embedded_location_field,
			&set_embedded_location_field_data, set_Computed_field_conditional);
		/* source_field */
		struct Set_Computed_field_conditional_data set_source_field_data;
		set_source_field_data.conditional_function = Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		set_source_field_data.computed_field_manager = field_modify->get_field_manager();
		Option_table_add_entry(option_table, "field", &source_field,
			&set_source_field_data, set_Computed_field_conditional);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			if (embedded_location_field && source_field)
			{
				return_code = field_modify->update_field_and_deaccess(
					Cmiss_field_create_embedded(field_modify->get_field_module(),
						source_field, embedded_location_field));
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_embedded.  "
					"Must specify both source field and element_xi field");
				return_code = 0;
			}
		}
		if (source_field)
		{
			Cmiss_field_destroy(&source_field);
		}
		if (embedded_location_field)
		{
			Cmiss_field_destroy(&embedded_location_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_embedded.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_embedded */

namespace {

char computed_field_find_mesh_location_type_string[] = "find_mesh_location";

class Computed_field_find_mesh_location : public Computed_field_core
{
private:
	Cmiss_fe_mesh_id mesh;
	enum Cmiss_field_find_mesh_location_search_mode search_mode;
	Cmiss_element_id element_cache;
	FE_value xi_cache[MAXIMUM_ELEMENT_XI_DIMENSIONS];

public:

	Computed_field_find_mesh_location(Cmiss_fe_mesh_id mesh) :
		Computed_field_core(),
		mesh(Cmiss_fe_mesh_access(mesh)),
		search_mode(CMISS_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_EXACT),
		element_cache(0)
	{
	};

	virtual ~Computed_field_find_mesh_location();

	Cmiss_field_id get_source_field()
	{
		return field->source_fields[0];
	}

	Cmiss_field_id get_mesh_field()
	{
		return field->source_fields[1];
	}

	Cmiss_fe_mesh_id get_mesh()
	{
		return mesh;
	}

	enum Cmiss_field_find_mesh_location_search_mode get_search_mode() const
	{
		return search_mode;
	}

	int set_search_mode(enum Cmiss_field_find_mesh_location_search_mode search_mode_in)
	{
		if (search_mode_in != search_mode)
		{
			search_mode = search_mode_in;
			Computed_field_changed(field);
		}
		return 1;
	}

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return (computed_field_find_mesh_location_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int clear_cache();

	int is_defined_at_location(Field_location* location);

	int has_numerical_components()
	{
		return 0;
	}

	virtual int make_string_cache(int component_number = -1);

	virtual Cmiss_element_id get_mesh_location_value(FE_value *xi) const;

	virtual Cmiss_field_value_type get_value_type() const
	{
		return CMISS_FIELD_VALUE_TYPE_MESH_LOCATION;
	}

};

Computed_field_find_mesh_location::~Computed_field_find_mesh_location()
{
	clear_cache();
	Cmiss_fe_mesh_destroy(&mesh);
}

Computed_field_core* Computed_field_find_mesh_location::copy()
{
	return new Computed_field_find_mesh_location(mesh);
}

int Computed_field_find_mesh_location::clear_cache()
{
	if (element_cache)
	{
		Cmiss_element_destroy(&element_cache);
	}
	return 1;
}

int Computed_field_find_mesh_location::compare(Computed_field_core *other_core)
{
	Computed_field_find_mesh_location* other;
	int return_code = 0;
	if (field && (other = dynamic_cast<Computed_field_find_mesh_location*>(other_core)))
	{
		return_code = (mesh == other->mesh);
	}
	return (return_code);
}

int Computed_field_find_mesh_location::is_defined_at_location(Field_location* location)
{
	return evaluate_cache_at_location(location);
}

int Computed_field_find_mesh_location::evaluate_cache_at_location(Field_location* location)
{
	int return_code = 0;

	ENTER(Computed_field_find_mesh_location::evaluate_cache_at_location);
	if (field && location)
	{
		if (Computed_field_evaluate_cache_at_location(get_source_field(), location))
		{
			if (element_cache)
			{
				Cmiss_element_destroy(&element_cache);
			}
			if (Computed_field_find_element_xi(get_mesh_field(),
					get_source_field()->values, get_source_field()->number_of_components,
					location->get_time(), &element_cache, xi_cache, Cmiss_fe_mesh_get_dimension(mesh),
					Cmiss_fe_mesh_get_region(mesh), /*propagate_field*/0,
					/*find_nearest*/(search_mode != CMISS_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_EXACT))
					&& element_cache)
			{
				Cmiss_element_access(element_cache);
				field->values_valid = 0; // not real valued
				return_code = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_find_mesh_location::evaluate_cache_at_location.  "
			"Invalid arguments.");
	}
	return (return_code);
}

int Computed_field_find_mesh_location::list()
{
	int return_code = 0;
	if (field)
	{
		display_message(INFORMATION_MESSAGE, "    search mode : ");
		if (search_mode == CMISS_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_NEAREST)
		{
			display_message(INFORMATION_MESSAGE, " find_nearest\n");
		}
		else
		{
			display_message(INFORMATION_MESSAGE, " find_exact\n");
		}
		display_message(INFORMATION_MESSAGE, "    mesh : ");
		char *mesh_name = Cmiss_fe_mesh_get_name(mesh);
		display_message(INFORMATION_MESSAGE, "%s\n", mesh_name);
		DEALLOCATE(mesh_name);
		display_message(INFORMATION_MESSAGE,
			"    mesh field : %s\n", get_mesh_field()->name);
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n", get_source_field()->name);
		return_code = 1;
	}
	return (return_code);
}

/** Returns allocated command string for reproducing field. Includes type */
char *Computed_field_find_mesh_location::get_command_string()
{
	char *command_string = 0;
	int error = 0;
	if (field)
	{
		append_string(&command_string, computed_field_find_mesh_location_type_string, &error);

		if (search_mode == CMISS_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_NEAREST)
		{
			append_string(&command_string, " find_nearest", &error);
		}
		else
		{
			append_string(&command_string, " find_exact", &error);
		}

		append_string(&command_string, " mesh ", &error);
		char *mesh_name = Cmiss_fe_mesh_get_name(mesh);
		append_string(&command_string, mesh_name, &error);
		DEALLOCATE(mesh_name);

		char *mesh_field_name = Cmiss_field_get_name(get_mesh_field());
		make_valid_token(&mesh_field_name);
		append_string(&command_string, " mesh_field ", &error);
		append_string(&command_string, mesh_field_name, &error);
		DEALLOCATE(mesh_field_name);

		char *source_field_name = Cmiss_field_get_name(get_source_field());
		make_valid_token(&source_field_name);
		append_string(&command_string, " source_field ", &error);
		append_string(&command_string, source_field_name, &error);
		DEALLOCATE(source_field_name);
	}
	return (command_string);
}


int Computed_field_find_mesh_location::make_string_cache(int component_number)
{
	if ((component_number > 0) || !element_cache || !field)
		return 0;
	if (field->string_cache)
		return 1;
	int error = 0;
	char tmp_string[50];
	sprintf(tmp_string,"%d : ", Cmiss_element_get_identifier(element_cache));
	append_string(&(field->string_cache), tmp_string, &error);
	int dimension = Cmiss_element_get_dimension(element_cache);
	for (int i = 0; i < dimension; i++)
	{
		if (0 < i)
		{
			sprintf(tmp_string,", %g", xi_cache[i]);
		}
		else
		{
			sprintf(tmp_string,"%g", xi_cache[i]);
		}
		append_string(&(field->string_cache), tmp_string, &error);
	}
	field->string_component = -1;
	return (!error);
}

Cmiss_element_id Computed_field_find_mesh_location::get_mesh_location_value(FE_value *xi) const
{
	for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
	{
		xi[i] = xi_cache[i];
	}
	return element_cache;
}

} // namespace

Cmiss_field_id Cmiss_field_module_create_find_mesh_location(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field,
	Cmiss_field_id mesh_field, Cmiss_fe_mesh_id mesh)
{
	struct Computed_field *field = NULL;
	int number_of_source_field_components = Computed_field_get_number_of_components(source_field);
	int number_of_mesh_field_components = Computed_field_get_number_of_components(mesh_field);
	if (field_module && source_field && mesh_field && mesh &&
		(number_of_source_field_components == number_of_mesh_field_components) &&
		Computed_field_has_numerical_components(source_field, NULL) &&
		Computed_field_has_numerical_components(mesh_field, NULL) &&
		(number_of_mesh_field_components >= Cmiss_fe_mesh_get_dimension(mesh)))
	{
		Cmiss_field_id source_fields[2];
		source_fields[0] = source_field;
		source_fields[1] = mesh_field;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			/*number_of_components*/1,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_find_mesh_location(mesh));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_find_mesh_location.  Invalid argument(s)");
	}
	return (field);
}

struct Cmiss_field_find_mesh_location : private Computed_field
{
	inline Computed_field_find_mesh_location *get_core()
	{
		return static_cast<Computed_field_find_mesh_location*>(core);
	}
};

Cmiss_field_find_mesh_location_id Cmiss_field_cast_find_mesh_location(
	Cmiss_field_id field)
{
	if (field)
	{
		if (dynamic_cast<Computed_field_find_mesh_location*>(field->core))
		{
			Cmiss_field_access(field);
			return (reinterpret_cast<Cmiss_field_find_mesh_location_id>(field));
		}
	}
	return 0;
}

int Cmiss_field_find_mesh_location_destroy(
	Cmiss_field_find_mesh_location_id *find_mesh_location_field_address)
{
	return Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id *>(find_mesh_location_field_address));
}

Cmiss_field_id Cmiss_field_find_mesh_location_get_attribute_field(
	Cmiss_field_find_mesh_location_id find_mesh_location_field,
	enum Cmiss_field_find_mesh_location_attribute attribute)
{
	Cmiss_field_id field = 0;
	if (find_mesh_location_field)
	{
		Computed_field_find_mesh_location *core = find_mesh_location_field->get_core();
		switch (attribute)
		{
		case CMISS_FIELD_FIND_MESH_LOCATION_ATTRIBUTE_SOURCE_FIELD:
			field = core->get_source_field();
			break;
		case CMISS_FIELD_FIND_MESH_LOCATION_ATTRIBUTE_MESH_FIELD:
			field = core->get_mesh_field();
			break;
		default:
			break;
		}
		if (field)
		{
			Cmiss_field_access(field);
		}
	}
	return field;
}

Cmiss_fe_mesh_id Cmiss_field_find_mesh_location_get_mesh(
	Cmiss_field_find_mesh_location_id find_mesh_location_field)
{
	Cmiss_fe_mesh_id mesh = 0;
	if (find_mesh_location_field)
	{
		mesh = find_mesh_location_field->get_core()->get_mesh();
		Cmiss_fe_mesh_access(mesh);
	}
	return mesh;
}

enum Cmiss_field_find_mesh_location_search_mode
	Cmiss_field_find_mesh_location_get_search_mode(
		Cmiss_field_find_mesh_location_id find_mesh_location_field)
{
	Cmiss_field_find_mesh_location_search_mode search_mode =
		CMISS_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_EXACT;
	if (find_mesh_location_field)
	{
		search_mode = find_mesh_location_field->get_core()->get_search_mode();
	}
	return search_mode;
}

int Cmiss_field_find_mesh_location_set_search_mode(
	Cmiss_field_find_mesh_location_id find_mesh_location_field,
	enum Cmiss_field_find_mesh_location_search_mode search_mode)
{
	int return_code = 0;
	if (find_mesh_location_field)
	{
		return_code = find_mesh_location_field->get_core()->set_search_mode(search_mode);
	}
	return return_code;
}

/***************************************************************************//**
 * Command modifier function which converts field into find_mesh_location type
 * (if it is not already) and allows its contents to be modified.
 */
int define_Computed_field_type_find_mesh_location(struct Parse_state *state,
	void *field_modify_void, void *computed_field_finite_element_package_void)
{
	int return_code;

	ENTER(define_Computed_field_type_find_mesh_location);
	USE_PARAMETER(computed_field_finite_element_package_void);
	Computed_field_modify_data * field_modify = (Computed_field_modify_data *)field_modify_void;
	if (state && field_modify)
	{
		return_code = 1;
		Cmiss_field_id mesh_field = 0;
		Cmiss_field_id source_field = 0;
		char *mesh_name = 0;
		int find_nearest_flag = 0;
		if (NULL != field_modify->get_field())
		{
			Cmiss_field_find_mesh_location_id find_mesh_location_field =
				Cmiss_field_cast_find_mesh_location(field_modify->get_field());
			if (find_mesh_location_field)
			{
				mesh_field = Cmiss_field_find_mesh_location_get_attribute_field(
					find_mesh_location_field, CMISS_FIELD_FIND_MESH_LOCATION_ATTRIBUTE_MESH_FIELD);
				source_field = Cmiss_field_find_mesh_location_get_attribute_field(
					find_mesh_location_field, CMISS_FIELD_FIND_MESH_LOCATION_ATTRIBUTE_SOURCE_FIELD);
				Cmiss_fe_mesh_id mesh = Cmiss_field_find_mesh_location_get_mesh(find_mesh_location_field);
				mesh_name = Cmiss_fe_mesh_get_name(mesh);
				Cmiss_fe_mesh_destroy(&mesh);
				Cmiss_field_find_mesh_location_destroy(&find_mesh_location_field);
				find_nearest_flag = (CMISS_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_EXACT !=
					Cmiss_field_find_mesh_location_get_search_mode(find_mesh_location_field));
			}
		}
		if (return_code)
		{
			struct Set_Computed_field_conditional_data set_mesh_field_data, set_source_field_data;
			Option_table *option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
				"A find_mesh_location field calculates the source_field then finds "
				"and returns the location in the mesh where the mesh_field has the "
				"same value. Use an embedded field to evaluate other fields on the "
				"mesh at the found location. Option find_nearest returns the location "
				"with the nearest value of the mesh_field if no exact match is found.");
			// find_nearest|find_exact
			Option_table_add_switch(option_table,
				"find_nearest", "find_exact", &find_nearest_flag);
			// mesh
			Option_table_add_string_entry(option_table, "mesh", &mesh_name,
				" [GROUP_REGION_NAME.]cmiss_mesh_1d|cmiss_mesh_2d|cmiss_mesh_3d");
			// mesh_field
			set_mesh_field_data.conditional_function =
				Computed_field_has_numerical_components;
			set_mesh_field_data.conditional_function_user_data = (void *)NULL;
			set_mesh_field_data.computed_field_manager =
				field_modify->get_field_manager();
			Option_table_add_entry(option_table, "mesh_field", &mesh_field,
				&set_mesh_field_data, set_Computed_field_conditional);
			// source_field
			set_source_field_data.conditional_function =
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data = (void *)NULL;
			set_source_field_data.computed_field_manager =
				field_modify->get_field_manager();
			Option_table_add_entry(option_table, "source_field", &source_field,
				&set_source_field_data, set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				Cmiss_fe_mesh_id mesh = 0;
				if (mesh_name)
				{
					mesh = Cmiss_field_module_get_fe_mesh_by_name(field_modify->get_field_module(), mesh_name);
					if (!mesh)
					{
						display_message(ERROR_MESSAGE, "define_Computed_field_type_find_mesh_location.  "
							"Unknown mesh : %s", mesh_name);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "define_Computed_field_type_find_mesh_location.  Must specify mesh.");
					return_code = 0;
				}
				if (return_code)
				{
					Cmiss_field_id field = Cmiss_field_module_create_find_mesh_location(field_modify->get_field_module(),
						source_field, mesh_field, mesh);
					if (field)
					{
						Cmiss_field_find_mesh_location_id find_mesh_location_field = Cmiss_field_cast_find_mesh_location(field);
						Cmiss_field_find_mesh_location_set_search_mode(find_mesh_location_field,
							(find_nearest_flag ? CMISS_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_NEAREST
							: CMISS_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_EXACT));
						Cmiss_field_find_mesh_location_destroy(&find_mesh_location_field);
						return_code = field_modify->update_field_and_deaccess(field);
						field = 0;
					}
					else
					{
						if ((!source_field) || (!mesh_field) ||
							(Cmiss_field_get_number_of_components(source_field) !=
								Cmiss_field_get_number_of_components(mesh_field)))
						{
							display_message(ERROR_MESSAGE, "define_Computed_field_type_find_mesh_location.  "
								"Failed due to source_field and mesh_field unspecified, or number of components different or lower than mesh dimension.");
							return_code = 0;
						}
					}
				}
				if (mesh)
				{
					Cmiss_fe_mesh_destroy(&mesh);
				}
			}
		}
		if (mesh_name)
		{
			DEALLOCATE(mesh_name);
		}
		if (mesh_field)
		{
			Cmiss_field_destroy(&mesh_field);
		}
		if (source_field)
		{
			Cmiss_field_destroy(&source_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_find_mesh_location.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

namespace {

char computed_field_xi_coordinates_type_string[] = "xi_coordinates";

class Computed_field_xi_coordinates : public Computed_field_core
{
public:
	Computed_field_xi_coordinates() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_xi_coordinates();
	}

	const char *get_type_string()
	{
		return(computed_field_xi_coordinates_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_xi_coordinates*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int is_defined_at_location(Field_location* location);
};

int Computed_field_xi_coordinates::is_defined_at_location(
	 Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_xi_coordinates::is_defined_at_location);
	if (field && location)
	{
		/* Only valid for Field_element_xi_location */
		if (dynamic_cast<Field_element_xi_location*>(location))
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
		display_message(ERROR_MESSAGE,
			"Computed_field_xi_coordinates_is_defined_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_xi_coordinates_is_defined_at_node */

int Computed_field_xi_coordinates::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	FE_value *temp;
	int element_dimension, i, j, return_code = 0;

	ENTER(Computed_field_xi_coordinates::evaluate_cache_at_location);
	if (field && location)
	{
		Field_element_xi_location *element_xi_location = dynamic_cast<Field_element_xi_location*>(location);

		if (element_xi_location)
		{
			FE_element* element = element_xi_location->get_element();
			const FE_value* xi = element_xi_location->get_xi();
			
			/* returns the values in xi, up to the element_dimension and padded
				with zeroes */
			element_dimension=get_FE_element_dimension(element);
			temp=field->derivatives;
			for (i=0;i<field->number_of_components;i++)
			{
				if (i<element_dimension)
				{
					field->values[i]=xi[i];
				}
				else
				{
					field->values[i]=0.0;
				}
				for (j=0;j<element_dimension;j++)
				{
					if (i==j)
					{
						*temp = 1.0;
					}
					else
					{
						*temp = 0.0;
					}
					temp++;
				}
			}
			/* derivatives are always calculated since they are merely part of
				the identity matrix */
			field->derivatives_valid=1;
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_xi_coordinates::evaluate_cache_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_xi_coordinates::evaluate_cache_at_location */


int Computed_field_xi_coordinates::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_xi_coordinates);
	if (field)
	{
		/* no extra parameters */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_xi_coordinates.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_xi_coordinates */

char *Computed_field_xi_coordinates::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string;

	ENTER(Computed_field_xi_coordinates::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		/* no command options */
		command_string =
			duplicate_string(computed_field_xi_coordinates_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_xi_coordinates::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_xi_coordinates::get_command_string */

} // namespace

int define_Computed_field_type_xi_coordinates(struct Parse_state *state,
	void *field_modify_void,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_XI_COORDINATES.
==============================================================================*/
{
	int return_code;
	Computed_field_modify_data *field_modify;

	ENTER(define_Computed_field_type_xi_coordinates);
	USE_PARAMETER(dummy_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		if (!state->current_token)
		{
			return_code = field_modify->update_field_and_deaccess(
				Computed_field_create_xi_coordinates(field_modify->get_field_module()));
		}
		else
		{
			if (strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
			{
				display_message(ERROR_MESSAGE,
					"Unknown option <%s>",state->current_token);
				display_parse_state_location(state);
			}
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_xi_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_xi_coordinates */

int Computed_field_is_type_xi_coordinates(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_xi_coordinates);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		if (dynamic_cast<Computed_field_xi_coordinates*>(field->core))
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
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_xi_coordinates.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_xi_coordinates */

struct Computed_field *Computed_field_create_xi_coordinates(
	struct Cmiss_field_module *field_module)
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/true,
		/*number_of_components*/3,
		/*number_of_source_fields*/0, NULL,
		/*number_of_source_values*/0, NULL,
		new Computed_field_xi_coordinates());

	return (field);
}

namespace {

char computed_field_basis_derivative_type_string[] = "basis_derivative";

class Computed_field_basis_derivative : public Computed_field_core
{
public:
	FE_field* fe_field;
	int order;
	int *xi_indices;
	FE_element_field_values* fe_element_field_values;
	/* need pointer to fe_field_manager so can call MANAGED_OBJECT_NOT_IN_USE in
		 Computed_field_basis_derivative::not_in_use */

	/* Keep a cache of FE_element_field_values as calculation is expensive */
	LIST(FE_element_field_values) *field_values_cache;

	Computed_field_basis_derivative(
		FE_field *fe_field, int order, int *xi_indices_in) :
		Computed_field_core(), fe_field(ACCESS(FE_field)(fe_field)),
		order(order)
	{
		int i;

		xi_indices = new int[order];
		for (i = 0 ; i < order ; i++)
		{
			xi_indices[i] = xi_indices_in[i];
		}
		fe_element_field_values = (FE_element_field_values*)NULL;
		field_values_cache = (LIST(FE_element_field_values) *)NULL;
	};

	virtual ~Computed_field_basis_derivative();

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_basis_derivative_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int clear_cache();

	int is_defined_at_location(Field_location* location);

	int has_multiple_times();

	int has_numerical_components();

	int calculate_FE_element_field_values_for_element(
		int calculate_derivatives, struct FE_element *element,
		FE_value time, struct FE_element *top_level_element);

	int set_values_at_location(Field_location* location, FE_value *values);

	int find_element_xi(
		FE_value *values, int number_of_values, struct FE_element **element,
		FE_value *xi, int element_dimension, double time,
		struct Cmiss_region *search_region);
};

Computed_field_basis_derivative::~Computed_field_basis_derivative()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{

	ENTER(Computed_field_basis_derivative::~Computed_field_basis_derivative);
	if (field)
	{
		delete [] xi_indices;
		if (fe_field)
		{
			DEACCESS(FE_field)(&(fe_field));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_basis_derivative::~Computed_field_basis_derivative.  "
			"Invalid arguments.");
	}
	LEAVE;

} /* Computed_field_basis_derivative::~Computed_field_basis_derivative */

Computed_field_core* Computed_field_basis_derivative::copy()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_basis_derivative* core =
		new Computed_field_basis_derivative(fe_field, order, xi_indices);

	return (core);
} /* Computed_field_basis_derivative::copy */

int Computed_field_basis_derivative::clear_cache()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_basis_derivative::clear_cache_type_specific);
	if (field)
	{
		if(fe_element_field_values)
		{
			clear_FE_element_field_values(fe_element_field_values);
		}
		if (field_values_cache)
		{
			DESTROY(LIST(FE_element_field_values))(&field_values_cache);
		}
		/* These are owned by the list */
		fe_element_field_values = (FE_element_field_values *)NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_basis_derivative::clear_cache_type_specific(.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_basis_derivative::clear_cache_type_specific( */

int Computed_field_basis_derivative::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	Computed_field_basis_derivative* other;
	int return_code;

	ENTER(Computed_field_basis_derivative::compare);
	if (field && (other = dynamic_cast<Computed_field_basis_derivative*>(other_core)))
	{
		return_code = (fe_field == other->fe_field);
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_basis_derivative::compare */

int Computed_field_basis_derivative::is_defined_at_location(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_basis_derivative::is_defined_at_location);
	if (field && location && fe_field)
	{
		return_code = 0;

		Field_element_xi_location *element_xi_location;
		Field_node_location *node_location;
		element_xi_location = dynamic_cast<Field_element_xi_location*>(location);
		if (element_xi_location)
		{
			FE_element* element = element_xi_location->get_element();

			return_code = FE_field_is_defined_in_element(fe_field,element);
		}
		else if (NULL != (node_location =	dynamic_cast<Field_node_location*>(location)))
		{
			FE_node *node = node_location->get_node();

			return_code = FE_field_is_defined_at_node(fe_field,node);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_basis_derivative::is_defined_at_location.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_basis_derivative::is_defined_at_location */

int Computed_field_basis_derivative::has_multiple_times()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Check the fe_field
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_basis_derivative::has_multiple_times);
	if (field)
	{
		return_code=FE_field_has_multiple_times(fe_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_basis_derivative::has_multiple_times.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_basis_derivative::has_multiple_times */

int Computed_field_basis_derivative::has_numerical_components()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_basis_derivative::has_numerical_components);
	if (field)
	{
		return_code=Value_type_is_numeric_simple(
			get_FE_field_value_type(fe_field));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_basis_derivative::has_numerical_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_basis_derivative::has_numerical_components */

int Computed_field_basis_derivative::calculate_FE_element_field_values_for_element(
	int calculate_derivatives,
	struct FE_element *element,FE_value time,struct FE_element *top_level_element)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Establishes the FE_element_field values necessary for evaluating <field>, which
must be of type COMPUTED_FIELD_BASIS_DERIVATIVE. <calculate_derivatives> flag
controls whether basis functions for derivatives are also evaluated. If <field>
is already set up in the correct way, does nothing.
==============================================================================*/
{
	int i, need_to_add_to_list, need_update, return_code;

	ENTER(Computed_field_basis_derivative::calculate_FE_element_field_values_for_element);
	if (field&&element)
	{
		return_code=1;
		/* clear the cache if values already cached for a node */
		if (field->node)
		{
			Computed_field_clear_cache(field);
		}
		/* ensure we have FE_element_field_values for element, with
			 derivatives_calculated if requested */
		if ((!fe_element_field_values)||
			(!FE_element_field_values_are_for_element_and_time(
				fe_element_field_values,element,time,top_level_element))||
			(calculate_derivatives&&
				(!FE_element_field_values_have_derivatives_calculated(fe_element_field_values))))
		{
			need_update = 0;
			need_to_add_to_list = 0;
			if (!field_values_cache)
			{
				field_values_cache = CREATE(LIST(FE_element_field_values))();
			}
			fe_element_field_values = FIND_BY_IDENTIFIER_IN_LIST(
				FE_element_field_values, element)(element, field_values_cache);
			if (!fe_element_field_values)
			{
				need_update = 1;
				fe_element_field_values = CREATE(FE_element_field_values)();
				if (fe_element_field_values)
				{
					need_to_add_to_list = 1;
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				if ((!FE_element_field_values_are_for_element_and_time(
						 fe_element_field_values,element,time,top_level_element))||
					(calculate_derivatives&&
						(!FE_element_field_values_have_derivatives_calculated(fe_element_field_values))))
				{
					need_update = 1;
					clear_FE_element_field_values(fe_element_field_values);
				}
			}
			if (return_code && need_update)
			{
				/* note that FE_element_field_values accesses the element */
				if (calculate_FE_element_field_values(element,fe_field,
						time,calculate_derivatives,fe_element_field_values,
						top_level_element))
				{

					for (i = 0 ; i < order ; i++)
					{
						FE_element_field_values_differentiate(fe_element_field_values,
							xi_indices[i]);
					}
					
					if (need_to_add_to_list)
					{
						/* Set a cache size limit */
						if (1000 < NUMBER_IN_LIST(FE_element_field_values)(field_values_cache))
						{
							REMOVE_ALL_OBJECTS_FROM_LIST(FE_element_field_values)
								(field_values_cache);
						}
						return_code = ADD_OBJECT_TO_LIST(FE_element_field_values)(
							fe_element_field_values, field_values_cache);
					}
				}
				else
				{
					/* clear element to indicate that values are clear */
					clear_FE_element_field_values(fe_element_field_values);
					return_code=0;
				}
			 
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_basis_derivative::calculate_FE_element_field_values_for_element.  "
				"Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_basis_derivative::calculate_FE_element_field_values_for_element.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_basis_derivative::calculate_FE_element_field_values_for_element */

int Computed_field_basis_derivative::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	enum Value_type value_type;
	int return_code = 0;

	ENTER(Computed_field_basis_derivative::evaluate_cache_at_location);
	if (field && location)
	{
		Field_element_xi_location *element_xi_location = dynamic_cast<Field_element_xi_location*>(location);
		//Field_node_location *node_location;
		if (element_xi_location)
		{
			FE_element* element = element_xi_location->get_element();
 			FE_element* top_level_element = element_xi_location->get_top_level_element();
			FE_value time = element_xi_location->get_time();
			const FE_value* xi = element_xi_location->get_xi();
			int number_of_derivatives = location->get_number_of_derivatives();

			/* 1. Precalculate any source fields that this field depends on.
				For type COMPUTED_FIELD_BASIS_DERIVATIVE, this means getting
				FE_element_field_values.*/
			return_code=calculate_FE_element_field_values_for_element(
				/*derivatives_required*/1,element,time,top_level_element);
			if (return_code)
			{
				/* 2. Calculate the field */
				value_type=get_FE_field_value_type(fe_field);
				/* component number -1 = calculate all components */
				switch (value_type)
				{
					case FE_VALUE_VALUE:
					case SHORT_VALUE:
					{
						if (number_of_derivatives)
						{
							return_code=calculate_FE_element_field(-1,
								fe_element_field_values,xi,field->values,
								field->derivatives);
							field->derivatives_valid = (0<number_of_derivatives);
						}
						else
						{
							return_code=calculate_FE_element_field(-1,
								fe_element_field_values,xi,field->values,
								(FE_value *)NULL);
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_basis_derivative::evaluate_cache_at_location.  "
							"Unsupported value type %s in basis_derivative field",
							Value_type_string(value_type));
						return_code=0;
					} break;
				}
			}
		}
#if 0
		else 
		{
			 //if (node_location = dynamic_cast<Field_node_location*>(location))
			 if (dynamic_cast<Field_node_location*>(location))
			 {
					display_message(ERROR_MESSAGE,
						 "Computed_field_basis_derivative::evaluate_cache_at_location.  "
						 "This field is valid for elements only.");
			 }
			 else
			 {
					display_message(ERROR_MESSAGE,
						 "Computed_field_basis_derivative::evaluate_cache_at_location.  "
						 "Location type unknown or not implemented.");
			 }
		}
#endif
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_basis_derivative::evaluate_cache_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_basis_derivative::evaluate_cache_at_location */

int Computed_field_basis_derivative::set_values_at_location(
	Field_location* location, FE_value *values)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Sets the <values> of the computed <field> over the <element>.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_basis_derivative::set_values_at_location);
	if (field && location && values)
	{
		display_message(WARNING_MESSAGE,
			"Computed_field_basis_derivative::set_values_at_location.  "
			"The basis derivative is calculated from another field and cannot be set.");
		return_code = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_basis_derivative::set_values_at_location.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_basis_derivative::set_values_at_location */

int Computed_field_basis_derivative::find_element_xi(
	FE_value *values, int number_of_values, struct FE_element **element,
	FE_value *xi, int element_dimension, double time,
	struct Cmiss_region *search_region)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_basis_derivative::find_element_xi);
	if (field && values && (number_of_values == field->number_of_components) &&
		element && xi && search_region)
	{
		return_code = Computed_field_perform_find_element_xi(field,
			values, number_of_values, time, element, xi, element_dimension, search_region,
			/*find_nearest_element*/0);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_basis_derivative::find_element_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_basis_derivative::find_element_xi */

int Computed_field_basis_derivative::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	char *field_name;
	int return_code;
	int i;

	ENTER(List_Computed_field_basis_derivative);
	if (field)
	{
		return_code=GET_NAME(FE_field)(fe_field,&field_name);
		if (return_code)
		{
			display_message(INFORMATION_MESSAGE,"    fe_field : %s\n",field_name);			
			display_message(INFORMATION_MESSAGE,"    order : %d\n",order);
			display_message(INFORMATION_MESSAGE,"    xi_indices : ");
			for (i=0;i<order;i++)
			{
				display_message(INFORMATION_MESSAGE," %d",xi_indices[i]+1);
			}
			display_message(INFORMATION_MESSAGE,"\n");

			DEALLOCATE(field_name);					 
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_basis_derivative.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_basis_derivative */

char *Computed_field_basis_derivative::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error, i;

	ENTER(Computed_field_basis_derivative::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{

		error = 0;
		append_string(&command_string,
			computed_field_basis_derivative_type_string, &error);

		append_string(&command_string, " fe_field ", &error);
		if (GET_NAME(FE_field)(fe_field, &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}

		append_string(&command_string, " order", &error);
		sprintf(temp_string, " %d", order);
		append_string(&command_string, temp_string, &error);

		append_string(&command_string, " xi_indices", &error);
		for (i = 0; i < order; i++)
		{
			sprintf(temp_string, " %d", xi_indices[i]+1);
			append_string(&command_string, temp_string, &error);
		}

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_basis_derivative::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_basis_derivative::get_command_string */

} //namespace

int Computed_field_is_type_basis_derivative(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_basis_derivative);
	if (field)
	{
		if (dynamic_cast<Computed_field_basis_derivative*>(field->core))
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
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_basis_derivative.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_basis_derivative */

/*****************************************************************************//**
 * Creates a field giving arbitrary order derivatives of a finite element field.
 * Modifies the calculated monomial coefficients by differentiating them wrt 
 * to the xi directions in accordance with the vector of <xi_indices> which is
 * length <order>.
 * 
 * @param field_module  Region field module which will own new field.
 * @param fe_field  FE_field to return derivatives w.r.t. xi for.
 * @param order  The order of the derivatives.
 * @param xi_indices  Array of length order giving the xi indices the derivative
 * is calculated with respect to.
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_basis_derivative(
	struct Cmiss_field_module *field_module,
	struct FE_field *fe_field, int order, int *xi_indices)
{
	char **component_names;
	int i, number_of_components, return_code;
	Computed_field *field = NULL;

	ENTER(Computed_field_create_basis_derivative);
	if (fe_field)
	{
		return_code=1;
		number_of_components = get_FE_field_number_of_components(fe_field);
		/* 1. make dynamic allocations for any new type-specific data */
		if (ALLOCATE(component_names, char *, number_of_components))
		{
			for (i = 0 ; i < number_of_components; i++)
			{
				if (!(component_names[i]=get_FE_field_component_name(fe_field,i)))
				{
					return_code = 0;
				}
			}
		}
		else
		{
			return_code = 0;
		}
		if (return_code)
		{
			field = Computed_field_create_generic(field_module,
				/*check_source_field_regions*/true, number_of_components,
				/*number_of_source_fields*/0, NULL,
				/*number_of_source_values*/0, NULL,
				new Computed_field_basis_derivative(
					fe_field, order, xi_indices));
			if (field)
			{
				// following should be a function call
				field->component_names = component_names;
			}
			else
			{
				return_code = 0;
			}
		}
		if ((!return_code) && (component_names))
		{
			for (i = 0 ; i < number_of_components ; i++)
			{
				DEALLOCATE(component_names[i]);
			}
			DEALLOCATE(component_names);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_basis_derivative.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
}

int Computed_field_get_type_basis_derivative(struct Computed_field *field,
	struct FE_field **fe_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_BASIS_DERIVATIVE, the FE_field being
"wrapped" by it is returned - otherwise an error is reported.
==============================================================================*/
{
	Computed_field_basis_derivative* core;
	int return_code;

	ENTER(Computed_field_get_type_basis_derivative);
	if (field&&(core=dynamic_cast<Computed_field_basis_derivative*>(field->core)))
	{
		*fe_field=core->fe_field;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_basis_derivative.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_basis_derivative */

int define_Computed_field_type_basis_derivative(struct Parse_state *state,
	void *field_modify_void,void *computed_field_finite_element_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Creates FE_fields with the given name, coordinate_system, value_type,
cm_field_type, number_of_components and component_names.
The actual finite_element wrapper is not made here but in response to the
FE_field being made and/or modified.
==============================================================================*/
{
	char basis_derivative_help[] =
		"The basis_derivative calculates a monomial derivative on element based fields.  It is not defined for nodes.  It allows you to calculate an arbitrary derivative by specifying an <order> and a list of <xi_indices> of length order.  This derivative then becomes the \"value\" for the field.";
	const char *current_token;
	int i, order, previous_state_index, return_code, *xi_indices, *temp_xi_indices;
	Computed_field_modify_data *field_modify;
	struct FE_field *fe_field;
	struct Option_table *option_table;

	ENTER(define_Computed_field_type_finite_element);
	USE_PARAMETER(computed_field_finite_element_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code = 1;
		order = 1;
		xi_indices = (int *)NULL;
		fe_field = (struct FE_field *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_basis_derivative_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code =
				Computed_field_get_type_basis_derivative(field_modify->get_field(), &fe_field);
		} else {
		}

		/* Assign default values for xi_indices */
		ALLOCATE(xi_indices, int, order);
		for (i = 0 ; i < order ; i++)
		{
			xi_indices[i] = 1;
		}

		if (return_code)
		{
			if (fe_field)
			{
				ACCESS(FE_field)(fe_field);
			}
			/* try to handle help first */
			current_token=state->current_token;
			if (current_token)
			{
				if (!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
				{
					option_table=CREATE(Option_table)();
					Option_table_add_help(option_table, basis_derivative_help);
					/* fe_field */
					Option_table_add_set_FE_field_from_FE_region(
						option_table, "fe_field" ,&fe_field,
					  Cmiss_region_get_FE_region(field_modify->get_region()));
					Option_table_add_int_positive_entry(option_table,
						"order", &order);
					Option_table_add_int_vector_entry(option_table,
						"xi_indices", xi_indices, &order);
					return_code=Option_table_multi_parse(option_table,state);
					DESTROY(Option_table)(&option_table);
				}
			}
			/* parse the fe_field if the "fe_field" token is next */
			if (return_code)
			{
				// store previous state so that we can return to it
				previous_state_index = state->current_index;

				/* parse the order of the differentiation. */
				option_table = CREATE(Option_table)();
				Option_table_add_help(option_table, basis_derivative_help);
				Option_table_add_int_positive_entry(option_table, "order",
					&order);
				/* Ignore all the other entries */
				Option_table_ignore_all_unmatched_entries(option_table);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);				
				/* Return back to where we were */
				shift_Parse_state(state, previous_state_index - state->current_index);
			}
			if (return_code)
			{
				/* Allocate memory for xi_indices array based on order
					 and default all index values to 1 */
				if (REALLOCATE(temp_xi_indices, xi_indices, int, order))
				{
					xi_indices = temp_xi_indices;
					for (i = 0 ; i < order ; i++)
					{
						xi_indices[i] = 1;
					}
				}

				option_table=CREATE(Option_table)();
				/* fe_field */
				Option_table_add_set_FE_field_from_FE_region(
					option_table, "fe_field" ,&fe_field,
					Cmiss_region_get_FE_region(field_modify->get_region()));
				Option_table_add_int_positive_entry(option_table,
					"order", &order);

				Option_table_add_int_vector_entry(option_table,
					"xi_indices", xi_indices, &order);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			if (return_code)
			{
				/* decrement each xi index so that the first index is 0 rather than 1*/
				for (i = 0 ; i < order ; i++)
				{
					xi_indices[i]--;
				}
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_basis_derivative(field_modify->get_field_module(),
						fe_field, order, xi_indices));
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_basis_derivative.  Failed");
				}
			}

			DEALLOCATE(xi_indices);

			if (fe_field)
			{
				DEACCESS(FE_field)(&fe_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_basis_derivative.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_basis_derivative */

struct FE_field_to_Computed_field_change_data
{
	Cmiss_region *cmiss_region;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct FE_region_changes *changes;
};

/***************************************************************************//**
 * Ensures there is an up-to-date computed field wrapper for <fe_field>.
 * @param fe_field  Field to wrap.
 * @param field_change_data_void FE_field_to_Computed_field_change_data. 
 */
int FE_field_to_Computed_field_change(struct FE_field *fe_field,
	void *field_change_data_void)
{
	int change;
	int return_code;
	struct FE_field_to_Computed_field_change_data *field_change_data;

	ENTER(FE_field_to_Computed_field_change);
	if (fe_field && (field_change_data =
		(struct FE_field_to_Computed_field_change_data *)field_change_data_void))
	{
		if (CHANGE_LOG_QUERY(FE_field)(field_change_data->changes->fe_field_changes,
			fe_field, &change))
		{
			return_code = 1;
			if (change &
				(CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_field) |
				 CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field) |
				 CHANGE_LOG_OBJECT_ADDED(FE_field)))
			{
				Cmiss_field_module *field_module =
					Cmiss_field_module_create(field_change_data->cmiss_region);
				struct Computed_field *existing_wrapper =
					FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
						Computed_field_wraps_fe_field, (void *)fe_field,
						field_change_data->computed_field_manager);
				if (existing_wrapper)
				{
					Cmiss_field_module_set_replace_field(field_module, existing_wrapper);
				}
				else
				{
					char *field_name = NULL;
					if (GET_NAME(FE_field)(fe_field, &field_name))
					{
						Cmiss_field_module_set_field_name(field_module, field_name);
						DEALLOCATE(field_name);
					}
					struct Coordinate_system *coordinate_system =
						get_FE_field_coordinate_system(fe_field);
					if (coordinate_system)
					{
						Cmiss_field_module_set_coordinate_system(field_module, *coordinate_system);
					}
				}
				Computed_field *field =
					Computed_field_create_finite_element_internal(field_module, fe_field);
				Cmiss_field_set_attribute_integer(field, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 1);
				Cmiss_field_destroy(&field);
				Cmiss_field_module_destroy(&field_module);
			}
		}
		else
		{
			return_code = 0;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"FE_field_to_Computed_field_change.  Could not propagate change(s)");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_to_Computed_field_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_to_Computed_field_change */

void Cmiss_region_FE_region_change(struct FE_region *fe_region,
	struct FE_region_changes *changes, void *cmiss_region_void)
{
	ENTER(Cmiss_region_FE_region_change);
	Cmiss_region *cmiss_region = reinterpret_cast<Cmiss_region *>(cmiss_region_void);
	if (fe_region && changes && cmiss_region)
	{
		int field_change_summary;
		CHANGE_LOG_GET_CHANGE_SUMMARY(FE_field)(changes->fe_field_changes,
			&field_change_summary);
		int check_field_wrappers =
			(field_change_summary & CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_field)) || 
			(field_change_summary & CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field)) || 
			(field_change_summary & CHANGE_LOG_OBJECT_ADDED(FE_field));
		int add_cmiss_number_field = FE_region_need_add_cmiss_number_field(fe_region);
		int add_xi_field = FE_region_need_add_xi_field(fe_region);
		if (check_field_wrappers || add_cmiss_number_field || add_xi_field)
		{
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(cmiss_region);
			Cmiss_field_module_begin_change(field_module);

			if (check_field_wrappers)
			{
				struct FE_field_to_Computed_field_change_data field_change_data;
				field_change_data.cmiss_region = cmiss_region;
				field_change_data.computed_field_manager = Cmiss_region_get_Computed_field_manager(cmiss_region);
				field_change_data.changes = changes;
				FE_region_for_each_FE_field(fe_region,
					FE_field_to_Computed_field_change, (void *)&field_change_data);
			}
			if (add_cmiss_number_field)
			{
				const char *cmiss_number_field_name = "cmiss_number";
				Cmiss_field_id field = Cmiss_field_module_find_field_by_name(field_module, cmiss_number_field_name);
				if (!field)
				{
					field = Computed_field_create_cmiss_number(field_module);
					Cmiss_field_set_name(field, cmiss_number_field_name);
					Cmiss_field_set_attribute_integer(field, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 1);
				}
				Cmiss_field_destroy(&field);
			}
			if (add_xi_field)
			{
				const char *xi_field_name = "xi";
				Cmiss_field_id field = Cmiss_field_module_find_field_by_name(field_module, xi_field_name);
				if (!field)
				{
					field = Computed_field_create_xi_coordinates(field_module);
					Cmiss_field_set_name(field, xi_field_name);
					Cmiss_field_set_attribute_integer(field, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 1);
				}
				Cmiss_field_destroy(&field);
			}
			Cmiss_field_module_end_change(field_module);
			Cmiss_field_module_destroy(&field_module);
		}
		if (field_change_summary & CHANGE_LOG_OBJECT_REMOVED(FE_field))
		{
			/* Currently we do nothing as the computed field wrapper is destroyed
				before the FE_field is removed from the manager.  This is not necessary
				and this response could be to delete the wrapper. */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_FE_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Cmiss_region_FE_region_change */

int Computed_field_contains_changed_FE_field(
	struct Computed_field *field, void *fe_field_change_log_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns true if <field> directly contains an FE_field and it is listed as
changed, added or removed in <fe_field_change_log>.
<fe_field_change_log_void> must point at a struct CHANGE_LOG<FE_field>.
==============================================================================*/
{
	int fe_field_change;
	enum FE_nodal_value_type nodal_value_type;
	int return_code, version_number;
	struct CHANGE_LOG(FE_field) *fe_field_change_log;
	struct FE_field *fe_field;

	ENTER(Computed_field_contains_changed_FE_field);
	if (field && (fe_field_change_log =
		(struct CHANGE_LOG(FE_field) *)fe_field_change_log_void))
	{
		if (dynamic_cast<Computed_field_finite_element*>(field->core))
		{
			return_code = Computed_field_get_type_finite_element(field, &fe_field);
		}
		else if (dynamic_cast<Computed_field_node_value*>(field->core))
		{
			return_code = Computed_field_get_type_node_value(field, &fe_field,
				&nodal_value_type, &version_number);
		}
		else
		{
			return_code = 0;
		}
		if (return_code)
		{
			return_code = CHANGE_LOG_QUERY(FE_field)(fe_field_change_log, fe_field,
				&fe_field_change) &&
				(fe_field_change != CHANGE_LOG_OBJECT_UNCHANGED(FE_field));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_contains_changed_FE_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_contains_changed_FE_field */

int Computed_field_add_source_FE_field_to_list(
	struct Computed_field *field, void *fe_field_list_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If <field> has a source FE_field, ensures it is in <fe_field_list>.
==============================================================================*/
{
	enum FE_nodal_value_type nodal_value_type;
	int return_code, version_number;
	struct FE_field *fe_field;
	struct LIST(FE_field) *fe_field_list;

	ENTER(Computed_field_add_source_FE_field_to_list);
	if (field && (fe_field_list = (struct LIST(FE_field) *)fe_field_list_void))
	{
		fe_field = (struct FE_field *)NULL;
		if (dynamic_cast<Computed_field_finite_element*>(field->core))
		{
			return_code = Computed_field_get_type_finite_element(field, &fe_field);
		}
		else if (dynamic_cast<Computed_field_node_value*>(field->core))
		{
			return_code = Computed_field_get_type_node_value(field, &fe_field,
				&nodal_value_type, &version_number);
		}
		else
		{
			return_code = 1;
		}
		if (return_code && fe_field)
		{
			if (!IS_OBJECT_IN_LIST(FE_field)(fe_field, fe_field_list))
			{
				return_code = ADD_OBJECT_TO_LIST(FE_field)(fe_field, fe_field_list);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_add_source_FE_field_to_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_add_source_FE_field_to_list */

struct LIST(FE_field)
	*Computed_field_get_defining_FE_field_list(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns the list of FE_fields that <field> depends on.
==============================================================================*/
{
	struct LIST(FE_field) *fe_field_list;

	ENTER(Computed_field_get_defining_FE_field_list);
	fe_field_list = (struct LIST(FE_field) *)NULL;
	if (field)
	{
		fe_field_list = CREATE(LIST(FE_field))();
		if (fe_field_list)
		{
			if (!Computed_field_for_each_ancestor(field,
				Computed_field_add_source_FE_field_to_list, (void *)fe_field_list))
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_get_defining_FE_field_list.  Failed");
				DESTROY(LIST(FE_field))(&fe_field_list);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_defining_FE_field_list.  Invalid argument(s)");
	}
	LEAVE;

	return (fe_field_list);
} /* Computed_field_get_defining_FE_field_list */

struct LIST(FE_field)
	*Computed_field_array_get_defining_FE_field_list(
		int number_of_fields, struct Computed_field **field_array)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns the compiled list of FE_fields that are required by any of
the <number_of_fields> fields in <field_array>.
==============================================================================*/
{
	int i;
	struct LIST(FE_field) *additional_fe_field_list, *fe_field_list;

	ENTER(Computed_field_get_defining_FE_field_list);
	fe_field_list = (struct LIST(FE_field) *)NULL;
	if ((0 < number_of_fields) && field_array)
	{
		fe_field_list = Computed_field_get_defining_FE_field_list(field_array[0]);
		for (i = 1 ; i < number_of_fields ; i++)
		{
			additional_fe_field_list = Computed_field_get_defining_FE_field_list(field_array[i]);
			FOR_EACH_OBJECT_IN_LIST(FE_field)(ensure_FE_field_is_in_list,
				(void *)fe_field_list, additional_fe_field_list);
			DESTROY(LIST(FE_field))(&additional_fe_field_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_array_get_defining_FE_field_list.  Invalid argument(s)");
	}
	LEAVE;

	return (fe_field_list);
} /* Computed_field_array_get_defining_FE_field_list */

int Computed_field_is_type_finite_element_iterator(
	struct Computed_field *field, void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 16 March 2007

DESCRIPTION :
Iterator/conditional function returning true if <field> is read only and a
wrapper for an FE_field.
==============================================================================*/
{
	int return_code;

	USE_PARAMETER(dummy_void);
	ENTER(Computed_field_is_type_finite_element_iterator);
	if (field)
	{
		return_code = Computed_field_is_type_finite_element(field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_finite_element_iterator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_finite_element_iterator */

int Computed_field_wraps_fe_field(
	struct Computed_field *field, void *fe_field_void)
{
	int return_code = 0;

	ENTER(Computed_field_wraps_fe_field);
	struct FE_field *fe_field = (struct FE_field *)fe_field_void;
	if (field && fe_field)
	{
		Computed_field_finite_element* core =
			dynamic_cast<Computed_field_finite_element*>(field->core);
		if (core)
		{
			return_code = (fe_field == core->fe_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_wraps_fe_field.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
}

int Computed_field_has_coordinate_fe_field(struct Computed_field *field,
	void *dummy)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Iterator/conditional function returning true if <field> is a wrapper for a
coordinate type fe_field.
==============================================================================*/
{
	Computed_field_finite_element* core;
	int return_code;

	ENTER(Computed_field_has_coordinate_fe_field);
	USE_PARAMETER(dummy);
	if (field)
	{
		return_code = 0;
		core = dynamic_cast<Computed_field_finite_element*>(field->core);
		if (core)
		{
			return_code = FE_field_is_coordinate_field(core->fe_field, NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_coordinate_fe_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_has_coordinate_fe_field */

int Computed_field_has_element_xi_fe_field(struct Computed_field *field,
	void *dummy)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Iterator/conditional function returning true if <field> is a wrapper for an
element_xi type fe_field.
==============================================================================*/
{
	Computed_field_finite_element* core;
	int return_code;

	ENTER(Computed_field_has_element_xi_fe_field);
	USE_PARAMETER(dummy);
	if (field)
	{
		return_code = 0;
		core = dynamic_cast<Computed_field_finite_element*>(field->core);
		if (core)
		{
			enum Value_type field_value_type = get_FE_field_value_type(core->fe_field);
			return_code = (field_value_type == ELEMENT_XI_VALUE);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_element_xi_fe_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_has_element_xi_fe_field */

int Computed_field_is_scalar_integer(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns true if <field> is a 1 integer component FINITE_ELEMENT wrapper.
==============================================================================*/
{
	Computed_field_finite_element* core;
	int return_code;

	ENTER(Computed_field_is_scalar_integer);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		if((1==field->number_of_components)&&
			(core = dynamic_cast<Computed_field_finite_element*>(field->core)))
		{
			return_code = (INT_VALUE==get_FE_field_value_type(core->fe_field));
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_scalar_integer.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_scalar_integer */

int Computed_field_is_scalar_integer_grid_in_element(
	struct Computed_field *field,void *element_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns true if <field> is a 1 integer component FINITE_ELEMENT wrapper which
is defined in <element> AND is grid-based.
Used for choosing field suitable for identifying grid points.
==============================================================================*/
{
	Computed_field_finite_element* core;
	int return_code;
	struct FE_element *element;

	ENTER(Computed_field_is_scalar_integer_grid_in_element);
	if (field&&(element=(struct FE_element *)element_void))
	{
		if ((1==field->number_of_components)&&
			(core=dynamic_cast<Computed_field_finite_element*>(field->core)))
		{
			return_code = (INT_VALUE==get_FE_field_value_type(core->fe_field))&&
				Computed_field_is_defined_in_element(field,element)&&
				FE_element_field_is_grid_based(element,core->fe_field);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_scalar_integer_grid_in_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_scalar_integer_grid_in_element */

int Computed_field_has_string_value_type(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns true if <field> is of string value type.
Currently only FE_fields can return string value type, hence this function is
restricted to this module.
Eventually, other computed fields will be of string type and this function will
belong elsewhere.
Note: not returning possible true result for embedded field as evaluate
at node function does not allow for string value either.
==============================================================================*/
{
	enum FE_nodal_value_type nodal_value_type;
	int return_code;
	int version_number;
	struct FE_field *fe_field;

	ENTER(Computed_field_has_string_value_type);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		fe_field = (struct FE_field *)NULL;
		if (dynamic_cast<Computed_field_finite_element*>(field->core))
		{
			Computed_field_get_type_finite_element(field, &fe_field);
		}
		else if (dynamic_cast<Computed_field_node_value*>(field->core))
		{
			Computed_field_get_type_node_value(field, &fe_field,
				&nodal_value_type, &version_number);
		}
		return_code = ((struct FE_field *)NULL != fe_field) &&
			(STRING_VALUE == get_FE_field_value_type(fe_field));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_string_value_type.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_has_string_value_type */

Computed_field_finite_element_package *
Computed_field_register_types_finite_element(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 24 July 2008

DESCRIPTION :
This function registers the finite_element related types of Computed_fields.
==============================================================================*/
{
	Computed_field_finite_element_package *return_ptr = 0;
	Computed_field_finite_element_package 
		*computed_field_finite_element_package = 
		new Computed_field_finite_element_package;

	ENTER(Computed_field_register_types_finite_element);
	if (computed_field_package)
	{
		Computed_field_package_add_type(computed_field_package,
			computed_field_finite_element_type_string,
			define_Computed_field_type_finite_element,
			computed_field_finite_element_package);
		Computed_field_package_add_type(computed_field_package,
			computed_field_cmiss_number_type_string,
			define_Computed_field_type_cmiss_number,
			computed_field_finite_element_package);
#if defined (COMPUTED_FIELD_ACCESS_COUNT)
		Computed_field_package_add_type(computed_field_package,
			computed_field_access_count_type_string,
			define_Computed_field_type_access_count,
			computed_field_finite_element_package);
#endif /* defined (COMPUTED_FIELD_ACCESS_COUNT) */
		Computed_field_package_add_type(computed_field_package,
			computed_field_xi_coordinates_type_string,
			define_Computed_field_type_xi_coordinates,
			computed_field_finite_element_package);
		Computed_field_package_add_type(computed_field_package,
			computed_field_node_value_type_string,
			define_Computed_field_type_node_value,
			computed_field_finite_element_package);
		Computed_field_package_add_type(computed_field_package,
			computed_field_find_mesh_location_type_string,
			define_Computed_field_type_find_mesh_location,
			computed_field_finite_element_package);
		Computed_field_package_add_type(computed_field_package,
			computed_field_embedded_type_string,
			define_Computed_field_type_embedded,
			computed_field_finite_element_package);
		Computed_field_package_add_type(computed_field_package,
			computed_field_basis_derivative_type_string,
			define_Computed_field_type_basis_derivative,
			computed_field_finite_element_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_finite_element.  Invalid argument(s)");
		return_ptr = (Computed_field_finite_element_package *)NULL;
	}
	LEAVE;

	return (return_ptr);
} /* Computed_field_register_types_finite_element */


int Computed_field_get_FE_field_time_array_index_at_FE_value_time(
	 struct Computed_field *field,FE_value time, FE_value *the_time_high,
	 FE_value *the_time_low, int *the_array_index,int *the_index_high,
	 int *the_index_low)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Given a <field> and <time>, checks that <field> has times defined and returns:
<the_array_index>, the array index of <field> times closest to <time>.
<the_index_high>, <the_index_low> the upper and lower limits for <the_array_index>
(ideally <the_index_high>==<the_index_low>==<the_array_index>).
<the_time_low> the time corresponding to <the_index_low>.
<the_time_high> the time corresponding to <the_index_high>.

All this information (rather than just <the_array_index> ) is returned so can
perform interpolation, etc.
==============================================================================*/
{
	Computed_field_finite_element* core;
	int return_code;

	ENTER(Computed_field_get_FE_field_time_array_index_at_FE_value_time);
	if (field)
	{
		core=dynamic_cast<Computed_field_finite_element*>(field->core);
		if (core)
		{
			 return_code = get_FE_field_time_array_index_at_FE_value_time(
					core->fe_field, time, the_time_high, the_time_low, the_array_index,
					the_index_high, the_index_low);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_FE_field_time_array_index_at_FE_value_time.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_scalar_integer_grid_in_element */

struct FE_time_sequence *Computed_field_get_FE_node_field_FE_time_sequence(
	 struct Computed_field *field, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 22 Feb 2008

DESCRIPTION :
Returns the <fe_time_sequence> corresponding to the <node> and <field>.  If the
<node> and <field> have no time dependence then the function will return NULL.
==============================================================================*/
{
	 FE_time_sequence *time_sequence;
	 FE_field *fe_field;
	 struct LIST(FE_field) *fe_field_list;

	 ENTER(Computed_field_get_FE_node_field_FE_time_sequence);
	 time_sequence = (FE_time_sequence *)NULL;
	 if (field)
	 {
			fe_field_list = Computed_field_get_defining_FE_field_list(field);
			if (fe_field_list)
			{
				 if (NUMBER_IN_LIST(FE_field)(fe_field_list) == 1)
				 {
						fe_field = FIRST_OBJECT_IN_LIST_THAT(FE_field)(
							 (LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL, (void *)NULL,
							 fe_field_list);
						time_sequence = get_FE_node_field_FE_time_sequence(node,
							 fe_field);
				 }
				 else
				 {
						display_message(ERROR_MESSAGE,
							 "Computed_field_get_FE_node_field_FE_time_sequence. None or"
							 "more than one FE element field is used to define this" 
							 "computed field, this function expects only one finite element"
							 "field at the corresponding node otherwise it may contain more than"
							 "one time sequence./n");
				 }
				 DESTROY(LIST(FE_field))(&fe_field_list);
			}
			else
			{
						display_message(ERROR_MESSAGE,
							 "Computed_field_get_FE_node_field_FE_time_sequence. Cannot get the"
							 "FE field list /n");
			}
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"Computed_field_get_FE_node_field_FE_time_sequence.  Invalid argument(s)");
		 time_sequence = (FE_time_sequence *)NULL;
	}
	LEAVE;

	return (time_sequence);
} /* Computed_field_get_FE_node_field_FE_time_sequence */
