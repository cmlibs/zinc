//******************************************************************************
// FILE : function_matrix_sum_implementation.cpp
//
// LAST MODIFIED : 3 September 2004
//
// DESCRIPTION :
//==============================================================================

#include <sstream>

#include "computed_variable/function_matrix_sum.hpp"
#include "computed_variable/function_variable.hpp"
#include "computed_variable/function_variable_matrix.hpp"
#include "computed_variable/function_variable_union.hpp"
#include "computed_variable/function_variable_value_specific.hpp"


// module classes
// ==============

// class Function_variable_matrix_sum
// ----------------------------------

EXPORT template<typename Value_type>
class Function_variable_matrix_sum : public Function_variable_matrix<Value_type>
//******************************************************************************
// LAST MODIFIED : 3 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_matrix<Value_type>;
	public:
		// constructor
		Function_variable_matrix_sum(
			const boost::intrusive_ptr< Function_matrix_sum<Value_type> >
			function_matrix_sum):
			Function_variable_matrix<Value_type>(function_matrix_sum){};
		Function_variable_matrix_sum(
			const boost::intrusive_ptr< Function_matrix_sum<Value_type> >
			function_matrix_sum,const Function_size_type row,
			const Function_size_type column):Function_variable_matrix<Value_type>(
			function_matrix_sum,row,column){};
		~Function_variable_matrix_sum(){}
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_matrix_sum<Value_type>(*this)));
		};
		//???DB.  Should operator() and get_entry do an evaluate?
		boost::intrusive_ptr< Function_variable_matrix<Value_type> > operator()(
			Function_size_type row=1,Function_size_type column=1) const
		{
			boost::intrusive_ptr< Function_matrix_sum<Value_type> >
				function_matrix_sum;
			boost::intrusive_ptr< Function_variable_matrix<Value_type> > result(0);

			if ((function_matrix_sum=boost::dynamic_pointer_cast<
				Function_matrix_sum<Value_type>,Function>(function_private))&&
				(row<=number_of_rows())&&(column<=number_of_columns()))
			{
				result=boost::intrusive_ptr< Function_variable_matrix<Value_type> >(
					new Function_variable_matrix_sum<Value_type>(function_matrix_sum,row,
					column));
			}

			return (result);
		}
	private:
		// copy constructor
		Function_variable_matrix_sum(
			const Function_variable_matrix_sum<Value_type>& variable):
			Function_variable_matrix<Value_type>(variable){};
};


// global classes
// ==============

// class Function_matrix_sum
// -------------------------

EXPORT template<typename Value_type>
ublas::matrix<Value_type,ublas::column_major> 
	Function_matrix_sum<Value_type>::constructor_values(0,0);

EXPORT template<typename Value_type>
Function_matrix_sum<Value_type>::Function_matrix_sum(
	const Function_variable_handle& sumand_1,
	const Function_variable_handle& sumand_2):Function_matrix<Value_type>(
	Function_matrix_sum<Value_type>::constructor_values),summand_1_private(0),
	summand_2_private(0)
//******************************************************************************
// LAST MODIFIED : 2 September 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	boost::intrusive_ptr< Function_variable_matrix<Value_type> > matrix_1,
		matrix_2;

	if ((matrix_1=boost::dynamic_pointer_cast<
		Function_variable_matrix<Value_type>,Function_variable>(sumand_1))&&
		(matrix_2=boost::dynamic_pointer_cast<Function_variable_matrix<Value_type>,
		Function_variable>(sumand_2)))
	{
		Function_size_type number_of_columns,number_of_rows;

		number_of_rows=matrix_1->number_of_rows();
		number_of_columns=matrix_1->number_of_columns();
		if ((matrix_2->number_of_rows()==number_of_rows)&&
			(matrix_2->number_of_columns()==number_of_columns))
		{
			summand_1_private=matrix_1;
			summand_2_private=matrix_2;
			values.resize(number_of_rows,number_of_columns);
		}
	}
}

EXPORT template<typename Value_type>
Function_matrix_sum<Value_type>::~Function_matrix_sum()
//******************************************************************************
// LAST MODIFIED : 30 August 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

EXPORT template<typename Value_type>
string_handle Function_matrix_sum<Value_type>::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 30 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		out << *(summand_1_private->get_string_representation());
		out << "+";
		out << *(summand_2_private->get_string_representation());
		*return_string=out.str();
	}

	return (return_string);
}

EXPORT template<typename Value_type>
Function_variable_handle Function_matrix_sum<Value_type>::input()
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle function;
	Function_variable_handle input_1(0),input_2(0),result(0);

	if (summand_1_private&&(function=summand_1_private->function()))
	{
		input_1=function->input();
	}
	if (summand_2_private&&(function=summand_2_private->function()))
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
Function_variable_handle Function_matrix_sum<Value_type>::output()
//******************************************************************************
// LAST MODIFIED : 2 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(
		new Function_variable_matrix_sum<Value_type>(
		boost::intrusive_ptr< Function_matrix_sum<Value_type> >(this))));
}

EXPORT template<typename Value_type>
bool Function_matrix_sum<Value_type>::operator==(const Function& function) const
//******************************************************************************
// LAST MODIFIED : 2 September 2004
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
			const Function_matrix_sum<Value_type>& function_matrix_sum=
				dynamic_cast<const Function_matrix_sum<Value_type>&>(function);

			result=
				equivalent(summand_1_private,function_matrix_sum.summand_1_private)&&
				equivalent(summand_2_private,function_matrix_sum.summand_2_private);
		}
		catch (std::bad_cast)
		{
			// do nothing
		}
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_handle Function_matrix_sum<Value_type>::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 3 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr< Function_variable_matrix<Value_type> >
		atomic_variable_matrix_sum;
	Function_handle result(0);
	Function_size_type column,row;

	if (atomic_variable&&
		equivalent(Function_handle(this),(atomic_variable->function)())&&
		(atomic_variable_matrix_sum=boost::dynamic_pointer_cast<
		Function_variable_matrix_sum<Value_type>,Function_variable>(
		atomic_variable))&&(0<(row=atomic_variable_matrix_sum->row()))&&
		(0<(column=atomic_variable_matrix_sum->column())))
	{
		boost::intrusive_ptr< Function_variable_matrix<Value_type> >
			temp_variable;
		ublas::matrix<Value_type,ublas::column_major> result_matrix(1,1);
		Value_type value_1,value_2;

		if ((temp_variable=(*summand_1_private)(row,column))&&
			(temp_variable->evaluate())&&(temp_variable->get_entry(value_1))&&
			(temp_variable=(*summand_2_private)(row,column))&&
			(temp_variable->evaluate())&&(temp_variable->get_entry(value_2)))
		{
			result_matrix(0,0)=value_1+value_2;
			values(row-1,column-1)=result_matrix(0,0);
			result=Function_handle(new Function_matrix<Value_type>(result_matrix));
		}
	}

	return (result);
}

EXPORT template<typename Value_type>
bool Function_matrix_sum<Value_type>::evaluate_derivative(Scalar&,
	Function_variable_handle,std::list<Function_variable_handle>&)
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (false);
}

template<>
bool Function_matrix_sum<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables);

EXPORT template<typename Value_type>
bool Function_matrix_sum<Value_type>::set_value(
	Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 2 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_sum<Value_type> >
		atomic_matrix_variable;
	boost::intrusive_ptr< Function_variable_value_specific<Value_type> >
		value_type;
	Function_handle function;

	result=false;
	if ((atomic_matrix_variable=boost::dynamic_pointer_cast<
		Function_variable_matrix_sum<Value_type>,Function_variable>(
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
		if (function=summand_1_private->function())
		{
			result=function->set_value(atomic_variable,atomic_value);
		}
		if (!result)
		{
			if (function=summand_2_private->function())
			{
				result=function->set_value(atomic_variable,atomic_value);
			}
		}
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_handle Function_matrix_sum<Value_type>::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 2 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle function,result(0);
	boost::intrusive_ptr< Function_variable_matrix_sum<Value_type> >
		atomic_variable_matrix_sum;
	ublas::matrix<Value_type,ublas::column_major> result_matrix(1,1);

	if (atomic_variable&&
		equivalent(Function_handle(this),(atomic_variable->function)())&&
		(atomic_variable_matrix_sum=boost::dynamic_pointer_cast<
		Function_variable_matrix_sum<Value_type>,Function_variable>(
		atomic_variable))&&
		(atomic_variable_matrix_sum->get_entry)(result_matrix(0,0)))
	{
		result=Function_handle(new Function_matrix<Value_type>(result_matrix));
	}
	if (!result)
	{
		if (function=summand_1_private->function())
		{
			result=function->get_value(atomic_variable);
		}
		if (!result)
		{
			if (function=summand_2_private->function())
			{
				result=function->get_value(atomic_variable);
			}
		}
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_matrix_sum<Value_type>::Function_matrix_sum(
	const Function_matrix_sum<Value_type>& function_matrix_sum):
	Function_matrix<Value_type>(function_matrix_sum){}
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================

EXPORT template<typename Value_type>
Function_matrix_sum<Value_type>& Function_matrix_sum<Value_type>::operator=(
	const Function_matrix_sum<Value_type>& function_matrix_sum)
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	this->values=function_matrix_sum.values;

	return (*this);
}
