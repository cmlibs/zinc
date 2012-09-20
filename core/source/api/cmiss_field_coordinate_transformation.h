/*******************************************************************************
 * FILE : cmiss_field_coordinate_transformation.h
 *
 * Field coordinate transformation operators.
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
 * Portions created by the Initial Developer are Copyright (C) 2012
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
#if !defined (CMISS_FIELD_COORDINATE_TRANSFORMATION_H)
#define CMISS_FIELD_COORDINATE_TRANSFORMATION_H

#include "types/cmiss_field_id.h"
#include "types/cmiss_field_module_id.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Creates a field which performs a coordinate transformation from the source
 * field values in their coordinate system type into the coordinate system type
 * of this field. Returned field has 3 components.
 * @see Cmiss_field_set_coordinate_system_type
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  Source field with values in its own coordinate system.
 * Must have 1 to 3 components.
 * @return Newly created field
 */
Cmiss_field_id Cmiss_field_module_create_coordinate_transformation(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field);

/***************************************************************************//**
 * Create a field which performs a coordinate transformation of vectors from
 * their original coordinate system and coordinate positions, to the coordinate
 * system of this field. Sets the number of components in returned field to 3
 * times the number of vectors expected from the source vector_field.
 *
 * @param field_module  Region field module which will own new field.
 * @param vector_field  Vector field to be transformed. Can be a single vector
 * (1,2 or 3 components), two vectors (4 or 6 components) or three vectors
 * (9 components).
 * @param coordinate_field  Field giving location where vector value is from.
 * @return Newly created field
 */
Cmiss_field_id Cmiss_field_module_create_vector_coordinate_transformation(
	Cmiss_field_module_id field_module, Cmiss_field_id vector_field,
	Cmiss_field_id coordinate_field);

#ifdef __cplusplus
}
#endif

#endif /* !defined (CMISS_FIELD_COORDINATE_TRANSFORMATION_H) */
