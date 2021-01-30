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
private:
	cmzn_region *region;  // non-accessed pointer to owning region
	FieldDerivative *lowerDerivative;  // next lower field derivative, accessed
	int cacheIndex;  // unique index of values in field value cache for region; 0 == value
	//int order;  // total order of derivative
	FE_mesh *mesh;  // non-accessed as mesh manages FieldDerivative with mesh
	int meshDimension;  // cached from mesh
	int meshOrder;  // order of derivatives w.r.t. mesh chart
	int access_count;

	/**
	 * @param region  Owning region for this derivative.
	 * @param meshIn  Mesh for chart derivative, or nullptr if none.
	 * @param lowerDerivative  Must be supplied if orderIn > 1.
	 */
	FieldDerivative(cmzn_region *regionIn, FE_mesh *meshIn, int meshOrderIn, FieldDerivative *lowerDerivativeIn);

	~FieldDerivative();

	FieldDerivative();  // not implemented
	FieldDerivative(const FieldDerivative &source);  // not implemented
	FieldDerivative& operator=(const FieldDerivative &source);  // not implemented

public:

	/** Create derivative w.r.t. mesh chart.
	 * @param mesh  Mesh whose chart is to be derived from.
	 * @param lowerDerivative  Optional lower field derivative to differentiate
	 * further. Note: may only be a derivative w.r.t. the same mesh if any.
	 * @return 
	 */
	static FieldDerivative *createMeshDerivative(FE_mesh *mesh, FieldDerivative *lowerDerivative);

	FieldDerivative *access()
	{
		++access_count;
		return this;
	}

	static int deaccess(FieldDerivative* &field_derivative);

	/** Only to be called by mesh on destruction */
	void clearMeshPrivate()
	{
		this->mesh = nullptr;
		this->meshDimension = 0;
		this->meshOrder = 0;
	}

	// should only be called by owning region
	// default arguments clear the region and cache index
	void setRegionAndCacheIndexPrivate(cmzn_region *regionIn = nullptr, int cacheIndexIn = -1)
	{
		this->region = regionIn;
		this->cacheIndex = cacheIndexIn;
	}

	int getCacheIndex() const
	{
		return this->cacheIndex;
	}

	/** @return  Non-accessed lower derivative or nullptr if none == value. */
	const FieldDerivative *getLowerDerivative() const
	{
		return this->lowerDerivative;
	}

	/** @return  Non-accessed mesh if derivative w.r.t. mesh chart, otherwise nullptr */
	FE_mesh *getMesh() const
	{
		return this->mesh;
	}

	int getMeshDimension() const
	{
		return this->meshDimension;
	}

	/** Get order of derivative w.r.t. mesh, >= 0 */
	int getMeshOrder() const
	{
		return this->meshOrder;
	}

	bool isMeshOnly() const
	{
		return true;  // until parameter derivatives added
	}

	/** @return  Number of individually evaluatable terms for mesh derivative part */
	int getMeshTermCount() const;

	/** @return  Non-accessed owning region, or nullptr if invalid. */
	cmzn_region *getRegion() const
	{
		return this->region;
	}

};

#endif /* !defined (__FIELD_DERIVATIVE_HPP__) */
