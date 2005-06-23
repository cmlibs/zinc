//******************************************************************************
// FILE : function_variable_union.cpp
//
// LAST MODIFIED : 17 May 2005
//
// DESCRIPTION :
//==============================================================================

#include <algorithm>
#include <iterator>
#include <typeinfo>
#include <iostream>
#include <sstream>

#include "computed_variable/function_variable_union.hpp"
#if defined (USE_FUNCTION_VARIABLE_UNION_EVALUATE)
#include "computed_variable/function_composite.hpp"
#include "computed_variable/function_matrix.hpp"
#endif // defined (USE_FUNCTION_VARIABLE_UNION_EVALUATE)

#if defined (USE_FUNCTION_VARIABLE_UNION_EVALUATE)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#include "computed_variable/function_derivative.hpp"
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE_UNION_EVALUATE)

// module classes
// ==============

bool repeat_atomic_variable(
	const std::list<Function_variable_handle>& variables_list,
	const std::list<Function_variable_handle>::iterator& variables_list_iterator,
	const Function_variable_iterator& atomic_variable_iterator)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
// Determines if (*atomic_variable_iterator) appears before it in the union.
//==============================================================================
{
	bool repeat;
	Function_variable_iterator local_atomic_variable_iterator;
	std::list<Function_variable_handle>::const_iterator
		local_variables_list_iterator;

	repeat=false;
	local_variables_list_iterator=variables_list.begin();
	while ((local_variables_list_iterator!=variables_list_iterator)&&!repeat)
	{
		local_atomic_variable_iterator=
			(*local_variables_list_iterator)->begin_atomic();
		while ((local_atomic_variable_iterator!=
			(*local_variables_list_iterator)->end_atomic())&&
			!(repeat=equivalent(*local_atomic_variable_iterator,
			*atomic_variable_iterator)))
		{
			++local_atomic_variable_iterator;
		}
		++local_variables_list_iterator;
	}

	return (repeat);
}

#if defined (USE_FUNCTION_VARIABLE_UNION_EVALUATE)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_derivatnew_union
// -------------------------------

class Function_derivatnew_union : public Function_derivatnew
//******************************************************************************
// LAST MODIFIED : 16 May 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// for construction exception
		class Construction_exception {};
		// constructor
		Function_derivatnew_union(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables);
		// destructor
		~Function_derivatnew_union();
	// inherited
	private:
#if defined (EVALUATE_RETURNS_VALUE)
		virtual Function_handle evaluate(Function_variable_handle atomic_variable);
#else // defined (EVALUATE_RETURNS_VALUE)
		virtual bool evaluate(Function_variable_handle atomic_variable);
#endif // defined (EVALUATE_RETURNS_VALUE)
	private:
		std::list<Function_derivatnew_handle> derivatives_list;
};

Function_derivatnew_union::Function_derivatnew_union(
	const Function_variable_handle& dependent_variable,
	const std::list<Function_variable_handle>& independent_variables):
	Function_derivatnew(dependent_variable,independent_variables)
//******************************************************************************
// LAST MODIFIED : 17 May 2005
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	Function_variable_union_handle variable_union;
	Function_variable_handle variable_union_wrapped;

	if (variable_union=boost::dynamic_pointer_cast<
		Function_variable_union,Function_variable>(dependent_variable))
	{
		bool valid;
		Function_derivatnew_handle derivative;
		Function_size_type i;
		std::list<Function_variable_handle>::iterator variables_iterator;

		valid=true;
		i=(variable_union->variables_list).size();
		variables_iterator=(variable_union->variables_list).begin();
		while (valid&&(i>0))
		{
			if (derivative=boost::dynamic_pointer_cast<Function_derivatnew,
				Function>((*variables_iterator)->derivative(independent_variables)))
			{
				derivatives_list.push_back(derivative);
			}
			else
			{
				valid=false;
			}
			++variables_iterator;
			--i;
		}
		if (valid)
		{
			std::list<Function_derivatnew_handle>::iterator derivative_iterator;

			i=derivatives_list.size();
			derivative_iterator=derivatives_list.begin();
			while (i>0)
			{
				(*derivative_iterator)->add_dependent_function(this);
				--i;
				++derivative_iterator;
			}
		}
		else
		{
			throw Function_derivatnew_union::Construction_exception();
		}
	}
	else
	{
		throw Function_derivatnew_union::Construction_exception();
	}
}

Function_derivatnew_union::~Function_derivatnew_union()
//******************************************************************************
// LAST MODIFIED : 16 May 2005
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
#if defined (CIRCULAR_SMART_POINTERS)
	// do nothing
#else // defined (CIRCULAR_SMART_POINTERS)
	Function_size_type i;
	std::list<Function_derivatnew_handle>::iterator derivative_iterator;

	i=derivatives_list.size();
	derivative_iterator=derivatives_list.begin();
	while (i>0)
	{
		(*derivative_iterator)->remove_dependent_function(this);
		--i;
		++derivative_iterator;
	}
#endif // defined (CIRCULAR_SMART_POINTERS)
}

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_derivatnew_union::evaluate(
	Function_variable_handle
#if defined (EVALUATE_RETURNS_VALUE)
	atomic_variable
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
	)
//******************************************************************************
// LAST MODIFIED : 17 May 2005
//
// DESCRIPTION :
//==============================================================================
{
#if defined (EVALUATE_RETURNS_VALUE)
	Function_handle result(0);
#else // defined (EVALUATE_RETURNS_VALUE)
	bool result(true);
#endif // defined (EVALUATE_RETURNS_VALUE)

	if (!evaluated())
	{
		Function_variable_union_handle dependent_variable_union;

#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
		result=false;
#endif // defined (EVALUATE_RETURNS_VALUE)
		if (dependent_variable_union=boost::dynamic_pointer_cast<
			Function_variable_union,Function_variable>(dependent_variable))
		{
			bool first,valid;
			Function_derivatnew_handle part_derivative;
			Function_size_type i,j,k,l,part_matrix_row,result_number_of_matrices,
				result_number_of_rows;
			Function_variable_handle part_derivative_output;
			Function_variable_iterator atomic_variable_iterator,
				atomic_variable_iterator_end;
			std::list<Function_derivatnew_handle>::iterator part_derivative_iterator;
			std::list<Function_size_type> rows_list;
			std::list<Function_size_type>::iterator rows_list_iterator;
			std::list<Function_variable_handle>&
				variables_list=dependent_variable_union->variables_list;
			std::list<Function_variable_handle>::iterator variable_iterator;
			std::list< std::list<Matrix>::iterator > part_matrix_iterators;

			i=derivatives_list.size();
			valid=(i==variables_list.size());
			first=true;
			part_derivative_iterator=derivatives_list.begin();
			variable_iterator=variables_list.begin();
			result_number_of_rows=0;
			while (valid&&(i>0))
			{
				Function_variable_handle variable= *variable_iterator;

				if ((part_derivative= *part_derivative_iterator)&&
					(part_derivative_output=part_derivative->output())&&
					(part_derivative_output->evaluate()))
				{
					Derivative_matrix&
						part_derivative_matrix=part_derivative->derivative_matrix;

					if (first)
					{
						first=false;
						result_number_of_matrices=part_derivative_matrix.size();
						result_number_of_rows=part_derivative_matrix.front().size1();
						rows_list.push_back(result_number_of_rows);
						for (part_matrix_row=0;part_matrix_row<result_number_of_rows;
							part_matrix_row++)
						{
							rows_list.push_back(part_matrix_row);
						}
					}
					else
					{
						if (valid=
							(result_number_of_matrices==part_derivative_matrix.size()))
						{
							rows_list.push_back(0);
							rows_list_iterator=rows_list.end();
							--rows_list_iterator;
							part_matrix_row=0;
							atomic_variable_iterator_end=variable->end_atomic();
							for (atomic_variable_iterator=variable->begin_atomic();
								atomic_variable_iterator!=atomic_variable_iterator_end;
								++atomic_variable_iterator)
							{
								if (1==(*atomic_variable_iterator)->number_differentiable())
								{
									if (!repeat_atomic_variable(variables_list,variable_iterator,
										atomic_variable_iterator))
									{
										rows_list.push_back(part_matrix_row);
										++(*rows_list_iterator);
										++result_number_of_rows;
									}
									++part_matrix_row;
								}
							}
						}
					}
					part_matrix_iterators.push_back(part_derivative_matrix.begin());
				}
				else
				{
					valid=false;
				}
				--i;
				++part_derivative_iterator;
				++variable_iterator;
			}
			//???DB.  Where I'm up to
			if (valid)
			{
				std::list<Matrix> matrices;

				derivative_matrix.clear();
				for (k=result_number_of_matrices;k>0;--k)
				{
					std::list< std::list<Matrix>::iterator >::iterator
						part_matrix_iterators_iterator=part_matrix_iterators.begin();
					Function_size_type result_number_of_columns=
						(*part_matrix_iterators_iterator)->size2(),result_row_number;
					Matrix matrix(result_number_of_rows,result_number_of_columns);

					result_row_number=0;
					rows_list_iterator=rows_list.begin();
					while (result_row_number<result_number_of_rows)
					{
						Matrix& part_matrix= **part_matrix_iterators_iterator;

						l= *rows_list_iterator;
						++rows_list_iterator;
						while (l>0)
						{
							i= *rows_list_iterator;
							++rows_list_iterator;
							for (j=0;j<result_number_of_columns;++j)
							{
								matrix(result_row_number,j)=part_matrix(i,j);
							}
							l--;
							++result_row_number;
						}
						(*part_matrix_iterators_iterator)++;
						part_matrix_iterators_iterator++;
					}
					matrices.push_back(matrix);
				}
				try
				{
					derivative_matrix=Derivative_matrix(matrices);
					set_evaluated();
#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
					result=true;
#endif // defined (EVALUATE_RETURNS_VALUE)
				}
				catch (Derivative_matrix::Construction_exception)
				{
					// do nothing
				}
			}
		}
	}
#if defined (EVALUATE_RETURNS_VALUE)
	//???DB.  To be done
	//???DB.  Has to find the appropriate derivative and row
	//???DB.  How does get_value work?
#endif // defined (EVALUATE_RETURNS_VALUE)

	return (result);
}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE_UNION_EVALUATE)


// class Function_variable_iterator_representation_atomic_union
// ------------------------------------------------------------

class Function_variable_iterator_representation_atomic_union: public
	Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_union(
			const bool begin,Function_variable_union_handle source):
			atomic_variable_iterator(0),variables_list_iterator(0),
			source(source)
		{
			if (source)
			{
				if (begin)
				{
					variables_list_iterator=(source->variables_list).begin();
					while (
						(variables_list_iterator!=(source->variables_list).end())&&
						((atomic_variable_iterator=
						((*variables_list_iterator)->begin_atomic)())==
						((*variables_list_iterator)->end_atomic)()))
					{
						++variables_list_iterator;
					}
					if (variables_list_iterator==(source->variables_list).end())
					{
						atomic_variable_iterator=0;
					}
				}
				else
				{
					variables_list_iterator=(source->variables_list).end();
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
				result=new Function_variable_iterator_representation_atomic_union(
					*this);
			}

			return (result);
		};
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_union(){};
	private:
		// increment
		void increment()
		{
			if (variables_list_iterator!=(source->variables_list).end())
			{
				do
				{
					++atomic_variable_iterator;
				} while ((atomic_variable_iterator!=
					((*variables_list_iterator)->end_atomic)())&&
					repeat_atomic_variable(source->variables_list,
					variables_list_iterator,atomic_variable_iterator));
				if (atomic_variable_iterator==
					((*variables_list_iterator)->end_atomic)())
				{
					do
					{
						++variables_list_iterator;
						if (variables_list_iterator!=(source->variables_list).end())
						{
							atomic_variable_iterator=
								((*variables_list_iterator)->begin_atomic)();
							while ((atomic_variable_iterator!=
								((*variables_list_iterator)->end_atomic)())&&
								repeat_atomic_variable(source->variables_list,
								variables_list_iterator,atomic_variable_iterator))
							{
								++atomic_variable_iterator;
							}
						}
					} while ((variables_list_iterator!=(source->variables_list).end())&&
						(atomic_variable_iterator==
						((*variables_list_iterator)->end_atomic)()));
					if (variables_list_iterator==(source->variables_list).end())
					{
						atomic_variable_iterator=0;
					}
				}
			}
		};
		// decrement
		void decrement()
		{
			bool done;

			done=false;
			if ((variables_list_iterator==(source->variables_list).end())||
				(atomic_variable_iterator==
				((*variables_list_iterator)->begin_atomic)()))
			{
				if (variables_list_iterator==(source->variables_list).begin())
				{
					done=true;
					variables_list_iterator=(source->variables_list).end();
					atomic_variable_iterator=0;
				}
				else
				{
					do
					{
						--variables_list_iterator;
						atomic_variable_iterator=((*variables_list_iterator)->end_atomic)();
					} while ((variables_list_iterator!=(source->variables_list).begin())&&
						(atomic_variable_iterator==
						((*variables_list_iterator)->begin_atomic)()));
					if ((variables_list_iterator==(source->variables_list).begin())&&
						(atomic_variable_iterator==
						((*variables_list_iterator)->begin_atomic)()))
					{
						done=true;
						variables_list_iterator=(source->variables_list).end();
						atomic_variable_iterator=0;
					}
				}
			}
			while (!done)
			{
				do
				{
					--atomic_variable_iterator;
					done= !repeat_atomic_variable(source->variables_list,
						variables_list_iterator,atomic_variable_iterator);
				} while ((atomic_variable_iterator!=
					((*variables_list_iterator)->begin_atomic)())&&!done);
				if (!done)
				{
					while ((variables_list_iterator!=(source->variables_list).begin())&&
						(atomic_variable_iterator==
						((*variables_list_iterator)->begin_atomic)()))
					{
						--variables_list_iterator;
						atomic_variable_iterator=((*variables_list_iterator)->end_atomic)();
					}
					if ((variables_list_iterator==(source->variables_list).begin())&&
						(atomic_variable_iterator==
						((*variables_list_iterator)->begin_atomic)()))
					{
						done=true;
						variables_list_iterator=(source->variables_list).end();
						atomic_variable_iterator=0;
					}
				}
			}
		};
		// equality
		bool equality(const Function_variable_iterator_representation
			* representation)
		{
			const Function_variable_iterator_representation_atomic_union
				*representation_union=dynamic_cast<
				const Function_variable_iterator_representation_atomic_union *>(
				representation);

			return (representation_union&&
				(variables_list_iterator==
				representation_union->variables_list_iterator)&&
				(atomic_variable_iterator==
				representation_union->atomic_variable_iterator));
		};
		// dereference
		Function_variable_handle dereference() const
		{
			return (*atomic_variable_iterator);
		};
	private:
		Function_variable_iterator_representation_atomic_union(const
			Function_variable_iterator_representation_atomic_union&
			representation):Function_variable_iterator_representation(),
			atomic_variable_iterator(representation.atomic_variable_iterator),
			variables_list_iterator(representation.variables_list_iterator),
			source(representation.source){};
	private:
		Function_variable_iterator atomic_variable_iterator;
		std::list<Function_variable_handle>::iterator variables_list_iterator;
		Function_variable_union_handle source;
};

// global classes
// ==============

// class Function_variable_union
// -----------------------------

Function_variable_union::Function_variable_union(
	const Function_variable_handle& variable_1,
	const Function_variable_handle& variable_2):
	Function_variable(Function_handle(0)),variables_list(0)
//******************************************************************************
// LAST MODIFIED : 30 June 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	variables_list.push_back(variable_1);
	variables_list.push_back(variable_2);
}

Function_variable_union::Function_variable_union(
	const Function_handle& function,
	const Function_variable_handle& variable_1,
	const Function_variable_handle& variable_2):Function_variable(function),
	variables_list(0)
//******************************************************************************
// LAST MODIFIED : 12 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	variables_list.push_back(variable_1);
	variables_list.push_back(variable_2);
}

Function_variable_union::Function_variable_union(
	std::list<Function_variable_handle>& variables_list):
	Function_variable(Function_handle(0)),variables_list(variables_list)
//******************************************************************************
// LAST MODIFIED : 30 June 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Function_variable_union::Function_variable_union(
	const Function_handle& function,
	std::list<Function_variable_handle>& variables_list):
	Function_variable(function),variables_list(variables_list)
//******************************************************************************
// LAST MODIFIED : 12 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Function_variable_handle Function_variable_union::clone() const
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle local_function=function();
	Function_size_type i;
	Function_variable_union_handle result(0);
	std::list<Function_variable_handle> local_variables_list;

	if (0<(i=variables_list.size()))
	{
		std::list<Function_variable_handle>::const_iterator iterator;

		iterator=variables_list.begin();
		while (i>0)
		{
			if (*iterator)
			{
				local_variables_list.push_back((*iterator)->clone());
			}
			else
			{
				local_variables_list.push_back(Function_variable_handle(0));
			}
			++iterator;
			--i;
		}
	}
	if (result=Function_variable_union_handle(new Function_variable_union(
		local_function,local_variables_list)))
	{
		result->value_private=value_private;
	}

	return (result);
}

#if defined (USE_FUNCTION_VARIABLE_UNION_EVALUATE)
#if defined (EVALUATE_RETURNS_VALUE)
Function_handle Function_variable_union::evaluate()
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool all_of_part,first=true,result_is_matrix=true;
	boost::intrusive_ptr< Function_matrix<Scalar> > part_matrix;
	Function_handle part(0),result(0);
	Function_size_type i=variables_list.size(),part_number_of_columns,
		part_number_of_rows,result_number_of_columns,result_number_of_rows;
	Function_variable_iterator atomic_variable_iterator,
		atomic_variable_iterator_end;
	std::list< boost::intrusive_ptr< Function_matrix<Scalar> > >
		part_matrix_list(0);
	std::list<Function_handle> part_list(0);
	std::list<Function_variable_handle>::iterator variable_iterator=
		variables_list.begin();

	while (i>0)
	{
		Function_variable_handle variable= *variable_iterator;

		if (first)
		{
			if (part=variable->evaluate())
			{
				first=false;
				part_list.push_back(part);
				if (result_is_matrix)
				{
					if (part_matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
						Function>(part))
					{
						part_matrix_list.push_back(part_matrix);
						result_number_of_rows=part_matrix->number_of_rows();
						result_number_of_columns=part_matrix->number_of_columns();
					}
					else
					{
						result_is_matrix=false;
					}
				}
			}
		}
		else
		{
			Function_size_type atomic_variable_index;
			std::list<Function_size_type> atomic_variable_index_list(0);

			atomic_variable_index=0;
			first=true;
			all_of_part=true;
			atomic_variable_iterator_end=variable->end_atomic();
			for (atomic_variable_iterator=variable->begin_atomic();
				atomic_variable_iterator!=atomic_variable_iterator_end;
				++atomic_variable_iterator)
			{
				if (repeat_atomic_variable(variables_list,variable_iterator,
					atomic_variable_iterator))
				{
					all_of_part=false;
				}
				else
				{
					if (first)
					{
						if (part=variable->evaluate())
						{
							if (result_is_matrix)
							{
								if (part_matrix=boost::dynamic_pointer_cast<
									Function_matrix<Scalar>,Function>(part))
								{
									part_number_of_rows=part_matrix->number_of_rows();
									part_number_of_columns=part_matrix->number_of_columns();
								}
							}
							else
							{
								result_is_matrix=false;
							}
						}
						first=false;
					}
					atomic_variable_index_list.push_back(atomic_variable_index);
				}
				++atomic_variable_index;
			}
			if (!first)
			{
				if (all_of_part)
				{
					part_list.push_back(part);
					if (result_is_matrix)
					{
						part_matrix_list.push_back(part_matrix);
						if (result_number_of_columns==part_number_of_columns)
						{
							result_number_of_rows += part_number_of_rows;
						}
						else
						{
							if (result_number_of_rows==part_number_of_rows)
							{
								result_number_of_columns += part_number_of_columns;
							}
							else
							{
								result_number_of_rows=
									(result_number_of_rows*result_number_of_columns)+
									(part_number_of_rows*part_number_of_columns);
								result_number_of_columns=1;
							}
						}
					}
				}
				else
				{
					Function_variable_iterator atomic_part_iterator;
					std::list<Function_size_type>::iterator
						atomic_variable_index_iterator,atomic_variable_index_iterator_end;

					atomic_variable_index=0;
					atomic_variable_index_iterator_end=atomic_variable_index_list.end();
					atomic_part_iterator=part->output()->begin_atomic();
					for (
						atomic_variable_index_iterator=atomic_variable_index_list.begin();
						atomic_variable_index_iterator!=atomic_variable_index_iterator_end;
						++atomic_variable_index_iterator)
					{
						while (atomic_variable_index!= *atomic_variable_index_iterator)
						{
							++atomic_variable_index;
							++atomic_part_iterator;
						}
						part_list.push_back((*atomic_part_iterator)->get_value());
					}
					if (result_is_matrix)
					{
						Function_size_type column,row;
						Matrix new_matrix(atomic_variable_index_list.size(),1);

						atomic_variable_index=0;
						atomic_variable_index_iterator_end=atomic_variable_index_list.end();
						i=0;
						row=1;
						column=1;
						for (
							atomic_variable_index_iterator=atomic_variable_index_list.begin();
							atomic_variable_index_iterator!=
							atomic_variable_index_iterator_end;
							++atomic_variable_index_iterator)
						{
							while (atomic_variable_index!= *atomic_variable_index_iterator)
							{
								++atomic_variable_index;
								++column;
								if (column>part_number_of_columns)
								{
									column=1;
									++row;
								}
							}
							new_matrix(i,0)=(*part_matrix)(row,column);
							++i;
						}
						part_matrix_list.push_back(
							boost::intrusive_ptr< Function_matrix<Scalar> >(
							new Function_matrix<Scalar>(new_matrix)));
						result_number_of_rows=
							(result_number_of_rows*result_number_of_columns)+i;
						result_number_of_columns=1;
					}
				}
			}
			first=false;
		}
		--i;
		++variable_iterator;
	}
	if ((0==i)&&!first)
	{
		if (result_is_matrix)
		{
			Function_size_type part_column,part_row,result_column=0,result_row=0;
			Matrix result_matrix(result_number_of_rows,result_number_of_columns);
			std::list< boost::intrusive_ptr< Function_matrix<Scalar> > >::iterator
				part_matrix_iterator=part_matrix_list.begin();

			for (i=part_matrix_list.size();i>0;--i)
			{
				part_matrix= *part_matrix_iterator;
				part_number_of_rows=part_matrix->number_of_rows();
				part_number_of_columns=part_matrix->number_of_columns();
				for (part_row=1;part_row<=part_number_of_rows;++part_row)
				{
					for (part_column=1;part_column<=part_number_of_columns;++part_column)
					{
						result_matrix(result_row,result_column)=
							(*part_matrix)(part_row,part_column);
						++result_column;
						if (result_column>=result_number_of_columns)
						{
							++result_row;
							result_column=0;
						}
					}
				}
				++part_matrix_iterator;
			}
			result=Function_handle(new Function_matrix<Scalar>(result_matrix));
		}
		else
		{
			result=Function_handle(new Function_composite(part_list));
		}
	}

	return (result);
}
#else // defined (EVALUATE_RETURNS_VALUE)
bool Function_variable_union::evaluate()
//******************************************************************************
// LAST MODIFIED : 16 May 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result=true;
	Function_size_type i=variables_list.size();
	std::list<Function_variable_handle>::iterator variable_iterator=
		variables_list.begin();

	while (i>0)
	{
		if (!((*variable_iterator)->evaluate()))
		{
			result=false;
		}
		--i;
		++variable_iterator;
	}

	return (result);
}
#endif // defined (EVALUATE_RETURNS_VALUE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
Function_handle Function_variable_union::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool all_of_part,first=true,valid=true;
	boost::intrusive_ptr< Function_matrix<Scalar> > part;
	Function_handle result(0);
	Function_size_type i=variables_list.size(),result_number_of_columns=0,
		result_number_of_rows=0;
	Function_variable_iterator atomic_variable_iterator,
		atomic_variable_iterator_end;
	std::list< boost::intrusive_ptr< Function_matrix<Scalar> > > part_list(0);
	std::list<Function_variable_handle>::iterator variable_iterator=
		variables_list.begin();

	while (valid&&(i>0))
	{
		Function_variable_handle variable= *variable_iterator;

		if (first)
		{
			if (part=boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
				variable->evaluate_derivative(independent_variables)))
			{
				first=false;
				part_list.push_back(part);
				result_number_of_rows=part->number_of_rows();
				result_number_of_columns=part->number_of_columns();
			}
			else
			{
				valid=false;
			}
		}
		else
		{
			Function_size_type atomic_variable_index;
			std::list<Function_size_type> atomic_variable_index_list(0);

			atomic_variable_index=0;
			first=true;
			all_of_part=true;
			atomic_variable_iterator=variable->begin_atomic();
			atomic_variable_iterator_end=variable->end_atomic();
			while (valid&&(atomic_variable_iterator!=atomic_variable_iterator_end))
			{
				if (1==(*atomic_variable_iterator)->number_differentiable())
				{
					if (repeat_atomic_variable(variables_list,variable_iterator,
						atomic_variable_iterator))
					{
						all_of_part=false;
					}
					else
					{
						if (first)
						{
							if (!((part=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
								Function>(variable->evaluate_derivative(
								independent_variables)))&&
								(part->number_of_columns()==result_number_of_columns)))
							{
								valid=false;
							}
							first=false;
						}
						atomic_variable_index_list.push_back(atomic_variable_index);
					}
					++atomic_variable_index;
				}
				++atomic_variable_iterator;
			}
			if (valid&&!first)
			{
				if (all_of_part)
				{
					part_list.push_back(part);
					result_number_of_rows += part->number_of_rows();
				}
				else
				{
					Function_size_type column,row;
					Function_size_type j=atomic_variable_index_list.size();
					Matrix new_matrix(j,result_number_of_columns);
					std::list<Function_size_type>::iterator
						atomic_variable_index_iterator;

					result_number_of_rows += j;
					row=0;
					atomic_variable_index_iterator=atomic_variable_index_list.begin();
					while (j>0)
					{
						atomic_variable_index=(*atomic_variable_index_iterator)+1;
						for (column=1;column<=result_number_of_columns;++column)
						{
							new_matrix(row,column-1)=(*part)(atomic_variable_index,column);
						}
						++row;
						++atomic_variable_index_iterator;
						--j;
					}
					part_list.push_back(boost::intrusive_ptr< Function_matrix<Scalar> >(
						new Function_matrix<Scalar>(new_matrix)));
				}
			}
			first=false;
		}
		--i;
		++variable_iterator;
	}
	if (valid&&(0==i))
	{
		Function_size_type number_of_parts;

		number_of_parts=part_list.size();
		if (0<number_of_parts)
		{
			if (1==number_of_parts)
			{
				result=part_list.back();
			}
			else
			{
				Function_size_type j,part_number_of_rows,part_column,part_row,
					result_column,result_row=0;;
				Matrix result_matrix(result_number_of_rows,result_number_of_columns);
				std::list< boost::intrusive_ptr< Function_matrix<Scalar> > >::iterator
					part_iterator=part_list.begin();

				j=number_of_parts;;
				while (valid&&(j>0))
				{
					Function_matrix<Scalar>& part_matrix= **part_iterator;

					part_number_of_rows=part_matrix.number_of_rows();
					for (part_row=1;part_row<=part_number_of_rows;++part_row)
					{
						part_column=1;
						for (result_column=0;result_column<result_number_of_columns;
							++result_column)
						{
							result_matrix(result_row,result_column)=
								part_matrix(part_row,part_column);
							++part_column;
						}
						++result_row;
					}
					--j;
					++part_iterator;
				}
				result=Function_handle(new Function_matrix<Scalar>(result_matrix));
			}
		}
	}

	return (result);
}
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
Function_handle Function_variable_union::derivative(
	const std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 17 May 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_handle(new Function_derivatnew_union(
		Function_variable_handle(this),independent_variables)));
}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE_UNION_EVALUATE)

string_handle Function_variable_union::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
// ???DB.  Overload << instead of get_string_representation?
// ???DB.  Flatten
//==============================================================================
{
	std::list<Function_variable_handle>::iterator
		variable_iterator=variables_list.begin();
	std::ostringstream out;
	string_handle return_string,temp_string;
	Function_size_type i=variables_list.size();

	if (return_string=new std::string)
	{
		out << "union(";
		while (i>0)
		{
			if (temp_string=(*variable_iterator)->get_string_representation())
			{
				out << *temp_string;
				delete temp_string;
			}
			++variable_iterator;
			--i;
			if (i>0)
			{
				out << ",";
			}
		}
		out << ")";
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_iterator Function_variable_union::begin_atomic() const
//******************************************************************************
// LAST MODIFIED : 15 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_iterator(
		new Function_variable_iterator_representation_atomic_union(true,
		Function_variable_union_handle(
		const_cast<Function_variable_union*>(this)))));
}

Function_variable_iterator Function_variable_union::end_atomic() const
//******************************************************************************
// LAST MODIFIED : 15 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_iterator(
		new Function_variable_iterator_representation_atomic_union(false,
		Function_variable_union_handle(
		const_cast<Function_variable_union*>(this)))));
}

std::reverse_iterator<Function_variable_iterator>
	Function_variable_union::rbegin_atomic() const
//******************************************************************************
// LAST MODIFIED : 15 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (std::reverse_iterator<Function_variable_iterator>(
		new Function_variable_iterator_representation_atomic_union(false,
		Function_variable_union_handle(
		const_cast<Function_variable_union*>(this)))));
}

std::reverse_iterator<Function_variable_iterator>
	Function_variable_union::rend_atomic() const
//******************************************************************************
// LAST MODIFIED : 15 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (std::reverse_iterator<Function_variable_iterator>(
		new Function_variable_iterator_representation_atomic_union(true,
		Function_variable_union_handle(
		const_cast<Function_variable_union*>(this)))));
}

void Function_variable_union::add_dependent_function(
#if defined (CIRCULAR_SMART_POINTERS)
	const Function_handle
#else // defined (CIRCULAR_SMART_POINTERS)
	Function*
#endif // defined (CIRCULAR_SMART_POINTERS)
	dependent_function)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Function_variable_handle>::iterator iterator,iterator_end;

	iterator_end=variables_list.end();
	for (iterator=variables_list.begin();iterator!=iterator_end;++iterator)
	{
		(*iterator)->add_dependent_function(dependent_function);
	}
}

void Function_variable_union::remove_dependent_function(
#if defined (CIRCULAR_SMART_POINTERS)
	const Function_handle
#else // defined (CIRCULAR_SMART_POINTERS)
	Function*
#endif // defined (CIRCULAR_SMART_POINTERS)
	dependent_function)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Function_variable_handle>::iterator iterator,iterator_end;

	iterator_end=variables_list.end();
	for (iterator=variables_list.begin();iterator!=iterator_end;++iterator)
	{
		(*iterator)->remove_dependent_function(dependent_function);
	}
}

bool Function_variable_union::equality_atomic(const
	Function_variable_handle&) const
//******************************************************************************
// LAST MODIFIED : 15 March 2004
//
// DESCRIPTION :
//???DB.  This should be == for everything?  How to get atomic?
//==============================================================================
{
	// should not come here - handled by overloading Function_variable::operator==
	Assert(false,std::logic_error(
		"Function_variable_union::equality_atomic.  "
		"Should not come here"));

	return (false);
}

Function_variable_union::Function_variable_union(
	const Function_variable_union& variable_union):
	Function_variable(variable_union),
	variables_list(variable_union.variables_list)
//******************************************************************************
// LAST MODIFIED : 12 March 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Function_variable_union& Function_variable_union::operator=(
	const Function_variable_union& variable_union)
//******************************************************************************
// LAST MODIFIED : 12 March 2004
//
// DESCRIPTION :
// Assignment operator.
//???DB.  Same as implicit?
//==============================================================================
{
	//???DB.  Does assignment for super class first?
	variables_list=variable_union.variables_list;

	return (*this);
}
