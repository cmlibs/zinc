/*******************************************************************************
FILE : api/cmiss_value_matrix.c

LAST MODIFIED : 12 August 2003

DESCRIPTION :
The public interface to the Cmiss_value_matrix object.
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
#include "api/cmiss_value_derivative_matrix.h"
#include "computed_variable/computed_value_derivative_matrix.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

Cmiss_value_id CREATE(Cmiss_value_derivative_matrix)(Cmiss_variable_id dependent_variable,
	int order, Cmiss_variable_id *independent_variables,
	int number_of_matrices, Cmiss_value_id *matrices)
/*******************************************************************************
LAST MODIFIED : 12 August 2003

DESCRIPTION :
Creates a Cmiss_value which contains a set of derivative matrices.
==============================================================================*/
{
	Cmiss_value_id return_value;
	int expected_number_of_matrices, i;

	ENTER(CREATE(Cmiss_value_derivative_matrix));
	expected_number_of_matrices=0;
	for (i = 0 ; i < order ; i++)
	{
		expected_number_of_matrices=2*expected_number_of_matrices+1;
	}
	if (dependent_variable && (order > 0) && independent_variables
		 && (number_of_matrices == expected_number_of_matrices) && matrices)
	{
		if (return_value=CREATE(Cmiss_value)())
		{
			ACCESS(Cmiss_value)(return_value);
			if (!Cmiss_value_derivative_matrix_set_type(return_value,
				dependent_variable,order,independent_variables,matrices))
			{
				DEACCESS(Cmiss_value)(&return_value);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(Cmiss_value_derivative_matrix).  "
				"Unable to Create(Cmiss_value).");
			return_value = (Cmiss_value_id)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Cmiss_value_derivative_matrix).  "
			"Invalid arguments.");
		return_value = (Cmiss_value_id)NULL;
	}
	LEAVE;

	return (return_value);
} /* CREATE(Cmiss_value_derivative_matrix) */
