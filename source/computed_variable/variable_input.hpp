//******************************************************************************
// FILE : variable_input.hpp
//
// LAST MODIFIED : 4 February 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__VARIABLE_INPUT_HPP__)
#define __VARIABLE_INPUT_HPP__

#include "computed_variable/variable_base.hpp"

#include <iterator>
#include <list>

#if defined (USE_ITERATORS)
#if !defined (USE_ITERATORS_NESTED)
#if !defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#include "computed_variable/variable_handle_iterator.hpp"
#endif // !defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // !defined (USE_ITERATORS_NESTED)
#endif // defined (USE_ITERATORS)

#if defined (USE_ITERATORS)
#if defined (OLD_CODE)
class Variable_input_atomic
//******************************************************************************
// LAST MODIFIED : 17 January 2004
//
// DESCRIPTION :
// A specification for an atomic input to a Variable.  Two atomic inputs can
// either be the same or completely disjoint/independent.
// within an input are independent.
//
// ???DB.  is_scalar (instead of size)?  What about value_type?
//==============================================================================
{
	public:
		//???DB.  Virtual methods can't be pure because then Iterator::operator*()
		//  can't return a Variable_input_atomic
		//???DB.  Should Iterator::operator*() return a
		//  Variable_input_atomic_handle?  No, because then operator->() then needs
		//  to return a Variable_input_atomic_handle*
		virtual bool operator==(const Variable_input_atomic&);
	protected:
		// constructor.  Protected so that can't create "plain" Variable_inputs
		Variable_input_atomic();
		// destructor.  Virtual for proper destruction of derived classes
		virtual ~Variable_input_atomic();
	private:
		// copy operations are private and undefined to prevent copying
		Variable_input_atomic(const Variable_input_atomic&);
		void operator=(const Variable_input_atomic&);
#if defined (USE_INTRUSIVE_SMART_POINTER)
//???DB.  Is smart pointer needed for this?
	private:
		int reference_count;
		friend void intrusive_ptr_add_ref(Variable_input_atomic *);
		friend void intrusive_ptr_release(Variable_input_atomic *);
#endif // defined (USE_INTRUSIVE_SMART_POINTER)
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_atomic>
	Variable_input_atomic_handle;
void intrusive_ptr_add_ref(Variable_input_atomic *);
void intrusive_ptr_release(Variable_input_atomic *);
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_atomic> Variable_input_atomic_handle;
#else
typedef Variable_input_atomic * Variable_input_atomic_handle;
#endif
#endif // defined (OLD_CODE)
#endif // defined (USE_ITERATORS)

class Variable_input;

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input> Variable_input_handle;
void intrusive_ptr_add_ref(Variable_input *);
void intrusive_ptr_release(Variable_input *);
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input> Variable_input_handle;
#else
typedef Variable_input * Variable_input_handle;
#endif

#if !defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
class Variable_input_iterator_representation
//******************************************************************************
// LAST MODIFIED : 26 January 2004
//
// DESCRIPTION :
// This is the class that iterators for derived Variable_inputs are derived
// from.
//==============================================================================
{
	friend class Variable_input_iterator;
	public:
		// destructor.  Virtual for proper destruction of derived classes
		virtual ~Variable_input_iterator_representation();
#if defined (OLD_CODE)
		// increment (prefix)
		virtual Variable_input_iterator_representation& operator++()=0;
		// increment (postfix)
		virtual Variable_input_iterator_representation operator++(int)=0;
		// equality
		virtual bool operator==(const Variable_input_iterator&)=0;
		// inequality
		virtual bool operator!=(const Variable_input_iterator&)=0;
		// dereference
		virtual Variable_input_handle operator*() const=0;
		// don't have a operator-> because its not needed and it would return
		//   a Variable_input_handle*
#endif // defined (OLD_CODE)
		// increment.  Do not want to return an object derived from
		//   Variable_input_iterator_representation
		virtual void increment()=0;
		// equality.  Used by Variable_input_iterator::operator== and !=
		virtual bool equality(const Variable_input_iterator_representation*)=0;
		// dereference
		virtual Variable_input_handle dereference() const=0;
	protected:
		// constructor.  Protected so that can't create "plain"
		//   Variable_input_iterator_representations
		Variable_input_iterator_representation();
	private:
		// copy operations are private and undefined to prevent copying
		Variable_input_iterator_representation(
			const Variable_input_iterator_representation&);
		virtual Variable_input_iterator_representation& operator=(
			const Variable_input_iterator_representation&);
	private:
		int reference_count;
};

class Variable_input_iterator:public std::iterator<std::input_iterator_tag,
	Variable_input_handle>
//******************************************************************************
// LAST MODIFIED : 23 January 2004
//
// DESCRIPTION :
// An iterator for Variable_inputs.
//
// Set up as a handle so that all inputs can have a begin_atomic_inputs and an
// end_atomic_inputs that return an iterator that is specialized for the derived
// input class.  Used instead of covariant returns because can't overload
// operators for a pointer.
//
// Didn't use Variable_input_handle because Variable_input_handle does not have
// ++ and because an iterator needs extra information such as the input that it
// is iterating over.
//
// ???DB.  Make into a template so that can reuse for Variables
//==============================================================================
{
	public:
		// constructor
		Variable_input_iterator(Variable_input_iterator_representation *);
		// copy constructor
		Variable_input_iterator(const Variable_input_iterator&);
		// assignment
		Variable_input_iterator& operator=(const Variable_input_iterator&);
		// destructor.  Virtual for proper destruction of derived classes
		~Variable_input_iterator();
		// increment (prefix)
		Variable_input_iterator& operator++();
		// increment (postfix)
		Variable_input_iterator operator++(int);
		// equality
		bool operator==(const Variable_input_iterator&);
		// inequality
		bool operator!=(const Variable_input_iterator&);
		// dereference
		Variable_input_handle operator*() const;
		// don't have a operator-> because its not needed and it would return
		//   a Variable_input_handle*
	private:
		Variable_input_iterator_representation *representation;
};
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // !defined (USE_ITERATORS_NESTED)

class Variable_input
//******************************************************************************
// LAST MODIFIED : 27 January 2004
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
#if defined (USE_ITERATORS)
		virtual Variable_input_handle clone() const=0;
#if defined (OLD_CODE)
		class Iterator:public std::iterator<std::input_iterator_tag,
			Variable_input_atomic,ptrdiff_t,Variable_input_atomic_handle>
			//???DB.  Virtual methods can't be pure because then begin_atomic_inputs
			//  and end_atomic_inputs can't return Iterators
		{
			public:
				// constructor
				Iterator();
				// destructor.  Virtual for proper destruction of derived classes
				virtual ~Iterator();
				// assignment
				virtual Iterator& operator=(const Iterator&);
				// increment (prefix)
				virtual Iterator operator++();
				// increment (postfix)
				virtual Iterator operator++(int);
				// equality
				virtual bool operator==(const Iterator&);
				// inequality
				virtual bool operator!=(const Iterator&);
				// dereference
				virtual Variable_input operator*();
				// access
				virtual Variable_input_handle operator->();
		};
		// for stepping through the atomic inputs that make up the Variable_input.
		//   Atomic inputs are indivisible so two atomic inputs are either the same
		//   or independent
		virtual Iterator begin_atomic_inputs()=0;
		virtual Iterator end_atomic_inputs()=0;
#endif // defined (OLD_CODE)
#if defined (USE_ITERATORS_NESTED)
		// need an iterator instead of a Variable_input_handle because
		//   Variable_input_handle does not have ++.  Also want a different ++ for
		//   each input type
		class Iterator:public std::iterator<std::input_iterator_tag,
			Variable_input_handle>
			//???DB.  Virtual methods can't be pure because then begin_atomic_inputs
			//  and end_atomic_inputs can't return Iterators
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
				virtual Variable_input_handle operator*() const;
				// don't have a operator-> because its not needed and it would return
				//   a Variable_input_handle*
		};
#endif // defined (USE_ITERATORS_NESTED)
		// atomic inputs are indivisible, that is, two atomic inputs are either the
		//   same or independent
		virtual bool is_atomic()=0;
		// for stepping through the atomic inputs that make up the Variable_input
#if defined (USE_ITERATORS_NESTED)
		virtual Iterator begin_atomic_inputs()=0;
		virtual Iterator end_atomic_inputs()=0;
#else // defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Variable_input_iterator begin_atomic_inputs()=0;
		virtual Variable_input_iterator end_atomic_inputs()=0;
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Handle_iterator<Variable_input_handle> begin_atomic_inputs()=0;
		virtual Handle_iterator<Variable_input_handle> end_atomic_inputs()=0;
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // defined (USE_ITERATORS_NESTED)
		// returns the number of atomic inputs that it makes sense to differentiate
		//   with respect to
		virtual Variable_size_type number_differentiable()=0;
#else // defined (USE_ITERATORS)
		// get the number of scalars specified
		virtual Variable_size_type size()=0;
#endif // defined (USE_ITERATORS)
#if defined (USE_SCALAR_MAPPING)
		// get the mapping between the scalars of <this> and the scalars of target.
		//   The mapping is represented as a series of pairs:
		//   - there is at least one pair
		//   - the first member of the first pair is 0
		//   - no two pairs have the same first member
		//   - the pairs are in increasing order of the first member
		//   - the first member is the starting index in <this>
		//   - the second member is the starting index in <target>.  If it is
		//     greater than or equal to target.size() then the index of <this> is
		//     not it <target>
		//   - the mapping is compressed by omitting pairs which have their first
		//     and second members the same increment more than the corresponding
		//     member in the previous pair eg.
		//     (i,j), (k,l) means (i,j),(i+1,j+1), ... (k-1,j+k-i-1),(k,l),
		//       (k+1,l+1), ... (size()-1,l+size()-k-1)
		//   - the last pair is (size(),target.size())
		//   Does the general work (composite <second>) and then calls
		//   scalar_mapping_local
			//???DB.  Use std::map instead ?  Not quite the same idea?
			//???DB.  Use std::vector instead of std::list?
		virtual std::list< std::pair<Variable_size_type,Variable_size_type> >
			scalar_mapping(Variable_input_handle target);
#endif // defined (USE_SCALAR_MAPPING)
#if defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS)
		// get the mapping between the scalars of <this> and the scalars of target.
		//   The mapping is represented as a series of pairs:
		//   - there is at least one pair
		//   - the first member of the first pair is 0
		//   - no two pairs have the same first member
		//   - the pairs are in increasing order of the first member
		//   - the first member is the starting index in <this>
		//   - the second member is the starting index in <target>.  If it is
		//     greater than or equal to target.size() then the index of <this> is
		//     not it <target>
		//   - the mapping is compressed by omitting pairs which have their first
		//     and second members the same increment more than the corresponding
		//     member in the previous pair eg.
		//     (i,j), (k,l) means (i,j),(i+1,j+1), ... (k-1,j+k-i-1),(k,l),
		//       (k+1,l+1), ... (size()-1,l+size()-k-1)
		//   Does the general work (composite <second>) and then calls
		//   scalar_mapping_local
			//???DB.  Use std::map instead ?  Not quite the same idea?
			//???DB.  Use std::vector instead of std::list?
		virtual std::list< std::pair<Variable_size_type,Variable_size_type> >
			scalar_mapping(const Variable_input& target) const;
#endif // defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS)
		// equality operator
		virtual bool operator==(const Variable_input&)=0;
#if defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS)
		// get the independent inputs that are in <this> or <second>.  The order of
		//   the inputs in <this> and <second> are maintained and all the <this>
		//   inputs are before all the <second> inputs.   Does the general work
		//   (composite <second>) and then calls operator_plus_local
		virtual Variable_input_handle operator+(const Variable_input& second);
		// get the independent inputs that are in <this> but not in <second>.  The
		//   order of the inputs in <this> is maintained.  Does the general work
		//   (composite <second>) and then calls operator_minus_local
		virtual Variable_input_handle operator-(const Variable_input& second);
		// get the independent inputs that are in <this> and in <second>.  The order
		//   of the inputs in <this> is maintained.  Does the general work
		//   (composite <second>) and then calls intersect_local
		virtual Variable_input_handle intersect(const Variable_input& second);
#endif // defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS)
	protected:
		// constructor.  Protected so that can't create "plain" Variable_inputs
		Variable_input();
		// destructor.  Virtual for proper destruction of derived classes
		virtual ~Variable_input();
#if defined (USE_SCALAR_MAPPING)
	private:
		// virtual method which has a default
		virtual std::list< std::pair<Variable_size_type,Variable_size_type> >
			scalar_mapping_local(Variable_input_handle target);
#endif // defined (USE_SCALAR_MAPPING)
#if defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS)
	private:
		// pure virtual methods which are specified for each sub-class
		virtual std::list< std::pair<Variable_size_type,Variable_size_type> >
			scalar_mapping_local(const Variable_input_handle& target)=0;
		// if operator_plus_local returns a zero handle then <*this> and <second>
		//   will be combined with Variable_input_composite.  operator_plus_local
		//   must handle the case where the intersection of <*this> and <second> is 
		//   not empty.  It should also handle the case where <*this> and <second>
		//   can combined without Variable_input_composite.
		virtual Variable_input_handle operator_plus_local(
			const Variable_input_handle& second)=0;
		virtual Variable_input_handle operator_minus_local(
			const Variable_input_handle& second)=0;
		virtual Variable_input_handle intersect_local(
			const Variable_input_handle& second)=0;
#endif // defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS)
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

#endif /* !defined (__VARIABLE_INPUT_HPP__) */
