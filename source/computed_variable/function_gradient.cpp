//******************************************************************************
// FILE : function_gradient.cpp
//
// LAST MODIFIED : 26 August 2004
//
// DESCRIPTION :
//==============================================================================

#include <sstream>

#include "computed_variable/function_gradient.hpp"
#include "computed_variable/function_variable.hpp"
#include "computed_variable/function_variable_matrix.hpp"

// module typedefs
// ===============

typedef boost::intrusive_ptr< Function_matrix<Scalar> >
	Function_matrix_scalar_handle;

// module classes
// ==============

// forward declaration so that can use _handle
class Function_variable_gradient;
typedef boost::intrusive_ptr<Function_variable_gradient>
	Function_variable_gradient_handle;

// class Function_variable_iterator_representation_atomic_gradient
// ---------------------------------------------------------------

class Function_variable_iterator_representation_atomic_gradient:
	public Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_gradient(
			const bool begin,Function_variable_gradient_handle variable);
		// a "virtual" constructor
		Function_variable_iterator_representation *clone();
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_gradient();
	private:
		// increment
		void increment();
		// decrement
		void decrement();
		// equality
		bool equality(
			const Function_variable_iterator_representation* representation);
		// dereference
		Function_variable_handle dereference() const;
	private:
		// copy constructor
		Function_variable_iterator_representation_atomic_gradient(const
			Function_variable_iterator_representation_atomic_gradient&);
	private:
		Function_variable_gradient_handle atomic_variable,variable;
		Function_variable_iterator atomic_dependent_iterator,
			atomic_dependent_iterator_begin,atomic_dependent_iterator_end;
		Function_variable_iterator atomic_independent_iterator,
			atomic_independent_iterator_begin,atomic_independent_iterator_end;
};


// class Function_variable_gradient
// ----------------------------------

class Function_variable_gradient : public Function_variable
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_gradient;
	friend class Function_variable_iterator_representation_atomic_gradient;
	public:
		// constructor
		Function_variable_gradient(
			const Function_gradient_handle& function_gradient):
			Function_variable(function_gradient),atomic_dependent_variable(0),
			atomic_independent_variable(0){};
		// constructor.  A zero <atomic_dependent_variable> indicates the whole
		//   gradient
		Function_variable_gradient(
			const Function_gradient_handle& function_gradient,
			Function_variable_handle& atomic_dependent_variable):
			Function_variable(function_gradient),atomic_dependent_variable(0),
			atomic_independent_variable(0)
		{
			if (function_gradient)
			{
				if (atomic_dependent_variable&&
					(1==atomic_dependent_variable->number_differentiable())&&
					(function_gradient->dependent_variable_private)&&
					(function_gradient->dependent_variable_private))
				{
					Function_variable_iterator atomic_dependent_iterator,
						atomic_independent_iterator,end_atomic_dependent_iterator,
						end_atomic_independent_iterator;

					atomic_dependent_iterator=
						function_gradient->dependent_variable_private->begin_atomic();
					end_atomic_dependent_iterator=
						function_gradient->dependent_variable_private->end_atomic();
					while ((atomic_dependent_iterator!=end_atomic_dependent_iterator)&&
						(1!=(*atomic_dependent_iterator)->number_differentiable()))
					{
						atomic_dependent_iterator++;
					}
					atomic_independent_iterator=
						function_gradient->independent_variable_private->begin_atomic();
					end_atomic_independent_iterator=
						function_gradient->independent_variable_private->end_atomic();
					while (
						(atomic_independent_iterator!=end_atomic_independent_iterator)&&
						(1!=(*atomic_independent_iterator)->number_differentiable()))
					{
						atomic_independent_iterator++;
					}
					while ((atomic_dependent_iterator!=end_atomic_dependent_iterator)&&
						(atomic_independent_iterator!=end_atomic_independent_iterator)&&
						!equivalent(*atomic_dependent_iterator,atomic_dependent_variable))
					{
						atomic_dependent_iterator++;
						while ((atomic_dependent_iterator!=end_atomic_dependent_iterator)&&
							(1!=(*atomic_dependent_iterator)->number_differentiable()))
						{
							atomic_dependent_iterator++;
						}
						atomic_independent_iterator++;
						while (
							(atomic_independent_iterator!=end_atomic_independent_iterator)&&
							(1!=(*atomic_independent_iterator)->number_differentiable()))
						{
							atomic_independent_iterator++;
						}
					}
					if ((atomic_dependent_iterator!=end_atomic_dependent_iterator)&&
						(atomic_independent_iterator!=end_atomic_independent_iterator))
					{
						this->atomic_dependent_variable=atomic_dependent_variable;
						this->atomic_independent_variable= *atomic_independent_iterator;
					}
				}
			}
		};
		// destructor
		~Function_variable_gradient(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_gradient_handle(
				new Function_variable_gradient(*this)));
		};
		Function_handle evaluate()
		{
			Function_gradient_handle function_gradient=
				boost::dynamic_pointer_cast<Function_gradient,Function>(function());
			Function_handle result(0);

			if (function_gradient)
			{
				Function_variable_handle dependent_variable,independent_variable;

				independent_variable=atomic_independent_variable;
				if (!(dependent_variable=atomic_dependent_variable))
				{
					dependent_variable=function_gradient->dependent_variable_private;
					independent_variable=function_gradient->independent_variable_private;
				}
				if (dependent_variable&&independent_variable)
				{
					bool valid;
					Function_matrix_scalar_handle derivative_value;
					Function_size_type number_of_derivative_values;
					Function_variable_iterator atomic_dependent_iterator,
						atomic_independent_iterator,end_atomic_dependent_iterator,
						end_atomic_independent_iterator;
					std::list<Function_matrix_scalar_handle> derivative_values;

					atomic_dependent_iterator=dependent_variable->begin_atomic();
					end_atomic_dependent_iterator=dependent_variable->end_atomic();
					while ((atomic_dependent_iterator!=end_atomic_dependent_iterator)&&
						(1!=(*atomic_dependent_iterator)->number_differentiable()))
					{
						atomic_dependent_iterator++;
					}
					atomic_independent_iterator=independent_variable->begin_atomic();
					end_atomic_independent_iterator=independent_variable->end_atomic();
					while (
						(atomic_independent_iterator!=end_atomic_independent_iterator)&&
						(1!=(*atomic_independent_iterator)->number_differentiable()))
					{
						atomic_independent_iterator++;
					}
					valid=true;
					while (valid&&
						(atomic_dependent_iterator!=end_atomic_dependent_iterator)&&
						(atomic_independent_iterator!=end_atomic_independent_iterator))
					{
						std::list<Function_variable_handle> independent_variables(1,
							*atomic_independent_iterator);

						if ((derivative_value=boost::dynamic_pointer_cast<
							Function_matrix<Scalar>,Function>(
							(*atomic_dependent_iterator)->evaluate_derivative(
							independent_variables)))&&
							(1==derivative_value->number_of_rows())&&
							(1==derivative_value->number_of_columns()))
						{
							derivative_values.push_back(derivative_value);
							atomic_dependent_iterator++;
							while (
								(atomic_dependent_iterator!=end_atomic_dependent_iterator)&&
								(1!=(*atomic_dependent_iterator)->number_differentiable()))
							{
								atomic_dependent_iterator++;
							}
							atomic_independent_iterator++;
							while (
								(atomic_independent_iterator!=end_atomic_independent_iterator)&&
								(1!=(*atomic_independent_iterator)->number_differentiable()))
							{
								atomic_independent_iterator++;
							}
						}
						else
						{
							valid=false;
						}
					}
					if (valid&&
						((atomic_dependent_iterator==end_atomic_dependent_iterator)||
						(atomic_independent_iterator==end_atomic_independent_iterator))&&
						(0<(number_of_derivative_values=derivative_values.size())))
					{
						Function_size_type i;
						Matrix result_matrix(number_of_derivative_values,1);
						std::list<Function_matrix_scalar_handle>::iterator
							derivative_value_iterator;

						derivative_value_iterator=derivative_values.begin();
						for (i=0;i<number_of_derivative_values;i++)
						{
							result_matrix(i,0)=(**derivative_value_iterator)(1,1);
							derivative_value_iterator++;
						}
						result=Function_handle(new Function_matrix<Scalar>(result_matrix));
					}
				}
			}

			return (result);
		};
		Function_handle evaluate_derivative(
			std::list<Function_variable_handle>& independent_variables)
		{
			Function_gradient_handle function_gradient=
				boost::dynamic_pointer_cast<Function_gradient,Function>(function());
			Function_handle result(0);

			if (function_gradient)
			{
				Function_variable_handle dependent_variable,independent_variable;

				independent_variable=atomic_independent_variable;
				if (!(dependent_variable=atomic_dependent_variable))
				{
					dependent_variable=function_gradient->dependent_variable_private;
					independent_variable=function_gradient->independent_variable_private;
				}
				if (dependent_variable&&independent_variable)
				{
					bool valid;
					Function_matrix_scalar_handle derivative_value;
					Function_size_type number_of_columns,number_of_rows;
					Function_variable_iterator atomic_dependent_iterator,
						atomic_independent_iterator,end_atomic_dependent_iterator,
						end_atomic_independent_iterator;
					std::list<Function_matrix_scalar_handle> derivative_values;
					std::list<Function_variable_handle> extended_independent_variables;

					extended_independent_variables=independent_variables;
					atomic_dependent_iterator=dependent_variable->begin_atomic();
					end_atomic_dependent_iterator=dependent_variable->end_atomic();
					while ((atomic_dependent_iterator!=end_atomic_dependent_iterator)&&
						(1!=(*atomic_dependent_iterator)->number_differentiable()))
					{
						atomic_dependent_iterator++;
					}
					atomic_independent_iterator=independent_variable->begin_atomic();
					end_atomic_independent_iterator=independent_variable->end_atomic();
					while (
						(atomic_independent_iterator!=end_atomic_independent_iterator)&&
						(1!=(*atomic_independent_iterator)->number_differentiable()))
					{
						atomic_independent_iterator++;
					}
					valid=true;
					while (valid&&
						(atomic_dependent_iterator!=end_atomic_dependent_iterator)&&
						(atomic_independent_iterator!=end_atomic_independent_iterator))
					{
						extended_independent_variables.push_front(
							*atomic_independent_iterator);
						if ((derivative_value=boost::dynamic_pointer_cast<
							Function_matrix<Scalar>,Function>(
							(*atomic_dependent_iterator)->evaluate_derivative(
							extended_independent_variables)))&&
							(1==derivative_value->number_of_rows())&&
							((0==derivative_values.size())||
							(derivative_values.front()->number_of_columns()==
							derivative_value->number_of_columns())))
						{
							derivative_values.push_back(derivative_value);
							atomic_dependent_iterator++;
							while (
								(atomic_dependent_iterator!=end_atomic_dependent_iterator)&&
								(1!=(*atomic_dependent_iterator)->number_differentiable()))
							{
								atomic_dependent_iterator++;
							}
							atomic_independent_iterator++;
							while (
								(atomic_independent_iterator!=end_atomic_independent_iterator)&&
								(1!=(*atomic_independent_iterator)->number_differentiable()))
							{
								atomic_independent_iterator++;
							}
						}
						else
						{
							valid=false;
						}
						extended_independent_variables.pop_front();
					}
					if (valid&&
						((atomic_dependent_iterator==end_atomic_dependent_iterator)||
						(atomic_independent_iterator==end_atomic_independent_iterator))&&
						(0<(number_of_rows=derivative_values.size()))&&
						(0<(number_of_columns=derivative_values.front()->
						number_of_columns())))
					{
						Function_size_type i,j;
						Matrix result_matrix(number_of_rows,number_of_columns);
						std::list<Function_matrix_scalar_handle>::iterator
							derivative_value_iterator;

						derivative_value_iterator=derivative_values.begin();
						for (i=0;i<number_of_rows;i++)
						{
							for (j=0;j<number_of_columns;j++)
							{
								result_matrix(i,j)=(**derivative_value_iterator)(1,j+1);
							}
							derivative_value_iterator++;
						}
						result=Function_handle(new Function_matrix<Scalar>(result_matrix));
					}
				}
			}

			return (result);
		};
		string_handle get_string_representation()
		{
			Function_gradient_handle function_gradient=
				boost::dynamic_pointer_cast<Function_gradient,Function>(function());
			string_handle return_string(0);

			if (return_string=new std::string)
			{
				std::ostringstream out;

				out << "grad(";
				if (atomic_dependent_variable)
				{
					out << *(atomic_dependent_variable->get_string_representation());
				}
				else
				{
					if (function_gradient&&
						(function_gradient->dependent_variable_private))
					{
						out << *(function_gradient->dependent_variable_private->
							get_string_representation());
					}
				}
				out << ",";
				if (atomic_independent_variable)
				{
					out << *(atomic_independent_variable->get_string_representation());
				}
				else
				{
					if (function_gradient&&
						(function_gradient->independent_variable_private))
					{
						out << *(function_gradient->independent_variable_private->
							get_string_representation());
					}
				}
				out << ")";
				*return_string=out.str();
			}

			return (return_string);
		};
		virtual Function_variable_iterator begin_atomic() const
		{
			return (Function_variable_iterator(new
				Function_variable_iterator_representation_atomic_gradient(
				true,Function_variable_gradient_handle(
				const_cast<Function_variable_gradient*>(this)))));
		};
		virtual Function_variable_iterator end_atomic() const
		{
			return (Function_variable_iterator(new
				Function_variable_iterator_representation_atomic_gradient(
				false,Function_variable_gradient_handle(
				const_cast<Function_variable_gradient*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(new
				Function_variable_iterator_representation_atomic_gradient(
				false,Function_variable_gradient_handle(
				const_cast<Function_variable_gradient*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(new
				Function_variable_iterator_representation_atomic_gradient(
				true,Function_variable_gradient_handle(
				const_cast<Function_variable_gradient*>(this)))));
		};
		Function_size_type number_differentiable()
		{
			Function_size_type result;

			result=0;
			if (this)
			{
				Function_gradient_handle function_gradient=
					boost::dynamic_pointer_cast<Function_gradient,Function>(function());
				Function_size_type i;

				if (atomic_dependent_variable)
				{
					result=atomic_dependent_variable->number_differentiable();
				}
				else
				{
					if (function_gradient&&
						(function_gradient->dependent_variable_private))
					{
						result=function_gradient->dependent_variable_private->
							number_differentiable();
					}
				}
				i=0;
				if (atomic_independent_variable)
				{
					i=atomic_independent_variable->number_differentiable();
				}
				else
				{
					if (function_gradient&&
						(function_gradient->independent_variable_private))
					{
						i=function_gradient->independent_variable_private->
							number_differentiable();
					}
				}
				if (i<result)
				{
					result=i;
				}
			}

			return (result);
		};
		bool equality_atomic(const Function_variable_handle& variable) const
		{
			bool result;
			Function_gradient_handle function_gradient=
				boost::dynamic_pointer_cast<Function_gradient,Function>(function());
			Function_variable_gradient_handle variable_gradient;

			result=false;
			if (variable_gradient=boost::dynamic_pointer_cast<
				Function_variable_gradient,Function_variable>(variable))
			{
				if (equivalent(variable_gradient->function(),function_gradient)&&
					equivalent(variable_gradient->atomic_dependent_variable,
					atomic_dependent_variable)&&
					equivalent(variable_gradient->atomic_independent_variable,
					atomic_independent_variable))
				{
					result=true;
				}
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_gradient(
			const Function_variable_gradient& variable_gradient):
			Function_variable(variable_gradient),
			atomic_dependent_variable(variable_gradient.atomic_dependent_variable),
			atomic_independent_variable(
			variable_gradient.atomic_independent_variable){};
		// assignment
		Function_variable_gradient& operator=(const Function_variable_gradient&);
	private:
		// if zero dependent then all
		Function_variable_handle atomic_dependent_variable;
		Function_variable_handle atomic_independent_variable;
};


// class Function_variable_iterator_representation_atomic_gradient
// ---------------------------------------------------------------

Function_variable_iterator_representation_atomic_gradient::
	Function_variable_iterator_representation_atomic_gradient(
	const bool begin,Function_variable_gradient_handle variable):
	atomic_variable(0),variable(variable),atomic_dependent_iterator(0),
	atomic_dependent_iterator_begin(0),atomic_dependent_iterator_end(0),
	atomic_independent_iterator(0),atomic_independent_iterator_begin(0),
	atomic_independent_iterator_end(0)
//******************************************************************************
// LAST MODIFIED : 26 August 2004
//
// DESCRIPTION :
// Constructor.  If <begin> then the constructed iterator points to the first
// atomic variable, otherwise it points to one past the last atomic variable.
//==============================================================================
{
	if (begin&&variable)
	{
		Function_gradient_handle function_gradient=
			boost::dynamic_pointer_cast<Function_gradient,Function>(
			variable->function());

		if (function_gradient&&(atomic_variable=boost::dynamic_pointer_cast<
			Function_variable_gradient,Function_variable>(variable->clone())))
		{
			atomic_dependent_iterator=0;
			atomic_dependent_iterator_begin=0;
			atomic_dependent_iterator_end=0;
			if (variable->atomic_dependent_variable)
			{
				atomic_dependent_iterator_begin=
					variable->atomic_dependent_variable->begin_atomic();
				atomic_dependent_iterator_end=
					variable->atomic_dependent_variable->end_atomic();
			}
			else
			{
				Function_variable_handle dependent_variable_local;

				if (dependent_variable_local=
					function_gradient->dependent_variable_private)
				{
					atomic_dependent_iterator_begin=
						dependent_variable_local->begin_atomic();
					atomic_dependent_iterator_end=
						dependent_variable_local->end_atomic();
				}
				else
				{
					// end
					atomic_variable=0;
				}
			}
			if (atomic_variable&&
				(atomic_dependent_iterator_begin!=atomic_dependent_iterator_end))
			{
				atomic_dependent_iterator=atomic_dependent_iterator_begin;
				while ((atomic_dependent_iterator!=atomic_dependent_iterator_end)&&
					(1!=(*atomic_dependent_iterator)->number_differentiable()))
				{
					atomic_dependent_iterator++;
				}
				if (atomic_dependent_iterator!=atomic_dependent_iterator_end)
				{
					atomic_independent_iterator=0;
					atomic_independent_iterator_begin=0;
					atomic_independent_iterator_end=0;
					if (variable->atomic_independent_variable)
					{
						atomic_independent_iterator_begin=
							variable->atomic_independent_variable->begin_atomic();
						atomic_independent_iterator_end=
							variable->atomic_independent_variable->end_atomic();
					}
					else
					{
						Function_variable_handle independent_variable_local;

						if (independent_variable_local=
							function_gradient->independent_variable_private)
						{
							atomic_independent_iterator_begin=
								independent_variable_local->begin_atomic();
							atomic_independent_iterator_end=
								independent_variable_local->end_atomic();
						}
						else
						{
							// end
							atomic_variable=0;
						}
					}
					if (atomic_variable&&(atomic_independent_iterator_begin!=
						atomic_independent_iterator_end))
					{
						atomic_independent_iterator=atomic_independent_iterator_begin;
						while ((atomic_independent_iterator!=
							atomic_independent_iterator_end)&&
							(1!=(*atomic_independent_iterator)->number_differentiable()))
						{
							atomic_independent_iterator++;
						}
						if (atomic_independent_iterator!=atomic_independent_iterator_end)
						{
							atomic_variable->atomic_dependent_variable=
								*atomic_dependent_iterator;
							atomic_variable->atomic_independent_variable=
								*atomic_independent_iterator;
						}
						else
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
				else
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
			if (!atomic_variable)
			{
				atomic_dependent_iterator=0;
				atomic_dependent_iterator_begin=0;
				atomic_dependent_iterator_end=0;
				atomic_independent_iterator=0;
				atomic_independent_iterator_begin=0;
				atomic_independent_iterator_end=0;
			}
			// variable is an output and cannot be set so leave value_private zero
		}
	}
}

Function_variable_iterator_representation
	*Function_variable_iterator_representation_atomic_gradient::clone()
//******************************************************************************
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_iterator_representation *result;

	result=0;
	if (this)
	{
		result=new Function_variable_iterator_representation_atomic_gradient(
			*this);
	}

	return (result);
}

Function_variable_iterator_representation_atomic_gradient::
	~Function_variable_iterator_representation_atomic_gradient()
//******************************************************************************
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

void Function_variable_iterator_representation_atomic_gradient::increment()
//******************************************************************************
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
// Increments the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		atomic_dependent_iterator++;
		while ((atomic_dependent_iterator!=atomic_dependent_iterator_end)&&
			(1!=(*atomic_dependent_iterator)->number_differentiable()))
		{
			atomic_dependent_iterator++;
		}
		atomic_independent_iterator++;
		while ((atomic_independent_iterator!=atomic_independent_iterator_end)&&
			(1!=(*atomic_independent_iterator)->number_differentiable()))
		{
			atomic_independent_iterator++;
		}
		if ((atomic_dependent_iterator!=atomic_dependent_iterator_end)&&
			(atomic_independent_iterator!=atomic_independent_iterator_end))
		{
			atomic_variable->atomic_dependent_variable= *atomic_dependent_iterator;
			atomic_variable->atomic_independent_variable=
				*atomic_independent_iterator;
		}
		else
		{
			atomic_variable=0;
		}
		if (!atomic_variable)
		{
			atomic_dependent_iterator=0;
			atomic_dependent_iterator_begin=0;
			atomic_dependent_iterator_end=0;
			atomic_independent_iterator=0;
			atomic_independent_iterator_begin=0;
			atomic_independent_iterator_end=0;
		}
	}
}

void Function_variable_iterator_representation_atomic_gradient::decrement()
//******************************************************************************
// LAST MODIFIED : 26 August 2004
//
// DESCRIPTION :
// Decrements the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		if ((atomic_dependent_iterator!=atomic_dependent_iterator_begin)&&
			(atomic_independent_iterator!=atomic_independent_iterator_begin))
		{
			atomic_dependent_iterator--;
			while ((atomic_dependent_iterator!=atomic_dependent_iterator_begin)&&
				(1!=(*atomic_dependent_iterator)->number_differentiable()))
			{
				atomic_dependent_iterator--;
			}
			atomic_independent_iterator--;
			while ((atomic_independent_iterator!=atomic_independent_iterator_begin)&&
				(1!=(*atomic_independent_iterator)->number_differentiable()))
			{
				atomic_independent_iterator--;
			}
			if ((1==(*atomic_dependent_iterator)->number_differentiable())&&
				(1==(*atomic_independent_iterator)->number_differentiable()))
			{
				atomic_variable->atomic_dependent_variable= *atomic_dependent_iterator;
				atomic_variable->atomic_independent_variable=
					*atomic_independent_iterator;
			}
			else
			{
				atomic_variable=0;
			}
		}
		else
		{
			atomic_variable=0;
		}
	}
	else
	{
		if (variable)
		{
			Function_gradient_handle function_gradient=
				boost::dynamic_pointer_cast<Function_gradient,Function>(
				variable->function());

			if (atomic_variable=boost::dynamic_pointer_cast<
				Function_variable_gradient,Function_variable>(variable->clone()))
			{
				atomic_dependent_iterator=0;
				atomic_dependent_iterator_begin=0;
				atomic_dependent_iterator_end=0;
				if (variable->atomic_dependent_variable)
				{
					atomic_dependent_iterator_begin=
						variable->atomic_dependent_variable->begin_atomic();
					atomic_dependent_iterator_end=
						variable->atomic_dependent_variable->end_atomic();
				}
				else
				{
					Function_variable_handle dependent_variable_local;

					if (dependent_variable_local=
						function_gradient->dependent_variable_private)
					{
						atomic_dependent_iterator_begin=
							dependent_variable_local->begin_atomic();
						atomic_dependent_iterator_end=
							dependent_variable_local->end_atomic();
					}
					else
					{
						// end
						atomic_variable=0;
					}
				}
				if (atomic_variable&&
					(atomic_dependent_iterator_begin!=atomic_dependent_iterator_end))
				{
					atomic_dependent_iterator=atomic_dependent_iterator_end;
					atomic_dependent_iterator--;
					while ((atomic_dependent_iterator!=atomic_dependent_iterator_begin)&&
						(1!=(*atomic_dependent_iterator)->number_differentiable()))
					{
						atomic_dependent_iterator--;
					}
					if (1==(*atomic_dependent_iterator)->number_differentiable())
					{
						atomic_independent_iterator=0;
						atomic_independent_iterator_begin=0;
						atomic_independent_iterator_end=0;
						if (variable->atomic_independent_variable)
						{
							atomic_independent_iterator_begin=
								variable->atomic_independent_variable->begin_atomic();
							atomic_independent_iterator_end=
								variable->atomic_independent_variable->end_atomic();
						}
						else
						{
							Function_variable_handle independent_variable_local;

							if (independent_variable_local=
								function_gradient->independent_variable_private)
							{
								atomic_independent_iterator_begin=
									independent_variable_local->begin_atomic();
								atomic_independent_iterator_end=
									independent_variable_local->end_atomic();
							}
							else
							{
								// end
								atomic_variable=0;
							}
						}
						if (atomic_variable&&(atomic_independent_iterator_begin!=
							atomic_independent_iterator_end))
						{
							atomic_independent_iterator=atomic_independent_iterator_end;
							atomic_independent_iterator--;
							while ((atomic_independent_iterator!=
								atomic_independent_iterator_begin)&&
								(1!=(*atomic_independent_iterator)->number_differentiable()))
							{
								atomic_independent_iterator--;
							}
							if (1==(*atomic_independent_iterator)->number_differentiable())
							{
								atomic_variable->atomic_dependent_variable=
									*atomic_dependent_iterator;
								atomic_variable->atomic_independent_variable=
									*atomic_independent_iterator;
							}
							else
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
					else
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
				// variable is an output and cannot be set so leave value_private zero
			}
		}
	}
	if (!atomic_variable)
	{
		atomic_dependent_iterator=0;
		atomic_dependent_iterator_begin=0;
		atomic_dependent_iterator_end=0;
		atomic_independent_iterator=0;
		atomic_independent_iterator_begin=0;
		atomic_independent_iterator_end=0;
	}
}

bool Function_variable_iterator_representation_atomic_gradient::equality(
	const Function_variable_iterator_representation * representation)
//******************************************************************************
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
// Tests <*this> and <*representation> for equality.
//==============================================================================
{
	bool result;
	const Function_variable_iterator_representation_atomic_gradient
		*representation_gradient=dynamic_cast<const
		Function_variable_iterator_representation_atomic_gradient *>(
		representation);

	result=false;
	if (representation_gradient)
	{
		result=equivalent(atomic_variable,
			representation_gradient->atomic_variable);
	}

	return (result);
}

Function_variable_handle
	Function_variable_iterator_representation_atomic_gradient::dereference() const
//******************************************************************************
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
// Returns the atomic variable for the iterator.
//==============================================================================
{
	return (atomic_variable);
}

Function_variable_iterator_representation_atomic_gradient::
	Function_variable_iterator_representation_atomic_gradient(const
	Function_variable_iterator_representation_atomic_gradient&
	representation):Function_variable_iterator_representation(),
	atomic_variable(0),variable(representation.variable),
	atomic_dependent_iterator(representation.atomic_dependent_iterator),
	atomic_dependent_iterator_begin(
	representation.atomic_dependent_iterator_begin),
	atomic_dependent_iterator_end(
	representation.atomic_dependent_iterator_end),
	atomic_independent_iterator(representation.atomic_independent_iterator),
	atomic_independent_iterator_begin(
	representation.atomic_independent_iterator_begin),
	atomic_independent_iterator_end(
	representation.atomic_independent_iterator_end)
//******************************************************************************
// LAST MODIFIED : 26 August 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (representation.atomic_variable)
	{
		atomic_variable=boost::dynamic_pointer_cast<
			Function_variable_gradient,Function_variable>(
			(representation.atomic_variable)->clone());
	}
}


// global classes
// ==============

// class Function_gradient
// -----------------------

Function_gradient::Function_gradient(
	const Function_variable_handle& dependent_variable,
	const Function_variable_handle& independent_variable):Function(),
	dependent_variable_private(dependent_variable),
	independent_variable_private(independent_variable){}
//******************************************************************************
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================

Function_gradient::~Function_gradient(){}
//******************************************************************************
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================

string_handle Function_gradient::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
//???DB.  Overload << instead of get_string_representation?
//==============================================================================
{
	string_handle return_string(0);

	if (return_string=new std::string)
	{
		std::ostringstream out;

		out << "grad(";
		if (dependent_variable_private)
		{
			out << *(dependent_variable_private->get_string_representation());
		}
		out << ",";
		if (independent_variable_private)
		{
			out << *(independent_variable_private->get_string_representation());
		}
		out << ")";
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_handle Function_gradient::input()
//******************************************************************************
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	return ((dependent_variable_private->function)()->input());
}

Function_variable_handle Function_gradient::output()
//******************************************************************************
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_gradient(
		Function_gradient_handle(this))));
}

bool Function_gradient::operator==(const Function& function) const
//******************************************************************************
// LAST MODIFIED : 25 August 2004
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
			const Function_gradient& function_gradient=
				dynamic_cast<const Function_gradient&>(function);

			result=equivalent(dependent_variable_private,
				function_gradient.dependent_variable_private)&&
				equivalent(independent_variable_private,
				function_gradient.independent_variable_private);
		}
		catch (std::bad_cast)
		{
			// do nothing
		}
	}

	return (result);
}

Function_handle Function_gradient::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_variable_gradient_handle atomic_gradient_variable;

	if (this&&(atomic_gradient_variable=boost::dynamic_pointer_cast<
		Function_variable_gradient,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_gradient_variable->function())&&
		(atomic_gradient_variable->atomic_dependent_variable)&&
		(atomic_gradient_variable->atomic_independent_variable))
	{
		std::list<Function_variable_handle> independent_variables(1,
			atomic_gradient_variable->atomic_independent_variable);

		result=(atomic_gradient_variable->atomic_dependent_variable->
			evaluate_derivative)(independent_variables);
	}

	return (result);
}

bool Function_gradient::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_gradient_handle atomic_variable_gradient;
	Function_variable_handle atomic_dependent_variable,
		atomic_independent_variable;

	result=false;
	if ((atomic_variable_gradient=boost::dynamic_pointer_cast<
		Function_variable_gradient,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_gradient->function())&&
		(1==atomic_variable_gradient->number_differentiable())&&
		(atomic_dependent_variable=atomic_variable_gradient->
		atomic_dependent_variable)&&(atomic_independent_variable=
		atomic_variable_gradient->atomic_independent_variable))
	{
		std::list<Function_variable_handle> extended_atomic_independent_variables=
			atomic_independent_variables;

		extended_atomic_independent_variables.push_front(
			atomic_independent_variable);
		result=(atomic_dependent_variable->function)()->evaluate_derivative(
			derivative,atomic_dependent_variable,
			extended_atomic_independent_variables);
	}

	return (result);
}

bool Function_gradient::set_value(Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_handle function;

	result=false;
	if (dependent_variable_private&&
		(function=(dependent_variable_private->function)()))
	{
		result=(function->set_value)(atomic_variable,atomic_value);
	}

	return (result);
}

Function_handle Function_gradient::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle function,result;

	result=0;
	if (dependent_variable_private&&
		(function=(dependent_variable_private->function)()))
	{
		result=(function->get_value)(atomic_variable);
	}

	return (result);
}

Function_gradient::Function_gradient(
	const Function_gradient& function_gradient):Function(),
	dependent_variable_private(function_gradient.dependent_variable_private),
	independent_variable_private(function_gradient.independent_variable_private)
//******************************************************************************
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Function_gradient& Function_gradient::operator=(
	const Function_gradient& function_gradient)
//******************************************************************************
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	dependent_variable_private=function_gradient.dependent_variable_private;
	independent_variable_private=function_gradient.independent_variable_private;

	return (*this);
}
