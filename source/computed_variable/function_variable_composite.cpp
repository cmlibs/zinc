//******************************************************************************
// FILE : function_variable_composite.cpp
//
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
//==============================================================================

#include <algorithm>
#include <iterator>
#include <typeinfo>
#include <iostream>
#include <sstream>

#include "computed_variable/function_variable_composite.hpp"

// module classes
// ==============

// class Function_variable_iterator_representation_atomic_composite
// ----------------------------------------------------------------

class Function_variable_iterator_representation_atomic_composite: public
	Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 17 March 2004
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
						variables_list_iterator++;
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
				atomic_variable_iterator++;
				if (atomic_variable_iterator==
					((*variables_list_iterator)->end_atomic)())
				{
					do
					{
						variables_list_iterator++;
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
						variables_list_iterator--;
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
						atomic_variable_iterator--;
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
				atomic_variable_iterator--;
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
// LAST MODIFIED : 5 August 2004
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
	for (i=variables_list.size();i>0;i--)
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
		variable_iterator++;
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
// LAST MODIFIED : 23 November 2004
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
		}
	}
	if (result=Function_variable_composite_handle(new Function_variable_composite(
		local_function,local_variables_list)))
	{
		result->value_private=value_private;
	}

	return (result);
}

string_handle Function_variable_composite::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 5 August 2004
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
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Function_variable_handle>::iterator iterator,iterator_end;

	iterator_end=variables_list.end();
	for (iterator=variables_list.begin();iterator!=iterator_end;iterator++)
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
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Function_variable_handle>::iterator iterator,iterator_end;

	iterator_end=variables_list.end();
	for (iterator=variables_list.begin();iterator!=iterator_end;iterator++)
	{
		(*iterator)->remove_dependent_function(dependent_function);
	}
}

bool Function_variable_composite::equality_atomic(
	const Function_variable_handle& variable) const
//******************************************************************************
// LAST MODIFIED : 13 August 2004
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
			variable_iterator_1++;
			variable_iterator_2++;
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
