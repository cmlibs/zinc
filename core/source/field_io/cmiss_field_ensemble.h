/***************************************************************************//**
 * FILE : cmiss_field_ensemble.h
 *
 * Implements a domain field consisting of set of indexed entries.
 * Warning: prototype!
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (CMZN_FIELD_ENSEMBLE_H)
#define CMZN_FIELD_ENSEMBLE_H

#include "zinc/zincsharedobject.h"
#include "zinc/types/fieldid.h"
#include "field_io/cmiss_field_ensemble_id.h"
#include "zinc/types/fieldmoduleid.h"

cmzn_field_id cmzn_fieldmodule_create_field_ensemble(cmzn_fieldmodule_id field_module);

cmzn_field_ensemble_id cmzn_field_cast_ensemble(cmzn_field_id field);

ZINC_C_INLINE cmzn_field_id cmzn_field_ensemble_base_cast(cmzn_field_ensemble_id ensemble)
{
	return (cmzn_field_id)(ensemble);
}

int cmzn_field_ensemble_destroy(cmzn_field_ensemble_id *ensemble_address);

/***************************************************************************//**
 * Create new entry in ensemble, unique identifier automatically generated
 * @param  ensemble_field  The ensemble to add the entry in.
 * @return  Iterator-handle to new entry in ensemble.
 */
cmzn_ensemble_iterator_id cmzn_field_ensemble_create_entry(
	cmzn_field_ensemble_id ensemble_field);

cmzn_ensemble_iterator_id cmzn_field_ensemble_create_entry_with_identifier(
	cmzn_field_ensemble_id ensemble_field, cmzn_ensemble_identifier identifier);

cmzn_ensemble_iterator_id cmzn_field_ensemble_find_entry_by_identifier(
	cmzn_field_ensemble_id ensemble_field, cmzn_ensemble_identifier identifier);

cmzn_ensemble_iterator_id cmzn_field_ensemble_find_or_create_entry(
	cmzn_field_ensemble_id ensemble_field, cmzn_ensemble_identifier identifier);

cmzn_ensemble_iterator_id cmzn_field_ensemble_get_first_entry(
	cmzn_field_ensemble_id ensemble_field);

unsigned int cmzn_field_ensemble_get_size(cmzn_field_ensemble_id ensemble_field);



int cmzn_ensemble_iterator_destroy(cmzn_ensemble_iterator_id *entry_address);

cmzn_ensemble_identifier cmzn_ensemble_iterator_get_identifier(
	cmzn_ensemble_iterator_id iterator);

/***************************************************************************//**
 * Iterate to next entry in same ensemble.
 * @param iterator  Iterator to be incremented. Invalidated if past last entry.
 * @return  1 on success, 0 if no more entries or iterator invalid.
 */
int cmzn_ensemble_iterator_increment(cmzn_ensemble_iterator_id iterator);



cmzn_field_id cmzn_fieldmodule_create_field_ensemble_group(cmzn_fieldmodule_id field_module,
	cmzn_field_ensemble_id ensemble_field);

cmzn_field_ensemble_group_id cmzn_field_cast_ensemble_group(cmzn_field_id field);

/***************************************************************************//**
 * Remove all entries from group.
 */
int cmzn_field_ensemble_group_clear(
	cmzn_field_ensemble_group_id ensemble_group_field);

int cmzn_field_ensemble_group_has_entry(
	cmzn_field_ensemble_group_id ensemble_group_field, cmzn_ensemble_iterator_id iterator);

int cmzn_field_ensemble_group_add_entry(
	cmzn_field_ensemble_group_id ensemble_group_field, cmzn_ensemble_iterator_id iterator);

int cmzn_field_ensemble_group_remove_entry(
		cmzn_field_ensemble_group_id ensemble_group_field, cmzn_ensemble_iterator_id iterator);

cmzn_ensemble_iterator_id cmzn_field_ensemble_group_get_first_entry(
	cmzn_field_ensemble_group_id ensemble_group_field);

/***************************************************************************//**
 * Iterate to next entry in this group.
 * @param ensemble_group_field  Ensemble group to iterate over.
 * @param iterator  Iterator to be incremented. Invalidated if past last entry.
 * @return  1 on success, 0 if no more entries or iterator invalid, including if
 * not from the same ensemble as this group.
 */
int cmzn_field_ensemble_group_increment_entry(cmzn_field_ensemble_group_id ensemble_group_field,
	cmzn_ensemble_iterator_id iterator);

int cmzn_ensemble_index_destroy(cmzn_ensemble_index_id *index_address);

/***************************************************************************//**
 * Determine if the index indexes its field using exactly the supplied ensembles
 * in the supplied order.
 * @return  1 if true, 0 if indexing ensembles are different.
 */
int cmzn_ensemble_index_has_index_ensembles(cmzn_ensemble_index_id index,
	int number_of_index_ensembles, cmzn_field_ensemble_id *index_ensemble_fields);

/***************************************************************************//**
 * Set index to span all entries in this ensemble, in order of increasing
 * identifier.
 * @return  1 on success, 0 if ensemble is not part of this index.
 */
int cmzn_ensemble_index_set_all_ensemble(cmzn_ensemble_index_id index,
	cmzn_field_ensemble_id ensemble_field);

/***************************************************************************//**
 * Set index to span a single entry for the iterator's ensemble.
 * @return  1 on success, 0 if ensemble is not part of this index.
 */
int cmzn_ensemble_index_set_entry(cmzn_ensemble_index_id index,
	cmzn_ensemble_iterator_id iterator);

/***************************************************************************//**
 * Set index to span all entries in this group for its ensemble, in order of
 * increasting identifier.
 * @return  1 on success, 0 if ensemble is not part of this index.
 */
int cmzn_ensemble_index_set_group(cmzn_ensemble_index_id index,
	cmzn_field_ensemble_group_id ensemble_group_field);


#endif /* !defined (CMZN_FIELD_ENSEMBLE_H) */
