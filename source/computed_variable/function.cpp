//******************************************************************************
// FILE : function.cpp
//
// LAST MODIFIED : 11 August 2004
//
// DESCRIPTION :
// See function.hpp
//
// NOTES :
// 1 Started as variable
//   - variable -> function
//   - variable_io_specifier -> function_variable (later change to variable, but
//     for now make different from existing)
// 2 From "Effective C++" by Scott Meyers
//   11 Define a copy constructor and an assignment operator for classes with
//     dynamically allocated memory
// 3 Pimpl - Pointer implementation.  http://c2.com/cgi-bin/wiki?PimplIdiom
// 4 To get correct linking with C functions, need
//     extern "C"
//     {
//     }
//   around header files
// 5 Started by using old dynamic memory allocation ie use new(nothrow), but
//   changed to new - program stops if throw an exception (without being caught)
// 6 Need -lstdc++ for standard library
// 7 Sort out constructors and destructors
//   Constructors and destructors are not inherited
// 8 Destructors need to be virtual so that if delete a pointer to a derived
//   object when the pointer is of type pointer to base class, the correct
//   destructor is called and delete is called with the correct size.
// 9 "Virtual" constructor.  Constructors cannot be virtual, but can get the
//   desired effect - see Stroustrup 3rd:424.
// 10 Constructors have to construct their members (using : member initializer
//   list).
// 11 The members' constructors are called before the body of the containing
//   class' own constructor is executed.  The constructors are called in the
//   order in which the members are declared in the class (rather than the order
//   in which the members appear in the initializer list).  If a member
//   constructor needs no arguments, the member need not be mentioned in the
//   member initializer list.  Stroustrup 3rd:247-8
// 12 The member destructors are called in the reverse order of construction
//   after the body of the class' own destructor has executed.
//   Stroustrup 3rd:247
// 13 Member initializers are essential for types for which initialization
//   differs from assignment.  Stroustrup 3rd:248
// 14 If the base clase doesn't have a constructor without arguments or another
//   constructor is required then the desired base class constructor should be
//   invoked in the initialization part of the derived class constructor.
//   Otherwise, if a base class constructor is not invoked the one with no
//   arguments will be implicitly invoked.
// 15 Be careful with smart pointers in constructors.  The following would cause
//   the objects destructor to be invoked because this starts with a reference
//   count of 0
//     f(Smart_pointer(this))
// 16 A reference cannot be made to refer to another object at run-time.  A
//   reference must be initialized so that it refers to an object.  If a data
//   member is a reference, it must be initialized in the constructors'
//   initializer list.
// 17 A local class cannot be used as a template argument.  Lischner 138
// 18 By default a single argument constructor also defines an implicit
//   conversion.  Implicit conversion from a particular constructor can be
//   suppressed by declaring the constructor implicit.  Stroustrup 3rd:284
// 19 Smart pointers 
// 19.1 Avoid bare pointers - Lischner 618)
// 19.1 Suggestions Alexandrescu 157-
// 20 Are using the ideas of "automatic differentiation" (technology for
//   automatically augmenting computer programs with statements for the
//   computation of derivatives), see
//     http://www-sop.inria.fr/tropics/ad/whatisad.html,
//   to get derivatives
// 21 Changed to intrusive smart pointer (boost/intrusive_ptr.hpp) because need
//   to be able to construct a smart pointer from any pointer (not just the new
//   one)
// 22 Need to sort out pointers and references
//   See Meyers 22,23
//   (Pass by) reference means that get the derived class rather than base class
//   Lischner 35 "A reference, unlike a pointer, cannot be made to refer to a
//     different object at runtime"
// 23 Functions seem to be midway between template and inheritence?
//   See Meyers 42
// 24 Bad returning dereferenced new because delete will never be called
//   See Meyers 31
// 25 Bad having methods that give access to data which is more private than the
//   method.  See Meyers ???DB
// 26 What about exceptions?
// 27 What happens when a constructor fails?
// 28 Overload << instead of get_string_representation?
// 29 Need a value type method for variables?
// 30 Would like to use const on methods more (means that <this> is not changed
//   (eg. virtual Function_handle clone() const=0), but prevented because of
//   instrusive smart pointer
// 31 For conversion to non-class type objects can have an operator of the
//   type name eg
//     operator Scalar() const
//   Lischner 118-9
//
// ???DB.  05Feb04
//   Need a value for each atomic specifier in set_input_value, even if the
//   specifier is empty, because set can change size eg. setting element/xi
//   - Could go back to a list of input/values, but there could still be an
//     element/xi hidden in a composite
//   - Could force components of composites to match - would mean need nested
//     composites
//   - Should sort itself out by resetting in reverse order?  What about
//     reverse order for atomic?
//   - Element/xi have to be set together (can't split into atomic) because
//     changing element can change the number of xi
//   - Avoid problem by not having input/value?  No, still want
//     current=get_value()
//     set_input_value(new)
//     set_input_value(current)
//     to leave you where you were
//   - Have get iterate forward, set iterate back and element last for
//     element/xi forward iteration?
//
// ???DB.  25Jun04
//   Need Function methods for all values which return the correct type so that
//   the set functions don't need to be friends
//   - make set functions friends?
//   - use Function_variable::get_value?
//==============================================================================

#include "computed_variable/function.hpp"

// class Function
// --------------

Function::Function():reference_count(0)
//******************************************************************************
// LAST MODIFIED : 11 February 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Function::Function(const Function&):reference_count(0)
//******************************************************************************
// LAST MODIFIED : 25 February 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{}

Function::~Function()
//******************************************************************************
// LAST MODIFIED : 11 February 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

void intrusive_ptr_add_ref(Function *function)
//******************************************************************************
// LAST MODIFIED : 11 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (function)
	{
		(function->reference_count)++;
	}
}

void intrusive_ptr_release(Function *function)
//******************************************************************************
// LAST MODIFIED : 11 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (function)
	{
		(function->reference_count)--;
		if (function->reference_count<=0)
		{
			delete function;
		}
	}
}
