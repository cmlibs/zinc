//******************************************************************************
// FILE : variable.cpp
//
// LAST MODIFIED : 4 February 2004
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
//
// NOTES :
// 1 Started as computed_variable.h
//   - C -> C++
//   - Cmiss_variable -> Variable
//     ???DB.  Need a more unique name than Variable?
//   - remove Cmiss_value
//   - use identifiers to specify independent variables rather than Variables
// 2 From "Effective C++" by Scott Meyers
//   11 Define a copy constructor and an assignment operator for classes with
//     dynamically allocated memory
// 3 Variable_identifier/Degree_of_freedom
//   - need to know how many values are referred to for calculating derivatives?
//     If don't know degree of freedom then derivative is zero and could have a
//     placeholder?  Difficult for composite (of known and unknown)?
//   - could have a method that returns a multi-range of indices
//   - is it "specific" to a particular variable (What about composition?  How
//     can you say differentiate wrt element/xi?) or may be used and mean
//     different things if applied to different variables?
// 4 Possible definitions
// 4.1 Variable has access to all its degrees of freedom (inputs) and a
//     Variable_identifier is a specifier for a list of degrees of freedom which
//     can resolve to locations in memory
//     ???DB.  Variables also have identifiers so that can comparisons easier?
//       Overlapping identifiers?
// 5 Pimpl - Pointer implementation.  http://c2.com/cgi-bin/wiki?PimplIdiom
// 6 To get correct linking with C functions, need
//     extern "C"
//     {
//     }
//   around header files
// 7 Started by using old dynamic memory allocation ie use new(nothrow), but
//   changed to new - program stops if throw an exception (without being caught)
// 8 Need -lstdc++ when start using standard library
// 9 I was trying too much by doing Pimpl at the same time as
//   Variable_inputs.  So moved Pimpl to *.pimpl and got rid of Pimpl for the
//   moment
// 10 Virtual destructors.  Needed if delete a pointer to a derived object when
//   the pointer is of type pointer to base class (so that correct destructor
//   is called and delete is called with the correct size).
// 11 "Virtual" constructor.  Constructors cannot be virtual, but can get the
//   desired effect - see Stroustrup 3rd:424.
// 12 Constructors have to construct their members (using : member initializer
//   list.
// 13 The members' constructors are called before the body of the containing
//   class' own constructor is executed.  The constructors are called in the
//   order in which the members are declared in the class (rather than the order
//   in which the members appear in the initializer list.  If a member
//   constructor needs no arguments, the member need not be mentioned in the
//   member initializer list.  Stroustrup 3rd:247-8
// 14 The member destructors are called in the reverse order of construction
//   after the body of the class' own destructor has executed.
//   Stroustrup 3rd:247
// 15 Member initializers are essential for types for which initialization
//   differs from assignment.  Stroustrup 3rd:248
// 16 A reference cannot be made to refer to another object at run-time.  A
//   reference must be initialized so that it refers to an object.  If a data
//   member is a reference, it must be initialized in the constructors'
//   initializer list.
// 17 A local class cannot be used as a template argument.  Lischner 138
// 18 By default a single argument constructor also defines an implicit
//   conversion.  Implicit conversion from a particular constructor can be
//   suppressed by declaring the constructor implicit.  Stroustrup 3rd:284
// 19 Smart pointers (avoid bare pointers - Lischner 618)
// 19.1 Suggestions Alexandrescu 157-
// 19.1.1 A smart pointer should not use member functions
// 19.2 Have added option (USE_SMART_POINTER) of boost::shared_ptr (can be used
//      with containers, std::autoptr cannot because it does a destructive
//      copy).  Tried Loki (Alexandrescu) but had lots of template compiler
//      errors which changed, but did not go away, as upgraded gcc
// 19.3 Having a smart pointer wrapper is not ideal because have to do things
//      differently in the api (if went to a Variable** for the bare pointer
//      case would then destructors wouldn't be called).  Could include smart
//      pointer into Variable (Pimpl), but lose code re-use
// 20 Are using the ideas of "automatic differentiation" (technology for
//    automatically augmenting computer programs with statements for the
//    computation of derivatives), see
//      http://www-sop.inria.fr/tropics/ad/whatisad.html,
//    to get derivatives
// 21 Tried templating the constructor for Variable_input so that don't need
//    string& and char* versions.  Problems
// 21.1 Can do if have template definition visible where the constructor is
//      instantiated, but this means having the definition in the .h file
// 21.2 Tried explicitly instantiating in .cpp eg
//        template Variable_input::Variable_input<char *>(char *);
//      but this doesn't compile because it doesn't have a return type.
//      However, a constructor doesn't have a return type (not even void).
//    Got round by just having string& and letting the string constructors
//    (called for pass by value) sort it out
// 22 Would like a general way of creating *_handle
// 22.1 Template handle<>.  Can't see how because want to rename a class, not
//      create a new one
// 22.2 Macro HANDLE() may have trouble with
//        #if defined (USE_SMART_POINTER)
//        #define HANDLE( object_type ) boost::shared_ptr<object_type>
//        #else // defined (USE_SMART_POINTER)
//        #define HANDLE( object_type ) object_type *
//        #endif // defined (USE_SMART_POINTER)
//
//        Variable_get_input_value get_input_value(HANDLE(Variable)(this));
//      when HANDLE is an ordinary pointer
// 23 Should only create a new _handle when create a new object to be pointed,
//   otherwise the object could be destroyed at the wrong time (eg when the
//   _handle that was created locally goes out of scope)
// 24 Changed to intrusive smart pointer because need to be able to construct
//   a smart pointer from any pointer (not just the new one)
//
// ???DB.  Need to sort out pointers and references
//   See Meyers 22,23
//   (Pass by) reference means that get the derived class rather than base class
//   Lischner 35 "A reference, unlike a pointer, cannot be made to refer to a
//     different object at runtime"
//   auto_ptr from STL?
// ???DB.  Variables seem to be midway between template and inheritence?
//   See Meyers 42
// ???DB.  Sort out constructors and destructors
//   Constructors and destructors are not inherited
// ???DB.  Sort out const for arguments
// ???DB.  References for return values?
// ???DB.  Merge Variable_input_composite into Variable_input (rather than
//   deriving so that don't have to use typeid)?
// ???DB.  Bad returning dereferenced new because delete will never be called
//   See Meyers 31
// ???DB.  Bad having methods that give access to data which is more private
//    than the method.  See Meyers ???DB
// ???DB.  What about exceptions?
// ???DB.  What about 0 smart/pointers?  Alternative is throwing an exception?
// ???DB.  What happens when a constructor fails?
// ???DB.  Make evaluate_derivative return a derivative matrix handle?
// ???DB.  Derivative and evaluate have to return NEW copies (otherwise original
//    could be modified)
// ???DB.  Can't create local Variable_handle of this because will end up being
//    destroyed.  Should only do Variable_handle(new )
//
// ???DB.  06Nov03.
//    Should size for Variable_input be moved to size_input_value for Variable?
//      for:
//      - don't have to make the inputs friends of the variable
//      - already doing for get and set input_value
//      against:
//      - more dynamic casts (indicates flakey polymorphism?)
//    Should a Variable_input always be associated with a Variable?  What about
//      Variable_input_composite?
//
// ???DB.  14Nov03
//   Overload << instead of get_string_representation?
//   Need get_string_representation for inputs to do
//     Variable_derivative::get_string_representation_local() properly?
//
// ???DB.  04Dec03
//   Need a result type method for variables?  Need type method for inputs?
//   Have Variable_output/result as well as Variable_input?  Merge?  Came from
//     Inverse
//   Do handles/smart pointers need to be set to 0 in destructors (to decrease
//     access count)?
//
// ???DB.  11Dec03
//   Would like to use const on methods more (means that <this> is not changed
//     (eg. virtual Variable_handle clone() const=0), but prevented because of
//     instrusive smart pointer
//
// ???DB.  04Feb04
//   Why are components needed?
//   - so that set_input_value can split input into atomic inputs and value into
//     components
//   Should components be variables or io_specifiers?
//   variable
//   - Have to be able to refer back to the variable that its a component of
//     eg. for a vector variable, a component is not just a scalar variable,
//     because it has to be the scalar at a particular location in the vector
//   io_specifier
//   - what should evaluate and evaluate_derivative do?
//   - should there be a derivative variable but no evaluate_derivative_method?
//   - should all variables store there result?  Then an io_specifier can always
//     be referring to memory.  A composite wouldn't have its own storage for
//     its result - maybe don't have composite variable, use composite
//     io_specifier instead
//==============================================================================

#include "computed_variable/variable_base.hpp"

#include <algorithm>
#include <iterator>
#include <new>
#include <sstream>
#include <string>
#include <typeinfo>

#include "computed_variable/variable.hpp"
#include "computed_variable/variable_composite.hpp"
#include "computed_variable/variable_derivative_matrix.hpp"
#if !defined (USE_ITERATORS)
#include "computed_variable/variable_input_composite.hpp"
#endif // !defined (USE_ITERATORS)

// class Variable_input_value
// --------------------------

Variable_input_value::Variable_input_value(
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	& input,Variable_handle& value):input_data(input),value_data(value)
#if defined (USE_INTRUSIVE_SMART_POINTER)
	,reference_count(0)
#endif
//******************************************************************************
// LAST MODIFIED : 2 February 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Variable_input_value::Variable_input_value(
	const Variable_input_value& input_value):input_data(input_value.input()),
	value_data(input_value.value())
#if defined (USE_INTRUSIVE_SMART_POINTER)
	,reference_count(0)
#endif
//******************************************************************************
// LAST MODIFIED : 16 October 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{}

Variable_input_value::~Variable_input_value()
//******************************************************************************
// LAST MODIFIED : 9 September 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

#if defined (USE_INTRUSIVE_SMART_POINTER)
void intrusive_ptr_add_ref(Variable_input_value *input_value)
//******************************************************************************
// LAST MODIFIED : 16 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	if (input_value)
	{
		(input_value->reference_count)++;
	}
}
#endif // defined (USE_INTRUSIVE_SMART_POINTER)

#if defined (USE_INTRUSIVE_SMART_POINTER)
void intrusive_ptr_release(Variable_input_value *input_value)
//******************************************************************************
// LAST MODIFIED : 16 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	if (input_value)
	{
		(input_value->reference_count)--;
		if (input_value->reference_count<=0)
		{
			delete input_value;
		}
	}
}
#endif // defined (USE_INTRUSIVE_SMART_POINTER)

Variable_handle Variable_input_value::value() const
//******************************************************************************
// LAST MODIFIED : 5 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (value_data);
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_input_value::input() const
//******************************************************************************
// LAST MODIFIED : 2 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (input_data);
}

// class Variable
// --------------

Variable::Variable()
#if defined (USE_INTRUSIVE_SMART_POINTER)
	:reference_count(0)
#endif
//******************************************************************************
// LAST MODIFIED : 16 October 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Variable::~Variable()
//******************************************************************************
// LAST MODIFIED : 7 September 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

#if defined (USE_INTRUSIVE_SMART_POINTER)
void intrusive_ptr_add_ref(Variable *variable)
//******************************************************************************
// LAST MODIFIED : 16 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	if (variable)
	{
		(variable->reference_count)++;
	}
}
#endif // defined (USE_INTRUSIVE_SMART_POINTER)

#if defined (USE_INTRUSIVE_SMART_POINTER)
void intrusive_ptr_release(Variable *variable)
//******************************************************************************
// LAST MODIFIED : 16 October 2003
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
#endif // defined (USE_INTRUSIVE_SMART_POINTER)

#if defined (USE_ITERATORS)
#if defined (USE_ITERATORS_NESTED)
Variable::Iterator::Iterator()
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Variable::Iterator::~Iterator()
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

#if defined (DO_NOT_WANT_TO_USE)
//???DB.  Want a compile or link time way of making sure that the Iterator
//  methods are defined for the Variable sub-classes
Variable::Iterator& Variable::Iterator::operator=(const Variable::Iterator&)
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
// Assignment.
//==============================================================================
{
	// should not come here - should be overloaded, but can't make pure
	Assert(false,std::logic_error(
		"Variable::Iterator::operator=().  Should not come here"));
	
	return (*this);
}

Variable::Iterator Variable::Iterator::operator++()
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
// Increment (prefix).
//==============================================================================
{
	// should not come here - should be overloaded, but can't make pure
	Assert(false,std::logic_error(
		"Variable::Iterator::operator++().  Should not come here"));
	
	return (*this);
}

Variable::Iterator Variable::Iterator::operator++(int)
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
// Increment (postfix).
//==============================================================================
{
	// should not come here - should be overloaded, but can't make pure
	Assert(false,std::logic_error(
		"Variable::Iterator::operator++(int).  Should not come here"));
	
	return (*this);
}

bool Variable::Iterator::operator==(const Iterator&)
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
// Equality.
//==============================================================================
{
	// should not come here - should be overloaded, but can't make pure
	Assert(false,std::logic_error(
		"Variable::Iterator::operator==(const Iterator&).  Should not come here"));
	
	return (false);
}

bool Variable::Iterator::operator!=(const Iterator&)
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
// Inquality.
//==============================================================================
{
	// should not come here - should be overloaded, but can't make pure
	Assert(false,std::logic_error(
		"Variable::Iterator::operator!=(const Iterator&).  Should not come here"));
	
	return (true);
}

Variable_handle& Variable::Iterator::operator*() const
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
// Dereference.
//==============================================================================
{
	Variable_handle *result_address=new Variable_handle(0);

	// should not come here - should be overloaded, but can't make pure
	Assert(false,std::logic_error(
		"Variable::Iterator::operator*().  Should not come here"));
	
	return (*result_address);
}
#endif // defined (DO_NOT_WANT_TO_USE)
#endif // defined (USE_ITERATORS_NESTED)
#endif // defined (USE_ITERATORS)

Variable_handle Variable::evaluate(
	std::list<Variable_input_value_handle>& values)
//******************************************************************************
// LAST MODIFIED : 1 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Variable_input_value_handle> current_values(0);
	Variable_handle result(0);

	// get the specified values
	std::for_each(values.begin(),values.end(),Variable_get_input_values(
		Variable_handle(this),current_values));
	// override the specified values
	std::for_each(values.begin(),values.end(),Variable_set_input_values(
		Variable_handle(this)));
	// do the local evaluate
	result=(evaluate_local)();
	// reset the current values
	std::for_each(current_values.rbegin(),current_values.rend(),
		Variable_set_input_values(Variable_handle(this)));

	return (result);
}

#if defined (USE_ITERATORS)
Variable_handle Variable::evaluate_derivative(
	std::list<
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	>& independent_variables,std::list<Variable_input_value_handle>& values)
//******************************************************************************
// LAST MODIFIED : 2 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Variable_input_value_handle> current_values(0);
	Variable_handle derivative(0);

	// get the specified values
	std::for_each(values.begin(),values.end(),Variable_get_input_values(
		Variable_handle(this),current_values));
	// override the specified values
	std::for_each(values.begin(),values.end(),Variable_set_input_values(
		Variable_handle(this)));
	// calculate the derivative matrix
	derivative=Variable_handle(new Variable_derivative_matrix(
		Variable_handle(this),independent_variables));
	// reset the current values
	std::for_each(current_values.rbegin(),current_values.rend(),
		Variable_set_input_values(this));

	return (derivative);
}
#else // defined (USE_ITERATORS)
class Variable_input_is_composite
//******************************************************************************
// LAST MODIFIED : 19 September 2003
//
// DESCRIPTION :
// A predicate for determining if an input is composite
//==============================================================================
{
	public:
		bool operator() (Variable_input_handle& input)
		{
			//???DB.  Not keen on using typeid.  Could merge being "composite" into
			//  Variable_composite rather than deriving Variable_input_composite
			return (typeid(*input)==typeid(Variable_input_composite));
		};
};

#if defined (GENERALIZE_COMPOSITE_INPUT)
Variable_handle Variable::evaluate_derivative(
	std::list<Variable_input_handle>& independent_variables,
	std::list<Variable_input_value_handle>& values)
//******************************************************************************
// LAST MODIFIED : 1 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Variable_input_handle>::iterator input_iterator;
	std::list<Variable_input_value_handle> current_values(0),no_values(0);
#if defined (GENERALIZE_COMPOSITE_INPUT)
#else // defined (GENERALIZE_COMPOSITE_INPUT)
	std::list<Variable_input_handle>::iterator input_composite_iterator;
	Variable_derivative_matrix_handle derivative_matrix;
#endif // defined (GENERALIZE_COMPOSITE_INPUT)
	Variable_handle derivative;
	Variable_input_composite_handle input_composite;
	Variable_input_is_composite is_composite_input_predicate;

	// get the specified values
	std::for_each(values.begin(),values.end(),Variable_get_input_values(
		Variable_handle(this),current_values));
	// override the specified values
	std::for_each(values.begin(),values.end(),Variable_set_input_values(
		Variable_handle(this)));
	// check for composite inputs in the list of independent_variables
	input_iterator=std::find_if(independent_variables.begin(),
		independent_variables.end(),is_composite_input_predicate);
	if (independent_variables.end()==input_iterator)
	{
		// none of the independent variables/inputs are composite
		// do the local evaluate derivative
		derivative=Variable_handle(new Variable_derivative_matrix(
			Variable_handle(this),independent_variables));
	}
	else
	{
		// an independent variable/input is composite
#if defined (USE_SMART_POINTER)
		input_composite=
			boost::dynamic_pointer_cast<Variable_input_composite,Variable_input>(
			*input_iterator);
#else /* defined (USE_SMART_POINTER) */
		input_composite=
			dynamic_cast<Variable_input_composite *>(*input_iterator);
#endif /* defined (USE_SMART_POINTER) */
#if defined (GENERALIZE_COMPOSITE_INPUT)
		derivative=input_composite->evaluate_derivative(Variable_handle(this),
			independent_variables,input_iterator,no_values);
#else // defined (GENERALIZE_COMPOSITE_INPUT)
		// calculate the derivatives for the variables/inputs making up the
		//   composite
		input_composite_iterator=input_composite->begin();
		*input_iterator= *input_composite_iterator;
		derivative=evaluate_derivative(independent_variables,no_values);
		derivative_matrix=
#if defined (USE_SMART_POINTER)
			boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>(
			derivative);
#else /* defined (USE_SMART_POINTER) */
			dynamic_cast<Variable_derivative_matrix *>(*derivative);
#endif /* defined (USE_SMART_POINTER) */
		input_composite_iterator++;
		Variable_input_composite_evaluate_derivative_functor
			evaluate_derivative_functor(Variable_handle(this),independent_variables,
			no_values,input_iterator,derivative_matrix);
		std::for_each(input_composite_iterator,input_composite->end(),
			evaluate_derivative_functor);
		*input_iterator=input_composite;
		// need to set independent_variables because evaluate_derivative copies
		derivative_matrix->independent_variables=independent_variables;
#endif // defined (GENERALIZE_COMPOSITE_INPUT)
	}
	// reset the current values
	std::for_each(current_values.rbegin(),current_values.rend(),
		Variable_set_input_values(this));

	return (derivative);
}
#else // defined (GENERALIZE_COMPOSITE_INPUT)
class Variable_input_composite_evaluate_derivative_functor
//******************************************************************************
// LAST MODIFIED : 31 December 2003
//
// DESCRIPTION :
// A unary functor (Functor) for merging derivative matrices for the different
// components of a composite input
//==============================================================================
{
	public:
		Variable_input_composite_evaluate_derivative_functor(
			const Variable_handle& variable,
			std::list<Variable_input_handle>& independent_variables,
			std::list<Variable_input_value_handle>& values,
			std::list<Variable_input_handle>::iterator& input_iterator,
			Variable_derivative_matrix_handle& result):input_iterator(input_iterator),
			independent_variables(independent_variables),values(values),
			independent_variable_number_of_values(0),result(result),variable(variable)
		{
			std::list<Matrix>::iterator result_iterator;
			std::list<Variable_input_handle>::iterator independent_variable_iterator;
			Variable_size_type i,j;

			input_index=0;
			result_input_iterator=result->independent_variables.begin();
			for (independent_variable_iterator=independent_variables.begin();
				independent_variable_iterator!=input_iterator;
				independent_variable_iterator++)
			{
				input_index++;
				result_input_iterator++;
			}
			independent_variable_number_of_values.resize(input_index);
			result_iterator=(result->matrices).begin();
			number_of_matrices=1;
			for (i=0;i<input_index;i++)
			{
				independent_variable_number_of_values[i]=result_iterator->size2();
				for (j=number_of_matrices;j>0;j--)
				{
					result_iterator++;
				}
				number_of_matrices *= 2;
			}
			for (i=independent_variables.size()-input_index;i>0;i--)
			{
				number_of_matrices *= 2;
			}
			number_of_matrices -= 1;
		};
		~Variable_input_composite_evaluate_derivative_functor() {};
		int operator() (Variable_input_handle& input)
		{
			int composite_independent_variable_factor,
				composite_independent_variable_step;
			std::list<Matrix>::iterator derivative_iterator,result_iterator;
			unsigned i,j,matrix_number,number_of_rows;
			Variable_derivative_matrix_handle derivative;
			Variable_handle derivative_uncast;
			Variable_size_type derivative_independent_number_of_values,
				result_independent_number_of_values;

			*input_iterator=input;
			// calculate the derivative for input
			derivative_uncast=variable->evaluate_derivative(independent_variables,
				values);
			derivative=
#if defined (USE_SMART_POINTER)
				boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>(
				derivative_uncast);
#else /* defined (USE_SMART_POINTER) */
				dynamic_cast<Variable_derivative_matrix *>(derivative_uncast);
#endif /* defined (USE_SMART_POINTER) */
			if (derivative)
			{
				// merge derivative matrix into result
				Assert((number_of_matrices==(derivative->matrices).size())&&
					(number_of_matrices==(result->matrices).size()),std::logic_error(
					"Variable_input_composite_evaluate_derivative_functor().  "
					"Incorrect number_of_matrices"));
				composite_independent_variable_factor=1;
				derivative_iterator=(derivative->matrices).begin();
				result_iterator=(result->matrices).begin();
				for (i=input_index;i>0;i--)
				{
					for (j=composite_independent_variable_factor;j>0;j--)
					{
						derivative_iterator++;
						result_iterator++;
					}
					composite_independent_variable_factor *= 2;
				}
				derivative_independent_number_of_values=derivative_iterator->size2();
				result_independent_number_of_values=result_iterator->size2();
				derivative_iterator=(derivative->matrices).begin();
				result_iterator=(result->matrices).begin();
				number_of_rows=result_iterator->size1();
				for (matrix_number=1;matrix_number<=number_of_matrices;matrix_number++)
				{
					Assert((number_of_rows==derivative_iterator->size1())&&
						(number_of_rows==result_iterator->size1()),std::logic_error(
						"Variable_input_composite_evaluate_derivative_functor().  "
						"Incorrect number_of_rows"));
					if ((matrix_number/composite_independent_variable_factor)%2)
					{
						/* derivative involves composite independent variable */
						int column_number,derivative_column_number,number_of_columns=
							(result_iterator->size2)()+(derivative_iterator->size2)(),
							result_column_number;
						Matrix matrix(number_of_rows,number_of_columns);

						composite_independent_variable_step=1;
						j=matrix_number;
						for (i=0;i<input_index;i++)
						{
							if (j%2)
							{
								composite_independent_variable_step *=
									independent_variable_number_of_values[i];
							}
							j /= 2;
						}
						derivative_column_number=0;
						result_column_number=0;
						column_number=0;
						while (column_number<number_of_columns)
						{
							for (j=composite_independent_variable_step*
								result_independent_number_of_values;j>0;j--)
							{
								for (i=0;i<number_of_rows;i++)
								{
									matrix(i,column_number)=
										(*result_iterator)(i,result_column_number);
								}
								result_column_number++;
								column_number++;
							}
							for (j=composite_independent_variable_step*
								derivative_independent_number_of_values;j>0;j--)
							{
								for (i=0;i<number_of_rows;i++)
								{
									matrix(i,column_number)=
										(*derivative_iterator)(i,derivative_column_number);
								}
								derivative_column_number++;
								column_number++;
							}
						}
						// update result matrix
						result_iterator->resize(number_of_rows,number_of_columns);
							//???DB.  Memory leak?
						*result_iterator=matrix;
					}
					derivative_iterator++;
					result_iterator++;
				}
			}
			else
			{
				//???DB.  Currently only know how to merge derivative matrices
			}
			// update independent variables for result
				//???DB.  Memory leak?
			*result_input_iterator=Variable_input_handle(
				new Variable_input_composite(*result_input_iterator,input));

			return (1);
		};
	private:
		std::list<Variable_input_handle>::iterator& input_iterator;
		std::list<Variable_input_handle>& independent_variables;
		std::list<Variable_input_handle>::iterator result_input_iterator;
		std::list<Variable_input_value_handle>& values;
		std::vector<Variable_size_type> independent_variable_number_of_values;
		Variable_derivative_matrix_handle& result;
		Variable_handle variable;
		Variable_size_type input_index,number_of_matrices;
};
#endif // defined (GENERALIZE_COMPOSITE_INPUT)

Variable_handle Variable::evaluate_derivative(
	std::list<Variable_input_handle>& independent_variables,
	std::list<Variable_input_value_handle>& values)
//******************************************************************************
// LAST MODIFIED : 1 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Variable_input_handle>::iterator input_iterator;
	std::list<Variable_input_value_handle> current_values(0),no_values(0);
#if defined (GENERALIZE_COMPOSITE_INPUT)
#else // defined (GENERALIZE_COMPOSITE_INPUT)
	std::list<Variable_input_handle>::iterator input_composite_iterator;
	Variable_derivative_matrix_handle derivative_matrix;
#endif // defined (GENERALIZE_COMPOSITE_INPUT)
	Variable_handle derivative;
	Variable_input_composite_handle input_composite;
	Variable_input_is_composite is_composite_input_predicate;

	// get the specified values
	std::for_each(values.begin(),values.end(),Variable_get_input_values(
		Variable_handle(this),current_values));
	// override the specified values
	std::for_each(values.begin(),values.end(),Variable_set_input_values(
		Variable_handle(this)));
	// check for composite inputs in the list of independent_variables
	input_iterator=std::find_if(independent_variables.begin(),
		independent_variables.end(),is_composite_input_predicate);
	if (independent_variables.end()==input_iterator)
	{
		// none of the independent variables/inputs are composite
		// do the local evaluate derivative
		derivative=Variable_handle(new Variable_derivative_matrix(
			Variable_handle(this),independent_variables));
	}
	else
	{
		// an independent variable/input is composite
#if defined (USE_SMART_POINTER)
		input_composite=
			boost::dynamic_pointer_cast<Variable_input_composite,Variable_input>(
			*input_iterator);
#else /* defined (USE_SMART_POINTER) */
		input_composite=
			dynamic_cast<Variable_input_composite *>(*input_iterator);
#endif /* defined (USE_SMART_POINTER) */
#if defined (GENERALIZE_COMPOSITE_INPUT)
		derivative=input_composite->evaluate_derivative(Variable_handle(this),
			independent_variables,input_iterator,no_values);
#else // defined (GENERALIZE_COMPOSITE_INPUT)
		// calculate the derivatives for the variables/inputs making up the
		//   composite
		input_composite_iterator=input_composite->begin();
		*input_iterator= *input_composite_iterator;
		derivative=evaluate_derivative(independent_variables,no_values);
		derivative_matrix=
#if defined (USE_SMART_POINTER)
			boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>(
			derivative);
#else /* defined (USE_SMART_POINTER) */
			dynamic_cast<Variable_derivative_matrix *>(*derivative);
#endif /* defined (USE_SMART_POINTER) */
		input_composite_iterator++;
		Variable_input_composite_evaluate_derivative_functor
			evaluate_derivative_functor(Variable_handle(this),independent_variables,
			no_values,input_iterator,derivative_matrix);
		std::for_each(input_composite_iterator,input_composite->end(),
			evaluate_derivative_functor);
		*input_iterator=input_composite;
		// need to set independent_variables because evaluate_derivative copies
		derivative_matrix->independent_variables=independent_variables;
#endif // defined (GENERALIZE_COMPOSITE_INPUT)
	}
	// reset the current values
	std::for_each(current_values.rbegin(),current_values.rend(),
		Variable_set_input_values(this));

	return (derivative);
}
#endif // defined (USE_ITERATORS)

#if defined (USE_ITERATORS)
#else // defined (USE_ITERATORS)
class Variable_get_input_value_functor
//******************************************************************************
// LAST MODIFIED : 31 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	public:
		Variable_get_input_value_functor(const Variable_handle& variable,
			std::list<Variable_handle>& variable_list):variable_list(variable_list),
			variable(variable){};
		~Variable_get_input_value_functor() {};
		int operator() (Variable_input_handle& input)
		{
			Variable_handle value;

			if (value=(variable->get_input_value)(input))
			{
				variable_list.push_back(value);
			}

			return (1);
		};
	private:
		std::list<Variable_handle>& variable_list;
		const Variable_handle& variable;
};
#endif // defined (USE_ITERATORS)

Variable_handle Variable::get_input_value(const
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	& input)
//******************************************************************************
// LAST MODIFIED : 2 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	Variable_handle result;
#if defined (USE_ITERATORS)
#else // defined (USE_ITERATORS)
	Variable_input_composite_handle input_composite;
#endif // defined (USE_ITERATORS)

	result=0;
#if defined (USE_ITERATORS)
	if (input)
	{
		std::list<Variable_handle> variable_list(0);
		Variable_handle value;
#if defined (USE_ITERATORS_NESTED)
		Variable_input::Iterator input_atomic_iterator;
#else // defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		Variable_input_iterator input_atomic_iterator(0);
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		Handle_iterator<
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		> input_atomic_iterator(0);
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // defined (USE_ITERATORS_NESTED)

		// not using std::for_each because then functor would have to be a friend of
		//   Variable
		for (input_atomic_iterator=input->
#if defined (USE_VARIABLE_INPUT)
			begin_atomic_inputs
#else // defined (USE_VARIABLE_INPUT)
			begin_atomic
#endif // defined (USE_VARIABLE_INPUT)
			();input_atomic_iterator!=input->
#if defined (USE_VARIABLE_INPUT)
			end_atomic_inputs
#else // defined (USE_VARIABLE_INPUT)
			end_atomic
#endif // defined (USE_VARIABLE_INPUT)
			();input_atomic_iterator++)
		{
			if (value=(get_input_value_local)(*input_atomic_iterator))
			{
				variable_list.push_back(value);
			}
		}
		result=Variable_handle(new Variable_composite(variable_list));
	}
#else // defined (USE_ITERATORS)
	if (input_composite=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_composite,Variable_input>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_composite *>(input);
#endif /* defined (USE_SMART_POINTER) */
		)
	{
#if defined (GENERALIZE_COMPOSITE_INPUT)
		result=input_composite->get_input_value(Variable_handle(this));
#else // defined (GENERALIZE_COMPOSITE_INPUT)
		std::list<Variable_handle> variable_list(0);

		std::for_each(input_composite->begin(),input_composite->end(),
			Variable_get_input_value_functor(Variable_handle(this),variable_list));
		result=Variable_handle(new Variable_composite(variable_list));
#endif // defined (GENERALIZE_COMPOSITE_INPUT)
	}
	else
	{
		result=(get_input_value_local)(input);
	}
#endif // defined (USE_ITERATORS)

	return (result);
}

bool Variable::set_input_value(const
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
	& value)
//******************************************************************************
// LAST MODIFIED : 4 February 2004
//
// DESCRIPTION :
//==============================================================================
{
#if defined (USE_ITERATORS)
	bool result;

	result=false;
	if (input)
	{
#if defined (USE_ITERATORS_NESTED)
		Variable::Iterator component_iterator;
		Variable_input::Iterator input_atomic_iterator;
#else // defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		Handle_iterator<
#if defined (USE_VARIABLES_AS_COMPONENTS)
			Variable_handle
#else // defined (USE_VARIABLES_AS_COMPONENTS)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
			> component_iterator(0);
		Handle_iterator<
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		> input_atomic_iterator(0);
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // defined (USE_ITERATORS_NESTED)

#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		//???DB.  Temporary
		result=set_input_value_local(input,value);
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		// not using std::for_each because then functor would have to be a friend of
		//   Variable
		//???DB.  No failure recovery?
		input_atomic_iterator=input->
#if defined (USE_VARIABLE_INPUT)
			begin_atomic_inputs
#else // defined (USE_VARIABLE_INPUT)
			begin_atomic
#endif // defined (USE_VARIABLE_INPUT)
			();
		component_iterator=value->begin_components();
		result=true;
		while (result&&(input_atomic_iterator!=input->
#if defined (USE_VARIABLE_INPUT)
			end_atomic_inputs
#else // defined (USE_VARIABLE_INPUT)
			end_atomic
#endif // defined (USE_VARIABLE_INPUT)
			())&&(component_iterator!=value->end_components()))
		{
			result=set_input_value_local(*input_atomic_iterator,*component_iterator);
			input_atomic_iterator++;
			component_iterator++;
		}
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
	}

	return (result);
#else // defined (USE_ITERATORS)
	//???DB.  Need to do composite input
	// return the result of the local set_input_value
	return (set_input_value_local(input,value));
#endif // defined (USE_ITERATORS)
}

Scalar Variable::norm() const
//******************************************************************************
// LAST MODIFIED : 4 December 2003
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

Variable_handle Variable::operator-(const Variable&) const
//******************************************************************************
// LAST MODIFIED : 6 December 2003
//
// DESCRIPTION :
// This is the default and returns a zero handle (failure).
//==============================================================================
{
	return (Variable_handle(0));
}

Variable_handle Variable::operator-=(const Variable&)
//******************************************************************************
// LAST MODIFIED : 6 December 2003
//
// DESCRIPTION :
// This is the default and returns a zero handle (failure).
//==============================================================================
{
	return (Variable_handle(0));
}

string_handle Variable::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 11 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;

	if (this)
	{
		return_string=(get_string_representation_local)();
	}
	else
	{
		return_string=new std::string;
	}

	return (return_string);
}
