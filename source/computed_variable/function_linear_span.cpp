//******************************************************************************
// FILE : function_linear_span.cpp
//
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
//==============================================================================

#include <sstream>

#include "computed_variable/function_linear_span.hpp"
#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable.hpp"
#include "computed_variable/function_variable_exclusion.hpp"
#include "computed_variable/function_variable_matrix.hpp"

// module classes
// ==============

// class Function_variable_linear_span
// -----------------------------------

class Function_variable_linear_span : public Function_variable_matrix<Scalar>
//******************************************************************************
// LAST MODIFIED : 3 December 2004
//
// DESCRIPTION :
// Evaluates to a vector.
//==============================================================================
{
	friend class Function_linear_span;
	public:
		// constructor
		Function_variable_linear_span(
			const Function_linear_span_handle& function_linear_span):
			Function_variable_matrix<Scalar>(function_linear_span){};
		Function_variable_linear_span(
			const Function_linear_span_handle& function_linear_span,
			const Function_size_type row,const Function_size_type column):
			Function_variable_matrix<Scalar>(function_linear_span,row,column){};
		// destructor
		~Function_variable_linear_span(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(new Function_variable_linear_span(
				*this)));
		};
		Function_handle evaluate()
		{
			Function_handle result(0);
			Function_linear_span_handle function_linear_span=
				boost::dynamic_pointer_cast<Function_linear_span,Function>(function());

			if (function_linear_span)
			{
#if defined (BEFORE_CACHING)
				boost::intrusive_ptr< Function_matrix<Scalar> > spanned_value;
				Function_handle spanning_value;
				Function_size_type number_of_spanned_columns,number_of_spanned_rows,
					number_of_spanned_values,number_of_spanning_values;
				Function_variable_handle spanned_variable,spanning_variable;

				if ((spanned_variable=function_linear_span->spanned_variable_private)&&
					(spanning_variable=function_linear_span->spanning_variable_private)&&
					(spanning_value=spanning_variable->get_value())&&
					(0<(number_of_spanning_values=spanning_variable->
					number_differentiable())))
				{
					Matrix basis_matrix(number_of_spanning_values,1);

					basis_matrix.clear();
					basis_matrix(0,0)=1;
					if (spanning_variable->set_value(Function_handle(
						new Function_matrix<Scalar>(basis_matrix))))
					{
						if ((spanned_value=boost::dynamic_pointer_cast<
							Function_matrix<Scalar>,Function>(spanned_variable->evaluate()))&&
							(0<(number_of_spanned_rows=spanned_value->number_of_rows()))&&
							(0<(number_of_spanned_columns=spanned_value->
							number_of_columns())))
						{
							Function_size_type k;

							number_of_spanned_values=
								number_of_spanned_rows*number_of_spanned_columns;
							if (0==row_private)
							{
								bool valid;
								Matrix temp_matrix(1,1);
								boost::intrusive_ptr< Function_matrix<Scalar> >
									one_value(new Function_matrix<Scalar>(temp_matrix)),
									zero_value(new Function_matrix<Scalar>(temp_matrix));
								Function_size_type i,j;
								Function_variable_iterator spanning_variable_iterator,
									spanning_variable_iterator_end;
								Matrix &result_matrix=function_linear_span->values;

								result_matrix.resize(number_of_spanning_values*
									number_of_spanned_values,1);
								valid=true;
								(*one_value)(1,1)=1;
								(*zero_value)(1,1)=0;
								spanning_variable_iterator=spanning_variable->begin_atomic();
								spanning_variable_iterator_end=spanning_variable->end_atomic();
								k=0;
								while (valid&&
									(spanning_variable_iterator!=spanning_variable_iterator_end))
								{
									for (i=1;i<=number_of_spanned_rows;i++)
									{
										for (j=1;j<=number_of_spanned_columns;j++)
										{
											result_matrix(k,0)=(*spanned_value)(i,j);
											k++;
										}
									}
									(*spanning_variable_iterator)->set_value(zero_value);
									spanning_variable_iterator++;
									if (spanning_variable_iterator!=
										spanning_variable_iterator_end)
									{
										(*spanning_variable_iterator)->set_value(one_value);
										valid=(spanned_value=boost::dynamic_pointer_cast<
											Function_matrix<Scalar>,Function>(
											spanned_variable->evaluate()))&&
											(spanned_value->number_of_rows()==
											number_of_spanned_rows)&&
											(spanned_value->number_of_columns()==
											number_of_spanned_columns);
									}
								}
								if (valid)
								{
									result=
										Function_handle(new Function_matrix<Scalar>(result_matrix));
								}
							}
							else
							{
								Matrix result_matrix(1,1);

								k=(row_private-1)/number_of_spanned_values;
								basis_matrix(0,0)=0;
								basis_matrix(k,0)=1;
								if ((spanning_variable->set_value(Function_handle(
									new Function_matrix<Scalar>(basis_matrix))))&&
									(spanned_value=boost::dynamic_pointer_cast<
									Function_matrix<Scalar>,Function>(
									spanned_variable->evaluate()))&&
									(spanned_value->number_of_rows()==number_of_spanned_rows)&&
									(spanned_value->number_of_columns()==
									number_of_spanned_columns))
								{
									k=(row_private-1)%number_of_spanned_values;
									result_matrix(0,0)=(*spanned_value)(
										1+k/number_of_spanned_columns,
										1+k%number_of_spanned_columns);
									if (((function_linear_span->values).size1()!=
										number_of_spanning_values*number_of_spanned_values)||
										((function_linear_span->values).size2()!=1))
									{
										(function_linear_span->values).resize(
											number_of_spanning_values*number_of_spanned_values,1);
									}
									(function_linear_span->values)(row_private-1,0)=
										result_matrix(0,0);
									result=
										Function_handle(new Function_matrix<Scalar>(result_matrix));
								}
							}
						}
						spanning_variable->set_value(spanning_value);
					}
				}
#else // defined (BEFORE_CACHING)
				if (!(function_linear_span->evaluated()))
				{
					boost::intrusive_ptr< Function_matrix<Scalar> > spanned_value;
					Function_handle spanning_value;
					Function_size_type number_of_spanned_columns,number_of_spanned_rows,
						number_of_spanned_values,number_of_spanning_values;
					Function_variable_handle spanned_variable,spanning_variable;

					if ((spanned_variable=
						function_linear_span->spanned_variable_private)&&
						(spanning_variable=
						function_linear_span->spanning_variable_private)&&
						(spanning_value=spanning_variable->get_value())&&
						(0<(number_of_spanning_values=spanning_variable->
						number_differentiable())))
					{
						Matrix basis_matrix(number_of_spanning_values,1);

						basis_matrix.clear();
						basis_matrix(0,0)=1;
						if (spanning_variable->set_value(Function_handle(
							new Function_matrix<Scalar>(basis_matrix))))
						{
							bool valid;

							valid=false;
							if ((spanned_value=boost::dynamic_pointer_cast<
								Function_matrix<Scalar>,Function>(spanned_variable->
								evaluate()))&&
								(0<(number_of_spanned_rows=spanned_value->number_of_rows()))&&
								(0<(number_of_spanned_columns=spanned_value->
								number_of_columns())))
							{
								Matrix temp_matrix(1,1);
								boost::intrusive_ptr< Function_matrix<Scalar> >
									one_value(new Function_matrix<Scalar>(temp_matrix)),
									zero_value(new Function_matrix<Scalar>(temp_matrix));
								Function_size_type i,j,k;
								Function_variable_iterator spanning_variable_iterator,
									spanning_variable_iterator_end;
								Matrix &result_matrix=function_linear_span->values;

								number_of_spanned_values=
									number_of_spanned_rows*number_of_spanned_columns;

								result_matrix.resize(number_of_spanning_values*
									number_of_spanned_values,1);
								valid=true;
								(*one_value)(1,1)=1;
								(*zero_value)(1,1)=0;
								spanning_variable_iterator=spanning_variable->begin_atomic();
								spanning_variable_iterator_end=
									spanning_variable->end_atomic();
								k=0;
								while (valid&&(spanning_variable_iterator!=
									spanning_variable_iterator_end))
								{
									for (i=1;i<=number_of_spanned_rows;i++)
									{
										for (j=1;j<=number_of_spanned_columns;j++)
										{
											result_matrix(k,0)=(*spanned_value)(i,j);
											k++;
										}
									}
									(*spanning_variable_iterator)->set_value(zero_value);
									spanning_variable_iterator++;
									if (spanning_variable_iterator!=
										spanning_variable_iterator_end)
									{
										(*spanning_variable_iterator)->set_value(one_value);
										valid=(spanned_value=boost::dynamic_pointer_cast<
											Function_matrix<Scalar>,Function>(
											spanned_variable->evaluate()))&&
											(spanned_value->number_of_rows()==
											number_of_spanned_rows)&&
											(spanned_value->number_of_columns()==
											number_of_spanned_columns);
									}
								}
							}
							spanning_variable->set_value(spanning_value);
							if (valid)
							{
								function_linear_span->set_evaluated();
							}
						}
					}
				}
				if (function_linear_span->evaluated())
				{
					result=get_value();
				}
#endif // defined (BEFORE_CACHING)
			}

			return (result);
		};
		Function_handle evaluate_derivative(
			std::list<Function_variable_handle>& independent_variables)
		{
			boost::intrusive_ptr< Function_matrix<Scalar> > spanned_derivative;
			Function_linear_span_handle function_linear_span=
				boost::dynamic_pointer_cast<Function_linear_span,Function>(function());
			Function_handle result(0),spanning_value;
			Function_size_type number_of_derivatives,number_of_spanned_values,
				number_of_spanning_values;
			Function_variable_handle spanned_variable,spanning_variable;

			if (function_linear_span&&
				(spanned_variable=function_linear_span->spanned_variable_private)&&
				(spanning_variable=function_linear_span->spanning_variable_private)&&
				(spanning_value=spanning_variable->get_value())&&
				(0<(number_of_spanning_values=spanning_variable->
				number_differentiable())))
			{
				Matrix basis_matrix(number_of_spanning_values,1);

				basis_matrix.clear();
				basis_matrix(0,0)=1;
				if (spanning_variable->set_value(Function_handle(
					new Function_matrix<Scalar>(basis_matrix))))
				{
					if ((spanned_derivative=boost::dynamic_pointer_cast<
						Function_matrix<Scalar>,Function>(spanned_variable->
						evaluate_derivative(independent_variables)))&&
						(0<(number_of_spanned_values=
						spanned_derivative->number_of_rows()))&&
						(0<(number_of_derivatives=spanned_derivative->number_of_columns())))
					{
						Function_size_type j,k;

						if (0==row_private)
						{
							bool valid;
							Matrix temp_matrix(1,1);
							boost::intrusive_ptr< Function_matrix<Scalar> >
								one_value(new Function_matrix<Scalar>(temp_matrix)),
								zero_value(new Function_matrix<Scalar>(temp_matrix));
							Function_size_type i;
							Function_variable_iterator spanning_variable_iterator,
								spanning_variable_iterator_end;
							Matrix result_matrix(
								number_of_spanning_values*number_of_spanned_values,
								number_of_derivatives);

							valid=true;
							(*one_value)(1,1)=1;
							(*zero_value)(1,1)=0;
							spanning_variable_iterator=spanning_variable->begin_atomic();
							spanning_variable_iterator_end=spanning_variable->end_atomic();
							k=0;
							while (valid&&
								(spanning_variable_iterator!=spanning_variable_iterator_end))
							{
								for (i=1;i<=number_of_spanned_values;i++)
								{
									for (j=1;j<=number_of_derivatives;j++)
									{
										result_matrix(k,j-1)=(*spanned_derivative)(i,j);
									}
									k++;
								}
								(*spanning_variable_iterator)->set_value(zero_value);
								spanning_variable_iterator++;
								if (spanning_variable_iterator!=spanning_variable_iterator_end)
								{
									(*spanning_variable_iterator)->set_value(one_value);
									valid=(spanned_derivative=boost::dynamic_pointer_cast<
										Function_matrix<Scalar>,Function>(
										spanned_variable->evaluate_derivative(
										independent_variables)))&&
										(spanned_derivative->number_of_rows()==
										number_of_spanned_values)&&
										(spanned_derivative->number_of_columns()==
										number_of_derivatives);
								}
							}
							if (valid)
							{
								result=
									Function_handle(new Function_matrix<Scalar>(result_matrix));
							}
						}
						else
						{
							Matrix result_matrix(1,number_of_derivatives);

							k=(row_private-1)/number_of_spanned_values;
							basis_matrix(0,0)=0;
							basis_matrix(k,0)=1;
							if ((spanning_variable->set_value(Function_handle(
								new Function_matrix<Scalar>(basis_matrix))))&&
								(spanned_derivative=boost::dynamic_pointer_cast<
								Function_matrix<Scalar>,Function>(
								spanned_variable->evaluate_derivative(independent_variables)))&&
								(spanned_derivative->number_of_rows()==
								number_of_spanned_values)&&
								(spanned_derivative->number_of_columns()==
								number_of_derivatives))
							{
								k=1+(row_private-1)%number_of_spanned_values;
								for (j=1;j<=number_of_derivatives;j++)
								{
									result_matrix(0,j-1)=(*spanned_derivative)(k,j);
								}
								result=
									Function_handle(new Function_matrix<Scalar>(result_matrix));
							}
						}
					}
					spanning_variable->set_value(spanning_value);
				}
			}

			return (result);
		};
		string_handle get_string_representation()
		{
			Function_linear_span_handle function_linear_span=
				boost::dynamic_pointer_cast<Function_linear_span,Function>(function());
			string_handle return_string(0);

			if (return_string=new std::string)
			{
				std::ostringstream out;

				out << "linear_span(";
				if (function_linear_span&&
					(function_linear_span->spanned_variable_private))
				{
					out << *(function_linear_span->spanned_variable_private->
						get_string_representation());
				}
				out << ",";
				if (function_linear_span&&
					(function_linear_span->spanning_variable_private))
				{
					out << *(function_linear_span->spanning_variable_private->
						get_string_representation());
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
				out << ",1]";
				*return_string=out.str();
			}

			return (return_string);
		};
	private:
		// copy constructor
		Function_variable_linear_span(
			const Function_variable_linear_span& variable_linear_span):
			Function_variable_matrix<Scalar>(variable_linear_span){};
};

// global classes
// ==============

// class Function_linear_span
// --------------------------

ublas::matrix<Scalar,ublas::column_major>
	Function_linear_span::constructor_values(0,0);

Function_linear_span::Function_linear_span(
	const Function_variable_handle& spanned_variable,
	const Function_variable_handle& spanning_variable):
	Function_matrix<Scalar>(Function_linear_span::constructor_values),
	spanned_variable_private(spanned_variable),
	spanning_variable_private(spanning_variable)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (spanned_variable_private)
	{
		spanned_variable_private->add_dependent_function(this);
	}
}

Function_linear_span::~Function_linear_span()
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
	if (spanned_variable_private)
	{
		spanned_variable_private->remove_dependent_function(this);
	}
#endif // defined (CIRCULAR_SMART_POINTERS)
}

string_handle Function_linear_span::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 10 November 2004
//
// DESCRIPTION :
//???DB.  Overload << instead of get_string_representation?
//==============================================================================
{
	string_handle return_string(0);

	if (return_string=new std::string)
	{
		std::ostringstream out;

		out << "linear_span(";
		if (spanned_variable_private)
		{
			out << *(spanned_variable_private->get_string_representation());
		}
		out << ",";
		if (spanning_variable_private)
		{
			out << *(spanning_variable_private->get_string_representation());
		}
		out << ")";
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_handle Function_linear_span::input()
//******************************************************************************
// LAST MODIFIED : 10 November 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_handle result(0);

	if (spanned_variable_private)
	{
		result=Function_variable_handle(new Function_variable_exclusion(
			(spanned_variable_private->function())->input(),
			spanning_variable_private));
	}

	return (result);
}

Function_variable_handle Function_linear_span::output()
//******************************************************************************
// LAST MODIFIED : 10 November 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_linear_span(
		Function_linear_span_handle(this))));
}

bool Function_linear_span::operator==(const Function& function) const
//******************************************************************************
// LAST MODIFIED : 10 November 2004
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
			const Function_linear_span& function_linear_span=
				dynamic_cast<const Function_linear_span&>(function);

			result=equivalent(spanned_variable_private,
				function_linear_span.spanned_variable_private)&&
				equivalent(spanning_variable_private,
				function_linear_span.spanning_variable_private);
		}
		catch (std::bad_cast)
		{
			// do nothing
		}
	}

	return (result);
}

Function_handle Function_linear_span::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 10 November 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	boost::intrusive_ptr<Function_variable_linear_span>
		atomic_variable_linear_span;

	if (this&&(atomic_variable_linear_span=boost::dynamic_pointer_cast<
		Function_variable_linear_span,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_linear_span->function())&&
		(0<atomic_variable_linear_span->row())&&
		(0<atomic_variable_linear_span->column()))
	{
		result=(atomic_variable_linear_span->evaluate)();
	}

	return (result);
}

bool Function_linear_span::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 10 November 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_handle atomic_variable_local;
	boost::intrusive_ptr<Function_variable_linear_span>
		atomic_variable_linear_span;

	result=false;
	if (this&&(atomic_variable_linear_span=boost::dynamic_pointer_cast<
		Function_variable_linear_span,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_linear_span->function())&&
		(0<atomic_variable_linear_span->row())&&
		(0<atomic_variable_linear_span->column())&&
		(0<atomic_independent_variables.size()))
	{
		boost::intrusive_ptr< Function_matrix<Scalar> > derivative_matrix=
			boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
			atomic_variable_linear_span->evaluate_derivative(
			atomic_independent_variables));

		if (derivative_matrix)
		{
			result=true;
			derivative=(*derivative_matrix)(1,1);
		}
	}

	return (result);
}

bool Function_linear_span::set_value(Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 1 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr<Function_variable_linear_span>
		atomic_variable_linear_span;
	boost::intrusive_ptr< Function_variable_value_specific<Scalar> > value_type;
	Function_handle function;

	result=false;
	if ((atomic_variable_linear_span=boost::dynamic_pointer_cast<
		Function_variable_linear_span,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_linear_span->function())&&
		atomic_value&&(atomic_value->value())&&(value_type=
		boost::dynamic_pointer_cast<Function_variable_value_specific<Scalar>,
		Function_variable_value>(atomic_value->value())))
	{
		result=value_type->set(values((atomic_variable_linear_span->row())-1,
			(atomic_variable_linear_span->column())-1),atomic_value);
	}
	if (result)
	{
		set_not_evaluated();
	}
	else
	{
		if (spanned_variable_private&&
			(function=spanned_variable_private->function()))
		{
			result=function->set_value(atomic_variable,atomic_value);
		}
		if (spanning_variable_private&&
			(function=spanning_variable_private->function()))
		{
			if (function->set_value(atomic_variable,atomic_value))
			{
				result=true;
			}
		}
	}

	return (result);
}

Function_handle Function_linear_span::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 10 November 2004
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr<Function_variable_linear_span>
		atomic_variable_linear_span;
	Function_handle function,result;
	ublas::matrix<Scalar,ublas::column_major> result_matrix(1,1);

	result=0;
	if (atomic_variable&&
		equivalent(Function_handle(this),(atomic_variable->function)())&&
		(atomic_variable_linear_span=boost::dynamic_pointer_cast<
		Function_variable_linear_span,Function_variable>(atomic_variable))&&
		(atomic_variable_linear_span->get_entry)(result_matrix(0,0)))
	{
		result=Function_handle(new Function_matrix<Scalar>(result_matrix));
	}
	if (!result)
	{
		if (spanned_variable_private&&
			(function=spanned_variable_private->function()))
		{
			result=function->get_value(atomic_variable);
		}
		if (!result)
		{
			if (spanning_variable_private&&
				(function=spanning_variable_private->function()))
			{
				result=function->get_value(atomic_variable);
			}
		}
	}

	return (result);
}

Function_linear_span::Function_linear_span(
	const Function_linear_span& function_linear_span):
	Function_matrix<Scalar>(function_linear_span),
	spanned_variable_private(function_linear_span.spanned_variable_private),
	spanning_variable_private(function_linear_span.spanning_variable_private)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (spanned_variable_private)
	{
		spanned_variable_private->add_dependent_function(this);
	}
}

Function_linear_span& Function_linear_span::operator=(
	const Function_linear_span& function_linear_span)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	if (function_linear_span.spanned_variable_private)
	{
		function_linear_span.spanned_variable_private->add_dependent_function(this);
	}
	if (spanned_variable_private)
	{
		spanned_variable_private->remove_dependent_function(this);
	}
	spanned_variable_private=function_linear_span.spanned_variable_private;
	spanning_variable_private=function_linear_span.spanning_variable_private;

	return (*this);
}
