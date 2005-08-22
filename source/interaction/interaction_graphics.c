/*******************************************************************************
FILE : interaction_graphics.c

LAST MODIFIED : 10 July 2000

DESCRIPTION :
Functions for building graphics assisting interaction, eg. rubber-band effect.
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
#include "general/debug.h"
#include "interaction/interaction_graphics.h"
#include "user_interface/message.h"

/*
Module functions
----------------
*/

int Interaction_volume_make_polyline_extents(
	struct Interaction_volume *interaction_volume,
	struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
Fills <graphics_object> - of type g_POLYLINE with lines marking the box
enclosing the <interaction volume>, used for rubber-banding. Lines are put at
time 0 in the graphics object; any other primitives at that time are cleared.
==============================================================================*/
{
	double model_vertex[8][3];
	/* use 0.999999 to avoid clipping problems for frustums */
	static double normalised_vertex[8][3]=
	{
		{-0.9999,-0.9999,-0.9999},
		{-0.9999,-0.9999, 0.9999},
		{-0.9999, 0.9999,-0.9999},
		{-0.9999, 0.9999, 0.9999},
		{ 0.9999,-0.9999,-0.9999},
		{ 0.9999,-0.9999, 0.9999},
		{ 0.9999, 0.9999,-0.9999},
		{ 0.9999, 0.9999, 0.9999}
	};
	float time;
	int i,return_code;
	struct GT_polyline *polyline;
	Triple *point,*points;

	ENTER(Interaction_volume_make_polyline_extents);
	if (interaction_volume&&graphics_object&&
		(g_POLYLINE==GT_object_get_type(graphics_object)))
	{
		return_code=1;
		/* get the 8 vertices of the interaction_volume frustum */
		for (i=0;(i<8)&&return_code;i++)
		{
			return_code=Interaction_volume_normalised_to_model_coordinates(
				interaction_volume,normalised_vertex[i],model_vertex[i]);
		}
		if (return_code)
		{
			/* make a polyline big enough for the 12 line segment edges of the box */
			if (ALLOCATE(points,Triple,24)&&
				(polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,/*line_width=default*/0,
					12,points,/*normalpoints*/NULL,0,(GTDATA *)NULL)))
			{
				point=points;
				/* lines spanning axis 1 */
				for (i=0;i<4;i++)
				{
					(*point)[0]=(float)model_vertex[i][0];
					(*point)[1]=(float)model_vertex[i][1];
					(*point)[2]=(float)model_vertex[i][2];
					point++;
					(*point)[0]=(float)model_vertex[i+4][0];
					(*point)[1]=(float)model_vertex[i+4][1];
					(*point)[2]=(float)model_vertex[i+4][2];
					point++;
				}
				/* lines spanning axis 2 */
				for (i=0;i<2;i++)
				{
					(*point)[0]=(float)model_vertex[i*4][0];
					(*point)[1]=(float)model_vertex[i*4][1];
					(*point)[2]=(float)model_vertex[i*4][2];
					point++;
					(*point)[0]=(float)model_vertex[i*4+2][0];
					(*point)[1]=(float)model_vertex[i*4+2][1];
					(*point)[2]=(float)model_vertex[i*4+2][2];
					point++;
					(*point)[0]=(float)model_vertex[i*4+1][0];
					(*point)[1]=(float)model_vertex[i*4+1][1];
					(*point)[2]=(float)model_vertex[i*4+1][2];
					point++;
					(*point)[0]=(float)model_vertex[i*4+3][0];
					(*point)[1]=(float)model_vertex[i*4+3][1];
					(*point)[2]=(float)model_vertex[i*4+3][2];
					point++;
				}
				/* lines spanning axis 3 */
				for (i=0;i<8;i++)
				{
					(*point)[0]=(float)model_vertex[i][0];
					(*point)[1]=(float)model_vertex[i][1];
					(*point)[2]=(float)model_vertex[i][2];
					point++;
				}
				/* put all graphics in at time 0 */
				time=0.0;
				if (GT_object_has_time(graphics_object,time))
				{
					GT_object_remove_primitives_at_time(graphics_object,time,
						(GT_object_primitive_object_name_conditional_function *)NULL,
						NULL);
				}
				if (!GT_OBJECT_ADD(GT_polyline)(graphics_object,time,polyline))
				{
					display_message(ERROR_MESSAGE,
						"Interaction_volume_make_polyline_extents.  "
						"Could not add primitive");
					return_code=0;
					DESTROY(GT_polyline)(&polyline);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Interaction_volume_make_polyline_extents.  Not enough memory)");
				DEALLOCATE(points);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Interaction_volume_make_polyline_extents.  Invalid volume");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Interaction_volume_make_polyline_extents.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Interaction_volume_make_polyline_extents */

