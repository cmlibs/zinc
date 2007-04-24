/*******************************************************************************
FILE : cmiss_element.c

LAST MODIFIED : 10 November 2004

DESCRIPTION :
The public interface to the Cmiss_elements.
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
#include <stdarg.h>
#include "api/cmiss_element.h"
#include "general/debug.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

int Cmiss_element_get_identifier(struct Cmiss_element *element)
/*******************************************************************************
LAST MODIFIED : 1 April 2004

DESCRIPTION :
Returns the integer identifier of the <element>.
==============================================================================*/
{
	int return_code;
	struct CM_element_information cm_id;

	ENTER(Cmiss_element_get_identifier);
	if (return_code = get_FE_element_identifier(element, &cm_id))
	{
		/* Check it is truly a CM_ELEMENT? */
		return_code = cm_id.number;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_element_get_identifier */

int Cmiss_element_get_dimension(Cmiss_element_id element)
/*******************************************************************************
LAST MODIFIED : 17 April 2007

DESCRIPTION :
Returns the dimension of the <element> or an error if it does not have a shape.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_element_get_dimension);
	return_code = get_FE_element_dimension(element);
	LEAVE;

	return (return_code);
} /* Cmiss_element_get_dimension */

int Cmiss_element_set_node(Cmiss_element_id element, int node_index,
	Cmiss_node_id node)
/*******************************************************************************
LAST MODIFIED : 11 November 2004

DESCRIPTION :
Sets node <node_number>, from 0 to number_of_nodes-1 of <element> to <node>.
<element> must already have a shape and node_scale_field_information.
Should only be called for unmanaged elements.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_element_set_node);
	return_code = set_FE_element_node(element, node_index, node);
	LEAVE;

	return (return_code);
} /* Cmiss_element_set_node */

Cmiss_element_id create_Cmiss_element_with_line_shape(int element_identifier,
	Cmiss_region_id region, int dimension)
/*******************************************************************************
LAST MODIFIED : 1 December 2004

DESCRIPTION :
Creates an element that has a line shape product of the specified <dimension>.
==============================================================================*/
{
	Cmiss_element_id element;
	struct FE_region *fe_region;

	ENTER(create_Cmiss_element);
	element = (Cmiss_element_id)NULL;
	if (fe_region=Cmiss_region_get_FE_region(region))
	{
		element = ACCESS(FE_element)(create_FE_element_with_line_shape(
			element_identifier, fe_region, dimension));
	}
	LEAVE;

	return (element);
} /* create_Cmiss_element */

int destroy_Cmiss_element(Cmiss_element_id *element_id_address)
/*******************************************************************************
LAST MODIFIED : 10 November 2004

DESCRIPTION :
Frees the memory for the element, sets <*element_address> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(destroy_Cmiss_element);
	return_code = DEACCESS(FE_element)(element_id_address);
	LEAVE;

	return (return_code);
} /* destroy_Cmiss_element */

