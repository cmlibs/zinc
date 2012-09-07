/*******************************************************************************
FILE : selected_graphic.c

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
#include <stdio.h>
#include "general/compare.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/object.h"
#include "graphics/selected_graphic.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct Selected_graphic
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
An item in a list indicating that the object with the given number is selected.
Used with graphics objects to mark selected objects for highlighting. The
optional subranges allow subobjects within a graphics object primitive to be
individually selected for highlighting.
==============================================================================*/
{
	int number;
	struct Multi_range *subranges;
	int access_count;
}; /* struct Selected_graphic */

FULL_DECLARE_INDEXED_LIST_TYPE(Selected_graphic);

/*
Module functions
----------------
*/

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Selected_graphic,number,int,compare_int)

/*
Global functions
----------------
*/

struct Selected_graphic *CREATE(Selected_graphic)(int number)
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Creates a Selected_graphics for the given number. Its subranges will be NULL.
==============================================================================*/
{
	struct Selected_graphic *selected_graphic;

	ENTER(CREATE(Selected_graphic));
	if (ALLOCATE(selected_graphic,struct Selected_graphic,1))
	{
		selected_graphic->number=number;
		selected_graphic->subranges=(struct Multi_range *)NULL;
		selected_graphic->access_count=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Selected_graphic).  Not enough memory");
	}
	LEAVE;

	return (selected_graphic);
} /* CREATE(Selected_graphic) */

int DESTROY(Selected_graphic)(
	struct Selected_graphic **selected_graphic_address)
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Destroys the Selected_graphic.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Selected_graphic));
	if (selected_graphic_address&&*selected_graphic_address)
	{
		if (0==(*selected_graphic_address)->access_count)
		{
			if ((*selected_graphic_address)->subranges)
			{
				DESTROY(Multi_range)(&((*selected_graphic_address)->subranges));
			}
			DEALLOCATE(*selected_graphic_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Selected_graphic).  Non-zero access count!");
			*selected_graphic_address=(struct Selected_graphic *)NULL;
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Selected_graphic).   Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Selected_graphic) */

struct Multi_range *Selected_graphic_get_subranges(
	struct Selected_graphic *selected_graphic)
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Returns the subranges in <selected_graphic>. Note the return value will be the
pointer to the subranges Multi_range in the selected_graphics; this may be NULL,
indicating there are no selected subranges. Also, it should not be modified -
its availability in this way is purely for efficiency of rendering.
==============================================================================*/
{
	struct Multi_range *subranges;

	ENTER(Selected_graphic_get_subranges);
	if (selected_graphic)
	{
		subranges=selected_graphic->subranges;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Selected_graphic_get_subranges.  Invalid argument(s)");
		subranges=(struct Multi_range *)NULL;
	}
	LEAVE;

	return (subranges);
} /* Selected_graphic_get_subranges */

int Selected_graphic_set_subranges(struct Selected_graphic *selected_graphic,
	struct Multi_range *subranges)
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
DESTROYs any subranges in <selected_graphic> and replaces them with the given
<subranges>, which may be NULL. Note that this function does not make a copy of
the given <subranges> - they will now be owned by the <selected_graphic>.
==============================================================================*/
{
	int return_code;

	ENTER(Selected_graphic_set_subranges);
	if (selected_graphic)
	{
		if (selected_graphic->subranges)
		{
			DESTROY(Multi_range)(&(selected_graphic->subranges));
		}
		selected_graphic->subranges=subranges;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Selected_graphic_set_subranges.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Selected_graphic_set_subranges */

DECLARE_OBJECT_FUNCTIONS(Selected_graphic)

DECLARE_INDEXED_LIST_FUNCTIONS(Selected_graphic)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Selected_graphic, \
	number,int,compare_int)
