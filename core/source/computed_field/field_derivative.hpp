/**
 * FILE : field_derivative.hpp
 *
 * Field derivative defining order and type of derivative operator to apply
 * to any given field. Each FieldDerivative has a unique index within its
 * owning region for efficient look up in field cache. Object describes how to
 * evaluate derivative, including links to next lower FieldDerivative so
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

class FE_mesh;

class FieldDerivative
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
	FieldDerivative *lowerDerivative;  // next lower field derivative, accessed
	int cacheIndex;  // unique index of values in field value cache for region; 0 == value
	int order;  // total order of derivative
	Type type;  // store for efficient type checking
	int access_count;

	/**
	 * @param region  Owning region for this derivative.
	 * @param lowerDerivative  Must be supplied if orderIn > 1.
	 */
	FieldDerivative(cmzn_region *regionIn, int orderIn, Type typeIn, FieldDerivative *lowerDerivative);

private:
	FieldDerivative(); // not implemented
	FieldDerivative(const FieldDerivative &source); // not implemented
	FieldDerivative& operator=(const FieldDerivative &source); // not implemented

	// should only be called by owning region
	// default arguments clear the region and cache index
	void setRegionAndCacheIndex(cmzn_region *regionIn=nullptr, int cacheIndexIn=-1)
	{
		this->region = regionIn;
		this->cacheIndex = cacheIndexIn;
	}

public:

	// abstract virtual destructor declaration
	virtual ~FieldDerivative();

	int getCacheIndex() const
	{
		return this->cacheIndex;
	}

	/** @return  Non-accessed lower derivative or nullptr if none == value. */
	const FieldDerivative *getLowerDerivative() const
	{
		return this->lowerDerivative;
	}

	int getOrder() const
	{
		return this->order;
	}

	/** @return  Non-accessed owning region, or nullptr if invalid. */
	cmzn_region *getRegion() const
	{
		return this->region;
	}

	/** @return  Number of individually evaluatable terms. 0 if variable */
	virtual int getTermCount() const = 0;

	Type getType() const
	{
		return this->type;
	}

	FieldDerivative *access()
	{
		++access_count;
		return this;
	}

	static int deaccess(FieldDerivative* &field_derivative);
};

/** Derivative w.r.t. mesh element xi */
class FieldDerivativeMesh : public FieldDerivative
{
	friend class FE_mesh;
private:
	FE_mesh *mesh;  // not accessed as it owns this object
	const int elementDimension;

	void clearMesh()
	{
		this->mesh = nullptr;
	}

	FieldDerivativeMesh(); // not implemented
	FieldDerivativeMesh(const FieldDerivativeMesh &source); // not implemented
	FieldDerivativeMesh& operator=(const FieldDerivativeMesh &source); // not implemented

public:
	FieldDerivativeMesh(FE_mesh *meshIn, int orderIn, FieldDerivative *lowerDerivative);

	int getElementDimension() const
	{
		return this->elementDimension;
	}

	/** @return  Non-accessed FE_mesh *, or nullptr if invalidated */
	FE_mesh *getMesh() const
	{
		return this->mesh;
	}

	virtual int getTermCount() const
	{
		int termCount = this->elementDimension;
		for (int d = 1; d < this->order; ++d)
			termCount *= this->elementDimension;
		return termCount;
	}

};

#endif /* !defined (__FIELD_DERIVATIVE_HPP__) */
