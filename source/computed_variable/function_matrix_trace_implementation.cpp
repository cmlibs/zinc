//******************************************************************************
// FILE : function_matrix_trace_implementation.cpp
//
// LAST MODIFIED : 28 February 2005
//
// DESCRIPTION :
//==============================================================================

#include <sstream>

#include "computed_variable/function_matrix_trace.hpp"
#include "computed_variable/function_variable.hpp"
#include "computed_variable/function_variable_matrix.hpp"
#include "computed_variable/function_variable_value_specific.hpp"


// module classes
// ==============

// class Function_variable_matrix_trace
// ------------------------------------

EXPORT template<typename Value_type>
class Function_variable_matrix_trace :
	public Function_variable_matrix<Value_type>
//******************************************************************************
// LAST MODIFIED : 28 February 2005
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_matrix<Value_type>;
	public:
		// constructor
		Function_variable_matrix_trace(
			const boost::intrusive_ptr< Function_matrix_trace<Value_type> >
			function_matrix_trace):
			Function_variable_matrix<Value_type>(function_matrix_trace,
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			false,
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			1,1){};
		~Function_variable_matrix_trace(){}
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_matrix_trace<Value_type>(*this)));
		};
#if defined (EVALUATE_RETURNS_VALUE)
		Function_handle evaluate()
		{
			Function_handle result(0);
			boost::intrusive_ptr< Function_matrix_trace<Value_type> >
				function_matrix_trace;

			if (function_matrix_trace=boost::dynamic_pointer_cast<
				Function_matrix_trace<Value_type>,Function>(function()))
			{
#if defined (BEFORE_CACHING)
				Function_size_type number_of_rows;
				boost::intrusive_ptr< Function_matrix<Value_type> > matrix;

				if ((matrix=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
					Function>(function_matrix_trace->matrix_private->evaluate()))&&
					(matrix->number_of_columns()==
					(number_of_rows=matrix->number_of_rows())))
				{
					Function_size_type i;
					Value_type sum;

					sum=0;
					for (i=1;i<=number_of_rows;i++)
					{
						sum += (*matrix)(i,i);
					}
					function_matrix_trace->values(0,0)=sum;
					result=Function_handle(new Function_matrix<Value_type>(
						function_matrix_trace->values));
				}
#else // defined (BEFORE_CACHING)
				if (!(function_matrix_trace->evaluated()))
				{
					Function_size_type number_of_rows;
					boost::intrusive_ptr< Function_matrix<Value_type> > matrix;

					if ((matrix=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
						Function>(function_matrix_trace->matrix_private->evaluate()))&&
						(matrix->number_of_columns()==
						(number_of_rows=matrix->number_of_rows())))
					{
						Function_size_type i;
						Value_type sum;

						sum=0;
						for (i=1;i<=number_of_rows;i++)
						{
							sum += (*matrix)(i,i);
						}
						function_matrix_trace->values(0,0)=sum;
						function_matrix_trace->set_evaluated();
					}
				}
				if (function_matrix_trace->evaluated())
				{
					result=Function_handle(new Function_matrix<Value_type>(
						function_matrix_trace->values));
				}
#endif // defined (BEFORE_CACHING)
			}

			return (result);
		};
#else // defined (EVALUATE_RETURNS_VALUE)
		bool evaluate()
		{
			bool result(true);
			boost::intrusive_ptr< Function_matrix_trace<Value_type> >
				function_matrix_trace;

			if (function_matrix_trace=boost::dynamic_pointer_cast<
				Function_matrix_trace<Value_type>,Function>(function()))
			{
#if defined (BEFORE_CACHING)
				Function_size_type number_of_rows;
				boost::intrusive_ptr< Function_matrix<Value_type> > matrix;

				result=false;
				if ((function_matrix_trace->matrix_private->evaluate)()&&
					(matrix=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
					Function>(function_matrix_trace->matrix_private->get_value()))&&
					(matrix->number_of_columns()==
					(number_of_rows=matrix->number_of_rows())))
				{
					Function_size_type i;
					Value_type sum;

					sum=0;
					for (i=1;i<=number_of_rows;i++)
					{
						sum += (*matrix)(i,i);
					}
					function_matrix_trace->values(0,0)=sum;
					result=true;
				}
#else // defined (BEFORE_CACHING)
				if (!(function_matrix_trace->evaluated()))
				{
					Function_size_type number_of_rows;
					boost::intrusive_ptr< Function_matrix<Value_type> > matrix;

					result=false;
					if ((function_matrix_trace->matrix_private->evaluate)()&&
						(matrix=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
						Function>(function_matrix_trace->matrix_private->get_value()))&&
						(matrix->number_of_columns()==
						(number_of_rows=matrix->number_of_rows())))
					{
						Function_size_type i;
						Value_type sum;

						sum=0;
						for (i=1;i<=number_of_rows;i++)
						{
							sum += (*matrix)(i,i);
						}
						function_matrix_trace->values(0,0)=sum;
						function_matrix_trace->set_evaluated();
						result=true;
					}
				}
#endif // defined (BEFORE_CACHING)
			}

			return (result);
		};
#endif // defined (EVALUATE_RETURNS_VALUE)
		Function_handle evaluate_derivative(std::list<Function_variable_handle>&)
		{
			return (0);
		};
		//???DB.  Should operator() and get_entry do an evaluate?
		boost::intrusive_ptr< Function_variable_matrix<Value_type> > operator()(
			Function_size_type row=1,Function_size_type column=1) const
		{
			boost::intrusive_ptr< Function_matrix_trace<Value_type> >
				function_matrix_trace;
			boost::intrusive_ptr< Function_variable_matrix<Value_type> > result(0);

			if ((function_matrix_trace=boost::dynamic_pointer_cast<
				Function_matrix_trace<Value_type>,Function>(function_private))&&
				(row<=number_of_rows())&&(column<=number_of_columns()))
			{
				result=boost::intrusive_ptr< Function_variable_matrix<Value_type> >(
					new Function_variable_matrix_trace<Value_type>(
					function_matrix_trace));
			}

			return (result);
		}
	private:
		// copy constructor
		Function_variable_matrix_trace(
			const Function_variable_matrix_trace<Value_type>& variable):
			Function_variable_matrix<Value_type>(variable){};
};


// global classes
// ==============

// class Function_matrix_trace
// ---------------------------

EXPORT template<typename Value_type>
ublas::matrix<Value_type,ublas::column_major> 
	Function_matrix_trace<Value_type>::constructor_values(1,1);

EXPORT template<typename Value_type>
Function_matrix_trace<Value_type>::Function_matrix_trace(
	const Function_variable_handle& matrix):Function_matrix<Value_type>(
	Function_matrix_trace<Value_type>::constructor_values),matrix_private(matrix)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (matrix_private)
	{
		matrix_private->add_dependent_function(this);
	}
}

EXPORT template<typename Value_type>
Function_matrix_trace<Value_type>::~Function_matrix_trace()
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
#if defined (CIRCULAR_SMART_POINTERS)
	// do nothing
#else // defined (CIRCULAR_SMART_POINTERS)
	if (matrix_private)
	{
		matrix_private->remove_dependent_function(this);
	}
#endif // defined (CIRCULAR_SMART_POINTERS)
}

EXPORT template<typename Value_type>
string_handle Function_matrix_trace<Value_type>::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 10 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		if (matrix_private)
		{
			out << "trace(";
			out << *(matrix_private->get_string_representation());
			out << ")";
		}
		else
		{
			out << "Invalid Function_matrix_trace";
		}
		*return_string=out.str();
	}

	return (return_string);
}

EXPORT template<typename Value_type>
Function_variable_handle Function_matrix_trace<Value_type>::input()
//******************************************************************************
// LAST MODIFIED : 10 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle function;
	Function_variable_handle result(0);

	if (matrix_private&&(function=matrix_private->function()))
	{
		result=function->input();
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_variable_handle Function_matrix_trace<Value_type>::output()
//******************************************************************************
// LAST MODIFIED : 10 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(
		new Function_variable_matrix_trace<Value_type>(
		boost::intrusive_ptr< Function_matrix_trace<Value_type> >(this))));
}

EXPORT template<typename Value_type>
bool Function_matrix_trace<Value_type>::operator==(
	const Function& function) const
//******************************************************************************
// LAST MODIFIED : 10 September 2004
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
			const Function_matrix_trace<Value_type>& function_matrix_trace=
				dynamic_cast<const Function_matrix_trace<Value_type>&>(function);

			result=equivalent(matrix_private,function_matrix_trace.matrix_private);
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
	Function_matrix_trace<Value_type>::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 13 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr< Function_variable_matrix<Value_type> >
		atomic_variable_matrix_trace;
#if defined (EVALUATE_RETURNS_VALUE)
	Function_handle result(0);
#else // defined (EVALUATE_RETURNS_VALUE)
	bool result(true);
#endif // defined (EVALUATE_RETURNS_VALUE)

	if (atomic_variable&&
		equivalent(Function_handle(this),(atomic_variable->function)())&&
		(atomic_variable_matrix_trace=boost::dynamic_pointer_cast<
		Function_variable_matrix_trace<Value_type>,Function_variable>(
		atomic_variable))&&(1==atomic_variable_matrix_trace->row())&&
		(1==atomic_variable_matrix_trace->column()))
	{
		result=atomic_variable_matrix_trace->evaluate();
	}

	return (result);
}

#if defined (OLD_CODE)
EXPORT template<typename Value_type>
Function_handle Function_matrix_trace<Value_type>::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 10 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr< Function_variable_matrix<Value_type> >
		atomic_variable_matrix_trace;
	Function_handle result(0);

	if (atomic_variable&&
		equivalent(Function_handle(this),(atomic_variable->function)())&&
		(atomic_variable_matrix_trace=boost::dynamic_pointer_cast<
		Function_variable_matrix_trace<Value_type>,Function_variable>(
		atomic_variable))&&(1==atomic_variable_matrix_trace->row())&&
		(1==atomic_variable_matrix_trace->column()))
	{
		boost::intrusive_ptr< Function_variable_matrix<Value_type> > temp_variable;
		Function_size_type i,number_of_rows;
		ublas::matrix<Value_type,ublas::column_major> result_matrix(1,1);
		Value_type sum,value;

		number_of_rows=matrix_private->number_of_rows();
		i=1;
		sum=0;
		while ((i<=number_of_rows)&&(temp_variable=(*matrix_private)(i,i))&&
			(temp_variable->evaluate())&&(temp_variable->get_entry(value)))
		{
			sum += value;
			i++;
		}
		if (i>number_of_rows);
		{
			result_matrix(0,0)=sum;
			values(0,0)=sum;
			result=Function_handle(new Function_matrix<Value_type>(result_matrix));
		}
	}

	return (result);
}
#endif // defined (OLD_CODE)

EXPORT template<typename Value_type>
bool Function_matrix_trace<Value_type>::evaluate_derivative(Scalar&,
	Function_variable_handle,std::list<Function_variable_handle>&)
//******************************************************************************
// LAST MODIFIED : 10 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (false);
}

#if !defined (AIX)
template<>
bool Function_matrix_trace<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables);
#endif // !defined (AIX)

EXPORT template<typename Value_type>
bool Function_matrix_trace<Value_type>::set_value(
	Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 1 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_trace<Value_type> >
		atomic_matrix_variable;
	boost::intrusive_ptr< Function_variable_value_specific<Value_type> >
		value_type;
	Function_handle function;

	result=false;
	if ((atomic_matrix_variable=boost::dynamic_pointer_cast<
		Function_variable_matrix_trace<Value_type>,Function_variable>(
		atomic_variable))&&
		equivalent(Function_handle(this),atomic_matrix_variable->function())&&
		atomic_value&&(atomic_value->value())&&(value_type=
		boost::dynamic_pointer_cast<Function_variable_value_specific<Value_type>,
		Function_variable_value>(atomic_value->value())))
	{
		result=value_type->set(values(0,0),atomic_value);
	}
	if (result)
	{
		set_not_evaluated();
	}
	else
	{
		if (function=matrix_private->function())
		{
			result=function->set_value(atomic_variable,atomic_value);
		}
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_handle Function_matrix_trace<Value_type>::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 10 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle function,result(0);
	boost::intrusive_ptr< Function_variable_matrix_trace<Value_type> >
		atomic_variable_matrix_trace;
	ublas::matrix<Value_type,ublas::column_major> result_matrix(1,1);

	if (atomic_variable&&
		equivalent(Function_handle(this),(atomic_variable->function)())&&
		(atomic_variable_matrix_trace=boost::dynamic_pointer_cast<
		Function_variable_matrix_trace<Value_type>,Function_variable>(
		atomic_variable))&&
		(atomic_variable_matrix_trace->get_entry)(result_matrix(0,0)))
	{
		result=Function_handle(new Function_matrix<Value_type>(result_matrix));
	}
	if (!result)
	{
		if (function=matrix_private->function())
		{
			result=function->get_value(atomic_variable);
		}
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_matrix_trace<Value_type>::Function_matrix_trace(
	const Function_matrix_trace<Value_type>& function_matrix_trace):
	Function_matrix<Value_type>(function_matrix_trace),
	matrix_private(function_matrix_trace.matrix_private)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (matrix_private)
	{
		matrix_private->add_dependent_function(this);
	}
}

EXPORT template<typename Value_type>
Function_matrix_trace<Value_type>& Function_matrix_trace<Value_type>::operator=(
	const Function_matrix_trace<Value_type>& function_matrix_trace)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	if (function_matrix_trace.matrix_private)
	{
		function_matrix_trace.matrix_private->add_dependent_function(this);
	}
	if (matrix_private)
	{
		matrix_private->remove_dependent_function(this);
	}

	return (*this);
}
