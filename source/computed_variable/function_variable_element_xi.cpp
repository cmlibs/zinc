//******************************************************************************
// FILE : function_variable_element_xi.cpp
//
// LAST MODIFIED : 2 July 2004
//
// DESCRIPTION :
//==============================================================================

#include <new>
#include <sstream>
#include <string>
#include <stdio.h>

extern "C"
{
#include "finite_element/finite_element_region.h"
//???DB.  Get rid of debug.h (C->C++)
#include "general/debug.h"
}
#include "computed_variable/function_variable_element_xi.hpp"
#include "computed_variable/function_variable_value_element.hpp"
#include "computed_variable/function_variable_value_scalar.hpp"


// module functions
// ================

bool Function_variable_element_xi_set_scalar_function(Scalar& value,
	const Function_variable_handle variable)
{
	bool result;
	Function_variable_element_xi_handle element_xi_variable;

	result=false;
	if (element_xi_variable=boost::dynamic_pointer_cast<
		Function_variable_element_xi,Function_variable>(variable))
	{
		result=element_xi_variable->get_xi(value);
	}

	return (result);
}

//???DB.  Should this ACCESS(FE_element)?
bool Function_variable_element_xi_set_element_function(
	struct FE_element*& element,const Function_variable_handle variable)
{
	bool result;
	Function_variable_element_xi_handle element_xi_variable;

	result=false;
	if (element_xi_variable=boost::dynamic_pointer_cast<
		Function_variable_element_xi,Function_variable>(variable))
	{
		result=element_xi_variable->get_element(element);
	}

	return (result);
}


// module classes
// ==============

// class Function_variable_iterator_representation_atomic_element_xi
// -----------------------------------------------------------------

class Function_variable_iterator_representation_atomic_element_xi:
	public Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_element_xi(
			const bool begin,Function_variable_element_xi_handle variable):
			variable_index(0),atomic_variable(0),variable(variable)
		{
			if (begin&&variable)
			{
				if (atomic_variable=
					boost::dynamic_pointer_cast<Function_variable_element_xi,
					Function_variable>(variable->clone()))
				{
					atomic_variable->indices.resize(1);
					atomic_variable->indices[0]=1;
					if (variable->element_private)
					{
						atomic_variable->xi_private=false;
						atomic_variable->value_private=Function_variable_value_handle(
							new Function_variable_value_element(
							Function_variable_element_xi_set_element_function));
					}
					else
					{
						if ((variable->xi_private)&&(0<variable->number_differentiable()))
						{
							if (0<(variable->indices).size())
							{
								atomic_variable->indices[0]=variable->indices[0];
							}
							atomic_variable->value_private=Function_variable_value_handle(
								new Function_variable_value_scalar(
								Function_variable_element_xi_set_scalar_function));
						}
						else
						{
							atomic_variable=0;
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
					Function_variable_iterator_representation_atomic_element_xi(*this);
			}

			return (result);
		};
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_element_xi(){};
	private:
		// increment
		void increment()
		{
			if (atomic_variable)
			{
				if (atomic_variable->element_private)
				{
					atomic_variable->element_private=false;
					if (variable->xi_private)
					{
						atomic_variable->xi_private=true;
						variable_index=0;
						atomic_variable->value_private=Function_variable_value_handle(
							new Function_variable_value_scalar(
							Function_variable_element_xi_set_scalar_function));
					}
					else
					{
						// end
						atomic_variable=0;
					}
				}
				else
				{
					Assert(atomic_variable->xi_private,std::logic_error(
						"Function_variable_iterator_representation_atomic_element_xi::"
						"increment.  Atomic variable should not have element and xi both "
						"false"));
					variable_index++;
				}
				if (atomic_variable)
				{
					Assert(atomic_variable->xi_private,std::logic_error(
						"Function_variable_iterator_representation_atomic_element_xi::"
						"increment.  Second and subsequent atomic variables should be xi"));
					if (variable_index<variable->number_differentiable())
					{
						if (variable_index<(variable->indices).size())
						{
							atomic_variable->indices[0]=(variable->indices)[variable_index];
						}
						else
						{
							atomic_variable->indices[0]=variable_index+1;
						}
					}
					else
					{
						// end
						atomic_variable=0;
					}
				}
			}
		};
		// decrement
		void decrement()
		{
			if (atomic_variable)
			{
				if (atomic_variable->element_private)
				{
					// end
					atomic_variable=0;
				}
				else
				{
					if (0==variable_index)
					{
						if (variable->element_private)
						{
							atomic_variable->element_private=true;
							atomic_variable->xi_private=false;
							atomic_variable->value_private=Function_variable_value_handle(
								new Function_variable_value_element(
								Function_variable_element_xi_set_element_function));
						}
						else
						{
							// end
							atomic_variable=0;
						}
					}
					else
					{
						variable_index--;
						if (variable_index<(variable->indices).size())
						{
							atomic_variable->indices[0]=(variable->indices)[variable_index];
						}
						else
						{
							atomic_variable->indices[0]=variable_index+1;
						}
					}
				}
			}
			else
			{
				if (atomic_variable=
					boost::dynamic_pointer_cast<Function_variable_element_xi,
					Function_variable>(variable->clone()))
				{
					atomic_variable->indices.resize(1);
					atomic_variable->indices[0]=1;
					if ((variable->xi_private)&&(0<variable->number_differentiable()))
					{
						atomic_variable->element_private=false;
						if (0<(variable->indices).size())
						{
							variable_index=((variable->indices).size())-1;
							atomic_variable->indices[0]=variable->indices[variable_index];
						}
						else
						{
							variable_index=variable->number_differentiable();
							atomic_variable->indices[0]=variable_index;
							variable_index--;
						}
						atomic_variable->value_private=Function_variable_value_handle(
							new Function_variable_value_scalar(
							Function_variable_element_xi_set_scalar_function));
					}
					else
					{
						if (variable->element_private)
						{
							atomic_variable->xi_private=false;
							atomic_variable->value_private=Function_variable_value_handle(
								new Function_variable_value_element(
								Function_variable_element_xi_set_element_function));
						}
						else
						{
							atomic_variable=0;
						}
					}
				}
			}
		};
		// equality
		bool equality(
			const Function_variable_iterator_representation* representation)
		{
			bool result;
			const Function_variable_iterator_representation_atomic_element_xi
				*representation_element_xi=dynamic_cast<
				const Function_variable_iterator_representation_atomic_element_xi *>(
				representation);

			result=false;
			if (representation_element_xi)
			{
				if (((0==atomic_variable)&&
					(0==representation_element_xi->atomic_variable))||
					(atomic_variable&&(representation_element_xi->atomic_variable)&&
					(*atomic_variable== *(representation_element_xi->atomic_variable))))
				{
					result=true;
				}
			}

			return (result);
		};
		// dereference
		Function_variable_handle dereference() const
		{
			return (atomic_variable);
		};
	private:
		// copy constructor
		Function_variable_iterator_representation_atomic_element_xi(
			const Function_variable_iterator_representation_atomic_element_xi&
			representation):Function_variable_iterator_representation(),
			variable_index(representation.variable_index),atomic_variable(0),
			variable(representation.variable)
		{
			if (representation.atomic_variable)
			{
				atomic_variable=boost::dynamic_pointer_cast<
					Function_variable_element_xi,Function_variable>(
					(representation.atomic_variable)->clone());
			}
		};
	private:
		Function_size_type variable_index;
		Function_variable_element_xi_handle atomic_variable,variable;
};


// global classes
// ==============

// class Function_variable_element_xi
// ----------------------------------

string_handle Function_variable_element_xi::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string(0);

	if (return_string=new std::string)
	{
		std::ostringstream out;

		if (element_private)
		{
			out << "element";
			if (xi_private)
			{
				out << "_";
			}
		}
		if (xi_private)
		{
			out << "xi";
			if (0<indices.size())
			{
				out << indices;
			}
		}
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_iterator Function_variable_element_xi::begin_atomic() const
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_iterator(
		new Function_variable_iterator_representation_atomic_element_xi(true,
		Function_variable_element_xi_handle(
		const_cast<Function_variable_element_xi*>(this)))));
}

Function_variable_iterator Function_variable_element_xi::end_atomic() const
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_iterator(
		new Function_variable_iterator_representation_atomic_element_xi(false,
		Function_variable_element_xi_handle(
		const_cast<Function_variable_element_xi*>(this)))));
}

std::reverse_iterator<Function_variable_iterator>
	Function_variable_element_xi::rbegin_atomic() const
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (std::reverse_iterator<Function_variable_iterator>(
		new Function_variable_iterator_representation_atomic_element_xi(
		false,Function_variable_element_xi_handle(
		const_cast<Function_variable_element_xi*>(this)))));
}

std::reverse_iterator<Function_variable_iterator>
	Function_variable_element_xi::rend_atomic() const
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (std::reverse_iterator<Function_variable_iterator>(
		new Function_variable_iterator_representation_atomic_element_xi(
		true,Function_variable_element_xi_handle(
		const_cast<Function_variable_element_xi*>(this)))));
}

Function_size_type Function_variable_element_xi::number_differentiable()
//******************************************************************************
// LAST MODIFIED : 2 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (number_of_xi());
}

bool Function_variable_element_xi::equality_atomic(
	const Function_variable_handle& variable) const
//******************************************************************************
// LAST MODIFIED : 2 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_element_xi_handle variable_element_xi;

	result=false;
	if (variable_element_xi=boost::dynamic_pointer_cast<
		Function_variable_element_xi,Function_variable>(variable))
	{
		if ((variable_element_xi->function()==function())&&
			(variable_element_xi->element_private==element_private)&&
			(variable_element_xi->xi_private==xi_private)&&
			((variable_element_xi->indices).size()==indices.size()))
		{
			int i=indices.size();

			result=true;
			while (result&&(i>0))
			{
				i--;
				if (!(indices[i]==(variable_element_xi->indices)[i]))
				{
					result=false;
				}
			}
		}
	}

	return (result);
}

Function_variable_element_xi::Function_variable_element_xi(
	const Function_handle& function,bool element,bool xi,
	const ublas::vector<Function_size_type>& indices):Function_variable(function),
	element_private(element),xi_private(xi),indices(indices){}
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================

Function_variable_element_xi::Function_variable_element_xi(
	const Function_handle& function,Function_size_type index):
	Function_variable(function),element_private(false),xi_private(true),indices(1)
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	indices[0]=index;
}

Function_variable_element_xi::Function_variable_element_xi(
	const Function_handle& function,
	const ublas::vector<Function_size_type>& indices):
	Function_variable(function),element_private(false),xi_private(true),
	indices(indices)
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	// remove repeated indices
	Function_size_type number_of_indices=indices.size();
	ublas::vector<Function_size_type> unique_indices(number_of_indices);
	Function_size_type i,j,number_of_unique_indices;

	number_of_unique_indices=0;
	for (i=0;i<number_of_indices;i++)
	{
		j=0;
		while ((j<number_of_unique_indices)&&(indices[i]!=unique_indices[j]))
		{
			j++;
		}
		if (j==number_of_unique_indices)
		{
			unique_indices[j]=indices[i];
			number_of_unique_indices++;
		}
	}
	if (number_of_indices!=number_of_unique_indices)
	{
		(this->indices).resize(number_of_unique_indices);
		for (i=0;i<number_of_unique_indices;i++)
		{
			(this->indices)[i]=unique_indices[i];
		}
	}
}

Function_variable_element_xi::~Function_variable_element_xi(){}
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
//==============================================================================

Function_variable_element_xi::Function_variable_element_xi(
	const Function_variable_element_xi& variable_element_xi):
	Function_variable(variable_element_xi),
	element_private(variable_element_xi.element_private),
	xi_private(variable_element_xi.xi_private),
	indices(variable_element_xi.indices){}
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================

#if defined (TO_BE_DONE)
//???DB.  Should these be on Functions?
Function_variable_handle operator-(const Function_variable& second) const
{
	Function_variable_element_xi_handle result(0);

	try
	{
		const Function_variable_matrix& second_vector=
			dynamic_cast<const Function_variable_matrix&>(second);
		Function_size_type i,number_of_values;

		number_of_values=second_vector.number_of_rows();
		if (this&&(1==second_vector.number_of_columns)&&
			(xi.size()==number_of_values)&&(0<number_of_values))
		{
			FE_value *increment_array,*xi_array;

			increment_array=new FE_value[number_of_values];
			xi_array=new FE_value[number_of_values];
			if (increment_array&&xi_array)
			{
				struct FE_element *increment_element;
				for (i=0;i<number_of_values;i++)
				{
					increment_array[i]=(FE_value)second_vector(i,1);
					xi_array[i]=(FE_value)xi[i];
				}
				increment_element=element;
				if (FE_element_xi_increment(&increment_element,xi_array,
					increment_array))
				{
					if (result=Function_element_xi_handle(new Function_element_xi(
						increment_element,xi)))
					{
						for (i=0;i<number_of_values;i++)
						{
							(result->xi)[i]=(Scalar)(xi_array[i]);
						}
					}
				}
			}
			delete [] increment_array;
			delete [] xi_array;
		}
	}
	catch (std::bad_cast)
	{
		// do nothing
	}

	return (result);
};
Function_variable_handle operator-=(const Function_variable& second)
{
	try
	{
		const Function_variable_matrix& second_vector=
			dynamic_cast<const Function_variable_matrix&>(second);
		Function_size_type i,number_of_values;

		number_of_values=second_vector.number_of_rows();
		if (this&&(1==second_vector.number_of_columns)&&
			(xi.size()==number_of_values)&&(0<number_of_values))
		{
			FE_value *increment_array,*xi_array;

			increment_array=new FE_value[number_of_values];
			xi_array=new FE_value[number_of_values];
			if (increment_array&&xi_array)
			{
				struct FE_element *increment_element;
				for (i=0;i<number_of_values;i++)
				{
					increment_array[i]=(FE_value)second_vector(i,1);
					xi_array[i]=(FE_value)xi[i];
				}
				increment_element=element;
				if (FE_element_xi_increment(&increment_element,xi_array,
					increment_array))
				{
					if (element)
					{
						DEACCESS(FE_element)(&element);
					}
					element=ACCESS(FE_element)(increment_element);
					for (i=0;i<number_of_values;i++)
					{
						xi[i]=(Scalar)(xi_array[i]);
					}
				}
			}
			delete [] increment_array;
			delete [] xi_array;
		}
	}
	catch (std::bad_cast)
	{
		// do nothing
	}

	return (Function_variable_element_xi_handle(this));
};
#endif // defined (TO_BE_DONE)
