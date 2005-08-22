/*******************************************************************************
FILE : computed_field_external.h

LAST MODIFIED : 23 January 2002

DESCRIPTION :
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
#if !defined (COMPUTED_FIELD_EXTERNAL_H)
#define COMPUTED_FIELD_EXTERNAL_H

int Computed_field_register_types_external(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
==============================================================================*/

int Computed_field_set_type_external(struct Computed_field *field,
	char *filename, int timeout,
	int number_of_source_values, FE_value *source_values,
	int number_of_source_fields, struct Computed_field **source_fields);
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_EXTERNAL.
<source_fields> must point to an array of <number_of_sources> pointers to
Computed_fields. The resulting field will have as many
components as <number_of_sources> * components + number_of_source_values.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/

int Computed_field_get_type_external(struct Computed_field *field,
	char **filename, int *timeout,
	int *number_of_source_values, FE_value **source_values,
	int *number_of_source_fields,struct Computed_field ***source_fields);
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
If the field is of type COMPUTED_FIELD_EXTERNAL, the function allocates and
returns in <**source_fields> an array containing the <number_of_sources> source
fields making up the composite field - otherwise an error is reported.
It is up to the calling function to DEALLOCATE the returned array. Note that the
fields in the returned array are not ACCESSed.
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_EXTERNAL_H) */
