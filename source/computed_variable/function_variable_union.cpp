//******************************************************************************
// FILE : function_variable_union.cpp
//
// LAST MODIFIED : 30 March 2005
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

// module classes
// ==============

// class Function_variable_iterator_representation_atomic_union
// ------------------------------------------------------------

bool repeat_atomic_variable(
	const std::list<Function_variable_handle>& variables_list,
	const std::list<Function_variable_handle>::iterator& variables_list_iterator,
	const Function_variable_iterator& atomic_variable_iterator)
//******************************************************************************
// LAST MODIFIED : 5 October 2004
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
			local_atomic_variable_iterator++;
		}
		local_variables_list_iterator++;
	}

	return (repeat);
}

class Function_variable_iterator_representation_atomic_union: public
	Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 12 March 2004
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
						variables_list_iterator++;
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
					atomic_variable_iterator++;
				} while ((atomic_variable_iterator!=
					((*variables_list_iterator)->end_atomic)())&&
					repeat_atomic_variable(source->variables_list,
					variables_list_iterator,atomic_variable_iterator));
				if (atomic_variable_iterator==
					((*variables_list_iterator)->end_atomic)())
				{
					do
					{
						variables_list_iterator++;
						if (variables_list_iterator!=(source->variables_list).end())
						{
							atomic_variable_iterator=
								((*variables_list_iterator)->begin_atomic)();
							while ((atomic_variable_iterator!=
								((*variables_list_iterator)->end_atomic)())&&
								repeat_atomic_variable(source->variables_list,
								variables_list_iterator,atomic_variable_iterator))
							{
								atomic_variable_iterator++;
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
						variables_list_iterator--;
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
					atomic_variable_iterator--;
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
						variables_list_iterator--;
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
// LAST MODIFIED : 11 January 2005
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
			iterator++;
			i--;
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
// LAST MODIFIED : 23 February 2005
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
				atomic_variable_index++;
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
						atomic_variable_index_iterator++)
					{
						while (atomic_variable_index!= *atomic_variable_index_iterator)
						{
							atomic_variable_index++;
							atomic_part_iterator++;
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
							atomic_variable_index_iterator++)
						{
							while (atomic_variable_index!= *atomic_variable_index_iterator)
							{
								atomic_variable_index++;
								column++;
								if (column>part_number_of_columns)
								{
									column=1;
									row++;
								}
							}
							new_matrix(i,0)=(*part_matrix)(row,column);
							i++;
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
						result_column++;
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
// LAST MODIFIED : 30 March 2005
//
// DESCRIPTION :
// ???DB.  To be done?
//==============================================================================
{
	return (false);
}
#endif // defined (EVALUATE_RETURNS_VALUE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
Function_handle Function_variable_union::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 25 February 2005
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
					atomic_variable_index++;
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
						for (column=1;column<=result_number_of_columns;column++)
						{
							new_matrix(row,column-1)=(*part)(atomic_variable_index,column);
						}
						row++;
						++atomic_variable_index_iterator;
						j--;
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
							result_column++)
						{
							result_matrix(result_row,result_column)=
								part_matrix(part_row,part_column);
							part_column++;
						}
						result_row++;
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
	const std::list<Function_variable_handle>&
//	independent_variables
	)
//******************************************************************************
// LAST MODIFIED : 30 March 2005
//
// DESCRIPTION :
// ???DB.  To be done
//==============================================================================
{
	return (Function_handle(0));
}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE_UNION_EVALUATE)

string_handle Function_variable_union::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 12 March 2004
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
			variable_iterator++;
			i--;
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
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Function_variable_handle>::iterator iterator,iterator_end;

	iterator_end=variables_list.end();
	for (iterator=variables_list.begin();iterator!=iterator_end;iterator++)
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
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Function_variable_handle>::iterator iterator,iterator_end;

	iterator_end=variables_list.end();
	for (iterator=variables_list.begin();iterator!=iterator_end;iterator++)
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
