/*******************************************************************************
FILE : cmiss_element.h

LAST MODIFIED : 15 January 2007

DESCRIPTION :
The public interface to Cmiss_element.
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
#ifndef __CMISS_ELEMENT_H__
#define __CMISS_ELEMENT_H__

#include "api/cmiss_node.h"

/*
Global types
------------
*/

struct Cmiss_region;
/*******************************************************************************
LAST MODIFIED : 14 August 2002

DESCRIPTION :
==============================================================================*/

#ifndef CMISS_REGION_ID_DEFINED
   typedef struct Cmiss_region * Cmiss_region_id;
   #define CMISS_REGION_ID_DEFINED
#endif /* CMISS_REGION_ID_DEFINED */

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_element FE_element

struct Cmiss_element;
typedef struct Cmiss_element *Cmiss_element_id;
/*******************************************************************************
LAST MODIFIED : 14 August 2002

DESCRIPTION :
==============================================================================*/

typedef int (*Cmiss_element_iterator_function)(Cmiss_element_id element,
  void *user_data);
/*******************************************************************************
LAST MODIFIED : 03 March 2005

DESCRIPTION :
Declare a pointer to a function of type
int function(struct Cmiss_element *element, void *user_data);
==============================================================================*/

/*
Global functions
----------------
*/

Cmiss_element_id create_Cmiss_element_with_line_shape(int element_identifier,
	Cmiss_region_id region, int dimension);
/*******************************************************************************
LAST MODIFIED : 1 December 2004

DESCRIPTION :
Creates an element that has a line shape product of the specified <dimension>.
==============================================================================*/

int Cmiss_element_set_node(Cmiss_element_id element, int node_index,
	Cmiss_node_id node);
/*******************************************************************************
LAST MODIFIED : 11 November 2004

DESCRIPTION :
Sets node <node_number>, from 0 to number_of_nodes-1 of <element> to <node>.
<element> must already have a shape and node_scale_field_information.
Should only be called for unmanaged elements.
==============================================================================*/

int Cmiss_element_get_identifier(Cmiss_element_id element);
/*******************************************************************************
LAST MODIFIED : 11 November 2004

DESCRIPTION :
Returns the integer identifier of the <element>.
==============================================================================*/

int destroy_Cmiss_element(Cmiss_element_id *element_id_address);
/*******************************************************************************
LAST MODIFIED : 17 January 2007

DESCRIPTION :
Deaccesses the element
==============================================================================*/

#endif /* __CMISS_ELEMENT_H__ */
