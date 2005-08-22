//******************************************************************************
// FILE : variable.hpp
//
// LAST MODIFIED : 9 February 2004
//
// DESCRIPTION :
// Variable's are expressions that are constructed for:
// - display eg. difference between measured and calculated positions
// - minimization eg. fitting by minimizing the difference between measured and
// 	 calculated positions
// - solution eg. solving a FEM variational formulation equals zero
//
// Variable's are able to be:
// - evaluated at a point (specific choice of values for independent variables)
// - differentiated at a point (specific choice of values for independent
// 	 variables ie. not symbolic)
// - composed ie. the results of one Variable can replace independent
// 	 variables for another Variable
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
#if !defined (__VARIABLE_HPP__)
#define __VARIABLE_HPP__

#include "computed_variable/variable_base.hpp"

#include <list>
#include <string>

//???DB.  Move to variable_base?  begin
//???DB.  Numerics need to go somewhere that can be shared with api
typedef double Scalar;

#include "boost/numeric/ublas/matrix.hpp"
#include "boost/numeric/ublas/vector.hpp"
#include "boost/numeric/ublas/io.hpp"

namespace ublas = boost::numeric::ublas;

// use column_major so that can use lapack=boost::numeric::bindings::lapack
typedef ublas::matrix<Scalar,ublas::column_major> Matrix;
typedef ublas::vector<Scalar> Vector;

//???DB.  Replace with smart pointer or return reference?
typedef std::string * string_handle;
//???DB.  Move to variable_base?  end

#if defined (USE_VARIABLE_INPUT)
#include "computed_variable/variable_input.hpp"
#else // defined (USE_VARIABLE_INPUT)
#include "computed_variable/variable_io_specifier.hpp"
#endif // defined (USE_VARIABLE_INPUT)

#if defined (USE_ITERATORS)
//#define USE_VARIABLE_ITERATORS
#endif // defined (USE_ITERATORS)

// forward declaration
class Variable;

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable> Variable_handle;
void intrusive_ptr_add_ref(Variable *);
void intrusive_ptr_release(Variable *);
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable> Variable_handle;
#else
typedef Variable * Variable_handle;
#endif

class Variable_input_value
//******************************************************************************
// LAST MODIFIED : 2 February 2003
//
// DESCRIPTION :
// An input/value pair.
//
//???DB.  Should this be Variable_io_specifier_value ie. use for output as well?
//==============================================================================
{
	public:
		// constructor
		Variable_input_value(
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			& input,Variable_handle& value);
		// copy constructor
		Variable_input_value(const Variable_input_value&);
		// assignment
		Variable_input_value& operator=(const Variable_input_value&);
		// destructor
		~Variable_input_value();
		// get input
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			 input() const;
		// get value
		Variable_handle value() const;
	private:
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			input_data;
		Variable_handle value_data;
#if defined (USE_INTRUSIVE_SMART_POINTER)
	private:
		int reference_count;
		friend void intrusive_ptr_add_ref(Variable_input_value *);
		friend void intrusive_ptr_release(Variable_input_value *);
#endif // defined (USE_INTRUSIVE_SMART_POINTER)
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_value> Variable_input_value_handle;
void intrusive_ptr_add_ref(Variable_input_value *);
void intrusive_ptr_release(Variable_input_value *);
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_value> Variable_input_value_handle;
#else
typedef Variable_input_value * Variable_input_value_handle;
#endif

class Variable
//******************************************************************************
// LAST MODIFIED : 4 February 2004
//
// DESCRIPTION :
//???DB.  Almost all the public methods could be non-pure virtual so that have
//  a default, but can over-ride.
//???DB.  Add a size_input_value method so that input_values don't have to be
//  friends of their associated variables?
//  ???DB.  Is this a multimethod - virtual function dispatch based on more
//    than one object (Alexandrescu, Chapter 11)?
//==============================================================================
{
	// so that Variable_derivative_matrix constructor can call
	//   evaluate_derivative_local
	friend class Variable_derivative_matrix_create_matrices_inner_functor;
	friend class Variable_derivative_matrix_create_matrices_outer_functor;
	public:
		// destructor.  Virtual for proper destruction of derived classes
			//???DB.  Would like to be protected, but have some operations where need
			//  to create Variable_handles which means need destructor for smart
			//  pointers
		virtual ~Variable();
		// components are indivisible
#if defined (USE_ITERATORS)
		// returns the number of components that are differentiable
		virtual Variable_size_type number_differentiable() const =0;
#if defined (USE_VARIABLES_AS_COMPONENTS)
		virtual bool is_component()=0;
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
		// for stepping through the components that make up the Variable
#if defined (USE_VARIABLES_AS_COMPONENTS)
#if defined (USE_ITERATORS_NESTED)
		// need an iterator instead of a Variable_handle because Variable_handle
		//   does not have ++.  Also want a different ++ for each variable type
		class Iterator:public std::iterator<std::input_iterator_tag,Variable_handle>
			//???DB.  Virtual methods can't be pure so that begin_components and
			//  end_components can return Iterators
		{
			public:
				// constructor
				Iterator();
				// destructor.  Virtual for proper destruction of derived classes
				virtual ~Iterator();
				// assignment
				virtual Iterator& operator=(const Iterator&);
				// increment (prefix)
				virtual Iterator& operator++();
				// increment (postfix)
				virtual Iterator operator++(int);
				// equality
				virtual bool operator==(const Iterator&);
				// inequality
				virtual bool operator!=(const Iterator&);
				// dereference
				virtual Variable_handle& operator*() const;
				// don't have a operator-> because its not needed and it would return
				//   a Variable_handle*
		};
		virtual Iterator begin_components()=0;
		virtual Iterator end_components()=0;
#else // defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Handle_iterator<Variable_handle> begin_components()=0;
		virtual Handle_iterator<Variable_handle> end_components()=0;
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // defined (USE_ITERATORS_NESTED)
#else // defined (USE_VARIABLES_AS_COMPONENTS)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Handle_iterator<Variable_io_specifier_handle> begin_components()=0;
		virtual Handle_iterator<Variable_io_specifier_handle> end_components()=0;
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
#else // defined (USE_ITERATORS)
		// get the number of scalars in the result
		virtual Variable_size_type size() const =0;
		// get the scalars in the result
		virtual Vector *scalars()=0;
#endif // defined (USE_VARIABLE_ITERATORS)
		// evaluate method which does the general work and then calls the virtual
		//   private evaluate.  Made virtual so that operators (eg composition) can
		//   overload it.
			//???DB.  Change into a () operator ?
		virtual Variable_handle evaluate(std::list<Variable_input_value_handle>&
			values);
			//???DB.  Something wrong here.  Want the type returned by evaluate to be
			//  able to be different for a derived class and not necessarily the
			//  derived class eg finite element would return a vector of reals.
			//???DB.  OK because vector of reals is still a Variable?  No, because
			//  argument and return objects are copied/sliced.  OK now that return
			//  type has been changed to Variable_handle as long as don't want to
			//  derive from Variable handle
			//???DB.  Means that we need run-time type checking (bad)?
		// evaluate derivative method which does the general work and then calls the
		//   virtual private evaluate derivative.  Made virtual so that operators
		//   (eg composition) can overload it.
		virtual Variable_handle evaluate_derivative(
			std::list<
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			>& independent_variables,
			std::list<Variable_input_value_handle>& values);
		// get input value method which does the general work and then calls the
		//   virtual get_input_value_local
		Variable_handle get_input_value(const
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			& input);
		// set input value method which does the general work and then calls the
		//   virtual set_input_value_local
		bool set_input_value(const
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			& input,
			const
#if defined (USE_VARIABLES_AS_COMPONENTS)
			Variable_handle
#else // defined (USE_VARIABLES_AS_COMPONENTS)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
			& value);
		// get string representation method which does the general work and then
		//   calls the virtual get_string_representation_local
			//???DB.  Is there any general work?  Should this be (non-pure) virtual
			//  and have no private method?
		string_handle get_string_representation();
		// the norm of a variable.  A negative result means that the norm is not
		//   defined
		virtual Scalar norm() const;
		virtual Variable_handle operator-(const Variable&) const;
		virtual Variable_handle operator-=(const Variable&);
		virtual Variable_handle clone() const=0;
	protected:
		// constructor.  Protected so that can't create "plain" Variables
		Variable();
	private:
		// copy operations are private and undefined to prevent copying
		Variable(const Variable&);
		void operator=(const Variable&);
		// virtual evaluate method which is specified by each sub-class
		virtual Variable_handle evaluate_local()=0;
			//???DB.  Something wrong here.  Want the type returned by evaluate to
			//  be able to be different for a derived class and not necessarily the
			//  derived class eg finite element would return a vector of reals
		// virtual evaluate derivative method which is specified by each sub-class.
#if defined (USE_ITERATORS)
#if defined (OLD_CODE)
		//???DB.  I don't think independent_variables should be a list of Iterators?
		virtual bool evaluate_derivative_local(Scalar& derivative_value,
			std::list<Variable_input::Iterator>& independent_variables)=0;
#endif // defined (OLD_CODE)
		// adds columns for the specified derivatives to the end of the matrix
#else // defined (USE_ITERATORS)
		// fills in the matrix assuming that it has been zeroed
#endif // defined (USE_ITERATORS)
		virtual bool evaluate_derivative_local(Matrix& matrix,
			std::list<
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			>& independent_variables)=0;
		// virtual get input value method which is specified by each sub-class
		virtual Variable_handle get_input_value_local(
			const
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			& input_atomic)=0;
		// virtual set input value method which is specified by each sub-class.
		virtual bool set_input_value_local(
			const
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			& input_atomic,
			const
#if defined (USE_VARIABLES_AS_COMPONENTS)
			Variable_handle
#else // defined (USE_VARIABLES_AS_COMPONENTS)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
			& value)=0;
		// virtual get string representation method which is specified by each
		//   sub-class
		virtual string_handle get_string_representation_local()=0;
#if defined (USE_INTRUSIVE_SMART_POINTER)
	private:
		int reference_count;
		friend void intrusive_ptr_add_ref(Variable *);
		friend void intrusive_ptr_release(Variable *);
#endif // defined (USE_INTRUSIVE_SMART_POINTER)
};

class Variable_set_input_values
//******************************************************************************
// LAST MODIFIED : 9 February 2004
//
// DESCRIPTION :
// A unary function (Functor) for setting a list of input values for a variable.
//==============================================================================
{
	public:
		Variable_set_input_values(Variable_handle variable):variable(variable){};
		~Variable_set_input_values() {};
		int operator() (Variable_input_value_handle& input_value)
		{
			int result;

			result=0;
			if (input_value&&(variable->set_input_value)(input_value->input(),
				input_value->value()))
			{
				result=1;
			}

			return (result);
		};
	private:
		Variable_handle variable;
};

class Variable_get_input_values
//******************************************************************************
// LAST MODIFIED : 2 February 2004
//
// DESCRIPTION :
// A unary function (Functor) for getting a list of input values for a variable.
//==============================================================================
{
	friend class Variable;
	public:
		Variable_get_input_values(Variable_handle variable,
			std::list<Variable_input_value_handle> &values):variable(variable),
			values(values){};
		~Variable_get_input_values() {};
		int operator() (Variable_input_value_handle& input_value)
		{
			Variable_handle value;
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
				input;

			input=input_value->input();
			value=(variable->get_input_value)(input);
			values.push_back(Variable_input_value_handle(new Variable_input_value(
				input,value)));

			return (1);
		};
	private:
		Variable_handle variable;
		std::list<Variable_input_value_handle>& values;
			//???DB.  Seems that should be using transform, but then have to set up
			//  the result to the correct size
};

#if defined (OLD_CODE)
//???DB.  Investigating using templates instead of inheritance
class VVariable;

class VVariable_input_value;

template<class T,class U> class VVariable_handle;

template<class T> class VVariable_handle<T,VVariable>
{
	public:
		VVariable_handle<T,VVariable>
			evaluate(std::list<VVariable_input_value>& values);
	private:
		VVariable* variable_private;
};
#endif // defined (OLD_CODE)
#endif /* !defined (__VARIABLE_HPP__) */
