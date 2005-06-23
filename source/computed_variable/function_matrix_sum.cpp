//******************************************************************************
// FILE : function_matrix_sum.cpp
//
// LAST MODIFIED : 23 May 2005
//
// DESCRIPTION :
//==============================================================================

#if defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_sum_implementation.cpp"
#else // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_sum.hpp"
#endif // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#if !defined (AIX)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#include "computed_variable/function_derivative.hpp"
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#if defined (OLD_CODE)
template<>
Function_handle Function_variable_matrix_sum<Scalar>::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
// ???DB.  To be done
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix_sum<Scalar> > function_matrix_sum;
	Function_handle result(0);

	if ((function_matrix_sum=boost::dynamic_pointer_cast<
		Function_matrix_sum<Scalar>,Function>(function()))&&
		(0<independent_variables.size()))
	{
		Function_size_type number_of_columns,number_of_dependent_values,
			number_of_independent_values,number_of_rows;
		boost::intrusive_ptr< Function_matrix<Scalar> > derivative_1,derivative_2,
			summand_1,summand_2;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_derivatnew_handle temp_function;
		Function_variable_handle temp_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

		if (
#if defined (EVALUATE_RETURNS_VALUE)
			(summand_1=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_sum->summand_1_private->evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
			(function_matrix_sum->summand_1_private->evaluate)()&&
			(summand_1=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_sum->summand_1_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
#if defined (EVALUATE_RETURNS_VALUE)
			(summand_2=boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
			function_matrix_sum->summand_2_private->evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
			(function_matrix_sum->summand_2_private->evaluate)()&&
			(summand_2=boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
			function_matrix_sum->summand_2_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
			(row_private<=(number_of_rows=summand_1->number_of_rows()))&&
			(number_of_rows==summand_2->number_of_rows())&&
			(column_private<=(number_of_columns=summand_1->number_of_columns()))&&
			(number_of_columns==summand_2->number_of_columns())&&
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(derivative_1=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_sum->summand_1_private->
			evaluate_derivative(independent_variables)))&&
			(derivative_2=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_sum->summand_2_private->
			evaluate_derivative(independent_variables)))&&
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(temp_function=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(function_matrix_sum->summand_1_private->derivative(
			independent_variables)))&&(temp_variable=temp_function->output())&&
			(temp_variable->evaluate())&&(temp_variable=temp_function->matrix(
			independent_variables))&&(derivative_1=boost::dynamic_pointer_cast<
			Function_matrix<Scalar>,Function>(temp_variable->get_value()))&&
			(temp_function=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(function_matrix_sum->summand_2_private->derivative(
			independent_variables)))&&(temp_variable=temp_function->output())&&
			(temp_variable->evaluate())&&(temp_variable=temp_function->matrix(
			independent_variables))&&(derivative_2=boost::dynamic_pointer_cast<
			Function_matrix<Scalar>,Function>(temp_variable->get_value()))&&
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(number_of_rows*number_of_columns==
			(number_of_dependent_values=derivative_1->number_of_rows()))&&
			(number_of_dependent_values==derivative_2->number_of_rows())&&
			(derivative_1->number_of_columns()==(number_of_independent_values=
			derivative_2->number_of_columns())))
		{
			Function_size_type dependent_row,i,j;

			if (0==row_private)
			{
				if (0==column_private)
				{
					for (i=1;i<=number_of_dependent_values;++i)
					{
						for (j=1;j<=number_of_independent_values;++j)
						{
							(*derivative_1)(i,j) += (*derivative_2)(i,j);
						}
					}
					result=derivative_1;
				}
				else
				{
					Matrix result_matrix(number_of_rows,number_of_independent_values);

					dependent_row=column_private;
					for (i=0;i<number_of_rows;++i)
					{
						for (j=1;j<=number_of_independent_values;++j)
						{
							result_matrix(i,j-1)=(*derivative_1)(dependent_row,j)+
								(*derivative_2)(dependent_row,j);
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
							result_matrix(i,j-1)=(*derivative_1)(dependent_row,j)+
								(*derivative_2)(dependent_row,j);
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
						result_matrix(0,j-1)=(*derivative_1)(dependent_row,j)+
							(*derivative_2)(dependent_row,j);
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
// class Function_derivatnew_matrix_sum
// ------------------------------------

class Function_derivatnew_matrix_sum : public Function_derivatnew
//******************************************************************************
// LAST MODIFIED : 21 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// for construction exception
		class Construction_exception {};
		// constructor
		Function_derivatnew_matrix_sum(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables);
		// destructor
		~Function_derivatnew_matrix_sum();
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
		Function_derivatnew_handle derivative_1,derivative_2;
};

Function_derivatnew_matrix_sum::Function_derivatnew_matrix_sum(
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
	boost::intrusive_ptr< Function_matrix_sum<Scalar> > function_matrix_sum;
	boost::intrusive_ptr< Function_variable_matrix_sum<Scalar> >
		variable_matrix_sum;

	if ((variable_matrix_sum=boost::dynamic_pointer_cast<
		Function_variable_matrix_sum<Scalar>,Function_variable>(
		dependent_variable))&&(function_matrix_sum=
		boost::dynamic_pointer_cast<Function_matrix_sum<Scalar>,Function>(
		dependent_variable->function())))
	{
		Function_derivatnew_handle local_derivative_1,local_derivative_2;

		if ((local_derivative_1=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(function_matrix_sum->summand_1_private->derivative(
			independent_variables)))&&
			(local_derivative_2=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(function_matrix_sum->summand_2_private->derivative(
			independent_variables))))
		{
			derivative_1=local_derivative_1;
			derivative_1->add_dependent_function(this);
			derivative_2=local_derivative_2;
			derivative_2->add_dependent_function(this);
		}
		else
		{
			throw Function_derivatnew_matrix_sum::Construction_exception();
		}
	}
	else
	{
		throw Function_derivatnew_matrix_sum::Construction_exception();
	}
}

Function_derivatnew_matrix_sum::~Function_derivatnew_matrix_sum()
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
	if (derivative_1)
	{
		derivative_1->remove_dependent_function(this);
	}
	if (derivative_2)
	{
		derivative_2->remove_dependent_function(this);
	}
#endif // defined (CIRCULAR_SMART_POINTERS)
}

template<>
Function_handle Function_variable_matrix_sum<Scalar>::derivative(
	const std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 20 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_handle(new Function_derivatnew_matrix_sum(
		Function_variable_handle(this),independent_variables)));
}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
template<>
Function_handle Function_variable_matrix_sum<Scalar>::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_derivatnew_matrix_sum::evaluate(Function_variable_handle
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
		boost::intrusive_ptr< Function_matrix_sum<Scalar> > function_matrix_sum;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		boost::intrusive_ptr< Function_variable_matrix_sum<Scalar> >
			variable_matrix_sum;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

		if (
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(variable_matrix_sum=boost::dynamic_pointer_cast<
			Function_variable_matrix_sum<Scalar>,Function_variable>(
			dependent_variable))&&
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(function_matrix_sum=boost::dynamic_pointer_cast<
			Function_matrix_sum<Scalar>,Function>(
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			dependent_variable->
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			function()))&&(0<independent_variables.size()))
		{
			boost::intrusive_ptr< Function_matrix<Scalar> > summand_1,summand_2;
			Function_size_type number_of_columns,number_of_dependent_values,
				number_of_independent_values,number_of_rows;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			boost::intrusive_ptr< Function_matrix<Scalar> > derivative_1,derivative_2;
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			Function_size_type column_private,row_private;
			Function_variable_handle temp_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

			if (
#if defined (EVALUATE_RETURNS_VALUE)
				(summand_1=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_sum->summand_1_private->evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
				(function_matrix_sum->summand_1_private->evaluate)()&&
				(summand_1=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_sum->summand_1_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
#if defined (EVALUATE_RETURNS_VALUE)
				(summand_2=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_sum->summand_2_private->evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
				(function_matrix_sum->summand_2_private->evaluate)()&&
				(summand_2=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_sum->summand_2_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
				(
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				row_private
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				=variable_matrix_sum->row())
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				<=(number_of_rows=summand_1->number_of_rows()))&&
				(number_of_rows==summand_2->number_of_rows())&&
				(
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				column_private
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				=variable_matrix_sum->column())
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				<=(number_of_columns=summand_1->number_of_columns()))&&
				(number_of_columns==summand_2->number_of_columns())&&
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(derivative_1=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_sum->summand_1_private->
				evaluate_derivative(independent_variables)))&&
				(derivative_2=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_sum->summand_2_private->
				evaluate_derivative(independent_variables)))&&
				(number_of_rows*number_of_columns==
				(number_of_dependent_values=derivative_1->number_of_rows()))&&
				(number_of_dependent_values==derivative_2->number_of_rows())&&
				(derivative_1->number_of_columns()==(number_of_independent_values=
				derivative_2->number_of_columns()))
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(temp_variable=derivative_1->output())&&(temp_variable->evaluate())&&
				(temp_variable=derivative_2->output())&&(temp_variable->evaluate())
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				)
			{
				Function_size_type dependent_row,i,j;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				Function_size_type k;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

				if (0==row_private)
				{
					if (0==column_private)
					{
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						for (i=1;i<=number_of_dependent_values;++i)
						{
							for (j=1;j<=number_of_independent_values;++j)
							{
								(*derivative_1)(i,j) += (*derivative_2)(i,j);
							}
						}
						result=derivative_1;
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						std::list<Matrix> matrices;
						std::list<Matrix>::iterator matrix_iterator_1,matrix_iterator_2;

						derivative_matrix.clear();
						matrix_iterator_1=(derivative_1->derivative_matrix).begin();
						matrix_iterator_2=(derivative_2->derivative_matrix).begin();
						for (k=(derivative_1->derivative_matrix).size();k>0;--k)
						{
							Matrix& matrix_derivative_1= *matrix_iterator_1;
							Matrix& matrix_derivative_2= *matrix_iterator_2;
							Matrix result_matrix(
								(number_of_dependent_values=number_of_rows*number_of_columns),
								(number_of_independent_values=matrix_derivative_1.size2()));

							for (i=0;i<number_of_dependent_values;++i)
							{
								for (j=0;j<number_of_independent_values;++j)
								{
									result_matrix(i,j)=matrix_derivative_1(i,j)+
										matrix_derivative_2(i,j);
								}
								dependent_row += number_of_columns;
							}
							matrices.push_back(result_matrix);
							matrix_iterator_1++;
							matrix_iterator_2++;
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

						dependent_row=column_private;
						for (i=0;i<number_of_rows;++i)
						{
							for (j=1;j<=number_of_independent_values;++j)
							{
								result_matrix(i,j-1)=(*derivative_1)(dependent_row,j)+
									(*derivative_2)(dependent_row,j);
							}
							dependent_row += number_of_columns;
						}
						result=Function_handle(new Function_matrix<Scalar>(result_matrix));
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						std::list<Matrix> matrices;
						std::list<Matrix>::iterator matrix_iterator_1,matrix_iterator_2;

						derivative_matrix.clear();
						matrix_iterator_1=(derivative_1->derivative_matrix).begin();
						matrix_iterator_2=(derivative_2->derivative_matrix).begin();
						for (k=(derivative_1->derivative_matrix).size();k>0;--k)
						{
							Matrix& matrix_derivative_1= *matrix_iterator_1;
							Matrix& matrix_derivative_2= *matrix_iterator_2;
							Matrix result_matrix(number_of_rows,
								(number_of_independent_values=matrix_derivative_1.size2()));

							dependent_row=column_private-1;
							for (i=0;i<number_of_rows;++i)
							{
								for (j=0;j<number_of_independent_values;++j)
								{
									result_matrix(i,j)=matrix_derivative_1(dependent_row,j)+
										matrix_derivative_2(dependent_row,j);
								}
								dependent_row += number_of_columns;
							}
							matrices.push_back(result_matrix);
							matrix_iterator_1++;
							matrix_iterator_2++;
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
								result_matrix(i,j-1)=(*derivative_1)(dependent_row,j)+
									(*derivative_2)(dependent_row,j);
							}
							++dependent_row;
						}
						result=Function_handle(new Function_matrix<Scalar>(result_matrix));
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						std::list<Matrix> matrices;
						std::list<Matrix>::iterator matrix_iterator_1,matrix_iterator_2;

						derivative_matrix.clear();
						matrix_iterator_1=(derivative_1->derivative_matrix).begin();
						matrix_iterator_2=(derivative_2->derivative_matrix).begin();
						for (k=(derivative_1->derivative_matrix).size();k>0;--k)
						{
							Matrix& matrix_derivative_1= *matrix_iterator_1;
							Matrix& matrix_derivative_2= *matrix_iterator_2;
							Matrix result_matrix(number_of_rows,
								(number_of_independent_values=matrix_derivative_1.size2()));

							dependent_row=(row_private-1)*number_of_columns;
							for (i=0;i<number_of_rows;++i)
							{
								for (j=0;j<number_of_independent_values;++j)
								{
									result_matrix(i,j)=matrix_derivative_1(dependent_row,j)+
										matrix_derivative_2(dependent_row,j);
								}
								++dependent_row;
							}
							matrices.push_back(result_matrix);
							matrix_iterator_1++;
							matrix_iterator_2++;
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
							result_matrix(0,j-1)=(*derivative_1)(dependent_row,j)+
								(*derivative_2)(dependent_row,j);
						}
						result=Function_handle(new Function_matrix<Scalar>(result_matrix));
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						std::list<Matrix> matrices;
						std::list<Matrix>::iterator matrix_iterator_1,matrix_iterator_2;

						derivative_matrix.clear();
						matrix_iterator_1=(derivative_1->derivative_matrix).begin();
						matrix_iterator_2=(derivative_2->derivative_matrix).begin();
						for (k=(derivative_1->derivative_matrix).size();k>0;--k)
						{
							Matrix& matrix_derivative_1= *matrix_iterator_1;
							Matrix& matrix_derivative_2= *matrix_iterator_2;
							Matrix result_matrix(number_of_rows,
								(number_of_independent_values=matrix_derivative_1.size2()));

							dependent_row=(row_private-1)*number_of_columns+column_private-1;
							for (j=0;j<number_of_independent_values;++j)
							{
								result_matrix(i,j)=matrix_derivative_1(dependent_row,j)+
									matrix_derivative_2(dependent_row,j);
							}
							matrices.push_back(result_matrix);
							matrix_iterator_1++;
							matrix_iterator_2++;
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
bool Function_matrix_sum<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 22 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_sum<Scalar> >
		atomic_variable_matrix_sum;

	result=false;
	if ((atomic_variable_matrix_sum=boost::dynamic_pointer_cast<
		Function_variable_matrix_sum<Scalar>,Function_variable>(
		atomic_variable))&&equivalent(Function_handle(this),
		atomic_variable_matrix_sum->function())&&
		(0<atomic_variable_matrix_sum->row())&&
		(0<atomic_variable_matrix_sum->column())&&
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
			(derivative_value=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(atomic_variable_matrix_sum->evaluate_derivative(
			atomic_independent_variables)))&&
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(derivative_function=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(atomic_variable_matrix_sum->derivative(
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
bool Function_matrix_sum<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_sum<Scalar> >
		atomic_variable_matrix_sum;
	Function_size_type column,row;

	result=false;
	if ((atomic_variable_matrix_sum=boost::dynamic_pointer_cast<
		Function_variable_matrix_sum<Scalar>,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_matrix_sum->function())&&
		(0<(row=atomic_variable_matrix_sum->row()))&&
		(0<(column=atomic_variable_matrix_sum->column())))
	{
		boost::intrusive_ptr< Function_variable_matrix<Scalar> > temp_variable;
		Function_handle function;
		Scalar value_1,value_2;

		if ((temp_variable=(*summand_1_private)(row,column))&&
			(function=temp_variable->function())&&(function->evaluate_derivative)(
			value_1,temp_variable,atomic_independent_variables)&&
			(temp_variable=(*summand_2_private)(row,column))&&
			(function=temp_variable->function())&&(function->evaluate_derivative)(
			value_2,temp_variable,atomic_independent_variables))
		{
			result=true;
			derivative=value_1+value_2;
		}
	}

	return (result);
}
#endif // defined (OLD_CODE)
#endif // !defined (AIX)
