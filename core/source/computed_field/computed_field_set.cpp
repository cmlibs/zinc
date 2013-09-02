/*******************************************************************************
FILE : computed_field_set.c

LAST MODIFIED : 7 May 2007

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
#include "zinc/fieldmodule.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"
#include <stdio.h>
#include "computed_field/computed_field_private.hpp"

cmzn_field_id Computed_field_manager_get_field_or_component(
	struct MANAGER(Computed_field) *computed_field_manager, const char *name,
	int *component_number_address)
{
	if ((!computed_field_manager) || (!name) || (!component_number_address))
		return 0;
	cmzn_field_id field =
		FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(name, computed_field_manager);
	if (field)
	{
		*component_number_address = -1;
	}
	else if (0 != strrchr(name, '.'))
	{
		char *name_copy = duplicate_string(name);
		char *field_component_name = strrchr(name_copy,'.');
		*field_component_name = '\0';
		++field_component_name;
		field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(name_copy, computed_field_manager);
		if (field)
		{
			int component_no = -1;
			const int number_of_components = cmzn_field_get_number_of_components(field);
			for (int i = 0; (i < number_of_components) && (component_no < 0); i++)
			{
				char *candidate_component_name = Computed_field_get_component_name(field, i);
				if (candidate_component_name)
				{
					if (0 == strcmp(field_component_name, candidate_component_name))
					{
						component_no = i;
					}
					DEALLOCATE(candidate_component_name);
				}
			}
			if (0 <= component_no)
			{
				if (1 == number_of_components)
				{
					/* already a single component field */
					component_no = -1;
				}
				*component_number_address = component_no;
			}
			else
			{
				field = 0;
			}
		}
		DEALLOCATE(name_copy);
	}
	return field;
}

