/*******************************************************************************
FILE : graphics_object.c

LAST MODIFIED : 20 February 2000

DESCRIPTION :
gtObject/gtWindow management routines.
???DB.  Used to be gtutil.c
???DB.  7 June 1994.  Merged GTTEXT into GTPOINT and GTPOINTSET and added a
	marker type and a marker size
???DB.  11 January 1997.  Added pointers to the nodes in a GTPOINTSET.  This is
	a temporary measure to allow the graphical_node_editor to work (will be
	replaced by the graphical FE_node). See below.
???RC.  Changed above nodes to integer names for	OpenGL picking.
==============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "command/parser.h"
#include "finite_element/computed_field.h"
#include "finite_element/finite_element.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/object.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/graphics_object.h"
#include "graphics/makegtobj.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
#include "user_interface/message.h"

/*
Global variables
----------------
*/
/*???DB.  I'm not sure that this should be here */
float global_line_width=1.,global_point_size=1.;


/*
Module types
------------
*/

FULL_DECLARE_INDEXED_LIST_TYPE(GT_object);

/*
Module functions
----------------
*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(GT_object,name,char *,strcmp)

static int GT_object_get_time_number(struct GT_object *graphics_object,
	float time)
/*******************************************************************************
LAST MODIFIED : 18 June 1998

DESCRIPTION :
The graphics object stores a list of times at which primitives/key frames are
kept. This function returns the index into this array that <time> is at,
starting with 1 for the first time, whereas 0 is returned if the time is not
found or there is an error.
==============================================================================*/
{
	float *times;
	int time_no;

	ENTER(GT_object_get_time_number);
	if (graphics_object)
	{
		if (0<(time_no=graphics_object->number_of_times))
		{
			if (times=graphics_object->times)
			{
				times += time_no-1;
				while ((time_no>0)&&(time< *times))
				{
					times--;
					time_no--;
				}
				if ((time_no>0)&&(time != *times))
				{
					time_no=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"GT_object_get_time_number.  Invalid times array");
				time_no=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_get_time_number.  Invalid arguments");
		time_no=0;
	}
	LEAVE;

	return (time_no);
} /* GT_object_get_time_number */

static int GT_object_get_less_than_or_equal_time_number(
	struct GT_object *graphics_object,float time)
/*******************************************************************************
LAST MODIFIED : 7 July 1998

DESCRIPTION :
The graphics object stores a list of times at which primitives/key frames are
kept. This function returns the index into this array of the first time less
than or equal to <time>, starting with 1 for the first time. A value of 0 is
returned if <time> is lower than any times in the array or an error occurs.
==============================================================================*/
{
	float *times;
	int time_no;

	ENTER(GT_object_get_less_than_or_equal_time_number);
	if (graphics_object)
	{
		if (0<(time_no=graphics_object->number_of_times))
		{
			if (times=graphics_object->times)
			{
				times += time_no-1;
				while ((time_no>0)&&(time< *times))
				{
					times--;
					time_no--;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"GT_object_get_less_than_or_equal_time_number.  Invalid times array");
				time_no=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_get_less_than_or_equal_time_number.  Invalid arguments");
		time_no=0;
	}
	LEAVE;

	return (time_no);
} /* GT_object_get_less_than_or_equal_time_number */

#if defined (FULL_NAMES)
#define GT_OBJECT_DELETE_TIME_NUMBER_(primitive_type) \
	GT_object_delete_time_number_ ## primitive_type
#else
#define GT_OBJECT_DELETE_TIME_NUMBER_(primitive_type) \
	godtn_ ## primitive_type
#endif
#define GT_OBJECT_DELETE_TIME_NUMBER(primitive_type) \
	GT_OBJECT_DELETE_TIME_NUMBER_(primitive_type)

#define DECLARE_GT_OBJECT_DELETE_TIME_NUMBER_FUNCTION(primitive_type, \
	gt_object_type,primitive_var,primitive_destroy_function) \
static int GT_OBJECT_DELETE_TIME_NUMBER(primitive_type)( \
	struct GT_object *graphics_object,int time_no) \
/***************************************************************************** \
LAST MODIFIED : 19 June 1997 \
\
DESCRIPTION : \
Removes time<time_no> and all the primitives in it from <graphics_object>. \
============================================================================*/ \
{ \
	float *times; \
	struct primitive_type *primitive,**primitive_ptr; \
	int return_code,i; \
\
	ENTER(GT_OBJECT_DELETE_TIME_NUMBER(primitive_type)); \
	/* check arguments */ \
	if (graphics_object&&(gt_object_type==graphics_object->object_type)&& \
		(0<time_no)&&(time_no <= graphics_object->number_of_times)) \
	{ \
		if (times=graphics_object->times) \
		{ \
			if (primitive_ptr=graphics_object->gu.primitive_var) \
			{ \
				primitive_ptr += time_no-1; \
				/* Remove primitives at this time */ \
				while (primitive= *primitive_ptr) \
				{ \
					*primitive_ptr=primitive->ptrnext; \
					primitive_destroy_function(&primitive); \
				} \
				/* Shift following times and primitives down */ \
				for (i=(graphics_object->number_of_times)-time_no;i>0;i--) \
				{ \
					times[0]=times[1]; \
					times++; \
					primitive_ptr[0]=primitive_ptr[1]; \
					primitive_ptr++; \
				} \
				graphics_object->number_of_times--; \
				/* do not reallocate to make times and primitive_var arrays smaller \
					 since this function is most often called by DESTROY(GT_object), or \
					 just before primitives are to be added again at the same time. \
					 However, must deallocate if there are no times left so DESTROY \
					 function can rely on this function to clean up. */ \
				if (0 >= graphics_object->number_of_times) \
				{ \
					DEALLOCATE(graphics_object->times); \
					DEALLOCATE(graphics_object->gu.primitive_var); \
				} \
				return_code=1; \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE,"GT_OBJECT_DELETE_TIME_NUMBER(" \
					#primitive_type ").  Invalid primitives array"); \
				return_code=0; \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE,"GT_OBJECT_DELETE_TIME_NUMBER(" \
				#primitive_type ").  Invalid times array"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"GT_OBJECT_DELETE_TIME_NUMBER(" \
	 		#primitive_type ").  Invalid arguments"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* GT_OBJECT_DELETE_TIME_NUMBER(primitive_type) */

DECLARE_GT_OBJECT_DELETE_TIME_NUMBER_FUNCTION(GT_glyph_set, \
	g_GLYPH_SET,gt_glyph_set,DESTROY(GT_glyph_set))
DECLARE_GT_OBJECT_DELETE_TIME_NUMBER_FUNCTION(GT_nurbs, \
	g_NURBS,gt_nurbs,DESTROY(GT_nurbs))
DECLARE_GT_OBJECT_DELETE_TIME_NUMBER_FUNCTION(GT_point, \
	g_POINT,gt_point,DESTROY(GT_point))
DECLARE_GT_OBJECT_DELETE_TIME_NUMBER_FUNCTION(GT_pointset, \
	g_POINTSET,gt_pointset,DESTROY(GT_pointset))
DECLARE_GT_OBJECT_DELETE_TIME_NUMBER_FUNCTION(GT_polyline, \
	g_POLYLINE,gt_polyline,DESTROY(GT_polyline))
DECLARE_GT_OBJECT_DELETE_TIME_NUMBER_FUNCTION(GT_surface, \
	g_SURFACE,gt_surface,DESTROY(GT_surface))
DECLARE_GT_OBJECT_DELETE_TIME_NUMBER_FUNCTION(GT_userdef, \
	g_POINT,gt_userdef,DESTROY(GT_userdef))
DECLARE_GT_OBJECT_DELETE_TIME_NUMBER_FUNCTION(GT_voltex, \
	g_VOLTEX,gt_voltex,DESTROY(GT_voltex))

static int GT_object_delete_time_number(struct GT_object *graphics_object,
	int time_no)
/*******************************************************************************
LAST MODIFIED : 6 July 1998

DESCRIPTION :
Type-less version of GT_OBJECT_DELETE_TIME_NUMBER macro.
==============================================================================*/
{
	int return_code;

	ENTER(GT_object_delete_time_number);
	if (graphics_object)
	{
		switch (graphics_object->object_type)
		{
			case g_GLYPH_SET:
			{
				return_code=GT_OBJECT_DELETE_TIME_NUMBER(GT_glyph_set)(
					graphics_object,time_no);
			} break;
			case g_NURBS:
			{
				return_code=GT_OBJECT_DELETE_TIME_NUMBER(GT_nurbs)(
					graphics_object,time_no);
			} break;
			case g_POINT:
			{
				return_code=GT_OBJECT_DELETE_TIME_NUMBER(GT_point)(
					graphics_object,time_no);
			} break;
			case g_POINTSET:
			{
				return_code=GT_OBJECT_DELETE_TIME_NUMBER(GT_pointset)(
					graphics_object,time_no);
			} break;
			case g_POLYLINE:
			{
				return_code=GT_OBJECT_DELETE_TIME_NUMBER(GT_polyline)(
					graphics_object,time_no);
			} break;
			case g_SURFACE:
			{
				return_code=GT_OBJECT_DELETE_TIME_NUMBER(GT_surface)(
					graphics_object,time_no);
			} break;
			case g_USERDEF:
			{
				return_code=GT_OBJECT_DELETE_TIME_NUMBER(GT_userdef)(
					graphics_object,time_no);
			} break;
			case g_VOLTEX:
			{
				return_code=GT_OBJECT_DELETE_TIME_NUMBER(GT_voltex)(
					graphics_object,time_no);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"GT_object_delete_time_number.  Unknown object type");
				return_code=0;
			}
		}
		GT_object_changed(graphics_object);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_delete_time_number.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_delete_time_number */

/*
Global functions
----------------
*/

char *get_GT_object_type_string(enum GT_object_type object_type)
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Returns a string describing the object type, suitable for writing to file
(and reinterpreting it later).
==============================================================================*/
{
	char *type_string;

	ENTER(get_GT_object_type_string);
	switch (object_type)
	{
		case g_GLYPH_SET:
		{
			type_string="GLYPH_SET";
		} break;
		case g_NURBS:
		{
			type_string="NURBS";
		} break;
		case g_POINT:
		{
			type_string="POINT";
		} break;
		case g_POINTSET:
		{
			type_string="POINTSET";
		} break;
		case g_POLYLINE:
		{
			type_string="POLYLINE";
		} break;
		case g_SURFACE:
		{
			type_string="SURFACE";
		} break;
		case g_USERDEF:
		{
			type_string="USERDEF";
		} break;
		case g_VOLTEX:
		{
			type_string="VOLTEX";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"get_GT_object_type_string.  Unknown object type");
			type_string=(char *)NULL;
		}
	}
	LEAVE;

	return type_string;
} /* get_GT_object_type_string */

int get_GT_object_type_from_string(char *type_string,
	enum GT_object_type *object_type)
/*******************************************************************************
LAST MODIFIED : 14 August 1998

DESCRIPTION :
Returns the object type from the string produced by function
get_GT_object_type_string. For compatibility, also supports converting old
enumerator numbers (as text) into the new enumerator values, with a warning.
==============================================================================*/
{
	char *compare_type_string;
	int return_code;
	enum GT_object_type temp_obj_type;

	ENTER(get_GT_object_type_from_string);
	if (type_string&&object_type)
	{
		return_code=1;
		temp_obj_type=g_OBJECT_TYPE_BEFORE_FIRST;
		temp_obj_type++;
		while ((temp_obj_type<g_OBJECT_TYPE_AFTER_LAST)&&
			(compare_type_string=get_GT_object_type_string(temp_obj_type))&&
			(!fuzzy_string_compare_same_length(compare_type_string,type_string)))
		{
			temp_obj_type++;
		}
		if (g_OBJECT_TYPE_AFTER_LAST==temp_obj_type)
		{
			display_message(ERROR_MESSAGE,"get_GT_object_type_from_string.  "
				"Object type string '%s' not recognized",type_string);
			return_code=0;
		}
		else
		{
			*object_type=temp_obj_type;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_GT_object_type_from_string.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_GT_object_type_from_string */

char *get_GT_polyline_type_string(enum GT_polyline_type polyline_type)
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Returns a string describing the polyline type, suitable for writing to file
(and reinterpreting it later).
==============================================================================*/
{
	char *type_string;

	ENTER(get_GT_polyline_type_string);
	switch (polyline_type)
	{
		case g_PLAIN:
		{
			type_string="PLAIN";
		} break;
		case g_NORMAL:
		{
			type_string="NORMAL";
		} break;
		case g_PLAIN_DISCONTINUOUS:
		{
			type_string="PLAIN_DISCONTINUOUS";
		} break;
		case g_NORMAL_DISCONTINUOUS:
		{
			type_string="NORMAL_DISCONTINUOUS";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"get_GT_polyline_type_string.  Unknown polyline type");
			type_string=(char *)NULL;
		}
	}
	LEAVE;

	return type_string;
} /* get_GT_polyline_type_string */

int get_GT_polyline_type_from_string(char *type_string,
	enum GT_polyline_type *polyline_type)
/*******************************************************************************
LAST MODIFIED : 14 August 1998

DESCRIPTION :
Returns the polyline type from the string produced by function
get_GT_polyline_type_string. For compatibility, also supports converting old
enumerator numbers (as text) into the new enumerator values, with a warning.
==============================================================================*/
{
	char *compare_type_string;
	int return_code;
	enum GT_polyline_type temp_poly_type;

	ENTER(get_GT_polyline_type_from_string);
	if (type_string&&polyline_type)
	{
		return_code=1;
		temp_poly_type=g_POLYLINE_TYPE_BEFORE_FIRST;
		temp_poly_type++;
		while ((temp_poly_type<g_POLYLINE_TYPE_AFTER_LAST)&&
			(compare_type_string=get_GT_polyline_type_string(temp_poly_type))&&
			(!fuzzy_string_compare_same_length(compare_type_string,type_string)))
		{
			temp_poly_type++;
		}
		if (g_POLYLINE_TYPE_AFTER_LAST==temp_poly_type)
		{
			/* Previously, polyline types were read in from .exgobj files as numbers
				 matching the enumerated values. This is unacceptable now that more
				 types have been added. For compatibility, however, the original numbers
				 are read in to the correct types with a warning message */
			if (fuzzy_string_compare("0",type_string))
			{
				*polyline_type=g_PLAIN;
			}
			else if (fuzzy_string_compare("1",type_string))
			{
				*polyline_type=g_NORMAL;
			}
			else if (fuzzy_string_compare("2",type_string))
			{
				*polyline_type=g_PLAIN_DISCONTINUOUS;
			}
			else if (fuzzy_string_compare("3",type_string))
			{
				*polyline_type=g_NORMAL_DISCONTINUOUS;
			}
			else
			{
				display_message(ERROR_MESSAGE,"get_GT_polyline_type_from_string.  "
					"Polyline type string '%s' not recognized",type_string);
				return_code=0;
			}
			if (return_code)
			{
				display_message(WARNING_MESSAGE,
					"Old style polyline type '%s' should be updated to '%s'",
					type_string,get_GT_polyline_type_string(*polyline_type));
			}
		}
		else
		{
			*polyline_type=temp_poly_type;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_GT_polyline_type_from_string.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_GT_polyline_type_from_string */

char *get_GT_surface_type_string(enum GT_surface_type surface_type)
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Returns a string describing the surface type, suitable for writing to file
(and reinterpreting it later).
==============================================================================*/
{
	char *type_string;

	ENTER(get_GT_surface_type_string);
	switch (surface_type)
	{
		case g_SHADED:
		{
			type_string="SHADED";
		} break;
		case g_SH_DISCONTINUOUS:
		{
			type_string="SH_DISCONTINUOUS";
		} break;
		case g_SHADED_TEXMAP:
		{
			type_string="SHADED_TEXMAP";
		} break;
		case g_SH_DISCONTINUOUS_TEXMAP:
		{
			type_string="SH_DISCONTINUOUS_TEXMAP";
		} break;
		case g_SH_DISCONTINUOUS_STRIP:
		{
			type_string="SH_DISCONTINUOUS_STRIP";
		} break;
		case g_SH_DISCONTINUOUS_STRIP_TEXMAP:
		{
			type_string="SH_DISCONTINUOUS_STRIP_TEXMAP";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"get_GT_surface_type_string.  Unknown surface type");
			type_string=(char *)NULL;
		}
	}
	LEAVE;

	return type_string;
} /* get_GT_surface_type_string */

int get_GT_surface_type_from_string(char *type_string,
	enum GT_surface_type *surface_type)
/*******************************************************************************
LAST MODIFIED : 14 August 1998

DESCRIPTION :
Returns the surface type from the string produced by function
get_GT_surface_type_string. For compatibility, also supports converting old
enumerator numbers (as text) into the new enumerator values, with a warning.
==============================================================================*/
{
	char *compare_type_string;
	int return_code;
	enum GT_surface_type temp_surf_type;

	ENTER(get_GT_surface_type_from_string);
	if (type_string&&surface_type)
	{
		return_code=1;
		temp_surf_type=g_SURFACE_TYPE_BEFORE_FIRST;
		temp_surf_type++;
		while ((temp_surf_type<g_SURFACE_TYPE_AFTER_LAST)&&
			(compare_type_string=get_GT_surface_type_string(temp_surf_type))&&
			(!fuzzy_string_compare_same_length(compare_type_string,type_string)))
		{
			temp_surf_type++;
		}
		if (g_SURFACE_TYPE_AFTER_LAST==temp_surf_type)
		{
			/* Previously, surface types were read in from .exgobj files as numbers
				 matching the enumerated values. This is unacceptable now that more
				 types have been added. For compatibility, however, the original numbers
				 are read in to the correct types with a warning message */
			if (fuzzy_string_compare("0",type_string))
			{
				*surface_type=g_SHADED;
			}
			else if (fuzzy_string_compare("3",type_string))
			{
				*surface_type=g_SH_DISCONTINUOUS;
			}
			else if (fuzzy_string_compare("6",type_string))
			{
				*surface_type=g_SHADED_TEXMAP;
			}
			else if (fuzzy_string_compare("7",type_string))
			{
				*surface_type=g_SH_DISCONTINUOUS_TEXMAP;
			}
			else
			{
				display_message(ERROR_MESSAGE,"get_GT_surface_type_from_string.  "
					"Surface type string '%s' not recognized",type_string);
				return_code=0;
			}
			if (return_code)
			{
				display_message(WARNING_MESSAGE,
					"Old style surface type '%s' should be updated to '%s'",
					type_string,get_GT_surface_type_string(*surface_type));
			}
		}
		else
		{
			*surface_type=temp_surf_type;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_GT_surface_type_from_string.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_GT_surface_type_from_string */

struct GT_glyph_set *morph_GT_glyph_set(float proportion,
	struct GT_glyph_set *initial,struct GT_glyph_set *final)
/*******************************************************************************
LAST MODIFIED : 23 February 2000

DESCRIPTION :
Creates a new GT_glyph_set which is the interpolation of two GT_glyph_sets.
The two glyph_sets must have the same number_of_points, glyph and data_type.
Text, if any, is copied from the <initial> glyph_set if proportion < 0.5,
otherwise from <final>.
==============================================================================*/
{
	char **labels;
	GTDATA *data;
	struct GT_glyph_set *glyph_set,*nearest_glyph_set;
	int i,j,number_of_points,*names,return_code;
	Triple *point_list,*axis1_list,*axis2_list,*axis3_list;

	ENTER(morph_GT_glyph_set);
	if ((initial->number_of_points==final->number_of_points)&&
		(initial->glyph==final->glyph)&&
		(initial->n_data_components==final->n_data_components))
	{
		return_code=1;
		number_of_points=initial->number_of_points;
		if (proportion<0.5)
		{
			nearest_glyph_set=initial;
		}
		else
		{
			nearest_glyph_set=final;
		}
		point_list=(Triple *)NULL;
		axis1_list=(Triple *)NULL;
		axis2_list=(Triple *)NULL;
		axis3_list=(Triple *)NULL;
		data=(GTDATA *)NULL;
		labels=(char **)NULL;
		names=(int *)NULL;
		if ((0<number_of_points)&&ALLOCATE(point_list,Triple,number_of_points)&&
			ALLOCATE(axis1_list,Triple,number_of_points)&&
			ALLOCATE(axis2_list,Triple,number_of_points)&&
			ALLOCATE(axis3_list,Triple,number_of_points))
		{
			for (i=0;i<number_of_points;i++)
			{
				for (j=0;j<3;j++)
				{
					point_list[i][j]=(1.0-proportion)*(initial->point_list)[i][j]+
						proportion*(final->point_list)[i][j];
					axis1_list[i][j]=(1.0-proportion)*(initial->axis1_list)[i][j]+
						proportion*(final->axis1_list)[i][j];
					axis2_list[i][j]=(1.0-proportion)*(initial->axis2_list)[i][j]+
						proportion*(final->axis2_list)[i][j];
					axis3_list[i][j]=(1.0-proportion)*(initial->axis3_list)[i][j]+
						proportion*(final->axis3_list)[i][j];
				}
			}
			/* check for and allocate any data */
			if (initial->n_data_components)
			{
				if (ALLOCATE(data,GTDATA,number_of_points*initial->n_data_components))
				{
					for (i=0;i<number_of_points*initial->n_data_components;i++)
					{
						data[i]=(1.0-proportion)*(initial->data)[i]+
							proportion*(final->data)[i];
					}
				}
				else
				{
					return_code=0;
				}
			}
			if (point_list&&nearest_glyph_set->labels)
			{
				if (ALLOCATE(labels,char *,number_of_points))
				{
					for (i=0;labels&&(i<number_of_points);i++)
					{
						if (nearest_glyph_set->labels[i])
						{
							if (ALLOCATE(labels[i],char,
								strlen(nearest_glyph_set->labels[i])+1))
							{
								strcpy(labels[i],nearest_glyph_set->labels[i]);
							}
							else
							{
								for (j=0;j<i;j++)
								{
									DEALLOCATE(labels[i]);
								}
								DEALLOCATE(labels);
							}
						}
						else
						{
							labels[i]=(char *)NULL;
						}
					}
				}
				if (!labels)
				{
					return_code=0;
				}
			}
			if (point_list&&nearest_glyph_set->names)
			{
				if (ALLOCATE(names,int,number_of_points))
				{
					for (i=0;i<number_of_points;i++)
					{
						names[i] = nearest_glyph_set->names[i];
					}
				}
				else
				{
					return_code=0;
				}
			}
			if (return_code)
			{
				if (glyph_set=CREATE(GT_glyph_set)(number_of_points,point_list,
					axis1_list,axis2_list,axis3_list,initial->glyph,labels,
					initial->n_data_components,data,
					nearest_glyph_set->object_name,names))
				{
#if defined (OLD_CODE)
/*???DB.  Have to be recursive on destroy_ and morph_ or neither */
					/* go recursive in case it is a linked list */
					if (initial->ptrnext&&final->ptrnext)
					{
						if (!(glyph_set->ptrnext=morph_GT_glyph_set(proportion,
							initial->ptrnext,final->ptrnext)))
						{
							DESTROY(GT_glyph_set)(&glyph_set);
						}
					}
#endif /* defined (OLD_CODE) */
				}
				else
				{
					return_code=0;
				}
			}
		}
		else
		{
			return_code=0;
		}
		if (!return_code)
		{
			/* no glyph_set has been produced; clean up allocated arrays */
			display_message(ERROR_MESSAGE,
				"morph_GT_glyph_set.  Could not create glyph_set");
			DEALLOCATE(point_list);
			DEALLOCATE(axis1_list);
			DEALLOCATE(axis2_list);
			DEALLOCATE(axis3_list);
			DEALLOCATE(data);
			if (labels)
			{
				for (i=0;i<nearest_glyph_set->number_of_points;i++)
				{
					DEALLOCATE(labels[i]);
				}
				DEALLOCATE(labels);
			}
			DEALLOCATE(names);
			glyph_set=(struct GT_glyph_set *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"morph_GT_glyph_set.  Inconsistent initial and final glyph_sets");
		glyph_set=(struct GT_glyph_set *)NULL;
	}
	LEAVE;

	return (glyph_set);
} /* morph_GT_glyph_set */

struct GT_pointset *morph_GT_pointset(float proportion,
	struct GT_pointset *initial,struct GT_pointset *final)
/*******************************************************************************
LAST MODIFIED : 16 April 1999

DESCRIPTION :
Creates a new GT_pointset which is the interpolation of two GT_pointsets.
==============================================================================*/
{
	char **source_text,**text;
	float marker_size;
	GTDATA *data;
	struct GT_pointset *point_set;
	int i,j,number_of_points,*names,*morph_names;
	Triple *point;

	ENTER(morph_GT_pointset);
	if (
#if !defined (ALLOW_MORPH_UNEQUAL_POINTSETS)
		(initial->n_pts==final->n_pts)&&
#endif /* !defined (ALLOW_MORPH_UNEQUAL_POINTSETS) */
		(initial->n_data_components==final->n_data_components)&&
		(initial->marker_type==final->marker_type))
	{
		number_of_points=initial->n_pts;
#if defined (ALLOW_MORPH_UNEQUAL_POINTSETS)
		if (final->n_pts<number_of_points)
		{
			number_of_points=final->n_pts;
		}
#endif /* defined (ALLOW_MORPH_UNEQUAL_POINTSETS) */
		marker_size=
			(1-proportion)*(initial->marker_size)+proportion*(final->marker_size);
		if (proportion<0.5)
		{
			source_text=initial->text;
			names=initial->names;
		}
		else
		{
			source_text=final->text;
			names=final->names;
		}
		point=(Triple *)NULL;
		morph_names=(int *)NULL;
		text=(char **)NULL;
		if ((number_of_points>0)&&ALLOCATE(point,Triple,number_of_points)&&
			(!names||ALLOCATE(morph_names,int,number_of_points)))
		{
			for (i=0;i<number_of_points;i++)
			{
				for (j=0;j<3;j++)
				{
					point[i][j]=(1.0-proportion)*(initial->pointlist)[i][j]+
						proportion*(final->pointlist)[i][j];
				}
				if (morph_names)
				{
					morph_names[i]=names[i];
				}
			}
			/* check for and allocate any data */
			if (initial->n_data_components)
			{
				if (ALLOCATE(data,GTDATA,number_of_points*initial->n_data_components))
				{
					for (i=0;i<number_of_points*initial->n_data_components;i++)
					{
						data[i]=(1.0-proportion)*(initial->data)[i]+
							proportion*(final->data)[i];
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"morph_GT_pointset.  Could not allocate data");
					DEALLOCATE(point);
				}
			}
			else
			{
				data=(GTDATA *)NULL;
			}
			if (point&&source_text)
			{
				if (ALLOCATE(text,char *,number_of_points))
				{
					for (i=0;text&&(i<number_of_points);i++)
					{
						if (source_text[i])
						{
							if (ALLOCATE(text[i],char,strlen(source_text[i])+1))
							{
								strcpy(text[i],source_text[i]);
							}
							else
							{
								for (j=0;j<i;j++)
								{
									DEALLOCATE(text[i]);
								}
								DEALLOCATE(text);
							}
						}
						else
						{
							text[i]=(char *)NULL;
						}
					}
				}
				if (!text)
				{
					display_message(ERROR_MESSAGE,
						"morph_GT_pointset.  Could not copy text");
					DEALLOCATE(point);
					DEALLOCATE(data);
				}
			}
			if (point)
			{
				if (point_set=CREATE(GT_pointset)(initial->n_pts,point,text,
					initial->marker_type,marker_size,initial->n_data_components,data,
					morph_names))
				{
#if defined (OLD_CODE)
/*???DB.  Have to be recursive on destroy_ and morph_ or neither */
					/* go recursive in case it is a linked list */
					if (initial->ptrnext&&final->ptrnext)
					{
						if (!(point_set->ptrnext=morph_GT_pointset(proportion,
							initial->ptrnext,final->ptrnext)))
						{
							DESTROY(GT_pointset)(&point_set);
						}
					}
#endif /* defined (OLD_CODE) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"morph_GT_pointset.  Could not create point_set");
					DEALLOCATE(data);
					DEALLOCATE(point);
					if (morph_names)
					{
						DEALLOCATE(morph_names);
					}
				}
			}
			else
			{
				if (morph_names)
				{
					DEALLOCATE(morph_names);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"morph_GT_pointset.  Could not allocate points");
			if (point)
			{
				DEALLOCATE(point);
			}
			point_set=(struct GT_pointset *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"morph_GT_pointset.  Inconsistent initial and final point_sets");
		point_set=(struct GT_pointset *)NULL;
	}
	LEAVE;

	return (point_set);
} /* morph_GT_pointset */

struct GT_polyline *morph_GT_polyline(float proportion,
	struct GT_polyline *initial,struct GT_polyline *final)
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Creates a new GT_polyline which is the interpolation of two GT_polylines.
==============================================================================*/
{
	GTDATA *data;
	struct GT_polyline *polyline;
	int i,j,number_of_nodes;
	Triple *normallist,*point;

	ENTER(morph_GT_polyline);
	if ((initial->n_pts==final->n_pts)&&
		(initial->n_data_components==final->n_data_components)&&
		(initial->polyline_type==final->polyline_type)&&
		((initial->normallist&&final->normallist)||
		((!initial->normallist)&&(!final->normallist))))
	{
		normallist = (Triple *)NULL;
		switch (initial->polyline_type)
		{
			case g_PLAIN_DISCONTINUOUS:
			case g_NORMAL_DISCONTINUOUS:
			{
				number_of_nodes=2*(initial->n_pts);
			} break;
			case g_PLAIN:
			case g_NORMAL:
			{
				number_of_nodes=initial->n_pts;
			} break;
			default:
			{
				number_of_nodes=0;
				display_message(ERROR_MESSAGE,
					"morph_GT_polyline.  Unknown polyline type");
			} break;
		}
		if ((number_of_nodes>0)&&(ALLOCATE(point,Triple,number_of_nodes)))
		{
			for (i=0;i<number_of_nodes;i++)
			{
				for (j=0;j<3;j++)
				{
					point[i][j]=(1.0-proportion)*(initial->pointlist)[i][j]+
						proportion*(final->pointlist)[i][j];
				}
			}
			if (initial->normallist)
			{
				if (ALLOCATE(normallist,Triple,number_of_nodes))
				{
					for (i=0;i<number_of_nodes;i++)
					{
						for (j=0;j<3;j++)
						{
							normallist[i][j]=(1.0-proportion)*(initial->normallist)[i][j]+
								proportion*(final->normallist)[i][j];
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"morph_GT_pointset.  Could not allocate normals");
					DEALLOCATE(point);
				}				
			}
			if (point)
			{
				/* check for and allocate any data */
				if (initial->n_data_components)
				{
					if (ALLOCATE(data,GTDATA,number_of_nodes*initial->n_data_components))
					{
						for (i=0;i<number_of_nodes*initial->n_data_components;i++)
						{
							data[i]=(1.0-proportion)*(initial->data)[i]+
								proportion*(final->data)[i];
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"morph_GT_pointset.  Could not allocate data");
						DEALLOCATE(point);

					}
				}
				else
				{
					data=(GTDATA *)NULL;
				}
			}
			if (point)
			{
				if (polyline=CREATE(GT_polyline)(initial->polyline_type,
					initial->n_pts,point,normallist,
					initial->n_data_components,data))
				{
#if defined (OLD_CODE)
/*???DB.  Have to be recursive on destroy_ and morph_ or neither */
					/* go recursive in case it is a linked list */
					if (initial->ptrnext&&final->ptrnext)
					{
						if (!(polyline->ptrnext=morph_GT_polyline(proportion,
							initial->ptrnext,final->ptrnext)))
						{
							DEALLOCATE(polyline->pointlist);
							DEALLOCATE(polyline->data);
							DESTROY(GT_polyline)(&polyline);
						}
					}
#endif /* defined (OLD_CODE) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"morph_GT_polyline.  Could not create polyline");
					DEALLOCATE(data);
					DEALLOCATE(point);
					if (normallist)
					{
						DEALLOCATE(normallist);
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"morph_GT_polyline.  Could not allocate points");
			polyline=(struct GT_polyline *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"morph_GT_polyline.  Inconsistent initial and final polylines");
		polyline=(struct GT_polyline *)NULL;
	}
	LEAVE;

	return (polyline);
} /* morph_GT_polyline */

struct GT_surface *morph_GT_surface(float proportion,
	struct GT_surface *initial,struct GT_surface *final)
/*******************************************************************************
LAST MODIFIED : 14 August 1998

DESCRIPTION :
Creates a new GT_surface which is the interpolation of two GT_surfaces.
==============================================================================*/
{
	GTDATA *data;
	struct GT_surface *surface;
	int i,j,number_of_nodes;
	Triple *normallist,*point,*texturelist;

	ENTER(morph_GT_surface);
	if ((initial->n_pts1==final->n_pts1)&&(initial->n_pts2==final->n_pts2)&&
		(initial->n_data_components==final->n_data_components)&&
		(initial->surface_type==final->surface_type)&&
		(initial->polygon==final->polygon))
	{
		switch (initial->polygon)
		{
			case g_QUADRILATERAL:
			{
				number_of_nodes=(initial->n_pts1)*(initial->n_pts2);
			} break;
			case g_TRIANGLE:
			{
				number_of_nodes=((1+initial->n_pts1)*(initial->n_pts1))/2;
			} break;
			default:
			{
				number_of_nodes=0;
				display_message(ERROR_MESSAGE,
					"morph_GT_surface.  Unknown polygon type");
			} break;
		}
		switch (initial->surface_type)
		{
			case g_SH_DISCONTINUOUS:
			{
				number_of_nodes *= 2;
			} break;
			case g_SHADED:
			case g_SHADED_TEXMAP:
			{
				/* OK */
			} break;
			default:
			{
				number_of_nodes=0;
				display_message(ERROR_MESSAGE,
					"morph_GT_surface.  Unknown surface type");
			} break;
		}
		if ((number_of_nodes>0)&&ALLOCATE(point,Triple,number_of_nodes))
		{
			for (i=0;i<number_of_nodes;i++)
			{
				for (j=0;j<3;j++)
				{
					point[i][j]=(1.0-proportion)*(initial->pointlist)[i][j]+
						proportion*(final->pointlist)[i][j];
				}
			}
			if (initial->normallist)
			{
				if (ALLOCATE(normallist,Triple,number_of_nodes))
				{
					for (i=0;i<number_of_nodes;i++)
					{
						for (j=0;j<3;j++)
						{
							normallist[i][j]=(1.0-proportion)*(initial->normallist)[i][j]+
								proportion*(final->normallist)[i][j];
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"morph_GT_pointset.  Could not allocate normals");
					DEALLOCATE(point);
				}				
			}
			else
			{
				normallist = (Triple *)NULL;
			}
			if (point)
			{
				if (initial->texturelist)
				{
					if (ALLOCATE(texturelist,Triple,number_of_nodes))
					{
						for (i=0;i<number_of_nodes;i++)
						{
							for (j=0;j<3;j++)
							{
								texturelist[i][j]=(1.0-proportion)*(initial->texturelist)[i][j]+
									proportion*(final->texturelist)[i][j];
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"morph_GT_pointset.  Could not allocate normals");
						DEALLOCATE(point);
						if (normallist)
						{
							DEALLOCATE(normallist);
						}
					}				
				}
				else
				{
					texturelist = (Triple *)NULL;
				}
			}
			if (point)
			{
				/* check for and allocate any data */
				if (initial->n_data_components)
				{
					number_of_nodes=(initial->n_pts1)*(initial->n_pts2);
					if (ALLOCATE(data,GTDATA,number_of_nodes*initial->n_data_components))
					{
						for (i=0;i<number_of_nodes*initial->n_data_components;i++)
						{
							data[i]=(1.0-proportion)*(initial->data)[i]+
								proportion*(final->data)[i];
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"morph_GT_surface.  Could not allocate data");
						DEALLOCATE(point);
						if (normallist)
						{
							DEALLOCATE(normallist);
						}
						if (texturelist)
						{
							DEALLOCATE(texturelist);
						}
					}
				}
				else
				{
					data=(GTDATA *)NULL;
				}
			}
			if (point)
			{
				if (surface=CREATE(GT_surface)(initial->surface_type,initial->polygon,
					initial->n_pts1,initial->n_pts2,point,normallist,texturelist,
					initial->n_data_components,data))
				{
					/* go recursive in case it is a linked list */
					if (initial->ptrnext&&final->ptrnext)
					{
#if defined (OLD_CODE)
/*???DB.  Have to be recursive on DESTROY and morph_ or neither */
						if (!(surface->ptrnext=morph_GT_surface(proportion,
							initial->ptrnext,final->ptrnext)))
						{
							DEALLOCATE(surface->pointlist);
							DEALLOCATE(surface->data);
							DESTROY(GT_surface)(&surface);
						}
#endif /* defined (OLD_CODE) */
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"morph_GT_surface.  Could not create surface");
					DEALLOCATE(data);
					DEALLOCATE(point);
					if (normallist)
					{
						DEALLOCATE(normallist);
					}
					if (texturelist)
					{
						DEALLOCATE(texturelist);
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"morph_GT_surface.  Could not allocate points");
			surface=(struct GT_surface *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"morph_GT_surface.  Inconsistent initial and final surfaces");
		surface=(struct GT_surface *)NULL;
	}
	LEAVE;

	return (surface);
} /* morph_GT_surface */

gtObject *morph_gtObject(char *name,float proportion,gtObject *initial,
	gtObject *final)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Creates a new gtObject which is the interpolation of two gtObjects.
==============================================================================*/
{
	gtObject *graphics_object;
	struct GT_polyline **polyline_address,*polyline_final,*polyline_initial;
	struct GT_pointset **pointset_address,*pointset_final,*pointset_initial;
	struct GT_surface **surface_address,*surface_final,*surface_initial;

	ENTER(morph_gtObject);
	/* check arguments */
	if ((0<=proportion)&&(proportion<=1)&&initial&&final&&
		(initial->object_type==final->object_type)/*&&
		(initial->default_material==final->default_material)*/)
	{
#if defined (OLD_CODE)
		/*???SAB.  The matrix is no longer part of the graphics object so it isn't
			interpolated */
		/* interpolate matrix */
		for (i=0;i<4;i++)
		{
			for (j=0;j<4;j++)
			{
				transform_matrix[i][j]=(1.0-proportion)*(initial->transform)[i][j]+
					proportion*(final->transform)[i][j];
			}
		}
#endif /* defined (OLD_CODE) */
		if (graphics_object=CREATE(GT_object)(name,initial->object_type,
			initial->default_material))
		{
			switch (initial->object_type)
			{
				case g_POINTSET:
				{
					pointset_address=graphics_object->gu.gt_pointset;
					pointset_initial= *(initial->gu.gt_pointset);
					pointset_final= *(final->gu.gt_pointset);
					while (pointset_initial&&pointset_final&&
						(*pointset_address=morph_GT_pointset(proportion,pointset_initial,
						pointset_final)))
					{
						pointset_initial=pointset_initial->ptrnext;
						pointset_final=pointset_final->ptrnext;
						pointset_address= &((*pointset_address)->ptrnext);
					}
					if (pointset_initial||pointset_final)
					{
						DESTROY(GT_object)(&graphics_object);
					}
					else
					{
						graphics_object->number_of_times=1;
					}
				} break;
				case g_POLYLINE:
				{
					polyline_address=graphics_object->gu.gt_polyline;
					polyline_initial= *(initial->gu.gt_polyline);
					polyline_final= *(final->gu.gt_polyline);
					while (polyline_initial&&polyline_final&&
						(*polyline_address=morph_GT_polyline(proportion,polyline_initial,
						polyline_final)))
					{
						polyline_initial=polyline_initial->ptrnext;
						polyline_final=polyline_final->ptrnext;
						polyline_address= &((*polyline_address)->ptrnext);
					}
					if (polyline_initial||polyline_final)
					{
						DESTROY(GT_object)(&graphics_object);
					}
					else
					{
						graphics_object->number_of_times=1;
					}
				} break;
				case g_SURFACE:
				{
					surface_address=graphics_object->gu.gt_surface;
					surface_initial= *(initial->gu.gt_surface);
					surface_final= *(final->gu.gt_surface);
					while (surface_initial&&surface_final&&
						(*surface_address=morph_GT_surface(proportion,surface_initial,
						surface_final)))
					{
						surface_initial=surface_initial->ptrnext;
						surface_final=surface_final->ptrnext;
						surface_address= &((*surface_address)->ptrnext);
					}
					if (surface_initial||surface_final)
					{
						DESTROY(GT_object)(&graphics_object);
					}
					else
					{
						graphics_object->number_of_times=1;
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"morph_gtObject.  Invalid graphics element type");
					DESTROY(GT_object)(&graphics_object);
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"morph_gtObject.  Could not create gtObject");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"morph_gtObject.  Initial and final objects do not match");
		graphics_object=(gtObject *)NULL;
	}
	LEAVE;

	return (graphics_object);
} /* morph_gtObject */

struct GT_surface *transform_GT_surface(struct GT_surface *initial,
	float *transformation)
/*******************************************************************************
LAST MODIFIED : 8 July 1999

DESCRIPTION :
Creates a new GT_surface which is the interpolation of two GT_surfaces.
==============================================================================*/
{
	GTDATA *data;
	struct GT_surface *surface;
	int i,j,number_of_nodes;
	Triple *normallist,*point,*texturelist;

	ENTER(transform_GT_surface);
	switch (initial->polygon)
	{
		case g_QUADRILATERAL:
		{
			number_of_nodes=(initial->n_pts1)*(initial->n_pts2);
		} break;
		case g_TRIANGLE:
		{
			number_of_nodes=((1+initial->n_pts1)*(initial->n_pts1))/2;
		} break;
		default:
		{
			number_of_nodes=0;
			display_message(ERROR_MESSAGE,
				"transform_GT_surface.  Unknown polygon type");
		} break;
	}
	switch (initial->surface_type)
	{
		case g_SH_DISCONTINUOUS:
		{
			number_of_nodes *= 2;
		} break;
		case g_SHADED:
		case g_SHADED_TEXMAP:
		{
			/* OK */
		} break;
		default:
		{
			number_of_nodes=0;
			display_message(ERROR_MESSAGE,
				"transform_GT_surface.  Unknown surface type");
		} break;
	}
	if ((number_of_nodes>0)&&ALLOCATE(point,Triple,number_of_nodes))
	{
		for (i=0;i<number_of_nodes;i++)
		{
			for (j=0;j<3;j++)
			{
				point[i][j] = transformation[12 + j]
					+ transformation[0 + j] * initial->pointlist[i][0]
					+ transformation[4 + j] * initial->pointlist[i][1]
					+ transformation[8 + j] * initial->pointlist[i][2];
			}
		}
		if (initial->normallist)
		{
			if (ALLOCATE(normallist,Triple,number_of_nodes))
			{
				for (i=0;i<number_of_nodes;i++)
				{
					for (j=0;j<3;j++)
					{
						normallist[i][j]=initial->normallist[i][j];
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"transform_GT_pointset.  Could not allocate normals");
				DEALLOCATE(point);
			}
		}
	  	else
		{
			normallist = (Triple *)NULL;
		}
		if (point)
		{
			if (initial->texturelist)
			{
				if (ALLOCATE(texturelist,Triple,number_of_nodes))
				{
					for (i=0;i<number_of_nodes;i++)
					{
						for (j=0;j<3;j++)
						{
							texturelist[i][j]=initial->texturelist[i][j];
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"transform_GT_pointset.  Could not allocate normals");
					DEALLOCATE(point);
					if (normallist)
					{
						DEALLOCATE(normallist);
					}
				}				
			}
			else
			{
				texturelist = (Triple *)NULL;
			}
		}
		if (point)
		{
			/* check for and allocate any data */
			if (initial->n_data_components)
			{
				number_of_nodes=(initial->n_pts1)*(initial->n_pts2);
				if (ALLOCATE(data,GTDATA,number_of_nodes*initial->n_data_components))
				{
					for (i=0;i<number_of_nodes*initial->n_data_components;i++)
					{
						data[i]=initial->data[i];
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"transform_GT_surface.  Could not allocate data");
					DEALLOCATE(point);
					if (normallist)
					{
						DEALLOCATE(normallist);
					}
					if (texturelist)
					{
						DEALLOCATE(texturelist);
					}
					}
			}
			else
			{
				data=(GTDATA *)NULL;
			}
		}
		if (point)
		{
			if (surface=CREATE(GT_surface)(initial->surface_type,initial->polygon,
				initial->n_pts1,initial->n_pts2,point,normallist,texturelist,
				initial->n_data_components,data))
			{
				/* go recursive in case it is a linked list */
				if (initial->ptrnext)
				{
#if defined (OLD_CODE)
					/*???DB.  Have to be recursive on DESTROY and transform_ or neither */
					if (!(surface->ptrnext=transform_GT_surface(proportion,
						initial->ptrnext,final->ptrnext)))
					{
						DEALLOCATE(surface->pointlist);
						DEALLOCATE(surface->data);
						DESTROY(GT_surface)(&surface);
					}
#endif /* defined (OLD_CODE) */
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"transform_GT_surface.  Could not create surface");
				DEALLOCATE(data);
				DEALLOCATE(point);
				if (normallist)
				{
					DEALLOCATE(normallist);
				}
				if (texturelist)
				{
					DEALLOCATE(texturelist);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transform_GT_surface.  Could not allocate points");
		surface=(struct GT_surface *)NULL;
	}
	LEAVE;

	return (surface);
} /* transform_GT_surface */

struct GT_object *transform_GT_object(struct GT_object *object,
	float *transformation)
/*******************************************************************************
LAST MODIFIED : 8 July 1999

DESCRIPTION :
Creates a new GT_object which is the transformation of <object>.
Only surfaces are implemented at the moment.
Normals are not updated (wavefront export doesn't use normals anyway).
==============================================================================*/
{
	struct GT_object *graphics_object;
	struct GT_surface *surface_input;

	ENTER(transform_GT_object);
	/* check arguments */
	if (object)
	{
		if (graphics_object=CREATE(GT_object)(object->name,object->object_type,
			object->default_material))
		{
			switch (object->object_type)
			{
#if defined (OLD_CODE)
				/* To be done, wavefront files don't do points or lines anyway */
				case g_POINTSET:
				{
				} break;
				case g_POLYLINE:
				{
				} break;
#endif /* defined (OLD_CODE) */
				case g_SURFACE:
				{
					surface_input = *(object->gu.gt_surface);
					while (surface_input && GT_OBJECT_ADD(GT_surface)(
						graphics_object, /*time*/0.0,
						transform_GT_surface(surface_input,	transformation)))
					{
						surface_input=surface_input->ptrnext;
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"transform_GT_object.  Invalid graphics element type");
					DESTROY(GT_object)(&graphics_object);
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"transform_GT_object.  Could not create gtObject");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transform_GT_object.  Initial and final objects do not match");
		graphics_object=(gtObject *)NULL;
	}
	LEAVE;

	return (graphics_object);
} /* transform_GT_object */

DECLARE_OBJECT_FUNCTIONS(GT_object)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(GT_object)

DECLARE_INDEXED_LIST_FUNCTIONS(GT_object)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(GT_object,name,char *, \
	strcmp)

struct GT_glyph_set *CREATE(GT_glyph_set)(int number_of_points,
	Triple *point_list,Triple *axis1_list,Triple *axis2_list,Triple *axis3_list,
	struct GT_object *glyph,char **labels,int n_data_components,GTDATA *data,
	int object_name,int *names)
/*******************************************************************************
LAST MODIFIED : 23 February 2000

DESCRIPTION :
Allocates memory and assigns fields for a GT_glyph_set. The glyph set shows
the object <glyph> at the specified <number_of_points> with positions given in
<point_list>, and principal axes in <axis1_list>, <axis2_list> and <axis3_list>.
The magnitude of these axes control scaling of the glyph at each point, while
their orientations - which need not be orthogonal - effect rotations and skew.
The optional <labels> parameter is an array of strings to be written beside each
glyph, while the optional <data> of number <n_data_components> per glyph allows
colouring of the glyphs by a spectrum.
The glyph_set will be marked as coming from the <object_name>, and integer
identifier, while the optional <names> contains an integer identifier per point.

???RC All arrays passed to this routine are owned by the new GT_glyph_set
and are deallocated by its DESTROY function.
==============================================================================*/
{
	struct GT_glyph_set *glyph_set;

	ENTER(CREATE(GT_glyph_set));
	if ((0<number_of_points)&&point_list&&axis1_list&&axis2_list&&axis3_list&&
		glyph)
	{
		if (ALLOCATE(glyph_set,struct GT_glyph_set,1))
		{
			glyph_set->number_of_points=number_of_points;
			glyph_set->point_list=point_list;
			glyph_set->axis1_list=axis1_list;
			glyph_set->axis2_list=axis2_list;
			glyph_set->axis3_list=axis3_list;
			glyph_set->glyph=ACCESS(GT_object)(glyph);
			glyph_set->labels=labels;
			glyph_set->n_data_components=n_data_components;
			glyph_set->data=data;
			glyph_set->object_name=object_name;
			glyph_set->names=names;
			glyph_set->ptrnext=(struct GT_glyph_set *)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(GT_glyph_set).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(GT_glyph_set).  Invalid argument(s)");
		glyph_set=(struct GT_glyph_set *)NULL;
	}
	LEAVE;

	return (glyph_set);
} /* CREATE(GT_glyph_set) */

int DESTROY(GT_glyph_set)(struct GT_glyph_set **glyph_set)
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Frees the frees the memory for <**glyph_set> and sets <*glyph_set> to NULL.
==============================================================================*/
{
	char *label,**labels;
	int return_code,i;

	ENTER(DESTROY(GT_glyph_set));
	if (glyph_set && *glyph_set)
	{
		DEALLOCATE((*glyph_set)->point_list);
		DEALLOCATE((*glyph_set)->axis1_list);
		DEALLOCATE((*glyph_set)->axis2_list);
		DEALLOCATE((*glyph_set)->axis3_list);
		DEACCESS(GT_object)(&((*glyph_set)->glyph));
		if (labels=(*glyph_set)->labels)
		{
			for (i=(*glyph_set)->number_of_points;i>0;i--)
			{
				label= *labels;
				DEALLOCATE(label);
				labels++;
			}
			DEALLOCATE((*glyph_set)->labels);
		}
		if ((*glyph_set)->data)
		{
			DEALLOCATE((*glyph_set)->data);
		}
		if ((*glyph_set)->names)
		{
			DEALLOCATE((*glyph_set)->names);
		}
		DEALLOCATE(*glyph_set);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(GT_glyph_set).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(GT_glyph_set) */

struct GT_nurbs *CREATE(GT_nurbs)(void)
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Creates a complete GT_nurbs structure.
==============================================================================*/
{
	struct GT_nurbs *nurbs;

	ENTER(CREATE(GT_nurbs));
	if (ALLOCATE(nurbs,struct GT_nurbs,1))
	{
		nurbs->nurbsprop=50;
		nurbs->sorder=0;
		nurbs->torder=0;
		nurbs->corder=0;
		nurbs->sknotcnt=0;
		nurbs->tknotcnt=0;
		nurbs->cknotcnt=0;
		nurbs->ccount=0;
		nurbs->pwlcnt=0;
		nurbs->maxs=0;
		nurbs->maxt=0;
		nurbs->sknots=(double *)NULL;
		nurbs->tknots=(double *)NULL;
		nurbs->cknots=(double *)NULL;
		nurbs->controlpts=(double *)NULL;
		nurbs->normal_control_points=(double *)NULL;
		nurbs->texture_control_points=(double *)NULL;
		nurbs->trimarray=(double *)NULL;
		nurbs->pwlarray=(double *)NULL;

		nurbs->ptrnext=(struct GT_nurbs *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(GT_nurbs).  Not enough memory");
	}
	LEAVE;

	return (nurbs);
} /* CREATE(GT_nurbs) */

int DESTROY(GT_nurbs)(struct GT_nurbs **nurbs)
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Frees the memory for <**nurbs> and its fields and sets <*nurbs> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(GT_nurbs));
	if (nurbs)
	{
		if (*nurbs)
		{
			if((*nurbs)->sknots)
			{
				DEALLOCATE((*nurbs)->sknots);
			}
			if((*nurbs)->tknots)
			{
				DEALLOCATE((*nurbs)->tknots);
			}
			if((*nurbs)->controlpts)
			{
				DEALLOCATE((*nurbs)->controlpts);
			}
			if((*nurbs)->normal_control_points)
			{
				DEALLOCATE((*nurbs)->normal_control_points);
			}
			if((*nurbs)->texture_control_points)
			{
				DEALLOCATE((*nurbs)->texture_control_points);
			}
			if((*nurbs)->cknots)
			{
				DEALLOCATE((*nurbs)->cknots);
			}
			if((*nurbs)->trimarray)
			{
				DEALLOCATE((*nurbs)->trimarray);
			}
			if((*nurbs)->pwlarray)
			{
				DEALLOCATE((*nurbs)->pwlarray);
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(GT_nurbs).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(GT_nurbs) */

int GT_nurbs_set_surface(struct GT_nurbs *nurbs,
	int sorder, int torder, int sknotcount, int tknotcount,
	double *sknots, double *tknots, 
	int scontrolcount, int tcontrolcount, double *control_points)
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Sets the surface in a GT_nurbs structure.
There must be sknotcount values in sknots,
tknotcount values in tknots and scontrolcount * tcontrolcount values
in the controlpoints, with the s direction varying more quickly.
The arrays are assigned directly to the object and not copied.
==============================================================================*/
{
	int return_code;

	ENTER(GT_nurbs_set_surface);
	if (nurbs && (sorder > 0) && (torder > 0) &&
		(sknotcount > 0) && (tknotcount > 0) && sknots && tknots
		&& (scontrolcount > 0) && (tcontrolcount > 0) && control_points)
	{
		if(nurbs->sknots)
		{
			DEALLOCATE(nurbs->sknots);
		}
		if(nurbs->tknots)
		{
			DEALLOCATE(nurbs->tknots);
		}
		if(nurbs->controlpts)
		{
			DEALLOCATE(nurbs->controlpts);
		}
		nurbs->sorder = sorder;
		nurbs->torder = torder;
		nurbs->sknotcnt = sknotcount;
		nurbs->tknotcnt = tknotcount;
		nurbs->sknots = sknots;
		nurbs->tknots = tknots;
		nurbs->maxs = scontrolcount;
		nurbs->maxt = tcontrolcount;
		nurbs->controlpts = control_points;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_nurbs_set_surface.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_nurbs_set_surface */

int GT_nurbs_set_nurb_trim_curve(struct GT_nurbs *nurbs,
	int order, int knotcount, double *knots,
	int control_count, double *control_points)
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Sets a Nurb curve used to trim the Nurbs surface in a GT_nurbs structure.
The arrays are assigned directly to the object and not copied.
==============================================================================*/
{
	int return_code;

	ENTER(GT_nurbs_set_nurb_trim_curve);
	if (nurbs && (order > 0) && (knotcount > 0) && knots 
		&& (control_count > 0) && control_points)
	{
		if(nurbs->cknots)
		{
			DEALLOCATE(nurbs->cknots);
		}
		if(nurbs->trimarray)
		{
			DEALLOCATE(nurbs->trimarray);
		}
		nurbs->corder = order;
		nurbs->cknotcnt = knotcount;
		nurbs->cknots = knots;
		nurbs->ccount = control_count;
		nurbs->trimarray = control_points;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_nurbs_set_nurb_trim_curve.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_nurbs_set_nurb_trim_curve */

int GT_nurbs_set_piecewise_linear_trim_curve(struct GT_nurbs *nurbs,
	int number_of_points, double *points)
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Sets a piecewise linear curve used to trim the Nurbs surface in a GT_nurbs structure.
The array is assigned directly to the object and not copied.
==============================================================================*/
{
	int return_code;

	ENTER(GT_nurbs_set_piecewise_linear_trim_curve);
	if (nurbs && (number_of_points > 1) && points)
	{
		if(nurbs->pwlarray)
		{
			DEALLOCATE(nurbs->pwlarray);
		}
		nurbs->pwlcnt = number_of_points;
		nurbs->pwlarray = points;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_nurbs_set_nurb_trim_curve.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_nurbs_set_piecewise_linear_trim_curve */

int GT_nurbs_set_normal_control_points(struct GT_nurbs *nurbs,
	double *normal_control_points)
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Sets the array for the normals.  The array is assigned directly and
the number of normals is assumed to be the same as the geometry control
points specified in set surface.  Each normal is assumed to have three components.
==============================================================================*/
{
	int return_code;

	ENTER(GT_nurbs_set_normal_control_points);
	if (nurbs)
	{
		if(nurbs->normal_control_points)
		{
			DEALLOCATE(nurbs->normal_control_points);
		}
		nurbs->normal_control_points = normal_control_points;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_nurbs_set_normal_control_points.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_nurbs_set_normal_control_points */

int GT_nurbs_set_texture_control_points(struct GT_nurbs *nurbs,
	double *texture_control_points)
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Sets the array for the texture.  The array is assigned directly and
the number of points is assumed to be the same as the geometry control
points specified in set surface.  Each point is assumed to have three
texture coordinates.
==============================================================================*/
{
	int return_code;

	ENTER(GT_nurbs_set_type);
	if (nurbs)
	{
		if(nurbs->texture_control_points)
		{
			DEALLOCATE(nurbs->texture_control_points);
		}
		nurbs->texture_control_points = texture_control_points;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_nurbs_set_texture_control_points.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_nurbs_set_texture_control_points */

struct GT_point *CREATE(GT_point)(Triple *position,char *text,
	gtMarkerType marker_type,float marker_size,int n_data_components,GTDATA *data)
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Allocates memory and assigns fields for a GT_point.  When the <marker_type> is
g_DERIVATIVE_MARKER, there should be 4 points in <pointlist> - first point is
for the <node>, next point is the end point for the xi1 derivative axis, etc.
If the end point is the same as the node point it is assumed that there isn't a
derivative in that xi direction.
==============================================================================*/
{
	struct GT_point *point;

	ENTER(CREATE(GT_point));
	if (position)
	{
		if (ALLOCATE(point,struct GT_point,1))
		{
			point->position=position;
			point->text=text;
			point->marker_type=marker_type;
			point->marker_size=marker_size;
			point->n_data_components=n_data_components;
			point->data=data;
			point->ptrnext=(struct GT_point *)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(GT_point).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(GT_point).  Invalid argument");
		point=(struct GT_point *)NULL;
	}
	LEAVE;

	return (point);
} /* CREATE(GT_point) */

int DESTROY(GT_point)(struct GT_point **point)
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Frees the frees the memory for <**point> and sets <*point> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(GT_point));
	if (point)
	{
		if (*point)
		{
			DEALLOCATE((*point)->position);
			if ((*point)->text)
			{
				DEALLOCATE((*point)->text);
			}
			if ((*point)->data)
			{
				DEALLOCATE((*point)->data);
			}
			DEALLOCATE(*point);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(GT_point).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(GT_point) */

struct GT_pointset *CREATE(GT_pointset)(int n_pts,Triple *pointlist,char **text,
	gtMarkerType marker_type,float marker_size,int n_data_components,GTDATA *data,
	int *names)
/*******************************************************************************
LAST MODIFIED : 1 June 1999

DESCRIPTION :
Allocates memory and assigns fields for a GT_pointset.  When the <marker_type>
is g_DERIVATIVE_MARKER, there should be 4*<n_pts> points in <pointlist> - in
each group of four points the first for the nodes, the next is the end points
for the xi1 derivative axis, etc.  If the end point is the same as the node
point it is assumed that there isn't a derivative in that xi direction.
==============================================================================*/
{
	struct GT_pointset *point_set;

	ENTER(CREATE(GT_pointset));
	if (pointlist)
	{
		if (ALLOCATE(point_set,struct GT_pointset,1))
		{
			point_set->n_pts=n_pts;
			point_set->pointlist=pointlist;
			point_set->text=text;
			point_set->marker_type=marker_type;
			point_set->marker_size=marker_size;
			point_set->n_data_components=n_data_components;
			point_set->data=data;
			point_set->ptrnext=(struct GT_pointset *)NULL;
			point_set->names=names;
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(GT_pointset).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(GT_pointset).  Invalid argument");
		point_set=(struct GT_pointset *)NULL;
	}
	LEAVE;

	return (point_set);
} /* CREATE(GT_pointset) */

int DESTROY(GT_pointset)(struct GT_pointset **pointset)
/*******************************************************************************
LAST MODIFIED : 16 April 1999

DESCRIPTION :
Frees the frees the memory for <**pointset> and sets <*pointset> to NULL.
==============================================================================*/
{
	char **text,*text_string;
	int i,return_code;

	ENTER(DESTROY(GT_pointset));
	if (pointset)
	{
		if (*pointset)
		{
			DEALLOCATE((*pointset)->pointlist);
			DEALLOCATE((*pointset)->data);
			if (text=(*pointset)->text)
			{
				for (i=(*pointset)->n_pts;i>0;i--)
				{
					text_string= *text;
					DEALLOCATE(text_string);
					text++;
				}
				DEALLOCATE((*pointset)->text);
			}
			if ((*pointset)->names)
			{
				DEALLOCATE((*pointset)->names);
			}
			DEALLOCATE(*pointset);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(GT_pointset).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(GT_pointset) */

struct GT_polyline *CREATE(GT_polyline)(enum GT_polyline_type polyline_type,
	int n_pts,Triple *pointlist,Triple *normallist,int n_data_components,GTDATA *data)
/*******************************************************************************
LAST MODIFIED : 31 May 1999

DESCRIPTION :
Allocates memory and assigns fields for a graphics polyline.
==============================================================================*/
{
	struct GT_polyline *polyline;

	ENTER(CREATE(GT_polyline));
	if (ALLOCATE(polyline,struct GT_polyline,1))
	{
		polyline->polyline_type=polyline_type;
		polyline->n_pts=n_pts;
		polyline->pointlist=pointlist;
		polyline->normallist=normallist;
		polyline->n_data_components=n_data_components;
		polyline->data=data;
		polyline->object_name=0;
		polyline->ptrnext=(struct GT_polyline *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(GT_polyline).  Not enough memory");
	}
	LEAVE;

	return (polyline);
} /* CREATE(GT_polyline) */

int DESTROY(GT_polyline)(struct GT_polyline **polyline)
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Frees the memory for <**polyline> and its fields and sets <*polyline> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(GT_polyline));
	if (polyline)
	{
		if (*polyline)
		{
			DEALLOCATE((*polyline)->pointlist);
			if ((*polyline)->normallist)
			{
				DEALLOCATE((*polyline)->normallist);
			}
			if ((*polyline)->data)
			{
				DEALLOCATE((*polyline)->data);
			}
			DEALLOCATE(*polyline);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(GT_polyline).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(GT_polyline) */

struct GT_surface *CREATE(GT_surface)(enum GT_surface_type surface_type,
	gtPolygonType polytype,int n_pts1,int n_pts2,Triple *pointlist,
	Triple *normallist, Triple *texturelist,
	int n_data_components,GTDATA *data)
/*******************************************************************************
LAST MODIFIED : 31 May 1999

DESCRIPTION :
Allocates memory and assigns fields for a graphics surface.
==============================================================================*/
{
	struct GT_surface *surface;

	ENTER(CREATE(GT_surface));
	if (ALLOCATE(surface,struct GT_surface,1))
	{
		surface->surface_type=surface_type;
		surface->polygon=polytype;
		surface->n_pts1=n_pts1;
		surface->n_pts2=n_pts2;
		surface->pointlist=pointlist;
		surface->normallist=normallist;
		surface->texturelist=texturelist;
		surface->n_data_components=n_data_components;
		surface->data=data;
		surface->object_name=0;
		surface->ptrnext=(struct GT_surface *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(GT_surface).  Not enough memory");
	}
	LEAVE;

	return (surface);
} /* CREATE(GT_surface) */

int DESTROY(GT_surface)(struct GT_surface **surface)
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Frees the memory for <**surface> and sets <*surface> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(GT_surface));
	if (surface)
	{
		if (*surface)
		{
			DEALLOCATE((*surface)->pointlist);
			if ((*surface)->normallist)
			{
				DEALLOCATE((*surface)->normallist);
			}
			if ((*surface)->texturelist)
			{
				DEALLOCATE((*surface)->texturelist);
			}
			if ((*surface)->data)
			{
				DEALLOCATE((*surface)->data);
			}
			DEALLOCATE(*surface);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(GT_surface).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(GT_surface) */

struct GT_userdef *CREATE(GT_userdef)(void *data,
	int (*destroy_function)(void **),int (*render_function)(void *))
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Allocates memory and assigns fields for a user-defined primitive.
Any data required for rendering the primitive should be passed in the
void *data parameter; a destroy_function should be given if dynamically
allocated data is passed. The render function is called with the data as a
parameter to render the user-defined primitive.
==============================================================================*/
{
	struct GT_userdef *userdef;

	ENTER(CREATE(GT_userdef));
	if (render_function)
	{
		if (ALLOCATE(userdef,struct GT_userdef,1))
		{
			userdef->data=data;
			userdef->destroy_function=destroy_function;
			userdef->render_function=render_function;
			userdef->ptrnext=(struct GT_userdef *)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(GT_userdef).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(GT_userdef).  No render_function");
		userdef=(struct GT_userdef *)NULL;
	}
	LEAVE;

	return (userdef);
} /* CREATE(GT_userdef) */

int DESTROY(GT_userdef)(struct GT_userdef **userdef)
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Frees the memory for <**userdef> and its fields and sets <*userdef> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(GT_userdef));
	if (userdef)
	{
		if (*userdef)
		{
			if (((*userdef)->data)&&((*userdef)->destroy_function))
			{
				(*userdef)->destroy_function(&((*userdef)->data));
			}
			DEALLOCATE(*userdef);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(GT_userdef).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(GT_userdef) */

struct GT_voltex *CREATE(GT_voltex)(int n_iso_polys,int n_vertices,
	int *triangle_list,struct VT_iso_vertex *vertex_list,
	struct Graphical_material **iso_poly_material,
	struct Environment_map **iso_env_map, double *iso_poly_cop,
	float *texturemap_coord,int *texturemap_index,int n_rep,
	int n_data_components, GTDATA *data)
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Allocates memory and assigns fields for a graphics volume texture.
==============================================================================*/
{
	struct GT_voltex *voltex;

	ENTER(CREATE(GT_voltex));
#if defined (DEBUG)
	/*???debug */
	printf("%d %p %p %p\n",n_iso_polys,triangle_list,iso_poly_material,
		vertex_list);
#endif /* defined (DEBUG) */
#if defined (OLD_CODE)
	/*???MS.  Allow creation of empty voltexs in volume_texture_editor.c */
  if ((n_iso_polys>0)&&triangle_list&&iso_poly_material&&iso_env_map&&
		vertex_list&&iso_poly_cop&&texturemap_coord&&texturemap_index)
	{
#endif /* defined (OLD_CODE) */
		if (ALLOCATE(voltex,struct GT_voltex,1))
		{
			voltex->n_iso_polys=n_iso_polys;
			voltex->n_vertices=n_vertices;
			voltex->n_rep=n_rep;
			voltex->triangle_list=triangle_list;
			voltex->vertex_list=vertex_list;
			voltex->iso_poly_material=iso_poly_material;
			voltex->iso_env_map=iso_env_map;
			voltex->iso_poly_cop=iso_poly_cop;
			voltex->texturemap_coord=texturemap_coord;
			voltex->texturemap_index=texturemap_index;
			voltex->object_name=0;
			voltex->ptrnext=(struct GT_voltex *)NULL;
			voltex->n_data_components=n_data_components;
			voltex->data=data;
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(GT_voltex).  Insufficient memory");
		}
#if defined (OLD_CODE)
	}
  else
	{
		display_message(ERROR_MESSAGE,"CREATE(GT_voltex).  Invalid argument(s)");
		voltex=(struct GT_voltex *)NULL;
	}
#endif /* defined (OLD_CODE) */
	LEAVE;

	return (voltex);
} /* CREATE(GT_voltex) */

int DESTROY(GT_voltex)(struct GT_voltex **voltex)
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Frees the memory for <**voltex> and sets <*voltex> to NULL.
???DB.  Free memory for fields ?
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(GT_voltex));
	if (voltex)
	{
		if (*voltex)
		{
			DEALLOCATE((*voltex)->triangle_list);
			DEALLOCATE((*voltex)->vertex_list);
			DEALLOCATE((*voltex)->texturemap_coord);
			DEALLOCATE((*voltex)->texturemap_index);
			DEALLOCATE((*voltex)->iso_poly_material);
			/*???DB.  Problem with memory leak ? */
			DEALLOCATE((*voltex)->iso_poly_cop);
			DEALLOCATE((*voltex)->iso_env_map);
			if((*voltex)->data)
			{
				DEALLOCATE((*voltex)->data);
			}
			DEALLOCATE(*voltex);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(GT_voltex).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(GT_voltex) */

int update_GT_voltex_materials_to_default(struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 1 February 2000

DESCRIPTION :
Voltexes differ from other graphics objects in that their materials are
specified for each polygon inside the primitive. Since they most often have the
same material throughout, this function is provided to copy the default_material
from <graphics_object> to the iso_poly_material array in the voltex. Of course,
<graphics_object> must be of type g_VOLTEX.
==============================================================================*/
{
	int i,j,return_code;
	struct GT_voltex *voltex;
	struct Graphical_material *default_material,**iso_poly_material;

	ENTER(update_GT_voltex_materials_to_default);
	if (graphics_object&&(g_VOLTEX==graphics_object->object_type)&&
		(graphics_object->gu.gt_voltex)&&
		(default_material=graphics_object->default_material))
	{
		for (j=0;j<graphics_object->number_of_times;j++)
		{
			voltex=graphics_object->gu.gt_voltex[j];
			while (voltex)
			{
				if (iso_poly_material=voltex->iso_poly_material)
				{
					i=3*voltex->n_iso_polys;
					while (i>0)
					{
						*iso_poly_material = default_material;
						iso_poly_material++;
						i--;
					}
				}
				voltex=voltex->ptrnext;
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_GT_voltex_materials_to_default.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* update_GT_voltex_materials_to_default */

struct GT_object *CREATE(GT_object)(char *name,enum GT_object_type object_type,
	struct Graphical_material *default_material)
/*******************************************************************************
LAST MODIFIED : 20 February 2000

DESCRIPTION :
Allocates memory and assigns fields for a graphics object.
==============================================================================*/
{
	struct GT_object *object;
	int return_code;

	ENTER(CREATE(GT_object));
	if (name)
	{
		if (ALLOCATE(object,gtObject,1)&&ALLOCATE(object->name,char,strlen(name)+1)
			&&(object->selected_graphic_list=CREATE(LIST(Selected_graphic))()))
		{
			object->times=(float *)NULL;
			object->access_count=0;
			strcpy(object->name,name);
			object->update_callback_list=(struct Graphics_object_callback_data *)NULL;
			return_code=1;
			switch (object_type)
			{
				case g_GLYPH_SET:
				{
					object->gu.gt_glyph_set=(struct GT_glyph_set **)NULL;
				} break;
				case g_NURBS:
				{
					object->gu.gt_nurbs=(struct GT_nurbs **)NULL;
				} break;
				case g_POINT:
				{
					object->gu.gt_point=(struct GT_point **)NULL;
				} break;
				case g_POINTSET:
				{
					object->gu.gt_pointset=(struct GT_pointset **)NULL;
				} break;
				case g_POLYLINE:
				{
					object->gu.gt_polyline=(struct GT_polyline **)NULL;
				} break;
				case g_SURFACE:
				{
					object->gu.gt_surface=(struct GT_surface **)NULL;
				} break;
				case g_USERDEF:
				{
					object->gu.gt_userdef=(struct GT_userdef **)NULL;
				} break;
				case g_VOLTEX:
				{
					object->gu.gt_voltex=(struct GT_voltex **)NULL;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"CREATE(GT_object).  Unknown object type");
					return_code= -1;
				} break;
			}
			if (0<return_code)
			{
#if defined (OPENGL_API)
				object->display_list = 0;
#endif /* defined (OPENGL_API) */
				object->display_list_current = 0;
				object->object_type=object_type;
				if (default_material)
				{
					object->default_material=ACCESS(Graphical_material)(default_material);
				}
				else
				{
					object->default_material=(struct Graphical_material *)NULL;
				}
				object->selected_material=(struct Graphical_material *)NULL;
				object->nextobject=(gtObject *)NULL;
				object->spectrum=(struct Spectrum *)NULL;
				object->number_of_times=0;
			}
			else
			{
				if (0==return_code)
				{
					display_message(ERROR_MESSAGE,
						"CREATE(GT_object).  Insufficient memory");
				}
				DESTROY(LIST(Selected_graphic))(&(object->selected_graphic_list));
				DEALLOCATE(object->name);
				DEALLOCATE(object);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(GT_object).  Insufficient memory");
			if (object)
			{
				if (object->name)
				{
					DEALLOCATE(object->name);
				}
				DEALLOCATE(object);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(GT_object).  Invalid argument(s)");
		object=(gtObject *)NULL;
	}
	LEAVE;

	return (object);
} /* CREATE(GT_object) */

int DESTROY(GT_object)(struct GT_object **object_ptr)
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Frees the memory for the fields of <**object>, frees the memory for <**object>
and sets <*object> to NULL.
==============================================================================*/
{
	int i,return_code;
	struct Graphics_object_callback_data *callback_data,*next;
	struct GT_object *object;

	ENTER(DESTROY(GT_object));
	if (object_ptr&&(object= *object_ptr))
	{
		if (0!=object->access_count)
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(GT_object).  Access count = %d",object->access_count);
			return_code=0;
		}
		else
		{
			/* clean up times and primitives */
			DESTROY(LIST(Selected_graphic))(&(object->selected_graphic_list));
			for (i=object->number_of_times;0<i;i--)
			{
				GT_object_delete_time_number(object,i);
			}
			DEALLOCATE(object->name);
			DEACCESS(Graphical_material)(&(object->default_material));
			DEACCESS(Spectrum)(&(object->spectrum));
			callback_data = object->update_callback_list;
			while(callback_data)
			{
				next = callback_data->next;
				DEALLOCATE(callback_data);
				callback_data = next;
			}
#if defined (OPENGL_API)
			if (object->display_list)
			{
				glDeleteLists(object->display_list,1);
			}
#endif /* defined (OPENGL_API) */
			/* DEACCESS ptrnext so that objects attached in linked-list may be
				 destroyed. Note that this means they should have been accessed! */
			if (object->nextobject)
			{
				DEACCESS(GT_object)(&(object->nextobject));
			}
			DEALLOCATE(*object_ptr);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(GT_object).  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(GT_object) */

int compile_GT_object(struct GT_object *graphics_object_list,void *time_void)
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Rebuilds the display list for each uncreated or morphing graphics object in the
<graphics_object_list>, a simple linked list. The object is compiled at the time
pointed to by <time_void>.
==============================================================================*/
{
	float *time;
	int return_code;
	struct GT_object *graphics_object;

	ENTER(compile_GT_object);
	if (graphics_object_list&&(time = (float *)time_void))
	{
		return_code=1;
		for (graphics_object=graphics_object_list;graphics_object != NULL;
			graphics_object=graphics_object->nextobject)
		{
			if (!(graphics_object->display_list_current))
			{
				if (graphics_object->display_list ||
					(graphics_object->display_list=glGenLists(1)))
				{
					glNewList(graphics_object->display_list,GL_COMPILE);
					if (FIRST_OBJECT_IN_LIST_THAT(Selected_graphic)(
						(LIST_CONDITIONAL_FUNCTION(Selected_graphic) *)NULL,(void *)NULL,
						graphics_object->selected_graphic_list))
					{
						/* child objects have no material or transformation */
						if (graphics_object->selected_material)
						{
							execute_Graphical_material(graphics_object->selected_material);
							makegtobject(graphics_object,*time,1);
						}
						else
						{
							display_message(ERROR_MESSAGE,"compile_GT_object.  "
								"Graphics object %s has selected graphics but no selected "
								"material",graphics_object->name);
						}
					}
					/* child objects have no material or transformation */
					if (graphics_object->default_material)
					{
						execute_Graphical_material(graphics_object->default_material);
					}
					makegtobject(graphics_object,*time,0);
					glEndList();
					graphics_object->display_list_current = 1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"compile_GT_object.  Unable to get display list");
					return_code=0;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"compile_GT_object.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* compile_GT_object */

int execute_GT_object(struct GT_object *graphics_object_list,void *time_void)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Rebuilds the display list for each uncreated or morphing graphics object in the
<graphics_object_list>, a simple linked list.
If the linked list has more than one graphics_object in it, the number of the
graphics object, starting at zero for the first
==============================================================================*/
{
	int graphics_object_no,return_code;
	struct GT_object *graphics_object;

	ENTER(execute_GT_object);
	if (graphics_object_list && time_void)
	{
		return_code=1;
		if (graphics_object_list->nextobject)
		{
			glPushName(-1);
		}
		graphics_object_no=0;
		for (graphics_object=graphics_object_list;graphics_object != NULL;
			graphics_object=graphics_object->nextobject)
		{
			if (0<graphics_object_no)
			{
				glLoadName(graphics_object_no);
			}
			graphics_object_no++;
			if (graphics_object->display_list_current)
			{
				/* construct object */
				glCallList(graphics_object->display_list);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_GT_object.  Display list not current");
				return_code=0;
			}
		}
		if (graphics_object_list->nextobject)
		{
			glPopName();
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_GT_object.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_GT_object */

int GT_object_changed(struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
External modules that change a GT_object should call this routine so that
objects interested in this GT_object will be notified that is has changed.
==============================================================================*/
{
	int return_code;
	struct Graphics_object_callback_data *callback_data;

	ENTER(GT_object_changed);

	if (graphics_object)
	{
		graphics_object->display_list_current = 0;
		callback_data = graphics_object->update_callback_list;
		while(callback_data)
		{
			(callback_data->callback)(graphics_object,
				callback_data->callback_user_data);
			callback_data = callback_data->next;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_changed. Invalid graphics object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_changed */

int GT_object_add_callback(struct GT_object *graphics_object, 
	Graphics_object_callback callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Adds a callback routine which is called whenever a GT_object is aware of
changes.  As the GT_object is not private, this relies on modules that change a
GT_object calling GT_object_changed.
==============================================================================*/
{
	int return_code;
	struct Graphics_object_callback_data *callback_data, *previous;

	ENTER(GT_object_add_callback);

	if (graphics_object && callback)
	{
		if(ALLOCATE(callback_data, struct Graphics_object_callback_data, 1))
		{
			callback_data->callback = callback;
			callback_data->callback_user_data = user_data;
			callback_data->next = (struct Graphics_object_callback_data *)NULL;
			if(graphics_object->update_callback_list)
			{
				previous = graphics_object->update_callback_list;
				while(previous->next)
				{
					previous = previous->next;
				}
				previous->next = callback_data;
			}
			else
			{
				graphics_object->update_callback_list = callback_data;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"GT_object_add_callback.  Unable to allocate callback data structure");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_add_callback.  Missing graphics_object object or callback");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_add_callback */

int GT_object_remove_callback(struct GT_object *graphics_object,
	Graphics_object_callback callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Removes a callback which was added previously
==============================================================================*/
{
	int return_code;
	struct Graphics_object_callback_data *callback_data, *previous;

	ENTER(GT_object_remove_callback);

	if (graphics_object && callback && graphics_object->update_callback_list)
	{
		callback_data = graphics_object->update_callback_list;
		if((callback_data->callback == callback)
			&& (callback_data->callback_user_data == user_data))
		{
			graphics_object->update_callback_list = callback_data->next;
			DEALLOCATE(callback_data);
			return_code = 1;
		}
		else
		{
			return_code = 0;
			while(!return_code && callback_data->next)
			{
				previous = callback_data;
				callback_data = callback_data->next;
				if((callback_data->callback == callback)
					&& (callback_data->callback_user_data == user_data))
				{
					previous->next = callback_data->next;
					DEALLOCATE(callback_data);
					return_code = 1;		
				}
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
"GT_object_remove_callback.  Unable to find callback and user_data specified");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"GT_object_remove_callback.  Missing graphics_object, callback or callback list");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_remove_callback */

int GT_object_has_time(struct GT_object *graphics_object,float time)
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Returns 1 if the time parameter is used by the graphics_object.
==============================================================================*/
{
	int return_code;

	ENTER(GT_object_has_time);
	/* check arguments */
	if (graphics_object)
	{
		return_code=(0<GT_object_get_time_number(graphics_object,time));
	}
	else
	{
		display_message(ERROR_MESSAGE,"GT_object_has_time.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_has_time */

int GT_object_get_number_of_times(struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 18 June 1998

DESCRIPTION :
Returns the number of times/primitive lists in the graphics_object.
==============================================================================*/
{
	int number_of_times;

	ENTER(GT_object_get_number_of_times);
	/* check arguments */
	if (graphics_object)
	{
		number_of_times=graphics_object->number_of_times;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_get_number_of_times.  Invalid arguments");
		number_of_times=0;
	}
	LEAVE;

	return (number_of_times);
} /* GT_object_get_number_of_times */

float GT_object_get_time(struct GT_object *graphics_object,int time_no)
/*******************************************************************************
LAST MODIFIED : 18 June 1998

DESCRIPTION :
Returns the time at <time_no> from the graphics_object.
Note that time numbers range from 1 to number_of_times.
==============================================================================*/
{
	float return_time,*times;

	ENTER(GT_object_get_time);
	if (graphics_object)
	{
		if ((0 <= time_no)&&(time_no < graphics_object->number_of_times))
		{
			if (times=graphics_object->times)
			{
				return_time=times[time_no-1];
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"GT_object_get_time.  Invalid times array");
				return_time=0.0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"GT_object_get_time.  Time number out of range");
/*???debug */
printf("GT_object_get_time.  Time number out of range in GT_object %s\n",
	graphics_object->name);
			return_time=0.0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_get_time.  Invalid argument(s)");
		return_time=0.0;
	}
	LEAVE;

	return (return_time);
} /* GT_object_get_time */

float GT_object_get_nearest_time(struct GT_object *graphics_object,float time)
/*******************************************************************************
LAST MODIFIED : 7 July 1998

DESCRIPTION :
Returns the nearest time to <time> in <graphics_object> at which graphics
primitives are called.
NOTE: presently finds the nearest time that is *lower* than <time>. When all
routines updated to use this, may be changed to get actual nearest time.
???RC modularise; get it to call a routine get_nearest time_number
???RC need another version to handle morphing/proportions?
==============================================================================*/
{
	float return_time,*times;
	int time_no;

	ENTER(GT_object_get_nearest_time);
	if (graphics_object)
	{
		if (0<(time_no=graphics_object->number_of_times))
		{
			if (times=graphics_object->times)
			{
				time_no--;
				times += time_no;
				while ((time_no>0)&&(time< *times))
				{
					times--;
					time_no--;
				}
				return_time=times[time_no];
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"GT_object_get_nearest_time.  Invalid times array");
				return_time=0.0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"GT_object_get_nearest_time.  No times defined for graphics object");
			return_time=0.0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_get_nearest_time.  Invalid argument(s)");
		return_time=0.0;
	}
	LEAVE;

	return (return_time);
} /* GT_object_get_nearest_time */

int get_graphics_object_range(struct GT_object *graphics_object,
	void *graphics_object_range_void)
/*******************************************************************************
LAST MODIFIED : 6 September 1999

DESCRIPTION :
Returns the range of the coordinates in <graphics_object> or 0 if object empty
or error occurs. First should be set to 1 outside routine. Several calls to
this routine for differing graphics objects (without settings first in between)
will produce the range of all the graphics objects.
???RC only does some object types.
???RC this could be cleaned up by getting primitives to find their own range.
==============================================================================*/
{
	float temp,*maximum,*minimum;
	int *first,number_of_positions,i,number_of_times,position_offset,return_code;
	struct Graphics_object_range_struct *graphics_object_range;
	struct GT_glyph_set *glyph_set,**glyph_set_ptr;
	struct GT_point *point,**point_ptr;
	struct GT_pointset *pointset,**pointset_ptr;
	struct GT_polyline *line,**line_ptr;
	struct GT_surface *surface,**surface_ptr;
	struct GT_voltex *voltex,**voltex_ptr;
	Triple *position;

	ENTER(get_graphics_object_range);
	if (graphics_object&&(graphics_object_range=
		(struct Graphics_object_range_struct *)graphics_object_range_void)&&
		(maximum=graphics_object_range->maximum)&&
		(minimum=graphics_object_range->minimum)&&
		(first=&(graphics_object_range->first)))
	{
		switch (graphics_object->object_type)
		{
			case g_POINT:
			{
				if ((point_ptr=(graphics_object->gu).gt_point)&&
					(0<(number_of_times=graphics_object->number_of_times)))
				{
					while (number_of_times>0)
					{
						point= *point_ptr;
						while (point)
						{
							number_of_positions=1;
							if ((position=point->position)&&(0<number_of_positions))
							{
								if (*first)
								{
									minimum[0]=(*position)[0];
									maximum[0]=minimum[0];
									minimum[1]=(*position)[1];
									maximum[1]=minimum[1];
									minimum[2]=(*position)[2];
									maximum[2]=minimum[2];
									number_of_positions--;
									position++;
									*first=0;
								}
								while (number_of_positions>0)
								{
									temp=(*position)[0];
									if (temp<minimum[0])
									{
										minimum[0]=temp;
									}
									else
									{
										if (temp>maximum[0])
										{
											maximum[0]=temp;
										}
									}
									temp=(*position)[1];
									if (temp<minimum[1])
									{
										minimum[1]=temp;
									}
									else
									{
										if (temp>maximum[1])
										{
											maximum[1]=temp;
										}
									}
									temp=(*position)[2];
									if (temp<minimum[2])
									{
										minimum[2]=temp;
									}
									else
									{
										if (temp>maximum[2])
										{
											maximum[2]=temp;
										}
									}
									number_of_positions--;
									position++;
								}
							}
							point=point->ptrnext;
						}
						point_ptr++;
						number_of_times--;
					}
				}
			} break;
			case g_GLYPH_SET:
			{
				if ((glyph_set_ptr=(graphics_object->gu).gt_glyph_set)&&
					(0<(number_of_times=graphics_object->number_of_times)))
				{
					while (number_of_times>0)
					{
						glyph_set= *glyph_set_ptr;
						while (glyph_set)
						{
							number_of_positions=glyph_set->number_of_points;
							if ((position=glyph_set->point_list)&&(0<number_of_positions))
							{
								if (*first)
								{
									minimum[0]=(*position)[0];
									maximum[0]=minimum[0];
									minimum[1]=(*position)[1];
									maximum[1]=minimum[1];
									minimum[2]=(*position)[2];
									maximum[2]=minimum[2];
									number_of_positions--;
									position++;
									*first=0;
								}
								while (number_of_positions>0)
								{
									temp=(*position)[0];
									if (temp<minimum[0])
									{
										minimum[0]=temp;
									}
									else
									{
										if (temp>maximum[0])
										{
											maximum[0]=temp;
										}
									}
									temp=(*position)[1];
									if (temp<minimum[1])
									{
										minimum[1]=temp;
									}
									else
									{
										if (temp>maximum[1])
										{
											maximum[1]=temp;
										}
									}
									temp=(*position)[2];
									if (temp<minimum[2])
									{
										minimum[2]=temp;
									}
									else
									{
										if (temp>maximum[2])
										{
											maximum[2]=temp;
										}
									}
									number_of_positions--;
									position++;
								}
							}
							glyph_set=glyph_set->ptrnext;
						}
						glyph_set_ptr++;
						number_of_times--;
					}
				}
			} break;
			case g_POINTSET:
			{
				if ((pointset_ptr=(graphics_object->gu).gt_pointset)&&
					(0<(number_of_times=graphics_object->number_of_times)))
				{
					while (number_of_times>0)
					{
						pointset= *pointset_ptr;
						while (pointset)
						{
							number_of_positions=pointset->n_pts;
							if ((position=pointset->pointlist)&&(0<number_of_positions))
							{
								if (*first)
								{
									minimum[0]=(*position)[0];
									maximum[0]=minimum[0];
									minimum[1]=(*position)[1];
									maximum[1]=minimum[1];
									minimum[2]=(*position)[2];
									maximum[2]=minimum[2];
									number_of_positions--;
									position++;
									*first=0;
								}
								while (number_of_positions>0)
								{
									temp=(*position)[0];
									if (temp<minimum[0])
									{
										minimum[0]=temp;
									}
									else
									{
										if (temp>maximum[0])
										{
											maximum[0]=temp;
										}
									}
									temp=(*position)[1];
									if (temp<minimum[1])
									{
										minimum[1]=temp;
									}
									else
									{
										if (temp>maximum[1])
										{
											maximum[1]=temp;
										}
									}
									temp=(*position)[2];
									if (temp<minimum[2])
									{
										minimum[2]=temp;
									}
									else
									{
										if (temp>maximum[2])
										{
											maximum[2]=temp;
										}
									}
									number_of_positions--;
									position++;
								}
							}
							pointset=pointset->ptrnext;
						}
						pointset_ptr++;
						number_of_times--;
					}
				}
			} break;
			case g_POLYLINE:
			{
				if ((line_ptr=(graphics_object->gu).gt_polyline)&&
					(0<(number_of_times=graphics_object->number_of_times)))
				{
					while (number_of_times>0)
					{
						line= *line_ptr;
						while (line)
						{
							switch (line->polyline_type)
							{
								case g_PLAIN:
								{
									number_of_positions=line->n_pts;
									position_offset=1;
								} break;
								case g_NORMAL:
								{
									number_of_positions=line->n_pts;
									position_offset=2;
								} break;
								case g_PLAIN_DISCONTINUOUS:
								{
									number_of_positions=2*(line->n_pts);
									position_offset=1;
								} break;
								case g_NORMAL_DISCONTINUOUS:
								{
									number_of_positions=2*(line->n_pts);
									position_offset=2;
								} break;
								default:
								{
									number_of_positions=0;
								} break;
							}
							if ((position=line->pointlist)&&(0<number_of_positions))
							{
								if (*first)
								{
									minimum[0]=(*position)[0];
									maximum[0]=minimum[0];
									minimum[1]=(*position)[1];
									maximum[1]=minimum[1];
									minimum[2]=(*position)[2];
									maximum[2]=minimum[2];
									number_of_positions--;
									position += position_offset;
									*first=0;
								}
								while (number_of_positions>0)
								{
									temp=(*position)[0];
									if (temp<minimum[0])
									{
										minimum[0]=temp;
									}
									else
									{
										if (temp>maximum[0])
										{
											maximum[0]=temp;
										}
									}
									temp=(*position)[1];
									if (temp<minimum[1])
									{
										minimum[1]=temp;
									}
									else
									{
										if (temp>maximum[1])
										{
											maximum[1]=temp;
										}
									}
									temp=(*position)[2];
									if (temp<minimum[2])
									{
										minimum[2]=temp;
									}
									else
									{
										if (temp>maximum[2])
										{
											maximum[2]=temp;
										}
									}
									number_of_positions--;
									position += position_offset;
								}
							}
							line=line->ptrnext;
						}
						line_ptr++;
						number_of_times--;
					}
				}
			} break;
			case g_SURFACE:
			{
				/* We will ignore range of morph point lists for morphing types */
				if ((surface_ptr=(graphics_object->gu).gt_surface)&&
					(0<(number_of_times=graphics_object->number_of_times)))
				{
					while (number_of_times>0)
					{
						surface= *surface_ptr;
						while (surface)
						{
							number_of_positions=(surface->n_pts1)*(surface->n_pts2);
							if ((position=surface->pointlist)&&(0<number_of_positions))
							{
								if (*first)
								{
									minimum[0]=(*position)[0];
									maximum[0]=minimum[0];
									minimum[1]=(*position)[1];
									maximum[1]=minimum[1];
									minimum[2]=(*position)[2];
									maximum[2]=minimum[2];
									number_of_positions--;
									position++;
									*first=0;
								}
								while (number_of_positions>0)
								{
									temp=(*position)[0];
									if (temp<minimum[0])
									{
										minimum[0]=temp;
									}
									else
									{
										if (temp>maximum[0])
										{
											maximum[0]=temp;
										}
									}
									temp=(*position)[1];
									if (temp<minimum[1])
									{
										minimum[1]=temp;
									}
									else
									{
										if (temp>maximum[1])
										{
											maximum[1]=temp;
										}
									}
									temp=(*position)[2];
									if (temp<minimum[2])
									{
										minimum[2]=temp;
									}
									else
									{
										if (temp>maximum[2])
										{
											maximum[2]=temp;
										}
									}
									number_of_positions--;
									position++;
								}
							}
							surface=surface->ptrnext;
						}
						surface_ptr++;
						number_of_times--;
					}
				}
			} break;
			case g_VOLTEX:
			{
				/* We will ignore range of morph point lists for morphing types */
				if ((voltex_ptr=(graphics_object->gu).gt_voltex)&&
					(0<(number_of_times=graphics_object->number_of_times)))
				{
					while (number_of_times>0)
					{
						voltex= *voltex_ptr;
						while (voltex)
						{
							number_of_positions=(voltex->n_vertices);
							i = 0;
							if ((0<number_of_positions))
							{
								position = (Triple *)voltex->vertex_list->coord;
								if (*first)
								{
									minimum[0]=(*position)[0];
									maximum[0]=minimum[0];
									minimum[1]=(*position)[1];
									maximum[1]=minimum[1];
									minimum[2]=(*position)[2];
									maximum[2]=minimum[2];
									number_of_positions--;
									*first=0;
								}
								while (number_of_positions>0)
								{
									position = (Triple *)voltex->vertex_list[i].coord;
									i++;
									temp=(*position)[0];
									if (temp<minimum[0])
									{
										minimum[0]=temp;
									}
									else
									{
										if (temp>maximum[0])
										{
											maximum[0]=temp;
										}
									}
									temp=(*position)[1];
									if (temp<minimum[1])
									{
										minimum[1]=temp;
									}
									else
									{
										if (temp>maximum[1])
										{
											maximum[1]=temp;
										}
									}
									temp=(*position)[2];
									if (temp<minimum[2])
									{
										minimum[2]=temp;
									}
									else
									{
										if (temp>maximum[2])
										{
											maximum[2]=temp;
										}
									}
									number_of_positions--;
								}
							}
							voltex=voltex->ptrnext;
						}
						voltex_ptr++;
						number_of_times--;
					}
				}
			} break;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_graphics_object_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_graphics_object_range */

int get_graphics_object_data_range(struct GT_object *graphics_object,
	void *graphics_object_data_range_void)
/*******************************************************************************
LAST MODIFIED : 7 July 1998

DESCRIPTION :
Returns the range of the data values stored in the graphics object.
Returned range generally used to set or enlarge spectrum ranges.
???RC likely to change when multiple data values stored in graphics_objects
for combining several spectrums.
==============================================================================*/
{
	float maximum,minimum;
	int i,j,return_code,first;
	struct Graphics_object_data_range_struct *graphics_object_data_range;
	GTDATA *field_data;
	struct GT_glyph_set *glyph_set,**glyph_set_ptr;
	struct GT_pointset *pointset,**pointset_ptr;
	struct GT_polyline *polyline,**polyline_ptr;
	struct GT_surface *surface,**surface_ptr;
	struct GT_voltex *voltex,**voltex_ptr;
	struct VT_iso_vertex *vertex;

	ENTER(get_graphics_object_data_range);
	/* check arguments */
	if (graphics_object&&(graphics_object_data_range=(struct
		Graphics_object_data_range_struct *)graphics_object_data_range_void))
	{
		first=graphics_object_data_range->first;
		minimum=graphics_object_data_range->minimum;
		maximum=graphics_object_data_range->maximum;
		return_code=1;
		switch (graphics_object->object_type)
		{
			case g_GLYPH_SET:
			{
				glyph_set_ptr=graphics_object->gu.gt_glyph_set;
				for (j=graphics_object->number_of_times;j>0;j--)
				{
					glyph_set= *glyph_set_ptr;
					while (glyph_set)
					{
						if (field_data=glyph_set->data)
						{
							if (first)
							{
								minimum= *field_data;
								maximum=minimum;
								first=0;
							}
							for (i=glyph_set->number_of_points;i>0;i--)
							{
								if (*field_data<minimum)
								{
									minimum= *field_data;
								}
								else
								{
									if (*field_data>maximum)
									{
										maximum= *field_data;
									}
								}
								field_data++;
							}
						}
						glyph_set=glyph_set->ptrnext;
					}
					glyph_set_ptr++;
				}
			} break;
			case g_POINTSET:
			{
				pointset_ptr=graphics_object->gu.gt_pointset;
				for (j=graphics_object->number_of_times;j>0;j--)
				{
					pointset= *pointset_ptr;
					while (pointset)
					{
						if (field_data=pointset->data)
						{
							if (first)
							{
								minimum= *field_data;
								maximum=minimum;
								first=0;
							}
							for (i=pointset->n_pts;i>0;i--)
							{
								if (*field_data<minimum)
								{
									minimum= *field_data;
								}
								else
								{
									if (*field_data>maximum)
									{
										maximum= *field_data;
									}
								}
								field_data++;
							}
						}
						pointset=pointset->ptrnext;
					}
					pointset_ptr++;
				}
			} break;
			case g_POLYLINE:
			{
				polyline_ptr=graphics_object->gu.gt_polyline;
				for (j=graphics_object->number_of_times;j>0;j--)
				{
					polyline= *polyline_ptr;
					while (polyline)
					{
						if (field_data=polyline->data)
						{
							if (first)
							{
								minimum= *field_data;
								maximum=minimum;
								first=0;
							}
							for (i=polyline->n_pts;i>0;i--)
							{
								if (*field_data<minimum)
								{
									minimum= *field_data;
								}
								else
								{
									if (*field_data>maximum)
									{
										maximum= *field_data;
									}
								}
								field_data++;
							}
						}
						polyline=polyline->ptrnext;
					}
					polyline_ptr++;
				}
			} break;
			case g_SURFACE:
			{
				surface_ptr=graphics_object->gu.gt_surface;
				for (j=graphics_object->number_of_times;j>0;j--)
				{
					/*???RC must have a first surface to get its type */
					if (surface= *surface_ptr)
					{
						switch (surface->surface_type)
						{
							case g_SH_DISCONTINUOUS:
							case g_SH_DISCONTINUOUS_TEXMAP:
							{
								while (surface)
								{
									if (field_data=surface->data)
									{
										if (first)
										{
											minimum= *field_data;
											maximum=minimum;
											first=0;
										}
										for (i=(surface->n_pts1)*(surface->n_pts2);i>0;i--)
										{
											if (*field_data<minimum)
											{
												minimum= *field_data;
											}
											else
											{
												if (*field_data>maximum)
												{
													maximum= *field_data;
												}
											}
											field_data++;
										}
									}
									surface=surface->ptrnext;
								}
								surface_ptr++;
							} break;
							default:
							{
								while (surface)
								{
									if (field_data=surface->data)
									{
										if (first)
										{
											minimum= *field_data;
											maximum=minimum;
											first=0;
										}
										switch (surface->polygon)
										{
											case g_QUADRILATERAL:
											{
												for (i=(surface->n_pts1)*(surface->n_pts2);i>0;i--)
												{
													if (*field_data<minimum)
													{
														minimum= *field_data;
													}
													else
													{
														if (*field_data>maximum)
														{
															maximum= *field_data;
														}
													}
													field_data++;
												}
											} break;
											case g_TRIANGLE:
											{
												for (i=(((surface->n_pts1)+1)*(surface->n_pts1))/2;i>0;
														 i--)
												{
													if (*field_data<minimum)
													{
														minimum= *field_data;
													}
													else
													{
														if (*field_data>maximum)
														{
															maximum= *field_data;
														}
													}
													field_data++;
												}
											} break;
										}
									}
									surface=surface->ptrnext;
								}
								surface_ptr++;
							} break;
						}
					}
				}
			} break;
			case g_VOLTEX:
			{
				voltex_ptr=graphics_object->gu.gt_voltex;
				if (first)
				{
					minimum=(*voltex_ptr)->data[(*voltex_ptr)->vertex_list->data_index];
					maximum=minimum;
					first=0;
				}
				for (j=graphics_object->number_of_times;j>0;j--)
				{
					voltex= *voltex_ptr;
					while (voltex)
					{
						vertex=voltex->vertex_list;
						for (i=(voltex->n_vertices)*(voltex->n_rep);i>0;i--)
						{
							if ((*voltex_ptr)->data[vertex->data_index]<minimum)
							{
								minimum=(*voltex_ptr)->data[vertex->data_index];
							}
							else
							{
								if ((*voltex_ptr)->data[vertex->data_index]>maximum)
								{
									maximum=(*voltex_ptr)->data[vertex->data_index];
								}
							}
							vertex++;
						}
						voltex=voltex->ptrnext;
					}
					voltex_ptr++;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"get_graphics_object_data_range.  Invalid graphics object type");
				return_code=0;
			} break;
		}
		graphics_object_data_range->first=first;
		graphics_object_data_range->minimum=minimum;
		graphics_object_data_range->maximum=maximum;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_graphics_object_data_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_graphics_object_data_range */

int get_graphics_object_time_range(struct GT_object *graphics_object,
	void *graphics_object_time_range_void)
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Enlarges the minimum and maximum time range by that of the graphics_object.
==============================================================================*/
{
	float *times;
	int return_code;
	struct Graphics_object_time_range_struct *graphics_object_time_range;

	ENTER(get_graphics_object_time_range);
	if (graphics_object&&(graphics_object_time_range=(struct
		Graphics_object_time_range_struct *)graphics_object_time_range_void))
	{
		return_code=1;
		if (0 < graphics_object->number_of_times)
		{
			if (times=graphics_object->times)
			{
				if (graphics_object_time_range->first)
				{
					graphics_object_time_range->minimum=times[0];
					graphics_object_time_range->maximum=
						times[graphics_object->number_of_times-1];
					graphics_object_time_range->first=0;
				}
				else
				{
					if (times[0] < graphics_object_time_range->minimum)
					{
						graphics_object_time_range->minimum = times[0];
					}
					if (times[graphics_object->number_of_times-1] >
						graphics_object_time_range->maximum)
					{
						graphics_object_time_range->maximum =
							times[graphics_object->number_of_times-1];
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"GT_object_get_time_range.  Invalid times array");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_graphics_object_time_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_graphics_object_time_range */

#define DECLARE_GT_OBJECT_ADD_FUNCTION(primitive_type, \
	gt_object_type,primitive_var) \
PROTOTYPE_GT_OBJECT_ADD_FUNCTION(primitive_type) \
/***************************************************************************** \
LAST MODIFIED : 22 October 1998 \
\
DESCRIPTION : \
Adds <primitive> to <graphics_object> at <time>, creating the new time if it \
does not already exist. If the <primitive> is NULL an empty time is added if \
there is not already one. \
============================================================================*/ \
{ \
	float *times; \
	struct primitive_type **primitive_ptr; \
	int return_code,time_no,i; \
\
	ENTER(GT_OBJECT_ADD(primitive_type)); \
	if (graphics_object&&(gt_object_type==graphics_object->object_type)) \
	{ \
		time_no= \
			GT_object_get_less_than_or_equal_time_number(graphics_object,time); \
		if ((0<time_no)&&(time==graphics_object->times[time_no-1])) \
		{ \
			/* add primitive to start of list at existing time */ \
			if (primitive_ptr=graphics_object->gu.primitive_var) \
			{ \
				if (primitive) \
				{ \
					primitive_ptr += time_no-1; \
					primitive->ptrnext=*primitive_ptr; \
					*primitive_ptr=primitive; \
				} \
            GT_object_changed(graphics_object); \
				return_code=1; \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"GT_OBJECT_ADD(" #primitive_type ").  Invalid primitives array"); \
				return_code=0; \
			} \
		} \
		else \
		{ \
			/* make a new time for the primitive */ \
			if (REALLOCATE(times,graphics_object->times,float, \
				graphics_object->number_of_times+1)) \
			{ \
				graphics_object->times=times; \
				if (REALLOCATE(primitive_ptr,(graphics_object->gu).primitive_var, \
					struct primitive_type *,graphics_object->number_of_times+1)) \
				{ \
					(graphics_object->gu).primitive_var=primitive_ptr; \
					times += graphics_object->number_of_times; \
					primitive_ptr += graphics_object->number_of_times; \
					(graphics_object->number_of_times)++; \
					for (i=(graphics_object->number_of_times)-time_no-1;i>0;i--) \
					{ \
						times--; \
						times[1]=times[0]; \
						primitive_ptr--; \
						primitive_ptr[1]=primitive_ptr[0]; \
					} \
					*times=time; \
					*primitive_ptr=primitive; \
               GT_object_changed(graphics_object); \
					return_code=1; \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE,"GT_OBJECT_ADD(" #primitive_type \
						").  Could not reallocate primitives"); \
					return_code=0; \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"GT_OBJECT_ADD(" #primitive_type ").  Could not reallocate times"); \
				(graphics_object->number_of_times)--; \
				return_code=0; \
			} \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"GT_OBJECT_ADD(" #primitive_type ").  Invalid arguments"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* GT_OBJECT_ADD(primitive_type) */

DECLARE_GT_OBJECT_ADD_FUNCTION(GT_glyph_set,g_GLYPH_SET,gt_glyph_set)
DECLARE_GT_OBJECT_ADD_FUNCTION(GT_nurbs,g_NURBS,gt_nurbs)
DECLARE_GT_OBJECT_ADD_FUNCTION(GT_point,g_POINT,gt_point)
DECLARE_GT_OBJECT_ADD_FUNCTION(GT_pointset,g_POINTSET,gt_pointset)
DECLARE_GT_OBJECT_ADD_FUNCTION(GT_polyline,g_POLYLINE,gt_polyline)
DECLARE_GT_OBJECT_ADD_FUNCTION(GT_surface,g_SURFACE,gt_surface)
DECLARE_GT_OBJECT_ADD_FUNCTION(GT_userdef,g_USERDEF,gt_userdef)
DECLARE_GT_OBJECT_ADD_FUNCTION(GT_voltex,g_VOLTEX,gt_voltex)

#define DECLARE_GT_OBJECT_GET_FUNCTION(primitive_type, \
	gt_object_type,primitive_var) \
PROTOTYPE_GT_OBJECT_GET_FUNCTION(primitive_type) \
/***************************************************************************** \
LAST MODIFIED : 19 June 1998 \
\
DESCRIPTION : \
Returns pointer to the primitive at the given time in graphics_object. \
============================================================================*/ \
{ \
	struct primitive_type *primitive; \
	int time_no; \
\
	ENTER(GT_OBJECT_GET(primitive_type)); \
	/* check arguments */ \
	if (graphics_object&&(gt_object_type==graphics_object->object_type)) \
	{ \
		if (0<(time_no=GT_object_get_time_number(graphics_object,time))) \
		{ \
			if (graphics_object->gu.primitive_var) \
			{ \
				primitive=graphics_object->gu.primitive_var[time_no-1]; \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"GT_OBJECT_GET(" #primitive_type ").  Invalid primitives array"); \
				primitive=(struct primitive_type *)NULL; \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"GT_OBJECT_GET(" #primitive_type ").  No primitives at time %g",time); \
			primitive=(struct primitive_type *)NULL; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"GT_OBJECT_GET(" #primitive_type ").  Invalid arguments"); \
		primitive=(struct primitive_type *)NULL; \
	} \
	LEAVE; \
\
	return (primitive); \
} /* GT_OBJECT_GET(primitive_type) */

DECLARE_GT_OBJECT_GET_FUNCTION(GT_glyph_set,g_GLYPH_SET,gt_glyph_set)
DECLARE_GT_OBJECT_GET_FUNCTION(GT_nurbs,g_NURBS,gt_nurbs)
DECLARE_GT_OBJECT_GET_FUNCTION(GT_point,g_POINT,gt_point)
DECLARE_GT_OBJECT_GET_FUNCTION(GT_pointset,g_POINTSET,gt_pointset)
DECLARE_GT_OBJECT_GET_FUNCTION(GT_polyline,g_POLYLINE,gt_polyline)
DECLARE_GT_OBJECT_GET_FUNCTION(GT_surface,g_SURFACE,gt_surface)
DECLARE_GT_OBJECT_GET_FUNCTION(GT_userdef,g_USERDEF,gt_userdef)
DECLARE_GT_OBJECT_GET_FUNCTION(GT_voltex,g_VOLTEX,gt_voltex)

int GT_object_delete_time(struct GT_object *graphics_object,float time)
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Removes all primitive at <time> from <graphics_object>.
==============================================================================*/
{
	int return_code,time_no;

	ENTER(GT_object_delete_time);
	if (graphics_object)
	{
		if (0<(time_no=GT_object_get_time_number(graphics_object,time)))
		{
			return_code=GT_object_delete_time_number(graphics_object,time_no);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"GT_object_delete_time.  No objects at time %g",time);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"GT_object_delete_time.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_delete_time */

#define DECLARE_GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME_FUNCTION( \
	primitive_type,gt_object_type,primitive_var,primitive_destroy_function) \
PROTOTYPE_GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME_FUNCTION(primitive_type)\
/***************************************************************************** \
LAST MODIFIED : 1 February 2000 \
\
DESCRIPTION : \
Removes all primitives from <graphics_object> at <time> for which the \
object_name member matches the given <object_name>. \
============================================================================*/ \
{ \
	struct primitive_type *primitive,**primitive_ptr; \
	int return_code,time_no; \
\
	ENTER(GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME(primitive_type)); \
	if (graphics_object&&(gt_object_type==graphics_object->object_type))\
	{ \
		if (0<(time_no=GT_object_get_time_number(graphics_object,time))) \
		{ \
			if (primitive_ptr=graphics_object->gu.primitive_var) \
			{ \
				primitive_ptr += time_no-1; \
				/* Remove primitives with object_name at this time */ \
				while (primitive= *primitive_ptr) \
				{ \
					if ((primitive->object_name)==object_name) \
					{ \
						*primitive_ptr=primitive->ptrnext; \
						primitive_destroy_function(&primitive); \
					} \
					else \
					{ \
						primitive_ptr=&(primitive->ptrnext); \
					} \
				} \
				/* must remove time if no primitives left there */ \
				/*???RC.  Not needed now that empty times are allowed? */ \
				if ((struct primitive_type *)NULL == \
					graphics_object->gu.primitive_var[time_no-1]) \
				{ \
					return_code=GT_OBJECT_DELETE_TIME_NUMBER(primitive_type)( \
						graphics_object,time_no); \
				} \
				else \
				{ \
					return_code=1; \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME(" #primitive_type \
					").  Invalid primitives array"); \
				return_code=0; \
			} \
		} \
		else \
		{ \
			return_code=1; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME(" #primitive_type \
			").  Invalid arguments"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME(primitive_type) */

DECLARE_GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME_FUNCTION(GT_glyph_set, \
	g_GLYPH_SET,gt_glyph_set,DESTROY(GT_glyph_set))
DECLARE_GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME_FUNCTION(GT_polyline, \
	g_POLYLINE,gt_polyline,DESTROY(GT_polyline))
DECLARE_GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME_FUNCTION(GT_surface, \
	g_SURFACE,gt_surface,DESTROY(GT_surface))
DECLARE_GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME_FUNCTION(GT_voltex, \
	g_VOLTEX,gt_voltex,DESTROY(GT_voltex))

int GT_object_clear_selected_graphic_list(struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Clears the list of selected primitives and subobjects in <graphics_object>.
==============================================================================*/
{
	int return_code;

	ENTER(GT_object_clear_selected_graphic_list);
	if (graphics_object)
	{
		return_code=REMOVE_ALL_OBJECTS_FROM_LIST(Selected_graphic)(
			graphics_object->selected_graphic_list);
		GT_object_changed(graphics_object);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_clear_selected_graphic_list.  Invalid graphics object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_clear_selected_graphic_list */

int GT_object_select_graphic(struct GT_object *graphics_object,int number,
	struct Multi_range *subranges)
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Selects graphics with the object_name <number> in <graphics_object>, with
optional <subranges> narrowing down the selection. Selected objects will be
rendered with the selected/highlight material.
Replaces any current selection with the same <number>.
Iff the function is successful the <subranges> will be owned by the
graphics_object.
==============================================================================*/
{
	int return_code;
	struct Selected_graphic *selected_graphic;

	ENTER(GT_object_select_graphic);
	return_code=0;
	if (graphics_object)
	{
		/* clear current selected_graphic for number */
		if (selected_graphic=FIND_BY_IDENTIFIER_IN_LIST(Selected_graphic,number)(
			number,graphics_object->selected_graphic_list))
		{
			REMOVE_OBJECT_FROM_LIST(Selected_graphic)(selected_graphic,
				graphics_object->selected_graphic_list);
		}
		if (selected_graphic=CREATE(Selected_graphic)(number))
		{
			if (((!subranges)||Selected_graphic_set_subranges(selected_graphic,
				subranges))&&ADD_OBJECT_TO_LIST(Selected_graphic)(selected_graphic,
					graphics_object->selected_graphic_list))
			{
				GT_object_changed(graphics_object);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"GT_object_select_graphic.  Error selecting graphic");
				DESTROY(Selected_graphic)(&selected_graphic);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"GT_object_select_graphic.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_select_graphic.  Invalid graphics object");
	}
	LEAVE;

	return (return_code);
} /* GT_object_select_graphic */

int GT_object_is_graphic_selected(struct GT_object *graphics_object,int number,
	struct Multi_range **subranges)
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Returns true if graphics with the given <number> are selected in
<graphics_object>, and if so, its selected subranges are returned with it. The
returned subranges (which can be NULL if there are none) are for short-term
viewing only as they belong the the Selected_graphic containing them.
==============================================================================*/
{
	int return_code;
	struct Selected_graphic *selected_graphic;

	ENTER(GT_object_is_graphic_selected);
	if (graphics_object&&subranges)
	{
		/* clear current selected_graphic for number */
		if (selected_graphic=FIND_BY_IDENTIFIER_IN_LIST(Selected_graphic,number)(
			number,graphics_object->selected_graphic_list))
		{
			return_code=1;
			*subranges=Selected_graphic_get_subranges(selected_graphic);
		}
		else
		{
			return_code=0;
			*subranges=(struct Multi_range *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_is_graphic_selected.  Invalid argument(s)");
		return_code=0;
		*subranges=(struct Multi_range *)NULL;
	}
	LEAVE;

	return (return_code);
} /* GT_object_is_graphic_selected */

struct Graphical_material *get_GT_object_default_material
	(struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Gets the default_material of a GT_object.
==============================================================================*/
{
	struct Graphical_material *material;

	ENTER(get_GT_object_default_material);
	if (graphics_object)
	{
		material = graphics_object->default_material;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_GT_object_default_material.  Invalid graphics object");
		material = (struct Graphical_material *)NULL;
	}
	LEAVE;

	return (material);
} /* get_GT_object_default_material */

int set_GT_object_default_material(struct GT_object *graphics_object,
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 20 February 2000

DESCRIPTION :
Sets the default_material of a GT_object.
==============================================================================*/
{
	int return_code;

	ENTER(set_GT_object_default_material);
	if (graphics_object)
	{
		if (material)
		{
			if (material != graphics_object->default_material)
			{
				REACCESS(Graphical_material)(&(graphics_object->default_material),
					material);
				GT_object_changed(graphics_object);
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_GT_object_default_material.  Invalid material object");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_GT_object_default_material.  Invalid graphics object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_GT_object_default_material */

struct Graphical_material *get_GT_object_selected_material
	(struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Gets the selected_material of a GT_object.
==============================================================================*/
{
	struct Graphical_material *material;

	ENTER(get_GT_object_selected_material);
	if (graphics_object)
	{
		material = graphics_object->selected_material;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_GT_object_selected_material.  Invalid graphics object");
		material = (struct Graphical_material *)NULL;
	}
	LEAVE;

	return (material);
} /* get_GT_object_selected_material */

int set_GT_object_selected_material(struct GT_object *graphics_object,
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 20 February 2000

DESCRIPTION :
Sets the selected_material of a GT_object.
==============================================================================*/
{
	int return_code;

	ENTER(set_GT_object_selected_material);
	if (graphics_object)
	{
		if (material)
		{
			if (material != graphics_object->selected_material)
			{
				REACCESS(Graphical_material)(&(graphics_object->selected_material),
					material);
				GT_object_changed(graphics_object);
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_GT_object_selected_material.  Invalid material object");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_GT_object_selected_material.  Invalid graphics object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_GT_object_selected_material */

int set_GT_object_Spectrum(struct GT_object *graphics_object,
	void *spectrum_void)
/*******************************************************************************
LAST MODIFIED : 15 June 1998

DESCRIPTION :
Sets the spectrum of a GT_object.
==============================================================================*/
{
	int return_code;
	struct Spectrum *spectrum;

	ENTER(set_GT_object_Spectrum);
	if (graphics_object)
	{
		if (spectrum=(struct Spectrum *)spectrum_void)
		{
			if (spectrum != graphics_object->spectrum)
			{
				ACCESS(Spectrum)(spectrum);
				if (graphics_object->spectrum)
				{
					DEACCESS(Spectrum)(&graphics_object->spectrum);
				}
				graphics_object->spectrum=spectrum;
				GT_object_changed(graphics_object);
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_GT_object_Spectrum.  Invalid spectrum object");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_GT_object_Spectrum.  Invalid graphics object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_GT_object_Spectrum */

struct Spectrum *get_GT_object_spectrum(struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Gets the spectrum of a GT_object.
==============================================================================*/
{
	struct Spectrum *spectrum;

	ENTER(get_GT_object_spectrum);
	if (graphics_object)
	{
		spectrum = graphics_object->spectrum;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_GT_object_spectrum.  Invalid graphics object");
		spectrum = (struct Spectrum *)NULL;
	}
	LEAVE;

	return (spectrum);
} /* get_GT_object_spectrum */

int set_GT_object_list_Spectrum(struct LIST(GT_object) *graphics_object_list,
	struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 18 October 1997

DESCRIPTION :
Sets the spectrum of all the GT_objects in a list.
==============================================================================*/
{
	int return_code;

	ENTER(set_GT_object_list_Spectrum);
	if (graphics_object_list)
	{
		if (spectrum)
		{
			return_code=FOR_EACH_OBJECT_IN_LIST(GT_object)(set_GT_object_Spectrum,
				(void *)(spectrum),graphics_object_list);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_GT_object_Spectrum.  Invalid spectrum object");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_GT_object_Spectrum.  Invalid graphics object list");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_GT_object_list_Spectrum */

int GT_object_list_contents(struct GT_object *graphics_object,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 August 1998

DESCRIPTION :
Writes out information contained in <graphics_object> including its name and
type.
==============================================================================*/
{
	char *material_name;
	int return_code;

	ENTER(GT_object_list_contents);
	if (graphics_object && (!dummy_void))
	{
		if (graphics_object->name)
		{
			return_code=1;
			display_message(INFORMATION_MESSAGE,graphics_object->name);
			display_message(INFORMATION_MESSAGE," = %s",
				get_GT_object_type_string(graphics_object->object_type));
			if (graphics_object->default_material&&GET_NAME(Graphical_material)(
				graphics_object->default_material,&material_name))
			{
				display_message(INFORMATION_MESSAGE," material %s",
					material_name);
				DEALLOCATE(material_name);
			}
			display_message(INFORMATION_MESSAGE,"; access_count=%d\n",
				graphics_object->access_count);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"GT_object_list_contents.  Missing graphics object name");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_list_contents.  Invalid graphics object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_list_contents */

#if defined (OLD_CODE)
int expand_spectrum_range_with_graphics_object(
	struct GT_object *graphics_object,void *spectrum_void)
/*******************************************************************************
LAST MODIFIED : 3 August 1998

DESCRIPTION :
Ensures the <spectrum> maximum and minimum is at least large enough to include
the range of data values in <graphics_object>.
==============================================================================*/
{
	int return_code;
	struct Graphics_object_data_range_struct graphics_object_data_range;
	struct Spectrum *spectrum;

	ENTER(expand_spectrum_range_with_graphics_object);
	display_message(INFORMATION_MESSAGE,
	"expand_spectrum_range_with_graphics_object.  Obsolete");
	return_code=1;
	/* check arguments */
	if (graphics_object&&(spectrum=(struct Spectrum *)spectrum_void))
	{
		/* get data range of graphics object */
		graphics_object_data_range.minimum=get_Spectrum_minimum(spectrum);
		graphics_object_data_range.maximum=get_Spectrum_maximum(spectrum);
		graphics_object_data_range.first=
			graphics_object_data_range.minimum > graphics_object_data_range.maximum;
		if (return_code=get_graphics_object_data_range(graphics_object,
			(void *)&graphics_object_data_range))
		{
			Spectrum_set_minimum_and_maximum(spectrum,
				graphics_object_data_range.minimum,graphics_object_data_range.maximum);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"expand_spectrum_range_with_graphics_object.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* expand_spectrum_range_with_graphics_object */
#endif /* defined (OLD_CODE) */

int set_Graphics_object(struct Parse_state *state,
	void *graphics_object_address_void,void *graphics_object_list_void)
/*******************************************************************************
LAST MODIFIED : 26 November 1998

DESCRIPTION :
Modifier function to set the graphics_object from a command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct GT_object *graphics_object,**graphics_object_address;
	struct LIST(GT_object) *graphics_object_list;

	ENTER(set_Graphics_Object);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((graphics_object_address=
					(struct GT_object **)graphics_object_address_void)&&
					(graphics_object_list=
					(struct LIST(GT_object)*)graphics_object_list_void ))
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*graphics_object_address)
						{
							DEACCESS(GT_object)(graphics_object_address);
							*graphics_object_address=(struct GT_object *)NULL;
						}
						return_code=1;
					}
					else
					{
						if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
							current_token,graphics_object_list))
						{
							if (*graphics_object_address != graphics_object)
							{
								ACCESS(GT_object)(graphics_object);
								if (*graphics_object_address)
								{
									DEACCESS(GT_object)(graphics_object_address);
								}
								*graphics_object_address=graphics_object;
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Unknown graphics object : %s",current_token);
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Graphics_object.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," GRAPHICS_OBJECT_NAME|none");
				/* if possible, then write the name */
				if (graphics_object_address=
					(struct GT_object **)graphics_object_address_void)
				{
					if (graphics_object= *graphics_object_address)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",graphics_object->name);
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"[none]");
					}
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing graphics_object name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Graphics_object.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Graphics_object */
