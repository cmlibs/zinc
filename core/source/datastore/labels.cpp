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

#include "zinc/status.h"
#include "datastore/labels.hpp"

DsLabels::DsLabels() :
	cmzn::RefCounted(),
	contiguous(true),
	firstFreeIdentifier(1),
	firstIdentifier(DS_LABEL_IDENTIFIER_INVALID),
	lastIdentifier(DS_LABEL_IDENTIFIER_INVALID),
	labelsCount(0),
	indexSize(0),
	activeIterators(0),
	inactiveIterators(0)
{
};

DsLabels::~DsLabels()
{
	DsLabelIterator *iterator;
	while (inactiveIterators)
	{
		iterator = inactiveIterators->next;
		delete inactiveIterators;
		inactiveIterators = iterator;
	}
	// can't free externally held objects, hence just clear labels pointer for safety
	iterator = activeIterators;
	while (iterator)
	{
		iterator->labels = 0;
		iterator = iterator->next;
	}
}

void DsLabels::updateFirstFreeIdentifier()
{
	if (this->firstFreeIdentifier != (this->lastIdentifier + 1))
	{
		while (DS_LABEL_INDEX_INVALID !=
			findLabelByIdentifier(this->firstFreeIdentifier))
		{
			++this->firstFreeIdentifier;
		}
	}
}

void DsLabels::setNotContiguous()
{
	if (this->contiguous)
	{
		this->contiguous = false;
		// this can be optimised:
		DsLabelIdentifier identifier = this->firstIdentifier;
		for (DsLabelIndex index = 0; index < this->indexSize; ++index)
		{
			this->identifiers.setValue(index, identifier);
			identifierMap[identifier] = index;
			++identifier;
		}
	}
}

/** private: caller must have checked identifier is not in use! */
DsLabelIndex DsLabels::createLabelPrivate(DsLabelIdentifier identifier)
{
	if (identifier < 0)
		return DS_LABEL_INDEX_INVALID;
	DsLabelIndex index = DS_LABEL_INDEX_INVALID;
	if (this->contiguous)
	{
		if (0 == this->labelsCount)
		{
			this->firstIdentifier = identifier;
		}
		else if ((identifier < this->firstIdentifier) || (identifier > (this->lastIdentifier + 1)))
		{
			setNotContiguous();
		}
		if (this->contiguous)
		{
			index = this->indexSize;
			this->lastIdentifier = identifier;
			this->firstFreeIdentifier = this->lastIdentifier + 1;
			++this->labelsCount;
			++this->indexSize;
			return index;
		}
	}
	// Future memory optimisation: reclaim unused indexes for new labels.
	// Note for reclaim to work would have to clear indexes into maps
	// as labels are deleted [for maps indexed by these labels].
	if (this->identifiers.setValue(this->indexSize, identifier))
	{
		index = this->indexSize;
		this->identifierMap[identifier] = index;
		if (identifier > this->lastIdentifier)
			this->lastIdentifier = identifier;
		if (identifier == this->firstFreeIdentifier)
			++this->firstFreeIdentifier;
		++this->labelsCount;
		++this->indexSize;
	}
	return index;
}

/** create with auto-generated unique identifier */
DsLabelIndex DsLabels::createLabel()
{
	updateFirstFreeIdentifier();
	return createLabelPrivate(this->firstFreeIdentifier);
}

/** fails if identifier already in use */
DsLabelIndex DsLabels::createLabel(DsLabelIdentifier identifier)
{
	DsLabelIndex index = findLabelByIdentifier(identifier);
	if (index >= 0)
		return DS_LABEL_INDEX_INVALID;
	return createLabelPrivate(identifier);
}

/** finds existing entry with identifier, or creates new one if none */
DsLabelIndex DsLabels::findOrCreateLabel(DsLabelIdentifier identifier)
{
	DsLabelIndex index = findLabelByIdentifier(identifier);
	if (index >= 0)
		return index;
	return createLabelPrivate(identifier);
}

int DsLabels::addLabelsRange(DsLabelIdentifier min, DsLabelIdentifier max,
	DsLabelIdentifier stride)
{
	if ((max < min) || (stride < 1))
		return CMZN_ERROR_ARGUMENT;
	// GRC This can be made more efficient
	for (DsLabelIdentifier identifier = min; identifier <= max; identifier += stride)
	{
		DsLabelIndex index = this->findOrCreateLabel(identifier);
		if (index < 0)
			return CMZN_ERROR_GENERAL;
	}
	return CMZN_OK;
}

int DsLabels::removeLabel(DsLabelIndex index)
{
	if ((index < 0) || (index >= this->indexSize))
		return 0;
	// GRC must check not in use
	if (this->contiguous)
	{
#if defined (FUTURE_CODE)
		if (static_cast<DsLabelIdentifier>(index) == this->lastIdentifier)
		{
			this->lastIdentifier--;
			this->firstFreeIdentifier--;
			--this->labelsCount;
			// GRC can't reduce indexSize yet: must clear parts of maps for reclaim
			--indexSize;
			return 1;
		}
		else
		{
			setNotContiguous();
		}
#else
		setNotContiguous();
#endif
	}
	DsLabelIdentifier identifier;
	if (this->identifiers.getValue(index, identifier))
	{
		if (identifier >= 0)
		{
			identifierMap.erase(identifier);
			this->identifiers.setValue(index, DS_LABEL_IDENTIFIER_INVALID);
			if (identifier <= this->firstFreeIdentifier)
				this->firstFreeIdentifier = identifier;
			--this->labelsCount;
			if (identifier == this->lastIdentifier)
			{
				if (0 == this->labelsCount)
				{
					this->firstIdentifier = DS_LABEL_IDENTIFIER_INVALID;
					this->lastIdentifier = DS_LABEL_IDENTIFIER_INVALID;
					this->firstFreeIdentifier = 1;
				}
				else
				{
					DsLabelIdentifierMapReverseIterator iter = identifierMap.rbegin();
					DsLabelIndex lastIndex = iter->second;
					this->identifiers.getValue(lastIndex, this->lastIdentifier);
				}
			}
			return 1;
		}
	}
	return 0;
}

int DsLabels::removeLabelWithIdentifier(DsLabelIdentifier identifier)
{
	DsLabelIndex index = findLabelByIdentifier(identifier);
	if (index >= 0)
		return removeLabel(index);
	return 0;
}

DsLabelIdentifier DsLabels::getLabelIdentifier(DsLabelIndex index)
{
	DsLabelIdentifier identifier = DS_LABEL_IDENTIFIER_INVALID;
	if ((0 <= index) && (index < this->indexSize))
	{
		if (this->contiguous)
			identifier = this->firstIdentifier + static_cast<DsLabelIdentifier>(index);
		else
			this->identifiers.getValue(index, identifier);
	}
	return identifier;
}

DsLabelIndex DsLabels::getFirstIndex()
{
	if (0 == this->labelsCount)
		return DS_LABEL_INDEX_INVALID;
	if (this->contiguous)
		return 0;
	return identifierMap.begin()->second;
}

DsLabelIndex DsLabels::getNextIndex(DsLabelIndex index)
{
	if (0 <= index)
	{
		if (this->contiguous)
		{
			if (index < (this->indexSize - 1))
				return (index + 1);
		}
		else
		{
			DsLabelIdentifier identifier = getLabelIdentifier(index);
			if (0 <= identifier)
			{
				// optimisation: check if index+1 -> identifier+1 so it is next
				if (getLabelIdentifier(index + 1) == (identifier + 1))
					return (index + 1);
				// O(logN) slow:
				// can be made more efficient by passing DsLabelIterator
				// to this function & keeping iterator in it
				DsLabelIdentifierMapIterator iter = identifierMap.find(identifier);
				iter++;
				if (iter != identifierMap.end())
				{
					return iter->second;
				}
			}
		}
	}
	return DS_LABEL_INDEX_INVALID;
}

DsLabelIndex DsLabels::getNextIndexBoolTrue(DsLabelIndex index,
	bool_array<DsLabelIndex>& boolArray)
{
	if (index < 0)
		return DS_LABEL_INDEX_INVALID;
	DsLabelIndex newIndex = index;
	if (this->contiguous)
	{
		do
		{
			++newIndex;
			if (newIndex >= this->indexSize)
			{
				newIndex = DS_LABEL_INDEX_INVALID;
				break;
			}
		} while (!boolArray.getBool(newIndex));
	}
	else
	{
		// can be made more efficient by passing DsLabelIterator
		// to this function & keeping iterator in it
		DsLabelIdentifier identifier = getLabelIdentifier(newIndex);
		DsLabelIdentifierMapIterator iter = identifierMap.find(identifier);
		do
		{
			iter++;
			if (iter == identifierMap.end())
			{
				newIndex = DS_LABEL_INDEX_INVALID;
				break;
			}
			newIndex = iter->second;
		} while (!boolArray.getBool(newIndex));
	}
	return newIndex;
}

DsLabelIterator *DsLabels::createLabelIteratorPrivate()
{
	DsLabelIterator *iterator = 0;
	if (this->inactiveIterators)
	{
		iterator = this->inactiveIterators;
		iterator->access();
		this->inactiveIterators = iterator->next;
		if (this->inactiveIterators)
			this->inactiveIterators->previous = 0;
	}
	else
		iterator = new DsLabelIterator();
	if (iterator)
	{
		iterator->labels = this;
		iterator->index = DS_LABEL_INDEX_INVALID;
		iterator->next = this->activeIterators;
		iterator->previous = 0;
		if (this->activeIterators)
			this->activeIterators->previous = iterator;
		this->activeIterators = iterator;
	}
	return iterator;
}

DsLabelIterator *DsLabels::createLabelIterator()
{
	DsLabelIterator *iterator = this->createLabelIteratorPrivate();
	if (iterator)
		iterator->index = this->getFirstIndex();
	return iterator;
}

DsLabelIterator *DsLabels::createLabelIteratorAtIndex(DsLabelIndex index)
{
	if (DS_LABEL_IDENTIFIER_INVALID == getLabelIdentifier(index))
		return 0;
	DsLabelIterator *iterator = this->createLabelIteratorPrivate();
	if (iterator)
		iterator->index = index;
	return iterator;
}

void DsLabels::destroyLabelIterator(DsLabelIterator *&iterator)
{
	if (iterator)
	{
		if (iterator->labels)
		{
			// reclaim to inactiveIterators
			if (iterator->previous)
				iterator->previous->next = iterator->next;
			if (iterator->next)
				iterator->next->previous = iterator->previous;
			if (iterator == iterator->labels->activeIterators)
				iterator->labels->activeIterators = iterator->next;
			iterator->previous = 0;
			iterator->next = iterator->labels->inactiveIterators;
			if (iterator->next)
				iterator->next->previous = iterator;
			iterator->labels->inactiveIterators = iterator;
			iterator->labels = 0;
		}
		else
		{
			delete iterator;
			iterator = 0;
		}
	}
}

int DsLabels::getIdentifierRanges(DsLabelIdentifierRanges& ranges)
{
	ranges.clear();
	if (this->getSize() > 0)
	{
		DsLabelIterator *iterator = this->createLabelIterator();
		DsLabelIdentifierRange range = { iterator->getIdentifier(), iterator->getIdentifier() };
		DsLabelIdentifier identifier;
		while (iterator->increment())
		{
			identifier = iterator->getIdentifier();
			if (identifier != (range.last + 1))
			{
				ranges.push_back(range);
				range.first = identifier;
			}
			range.last = identifier;
		}
		ranges.push_back(range);
		cmzn::DEACCESS(iterator);
	}
	return CMZN_OK;
}

DsLabelIterator::DsLabelIterator() :
	cmzn::RefCounted(),
	labels(0),
	index(0),
	next(0),
	previous(0)
{
}

DsLabelIterator::~DsLabelIterator()
{
}
