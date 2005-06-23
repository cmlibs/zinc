//******************************************************************************
// FILE : function_matrix_resize.cpp
//
// LAST MODIFIED : 23 May 2005
//
// DESCRIPTION :
//==============================================================================

#if defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_resize_implementation.cpp"
#else // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_resize.hpp"
#endif // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#if !defined (AIX)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#include "computed_variable/function_derivative.hpp"
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#if defined (OLD_CODE)
template<>
Function_handle Function_variable_matrix_resize<Scalar>::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix_resize<Scalar> >
		function_matrix_resize;
	Function_handle result(0);

	if ((function_matrix_resize=boost::dynamic_pointer_cast<
		Function_matrix_resize<Scalar>,Function>(function()))&&
		(0<independent_variables.size()))
	{
		Function_size_type number_of_columns,number_of_independent_values,
			number_of_rows,size;
		boost::intrusive_ptr< Function_matrix<Scalar> > derivative,matrix;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_derivatnew_handle temp_function;
		Function_variable_handle temp_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

		if (
#if defined (EVALUATE_RETURNS_VALUE)
			(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_resize->matrix_private->evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
			(function_matrix_resize->matrix_private->evaluate)()&&
			(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_resize->matrix_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
			(0<(size=(matrix->number_of_columns())*(matrix->number_of_rows())))&&
			(0<(number_of_columns=
			function_matrix_resize->number_of_columns_private))&&
			(0==size%number_of_columns)&&
			(row_private<=(number_of_rows=size/number_of_columns))&&
			(column_private<=number_of_columns)&&
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(derivative=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_resize->matrix_private->
			evaluate_derivative(independent_variables)))&&
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(temp_function=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(function_matrix_resize->matrix_private->derivative(
			independent_variables)))&&(temp_variable=temp_function->output())&&
			(temp_variable->evaluate())&&(temp_variable=temp_function->matrix(
			independent_variables))&&(derivative=boost::dynamic_pointer_cast<
			Function_matrix<Scalar>,Function>(temp_variable->get_value()))&&
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(number_of_rows*number_of_columns==
			derivative->number_of_rows()))
		{
			Function_size_type dependent_row,i,j;

			number_of_independent_values=derivative->number_of_columns();
			if (0==row_private)
			{
				if (0==column_private)
				{
					result=derivative;
				}
				else
				{
					Matrix result_matrix(number_of_rows,number_of_independent_values);

					dependent_row=column_private;
					for (i=0;i<number_of_rows;++i)
					{
						for (j=1;j<=number_of_independent_values;++j)
						{
							result_matrix(i,j-1)=(*derivative)(dependent_row,j);
						}
						dependent_row += number_of_columns;
					}
					result=Function_handle(new Function_matrix<Scalar>(result_matrix));
				}
			}
			else
			{
				if (0==column_private)
				{
					Matrix result_matrix(number_of_columns,number_of_independent_values);

					dependent_row=(row_private-1)*number_of_columns+1;
					for (i=0;i<number_of_columns;++i)
					{
						for (j=1;j<=number_of_independent_values;++j)
						{
							result_matrix(i,j-1)=(*derivative)(dependent_row,j);
						}
						++dependent_row;
					}
					result=Function_handle(new Function_matrix<Scalar>(result_matrix));
				}
				else
				{
					Matrix result_matrix(1,number_of_independent_values);

					dependent_row=(row_private-1)*number_of_columns+column_private;
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
// class Function_derivatnew_matrix_resize
// ---------------------------------------

class Function_derivatnew_matrix_resize : public Function_derivatnew
//******************************************************************************
// LAST MODIFIED : 22 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// for construction exception
		class Construction_exception {};
		// constructor
		Function_derivatnew_matrix_resize(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables);
		// destructor
		~Function_derivatnew_matrix_resize();
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

Function_derivatnew_matrix_resize::Function_derivatnew_matrix_resize(
	const Function_variable_handle& dependent_variable,
	const std::list<Function_variable_handle>& independent_variables):
	Function_derivatnew(dependent_variable,independent_variables)
//******************************************************************************
// LAST MODIFIED : 22 April 2005
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix_resize<Scalar> >
		function_matrix_resize;
	boost::intrusive_ptr< Function_variable_matrix_resize<Scalar> >
		variable_matrix_resize;

	if ((variable_matrix_resize=boost::dynamic_pointer_cast<
		Function_variable_matrix_resize<Scalar>,Function_variable>(
		dependent_variable))&&(function_matrix_resize=
		boost::dynamic_pointer_cast<Function_matrix_resize<Scalar>,Function>(
		dependent_variable->function())))
	{
		if (matrix_derivative=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(function_matrix_resize->matrix_private->derivative(
			independent_variables)))
		{
			matrix_derivative->add_dependent_function(this);
		}
		else
		{
			throw Function_derivatnew_matrix_resize::Construction_exception();
		}
	}
	else
	{
		throw Function_derivatnew_matrix_resize::Construction_exception();
	}
}

Function_derivatnew_matrix_resize::~Function_derivatnew_matrix_resize()
//******************************************************************************
// LAST MODIFIED : 22 April 2005
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
Function_handle Function_variable_matrix_resize<Scalar>::derivative(
	const std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 20 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_handle(new Function_derivatnew_matrix_resize(
		Function_variable_handle(this),independent_variables)));
}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
template<>
Function_handle Function_variable_matrix_resize<Scalar>::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_derivatnew_matrix_resize::evaluate(Function_variable_handle
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
		boost::intrusive_ptr< Function_matrix_resize<Scalar> >
			function_matrix_resize;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		boost::intrusive_ptr< Function_variable_matrix_resize<Scalar> >
			function_variable_matrix_resize;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		result=false;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		if (
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(function_variable_matrix_resize=boost::dynamic_pointer_cast<
			Function_variable_matrix_resize<Scalar>,Function_variable>(
			dependent_variable))&&
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(function_matrix_resize=boost::dynamic_pointer_cast<
			Function_matrix_resize<Scalar>,Function>(
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			dependent_variable->
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			function()))&&(0<independent_variables.size()))
		{
			boost::intrusive_ptr< Function_matrix<Scalar> > matrix;
			Function_size_type number_of_columns,number_of_independent_values,
				number_of_rows,size;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			boost::intrusive_ptr< Function_matrix<Scalar> > derivative;
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			Function_size_type column_private,row_private;
			Function_variable_handle temp_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

			if (
#if defined (EVALUATE_RETURNS_VALUE)
				(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_resize->matrix_private->evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
				(function_matrix_resize->matrix_private->evaluate)()&&
				(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_resize->matrix_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
				(0<(size=(matrix->number_of_columns())*(matrix->number_of_rows())))&&
				(0<(number_of_columns=
				function_matrix_resize->number_of_columns_private))&&
				(0==size%number_of_columns)&&
				(
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				row_private
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				=function_variable_matrix_resize->row())
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				<=(number_of_rows=size/number_of_columns))&&
				(
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				column_private
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				=function_variable_matrix_resize->column())
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				<=number_of_columns)&&
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(derivative=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_resize->matrix_private->
				evaluate_derivative(independent_variables)))&&
				(number_of_rows*number_of_columns==derivative->number_of_rows())
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(temp_variable=matrix_derivative->output())&&
				(temp_variable->evaluate())
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
						result=derivative;
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						derivative_matrix=matrix_derivative->derivative_matrix;
						result=true;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					}
					else
					{
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						Matrix result_matrix(number_of_rows,number_of_independent_values);

						dependent_row=column_private;
						for (i=0;i<number_of_rows;++i)
						{
							for (j=1;j<=number_of_independent_values;++j)
							{
								result_matrix(i,j-1)=(*derivative)(dependent_row,j);
							}
							dependent_row += number_of_columns;
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

							dependent_row=column_private-1;
							for (i=0;i<number_of_rows;++i)
							{
								for (j=0;j<number_of_independent_values;++j)
								{
									result_matrix(i,j)=derivative(dependent_row,j);
								}
								dependent_row += number_of_columns;
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

						dependent_row=(row_private-1)*number_of_columns+1;
						for (i=0;i<number_of_columns;++i)
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
							Matrix result_matrix(number_of_columns,
								(number_of_independent_values=derivative.size2()));

							dependent_row=(row_private-1)*number_of_columns;
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
					else
					{
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						Matrix result_matrix(1,number_of_independent_values);

						dependent_row=(row_private-1)*number_of_columns+column_private;
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

							dependent_row=(row_private-1)*number_of_columns+column_private-1;
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
bool Function_matrix_resize<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 22 April 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_resize<Scalar> >
		atomic_variable_matrix_resize;

	result=false;
	if ((atomic_variable_matrix_resize=boost::dynamic_pointer_cast<
		Function_variable_matrix_resize<Scalar>,Function_variable>(
		atomic_variable))&&equivalent(Function_handle(this),
		atomic_variable_matrix_resize->function())&&
		(0<atomic_variable_matrix_resize->row())&&
		(0<atomic_variable_matrix_resize->column())&&
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
			Function_matrix<Scalar>,Function>(atomic_variable_matrix_resize->
			evaluate_derivative(atomic_independent_variables)))&&
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(derivative_function=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(atomic_variable_matrix_resize->derivative(
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
