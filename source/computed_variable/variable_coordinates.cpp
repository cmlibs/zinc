//******************************************************************************
// FILE : variable_coordinates.cpp
//
// LAST MODIFIED : 26 November 2003
//
// DESCRIPTION :
// Implements variables which transform between coordinate systems.
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

#include "computed_variable/variable_coordinates.hpp"
#include "computed_variable/variable_vector.hpp"

// module classes
// ==============

// class Variable_input_prolate_spheroidal_to_rectangular_cartesian
// ----------------------------------------------------------------

class Variable_input_prolate_spheroidal_to_rectangular_cartesian :
	public Variable_input
//******************************************************************************
// LAST MODIFIED : 26 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	friend class Variable_prolate_spheroidal_to_rectangular_cartesian;
	public:
		Variable_input_prolate_spheroidal_to_rectangular_cartesian(
			const Variable_prolate_spheroidal_to_rectangular_cartesian_handle&
			variable_prolate_spheroidal_to_rectangular_cartesian,bool lambda=true,
			bool mu=true,bool theta=true,bool focus=false):focus(focus),
			lambda(lambda),mu(mu),theta(theta),
			variable_prolate_spheroidal_to_rectangular_cartesian(
			variable_prolate_spheroidal_to_rectangular_cartesian) {};
		~Variable_input_prolate_spheroidal_to_rectangular_cartesian() {};
		Variable_size_type size()
		{
			Variable_size_type result;

			result=0;
			if (lambda)
			{
				result++;
			}
			if (mu)
			{
				result++;
			}
			if (theta)
			{
				result++;
			}
			if (focus)
			{
				result++;
			}

			return (result);
		};
		virtual bool operator==(const Variable_input& input)
		{
			try
			{
				const Variable_input_prolate_spheroidal_to_rectangular_cartesian&
					input_prolate_spheroidal_to_rectangular_cartesian=dynamic_cast<
					const Variable_input_prolate_spheroidal_to_rectangular_cartesian&>(
					input);

				return ((variable_prolate_spheroidal_to_rectangular_cartesian==
					input_prolate_spheroidal_to_rectangular_cartesian.
					variable_prolate_spheroidal_to_rectangular_cartesian)&&
					(lambda==input_prolate_spheroidal_to_rectangular_cartesian.lambda)&&
					(mu==input_prolate_spheroidal_to_rectangular_cartesian.mu)&&
					(theta==input_prolate_spheroidal_to_rectangular_cartesian.theta)&&
					(focus==input_prolate_spheroidal_to_rectangular_cartesian.focus));
			}
			catch (std::bad_cast)
			{
				return (false);
			}
		};
	private:
		bool focus,lambda,mu,theta;
		Variable_prolate_spheroidal_to_rectangular_cartesian_handle
			variable_prolate_spheroidal_to_rectangular_cartesian;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<
	Variable_input_prolate_spheroidal_to_rectangular_cartesian>
	Variable_input_prolate_spheroidal_to_rectangular_cartesian_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<
	Variable_input_prolate_spheroidal_to_rectangular_cartesian>
	Variable_input_prolate_spheroidal_to_rectangular_cartesian_handle;
#else
typedef Variable_input_prolate_spheroidal_to_rectangular_cartesian *
	Variable_input_prolate_spheroidal_to_rectangular_cartesian_handle;
#endif

// global classes
// ==============

// class Variable_vector
// ---------------------

Variable_prolate_spheroidal_to_rectangular_cartesian::
	Variable_prolate_spheroidal_to_rectangular_cartesian(const Scalar lambda,
	const Scalar mu,const Scalar theta,const Scalar focus):
	Variable(),focus(focus),lambda(lambda),mu(mu),theta(theta) {}
//******************************************************************************
// LAST MODIFIED : 20 November 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================

Variable_prolate_spheroidal_to_rectangular_cartesian::
	Variable_prolate_spheroidal_to_rectangular_cartesian(
	const Variable_prolate_spheroidal_to_rectangular_cartesian& variable):
	Variable(),focus(variable.focus),lambda(variable.lambda),mu(variable.mu),
	theta(variable.theta) {}
//******************************************************************************
// LAST MODIFIED : 20 November 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================

Variable_prolate_spheroidal_to_rectangular_cartesian&
	Variable_prolate_spheroidal_to_rectangular_cartesian::operator=(
	const Variable_prolate_spheroidal_to_rectangular_cartesian& variable)
//******************************************************************************
// LAST MODIFIED : 20 November 2003
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	this->lambda=variable.lambda;
	this->mu=variable.mu;
	this->theta=variable.theta;
	this->focus=variable.focus;

	return (*this);
}

Variable_prolate_spheroidal_to_rectangular_cartesian::
	~Variable_prolate_spheroidal_to_rectangular_cartesian()
//******************************************************************************
// LAST MODIFIED : 20 November 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

Variable_size_type Variable_prolate_spheroidal_to_rectangular_cartesian::size()
//******************************************************************************
// LAST MODIFIED : 20 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (3);
}

Vector *Variable_prolate_spheroidal_to_rectangular_cartesian::scalars()
//******************************************************************************
// LAST MODIFIED : 20 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (evaluate_local()->scalars());
}

Variable_input_handle Variable_prolate_spheroidal_to_rectangular_cartesian::
	input_prolate()
//******************************************************************************
// LAST MODIFIED : 20 November 2003
//
// DESCRIPTION :
// Returns the prolate input.
//==============================================================================
{
	return (Variable_input_handle(
		new Variable_input_prolate_spheroidal_to_rectangular_cartesian(
		Variable_prolate_spheroidal_to_rectangular_cartesian_handle(this),
		true,true,true,false)));
}

Variable_input_handle Variable_prolate_spheroidal_to_rectangular_cartesian::
	input_lambda()
//******************************************************************************
// LAST MODIFIED : 20 November 2003
//
// DESCRIPTION :
// Returns the lambda input.
//==============================================================================
{
	return (Variable_input_handle(
		new Variable_input_prolate_spheroidal_to_rectangular_cartesian(
		Variable_prolate_spheroidal_to_rectangular_cartesian_handle(this),
		true,false,false,false)));
}

Variable_input_handle Variable_prolate_spheroidal_to_rectangular_cartesian::
	input_mu()
//******************************************************************************
// LAST MODIFIED : 20 November 2003
//
// DESCRIPTION :
// Returns the mu input.
//==============================================================================
{
	return (Variable_input_handle(
		new Variable_input_prolate_spheroidal_to_rectangular_cartesian(
		Variable_prolate_spheroidal_to_rectangular_cartesian_handle(this),
		false,true,false,false)));
}

Variable_input_handle Variable_prolate_spheroidal_to_rectangular_cartesian::
	input_theta()
//******************************************************************************
// LAST MODIFIED : 20 November 2003
//
// DESCRIPTION :
// Returns the theta input.
//==============================================================================
{
	return (Variable_input_handle(
		new Variable_input_prolate_spheroidal_to_rectangular_cartesian(
		Variable_prolate_spheroidal_to_rectangular_cartesian_handle(this),
		false,false,true,false)));
}

Variable_input_handle Variable_prolate_spheroidal_to_rectangular_cartesian::
	input_focus()
//******************************************************************************
// LAST MODIFIED : 20 November 2003
//
// DESCRIPTION :
// Returns the focus input.
//==============================================================================
{
	return (Variable_input_handle(
		new Variable_input_prolate_spheroidal_to_rectangular_cartesian(
		Variable_prolate_spheroidal_to_rectangular_cartesian_handle(this),
		false,false,false,true)));
}

Variable_handle Variable_prolate_spheroidal_to_rectangular_cartesian::
	evaluate_local()
//******************************************************************************
// LAST MODIFIED : 20 November 2003
//
// DESCRIPTION :
// Duplicate <this> so that <this> isn't changed by operations on the result.
//==============================================================================
{
	double a1,a2,a3,a4,a5,a6,a7;
	Vector rectangular_cartesian(3);

	a1=(double)focus*sinh((double)lambda);
	a2=(double)focus*cosh((double)lambda);
	a3=sin((double)mu);
	a4=cos((double)mu);
	a5=sin((double)theta);
	a6=cos((double)theta);
	a7=a1*a3;
	rectangular_cartesian[0]=(Scalar)(a2*a4);
	rectangular_cartesian[1]=(Scalar)(a7*a6);
	rectangular_cartesian[2]=(Scalar)(a7*a5);

	return (Variable_handle(new Variable_vector(rectangular_cartesian)));
}

void Variable_prolate_spheroidal_to_rectangular_cartesian::
	evaluate_derivative_local(Matrix& matrix,
	std::list<Variable_input_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 21 November 2003
//
// DESCRIPTION :
// ???DB.  Currently up to and including order 2 derivatives in prolate
//==============================================================================
{
	bool zero_derivative;
	std::list<Variable_input_handle>::iterator independent_variable_iterator;
	Variable_input_prolate_spheroidal_to_rectangular_cartesian_handle
		input_prolate_spheroidal_to_rectangular_cartesian_handle;
	Variable_size_type i,j,k;

	Assert(3==matrix.size1(),std::logic_error(
		"Variable_prolate_spheroidal_to_rectangular_cartesian::"
		"evaluate_derivative_local.  Incorrect matrix size 1"));
	// check for zero derivative
	zero_derivative=false;
	independent_variable_iterator=independent_variables.begin();
	i=independent_variables.size();
	while (!zero_derivative&&(i>0))
	{
		zero_derivative=!((input_prolate_spheroidal_to_rectangular_cartesian_handle=
#if defined (USE_SMART_POINTER)
			boost::dynamic_pointer_cast<
			Variable_input_prolate_spheroidal_to_rectangular_cartesian,
			Variable_input>(*independent_variable_iterator)
#else /* defined (USE_SMART_POINTER) */
			dynamic_cast<
			Variable_input_prolate_spheroidal_to_rectangular_cartesian *>(
			*independent_variable_iterator)
#endif /* defined (USE_SMART_POINTER) */
			)&&(input_prolate_spheroidal_to_rectangular_cartesian_handle->
			variable_prolate_spheroidal_to_rectangular_cartesian==
			Variable_handle(this)));
		independent_variable_iterator++;
		i--;
	}
	if (!zero_derivative)
	{
		double aa1,aa2,a1,a2,a3,a4,a5,a6,a7,a8,a9;
		Variable_size_type input_size,matrix_number_of_columns=matrix.size2(),
			number_of_columns;
		boost::numeric::ublas::matrix<Variable_size_type>
			derivative_indices(matrix_number_of_columns,4);
		boost::numeric::ublas::vector<Variable_size_type> derivative_mapping(4);

		independent_variable_iterator=independent_variables.begin();
		number_of_columns=0;
		for (i=independent_variables.size();i>0;i--)
		{
			input_prolate_spheroidal_to_rectangular_cartesian_handle=
#if defined (USE_SMART_POINTER)
				boost::dynamic_pointer_cast<
				Variable_input_prolate_spheroidal_to_rectangular_cartesian,
				Variable_input>(*independent_variable_iterator)
#else /* defined (USE_SMART_POINTER) */
				dynamic_cast<
				Variable_input_prolate_spheroidal_to_rectangular_cartesian *>(
				*independent_variable_iterator)
#endif /* defined (USE_SMART_POINTER) */
			;
			input_size=
				input_prolate_spheroidal_to_rectangular_cartesian_handle->size();
			// set up derivative mapping
			j=0;
			if (input_prolate_spheroidal_to_rectangular_cartesian_handle->lambda)
			{
				derivative_mapping[j]=0;
				j++;
			}
			if (input_prolate_spheroidal_to_rectangular_cartesian_handle->mu)
			{
				derivative_mapping[j]=1;
				j++;
			}
			if (input_prolate_spheroidal_to_rectangular_cartesian_handle->theta)
			{
				derivative_mapping[j]=2;
				j++;
			}
			if (input_prolate_spheroidal_to_rectangular_cartesian_handle->focus)
			{
				derivative_mapping[j]=3;
				j++;
			}
			if (0==number_of_columns)
			{
				Assert((input_size<=matrix_number_of_columns),std::logic_error(
					"Variable_prolate_spheroidal_to_rectangular_cartesian::"
					"evaluate_derivative_local.  Incorrect matrix size 2"));
				for (j=0;j<input_size;j++)
				{
					derivative_indices(j,0)=0;
					derivative_indices(j,1)=0;
					derivative_indices(j,2)=0;
					derivative_indices(j,3)=0;
					derivative_indices(j,derivative_mapping[j])=1;
				}
				number_of_columns=input_size;
			}
			else
			{
				Assert((input_size*number_of_columns<=matrix_number_of_columns),
					std::logic_error(
					"Variable_prolate_spheroidal_to_rectangular_cartesian::"
					"evaluate_derivative_local.  Incorrect matrix size 3"));
				for (j=0;j<number_of_columns;j++)
				{
					for (k=1;k<input_size;k++)
					{
						derivative_indices(j+k*number_of_columns,0)=derivative_indices(j,0);
						derivative_indices(j+k*number_of_columns,1)=derivative_indices(j,1);
						derivative_indices(j+k*number_of_columns,2)=derivative_indices(j,2);
						derivative_indices(j+k*number_of_columns,3)=derivative_indices(j,3);
						derivative_indices(j+k*number_of_columns,derivative_mapping[k])++;
					}
					derivative_indices(j,derivative_mapping[0])++;
				}
				number_of_columns *= input_size;
			}
			independent_variable_iterator++;
		}
		Assert(number_of_columns==matrix_number_of_columns,std::logic_error(
			"Variable_prolate_spheroidal_to_rectangular_cartesian::"
			"evaluate_derivative_local.  Incorrect matrix size 4"));
		// assign derivatives
		aa1=sinh(lambda);
		aa2=cosh(lambda);
		a3=sin(mu);
		a4=cos(mu);
		a5=sin(theta);
		a6=cos(theta);
		for (j=0;j<matrix_number_of_columns;j++)
		{
			// focus order
			if (derivative_indices(j,3)<=1)
			{
				if (0==derivative_indices(j,3))
				{
					a1=focus*aa1;
					a2=focus*aa2;
				}
				else
				{
					a1=aa1;
					a2=aa2;
				}
				a7=a1*a3;
				a8=a1*a4;
				a9=a2*a3;
				// lambda order
				switch (derivative_indices(j,0))
				{
					case 0:
					{
						// mu order
						switch (derivative_indices(j,1))
						{
							case 0:
							{
								// theta order
								switch (derivative_indices(j,2))
								{
									case 0:
									{
										matrix(0,j)=a2*a4;
										matrix(1,j)=a7*a6;
										matrix(2,j)=a7*a5;
									} break;
									case 1:
									{
										matrix(0,j)=0;
										matrix(1,j)= -a7*a5;
										matrix(2,j)=a7*a6;
									} break;
									case 2:
									{
										matrix(0,j)=0;
										matrix(1,j)= -a7*a6;
										matrix(2,j)= -a7*a5;
									} break;
								}
							} break;
							case 1:
							{
								// theta order
								switch (derivative_indices(j,2))
								{
									case 0:
									{
										matrix(0,j)= -a9;
										matrix(1,j)=a8*a6;
										matrix(2,j)=a8*a5;
									} break;
									case 1:
									{
										matrix(0,j)=0;
										matrix(1,j)= -a8*a5;
										matrix(2,j)=a8*a6;
									} break;
								}
							} break;
							case 2:
							{
								// theta order
								switch (derivative_indices(j,2))
								{
									case 0:
									{
										matrix(0,j)= -a2*a4;
										matrix(1,j)= -a7*a6;
										matrix(2,j)= -a7*a5;
									} break;
								}
							} break;
						}
					} break;
					case 1:
					{
						// mu order
						switch (derivative_indices(j,1))
						{
							case 0:
							{
								// theta order
								switch (derivative_indices(j,2))
								{
									case 0:
									{
										matrix(0,j)=a8;
										matrix(1,j)=a9*a6;
										matrix(2,j)=a9*a5;
									} break;
									case 1:
									{
										matrix(0,j)=0;
										matrix(1,j)= -a9*a5;
										matrix(2,j)=a9*a6;
									} break;
								}
							} break;
							case 1:
							{
								// theta order
								switch (derivative_indices(j,2))
								{
									case 0:
									{
										matrix(0,j)= -a1*a3;
										matrix(1,j)=a2*a4*a6;
										matrix(2,j)=a2*a4*a5;
									} break;
								}
							} break;
						}
					} break;
					case 2:
					{
						// mu order
						switch (derivative_indices(j,1))
						{
							case 0:
							{
								// theta order
								switch (derivative_indices(j,2))
								{
									case 0:
									{
										matrix(0,j)=a2*a4;
										matrix(1,j)=a7*a6;
										matrix(2,j)=a7*a5;
									} break;
								}
							} break;
						}
					} break;
				}
			}
		}
	}
}

Variable_handle Variable_prolate_spheroidal_to_rectangular_cartesian::
	get_input_value_local(const Variable_input_handle& input)
//******************************************************************************
// LAST MODIFIED : 20 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_vector_handle values_vector;
	Variable_input_prolate_spheroidal_to_rectangular_cartesian_handle
		input_prolate_spheroidal_to_rectangular_cartesian_handle;

	if ((input_prolate_spheroidal_to_rectangular_cartesian_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<
		Variable_input_prolate_spheroidal_to_rectangular_cartesian,
		Variable_input>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<
		Variable_input_prolate_spheroidal_to_rectangular_cartesian *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_prolate_spheroidal_to_rectangular_cartesian_handle->
		variable_prolate_spheroidal_to_rectangular_cartesian==
		Variable_handle(this)))
	{
		Variable_size_type j,number_of_input_values=
			input_prolate_spheroidal_to_rectangular_cartesian_handle->size();

		values_vector=Variable_vector_handle(new Variable_vector(
			Vector(number_of_input_values)));
		j=0;
		if (input_prolate_spheroidal_to_rectangular_cartesian_handle->lambda)
		{
			(*values_vector)[j]=lambda;
			j++;
		}
		if (input_prolate_spheroidal_to_rectangular_cartesian_handle->mu)
		{
			(*values_vector)[j]=mu;
			j++;
		}
		if (input_prolate_spheroidal_to_rectangular_cartesian_handle->theta)
		{
			(*values_vector)[j]=theta;
			j++;
		}
		if (input_prolate_spheroidal_to_rectangular_cartesian_handle->focus)
		{
			(*values_vector)[j]=focus;
			j++;
		}
	}
	else
	{
		values_vector=Variable_vector_handle((Variable_vector *)0);
	}

	return (values_vector);
}

int Variable_prolate_spheroidal_to_rectangular_cartesian::set_input_value_local(
	const Variable_input_handle& input,const Variable_handle& values)
//******************************************************************************
// LAST MODIFIED : 20 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	int return_code;
	Variable_input_prolate_spheroidal_to_rectangular_cartesian_handle
		input_prolate_spheroidal_to_rectangular_cartesian_handle;
	Vector *values_vector;

	return_code=0;
	if ((input_prolate_spheroidal_to_rectangular_cartesian_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<
		Variable_input_prolate_spheroidal_to_rectangular_cartesian,
		Variable_input>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<
		Variable_input_prolate_spheroidal_to_rectangular_cartesian *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_prolate_spheroidal_to_rectangular_cartesian_handle->
		variable_prolate_spheroidal_to_rectangular_cartesian==
		Variable_handle(this))&&(values_vector=values->scalars()))
	{
		Variable_size_type j,number_of_input_values=
			input_prolate_spheroidal_to_rectangular_cartesian_handle->size();

		if (values_vector->size()==number_of_input_values)
		{
			j=0;
			if (input_prolate_spheroidal_to_rectangular_cartesian_handle->lambda)
			{
				lambda=(*values_vector)[j];
				j++;
			}
			if (input_prolate_spheroidal_to_rectangular_cartesian_handle->mu)
			{
				mu=(*values_vector)[j];
				j++;
			}
			if (input_prolate_spheroidal_to_rectangular_cartesian_handle->theta)
			{
				theta=(*values_vector)[j];
				j++;
			}
			if (input_prolate_spheroidal_to_rectangular_cartesian_handle->focus)
			{
				focus=(*values_vector)[j];
				j++;
			}
		}
		delete values_vector;
	}

	return (return_code);
}

string_handle Variable_prolate_spheroidal_to_rectangular_cartesian::
	get_string_representation_local()
//******************************************************************************
// LAST MODIFIED : 20 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		out << "prolate to rectangular cartesian";
		*return_string=out.str();
	}

	return (return_string);
}
