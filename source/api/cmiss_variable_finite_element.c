/*******************************************************************************
FILE : api/cmiss_variable_finite_element.c

LAST MODIFIED : 4 November 2004

DESCRIPTION :
The public interface to the Cmiss_variable_finite_element object.
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
#include "api/cmiss_variable_finite_element.h"
#include "computed_variable/computed_variable_finite_element.h"
#include "finite_element/finite_element_region.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

Cmiss_variable_id CREATE(Cmiss_variable_finite_element)(Cmiss_region_id region,
	char *field_name,char *component_name)
/*******************************************************************************
LAST MODIFIED : 4 November 2004

DESCRIPTION :
Creates a Cmiss_variable which represents the <field_name> in <region>.  If
<component_name> is not NULL then that is used to select a particular component.
==============================================================================*/
{
	Cmiss_variable_id return_variable;
	struct FE_field *fe_field;
	struct FE_region *fe_region;

	ENTER(CREATE(Cmiss_variable_finite_element));
	return_variable=(Cmiss_variable_id)NULL;
	if (region&&field_name)
	{
		if ((fe_region=Cmiss_region_get_FE_region(region))&&
			(fe_field=FE_region_get_FE_field_from_name(fe_region,field_name)))
		{
			char *name;
			int component_number;

			name=(char *)NULL;
			if (GET_NAME(FE_field)(fe_field,&name)&&
				(return_variable=CREATE(Cmiss_variable)(
				(struct Cmiss_variable_package *)NULL,name)))
			{
				ACCESS(Cmiss_variable)(return_variable);
				if (name)
				{
					DEALLOCATE(name);
				}
				if (component_name)
				{
					component_number=get_FE_field_number_of_components(fe_field);
					if (0<component_number)
					{
						name=(char *)NULL;
						do
						{
							component_number--;
							if (name)
							{
								DEALLOCATE(name);
							}
							name=get_FE_field_component_name(fe_field,component_number);
						} while ((component_number>0)&&strcmp(name,component_name));
						if (!name||strcmp(name,component_name))
						{
							DEACCESS(Cmiss_variable)(&return_variable);
						}
						if (name)
						{
							DEALLOCATE(name);
						}
					}
				}
				else
				{
					component_number= -1;
				}
				if (return_variable)
				{
					if (!Cmiss_variable_finite_element_set_type(return_variable,fe_field,
							 component_number))
					{
						DEACCESS(Cmiss_variable)(&return_variable);
					}
				}
			}
			else
			{
				if (name)
				{
					DEALLOCATE(name);
				}
			}
		}
	}
	LEAVE;

	return (return_variable);
} /* CREATE(Cmiss_variable_finite_element) */


Cmiss_variable_id CREATE(Cmiss_variable_element_xi)(char *name, int dimension)
/*******************************************************************************
LAST MODIFIED : 13 August 2003

DESCRIPTION :
Creates a Cmiss_variable which represents an element xi location of <dimension>.
==============================================================================*/
{
	Cmiss_variable_id return_variable;

	ENTER(CREATE(Cmiss_variable_element_xi));
	return_variable = (Cmiss_variable_id)NULL;
	if (name)
	{
		if (return_variable=CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL,name))
		{
			ACCESS(Cmiss_variable)(return_variable);
			if (!Cmiss_variable_element_xi_set_type(return_variable, dimension))
			{
				DEACCESS(Cmiss_variable)(&return_variable);
			}
		}
	}
	LEAVE;

	return (return_variable);
} /* CREATE(Cmiss_variable_element_xi) */

Cmiss_variable_id CREATE(Cmiss_variable_nodal_value)(char *name,
	Cmiss_variable_id fe_variable, struct FE_node *node,
	enum FE_nodal_value_type value_type, int version)
/*******************************************************************************
LAST MODIFIED : 14 August 2003

DESCRIPTION :
Creates a Cmiss_variable which represents a nodal degree of freedom or a set of
nodal_degrees of freedom.
==============================================================================*/
{
	Cmiss_variable_id return_variable;

	ENTER(CREATE(Cmiss_variable_nodal_value));
	return_variable = (Cmiss_variable_id)NULL;
	if (name)
	{
		if (return_variable=CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL,name))
		{
			ACCESS(Cmiss_variable)(return_variable);
			if (!Cmiss_variable_nodal_value_set_type(return_variable, fe_variable,
				node, value_type, version))
			{
				DEACCESS(Cmiss_variable)(&return_variable);
			}
		}
	}
	LEAVE;

	return (return_variable);
} /* CREATE(Cmiss_variable_nodal_value) */
