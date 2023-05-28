/**
 * FILE : finite_element_domain.hpp
 *
 * Abstract base class for finite element domains built on a set of labels.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_DOMAIN_HPP)
#define FINITE_ELEMENT_DOMAIN_HPP

#include "cmlibs/zinc/types/regionid.h"
#include "datastore/labels.hpp"
#include "datastore/labelschangelog.hpp"
#include "general/refcounted.hpp"
#include <list>

struct FE_region;

/** Abstract base class for object mapping from labels owned by an FE_domain.
 * Interface for notifying when the objects are destroyed */
class FE_domain_mapper
{
protected:

	virtual ~FE_domain_mapper()
	{
	}

public:

	/** Notify that all objects in domain have been destroyed, so group must be cleared */
	virtual void destroyedAllObjects() = 0;

	/** Notify that one object has been destroyed, so must ensure it is not in group */
	virtual void destroyedObject(DsLabelIndex destroyedIndex) = 0;

	/** Notify that a group of object indexes have been removed, so must ensure
	 * indexes are not in callee group */
	virtual void destroyedObjectGroup(const DsLabelsGroup& destroyedLabelsGroup) = 0;
};

/**
 * Abstract base class for finite element domain types built on a list of labels,
 * including FE_nodeset and FE_mesh.
 */
class FE_domain : public cmzn::RefCounted
{
protected:

	FE_region *fe_region; // owning region, not accessed
	const int dimension;

	DsLabels labels; // object identifiers

	// log of objects added, removed or otherwise changed
	DsLabelsChangeLog *changeLog;

	// list of objects mapping from labels owned by this FE_domain
	// to notify when objects are destroyed
	std::list<FE_domain_mapper *> mappers;

	// when objects are destroyed, store their indexes to notify groups
	DsLabelsGroup *destroyedLabelsGroup;

	FE_domain(FE_region *fe_region, int dimensionIn);

	virtual ~FE_domain();

	virtual void createChangeLog();

public:

	virtual void detach_from_FE_region();

	int getDimension() const
	{
		return this->dimension;
	}

	FE_region *get_FE_region() const
	{
		return this->fe_region;
	}

	virtual const char *getName() const = 0;

	const DsLabels& getLabels() const
	{
		return this->labels;
	}

	/** @return  Non-accessed owning region, or nullptr if detached during cleanup */
	cmzn_region* getRegion() const;

	virtual void clear();

	void clearChangeLog()
	{
		this->createChangeLog();
	}

	/** @return Accessed changes */
	DsLabelsChangeLog *extractChangeLog();

	/** @return non-Accessed changes */
	DsLabelsChangeLog *getChangeLog()
	{
		return this->changeLog;
	}

	/** get size i.e. number of object/labels in domain */
	int getSize() const
	{
		return this->labels.getSize();
	}

	virtual void list_btree_statistics();

	DsLabelIndex findIndexByIdentifier(DsLabelIdentifier identifier) const
	{
		return this->labels.findLabelByIdentifier(identifier);
	}

	DsLabelsGroup *createLabelsGroup();

	/** Add domain mapper for notifying when objects destroyed */
	void addMapper(FE_domain_mapper* mapper)
	{
		this->mappers.push_back(mapper);
	}

	/** Remove domain mapper for notifying when objects destroyed */
	void removeMapper(FE_domain_mapper* mapper)
	{
		this->mappers.remove(mapper);
	}
};

#endif /* !defined (FINITE_ELEMENT_DOMAIN_HPP) */
