//******************************************************************************
// FILE : function_matrix_determinant_implementation.cpp
//
// LAST MODIFIED : 13 January 2005
//
// DESCRIPTION :
//==============================================================================

#include <sstream>

#include "computed_variable/function_matrix_determinant.hpp"
#include "computed_variable/function_variable.hpp"
#include "computed_variable/function_variable_matrix.hpp"
#include "computed_variable/function_variable_value_specific.hpp"


// module classes
// ==============

// class Function_variable_matrix_determinant
// ------------------------------------------

EXPORT template<typename Value_type>
class Function_variable_matrix_determinant :
	public Function_variable_matrix<Value_type>
//******************************************************************************
// LAST MODIFIED : 13 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_matrix<Value_type>;
	public:
		// constructor
		Function_variable_matrix_determinant(
			const boost::intrusive_ptr< Function_matrix_determinant<Value_type> >
			function_matrix_determinant):
			Function_variable_matrix<Value_type>(function_matrix_determinant,1,1){};
		~Function_variable_matrix_determinant(){}
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_matrix_determinant<Value_type>(*this)));
		};
#if defined (EVALUATE_RETURNS_VALUE)
		Function_handle evaluate()
		{
			Function_handle result(0);
			boost::intrusive_ptr< Function_matrix_determinant<Value_type> >
				function_matrix_determinant;

			if (function_matrix_determinant=boost::dynamic_pointer_cast<
				Function_matrix_determinant<Value_type>,Function>(function()))
			{
#if defined (BEFORE_CACHING)
				boost::intrusive_ptr< Function_matrix<Value_type> > matrix;

				if ((matrix=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
					Function>(function_matrix_determinant->matrix_private->
					evaluate()))&&
					(matrix->number_of_columns()==matrix->number_of_rows()))
				{
					if (matrix->determinant(function_matrix_determinant->values(0,0)))
					{
						result=Function_handle(new Function_matrix<Value_type>(
							function_matrix_determinant->values));
					}
				}
#else // defined (BEFORE_CACHING)
				if (!(function_matrix_determinant->evaluated()))
				{
					boost::intrusive_ptr< Function_matrix<Value_type> > matrix;

					if ((matrix=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
						Function>(function_matrix_determinant->matrix_private->
						evaluate()))&&
						(matrix->number_of_columns()==matrix->number_of_rows()))
					{
						if (matrix->determinant(function_matrix_determinant->values(0,0)))
						{
							function_matrix_determinant->set_evaluated();
						}
					}
				}
				if (function_matrix_determinant->evaluated())
				{
					result=Function_handle(new Function_matrix<Value_type>(
						function_matrix_determinant->values));
				}
#endif // defined (BEFORE_CACHING)
			}

			return (result);
		};
#else // defined (EVALUATE_RETURNS_VALUE)
		bool evaluate()
		{
			bool result(true);
			boost::intrusive_ptr< Function_matrix_determinant<Value_type> >
				function_matrix_determinant;

			if (function_matrix_determinant=boost::dynamic_pointer_cast<
				Function_matrix_determinant<Value_type>,Function>(function()))
			{
#if defined (BEFORE_CACHING)
				boost::intrusive_ptr< Function_matrix<Value_type> > matrix;

				result=false;
				if ((function_matrix_determinant->matrix_private->evaluate())&&
					(matrix=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
					Function>(function_matrix_determinant->matrix_private->get_value()))&&
					(matrix->number_of_columns()==matrix->number_of_rows()))
				{
					result=matrix->determinant(function_matrix_determinant->values(0,0));
				}
#else // defined (BEFORE_CACHING)
				if (!(function_matrix_determinant->evaluated()))
				{
					boost::intrusive_ptr< Function_matrix<Value_type> > matrix;

					result=false;
					if ((function_matrix_determinant->matrix_private->evaluate())&&
						(matrix=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
						Function>(function_matrix_determinant->matrix_private->
						get_value()))&&
						(matrix->number_of_columns()==matrix->number_of_rows()))
					{
						if (matrix->determinant(function_matrix_determinant->values(0,0)))
						{
							function_matrix_determinant->set_evaluated();
							result=true;
						}
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
			boost::intrusive_ptr< Function_matrix_determinant<Value_type> >
				function_matrix_determinant;
			boost::intrusive_ptr< Function_variable_matrix<Value_type> > result(0);

			if ((function_matrix_determinant=boost::dynamic_pointer_cast<
				Function_matrix_determinant<Value_type>,Function>(function_private))&&
				(row<=number_of_rows())&&(column<=number_of_columns()))
			{
				result=boost::intrusive_ptr< Function_variable_matrix<Value_type> >(
					new Function_variable_matrix_determinant<Value_type>(
					function_matrix_determinant));
			}

			return (result);
		}
	private:
		// copy constructor
		Function_variable_matrix_determinant(
			const Function_variable_matrix_determinant<Value_type>& variable):
			Function_variable_matrix<Value_type>(variable){};
};


// global classes
// ==============

// class Function_matrix_determinant
// ---------------------------------

EXPORT template<typename Value_type>
ublas::matrix<Value_type,ublas::column_major> 
	Function_matrix_determinant<Value_type>::constructor_values(1,1);

EXPORT template<typename Value_type>
Function_matrix_determinant<Value_type>::Function_matrix_determinant(
	const Function_variable_handle& matrix):Function_matrix<Value_type>(
	Function_matrix_determinant<Value_type>::constructor_values),
	matrix_private(matrix)
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
Function_matrix_determinant<Value_type>::~Function_matrix_determinant()
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
string_handle Function_matrix_determinant<Value_type>::
	get_string_representation()
//******************************************************************************
// LAST MODIFIED : 14 September 2004
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
			out << "determinant(";
			out << *(matrix_private->get_string_representation());
			out << ")";
		}
		else
		{
			out << "Invalid Function_matrix_determinant";
		}
		*return_string=out.str();
	}

	return (return_string);
}

EXPORT template<typename Value_type>
Function_variable_handle Function_matrix_determinant<Value_type>::input()
//******************************************************************************
// LAST MODIFIED : 14 September 2004
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
Function_variable_handle Function_matrix_determinant<Value_type>::output()
//******************************************************************************
// LAST MODIFIED : 14 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(
		new Function_variable_matrix_determinant<Value_type>(
		boost::intrusive_ptr< Function_matrix_determinant<Value_type> >(this))));
}

EXPORT template<typename Value_type>
bool Function_matrix_determinant<Value_type>::operator==(
	const Function& function) const
//******************************************************************************
// LAST MODIFIED : 14 September 2004
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
			const Function_matrix_determinant<Value_type>&
				function_matrix_determinant=
				dynamic_cast<const Function_matrix_determinant<Value_type>&>(function);

			result=equivalent(matrix_private,
				function_matrix_determinant.matrix_private);
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
	Function_matrix_determinant<Value_type>::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 13 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr< Function_variable_matrix<Value_type> >
		atomic_variable_matrix_determinant;
#if defined (EVALUATE_RETURNS_VALUE)
	Function_handle result(0);
#else // defined (EVALUATE_RETURNS_VALUE)
	bool result(true);
#endif // defined (EVALUATE_RETURNS_VALUE)

	if (atomic_variable&&
		equivalent(Function_handle(this),(atomic_variable->function)())&&
		(atomic_variable_matrix_determinant=boost::dynamic_pointer_cast<
		Function_variable_matrix_determinant<Value_type>,Function_variable>(
		atomic_variable))&&(1==atomic_variable_matrix_determinant->row())&&
		(1==atomic_variable_matrix_determinant->column()))
	{
		result=atomic_variable_matrix_determinant->evaluate();
	}

	return (result);
}

#if defined (OLD_CODE)
EXPORT template<typename Value_type>
Function_handle Function_matrix_determinant<Value_type>::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 15 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr< Function_variable_matrix<Value_type> >
		atomic_variable_matrix_determinant;
	Function_handle result(0);
	boost::intrusive_ptr<Function_matrix<Value_type> > function_matrix;

	if (atomic_variable&&
		equivalent(Function_handle(this),(atomic_variable->function)())&&
		(atomic_variable_matrix_determinant=boost::dynamic_pointer_cast<
		Function_variable_matrix_determinant<Value_type>,Function_variable>(
		atomic_variable))&&(1==atomic_variable_matrix_determinant->row())&&
		(1==atomic_variable_matrix_determinant->column())&&
		(function_matrix=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
		Function>(matrix_private->evaluate())))
	{
		ublas::matrix<Value_type,ublas::column_major> result_matrix(1,1);

		if (function_matrix->determinant(result_matrix(0,0)))
		{
			values(0,0)=result_matrix(0,0);
			result=Function_handle(new Function_matrix<Value_type>(result_matrix));
		}
	}

	return (result);
}
#endif // defined (OLD_CODE)

EXPORT template<typename Value_type>
bool Function_matrix_determinant<Value_type>::evaluate_derivative(Scalar&,
	Function_variable_handle,std::list<Function_variable_handle>&)
//******************************************************************************
// LAST MODIFIED : 14 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (false);
}

#if !defined (AIX)
template<>
bool Function_matrix_determinant<Scalar>::evaluate_derivative(
	Scalar& derivative,Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables);
#endif // !defined (AIX)

EXPORT template<typename Value_type>
bool Function_matrix_determinant<Value_type>::set_value(
	Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 1 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_determinant<Value_type> >
		atomic_matrix_variable;
	boost::intrusive_ptr< Function_variable_value_specific<Value_type> >
		value_type;
	Function_handle function;

	result=false;
	if ((atomic_matrix_variable=boost::dynamic_pointer_cast<
		Function_variable_matrix_determinant<Value_type>,Function_variable>(
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
Function_handle Function_matrix_determinant<Value_type>::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 14 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle function,result(0);
	boost::intrusive_ptr< Function_variable_matrix_determinant<Value_type> >
		atomic_variable_matrix_determinant;
	ublas::matrix<Value_type,ublas::column_major> result_matrix(1,1);

	if (atomic_variable&&
		equivalent(Function_handle(this),(atomic_variable->function)())&&
		(atomic_variable_matrix_determinant=boost::dynamic_pointer_cast<
		Function_variable_matrix_determinant<Value_type>,Function_variable>(
		atomic_variable))&&
		(atomic_variable_matrix_determinant->get_entry)(result_matrix(0,0)))
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
Function_matrix_determinant<Value_type>::Function_matrix_determinant(
	const Function_matrix_determinant<Value_type>& function_matrix_determinant):
	Function_matrix<Value_type>(function_matrix_determinant),
	matrix_private(function_matrix_determinant.matrix_private)
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
Function_matrix_determinant<Value_type>&
	Function_matrix_determinant<Value_type>::operator=(
	const Function_matrix_determinant<Value_type>& function_matrix_determinant)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	if (function_matrix_determinant.matrix_private)
	{
		function_matrix_determinant.matrix_private->add_dependent_function(this);
	}
	if (matrix_private)
	{
		matrix_private->remove_dependent_function(this);
	}
	matrix_private=function_matrix_determinant.matrix_private;
	values=function_matrix_determinant.values;

	return (*this);
}
