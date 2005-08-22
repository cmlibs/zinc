//******************************************************************************
// FILE : variable_finite_element.cpp
//
// LAST MODIFIED : 9 February 2004
//
// DESCRIPTION :
// Finite element types - element/xi and finite element field.
//==============================================================================
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "computed_variable/variable_base.hpp"

#include <new>
#include <sstream>
#include <string>
#include <stdio.h>

#include "computed_variable/variable_finite_element.hpp"
#include "computed_variable/variable_vector.hpp"
extern "C"
{
#include "finite_element/finite_element_region.h"
//???DB.  Get rid of debug.h (C->C++)
#include "general/debug.h"
}

// module classes
// ==============

class Variable_input_element_xi;

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_element_xi>
	Variable_input_element_xi_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_element_xi>
	Variable_input_element_xi_handle;
#else
typedef Variable_input_element_xi * Variable_input_element_xi_handle;
#endif

#if defined (USE_ITERATORS)
#if defined (USE_ITERATORS_NESTED)
#else // defined (USE_ITERATORS_NESTED)
// class Variable_input_iterator_representation_atomic_element_xi
// --------------------------------------------------------------

class Variable_input_iterator_representation_atomic_element_xi: public
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
	Variable_input_iterator_representation
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
	Handle_iterator_representation<
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		>
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Variable_input_iterator_representation_atomic_element_xi(const bool begin,
			Variable_input_element_xi_handle input);
		// destructor
		~Variable_input_iterator_representation_atomic_element_xi();
		// increment
		void increment();
		// equality
		bool equality(const
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
			Variable_input_iterator_representation
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
			Handle_iterator_representation<
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			>
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
			* representation);
		// dereference
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			dereference() const;
	private:
		Variable_input_element_xi_handle atomic_input,input;
		Variable_size_type input_index;
};
#endif // defined (USE_ITERATORS_NESTED)
#endif // defined (USE_ITERATORS)

// class Variable_input_element_xi
// -------------------------------

class Variable_input_element_xi : public
#if defined (USE_VARIABLE_INPUT)
	Variable_input
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Variable_element_xi;
	friend class Variable_finite_element;
	friend class Variable_finite_element_check_derivative_functor;
#if defined (USE_ITERATORS_NESTED)
#else // defined (USE_ITERATORS_NESTED)
	friend class Variable_input_iterator_representation_atomic_element_xi;
#endif // defined (USE_ITERATORS_NESTED)
	public:
		Variable_input_element_xi(
			const Variable_finite_element_handle& variable_finite_element,
			bool element=true,bool xi=true):element(element),xi(xi),indices(0),
			variable_element_xi(),variable_finite_element(variable_finite_element){};
		Variable_input_element_xi(
			const Variable_finite_element_handle& variable_finite_element,
			Variable_size_type index):element(false),xi(true),indices(1),
			variable_element_xi(),variable_finite_element(variable_finite_element)
		{
			indices[0]=index;
		};
		Variable_input_element_xi(
			const Variable_finite_element_handle& variable_finite_element,
			const ublas::vector<Variable_size_type>& indices):
			element(false),xi(true),indices(indices),variable_element_xi(),
			variable_finite_element(variable_finite_element)
		{
			// remove repeated indices
			Variable_size_type number_of_indices=indices.size();
			ublas::vector<Variable_size_type> unique_indices(number_of_indices);
			Variable_size_type i,j,number_of_unique_indices;

			number_of_unique_indices=0;
			for (i=0;i<number_of_indices;i++)
			{
				j=0;
				while ((j<number_of_unique_indices)&&(indices[i]!=unique_indices[j]))
				{
					j++;
				}
				if (j==number_of_unique_indices)
				{
					unique_indices[j]=indices[i];
					number_of_unique_indices++;
				}
			}
			if (number_of_indices!=number_of_unique_indices)
			{
				(this->indices).resize(number_of_unique_indices);
				for (i=0;i<number_of_unique_indices;i++)
				{
					(this->indices)[i]=unique_indices[i];
				}
			}
		};
		Variable_input_element_xi(
			const Variable_element_xi_handle& variable_element_xi,bool element=true,
			bool xi=true):element(element),xi(xi),indices(0),
			variable_element_xi(variable_element_xi),variable_finite_element(){};
		Variable_input_element_xi(
			const Variable_element_xi_handle& variable_element_xi,
			Variable_size_type index):element(false),xi(true),indices(1),
			variable_element_xi(variable_element_xi),variable_finite_element()
		{
			indices[0]=index;
		};
		Variable_input_element_xi(
			const Variable_element_xi_handle& variable_element_xi,
			const ublas::vector<Variable_size_type>& indices):
			element(false),xi(true),indices(indices),
			variable_element_xi(variable_element_xi),variable_finite_element()
		{
			// remove repeated indices
			Variable_size_type number_of_indices=indices.size();
			ublas::vector<Variable_size_type> unique_indices(number_of_indices);
			Variable_size_type i,j,number_of_unique_indices;

			number_of_unique_indices=0;
			for (i=0;i<number_of_indices;i++)
			{
				j=0;
				while ((j<number_of_unique_indices)&&(indices[i]!=unique_indices[j]))
				{
					j++;
				}
				if (j==number_of_unique_indices)
				{
					unique_indices[j]=indices[i];
					number_of_unique_indices++;
				}
			}
			if (number_of_indices!=number_of_unique_indices)
			{
				(this->indices).resize(number_of_unique_indices);
				for (i=0;i<number_of_unique_indices;i++)
				{
					(this->indices)[i]=unique_indices[i];
				}
			}
		};
		~Variable_input_element_xi(){};
#if defined (USE_ITERATORS)
		// copy constructor
		Variable_input_element_xi(
			const Variable_input_element_xi& input_element_xi):
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			(),
			element(input_element_xi.element),xi(input_element_xi.xi),
			indices(input_element_xi.indices),
			variable_element_xi(input_element_xi.variable_element_xi),
			variable_finite_element(input_element_xi.variable_finite_element){};
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		clone() const
		{
			return (Variable_input_element_xi_handle(
				new Variable_input_element_xi(*this)));
		};
#if defined (USE_ITERATORS_NESTED)
		class Iterator:public std::iterator<std::input_iterator_tag,
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			>
		{
			public:
				// constructor
				Iterator(Variable_input_element_xi_handle input):input(input),
					input_index(0)
				{
					if (atomic_input=
#if defined (USE_SMART_POINTER)
						boost::dynamic_pointer_cast<Variable_input_element_xi,
#if defined (USE_VARIABLE_INPUT)
						Variable_input
#else // defined (USE_VARIABLE_INPUT)
						Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
						>
#else /* defined (USE_SMART_POINTER) */
						dynamic_cast<Variable_input_element_xi *>
#endif /* defined (USE_SMART_POINTER) */
						(input->clone()))
					{
						atomic_input->indices.resize(1);
						atomic_input->indices[0]=0;
					}
				};
				// destructor
				~Iterator(){};
				// assignment
				Iterator& operator=(const Iterator& iterator)
				{
					atomic_input=iterator.atomic_input;
					input=iterator.input;
					input_index=iterator.input_index;

					return (*this);
				};
				// increment (prefix)
				Iterator& operator++()
				{
					if (atomic_input->element)
					{
						atomic_input->element=false;
						if (input->xi)
						{
							atomic_input->xi=true;
							input_index=0;
						}
						else
						{
							atomic_input->xi=false;
						}
					}
					else
					{
						if (atomic_input->xi)
						{
							input_index++;
						}
					}
					if (atomic_input->xi)
					{
						if (0<(input->indices).size())
						{
							if (input_index<(input->indices).size())
							{
								atomic_input->indices[0]=(input->indices)[input_index];
							}
							else
							{
								atomic_input->xi=false;
							}
						}
						else
						{
							if (input_index<input->number_differentiable())
							{
								atomic_input->indices[0]=input_index;
							}
							else
							{
								atomic_input->xi=false;
							}
						}
						if (!(atomic_input->xi))
						{
							input_index=0;
							atomic_input->indices[0]=0;
						}
					}

					return (*this);
				};
				// increment (postfix)
				Iterator operator++(int)
				{
					Iterator tmp= *this;

					++(*this);

					return (tmp);
				};
				// equality
				bool operator==(const Iterator& iterator)
				{
					bool result;

					result=false;
					if ((input==iterator.input)&&
						(*atomic_input== *(iterator.atomic_input)))
					{
						result=true;
					}

					return (result);
				};
				// inequality
				bool operator!=(const Iterator& iterator)
				{
					return (!((*this)==iterator));
				}
				// dereference
#if defined (USE_VARIABLE_INPUT)
				Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
				Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
				operator*() const
				{
					return (atomic_input);
				};
				// don't have a operator-> because its not needed and it would return
				//   a Variable_input_handle*
			private:
				Variable_input_element_xi_handle atomic_input,input;
				Variable_size_type input_index;
		};
#endif // defined (USE_ITERATORS_NESTED)
		virtual bool is_atomic()
		{
			return ((element&&!xi)||(!element&&xi&&((1==indices.size())||
				(variable_element_xi&&
				(1==variable_element_xi->number_differentiable()))||
				(variable_finite_element&&(1==(variable_finite_element->xi).size())))));
		}
#if defined (USE_ITERATORS_NESTED)
		virtual Iterator begin_atomic_inputs();
		virtual Iterator end_atomic_inputs();
#else // defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Variable_input_iterator begin_atomic_inputs()
		{
			return (Variable_input_iterator(
				new Variable_input_iterator_representation_atomic_element_xi(true,
				Variable_input_element_xi_handle(this))));
		};
		virtual Variable_input_iterator end_atomic_inputs()
		{
			return (Variable_input_iterator(
				new Variable_input_iterator_representation_atomic_element_xi(false,
				Variable_input_element_xi_handle(this))));
		};
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#if defined (USE_VARIABLE_INPUT)
		virtual Handle_iterator<Variable_input_handle> begin_atomic_inputs()
		{
			return (Handle_iterator<Variable_input_handle>(
				new Variable_input_iterator_representation_atomic_element_xi(true,
				Variable_input_element_xi_handle(this))));
		};
		virtual Handle_iterator<Variable_input_handle> end_atomic_inputs()
		{
			return (Handle_iterator<Variable_input_handle>(
				new Variable_input_iterator_representation_atomic_element_xi(false,
				Variable_input_element_xi_handle(this))));
		};
#else // defined (USE_VARIABLE_INPUT)
		virtual Handle_iterator<Variable_io_specifier_handle> begin_atomic()
		{
			return (Handle_iterator<Variable_io_specifier_handle>(
				new Variable_input_iterator_representation_atomic_element_xi(true,
				Variable_input_element_xi_handle(this))));
		};
		virtual Handle_iterator<Variable_io_specifier_handle> end_atomic()
		{
			return (Handle_iterator<Variable_io_specifier_handle>(
				new Variable_input_iterator_representation_atomic_element_xi(false,
				Variable_input_element_xi_handle(this))));
		};
#endif // defined (USE_VARIABLE_INPUT)
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // defined (USE_ITERATORS_NESTED)
#endif // defined (USE_ITERATORS)
		Variable_size_type
#if defined (USE_ITERATORS)
			number_differentiable
#else // defined (USE_ITERATORS)
			size
#endif // defined (USE_ITERATORS)
			()
		{
			Variable_size_type result;

			result=0;
			if (xi)
			{
				result=indices.size();
				if (0==result)
				{
					if (variable_element_xi)
					{
						result=variable_element_xi->
#if defined (USE_ITERATORS)
							number_differentiable
#else // defined (USE_ITERATORS)
							size
#endif // defined (USE_ITERATORS)
							();
					}
					else if (variable_finite_element)
					{
						result=(variable_finite_element->xi).size();
					}
				}
			}

			return (result);
		};
		virtual bool operator==(const
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			& input)
		{
			try
			{
				const Variable_input_element_xi& input_element_xi=
					dynamic_cast<const Variable_input_element_xi&>(input);
				bool result;

				result=false;
				if ((input_element_xi.variable_element_xi==variable_element_xi)&&
					(input_element_xi.variable_finite_element==variable_finite_element)&&
					(input_element_xi.element==element)&&(input_element_xi.xi==xi)&&
					(input_element_xi.indices.size()==indices.size()))
				{
					int i=indices.size();

					result=true;
					while (result&&(i>0))
					{
						i--;
						if (!(indices[i]==input_element_xi.indices[i]))
						{
							result=false;
						}
					}
				}

				return (result);
			}
			catch (std::bad_cast)
			{
				return (false);
			}
		};
#if defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS) || defined (USE_SCALAR_MAPPING)
	private:
		virtual std::list< std::pair<Variable_size_type,Variable_size_type> >
			scalar_mapping_local(Variable_input_handle target)
		{
			std::list< std::pair<Variable_size_type,Variable_size_type> > result(0);
			const Variable_input_element_xi_handle input_element_xi=
#if defined (USE_SMART_POINTER)
				boost::dynamic_pointer_cast<Variable_input_element_xi,Variable_input>
#else /* defined (USE_SMART_POINTER) */
				dynamic_cast<Variable_input_element_xi *>
#endif /* defined (USE_SMART_POINTER) */
				(target);
			Variable_size_type i,j,source_size,target_size;

			target_size=target->size();
			source_size=size();
			if (input_element_xi)
			{
				if ((variable_element_xi==input_element_xi->variable_element_xi)&&
					(variable_finite_element==input_element_xi->variable_finite_element))
				{
					if (xi)
					{
						if (0==indices.size())
						{
							if (input_element_xi->xi)
							{
								if (0==(input_element_xi->indices).size())
								{
									result.push_back(
										std::pair<Variable_size_type,Variable_size_type>(0,0));
								}
								else
								{
									Assert(target_size==(input_element_xi->indices).size(),
										std::logic_error(
										"Variable_input_element_xi::scalar_mapping_local.  "
										"Error in calculating target size 1"));
									for (i=0;i<source_size;i++)
									{
										j=0;
										while ((j<target_size)&&
											(i!=(input_element_xi->indices)[j]))
										{
											j++;
										}
										result.push_back(
											std::pair<Variable_size_type,Variable_size_type>(i,j));
									}
								}
							}
						}
						else
						{
							Assert(source_size==indices.size(),std::logic_error(
								"Variable_input_element_xi::scalar_mapping_local.  "
								"Error in calculating source size"));
							if (input_element_xi->xi)
							{
								if (0==(input_element_xi->indices).size())
								{
									for (i=0;i<source_size;i++)
									{
										result.push_back(
											std::pair<Variable_size_type,Variable_size_type>(i,
											indices[i]));
									}
								}
								else
								{
									Assert(target_size==(input_element_xi->indices).size(),
										std::logic_error(
										"Variable_input_element_xi::scalar_mapping_local.  "
										"Error in calculating target size 2"));
									for (i=0;i<source_size;i++)
									{
										j=0;
										while ((j<target_size)&&
											(indices[i]!=(input_element_xi->indices)[j]))
										{
											j++;
										}
										result.push_back(
											std::pair<Variable_size_type,Variable_size_type>(
											indices[i],j));
									}
								}
							}
						}
					}
				}
			}
			if (0==result.size())
			{
				result.push_back(std::pair<Variable_size_type,Variable_size_type>(
					0,target_size));
			}
#if defined (USE_SCALAR_MAPPING)
			if (0<source_size)
			{
				result.push_back(std::pair<Variable_size_type,Variable_size_type>(
					source_size,target_size));
			}
#endif // defined (USE_SCALAR_MAPPING)

			return (result);
		};
#endif // defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS) || defined (USE_SCALAR_MAPPING)
#if defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS)
		virtual Variable_input_handle operator_plus_local(
			const Variable_input_handle& second)
		{
			Variable_input_handle result(0);
			const Variable_input_element_xi_handle input_element_xi=
#if defined (USE_SMART_POINTER)
				boost::dynamic_pointer_cast<Variable_input_element_xi,Variable_input>
#else /* defined (USE_SMART_POINTER) */
				dynamic_cast<Variable_input_element_xi *>
#endif /* defined (USE_SMART_POINTER) */
				(second);

			if (input_element_xi)
			{
				if ((variable_element_xi==input_element_xi->variable_element_xi)&&
					(variable_finite_element==input_element_xi->variable_finite_element))
				{
					if (xi||(input_element_xi->xi))
					{
						if ((xi&&(0==indices.size()))||
							((input_element_xi->xi)&&(0==(input_element_xi->indices).size())))
						{
							if (variable_element_xi)
							{
								result=Variable_input_handle(new Variable_input_element_xi(
									variable_element_xi,element||input_element_xi->element,true));
							}
							if (variable_finite_element)
							{
								result=Variable_input_handle(new Variable_input_element_xi(
									variable_finite_element,element||input_element_xi->element,
									true));
							}
						}
						else
						{
							if (xi&&(0!=indices.size()))
							{
								if ((input_element_xi->xi)&&
									(0!=(input_element_xi->indices).size()))
								{
									std::list<Variable_size_type> indices_plus_list(0);
									Variable_size_type i,j;

									i=0;
									for (j=indices.size();j>0;j--)
									{
										indices_plus_list.push_back(indices[i]);
										i++;
									}
									i=0;
									for (j=(input_element_xi->indices).size();j>0;j--)
									{
										if (indices_plus_list.end()==std::find(
											indices_plus_list.begin(),indices_plus_list.end(),
											(input_element_xi->indices)[i]))
										{
											indices_plus_list.push_back(
												(input_element_xi->indices)[i]);
										}
										i++;
									}

									ublas::vector<Variable_size_type>
										indices_plus(indices_plus_list.size());
									std::list<Variable_size_type>::iterator indices_iterator;

									indices_iterator=indices_plus_list.begin();
									i=0;
									for (j=indices_plus_list.size();j>0;j--)
									{
										indices_plus[i]= *indices_iterator;
										i++;
									}
									if (variable_element_xi)
									{
										result=Variable_input_handle(new Variable_input_element_xi(
											variable_element_xi,indices_plus));
									}
									if (variable_finite_element)
									{
										result=Variable_input_handle(new Variable_input_element_xi(
											variable_finite_element,indices_plus));
									}
								}
								else
								{
									if (variable_element_xi)
									{
										result=Variable_input_handle(new Variable_input_element_xi(
											variable_element_xi,indices));
									}
									if (variable_finite_element)
									{
										result=Variable_input_handle(new Variable_input_element_xi(
											variable_finite_element,indices));
									}
								}
							}
							else
							{
								if (variable_element_xi)
								{
									result=Variable_input_handle(new Variable_input_element_xi(
										variable_element_xi,input_element_xi->indices));
								}
								if (variable_finite_element)
								{
									result=Variable_input_handle(new Variable_input_element_xi(
										variable_finite_element,input_element_xi->indices));
								}
							}
						}
					}
					else
					{
						if (variable_element_xi)
						{
							result=Variable_input_handle(new Variable_input_element_xi(
								variable_element_xi,element||input_element_xi->element,false));
						}
						if (variable_finite_element)
						{
							result=Variable_input_handle(new Variable_input_element_xi(
								variable_finite_element,element||input_element_xi->element,
								false));
						}
					}
				}
			}

			return (result);
		};
		virtual Variable_input_handle operator_minus_local(
			const Variable_input_handle& second)
		{
			Variable_input_handle result(0);
			const Variable_input_element_xi_handle input_element_xi=
#if defined (USE_SMART_POINTER)
				boost::dynamic_pointer_cast<Variable_input_element_xi,Variable_input>
#else /* defined (USE_SMART_POINTER) */
				dynamic_cast<Variable_input_element_xi *>
#endif /* defined (USE_SMART_POINTER) */
				(second);

			if (input_element_xi&&
				((variable_element_xi==input_element_xi->variable_element_xi)&&
				(variable_finite_element==input_element_xi->variable_finite_element)))
			{
				if (xi&&(input_element_xi->xi))
				{
					if (0==(input_element_xi->indices).size())
					{
						if (variable_element_xi)
						{
							result=Variable_input_handle(new Variable_input_element_xi(
								variable_element_xi,element&&!(input_element_xi->element),
								false));
						}
						if (variable_finite_element)
						{
							result=Variable_input_handle(new Variable_input_element_xi(
								variable_finite_element,element&&!(input_element_xi->element),
								false));
						}
					}
					else
					{
						std::list<Variable_size_type> indices_minus_list(0);
						Variable_size_type i,j;

						if (0==indices.size())
						{
							j=0;
							if (variable_element_xi)
							{
								j=variable_element_xi->size();
							}
							if (variable_finite_element)
							{
								j=(variable_finite_element->xi).size();
							}
							i=0;
							for (;j>0;j--)
							{
								indices_minus_list.push_back(i);
								i++;
							}
						}
						else
						{
							i++;
							for (j=indices.size();j>0;j--)
							{
								indices_minus_list.push_back(indices[i]);
								i++;
							}
						}
						i=0;
						for (j=(input_element_xi->indices).size();j>0;j--)
						{
							indices_minus_list.remove((input_element_xi->indices)[i]);
							i++;
						}
						if (0==indices_minus_list.size())
						{
							if (variable_element_xi)
							{
								result=Variable_input_handle(new Variable_input_element_xi(
									variable_element_xi,element&&!(input_element_xi->element),
									false));
							}
							if (variable_finite_element)
							{
								result=Variable_input_handle(new Variable_input_element_xi(
									variable_finite_element,element&&!(input_element_xi->element),
									false));
							}
						}
						else
						{
							ublas::vector<Variable_size_type>
								indices_minus(indices_minus_list.size());
							std::list<Variable_size_type>::iterator indices_iterator;

							indices_iterator=indices_minus_list.begin();
							i=0;
							for (j=indices_minus_list.size();j>0;j--)
							{
								indices_minus[i]= *indices_iterator;
								i++;
							}
							if (variable_element_xi)
							{
								result=Variable_input_handle(new Variable_input_element_xi(
									variable_element_xi,indices_minus));
							}
							if (variable_finite_element)
							{
								result=Variable_input_handle(new Variable_input_element_xi(
									variable_finite_element,indices_minus));
							}
						}
					}
				}
			}
			else
			{
				if (0==result)
				{
					result=Variable_input_handle(this);
				}
			}

			return (result);
		};
		virtual Variable_input_handle intersect_local(
			const Variable_input_handle& second)
		{
			Variable_input_handle result(0);
			const Variable_input_element_xi_handle input_element_xi=
#if defined (USE_SMART_POINTER)
				boost::dynamic_pointer_cast<Variable_input_element_xi,Variable_input>
#else /* defined (USE_SMART_POINTER) */
				dynamic_cast<Variable_input_element_xi *>
#endif /* defined (USE_SMART_POINTER) */
				(second);

			if (input_element_xi&&
				((variable_element_xi==input_element_xi->variable_element_xi)&&
				(variable_finite_element==input_element_xi->variable_finite_element)))
			{
				if (xi&&(input_element_xi->xi))
				{
					if (0==indices.size())
					{
						if (0==(input_element_xi->indices).size())
						{
							if (variable_element_xi)
							{
								result=Variable_input_handle(new Variable_input_element_xi(
									variable_element_xi,element&&(input_element_xi->element),
									true));
							}
							if (variable_finite_element)
							{
								result=Variable_input_handle(new Variable_input_element_xi(
									variable_finite_element,element&&(input_element_xi->element),
									true));
							}
						}
						else
						{
							if (variable_element_xi)
							{
								result=Variable_input_handle(new Variable_input_element_xi(
									variable_element_xi,input_element_xi->indices));
							}
							if (variable_finite_element)
							{
								result=Variable_input_handle(new Variable_input_element_xi(
									variable_finite_element,input_element_xi->indices));
							}
						}
					}
					else
					{
						if (0==(input_element_xi->indices).size())
						{
							if (variable_element_xi)
							{
								result=Variable_input_handle(new Variable_input_element_xi(
									variable_element_xi,indices));
							}
							if (variable_finite_element)
							{
								result=Variable_input_handle(new Variable_input_element_xi(
									variable_finite_element,indices));
							}
						}
						else
						{
							std::list<Variable_size_type> indices_intersect_list(0);
							Variable_size_type i,j;

							i=0;
							for (j=indices.size();j>0;j--)
							{
								if ((input_element_xi->indices).end()!=std::find(
									(input_element_xi->indices).begin(),
									(input_element_xi->indices).end(),indices[i]))
								{
									indices_intersect_list.push_back(indices[i]);
								}
								i++;
							}
							if (0==indices_intersect_list.size())
							{
								result=Variable_input_handle(0);
							}
							else
							{
								ublas::vector<Variable_size_type>
									indices_intersect(indices_intersect_list.size());
								std::list<Variable_size_type>::iterator indices_iterator;

								indices_iterator=indices_intersect_list.begin();
								i=0;
								for (j=indices_intersect_list.size();j>0;j--)
								{
									indices_intersect[i]= *indices_iterator;
									i++;
								}
								if (variable_element_xi)
								{
									result=Variable_input_handle(new Variable_input_element_xi(
										variable_element_xi,indices_intersect));
								}
								if (variable_finite_element)
								{
									result=Variable_input_handle(new Variable_input_element_xi(
										variable_finite_element,indices_intersect));
								}
							}
						}
					}
				}
				else
				{
					if (element&&(input_element_xi->element))
					{
						if (variable_element_xi)
						{
							result=Variable_input_handle(new Variable_input_element_xi(
								variable_element_xi,true,false));
						}
						if (variable_finite_element)
						{
							result=Variable_input_handle(new Variable_input_element_xi(
								variable_finite_element,true,false));
						}
					}
				}
			}

			return (result);
		};
#endif // defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS)
	private:
		bool element,xi;
		ublas::vector<Variable_size_type> indices;
		Variable_element_xi_handle variable_element_xi;
		Variable_finite_element_handle variable_finite_element;
};

#if defined (USE_ITERATORS)
#if defined (USE_ITERATORS_NESTED)
#else // defined (USE_ITERATORS_NESTED)
// class Variable_input_iterator_representation_atomic_element_xi
// --------------------------------------------------------------

Variable_input_iterator_representation_atomic_element_xi::
	Variable_input_iterator_representation_atomic_element_xi(const bool begin,
	Variable_input_element_xi_handle input):atomic_input(0),input(input),
	input_index(0)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Constructor.  If <begin> then the constructed iterator points to the first
// atomic input, otherwise it points to one past the last atomic input.
//==============================================================================
{
	if (begin&&input)
	{
		if (atomic_input=
#if defined (USE_SMART_POINTER)
			boost::dynamic_pointer_cast<Variable_input_element_xi,
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			>
#else /* defined (USE_SMART_POINTER) */
			dynamic_cast<Variable_input_element_xi *>
#endif /* defined (USE_SMART_POINTER) */
			(input->clone()))
		{
			atomic_input->indices.resize(1);
			atomic_input->indices[0]=0;
			if (input->element)
			{
				atomic_input->xi=false;
			}
			else
			{
				if ((input->xi)&&(0<input->number_differentiable()))
				{
					if (0<(input->indices).size())
					{
						atomic_input->indices[0]=input->indices[0];
					}
				}
				else
				{
					atomic_input=0;
				}
			}
		}
	}
}

Variable_input_iterator_representation_atomic_element_xi::
	~Variable_input_iterator_representation_atomic_element_xi()
//******************************************************************************
// LAST MODIFIED : 26 January 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
}

void Variable_input_iterator_representation_atomic_element_xi::increment()
//******************************************************************************
// LAST MODIFIED : 26 January 2004
//
// DESCRIPTION :
// Increments the iterator to the next atomic input.  NULL <atomic_input> is the
// end iterator.
//==============================================================================
{
	if (atomic_input)
	{
		if (atomic_input->element)
		{
			atomic_input->element=false;
			if (input->xi)
			{
				atomic_input->xi=true;
				input_index=0;
			}
			else
			{
				// end
				atomic_input=0;
			}
		}
		else
		{
			Assert(atomic_input->xi,std::logic_error(
				"Variable_input_iterator_representation_atomic_element_xi::"
				"increment.  Atomic input should not have element and xi both "
				"false"));
			input_index++;
		}
		if (atomic_input)
		{
			Assert(atomic_input->xi,std::logic_error(
				"Variable_input_iterator_representation_atomic_element_xi::"
				"increment.  Second and subsequent atomic inputs should be xi"));
			if (input_index<input->number_differentiable())
			{
				if (input_index<(input->indices).size())
				{
					atomic_input->indices[0]=(input->indices)[input_index];
				}
				else
				{
					atomic_input->indices[0]=input_index;
				}
			}
			else
			{
				// end
				atomic_input=0;
			}
		}
	}
}

bool Variable_input_iterator_representation_atomic_element_xi::equality(
	const
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
	Variable_input_iterator_representation
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
	Handle_iterator_representation<
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	>
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
	* representation)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Tests <*this> and <*representation> for equality.
//==============================================================================
{
	const Variable_input_iterator_representation_atomic_element_xi
		*representation_element_xi=dynamic_cast<
		const Variable_input_iterator_representation_atomic_element_xi *>(
		representation);

	return (representation_element_xi&&
		(input==representation_element_xi->input)&&
		((atomic_input&&(representation_element_xi->atomic_input)&&
		(*atomic_input== *(representation_element_xi->atomic_input)))||
		(!atomic_input&&!(representation_element_xi->atomic_input))));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_input_iterator_representation_atomic_element_xi::dereference() const
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the atomic input for the iterator.
//==============================================================================
{
	return (atomic_input);
}
#endif // defined (USE_ITERATORS_NESTED)
#endif // defined (USE_ITERATORS)

struct Count_nodal_values_data
{
	enum FE_nodal_value_type value_type;
	int component_number,number_of_components,number_of_values,version;
	struct FE_field *fe_field;
	int *node_offsets,number_of_node_offsets,*number_of_node_values;
	struct FE_node **offset_nodes;
}; /* struct Count_nodal_values_data */

static int component_count_nodal_values(struct FE_node *node,
	struct FE_field *fe_field,int component_number,
	enum FE_nodal_value_type value_type,int version)
/*******************************************************************************
LAST MODIFIED : 29 December 2003

DESCRIPTION :
==============================================================================*/
{
	enum FE_nodal_value_type *nodal_value_type,*nodal_value_types;
	int i,number_of_versions,number_of_values;

	ENTER(component_count_nodal_values);
	number_of_versions=get_FE_node_field_component_number_of_versions(node,
		fe_field,component_number);
	if (version<number_of_versions)
	{
		number_of_values=1+get_FE_node_field_component_number_of_derivatives(node,
			fe_field,component_number);
		if (FE_NODAL_UNKNOWN!=value_type)
		{
			/*???DB.  Could use FE_nodal_value_version_exists instead */
			i=number_of_values;
			number_of_values=0;
			if (nodal_value_types=get_FE_node_field_component_nodal_value_types(node,
				fe_field,component_number))
			{
				nodal_value_type=nodal_value_types;
				while ((i>0)&&(value_type!= *nodal_value_type))
				{
					nodal_value_type++;
					i--;
				}
				if (i>0)
				{
					number_of_values++;
				}
				DEALLOCATE(nodal_value_types);
			}
		}
		if (version<0)
		{
			number_of_values *= number_of_versions;
		}
	}
	else
	{
		number_of_values=0;
	}
	LEAVE;

	return (number_of_values);
} /* component_count_nodal_values */

static int count_nodal_values(struct FE_node *node,
	void *count_nodal_values_data_void)
/*******************************************************************************
LAST MODIFIED : 29 April 2003

DESCRIPTION :
Counts the number of nodal values specified by <count_nodal_values_data_void> at
the <node>.
==============================================================================*/
{
	int component_number,node_number,number_of_values,return_code;
	struct Count_nodal_values_data *count_nodal_values_data;
	struct FE_node **offset_nodes;

	ENTER(count_nodal_values);
	return_code=0;
	/* check arguments */
	if (node&&(count_nodal_values_data=(struct Count_nodal_values_data *)
		count_nodal_values_data_void))
	{
		if (offset_nodes=count_nodal_values_data->offset_nodes)
		{
			node_number=0;
			while ((node_number<count_nodal_values_data->number_of_node_offsets)&&
				(node!=offset_nodes[node_number]))
			{
				node_number++;
			}
			if (node_number<count_nodal_values_data->number_of_node_offsets)
			{
				(count_nodal_values_data->node_offsets)[node_number]=
					count_nodal_values_data->number_of_values;
			}
		}
		if ((0<=(component_number=count_nodal_values_data->component_number))&&
			(component_number<count_nodal_values_data->number_of_components))
		{
			number_of_values=component_count_nodal_values(node,
				count_nodal_values_data->fe_field,component_number,
				count_nodal_values_data->value_type,
				count_nodal_values_data->version);
		}
		else
		{
			number_of_values=0;
			for (component_number=0;component_number<
				count_nodal_values_data->number_of_components;component_number++)
			{
				number_of_values += component_count_nodal_values(node,
					count_nodal_values_data->fe_field,component_number,
					count_nodal_values_data->value_type,
					count_nodal_values_data->version);
			}
		}
		if ((offset_nodes=count_nodal_values_data->offset_nodes)&&
			(node_number<count_nodal_values_data->number_of_node_offsets))
		{
			(count_nodal_values_data->number_of_node_values)[node_number]=
				number_of_values;
		}
		count_nodal_values_data->number_of_values += number_of_values;
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* count_nodal_values */

#if defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS) || defined (USE_SCALAR_MAPPING)
struct Scalar_mapping_nodal_values_data
{
	Scalar_mapping_nodal_values_data(struct FE_node *source_node,
		struct FE_node *target_node,enum FE_nodal_value_type source_value_type,
		enum FE_nodal_value_type target_value_type,int source_version,
		int target_version,int source_component_number,int target_component_number,
		struct FE_field *fe_field,
		std::list< std::pair<Variable_size_type,Variable_size_type> >&
		scalar_mapping):source_value_type(source_value_type),
		target_value_type(target_value_type),
		source_component_number(source_component_number),
		target_component_number(target_component_number),
		source_version(source_version),target_version(target_version),
		number_of_components(get_FE_field_number_of_components(fe_field)),
		fe_field(fe_field),source_node(source_node),target_node(target_node),
		source_index(0),target_index(0),scalar_mapping(scalar_mapping) {};
	enum FE_nodal_value_type source_value_type,target_value_type;
	int source_component_number,target_component_number;
	int source_version,target_version;
	int number_of_components;
	struct FE_field *fe_field;
	struct FE_node *source_node,*target_node;
	Variable_size_type source_index,target_index;
	std::list< std::pair<Variable_size_type,Variable_size_type> >& scalar_mapping;
}; /* struct Scalar_mapping_nodal_values_data */

static int component_scalar_mapping_nodal_values(struct FE_node *node,
	int component_number,struct Scalar_mapping_nodal_values_data
	*scalar_mapping_nodal_values_data)
/*******************************************************************************
LAST MODIFIED : 29 December 2003

DESCRIPTION :
==============================================================================*/
{
	enum FE_nodal_value_type *nodal_value_type,*nodal_value_types,value_type;
	int number_of_versions,number_of_values,return_code,source_value_index,
		source_version,target_value_index,target_version,version;
	struct FE_field *fe_field;
	std::list< std::pair<Variable_size_type,Variable_size_type> >
		&scalar_mapping=scalar_mapping_nodal_values_data->scalar_mapping;
	std::list< std::pair<Variable_size_type,Variable_size_type> >::iterator
		scalar_mapping_iterator;
	Variable_size_type i,number_of_source_values,number_of_target_values,
		source_index,target_index;

	ENTER(component_scalar_mapping_nodal_values);
	return_code=0;
	fe_field=scalar_mapping_nodal_values_data->fe_field;
	if ((FE_NODAL_UNKNOWN==scalar_mapping_nodal_values_data->source_value_type)||
		(FE_NODAL_UNKNOWN==scalar_mapping_nodal_values_data->target_value_type))
	{
		if (nodal_value_types=get_FE_node_field_component_nodal_value_types(
			node,fe_field,component_number))
		{
			return_code=1;
		}
	}
	else
	{
		return_code=1;
		nodal_value_types=(FE_nodal_value_type *)NULL;
	}
	if (return_code)
	{
		number_of_values=1+get_FE_node_field_component_number_of_derivatives(node,
			fe_field,component_number);
		number_of_versions=get_FE_node_field_component_number_of_versions(node,
			fe_field,component_number);
		source_index=scalar_mapping_nodal_values_data->source_index;
		source_version=scalar_mapping_nodal_values_data->source_version;
		value_type=scalar_mapping_nodal_values_data->source_value_type;
		if (FE_NODAL_UNKNOWN==value_type)
		{
			source_value_index= -1;
		}
		else
		{
			source_value_index=0;
			nodal_value_type=nodal_value_types;
			while ((source_value_index<number_of_values)&&
				(value_type!= *nodal_value_type))
			{
				nodal_value_type++;
				source_value_index++;
			}
		}
		target_index=scalar_mapping_nodal_values_data->target_index;
		target_version=scalar_mapping_nodal_values_data->target_version;
		value_type=scalar_mapping_nodal_values_data->target_value_type;
		if (FE_NODAL_UNKNOWN==value_type)
		{
			target_value_index= -1;
		}
		else
		{
			target_value_index=0;
			nodal_value_type=nodal_value_types;
			while ((target_value_index<number_of_values)&&
				(value_type!= *nodal_value_type))
			{
				nodal_value_type++;
				target_value_index++;
			}
		}
		number_of_target_values=0;
		if (((node==scalar_mapping_nodal_values_data->target_node)||
			(NULL==scalar_mapping_nodal_values_data->target_node))&&
			(target_version<number_of_versions)&&
			(target_value_index<number_of_values))
		{
			// calculate number_of_target_values at node
			if (-1==target_value_index)
			{
				number_of_target_values=(Variable_size_type)number_of_values;
			}
			else
			{
				number_of_target_values=1;
			}
			if (-1==target_version)
			{
				number_of_target_values *= (Variable_size_type)number_of_versions;
			}
			if (0<number_of_target_values)
			{
				// update "not in target" entries in <scalar_mapping>
				scalar_mapping_iterator=scalar_mapping.begin();
				for (i=scalar_mapping.size();i>0;i--)
				{
					if (target_index==scalar_mapping_iterator->second)
					{
						scalar_mapping_iterator->second += number_of_target_values;
					}
					scalar_mapping_iterator++;
				}
				// update saved target_index
				scalar_mapping_nodal_values_data->target_index +=
					number_of_target_values;
			}
		}
		if (((node==scalar_mapping_nodal_values_data->source_node)||
			(NULL==scalar_mapping_nodal_values_data->source_node))&&
			(source_version<number_of_versions)&&
			(source_value_index<number_of_values))
		{
			if (((node==scalar_mapping_nodal_values_data->target_node)||
				(NULL==scalar_mapping_nodal_values_data->target_node))&&
				(target_version<number_of_versions)&&
				(target_value_index<number_of_values))
			{
				// update scalar_mapping
				for (version=0;version<number_of_versions;version++)
				{
					if ((-1==version)||(source_version==version))
					{
						if ((-1==version)||(target_version==version))
						{
							if (0<=source_value_index)
							{
								if (0<=target_value_index)
								{
									if ((0==scalar_mapping.size())||
										(source_index-(scalar_mapping.back()).first!=
										target_index-(scalar_mapping.back()).second))
									{
										scalar_mapping.push_back(
											std::pair<Variable_size_type,Variable_size_type>(
											source_index,target_index));
									}
								}
								else
								{
									if ((0==scalar_mapping.size())||
										(source_index-(scalar_mapping.back()).first!=
										target_index+source_value_index-
										(scalar_mapping.back()).second))
									{
										scalar_mapping.push_back(
											std::pair<Variable_size_type,Variable_size_type>(
											source_index,target_index+source_value_index));
									}
								}
							}
							else
							{
								if (0<=target_value_index)
								{
									if ((0<target_value_index)&&
										((0==scalar_mapping.size())||
										(scalar_mapping_nodal_values_data->target_index!=
										(scalar_mapping.back()).second)))
									{
										scalar_mapping.push_back(
											std::pair<Variable_size_type,Variable_size_type>(
											source_index,
											scalar_mapping_nodal_values_data->target_index));
									}
									if ((0==scalar_mapping.size())||
										(source_index+target_value_index-
										(scalar_mapping.back()).first!=
										target_index-(scalar_mapping.back()).second))
									{
										scalar_mapping.push_back(
											std::pair<Variable_size_type,Variable_size_type>(
											source_index+target_value_index,target_index));
									}
									if (target_value_index+1<number_of_values)
									{
										scalar_mapping.push_back(
											std::pair<Variable_size_type,Variable_size_type>(
											source_index+target_value_index+1,
											scalar_mapping_nodal_values_data->target_index));
									}
								}
								else
								{
									if ((0==scalar_mapping.size())||
										(source_index-(scalar_mapping.back()).first!=
										target_index-(scalar_mapping.back()).second))
									{
										scalar_mapping.push_back(
											std::pair<Variable_size_type,Variable_size_type>(
											source_index,target_index));
									}
								}
							}
						}
						else
						{
							if ((0==scalar_mapping.size())||
								(scalar_mapping_nodal_values_data->target_index!=
								(scalar_mapping.back()).second))
							{
								scalar_mapping.push_back(
									std::pair<Variable_size_type,Variable_size_type>(source_index,
									scalar_mapping_nodal_values_data->target_index));
							}
						}
						if (0<=source_value_index)
						{
							source_index++;
						}
						else
						{
							source_index += number_of_values;
						}
					}
					if ((-1==version)||(target_version==version))
					{
						if (0<=target_value_index)
						{
							target_index++;
						}
						else
						{
							target_index += number_of_values;
						}
					}
				}
			}
			else
			{
				// calculate number_of_source_values at node
				if (-1==source_value_index)
				{
					number_of_source_values=(Variable_size_type)number_of_values;
				}
				else
				{
					number_of_source_values=1;
				}
				if (-1==source_version)
				{
					number_of_source_values *= (Variable_size_type)number_of_versions;
				}
				if (0<number_of_source_values)
				{
					// update scalar_mapping
					if ((0==scalar_mapping.size())||
						(target_index>(scalar_mapping.back()).second))
					{
						scalar_mapping.push_back(
							std::pair<Variable_size_type,Variable_size_type>(source_index,
							target_index));
					}
					// update saved source_index
					scalar_mapping_nodal_values_data->source_index +=
						number_of_source_values;
				}
			}
		}
		DEALLOCATE(nodal_value_types);
	}
	LEAVE;

	return (return_code);
} /* component_scalar_mapping_nodal_values */

static int scalar_mapping_nodal_values(struct FE_node *node,
	void *scalar_mapping_nodal_values_data_void)
/*******************************************************************************
LAST MODIFIED : 29 December 2003

DESCRIPTION :
Updates the scalar mapping to include the <node> using the source and target
information from <scalar_mapping_nodal_values_data_void>.
==============================================================================*/
{
	int component_number,number_of_components,return_code;
	struct Scalar_mapping_nodal_values_data *scalar_mapping_nodal_values_data;

	ENTER(scalar_mapping_nodal_values);
	return_code=0;
	/* check arguments */
	if (node&&(scalar_mapping_nodal_values_data=
		(struct Scalar_mapping_nodal_values_data *)
		scalar_mapping_nodal_values_data_void))
	{
		if ((node==scalar_mapping_nodal_values_data->source_node)||
			(NULL==scalar_mapping_nodal_values_data->source_node)||
			(node==scalar_mapping_nodal_values_data->target_node)||
			(NULL==scalar_mapping_nodal_values_data->target_node))
		{
			number_of_components=
				scalar_mapping_nodal_values_data->number_of_components;
			component_number=
				scalar_mapping_nodal_values_data->source_component_number;
			if ((component_number==scalar_mapping_nodal_values_data->
				target_component_number)&&(0<=component_number)&&
				(component_number<number_of_components))
			{
				component_scalar_mapping_nodal_values(node,component_number,
					scalar_mapping_nodal_values_data);
			}
			else
			{
				component_scalar_mapping_nodal_values(node,component_number,
					scalar_mapping_nodal_values_data);
			}
		}
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* scalar_mapping_nodal_values */
#endif // defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS) || defined (USE_SCALAR_MAPPING)

class Variable_input_nodal_values;

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_nodal_values>
	Variable_input_nodal_values_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_nodal_values>
	Variable_input_nodal_values_handle;
#else
typedef Variable_input_nodal_values * Variable_input_nodal_values_handle;
#endif

#if defined (USE_ITERATORS)
#if defined (USE_ITERATORS_NESTED)
#else // defined (USE_ITERATORS_NESTED)
// class Variable_input_iterator_representation_atomic_nodal_values
// --------------------------------------------------------------

class Variable_input_iterator_representation_atomic_nodal_values:public
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
	Variable_input_iterator_representation
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
	Handle_iterator_representation<
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	>
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
//******************************************************************************
// LAST MODIFIED : 4 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend int get_next_node(struct FE_node *node,void *get_next_node_data_void);
	public:
		// constructor
		Variable_input_iterator_representation_atomic_nodal_values(const bool begin,
			Variable_input_nodal_values_handle input);
		// destructor
		~Variable_input_iterator_representation_atomic_nodal_values();
		// increment
		void increment();
		// equality
		bool equality(const
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
			Variable_input_iterator_representation
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
			Handle_iterator_representation<
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			>
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
			* representation);
		// dereference
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			dereference() const;
	private:
		enum FE_nodal_value_type *value_types;
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		Handle_iterator<
#if defined (USE_VARIABLES_AS_COMPONENTS)
			Variable_handle
#else // defined (USE_VARIABLES_AS_COMPONENTS)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
			> component_iterator,component_end;
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		int number_of_values,number_of_versions,value_type_index;
		struct FE_region *fe_region;
		Variable_input_nodal_values_handle atomic_input,input;
};
#endif // defined (USE_ITERATORS_NESTED)
#endif // defined (USE_ITERATORS)

// class Variable_input_nodal_values
// ---------------------------------

class Variable_input_nodal_values : public
#if defined (USE_VARIABLE_INPUT)
	Variable_input
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Variable_finite_element;
	friend class Variable_finite_element_check_derivative_functor;
#if defined (USE_ITERATORS)
	friend class Variable_input_iterator_representation_atomic_nodal_values;
#endif // defined (USE_ITERATORS)
	friend int get_next_node(struct FE_node *node,void *get_next_node_data_void);
	public:
		Variable_input_nodal_values(
			const Variable_finite_element_handle& variable_finite_element):
			value_type(FE_NODAL_UNKNOWN),version(-1),node((struct FE_node *)NULL),
			variable_finite_element(variable_finite_element){};
		Variable_input_nodal_values(
			const Variable_finite_element_handle& variable_finite_element,
			struct FE_node *node,enum FE_nodal_value_type value_type,int version):
			value_type(value_type),version(version),node(node),
			variable_finite_element(variable_finite_element)
		{
			if (node)
			{
				ACCESS(FE_node)(node);
			}
		};
		Variable_input_nodal_values(
			const Variable_finite_element_handle& variable_finite_element,
			struct FE_node *node):value_type(FE_NODAL_UNKNOWN),version(-1),
			node(node),variable_finite_element(variable_finite_element)
		{
			if (node)
			{
				ACCESS(FE_node)(node);
			}
		};
		Variable_input_nodal_values(
			const Variable_finite_element_handle& variable_finite_element,
			enum FE_nodal_value_type value_type):value_type(value_type),version(-1),
			node((struct FE_node *)NULL),
			variable_finite_element(variable_finite_element){};
		Variable_input_nodal_values(
			const Variable_finite_element_handle& variable_finite_element,
			int version):value_type(FE_NODAL_UNKNOWN),version(version),
			node((struct FE_node *)NULL),
			variable_finite_element(variable_finite_element){};
		~Variable_input_nodal_values()
		{
			DEACCESS(FE_node)(&node);
		};
#if defined (USE_ITERATORS)
		// copy constructor
		Variable_input_nodal_values(
			const Variable_input_nodal_values& input_nodal_values):
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			(),
			value_type(input_nodal_values.value_type),
			version(input_nodal_values.version),node(input_nodal_values.node),
			variable_finite_element(input_nodal_values.variable_finite_element){};
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			clone() const
		{
			return (Variable_input_nodal_values_handle(
				new Variable_input_nodal_values(*this)));
		};
		virtual bool is_atomic()
		{
			bool result;
			enum FE_nodal_value_type *nodal_value_type,*nodal_value_types;
			int component_number,i,number_of_components,number_of_values,
				number_of_versions;
			struct FE_field *fe_field;

			result=false;
			component_number=0;
			if (node&&variable_finite_element&&
				(fe_field=variable_finite_element->field)&&
				((1==(number_of_components=get_FE_field_number_of_components(
				fe_field)))||
				((0<=(component_number=variable_finite_element->component_number))&&
				(component_number<number_of_components))))
			{
				//???DB.  Could use FE_nodal_value_version_exists instead?  Not quite,
				//  doesn't handle FE_NODAL_UNKNOWN
				number_of_versions=get_FE_node_field_component_number_of_versions(node,
					fe_field,component_number);
				if ((version<number_of_versions)&&
					((0<=version)||(1==number_of_versions)))
				{
					number_of_values=1+get_FE_node_field_component_number_of_derivatives(
						node,fe_field,component_number);
					if (FE_NODAL_UNKNOWN==value_type)
					{
						if (1==number_of_values)
						{
							result=true;
						}
					}
					else
					{
						if (nodal_value_types=get_FE_node_field_component_nodal_value_types(
							node,fe_field,component_number))
						{
							i=number_of_values;
							nodal_value_type=nodal_value_types;
							while ((i>0)&&(value_type!= *nodal_value_type))
							{
								nodal_value_type++;
								i--;
							}
							if (i>0)
							{
								result=true;
							}
							DEALLOCATE(nodal_value_types);
						}
					}
				}
			}

			return (result);
		}
#if defined (USE_ITERATORS_NESTED)
		virtual Iterator begin_atomic_inputs();
		virtual Iterator end_atomic_inputs();
#else // defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Variable_input_iterator begin_atomic_inputs()
		{
			return (Variable_input_iterator(
				new Variable_input_iterator_representation_atomic_nodal_values(true,
				Variable_input_nodal_values_handle(this))));
		};
		virtual Variable_input_iterator end_atomic_inputs()
		{
			return (Variable_input_iterator(
				new Variable_input_iterator_representation_atomic_nodal_values(false,
				Variable_input_nodal_values_handle(this))));
		};
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#if defined (USE_VARIABLE_INPUT)
		virtual Handle_iterator<Variable_input_handle> begin_atomic_inputs()
		{
			return (Handle_iterator<Variable_input_handle>(
				new Variable_input_iterator_representation_atomic_nodal_values(true,
				Variable_input_nodal_values_handle(this))));
		};
		virtual Handle_iterator<Variable_input_handle> end_atomic_inputs()
		{
			return (Handle_iterator<Variable_input_handle>(
				new Variable_input_iterator_representation_atomic_nodal_values(false,
				Variable_input_nodal_values_handle(this))));
		};
#else // defined (USE_VARIABLE_INPUT)
		virtual Handle_iterator<Variable_io_specifier_handle> begin_atomic()
		{
			return (Handle_iterator<Variable_io_specifier_handle>(
				new Variable_input_iterator_representation_atomic_nodal_values(true,
				Variable_input_nodal_values_handle(this))));
		};
		virtual Handle_iterator<Variable_io_specifier_handle> end_atomic()
		{
			return (Handle_iterator<Variable_io_specifier_handle>(
				new Variable_input_iterator_representation_atomic_nodal_values(false,
				Variable_input_nodal_values_handle(this))));
		};
#endif // defined (USE_VARIABLE_INPUT)
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // defined (USE_ITERATORS_NESTED)
#endif // defined (USE_ITERATORS)
		Variable_size_type
#if defined (USE_ITERATORS)
			number_differentiable
#else // defined (USE_ITERATORS)
			size
#endif // defined (USE_ITERATORS)
			()
		{
			int component_number,number_of_components,return_code;
			struct Count_nodal_values_data count_nodal_values_data;
			struct FE_field *fe_field;
			struct FE_region *fe_region;
			Variable_size_type result;

			result=0;
			if (variable_finite_element&&(fe_field=variable_finite_element->field)&&
				(fe_region=variable_finite_element->region())&&
				(0<(number_of_components=get_FE_field_number_of_components(fe_field))))
			{
				component_number=variable_finite_element->component_number;
				count_nodal_values_data.number_of_values=0;
				count_nodal_values_data.value_type=value_type;
				count_nodal_values_data.version=version;
				count_nodal_values_data.fe_field=fe_field;
				count_nodal_values_data.component_number=component_number;
				count_nodal_values_data.number_of_components=number_of_components;
				count_nodal_values_data.number_of_node_offsets=0;
				count_nodal_values_data.offset_nodes=(struct FE_node **)NULL;
				count_nodal_values_data.node_offsets=(int *)NULL;
				count_nodal_values_data.number_of_node_values=(int *)NULL;
				if (node)
				{
					return_code=count_nodal_values(node,(void *)&count_nodal_values_data);
				}
				else
				{
					return_code=FE_region_for_each_FE_node(fe_region,count_nodal_values,
						(void *)&count_nodal_values_data);
				}
				if (return_code)
				{
					result=(Variable_size_type)(count_nodal_values_data.number_of_values);
				}
			}

			return (result);
		};
		virtual bool operator==(const
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			& input)
		{
			try
			{
				const Variable_input_nodal_values& input_nodal_values=
					dynamic_cast<const Variable_input_nodal_values&>(input);

				return (
					(input_nodal_values.variable_finite_element==
					variable_finite_element)&&
					(input_nodal_values.value_type==value_type)&&
					(input_nodal_values.version==version)&&
					(input_nodal_values.node==node));
			}
			catch (std::bad_cast)
			{
				return (false);
			};
		};
#if defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS) || defined (USE_SCALAR_MAPPING)
	private:
		virtual std::list< std::pair<Variable_size_type,Variable_size_type> >
			scalar_mapping_local(Variable_input_handle target)
		{
			std::list< std::pair<Variable_size_type,Variable_size_type> > result(0);

			if (this)
			{
				const Variable_input_nodal_values_handle input_nodal_values=
#if defined (USE_SMART_POINTER)
					boost::dynamic_pointer_cast<Variable_input_nodal_values,
					Variable_input>
#else /* defined (USE_SMART_POINTER) */
					dynamic_cast<Variable_input_nodal_values *>
#endif /* defined (USE_SMART_POINTER) */
					(target);

				if (input_nodal_values)
				{
					if (*this== *input_nodal_values)
					{
						result.push_back(std::pair<Variable_size_type,Variable_size_type>(
							0,0));
					}
					else
					{
						if (variable_finite_element&&
							(input_nodal_values->variable_finite_element)&&
							(variable_finite_element->field==
							input_nodal_values->variable_finite_element->field)&&
							((variable_finite_element->component_number==
							input_nodal_values->variable_finite_element->component_number)||
							(-1==variable_finite_element->component_number)||
							(-1==input_nodal_values->variable_finite_element->
							component_number))&&
							((node==input_nodal_values->node)||
							(NULL==node)||(NULL==input_nodal_values->node))&&
							((value_type==input_nodal_values->value_type)||
							(FE_NODAL_UNKNOWN==value_type)||
							(FE_NODAL_UNKNOWN==input_nodal_values->value_type))&&
							((version==input_nodal_values->version)||
							(-1==version)||(-1==input_nodal_values->version)))
						{
							int number_of_components,return_code;
							struct FE_field *fe_field;
							struct FE_region *fe_region;

							if ((fe_field=variable_finite_element->field)&&
								(fe_region=variable_finite_element->region())&&
								(0<(number_of_components=get_FE_field_number_of_components(
								fe_field))))
							{
								struct Scalar_mapping_nodal_values_data
									scalar_mapping_nodal_values_data(node,input_nodal_values->
									node,value_type,input_nodal_values->value_type,version,
									input_nodal_values->version,variable_finite_element->
									component_number,input_nodal_values->variable_finite_element->
									component_number,fe_field,result);

								if (node&&(input_nodal_values->node))
								{
									return_code=scalar_mapping_nodal_values(node,
										(void *)&scalar_mapping_nodal_values_data);
								}
								else
								{
									return_code=FE_region_for_each_FE_node(fe_region,
										scalar_mapping_nodal_values,
										(void *)&scalar_mapping_nodal_values_data);
								}
							}
						}
					}
				}
				if (0==result.size())
				{
					result.push_back(std::pair<Variable_size_type,Variable_size_type>(
						0,target->size()));
				}
#if defined (USE_SCALAR_MAPPING)
				if (0<size())
				{
					result.push_back(std::pair<Variable_size_type,Variable_size_type>(
						size(),target->size()));
				}
#endif // defined (USE_SCALAR_MAPPING)
			}

			return (result);
		};
#endif // defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS) || defined (USE_SCALAR_MAPPING)
#if defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS)
//???DB.  Where I'm up to
		virtual Variable_input_handle operator_plus_local(
			const Variable_input_handle& second)
		{
			Variable_input_handle result(0);

			if (this)
			{
				const Variable_input_nodal_values_handle input_nodal_values=
#if defined (USE_SMART_POINTER)
					boost::dynamic_pointer_cast<Variable_input_nodal_values,
					Variable_input>
#else /* defined (USE_SMART_POINTER) */
					dynamic_cast<Variable_input_nodal_values *>
#endif /* defined (USE_SMART_POINTER) */
					(second);

				if (input_nodal_values&&(variable_finite_element==
					input_nodal_values->variable_finite_element))
				{
					if (NULL==node)
					{
						if (NULL==input_nodal_values->node)
						{
							if (FE_NODAL_UNKNOWN==value_type)
							{
								if (FE_NODAL_UNKNOWN==input_nodal_values->value_type)
								{
									if ((-1==version)||(-1==input_nodal_values->version))
									{
										result=Variable_input_handle(
											new Variable_input_nodal_values(
											variable_finite_element));
									}
									else
									{
										if (version==input_nodal_values->version)
										{
											result=Variable_input_handle(
												new Variable_input_nodal_values(
												variable_finite_element,(struct FE_node *)NULL,
												FE_NODAL_UNKNOWN,version));
										}
									}
								}
								else
								{
									if (-1==version)
									{
										result=Variable_input_handle(
											new Variable_input_nodal_values(
											variable_finite_element));
									}
									else
									{
										if (version==input_nodal_values->version)
										{
											result=Variable_input_handle(
												new Variable_input_nodal_values(
												variable_finite_element,(struct FE_node *)NULL,
												FE_NODAL_UNKNOWN,version));
										}
									}
								}
							}
							else
							{
								if (FE_NODAL_UNKNOWN==input_nodal_values->value_type)
								{
									if (-1==input_nodal_values->version)
									{
										result=Variable_input_handle(
											new Variable_input_nodal_values(
											variable_finite_element));
									}
									else
									{
										if (version==input_nodal_values->version)
										{
											result=Variable_input_handle(
												new Variable_input_nodal_values(
												variable_finite_element,(struct FE_node *)NULL,
												FE_NODAL_UNKNOWN,version));
										}
									}
								}
								else
								{
									if (value_type==input_nodal_values->value_type)
									{
										if ((-1==version)||(-1==input_nodal_values->version))
										{
											result=Variable_input_handle(
												new Variable_input_nodal_values(
												variable_finite_element,(struct FE_node *)NULL,
												value_type,-1));
										}
										else
										{
											if (version==input_nodal_values->version)
											{
												result=Variable_input_handle(
													new Variable_input_nodal_values(
													variable_finite_element,(struct FE_node *)NULL,
													value_type,version));
											}
										}
									}
								}
							}
						}
						else
						{
							if (FE_NODAL_UNKNOWN==value_type)
							{
								if ((-1==version)||(version==input_nodal_values->version))
								{
									result=Variable_input_handle(
										new Variable_input_nodal_values(
										variable_finite_element,(struct FE_node *)NULL,
										FE_NODAL_UNKNOWN,version));
								}
							}
							else
							{
								if (value_type==input_nodal_values->value_type)
								{
									if ((-1==version)||(version==input_nodal_values->version))
									{
										result=Variable_input_handle(
											new Variable_input_nodal_values(
											variable_finite_element,(struct FE_node *)NULL,
											value_type,version));
									}
								}
							}
						}
					}
					else
					{
						if (NULL==input_nodal_values->node)
						{
							if (FE_NODAL_UNKNOWN==input_nodal_values->value_type)
							{
								if (-1==input_nodal_values->version)
								{
									result=Variable_input_handle(
										new Variable_input_nodal_values(
										variable_finite_element));
								}
								else
								{
									if (version==input_nodal_values->version)
									{
										result=Variable_input_handle(
											new Variable_input_nodal_values(
											variable_finite_element,(struct FE_node *)NULL,
											FE_NODAL_UNKNOWN,version));
									}
								}
							}
							else
							{
								if (value_type==input_nodal_values->value_type)
								{
									if ((-1==input_nodal_values->version)||
										(version==input_nodal_values->version))
									{
										result=Variable_input_handle(
											new Variable_input_nodal_values(
											variable_finite_element,(struct FE_node *)NULL,
											value_type,input_nodal_values->version));
									}
								}
							}
						}
						else
						{
							if (node==input_nodal_values->node)
							{
								if (FE_NODAL_UNKNOWN==value_type)
								{
									if ((-1==version)||(version==input_nodal_values->version))
									{
										result=Variable_input_handle(
											new Variable_input_nodal_values(
											variable_finite_element,node,
											FE_NODAL_UNKNOWN,version));
									}
								}
								else
								{
									if (FE_NODAL_UNKNOWN==input_nodal_values->value_type)
									{
										if ((-1==input_nodal_values->version)||
											(version==input_nodal_values->version))
										{
											result=Variable_input_handle(
												new Variable_input_nodal_values(
												variable_finite_element,node,
												FE_NODAL_UNKNOWN,input_nodal_values->version));
										}
									}
									else
									{
										if (value_type==input_nodal_values->value_type)
										{
											if ((-1==version)||(-1==input_nodal_values->version))
											{
												result=Variable_input_handle(
													new Variable_input_nodal_values(
													variable_finite_element,node,value_type,-1));
											}
											else
											{
												if (version==input_nodal_values->version)
												{
													result=Variable_input_handle(
														new Variable_input_nodal_values(
														variable_finite_element,node,
														value_type,version));
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}

			return (result);
		};
		virtual Variable_input_handle operator_minus_local(
			const Variable_input_handle& second)
		{
			Variable_input_handle result(0);
			const Variable_input_nodal_values_handle input_nodal_values=
#if defined (USE_SMART_POINTER)
				boost::dynamic_pointer_cast<Variable_input_nodal_values,Variable_input>
#else /* defined (USE_SMART_POINTER) */
				dynamic_cast<Variable_input_nodal_values *>
#endif /* defined (USE_SMART_POINTER) */
				(second);

			if (input_nodal_values&&
				(variable_finite_element==input_nodal_values->variable_finite_element))
			{
				if (xi&&(input_nodal_values->xi))
				{
					if (0==(input_nodal_values->indices).size())
					{
						result=Variable_input_handle(new Variable_input_nodal_values(
							variable_finite_element,element&&!(input_nodal_values->element),
							false));
					}
					else
					{
						std::list<Variable_size_type> indices_minus_list(0);
						Variable_size_type i,j;

						if (0==indices.size())
						{
							j=(variable_finite_element->xi).size();
							i=0;
							for (;j>0;j--)
							{
								indices_minus_list.push_back(i);
								i++;
							}
						}
						else
						{
							i++;
							for (j=indices.size();j>0;j--)
							{
								indices_minus_list.push_back(indices[i]);
								i++;
							}
						}
						i=0;
						for (j=(input_nodal_values->indices).size();j>0;j--)
						{
							indices_minus_list.remove((input_nodal_values->indices)[i]);
							i++;
						}
						if (0==indices_minus_list.size())
						{
							result=Variable_input_handle(new Variable_input_nodal_values(
								variable_finite_element,element&&!(input_nodal_values->element),
								false));
						}
						else
						{
							ublas::vector<Variable_size_type>
								indices_minus(indices_minus_list.size());
							std::list<Variable_size_type>::iterator indices_iterator;

							indices_iterator=indices_minus_list.begin();
							i=0;
							for (j=indices_minus_list.size();j>0;j--)
							{
								indices_minus[i]= *indices_iterator;
								i++;
							}
							result=Variable_input_handle(new Variable_input_nodal_values(
								variable_finite_element,indices_minus));
						}
					}
				}
			}
			else
			{
				if (0==result)
				{
					result=Variable_input_handle(this);
				}
			}

			return (result);
		};
		virtual Variable_input_handle intersect_local(
			const Variable_input_handle& second)
		{
			Variable_input_handle result(0);
			const Variable_input_nodal_values_handle input_nodal_values=
#if defined (USE_SMART_POINTER)
				boost::dynamic_pointer_cast<Variable_input_nodal_values,Variable_input>
#else /* defined (USE_SMART_POINTER) */
				dynamic_cast<Variable_input_nodal_values *>
#endif /* defined (USE_SMART_POINTER) */
				(second);

			if (input_nodal_values&&
				(variable_finite_element==input_nodal_values->variable_finite_element))
			{
				if (xi&&(input_nodal_values->xi))
				{
					if (0==indices.size())
					{
						if (0==(input_nodal_values->indices).size())
						{
							result=Variable_input_handle(new Variable_input_nodal_values(
								variable_finite_element,element&&(input_nodal_values->element),
								true));
						}
						else
						{
							result=Variable_input_handle(new Variable_input_nodal_values(
								variable_finite_element,input_nodal_values->indices));
						}
					}
					else
					{
						if (0==(input_nodal_values->indices).size())
						{
							result=Variable_input_handle(new Variable_input_nodal_values(
								variable_finite_element,indices));
						}
						else
						{
							std::list<Variable_size_type> indices_intersect_list(0);
							Variable_size_type i,j;

							i=0;
							for (j=indices.size();j>0;j--)
							{
								if ((input_nodal_values->indices).end()!=std::find(
									(input_nodal_values->indices).begin(),
									(input_nodal_values->indices).end(),indices[i]))
								{
									indices_intersect_list.push_back(indices[i]);
								}
								i++;
							}
							if (0==indices_intersect_list.size())
							{
								result=Variable_input_handle(0);
							}
							else
							{
								ublas::vector<Variable_size_type>
									indices_intersect(indices_intersect_list.size());
								std::list<Variable_size_type>::iterator indices_iterator;

								indices_iterator=indices_intersect_list.begin();
								i=0;
								for (j=indices_intersect_list.size();j>0;j--)
								{
									indices_intersect[i]= *indices_iterator;
									i++;
								}
								result=Variable_input_handle(new Variable_input_nodal_values(
									variable_finite_element,indices_intersect));
							}
						}
					}
				}
				else
				{
					if (element&&(input_nodal_values->element))
					{
						result=Variable_input_handle(new Variable_input_nodal_values(
							variable_finite_element,true,false));
					}
				}
			}

			return (result);
		};
#endif // defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS)
	private:
		enum FE_nodal_value_type value_type;
		int version;
		struct FE_node *node;
		Variable_finite_element_handle variable_finite_element;
};

#if defined (USE_ITERATORS)
#if defined (USE_ITERATORS_NESTED)
#else // defined (USE_ITERATORS_NESTED)
struct Get_next_node_data
{
	int found;
	Variable_input_iterator_representation_atomic_nodal_values *representation;
}; /* struct Get_next_node_data */

int get_next_node(struct FE_node *node,void *get_next_node_data_void)
/*******************************************************************************
LAST MODIFIED : 28 January 2004

DESCRIPTION :
Finds the node after the on in <get_next_node_data>.
==============================================================================*/
{
	enum FE_nodal_value_type *nodal_value_type,*nodal_value_types;
	int number_of_values,number_of_versions,return_code,value_type_index;
	struct Get_next_node_data *get_next_node_data;
	Variable_finite_element_handle variable_finite_element;
	Variable_input_iterator_representation_atomic_nodal_values *representation;
	Variable_input_nodal_values_handle atomic_input,input;

	ENTER(get_next_node);
	return_code=0;
	/* check arguments */
	if (node&&
		(get_next_node_data=(struct Get_next_node_data *)get_next_node_data_void)&&
		(representation=get_next_node_data->representation)&&
		(input=representation->input)&&(atomic_input=representation->atomic_input)&&
		(variable_finite_element=atomic_input->variable_finite_element))
	{
		if (get_next_node_data->found)
		{
			number_of_versions=(variable_finite_element->number_of_versions)(node,0);
			if (representation->input->version<number_of_versions)
			{
				number_of_values=1+(variable_finite_element->number_of_derivatives)(
					node,0);
				if (nodal_value_types=(variable_finite_element->nodal_value_types)(
					node,0))
				{
					value_type_index=0;
					if (FE_NODAL_UNKNOWN==representation->input->value_type)
					{
						return_code=1;
					}
					else
					{
						nodal_value_type=nodal_value_types;
						while ((value_type_index<number_of_values)&&
							(*nodal_value_type!=representation->input->value_type))
						{
							value_type_index++;
							nodal_value_type++;
						}
						if (value_type_index<number_of_values)
						{
							return_code=1;
						}
					}
					if (return_code)
					{
						representation->number_of_versions=number_of_versions;
						representation->number_of_values=number_of_values;
						DEALLOCATE(representation->value_types);
						representation->value_types=nodal_value_types;
						representation->value_type_index=value_type_index;
					}
					else
					{
						DEALLOCATE(nodal_value_types);
					}
				}
			}
		}
		else
		{
			if (node==get_next_node_data->representation->atomic_input->node)
			{
				get_next_node_data->found=1;
			}
		}
	}
	LEAVE;

	return (return_code);
} /* get_next_node */

// class Variable_input_iterator_representation_atomic_nodal_values
// ----------------------------------------------------------------

Variable_input_iterator_representation_atomic_nodal_values::
	Variable_input_iterator_representation_atomic_nodal_values(const bool begin,
	Variable_input_nodal_values_handle input):value_types(0),
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
	component_iterator(input->variable_finite_element->begin_components()),
	component_end(input->variable_finite_element->end_components()),
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
	number_of_values(0),number_of_versions(0),value_type_index(0),
	fe_region(input->variable_finite_element->region()),atomic_input(0),
	input(input)
//******************************************************************************
// LAST MODIFIED : 30 January 2004
//
// DESCRIPTION :
// Constructor.  If <begin> then the constructed iterator points to the first
// atomic input, otherwise it points to one past the last atomic input.
//
//???DB.  Have to clone variable_finite_element as well as input
//==============================================================================
{
	ACCESS(FE_region)(fe_region);
	if (begin&&input&&(input->variable_finite_element))
	{
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		Variable_finite_element_handle variable_finite_element;

		while ((component_iterator!=component_end)&&
			(!(variable_finite_element=
#if defined (USE_SMART_POINTER)
			boost::dynamic_pointer_cast<Variable_finite_element,Variable>
#else /* defined (USE_SMART_POINTER) */
			dynamic_cast<Variable_finite_element *>
#endif /* defined (USE_SMART_POINTER) */
			(*component_iterator))||
			(1!=variable_finite_element->number_differentiable())))
		{
			component_iterator++;
		}
		if (component_iterator!=component_end)
		{
			if (atomic_input=Variable_input_nodal_values_handle(
				new Variable_input_nodal_values(variable_finite_element,input->node,
				input->value_type,input->version)))
			{
				//???DB.  This can be made more efficient by adding iterators to
				//  LISTs or by adding a GET_NEXT function to LISTs
				struct Get_next_node_data get_next_node_data;
				struct FE_node *save_node;

				// want first so set found to true
				get_next_node_data.found=1;
				get_next_node_data.representation=this;
				// save atomic_input->node for DEACCESSing
				save_node=atomic_input->node;
				if (atomic_input->node=FE_region_get_first_FE_node_that(fe_region,
					get_next_node,&get_next_node_data))
				{
					ACCESS(FE_node)(atomic_input->node);
					if (input->version<0)
					{
						atomic_input->version=0;
					}
					atomic_input->value_type=value_types[value_type_index];
				}
				else
				{
					atomic_input=0;
				}
				DEACCESS(FE_node)(&save_node);
			}
		}
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
	}
}

Variable_input_iterator_representation_atomic_nodal_values::
	~Variable_input_iterator_representation_atomic_nodal_values()
//******************************************************************************
// LAST MODIFIED : 28 January 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	DEACCESS(FE_region)(&fe_region);
}

void Variable_input_iterator_representation_atomic_nodal_values::increment()
//******************************************************************************
// LAST MODIFIED : 28 January 2004
//
// DESCRIPTION :
// Increments the iterator to the next atomic input.  The incrementing order
// from fastest to slowest is: value type, version, node, component.
//==============================================================================
{
	if (atomic_input)
	{
		bool finished;

		finished=false;
		if (FE_NODAL_UNKNOWN==input->value_type)
		{
			value_type_index++;
			if (value_type_index<number_of_values)
			{
				atomic_input->value_type=value_types[value_type_index];
				finished=true;
			}
		}
		if (!finished)
		{
			if (input->version<0)
			{
				(atomic_input->version)++;
				if (atomic_input->version<number_of_versions)
				{
					if (FE_NODAL_UNKNOWN==input->value_type)
					{
						value_type_index=0;
						atomic_input->value_type=value_types[value_type_index];
					}
					finished=true;
				}
			}
			if (!finished)
			{
				//???DB.  This can be made more efficient by adding iterators to
				//  LISTs or by adding a GET_NEXT function to LISTs
				struct Get_next_node_data get_next_node_data;
				struct FE_node *save_node;

				if ((struct FE_node *)NULL!=input->node)
				{
					get_next_node_data.found=0;
					get_next_node_data.representation=this;
					// save atomic_input->node for DEACCESSing
					save_node=atomic_input->node;
					if (atomic_input->node=FE_region_get_first_FE_node_that(
						fe_region,get_next_node,&get_next_node_data))
					{
						ACCESS(FE_node)(atomic_input->node);
						if (input->version<0)
						{
							atomic_input->version=0;
						}
						atomic_input->value_type=value_types[value_type_index];
						finished=true;
					}
					DEACCESS(FE_node)(&save_node);
				}
				if (!finished)
				{
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
					Variable_finite_element_handle variable_finite_element;

					do
					{
						component_iterator++;
					} while ((component_iterator!=component_end)&&
						(!(variable_finite_element=
#if defined (USE_SMART_POINTER)
						boost::dynamic_pointer_cast<Variable_finite_element,Variable>
#else /* defined (USE_SMART_POINTER) */
						dynamic_cast<Variable_finite_element *>
#endif /* defined (USE_SMART_POINTER) */
						(*component_iterator))||
						(1!=variable_finite_element->number_differentiable())));
					if (component_iterator!=component_end)
					{
						atomic_input->variable_finite_element=variable_finite_element;
						// want first so set found to true
						get_next_node_data.found=1;
						get_next_node_data.representation=this;
						// save atomic_input->node for DEACCESSing
						save_node=atomic_input->node;
						if (atomic_input->node=FE_region_get_first_FE_node_that(
							fe_region,get_next_node,&get_next_node_data))
						{
							ACCESS(FE_node)(atomic_input->node);
							if (input->version<0)
							{
								atomic_input->version=0;
							}
							atomic_input->value_type=value_types[value_type_index];
							finished=true;
						}
						DEACCESS(FE_node)(&save_node);
					}
					if (!finished)
					{
						// end
						atomic_input=0;
					}
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
				}
			}
		}
	}
}

bool Variable_input_iterator_representation_atomic_nodal_values::equality(
	const
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
	Variable_input_iterator_representation
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
	Handle_iterator_representation<
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	>
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
	* representation)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Tests <*this> and <*representation> for equality.
//==============================================================================
{
	const Variable_input_iterator_representation_atomic_nodal_values
		*representation_nodal_values=dynamic_cast<
		const Variable_input_iterator_representation_atomic_nodal_values *>(
		representation);

	return (representation_nodal_values&&
		(input==representation_nodal_values->input)&&
		((atomic_input&&(representation_nodal_values->atomic_input)&&
		(*atomic_input== *(representation_nodal_values->atomic_input)))||
		(!atomic_input&&!(representation_nodal_values->atomic_input))));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_input_iterator_representation_atomic_nodal_values::dereference()
	const
//******************************************************************************
// LAST MODIFIED : 26 January 2004
//
// DESCRIPTION :
// Returns the atomic input for the iterator.
//==============================================================================
{
	return (atomic_input);
}
#endif // defined (USE_ITERATORS_NESTED)
#endif // defined (USE_ITERATORS)


// global classes
// ==============

// class Variable_element_xi
// -------------------------

Variable_element_xi::Variable_element_xi(struct FE_element *element,
	const Vector& xi):Variable(),xi(xi),element(element)
//******************************************************************************
// LAST MODIFIED : 10 November 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (element)
	{
		if (xi.size()==(Variable_size_type)get_FE_element_dimension(element))
		{
			ACCESS(FE_element)(element);
		}
		else
		{
			throw std::invalid_argument("Variable_element_xi::Variable_element_xi");
		}
	}
}

Variable_element_xi::Variable_element_xi(
	const Variable_element_xi& variable_element_xi):Variable(),
	xi(variable_element_xi.xi),element(variable_element_xi.element)
//******************************************************************************
// LAST MODIFIED : 10 November 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (element)
	{
		if (xi.size()==(Variable_size_type)get_FE_element_dimension(element))
		{
			ACCESS(FE_element)(element);
		}
		else
		{
			throw std::invalid_argument("Variable_element_xi::Variable_element_xi");
		}
	}
}

Variable_element_xi& Variable_element_xi::operator=(
	const Variable_element_xi& variable_element_xi)
//******************************************************************************
// LAST MODIFIED : 10 November 2003
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	Variable_size_type number_of_xi=variable_element_xi.xi.size();

	if (variable_element_xi.element)
	{
		if ((0==number_of_xi)||
			(number_of_xi==(Variable_size_type)get_FE_element_dimension(
			variable_element_xi.element)))
		{
			if (element)
			{
				DEACCESS(FE_element)(&element);
				element=ACCESS(FE_element)(variable_element_xi.element);
			}
			xi.resize(number_of_xi);
			xi=variable_element_xi.xi;
		}
		else
		{
			throw std::invalid_argument("Variable_element_xi::operator=");
		}
	}
	else
	{
		if (element)
		{
			DEACCESS(FE_element)(&element);
		}
		xi.resize(number_of_xi);
		xi=variable_element_xi.xi;
	}

	return (*this);
}

Variable_element_xi::~Variable_element_xi()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	DEACCESS(FE_element)(&element);
}

#if defined (USE_ITERATORS)
#if defined (USE_VARIABLES_AS_COMPONENTS)
bool Variable_element_xi::is_component()
//******************************************************************************
// LAST MODIFIED : 29 January 2004
//
// DESCRIPTION :
// ???DB.  Where I'm up to
// ???DB.  Not correct.  A component should refer back to the variable rather
//   than having its own storage.  Like inputs.  No difference?
//==============================================================================
{
	return ((element&&(0==xi.size()))||(!element&&(1==xi.size())));
}
#else // defined (USE_VARIABLES_AS_COMPONENTS)
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
#endif // defined (USE_ITERATORS)

Variable_size_type Variable_element_xi::
#if defined (USE_ITERATORS)
	number_differentiable
#else // defined (USE_ITERATORS)
	size
#endif // defined (USE_ITERATORS)
	() const
//******************************************************************************
// LAST MODIFIED : 20 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (xi.size());
}

#if defined (USE_ITERATORS)
#else // defined (USE_ITERATORS)
Vector *Variable_element_xi::scalars()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (new Vector(xi));
}
#endif // defined (USE_ITERATORS)

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_element_xi::input_element_xi()
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the element/xi input.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_element_xi(Variable_element_xi_handle(this))));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_element_xi::input_element()
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the element input.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_element_xi(Variable_element_xi_handle(this),true,
		false)));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_element_xi::input_xi()
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the xi input.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_element_xi(Variable_element_xi_handle(this),false)));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_element_xi::input_xi(Variable_size_type index)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the xi input.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_element_xi(Variable_element_xi_handle(this),index)));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_element_xi::input_xi(const ublas::vector<Variable_size_type> indices)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the xi input.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_element_xi(Variable_element_xi_handle(this),indices)));
}

Variable_handle Variable_element_xi::operator-(const Variable& second)
	const
//******************************************************************************
// LAST MODIFIED : 20 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	Variable_element_xi_handle result(0);

	try
	{
		const Variable_vector& second_vector=
			dynamic_cast<const Variable_vector&>(second);
		Variable_size_type i,number_of_values;

		number_of_values=second_vector.
#if defined (USE_ITERATORS)
			number_differentiable
#else // defined (USE_ITERATORS)
			size
#endif // defined (USE_ITERATORS)
			();
		if (this&&(xi.size()==number_of_values)&&(0<number_of_values))
		{
			FE_value *increment_array,*xi_array;

			increment_array=new FE_value[number_of_values];
			xi_array=new FE_value[number_of_values];
			if (increment_array&&xi_array)
			{
				struct FE_element *increment_element;
				for (i=0;i<number_of_values;i++)
				{
					increment_array[i]=(FE_value)second_vector[i];
					xi_array[i]=(FE_value)xi[i];
				}
				increment_element=element;
				if (FE_element_xi_increment(&increment_element,xi_array,
					increment_array))
				{
					if (result=Variable_element_xi_handle(new Variable_element_xi(
						increment_element,xi)))
					{
						for (i=0;i<number_of_values;i++)
						{
							(result->xi)[i]=(Scalar)(xi_array[i]);
						}
					}
				}
			}
			delete [] increment_array;
			delete [] xi_array;
		}
	}
	catch (std::bad_cast)
	{
		// do nothing
	}

	return (result);
}

Variable_handle Variable_element_xi::operator-=(const Variable& second)
//******************************************************************************
// LAST MODIFIED : 20 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	try
	{
		const Variable_vector& second_vector=
			dynamic_cast<const Variable_vector&>(second);
		Variable_size_type i,number_of_values;

		number_of_values=second_vector.
#if defined (USE_ITERATORS)
			number_differentiable
#else // defined (USE_ITERATORS)
			size
#endif // defined (USE_ITERATORS)
			();
		if (this&&(xi.size()==number_of_values)&&(0<number_of_values))
		{
			FE_value *increment_array,*xi_array;

			increment_array=new FE_value[number_of_values];
			xi_array=new FE_value[number_of_values];
			if (increment_array&&xi_array)
			{
				struct FE_element *increment_element;
				for (i=0;i<number_of_values;i++)
				{
					increment_array[i]=(FE_value)second_vector[i];
					xi_array[i]=(FE_value)xi[i];
				}
				increment_element=element;
				if (FE_element_xi_increment(&increment_element,xi_array,
					increment_array))
				{
					if (element)
					{
						DEACCESS(FE_element)(&element);
					}
					element=ACCESS(FE_element)(increment_element);
					for (i=0;i<number_of_values;i++)
					{
						xi[i]=(Scalar)(xi_array[i]);
					}
				}
			}
			delete [] increment_array;
			delete [] xi_array;
		}
	}
	catch (std::bad_cast)
	{
		// do nothing
	}

	return (Variable_element_xi_handle(this));
}

Variable_handle Variable_element_xi::clone() const
//******************************************************************************
// LAST MODIFIED : 8 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (Variable_element_xi_handle(new Variable_element_xi(*this)));
}

Variable_handle Variable_element_xi::evaluate_local()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Duplicate <this> so that <this> isn't changed by operations on the result.
//==============================================================================
{
	return (Variable_handle(new Variable_element_xi(*this)));
}

#if defined (USE_ITERATORS)
//???DB.  To be done
#else // defined (USE_ITERATORS)
bool Variable_element_xi::evaluate_derivative_local(Matrix& matrix,
	std::list<
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Variable_size_type i,index,number_of_input_values,number_of_values;
	Variable_input_element_xi_handle input_element_xi_handle;

	result=true;
	// matrix is zero'd on entry
	if ((1==independent_variables.size())&&(input_element_xi_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_element_xi,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(independent_variables.front())
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_element_xi *>(independent_variables.front())
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_element_xi_handle->variable_element_xi==Variable_handle(this))&&
		(input_element_xi_handle->xi))
	{
		number_of_values=this->size();
		number_of_input_values=(input_element_xi_handle->indices).size();
		Assert((number_of_values==matrix.size1())&&((0==number_of_input_values)||
			(number_of_input_values==matrix.size2())),std::logic_error(
			"Variable_element_xi::evaluate_derivative_local.  "
			"Incorrect matrix size"));
		if (0==number_of_input_values)
		{
			for (i=0;i<number_of_values;i++)
			{
				matrix(i,i)=1;
			}
		}
		else
		{
			for (i=0;i<number_of_input_values;i++)
			{
				index=input_element_xi_handle->indices[i];
				if ((0<index)&&(index<=number_of_values))
				{
					matrix(index-1,i)=1;
				}
			}
		}
	}

	return (result);
}
#endif // defined (USE_ITERATORS)

Variable_handle Variable_element_xi::get_input_value_local(
	const
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	& input)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	Variable_handle value_element_xi;
	Variable_input_element_xi_handle input_element_xi_handle;

	if ((input_element_xi_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_element_xi,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_element_xi *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_element_xi_handle->variable_element_xi==
		Variable_element_xi_handle(this)))
	{
		if (input_element_xi_handle->xi)
		{
			Variable_size_type number_of_input_values=
				input_element_xi_handle->
#if defined (USE_ITERATORS)
				number_differentiable
#else // defined (USE_ITERATORS)
				size
#endif // defined (USE_ITERATORS)
				();

			if (0==(input_element_xi_handle->indices).size())
			{
				if (input_element_xi_handle->element)
				{
					value_element_xi=Variable_element_xi_handle(new Variable_element_xi(
						element,xi));
				}
				else
				{
					value_element_xi=Variable_vector_handle(new Variable_vector(xi));
				}
			}
			else
			{
				Variable_size_type i,index,number_of_values=this->
#if defined (USE_ITERATORS)
					number_differentiable
#else // defined (USE_ITERATORS)
					size
#endif // defined (USE_ITERATORS)
					();
				ublas::vector<Scalar> selected_values(number_of_input_values);

				for (i=0;i<number_of_input_values;i++)
				{
					index=input_element_xi_handle->indices[i];
					if ((0<index)&&(index<=number_of_values))
					{
						selected_values[i]=xi[index-1];
					}
				}
#if defined (OLD_CODE)
				//???DB.  Doesn't make sense to have element and indices
				if (input_element_xi_handle->element)
				{
					value_element_xi=Variable_element_xi_handle(
						new Variable_element_xi(element,selected_values));
				}
				else
				{
#endif // defined (OLD_CODE)
					value_element_xi=Variable_vector_handle(
						new Variable_vector(selected_values));
#if defined (OLD_CODE)
				}
#endif // defined (OLD_CODE)
			}
		}
		else
		{
			if (input_element_xi_handle->element)
			{
				value_element_xi=Variable_element_xi_handle(
					new Variable_element_xi(element,Vector(0)));
			}
			else
			{
				value_element_xi=Variable_vector_handle(new Variable_vector(Vector(0)));
			}
		}
	}
	else
	{
		value_element_xi=Variable_element_xi_handle((Variable_element_xi *)0);
	}

	return (value_element_xi);
}

bool Variable_element_xi::set_input_value_local(
	const
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
	& values)
//******************************************************************************
// LAST MODIFIED : 9 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Variable_input_element_xi_handle input_element_xi_handle;

	result=false;
	if ((input_element_xi_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_element_xi,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_element_xi *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_element_xi_handle->variable_element_xi==
		Variable_element_xi_handle(this)))
	{
		if (input_element_xi_handle->element)
		{
			Variable_element_xi_handle values_element_xi_handle;

			if (values_element_xi_handle=
#if defined (USE_SMART_POINTER)
				boost::dynamic_pointer_cast<Variable_element_xi,Variable>(values)
#else /* defined (USE_SMART_POINTER) */
				dynamic_cast<Variable_element_xi *>(values)
#endif /* defined (USE_SMART_POINTER) */
				)
			{
				if (input_element_xi_handle->xi)
				{
					Variable_size_type number_of_input_values=
						(input_element_xi_handle->indices).size();

					if (0==number_of_input_values)
					{
						Variable_size_type number_of_xi=
							(values_element_xi_handle->xi).size();

						if (!(values_element_xi_handle->element)||
							(number_of_xi==(Variable_size_type)get_FE_element_dimension(
							values_element_xi_handle->element)))
						{
							DEACCESS(FE_element)(&element);
							if (values_element_xi_handle->element)
							{
								element=ACCESS(FE_element)(values_element_xi_handle->element);
							}
							xi.resize(number_of_xi);
							xi=values_element_xi_handle->xi;
							result=true;
						}
					}
				}
			}
		}
		else
		{
			if (input_element_xi_handle->xi)
			{
				Vector *values_vector;

				if (values_vector=
#if defined (USE_ITERATORS)
					//???DB.  To be done
					0
#else // defined (USE_ITERATORS)
					values->scalars()
#endif // defined (USE_ITERATORS)
					)
				{
					Variable_size_type number_of_input_values=
						(input_element_xi_handle->indices).size();

					if (0==number_of_input_values)
					{
						if ((this->xi).size()==values_vector->size())
						{
							this->xi= *values_vector;
						}
					}
					else
					{
						if (number_of_input_values==values_vector->size())
						{
							Variable_size_type i,index,number_of_values=(this->xi).size();

							for (i=0;i<number_of_input_values;i++)
							{
								index=input_element_xi_handle->indices[i];
								if ((0<index)&&(index<=number_of_values))
								{
									(this->xi)[index-1]=(*values_vector)[i];
								}
							}
						}
					}
					delete values_vector;
					result=true;
				}
			}
		}
	}

	return (result);
}

string_handle Variable_element_xi::get_string_representation_local()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		if (element)
		{
			out << "element=" << FE_element_get_cm_number(element) << " ";
		}
		out << "xi=" << xi;
		*return_string=out.str();
	}

	return (return_string);
}

// class Variable_finite_element
// -----------------------------

Variable_finite_element::Variable_finite_element(struct FE_field *field):
	Variable(),component_number(-1),time(0),element((struct FE_element *)NULL),
	field(field),node((struct FE_node *)NULL),xi()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (field)
	{
		ACCESS(FE_field)(field);
	}
}

Variable_finite_element::Variable_finite_element(struct FE_field *field,
	int component_number):Variable(),component_number(component_number-1),time(0),
	element((struct FE_element *)NULL),field(field),node((struct FE_node *)NULL),
	xi()
//******************************************************************************
// LAST MODIFIED : 28 January 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (field)
	{
		ACCESS(FE_field)(field);
		if ((component_number<0)||
			(component_number>get_FE_field_number_of_components(field)))
		{
			component_number= -1;
		}
	}
}

Variable_finite_element::Variable_finite_element(struct FE_field *field,
	const std::string component_name):Variable(),component_number(-1),time(0),
	element((struct FE_element *)NULL),field(field),node((struct FE_node *)NULL),
	xi()
//******************************************************************************
// LAST MODIFIED : 12 November 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (field)
	{
		char *name;
		int i;

		i=get_FE_field_number_of_components(field);
		if (0<i)
		{
			name=(char *)NULL;
			do
			{
				i--;
				if (name)
				{
					DEALLOCATE(name);
				}
				name=get_FE_field_component_name(field,i);
			} while ((i>0)&&(std::string(name)!=component_name));
			if (std::string(name)==component_name)
			{
				component_number=i;
			}
		}
		ACCESS(FE_field)(field);
	}
}

Variable_finite_element::Variable_finite_element(
	const Variable_finite_element& variable_finite_element):
	Variable(),component_number(variable_finite_element.component_number),
	time(variable_finite_element.time),element(variable_finite_element.element),
	field(variable_finite_element.field),node(variable_finite_element.node),
	xi(variable_finite_element.xi)
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Variable_finite_element& Variable_finite_element::operator=(
	const Variable_finite_element& variable_finite_element)
//******************************************************************************
// LAST MODIFIED : 10 November 2003
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	component_number=variable_finite_element.component_number;
	time=variable_finite_element.time;
	DEACCESS(FE_element)(&element);
	if (variable_finite_element.element)
	{
		element=ACCESS(FE_element)(variable_finite_element.element);
	}
	DEACCESS(FE_field)(&field);
	if (variable_finite_element.field)
	{
		field=ACCESS(FE_field)(variable_finite_element.field);
	}
	DEACCESS(FE_node)(&node);
	if (variable_finite_element.node)
	{
		node=ACCESS(FE_node)(variable_finite_element.node);
	}
	xi.resize(variable_finite_element.xi.size());
	xi=variable_finite_element.xi;

	return (*this);
}

Variable_finite_element::~Variable_finite_element()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	DEACCESS(FE_element)(&element);
	DEACCESS(FE_field)(&field);
	DEACCESS(FE_node)(&node);
}

Variable_size_type Variable_finite_element::
#if defined (USE_ITERATORS)
	number_differentiable
#else // defined (USE_ITERATORS)
	size
#endif // defined (USE_ITERATORS)
	() const
//******************************************************************************
// LAST MODIFIED : 20 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	Variable_size_type result;

	result=get_FE_field_number_of_components(field);
	if (-1!=component_number)
	{
		if ((0<=component_number)&&((Variable_size_type)component_number<result))
		{
			result=1;
		}
		else
		{
			result=0;
		}
	}

	return (result);
}

#if defined (USE_ITERATORS)
#else // defined (USE_ITERATORS)
Vector *Variable_finite_element::scalars()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (evaluate_local()->scalars());
}
#endif // defined (USE_ITERATORS)

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_finite_element::input_element_xi()
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the element/xi input.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_element_xi(Variable_finite_element_handle(this))));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_finite_element::input_element()
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the element input.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_element_xi(Variable_finite_element_handle(this),true,
		false)));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_finite_element::input_xi()
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the xi input.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_element_xi(Variable_finite_element_handle(this),
		false)));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_finite_element::input_xi(
	Variable_size_type index)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the xi input.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_element_xi(Variable_finite_element_handle(this),
		index)));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_finite_element::input_xi(
	const ublas::vector<Variable_size_type> indices)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the xi input.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_element_xi(Variable_finite_element_handle(this),
		indices)));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_finite_element::input_nodal_values()
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the nodal values input.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_nodal_values(Variable_finite_element_handle(this))));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_finite_element::input_nodal_values(
	struct FE_node *node,enum FE_nodal_value_type value_type,int version)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the nodal values input.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_nodal_values(Variable_finite_element_handle(this),node,
		value_type,version)));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_finite_element::input_nodal_values(struct FE_node *node)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the nodal values input.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_nodal_values(Variable_finite_element_handle(this),
		node)));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_finite_element::input_nodal_values(
	enum FE_nodal_value_type value_type)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the nodal values input.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_nodal_values(Variable_finite_element_handle(this),
		value_type)));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_finite_element::input_nodal_values(int version)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the nodal values input.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_nodal_values(Variable_finite_element_handle(this),
		version)));
}

struct FE_region *Variable_finite_element::region() const
//******************************************************************************
// LAST MODIFIED : 28 January 2004
//
// DESCRIPTION :
// return the region that the field is defined for
//
// NB.  The calling program should use ACCESS(FE_region) and
//   DEACCESS(FE_region) to manage the lifetime of the returned region
//==============================================================================
{
	return (FE_field_get_FE_region(field));
}

Variable_size_type Variable_finite_element::number_of_versions(
	struct FE_node *node,Variable_size_type component_number)
//******************************************************************************
// LAST MODIFIED : 28 January 2004
//
// DESCRIPTION :
// Return the number of versions for the component at the node.
//==============================================================================
{
	int local_component_number;

	local_component_number=this->component_number;
	if (-1==local_component_number)
	{
		local_component_number=(int)component_number;
	}

	return ((Variable_size_type)get_FE_node_field_component_number_of_versions(
		node,field,local_component_number));
}

Variable_size_type Variable_finite_element::number_of_derivatives(
	struct FE_node *node,Variable_size_type component_number)
//******************************************************************************
// LAST MODIFIED : 28 January 2004
//
// DESCRIPTION :
// Return the number of derivatives for the component at the node.
//==============================================================================
{
	int local_component_number;

	local_component_number=this->component_number;
	if (-1==local_component_number)
	{
		local_component_number=(int)component_number;
	}

	return ((Variable_size_type)get_FE_node_field_component_number_of_derivatives(
		node,field,local_component_number));
}

enum FE_nodal_value_type *Variable_finite_element::nodal_value_types(
	struct FE_node *node,Variable_size_type component_number)
//******************************************************************************
// LAST MODIFIED : 28 January 2004
//
// DESCRIPTION :
// Return the nodal value types for the component at the node.
//
// NB.  The calling program should DEALLOCATE the returned array when its no
//   longer needed
//==============================================================================
{
	int local_component_number;

	local_component_number=this->component_number;
	if (-1==local_component_number)
	{
		local_component_number=(int)component_number;
	}

	return (get_FE_node_field_component_nodal_value_types(node,field,
		local_component_number));
}

Variable_handle Variable_finite_element::clone() const
//******************************************************************************
// LAST MODIFIED : 8 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (Variable_finite_element_handle(new Variable_finite_element(*this)));
}

Variable_handle Variable_finite_element::evaluate_local()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	int element_dimension,number_of_components;
	FE_value *values,*xi_coordinates;
	Variable_handle result;

	values=(FE_value *)NULL;
	xi_coordinates=(FE_value *)NULL;
	if (field&&
		(0<(number_of_components=get_FE_field_number_of_components(field)))&&
		ALLOCATE(values,FE_value,number_of_components)&&((node&&!element)||
		(!node&&element&&(0<(element_dimension=get_FE_element_dimension(element)))&&
		((Variable_size_type)element_dimension==xi.size())&&ALLOCATE(xi_coordinates,
		FE_value,element_dimension))))
	{
		int i;
		Variable_size_type j,number_of_values;

		if (0<=component_number)
		{
			number_of_values=1;
		}
		else
		{
			number_of_values=(Variable_size_type)number_of_components;
		}
		for (i=0;i<element_dimension;i++)
		{
			xi_coordinates[i]=(FE_value)(xi[i]);
		}
		if (calculate_FE_field(field,component_number,node,element,xi_coordinates,
			(FE_value)time,values))
		{
			Vector result_vector(number_of_values);

			for (j=0;j<number_of_values;j++)
			{
				result_vector[j]=values[j];
			}

			result=Variable_handle(new Variable_vector(result_vector));
		}
		else
		{
			result=Variable_handle(new Variable_vector(Vector(0)));
		}
	}
	else
	{
		result=Variable_handle(new Variable_vector(Vector(0)));
	}
	DEALLOCATE(xi_coordinates);
	DEALLOCATE(values);

	return (result);
}

#if defined (USE_ITERATORS)
//???DB.  To be done
#else // defined (USE_ITERATORS)
static int extract_component_values(
	struct FE_element_field_values *element_field_values,
	int number_of_components,int component_number,
	int **numbers_of_component_values_address,
	FE_value ***component_values_address)
/*******************************************************************************
LAST MODIFIED : 9 November 2003

DESCRIPTION :
Extracts the values for the component(s) (<number_of_components> and
<component_number>) from <element_field_values> and puts in
<*component_values_address> (after allocating storage).

If <*numbers_of_component_values_address> is NULL then storage is allocated for
it and the number of values extracted for each component entered.  Otherwise,
checks that the number of values extracted for each component agrees with the
entries.
==============================================================================*/
{
	FE_value **component_values;
	int extract_numbers_of_component_values,i,number_of_component_values,
		*numbers_of_component_values,return_code;

	ENTER(extract_component_values);
	return_code=0;
	Assert(element_field_values&&
		(((1==number_of_components)&&(0<=component_number))||
		((0<number_of_components)&&(-1==component_number)))&&
		numbers_of_component_values_address&&component_values_address,
		std::logic_error("extract_component_values.  Invalid argument(s)"));
	extract_numbers_of_component_values=
		!(*numbers_of_component_values_address);
	/* allocate storage for components */
	if (extract_numbers_of_component_values)
	{
		ALLOCATE(numbers_of_component_values,int,number_of_components);
	}
	else
	{
		numbers_of_component_values= *numbers_of_component_values_address;
	}
	ALLOCATE(component_values,FE_value *,number_of_components);
	if (component_values&&numbers_of_component_values)
	{
		if (-1==component_number)
		{
			i=0;
			return_code=1;
			while (return_code&&(i<number_of_components))
			{
				if (return_code=FE_element_field_values_get_component_values(
					element_field_values,i,&number_of_component_values,
					component_values+i))
				{
					if (extract_numbers_of_component_values)
					{
						numbers_of_component_values[i]=number_of_component_values;
					}
					else
					{
						if (numbers_of_component_values[i]!=number_of_component_values)
						{
							DEALLOCATE(component_values[i]);
							return_code=0;
						}
					}
					i++;
				}
			}
			if (!return_code)
			{
				while (i>0)
				{
					i--;
					DEALLOCATE(component_values[i]);
				}
			}
		}
		else
		{
			if (return_code=FE_element_field_values_get_component_values(
				element_field_values,component_number,&number_of_component_values,
				component_values))
			{
				if (extract_numbers_of_component_values)
				{
					*numbers_of_component_values=number_of_component_values;
				}
				else
				{
					if (*numbers_of_component_values!=number_of_component_values)
					{
						DEALLOCATE(*component_values);
						return_code=0;
					}
				}
			}
		}
		if (return_code)
		{
			*component_values_address=component_values;
			*numbers_of_component_values_address=numbers_of_component_values;
		}
	}
	if (!return_code)
	{
		DEALLOCATE(component_values);
		if (extract_numbers_of_component_values)
		{
			DEALLOCATE(numbers_of_component_values);
		}
	}
	LEAVE;

	return (return_code);
} /* extract_component_values */

static int extract_component_monomial_info(
	struct FE_element_field_values *element_field_values,
	int number_of_components,int component_number,int number_of_xi,
	int **numbers_of_component_values_address,
	int ***component_monomial_info_address,FE_value **monomial_values_address)
/*******************************************************************************
LAST MODIFIED : 9 November 2003

DESCRIPTION :
Extracts the component monomial info (<number_of_components> and
<component_number>) from <element_field_values> and puts in
<*component_monomial_info_address> (after allocating storage).

If <*numbers_of_component_values_address> is NULL then storage is allocated for
it and the number of values for each component calculated and entered.
Otherwise, checks that the number of values extracted for each component agrees
with the entries.

If <monomial_values_address> is not NULL and <*monomial_values_address> is NULL,
then storage for the maximum number of component values is allocated and
assigned to <*monomial_values_address>.
==============================================================================*/
{
	FE_value *monomial_values;
	int **component_monomial_info,extract_numbers_of_component_values,i,j,
		maximum_number_of_component_values,number_of_component_values,
		*numbers_of_component_values,return_code;

	ENTER(extract_component_monomial_info);
	return_code=0;
	Assert(element_field_values&&
		(((1==number_of_components)&&(0<=component_number))||
		((0<number_of_components)&&(-1==component_number)))&&(0<number_of_xi)&&
		numbers_of_component_values_address&&component_monomial_info_address,
		std::logic_error("extract_component_monomial_info.  Invalid argument(s)"));
	/* allocate storage for components */
	extract_numbers_of_component_values=
		!(*numbers_of_component_values_address);
	/* allocate storage for components */
	if (extract_numbers_of_component_values)
	{
		ALLOCATE(numbers_of_component_values,int,number_of_components);
	}
	else
	{
		numbers_of_component_values= *numbers_of_component_values_address;
	}
	ALLOCATE(component_monomial_info,int *,number_of_components);
	if (numbers_of_component_values&&component_monomial_info)
	{
		i=0;
		return_code=1;
		maximum_number_of_component_values=0;
		while (return_code&&(i<number_of_components))
		{
			if (ALLOCATE(component_monomial_info[i],int,number_of_xi+1))
			{
				if (-1==component_number)
				{
					return_code=FE_element_field_values_get_monomial_component_info(
						element_field_values,i,component_monomial_info[i]);
				}
				else
				{
					return_code=FE_element_field_values_get_monomial_component_info(
						element_field_values,component_number,component_monomial_info[i]);
				}
				if (return_code)
				{
					number_of_component_values=1;
					for (j=1;j<=component_monomial_info[i][0];j++)
					{
						number_of_component_values *= 1+component_monomial_info[i][j];
					}
					if (number_of_component_values>maximum_number_of_component_values)
					{
						maximum_number_of_component_values=number_of_component_values;
					}
					if (extract_numbers_of_component_values)
					{
						numbers_of_component_values[i]=number_of_component_values;
					}
					else
					{
						if (numbers_of_component_values[i]!=number_of_component_values)
						{
							return_code=0;
						}
					}
				}
				i++;
			}
		}
		if (return_code)
		{
			if (monomial_values_address&&!(*monomial_values_address))
			{
				if ((0<maximum_number_of_component_values)&&ALLOCATE(monomial_values,
					FE_value,maximum_number_of_component_values))
				{
					*monomial_values_address=monomial_values;
				}
				else
				{
					return_code=0;
				}
			}
			if (return_code)
			{
				*numbers_of_component_values_address=numbers_of_component_values;
				*component_monomial_info_address=component_monomial_info;
			}
		}
		if (!return_code)
		{
			while (i>0)
			{
				i--;
				DEALLOCATE(component_monomial_info[i]);
			}
		}
	}
	if (!return_code)
	{
		DEALLOCATE(component_monomial_info);
		if (extract_numbers_of_component_values)
		{
			DEALLOCATE(numbers_of_component_values);
		}
	}
	LEAVE;

	return (return_code);
} /* extract_component_monomial_info */

static int nodal_value_calculate_component_values(
	Variable_finite_element_handle fe_variable,struct FE_field *fe_field,
	int component_number,FE_value fe_time,struct FE_element *element,
	struct FE_node *node,enum FE_nodal_value_type nodal_value_type,int version,
	int *number_of_element_field_nodes_address,
	struct FE_node ***element_field_nodes_address,
	struct FE_element_field_values *element_field_values,
	int *number_of_nodal_values_address,int **numbers_of_component_values_address,
	FE_value ****component_values_address)
/*******************************************************************************
LAST MODIFIED : 29 December 2003

DESCRIPTION :
Calculate the component values for the derivatives of the specified components
(<component_number>) of <fe_field> in the <element> with respect to the
specified nodal values (<node>, <nodal_value_type> and <version>.

<*number_of_nodal_values_address> is calculated and an array with an entry for
each of the specified nodal values (<*number_of_nodal_values_address> long> is
allocated, NULLed and assigned to <*component_values_address>.  For each nodal
value, if the derivative is not identically zero then a 2 dimensional array
(first index is component number and second index is component value number) is
allocated, filled-in and assigned to the corresponding entry in
<*component_values_address>.

The interpolation function for the <fe_field> component(s) is assumed to be a
linear combination of basis functions polynomial in the element coordinates.
The coefficients for the linear combination are the nodal values.  So, the
derivative with respect to a nodal value is the corresponding basis function.
The component values are for the monomials.  To calculate these, all the nodal
values for the <element> are set to zero, the nodal value of interest is set to
one and <calculate_FE_element_field_values> is used.

<*number_of_element_field_nodes_address> and <*element_field_nodes_address> are
for the <fe_field> and <element> and can be passed in or computed in here.
<element_field_values> is working storage that is passed in.

???DB.  Can get all from <fe_variable>.  Do multiple times?
==============================================================================*/
{
	FE_value ***component_values,***nodal_value_component_values;
	int *element_field_nodal_value_offsets,*element_field_number_of_nodal_values,
		*element_field_number_of_specified_nodal_values,i,j,k,number_of_components,
		number_of_element_field_nodes,number_of_nodal_values,
		number_of_saved_element_field_nodes,number_of_values,number_of_versions,
		return_code;
	struct Count_nodal_values_data count_nodal_values_data;
	struct FE_region *fe_region;
	struct FE_node **element_field_nodes;

	ENTER(nodal_value_calculate_component_values);
	return_code=0;
	/* check arguments */
	fe_region=FE_field_get_FE_region(fe_field);
	number_of_components=get_FE_field_number_of_components(fe_field);
	Assert(fe_variable&&fe_field&&fe_region&&(0<number_of_components)&&
		((-1==component_number)||
		((0<=component_number)&&(component_number<number_of_components)))&&
		element&&element_field_values&&number_of_nodal_values_address&&
		component_values_address,std::logic_error(
		"nodal_value_calculate_component_values.  Invalid argument(s)"));
	{
		if (-1!=component_number)
		{
			number_of_components=1;
		}
		if (number_of_element_field_nodes_address&&element_field_nodes_address)
		{
			number_of_element_field_nodes= *number_of_element_field_nodes_address;
			element_field_nodes= *element_field_nodes_address;
		}
		else
		{
			number_of_element_field_nodes=0;
			element_field_nodes=(struct FE_node **)NULL;
		}
		if ((number_of_element_field_nodes>0)&&element_field_nodes)
		{
			return_code=0;
		}
		else
		{
			return_code=calculate_FE_element_field_nodes(element,
				fe_field,&number_of_element_field_nodes,
				&element_field_nodes,(struct FE_element *)NULL);
		}
		if (return_code)
		{
			//???DB.  To be done/Needs finishing
			/* set up working storage */
			Variable_handle *element_field_saved_nodal_values;

			ALLOCATE(element_field_nodal_value_offsets,int,
				number_of_element_field_nodes);
			ALLOCATE(element_field_number_of_specified_nodal_values,int,
				number_of_element_field_nodes);
			ALLOCATE(element_field_number_of_nodal_values,int,
				number_of_element_field_nodes);
			element_field_saved_nodal_values=
				new Variable_handle[number_of_element_field_nodes];
			if (element_field_nodal_value_offsets&&
				element_field_number_of_specified_nodal_values&&
				element_field_number_of_nodal_values&&element_field_saved_nodal_values)
			{
				/* NULL working storage and calculate total number of nodal values for
					each <element_field_node> */
				for (i=0;i<number_of_element_field_nodes;i++)
				{
					element_field_saved_nodal_values[i]=Variable_handle(0);
					element_field_nodal_value_offsets[i]= -1;
					element_field_number_of_specified_nodal_values[i]=0;
					element_field_number_of_nodal_values[i]=0;
					if (-1==component_number)
					{
						for (j=0;j<number_of_components;j++)
						{
							number_of_versions=
								get_FE_node_field_component_number_of_versions(
								element_field_nodes[i],fe_field,j);
							if (version<number_of_versions)
							{
								number_of_values=
									(1+get_FE_node_field_component_number_of_derivatives(
									element_field_nodes[i],fe_field,j));
								if (version<0)
								{
									element_field_number_of_nodal_values[i] +=
										number_of_values*number_of_versions;
								}
								else
								{
									element_field_number_of_nodal_values[i] += number_of_values;
								}
							}
						}
					}
					else
					{
						number_of_versions=
							get_FE_node_field_component_number_of_versions(
							element_field_nodes[i],fe_field,component_number);
						if (version<number_of_versions)
						{
							number_of_values=
								(1+get_FE_node_field_component_number_of_derivatives(
								element_field_nodes[i],fe_field,component_number));
							if (version<0)
							{
								element_field_number_of_nodal_values[i] +=
									number_of_values*number_of_versions;
							}
							else
							{
								element_field_number_of_nodal_values[i] += number_of_values;
							}
						}
					}
				}
				/* calculate total number of specified nodal values (<number_of_values>)
					and the number of specified nodal values for the
					<element_field_nodes> */
				count_nodal_values_data.number_of_values=0;
				count_nodal_values_data.value_type=nodal_value_type;
				count_nodal_values_data.version=version;
				count_nodal_values_data.fe_field=fe_field;
				count_nodal_values_data.component_number=component_number;
				count_nodal_values_data.number_of_components=number_of_components;
				count_nodal_values_data.number_of_node_offsets=
					number_of_element_field_nodes;
				count_nodal_values_data.offset_nodes=element_field_nodes;
				count_nodal_values_data.node_offsets=element_field_nodal_value_offsets;
				count_nodal_values_data.number_of_node_values=
					element_field_number_of_specified_nodal_values;
				if (node)
				{
					return_code=count_nodal_values(node,(void *)&count_nodal_values_data);
				}
				else
				{
					return_code=FE_region_for_each_FE_node(fe_region,
						count_nodal_values,(void *)&count_nodal_values_data);
				}
				number_of_nodal_values=count_nodal_values_data.number_of_values;
				if (return_code&&(0<number_of_nodal_values))
				{
					/* allocate storage for the component_values */
					if (return_code&&ALLOCATE(component_values,FE_value **,
						number_of_nodal_values))
					{
						/* NULL the component values array */
						nodal_value_component_values=component_values;
						for (i=number_of_nodal_values;i>0;i--)
						{
							*nodal_value_component_values=(FE_value **)NULL;
							nodal_value_component_values++;
						}
						/* save and zero all the nodal values for the <fe_field> on the
							element */
						number_of_saved_element_field_nodes=0;
						while (return_code&&(number_of_saved_element_field_nodes<
							number_of_element_field_nodes))
						{
							Variable_input_nodal_values_handle 
								input(new Variable_input_nodal_values(fe_variable,
								element_field_nodes[number_of_saved_element_field_nodes]));

							if (element_field_saved_nodal_values[
								number_of_saved_element_field_nodes]=
								(fe_variable->get_input_value)(input))
							{
								Variable_size_type i,number_of_values=
									(element_field_saved_nodal_values[
									number_of_saved_element_field_nodes])->size();
								Vector zero_vector(number_of_values);

								number_of_saved_element_field_nodes++;
								for (i=0;i<number_of_values;i++)
								{
									zero_vector[i]=(Scalar)0;
								}
								return_code=(fe_variable->set_input_value)(input,
									Variable_vector_handle(new Variable_vector(zero_vector)));
							}
							else
							{
								return_code=0;
							}
						}
						/* calculate component values for nodal value derivatives */
						i=0;
						while (return_code&&(i<number_of_element_field_nodes))
						{
							if ((element_field_nodal_value_offsets[i]>=0)&&
								(element_field_number_of_specified_nodal_values[i]>0))
							{
								Variable_size_type j,number_of_values=(Variable_size_type)
									(element_field_number_of_specified_nodal_values[i]);
								Variable_vector_handle unit_vector(new Variable_vector(
									Vector(number_of_values)));
								Variable_input_nodal_values_handle 
									input(new Variable_input_nodal_values(fe_variable,
									element_field_nodes[i],nodal_value_type,version));

								for (j=0;j<number_of_values;j++)
								{
									(*unit_vector)[j]=(Scalar)0;
								}
								nodal_value_component_values=component_values+
									element_field_nodal_value_offsets[i];
								Assert((int)number_of_values==
									element_field_number_of_specified_nodal_values[i],
									std::logic_error("nodal_value_calculate_component_values.  "
									"Incorrect number of nodal values"));
								j=0;
								while (return_code&&(j<number_of_values))
								{
									/* set nodal values */
									(*unit_vector)[j]=(Scalar)1;
									if ((fe_variable->set_input_value)(input,unit_vector)&&
										clear_FE_element_field_values(element_field_values)&&
										FE_element_field_values_set_no_modify(
										element_field_values)&&
										calculate_FE_element_field_values(element,fe_field,
										fe_time,(char)0,element_field_values,
										(struct FE_element *)NULL))
									{
										return_code=extract_component_values(
											element_field_values,number_of_components,
											component_number,numbers_of_component_values_address,
											nodal_value_component_values);
									}
									else
									{
										return_code=0;
									}
									/* set nodal values */
									(*unit_vector)[j]=(Scalar)0;
									nodal_value_component_values++;
									j++;
								}
								if (return_code)
								{
									// zero the last 1 from the above loop
									return_code=(fe_variable->set_input_value)(input,unit_vector);
								}
								if (!return_code)
								{
									while (j>0)
									{
										j--;
										nodal_value_component_values--;
										if (*nodal_value_component_values)
										{
											for (k=0;k<number_of_components;k++)
											{
												DEALLOCATE((*nodal_value_component_values)[k]);
											}
											DEALLOCATE(*nodal_value_component_values);
										}
									}
								}
							}
							i++;
						}
						if (return_code)
						{
							*number_of_nodal_values_address=number_of_nodal_values;
							*component_values_address=component_values;
						}
						else
						{
							while (i>0)
							{
								i--;
								if ((element_field_nodal_value_offsets[i]>=0)&&
									(element_field_number_of_specified_nodal_values[i]>0))
								{
									nodal_value_component_values=component_values+
										element_field_nodal_value_offsets[i];
									for (j=0;
										j<element_field_number_of_specified_nodal_values[i];j++)
									{
										if (*nodal_value_component_values)
										{
											for (k=0;k<number_of_components;k++)
											{
												DEALLOCATE((*nodal_value_component_values)[k]);
											}
										}
										DEALLOCATE(*nodal_value_component_values);
										nodal_value_component_values++;
									}
								}
							}
							DEALLOCATE(component_values);
						}
						/* reset all the nodal values for the <fe_field> on the element */
						i=0;
						while (return_code&&(i<number_of_saved_element_field_nodes))
						{
							Variable_input_nodal_values_handle 
								input(new Variable_input_nodal_values(fe_variable,
								element_field_nodes[i]));

							return_code=(fe_variable->set_input_value)(input,
								element_field_saved_nodal_values[i]);
							i++;
						}
					}
					else
					{
						return_code=0;
					}
				}
				else
				{
					return_code=0;
				}
				/* destroy working computed values for saving current values and
					setting temporary values */
				for (i=0;i<number_of_element_field_nodes;i++)
				{
					element_field_saved_nodal_values[i]=Variable_vector_handle(0);
				}
			}
			else
			{
				return_code=0;
			}
			/* get rid of working storage */
			delete [] element_field_saved_nodal_values;
			DEALLOCATE(element_field_number_of_nodal_values);
			DEALLOCATE(element_field_number_of_specified_nodal_values);
			DEALLOCATE(element_field_nodal_value_offsets);
		}
		if (return_code&&number_of_element_field_nodes_address&&
			element_field_nodes_address)
		{
			*number_of_element_field_nodes_address=number_of_element_field_nodes;
			*element_field_nodes_address=element_field_nodes;
		}
	}
	LEAVE;

	return (return_code);
} /* nodal_value_calculate_component_values */

static int calculate_monomial_derivative_values(int number_of_xi_coordinates,
	int *xi_orders,int *xi_derivative_orders,FE_value *xi_coordinates,
	FE_value *monomial_derivative_values)
/*******************************************************************************
LAST MODIFIED : 9 November 2003

DESCRIPTION :
For the specified monomials (<number_of_xi_coordinates> and order<=<xi_orders>
for each xi), calculates there derivatives (<xi_derivative_orders>) at
<xi_coordinates> and stores in <monomial_derivative_values>.  Expects the memory
to already be allocated for the <monomial_derivative_values>.
NB.  xi_1 is varying slowest (xi_n fastest)
==============================================================================*/
{
	FE_value *temp_value,*value,xi,*xi_coordinate,xi_power;
	int derivative_order,i,j,k,order,number_of_values,return_code,
		*xi_derivative_order,*xi_order,zero_derivative;

	ENTER(calculate_monomial_derivative_values);
	return_code=0;
	Assert((0<number_of_xi_coordinates)&&(xi_order=xi_orders)&&
		(xi_derivative_order=xi_derivative_orders)&&(xi_coordinate=xi_coordinates)&&
		monomial_derivative_values,std::logic_error(
		"calculate_monomial_derivative_values.  Invalid argument(s)"));
	{
		/* check for zero derivative */
		zero_derivative=0;
		number_of_values=1;
		for (i=number_of_xi_coordinates;i>0;i--)
		{
			order= *xi_order;
			if (*xi_derivative_order>order)
			{
				zero_derivative=1;
			}
			number_of_values *= order+1;
			xi_derivative_order++;
			xi_order++;
		}
		value=monomial_derivative_values;
		if (zero_derivative)
		{
			/* zero derivative */
			for (k=number_of_values;k>0;k--)
			{
				*value=(FE_value)0;
				value++;
			}
		}
		else
		{
			xi_order=xi_orders;
			xi_derivative_order=xi_derivative_orders;
			*value=1;
			number_of_values=1;
			for (i=number_of_xi_coordinates;i>0;i--)
			{
				xi= *xi_coordinate;
				xi_coordinate++;
				derivative_order= *xi_derivative_order;
				xi_derivative_order++;
				order= *xi_order;
				xi_order++;
				if (0<derivative_order)
				{
					xi_power=1;
					for (j=derivative_order;j>1;j--)
					{
						xi_power *= (FE_value)j;
					}
					value += number_of_values*(derivative_order-1);
					for (j=derivative_order;j<=order;j++)
					{
						temp_value=monomial_derivative_values;
						for (k=number_of_values;k>0;k--)
						{
							value++;
							*value=(*temp_value)*xi_power;
							temp_value++;
						}
						xi_power *= xi*(float)(j+1)/(float)(j-derivative_order+1);
					}
					temp_value=monomial_derivative_values;
					for (k=derivative_order*number_of_values;k>0;k--)
					{
						*temp_value=0;
						temp_value++;
					}
				}
				else
				{
					xi_power=xi;
					for (j=order;j>0;j--)
					{
						temp_value=monomial_derivative_values;
						for (k=number_of_values;k>0;k--)
						{
							value++;
							*value=(*temp_value)*xi_power;
							temp_value++;
						}
						xi_power *= xi;
					}
				}
				number_of_values *= (order+1);
			}
		}
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* calculate_monomial_derivative_values */
#endif // defined (USE_ITERATORS)

class Variable_finite_element_check_derivative_functor
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//
// NOTES :
// 1. Composite inputs are already handled by evaluate_derivative.
//
// ???DB.  Only valid for monomial standard basis and nodal value based.  This
//   is enforced by <FE_element_field_values_get_monomial_component_info> and
//   <FE_element_field_values_get_component_values>.
//==============================================================================
{
	public:
		Variable_finite_element_check_derivative_functor(
			Variable_finite_element_handle variable_finite_element,
			bool& zero_derivative,
			Variable_input_nodal_values_handle& nodal_values_input,
			ublas::vector<Variable_input_element_xi_handle>::iterator
			element_xi_input_iterator,Variable_size_type& number_of_columns):
			first(true),zero_derivative(zero_derivative),
			variable_finite_element(variable_finite_element),
			nodal_values_input(nodal_values_input),
			element_xi_input_iterator(element_xi_input_iterator),
			number_of_columns(number_of_columns){};
		~Variable_finite_element_check_derivative_functor(){};
		int operator() (
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			& input)
		{
#if defined (USE_ITERATORS)
			//???DB.  To be done
			//???DB.  May be similar to this because atomic inputs have size 1?
			number_of_columns=1;
#else // defined (USE_ITERATORS)
			if (first)
			{
				number_of_columns=input->size();
			}
			else
			{
				number_of_columns *= input->size();
			}
#endif // defined (USE_ITERATORS)
			*element_xi_input_iterator=Variable_input_element_xi_handle(0);
			if (!zero_derivative)
			{
				Variable_input_element_xi_handle input_element_xi_handle;
				Variable_input_nodal_values_handle input_nodal_values_handle;

				if ((input_element_xi_handle=
#if defined (USE_SMART_POINTER)
					boost::dynamic_pointer_cast<Variable_input_element_xi,
#if defined (USE_VARIABLE_INPUT)
					Variable_input
#else // defined (USE_VARIABLE_INPUT)
					Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
					>(input)
#else /* defined (USE_SMART_POINTER) */
					dynamic_cast<Variable_input_element_xi *>(input)
#endif /* defined (USE_SMART_POINTER) */
					)&&(variable_finite_element==
					input_element_xi_handle->variable_finite_element)&&
					(input_element_xi_handle->xi))
				{
					// not checking order of derivative against order of polynomial
					*element_xi_input_iterator=input_element_xi_handle;
				}
				else if ((input_nodal_values_handle=
#if defined (USE_SMART_POINTER)
					boost::dynamic_pointer_cast<Variable_input_nodal_values,
#if defined (USE_VARIABLE_INPUT)
					Variable_input
#else // defined (USE_VARIABLE_INPUT)
					Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
					>(input)
#else /* defined (USE_SMART_POINTER) */
					dynamic_cast<Variable_input_nodal_values *>(input)
#endif /* defined (USE_SMART_POINTER) */
					)&&(variable_finite_element==
					input_nodal_values_handle->variable_finite_element))
				{
					if (nodal_values_input)
					{
						// because the nodal values are the coefficients for a linear
						//   combination
						zero_derivative=true;
					}
					else
					{
						nodal_values_input=input_nodal_values_handle;
					}
				}
				else
				{
					zero_derivative=true;
				}
				first=false;
			}
			element_xi_input_iterator++;

			return (1);
		};
	private:
		bool first;
		bool& zero_derivative;
		Variable_finite_element_handle variable_finite_element;
		Variable_input_nodal_values_handle& nodal_values_input;
		ublas::vector<Variable_input_element_xi_handle>::iterator
			element_xi_input_iterator;
		Variable_size_type& number_of_columns;
};

#if defined (USE_ITERATORS)
//???DB.  To be done
#else // defined (USE_ITERATORS)
bool Variable_finite_element::evaluate_derivative_local(Matrix& matrix,
	std::list<
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// ???DB.  Throw an exception for failure?
//==============================================================================
{
	bool result,zero_derivative;
	Variable_size_type derivative_order=independent_variables.size(),
		number_of_columns,number_of_components;
	Variable_input_nodal_values_handle nodal_values_input(0);
	ublas::vector<Variable_input_element_xi_handle>
		element_xi_inputs(derivative_order);

	result=true;
	// matrix is zero'd on entry
	// check independent variables for a zero matrix
	number_of_components=this->size();
	zero_derivative=false;
	number_of_columns=0;
	std::for_each(independent_variables.begin(),independent_variables.end(),
		Variable_finite_element_check_derivative_functor(
		Variable_finite_element_handle(this),zero_derivative,
		nodal_values_input,element_xi_inputs.begin(),number_of_columns));
	Assert((number_of_components==matrix.size1())&&
		(number_of_columns==matrix.size2()),
		std::logic_error("Variable_finite_element::evaluate_derivative_local.  "
		"Incorrect matrix size"));
	if (!zero_derivative)
	{
		FE_value **component_values=(FE_value **)NULL,
			*monomial_derivative_values=(FE_value *)NULL,
			***nodal_value_component_values=(FE_value ***)NULL;
		int **component_monomial_info=(int **)NULL,
			*numbers_of_component_values=(int *)NULL,number_of_element_field_nodes=0,
			number_of_nodal_values=0,return_code=0;
		struct FE_element_field_values *element_field_values;
		struct FE_node **element_field_nodes=(struct FE_node **)NULL;
		Variable_size_type number_of_xi=xi.size();

		// set up temporary storage
		element_field_values=CREATE(FE_element_field_values)();
		if (element_field_values)
		{
			if (nodal_values_input)
			{
				if (nodal_value_calculate_component_values(
					Variable_finite_element_handle(this),field,component_number,
					(FE_value)time,element,nodal_values_input->node,
					nodal_values_input->value_type,nodal_values_input->version,
					&number_of_element_field_nodes,&element_field_nodes,
					element_field_values,&number_of_nodal_values,
					&numbers_of_component_values,&nodal_value_component_values)&&
					extract_component_monomial_info(element_field_values,
					number_of_components,component_number,number_of_xi,
					&numbers_of_component_values,&component_monomial_info,
					&monomial_derivative_values))
				{
					return_code=1;
				}
			}
			else
			{
				if (clear_FE_element_field_values(element_field_values)&&
					calculate_FE_element_field_values(element,field,(FE_value)time,
					(char)0,element_field_values,(struct FE_element *)NULL)&&
					extract_component_values(element_field_values,number_of_components,
					component_number,&numbers_of_component_values,&component_values)&&
					extract_component_monomial_info(element_field_values,
					number_of_components,component_number,number_of_xi,
					&numbers_of_component_values,&component_monomial_info,
					&monomial_derivative_values))
				{
					return_code=1;
				}
			}
			if (return_code)
			{
				FE_value *xi_array;
				int *xi_derivative_orders;

				xi_array=new FE_value[number_of_xi];
				xi_derivative_orders=new int[number_of_xi];
				if (xi_array&&xi_derivative_orders)
				{
					bool carry;
					FE_value **derivative_component_values=(FE_value **)NULL,
						*value_address_1,*value_address_2;
					Variable_size_type column_number,i;
					ublas::vector<Variable_size_type>
						derivative_independent_values(derivative_order+1);
					int j;
					Scalar component_value;

					for (i=0;i<=derivative_order;i++)
					{
						derivative_independent_values[i]=0;
					}
					for (i=0;i<number_of_xi;i++)
					{
						xi_array[i]=(FE_value)(xi[i]);
					}
					/* chose the component values to use for calculating the
						values */
					if (nodal_values_input)
					{
						derivative_component_values=nodal_value_component_values[0];
					}
					else
					{
						derivative_component_values=component_values;
					}
					/* loop over the values within the derivative block */
					column_number=0;
					while (return_code&&
						(0==derivative_independent_values[derivative_order]))
					{
						if (derivative_component_values)
						{
							for (i=0;i<number_of_xi;i++)
							{
								xi_derivative_orders[i]=0;
							}
							for (i=0;i<derivative_order;i++)
							{
								if (element_xi_inputs[i])
								{
									// element/xi derivative
									if (0<element_xi_inputs[i]->indices.size())
									{
										xi_derivative_orders[(element_xi_inputs[i]->indices)[
											derivative_independent_values[i]]-1]++;
									}
									else
									{
										xi_derivative_orders[
											derivative_independent_values[i]]++;
									}
								}
							}
							/* loop over components */
							i=0;
							while (return_code&&(i<number_of_components))
							{
								if (return_code=calculate_monomial_derivative_values(
									number_of_xi,component_monomial_info[i]+1,
									xi_derivative_orders,xi_array,monomial_derivative_values))
								{
									/* calculate the derivative */
									component_value=(Scalar)0;
									value_address_1=monomial_derivative_values;
									value_address_2=derivative_component_values[i];
									for (j=numbers_of_component_values[i];j>0;j--)
									{
										component_value +=
											(double)(*value_address_1)*
											(double)(*value_address_2);
										value_address_1++;
										value_address_2++;
									}
									matrix(i,column_number)=component_value;
								}
								i++;
							}
						}
						/* step to next value within derivative block */
						column_number++;
						i=0;
						carry=true;
						do
						{
							derivative_independent_values[i]++;
							if (element_xi_inputs[i])
							{
								Variable_size_type
									number_of_values=(element_xi_inputs[i]->indices).size();

								if (0<number_of_values)
								{
									if (derivative_independent_values[i]>=number_of_values)
									{
										derivative_independent_values[i]=0;
									}
									else
									{
										carry=false;
									}
								}
								else
								{
									if (derivative_independent_values[i]>=number_of_xi)
									{
										derivative_independent_values[i]=0;
									}
									else
									{
										carry=false;
									}
								}
							}
							else
							{
								if (derivative_independent_values[i]>=
									(Variable_size_type)number_of_nodal_values)
								{
									derivative_independent_values[i]=0;
								}
								else
								{
									carry=false;
								}
								// update the component values to use for calculating the values
								derivative_component_values=
									nodal_value_component_values[
									derivative_independent_values[i]];
							}
							i++;
						} while ((i<derivative_order)&&carry);
						if (carry)
						{
							derivative_independent_values[derivative_order]++;
						}
					}
				}
				delete [] xi_array;
				delete [] xi_derivative_orders;
			}
		}
		// remove temporary storage
		if (element_field_values)
		{
			DESTROY(FE_element_field_values)(&element_field_values);
		}
	}

	return (result);
}
#endif // defined (USE_ITERATORS)

enum Swap_nodal_values_type
{
	SWAP_NODAL_VALUES_GET,
	SWAP_NODAL_VALUES_SET,
	SWAP_NODAL_VALUES_SWAP
}; /* enum Swap_nodal_values_type */

struct Swap_nodal_values_data
{
	enum FE_nodal_value_type value_type;
	enum Swap_nodal_values_type swap_type;
	FE_value *values;
	int component_number,number_of_components,number_of_values,value_number,
		version;
	struct FE_field *fe_field;
}; /* struct Swap_nodal_values_data */

static int swap_nodal_values(struct FE_node *node,
	void *swap_nodal_values_data_void)
/*******************************************************************************
LAST MODIFIED : 29 December 2003

DESCRIPTION :
Swaps the nodal values specified by <swap_nodal_values_data_void> at the <node>.
==============================================================================*/
{
	enum FE_nodal_value_type *nodal_value_type,*nodal_value_types,value_type;
	enum Swap_nodal_values_type swap_type;
	FE_value swap_value,*swap_value_1,*swap_value_2,*value,*values;
	int all_components,component_number,i,j,number_of_swap_values,
		number_of_values,number_of_values_per_version,number_of_versions,
		return_code,version;
	struct FE_field *fe_field;
	struct Swap_nodal_values_data *swap_nodal_values_data;

	ENTER(swap_nodal_values);
	return_code=0;
	/* check arguments */
	if (node&&(swap_nodal_values_data=(struct Swap_nodal_values_data *)
		swap_nodal_values_data_void))
	{
		fe_field=swap_nodal_values_data->fe_field;
		swap_type=swap_nodal_values_data->swap_type;
		value_type=swap_nodal_values_data->value_type;
		if (get_FE_nodal_field_FE_value_values(fe_field,node,&number_of_values,
			&values))
		{
			value=values;
			return_code=1;
			component_number=0;
			all_components=(swap_nodal_values_data->component_number<0)||
				(swap_nodal_values_data->component_number>=
				swap_nodal_values_data->number_of_components);
			while (return_code&&(component_number<
				swap_nodal_values_data->number_of_components))
			{
				number_of_versions=get_FE_node_field_component_number_of_versions(node,
					fe_field,component_number);
				version=swap_nodal_values_data->version;
				number_of_values_per_version=
					1+get_FE_node_field_component_number_of_derivatives(node,fe_field,
					component_number);
				if ((version<number_of_versions)&&(all_components||
					(component_number==swap_nodal_values_data->component_number)))
				{
					swap_value_1=(swap_nodal_values_data->values)+
						(swap_nodal_values_data->value_number);
					if (FE_NODAL_UNKNOWN==value_type)
					{
						if (0<=version)
						{
							swap_value_2=value+(version*number_of_values_per_version);
							number_of_swap_values=number_of_values_per_version;
						}
						else
						{
							swap_value_2=value;
							number_of_swap_values=
								number_of_versions*number_of_values_per_version;
						}
						switch (swap_type)
						{
							case SWAP_NODAL_VALUES_GET:
							{
								memcpy(swap_value_1,swap_value_2,
									number_of_swap_values*sizeof(FE_value));
							} break;
							case SWAP_NODAL_VALUES_SET:
							{
								memcpy(swap_value_2,swap_value_1,
									number_of_swap_values*sizeof(FE_value));
							} break;
							case SWAP_NODAL_VALUES_SWAP:
							{
								for (i=number_of_swap_values;i>0;i--)
								{
									swap_value= *swap_value_1;
									*swap_value_1= *swap_value_2;
									*swap_value_2=swap_value;
									swap_value_1++;
									swap_value_2++;
								}
							} break;
						}
						swap_nodal_values_data->value_number += number_of_swap_values;
					}
					else
					{
						if (nodal_value_types=get_FE_node_field_component_nodal_value_types(
							node,fe_field,component_number))
						{
							nodal_value_type=nodal_value_types;
							i=0;
							while ((i<number_of_values_per_version)&&
								(value_type!= *nodal_value_type))
							{
								nodal_value_type++;
								i++;
							}
							if (i<number_of_values_per_version)
							{
								if (0<=version)
								{
									i += version*number_of_values_per_version;
									swap_value_1=(swap_nodal_values_data->values)+
										(swap_nodal_values_data->value_number);
									swap_value_2=value+i;
									switch (swap_type)
									{
										case SWAP_NODAL_VALUES_GET:
										{
											*swap_value_1= *swap_value_2;
										} break;
										case SWAP_NODAL_VALUES_SET:
										{
											*swap_value_2= *swap_value_1;
										} break;
										case SWAP_NODAL_VALUES_SWAP:
										{
											swap_value= *swap_value_1;
											*swap_value_1= *swap_value_2;
											*swap_value_2=swap_value;
										} break;
									}
									(swap_nodal_values_data->value_number)++;
								}
								else
								{
									swap_value_1=(swap_nodal_values_data->values)+
										(swap_nodal_values_data->value_number);
									swap_value_2=value+i;
									switch (swap_type)
									{
										case SWAP_NODAL_VALUES_GET:
										{
											for (j=number_of_versions;j>0;j--)
											{
												*swap_value_1= *swap_value_2;
												swap_value_1++;
												swap_value_2 += number_of_values_per_version;
											}
										} break;
										case SWAP_NODAL_VALUES_SET:
										{
											for (j=number_of_versions;j>0;j--)
											{
												*swap_value_2= *swap_value_1;
												swap_value_1++;
												swap_value_2 += number_of_values_per_version;
											}
										} break;
										case SWAP_NODAL_VALUES_SWAP:
										{
											for (j=number_of_versions;j>0;j--)
											{
												swap_value= *swap_value_1;
												*swap_value_1= *swap_value_2;
												*swap_value_2=swap_value;
												swap_value_1++;
												swap_value_2 += number_of_values_per_version;
											}
										} break;
									}
									swap_nodal_values_data->value_number += number_of_versions;
								}
							}
							DEALLOCATE(nodal_value_types);
						}
						else
						{
							return_code=0;
						}
					}
				}
				value += number_of_values_per_version*number_of_versions;
				component_number++;
			}
			if (return_code&&((SWAP_NODAL_VALUES_SET==swap_type)||
				(SWAP_NODAL_VALUES_SWAP==swap_type)))
			{
				return_code=set_FE_nodal_field_FE_value_values(fe_field,node,values,
					&number_of_values);
			}
			DEALLOCATE(values);
		}
	}
	LEAVE;

	return (return_code);
} /* swap_nodal_values */

Variable_handle Variable_finite_element::get_input_value_local(
	const
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	& input)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	Variable_handle result;
	Variable_input_element_xi_handle input_element_xi_handle;
	Variable_input_nodal_values_handle input_nodal_values_handle;

	result=Variable_vector_handle((Variable_vector *)0);
	if ((input_element_xi_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_element_xi,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_element_xi *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_element_xi_handle->variable_finite_element==
		Variable_finite_element_handle(this)))
	{
		if (input_element_xi_handle->xi)
		{
			Variable_size_type number_of_input_values=
				input_element_xi_handle->
#if defined (USE_ITERATORS)
				number_differentiable
#else // defined (USE_ITERATORS)
				size
#endif // defined (USE_ITERATORS)
				();

			if (0==(input_element_xi_handle->indices).size())
			{
				if (input_element_xi_handle->element)
				{
					result=Variable_handle(new Variable_element_xi(element,xi));
				}
				else
				{
					result=Variable_handle(new Variable_vector(xi));
				}
			}
			else
			{
				Variable_size_type i,index,number_of_values=this->
#if defined (USE_ITERATORS)
					number_differentiable
#else // defined (USE_ITERATORS)
				size
#endif // defined (USE_ITERATORS)
					();
				ublas::vector<Scalar> selected_values(number_of_input_values);

				for (i=0;i<number_of_input_values;i++)
				{
					index=input_element_xi_handle->indices[i];
					if ((0<index)&&(index<=number_of_values))
					{
						selected_values[i]=xi[index-1];
					}
				}
#if defined (OLD_CODE)
				//???DB.  Doesn't make sense to have element and indices
				if (input_element_xi_handle->element)
				{
					result=Variable_handle(new Variable_element_xi(element,
						selected_values));
				}
				else
				{
#endif // defined (OLD_CODE)
					result=Variable_handle(new Variable_vector(selected_values));
#if defined (OLD_CODE)
				}
#endif // defined (OLD_CODE)
			}
		}
		else
		{
			if (input_element_xi_handle->element)
			{
				result=Variable_handle(new Variable_element_xi(element,Vector(0)));
			}
			else
			{
				result=Variable_handle(new Variable_vector(Vector(0)));
			}
		}
	}
	else if ((input_nodal_values_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_nodal_values,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_nodal_values *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_nodal_values_handle->variable_finite_element==
		Variable_finite_element_handle(this)))
	{
		int return_code;
		Variable_size_type number_of_values=input_nodal_values_handle->
#if defined (USE_ITERATORS)
			number_differentiable
#else // defined (USE_ITERATORS)
			size
#endif // defined (USE_ITERATORS)
			();
		struct FE_region *region;
		struct Swap_nodal_values_data swap_nodal_values_data;

		if (field&&(region=this->region())&&(0<number_of_values))
		{
			swap_nodal_values_data.swap_type=SWAP_NODAL_VALUES_GET;
			swap_nodal_values_data.number_of_values=0;
			swap_nodal_values_data.value_type=input_nodal_values_handle->value_type;
			swap_nodal_values_data.version=input_nodal_values_handle->version;
			swap_nodal_values_data.fe_field=field;
			swap_nodal_values_data.component_number=component_number;
			swap_nodal_values_data.number_of_components=
				get_FE_field_number_of_components(field);
			swap_nodal_values_data.value_number=0;
			if (ALLOCATE(swap_nodal_values_data.values,FE_value,number_of_values))
			{
				if (input_nodal_values_handle->node)
				{
					return_code=swap_nodal_values(input_nodal_values_handle->node,
						(void *)&swap_nodal_values_data);
				}
				else
				{
					return_code=FE_region_for_each_FE_node(region,swap_nodal_values,
						(void *)&swap_nodal_values_data);
				}
				if (return_code)
				{
					Variable_vector_handle values_vector;

					values_vector=Variable_vector_handle(new Variable_vector(Vector(
						number_of_values)));
					if (values_vector)
					{
						Variable_size_type i;

						for (i=0;i<number_of_values;i++)
						{
							(*values_vector)[i]=(swap_nodal_values_data.values)[i];
						}
						result=values_vector;
					}
				}
				DEALLOCATE(swap_nodal_values_data.values);
			}
		}
	}

	return (result);
}

bool Variable_finite_element::set_input_value_local(
	const
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
	& values)
//******************************************************************************
// LAST MODIFIED : 9 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Variable_input_element_xi_handle input_element_xi_handle;
	Variable_input_nodal_values_handle input_nodal_values_handle;

	result=false;
	if ((input_element_xi_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_element_xi,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_element_xi *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_element_xi_handle->variable_finite_element==
		Variable_finite_element_handle(this)))
	{
		if (input_element_xi_handle->element)
		{
			// no indices if have element
			Variable_element_xi_handle values_element_xi;

			if (values_element_xi=
#if defined (USE_SMART_POINTER)
				boost::dynamic_pointer_cast<Variable_element_xi,Variable>(values)
#else /* defined (USE_SMART_POINTER) */
				dynamic_cast<Variable_element_xi *>(values)
#endif /* defined (USE_SMART_POINTER) */
				)
			{
				if (input_element_xi_handle->xi)
				{
					xi.resize((values_element_xi->xi).size());
					xi=values_element_xi->xi;
				}
				if (element)
				{
					DEACCESS(FE_element)(&element);
				}
				if (values_element_xi->element)
				{
					element=ACCESS(FE_element)(values_element_xi->element);
				}
				result=true;
			}
		}
		else
		{
			Vector *values_vector;

			if (values_vector=
#if defined (USE_ITERATORS)
				//???DB.  To be done
				0
#else // defined (USE_ITERATORS)
				values->scalars()
#endif // defined (USE_ITERATORS)
				)
			{
				Variable_size_type
					number_of_values=input_element_xi_handle->indices.size(),
					number_of_xi=xi.size();

				if (0<number_of_values)
				{
					Variable_size_type i,index;

					for (i=0;i<number_of_values;i++)
					{
						index=input_element_xi_handle->indices[i];
						if (index<number_of_xi)
						{
							xi[index]=(*values_vector)[i];
						}
					}
				}
				else
				{
					if (number_of_xi==values_vector->size())
					{
						xi=(*values_vector);
					}
				}
				delete values_vector;
				result=true;
			}
		}
	}
	else if ((input_nodal_values_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_nodal_values,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_nodal_values *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_nodal_values_handle->variable_finite_element==
		Variable_finite_element_handle(this)))
	{
		Variable_size_type i,number_of_values=input_nodal_values_handle->
#if defined (USE_ITERATORS)
			number_differentiable
#else // defined (USE_ITERATORS)
			size
#endif // defined (USE_ITERATORS)
			();
		struct FE_region *region;
		struct Swap_nodal_values_data swap_nodal_values_data;
		Vector *values_vector;

		if (field&&(region=this->region())&&(0<number_of_values)&&
			(number_of_values==values->
#if defined (USE_ITERATORS)
			number_differentiable
#else // defined (USE_ITERATORS)
			size
#endif // defined (USE_ITERATORS)
			())&&(values_vector=
#if defined (USE_ITERATORS)
			//???DB.  To be done
			0
#else // defined (USE_ITERATORS)
			values->scalars()
#endif // defined (USE_ITERATORS)
			))
		{
			swap_nodal_values_data.swap_type=SWAP_NODAL_VALUES_SET;
			swap_nodal_values_data.number_of_values=0;
			swap_nodal_values_data.value_type=input_nodal_values_handle->value_type;
			swap_nodal_values_data.version=input_nodal_values_handle->version;
			swap_nodal_values_data.fe_field=field;
			swap_nodal_values_data.component_number=component_number;
			swap_nodal_values_data.number_of_components=
				get_FE_field_number_of_components(field);
			swap_nodal_values_data.value_number=0;
			if (ALLOCATE(swap_nodal_values_data.values,FE_value,number_of_values))
			{
				for (i=0;i<number_of_values;i++)
				{
					(swap_nodal_values_data.values)[i]=(FE_value)((*values_vector)[i]);
				}
				if (input_nodal_values_handle->node)
				{
					swap_nodal_values(input_nodal_values_handle->node,
						(void *)&swap_nodal_values_data);
				}
				else
				{
					FE_region_for_each_FE_node(region,swap_nodal_values,
						(void *)&swap_nodal_values_data);
				}
				DEALLOCATE(swap_nodal_values_data.values);
				result=true;
			}
			delete values_vector;
		}
	}

	return (result);
}

string_handle Variable_finite_element::get_string_representation_local()
//******************************************************************************
// LAST MODIFIED : 10 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		if (field)
		{
			out << get_FE_field_name(field);
		}
		if (-1!=component_number)
		{
			out << "(" << component_number << ")";
		}
		*return_string=out.str();
	}

	return (return_string);
}
