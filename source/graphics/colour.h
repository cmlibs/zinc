/*******************************************************************************
FILE : colour.h

LAST MODIFIED : 18 June 1996

DESCRIPTION :
Colour structures and support code.
???DB.  I'm not sure that colour needs to be abstracted/formalized.
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
#if !defined( COLOUR_H )
#define COLOUR_H
#include "command/parser.h"

#define COLOUR_PRECISION float
#define COLOUR_PRECISION_STRING "f"
#define COLOUR_NUM_FORMAT "%6.4" COLOUR_PRECISION_STRING

#define COLOUR_VECTOR(colour_struct_ptr) \
((COLOUR_PRECISION *)colour_struct_ptr)
/*
Global types
------------
*/
struct Colour
/*******************************************************************************
LAST MODIFIED : 9 November 1994

DESCRIPTION :
==============================================================================*/
{
	COLOUR_PRECISION blue,green,red;
}; /* struct Colour */

/*
Global functions
----------------
*/
struct Colour *create_Colour(COLOUR_PRECISION red,COLOUR_PRECISION green,
	COLOUR_PRECISION blue);
/*******************************************************************************
LAST MODIFIED : 11 November 1994

DESCRIPTION :
Allocates memory and assigns field for a colour.
==============================================================================*/

void destroy_Colour(struct Colour **colour_address);
/*******************************************************************************
LAST MODIFIED : 11 November 1994

DESCRIPTION :
Frees the memory for the colour and sets <*colour_address> to NULL.
==============================================================================*/

int set_Colour(struct Parse_state *state,void *colour_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 18 June 1996

DESCRIPTION :
A modifier function to set the colour rgb values.
==============================================================================*/
#endif
