//******************************************************************************
// FILE : variable.cpp
//
// LAST MODIFIED : 25 November 2003
//
// DESCRIPTION :
//
// NOTES :
// 1 To get correct linking with C functions, need
//     extern "C"
//     {
//     }
//   around header files
// 2 Started by using old dynamic memory allocation ie use new(nothrow), but
//   changed to new - program stops if throw an exception (without being caught)
// 3 Need -lstdc++ when start using standard library
// 4 I was trying too much by doing Pimpl at the same time as
//   Variable_inputs.  So moved Pimpl to *.pimpl and got rid of Pimpl for the
//   moment
// 5 Virtual destructors.  Needed if delete a pointer to a derived object when
//   the pointer is of type pointer to base class (so that correct destructor
//   is called and delete is called with the correct size).
// 6 "Virtual" constructor.  Constructors cannot be virtual, but can get the
//   desired effect - see Stroustrup 3rd:424.
// 7 Constructors have to construct their members (using : member initializer
//   list.
// 8 The members' constructors are called before the body of the containing
//   class' own constructor is executed.  The constructors are called in the
//   order in which the members are declared in the class (rather than the order
//   in which the members appear in the initializer list.  If a member
//   constructor needs no arguments, the member need not be mentioned in the
//   member initializer list.  Stroustrup 3rd:247-8
// 9 The member destructors are called in the reverse order of construction
//   after the body of the class' own destructor has executed.
//   Stroustrup 3rd:247
// 10 Member initializers are essential for types for which initialization
//   differs from assignment.  Stroustrup 3rd:248
// 11 A reference cannot be made to refer to another object at run-time.  A
//   reference must be initialized so that it refers to an object.  If a data
//   member is a reference, it must be initialized in the constructors'
//   initializer list.
// 12 A local class cannot be used as a template argument.  Lischner 138
// 13 By default a single argument constructor also defines an implicit
//   conversion.  Implicit conversion from a particular constructor can be
//   suppressed by declaring the constructor implicit.  Stroustrup 3rd:284
// 14 Smart pointers (avoid bare pointers - Lischner 618)
// 14.1 Suggestions Alexandrescu 157-
// 14.1.1 A smart pointer should not use member functions
// 14.2 Have added option (USE_SMART_POINTER) of boost::shared_ptr (can be used
//      with containers, std::autoptr cannot because it does a destructive
//      copy).  Tried Loki (Alexandrescu) but had lots of template compiler
//      errors which changed, but did not go away, as upgraded gcc
// 14.3 Having a smart pointer wrapper is not ideal because have to do things
//      differently in the api (if went to a Variable** for the bare pointer
//      case would then destructors wouldn't be called).  Could include smart
//      pointer into Variable (Pimpl), but lose code re-use
// 15 Are using the ideas of "automatic differentiation" (technology for
//    automatically augmenting computer programs with statements for the
//    computation of derivatives), see
//      http://www-sop.inria.fr/tropics/ad/whatisad.html,
//    to get derivatives
// 16 Tried templating the constructor for Variable_input so that don't need
//    string& and char* versions.  Problems
// 16.1 Can do if have template definition visible where the constructor is
//      instantiated, but this means having the definition in the .h file
// 16.2 Tried explicitly instantiating in .cpp eg
//        template Variable_input::Variable_input<char *>(char *);
//      but this doesn't compile because it doesn't have a return type.
//      However, a constructor doesn't have a return type (not even void).
//    Got round by just having string& and letting the string constructors
//    (called for pass by value) sort it out
// 17 Would like a general way of creating *_handle
// 17.1 Template handle<>.  Can't see how because want to rename a class, not
//      create a new one
// 17.2 Macro HANDLE() may have trouble with
//        #if defined (USE_SMART_POINTER)
//        #define HANDLE( object_type ) boost::shared_ptr<object_type>
//        #else // defined (USE_SMART_POINTER)
//        #define HANDLE( object_type ) object_type *
//        #endif // defined (USE_SMART_POINTER)
//
//        Variable_get_input_value get_input_value(HANDLE(Variable)(this));
//      when HANDLE is an ordinary pointer
// 18 Should only create a new _handle when create a new object to be pointed,
//   otherwise the object could be destroyed at the wrong time (eg when the
//   _handle that was created locally goes out of scope)
// 19 Changed to intrusive smart pointer because need to be able to construct
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
//==============================================================================

#include <algorithm>
#include <iterator>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>

//???DB.  Put in include?
const bool Assert_on=true;

template<class Assertion,class Exception>inline void Assert(
	Assertion assertion,Exception exception)
{
	if (Assert_on&&!(assertion)) throw exception;
}

#include "computed_variable/variable.hpp"
#include "computed_variable/variable_composite.hpp"
#include "computed_variable/variable_derivative_matrix.hpp"
#include "computed_variable/variable_input_composite.hpp"

// class Variable_input_value
// --------------------------

Variable_input_value::Variable_input_value(Variable_input_handle& input,
	Variable_handle& value):input_data(input),value_data(value)
#if defined (USE_INTRUSIVE_SMART_POINTER)
	,reference_count(0)
#endif
//******************************************************************************
// LAST MODIFIED : 16 October 2003
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

Variable_input_handle Variable_input_value::input() const
//******************************************************************************
// LAST MODIFIED : 5 October 2003
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

#if defined (OLD_CODE)
Variable::size_type Variable::size()
//******************************************************************************
// LAST MODIFIED : 23 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	//???DB.  Define for each derived type
	return (0);
}
#endif

class Variable_get_input_value
//******************************************************************************
// LAST MODIFIED : 16 October 2003
//
// DESCRIPTION :
// A unary function (Functor) for getting a list of input values for a variable.
//==============================================================================
{
	friend class Variable;
	public:
		Variable_get_input_value(Variable_handle variable,
			std::list<Variable_input_value_handle> &values):variable(variable),
			values(values){};
		~Variable_get_input_value() {};
		int operator() (Variable_input_value_handle& input_value)
		{
			Variable_handle value;
			Variable_input_handle input;

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

class Variable_set_input_value
//******************************************************************************
// LAST MODIFIED : 16 October 2003
//
// DESCRIPTION :
// A unary function (Functor) for setting a list of input values for a variable.
//==============================================================================
{
	public:
		Variable_set_input_value(Variable_handle variable):variable(variable){};
		~Variable_set_input_value() {};
		int operator() (Variable_input_value_handle& input_value)
		{
			return ((variable->set_input_value)(input_value->input(),
				input_value->value()));
		};
	private:
		Variable_handle variable;
};

Variable_handle Variable::evaluate(
	std::list<Variable_input_value_handle>& values)
//******************************************************************************
// LAST MODIFIED : 25 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Variable_input_value_handle> current_values;
	Variable_handle variable;

	// get the specified values
	std::for_each(values.begin(),values.end(),Variable_get_input_value(
		Variable_handle(this),current_values));
	// override the specified values
	std::for_each(values.begin(),values.end(),Variable_set_input_value(
		Variable_handle(this)));
	// do the local evaluate
	variable=(evaluate_local)();
	// reset the current values
	std::for_each(current_values.rbegin(),current_values.rend(),
		Variable_set_input_value(Variable_handle(this)));

	return (variable);
}

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

class Find_result_input_iterator
{
	public:
		Find_result_input_iterator(
			std::list<Variable_input_handle>::iterator& result_input_iterator,
			int& composite_independent_variable_factor):
			result_input_iterator(result_input_iterator),
			composite_independent_variable_factor(
			composite_independent_variable_factor){};
		~Find_result_input_iterator() {};
		int operator() (Variable_input_handle&)
		{
			composite_independent_variable_factor *= 2;
			result_input_iterator++;

			return (1);
		};
	private:
		std::list<Variable_input_handle>::iterator& result_input_iterator;
		int& composite_independent_variable_factor;
};

class Calculate_composite_independent_variable_step
{
	public:
		Calculate_composite_independent_variable_step(
			int& composite_independent_variable_step,int derivative_index):
			composite_independent_variable_step(
			composite_independent_variable_step),derivative_index(derivative_index)
		{
			composite_independent_variable_step=1;
		};
		~Calculate_composite_independent_variable_step() {};
		int operator() (Variable_input_handle& input)
		{
			if (derivative_index%2)
			{
				composite_independent_variable_step *= input->size();
			}
			derivative_index /= 2;

			return (1);
		};
	private:
		int& composite_independent_variable_step;
		int derivative_index;
};

class Variable_input_composite_evaluate_derivative_functor
//******************************************************************************
// LAST MODIFIED : 25 November 2003
//
// DESCRIPTION :
// A unary functor (Functor) for ???DB
//
// ???DB.  To be done
// - Merge derivative matrices as go?
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
			result(result),variable(variable) {};
		~Variable_input_composite_evaluate_derivative_functor() {};
		int operator() (Variable_input_handle& input)
		{
			int composite_independent_variable_factor,
				composite_independent_variable_step;
			std::list<Matrix>::iterator derivative_iterator,result_iterator;
			std::list<Variable_input_handle>::iterator result_input_iterator;
			unsigned i,j,matrix_number,number_of_matrices,number_of_rows;
			Variable_derivative_matrix_handle derivative;
			Variable_handle derivative_uncast;

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
				number_of_matrices=1;
				for (i=independent_variables.size();i>0;i--)
				{
					number_of_matrices *= 2;
				}
				number_of_matrices -= 1;
				Assert((number_of_matrices==(derivative->matrices).size())&&
					(number_of_matrices==(result->matrices).size()),std::logic_error(
					"Variable_input_composite_evaluate_derivative_functor().  "
					"Incorrect number_of_matrices"));
				composite_independent_variable_factor=1;
				result_input_iterator=result->independent_variables.begin();
				std::for_each(independent_variables.begin(),input_iterator,
					Find_result_input_iterator(result_input_iterator,
					composite_independent_variable_factor));
				derivative_iterator=(derivative->matrices).begin();
				result_iterator=(result->matrices).begin();
				number_of_rows=variable->size();
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

						std::for_each(independent_variables.begin(),input_iterator,
							Calculate_composite_independent_variable_step(
							composite_independent_variable_step,matrix_number));
						derivative_column_number=0;
						result_column_number=0;
						column_number=0;
						while (column_number<number_of_columns)
						{
							for (j=composite_independent_variable_step*
								((*result_input_iterator)->size());j>0;j--)
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
								((*input_iterator)->size());j>0;j--)
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
		std::list<Variable_input_value_handle>& values;
		Variable_derivative_matrix_handle& result;
		Variable_handle variable;
};

Variable_handle Variable::evaluate_derivative(
	std::list<Variable_input_handle>& independent_variables,
	std::list<Variable_input_value_handle>& values)
//******************************************************************************
// LAST MODIFIED : 25 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Variable_input_handle>::iterator input_composite_iterator,
		input_iterator;
	std::list<Variable_input_value_handle> current_values;
	Variable_derivative_matrix_handle derivative;
	Variable_handle derivative_uncast;
	Variable_input_composite_handle input_composite;
	Variable_input_is_composite is_composite_input_predicate;

	// check for composite inputs in the list of independent_variables
	input_iterator=std::find_if(independent_variables.begin(),
		independent_variables.end(),is_composite_input_predicate);
	if (independent_variables.end()==input_iterator)
	{
		// none of the independent variables/inputs are composite
		// get the specified values
		std::for_each(values.begin(),values.end(),Variable_get_input_value(
			Variable_handle(this),current_values));
		// override the specified values
		std::for_each(values.begin(),values.end(),Variable_set_input_value(
			Variable_handle(this)));
		// do the local evaluate derivative
		derivative=Variable_derivative_matrix_handle(new Variable_derivative_matrix(
			Variable_handle(this),independent_variables));
		// reset the current values
		std::for_each(current_values.rbegin(),current_values.rend(),
			Variable_set_input_value(this));
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
		// calculate the derivatives for the variables/inputs making up the
		//   composite
		input_composite_iterator=input_composite->begin();
		*input_iterator= *input_composite_iterator;
		//???DB.  Why can't the values be set up once and the derivatives
		//  evaluated locally?
		derivative_uncast=evaluate_derivative(independent_variables,values);
		derivative=
#if defined (USE_SMART_POINTER)
			boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>(
			derivative_uncast);
#else /* defined (USE_SMART_POINTER) */
			dynamic_cast<Variable_derivative_matrix *>(*derivative_uncast);
#endif /* defined (USE_SMART_POINTER) */
		input_composite_iterator++;
		Variable_input_composite_evaluate_derivative_functor
			evaluate_derivative_functor(Variable_handle(this),independent_variables,
			values,input_iterator,derivative);
		std::for_each(input_composite_iterator,input_composite->end(),
			evaluate_derivative_functor);
		*input_iterator=input_composite;
	}

	return (derivative);
}

class Variable_get_input_value_functor
//******************************************************************************
// LAST MODIFIED : 21 November 2003
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
			variable_list.push_back((variable->get_input_value)(input));

			return (1);
		};
	private:
		std::list<Variable_handle>& variable_list;
		const Variable_handle& variable;
};

Variable_handle Variable::get_input_value(const Variable_input_handle& input)
//******************************************************************************
// LAST MODIFIED : 25 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_handle result;
	Variable_input_composite_handle input_composite;

	result=0;
	if (input_composite=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_composite,Variable_input>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_composite *>(input);
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		std::list<Variable_handle> variable_list(0);

		std::for_each(input_composite->begin(),input_composite->end(),
			Variable_get_input_value_functor(Variable_handle(this),variable_list));
		result=Variable_handle(new Variable_composite(variable_list));
	}
	else
	{
		result=(get_input_value_local)(input);
	}

	return (result);
}

int Variable::set_input_value(const Variable_input_handle& input,
	const Variable_handle& value)
//******************************************************************************
// LAST MODIFIED : 18 September 2003
//
// DESCRIPTION :
// ???DB.  Need to do composite input
//==============================================================================
{
	// return the result of the local set_input_value
	return (set_input_value_local(input,value));
}

string_handle Variable::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 18 September 2003
//
// DESCRIPTION :
//==============================================================================
{
	// return the result of the local get_string_representation
	return ((get_string_representation_local)());
}
