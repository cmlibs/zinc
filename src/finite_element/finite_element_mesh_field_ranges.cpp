/**
 * FILE : finite_element_mesh_field_ranges.cpp
 *
 * Caches field ranges in elements, owned by FE_mesh.
 */
 /* OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "opencmiss/zinc/region.h"
#include "computed_field/field_cache.hpp"
#include "computed_field/field_range.hpp"
#include "computed_field/computed_field_subobject_group.hpp"
#include "datastore/labelsgroup.hpp"
#include "finite_element/finite_element_mesh.hpp"
#include "finite_element/finite_element_mesh_field_ranges.hpp"
#include "finite_element/finite_element_region_private.h"
#include "region/cmiss_region.hpp"
#include <vector>


FeMeshFieldRanges::FeMeshFieldRanges(FeMeshFieldRangesCache *meshFieldRangesCacheIn, cmzn_field_element_group *fieldElementGroupIn) :
	meshFieldRangesCache(meshFieldRangesCacheIn),
	fieldElementGroup(fieldElementGroupIn),
	evaluated(false),
	totalRange(nullptr),
	tolerance(0.0),
	access_count(1)
{
}

FeMeshFieldRanges::~FeMeshFieldRanges()
{
	this->clearRanges();
	// cache is null if already destroyed
	if (this->meshFieldRangesCache)
	{
		this->meshFieldRangesCache->removeMeshFieldRanges(this);
	}
}

void FeMeshFieldRanges::clearRanges()
{
	if (!this->fieldElementGroup)
	{
		if (this->meshFieldRangesCache)
		{
			const DsLabels *labels = this->meshFieldRangesCache->getLabels();
			if (labels)
			{
				// only the master group is responsible for freeing range objects
				const DsLabelIndex indexLimit = labels->getIndexSize();
				for (DsLabelIndex index = 0; index < indexLimit; ++index)
				{
					const FeElementFieldRange *elementFieldRange = this->elementFieldRanges.getValue(index);
					delete elementFieldRange;
				}
			}
		}
	}
	this->elementFieldRanges.clear();
	delete this->totalRange;
	this->totalRange = nullptr;
	this->tolerance = 0.0;
	this->evaluated = false;
}

void FeMeshFieldRanges::deaccess(FeMeshFieldRanges*& meshFieldRanges)
{
	if (meshFieldRanges)
	{
		--(meshFieldRanges->access_count);
		if (meshFieldRanges->access_count <= 0)
		{
			delete meshFieldRanges;
		}
		meshFieldRanges = nullptr;
	}
}

void FeMeshFieldRanges::detachFromCache()
{
	this->clearRanges();
	this->meshFieldRangesCache = nullptr;
}


FeMeshFieldRangesCache::FeMeshFieldRangesCache(FE_mesh *meshIn, cmzn_field *fieldIn) :
	mesh(meshIn),
	field(fieldIn),
	masterRanges(new FeMeshFieldRanges(this, nullptr)),
	access_count(1)
{
}

FeMeshFieldRangesCache::~FeMeshFieldRangesCache()
{
	// important: detachFromCache also clears it
	for (std::map<cmzn_field_element_group *, FeMeshFieldRanges *>::iterator iter = this->groupRanges.begin();
		iter != this->groupRanges.end(); ++iter)
	{
		iter->second->detachFromCache();
	}
	this->masterRanges->detachFromCache();
	FeMeshFieldRanges::deaccess(this->masterRanges);
	// mesh is null if already destroyed
	if (this->mesh)
	{
		this->mesh->removeMeshFieldRangesCache(this);
	}
}

void FeMeshFieldRangesCache::clearRanges()
{
	for (std::map<cmzn_field_element_group *, FeMeshFieldRanges *>::iterator iter = this->groupRanges.begin();
		iter != this->groupRanges.end(); ++iter)
	{
		iter->second->clearRanges();
	}
	this->masterRanges->clearRanges();
}

void FeMeshFieldRangesCache::deaccess(FeMeshFieldRangesCache*& meshFieldRangesCache)
{
	if (meshFieldRangesCache)
	{
		--(meshFieldRangesCache->access_count);
		if (meshFieldRangesCache->access_count <= 0)
		{
			delete meshFieldRangesCache;
		}
		meshFieldRangesCache = nullptr;
	}
}

void FeMeshFieldRangesCache::detachFromMesh()
{
	// must clear any labels maps as labels are now destroyed
	this->clearRanges();
	this->mesh = nullptr;
}

void FeMeshFieldRangesCache::evaluateMeshFieldRanges(cmzn_fieldcache& fieldcache, FeMeshFieldRanges *meshFieldRanges)
{
	if (!((this->mesh) && (this->mesh->get_FE_region()) && (this->mesh->get_FE_region()->getRegion())))
	{
		display_message(ERROR_MESSAGE, "FeMeshFieldRangesCache::evaluateMeshFieldRanges.  Called when detached from mesh/region");
		return;
	}
	cmzn_region *region = this->mesh->get_FE_region()->getRegion();
	// enforce critical section for remainder of scope: only one thread can evaluate ranges at a time
	const std::lock_guard<std::mutex> lock(this->evaluateMutex);
	if (meshFieldRanges->isEvaluated())
	{
		// another thread did it while we were waiting
		return;
	}

	cmzn_fieldrange *fieldrange = cmzn_fieldrange::create(&fieldcache);

	const int componentsCount = this->field->getNumberOfComponents();
	std::vector<double> values(componentsCount * 2);
	double *minimums = values.data();
	double *maximums = values.data() + componentsCount;

	FeElementFieldRange *totalRange = nullptr;
	cmzn_field_element_group *elementGroup = meshFieldRanges->getFieldElementGroup();
	DsLabelsGroup *labelsGroup = (elementGroup) ?
		&(Computed_field_element_group_core_cast(elementGroup)->getLabelsGroup()) : nullptr;
	cmzn_elementiterator *elemIter = this->mesh->createElementiterator(labelsGroup);
	cmzn_element *element;
	while ((element = elemIter->nextElement()))
	{
		DsLabelIndex elementIndex = element->getIndex();
		const FeElementFieldRange *elementFieldRange = this->masterRanges->getElementFieldRange(elementIndex);
		if (!elementFieldRange)
		{
			if ((CMZN_OK == fieldcache.setElement(element)) &&
				(CMZN_OK == fieldrange->evaluateRange(this->field, &fieldcache)) &&
				(CMZN_OK == fieldrange->getRange(componentsCount, minimums, maximums)))
			{
				elementFieldRange = new FeElementFieldRange(componentsCount, minimums, maximums);
				this->masterRanges->setElementFieldRange(elementIndex, elementFieldRange);
			}
		}
		if (elementFieldRange)
		{
			if (totalRange)
			{
				totalRange->enlarge(componentsCount, *elementFieldRange);
			}
			else
			{
				totalRange = new FeElementFieldRange(componentsCount, *elementFieldRange);
			}
			if (elementGroup)
			{
				meshFieldRanges->setElementFieldRange(elementIndex, elementFieldRange);
			}
		}
	}
	cmzn_fieldrange::deaccess(fieldrange);
	meshFieldRanges->setTotalRange(componentsCount, totalRange);
	meshFieldRanges->setEvaluated();
}

const DsLabels *FeMeshFieldRangesCache::getLabels() const
{
	return this->mesh ? &(this->mesh->getLabels()) : nullptr;
}

FeMeshFieldRanges *FeMeshFieldRangesCache::getMeshFieldRanges(cmzn_field_element_group *elementGroup)
{
	if (!elementGroup)
	{
		return this->masterRanges->access();
	}
	std::map<cmzn_field_element_group *, FeMeshFieldRanges *>::iterator iter = this->groupRanges.find(elementGroup);
	if (iter != this->groupRanges.end())
	{
		return iter->second->access();
	}
	FeMeshFieldRanges *meshFieldRanges = new FeMeshFieldRanges(this, elementGroup);
	this->groupRanges[elementGroup] = meshFieldRanges;
	return meshFieldRanges;
}

void FeMeshFieldRangesCache::removeMeshFieldRanges(FeMeshFieldRanges *meshFieldRanges)
{
	cmzn_field_element_group *elementGroup = meshFieldRanges->getFieldElementGroup();
	if (elementGroup)
	{
		this->groupRanges.erase(elementGroup);
	}
}
