//******************************************************************************
// FILE : variable_input.hpp
//
// LAST MODIFIED : 6 November 2003
//
// DESCRIPTION :
//==============================================================================
#if !defined (__VARIABLE_INPUT_HPP__)
#define __VARIABLE_INPUT_HPP__

#include <list>

//???DB.  Move into own include?
#define USE_SMART_POINTER
#define USE_INTRUSIVE_SMART_POINTER

#if defined (USE_INTRUSIVE_SMART_POINTER) && !defined (USE_SMART_POINTER)
#define USE_SMART_POINTER
#endif

#if defined (USE_INTRUSIVE_SMART_POINTER)
#include "boost/intrusive_ptr.hpp"
#elif defined (USE_SMART_POINTER)
#include "boost/shared_ptr.hpp"
#endif

//???DB.  Move into own include?
typedef unsigned int Variable_size_type;

class Variable_input
//******************************************************************************
// LAST MODIFIED : 6 November 2003
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
// ???DB.  Disable constructor (make protected) so that can only be got from
//   Variable?
// ???DB.  Only specialized within modules?
//==============================================================================
{
	public:
		// get the number of scalars specified
		virtual Variable_size_type size()=0;
		virtual bool operator==(const Variable_input&)=0;
	protected:
		// constructor.  Protected so that can't create "plain" Variable_inputs
		Variable_input();
		// destructor.  Virtual for proper destruction of derived classes
		virtual ~Variable_input();
	private:
		// copy operations are private and undefined to prevent copying
		Variable_input(const Variable_input&);
		void operator=(const Variable_input&);
#if defined (USE_INTRUSIVE_SMART_POINTER)
	private:
		int reference_count;
		friend void intrusive_ptr_add_ref(Variable_input *);
		friend void intrusive_ptr_release(Variable_input *);
#endif // defined (USE_INTRUSIVE_SMART_POINTER)
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input> Variable_input_handle;
void intrusive_ptr_add_ref(Variable_input *);
void intrusive_ptr_release(Variable_input *);
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input> Variable_input_handle;
#else
typedef Variable_input * Variable_input_handle;
#endif

#endif /* !defined (__VARIABLE_INPUT_HPP__) */
