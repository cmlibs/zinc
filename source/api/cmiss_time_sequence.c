/***************************************************************************//**
 * FILE : cmiss_time_sequence.c
 *
 * Implementation of the public interface to Cmiss_time_sequence.
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

/*#include <stdarg.h>*/
#include "api/cmiss_time_sequence.h"
/*#include "general/debug.h"*/
/*#include "general/object.h"*/
#include "finite_element/finite_element.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

Cmiss_time_sequence_id Cmiss_time_sequence_access(
	Cmiss_time_sequence_id time_sequence)
{
	return (Cmiss_time_sequence_id)(ACCESS(FE_time_sequence)(
		(struct FE_time_sequence *)time_sequence));
}

int Cmiss_time_sequence_destroy(Cmiss_time_sequence_id *time_sequence_address)
{
	return DESTROY(FE_time_sequence)(
		(struct FE_time_sequence **)time_sequence_address);
}

int Cmiss_time_sequence_set_value(Cmiss_time_sequence_id time_sequence,
	int time_index, double time)
{
	struct FE_time_sequence *fe_time_sequence;

	fe_time_sequence = (struct FE_time_sequence *)time_sequence;
	if (FE_time_sequence_is_in_use(fe_time_sequence))
	{
		display_message(ERROR_MESSAGE, "Cmiss_time_sequence_set_value.  "
			"Cannot modify time sequence while in use");
		return 0;
	}
	return FE_time_sequence_set_time_and_index(fe_time_sequence, time_index, time);
}
