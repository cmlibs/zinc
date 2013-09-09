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
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (__FIELD_LOCATION_HPP__)
#define __FIELD_LOCATION_HPP__

#include "computed_field/computed_field.h"
#include "general/value.h"

struct cmzn_field;

class Field_location
{
protected:
	FE_value time;
	int number_of_derivatives;

	Field_location(FE_value time = 0.0, int number_of_derivatives = 0) :
		time(time), number_of_derivatives(number_of_derivatives)
	{}

public:

	/* Abstract virtual destructor declaration as we will not make objects of this
		parent class */
	virtual ~Field_location() = 0;

	virtual Field_location *clone() = 0;

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

	/** use with care in field evaluation & restore after evaluating with */
	void set_number_of_derivatives(int number_of_derivatives_in)
	{
		number_of_derivatives = number_of_derivatives_in;
	}

	virtual int set_values_for_location(cmzn_field * /*field*/,
		const FE_value * /*values*/)
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

	virtual Field_location *clone()
	{
		return new Field_element_xi_location(element, xi, time, top_level_element);
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

	virtual Field_location *clone()
	{
		return new Field_node_location(node, time);
	}

	FE_node *get_node()
	{
		return node;
	}

	void set_node(FE_node *node_in)
	{
		REACCESS(FE_node)(&node, node_in);
	}
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

	virtual Field_location *clone()
	{
		return new Field_time_location(time);
	}
};

class Field_coordinate_location : public Field_location
{
private:
	cmzn_field *reference_field;
	int number_of_values;
	FE_value *values;
	FE_value *derivatives;

public:
	Field_coordinate_location(cmzn_field *reference_field_in,
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

	virtual Field_location *clone()
	{
		return new Field_coordinate_location(reference_field, number_of_values, values, time, number_of_derivatives, derivatives);
	}

	cmzn_field *get_reference_field()
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

	int set_field_values(cmzn_field_id reference_field_in,
		int number_of_values_in, const FE_value *values_in);

	int set_values_for_location(cmzn_field *field,
		const FE_value *values);

};

#endif /* !defined (__FIELD_LOCATION_HPP__) */
