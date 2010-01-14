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

/* Workaround to revision C6810. It would be cleaner to just #include "api/cmiss_field.h" */
#ifndef CMISS_FIELD_ID_DEFINED
	/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
	#define Cmiss_field Computed_field
	struct Cmiss_field;
	typedef struct Cmiss_field *Cmiss_field_id;
	#define CMISS_FIELD_ID_DEFINED
#endif /* CMISS_FIELD_ID_DEFINED */

/***************************************************************************//**
 * Field module, obtained from region, which owns fields and must be passed to
 * field factory create methods.
 */
struct Cmiss_field_module;

typedef struct Cmiss_field_module *Cmiss_field_module_id;

/*******************************************************************************
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
 * Returns the field of the specified name from the field module.
 *
 * @param field_module  Region field module in which to find the field.
 * @param field_name  The name of the field to find.
 * @return  New reference to field of specified name, or NULL if not found.
 */
Cmiss_field_id Cmiss_field_module_find_field_by_name(
	Cmiss_field_module_id field_module, const char *field_name);

#endif /* __CMISS_FIELD_MODULE_H__ */
