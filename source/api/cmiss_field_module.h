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
Global types
------------
*/

#ifndef CMISS_FIELD_ID_DEFINED
	struct Cmiss_field;
	typedef struct Cmiss_field *Cmiss_field_id;
	#define CMISS_FIELD_ID_DEFINED
#endif /* CMISS_FIELD_ID_DEFINED */

/***************************************************************************//**
 * Field module, obtained from region, which owns fields and must be passed to
 * field factory create methods.
 */
#ifndef CMISS_FIELD_MODULE_ID_DEFINED
	struct Cmiss_field_module;
	typedef struct Cmiss_field_module *Cmiss_field_module_id;
	#define CMISS_FIELD_MODULE_ID_DEFINED
#endif /* CMISS_FIELD_MODULE_ID_DEFINED */

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
 * @return  1 on success, 0 if invalid arguments.
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
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_module_begin_change(Cmiss_field_module_id field_module);

/***************************************************************************//**
 * Decrement cache level or end caching of changes for this field module.
 * Call Cmiss_field_module_begin_change before making multiple changes
 * and call this afterwards. When change level is restored to zero,
 * cached change messages are sent out to clients.
 *
 * @param field_module  The field_module to end change cache on.
 * @return  1 on success, 0 on failure.
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
 * Define a field as per the 'gfx define field' command in the standalone cmgui
 * application. However this define field only enables "region-safe" fields.
 *
 * NOTE: This function may be removed in the future once more API functions are
 * made available to the users.
 *
 * @param field_module Handle to the field module to use.
 * @param command  Command to be executed.
 * @return  1 if command completed successfully, otherwise 0.
 */
int Cmiss_field_module_define_field(Cmiss_field_module_id field_module,
	const char *command_string);

/***************************************************************************//**
 * Create a named field as per the 'gfx define field' command in the standalone
 * cmgui application. Unlike Cmiss_field_module_define_field, this function only
 * succeeds if an existing field with the given name does not already exist. The
 * returned field handle needs to be destroyed by the calling application.
 *
 * NOTE: This function may be removed in the future once more API functions are
 * made available to the users.
 *
 * @param field_module Handle to the field module to use.
 * @param field_name The name of the field to create.
 * @param command  Command to be executed (excluding the field name).
 * @return  The newly created field if successful, NULL otherwise.
 */
Cmiss_field_id Cmiss_field_module_create_field(Cmiss_field_module_id field_module,
		const char* field_name, const char *command_string);

#endif /* __CMISS_FIELD_MODULE_H__ */
