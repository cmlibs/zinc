/*******************************************************************************
FILE : api/cmiss_function_finite_element.cpp

LAST MODIFIED : 4 November 2004

DESCRIPTION :
The public interface to the Cmiss_function_element, Cmiss_function_element_xi
and Cmiss_function_finite_element objects.
==============================================================================*/

#include <new>
extern "C" {
#include "finite_element/finite_element_region.h"
}
#include "api/cmiss_function_finite_element.h"
#include "api/cmiss_region.h"
#include "computed_variable/function_finite_element.hpp"

/*
Global functions
----------------
*/

Cmiss_function_id Cmiss_function_element_create(Cmiss_element_id element)
/*******************************************************************************
LAST MODIFIED : 3 May 2004

DESCRIPTION :
Creates a Cmiss_function which represents the <element>.
==============================================================================*/
{
	return (reinterpret_cast<Cmiss_function_id>(new Function_handle(
		new Function_element(element))));
}

int Cmiss_function_element_dimension(Cmiss_function_id function_element,
	unsigned int *dimension_address)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Gets the <*dimension_address> of the <function_element>.  Returns a non-zero for
success.
==============================================================================*/
{
	int return_code;
	Function_element_handle *function_element_handle_address;

	return_code=0;
	if (dimension_address&&(function_element_handle_address=
		reinterpret_cast<Function_element_handle *>(function_element))&&
		(*function_element_handle_address))
	{
		return_code=1;
		*dimension_address=((*function_element_handle_address)->dimension)();
	}

	return (return_code);
}

int Cmiss_function_element_element_value(Cmiss_function_id function_element,
	Cmiss_element_id *element_address)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Gets the <*element_address> of the <function_element>.  Returns a non-zero for
success.

NB.  The calling program should use ACCESS(Cmiss_element) and
DEACCESS(Cmiss_element) to manage the lifetime of the returned element
==============================================================================*/
{
	int return_code;
	Function_element_handle *function_element_handle_address;

	return_code=0;
	if (element_address&&(function_element_handle_address=
		reinterpret_cast<Function_element_handle *>(function_element))&&
		(*function_element_handle_address))
	{
		return_code=1;
		*element_address=((*function_element_handle_address)->element_value)();
	}

	return (return_code);
}

Cmiss_function_id Cmiss_function_element_xi_create(Cmiss_element_id element,
	unsigned int number_of_xi,Scalar *xi)
/*******************************************************************************
LAST MODIFIED : 5 May 2004

DESCRIPTION :
Creates a Cmiss_function which represents the <element>/<xi> location.  The
number of values in <xi>, <number_of_xi> and the dimension of <element> should
be equal.
==============================================================================*/
{
	Cmiss_function_id result;

	result=0;
	if (element)
	{
		if ((number_of_xi==(unsigned int)get_FE_element_dimension(element))&&
			(0==number_of_xi)||((0<number_of_xi)&&xi))
		{
			Function_size_type i;
			Vector xi_vector(number_of_xi);

			for (i=0;i<number_of_xi;i++)
			{
				xi_vector[i]=xi[i];
			}
			result=reinterpret_cast<Cmiss_function_id>(new Function_handle(
				new Function_element_xi(element,xi_vector)));
		}
	}

	return (result);
}

Cmiss_function_variable_id Cmiss_function_element_xi_element(
	Cmiss_function_id function_element_xi)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Returns a variable that refers to the element part of the <function_element_xi>.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	Function_element_xi_handle *function_element_xi_handle_address;

	result=0;
	if ((function_element_xi_handle_address=
		reinterpret_cast<Function_element_xi_handle *>(function_element_xi))&&
		(*function_element_xi_handle_address))
	{
		result=reinterpret_cast<Cmiss_function_variable_id>(
			new Function_variable_handle(((*function_element_xi_handle_address)->
			element)()));
	}

	return (result);
}

Cmiss_function_variable_id Cmiss_function_element_xi_xi(
	Cmiss_function_id function_element_xi)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Returns a variable that refers to the xi part of the <function_element_xi>.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	Function_element_xi_handle *function_element_xi_handle_address;

	result=0;
	if ((function_element_xi_handle_address=
		reinterpret_cast<Function_element_xi_handle *>(function_element_xi))&&
		(*function_element_xi_handle_address))
	{
		result=reinterpret_cast<Cmiss_function_variable_id>(
			new Function_variable_handle(((*function_element_xi_handle_address)->
			xi)()));
	}

	return (result);
}

Cmiss_function_variable_id Cmiss_function_element_xi_xi_entry(
	Cmiss_function_id function_element_xi,unsigned int index)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Returns a variable that refers to the xi entry (<index>) of the
<function_element_xi>.  <index> number 1 is the first entry.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	Function_element_xi_handle *function_element_xi_handle_address;

	result=0;
	if ((0<index)&&(function_element_xi_handle_address=
		reinterpret_cast<Function_element_xi_handle *>(function_element_xi))&&
		(*function_element_xi_handle_address))
	{
		result=reinterpret_cast<Cmiss_function_variable_id>(
			new Function_variable_handle(((*function_element_xi_handle_address)->xi)(
			index)));
	}

	return (result);
}

int Cmiss_function_element_xi_number_of_xi(
	Cmiss_function_id function_element_xi,unsigned int *number_of_xi_address)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Gets the <*number_of_xi_address> of the <function_element_xi>.  Returns a
non-zero for success.
==============================================================================*/
{
	int return_code;
	Function_element_xi_handle *function_element_xi_handle_address;

	return_code=0;
	if (number_of_xi_address&&(function_element_xi_handle_address=
		reinterpret_cast<Function_element_xi_handle *>(function_element_xi))&&
		(*function_element_xi_handle_address))
	{
		return_code=1;
		*number_of_xi_address=((*function_element_xi_handle_address)->
			number_of_xi)();
	}

	return (return_code);
}

int Cmiss_function_element_xi_element_value(
	Cmiss_function_id function_element_xi,Cmiss_element_id *element_address)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Gets the <*element_address> of the <function_element_xi>.  Returns a non-zero
for success.

NB.  The calling program should use ACCESS(Cmiss_element) and
DEACCESS(Cmiss_element) to manage the lifetime of the returned element
==============================================================================*/
{
	int return_code;
	Function_element_xi_handle *function_element_xi_handle_address;

	return_code=0;
	if (element_address&&(function_element_xi_handle_address=
		reinterpret_cast<Function_element_xi_handle *>(function_element_xi))&&
		(*function_element_xi_handle_address))
	{
		return_code=1;
		*element_address=((*function_element_xi_handle_address)->element_value)();
	}

	return (return_code);
}

int Cmiss_function_element_xi_xi_value(Cmiss_function_id function_element_xi,
	unsigned int index,Scalar *value_address)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Gets the <*value_address> for the specified xi entry (<index>) of the
<function_element_xi>.  <index> number 1 is the first entry.  Returns a non-zero
for success.
==============================================================================*/
{
	int return_code;
	Function_element_xi_handle *function_element_xi_handle_address;

	return_code=0;
	if ((0<index)&&value_address&&(function_element_xi_handle_address=
		reinterpret_cast<Function_element_xi_handle *>(function_element_xi))&&
		(*function_element_xi_handle_address))
	{
		return_code=1;
		*value_address=((*function_element_xi_handle_address)->xi_value)(index);
	}

	return (return_code);
}

Cmiss_function_id Cmiss_function_finite_element_create(Cmiss_region_id region,
	char *field)
/*******************************************************************************
LAST MODIFIED : 4 November 2004

DESCRIPTION :
Creates a Cmiss_function which represents the <field> in <region>.
==============================================================================*/
{
	Cmiss_function_id result;
	struct FE_field *fe_field;
	struct FE_region *fe_region;

	result=0;
	if (region&&field)
	{
		if ((fe_region=Cmiss_region_get_FE_region(region))&&
			(fe_field=FE_region_get_FE_field_from_name(fe_region,field)))
		{
			result=reinterpret_cast<Cmiss_function_id>(new Function_handle(
				new Function_finite_element(fe_field)));
		}
	}

	return (result);
}

Cmiss_function_variable_id Cmiss_function_finite_element_component(
	Cmiss_function_id function_finite_element,char *name,unsigned int number)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Returns a variable that refers to a component of the <function_finite_element>.
If <name> is not NULL, then the component with the <name> is specified.  If
<name> is NULL, then the component with the <number> is specified.  Component
<number> 1 is the first component.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	Function_finite_element_handle *function_finite_element_handle_address;

	result=0;
	if ((function_finite_element_handle_address=
		reinterpret_cast<Function_finite_element_handle *>(
		function_finite_element))&&(*function_finite_element_handle_address))
	{
		if (name)
		{
			result=reinterpret_cast<Cmiss_function_variable_id>(
				new Function_variable_handle(
				((*function_finite_element_handle_address)->component)(name)));
		}
		else
		{
			if (0<number)
			{
				result=reinterpret_cast<Cmiss_function_variable_id>(
					new Function_variable_handle(
					((*function_finite_element_handle_address)->component)(number)));
			}
		}
	}

	return (result);
}

Cmiss_function_variable_id Cmiss_function_finite_element_nodal_values(
	Cmiss_function_id function_finite_element,char *component_name,
	unsigned int component_number,Cmiss_node_id node,
	enum FE_nodal_value_type value_type,unsigned int version)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Returns a variable that refers to a subset of the nodal values for the
<function_finite_element>.

If <component_name> is not NULL, then the nodal values must be for the named
component.  If <component_name> is NULL and <component_number> is not zero then
the nodal values must be for that number component.

If <node> is not zero then the nodal values must be for that <node>.

If <value_type> is not FE_NODAL_UNKNOWN then the nodal values must be of that
<value_type>.

If <version> is not zero then the nodal values must be for that <version>.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	Function_finite_element_handle *function_finite_element_handle_address;

	result=0;
	if ((function_finite_element_handle_address=
		reinterpret_cast<Function_finite_element_handle *>(
		function_finite_element))&&(*function_finite_element_handle_address))
	{
		if (component_name)
		{
			result=reinterpret_cast<Cmiss_function_variable_id>(
				new Function_variable_handle(
				((*function_finite_element_handle_address)->nodal_values)(
				component_name,node,value_type,version)));
		}
		else
		{
			if (0<component_number)
			{
				result=reinterpret_cast<Cmiss_function_variable_id>(
					new Function_variable_handle(
					((*function_finite_element_handle_address)->nodal_values)(
					component_number,node,value_type,version)));
			}
		}
	}

	return (result);
}

Cmiss_function_variable_id Cmiss_function_finite_element_element_xi(
	Cmiss_function_id function_finite_element)
/*******************************************************************************
LAST MODIFIED : 17 May 2004

DESCRIPTION :
Returns a variable that refers to the element/xi input of the
<function_finite_element>.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	Function_finite_element_handle *function_finite_element_handle_address;

	result=0;
	if ((function_finite_element_handle_address=
		reinterpret_cast<Function_finite_element_handle *>(
		function_finite_element))&&(*function_finite_element_handle_address))
	{
		result=reinterpret_cast<Cmiss_function_variable_id>(
			new Function_variable_handle(((*function_finite_element_handle_address)->
			element_xi)()));
	}

	return (result);
}

Cmiss_function_variable_id Cmiss_function_finite_element_element(
	Cmiss_function_id function_finite_element)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Returns a variable that refers to the element part of the
<function_finite_element>.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	Function_finite_element_handle *function_finite_element_handle_address;

	result=0;
	if ((function_finite_element_handle_address=
		reinterpret_cast<Function_finite_element_handle *>(
		function_finite_element))&&(*function_finite_element_handle_address))
	{
		result=reinterpret_cast<Cmiss_function_variable_id>(
			new Function_variable_handle(((*function_finite_element_handle_address)->
			element)()));
	}

	return (result);
}

Cmiss_function_variable_id Cmiss_function_finite_element_xi(
	Cmiss_function_id function_finite_element)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Returns a variable that refers to the xi part of the <function_finite_element>.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	Function_finite_element_handle *function_finite_element_handle_address;

	result=0;
	if ((function_finite_element_handle_address=
		reinterpret_cast<Function_finite_element_handle *>(
		function_finite_element))&&(*function_finite_element_handle_address))
	{
		result=reinterpret_cast<Cmiss_function_variable_id>(
			new Function_variable_handle(((*function_finite_element_handle_address)->
			xi)()));
	}

	return (result);
}

Cmiss_function_variable_id Cmiss_function_finite_element_xi_entry(
	Cmiss_function_id function_finite_element,unsigned int index)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Returns a variable that refers to the xi entry (<index>) of the
<function_finite_element>.  <index> number 1 is the first entry.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	Function_finite_element_handle *function_finite_element_handle_address;

	result=0;
	if ((0<index)&&(function_finite_element_handle_address=
		reinterpret_cast<Function_finite_element_handle *>(
		function_finite_element))&&(*function_finite_element_handle_address))
	{
		result=reinterpret_cast<Cmiss_function_variable_id>(
			new Function_variable_handle(((*function_finite_element_handle_address)->
			xi)(index)));
	}

	return (result);
}

int Cmiss_function_finite_element_number_of_components(
	Cmiss_function_id function_finite_element,
	unsigned int *number_of_components_address)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Gets the <*number_of_components_address> of the <function_finite_element>.
Returns a non-zero for success.
==============================================================================*/
{
	int return_code;
	Function_finite_element_handle *function_finite_element_handle_address;

	return_code=0;
	if (number_of_components_address&&(function_finite_element_handle_address=
		reinterpret_cast<Function_finite_element_handle *>(
		function_finite_element))&&(*function_finite_element_handle_address))
	{
		return_code=1;
		*number_of_components_address=((*function_finite_element_handle_address)->
			number_of_components)();
	}

	return (return_code);
}

int Cmiss_function_finite_element_region(
	Cmiss_function_id function_finite_element,Cmiss_region_id *region_address)
/*******************************************************************************
LAST MODIFIED : 3 November 2004

DESCRIPTION :
Gets the <*region_address> of the <function_finite_element>.  Returns a non-zero
for success.

NB.  The calling program should use ACCESS(Cmiss_region) and
DEACCESS(Cmiss_region) to manage the lifetime of the returned element
==============================================================================*/
{
	int return_code;
	Function_finite_element_handle *function_finite_element_handle_address;

	return_code=0;
	if (region_address&&(function_finite_element_handle_address=
		reinterpret_cast<Function_finite_element_handle *>(
		function_finite_element))&&(*function_finite_element_handle_address))
	{
		return_code=1;
		*region_address=((*function_finite_element_handle_address)->region)();
	}

	return (return_code);
}

int Cmiss_function_finite_element_number_of_versions(
	Cmiss_function_id function_finite_element,unsigned int component_number,
	Cmiss_node_id node,unsigned int *number_of_versions_address)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Gets the <*number_of_versions_address> for the <component_number> and <node> of
the <function_finite_element>.  Returns a non-zero for success.
==============================================================================*/
{
	int return_code;
	Function_finite_element_handle *function_finite_element_handle_address;

	return_code=0;
	if (number_of_versions_address&&(function_finite_element_handle_address=
		reinterpret_cast<Function_finite_element_handle *>(
		function_finite_element))&&(*function_finite_element_handle_address))
	{
		return_code=1;
		*number_of_versions_address=((*function_finite_element_handle_address)->
			number_of_versions)(component_number,node);
	}

	return (return_code);
}

int Cmiss_function_finite_element_number_of_derivatives(
	Cmiss_function_id function_finite_element,unsigned int component_number,
	Cmiss_node_id node,unsigned int *number_of_derivatives_address)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Gets the <*number_of_derivatives_address> for the <component_number> and <node>
of the <function_finite_element>.  Returns a non-zero for success.
==============================================================================*/
{
	int return_code;
	Function_finite_element_handle *function_finite_element_handle_address;

	return_code=0;
	if (number_of_derivatives_address&&(function_finite_element_handle_address=
		reinterpret_cast<Function_finite_element_handle *>(
		function_finite_element))&&(*function_finite_element_handle_address))
	{
		return_code=1;
		*number_of_derivatives_address=((*function_finite_element_handle_address)->
			number_of_derivatives)(component_number,node);
	}

	return (return_code);
}

int Cmiss_function_finite_element_nodal_value_types(
	Cmiss_function_id function_finite_element,unsigned int component_number,
	Cmiss_node_id node,enum FE_nodal_value_type **nodal_value_types_address)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Gets the (1+number_of_derivatives) nodal value types for the <component_number>
and <node> of the <function_finite_element>.  Returns a non-zero for success.

NB.  The calling program should DEALLOCATE the returned array when it is no
longer needed.
==============================================================================*/
{
	int return_code;
	Function_finite_element_handle *function_finite_element_handle_address;

	return_code=0;
	if (nodal_value_types_address&&(function_finite_element_handle_address=
		reinterpret_cast<Function_finite_element_handle *>(
		function_finite_element))&&(*function_finite_element_handle_address))
	{
		return_code=1;
		*nodal_value_types_address=((*function_finite_element_handle_address)->
			nodal_value_types)(component_number,node);
	}

	return (return_code);
}

int Cmiss_function_finite_element_get_nodal_value(
	Cmiss_function_id function_finite_element,unsigned int component_number,
	Cmiss_node_id node,enum FE_nodal_value_type value_type,unsigned int version,
	Scalar *value_address)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Returns a non-zero and gets the value if exactly one nodal value is specified,
otherwise return zero.
==============================================================================*/
{
	int return_code;
	Function_finite_element_handle *function_finite_element_handle_address;

	return_code=0;
	if (value_address&&(function_finite_element_handle_address=
		reinterpret_cast<Function_finite_element_handle *>(
		function_finite_element))&&(*function_finite_element_handle_address))
	{
		if ((*function_finite_element_handle_address)->get_nodal_value(
			component_number,node,value_type,version,*value_address))
		{
			return_code=1;
		}
	}

	return (return_code);
}

int Cmiss_function_finite_element_set_nodal_value(
	Cmiss_function_id function_finite_element,unsigned int component_number,
	Cmiss_node_id node,enum FE_nodal_value_type value_type,unsigned int version,
	Scalar value)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Returns a non-zero and sets the value if exactly one nodal value is specified,
otherwise return zero.
==============================================================================*/
{
	int return_code;
	Function_finite_element_handle *function_finite_element_handle_address;

	return_code=0;
	if ((function_finite_element_handle_address=
		reinterpret_cast<Function_finite_element_handle *>(
		function_finite_element))&&(*function_finite_element_handle_address))
	{
		if ((*function_finite_element_handle_address)->set_nodal_value(
			component_number,node,value_type,version,value))
		{
			return_code=1;
		}
	}

	return (return_code);
}

int Cmiss_function_finite_element_number_of_xi(
	Cmiss_function_id function_finite_element,unsigned int *number_of_xi_address)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Gets the <*number_of_xi_address> of the <function_finite_element>.  Returns a
non-zero for success.
==============================================================================*/
{
	int return_code;
	Function_finite_element_handle *function_finite_element_handle_address;

	return_code=0;
	if (number_of_xi_address&&(function_finite_element_handle_address=
		reinterpret_cast<Function_finite_element_handle *>(
		function_finite_element))&&(*function_finite_element_handle_address))
	{
		return_code=1;
		*number_of_xi_address=((*function_finite_element_handle_address)->
			number_of_xi)();
	}

	return (return_code);
}

int Cmiss_function_finite_element_element_value(
	Cmiss_function_id function_finite_element,
	Cmiss_element_id *element_address)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Gets the <*element_address> of the <function_finite_element>.  Returns a
non-zero for success.

NB.  The calling program should use ACCESS(Cmiss_element) and
DEACCESS(Cmiss_element) to manage the lifetime of the returned element
==============================================================================*/
{
	int return_code;
	Function_finite_element_handle *function_finite_element_handle_address;

	return_code=0;
	if (element_address&&(function_finite_element_handle_address=
		reinterpret_cast<Function_finite_element_handle *>(
		function_finite_element))&&(*function_finite_element_handle_address))
	{
		return_code=1;
		*element_address=((*function_finite_element_handle_address)->
			element_value)();
	}

	return (return_code);
}

int Cmiss_function_finite_element_xi_value(
	Cmiss_function_id function_finite_element,unsigned int index,
	Scalar *value_address)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Gets the <*value_address> for the specified xi entry (<index>) of the
<function_finite_element>.  <index> number 1 is the first entry.  Returns a
non-zero for success.
==============================================================================*/
{
	int return_code;
	Function_finite_element_handle *function_finite_element_handle_address;

	return_code=0;
	if (value_address&&(0<index)&&(function_finite_element_handle_address=
		reinterpret_cast<Function_finite_element_handle *>(
		function_finite_element))&&(*function_finite_element_handle_address))
	{
		return_code=1;
		*value_address=((*function_finite_element_handle_address)->xi_value)(index);
	}

	return (return_code);
}
