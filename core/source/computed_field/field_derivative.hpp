/**
 * FILE : field_derivative.hpp
 *
 * Field derivative defining order and type of derivative operator to apply
 * to any given field. Each Field_derivative has a unique index within its
 * owning region for efficient look up in field cache. Object describes how to
 * evaluate derivative, including links to next lower Field_derivative so
 * can evaluate downstream derivatives using rules.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (__FIELD_DERIVATIVE_HPP__)
#define __FIELD_DERIVATIVE_HPP__

#include "opencmiss/zinc/types/regionid.h"

class Field_derivative
{
	friend struct cmzn_region;
public:
	// enumeration of all derivative types 
	enum Type
	{
		TYPE_INVALID = 0,
		TYPE_ELEMENT_XI = 1
	};

protected:
	cmzn_region *region;  // non-accessed pointer to owning region
	Field_derivative *lower_derivative;  // next lower field derivative, accessed
	int cache_index;  // unique index of values in field value cache for region; 0 == value
	int order;  // total order of derivative
	Type type;  // store for efficient type checking
	int access_count;

	Field_derivative(int order_in, Type type_in) :
		region(0),
		lower_derivative(0),
		cache_index(-1),
		order(order_in),
		type(type_in),
		access_count(1)
	{}

private:
	Field_derivative();  // not implemented for abstract base class

	// should only be called by owning region
	void set_cache_index(int cache_index_in)
	{
		this->cache_index = cache_index_in;
	}

	// should only be called by owning region; take over access of argument
	void set_lower_derivative(Field_derivative *lower_derivative_in)
	{
		this->lower_derivative = lower_derivative_in;
	}

	// should only be called by owning region
	void set_region(cmzn_region *region_in)
	{
		this->region = region_in;
	}

public:

	// abstract virtual destructor declaration
	virtual ~Field_derivative();

	int get_cache_index() const
	{
		return this->cache_index;
	}

	/** @return  Non-accessed lower derivative or nullptr if none == value. */
	Field_derivative *get_lower_derivative() const
	{
		return this->lower_derivative;
	}

	int get_order() const
	{
		return this->order;
	}

	cmzn_region *get_region() const
	{
		return this->region;
	}

	/** @return  Number of individually evaluatable terms. 0 if variable */
	virtual int get_term_count() const = 0;

	Type get_type() const
	{
		return this->type;
	}

	Field_derivative *access()
	{
		++access_count;
		return this;
	}

	static int deaccess(Field_derivative* &field_derivative);
};

class Field_derivative_element_xi : public Field_derivative
{
private:
	int element_dimension;  // revise when mesh objects properly exist

public:
	Field_derivative_element_xi(int element_dimension_in, int order_in) :
		Field_derivative(order_in, TYPE_ELEMENT_XI),
		element_dimension(element_dimension_in)
	{
	}

	int get_element_dimension() const
	{
		return this->element_dimension;
	}

	virtual int get_term_count() const
	{
		int term_count = this->element_dimension;
		for (int d = 1; d < this->element_dimension; ++d)
			term_count *= this->element_dimension;
		return term_count;
	}

};

#endif /* !defined (__FIELD_DERIVATIVE_HPP__) */
