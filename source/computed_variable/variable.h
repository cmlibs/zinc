//******************************************************************************
// FILE : variable.h
//
// LAST MODIFIED : 9 September 2003
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
//==============================================================================
#if !defined (__VARIABLE_H__)
#define __VARIABLE_H__

#include <list>
#include <string>
using namespace std;

class Variable_input
//******************************************************************************
// LAST MODIFIED : 8 September 2003
//
// DESCRIPTION :
// A specification for an input to a Variable.
//
// ???DB.  What about associated values (Variables)?  See Variable_input_value
// ???DB.  What about outputs?
// ???DB.  Could just be string?  Start as a class, with a string passed to the
//   constructor (could have other constructors for derived classes?), so that
//   can clarify ideas
// ???DB.  Overload operators like + to get appending etc?
// ???DB.  Be an array with flattening (like Perl)
// ???DB.  What about a hash?
// ???DB.  Can be a composite.  Associated with one value for evaluate or one
//   order for differentiation
//==============================================================================
{
	// can be used by any function
	public:
		// constructor
		Variable_input(string& specification_string);
		Variable_input(char *specification_string);
		// copy constructor
		Variable_input(const Variable_input&);
		// assignment
		Variable_input& operator=(const Variable_input&);
		// destructor
		~Variable_input();
		// get the number of reals specified
			// ???DB.  What if can't determine or has non-real(s)?
		int size();
	private:
		string specification_string;
}; // class Variable_input

// forward declaration
class Variable;

class Variable_input_value
//******************************************************************************
// LAST MODIFIED : 4 September 2003
//
// DESCRIPTION :
// An input/value pair.
//==============================================================================
{
	// can be used by any function
	public:
		// constructor
		Variable_input_value(Variable_input *input,Variable *value);
		// copy constructor
		Variable_input_value(const Variable_input_value&);
		// assignment
		Variable_input_value& operator=(const Variable_input_value&);
		// destructor
		~Variable_input_value();
		// get input
		Variable_input *input();
		// get value
		Variable *value();
	// can be used by member functions and friends of the class, and can be used
	//   by member functions and friends of a derived class
	private:
		Variable_input *input_private;
		Variable *value_private;
}; // class Variable_input_value

class Variable
//******************************************************************************
// LAST MODIFIED : 8 September 2003
//
// DESCRIPTION :
//==============================================================================
{
	// can be used by any function
	public:
		// constructor
		Variable(const string& name);
		Variable(const char *name);
		// copy constructor
		Variable(const Variable&);
		// assignment
		Variable& operator=(const Variable&);
		// destructor
		virtual ~Variable();
		// evaluate method which does the general work and then calls the virtual
		//   private evaluate.  const means that does not change <this> and so can
		//   be called for constants
			//???DB.  Get rid of comments about protected/private?  Not so worried
			//  about secrecy as about not having to re-compile users when change
			//  private/protected
			//???DB.  Change into a () operator ?
		Variable *evaluate(list<Variable_input_value> *values);
			//???DB.  Something wrong here.  Want the type returned by evaluate to be
			//  able to be different for a derived class and not necessarily the
			//  derived class eg finite element would return a vector of reals.
			//???DB.  OK because vector of reals is still a Variable?
			//???DB.  Means that we need run-time type checking (bad)?
		// get and set input value method which does the general work and then calls
		//   the virtual private get_set_input_value
		int get_set_input_value(Variable_input_value& input_value);
			//???DB.  Should return a Variable rather than an int?
		// get string representation method which does the general work and then
		//   calls the virtual private get_string_representation
		string *get_string_representation();
	// can be used by member functions and friends of the class
	private:
		// virtual evaluate method which is specified by each sub-class
		virtual Variable *evaluate_local()=0;
			//???DB.  Something wrong here.  Want the type returned by evaluate to
			//  be able to be different for a derived class and not necessarily the
			//  derived class eg finite element would return a vector of reals
			//???DB.  Change into a () operator ?
		// virtual get and set input value method which is specified by each
		//   sub-class
		virtual int get_set_input_value_local(Variable_input_value & input_value)=0;
		// virtual get string representation method which is specified by each
		//   sub-class
		virtual string *get_string_representation_local()=0;
		string name;
}; // class Variable
#endif /* !defined (__VARIABLE_H__) */
