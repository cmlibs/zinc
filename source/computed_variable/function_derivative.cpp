//******************************************************************************
// FILE : function_derivative.cpp
//
// LAST MODIFIED : 11 June 2004
//
// DESCRIPTION :
//==============================================================================

#include <sstream>

#include "computed_variable/function_derivative.hpp"
#include "computed_variable/function_variable.hpp"

// module classes
// ==============

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
// LAST MODIFIED : 10 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_derivative;
	friend class Function_variable_iterator_representation_atomic_derivative;
	friend bool is_atomic(Function_variable_derivative_handle variable);
	public:
		// constructor
		Function_variable_derivative(
			const Function_derivative_handle& function_derivative):
			Function_variable(),function_derivative(function_derivative),
			atomic_dependent_variable(0),atomic_independent_variables(0){};
		// constructor.  A zero <atomic_dependent_variable> indicates the whole
		//   dependent variable.  An empty <atomic_independent_variables> list
		//   indicates all derivatives.  A zero <atomic_independent_variables> list
		//   entry indicates all derivatives for the entry
		Function_variable_derivative(
			const Function_derivative_handle& function_derivative,
			Function_variable_handle& atomic_dependent_variable,
			std::list<Function_variable_handle>& atomic_independent_variables):
			Function_variable(),function_derivative(function_derivative),
			atomic_dependent_variable(0),atomic_independent_variables(0)
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
						(*atomic_variable_iterator!=atomic_dependent_variable))
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
								(*atomic_variable_iterator!= *variable_iterator_1))
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
			return (Function_variable_derivative_handle(
				new Function_variable_derivative(*this)));
		};
		Function_handle function()
		{
			return (function_derivative);
		};
		Function_handle evaluate()
		{
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
						std::list<Function_variable_handle> independent_variables_local=
							function_derivative->independent_variables_private;

						if (order==atomic_independent_variables.size())
						{
							std::list<Function_variable_handle>::iterator iterator_1,
								iterator_1_end,iterator_2;

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
						result=dependent_variable->evaluate_derivative(
							independent_variables_local);
					}
				}
			}

			return (result);
		};
		string_handle get_string_representation()
		{
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
			Function_variable_derivative_handle variable_derivative;

			result=false;
			if (variable_derivative=boost::dynamic_pointer_cast<
				Function_variable_derivative,Function_variable>(variable))
			{
				if ((variable_derivative->function_derivative==function_derivative)&&
					(((0==variable_derivative->atomic_dependent_variable)&&
					(0==atomic_dependent_variable))||(atomic_dependent_variable&&
					(variable_derivative->atomic_dependent_variable)&&
					(*(variable_derivative->atomic_dependent_variable)==
					*atomic_dependent_variable))))
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
							(((0== *variable_iterator_1)&&(0== *variable_iterator_2))||
							((*variable_iterator_1)&&(*variable_iterator_2)&&
							(**variable_iterator_1== **variable_iterator_2))))
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
		// copy constructor
		Function_variable_derivative(
			const Function_variable_derivative& variable_derivative):
			Function_variable(),
			function_derivative(variable_derivative.function_derivative),
			atomic_dependent_variable(variable_derivative.atomic_dependent_variable),
			atomic_independent_variables(
			variable_derivative.atomic_independent_variables){};
		// assignment
		Function_variable_derivative& operator=(
			const Function_variable_derivative&);
	private:
		Function_derivative_handle function_derivative;
		// if zero then all
		Function_variable_handle atomic_dependent_variable;
		// if empty list then all.  If zero list entry then all for that independent
		//   variable
		std::list<Function_variable_handle> atomic_independent_variables;
};

bool is_atomic(Function_variable_derivative_handle variable)
{
	bool result;

	result=false;
	if (variable&&(variable->atomic_dependent_variable))
	{
		std::list<Function_variable_handle>::iterator variable_iterator,
			variable_iterator_end;

		variable_iterator=(variable->atomic_independent_variables).begin();
		variable_iterator_end=(variable->atomic_independent_variables).end();
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
}


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
// LAST MODIFIED : 11 June 2004
//
// DESCRIPTION :
// Constructor.  If <begin> then the constructed iterator points to the first
// atomic variable, otherwise it points to one past the last atomic variable.
//==============================================================================
{
	if (begin&&variable&&(variable->function_derivative))
	{
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
					variable->function_derivative->dependent_variable_private)
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
						variable->function_derivative->independent_variables_private;

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
		if (variable&&(variable->function_derivative))
		{
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
						variable->function_derivative->dependent_variable_private)
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
							variable->function_derivative->independent_variables_private;

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
// LAST MODIFIED : 10 June 2004
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
		if (
			((0==atomic_variable)&&(0==representation_derivative->atomic_variable))||
			(atomic_variable&&(representation_derivative->atomic_variable)&&
			(*atomic_variable== *(representation_derivative->atomic_variable))))
		{
			result=true;
		}
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


// global classes
// ==============

// class Function_derivative
// -------------------------

Function_derivative::Function_derivative(
	const Function_variable_handle& dependent_variable,
	std::list<Function_variable_handle>& independent_variables):Function(),
	dependent_variable_private(dependent_variable),
	independent_variables_private(independent_variables){}
//******************************************************************************
// LAST MODIFIED : 9 June 2004
//
// DESCRIPTION :
//==============================================================================

Function_derivative::~Function_derivative(){}
//******************************************************************************
// LAST MODIFIED : 8 June 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================

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

Function_handle Function_derivative::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 9 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_variable_derivative_handle atomic_derivative_variable;

	if ((atomic_derivative_variable=boost::dynamic_pointer_cast<
		Function_variable_derivative,Function_variable>(atomic_variable))&&
		(this==atomic_derivative_variable->function_derivative)&&
		is_atomic(atomic_derivative_variable)&&
		(atomic_derivative_variable->atomic_dependent_variable))
	{
		result=(atomic_derivative_variable->atomic_dependent_variable->
			evaluate_derivative)(atomic_derivative_variable->
			atomic_independent_variables);
	}

	return (result);
}

bool Function_derivative::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 9 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_handle atomic_variable_local;
	Function_variable_derivative_handle atomic_variable_derivative;

	result=false;
	if ((atomic_variable_derivative=boost::dynamic_pointer_cast<
		Function_variable_derivative,Function_variable>(atomic_variable))&&
		(this==atomic_variable_derivative->function_derivative)&&
		is_atomic(atomic_variable_derivative)&&
		(1==atomic_variable_derivative->number_differentiable())&&
		(atomic_variable_local=
		atomic_variable_derivative->atomic_dependent_variable))
	{
		std::list<Function_variable_handle> merged_independent_variables=
			atomic_variable_derivative->atomic_independent_variables;

		merged_independent_variables.insert(merged_independent_variables.end(),
			atomic_independent_variables.begin(),atomic_independent_variables.end());
		result=(atomic_variable_local->function)()->evaluate_derivative(
			derivative,atomic_variable_local,merged_independent_variables);
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

Function_derivative::Function_derivative(
	const Function_derivative& function_derivative):Function(),
	dependent_variable_private(function_derivative.dependent_variable_private),
	independent_variables_private(
	function_derivative.independent_variables_private)
//******************************************************************************
// LAST MODIFIED : 9 June 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Function_derivative& Function_derivative::operator=(
	const Function_derivative& function_derivative)
//******************************************************************************
// LAST MODIFIED : 9 June 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	dependent_variable_private=function_derivative.dependent_variable_private;
	independent_variables_private=
		function_derivative.independent_variables_private;

	return (*this);
}
