//******************************************************************************
// FILE : function_matrix_resize_implementation.cpp
//
// LAST MODIFIED : 22 April 2005
//
// DESCRIPTION :
//==============================================================================
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <sstream>

#include "computed_variable/function_matrix_resize.hpp"
#include "computed_variable/function_variable.hpp"
#include "computed_variable/function_variable_matrix.hpp"
#include "computed_variable/function_variable_union.hpp"
#include "computed_variable/function_variable_value_specific.hpp"


// module classes
// ==============

// class Function_variable_matrix_resize
// -------------------------------------

EXPORT template<typename Value_type>
class Function_variable_matrix_resize :
	public Function_variable_matrix<Value_type>
//******************************************************************************
// LAST MODIFIED : 22 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_matrix<Value_type>;
	public:
		// constructor
		Function_variable_matrix_resize(
			const boost::intrusive_ptr< Function_matrix_resize<Value_type> >
			function_matrix_resize):
			Function_variable_matrix<Value_type>(function_matrix_resize
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			,false
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			){};
		Function_variable_matrix_resize(
			const boost::intrusive_ptr< Function_matrix_resize<Value_type> >
			function_matrix_resize,const Function_size_type row,
			const Function_size_type column):Function_variable_matrix<Value_type>(
			function_matrix_resize,
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			false,
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			row,column){};
		~Function_variable_matrix_resize(){}
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_matrix_resize<Value_type>(*this)));
		};
#if defined (EVALUATE_RETURNS_VALUE)
		Function_handle evaluate()
		{
			Function_handle result(0);
			boost::intrusive_ptr< Function_matrix_resize<Value_type> >
				function_matrix_resize;

			if (function_matrix_resize=boost::dynamic_pointer_cast<
				Function_matrix_resize<Value_type>,Function>(this->function()))
			{
#if defined (BEFORE_CACHING)
				Function_size_type number_of_columns,number_of_columns_input,
					number_of_rows,size;
				boost::intrusive_ptr< Function_matrix<Value_type> > matrix;

				if ((matrix=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
					Function>(function_matrix_resize->matrix_private->evaluate()))&&
					(0<(size=(number_of_columns_input=matrix->number_of_columns())*
					(matrix->number_of_rows())))&&(0<(number_of_columns=
					function_matrix_resize->number_of_columns_private))&&
					(0==size%number_of_columns)&&
					(row_private<=(number_of_rows=size/number_of_columns))&&
					(column_private<=number_of_columns))
				{
					Function_size_type i,j,k;

					function_matrix_resize->values.resize(number_of_rows,
						number_of_columns);
					k=0;
					for (i=0;i<number_of_rows;++i)
					{
						for (j=0;j<number_of_columns;++j)
						{
							function_matrix_resize->values(i,j)=(*matrix)(
								k/number_of_columns_input+1,k%number_of_columns_input+1);
							++k;
						}
					}
					if (0==row_private)
					{
						if (0==column_private)
						{
							result=Function_handle(new Function_matrix<Value_type>(
								function_matrix_resize->values));
						}
						else
						{
							ublas::matrix<Value_type,ublas::column_major>
								result_matrix(number_of_rows,1);

							for (i=0;i<number_of_rows;++i)
							{
								result_matrix(i,0)=(function_matrix_resize->values)(
									i,column_private-1);
							}
							result=Function_handle(new Function_matrix<Value_type>(
								result_matrix));
						}
					}
					else
					{
						if (0==column_private)
						{
							ublas::matrix<Value_type,ublas::column_major>
								result_matrix(1,number_of_columns);

							for (j=0;j<number_of_columns;++j)
							{
								result_matrix(0,j)=(function_matrix_resize->values)(
									row_private-1,j);
							}
							result=Function_handle(new Function_matrix<Value_type>(
								result_matrix));
						}
						else
						{
							ublas::matrix<Value_type,ublas::column_major>
								result_matrix(1,1);
							
							result_matrix(0,0)=(function_matrix_resize->values)(
								row_private-1,column_private-1);
							result=Function_handle(new Function_matrix<Value_type>(
								result_matrix));
						}
					}
				}
#else // defined (BEFORE_CACHING)
				if (!(function_matrix_resize->evaluated()))
				{
					Function_size_type number_of_columns,number_of_columns_input,
						number_of_rows,size;
					boost::intrusive_ptr< Function_matrix<Value_type> > matrix;

					if ((matrix=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
						Function>(function_matrix_resize->matrix_private->evaluate()))&&
						(0<(size=(number_of_columns_input=matrix->number_of_columns())*
						(matrix->number_of_rows())))&&(0<(number_of_columns=
						function_matrix_resize->number_of_columns_private))&&
						(0==size%number_of_columns)&&
						(this->row_private<=(number_of_rows=size/number_of_columns))&&
						(this->column_private<=number_of_columns))
					{
						Function_size_type i,j,k;

						function_matrix_resize->values.resize(number_of_rows,
							number_of_columns);
						k=0;
						for (i=0;i<number_of_rows;++i)
						{
							for (j=0;j<number_of_columns;++j)
							{
								function_matrix_resize->values(i,j)=(*matrix)(
									k/number_of_columns_input+1,k%number_of_columns_input+1);
								++k;
							}
						}
						function_matrix_resize->set_evaluated();
					}
				}
				if (function_matrix_resize->evaluated())
				{
					result=this->get_value();
				}
#endif // defined (BEFORE_CACHING)
			}

			return (result);
		};
#else // defined (EVALUATE_RETURNS_VALUE)
		bool evaluate()
		{
			bool result(true);
			boost::intrusive_ptr< Function_matrix_resize<Value_type> >
				function_matrix_resize;

			if (function_matrix_resize=boost::dynamic_pointer_cast<
				Function_matrix_resize<Value_type>,Function>(function()))
			{
#if defined (BEFORE_CACHING)
				Function_size_type number_of_columns,number_of_columns_input,
					number_of_rows,size;
				boost::intrusive_ptr< Function_matrix<Value_type> > matrix;

				result=false;
				if ((function_matrix_resize->matrix_private->evaluate)()&&
					(matrix=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
					Function>(function_matrix_resize->matrix_private->get_value()))&&
					(0<(size=(number_of_columns_input=matrix->number_of_columns())*
					(matrix->number_of_rows())))&&(0<(number_of_columns=
					function_matrix_resize->number_of_columns_private))&&
					(0==size%number_of_columns)&&
					(row_private<=(number_of_rows=size/number_of_columns))&&
					(column_private<=number_of_columns))
				{
					Function_size_type i,j,k;

					function_matrix_resize->values.resize(number_of_rows,
						number_of_columns);
					k=0;
					for (i=0;i<number_of_rows;++i)
					{
						for (j=0;j<number_of_columns;++j)
						{
							function_matrix_resize->values(i,j)=(*matrix)(
								k/number_of_columns_input+1,k%number_of_columns_input+1);
							++k;
						}
					}
					result=true;
				}
#else // defined (BEFORE_CACHING)
				if (!(function_matrix_resize->evaluated()))
				{
					Function_size_type number_of_columns,number_of_columns_input,
						number_of_rows,size;
					boost::intrusive_ptr< Function_matrix<Value_type> > matrix;

					result=false;
					if ((function_matrix_resize->matrix_private->evaluate)()&&
						(matrix=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
						Function>(function_matrix_resize->matrix_private->get_value()))&&
						(0<(size=(number_of_columns_input=matrix->number_of_columns())*
						(matrix->number_of_rows())))&&(0<(number_of_columns=
						function_matrix_resize->number_of_columns_private))&&
						(0==size%number_of_columns)&&
						(row_private<=(number_of_rows=size/number_of_columns))&&
						(column_private<=number_of_columns))
					{
						Function_size_type i,j,k;

						function_matrix_resize->values.resize(number_of_rows,
							number_of_columns);
						k=0;
						for (i=0;i<number_of_rows;++i)
						{
							for (j=0;j<number_of_columns;++j)
							{
								function_matrix_resize->values(i,j)=(*matrix)(
									k/number_of_columns_input+1,k%number_of_columns_input+1);
								++k;
							}
						}
						function_matrix_resize->set_evaluated();
						result=true;
					}
				}
#endif // defined (BEFORE_CACHING)
			}

			return (result);
		};
#endif // defined (EVALUATE_RETURNS_VALUE)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_handle evaluate_derivative(std::list<Function_variable_handle>&);
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		virtual Function_handle derivative(
			const std::list<Function_variable_handle>&);
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		//???DB.  Should operator() and get_entry do an evaluate?
		boost::intrusive_ptr< Function_variable_matrix<Value_type> > operator()(
			Function_size_type row=1,Function_size_type column=1) const
		{
			boost::intrusive_ptr< Function_matrix_resize<Value_type> >
				function_matrix_resize;
			boost::intrusive_ptr< Function_variable_matrix<Value_type> > result(0);

			if ((function_matrix_resize=boost::dynamic_pointer_cast<
				Function_matrix_resize<Value_type>,Function>(this->function_private))&&
				(row<=this->number_of_rows())&&(column<=this->number_of_columns()))
			{
				result=boost::intrusive_ptr< Function_variable_matrix<Value_type> >(
					new Function_variable_matrix_resize<Value_type>(
					function_matrix_resize,row,column));
			}

			return (result);
		}
	private:
		// copy constructor
		Function_variable_matrix_resize(
			const Function_variable_matrix_resize<Value_type>& variable):
			Function_variable_matrix<Value_type>(variable){};
};


// global classes
// ==============

// class Function_matrix_resize
// ----------------------------

EXPORT template<typename Value_type>
ublas::matrix<Value_type,ublas::column_major> 
	Function_matrix_resize<Value_type>::constructor_values(0,0);

EXPORT template<typename Value_type>
Function_matrix_resize<Value_type>::Function_matrix_resize(
	const Function_variable_handle& matrix,
	const Function_size_type number_of_columns):Function_matrix<Value_type>(
	Function_matrix_resize<Value_type>::constructor_values),
	number_of_columns_private(number_of_columns),matrix_private(matrix)
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
Function_matrix_resize<Value_type>::~Function_matrix_resize()
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
string_handle Function_matrix_resize<Value_type>::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 23 November 2004
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
			out << "resize(";
			out << *(matrix_private->get_string_representation());
			out << ",n_columns=";
			out << number_of_columns_private;
			out << ")";
		}
		else
		{
			out << "Invalid Function_matrix_resize";
		}
		*return_string=out.str();
	}

	return (return_string);
}

EXPORT template<typename Value_type>
Function_variable_handle Function_matrix_resize<Value_type>::input()
//******************************************************************************
// LAST MODIFIED : 7 October 2004
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
Function_variable_handle Function_matrix_resize<Value_type>::output()
//******************************************************************************
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(
		new Function_variable_matrix_resize<Value_type>(
		boost::intrusive_ptr< Function_matrix_resize<Value_type> >(this))));
}

EXPORT template<typename Value_type>
bool Function_matrix_resize<Value_type>::operator==(
	const Function& function) const
//******************************************************************************
// LAST MODIFIED : 7 October 2004
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
			const Function_matrix_resize<Value_type>& function_matrix_resize=
				dynamic_cast<const Function_matrix_resize<Value_type>&>(function);

			result=
				equivalent(matrix_private,function_matrix_resize.matrix_private);
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
	Function_matrix_resize<Value_type>::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 13 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr< Function_variable_matrix_resize<Value_type> >
		atomic_variable_matrix_resize;
#if defined (EVALUATE_RETURNS_VALUE)
	Function_handle result(0);
#else // defined (EVALUATE_RETURNS_VALUE)
	bool result(true);
#endif // defined (EVALUATE_RETURNS_VALUE)

	if ((atomic_variable_matrix_resize=boost::dynamic_pointer_cast<
		Function_variable_matrix_resize<Value_type>,Function_variable>(
		atomic_variable))&&equivalent(Function_handle(this),
		atomic_variable_matrix_resize->function())&&
		(0<atomic_variable_matrix_resize->row())&&
		(0<atomic_variable_matrix_resize->column()))
	{
		result=atomic_variable_matrix_resize->evaluate();
	}

	return (result);
}

EXPORT template<typename Value_type>
bool Function_matrix_resize<Value_type>::evaluate_derivative(Scalar&,
	Function_variable_handle,std::list<Function_variable_handle>&)
//******************************************************************************
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (false);
}

#if !defined (AIX)
template<>
bool Function_matrix_resize<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables);
#endif // !defined (AIX)

EXPORT template<typename Value_type>
bool Function_matrix_resize<Value_type>::set_value(
	Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 1 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_resize<Value_type> >
		atomic_matrix_variable;
	boost::intrusive_ptr< Function_variable_value_specific<Value_type> >
		value_type;
	Function_handle function;

	result=false;
	if ((atomic_matrix_variable=boost::dynamic_pointer_cast<
		Function_variable_matrix_resize<Value_type>,Function_variable>(
		atomic_variable))&&
		equivalent(Function_handle(this),atomic_matrix_variable->function())&&
		atomic_value&&(atomic_value->value())&&(value_type=
		boost::dynamic_pointer_cast<Function_variable_value_specific<Value_type>,
		Function_variable_value>(atomic_value->value())))
	{
		result=value_type->set(values((atomic_matrix_variable->row())-1,
			(atomic_matrix_variable->column())-1),atomic_value);
	}
	if (result)
	{
		this->set_not_evaluated();
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
Function_handle Function_matrix_resize<Value_type>::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle function,result(0);
	boost::intrusive_ptr< Function_variable_matrix_resize<Value_type> >
		atomic_variable_matrix_resize;
	ublas::matrix<Value_type,ublas::column_major> result_matrix(1,1);

	if (atomic_variable&&
		equivalent(Function_handle(this),(atomic_variable->function)())&&
		(atomic_variable_matrix_resize=boost::dynamic_pointer_cast<
		Function_variable_matrix_resize<Value_type>,Function_variable>(
		atomic_variable))&&
		(atomic_variable_matrix_resize->get_entry)(result_matrix(0,0)))
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
Function_matrix_resize<Value_type>::Function_matrix_resize(
	const Function_matrix_resize<Value_type>& function_matrix_resize):
	Function_matrix<Value_type>(function_matrix_resize),
	number_of_columns_private(function_matrix_resize.number_of_columns_private),
	matrix_private(function_matrix_resize.matrix_private)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (matrix_private)
	{
		matrix_private->add_dependent_function(Function_handle(this));
	}
}

EXPORT template<typename Value_type>
Function_matrix_resize<Value_type>&
	Function_matrix_resize<Value_type>::operator=(
	const Function_matrix_resize<Value_type>& function_matrix_resize)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	number_of_columns_private=
		function_matrix_resize.number_of_columns_private;
	if (this->function_matrix_determinant.matrix_private)
	{
		this->function_matrix_determinant.matrix_private->add_dependent_function(this);
	}
	if (matrix_private)
	{
		matrix_private->remove_dependent_function(this);
	}
	matrix_private=function_matrix_resize.matrix_private;
	this->values=function_matrix_resize.values;

	return (*this);
}
