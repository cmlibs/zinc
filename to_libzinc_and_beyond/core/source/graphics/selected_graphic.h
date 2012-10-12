/*******************************************************************************
FILE : selected_graphic.h

LAST MODIFIED : 18 February 2000

DESCRIPTION :
Indexed lists for storing selected objects in graphics by number, and ranges
of subobject numbers if required.
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
#if !defined (SELECTED_GRAPHIC_H)
#define SELECTED_GRAPHIC_H

#include "general/list.h"
#include "general/multi_range.h"
#include "general/object.h"

/*
Global types
------------
*/

struct Selected_graphic;
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
The contents of this object are private.
==============================================================================*/

DECLARE_LIST_TYPES(Selected_graphic);

/*
Global functions
----------------
*/

struct Selected_graphic *CREATE(Selected_graphic)(int number);
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Creates a Selected_graphics for the given number. Its subranges will be NULL.
==============================================================================*/

int DESTROY(Selected_graphic)(
	struct Selected_graphic **selected_graphic_address);
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Destroys the Selected_graphic.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Selected_graphic);
PROTOTYPE_LIST_FUNCTIONS(Selected_graphic);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Selected_graphic,number,int);

struct Multi_range *Selected_graphic_get_subranges(
	struct Selected_graphic *selected_graphic);
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Returns the subranges in <selected_graphic>. Note the return value will be the
pointer to the subranges Multi_range in the selected_graphics; this may be NULL,
indicating there are no selected subranges. Also, it should not be modified -
its availability in this way is purely for efficiency of rendering.
==============================================================================*/

int Selected_graphic_set_subranges(struct Selected_graphic *selected_graphic,
	struct Multi_range *subranges);
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
DESTROYs any subranges in <selected_graphic> and replaces them with the given
<subranges>, which may be NULL. Note that this function does not make a copy of
the given <subranges> - they will now be owned by the <selected_graphic>.
==============================================================================*/

#endif /* !defined (SELECTED_GRAPHIC_H) */
