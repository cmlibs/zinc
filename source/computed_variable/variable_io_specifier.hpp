//******************************************************************************
// FILE : variable_io_specifier.hpp
//
// LAST MODIFIED : 2 February 2004
//
// DESCRIPTION :
// An abstract class for specifying inputs/independents and outputs/dependents
// of a variable.
//
// ???DB.  Should input_specifier and output_specifier be derived from
//   io_specifier?
//==============================================================================
#if !defined (__VARIABLE_IO_SPECIFIER_HPP__)
#define __VARIABLE_IO_SPECIFIER_HPP__

#include "computed_variable/variable_base.hpp"

#include <iterator>

#if !defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#include "computed_variable/variable_handle_iterator.hpp"
#endif // !defined (DO_NOT_USE_ITERATOR_TEMPLATES)

//???DB.  Should forward declarations for Variable, Variable_handle,
//  Variable_io_specifier and Variable_io_specifier_handle be in variable_base?
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

class Variable_io_specifier;

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_io_specifier>
	Variable_io_specifier_handle;
void intrusive_ptr_add_ref(Variable_io_specifier *);
void intrusive_ptr_release(Variable_io_specifier *);
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_io_specifier> Variable_io_specifier_handle;
#else
typedef Variable_io_specifier * Variable_io_specifier_handle;
#endif

#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
class Variable_io_specifier_iterator_representation
//******************************************************************************
// LAST MODIFIED : 29 January 2004
//
// DESCRIPTION :
// This is the class that iterator representations for derived
// Variable_io_specifiers are derived from.
//==============================================================================
{
	friend class Variable_io_specifier_iterator;
	public:
		// destructor.  Virtual for proper destruction of derived classes
		virtual ~Variable_io_specifier_iterator_representation();
		// increment.  Do not want to return an object derived from
		//   Variable_io_specifier_iterator_representation
		virtual void increment()=0;
		// equality.  Used by Variable_io_specifier_iterator::operator== and !=
		virtual bool equality(
			const Variable_io_specifier_iterator_representation*)=0;
		// dereference
		virtual Variable_io_specifier_handle dereference() const=0;
	protected:
		// constructor.  Protected so that can't create "plain"
		//   Variable_io_specifier_iterator_representations
		Variable_io_specifier_iterator_representation();
	private:
		// copy operations are private and undefined to prevent copying
		Variable_io_specifier_iterator_representation(
			const Variable_io_specifier_iterator_representation&);
		virtual Variable_io_specifier_iterator_representation& operator=(
			const Variable_io_specifier_iterator_representation&);
	private:
		int reference_count;
};

class Variable_io_specifier_iterator:public std::iterator<
	std::input_iterator_tag,Variable_io_specifier_handle>
//******************************************************************************
// LAST MODIFIED : 29 January 2004
//
// DESCRIPTION :
// An iterator for Variable_io_specifiers.
//
// Set up as a handle so that all io_specifiers can have a begin_atomic and an
// end_atomic that return an iterator that is specialized for the derived
// io_specifier class.  Used instead of covariant returns because can't overload
// operators for a pointer.
//
// Didn't use Variable_io_specifier_handle as the iterator because smart
// pointers don't have ++ and because an iterator needs extra information such
// as the io_specifier that it is iterating over.
//==============================================================================
{
	public:
		// constructor
		Variable_io_specifier_iterator(
			Variable_io_specifier_iterator_representation *);
		// copy constructor
		Variable_io_specifier_iterator(const Variable_io_specifier_iterator&);
		// assignment
		Variable_io_specifier_iterator& operator=(
			const Variable_io_specifier_iterator&);
		// destructor.  Virtual for proper destruction of derived classes
		~Variable_io_specifier_iterator();
		// increment (prefix)
		Variable_io_specifier_iterator& operator++();
		// increment (postfix)
		Variable_io_specifier_iterator operator++(int);
		// equality
		bool operator==(const Variable_io_specifier_iterator&);
		// inequality
		bool operator!=(const Variable_io_specifier_iterator&);
		// dereference
		Variable_io_specifier_handle operator*() const;
		//???DB.  Don't have a operator-> because its not needed and it would return
		//   a Variable_io_specifier_handle* ?
	private:
		Variable_io_specifier_iterator_representation *representation;
};
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)

enum Variable_io_specifier_type
{
	INPUT_INDEPENDENT,
	MIXED_IO,
	OUTPUT_DEPENDENT
};

class Variable_io_specifier
//******************************************************************************
// LAST MODIFIED : 30 January 2004
//
// DESCRIPTION :
// A specification for an input/independent or an output/dependent of a
// Variable.
//==============================================================================
{
	public:
		virtual Variable_io_specifier_handle clone() const=0;
		// atomic io_specifiers are indivisible, that is, two atomic io_specifiers
		//   are either the same or disjoint
		virtual bool is_atomic()=0;
		// for stepping through the atomic io_specifiers that make up the
		//   Variable_io_specifier
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Variable_io_specifier_iterator begin_atomic()=0;
		virtual Variable_io_specifier_iterator end_atomic()=0;
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Handle_iterator<Variable_io_specifier_handle> begin_atomic()=0;
		virtual Handle_iterator<Variable_io_specifier_handle> end_atomic()=0;
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		// returns the number of atomic io_specifiers that it makes sense to
		//   differentiate (output/dependent) or differentiate with respect to
		//   (input/independent)
		virtual Variable_size_type number_differentiable()=0;
		// equality operator
		virtual bool operator==(const Variable_io_specifier&)=0;
	protected:
		// constructor.  Protected so that can't create "plain"
		//   Variable_io_specifiers
		Variable_io_specifier(const enum Variable_io_specifier_type type=MIXED_IO);
		// destructor.  Virtual for proper destruction of derived classes
		virtual ~Variable_io_specifier();
	private:
		// copy operations are private and undefined to prevent copying
		Variable_io_specifier(const Variable_io_specifier&);
		void operator=(const Variable_io_specifier&);
	private:
		enum Variable_io_specifier_type type;
#if defined (USE_INTRUSIVE_SMART_POINTER)
	private:
		int reference_count;
		friend void intrusive_ptr_add_ref(Variable_io_specifier *);
		friend void intrusive_ptr_release(Variable_io_specifier *);
#endif // defined (USE_INTRUSIVE_SMART_POINTER)
};

#endif /* !defined (__VARIABLE_IO_SPECIFIER_HPP__) */
