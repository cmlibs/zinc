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
}
#include "computed_field/computed_field_private.hpp"
#include "computed_field/field_ensemble.hpp"
#include "general/block_array.hpp"
//#include "user_interface/message.h"

namespace {

char field_ensemble_type_string[] = "ensemble";

typedef std::map<EnsembleEntryIdentifier,EnsembleEntryRef> EnsembleEntryMap;
typedef std::map<EnsembleEntryIdentifier,EnsembleEntryRef>::iterator EnsembleEntryMapIterator;

class Field_ensemble : public Computed_field_core
{
private:
	bool dense;
	block_array<EnsembleEntryRef,EnsembleEntryIdentifier> entries; // used only if not dense
	EnsembleEntryMap identifierMap; // used only if not dense
	EnsembleEntryIdentifier firstFreeIdentifier, lastIdentifier;
	EnsembleEntryRef entryCount, maxRef;

public:
	Field_ensemble() :
		Computed_field_core(),
		dense(true),
		firstFreeIdentifier(1),
		lastIdentifier(0),
		entryCount(0),
		maxRef(0)
	{
	};

	~Field_ensemble()
	{
	}
	
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

	char* get_command_string();
	
	void updateFirstFreeIdentifier();
	void notDense();
	void notSorted();
	EnsembleEntryRef createEntryPrivate(EnsembleEntryIdentifier identifier);

public:

	EnsembleEntryRef createEntry();
	EnsembleEntryRef createEntry(EnsembleEntryIdentifier identifier);
	EnsembleEntryRef findOrCreateEntry(EnsembleEntryIdentifier identifier);
	EnsembleEntryRef findEntry(EnsembleEntryIdentifier identifier);
	int removeEntry(EnsembleEntryRef ref);
	int removeEntryIdentifier(EnsembleEntryIdentifier identifier);
	EnsembleEntryIdentifier getIdentifier(EnsembleEntryRef ref);

};

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
			EnsembleEntryIdentifier identifier = getIdentifier(ref);
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
	return (1);
}

char *Field_ensemble::get_command_string()
{
	return NULL;
}

inline void Field_ensemble::updateFirstFreeIdentifier()
{
	if (firstFreeIdentifier != (lastIdentifier + 1))
	{
		while (findEntry(firstFreeIdentifier))
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
		for (EnsembleEntryIdentifier identifier = 1; identifier <= lastIdentifier; identifier++)
		{
			entries.setValue(/*index*/identifier-1, identifier);
			identifierMap[identifier] = identifier;
		}
	}
}

/** private: caller must have checked identifier is not in use! */
inline EnsembleEntryRef Field_ensemble::createEntryPrivate(EnsembleEntryIdentifier identifier)
{
	if (identifier == 0)
		return 0;
	EnsembleEntryRef ref = 0;
	if (dense && (identifier > lastIdentifier + 1))
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
		// as entries are deleted [in parameter fields indexed by this ensemble].
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
inline EnsembleEntryRef Field_ensemble::createEntry()
{
	updateFirstFreeIdentifier();
	return createEntryPrivate(firstFreeIdentifier);
}

/** fails if identifier already in use */
inline EnsembleEntryRef Field_ensemble::createEntry(EnsembleEntryIdentifier identifier)
{
	EnsembleEntryRef ref = findEntry(identifier);
	if (ref)
		return 0;
	return createEntryPrivate(identifier);
}

/** finds existing entry with identifier, or creates new one if none */
inline EnsembleEntryRef Field_ensemble::findOrCreateEntry(EnsembleEntryIdentifier identifier)
{
	EnsembleEntryRef ref = findEntry(identifier);
	if (ref)
		return ref;
	return createEntryPrivate(identifier);
}

inline EnsembleEntryRef Field_ensemble::findEntry(EnsembleEntryIdentifier identifier)
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

inline int Field_ensemble::removeEntry(EnsembleEntryRef ref)
{
	if ((ref == 0) || (ref > maxRef))
		return 0;
	// GRC must check not in use
	if (dense)
	{
		if (static_cast<EnsembleEntryIdentifier>(ref) == lastIdentifier)
		{
			lastIdentifier--;
			firstFreeIdentifier--;
			entryCount--;
			return 1;
		}
		else
		{
			notDense();
		}
	}
	// not dense
	EnsembleEntryIdentifier identifier;
	if (entries.getValue(/*index*/(ref-1), identifier))
	{
		// GRC must check not in use:
		identifierMap.erase(identifier);
		entries.setValue(/*index*/(ref-1), 0);
		entryCount--;
		return 1;
	}
	return 0;
}

inline int Field_ensemble::removeEntryIdentifier(EnsembleEntryIdentifier identifier)
{
	EnsembleEntryRef ref = findEntry(identifier);
	if (ref)
		return removeEntry(ref);
	return 0;
}

} //namespace

struct Computed_field *Cmiss_field_create_ensemble(
	struct Cmiss_field_module *field_module)
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/false,
		/*number_of_components*/1,
		/*number_of_source_fields*/0, NULL,
		/*number_of_source_values*/0, NULL,
		new Field_ensemble());

	return (field);
}
