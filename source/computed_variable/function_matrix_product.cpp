//******************************************************************************
// FILE : function_matrix_product.cpp
//
// LAST MODIFIED : 21 April 2005
//
// DESCRIPTION :
//
// DESIGN:
// Function_matrix_product doesn't work when the multiplier or multiplicand is a
//   variable that wraps a matrix variable.  The multiplier and multiplicand
//   need have matrix values, but don't have to be matrix variables.
//
// Possible solutions:
// 1 Make multiplier and multiplicand variables (rather than matrix variables).
//   Tried in function_matrix_product.*.test.  evaluate_derivative (below) needs
//   to use variable_matrix methods on multiplier and multiplicand.
// 2 Check for multiplier or multiplicand that wraps a matrix variable.
//   Not general.  Special case.  What about nested wrappers?
// 3 Move function_matrix_product to function_product.  After evaluating
//   multiplier and multiplicand, use the values operator*
//   How does evaluate_derivative work?
// 4 Extend variable_value to have a matrix value type
//   evaluate_derivative still needs to use variable_matrix methods on
//   multiplier and multiplicand
// 5 Change Function_variable_wrapper.  Currently,
//   - changes a variable's base methods at run time by wrapping
//   - loses a variable's type and extra methods
//   - like envelope/letter in
//     http://www.bell-labs.com/user/cope/Patterns/C++Idioms/EuroPLoP98.html
//   For Function_variable_composition
//   - inherits from Function_variable_wrapper
//   - wraps output_private of the composition function
//   - redefines clone, evaluate and evaluate_derivative
//   Note:
//   - couldn't be a created from output_private
//   - on the fly, would like to be able inherit from wrapper and output_private
//==============================================================================

#if defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_product_implementation.cpp"
#else // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_product.hpp"
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
Function_handle Function_variable_matrix_product<Scalar>::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
// ???DB.  To be done
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix_product<Scalar> >
		function_matrix_product;
	Function_handle result(0);
	Function_size_type order;

	if ((function_matrix_product=boost::dynamic_pointer_cast<
		Function_matrix_product<Scalar>,Function>(function()))&&
		(0<(order=independent_variables.size())))
	{
		Function_size_type number_of_columns,number_of_rows,number_in_sum;
		boost::intrusive_ptr< Function_matrix<Scalar> > multiplicand,
			multiplier;

		if (
#if defined (EVALUATE_RETURNS_VALUE)
			(multiplier=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_product->multiplier_private->evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
			(function_matrix_product->multiplier_private->evaluate)()&&
			(multiplier=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_product->multiplier_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
#if defined (EVALUATE_RETURNS_VALUE)
			(multiplicand=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_product->multiplicand_private->
			evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
			(function_matrix_product->multiplicand_private->evaluate)()&&
			(multiplicand=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_product->multiplicand_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
			(row_private<=(number_of_rows=multiplier->number_of_rows()))&&
			((number_in_sum=multiplier->number_of_columns())==
			multiplicand->number_of_rows())&&
			(column_private<=(number_of_columns=multiplicand->number_of_columns())))
		{
			bool valid;
			Function_variable_handle intermediate_variable(0);
			Function_size_type column_first,column_last,row_first,row_last;
			std::list<Function_variable_handle> intermediate_variables_list(0);

			valid=true;
			if (0==row_private)
			{
				row_first=1;
				row_last=number_of_rows;
				intermediate_variables_list.push_back(
					function_matrix_product->multiplier_private);
			}
			else
			{
				Function_variable_iterator variable_iterator,variable_iterator_end;
				Function_size_type i,j;

				row_first=row_private;
				row_last=row_private;
				i=1;
				variable_iterator=function_matrix_product->
					multiplier_private->begin_atomic();
				variable_iterator_end=function_matrix_product->
					multiplier_private->end_atomic();
				while ((variable_iterator!=variable_iterator_end)&&(i<row_private))
				{
					j=1;
					while ((variable_iterator!=variable_iterator_end)&&
						(j<=number_in_sum))
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
						(j<=number_in_sum))
					{
						intermediate_variables_list.push_back(
							(*variable_iterator)->clone());
						++variable_iterator;
						++j;
					}
					valid=(j>number_in_sum);
				}
			}
			if (0==column_private)
			{
				column_first=1;
				column_last=number_of_columns;
				if (valid)
				{
					intermediate_variables_list.push_back(
						function_matrix_product->multiplicand_private);
				}
			}
			else
			{
				Function_variable_iterator variable_iterator,variable_iterator_end;
				Function_size_type i,j;

				column_first=column_private;
				column_last=column_private;
				variable_iterator=function_matrix_product->
					multiplicand_private->begin_atomic();
				variable_iterator_end=function_matrix_product->
					multiplicand_private->end_atomic();
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
					i=number_in_sum;
					--i;
					while ((variable_iterator!=variable_iterator_end)&&(i>0))
					{
						j=multiplicand->number_of_columns();
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
			if (valid)
			{
				intermediate_variable=new Function_variable_composite(
					intermediate_variables_list);
			}
			number_of_rows=row_last-row_first+1;
			number_of_columns=column_last-column_first+1;
			if (intermediate_variable)
			{
				Function_size_type
					number_of_dependent_values=number_of_rows*number_of_columns,
					number_of_independent_values=(number_of_rows+number_of_columns)*
					number_in_sum;
				Function_derivative_matrix_handle derivative_f(0),derivative_g(0);
				Function_size_type dependent_row,i,independent_column,j,k;
				Matrix Df(number_of_dependent_values,number_of_independent_values),
					D2f(number_of_dependent_values,
					number_of_independent_values*number_of_independent_values);
				std::list<Function_variable_handle>
					intermediate_independent_variables(order,intermediate_variable);
				std::list<Matrix> matrices(0);
				std::list<Matrix>::iterator matrix_iterator,matrix_iterator_end;

				Df.clear();
				dependent_row=0;
				for (i=row_first;i<=row_last;++i)
				{
					for (j=column_first;j<=column_last;++j)
					{
						independent_column=(i-row_first)*number_in_sum;
						for (k=1;k<=number_in_sum;++k)
						{
							Df(dependent_row,independent_column)=(*multiplicand)(k,j);
							++independent_column;
						}
						independent_column=number_of_rows*number_in_sum+j-column_first;
						for (k=1;k<=number_in_sum;++k)
						{
							Df(dependent_row,independent_column)=(*multiplier)(i,k);
							independent_column += number_of_columns;
						}
						++dependent_row;
					}
				}
				if (1<order)
				{
					Function_size_type column_1,column_2,step_1,step_2;

					D2f.clear();
					dependent_row=0;
					step_1=number_of_independent_values+number_of_columns;
					step_2=number_of_columns*number_of_independent_values+1;
					for (i=0;i<number_of_rows;++i)
					{
						for (j=0;j<number_of_columns;++j)
						{
							// multiplier_index=i*number_in_sum+k;
							// multiplicand_index=
							//   number_of_rows*number_in_sum+k*number_of_columns+j;
							// column_1=multiplier_index*number_of_independent_values+
							//   multiplicand_index;
							// column_2=multiplicand_index*number_of_independent_values+
							//   multiplier_index;
							column_1=i*number_in_sum*number_of_independent_values+
								number_of_rows*number_in_sum+j;
							column_2=(number_of_rows*number_in_sum+j)*
								number_of_independent_values+(i*number_in_sum);
							for (k=0;k<number_in_sum;++k)
							{
								D2f(dependent_row,column_1)=1;
								D2f(dependent_row,column_2)=1;
								column_1 += step_1;
								column_2 += step_2;
							}
							++dependent_row;
						}
					}
				}
				for (i=0;i<order;++i)
				{
					matrices.push_back(Df);
					matrix_iterator_end=matrices.end();
					--matrix_iterator_end;
					matrix_iterator=matrices.begin();
					while (matrix_iterator!=matrix_iterator_end)
					{
						if (number_of_independent_values==matrix_iterator->size2())
						{
							matrices.push_back(D2f);
						}
						else
						{
							Matrix zero_matrix(number_of_dependent_values,
								(matrix_iterator->size2())*number_of_independent_values);

							zero_matrix.clear();
							matrices.push_back(zero_matrix);
						}
						++matrix_iterator;
					}
				}
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
					std::cout << "Function_variable_matrix_product<Scalar>::evaluate_derivative.  Failed" << std::endl;
				}
			}
		}
	}

	return (result);
}
#endif // defined (OLD_CODE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_derivatnew_matrix_product
// ----------------------------------------

class Function_derivatnew_matrix_product : public Function_derivatnew
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
		Function_derivatnew_matrix_product(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables);
		// destructor
		~Function_derivatnew_matrix_product();
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

Function_derivatnew_matrix_product::Function_derivatnew_matrix_product(
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
	boost::intrusive_ptr< Function_matrix_product<Scalar> >
		function_matrix_product;
	boost::intrusive_ptr< Function_variable_matrix<Scalar> > multiplicand,
		multiplier;
	boost::intrusive_ptr< Function_variable_matrix_product<Scalar> >
		variable_matrix_product;

	if ((variable_matrix_product=boost::dynamic_pointer_cast<
		Function_variable_matrix_product<Scalar>,Function_variable>(
		dependent_variable))&&(function_matrix_product=
		boost::dynamic_pointer_cast<Function_matrix_product<Scalar>,
		Function>(dependent_variable->function()))&&(multiplicand=
		boost::dynamic_pointer_cast<Function_variable_matrix<Scalar>,
		Function_variable>(function_matrix_product->multiplicand_private))&&
		(multiplier=boost::dynamic_pointer_cast<Function_variable_matrix<Scalar>,
		Function_variable>(function_matrix_product->multiplier_private)))
	{
		bool valid;
		Function_variable_handle intermediate_variable(0);
		Function_size_type column,column_first,column_last,number_of_columns,
			number_of_rows,number_in_sum,row,row_first,row_last;
		std::list<Function_variable_handle> intermediate_variables_list(0);

		valid=true;
		number_of_rows=multiplier->number_of_rows();
		number_in_sum=multiplier->number_of_columns();
		number_of_columns=multiplicand->number_of_columns();
		row=variable_matrix_product->row();
		column=variable_matrix_product->column();
		if (0==row)
		{
			row_first=1;
			row_last=number_of_rows;
			intermediate_variables_list.push_back(
				function_matrix_product->multiplier_private);
		}
		else
		{
			Function_variable_iterator variable_iterator,variable_iterator_end;
			Function_size_type i,j;

			row_first=row;
			row_last=row;
			i=1;
			variable_iterator=function_matrix_product->
				multiplier_private->begin_atomic();
			variable_iterator_end=function_matrix_product->
				multiplier_private->end_atomic();
			while ((variable_iterator!=variable_iterator_end)&&(i<row))
			{
				j=1;
				while ((variable_iterator!=variable_iterator_end)&&
					(j<=number_in_sum))
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
					(j<=number_in_sum))
				{
					intermediate_variables_list.push_back(
						(*variable_iterator)->clone());
					++variable_iterator;
					++j;
				}
				valid=(j>number_in_sum);
			}
		}
		if (0==column)
		{
			column_first=1;
			column_last=number_of_columns;
			if (valid)
			{
				intermediate_variables_list.push_back(
					function_matrix_product->multiplicand_private);
			}
		}
		else
		{
			Function_variable_iterator variable_iterator,variable_iterator_end;
			Function_size_type i,j;

			column_first=column;
			column_last=column;
			variable_iterator=function_matrix_product->
				multiplicand_private->begin_atomic();
			variable_iterator_end=function_matrix_product->
				multiplicand_private->end_atomic();
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
				i=number_in_sum;
				--i;
				while ((variable_iterator!=variable_iterator_end)&&(i>0))
				{
					j=multiplicand->number_of_columns();
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
		if (valid&&(intermediate_variable=Function_variable_handle(
			new Function_variable_composite(intermediate_variables_list)))&&
			(derivative_g=boost::dynamic_pointer_cast<Function_derivatnew,Function>(
			intermediate_variable->derivative(independent_variables))))
		{
			derivative_g->add_dependent_function(this);
		}
		else
		{
			throw Function_derivatnew_matrix_product::Construction_exception();
		}
	}
	else
	{
		throw Function_derivatnew_matrix_product::Construction_exception();
	}
}

Function_derivatnew_matrix_product::
	~Function_derivatnew_matrix_product()
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
Function_handle Function_variable_matrix_product<Scalar>::derivative(
	const std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 20 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_handle(new Function_derivatnew_matrix_product(
		Function_variable_handle(this),independent_variables)));
}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
template<>
Function_handle Function_variable_matrix_product<Scalar>::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_derivatnew_matrix_product::evaluate(Function_variable_handle
#if defined (EVALUATE_RETURNS_VALUE)
	atomic_variable
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
	)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
//******************************************************************************
// LAST MODIFIED : 21 April 2005
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
		boost::intrusive_ptr< Function_matrix_product<Scalar> >
			function_matrix_product;
		Function_size_type order;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		boost::intrusive_ptr< Function_variable_matrix_product<Scalar> >
			variable_matrix_product;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		result=false;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		if (
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(variable_matrix_product=boost::dynamic_pointer_cast<
			Function_variable_matrix_product<Scalar>,Function_variable>(
			dependent_variable))&&
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(function_matrix_product=boost::dynamic_pointer_cast<
			Function_matrix_product<Scalar>,Function>(
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			dependent_variable->
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			function()))&&(0<(order=independent_variables.size())))
		{
			boost::intrusive_ptr< Function_matrix<Scalar> > multiplicand,
				multiplier;
			Function_size_type number_of_columns,number_of_rows,number_in_sum;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			Function_size_type column_private,row_private;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

			if (
#if defined (EVALUATE_RETURNS_VALUE)
				(multiplier=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_product->multiplier_private->evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
				(function_matrix_product->multiplier_private->evaluate)()&&
				(multiplier=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_product->multiplier_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
#if defined (EVALUATE_RETURNS_VALUE)
				(multiplicand=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_product->multiplicand_private->
				evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
				(function_matrix_product->multiplicand_private->evaluate)()&&
				(multiplicand=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_product->multiplicand_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
				(
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				row_private
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				=variable_matrix_product->row())
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				<=(number_of_rows=multiplier->number_of_rows()))&&
				((number_in_sum=multiplier->number_of_columns())==
				multiplicand->number_of_rows())&&
				(
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				column_private
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				=variable_matrix_product->column())
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				<=(number_of_columns=multiplicand->number_of_columns())))
			{
				Function_size_type column_first,column_last,row_first,row_last;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				bool valid;
				Function_variable_handle intermediate_variable(0);
				std::list<Function_variable_handle> intermediate_variables_list(0);
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				valid=true;
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				if (0==row_private)
				{
					row_first=1;
					row_last=number_of_rows;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					intermediate_variables_list.push_back(
						function_matrix_product->multiplier_private);
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				}
				else
				{
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					Function_variable_iterator variable_iterator,variable_iterator_end;
					Function_size_type i,j;
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

					row_first=row_private;
					row_last=row_private;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					i=1;
					variable_iterator=function_matrix_product->
						multiplier_private->begin_atomic();
					variable_iterator_end=function_matrix_product->
						multiplier_private->end_atomic();
					while ((variable_iterator!=variable_iterator_end)&&(i<row_private))
					{
						j=1;
						while ((variable_iterator!=variable_iterator_end)&&
							(j<=number_in_sum))
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
							(j<=number_in_sum))
						{
							intermediate_variables_list.push_back(
								(*variable_iterator)->clone());
							++variable_iterator;
							++j;
						}
						valid=(j>number_in_sum);
					}
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				}
				if (0==column_private)
				{
					column_first=1;
					column_last=number_of_columns;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					if (valid)
					{
						intermediate_variables_list.push_back(
							function_matrix_product->multiplicand_private);
					}
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				}
				else
				{
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					Function_variable_iterator variable_iterator,variable_iterator_end;
					Function_size_type i,j;
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

					column_first=column_private;
					column_last=column_private;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					variable_iterator=function_matrix_product->
						multiplicand_private->begin_atomic();
					variable_iterator_end=function_matrix_product->
						multiplicand_private->end_atomic();
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
						i=number_in_sum;
						--i;
						while ((variable_iterator!=variable_iterator_end)&&(i>0))
						{
							j=multiplicand->number_of_columns();
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
				number_of_rows=row_last-row_first+1;
				number_of_columns=column_last-column_first+1;
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
					Function_size_type
						number_of_dependent_values=number_of_rows*number_of_columns,
						number_of_independent_values=(number_of_rows+number_of_columns)*
						number_in_sum;
					Function_size_type dependent_row,i,independent_column,j,k;
					Matrix Df(number_of_dependent_values,number_of_independent_values),
						D2f(number_of_dependent_values,
						number_of_independent_values*number_of_independent_values);
					std::list<Matrix> matrices(0);
					std::list<Matrix>::iterator matrix_iterator,matrix_iterator_end;

					Df.clear();
					dependent_row=0;
					for (i=row_first;i<=row_last;++i)
					{
						for (j=column_first;j<=column_last;++j)
						{
							independent_column=(i-row_first)*number_in_sum;
							for (k=1;k<=number_in_sum;++k)
							{
								Df(dependent_row,independent_column)=(*multiplicand)(k,j);
								++independent_column;
							}
							independent_column=number_of_rows*number_in_sum+j-column_first;
							for (k=1;k<=number_in_sum;++k)
							{
								Df(dependent_row,independent_column)=(*multiplier)(i,k);
								independent_column += number_of_columns;
							}
							++dependent_row;
						}
					}
					if (1<order)
					{
						Function_size_type column_1,column_2,step_1,step_2;

						D2f.clear();
						dependent_row=0;
						step_1=number_of_independent_values+number_of_columns;
						step_2=number_of_columns*number_of_independent_values+1;
						for (i=0;i<number_of_rows;++i)
						{
							for (j=0;j<number_of_columns;++j)
							{
								// multiplier_index=i*number_in_sum+k;
								// multiplicand_index=
								//   number_of_rows*number_in_sum+k*number_of_columns+j;
								// column_1=multiplier_index*number_of_independent_values+
								//   multiplicand_index;
								// column_2=multiplicand_index*number_of_independent_values+
								//   multiplier_index;
								column_1=i*number_in_sum*number_of_independent_values+
									number_of_rows*number_in_sum+j;
								column_2=(number_of_rows*number_in_sum+j)*
									number_of_independent_values+(i*number_in_sum);
								for (k=0;k<number_in_sum;++k)
								{
									D2f(dependent_row,column_1)=1;
									D2f(dependent_row,column_2)=1;
									column_1 += step_1;
									column_2 += step_2;
								}
								++dependent_row;
							}
						}
					}
					for (i=0;i<order;++i)
					{
						matrices.push_back(Df);
						matrix_iterator_end=matrices.end();
						--matrix_iterator_end;
						matrix_iterator=matrices.begin();
						while (matrix_iterator!=matrix_iterator_end)
						{
							if (number_of_independent_values==matrix_iterator->size2())
							{
								matrices.push_back(D2f);
							}
							else
							{
								Matrix zero_matrix(number_of_dependent_values,
									(matrix_iterator->size2())*number_of_independent_values);

								zero_matrix.clear();
								matrices.push_back(zero_matrix);
							}
							++matrix_iterator;
						}
					}
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
						std::cout << "Function_variable_matrix_product<Scalar>::evaluate_derivative.  Failed" << std::endl;
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
bool Function_matrix_product<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 21 April 2005
//
// DESCRIPTION :
// ???DB.  To be done
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_product<Scalar> >
		atomic_variable_matrix_product;

	result=false;
	if ((atomic_variable_matrix_product=boost::dynamic_pointer_cast<
		Function_variable_matrix_product<Scalar>,Function_variable>(
		atomic_variable))&&equivalent(Function_handle(this),
		atomic_variable_matrix_product->function())&&
		(0<atomic_variable_matrix_product->row())&&
		(0<atomic_variable_matrix_product->column())&&
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
			Function>(atomic_variable_matrix_product->evaluate_derivative(
			atomic_independent_variables)))&&
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(derivative_function=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(atomic_variable_matrix_product->derivative(
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
