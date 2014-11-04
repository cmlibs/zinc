/**
 * FILE : datastore/mapindexing.hpp
 * 
 * Implements a multidimensional indexing object for addressing data in a
 * datastore map.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (CMZN_DATASTORE_MAPINDEXING_HPP)
#define CMZN_DATASTORE_MAPINDEXING_HPP

#include "datastore/labels.hpp"
#include "datastore/labelsgroup.hpp"

// Declare integer type large enough to address map
// For 64-bit builds and large datasets we will eventually want an 8-byte integer e.g.:
// typedef unsigned long long int DsMapAddressType;
typedef unsigned int DsMapAddressType;

class DsMapBase;

/**
 * Index to subset of a DsMap via array of DsLabels, used to get and set values.
 * For each DsLabels it may represent:
 * - unspecified = all entries in DsLabels in order of increasing identifier.
 * - a single entry, specified using a DsLabelIterator
 * - a group of entries in order of increasing identifier, specified using a
 *   DsLabelsGroup.
 * - (Future: may need to permit a sequence of entries in specified order)
 * Where there are groups specified for multiple DsLabels, all permutations of
 * all entries in the groups are indexed in order of first DsLabels changing
 * slowest to last DsLabels changing fastest, like C arrays.
 * GRC may review order
 * Note:
 * Not to be shared between threads as internal mutable members are modified
 * by DsMap<>::setValues();
 */
struct DsMapIndexing : public cmzn::RefCounted
{
	friend class DsMapBase;

	class Indexing
	{

	public:
		DsLabels *labels;
		// only zero or one of following set; zero means use all labels
		DsLabelIterator *iterator;
		DsLabelsGroup *labelsGroup;
		// following mutable members are used by DsMap<>::setValues
		mutable DsLabelIndex indexLimit;
		mutable DsLabelIndex firstIndex;
		mutable DsLabelIterator *valuesIterator;

	private:

		void clearLabelIterator()
		{
			cmzn::DEACCESS(this->iterator);
		}

		void clearLabelsGroup()
		{
			cmzn::DEACCESS(this->labelsGroup);
		}

	public:

		Indexing() :
			labels(0),
			iterator(0),
			labelsGroup(0),
			indexLimit(0),
			firstIndex(DS_LABEL_INDEX_INVALID),
			valuesIterator(0)
		{
		}

		/* second stage of construction */
		void setLabels(DsLabels *labelsIn)
		{
			this->labels = labelsIn;
		}

		~Indexing()
		{
			cmzn::DEACCESS(this->valuesIterator);
			clearLabelIterator();
			clearLabelsGroup();
		}

		unsigned int getEntryCount() const
		{
			if (this->iterator)
				return (this->iterator->getIndex() >= 0) ? 1 : 0;
			if (this->labelsGroup)
				return this->labelsGroup->getSize();
			return this->labels->getSize();
		}

		void setAll()
		{
			clearLabelIterator();
			clearLabelsGroup();
		}

		void setEntry(DsLabelIterator& iteratorIn)
		{
			if (this->iterator)
				this->iterator->setIndex(iteratorIn.getIndex());
			else
				this->iterator = this->labels->createLabelIteratorAtIndex(iteratorIn.getIndex());
			clearLabelsGroup();
		}

		void setIndex(DsLabelIndex indexIn)
		{
			if (this->iterator)
				this->iterator->setIndex(indexIn);
			else
				this->iterator = this->labels->createLabelIteratorAtIndex(indexIn);
			clearLabelsGroup();
		}

		void setGroup(DsLabelsGroup *labelsGroupIn)
		{
			cmzn::REACCESS(this->labelsGroup, labelsGroupIn);
			clearLabelIterator();
		}

		void calculateIndexLimit()
		{
			if (iterator)
				indexLimit = iterator->getIndex() + 1;
			else if (labelsGroup)
				indexLimit = labelsGroup->getIndexLimit();
			else
				indexLimit = this->labels->getIndexSize(); // GRC this is conservative
		}

		/** @return true if indexing is dense from 0..indexLimit-1 */
		bool isDense()
		{
			if (this->labels->hasUnusedIndexes())
				return false;
			if (iterator)
				return (iterator->getIndex() == 0);
			if (labelsGroup)
				return labelsGroup->isDense();
			return true;
		}

		bool isDenseAbove(DsLabelIndex belowIndex)
		{
			if (this->labels->hasUnusedIndexes())
				return false;
			if (iterator)
				return (iterator->getIndex() == (belowIndex + 1));
			if (labelsGroup)
				return labelsGroup->isDenseAbove(belowIndex);
			return true;
		}

		bool iterationBegin()
		{
			if (this->iterator)
			{
				// no valuesIterator when single entry is indexed
				firstIndex = this->iterator->getIndex();
				valuesIterator = 0;
			}
			else if (this->labelsGroup)
			{
				firstIndex = this->labelsGroup->getFirstIndex();
				valuesIterator = this->labels->createLabelIteratorAtIndex(firstIndex);
			}
			else
			{
				firstIndex = this->labels->getFirstIndex();
				valuesIterator = this->labels->createLabelIteratorAtIndex(firstIndex);
			}
			return (firstIndex >= 0) && (iterator || valuesIterator);
		}

		DsLabelIndex getIterationIndex()
		{
			return (this->iterator ? this->iterator->getIndex() : this->valuesIterator->getIndex());
		}

		/** @return  true if more to come, false if last */
		bool iterationNext()
		{
			if (iterator)
				return false;
			if (labelsGroup)
			{
				if (labelsGroup->incrementLabelIterator(valuesIterator))
					return true;
			}
			else
			{
				if (valuesIterator->increment())
					return true;
			}
			valuesIterator->setIndex(firstIndex);
			return false;
		}

		void iterationEnd()
		{
			if (valuesIterator)
				cmzn::DEACCESS(this->valuesIterator);
			firstIndex = 0;
		}

	};

private:
	DsMapBase *mapBase;
	int labelsArraySize;
	Indexing *indexing;

	DsMapIndexing(DsMapBase& mapBaseIn,
		int labelsArraySizeIn, DsLabels **labelsArrayIn);
	
	static DsMapIndexing *create(DsMapBase& mapBaseIn,
		int labelsArraySizeIn, DsLabels **labelsArrayIn);

	/** get internal Indexing object for supplied DsLabels */
	Indexing *getIndexingForLabels(DsLabels *labelsIn)
	{
		for (int i = 0; i < labelsArraySize; i++)
		{
			if (labelsIn == indexing[i].labels)
				return &(indexing[i]);
		}
		return 0;
	}

public:

	virtual ~DsMapIndexing();

	/** @return  true if the map of this index matches that passed, otherwise false */
	bool indexesMap(const DsMapBase *mapBaseIn) const
	{
		return (mapBaseIn == this->mapBase);
	}

	int hasLabelsArray(int labelsArraySizeIn, DsLabels **labelsArrayIn)
	{
		if (labelsArraySizeIn != labelsArraySize)
			return 0;
		for (int i = 0; i < labelsArraySize; i++)
		{
			if ((!labelsArrayIn[i]) || (labelsArrayIn[i] != indexing[i].labels))
				return 0;
		}
		return 1;
	}

	/** gets the number of permutations of labels this index covers */
	DsMapAddressType getEntryCount() const
	{
		DsMapAddressType count = 1;
		for (int i = 0; i < labelsArraySize; i++)
		{
			count *= indexing[i].getEntryCount();
		}
		return count;
	}

	int setAllLabels(DsLabels *labels)
	{
		Indexing *indexing = getIndexingForLabels(labels);
		if (!indexing)
			return 0;
		indexing->setAll();
		return 1;
	}

	int setEntry(DsLabelIterator& iterator)
	{
		Indexing *indexing = getIndexingForLabels(iterator.getLabels());
		if (!indexing)
			return 0;
		indexing->setEntry(iterator);
		return 1;
	}

	int setEntryIndex(DsLabels& labels, DsLabelIndex indexIn)
	{
		Indexing *indexing = getIndexingForLabels(&labels);
		if (!indexing)
			return 0;
		indexing->setIndex(indexIn);
		return 1;
	}

	int setEntryIdentifier(DsLabels& labels, DsLabelIdentifier identifierIn)
	{
		Indexing *indexing = getIndexingForLabels(&labels);
		if (!indexing)
			return 0;
		indexing->setIndex(labels.findLabelByIdentifier(identifierIn));
		return 1;
	}

	int setGroup(DsLabelsGroup *labelsGroup)
	{
		Indexing *indexing = getIndexingForLabels(labelsGroup->getLabels());
		if (!indexing)
			return 0;
		indexing->setGroup(labelsGroup);
		return 1;
	}

	/**
	 * Precalculates indexLimit values for each labels.
	 * Note: must call getEntryCount() first to confirm no invalid indexes!
	 */
	void calculateIndexLimits()
	{
		for (int i = 0; i < labelsArraySize; i++)
		{
			indexing[i].calculateIndexLimit();
		}
	}

	/**
	 * Returns limiting entry index for the indexing of the selected labels.
	 * Must have called calculateIndexLimits first
	 * @param labelsNumber  Index from 0 to number_of_labels-1
	 */
	DsLabelIndex getIndexLimit(int labelsNumber)
	{
		return indexing[labelsNumber].indexLimit;
	}

	/**
	 * Returns true if all indexes are specified from 0..indexLimit-1
	 * for the selected labels 
	 * @param labelsNumber  Index from 0 to labelsArraySize-1
	 */
	bool isDenseOnLabels(int labelsNumber)
	{
		return indexing[labelsNumber].isDense();
	}

	/**
	 * Returns true if all indexes are specified from belowIndex+1..indexLimit-1
	 * for the selected labels 
	 * @param labelsNumber  Index from 0 to labelsArraySize-1
	 * @param belowIndex  Index one below dense range being checked.
	 */
	bool isDenseOnLabelsAbove(int labelsNumber,
		DsLabelIndex belowIndex)
	{
		return indexing[labelsNumber].isDenseAbove(belowIndex);
	}

	/** obtain iterators pointing at the first indexes in each labels indexing
	 * also remember firstIndex for reiteration.
	 * Callers must invoke iterationEnd() after successful call!
	 * @return  true on success, false on failure with temporaries cleaned up.
	 */
	bool iterationBegin()
	{
		for (int i = 0; i < labelsArraySize; i++)
		{
			if (!indexing[i].iterationBegin())
			{
				iterationEnd();
				return false;
			}
		}
		return true;
	}

	DsLabelIndex getIterationIndex(int labelsNumber)
	{
		return indexing[labelsNumber].getIterationIndex();
	}

	/** @return  true if more to come, false if past last */
	bool iterationNext()
	{
		for (int i = labelsArraySize - 1; 0 <= i; i--)
		{
			if (indexing[i].iterationNext())
				return true;
		}
		return false;
	}

	void iterationEnd()
	{
		for (int i = 0; i < labelsArraySize; i++)
		{
			indexing[i].iterationEnd();
		}			
	}
};

typedef cmzn::RefHandle<DsMapIndexing> HDsMapIndexing;

#endif /* !defined (CMZN_DATASTORE_MAPINDEXING_HPP) */
