/*****************************************************************************//**
 * FILE : cmiss_field_finite_element.h
 * 
 * Implements fields interpolated over finite element meshes with
 * parameters indexed by nodes.
 *
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
#if !defined (CMISS_FIELD_FINITE_ELEMENT_H)
#define CMISS_FIELD_FINITE_ELEMENT_H

#include "api/cmiss_field.h"
#include "api/cmiss_field_module.h"

#ifndef CMISS_FIELD_FINITE_ELEMENT_ID_DEFINED
	/** Handle to a finite_element type Cmiss_field */
	struct Cmiss_field_finite_element;
	typedef struct Cmiss_field_finite_element *Cmiss_field_finite_element_id;
	#define CMISS_FIELD_FINITE_ELEMENT_ID_DEFINED
#endif /* CMISS_FIELD_FINITE_ELEMENT_ID_DEFINED */

/***************************************************************************//**
 * Creates or finds existing real-valued finite_element field which can be
 * interpolated over a finite element mesh with parameters indexed by nodes.
 *
 * @param field_module  Region field module which will own new field.
 * @param name  The name for the field. Must be unique in the field module or
 * identify an existing finite_element field with identical definition.
 * @param number_of_components  The number of components for the new field.
 * @return  Handle to the found or newly created field.
 */
Cmiss_field_id Cmiss_field_module_create_finite_element(
	Cmiss_field_module_id field_module, const char *name,
	int number_of_components);

/*****************************************************************************//**
 * If the field is of type finite_element then this function returns the
 * finite_element specific representation, otherwise returns NULL.
 * Caller is responsible for destroying the returned derived field reference.
 *
 * @param field  The field to be cast.
 * @return  Finite_element specific representation if the input field is of this
 * type, otherwise returns NULL.
 */
Cmiss_field_finite_element_id Cmiss_field_cast_finite_element(Cmiss_field_id field);

/*******************************************************************************
 * Destroys this reference to the finite_element field (and sets it to NULL).
 * Internally this just decrements the reference count.
 */
int Cmiss_field_finite_element_destroy(
	Cmiss_field_finite_element_id *finite_element_field_address);

/***************************************************************************//**
 * Allows the setting of a string if that is the type of field represented.
 * @param component_number  Component number starting at 1.
 * @param time  The time value to store the string at. Currently unused.
 */
int Cmiss_field_finite_element_set_string_at_node(
	Cmiss_field_finite_element_id finite_element_field, int component_number,
	Cmiss_node_id node, double time, const char *string);

/***************************************************************************//**
 * Defines the finite_element field at the specified node.
 * @param fe_time_sequence  Optionally defines multiple times for the field. If
 * it is NULL then the field will be defined as constant for all times.
 * @param node_field_creator  Optionally defines different versions and/or
 * derivative types. If it is NULL then a single nodal value for each component
 * will be defined.
 */
int Cmiss_field_finite_element_define_at_node(
	Cmiss_field_finite_element_id finite_element_field, Cmiss_node_id node,
	struct Cmiss_time_sequence *time_sequence,
	struct Cmiss_node_field_creator *node_field_creator);

#endif /* !defined (CMISS_FIELD_FINITE_ELEMENT_H) */
