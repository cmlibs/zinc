//******************************************************************************
// FILE : variable_handle_iterator.hpp
//
// LAST MODIFIED : 28 January 2004
//
// DESCRIPTION :
// Handle iterator templates.
//==============================================================================
#if !defined (__VARIABLE_HANDLE_ITERATOR_HPP__)
#define __VARIABLE_HANDLE_ITERATOR_HPP__

#include <iterator>

template<typename Handle> class Handle_iterator;

template<typename Handle> class Handle_iterator_representation
//******************************************************************************
// LAST MODIFIED : 28 January 2004
//
// DESCRIPTION :
// If B is the base class then for a derived class A, the iterator will be of
// type Handle_iterator<B> with a representation derived from
// Handle_iterator_representation<B>.
//==============================================================================
{
	friend class Handle_iterator<Handle>;
	public:
		// destructor.  Virtual for proper destruction of derived classes
		virtual ~Handle_iterator_representation();
		// increment.  Do not want to return an object derived from
		//   Handle_iterator_representation
		virtual void increment()=0;
		// equality.  Used by Handle_iterator::operator== and !=
		virtual bool equality(const Handle_iterator_representation*)=0;
		// dereference
		virtual Handle dereference() const=0;
	protected:
		// constructor.  Protected so that can't create "plain"
		//   Handle_iterator_representations
		Handle_iterator_representation();
	private:
		// copy operations are private and undefined to prevent copying
		Handle_iterator_representation(const Handle_iterator_representation&);
		Handle_iterator_representation& operator=(
			const Handle_iterator_representation&);
	private:
		int reference_count;
};

template<typename Handle> class Handle_iterator:
	public std::iterator<std::input_iterator_tag,Handle>
//******************************************************************************
// LAST MODIFIED : 27 January 2004
//
// DESCRIPTION :
// An iterator for Handles.
//
// Set up as a handle so that all inputs can have a begin_atomic_inputs and an
// end_atomic_inputs that return an iterator that is specialized for the derived
// input class.  Used instead of covariant returns because can't overload
// operators for a pointer.
//
// Didn't use Handle because Handle does not have ++ and because an iterator
// needs extra information such as the input that it is iterating over.
//==============================================================================
{
	public:
		// constructor
		Handle_iterator(Handle_iterator_representation<Handle> *);
		// copy constructor
		Handle_iterator(const Handle_iterator&);
		// assignment
		Handle_iterator& operator=(const Handle_iterator&);
		// destructor.  Virtual for proper destruction of derived classes
		~Handle_iterator();
		// increment (prefix)
		Handle_iterator& operator++();
		// increment (postfix)
		Handle_iterator operator++(int);
		// equality
		bool operator==(const Handle_iterator&);
		// inequality
		bool operator!=(const Handle_iterator&);
		// dereference
		Handle operator*() const;
		// don't have a operator-> because its not needed and it would return a
		//   Handle*
	private:
		Handle_iterator_representation<Handle> *representation;
};

// some compilers (g++ ?) need the template definitions in all source files
#include "computed_variable/variable_handle_iterator.cpp"

#endif /* !defined (__VARIABLE_HANDLE_ITERATOR_HPP__) */
