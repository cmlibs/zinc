//******************************************************************************
// FILE : function_variable_union.cpp
//
// LAST MODIFIED : 19 March 2004
//
// DESCRIPTION :
//==============================================================================

#include <algorithm>
#include <iterator>
#include <typeinfo>
#include <iostream>

#include "computed_variable/function_variable_union.hpp"

// module classes
// ==============

// class Function_variable_iterator_representation_atomic_union
// ------------------------------------------------------------

bool repeat_atomic_variable(
	const std::list<Function_variable_handle>& variables_list,
	const std::list<Function_variable_handle>::iterator& variables_list_iterator,
	const Function_variable_iterator& atomic_variable_iterator)
//******************************************************************************
// LAST MODIFIED : 18 March 2004
//
// DESCRIPTION :
// Determines if (*atomic_variable_iterator) appears before it in the union.
//==============================================================================
{
	bool repeat;
	Function_variable_iterator local_atomic_variable_iterator;
	std::list<Function_variable_handle>::const_iterator
		local_variables_list_iterator;

	repeat=false;
	local_atomic_variable_iterator=(*variables_list_iterator)->begin_atomic();
	while ((local_atomic_variable_iterator!=atomic_variable_iterator)&&
		!(repeat=(**local_atomic_variable_iterator== **atomic_variable_iterator)))
	{
		local_atomic_variable_iterator++;
	}
	if (!repeat)
	{
		local_variables_list_iterator=variables_list.begin();
		while ((local_variables_list_iterator!=variables_list_iterator)&&!repeat)
		{
			local_atomic_variable_iterator=
				(*local_variables_list_iterator)->begin_atomic();
			while ((local_atomic_variable_iterator!=
				(*local_variables_list_iterator)->end_atomic())&&
				!(repeat=(**local_atomic_variable_iterator==
				**atomic_variable_iterator)))
			{
				local_atomic_variable_iterator++;
			}
			local_variables_list_iterator++;
		}
	}

	return (repeat);
}

class Function_variable_iterator_representation_atomic_union: public
	Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 12 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_union(
			const bool begin,Function_variable_union_handle source):
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
				result=new Function_variable_iterator_representation_atomic_union(
					*this);
			}

			return (result);
		};
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_union(){};
	private:
		// increment
		void increment()
		{
			if (variables_list_iterator!=(source->variables_list).end())
			{
				do
				{
					atomic_variable_iterator++;
				} while ((atomic_variable_iterator!=
					((*variables_list_iterator)->end_atomic)())&&
					repeat_atomic_variable(source->variables_list,
					variables_list_iterator,atomic_variable_iterator));
				if (atomic_variable_iterator==
					((*variables_list_iterator)->end_atomic)())
				{
					do
					{
						variables_list_iterator++;
						if (variables_list_iterator!=(source->variables_list).end())
						{
							atomic_variable_iterator=
								((*variables_list_iterator)->begin_atomic)();
							while ((atomic_variable_iterator!=
								((*variables_list_iterator)->end_atomic)())&&
								repeat_atomic_variable(source->variables_list,
								variables_list_iterator,atomic_variable_iterator))
							{
								atomic_variable_iterator++;
							}
						}
					} while ((variables_list_iterator!=(source->variables_list).end())&&
						(atomic_variable_iterator==
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
			bool done;

			done=false;
			if ((variables_list_iterator==(source->variables_list).end())||
				(atomic_variable_iterator==
				((*variables_list_iterator)->begin_atomic)()))
			{
				if (variables_list_iterator==(source->variables_list).begin())
				{
					done=true;
					variables_list_iterator=(source->variables_list).end();
					atomic_variable_iterator=0;
				}
				else
				{
					do
					{
						variables_list_iterator--;
						atomic_variable_iterator=((*variables_list_iterator)->end_atomic)();
					} while ((variables_list_iterator!=(source->variables_list).begin())&&
						(atomic_variable_iterator==
						((*variables_list_iterator)->begin_atomic)()));
					if ((variables_list_iterator==(source->variables_list).begin())&&
						(atomic_variable_iterator==
						((*variables_list_iterator)->begin_atomic)()))
					{
						done=true;
						variables_list_iterator=(source->variables_list).end();
						atomic_variable_iterator=0;
					}
				}
			}
			while (!done)
			{
				do
				{
					atomic_variable_iterator--;
					done= !repeat_atomic_variable(source->variables_list,
						variables_list_iterator,atomic_variable_iterator);
				} while ((atomic_variable_iterator!=
					((*variables_list_iterator)->begin_atomic)())&&!done);
				if (!done)
				{
					while ((variables_list_iterator!=(source->variables_list).begin())&&
						(atomic_variable_iterator==
						((*variables_list_iterator)->begin_atomic)()))
					{
						variables_list_iterator--;
						atomic_variable_iterator=((*variables_list_iterator)->end_atomic)();
					}
					if ((variables_list_iterator==(source->variables_list).begin())&&
						(atomic_variable_iterator==
						((*variables_list_iterator)->begin_atomic)()))
					{
						done=true;
						variables_list_iterator=(source->variables_list).end();
						atomic_variable_iterator=0;
					}
				}
			}
		};
		// equality
		bool equality(const Function_variable_iterator_representation
			* representation)
		{
			const Function_variable_iterator_representation_atomic_union
				*representation_union=dynamic_cast<
				const Function_variable_iterator_representation_atomic_union *>(
				representation);

			return (representation_union&&
				(variables_list_iterator==
				representation_union->variables_list_iterator)&&
				(atomic_variable_iterator==
				representation_union->atomic_variable_iterator));
		};
		// dereference
		Function_variable_handle dereference() const
		{
			return (*atomic_variable_iterator);
		};
	private:
		Function_variable_iterator_representation_atomic_union(const
			Function_variable_iterator_representation_atomic_union&
			representation):Function_variable_iterator_representation(),
			atomic_variable_iterator(representation.atomic_variable_iterator),
			variables_list_iterator(representation.variables_list_iterator),
			source(representation.source){};
	private:
		Function_variable_iterator atomic_variable_iterator;
		std::list<Function_variable_handle>::iterator variables_list_iterator;
		Function_variable_union_handle source;
};

// global classes
// ==============

// class Function_variable_union
// -----------------------------

Function_variable_union::Function_variable_union(
	const Function_variable_handle& variable_1,
	const Function_variable_handle& variable_2):
	Function_variable(),variables_list(0)
//******************************************************************************
// LAST MODIFIED : 12 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	variables_list.push_back(variable_1);
	variables_list.push_back(variable_2);
}

Function_variable_union::Function_variable_union(
	const Function_handle& function,
	const Function_variable_handle& variable_1,
	const Function_variable_handle& variable_2):Function_variable(function),
	variables_list(0)
//******************************************************************************
// LAST MODIFIED : 12 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	variables_list.push_back(variable_1);
	variables_list.push_back(variable_2);
}

Function_variable_union::Function_variable_union(
	std::list<Function_variable_handle>& variables_list):
	Function_variable(),variables_list(variables_list)
//******************************************************************************
// LAST MODIFIED : 12 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Function_variable_union::Function_variable_union(
	const Function_handle& function,
	std::list<Function_variable_handle>& variables_list):
	Function_variable(function),variables_list(variables_list)
//******************************************************************************
// LAST MODIFIED : 12 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Function_variable_handle Function_variable_union::clone() const
//******************************************************************************
// LAST MODIFIED : 12 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_union(*this)));
}

string_handle Function_variable_union::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 12 March 2004
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
		out << "union(";
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

Function_variable_iterator Function_variable_union::begin_atomic() const
//******************************************************************************
// LAST MODIFIED : 15 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_iterator(
		new Function_variable_iterator_representation_atomic_union(true,
		Function_variable_union_handle(
		const_cast<Function_variable_union*>(this)))));
}

Function_variable_iterator Function_variable_union::end_atomic() const
//******************************************************************************
// LAST MODIFIED : 15 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_iterator(
		new Function_variable_iterator_representation_atomic_union(false,
		Function_variable_union_handle(
		const_cast<Function_variable_union*>(this)))));
}

std::reverse_iterator<Function_variable_iterator>
	Function_variable_union::rbegin_atomic() const
//******************************************************************************
// LAST MODIFIED : 15 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (std::reverse_iterator<Function_variable_iterator>(
		new Function_variable_iterator_representation_atomic_union(false,
		Function_variable_union_handle(
		const_cast<Function_variable_union*>(this)))));
}

std::reverse_iterator<Function_variable_iterator>
	Function_variable_union::rend_atomic() const
//******************************************************************************
// LAST MODIFIED : 15 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (std::reverse_iterator<Function_variable_iterator>(
		new Function_variable_iterator_representation_atomic_union(true,
		Function_variable_union_handle(
		const_cast<Function_variable_union*>(this)))));
}

bool Function_variable_union::equality_atomic(const
	Function_variable_handle&) const
//******************************************************************************
// LAST MODIFIED : 15 March 2004
//
// DESCRIPTION :
//???DB.  This should be == for everything?  How to get atomic?
//==============================================================================
{
	// should not come here - handled by overloading Function_variable::operator==
	Assert(false,std::logic_error(
		"Function_variable_union::equality_atomic.  "
		"Should not come here"));

	return (false);
}

Function_variable_union::Function_variable_union(
	const Function_variable_union& variable_union):
	Function_variable(variable_union),
	variables_list(variable_union.variables_list)
//******************************************************************************
// LAST MODIFIED : 12 March 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Function_variable_union& Function_variable_union::operator=(
	const Function_variable_union& variable_union)
//******************************************************************************
// LAST MODIFIED : 12 March 2004
//
// DESCRIPTION :
// Assignment operator.
//???DB.  Same as implicit?
//==============================================================================
{
	//???DB.  Does assignment for super class first?
	variables_list=variable_union.variables_list;

	return (*this);
}
