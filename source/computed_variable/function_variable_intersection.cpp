//******************************************************************************
// FILE : function_variable_intersection.cpp
//
// LAST MODIFIED : 11 January 2005
//
// DESCRIPTION :
//==============================================================================

#include <algorithm>
#include <iterator>
#include <typeinfo>
#include <iostream>
#include <sstream>

#include "computed_variable/function_variable_intersection.hpp"

// module classes
// ==============

// class Function_variable_iterator_representation_atomic_intersection
// -------------------------------------------------------------------

bool intersect_atomic_variable(
	const std::list<Function_variable_handle>& variables_list,
	const Function_variable_iterator& atomic_variable_iterator)
//******************************************************************************
// LAST MODIFIED : 13 August 2004
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
			local_atomic_variable_iterator++;
		}
		intersect= !repeat;
		local_variables_list_iterator++;
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
				local_atomic_variable_iterator++;
			}
			local_variables_list_iterator++;
		}
	}

	return (intersect);
}

class Function_variable_iterator_representation_atomic_intersection: public
	Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 18 March 2004
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
							atomic_variable_iterator++;
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
					atomic_variable_iterator++;
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
					atomic_variable_iterator--;
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
// LAST MODIFIED : 11 January 2005
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
			iterator++;
			i--;
		}
	}
	if (result=Function_variable_intersection_handle(
		new Function_variable_intersection(local_function,local_variables_list)))
	{
		result->value_private=value_private;
	}

	return (result);
}

string_handle Function_variable_intersection::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 18 March 2004
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

void Function_variable_intersection::remove_dependent_function(
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
