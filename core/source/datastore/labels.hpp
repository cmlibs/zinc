/**
 * FILE : datastore/labels.hpp
 * 
 * Implements a set of labels identifying nodes, elements, field components.
 * Used to index a dimension of a datastore map.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (CMZN_DATASTORE_LABELS_HPP)
#define CMZN_DATASTORE_LABELS_HPP

#include <map>
#include <string>
#include <vector>
#include "general/block_array.hpp"
#include "general/refcounted.hpp"
#include "general/refhandle.hpp"

/**
 * The type of the unique identifier i.e. user number for each label in DsLabels
 * Currently restricted to be non-negative integer.
 */
typedef int DsLabelIdentifier;

struct DsLabelIdentifierRange
{
	DsLabelIdentifier first, last;
};

typedef std::vector<DsLabelIdentifierRange> DsLabelIdentifierRanges;

const DsLabelIdentifier DS_LABEL_IDENTIFIER_INVALID = -1;

/**
 * The array index of a label in the DsLabels, starting at 0.
 * Special value DS_LABEL_INDEX_INVALID means invalid.
 */
typedef int DsLabelIndex;

const DsLabelIndex DS_LABEL_INDEX_INVALID = -1;

typedef std::map<DsLabelIdentifier,DsLabelIndex> DsLabelIdentifierMap;
typedef std::map<DsLabelIdentifier,DsLabelIndex>::iterator DsLabelIdentifierMapIterator;
typedef std::map<DsLabelIdentifier,DsLabelIndex>::reverse_iterator DsLabelIdentifierMapReverseIterator;

class DsLabelIterator;

/**
 * A set of entries with unique identifiers, used to label nodes, elements,
 * field components etc. for indexing into a datastore map.
 */
class DsLabels : public cmzn::RefCounted
{
	std::string name; // optional
	bool contiguous; // true while all entries from firstIdentifier..lastIdentifier exist and are in order
	DsLabelIdentifier firstFreeIdentifier;
	DsLabelIdentifier firstIdentifier; // used if contiguous: identifier of first index
	DsLabelIdentifier lastIdentifier; // used if contiguous: identifier of last valid index
	block_array<DsLabelIndex,DsLabelIdentifier> identifiers; // used only if not contiguous
	DsLabelIdentifierMap identifierMap; // used only if not contiguous
	int labelsCount; // number of valid labels
	int indexSize; // allocated label array size; can have holes where labels removed

	// linked-lists of active iterators for updating when defragmenting memory,
	// and inactive iterators ready for use without allocating on the heap
	DsLabelIterator *activeIterators;
	DsLabelIterator *inactiveIterators;

	template<class REFCOUNTED>
		friend inline void cmzn::Deaccess(REFCOUNTED*& labelIterator);

public:

	DsLabels();
	DsLabels(const DsLabels&); // not implemented
	~DsLabels();
	DsLabels& operator=(const DsLabels&); // not implemented

private:

	void updateFirstFreeIdentifier();

	void setNotContiguous();

	DsLabelIndex createLabelPrivate(DsLabelIdentifier identifier);

	DsLabelIterator *createLabelIteratorPrivate();

	static void destroyLabelIterator(DsLabelIterator *&iterator);

public:

	bool isContiguous()
	{
		return this->contiguous;
	}

	std::string getName() const
	{
		return this->name;
	}

	void setName(const std::string& nameIn)
	{
		this->name = nameIn;
	}

	DsLabelIndex getSize()
	{
		return labelsCount;
	}

	/** @return  true if any indexes are unused, false if all are valid */
	bool hasUnusedIndexes()
	{
		return (labelsCount < indexSize);
	}

	/** @return  maximum index which has ever existed; entry may be void */
	DsLabelIndex getIndexSize()
	{
		return indexSize;
	}
	
	/** @return CMZN_OK on success, any other error code on failure */
	int addLabelsRange(DsLabelIdentifier min, DsLabelIdentifier max,
		DsLabelIdentifier stride = 1);

	DsLabelIndex createLabel();

	DsLabelIndex createLabel(DsLabelIdentifier identifier);

	DsLabelIndex findOrCreateLabel(DsLabelIdentifier identifier);

	DsLabelIndex findLabelByIdentifier(DsLabelIdentifier identifier);

	int removeLabel(DsLabelIndex index);

	int removeLabelWithIdentifier(DsLabelIdentifier identifier);

	DsLabelIdentifier getLabelIdentifier(DsLabelIndex index);

	/**
	 * Get first label index in set or DS_LABEL_INDEX_INVALID if none.
	 * Currently returns index with the lowest identifier in set
	 */
	DsLabelIndex getFirstIndex();
	DsLabelIndex getNextIndex(DsLabelIndex index);
	DsLabelIndex getNextIndexBoolTrue(DsLabelIndex index, bool_array<DsLabelIndex>& boolArray);
	
	DsLabelIterator *createLabelIterator();
	/** creates an iterator giving a handle to a label at index in the labels
	 * @return accessed iterator, 0 if no label at index */
	DsLabelIterator *createLabelIteratorAtIndex(DsLabelIndex index);

	int getIdentifierRanges(DsLabelIdentifierRanges& ranges);
};

typedef cmzn::RefHandle<DsLabels> HDsLabels;

/**
 * An external iterator/handle to a single Label in the DsLabels.
 * Maintains pointer to owning labels for type safety.
 */
class DsLabelIterator : public cmzn::RefCounted
{
	friend class DsLabels;
	friend class DsLabelsGroup;
	
private:
	DsLabels *labels;
	DsLabelIndex index;
	DsLabelIterator *next, *previous; // for linked-list in owning DsLabels

	DsLabelIterator();
	DsLabelIterator(const DsLabelIterator&); // not implemented
	virtual ~DsLabelIterator();
	DsLabelIterator& operator=(const DsLabelIterator&); // not implemented

public:

	// caller must check valid index
	void setIndex(DsLabelIndex newIndex)
	{
		this->index = newIndex;
	}

	bool isValid() const { return (this->index != 0); }
	DsLabels *getLabels() const { return this->labels; }
	DsLabelIndex getIndex() const { return this->index; }
	DsLabelIdentifier getIdentifier() const
	{
		return this->labels->getLabelIdentifier(this->index);
	}

	bool increment()
	{
		if (this->labels)
		{
			this->index = this->labels->getNextIndex(this->index);
			return (this->index >= 0);
		}
		return false;
	}
};

namespace cmzn
{

// specialisation to handle ownership by DsLabels
template<> inline void Deaccess(::DsLabelIterator*& labelIterator)
{
	if (labelIterator)
	{
		--labelIterator->access_count;
		if (labelIterator->access_count <= 0)
			::DsLabels::destroyLabelIterator(labelIterator);
		labelIterator = 0;
	}
}

}

typedef cmzn::RefHandle<DsLabelIterator> HDsLabelIterator;

inline DsLabelIndex DsLabels::findLabelByIdentifier(DsLabelIdentifier identifier)
{
	DsLabelIndex index = DS_LABEL_INDEX_INVALID;
	if (identifier >= 0)
	{
		if (this->contiguous)
		{
			if ((identifier >= this->firstIdentifier) && (identifier <= this->lastIdentifier))
				index = static_cast<DsLabelIndex>(identifier - this->firstIdentifier);
		}
		else
		{
			DsLabelIdentifierMapIterator iter = identifierMap.find(identifier);
			if (iter != identifierMap.end())
				index = iter->second;
		}
	}
	return index;
}

#endif /* !defined (CMZN_DATASTORE_LABELS_HPP) */
