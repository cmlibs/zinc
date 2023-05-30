/**
 * FILE : finite_element_mesh_field_ranges.cpp
 *
 * Caches field ranges in elements, owned by FE_mesh.
 */
 /* Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "cmlibs/zinc/region.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/field_cache.hpp"
#include "computed_field/field_range.hpp"
#include "datastore/labelsgroup.hpp"
#include "finite_element/finite_element_mesh.hpp"
#include "finite_element/finite_element_mesh_field_ranges.hpp"
#include "finite_element/finite_element_region_private.h"
#include "region/cmiss_region.hpp"
#include "mesh/mesh.hpp"
#include "mesh/mesh_group.hpp"
#include <vector>


FeMeshFieldRanges::FeMeshFieldRanges(FeMeshFieldRangesCache* meshFieldRangesCacheIn, cmzn_mesh_group* meshGroupIn) :
	meshFieldRangesCache(meshFieldRangesCacheIn),
	meshGroup(meshGroupIn),
	totalRange(nullptr),
	tolerance(0.0),
	evaluated(false),
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
	if (!this->meshGroup)
	{
		if (this->meshFieldRangesCache)
		{
			const DsLabels *labels = this->meshFieldRangesCache->getLabels();
			if (labels)
			{
				// avoid looping through indexes if already clear
				if (this->elementFieldRanges.getBlockCount() > 0)
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
	}
	this->elementFieldRanges.clear();
	delete this->totalRange;
	this->totalRange = nullptr;
	this->tolerance = 0.0;
	this->evaluated = false;
}

void FeMeshFieldRanges::clearElementRange(DsLabelIndex elementIndex)
{
	const FeElementFieldRange *elementFieldRange = this->elementFieldRanges.getValue(elementIndex);
	if (elementFieldRange)
	{
		if (!this->meshGroup)
		{
			// only the master group is responsible for freeing range objects
			delete elementFieldRange;
		}
		this->elementFieldRanges.setValue(elementIndex, nullptr);
	}
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
	// only called from Cache destructor; ranges have already been cleared
	this->meshFieldRangesCache = nullptr;
}


// only called from FE_mesh::getMeshFieldRangesCache()
FeMeshFieldRangesCache::FeMeshFieldRangesCache(FE_mesh *feMeshIn, cmzn_field *fieldIn) :
	feMesh(feMeshIn),
	field(fieldIn),
	masterRanges(new FeMeshFieldRanges(this, nullptr)),
	access_count(1)
{
	this->feMesh->addMapper(this);
}

FeMeshFieldRangesCache::~FeMeshFieldRangesCache()
{
	// mesh is nullptr if already destroyed & cache is detached
	if (this->feMesh)
	{
		this->feMesh->removeMeshFieldRangesCache(this);
		this->detachFromMesh();
	}
	for (std::map<cmzn_mesh_group *, FeMeshFieldRanges *>::iterator iter = this->groupRanges.begin();
		iter != this->groupRanges.end(); ++iter)
	{
		iter->second->detachFromCache();
	}
	this->masterRanges->detachFromCache();
	FeMeshFieldRanges::deaccess(this->masterRanges);
}

void FeMeshFieldRangesCache::clearAllRanges()
{
	for (std::map<cmzn_mesh_group*, FeMeshFieldRanges*>::iterator iter = this->groupRanges.begin();
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
	this->clearAllRanges();
	this->feMesh->removeMapper(this);
	this->feMesh = nullptr;
}

void FeMeshFieldRangesCache::evaluateMeshFieldRanges(cmzn_fieldcache& fieldcache, FeMeshFieldRanges *meshFieldRanges)
{
	if (!((this->feMesh) && (this->feMesh->get_FE_region()) && (this->feMesh->get_FE_region()->getRegion())))
	{
		display_message(ERROR_MESSAGE, "FeMeshFieldRangesCache::evaluateMeshFieldRanges.  Called when detached from mesh/region");
		return;
	}
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
	cmzn_mesh_group *meshGroup = meshFieldRanges->getMeshGroup();
	const DsLabelsGroup* labelsGroup = (meshGroup) ? meshGroup->getLabelsGroup() : nullptr;
	cmzn_elementiterator *elemIter = this->feMesh->createElementiterator(labelsGroup);
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
			if (meshGroup)
			{
				meshFieldRanges->setElementFieldRange(elementIndex, elementFieldRange);
			}
		}
	}
    cmzn_elementiterator_destroy(&elemIter);
	cmzn_fieldrange::deaccess(fieldrange);
	meshFieldRanges->setTotalRange(componentsCount, totalRange);
	meshFieldRanges->setEvaluated();
}

const DsLabels *FeMeshFieldRangesCache::getLabels() const
{
	return (this->feMesh) ? &(this->feMesh->getLabels()) : nullptr;
}

FeMeshFieldRanges *FeMeshFieldRangesCache::getMeshFieldRanges(cmzn_mesh* meshIn)
{
	if ((!meshIn) || (meshIn->getFeMesh() != this->feMesh))
	{
		display_message(ERROR_MESSAGE, "FeMeshFieldRangesCache::getMeshFieldRanges.  Invalid mesh");
		return nullptr;
	}
	cmzn_mesh_group* meshGroup = dynamic_cast<cmzn_mesh_group*>(meshIn);
	if (!meshGroup)
	{
		return this->masterRanges->access();
	}
	std::map<cmzn_mesh_group*, FeMeshFieldRanges*>::iterator iter = this->groupRanges.find(meshGroup);
	if (iter != this->groupRanges.end())
	{
		return iter->second->access();
	}
	FeMeshFieldRanges *meshFieldRanges = new FeMeshFieldRanges(this, meshGroup);
	this->groupRanges[meshGroup] = meshFieldRanges;
	return meshFieldRanges;
}

void FeMeshFieldRangesCache::removeMeshFieldRanges(FeMeshFieldRanges *meshFieldRanges)
{
	cmzn_mesh_group *meshGroup = meshFieldRanges->getMeshGroup();
	if (meshGroup)
	{
		this->groupRanges.erase(meshGroup);
	}
}

void FeMeshFieldRangesCache::destroyedAllObjects()
{
	this->clearAllRanges();
}

void FeMeshFieldRangesCache::destroyedObject(DsLabelIndex destroyedIndex)
{
	if (this->masterRanges->getElementFieldRange(destroyedIndex))
	{
		for (std::map<cmzn_mesh_group*, FeMeshFieldRanges*>::iterator iter = this->groupRanges.begin();
			iter != this->groupRanges.end(); ++iter)
		{
			iter->second->clearElementRange(destroyedIndex);
		}
		this->masterRanges->clearElementRange(destroyedIndex);
	}
}

void FeMeshFieldRangesCache::destroyedObjectGroup(const DsLabelsGroup& destroyedLabelsGroup)
{
	// This could be made faster by working directly with data blocks
	DsLabelIndex elementIndex = -1;  // So first increment gives 0 == first valid index
	while (destroyedLabelsGroup.incrementIndex(elementIndex))
	{
		this->destroyedObject(elementIndex);
	}
}
