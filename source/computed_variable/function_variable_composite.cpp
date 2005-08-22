//******************************************************************************
// FILE : function_variable_composite.cpp
//
// LAST MODIFIED : 9 May 2005
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

#include <algorithm>
#include <iterator>
#include <typeinfo>
#include <iostream>
#include <sstream>

#include "computed_variable/function_composite.hpp"
#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable_composite.hpp"

#if defined (USE_FUNCTION_VARIABLE_COMPOSITE_EVALUATE)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#include "computed_variable/function_derivative.hpp"
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE_COMPOSITE_EVALUATE)

// module classes
// ==============

#if defined (USE_FUNCTION_VARIABLE_COMPOSITE_EVALUATE)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_derivatnew_composite
// -----------------------------------

class Function_derivatnew_composite : public Function_derivatnew
//******************************************************************************
// LAST MODIFIED : 13 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// for construction exception
		class Construction_exception {};
		// constructor
		Function_derivatnew_composite(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables);
		// destructor
		~Function_derivatnew_composite();
	// inherited
	private:
#if defined (EVALUATE_RETURNS_VALUE)
		virtual Function_handle evaluate(Function_variable_handle atomic_variable);
#else // defined (EVALUATE_RETURNS_VALUE)
		virtual bool evaluate(Function_variable_handle atomic_variable);
#endif // defined (EVALUATE_RETURNS_VALUE)
	private:
		std::list<Function_derivatnew_handle> derivatives_list;
};

Function_derivatnew_composite::Function_derivatnew_composite(
	const Function_variable_handle& dependent_variable,
	const std::list<Function_variable_handle>& independent_variables):
	Function_derivatnew(dependent_variable,independent_variables)
//******************************************************************************
// LAST MODIFIED : 9 May 2005
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	Function_composite_handle function_composite;
	Function_variable_composite_handle variable_composite;
	Function_variable_handle variable_composite_wrapped;

	if (variable_composite=boost::dynamic_pointer_cast<
		Function_variable_composite,Function_variable>(dependent_variable))
	{
		bool valid;
		Function_derivatnew_handle derivative;
		Function_size_type i;
		std::list<Function_variable_handle>::iterator variables_iterator;

		valid=true;
		i=(variable_composite->variables_list).size();
		variables_iterator=(variable_composite->variables_list).begin();
		while (valid&&(i>0))
		{
			if (derivative=boost::dynamic_pointer_cast<Function_derivatnew,
				Function>((*variables_iterator)->derivative(independent_variables)))
			{
				derivatives_list.push_back(derivative);
			}
			else
			{
				valid=false;
			}
			++variables_iterator;
			--i;
		}
		if (valid)
		{
			std::list<Function_derivatnew_handle>::iterator derivative_iterator;

			i=derivatives_list.size();
			derivative_iterator=derivatives_list.begin();
			while (i>0)
			{
				(*derivative_iterator)->add_dependent_function(this);
				--i;
				++derivative_iterator;
			}
		}
		else
		{
			throw Function_derivatnew_composite::Construction_exception();
		}
	}
	else
	{
		throw Function_derivatnew_composite::Construction_exception();
	}
}

Function_derivatnew_composite::~Function_derivatnew_composite()
//******************************************************************************
// LAST MODIFIED : 9 May 2005
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
#if defined (CIRCULAR_SMART_POINTERS)
	// do nothing
#else // defined (CIRCULAR_SMART_POINTERS)
	Function_size_type i;
	std::list<Function_derivatnew_handle>::iterator derivative_iterator;

	i=derivatives_list.size();
	derivative_iterator=derivatives_list.begin();
	while (i>0)
	{
		(*derivative_iterator)->remove_dependent_function(this);
		--i;
		++derivative_iterator;
	}
#endif // defined (CIRCULAR_SMART_POINTERS)
}

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_derivatnew_composite::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 14 April 2005
//
// DESCRIPTION :
//==============================================================================
{
#if defined (EVALUATE_RETURNS_VALUE)
	Function_handle result(0);
#else // defined (EVALUATE_RETURNS_VALUE)
	bool result(true);
#endif // defined (EVALUATE_RETURNS_VALUE)

	if (
#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
		equivalent(Function_handle(this),atomic_variable->function())&&
#endif // defined (EVALUATE_RETURNS_VALUE)
		!evaluated())
	{
		bool first,valid;
		Function_derivatnew_handle part;
		Function_size_type i,j,k,l,result_number_of_matrices,result_number_of_rows;
		Function_variable_handle part_output;
		std::list<Function_derivatnew_handle>::iterator part_iterator;
		std::list< std::list<Matrix>::iterator > part_matrix_iterators;
#if defined (OLD_CODE)
		Function_size_type order;
		std::vector<Function_size_type> numbers_of_independent_values;
#endif // defined (OLD_CODE)

#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
		result=false;
#endif // defined (EVALUATE_RETURNS_VALUE)
		valid=true;
		first=true;
		i=derivatives_list.size();
		part_iterator=derivatives_list.begin();
		result_number_of_rows=0;
		while (valid&&(i>0))
		{
			if ((part= *part_iterator)&&(part_output=part->output())&&
				(part_output->evaluate()))
			{
				Derivative_matrix& part_derivative_matrix=part->derivative_matrix;

				if (first)
				{
					first=false;
					result_number_of_matrices=part_derivative_matrix.size();
#if defined (OLD_CODE)
					order=part_derivative_matrix.order;
					numbers_of_independent_values=
						part_derivative_matrix.numbers_of_independent_values;
#endif // defined (OLD_CODE)
				}
				else
				{
#if defined (OLD_CODE)
					valid=(order==part_derivative_matrix.order)&&
						(result_number_of_matrices==part_derivative_matrix.size());
					j=order;
					while (valid&&(j>0))
					{
						j--;
						valid=(numbers_of_independent_values[j]==
							part_derivative_matrix.numbers_of_independent_values[j]);
					}
#else // defined (OLD_CODE)
					valid=(result_number_of_matrices==part_derivative_matrix.size());
#endif // defined (OLD_CODE)
				}
				part_matrix_iterators.push_back(part_derivative_matrix.begin());
				result_number_of_rows += part_derivative_matrix.front().size1();
			}
			else
			{
				valid=false;
			}
			i--;
			part_iterator++;
		}
		if (valid)
		{
			std::list<Matrix> matrices;
			derivative_matrix.clear();
			for (i=result_number_of_matrices;i>0;i--)
			{
				std::list< std::list<Matrix>::iterator >::iterator
					part_matrix_iterators_iterator=part_matrix_iterators.begin();
				Function_size_type result_number_of_columns=
					(*part_matrix_iterators_iterator)->size2(),result_row_number;;
				Matrix matrix(result_number_of_rows,result_number_of_columns);

				result_row_number=0;
				for (j=part_matrix_iterators.size();j>0;j--)
				{
					Matrix& part_matrix= **part_matrix_iterators_iterator;
					Function_size_type part_number_of_rows=part_matrix.size1();

					for (k=0;k<part_number_of_rows;k++)
					{
						for (l=0;l<result_number_of_columns;l++)
						{
							matrix(result_row_number,l)=part_matrix(k,l);
						}
						result_row_number++;
					}
					(*part_matrix_iterators_iterator)++;
					part_matrix_iterators_iterator++;
				}
				matrices.push_back(matrix);
			}
			try
			{
				derivative_matrix=Derivative_matrix(matrices);
				set_evaluated();
#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
				result=true;
#endif // defined (EVALUATE_RETURNS_VALUE)
			}
			catch (Derivative_matrix::Construction_exception)
			{
				// do nothing
			}
		}
	}
#if defined (EVALUATE_RETURNS_VALUE)
	//???DB.  To be done
	//???DB.  Has to find the appropriate derivative and row
	//???DB.  How does get_value work?
#endif // defined (EVALUATE_RETURNS_VALUE)

	return (result);
}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE_COMPOSITE_EVALUATE)


// class Function_variable_iterator_representation_atomic_composite
// ----------------------------------------------------------------

class Function_variable_iterator_representation_atomic_composite: public
	Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_composite(
			const bool begin,Function_variable_composite_handle source):
			atomic_variable_iterator(0),variables_list_iterator(0),
			source(source)
		{
			if (source)
			{
				if (begin)
				{
					variables_list_iterator=(source->variables_list).begin();
					while (
						(variables_list_iterator!=(source->variables_list).end())&&
						((atomic_variable_iterator=
						((*variables_list_iterator)->begin_atomic)())==
						((*variables_list_iterator)->end_atomic)()))
					{
						++variables_list_iterator;
					}
					if (variables_list_iterator==(source->variables_list).end())
					{
						atomic_variable_iterator=0;
					}
				}
				else
				{
					variables_list_iterator=(source->variables_list).end();
				}
			}
		};
		// a "virtual" constructor
		Function_variable_iterator_representation *clone()
		{
			Function_variable_iterator_representation *result;

			result=0;
			if (this)
			{
				result=new
					Function_variable_iterator_representation_atomic_composite(
					*this);
			}

			return (result);
		};
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_composite(){};
	private:
		// increment
		void increment()
		{
			if (variables_list_iterator!=(source->variables_list).end())
			{
				++atomic_variable_iterator;
				if (atomic_variable_iterator==
					((*variables_list_iterator)->end_atomic)())
				{
					do
					{
						++variables_list_iterator;
					} while (
						(variables_list_iterator!=(source->variables_list).end())&&
						((atomic_variable_iterator=
						((*variables_list_iterator)->begin_atomic)())==
						((*variables_list_iterator)->end_atomic)()));
					if (variables_list_iterator==(source->variables_list).end())
					{
						atomic_variable_iterator=0;
					}
				}
			}
		};
		// decrement
		void decrement()
		{
			if ((variables_list_iterator==
				(source->variables_list).end())||
				(atomic_variable_iterator==
				((*variables_list_iterator)->begin_atomic)()))
			{
				if (variables_list_iterator!=
					(source->variables_list).begin())
				{
					do
					{
						--variables_list_iterator;
						atomic_variable_iterator=((*variables_list_iterator)->end_atomic)();
					} while ((variables_list_iterator!=
						(source->variables_list).begin())&&
						(atomic_variable_iterator==
						((*variables_list_iterator)->begin_atomic)()));
					if ((variables_list_iterator==
						(source->variables_list).begin())&&
						(atomic_variable_iterator==
						((*variables_list_iterator)->begin_atomic)()))
					{
						variables_list_iterator=(source->variables_list).end();
						atomic_variable_iterator=0;
					}
					else
					{
						--atomic_variable_iterator;
					}
				}
				else
				{
					variables_list_iterator=(source->variables_list).end();
					atomic_variable_iterator=0;
				}
			}
			else
			{
				--atomic_variable_iterator;
			}
		};
		// equality
		bool equality(const Function_variable_iterator_representation
			* representation)
		{
			const Function_variable_iterator_representation_atomic_composite
				*representation_composite=dynamic_cast<
				const Function_variable_iterator_representation_atomic_composite *>(
				representation);

			return (representation_composite&&
				(variables_list_iterator==
				representation_composite->variables_list_iterator)&&
				(atomic_variable_iterator==
				representation_composite->atomic_variable_iterator));
		};
		// dereference
		Function_variable_handle dereference() const
		{
			return (*atomic_variable_iterator);
		};
	private:
		Function_variable_iterator_representation_atomic_composite(const
			Function_variable_iterator_representation_atomic_composite&
			representation):Function_variable_iterator_representation(),
			atomic_variable_iterator(representation.atomic_variable_iterator),
			variables_list_iterator(representation.variables_list_iterator),
			source(representation.source){};
	private:
		Function_variable_iterator atomic_variable_iterator;
		std::list<Function_variable_handle>::iterator variables_list_iterator;
		Function_variable_composite_handle source;
};

// global classes
// ==============

// class Function_variable_composite
// ---------------------------------

#if defined (COMPOSITE_FLATTENING)
Function_variable_composite::Function_variable_composite(
	const Function_variable_handle& variable_1,
	const Function_variable_handle& variable_2):
	Function_variable(Function_handle(0)),variables_list(0)
//******************************************************************************
// LAST MODIFIED : 30 June 2004
//
// DESCRIPTION :
// Constructor.  Needs to "flatten" the <variables_list> ie. expand any
// composites.
//==============================================================================
{
	Function_variable_composite_handle variable_1_composite=
		boost::dynamic_pointer_cast<Function_variable_composite,
		Function_variable>(variable_1);
	Function_variable_composite_handle variable_2_composite=
		boost::dynamic_pointer_cast<Function_variable_composite,
		Function_variable>(variable_2);

	if (variable_1_composite)
	{
		variables_list.insert(variables_list.end(),
			(variable_1_composite->variables_list).begin(),
			(variable_1_composite->variables_list).end());
	}
	else
	{
		variables_list.push_back(variable_1);
	}
	if (variable_2_composite)
	{
		variables_list.insert(variables_list.end(),
			(variable_2_composite->variables_list).begin(),
			(variable_2_composite->variables_list).end());
	}
	else
	{
		variables_list.push_back(variable_2);
	}
}
#else // defined (COMPOSITE_FLATTENING)
Function_variable_composite::Function_variable_composite(
	const Function_variable_handle& variable_1,
	const Function_variable_handle& variable_2):
	Function_variable(Function_handle(0)),variables_list(0)
//******************************************************************************
// LAST MODIFIED : 30 June 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	variables_list.push_back(variable_1);
	variables_list.push_back(variable_2);
}

Function_variable_composite::Function_variable_composite(
	const Function_handle& function,
	const Function_variable_handle& variable_1,
	const Function_variable_handle& variable_2):Function_variable(function),
	variables_list(0)
//******************************************************************************
// LAST MODIFIED : 10 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	variables_list.push_back(variable_1);
	variables_list.push_back(variable_2);
}
#endif // defined (COMPOSITE_FLATTENING)

#if defined (COMPOSITE_FLATTENING)
Function_variable_composite::Function_variable_composite(
	std::list<Function_variable_handle>& variables_list):
	Function_variable(Function_handle(0)),variables_list(0)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
// Constructor.  Needs to "flatten" the <variables_list> ie. expand any
// composites.
//==============================================================================
{
	std::list<Function_variable_handle>::iterator variable_iterator;
	Function_size_type i;

	// "flatten" the <variables_list>.  Does not need to be recursive because
	//   composite variables have flat lists
	variable_iterator=variables_list.begin();
	for (i=variables_list.size();i>0;--i)
	{
		if (*variable_iterator)
		{
			Function_variable_composite_handle variable_composite=
				boost::dynamic_pointer_cast<Function_variable_composite,
				Function_variable>(*variable_iterator);

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
		}
		++variable_iterator;
	}
}
#else // defined (COMPOSITE_FLATTENING)
Function_variable_composite::Function_variable_composite(
	std::list<Function_variable_handle>& variables_list):
	Function_variable(Function_handle(0)),variables_list(variables_list)
//******************************************************************************
// LAST MODIFIED : 30 June 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Function_variable_composite::Function_variable_composite(
	const Function_handle& function,
	std::list<Function_variable_handle>& variables_list):
	Function_variable(function),variables_list(variables_list)
//******************************************************************************
// LAST MODIFIED : 10 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}
#endif // defined (COMPOSITE_FLATTENING)

Function_variable_handle Function_variable_composite::clone() const
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle local_function=function();
	Function_size_type i;
	Function_variable_composite_handle result(0);
	std::list<Function_variable_handle> local_variables_list;

	if (0<(i=variables_list.size()))
	{
		std::list<Function_variable_handle>::const_iterator iterator;

		iterator=variables_list.begin();
		while (i>0)
		{
			if (*iterator)
			{
				local_variables_list.push_back((*iterator)->clone());
			}
			else
			{
				local_variables_list.push_back(Function_variable_handle(0));
			}
			++iterator;
			--i;
		}
	}
	if (result=Function_variable_composite_handle(new Function_variable_composite(
		local_function,local_variables_list)))
	{
		result->value_private=value_private;
	}

	return (result);
}

#if defined (USE_FUNCTION_VARIABLE_COMPOSITE_EVALUATE)
#if defined (EVALUATE_RETURNS_VALUE)
Function_handle Function_variable_composite::evaluate()
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool first=true,result_is_matrix=true;
	boost::intrusive_ptr< Function_matrix<Scalar> > part_matrix;
	Function_handle part(0),result(0);
	Function_size_type i=variables_list.size(),part_number_of_columns,
		part_number_of_rows,result_number_of_columns,result_number_of_rows;
	std::list< boost::intrusive_ptr< Function_matrix<Scalar> > >
		part_matrix_list(0);
	std::list<Function_handle> part_list(0);
	std::list<Function_variable_handle>::iterator variable_iterator=
		variables_list.begin();

	while ((i>0)&&(part=(*variable_iterator)->evaluate()))
	{
		part_list.push_back(part);
		if (result_is_matrix)
		{
			if (part_matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(part))
			{
				part_matrix_list.push_back(part_matrix);
				if (first)
				{
					result_number_of_rows=part_matrix->number_of_rows();
					result_number_of_columns=part_matrix->number_of_columns();
				}
				else
				{
					part_number_of_rows=part_matrix->number_of_rows();
					part_number_of_columns=part_matrix->number_of_columns();
					if (result_number_of_columns==part_number_of_columns)
					{
						result_number_of_rows += part_number_of_rows;
					}
					else
					{
						if (result_number_of_rows==part_number_of_rows)
						{
							result_number_of_columns += part_number_of_columns;
						}
						else
						{
							result_number_of_rows=
								(result_number_of_rows*result_number_of_columns)+
								(part_number_of_rows*part_number_of_columns);
							result_number_of_columns=1;
						}
					}
				}
			}
			else
			{
				result_is_matrix=false;
			}
		}
		first=false;
		--i;
		++variable_iterator;
	}
	if (0==i)
	{
		if (result_is_matrix)
		{
			Function_size_type part_column,part_row,result_column=0,result_row=0;
			Matrix result_matrix(result_number_of_rows,result_number_of_columns);
			std::list< boost::intrusive_ptr< Function_matrix<Scalar> > >::iterator
				part_matrix_iterator=part_matrix_list.begin();

			for (i=part_matrix_list.size();i>0;--i)
			{
				part_matrix= *part_matrix_iterator;
				part_number_of_rows=part_matrix->number_of_rows();
				part_number_of_columns=part_matrix->number_of_columns();
				for (part_row=1;part_row<=part_number_of_rows;++part_row)
				{
					for (part_column=1;part_column<=part_number_of_columns;++part_column)
					{
						result_matrix(result_row,result_column)=
							(*part_matrix)(part_row,part_column);
						++result_column;
						if (result_column>=result_number_of_columns)
						{
							++result_row;
							result_column=0;
						}
					}
				}
				++part_matrix_iterator;
			}
			result=Function_handle(new Function_matrix<Scalar>(result_matrix));
		}
		else
		{
			result=Function_handle(new Function_composite(part_list));
		}
	}

	return (result);
}
#else // defined (EVALUATE_RETURNS_VALUE)
bool Function_variable_composite::evaluate()
//******************************************************************************
// LAST MODIFIED : 13 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result=true;
	Function_size_type i=variables_list.size();
	std::list<Function_variable_handle>::iterator variable_iterator=
		variables_list.begin();

	while (i>0)
	{
		if (!((*variable_iterator)->evaluate()))
		{
			result=false;
		}
		--i;
		++variable_iterator;
	}

	return (result);
}
#endif // defined (EVALUATE_RETURNS_VALUE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#if defined (OLD_CODE)
Function_handle Function_variable_composite::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool first=true,valid=true;
	Function_derivative_matrix_handle part(0);
	Function_handle result(0);
	Function_size_type i=variables_list.size(),number_of_matrices,
		result_number_of_rows=0;;
	std::list<Function_derivative_matrix_handle> part_list(0);
	std::list<Function_variable_handle>::iterator variable_iterator=
		variables_list.begin();

	while (valid&&(i>0)&&
		(part=Function_derivative_matrix_handle(
		new Function_derivative_matrix(*variable_iterator,independent_variables)))
#if defined (OLD_CODE)
		(part=boost::dynamic_pointer_cast<Function_derivative_matrix,Function>(
		(*variable_iterator)->evaluate_derivative(independent_variables)))
#endif // defined (OLD_CODE)
		)
	{
		if (first)
		{
			number_of_matrices=(part->matrices).size();
		}
		else
		{
			valid=(number_of_matrices==(part->matrices).size());
		}
		part_list.push_back(part);
		result_number_of_rows += (part->matrices).back().size1();
		first=false;
		--i;
		++variable_iterator;
	}
	if (valid&&(0==i))
	{
		Function_size_type number_of_parts;

		number_of_parts=part_list.size();
		if (0<number_of_parts)
		{
			if (1==number_of_parts)
			{
				result=part_list.back();
			}
			else
			{
				Function_size_type j,part_number_of_rows,part_row,result_column,
					result_row=0;;
				std::list<Function_derivative_matrix_handle>::iterator part_iterator=
					part_list.begin();
				std::list<Matrix> result_matrices(0);
				std::list<Matrix>::iterator matrix_iterator;
				std::vector< std::list<Matrix>::iterator >
					part_matrix_iterators(number_of_parts);

				for (i=0;i<number_of_parts;++i)
				{
					part_matrix_iterators[i]=(*part_iterator)->matrices.begin();
					++part_iterator;
				}
				i=number_of_matrices;
				while (valid&&(i>0))
				{
					Function_size_type result_number_of_columns=
						part_matrix_iterators[0]->size2();
					Matrix result_matrix(result_number_of_rows,result_number_of_columns);

					j=0;;
					while (valid&&(j<number_of_parts))
					{
						Matrix& part_matrix= *part_matrix_iterators[j];

						if (result_number_of_columns==part_matrix.size2())
						{
							part_number_of_rows=part_matrix.size1();
							for (part_row=0;part_row<part_number_of_rows;++part_row)
							{
								for (result_column=0;result_column<result_number_of_columns;
									++result_column)
								{
									result_matrix(result_row,result_column)=
										part_matrix(part_row,result_column);
								}
								++result_row;
							}
						}
						else
						{
							valid=false;
						}
						++part_matrix_iterators[j];
						++j;
					}
					result_matrices.push_back(result_matrix);
					--i;
				}
				if (valid)
				{
					result=Function_handle(new Function_derivative_matrix(
						Function_variable_handle(this),independent_variables,
						result_matrices));
				}
			}
		}
	}

	return (result);
}
#endif // defined (OLD_CODE)

Function_handle Function_variable_composite::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool first=true,valid=true;
	boost::intrusive_ptr< Function_matrix<Scalar> > part(0);
	Function_handle result(0);
	Function_size_type i=variables_list.size(),result_number_of_columns=0,
		result_number_of_rows=0;;
	std::list< boost::intrusive_ptr< Function_matrix<Scalar> > > part_list(0);
	std::list<Function_variable_handle>::iterator variable_iterator=
		variables_list.begin();

	while (valid&&(i>0)&&
		(part=boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
		(*variable_iterator)->evaluate_derivative(independent_variables)))
		)
	{
		if (first)
		{
			result_number_of_columns=part->number_of_columns();
		}
		else
		{
			valid=(part->number_of_columns()==result_number_of_columns);
		}
		part_list.push_back(part);
		result_number_of_rows += part->number_of_rows();
		first=false;
		--i;
		++variable_iterator;
	}
	if (valid&&(0==i))
	{
		Function_size_type number_of_parts;

		number_of_parts=part_list.size();
		if (0<number_of_parts)
		{
			if (1==number_of_parts)
			{
				result=part_list.back();
			}
			else
			{
				Function_size_type j,part_number_of_rows,part_column,part_row,
					result_column,result_row=0;;
				Matrix result_matrix(result_number_of_rows,result_number_of_columns);
				std::list< boost::intrusive_ptr< Function_matrix<Scalar> > >::iterator
					part_iterator=part_list.begin();

				j=number_of_parts;;
				while (valid&&(j>0))
				{
					Function_matrix<Scalar>& part_matrix= **part_iterator;

					part_number_of_rows=part_matrix.number_of_rows();
					for (part_row=1;part_row<=part_number_of_rows;++part_row)
					{
						part_column=1;
						for (result_column=0;result_column<result_number_of_columns;
							++result_column)
						{
							result_matrix(result_row,result_column)=
								part_matrix(part_row,part_column);
							++part_column;
						}
						++result_row;
					}
					--j;
					++part_iterator;
				}
				result=Function_handle(new Function_matrix<Scalar>(result_matrix));
			}
		}
	}

	return (result);
}
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
Function_handle Function_variable_composite::derivative(
	const std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 14 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_handle(new Function_derivatnew_composite(
		Function_variable_handle(this),independent_variables)));
}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE_COMPOSITE_EVALUATE)

string_handle Function_variable_composite::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
// ???DB.  Overload << instead of get_string_representation?
// ???DB.  Flatten
//==============================================================================
{
	std::list<Function_variable_handle>::iterator
		variable_iterator=variables_list.begin();
	std::ostringstream out;
	string_handle return_string,temp_string;
	Function_size_type i=variables_list.size();

	if (return_string=new std::string)
	{
		out << "composite(";
		while (i>0)
		{
			if ((*variable_iterator)&&
				(temp_string=(*variable_iterator)->get_string_representation()))
			{
				out << *temp_string;
				delete temp_string;
			}
			++variable_iterator;
			--i;
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

Function_variable_iterator Function_variable_composite::begin_atomic() const
//******************************************************************************
// LAST MODIFIED : 15 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_iterator(
		new Function_variable_iterator_representation_atomic_composite(true,
		Function_variable_composite_handle(
		const_cast<Function_variable_composite*>(this)))));
}

Function_variable_iterator Function_variable_composite::end_atomic() const
//******************************************************************************
// LAST MODIFIED : 15 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_iterator(
		new Function_variable_iterator_representation_atomic_composite(false,
		Function_variable_composite_handle(
		const_cast<Function_variable_composite*>(this)))));
}

std::reverse_iterator<Function_variable_iterator>
	Function_variable_composite::rbegin_atomic() const
//******************************************************************************
// LAST MODIFIED : 15 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (std::reverse_iterator<Function_variable_iterator>(
		new Function_variable_iterator_representation_atomic_composite(false,
		Function_variable_composite_handle(
		const_cast<Function_variable_composite*>(this)))));
}

std::reverse_iterator<Function_variable_iterator>
	Function_variable_composite::rend_atomic() const
//******************************************************************************
// LAST MODIFIED : 15 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (std::reverse_iterator<Function_variable_iterator>(
		new Function_variable_iterator_representation_atomic_composite(true,
		Function_variable_composite_handle(
		const_cast<Function_variable_composite*>(this)))));
}

class Function_variable_composite_sum_number_differentiable_functor
//******************************************************************************
// LAST MODIFIED : 11 February 2004
//
// DESCRIPTION :
// A unary function (functor) for summing the number_differentiable of the
// variables that make up a composite variable.
//==============================================================================
{
	public:
		Function_variable_composite_sum_number_differentiable_functor(
			Function_size_type& sum):sum(sum)
		{
			sum=0;
		};
		int operator() (const Function_variable_handle& variable)
		{
			sum += variable->number_differentiable();
			return (0);
		};
	private:
		Function_size_type& sum;
};

Function_size_type Function_variable_composite::number_differentiable()
//******************************************************************************
// LAST MODIFIED : 11 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_size_type sum;

	// get the specified values
	std::for_each(variables_list.begin(),variables_list.end(),
		Function_variable_composite_sum_number_differentiable_functor(sum));

	return (sum);
}

void Function_variable_composite::add_dependent_function(
#if defined (CIRCULAR_SMART_POINTERS)
	const Function_handle
#else // defined (CIRCULAR_SMART_POINTERS)
	Function*
#endif // defined (CIRCULAR_SMART_POINTERS)
	dependent_function)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Function_variable_handle>::iterator iterator,iterator_end;

	iterator_end=variables_list.end();
	for (iterator=variables_list.begin();iterator!=iterator_end;++iterator)
	{
		(*iterator)->add_dependent_function(dependent_function);
	}
}

void Function_variable_composite::remove_dependent_function(
#if defined (CIRCULAR_SMART_POINTERS)
	const Function_handle
#else // defined (CIRCULAR_SMART_POINTERS)
	Function*
#endif // defined (CIRCULAR_SMART_POINTERS)
	dependent_function)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Function_variable_handle>::iterator iterator,iterator_end;

	iterator_end=variables_list.end();
	for (iterator=variables_list.begin();iterator!=iterator_end;++iterator)
	{
		(*iterator)->remove_dependent_function(dependent_function);
	}
}

bool Function_variable_composite::equality_atomic(
	const Function_variable_handle& variable) const
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_composite_handle variable_composite;

	result=false;
	if (variable_composite=boost::dynamic_pointer_cast<
		Function_variable_composite,Function_variable>(variable))
	{
		std::list<Function_variable_handle>::const_iterator
			variable_iterator_1,variable_iterator_1_end,variable_iterator_2,
			variable_iterator_2_end;

		variable_iterator_1=variables_list.begin();
		variable_iterator_1_end=variables_list.end();
		variable_iterator_2=
			(variable_composite->variables_list).begin();
		variable_iterator_2_end=
			(variable_composite->variables_list).end();
		while ((variable_iterator_1!=variable_iterator_1_end)&&
			(variable_iterator_2!=variable_iterator_2_end)&&
			equivalent(*variable_iterator_1,*variable_iterator_2))
		{
			++variable_iterator_1;
			++variable_iterator_2;
		}
		result=((variable_iterator_1==variable_iterator_1_end)&&
			(variable_iterator_2==variable_iterator_2_end));
	}

	return (result);
}

Function_variable_composite::Function_variable_composite(
	const Function_variable_composite& variable_composite):
	Function_variable(variable_composite),
	variables_list(variable_composite.variables_list)
//******************************************************************************
// LAST MODIFIED : 25 February 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Function_variable_composite& Function_variable_composite::operator=(
	const Function_variable_composite& variable_composite)
//******************************************************************************
// LAST MODIFIED : 11 February 2004
//
// DESCRIPTION :
// Assignment operator.
//???DB.  Same as implicit?
//==============================================================================
{
	//???DB.  Does assignment for super class first?
	variables_list=variable_composite.variables_list;

	return (*this);
}
