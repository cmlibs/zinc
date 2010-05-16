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

char field_ensemble_type_string[] = "ensemble";

class Field_ensemble : public Computed_field_core
{
private:

typedef std::map<Cmiss_ensemble_identifier,EnsembleEntryRef> EnsembleEntryMap;
typedef std::map<Cmiss_ensemble_identifier,EnsembleEntryRef>::iterator EnsembleEntryMapIterator;

	bool dense; // dense is true if all entries from 1..lastIdentifier exist and are in order
	block_array<EnsembleEntryRef,Cmiss_ensemble_identifier> entries; // used only if not dense
	EnsembleEntryMap identifierMap; // used only if not dense
	Cmiss_ensemble_identifier firstFreeIdentifier, lastIdentifier;
	EnsembleEntryRef entryCount, maxRef;

	// linked-lists of active iterators is maintained to keep track of entry refs for
	// reclaiming memory. When destroyed these are placed on the available list for
	// re-use without hitting the heap.
	Cmiss_ensemble_iterator *activeIterators, *availableIterators;
	
public:
	Field_ensemble() :
		Computed_field_core(),
		dense(true),
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

	char *get_type_string()
	{
		return (field_ensemble_type_string);
	}

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
	void notDense();
	void notSorted();
	EnsembleEntryRef createEntryPrivate(Cmiss_ensemble_identifier identifier);

public:

	EnsembleEntryRef createEntry();
	EnsembleEntryRef createEntry(Cmiss_ensemble_identifier identifier);
	EnsembleEntryRef findOrCreateEntry(Cmiss_ensemble_identifier identifier);
	EnsembleEntryRef findEntryByIdentifier(Cmiss_ensemble_identifier identifier);
	int removeEntry(EnsembleEntryRef ref);
	int removeEntryWithIdentifier(Cmiss_ensemble_identifier identifier);
	Cmiss_ensemble_identifier getEntryIdentifier(EnsembleEntryRef ref);
	EnsembleEntryRef getFirstEntryRef();
	EnsembleEntryRef getNextEntryRef(EnsembleEntryRef ref);
	EnsembleEntryRef getNextEntryRefBoolTrue(EnsembleEntryRef ref, bool_array<EnsembleEntryRef>& values);

	static int incrementEnsembleEntry(Cmiss_ensemble_iterator *iterator);
	
	/** packages Ref with ensemble pointer for external use */
	Cmiss_ensemble_iterator *createEnsembleEntry(EnsembleEntryRef ref);
	static void freeEnsembleEntry(Cmiss_ensemble_iterator *&iterator);
};


char field_ensemble_group_type_string[] = "ensemble_group";

class Field_ensemble_group : public Computed_field_core
{
private:
	Field_ensemble *ensemble;
	bool_array<EnsembleEntryRef> values;
	
public:
	Field_ensemble_group(Field_ensemble *from_ensemble) :
		Computed_field_core(),
		ensemble(from_ensemble)
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

	char *get_type_string()
	{
		return (field_ensemble_group_type_string);
	}

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

	void clear();

	/** be careful that ref is for this ensemble */
	bool hasEntryRef(EnsembleEntryRef ref)
	{
		return values.getBool(ref);
	}

	/** be careful that ref is for this ensemble */
	int setEntryRef(EnsembleEntryRef ref, bool inGroup)
	{
		return values.setBool(ref, inGroup);
	}

	bool hasEntry(Cmiss_ensemble_iterator *iterator);
	int setEntry(Cmiss_ensemble_iterator *iterator, bool inGroup);

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

	void setRef(Cmiss::EnsembleEntryRef newRef) { ref = newRef; }

public:
	Cmiss::Field_ensemble *getEnsemble() { return ensemble; }
	Cmiss::EnsembleEntryRef getRef() { return ref; }
	Cmiss_ensemble_identifier getIdentifier() { return ensemble->getEntryIdentifier(ref); }

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
 */
struct Cmiss_ensemble_index
{
	class Indexing
	{
		Cmiss_field_ensemble *field_ensemble;
		// only zero or one of following set
		Cmiss_ensemble_iterator *iterator;
		Cmiss_field_ensemble_group *field_ensemble_group;
	public:
		Indexing() :
			field_ensemble(NULL),
			iterator(NULL),
			field_ensemble_group(NULL)
		{
		}

		void setEnsemble(Cmiss_field_ensemble *in_field_ensemble)
		{
			Cmiss_field_access(reinterpret_cast<Cmiss_field *>(field_ensemble));
			field_ensemble = in_field_ensemble;
		}

		~Indexing()
		{
			Cmiss_field_destroy(reinterpret_cast<Cmiss_field **>(&field_ensemble));
			if (iterator)
				Cmiss_ensemble_iterator_destroy(&iterator);
			if (field_ensemble_group)
				Cmiss_field_destroy(reinterpret_cast<Cmiss_field **>(&field_ensemble_group));
		}
	};
private:
	Cmiss_field *indexee;
	int number_of_ensembles;
	Indexing *indexing;

	Cmiss_ensemble_index(Cmiss_field *in_indexee,
		int in_number_of_ensembles, Cmiss_field_ensemble **in_ensembles) :
			indexee(Cmiss_field_access(in_indexee)),
			number_of_ensembles(in_number_of_ensembles),
			indexing(new Indexing[number_of_ensembles])
	{
		if (in_ensembles)
		{
			for (int i = 0; i < number_of_ensembles; i++)
			{
				indexing[i].setEnsemble(in_ensembles[i]);
			}
		}
	}

public:
	
	static Cmiss_ensemble_index *create(Cmiss_field *in_indexee,
		int in_number_of_ensembles, Cmiss_field_ensemble **in_ensembles)
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

	int setEntry(Cmiss_ensemble_iterator *iterator);
	int clearEntries(Cmiss_field_ensemble *ensemble_field);
};

namespace Cmiss
{

inline int Field_ensemble::incrementEnsembleEntry(Cmiss_ensemble_iterator *iterator)
{
	if ((!iterator) || (NULL == iterator->getEnsemble()))
		return 0;
	iterator->setRef(iterator->getEnsemble()->getNextEntryRef(iterator->ref));
	return (iterator->ref != 0);
}

inline bool Field_ensemble_group::hasEntry(Cmiss_ensemble_iterator *iterator)
{
	if (iterator->getEnsemble() != ensemble)
		return 0;
	return hasEntryRef(iterator->getRef());
}

inline int Field_ensemble_group::setEntry(Cmiss_ensemble_iterator *iterator, bool inGroup)
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
