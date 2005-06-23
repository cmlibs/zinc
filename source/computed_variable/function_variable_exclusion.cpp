//******************************************************************************
// FILE : function_variable_exclusion.cpp
//
// LAST MODIFIED : 9 May 2005
//
// DESCRIPTION :
//==============================================================================

#include <algorithm>
#include <iterator>
#include <typeinfo>
#include <iostream>
#include <sstream>

#include "computed_variable/function_variable_exclusion.hpp"

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#include "computed_variable/function_derivative.hpp"
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

// module classes
// ==============

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_derivatnew_exclusion
// -----------------------------------

class Function_derivatnew_exclusion : public Function_derivatnew
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
		Function_derivatnew_exclusion(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables);
		// destructor
		~Function_derivatnew_exclusion();
	// inherited
	private:
#if defined (EVALUATE_RETURNS_VALUE)
		virtual Function_handle evaluate(Function_variable_handle atomic_variable);
#else // defined (EVALUATE_RETURNS_VALUE)
		virtual bool evaluate(Function_variable_handle atomic_variable);
#endif // defined (EVALUATE_RETURNS_VALUE)
	private:
		Function_derivatnew_handle universe_derivative;
};

Function_derivatnew_exclusion::Function_derivatnew_exclusion(
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
	Function_variable_exclusion_handle variable_exclusion;

	if (variable_exclusion=boost::dynamic_pointer_cast<
		Function_variable_exclusion,Function_variable>(dependent_variable))
	{
		if ((universe_derivative=boost::dynamic_pointer_cast<
			Function_derivatnew,Function>(variable_exclusion->universe->derivative(
			independent_variables))))
		{
			universe_derivative->add_dependent_function(this);
		}
		else
		{
			throw Function_derivatnew_exclusion::Construction_exception();
		}
	}
	else
	{
		throw Function_derivatnew_exclusion::Construction_exception();
	}
}

Function_derivatnew_exclusion::~Function_derivatnew_exclusion()
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
	if (universe_derivative)
	{
		universe_derivative->remove_dependent_function(this);
	}
#endif // defined (CIRCULAR_SMART_POINTERS)
}

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_derivatnew_exclusion::evaluate(
	Function_variable_handle
#if defined (EVALUATE_RETURNS_VALUE)
	atomic_variable
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
	)
//******************************************************************************
// LAST MODIFIED : 9 May 2005
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
		Function_variable_exclusion_handle dependent_variable_exclusion;

#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
		result=false;
#endif // defined (EVALUATE_RETURNS_VALUE)
		if (dependent_variable_exclusion=boost::dynamic_pointer_cast<
			Function_variable_exclusion,Function_variable>(dependent_variable))
		{
			Function_variable_handle universe_output;

			derivative_matrix.clear();
			if ((universe_output=universe_derivative->output())&&
				(universe_output->evaluate()))
			{
				Derivative_matrix&
					universe_derivative_matrix=universe_derivative->derivative_matrix;
				Function_size_type
					result_number_of_matrices=universe_derivative_matrix.size(),
					result_number_of_rows,row,
					universe_number_of_rows=(universe_derivative_matrix.front()).size1();
				Function_variable_iterator exclusion_atomic_variable_iterator,
					exclusion_atomic_variable_iterator_begin,
					exclusion_atomic_variable_iterator_end,
					universe_atomic_variable_iterator,
					universe_atomic_variable_iterator_end;
				std::vector<Function_size_type>
					universe_row_mapping(universe_number_of_rows);

				// count number of rows and set up mapping
				result_number_of_rows=0;
				row=0;
				exclusion_atomic_variable_iterator_begin=
					dependent_variable_exclusion->exclusion->begin_atomic();
				exclusion_atomic_variable_iterator_end=
					dependent_variable_exclusion->exclusion->end_atomic();
				universe_atomic_variable_iterator=
					dependent_variable_exclusion->universe->begin_atomic();
				universe_atomic_variable_iterator_end=
					dependent_variable_exclusion->universe->end_atomic();
				while (universe_atomic_variable_iterator!=
					universe_atomic_variable_iterator_end)
				{
					exclusion_atomic_variable_iterator=
						exclusion_atomic_variable_iterator_begin;
					while ((exclusion_atomic_variable_iterator!=
						exclusion_atomic_variable_iterator_end)&&
						!equivalent(*exclusion_atomic_variable_iterator,
						*universe_atomic_variable_iterator))
					{
						++exclusion_atomic_variable_iterator;
					}
					if (exclusion_atomic_variable_iterator==
						exclusion_atomic_variable_iterator_end)
					{
						universe_row_mapping[row]=result_number_of_rows;
						++result_number_of_rows;
					}
					else
					{
						universe_row_mapping[row]=universe_number_of_rows;
					}
					++universe_atomic_variable_iterator;
					++row;
				}
				if (0<result_number_of_rows)
				{
					Function_size_type i,j,k;
					std::list<Matrix> matrices;
					std::list<Matrix>::iterator universe_matrix_iterator;

					k=result_number_of_matrices;
					universe_matrix_iterator=universe_derivative_matrix.begin();
					while (k>0)
					{
						Matrix& universe_matrix= *universe_matrix_iterator;
						Function_size_type number_of_columns=universe_matrix.size2();
						Matrix result_matrix(result_number_of_rows,number_of_columns);

						for (i=0;i<universe_number_of_rows;++i)
						{
							if ((row=universe_row_mapping[i])<universe_number_of_rows)
							{
								for (j=0;j<number_of_columns;++j)
								{
									result_matrix(row,j)=universe_matrix(i,j);
								}
							}
						}
						matrices.push_back(result_matrix);
						++universe_matrix_iterator;
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


// class Function_variable_iterator_representation_atomic_exclusion
// ----------------------------------------------------------------

bool include_atomic_variable(Function_variable_handle universe,
	Function_variable_handle exclusion,
	const Function_variable_iterator& atomic_variable_iterator)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
// Determines if (*atomic_variable_iterator) appears in the universe-exclusion.
//==============================================================================
{
	bool include;
	Function_variable_iterator local_atomic_variable_iterator;

	include=true;
	// ignore duplicates in universe
	local_atomic_variable_iterator=universe->begin_atomic();
	while ((local_atomic_variable_iterator!=atomic_variable_iterator)&&
		(include= !equivalent(*local_atomic_variable_iterator,
		*atomic_variable_iterator)))
	{
		++local_atomic_variable_iterator;
	}
	if (include)
	{
		local_atomic_variable_iterator=exclusion->begin_atomic();
		while ((local_atomic_variable_iterator!=exclusion->end_atomic())&&(include=
			!equivalent(*local_atomic_variable_iterator,*atomic_variable_iterator)))
		{
			++local_atomic_variable_iterator;
		}
	}

	return (include);
}

class Function_variable_iterator_representation_atomic_exclusion: public
	Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_exclusion(
			const bool begin,Function_variable_exclusion_handle source):
			end(true),atomic_variable_iterator(0),source(source)
		{
			if (source)
			{
				if (begin)
				{
					atomic_variable_iterator=source->universe->begin_atomic();
					while ((atomic_variable_iterator!=source->universe->end_atomic())&&
						!include_atomic_variable(source->universe,source->exclusion,
						atomic_variable_iterator))
					{
						++atomic_variable_iterator;
					}
					if (atomic_variable_iterator==source->universe->end_atomic())
					{
						atomic_variable_iterator=0;
					}
					else
					{
						end=false;
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
				result=new Function_variable_iterator_representation_atomic_exclusion(
					*this);
			}

			return (result);
		};
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_exclusion(){};
	private:
		// increment
		void increment()
		{
			if (!end)
			{
				do
				{
					++atomic_variable_iterator;
				} while ((atomic_variable_iterator!=source->universe->end_atomic())&&
					!include_atomic_variable(source->universe,source->exclusion,
					atomic_variable_iterator));
				if (atomic_variable_iterator==source->universe->end_atomic())
				{
					atomic_variable_iterator=0;
					end=true;
				}
			}
		};
		// decrement
		void decrement()
		{
			if (end)
			{
				atomic_variable_iterator=source->universe->end_atomic();
			}
			if (atomic_variable_iterator==source->universe->begin_atomic())
			{
				atomic_variable_iterator=0;
				end=true;
			}
			else
			{
				bool include;

				do
				{
					--atomic_variable_iterator;
					include=include_atomic_variable(source->universe,source->exclusion,
						atomic_variable_iterator);
				} while ((atomic_variable_iterator!=source->universe->begin_atomic())&&
					!include);
				if (!include)
				{
					end=true;
				}
			}
		};
		// equality
		bool equality(const Function_variable_iterator_representation
			* representation)
		{
			const Function_variable_iterator_representation_atomic_exclusion
				*representation_exclusion=dynamic_cast<
				const Function_variable_iterator_representation_atomic_exclusion *>(
				representation);

			return (representation_exclusion&&(end==representation_exclusion->end)&&
				(atomic_variable_iterator==
				representation_exclusion->atomic_variable_iterator));
		};
		// dereference
		Function_variable_handle dereference() const
		{
			return (*atomic_variable_iterator);
		};
	private:
		Function_variable_iterator_representation_atomic_exclusion(const
			Function_variable_iterator_representation_atomic_exclusion&
			representation):Function_variable_iterator_representation(),
			end(representation.end),
			atomic_variable_iterator(representation.atomic_variable_iterator),
			source(representation.source){};
	private:
		bool end;
		Function_variable_iterator atomic_variable_iterator;
		Function_variable_exclusion_handle source;
};

// global classes
// ==============

// class Function_variable_exclusion
// ---------------------------------

Function_variable_exclusion::Function_variable_exclusion(
	const Function_variable_handle& universe,
	const Function_variable_handle& exclusion):
	Function_variable(Function_handle(0)),exclusion(exclusion),universe(universe)
//******************************************************************************
// LAST MODIFIED : 30 June 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Function_variable_exclusion::Function_variable_exclusion(
	const Function_handle& function,const Function_variable_handle& universe,
	const Function_variable_handle& exclusion):Function_variable(function),
	exclusion(exclusion),universe(universe)
//******************************************************************************
// LAST MODIFIED : 19 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Function_variable_handle Function_variable_exclusion::clone() const
//******************************************************************************
// LAST MODIFIED : 23 November 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle local_function=function();
	Function_variable_exclusion_handle result(0);
	Function_variable_handle local_exclusion(0),local_universe(0);

	if (exclusion)
	{
		local_exclusion=exclusion->clone();
	}
	if (universe)
	{
		local_universe=universe->clone();
	}
	if (result=Function_variable_exclusion_handle(new Function_variable_exclusion(
		local_function,local_universe,local_exclusion)))
	{
		result->value_private=value_private;
	}

	return (result);
}

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
Function_handle Function_variable_exclusion::derivative(
	const std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 6 May 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_handle(new Function_derivatnew_exclusion(
		Function_variable_handle(this),independent_variables)));
}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

string_handle Function_variable_exclusion::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 19 March 2004
//
// DESCRIPTION :
// ???DB.  Overload << instead of get_string_representation?
//==============================================================================
{
	std::ostringstream out;
	string_handle return_string,temp_string;

	if (return_string=new std::string)
	{
		out << "exclusion(";
		if (universe&&(temp_string=universe->get_string_representation()))
		{
			out << *temp_string;
			delete temp_string;
		}
		out << ",";
		if (exclusion&&(temp_string=exclusion->get_string_representation()))
		{
			out << *temp_string;
			delete temp_string;
		}
		out << ")";
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_iterator Function_variable_exclusion::begin_atomic() const
//******************************************************************************
// LAST MODIFIED : 19 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_iterator(
		new Function_variable_iterator_representation_atomic_exclusion(true,
		Function_variable_exclusion_handle(
		const_cast<Function_variable_exclusion*>(this)))));
}

Function_variable_iterator Function_variable_exclusion::end_atomic() const
//******************************************************************************
// LAST MODIFIED : 19 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_iterator(
		new Function_variable_iterator_representation_atomic_exclusion(false,
		Function_variable_exclusion_handle(
		const_cast<Function_variable_exclusion*>(this)))));
}

std::reverse_iterator<Function_variable_iterator>
	Function_variable_exclusion::rbegin_atomic() const
//******************************************************************************
// LAST MODIFIED : 19 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (std::reverse_iterator<Function_variable_iterator>(
		new Function_variable_iterator_representation_atomic_exclusion(false,
		Function_variable_exclusion_handle(
		const_cast<Function_variable_exclusion*>(this)))));
}

std::reverse_iterator<Function_variable_iterator>
	Function_variable_exclusion::rend_atomic() const
//******************************************************************************
// LAST MODIFIED : 19 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (std::reverse_iterator<Function_variable_iterator>(
		new Function_variable_iterator_representation_atomic_exclusion(true,
		Function_variable_exclusion_handle(
		const_cast<Function_variable_exclusion*>(this)))));
}

void Function_variable_exclusion::add_dependent_function(
#if defined (CIRCULAR_SMART_POINTERS)
	const Function_handle
#else // defined (CIRCULAR_SMART_POINTERS)
	Function*
#endif // defined (CIRCULAR_SMART_POINTERS)
	dependent_function)
//******************************************************************************
// LAST MODIFIED : y December 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (universe)
	{
		universe->add_dependent_function(dependent_function);
	}
}

void Function_variable_exclusion::remove_dependent_function(
#if defined (CIRCULAR_SMART_POINTERS)
	const Function_handle
#else // defined (CIRCULAR_SMART_POINTERS)
	Function*
#endif // defined (CIRCULAR_SMART_POINTERS)
	dependent_function)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (universe)
	{
		universe->remove_dependent_function(dependent_function);
	}
}

bool Function_variable_exclusion::equality_atomic(const
	Function_variable_handle&) const
//******************************************************************************
// LAST MODIFIED : 19 March 2004
//
// DESCRIPTION :
//???DB.  This should be == for everything?  How to get atomic?
//==============================================================================
{
	// should not come here - handled by overloading Function_variable::operator==
	Assert(false,std::logic_error(
		"Function_variable_exclusion::equality_atomic.  "
		"Should not come here"));

	return (false);
}

Function_variable_exclusion::Function_variable_exclusion(
	const Function_variable_exclusion& variable_exclusion):
	Function_variable(variable_exclusion),exclusion(variable_exclusion.exclusion),
	universe(variable_exclusion.universe)
//******************************************************************************
// LAST MODIFIED : 19 March 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{}

Function_variable_exclusion& Function_variable_exclusion::operator=(
	const Function_variable_exclusion& variable_exclusion)
//******************************************************************************
// LAST MODIFIED : 19 March 2004
//
// DESCRIPTION :
// Assignment operator.
//???DB.  Same as implicit?
//==============================================================================
{
	//???DB.  Does assignment for super class first?
	universe=variable_exclusion.universe;
	exclusion=variable_exclusion.exclusion;

	return (*this);
}
