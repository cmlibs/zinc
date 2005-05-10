//******************************************************************************
// FILE : function_matrix_divide_by_scalar.cpp
//
// LAST MODIFIED : 20 April 2005
//
// DESCRIPTION :
//
// DESIGN:
//==============================================================================

#if defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_divide_by_scalar_implementation.cpp"
#else // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_divide_by_scalar.hpp"
#endif // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#if !defined (AIX)
#include "computed_variable/function_derivative_matrix.hpp"
#include "computed_variable/function_variable_composite.hpp"

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#include "computed_variable/function_derivative.hpp"
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#if defined (OLD_CODE)
template<>
Function_handle
	Function_variable_matrix_divide_by_scalar<Scalar>::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 20 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix_divide_by_scalar<Scalar> >
		function_matrix_divide_by_scalar;
	Function_handle result(0);
	Function_size_type order;

	if ((function_matrix_divide_by_scalar=boost::dynamic_pointer_cast<
		Function_matrix_divide_by_scalar<Scalar>,Function>(function()))&&
		(0<(order=independent_variables.size())))
	{
		boost::intrusive_ptr< Function_matrix<Scalar> > dividend,divisor;
		Function_size_type number_of_columns,number_of_rows;
		Scalar divisor_value;

		if (
#if defined (EVALUATE_RETURNS_VALUE)
			(dividend=boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
			function_matrix_divide_by_scalar->dividend_private->evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
			(function_matrix_divide_by_scalar->dividend_private->evaluate)()&&
			(dividend=boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
			function_matrix_divide_by_scalar->dividend_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
#if defined (EVALUATE_RETURNS_VALUE)
			(divisor=boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
			function_matrix_divide_by_scalar->divisor_private->evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
			(function_matrix_divide_by_scalar->divisor_private->evaluate)()&&
			(divisor=boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
			function_matrix_divide_by_scalar->divisor_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
			(row_private<=(number_of_rows=dividend->number_of_rows()))&&
			(column_private<=(number_of_columns=dividend->number_of_columns()))&&
			(1==divisor->number_of_rows())&&(1==divisor->number_of_columns())&&
			(0!=(divisor_value=(*divisor)(1,1))))
		{
			bool valid;
			Function_size_type column_first,column_last,i,j,row_first,row_last;
			Function_variable_handle intermediate_variable(0);
			Function_variable_iterator variable_iterator,variable_iterator_end;
			std::list<Function_variable_handle> intermediate_variables_list(0);

			valid=true;
			intermediate_variables_list.push_back(
				function_matrix_divide_by_scalar->divisor_private);
			if (0==row_private)
			{
				row_first=1;
				row_last=number_of_rows;
				if (0==column_private)
				{
					column_first=1;
					column_last=number_of_columns;
					intermediate_variables_list.push_back(
						function_matrix_divide_by_scalar->dividend_private);
				}
				else
				{
					column_first=column_private;
					column_last=column_private;
					variable_iterator=function_matrix_divide_by_scalar->
						dividend_private->begin_atomic();
					variable_iterator_end=function_matrix_divide_by_scalar->
						dividend_private->end_atomic();
					j=1;
					while ((variable_iterator!=variable_iterator_end)&&
						(j<column_private))
					{
						++variable_iterator;
						++j;
					}
					valid=(variable_iterator!=variable_iterator_end);
					if (valid)
					{
						intermediate_variables_list.push_back(
							(*variable_iterator)->clone());
						i=number_of_rows;
						--i;
						while ((variable_iterator!=variable_iterator_end)&&(i>0))
						{
							j=number_of_columns;
							while ((variable_iterator!=variable_iterator_end)&&(j>0))
							{
								++variable_iterator;
								--j;
							}
							valid=(variable_iterator!=variable_iterator_end);
							if (valid)
							{
								intermediate_variables_list.push_back(
									(*variable_iterator)->clone());
							}
							--i;
						}
					}
				}
			}
			else
			{
				row_first=row_private;
				row_last=row_private;
				if (0==column_private)
				{
					column_first=1;
					column_last=number_of_columns;
					i=1;
					variable_iterator=function_matrix_divide_by_scalar->
						dividend_private->begin_atomic();
					variable_iterator_end=function_matrix_divide_by_scalar->
						dividend_private->end_atomic();
					while ((variable_iterator!=variable_iterator_end)&&(i<row_private))
					{
						j=1;
						while ((variable_iterator!=variable_iterator_end)&&
							(j<=number_of_columns))
						{
							++variable_iterator;
							++j;
						}
						++i;
					}
					valid=(variable_iterator!=variable_iterator_end);
					if (valid)
					{
						j=1;
						while ((variable_iterator!=variable_iterator_end)&&
							(j<=number_of_columns))
						{
							intermediate_variables_list.push_back(
								(*variable_iterator)->clone());
							++variable_iterator;
							++j;
						}
						valid=(j>number_of_columns);
					}
				}
				else
				{
					column_first=column_private;
					column_last=column_private;
					i=1;
					variable_iterator=function_matrix_divide_by_scalar->
						dividend_private->begin_atomic();
					variable_iterator_end=function_matrix_divide_by_scalar->
						dividend_private->end_atomic();
					while ((variable_iterator!=variable_iterator_end)&&(i<row_private))
					{
						j=1;
						while ((variable_iterator!=variable_iterator_end)&&
							(j<=number_of_columns))
						{
							++variable_iterator;
							++j;
						}
						++i;
					}
					valid=(variable_iterator!=variable_iterator_end);
					if (valid)
					{
						j=1;
						while ((variable_iterator!=variable_iterator_end)&&
							(j<column_private))
						{
							++variable_iterator;
							++j;
						}
						valid=(variable_iterator!=variable_iterator_end);
						if (valid)
						{
							intermediate_variables_list.push_back(
								(*variable_iterator)->clone());
						}
					}
				}
			}
			if (valid)
			{
				intermediate_variable=new Function_variable_composite(
					intermediate_variables_list);
			}
			if (intermediate_variable)
			{
				Function_derivative_matrix_handle derivative_f(0),derivative_g(0);
				Function_size_type k,l,m,n,number_of_dependent_values,
					number_of_independent_values,number_of_matrices;
				Scalar a,b;
				std::list<Function_variable_handle>
					intermediate_independent_variables(order,intermediate_variable);
				std::list<Matrix> matrices(0);
				std::vector<Matrix> Df(order);

				number_of_rows=row_last-row_first+1;
				number_of_columns=column_last-column_first+1;
				number_of_dependent_values=number_of_rows*number_of_columns;
				number_of_independent_values=number_of_dependent_values+1;
				number_of_matrices=1;
				a=1./divisor_value;
				b= -a*a;
				for (l=0;l<order;++l)
				{
					Matrix& Df_ref=Df[l];

					Df_ref.resize(number_of_dependent_values,
						number_of_independent_values);
					Df_ref.clear();
					k=0;
					for (i=row_first;i<=row_last;++i)
					{
						for (j=column_first;j<=column_last;++j)
						{
							Df_ref(k,0)=b*(*dividend)(i,j);
							n=k+1;
							for (m=0;m<=l;++m)
							{
								Df_ref(k,n)=a;
								n *= number_of_dependent_values+1;
							}
							++k;
						}
					}
					number_of_independent_values *= number_of_dependent_values+1;
					a *= -(Scalar)(l+1)/divisor_value;
					b *= -(Scalar)(l+2)/divisor_value;
					number_of_matrices *= 2;
				}
				number_of_matrices -= 1;
				{
					std::vector<Function_size_type> derivative_order(number_of_matrices);

					k=1;
					n=0;
					for (i=0;i<order;++i)
					{
						matrices.push_back(Df[0]);
						derivative_order[n]=0;
						++n;
						for (j=1;j<k;++j)
						{
							derivative_order[n]=derivative_order[n-k]+1;
							matrices.push_back(Df[derivative_order[n]]);
							++n;
						}
						k *= 2;
					}
				}
#if defined (OLD_CODE)
				k=1;
				n=1;
				for (i=0;i<order;++i)
				{
					for (j=1;j<=k;++j)
					{
						l=n;
						m=0;
						while (l>0)
						{
							if (1==l%2)
							{
								++m;
							}
							l /= 2;
						}
						--m;
						matrices.push_back(Df[m]);
						++n;
					}
					k *= 2;
				}
#endif // defined (OLD_CODE)
				try
				{
					derivative_f=new Function_derivative_matrix(this,
						intermediate_independent_variables,matrices);
					derivative_g=new Function_derivative_matrix(intermediate_variable,
						independent_variables);
					if (derivative_f&&derivative_g)
					{
						Function_derivative_matrix_handle derivative_matrix=
							Function_derivative_matrix_compose(this,derivative_f,
							derivative_g);

						if (derivative_matrix)
						{
							result=derivative_matrix->matrix(independent_variables);
						}
					}
				}
				catch (Function_derivative_matrix::Construction_exception)
				{
					// do nothing
					//???debug
					std::cout << "Function_variable_matrix_divide_by_scalar<Scalar>::evaluate_derivative.  Failed" << std::endl;
				}
			}
		}
	}

	return (result);
}
#endif // defined (OLD_CODE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_derivatnew_matrix_divide_by_scalar
// -------------------------------------------------

class Function_derivatnew_matrix_divide_by_scalar : public Function_derivatnew
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
		Function_derivatnew_matrix_divide_by_scalar(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables);
		// destructor
		~Function_derivatnew_matrix_divide_by_scalar();
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
		Function_derivatnew_handle derivative_g;
};

Function_derivatnew_matrix_divide_by_scalar::
	Function_derivatnew_matrix_divide_by_scalar(
	const Function_variable_handle& dependent_variable,
	const std::list<Function_variable_handle>& independent_variables):
	Function_derivatnew(dependent_variable,independent_variables)
//******************************************************************************
// LAST MODIFIED : 21 April 2005
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix_divide_by_scalar<Scalar> >
		function_matrix_divide_by_scalar;
	boost::intrusive_ptr< Function_variable_matrix_divide_by_scalar<Scalar> >
		variable_matrix_divide_by_scalar;

	if ((variable_matrix_divide_by_scalar=boost::dynamic_pointer_cast<
		Function_variable_matrix_divide_by_scalar<Scalar>,Function_variable>(
		dependent_variable))&&(function_matrix_divide_by_scalar=
		boost::dynamic_pointer_cast<Function_matrix_divide_by_scalar<Scalar>,
		Function>(dependent_variable->function())))
	{
		bool valid;
		Function_size_type column,column_first,column_last,i,j,number_of_columns,
			number_of_rows,row,row_first,row_last;
		Function_variable_handle intermediate_variable(0);
		Function_variable_iterator variable_iterator,variable_iterator_end;
		std::list<Function_variable_handle> intermediate_variables_list(0);

		valid=true;
		number_of_rows=variable_matrix_divide_by_scalar->number_of_rows();
		number_of_columns=variable_matrix_divide_by_scalar->number_of_columns();
		row=variable_matrix_divide_by_scalar->row();
		column=variable_matrix_divide_by_scalar->column();
		intermediate_variables_list.push_back(
			function_matrix_divide_by_scalar->divisor_private);
		if (0==row)
		{
			row_first=1;
			row_last=number_of_rows;
			if (0==column)
			{
				column_first=1;
				column_last=number_of_columns;
				intermediate_variables_list.push_back(
					function_matrix_divide_by_scalar->dividend_private);
			}
			else
			{
				column_first=column;
				column_last=column;
				variable_iterator=function_matrix_divide_by_scalar->
					dividend_private->begin_atomic();
				variable_iterator_end=function_matrix_divide_by_scalar->
					dividend_private->end_atomic();
				j=1;
				while ((variable_iterator!=variable_iterator_end)&&(j<column))
				{
					++variable_iterator;
					++j;
				}
				valid=(variable_iterator!=variable_iterator_end);
				if (valid)
				{
					intermediate_variables_list.push_back(
						(*variable_iterator)->clone());
					i=number_of_rows;
					--i;
					while ((variable_iterator!=variable_iterator_end)&&(i>0))
					{
						j=number_of_columns;
						while ((variable_iterator!=variable_iterator_end)&&(j>0))
						{
							++variable_iterator;
							--j;
						}
						valid=(variable_iterator!=variable_iterator_end);
						if (valid)
						{
							intermediate_variables_list.push_back(
								(*variable_iterator)->clone());
						}
						--i;
					}
				}
			}
		}
		else
		{
			row_first=row;
			row_last=row;
			if (0==column)
			{
				column_first=1;
				column_last=number_of_columns;
				i=1;
				variable_iterator=function_matrix_divide_by_scalar->
					dividend_private->begin_atomic();
				variable_iterator_end=function_matrix_divide_by_scalar->
					dividend_private->end_atomic();
				while ((variable_iterator!=variable_iterator_end)&&(i<row))
				{
					j=1;
					while ((variable_iterator!=variable_iterator_end)&&
						(j<=number_of_columns))
					{
						++variable_iterator;
						++j;
					}
					++i;
				}
				valid=(variable_iterator!=variable_iterator_end);
				if (valid)
				{
					j=1;
					while ((variable_iterator!=variable_iterator_end)&&
						(j<=number_of_columns))
					{
						intermediate_variables_list.push_back(
							(*variable_iterator)->clone());
						++variable_iterator;
						++j;
					}
					valid=(j>number_of_columns);
				}
			}
			else
			{
				column_first=column;
				column_last=column;
				i=1;
				variable_iterator=function_matrix_divide_by_scalar->
					dividend_private->begin_atomic();
				variable_iterator_end=function_matrix_divide_by_scalar->
					dividend_private->end_atomic();
				while ((variable_iterator!=variable_iterator_end)&&(i<row))
				{
					j=1;
					while ((variable_iterator!=variable_iterator_end)&&
						(j<=number_of_columns))
					{
						++variable_iterator;
						++j;
					}
					++i;
				}
				valid=(variable_iterator!=variable_iterator_end);
				if (valid)
				{
					j=1;
					while ((variable_iterator!=variable_iterator_end)&&(j<column))
					{
						++variable_iterator;
						++j;
					}
					valid=(variable_iterator!=variable_iterator_end);
					if (valid)
					{
						intermediate_variables_list.push_back(
							(*variable_iterator)->clone());
					}
				}
			}
		}
		if (valid&&(intermediate_variable=Function_variable_handle(
			new Function_variable_composite(intermediate_variables_list)))&&
			(derivative_g=boost::dynamic_pointer_cast<Function_derivatnew,Function>(
			intermediate_variable->derivative(independent_variables))))
		{
			derivative_g->add_dependent_function(this);
		}
		else
		{
			throw Function_derivatnew_matrix_divide_by_scalar::
				Construction_exception();
		}
	}
	else
	{
		throw Function_derivatnew_matrix_divide_by_scalar::Construction_exception();
	}
}

Function_derivatnew_matrix_divide_by_scalar::
	~Function_derivatnew_matrix_divide_by_scalar()
//******************************************************************************
// LAST MODIFIED : 20 April 2005
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
#if defined (CIRCULAR_SMART_POINTERS)
	// do nothing
#else // defined (CIRCULAR_SMART_POINTERS)
	if (derivative_g)
	{
		derivative_g->remove_dependent_function(this);
	}
#endif // defined (CIRCULAR_SMART_POINTERS)
}

template<>
Function_handle Function_variable_matrix_divide_by_scalar<Scalar>::derivative(
	const std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 20 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_handle(new Function_derivatnew_matrix_divide_by_scalar(
		Function_variable_handle(this),independent_variables)));
}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
template<>
Function_handle
	Function_variable_matrix_divide_by_scalar<Scalar>::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_derivatnew_matrix_divide_by_scalar::evaluate(
	Function_variable_handle
#if defined (EVALUATE_RETURNS_VALUE)
	atomic_variable
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
	)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
//******************************************************************************
// LAST MODIFIED : 20 April 2005
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
		boost::intrusive_ptr< Function_matrix_divide_by_scalar<Scalar> >
			function_matrix_divide_by_scalar;
		Function_size_type order;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		boost::intrusive_ptr< Function_variable_matrix_divide_by_scalar<Scalar> >
			variable_matrix_divide_by_scalar;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		result=false;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		if (
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(variable_matrix_divide_by_scalar=boost::dynamic_pointer_cast<
			Function_variable_matrix_divide_by_scalar<Scalar>,Function_variable>(
			dependent_variable))&&
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(function_matrix_divide_by_scalar=boost::dynamic_pointer_cast<
			Function_matrix_divide_by_scalar<Scalar>,Function>(
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			dependent_variable->
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			function()))&&(0<(order=independent_variables.size())))
		{
			boost::intrusive_ptr< Function_matrix<Scalar> > dividend,divisor;
			Function_size_type number_of_columns,number_of_rows;
			Scalar divisor_value;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			Function_size_type column_private,row_private;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

			if (
#if defined (EVALUATE_RETURNS_VALUE)
				(dividend=boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
				function_matrix_divide_by_scalar->dividend_private->evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
				(function_matrix_divide_by_scalar->dividend_private->evaluate)()&&
				(dividend=boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
				function_matrix_divide_by_scalar->dividend_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
#if defined (EVALUATE_RETURNS_VALUE)
				(divisor=boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
				function_matrix_divide_by_scalar->divisor_private->evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
				(function_matrix_divide_by_scalar->divisor_private->evaluate)()&&
				(divisor=boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
				function_matrix_divide_by_scalar->divisor_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
				(
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				row_private
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				=variable_matrix_divide_by_scalar->row())
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				<=(number_of_rows=dividend->number_of_rows()))&&
				(
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				column_private
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				=variable_matrix_divide_by_scalar->column())
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				<=(number_of_columns=dividend->number_of_columns()))&&
				(1==divisor->number_of_rows())&&(1==divisor->number_of_columns())&&
				(0!=(divisor_value=(*divisor)(1,1))))
			{
				Function_size_type column_first,column_last,i,j,row_first,row_last;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				bool valid;
				Function_variable_handle intermediate_variable(0);
				Function_variable_iterator variable_iterator,variable_iterator_end;
				std::list<Function_variable_handle> intermediate_variables_list(0);
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				valid=true;
				intermediate_variables_list.push_back(
					function_matrix_divide_by_scalar->divisor_private);
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				if (0==row_private)
				{
					row_first=1;
					row_last=number_of_rows;
					if (0==column_private)
					{
						column_first=1;
						column_last=number_of_columns;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						intermediate_variables_list.push_back(
							function_matrix_divide_by_scalar->dividend_private);
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					}
					else
					{
						column_first=column_private;
						column_last=column_private;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						variable_iterator=function_matrix_divide_by_scalar->
							dividend_private->begin_atomic();
						variable_iterator_end=function_matrix_divide_by_scalar->
							dividend_private->end_atomic();
						j=1;
						while ((variable_iterator!=variable_iterator_end)&&
							(j<column_private))
						{
							++variable_iterator;
							++j;
						}
						valid=(variable_iterator!=variable_iterator_end);
						if (valid)
						{
							intermediate_variables_list.push_back(
								(*variable_iterator)->clone());
							i=number_of_rows;
							--i;
							while ((variable_iterator!=variable_iterator_end)&&(i>0))
							{
								j=number_of_columns;
								while ((variable_iterator!=variable_iterator_end)&&(j>0))
								{
									++variable_iterator;
									--j;
								}
								valid=(variable_iterator!=variable_iterator_end);
								if (valid)
								{
									intermediate_variables_list.push_back(
										(*variable_iterator)->clone());
								}
								--i;
							}
						}
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					}
				}
				else
				{
					row_first=row_private;
					row_last=row_private;
					if (0==column_private)
					{
						column_first=1;
						column_last=number_of_columns;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						i=1;
						variable_iterator=function_matrix_divide_by_scalar->
							dividend_private->begin_atomic();
						variable_iterator_end=function_matrix_divide_by_scalar->
							dividend_private->end_atomic();
						while ((variable_iterator!=variable_iterator_end)&&(i<row_private))
						{
							j=1;
							while ((variable_iterator!=variable_iterator_end)&&
								(j<=number_of_columns))
							{
								++variable_iterator;
								++j;
							}
							++i;
						}
						valid=(variable_iterator!=variable_iterator_end);
						if (valid)
						{
							j=1;
							while ((variable_iterator!=variable_iterator_end)&&
								(j<=number_of_columns))
							{
								intermediate_variables_list.push_back(
									(*variable_iterator)->clone());
								++variable_iterator;
								++j;
							}
							valid=(j>number_of_columns);
						}
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					}
					else
					{
						column_first=column_private;
						column_last=column_private;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						i=1;
						variable_iterator=function_matrix_divide_by_scalar->
							dividend_private->begin_atomic();
						variable_iterator_end=function_matrix_divide_by_scalar->
							dividend_private->end_atomic();
						while ((variable_iterator!=variable_iterator_end)&&(i<row_private))
						{
							j=1;
							while ((variable_iterator!=variable_iterator_end)&&
								(j<=number_of_columns))
							{
								++variable_iterator;
								++j;
							}
							++i;
						}
						valid=(variable_iterator!=variable_iterator_end);
						if (valid)
						{
							j=1;
							while ((variable_iterator!=variable_iterator_end)&&
								(j<column_private))
							{
								++variable_iterator;
								++j;
							}
							valid=(variable_iterator!=variable_iterator_end);
							if (valid)
							{
								intermediate_variables_list.push_back(
									(*variable_iterator)->clone());
							}
						}
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					}
				}
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				if (valid)
				{
					intermediate_variable=new Function_variable_composite(
						intermediate_variables_list);
				}
				if (intermediate_variable)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				{
					Function_size_type k,l,m,n,number_of_dependent_values,
						number_of_independent_values,number_of_matrices;
					Scalar a,b;
					std::list<Matrix> matrices(0);
					std::vector<Matrix> Df(order);

					number_of_rows=row_last-row_first+1;
					number_of_columns=column_last-column_first+1;
					number_of_dependent_values=number_of_rows*number_of_columns;
					number_of_independent_values=number_of_dependent_values+1;
					number_of_matrices=1;
					a=1./divisor_value;
					b= -a*a;
					for (l=0;l<order;++l)
					{
						Matrix& Df_ref=Df[l];

						Df_ref.resize(number_of_dependent_values,
							number_of_independent_values);
						Df_ref.clear();
						k=0;
						for (i=row_first;i<=row_last;++i)
						{
							for (j=column_first;j<=column_last;++j)
							{
								Df_ref(k,0)=b*(*dividend)(i,j);
								n=k+1;
								for (m=0;m<=l;++m)
								{
									Df_ref(k,n)=a;
									n *= number_of_dependent_values+1;
								}
								++k;
							}
						}
						number_of_independent_values *= number_of_dependent_values+1;
						a *= -(Scalar)(l+1)/divisor_value;
						b *= -(Scalar)(l+2)/divisor_value;
						number_of_matrices *= 2;
					}
					number_of_matrices -= 1;
					{
						std::vector<Function_size_type>
							derivative_order(number_of_matrices);

						k=1;
						n=0;
						for (i=0;i<order;++i)
						{
							matrices.push_back(Df[0]);
							derivative_order[n]=0;
							++n;
							for (j=1;j<k;++j)
							{
								derivative_order[n]=derivative_order[n-k]+1;
								matrices.push_back(Df[derivative_order[n]]);
								++n;
							}
							k *= 2;
						}
					}
#if defined (OLD_CODE)
					k=1;
					n=1;
					for (i=0;i<order;++i)
					{
						for (j=1;j<=k;++j)
						{
							l=n;
							m=0;
							while (l>0)
							{
								if (1==l%2)
								{
									++m;
								}
								l /= 2;
							}
							--m;
							matrices.push_back(Df[m]);
							++n;
						}
						k *= 2;
					}
#endif // defined (OLD_CODE)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					try
					{
						std::list<Function_variable_handle>
							intermediate_independent_variables(order,intermediate_variable);
						Function_derivative_matrix_handle derivative_f=
							new Function_derivative_matrix(this,
							intermediate_independent_variables,matrices),derivative_g=
							new Function_derivative_matrix(intermediate_variable,
							independent_variables);

						if (derivative_f&&derivative_g)
						{
							Function_derivative_matrix_handle derivative_matrix=
								Function_derivative_matrix_compose(this,derivative_f,
								derivative_g);

							if (derivative_matrix)
							{
								result=derivative_matrix->matrix(independent_variables);
							}
						}
					}
					catch (Function_derivative_matrix::Construction_exception)
					{
						// do nothing
						//???debug
						std::cout << "Function_variable_matrix_divide_by_scalar<Scalar>::evaluate_derivative.  Failed" << std::endl;
					}
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					try
					{
						Derivative_matrix temp_derivative_matrix(matrices);
						Function_variable_handle derivative_g_output;

						if ((derivative_g_output=derivative_g->output())&&
							(derivative_g_output->evaluate()))
						{
							derivative_matrix=
								temp_derivative_matrix*(derivative_g->derivative_matrix);
							result=true;
						}
					}
					catch (Derivative_matrix::Construction_exception)
					{
						// do nothing
						//???debug
						std::cout << "Function_derivatnew_matrix_determinant>::evaluate.  Failed" << std::endl;
					}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
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
bool Function_matrix_divide_by_scalar<Scalar>::evaluate_derivative(
	Scalar& derivative,Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 6 March 2005
//
// DESCRIPTION :
// ???DB.  To be done
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_divide_by_scalar<Scalar> >
		atomic_variable_matrix_divide_by_scalar;

	result=false;
	if ((atomic_variable_matrix_divide_by_scalar=boost::dynamic_pointer_cast<
		Function_variable_matrix_divide_by_scalar<Scalar>,Function_variable>(
		atomic_variable))&&equivalent(Function_handle(this),
		atomic_variable_matrix_divide_by_scalar->function())&&
		(0<atomic_variable_matrix_divide_by_scalar->row())&&
		(0<atomic_variable_matrix_divide_by_scalar->column())&&
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
			Function>(atomic_variable_matrix_divide_by_scalar->evaluate_derivative(
			atomic_independent_variables)))&&
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(derivative_function=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(atomic_variable_matrix_divide_by_scalar->derivative(
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
