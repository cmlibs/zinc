//******************************************************************************
// FILE : function_matrix_product_implementation.cpp
//
// LAST MODIFIED : 1 October 2004
//
// DESCRIPTION :
//==============================================================================

#include <sstream>

#include "computed_variable/function_matrix_product.hpp"
#include "computed_variable/function_variable.hpp"
#include "computed_variable/function_variable_matrix.hpp"
#include "computed_variable/function_variable_union.hpp"
#include "computed_variable/function_variable_value_specific.hpp"


// module classes
// ==============

// class Function_variable_matrix_product
// --------------------------------------

EXPORT template<typename Value_type>
class Function_variable_matrix_product :
	public Function_variable_matrix<Value_type>
//******************************************************************************
// LAST MODIFIED : 1 October 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_matrix<Value_type>;
	public:
		// constructor
		Function_variable_matrix_product(
			const boost::intrusive_ptr< Function_matrix_product<Value_type> >
			function_matrix_product):
			Function_variable_matrix<Value_type>(function_matrix_product){};
		Function_variable_matrix_product(
			const boost::intrusive_ptr< Function_matrix_product<Value_type> >
			function_matrix_product,const Function_size_type row,
			const Function_size_type column):Function_variable_matrix<Value_type>(
			function_matrix_product,row,column){};
		~Function_variable_matrix_product(){}
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_matrix_product<Value_type>(*this)));
		};
		Function_handle evaluate()
		{
			Function_handle result(0);
			boost::intrusive_ptr< Function_matrix_product<Value_type> >
				function_matrix_product;

			if (function_matrix_product=boost::dynamic_pointer_cast<
				Function_matrix_product<Value_type>,Function>(function()))
			{
				Function_size_type number_of_columns,number_of_rows,number_in_sum;
				boost::intrusive_ptr< Function_matrix<Value_type> > multiplicand,
					multiplier;

				if ((multiplier=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
					Function>(function_matrix_product->multiplier_private->evaluate()))&&
					(multiplicand=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
					Function>(function_matrix_product->multiplicand_private->
					evaluate()))&&
					(row_private<=(number_of_rows=multiplier->number_of_rows()))&&
					((number_in_sum=multiplier->number_of_columns())==
					multiplicand->number_of_rows())&&
					(column_private<=(number_of_columns=multiplicand->
					number_of_columns())))
				{
					Function_size_type i,j,k;
					Value_type sum;

					function_matrix_product->values.resize(number_of_rows,
						number_of_columns);
					for (i=1;i<=number_of_rows;i++)
					{
						for (j=1;j<=number_of_columns;j++)
						{
							sum=0;
							for (k=1;k<=number_in_sum;k++)
							{
								sum += (*multiplier)(i,k)*(*multiplicand)(k,j);
							}
							function_matrix_product->values(i-1,j-1)=sum;
						}
					}
					if (0==row_private)
					{
						if (0==column_private)
						{
							result=Function_handle(new Function_matrix<Value_type>(
								function_matrix_product->values));
						}
						else
						{
							ublas::matrix<Value_type,ublas::column_major>
								result_matrix(number_of_rows,1);

							for (i=0;i<number_of_rows;i++)
							{
								result_matrix(i,0)=(function_matrix_product->values)(
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

							for (j=0;j<number_of_columns;j++)
							{
								result_matrix(0,j)=(function_matrix_product->values)(
									row_private-1,j);
							}
							result=Function_handle(new Function_matrix<Value_type>(
								result_matrix));
						}
						else
						{
							ublas::matrix<Value_type,ublas::column_major>
								result_matrix(1,1);
							
							result_matrix(0,0)=(function_matrix_product->values)(
								row_private-1,column_private-1);
							result=Function_handle(new Function_matrix<Value_type>(
								result_matrix));
						}
					}
				}
			}

			return (result);
		};
		Function_handle evaluate_derivative(std::list<Function_variable_handle>&)
		{
			return (0);
		};
		//???DB.  Should operator() and get_entry do an evaluate?
		boost::intrusive_ptr< Function_variable_matrix<Value_type> > operator()(
			Function_size_type row=1,Function_size_type column=1) const
		{
			boost::intrusive_ptr< Function_matrix_product<Value_type> >
				function_matrix_product;
			boost::intrusive_ptr< Function_variable_matrix<Value_type> > result(0);

			if ((function_matrix_product=boost::dynamic_pointer_cast<
				Function_matrix_product<Value_type>,Function>(function_private))&&
				(row<=number_of_rows())&&(column<=number_of_columns()))
			{
				result=boost::intrusive_ptr< Function_variable_matrix<Value_type> >(
					new Function_variable_matrix_product<Value_type>(
					function_matrix_product,row,column));
			}

			return (result);
		}
	private:
		// copy constructor
		Function_variable_matrix_product(
			const Function_variable_matrix_product<Value_type>& variable):
			Function_variable_matrix<Value_type>(variable){};
};


// global classes
// ==============

// class Function_matrix_product
// -----------------------------

EXPORT template<typename Value_type>
ublas::matrix<Value_type,ublas::column_major> 
	Function_matrix_product<Value_type>::constructor_values(0,0);

EXPORT template<typename Value_type>
Function_matrix_product<Value_type>::Function_matrix_product(
	const Function_variable_handle& multiplier,
	const Function_variable_handle& multiplicand):Function_matrix<Value_type>(
	Function_matrix_product<Value_type>::constructor_values),
	multiplicand_private(multiplicand),multiplier_private(multiplier){}
//******************************************************************************
// LAST MODIFIED : 23 September 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================

EXPORT template<typename Value_type>
Function_matrix_product<Value_type>::~Function_matrix_product()
//******************************************************************************
// LAST MODIFIED : 6 September 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

EXPORT template<typename Value_type>
string_handle Function_matrix_product<Value_type>::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 8 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		if (multiplier_private&&multiplicand_private)
		{
			out << *(multiplier_private->get_string_representation());
			out << "*";
			out << *(multiplicand_private->get_string_representation());
		}
		else
		{
			out << "Invalid Function_matrix_product";
		}
		*return_string=out.str();
	}

	return (return_string);
}

EXPORT template<typename Value_type>
Function_variable_handle Function_matrix_product<Value_type>::input()
//******************************************************************************
// LAST MODIFIED : 6 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle function;
	Function_variable_handle input_1(0),input_2(0),result(0);

	if (multiplier_private&&(function=multiplier_private->function()))
	{
		input_1=function->input();
	}
	if (multiplicand_private&&(function=multiplicand_private->function()))
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
Function_variable_handle Function_matrix_product<Value_type>::output()
//******************************************************************************
// LAST MODIFIED : 6 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(
		new Function_variable_matrix_product<Value_type>(
		boost::intrusive_ptr< Function_matrix_product<Value_type> >(this))));
}

EXPORT template<typename Value_type>
bool Function_matrix_product<Value_type>::operator==(
	const Function& function) const
//******************************************************************************
// LAST MODIFIED : 6 September 2004
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
			const Function_matrix_product<Value_type>& function_matrix_product=
				dynamic_cast<const Function_matrix_product<Value_type>&>(function);

			result=equivalent(multiplier_private,
				function_matrix_product.multiplier_private)&&
				equivalent(multiplicand_private,
				function_matrix_product.multiplicand_private);
		}
		catch (std::bad_cast)
		{
			// do nothing
		}
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_handle Function_matrix_product<Value_type>::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 1 October 2004
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr< Function_variable_matrix_product<Value_type> >
		atomic_variable_matrix_product;
	Function_handle result(0);

	if ((atomic_variable_matrix_product=boost::dynamic_pointer_cast<
		Function_variable_matrix_product<Value_type>,Function_variable>(
		atomic_variable))&&equivalent(Function_handle(this),
		atomic_variable_matrix_product->function())&&
		(0<atomic_variable_matrix_product->row())&&
		(0<atomic_variable_matrix_product->column()))
	{
		result=atomic_variable_matrix_product->evaluate();
	}

	return (result);
}

#if defined (OLD_CODE)
EXPORT template<typename Value_type>
Function_handle Function_matrix_product<Value_type>::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 22 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr< Function_variable_matrix<Value_type> >
		atomic_variable_matrix_product;
	Function_handle result(0);
	Function_size_type column,row;

	if (atomic_variable&&
		equivalent(Function_handle(this),(atomic_variable->function)())&&
		(atomic_variable_matrix_product=boost::dynamic_pointer_cast<
		Function_variable_matrix_product<Value_type>,Function_variable>(
		atomic_variable))&&(0<(row=atomic_variable_matrix_product->row()))&&
		(0<(column=atomic_variable_matrix_product->column())))
	{
		boost::intrusive_ptr< Function_matrix<Value_type> > multiplicand,multiplier;
		Function_size_type number_of_columns,number_of_rows,number_in_sum;

		if ((multiplier=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
			Function>(multiplier_private->evaluate()))&&(multiplicand=
			boost::dynamic_pointer_cast<Function_matrix<Value_type>,Function>(
			multiplicand_private->evaluate()))&&
			(row<=(number_of_rows=multiplier->number_of_rows()))&&
			((number_in_sum=multiplier->number_of_columns())==
			multiplicand->number_of_rows())&&
			(column<=(number_of_columns=multiplicand->number_of_columns())))
		{
			Function_size_type copy_number_of_columns,copy_number_of_rows,i,j;
			ublas::matrix<Value_type,ublas::column_major> result_matrix(1,1);
			Value_type sum;

			sum=0;
			if ((number_of_rows!=(copy_number_of_rows=values.size1()))||
				(number_of_columns!=(copy_number_of_columns=values.size2())))
			{
				ublas::matrix<Value_type,ublas::column_major> save_values=values;

				if (number_of_rows<copy_number_of_rows)
				{
					copy_number_of_rows=number_of_rows;
				}
				if (number_of_columns<copy_number_of_columns)
				{
					copy_number_of_columns=number_of_columns;
				}
				values.resize(number_of_rows,number_of_columns);
				for (i=0;i<number_of_rows;i++)
				{
					for (j=0;j<number_of_columns;j++)
					{
						values(i,j)=save_values(i,j);
					}
				}
			}
			for (i=1;i<=number_in_sum;i++)
			{
				sum += (*multiplier)(row,i)*(*multiplicand)(i,column);
			}
			values(row-1,column-1)=sum;
			result_matrix(0,0)=sum;
			result=Function_handle(new Function_matrix<Value_type>(result_matrix));
		}
	}

	return (result);
}
#endif // defined (OLD_CODE)

EXPORT template<typename Value_type>
bool Function_matrix_product<Value_type>::evaluate_derivative(Scalar&,
	Function_variable_handle,std::list<Function_variable_handle>&)
//******************************************************************************
// LAST MODIFIED : 6 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (false);
}

#if !defined (AIX)
template<>
bool Function_matrix_product<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables);
#endif // !defined (AIX)

EXPORT template<typename Value_type>
bool Function_matrix_product<Value_type>::set_value(
	Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 6 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_product<Value_type> >
		atomic_matrix_variable;
	boost::intrusive_ptr< Function_variable_value_specific<Value_type> >
		value_type;
	Function_handle function;

	result=false;
	if ((atomic_matrix_variable=boost::dynamic_pointer_cast<
		Function_variable_matrix_product<Value_type>,Function_variable>(
		atomic_variable))&&
		equivalent(Function_handle(this),atomic_matrix_variable->function())&&
		atomic_value&&(atomic_value->value())&&(value_type=
		boost::dynamic_pointer_cast<Function_variable_value_specific<Value_type>,
		Function_variable_value>(atomic_value->value())))
	{
		result=value_type->set(values((atomic_matrix_variable->row())-1,
			(atomic_matrix_variable->column())-1),atomic_value);
	}
	if (!result)
	{
		if (function=multiplier_private->function())
		{
			result=function->set_value(atomic_variable,atomic_value);
		}
		if (!result)
		{
			if (function=multiplicand_private->function())
			{
				result=function->set_value(atomic_variable,atomic_value);
			}
		}
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_handle Function_matrix_product<Value_type>::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 6 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle function,result(0);
	boost::intrusive_ptr< Function_variable_matrix_product<Value_type> >
		atomic_variable_matrix_product;
	ublas::matrix<Value_type,ublas::column_major> result_matrix(1,1);

	if (atomic_variable&&
		equivalent(Function_handle(this),(atomic_variable->function)())&&
		(atomic_variable_matrix_product=boost::dynamic_pointer_cast<
		Function_variable_matrix_product<Value_type>,Function_variable>(
		atomic_variable))&&
		(atomic_variable_matrix_product->get_entry)(result_matrix(0,0)))
	{
		result=Function_handle(new Function_matrix<Value_type>(result_matrix));
	}
	if (!result)
	{
		if (function=multiplier_private->function())
		{
			result=function->get_value(atomic_variable);
		}
		if (!result)
		{
			if (function=multiplicand_private->function())
			{
				result=function->get_value(atomic_variable);
			}
		}
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_matrix_product<Value_type>::Function_matrix_product(
	const Function_matrix_product<Value_type>& function_matrix_product):
	Function_matrix<Value_type>(function_matrix_product),
	multiplicand_private(function_matrix_product.multiplicand_private),
	multiplier_private(function_matrix_product.multiplier_private){}
//******************************************************************************
// LAST MODIFIED : 6 September 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================

EXPORT template<typename Value_type>
Function_matrix_product<Value_type>& Function_matrix_product<Value_type>::
	operator=(const Function_matrix_product<Value_type>& function_matrix_product)
//******************************************************************************
// LAST MODIFIED : 6 September 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	this->multiplicand_private=function_matrix_product.multiplicand_private;
	this->multiplier_private=function_matrix_product.multiplier_private;
	this->values=function_matrix_product.values;

	return (*this);
}
