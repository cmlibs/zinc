//******************************************************************************
// FILE : variable_matrix.cpp
//
// LAST MODIFIED : 26 November 2003
//
// DESCRIPTION :
//???DB.  Should be template?
//==============================================================================

#include <new>
#include <sstream>
#include <string>

//???DB.  Put in include?
const bool Assert_on=true;

template<class Assertion,class Exception>inline void Assert(
	Assertion assertion,Exception exception)
{
	if (Assert_on&&!(assertion)) throw exception;
}

#include "computed_variable/variable_matrix.hpp"
#include "computed_variable/variable_vector.hpp"

// module classes
// ==============

// class Variable_input_matrix_values
// ----------------------------------

class Variable_input_matrix_values : public Variable_input
//******************************************************************************
// LAST MODIFIED : 26 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	friend class Variable_matrix;
	public:
		Variable_input_matrix_values(const Variable_matrix_handle& variable_matrix):
			variable_matrix(variable_matrix),indices(){};
		Variable_input_matrix_values(const Variable_matrix_handle& variable_matrix,
			Variable_size_type row,Variable_size_type column):variable_matrix(
			variable_matrix),indices(1)
		{
			indices[0].first=row;
			indices[0].second=column;
		};
		Variable_input_matrix_values(const Variable_matrix_handle& variable_matrix,
			const boost::numeric::ublas::vector<
			std::pair<Variable_size_type,Variable_size_type> >& indices):
			variable_matrix(variable_matrix),indices(indices){};
		~Variable_input_matrix_values(){};
		Variable_size_type size()
		{
			Variable_size_type result;

			result=indices.size();
			if (0==result)
			{
				result=variable_matrix->size();
			}

			return (result);
		};
		virtual bool operator==(const Variable_input& input)
		{
			try
			{
				const Variable_input_matrix_values& input_matrix_values=
					dynamic_cast<const Variable_input_matrix_values&>(input);
				bool result;

				result=false;
				if ((variable_matrix==input_matrix_values.variable_matrix)&&
					(indices.size()==input_matrix_values.indices.size()))
				{
					int i=indices.size();

					result=true;
					while (result&&(i>0))
					{
						i--;
						if (!(indices[i]==input_matrix_values.indices[i]))
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
		};
	private:
		Variable_matrix_handle variable_matrix;
		boost::numeric::ublas::vector<
			std::pair<Variable_size_type,Variable_size_type> > indices;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_matrix_values>
	Variable_input_matrix_values_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_matrix_values>
	Variable_input_matrix_values_handle;
#else
typedef Variable_input_matrix_values * Variable_input_matrix_values_handle;
#endif

// global classes
// ==============

// class Variable_matrix
// ---------------------

Variable_matrix::Variable_matrix(Matrix& values):Variable(),values(values)
//******************************************************************************
// LAST MODIFIED : 23 October 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
}

Variable_matrix::Variable_matrix(const Variable_matrix& variable_matrix):
	Variable(),values(variable_matrix.values)
//******************************************************************************
// LAST MODIFIED : 23 October 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Variable_matrix& Variable_matrix::operator=(
	const Variable_matrix& variable_matrix)
//******************************************************************************
// LAST MODIFIED : 23 October 2003
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	this->values=variable_matrix.values;

	return (*this);
}

Variable_matrix::~Variable_matrix()
//******************************************************************************
// LAST MODIFIED : 23 October 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

Variable_size_type Variable_matrix::size()
//******************************************************************************
// LAST MODIFIED : 24 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	return ((values.size1())*(values.size2()));
}

Vector *Variable_matrix::scalars()
//******************************************************************************
// LAST MODIFIED : 24 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	Vector *values_vector;

	if (values_vector=new Vector(size()))
	{
		Variable_size_type i,j,k,number_of_columns=values.size2(),
			number_of_rows=values.size1();

		k=0;
		for (i=0;i<number_of_rows;i++)
		{
			for (j=0;j<number_of_columns;j++)
			{
				(*values_vector)[k]=values(i,j);
				k++;
			}
		}
	}

	return (values_vector);
}

Variable_matrix_handle Variable_matrix::sub_matrix(Variable_size_type row_low,
	Variable_size_type row_high,Variable_size_type column_low,
	Variable_size_type column_high)
//******************************************************************************
// LAST MODIFIED : 6 November 2003
//
// DESCRIPTION :
// Returns the specified sub-matrix.
//==============================================================================
{
	Variable_matrix_handle result;

	if ((0<row_low)&&(row_low<=row_high)&&(row_high<=values.size1())&&
		(0<column_low)&&(column_low<=column_high)&&(column_high<=values.size2()))
	{
		Variable_size_type i,j,number_of_columns=column_high-column_low+1,
			number_of_rows=row_high-row_low+1;
		Matrix temp_matrix(number_of_rows,number_of_columns);

		for (i=0;i<number_of_rows;i++)
		{
			for (j=0;j<number_of_columns;j++)
			{
				temp_matrix(i,j)=values(i+row_low-1,j+column_low-1);
			}
		}
		result=Variable_matrix_handle(new Variable_matrix(temp_matrix));
	}
	else
	{
		Matrix temp_matrix(0,0);

		result=Variable_matrix_handle(new Variable_matrix(temp_matrix));
	}

	return (result);
}

Variable_size_type Variable_matrix::number_of_rows()
//******************************************************************************
// LAST MODIFIED : 6 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (values.size1());
}

Variable_size_type Variable_matrix::number_of_columns()
//******************************************************************************
// LAST MODIFIED : 6 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (values.size2());
}

Variable_input_handle Variable_matrix::input_values()
//******************************************************************************
// LAST MODIFIED : 23 October 2003
//
// DESCRIPTION :
// Returns the values input for a matrix.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_matrix_values(
		Variable_matrix_handle(this))));
}

Variable_input_handle Variable_matrix::input_values(Variable_size_type row,
	Variable_size_type column)
//******************************************************************************
// LAST MODIFIED : 24 October 2003
//
// DESCRIPTION :
// Returns the values input for a matrix.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_matrix_values(
		Variable_matrix_handle(this),row,column)));
}

Variable_input_handle Variable_matrix::input_values(
	const boost::numeric::ublas::vector<
	std::pair<Variable_size_type,Variable_size_type> > indices)
//******************************************************************************
// LAST MODIFIED : 23 October 2003
//
// DESCRIPTION :
// Returns the values input for a matrix.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_matrix_values(
		Variable_matrix_handle(this),indices)));
}

Variable_handle Variable_matrix::evaluate_local()
//******************************************************************************
// LAST MODIFIED : 23 October 2003
//
// DESCRIPTION :
// Duplicate <this> so that <this> isn't changed by operations on the result.
//
//???DB.  Should this turn the matrix into a vector?  No this is done by scalars
//==============================================================================
{
	return (Variable_handle(new Variable_matrix(*this)));
}

void Variable_matrix::evaluate_derivative_local(Matrix& matrix,
	std::list<Variable_input_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 5 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_size_type i,j,k,number_of_values;
	Variable_input_matrix_values_handle input_values_handle;

	// matrix is zero'd on entry
	if ((1==independent_variables.size())&&(input_values_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_matrix_values,Variable_input>(
		independent_variables.front())
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_matrix_values *>(independent_variables.front())
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_values_handle->variable_matrix==Variable_handle(this)))
	{
		Assert((this->size()==matrix.size1())&&
			(input_values_handle->size()==matrix.size2()),std::logic_error(
			"Variable_matrix::evaluate_derivative_local.  "
			"Incorrect matrix size"));
		number_of_values=(input_values_handle->indices).size();
		if (0==number_of_values)
		{
			number_of_values=this->size();
			for (i=0;i<number_of_values;i++)
			{
				matrix(i,i)=1;
			}
		}
		else
		{
			Variable_size_type number_of_columns=values.size2(),
				number_of_rows=values.size1();

			for (k=0;k<number_of_values;k++)
			{
				i=(input_values_handle->indices)[k].first;
				j=(input_values_handle->indices)[k].second;
				if ((0<i)&&(i<=number_of_rows)&&(0<j)&&(j<=number_of_columns))
				{
					matrix((i-1)*number_of_columns+(j-1),k)=1;
				}
			}
		}
	}
}

Variable_handle Variable_matrix::get_input_value_local(
	const Variable_input_handle& input)
//******************************************************************************
// LAST MODIFIED : 5 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_vector_handle values_vector;
	Variable_input_matrix_values_handle input_values_handle;

	if ((input_values_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_matrix_values,Variable_input>(
		input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_matrix_values *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_values_handle->variable_matrix==Variable_matrix_handle(this)))
	{
		if (0==(input_values_handle->indices).size())
		{
			Variable_size_type i,j,k,number_of_columns=values.size2(),
				number_of_rows=values.size1();
			Vector extracted_values(number_of_rows*number_of_columns);

			k=0;
			for (i=0;i<number_of_rows;i++)
			{
				for (j=0;j<number_of_columns;j++)
				{
					extracted_values[k]=values(i,j);
					k++;
				}
			}
			values_vector=Variable_vector_handle(new Variable_vector(
				extracted_values));
		}
		else
		{
			Variable_size_type i,j,k,number_of_columns=values.size2(),
				number_of_rows=values.size1(),
				number_of_selected_values=input_values_handle->size();
			Vector selected_values(number_of_selected_values);

			for (k=0;k<number_of_selected_values;k++)
			{
				i=(input_values_handle->indices)[k].first;
				j=(input_values_handle->indices)[k].second;
				if ((0<i)&&(i<=number_of_rows)&&(0<j)&&(j<=number_of_columns))
				{
					selected_values[k]=values(i-1,j-1);
				}
				else
				{
					selected_values[k]=(Scalar)0;
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

int Variable_matrix::set_input_value_local(const Variable_input_handle& input,
	const Variable_handle& values)
//******************************************************************************
// LAST MODIFIED : 5 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	int return_code;
	Variable_input_matrix_values_handle input_values_handle;
	Vector *values_vector;

	return_code=0;
	if ((input_values_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_matrix_values,Variable_input>(
		input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_matrix_values *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_values_handle->variable_matrix==Variable_matrix_handle(this))&&
		(values_vector=values->scalars()))
	{
		Variable_size_type number_of_specified_values=
			(input_values_handle->indices).size();

		if (0==number_of_specified_values)
		{
			if (this->size()==values_vector->size())
			{
				Variable_size_type i,j,k,number_of_columns=(this->values).size2(),
					number_of_rows=(this->values).size1();

				k=0;
				for (i=0;i<number_of_rows;i++)
				{
					for (j=0;j<number_of_columns;j++)
					{
						(this->values)(i,j)=(*values_vector)[k];
						k++;
					}
				}
				return_code=1;
			}
		}
		else
		{
			if (number_of_specified_values==values_vector->size())
			{
				Variable_size_type i,j,k,number_of_columns=(this->values).size2(),
					number_of_rows=(this->values).size1();

				for (k=0;k<number_of_specified_values;k++)
				{
					i=(input_values_handle->indices)[k].first;
					j=(input_values_handle->indices)[k].second;
					if ((0<i)&&(i<=number_of_rows)&&(0<j)&&(j<=number_of_columns))
					{
						(this->values)(i-1,j-1)=(*values_vector)[k];
					}
				}
				return_code=1;
			}
		}
		delete values_vector;
	}

	return (return_code);
}

string_handle Variable_matrix::get_string_representation_local()
//******************************************************************************
// LAST MODIFIED : 23 October 2003
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
