//******************************************************************************
// FILE : function_variable.cpp
//
// LAST MODIFIED : 19 May 2004
//
// DESCRIPTION :
// See function_variable.hpp
//
// NOTES:
// ???DB.  15feb04
//   Iterate by function group rather than atomic?
// ???DB.  01Mar04
//   Where Handle_iterator<> and Handle_iterator_representation<>
//   (function_handle_iterator.hpp) came from:
//   - Need to be able to step through the atomic variables that make up a
//     variable - an iterator.
//   - Iterators are very like pointers, so wanted protection -
//     reference_count'ing and "representation"
//   - Originally, needed to step through components and inputs - use templates.
//   What went wrong
//   - std::reverse_iterator keeps an iterator one ahead of where it is and to
//     do a dereference, makes a temporary copy of the iterator, decrements it
//     and dereferences it
//     - To start with, a copy for Handle_iterator increased the access count
//       on the representation without copying the representation and so, since
//       the state information is in the representation, both the original and
//       copy are decremented.  Changed to deep copy (reference_count obselete)
//     - For a Function_variable_matrix, the end iterator has a zero 
//       atomic_variable (part of representation).  When the temporary iterator
//       is decremented, an atomic_variable is created which is the last atomic
//       variable and a reference to this atomic variable is returned by the
//       dereference.  However, the temporary iterator is destroyed on leaving
//       dereference which in turn destroys the atomic_variable (before it can
//       be used).  This is fixed by using Handle instead of Handle& for the
//       reference in std::iterator
//   Main problem:
//     Unlike for most iterators, there isn't a pool of objects with the
//     iterator pointing to one.  Instead there is a description of the pool and
//     the iterator has the information to create one
//   What to do:
//   - get rid of reference_count (deep copy)?  Yes
//   - get rid of representation?  No - would result in "slicing" when returned
//     Function_variable_iterator
//   - get rid of template?  Yes
//   - have an iterator which does not inherit from std::iterator?  No
// ???DB.  04Mar04
//   What are functions and variables?
//   - a function is a mapping.  It takes inputs to outputs
//   - variables specify inputs/outputs of functions
//   How should functions and variables be used?
//   - string representation?
//     - variable
//       - current value string
//       - need description string?
//     - function
//       - description string
//       - need names for all functions
//   - methods?
//     - only methods for functions are creation and variable creation?
//   - combinations (composite, union, intersection, ...)?
//     - for variables and not for functions?
//       - means that evaluate and evaluate_derivative, for variable, have to
//         return variables (not functions)
//   - how to write Newton-Raphson x[k+1]=x[k]-f(x[k])/f'(x[k])
//     - attempt 1
//       Function f;
//       Variable df_x_k,f_x_k,step,x,x_k,y;
//
//       x=f->input();  // OK
//       y=f->output();  // OK
//       f_x_k=(y->evaluate)(x,x_k);  // OK
//       df_x_k=(y->evaluate_derivative)(list(x),x,x_k);  // OK
//       step=solve(df_x_k,f_x_k);
//         // how to overload solve when only have Variable?  How to get matrix?
//       x_k -= step;
//         // how to overload operator-= when only have Variable?
//     - attempt 2
//       Function_handle f;
//       Function_matrix_handle df_x_k,step,x_k;
//       Variable f_x_k,x,y;
//
//       x=f->input();
//       y=f->output();
//       f_x_k=(y->evaluate)(x,x_k);
//       df_x_k=(((dynamic_pointer_cast<Function_derivative_matrix,Function>(
//         (y->evaluate_derivative)(list(x),x,x_k)))->function())->matrix)(
//         list(x));
//         // C++.  Can dynamic_pointer_cast be hidden by overloading operator=?
//         // Perl.  dynamic_pointer_cast hidden by using get_type_id_string
//       step=df_x_k->solve(f_x_k);
//       x_k -= step;
//     - attempt 2.  api
//       Cmiss_function_id f,step,x_k;
//       Cmiss_function_variable_id f_x_k,x,y;
//       Cmiss_function_variable_list_id independent_variables;
//
//       x=Cmiss_function_input(f);
//       independent_variables=Cmiss_function_variable_list_create();
//       Cmiss_function_variable_list_add(independent_variables,x);
//       y=Cmiss_function_output(f);
//       f_x_k=Cmiss_function_variable_evaluate(y,x,Cmiss_function_output(x_k));
//       df_x_k=Cmiss_function_derivative_matrix_get_matrix(
//         Cmiss_function_variable_function(
//         Cmiss_function_variable_evaluate_derivative(y,independent_variables,
//         x,Cmiss_function_output(x_k))),independent_variables);
//       step=Cmiss_function_matrix_solve(df_x_k,Cmiss_function_output(f_x_k));
//       Cmiss_function_decrement(x_k,step);
//     - attempt 2.  Perl
//       $x=$f->input();
//       $y=$f->output();
//       $f_x_k=$y->evaluate(input=>$x,value=>$x_k);
//       $df_x_k=(($y->evaluate_derivative(independent=>[$x],input=>$x,
//         value=>$x_k))->function())->matrix(independent=>[$x]);
//       $x_k -= $df_x_k->solve($f_x_k);
// ???DB.  05Mar04
//     Change to Function_variable::evaluate and evaluate_derivative returning
//     Function_handles - this means that need combinations (composite, union,
//     intersection, ...) of Functions.  Get rid of Function_variable::function.
//     Change to evaluate_derivative calculating a matrix rather than a
//     derivative matrix.  Don't have a method for calculating the derivative
//     matrix.  Maybe get rid of derivative_matrix altogether?
//     Change from Function_variable_handle to Function_handle for value
//     argument of evaluate, evaluate_derivative, set_value and rset_value.
//     Change from default arguments to two versions - with and without
//     input/value - for evaluate and evaluate_derivative.
//     Add operator= which do dynamic_pointer_cast for classes derived from
//     Function.  This is not an implicit type conversion (see Stroustrup 275)
//     Add Cmiss_function_decrement to api.
//     Overload operator-= for C++ and Perl.
//     - Perl
//       $x=$f->input();
//       $y=$f->output();
//       $f_x_k=$y->evaluate(input=>$x,value=>$x_k);
//       $df_x_k=$y->evaluate_derivative(independent=>[$x],input=>$x,
//         value=>$x_k);
//       $x_k -= $df_x_k->solve($f_x_k);
//     - api
//       Cmiss_function_id df_x_k,f,f_x_k,step,x_k;
//       Cmiss_function_variable_id x,y;
//       Cmiss_function_variable_list_id independent_variables;
//
//       x=Cmiss_function_input(f);
//       independent_variables=Cmiss_function_variable_list_create();
//       Cmiss_function_variable_list_add(independent_variables,x);
//       y=Cmiss_function_output(f);
//       f_x_k=Cmiss_function_variable_evaluate(y,x,x_k);
//       df_x_k=Cmiss_function_variable_evaluate_derivative(y,
//         independent_variables,x,x_k);
//       step=Cmiss_function_matrix_solve(df_x_k,Cmiss_function_output(f_x_k));
//       Cmiss_function_decrement(x_k,step);
//     - C++
//       Function_handle f;
//       Function_matrix_handle df_x_k,f_x_k,step,x_k;
//       Function_variable_handle x,y;
//       std::list<Function_variable_handle> independent_variables(0)
//
//       x=f->input();
//       independent_variables.push_back(x);
//       y=f->output();
//       f_x_k=y->evaluate(x,x_k->output());
//       df_x_k=y->evaluate_derivative(independent_variables,x,x_k);
//       step=df_x_k->solve(f_x_k);
//       x_k -= step;
// ???DB.  Stroustrup 281.  If implicit type conversion is desired for all
//   operands of an operation, the function implementing it must be a nonmember
//   function taking a const reference argument or a non-reference argument.
//==============================================================================

//???debug.  See t/37.com
//#define CHANGE_ELEMENT_DIMENSION

#include "computed_variable/function_derivative_matrix.hpp"
#include "computed_variable/function_variable.hpp"
#include "computed_variable/function_composite.hpp"

// class Function_variable_iterator_representation
// -----------------------------------------------

Function_variable_iterator_representation::
	Function_variable_iterator_representation()
//******************************************************************************
// LAST MODIFIED : 2 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Function_variable_iterator_representation::
	~Function_variable_iterator_representation()
//******************************************************************************
// LAST MODIFIED : 2 March 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{}


// class Function_variable_iterator
// --------------------------------

Function_variable_iterator::Function_variable_iterator():representation(0)
//******************************************************************************
// LAST MODIFIED : 2 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Function_variable_iterator::Function_variable_iterator(
	Function_variable_iterator_representation *representation):
	representation(representation)
//******************************************************************************
// LAST MODIFIED : 2 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Function_variable_iterator& Function_variable_iterator::operator=(
	const Function_variable_iterator& iterator)
//******************************************************************************
// LAST MODIFIED : 3 March 2004
//
// DESCRIPTION :
// Assignment.
//==============================================================================
{
	delete representation;
	representation=0;
	if (iterator.representation)
	{
		representation=(iterator.representation)->clone();
	}

	return (*this);
}

Function_variable_iterator::Function_variable_iterator(
	const Function_variable_iterator& iterator):representation(0)
//******************************************************************************
// LAST MODIFIED : 2 March 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (iterator.representation)
	{
		representation=(iterator.representation)->clone();
	}
}

Function_variable_iterator::~Function_variable_iterator()
//******************************************************************************
// LAST MODIFIED : 2 March 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	delete representation;
}

Function_variable_iterator& Function_variable_iterator::operator++()
//******************************************************************************
// LAST MODIFIED : 2 March 2004
//
// DESCRIPTION :
// Increment (prefix).
//==============================================================================
{
	if (representation)
	{
		representation->increment();
	}

	return (*this);
}

Function_variable_iterator Function_variable_iterator::operator++(int)
//******************************************************************************
// LAST MODIFIED : 2 March 2004
//
// DESCRIPTION :
// Increment (postfix).
//==============================================================================
{
	Function_variable_iterator tmp= *this;

	if (representation)
	{
		representation->increment();
	}

	return (tmp);
}

Function_variable_iterator& Function_variable_iterator::operator--()
//******************************************************************************
// LAST MODIFIED : 2 March 2004
//
// DESCRIPTION :
// Decrement (prefix).
//==============================================================================
{
	if (representation)
	{
		representation->decrement();
	}

	return (*this);
}

Function_variable_iterator Function_variable_iterator::operator--(int)
//******************************************************************************
// LAST MODIFIED : 2 March 2004
//
// DESCRIPTION :
// Decrement (postfix).
//==============================================================================
{
	Function_variable_iterator tmp= *this;

	if (representation)
	{
		representation->decrement();
	}

	return (tmp);
}

bool Function_variable_iterator::operator==(
	const Function_variable_iterator& iterator) const
//******************************************************************************
// LAST MODIFIED : 2 March 2004
//
// DESCRIPTION :
// Equality.
//==============================================================================
{
	bool result;

	result=false;
	if (representation)
	{
		result=(representation->equality)(iterator.representation);
	}
	else
	{
		if (!(iterator.representation))
		{
			result=true;
		}
	}

	return (result);
}

bool Function_variable_iterator::operator!=(
	const Function_variable_iterator& iterator) const
//******************************************************************************
// LAST MODIFIED : 2 March 2004
//
// DESCRIPTION :
// Inequality.
//==============================================================================
{
	bool result;

	result=false;
	if (this)
	{
		result= !((*this)==iterator);
	}

	return (result);
}

Function_variable_handle Function_variable_iterator::operator*() const
//******************************************************************************
// LAST MODIFIED : 2 March 2004
//
// DESCRIPTION :
// Dereference.
//==============================================================================
{
	Function_variable_handle result(0);

	if (representation)
	{
		result=(representation->dereference)();
	}
	return (result);
}


// class Function_variable
// -----------------------

Function_handle Function_variable::function()
//******************************************************************************
// LAST MODIFIED : 10 March 2004
//
// DESCRIPTION :
// This is the default.  Also by default, function_private is zero.
//==============================================================================
{
	return (function_private);
}

Function_variable_value_handle Function_variable::value()
//******************************************************************************
// LAST MODIFIED : 10 March 2004
//
// DESCRIPTION :
// This is the default.  Also by default, value_private is zero.
//==============================================================================
{
	return (value_private);
}

Function_handle Function_variable::evaluate()
//******************************************************************************
// LAST MODIFIED : 5 March 2004
//
// DESCRIPTION :
//???DB.  Merge atomic_results functions ie. join scalars onto vectors?
//==============================================================================
{
	Function_handle result(0);

	if (this)
	{
		Function_variable_iterator atomic_iterator=begin_atomic();
		std::list<Function_handle> atomic_results(0);

		// do the local evaluate
		while (atomic_iterator!=end_atomic())
		{
			Function_handle atomic_result;

			Assert((*atomic_iterator)&&((*atomic_iterator)->function()),
				std::logic_error(
				"Function_variable::evaluate().  Atomic variable missing function()"));
			if (atomic_result=
				((*atomic_iterator)->function())->evaluate(*atomic_iterator))
			{
				atomic_results.push_back(atomic_result);
			}
			atomic_iterator++;
		}
		if (0<atomic_results.size())
		{
			result=Function_handle(new Function_composite(atomic_results));
		}
	}

	return (result);
}

Function_handle Function_variable::evaluate(Function_variable_handle input,
	Function_handle value)
//******************************************************************************
// LAST MODIFIED : 5 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);

	if (this)
	{
		Function_handle current_value(0);

		if (input&&value)
		{
			// get the specified value
			current_value=input->evaluate();
			// override the specified value
			input->set_value(value);
		}
		result=evaluate();
		if (input&&value)
		{
			// reset the current value (reverse order to evaluate'ing)
			input->rset_value(current_value);
		}
	}

	return (result);
}

Function_handle Function_variable::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 11 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);

	if (this)
	{
		//???DB.  Do what the Function_derivative_matrix constructor does for a
		//  single matrix here.  Get rid of Function_derivative_matrix?
		//???DB.  Have to use handle.  Using Function_derivative_matrix would mess
		//  up the reference count
		Function_derivative_matrix_handle derivative_matrix(
			new Function_derivative_matrix(Function_variable_handle(this),
			independent_variables));

		result=derivative_matrix->matrix(independent_variables);
	}

	return (result);
}

Function_handle Function_variable::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables,
	Function_variable_handle input,Function_handle value)
//******************************************************************************
// LAST MODIFIED : 5 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);

	if (this)
	{
		Function_handle current_value(0);

		if (input)
		{
			// get the specified value
			current_value=input->evaluate();
			// override the specified value
			input->set_value(value);
		}
		result=evaluate_derivative(independent_variables);
		if (input)
		{
			// reset the current value (reverse order to evaluate'ing)
			input->rset_value(current_value);
		}
	}

	return (result);
}

bool Function_variable::set_value(Function_handle value)
//******************************************************************************
// LAST MODIFIED : 19 May 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_handle value_output;

	result=false;
	if (this&&value&&(value_output=value->output()))
	{
		Function_variable_iterator atomic_value_iterator=
			value_output->begin_atomic(),atomic_variable_iterator=begin_atomic();

		// not using std::for_each because then functor would have to be a friend of
		//   Function
		// there needs to be just on value_output.  (value->output())->end_atomic()
		//   would be for a different variable than atomic_variable_iterator
		while ((atomic_variable_iterator!=end_atomic())&&
			(atomic_value_iterator!=value_output->end_atomic()))
		{
			Assert((*atomic_variable_iterator)&&
				((*atomic_variable_iterator)->function()),std::logic_error(
				"Function_variable::set_value().  "
				"Atomic variable missing function()"));
#if defined (CHANGE_ELEMENT_DIMENSION)
			//???DB.  To handle when changing to an element with a different
			// dimension
			//???DB.  Won't work in general
			if (((*atomic_variable_iterator)->function())->set_value(
				*atomic_variable_iterator,*atomic_value_iterator))
			{
				result=true;
				atomic_value_iterator++;
			}
			atomic_variable_iterator++;
#else // defined (CHANGE_ELEMENT_DIMENSION)
			if (((*atomic_variable_iterator)->function())->set_value(
				*atomic_variable_iterator,*atomic_value_iterator))
			{
				result=true;
			}
			atomic_variable_iterator++;
			atomic_value_iterator++;
#endif // defined (CHANGE_ELEMENT_DIMENSION)
		}
	}

	return (result);
}

bool Function_variable::rset_value(Function_handle value)
//******************************************************************************
// LAST MODIFIED : 19 May 2004
//
// DESCRIPTION :
// Setting from back to front
//==============================================================================
{
	bool result;
	Function_variable_handle value_output;

	result=false;
	if (this&&value&&(value_output=value->output()))
	{
		std::reverse_iterator<Function_variable_iterator>
			atomic_value_iterator=value_output->rbegin_atomic(),
			atomic_variable_iterator=rbegin_atomic();

		// not using std::for_each because then functor would have to be a friend of
		//   Function
		// there needs to be just on value_output.  (value->output())->end_atomic()
		//   would be for a different variable than atomic_variable_iterator
		while ((atomic_variable_iterator!=rend_atomic())&&
			(atomic_value_iterator!=value_output->rend_atomic()))
		{
			Assert((*atomic_variable_iterator)&&
				((*atomic_variable_iterator)->function()),std::logic_error(
				"Function_variable::rset_value().  "
				"Atomic variable missing function()"));
#if defined (CHANGE_ELEMENT_DIMENSION)
			//???DB.  To handle when changing to an element with a different
			// dimension
			//???DB.  Won't work in general
			if (((*atomic_variable_iterator)->function())->set_value(
				*atomic_variable_iterator,*atomic_value_iterator))
			{
				atomic_value_iterator++;
				result=true;
			}
			atomic_variable_iterator++;
#else // defined (CHANGE_ELEMENT_DIMENSION)
			if (((*atomic_variable_iterator)->function())->set_value(
				*atomic_variable_iterator,*atomic_value_iterator))
			{
				result=true;
			}
			atomic_variable_iterator++;
			atomic_value_iterator++;
#endif // defined (CHANGE_ELEMENT_DIMENSION)
		}
	}

	return (result);
}

class Function_variable_sum_number_differentiable_functor
//******************************************************************************
// LAST MODIFIED : 19 March 2004
//
// DESCRIPTION :
// A unary function (functor) for summing the number_differentiable of the
// atomic variables that make up a variable.
//==============================================================================
{
	public:
		Function_variable_sum_number_differentiable_functor(
			Function_size_type& sum):sum(sum)
		{
			sum=0;
		};
		int operator() (const Function_variable_handle& variable)
		{
			sum += variable->number_differentiable();
			return (0);
		};
	private:
		Function_size_type& sum;
};

Function_size_type Function_variable::number_differentiable()
//******************************************************************************
// LAST MODIFIED : 19 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_size_type sum;

	// get the specified values
	std::for_each(begin_atomic(),end_atomic(),
		Function_variable_sum_number_differentiable_functor(sum));

	return (sum);
}

bool Function_variable::operator==(const Function_variable& variable) const
//******************************************************************************
// LAST MODIFIED : 18 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_iterator iterator_1,iterator_2;
#if defined (OLD_CODE)
	//???DB.  More complicated than this
	static int call_count=0;
#endif // defined (OLD_CODE)

#if defined (OLD_CODE)
	//???DB.  More complicated than this
	call_count++;
	Assert(1==call_count,std::logic_error("Function_variable::operator==.  "
		"Infinite recursion resulting from testing equality of source variable in "
		"Function_variable_iterator_representation::equality"));
#endif // defined (OLD_CODE)
	result=true;
	iterator_1=begin_atomic();
	iterator_2=variable.begin_atomic();
	while (result&&(iterator_1!=end_atomic())&&
		(iterator_2!=variable.end_atomic()))
	{
		result=(*iterator_1)->equality_atomic(*iterator_2);
		iterator_1++;
		iterator_2++;
	}
	if (result)
	{
		result=((iterator_1==end_atomic())&&(iterator_2==variable.end_atomic()));
	}
#if defined (OLD_CODE)
	//???DB.  More complicated than this
	call_count--;
#endif // defined (OLD_CODE)

	return (result);
}

Scalar Function_variable::norm() const
//******************************************************************************
// LAST MODIFIED : 13 February 2004
//
// DESCRIPTION :
// Calculates the norm of <this>.  A negative result means that the norm is not
// defined.
//
// This is the default and returns a negative Scalar.
//==============================================================================
{
	return ((Scalar)-1);
}

Function_variable_handle Function_variable::operator-(
	const Function_variable&) const
//******************************************************************************
// LAST MODIFIED : 13 February 2004
//
// DESCRIPTION :
// This is the default and returns a zero handle (failure).
//==============================================================================
{
	return (Function_variable_handle(0));
}

Function_variable_handle Function_variable::operator-=(const Function_variable&)
//******************************************************************************
// LAST MODIFIED : 13 February 2004
//
// DESCRIPTION :
// This is the default and returns a zero handle (failure).
//==============================================================================
{
	return (Function_variable_handle(0));
}

Function_variable_handle Function_variable::operator+(
	const Function_variable&) const
//******************************************************************************
// LAST MODIFIED : 13 February 2004
//
// DESCRIPTION :
// This is the default and returns a zero handle (failure).
//==============================================================================
{
	return (Function_variable_handle(0));
}

Function_variable_handle Function_variable::operator+=(const Function_variable&)
//******************************************************************************
// LAST MODIFIED : 13 February 2004
//
// DESCRIPTION :
// This is the default and returns a zero handle (failure).
//==============================================================================
{
	return (Function_variable_handle(0));
}

Function_variable::Function_variable():function_private(0),value_private(0),
	reference_count(0)
//******************************************************************************
// LAST MODIFIED : 10 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Function_variable::Function_variable(const Function_handle& function):
	function_private(function),value_private(0),reference_count(0)
//******************************************************************************
// LAST MODIFIED : 10 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Function_variable::Function_variable(const Function_variable& variable):
	function_private(variable.function_private),
	value_private(variable.value_private),reference_count(0)
//******************************************************************************
// LAST MODIFIED : 10 March 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{}

Function_variable::~Function_variable()
//******************************************************************************
// LAST MODIFIED : 11 February 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

void intrusive_ptr_add_ref(Function_variable *variable)
//******************************************************************************
// LAST MODIFIED : 11 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (variable)
	{
		(variable->reference_count)++;
	}
}

void intrusive_ptr_release(Function_variable *variable)
//******************************************************************************
// LAST MODIFIED : 11 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (variable)
	{
		(variable->reference_count)--;
		if (variable->reference_count<=0)
		{
			delete variable;
		}
	}
}
