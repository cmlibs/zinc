/***************************************************************************//**
 * FILE : field_ensemble.cpp
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
#include <map>
extern "C" {
#include "computed_field/computed_field.h"
#include "user_interface/message.h"
}
#include "computed_field/field_ensemble.hpp"

namespace Cmiss
{

Field_ensemble::~Field_ensemble()
{
	Cmiss_ensemble_iterator *iterator;
	while (availableIterators)
	{
		iterator = availableIterators->next;
		DEALLOCATE(availableIterators);
		availableIterators = iterator;
	}
	// can't free externally held entries, hence just clear
	// ensemble pointer so cleaned up referencing object
	iterator = activeIterators;
	while (iterator)
	{
		iterator->ensemble = NULL;
		iterator = iterator->next;
	}
}

int Field_ensemble::evaluate_cache_at_location(Field_location* location)
{
	int return_code = 0;
	if (field && location)
	{
		field->values[0] = 0;
#if defined (FUTURE_CODE)
		EnsembleEntryRef ref;
		if (location.getEnsembleEntry(this, ref))
		{
			Cmiss_ensemble_identifier identifier = getIdentifier(ref);
			if (0 < identifier)
			{
				field->values[0] = static_cast<FE_value>(identifier);
				return_code = 1;
			}
		}
#endif
		field->derivatives_valid = 0;
	}
	return (return_code);
}

int Field_ensemble::list()
{
	display_message(INFORMATION_MESSAGE, "    entry count: %d\n", entryCount);
	return (1);
}

void Field_ensemble::updateFirstFreeIdentifier()
{
	if (firstFreeIdentifier != (lastIdentifier + 1))
	{
		while (findEntryByIdentifier(firstFreeIdentifier))
		{
			firstFreeIdentifier++;
		}
	}
}

void Field_ensemble::notDense()
{
	if (dense)
	{
		dense = false;
		// this can be optimised:
		for (Cmiss_ensemble_identifier identifier = 1; identifier <= lastIdentifier; identifier++)
		{
			entries.setValue(/*index*/identifier-1, identifier);
			identifierMap[identifier] = static_cast<EnsembleEntryRef>(identifier);
		}
	}
}

/** private: caller must have checked identifier is not in use! */
EnsembleEntryRef Field_ensemble::createEntryPrivate(Cmiss_ensemble_identifier identifier)
{
	if (identifier == 0)
		return 0;
	EnsembleEntryRef ref = 0;
	if (dense && (identifier > firstFreeIdentifier))
	{
		notDense();
	}
	if (dense)
	{
		ref = static_cast<EnsembleEntryRef>(identifier);
		lastIdentifier = identifier;
		firstFreeIdentifier = lastIdentifier + 1;
		entryCount++;
		maxRef++;
	}
	else
	{
		// Future memory optimisation: reclaim unused indexes for new entries.
		// Note for reclaim to work would have to clear indexes into parameters
		// as entries are deleted [for parameter fields indexed by this ensemble].
		if (entries.setValue(/*index*/maxRef, identifier))
		{
			ref = maxRef + 1;
			identifierMap[identifier] = ref;
			if (identifier > lastIdentifier)
			{
				lastIdentifier = identifier;
			}
			if (identifier == firstFreeIdentifier)
			{
				firstFreeIdentifier++;
			}
			entryCount++;
			maxRef++;
		}
	}
	return ref;
}

/** create with auto-generated unique identifier */
EnsembleEntryRef Field_ensemble::createEntry()
{
	updateFirstFreeIdentifier();
	return createEntryPrivate(firstFreeIdentifier);
}

/** fails if identifier already in use */
EnsembleEntryRef Field_ensemble::createEntry(Cmiss_ensemble_identifier identifier)
{
	EnsembleEntryRef ref = findEntryByIdentifier(identifier);
	if (ref)
		return 0;
	return createEntryPrivate(identifier);
}

/** finds existing entry with identifier, or creates new one if none */
EnsembleEntryRef Field_ensemble::findOrCreateEntry(Cmiss_ensemble_identifier identifier)
{
	EnsembleEntryRef ref = findEntryByIdentifier(identifier);
	if (ref)
		return ref;
	return createEntryPrivate(identifier);
}

EnsembleEntryRef Field_ensemble::findEntryByIdentifier(Cmiss_ensemble_identifier identifier)
{
	EnsembleEntryRef ref = 0;
	if (identifier > 0)
	{
		if (dense)
		{
			if (identifier <= lastIdentifier)
			{
				ref = static_cast<EnsembleEntryRef>(identifier);
			}
		}
		else
		{
			EnsembleEntryMapIterator iter = identifierMap.find(identifier);
			if (iter != identifierMap.end())
			{
				ref = iter->second;
			}
		}
	}
	return ref;
}

int Field_ensemble::removeEntry(EnsembleEntryRef ref)
{
	if ((ref == 0) || (ref > maxRef))
		return 0;
	// GRC must check not in use
	if (dense)
	{
#if defined (FUTURE_CODE)
		if (static_cast<Cmiss_ensemble_identifier>(ref) == lastIdentifier)
		{
			lastIdentifier--;
			firstFreeIdentifier--;
			entryCount--;
			// GRC can't reduce maxRef yet: must clear parameters for reclaim
			maxRef--;
			return 1;
		}
		else
		{
			notDense();
		}
#else
		notDense();
#endif
	}
	Cmiss_ensemble_identifier identifier;
	if (entries.getValue(/*index*/(ref-1), identifier))
	{
		identifierMap.erase(identifier);
		entries.setValue(/*index*/(ref-1), 0);
		if (identifier <= firstFreeIdentifier)
			firstFreeIdentifier = identifier;
		entryCount--;
		if (identifier == lastIdentifier)
		{
			if (0 == entryCount)
			{
				lastIdentifier = 0;
				firstFreeIdentifier = 1;
			}
			else
			{
				EnsembleEntryMapIterator iter = identifierMap.end();
				iter--;
				EnsembleEntryRef lastRef = iter->second;
				entries.getValue(/*index*/(lastRef-1), lastIdentifier);
			}
		}
		return 1;
	}
	return 0;
}

int Field_ensemble::removeEntryWithIdentifier(Cmiss_ensemble_identifier identifier)
{
	EnsembleEntryRef ref = findEntryByIdentifier(identifier);
	if (ref)
		return removeEntry(ref);
	return 0;
}

Cmiss_ensemble_identifier Field_ensemble::getEntryIdentifier(EnsembleEntryRef ref)
{
	Cmiss_ensemble_identifier identifier = 0;
	if (ref > 0)
	{
		entries.getValue(/*index*/(ref-1), identifier);
	}
	return identifier;
}

EnsembleEntryRef Field_ensemble::getFirstEntryRef()
{
	if (0 == entryCount)
		return 0;
	if (dense)
		return 1;
	return identifierMap.begin()->second;
}

EnsembleEntryRef Field_ensemble::getNextEntryRef(EnsembleEntryRef ref)
{
	if (0 == ref)
		return 0;
	if (dense)
	{
		if (ref < maxRef)
			return (ref + 1);
	}
	else
	{
		// optimisation: check if ref+1 -> identifier+1 so it is next 
		Cmiss_ensemble_identifier identifier = getEntryIdentifier(ref);
		if (getEntryIdentifier(ref + 1) == (identifier + 1))
			return ref + 1;
		// O(logN) slow:
		// can be made more efficient by passing Cmiss_ensemble_iterator
		// to this function & keeping iterator in it
		EnsembleEntryMapIterator iter = identifierMap.find(identifier);
		iter++;
		if (iter != identifierMap.end())
			return iter->second;
	}
	return 0;
}

EnsembleEntryRef Field_ensemble::getNextEntryRefBoolTrue(EnsembleEntryRef ref,
	bool_array<EnsembleEntryRef>& values)
{
	if (0 == ref)
		return 0;
	EnsembleEntryRef newRef = ref;
	if (dense)
	{
		do
		{
			newRef++;
			if (newRef > maxRef)
			{
				newRef = 0;
				break;
			}
		} while (!values.getBool(newRef));
	}
	else
	{
		// can be made more efficient by passing Cmiss_ensemble_iterator
		// to this function & keeping iterator in it
		Cmiss_ensemble_identifier identifier = getEntryIdentifier(newRef);
		EnsembleEntryMapIterator iter = identifierMap.find(identifier);
		do
		{
			iter++;
			if (iter == identifierMap.end())
			{
				newRef = 0;
				break;
			}
			newRef = iter->second;
		} while (!values.getBool(newRef));
	}
	return newRef;
}

Cmiss_ensemble_iterator *Field_ensemble::createEnsembleEntry(EnsembleEntryRef ref)
{
	if (0 == getEntryIdentifier(ref))
		return NULL;
	Cmiss_ensemble_iterator *iterator = NULL;
	if (availableIterators)
	{
		iterator = availableIterators;
		availableIterators = iterator->next;
		if (availableIterators)
			availableIterators->previous = NULL;
	}
	else
	{
		ALLOCATE(iterator, struct Cmiss_ensemble_iterator, 1);
	}
	if (iterator)
	{
		iterator->ensemble = this;
		iterator->ref = ref;
		iterator->next = activeIterators;
		iterator->previous = NULL;
		if (activeIterators)
			activeIterators->previous = iterator;
		activeIterators = iterator;
	}
	return iterator;
}

/* static */
void Field_ensemble::freeEnsembleEntry(Cmiss_ensemble_iterator *&iterator)
{
	if (iterator && iterator->ensemble)
	{
		if (iterator->previous)
		{
			iterator->previous->next = iterator->next;
			iterator->previous = NULL;
		}
		if (iterator->next)
			iterator->next->previous = iterator->previous;
		iterator->next = iterator->ensemble->availableIterators;
		if (iterator->ensemble->availableIterators)
			iterator->ensemble->availableIterators->previous = iterator;
		iterator->ensemble->availableIterators = iterator;
		iterator->ensemble = NULL;
		iterator = NULL;
	}
	else
	{
		DEALLOCATE(iterator);
	}
}

int Field_ensemble_group::evaluate_cache_at_location(Field_location* location)
{
	int return_code = 0;
	if (field && location)
	{
		field->values[0] = 0;
#if defined (FUTURE_CODE)
		EnsembleEntryRef ref;
		if (location.getEnsembleEntry(this, ref))
		{
			field->values = hasEntry(ref) ? 1.0 : 0.0;
			return_code = 1;
		}
#endif
		field->derivatives_valid = 0;
	}
	return (return_code);
}

int Field_ensemble_group::list()
{
	if (field)
	{
		display_message(INFORMATION_MESSAGE, "    ensemble: %d\n", field->source_fields[0]->name);
	}
	else
	{
		display_message(ERROR_MESSAGE, "Field_ensemble_group::list.  Missing field");
	}
	return (1);
}

} // namespace Cmiss

struct Computed_field *Cmiss_field_module_create_ensemble(
	struct Cmiss_field_module *field_module)
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/false,
		/*number_of_components*/1,
		/*number_of_source_fields*/0, NULL,
		/*number_of_source_values*/0, NULL,
		new Cmiss::Field_ensemble());

	return (field);
}

Cmiss_field_ensemble *Cmiss_field_cast_ensemble(Cmiss_field_id field)
{
	if (dynamic_cast<Cmiss::Field_ensemble*>(field->core))
	{
		Cmiss_field_access(field);
		return (reinterpret_cast<Cmiss_field_ensemble *>(field));
	}
	else
	{
		return (NULL);
	}
}

struct Cmiss_ensemble_iterator *Cmiss_field_ensemble_create_entry(
	struct Cmiss_field_ensemble *ensemble_field)
{
	struct Cmiss_ensemble_iterator *iterator = NULL;
	if (ensemble_field)
	{
		Cmiss::Field_ensemble *ensemble = Cmiss_field_ensemble_core_cast(ensemble_field);
		Cmiss::EnsembleEntryRef ref = ensemble->createEntry();
		iterator = ensemble->createEnsembleEntry(ref);
	}
	return (iterator);
}

struct Cmiss_ensemble_iterator *Cmiss_field_ensemble_create_entry_with_identifier(
	struct Cmiss_field_ensemble *ensemble_field, Cmiss_ensemble_identifier identifier)
{
	struct Cmiss_ensemble_iterator *iterator = NULL;
	if (ensemble_field)
	{
		Cmiss::Field_ensemble *ensemble = Cmiss_field_ensemble_core_cast(ensemble_field);
		Cmiss::EnsembleEntryRef ref = ensemble->createEntry(identifier);
		iterator = ensemble->createEnsembleEntry(ref);
	}
	return (iterator);
}

struct Cmiss_ensemble_iterator *Cmiss_field_ensemble_find_entry_by_identifier(
	struct Cmiss_field_ensemble *ensemble_field, Cmiss_ensemble_identifier identifier)
{
	struct Cmiss_ensemble_iterator *iterator = NULL;
	if (ensemble_field)
	{
		Cmiss::Field_ensemble *ensemble = Cmiss_field_ensemble_core_cast(ensemble_field);
		Cmiss::EnsembleEntryRef ref = ensemble->findEntryByIdentifier(identifier);
		iterator = ensemble->createEnsembleEntry(ref);
	}
	return (iterator);
}

struct Cmiss_ensemble_iterator *Cmiss_field_ensemble_find_or_create_entry(
	struct Cmiss_field_ensemble *ensemble_field, Cmiss_ensemble_identifier identifier)
{
	struct Cmiss_ensemble_iterator *iterator = NULL;
	if (ensemble_field)
	{
		Cmiss::Field_ensemble *ensemble = Cmiss_field_ensemble_core_cast(ensemble_field);
		Cmiss::EnsembleEntryRef ref = ensemble->findEntryByIdentifier(identifier);
		if (!ref)
		{
			ref = ensemble->createEntry(identifier);
		}
		iterator = ensemble->createEnsembleEntry(ref);
	}
	return (iterator);
}

struct Cmiss_ensemble_iterator *Cmiss_field_ensemble_get_first_entry(
	struct Cmiss_field_ensemble *ensemble_field)
{
	struct Cmiss_ensemble_iterator *iterator = NULL;
	if (ensemble_field)
	{
		Cmiss::Field_ensemble *ensemble = Cmiss_field_ensemble_core_cast(ensemble_field);
		Cmiss::EnsembleEntryRef ref = ensemble->getFirstEntryRef();
		iterator = ensemble->createEnsembleEntry(ref);
	}
	return (iterator);
}


int Cmiss_ensemble_iterator_destroy(struct Cmiss_ensemble_iterator **iterator_address)
{
	if ((iterator_address) && (*iterator_address))
	{
		Cmiss::Field_ensemble::freeEnsembleEntry(*iterator_address);
		return 1;
	}
	return 0;
}

Cmiss_ensemble_identifier Cmiss_ensemble_iterator_get_identifier(struct Cmiss_ensemble_iterator *iterator)
{
	Cmiss_ensemble_identifier identifier = 0;
	if (iterator && (iterator->getEnsemble()))
	{
		identifier = iterator->getEnsemble()->getEntryIdentifier(iterator->getRef());
	}
	return identifier;
}

int Cmiss_ensemble_iterator_increment(struct Cmiss_ensemble_iterator *iterator)
{
	return Cmiss::Field_ensemble::incrementEnsembleEntry(iterator);
}

struct Computed_field *Cmiss_field_module_create_ensemble_group(
	struct Cmiss_field_module *field_module, struct Cmiss_field_ensemble *ensemble_field)
{
	Computed_field *field = NULL;
	if (ensemble_field)
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			/*number_of_components*/1,
			/*number_of_source_fields*/1, reinterpret_cast<struct Computed_field **>(&ensemble_field),
			/*number_of_source_values*/0, NULL,
			new Cmiss::Field_ensemble_group(Cmiss_field_ensemble_core_cast(ensemble_field)));
	}
	return (field);
}

Cmiss_field_ensemble_group *Cmiss_field_cast_ensemble_group(Cmiss_field_id field)
{
	if (dynamic_cast<Cmiss::Field_ensemble_group*>(field->core))
	{
		Cmiss_field_access(field);
		return (reinterpret_cast<Cmiss_field_ensemble_group *>(field));
	}
	else
	{
		return (NULL);
	}
}

int Cmiss_field_ensemble_group_clear(
	struct Cmiss_field_ensemble_group *ensemble_group_field)
{
	if (NULL == ensemble_group_field)
		return 0;
	Cmiss::Field_ensemble_group *ensemble_group =
		Cmiss_field_ensemble_group_core_cast(ensemble_group_field);
	ensemble_group->clear();
	return 1;
}

int Cmiss_field_ensemble_group_has_entry(
	struct Cmiss_field_ensemble_group *ensemble_group_field, struct Cmiss_ensemble_iterator *iterator)
{
	if ((NULL == ensemble_group_field) || (NULL == iterator))
		return 0;
	Cmiss::Field_ensemble_group *ensemble_group =
		Cmiss_field_ensemble_group_core_cast(ensemble_group_field);
	return ensemble_group->hasEntry(iterator);
}

int Cmiss_field_ensemble_group_add_entry(
	struct Cmiss_field_ensemble_group *ensemble_group_field, struct Cmiss_ensemble_iterator *iterator)
{
	if ((NULL == ensemble_group_field) || (NULL == iterator))
		return 0;
	Cmiss::Field_ensemble_group *ensemble_group =
		Cmiss_field_ensemble_group_core_cast(ensemble_group_field);
	return ensemble_group->setEntry(iterator, true);
}

int Cmiss_field_ensemble_group_remove_entry(
	struct Cmiss_field_ensemble_group *ensemble_group_field, struct Cmiss_ensemble_iterator *iterator)
{
	if ((NULL == ensemble_group_field) || (NULL == iterator))
		return 0;
	Cmiss::Field_ensemble_group *ensemble_group =
		Cmiss_field_ensemble_group_core_cast(ensemble_group_field);
	return ensemble_group->setEntry(iterator, false);
}

int Cmiss_field_ensemble_group_increment_entry(
	struct Cmiss_field_ensemble_group *ensemble_group_field, struct Cmiss_ensemble_iterator *iterator)
{
	if ((NULL == ensemble_group_field) || (NULL == iterator))
		return 0;
	Cmiss::Field_ensemble_group *ensemble_group =
		Cmiss_field_ensemble_group_core_cast(ensemble_group_field);
	return ensemble_group->incrementEnsembleEntry(iterator);
}

int Cmiss_ensemble_index_destroy(struct Cmiss_ensemble_index **index_address)
{
	if ((index_address) && (*index_address))
	{
		delete *index_address;
		*index_address = NULL;
		return 1;
	}
	return 0;
}

int Cmiss_ensemble_index_set_all_ensemble(struct Cmiss_ensemble_index *index,
	Cmiss_field_ensemble *ensemble_field)
{
	if ((!index) || (!ensemble_field))
		return 0;
	return index->setAllEnsemble(ensemble_field);
}

int Cmiss_ensemble_index_set_entry(struct Cmiss_ensemble_index *index,
	Cmiss_ensemble_iterator *iterator)
{
	if ((!index) || (!iterator))
		return 0;
	return index->setEntry(iterator);
}

int Cmiss_ensemble_index_set_group(struct Cmiss_ensemble_index *index,
	Cmiss_field_ensemble_group *ensemble_group_field)
{
	if ((!index) || (!ensemble_group_field))
		return 0;
	return index->setGroup(ensemble_group_field);
}
