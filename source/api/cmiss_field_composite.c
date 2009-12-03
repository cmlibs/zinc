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
 * Portions created by the Initial Developer are Copyright (C) 2009
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

#include "api/cmiss_field.h"
#include "api/cmiss_field_composite.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_composite.h"
#include "general/debug.h"
#include "user_interface/message.h"

Cmiss_field_id Cmiss_field_create_concatenate(Cmiss_field_factory_id field_factory,
	int number_of_source_fields, Cmiss_field_id *source_fields)
{
	int *temp_source_field_numbers, *temp_source_value_numbers,i,j,k,
		number_of_components_per_field,number_of_components;
	Cmiss_field_id field = NULL;
	
	ENTER(Cmiss_field_create_composite);
	
	if (source_fields && number_of_source_fields > 0)
	{
		number_of_components = 0;
		for (i=0; i < number_of_source_fields; i++)
		{
			number_of_components += 
				Computed_field_get_number_of_components(source_fields[i]);
		}
		if (ALLOCATE(temp_source_field_numbers, int, number_of_components)
			&& ALLOCATE(temp_source_value_numbers, int, number_of_components))
		{
			k = 0;
			for (i=0; i < number_of_source_fields; i++)
			{
				number_of_components_per_field = 
					Computed_field_get_number_of_components(source_fields[i]);
				for (j=0;j < number_of_components_per_field;j++)
				{
					temp_source_field_numbers[k+j] = i;
					temp_source_value_numbers[k+j] = j;
				}
				k += number_of_components_per_field;
			}
			field = Computed_field_create_composite(field_factory,
				number_of_components, number_of_source_fields, source_fields,
				0, NULL, temp_source_field_numbers,temp_source_value_numbers);
			DEALLOCATE(temp_source_field_numbers);
			DEALLOCATE(temp_source_value_numbers);	
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_field_create_concatenate.  Out of memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_create_concatenate.  Invalid argument(s)");
	}

	LEAVE;

	return(field);
}
