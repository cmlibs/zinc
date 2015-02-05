/**
 * FILE : datastore/labelsgroup.hpp
 * 
 * Implements a subset of a datastore labels set.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (CMZN_DATASTORE_LABELSGROUP_HPP)
#define CMZN_DATASTORE_LABELSGROUP_HPP

#include "datastore/labels.hpp"

/**
 * A subset of a datastore labels set.
 * Implemented using a bool_array
 */
class DsLabelsGroup : public cmzn::RefCounted
{
private:
	DsLabels *labels;
	int labelsCount;
	// indexLimit is at least one greater than highest index in group, updated to exact index when queried
	int indexLimit;
	bool_array<DsLabelIndex> values;

	DsLabelsGroup(DsLabels *labelsIn);
	DsLabelsGroup(const DsLabelsGroup&); // not implemented
	~DsLabelsGroup();
	DsLabelsGroup& operator=(const DsLabelsGroup&); // not implemented

public:
	static DsLabelsGroup *create(DsLabels *labelsIn);

	DsLabels *getLabels()
	{
		return this->labels;
	}
	
	void clear()
	{
		values.clear();
		labelsCount = 0;
		indexLimit = 0;
	}

	DsLabelIndex getSize() const
	{
		return labelsCount;
	}

	DsLabelIndex getIndexLimit()
	{
		if (indexLimit > 0)
		{
			DsLabelIndex index = indexLimit - 1;
			if (values.updateLastTrueIndex(index))
				indexLimit = index + 1;
		}
		return indexLimit;
	}

	/** @return true if group contains all entries from 1..indexLimit */
	bool isDense()
	{
		getIndexLimit();
		return (labelsCount == indexLimit);
	}

	/** @return true if group contains all entries from belowIndex+1..indexLimit */
	bool isDenseAbove(DsLabelIndex belowIndex)
	{
		getIndexLimit();
		// GRC: this can be expensive:
		return values.isRangeTrue(/*minIndex*/belowIndex + 1, /*minIndex*/this->indexLimit-1);
	}
	
	/** caller must be careful that index is for this labels */
	bool hasIndex(DsLabelIndex index) const
	{
		if (index < 0)
			return false;
		return values.getBool(/*index*/index);
	}

	/** be careful that index is for this labels */
	int setIndex(DsLabelIndex index, bool inGroup)
	{
		if (index < 0)
			return false;
		bool wasInGroup;
		if (values.setBool(index, inGroup, wasInGroup))
		{
			if (inGroup != wasInGroup)
			{
				if (inGroup)
				{
					labelsCount++;
					if (index >= indexLimit)
						indexLimit = index + 1;
				}
				else
				{
					labelsCount--;
				}
			}
			return 1;
		}
		return 0;
	}

	/**
	 * Get first label index in group or DS_LABEL_INDEX_INVALID if none.
	 * Currently returns index with the lowest identifier in set
	 */
	DsLabelIndex getFirstIndex()
	{
		DsLabelIndex index = this->labels->getFirstIndex();
		if (this->hasIndex(index))
			return index;
		return this->labels->getNextIndexBoolTrue(index, values);
	}

	DsLabelIndex getNextIndex(DsLabelIndex index)
	{
		return this->labels->getNextIndexBoolTrue(index, values);
	}

	int incrementLabelIterator(DsLabelIterator *iterator);

};

#endif /* !defined (CMZN_DATASTORE_LABELSGROUP_HPP) */
