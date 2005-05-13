//******************************************************************************
// FILE : function_matrix_divide_by_scalar_implementation.cpp
//
// LAST MODIFIED : 20 April 2005
//
// DESCRIPTION :
//==============================================================================

#include <sstream>

#include "computed_variable/function_matrix_divide_by_scalar.hpp"
#include "computed_variable/function_variable.hpp"
#include "computed_variable/function_variable_matrix.hpp"
#include "computed_variable/function_variable_union.hpp"
#include "computed_variable/function_variable_value_specific.hpp"


// module classes
// ==============

// class Function_variable_matrix_divide_by_scalar
// -----------------------------------------------

EXPORT template<typename Value_type>
class Function_variable_matrix_divide_by_scalar :
	public Function_variable_matrix<Value_type>
//******************************************************************************
// LAST MODIFIED : 20 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_matrix<Value_type>;
	public:
		// constructor
		Function_variable_matrix_divide_by_scalar(
			const boost::intrusive_ptr< Function_matrix_divide_by_scalar<Value_type> >
			function_matrix_divide_by_scalar):
			Function_variable_matrix<Value_type>(function_matrix_divide_by_scalar
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			,false
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			){};
		Function_variable_matrix_divide_by_scalar(
			const boost::intrusive_ptr< Function_matrix_divide_by_scalar<Value_type> >
			function_matrix_divide_by_scalar,const Function_size_type row,
			const Function_size_type column):Function_variable_matrix<Value_type>(
			function_matrix_divide_by_scalar,
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			false,
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			row,column){};
		~Function_variable_matrix_divide_by_scalar(){}
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_matrix_divide_by_scalar<Value_type>(*this)));
		};
#if defined (EVALUATE_RETURNS_VALUE)
		Function_handle evaluate()
		{
			Function_handle result(0);
			boost::intrusive_ptr< Function_matrix_divide_by_scalar<Value_type> >
				function_matrix_divide_by_scalar;

			if (function_matrix_divide_by_scalar=boost::dynamic_pointer_cast<
				Function_matrix_divide_by_scalar<Value_type>,Function>(this->function()))
			{
#if defined (BEFORE_CACHING)
				boost::intrusive_ptr< Function_matrix<Value_type> > dividend,divisor;
				Function_size_type number_of_columns,number_of_rows;
				Value_type divisor_value;

				if ((dividend=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
					Function>(function_matrix_divide_by_scalar->dividend_private->
					evaluate()))&&(divisor=boost::dynamic_pointer_cast<Function_matrix<
					Value_type>,Function>(function_matrix_divide_by_scalar->
					divisor_private->evaluate()))&&
					(row_private<=(number_of_rows=dividend->number_of_rows()))&&
					(column_private<=(number_of_columns=dividend->number_of_columns()))&&
					(1==divisor->number_of_rows())&&(1==divisor->number_of_columns())&&
					(0!=(divisor_value=(*divisor)(1,1))))
				{
					Function_size_type i,j;
					Value_type a;

					function_matrix_divide_by_scalar->values.resize(number_of_rows,
						number_of_columns);
					a=1/divisor_value;
					for (i=1;i<=number_of_rows;++i)
					{
						for (j=1;j<=number_of_columns;++j)
						{
							function_matrix_divide_by_scalar->values(i-1,j-1)=
								a*(*dividend)(i,j);
						}
					}
					if (0==row_private)
					{
						if (0==column_private)
						{
							result=Function_handle(new Function_matrix<Value_type>(
								function_matrix_divide_by_scalar->values));
						}
						else
						{
							ublas::matrix<Value_type,ublas::column_major>
								result_matrix(number_of_rows,1);

							for (i=0;i<number_of_rows;++i)
							{
								result_matrix(i,0)=(function_matrix_divide_by_scalar->values)(
									i,column_private-1);
							}
							result=Function_handle(new Function_matrix<Value_type>(
								result_matrix));
						}
					}
					else
					{
						if (0==column_private)
						{
							ublas::matrix<Value_type,ublas::column_major>
								result_matrix(1,number_of_columns);

							for (j=0;j<number_of_columns;++j)
							{
								result_matrix(0,j)=(function_matrix_divide_by_scalar->values)(
									row_private-1,j);
							}
							result=Function_handle(new Function_matrix<Value_type>(
								result_matrix));
						}
						else
						{
							ublas::matrix<Value_type,ublas::column_major>
								result_matrix(1,1);
							
							result_matrix(0,0)=(function_matrix_divide_by_scalar->values)(
								row_private-1,column_private-1);
							result=Function_handle(new Function_matrix<Value_type>(
								result_matrix));
						}
					}
				}
#else // defined (BEFORE_CACHING)
				if (!(function_matrix_divide_by_scalar->evaluated()))
				{
					boost::intrusive_ptr< Function_matrix<Value_type> > dividend,divisor;
					Function_size_type number_of_columns,number_of_rows;
					Value_type divisor_value;

					if ((dividend=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
						Function>(function_matrix_divide_by_scalar->dividend_private->
						evaluate()))&&(divisor=boost::dynamic_pointer_cast<Function_matrix<
						Value_type>,Function>(function_matrix_divide_by_scalar->
						divisor_private->evaluate()))&&
						(this->row_private<=(number_of_rows=dividend->number_of_rows()))&&
						(this->column_private<=
						(number_of_columns=dividend->number_of_columns()))&&
						(1==divisor->number_of_rows())&&(1==divisor->number_of_columns())&&
						(0!=(divisor_value=(*divisor)(1,1))))
					{
						Function_size_type i,j;
						Value_type a;

						function_matrix_divide_by_scalar->values.resize(number_of_rows,
							number_of_columns);
						a=1/divisor_value;
						for (i=1;i<=number_of_rows;++i)
						{
							for (j=1;j<=number_of_columns;++j)
							{
								function_matrix_divide_by_scalar->values(i-1,j-1)=
									a*(*dividend)(i,j);
							}
						}
						function_matrix_divide_by_scalar->set_evaluated();
					}
				}
				if (function_matrix_divide_by_scalar->evaluated())
				{
					result=this->get_value();
				}
#endif // defined (BEFORE_CACHING)
			}

			return (result);
		};
#else // defined (EVALUATE_RETURNS_VALUE)
		bool evaluate()
		{
			bool result(true);
			boost::intrusive_ptr< Function_matrix_divide_by_scalar<Value_type> >
				function_matrix_divide_by_scalar;

			if (function_matrix_divide_by_scalar=boost::dynamic_pointer_cast<
				Function_matrix_divide_by_scalar<Value_type>,Function>(function()))
			{
#if defined (BEFORE_CACHING)
				Function_size_type number_of_columns,number_of_rows,number_in_sum;
				boost::intrusive_ptr< Function_matrix<Value_type> > dividend,divisor;
				Value_type divisor_value;

				result=false;
				if ((function_matrix_divide_by_scalar->dividend_private->evaluate)()&&
					(dividend=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
					Function>(function_matrix_divide_by_scalar->dividend_private->
					get_value()))&&
					(function_matrix_divide_by_scalar->divisor_private->evaluate)()&&
					(divisor=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
					Function>(function_matrix_divide_by_scalar->divisor_private->
					get_value()))&&
					(row_private<=(number_of_rows=dividend->number_of_rows()))&&
					(column_private<=(number_of_columns=dividend->number_of_columns()))&&
					(1==divisor->number_of_rows())&&(1==divisor->number_of_columns())&&
					(0!=(divisor_value=(*divisor)(1,1))))
				{
					Function_size_type i,j;
					Value_type a;

					function_matrix_divide_by_scalar->values.resize(number_of_rows,
						number_of_columns);
					a=1/divisor_value;
					for (i=1;i<=number_of_rows;++i)
					{
						for (j=1;j<=number_of_columns;++j)
						{
							function_matrix_divide_by_scalar->values(i-1,j-1)=
								a*(*dividend)(i,j);
						}
					}
					result=true;
				}
#else // defined (BEFORE_CACHING)
				if (!(function_matrix_divide_by_scalar->evaluated()))
				{
					boost::intrusive_ptr< Function_matrix<Value_type> > dividend,divisor;
					Function_size_type number_of_columns,number_of_rows;
					Value_type divisor_value;

					result=false;
					if ((function_matrix_divide_by_scalar->dividend_private->evaluate)()&&
						(dividend=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
						Function>(function_matrix_divide_by_scalar->dividend_private->
						get_value()))&&
						(function_matrix_divide_by_scalar->divisor_private->evaluate)()&&
						(divisor=boost::dynamic_pointer_cast<Function_matrix<
						Value_type>,Function>(function_matrix_divide_by_scalar->
						divisor_private->get_value()))&&
						(row_private<=(number_of_rows=dividend->number_of_rows()))&&
						(column_private<=
						(number_of_columns=dividend->number_of_columns()))&&
						(1==divisor->number_of_rows())&&(1==divisor->number_of_columns())&&
						(0!=(divisor_value=(*divisor)(1,1))))
					{
						Function_size_type i,j;
						Value_type a;

						function_matrix_divide_by_scalar->values.resize(number_of_rows,
							number_of_columns);
						a=1/divisor_value;
						for (i=1;i<=number_of_rows;++i)
						{
							for (j=1;j<=number_of_columns;++j)
							{
								function_matrix_divide_by_scalar->values(i-1,j-1)=
									a*(*dividend)(i,j);
							}
						}
						function_matrix_divide_by_scalar->set_evaluated();
						result=true;
					}
				}
#endif // defined (BEFORE_CACHING)
			}

			return (result);
		};
#endif // defined (EVALUATE_RETURNS_VALUE)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_handle evaluate_derivative(std::list<Function_variable_handle>&);
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		virtual Function_handle derivative(
			const std::list<Function_variable_handle>&);
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		//???DB.  Should operator() and get_entry do an evaluate?
		boost::intrusive_ptr< Function_variable_matrix<Value_type> > operator()(
			Function_size_type row=1,Function_size_type column=1) const
		{
			boost::intrusive_ptr< Function_matrix_divide_by_scalar<Value_type> >
				function_matrix_divide_by_scalar;
			boost::intrusive_ptr< Function_variable_matrix<Value_type> > result(0);

			if ((function_matrix_divide_by_scalar=boost::dynamic_pointer_cast<
				Function_matrix_divide_by_scalar<Value_type>,Function>(
				this->function_private))&&
				(row<=this->number_of_rows())&&(column<=this->number_of_columns()))
			{
				result=boost::intrusive_ptr< Function_variable_matrix<Value_type> >(
					new Function_variable_matrix_divide_by_scalar<Value_type>(
					function_matrix_divide_by_scalar,row,column));
			}

			return (result);
		}
	private:
		// copy constructor
		Function_variable_matrix_divide_by_scalar(
			const Function_variable_matrix_divide_by_scalar<Value_type>& variable):
			Function_variable_matrix<Value_type>(variable){};
};


// global classes
// ==============

// class Function_matrix_divide_by_scalar
// --------------------------------------

EXPORT template<typename Value_type>
ublas::matrix<Value_type,ublas::column_major> 
	Function_matrix_divide_by_scalar<Value_type>::constructor_values(0,0);

EXPORT template<typename Value_type>
Function_matrix_divide_by_scalar<Value_type>::Function_matrix_divide_by_scalar(
	const Function_variable_handle& dividend,
	const Function_variable_handle& divisor):Function_matrix<Value_type>(
	Function_matrix_divide_by_scalar<Value_type>::constructor_values),
	dividend_private(dividend),divisor_private(divisor)
//******************************************************************************
// LAST MODIFIED : 6 March 2005
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (divisor_private)
	{
		divisor_private->add_dependent_function(this);
	}
	if (dividend_private)
	{
		dividend_private->add_dependent_function(this);
	}
}

EXPORT template<typename Value_type>
Function_matrix_divide_by_scalar<Value_type>::
	~Function_matrix_divide_by_scalar()
//******************************************************************************
// LAST MODIFIED : 6 March 2005
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
#if defined (CIRCULAR_SMART_POINTERS)
	// do nothing
#else // defined (CIRCULAR_SMART_POINTERS)
	if (divisor_private)
	{
		divisor_private->remove_dependent_function(this);
	}
	if (dividend_private)
	{
		dividend_private->remove_dependent_function(this);
	}
#endif // defined (CIRCULAR_SMART_POINTERS)
}

EXPORT template<typename Value_type>
string_handle Function_matrix_divide_by_scalar<Value_type>::
	get_string_representation()
//******************************************************************************
// LAST MODIFIED : 6 March 2005
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		if (dividend_private&&divisor_private)
		{
			out << *(dividend_private->get_string_representation());
			out << "/";
			out << *(divisor_private->get_string_representation());
		}
		else
		{
			out << "Invalid Function_matrix_divide_by_scalar";
		}
		*return_string=out.str();
	}

	return (return_string);
}

EXPORT template<typename Value_type>
Function_variable_handle Function_matrix_divide_by_scalar<Value_type>::input()
//******************************************************************************
// LAST MODIFIED : 6 March 2005
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle function;
	Function_variable_handle input_1(0),input_2(0),result(0);

	if (dividend_private&&(function=dividend_private->function()))
	{
		input_1=function->input();
	}
	if (divisor_private&&(function=divisor_private->function()))
	{
		input_2=function->input();
	}
	if (input_1)
	{
		if (input_2)
		{
			result=Function_variable_handle(new Function_variable_union(
				Function_handle(this),input_1,input_2));
		}
		else
		{
			result=input_1;
		}
	}
	else
	{
		result=input_2;
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_variable_handle Function_matrix_divide_by_scalar<Value_type>::output()
//******************************************************************************
// LAST MODIFIED : 6 March 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(
		new Function_variable_matrix_divide_by_scalar<Value_type>(
		boost::intrusive_ptr< Function_matrix_divide_by_scalar<Value_type> >(
		this))));
}

EXPORT template<typename Value_type>
bool Function_matrix_divide_by_scalar<Value_type>::operator==(
	const Function& function) const
//******************************************************************************
// LAST MODIFIED : 6 March 2005
//
// DESCRIPTION :
// Equality operator.
//==============================================================================
{
	bool result;

	result=false;
	if (this)
	{
		try
		{
			const Function_matrix_divide_by_scalar<Value_type>&
				function_matrix_divide_by_scalar=
				dynamic_cast<const Function_matrix_divide_by_scalar<Value_type>&>(
				function);

			result=equivalent(dividend_private,
				function_matrix_divide_by_scalar.dividend_private)&&
				equivalent(divisor_private,
				function_matrix_divide_by_scalar.divisor_private);
		}
		catch (std::bad_cast)
		{
			// do nothing
		}
	}

	return (result);
}

EXPORT template<typename Value_type>
#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_matrix_divide_by_scalar<Value_type>::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 6 March 2005
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr< Function_variable_matrix_divide_by_scalar<Value_type> >
		atomic_variable_matrix_divide_by_scalar;
#if defined (EVALUATE_RETURNS_VALUE)
	Function_handle result(0);
#else // defined (EVALUATE_RETURNS_VALUE)
	bool result(true);
#endif // defined (EVALUATE_RETURNS_VALUE)

	if ((atomic_variable_matrix_divide_by_scalar=boost::dynamic_pointer_cast<
		Function_variable_matrix_divide_by_scalar<Value_type>,Function_variable>(
		atomic_variable))&&equivalent(Function_handle(this),
		atomic_variable_matrix_divide_by_scalar->function())&&
		(0<atomic_variable_matrix_divide_by_scalar->row())&&
		(0<atomic_variable_matrix_divide_by_scalar->column()))
	{
		result=atomic_variable_matrix_divide_by_scalar->evaluate();
	}

	return (result);
}

EXPORT template<typename Value_type>
bool Function_matrix_divide_by_scalar<Value_type>::evaluate_derivative(Scalar&,
	Function_variable_handle,std::list<Function_variable_handle>&)
//******************************************************************************
// LAST MODIFIED : 6 March 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (false);
}

#if !defined (AIX)
template<>
bool Function_matrix_divide_by_scalar<Scalar>::evaluate_derivative(
	Scalar& derivative,Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables);
#endif // !defined (AIX)

EXPORT template<typename Value_type>
bool Function_matrix_divide_by_scalar<Value_type>::set_value(
	Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 6 March 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_divide_by_scalar<Value_type> >
		atomic_matrix_variable;
	boost::intrusive_ptr< Function_variable_value_specific<Value_type> >
		value_type;
	Function_handle function;

	result=false;
	if ((atomic_matrix_variable=boost::dynamic_pointer_cast<
		Function_variable_matrix_divide_by_scalar<Value_type>,Function_variable>(
		atomic_variable))&&
		equivalent(Function_handle(this),atomic_matrix_variable->function())&&
		atomic_value&&(atomic_value->value())&&(value_type=
		boost::dynamic_pointer_cast<Function_variable_value_specific<Value_type>,
		Function_variable_value>(atomic_value->value())))
	{
		result=value_type->set(values((atomic_matrix_variable->row())-1,
			(atomic_matrix_variable->column())-1),atomic_value);
	}
	if (result)
	{
		this->set_not_evaluated();
	}
	else
	{
		if (function=dividend_private->function())
		{
			result=function->set_value(atomic_variable,atomic_value);
		}
		if (!result)
		{
			if (function=divisor_private->function())
			{
				result=function->set_value(atomic_variable,atomic_value);
			}
		}
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_handle Function_matrix_divide_by_scalar<Value_type>::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 6 March 2005
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle function,result(0);
	boost::intrusive_ptr< Function_variable_matrix_divide_by_scalar<Value_type> >
		atomic_variable_matrix_divide_by_scalar;
	ublas::matrix<Value_type,ublas::column_major> result_matrix(1,1);

	if (atomic_variable&&
		equivalent(Function_handle(this),(atomic_variable->function)())&&
		(atomic_variable_matrix_divide_by_scalar=boost::dynamic_pointer_cast<
		Function_variable_matrix_divide_by_scalar<Value_type>,Function_variable>(
		atomic_variable))&&
		(atomic_variable_matrix_divide_by_scalar->get_entry)(result_matrix(0,0)))
	{
		result=Function_handle(new Function_matrix<Value_type>(result_matrix));
	}
	if (!result)
	{
		if (function=dividend_private->function())
		{
			result=function->get_value(atomic_variable);
		}
		if (!result)
		{
			if (function=divisor_private->function())
			{
				result=function->get_value(atomic_variable);
			}
		}
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_matrix_divide_by_scalar<Value_type>::Function_matrix_divide_by_scalar(
	const Function_matrix_divide_by_scalar<Value_type>&
	function_matrix_divide_by_scalar):
	Function_matrix<Value_type>(function_matrix_divide_by_scalar),
	dividend_private(function_matrix_divide_by_scalar.dividend_private),
	divisor_private(function_matrix_divide_by_scalar.divisor_private)
//******************************************************************************
// LAST MODIFIED : 6 March 2005
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (divisor_private)
	{
		divisor_private->add_dependent_function(this);
	}
	if (dividend_private)
	{
		dividend_private->add_dependent_function(this);
	}
}

EXPORT template<typename Value_type>
Function_matrix_divide_by_scalar<Value_type>&
	Function_matrix_divide_by_scalar<Value_type>::operator=(
	const Function_matrix_divide_by_scalar<Value_type>&
	function_matrix_divide_by_scalar)
//******************************************************************************
// LAST MODIFIED : 6 March 2005
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	if (function_matrix_divide_by_scalar.divisor_private)
	{
		function_matrix_divide_by_scalar.divisor_private->add_dependent_function(
			this);
	}
	if (divisor_private)
	{
		divisor_private->remove_dependent_function(this);
	}
	divisor_private=function_matrix_divide_by_scalar.divisor_private;
	if (function_matrix_divide_by_scalar.dividend_private)
	{
		function_matrix_divide_by_scalar.dividend_private->add_dependent_function(
			this);
	}
	if (dividend_private)
	{
		dividend_private->remove_dependent_function(this);
	}
	dividend_private=function_matrix_divide_by_scalar.dividend_private;
	this->values=function_matrix_divide_by_scalar.values;

	return (*this);
}
