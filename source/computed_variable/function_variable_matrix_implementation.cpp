//******************************************************************************
// FILE : function_variable_matrix_implementation.cpp
//
// LAST MODIFIED : 13 January 2005
//
// DESCRIPTION :
//==============================================================================

#include <sstream>

#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable_matrix.hpp"
#include "computed_variable/function_variable_value_specific.hpp"


// module classes
// ==============

// class Function_variable_iterator_representation_atomic_matrix
// -------------------------------------------------------------

EXPORT template<typename Value_type>
class Function_variable_iterator_representation_atomic_matrix :
	public Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 1 September 2004
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
					if (0==atomic_variable->row_private)
					{
						atomic_variable->row_private=1;
					}
					if (0==atomic_variable->column_private)
					{
						atomic_variable->column_private=1;
					}
					if ((atomic_variable->row_private<=variable->number_of_rows())&&
						(atomic_variable->column_private<=variable->number_of_columns()))
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
				if (0==variable->column_private)
				{
					(atomic_variable->column_private)++;
					if (atomic_variable->column_private>(variable->number_of_columns)())
					{
						if (0==variable->row_private)
						{
							(atomic_variable->column_private)=1;
							(atomic_variable->row_private)++;
							if (atomic_variable->row_private>(variable->number_of_rows)())
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
					if (0==variable->row_private)
					{
						(atomic_variable->row_private)++;
						if (atomic_variable->row_private>(variable->number_of_rows)())
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
					if (0==variable->column_private)
					{
						(atomic_variable->column_private)--;
						if (atomic_variable->column_private<1)
						{
							if (0==variable->row_private)
							{
								atomic_variable->column_private=(variable->number_of_columns)();
								(atomic_variable->row_private)--;
								if (atomic_variable->row_private<1)
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
						if (0==variable->row_private)
						{
							(atomic_variable->row_private)--;
							if (atomic_variable->row_private<1)
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
						if (0==atomic_variable->row_private)
						{
							atomic_variable->row_private=number_of_rows_local;
						}
						if (0==atomic_variable->column_private)
						{
							atomic_variable->column_private=number_of_columns_local;
						}
						if ((atomic_variable->row_private<=number_of_rows_local)&&
							(atomic_variable->column_private<=number_of_columns_local))
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
				result=
					equivalent(atomic_variable,representation_matrix->atomic_variable);
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
Function_variable_matrix<Value_type>::Function_variable_matrix(
	const Function_handle function):Function_variable(function),column_private(0),
	row_private(0)
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	// do nothing
}

EXPORT template<typename Value_type>
Function_variable_matrix<Value_type>::Function_variable_matrix(
	const Function_handle function,const Function_size_type row,
	const Function_size_type column):Function_variable(function),
	column_private(column),row_private(row)
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (function)
	{
		if ((0!=this->row_private)&&(0!=this->column_private))
		{
			value_private=Function_variable_value_handle(
				new Function_variable_value_specific<Value_type>(
				Function_variable_matrix_set_value_function<Value_type>));
		}
	}
	else
	{
		this->row_private=0;
		this->column_private=0;
	}
}

EXPORT template<typename Value_type>
Function_variable_handle Function_variable_matrix<Value_type>::clone() const
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(
		new Function_variable_matrix<Value_type>(*this)));
}

EXPORT template<typename Value_type>
string_handle Function_variable_matrix<Value_type>::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 1 September 2004
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
		if (0==row_private)
		{
			out << "1:" << number_of_rows();
		}
		else
		{
			out << row_private;
		}
		out << ",";
		if (0==column_private)
		{
			out << "1:" << number_of_columns();
		}
		else
		{
			out << column_private;
		}
		out << "]";
		*return_string=out.str();
	}

	return (return_string);
}

EXPORT template<typename Value_type>
#if defined (EVALUATE_RETURNS_VALUE)
Function_handle Function_variable_matrix<Value_type>::evaluate()
//******************************************************************************
// LAST MODIFIED : 3 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle function_local,result(0);

	if (function_local=function())
	{
		if (0<row_private)
		{
			if (0<column_private)
			{
				ublas::matrix<Value_type,ublas::column_major> temp_matrix(1,1);

				if ((function_local->evaluate)(Function_variable_handle(this))&&
					get_entry(temp_matrix(0,0)))
				{
					result=Function_handle(new Function_matrix<Value_type>(temp_matrix));
				}
			}
			else
			{
				bool valid;
				boost::intrusive_ptr< Function_variable_matrix<Value_type> >
					temp_variable;
				Function_size_type j,number_of_columns=this->number_of_columns();
				ublas::matrix<Value_type,ublas::column_major>
					temp_matrix(1,number_of_columns);

				valid=true;
				j=0;
				while (valid&&(j<number_of_columns))
				{
					valid=((temp_variable=(*this)(row_private,j+1))&&
						(function_local->evaluate)(temp_variable)&&
						(temp_variable->get_entry(temp_matrix(0,j))));
					j++;
				}
				if (valid)
				{
					result=Function_handle(new Function_matrix<Value_type>(temp_matrix));
				}
			}
		}
		else
		{
			if (0<column_private)
			{
				bool valid;
				boost::intrusive_ptr< Function_variable_matrix<Value_type> >
					temp_variable;
				Function_size_type i,number_of_rows=this->number_of_rows();
				ublas::matrix<Value_type,ublas::column_major>
					temp_matrix(number_of_rows,1);

				valid=true;
				i=0;
				while (valid&&(i<number_of_rows))
				{
					valid=((temp_variable=(*this)(i+1,column_private))&&
						(function_local->evaluate)(temp_variable)&&
						(temp_variable->get_entry(temp_matrix(i,0))));
					i++;
				}
				if (valid)
				{
					result=Function_handle(new Function_matrix<Value_type>(temp_matrix));
				}
			}
			else
			{
				bool valid;
				boost::intrusive_ptr< Function_variable_matrix<Value_type> >
					temp_variable;
				Function_size_type i,j,number_of_columns=this->number_of_columns(),
					number_of_rows=this->number_of_rows();
				ublas::matrix<Value_type,ublas::column_major>
					temp_matrix(number_of_rows,number_of_columns);

				valid=true;
				i=0;
				while (valid&&(i<number_of_rows))
				{
					j=0;
					while (valid&&(j<number_of_columns))
					{
						valid=((temp_variable=(*this)(i+1,j+1))&&
							(function_local->evaluate)(temp_variable)&&
							(temp_variable->get_entry(temp_matrix(i,j))));
						j++;
					}
					i++;
				}
				if (valid)
				{
					result=Function_handle(new Function_matrix<Value_type>(temp_matrix));
				}
			}
		}
	}

	return (result);
}
#else // defined (EVALUATE_RETURNS_VALUE)
bool Function_variable_matrix<Value_type>::evaluate()
//******************************************************************************
// LAST MODIFIED : 13 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_handle function_local;

	result=false;
	if (function_local=function())
	{
		if (0<row_private)
		{
			if (0<column_private)
			{
				result=(function_local->evaluate)(Function_variable_handle(this));
			}
			else
			{
				boost::intrusive_ptr< Function_variable_matrix<Value_type> >
					temp_variable;
				Function_size_type j,number_of_columns=this->number_of_columns();

				result=true;
				j=0;
				while (result&&(j<number_of_columns))
				{
					result=(temp_variable=(*this)(row_private,j+1))&&
						(function_local->evaluate)(temp_variable);
					j++;
				}
			}
		}
		else
		{
			if (0<column_private)
			{
				boost::intrusive_ptr< Function_variable_matrix<Value_type> >
					temp_variable;
				Function_size_type i,number_of_rows=this->number_of_rows();

				result=true;
				i=0;
				while (result&&(i<number_of_rows))
				{
					result=(temp_variable=(*this)(i+1,column_private))&&
						(function_local->evaluate)(temp_variable);
					i++;
				}
			}
			else
			{
				boost::intrusive_ptr< Function_variable_matrix<Value_type> >
					temp_variable;
				Function_size_type i,j,number_of_columns=this->number_of_columns(),
					number_of_rows=this->number_of_rows();

				result=true;
				i=0;
				while (result&&(i<number_of_rows))
				{
					j=0;
					while (result&&(j<number_of_columns))
					{
						result=(temp_variable=(*this)(i+1,j+1))&&
							(function_local->evaluate)(temp_variable);
						j++;
					}
					i++;
				}
			}
		}
	}

	return (result);
}
#endif // defined (EVALUATE_RETURNS_VALUE)

EXPORT template<typename Value_type>
Function_handle Function_variable_matrix<Value_type>::get_value() const
//******************************************************************************
// LAST MODIFIED : 3 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);

	if (0<row_private)
	{
		if (0<column_private)
		{
			ublas::matrix<Value_type,ublas::column_major> temp_matrix(1,1);

			if (get_entry(temp_matrix(0,0)))
			{
				result=Function_handle(new Function_matrix<Value_type>(temp_matrix));
			}
		}
		else
		{
			bool valid;
			boost::intrusive_ptr< Function_variable_matrix<Value_type> >
				temp_variable;
			Function_size_type j,number_of_columns=this->number_of_columns();
			ublas::matrix<Value_type,ublas::column_major>
				temp_matrix(1,number_of_columns);

			valid=true;
			j=0;
			while (valid&&(j<number_of_columns))
			{
				valid=((temp_variable=(*this)(row_private,j+1))&&
					(temp_variable->get_entry(temp_matrix(0,j))));
				j++;
			}
			if (valid)
			{
				result=Function_handle(new Function_matrix<Value_type>(temp_matrix));
			}
		}
	}
	else
	{
		if (0<column_private)
		{
			bool valid;
			boost::intrusive_ptr< Function_variable_matrix<Value_type> >
				temp_variable;
			Function_size_type i,number_of_rows=this->number_of_rows();
			ublas::matrix<Value_type,ublas::column_major>
				temp_matrix(number_of_rows,1);

			valid=true;
			i=0;
			while (valid&&(i<number_of_rows))
			{
				valid=((temp_variable=(*this)(i+1,column_private))&&
					(temp_variable->get_entry(temp_matrix(i,0))));
				i++;
			}
			if (valid)
			{
				result=Function_handle(new Function_matrix<Value_type>(temp_matrix));
			}
		}
		else
		{
			bool valid;
			boost::intrusive_ptr< Function_variable_matrix<Value_type> >
				temp_variable;
			Function_size_type i,j,number_of_columns=this->number_of_columns(),
				number_of_rows=this->number_of_rows();
			ublas::matrix<Value_type,ublas::column_major>
				temp_matrix(number_of_rows,number_of_columns);

			valid=true;
			i=0;
			while (valid&&(i<number_of_rows))
			{
				j=0;
				while (valid&&(j<number_of_columns))
				{
					valid=((temp_variable=(*this)(i+1,j+1))&&
						(temp_variable->get_entry(temp_matrix(i,j))));
					j++;
				}
				i++;
			}
			if (valid)
			{
				result=Function_handle(new Function_matrix<Value_type>(temp_matrix));
			}
		}
	}

	return (result);
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
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_size_type result;

	result=0;
	if (0==row_private)
	{
		if (0==column_private)
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
		if (0==column_private)
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
Scalar Function_variable_matrix<Value_type>::norm() const
//******************************************************************************
// LAST MODIFIED : 6 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool valid;
	boost::intrusive_ptr< Function_variable_matrix<Value_type> >
		temp_variable;
	Function_size_type i,j,number_of_columns,number_of_rows;
	Scalar result;
	Value_type sum,value;

	number_of_rows=this->number_of_rows();
	number_of_columns=this->number_of_columns();
	valid=true;
	sum=0;
	i=1;
	while (valid&&(i<=number_of_rows))
	{
		j=1;
		while (valid&&(j<=number_of_columns))
		{
			if ((temp_variable=(*this)(i,j))&&
				(temp_variable->get_entry(value)))
			{
				sum += value*value;
			}
			else
			{
				valid=false;
			}
			j++;
		}
		i++;
	}
	if (valid)
	{
		result=std::sqrt((Scalar)sum);
	}
	else
	{
		result= -1;
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_variable_handle Function_variable_matrix<Value_type>::operator-(
	const Function_variable& second) const
//******************************************************************************
// LAST MODIFIED : 17 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_handle result(0);

	try
	{
		const Function_variable_matrix<Value_type>& second_matrix=
			dynamic_cast<const Function_variable_matrix<Value_type>&>(second);
		Function_size_type number_of_columns,number_of_rows;

		number_of_rows=this->number_of_rows();
		number_of_columns=this->number_of_columns();
		if ((number_of_rows==second_matrix.number_of_rows())&&
			(number_of_columns==second_matrix.number_of_columns()))
		{
			bool valid;
			boost::intrusive_ptr< Function_matrix<Value_type> > temp_function;
			boost::intrusive_ptr< Function_variable_matrix<Value_type> >
				temp_variable;
			Function_size_type i,j;
			ublas::matrix<Value_type,ublas::column_major>
				temp_matrix(number_of_rows,number_of_columns);
			Value_type value_1,value_2;

			valid=true;
			i=0;
			while (valid&&(i<number_of_rows))
			{
				j=0;
				while (valid&&(j<number_of_columns))
				{
					if ((temp_variable=(*this)(i+1,j+1))&&
						(temp_variable->get_entry(value_1))&&
						(temp_variable=second_matrix(i+1,j+1))&&
						(temp_variable->get_entry(value_2)))
					{
						temp_matrix(i,j)=value_1-value_2;
					}
					else
					{
						valid=false;
					}
					j++;
				}
				i++;
			}
			if (valid&&(temp_function=new Function_matrix<Value_type>(temp_matrix)))
			{
				result=temp_function->output();
			}
		}
	}
	catch (std::bad_cast)
	{
		// general case
		Function_size_type number_of_columns,number_of_rows;

		number_of_rows=this->number_of_rows();
		number_of_columns=this->number_of_columns();
		if ((0<number_of_rows)&&(0<number_of_columns))
		{
			bool valid;
			boost::intrusive_ptr< Function_matrix<Value_type> > temp_function;
			boost::intrusive_ptr< Function_variable_matrix<Value_type> >
				temp_variable;
			Function_size_type i,j;
			Function_variable_iterator second_iterator,second_iterator_end;
			boost::intrusive_ptr< Function_variable_value_specific<Value_type> >
				variable_value;
			ublas::matrix<Value_type,ublas::column_major>
				temp_matrix(number_of_rows,number_of_columns);
			Value_type value_1,value_2;

			valid=true;
			i=0;
			second_iterator=second.begin_atomic();
			second_iterator_end=second.end_atomic();
			while (valid&&(i<number_of_rows))
			{
				j=0;
				while (valid&&(j<number_of_columns))
				{
					if ((temp_variable=(*this)(i+1,j+1))&&
						(temp_variable->get_entry(value_1))&&
						(second_iterator!=second_iterator_end)&&
						(variable_value=boost::dynamic_pointer_cast<
						Function_variable_value_specific<Value_type>,
						Function_variable_value>((*second_iterator)->value()))&&
						(variable_value->set(value_2,*second_iterator)))
					{
						temp_matrix(i,j)=value_1-value_2;
						second_iterator++;
					}
					else
					{
						valid=false;
					}
					j++;
				}
				i++;
			}
			if (valid&&(second_iterator==second_iterator_end)&&
				(temp_function=new Function_matrix<Value_type>(temp_matrix)))
			{
				result=temp_function->output();
			}
		}
	}

	return (result);
}

EXPORT template<typename Value_type>
boost::intrusive_ptr< Function_variable_matrix<Value_type> >
	Function_variable_matrix<Value_type>::operator()(Function_size_type row,
	Function_size_type column) const
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix<Value_type> > function_matrix;
	boost::intrusive_ptr< Function_variable_matrix<Value_type> > result(0);

	if ((function_matrix=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
		Function>(function_private))&&(row<=number_of_rows())&&
		(column<=number_of_columns()))
	{
		result=boost::intrusive_ptr< Function_variable_matrix<Value_type> >(
			new Function_variable_matrix<Value_type>(function_matrix,row,column));
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_size_type Function_variable_matrix<Value_type>::number_of_rows() const
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix<Value_type> > function_matrix;
	Function_size_type result;

	result=0;
	if (function_matrix=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
		Function>(function_private))
	{
		result=function_matrix->number_of_rows();
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_size_type Function_variable_matrix<Value_type>::number_of_columns()
	const
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix<Value_type> > function_matrix;
	Function_size_type result;

	result=0;
	if (function_matrix=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
		Function>(function_private))
	{
		result=function_matrix->number_of_columns();
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_size_type Function_variable_matrix<Value_type>::row() const
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (row_private);
}

EXPORT template<typename Value_type>
Function_size_type Function_variable_matrix<Value_type>::column() const
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (column_private);
}

EXPORT template<typename Value_type>
bool Function_variable_matrix<Value_type>::get_entry(Value_type& value) const
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_matrix<Value_type> > function_matrix;

	result=false;
	if ((function_matrix=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
		Function>(function_private))&&(0<row_private)&&
		(row_private<=number_of_rows())&&(0<column_private)&&
		(column_private<=number_of_columns()))
	{
		result=true;
		value=(*function_matrix)(row_private,column_private);
	}

	return (result);
}

EXPORT template<typename Value_type>
bool Function_variable_matrix<Value_type>::equality_atomic(
	const Function_variable_handle& variable) const
//******************************************************************************
// LAST MODIFIED : 1 September 2004
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
		result=equivalent(function_private,variable_matrix->function())&&
			(row_private==variable_matrix->row_private)&&
			(column_private==variable_matrix->column_private)&&
			(typeid(*this)==typeid(*variable_matrix));
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_variable_matrix<Value_type>::Function_variable_matrix(
	const Function_variable_matrix<Value_type>& variable):
	Function_variable(variable),column_private(variable.column_private),
	row_private(variable.row_private)
//******************************************************************************
// LAST MODIFIED : 1 September 2004
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
