//******************************************************************************
// FILE : function_derivative.cpp
//
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
//==============================================================================

#include <sstream>

#include "computed_variable/function_derivative.hpp"
#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable.hpp"

// module classes
// ==============

#if defined (BEFORE_CACHING)
// forward declaration so that can use _handle
class Function_variable_derivative;
typedef boost::intrusive_ptr<Function_variable_derivative>
	Function_variable_derivative_handle;

// class Function_variable_iterator_representation_atomic_derivative
// -----------------------------------------------------------------

class Function_variable_iterator_representation_atomic_derivative:
	public Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 8 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_derivative(
			const bool begin,Function_variable_derivative_handle variable);
		// a "virtual" constructor
		Function_variable_iterator_representation *clone();
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_derivative();
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
		Function_variable_iterator_representation_atomic_derivative(const
			Function_variable_iterator_representation_atomic_derivative&);
	private:
		Function_variable_derivative_handle atomic_variable,variable;
		Function_variable_iterator atomic_dependent_iterator,
			atomic_dependent_iterator_begin,atomic_dependent_iterator_end;
		std::list<Function_variable_iterator> atomic_independent_iterators,
			atomic_independent_iterators_begin,atomic_independent_iterators_end;
};


// class Function_variable_derivative
// ----------------------------------

class Function_variable_derivative : public Function_variable
//******************************************************************************
// LAST MODIFIED : 23 November 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_derivative;
	friend class Function_variable_iterator_representation_atomic_derivative;
	public:
		// constructor
		Function_variable_derivative(
			const Function_derivative_handle& function_derivative):
			Function_variable(function_derivative),atomic_dependent_variable(0),
			atomic_independent_variables(0){};
		// constructor.  A zero <atomic_dependent_variable> indicates the whole
		//   dependent variable.  An empty <atomic_independent_variables> list
		//   indicates all derivatives.  A zero <atomic_independent_variables> list
		//   entry indicates all derivatives for the entry
		Function_variable_derivative(
			const Function_derivative_handle& function_derivative,
			Function_variable_handle& atomic_dependent_variable,
			std::list<Function_variable_handle>& atomic_independent_variables):
			Function_variable(function_derivative),atomic_dependent_variable(0),
			atomic_independent_variables(0)
		{
			if (function_derivative)
			{
				Function_size_type order;

				if (atomic_dependent_variable&&
					(function_derivative->dependent_variable_private))
				{
					Function_variable_iterator atomic_variable_iterator,
						end_atomic_variable_iterator;

					atomic_variable_iterator=
						function_derivative->dependent_variable_private->begin_atomic();
					end_atomic_variable_iterator=
						function_derivative->dependent_variable_private->end_atomic();
					while ((atomic_variable_iterator!=end_atomic_variable_iterator)&&
						!equivalent(*atomic_variable_iterator,atomic_dependent_variable))
					{
						atomic_variable_iterator++;
					}
					if (atomic_variable_iterator!=end_atomic_variable_iterator)
					{
						this->atomic_dependent_variable=atomic_dependent_variable;
					}
				}
				order=atomic_independent_variables.size();
				if ((0<order)&&(function_derivative->dependent_variable_private)&&
					((function_derivative->independent_variables_private).size()==order))
				{
					Function_variable_iterator atomic_variable_iterator,
						end_atomic_variable_iterator;
					std::list<Function_variable_handle>::iterator variable_iterator_1,
						variable_iterator_1_end,variable_iterator_2;

					(this->atomic_independent_variables).resize(order);
					variable_iterator_1_end=atomic_independent_variables.end();
					variable_iterator_2=(this->atomic_independent_variables).begin();
					end_atomic_variable_iterator=
						function_derivative->dependent_variable_private->end_atomic();
					for (variable_iterator_1=atomic_independent_variables.begin();
						variable_iterator_1!=variable_iterator_1_end;variable_iterator_1++)
					{
						*variable_iterator_2=0;
						if (*variable_iterator_1)
						{
							atomic_variable_iterator=
								function_derivative->dependent_variable_private->begin_atomic();
							while ((atomic_variable_iterator!=end_atomic_variable_iterator)&&
								!equivalent(*atomic_variable_iterator,*variable_iterator_1))
							{
								atomic_variable_iterator++;
							}
							if (atomic_variable_iterator!=end_atomic_variable_iterator)
							{
								*variable_iterator_2= *variable_iterator_1;
							}
						}
						variable_iterator_2++;
					}
				}
			}
		};
		// destructor
		~Function_variable_derivative(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			Function_derivative_handle function_derivative=
				boost::dynamic_pointer_cast<Function_derivative,Function>(function());
			Function_size_type i;
			Function_variable_derivative_handle result(0);
			Function_variable_handle local_atomic_dependent_variable(0);
			std::list<Function_variable_handle>
				local_atomic_independent_variables(0);
			std::list<Function_variable_handle>::const_iterator iterator;

			if (atomic_dependent_variable)
			{
				local_atomic_dependent_variable=atomic_dependent_variable->clone();
			}
			if (0<(i=atomic_independent_variables.size()))
			{
				iterator=atomic_independent_variables.begin();
				while (i>0)
				{
					if (*iterator)
					{
						local_atomic_independent_variables.push_back(
							(*iterator)->clone());
					}
					else
					{
						local_atomic_independent_variables.push_back(
							Function_variable_handle(0));
					}
					iterator++;
					i--;
				}
			}
			if (result=Function_variable_derivative_handle(
				new Function_variable_derivative(function_derivative,
				local_atomic_dependent_variable,local_atomic_independent_variables)))
			{
				result->value_private=value_private;
			}

			return (result);
		};
		Function_handle evaluate()
		{
			Function_derivative_handle function_derivative=
				boost::dynamic_pointer_cast<Function_derivative,Function>(function());
			Function_handle result(0);

			if (function_derivative)
			{
				Function_variable_handle dependent_variable;

				if (!(dependent_variable=atomic_dependent_variable))
				{
					dependent_variable=function_derivative->dependent_variable_private;
				}
				if (dependent_variable)
				{
					Function_size_type order;

					order=(function_derivative->independent_variables_private).size();
					if (0<order)
					{
						std::list<Function_variable_handle> independent_variables=
							function_derivative->independent_variables_private;

						if (order==atomic_independent_variables.size())
						{
							std::list<Function_variable_handle>::iterator iterator_1,
								iterator_1_end,iterator_2;

							iterator_1_end=atomic_independent_variables.end();
							iterator_2=independent_variables.begin();
							for (iterator_1=atomic_independent_variables.begin();
								iterator_1!=iterator_1_end;iterator_1++)
							{
								if (*iterator_1)
								{
									*iterator_2= *iterator_1;
								}
								iterator_2++;
							}
						}
						result=
							dependent_variable->evaluate_derivative(independent_variables);
					}
				}
			}

			return (result);
		};
		Function_handle evaluate_derivative(
			std::list<Function_variable_handle>& independent_variables)
		{
			Function_derivative_handle function_derivative=
				boost::dynamic_pointer_cast<Function_derivative,Function>(function());
			Function_handle result(0);

			//???debug
			std::cout << "enter Function_variable_derivative::evaluate_derivative.  " << std::endl;
			if (function_derivative)
			{
				Function_variable_handle dependent_variable;

				if (!(dependent_variable=atomic_dependent_variable))
				{
					dependent_variable=function_derivative->dependent_variable_private;
				}
				if (dependent_variable)
				{
					Function_size_type order;

					order=(function_derivative->independent_variables_private).size();
					if (0<order)
					{
						boost::intrusive_ptr< Function_matrix<Scalar> >
							derivative_matrix(0);
						Function_size_type number_of_independent_values;
						std::list<Function_variable_handle> independent_variables_local=
							function_derivative->independent_variables_private;
						std::list<Function_variable_handle>::iterator iterator_1,
							iterator_1_end;

						iterator_1_end=independent_variables.end();
						number_of_independent_values=1;
						for (iterator_1=independent_variables.begin();
							iterator_1!=iterator_1_end;iterator_1++)
						{
							number_of_independent_values *=
								(*iterator_1)->number_differentiable();
						}
						if (order==atomic_independent_variables.size())
						{
							std::list<Function_variable_handle>::iterator iterator_2;

							iterator_1_end=atomic_independent_variables.end();
							iterator_2=independent_variables_local.begin();
							for (iterator_1=atomic_independent_variables.begin();
								iterator_1!=iterator_1_end;iterator_1++)
							{
								if (*iterator_1)
								{
									*iterator_2= *iterator_1;
								}
								iterator_2++;
							}
						}
						independent_variables_local.insert(
							independent_variables_local.end(),independent_variables.begin(),
							independent_variables.end());
						if (derivative_matrix=boost::dynamic_pointer_cast<
							Function_matrix<Scalar>,Function>(dependent_variable->
							evaluate_derivative(independent_variables_local)))
						{
							Function_size_type
								number_of_columns=derivative_matrix->number_of_columns(),
								number_of_rows=derivative_matrix->number_of_rows();

							if (number_of_columns==number_of_independent_values)
							{
								result=derivative_matrix;
							}
							else
							{
								Function_size_type number_of_dependent_values=number_of_rows*
									(number_of_columns/number_of_independent_values);
								Function_size_type column,i,j,row;
								Matrix result_matrix(number_of_dependent_values,
									number_of_independent_values);

								row=1;
								column=1;
								for (i=0;i<number_of_dependent_values;i++)
								{
									for (j=0;j<number_of_independent_values;j++)
									{
										result_matrix(i,j)=(*derivative_matrix)(row,column);
										column++;
									}
									if (column>number_of_columns)
									{
										row++;
										column=1;
									}
								}
								result=
									Function_handle(new Function_matrix<Scalar>(result_matrix));
							}
						}
					}
				}
			}
			//???debug
			std::cout << "leave Function_variable_derivative::evaluate_derivative.  " << result << std::endl;

			return (result);
		};
		string_handle get_string_representation()
		{
			Function_derivative_handle function_derivative=
				boost::dynamic_pointer_cast<Function_derivative,Function>(function());
			string_handle return_string(0);

			if (return_string=new std::string)
			{
				Function_size_type order;
				std::ostringstream out;

				out << "d(";
				if (atomic_dependent_variable)
				{
					out << *(atomic_dependent_variable->get_string_representation());
				}
				else
				{
					if (function_derivative&&
						(function_derivative->dependent_variable_private))
					{
						out << *(function_derivative->dependent_variable_private->
							get_string_representation());
					}
				}
				out << ")/d(";
				order=atomic_independent_variables.size();
				if (0<order)
				{
					if (function_derivative&&(order==
						(function_derivative->independent_variables_private).size()))
					{
						bool first;
						std::list<Function_variable_handle>::iterator variable_iterator_1,
							variable_iterator_1_end,variable_iterator_2;

						variable_iterator_1_end=atomic_independent_variables.end();
						variable_iterator_2=
							(function_derivative->independent_variables_private).begin();
						first=true;
						for (variable_iterator_1=atomic_independent_variables.begin();
							variable_iterator_1!=variable_iterator_1_end;
							variable_iterator_1++)
						{
							if (!first)
							{
								out << ",";
							}
							if (*variable_iterator_1)
							{
								out << *((*variable_iterator_1)->get_string_representation());
							}
							else
							{
								if (*variable_iterator_2)
								{
									out << *((*variable_iterator_2)->get_string_representation());
								}
							}
							variable_iterator_2++;
							first=false;
						}
					}
				}
				else
				{
					if (function_derivative&&
						(0<(function_derivative->independent_variables_private).size()))
					{
						bool first;
						std::list<Function_variable_handle>::iterator variable_iterator,
							variable_iterator_end;

						variable_iterator_end=
							(function_derivative->independent_variables_private).end();
						first=true;
						for (variable_iterator=
							(function_derivative->independent_variables_private).begin();
							variable_iterator!=variable_iterator_end;variable_iterator++)
						{
							if (!first)
							{
								out << ",";
							}
							if (*variable_iterator)
							{
								out << *((*variable_iterator)->get_string_representation());
							}
							first=false;
						}
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
				Function_variable_iterator_representation_atomic_derivative(
				true,Function_variable_derivative_handle(
				const_cast<Function_variable_derivative*>(this)))));
		};
		virtual Function_variable_iterator end_atomic() const
		{
			return (Function_variable_iterator(new
				Function_variable_iterator_representation_atomic_derivative(
				false,Function_variable_derivative_handle(
				const_cast<Function_variable_derivative*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(new
				Function_variable_iterator_representation_atomic_derivative(
				false,Function_variable_derivative_handle(
				const_cast<Function_variable_derivative*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(new
				Function_variable_iterator_representation_atomic_derivative(
				true,Function_variable_derivative_handle(
				const_cast<Function_variable_derivative*>(this)))));
		};
		Function_size_type number_differentiable()
		{
			Function_size_type number_of_columns,number_of_rows,result;

			result=0;
			number_of_rows=0;
			number_of_columns=0;
			if (this)
			{
				Function_derivative_handle function_derivative=
					boost::dynamic_pointer_cast<Function_derivative,Function>(function());
				Function_size_type order;

				if (atomic_dependent_variable)
				{
					number_of_rows=atomic_dependent_variable->number_differentiable();
				}
				else
				{
					if (function_derivative&&
						(function_derivative->dependent_variable_private))
					{
						number_of_rows=function_derivative->dependent_variable_private->
							number_differentiable();
					}
				}
				order=atomic_independent_variables.size();
				if (0<order)
				{
					if (function_derivative&&(order==
						(function_derivative->independent_variables_private).size()))
					{
						bool first;
						Function_size_type temp;
						std::list<Function_variable_handle>::iterator variable_iterator_1,
							variable_iterator_1_end,variable_iterator_2;

						variable_iterator_1_end=atomic_independent_variables.end();
						variable_iterator_2=
							(function_derivative->independent_variables_private).begin();
						first=true;
						for (variable_iterator_1=atomic_independent_variables.begin();
							variable_iterator_1!=variable_iterator_1_end;
							variable_iterator_1++)
						{
							temp=0;
							if (*variable_iterator_1)
							{
								temp=(*variable_iterator_1)->number_differentiable();
							}
							else
							{
								if (*variable_iterator_2)
								{
									temp=(*variable_iterator_2)->number_differentiable();
								}
							}
							if (first)
							{
								number_of_columns=temp;
							}
							else
							{
								number_of_columns *= temp;
							}
							variable_iterator_2++;
							first=false;
						}
					}
				}
				else
				{
					if (function_derivative&&
						(0<(function_derivative->independent_variables_private).size()))
					{
						bool first;
						std::list<Function_variable_handle>::iterator variable_iterator,
							variable_iterator_end;

						variable_iterator_end=
							(function_derivative->independent_variables_private).end();
						first=true;
						for (variable_iterator=
							(function_derivative->independent_variables_private).begin();
							variable_iterator!=variable_iterator_end;variable_iterator++)
						{
							if (*variable_iterator)
							{
								if (first)
								{
									number_of_columns=
										(*variable_iterator)->number_differentiable();
								}
								else
								{
									number_of_columns *=
										(*variable_iterator)->number_differentiable();
								}
							}
							first=false;
						}
					}
				}
			}
			result=number_of_rows*number_of_columns;

			return (result);
		};
		bool equality_atomic(const Function_variable_handle& variable) const
		{
			bool result;
			Function_derivative_handle function_derivative=
				boost::dynamic_pointer_cast<Function_derivative,Function>(function());
			Function_variable_derivative_handle variable_derivative;

			result=false;
			if (variable_derivative=boost::dynamic_pointer_cast<
				Function_variable_derivative,Function_variable>(variable))
			{
				if (equivalent(variable_derivative->function(),function_derivative)&&
					equivalent(variable_derivative->atomic_dependent_variable,
					atomic_dependent_variable))
				{
					if ((variable_derivative->atomic_independent_variables).size()==
						atomic_independent_variables.size())
					{
						std::list<Function_variable_handle>::const_iterator
							variable_iterator_1=atomic_independent_variables.begin(),
							variable_iterator_1_end=atomic_independent_variables.end();
						std::list<Function_variable_handle>::iterator variable_iterator_2;

						variable_iterator_2=
							(variable_derivative->atomic_independent_variables).begin();
						while ((variable_iterator_1!=variable_iterator_1_end)&&
							equivalent(*variable_iterator_1,*variable_iterator_2))
						{
							variable_iterator_1++;
							variable_iterator_2++;
						}
						if (variable_iterator_1==variable_iterator_1_end)
						{
							result=true;
						}
					}
				}
			}

			return (result);
		};
	private:
		bool is_atomic()
		{
			bool result;

			result=false;
			if (this&&atomic_dependent_variable)
			{
				std::list<Function_variable_handle>::iterator variable_iterator,
					variable_iterator_end;

				variable_iterator=atomic_independent_variables.begin();
				variable_iterator_end=atomic_independent_variables.end();
				while ((variable_iterator!=variable_iterator_end)&&(*variable_iterator))
				{
					variable_iterator++;
				}
				if (variable_iterator==variable_iterator_end)
				{
					result=true;
				}
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_derivative(
			const Function_variable_derivative& variable_derivative):
			Function_variable(variable_derivative),
			atomic_dependent_variable(variable_derivative.atomic_dependent_variable),
			atomic_independent_variables(
			variable_derivative.atomic_independent_variables){};
		// assignment
		Function_variable_derivative& operator=(
			const Function_variable_derivative&);
	private:
		// if zero then all
		Function_variable_handle atomic_dependent_variable;
		// if empty list then all.  If zero list entry then all for that independent
		//   variable
		std::list<Function_variable_handle> atomic_independent_variables;
};


// class Function_variable_iterator_representation_atomic_derivative
// -----------------------------------------------------------------

Function_variable_iterator_representation_atomic_derivative::
	Function_variable_iterator_representation_atomic_derivative(
	const bool begin,Function_variable_derivative_handle variable):
	atomic_variable(0),variable(variable),atomic_dependent_iterator(0),
	atomic_dependent_iterator_begin(0),atomic_dependent_iterator_end(0),
	atomic_independent_iterators(0),atomic_independent_iterators_begin(0),
	atomic_independent_iterators_end(0)
//******************************************************************************
// LAST MODIFIED : 30 June 2004
//
// DESCRIPTION :
// Constructor.  If <begin> then the constructed iterator points to the first
// atomic variable, otherwise it points to one past the last atomic variable.
//==============================================================================
{
	if (begin&&variable)
	{
		Function_derivative_handle function_derivative=
			boost::dynamic_pointer_cast<Function_derivative,Function>(
			variable->function());

		if (function_derivative&&(atomic_variable=boost::dynamic_pointer_cast<
			Function_variable_derivative,Function_variable>(variable->clone())))
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
					function_derivative->dependent_variable_private)
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
					Function_size_type order;
					std::list<Function_variable_handle> independent_variables_local=
						function_derivative->independent_variables_private;

					atomic_variable->atomic_dependent_variable=
						*atomic_dependent_iterator;
					order=(variable->atomic_independent_variables).size();
					if (0<order)
					{
						if (order==independent_variables_local.size())
						{
							std::list<Function_variable_handle>::iterator variable_iterator_1,
								variable_iterator_1_end,variable_iterator_2,variable_iterator_3;
							std::list<Function_variable_iterator>::iterator
								independent_iterators_begin_iterator,
								independent_iterators_end_iterator,
								independent_iterators_iterator;

							atomic_independent_iterators.resize(order);
							atomic_independent_iterators_begin.resize(order);
							atomic_independent_iterators_end.resize(order);
							independent_iterators_iterator=
								atomic_independent_iterators.begin();
							independent_iterators_begin_iterator=
								atomic_independent_iterators_begin.begin();
							independent_iterators_end_iterator=
								atomic_independent_iterators_end.begin();
							variable_iterator_1=
								(variable->atomic_independent_variables).begin();
							variable_iterator_1_end=
								(variable->atomic_independent_variables).end();
							variable_iterator_2=independent_variables_local.begin();
							variable_iterator_3=
								(atomic_variable->atomic_independent_variables).begin();
							while (atomic_variable&&
								(variable_iterator_1!=variable_iterator_1_end))
							{
								if (*variable_iterator_1)
								{
									*independent_iterators_begin_iterator=
										(*variable_iterator_1)->begin_atomic();
									*independent_iterators_end_iterator=
										(*variable_iterator_1)->end_atomic();
								}
								else
								{
									if (*variable_iterator_2)
									{
										*independent_iterators_begin_iterator=
											(*variable_iterator_2)->begin_atomic();
										*independent_iterators_end_iterator=
											(*variable_iterator_2)->end_atomic();
									}
									else
									{
										// end
										atomic_variable=0;
									}
								}
								if (atomic_variable)
								{
									*independent_iterators_iterator=
										*independent_iterators_begin_iterator;
									while ((*independent_iterators_iterator!=
										*independent_iterators_end_iterator)&&
										(1!=(**independent_iterators_iterator)->
										number_differentiable()))
									{
										(*independent_iterators_iterator)++;
									}
									if (*independent_iterators_iterator==
										*independent_iterators_end_iterator)
									{
										// end
										atomic_variable=0;
									}
									else
									{
										*variable_iterator_3= **independent_iterators_iterator;
									}
								}
								variable_iterator_1++;
								variable_iterator_2++;
								variable_iterator_3++;
								independent_iterators_iterator++;
								independent_iterators_begin_iterator++;
								independent_iterators_end_iterator++;
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
						if (0<(order=independent_variables_local.size()))
						{
							std::list<Function_variable_handle>::iterator variable_iterator_1,
								variable_iterator_1_end,variable_iterator_2;
							std::list<Function_variable_iterator>::iterator
								independent_iterators_begin_iterator,
								independent_iterators_end_iterator,
								independent_iterators_iterator;

							atomic_independent_iterators.resize(order);
							atomic_independent_iterators_begin.resize(order);
							atomic_independent_iterators_end.resize(order);
							independent_iterators_iterator=
								atomic_independent_iterators.begin();
							independent_iterators_begin_iterator=
								atomic_independent_iterators_begin.begin();
							independent_iterators_end_iterator=
								atomic_independent_iterators_end.begin();
							variable_iterator_1=independent_variables_local.begin();
							variable_iterator_1_end=independent_variables_local.end();
							(atomic_variable->atomic_independent_variables).resize(order);
							variable_iterator_2=
								(atomic_variable->atomic_independent_variables).begin();
							while (atomic_variable&&
								(variable_iterator_1!=variable_iterator_1_end))
							{
								if (*variable_iterator_1)
								{
									*independent_iterators_begin_iterator=
										(*variable_iterator_1)->begin_atomic();
									*independent_iterators_end_iterator=
										(*variable_iterator_1)->end_atomic();
								}
								else
								{
									// end
									atomic_variable=0;
								}
								if (atomic_variable)
								{
									*independent_iterators_iterator=
										*independent_iterators_begin_iterator;
									while ((*independent_iterators_iterator!=
										*independent_iterators_end_iterator)&&
										(1!=(**independent_iterators_iterator)->
										number_differentiable()))
									{
										(*independent_iterators_iterator)++;
									}
									if (*independent_iterators_iterator==
										*independent_iterators_end_iterator)
									{
										// end
										atomic_variable=0;
									}
									else
									{
										*variable_iterator_2= **independent_iterators_iterator;
									}
								}
								variable_iterator_1++;
								variable_iterator_2++;
								independent_iterators_iterator++;
								independent_iterators_begin_iterator++;
								independent_iterators_end_iterator++;
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

Function_variable_iterator_representation
	*Function_variable_iterator_representation_atomic_derivative::clone()
//******************************************************************************
// LAST MODIFIED : 8 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_iterator_representation *result;

	result=0;
	if (this)
	{
		result=new Function_variable_iterator_representation_atomic_derivative(
			*this);
	}

	return (result);
}

Function_variable_iterator_representation_atomic_derivative::
	~Function_variable_iterator_representation_atomic_derivative()
//******************************************************************************
// LAST MODIFIED : 8 June 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

void Function_variable_iterator_representation_atomic_derivative::increment()
//******************************************************************************
// LAST MODIFIED : 10 June 2004
//
// DESCRIPTION :
// Increments the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		if (0<(atomic_variable->atomic_independent_variables).size())
		{
			bool finished;
			std::list<Function_variable_handle>::iterator variable_iterator_1,
				variable_iterator_1_begin;
			std::list<Function_variable_iterator>::iterator
				independent_iterators_begin_iterator,
				independent_iterators_end_iterator,
				independent_iterators_iterator;

			variable_iterator_1=(atomic_variable->atomic_independent_variables).end();
			variable_iterator_1_begin=
				(atomic_variable->atomic_independent_variables).begin();
			independent_iterators_iterator=
				atomic_independent_iterators.end();
			independent_iterators_begin_iterator=
				atomic_independent_iterators_begin.end();
			independent_iterators_end_iterator=
				atomic_independent_iterators_end.end();
			finished=false;
			while ((variable_iterator_1!=variable_iterator_1_begin)&&!finished)
			{
				variable_iterator_1--;
				independent_iterators_iterator--;
				independent_iterators_begin_iterator--;
				independent_iterators_end_iterator--;
				(*independent_iterators_iterator)++;
				if (*independent_iterators_iterator==
					*independent_iterators_end_iterator)
				{
					*independent_iterators_iterator=
						*independent_iterators_begin_iterator;
				}
				else
				{
					finished=true;
				}
				*variable_iterator_1= **independent_iterators_iterator;
			}
			if (!finished)
			{
				atomic_dependent_iterator++;
				if (atomic_dependent_iterator!=atomic_dependent_iterator_end)
				{
					atomic_variable->atomic_dependent_variable=
						*atomic_dependent_iterator;
					finished=true;
				}
				if (!finished)
				{
					atomic_variable=0;
				}
			}
		}
		else
		{
			atomic_variable=0;
		}
	}
}

void Function_variable_iterator_representation_atomic_derivative::decrement()
//******************************************************************************
// LAST MODIFIED : 11 June 2004
//
// DESCRIPTION :
// Decrements the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		if (0<(atomic_variable->atomic_independent_variables).size())
		{
			bool finished;
			std::list<Function_variable_handle>::iterator variable_iterator_1,
				variable_iterator_1_begin;
			std::list<Function_variable_iterator>::iterator
				independent_iterators_begin_iterator,
				independent_iterators_end_iterator,
				independent_iterators_iterator;

			variable_iterator_1=(atomic_variable->atomic_independent_variables).end();
			variable_iterator_1_begin=
				(atomic_variable->atomic_independent_variables).begin();
			independent_iterators_iterator=
				atomic_independent_iterators.end();
			independent_iterators_begin_iterator=
				atomic_independent_iterators_begin.end();
			independent_iterators_end_iterator=
				atomic_independent_iterators_end.end();
			finished=false;
			while ((variable_iterator_1!=variable_iterator_1_begin)&&!finished)
			{
				variable_iterator_1--;
				independent_iterators_iterator--;
				independent_iterators_begin_iterator--;
				independent_iterators_end_iterator--;
				if (*independent_iterators_iterator==
					*independent_iterators_begin_iterator)
				{
					*independent_iterators_iterator=
						*independent_iterators_end_iterator;
				}
				else
				{
					finished=true;
				}
				(*independent_iterators_iterator)--;
				*variable_iterator_1= **independent_iterators_iterator;
			}
			if (!finished)
			{
				if (atomic_dependent_iterator!=atomic_dependent_iterator_begin)
				{
					atomic_dependent_iterator--;
					atomic_variable->atomic_dependent_variable=
						*atomic_dependent_iterator;
					finished=true;
				}
				if (!finished)
				{
					atomic_variable=0;
				}
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
			Function_derivative_handle function_derivative=
				boost::dynamic_pointer_cast<Function_derivative,Function>(
				variable->function());

			if (atomic_variable=boost::dynamic_pointer_cast<
				Function_variable_derivative,Function_variable>(variable->clone()))
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
						function_derivative->dependent_variable_private)
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
					Function_size_type number_differentiable_local;

					atomic_dependent_iterator=atomic_dependent_iterator_end;
					atomic_dependent_iterator--;
					while ((1!=(number_differentiable_local=
						(*atomic_dependent_iterator)->number_differentiable()))&&
						(atomic_dependent_iterator!=atomic_dependent_iterator_begin))
					{
						atomic_dependent_iterator--;
					}
					if (1==number_differentiable_local)
					{
						Function_size_type order;
						std::list<Function_variable_handle> independent_variables_local=
							function_derivative->independent_variables_private;

						atomic_variable->atomic_dependent_variable=
							*atomic_dependent_iterator;
						order=(variable->atomic_independent_variables).size();
						if (0<order)
						{
							if (order==independent_variables_local.size())
							{
								std::list<Function_variable_handle>::iterator
									variable_iterator_1,variable_iterator_1_end,
									variable_iterator_2,variable_iterator_3;
								std::list<Function_variable_iterator>::iterator
									independent_iterators_begin_iterator,
									independent_iterators_end_iterator,
									independent_iterators_iterator;

								atomic_independent_iterators.resize(order);
								atomic_independent_iterators_begin.resize(order);
								atomic_independent_iterators_end.resize(order);
								independent_iterators_iterator=
									atomic_independent_iterators.begin();
								independent_iterators_begin_iterator=
									atomic_independent_iterators_begin.begin();
								independent_iterators_end_iterator=
									atomic_independent_iterators_end.begin();
								variable_iterator_1=
									(variable->atomic_independent_variables).begin();
								variable_iterator_1_end=
									(variable->atomic_independent_variables).end();
								variable_iterator_2=independent_variables_local.begin();
								variable_iterator_3=
									(atomic_variable->atomic_independent_variables).begin();
								while (atomic_variable&&
									(variable_iterator_1!=variable_iterator_1_end))
								{
									if (*variable_iterator_1)
									{
										*independent_iterators_begin_iterator=
											(*variable_iterator_1)->begin_atomic();
										*independent_iterators_end_iterator=
											(*variable_iterator_1)->end_atomic();
									}
									else
									{
										if (*variable_iterator_2)
										{
											*independent_iterators_begin_iterator=
												(*variable_iterator_2)->begin_atomic();
											*independent_iterators_end_iterator=
												(*variable_iterator_2)->end_atomic();
										}
										else
										{
											// end
											atomic_variable=0;
										}
									}
									if (atomic_variable&&(*independent_iterators_begin_iterator!=
										*independent_iterators_end_iterator))
									{
										*independent_iterators_iterator=
											*independent_iterators_begin_iterator;
										(*independent_iterators_iterator)--;
										while ((1!=(number_differentiable_local=
											(**independent_iterators_iterator)->
											number_differentiable()))&&
											(*independent_iterators_iterator!=
											*independent_iterators_begin_iterator))
										{
											(*independent_iterators_iterator)--;
										}
										if (1==number_differentiable_local)
										{
											*variable_iterator_3= **independent_iterators_iterator;
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
									variable_iterator_1++;
									variable_iterator_2++;
									variable_iterator_3++;
									independent_iterators_iterator++;
									independent_iterators_begin_iterator++;
									independent_iterators_end_iterator++;
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
							if (0<(order=independent_variables_local.size()))
							{
								std::list<Function_variable_handle>::iterator
									variable_iterator_1,variable_iterator_1_end,
									variable_iterator_2;
								std::list<Function_variable_iterator>::iterator
									independent_iterators_begin_iterator,
									independent_iterators_end_iterator,
									independent_iterators_iterator;

								atomic_independent_iterators.resize(order);
								atomic_independent_iterators_begin.resize(order);
								atomic_independent_iterators_end.resize(order);
								independent_iterators_iterator=
									atomic_independent_iterators.begin();
								independent_iterators_begin_iterator=
									atomic_independent_iterators_begin.begin();
								independent_iterators_end_iterator=
									atomic_independent_iterators_end.begin();
								variable_iterator_1=independent_variables_local.begin();
								variable_iterator_1_end=independent_variables_local.end();
								(atomic_variable->atomic_independent_variables).resize(order);
								variable_iterator_2=
									(atomic_variable->atomic_independent_variables).begin();
								while (atomic_variable&&
									(variable_iterator_1!=variable_iterator_1_end))
								{
									if (*variable_iterator_1)
									{
										*independent_iterators_begin_iterator=
											(*variable_iterator_1)->begin_atomic();
										*independent_iterators_end_iterator=
											(*variable_iterator_1)->end_atomic();
									}
									else
									{
										// end
										atomic_variable=0;
									}
									if (atomic_variable&&(*independent_iterators_begin_iterator!=
										*independent_iterators_end_iterator))
									{
										*independent_iterators_iterator=
											*independent_iterators_begin_iterator;
										(*independent_iterators_iterator)--;
										while ((1!=(number_differentiable_local=
											(**independent_iterators_iterator)->
											number_differentiable()))&&
											(*independent_iterators_iterator!=
											*independent_iterators_begin_iterator))
										{
											(*independent_iterators_iterator)--;
										}
										if (1==number_differentiable_local)
										{
											*variable_iterator_2= **independent_iterators_iterator;
										}
										else
										{
											// end
											atomic_variable=0;
										}
									}
									variable_iterator_1++;
									variable_iterator_2++;
									independent_iterators_iterator++;
									independent_iterators_begin_iterator++;
									independent_iterators_end_iterator++;
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
}

bool Function_variable_iterator_representation_atomic_derivative::equality(
	const Function_variable_iterator_representation * representation)
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
// Tests <*this> and <*representation> for equality.
//==============================================================================
{
	bool result;
	const Function_variable_iterator_representation_atomic_derivative
		*representation_derivative=dynamic_cast<const
		Function_variable_iterator_representation_atomic_derivative *>(
		representation);

	result=false;
	if (representation_derivative)
	{
		result=equivalent(atomic_variable,
			representation_derivative->atomic_variable);
	}

	return (result);
}

Function_variable_handle
	Function_variable_iterator_representation_atomic_derivative::
	dereference() const
//******************************************************************************
// LAST MODIFIED : 8 June 2004
//
// DESCRIPTION :
// Returns the atomic variable for the iterator.
//==============================================================================
{
	return (atomic_variable);
}

Function_variable_iterator_representation_atomic_derivative::
	Function_variable_iterator_representation_atomic_derivative(const
	Function_variable_iterator_representation_atomic_derivative&
	representation):Function_variable_iterator_representation(),
	atomic_variable(0),variable(representation.variable),
	atomic_dependent_iterator(representation.atomic_dependent_iterator),
	atomic_dependent_iterator_begin(
	representation.atomic_dependent_iterator_begin),
	atomic_dependent_iterator_end(representation.atomic_dependent_iterator_end),
	atomic_independent_iterators(representation.atomic_independent_iterators),
	atomic_independent_iterators_begin(
	representation.atomic_independent_iterators_begin),
	atomic_independent_iterators_end(
	representation.atomic_independent_iterators_end)
//******************************************************************************
// LAST MODIFIED : 9 June 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (representation.atomic_variable)
	{
		atomic_variable=boost::dynamic_pointer_cast<
			Function_variable_derivative,Function_variable>(
			(representation.atomic_variable)->clone());
	}
}
#else // defined (BEFORE_CACHING)
// class Function_variable_derivative
// ----------------------------------

class Function_variable_derivative : public Function_variable_matrix<Scalar>
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_derivative;
	public:
		// constructor
		Function_variable_derivative(
			const Function_derivative_handle& function_derivative):
			Function_variable_matrix<Scalar>(function_derivative){};
		Function_variable_derivative(
			const Function_derivative_handle& function_derivative,
			const Function_size_type row,const Function_size_type column):
			Function_variable_matrix<Scalar>(function_derivative,row,column){};
		// destructor
		~Function_variable_derivative(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(new Function_variable_derivative(
				*this)));
		};
		Function_handle evaluate()
		{
			Function_derivative_handle function_derivative;
			Function_handle result(0);

			if (function_derivative=
				boost::dynamic_pointer_cast<Function_derivative,Function>(function()))
			{
				Function_size_type i,j,number_of_columns,number_of_rows;
				Matrix &values_local=function_derivative->values;

				if (!(function_derivative->evaluated()))
				{
					boost::intrusive_ptr< Function_matrix<Scalar> > derivative_matrix(0);

					if (derivative_matrix=boost::dynamic_pointer_cast<
						Function_matrix<Scalar>,Function>(function_derivative->
						dependent_variable_private->evaluate_derivative(
						function_derivative->independent_variables_private)))
					{
						number_of_rows=derivative_matrix->number_of_rows();
						number_of_columns=derivative_matrix->number_of_columns();
						values_local.resize(number_of_rows,number_of_columns);
						for (i=0;i<number_of_rows;i++)
						{
							for (j=0;j<number_of_columns;j++)
							{
								values_local(i,j)=(*derivative_matrix)(i+1,j+1);
							}
						}
						function_derivative->set_evaluated();
					}
				}
				if (function_derivative->evaluated())
				{
					if ((0==row_private)&&(0==column_private))
					{
						result=Function_handle(new Function_matrix<Scalar>(values_local));
					}
					else
					{
						if ((0<(number_of_rows=values_local.size1()))&&
							(0<(number_of_columns=values_local.size2()))&&
							(row_private<=number_of_rows)&&
							(column_private<=number_of_columns))
						{
							if (0==row_private)
							{
								Matrix sub_matrix(number_of_rows,1);
								
								j=column_private-1;
								for (i=0;i<number_of_rows;i++)
								{
									sub_matrix(i,0)=values_local(i,j);
								}
								result=Function_handle(new Function_matrix<Scalar>(sub_matrix));
							}
							else
							{
								if (0==column_private)
								{
									Matrix sub_matrix(1,number_of_columns);
									
									i=row_private-1;
									for (j=0;j<number_of_columns;j++)
									{
										sub_matrix(0,j)=values_local(i,j);
									}
									result=Function_handle(new Function_matrix<Scalar>(
										sub_matrix));
								}
								else
								{
									Matrix sub_matrix(1,1);
									
									sub_matrix(0,0)=values_local(row_private-1,column_private-1);
									result=Function_handle(new Function_matrix<Scalar>(
										sub_matrix));
								}
							}
						}
					}
				}
			}

			return (result);
		};
		Function_handle evaluate_derivative(
			std::list<Function_variable_handle>& independent_variables)
		{
			Function_derivative_handle function_derivative;
			Function_handle result(0);

			if (function_derivative=
				boost::dynamic_pointer_cast<Function_derivative,Function>(function()))
			{
				Function_size_type i,j,number_of_independent_values;
				Function_variable_handle dependent_variable(0);

				if (0==row_private)
				{
					dependent_variable=function_derivative->dependent_variable_private;
				}
				else
				{
					Function_variable_iterator atomic_iterator,atomic_iterator_end;

					atomic_iterator=function_derivative->dependent_variable_private->
						begin_atomic();
					atomic_iterator_end=function_derivative->dependent_variable_private->
						end_atomic();
					i=1;
					while ((i<row_private)&&(atomic_iterator!=atomic_iterator_end))
					{
						if (1==(*atomic_iterator)->number_differentiable())
						{
							i++;
						}
						atomic_iterator++;
					}
					if (atomic_iterator!=atomic_iterator_end)
					{
						// OK not to clone, because atomic_iterator is a local variable
						//   and won't be used again
						dependent_variable= *atomic_iterator;
					}
				}
				if (dependent_variable)
				{
					bool valid=true;
					std::list<Function_variable_handle> independent_variables_local=
						function_derivative->independent_variables_private;
					std::list<Function_variable_handle>::iterator iterator,iterator_end;

					if (0<column_private)
					{
						iterator=independent_variables_local.begin();
						iterator_end=independent_variables_local.end();
						i=column_private-1;
						while (valid&&(iterator!=iterator_end))
						{
							number_of_independent_values=(*iterator)->number_differentiable();
							if (0<number_of_independent_values)
							{
								Function_variable_iterator atomic_iterator,atomic_iterator_end;

								j=i%number_of_independent_values;
								i /= number_of_independent_values;
								atomic_iterator=(*iterator)->begin_atomic();
								atomic_iterator_end=(*iterator)->end_atomic();
								while ((j>0)&&(atomic_iterator!=atomic_iterator_end))
								{
									if (1==(*atomic_iterator)->number_differentiable())
									{
										j--;
									}
									atomic_iterator++;
								}
								if (atomic_iterator!=atomic_iterator_end)
								{
									// OK not to clone, because atomic_iterator is a local
									//   variable and won't be used again
									(*iterator)= *atomic_iterator;
								}
								else
								{
									valid=false;
								}
							}
							else
							{
								valid=false;
							}
							iterator++;
						}
						if (0!=i)
						{
							valid=false;
						}
					}
					if (valid)
					{
						boost::intrusive_ptr< Function_matrix<Scalar> >
							derivative_matrix(0);

						// Could have used
//						independent_variables_local.insert(
//							independent_variables_local.end(),independent_variables.begin(),
//							independent_variables.end());
						//   but already need loop
						iterator_end=independent_variables.end();
						number_of_independent_values=1;
						for (iterator=independent_variables.begin();
							iterator!=iterator_end;iterator++)
						{
							number_of_independent_values *=
								(*iterator)->number_differentiable();
							independent_variables_local.push_back(*iterator);
						}
						if (derivative_matrix=boost::dynamic_pointer_cast<
							Function_matrix<Scalar>,Function>(dependent_variable->
							evaluate_derivative(independent_variables_local)))
						{
							Function_size_type
								number_of_columns=derivative_matrix->number_of_columns(),
								number_of_rows=derivative_matrix->number_of_rows();

							if (number_of_columns==number_of_independent_values)
							{
								result=derivative_matrix;
							}
							else
							{
								Function_size_type number_of_dependent_values=number_of_rows*
									(number_of_columns/number_of_independent_values);
								Function_size_type column,row;
								Matrix result_matrix(number_of_dependent_values,
									number_of_independent_values);

								row=1;
								column=1;
								for (i=0;i<number_of_dependent_values;i++)
								{
									for (j=0;j<number_of_independent_values;j++)
									{
										result_matrix(i,j)=(*derivative_matrix)(row,column);
										column++;
									}
									if (column>number_of_columns)
									{
										row++;
										column=1;
									}
								}
								result=
									Function_handle(new Function_matrix<Scalar>(result_matrix));
							}
						}
					}
				}
			}

			return (result);
		};
		string_handle get_string_representation()
		{
			Function_derivative_handle function_derivative=
				boost::dynamic_pointer_cast<Function_derivative,Function>(function());
			string_handle return_string(0);

			if ((return_string=new std::string)&&function_derivative)
			{
				bool first;
				std::list<Function_variable_handle>::iterator variable_iterator,
					variable_iterator_end;
				std::ostringstream out;

				out << "d(";
				if (function_derivative->dependent_variable_private)
				{
					out << *(function_derivative->dependent_variable_private->
						get_string_representation());
				}
				out << ")/d(";
				variable_iterator_end=
					(function_derivative->independent_variables_private).end();
				first=true;
				for (variable_iterator=
					(function_derivative->independent_variables_private).begin();
					variable_iterator!=variable_iterator_end;
					variable_iterator++)
				{
					if (!first)
					{
						out << ",";
					}
					if (*variable_iterator)
					{
						out << *((*variable_iterator)->get_string_representation());
					}
					first=false;
				}
				out << ")";
				out << "[";
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
		};
	private:
		// copy constructor
		Function_variable_derivative(
			const Function_variable_derivative& variable_derivative):
			Function_variable_matrix<Scalar>(variable_derivative){};
};
#endif // defined (BEFORE_CACHING)


// global classes
// ==============

// class Function_derivative
// -------------------------

#if defined (BEFORE_CACHING)
#else // defined (BEFORE_CACHING)
ublas::matrix<Scalar,ublas::column_major>
  Function_derivative::constructor_values(0,0);
#endif // defined (BEFORE_CACHING)

Function_derivative::Function_derivative(
	const Function_variable_handle& dependent_variable,
	std::list<Function_variable_handle>& independent_variables):
#if defined (BEFORE_CACHING)
	Function(),
#else // defined (BEFORE_CACHING)
	Function_matrix<Scalar>(Function_derivative::constructor_values),
#endif // defined (BEFORE_CACHING)
	dependent_variable_private(dependent_variable),
	independent_variables_private(independent_variables)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (dependent_variable_private)
	{
		dependent_variable_private->add_dependent_function(this);
	}
}

Function_derivative::~Function_derivative()
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
	if (dependent_variable_private)
	{
		dependent_variable_private->remove_dependent_function(this);
	}
#endif // defined (CIRCULAR_SMART_POINTERS)
}

string_handle Function_derivative::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 9 June 2004
//
// DESCRIPTION :
//???DB.  Overload << instead of get_string_representation?
//==============================================================================
{
	string_handle return_string(0);

	if (return_string=new std::string)
	{
		std::ostringstream out;

		out << "d(";
		if (dependent_variable_private)
		{
			out << *(dependent_variable_private->get_string_representation());
		}
		out << ")/d(";
		if (0<independent_variables_private.size())
		{
			bool first;
			std::list<Function_variable_handle>::iterator variable_iterator,
				variable_iterator_end;

			variable_iterator_end=independent_variables_private.end();
			first=true;
			for (variable_iterator=independent_variables_private.begin();
				variable_iterator!=variable_iterator_end;variable_iterator++)
			{
				if (!first)
				{
					out << ",";
				}
				if (*variable_iterator)
				{
					out << *((*variable_iterator)->get_string_representation());
				}
				first=false;
			}
		}
		out << ")";
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_handle Function_derivative::input()
//******************************************************************************
// LAST MODIFIED : 9 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return ((dependent_variable_private->function)()->input());
}

Function_variable_handle Function_derivative::output()
//******************************************************************************
// LAST MODIFIED : 9 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_derivative(
		Function_derivative_handle(this))));
}

bool Function_derivative::operator==(const Function& function) const
//******************************************************************************
// LAST MODIFIED : 13 August 2004
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
			const Function_derivative& function_derivative=
				dynamic_cast<const Function_derivative&>(function);

			if (equivalent(dependent_variable_private,
				function_derivative.dependent_variable_private))
			{
				std::list<Function_variable_handle>::const_iterator
					variable_iterator_1,variable_iterator_1_end,variable_iterator_2,
					variable_iterator_2_end;

				variable_iterator_1=independent_variables_private.begin();
				variable_iterator_1_end=independent_variables_private.end();
				variable_iterator_2=
					function_derivative.independent_variables_private.begin();
				variable_iterator_2_end=
					function_derivative.independent_variables_private.end();
				while ((variable_iterator_1!=variable_iterator_1_end)&&
					(variable_iterator_2!=variable_iterator_2_end)&&
					equivalent(*variable_iterator_1,*variable_iterator_2))
				{
					variable_iterator_1++;
					variable_iterator_2++;
				}
				result=((variable_iterator_1==variable_iterator_1_end)&&
					(variable_iterator_2==variable_iterator_2_end));
			}
		}
		catch (std::bad_cast)
		{
			// do nothing
		}
	}

	return (result);
}

Function_handle Function_derivative::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 2 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	boost::intrusive_ptr< Function_variable_derivative >
		atomic_variable_derivative;

	if (this&&(atomic_variable_derivative=boost::dynamic_pointer_cast<
		Function_variable_derivative,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_derivative->function())&&
#if defined (BEFORE_CACHING)
		(atomic_variable_derivative->is_atomic)()&&
		(atomic_variable_derivative->atomic_dependent_variable)
#else // defined (BEFORE_CACHING)
		(0<atomic_variable_derivative->row())&&
		(0<atomic_variable_derivative->column())
#endif // defined (BEFORE_CACHING)
		)
	{
#if defined (BEFORE_CACHING)
		result=(atomic_variable_derivative->atomic_dependent_variable->
			evaluate_derivative)(atomic_variable_derivative->
			atomic_independent_variables);
#else // defined (BEFORE_CACHING)
		result=(atomic_variable_derivative->evaluate)();
#endif // defined (BEFORE_CACHING)
	}

	return (result);
}

bool Function_derivative::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 2 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
#if defined (BEFORE_CACHING)
	Function_variable_handle atomic_variable_local;
#else // defined (BEFORE_CACHING)
#endif // defined (BEFORE_CACHING)
	boost::intrusive_ptr< Function_variable_derivative >
		atomic_variable_derivative;

	result=false;
	if (this&&(atomic_variable_derivative=boost::dynamic_pointer_cast<
		Function_variable_derivative,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_derivative->function())&&
#if defined (BEFORE_CACHING)
		(atomic_variable_derivative->is_atomic)()&&
		(1==atomic_variable_derivative->number_differentiable())&&
		(atomic_variable_local=
		atomic_variable_derivative->atomic_dependent_variable)
#else // defined (BEFORE_CACHING)
		(0<atomic_variable_derivative->row())&&
		(0<atomic_variable_derivative->column())&&
		(0<atomic_independent_variables.size())
#endif // defined (BEFORE_CACHING)
		)
	{
#if defined (BEFORE_CACHING)
		std::list<Function_variable_handle> merged_independent_variables=
			atomic_variable_derivative->atomic_independent_variables;

		merged_independent_variables.insert(merged_independent_variables.end(),
			atomic_independent_variables.begin(),atomic_independent_variables.end());
		result=(atomic_variable_local->function)()->evaluate_derivative(
			derivative,atomic_variable_local,merged_independent_variables);
#else // defined (BEFORE_CACHING)
		boost::intrusive_ptr< Function_matrix<Scalar> > derivative_matrix=
			boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
			atomic_variable_derivative->evaluate_derivative(
			atomic_independent_variables));

		if (derivative_matrix)
		{
			result=true;
			derivative=(*derivative_matrix)(1,1);
		}
#endif // defined (BEFORE_CACHING)
	}

	return (result);
}

bool Function_derivative::set_value(Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 9 June 2004
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

Function_handle Function_derivative::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 23 June 2004
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

Function_derivative::Function_derivative(
	const Function_derivative& function_derivative):
#if defined (BEFORE_CACHING)
	Function(),
#else // defined (BEFORE_CACHING)
	Function_matrix<Scalar>(function_derivative),
#endif // defined (BEFORE_CACHING)
	dependent_variable_private(function_derivative.dependent_variable_private),
	independent_variables_private(
	function_derivative.independent_variables_private)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (dependent_variable_private)
	{
		dependent_variable_private->add_dependent_function(this);
	}
}

Function_derivative& Function_derivative::operator=(
	const Function_derivative& function_derivative)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	if (function_derivative.dependent_variable_private)
	{
		function_derivative.dependent_variable_private->add_dependent_function(
			this);
	}
	if (dependent_variable_private)
	{
		dependent_variable_private->remove_dependent_function(this);
	}
	dependent_variable_private=function_derivative.dependent_variable_private;
	independent_variables_private=
		function_derivative.independent_variables_private;

	return (*this);
}
