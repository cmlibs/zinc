//******************************************************************************
// FILE : function_variable_exclusion.cpp
//
// LAST MODIFIED : 11 April 2004
//
// DESCRIPTION :
//==============================================================================

#include <algorithm>
#include <iterator>
#include <typeinfo>
#include <iostream>
#include <sstream>

#include "computed_variable/function_variable_exclusion.hpp"

// module classes
// ==============

// class Function_variable_iterator_representation_atomic_exclusion
// ----------------------------------------------------------------

bool include_atomic_variable(Function_variable_handle universe,
	Function_variable_handle exclusion,
	const Function_variable_iterator& atomic_variable_iterator)
//******************************************************************************
// LAST MODIFIED : 19 March 2004
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
		(include= !(**local_atomic_variable_iterator== **atomic_variable_iterator)))
	{
		local_atomic_variable_iterator++;
	}
	if (include)
	{
		local_atomic_variable_iterator=exclusion->begin_atomic();
		while ((local_atomic_variable_iterator!=exclusion->end_atomic())&&(include=
			!(**local_atomic_variable_iterator== **atomic_variable_iterator)))
		{
			local_atomic_variable_iterator++;
		}
	}

	return (include);
}

class Function_variable_iterator_representation_atomic_exclusion: public
	Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 19 March 2004
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
						atomic_variable_iterator++;
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
					atomic_variable_iterator++;
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
					atomic_variable_iterator--;
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
	Function_variable(),exclusion(exclusion),universe(universe)
//******************************************************************************
// LAST MODIFIED : 19 March 2004
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
// LAST MODIFIED : 19 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_exclusion(*this)));
}

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
