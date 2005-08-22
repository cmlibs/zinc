//******************************************************************************
// FILE : function_variable_intersection.cpp
//
// LAST MODIFIED : 10 May 2005
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

#include "computed_variable/function_variable_intersection.hpp"

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#include "computed_variable/function_derivative.hpp"
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

// module classes
// ==============

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_derivatnew_intersection
// --------------------------------------

class Function_derivatnew_intersection : public Function_derivatnew
//******************************************************************************
// LAST MODIFIED : 9 May 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// for construction exception
		class Construction_exception {};
		// constructor
		Function_derivatnew_intersection(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables);
		// destructor
		~Function_derivatnew_intersection();
	// inherited
	private:
#if defined (EVALUATE_RETURNS_VALUE)
		virtual Function_handle evaluate(Function_variable_handle atomic_variable);
#else // defined (EVALUATE_RETURNS_VALUE)
		virtual bool evaluate(Function_variable_handle atomic_variable);
#endif // defined (EVALUATE_RETURNS_VALUE)
	private:
		Function_derivatnew_handle first_variable_derivative;
};

Function_derivatnew_intersection::Function_derivatnew_intersection(
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
	Function_variable_intersection_handle variable_intersection;

	if (variable_intersection=boost::dynamic_pointer_cast<
		Function_variable_intersection,Function_variable>(dependent_variable))
	{
		Function_variable_handle first_variable;

		if ((first_variable=(variable_intersection->variables_list).front())&&
			(first_variable_derivative=boost::dynamic_pointer_cast<
			Function_derivatnew,Function>(first_variable->derivative(
			independent_variables))))
		{
			first_variable_derivative->add_dependent_function(this);
		}
		else
		{
			throw Function_derivatnew_intersection::Construction_exception();
		}
	}
	else
	{
		throw Function_derivatnew_intersection::Construction_exception();
	}
}

Function_derivatnew_intersection::~Function_derivatnew_intersection()
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
	if (first_variable_derivative)
	{
		first_variable_derivative->remove_dependent_function(this);
	}
#endif // defined (CIRCULAR_SMART_POINTERS)
}

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_derivatnew_intersection::evaluate(
	Function_variable_handle
#if defined (EVALUATE_RETURNS_VALUE)
	atomic_variable
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
	)
//******************************************************************************
// LAST MODIFIED : 10 May 2005
//
// DESCRIPTION :
//==============================================================================
{
#if defined (EVALUATE_RETURNS_VALUE)
	Function_handle result(0);
#else // defined (EVALUATE_RETURNS_VALUE)
	bool result(true);
#endif // defined (EVALUATE_RETURNS_VALUE)

	if (!evaluated())
	{
		Function_variable_intersection_handle dependent_variable_intersection;

#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
		result=false;
#endif // defined (EVALUATE_RETURNS_VALUE)
		if (dependent_variable_intersection=boost::dynamic_pointer_cast<
			Function_variable_intersection,Function_variable>(dependent_variable))
		{
			Function_variable_handle first_derivative_output;

			derivative_matrix.clear();
			if ((first_derivative_output=first_variable_derivative->output())&&
				(first_derivative_output->evaluate()))
			{
				Derivative_matrix&
					first_variable_derivative_matrix=first_variable_derivative->
					derivative_matrix;
				Function_size_type
					result_number_of_matrices=first_variable_derivative_matrix.size(),
					result_number_of_rows,row,first_variable_number_of_rows=
					(first_variable_derivative_matrix.front()).size1();
				Function_variable_handle intersection_variable;
				Function_variable_iterator intersection_atomic_variable_iterator,
					intersection_atomic_variable_iterator_end,
					first_variable_atomic_variable_iterator,
					first_variable_atomic_variable_iterator_end;
				std::list<Function_variable_handle>::iterator
					intersection_variable_iterator,intersection_variable_iterator_begin,
					intersection_variable_iterator_end;
				std::vector<Function_size_type>
					first_variable_row_mapping(first_variable_number_of_rows);

				// count number of rows and set up mapping
				result_number_of_rows=0;
				row=0;
				intersection_variable_iterator_begin=
					(dependent_variable_intersection->variables_list).begin();
				intersection_variable_iterator_end=
					(dependent_variable_intersection->variables_list).end();
				first_variable_atomic_variable_iterator=
					((dependent_variable_intersection->variables_list).front())->
					begin_atomic();
				first_variable_atomic_variable_iterator_end=
					((dependent_variable_intersection->variables_list).front())->
					end_atomic();
				while (first_variable_atomic_variable_iterator!=
					first_variable_atomic_variable_iterator_end)
				{
					intersection_variable_iterator=intersection_variable_iterator_begin;
					do
					{
						intersection_variable_iterator++;
						if (intersection_variable= *intersection_variable_iterator)
						{
							intersection_atomic_variable_iterator=
								intersection_variable->begin_atomic();
							intersection_atomic_variable_iterator_end=
								intersection_variable->end_atomic();
							while ((intersection_atomic_variable_iterator!=
								intersection_atomic_variable_iterator_end)&&
								!equivalent(*intersection_atomic_variable_iterator,
								*first_variable_atomic_variable_iterator))
							{
								++intersection_atomic_variable_iterator;
							}
						}
					} while ((intersection_variable_iterator!=
						intersection_variable_iterator_end)&&
						(intersection_atomic_variable_iterator!=
						intersection_atomic_variable_iterator_end));
					if (intersection_variable_iterator==
						intersection_variable_iterator_end)
					{
						first_variable_row_mapping[row]=result_number_of_rows;
						++result_number_of_rows;
					}
					else
					{
						first_variable_row_mapping[row]=first_variable_number_of_rows;
					}
					++first_variable_atomic_variable_iterator;
					++row;
				}
				if (0<result_number_of_rows)
				{
					Function_size_type i,j,k;
					std::list<Matrix> matrices;
					std::list<Matrix>::iterator first_variable_matrix_iterator;

					k=result_number_of_matrices;
					first_variable_matrix_iterator=
						first_variable_derivative_matrix.begin();
					while (k>0)
					{
						Matrix& first_variable_matrix= *first_variable_matrix_iterator;
						Function_size_type number_of_columns=first_variable_matrix.size2();
						Matrix result_matrix(result_number_of_rows,number_of_columns);

						for (i=0;i<first_variable_number_of_rows;++i)
						{
							if ((row=first_variable_row_mapping[i])<
								first_variable_number_of_rows)
							{
								for (k=0;k<number_of_columns;++k)
								{
									result_matrix(row,j)=first_variable_matrix(i,j);
								}
							}
						}
						matrices.push_back(result_matrix);
						++first_variable_matrix_iterator;
						--k;
					}
					derivative_matrix=Derivative_matrix(matrices);
					set_evaluated();
#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
					result=true;
#endif // defined (EVALUATE_RETURNS_VALUE)
				}
			}
		}
	}
#if defined (EVALUATE_RETURNS_VALUE)
	if (evaluated())
	{
		result=get_value(atomic_variable);
	}
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)

	return (result);
}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)


// class Function_variable_iterator_representation_atomic_intersection
// -------------------------------------------------------------------

bool intersect_atomic_variable(
	const std::list<Function_variable_handle>& variables_list,
	const Function_variable_iterator& atomic_variable_iterator)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
// Determines if (*atomic_variable_iterator) is in all the variables.
//==============================================================================
{
	bool intersect;
	Function_variable_iterator local_atomic_variable_iterator;
	std::list<Function_variable_handle>::const_iterator
		local_variables_list_iterator;

	intersect=false;
	local_variables_list_iterator=variables_list.begin();
	if (local_variables_list_iterator!=variables_list.end())
	{
		bool repeat;

		// ignore duplicates in first variable
		repeat=false;
		local_atomic_variable_iterator=
			(*local_variables_list_iterator)->begin_atomic();
		while ((local_atomic_variable_iterator!=atomic_variable_iterator)&&
			!(repeat=equivalent(*local_atomic_variable_iterator,
			*atomic_variable_iterator)))
		{
			++local_atomic_variable_iterator;
		}
		intersect= !repeat;
		++local_variables_list_iterator;
		while (intersect&&(local_variables_list_iterator!=variables_list.end()))
		{
			intersect=false;
			local_atomic_variable_iterator=
				(*local_variables_list_iterator)->begin_atomic();
			while ((local_atomic_variable_iterator!=
				(*local_variables_list_iterator)->end_atomic())&&
				!(intersect=equivalent(*local_atomic_variable_iterator,
				*atomic_variable_iterator)))
			{
				++local_atomic_variable_iterator;
			}
			++local_variables_list_iterator;
		}
	}

	return (intersect);
}

class Function_variable_iterator_representation_atomic_intersection: public
	Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_intersection(
			const bool begin,Function_variable_intersection_handle source):
			end(true),atomic_variable_iterator(0),source(source)
		{
			if (source)
			{
				if (begin)
				{
					std::list<Function_variable_handle>::iterator
						local_variables_list_iterator;

					local_variables_list_iterator=(source->variables_list).begin();
					if (local_variables_list_iterator!=(source->variables_list).end())
					{
						atomic_variable_iterator=
							((*local_variables_list_iterator)->begin_atomic)();
						while ((atomic_variable_iterator!=
							(*local_variables_list_iterator)->end_atomic())&&
							!intersect_atomic_variable(source->variables_list,
							atomic_variable_iterator))
						{
							++atomic_variable_iterator;
						}
						if (atomic_variable_iterator==
							(*local_variables_list_iterator)->end_atomic())
						{
							atomic_variable_iterator=0;
						}
						else
						{
							end=false;
						}
					}
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
					Function_variable_iterator_representation_atomic_intersection(*this);
			}

			return (result);
		};
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_intersection(){};
	private:
		// increment
		void increment()
		{
			if (!end)
			{
				Function_variable_iterator end_atomic_variable_iterator;

				end_atomic_variable_iterator=(*((source->variables_list).begin()))->
					end_atomic();
				do
				{
					++atomic_variable_iterator;
				} while ((atomic_variable_iterator!=end_atomic_variable_iterator)&&
					!intersect_atomic_variable(source->variables_list,
					atomic_variable_iterator));
				if (atomic_variable_iterator==end_atomic_variable_iterator)
				{
					atomic_variable_iterator=0;
					end=true;
				}
			}
		};
		// decrement
		void decrement()
		{
			if (end&&(0<(source->variables_list).size()))
			{
				atomic_variable_iterator=
					(*((source->variables_list).begin()))->end_atomic();
				if (atomic_variable_iterator==
					(*((source->variables_list).begin()))->begin_atomic())
				{
					atomic_variable_iterator=0;
				}
				else
				{
					end=false;
				}
			}
			if (!end)
			{
				bool intersect;
				Function_variable_iterator begin_atomic_variable_iterator;

				intersect=false;
				begin_atomic_variable_iterator=(*((source->variables_list).begin()))->
					begin_atomic();
				while ((atomic_variable_iterator!=begin_atomic_variable_iterator)&&
					!intersect)
				{
					--atomic_variable_iterator;
					intersect=intersect_atomic_variable(source->variables_list,
						atomic_variable_iterator);
				}
				if (!intersect)
				{
					atomic_variable_iterator=0;
					end=true;
				}
			}
		};
		// equality
		bool equality(const Function_variable_iterator_representation
			* representation)
		{
			const Function_variable_iterator_representation_atomic_intersection
				*representation_intersection=dynamic_cast<
				const Function_variable_iterator_representation_atomic_intersection *>(
				representation);
			
			return (representation_intersection&&
				(end==representation_intersection->end)&&
				(atomic_variable_iterator==
				representation_intersection->atomic_variable_iterator));
		};
		// dereference
		Function_variable_handle dereference() const
		{
			return (*atomic_variable_iterator);
		};
	private:
		Function_variable_iterator_representation_atomic_intersection(const
			Function_variable_iterator_representation_atomic_intersection&
			representation):Function_variable_iterator_representation(),
			end(representation.end),
			atomic_variable_iterator(representation.atomic_variable_iterator),
			source(representation.source){};
	private:
		bool end;
		Function_variable_iterator atomic_variable_iterator;
		Function_variable_intersection_handle source;
};

// global classes
// ==============

// class Function_variable_intersection
// ------------------------------------

Function_variable_intersection::Function_variable_intersection(
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

Function_variable_intersection::Function_variable_intersection(
	const Function_handle& function,
	const Function_variable_handle& variable_1,
	const Function_variable_handle& variable_2):Function_variable(function),
	variables_list(0)
//******************************************************************************
// LAST MODIFIED : 18 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	variables_list.push_back(variable_1);
	variables_list.push_back(variable_2);
}

Function_variable_intersection::Function_variable_intersection(
	std::list<Function_variable_handle>& variables_list):
	Function_variable(Function_handle(0)),variables_list(variables_list)
//******************************************************************************
// LAST MODIFIED : 30 June 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Function_variable_intersection::Function_variable_intersection(
	const Function_handle& function,
	std::list<Function_variable_handle>& variables_list):
	Function_variable(function),variables_list(variables_list)
//******************************************************************************
// LAST MODIFIED : 18 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Function_variable_handle Function_variable_intersection::clone() const
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle local_function=function();
	Function_size_type i;
	Function_variable_intersection_handle result(0);
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
	if (result=Function_variable_intersection_handle(
		new Function_variable_intersection(local_function,local_variables_list)))
	{
		result->value_private=value_private;
	}

	return (result);
}

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
Function_handle Function_variable_intersection::derivative(
	const std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 10 May 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_handle(new Function_derivatnew_intersection(
		Function_variable_handle(this),independent_variables)));
}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

string_handle Function_variable_intersection::get_string_representation()
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
		out << "intersection(";
		while (i>0)
		{
			if (temp_string=(*variable_iterator)->get_string_representation())
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

Function_variable_iterator Function_variable_intersection::begin_atomic() const
//******************************************************************************
// LAST MODIFIED : 18 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_iterator(
		new Function_variable_iterator_representation_atomic_intersection(true,
		Function_variable_intersection_handle(
		const_cast<Function_variable_intersection*>(this)))));
}

Function_variable_iterator Function_variable_intersection::end_atomic() const
//******************************************************************************
// LAST MODIFIED : 18 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_iterator(
		new Function_variable_iterator_representation_atomic_intersection(false,
		Function_variable_intersection_handle(
		const_cast<Function_variable_intersection*>(this)))));
}

std::reverse_iterator<Function_variable_iterator>
	Function_variable_intersection::rbegin_atomic() const
//******************************************************************************
// LAST MODIFIED : 18 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (std::reverse_iterator<Function_variable_iterator>(
		new Function_variable_iterator_representation_atomic_intersection(false,
		Function_variable_intersection_handle(
		const_cast<Function_variable_intersection*>(this)))));
}

std::reverse_iterator<Function_variable_iterator>
	Function_variable_intersection::rend_atomic() const
//******************************************************************************
// LAST MODIFIED : 18 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (std::reverse_iterator<Function_variable_iterator>(
		new Function_variable_iterator_representation_atomic_intersection(true,
		Function_variable_intersection_handle(
		const_cast<Function_variable_intersection*>(this)))));
}

void Function_variable_intersection::add_dependent_function(
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

void Function_variable_intersection::remove_dependent_function(
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

bool Function_variable_intersection::equality_atomic(const
	Function_variable_handle&) const
//******************************************************************************
// LAST MODIFIED : 18 March 2004
//
// DESCRIPTION :
//???DB.  This should be == for everything?  How to get atomic?
//==============================================================================
{
	// should not come here - handled by overloading Function_variable::operator==
	Assert(false,std::logic_error(
		"Function_variable_intersection::equality_atomic.  "
		"Should not come here"));

	return (false);
}

Function_variable_intersection::Function_variable_intersection(
	const Function_variable_intersection& variable_intersection):
	Function_variable(variable_intersection),
	variables_list(variable_intersection.variables_list)
//******************************************************************************
// LAST MODIFIED : 18 March 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Function_variable_intersection& Function_variable_intersection::operator=(
	const Function_variable_intersection& variable_intersection)
//******************************************************************************
// LAST MODIFIED : 18 March 2004
//
// DESCRIPTION :
// Assignment operator.
//???DB.  Same as implicit?
//==============================================================================
{
	//???DB.  Does assignment for super class first?
	variables_list=variable_intersection.variables_list;

	return (*this);
}
