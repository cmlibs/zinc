/**
 * FILE : finite_element_mesh_field_ranges.hpp
 *
 * Caches field ranges in elements, owned by FE_mesh.
 */
 /* OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_MESH_FIELD_RANGES_HPP)
#define FINITE_ELEMENT_MESH_FIELD_RANGES_HPP

#include "opencmiss/zinc/types/fieldcacheid.h"
#include "opencmiss/zinc/types/fieldid.h"
#include "opencmiss/zinc/types/fieldsubobjectgroupid.h"
#include "opencmiss/zinc/zincconfigure.h"
#include "datastore/labels.hpp"
#include "general/block_array.hpp"
#include <map>
#include <atomic>
#include <mutex>


class FE_mesh;
class FeMeshFieldRangesCache;


class FeElementFieldRange
{
public:
	const int componentsCount;
	FE_value *ranges;  // minimums, maximums

	FeElementFieldRange(int componentsCountIn, FE_value *minimums, FE_value *maximums) :
		componentsCount(componentsCountIn),
		ranges(new FE_value[componentsCountIn * 2])
	{
		for (int i = 0; i < componentsCountIn; ++i)
		{
			this->ranges[i] = minimums[i];
			this->ranges[i + componentsCountIn] = maximums[i];
		}
	}

	~FeElementFieldRange()
	{
		delete[] this->ranges;
	}

	/** @return True if values in range with tolerance, otherwise false. */
	bool valuesInRange(FE_value *values, FE_value tolerance = 0.0) const
	{
		const FE_value *minimums = this->ranges;
		const FE_value *maximums = minimums + this->componentsCount;
		for (int i = 0; i < this->componentsCount; ++i)
		{
			if (values[i] < (minimums[i] - tolerance))
			{
				return false;
			}
			if (values[i] > (maximums[i] + tolerance))
			{
				return false;
			}
		}
		return true;
	}

};


/**
 * Ranges of elements for field on mesh for a particular group or the whole mesh.
 * Note ranges are invalid if field has modifications; client must check this.
 */
class FeMeshFieldRanges
{
	FeMeshFieldRangesCache *meshFieldRangesCache;  // owner, not accessed
	cmzn_field_element_group *fieldElementGroup;  // optional group this is for, not accessed
	// Note: owns and frees FeElementFieldRange objects only if no fieldElementGroup
	block_array<DsLabelIndex, const FeElementFieldRange *> elementFieldRanges;
	bool evaluated;  // true if ranges have been evaluate (but client must check field has not been modified)
	std::atomic_int access_count;

public:

	FeMeshFieldRanges(FeMeshFieldRangesCache *meshFieldRangesCacheIn, cmzn_field_element_group *fieldElementGroupIn);

	~FeMeshFieldRanges();

	void clearRanges();

	FeMeshFieldRanges *access()
	{
		++this->access_count;
		return this;
	}

	static void deaccess(FeMeshFieldRanges*& meshFieldRanges);

	/** Called when owning cache is destroyed. Clears ranges and pointer to owning cache pointer. */
	void detachFromCache();

	/** Client must call before using cache */
	void checkEvaluate()
	{
		if (!this->evaluated)
		{
			this->doEvaluate();
		}
	}

	/** Client must check before using cache, otherwise call parent cache evaluate */
	bool isEvaluated() const
	{
		return this->evaluated;
	}

	// only set by mutex-protected evaluate code
	void setEvaluated()
	{
		this->evaluated = true;
	}

	/** @return  Element field range or nullptr if none */
	const FeElementFieldRange *getElementFieldRange(DsLabelIndex elementIndex) const
	{
		return this->elementFieldRanges.getValue(elementIndex);
	}

	/** @return  Optional element group this is for */
	cmzn_field_element_group *getFieldElementGroup() const
	{
		return this->fieldElementGroup;
	}

	/** Set the element field range for element index.
	 * Expects no existing element field range for the element index.
	 * If no fieldElementGroup, ownership and cleanup are passed to this object.
	 * @return  True on success, false on failure. */
	bool setElementFieldRange(DsLabelIndex elementIndex, const FeElementFieldRange *elementFieldRange)
	{
		return this->elementFieldRanges.setValue(elementIndex, elementFieldRange);
	}

private:

	void doEvaluate();
};


/**
 * Caches of ranges of elements for field on mesh, for whole mesh or groups.
 */
class FeMeshFieldRangesCache
{
	FE_mesh *mesh;  // not accessed
	cmzn_field *field;  // not accessed
	FeMeshFieldRanges *masterRanges;  // all currently calculated ranges, or whole mesh if all evaluated
	std::map<cmzn_field_element_group *, FeMeshFieldRanges *> groupRanges;  // ranges for particular groups
	// because cache is shared between threads, must lock evaluateMutex when evaluating ranges
	std::mutex evaluateMutex;
	int access_count;

public:
	FeMeshFieldRangesCache(FE_mesh *meshIn, cmzn_field *fieldIn);

	~FeMeshFieldRangesCache();

	/** Clear all ranges, but do not remove them. */
	void clearRanges();

	FeMeshFieldRangesCache *access()
	{
		++this->access_count;
		return this;
	}

	static void deaccess(FeMeshFieldRangesCache*& meshFieldRangesCache);

	/** Called when owning mesh is destroyed. Clears ranges and owning mesh pointer. */
	void detachFromMesh();

	/** Evaluate ranges of all elements for mesh field ranges and mark as evaluated.
	 * @param fieldcache  Field */
	void evaluateMeshFieldRanges(cmzn_fieldcache *fieldcache, FeMeshFieldRanges *meshFieldRanges);

	cmzn_field *getField() const
	{
		return this->field;
	}

	/** @return  Element labels object if still attached to mesh, otherwise nullptr */
	const DsLabels *getLabels() const;

	/** @return  Accessed ranges for element group or whole mesh.
	 * @param elementGroup  Optional element group field; if nullptr use whole mesh.
	 */
	FeMeshFieldRanges *getMeshFieldRanges(cmzn_field_element_group *elementGroup);

	/** Called by ~FeMeshFieldRanges to remove from parent */
	void removeMeshFieldRanges(FeMeshFieldRanges *meshFieldRanges);
};

#endif /* !defined (FINITE_ELEMENT_MESH_FIELD_RANGES_HPP) */
