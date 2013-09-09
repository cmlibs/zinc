/***************************************************************************//**
 * FILE : field_ensemble.hpp
 * 
 * Implements a domain field consisting of set of indexed entries.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (FIELD_ENSEMBLE_HPP)
#define FIELD_ENSEMBLE_HPP

#include "computed_field/computed_field_private.hpp"
#include "field_io/cmiss_field_ensemble.h"
#include "general/block_array.hpp"

namespace cmzn
{

/***************************************************************************//**
 * An internal reference to an ensemble entry.
 * Actually an array index starting at 0 with -1 == invalid.
 * Must not be in external API - use cmzn_ensemble_iterator instead.
 * @see cmzn_ensemble_iterator.
 */
typedef int EnsembleEntryRef;

const cmzn_ensemble_identifier CMZN_INVALID_ENSEMBLE_IDENTIFIER = -1;
const EnsembleEntryRef CMZN_INVALID_ENSEMBLE_ENTRY_REF = -1;

/***************************************************************************//**
 * A domain comprising a set of entries / indexes with unique unsigned integer
 * identifiers. 
 */
class Field_ensemble : public Computed_field_core
{
private:

typedef std::map<cmzn_ensemble_identifier,EnsembleEntryRef> EnsembleEntryMap;
typedef std::map<cmzn_ensemble_identifier,EnsembleEntryRef>::iterator EnsembleEntryMapIterator;
typedef std::map<cmzn_ensemble_identifier,EnsembleEntryRef>::reverse_iterator EnsembleEntryMapReverseIterator;

	bool contiguous; // true while all entries from firstIdentifier..lastIdentifier exist and are in order
	block_array<EnsembleEntryRef,cmzn_ensemble_identifier> entries; // used only if not contiguous
	EnsembleEntryMap identifierMap; // used only if not contiguous
	cmzn_ensemble_identifier firstFreeIdentifier, firstIdentifier, lastIdentifier;
	int entryCount; // number of valid entries
	int refCount; // number of array slots allocated; some unused where members removed

	// linked-lists of active iterators is maintained to keep track of entry refs for
	// reclaiming memory. When destroyed these are placed on the available list for
	// re-use without hitting the heap.
	cmzn_ensemble_iterator *activeIterators, *availableIterators;
	
public:
	Field_ensemble() :
		Computed_field_core(),
		contiguous(true),
		firstFreeIdentifier(1),
		firstIdentifier(CMZN_INVALID_ENSEMBLE_IDENTIFIER),
		lastIdentifier(CMZN_INVALID_ENSEMBLE_IDENTIFIER),
		entryCount(0),
		refCount(0),
		activeIterators(NULL),
		availableIterators(NULL)
	{
	};

	~Field_ensemble();
	
private:
	Computed_field_core *copy()
	{
		return NULL; // not supported
	}

	const char *get_type_string();

	int compare(Computed_field_core* other_field)
	{
		return (NULL != dynamic_cast<Field_ensemble*>(other_field)) ? 1 : 0;
	}

	int evaluate(cmzn_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string()
	{ 
		return NULL;
	}
	
	void updateFirstFreeIdentifier();

	void setNotContiguous();

	EnsembleEntryRef createEntryPrivate(cmzn_ensemble_identifier identifier);

public:

	EnsembleEntryRef size()
	{
		return entryCount;
	}

	/** @return  true if any refs void, false if all are valid */
	bool hasVoidRefs()
	{
		return (entryCount < refCount);
	}

	/** @return  maximum ref which has ever existed; entry may be void */
	EnsembleEntryRef getRefCount()
	{
		return refCount;
	}
	
	EnsembleEntryRef createEntry();
	EnsembleEntryRef createEntry(cmzn_ensemble_identifier identifier);
	EnsembleEntryRef findOrCreateEntry(cmzn_ensemble_identifier identifier);
	EnsembleEntryRef findEntryByIdentifier(cmzn_ensemble_identifier identifier);
	int removeEntry(EnsembleEntryRef ref);
	int removeEntryWithIdentifier(cmzn_ensemble_identifier identifier);
	cmzn_ensemble_identifier getEntryIdentifier(EnsembleEntryRef ref);

	/** Get ref to entry in group with lowest identifier */
	EnsembleEntryRef getFirstEntryRef();

	EnsembleEntryRef getNextEntryRef(EnsembleEntryRef ref);
	EnsembleEntryRef getNextEntryRefBoolTrue(EnsembleEntryRef ref, bool_array<EnsembleEntryRef>& values);

	static int incrementEnsembleEntry(cmzn_ensemble_iterator *iterator);
	
	/** creates an iterator out of an internal ref including ensemble pointer */
	cmzn_ensemble_iterator *createEnsembleIterator(EnsembleEntryRef ref);
	static void freeEnsembleIterator(cmzn_ensemble_iterator *&iterator);
};


/***************************************************************************//**
 * A group of entries from a single ensemble.
 * Implemented using a bool_array
 */
class Field_ensemble_group : public Computed_field_core
{
private:
	Field_ensemble *ensemble;
	int entryCount;
	// refLimit is at least one greater than highest member of group, updated to exact index when queried
	int refLimit;
	bool_array<EnsembleEntryRef> values;
	
public:
	Field_ensemble_group(Field_ensemble *from_ensemble) :
		Computed_field_core(),
		ensemble(from_ensemble),
		entryCount(0),
		refLimit(0)
	{
	};

	virtual bool attach_to_field(Computed_field* parent)
	{
		if (Computed_field_core::attach_to_field(parent))
		{
			if (ensemble)
				return true;
		}
		return false;
	};

	~Field_ensemble_group()
	{
	}
	
private:

	Computed_field_core *copy()
	{
		return NULL; // not supported
	}

	const char *get_type_string();

	int compare(Computed_field_core* other_field)
	{
		return (NULL != dynamic_cast<Field_ensemble_group*>(other_field)) ? 1 : 0;
	}

	int evaluate(cmzn_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string()
	{ 
		return NULL;
	}

public:

	Field_ensemble *getEnsemble()
	{
		return ensemble;
	}
	
	void clear()
	{
		values.clear();
		entryCount = 0;
		refLimit = 0;
	}

	EnsembleEntryRef size()
	{
		return entryCount;
	}

	EnsembleEntryRef getRefLimit()
	{
		if (refLimit > 0)
		{
			EnsembleEntryRef index = refLimit - 1;
			if (values.updateLastTrueIndex(index))
			{
				refLimit = index + 1;
			}
		}
		return refLimit;
	}

	/** @return true if group contains all entries from 1..refLimit */
	bool isDense()
	{
		getRefLimit();
		return (entryCount == refLimit);
	}

	/** @return true if group contains all entries from belowRef+1..refLimit */
	bool isDenseAbove(EnsembleEntryRef belowRef)
	{
		getRefLimit();
		// GRC: this can be expensive:
		return values.isRangeTrue(/*minIndex*/belowRef + 1, /*minIndex*/refLimit-1);
	}
	
	/** be careful that ref is for this ensemble */
	bool hasEntryRef(EnsembleEntryRef ref) const
	{
		if (ref < 0)
			return false;
		return values.getBool(/*index*/ref);
	}

	/** be careful that ref is for this ensemble */
	int setEntryRef(EnsembleEntryRef ref, bool inGroup)
	{
		if (ref < 0)
			return false;
		bool wasInGroup;
		if (values.setBool(/*index*/ref, inGroup, wasInGroup))
		{
			if (inGroup != wasInGroup)
			{
				if (inGroup)
				{
					entryCount++;
					if (ref >= refLimit)
						refLimit = ref + 1;
				}
				else
				{
					entryCount--;
				}
			}
			return 1;
		}
		return 0;
	}

	bool hasEntry(const cmzn_ensemble_iterator *iterator) const;

	int setEntry(const cmzn_ensemble_iterator *iterator, bool inGroup);

	/** Get ref to entry in group with lowest identifier */
	EnsembleEntryRef getFirstEntryRef()
	{
		EnsembleEntryRef ref = ensemble->getFirstEntryRef();
		if (hasEntryRef(ref))
			return ref;
		return ensemble->getNextEntryRefBoolTrue(ref, values);
	}

	EnsembleEntryRef getNextEntryRef(EnsembleEntryRef ref)
	{
		return ensemble->getNextEntryRefBoolTrue(ref, values);		
	}

	int incrementEnsembleEntry(cmzn_ensemble_iterator *iterator);

};


} // namespace cmzn

inline cmzn::Field_ensemble *cmzn_field_ensemble_core_cast(
	cmzn_field_ensemble *ensemble_field)
{
	return (static_cast<cmzn::Field_ensemble*>(
		reinterpret_cast<cmzn_field*>(ensemble_field)->core));
}

inline cmzn::Field_ensemble_group *cmzn_field_ensemble_group_core_cast(
	cmzn_field_ensemble_group *ensemble_group_field)
{
	return (static_cast<cmzn::Field_ensemble_group*>(
		reinterpret_cast<cmzn_field*>(ensemble_group_field)->core));
}

/***************************************************************************//**
 * External iterator-handle to an ensemble entry.
 * Maintains pointer to owning ensemble for type safety.
 * ???GRC add map iterator member to improve efficiency.
 */
struct cmzn_ensemble_iterator
{
	friend class cmzn::Field_ensemble;
	friend class cmzn::Field_ensemble_group;
	
private:
	cmzn::Field_ensemble *ensemble;
	cmzn::EnsembleEntryRef ref;
	cmzn_ensemble_iterator *next, *previous; // for linked-list in owning ensemble

public:
	void setRef(cmzn::EnsembleEntryRef newRef) { ref = newRef; }

	bool isValid() const { return (ref != 0); }
	cmzn::Field_ensemble *getEnsemble() const { return ensemble; }
	cmzn::EnsembleEntryRef getRef() const { return ref; }
	cmzn_ensemble_identifier getIdentifier() const { return ensemble->getEntryIdentifier(ref); }
	bool increment()
	{
		ref = ensemble->getNextEntryRef(ref);
		return (ref >= 0);
	}
};

/***************************************************************************//**
 * Index to 1 or multiple entries in N ensembles, used to address parameters.
 * For each ensemble it may represent:
 * - unspecified = all entries in ensemble in order of increasing identifier.
 * - a single entry, specified using a cmzn_ensemble_iterator
 * - a group of entries in order of increasing identifier, specified using a
 *   cmzn_field_ensemble_group.
 * - (Future: may need to permit a sequence of entries in specified order)
 * Where there are groups specified for multiple ensembles, all permutations of
 * all entries in the groups are indexed in order of first ensemble changing
 * slowest to last ensemble changing fastest, like C arrays.
 * GRC may review order
 * Note:
 * Not to be shared between threads as internal mutable members are modified
 * by Field_parameter::setValues();
 */
struct cmzn_ensemble_index
{
	class Indexing
	{

	public:
		cmzn::Field_ensemble *ensemble;
		// only zero or one of following set; zero means use all entries in ensemble
		cmzn_ensemble_iterator *iterator;
		cmzn::Field_ensemble_group *ensemble_group;
		// following mutable members are used by Field_parameter::setValues
		mutable cmzn::EnsembleEntryRef indexRefLimit;
		mutable cmzn::EnsembleEntryRef firstRef;
		mutable cmzn_ensemble_iterator *valuesIterator;

	private:

		void clear_iterator()
		{
			if (iterator)
				cmzn_ensemble_iterator_destroy(&iterator);
		}

		void clear_ensemble_group()
		{
			if (ensemble_group)
			{
				cmzn_field *field = ensemble_group->getField();
				cmzn_field_destroy(&field);
				ensemble_group = NULL;
			}
		}

	public:

		Indexing() :
			ensemble(NULL),
			iterator(NULL),
			ensemble_group(NULL),
			indexRefLimit(0),
			firstRef(cmzn::CMZN_INVALID_ENSEMBLE_ENTRY_REF),
			valuesIterator(NULL)
		{
		}

		/* second stage of construction */
		void setEnsemble(cmzn::Field_ensemble *in_ensemble)
		{
			ensemble = in_ensemble;
		}

		~Indexing()
		{
			if (valuesIterator)
				cmzn_ensemble_iterator_destroy(&valuesIterator);
			clear_iterator();
			clear_ensemble_group();
		}

		unsigned int getEntryCount() const
		{
			if (iterator)
				return (iterator->getRef() >= 0) ? 1 : 0;
			if (ensemble_group)
				return ensemble_group->size();
			return ensemble->size();
		}

		void setAllEnsemble()
		{
			clear_iterator();
			clear_ensemble_group();
		}

		void setEntry(cmzn_ensemble_iterator *in_iterator)
		{
			if (iterator)
				iterator->setRef(in_iterator->getRef());
			else
				iterator = ensemble->createEnsembleIterator(in_iterator->getRef());
			clear_ensemble_group();
		}

		void setGroup(cmzn::Field_ensemble_group *in_ensemble_group)
		{
			cmzn_field_access(in_ensemble_group->getField());
			clear_ensemble_group();
			ensemble_group = in_ensemble_group;
			clear_iterator();
		}

		void calculateIndexRefLimit()
		{
			if (iterator)
				indexRefLimit = iterator->getRef() + 1;
			else if (ensemble_group)
				indexRefLimit = ensemble_group->getRefLimit();
			else
				indexRefLimit = ensemble->getRefCount(); // GRC this is conservative
		}

		/** @return true if indexing is dense from 0..indexRefLimit-1 */
		bool isDense()
		{
			if (ensemble->hasVoidRefs())
				return false;
			if (iterator)
				return (iterator->getRef() == 0);
			if (ensemble_group)
				return ensemble_group->isDense();
			return true;
		}

		bool isDenseAbove(cmzn::EnsembleEntryRef belowRef)
		{
			if (ensemble->hasVoidRefs())
				return false;
			if (iterator)
			{
				return (iterator->getRef() == (belowRef + 1));
			}
			if (ensemble_group)
				return ensemble_group->isDenseAbove(belowRef);
			return true;
		}

		bool iterationBegin()
		{
			if (iterator)
			{
				// no valuesIterator when single entry is indexed
				firstRef = iterator->getRef();
				valuesIterator = NULL;
			}
			else if (ensemble_group)
			{
				firstRef = ensemble_group->getFirstEntryRef();
				valuesIterator = ensemble->createEnsembleIterator(firstRef);
			}
			else
			{
				firstRef = ensemble->getFirstEntryRef();
				valuesIterator = ensemble->createEnsembleIterator(firstRef);
			}
			return (firstRef >= 0) && (iterator || valuesIterator);
		}

		cmzn::EnsembleEntryRef iterationRef()
		{
			return (iterator ? iterator->getRef() : valuesIterator->getRef());
		}

		/** @return  true if more to come, false if last */
		bool iterationNext()
		{
			if (iterator)
				return false;
			if (ensemble_group)
			{
				if (ensemble_group->incrementEnsembleEntry(valuesIterator))
					return true;
			}
			else
			{
				if (valuesIterator->increment())
					return true;
			}
			valuesIterator->setRef(firstRef);
			return false;
		}

		void iterationEnd()
		{
			if (valuesIterator)
				cmzn_ensemble_iterator_destroy(&valuesIterator);
			firstRef = 0;
		}

	};

private:
	cmzn_field *indexee;
	int number_of_ensembles;
	Indexing *indexing;

	cmzn_ensemble_index(cmzn_field *in_indexee,
		int in_number_of_ensembles, cmzn::Field_ensemble **in_ensembles) :
			indexee(cmzn_field_access(in_indexee)),
			number_of_ensembles(in_number_of_ensembles),
			indexing(new Indexing[number_of_ensembles])
	{
		for (int i = 0; i < number_of_ensembles; i++)
		{
			indexing[i].setEnsemble(in_ensembles[i]);
		}
	}

	Indexing *getIndexing(cmzn::Field_ensemble *ensemble)
	{
		for (int i = 0; i < number_of_ensembles; i++)
		{
			if (ensemble == indexing[i].ensemble)
				return &(indexing[i]);
		}
		return NULL;
	}

public:
	
	static cmzn_ensemble_index *create(cmzn_field *in_indexee,
		int in_number_of_ensembles, cmzn::Field_ensemble **in_ensembles)
	{
		if ((NULL == in_indexee) || (in_number_of_ensembles < 0) ||
			((in_number_of_ensembles > 0) && (NULL == in_ensembles)))
			return NULL;
		for (int i = 0; i < in_number_of_ensembles; i++)
		{
			if (NULL == in_ensembles[i])
				return NULL;
		}
		return new cmzn_ensemble_index(in_indexee, in_number_of_ensembles, in_ensembles);
	}

	~cmzn_ensemble_index()
	{
		delete[] indexing;
		cmzn_field_destroy(&indexee);
	}

	/** @return  1 if the indexee of this index is field, 0 if not */
	int indexesField(cmzn_field *field) const
	{
		return (field == indexee);
	}

	int hasIndexEnsembles(int number_of_index_ensembles, cmzn_field_ensemble **index_ensemble_fields)
	{
		if (number_of_index_ensembles != number_of_ensembles)
			return 0;
		for (int i = 0; i < number_of_ensembles; i++)
		{
			if (!index_ensemble_fields[i])
				return 0;
			if (cmzn_field_ensemble_core_cast(index_ensemble_fields[i]) != indexing[i].ensemble)
				return 0;
		}
		return 1;
	}

	/** gets the number of permutations of ensemble entries this index covers */
	unsigned int getEntryCount() const
	{
		unsigned int count = 1;
		for (int i = 0; i < number_of_ensembles; i++)
		{
			count *= indexing[i].getEntryCount();
		}
		return count;
	}

	int setAllEnsemble(cmzn::Field_ensemble *ensemble)
	{
		Indexing *indexing = getIndexing(ensemble);
		if (!indexing)
			return 0;
		indexing->setAllEnsemble();
		return 1;
	}

	int setEntry(cmzn_ensemble_iterator *iterator)
	{
		Indexing *indexing = getIndexing(iterator->getEnsemble());
		if (!indexing)
			return 0;
		indexing->setEntry(iterator);
		return 1;
	}

	int setGroup(cmzn::Field_ensemble_group *ensemble_group)
	{
		Indexing *indexing = getIndexing(ensemble_group->getEnsemble());
		if (!indexing)
			return 0;
		indexing->setGroup(ensemble_group);
		return 1;
	}

	/**
	 * Precalculates indexRefLimit values for each ensemble.
	 * Note: must call getEntryCount() first to confirm no invalid refs!
	 */
	void calculateIndexRefLimits()
	{
		for (int i = 0; i < number_of_ensembles; i++)
		{
			indexing[i].calculateIndexRefLimit();
		}
	}

	/**
	 * Returns limiting entry ref for the indexing of the selected ensemble.
	 * Must have called calculateIndexRefLimits first
	 * @param ensemble_array_index  Index from 0 to number_of_ensembles-1
	 */
	cmzn::EnsembleEntryRef indexRefLimit(int ensemble_array_index)
	{
		return indexing[ensemble_array_index].indexRefLimit;
	}

	/**
	 * Returns true if all refs are specified from 0..indexRefLimit-1
	 * for the selected ensemble 
	 * @param ensemble_array_index  Index from 0 to number_of_ensembles-1
	 */
	bool isDenseOnEnsemble(int ensemble_array_index)
	{
		return indexing[ensemble_array_index].isDense();
	}

	/**
	 * Returns true if all refs are specified from belowRef+1..maxIndexRef-1
	 * for the selected ensemble 
	 * @param ensemble_array_index  Index from 0 to number_of_ensembles-1
	 * @param belowRef  Ref one below dense range being checked.
	 */
	bool isDenseOnEnsembleAbove(int ensemble_array_index,
		cmzn::EnsembleEntryRef belowRef)
	{
		return indexing[ensemble_array_index].isDenseAbove(belowRef);
	}

	/** obtain iterators pointing at the first refs in each ensemble indexing
	 * also remember firstRef for reiteration.
	 * Callers must invoke iterationEnd() after successful call!
	 * @return  true on success, false on failure with temporaries cleaned up.
	 */
	bool iterationBegin()
	{
		for (int i = 0; i < number_of_ensembles; i++)
		{
			if (!indexing[i].iterationBegin())
			{
				iterationEnd();
				return false;
			}
		}
		return true;
	}

	cmzn::EnsembleEntryRef iterationRef(int ensemble_array_index)
	{
		return indexing[ensemble_array_index].iterationRef();
	}

	/** @return  true if more to come, false if past last */
	bool iterationNext()
	{
		for (int i = number_of_ensembles - 1; 0 <= i; i--)
		{
			if (indexing[i].iterationNext())
				return true;
		}
		return false;
	}

	void iterationEnd()
	{
		for (int i = 0; i < number_of_ensembles; i++)
		{
			indexing[i].iterationEnd();
		}			
	}
};

namespace cmzn
{

inline int Field_ensemble::incrementEnsembleEntry(cmzn_ensemble_iterator *iterator)
{
	if ((!iterator) || (NULL == iterator->getEnsemble()))
		return 0;
	iterator->increment();
	return (iterator->ref >= 0);
}

inline bool Field_ensemble_group::hasEntry(const cmzn_ensemble_iterator *iterator) const
{
	if (iterator->getEnsemble() != ensemble)
		return 0;
	return hasEntryRef(iterator->getRef());
}

inline int Field_ensemble_group::setEntry(const cmzn_ensemble_iterator *iterator, bool inGroup)
{
	if (iterator->getEnsemble() != ensemble)
		return 0;
	return setEntryRef(iterator->getRef(), inGroup);
}

inline int Field_ensemble_group::incrementEnsembleEntry(cmzn_ensemble_iterator *iterator)
{
	if (!iterator || (iterator->getEnsemble() != ensemble))
		return 0;
	iterator->setRef(getNextEntryRef(iterator->ref));
	return (iterator->getRef() >= 0);
}

} // namespace cmzn

#endif /* !defined (FIELD_ENSEMBLE_HPP) */
