//******************************************************************************
// FILE : function_matrix_transpose.cpp
//
// LAST MODIFIED : 23 May 2005
//
// DESCRIPTION :
//==============================================================================

#if defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_transpose_implementation.cpp"
#else // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_transpose.hpp"
#endif // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#if !defined (AIX)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#include "computed_variable/function_derivative.hpp"
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#if defined (OLD_CODE)
template<>
Function_handle Function_variable_matrix_transpose<Scalar>::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
// ???DB.  To be done
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix_transpose<Scalar> >
		function_matrix_transpose;
	Function_handle result(0);

	if ((function_matrix_transpose=boost::dynamic_pointer_cast<
		Function_matrix_transpose<Scalar>,Function>(function()))&&
		(0<independent_variables.size()))
	{
		Function_size_type number_of_columns,number_of_dependent_values,
			number_of_independent_values,number_of_rows;
		boost::intrusive_ptr< Function_matrix<Scalar> > derivative,matrix;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_derivatnew_handle temp_function;
		Function_variable_handle temp_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

		if (
#if defined (EVALUATE_RETURNS_VALUE)
			(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_transpose->matrix_private->evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
			(function_matrix_transpose->matrix_private->evaluate)()&&
			(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_transpose->matrix_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
			(row_private<=(number_of_rows=matrix->number_of_columns()))&&
			(column_private<=(number_of_columns=matrix->number_of_rows()))&&
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(derivative=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_transpose->matrix_private->
			evaluate_derivative(independent_variables)))&&
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(temp_function=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(function_matrix_transpose->matrix_private->derivative(
			independent_variables)))&&(temp_variable=temp_function->output())&&
			(temp_variable->evaluate())&&(temp_variable=temp_function->matrix(
			independent_variables))&&(derivative=boost::dynamic_pointer_cast<
			Function_matrix<Scalar>,Function>(temp_variable->get_value()))&&
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(number_of_rows*number_of_columns==
			(number_of_dependent_values=derivative->number_of_rows())))
		{
			Function_size_type dependent_row,i,j;

			number_of_independent_values=derivative->number_of_columns();
			if (0==row_private)
			{
				if (0==column_private)
				{
					Matrix result_matrix(number_of_dependent_values,
						number_of_independent_values);

					for (i=1;i<=number_of_dependent_values;++i)
					{
						dependent_row=((i-1)%number_of_rows)*number_of_columns+
							(i-1)/number_of_rows;
						for (j=1;j<=number_of_independent_values;++j)
						{
							result_matrix(dependent_row,j-1)=(*derivative)(i,j);
						}
					}
					result=Function_handle(new Function_matrix<Scalar>(result_matrix));
				}
				else
				{
					Matrix result_matrix(number_of_rows,number_of_independent_values);

					dependent_row=(column_private-1)*number_of_rows+1;
					for (i=0;i<number_of_rows;++i)
					{
						for (j=1;j<=number_of_independent_values;++j)
						{
							result_matrix(i,j-1)=(*derivative)(dependent_row,j);
						}
						++dependent_row;
					}
					result=Function_handle(new Function_matrix<Scalar>(result_matrix));
				}
			}
			else
			{
				if (0==column_private)
				{
					Matrix result_matrix(number_of_columns,number_of_independent_values);

					dependent_row=row_private;
					for (i=0;i<number_of_columns;++i)
					{
						for (j=1;j<=number_of_independent_values;++j)
						{
							result_matrix(i,j-1)=(*derivative)(dependent_row,j);
						}
						dependent_row += number_of_rows;
					}
					result=Function_handle(new Function_matrix<Scalar>(result_matrix));
				}
				else
				{
					Matrix result_matrix(1,number_of_independent_values);

					dependent_row=(column_private-1)*number_of_rows+row_private;
					for (j=1;j<=number_of_independent_values;++j)
					{
						result_matrix(0,j-1)=(*derivative)(dependent_row,j);
					}
					result=Function_handle(new Function_matrix<Scalar>(result_matrix));
				}
			}
		}
	}

	return (result);
}
#endif // defined (OLD_CODE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_derivatnew_matrix_transpose
// ------------------------------------------

class Function_derivatnew_matrix_transpose : public Function_derivatnew
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
		Function_derivatnew_matrix_transpose(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables);
		// destructor
		~Function_derivatnew_matrix_transpose();
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

Function_derivatnew_matrix_transpose::Function_derivatnew_matrix_transpose(
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
	boost::intrusive_ptr< Function_matrix_transpose<Scalar> >
		function_matrix_transpose;
	boost::intrusive_ptr< Function_variable_matrix_transpose<Scalar> >
		variable_matrix_transpose;

	if ((variable_matrix_transpose=boost::dynamic_pointer_cast<
		Function_variable_matrix_transpose<Scalar>,Function_variable>(
		dependent_variable))&&(function_matrix_transpose=
		boost::dynamic_pointer_cast<Function_matrix_transpose<Scalar>,Function>(
		dependent_variable->function())))
	{
		if (matrix_derivative=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(function_matrix_transpose->matrix_private->derivative(
			independent_variables)))
		{
			matrix_derivative->add_dependent_function(this);
		}
		else
		{
			throw Function_derivatnew_matrix_transpose::Construction_exception();
		}
	}
	else
	{
		throw Function_derivatnew_matrix_transpose::Construction_exception();
	}
}

Function_derivatnew_matrix_transpose::~Function_derivatnew_matrix_transpose()
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
Function_handle Function_variable_matrix_transpose<Scalar>::derivative(
	const std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 26 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_handle(new Function_derivatnew_matrix_transpose(
		Function_variable_handle(this),independent_variables)));
}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
template<>
Function_handle Function_variable_matrix_transpose<Scalar>::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_derivatnew_matrix_transpose::evaluate(
	Function_variable_handle
#if defined (EVALUATE_RETURNS_VALUE)
	atomic_variable
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
	)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
//******************************************************************************
// LAST MODIFIED : 23 May 2005
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
		boost::intrusive_ptr< Function_matrix_transpose<Scalar> >
			function_matrix_transpose;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		boost::intrusive_ptr< Function_variable_matrix_transpose<Scalar> >
			variable_matrix_transpose;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

		if (
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(variable_matrix_transpose=boost::dynamic_pointer_cast<
			Function_variable_matrix_transpose<Scalar>,Function_variable>(
			dependent_variable))&&
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(function_matrix_transpose=boost::dynamic_pointer_cast<
			Function_matrix_transpose<Scalar>,Function>(
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			dependent_variable->
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			function()))&&(0<independent_variables.size()))
		{
			boost::intrusive_ptr< Function_matrix<Scalar> > derivative,matrix;
			Function_size_type number_of_columns,number_of_dependent_values,
				number_of_independent_values,number_of_rows;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			Function_size_type column_private,row_private;
			Function_variable_handle temp_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

			if (
#if defined (EVALUATE_RETURNS_VALUE)
				(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_transpose->matrix_private->evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
				(function_matrix_transpose->matrix_private->evaluate)()&&
				(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_transpose->matrix_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
				(
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				row_private
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				=variable_matrix_transpose->row())
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				<=(number_of_rows=matrix->number_of_columns()))&&
				(
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				column_private
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				=variable_matrix_transpose->column())
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				<=(number_of_columns=matrix->number_of_rows()))&&
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(derivative=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_transpose->matrix_private->
				evaluate_derivative(independent_variables)))&&
				(number_of_rows*number_of_columns==
				(number_of_dependent_values=derivative->number_of_rows()))
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(temp_variable=matrix_derivative->output())&&(temp_variable->evaluate())
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				)
			{
				Function_size_type dependent_row,i,j;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				Function_size_type k;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				number_of_independent_values=derivative->number_of_columns();
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				if (0==row_private)
				{
					if (0==column_private)
					{
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						Matrix result_matrix(number_of_dependent_values,
							number_of_independent_values);

						for (i=1;i<=number_of_dependent_values;++i)
						{
							dependent_row=((i-1)%number_of_rows)*number_of_columns+
								(i-1)/number_of_rows;
							for (j=1;j<=number_of_independent_values;++j)
							{
								result_matrix(dependent_row,j-1)=(*derivative)(i,j);
							}
						}
						result=Function_handle(new Function_matrix<Scalar>(result_matrix));
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						std::list<Matrix> matrices;
						std::list<Matrix>::iterator matrix_iterator;

						derivative_matrix.clear();
						matrix_iterator=(matrix_derivative->derivative_matrix).begin();
						number_of_dependent_values=matrix_iterator->size1();
						for (k=(matrix_derivative->derivative_matrix).size();k>0;--k)
						{
							Matrix& derivative= *matrix_iterator;
							Matrix result_matrix(number_of_dependent_values,
								(number_of_independent_values=derivative.size2()));

							for (i=0;i<number_of_dependent_values;++i)
							{
								dependent_row=(i%number_of_rows)*number_of_columns+
									i/number_of_rows;
								for (j=0;j<number_of_independent_values;++j)
								{
									result_matrix(dependent_row,j)=derivative(i,j);
								}
							}
							matrices.push_back(result_matrix);
							matrix_iterator++;
						}
						derivative_matrix=Derivative_matrix(matrices);
						set_evaluated();
#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
						result=true;
#endif // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					}
					else
					{
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						Matrix result_matrix(number_of_rows,number_of_independent_values);

						dependent_row=(column_private-1)*number_of_rows+1;
						for (i=0;i<number_of_rows;++i)
						{
							for (j=1;j<=number_of_independent_values;++j)
							{
								result_matrix(i,j-1)=(*derivative)(dependent_row,j);
							}
							++dependent_row;
						}
						result=Function_handle(new Function_matrix<Scalar>(result_matrix));
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						std::list<Matrix> matrices;
						std::list<Matrix>::iterator matrix_iterator;

						derivative_matrix.clear();
						matrix_iterator=(matrix_derivative->derivative_matrix).begin();
						for (k=(matrix_derivative->derivative_matrix).size();k>0;--k)
						{
							Matrix& derivative= *matrix_iterator;
							Matrix result_matrix(number_of_rows,
								(number_of_independent_values=derivative.size2()));

							dependent_row=(column_private-1)*number_of_rows;
							for (i=0;i<number_of_rows;++i)
							{
								for (j=0;j<number_of_independent_values;++j)
								{
									result_matrix(i,j)=derivative(dependent_row,j);
								}
								++dependent_row;
							}
							matrices.push_back(result_matrix);
							matrix_iterator++;
						}
						derivative_matrix=Derivative_matrix(matrices);
						set_evaluated();
#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
						result=true;
#endif // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					}
				}
				else
				{
					if (0==column_private)
					{
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						Matrix result_matrix(number_of_columns,
							number_of_independent_values);

						dependent_row=row_private;
						for (i=0;i<number_of_columns;++i)
						{
							for (j=1;j<=number_of_independent_values;++j)
							{
								result_matrix(i,j-1)=(*derivative)(dependent_row,j);
							}
							dependent_row += number_of_rows;
						}
						result=Function_handle(new Function_matrix<Scalar>(result_matrix));
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						std::list<Matrix> matrices;
						std::list<Matrix>::iterator matrix_iterator;

						derivative_matrix.clear();
						matrix_iterator=(matrix_derivative->derivative_matrix).begin();
						for (k=(matrix_derivative->derivative_matrix).size();k>0;--k)
						{
							Matrix& derivative= *matrix_iterator;
							Matrix result_matrix(number_of_columns,
								(number_of_independent_values=derivative.size2()));

							dependent_row=row_private-1;
							for (i=0;i<number_of_rows;++i)
							{
								for (j=0;j<number_of_independent_values;++j)
								{
									result_matrix(i,j)=derivative(dependent_row,j);
								}
								dependent_row += number_of_rows;
							}
							matrices.push_back(result_matrix);
							matrix_iterator++;
						}
						derivative_matrix=Derivative_matrix(matrices);
						set_evaluated();
#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
						result=true;
#endif // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					}
					else
					{
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						Matrix result_matrix(1,number_of_independent_values);

						dependent_row=(column_private-1)*number_of_rows+row_private;
						for (j=1;j<=number_of_independent_values;++j)
						{
							result_matrix(0,j-1)=(*derivative)(dependent_row,j);
						}
						result=Function_handle(new Function_matrix<Scalar>(result_matrix));
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						std::list<Matrix> matrices;
						std::list<Matrix>::iterator matrix_iterator;

						derivative_matrix.clear();
						matrix_iterator=(matrix_derivative->derivative_matrix).begin();
						for (k=(matrix_derivative->derivative_matrix).size();k>0;--k)
						{
							Matrix& derivative= *matrix_iterator;
							Matrix result_matrix(1,
								(number_of_independent_values=derivative.size2()));

							dependent_row=(column_private-1)*number_of_rows+row_private-1;
							for (j=0;j<number_of_independent_values;++j)
							{
								result_matrix(0,j)=derivative(dependent_row,j);
							}
							matrices.push_back(result_matrix);
							matrix_iterator++;
						}
						derivative_matrix=Derivative_matrix(matrices);
						set_evaluated();
#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
						result=true;
#endif // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					}
				}
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
bool Function_matrix_transpose<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 26 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_transpose<Scalar> >
		atomic_variable_matrix_transpose;

	result=false;
	if ((atomic_variable_matrix_transpose=boost::dynamic_pointer_cast<
		Function_variable_matrix_transpose<Scalar>,Function_variable>(
		atomic_variable))&&equivalent(Function_handle(this),
		atomic_variable_matrix_transpose->function())&&
		(0<atomic_variable_matrix_transpose->row())&&
		(0<atomic_variable_matrix_transpose->column())&&
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
			Function_matrix<Scalar>,Function>(atomic_variable_matrix_transpose->
			evaluate_derivative(atomic_independent_variables)))&&
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(derivative_function=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(atomic_variable_matrix_transpose->derivative(
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
#endif // !defined (AIX)
