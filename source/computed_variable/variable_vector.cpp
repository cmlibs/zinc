//******************************************************************************
// FILE : variable_vector.cpp
//
// LAST MODIFIED : 9 February 2004
//
// DESCRIPTION :
//???DB.  Should be template?
//==============================================================================

#include "computed_variable/variable_base.hpp"

#include <new>
#include <sstream>
#include <string>
#include <stdio.h>

#include "computed_variable/variable_vector.hpp"

// module classes
// ==============

// class Variable_input_vector_values
// ----------------------------------

class Variable_input_vector_values;

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_vector_values>
	Variable_input_vector_values_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_vector_values>
	Variable_input_vector_values_handle;
#else
typedef Variable_input_vector_values * Variable_input_vector_values_handle;
#endif

class Variable_input_vector_values : public
#if defined (USE_VARIABLE_INPUT)
	Variable_input
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Variable_vector;
	public:
		Variable_input_vector_values(const Variable_vector_handle& variable_vector):
			variable_vector(variable_vector),indices(){};
		Variable_input_vector_values(const Variable_vector_handle& variable_vector,
			Variable_size_type index):variable_vector(variable_vector),indices(1)
		{
			indices[0]=index;
		};
		Variable_input_vector_values(const Variable_vector_handle& variable_vector,
			const ublas::vector<Variable_size_type>& indices):
			variable_vector(variable_vector),indices(indices){};
		~Variable_input_vector_values(){};
#if defined (USE_ITERATORS)
		// copy constructor
		Variable_input_vector_values(
			const Variable_input_vector_values& input_vector_values):
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			(),
			variable_vector(input_vector_values.variable_vector),
			indices(input_vector_values.indices){};
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			clone() const
		{
			return (Variable_input_vector_values_handle(
				new Variable_input_vector_values(*this)));
		}
		//???DB.  To be done
		virtual bool is_atomic();
#if defined (USE_ITERATORS_NESTED)
		virtual Iterator begin_atomic_inputs();
		virtual Iterator end_atomic_inputs();
#else // defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Variable_input_iterator begin_atomic_inputs();
		virtual Variable_input_iterator end_atomic_inputs();
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#if defined (USE_VARIABLE_INPUT)
		virtual Handle_iterator<Variable_input_handle> begin_atomic_inputs();
		virtual Handle_iterator<Variable_input_handle> end_atomic_inputs();
#else // defined (USE_VARIABLE_INPUT)
		virtual Handle_iterator<Variable_io_specifier_handle> begin_atomic();
		virtual Handle_iterator<Variable_io_specifier_handle> end_atomic();
#endif // defined (USE_VARIABLE_INPUT)
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // defined (USE_ITERATORS_NESTED)
#endif // defined (USE_ITERATORS)
		virtual Variable_size_type
#if defined (USE_ITERATORS)
			number_differentiable
#else // defined (USE_ITERATORS)
			size
#endif // defined (USE_ITERATORS)
			()
		{
			Variable_size_type result;

			result=indices.size();
			if (0==result)
			{
				result=variable_vector->
#if defined (USE_ITERATORS)
				number_differentiable
#else // defined (USE_ITERATORS)
				size
#endif // defined (USE_VARIABLE_ITERATORS)
				();
			}

			return (result);
		};
		virtual bool operator==(const
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			& input)
		{
			try
			{
				const Variable_input_vector_values& input_vector_values=
					dynamic_cast<const Variable_input_vector_values&>(input);
				bool result;

				result=false;
				if ((variable_vector==input_vector_values.variable_vector)&&
					(indices.size()==input_vector_values.indices.size()))
				{
					int i=indices.size();

					result=true;
					while (result&&(i>0))
					{
						i--;
						if (!(indices[i]==input_vector_values.indices[i]))
						{
							result=false;
						}
					}
				}

				return (result);
			}
			catch (std::bad_cast)
			{
				return (false);
			}
		};
#if defined (USE_SCALAR_MAPPING)
	private:
		virtual std::list< std::pair<Variable_size_type,Variable_size_type> >
			scalar_mapping_local(Variable_input_handle target)
		{
			std::list< std::pair<Variable_size_type,Variable_size_type> > result(0);
			const Variable_input_vector_values_handle input_vector_values=
#if defined (USE_SMART_POINTER)
				boost::dynamic_pointer_cast<Variable_input_vector_values,
#if defined (USE_VARIABLE_INPUT)
				Variable_input
#else // defined (USE_VARIABLE_INPUT)
				Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
				>
#else /* defined (USE_SMART_POINTER) */
				dynamic_cast<Variable_input_vector_values *>
#endif /* defined (USE_SMART_POINTER) */
				(target);
			Variable_size_type source_size,target_size;

			target_size=target->size();
			source_size=size();
			if (input_vector_values)
			{
				if (variable_vector==input_vector_values->variable_vector)
				{
					if (0==indices.size())
					{
						if (0==(input_vector_values->indices).size())
						{
							result.push_back(
								std::pair<Variable_size_type,Variable_size_type>(0,0));
						}
						else
						{
							Assert(target_size==(input_vector_values->indices).size(),
								std::logic_error(
								"Variable_input_vector_values::scalar_mapping_local.  "
								"Error in calculating target size 1"));
							
							Variable_size_type i,j;

							for (i=0;i<source_size;i++)
							{
								j=0;
								while ((j<target_size)&&(i!=(input_vector_values->indices)[j]))
								{
									j++;
								}
								if ((0==i)||(i-(result.back()).first!=j-(result.back()).second))
								{
									result.push_back(std::pair<Variable_size_type,
										Variable_size_type>(i,j));
								}
							}
						}
					}
					else
					{
						Assert(source_size==indices.size(),std::logic_error(
							"Variable_input_vector_values::scalar_mapping_local.  "
							"Error in calculating source size"));
						if (0==(input_vector_values->indices).size())
						{
							Variable_size_type i,j;

							for (i=0;i<source_size;i++)
							{
								j=indices[i];
								if ((0==i)||(i-(result.back()).first!=j-(result.back()).second))
								{
									result.push_back(std::pair<Variable_size_type,
										Variable_size_type>(i,j));
								}
							}
						}
						else
						{
							Assert(target_size==(input_vector_values->indices).size(),
								std::logic_error(
								"Variable_input_vector_values::scalar_mapping_local.  "
								"Error in calculating target size 2"));

							Variable_size_type i,j;

							for (i=0;i<source_size;i++)
							{
								j=0;
								while ((j<target_size)&&
									(indices[i]!=(input_vector_values->indices)[j]))
								{
									j++;
								}
								if ((0==i)||(i-(result.back()).first!=j-(result.back()).second))
								{
									result.push_back(std::pair<Variable_size_type,
										Variable_size_type>(i,j));
								}
							}
						}
					}
				}
			}
			if (0==result.size())
			{
				result.push_back(std::pair<Variable_size_type,Variable_size_type>(
					0,target_size));
			}
			if (0<source_size)
			{
				result.push_back(std::pair<Variable_size_type,Variable_size_type>(
					source_size,target_size));
			}

			return (result);
		};
#endif // defined (USE_SCALAR_MAPPING)
	private:
		Variable_vector_handle variable_vector;
		ublas::vector<Variable_size_type> indices;
};

// global classes
// ==============

// class Variable_vector
// ---------------------

Variable_vector::Variable_vector(const Vector& values):Variable(),values(values)
//******************************************************************************
// LAST MODIFIED : 24 October 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
}

Variable_vector::Variable_vector(const Variable_vector& variable_vector):
	Variable(),values(variable_vector.values)
//******************************************************************************
// LAST MODIFIED : 22 October 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Variable_vector& Variable_vector::operator=(
	const Variable_vector& variable_vector)
//******************************************************************************
// LAST MODIFIED : 22 October 2003
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	this->values=variable_vector.values;

	return (*this);
}

Variable_vector::~Variable_vector()
//******************************************************************************
// LAST MODIFIED : 7 September 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

Scalar& Variable_vector::operator[](Variable_size_type i)
//******************************************************************************
// LAST MODIFIED : 9 November 2003
//
// DESCRIPTION :
// Indexing.
//==============================================================================
{
	return (values[i]);
}

Scalar Variable_vector::operator[](Variable_size_type i) const
//******************************************************************************
// LAST MODIFIED : 23 December 2003
//
// DESCRIPTION :
// Indexing.
//==============================================================================
{
	return (values[i]);
}

Variable_size_type Variable_vector::
#if defined (USE_ITERATORS)
	number_differentiable
#else // defined (USE_ITERATORS)
	size
#endif // defined (USE_VARIABLE_ITERATORS)
	() const
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (values.size());
}

#if defined (USE_ITERATORS)
#else // defined (USE_ITERATORS)
Vector *Variable_vector::scalars()
//******************************************************************************
// LAST MODIFIED : 24 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (new Vector(values));
}
#endif // defined (USE_VARIABLE_ITERATORS)

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_vector::input_values()
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the values input for a vector.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_vector_values(Variable_vector_handle(this))));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_vector::input_values(Variable_size_type index)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the values input for a vector.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_vector_values(Variable_vector_handle(this),index)));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_vector::input_values(
	const ublas::vector<Variable_size_type> indices)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the values input for a vector.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_vector_values(Variable_vector_handle(this),indices)));
}

Scalar Variable_vector::norm() const
//******************************************************************************
// LAST MODIFIED : 11 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	Scalar result;
	Variable_size_type i,number_of_values;

	result=0;
	if (this&&(0<(number_of_values=values.size())))
	{
		for (i=0;i<number_of_values;i++)
		{
			result += values[i]*values[i];
		}
		result=sqrt(result);
	}

	return (result);
}

Variable_handle Variable_vector::operator-(const Variable& second) const
//******************************************************************************
// LAST MODIFIED : 12 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_vector_handle result(0);

	try
	{
		const Variable_vector& second_vector=
			dynamic_cast<const Variable_vector&>(second);
		Variable_size_type i,number_of_values;

		number_of_values=second_vector.values.size();
		if (this&&(values.size()==number_of_values)&&
			(result=Variable_vector_handle(new Variable_vector(*this))))
		{
			for (i=0;i<number_of_values;i++)
			{
				(result->values)[i] -= (second_vector.values)[i];
			}
		}
	}
	catch (std::bad_cast)
	{
		// do nothing
	}

	return (result);
}

Variable_handle Variable_vector::operator-=(const Variable& second)
//******************************************************************************
// LAST MODIFIED : 15 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	try
	{
		const Variable_vector& second_vector=
			dynamic_cast<const Variable_vector&>(second);
		Variable_size_type i,number_of_values;

		number_of_values=second_vector.values.size();
		if (this&&(values.size()==number_of_values))
		{
			for (i=0;i<number_of_values;i++)
			{
				values[i] -= (second_vector.values)[i];
			}
		}
	}
	catch (std::bad_cast)
	{
		// do nothing
	}

	return (Variable_vector_handle(this));
}

Variable_handle Variable_vector::clone() const
//******************************************************************************
// LAST MODIFIED : 8 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (Variable_vector_handle(new Variable_vector(*this)));
}

Variable_handle Variable_vector::evaluate_local()
//******************************************************************************
// LAST MODIFIED : 22 October 2003
//
// DESCRIPTION :
// Duplicate <this> so that <this> isn't changed by operations on the result.
//==============================================================================
{
	return (Variable_handle(new Variable_vector(*this)));
}

#if defined (USE_ITERATORS)
//???DB.  To be done
#else // defined (USE_ITERATORS)
bool Variable_vector::evaluate_derivative_local(Matrix& matrix,
	std::list<
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Variable_size_type i,index,number_of_input_values,number_of_values;
	Variable_input_vector_values_handle input_values_handle;

	result=true;
	// matrix is zero'd on entry
	if ((1==independent_variables.size())&&(input_values_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_vector_values,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(independent_variables.front())
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_vector_values *>(independent_variables.front())
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_values_handle->variable_vector==Variable_handle(this)))
	{
		number_of_values=this->size();
		number_of_input_values=(input_values_handle->indices).size();
		Assert((number_of_values==matrix.size1())&&((0==number_of_input_values)||
			(number_of_input_values==matrix.size2())),std::logic_error(
			"Variable_vector::evaluate_derivative_local.  "
			"Incorrect matrix size"));
		if (0==number_of_input_values)
		{
			for (i=0;i<number_of_values;i++)
			{
				matrix(i,i)=1;
			}
		}
		else
		{
			for (i=0;i<number_of_input_values;i++)
			{
				index=input_values_handle->indices[i];
				if ((0<index)&&(index<=number_of_values))
				{
					matrix(index-1,i)=1;
				}
			}
		}
	}

	return (result);
}
#endif // defined (USE_ITERATORS)

Variable_handle Variable_vector::get_input_value_local(
	const
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	& input)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	Variable_vector_handle values_vector;
	Variable_input_vector_values_handle input_values_handle;

	if ((input_values_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_vector_values,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_vector_values *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_values_handle->variable_vector==Variable_vector_handle(this)))
	{
		Variable_size_type number_of_input_values=input_values_handle->
#if defined (USE_ITERATORS)
			number_differentiable
#else // defined (USE_ITERATORS)
			size
#endif // defined (USE_VARIABLE_ITERATORS)
			();

		if (0==(input_values_handle->indices).size())
		{
			values_vector=Variable_vector_handle(new Variable_vector(values));
		}
		else
		{
			Variable_size_type i,index,number_of_values=this->
#if defined (USE_ITERATORS)
				number_differentiable
#else // defined (USE_ITERATORS)
				size
#endif // defined (USE_VARIABLE_ITERATORS)
				();
			ublas::vector<Scalar> selected_values(number_of_input_values);

			for (i=0;i<number_of_input_values;i++)
			{
				index=input_values_handle->indices[i];
				if ((0<index)&&(index<=number_of_values))
				{
					selected_values[i]=values[index-1];
				}
			}
			values_vector=
				Variable_vector_handle(new Variable_vector(selected_values));
		}
	}
	else
	{
		values_vector=Variable_vector_handle((Variable_vector *)0);
	}

	return (values_vector);
}

bool Variable_vector::set_input_value_local(const
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
	&
#if defined (USE_ITERATORS)
	//???DB.  To be done
#else // defined (USE_ITERATORS)
	values
#endif // defined (USE_VARIABLE_ITERATORS)
	)
//******************************************************************************
// LAST MODIFIED : 9 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Variable_input_vector_values_handle input_values_handle;
	Vector *values_vector;

	result=false;
	if ((input_values_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_vector_values,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_vector_values *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_values_handle->variable_vector==Variable_vector_handle(this))&&
		(values_vector=
#if defined (USE_ITERATORS)
		//???DB.  To be done
		0
#else // defined (USE_ITERATORS)
		values->scalars()
#endif // defined (USE_VARIABLE_ITERATORS)
		))
	{
		Variable_size_type number_of_input_values=
			(input_values_handle->indices).size();

		if (0==number_of_input_values)
		{
			if ((this->values).size()==values_vector->size())
			{
				this->values= *values_vector;
				result=true;
			}
		}
		else
		{
			if (number_of_input_values==values_vector->size())
			{
				Variable_size_type i,index,number_of_values=(this->values).size();

				for (i=0;i<number_of_input_values;i++)
				{
					index=input_values_handle->indices[i];
					if ((0<index)&&(index<=number_of_values))
					{
						(this->values)[index-1]=(*values_vector)[i];
					}
				}
				result=true;
			}
		}
		delete values_vector;
	}

	return (result);
}

string_handle Variable_vector::get_string_representation_local()
//******************************************************************************
// LAST MODIFIED : 22 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		out << values;
		*return_string=out.str();
	}

	return (return_string);
}
