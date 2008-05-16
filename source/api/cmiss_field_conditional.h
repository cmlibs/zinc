/*******************************************************************************
FILE : cmiss_field_conditional.h

LAST MODIFIED : 16 May 2008

DESCRIPTION :
Implements cmiss fields which conditionally calculate their inputs.
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
#if !defined (CMISS_FIELD_CONDITIONAL_H)
#define CMISS_FIELD_CONDITIONAL_H

#include "api/cmiss_field.h"

Cmiss_field_id Cmiss_field_create_if(
	Cmiss_field_id source_field_one,
	Cmiss_field_id source_field_two,
	Cmiss_field_id source_field_three);
/*******************************************************************************
LAST MODIFIED : 16 May 2008

DESCRIPTION :
Converts <field> to type CMISS_FIELD_IF with the supplied
fields, <source_field_one>, <source_field_two> and <source_field_three>.
Sets the number of components equal to the source_fields.
For each component, if the value of source_field_one is TRUE (non-zero) then
the result will be the value of source_field_two, otherwise the result will
be source_field_three.
==============================================================================*/

#endif /* !defined (CMISS_FIELD_CONDITIONAL_H) */
