/**
 * FILE : finite_element_domain.hpp
 *
 * Abstract base class for finite element domains built on a set of labels.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_DOMAIN_HPP)
#define FINITE_ELEMENT_DOMAIN_HPP

#include "datastore/labels.hpp"
#include "datastore/labelschangelog.hpp"
#include <list>

struct FE_region;

/** Abstract base class for object containing groups of labels owned by an FE_domain.
 * Interface for notifying when the objects are destroyed */
class FE_domain_group
{
public:
	/** Notify that all objects in domain have been destroyed, so group must be cleared */
	virtual void destroyedAllObjects() = 0;

	/** Notify that one object has been destroyed, so must ensure it is not in group */
	virtual void destroyedObject(DsLabelIndex destroyedIndex) = 0;

	/** Notify that a group of object indexes have been removed, so must ensure
	 * indexes are not in callee group */
	virtual void destroyedObjectGroup(DsLabelsGroup& destroyedLabelsGroup) = 0;
};

/**
 * Abstract base class for finite element domain types built on a list of labels,
 * including FE_nodeset and FE_mesh.
 */
class FE_domain
{
protected:

	FE_region *fe_region; // owning region, not accessed
	const int dimension;

	DsLabels labels; // object identifiers

	// log of objects added, removed or otherwise changed
	DsLabelsChangeLog *changeLog;

	// list of objects containing groups of labels owned by this FE_domain
	// to notify when objects are destroyed
	std::list<FE_domain_group *> groups;

	// when objects are destroyed, store their indexes to notify groups
	DsLabelsGroup *destroyedLabelsGroup;

	mutable int access_count;

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

	/** Add group for notification when objects destroyed */
	void addGroup(FE_domain_group *group)
	{
		this->groups.push_back(group);
	}

	/** Remove group for notification when objects destroyed */
	void removeGroup(FE_domain_group *group)
	{
		this->groups.remove(group);
	}
};

#endif /* !defined (FINITE_ELEMENT_DOMAIN_HPP) */
