//******************************************************************************
// FILE : variable.hpp
//
// LAST MODIFIED : 11 December 2003
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
#if !defined (__VARIABLE_HPP__)
#define __VARIABLE_HPP__

#include <list>
#include <string>

//???DB.  Numerics need to go somewhere that can be shared with api
typedef double Scalar;

#include "boost/numeric/ublas/matrix.hpp"
#include "boost/numeric/ublas/vector.hpp"
#include "boost/numeric/ublas/io.hpp"

namespace ublas = boost::numeric::ublas;

// use column_major so that can use lapack=boost::numeric::bindings::lapack
typedef ublas::matrix<Scalar,ublas::column_major> Matrix;
typedef ublas::vector<Scalar> Vector;

#include "computed_variable/variable_input.hpp"

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
// LAST MODIFIED : 20 October 2003
//
// DESCRIPTION :
// An input/value pair.
//==============================================================================
{
	public:
		// constructor
		Variable_input_value(Variable_input_handle& input,Variable_handle& value);
		// copy constructor
		Variable_input_value(const Variable_input_value&);
		// assignment
		Variable_input_value& operator=(const Variable_input_value&);
		// destructor
		~Variable_input_value();
		// get input
		Variable_input_handle input() const;
		// get value
		Variable_handle value() const;
	private:
		Variable_input_handle input_data;
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

//???DB.  Replace with smart pointer or return reference?
typedef std::string * string_handle;

class Variable
//******************************************************************************
// LAST MODIFIED : 11 December 2003
//
// DESCRIPTION :
//???DB.  Almost all the public methods could be non-pure virtual so that have
//  a default, but can over-ride.
//???DB.  Add a size_input_value method so that input_values don't have to be
//  friends of their associated variables?
//==============================================================================
{
	// so that Variable_derivative_matrix constructor can call
	//   evaluate_derivative_local
	friend class Variable_derivative_matrix_create_matrices_inner_functor;
	friend class Variable_derivative_matrix_create_matrices_outer_functor;
	public:
		// destructor.  Virtual for proper destruction of derived classes
			//???DB.  Would like to be protected, but have some operations where need
			//  to create Variable_handles which means need destuctor for smart
			//  pointers
		virtual ~Variable();
		// get the number of scalars in the result
		virtual Variable_size_type size() const =0;
		// get the scalars in the result
		virtual Vector *scalars()=0;
		// evaluate method which does the general work and then calls the virtual
		//   private evaluate.  Made virtual so that operators can overload it.
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
		//   can overload it.
		virtual Variable_handle evaluate_derivative(
			std::list<Variable_input_handle>& independent_variables,
			std::list<Variable_input_value_handle>& values);
		// get input value method which does the general work and then calls the
		//   virtual private get_input_value
		Variable_handle get_input_value(const Variable_input_handle& input);
		// set input value method which does the general work and then calls the
		//   virtual private set_input_value
		int set_input_value(const Variable_input_handle& input,
			const Variable_handle& value);
		// get string representation method which does the general work and then
		//   calls the virtual private get_string_representation
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
		//   Fills in the matrix assuming that it has been zeroed
		virtual void evaluate_derivative_local(Matrix& matrix,
			std::list<Variable_input_handle>& independent_variables)=0;
		// virtual get input value method which is specified by each sub-class
		virtual Variable_handle get_input_value_local(
			const Variable_input_handle& input)=0;
		// virtual set input value method which is specified by each sub-class
		virtual int set_input_value_local(const Variable_input_handle& input,
			const Variable_handle& value)=0;
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
#endif /* !defined (__VARIABLE_HPP__) */
