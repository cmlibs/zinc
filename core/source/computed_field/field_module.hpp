/***************************************************************************//**
 * FILE : field_module.hpp
 *
 * Internal header of field module api.
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
 * Portions created by the Initial Developer are Copyright (C) 2011
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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
#if !defined (FIELD_MODULE_H)
#define FIELD_MODULE_H

#include "zinc/fieldmodule.h"

/***************************************************************************//**
 * Creates field module object needed to create fields in supplied region.
 * Internally: Also used to set new field default arguments prior to create.
 *
 * @param region  The owning region.
 * @return  Field module for the supplied region.
 */
struct cmzn_field_module *cmzn_field_module_create(struct cmzn_region *region);

/***************************************************************************//**
 * Candidate for external API.
 * @return  1 if field is from this field_module, otherwise 0.
 */
int cmzn_field_module_contains_field(cmzn_field_module_id field_module,
	cmzn_field_id field);

/***************************************************************************//**
 * Internal, non-accessing version of cmzn_field_module_get_region.
 *
 * @param field_module  The field module to query.
 * @return  Non-accessed handle to owning region for field_module.
 */
struct cmzn_region *cmzn_field_module_get_region_internal(
	struct cmzn_field_module *field_module);

/***************************************************************************//**
 * Get non-accessed pointer to master region for this field_module.
 *
 * @param field_module  The field module to query.
 * @return  Non-accessed handle to master region for field_module.
 */
struct cmzn_region *cmzn_field_module_get_master_region_internal(
	struct cmzn_field_module *field_module);

/***************************************************************************//**
 * Sets the name (or name stem if non-unique) of the next field to be created
 * with this field_module.
 *
 * @param field_module  The field module to create fields in.
 * @param field_name  Field name or name stem.
 * @return  Non-zero on success, 0 on failure.
 */
int cmzn_field_module_set_field_name(struct cmzn_field_module *field_module,
	const char *field_name);

/***************************************************************************//**
 * Gets a copy of the field name/stem set in this field_module.
 * Up to caller to DEALLOCATE.
 *
 * @param field_module  The field module to create fields in.
 * @return  Allocated copy of the name, or NULL if none.
 */
char *cmzn_field_module_get_field_name(
	struct cmzn_field_module *field_module);

/***************************************************************************//**
 * Sets the coordinate system to be used for subsequent fields created with
 * this field module.
 *
 * @param field_module  The field module to create fields in.
 * @param coordinate_system  The coordinate system to set.
 * @return  1 on success, 0 on failure.
 */
int cmzn_field_module_set_coordinate_system(
	struct cmzn_field_module *field_module,
	struct Coordinate_system coordinate_system);

/***************************************************************************//**
 * Returns the default coordinate system set in the field_module.
 *
 * @param field_module  The field module to create fields in.
 * @return  Copy of default coordinate system.
 */
struct Coordinate_system cmzn_field_module_get_coordinate_system(
	struct cmzn_field_module *field_module);

/***************************************************************************//**
 * Returns true if the default coordinate system has been explicitly set. 
 *
 * @param field_module  The field module to create fields in.
 * @return  1 if coordinate system set, 0 if never set.
 */
int cmzn_field_module_coordinate_system_is_set(
	struct cmzn_field_module *field_module);

/***************************************************************************//**
 * Sets the replace_field that will be redefined by the next field
 * created with the field module. Cleared after next field create call.
 * Field name and coordinate system defaults are taken from supplied field.
 *
 * @param field_module  The field module to create fields in.
 * @param replace_field  Existing field to be overwritten on next create. Can
 * be NULL to clear.
 * @return  1 on success, 0 on failure.
 */
int cmzn_field_module_set_replace_field(
	struct cmzn_field_module *field_module,
	struct Computed_field *replace_field);

/***************************************************************************//**
 * Gets the replace_field, if any, that will be redefined by the next field
 * created with the field module. Cleared after next field create call.
 *
 * @param field_module  The field module to create fields in.
 * @return  Existing field to be replaced which caller must deaccess, or NULL
 * if none.
 */
struct Computed_field *cmzn_field_module_get_replace_field(
	struct cmzn_field_module *field_module);

/**
 * Internal use only.
 * @return  ACCESSed xi field with name "xi", existing or new.
 */
cmzn_field_id cmzn_field_module_get_or_create_xi_field(
	cmzn_field_module_id field_module);

#endif /* !defined (FIELD_MODULE_H) */
