/*******************************************************************************
FILE : rendergl.c

LAST MODIFIED : 16 November 2000

DESCRIPTION :
GL rendering calls - API specific.
???DB.  Should this be render.c ?
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
/*???DB.  For debug */
#include <stdio.h>
#include <math.h>
#if defined (OPENGL_API)
#include <GL/glu.h>
#endif /* defined (OPENGL_API) */
#include "general/debug.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/graphics_library.h"
#include "graphics/graphics_object.h"
#include "graphics/light_model.h"
#include "graphics/mcubes.h"
#include "graphics/rendergl.h"
#include "graphics/spectrum.h"
#include "user_interface/message.h"
#include "graphics/graphics_object_private.h"

/*
Global functions
----------------
*/

int draw_glyphsetGL(int number_of_points,Triple *point_list, Triple *axis1_list,
	Triple *axis2_list, Triple *axis3_list, Triple *scale_list,
	struct GT_object *glyph, char **labels,
	int number_of_data_components, GTDATA *data, int *names,
	struct Graphical_material *material, struct Spectrum *spectrum,
	int draw_selected, int some_selected,
	struct Multi_range *selected_name_ranges)
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Draws graphics object <glyph> at <number_of_points> points given by the
positions in <point_list> and oriented and scaled by <axis1_list>, <axis2_list>
and <axis3_list>, each axis additionally scaled by its value in <scale_list>.
If the glyph is part of a linked list through its nextobject
member, these attached glyphs are also executed.
Writes the <labels> array strings, if supplied, beside each glyph point.
If <names> are supplied these identify each point/glyph for OpenGL picking.
If <draw_selected> is set, then only those <names> in <selected_name_ranges>
are drawn, otherwise only those names not there are drawn.
If <some_selected> is true, <selected_name_ranges> indicates the points that
are selected, or all points if <selected_name_ranges> is NULL.
==============================================================================*/
{
	char **label;
	GLfloat f, f0, f1, transformation[16], x, y, z;
	GTDATA *datum;
	int draw_all, i, j, mirror_mode, *name, name_selected, number_of_glyphs,
		return_code;
	struct GT_object *temp_glyph;
	struct Spectrum_render_data *render_data;
	Triple *axis1, *axis2, *axis3, *point, *scale, temp_axis1, temp_axis2,
		temp_axis3, temp_point;

	ENTER(draw_glyphsetGL);
	if (((0 == number_of_points) || (0 < number_of_points) && point_list &&
		axis1_list && axis2_list && axis3_list && scale_list) && glyph)
	{
		if ((0==number_of_points) ||
			(draw_selected&&((!names) || (!some_selected)))||
			((!draw_selected)&&(some_selected && (!selected_name_ranges))))
		{
			/* nothing to draw */
			return_code=1;
		}
		else
		{
			mirror_mode = GT_object_get_glyph_mirror_mode(glyph);
			if (mirror_mode)
			{
				f = -1.0;
			}
			else
			{
				f = 0.0;
			}
#if defined (OPENGL_API)
			if ((!data)||(render_data=spectrum_start_renderGL
				(spectrum,material,number_of_data_components)))
			{
				draw_all = (!names) ||
					(draw_selected&&some_selected&&(!selected_name_ranges)) ||
					((!draw_selected)&&(!some_selected));
				point = point_list;
				axis1 = axis1_list;
				axis2 = axis2_list;
				axis3 = axis3_list;
				scale = scale_list;
				/* if there is data to plot, start the spectrum rendering */
				if (data)
				{
					datum=data;
				}
				if (name=names)
				{
					/* make space for picking name on name stack */
					glPushName(0);
				}
				label=labels;
				/* try to draw points and lines faster */
				if (0 == strcmp(glyph->name, "point"))
				{
					/* disable lighting so rendered in flat diffuse colour */
					/*???RC glPushAttrib and glPopAttrib are *very* slow */
					glPushAttrib(GL_ENABLE_BIT);
					glDisable(GL_LIGHTING);
					if (names||labels||selected_name_ranges)
					{
						/* cannot put glLoadName between glBegin and glEnd */
						for (i=0;i<number_of_points;i++)
						{
							if (draw_all||((name_selected=Multi_range_is_value_in_range(
								selected_name_ranges,*name))&&draw_selected)||
								((!name_selected)&&(!draw_selected)))
							{
								/* set the spectrum for this datum, if any */
								if (data)
								{
									spectrum_renderGL_value(spectrum,material,render_data,datum);
								}
								x = (*point)[0];
								y = (*point)[1];
								z = (*point)[2];
								if (names)
								{
									glLoadName((GLuint)(*name));
								}
								glBegin(GL_POINTS);
								glVertex3f(x, y, z);
								glEnd();
								if (labels && *label)
								{
									glRasterPos3f(x, y, z);
									wrapperPrintText(*label);
								}
							}
							/* advance pointers */
							point++;
							if (data)
							{
								datum += number_of_data_components;
							}
							if (names)
							{
								name++;
							}
							if (labels)
							{
								label++;
							}
						}
					}
					else
					{
						/* more efficient if all points put out between glBegin and glEnd */
						glBegin(GL_POINTS);
						for (i=0;i<number_of_points;i++)
						{
							/* set the spectrum for this datum, if any */
							if (data)
							{
								spectrum_renderGL_value(spectrum,material,render_data,datum);
								datum+=number_of_data_components;
							}
							x = (*point)[0];
							y = (*point)[1];
							z = (*point)[2];
							point++;
							glVertex3f(x,y,z);
						}
						glEnd();
					}
					/* restore previous lighting state */
					glPopAttrib();
				}
				else if ((0 == strcmp(glyph->name, "line")) ||
					(0 == strcmp(glyph->name, "mirror_line")))
				{
					/* disable lighting so rendered in flat diffuse colour */
					/*???RC glPushAttrib and glPopAttrib are *very* slow */
					glPushAttrib(GL_ENABLE_BIT);
					glDisable(GL_LIGHTING);
					if (names||labels||selected_name_ranges)
					{
						/* cannot put glLoadName between glBegin and glEnd */
						for (i=0;i<number_of_points;i++)
						{
							if (draw_all||((name_selected=Multi_range_is_value_in_range(
								selected_name_ranges,*name))&&draw_selected)||
								((!name_selected)&&(!draw_selected)))
							{
								/* set the spectrum for this datum, if any */
								if (data)
								{
									spectrum_renderGL_value(spectrum,material,render_data,datum);
								}
								if (names)
								{
									glLoadName((GLuint)(*name));
								}
								f0 = f*(*scale)[0];
								x = (*point)[0] + f0*(*axis1)[0];
								y = (*point)[1] + f0*(*axis1)[1];
								z = (*point)[2] + f0*(*axis1)[2];
								if (labels && *label)
								{
									glRasterPos3f(x,y,z);
									wrapperPrintText(*label);
								}
								glBegin(GL_LINES);
								glVertex3f(x,y,z);
								f1 = (*scale)[0];
								x = (*point)[0] + f1*(*axis1)[0];
								y = (*point)[1] + f1*(*axis1)[1];
								z = (*point)[2] + f1*(*axis1)[2];
								glVertex3f(x,y,z);
								glEnd();
							}
							/* advance pointers */
							point++;
							axis1++;
							scale++;
							if (data)
							{
								datum+=number_of_data_components;
							}
							if (names)
							{
								name++;
							}
							if (labels)
							{
								label++;
							}
						}
					}
					else
					{
						/* more efficient if all lines put out between glBegin and glEnd */
						glBegin(GL_LINES);
						for (i=0;i<number_of_points;i++)
						{
							/* set the spectrum for this datum, if any */
							if (data)
							{
								spectrum_renderGL_value(spectrum,material,render_data,datum);
								datum+=number_of_data_components;
							}
							f0 = f*(*scale)[0];
							x = (*point)[0] + f0*(*axis1)[0];
							y = (*point)[1] + f0*(*axis1)[1];
							z = (*point)[2] + f0*(*axis1)[2];
							glVertex3f(x,y,z);
							f1 = (*scale)[0];
							x = (*point)[0] + f1*(*axis1)[0];
							y = (*point)[1] + f1*(*axis1)[1];
							z = (*point)[2] + f1*(*axis1)[2];
							glVertex3f(x,y,z);
							point++;
							axis1++;
							scale++;
						}
						glEnd();
					}
					/* restore previous lighting state */
					glPopAttrib();
				}
				else if (0==strcmp(glyph->name,"cross"))
				{
					/* disable lighting so rendered in flat diffuse colour */
					/*???RC glPushAttrib and glPopAttrib are *very* slow */
					glPushAttrib(GL_ENABLE_BIT);
					glDisable(GL_LIGHTING);
					if (names||labels||selected_name_ranges)
					{
						/* cannot put glLoadName between glBegin and glEnd */
						for (i=0;i<number_of_points;i++)
						{
							if (draw_all||((name_selected=Multi_range_is_value_in_range(
								selected_name_ranges,*name))&&draw_selected)||
								((!name_selected)&&(!draw_selected)))
							{
								/* set the spectrum for this datum, if any */
								if (data)
								{
									spectrum_renderGL_value(spectrum,material,render_data,datum);
								}
								if (names)
								{
									glLoadName((GLuint)(*name));
								}
								resolve_glyph_axes(*point, *axis1, *axis2, *axis3, *scale,
									/*mirror*/0, /*reverse*/0,
									temp_point, temp_axis1, temp_axis2, temp_axis3);

								x = temp_point[0];
								y = temp_point[1];
								z = temp_point[2];
								if (labels && *label)
								{
									glRasterPos3f(x,y,z);
									wrapperPrintText(*label);
								}
								glBegin(GL_LINES);
								/* x-line */
								x = temp_point[0] - 0.5*temp_axis1[0];
								y = temp_point[1] - 0.5*temp_axis1[1];
								z = temp_point[2] - 0.5*temp_axis1[2];
								glVertex3f(x,y,z);
								x += temp_axis1[0];
								y += temp_axis1[1];
								z += temp_axis1[2];
								glVertex3f(x,y,z);
								/* y-line */
								x = temp_point[0] - 0.5*temp_axis2[0];
								y = temp_point[1] - 0.5*temp_axis2[1];
								z = temp_point[2] - 0.5*temp_axis2[2];
								glVertex3f(x,y,z);
								x += temp_axis2[0];
								y += temp_axis2[1];
								z += temp_axis2[2];
								glVertex3f(x,y,z);
								/* z-line */
								x = temp_point[0] - 0.5*temp_axis3[0];
								y = temp_point[1] - 0.5*temp_axis3[1];
								z = temp_point[2] - 0.5*temp_axis3[2];
								glVertex3f(x,y,z);
								x += temp_axis3[0];
								y += temp_axis3[1];
								z += temp_axis3[2];
								glVertex3f(x,y,z);
								glEnd();
							}
							/* advance pointers */
							point++;
							axis1++;
							axis2++;
							axis3++;
							scale++;
							if (data)
							{
								datum += number_of_data_components;
							}
							if (names)
							{
								name++;
							}
							if (labels)
							{
								label++;
							}
						}
					}
					else
					{
						/* more efficient if all lines put out between glBegin and glEnd */
						glBegin(GL_LINES);
						for (i=0;i<number_of_points;i++)
						{
							/* set the spectrum for this datum, if any */
							if (data)
							{
								spectrum_renderGL_value(spectrum,material,render_data,datum);
								datum += number_of_data_components;
							}
							resolve_glyph_axes(*point, *axis1, *axis2, *axis3, *scale,
								/*mirror*/0, /*reverse*/0,
								temp_point, temp_axis1, temp_axis2, temp_axis3);
							/* x-line */
							x = temp_point[0] - 0.5*temp_axis1[0];
							y = temp_point[1] - 0.5*temp_axis1[1];
							z = temp_point[2] - 0.5*temp_axis1[2];
							glVertex3f(x,y,z);
							x += temp_axis1[0];
							y += temp_axis1[1];
							z += temp_axis1[2];
							glVertex3f(x,y,z);
							/* y-line */
							x = temp_point[0] - 0.5*temp_axis2[0];
							y = temp_point[1] - 0.5*temp_axis2[1];
							z = temp_point[2] - 0.5*temp_axis2[2];
							glVertex3f(x,y,z);
							x += temp_axis2[0];
							y += temp_axis2[1];
							z += temp_axis2[2];
							glVertex3f(x,y,z);
							/* z-line */
							x = temp_point[0] - 0.5*temp_axis3[0];
							y = temp_point[1] - 0.5*temp_axis3[1];
							z = temp_point[2] - 0.5*temp_axis3[2];
							glVertex3f(x,y,z);
							x += temp_axis3[0];
							y += temp_axis3[1];
							z += temp_axis3[2];
							glVertex3f(x,y,z);
							point++;
							axis1++;
							axis2++;
							axis3++;
							scale++;
						}
						glEnd();
					}
					/* restore previous lighting state */
					glPopAttrib();
				}
				else
				{
					/* must push and pop the modelview matrix */
					glMatrixMode(GL_MODELVIEW);
					for (i = 0; i < number_of_points; i++)
					{
						if (draw_all||((name_selected=Multi_range_is_value_in_range(
							selected_name_ranges,*name))&&draw_selected)||
							((!name_selected)&&(!draw_selected)))
						{
							if (names)
							{
								glLoadName((GLuint)(*name));
							}
							/* set the spectrum for this datum, if any */
							if (data)
							{
								spectrum_renderGL_value(spectrum,material,render_data,datum);
							}
							if (mirror_mode)
							{
								number_of_glyphs = 2;
							}
							else
							{
								number_of_glyphs = 1;
							}
							for (j = 0; j < number_of_glyphs; j++)
							{
								/* store the current modelview matrix */
								glPushMatrix();
								resolve_glyph_axes(*point, *axis1, *axis2, *axis3, *scale,
									/*mirror*/j, /*reverse*/mirror_mode,
									temp_point, temp_axis1, temp_axis2, temp_axis3);

								/* make transformation matrix for manipulating glyph */
								transformation[ 0] = temp_axis1[0];
								transformation[ 1] = temp_axis1[1];
								transformation[ 2] = temp_axis1[2];
								transformation[ 3] = 0.0;
								transformation[ 4] = temp_axis2[0];
								transformation[ 5] = temp_axis2[1];
								transformation[ 6] = temp_axis2[2];
								transformation[ 7] = 0.0;
								transformation[ 8] = temp_axis3[0];
								transformation[ 9] = temp_axis3[1];
								transformation[10] = temp_axis3[2];
								transformation[11] = 0.0;
								transformation[12] = temp_point[0];
								transformation[13] = temp_point[1];
								transformation[14] = temp_point[2];
								transformation[15] = 1.0;
								glMultMatrixf(transformation);
								if (mirror_mode)
								{
									/* ignore first glyph since just a wrapper for the second */
									temp_glyph = GT_object_get_next_object(glyph);
								}
								else
								{
									temp_glyph = glyph;
								}
								/* call the glyph display lists of the linked-list of glyphs */
								while (temp_glyph)
								{
									glCallList(temp_glyph->display_list);
									temp_glyph = GT_object_get_next_object(temp_glyph);
								}
								/* restore the original modelview matrix */
								glPopMatrix();
							}
						}
						/* advance pointers */
						point++;
						axis1++;
						axis2++;
						axis3++;
						scale++;
						if (data)
						{
							datum += number_of_data_components;
						}
						if (names)
						{
							name++;
						}
					}
					/* output label at each point, if supplied */
					if (label=labels)
					{
						name=names;
						/* disable lighting so rendered in flat diffuse colour */
						/*???RC glPushAttrib and glPopAttrib are *very* slow */
						glPushAttrib(GL_ENABLE_BIT);
						glDisable(GL_LIGHTING);
						point=point_list;
						datum=data;
						for (i=0;i<number_of_points;i++)
						{
							if (draw_all||((name_selected=Multi_range_is_value_in_range(
								selected_name_ranges,*name))&&draw_selected)||
								((!name_selected)&&(!draw_selected))&&(*label))
							{
								if (names)
								{
									glLoadName((GLuint)(*name));
								}
								x=(*point)[0];
								y=(*point)[1];
								z=(*point)[2];
								/* set the spectrum for this datum, if any */
								if (data)
								{
									spectrum_renderGL_value(spectrum,material,render_data,datum);
								}
								glRasterPos3f(x,y,z);
								wrapperPrintText(*label);
							}
							/* advance pointers */
							point++;
							if (data)
							{
								datum += number_of_data_components;
							}
							if (names)
							{
								name++;
							}
							label++;
						}
						/* restore previous lighting state */
						glPopAttrib();
					}
				}
				if (names)
				{
					/* free space for point number on picking name stack */
					glPopName();
				}
				if (data)
				{
					spectrum_end_renderGL(spectrum,render_data);
				}
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"draw_glyphsetGL.  Unable to start data rendering");
				return_code=0;
			}
#endif /* defined (OPENGL_API) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_glyphsetGL. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_glyphsetGL */

int draw_pointsetGL(int n_pts,Triple *point_list,char **text,
	gtMarkerType marker_type,float marker_size,int *names,
	int number_of_data_components, GTDATA *data,
	struct Graphical_material *material, struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
???RC.  21/12/97 The optional names are supplied to allow identification of the
object with OpenGL picking. (used to use a accessed pointer to an FE_node which
did not allow other objects to be identified, nor nodes to be destroyed). Since
can't change picking names between glBegin and glEnd, must have a separate
begin/end bracket for each point. This may reduce rendering performance a
little, but on the upside, spectrum handling and text output are done
simultaneously.
==============================================================================*/
{
	char **text_string;
	float half_marker_size,x,x1,x2,x3,y,y1,y2,y3,z,z1,z2,z3;
	GTDATA *datum;
	int i,*name,offset,return_code;
	struct Spectrum_render_data *render_data;
	Triple *point;

	ENTER(draw_pointsetGL);
	/* default return code */
	return_code=0;
	/* checking arguments */
	if (point_list&&(0<n_pts))
	{
#if defined (OPENGL_API)
		if ((!data)||(render_data=spectrum_start_renderGL
			(spectrum,material,number_of_data_components)))
		{
			point=point_list;
			/* if there is data to plot, start the spectrum rendering */
			if (data)
			{
				datum = data;
			}
			/* draw markers */
			half_marker_size=marker_size/2;
			if ((g_POINT_MARKER==marker_type)||
				((g_PLUS_MARKER==marker_type)&&(0>=half_marker_size)))
			{
				glBegin(GL_POINTS);
				for (i=0;i<n_pts;i++)
				{
					/* set the spectrum for this datum, if any */
					if (data)
					{
						spectrum_renderGL_value(spectrum,material,render_data,datum);
						datum += number_of_data_components;
					}
					x=(*point)[0];
					y=(*point)[1];
					z=(*point)[2];
					point++;
					glVertex3f(x,y,z);
				}
				glEnd();
			}
			else
			{
				switch (marker_type)
				{
					case g_PLUS_MARKER:
					{
						glBegin(GL_LINES);
						for (i=0;i<n_pts;i++)
						{
							/* set the spectrum for this datum, if any */
							if (data)
							{
								spectrum_renderGL_value(spectrum,material,render_data,datum);
								datum += number_of_data_components;
							}
							x=(*point)[0];
							y=(*point)[1];
							z=(*point)[2];
							point++;
							/* first line */
							glVertex3f(x-half_marker_size,y,z);
							glVertex3f(x+half_marker_size,y,z);
							/* second line */
							glVertex3f(x,y-half_marker_size,z);
							glVertex3f(x,y+half_marker_size,z);
							/* third line */
							glVertex3f(x,y,z-half_marker_size);
							glVertex3f(x,y,z+half_marker_size);
						}
						glEnd();
					} break;
					case g_DERIVATIVE_MARKER:
					{
						glBegin(GL_LINES);
						for (i=0;i<n_pts;i++)
						{
							/* set the spectrum for this datum, if any */
							if (data)
							{
								spectrum_renderGL_value(spectrum,material,render_data,datum);
								datum += number_of_data_components;
							}
							x=(*point)[0];
							y=(*point)[1];
							z=(*point)[2];
							point++;
							x1=(*point)[0];
							y1=(*point)[1];
							z1=(*point)[2];
							point++;
							if ((x1!=x)||(y1!=y)||(z1!=z))
							{
								glVertex3f(x,y,z);
								glVertex3f(x1,y1,z1);
							}
							x2=(*point)[0];
							y2=(*point)[1];
							z2=(*point)[2];
							point++;
							if ((x2!=x)||(y2!=y)||(z2!=z))
							{
								glVertex3f(x,y,z);
								glVertex3f(x2,y2,z2);
							}
							x3=(*point)[0];
							y3=(*point)[1];
							z3=(*point)[2];
							point++;
							if ((x3!=x)||(y3!=y)||(z3!=z))
							{
								glVertex3f(x,y,z);
								glVertex3f(x3,y3,z3);
							}
						}
						glEnd();
					} break;
				}
			}
			/* output text at each point, if supplied */
			if (text_string = text)
			{
				point=point_list;
				datum=data;
				if (g_DERIVATIVE_MARKER==marker_type)
				{
					offset=4;
				}
				else
				{
					offset=1;
				}
				for (i=0;i<n_pts;i++)
				{
					x=(*point)[0];
					y=(*point)[1];
					z=(*point)[2];
					point += offset;
					/* set the spectrum for this datum, if any */
					if (data)
					{
						spectrum_renderGL_value(spectrum,material,render_data,datum);
						datum += number_of_data_components;
					}
					glRasterPos3f(x,y,z);
					wrapperPrintText(*text_string);
					text_string++;
				}
			}
			/* picking */
			if (name=names)
			{
				point=point_list;
				datum=data;
				/* make a first name to load over with each name */
				glPushName(0);
				switch (marker_type)
				{
					case g_POINT_MARKER:
					case g_PLUS_MARKER:
					{
						for (i=0;i<n_pts;i++)
						{
							x=(*point)[0];
							y=(*point)[1];
							z=(*point)[2];
							point++;
							/* set the spectrum for this datum, if any */
							if (data)
							{
								spectrum_renderGL_value(spectrum,material,render_data,datum);
								datum += number_of_data_components;
							}
							glLoadName((GLuint)(*name));
							glBegin(GL_POINTS);
							glVertex3f(x,y,z);
							glEnd();
							name++;
						}
					} break;
					case g_DERIVATIVE_MARKER:
					{
						for (i=0;i<n_pts;i++)
						{
							x=(*point)[0];
							y=(*point)[1];
							z=(*point)[2];
							point++;
							x1=(*point)[0];
							y1=(*point)[1];
							z1=(*point)[2];
							point++;
							x2=(*point)[0];
							y2=(*point)[1];
							z2=(*point)[2];
							point++;
							x3=(*point)[0];
							y3=(*point)[1];
							z3=(*point)[2];
							point++;
							/* set the spectrum for this datum, if any */
							if (data)
							{
								spectrum_renderGL_value(spectrum,material,render_data,datum);
								datum += number_of_data_components;
							}
							/* output names */
							glLoadName((GLuint)(*name));
							glBegin(GL_POINTS);
							glVertex3f(x,y,z);
							glEnd();
							if ((x1!=x)||(y1!=y)||(z1!=z))
							{
								glPushName(1);
								glBegin(GL_POINTS);
								glVertex3f(x1,y1,z1);
								glEnd();
								glPopName();
							}
							if ((x2!=x)||(y2!=y)||(z2!=z))
							{
								glPushName(2);
								glBegin(GL_POINTS);
								glVertex3f(x2,y2,z2);
								glEnd();
								glPopName();
							}
							if ((x3!=x)||(y3!=y)||(z3!=z))
							{
								glPushName(3);
								glBegin(GL_POINTS);
								glVertex3f(x3,y3,z3);
								glEnd();
								glPopName();
							}
						}
						name++;
					} break;
				}
				glPopName();
			}
			if (data)
			{
				spectrum_end_renderGL(spectrum,render_data);
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"draw_pointsetGL.  Missing spectrum");
			return_code=0;
		}
#endif /* defined (OPENGL_API) */
	}
	else
	{
		if (0<n_pts)
		{
			display_message(ERROR_MESSAGE,"drawpointsetGL. Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* draw_pointsetGL */

int draw_polylineGL(Triple *point_list, Triple *normal_list, int n_pts,
	int number_of_data_components, GTDATA *data,
	struct Graphical_material *material,struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
==============================================================================*/
{
	int return_code;
#if defined (OPENGL_API)
	GTDATA *datum;
	int i;
	Triple *normal, *point;
#endif /* defined (OPENGL_API) */
	struct Spectrum_render_data *render_data;

	ENTER(drawpolylineGL);
#if defined (DEBUG)
	printf("draw_polylineGL %d\n",n_pts);
#endif /* defined (DEBUG) */

	/* checking arguments */
	if (point_list&&(0<n_pts)&&((!data)||(render_data=
		spectrum_start_renderGL(spectrum,material,number_of_data_components))))
	{
#if defined (OPENGL_API)
		point=point_list;
		if (normal_list)
		{
			normal = normal_list;
		}
		if (data)
		{
			datum=data;			
		}
		glBegin(GL_LINE_STRIP);
		for (i=n_pts;i>0;i--)
		{
			if (data)
			{
				spectrum_renderGL_value(spectrum,material,render_data,datum);
				datum += number_of_data_components;
			}
			if (normal_list)
			{
				glNormal3fv(*normal);
				normal++;
			}
			glVertex3fv(*point);
			point++;
		}
		glEnd();
		if (data)
		{
			spectrum_end_renderGL(spectrum,render_data);
		}
		return_code=1;
	}
	else
	{
		if (0<n_pts)
		{
			display_message(ERROR_MESSAGE,"draw_polylineGL.  Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
#endif /* defined (OPENGL_API) */
	}
	LEAVE;

	return (return_code);
} /* draw_polylineGL */

int draw_dc_polylineGL(Triple *point_list,Triple *normal_list, int n_pts, 
	int number_of_data_components, GTDATA *data,
	struct Graphical_material *material,struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
==============================================================================*/
{
	int return_code;
#if defined (OPENGL_API)
	int i;
	Triple *point, *normal;
	GTDATA *datum;
#endif /* defined (OPENGL_API) */
	struct Spectrum_render_data *render_data;

	ENTER(draw_dc_polylineGL);

	/* checking arguments */
	if (point_list&&(0<n_pts)&&((!data)||(render_data=
		spectrum_start_renderGL(spectrum,material,number_of_data_components))))
	{
#if defined (OPENGL_API)
		point=point_list;
		if (data)
		{
			datum=data;
		}
		if (normal_list)
		{
			normal = normal_list;
		}
		glBegin(GL_LINES);
		for (i=n_pts;i>0;i--)
		{
			if (data)
			{
				spectrum_renderGL_value(spectrum,material,render_data,datum);
				datum += number_of_data_components;
			}
			if (normal_list)
			{
				glNormal3fv(*normal);
				normal++;
			}
			glVertex3fv(*point);
			point++;
			if (data)
			{
				spectrum_renderGL_value(spectrum,material,render_data,datum);
				datum += number_of_data_components;
			}
			if (normal_list)
			{
				glNormal3fv(*normal);
				normal++;
			}
			glVertex3fv(*point);
			point++;
		}
		glEnd();
#endif /* defined (OPENGL_API) */
		if (data)
		{
			spectrum_end_renderGL(spectrum,render_data);
		}
		return_code=1;
	}
	else
	{
		if (0<n_pts)
		{
			display_message(ERROR_MESSAGE,"draw_dc_polylineGL.  Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* draw_dc_polylineGL */

int draw_surfaceGL(Triple *surfpts, Triple *normalpoints, Triple *tangentpoints,
	Triple *texturepoints, int npts1, int npts2, gtPolygonType polygon_type,
	int number_of_data_components, GTDATA *data, 
	struct Graphical_material *material, struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 28 November 2003

DESCRIPTION :
==============================================================================*/
{
	GTDATA *data_1,*data_2;
	int i,j,return_code;
	Triple *surface_point_1,*surface_point_2, *normal_point_1, *normal_point_2,
		*texture_point_1, *texture_point_2;
#if defined GL_VERSION_1_3
	Triple *tangent_point_1, *tangent_point_2;
#endif /* defined GL_VERSION_1_3 */
	struct Spectrum_render_data *render_data;

	ENTER(draw_surfaceGL);
#if ! defined GL_VERSION_1_3
	USE_PARAMETER(tangentpoints);
#endif /* ! defined GL_VERSION_1_3 */
	/* checking arguments */
	if (surfpts&&(1<npts1)&&(1<npts2)&&((!data)||(render_data=
		spectrum_start_renderGL(spectrum,material,number_of_data_components))))
	{
#if defined (OPENGL_API)
#if defined GL_VERSION_1_3
		if (tangentpoints)
		{
			if (!Graphics_library_check_extension(GL_VERSION_1_3))
			{
				/* Ignore these tangentpoints */
				tangentpoints = (Triple *)NULL;
			}
		}
#endif /* defined GL_VERSION_1_3 */
		switch (polygon_type)
		{
			case g_QUADRILATERAL:
			{
				for (i=0;i<npts1-1;i++)
				{
					glBegin(GL_QUAD_STRIP);
					for (j=0;j<npts2;j++)
					{
						if (normalpoints)
						{
							glNormal3fv(normalpoints[i+npts1*j]);
						}
#if defined GL_VERSION_1_3
						if (tangentpoints)
						{
							glMultiTexCoord3fv(GL_TEXTURE1_ARB,tangentpoints[i+npts1*j]);
						}
#endif /* defined GL_VERSION_1_3 */
						if (texturepoints)
						{
							glTexCoord3fv(texturepoints[i+npts1*j]);
						}
						/* putting the spectrum render after the definition of the texture
							coordinates allows the spectrum to override them */
						if (data)
						{
							spectrum_renderGL_value(spectrum,material,render_data,data+
								number_of_data_components*(i+npts1*j));
						}
						glVertex3fv(surfpts[i+npts1*j]);
						if (normalpoints)
						{
							glNormal3fv(normalpoints[i+npts1*j+1]);
						}
#if defined GL_VERSION_1_3
						if (tangentpoints)
						{
							glMultiTexCoord3fv(GL_TEXTURE1_ARB,tangentpoints[i+npts1*j+1]);
						}
#endif /* defined GL_VERSION_1_3 */
						if (texturepoints)
						{
							glTexCoord3fv(texturepoints[i+npts1*j+1]);
						}
						if (data)
						{
							spectrum_renderGL_value(spectrum,material,render_data,data+
								number_of_data_components*(i+npts1*j+1));
						}
						glVertex3fv(surfpts[i+npts1*j+1]);
					}
					glEnd();
				}
			} break;
			case g_TRIANGLE:
			{
				surface_point_1=surfpts;
				surface_point_2=surfpts+npts1;
				if (normalpoints)
				{
					normal_point_1=normalpoints;
					normal_point_2=normalpoints+npts1;
				}
#if defined GL_VERSION_1_3
				if (tangentpoints)
				{
					tangent_point_1=tangentpoints;
					tangent_point_2=tangentpoints+npts1;
				}
#endif /* defined GL_VERSION_1_3 */
				if (texturepoints)
				{
					texture_point_1 = texturepoints;
					texture_point_2 = texturepoints+npts1;				
				}
				if (data)
				{
					data_1=data;
					data_2=data+npts1*number_of_data_components;
				}
				for (i=npts1-1;i>0;i--)
				{
					glBegin(GL_TRIANGLE_STRIP);
					if (normalpoints)
					{
						glNormal3fv(*normal_point_1);
						normal_point_1++;
					}
#if defined GL_VERSION_1_3
					if (tangentpoints)
					{
						glMultiTexCoord3fv(GL_TEXTURE1_ARB,*tangent_point_1);
						tangent_point_1++;
					}
#endif /* defined GL_VERSION_1_3 */
					if (texturepoints)
					{
						glTexCoord3fv(*texture_point_1);
						texture_point_1++;
					}
					if (data)
					{
						spectrum_renderGL_value(spectrum,material,render_data,
							data_1);
						data_1 += number_of_data_components;
					}
					glVertex3fv(*surface_point_1);
					surface_point_1++;
					for (j=i;j>0;j--)
					{
						if (normalpoints)
						{
							glNormal3fv(*normal_point_2);
							normal_point_2++;
						}
#if defined GL_VERSION_1_3
						if (tangentpoints)
						{
							glMultiTexCoord3fv(GL_TEXTURE1_ARB,*tangent_point_2);
							tangent_point_2++;
						}
#endif /* defined GL_VERSION_1_3 */
						if (texturepoints)
						{
							glTexCoord3fv(*texture_point_2);
							texture_point_2++;
						}
						if (data)
						{
							spectrum_renderGL_value(spectrum,material,render_data,
								data_2);
							data_2 += number_of_data_components;
						}
						glVertex3fv(*surface_point_2);
						surface_point_2++;
						if (normalpoints)
						{
							glNormal3fv(*normal_point_1);
							normal_point_1++;
						}
#if defined GL_VERSION_1_3
						if (tangentpoints)
						{
							glMultiTexCoord3fv(GL_TEXTURE1_ARB,*tangent_point_1);
							tangent_point_1++;
						}
#endif /* defined GL_VERSION_1_3 */
						if (texturepoints)
						{
							glTexCoord3fv(*texture_point_1);
							texture_point_1++;
						}
						if (data)
						{
							spectrum_renderGL_value(spectrum,material,render_data,
								data_1);
							data_1 += number_of_data_components;
						}
						glVertex3fv(*surface_point_1);
						surface_point_1++;
					}
					glEnd();
				}
			} break;
		}
#endif /* defined (OPENGL_API) */
		if (data)
		{
			spectrum_end_renderGL(spectrum,render_data);
		}
		return_code=1;
	}
	else
	{
		if ((1<npts1)&&(1<npts2))
		{
			display_message(ERROR_MESSAGE,
				"draw_surfaceGL.  Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* draw_surfaceGL */

int draw_dc_surfaceGL(Triple *surfpts, Triple *normal_points, 
	Triple *texture_points, 
	int npolys,int npp,gtPolygonType polygon_type,int strip,
	int number_of_data_components,GTDATA *data,
	struct Graphical_material *material,struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
If the <polygon_type> is g_TRIANGLE or g_QUADRILATERAL, the discontinuous
sets of vertices are drawn as separate triangles or quadrilaterals - or as
strips of the respective types if the <strip> flag is set. Otherwise a single
polygon is drawn for each of the <npolys>.
==============================================================================*/
{
#if defined (OPENGL_API)
	GLenum mode;
#endif /* defined (OPENGL_API) */
	GTDATA *data_item;
	int i,j,return_code;
	struct Spectrum_render_data *render_data;
	Triple *normal_point, *point, *texture_point;

	ENTER(draw_data_dc_surfaceGL);
	/* checking arguments */
	if (surfpts&&(0<npolys)&&(2<npp)&&((!data)||(render_data=
		spectrum_start_renderGL(spectrum,material,number_of_data_components))))
	{
		if (data)
		{
			data_item=data;
		}
		point=surfpts;
		if (normal_points)
		{
			normal_point = normal_points;
		}
		if (texture_points)
		{
			texture_point = texture_points;
		}
#if defined (OPENGL_API)
		switch (polygon_type)
		{
			case g_QUADRILATERAL:
			{
				if (strip)
				{
					mode=GL_QUAD_STRIP;
				}
				else
				{
					mode=GL_QUADS;
				}
			} break;
			case g_TRIANGLE:
			{
				if (strip)
				{
					mode=GL_TRIANGLE_STRIP;
				}
				else
				{
					mode=GL_TRIANGLES;
				}
			} break;
			default:
			{
				mode=GL_POLYGON;
			}
		}
		for (i=0;i<npolys;i++)
		{
			glBegin(mode);
			for (j=0;j<npp;j++)
			{
				if (data)
				{
					spectrum_renderGL_value(spectrum,material,render_data,data_item);
					data_item += number_of_data_components;
				}
				if (normal_points)
				{
					glNormal3fv(*normal_point);
					normal_point++;
				}
				if (texture_points)
				{
					glNormal3fv(*texture_point);
					texture_point++;
				}				
				glVertex3fv(*point);
				point++;
			}
			glEnd();
		}
#endif /* defined (OPENGL_API) */
		if (data)
		{
			spectrum_end_renderGL(spectrum,render_data);
		}
		return_code=1;
	}
	else
	{
		if (0<npolys)
		{
			display_message(ERROR_MESSAGE,
				"draw_dc_surfaceGL.  Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* draw_dc_surfaceGL */

int draw_nurbsGL(struct GT_nurbs *nurbptr)
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
==============================================================================*/
{
	int return_code;
#if defined (OPENGL_API)
	GLfloat *cknots_temp,*control_temp,*normal_temp,*pwl_temp,*sknots_temp,*tknots_temp,
		*texture_temp,*trimarray_temp;
	GLUnurbsObj *the_nurb;
	int i;
#endif /* defined (OPENGL_API) */

	ENTER(draw_nurbsGL);
	/* default return code */
	return_code=0;
	/* checking arguments */
	if (nurbptr)
	{
#if defined (OPENGL_API)
		/* allocate local temporary arrays */
		ALLOCATE(sknots_temp,GLfloat,nurbptr->sknotcnt);
		ALLOCATE(tknots_temp,GLfloat,nurbptr->tknotcnt);
		ALLOCATE(control_temp,GLfloat,4*(nurbptr->maxs)*(nurbptr->maxt));
		/* check memory allocation */
		if (sknots_temp&&tknots_temp&&control_temp)
		{
			/* move sknots into temp float array */
			for (i=0;i<nurbptr->sknotcnt;i++)
			{
				sknots_temp[i]=(GLfloat)nurbptr->sknots[i];
			}
			/* move tknots into a temp float array */
			for (i=0;i<nurbptr->tknotcnt;i++)
			{
				tknots_temp[i]=(GLfloat)nurbptr->tknots[i];
			}
			if (nurbptr->normal_control_points)
			{
				/* move normals into a temp float array */
				ALLOCATE(normal_temp,GLfloat,4*(nurbptr->maxs)*(nurbptr->maxt));
				for (i=0;i<(4*nurbptr->maxt*nurbptr->maxs);i++)
				{
					normal_temp[i]=(GLfloat)nurbptr->normal_control_points[i];
				}
			}
			if (nurbptr->texture_control_points)
			{
				/* move textures into a temp float array */
				ALLOCATE(texture_temp,GLfloat,4*(nurbptr->maxs)*(nurbptr->maxt));
				for (i=0;i<(4*nurbptr->maxt*nurbptr->maxs);i++)
				{
					texture_temp[i]=(GLfloat)nurbptr->texture_control_points[i];
				}
			}
			if (nurbptr->cknotcnt>0)
			{
				/* move cknots into a temp float array */
				ALLOCATE(cknots_temp,float,nurbptr->cknotcnt);
				for (i=0;i<nurbptr->cknotcnt;i++)
				{
				cknots_temp[i]=(GLfloat)nurbptr->cknots[i];
				}
			}
			/* move controlpts into a temp float array */
			for (i=0;i<(4*nurbptr->maxt*nurbptr->maxs);i++)
			{
				control_temp[i]=(GLfloat)nurbptr->controlpts[i];
			}
			if (nurbptr->ccount>0)
			{
				/* move trimarray into a temp float array */
				ALLOCATE(trimarray_temp,float,3*nurbptr->ccount);
				for (i=0;i<(3*nurbptr->ccount);i++)
				{
					trimarray_temp[i]=(GLfloat)nurbptr->trimarray[i];
				}
			}
			if (nurbptr->pwlcnt>0)
			{
				/* move pwlarray into a temp float array */
				ALLOCATE(pwl_temp,float,3*nurbptr->pwlcnt);
				for (i=0;i<(3*nurbptr->pwlcnt);i++)
				{
					pwl_temp[i]=(GLfloat)nurbptr->pwlarray[i];
				}
			}
			/* store the evaluator attributes */
			glPushAttrib(GL_EVAL_BIT);
			if(!nurbptr->normal_control_points)
			{
				/* automatically calculate surface normals */
				glEnable(GL_AUTO_NORMAL);
			}
			the_nurb=gluNewNurbsRenderer();
			/* set the maximum sampled pixel size */
#if defined (OLD_CODE)
			gluNurbsProperty(the_nurb,GLU_DISPLAY_MODE,GLU_OUTLINE_POLYGON);
			gluNurbsProperty(the_nurb,GLU_SAMPLING_TOLERANCE,5000.);
			gluNurbsProperty(the_nurb,GLU_SAMPLING_TOLERANCE,
				(GLfloat)nurbptr->nurbsprop);
#endif /* defined (OLD_CODE) */
			/* start the nurbs surface */
			gluBeginSurface(the_nurb);
			/* draw the nurbs surface */
			gluNurbsSurface(the_nurb,nurbptr->sknotcnt,sknots_temp,nurbptr->tknotcnt,
				tknots_temp,4,4*(nurbptr->maxs),control_temp,nurbptr->sorder,
				nurbptr->torder,GL_MAP2_VERTEX_4);
			if(nurbptr->normal_control_points)
			{
				gluNurbsSurface(the_nurb,nurbptr->sknotcnt,sknots_temp,nurbptr->tknotcnt,
					tknots_temp,4,4*(nurbptr->maxs),normal_temp,nurbptr->sorder,
					nurbptr->torder,GL_MAP2_NORMAL);
			}
			if(nurbptr->texture_control_points)
			{
				gluNurbsSurface(the_nurb,nurbptr->sknotcnt,sknots_temp,nurbptr->tknotcnt,
					tknots_temp,4,4*(nurbptr->maxs),texture_temp,nurbptr->sorder,
					nurbptr->torder,GL_MAP2_TEXTURE_COORD_3);
			}
			if (nurbptr->cknotcnt>0)
			{
				/* trim the nurbs surface with a nurbs curve */
				gluBeginTrim(the_nurb);
				gluNurbsCurve(the_nurb,nurbptr->cknotcnt,cknots_temp,3,trimarray_temp,
					nurbptr->corder,GLU_MAP1_TRIM_3);
				gluEndTrim(the_nurb);
			}
			if (nurbptr->pwlcnt)
			{
				/* trim the nurbs surface with a pwl curve */
				gluBeginTrim(the_nurb);
				gluPwlCurve(the_nurb,nurbptr->pwlcnt,pwl_temp,3,GLU_MAP1_TRIM_3);
				gluEndTrim(the_nurb);
			}
			/* end the nurbs surface */
			gluEndSurface(the_nurb);
			/* restore the evaluator attributes */
			glPopAttrib();
		}
		else
		{
			display_message(ERROR_MESSAGE,"drawnurbsGL.	Memory allocation failed");
			return_code=0;
		}
		/* deallocate local temp arrays */
		DEALLOCATE(control_temp);
		if (nurbptr->ccount>0)
		{
			DEALLOCATE(trimarray_temp);
		}
		if (nurbptr->normal_control_points)
		{
			DEALLOCATE(normal_temp);
		}
		if (nurbptr->texture_control_points)
		{
			DEALLOCATE(texture_temp);
		}
		if (nurbptr->cknotcnt>0)
		{
			DEALLOCATE(cknots_temp);
		}
		DEALLOCATE(sknots_temp);
		DEALLOCATE(tknots_temp);
		if(nurbptr->pwlcnt>0)
		{
			DEALLOCATE(pwl_temp);
		}
#endif /* defined (OPENGL_API) */
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"drawnurbsGL.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_nurbsGL */

int draw_voltexGL(int n_iso_polys,int *triangle_list,
	struct VT_iso_vertex *vertex_list,int n_vertices,int n_rep,
	struct Graphical_material **per_vertex_materials,
	int *iso_poly_material_index,
	struct Environment_map **per_vertex_environment_maps,
	int *iso_poly_environment_map_index,
	float *texturemap_coord,int *texturemap_index,int number_of_data_components,
	GTDATA *data, struct Graphical_material *default_material,
	struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 8 August 2002

DESCRIPTION :
Numbers in <iso_poly_material_index> are indices into the materials in the
<per_vertex_materials>. A zero value denotes use of the default material;
and index of 1 means the first material in the per_vertex_materials. Not
supplying the <iso_poly_material_index> gives the default material to all
vertices.
Use of <iso_poly_environment_map_index> and <per_vertex_environment_maps> is
exactly the same as for materials. Note environment map materials are used in
preference to normal materials.
==============================================================================*/
{
	int first, i, ii, return_code;
	struct Environment_map *environment_map;
	struct Graphical_material *last_material, *next_material;
	struct Spectrum_render_data *render_data;

	ENTER(draw_voltexGL);
	/* default return code */
	return_code = 0;
	/* checking arguments */
	if (triangle_list && vertex_list &&
		((!iso_poly_material_index) || per_vertex_materials) &&
		((!iso_poly_environment_map_index) || per_vertex_environment_maps) &&
		((!texturemap_coord) && (!texturemap_index) ||
			(texturemap_coord && texturemap_index)) &&
		(0 < n_rep) && (0 < n_iso_polys))
	{
#if defined (OPENGL_API)
		last_material = (struct Graphical_material *)NULL;
		if (iso_poly_environment_map_index &&
			iso_poly_environment_map_index[0])
		{
			if (environment_map = per_vertex_environment_maps[
				iso_poly_environment_map_index[0] - 1])
			{
				last_material =
					environment_map->face_material[texturemap_index[0]];
			}
		}
		else if (iso_poly_material_index && iso_poly_material_index[0])
		{
			last_material = per_vertex_materials[iso_poly_material_index[0] - 1];
		}
		if (!last_material)
		{
			last_material = default_material;
		}
		first = 1;
		if ((!data)||(render_data=spectrum_start_renderGL(spectrum,last_material,
				number_of_data_components)))
		{
			for (ii = 0; ii < n_rep; ii++)
			{
				for (i = 0; i < n_iso_polys; i++)
				{
					next_material = default_material;
					/* if an environment map exists use it in preference to a material */
					if (iso_poly_environment_map_index &&
						iso_poly_environment_map_index[i*3])
					{
						if (environment_map = per_vertex_environment_maps[
							iso_poly_environment_map_index[i*3] - 1])
						{
							next_material =
								environment_map->face_material[texturemap_index[i*3]];
						}
					}
					else
					{
						if (iso_poly_material_index && iso_poly_material_index[i*3])
						{
							next_material =
								per_vertex_materials[iso_poly_material_index[i*3] - 1];
						}
					}
					if (!next_material)
					{
						next_material = default_material;
					}
					if (first || (next_material != last_material))
					{
						if (last_material &&
							Graphical_material_get_texture(last_material) &&
							next_material &&
							(!Graphical_material_get_texture(next_material)))
						{
							/* turn off last texture */
							execute_Texture((struct Texture *)NULL);
						}
						execute_Graphical_material(next_material);
						last_material = next_material;
						first = 0;
					}

					glBegin(GL_TRIANGLES);
					if (texturemap_coord)
					{
						glTexCoord3fv(&(texturemap_coord[3*(3*i+0)]));
					}
					if (data)
					{
						spectrum_renderGL_value(spectrum,last_material,render_data,
							data+vertex_list[triangle_list[i*3+0]+n_vertices*ii].data_index);
					}
					glNormal3fv(
						&(vertex_list[triangle_list[i*3+0]+n_vertices*ii].normal[0]));
					glVertex3fv(
						&(vertex_list[triangle_list[i*3+0]+n_vertices*ii].coord[0]));

					next_material = default_material;
					if (iso_poly_environment_map_index &&
						iso_poly_environment_map_index[i*3+2])
					{
						if (environment_map = per_vertex_environment_maps[
							iso_poly_environment_map_index[i*3+2] - 1])
						{
							next_material =
								environment_map->face_material[texturemap_index[i*3+2]];
						}
					}
					else
					{
						if (iso_poly_material_index && iso_poly_material_index[i*3+2])
						{
							next_material =
								per_vertex_materials[iso_poly_material_index[i*3+2] - 1];
						}
					}
					if (!next_material)
					{
						next_material = default_material;
					}
					/* Note cannot change material per vertex if it has a texture */
					if ((next_material != last_material) && next_material && 
						(!Graphical_material_get_texture(next_material)))
					{
						execute_Graphical_material(next_material);
						last_material=next_material;
					}

					if (texturemap_coord)
					{
						glTexCoord3fv(&(texturemap_coord[3*(3*i+2)]));
					}
					if (data)
					{
						spectrum_renderGL_value(spectrum,last_material,render_data,
							data+vertex_list[triangle_list[i*3+2]+n_vertices*ii].data_index);
					}
					glNormal3fv(
						&(vertex_list[triangle_list[i*3+2]+n_vertices*ii].normal[0]));
					glVertex3fv(
						&(vertex_list[triangle_list[i*3+2]+n_vertices*ii].coord[0]));

					next_material=default_material;
					if (iso_poly_environment_map_index &&
						iso_poly_environment_map_index[i*3+1])
					{
						if (environment_map = per_vertex_environment_maps[
							iso_poly_environment_map_index[i*3+1] - 1])
						{
							next_material =
								environment_map->face_material[texturemap_index[i*3+1]];
						}
					}
					else
					{
						if (iso_poly_material_index && iso_poly_material_index[i*3+1])
						{
							next_material =
								per_vertex_materials[iso_poly_material_index[i*3+1] - 1];
						}
					}
					if (!next_material)
					{
						next_material = default_material;
					}
					/* Note cannot change material per vertex if it has a texture */
					if ((next_material != last_material) && next_material && 
						(!Graphical_material_get_texture(next_material)))
					{
						execute_Graphical_material(next_material);
						last_material=next_material;
					}

					if (texturemap_coord)
					{
						glTexCoord3fv(&(texturemap_coord[3*(3*i+1)]));
					}
					if (data)
					{
						spectrum_renderGL_value(spectrum,last_material,render_data,
							data+vertex_list[triangle_list[i*3+1]+n_vertices*ii].data_index);
					}
					glNormal3fv(
						&(vertex_list[triangle_list[i*3+1]+n_vertices*ii].normal[0]));
					glVertex3fv(
						&(vertex_list[triangle_list[i*3+1]+n_vertices*ii].coord[0]));

					glEnd();
				}
			}
			if (data)
			{
				spectrum_end_renderGL(spectrum,render_data);
			}
		}
#endif /* defined (OPENGL_API) */
		return_code=1;
	}
	else
	{
		if ((0<n_rep)&&(0<n_iso_polys))
		{
			display_message(ERROR_MESSAGE,"draw_voltexGL.  Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* draw_voltexGL */
