/*******************************************************************************
FILE : computed_field_conditional.h

LAST MODIFIED : 27 July 2007

DESCRIPTION :
Implements computed fields which conditionally calculate their inputs.
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
 * Portions created by the Initial Developer are Copyright (C) 2005
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
#if !defined (COMPUTED_FIELD_CONDITIONAL_H)
#define COMPUTED_FIELD_CONDITIONAL_H

#include "general/value.h"
#include "api/cmiss_field.h"
#include "api/cmiss_field_conditional.h"

#define Computed_field_create_if Cmiss_field_module_create_if

/*****************************************************************************//**
 * Creates a conditional field with the same number of components as each of the
 * source_fields. For each component, if the value of source_field_one is TRUE
 * (non-zero) then the result will be the value of source_field_two, otherwise the
 * component result will be taken from source_field_three.
 * 
 * @param field_module  Region field module which will own new field.
 * @param source_field_one  Conditional field.
 * @param source_field_two  TRUE = non-zero conditional component results.
 * @param source_field_three  FALSE = zero conditional component results.
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_if(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two,
	struct Computed_field *source_field_three);

#endif /* !defined (COMPUTED_FIELD_CONDITIONAL_H) */
