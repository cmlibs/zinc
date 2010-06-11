/*******************************************************************************
FILE : cmiss_rendition.c

DESCRIPTION :
The public interface to the cmiss_rendition which supplies graphics of region 
to Cmgui.
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

#include "api/cmiss_rendition.h"
#include "general/debug.h"
#include "graphics/cmiss_graphic.h"
#include "graphics/cmiss_rendition.h"

Cmiss_rendition_id Cmiss_rendition_get_from_region(Cmiss_region_id root_region)
{
	return Cmiss_region_get_rendition(root_region);
}

Cmiss_graphic_id Cmiss_rendition_create_lines(Cmiss_rendition_id rendition)
{
	Cmiss_graphic_id graphic = NULL;

	if (rendition)
	{
		if (NULL != (graphic=CREATE(Cmiss_graphic)(CMISS_GRAPHIC_LINES)))
		{
			Cmiss_rendition_add_graphic(rendition, graphic, -1);
		}
	}
	return graphic;
}

Cmiss_graphic_id Cmiss_rendition_create_surfaces(Cmiss_rendition_id rendition)
{
	Cmiss_graphic_id graphic = NULL;
	if (rendition)
	{
		if (NULL != (graphic=CREATE(Cmiss_graphic)(CMISS_GRAPHIC_SURFACES)))
		{
			Cmiss_rendition_add_graphic(rendition, graphic, -1);
		}
	}
	return ACCESS(Cmiss_graphic)(graphic);
}

Cmiss_graphic_id Cmiss_rendition_create_node_points(Cmiss_rendition_id rendition)
{
	Cmiss_graphic_id graphic = NULL;
	if (rendition)
	{
		if (NULL != (graphic=CREATE(Cmiss_graphic)(CMISS_GRAPHIC_NODE_POINTS)))
		{
			Cmiss_rendition_add_graphic(rendition, graphic, -1);
		}
	}
	return ACCESS(Cmiss_graphic)(graphic);
}

Cmiss_graphic_id Cmiss_rendition_create_static(Cmiss_rendition_id rendition)
{
	Cmiss_graphic_id graphic = NULL;
	if (rendition)
	{
		if (NULL != (graphic=CREATE(Cmiss_graphic)(CMISS_GRAPHIC_STATIC)))
		{
			Cmiss_rendition_add_graphic(rendition, graphic, -1);
		}
	}
	return ACCESS(Cmiss_graphic)(graphic);
}
