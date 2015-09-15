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
#include "general/message.h"

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
	// can't free externally held objects, hence just invalidate for safety
	iterator = activeIterators;
	while (iterator)
	{
		iterator->invalidate();
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

int DsLabels::setNotContiguous()
{
	if (this->contiguous)
	{
		// this can be optimised:
		DsLabelIdentifier identifier = this->firstIdentifier;
		for (DsLabelIndex index = 0; index < this->indexSize; ++index)
		{
			if (!(this->identifiers.setValue(index, identifier) &&
					this->identifierToIndexMap.insert(*this, index)))
			{
				display_message(ERROR_MESSAGE, "DsLabels::setNotContiguous.  Failed");
				return CMZN_ERROR_MEMORY;
			}
			++identifier;
		}
		this->contiguous = false;
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
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
			if (CMZN_OK != this->setNotContiguous())
				return index;
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
		// must increase these now to allow identifiers to be queried by map
		++this->labelsCount;
		++this->indexSize;
		if (!this->identifierToIndexMap.insert(*this, index))
		{
			display_message(ERROR_MESSAGE, "DsLabels::createLabelPrivate. Failed to insert index into map");
			--this->labelsCount;
			--this->indexSize;
			return DS_LABEL_INDEX_INVALID;
		}
		if (identifier > this->lastIdentifier)
			this->lastIdentifier = identifier;
		if (identifier == this->firstFreeIdentifier)
			++this->firstFreeIdentifier;
	}
	else
		display_message(ERROR_MESSAGE, "DsLabels::createLabelPrivate. Failed to set identifier");
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
	if (this->contiguous && (stride == 1) && (0 == this->labelsCount))
	{
		// fast case
		this->firstIdentifier = min;
		this->lastIdentifier = max;
		this->firstFreeIdentifier = max + 1;
		this->labelsCount = max - min + 1;
		this->indexSize = max - min + 1;
	}
	else
	{
		for (DsLabelIdentifier identifier = min; identifier <= max; identifier += stride)
		{
			DsLabelIndex index = this->findOrCreateLabel(identifier);
			if (index < 0)
				return CMZN_ERROR_GENERAL;
		}
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
			this->identifierToIndexMap.erase(*this, index);
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
					DsLabelIndex lastIndex = this->identifierToIndexMap.get_last_object();
					this->lastIdentifier = this->getIdentifier(lastIndex);
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

DsLabelIndex DsLabels::getFirstIndex()
{
	if (0 == this->labelsCount)
		return DS_LABEL_INDEX_INVALID;
	if (this->contiguous)
		return 0;
	return this->identifierToIndexMap.get_first_object();
}

DsLabelIterator *DsLabels::createLabelIterator()
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
		iterator->iter = (this->contiguous) ? 0 : new DsLabelIdentifierToIndexMapIterator(&this->identifierToIndexMap);
		iterator->index = DS_LABEL_INDEX_INVALID;
		iterator->next = this->activeIterators;
		iterator->previous = 0;
		if (this->activeIterators)
			this->activeIterators->previous = iterator;
		this->activeIterators = iterator;
	}
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
			iterator->invalidate();
		}
		else
		{
			delete iterator;
		}
		iterator = 0;
	}
}

int DsLabels::getIdentifierRanges(DsLabelIdentifierRanges& ranges)
{
	ranges.clear();
	DsLabelIterator *iterator = this->createLabelIterator();
	if (!iterator)
		return CMZN_ERROR_MEMORY;
	if (iterator->nextIndex() != DS_LABEL_INDEX_INVALID)
	{
		DsLabelIdentifier identifier = iterator->getIdentifier();
		DsLabelIdentifierRange range = { identifier, identifier };
		while (iterator->nextIndex() != DS_LABEL_INDEX_INVALID)
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
	}
	cmzn::Deaccess(iterator);
	return CMZN_OK;
}

DsLabelIterator::DsLabelIterator() :
	cmzn::RefCounted(),
	labels(0),
	iter(0),
	index(DS_LABEL_INDEX_INVALID),
	next(0),
	previous(0)
{
}

DsLabelIterator::~DsLabelIterator()
{
	delete this->iter;
}

void DsLabelIterator::invalidate()
{
	if (this->labels)
	{
		delete this->iter;
		this->iter = 0;
		this->labels = 0;
	}
}
