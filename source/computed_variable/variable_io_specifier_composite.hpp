//******************************************************************************
// FILE : variable_io_specifier_composite.hpp
//
// LAST MODIFIED : 30 January 2004
//
// DESCRIPTION :
// A list of specifiers joined together end on end.  There can be repeats in the
// list of atomic specifiers [begin_atomic(),end_atomic()).
//==============================================================================
#if !defined (__VARIABLE_IO_SPECIFIER_COMPOSITE_HPP__)
#define __VARIABLE_IO_SPECIFIER_COMPOSITE_HPP__

#include "computed_variable/variable_base.hpp"

#include <list>

#include "computed_variable/variable_io_specifier.hpp"

class Variable_io_specifier_composite : public Variable_io_specifier
//******************************************************************************
// LAST MODIFIED : 30 January 2004
//
// DESCRIPTION :
// A composite of other io_specifier(s).
//
// Composite io_specifiers are "flat" in the sense that the list of
// io_specifiers does not contain composite io_specifiers.  This means that the
// constructors have to flatten the list.
//???DB.  Not sure if this "flattening" is useful
//==============================================================================
{
	friend class Variable_io_specifier_iterator_representation_atomic_composite;
	public:
		// constructor
		Variable_io_specifier_composite(
			const Variable_io_specifier_handle& io_specifier_1,
			const Variable_io_specifier_handle& io_specifier_2);
		Variable_io_specifier_composite(
			std::list<Variable_io_specifier_handle>& io_specifiers_list);
		// copy constructor
		Variable_io_specifier_composite(const Variable_io_specifier_composite&);
		virtual Variable_io_specifier_handle clone() const;
		virtual bool is_atomic();
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Variable_io_specifier_iterator begin_atomic();
		virtual Variable_io_specifier_iterator end_atomic();
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Handle_iterator<Variable_io_specifier_handle> begin_atomic();
		virtual Handle_iterator<Variable_io_specifier_handle> end_atomic();
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Variable_size_type number_differentiable();
		// assignment
		Variable_io_specifier_composite& operator=(
			const Variable_io_specifier_composite&);
		// define inherited virtual methods
		virtual bool operator==(const Variable_io_specifier&);
	private:
		std::list<Variable_io_specifier_handle> io_specifiers_list;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_io_specifier_composite>
	Variable_io_specifier_composite_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_io_specifier_composite>
	Variable_io_specifier_composite_handle;
#else
typedef Variable_io_specifier_composite *
	Variable_io_specifier_composite_handle;
#endif

#endif /* !defined (__VARIABLE_IO_SPECIFIER_COMPOSITE_HPP__) */
