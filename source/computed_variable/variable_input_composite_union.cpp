//******************************************************************************
// FILE : variable_input_composite_union.cpp
//
// LAST MODIFIED : 4 February 2004
//
// DESCRIPTION :
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

#include "computed_variable/variable_base.hpp"

#include <algorithm>
#include <iterator>
#include <typeinfo>
#include <iostream>

#include "computed_variable/variable_input_composite_union.hpp"

#if defined (GENERALIZE_COMPOSITE_INPUT)
#include "computed_variable/variable_derivative_matrix.hpp"

// class Variable_input_composite_union
// ------------------------------------

Variable_input_composite_union::Variable_input_composite_union(
	const Variable_input_handle& input_1,const Variable_input_handle& input_2):
	Variable_input_composite(),inputs_list(0)
//******************************************************************************
// LAST MODIFIED : 12 January 2004
//
// DESCRIPTION :
// Constructor.  Needs to "flatten" the <inputs_list> ie expand any composites.
//==============================================================================
{
	Variable_input_composite_union_handle input_1_composite_union=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_composite_union,Variable_input>(
		input_1);
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_composite_union *>(input_1);
#endif /* defined (USE_SMART_POINTER) */
	Variable_input_composite_union_handle input_2_composite_union=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_composite_union,Variable_input>(
		input_2);
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_composite_union *>(input_2);
#endif /* defined (USE_SMART_POINTER) */

	//  Variable_composite rather than deriving Variable_input_composite
	if (input_1_composite_union)
	{
		inputs_list.insert(inputs_list.end(),
			(input_1_composite_union->inputs_list).begin(),
			(input_1_composite_union->inputs_list).end());
	}
	else
	{
		inputs_list.push_back(input_1);
	}
	if (input_2_composite_union)
	{
		inputs_list.insert(inputs_list.end(),
			(input_2_composite_union->inputs_list).begin(),
			(input_2_composite_union->inputs_list).end());
	}
	else
	{
		inputs_list.push_back(input_2);
	}
}

Variable_input_composite_union::Variable_input_composite_union(
	std::list<Variable_input_handle>& inputs_list) : Variable_input_composite(),
	inputs_list(0)
//******************************************************************************
// LAST MODIFIED : 12 January 2004
//
// DESCRIPTION :
// Constructor.  Needs to "flatten" the <inputs_list> ie expand any unions.
//
// ???DB.  Is flattening necessary or desirable?
//==============================================================================
{
	std::list<Variable_input_handle>::iterator input_iterator;
	Variable_size_type i;

	// "flatten" the <inputs_list>.  Does not need to be recursive because
	//   composite union inputs have flat lists
	input_iterator=inputs_list.begin();
	for (i=inputs_list.size();i>0;i--)
	{
		Variable_input_composite_union_handle input_composite_union=
#if defined (USE_SMART_POINTER)
			boost::dynamic_pointer_cast<Variable_input_composite_union,
			Variable_input>(*input_iterator);
#else /* defined (USE_SMART_POINTER) */
			dynamic_cast<Variable_input_composite_union *>(*input_iterator);
#endif /* defined (USE_SMART_POINTER) */

		if (input_composite_union)
		{
			(this->inputs_list).insert((this->inputs_list).end(),
				(input_composite_union->inputs_list).begin(),
				(input_composite_union->inputs_list).end());
		}
		else
		{
			(this->inputs_list).push_back(*input_iterator);
		}
		input_iterator++;
	}
}

Variable_input_composite_union& Variable_input_composite_union::operator=(
	const Variable_input_composite_union& input_composite_union)
//******************************************************************************
// LAST MODIFIED : 9 October 2003
//
// DESCRIPTION :
// Assignment operator.
//???DB.  Same as implicit?
//==============================================================================
{
	//???DB.  Does assignment for super class first?
	inputs_list=input_composite_union.inputs_list;

	return (*this);
}

class Variable_input_composite_union_evaluate_derivative_functor
//******************************************************************************
// LAST MODIFIED : 13 January 2004
//
// DESCRIPTION :
// A unary functor (Functor) for merging derivative matrices for the different
// components of a composite_union input
//==============================================================================
{
	public:
		Variable_input_composite_union_evaluate_derivative_functor(
			const Variable_handle& variable,
			std::list<Variable_input_handle>& independent_variables,
			std::list<Variable_input_value_handle>& values,
			std::list<Variable_input_handle>::iterator& input_iterator,
			Variable_derivative_matrix_handle& result):input_iterator(input_iterator),
			independent_variables(independent_variables),values(values),
			independent_variable_number_of_values(0),result(result),variable(variable)
		{
			std::list<Matrix>::iterator result_iterator;
			std::list<Variable_input_handle>::iterator independent_variable_iterator;
			Variable_size_type i,j;

			input_index=0;
			result_input_iterator=result->independent_variables.begin();
			for (independent_variable_iterator=independent_variables.begin();
				independent_variable_iterator!=input_iterator;
				independent_variable_iterator++)
			{
				input_index++;
				result_input_iterator++;
			}
			independent_variable_number_of_values.resize(input_index);
			result_iterator=(result->matrices).begin();
			number_of_matrices=1;
			for (i=0;i<input_index;i++)
			{
				independent_variable_number_of_values[i]=result_iterator->size2();
				for (j=number_of_matrices;j>0;j--)
				{
					result_iterator++;
				}
				number_of_matrices *= 2;
			}
			for (i=independent_variables.size()-input_index;i>0;i--)
			{
				number_of_matrices *= 2;
			}
			number_of_matrices -= 1;
		};
		~Variable_input_composite_union_evaluate_derivative_functor() {};
		int operator() (Variable_input_handle& input)
		{
			int composite_independent_variable_factor,
				composite_independent_variable_step;
			std::list<Matrix>::iterator derivative_iterator,result_iterator;
			unsigned i,j,matrix_number,number_of_rows;
			Variable_derivative_matrix_handle derivative;
			Variable_handle derivative_uncast;
			Variable_size_type derivative_independent_number_of_values,
				result_independent_number_of_values;

			*input_iterator=input;
			// calculate the derivative for input
			derivative_uncast=variable->evaluate_derivative(independent_variables,
				values);
			derivative=
#if defined (USE_SMART_POINTER)
				boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>(
				derivative_uncast);
#else /* defined (USE_SMART_POINTER) */
				dynamic_cast<Variable_derivative_matrix *>(derivative_uncast);
#endif /* defined (USE_SMART_POINTER) */
			if (derivative)
			{
				// merge derivative matrix into result
				Assert((number_of_matrices==(derivative->matrices).size())&&
					(number_of_matrices==(result->matrices).size()),std::logic_error(
					"Variable_input_composite_union_evaluate_derivative_functor().  "
					"Incorrect number_of_matrices"));
				composite_independent_variable_factor=1;
				derivative_iterator=(derivative->matrices).begin();
				result_iterator=(result->matrices).begin();
				for (i=input_index;i>0;i--)
				{
					for (j=composite_independent_variable_factor;j>0;j--)
					{
						derivative_iterator++;
						result_iterator++;
					}
					composite_independent_variable_factor *= 2;
				}
				derivative_independent_number_of_values=derivative_iterator->size2();
				result_independent_number_of_values=result_iterator->size2();
				derivative_iterator=(derivative->matrices).begin();
				result_iterator=(result->matrices).begin();
				number_of_rows=result_iterator->size1();
				for (matrix_number=1;matrix_number<=number_of_matrices;matrix_number++)
				{
					Assert((number_of_rows==derivative_iterator->size1())&&
						(number_of_rows==result_iterator->size1()),std::logic_error(
						"Variable_input_composite_union_evaluate_derivative_functor().  "
						"Incorrect number_of_rows"));
					if ((matrix_number/composite_independent_variable_factor)%2)
					{
						/* derivative involves composite independent variable */
						int column_number,derivative_column_number,number_of_columns=
							(result_iterator->size2)()+(derivative_iterator->size2)(),
							result_column_number;
						Matrix matrix(number_of_rows,number_of_columns);

						composite_independent_variable_step=1;
						j=matrix_number;
						for (i=0;i<input_index;i++)
						{
							if (j%2)
							{
								composite_independent_variable_step *=
									independent_variable_number_of_values[i];
							}
							j /= 2;
						}
						derivative_column_number=0;
						result_column_number=0;
						column_number=0;
						while (column_number<number_of_columns)
						{
							for (j=composite_independent_variable_step*
								result_independent_number_of_values;j>0;j--)
							{
								for (i=0;i<number_of_rows;i++)
								{
									matrix(i,column_number)=
										(*result_iterator)(i,result_column_number);
								}
								result_column_number++;
								column_number++;
							}
							for (j=composite_independent_variable_step*
								derivative_independent_number_of_values;j>0;j--)
							{
								for (i=0;i<number_of_rows;i++)
								{
									matrix(i,column_number)=
										(*derivative_iterator)(i,derivative_column_number);
								}
								derivative_column_number++;
								column_number++;
							}
						}
						// update result matrix
						result_iterator->resize(number_of_rows,number_of_columns);
							//???DB.  Memory leak?
						*result_iterator=matrix;
					}
					derivative_iterator++;
					result_iterator++;
				}
			}
			else
			{
				//???DB.  Currently only know how to merge derivative matrices
			}
			// update independent variables for result
				//???DB.  Memory leak?
			*result_input_iterator=Variable_input_handle(
				new Variable_input_composite_union(*result_input_iterator,input));

			return (1);
		};
	private:
		std::list<Variable_input_handle>::iterator& input_iterator;
		std::list<Variable_input_handle>& independent_variables;
		std::list<Variable_input_handle>::iterator result_input_iterator;
		std::list<Variable_input_value_handle>& values;
		std::vector<Variable_size_type> independent_variable_number_of_values;
		Variable_derivative_matrix_handle& result;
		Variable_handle variable;
		Variable_size_type input_index,number_of_matrices;
};

Variable_handle Variable_input_composite_union::evaluate_derivative(
	Variable_handle dependent_variable,
	std::list<Variable_input_handle>& independent_variables,
	std::list<Variable_input_handle>::iterator& composite_independent_variable,
	std::list<Variable_input_value_handle>& values)
//******************************************************************************
// LAST MODIFIED : 13 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Variable_input_handle>::iterator input_composite_iterator;
	std::list<Variable_input_value_handle> current_values(0),no_values(0);
	Variable_derivative_matrix_handle derivative_matrix;
	Variable_handle derivative;

	// get the specified values
	std::for_each(values.begin(),values.end(),Variable_get_input_values(
		dependent_variable,current_values));
	// override the specified values
	std::for_each(values.begin(),values.end(),Variable_set_input_values(
		dependent_variable));
	input_composite_iterator=inputs_list.begin();
	*composite_independent_variable= *input_composite_iterator;
	derivative=dependent_variable->evaluate_derivative(independent_variables,
		no_values);
	derivative_matrix=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>(
		derivative);
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_derivative_matrix *>(*derivative);
#endif /* defined (USE_SMART_POINTER) */
	input_composite_iterator++;
	Variable_input_composite_union_evaluate_derivative_functor
		evaluate_derivative_functor(dependent_variable,independent_variables,
		no_values,composite_independent_variable,derivative_matrix);
	std::for_each(input_composite_iterator,inputs_list.end(),
		evaluate_derivative_functor);
	*composite_independent_variable=Variable_input_handle(this);
	// need to set independent_variables because evaluate_derivative copies
	derivative_matrix->independent_variables=independent_variables;
	// reset the current values
	std::for_each(current_values.rbegin(),current_values.rend(),
		Variable_set_input_values(dependent_variable));

	return (derivative);
}

Variable_handle Variable_input_composite_union::get_input_value(
	Variable_handle
	//???DB.  To be done
	//variable
	)
//******************************************************************************
// LAST MODIFIED : 14 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	Variable_handle result(0);

	//???DB.  To be done

	return (result);
}

bool Variable_input_composite_union::operator==(const Variable_input& input)
//******************************************************************************
// LAST MODIFIED : 26 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	try
	{
		const Variable_input_composite_union& input_composite_union=
			dynamic_cast<const Variable_input_composite_union&>(input);

		return (inputs_list==input_composite_union.inputs_list);
	}
	catch (std::bad_cast)
	{
		return (false);
	}
}

#if defined (USE_SCALAR_MAPPING)
std::list< std::pair<Variable_size_type,Variable_size_type> >
	Variable_input_composite_union::scalar_mapping_target(
	Variable_input_handle
//???DB.  To be done	source
	)
//******************************************************************************
// LAST MODIFIED : 13 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	std::list< std::pair<Variable_size_type,Variable_size_type> > result(0);

	//???DB.  To be done

	return (result);
}
#endif // defined (USE_SCALAR_MAPPING)

#if defined (USE_SCALAR_MAPPING)
std::list< std::pair<Variable_size_type,Variable_size_type> >
	Variable_input_composite_union::scalar_mapping_local(
	Variable_input_handle
//???DB.  To be done	target
	)
//******************************************************************************
// LAST MODIFIED : 13 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	std::list< std::pair<Variable_size_type,Variable_size_type> > result(0);

	//???DB.  To be done

	return (result);
}
#endif // defined (USE_SCALAR_MAPPING)

#endif // defined (GENERALIZE_COMPOSITE_INPUT)
