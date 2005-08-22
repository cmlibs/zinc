//******************************************************************************
// FILE : variable_composite.cpp
//
// LAST MODIFIED : 9 February 2004
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

#include <new>
#include <sstream>
#include <string>
#include <stdio.h>

#include "computed_variable/variable_composite.hpp"
#include "computed_variable/variable_derivative_matrix.hpp"
#include "computed_variable/variable_vector.hpp"

// global classes
// ==============

// class Variable_composite
// ------------------------

Variable_composite::Variable_composite(
	std::list<Variable_handle>& variables_list):Variable(),
	variables_list(0)
//******************************************************************************
// LAST MODIFIED : 24 November 2003
//
// DESCRIPTION :
// Constructor.  Needs to "flatten" the <variables_list> ie expand any
// composites.
//==============================================================================
{
	std::list<Variable_handle>::iterator variable_iterator;
	Variable_size_type i;

	// "flatten" the <variables_list>.  Does not need to be recursive because
	//   composite variables have flat lists
	variable_iterator=variables_list.begin();
	for (i=variables_list.size();i>0;i--)
	{
		Variable_composite_handle variable_composite=
#if defined (USE_SMART_POINTER)
			boost::dynamic_pointer_cast<Variable_composite,Variable>(
			*variable_iterator);
#else /* defined (USE_SMART_POINTER) */
			dynamic_cast<Variable_composite *>(*variable_iterator);
#endif /* defined (USE_SMART_POINTER) */

		if (variable_composite)
		{
			(this->variables_list).insert((this->variables_list).end(),
				(variable_composite->variables_list).begin(),
				(variable_composite->variables_list).end());
		}
		else
		{
			(this->variables_list).push_back(*variable_iterator);
		}
		variable_iterator++;
	}
}

Variable_composite::Variable_composite(
	const Variable_composite& variable_composite):Variable(),
	variables_list(variable_composite.variables_list)
//******************************************************************************
// LAST MODIFIED : 16 November 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Variable_composite& Variable_composite::operator=(
	const Variable_composite& variable_composite)
//******************************************************************************
// LAST MODIFIED : 16 November 2003
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	variables_list=variable_composite.variables_list;

	return (*this);
}

Variable_composite::~Variable_composite()
//******************************************************************************
// LAST MODIFIED : 16 November 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

Variable_handle Variable_composite::clone() const
//******************************************************************************
// LAST MODIFIED : 8 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (Variable_composite_handle(new Variable_composite(*this)));
}

class Variable_composite_evaluate_functor
//******************************************************************************
// LAST MODIFIED : 20 January 2004
//
// DESCRIPTION :
// A unary function (functor) for evaluating a composite variable.
//==============================================================================
{
	public:
		Variable_composite_evaluate_functor(Variable_handle& result,
			std::list<Variable_input_value_handle>& values):
			result_composite(0),result_vector(0),result(result),values(values)
		{
		};
		int operator() (Variable_handle& variable)
		{
			Variable_handle result_part=(variable->evaluate)(values);

			if (!result_composite)
			{
				std::list<Variable_handle> variables_list(0);

				variables_list.push_back(result_part);
				result_composite=Variable_composite_handle(new Variable_composite(
					variables_list));
			}
			else
			{
				(result_composite->variables_list).push_back(result_part);
			}
			if (result_composite)
			{
				if (result!=result_composite)
				{
					Variable_vector_handle variable_vector=
#if defined (USE_SMART_POINTER)
						boost::dynamic_pointer_cast<Variable_vector,Variable>(
						result_part);
#else /* defined (USE_SMART_POINTER) */
						dynamic_cast<Variable_vector *>(result_part);
#endif /* defined (USE_SMART_POINTER) */

					if (variable_vector)
					{
						if (result_vector)
						{
#if defined (USE_ITERATORS)
							//???DB.  To be done
#else // defined (USE_ITERATORS)
							Vector *vector_1=result_vector->scalars(),
								*vector_2=variable_vector->scalars();
							Variable_size_type i,j,size_1=vector_1->size(),
								size_2=vector_2->size();
							Vector vector_composite(size_1+size_2);

							for (i=0;i<size_1;i++)
							{
								vector_composite[i]=(*vector_1)[i];
							}
							for (j=0;j<size_2;j++)
							{
								vector_composite[i]=(*vector_2)[j];
								i++;
							}
							result_vector=Variable_vector_handle(new Variable_vector(
								vector_composite));
#endif // defined (USE_ITERATORS)
						}
						else
						{
							result_vector=variable_vector;
						}
						result=result_vector;
					}
					else
					{
						result_vector=0;
						result=result_composite;
					}
				}
			}

			return (0);
		};
	private:
		Variable_composite_handle result_composite;
		Variable_vector_handle result_vector;
		Variable_handle& result;
		std::list<Variable_input_value_handle>& values;
};

Variable_handle Variable_composite::evaluate(
	std::list<Variable_input_value_handle>& values)
//******************************************************************************
// LAST MODIFIED : 1 January 2004
//
// DESCRIPTION :
// Overload Variable::evaluate.
//==============================================================================
{
	std::list<Variable_input_value_handle> current_values(0),no_values(0);
	Variable_handle result(0);

	// get the specified values
	std::for_each(values.begin(),values.end(),Variable_get_input_values(
		Variable_handle(this),current_values));
	// override the specified values
	std::for_each(values.begin(),values.end(),Variable_set_input_values(
		Variable_handle(this)));
	std::for_each(variables_list.begin(),variables_list.end(),
		Variable_composite_evaluate_functor(result,no_values));
	// reset the current values
	std::for_each(current_values.rbegin(),current_values.rend(),
		Variable_set_input_values(Variable_handle(this)));

	return (result);
}

class Variable_composite_evaluate_derivative_functor
//******************************************************************************
// LAST MODIFIED : 2 February 2004
//
// DESCRIPTION :
// A unary function (functor) for evaluating the derivative of a composite
// variable.
//==============================================================================
{
	public:
		Variable_composite_evaluate_derivative_functor(
			const Variable_composite_handle& dependent_variable,
			Variable_derivative_matrix_handle& result,
			std::list<
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			>& independent_variables,
			std::list<Variable_input_value_handle>& values):first(true),
			dependent_variable(dependent_variable),result(result),
			independent_variables(independent_variables),values(values)
		{
		};
		int operator() (Variable_handle& variable)
		{
			if (first||result)
			{
				Variable_handle result_part=(variable->evaluate_derivative)(
					independent_variables,values);

				if (first)
				{
					Variable_derivative_matrix_handle derivative=
#if defined (USE_SMART_POINTER)
						boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>(
						result_part);
#else /* defined (USE_SMART_POINTER) */
					dynamic_cast<Variable_derivative_matrix *>(result_part);
#endif /* defined (USE_SMART_POINTER) */

					if (derivative)
					{
						derivative->dependent_variable=dependent_variable;
						result=derivative;
					}
					first=false;
				}
				else
				{
					if (result)
					{
						Variable_derivative_matrix_handle derivative=
#if defined (USE_SMART_POINTER)
							boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>(
							result_part);
#else /* defined (USE_SMART_POINTER) */
						dynamic_cast<Variable_derivative_matrix *>(result_part);
#endif /* defined (USE_SMART_POINTER) */

						if (derivative)
						{
							std::list<Matrix>::iterator derivative_iterator,result_iterator;
							Variable_size_type derivative_number_of_rows,i,j,k,matrix_number,
								number_of_matrices,number_of_rows,result_number_of_rows;

							/* combine matrices */
							number_of_matrices=1;
							for (i=independent_variables.size();i>0;i--)
							{
								number_of_matrices *= 2;
							}
							number_of_matrices -= 1;
							Assert((number_of_matrices==(derivative->matrices).size())&&
								(number_of_matrices==(result->matrices).size()),
								std::logic_error(
								"Variable_composite_evaluate_derivative_functor().  "
								"Incorrect number_of_matrices"));
							derivative_iterator=(derivative->matrices).begin();
							result_iterator=(result->matrices).begin();
							if (0<number_of_matrices)
							{
								derivative_number_of_rows=derivative_iterator->size1();
								result_number_of_rows=result_iterator->size1();
								number_of_rows=derivative_number_of_rows+result_number_of_rows;
							}
							for (matrix_number=1;matrix_number<=number_of_matrices;
								matrix_number++)
							{
								Variable_size_type
									number_of_columns=result_iterator->size2();
								Matrix matrix(number_of_rows,number_of_columns);

								Assert((number_of_columns==derivative_iterator->size2())&&
									(derivative_number_of_rows==derivative_iterator->size1())&&
									(result_number_of_rows==result_iterator->size1()),
									std::logic_error(
									"Variable_composite_evaluate_derivative_functor().  "
									"Incorrect matrix size"));
								for (j=0;j<number_of_columns;j++)
								{
									for (i=0;i<result_number_of_rows;i++)
									{
										matrix(i,j)=(*result_iterator)(i,j);
									}
									for (k=0;k<derivative_number_of_rows;k++)
									{
										matrix(i,j)=(*derivative_iterator)(k,j);
										i++;
									}
								}
								// update result matrix
								result_iterator->resize(number_of_rows,number_of_columns);
								*result_iterator=matrix;
								result_iterator++;
								derivative_iterator++;
							}
						}
						else
						{
							result=0;
						}
					}
				}
			}

			return (0);
		};
	private:
		bool first;
		Variable_composite_handle dependent_variable;
		Variable_derivative_matrix_handle& result;
		std::list<
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			>& independent_variables;
		std::list<Variable_input_value_handle>& values;
};

Variable_handle Variable_composite::evaluate_derivative(
	std::list<
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	>& independent_variables,
	std::list<Variable_input_value_handle>& values)
//******************************************************************************
// LAST MODIFIED : 2 February 2004
//
// DESCRIPTION :
// Overload Variable::evaluate_derivative.
//==============================================================================
{
	std::list<Variable_input_value_handle> current_values(0),no_values(0);
	Variable_derivative_matrix_handle result(0);

	// get the specified values
	std::for_each(values.begin(),values.end(),Variable_get_input_values(
		Variable_handle(this),current_values));
	// override the specified values
	std::for_each(values.begin(),values.end(),Variable_set_input_values(
		Variable_handle(this)));
	// evaluate derivative
	std::for_each(variables_list.begin(),variables_list.end(),
		Variable_composite_evaluate_derivative_functor(
		Variable_composite_handle(this),result,independent_variables,no_values));
	// reset the current values
	std::for_each(current_values.rbegin(),current_values.rend(),
		Variable_set_input_values(Variable_handle(this)));

	return (result);
}

#if defined (USE_ITERATORS)
//???DB.  To be done
#else // defined (USE_ITERATORS)
class Variable_composite_calculate_size_functor
//******************************************************************************
// LAST MODIFIED : 11 December 2003
//
// DESCRIPTION :
// A unary function (functor) for calculating the size of a composite variable.
//==============================================================================
{
	public:
		Variable_composite_calculate_size_functor(Variable_size_type& size):
			size(size)
		{
			size=0;
		};
		int operator() (const Variable_handle& variable)
		{
			size += variable->size();

			return (0);
		};
	private:
		Variable_size_type& size;
};

Variable_size_type Variable_composite::size() const
//******************************************************************************
// LAST MODIFIED : 11 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_size_type result;

	// get the specified values
	std::for_each(variables_list.begin(),variables_list.end(),
		Variable_composite_calculate_size_functor(result));

	return (result);
}

Vector *Variable_composite::scalars()
//******************************************************************************
// LAST MODIFIED : 16 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Variable_handle>::iterator variable_iterator;
	Variable_size_type i,j,k,size_part;
	Vector *vector=new Vector(size()),*vector_part(0);

	variable_iterator=variables_list.begin();
	i=variables_list.size();
	j=0;
	while ((i>0)&&vector)
	{
		if (vector_part=(*variable_iterator)->scalars())
		{
			size_part=vector_part->size();
			for (k=0;k<size_part;k++)
			{
				(*vector)[j]=(*vector_part)[k];
				j++;
			}
			delete vector_part;
		}
		else
		{
			delete vector;
			vector=0;
		}
		i--;
	}

	return (vector);
}
#endif // defined (USE_ITERATORS)

Variable_handle Variable_composite::evaluate_local()
//******************************************************************************
// LAST MODIFIED : 16 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	// should not come here - handled by overloading Variable::evaluate
	Assert(false,std::logic_error(
		"Variable_composite::evaluate_local.  "
		"Should not come here"));

	return (0);
}

bool Variable_composite::evaluate_derivative_local(Matrix&,
	std::list<
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	>&)
//******************************************************************************
// LAST MODIFIED : 2 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	// should not come here - handled by overloading Variable::evaluate_derivative
	Assert(false,std::logic_error(
		"Variable_composite::evaluate_derivative_local.  "
		"Should not come here"));

	return (false);
}

Variable_handle Variable_composite::get_input_value_local(
	const
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	& input)
//******************************************************************************
// LAST MODIFIED : 2 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Variable_handle>::iterator variable_iterator=variables_list.begin();
	Variable_handle result(0);
	Variable_size_type i=variables_list.size();

	while ((i>0)&&!result)
	{
		result=(*variable_iterator)->get_input_value(input);
		variable_iterator++;
		i--;
	}

	return (result);
}

bool Variable_composite::set_input_value_local(
	const
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	& input,
	const
#if defined (USE_VARIABLES_AS_COMPONENTS)
	Variable_handle
#else // defined (USE_VARIABLES_AS_COMPONENTS)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
	& value)
//******************************************************************************
// LAST MODIFIED : 9 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	int result;
	std::list<Variable_handle>::iterator variable_iterator=variables_list.begin();
	Variable_size_type i=variables_list.size();

	result=false;
	while (i>0)
	{
		if ((*variable_iterator)->set_input_value(input,value))
		{
			result=true;
		}
		variable_iterator++;
		i--;
	}

	return (result);
}

string_handle Variable_composite::get_string_representation_local()
//******************************************************************************
// LAST MODIFIED : 17 November 2003
//
// DESCRIPTION :
//???DB.  Overload << instead of get_string_representation?
//==============================================================================
{
	std::list<Variable_handle>::iterator variable_iterator=variables_list.begin();
	std::ostringstream out;
	string_handle return_string;
	Variable_size_type i=variables_list.size();

	if (return_string=new std::string)
	{
		out << "[" << i << "](";
		while (i>0)
		{
			out << *((*variable_iterator)->get_string_representation());
			variable_iterator++;
			i--;
			if (i>0)
			{
				out << ",";
			}
		}
		out << ")";
		*return_string=out.str();
	}

	return (return_string);
}
