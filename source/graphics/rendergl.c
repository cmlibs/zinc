/*******************************************************************************
FILE : rendergl.c

LAST MODIFIED : 7 July 2000

DESCRIPTION :
GL rendering calls - API specific.
???DB.  Should this be render.c ?
==============================================================================*/
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

/*
Global functions
----------------
*/

int draw_glyphsetGL(int number_of_points,Triple *point_list,Triple *axis1_list,
	Triple *axis2_list,Triple *axis3_list,struct GT_object *glyph,char **labels,
	int number_of_data_components,GTDATA *data,int *names,
	struct Graphical_material *material,struct Spectrum *spectrum,
	int draw_selected,int some_selected,struct Multi_range *selected_name_ranges)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
Draws graphics object <glyph> at <number_of_points> points given by the
positions in <point_list> and oriented and scaled by <axis1_list>, <axis2_list>
and <axis3_list>. If the glyph is part of a linked list through its nextobject
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
	GLfloat transformation[16],x,y,z;
	GTDATA *datum;
	int draw_all,i,*name,name_selected,return_code;
	struct GT_object *temp_glyph;
	struct Spectrum_render_data *render_data;
	Triple *point,*axis1,*axis2,*axis3;
#if defined (DEBUG)
	/*???debug*/
	int m,n;
#endif /* defined (DEBUG) */

	ENTER(draw_glyphsetGL);
	if (((0==number_of_points)||(0<number_of_points)&&point_list&&axis1_list&&
		axis2_list&&axis3_list)&&glyph)
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
#if defined (OPENGL_API)
			if ((!data)||(render_data=spectrum_start_renderGL
				(spectrum,material,number_of_data_components)))
			{
				draw_all = (!names) ||
					(draw_selected&&some_selected&&(!selected_name_ranges)) ||
					((!draw_selected)&&(!some_selected));
				point=point_list;
				axis1=axis1_list;
				axis2=axis2_list;
				axis3=axis3_list;
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
				if (0==strcmp(glyph->name,"point"))
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
								x=(*point)[0];
								y=(*point)[1];
								z=(*point)[2];
								if (names)
								{
									glLoadName((GLuint)(*name));
								}
								glBegin(GL_POINTS);
								glVertex3f(x,y,z);
								glEnd();
								if (labels)
								{
									glRasterPos3f(x,y,z);
									wrapperPrintText(*label);
								}
							}
							/* advance pointers */
							point++;
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
							x=(*point)[0];
							y=(*point)[1];
							z=(*point)[2];
							point++;
							glVertex3f(x,y,z);
						}
						glEnd();
					}
					/* restore previous lighting state */
					glPopAttrib();
				}
				else if (0==strcmp(glyph->name,"line"))
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
								x=(*point)[0];
								y=(*point)[1];
								z=(*point)[2];
								if (labels)
								{
									glRasterPos3f(x,y,z);
									wrapperPrintText(*label);
								}
								glBegin(GL_LINES);
								glVertex3f(x,y,z);
								x+=(*axis1)[0];
								y+=(*axis1)[1];
								z+=(*axis1)[2];
								glVertex3f(x,y,z);
								glEnd();
							}
							/* advance pointers */
							point++;
							axis1++;
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
							x=(*point)[0];
							y=(*point)[1];
							z=(*point)[2];
							glVertex3f(x,y,z);
							x+=(*axis1)[0];
							y+=(*axis1)[1];
							z+=(*axis1)[2];
							glVertex3f(x,y,z);
							point++;
							axis1++;
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
								x=(*point)[0];
								y=(*point)[1];
								z=(*point)[2];
								if (labels)
								{
									glRasterPos3f(x,y,z);
									wrapperPrintText(*label);
								}
								glBegin(GL_LINES);
								/* x-line */
								x=(*point)[0]-0.5*(*axis1)[0];
								y=(*point)[1]-0.5*(*axis1)[1];
								z=(*point)[2]-0.5*(*axis1)[2];
								glVertex3f(x,y,z);
								x+=(*axis1)[0];
								y+=(*axis1)[1];
								z+=(*axis1)[2];
								glVertex3f(x,y,z);
								/* y-line */
								x=(*point)[0]-0.5*(*axis2)[0];
								y=(*point)[1]-0.5*(*axis2)[1];
								z=(*point)[2]-0.5*(*axis2)[2];
								glVertex3f(x,y,z);
								x+=(*axis2)[0];
								y+=(*axis2)[1];
								z+=(*axis2)[2];
								glVertex3f(x,y,z);
								/* z-line */
								x=(*point)[0]-0.5*(*axis3)[0];
								y=(*point)[1]-0.5*(*axis3)[1];
								z=(*point)[2]-0.5*(*axis3)[2];
								glVertex3f(x,y,z);
								x+=(*axis3)[0];
								y+=(*axis3)[1];
								z+=(*axis3)[2];
								glVertex3f(x,y,z);
								glEnd();
							}
							/* advance pointers */
							point++;
							axis1++;
							axis2++;
							axis3++;
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
							/* x-line */
							x=(*point)[0]-0.5*(*axis1)[0];
							y=(*point)[1]-0.5*(*axis1)[1];
							z=(*point)[2]-0.5*(*axis1)[2];
							glVertex3f(x,y,z);
							x+=(*axis1)[0];
							y+=(*axis1)[1];
							z+=(*axis1)[2];
							glVertex3f(x,y,z);
							/* y-line */
							x=(*point)[0]-0.5*(*axis2)[0];
							y=(*point)[1]-0.5*(*axis2)[1];
							z=(*point)[2]-0.5*(*axis2)[2];
							glVertex3f(x,y,z);
							x+=(*axis2)[0];
							y+=(*axis2)[1];
							z+=(*axis2)[2];
							glVertex3f(x,y,z);
							/* z-line */
							x=(*point)[0]-0.5*(*axis3)[0];
							y=(*point)[1]-0.5*(*axis3)[1];
							z=(*point)[2]-0.5*(*axis3)[2];
							glVertex3f(x,y,z);
							x+=(*axis3)[0];
							y+=(*axis3)[1];
							z+=(*axis3)[2];
							glVertex3f(x,y,z);
							point++;
							axis1++;
							axis2++;
							axis3++;
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
					for (i=0;i<number_of_points;i++)
					{
						if (draw_all||((name_selected=Multi_range_is_value_in_range(
							selected_name_ranges,*name))&&draw_selected)||
							((!name_selected)&&(!draw_selected)))
						{
							if (names)
							{
								glLoadName((GLuint)(*name));
							}
							/* store the current modelview matrix */
							glPushMatrix();
							/* make transformation matrix for manipulating glyph */
							transformation[ 0] = (*axis1)[0];
							transformation[ 1] = (*axis1)[1];
							transformation[ 2] = (*axis1)[2];
							transformation[ 3] = 0.0;
							transformation[ 4] = (*axis2)[0];
							transformation[ 5] = (*axis2)[1];
							transformation[ 6] = (*axis2)[2];
							transformation[ 7] = 0.0;
							transformation[ 8] = (*axis3)[0];
							transformation[ 9] = (*axis3)[1];
							transformation[10] = (*axis3)[2];
							transformation[11] = 0.0;
							transformation[12] = (*point)[0];
							transformation[13] = (*point)[1];
							transformation[14] = (*point)[2];
							transformation[15] = 1.0;
							glMultMatrixf(transformation);
							/* set the spectrum for this datum, if any */
							if (data)
							{
								spectrum_renderGL_value(spectrum,material,render_data,datum);
							}
							/* call the glyph display lists of the linked-list of glyphs */
							for (temp_glyph=glyph;temp_glyph != NULL;
								temp_glyph=temp_glyph->nextobject)
							{
								glCallList(temp_glyph->display_list);
							}
							/* restore the original modelview matrix */
							glPopMatrix();
						}
						/* advance pointers */
						point++;
						axis1++;
						axis2++;
						axis3++;
						if (data)
						{
							datum+=number_of_data_components;
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
								((!name_selected)&&(!draw_selected)))
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
								datum+=number_of_data_components;
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
						datum+=number_of_data_components;
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
								datum+=number_of_data_components;
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
								datum+=number_of_data_components;
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
			if (text_string=text)
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
						datum+=number_of_data_components;
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
								datum+=number_of_data_components;
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
								datum+=number_of_data_components;
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
				datum+=number_of_data_components;
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
				datum+=number_of_data_components;
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
				datum+=number_of_data_components;
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

int draw_surfaceGL(Triple *surfpts, Triple *normalpoints,
	Triple *texturepoints, int npts1, int npts2, gtPolygonType polygon_type,
	int number_of_data_components, GTDATA *data, 
	struct Graphical_material *material, struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
==============================================================================*/
{
	GTDATA *data_1,*data_2;
	int i,j,return_code;
	Triple *surface_point_1,*surface_point_2, *normal_point_1, *normal_point_2,
		*texture_point_1, *texture_point_2;
	struct Spectrum_render_data *render_data;

	ENTER(draw_surfaceGL);
	/* checking arguments */
	if (surfpts&&(1<npts1)&&(1<npts2)&&((!data)||(render_data=
		spectrum_start_renderGL(spectrum,material,number_of_data_components))))
	{
#if defined (OPENGL_API)
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
						if (texturepoints)
						{
							glTexCoord2fv(texturepoints[i+npts1*j]);
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
						if (texturepoints)
						{
							glTexCoord2fv(texturepoints[i+npts1*j+1]);
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
				if (texturepoints)
				{
					texture_point_1=texturepoints;
					texture_point_2=texturepoints+npts1;				
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
					if (texturepoints)
					{
						glTexCoord2fv(*texture_point_1);
						texture_point_1++;
					}
					if (data)
					{
						spectrum_renderGL_value(spectrum,material,render_data,
							data_1);
						data_1+=number_of_data_components;
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
						if (texturepoints)
						{
							glTexCoord2fv(*texture_point_2);
							texture_point_2++;
						}
						if (data)
						{
							spectrum_renderGL_value(spectrum,material,render_data,
								data_2);
							data_2+=number_of_data_components;
						}
						glVertex3fv(*surface_point_2);
						surface_point_2++;
						if (normalpoints)
						{
							glNormal3fv(*normal_point_1);
							normal_point_1++;
						}
						if (texturepoints)
						{
							glTexCoord2fv(*texture_point_1);
							texture_point_1++;
						}
						if (data)
						{
							spectrum_renderGL_value(spectrum,material,render_data,
								data_1);
							data_1+=number_of_data_components;
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
	struct Graphical_material **iso_poly_material,struct Environment_map **iso_env_map,
	float *texturemap_coord,int *texturemap_index,int number_of_data_components,
	GTDATA *data, struct Graphical_material *default_material,struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 7 July

DESCRIPTION :
==============================================================================*/
{
	int i,ii,return_code;
	struct Graphical_material *last_material,*next_material;
	struct Spectrum_render_data *render_data;

	ENTER(draw_voltexGL);
	/* default return code */
	return_code=0;
	/* checking arguments */
	if (triangle_list&&vertex_list&&iso_env_map&&
		((!texturemap_coord)&&(!texturemap_index)||(texturemap_coord&&texturemap_index))
		&&(0<n_rep)&&(0<n_iso_polys)&&default_material)
	{
#if defined (OPENGL_API)
		if (!(iso_poly_material&&(last_material=iso_poly_material[0])))
		{
			last_material=default_material;
		}
		if (last_material)
		{
			execute_Graphical_material(last_material);
		}
		if ((!data)||(render_data=spectrum_start_renderGL(spectrum,last_material,
				number_of_data_components)))
		{
			for (ii=0;ii<n_rep;ii++)
			{
				for (i=0;i<n_iso_polys;i++)
				{
					next_material=default_material;
					/* if an environment map exists use it in preference to a material */
					if (iso_env_map[i*3])
					{
						if ((iso_env_map[i*3]->face_material)[texturemap_index[i*3]])
						{
							next_material=
								iso_env_map[i*3]->face_material[texturemap_index[i*3]];
						}
					}
					else
					{
						if (iso_poly_material)
						{
							next_material=iso_poly_material[i*3];
						}
					}
					if (!next_material)
					{
						next_material=default_material;
					}
					if (next_material != last_material)
					{
						execute_Graphical_material(next_material);
						last_material=next_material;
					}

					glBegin(GL_TRIANGLES);
					if (texturemap_coord)
					{
						glTexCoord2fv(&(texturemap_coord[3*(3*i+0)]));
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

					next_material=default_material;
					if (iso_env_map[i*3+2])
					{
						if (iso_env_map[i*3+2]->face_material[texturemap_index[i*3+2]])
						{
							next_material=
								iso_env_map[i*3+2]->face_material[texturemap_index[i*3+2]];
						}
					}
					else
					{
						if (iso_poly_material)
						{
							next_material=iso_poly_material[i*3+2];
						}
					}
					if (!next_material)
					{
						next_material=default_material;
					}
					if (next_material != last_material)
					{
						execute_Graphical_material(next_material);
						last_material=next_material;
					}

					if (texturemap_coord)
					{
						glTexCoord2fv(&(texturemap_coord[3*(3*i+2)]));
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
					if (iso_env_map[i*3+1])
					{
						if (iso_env_map[i*3+1]->face_material[texturemap_index[i*3+1]])
						{
							next_material=
								iso_env_map[i*3+1]->face_material[texturemap_index[i*3+1]];
						}
					}
					else
					{
						if (iso_poly_material)
						{
							next_material=iso_poly_material[i*3+1];
						}
					}
					if (!next_material)
					{
						next_material=default_material;
					}
					if (next_material != last_material)
					{
						execute_Graphical_material(next_material);
						last_material=next_material;
					}

					if (texturemap_coord)
					{
						glTexCoord2fv(&(texturemap_coord[3*(3*i+1)]));
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
