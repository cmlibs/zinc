/***************************************************************************//**
 * FILE : field_ensemble.hpp
 * 
 * Implements a domain field consisting of set of indexed entries.
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2010
 * the Initial Developer. All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#if !defined (FIELD_ENSEMBLE_HPP)
#define FIELD_ENSEMBLE_HPP

extern "C" {
#include "api/cmiss_field_ensemble.h"
}
#include "computed_field/computed_field_private.hpp"
#include "general/block_array.hpp"

namespace Cmiss
{

/***************************************************************************//**
 * An internal reference to an ensemble entry.
 * Actually an array index starting at 1, with 0 = invalid.
 * Must not be in external API - use Cmiss_ensemble_iterator instead.
 * @see Cmiss_ensemble_iterator.
 */
typedef unsigned int EnsembleEntryRef;

/***************************************************************************//**
 * A domain comprising a set of entries / indexes with unique unsigned integer
 * identifiers. 
 */
class Field_ensemble : public Computed_field_core
{
private:

typedef std::map<Cmiss_ensemble_identifier,EnsembleEntryRef> EnsembleEntryMap;
typedef std::map<Cmiss_ensemble_identifier,EnsembleEntryRef>::iterator EnsembleEntryMapIterator;
typedef std::map<Cmiss_ensemble_identifier,EnsembleEntryRef>::reverse_iterator EnsembleEntryMapReverseIterator;

	bool contiguous; // true while all entries from 1..lastIdentifier exist and are in order
	block_array<EnsembleEntryRef,Cmiss_ensemble_identifier> entries; // used only if not contiguous
	EnsembleEntryMap identifierMap; // used only if not contiguous
	Cmiss_ensemble_identifier firstFreeIdentifier, lastIdentifier;
	EnsembleEntryRef entryCount, maxRef;

	// linked-lists of active iterators is maintained to keep track of entry refs for
	// reclaiming memory. When destroyed these are placed on the available list for
	// re-use without hitting the heap.
	Cmiss_ensemble_iterator *activeIterators, *availableIterators;
	
public:
	Field_ensemble() :
		Computed_field_core(),
		contiguous(true),
		firstFreeIdentifier(1),
		lastIdentifier(0),
		entryCount(0),
		maxRef(0),
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

	char *get_type_string();

	int compare(Computed_field_core* other_field)
	{
		return (NULL != dynamic_cast<Field_ensemble*>(other_field)) ? 1 : 0;
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string()
	{ 
		return NULL;
	}
	
	void updateFirstFreeIdentifier();

	void setNotContiguous();

	EnsembleEntryRef createEntryPrivate(Cmiss_ensemble_identifier identifier);

public:

	EnsembleEntryRef size()
	{
		return entryCount;
	}

	/** @return  true if any refs from 1..maxRef are void, false if all are valid */ 
	bool hasVoidRefs()
	{
		return (entryCount < maxRef);
	}

	/** @return  maximum ref which has ever existed; entry may be void */
	EnsembleEntryRef getMaxRef()
	{
		return maxRef;
	}
	
	EnsembleEntryRef createEntry();
	EnsembleEntryRef createEntry(Cmiss_ensemble_identifier identifier);
	EnsembleEntryRef findOrCreateEntry(Cmiss_ensemble_identifier identifier);
	EnsembleEntryRef findEntryByIdentifier(Cmiss_ensemble_identifier identifier);
	int removeEntry(EnsembleEntryRef ref);
	int removeEntryWithIdentifier(Cmiss_ensemble_identifier identifier);
	Cmiss_ensemble_identifier getEntryIdentifier(EnsembleEntryRef ref);

	/** Get ref to entry in group with lowest identifier */
	EnsembleEntryRef getFirstEntryRef();

	EnsembleEntryRef getNextEntryRef(EnsembleEntryRef ref);
	EnsembleEntryRef getNextEntryRefBoolTrue(EnsembleEntryRef ref, bool_array<EnsembleEntryRef>& values);

	static int incrementEnsembleEntry(Cmiss_ensemble_iterator *iterator);
	
	/** creates an iterator out of an internal ref including ensemble pointer */
	Cmiss_ensemble_iterator *createEnsembleIterator(EnsembleEntryRef ref);
	static void freeEnsembleIterator(Cmiss_ensemble_iterator *&iterator);
};


/***************************************************************************//**
 * A group of entries from a single ensemble.
 * Implemented using a bool_array
 */
class Field_ensemble_group : public Computed_field_core
{
private:
	Field_ensemble *ensemble;
	EnsembleEntryRef entryCount;
	// maxRef is an upper bound, updated to exact index when queried
	EnsembleEntryRef maxRef;
	bool_array<EnsembleEntryRef> values;
	
public:
	Field_ensemble_group(Field_ensemble *from_ensemble) :
		Computed_field_core(),
		ensemble(from_ensemble),
		entryCount(0),
		maxRef(0)
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

	char *get_type_string();

	int compare(Computed_field_core* other_field)
	{
		return (NULL != dynamic_cast<Field_ensemble_group*>(other_field)) ? 1 : 0;
	}

	int evaluate_cache_at_location(Field_location* location);

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
		maxRef = 0;
	}

	EnsembleEntryRef size()
	{
		return entryCount;
	}

	EnsembleEntryRef getMaxRef()
	{
		if (maxRef > 0)
		{
			EnsembleEntryRef index = maxRef - 1;
			if (values.updateLastTrueIndex(index))
			{
				maxRef = index + 1;
			}
		}
		return maxRef;
	}

	/** @return true if group contains all entries from 1..maxRef */
	bool isDense()
	{
		getMaxRef();
		return (entryCount == maxRef);
	}

	/** @return true if group contains all entries from belowRef+1..maxRef */
	bool isDenseAbove(EnsembleEntryRef belowRef)
	{
		getMaxRef();
		// GRC: this can be expensive:
		return values.isRangeTrue(/*index*/belowRef, /*index*/maxRef-1);
	}
	
	/** be careful that ref is for this ensemble */
	bool hasEntryRef(EnsembleEntryRef ref) const
	{
		if (ref == 0)
			return false;
		return values.getBool(/*index*/ref-1);
	}

	/** be careful that ref is for this ensemble */
	int setEntryRef(EnsembleEntryRef ref, bool inGroup)
	{
		bool wasInGroup;
		if (values.setBool(/*index*/ref-1, inGroup, wasInGroup))
		{
			if (inGroup != wasInGroup)
			{
				if (inGroup)
				{
					entryCount++;
					if (ref > maxRef)
						maxRef = ref;
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

	bool hasEntry(const Cmiss_ensemble_iterator *iterator) const;

	int setEntry(const Cmiss_ensemble_iterator *iterator, bool inGroup);

	/** Get ref to entry in group with lowest identifier */
	EnsembleEntryRef getFirstEntryRef()
	{
		EnsembleEntryRef ref = ensemble->getFirstEntryRef();
		if (hasEntryRef(ref) || (0 == ref))
			return ref;
		return ensemble->getNextEntryRefBoolTrue(ref, values);
	}

	EnsembleEntryRef getNextEntryRef(EnsembleEntryRef ref)
	{
		return ensemble->getNextEntryRefBoolTrue(ref, values);		
	}

	int incrementEnsembleEntry(Cmiss_ensemble_iterator *iterator);

};


} // namespace Cmiss

inline Cmiss::Field_ensemble *Cmiss_field_ensemble_core_cast(
	Cmiss_field_ensemble *ensemble_field)
{
	return (static_cast<Cmiss::Field_ensemble*>(
		reinterpret_cast<Cmiss_field*>(ensemble_field)->core));
}

inline Cmiss::Field_ensemble_group *Cmiss_field_ensemble_group_core_cast(
	Cmiss_field_ensemble_group *ensemble_group_field)
{
	return (static_cast<Cmiss::Field_ensemble_group*>(
		reinterpret_cast<Cmiss_field*>(ensemble_group_field)->core));
}

/***************************************************************************//**
 * External iterator-handle to an ensemble entry.
 * Maintains pointer to owning ensemble for type safety.
 * ???GRC add map iterator member to improve efficiency.
 */
struct Cmiss_ensemble_iterator
{
	friend class Cmiss::Field_ensemble;
	friend class Cmiss::Field_ensemble_group;
	
private:
	Cmiss::Field_ensemble *ensemble;
	Cmiss::EnsembleEntryRef ref;
	Cmiss_ensemble_iterator *next, *previous; // for linked-list in owning ensemble

public:
	void setRef(Cmiss::EnsembleEntryRef newRef) { ref = newRef; }

	bool isValid() const { return (ref != 0); }
	Cmiss::Field_ensemble *getEnsemble() const { return ensemble; }
	Cmiss::EnsembleEntryRef getRef() const { return ref; }
	Cmiss_ensemble_identifier getIdentifier() const { return ensemble->getEntryIdentifier(ref); }
	bool increment()
	{
		ref = ensemble->getNextEntryRef(ref);
		return (ref != 0);
	}
};

/***************************************************************************//**
 * Index to 1 or multiple entries in N ensembles, used to address parameters.
 * For each ensemble it may represent:
 * - unspecified = all entries in ensemble in order of increasing identifier.
 * - a single entry, specified using a Cmiss_ensemble_iterator
 * - a group of entries in order of increasing identifier, specified using a
 *   Cmiss_field_ensemble_group.
 * - (Future: may need to permit a sequence of entries in specified order)
 * Where there are groups specified for multiple ensembles, all permutations of
 * all entries in the groups are indexed in order of first ensemble changing
 * slowest to last ensemble changing fastest, like C arrays.
 * GRC may review order
 * Note:
 * Not to be shared between threads as internal mutable members are modified
 * by Field_parameter::setValues();
 */
struct Cmiss_ensemble_index
{
	class Indexing
	{

	public:
		Cmiss::Field_ensemble *ensemble;
		// only zero or one of following set; zero means use all entries in ensemble
		Cmiss_ensemble_iterator *iterator;
		Cmiss::Field_ensemble_group *ensemble_group;
		// following mutable members are used by Field_parameter::setValues
		mutable Cmiss::EnsembleEntryRef maxIndexRef;
		mutable Cmiss::EnsembleEntryRef firstRef;
		mutable Cmiss_ensemble_iterator *valuesIterator;

	private:

		void clear_iterator()
		{
			if (iterator)
				Cmiss_ensemble_iterator_destroy(&iterator);
		}

		void clear_ensemble_group()
		{
			if (ensemble_group)
			{
				Cmiss_field *field = ensemble_group->getField();
				Cmiss_field_destroy(&field);
				ensemble_group = NULL;
			}
		}

	public:

		Indexing() :
			ensemble(NULL),
			iterator(NULL),
			ensemble_group(NULL),
			maxIndexRef(0),
			firstRef(0),
			valuesIterator(NULL)
		{
		}

		/* second stage of construction */
		void setEnsemble(Cmiss::Field_ensemble *in_ensemble)
		{
			ensemble = in_ensemble;
		}

		~Indexing()
		{
			if (valuesIterator)
				Cmiss_ensemble_iterator_destroy(&valuesIterator);
			clear_iterator();
			clear_ensemble_group();
		}

		unsigned int getEntryCount() const
		{
			if (iterator)
				return (iterator->getRef() > 0) ? 1 : 0;
			if (ensemble_group)
				return ensemble_group->size();
			return ensemble->size();
		}

		void setAllEnsemble()
		{
			clear_iterator();
			clear_ensemble_group();
		}

		void setEntry(Cmiss_ensemble_iterator *in_iterator)
		{
			if (iterator)
				iterator->setRef(in_iterator->getRef());
			else
				iterator = ensemble->createEnsembleIterator(in_iterator->getRef());
			clear_ensemble_group();
		}

		void setGroup(Cmiss::Field_ensemble_group *in_ensemble_group)
		{
			Cmiss_field_access(in_ensemble_group->getField());
			clear_ensemble_group();
			ensemble_group = in_ensemble_group;
			clear_iterator();
		}

		void calculateMaxIndexRef()
		{
			if (iterator)
				maxIndexRef = iterator->getRef();
			else if (ensemble_group)
				maxIndexRef = ensemble_group->getMaxRef();
			else
				maxIndexRef = ensemble->getMaxRef(); // GRC this is conservative
		}

		/** @return true if indexing is dense from 1..maxIndexRef */
		bool isDense()
		{
			if (ensemble->hasVoidRefs())
				return false;
			if (iterator)
				return (iterator->getRef() == 1);
			if (ensemble_group)
				return ensemble_group->isDense();
			return true;
		}

		bool isDenseAbove(Cmiss::EnsembleEntryRef belowRef)
		{
			if (ensemble->hasVoidRefs())
				return false;
			if (iterator)
				return (iterator->getRef() == (belowRef + 1));
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
			return (firstRef != 0) && (iterator || valuesIterator);
		}

		Cmiss::EnsembleEntryRef iterationRef()
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
				Cmiss_ensemble_iterator_destroy(&valuesIterator);
			firstRef = 0;
		}

	};

private:
	Cmiss_field *indexee;
	int number_of_ensembles;
	Indexing *indexing;

	Cmiss_ensemble_index(Cmiss_field *in_indexee,
		int in_number_of_ensembles, Cmiss::Field_ensemble **in_ensembles) :
			indexee(Cmiss_field_access(in_indexee)),
			number_of_ensembles(in_number_of_ensembles),
			indexing(new Indexing[number_of_ensembles])
	{
		for (int i = 0; i < number_of_ensembles; i++)
		{
			indexing[i].setEnsemble(in_ensembles[i]);
		}
	}

	Indexing *getIndexing(Cmiss::Field_ensemble *ensemble)
	{
		for (int i = 0; i < number_of_ensembles; i++)
		{
			if (ensemble == indexing[i].ensemble)
				return &(indexing[i]);
		}
		return NULL;
	}

public:
	
	static Cmiss_ensemble_index *create(Cmiss_field *in_indexee,
		int in_number_of_ensembles, Cmiss::Field_ensemble **in_ensembles)
	{
		if ((NULL == in_indexee) || (in_number_of_ensembles < 0) ||
			((in_number_of_ensembles > 0) && (NULL == in_ensembles)))
			return NULL;
		for (int i = 0; i < in_number_of_ensembles; i++)
		{
			if (NULL == in_ensembles[i])
				return NULL;
		}
		return new Cmiss_ensemble_index(in_indexee, in_number_of_ensembles, in_ensembles);
	}

	~Cmiss_ensemble_index()
	{
		delete[] indexing;
		Cmiss_field_destroy(&indexee);
	}

	/** @return  1 if the indexee of this index is field, 0 if not */
	int indexesField(Cmiss_field *field) const
	{
		return (field == indexee);
	}

	int hasIndexEnsembles(int number_of_index_ensembles, Cmiss_field_ensemble **index_ensemble_fields)
	{
		if (number_of_index_ensembles != number_of_ensembles)
			return 0;
		for (int i = 0; i < number_of_ensembles; i++)
		{
			if (!index_ensemble_fields[i])
				return 0;
			if (Cmiss_field_ensemble_core_cast(index_ensemble_fields[i]) != indexing[i].ensemble)
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

	int setAllEnsemble(Cmiss::Field_ensemble *ensemble)
	{
		Indexing *indexing = getIndexing(ensemble);
		if (!indexing)
			return 0;
		indexing->setAllEnsemble();
		return 1;
	}

	int setEntry(Cmiss_ensemble_iterator *iterator)
	{
		Indexing *indexing = getIndexing(iterator->getEnsemble());
		if (!indexing)
			return 0;
		indexing->setEntry(iterator);
		return 1;
	}

	int setGroup(Cmiss::Field_ensemble_group *ensemble_group)
	{
		Indexing *indexing = getIndexing(ensemble_group->getEnsemble());
		if (!indexing)
			return 0;
		indexing->setGroup(ensemble_group);
		return 1;
	}

	/**
	 * Precalculates maxIndexRef values for each ensemble.
	 * Note: must call getEntryCount() first to confirm no invalid refs!
	 */
	void calculateMaxIndexRefs()
	{
		for (int i = 0; i < number_of_ensembles; i++)
		{
			indexing[i].calculateMaxIndexRef();
		}
	}

	/**
	 * Returns maximum entry ref for the indexing of the selected ensemble.
	 * Must have called calculateMaxIndexRefs first
	 * @param ensemble_array_index  Index from 0 to number_of_ensembles-1
	 */
	Cmiss::EnsembleEntryRef maxIndexRef(int ensemble_array_index)
	{
		return indexing[ensemble_array_index].maxIndexRef;
	}

	/**
	 * Returns true if all refs are specified from 1..maxIndexRef
	 * for the selected ensemble 
	 * @param ensemble_array_index  Index from 0 to number_of_ensembles-1
	 */
	bool isDenseOnEnsemble(int ensemble_array_index)
	{
		return indexing[ensemble_array_index].isDense();
	}

	/**
	 * Returns true if all refs are specified from belowRef+1..maxIndexRef
	 * for the selected ensemble 
	 * @param ensemble_array_index  Index from 0 to number_of_ensembles-1
	 * @param belowRef  Ref one below dense range being checked.
	 */
	bool isDenseOnEnsembleAbove(int ensemble_array_index,
		Cmiss::EnsembleEntryRef belowRef)
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

	Cmiss::EnsembleEntryRef iterationRef(int ensemble_array_index)
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

namespace Cmiss
{

inline int Field_ensemble::incrementEnsembleEntry(Cmiss_ensemble_iterator *iterator)
{
	if ((!iterator) || (NULL == iterator->getEnsemble()))
		return 0;
	iterator->increment();
	return (iterator->ref != 0);
}

inline bool Field_ensemble_group::hasEntry(const Cmiss_ensemble_iterator *iterator) const
{
	if (iterator->getEnsemble() != ensemble)
		return 0;
	return hasEntryRef(iterator->getRef());
}

inline int Field_ensemble_group::setEntry(const Cmiss_ensemble_iterator *iterator, bool inGroup)
{
	if (iterator->getEnsemble() != ensemble)
		return 0;
	return setEntryRef(iterator->getRef(), inGroup);
}

inline int Field_ensemble_group::incrementEnsembleEntry(Cmiss_ensemble_iterator *iterator)
{
	if (!iterator || (iterator->getEnsemble() != ensemble))
		return 0;
	iterator->setRef(getNextEntryRef(iterator->ref));
	return (iterator->getRef() != 0);
}

} // namespace Cmiss

#endif /* !defined (FIELD_ENSEMBLE_HPP) */
