//******************************************************************************
// FILE : function_derivative.cpp
//
// LAST MODIFIED : 27 January 2005
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
// LAST MODIFIED : 13 January 2005
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
#if defined (EVALUATE_RETURNS_VALUE)
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
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					Function_derivatnew_handle temp_function;
					Function_variable_handle temp_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

					if (
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						derivative_matrix=boost::dynamic_pointer_cast<
						Function_matrix<Scalar>,Function>(function_derivative->
						dependent_variable_private->evaluate_derivative(
						function_derivative->independent_variables_private))
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						(temp_function=boost::dynamic_pointer_cast<Function_derivatnew,
						Function>(function_derivative->dependent_variable_private->
						derivative(function_derivative->independent_variables_private)))&&
						(temp_variable=temp_function->output())&&
						(temp_variable->evaluate())&&(temp_variable=temp_function->matrix(
						function_derivative->independent_variables_private))&&
						(derivative_matrix=boost::dynamic_pointer_cast<Function_matrix<
						Scalar>,Function>(temp_variable->get_value()))
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						)
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
#else // defined (EVALUATE_RETURNS_VALUE)
		bool evaluate()
		{
			bool result(true);
			Function_derivative_handle function_derivative;

			if (function_derivative=
				boost::dynamic_pointer_cast<Function_derivative,Function>(function()))
			{
				Function_size_type i,j,number_of_columns,number_of_rows;
				Matrix &values_local=function_derivative->values;

				if (!(function_derivative->evaluated()))
				{
					boost::intrusive_ptr< Function_matrix<Scalar> > derivative_matrix(0);
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					Function_derivatnew_handle temp_function;
					Function_variable_handle temp_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

					result=false;
					if (
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						derivative_matrix=boost::dynamic_pointer_cast<
						Function_matrix<Scalar>,Function>(function_derivative->
						dependent_variable_private->evaluate_derivative(
						function_derivative->independent_variables_private))
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						(temp_function=boost::dynamic_pointer_cast<Function_derivatnew,
						Function>(function_derivative->dependent_variable_private->
						derivative(function_derivative->independent_variables_private)))&&
						(temp_variable=temp_function->output())&&
						(temp_variable->evaluate())&&(temp_variable=temp_function->matrix(
						function_derivative->independent_variables_private))&&
						(derivative_matrix=boost::dynamic_pointer_cast<Function_matrix<
						Scalar>,Function>(temp_variable->get_value()))
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						)
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
						result=true;
					}
				}
			}

			return (result);
		};
#endif // defined (EVALUATE_RETURNS_VALUE)
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
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						Function_derivatnew_handle temp_function;
						Function_variable_handle temp_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

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
						if (
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
							derivative_matrix=boost::dynamic_pointer_cast<
							Function_matrix<Scalar>,Function>(dependent_variable->
							evaluate_derivative(independent_variables_local))
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
							(temp_function=boost::dynamic_pointer_cast<Function_derivatnew,
							Function>(dependent_variable->derivative(
							independent_variables_local)))&&
							(temp_variable=temp_function->output())&&
							(temp_variable->evaluate())&&(temp_variable=temp_function->matrix(
							independent_variables_local))&&
							(derivative_matrix=boost::dynamic_pointer_cast<Function_matrix<
							Scalar>,Function>(temp_variable->get_value()))
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
							)
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

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_derivative::evaluate(Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 13 January 2005
//
// DESCRIPTION :
//==============================================================================
{
#if defined (EVALUATE_RETURNS_VALUE)
	Function_handle result(0);
#else // defined (EVALUATE_RETURNS_VALUE)
	bool result(true);
#endif // defined (EVALUATE_RETURNS_VALUE)
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


#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable_composite.hpp"
#include "computed_variable/function_variable_matrix.hpp"
#include "computed_variable/function_variable_value_scalar.hpp"

// module typedefs
// ===============

typedef boost::intrusive_ptr< Function_matrix<Scalar> >
	Function_matrix_scalar_handle;

typedef boost::intrusive_ptr< Function_variable_matrix<Scalar> >
	Function_variable_matrix_scalar_handle;

// module classes
// ==============

// class Function_derivatnew_get_matrix_functor
// --------------------------------------------

class Function_derivatnew_get_matrix_functor
//******************************************************************************
// LAST MODIFIED : 7 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		Function_derivatnew_get_matrix_functor(
			Function_size_type& partial_independent_variables_index,
			Function_size_type& matrix_reverse_index,
			Function_size_type& number_of_columns,
			std::list<Function_variable_handle>& partial_independent_variables):
			partial_independent_variable_iterator(partial_independent_variables.
			rbegin()),
			partial_independent_variables_index(partial_independent_variables_index),
			matrix_reverse_index(matrix_reverse_index),
			number_of_columns(number_of_columns)
		{
			matrix_reverse_index=0;
			partial_independent_variables_index=partial_independent_variables.size();
			number_of_columns=1;
		};
		int operator() (Function_variable_handle& independent_variable)
		{
			bool found;

			matrix_reverse_index *= 2;
			if (partial_independent_variables_index>0)
			{
				found=equivalent(*partial_independent_variable_iterator,
					independent_variable);
			}
			else
			{
				found=false;
			}
			if (found)
			{
				number_of_columns *= (*partial_independent_variable_iterator)->
					number_differentiable();
				partial_independent_variables_index--;
				partial_independent_variable_iterator++;
			}
			else
			{
				matrix_reverse_index += 1;
			}

			return (0);
		}
	private:
		std::list<Function_variable_handle>::reverse_iterator
			partial_independent_variable_iterator;
		Function_size_type& partial_independent_variables_index;
		Function_size_type& matrix_reverse_index;
		Function_size_type& number_of_columns;
};


// class Function_variable_derivatnew
// ----------------------------------

// forward declaration so that can use _handle
class Function_variable_derivatnew;
typedef boost::intrusive_ptr<Function_variable_derivatnew>
	Function_variable_derivatnew_handle;

class Function_variable_derivatnew :
	public Function_variable_matrix<Scalar>
//******************************************************************************
// LAST MODIFIED : 21 January 2005
//
// DESCRIPTION :
// <column> and <row> start from one when referencing a matrix entry.  Zero
// indicates all.
//==============================================================================
{
	friend class Function_derivatnew;
	public:
		// constructor
		Function_variable_derivatnew(
			const Function_derivatnew_handle function_derivatnew,
			const std::list<Function_variable_handle>& partial_independent_variables):
			Function_variable_matrix<Scalar>(function_derivatnew,0,0),
			matrix_reverse_index(0),number_of_columns_private(0),
			number_of_rows_private(0),
			partial_independent_variables(partial_independent_variables)
		{
			if (function_derivatnew)
			{
				if (function_derivatnew->dependent_variable)
				{
					number_of_rows_private=(function_derivatnew->dependent_variable)->
						number_differentiable();
				}
				if (0<(this->partial_independent_variables).size())
				{
					Function_size_type index;

					std::for_each(
						(function_derivatnew->independent_variables).rbegin(),
						(function_derivatnew->independent_variables).rend(),
						Function_derivatnew_get_matrix_functor(index,
						matrix_reverse_index,number_of_columns_private,
						this->partial_independent_variables));
					if (0!=index)
					{
						(this->partial_independent_variables).clear();
						matrix_reverse_index=0;
						number_of_columns_private=0;
					}
				}
			}
			else
			{
				(this->partial_independent_variables).clear();
			}
		};
		Function_variable_derivatnew(
			const Function_derivatnew_handle function_derivatnew,
			const std::list<Function_variable_handle>& partial_independent_variables,
			const Function_size_type row,const Function_size_type column):
			Function_variable_matrix<Scalar>(function_derivatnew,row,column),
			matrix_reverse_index(0),number_of_columns_private(0),
			number_of_rows_private(0),
			partial_independent_variables(partial_independent_variables)
		{
			if (function_derivatnew)
			{
				if (function_derivatnew->dependent_variable)
				{
					number_of_rows_private=(function_derivatnew->dependent_variable)->
						number_differentiable();
				}
				if (0<partial_independent_variables.size())
				{
					Function_size_type index;

					//???DB.  Can be very expensive
					std::for_each(
						(function_derivatnew->independent_variables).rbegin(),
						(function_derivatnew->independent_variables).rend(),
						Function_derivatnew_get_matrix_functor(index,
						matrix_reverse_index,number_of_columns_private,
						this->partial_independent_variables));
					if (0==index)
					{
						if (row_private>number_of_rows_private)
						{
							row_private=0;
						}
						if (column_private>number_of_columns_private)
						{
							column_private=0;
						}
						if ((0!=row_private)&&(0!=column_private))
						{
							value_private=Function_variable_value_handle(
								new Function_variable_value_specific<Scalar>(
								Function_variable_matrix_set_value_function<Scalar>));
						}
					}
					else
					{
						(this->partial_independent_variables).clear();
						matrix_reverse_index=0;
							(function_derivatnew->derivative_matrix).rend();
						number_of_columns_private=0;
						row_private=0;
						column_private=0;
					}
				}
				else
				{
					row_private=0;
					column_private=0;
				}
			}
			else
			{
				(this->partial_independent_variables).clear();
				row_private=0;
				column_private=0;
			}
		};
		// destructor
		~Function_variable_derivatnew(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			Function_derivatnew_handle function_derivatnew=
				boost::dynamic_pointer_cast<Function_derivatnew,Function>(
				function());
			Function_size_type i;
			Function_variable_handle result(0);

			if (function_derivatnew)
			{
#if defined (OLD_CODE)
				std::list<Function_variable_handle>
					local_partial_independent_variables(0);
				std::list<Function_variable_handle>::const_iterator iterator;

				if (0<(i=partial_independent_variables.size()))
				{
					iterator=partial_independent_variables.begin();
					while (i>0)
					{
						if (*iterator)
						{
							local_partial_independent_variables.push_back(
								(*iterator)->clone());
						}
						else
						{
							local_partial_independent_variables.push_back(
								Function_variable_handle(0));
						}
						iterator++;
						i--;
					}
				}
				// constructor sets value_private
				result=Function_variable_handle(
					new Function_variable_derivatnew(
					function_derivatnew,local_partial_independent_variables,
					row_private,column_private));
#else // defined (OLD_CODE)
				Function_variable_derivatnew_handle temp_result;
				std::list<Function_variable_handle>
					local_partial_independent_variables(0);
				std::list<Function_variable_handle>::iterator iterator;

				// constructor sets value_private
				temp_result=Function_variable_derivatnew_handle(
					new Function_variable_derivatnew(*this));
				if (0<(i=(temp_result->partial_independent_variables).size()))
				{
					iterator=(temp_result->partial_independent_variables).begin();
					while (i>0)
					{
						if (*iterator)
						{
							*iterator=(*iterator)->clone();
						}
						iterator++;
						i--;
					}
				}
				result=temp_result;
#endif // defined (OLD_CODE)
			}

			return (result);
		};
		string_handle get_string_representation()
		{
			Function_derivatnew_handle function_derivatnew=
				boost::dynamic_pointer_cast<Function_derivatnew,Function>(
				function());
			string_handle return_string(0),temp_string;

			if (0==partial_independent_variables.size())
			{
				if (function_derivatnew)
				{
					return_string=function_derivatnew->get_string_representation();
				}
			}
			else
			{
				if (return_string=new std::string)
				{
					std::list<Function_variable_handle>::iterator independent_variable;
					std::ostringstream out;

					out << "d";
					out << "(";
					if (function_derivatnew&&
						(function_derivatnew->dependent_variable))
					{
						if (temp_string=function_derivatnew->dependent_variable->
							get_string_representation())
						{
							out << *temp_string;
							delete temp_string;
						}
					}
					out << ")/d(";
					independent_variable=partial_independent_variables.begin();
					while (independent_variable!=partial_independent_variables.end())
					{
						if (temp_string=(*independent_variable)->
							get_string_representation())
						{
							out << *temp_string;
							delete temp_string;
						}
						independent_variable++;
						if (independent_variable!=partial_independent_variables.end())
						{
							out << ",";
						}
					}
					out << ")";
#if defined (OLD_CODE)
					if ((0!=row_private)||(0!=column_private))
					{
						out << "[";
						if (0!=row_private)
						{
							out << row_private;
						}
						else
						{
							out << "*";
						}
						out << ",";
						if (0!=column_private)
						{
							out << column_private;
						}
						else
						{
							out << "*";
						}
						out << "]";
					}
#endif // defined (OLD_CODE)
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
			}

			return (return_string);
		};
#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
		bool evaluate()
		{
			Function_derivatnew_handle function_derivatnew=
				boost::dynamic_pointer_cast<Function_derivatnew,Function>(
				function());
			Function_size_type i;
			std::list<Function_variable_handle>::iterator
				partial_independent_variable_iterator;

			// recalculate number of rows and columns
			if (function_derivatnew)
			{
				number_of_rows_private=(function_derivatnew->dependent_variable)->
					number_differentiable();
			}
			else
			{
				number_of_rows_private=0;
			}
			i=partial_independent_variables.size();
			if (0<i)
			{
				number_of_columns_private=1;
				partial_independent_variable_iterator=
					partial_independent_variables.begin();
				while (i>0)
				{
					number_of_columns_private *=
						(*partial_independent_variable_iterator)->number_differentiable();
					partial_independent_variable_iterator++;
					i--;
				}
			}
			else
			{
				number_of_columns_private=0;
			}

			return (Function_variable_matrix<Scalar>::evaluate());
		};
#endif // defined (EVALUATE_RETURNS_VALUE)
		Function_variable_matrix_scalar_handle operator()(
			Function_size_type row,Function_size_type column) const
		{
			Function_variable_matrix_scalar_handle result(0);

			if ((row<=number_of_rows())&&(column<=number_of_columns()))
			{
				result=Function_variable_matrix_scalar_handle(
					new Function_variable_derivatnew(
					boost::dynamic_pointer_cast<Function_derivatnew,Function>(
					function_private),partial_independent_variables,row,column));
			}

			return (result);
		};
		Function_size_type number_of_rows() const
		{
			return (number_of_rows_private);
		};
		Function_size_type number_of_columns() const
		{
			return (number_of_columns_private);
		};
		bool get_entry(Scalar& value) const
		{
			bool result;
			Function_derivatnew_handle function_derivatnew=
				boost::dynamic_pointer_cast<Function_derivatnew,Function>(
				function());

			result=false;
			if (function_derivatnew&&(matrix_reverse_index<(function_derivatnew->
				derivative_matrix).size()))
			{
				Function_size_type i;
				std::list<Matrix>::reverse_iterator matrix_reverse_iterator;

				matrix_reverse_iterator=
					(function_derivatnew->derivative_matrix).rbegin();
				for (i=matrix_reverse_index;i>0;i--)
				{
					matrix_reverse_iterator++;
				}
				value=(*matrix_reverse_iterator)(row_private-1,column_private-1);
				result=true;
			}

			return (result);
		};
	private:
		bool equality_atomic(const Function_variable_handle& variable) const
		{
			bool result;
			Function_variable_derivatnew_handle
				variable_derivatnew;

			result=false;
			if (variable_derivatnew=boost::dynamic_pointer_cast<
				Function_variable_derivatnew,Function_variable>(variable))
			{
				if (equivalent(function(),variable_derivatnew->function())&&
					(row_private==variable_derivatnew->row_private)&&
					(column_private==variable_derivatnew->column_private))
				{
					std::list<Function_variable_handle>::const_iterator
						variable_iterator_1,variable_iterator_1_end,variable_iterator_2,
						variable_iterator_2_end;

					variable_iterator_1=partial_independent_variables.begin();
					variable_iterator_1_end=partial_independent_variables.end();
					variable_iterator_2=
						variable_derivatnew->partial_independent_variables.begin();
					variable_iterator_2_end=
						variable_derivatnew->partial_independent_variables.end();
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

			return (result);
		};
	private:
		// copy constructor
		Function_variable_derivatnew(
			const Function_variable_derivatnew& variable):
			Function_variable_matrix<Scalar>(variable),
			matrix_reverse_index(variable.matrix_reverse_index),
			number_of_columns_private(variable.number_of_columns_private),
			number_of_rows_private(variable.number_of_rows_private),
			partial_independent_variables(variable.partial_independent_variables){};
	private:
		Function_size_type matrix_reverse_index,number_of_columns_private,
			number_of_rows_private;
		std::list<Function_variable_handle> partial_independent_variables;
		// if zero then all
		Function_variable_handle atomic_dependent_variable;
		// if empty list then all.  If zero list entry then all for that independent
		//   variable
		std::list<Function_variable_handle> atomic_independent_variables;
};


// global classes
// ==============

// class Function_derivatnew
// -------------------------

Function_derivatnew::Function_derivatnew(
	const Function_variable_handle& dependent_variable,
	const std::list<Function_variable_handle>& independent_variables):
	Function(),derivative_matrix(),dependent_variable(dependent_variable),
	independent_variables(independent_variables)
//******************************************************************************
// LAST MODIFIED : 27 January 2005
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (dependent_variable)
	{
		dependent_variable->add_dependent_function(this);
	}
}

Function_derivatnew::~Function_derivatnew()
//******************************************************************************
// LAST MODIFIED : 27 January 2005
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
#if defined (CIRCULAR_SMART_POINTERS)
	// do nothing
#else // defined (CIRCULAR_SMART_POINTERS)
	if (dependent_variable)
	{
		dependent_variable->remove_dependent_function(this);
	}
#endif // defined (CIRCULAR_SMART_POINTERS)
}

string_handle Function_derivatnew::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 21 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string,temp_string;

	if (return_string=new std::string)
	{
		std::list<Function_variable_handle>::iterator independent_variable;
		std::ostringstream out;

		out << "d(";
		if (dependent_variable)
		{
			if (temp_string=dependent_variable->get_string_representation())
			{
				out << *temp_string;
				delete temp_string;
			}
		}
		out << ")/d(";
		independent_variable=independent_variables.begin();
		while (independent_variable!=independent_variables.end())
		{
			if (temp_string=(*independent_variable)->get_string_representation())
			{
				out << *temp_string;
				delete temp_string;
			}
			independent_variable++;
			if (independent_variable!=independent_variables.end())
			{
				out << ",";
			}
		}
		out << ")";
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_handle Function_derivatnew::input()
//******************************************************************************
// LAST MODIFIED : 21 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle dependent_variable_function;
	Function_variable_handle result(0);

	if (dependent_variable_function=dependent_variable->function())
	{
		result=dependent_variable_function->input();
	}

	return (result);
}

Function_variable_handle Function_derivatnew::output()
//******************************************************************************
// LAST MODIFIED : 20 January 2005
//
// DESCRIPTION :
//==============================================================================
{
#if defined (OLD_CODE)
	Function_size_type number_of_composites;
	Function_variable_handle result(0);
	std::list<Function_variable_handle> composite_variables_list(0);
	std::list<Function_variable_handle>::iterator independent_variable_iterator;
	std::list< std::list<Function_variable_handle> >
		matrix_independent_variables_list(0);
	std::list< std::list<Function_variable_handle> >::iterator last,
		matrix_independent_variables_iterator;

	for (independent_variable_iterator=independent_variables.begin();
		independent_variable_iterator!=independent_variables.end();
		independent_variable_iterator++)
	{
		Function_variable_handle independent_variable=
			*independent_variable_iterator;
		std::list<Function_variable_handle> matrix_independent_variables(0);

		matrix_independent_variables.push_back(independent_variable);
		composite_variables_list.push_back(Function_variable_handle(
			new Function_variable_derivatnew(
			Function_derivatnew_handle(this),matrix_independent_variables)));
		matrix_independent_variables_list.push_back(matrix_independent_variables);
		last=matrix_independent_variables_list.end();
		last--;
		for (matrix_independent_variables_iterator=
			matrix_independent_variables_list.begin();
			matrix_independent_variables_iterator!=last;
			matrix_independent_variables_iterator++)
		{
			matrix_independent_variables= *matrix_independent_variables_iterator;
			matrix_independent_variables.push_back(independent_variable);
			composite_variables_list.push_back(Function_variable_handle(
				new Function_variable_derivatnew(
				Function_derivatnew_handle(this),
				matrix_independent_variables)));
		}
	}
	if (0<(number_of_composites=composite_variables_list.size()))
	{
		if (1==number_of_composites)
		{
			result=Function_variable_handle(composite_variables_list.front());
		}
		else
		{
			result=Function_variable_handle(new Function_variable_composite(
				Function_derivatnew_handle(this),composite_variables_list));
		}
	}

	return (result);
#endif // defined (OLD_CODE)
	return (matrix(independent_variables));
}

Function_variable_handle Function_derivatnew::matrix(
	std::list<Function_variable_handle>& partial_independent_variables)
//******************************************************************************
// LAST MODIFIED : 23 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_derivatnew_handle(new Function_variable_derivatnew(
		Function_derivatnew_handle(this),partial_independent_variables)));
}

#if defined (OLD_CODE)
//???DB.  compose and inverse
// - it looks like Function_derivatnew needs to inherit from
//   Function_derivative_matrix or be able to be constant
// - could move to evaluate for Function_derivatnew_composition and
//   Function_derivatnew_inverse respectively
// - could have Derivative_matrix class and make Function_derivatnew have a
//   member of this type (replaces matrices)
Function_derivatnew_handle Function_derivatnew_compose(
	const Function_variable_handle&
	//???DB.  To be done
//	dependent_variable
	,
	const Function_derivatnew_handle& derivative_f,
	const Function_derivatnew_handle& derivative_g)
//******************************************************************************
// LAST MODIFIED : 9 December 2004
//
// DESCRIPTION :
// This function implements the chain rule for differentiation.
//
// If
//   Y=G(X), H(X)=F(G(X))
// then
//   dH/dXi=
//     sum{p=1,m:dF/dYp*dGp/dXi}
//   d2H/dXidXj=
//     sum{p=1,m:sum{q=1,m:d2F/dYpdYq*dGp/dXi*dGq/dXj}}+
//     sum{p=1,m:dF/dYp*d2Gp/dXidXj}
//   d3H/dXidXjdXk=
//     sum{p=1,m:sum{q=1,m:sum{r=1,m:d3F/dYpdYqdYr*dGp/dXi*dGq/dXj*dGr/dXk}}}+
//     sum{p=1,m:sum{q=1,m:d2F/dYpdYq*
//       [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]}}+
//     sum{p=1,m:dF/dYp*d3Gp/dXidXjdXk}
//   d4H/dXidXjdXkdXl=
//     sum{p=1,m:sum{q=1,m:sum{r=1,m:sum{s=1,m:
//       d4F/dYpdYqdYrdYs*dGp/dXi*dGq/dXj*dGr/dXk*dGr/dXl}}}}+
//     sum{p=1,m:sum{q=1,m:sum{r=1,m:d3F/dYpdYqdYr*
//       [d2Gp/dXidXj*dGq/dXk*dGr/dXl+
//       d2Gp/dXidXk*dGq/dXj*dGr/dXl+
//       d2Gp/dXidXl*dGq/dXj*dGr/dXk+
//       dGp/dXi*d2Gq/dXjdXk*dGr/dXl+
//       dGp/dXi*d2Gq/dXjdXl*dGr/dXk+
//       dGp/dXi*dGq/dXj*d2Gr/dXkdXl]}}}+
//     sum{p=1,m:sum{q=1,m:d2F/dYpdYq*
//       [d3Gp/dXidXjdXk*dGq/dXl+
//       d3Gp/dXidXjdXl*dGq/dXk+
//       d3Gp/dXidXkdXl*dGq/dXj+
//       dGp/dXi*d3Gq/dXjdXkdXl+
//       d2Gp/dXidXj*d2Gq/dXkdXl+
//       d2Gp/dXidXk*d2Gq/dXjdXl+
//       d2Gp/dXidXl*d2Gq/dXjdXk]}}+
//     sum{p=1,m:dF/dYp*d4Gp/dXidXjdXkdXl}
//   ...
// where m=length(Y).
//
// There are some parts above which don't look symmetric eg the p's and q's in
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]
// However for "reasonable" functions the order of differentiation does not
// change the result.  Assuming the functions are "reasonable" gives
//   d2F/dYpdYq=d2F/dYqdYp
// and so
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]
// can be replaced by
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+d2Gp/dXjdXk*dGq/dXi]
// even though they are not equal because
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+d2Gp/dXjdXk*dGq/dXi]+
//   [d2Gq/dXidXj*dGp/dXk+d2Gq/dXidXk*dGp/dXj+d2Gq/dXjdXk*dGp/dXi]
//   =
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]+
//   [d2Gq/dXidXj*dGp/dXk+d2Gq/dXidXk*dGp/dXj+dGq/dXi*d2Gp/dXjdXk]
//
// To calculate a derivative of H, the derivatives of all orders of F and G are
// needed and so derivatives of all orders are stored in the derivative matrices
// in the following order
//   d/dX,d/dY,d2/dXdY,d/dZ,d2/dXdZ,d2/dYdZ,d3/dXdYdZ
// NB.  This assumes that order of differentiation does not change the result
//
// In this function, f is the <dependent_variable> and g is the source
// variables for the composition variable.
//==============================================================================
{
	Function_derivatnew_handle result(0);
	Function_size_type i;

	if (derivative_f&&derivative_g&&
		(0<(i=(derivative_f->independent_variables).size())))
	{
		std::list<Function_variable_handle>::iterator independent_variable_iterator;
		Function_variable_handle last_independent_variable;

		// check that all derivative_f independent variables are the same
		if (last_independent_variable=(derivative_f->independent_variables).back())
		{
			independent_variable_iterator=
				(derivative_f->independent_variables).begin();
			i--;
			while ((i>0)&&(*independent_variable_iterator)&&
				equivalent(*independent_variable_iterator,last_independent_variable))
			{
				independent_variable_iterator++;
				i--;
			}
		}
		if (0==i)
		{
			bool found;
			bool *not_used;
			Scalar product,sum;
			std::list<Matrix> matrices_result(0);
			std::list<Matrix>::iterator matrix_f,matrix_g,matrix_result;
			Function_size_type column_number_result,index_result,j,k,l,
				number_of_columns_f,number_of_columns_result,
				number_of_intermediate_values,number_of_matrices,number_of_rows,order,
				offset_f,offset_g,order_result,p,q,r,row_number,s;
			Function_size_type *column_numbers_g,*index_f,*index_g,*mapping_g,
				*mapping_result,*numbers_of_independent_values,*product_orders,*order_g,
				*sub_order_g;
			std::list<Matrix>::iterator *matrices_g;

			// initialize
			number_of_rows=((derivative_f->matrices).front()).size1();
			number_of_intermediate_values=((derivative_g->matrices).front()).size1();
			independent_variable_iterator=
				(derivative_g->independent_variables).begin();
			order=(derivative_g->independent_variables).size();
			not_used=new bool[order+1];
			column_numbers_g=new Function_size_type[order+1];
			index_f=new Function_size_type[order+1];
			index_g=new Function_size_type[order+1];
			mapping_g=new Function_size_type[order+1];
			mapping_result=new Function_size_type[order+1];
			numbers_of_independent_values=new Function_size_type[order];
			product_orders=new Function_size_type[order+1];
			order_g=new Function_size_type[order+1];
			sub_order_g=new Function_size_type[order+1];
			matrices_g=new std::list<Matrix>::iterator[order+1];
			if (not_used&&column_numbers_g&&index_f&&index_g&&mapping_g&&
				mapping_result&&numbers_of_independent_values&&product_orders&&order_g&&
				sub_order_g&&matrices_g)
			{
				number_of_matrices=1;
				matrix_g=(derivative_g->matrices).begin();
				for (i=0;i<order;i++)
				{
					Assert(number_of_matrices<=(derivative_g->matrices).size(),
						std::logic_error("Function_derivatnew_compose.  "
						"Invalid number of matrices for derivative g"));
					numbers_of_independent_values[i]=matrix_g->size2();
					for (j=number_of_matrices;j>0;j--)
					{
						Matrix new_matrix(number_of_rows,matrix_g->size2());

						matrices_result.push_back(new_matrix);
						matrix_g++;
					}
					number_of_matrices *= 2;
				}
				number_of_matrices -= 1;
				Assert((number_of_matrices==(derivative_f->matrices).size())&&
					(number_of_matrices==(derivative_g->matrices).size()),
					std::logic_error("Function_derivatnew_compose.  "
					"Invalid number of matrices"));
				/* loop over dependent values (rows) */
				row_number=0;
				while (row_number<number_of_rows)
				{
					/* loop over derivatives (matrices) */
					matrix_result=matrices_result.begin();
					// index_result stores which derivative (matrix) of result (H) is
					//   being calculated
					index_result=1;
					while (index_result<=number_of_matrices)
					{
						// determine the derivative being evaluated and calculate its order
						//   and number of values
						order_result=0;
						number_of_columns_result=1;
						j=index_result;
						i=0;
						while (j>0)
						{
							if (j%2)
							{
								index_g[order_result]=0;
								mapping_result[order_result]=i;
								order_result++;
								number_of_columns_result *= numbers_of_independent_values[i];
							}
							j /= 2;
							i++;
						}
						// calculate the values for the derivative
						column_number_result=0;
						while (column_number_result<number_of_columns_result)
						{
							// loop over the sums for different order derivatives of f
							sum=(Scalar)0;
							matrix_f=(derivative_f->matrices).begin();
							offset_f=1;
							j=0;
							while (j<order_result)
							{
								// initialize the orders for the derivatives of g that are
								//   multiplied together.  There are j+1 derivatives of g and
								//   their orders have to sum to the order of the derivative of
								//   result (order_result)
								for (l=0;l<j;l++)
								{
									product_orders[l]=1;
								}
								product_orders[j]=order_result-j;
								// loop over the possible ways of dividing the order_result
								//   independent variables, in mapping_result, into j+1
								//   non-empty sets, where the order of the sets and the order
								//   within the sets are not important.  For each possible way,
								//   loop across the row of matrix_f and down the columns of
								//   matrix_g and
								//   - calculate the product of the derivatives of g represented
								//     by the sets
								//   - multiply the product by the corresponding derivative of f
								//   - add the result to result
								do
								{
									// initialize the variable assigment
									// for each of the independent variables being differentiated
									//   with respect to in the product have:
									//   mapping_g - a reordering of the variables without
									//     changing the orders of the partial derivatives.  It is
									//     a location in order_g and sub_order_g
									//   order_g - the order of the partial derivative they're in
									//   sub_order_g - their position in the variables in partial
									//     derivatives of the same order
									r=0;
									l=0;
									while (l<=j)
									{
										q=0;
										do
										{
											for (p=0;p<product_orders[l];p++)
											{
												mapping_g[r]=r;
												order_g[r]=product_orders[l];
												sub_order_g[r]=q;
												r++;
												q++;
											}
											l++;
										} while ((l<=j)&&(product_orders[l]==product_orders[l-1]));
									}
									// find the column numbers of matrix g for the partial
									//   derivatives in the product
									// r is the number of the partial derivative within partial
									//   derivatives of the same order
									r=0;
									for (l=0;l<=j;l++)
									{
										// initialize the value position within the partial
										//   derivative of f
										index_f[l]=0;
										// determine which independent variables are used in the
										//   partial derivative of g
										matrix_g=(derivative_g->matrices).begin();
										offset_g=1;
										for (p=0;p<order_result;p++)
										{
											q=mapping_g[p];
											// is the same partial derivative within the partial
											//   derivatives of the same order
											if ((product_orders[j]==order_g[q])&&
												(r==sub_order_g[q]/order_g[q]))
											{
												not_used[p]=false;
											}
											else
											{
												not_used[p]=true;
											}
											//???DB.  matrix_g += offset_g;
											for (s=offset_g;s>0;s--)
											{
												matrix_g++;
											}
											offset_g *= 2;
										}
										matrix_g--;
										for (p=order_result;p>0;p--)
										{
											offset_g /= 2;
											if (not_used[p-1])
											{
												//???DB.  matrix_g -= offset_g;
												for (s=offset_g;s>0;s--)
												{
													matrix_g--;
												}
											}
										}
										matrices_g[l]=matrix_g;
										// second the index of the value
										column_numbers_g[l]=0;
										for (p=0;p<order_result;p++)
										{
											if (!not_used[p])
											{
												column_numbers_g[l]=numbers_of_independent_values[
													mapping_result[p]]*column_numbers_g[l]+index_g[p];
											}
										}
										// increment r (the number of the partial derivative within
										//   partial derivatives of the same order
										if ((l<j)&&(product_orders[l]==product_orders[l+1]))
										{
											r++;
										}
										else
										{
											r=0;
										}
									}
									number_of_columns_f=matrix_f->size2();
									// loop across the row of matrix_f and down the columns of
									//   matrix_g
									k=0;
									while (k<number_of_columns_f)
									{
										// calculate the product
										product=(*matrix_f)(row_number,k);
										l=0;
										while (l<=j)
										{
											product *=
												(*matrices_g[l])(index_f[l],column_numbers_g[l]);
											l++;
										}
										// add to sum
										sum += product;
										// increment to next value for derivative in matrix_f and
										//   matrix_g
										k++;
										l=j;
										index_f[l]++;
										while ((l>0)&&(index_f[l]>=number_of_intermediate_values))
										{
											index_f[l]=0;
											l--;
											index_f[l]++;
										}
									}
									// move to the next choice for the j+1 sets
									// first try leaving the number of variables in each set the
									//   same (don't change product_orders).  Do this by running
									//   through the permutations of order_result things with
									//   restrictions
									//   - start with the permutation 0,1,2, ... order_result-1
									//   - for the current permutation
									//   - start at the end and run back looking for an entry
									//     which is less than one of the entries further on and
									//     for which the restrictions hold
									//   - find the smallest entry that is further on than the
									//     current entry, greater than the current entry and
									//     satisfies the restrictions
									//   - put the smallest in the current and add the remaining
									//     entries in increasing order
									l=order_result-1;
									q=mapping_g[l];
									found=false;
									while ((l>0)&&!found)
									{
										p=q;
										l--;
										q=mapping_g[l];
										// check if there is a value further on with a greater index
										//   (unrestricted permutations)
										if (q<p)
										{
											// apply restrictions
											// if have same order
											if (order_g[p]==order_g[q])
											{
												// check that p and q are not in the same partial
												//   derivative and that second set doesn't have values
												//   less than the first value of q's set
												// the order of sets of the same size being unimportant
												//   is equivalent to having the first value of the
												//   first set always less than all the values of the
												//   second set
												if ((sub_order_g[p]/order_g[p]!=
													sub_order_g[q]/order_g[p])&&
													(0!=sub_order_g[q]%order_g[p]))
												{
													found=true;
												}
											}
											else
											{
												// check that q hasn't been tried in a partial
												//   derivative of order_g[p]
												if (order_g[q]<order_g[p])
												{
													found=true;
												}
											}
										}
									}
									if (found)
									{
										// mark as unused the values after l
										for (p=0;p<order_result;p++)
										{
											not_used[p]=false;
										}
										for (p=l;p<order_result;p++)
										{
											not_used[mapping_g[p]]=true;
										}
										q=mapping_g[l];
										p=q;
										found=false;
										// find the smallest valid value after l which is greater
										//   than mapping_g[l]
										do
										{
											p++;
											if (not_used[p])
											{
												if (order_g[p]==order_g[q])
												{
													if ((sub_order_g[p]/order_g[p]!=
														sub_order_g[q]/order_g[p])&&
														(0!=sub_order_g[q]%order_g[p]))
													{
														found=true;
													}
												}
												else
												{
													if (order_g[p]>order_g[q])
													{
														found=true;
													}
												}
											}
										} while (!found);
										// put the smallest value in l
										mapping_g[l]=p;
										not_used[p]=false;
										// put the unused values in increasing order after l
										for (p=0;p<order_result;p++)
										{
											if (not_used[p])
											{
												l++;
												mapping_g[l]=p;
											}
										}
									}
									else
									{
										// look for another choice of the j+1 set sizes.  Having the
										//   order of the sets being unimportant is equivalent to
										//   having the sizes in non-decreasing order and starting
										//   with sizes 1,2,...order_result-j
										l=j;
										while ((l>0)&&
											(product_orders[l]==product_orders[l-1]))
										{
											l--;
										}
										if (l>0)
										{
											(product_orders[l])--;
											while ((l>0)&&
												(product_orders[l]==product_orders[l-1]))
											{
												l--;
											}
											if (l>0)
											{
												// have found a new choice of set sizes re-initialize
												//   the variable assignment
												(product_orders[l-1])++;
											}
										}
									}
								} while (l>0);
								offset_f *= 2;
								//???DB.  matrix_f += offset_f;
								for (s=offset_f;s>0;s--)
								{
									matrix_f++;
								}
								j++;
							}
							(*matrix_result)(row_number,column_number_result)=sum;
							// increment to next value for derivative in matrix_g
							j=order_result-1;
							index_g[j]++;
							k=mapping_result[j];
							while ((j>0)&&(index_g[j]>=numbers_of_independent_values[k]))
							{
								index_g[j]=0;
								j--;
								k=mapping_result[j];
								index_g[j]++;
							}
							// increment to next column in derivative (matrix) of result
							column_number_result++;
						}
						// increment to next derivative (matrix)
						index_result++;
						matrix_result++;
					}
					// increment to next row
					row_number++;
				}
				//???DB.  To be done
//				result=Function_derivatnew_handle(new Function_derivatnew(
//					dependent_variable,derivative_g->independent_variables,
//					matrices_result));
			}
			delete [] not_used;
			delete [] column_numbers_g;
			delete [] index_f;
			delete [] index_g;
			delete [] mapping_g;
			delete [] mapping_result;
			delete [] numbers_of_independent_values;
			delete [] product_orders;
			delete [] order_g;
			delete [] sub_order_g;
			delete [] matrices_g;
		}
	}

	return (result);
}

Function_handle Function_derivatnew::inverse()
//******************************************************************************
// LAST MODIFIED : 9 December 2004
//
// DESCRIPTION :
// Compute the composition inverse from the chain rule and the relation
//   identity=U_inv(U(X))
// Then using
//   H=identity
//   F=U_inv
//   G=U
// in Function_derivatnew_compose gives
//   dU_inv/dY=(dU/dX)^(-1)
//   d2U_inv/dY2=
//     -[sum{p=1,m:dF/dYp*d2Gp/dXidXj}]
//     *(dU/dX)^(-1)*(dU/dX)^(-1)
//   d3U_inv/dY3=
//     -[sum{p=1,m:sum{q=1,m:d2F/dYpdYq*
//       [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]}}+
//     sum{p=1,m:dF/dYp*d3Gp/dXidXjdXk}]
//     *(dU/dX)^(-1)*(dU/dX)^(-1)*(dU/dX)^(-1)
//   d4U_inv/dY4=
//     -[sum{p=1,m:sum{q=1,m:sum{r=1,m:d3F/dYpdYqdYr*
//       [d2Gp/dXidXj*dGq/dXk*dGr/dXl+
//       d2Gp/dXidXk*dGq/dXj*dGr/dXl+
//       d2Gp/dXidXl*dGq/dXj*dGr/dXk+
//       dGp/dXi*d2Gq/dXjdXk*dGr/dXl+
//       dGp/dXi*d2Gq/dXjdXl*dGr/dXk+
//       dGp/dXi*dGq/dXj*d2Gr/dXkdXl]}}}+
//     sum{p=1,m:sum{q=1,m:d2F/dYpdYq*
//       [d3Gp/dXidXjdXk*dGq/dXl+
//       d3Gp/dXidXjdXl*dGq/dXk+
//       d3Gp/dXidXkdXl*dGq/dXj+
//       dGp/dXi*d3Gq/dXjdXkdXl+
//       d2Gp/dXidXj*d2Gq/dXkdXl+
//       d2Gp/dXidXk*d2Gq/dXjdXl+
//       d2Gp/dXidXl*d2Gq/dXjdXk]}}+
//     sum{p=1,m:dF/dYp*d4Gp/dXidXjdXkdXl}]
//     *(dU/dX)^(-1)*(dU/dX)^(-1)*(dU/dX)^(-1)
//   ...
// where m=length(Y).
//
//
// If
//   Y=G(X), H(X)=F(G(X))
// then
//   dH/dXi=
//     sum{p=1,m:dF/dYp*dGp/dXi}
//   d2H/dXidXj=
//     sum{p=1,m:sum{q=1,m:d2F/dYpdYq*dGp/dXi*dGq/dXj}}+
//     sum{p=1,m:dF/dYp*d2Gp/dXidXj}
//   d3H/dXidXjdXk=
//     sum{p=1,m:sum{q=1,m:sum{r=1,m:d3F/dYpdYqdYr*dGp/dXi*dGq/dXj*dGr/dXk}}}+
//     sum{p=1,m:sum{q=1,m:d2F/dYpdYq*
//       [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]}}+
//     sum{p=1,m:dF/dYp*d3Gp/dXidXjdXk}
//   d4H/dXidXjdXkdXl=
//     sum{p=1,m:sum{q=1,m:sum{r=1,m:sum{s=1,m:
//       d4F/dYpdYqdYrdYs*dGp/dXi*dGq/dXj*dGr/dXk*dGr/dXl}}}}+
//     sum{p=1,m:sum{q=1,m:sum{r=1,m:d3F/dYpdYqdYr*
//       [d2Gp/dXidXj*dGq/dXk*dGr/dXl+
//       d2Gp/dXidXk*dGq/dXj*dGr/dXl+
//       d2Gp/dXidXl*dGq/dXj*dGr/dXk+
//       dGp/dXi*d2Gq/dXjdXk*dGr/dXl+
//       dGp/dXi*d2Gq/dXjdXl*dGr/dXk+
//       dGp/dXi*dGq/dXj*d2Gr/dXkdXl]}}}+
//     sum{p=1,m:sum{q=1,m:d2F/dYpdYq*
//       [d3Gp/dXidXjdXk*dGq/dXl+
//       d3Gp/dXidXjdXl*dGq/dXk+
//       d3Gp/dXidXkdXl*dGq/dXj+
//       dGp/dXi*d3Gq/dXjdXkdXl+
//       d2Gp/dXidXj*d2Gq/dXkdXl+
//       d2Gp/dXidXk*d2Gq/dXjdXl+
//       d2Gp/dXidXl*d2Gq/dXjdXk]}}+
//     sum{p=1,m:dF/dYp*d4Gp/dXidXjdXkdXl}
//   ...
// where m=length(Y).
//
// There are some parts above which don't look symmetric eg the p's and q's in
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]
// However for "reasonable" functions the order of differentiation does not
// change the result.  Assuming the functions are "reasonable" gives
//   d2F/dYpdYq=d2F/dYqdYp
// and so
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]
// can be replaced by
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+d2Gp/dXjdXk*dGq/dXi]
// even though they are not equal because
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+d2Gp/dXjdXk*dGq/dXi]+
//   [d2Gq/dXidXj*dGp/dXk+d2Gq/dXidXk*dGp/dXj+d2Gq/dXjdXk*dGp/dXi]
//   =
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]+
//   [d2Gq/dXidXj*dGp/dXk+d2Gq/dXidXk*dGp/dXj+dGq/dXi*d2Gp/dXjdXk]
//
// To calculate a derivative of H, the derivatives of all orders of F and G are
// needed and so derivatives of all orders are stored in the derivative matrices
// in the following order
//   d/dX,d/dY,d2/dXdY,d/dZ,d2/dXdZ,d2/dYdZ,d3/dXdYdZ
// NB.  This assumes that order of differentiation does not change the result
//
// In this function, f is the <dependent_variable> and g is the source
// variables for the composition variable.
//==============================================================================
{
	Function_derivatnew_handle derivative(this);
	Function_derivatnew_handle result(0);
	Function_size_type i;

	if (derivative&&(0<(i=(derivative->independent_variables).size()))&&
		dependent_variable)
	{
		std::list<Function_variable_handle> inverse_independent_variables;
		std::list<Function_variable_handle>::iterator independent_variable_iterator;
		Function_variable_handle inverse_dependent_variable,
			inverse_independent_variable;

		// check that all derivative independent variables are the same
		if ((inverse_independent_variable=derivative->dependent_variable)&&
			(inverse_dependent_variable=(derivative->independent_variables).front()))
		{
			independent_variable_iterator=(derivative->independent_variables).begin();
			while ((i>0)&&(*independent_variable_iterator)&&
				equivalent(*independent_variable_iterator,inverse_dependent_variable))
			{
				inverse_independent_variables.push_back(inverse_independent_variable);
				independent_variable_iterator++;
				i--;
			}
		}
		if ((0==i)&&(((derivative->matrices).front()).size1()==
			((derivative->matrices).front()).size2()))
		{
			bool found;
			bool *not_used;
			Scalar product,sum;
			std::list<Matrix> matrices_result(0);
			std::list<Matrix>::iterator matrix_f,matrix_g;
			Function_size_type column_number_result,j,k,l,number_of_columns_f,
				number_of_columns_result,number_of_rows,number_of_steps,offset,offset_f,
				offset_g,order_result,p,q,r,row_number,s,step;
			Function_size_type number_of_matrices=(derivative->matrices).size();
			Function_size_type order=(derivative->independent_variables).size();
			Function_size_type *column_numbers_g,*index_f,*index_g,*mapping_g,
				*product_orders,*order_g,*sub_order_g;
			std::list<Matrix>::iterator *matrices_g;
			ublas::vector<Function_size_type> matrix_orders(number_of_matrices);
			ublas::vector<Matrix> order_matrices(order);

			// initialize
			number_of_rows=((derivative->matrices).front()).size1();
			not_used=new bool[order+1];
			column_numbers_g=new Function_size_type[order+1];
			index_f=new Function_size_type[order+1];
			index_g=new Function_size_type[order+1];
			mapping_g=new Function_size_type[order+1];
			product_orders=new Function_size_type[order+1];
			order_g=new Function_size_type[order+1];
			sub_order_g=new Function_size_type[order+1];
			matrices_g=new std::list<Matrix>::iterator[order+1];
			if (not_used&&column_numbers_g&&index_f&&index_g&&mapping_g&&
				product_orders&&order_g&&sub_order_g&&matrices_g)
			{
				Matrix& matrix_inverse=order_matrices[0];
				Function_matrix_scalar_handle matrix_inverse_handle(0);

				// calculate first order derivative
				matrix_inverse.resize(number_of_rows,number_of_rows);
				for (i=0;i<number_of_rows;i++)
				{
					for (j=0;j<number_of_rows;j++)
					{
						if (i==j)
						{
							matrix_inverse(i,j)=(Scalar)1;
						}
						else
						{
							matrix_inverse(i,j)=(Scalar)0;
						}
					}
				}
				if (matrix_inverse_handle=Function_matrix_scalar_handle(
					new Function_matrix<Scalar>(matrix_inverse)))
				{
					if (matrix_inverse_handle=Function_matrix<Scalar>(
						(derivative->matrices).front()).solve(matrix_inverse_handle))
					{
						for (i=0;i<number_of_rows;i++)
						{
							for (j=0;j<number_of_rows;j++)
							{
								matrix_inverse(i,j)=(*matrix_inverse_handle)(i+1,j+1);
							}
						}
					}
					matrix_inverse_handle=0;
				}
				matrices_result.push_back(matrix_inverse);
				number_of_matrices=1;
				matrix_orders[0]=0;
				order_matrices[0].resize(number_of_rows,number_of_rows);
				order_matrices[0]=matrix_inverse;
				// loop of order.  Can use order rather than matrix because all the
				//   independent variables are the same and so all matrices for an
				//   order are the same
				number_of_columns_result=number_of_rows*number_of_rows;
				order_result=2;
				while (order_result<=order)
				{
					Matrix& matrix_new=order_matrices[order_result-1];

					matrix_new.resize(number_of_rows,number_of_columns_result);
					// calculate order_result derivative
					for (i=0;i<=order_result;i++)
					{
						index_g[i]=0;
					}
					// calculate the values for the derivative
					column_number_result=0;
					while (column_number_result<number_of_columns_result)
					{
						/* loop over dependent values (rows) */
						row_number=0;
						while (row_number<number_of_rows)
						{
							// loop over the sums for different order derivatives of f
							sum=(Scalar)0;
							matrix_f=matrices_result.begin();
							offset_f=1;
							j=0;
							while (j<order_result-1)
							{
								// initialize the orders for the derivatives of g that are
								//   multiplied together.  There are j+1 derivatives of g and
								//   their orders have to sum to the order of the derivative
								//   of result (order_result)
								for (l=0;l<j;l++)
								{
									product_orders[l]=1;
								}
								product_orders[j]=order_result-j;
								// loop over the possible ways of dividing the order_result
								//   independent variables into j+1 non-empty sets, where the
								//   order of the sets and the order within the sets are not
								//   important.  For each possible way, loop across the row of
								//   matrix_f and down the columns of matrix_g and
								//   - calculate the product of the derivatives of g
								//     represented by the sets
								//   - multiply the product by the corresponding derivative of
								//     f
								//   - add the result to result
								do
								{
									// initialize the variable assigment
									// for each of the independent variables being
									//   differentiated with respect to in the product have:
									//   mapping_g - a reordering of the variables without
									//     changing the orders of the partial derivatives.  It
									//     is a location in order_g and sub_order_g
									//   order_g - the order of the partial derivative they're
									//     in
									//   sub_order_g - their position in the variables in
									//     partial derivatives of the same order
									r=0;
									l=0;
									while (l<=j)
									{
										q=0;
										do
										{
											for (p=0;p<product_orders[l];p++)
											{
												mapping_g[r]=r;
												order_g[r]=product_orders[l];
												sub_order_g[r]=q;
												r++;
												q++;
											}
											l++;
										} while ((l<=j)&&
											(product_orders[l]==product_orders[l-1]));
									}
									// find the column numbers of matrix g for the partial
									//   derivatives in the product
									// r is the number of the partial derivative within partial
									//   derivatives of the same order
									r=0;
									for (l=0;l<=j;l++)
									{
										// initialize the value position within the partial
										//   derivative of f
										index_f[l]=0;
										// determine which independent variables are used in the
										//   partial derivative of g
										matrix_g=(derivative->matrices).begin();
										offset_g=1;
										for (p=0;p<order_result;p++)
										{
											q=mapping_g[p];
											// is the same partial derivative within the partial
											//   derivatives of the same order
											if ((product_orders[j]==order_g[q])&&
												(r==sub_order_g[q]/order_g[q]))
											{
												not_used[p]=false;
											}
											else
											{
												not_used[p]=true;
											}
											//???DB.  matrix_g += offset_g;
											for (s=offset_g;s>0;s--)
											{
												matrix_g++;
											}
											offset_g *= 2;
										}
										matrix_g--;
										for (p=order_result;p>0;p--)
										{
											offset_g /= 2;
											if (not_used[p-1])
											{
												//???DB.  matrix_g -= offset_g;
												for (s=offset_g;s>0;s--)
												{
													matrix_g--;
												}
											}
										}
										matrices_g[l]=matrix_g;
										// second the index of the value
										column_numbers_g[l]=0;
										for (p=0;p<order_result;p++)
										{
											if (!not_used[p])
											{
//												column_numbers_g[l]=numbers_of_independent_values[
//													mapping_result[p]]*column_numbers_g[l]+index_g[p];
												column_numbers_g[l]=
													number_of_rows*column_numbers_g[l]+index_g[p];
											}
										}
										// increment r (the number of the partial derivative
										//   within partial derivatives of the same order
										if ((l<j)&&(product_orders[l]==product_orders[l+1]))
										{
											r++;
										}
										else
										{
											r=0;
										}
									}
									number_of_columns_f=matrix_f->size2();
									// loop across the row of matrix_f and down the columns of
									//   matrix_g
									k=0;
									while (k<number_of_columns_f)
									{
										// calculate the product
										product=(*matrix_f)(row_number,k);
										l=0;
										while (l<=j)
										{
											product *=
												(*matrices_g[l])(index_f[l],column_numbers_g[l]);
											l++;
										}
										// add to sum
										sum += product;
										// increment to next value for derivative in matrix_f and
										//   matrix_g
										k++;
										l=j;
										index_f[l]++;
										while ((l>0)&&(index_f[l]>=number_of_rows))
										{
											index_f[l]=0;
											l--;
											index_f[l]++;
										}
									}
									// move to the next choice for the j+1 sets
									// first try leaving the number of variables in each set the
									//   same (don't change product_orders).  Do this by running
									//   through the permutations of order_result things with
									//   restrictions
									//   - start with the permutation 0,1,2, ... order_result-1
									//   - for the current permutation
									//   - start at the end and run back looking for an entry
									//     which is less than one of the entries further on and
									//     for which the restrictions hold
									//   - find the smallest entry that is further on than the
									//     current entry, greater than the current entry and
									//     satisfies the restrictions
									//   - put the smallest in the current and add the remaining
									//     entries in increasing order
									l=order_result-1;
									q=mapping_g[l];
									found=false;
									while ((l>0)&&!found)
									{
										p=q;
										l--;
										q=mapping_g[l];
										// check if there is a value further on with a greater
										//   index (unrestricted permutations)
										if (q<p)
										{
											// apply restrictions
											// if have same order
											if (order_g[p]==order_g[q])
											{
												// check that p and q are not in the same partial
												//   derivative and that second set doesn't have
												//   values less than the first value of q's set
												// the order of sets of the same size being
												//   unimportant is equivalent to having the first
												//   value of the first set always less than all the
												//   values of the second set
												if ((sub_order_g[p]/order_g[p]!=
													sub_order_g[q]/order_g[p])&&
													(0!=sub_order_g[q]%order_g[p]))
												{
													found=true;
												}
											}
											else
											{
												// check that q hasn't been tried in a partial
												//   derivative of order_g[p]
												if (order_g[q]<order_g[p])
												{
													found=true;
												}
											}
										}
									}
									if (found)
									{
										// mark as unused the values after l
										for (p=0;p<order_result;p++)
										{
											not_used[p]=false;
										}
										for (p=l;p<order_result;p++)
										{
											not_used[mapping_g[p]]=true;
										}
										q=mapping_g[l];
										p=q;
										found=false;
										// find the smallest valid value after l which is greater
										//   than mapping_g[l]
										do
										{
											p++;
											if (not_used[p])
											{
												if (order_g[p]==order_g[q])
												{
													if ((sub_order_g[p]/order_g[p]!=
														sub_order_g[q]/order_g[p])&&
														(0!=sub_order_g[q]%order_g[p]))
													{
														found=true;
													}
												}
												else
												{
													if (order_g[p]>order_g[q])
													{
														found=true;
													}
												}
											}
										} while (!found);
										// put the smallest value in l
										mapping_g[l]=p;
										not_used[p]=false;
										// put the unused values in increasing order after l
										for (p=0;p<order_result;p++)
										{
											if (not_used[p])
											{
												l++;
												mapping_g[l]=p;
											}
										}
									}
									else
									{
										// look for another choice of the j+1 set sizes.  Having
										//   the order of the sets being unimportant is equivalent
										//   to having the sizes in non-decreasing order and
										//   starting with sizes 1,2,...order_result-j
										l=j;
										while ((l>0)&&
											(product_orders[l]==product_orders[l-1]))
										{
											l--;
										}
										if (l>0)
										{
											(product_orders[l])--;
											while ((l>0)&&
												(product_orders[l]==product_orders[l-1]))
											{
												l--;
											}
											if (l>0)
											{
												// have found a new choice of set sizes re-initialize
												//   the variable assignment
												(product_orders[l-1])++;
											}
										}
									}
								} while (l>0);
								offset_f *= 2;
								//???DB.  matrix_f += offset_f;
								for (s=offset_f;s>0;s--)
								{
									matrix_f++;
								}
								j++;
							}
							matrix_new(row_number,column_number_result)= -sum;
							// increment to next row
							row_number++;
						}
						// increment to next value for derivative in matrix_g
						j=order_result-1;
						index_g[j]++;
//							k=mapping_result[j];
//							while ((j>0)&&(index_g[j]>=numbers_of_independent_values[k]))
						while ((j>0)&&(index_g[j]>=number_of_rows))
						{
							index_g[j]=0;
							j--;
//								k=mapping_result[j];
							index_g[j]++;
						}
						// increment to next column in derivative (matrix) of result
						column_number_result++;
					}
					// "multiply" by inverse of first order derivative order_result times
					number_of_steps=1;
					step=column_number_result/number_of_rows;
					for (k=order_result;k>0;k--)
					{
						Matrix matrix_temp(number_of_rows,number_of_rows);

						for (l=0;l<number_of_steps;l++)
						{
							for (p=0;p<step;p++)
							{
								offset=l*number_of_rows*step+p;
								for (j=0;j<number_of_rows;j++)
								{
									for (i=0;i<number_of_rows;i++)
									{
										matrix_temp(i,j)=matrix_new(i,offset);
									}
									offset += step;
								}
								// multiply matrix_temp by matrix_inverse and put back
								offset=l*number_of_rows*step+p;
								for (j=0;j<number_of_rows;j++)
								{
									for (i=0;i<number_of_rows;i++)
									{
										sum=0;
										for (q=0;q<number_of_rows;q++)
										{
											sum += matrix_temp(i,q)*matrix_inverse(q,j);
										}
										matrix_new(i,offset)=sum;
									}
									offset += step;
								}
							}
						}
						number_of_steps *= number_of_rows;
						step /= number_of_rows;
					}
					// update matrices_result list
					k=matrices_result.size();
					matrices_result.push_back(matrix_inverse);
					matrix_orders[k]=0;
					for (i=0;i<k;i++)
					{
						matrix_orders[i+k+1]=matrix_orders[i]+1;
						matrices_result.push_back(order_matrices[matrix_orders[i+k+1]]);
					}
					number_of_columns_result *= number_of_rows;
					order_result++;
				}
				//???DB.  To be done
//				result=Function_derivatnew_handle(new Function_derivatnew(
//					inverse_dependent_variable,inverse_independent_variables,
//					matrices_result));
			}
			delete [] not_used;
			delete [] column_numbers_g;
			delete [] index_f;
			delete [] index_g;
			delete [] mapping_g;
			delete [] product_orders;
			delete [] order_g;
			delete [] sub_order_g;
			delete [] matrices_g;
		}
	}

	return (result);
}
#endif // defined (OLD_CODE)

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_derivatnew::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 20 January 2005
//
// DESCRIPTION :
// ???DB.  Eventually
// - this should be abstract and evaluate_derivative should be removed from
//   Function.  Then its handled through the derivative method of the variable
//   and the evaluate of the Function_derivatnew sub-class for the specific
//   function
// - is Function_derivatnew needed?  Only additional method is matrix()
//==============================================================================
{
#if defined (EVALUATE_RETURNS_VALUE)
	Function_handle result(0);
#else // defined (EVALUATE_RETURNS_VALUE)
	bool result(true);
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_variable_derivatnew_handle atomic_variable_derivatnew;

#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
	//???debug
	if (!evaluated())
#endif // defined (EVALUATE_RETURNS_VALUE)
	if (atomic_variable_derivatnew=boost::dynamic_pointer_cast<
		Function_variable_derivatnew,Function_variable>(atomic_variable))
	{
		if (equivalent(Function_handle(this),
			atomic_variable_derivatnew->function()))
		{
			if (!evaluated())
			{
				bool valid;
				Function_derivatnew_handle derivative_function;
				Function_variable_handle local_dependent_variable=dependent_variable;
				std::list<Function_variable_handle> local_independent_variables;

				valid=true;
				while (valid&&boost::dynamic_pointer_cast<Function_variable_derivatnew,
					Function_variable>(local_dependent_variable))
				{
					if (derivative_function=boost::dynamic_pointer_cast<
						Function_derivatnew,Function>(local_dependent_variable->function()))
					{
						local_dependent_variable=derivative_function->dependent_variable;
						local_independent_variables.insert(
							local_independent_variables.begin(),
							(derivative_function->independent_variables).begin(),
							(derivative_function->independent_variables).end());
					}
					else
					{
						valid=false;
					}
				}
				if (valid)
				{
					bool incremented_independent;
					Function_size_type i,local_order=local_independent_variables.size(),
						number_of_dependent_values=
						local_dependent_variable->number_differentiable();
					std::list<Function_variable_handle>::iterator
						independent_variable_iterator,local_independent_variable_iterator;
					std::list<Function_variable_iterator>
						atomic_local_independent_iterators_begin,
						atomic_local_independent_iterators_end;
					std::list< std::list<Function_variable_handle> >
						matrix_independent_variables;
					std::list<Matrix> matrices;

					local_independent_variable_iterator=
						local_independent_variables.begin();
					for (i=local_order;i>0;i--)
					{
						number_of_dependent_values *=
							(*local_independent_variable_iterator)->number_differentiable();
						atomic_local_independent_iterators_begin.push_back(
							(*local_independent_variable_iterator)->begin_atomic());
						atomic_local_independent_iterators_end.push_back(
							(*local_independent_variable_iterator)->end_atomic());
						local_independent_variable_iterator++;
					}
					valid=(number_of_dependent_values>0);
					independent_variable_iterator=independent_variables.begin();
					while (valid&&
						(independent_variable_iterator!=independent_variables.end()))
					{
						Function_variable_handle independent_variable=
							*independent_variable_iterator;
						Function_size_type number_of_independent_values=
							independent_variable->number_differentiable();
						std::list<Matrix>::iterator matrix_iterator,last;
						std::list< std::list<Function_variable_handle> >::iterator
							matrix_independent_variables_iterator;

						// calculate the derivative of dependent variable with respect to
						//   independent variable and add to matrix list
						{
							Function_size_type row,column;
							Function_variable_iterator atomic_dependent_variable_iterator(0),
								atomic_independent_variable_iterator(0);
							Matrix new_matrix(number_of_dependent_values,
								number_of_independent_values);
							std::list<Function_variable_handle>
								new_matrix_independent_variables;
							std::list<Function_variable_iterator>
								atomic_local_independent_iterators;
							std::list<Function_variable_iterator>::iterator
								independent_iterators_begin_iterator,
								independent_iterators_end_iterator,
								independent_iterators_iterator;

							new_matrix_independent_variables.push_back(independent_variable);
							row=0;
							independent_iterators_begin_iterator=
								atomic_local_independent_iterators_begin.begin();
							independent_iterators_end_iterator=
								atomic_local_independent_iterators_end.begin();
							for (i=local_order;i>0;--i)
							{
								atomic_independent_variable_iterator=
									*independent_iterators_begin_iterator;
								while ((atomic_independent_variable_iterator!=
									*independent_iterators_end_iterator)&&
									(1!=(*atomic_independent_variable_iterator)->
									number_differentiable()))
								{
									++atomic_independent_variable_iterator;
								}
								atomic_local_independent_iterators.push_back(
									atomic_independent_variable_iterator);
								++independent_iterators_begin_iterator;
								++independent_iterators_end_iterator;
							}
							atomic_dependent_variable_iterator=
								local_dependent_variable->begin_atomic();
							while (valid&&(atomic_dependent_variable_iterator!=
								local_dependent_variable->end_atomic()))
							{
								Function_variable_handle atomic_dependent_variable=
									*atomic_dependent_variable_iterator;

								if (1==atomic_dependent_variable->number_differentiable())
								{
									std::list<Function_variable_handle>
										new_matrix_atomic_independent_variables;

									Assert(atomic_dependent_variable&&
										(atomic_dependent_variable->function()),std::logic_error(
										"Function_derivatnew::Function_derivatnew.  "
										"Atomic variable missing function()"));
									independent_iterators_iterator=
										atomic_local_independent_iterators.begin();
									for (i=local_order;i>0;--i)
									{
										new_matrix_atomic_independent_variables.push_back(
											*(*independent_iterators_iterator));
										++independent_iterators_iterator;
									}
									column=0;
									atomic_independent_variable_iterator=independent_variable->
										begin_atomic();
									while (valid&&(atomic_independent_variable_iterator!=
										independent_variable->end_atomic()))
									{
										Function_variable_handle atomic_independent_variable=
											*atomic_independent_variable_iterator;

										if (1==atomic_independent_variable->number_differentiable())
										{
#if defined (NEW_CODE)
											Function_derivatnew_handle function_derivative;
											Function_variable_handle variable_derivative;
											Function_matrix_scalar_handle matrix_derivative;

											new_matrix_atomic_independent_variables.push_back(
												atomic_independent_variable);
											if ((function_derivative=boost::dynamic_pointer_cast<
												Function_derivatnew,Function>(
												atomic_dependent_variable->
												derivative(new_matrix_atomic_independent_variables)))&&
												(variable_derivative=function_derivative->output())&&
												(variable_derivative->evaluate)()&&
												(matrix_derivative=boost::dynamic_pointer_cast<
												Function_matrix<Scalar>,Function>((function_derivative->
												matrix())->get_value())))
											{
												new_matrix(row,column)=(*matrix_derivative)(1,1);
											}
											else
											{
												valid=false;
											}
											new_matrix_atomic_independent_variables.pop_back();
#else // defined (NEW_CODE)
											new_matrix_atomic_independent_variables.push_back(
												atomic_independent_variable);
											if (!((atomic_dependent_variable->function())->
												evaluate_derivative(new_matrix(row,column),
												atomic_dependent_variable,
												new_matrix_atomic_independent_variables)))
											{
												valid=false;
											}
											new_matrix_atomic_independent_variables.pop_back();
#endif // defined (NEW_CODE)
											column++;
										}
										atomic_independent_variable_iterator++;
									}
									// move to next row
									i=local_order;
									incremented_independent=false;
									independent_iterators_begin_iterator=
										atomic_local_independent_iterators_begin.end();
									independent_iterators_iterator=
										atomic_local_independent_iterators.end();
									independent_iterators_end_iterator=
										atomic_local_independent_iterators_end.end();
									while (!incremented_independent&&(i>0))
									{
										--independent_iterators_begin_iterator;
										--independent_iterators_iterator;
										--independent_iterators_end_iterator;
										--i;
										atomic_independent_variable_iterator=
											*independent_iterators_iterator;
										do
										{
											++atomic_independent_variable_iterator;
										}
										while ((atomic_independent_variable_iterator!=
											*independent_iterators_end_iterator)&&
											(1!=(*atomic_independent_variable_iterator)->
											number_differentiable()));
										if (atomic_independent_variable_iterator==
											*independent_iterators_end_iterator)
										{
											atomic_independent_variable_iterator=
												*independent_iterators_begin_iterator;
											while ((atomic_independent_variable_iterator!=
												*independent_iterators_end_iterator)&&
												(1!=(*atomic_independent_variable_iterator)->
												number_differentiable()))
											{
												++atomic_independent_variable_iterator;
											}
										}
										else
										{
											incremented_independent=true;
										}
										*independent_iterators_iterator=
											atomic_independent_variable_iterator;
									}
									if (!incremented_independent)
									{
										atomic_dependent_variable_iterator++;
									}
									row++;
								}
								else
								{
									atomic_dependent_variable_iterator++;
								}
							}
							matrices.push_back(new_matrix);
							matrix_independent_variables.push_back(
								new_matrix_independent_variables);
						}
						last=matrices.end();
						last--;
						matrix_independent_variables_iterator=
							matrix_independent_variables.begin();
						matrix_iterator=matrices.begin();
						while (valid&&(matrix_iterator!=last))
						{
							Function_size_type row,column;
							Function_variable_iterator atomic_dependent_variable_iterator(0),
								atomic_independent_variable_iterator(0);
							Matrix& matrix= *matrix_iterator;
							Matrix new_matrix((matrix.size1)(),
								number_of_independent_values*(matrix.size2)());
							std::list<Function_variable_handle>
								new_matrix_independent_variables;
							std::list<Function_variable_iterator>
								atomic_local_independent_iterators;
							std::list<Function_variable_iterator>::iterator
								independent_iterators_begin_iterator,
								independent_iterators_end_iterator,
								independent_iterators_iterator;

							new_matrix_independent_variables=
								*matrix_independent_variables_iterator;
							new_matrix_independent_variables.push_back(independent_variable);
							//???DB.  Generalize code above to have a set up matrix function?
							row=0;
							atomic_dependent_variable_iterator=
								local_dependent_variable->begin_atomic();
							independent_iterators_begin_iterator=
								atomic_local_independent_iterators_begin.begin();
							independent_iterators_end_iterator=
								atomic_local_independent_iterators_end.begin();
							for (i=local_order;i>0;--i)
							{
								atomic_independent_variable_iterator=
									*independent_iterators_begin_iterator;
								while ((atomic_independent_variable_iterator!=
									*independent_iterators_end_iterator)&&
									(1!=(*atomic_independent_variable_iterator)->
									number_differentiable()))
								{
									++atomic_independent_variable_iterator;
								}
								atomic_local_independent_iterators.push_back(
									*independent_iterators_begin_iterator);
								++independent_iterators_begin_iterator;
								++independent_iterators_end_iterator;
							}
							while (valid&&(atomic_dependent_variable_iterator!=
								local_dependent_variable->end_atomic()))
							{
								Function_variable_handle atomic_dependent_variable=
									*atomic_dependent_variable_iterator;

								if (1==atomic_dependent_variable->number_differentiable())
								{
									bool no_derivative;
									std::vector<Function_variable_iterator>
										atomic_independent_variable_iterators(
										new_matrix_independent_variables.size());
									std::list<Function_variable_handle>::iterator
										new_matrix_independent_variables_iterator;

									Assert(atomic_dependent_variable&&
										(atomic_dependent_variable->function()),std::logic_error(
										"Function_derivatnew::Function_derivatnew.  "
										"Atomic variable missing function()"));
									new_matrix_independent_variables_iterator=
										new_matrix_independent_variables.begin();
									i=0;
									no_derivative=false;
									while ((new_matrix_independent_variables_iterator!=
										new_matrix_independent_variables.end())&&!no_derivative)
									{
										atomic_independent_variable_iterators[i]=
											(*new_matrix_independent_variables_iterator)->
											begin_atomic();
										while ((atomic_independent_variable_iterators[i]!=
											(*new_matrix_independent_variables_iterator)->
											end_atomic())&&
											(1!=(*(atomic_independent_variable_iterators[i]))->
											number_differentiable()))
										{
											atomic_independent_variable_iterators[i]++;
										}
										if (atomic_independent_variable_iterators[i]==
											(*new_matrix_independent_variables_iterator)->
											end_atomic())
										{
											no_derivative=true;
										}
										else
										{
											new_matrix_independent_variables_iterator++;
										}
										i++;
									}
									if (new_matrix_independent_variables_iterator==
										new_matrix_independent_variables.end())
									{
										column=0;
										do
										{
											std::list<Function_variable_handle>
												new_matrix_atomic_independent_variables(0);

											i=new_matrix_independent_variables.size();
											while (i>0)
											{
												i--;
												new_matrix_atomic_independent_variables.push_front(
													*(atomic_independent_variable_iterators[i]));
											}
											independent_iterators_iterator=
												atomic_local_independent_iterators.end();
											for (i=local_order;i>0;--i)
											{
												--independent_iterators_iterator;
												new_matrix_atomic_independent_variables.push_front(
													*(*independent_iterators_iterator));
											}
											if ((atomic_dependent_variable->function())->
												evaluate_derivative(new_matrix(row,column),
												atomic_dependent_variable,
												new_matrix_atomic_independent_variables))
											{
												// move to next column
												i=new_matrix_independent_variables.size();
												new_matrix_independent_variables_iterator=
													new_matrix_independent_variables.end();
												if (i>0)
												{
													i--;
													new_matrix_independent_variables_iterator--;
													atomic_independent_variable_iterators[i]++;
													while ((atomic_independent_variable_iterators[i]!=
														(*new_matrix_independent_variables_iterator)->
														end_atomic())&&
														(1!=(*(atomic_independent_variable_iterators[i]))->
														number_differentiable()))
													{
														atomic_independent_variable_iterators[i]++;
													}
													while ((i>0)&&
														((*new_matrix_independent_variables_iterator)->
														end_atomic()==
														atomic_independent_variable_iterators[i]))
													{
														atomic_independent_variable_iterators[i]=
															(*new_matrix_independent_variables_iterator)->
															begin_atomic();
														while ((atomic_independent_variable_iterators[i]!=
															(*new_matrix_independent_variables_iterator)->
															end_atomic())&&(1!=
															(*(atomic_independent_variable_iterators[i]))->
															number_differentiable()))
														{
															atomic_independent_variable_iterators[i]++;
														}
														i--;
														atomic_independent_variable_iterators[i]++;
														new_matrix_independent_variables_iterator--;
														while ((atomic_independent_variable_iterators[i]!=
															(*new_matrix_independent_variables_iterator)->
															end_atomic())&&(1!=
															(*(atomic_independent_variable_iterators[i]))->
															number_differentiable()))
														{
															atomic_independent_variable_iterators[i]++;
														}
													}
												}
											}
											else
											{
												valid=false;
											}
											column++;
										} while (valid&&
											((new_matrix_independent_variables.front())->
											end_atomic()!=atomic_independent_variable_iterators[0]));
									}
									// move to next row
									i=local_order;
									incremented_independent=false;
									independent_iterators_begin_iterator=
										atomic_local_independent_iterators_begin.end();
									independent_iterators_iterator=
										atomic_local_independent_iterators.end();
									independent_iterators_end_iterator=
										atomic_local_independent_iterators_end.end();
									while (!incremented_independent&&(i>0))
									{
										--independent_iterators_begin_iterator;
										--independent_iterators_iterator;
										--independent_iterators_end_iterator;
										--i;
										atomic_independent_variable_iterator=
											*independent_iterators_iterator;
										do
										{
											++atomic_independent_variable_iterator;
										}
										while ((atomic_independent_variable_iterator!=
											*independent_iterators_end_iterator)&&
											(1!=(*atomic_independent_variable_iterator)->
											number_differentiable()));
										if (atomic_independent_variable_iterator==
											*independent_iterators_end_iterator)
										{
											atomic_independent_variable_iterator=
												*independent_iterators_begin_iterator;
											while ((atomic_independent_variable_iterator!=
												*independent_iterators_end_iterator)&&
												(1!=(*atomic_independent_variable_iterator)->
												number_differentiable()))
											{
												++atomic_independent_variable_iterator;
											}
										}
										else
										{
											incremented_independent=true;
										}
										*independent_iterators_iterator=
											atomic_independent_variable_iterator;
									}
									if (!incremented_independent)
									{
										atomic_dependent_variable_iterator++;
									}
									row++;
								}
								else
								{
									atomic_dependent_variable_iterator++;
								}
							}
							matrices.push_back(new_matrix);
							matrix_independent_variables.push_back(
								new_matrix_independent_variables);
							matrix_independent_variables_iterator++;
							matrix_iterator++;
						}
						independent_variable_iterator++;
					}
					if (valid)
					{
						derivative_matrix=Derivative_matrix(matrices);
						set_evaluated();
					}
				}
#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
				result=valid;
#endif // defined (EVALUATE_RETURNS_VALUE)
			}
#if defined (EVALUATE_RETURNS_VALUE)
			if (evaluated())
			{
				result=get_value(atomic_variable);
			}
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
		}
	}
#if defined (EVALUATE_RETURNS_VALUE)
	else
	{
		result=get_value(atomic_variable);
	}
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)

	return (result);
}

bool Function_derivatnew::evaluate_derivative(Scalar&,
	Function_variable_handle,
	std::list<Function_variable_handle>&)
//******************************************************************************
// LAST MODIFIED : 21 December 2004
//
// DESCRIPTION :
// ???DB.  evaluate_derivative is being replaced by Function_derivatnew, so
//   haven't written this
//==============================================================================
{
	return (false);
}

bool Function_derivatnew::set_value(
	Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 17 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_handle function;
	Function_variable_derivatnew_handle
		atomic_derivatnew_variable;
	Function_variable_value_scalar_handle value_scalar;

	result=false;
	if (this)
	{
		if ((atomic_derivatnew_variable=boost::dynamic_pointer_cast<
			Function_variable_derivatnew,Function_variable>(
			atomic_variable))&&equivalent(Function_handle(this),
			atomic_derivatnew_variable->function())&&
			(atomic_derivatnew_variable->matrix_reverse_index<
			derivative_matrix.size())&&
			atomic_value&&(atomic_value->value())&&
			(std::string("Scalar")==(atomic_value->value())->type())&&
			(value_scalar=boost::dynamic_pointer_cast<Function_variable_value_scalar,
			Function_variable_value>(atomic_value->value())))
		{
			Function_size_type i;
			std::list<Matrix>::reverse_iterator matrix_reverse_iterator;

			matrix_reverse_iterator=derivative_matrix.rbegin();
			for (i=atomic_derivatnew_variable->matrix_reverse_index;i>0;i--)
			{
				matrix_reverse_iterator++;
			}
			result=value_scalar->set((*matrix_reverse_iterator)(
				(atomic_derivatnew_variable->row_private),
				(atomic_derivatnew_variable->column_private)),atomic_value);
		}
		else if (dependent_variable&&(function=(dependent_variable->function)()))
		{
			result=(function->set_value)(atomic_variable,atomic_value);
		}
		if (result)
		{
			set_not_evaluated();
		}
	}

	return (result);
}

Function_handle Function_derivatnew::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 9 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_variable_derivatnew_handle
		atomic_variable_derivatnew;
	Matrix result_matrix(1,1);

	if (this)
	{
		if (equivalent(Function_handle(this),(atomic_variable->function)())&&
			(atomic_variable_derivatnew=boost::dynamic_pointer_cast<
			Function_variable_derivatnew,Function_variable>(
			atomic_variable))&&atomic_variable_derivatnew->get_entry(
			result_matrix(0,0)))
		{
			result=Function_handle(new Function_matrix<Scalar>(result_matrix));
		}
	}

	return (result);
}

Function_derivatnew::Function_derivatnew(
	const Function_derivatnew& derivatnew) :
	Function(derivatnew),
	derivative_matrix(derivatnew.derivative_matrix),
	dependent_variable(derivatnew.dependent_variable),
	independent_variables(derivatnew.independent_variables)
//******************************************************************************
// LAST MODIFIED : 27 January 2005
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (dependent_variable)
	{
		dependent_variable->add_dependent_function(this);
	}
}

Function_derivatnew& Function_derivatnew::operator=(
	const Function_derivatnew& derivatnew)
//******************************************************************************
// LAST MODIFIED : 27 January 2005
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	derivative_matrix=derivatnew.derivative_matrix;
	if (derivatnew.dependent_variable)
	{
		derivatnew.dependent_variable->add_dependent_function(this);
	}
	if (dependent_variable)
	{
		dependent_variable->remove_dependent_function(this);
	}
	dependent_variable=derivatnew.dependent_variable;
	independent_variables=derivatnew.independent_variables;

	return (*this);
}

bool Function_derivatnew::operator==(const Function& function) const
//******************************************************************************
// LAST MODIFIED : 9 December 2004
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
			const Function_derivatnew& function_derivatnew=
				dynamic_cast<const Function_derivatnew&>(function);

			if (equivalent(dependent_variable,
				function_derivatnew.dependent_variable))
			{
				std::list<Function_variable_handle>::const_iterator
					variable_iterator_1,variable_iterator_1_end,variable_iterator_2,
					variable_iterator_2_end;

				variable_iterator_1=independent_variables.begin();
				variable_iterator_1_end=independent_variables.end();
				variable_iterator_2=
					function_derivatnew.independent_variables.begin();
				variable_iterator_2_end=
					function_derivatnew.independent_variables.end();
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
