/*******************************************************************************
FILE : defined_graphics_object.c

LAST MODIFIED : 20 June 2002

DESCRIPTION :
Routines which construct graphics objects. (but do not depend on finite elements)
==============================================================================*/
#include <stdio.h>
#include <math.h>
#include "general/debug.h"
#include "graphics/graphics_object.h"
#include "graphics/spectrum.h"
#include "graphics/spectrum_settings.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Global functions
----------------
*/

int create_Spectrum_colour_bar(struct GT_object **graphics_object_address,
	char *name,struct Spectrum *spectrum,Triple bar_centre,Triple bar_axis,
	Triple side_axis,float bar_length,float bar_radius,float extend_length,
	int tick_divisions,float tick_length,char *number_format,
	struct Graphical_material *bar_material,
	struct Graphical_material *tick_label_material)
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
Creates a coloured bar with annotation for displaying the scale of <spectrum>.
First makes a graphics object named <name> consisting of a cylinder of size
<bar_length>, <bar_radius> centred at <bar_centre> and aligned with the
<bar_axis>, combining the <bar_material> with the <spectrum>. The bar is
coloured by the current range of the spectrum. An extra <extend_length> is added
on to each end of the bar and given the colour the spectrum returns outside its
range at that end.
The <side_axis> must not be collinear with <bar_axis> and is used to calculate
points around the bar and the direction of the <tick_divisions>+1 ticks at which
spectrum values are written.
Attached to the bar graphics_object are two graphics objects using the
<tick_label_material>, one containing the ticks, the other the labels. The
labels are written using the <number_format>.
On successful return a pointer to the bar_graphics_object is put at
<*graphics_object_address>. If there is already a colour_bar at this address it
is cleared and redefined.

The side_axis is made to be orthogonal to the bar_axis, and both are made unit
length by this function.

???RC Function probably belongs elsewhere. We have module
finite_element_to_graphics_object, but how about a more generic module for
graphics_objects that don't come from finite_elements?
==============================================================================*/
{
#define NUMBER_STRLEN (100)
	char **labels, number_string[NUMBER_STRLEN];
	float cos_theta,extend_fraction,half_final_length,length_factor,magnitude,
		sin_theta,spectrum_factor,spectrum_minimum,spectrum_maximum,theta,time,
		unit_factor;
	int allocated_labels,i,j,number_of_ticks,points_along_bar,points_around_bar,
		return_code;
	GTDATA spectrum_value,*data,*datum;
	struct GT_object *bar_graphics_object,*label_graphics_object,
		*tick_graphics_object;
	struct GT_pointset *pointset;
	struct GT_polyline *polyline;
	struct GT_surface *surface;
	Triple base,centre,front_axis,*point,*points,*normal,*normalpoints,
		scaled_axis,scaled_front,scaled_side;

	ENTER(create_Spectrum_colour_bar);
	if (graphics_object_address&&name&&spectrum&&centre&&bar_axis&&side_axis&&
		(0.0<bar_length)&&(0.0<bar_radius)&&(0.0<=extend_length)&&
		(0<tick_divisions)&&(100>=tick_divisions)&&(0.0<=tick_length)&&
		number_format&&number_string&&bar_material&&tick_label_material)
	{
		return_code=1;
		/* add all graphics objects at time 0.0 */
		time = 0.0;
		/* calculate and get range of spectrum */
		Spectrum_calculate_range(spectrum);
		spectrum_minimum=get_Spectrum_minimum(spectrum);
		spectrum_maximum=get_Spectrum_maximum(spectrum);
		/* get orthogonal unit vectors along bar_axis, side and front */
		if (0.0<(magnitude=sqrt(bar_axis[0]*bar_axis[0]+bar_axis[1]*bar_axis[1]+
			bar_axis[2]*bar_axis[2])))
		{
			bar_axis[0] /= magnitude;
			bar_axis[1] /= magnitude;
			bar_axis[2] /= magnitude;
			/* get front = unit cross product of bar_axis and side_axis */
			front_axis[0]=bar_axis[1]*side_axis[2] - bar_axis[2]*side_axis[1];
			front_axis[1]=bar_axis[2]*side_axis[0] - bar_axis[0]*side_axis[2];
			front_axis[2]=bar_axis[0]*side_axis[1] - bar_axis[1]*side_axis[0];
			if (0.0<(magnitude=sqrt(front_axis[0]*front_axis[0]+
				front_axis[1]*front_axis[1]+front_axis[2]*front_axis[2])))
			{
				front_axis[0] /= magnitude;
				front_axis[1] /= magnitude;
				front_axis[2] /= magnitude;
				/* now get side_axis = front_axis (x) bar_axis */
				side_axis[0]=front_axis[1]*bar_axis[2] - front_axis[2]*bar_axis[1];
				side_axis[1]=front_axis[2]*bar_axis[0] - front_axis[0]*bar_axis[2];
				side_axis[2]=front_axis[0]*bar_axis[1] - front_axis[1]*bar_axis[0];
			}
			else
			{
				display_message(ERROR_MESSAGE,"create_Spectrum_colour_bar.  "
					"side axis (tick direction) is in-line with bar axis");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Spectrum_colour_bar.  Invalid bar axis");
			return_code=0;
		}
		if (return_code)
		{
			if (*graphics_object_address)
			{
				/* check whether the existing graphics object was really a colour_bar */
				bar_graphics_object = *graphics_object_address;
				if ((g_SURFACE == bar_graphics_object->object_type) &&
					(0 == strcmp(bar_graphics_object->name,name)) &&
					(tick_graphics_object = bar_graphics_object->nextobject) &&
					(g_POLYLINE == tick_graphics_object->object_type) &&
					(label_graphics_object = tick_graphics_object->nextobject) &&
					(g_POINTSET == label_graphics_object->object_type))
				{
					GT_object_remove_primitives_at_time(bar_graphics_object,time,
						(GT_object_primitive_object_name_conditional_function *)NULL,
						NULL);
					GT_object_remove_primitives_at_time(tick_graphics_object,time,
						(GT_object_primitive_object_name_conditional_function *)NULL,
						NULL);
					GT_object_remove_primitives_at_time(label_graphics_object,time,
						(GT_object_primitive_object_name_conditional_function *)NULL,
						NULL);
					/* valid existing colour_bar */
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Spectrum_colour_bar.  Invalid existing colour_bar");
					return_code=0;
				}
			}
			else
			{
				/* new colour_bar: create and link the bar, ticks and labels */
				if ((bar_graphics_object=
					CREATE(GT_object)(name,g_SURFACE,bar_material))&&
					(tick_graphics_object=
						CREATE(GT_object)("ticks",g_POLYLINE,tick_label_material))&&
					(label_graphics_object=
						CREATE(GT_object)("labels",g_POINTSET,tick_label_material)))
				{
					bar_graphics_object->nextobject =
						ACCESS(GT_object)(tick_graphics_object);
					tick_graphics_object->nextobject =
						ACCESS(GT_object)(label_graphics_object);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Spectrum_colour_bar.  Could not create graphics objects");
					if (bar_graphics_object)
					{
						DESTROY(GT_object)(&bar_graphics_object);
						if (tick_graphics_object)
						{
							DESTROY(GT_object)(&tick_graphics_object);
						}
					}
					return_code=0;
				}
			}
		}
		if (return_code)
		{
			/* create the colour bar */
			points=(Triple *)NULL;
			data=(GTDATA *)NULL;
			surface=(struct GT_surface *)NULL;
			points_along_bar = 109;
			points_around_bar = 25;
			if (ALLOCATE(points,Triple,points_along_bar*points_around_bar)&&
				ALLOCATE(normalpoints,Triple,points_along_bar*points_around_bar)&&
				ALLOCATE(data,GTDATA,points_along_bar*points_around_bar))
			{
				extend_fraction=extend_length/bar_length;
				half_final_length=0.5*bar_length+extend_length;
				scaled_axis[0]=half_final_length*bar_axis[0];
				scaled_axis[1]=half_final_length*bar_axis[1];
				scaled_axis[2]=half_final_length*bar_axis[2];
				point=points;
				normal=normalpoints;
				datum=data;
				for (i=0;i<points_along_bar;i++)
				{
#if defined (OLD_CODE)
					point=points+i;
					normal=point+points_along_bar*points_around_bar;
					datum=data+i;
#endif /* defined (OLD_CODE) */
					/* get factor ranging from 0.0 to 1.0 over extended length */
					unit_factor=(float)i/(float)(points_along_bar-1);
					/* get factor ranging from -1 to +1 along bar */
					length_factor=2.0*unit_factor-1.0;
					/* get centre of bar for ith point */
					centre[0]=bar_centre[0] + length_factor*scaled_axis[0];
					centre[1]=bar_centre[1] + length_factor*scaled_axis[1];
					centre[2]=bar_centre[2] + length_factor*scaled_axis[2];
					/* get factor ranging from 0 to 1 over the bar_length and extending
						 outside that range in the extend_length */
					spectrum_factor=unit_factor*(1.0+2.0*extend_fraction)-extend_fraction;
					/* interpolate to get spectrum value */
					spectrum_value = (GTDATA)((1.0-spectrum_factor)*spectrum_minimum +
						spectrum_factor*spectrum_maximum);
					for (j=0;j<points_around_bar;j++)
					{
						theta=(float) j * 2.0 * PI / (float)(points_around_bar-1);
						cos_theta=cos(theta);
						sin_theta=sin(theta);
						scaled_side[0]=cos_theta*side_axis[0];
						scaled_side[1]=cos_theta*side_axis[1];
						scaled_side[2]=cos_theta*side_axis[2];
						scaled_front[0]=sin_theta*front_axis[0];
						scaled_front[1]=sin_theta*front_axis[1];
						scaled_front[2]=sin_theta*front_axis[2];
						/* set point */
						(*point)[0]=centre[0]+bar_radius*(scaled_side[0]+scaled_front[0]);
						(*point)[1]=centre[1]+bar_radius*(scaled_side[1]+scaled_front[1]);
						(*point)[2]=centre[2]+bar_radius*(scaled_side[2]+scaled_front[2]);
						point++;
						/* set normal */
						(*normal)[0]=scaled_side[0]+scaled_front[0];
						(*normal)[1]=scaled_side[1]+scaled_front[1];
						(*normal)[2]=scaled_side[2]+scaled_front[2];
						normal++;
						/* set data */
						*datum=spectrum_value;
						datum++;
					}
				}
				return_code=(
					(surface=CREATE(GT_surface)(g_SHADED,g_QUADRILATERAL,
						points_around_bar,points_along_bar,points,normalpoints,
						/*tangentpoints*/(Triple *)NULL,
						/*texturepoints*/(Triple *)NULL,/*n_data_components*/1,data)) &&
					set_GT_object_default_material(bar_graphics_object,bar_material) &&
					set_GT_object_Spectrum(bar_graphics_object,spectrum)&&
					GT_OBJECT_ADD(GT_surface)(bar_graphics_object,time,surface));
			}
			else
			{
				return_code=0;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"create_Spectrum_colour_bar.  Could not build spectrum bar");
				if (surface)
				{
					DESTROY(GT_surface)(&surface);
				}
				else
				{
					DEALLOCATE(points);
					DEALLOCATE(data);
				}
			}
		}
		/* get values for calculating tick and label positions */
		number_of_ticks=tick_divisions+1;
		scaled_axis[0]=bar_length*bar_axis[0];
		scaled_axis[1]=bar_length*bar_axis[1];
		scaled_axis[2]=bar_length*bar_axis[2];
		base[0]=bar_centre[0]-0.5*scaled_axis[0]+bar_radius*side_axis[0];
		base[1]=bar_centre[1]-0.5*scaled_axis[1]+bar_radius*side_axis[1];
		base[2]=bar_centre[2]-0.5*scaled_axis[2]+bar_radius*side_axis[2];
		scaled_side[0]=tick_length*side_axis[0];
		scaled_side[1]=tick_length*side_axis[1];
		scaled_side[2]=tick_length*side_axis[2];
		if (return_code)
		{
			/* create the scale ticks */
			points=(Triple *)NULL;
			polyline=(struct GT_polyline *)NULL;
			if (ALLOCATE(points,Triple,2*number_of_ticks))
			{
				point=points;
				for (i=0;i<number_of_ticks;i++)
				{
					/* get factor ranging from 0.0 to 1.0 over bar_length */
					unit_factor=(float)i/(float)(number_of_ticks-1);
					(*point)[0]=base[0]+unit_factor*scaled_axis[0];
					(*point)[1]=base[1]+unit_factor*scaled_axis[1];
					(*point)[2]=base[2]+unit_factor*scaled_axis[2];
					(*(point+1))[0]=(*point)[0]+scaled_side[0];
					(*(point+1))[1]=(*point)[1]+scaled_side[1];
					(*(point+1))[2]=(*point)[2]+scaled_side[2];
					point += 2;
				}
				return_code=(
					(polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,/*line_width=default*/0,
						number_of_ticks,points,/*normalpoints*/(Triple *)NULL,
						/*n_data_components*/0,(GTDATA *)NULL))&&
					set_GT_object_default_material(tick_graphics_object,
						tick_label_material) &&
					GT_OBJECT_ADD(GT_polyline)(tick_graphics_object,time,polyline));
			}
			else
			{
				return_code=0;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"create_Spectrum_colour_bar.  Could not build scale ticks");
				if (polyline)
				{
					DESTROY(GT_polyline)(&polyline);
				}
				else
				{
					DEALLOCATE(points);
				}
			}
		}
		/* labels are drawn at the ends of the ticks */
		base[0] += scaled_side[0];
		base[1] += scaled_side[1];
		base[2] += scaled_side[2];
		if (return_code)
		{
			/* create the scale labels */
			points=(Triple *)NULL;
			labels=(char **)NULL;
			pointset=(struct GT_pointset *)NULL;
			allocated_labels=0;
			if (ALLOCATE(points,Triple,number_of_ticks)&&
				ALLOCATE(labels,char *,number_of_ticks))
			{
				point=points;
				for (i=0;return_code&&(i<number_of_ticks);i++)
				{
					/* get factor ranging from 0.0 to 1.0 over bar_length */
					unit_factor=(float)i/(float)(number_of_ticks-1);
					(*point)[0]=base[0]+unit_factor*scaled_axis[0];
					(*point)[1]=base[1]+unit_factor*scaled_axis[1];
					(*point)[2]=base[2]+unit_factor*scaled_axis[2];
					point++;
					/* interpolate to get spectrum value */
					spectrum_value=(GTDATA)((1.0-unit_factor)*spectrum_minimum +
						unit_factor*spectrum_maximum);
					if (sprintf(number_string, number_format,
						spectrum_value) > (NUMBER_STRLEN - 2))
					{
						/* Add the termination */
						number_string[NUMBER_STRLEN-1] = 0;
						display_message(WARNING_MESSAGE,
							"create_Spectrum_colour_bar.  Tick label was truncated.");
					}
					if (ALLOCATE(labels[i],char,strlen(number_string)+1))
					{
						allocated_labels++;
						strcpy(labels[i],number_string);
					}
					else
					{
						return_code=0;
					}
				}
				return_code=((pointset=
					CREATE(GT_pointset)(number_of_ticks,points,labels,g_NO_MARKER,0.0,
						/*n_data_components*/0,(GTDATA *)NULL,(int *)NULL)) &&
					set_GT_object_default_material(label_graphics_object,
						tick_label_material) &&
					GT_OBJECT_ADD(GT_pointset)(label_graphics_object,time,pointset));
			}
			else
			{
				return_code=0;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"create_Spectrum_colour_bar.  Could not build scale labels");
				if (pointset)
				{
					DESTROY(GT_pointset)(&pointset);
				}
				else
				{
					DEALLOCATE(points);
					for (i=0;i<allocated_labels;i++)
					{
						DEALLOCATE(labels[i]);
					}
					DEALLOCATE(labels);
				}
			}
			if (!(*graphics_object_address))
			{
				if (return_code)
				{
					*graphics_object_address = bar_graphics_object;
				}
				else
				{
					DESTROY(GT_object)(&bar_graphics_object);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Spectrum_colour_bar.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* create_Spectrum_colour_bar */
