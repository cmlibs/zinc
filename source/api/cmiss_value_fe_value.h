/*******************************************************************************
FILE : api/cmiss_value_fe_value.h

LAST MODIFIED : 10 September 2003

DESCRIPTION :
The public interface to the Cmiss_value_fe_value object.
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
#ifndef __API_CMISS_VALUE_FE_VALUE_H__
#define __API_CMISS_VALUE_FE_VALUE_H__

#include "api/cmiss_value.h"
/* SAB These will need to have an equivalent in the api. */
#include "general/object.h"
#include "general/value.h"

Cmiss_value_id CREATE(Cmiss_value_FE_value)(FE_value value);
/*******************************************************************************
LAST MODIFIED : 20 August 2003

DESCRIPTION :
Creates a Cmiss_value which contains an fe_value.
==============================================================================*/

Cmiss_value_id CREATE(Cmiss_value_FE_value_vector)(int number_of_values,
	FE_value *values);
/*******************************************************************************
LAST MODIFIED : 20 August 2003

DESCRIPTION :
Creates a Cmiss_value which contains a vector of fe_values.
==============================================================================*/

Cmiss_value_id CREATE(Cmiss_value_FE_value_matrix)(int number_of_rows,
	int number_of_columns, FE_value *values);
/*******************************************************************************
LAST MODIFIED : 10 September 2003

DESCRIPTION :
Creates a Cmiss_value which contains a vector of fe_values.
==============================================================================*/

#endif /* __API_CMISS_VALUE_FE_VALUE_H__ */
