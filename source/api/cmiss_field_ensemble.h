/***************************************************************************//**
 * FILE : cmiss_field_ensemble.h
 * 
 * Implements a domain field consisting of set of indexed entries.
 * Warning: prototype!
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

#if !defined (CMISS_FIELD_ENSEMBLE_H)
#define CMISS_FIELD_ENSEMBLE_H

#include "api/cmiss_field.h"
#include "api/cmiss_field_module.h"

/***************************************************************************//**
 * The ensemble type specific handle to an ensemble Cmiss_field.
 * This is a domain comprising a set of entries with unique identifiers.
 */
struct Cmiss_field_ensemble;
typedef struct Cmiss_field_ensemble *Cmiss_field_ensemble_id;


/***************************************************************************//**
 * The ensemble type specific handle to a ensemble group Cmiss_field.
 * This field stores a subset of entries from an ensemble.
 * When evaluated, returns 1 for entries in the group, 0 otherwise.
 */
struct Cmiss_field_ensemble_group;
typedef struct Cmiss_field_ensemble_group *Cmiss_field_ensemble_group_id;


/***************************************************************************//**
 * The numeric type of the unique identifier for each entry in an ensemble.
 */
typedef unsigned int Cmiss_ensemble_identifier;


/***************************************************************************//**
 * Iterator-handle for referencing an entry in a Cmiss_field_ensemble.
 */
struct Cmiss_ensemble_iterator;
typedef struct Cmiss_ensemble_iterator *Cmiss_ensemble_iterator_id;


/***************************************************************************//**
 * Index to a selection of entries from N ensembles.
 * Used for getting/setting parameters.
 */
struct Cmiss_ensemble_index;
typedef struct Cmiss_ensemble_index *Cmiss_ensemble_index_id;



Cmiss_field_id Cmiss_field_module_create_ensemble(Cmiss_field_module_id field_module);

Cmiss_field_ensemble_id Cmiss_field_cast_ensemble(Cmiss_field_id field);

/***************************************************************************//**
 * Create new entry in ensemble, unique identifier automatically generated
 * @param  ensemble_field  The ensemble to add the entry in.
 * @return  Iterator-handle to new entry in ensemble.
 */
Cmiss_ensemble_iterator_id Cmiss_field_ensemble_create_entry(
	Cmiss_field_ensemble_id ensemble_field);

Cmiss_ensemble_iterator_id Cmiss_field_ensemble_create_entry_with_identifier(
	Cmiss_field_ensemble_id ensemble_field, Cmiss_ensemble_identifier identifier);

Cmiss_ensemble_iterator_id Cmiss_field_ensemble_find_entry_by_identifier(
	Cmiss_field_ensemble_id ensemble_field, Cmiss_ensemble_identifier identifier);

Cmiss_ensemble_iterator_id Cmiss_field_ensemble_find_or_create_entry(
	Cmiss_field_ensemble_id ensemble_field, Cmiss_ensemble_identifier identifier);

Cmiss_ensemble_iterator_id Cmiss_field_ensemble_get_first_entry(
	Cmiss_field_ensemble_id ensemble_field);



int Cmiss_ensemble_iterator_destroy(Cmiss_ensemble_iterator_id *entry_address);

Cmiss_ensemble_identifier Cmiss_ensemble_iterator_get_identifier(
	Cmiss_ensemble_iterator_id iterator);

/***************************************************************************//**
 * Iterate to next entry in same ensemble.
 * @param iterator  Iterator to be incremented. Invalidated if past last entry.
 * @return  1 on success, 0 if no more entries or iterator invalid.
 */
int Cmiss_ensemble_iterator_increment(Cmiss_ensemble_iterator_id iterator);



Cmiss_field_id Cmiss_field_module_create_ensemble_group(Cmiss_field_module_id field_module,
	Cmiss_field_ensemble_id ensemble_field);

Cmiss_field_ensemble_group_id Cmiss_field_cast_ensemble_group(Cmiss_field_id field);

/***************************************************************************//**
 * Remove all entries from group.
 */
int Cmiss_field_ensemble_group_clear(
	Cmiss_field_ensemble_group_id ensemble_group_field);

int Cmiss_field_ensemble_group_has_entry(
	Cmiss_field_ensemble_group_id ensemble_group_field, Cmiss_ensemble_iterator_id iterator);

int Cmiss_field_ensemble_group_add_entry(
	Cmiss_field_ensemble_group_id ensemble_group_field, Cmiss_ensemble_iterator_id iterator);

int Cmiss_field_ensemble_group_remove_entry(
		Cmiss_field_ensemble_group_id ensemble_group_field, Cmiss_ensemble_iterator_id iterator);

Cmiss_ensemble_iterator_id Cmiss_field_ensemble_group_get_first_entry(
	Cmiss_field_ensemble_group_id ensemble_group_field);

/***************************************************************************//**
 * Iterate to next entry in this group.
 * @param ensemble_group_field  Ensemble group to iterate over.
 * @param iterator  Iterator to be incremented. Invalidated if past last entry.
 * @return  1 on success, 0 if no more entries or iterator invalid, including if
 * not from the same ensemble as this group.
 */
int Cmiss_field_ensemble_group_increment_entry(Cmiss_field_ensemble_group_id ensemble_group_field,
	Cmiss_ensemble_iterator_id iterator);



int Cmiss_ensemble_index_destroy(Cmiss_ensemble_index_id *index_address);

/***************************************************************************//**
 * Set index to span all entries in this ensemble, in order of increasing
 * identifier.
 * @return  1 on success, 0 if ensemble is not part of this index.
 */
int Cmiss_ensemble_index_set_all_ensemble(Cmiss_ensemble_index_id index,
	Cmiss_field_ensemble_id ensemble_field);

/***************************************************************************//**
 * Set index to span a single entry for the iterator's ensemble.
 * @return  1 on success, 0 if ensemble is not part of this index.
 */
int Cmiss_ensemble_index_set_entry(Cmiss_ensemble_index_id index,
	Cmiss_ensemble_iterator_id iterator);

/***************************************************************************//**
 * Set index to span all entries in this group for its ensemble, in order of
 * increasting identifier.
 * @return  1 on success, 0 if ensemble is not part of this index.
 */
int Cmiss_ensemble_index_set_group(Cmiss_ensemble_index_id index,
	Cmiss_field_ensemble_group_id ensemble_group_field);


#endif /* !defined (CMISS_FIELD_ENSEMBLE_H) */
