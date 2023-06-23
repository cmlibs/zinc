/**
 * FILE : finite_element_mesh_field_ranges.hpp
 *
 * Caches field ranges in elements, owned by FE_mesh.
 */
 /* Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_MESH_FIELD_RANGES_HPP)
#define FINITE_ELEMENT_MESH_FIELD_RANGES_HPP

#include "cmlibs/zinc/types/fieldcacheid.h"
#include "cmlibs/zinc/types/fieldid.h"
#include "cmlibs/zinc/types/meshid.h"
#include "cmlibs/zinc/zincconfigure.h"
#include "datastore/labels.hpp"
#include "finite_element/finite_element_domain.hpp"
#include "general/block_array.hpp"
#include <map>
#include <atomic>
#include <mutex>


class FE_mesh;
class FeMeshFieldRangesCache;

class FeElementFieldRange
{
public:
	FE_value *ranges;  // minimums, maximums

	FeElementFieldRange(int componentsCount, FE_value *minimums, FE_value *maximums) :
		ranges(new FE_value[componentsCount * 2])
	{
		for (int i = 0; i < componentsCount; ++i)
		{
			this->ranges[i] = minimums[i];
			this->ranges[i + componentsCount] = maximums[i];
		}
	}

	/** copy constructor requires componentsCount */
	FeElementFieldRange(int componentsCount, const FeElementFieldRange& source) :
		ranges(new FE_value[static_cast<size_t>(componentsCount) * 2])
	{
		for (int i = 0; i < componentsCount * 2; ++i)
		{
			this->ranges[i] = source.ranges[i];
		}
	}

	~FeElementFieldRange()
	{
		delete[] this->ranges;
	}

	/** enlarge range to fit source */
	void enlarge(int componentsCount, const FeElementFieldRange& source)
	{
		for (int i = 0; i < componentsCount; ++i)
		{
			if (this->ranges[i] > source.ranges[i])
			{
				this->ranges[i] = source.ranges[i];
			}
		}
		for (int i = componentsCount; i < componentsCount * 2; ++i)
		{
			if (this->ranges[i] < source.ranges[i])
			{
				this->ranges[i] = source.ranges[i];
			}
		}
	}

	/** @return  Maximum value of any component (maximum - minimum). */
	FE_value getMaxRange(int componentsCount)
	{
		const FE_value *minimums = this->ranges;
		const FE_value *maximums = minimums + componentsCount;
		FE_value maxRange = 0.0;
		for (int i = 0; i < componentsCount; ++i)
		{
			const FE_value componentRange = maximums[i] - minimums[i];
			if (maxRange < componentRange)
			{
				maxRange = componentRange;
			}
		}
		return maxRange;
	}

	/** @return True if values in range with tolerance, otherwise false. */
	bool valuesInRange(int componentsCount, const FE_value *values, FE_value tolerance) const
	{
		const FE_value *minimums = this->ranges;
		const FE_value *maximums = minimums + componentsCount;
		for (int i = 0; i < componentsCount; ++i)
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
	cmzn_mesh_group* meshGroup;  // optional mesh group this is for, not accessed
	// Note: owns and frees FeElementFieldRange objects only if no fieldElementGroup
	block_array<DsLabelIndex, const FeElementFieldRange *> elementFieldRanges;
	FeElementFieldRange *totalRange;  // total range of whole mesh, if valid
	FE_value tolerance;  // a fraction of the totalRange to cover approximation in each element
	bool evaluated;  // true if ranges have been evaluate (but client must check field has not been modified)
	std::atomic_int access_count;

public:

	FeMeshFieldRanges(FeMeshFieldRangesCache* meshFieldRangesCacheIn, cmzn_mesh_group* meshGroupIn);

	~FeMeshFieldRanges();

	// only set by single thread making modification
	void clearRanges();

	/** Clear range for a single element, usually when destroyed */
	void clearElementRange(DsLabelIndex elementIndex);

	FeMeshFieldRanges *access()
	{
		++this->access_count;
		return this;
	}

	static void deaccess(FeMeshFieldRanges*& meshFieldRanges);

	/** Called when owning cache is destroyed. Clears ranges and pointer to owning cache pointer. */
	void detachFromCache();

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

	/** Set the element field range for element index.
	 * Expects no existing element field range for the element index.
	 * If no fieldElementGroup, ownership and cleanup are passed to this object.
	 * @return  True on success, false on failure. */
	bool setElementFieldRange(DsLabelIndex elementIndex, const FeElementFieldRange *elementFieldRange)
	{
		return this->elementFieldRanges.setValue(elementIndex, elementFieldRange);
	}

	/** @return  Optional mesh group the ranges are for */
	cmzn_mesh_group* getMeshGroup() const
	{
		return this->meshGroup;
	}

	/** @return  Small tolerance for testing whether in element range */
	FE_value getTolerance() const
	{
		return this->tolerance;
	}

	void setTotalRange(int componentsCount, FeElementFieldRange *totalRangeIn)
	{
		delete this->totalRange;
		this->tolerance = 0.0;
		this->totalRange = totalRangeIn;
		if (this->totalRange)
		{
			this->tolerance = 1.0E-2 * this->totalRange->getMaxRange(componentsCount);
		}
		else
		{
			this->tolerance = 0.0;
		}
	}

};


/**
 * Caches of ranges of elements for field on mesh, for whole mesh or groups.
 */
class FeMeshFieldRangesCache : public FE_domain_mapper
{
	FE_mesh* feMesh;  // not accessed
	cmzn_field* field;  // not accessed
	FeMeshFieldRanges* masterRanges;  // all currently calculated ranges, or whole mesh if all evaluated
	std::map<cmzn_mesh_group*, FeMeshFieldRanges*> groupRanges;  // ranges for particular mesh groups
	// because cache is shared between threads, must lock evaluateMutex when evaluating ranges
	std::mutex evaluateMutex;
	int access_count;

public:
	FeMeshFieldRangesCache(FE_mesh *feMeshIn, cmzn_field *fieldIn);

	virtual ~FeMeshFieldRangesCache();

	/** Clear all ranges, but do not remove them. */
	void clearAllRanges();

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
	void evaluateMeshFieldRanges(cmzn_fieldcache& fieldcache, FeMeshFieldRanges *meshFieldRanges);

	cmzn_field *getField() const
	{
		return this->field;
	}

	/** @return  Element labels object if still attached to mesh, otherwise nullptr */
	const DsLabels* getLabels() const;

	/** @return  Owning mesh or nullptr if detached (mesh destroyed) */
	FE_mesh* getFeMesh() const
	{
		return this->feMesh;
	}

	/** @return  Accessed ranges for element group or whole mesh.
	 * @param meshIn  Mesh or mesh group ranges are for.
	 */
	FeMeshFieldRanges* getMeshFieldRanges(cmzn_mesh* meshIn);

	/** Called by ~FeMeshFieldRanges to remove from parent */
	void removeMeshFieldRanges(FeMeshFieldRanges* meshFieldRanges);

	/** notification from parent FE_mesh that all elements have been destroyed: clear ranges */
	virtual void destroyedAllObjects();

	/** notification from parent FE_mesh that an element has been destroyed: remove its range */
	virtual void destroyedObject(DsLabelIndex destroyedIndex);

	/** notification from parent FE_mesh that a group of elements has been destroyed: remove their ranges */
	virtual void destroyedObjectGroup(const DsLabelsGroup& destroyedLabelsGroup);
};

#endif /* !defined (FINITE_ELEMENT_MESH_FIELD_RANGES_HPP) */
