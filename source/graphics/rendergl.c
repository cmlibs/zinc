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
#include "graphics/font.h"
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

static int draw_glyphsetGL(int number_of_points,Triple *point_list, Triple *axis1_list,
	Triple *axis2_list, Triple *axis3_list, Triple *scale_list,
	struct GT_object *glyph, char **labels,
	int number_of_data_components, GTDATA *data, int *names,
	int label_bounds_dimension, int label_bounds_components, float *label_bounds,
	struct Graphical_material *material, struct Graphical_material *secondary_material, 
	struct Spectrum *spectrum, struct Graphics_font *font, int draw_selected, int some_selected,
	struct Multi_range *selected_name_ranges, struct GT_object_compile_context *context)
/*******************************************************************************
LAST MODIFIED : 22 November 2005

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
	float *label_bound;
	GLfloat f, f0, f1, transformation[16], x, y, z;
	Graphics_object_glyph_labels_function glyph_labels_function;
	GTDATA *datum;
	int draw_all, i, j, mirror_mode, *name, name_selected, label_bounds_per_glyph,
		number_of_glyphs, return_code;
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
				label_bound = label_bounds;
				if (label_bounds)
				{
					label_bounds_per_glyph = pow(2, label_bounds_dimension);
				}
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
									Graphics_font_rendergl_text(font, *label);
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
									Graphics_font_rendergl_text(font, *label);
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
									Graphics_font_rendergl_text(font, *label);
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
								if (glyph_labels_function = Graphics_object_get_glyph_labels_function(glyph))
								{
									return_code = (*glyph_labels_function)(*scale,
										label_bounds_dimension, label_bounds_components, label_bound,
										material, secondary_material, font, context);
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
						if (label_bounds)
						{
							label_bound += label_bounds_components * label_bounds_per_glyph;
						}
					}
					/* output label at each point, if supplied */
					if ((label=labels) && !label_bounds)
					{
						/* Default is to draw the label value at the origin */
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

								/* Default is to draw the label value near the origin */
								glRasterPos3f(x,y,z);
								Graphics_font_rendergl_text(font, *label);
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

static int draw_pointsetGL(int n_pts,Triple *point_list,char **text,
	gtMarkerType marker_type,float marker_size,int *names,
	int number_of_data_components, GTDATA *data,
	struct Graphical_material *material, struct Spectrum *spectrum,
	struct Graphics_font *font)
/*******************************************************************************
LAST MODIFIED : 18 November 2005

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
					Graphics_font_rendergl_text(font, *text_string);
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

static int draw_polylineGL(Triple *point_list, Triple *normal_list, int n_pts,
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

static int draw_dc_polylineGL(Triple *point_list,Triple *normal_list, int n_pts, 
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

static int draw_surfaceGL(Triple *surfpts, Triple *normalpoints, Triple *tangentpoints,
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

static int draw_dc_surfaceGL(Triple *surfpts, Triple *normal_points, 
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

static int draw_nurbsGL(struct GT_nurbs *nurbptr)
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

static int draw_voltexGL(int number_of_vertices, struct VT_iso_vertex **vertex_list,
	int number_of_triangles, struct VT_iso_triangle **triangle_list,
	int number_of_data_components, int number_of_texture_coordinates,
	struct Graphical_material *default_material, struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 11 November 2005

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
	int i, j, return_code;
	struct Spectrum_render_data *render_data;
	struct VT_iso_triangle *triangle;
	struct VT_iso_vertex *vertex;

	ENTER(draw_voltexGL);
	/* default return code */
	return_code = 0;
	/* checking arguments */
	if (triangle_list && vertex_list && (0 < number_of_vertices) && (0 < number_of_triangles))
	{
#if defined (OPENGL_API)
		if ((!number_of_data_components) ||
			(render_data=spectrum_start_renderGL(spectrum,default_material,
				number_of_data_components)))
		{
			glBegin(GL_TRIANGLES);
			for (i = 0; i < number_of_triangles; i++)
			{
				triangle = triangle_list[i];
				for (j = 0 ; j < 3 ; j++)
				{
					vertex = triangle->vertices[j];
					if (number_of_data_components)
					{
						spectrum_renderGL_value(spectrum,default_material,render_data,
							vertex->data);
					}
					if (number_of_texture_coordinates)
					{
						glTexCoord3fv(vertex->texture_coordinates);
					}
					glNormal3fv(vertex->normal);
					glVertex3fv(vertex->coordinates);
				}
			}
			glEnd();
			if (number_of_data_components)
			{
				spectrum_end_renderGL(spectrum,render_data);
			}
#endif /* defined (OPENGL_API) */
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"draw_voltexGL.  Unable to render data.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_voltexGL.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_voltexGL */

int render_GT_object_opengl(gtObject *object, struct GT_object_compile_context *context)
/*******************************************************************************
LAST MODIFIED : 27 October 2005

DESCRIPTION :
Convert graphical object into API object.
The <context> is used to control how the object is compiled.
==============================================================================*/
{
	float proportion,*times;
	int itime, name_selected, number_of_times, picking_names, return_code, strip,
		wireframe_flag;
#if defined (OPENGL_API)
	int lighting_off, line_width;
#endif /* defined (OPENGL_API) */
	struct Graphical_material *material, *secondary_material;
	struct GT_glyph_set *interpolate_glyph_set,*glyph_set,*glyph_set_2;
	struct GT_nurbs *nurbs;
	struct GT_point *point;
	struct GT_pointset *interpolate_point_set,*point_set,*point_set_2;
	struct GT_polyline *interpolate_line,*line,*line_2;
	struct GT_surface *interpolate_surface,*surface,*surface_2;
	struct GT_userdef *userdef;
	struct GT_voltex *voltex;
	struct Multi_range *selected_name_ranges;
	struct Spectrum *spectrum;
	union GT_primitive_list *primitive_list1, *primitive_list2;

	ENTER(makegtobject);
/*???debug */
/*printf("enter makegtobject %d  %d %d %d\n",GT_object_get_type(object),g_POINTSET,
	g_POLYLINE,g_SURFACE);*/
	/* check arguments */
	if (object)
	{
		return_code = 1;
		spectrum=get_GT_object_spectrum(object);
		/* determine if picking names are to be output */
		picking_names=(GRAPHICS_NO_SELECT != GT_object_get_select_mode(object));
		/* determine which material to use */
		if (context->draw_selected)
		{
			material = get_GT_object_selected_material(object);
		}
		else
		{
			material = get_GT_object_default_material(object);
		}
		secondary_material = get_GT_object_secondary_material(object);
		switch (object->coordinate_system)
		{
			case g_MODEL_COORDINATES:
			{
				/* Do nothing */
			} break;
			case g_NDC_COORDINATES:
			{
				glCallList(context->ndc_display_list);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,"makegtobject.  Invalid object coordinate system.");
				return_code=0;				
			} break;
		}
		number_of_times = GT_object_get_number_of_times(object);
		if (0 < number_of_times)
		{
			itime = number_of_times;
			if ((itime > 1) && (times = object->times))
			{
				itime--;
				times += itime;
				if (context->time >= *times)
				{
					proportion = 0;
				}
				else
				{
					while ((itime>0)&&(context->time < *times))
					{
						itime--;
						times--;
					}
					if (context->time< *times)
					{
						proportion=0;
					}
					else
					{
						proportion=times[1]-times[0];
						if (proportion>0)
						{
							proportion=(context->time-times[0])/proportion;
						}
						else
						{
							proportion=0;
						}
					}
				}
			}
			else
			{
				itime = 0;
				proportion = 0;
			}
			if (object->primitive_lists &&
				(primitive_list1 = object->primitive_lists + itime))
			{
				if (proportion > 0)
				{
					if (!(primitive_list2 = object->primitive_lists + itime + 1))
					{
						display_message(ERROR_MESSAGE,
							"makegtobject.  Invalid primitive_list");
						return_code = 0;
					}
				}
				else
				{
					primitive_list2 = (union GT_primitive_list *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"makegtobject.  Invalid primitive_lists");
				return_code = 0;
			}
		}
		if ((0 < number_of_times) && return_code)
		{
			switch (GT_object_get_type(object))
			{
				case g_GLYPH_SET:
				{
					if (glyph_set = primitive_list1->gt_glyph_set.first)
					{
#if defined (OPENGL_API)
						/* store the transform attribute group to save current matrix mode
							 and GL_NORMALIZE flag. */
						glPushAttrib(GL_TRANSFORM_BIT);
						/* Must enable GL_NORMALIZE so that normals are normalized after
							 scaling and shear by the transformations in the glyph set -
							 otherwise lighting will be wrong. Since this may reduce
							 performance, only enable for glyph_sets. */
						glEnable(GL_NORMALIZE);
						if (picking_names)
						{
							glPushName(0);
						}
#endif /* defined (OPENGL_API) */
						if (proportion > 0)
						{
							glyph_set_2 = primitive_list2->gt_glyph_set.first;
							while (glyph_set&&glyph_set_2)
							{
								if (interpolate_glyph_set=morph_GT_glyph_set(proportion,
									glyph_set,glyph_set_2))
								{
									if (picking_names)
									{
										/* put out name for picking - cast to GLuint */
										glLoadName((GLuint)interpolate_glyph_set->object_name);
									}
									/* work out if subobjects selected */
									selected_name_ranges=(struct Multi_range *)NULL;
									name_selected=GT_object_is_graphic_selected(object,
										glyph_set->object_name,&selected_name_ranges);
									draw_glyphsetGL(interpolate_glyph_set->number_of_points,
										interpolate_glyph_set->point_list,
										interpolate_glyph_set->axis1_list,
										interpolate_glyph_set->axis2_list,
										interpolate_glyph_set->axis3_list,
										interpolate_glyph_set->scale_list,
										interpolate_glyph_set->glyph,
										interpolate_glyph_set->labels,
										interpolate_glyph_set->n_data_components,
										interpolate_glyph_set->data,
										interpolate_glyph_set->names,
										/*label_bounds_dimension*/0, /*label_bounds_components*/0, /*label_bounds*/(float *)NULL,
										material, secondary_material, spectrum, 
										interpolate_glyph_set->font,
										context->draw_selected,name_selected,selected_name_ranges,
										context);
									DESTROY(GT_glyph_set)(&interpolate_glyph_set);
								}
								glyph_set=glyph_set->ptrnext;
								glyph_set_2=glyph_set_2->ptrnext;
							}
						}
						else
						{
							while (glyph_set)
							{
								if (picking_names)
								{
									/* put out name for picking - cast to GLuint */
									glLoadName((GLuint)glyph_set->object_name);
								}
								/* work out if subobjects selected */
								selected_name_ranges=(struct Multi_range *)NULL;
								name_selected=GT_object_is_graphic_selected(object,
									glyph_set->object_name,&selected_name_ranges);
								draw_glyphsetGL(glyph_set->number_of_points,
									glyph_set->point_list, glyph_set->axis1_list,
									glyph_set->axis2_list, glyph_set->axis3_list,
									glyph_set->scale_list, glyph_set->glyph,
									glyph_set->labels, glyph_set->n_data_components,
									glyph_set->data, glyph_set->names,
									glyph_set->label_bounds_dimension, glyph_set->label_bounds_components,
									glyph_set->label_bounds, material, secondary_material, 
									spectrum, glyph_set->font, 
									context->draw_selected, name_selected, selected_name_ranges,
									context);
								glyph_set=glyph_set->ptrnext;
							}
						}
#if defined (OPENGL_API)
						if (picking_names)
						{
							glPopName();
						}
						/* restore the transform attribute group */
						glPopAttrib();
#endif /* defined (OPENGL_API) */
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"makegtobject.  Missing glyph_set");
						return_code=0;
					}
				} break;
				case g_POINT:
				{
					if (point = primitive_list1->gt_point.first)
					{
						draw_pointsetGL(1, point->position, &(point->text),
							point->marker_type,
							point->marker_size, /*names*/(int *)NULL, 
							point->n_data_components, point->data,
							material,spectrum,point->font);
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"makegtobject.  Missing point");
						return_code=0;
					}
				} break;
				case g_POINTSET:
				{
					if (point_set = primitive_list1->gt_pointset.first)
					{
#if defined (OPENGL_API)
						/* disable lighting so rendered in flat diffuse colour */
						/*???RC glPushAttrib and glPopAttrib are *very* slow */
						glPushAttrib(GL_ENABLE_BIT);
						glDisable(GL_LIGHTING);
#if defined GL_ARB_vertex_program && defined GL_ARB_fragment_program
						if (Graphics_library_check_extension(GL_ARB_vertex_program) &&
							Graphics_library_check_extension(GL_ARB_fragment_program))
						{
							glDisable(GL_VERTEX_PROGRAM_ARB);
							glDisable(GL_FRAGMENT_PROGRAM_ARB);
							glDisable(GL_VERTEX_PROGRAM_TWO_SIDE_ARB);
						}
#endif /* defined GL_ARB_vertex_program && defined GL_ARB_fragment_program */
#endif /* defined (OPENGL_API) */
						if (proportion>0)
						{
							point_set_2 = primitive_list2->gt_pointset.first;
							while (point_set&&point_set_2)
							{
								if (interpolate_point_set=morph_GT_pointset(proportion,
									point_set,point_set_2))
								{
									draw_pointsetGL(interpolate_point_set->n_pts,
										interpolate_point_set->pointlist,
										interpolate_point_set->text,
										interpolate_point_set->marker_type,
										interpolate_point_set->marker_size, point_set->names,
										interpolate_point_set->n_data_components,
										interpolate_point_set->data,
										material,spectrum,interpolate_point_set->font);
									DESTROY(GT_pointset)(&interpolate_point_set);
								}
								point_set=point_set->ptrnext;
								point_set_2=point_set_2->ptrnext;
							}
						}
						else
						{
							while (point_set)
							{
								draw_pointsetGL(point_set->n_pts,point_set->pointlist,
									point_set->text,point_set->marker_type,point_set->marker_size,
									point_set->names,point_set->n_data_components,point_set->data,
									material,spectrum,point_set->font);
								point_set=point_set->ptrnext;
							}
						}
#if defined (OPENGL_API)
						/* restore previous lighting state */
						glPopAttrib();
#endif /* defined (OPENGL_API) */
						return_code=1;
					}
					else
					{
						/*???debug*/printf("! makegtobject.  Missing point");
						display_message(ERROR_MESSAGE,"makegtobject.  Missing point");
						return_code=0;
					}
				} break;
				case g_VOLTEX:
				{
					voltex = primitive_list1->gt_voltex.first;
#if defined (OPENGL_API)
					/* save transformation attributes state */
					if (voltex)
					{
						if (voltex->voltex_type == g_VOLTEX_WIREFRAME_SHADED_TEXMAP)
						{
							glPushAttrib(GL_TRANSFORM_BIT | GL_POLYGON_BIT);
							glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);							
						}
						else
						{
							glPushAttrib(GL_TRANSFORM_BIT);
						}
						/*???RC Why do we need NORMALIZE on for voltex? */
						glEnable(GL_NORMALIZE);
						if (picking_names)
						{
							glPushName(0);
						}
#endif /* defined (OPENGL_API) */
						while (voltex)
						{
							/* work out if subobjects selected */
							selected_name_ranges=(struct Multi_range *)NULL;
							name_selected=GT_object_is_graphic_selected(object,
								voltex->object_name,&selected_name_ranges);
							if ((name_selected&&context->draw_selected)||
								((!name_selected)&&(!context->draw_selected)))
							{
								if (picking_names)
								{
									/* put out name for picking - cast to GLuint */
									glLoadName(voltex->object_name);
								}
								draw_voltexGL(voltex->number_of_vertices, voltex->vertex_list,
									voltex->number_of_triangles, voltex->triangle_list,
									voltex->n_data_components, voltex->n_texture_coordinates,
									material,spectrum);
							}
							voltex=voltex->ptrnext;
						}
						return_code=1;
#if defined (OPENGL_API)
						if (picking_names)
						{
							glPopName();
						}
						/* restore previous coloring state */
						glPopAttrib();
#endif /* defined (OPENGL_API) */
					}
				} break;
				case g_POLYLINE:
				{
					/*???debug */
					/*printf("g_POLYLINE time=%g proportion=%g\n",time,proportion);*/
					if (line = primitive_list1->gt_polyline.first)
					{
						if (proportion>0)
						{
							line_2 = primitive_list2->gt_polyline.first;
						}
#if defined (OPENGL_API)
						if (lighting_off=((g_PLAIN == line->polyline_type)||
							(g_PLAIN_DISCONTINUOUS == line->polyline_type)))
						{
							/* disable lighting so rendered in flat diffuse colour */
							/*???RC glPushAttrib and glPopAttrib are *very* slow */
							glPushAttrib(GL_ENABLE_BIT);
							glDisable(GL_LIGHTING);
#if defined GL_ARB_vertex_program && defined GL_ARB_fragment_program
							if (Graphics_library_check_extension(GL_ARB_vertex_program) &&
								Graphics_library_check_extension(GL_ARB_fragment_program))
							{
								glDisable(GL_VERTEX_PROGRAM_ARB);
								glDisable(GL_FRAGMENT_PROGRAM_ARB);
								glDisable(GL_VERTEX_PROGRAM_TWO_SIDE_ARB);
							}
#endif /* defined GL_ARB_vertex_program && defined GL_ARB_fragment_program */
						}
						if (picking_names)
						{
							glPushName(0);
						}
#endif /* defined (OPENGL_API) */
						switch (line->polyline_type)
						{
							case g_PLAIN:
							case g_NORMAL:
							{
								if (proportion>0)
								{
									while (line&&line_2)
									{
										/* work out if subobjects selected */
										selected_name_ranges=(struct Multi_range *)NULL;
										name_selected=GT_object_is_graphic_selected(object,
											line->object_name,&selected_name_ranges);
										if ((name_selected&&context->draw_selected)||
											((!name_selected)&&(!context->draw_selected)))
										{
											if (interpolate_line=morph_GT_polyline(proportion,line,
												line_2))
											{
												if (picking_names)
												{
													/* put out name for picking - cast to GLuint */
													glLoadName((GLuint)interpolate_line->object_name);
												}
												draw_polylineGL(interpolate_line->pointlist,
													interpolate_line->normallist, interpolate_line->n_pts,
													interpolate_line->n_data_components,
													interpolate_line->data, material,
													spectrum);
												DESTROY(GT_polyline)(&interpolate_line);
											}
										}
										line=line->ptrnext;
										line_2=line_2->ptrnext;
									}
								}
								else
								{
									line_width = 0;
									while (line)
									{
										/* work out if subobjects selected */
										selected_name_ranges=(struct Multi_range *)NULL;
										name_selected=GT_object_is_graphic_selected(object,
											line->object_name,&selected_name_ranges);
										if ((name_selected&&context->draw_selected)||
											((!name_selected)&&(!context->draw_selected)))
										{
											if (picking_names)
											{
												/* put out name for picking - cast to GLuint */
												glLoadName((GLuint)line->object_name);
											}
											if (line->line_width != line_width)
											{
												if (line->line_width)
												{
													glLineWidth(line->line_width);
												}
												else
												{
													glLineWidth(global_line_width);
												}
												line_width = line->line_width;
											}
											draw_polylineGL(line->pointlist,line->normallist,
												line->n_pts, line->n_data_components, line->data,
												material,spectrum);
										}
										line=line->ptrnext;
									}
									if (line_width)
									{
										glLineWidth(global_line_width);
									}
								}
								return_code=1;
							} break;
							case g_PLAIN_DISCONTINUOUS:
							case g_NORMAL_DISCONTINUOUS:
							{
								if (proportion>0)
								{
									while (line&&line_2)
									{
										/* work out if subobjects selected */
										selected_name_ranges=(struct Multi_range *)NULL;
										name_selected=GT_object_is_graphic_selected(object,
											line->object_name,&selected_name_ranges);
										if ((name_selected&&context->draw_selected)||
											((!name_selected)&&(!context->draw_selected)))
										{
											if (interpolate_line=morph_GT_polyline(proportion,line,
												line_2))
											{
												if (picking_names)
												{
													/* put out name for picking - cast to GLuint */
													glLoadName((GLuint)interpolate_line->object_name);
												}
												draw_dc_polylineGL(interpolate_line->pointlist,
													interpolate_line->normallist, interpolate_line->n_pts,
													interpolate_line->n_data_components,
													interpolate_line->data,
													material,spectrum);
												DESTROY(GT_polyline)(&interpolate_line);
											}
										}
										line=line->ptrnext;
										line_2=line_2->ptrnext;
									}
								}
								else
								{
									while (line)
									{
										/* work out if subobjects selected */
										selected_name_ranges=(struct Multi_range *)NULL;
										name_selected=GT_object_is_graphic_selected(object,
											line->object_name,&selected_name_ranges);
										if ((name_selected&&context->draw_selected)||
											((!name_selected)&&(!context->draw_selected)))
										{
											if (picking_names)
											{
												/* put out name for picking - cast to GLuint */
												glLoadName((GLuint)line->object_name);
											}
											draw_dc_polylineGL(line->pointlist,line->normallist, 
												line->n_pts,line->n_data_components,line->data,
												material,spectrum);
										}
										line=line->ptrnext;
									}
								}
								return_code=1;
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"makegtobject.  Invalid line type");
								return_code=0;
							} break;
						}
#if defined (OPENGL_API)
						if (picking_names)
						{
							glPopName();
						}
						if (lighting_off)
						{
							/* restore previous lighting state */
							glPopAttrib();
						}
#endif /* defined (OPENGL_API) */
					}
					else
					{
						/*???debug*/printf("! makegtobject.  Missing line");
						display_message(ERROR_MESSAGE,"makegtobject.  Missing line");
						return_code=0;
					}
				} break;
				case g_SURFACE:
				{
					if (surface = primitive_list1->gt_surface.first)
					{
#if defined (OPENGL_API)
						if (picking_names)
						{
							glPushName(0);
						}
#endif /* defined (OPENGL_API) */
						if (proportion>0)
						{
							surface_2 = primitive_list2->gt_surface.first;
						}
						switch (surface->surface_type)
						{
							case g_SHADED:
							case g_SHADED_TEXMAP:
							case g_WIREFRAME_SHADED_TEXMAP:
							{
								if (surface->surface_type == g_WIREFRAME_SHADED_TEXMAP)
								{
									glPushAttrib(GL_POLYGON_BIT);
									glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
									wireframe_flag = 1;
								}
								else
								{
									wireframe_flag = 0;
								}
								if (proportion>0)
								{
									while (surface&&surface_2)
									{
										/* work out if subobjects selected */
										selected_name_ranges=(struct Multi_range *)NULL;
										name_selected=GT_object_is_graphic_selected(object,
											surface->object_name,&selected_name_ranges);
										if ((name_selected&&context->draw_selected)||
											((!name_selected)&&(!context->draw_selected)))
										{
											if (interpolate_surface=morph_GT_surface(proportion,
												surface,surface_2))
											{
												if (picking_names)
												{
													/* put out name for picking - cast to GLuint */
													glLoadName((GLuint)interpolate_surface->object_name);
												}
												draw_surfaceGL(interpolate_surface->pointlist,
													interpolate_surface->normallist,
													interpolate_surface->tangentlist,
													interpolate_surface->texturelist,
													interpolate_surface->n_pts1,
													interpolate_surface->n_pts2,
													interpolate_surface->polygon,
													interpolate_surface->n_data_components,
													interpolate_surface->data,
													material, spectrum);
												DESTROY(GT_surface)(&interpolate_surface);
											}
										}
										surface=surface->ptrnext;
										surface_2=surface_2->ptrnext;
									}
								}
								else
								{
									while (surface)
									{
										/* work out if subobjects selected */
										selected_name_ranges=(struct Multi_range *)NULL;
										name_selected=GT_object_is_graphic_selected(object,
											surface->object_name,&selected_name_ranges);
										if ((name_selected&&context->draw_selected)||
											((!name_selected)&&(!context->draw_selected)))
										{
											if (picking_names)
											{
												/* put out name for picking - cast to GLuint */
												glLoadName((GLuint)surface->object_name);
											}
											draw_surfaceGL(surface->pointlist, surface->normallist,
												surface->tangentlist,
												surface->texturelist, surface->n_pts1,
												surface->n_pts2, surface->polygon,
												surface->n_data_components, surface->data,
												material, spectrum);
										}
										surface=surface->ptrnext;
									}
								}
								if (wireframe_flag)
								{
									glPopAttrib();
								}
								return_code=1;
							} break;
							case g_SH_DISCONTINUOUS:
							case g_SH_DISCONTINUOUS_STRIP:
							case g_SH_DISCONTINUOUS_TEXMAP:
							case g_SH_DISCONTINUOUS_STRIP_TEXMAP:
							{
								strip=((g_SH_DISCONTINUOUS_STRIP_TEXMAP==surface->surface_type)
									||(g_SH_DISCONTINUOUS_STRIP==surface->surface_type));
								if (proportion>0)
								{
									while (surface&&surface_2)
									{
										/* work out if subobjects selected */
										selected_name_ranges=(struct Multi_range *)NULL;
										name_selected=GT_object_is_graphic_selected(object,
											surface->object_name,&selected_name_ranges);
										if ((name_selected&&context->draw_selected)||
											((!name_selected)&&(!context->draw_selected)))
										{
											if (interpolate_surface=morph_GT_surface(proportion,
												surface,surface_2))
											{
												if (picking_names)
												{
													/* put out name for picking - cast to GLuint */
													glLoadName((GLuint)interpolate_surface->object_name);
												}
												draw_dc_surfaceGL(interpolate_surface->pointlist,
													interpolate_surface->normallist,
													interpolate_surface->texturelist,
													interpolate_surface->n_pts1,
													interpolate_surface->n_pts2,
													interpolate_surface->polygon,strip,
													interpolate_surface->n_data_components,
													interpolate_surface->data,
													material,spectrum);
												DESTROY(GT_surface)(&interpolate_surface);
											}
										}
										surface=surface->ptrnext;
										surface_2=surface_2->ptrnext;
									}
								}
								else
								{
									while (surface)
									{
										/* work out if subobjects selected */
										selected_name_ranges=(struct Multi_range *)NULL;
										name_selected=GT_object_is_graphic_selected(object,
											surface->object_name,&selected_name_ranges);
										if ((name_selected&&context->draw_selected)||
											((!name_selected)&&(!context->draw_selected)))
										{
											if (picking_names)
											{
												/* put out name for picking - cast to GLuint */
												glLoadName((GLuint)surface->object_name);
											}
											draw_dc_surfaceGL(surface->pointlist,surface->normallist,
												surface->texturelist,surface->n_pts1,surface->n_pts2,
												surface->polygon,strip, surface->n_data_components,
												surface->data,material,spectrum);
										}
										surface=surface->ptrnext;
									}
								}
								return_code=1;
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"makegtobject.  Invalid surface type");
								return_code=0;
							} break;
						}
#if defined (OPENGL_API)
						if (picking_names)
						{
							glPopName();
						}
#endif /* defined (OPENGL_API) */
					}
					else
					{
						/*???debug*/printf("! makegtobject.  Missing surface");
						display_message(ERROR_MESSAGE,"makegtobject.  Missing surface");
						return_code=0;
					}
				} break;
				case g_NURBS:
				{
#if defined (OPENGL_API)
					/* store transformation attributes and GL_NORMALIZE */
					glPushAttrib(GL_TRANSFORM_BIT);
					glEnable(GL_NORMALIZE);
					if (picking_names)
					{
						glPushName(0);
					}
#endif /* defined (OPENGL_API) */
					if (nurbs = primitive_list1->gt_nurbs.first)
					{
						return_code = 1;
						while(return_code && nurbs)
						{
							if (picking_names)
							{
								/* put out name for picking - cast to GLuint */
								glLoadName((GLuint)nurbs->object_name);
							}
							/* work out if subobjects selected */
							selected_name_ranges=(struct Multi_range *)NULL;
							name_selected=GT_object_is_graphic_selected(object,
								nurbs->object_name,&selected_name_ranges);
							if ((name_selected&&context->draw_selected)||
								((!name_selected)&&(!context->draw_selected)))
							{
								return_code = draw_nurbsGL(nurbs);
							}
							nurbs=nurbs->ptrnext;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"makegtobject.  Missing nurbs");
						return_code=0;
					}
#if defined (OPENGL_API)
					if (picking_names)
					{
						glPopName();
					}
					/* restore the transform attribute group */
					glPopAttrib();
#endif /* defined (OPENGL_API) */
				} break;
				case g_USERDEF:
				{
#if defined (OPENGL_API)
					/* save transformation attributes state */
					glPushAttrib(GL_TRANSFORM_BIT);
					glEnable(GL_NORMALIZE);
#endif /* defined (OPENGL_API) */
					if (userdef = primitive_list1->gt_userdef.first)
					{
						if (userdef->render_function)
						{
							(userdef->render_function)(userdef->data);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"makegtobject.  Missing render function user defined object");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"makegtobject.  Missing userdef");
						return_code=0;
					}
#if defined (OPENGL_API)
					/* restore previous transformation attributes */
					glPopAttrib();
#endif /* defined (OPENGL_API) */
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,"makegtobject.  Invalid object type");
					return_code=0;
				} break;
			}
		}
		switch (object->coordinate_system)
		{
			case g_MODEL_COORDINATES:
			{
				/* Do nothing */
			} break;
			case g_NDC_COORDINATES:
			{
				glCallList(context->end_ndc_display_list);
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"makegtobject.  Missing object");
		return_code=0;
	}
/*???debug */
/*printf("leave makegtobject\n");*/
	LEAVE;

	return (return_code);
} /* makegtobject */
