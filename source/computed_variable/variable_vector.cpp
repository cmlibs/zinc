//******************************************************************************
// FILE : variable_vector.cpp
//
// LAST MODIFIED : 9 November 2003
//
// DESCRIPTION :
//???DB.  Should be template?
//==============================================================================

#include <new>
#include <sstream>
#include <string>
#include <stdio.h>

//???DB.  Put in include?
const bool Assert_on=true;

template<class Assertion,class Exception>inline void Assert(
	Assertion assertion,Exception exception)
{
	if (Assert_on&&!(assertion)) throw exception;
}

#include "computed_variable/variable_vector.hpp"

// module classes
// ==============

// class Variable_input_vector_values
// ----------------------------------

class Variable_input_vector_values : public Variable_input
//******************************************************************************
// LAST MODIFIED : 6 November 2003
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
			const boost::numeric::ublas::vector<Variable_size_type>& indices):
			variable_vector(variable_vector),indices(indices){};
		~Variable_input_vector_values(){};
		Variable_size_type size()
		{
			Variable_size_type result;

			result=indices.size();
			if (0==result)
			{
				result=variable_vector->size();
			}

			return (result);
		};
		virtual bool operator==(const Variable_input& input)
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
		};
	private:
		Variable_vector_handle variable_vector;
		boost::numeric::ublas::vector<Variable_size_type> indices;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_vector_values>
	Variable_input_vector_values_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_vector_values>
	Variable_input_vector_values_handle;
#else
typedef Variable_input_vector_values * Variable_input_vector_values_handle;
#endif

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

#if defined (NEW_CODE)
const Scalar& Variable_vector::operator[](Variable_size_type i) const
//******************************************************************************
// LAST MODIFIED : 9 November 2003
//
// DESCRIPTION :
// Indexing.
//==============================================================================
{
	return (values[i]);
}
#endif // defined (NEW_CODE)

Variable_size_type Variable_vector::size()
//******************************************************************************
// LAST MODIFIED : 24 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (values.size());
}

Vector *Variable_vector::scalars()
//******************************************************************************
// LAST MODIFIED : 24 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (new Vector(values));
}

Variable_input_handle Variable_vector::input_values()
//******************************************************************************
// LAST MODIFIED : 23 October 2003
//
// DESCRIPTION :
// Returns the values input for a vector.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_vector_values(
		Variable_vector_handle(this))));
}

Variable_input_handle Variable_vector::input_values(Variable_size_type index)
//******************************************************************************
// LAST MODIFIED : 24 October 2003
//
// DESCRIPTION :
// Returns the values input for a vector.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_vector_values(
		Variable_vector_handle(this),index)));
}

Variable_input_handle Variable_vector::input_values(
	const boost::numeric::ublas::vector<Variable_size_type> indices)
//******************************************************************************
// LAST MODIFIED : 24 October 2003
//
// DESCRIPTION :
// Returns the values input for a vector.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_vector_values(
		Variable_vector_handle(this),indices)));
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

void Variable_vector::evaluate_derivative_local(Matrix& matrix,
	std::list<Variable_input_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 5 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_size_type i,index,number_of_input_values,number_of_values;
	Variable_input_vector_values_handle input_values_handle;

	// matrix is zero'd on entry
	if ((1==independent_variables.size())&&(input_values_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_vector_values,Variable_input>(
		independent_variables.front())
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
}

Variable_handle Variable_vector::get_input_value_local(
	const Variable_input_handle& input)
//******************************************************************************
// LAST MODIFIED : 5 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_vector_handle values_vector;
	Variable_input_vector_values_handle input_values_handle;

	if ((input_values_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_vector_values,Variable_input>(
		input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_vector_values *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_values_handle->variable_vector==Variable_vector_handle(this)))
	{
		Variable_size_type number_of_input_values=input_values_handle->size();

		if (0==(input_values_handle->indices).size())
		{
			values_vector=Variable_vector_handle(new Variable_vector(values));
		}
		else
		{
			Variable_size_type i,index,number_of_values=this->size();
			boost::numeric::ublas::vector<Scalar>
				selected_values(number_of_input_values);

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

int Variable_vector::set_input_value_local(const Variable_input_handle& input,
	const Variable_handle& values)
//******************************************************************************
// LAST MODIFIED : 5 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	int return_code;
	Variable_input_vector_values_handle input_values_handle;
	Vector *values_vector;

	return_code=0;
	if ((input_values_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_vector_values,Variable_input>(
		input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_vector_values *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_values_handle->variable_vector==Variable_vector_handle(this))&&
		(values_vector=values->scalars()))
	{
		Variable_size_type number_of_input_values=
			(input_values_handle->indices).size();

		if (0==number_of_input_values)
		{
			if ((this->values).size()==values_vector->size())
			{
				this->values= *values_vector;
				return_code=1;
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
				return_code=1;
			}
		}
		delete values_vector;
	}

	return (return_code);
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
