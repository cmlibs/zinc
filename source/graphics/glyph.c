/*******************************************************************************
FILE : glyph.c

LAST MODIFIED : 10 June 2004

DESCRIPTION :
Glyphs are GT_objects which contain simple geometric shapes such as
cylinders, arrows and axes which are (or should) fit into a unit (1x1x1) cube,
with the major axes of the glyph aligned with the x, y and z axes. 
The logical centre of each glyph should be at (0,0,0). This should be
interpreted as follows:
- if the glyph is symmetrical along any axis, its coordinates in that
direction should vary from -0.5 to +0.5;
- if the glyph involves any sort of arrow that is unsymmetric in its direction
(ie. is single-headed), (0,0,0) should be at the base of the arrow.
- axes should therefore be centred at (0,0,0) and extend to 1 in each axis
direction. Axis titles "x", "y" and "z" may be outside the unit cube.

Glyphs are referenced by GT_glyph_set objects. Glyphs themselves should not
reference graphical materials or spectrums.
==============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "general/debug.h"
#include "graphics/glyph.h"
#include "graphics/graphics_object.h"
#include "user_interface/message.h"

/*
Module functions
----------------
*/

static int construct_tube(int number_of_segments_around,float x1,float r1,
	float x2,float r2,float cy,float cz,int primary_axis,Triple *vertex_list,
	Triple *normal_list)
/*******************************************************************************
LAST MODIFIED : 17 July 1998

DESCRIPTION :
Adds vertices and normals for a tube/cone/anulus/disc stretching from position
x1 with radius r1 to position x2 with radius r2. The axis of the cylinder is
parallel with the x axis and its centre is at cy,cz. If <primary_axis> is 1, the
above is the case, if its value is 2, x->y, y->z and z->x, and a further
permutation if <primary_axis> is 3. Values other than 1, 2 or 3 are taken as 1.
The vertices and normals are added to create a single quadrilateral strip
suitable for using with GT_surfaces of type g_SH_DISCONTINUOUS_STRIP.
==============================================================================*/
{
	float longitudinal_normal,normal_angle,radial_normal,theta,y,z;
	int j,ix,iy,iz,return_code;
	Triple *normal,*vertex;

	ENTER(construct_tube);
	if ((2<number_of_segments_around)&&((x1 != x2)||(r1 != r2))&&
		vertex_list&&normal_list)
	{
		return_code=1;
		switch (primary_axis)
		{
			case 2:
			{
				ix=1;
				iy=2;
				iz=0;
			} break;
			case 3:
			{
				ix=2;
				iy=0;
				iz=1;
			} break;
			default:
			{
				ix=0;
				iy=1;
				iz=2;
			} break;
		}
		vertex=vertex_list;
		normal=normal_list;
		/* get radial and longitudinal components of surface normals */
		normal_angle=atan2(r2-r1,x2-x1);
		radial_normal=cos(normal_angle);
		longitudinal_normal=-sin(normal_angle);
		for (j=0;j <= number_of_segments_around;j++)
		{
			theta=2.0*PI*(float)j/(float)number_of_segments_around;
			y = sin(theta);
			z = cos(theta);
			(*vertex)[ix] = x1;
			(*vertex)[iy] = cy+r1*y;
			(*vertex)[iz] = cz+r1*z;
			vertex++;
			(*vertex)[ix] = x2;
			(*vertex)[iy] = cy+r2*y;
			(*vertex)[iz] = cz+r2*z;
			vertex++;
			y *= radial_normal;
			z *= radial_normal;
			(*normal)[ix] = longitudinal_normal;
			(*normal)[iy] = y;
			(*normal)[iz] = z;
			normal++;
			(*normal)[ix] = longitudinal_normal;
			(*normal)[iy] = y;
			(*normal)[iz] = z;
			normal++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"construct_tube.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* construct_tube */

/*
Global functions
----------------
*/

struct GT_object *make_glyph_arrow_line(char *name,float head_length,
	float half_head_width)
/*******************************************************************************
LAST MODIFIED : 3 August 1999

DESCRIPTION :
Creates a graphics object named <name> consisting of a line from <0,0,0> to
<1,0,0> with 4 arrow head ticks <head_length> long and <half_head_width> out
from the shaft.
==============================================================================*/
{
	int j;
	struct GT_object *glyph;
	struct GT_polyline *polyline;
	Triple *points,*vertex;

	ENTER(make_glyph_arrow_line);
	if (name)
	{
		polyline=(struct GT_polyline *)NULL;
		if (ALLOCATE(points,Triple,10))
		{
			vertex=points;
			/* most coordinates are 0.0, so clear them all to that */
			for (j=0;j<10;j++)
			{
				(*vertex)[0]=0.0;
				(*vertex)[1]=0.0;
				(*vertex)[2]=0.0;
				vertex++;
			}
			/* x-axis */
			points[ 1][0]=1.0;
			points[ 2][0]=1.0;
			points[ 3][0]=1.0-head_length;
			points[ 3][1]=half_head_width;
			points[ 4][0]=1.0;
			points[ 5][0]=1.0-head_length;
			points[ 5][2]=half_head_width;
			points[ 6][0]=1.0;
			points[ 7][0]=1.0-head_length;
			points[ 7][1]=-half_head_width;
			points[ 8][0]=1.0;
			points[ 9][0]=1.0-head_length;
			points[ 9][2]=-half_head_width;
			if (!(polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,/*line_width=default*/0,
				5,points,/*normalpoints*/(Triple *)NULL,g_NO_DATA,(GTDATA *)NULL)))
			{
				DEALLOCATE(points);
			}
		}
		if (polyline)
		{
			if (glyph=CREATE(GT_object)(name,g_POLYLINE,
				(struct Graphical_material *)NULL))
			{
				if (!GT_OBJECT_ADD(GT_polyline)(glyph,/*time*/0.0,polyline))
				{
					DESTROY(GT_object)(&glyph);
				}
			}
			if (!glyph)
			{
				DESTROY(GT_polyline)(&polyline);
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"make_glyph_arrow_line.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_glyph_arrow_line.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_arrow_line */

struct GT_object *make_glyph_arrow_solid(char *name, int primary_axis,
	int number_of_segments_around,float shaft_length,float shaft_radius,
	float cone_radius)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Creates a graphics object named <name> resembling an arrow made from a cone on
a cylinder. The base of the arrow is at (0,0,0) while its head lies at (1,0,0).
The radius of the cone is <cone_radius>. The cylinder is <shaft_length> long
with its radius given by <shaft_radius>. The ends of the arrow and the cone
are both closed.  Primary axis is either 1,2 or 3 and indicates the direction
the arrow points in.
==============================================================================*/
{
	float r1,r2,x1,x2;
	int i;
	struct GT_object *glyph;
	struct GT_surface *surface;
	Triple *points, *normalpoints;

	ENTER(make_glyph_arrow_solid);
	if (name&&(2<number_of_segments_around)&&(0<shaft_radius)&&(1>shaft_radius)&&
		(0<shaft_length)&&(1>shaft_length))
	{
		if (glyph=CREATE(GT_object)(name,g_SURFACE,
			(struct Graphical_material *)NULL))
		{
			for (i=0;(i<4)&&glyph;i++)
			{
				if (ALLOCATE(points,Triple,2*(number_of_segments_around+1))&&
					ALLOCATE(normalpoints,Triple,2*(number_of_segments_around+1)))
				{
					switch (i)
					{
						case 0:
						{
							/* base of shaft */
							x1=0.0;
							r1=0.0;
							x2=0.0;
							r2=shaft_radius;
						} break;
						case 1:
						{
							/* shaft */
							x1=0.0;
							r1=shaft_radius;
							x2=shaft_length;
							r2=shaft_radius;
						} break;
						case 2:
						{
							/* base of head */
							x1=shaft_length;
							r1=shaft_radius;
							x2=shaft_length;
							r2=cone_radius;
						} break;
						case 3:
						{
							/* head */
							x1=shaft_length;
							r1=cone_radius;
							x2=1.0;
							r2=0.0;
						} break;
					}
					if (!construct_tube(number_of_segments_around,x1,r1,x2,r2,0.0,0.0,
						primary_axis,points,normalpoints))
					{
						DEALLOCATE(points);
						DEALLOCATE(normalpoints);
					}
				}
				if (points&&(surface=CREATE(GT_surface)(g_SHADED_TEXMAP,
					g_QUADRILATERAL,2,number_of_segments_around+1,points,
					normalpoints,/*texturepoints*/(Triple *)NULL,
					/*tangentpoints*/(Triple *)NULL,g_NO_DATA,(GTDATA *)NULL)))
				{
					if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
					{
						DESTROY(GT_surface)(&surface);
						DESTROY(GT_object)(&glyph);
					}
				}
				else
				{
					DEALLOCATE(points);
					DEALLOCATE(normalpoints);
					DESTROY(GT_object)(&glyph);
				}
			}
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"make_glyph_arrow_solid.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_glyph_arrow_solid.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_arrow_solid */

struct GT_object *make_glyph_axes(char *name, int make_solid, float head_length,
	float half_head_width,char **labels, float label_offset)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Creates a graphics object named <name> consisting of three axis arrows heading
from <0,0,0> to 1 in each of their directions. The arrows are made up of lines,
with a 4-way arrow head so it looks normal from the other two axes. If <labels>
is specified then it is assumed to point to an array of 3 strings which will
be used to label each arrow and are attached to it so
that the two objects are displayed and destroyed together. The labels are
located on the respective axes <label_offset> past 1.0.
The length and width of the arrow heads are specified by the final parameters.
==============================================================================*/
{
	char *glyph_name,**text;
	int j;
	struct Colour colour;
	struct Graphical_material *material;
	struct GT_object *glyph,*arrow2,*arrow3,*labels_object;
	struct GT_pointset *pointset;
	struct GT_polyline *polyline;
	Triple *points,*vertex;

	ENTER(make_glyph_axes);
	if (name)
	{
		if (make_solid)
		{
			if (ALLOCATE(glyph_name, char, strlen(name) + 8))
			{
				glyph = make_glyph_arrow_solid(name, /*primary_axis*/1,
					/*number_of_segments_around*/12, /*shaft_length*/2./3.,
					/*shaft_radius*/1./20., /*cone_radius*/1./8.);
				material = CREATE(Graphical_material)("red");
				colour.red = 1;
				colour.green = 0;
				colour.blue = 0;
				Graphical_material_set_diffuse(material, &colour);
				set_GT_object_default_material(glyph, material);

				sprintf(glyph_name, "%s_arrow2", name);
				arrow2 = make_glyph_arrow_solid(glyph_name, /*primary_axis*/2,
					/*number_of_segments_around*/12, /*shaft_length*/2./3.,
					/*shaft_radius*/1./20., /*cone_radius*/1./8.);
				material = CREATE(Graphical_material)("green");
				colour.red = 0;
				colour.green = 1;
				colour.blue = 0;
				Graphical_material_set_diffuse(material, &colour);
				set_GT_object_default_material(arrow2, material);
				glyph->nextobject=ACCESS(GT_object)(arrow2);

				sprintf(glyph_name, "%s_arrow3", name);
				arrow3 = make_glyph_arrow_solid(glyph_name, /*primary_axis*/3,
					/*number_of_segments_around*/12, /*shaft_length*/2./3.,
					/*shaft_radius*/1./20., /*cone_radius*/1./8.);
				material = CREATE(Graphical_material)("blue");
				colour.red = 0;
				colour.green = 0;
				colour.blue = 1;
				Graphical_material_set_diffuse(material, &colour);
				set_GT_object_default_material(arrow3, material);
				arrow2->nextobject=ACCESS(GT_object)(arrow3);
				
				DEALLOCATE(glyph_name);
			}
		}
		else
		{
			polyline=(struct GT_polyline *)NULL;
			if (ALLOCATE(points,Triple,30))
			{
				vertex=points;
				/* most coordinates are 0.0, so clear them all to that */
				for (j=0;j<30;j++)
				{
					(*vertex)[0]=0.0;
					(*vertex)[1]=0.0;
					(*vertex)[2]=0.0;
					vertex++;
				}
				/* x-axis */
				points[ 1][0]=1.0;
				points[ 2][0]=1.0;
				points[ 3][0]=1.0-head_length;
				points[ 3][1]=half_head_width;
				points[ 4][0]=1.0;
				points[ 5][0]=1.0-head_length;
				points[ 5][2]=half_head_width;
				points[ 6][0]=1.0;
				points[ 7][0]=1.0-head_length;
				points[ 7][1]=-half_head_width;
				points[ 8][0]=1.0;
				points[ 9][0]=1.0-head_length;
				points[ 9][2]=-half_head_width;
				/* y-axis */
				points[11][1]=1.0;
				points[12][1]=1.0;
				points[13][1]=1.0-head_length;
				points[13][2]=half_head_width;
				points[14][1]=1.0;
				points[15][1]=1.0-head_length;
				points[15][0]=half_head_width;
				points[16][1]=1.0;
				points[17][1]=1.0-head_length;
				points[17][2]=-half_head_width;
				points[18][1]=1.0;
				points[19][1]=1.0-head_length;
				points[19][0]=-half_head_width;
				/* z-axis */
				points[21][2]=1.0;
				points[22][2]=1.0;
				points[23][2]=1.0-head_length;
				points[23][0]=half_head_width;
				points[24][2]=1.0;
				points[25][2]=1.0-head_length;
				points[25][1]=half_head_width;
				points[26][2]=1.0;
				points[27][2]=1.0-head_length;
				points[27][0]=-half_head_width;
				points[28][2]=1.0;
				points[29][2]=1.0-head_length;
				points[29][1]=-half_head_width;
				if (polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,/*line_width=default*/0,
						15,points,/*normalpoints*/(Triple *)NULL,g_NO_DATA,(GTDATA *)NULL))
				{
					if (glyph=CREATE(GT_object)(name,g_POLYLINE,
							(struct Graphical_material *)NULL))
					{
						GT_OBJECT_ADD(GT_polyline)(glyph,/*time*/0.0,polyline);
					}
				}
				else
				{
					DEALLOCATE(points);
				}
			}
		}
		if (glyph && labels)
		{
			pointset=(struct GT_pointset *)NULL;
			if (ALLOCATE(points,Triple,3)&&
				ALLOCATE(text,char *,3)&&
				ALLOCATE(text[0],char,strlen(labels[0]) + 1)&&
				ALLOCATE(text[1],char,strlen(labels[1]) + 1)&&
				ALLOCATE(text[2],char,strlen(labels[2]) + 1)&&
				ALLOCATE(glyph_name,char,strlen(name)+8))
			{
				sprintf(glyph_name,"%s_labels",name);
				points[0][0]=1.0+label_offset;
				points[0][1]=0.0;
				points[0][2]=0.0;
				strcpy(text[0],labels[0]);
				points[1][0]=0.0;
				points[1][1]=1.0+label_offset;
				points[1][2]=0.0;
				strcpy(text[1],labels[1]);
				points[2][0]=0.0;
				points[2][1]=0.0;
				points[2][2]=1.0+label_offset;
				strcpy(text[2],labels[2]);
				if (pointset=CREATE(GT_pointset)(3,points,text,g_NO_MARKER,0.0,
					g_NO_DATA,(GTDATA *)NULL,(int *)NULL))
				{
					if (labels_object=CREATE(GT_object)(glyph_name,g_POINTSET,
						(struct Graphical_material *)NULL))
					{
						GT_OBJECT_ADD(GT_pointset)(labels_object,/*time*/0.0,pointset);
						glyph->nextobject=ACCESS(GT_object)(labels_object);
					}
				}
				else
				{
					DEALLOCATE(text[0]);
					DEALLOCATE(text[1]);
					DEALLOCATE(text[2]);
					DEALLOCATE(text);
					DEALLOCATE(points);
				}
				DEALLOCATE(glyph_name);
			}
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,"make_glyph_axes.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_axes.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_axes */

struct GT_object *make_glyph_cone(char *name,int number_of_segments_around)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Creates a graphics object named <name> resembling a cone with the given
<number_of_segments_around>. The base of the cone is at <0,0,0> while its head
lies at <1,0,0>. The radius of the cone is 0.5 at its base.
==============================================================================*/
{
	struct GT_object *glyph;
	struct GT_surface *surface;
	Triple *points,*normalpoints;

	ENTER(make_glyph_cone);
	if (name&&(2<number_of_segments_around))
	{
		surface=(struct GT_surface *)NULL;
		if (ALLOCATE(points,Triple,2*(number_of_segments_around+1))&&
			ALLOCATE(normalpoints,Triple,2*(number_of_segments_around+1)))
		{
			construct_tube(number_of_segments_around, 0.0, 0.5, 1.0, 0.0, 0.0, 0.0, 1,
				points,normalpoints);
			if (!(surface=CREATE(GT_surface)(g_SHADED_TEXMAP,g_QUADRILATERAL,2,
				number_of_segments_around+1,points,normalpoints,
				/*texturepoints*/(Triple *)NULL,/*tangentpoints*/(Triple *)NULL,
				g_NO_DATA,(GTDATA *)NULL)))
			{
				DEALLOCATE(points);
				DEALLOCATE(normalpoints);
			}
		}
		if (surface)
		{
			if (glyph=CREATE(GT_object)(name,g_SURFACE,
				(struct Graphical_material *)NULL))
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,"make_glyph_cone.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_cone.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_cone */

struct GT_object *make_glyph_cone_solid(char *name,int number_of_segments_around)
/*******************************************************************************
LAST MODIFIED : 20 January 2004

DESCRIPTION :
Creates a graphics object named <name> resembling a cone with the given
<number_of_segments_around>. The base of the cone is at <0,0,0> while its head
lies at <1,0,0>. The radius of the cone is 0.5 at its base.  This cone has a
solid base.
==============================================================================*/
{
	struct GT_object *glyph;
	struct GT_surface *surface;
	Triple *points,*normalpoints;

	ENTER(make_glyph_cone_solid);
	if (name&&(2<number_of_segments_around))
	{
		if (glyph=CREATE(GT_object)(name,g_SURFACE,
				(struct Graphical_material *)NULL))
		{
			surface=(struct GT_surface *)NULL;
			if (ALLOCATE(points,Triple,2*(number_of_segments_around+1))&&
				ALLOCATE(normalpoints,Triple,2*(number_of_segments_around+1)))
			{
				construct_tube(number_of_segments_around, 0.0, 0.5, 1.0, 0.0, 0.0, 0.0, 1,
					points,normalpoints);
				if (!(surface=CREATE(GT_surface)(g_SHADED_TEXMAP,g_QUADRILATERAL,2,
							number_of_segments_around+1,points,normalpoints,
							/*texturepoints*/(Triple *)NULL,/*tangentpoints*/(Triple *)NULL,
							g_NO_DATA,(GTDATA *)NULL)))
				{
					DEALLOCATE(points);
					DEALLOCATE(normalpoints);
				}
			}
			if (surface)
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			
			}
			else
			{
				glyph=(struct GT_object *)NULL;
			}
			if (ALLOCATE(points,Triple,2*(number_of_segments_around+1))&&
				ALLOCATE(normalpoints,Triple,2*(number_of_segments_around+1)))
			{
				construct_tube(number_of_segments_around, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 1,
					points,normalpoints);
				if (!(surface=CREATE(GT_surface)(g_SHADED_TEXMAP,g_QUADRILATERAL,2,
							number_of_segments_around+1,points,normalpoints,
							/*texturepoints*/(Triple *)NULL,/*tangentpoints*/(Triple *)NULL,
							g_NO_DATA,(GTDATA *)NULL)))
				{
					DEALLOCATE(points);
					DEALLOCATE(normalpoints);
				}
			}
			if (surface)
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			
			}
			else
			{
				DESTROY(GT_object)(&glyph);
			}
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,"make_glyph_cone_solid.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_cone_solid.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_cone_solid */

struct GT_object *make_glyph_cross(char *name)
/*******************************************************************************
LAST MODIFIED : 16 July 1999

DESCRIPTION :
Creates a graphics object named <name> consisting of a 3 lines:
from <-0.5,0,0> to <+0.5,0,0>
from <0,-0.5,0> to <0,+0.5,0>
from <0,0,-0.5> to <0,0,+0.5>
==============================================================================*/
{
	struct GT_object *glyph;
	struct GT_polyline *polyline;
	Triple *points;

	ENTER(make_glyph_cross);
	if (name)
	{
		polyline=(struct GT_polyline *)NULL;
		if (ALLOCATE(points,Triple,6))
		{
			/* x-line */
			points[0][0]=-0.5;
			points[0][1]=0.0;
			points[0][2]=0.0;
			points[1][0]=+0.5;
			points[1][1]=0.0;
			points[1][2]=0.0;
			/* y-line */
			points[2][0]=0.0;
			points[2][1]=-0.5;
			points[2][2]=0.0;
			points[3][0]=0.0;
			points[3][1]=+0.5;
			points[3][2]=0.0;
			/* z-line */
			points[4][0]=0.0;
			points[4][1]=0.0;
			points[4][2]=-0.5;
			points[5][0]=0.0;
			points[5][1]=0.0;
			points[5][2]=+0.5;
			if (!(polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,/*line_width=default*/0,
				3,points,/*normalpoints*/(Triple *)NULL,g_NO_DATA,(GTDATA *)NULL)))
			{
				DEALLOCATE(points);
			}
		}
		if (polyline)
		{
			if (glyph=CREATE(GT_object)(name,g_POLYLINE,
				(struct Graphical_material *)NULL))
			{
				if (!GT_OBJECT_ADD(GT_polyline)(glyph,/*time*/0.0,polyline))
				{
					DESTROY(GT_object)(&glyph);
				}
			}
			if (!glyph)
			{
				DESTROY(GT_polyline)(&polyline);
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,"make_glyph_cross.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_cross.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_cross */

struct GT_object *make_glyph_cube_solid(char *name)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Creates a graphics object named <name> consisting of a unit-sized GT_surface
cube centred at <0,0,0>.
==============================================================================*/
{
	float factor;
	int a, b, c, i;
	struct GT_object *glyph;
	struct GT_surface *surface;
	Triple *point, *points, *normalpoint, *normalpoints;

	ENTER(make_glyph_cube_solid);
	if (name)
	{
		surface=(struct GT_surface *)NULL;
		if (ALLOCATE(points,Triple,24)&&
			ALLOCATE(normalpoints,Triple,24))
		{
			point = points;
			normalpoint = normalpoints;
			/* all coordinates are +0.5 or -0.5, so clear them all to former */
			for (i = 0; i < 6; i++)
			{
				a = i / 2;
				if ((2*a) == i)
				{
					factor = -1.0;
				}
				else
				{
					factor = 1.0;
				}
				b = (a + 1) % 3;
				c = (a + 2) % 3;
				/* vertices */
				(*point)[a] = 0.5*factor;
				(*point)[b] = 0.5*factor;
				(*point)[c] = 0.5;
				point++;
				(*point)[a] = 0.5*factor;
				(*point)[b] = -0.5*factor;
				(*point)[c] = 0.5;
				point++;
				(*point)[a] = 0.5*factor;
				(*point)[b] = -0.5*factor;
				(*point)[c] = -0.5;
				point++;
				(*point)[a] = 0.5*factor;
				(*point)[b] = 0.5*factor;
				(*point)[c] = -0.5;
				point++;
				/* normals */
				(*normalpoint)[a] = factor;
				(*normalpoint)[b] = 0.0;
				(*normalpoint)[c] = 0.0;
				normalpoint++;
				(*normalpoint)[a] = factor;
				(*normalpoint)[b] = 0.0;
				(*normalpoint)[c] = 0.0;
				normalpoint++;
				(*normalpoint)[a] = factor;
				(*normalpoint)[b] = 0.0;
				(*normalpoint)[c] = 0.0;
				normalpoint++;
				(*normalpoint)[a] = factor;
				(*normalpoint)[b] = 0.0;
				(*normalpoint)[c] = 0.0;
				normalpoint++;
			}
			if (!(surface=CREATE(GT_surface)(g_SH_DISCONTINUOUS,g_QUADRILATERAL,6,
				4,points,normalpoints,/*texturepoints*/(Triple *)NULL,
				/*tangentpoints*/(Triple *)NULL,g_NO_DATA,(GTDATA *)NULL)))
			{
				DEALLOCATE(points);
				DEALLOCATE(normalpoints);
			}
		}
		if (surface)
		{
			if (glyph=CREATE(GT_object)(name,g_SURFACE,
				(struct Graphical_material *)NULL))
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"make_glyph_cube_solid.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_cube_solid.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_cube_solid */

struct GT_object *make_glyph_cube_wireframe(char *name)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Creates a graphics object named <name> consisting of lines marking a unit-sized
wireframe cube centred at <0,0,0>.
==============================================================================*/
{
	int a, b, c, i;
	struct GT_object *glyph;
	struct GT_polyline *polyline;
	Triple *points, *vertex;

	ENTER(make_glyph_cube_wireframe);
	if (name)
	{
		polyline = (struct GT_polyline *)NULL;
		if (ALLOCATE(points, Triple, 24))
		{
			vertex=points;
			/* all coordinates are +0.5 or -0.5, so clear them all to former */
			for (a = 0; a < 3; a++)
			{
				b = (a + 1) % 3;
				c = (a + 2) % 3;
				for (i = 0; i < 8; i++)
				{
					if (0 == (i % 2))
					{
						(*vertex)[a] = -0.5;
					}
					else
					{
						(*vertex)[a] = 0.5;
					}
					if (2 > (i % 4))
					{
						(*vertex)[b] = -0.5;
					}
					else
					{
						(*vertex)[b] = 0.5;
					}
					if (4 > i)
					{
						(*vertex)[c] = -0.5;
					}
					else
					{
						(*vertex)[c] = 0.5;
					}
					vertex++;
				}
			}
			if (!(polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,/*line_width=default*/0,
				12,points,/*normalpoints*/(Triple *)NULL,g_NO_DATA,(GTDATA *)NULL)))
			{
				DEALLOCATE(points);
			}
		}
		if (polyline)
		{
			if (glyph=CREATE(GT_object)(name,g_POLYLINE,
				(struct Graphical_material *)NULL))
			{
				if (!GT_OBJECT_ADD(GT_polyline)(glyph,/*time*/0.0,polyline))
				{
					DESTROY(GT_object)(&glyph);
				}
			}
			if (!glyph)
			{
				DESTROY(GT_polyline)(&polyline);
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"make_glyph_cube_wireframe.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_glyph_cube_wireframe.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_cube_wireframe */

struct GT_object *make_glyph_cylinder(char *name,int number_of_segments_around)
/*******************************************************************************
LAST MODIFIED : 14 July 1999

DESCRIPTION :
Creates a graphics object named <name> resembling a cylinder with the given
<number_of_segments_around>. The cylinder is centred at (0.5,0,0) and its axis
lies in the direction <1,0,0>. It fits into the unit cube spanning from
(0,-0.5,-0.5) to (0,+0.5,+0.5).
==============================================================================*/
{
	struct GT_object *glyph;
	struct GT_surface *surface;
	Triple *points,*normalpoints;

	ENTER(make_glyph_cylinder);
	if (name&&(2<number_of_segments_around))
	{
		surface=(struct GT_surface *)NULL;
		if (ALLOCATE(points,Triple,2*(number_of_segments_around+1))&&
			ALLOCATE(normalpoints,Triple,2*(number_of_segments_around+1)))
		{
			construct_tube(number_of_segments_around,0.0,0.5,1.0,0.5,0.0,0.0,1,
				points,normalpoints);
			if (!(surface=CREATE(GT_surface)(g_SHADED_TEXMAP,g_QUADRILATERAL,2,
				number_of_segments_around+1,points,normalpoints,
				/*tangentpoints*/(Triple *)NULL,/*texturepoints*/(Triple *)NULL,
				g_NO_DATA,(GTDATA *)NULL)))
			{
				DEALLOCATE(points);
				DEALLOCATE(normalpoints);
			}
		}
		if (surface)
		{
			if (glyph=CREATE(GT_object)(name,g_SURFACE,
				(struct Graphical_material *)NULL))
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"make_glyph_cylinder.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_cylinder.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_cylinder */

struct GT_object *make_glyph_cylinder_solid(char *name,int number_of_segments_around)
/*******************************************************************************
LAST MODIFIED : 20 January 2004

DESCRIPTION :
Creates a graphics object named <name> resembling a cylinder with the given
<number_of_segments_around>. The cylinder is centred at (0.5,0,0) and its axis
lies in the direction <1,0,0>. It fits into the unit cube spanning from
(0,-0.5,-0.5) to (0,+0.5,+0.5).  This cylinder has its ends covered over.
==============================================================================*/
{
	struct GT_object *glyph;
	struct GT_surface *surface;
	Triple *points,*normalpoints;

	ENTER(make_glyph_cylinder);
	if (name&&(2<number_of_segments_around))
	{
		if (glyph=CREATE(GT_object)(name,g_SURFACE,
			(struct Graphical_material *)NULL))
		{
			surface=(struct GT_surface *)NULL;
			if (ALLOCATE(points,Triple,2*(number_of_segments_around+1))&&
				ALLOCATE(normalpoints,Triple,2*(number_of_segments_around+1)))
			{
				construct_tube(number_of_segments_around,0.0,0.5,1.0,0.5,0.0,0.0,1,
					points,normalpoints);
				if (!(surface=CREATE(GT_surface)(g_SHADED_TEXMAP,g_QUADRILATERAL,2,
							number_of_segments_around+1,points,normalpoints,
							/*tangentpoints*/(Triple *)NULL,/*texturepoints*/(Triple *)NULL,
							g_NO_DATA,(GTDATA *)NULL)))
				{
					DEALLOCATE(points);
					DEALLOCATE(normalpoints);
				}
			}
			if (surface)
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			}
			else
			{
				DESTROY(GT_object)(&glyph);
			}
			/* Cover over the ends */
			if (ALLOCATE(points,Triple,2*(number_of_segments_around+1))&&
				ALLOCATE(normalpoints,Triple,2*(number_of_segments_around+1)))
			{
				construct_tube(number_of_segments_around,0.0,0.0,0.0,0.5,0.0,0.0,1,
					points,normalpoints);
				if (!(surface=CREATE(GT_surface)(g_SHADED_TEXMAP,g_QUADRILATERAL,2,
							number_of_segments_around+1,points,normalpoints,
							/*tangentpoints*/(Triple *)NULL,/*texturepoints*/(Triple *)NULL,
							g_NO_DATA,(GTDATA *)NULL)))
				{
					DEALLOCATE(points);
					DEALLOCATE(normalpoints);
				}
			}
			if (surface)
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			}
			else
			{
				DESTROY(GT_object)(&glyph);
			}
			if (ALLOCATE(points,Triple,2*(number_of_segments_around+1))&&
				ALLOCATE(normalpoints,Triple,2*(number_of_segments_around+1)))
			{
				construct_tube(number_of_segments_around,1.0,0.0,1.0,0.5,0.0,0.0,1,
					points,normalpoints);
				if (!(surface=CREATE(GT_surface)(g_SHADED_TEXMAP,g_QUADRILATERAL,2,
							number_of_segments_around+1,points,normalpoints,
							/*tangentpoints*/(Triple *)NULL,/*texturepoints*/(Triple *)NULL,
							g_NO_DATA,(GTDATA *)NULL)))
				{
					DEALLOCATE(points);
					DEALLOCATE(normalpoints);
				}
			}
			if (surface)
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			}
			else
			{
				DESTROY(GT_object)(&glyph);
			}
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"make_glyph_cylinder.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_cylinder.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_cylinder */

struct GT_object *make_glyph_line(char *name)
/*******************************************************************************
LAST MODIFIED : 14 July 1999

DESCRIPTION :
Creates a graphics object named <name> consisting of a line from <0,0,0> to
<1,0,0>.
==============================================================================*/
{
	struct GT_object *glyph;
	struct GT_polyline *polyline;
	Triple *points;

	ENTER(make_glyph_line);
	if (name)
	{
		polyline=(struct GT_polyline *)NULL;
		if (ALLOCATE(points,Triple,2))
		{
			points[0][0]=0.0;
			points[0][1]=0.0;
			points[0][2]=0.0;
			points[1][0]=1.0;
			points[1][1]=0.0;
			points[1][2]=0.0;
			if (!(polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,/*line_width=default*/0,
				1,points,/*normalpoints*/(Triple *)NULL,g_NO_DATA,(GTDATA *)NULL)))
			{
				DEALLOCATE(points);
			}
		}
		if (polyline)
		{
			if (glyph=CREATE(GT_object)(name,g_POLYLINE,
				(struct Graphical_material *)NULL))
			{
				if (!GT_OBJECT_ADD(GT_polyline)(glyph,/*time*/0.0,polyline))
				{
					DESTROY(GT_object)(&glyph);
				}
			}
			if (!glyph)
			{
				DESTROY(GT_polyline)(&polyline);
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,"make_glyph_line.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_line.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_line */

struct GT_object *make_glyph_mirror(char *name, struct GT_object *mirror_glyph)
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Makes a glyph with the given <name> that automatically mirrors the given
<mirror_glyph>.
==============================================================================*/
{
	struct GT_object *glyph;

	ENTER(make_glyph_mirror);
	if (name && mirror_glyph)
	{
		/*???temporary. Use dummy GT_object until we have struct Glyph */
		if (glyph = CREATE(GT_object)(name, g_SURFACE,
			(struct Graphical_material *)NULL))
		{
			GT_object_set_glyph_mirror_mode(glyph, 1);
			glyph->nextobject = ACCESS(GT_object)(mirror_glyph);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "make_glyph_mirror.  Invalid argument(s)");
		glyph = (struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_mirror */

struct GT_object *make_glyph_point(char *name,gtMarkerType marker_type,
	float marker_size)
/*******************************************************************************
LAST MODIFIED : 1 December 1998

DESCRIPTION :
Creates a graphics object named <name> consisting of a single point at <0,0,0>.
The point will be drawn with the given <marker_type> and <marker_size>.
==============================================================================*/
{
	struct GT_object *glyph;
	struct GT_pointset *pointset;
	Triple *points;

	ENTER(make_glyph_point);
	if (name)
	{
		pointset=(struct GT_pointset *)NULL;
		if (ALLOCATE(points,Triple,1))
		{
			(*points)[0]=0.0;
			(*points)[1]=0.0;
			(*points)[2]=0.0;
			if (!(pointset=CREATE(GT_pointset)(1,points,(char **)NULL,marker_type,
				marker_size,g_NO_DATA,(GTDATA *)NULL,(int *)NULL)))
			{
				DEALLOCATE(points);
			}
		}
		if (pointset)
		{
			if (glyph=CREATE(GT_object)(name,g_POINTSET,
				(struct Graphical_material *)NULL))
			{
				if (!GT_OBJECT_ADD(GT_pointset)(glyph,/*time*/0.0,pointset))
				{
					DESTROY(GT_object)(&glyph);
				}
			}
			if (!glyph)
			{
				DESTROY(GT_pointset)(&pointset);
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,"make_glyph_point.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_point.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_point */

struct GT_object *make_glyph_sheet(char *name)
/*******************************************************************************
LAST MODIFIED : 5 May 1999

DESCRIPTION :
Creates a graphics object named <name> resembling a square sheet spanning from
coordinate <-0.5,-0.5,0> to <0.5,0.5,0>.
==============================================================================*/
{
	struct GT_object *glyph;
	struct GT_surface *surface;
	Triple *point,*points,*normalpoints;

	ENTER(make_glyph_sheet);
	if (name)
	{
		surface=(struct GT_surface *)NULL;
		if (ALLOCATE(points,Triple,4)&&
			ALLOCATE(normalpoints,Triple,4))
		{
			point = points;
			/* vertices */
			(*point)[0] = -0.5;
			(*point)[1] = -0.5;
			(*point)[2] = 0.0;
			point++;
			(*point)[0] = 0.5;
			(*point)[1] = -0.5;
			(*point)[2] = 0.0;
			point++;
			(*point)[0] = 0.5;
			(*point)[1] = 0.5;
			(*point)[2] = 0.0;
			point++;
			(*point)[0] = -0.5;
			(*point)[1] = 0.5;
			(*point)[2] = 0.0;
			/* normals */
			point = normalpoints;
			(*point)[0] = 0.0;
			(*point)[1] = 0.0;
			(*point)[2] = 1.0;
			point++;
			(*point)[0] = 0.0;
			(*point)[1] = 0.0;
			(*point)[2] = 1.0;
			point++;
			(*point)[0] = 0.0;
			(*point)[1] = 0.0;
			(*point)[2] = 1.0;
			point++;
			(*point)[0] = 0.0;
			(*point)[1] = 0.0;
			(*point)[2] = 1.0;
			point++;
			if (!(surface=CREATE(GT_surface)(g_SH_DISCONTINUOUS,g_QUADRILATERAL,1,
				4,points,normalpoints,/*texturepoints*/(Triple *)NULL,
				/*tangentpoints*/(Triple *)NULL,g_NO_DATA,(GTDATA *)NULL)))
			{
				DEALLOCATE(points);
				DEALLOCATE(normalpoints);
			}
		}
		if (surface)
		{
			if (glyph=CREATE(GT_object)(name,g_SURFACE,
				(struct Graphical_material *)NULL))
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"make_glyph_sheet.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_sheet.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_sheet */

struct GT_object *make_glyph_sphere(char *name,int number_of_segments_around,
	int number_of_segments_down)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Creates a graphics object named <name> resembling a sphere with the given
<number_of_segments_around> and <number_of_segments_down> from pole to pole.
The sphere is centred at (0,0,0) and its poles are on the (1,0,0) line. It fits
into the unit cube spanning from -0.5 to +0.5 across all axes. Parameter
<number_of_segments_around> should normally be an even number at least 6 and
twice <number_of_segments_down> look remotely spherical.
==============================================================================*/
{
	float longitudinal_normal,phi,radial_normal,theta,x,y,z;
	int i,j;
	struct GT_object *glyph;
	struct GT_surface *surface;
	Triple *normal,*normalpoints,*points,*vertex;

	ENTER(make_glyph_sphere);
	if (name&&(2<number_of_segments_around)&&(1<number_of_segments_down))
	{
		surface=(struct GT_surface *)NULL;
		if (ALLOCATE(points,Triple,
			(number_of_segments_down+1)*(number_of_segments_around+1))&&
			ALLOCATE(normalpoints,Triple,(number_of_segments_down+1)*
				(number_of_segments_around+1)))
		{
			/*vertex=points;
				normal=points+(number_of_segments_down+1)*(number_of_segments_around+1);*/
			for (i=0;i <= number_of_segments_down;i++)
			{
				phi=PI*(float)i/(float)number_of_segments_down;
				x=-0.5*cos(phi);
				radial_normal=sin(phi);
				longitudinal_normal=2*x;
				/*printf("x=%g l=%g r=%g\n",x,longitudinal_normal,radial_normal);*/
				vertex=points+i;
				normal=normalpoints+i;
				for (j=0;j <= number_of_segments_around;j++)
				{
					theta=2.0*PI*(float)j/(float)number_of_segments_around;
					y = radial_normal*sin(theta);
					z = radial_normal*cos(theta);
					(*vertex)[0] = x;
					(*vertex)[1] = 0.5*y;
					(*vertex)[2] = 0.5*z;
					vertex += (number_of_segments_down+1);
					(*normal)[0] = longitudinal_normal;
					(*normal)[1] = y;
					(*normal)[2] = z;
					normal += (number_of_segments_down+1);
				}
			}
			if (!(surface=CREATE(GT_surface)(g_SHADED_TEXMAP,g_QUADRILATERAL,
				number_of_segments_down+1,number_of_segments_around+1,
				points,normalpoints,/*tangentpoints*/(Triple *)NULL,
            /*texturepoints*/(Triple *)NULL,g_NO_DATA,(GTDATA *)NULL)))
			{
				DEALLOCATE(points);
				DEALLOCATE(normalpoints);
			}
		}
		if (surface)
		{
			if (glyph=CREATE(GT_object)(name,g_SURFACE,
				(struct Graphical_material *)NULL))
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"make_glyph_sphere.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_sphere.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_sphere */

struct LIST(GT_object) *make_standard_glyphs(void)
/*******************************************************************************
LAST MODIFIED : 16 July 2002

DESCRIPTION :
Creates a list of standard glyphs for the cmgui and unemap applications.
==============================================================================*/
{
	char *labels_xyz[] = {"x","y","z"}, *labels_fsn[] = {"f","s","n"},
		 *labels_123[] = {"1","2","3"};
	struct GT_object *glyph, *mirror_glyph;
	struct LIST(GT_object) *glyph_list;

	ENTER(make_glyph_sphere);
	if (glyph_list = CREATE(LIST(GT_object))())
	{
		/* add standard glyphs */
		if (glyph=make_glyph_arrow_line("arrow_line",0.25,0.125))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		mirror_glyph = glyph;
		if (glyph=make_glyph_mirror("mirror_arrow_line",mirror_glyph))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_arrow_solid("arrow_solid",/*primary_axis*/1,
				12,2./3.,1./6.,/*cone_radius*/0.5))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		mirror_glyph = glyph;
		if (glyph=make_glyph_mirror("mirror_arrow_solid",mirror_glyph))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_axes("axes_xyz",/*make_solid*/0,0.1,0.025,labels_xyz, 0.1))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_axes("axes_fsn",/*make_solid*/0,0.1,0.025,labels_fsn, 0.1))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_axes("axes_123",/*make_solid*/0,0.1,0.025,labels_123, 0.1))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_axes("axes",/*make_solid*/0,0.1,0.025,(char **)NULL, 0.1))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_axes("axes_solid",/*make_solid*/1,0.1,0.025,(char **)NULL, 0.1))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_axes("axes_solid_xyz",/*make_solid*/1,0.1,0.025,
			labels_xyz, 0.1))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_cone("cone",12))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		mirror_glyph = glyph;
		if (glyph=make_glyph_mirror("mirror_cone",mirror_glyph))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_cone_solid("cone_solid",12))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_cross("cross"))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_cube_solid("cube_solid"))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_cube_wireframe("cube_wireframe"))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_cylinder("cylinder6",6))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_cylinder("cylinder",12))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_cylinder_solid("cylinder_solid",12))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_cylinder("cylinder_hires",48))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_cylinder_solid("cylinder_solid_hires",48))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_sphere("diamond",4,2))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_line("line"))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		mirror_glyph = glyph;
		if (glyph=make_glyph_mirror("mirror_line",mirror_glyph))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_point("point",g_POINT_MARKER,0))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_sheet("sheet"))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_sphere("sphere",12,6))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
		if (glyph=make_glyph_sphere("sphere_hires",48,24))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_standard_glyphs.  Could not create glyph list");
	}
	LEAVE;

	return (glyph_list);
} /* make_standard_glyphs */
