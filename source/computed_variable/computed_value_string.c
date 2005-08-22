/*******************************************************************************
FILE : computed_value_string.c

LAST MODIFIED : 25 July 2003

DESCRIPTION :
The beginnings of a Cmiss_value_string type, NOT IMPLEMENTED
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
#include "computed_variable/computed_value.h"
#include "computed_variable/computed_value_private.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Module constants
----------------
*/
static char Cmiss_value_string_type_string[]="String";

int Cmiss_value_string_set_type(Cmiss_value_id value,char *string);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Makes <value> of type string and sets its <string>.  After success, the <value>
is responsible for DEALLOCATEing <string>.
==============================================================================*/

DECLARE_CMISS_VALUE_IS_TYPE_FUNCTION(string)

int Cmiss_value_string_get_type(Cmiss_value_id value,char **string_address);
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
If <value> is of type string, gets its <*string_address>.

The calling program must not DEALLOCATE the returned <*string_address>.
==============================================================================*/
