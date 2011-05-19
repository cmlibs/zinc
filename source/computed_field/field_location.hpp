//******************************************************************************
// FILE : field_location.hpp
//
// LAST MODIFIED : 9 August 2006
//
// DESCRIPTION :
// An class hierarchy for specifying locations at which to evaluate fields.
// These are transient objects used internally in Computed_fields and so do not
// access, copy or allocate their members.
//==============================================================================
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
 * Portions created by the Initial Developer are Copyright (C) 2006
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
#if !defined (__FIELD_LOCATION_HPP__)
#define __FIELD_LOCATION_HPP__

extern "C" {
#include "computed_field/computed_field.h"
#include "general/value.h"
}

struct Cmiss_field;

class Field_location
{
protected:
	FE_value time;
	int number_of_derivatives;
	int assign_to_cache; // if set, assignment is to cache only, not to field

   Field_location(FE_value time = 0.0, int number_of_derivatives = 0) : 
		time(time), number_of_derivatives(number_of_derivatives),
		assign_to_cache(0)
	{};

public:

   /* Abstract virtual destructor declaration as we will not make objects of this
		parent class */
	virtual ~Field_location() = 0;

	FE_value get_time()
	{
		return time;
	}

	void set_time(FE_value new_time)
	{
		time = new_time;
	}

	int get_number_of_derivatives()
	{
		return number_of_derivatives;
	}

	int get_assign_to_cache() const
	{
		return assign_to_cache;
	}

	void set_assign_to_cache(int assign_to_cache_in)
	{
		assign_to_cache = (assign_to_cache_in != 0);
	}

	virtual int check_cache_for_location(Cmiss_field * /*field*/)
	{
		/* Default is that the cache is invalid */
		return 0;
	}

	virtual int update_cache_for_location(Cmiss_field * /*field*/)
	{
		/* Don't need to do anything */
		return 1;
	}

	virtual int set_values_for_location(Cmiss_field * /*field*/,
		FE_value * /*values*/)
	{
		/* Default is that the location can't set the values */
		return 0;
	}
};

class Field_element_xi_location : public Field_location
{
private:
	struct FE_element *element;
	int dimension;
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct FE_element *top_level_element;

public:
	Field_element_xi_location(struct FE_element *element_in,
			const FE_value *xi_in = NULL, FE_value time_in = 0.0,
			struct FE_element *top_level_element_in = NULL, int number_of_derivatives_in = 0) :
		Field_location(time_in, number_of_derivatives_in),
		element(element_in ? ACCESS(FE_element)(element_in) : 0),
		dimension(element_in ? get_FE_element_dimension(element_in) : 0),
		top_level_element(top_level_element_in ? ACCESS(FE_element)(top_level_element_in) : 0)
	{
		if (xi_in)
		{
			for (int i = 0; i < dimension; i++)
			{
				xi[i] = xi_in[i];
			}
		}
		else
		{
			for (int i = 0; i < dimension; i++)
			{
				xi[i] = 0.0;
			}
		}
	}

	// blank constructor - caller should call set_element_xi & check valid return
	Field_element_xi_location(FE_value time_in = 0.0, int number_of_derivatives_in = 0) :
		Field_location(time_in, number_of_derivatives_in),
		element(0),
		dimension(0),
		top_level_element(0)
	{
	}
	
   ~Field_element_xi_location()
	{
		DEACCESS(FE_element)(&element);
		if (top_level_element)
			DEACCESS(FE_element)(&top_level_element);
	}

	int get_dimension() const
	{
		return dimension;
	}

	struct FE_element *get_element()
	{
		return element;
	}

	const FE_value *get_xi() const
	{
		return xi;
	}

	FE_element *get_top_level_element()
	{
		return top_level_element;
	}

	int set_element_xi(struct FE_element *element_in,
		int number_of_xi_in, const FE_value *xi_in,
		struct FE_element *top_level_element_in = NULL);

	/** use with care in field evaluation & restore after evaluating with */
	void set_number_of_derivatives(int number_of_derivatives_in)
	{
		number_of_derivatives = number_of_derivatives_in;
	}

	int check_cache_for_location(Cmiss_field *field);

	int update_cache_for_location(Cmiss_field *field);
};

class Field_node_location : public Field_location
{
private:
	struct FE_node *node;

public:
	Field_node_location(struct FE_node *node,
		FE_value time = 0, int number_of_derivatives = 0):
		Field_location(time, number_of_derivatives),
		node(ACCESS(FE_node)(node))
	{
	}
	
   ~Field_node_location()
	{
   	DEACCESS(FE_node)(&node);
	}

	FE_node *get_node()
	{
		return node;
	}

	void set_node(FE_node *node_in)
	{
		REACCESS(FE_node)(&node, node_in);
	}

	int check_cache_for_location(Cmiss_field *field);

	int update_cache_for_location(Cmiss_field *field);
};

class Field_time_location : public Field_location
{
public:
	Field_time_location(FE_value time = 0, int number_of_derivatives = 0):
		Field_location(time, number_of_derivatives)
	{
	}

   ~Field_time_location()
	{
	}

	int check_cache_for_location(Cmiss_field *field);

	int update_cache_for_location(Cmiss_field *field);
};

class Field_coordinate_location : public Field_location
{
private:
	Cmiss_field *reference_field;
	int number_of_values;
	FE_value *values;
	FE_value *derivatives;

public:
	Field_coordinate_location(Cmiss_field *reference_field_in,
		int number_of_values_in, const FE_value* values_in, FE_value time = 0,
		int number_of_derivatives_in = 0, const FE_value* derivatives_in = NULL);
	
	// blank constructor - caller should call set_field_values & check valid return
	Field_coordinate_location(FE_value time = 0) :
		Field_location(time),
		reference_field(0),
		number_of_values(0),
		values(0),
		derivatives(0)
	{
	}

	~Field_coordinate_location();

	Cmiss_field *get_reference_field()
	{
		return reference_field;
	}

	int get_number_of_values()
	{
		return number_of_values;
	}

	FE_value *get_values()
	{
		return values;
	}

	int set_field_values(Cmiss_field_id reference_field_in,
		int number_of_values_in, FE_value *values_in);

	int check_cache_for_location(Cmiss_field *field);

	int update_cache_for_location(Cmiss_field *field);

	int set_values_for_location(Cmiss_field *field,
		FE_value *values);

};

#endif /* !defined (__FIELD_LOCATION_HPP__) */
