//******************************************************************************
// FILE : function_variable_matrix_implementation.cpp
//
// LAST MODIFIED : 20 July 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_VARIABLE_MATRIX_IMPLEMENTATION_CPP__)
#define __FUNCTION_VARIABLE_MATRIX_IMPLEMENTATION_CPP__

#include <sstream>

#include "computed_variable/function_variable_matrix.hpp"
#include "computed_variable/function_variable_value.hpp"


// module classes
// ==============

// class Function_variable_iterator_representation_atomic_matrix
// -------------------------------------------------------------

EXPORT template<typename Value_type>
class Function_variable_iterator_representation_atomic_matrix :
	public Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 13 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_matrix(const bool begin,
			boost::intrusive_ptr< Function_variable_matrix<Value_type> > variable):
			Function_variable_iterator_representation(),atomic_variable(0),
			variable(variable)
		{
			if (begin&&variable)
			{
				if (atomic_variable=boost::dynamic_pointer_cast<
					Function_variable_matrix<Value_type>,Function_variable>(
					variable->clone()))
				{
					if (0==atomic_variable->row)
					{
						atomic_variable->row=1;
					}
					if (0==atomic_variable->column)
					{
						atomic_variable->column=1;
					}
					if ((atomic_variable->row<=variable->number_of_rows())&&
						(atomic_variable->column<=variable->number_of_columns()))
					{
						atomic_variable->value_private=Function_variable_value_handle(
							new Function_variable_value_specific<Value_type>(
							Function_variable_matrix_set_value_function<Value_type>));
					}
					else
					{
						atomic_variable=0;
					}
				}
			}
		};
		// a "virtual" constructor
		Function_variable_iterator_representation *clone()
		{
			Function_variable_iterator_representation *result;

			result=0;
			if (this)
			{
				result=new Function_variable_iterator_representation_atomic_matrix(
					*this);
			}

			return (result);
		};
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_matrix(){};
	private:
		// increment
		void increment()
		{
			if (variable&&atomic_variable)
			{
				if (0==variable->column)
				{
					(atomic_variable->column)++;
					if (atomic_variable->column>(variable->number_of_columns)())
					{
						if (0==variable->row)
						{
							(atomic_variable->column)=1;
							(atomic_variable->row)++;
							if (atomic_variable->row>(variable->number_of_rows)())
							{
								// end
								atomic_variable=0;
							}
						}
						else
						{
							// end
							atomic_variable=0;
						}
					}
				}
				else
				{
					if (0==variable->row)
					{
						(atomic_variable->row)++;
						if (atomic_variable->row>(variable->number_of_rows)())
						{
							// end
							atomic_variable=0;
						}
					}
					else
					{
						// end
						atomic_variable=0;
					}
				}
			}
		};
		// decrement
		void decrement()
		{
			if (variable)
			{
				if (atomic_variable)
				{
					if (0==variable->column)
					{
						(atomic_variable->column)--;
						if (atomic_variable->column<1)
						{
							if (0==variable->row)
							{
								atomic_variable->column=(variable->number_of_columns)();
								(atomic_variable->row)--;
								if (atomic_variable->row<1)
								{
									// end
									atomic_variable=0;
								}
							}
							else
							{
								// end
								atomic_variable=0;
							}
						}
					}
					else
					{
						if (0==variable->row)
						{
							(atomic_variable->row)--;
							if (atomic_variable->row<1)
							{
								// end
								atomic_variable=0;
							}
						}
						else
						{
							// end
							atomic_variable=0;
						}
					}
				}
				else
				{
					if (atomic_variable=boost::dynamic_pointer_cast<
						Function_variable_matrix<Value_type>,Function_variable>(
						variable->clone()))
					{
						Function_size_type number_of_columns_local,number_of_rows_local;

						number_of_rows_local=(variable->number_of_rows)();
						number_of_columns_local=(variable->number_of_columns)();
						if (0==atomic_variable->row)
						{
							atomic_variable->row=number_of_rows_local;
						}
						if (0==atomic_variable->column)
						{
							atomic_variable->column=number_of_columns_local;
						}
						if ((atomic_variable->row<=number_of_rows_local)&&
							(atomic_variable->column<=number_of_columns_local))
						{
							atomic_variable->value_private=Function_variable_value_handle(
								new Function_variable_value_specific<Value_type>(
								Function_variable_matrix_set_value_function<Value_type>));
						}
						else
						{
							atomic_variable=0;
						}
					}
				}
			}
		};
		// equality
		bool equality(
			const Function_variable_iterator_representation* representation)
		{
			bool result;
			const Function_variable_iterator_representation_atomic_matrix
				*representation_matrix=dynamic_cast<
				const Function_variable_iterator_representation_atomic_matrix *>(
				representation);

			result=false;
			if (representation_matrix)
			{
				if (
					((0==atomic_variable)&&(0==representation_matrix->atomic_variable))||
					(atomic_variable&&(representation_matrix->atomic_variable)&&
					(*atomic_variable== *(representation_matrix->atomic_variable))))
				{
					result=true;
				}
			}

			return (result);
		};
		// dereference
		Function_variable_handle dereference() const
		{
			return (atomic_variable);
		};
	private:
		// copy constructor
		Function_variable_iterator_representation_atomic_matrix(const
			Function_variable_iterator_representation_atomic_matrix& representation):
			Function_variable_iterator_representation(),atomic_variable(0),
			variable(representation.variable)
		{
			if (representation.atomic_variable)
			{
				atomic_variable=boost::dynamic_pointer_cast<
					Function_variable_matrix<Value_type>,Function_variable>(
					(representation.atomic_variable)->clone());
			}
		};
	private:
		boost::intrusive_ptr< Function_variable_matrix<Value_type> >
			atomic_variable,variable;
};


// global classes
// ==============

// class Function_variable_matrix
// ------------------------------

EXPORT template<typename Value_type>
string_handle Function_variable_matrix<Value_type>::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 13 July 2004
//
// DESCRIPTION :
//???DB.  Should print values?  How to get?
//==============================================================================
{
	string_handle return_string(0);

	if (return_string=new std::string)
	{
		std::ostringstream out;

		out << "matrix[";
		if (0==row)
		{
			out << "1:" << number_of_rows();
		}
		else
		{
			out << row;
		}
		out << ",";
		if (0==column)
		{
			out << "1:" << number_of_columns();
		}
		else
		{
			out << column;
		}
		out << "]";
		*return_string=out.str();
	}

	return (return_string);
}

EXPORT template<typename Value_type>
Function_variable_iterator Function_variable_matrix<Value_type>::
	begin_atomic() const
//******************************************************************************
// LAST MODIFIED : 13 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_iterator(
		new Function_variable_iterator_representation_atomic_matrix<Value_type>(
		true,boost::intrusive_ptr< Function_variable_matrix<Value_type> >(
		const_cast<Function_variable_matrix<Value_type>*>(this)))));
}

EXPORT template<typename Value_type>
Function_variable_iterator Function_variable_matrix<Value_type>::
	end_atomic() const
//******************************************************************************
// LAST MODIFIED : 13 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_iterator(
		new Function_variable_iterator_representation_atomic_matrix<Value_type>(
		false,boost::intrusive_ptr< Function_variable_matrix<Value_type> >(
		const_cast<Function_variable_matrix<Value_type>*>(this)))));
}

EXPORT template<typename Value_type>
std::reverse_iterator<Function_variable_iterator>
	Function_variable_matrix<Value_type>::rbegin_atomic() const
//******************************************************************************
// LAST MODIFIED : 13 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (std::reverse_iterator<Function_variable_iterator>(
		new Function_variable_iterator_representation_atomic_matrix<Value_type>(
		false,boost::intrusive_ptr< Function_variable_matrix<Value_type> >(
		const_cast<Function_variable_matrix<Value_type>*>(this)))));
}

EXPORT template<typename Value_type>
std::reverse_iterator<Function_variable_iterator>
	Function_variable_matrix<Value_type>::rend_atomic() const
//******************************************************************************
// LAST MODIFIED : 13 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (std::reverse_iterator<Function_variable_iterator>(
		new Function_variable_iterator_representation_atomic_matrix<Value_type>(
		true,boost::intrusive_ptr< Function_variable_matrix<Value_type> >(
		const_cast<Function_variable_matrix<Value_type>*>(this)))));
}

EXPORT template<typename Value_type>
Function_size_type Function_variable_matrix<Value_type>::number_differentiable()
//******************************************************************************
// LAST MODIFIED : 13 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_size_type result;

	result=0;
	if (0==row)
	{
		if (0==column)
		{
			result=number_of_rows()*number_of_columns();
		}
		else
		{
			result=number_of_rows();
		}
	}
	else
	{
		if (0==column)
		{
			result=number_of_columns();
		}
		else
		{
			result=1;
		}
	}

	return (result);
}

EXPORT template<typename Value_type>
bool Function_variable_matrix<Value_type>::equality_atomic(
	const Function_variable_handle& variable) const
//******************************************************************************
// LAST MODIFIED : 13 July 2004
//
// DESCRIPTION :
// Need typeid because want most derived class to be the same (some functions
// use class Function_variable_matrix for more than one variable class)
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix<Value_type> > variable_matrix;

	result=false;
	if (variable_matrix=boost::dynamic_pointer_cast<
		Function_variable_matrix<Value_type>,Function_variable>(variable))
	{
		result=(function_private==variable_matrix->function())&&
			(row==variable_matrix->row)&&(column==variable_matrix->column)&&
			(typeid(*this)==typeid(*variable_matrix));
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_variable_matrix<Value_type>::Function_variable_matrix(
	const Function_handle function):Function_variable(function),column(0),row(0)
//******************************************************************************
// LAST MODIFIED : 13 July 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	// do nothing
#if defined (OLD_CODE)
	if (function)
	{
		if (1==number_of_rows())
		{
			row=1;
		}
		if (1==number_of_columns())
		{
			column=1;
		}
		if ((0!=row)&&(0!=column))
		{
			value_private=Function_variable_value_handle(
				new Function_variable_value_specific<Value_type>(
				Function_variable_matrix_set_value_function<Value_type>));
		}
	}
#endif // defined (OLD_CODE)
}

EXPORT template<typename Value_type>
Function_variable_matrix<Value_type>::Function_variable_matrix(
	const Function_handle function,const Function_size_type row,
	const Function_size_type column):Function_variable(function),column(column),
	row(row)
//******************************************************************************
// LAST MODIFIED : 13 July 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (function)
	{
#if defined (OLD_CODE)
		Function_size_type number_of_columns_local,number_of_rows_local;

		if (1==(number_of_rows_local=number_of_rows()))
		{
			this->row=1;
		}
		else
		{
			if (this->row>number_of_rows_local)
			{
				this->row=0;
			}
		}
		if (1==(number_of_columns_local=number_of_columns()))
		{
			this->column=1;
		}
		else
		{
			if (this->column>number_of_columns_local)
			{
				this->column=0;
			}
		}
#endif // defined (OLD_CODE)
		if ((0!=this->row)&&(0!=this->column))
		{
			value_private=Function_variable_value_handle(
				new Function_variable_value_specific<Value_type>(
				Function_variable_matrix_set_value_function<Value_type>));
		}
	}
	else
	{
		this->row=0;
		this->column=0;
	}
}

EXPORT template<typename Value_type>
Function_variable_matrix<Value_type>::Function_variable_matrix(
	const Function_variable_matrix<Value_type>& variable):
	Function_variable(variable),column(variable.column),row(variable.row)
//******************************************************************************
// LAST MODIFIED : 13 July 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	// do nothing
}

EXPORT template<typename Value_type>
Function_variable_matrix<Value_type>::~Function_variable_matrix()
//******************************************************************************
// LAST MODIFIED : 13 July 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}


// global functions
// ================

EXPORT template<typename Value_type>
bool Function_variable_matrix_set_value_function(Value_type& value,
	const Function_variable_handle variable)
//******************************************************************************
// LAST MODIFIED : 13 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix<Value_type> > matrix_variable;

	result=false;
	if (matrix_variable=boost::dynamic_pointer_cast<
		Function_variable_matrix<Value_type>,Function_variable>(variable))
	{
		result=(matrix_variable->get_entry)(value);
	}

	return (result);
}
#endif // !defined (__FUNCTION_VARIABLE_MATRIX_IMPLEMENTATION_CPP__)
