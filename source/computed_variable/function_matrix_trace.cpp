//******************************************************************************
// FILE : function_matrix_trace.cpp
//
// LAST MODIFIED : 26 April 2005
//
// DESCRIPTION :
//==============================================================================

#if defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_trace_implementation.cpp"
#else // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_trace.hpp"
#endif // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#if !defined (AIX)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#include "computed_variable/function_derivative.hpp"
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#if defined (OLD_CODE)
template<>
Function_handle Function_variable_matrix_trace<Scalar>::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
// ???DB.  To be done
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix_trace<Scalar> > function_matrix_trace;
	Function_handle result(0);

	if ((function_matrix_trace=boost::dynamic_pointer_cast<
		Function_matrix_trace<Scalar>,Function>(function()))&&
		(0<independent_variables.size()))
	{
		Function_size_type number_of_independent_values,number_of_rows;
		boost::intrusive_ptr< Function_matrix<Scalar> > derivative,matrix;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_derivatnew_handle temp_function;
		Function_variable_handle temp_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

		if (
#if defined (EVALUATE_RETURNS_VALUE)
			(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_trace->matrix_private->evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
			(function_matrix_trace->matrix_private->evaluate)()&&
			(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_trace->matrix_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
			(matrix->number_of_columns()==(number_of_rows=matrix->number_of_rows()))&&
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(derivative=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_trace->matrix_private->evaluate_derivative(
			independent_variables)))&&
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(temp_function=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(function_matrix_trace->matrix_private->derivative(
			independent_variables)))&&(temp_variable=temp_function->output())&&
			(temp_variable->evaluate())&&(temp_variable=temp_function->matrix(
			independent_variables))&&(derivative=boost::dynamic_pointer_cast<
			Function_matrix<Scalar>,Function>(temp_variable->get_value()))&&
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(number_of_rows*number_of_rows==derivative->number_of_rows())&&
			(0<(number_of_independent_values=derivative->number_of_columns())))
		{
			Function_size_type dependent_row,i,j;
			Matrix result_matrix(1,number_of_independent_values);
			Scalar sum;

			for (j=1;j<=number_of_independent_values;++j)
			{
				sum=0;
				dependent_row=1;
				for (i=0;i<number_of_rows;++i)
				{
					sum += (*derivative)(dependent_row,j);
					dependent_row += number_of_rows+1;
				}
				result_matrix(0,j-1)=sum;
			}
			result=Function_handle(new Function_matrix<Scalar>(result_matrix));
		}
	}

	return (result);
}
#endif // defined (OLD_CODE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_derivatnew_matrix_trace
// --------------------------------------

class Function_derivatnew_matrix_trace : public Function_derivatnew
//******************************************************************************
// LAST MODIFIED : 26 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// for construction exception
		class Construction_exception {};
		// constructor
		Function_derivatnew_matrix_trace(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables);
		// destructor
		~Function_derivatnew_matrix_trace();
	// inherited
	private:
#if defined (EVALUATE_RETURNS_VALUE)
		virtual Function_handle evaluate(Function_variable_handle atomic_variable);
#else // defined (EVALUATE_RETURNS_VALUE)
		virtual bool evaluate(Function_variable_handle atomic_variable);
#endif // defined (EVALUATE_RETURNS_VALUE)
	private:
		// copy operations are private and undefined to prevent copying
		void operator=(const Function&);
	private:
		Function_derivatnew_handle matrix_derivative;
};

Function_derivatnew_matrix_trace::Function_derivatnew_matrix_trace(
	const Function_variable_handle& dependent_variable,
	const std::list<Function_variable_handle>& independent_variables):
	Function_derivatnew(dependent_variable,independent_variables)
//******************************************************************************
// LAST MODIFIED : 26 April 2005
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix_trace<Scalar> > function_matrix_trace;
	boost::intrusive_ptr< Function_variable_matrix_trace<Scalar> >
		variable_matrix_trace;

	if ((variable_matrix_trace=boost::dynamic_pointer_cast<
		Function_variable_matrix_trace<Scalar>,Function_variable>(
		dependent_variable))&&(function_matrix_trace=boost::dynamic_pointer_cast<
		Function_matrix_trace<Scalar>,Function>(dependent_variable->function())))
	{
		if (matrix_derivative=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(function_matrix_trace->matrix_private->derivative(
			independent_variables)))
		{
			matrix_derivative->add_dependent_function(this);
		}
		else
		{
			throw Function_derivatnew_matrix_trace::Construction_exception();
		}
	}
	else
	{
		throw Function_derivatnew_matrix_trace::Construction_exception();
	}
}

Function_derivatnew_matrix_trace::~Function_derivatnew_matrix_trace()
//******************************************************************************
// LAST MODIFIED : 26 April 2005
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
#if defined (CIRCULAR_SMART_POINTERS)
	// do nothing
#else // defined (CIRCULAR_SMART_POINTERS)
	if (matrix_derivative)
	{
		matrix_derivative->remove_dependent_function(this);
	}
#endif // defined (CIRCULAR_SMART_POINTERS)
}

template<>
Function_handle Function_variable_matrix_trace<Scalar>::derivative(
	const std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 26 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_handle(new Function_derivatnew_matrix_trace(
		Function_variable_handle(this),independent_variables)));
}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
template<>
Function_handle Function_variable_matrix_trace<Scalar>::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_derivatnew_matrix_trace::evaluate(
	Function_variable_handle
#if defined (EVALUATE_RETURNS_VALUE)
	atomic_variable
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
	)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
//******************************************************************************
// LAST MODIFIED : 26 April 2005
//
// DESCRIPTION :
//==============================================================================
{
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE) || defined (EVALUATE_RETURNS_VALUE)
	Function_handle result(0);
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE) || defined (EVALUATE_RETURNS_VALUE)
	bool result(true);
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE) || defined (EVALUATE_RETURNS_VALUE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
	if (!evaluated())
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
	{
		boost::intrusive_ptr< Function_matrix_trace<Scalar> > function_matrix_trace;

		if (
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			boost::dynamic_pointer_cast<Function_variable_matrix_trace<Scalar>,
			Function_variable>(dependent_variable)&&
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(function_matrix_trace=boost::dynamic_pointer_cast<
			Function_matrix_trace<Scalar>,Function>(
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			dependent_variable->
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			function()))&&(0<independent_variables.size()))
		{
			boost::intrusive_ptr< Function_matrix<Scalar> > derivative,matrix;
			Function_size_type number_of_independent_values,number_of_rows;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			Function_variable_handle temp_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

			if (
#if defined (EVALUATE_RETURNS_VALUE)
				(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_trace->matrix_private->evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
				(function_matrix_trace->matrix_private->evaluate)()&&
				(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_trace->matrix_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
				(matrix->number_of_columns()==
				(number_of_rows=matrix->number_of_rows()))&&
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(derivative=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_trace->matrix_private->evaluate_derivative(
				independent_variables)))&&
				(number_of_rows*number_of_rows==derivative->number_of_rows())&&
				(0<(number_of_independent_values=derivative->number_of_columns()))
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(temp_variable=matrix_derivative->output())&&(temp_variable->evaluate())
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				)
			{
				Function_size_type dependent_row,i,j;
				Scalar sum;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				Matrix result_matrix(1,number_of_independent_values);
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				Function_size_type k;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				for (j=1;j<=number_of_independent_values;++j)
				{
					sum=0;
					dependent_row=1;
					for (i=0;i<number_of_rows;++i)
					{
						sum += (*derivative)(dependent_row,j);
						dependent_row += number_of_rows+1;
					}
					result_matrix(0,j-1)=sum;
				}
				result=Function_handle(new Function_matrix<Scalar>(result_matrix));
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				std::list<Matrix> matrices;
				std::list<Matrix>::iterator matrix_iterator;

				derivative_matrix.clear();
				matrix_iterator=(matrix_derivative->derivative_matrix).begin();
				for (k=(matrix_derivative->derivative_matrix).size();k>0;--k)
				{
					Matrix& matrix_ref= *matrix_iterator;
					Matrix result_matrix(1,
						(number_of_independent_values=matrix_ref.size2()));

					for (j=0;j<number_of_independent_values;++j)
					{
						sum=0;
						dependent_row=0;
						for (i=number_of_rows;i>0;--i)
						{
							sum += matrix_ref(dependent_row,j);
							dependent_row += number_of_rows+1;
						}
						result_matrix(0,j)=sum;
					}
					matrices.push_back(result_matrix);
					matrix_iterator++;
				}
				derivative_matrix=Derivative_matrix(matrices);
				result=true;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			}
		}
	}
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#if defined (EVALUATE_RETURNS_VALUE)
	if (evaluated())
	{
		result=get_value(atomic_variable);
	}
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

	return (result);
}

template<>
bool Function_matrix_trace<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 26 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_trace<Scalar> >
		atomic_variable_matrix_trace;

	result=false;
	if ((atomic_variable_matrix_trace=boost::dynamic_pointer_cast<
		Function_variable_matrix_trace<Scalar>,Function_variable>(
		atomic_variable))&&equivalent(Function_handle(this),
		atomic_variable_matrix_trace->function())&&
		(0<atomic_variable_matrix_trace->row())&&
		(0<atomic_variable_matrix_trace->column())&&
		(0<atomic_independent_variables.size()))
	{
		boost::intrusive_ptr< Function_matrix<Scalar> > derivative_value;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_derivatnew_handle derivative_function;
		Function_variable_handle derivative_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

		if (
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(derivative_value=boost::dynamic_pointer_cast<
			Function_matrix<Scalar>,Function>(atomic_variable_matrix_trace->
			evaluate_derivative(atomic_independent_variables)))&&
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(derivative_function=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(atomic_variable_matrix_trace->derivative(
			atomic_independent_variables)))&&(derivative_variable=
			derivative_function->output())&&(derivative_variable->evaluate())&&
			(derivative_variable=derivative_function->matrix(
			atomic_independent_variables))&&
			(derivative_value=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(derivative_variable->get_value()))&&
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(1==derivative_value->number_of_rows())&&
			(1==derivative_value->number_of_columns()))
		{
			derivative=(*derivative_value)(1,1);
			result=true;
		}
	}

	return (result);
}

#if defined (OLD_CODE)
template<>
bool Function_matrix_trace<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_trace<Scalar> >
		atomic_variable_matrix_trace;

	result=false;
	if ((atomic_variable_matrix_trace=boost::dynamic_pointer_cast<
		Function_variable_matrix_trace<Scalar>,Function_variable>(
		atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_matrix_trace->function())&&
		(1==atomic_variable_matrix_trace->row())&&
		(1==atomic_variable_matrix_trace->column()))
	{
		boost::intrusive_ptr< Function_variable_matrix<Scalar> > temp_variable;
		Function_handle function;
		Function_size_type i,number_of_rows;
		Scalar sum,value;

		sum=0;
		number_of_rows=matrix_private->number_of_rows();
		i=1;
		while ((i<=number_of_rows)&&(temp_variable=(*matrix_private)(i,i))&&
			(function=temp_variable->function())&&(function->evaluate_derivative)(
			value,temp_variable,atomic_independent_variables))
		{
			sum += value;
			++i;
		}
		if (i>number_of_rows)
		{
			result=true;
			derivative=sum;
		}
	}

	return (result);
}
#endif // defined (OLD_CODE)
#endif // !defined (AIX)
