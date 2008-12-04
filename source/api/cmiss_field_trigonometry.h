/*******************************************************************************
FILE : cmiss_field_trigonometry.h

DESCRIPTION :
The public interface to the Cmiss_fields that perform trigonometry.
==============================================================================*/
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
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * Shane Blackett (shane at blackett.co.nz)
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
#ifndef __CMISS_FIELD_TRIGONOMETRY_H__
#define __CMISS_FIELD_TRIGONOMETRY_H__

/*****************************************************************************//**
 * Creates a field where the components are the sine value (using radians) of the
 * components of the source_field.
 * 
 * @param source_field Input field
 * @return Newly created field
 */
Cmiss_field_id Cmiss_field_create_sin(
	Cmiss_field_id source_field);

/*****************************************************************************//**
 * Creates a field where the components are the cosine value (using radians) of the
 * components of the source_field.
 * 
 * @param source_field Input field (components in radians)
 * @return Newly created field
 */
Cmiss_field_id Cmiss_field_create_cos(
	Cmiss_field_id source_field);

/*****************************************************************************//**
 * Creates a field where the components are the trigonometric tangent value 
 * (using radians) of the components of the source_field.
 * 
 * @param source_field Input field (components in radians)
 * @return Newly created field
 */
Cmiss_field_id Cmiss_field_create_tan(
	Cmiss_field_id source_field);

/*****************************************************************************//**
 * Creates a field where the components are the arcsine value (using radians) of the
 * components of the source_field.
 * 
 * @param source_field Input field (components in radians)
 * @return Newly created field
 */
Cmiss_field_id Cmiss_field_create_asin(
	Cmiss_field_id source_field);

/*****************************************************************************//**
 * Creates a field where the components are the arccosine value (using radians) of the
 * components of the source_field.
 * 
 * @param source_field Input field
 * @return Newly created field (components in radians)
 */
Cmiss_field_id Cmiss_field_create_acos(
	Cmiss_field_id source_field);

/*****************************************************************************//**
 * Creates a field where the components are the arctangent value (using radians) of the
 * components of the source_field.
 * 
 * @param source_field Input field
 * @return Newly created field (components in radians)
 */
Cmiss_field_id Cmiss_field_create_atan(
	Cmiss_field_id source_field);

/*****************************************************************************//**
 * Creates a field where the components are calculated using the atan2 c function,
 * so that the angle returned (in radians) is the angle between a positive x axis in
 * a plane and the vector (x,y) where x is the source_field_one component and y is
 * the source_field_two component.
 * 
 * @param source_field_one First input field
 * @param source_field_two Second input field
 * @return Newly created field (components in radians)
 */
Cmiss_field_id Cmiss_field_create_atan2(
	Cmiss_field_id source_field_one,
	Cmiss_field_id source_field_two);


#endif /* __CMISS_FIELD_TRIGONOMETRY_H__ */
