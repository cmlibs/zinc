/*******************************************************************************
FILE : cmiss_field_time.h

LAST MODIFIED : 16 Nov 2011

DESCRIPTION :
Implements cmiss fields that is controlled by time.
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
#ifndef CMZN_FIELDTIME_H__
#define CMZN_FIELDTIME_H__

#include "types/fieldid.h"
#include "types/fieldmoduleid.h"
#include "types/timekeeperid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************************//**
 * Creates a field whose value equals the source_field evaluated at the time
 * given by time_field, overriding any time prescribed for field evaluation.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  Field to evaluate.
 * @param time_field  Field providing time value to evaluate at.
 * @return  Handle to a new time lookup field on success, NULL on failure.
 */
ZINC_API cmzn_field_id cmzn_field_module_create_time_lookup(
	cmzn_field_module_id field_module, cmzn_field_id source_field,
	cmzn_field_id time_field);

/***************************************************************************//**
 * Creates a field which returns the current time from the supplied time keeper.
 *
 * @param field_module  Region field module which will own new field.
 * @param time_keeper  cmzn_time_keeper object.
 * @return  Handle to a new time value field on success, NULL on failure.
 */
ZINC_API cmzn_field_id cmzn_field_module_create_time_value(
	cmzn_field_module_id field_module, cmzn_time_keeper_id time_keeper);

#ifdef __cplusplus
}
#endif

#endif
