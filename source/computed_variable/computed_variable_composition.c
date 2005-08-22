/*******************************************************************************
FILE : computed_variable_composition.c

LAST MODIFIED : 24 July 2003

DESCRIPTION :
Implements the composition computed variable - independent variables, for a
dependent variable, are calculated from source variables.

???DB.  Get rid of for_each?
	- USE_FOR_EACH_SOURCE_VARIABLE.  If changed to a list of pairs, would have to
		put the dependent variable into some of the user_data

???DB.  Need a composite as well?
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
#include "computed_variable/computed_value_derivative_matrix.h"
#include "computed_variable/computed_value_matrix.h"
#include "computed_variable/computed_variable_composite.h"
#include "computed_variable/computed_variable_composition.h"
#include "computed_variable/computed_variable_identity.h"
#include "computed_variable/computed_variable_private.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Module constants
----------------
*/
static char Cmiss_variable_composition_type_string[]="Composition";

/*
Module types
------------
*/
struct Cmiss_variable_composition_type_specific_data
/*******************************************************************************
LAST MODIFIED : 9 July 2003

DESCRIPTION :
Gives a way of calculating the <independent_variables> for a
<dependent_variable>.
==============================================================================*/
{
	Cmiss_variable_id dependent_variable,*independent_variables,*source_variables;
	int number_of_source_variables;
}; /* struct Cmiss_variable_composition_type_specific_data */

#if defined (USE_FOR_EACH_SOURCE_VARIABLE)
/*???DB.  Seems like over-kill.  Should change to a list of pairs if going to do
	this */
typedef int (*Cmiss_variable_composition_source_variable_iterator)(
	Cmiss_variable_id variable,Cmiss_variable_id source_variable,
	Cmiss_variable_id independent_variable,void *data_void);

struct Cmiss_variable_composition_evaluate_derivative_source_variable_data
/*******************************************************************************
LAST MODIFIED : 13 July 2003

DESCRIPTION :
Used for storing the information source variable derivatives.
==============================================================================*/
{
	Cmiss_value_id *source_derivatives;
	Cmiss_variable_id *independent_variables;
	int number_of_source_derivatives,order;
}; /* struct
	Cmiss_variable_composition_evaluate_derivative_source_variable_data */

struct Cmiss_variable_composition_accumulate_total_derivative_data
/*******************************************************************************
LAST MODIFIED : 13 July 2003

DESCRIPTION :
==============================================================================*/
{
	int order;
	Cmiss_value_id *source_derivative_address,total_derivative;
	Cmiss_variable_id *independent_variables;
}; /* struct Cmiss_variable_composition_accumulate_total_derivative_data */
#endif /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */

/*
Module functions
----------------
*/
#if defined (USE_FOR_EACH_SOURCE_VARIABLE)
/*???DB.  Seems like over-kill.  Should change to a list of pairs if going to do
	this */
static int Cmiss_variable_composition_for_each_source_variable(
	struct Cmiss_variable *variable,
	Cmiss_variable_composition_source_variable_iterator iterator,void *user_data)
/*******************************************************************************
LAST MODIFIED : 10 July 2003

DESCRIPTION :
Applies the <iterator> to each of the <variable>s source/independent variable
pairs until finish or the <iterator> returns zero.
==============================================================================*/
{
	int i,return_code;
	Cmiss_variable_id *independent_variable_address;
	Cmiss_variable_id *source_variable_address;
	struct Cmiss_variable_composition_type_specific_data *data;

	ENTER(Cmiss_variable_composition_for_each_source_variable);
	return_code=0;
	/* check arguments */
	data=(struct Cmiss_variable_composition_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(variable&&data&&iterator&&
		CMISS_VARIABLE_IS_TYPE(composition)(variable),return_code,0)
	{
		return_code=1;
		i=data->number_of_source_variables;
		source_variable_address=data->source_variables;
		independent_variable_address=data->independent_variables;
		while (return_code&&(i>0))
		{
			return_code=(iterator)(variable,*source_variable_address,
				*independent_variable_address,user_data);
			i--;
			source_variable_address++;
			independent_variable_address++;
		}
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_composition_for_each_source_variable */

static int Cmiss_variable_composition_evaluate_source_variable(
	Cmiss_variable_id variable,Cmiss_variable_id source_variable,
	Cmiss_variable_id independent_variable,
	void *independent_variable_value_list_void)
/*******************************************************************************
LAST MODIFIED : 10 July 2003

DESCRIPTION :
Evaluates the <source_variable> and adds the <independent_variable>-value pair
to the <independent_variable_value_list_void>.
==============================================================================*/
{
	Cmiss_value_id source_value;
	int return_code;
	struct Cmiss_variable_value *independent_variable_value,
		*source_variable_value;
	struct LIST(Cmiss_variable_value) *independent_variable_value_list;

	ENTER(Cmiss_variable_composition_evaluate_source_variable);
	return_code=0;
	ASSERT_IF(variable&&source_variable&&independent_variable,return_code,0)
	{
		if (independent_variable_value_list=
			(struct LIST(Cmiss_variable_value) *)independent_variable_value_list_void)
		{
			if (source_value=CREATE(Cmiss_value)())
			{
				ACCESS(Cmiss_value)(source_value);
				if (source_variable_value=CREATE(Cmiss_variable_value)(
					source_variable,source_value))
				{
					ACCESS(Cmiss_variable_value)(source_variable_value);
					if (independent_variable_value=CREATE(Cmiss_variable_value)(
						independent_variable,source_value))
					{
						ACCESS(Cmiss_variable_value)(independent_variable_value);
						if (Cmiss_variable_evaluate(source_variable_value,
							(struct LIST(Cmiss_variable_value) *)NULL))
						{
							if (ADD_OBJECT_TO_LIST(Cmiss_variable_value)(
								independent_variable_value,independent_variable_value_list))
							{
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Cmiss_variable_composition_evaluate_source_variable.  "
									"Could not add result to independent_variable_value_list");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_variable_composition_evaluate_source_variable.  "
								"Could not evaluate source variable");
						}
						DEACCESS(Cmiss_variable_value)(&independent_variable_value);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_variable_composition_evaluate_source_variable.  "
							"Could not create independent_variable_value");
					}
					DEACCESS(Cmiss_variable_value)(&source_variable_value);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_variable_composition_evaluate_source_variable.  "
						"Could not create source_variable_value");
				}
				DEACCESS(Cmiss_value)(&source_value);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_variable_composition_evaluate_source_variable.  "
					"Could not create source_value");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_composition_evaluate_source_variable.  "
				"Invalid argument");
		}
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_composition_evaluate_source_variable */

static int Cmiss_variable_composition_evaluate_derivative_source_variable(
	Cmiss_variable_id variable,Cmiss_variable_id source_variable,
	Cmiss_variable_id independent_variable,void *data_void)
/*******************************************************************************
LAST MODIFIED : 13 July 2003

DESCRIPTION :
Evaluates the <source_variable> and adds the <independent_variable>-value pair
to a list contained in <data_void>.
==============================================================================*/
{
	Cmiss_value_id derivative_matrix,*source_derivatives;
	int return_code;
	struct Cmiss_variable_composition_evaluate_derivative_source_variable_data
		*data;

	ENTER(Cmiss_variable_composition_evaluate_derivative_source_variable);
	return_code=0;
	data=(struct
		Cmiss_variable_composition_evaluate_derivative_source_variable_data *)
		data_void;
	ASSERT_IF(variable&&source_variable&&independent_variable&&data,return_code,0)
	{
		if (derivative_matrix=CREATE(Cmiss_value)())
		{
			ACCESS(Cmiss_value)(derivative_matrix);
			if (Cmiss_variable_evaluate_derivative(source_variable,data->order,
				data->independent_variables,(struct LIST(Cmiss_variable_value) *)NULL,
				derivative_matrix))
			{
				if (REALLOCATE(source_derivatives,data->source_derivatives,
					Cmiss_value_id,(data->number_of_source_derivatives)+1))
				{
					data->source_derivatives=source_derivatives;
					source_derivatives[data->number_of_source_derivatives]=
						ACCESS(Cmiss_value)(derivative_matrix);
					(data->number_of_source_derivatives)++;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_variable_composition_evaluate_derivative_source_variable.  "
						"Could not reallocate source_derivatives");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_variable_composition_evaluate_derivative_source_variable.  "
					"Could not evaluate source variable");
			}
			DEACCESS(Cmiss_value)(&derivative_matrix);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_composition_evaluate_derivative_source_variable.  "
				"Could not create derivative_matrix");
		}
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_composition_evaluate_derivative_source_variable */

static int Cmiss_variable_composition_accumulate_total_derivative(
	Cmiss_variable_id variable,Cmiss_variable_id source_variable,
	Cmiss_variable_id independent_variable,void *data_void)
/*******************************************************************************
LAST MODIFIED : 11 July 2003

DESCRIPTION :
???DB.  Need order'th not 1st derivative of dependent_variable
==============================================================================*/
{
	int return_code;
	Cmiss_value_id independent_derivative,source_derivative;
	struct Cmiss_variable_composition_accumulate_total_derivative_data *user_data;
	struct Cmiss_variable_composition_type_specific_data *composition_data;

	ENTER(Cmiss_variable_composition_accumulate_total_derivative);
	return_code=0;
	/* check arguments */
	if (variable&&
		(composition_data=(struct Cmiss_variable_composition_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable))&&source_variable&&
		independent_variable&&(user_data=
		(struct Cmiss_variable_composition_accumulate_total_derivative_data *)
		data_void)&&(user_data->source_derivative_address)&&
		(source_derivative= *(user_data->source_derivative_address)))
	{
		(user_data->source_derivative_address)++;
		if (independent_derivative=CREATE(Cmiss_value)())
		{
			ACCESS(Cmiss_value)(independent_derivative);
			if (Cmiss_variable_evaluate_derivative(
				composition_data->dependent_variable,1,&independent_variable,
				(struct LIST(Cmiss_variable_value) *)NULL,independent_derivative))
			{
				return_code=Cmiss_value_multiply_and_accumulate(independent_derivative,
					source_derivative,user_data->total_derivative);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_variable_composition_accumulate_total_derivative.  "
					"Could not evaluate derivative of <variable> with respect to "
					"<independent_variable>");
			}
			DEACCESS(Cmiss_value)(&independent_derivative);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_composition_accumulate_total_derivative.  "
				"Could not create <independent_derivative>");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_composition_accumulate_total_derivative.  "
			"Invalid argument(s).  %p %p %p %p",variable,source_variable,
			independent_variable,data_void);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_composition_accumulate_total_derivative */
#endif /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */

/*
Module methods
--------------
*/
static START_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(composition)
{
	int i;

	DEACCESS(Cmiss_variable)(&(data->dependent_variable));
	if (data->independent_variables)
	{
		for (i=0;i<data->number_of_source_variables;i++)
		{
			DEACCESS(Cmiss_variable)(&((data->independent_variables)[i]));
		}
		DEALLOCATE(data->independent_variables);
	}
	if (data->source_variables)
	{
		for (i=0;i<data->number_of_source_variables;i++)
		{
			DEACCESS(Cmiss_variable)(&((data->source_variables)[i]));
		}
		DEALLOCATE(data->source_variables);
	}
	return_code=1;
}
END_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(composition)

static START_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(composition)
{
	int i;

	if (0<source->number_of_source_variables)
	{
		ALLOCATE(destination->independent_variables,Cmiss_variable_id,
			source->number_of_source_variables);
		ALLOCATE(destination->source_variables,Cmiss_variable_id,
			source->number_of_source_variables);
		if ((destination->independent_variables)&&(destination->source_variables))
		{
			destination->dependent_variable=
				ACCESS(Cmiss_variable)(source->dependent_variable);
			destination->number_of_source_variables=
				source->number_of_source_variables;
			for (i=0;i<source->number_of_source_variables;i++)
			{
				(destination->independent_variables)[i]=
					ACCESS(Cmiss_variable)((source->independent_variables)[i]);
				(destination->source_variables)[i]=
					ACCESS(Cmiss_variable)((source->source_variables)[i]);
			}
		}
		else
		{
			DEALLOCATE(destination);
		}
	}
	else
	{
		DEALLOCATE(destination);
	}
}
END_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(composition)

#if defined (OLD_CODE)
/*???DB.  Before using composite and identity to do it properly? */
static START_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	composition)
{
	int i;
	struct Cmiss_variable_composition_type_specific_data *data;
	struct LIST(Cmiss_variable_value)
		*composition_independent_variable_value_list;
#if defined (USE_FOR_EACH_SOURCE_VARIABLE)
/*???DB.  Seems like over-kill.  Should change to a list of pairs if going to do
	this */
	struct Cmiss_variable_composition_accumulate_total_derivative_data
		cmiss_variable_composition_accumulate_total_derivative_data;
	struct Cmiss_variable_composition_evaluate_derivative_source_variable_data
		cmiss_variable_composition_evaluate_derivative_source_variable_data;
#else /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */
	Cmiss_value_id derivative_matrix,*source_derivative_address,
		*source_derivatives,source_value;
	Cmiss_variable_id dependent_variable,
		*composition_independent_variable_address,
		*composition_independent_variables,*derivative_independent_variables,
		*source_variable_address,*source_variables;
	int j,number_of_source_derivatives,number_of_source_variables;
	struct Cmiss_variable_value *composition_independent_variable_value,
		*source_variable_value;
#endif /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */

	data=(struct Cmiss_variable_composition_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->dependent_variable)&&
		(0<data->number_of_source_variables)&&(data->independent_variables)&&
		(data->source_variables),return_code,0)
	{
		if (composition_independent_variable_value_list=
			CREATE_LIST(Cmiss_variable_value)())
		{
			/* evaluate the source variables */
#if defined (USE_FOR_EACH_SOURCE_VARIABLE)
/*???DB.  Seems like over-kill.  Should change to a list of pairs if going to do
	this */
			if (Cmiss_variable_composition_for_each_source_variable(variable,
				Cmiss_variable_composition_evaluate_source_variable,
				(void *)composition_independent_variable_value_list))
#else /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */
			dependent_variable=data->dependent_variable;
			number_of_source_variables=data->number_of_source_variables;
			source_variables=data->source_variables;
			composition_independent_variables=data->independent_variables;
			return_code=1;
			i=number_of_source_variables;
			source_variable_address=source_variables;
			composition_independent_variable_address=
				composition_independent_variables;
			while (return_code&&(i>0))
			{
				if (source_value=CREATE(Cmiss_value)())
				{
					ACCESS(Cmiss_value)(source_value);
					if (source_variable_value=CREATE(Cmiss_variable_value)(
						*source_variable_address,source_value))
					{
						ACCESS(Cmiss_variable_value)(source_variable_value);
						if (composition_independent_variable_value=
							CREATE(Cmiss_variable_value)(
							*composition_independent_variable_address,source_value))
						{
							ACCESS(Cmiss_variable_value)(
							composition_independent_variable_value);
							if (Cmiss_variable_evaluate(source_variable_value,
								(struct LIST(Cmiss_variable_value) *)NULL))
							{
								if (!ADD_OBJECT_TO_LIST(Cmiss_variable_value)(
									composition_independent_variable_value,
									composition_independent_variable_value_list))
								{
									display_message(ERROR_MESSAGE,
										"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
										"Could not add result to "
										"composition_independent_variable_value_list");
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
									"Could not evaluate source variable");
								return_code=0;
							}
							DEACCESS(Cmiss_variable_value)(
								&composition_independent_variable_value);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
								"Could not create composition_independent_variable_value");
							return_code=0;
						}
						DEACCESS(Cmiss_variable_value)(&source_variable_value);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
							"Could not create source_variable_value");
						return_code=0;
					}
					DEACCESS(Cmiss_value)(&source_value);
				}
				i--;
				source_variable_address++;
				composition_independent_variable_address++;
			}
			if (return_code)
#endif /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */
			{
				/* evaluate the source derivatives */
#if defined (USE_FOR_EACH_SOURCE_VARIABLE)
/*???DB.  Seems like over-kill.  Should change to a list of pairs if going to do
	this */
				cmiss_variable_composition_evaluate_derivative_source_variable_data.
					order=order;
				cmiss_variable_composition_evaluate_derivative_source_variable_data.
					independent_variables=independent_variables;
				cmiss_variable_composition_evaluate_derivative_source_variable_data.
					number_of_source_derivatives=0;
				cmiss_variable_composition_evaluate_derivative_source_variable_data.
					source_derivatives=(Cmiss_value_id *)NULL;
				if (Cmiss_variable_composition_for_each_source_variable(variable,
					Cmiss_variable_composition_evaluate_derivative_source_variable,
					(void *)&cmiss_variable_composition_evaluate_derivative_source_variable_data)&&
					(cmiss_variable_composition_evaluate_derivative_source_variable_data.
					number_of_source_derivatives==data->number_of_source_variables))
#else /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */
				i=number_of_source_variables;
				source_variable_address=source_variables;
				composition_independent_variable_address=
					composition_independent_variables;
				source_derivatives=(Cmiss_value_id *)NULL;
				number_of_source_derivatives=0;
				while (return_code&&(i>0))
				{
					if (derivative_matrix=CREATE(Cmiss_value)())
					{
						ACCESS(Cmiss_value)(derivative_matrix);
						if (Cmiss_variable_evaluate_derivative(*source_variable_address,
							order,independent_variables,
							(struct LIST(Cmiss_variable_value) *)NULL,derivative_matrix))
						{
							if (REALLOCATE(source_derivative_address,source_derivatives,
								Cmiss_value_id,number_of_source_derivatives+1))
							{
								source_derivatives=source_derivative_address;
								source_derivatives[number_of_source_derivatives]=
									ACCESS(Cmiss_value)(derivative_matrix);
								number_of_source_derivatives++;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
									"Could not reallocate source_derivatives");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
								"Could not evaluate source variable");
							return_code=0;
						}
						DEACCESS(Cmiss_value)(&derivative_matrix);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
							"Could not create derivative_matrix");
						return_code=0;
					}
					i--;
					source_variable_address++;
					composition_independent_variable_address++;
				}
				if (return_code&&(number_of_source_derivatives==
					number_of_source_variables))
#endif /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */
				{
					/* swap the <composition_independent_variable_value_list> with the
						current values */
					if (FOR_EACH_OBJECT_IN_LIST(Cmiss_variable_value)(
						Cmiss_variable_get_set_independent_variable_value_iterator,
						(void *)(data->dependent_variable),
						composition_independent_variable_value_list))
					{
						/* derivative=
							d(data->variable)/d(data->independent_variables)+
							sum{d(data->variable)/d(data->variable->independent_variable)*
							d(data->variable->source_variable)/d(data->independent_variables)}
							*/
						if (Cmiss_variable_evaluate_derivative(data->dependent_variable,
							order,independent_variables,
							(struct LIST(Cmiss_variable_value) *)NULL,value))
						{
#if defined (USE_FOR_EACH_SOURCE_VARIABLE)
/*???DB.  Seems like over-kill.  Should change to a list of pairs if going to do
	this */
							cmiss_variable_composition_accumulate_total_derivative_data.order=
								order;
							cmiss_variable_composition_accumulate_total_derivative_data.
								independent_variables=independent_variables;
							cmiss_variable_composition_accumulate_total_derivative_data.
								total_derivative=value;
							cmiss_variable_composition_accumulate_total_derivative_data.
								source_derivative_address=
								cmiss_variable_composition_evaluate_derivative_source_variable_data.
								source_derivatives;
							return_code=Cmiss_variable_composition_for_each_source_variable(
								variable,Cmiss_variable_composition_accumulate_total_derivative,
								(void *)&cmiss_variable_composition_accumulate_total_derivative_data);
#else /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */
							ALLOCATE(derivative_independent_variables,Cmiss_variable_id,
								order);
							if (derivative_independent_variables&&
								(derivative_matrix=CREATE(Cmiss_value)()))
							{
								ACCESS(Cmiss_value)(derivative_matrix);
								i=number_of_source_variables;
								source_derivative_address=source_derivatives;
								source_variable_address=source_variables;
								composition_independent_variable_address=
									composition_independent_variables;
								while (return_code&&(i>0))
								{
									for (j=0;j<order;j++)
									{
										derivative_independent_variables[j]=
											*composition_independent_variable_address;
									}
									if (Cmiss_variable_evaluate_derivative(
										data->dependent_variable,order,
										derivative_independent_variables,
										(struct LIST(Cmiss_variable_value) *)NULL,
										derivative_matrix))
									{
										return_code=Cmiss_value_multiply_and_accumulate(
											derivative_matrix,*source_derivative_address,value);
										source_derivative_address++;
									}
									else
									{
										return_code=0;
										display_message(ERROR_MESSAGE,
											"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
											"Could not evaluate derivative of <variable> with "
											"respect to <composition_independent_variable>");
									}
									i--;
									source_variable_address++;
									composition_independent_variable_address++;
								}
								DEACCESS(Cmiss_value)(&derivative_matrix);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
									"Could not create <derivative_matrix>");
								return_code=0;
							}
							if (derivative_independent_variables)
							{
								DEALLOCATE(derivative_independent_variables);
							}
#endif /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */
						}
						if (!FOR_EACH_OBJECT_IN_LIST(Cmiss_variable_value)(
							Cmiss_variable_get_set_independent_variable_value_iterator,
							(void *)(data->dependent_variable),
							composition_independent_variable_value_list))
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
								"Could not reset "
								"<composition_independent_variable_value_list>");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
							"Could not over-ride "
							"<composition_independent_variable_value_list>");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
						"Could not evaluate source derivatives");
					return_code=0;
				}
#if defined (USE_FOR_EACH_SOURCE_VARIABLE)
/*???DB.  Seems like over-kill.  Should change to a list of pairs if going to do
	this */
				if (cmiss_variable_composition_evaluate_derivative_source_variable_data.
					source_derivatives)
				{
					for (i=0;i<
						cmiss_variable_composition_evaluate_derivative_source_variable_data.
						number_of_source_derivatives;i++)
					{
						DEACCESS(Cmiss_value)(
							(cmiss_variable_composition_evaluate_derivative_source_variable_data.
							source_derivatives)+i);
					}
					DEALLOCATE(
						cmiss_variable_composition_evaluate_derivative_source_variable_data.
						source_derivatives);
				}
#else /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */
				if (source_derivative_address=source_derivatives)
				{
					for (i=0;i<number_of_source_derivatives;i++)
					{
						DEACCESS(Cmiss_value)(source_derivative_address);
						source_derivative_address++;
					}
					DEALLOCATE(source_derivatives);
				}
#endif /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
					"Could not evaluate source variables");
				return_code=0;
			}
			DESTROY_LIST(Cmiss_variable_value)(
				&composition_independent_variable_value_list);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
				"Could not create source variable values list");
			return_code=0;
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(composition)
#endif /* defined (OLD_CODE) */

static START_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	composition)
/*******************************************************************************
LAST MODIFIED : 20 July 2003

DESCRIPTION :
h(x)=f(g(x))
==============================================================================*/
{
	Cmiss_value_id f_derivative_matrix,g_derivative_matrix,*matrices,*matrix,
		value_type;
	Cmiss_variable_id *composite_variables,dependent_variable,
		f_composite_variable,g_composite_variable,independent_variable,
		*independent_variable_address;
	int i,j,number_of_columns,number_of_matrices,number_of_rows,
		number_of_source_variables,number_of_values,offset;
	struct Cmiss_variable_composition_type_specific_data *data;
	struct LIST(Cmiss_variable_value)
		*composition_independent_variable_value_list;
	struct Matrix *matrix_local;

#if defined (USE_FOR_EACH_SOURCE_VARIABLE)
/*???DB.  Seems like over-kill.  Should change to a list of pairs if going to do
	this */
#else /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */
	Cmiss_value_id source_value;
	Cmiss_variable_id *composition_independent_variable_address,
		*composition_independent_variables,
		*source_variable_address,*source_variables;
	struct Cmiss_variable_value *composition_independent_variable_value,
		*source_variable_value;
#endif /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */

	data=(struct Cmiss_variable_composition_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->dependent_variable)&&
		(0<data->number_of_source_variables)&&(data->independent_variables)&&
		(data->source_variables),return_code,0)
	{
		dependent_variable=data->dependent_variable;
		number_of_source_variables=data->number_of_source_variables;
		/* set up working space */
		composition_independent_variable_value_list=
			CREATE_LIST(Cmiss_variable_value)();
		if (f_derivative_matrix=CREATE(Cmiss_value)())
		{
			ACCESS(Cmiss_value)(f_derivative_matrix);
		}
		if (g_derivative_matrix=CREATE(Cmiss_value)())
		{
			ACCESS(Cmiss_value)(g_derivative_matrix);
		}
		if (value_type=CREATE(Cmiss_value)())
		{
			ACCESS(Cmiss_value)(value_type);
		}
		if (composition_independent_variable_value_list&&f_derivative_matrix&&
			g_derivative_matrix&&value_type)
		{
			/* evaluate derivative of g */
			/* set up variable g */
			if (ALLOCATE(composite_variables,Cmiss_variable_id,
				number_of_source_variables+order))
			{
				for (i=0;i<number_of_source_variables;i++)
				{
					composite_variables[i]=
						ACCESS(Cmiss_variable)((data->source_variables)[i]);
				}
				j=number_of_source_variables;
				i=0;
				return_code=1;
				while (return_code&&(i<order))
				{
					if (composite_variables[j]=CREATE(Cmiss_variable)(
						(struct Cmiss_variable_package *)NULL,"variable"))
					{
						ACCESS(Cmiss_variable)(composite_variables[j]);
						return_code=Cmiss_variable_identity_set_type(composite_variables[j],
							independent_variables[i]);
						j++;
					}
					else
					{
						return_code=0;
					}
					i++;
				}
				if (return_code)
				{
					if (g_composite_variable=CREATE(Cmiss_variable)(
						(struct Cmiss_variable_package *)NULL,"variable"))
					{
						ACCESS(Cmiss_variable)(g_composite_variable);
						if (return_code=Cmiss_variable_composite_set_type(
							g_composite_variable,number_of_source_variables+order,
							composite_variables))
						{
							/* composite_variables is now managed by g_composite_variable */
							composite_variables=(Cmiss_variable_id *)NULL;
							if (!(return_code=Cmiss_variable_evaluate_derivative(
								g_composite_variable,order,independent_variables,
								(struct LIST(Cmiss_variable_value) *)NULL,g_derivative_matrix)))
							{
								display_message(ERROR_MESSAGE,
									"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
									"Could not evaluate derivative of g");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
								"Could not set up g_composite_variable");
						}
						DEACCESS(Cmiss_variable)(&g_composite_variable);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
							"Could not create g_composite_variable");
						return_code=0;
					}
					if (composite_variables)
					{
						j=number_of_source_variables+order;
						while (j>0)
						{
							j--;
							DEACCESS(Cmiss_variable)(composite_variables+j);
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
						"Could not set up composite_variables for g");
					while (j>0)
					{
						j--;
						DEACCESS(Cmiss_variable)(composite_variables+j);
					}
				}
				if (composite_variables)
				{
					DEALLOCATE(composite_variables);
				}
			}
			if (return_code)
			{
				/* evaluate the source variables */
#if defined (USE_FOR_EACH_SOURCE_VARIABLE)
/*???DB.  Seems like over-kill.  Should change to a list of pairs if going to do
	this */
				if (Cmiss_variable_composition_for_each_source_variable(variable,
					Cmiss_variable_composition_evaluate_source_variable,
					(void *)composition_independent_variable_value_list))
#else /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */
				source_variables=data->source_variables;
				composition_independent_variables=data->independent_variables;
				return_code=1;
				i=number_of_source_variables;
				source_variable_address=source_variables;
				composition_independent_variable_address=
					composition_independent_variables;
				while (return_code&&(i>0))
				{
					if (source_value=CREATE(Cmiss_value)())
					{
						ACCESS(Cmiss_value)(source_value);
						if (source_variable_value=CREATE(Cmiss_variable_value)(
							*source_variable_address,source_value))
						{
							ACCESS(Cmiss_variable_value)(source_variable_value);
							if (composition_independent_variable_value=
								CREATE(Cmiss_variable_value)(
								*composition_independent_variable_address,source_value))
							{
								ACCESS(Cmiss_variable_value)(
								composition_independent_variable_value);
								if (Cmiss_variable_evaluate(source_variable_value,
									(struct LIST(Cmiss_variable_value) *)NULL))
								{
									if (!ADD_OBJECT_TO_LIST(Cmiss_variable_value)(
										composition_independent_variable_value,
										composition_independent_variable_value_list))
									{
										display_message(ERROR_MESSAGE,
											"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
											"Could not add result to "
											"composition_independent_variable_value_list");
										return_code=0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
										"Could not evaluate source variable");
									return_code=0;
								}
								DEACCESS(Cmiss_variable_value)(
									&composition_independent_variable_value);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
									"Could not create composition_independent_variable_value");
								return_code=0;
							}
							DEACCESS(Cmiss_variable_value)(&source_variable_value);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
								"Could not create source_variable_value");
							return_code=0;
						}
						DEACCESS(Cmiss_value)(&source_value);
					}
					i--;
					source_variable_address++;
					composition_independent_variable_address++;
				}
				if (return_code)
#endif /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */
				{
					/* evaluate derivative of f */
					/* set up variable to differentiate f with respect to */
					if (ALLOCATE(composite_variables,Cmiss_variable_id,
						number_of_source_variables+order))
					{
						for (i=0;i<number_of_source_variables;i++)
						{
							composite_variables[i]=
								ACCESS(Cmiss_variable)((data->independent_variables)[i]);
						}
						j=number_of_source_variables;
						for (i=0;i<order;i++)
						{
							composite_variables[j]=
								ACCESS(Cmiss_variable)(independent_variables[i]);
							j++;
						}
						if (f_composite_variable=CREATE(Cmiss_variable)(
							(struct Cmiss_variable_package *)NULL,"variable"))
						{
							ACCESS(Cmiss_variable)(f_composite_variable);
							if (return_code=Cmiss_variable_composite_set_type(
								f_composite_variable,number_of_source_variables+order,
								composite_variables))
							{
								/* composite_variables is now managed by f_composite_variable */
								composite_variables=(Cmiss_variable_id *)NULL;
								if (ALLOCATE(independent_variable_address,Cmiss_variable_id,
									order))
								{
									for (i=0;i<order;i++)
									{
										independent_variable_address[i]=f_composite_variable;
									}
									/* swap the <composition_independent_variable_value_list> with
										the current values */
									if (FOR_EACH_OBJECT_IN_LIST(Cmiss_variable_value)(
										Cmiss_variable_get_set_independent_variable_value_iterator,
										(void *)dependent_variable,
										composition_independent_variable_value_list))
									{
										if (!(return_code=Cmiss_variable_evaluate_derivative(
											dependent_variable,order,independent_variable_address,
											(struct LIST(Cmiss_variable_value) *)NULL,
											f_derivative_matrix)))
										{
											display_message(ERROR_MESSAGE,
												"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
												"Could not evaluate derivative of f");
										}
										if (!FOR_EACH_OBJECT_IN_LIST(Cmiss_variable_value)(
											Cmiss_variable_get_set_independent_variable_value_iterator,
											(void *)dependent_variable,
											composition_independent_variable_value_list))
										{
											display_message(ERROR_MESSAGE,
												"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
												"Could not reset "
												"<composition_independent_variable_value_list>");
											return_code=0;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
											"Could not over-ride "
											"<composition_independent_variable_value_list>");
										return_code=0;
									}
									DEALLOCATE(independent_variable_address);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
										"Could not allocate independent variables for f derivative");
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
									"Could not set up f_composite_variable");
							}
							DEACCESS(Cmiss_variable)(&f_composite_variable);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
								"Could not create f_composite_variable");
							return_code=0;
						}
						if (composite_variables)
						{
							j=number_of_source_variables+order;
							while (j>0)
							{
								j--;
								DEACCESS(Cmiss_variable)(composite_variables+j);
							}
							DEALLOCATE(composite_variables);
						}
					}
				}
			}
			if (return_code)
			{
				/* multiply derivative matrices */
				/* set up result */
				/* get number of rows in derivative matrix */
				if (return_code=Cmiss_variable_get_value_type(dependent_variable,
					value_type))
				{
					return_code=Cmiss_value_get_reals(value_type,&number_of_rows,
						(FE_value **)NULL);
				}
				if (return_code&&(0<number_of_rows))
				{
					/* set up the matrices in the derivative */
					i=order;
					number_of_matrices=0;
					matrices=(Cmiss_value_id *)NULL;
					independent_variable_address=independent_variables;
					while (return_code&&(i>0))
					{
						independent_variable= *independent_variable_address;
						if (return_code=Cmiss_variable_get_value_type(independent_variable,
							value_type))
						{
							return_code=Cmiss_value_get_reals(value_type,&number_of_values,
								(FE_value **)NULL);
							if (return_code&&(0<number_of_values))
							{
								if (REALLOCATE(matrix,matrices,Cmiss_value_id,
									2*number_of_matrices+1))
								{
									matrices=matrix;
									matrix += number_of_matrices;
									if (*matrix=CREATE(Cmiss_value)())
									{
										number_of_matrices++;
										if (matrix_local=CREATE(Matrix)("matrix",DENSE,
											number_of_rows,number_of_values))
										{
											if (return_code=Cmiss_value_matrix_set_type(*matrix,
												matrix_local))
											{
												offset=number_of_matrices;
												j=number_of_matrices-1;
												while (return_code&&(j>0))
												{
													matrix++;
													if (return_code=Cmiss_value_matrix_get_dimensions(
														*(matrix-offset),(int *)NULL,&number_of_columns))
													{
														if (*matrix=CREATE(Cmiss_value)())
														{
															number_of_matrices++;
															if (matrix_local=CREATE(Matrix)("matrix",DENSE,
																number_of_rows,number_of_values*
																number_of_columns))
															{
																if (return_code=Cmiss_value_matrix_set_type(
																	*matrix,matrix_local))
																{
																	j--;
																}
																else
																{
																	DESTROY(Matrix)(&matrix_local);
																}
															}
															else
															{
																return_code=0;
															}
														}
														else
														{
															return_code=0;
														}
													}
												}
											}
											else
											{
												DESTROY(Matrix)(&matrix_local);
											}
										}
										else
										{
											return_code=0;
										}
									}
									else
									{
										return_code=0;
									}
								}
								else
								{
									return_code=0;
								}
							}
							else
							{
								return_code=0;
							}
						}
						i--;
						independent_variable_address++;
					}
					if (return_code)
					{
						if (ALLOCATE(independent_variable_address,Cmiss_variable_id,order))
						{
							for (i=0;i<order;i++)
							{
								independent_variable_address[i]=independent_variables[i];
							}
							if (return_code=Cmiss_value_derivative_matrix_set_type(value,
								variable,order,independent_variable_address,matrices))
							{
								/* independent_variable_address and matrices are now managed by
									value */
								independent_variable_address=(Cmiss_variable_id *)NULL;
								matrices=(Cmiss_value_id *)NULL;
								if (!(return_code=Cmiss_value_multiply_and_accumulate(value,
									f_derivative_matrix,g_derivative_matrix)))
								{
									display_message(ERROR_MESSAGE,
										"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
										"Could not multiply f_derivative_matrix and "
										"g_derivative_matrix");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
									"Could not set value to be a derivative matrix");
							}
							if (independent_variable_address)
							{
								DEALLOCATE(independent_variable_address);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
								"Could not allocate <independent_variable_address>");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
							"Could not create matrices for derivative matrix");
						return_code=0;
					}
					if (matrices)
					{
						matrix=matrices;
						i=number_of_matrices;
						while (i>0)
						{
							DESTROY(Cmiss_value)(matrix);
							matrix++;
							i--;
						}
						DEALLOCATE(matrices);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
						"Could not get number_of_rows");
					return_code=0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_composition_evaluate_derivative_type_specific.  "
				"Could not set up working space.  %p %p %p %p",
				composition_independent_variable_value_list,f_derivative_matrix,
				g_derivative_matrix,value_type);
			return_code=0;
		}
		if (value_type)
		{
			DEACCESS(Cmiss_value)(&value_type);
		}
		if (g_derivative_matrix)
		{
			DEACCESS(Cmiss_value)(&g_derivative_matrix);
		}
		if (f_derivative_matrix)
		{
			DEACCESS(Cmiss_value)(&f_derivative_matrix);
		}
		if (composition_independent_variable_value_list)
		{
			DESTROY_LIST(Cmiss_variable_value)(
				&composition_independent_variable_value_list);
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(composition)

static START_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(composition)
{
	struct Cmiss_variable_composition_type_specific_data *data;
	struct Cmiss_variable_value *temp_variable_value;
	struct LIST(Cmiss_variable_value) *independent_variable_value_list;
#if defined (USE_FOR_EACH_SOURCE_VARIABLE)
/*???DB.  Seems like over-kill.  Should change to a list of pairs if going to do
	this */
#else /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */
	Cmiss_value_id source_value;
	Cmiss_variable_id *independent_variable_address,*source_variable_address;
	int i;
	struct Cmiss_variable_value *independent_variable_value,
		*source_variable_value;
#endif /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */

	data=(struct Cmiss_variable_composition_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->dependent_variable)&&
		(0<data->number_of_source_variables)&&(data->independent_variables)&&
		(data->source_variables),return_code,0)
	{
		if (independent_variable_value_list=CREATE_LIST(Cmiss_variable_value)())
		{
			/* evaluate the source variables */
#if defined (USE_FOR_EACH_SOURCE_VARIABLE)
/*???DB.  Seems like over-kill.  Should change to a list of pairs if going to do
	this */
			if (Cmiss_variable_composition_for_each_source_variable(variable,
				Cmiss_variable_composition_evaluate_source_variable,
				(void *)independent_variable_value_list))
#else /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */
			return_code=1;
			i=data->number_of_source_variables;
			source_variable_address=data->source_variables;
			independent_variable_address=data->independent_variables;
			while (return_code&&(i>0))
			{
				if (source_value=CREATE(Cmiss_value)())
				{
					ACCESS(Cmiss_value)(source_value);
					if (source_variable_value=CREATE(Cmiss_variable_value)(
						*source_variable_address,source_value))
					{
						ACCESS(Cmiss_variable_value)(source_variable_value);
						if (independent_variable_value=CREATE(Cmiss_variable_value)(
							*independent_variable_address,source_value))
						{
							ACCESS(Cmiss_variable_value)(independent_variable_value);
							if (Cmiss_variable_evaluate(source_variable_value,
								(struct LIST(Cmiss_variable_value) *)NULL))
							{
								if (!ADD_OBJECT_TO_LIST(Cmiss_variable_value)(
									independent_variable_value,independent_variable_value_list))
								{
									display_message(ERROR_MESSAGE,
										"Cmiss_variable_composition_evaluate_type_specific.  "
										"Could not add result to independent_variable_value_list");
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Cmiss_variable_composition_evaluate_type_specific.  "
									"Could not evaluate source variable");
								return_code=0;
							}
							DEACCESS(Cmiss_variable_value)(&independent_variable_value);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_variable_composition_evaluate_type_specific.  "
								"Could not create independent_variable_value");
							return_code=0;
						}
						DEACCESS(Cmiss_variable_value)(&source_variable_value);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_variable_composition_evaluate_type_specific.  "
							"Could not create source_variable_value");
						return_code=0;
					}
					DEACCESS(Cmiss_value)(&source_value);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_variable_composition_evaluate_type_specific.  "
						"Could not create source_value");
					return_code=0;
				}
				i--;
				source_variable_address++;
				independent_variable_address++;
			}
			if (return_code)
#endif /* defined (USE_FOR_EACH_SOURCE_VARIABLE) */
			{
				/* swap the <independent_variable_value_list> with the current values */
				if (FOR_EACH_OBJECT_IN_LIST(Cmiss_variable_value)(
					Cmiss_variable_get_set_independent_variable_value_iterator,
					(void *)(data->dependent_variable),independent_variable_value_list))
				{
					if (temp_variable_value=CREATE(Cmiss_variable_value)(
						data->dependent_variable,value))
					{
						ACCESS(Cmiss_variable_value)(temp_variable_value);
						return_code=Cmiss_variable_evaluate(temp_variable_value,
							(struct LIST(Cmiss_variable_value) *)NULL);
						DEACCESS(Cmiss_variable_value)(&temp_variable_value);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_variable_composition_evaluate_type_specific.  "
							"Could not create temp_variable_value");
					}
					if (!FOR_EACH_OBJECT_IN_LIST(Cmiss_variable_value)(
						Cmiss_variable_get_set_independent_variable_value_iterator,
						(void *)(data->dependent_variable),independent_variable_value_list))
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_variable_composition_evaluate_type_specific.  "
							"Could not reset <independent_variable_value_list>");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_variable_composition_evaluate_type_specific.  "
						"Could not over-ride <independent_variable_value_list>");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_variable_composition_evaluate_type_specific.  "
					"Could not evaluate source variables");
				return_code=0;
			}
			DESTROY_LIST(Cmiss_variable_value)(&independent_variable_value_list);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_composition_evaluate_type_specific.  "
				"Could not create source variable values list");
			return_code=0;
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(composition)

static
	START_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	composition)
{
	Cmiss_variable_id *composition_variable_address;
	int i;
	struct Cmiss_variable_composition_type_specific_data *data;

	data=(struct Cmiss_variable_composition_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->dependent_variable)&&
		(0<data->number_of_source_variables)&&(data->independent_variables)&&
		(data->source_variables),return_code,0)
	{
		i=data->number_of_source_variables;
		composition_variable_address=data->independent_variables;
		while ((i>0)&&!Cmiss_variable_same_variable(independent_variable,
			*composition_variable_address))
		{
			i--;
			composition_variable_address++;
		}
		if (0==i)
		{
			return_code=Cmiss_variable_get_independent_variable_value(
				data->dependent_variable,independent_variable,value);
		}
		i=data->number_of_source_variables;
		composition_variable_address=data->source_variables;
		while (i>0)
		{
			if (Cmiss_variable_get_independent_variable_value(
				*composition_variable_address,independent_variable,value))
			{
				return_code=1;
			}
			i--;
			composition_variable_address++;
		}
	}
}
END_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	composition)

static
	START_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	composition)
{
	Cmiss_variable_id *composition_variable_address;
	int i;
	struct Cmiss_variable_composition_type_specific_data *data;

	data=(struct Cmiss_variable_composition_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->dependent_variable)&&
		(0<data->number_of_source_variables)&&(data->independent_variables)&&
		(data->source_variables),return_code,0)
	{
		i=data->number_of_source_variables;
		composition_variable_address=data->independent_variables;
		while ((i>0)&&!Cmiss_variable_same_variable(independent_variable,
			*composition_variable_address))
		{
			i--;
			composition_variable_address++;
		}
		if (0==i)
		{
			return_code=Cmiss_variable_get_set_independent_variable_value(
				data->dependent_variable,independent_variable,value);
		}
		i=data->number_of_source_variables;
		composition_variable_address=data->source_variables;
		while (i>0)
		{
			if (Cmiss_variable_get_set_independent_variable_value(
				*composition_variable_address,independent_variable,value))
			{
				return_code=1;
			}
			i--;
			composition_variable_address++;
		}
	}
}
END_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	composition)

static START_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(composition)
{
	struct Cmiss_variable_composition_type_specific_data *data;

	data=(struct Cmiss_variable_composition_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->dependent_variable)&&
		(0<data->number_of_source_variables)&&(data->independent_variables)&&
		(data->source_variables),return_code,0)
	{
		return_code=Cmiss_variable_get_value_type(data->dependent_variable,type);
	}
}
END_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(composition)

static START_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(composition)
{
	Cmiss_variable_id *independent_variable_address,*source_variable_address;
	int i;
	struct Cmiss_variable_composition_type_specific_data *data;
	struct Cmiss_variable_value *temp_variable_value;
	struct LIST(Cmiss_variable_value) *augmented_values;

	data=(struct Cmiss_variable_composition_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->dependent_variable)&&
		(0<data->number_of_source_variables)&&(data->independent_variables)&&
		(data->source_variables),return_code,0)
	{
		if (augmented_values=CREATE_LIST(Cmiss_variable_value)())
		{
			return_code=COPY_LIST(Cmiss_variable_value)(augmented_values,values);
			i=data->number_of_source_variables;
			independent_variable_address=data->independent_variables;
			source_variable_address=data->source_variables;
			while (return_code&&(i>0))
			{
				if (return_code=Cmiss_variable_is_defined(*source_variable_address,
					values))
				{
					if (temp_variable_value=CREATE(Cmiss_variable_value)(
						*independent_variable_address,(Cmiss_value_id)NULL))
					{
						ACCESS(Cmiss_variable_value)(temp_variable_value);
						if (return_code=ADD_OBJECT_TO_LIST(Cmiss_variable_value)(
							temp_variable_value,augmented_values))
						{
							i--;
							independent_variable_address++;
							source_variable_address++;
						}
						DEACCESS(Cmiss_variable_value)(&temp_variable_value);
					}
					else
					{
						return_code=0;
					}
				}
			}
			if (return_code)
			{
				return_code=Cmiss_variable_is_defined(data->dependent_variable,
					augmented_values);
			}
			DESTROY_LIST(Cmiss_variable_value)(&augmented_values);
		}
	}
}
END_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(composition)

static START_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	composition)
{
	Cmiss_variable_id *composition_variable_address;
	int i;
	struct Cmiss_variable_composition_type_specific_data *data;

	data=(struct Cmiss_variable_composition_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->dependent_variable)&&
		(0<data->number_of_source_variables)&&(data->independent_variables)&&
		(data->source_variables),return_code,0)
	{
		if (return_code=Cmiss_variable_is_independent_variable_of(
			data->dependent_variable,independent_variable))
		{
			i=data->number_of_source_variables;
			composition_variable_address=data->independent_variables;
			while ((i>0)&&!Cmiss_variable_same_variable(independent_variable,
				*composition_variable_address))
			{
				i--;
				composition_variable_address++;
			}
			if (0!=i)
			{
				return_code=0;
			}
		}
		i=data->number_of_source_variables;
		composition_variable_address=data->source_variables;
		while (!return_code&&(i>0))
		{
			return_code=Cmiss_variable_is_independent_variable_of(
				*composition_variable_address,independent_variable);
			i--;
			composition_variable_address++;
		}
	}
}
END_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	composition)

static START_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(composition)
{
	int i;
	struct Cmiss_variable_composition_type_specific_data *data;

	data=(struct Cmiss_variable_composition_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->dependent_variable)&&
		(0<data->number_of_source_variables)&&(data->independent_variables)&&
		(data->source_variables),return_code,0)
	{
		return_code=Cmiss_variable_not_in_use(data->dependent_variable);
		i=0;
		while (return_code&&(i<data->number_of_source_variables))
		{
			return_code=Cmiss_variable_not_in_use((data->source_variables)[i]);
			i++;
		}
	}
}
END_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(composition)

static START_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(composition)
{
	struct Cmiss_variable_composition_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_variable_composition_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_1);
	data_2=(struct Cmiss_variable_composition_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&(data_1->dependent_variable)&&
		(0<data_1->number_of_source_variables)&&(data_1->independent_variables)&&
		(data_1->source_variables)&&data_2&&(data_2->dependent_variable)&&
		(0<data_2->number_of_source_variables)&&(data_2->independent_variables)&&
		(data_2->source_variables),return_code,0)
	{
		return_code=Cmiss_variable_overlap(data_1->dependent_variable,
			data_2->dependent_variable);
	}
}
END_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(composition)

static START_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(composition)
{
	int i,j;
	struct Cmiss_variable_composition_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_variable_composition_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_1);
	data_2=(struct Cmiss_variable_composition_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&(data_1->dependent_variable)&&
		(0<data_1->number_of_source_variables)&&(data_1->independent_variables)&&
		(data_1->source_variables)&&data_2&&(data_2->dependent_variable)&&
		(0<data_2->number_of_source_variables)&&(data_2->independent_variables)&&
		(data_2->source_variables),return_code,0)
	{
		if (data_1->number_of_source_variables==data_2->number_of_source_variables)
		{
			if (return_code=Cmiss_variable_same_variable(data_1->dependent_variable,
				data_2->dependent_variable))
			{
				i=0;
				do
				{
					j=0;
					do
					{
						if (Cmiss_variable_same_variable((data_1->independent_variables)[i],
							(data_2->independent_variables)[j]))
						{
							return_code=Cmiss_variable_same_variable(
								(data_1->source_variables)[i],(data_2->source_variables)[j]);
						}
						j++;
					} while (!return_code&&(j<data_2->number_of_source_variables));
					i++;
				} while (return_code&&(i<data_1->number_of_source_variables));
			}
		}
	}
}
END_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(composition)

static
	START_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	composition)
{
	Cmiss_variable_id *composition_variable_address;
	int i;
	struct Cmiss_variable_composition_type_specific_data *data;

	data=(struct Cmiss_variable_composition_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->dependent_variable)&&
		(0<data->number_of_source_variables)&&(data->independent_variables)&&
		(data->source_variables),return_code,0)
	{
		i=data->number_of_source_variables;
		composition_variable_address=data->independent_variables;
		while ((i>0)&&!Cmiss_variable_same_variable(independent_variable,
			*composition_variable_address))
		{
			i--;
			composition_variable_address++;
		}
		if (0==i)
		{
			return_code=Cmiss_variable_set_independent_variable_value(
				data->dependent_variable,independent_variable,value);
		}
		i=data->number_of_source_variables;
		composition_variable_address=data->source_variables;
		while (i>0)
		{
			if (Cmiss_variable_set_independent_variable_value(
				*composition_variable_address,independent_variable,value))
			{
				return_code=1;
			}
			i--;
			composition_variable_address++;
		}
	}
}
END_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	composition)

/*
Global functions
----------------
*/
int Cmiss_variable_composition_set_type(Cmiss_variable_id composition,
	Cmiss_variable_id dependent_variable,int number_of_source_variables,
	Cmiss_variable_id *source_variables,Cmiss_variable_id *independent_variables)
/*******************************************************************************
LAST MODIFIED : 10 July 2003

DESCRIPTION :
Sets <composition> to be the <dependent_variable> with its
<independent_variables> calculated using the corresponding <source_variables>.

This function ACCESSes the <dependent_variable>, <source_variables> and
<independent_variables>.  After success, the <composition> is responsible for
DEACCESS/DEALLOCATEing <dependent_variable>, <source_variables> and
<independent_variables>.
==============================================================================*/
{
	int i,return_code;
	struct Cmiss_variable_composition_type_specific_data *data;

	ENTER(Cmiss_variable_composition_set_type);
	return_code=0;
	/* check arguments */
	if (composition&&dependent_variable&&(0<number_of_source_variables)&&
		source_variables&&independent_variables)
	{
		i=0;
		while ((i<number_of_source_variables)&&source_variables[i]&&
			independent_variables[i]&&Cmiss_variable_same_value_type(
			source_variables[i],independent_variables[i]))
		{
			i++;
		}
		if (i==number_of_source_variables)
		{
			/* 1.  Make dynamic allocations for any new type-specific data */
			if (ALLOCATE(data,struct Cmiss_variable_composition_type_specific_data,1))
			{
				/* 2.  Clear current type-specific data */
				Cmiss_variable_clear_type(composition);
				/* 3.  Establish the new type */
				if (return_code=Cmiss_variable_set_type_specific_information(
					composition,Cmiss_variable_composition_type_string,(void *)data))
				{
					data->number_of_source_variables=number_of_source_variables;
					data->dependent_variable=ACCESS(Cmiss_variable)(dependent_variable);
					data->source_variables=source_variables;
					data->independent_variables=independent_variables;
					for (i=0;i<number_of_source_variables;i++)
					{
						ACCESS(Cmiss_variable)(source_variables[i]);
						ACCESS(Cmiss_variable)(independent_variables[i]);
					}
					/* set all the methods */
					return_code=CMISS_VARIABLE_ESTABLISH_METHODS(composition,composition);
				}
				else
				{
					display_message(ERROR_MESSAGE,"Cmiss_variable_composition_set_type.  "
						"Could not set type specific information");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Cmiss_variable_composition_set_type.  "
					"Could not ALLOCATE type specific data");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_composition_set_type.  "
				"Missing independent variable(s)");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_composition_set_type.  "
			"Invalid argument(s).  %p %p %d %p %p\n",composition,dependent_variable,
			number_of_source_variables,source_variables,independent_variables);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_composition_set_type */

#if defined (OLD_CODE)
/*???DB.  When source/independent were in struct Cmiss_variable */
int Cmiss_variable_set_independent_variable_source_variable(
	Cmiss_variable_id variable,
	Cmiss_variable_id independent_variable,
	Cmiss_variable_id source_variable)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Sets the values for <independent_variable> of the <variable> to come from the
<source_variable>.  The types of the values for <independent_variable> and
<source_variable> must match.  If <source_variable> is NULL then a previous
setting of the source variable for <independent_variable> is cleared.
==============================================================================*/
{
	int i,j,overlapping_variables,return_code;
	Cmiss_variable_id *independent_variable_address,*source_variable_address;

	ENTER(Cmiss_variable_set_independent_variable_source_variable);
	return_code=0;
	if (variable&&independent_variable)
	{
		/* check that results of <independent_variable> and <source_variable>
			match */
		if (!source_variable||Cmiss_variable_same_value_type(
			independent_variable,source_variable))
		{
			/* pass on to source variables */
			return_code=1;
			i=variable->number_of_source_variables;
			j=0;
			overlapping_variables=0;
			independent_variable_address=variable->independent_variables;
			source_variable_address=variable->source_variables;
			while (return_code&&(i>0))
			{
				if (j>0)
				{
					*(independent_variable_address-j)= *independent_variable_address;
					*(source_variable_address-j)= *source_variable_address;
				}
				if (Cmiss_variable_same_variable(*independent_variable_address,
					independent_variable))
				{
					DEACCESS(Cmiss_variable)(independent_variable_address);
					DEACCESS(Cmiss_variable)(source_variable_address);
					j++;
				}
				else
				{
					if (!source_variable&&Cmiss_variable_overlap(
						*independent_variable_address,independent_variable))
					{
						overlapping_variables=1;
					}
					return_code=
						Cmiss_variable_set_independent_variable_source_variable(
						*source_variable_address,independent_variable,source_variable);
				}
				i--;
				source_variable_address++;
				independent_variable_address++;
			}
			variable->number_of_source_variables -= j;
			if (return_code)
			{
				/* add <source_variable> if it is an <independent_variable> of
					<variable> */
					/*???DB.  Immediate independent variable */
				if ((source_variable||overlapping_variables)&&
					Cmiss_variable_is_independent_variable_of(variable,
					independent_variable))
				{
					if (j<1)
					{
						if (REALLOCATE(source_variable_address,variable->source_variables,
							Cmiss_variable_id,(variable->number_of_source_variables)+1))
						{
							variable->source_variables=source_variable_address;
							if (REALLOCATE(independent_variable_address,
								variable->independent_variables,Cmiss_variable_id ,
								(variable->number_of_source_variables)+1))
							{
								variable->independent_variables=independent_variable_address;
							}
						}
					}
					else
					{
						independent_variable_address=variable->independent_variables;
						source_variable_address=variable->source_variables;
					}
					if (independent_variable_address&&source_variable_address)
					{
						independent_variable_address[variable->number_of_source_variables]=
							ACCESS(Cmiss_variable)(independent_variable);
						source_variable_address[variable->number_of_source_variables]=
							ACCESS(Cmiss_variable)(source_variable);
						(variable->number_of_source_variables)++;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_variable_set_independent_variable_source_variable.  "
							"Insufficient memory for extending source/independent variable "
							"pairs");
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_set_independent_variable_source_variable.  "
				"Source and independent value type mis-match");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_set_independent_variable_source_variable.  "
			"Invalid argument(s).  %p %p",variable,independent_variable);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_set_independent_variable_source_variable */
#endif /* defined (OLD_CODE) */


DECLARE_CMISS_VARIABLE_IS_TYPE_FUNCTION(composition)
