/*****************************************************************************//**
 * FILE : cmiss_field_module.h
 *
 * Public interface to the field module including its generic functions.
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
#ifndef __CMISS_FIELD_MODULE_H__
#define __CMISS_FIELD_MODULE_H__

#include "types/cmiss_region_id.h"
#include "types/cmiss_field_id.h"
#include "types/cmiss_field_module_id.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Automatic scalar broadcast
 *
 * For field constructors (Cmiss_field_module_create~ functions) which specify
 * the they apply automatic scalar broadcast for their source fields arguments,
 * if the one of the source fields has multiple components and the
 * other is a scalar, then the scalar will be automatically replicated so that
 * it matches the number of components in the multiple component field.
 * For example the result of
 * ADD(CONSTANT([1 2 3 4], CONSTANT([10]) is [11 12 13 14].
 */

/*
Global functions
----------------
*/

/***************************************************************************//**
 * Returns a new reference to the field module with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param field_module  The field module to obtain a new reference to.
 * @return  New field module reference with incremented reference count.
 */
Cmiss_field_module_id Cmiss_field_module_access(Cmiss_field_module_id field_module);

/***************************************************************************//**
 * Destroys reference to the field module and sets pointer/handle to NULL.
 * Internally this just decrements the reference count.
 *
 * @param field_module_address  Address of field module reference.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_field_module_destroy(Cmiss_field_module_id *field_module_address);

/***************************************************************************//**
 * Begin caching or increment cache level for this field module. Call this
 * function before making multiple changes to fields, nodes, elements etc. from
 * this field module to minimise number of change messages sent to clients.
 * Must call Cmiss_field_module_end_change after making changes.
 * Note that field module changes are always cached when the region changes are
 * being cached.
 *
 * @see Cmiss_region_begin_change
 * @param field_module  The field_module to begin change cache on.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_field_module_begin_change(Cmiss_field_module_id field_module);

/***************************************************************************//**
 * Decrement cache level or end caching of changes for this field module.
 * Call Cmiss_field_module_begin_change before making multiple changes
 * and call this afterwards. When change level is restored to zero,
 * cached change messages are sent out to clients.
 *
 * @param field_module  The field_module to end change cache on.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_field_module_end_change(Cmiss_field_module_id field_module);

/***************************************************************************//**
 * Returns the field of the specified name from the field module.
 *
 * @param field_module  Region field module in which to find the field.
 * @param field_name  The name of the field to find.
 * @return  New reference to field of specified name, or NULL if not found.
 */
Cmiss_field_id Cmiss_field_module_find_field_by_name(
	Cmiss_field_module_id field_module, const char *field_name);

/***************************************************************************//**
 * Create a field iterator object for iterating through the fields in the field
 * module, in alphabetical order of name. The iterator initially points at the
 * position before the first field, so the first call to
 * Cmiss_field_iterator_next() returns the first field and advances the
 * iterator.
 * Iterator becomes invalid if fields are added, removed or renamed while in use.
 *
 * @param field_module  Handle to the field_module whose fields are to be
 * iterated over.
 * @return  Handle to field_iterator at position before first, or NULL if
 * error.
 */
Cmiss_field_iterator_id Cmiss_field_module_create_field_iterator(
	Cmiss_field_module_id field_module);

/***************************************************************************//**
 * Defines, for all elements of all meshes in field module, face elements of
 * dimension one lower in the associated face mesh, and all their faces
 * recursively down to 1 dimensional lines.
 *
 * @param field_module  Handle to the field_module owning the meshes to define
 * faces for.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_field_module_define_all_faces(Cmiss_field_module_id field_module);

/***************************************************************************//**
 * Gets the region this field module can create fields for.
 *
 * @param field_module  The field module to query.
 * @return  Accessed handle to owning region for field_module.
 */
Cmiss_region_id Cmiss_field_module_get_region(Cmiss_field_module_id field_module);

#ifdef __cplusplus
}
#endif

#endif /* __CMISS_FIELD_MODULE_H__ */
