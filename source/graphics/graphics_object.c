/*******************************************************************************
FILE : graphics_object.c

LAST MODIFIED : 28 November 2003

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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "general/octree.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/decimate_voltex.h"
#include "graphics/font.h"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/rendergl.h"
#include "graphics/spectrum.h"
#include "graphics/volume_texture.h"
#include "user_interface/message.h"
#include "graphics/graphics_object_private.h"

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
	int time_number;

	ENTER(GT_object_get_time_number);
	if (graphics_object)
	{
		if (0<(time_number=graphics_object->number_of_times))
		{
			if (times=graphics_object->times)
			{
				times += time_number-1;
				while ((time_number>0)&&(time< *times))
				{
					times--;
					time_number--;
				}
				if ((time_number>0)&&(time != *times))
				{
					time_number=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"GT_object_get_time_number.  Invalid times array");
				time_number=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_get_time_number.  Invalid arguments");
		time_number=0;
	}
	LEAVE;

	return (time_number);
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
	int time_number;

	ENTER(GT_object_get_less_than_or_equal_time_number);
	if (graphics_object)
	{
		if (0<(time_number=graphics_object->number_of_times))
		{
			if (times=graphics_object->times)
			{
				times += time_number-1;
				while ((time_number>0)&&(time< *times))
				{
					times--;
					time_number--;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"GT_object_get_less_than_or_equal_time_number.  Invalid times array");
				time_number=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_get_less_than_or_equal_time_number.  Invalid arguments");
		time_number=0;
	}
	LEAVE;

	return (time_number);
} /* GT_object_get_less_than_or_equal_time_number */

#if ! defined (SHORT_NAMES)
#define GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(primitive_type) \
	GT_object_remove_primitives_at_time_number_ ## primitive_type
#else
#define GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(primitive_type) \
	gorptn_ ## primitive_type
#endif

#define DECLARE_GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER_FUNCTION( \
	primitive_type,gt_object_type,primitive_var) \
static int GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(primitive_type)( \
	struct GT_object *graphics_object, int time_number, \
	GT_object_primitive_object_name_conditional_function *conditional_function, \
	void *user_data) \
/***************************************************************************** \
LAST MODIFIED : 17 March 2003 \
\
DESCRIPTION : \
Removes all primitives from <graphics_object> at <time_number>. \
The optional <conditional_function> allows a subset of the primitives to \
be removed. This function is called with the object_name integer associated \
with each primitive plus the void *<user_data> supplied here. A true result \
from the conditional_function causes the primitive to be removed. \
============================================================================*/ \
{ \
	float *times; \
	int i, return_code; \
	struct primitive_type *last_primitive, *primitive, **primitive_ptr; \
	union GT_primitive_list *primitive_list; \
\
	ENTER(GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(primitive_type)); \
	if (graphics_object && (gt_object_type == graphics_object->object_type) && \
		(0 < time_number) && (time_number <= graphics_object->number_of_times)) \
	{ \
		if (times = graphics_object->times) \
		{ \
			if (graphics_object->primitive_lists) \
			{ \
				primitive_list = graphics_object->primitive_lists + time_number - 1; \
				last_primitive = (struct primitive_type *)NULL; \
				primitive_ptr = &(primitive_list->primitive_var.first); \
				/* remove primitives at this time */ \
				while (primitive = *primitive_ptr) \
				{ \
					if ((!conditional_function) || \
						(conditional_function)(primitive->object_name, user_data)) \
					{ \
						*primitive_ptr = primitive->ptrnext; \
						DESTROY(primitive_type)(&primitive); \
					} \
					else \
					{ \
						last_primitive = primitive; \
						primitive_ptr = &(primitive->ptrnext); \
					} \
				} \
				primitive_list->primitive_var.last = last_primitive; \
				/* must remove time if no primitives left there */ \
				if ((struct primitive_type *)NULL == primitive_list->primitive_var.first) \
				{ \
					times += time_number - 1; \
					/* Shift following times and primitives down */ \
					for (i = (graphics_object->number_of_times)-time_number; i > 0; i--) \
					{ \
						*times = *(times + 1); \
						times++; \
						primitive_list->primitive_var.first = primitive_list[1].primitive_var.first; \
						primitive_list->primitive_var.last = primitive_list[1].primitive_var.last; \
						primitive_ptr++; \
					} \
					graphics_object->number_of_times--; \
					/* do not reallocate to make times and primitive_var arrays smaller \
						 since this function is most often called by DESTROY(GT_object), \
						 or just before primitives are to be added again at the same time. \
						 However, must deallocate if there are no times left so DESTROY \
						 function can rely on this function to clean up. */ \
					if (0 == graphics_object->number_of_times) \
					{ \
						DEALLOCATE(graphics_object->times); \
						graphics_object->times = (float *)NULL; \
						DEALLOCATE(graphics_object->primitive_lists); \
					} \
				} \
				return_code = 1; \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(" \
					#primitive_type ").  Invalid primitive lists"); \
				return_code = 0; \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(" \
				#primitive_type ").  Invalid times array"); \
			return_code = 0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
	 		"GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(" \
	 		#primitive_type ").  Invalid arguments"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(primitive_type) */

#define DECLARE_GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER_AUXILIARY_FUNCTION( \
	primitive_type,gt_object_type,primitive_var) \
int GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(primitive_type)( \
	struct GT_object *graphics_object, int time_number, \
	GT_object_primitive_object_name_conditional_function *conditional_function, \
	void *user_data) \
/***************************************************************************** \
LAST MODIFIED : 17 March 2003 \
\
DESCRIPTION : \
Version of GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER for primitives with \
both an <object_name> and an <auxiliary_object_name> that may satisfy the \
conditional_function. \
============================================================================*/ \
{ \
	float *times; \
	int i, return_code; \
	struct primitive_type *last_primitive, *primitive, **primitive_ptr; \
	union GT_primitive_list *primitive_list; \
\
	ENTER(GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(primitive_type)); \
	if (graphics_object && (gt_object_type == graphics_object->object_type) && \
		(0 < time_number) && (time_number <= graphics_object->number_of_times)) \
	{ \
		if (times = graphics_object->times) \
		{ \
			if (graphics_object->primitive_lists) \
			{ \
				primitive_list = graphics_object->primitive_lists + time_number - 1; \
				last_primitive = (struct primitive_type *)NULL; \
				primitive_ptr = &(primitive_list->primitive_var.first); \
				/* remove primitives at this time */ \
				while (primitive = *primitive_ptr) \
				{ \
					if ((!conditional_function) || \
						(conditional_function)(primitive->object_name, user_data) || \
						(conditional_function)(primitive->auxiliary_object_name, \
							user_data)) \
					{ \
						*primitive_ptr = primitive->ptrnext; \
						DESTROY(primitive_type)(&primitive); \
					} \
					else \
					{ \
						last_primitive = primitive; \
						primitive_ptr = &(primitive->ptrnext); \
					} \
				} \
				primitive_list->primitive_var.last = last_primitive; \
				/* must remove time if no primitives left there */ \
				if ((struct primitive_type *)NULL == primitive_list->primitive_var.first) \
				{ \
					times += time_number - 1; \
					/* Shift following times and primitives down */ \
					for (i = (graphics_object->number_of_times)-time_number; i > 0; i--) \
					{ \
						*times = *(times + 1); \
						times++; \
						primitive_list->primitive_var.first = primitive_list[1].primitive_var.first; \
						primitive_list->primitive_var.last = primitive_list[1].primitive_var.last; \
						primitive_ptr++; \
					} \
					graphics_object->number_of_times--; \
					/* do not reallocate to make times and primitive_var arrays smaller \
						 since this function is most often called by DESTROY(GT_object), \
						 or just before primitives are to be added again at the same time. \
						 However, must deallocate if there are no times left so DESTROY \
						 function can rely on this function to clean up. */ \
					if (0 == graphics_object->number_of_times) \
					{ \
						DEALLOCATE(graphics_object->times); \
						graphics_object->times = (float *)NULL; \
						DEALLOCATE(graphics_object->primitive_lists); \
					} \
				} \
				return_code = 1; \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(" \
					#primitive_type ").  Invalid primitive lists"); \
				return_code = 0; \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(" \
				#primitive_type ").  Invalid times array"); \
			return_code = 0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
	 		"GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(" \
	 		#primitive_type ").  Invalid arguments"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(primitive_type) */

DECLARE_GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER_AUXILIARY_FUNCTION( \
	GT_glyph_set,g_GLYPH_SET,gt_glyph_set)
DECLARE_GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER_FUNCTION( \
	GT_nurbs,g_NURBS,gt_nurbs)
DECLARE_GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER_FUNCTION( \
	GT_point,g_POINT,gt_point)
DECLARE_GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER_FUNCTION( \
	GT_pointset,g_POINTSET,gt_pointset)
DECLARE_GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER_FUNCTION( \
	GT_polyline,g_POLYLINE,gt_polyline)
DECLARE_GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER_FUNCTION( \
	GT_surface,g_SURFACE,gt_surface)
DECLARE_GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER_FUNCTION( \
	GT_userdef,g_USERDEF,gt_userdef)
DECLARE_GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER_FUNCTION( \
	GT_voltex,g_VOLTEX,gt_voltex)

static int GT_object_remove_primitives_at_time_number(
	struct GT_object *graphics_object, int time_number,
	GT_object_primitive_object_name_conditional_function *conditional_function,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 5 February 2003

DESCRIPTION :
Removes primitives at <time_number> from <graphics_object>.
The optional <conditional_function> allows a subset of the primitives to
be removed. This function is called with the object_name integer associated
with each primitive plus the void *<user_data> supplied here. A true result
from the conditional_function causes the primitive to be removed.
==============================================================================*/
{
	int return_code;

	ENTER(GT_object_remove_primitives_at_time_number);
	if (graphics_object)
	{
		switch (graphics_object->object_type)
		{
			case g_GLYPH_SET:
			{
				return_code=GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(GT_glyph_set)(
					graphics_object, time_number, conditional_function, user_data);
			} break;
			case g_NURBS:
			{
				return_code=GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(GT_nurbs)(
					graphics_object, time_number, conditional_function, user_data);
			} break;
			case g_POINT:
			{
				return_code=GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(GT_point)(
					graphics_object, time_number, conditional_function, user_data);
			} break;
			case g_POINTSET:
			{
				return_code=GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(GT_pointset)(
					graphics_object, time_number, conditional_function, user_data);
			} break;
			case g_POLYLINE:
			{
				return_code=GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(GT_polyline)(
					graphics_object, time_number, conditional_function, user_data);
			} break;
			case g_SURFACE:
			{
				return_code=GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(GT_surface)(
					graphics_object, time_number, conditional_function, user_data);
			} break;
			case g_USERDEF:
			{
				return_code=GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(GT_userdef)(
					graphics_object, time_number, conditional_function, user_data);
			} break;
			case g_VOLTEX:
			{
				return_code=GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(GT_voltex)(
					graphics_object, time_number, conditional_function, user_data);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"GT_object_remove_primitives_at_time_number.  Unknown object type");
				return_code = 0;
			}
		}
		GT_object_changed(graphics_object);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_remove_primitives_at_time_number.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_remove_primitives_at_time_number */

/*
Global functions
----------------
*/

enum GT_object_type GT_object_get_type(struct GT_object *gt_object)
/*******************************************************************************
LAST MODIFIED : 17 June 2004

DESCRIPTION :
Returns the object type from the gt_object.
==============================================================================*/
{
	enum GT_object_type type;

	ENTER(GT_object_get_type);
	if (gt_object)
	{
		type = gt_object->object_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_get_type.  Invalid argument(s)");
		type = g_OBJECT_TYPE_INVALID; 
	}
	LEAVE;

	return (type);
} /* GT_object_get_type */

struct GT_object *GT_object_get_next_object(struct GT_object *gt_object)
/*******************************************************************************
LAST MODIFIED : 17 June 2004

DESCRIPTION :
Returns the next object from the gt_object.
==============================================================================*/
{
	struct GT_object *next;

	ENTER(GT_object_get_next_object);
	if (gt_object)
	{
		next = gt_object->nextobject;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_get_next_object.  Invalid argument(s)");
		next = (struct GT_object *)NULL;
	}
	LEAVE;

	return (next);
} /* GT_object_get_next_object */

int GT_object_set_next_object(struct GT_object *gt_object,
	struct GT_object *next_object)
/*******************************************************************************
LAST MODIFIED : 17 June 2004

DESCRIPTION :
Sets the next object for the gt_object.
==============================================================================*/
{
	int return_code;

	ENTER(GT_object_set_next_object);
	if (gt_object)
	{
		REACCESS(GT_object)(&gt_object->nextobject, next_object);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_set_next_object.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_set_next_object */

int GT_object_compare_name(struct GT_object *gt_object,
	char *name)
/*******************************************************************************
LAST MODIFIED : 17 June 2004

DESCRIPTION :
Returns true if the name of the <gt_object> matches the string <name> exactly.
==============================================================================*/
{
	int return_code;

	ENTER(GT_object_set_next_object);
	if (gt_object && name)
	{
		if (!strcmp(gt_object->name, name))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_compare_name.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_compare_name */

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
	struct GT_glyph_set *initial, struct GT_glyph_set *final)
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Creates a new GT_glyph_set which is the interpolation of two GT_glyph_sets.
The two glyph_sets must have the same number_of_points, glyph and data_type.
Text, if any, is copied from the <initial> glyph_set if proportion < 0.5,
otherwise from <final>.
???RC This could be improved by interpolating quaternions for the axes, but
this gets tricky when done on all axes consistently.
==============================================================================*/
{
	char **labels;
	GTDATA *data;
	struct GT_glyph_set *glyph_set, *nearest_glyph_set;
	int i, j, number_of_points, *names, return_code;
	Triple *point_list, *axis1_list, *axis2_list, *axis3_list, *scale_list;

	ENTER(morph_GT_glyph_set);
	if ((initial->number_of_points == final->number_of_points) &&
		(initial->glyph == final->glyph) &&
		(initial->n_data_components == final->n_data_components))
	{
		return_code = 1;
		number_of_points = initial->number_of_points;
		if (proportion < 0.5)
		{
			nearest_glyph_set = initial;
		}
		else
		{
			nearest_glyph_set = final;
		}
		point_list = (Triple *)NULL;
		axis1_list = (Triple *)NULL;
		axis2_list = (Triple *)NULL;
		axis3_list = (Triple *)NULL;
		scale_list = (Triple *)NULL;
		data = (GTDATA *)NULL;
		labels = (char **)NULL;
		names = (int *)NULL;
		if ((0 < number_of_points) &&
			ALLOCATE(point_list, Triple, number_of_points) &&
			ALLOCATE(axis1_list, Triple, number_of_points) &&
			ALLOCATE(axis2_list, Triple, number_of_points) &&
			ALLOCATE(axis3_list, Triple, number_of_points) &&
			ALLOCATE(scale_list, Triple, number_of_points))
		{
			for (i = 0; i < number_of_points; i++)
			{
				for (j = 0; j < 3; j++)
				{
					point_list[i][j] = (1.0 - proportion)*(initial->point_list)[i][j] +
						proportion*(final->point_list)[i][j];
					axis1_list[i][j] = (1.0 - proportion)*(initial->axis1_list)[i][j] +
						proportion*(final->axis1_list)[i][j];
					axis2_list[i][j] = (1.0 - proportion)*(initial->axis2_list)[i][j] +
						proportion*(final->axis2_list)[i][j];
					axis3_list[i][j] = (1.0 - proportion)*(initial->axis3_list)[i][j] +
						proportion*(final->axis3_list)[i][j];
					scale_list[i][j] = (1.0 - proportion)*(initial->scale_list)[i][j] +
						proportion*(final->scale_list)[i][j];
				}
			}
			/* check for and allocate any data */
			if (initial->n_data_components)
			{
				if (ALLOCATE(data,GTDATA,number_of_points*initial->n_data_components))
				{
					for (i = 0; i < number_of_points*initial->n_data_components; i++)
					{
						data[i] = (1.0 - proportion)*(initial->data)[i] +
							proportion*(final->data)[i];
					}
				}
				else
				{
					return_code = 0;
				}
			}
			if (point_list && nearest_glyph_set->labels)
			{
				if (ALLOCATE(labels, char *, number_of_points))
				{
					for (i = 0; labels && (i < number_of_points); i++)
					{
						if (nearest_glyph_set->labels[i])
						{
							if (!(labels[i] = duplicate_string(nearest_glyph_set->labels[i])))
							{
								for (j = 0; j < i; j++)
								{
									DEALLOCATE(labels[i]);
								}
								DEALLOCATE(labels);
							}
						}
						else
						{
							labels[i] = (char *)NULL;
						}
					}
				}
				if (!labels)
				{
					return_code = 0;
				}
			}
			if (point_list && nearest_glyph_set->names)
			{
				if (ALLOCATE(names, int, number_of_points))
				{
					for (i = 0; i < number_of_points; i++)
					{
						names[i] = nearest_glyph_set->names[i];
					}
				}
				else
				{
					return_code = 0;
				}
			}
			if (return_code)
			{
				if (glyph_set = CREATE(GT_glyph_set)(number_of_points, point_list,
					axis1_list, axis2_list, axis3_list, scale_list, initial->glyph,
					initial->font, labels, initial->n_data_components, data,
					/*label_bounds_dimension*/0, /*label_bounds_components*/0, /*label_bounds*/(float *)NULL,
					nearest_glyph_set->object_name, names))
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
					return_code = 0;
				}
			}
		}
		else
		{
			return_code = 0;
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
			DEALLOCATE(scale_list);
			DEALLOCATE(data);
			if (labels)
			{
				for (i = 0; i < nearest_glyph_set->number_of_points; i++)
				{
					DEALLOCATE(labels[i]);
				}
				DEALLOCATE(labels);
			}
			DEALLOCATE(names);
			glyph_set = (struct GT_glyph_set *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"morph_GT_glyph_set.  Inconsistent initial and final glyph_sets");
		glyph_set = (struct GT_glyph_set *)NULL;
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
						morph_names,initial->font))
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
		(initial->line_width==final->line_width)&&
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
					initial->line_width,initial->n_pts,point,normallist,
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
LAST MODIFIED : 28 November 2003

DESCRIPTION :
Creates a new GT_surface which is the interpolation of two GT_surfaces.
==============================================================================*/
{
	GTDATA *data;
	struct GT_surface *surface;
	int i,j,number_of_nodes;
	Triple *normallist,*point,*tangentlist,*texturelist;

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
			if (initial->tangentlist)
			{
				if (ALLOCATE(tangentlist,Triple,number_of_nodes))
				{
					for (i=0;i<number_of_nodes;i++)
					{
						for (j=0;j<3;j++)
						{
							tangentlist[i][j]=(1.0-proportion)*(initial->tangentlist)[i][j]+
								proportion*(final->tangentlist)[i][j];
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
				tangentlist = (Triple *)NULL;
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
						if (tangentlist)
						{
							DEALLOCATE(tangentlist);
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
						if (tangentlist)
						{
							DEALLOCATE(tangentlist);
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
					initial->n_pts1,initial->n_pts2,point,normallist,tangentlist,texturelist,
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
					if (tangentlist)
					{
						DEALLOCATE(tangentlist);
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
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Creates a new gtObject which is the interpolation of two gtObjects.
==============================================================================*/
{
	float time;
	gtObject *graphics_object;
	int i, number_of_times, return_code;
	struct GT_polyline *polyline, *polyline_final, *polyline_initial;
	struct GT_pointset *pointset, *pointset_final, *pointset_initial;
	struct GT_surface *surface, *surface_final, *surface_initial;

	ENTER(morph_gtObject);
	graphics_object = (struct GT_object *)NULL;
	if ((0<=proportion)&&(proportion<=1)&&initial&&final&&
		(initial->object_type==final->object_type) &&
		(0 < (number_of_times = initial->number_of_times)) &&
		(number_of_times == final->number_of_times) &&
		initial->times && initial->primitive_lists &&
		final->times && final->primitive_lists /*&&
		(initial->default_material == final->default_material)*/)
	{
		if (graphics_object=CREATE(GT_object)(name,initial->object_type,
				initial->default_material))
		{
			return_code = 1;
			for (i = 0; (i < number_of_times) && return_code; i++)
			{
				if (initial->times[i] == final->times[i])
				{
					time = initial->times[i];
					switch (initial->object_type)
					{
						case g_POINTSET:
						{
							pointset_initial = initial->primitive_lists[i].gt_pointset.first;
							pointset_final = final->primitive_lists[i].gt_pointset.first;
							while (pointset_initial && pointset_final && return_code)
							{
								if (pointset = morph_GT_pointset(proportion, pointset_initial,
									pointset_final))
								{
									if (!GT_OBJECT_ADD(GT_pointset)(graphics_object, time,
										pointset))
									{
										DESTROY(GT_pointset)(&pointset);
										return_code = 0;
									}
								}
								else
								{
									return_code = 0;
								}
								pointset_initial = pointset_initial->ptrnext;
								pointset_final = pointset_final->ptrnext;
							}
							if (pointset_initial || pointset_final)
							{
								return_code = 0;
							}
						} break;
						case g_POLYLINE:
						{
							polyline_initial = initial->primitive_lists[i].gt_polyline.first;
							polyline_final= final->primitive_lists[i].gt_polyline.first;
							while (polyline_initial && polyline_final && return_code)
							{
								if (polyline = morph_GT_polyline(proportion, polyline_initial,
									polyline_final))
								{
									if (!GT_OBJECT_ADD(GT_polyline)(graphics_object, time,
										polyline))
									{
										DESTROY(GT_polyline)(&polyline);
										return_code = 0;
									}
								}
								else
								{
									return_code = 0;
								}
								polyline_initial = polyline_initial->ptrnext;
								polyline_final = polyline_final->ptrnext;
						}
							if (polyline_initial || polyline_final)
							{
								return_code = 0;
							}
						} break;
						case g_SURFACE:
						{
							surface_initial = initial->primitive_lists[i].gt_surface.first;
							surface_final= final->primitive_lists[i].gt_surface.first;
							while (surface_initial && surface_final && return_code)
							{
								if (surface = morph_GT_surface(proportion, surface_initial,
									surface_final))
								{
									if (!GT_OBJECT_ADD(GT_surface)(graphics_object, time,
										surface))
									{
										DESTROY(GT_surface)(&surface);
										return_code = 0;
									}
								}
								else
								{
									return_code = 0;
								}
								surface_initial = surface_initial->ptrnext;
								surface_final = surface_final->ptrnext;
							}
							if (surface_initial || surface_final)
							{
								return_code = 0;
							}
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"morph_gtObject.  Invalid graphics element type");
							return_code = 0;
						} break;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"morph_gtObject.  Graphics object have different times");
					return_code = 0;
				}
			}
			if (!return_code)
			{
				DESTROY(GT_object)(&graphics_object);
				graphics_object = (struct GT_object *)NULL;
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
	Triple *normallist,*point,*tangentlist,*texturelist;

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
		if (initial->tangentlist)
		{
			if (ALLOCATE(tangentlist,Triple,number_of_nodes))
			{
				for (i=0;i<number_of_nodes;i++)
				{
					for (j=0;j<3;j++)
					{
						tangentlist[i][j]=initial->tangentlist[i][j];
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
			tangentlist = (Triple *)NULL;
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
					if (tangentlist)
					{
						DEALLOCATE(tangentlist);
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
					if (tangentlist)
					{
						DEALLOCATE(tangentlist);
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
				initial->n_pts1,initial->n_pts2,point,normallist,tangentlist,texturelist,
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
				if (tangentlist)
				{
					DEALLOCATE(tangentlist);
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
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Creates a new GT_object which is the transformation of <object>.
Only surfaces are implemented at the moment.
Normals are not updated (wavefront export doesn't use normals anyway).
==============================================================================*/
{
	int i, number_of_times, return_code;
	struct GT_object *graphics_object;
	struct GT_surface *surface_input;
	union GT_primitive_list *primitive_list;

	ENTER(transform_GT_object);
	graphics_object = (struct GT_object *)NULL;
	if (object)
	{
		if (graphics_object=CREATE(GT_object)(object->name,object->object_type,
			object->default_material))
		{
			return_code = 1;
			number_of_times = object->number_of_times;
			if ((0 == number_of_times) || object->primitive_lists)
			{
				for (i = 0; (i < number_of_times) && return_code; i++)
				{
					primitive_list = object->primitive_lists + i;
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
							surface_input = primitive_list->gt_surface.first;
							while (surface_input && GT_OBJECT_ADD(GT_surface)(
								graphics_object, /*time*/0.0,
								transform_GT_surface(surface_input,	transformation)))
							{
								surface_input = surface_input->ptrnext;
							}
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"transform_GT_object.  Invalid graphics element type");
							return_code = 0;
						} break;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"transform_GT_object.  Invalid primitive_lists");
				return_code = 0;
			}
			if (!return_code)
			{
				DESTROY(GT_object)(&graphics_object);
				graphics_object = (struct GT_object *)NULL;
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
	Triple *point_list, Triple *axis1_list, Triple *axis2_list,
	Triple *axis3_list, Triple *scale_list, struct GT_object *glyph,
	struct Graphics_font *font, char **labels, int n_data_components, GTDATA *data,
	int label_bounds_dimension, int label_bounds_components, float *label_bounds,
	int object_name, int *names)
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Allocates memory and assigns fields for a GT_glyph_set. The glyph set shows
the object <glyph> at the specified <number_of_points> with positions given in
<point_list>, and principal axes in <axis1_list>, <axis2_list> and <axis3_list>.
The magnitude of these axes control scaling of the glyph at each point, while
their orientations - which need not be orthogonal - effect rotations and skew.
There magnitudes also multiplied by the <scale_list> values, 1 value per axis,
which permit certain glyphs to reverse direction with negative values.
The optional <labels> parameter is an array of strings to be written beside each
glyph, while the optional <data> of number <n_data_components> per glyph allows
colouring of the glyphs by a spectrum.
The glyph_set will be marked as coming from the <object_name>, and integer
identifier, while the optional <names> contains an integer identifier per point.
Note: All arrays passed to this routine are owned by the new GT_glyph_set
and are deallocated by its DESTROY function.
==============================================================================*/
{
	struct GT_glyph_set *glyph_set;

	ENTER(CREATE(GT_glyph_set));
	if ((0 < number_of_points) && point_list && axis1_list && axis2_list &&
		axis3_list && scale_list && glyph && (!labels || font))
	{
		if (ALLOCATE(glyph_set,struct GT_glyph_set,1))
		{
			glyph_set->number_of_points = number_of_points;
			glyph_set->point_list = point_list;
			glyph_set->axis1_list = axis1_list;
			glyph_set->axis2_list = axis2_list;
			glyph_set->axis3_list = axis3_list;
			glyph_set->scale_list = scale_list;
			glyph_set->glyph = ACCESS(GT_object)(glyph);
			if (font)
			{
				glyph_set->font = ACCESS(Graphics_font)(font);
			}
			else
			{
				glyph_set->font = (struct Graphics_font *)NULL;
			}
			glyph_set->labels = labels;
			glyph_set->n_data_components = n_data_components;
 			glyph_set->data = data;
			glyph_set->label_bounds_dimension = label_bounds_dimension;
			glyph_set->label_bounds_components = label_bounds_components;
			glyph_set->label_bounds = label_bounds;

			glyph_set->object_name = object_name;
			glyph_set->auxiliary_object_name = 0;
			glyph_set->names = names;
			glyph_set->ptrnext = (struct GT_glyph_set *)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(GT_glyph_set).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(GT_glyph_set).  Invalid argument(s)");
		glyph_set = (struct GT_glyph_set *)NULL;
	}
	LEAVE;

	return (glyph_set);
} /* CREATE(GT_glyph_set) */

int DESTROY(GT_glyph_set)(struct GT_glyph_set **glyph_set_address)
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Frees the frees the memory for <**glyph_set_address> and sets
<*glyph_set_address> to NULL.
==============================================================================*/
{
	char *label, **labels;
	int return_code, i;
	struct GT_glyph_set *glyph_set;

	ENTER(DESTROY(GT_glyph_set));
	if (glyph_set_address && (glyph_set = *glyph_set_address))
	{
		DEALLOCATE(glyph_set->point_list);
		DEALLOCATE(glyph_set->axis1_list);
		DEALLOCATE(glyph_set->axis2_list);
		DEALLOCATE(glyph_set->axis3_list);
		DEALLOCATE(glyph_set->scale_list);
		DEACCESS(GT_object)(&(glyph_set->glyph));
		if (glyph_set->font)
		{
			DEACCESS(Graphics_font)(&(glyph_set->font));
		}
		if (labels = glyph_set->labels)
		{
			for (i = glyph_set->number_of_points; 0 < i; i--)
			{
				label = *labels;
				DEALLOCATE(label);
				labels++;
			}
			DEALLOCATE(glyph_set->labels);
		}
		if (glyph_set->label_bounds)
		{
			DEALLOCATE(glyph_set->label_bounds);
		}
		if (glyph_set->data)
		{
			DEALLOCATE(glyph_set->data);
		}
		if (glyph_set->names)
		{
			DEALLOCATE(glyph_set->names);
		}
		DEALLOCATE(*glyph_set_address);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(GT_glyph_set).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(GT_glyph_set) */

int GT_glyph_set_set_integer_identifier(struct GT_glyph_set *glyph_set,
	int identifier)
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Sets the integer identifier used by the graphics to distinguish this object.
==============================================================================*/
{
	int return_code;

	ENTER(GT_glyph_set_set_integer_identifier);
	if (glyph_set)
	{
		glyph_set->object_name = identifier;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_glyph_set_set_integer_identifier.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_glyph_set_set_integer_identifier */

int GT_glyph_set_set_auxiliary_integer_identifier(struct GT_glyph_set *glyph_set,
	int identifier)
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Sets the integer identifier used by the graphics to distinguish this object.
==============================================================================*/
{
	int return_code;

	ENTER(GT_glyph_set_set_auxiliary_integer_identifier);
	if (glyph_set)
	{
		glyph_set->auxiliary_object_name = identifier;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_glyph_set_set_auxiliary_integer_identifier.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_glyph_set_set_auxiliary_integer_identifier */

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
		nurbs->object_name=0;

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
			DEALLOCATE(*nurbs);
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

int GT_nurbs_set_integer_identifier(struct GT_nurbs *nurbs,
	int identifier)
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Sets the integer identifier used by the graphics to distinguish this object.
==============================================================================*/
{
	int return_code;

	ENTER(GT_nurbs_set_integer_identifier);
	if (nurbs)
	{
		nurbs->object_name = identifier;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_nurbs_set_integer_identifier.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_nurbs_set_integer_identifier */

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
	gtMarkerType marker_type,float marker_size,int n_data_components,
	int object_name, GTDATA *data, struct Graphics_font *font)
/*******************************************************************************
LAST MODIFIED : 18 November 2005

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
	if (position && (!text || font))
	{
		if (ALLOCATE(point,struct GT_point,1))
		{
			point->position=position;
			point->text=text;
			point->marker_type=marker_type;
			point->marker_size=marker_size;
			point->n_data_components=n_data_components;
			point->data=data;
			if (font)
			{
				point->font = ACCESS(Graphics_font)(font);
			}
			else
			{
				point->font = (struct Graphics_font *)NULL;
			}
			point->object_name = object_name;
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
			DEACCESS(Graphics_font)(&((*point)->font));
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

int GT_point_set_integer_identifier(struct GT_point *point,
	int identifier)
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Sets the integer identifier used by the graphics to distinguish this object.
==============================================================================*/
{
	int return_code;

	ENTER(GT_point_set_integer_identifier);
	if (point)
	{
		point->object_name = identifier;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_point_set_integer_identifier.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_point_set_integer_identifier */

struct GT_pointset *CREATE(GT_pointset)(int n_pts,Triple *pointlist,char **text,
	gtMarkerType marker_type,float marker_size,int n_data_components,GTDATA *data,
	int *names, struct Graphics_font *font)
/*******************************************************************************
LAST MODIFIED : 18 November 2005

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
	if (pointlist && (!text || font))
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
			if (font)
			{
				point_set->font = ACCESS(Graphics_font)(font);
			}
			else
			{
				point_set->font = (struct Graphics_font *)NULL;
			}
			point_set->ptrnext=(struct GT_pointset *)NULL;
			point_set->object_name = 0;
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
			if ((*pointset)->font)
			{
				DEACCESS(Graphics_font)(&((*pointset)->font));
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

int GT_pointset_set_integer_identifier(struct GT_pointset *pointset,
	int identifier)
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Sets the integer identifier used by the graphics to distinguish this object.
==============================================================================*/
{
	int return_code;

	ENTER(GT_pointset_set_integer_identifier);
	if (pointset)
	{
		pointset->object_name = identifier;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_pointset_set_integer_identifier.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_pointset_set_integer_identifier */

int GT_pointset_get_point_list(struct GT_pointset *pointset, int *number_of_points,
	Triple **positions)
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
SAB Added to allow GT_pointset to be hidden but should be replaced.
Gets the <number_of_points> and the <positions> of those points into the 
<pointset> object.  The <positions> pointer is copied directly from the internal
storage.
==============================================================================*/
{
	int return_code;

	ENTER(GT_pointset_set_integer_identifier);
	if (pointset)
	{
		*number_of_points = pointset->n_pts;
		*positions = pointset->pointlist;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_pointset_set_point_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_pointset_set_integer_identifier */

int GT_pointset_set_point_list(struct GT_pointset *pointset, int number_of_points,
	Triple *positions)
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
SAB Added to allow GT_pointset to be hidden but should be replaced.
Sets the <number_of_points> and the <positions> of those points into the 
<pointset> object.  The <positions> pointer is copied directly overwriting the
current storage and the internal data, text and names arrays are messed up.
==============================================================================*/
{
	int return_code;

	ENTER(GT_pointset_set_integer_identifier);
	if (pointset)
	{
		pointset->n_pts = number_of_points;
		pointset->pointlist = positions;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_pointset_set_point_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_pointset_set_integer_identifier */

struct GT_polyline *CREATE(GT_polyline)(enum GT_polyline_type polyline_type,
	int line_width, int n_pts,Triple *pointlist,Triple *normallist,
	int n_data_components,GTDATA *data)
/*******************************************************************************
LAST MODIFIED : 22 April 2004

DESCRIPTION :
Allocates memory and assigns fields for a graphics polyline.
==============================================================================*/
{
	struct GT_polyline *polyline;

	ENTER(CREATE(GT_polyline));
	if (ALLOCATE(polyline,struct GT_polyline,1))
	{
		polyline->polyline_type=polyline_type;
		polyline->line_width=line_width;
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

int GT_polyline_set_integer_identifier(struct GT_polyline *polyline,
	int identifier)
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Sets the integer identifier used by the graphics to distinguish this object.
==============================================================================*/
{
	int return_code;

	ENTER(GT_polyline_set_integer_identifier);
	if (polyline)
	{
		polyline->object_name = identifier;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_polyline_set_integer_identifier.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_polyline_set_integer_identifier */

struct GT_surface *CREATE(GT_surface)(enum GT_surface_type surface_type,
	gtPolygonType polytype,int n_pts1,int n_pts2,Triple *pointlist,
	Triple *normallist, Triple *tangentlist, Triple *texturelist,
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
		surface->tangentlist=tangentlist;
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
			if ((*surface)->tangentlist)
			{
				DEALLOCATE((*surface)->tangentlist);
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

int GT_surface_set_integer_identifier(struct GT_surface *surface,
	int identifier)
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Sets the integer identifier used by the graphics to distinguish this object.
==============================================================================*/
{
	int return_code;

	ENTER(GT_surface_set_integer_identifier);
	if (surface)
	{
		surface->object_name = identifier;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_surface_set_integer_identifier.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_surface_set_integer_identifier */

struct GT_userdef *CREATE(GT_userdef)(void *data,
	int (*destroy_function)(void **),int (*render_function)(void *))
/*******************************************************************************
LAST MODIFIED : 22 January 2003

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
			userdef->object_name = 0;
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

struct GT_voltex *CREATE(GT_voltex)(
	int number_of_vertices, struct VT_iso_vertex **vertex_list,
	int number_of_triangles, struct VT_iso_triangle **triangle_list,
	int n_data_components, int n_texture_coordinates, 
	enum GT_voltex_type voltex_type)
/*******************************************************************************
LAST MODIFIED : 17 February 2006

DESCRIPTION :
Allocates memory and assigns fields for a graphics volume texture.
==============================================================================*/
{
	struct GT_voltex *voltex;

	ENTER(CREATE(GT_voltex));
  if (number_of_vertices && vertex_list &&
	  number_of_triangles && triangle_list)
	{
		if (ALLOCATE(voltex,struct GT_voltex,1))
		{
			voltex->number_of_vertices = number_of_vertices;
			voltex->vertex_list = vertex_list;
			voltex->number_of_triangles = number_of_triangles;
			voltex->triangle_list = triangle_list;
			voltex->vertex_octree = (struct Octree *)NULL;
			voltex->object_name=0;
			voltex->ptrnext=(struct GT_voltex *)NULL;
			voltex->n_data_components=n_data_components;
			voltex->n_texture_coordinates=n_texture_coordinates;
			voltex->voltex_type = voltex_type;
		}
		else
		{
			display_message(ERROR_MESSAGE, "CREATE(GT_voltex).  Insufficient memory");
		}
	}
  else
	{
		display_message(ERROR_MESSAGE, "CREATE(GT_voltex).  Invalid argument(s)");
		voltex = (struct GT_voltex *)NULL;
	}
	LEAVE;

	return (voltex);
} /* CREATE(GT_voltex) */

int DESTROY(GT_voltex)(struct GT_voltex **voltex_address)
/*******************************************************************************
LAST MODIFIED : 26 October 2005

DESCRIPTION :
Frees the memory for <**voltex_address> and sets <*voltex_address> to NULL.
==============================================================================*/
{
	int i, return_code;
	struct GT_voltex *voltex;

	ENTER(DESTROY(GT_voltex));
	if (voltex_address && (voltex = *voltex_address))
	{
		for (i = 0 ; i < voltex->number_of_triangles ; i++)
		{
			DESTROY(VT_iso_triangle)(voltex->triangle_list + i);
		}
		DEALLOCATE(voltex->triangle_list);
		for (i = 0 ; i < voltex->number_of_vertices ; i++)
		{
			DESTROY(VT_iso_vertex)(voltex->vertex_list + i);
		}
		DEALLOCATE(voltex->vertex_list);
		if (voltex->vertex_octree)
		{
			DESTROY(Octree)(&voltex->vertex_octree);
		}

		DEALLOCATE(*voltex_address);
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

int GT_voltex_set_integer_identifier(struct GT_voltex *voltex,
	int identifier)
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Sets the integer identifier used by the graphics to distinguish this object.
==============================================================================*/
{
	int return_code;

	ENTER(GT_voltex_set_integer_identifier);
	if (voltex)
	{
		voltex->object_name = identifier;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_voltex_set_integer_identifier.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_voltex_set_integer_identifier */

int GT_voltex_get_number_of_triangles(struct GT_voltex *voltex)
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Returns the number of polygons used in the GT_voltex.
==============================================================================*/
{
	int number_of_triangles;

	ENTER(GT_voltex_get_number_of_triangles);
	if (voltex)
	{
		number_of_triangles = voltex->number_of_triangles;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_voltex_get_number_of_triangles.  Invalid argument(s)");
		number_of_triangles = 0;
	}
	LEAVE;

	return (number_of_triangles);
} /* GT_voltex_get_number_of_triangles */

struct VT_iso_triangle **GT_voltex_get_triangle_list(struct GT_voltex *voltex)
/*******************************************************************************
LAST MODIFIED : 10 November 2005

DESCRIPTION :
Returns the internal pointer to the triangles used in the GT_voltex.
==============================================================================*/
{
	struct VT_iso_triangle **triangle_list;

	ENTER(GT_voltex_get_number_of_polygons);
	if (voltex)
	{
		triangle_list = voltex->triangle_list;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_voltex_get_triangle_list.  Invalid argument(s)");
		triangle_list = (struct VT_iso_triangle **)NULL;
	}
	LEAVE;

	return (triangle_list);
} /* GT_voltex_get_triangle_list */

int GT_voltex_get_number_of_vertices(struct GT_voltex *voltex)
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Sets the integer identifier used by the graphics to distinguish this object.
==============================================================================*/
{
	int number_of_vertices;

	ENTER(GT_voltex_get_number_of_vertices);
	if (voltex)
	{
		number_of_vertices = voltex->number_of_vertices;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_voltex_get_number_of_vertices.  Invalid argument(s)");
		number_of_vertices = 0;
	}
	LEAVE;

	return (number_of_vertices);
} /* GT_voltex_get_number_of_vertices */

struct VT_iso_vertex **GT_voltex_get_vertex_list(struct GT_voltex *voltex)
/*******************************************************************************
LAST MODIFIED : 10 November 2005

DESCRIPTION :
Returns the internal pointer to the list of vertices used in the GT_voltex.
==============================================================================*/
{
	struct VT_iso_vertex **vertex_list;

	ENTER(GT_voltex_get_vertex_list);
	if (voltex)
	{
		vertex_list = voltex->vertex_list;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_voltex_get_vertex_list.  Invalid argument(s)");
		vertex_list = (struct VT_iso_vertex **)NULL;
	}
	LEAVE;

	return (vertex_list);
} /* GT_voltex_get_vertex_list */

struct GT_object *CREATE(GT_object)(char *name,enum GT_object_type object_type,
	struct Graphical_material *default_material)
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Allocates memory and assigns fields for a graphics object.
==============================================================================*/
{
	struct GT_object *object;
	int return_code;

	ENTER(CREATE(GT_object));
	if (name)
	{
		if (ALLOCATE(object, gtObject, 1) &&
			(object->name = duplicate_string(name)) &&
			(object->selected_graphic_list = CREATE(LIST(Selected_graphic))()))
		{
			object->select_mode=GRAPHICS_NO_SELECT;
			object->times = (float *)NULL;
			object->primitive_lists = (union GT_primitive_list *)NULL;
			object->update_callback_list =
				(struct Graphics_object_callback_data *)NULL;
			object->coordinate_system = g_MODEL_COORDINATES;
			object->glyph_labels_function = (Graphics_object_glyph_labels_function)NULL;
			object->access_count = 0;
			return_code = 1;
			switch (object_type)
			{
				case g_GLYPH_SET:
				case g_NURBS:
				case g_POINT:
				case g_POINTSET:
				case g_POLYLINE:
				case g_SURFACE:
				case g_USERDEF:
				case g_VOLTEX:
				{
					/* these are valid object_types */
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
				object->compile_status = GRAPHICS_NOT_COMPILED;
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
				object->secondary_material=(struct Graphical_material *)NULL;
				object->nextobject=(gtObject *)NULL;
				object->spectrum=(struct Spectrum *)NULL;
				object->number_of_times=0;
				/*???temporary*/
				object->glyph_mirror_mode=0;
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
LAST MODIFIED : 17 March 2003

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
				GT_object_remove_primitives_at_time_number(object, i,
					(GT_object_primitive_object_name_conditional_function *)NULL,
					(void *)NULL);
			}
			DEALLOCATE(object->name);
			if (object->default_material)
			{
				DEACCESS(Graphical_material)(&(object->default_material));
			}
			if (object->selected_material)
			{
				DEACCESS(Graphical_material)(&(object->selected_material));
			}
			if (object->secondary_material)
			{
				DEACCESS(Graphical_material)(&(object->secondary_material));
			}
			if (object->spectrum)
			{
				DEACCESS(Spectrum)(&(object->spectrum));
			}
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

int compile_GT_object(struct GT_object *graphics_object_list, 
	struct GT_object_compile_context *context)
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Rebuilds the display list for each uncreated or morphing graphics object in the
<graphics_object_list>, a simple linked list. The object is compiled using the
supplied <compile_context>.
==============================================================================*/
{
	int i, return_code;
	struct GT_object *graphics_object;

	ENTER(compile_GT_object);
	if (graphics_object_list)
	{
		return_code = 1;
		for (graphics_object=graphics_object_list;graphics_object != NULL;
			graphics_object=graphics_object->nextobject)
		{
			if (GRAPHICS_COMPILED != graphics_object->compile_status)
			{
				/* compile components of graphics objects first */
				if (graphics_object->default_material)
				{
					compile_Graphical_material(graphics_object->default_material,
						(void *)NULL);
				}
				if (graphics_object->selected_material)
				{
					compile_Graphical_material(graphics_object->selected_material,
						(void *)NULL);
				}
				if (graphics_object->secondary_material)
				{
					compile_Graphical_material(graphics_object->secondary_material,
						(void *)NULL);
				}
				switch (graphics_object->object_type)
				{
					case g_GLYPH_SET:
					{
						struct GT_glyph_set *glyph_set;

						if (graphics_object->primitive_lists)
						{
							for (i = 0 ; i < graphics_object->number_of_times ; i++)
							{
								if (glyph_set =
									graphics_object->primitive_lists[i].gt_glyph_set.first)
								{
									compile_GT_object(glyph_set->glyph, context);
									if (glyph_set->font)
									{
										Graphics_font_compile(glyph_set->font, context->graphics_buffer);
									}
								}
							}
						}
					} break;
					case g_POINT:
					{
						struct GT_point *point;

						if (graphics_object->primitive_lists)
						{
							for (i = 0 ; i < graphics_object->number_of_times ; i++)
							{
								if (point =
									graphics_object->primitive_lists[i].gt_point.first)
								{
									if (point->font)
									{
										Graphics_font_compile(point->font, context->graphics_buffer);
									}
								}
							}
						}
					} break;
					case g_POINTSET:
					{
						struct GT_pointset *point_set;

						if (graphics_object->primitive_lists)
						{
							for (i = 0 ; i < graphics_object->number_of_times ; i++)
							{
								if (point_set =
									graphics_object->primitive_lists[i].gt_pointset.first)
								{
									if (point_set->font)
									{
										Graphics_font_compile(point_set->font, context->graphics_buffer);
									}
								}
							}
						}
					} break;
				}
				if (GRAPHICS_NOT_COMPILED == graphics_object->compile_status)
				{
					if (graphics_object->display_list ||
						(graphics_object->display_list=glGenLists(1)))
					{
						glNewList(graphics_object->display_list,GL_COMPILE);
						if ((GRAPHICS_SELECT_ON == graphics_object->select_mode) ||
							(GRAPHICS_DRAW_SELECTED == graphics_object->select_mode))
						{
							context->draw_selected = 1;
							if (graphics_object->selected_material)
							{
								if (FIRST_OBJECT_IN_LIST_THAT(Selected_graphic)(
									(LIST_CONDITIONAL_FUNCTION(Selected_graphic) *)NULL,
									(void *)NULL,graphics_object->selected_graphic_list))
								{
									execute_Graphical_material(
										graphics_object->selected_material);
									render_GT_object_opengl(graphics_object,context);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"compile_GT_object.  "
									"Graphics object %s has no selected material",
									graphics_object->name);
							}
						}
						if (GRAPHICS_DRAW_SELECTED != graphics_object->select_mode)
						{
							context->draw_selected = 0;
							if (graphics_object->default_material)
							{
								execute_Graphical_material(graphics_object->default_material);
							}
							render_GT_object_opengl(graphics_object, context);
						}
						execute_Graphical_material((struct Graphical_material *)NULL);
						glEndList();
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"compile_GT_object.  Unable to get display list");
						return_code = 0;
					}
				}
				if (return_code)
				{
					graphics_object->compile_status = GRAPHICS_COMPILED;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"compile_GT_object.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* compile_GT_object */

int execute_GT_object(struct GT_object *graphics_object_list)
/*******************************************************************************
LAST MODIFIED : 22 November 2005

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
	if (graphics_object_list)
	{
		return_code=1;
		if (graphics_object_list->nextobject)
		{
			glPushName(0);
		}
		graphics_object_no=0;
		for (graphics_object=graphics_object_list;graphics_object != NULL;
			graphics_object=graphics_object->nextobject)
		{
			if (0<graphics_object_no)
			{
				glLoadName((GLuint)graphics_object_no);
			}
			graphics_object_no++;
			if (GRAPHICS_COMPILED == graphics_object->compile_status)
			{
				/* construct object */
				glCallList(graphics_object->display_list);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_GT_object.  Graphics object not compiled");
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

static int GT_object_inform_clients(struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Sends a callback to all registered clients indicating the GT_object_has changed.
==============================================================================*/
{
	int return_code;
	struct Graphics_object_callback_data *callback_data;

	ENTER(GT_object_inform_clients);
	if (graphics_object)
	{
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
			"GT_object_inform_clients.  Invalid graphics object");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_inform_clients */

int GT_object_changed(struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
External modules that change a GT_object should call this routine so that
objects interested in this GT_object will be notified that is has changed.
==============================================================================*/
{
	int return_code;

	ENTER(GT_object_changed);

	if (graphics_object)
	{
		if (graphics_object->nextobject)
		{
			GT_object_changed(graphics_object->nextobject);
		}
		graphics_object->compile_status = GRAPHICS_NOT_COMPILED;
		return_code = GT_object_inform_clients(graphics_object);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_changed.  Invalid graphics object");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_changed */

int GT_object_Graphical_material_change(struct GT_object *graphics_object,
	struct LIST(Graphical_material) *changed_material_list)
/*******************************************************************************
LAST MODIFIED : 20 March 2002

DESCRIPTION :
Tells the <graphics_object> that the materials in the <changed_material_list>
have changed. If any of these materials are used in any graphics object,
changes the compile_status to CHILD_GRAPHICS_NOT_COMPILED and
informs clients of the need to recompile and redraw. Note that if a spectrum is
in use the more expensive GRAPHICS_NOT_COMPILED status is necessarily set.
Note: Passing a NULL <changed_material_list> indicates the equivalent of a
change to any material in use in the linked graphics objects.
==============================================================================*/
{
	int return_code;

	ENTER(GT_object_Graphical_material_change);
	if (graphics_object)
	{
		while (graphics_object)
		{
			if ((graphics_object->default_material &&
				((!changed_material_list) || IS_OBJECT_IN_LIST(Graphical_material)(
					graphics_object->default_material, changed_material_list))) ||
				(graphics_object->selected_material &&
					((!changed_material_list) || IS_OBJECT_IN_LIST(Graphical_material)(
						graphics_object->selected_material, changed_material_list))) ||
				(graphics_object->secondary_material &&
					((!changed_material_list) || IS_OBJECT_IN_LIST(Graphical_material)(
						graphics_object->secondary_material, changed_material_list))))
			{
				if (graphics_object->spectrum)
				{
					/* need to rebuild display list when spectrum in use */
					graphics_object->compile_status = GRAPHICS_NOT_COMPILED;
				}
				else
				{
					if (GRAPHICS_NOT_COMPILED != graphics_object->compile_status)
					{
						graphics_object->compile_status = CHILD_GRAPHICS_NOT_COMPILED;
					}
				}
				GT_object_inform_clients(graphics_object);
			}
			graphics_object = graphics_object->nextobject;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_Graphical_material_change.  Invalid graphics object");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_Graphical_material_change */

int GT_object_Spectrum_change(struct GT_object *graphics_object,
	struct LIST(Spectrum) *changed_spectrum_list)
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
Tells the <graphics_object> that the spectrums in the <changed_spectrum_list>
have changed. If any of these spectrums are used in any graphics object,
changes the compile_status to GRAPHICS_NOT_COMPILED and
informs clients of the need to recompile and redraw.
Note: Passing a NULL <changed_spectrum_list> indicates the equivalent of a
change to any spectrum in use in the linked graphics objects.
==============================================================================*/
{
	int return_code;

	ENTER(GT_object_Spectrum_change);
	if (graphics_object)
	{
		while (graphics_object)
		{
			if (graphics_object->spectrum &&
				((!changed_spectrum_list) || IS_OBJECT_IN_LIST(Spectrum)(
					graphics_object->spectrum, changed_spectrum_list)))
			{
				/* need to rebuild display list when spectrum in use */
				graphics_object->compile_status = GRAPHICS_NOT_COMPILED;
				GT_object_inform_clients(graphics_object);
			}
			graphics_object = graphics_object->nextobject;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_Spectrum_change.  Invalid graphics object");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_Spectrum_change */

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

int GT_object_has_primitives_at_time(struct GT_object *graphics_object,
	float time)
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Returns true if <graphics_object> has primitives stored exactly at <time>.
==============================================================================*/
{
	int return_code, time_number;
	union GT_primitive_list *primitive_list;

	ENTER(GT_object_has_primitives_at_time);
	return_code = 0;
	if (graphics_object)
	{
		if (graphics_object->times && graphics_object->primitive_lists)
		{
			if (0 < (time_number = GT_object_get_time_number(graphics_object, time)))
			{
				/*???RC Note exact real number comparison; could be problematic */
				if (graphics_object->times[time_number - 1] == time)
				{
					primitive_list = graphics_object->primitive_lists + time_number - 1;
					switch (graphics_object->object_type)
					{
						case g_GLYPH_SET:
						{
							if (primitive_list->gt_glyph_set.first)
							{
								return_code = 1;
							}
						} break;
						case g_NURBS:
						{
							if (primitive_list->gt_nurbs.first)
							{
								return_code = 1;
							}
						} break;
						case g_POINT:
						{
							if (primitive_list->gt_point.first)
							{
								return_code = 1;
							}
						} break;
						case g_POINTSET:
						{
							if (primitive_list->gt_pointset.first)
							{
								return_code = 1;
							}
						} break;
						case g_POLYLINE:
						{
							if (primitive_list->gt_polyline.first)
							{
								return_code = 1;
							}
						} break;
						case g_SURFACE:
						{
							if (primitive_list->gt_surface.first)
							{
								return_code = 1;
							}
						} break;
						case g_USERDEF:
						{
							if (primitive_list->gt_userdef.first)
							{
								return_code = 1;
							}
						} break;
						case g_VOLTEX:
						{
							if (primitive_list->gt_voltex.first)
							{
								return_code = 1;
							}
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"GT_object_remove_primitives_at_time.  Unknown object type");
						} break;
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"GT_object_has_primitives_at_time.  Invalid arguments");
	}
	LEAVE;

	return (return_code);
} /* GT_object_has_primitives_at_time */

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

float GT_object_get_time(struct GT_object *graphics_object,int time_number)
/*******************************************************************************
LAST MODIFIED : 18 June 1998

DESCRIPTION :
Returns the time at <time_number> from the graphics_object.
Note that time numbers range from 1 to number_of_times.
==============================================================================*/
{
	float return_time,*times;

	ENTER(GT_object_get_time);
	if (graphics_object)
	{
		if ((0 <= time_number)&&(time_number < graphics_object->number_of_times))
		{
			if (times=graphics_object->times)
			{
				return_time=times[time_number-1];
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
	int time_number;

	ENTER(GT_object_get_nearest_time);
	if (graphics_object)
	{
		if (0<(time_number=graphics_object->number_of_times))
		{
			if (times=graphics_object->times)
			{
				time_number--;
				times += time_number;
				while ((time_number>0)&&(time< *times))
				{
					times--;
					time_number--;
				}
				return_time=times[time_number];
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
LAST MODIFIED : 17 March 2003

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
	int *first, i, j, number_of_positions, number_of_times, position_offset,
		return_code;
	struct Graphics_object_range_struct *graphics_object_range;
	struct GT_glyph_set *glyph_set;
	struct GT_point *point;
	struct GT_pointset *pointset;
	struct GT_polyline *polyline;
	struct GT_surface *surface;
	struct GT_voltex *voltex;
	Triple *position;
	union GT_primitive_list *primitive_list;

	ENTER(get_graphics_object_range);
	if (graphics_object&&(graphics_object_range=
		(struct Graphics_object_range_struct *)graphics_object_range_void)&&
		(maximum=graphics_object_range->maximum)&&
		(minimum=graphics_object_range->minimum)&&
		(first=&(graphics_object_range->first)))
	{
		return_code = 1;
		number_of_times = graphics_object->number_of_times;
		if ((0 == number_of_times) || graphics_object->primitive_lists)
		{
			for (j = 0; (j < number_of_times) && return_code; j++)
			{
				primitive_list = graphics_object->primitive_lists + j;
				switch (graphics_object->object_type)
				{
					case g_POINT:
					{
						point = primitive_list->gt_point.first;
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
					} break;
					case g_GLYPH_SET:
					{
						glyph_set = primitive_list->gt_glyph_set.first;
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
					} break;
					case g_POINTSET:
					{
						pointset = primitive_list->gt_pointset.first;
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
					} break;
					case g_POLYLINE:
					{
						polyline = primitive_list->gt_polyline.first;
						while (polyline)
						{
							switch (polyline->polyline_type)
							{
								case g_PLAIN:
								{
									number_of_positions=polyline->n_pts;
									position_offset=1;
								} break;
								case g_NORMAL:
								{
									number_of_positions=polyline->n_pts;
									position_offset=2;
								} break;
								case g_PLAIN_DISCONTINUOUS:
								{
									number_of_positions=2*(polyline->n_pts);
									position_offset=1;
								} break;
								case g_NORMAL_DISCONTINUOUS:
								{
									number_of_positions=2*(polyline->n_pts);
									position_offset=2;
								} break;
								default:
								{
									number_of_positions=0;
								} break;
							}
							if ((position=polyline->pointlist)&&(0<number_of_positions))
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
							polyline = polyline->ptrnext;
						}
					} break;
					case g_SURFACE:
					{
						/* We will ignore range of morph point lists for morphing types */
						surface = primitive_list->gt_surface.first;
						while (surface)
						{
							switch(surface->polygon)
							{
								case g_QUADRILATERAL:
								{
									number_of_positions=(surface->n_pts1)*(surface->n_pts2);
								} break;
								case g_TRIANGLE:
								{
									number_of_positions=(surface->n_pts1)*
										(surface->n_pts1+1)/2;
								} break;
							}
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
					} break;
					case g_VOLTEX:
					{
						/* We will ignore range of morph point lists for morphing types */
						voltex = primitive_list->gt_voltex.first;
						while (voltex)
						{
							number_of_positions=(voltex->number_of_vertices);
							i = 0;
							if ((0<number_of_positions))
							{
								position = (Triple *)voltex->vertex_list[0]->coordinates;
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
									position = (Triple *)voltex->vertex_list[i]->coordinates;
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
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"get_graphics_object_range.  Invalid graphics object type");
						return_code = 0;
					} break;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_graphics_object_range.  Invalid primitive_lists");
			return_code = 0;
		}
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
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Returns the range of the data values stored in the graphics object.
Returned range generally used to set or enlarge spectrum ranges.
???RC likely to change when multiple data values stored in graphics_objects
for combining several spectrums.
==============================================================================*/
{
	float maximum,minimum;
	int i, j, number_of_times, return_code, first;
	struct Graphics_object_data_range_struct *graphics_object_data_range;
	GTDATA *field_data;
	struct GT_glyph_set *glyph_set;
	struct GT_pointset *pointset;
	struct GT_polyline *polyline;
	struct GT_surface *surface;
	struct GT_voltex *voltex;
	struct VT_iso_vertex **vertex;
	union GT_primitive_list *primitive_list;

	ENTER(get_graphics_object_data_range);
	if (graphics_object && (graphics_object_data_range = (
		struct Graphics_object_data_range_struct *)graphics_object_data_range_void))
	{
		return_code = 1;
		first=graphics_object_data_range->first;
		minimum=graphics_object_data_range->minimum;
		maximum=graphics_object_data_range->maximum;
		number_of_times = graphics_object->number_of_times;
		if ((0 == number_of_times) || graphics_object->primitive_lists)
		{
			for (j = 0; (j < number_of_times) && return_code; j++)
			{
				primitive_list = graphics_object->primitive_lists + j;
				switch (graphics_object->object_type)
				{
					case g_GLYPH_SET:
					{
						glyph_set = primitive_list->gt_glyph_set.first;
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
					} break;
					case g_POINTSET:
					{
						pointset = primitive_list->gt_pointset.first;
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
					} break;
					case g_POLYLINE:
					{
						polyline = primitive_list->gt_polyline.first;
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
					} break;
					case g_SURFACE:
					{
						surface = primitive_list->gt_surface.first;
						/*???RC must have a first surface to get its type */
						if (surface)
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
								} break;
							}
						}
					} break;
					case g_VOLTEX:
					{
						voltex = primitive_list->gt_voltex.first;
						if (first && voltex)
						{
							minimum = voltex->vertex_list[0]->data[0];
							maximum = minimum;
							first = 0;
						}
						while (voltex)
						{
							vertex = voltex->vertex_list;
							for (i=voltex->number_of_vertices;i>0;i--)
							{
								if ((*vertex)->data[0] < minimum)
								{
									minimum = (*vertex)->data[0];
								}
								else
								{
									if ((*vertex)->data[0] > maximum)
									{
										maximum = (*vertex)->data[0];
									}
								}
								vertex++;
							}
							voltex = voltex->ptrnext;
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"get_graphics_object_data_range.  "
							"Invalid graphics object type");
						return_code = 0;
					} break;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_graphics_object_data_range.  Invalid primitive_lists");
			return_code=0;
		}
		graphics_object_data_range->first = first;
		graphics_object_data_range->minimum = minimum;
		graphics_object_data_range->maximum = maximum;
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
LAST MODIFIED : 20 May 2003 \
\
DESCRIPTION : \
Adds <primitive> to <graphics_object> at <time>, creating the new time if it \
does not already exist. If the <primitive> is NULL an empty time is added if \
there is not already one. <primitive> is a NULL-terminated linked-list. \
============================================================================*/ \
{ \
	float *times; \
	int return_code, time_number, i; \
	union GT_primitive_list *primitive_list; \
\
	ENTER(GT_OBJECT_ADD(primitive_type)); \
	if (graphics_object && (gt_object_type == graphics_object->object_type)) \
	{ \
		return_code = 1; \
		time_number= \
			GT_object_get_less_than_or_equal_time_number(graphics_object,time); \
		if ((0 < time_number) && (time==graphics_object->times[time_number-1])) \
		{ \
			primitive_list = graphics_object->primitive_lists + time_number - 1; \
		} \
		else \
		{ \
			/* make a new time & primitive_list */ \
			if (REALLOCATE(times, graphics_object->times, float, \
				graphics_object->number_of_times + 1)) \
			{ \
				graphics_object->times = times; \
				if (REALLOCATE(primitive_list, (graphics_object->primitive_lists), \
					union GT_primitive_list, graphics_object->number_of_times + 1)) \
				{ \
					(graphics_object->primitive_lists) = primitive_list; \
					times += graphics_object->number_of_times; \
					primitive_list += graphics_object->number_of_times; \
					(graphics_object->number_of_times)++; \
					for (i = (graphics_object->number_of_times)-time_number-1; i>0; i--) \
					{ \
						times--; \
						times[1] = times[0]; \
						primitive_list--; \
						primitive_list[1].primitive_var.first = primitive_list->primitive_var.first; \
						primitive_list[1].primitive_var.last = primitive_list->primitive_var.last; \
					} \
					*times = time; \
					primitive_list->primitive_var.first = (struct primitive_type *)NULL; \
					primitive_list->primitive_var.last = (struct primitive_type *)NULL; \
				} \
				else \
				{ \
					return_code = 0; \
				} \
			} \
			else \
			{ \
				return_code = 0; \
			} \
		} \
		if (return_code) \
		{ \
			/* note this adds primitives to the end of the list; used to be start */ \
			if (primitive_list->primitive_var.last) \
			{ \
				primitive_list->primitive_var.last->ptrnext = primitive; \
			} \
			else \
			{ \
				primitive_list->primitive_var.first = primitive; \
				primitive_list->primitive_var.last = primitive; \
			} \
			/* advance last pointer to last primitive in linked-list */ \
			if (primitive_list->primitive_var.last) \
			{ \
				while (primitive_list->primitive_var.last->ptrnext) \
				{ \
					primitive_list->primitive_var.last = \
						primitive_list->primitive_var.last->ptrnext; \
				} \
			} \
			GT_object_changed(graphics_object); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"GT_OBJECT_ADD(" #primitive_type ").  Failed"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"GT_OBJECT_ADD(" #primitive_type ").  Invalid arguments"); \
		return_code = 0; \
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
LAST MODIFIED : 17 March 2003 \
\
DESCRIPTION : \
Returns pointer to the primitive at the given time in graphics_object. \
============================================================================*/ \
{ \
	struct primitive_type *primitive; \
	int time_number; \
\
	ENTER(GT_OBJECT_GET(primitive_type)); \
	primitive = (struct primitive_type *)NULL; \
	if (graphics_object&&(gt_object_type==graphics_object->object_type)) \
	{ \
		if (0 < (time_number = GT_object_get_time_number(graphics_object,time))) \
		{ \
			if (graphics_object->primitive_lists) \
			{ \
				primitive = graphics_object->primitive_lists[time_number - 1]. \
					primitive_var.first; \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"GT_OBJECT_GET(" #primitive_type ").  Invalid primitive_lists"); \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"GT_OBJECT_GET(" #primitive_type ").  No primitives at time %g",time); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"GT_OBJECT_GET(" #primitive_type ").  Invalid arguments"); \
	} \
	LEAVE; \
\
	return (primitive); \
} /* GT_OBJECT_GET(primitive_type) */

DECLARE_GT_OBJECT_GET_FUNCTION(GT_pointset,g_POINTSET,gt_pointset)
DECLARE_GT_OBJECT_GET_FUNCTION(GT_polyline,g_POLYLINE,gt_polyline)
DECLARE_GT_OBJECT_GET_FUNCTION(GT_surface,g_SURFACE,gt_surface)

int GT_object_remove_primitives_at_time(
	struct GT_object *graphics_object, float time,
	GT_object_primitive_object_name_conditional_function *conditional_function,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 25 March 2003

DESCRIPTION :
Removes primitives at <time> from <graphics_object>.
The optional <conditional_function> allows a subset of the primitives to
be removed. This function is called with the object_name integer associated
with each primitive plus the void *<user_data> supplied here. A true result
from the conditional_function causes the primitive to be removed.
==============================================================================*/
{
	int return_code, time_number;

	ENTER(GT_object_remove_primitives_at_time);
	if (graphics_object)
	{
		if (0 < (time_number = GT_object_get_time_number(graphics_object, time)))
		{
			return_code = GT_object_remove_primitives_at_time_number(graphics_object,
				time_number, conditional_function, user_data);
		}
		else
		{
			/* graphics_object does not have that time, so no need to remove */
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_remove_primitives_at_time.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_remove_primitives_at_time */

#if ! defined (SHORT_NAMES)
#define GT_OBJECT_TRANSFER_PRIMITIVES_AT_TIME_NUMBER_(primitive_type) \
	GT_object_transfer_primitives_at_time_number_ ## primitive_type
#else
#define GT_OBJECT_TRANSFER_PRIMITIVES_AT_TIME_NUMBER_(primitive_type) \
	gotptn_ ## primitive_type
#endif
#define GT_OBJECT_TRANSFER_PRIMITIVES_AT_TIME_NUMBER(primitive_type) \
	GT_OBJECT_TRANSFER_PRIMITIVES_AT_TIME_NUMBER_(primitive_type)

#define DECLARE_GT_OBJECT_TRANSFER_PRIMITIVES_AT_TIME_NUMBER_FUNCTION( \
	primitive_type,gt_object_type,primitive_var) \
static int GT_OBJECT_TRANSFER_PRIMITIVES_AT_TIME_NUMBER(primitive_type)( \
	struct GT_object *destination, struct GT_object *source, \
	int time_number, float time) \
/***************************************************************************** \
LAST MODIFIED : 18 March 2003 \
\
DESCRIPTION : \
Transfers the primitives stored at <time_number> in <source> to <time> in \
<destination>. Primitives are added after any in <destination> at <time>. \
============================================================================*/ \
{ \
	int return_code; \
	struct primitive_type *primitive; \
	union GT_primitive_list *primitive_list; \
\
	ENTER(GT_OBJECT_TRANSFER_PRIMITIVES_AT_TIME_NUMBER(primitive_type)); \
	return_code = 0; \
	if (destination && (gt_object_type == destination->object_type) && \
		source && (gt_object_type == source->object_type) && \
		source->primitive_lists && \
		(0 < time_number) && (time_number <= source->number_of_times)) \
	{ \
		primitive_list = source->primitive_lists + time_number - 1; \
		primitive = primitive_list->primitive_var.first; \
		if (GT_OBJECT_ADD(primitive_type)(destination, time, primitive)) \
		{ \
			primitive_list->primitive_var.first = (struct primitive_type *)NULL; \
			primitive_list->primitive_var.last = (struct primitive_type *)NULL; \
			GT_object_changed(source); \
			return_code = 1; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"GT_OBJECT_TRANSFER_PRIMITIVES_AT_TIME_NUMBER(" #primitive_type \
			").  Invalid arguments"); \
	} \
	LEAVE; \
\
	return (return_code); \
} /* GT_OBJECT_TRANSFER_PRIMITIVES_AT_TIME_NUMBER(primitive_type) */

DECLARE_GT_OBJECT_TRANSFER_PRIMITIVES_AT_TIME_NUMBER_FUNCTION(GT_glyph_set, \
	g_GLYPH_SET,gt_glyph_set)
DECLARE_GT_OBJECT_TRANSFER_PRIMITIVES_AT_TIME_NUMBER_FUNCTION(GT_polyline, \
	g_POLYLINE,gt_polyline)
DECLARE_GT_OBJECT_TRANSFER_PRIMITIVES_AT_TIME_NUMBER_FUNCTION(GT_surface, \
	g_SURFACE,gt_surface)
DECLARE_GT_OBJECT_TRANSFER_PRIMITIVES_AT_TIME_NUMBER_FUNCTION(GT_voltex, \
	g_VOLTEX,gt_voltex)

int GT_object_transfer_primitives_at_time(struct GT_object *destination,
	struct GT_object *source, float time)
/*******************************************************************************
LAST MODIFIED : 18 March 2003

DESCRIPTION :
Transfers the primitives stored at exactly <time> in <source> to <time> in
<destination>. Should already have called GT_object_has_primitives_at_time
with <source> to verify it has primitives at that time.
Primitives are added after any in <destination> at <time>.
==============================================================================*/
{
	int return_code, time_number;

	ENTER(GT_object_transfer_primitives_at_time);
	return_code = 0;
	if (destination && source &&
		(destination->object_type == source->object_type) && source->times &&
		(0 < (time_number = GT_object_get_time_number(source, time))) &&
		(source->times[time_number - 1] == time))
	{
		switch (source->object_type)
		{
			case g_GLYPH_SET:
			{
				return_code=GT_OBJECT_TRANSFER_PRIMITIVES_AT_TIME_NUMBER(GT_glyph_set)(
					destination, source, time_number, time);
			} break;
			case g_POLYLINE:
			{
				return_code=GT_OBJECT_TRANSFER_PRIMITIVES_AT_TIME_NUMBER(GT_polyline)(
					destination, source, time_number, time);
			} break;
			case g_SURFACE:
			{
				return_code=GT_OBJECT_TRANSFER_PRIMITIVES_AT_TIME_NUMBER(GT_surface)(
					destination, source, time_number, time);
			} break;
			case g_VOLTEX:
			{
				return_code=GT_OBJECT_TRANSFER_PRIMITIVES_AT_TIME_NUMBER(GT_voltex)(
					destination, source, time_number, time);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"GT_object_remove_primitives_at_time_number.  "
					"Not enabled for this object_type");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_transfer_primitives_at_time.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_transfer_primitives_at_time */

#define DECLARE_GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME_FUNCTION( \
	primitive_type,gt_object_type,primitive_var) \
PROTOTYPE_GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME_FUNCTION( \
	primitive_type) \
/***************************************************************************** \
LAST MODIFIED : 18 March 2003 \
\
DESCRIPTION : \
Returns the first primitives in <graphics_object> at <time> that have the \
given <object_name>, or NULL if there are no primitives or none with the name. \
The extracted primitives are returned in a linked-list. \
============================================================================*/ \
{ \
	int time_number; \
	struct primitive_type *last_primitive, *primitive; \
	union GT_primitive_list *primitive_list; \
\
	ENTER(GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(primitive_type)); \
	primitive = (struct primitive_type *)NULL; \
	if (graphics_object && (gt_object_type == graphics_object->object_type)) \
	{ \
		if (graphics_object->primitive_lists && \
			(0 < (time_number = GT_object_get_time_number(graphics_object, time)))) \
		{ \
			primitive_list = graphics_object->primitive_lists + time_number - 1; \
			if (primitive = primitive_list->primitive_var.first) \
			{ \
				if (primitive->object_name == object_name) \
				{ \
					last_primitive = primitive; \
					while ((last_primitive->ptrnext) && \
						(last_primitive->ptrnext->object_name == object_name)) \
					{ \
						last_primitive = last_primitive->ptrnext; \
					} \
					primitive_list->primitive_var.first = last_primitive->ptrnext; \
					if (last_primitive->ptrnext) \
					{ \
						last_primitive->ptrnext = (struct primitive_type *)NULL; \
					} \
					else \
					{ \
						primitive_list->primitive_var.last = \
							(struct primitive_type *)NULL; \
					} \
					GT_object_changed(graphics_object); \
				} \
				else \
				{ \
					primitive = (struct primitive_type *)NULL; \
				} \
			} \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(" #primitive_type \
			").  Invalid arguments"); \
	} \
	LEAVE; \
\
	return (primitive); \
} /* GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(primitive_type) */

DECLARE_GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME_FUNCTION(GT_polyline, \
	g_POLYLINE,gt_polyline)
DECLARE_GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME_FUNCTION(GT_surface, \
	g_SURFACE,gt_surface)
DECLARE_GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME_FUNCTION(GT_voltex, \
	g_VOLTEX,gt_voltex)

#define DECLARE_GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME_AUXILIARY_FUNCTION( \
	primitive_type,gt_object_type,primitive_var) \
PROTOTYPE_GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME_FUNCTION( \
	primitive_type) \
/***************************************************************************** \
LAST MODIFIED : 18 March 2003 \
\
DESCRIPTION : \
Returns the first primitives in <graphics_object> at <time> that have the \
given <object_name>, or NULL if there are no primitives or none with the name. \
The extracted primitives are returned in a linked-list. \
Version for objects using the auxiliary_object_name in place of object_name. \
============================================================================*/ \
{ \
	int time_number; \
	struct primitive_type *last_primitive, *primitive; \
	union GT_primitive_list *primitive_list; \
\
	ENTER(GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME_AUXILIARY(primitive_type)); \
	primitive = (struct primitive_type *)NULL; \
	if (graphics_object && (gt_object_type == graphics_object->object_type)) \
	{ \
		if (graphics_object->primitive_lists && \
			(0 < (time_number = GT_object_get_time_number(graphics_object, time)))) \
		{ \
			primitive_list = graphics_object->primitive_lists + time_number - 1; \
			if (primitive = primitive_list->primitive_var.first) \
			{ \
				if (primitive->auxiliary_object_name == object_name) \
				{ \
					last_primitive = primitive; \
					while ((last_primitive->ptrnext) && \
						(last_primitive->ptrnext->auxiliary_object_name == object_name)) \
					{ \
						last_primitive = last_primitive->ptrnext; \
					} \
					primitive_list->primitive_var.first = last_primitive->ptrnext; \
					if (last_primitive->ptrnext) \
					{ \
						last_primitive->ptrnext = (struct primitive_type *)NULL; \
					} \
					else \
					{ \
						primitive_list->primitive_var.last = \
							(struct primitive_type *)NULL; \
					} \
					GT_object_changed(graphics_object); \
				} \
				else \
				{ \
					primitive = (struct primitive_type *)NULL; \
				} \
			} \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME_AUXILIARY(" #primitive_type \
			").  Invalid arguments"); \
	} \
	LEAVE; \
\
	return (primitive); \
} /* GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME_AUXILIARY(primitive_type) */

DECLARE_GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME_AUXILIARY_FUNCTION( \
	GT_glyph_set,g_GLYPH_SET,gt_glyph_set)

static int GT_voltex_merge_GT_voltex(struct GT_voltex *existing_voltex,
	struct GT_voltex *voltex)
/*******************************************************************************
LAST MODIFIED : 26 October 2005

DESCRIPTION :
Merge the vertices from <voltex> into <existing_voltex>.  These can be the
same voltex in which case shared vertices will be merged.
==============================================================================*/
{
	int i, j, k, number_of_triangles, return_code, vertex_count;
	struct Octree_object *neighbour, *octree_vertex;
	struct LIST(Octree_object) *neighbours;
	struct VT_iso_vertex *existing_vertex, *vertex; 

	ENTER(GT_voltex_merge_GT_voltex);
	return_code = 1;

	neighbours = CREATE(LIST(Octree_object))();
	if (existing_voltex == voltex)
	{
		vertex_count = 0;
	}
	else
	{
		vertex_count = existing_voltex->number_of_vertices;
		REALLOCATE(existing_voltex->vertex_list, 
			existing_voltex->vertex_list, struct VT_iso_vertex *, 
			vertex_count + voltex->number_of_vertices);
	}
	for (i = 0 ; i < voltex->number_of_vertices ; i++)
	{
		vertex = voltex->vertex_list[i];
		Octree_add_objects_near_coordinate_to_list(
			existing_voltex->vertex_octree,
			/*dimension*/3, vertex->coordinates,
			/*radius*/0.001, neighbours);
		if (0 == NUMBER_IN_LIST(Octree_object)(neighbours))
		{
			octree_vertex = CREATE(Octree_object)(/*dimension*/3,
				vertex->coordinates);
			if ((existing_voltex != voltex) || (vertex_count < i))
			{
				existing_voltex->vertex_list[vertex_count] = vertex;
				vertex->index = vertex_count;
			}

#if defined (DEBUG)
			/* Move the points so that if they aren't using the same vertex from adjacent triangles
				then they won't stay connected */
			existing_voltex->vertex_list[vertex_count]->coordinates[0] += (float)vertex_count / 1000.0;
			existing_voltex->vertex_list[vertex_count]->coordinates[1] += (float)vertex_count / 1000.0;
			existing_voltex->vertex_list[vertex_count]->coordinates[2] += (float)vertex_count / 1000.0;
#endif /* defined (DEBUG) */

			Octree_object_set_user_data(octree_vertex, (void *)vertex);
			Octree_add_object(existing_voltex->vertex_octree,
				octree_vertex);
			vertex_count++;
		}
		else
		{
			/* Just collapse this vertex to the first one */
			neighbour = FIRST_OBJECT_IN_LIST_THAT(Octree_object)(
				(LIST_CONDITIONAL_FUNCTION(Octree_object) *)NULL,
				(void *)NULL, neighbours);
			existing_vertex = (struct VT_iso_vertex *) Octree_object_get_user_data(neighbour);
			REMOVE_ALL_OBJECTS_FROM_LIST(Octree_object)(neighbours);

			number_of_triangles = existing_vertex->number_of_triangles;
			if (REALLOCATE(existing_vertex->triangles, existing_vertex->triangles,
					struct VT_iso_triangle *, number_of_triangles + vertex->number_of_triangles))
			{
				for (j = 0 ; j < vertex->number_of_triangles ;j++)
				{
					for (k = 0 ; k < 3 ; k++)
					{
						if (vertex->triangles[j]->vertices[k] == vertex)
						{
							vertex->triangles[j]->vertices[k] = existing_vertex;
						}
					}
					existing_vertex->triangles[number_of_triangles + j] = vertex->triangles[j];
				}
				existing_vertex->number_of_triangles += vertex->number_of_triangles;
			}
			else
			{
				display_message(ERROR_MESSAGE, "GT_voltex_merge_GT_voltex.  "
					"Unable to allocate triangle pointer array.");
			}

			/* Just add normals as they haven't been normalised yet and so their
				magnitudes indicate the number of triangles contributing to the vector */
			existing_vertex->normal[0] += vertex->normal[0];
			existing_vertex->normal[1] += vertex->normal[1];
			existing_vertex->normal[2] += vertex->normal[2];

			/* Just keep the original data value for now.  Could try and accumulate
				and then average them out later. */

			/* No longer need this vertex */
			DESTROY(VT_iso_vertex)(&vertex);
		}
	}
	/* Update triangle pointers */
	if (existing_voltex != voltex)
	{
		REALLOCATE(existing_voltex->triangle_list, 
			existing_voltex->triangle_list, struct VT_iso_triangle *, 
			existing_voltex->number_of_triangles + voltex->number_of_triangles);
		for (i = 0 ; i < voltex->number_of_triangles ; i++)
		{
			existing_voltex->triangle_list[i + existing_voltex->number_of_triangles]
				= voltex->triangle_list[i];
			existing_voltex->triangle_list[i]->index += existing_voltex->number_of_triangles;
		}
		existing_voltex->number_of_triangles += voltex->number_of_triangles;

		voltex->number_of_vertices = 0;
		DEALLOCATE(voltex->vertex_list);
		voltex->number_of_triangles = 0;
		DEALLOCATE(voltex->triangle_list);
	}
	REALLOCATE(existing_voltex->vertex_list, 
		existing_voltex->vertex_list, struct VT_iso_vertex *, 
		vertex_count);
	existing_voltex->number_of_vertices = vertex_count;
	DESTROY(LIST(Octree_object))(&neighbours);

	LEAVE;

	return (return_code);
}

int GT_object_merge_GT_voltex(struct GT_object *graphics_object,
	struct GT_voltex *voltex)
/*******************************************************************************
LAST MODIFIED : 26 October 2005

DESCRIPTION :
If <graphics_object> does not already contain a GT_voltex then the <voltex> is
added in the normal way.  If a GT_voltex is already contained in the 
<graphics_object> then the new <voltex> is merged into the existing one and 
any co-located vertices are merged, stitching the two voltexes together.
==============================================================================*/
{
	int return_code, time_number;
	struct GT_voltex *existing_voltex;

	ENTER(GT_object_merge_GT_voltex);
	if (graphics_object && (g_VOLTEX == graphics_object->object_type) && voltex)
	{
		if (0 < (time_number = GT_object_get_time_number(graphics_object,
			/*time*/0)))
		{
			if (graphics_object->primitive_lists)
			{
				existing_voltex = graphics_object->primitive_lists[time_number - 1].
					gt_voltex.first;
				return_code = GT_voltex_merge_GT_voltex(existing_voltex,
 					voltex);
				DESTROY(GT_voltex)(&voltex);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"GT_object_merge_GT_voltex.  Invalid primitive_lists");
			}
		}
		else
		{
			if (!voltex->vertex_octree)
			{
				voltex->vertex_octree = CREATE(Octree)();
				/* Merge the voltex with itself to match colocated vertices */
				GT_voltex_merge_GT_voltex(voltex, voltex);
			}
			return_code=GT_OBJECT_ADD(GT_voltex)(graphics_object, /*time*/0,
				voltex);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_merge_GT_voltex.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_merge_GT_voltex */

int GT_object_decimate_GT_voltex(struct GT_object *graphics_object,
	double threshold_distance)
/*******************************************************************************
LAST MODIFIED : 11 November 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code, time_number;
	struct GT_voltex *voltex;

	ENTER(GT_object_decimate_GT_voltex);
	if (graphics_object && (g_VOLTEX == graphics_object->object_type))
	{
		if (0 < (time_number = GT_object_get_time_number(graphics_object,
			/*time*/0)))
		{
			if (graphics_object->primitive_lists)
			{
				voltex = graphics_object->primitive_lists[time_number - 1].
					gt_voltex.first;
				while (voltex)
				{
					GT_voltex_decimate_triangles(voltex, threshold_distance);
					voltex = voltex->ptrnext;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"GT_object_decimate_GT_voltex.  Invalid primitive_lists");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"GT_object_decimate_GT_voltex.  Graphics object does not contain a voltex.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_decimate_GT_voltex.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_decimate_GT_voltex */

static int GT_voltex_normalise_normals(struct GT_voltex *voltex)
/*******************************************************************************
LAST MODIFIED : 28 October 2005

DESCRIPTION :
If a GT_voltex is contained in the <graphics_object> then normals are 
calculated for each of the VT_iso_vertices using the surrounding triangles.
==============================================================================*/
{
	int i, return_code;

	ENTER(GT_voltex_normalise_normals);
	if (voltex)
	{
		for (i = 0 ; i < voltex->number_of_vertices ; i++)
		{
			VT_iso_vertex_normalise_normal(voltex->vertex_list[i]);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_voltex_normalise_normals.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_voltex_normalise_normals */

int GT_object_normalise_GT_voltex_normals(struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 28 October 2005

DESCRIPTION :
If a GT_voltex is contained in the <graphics_object> then normals are 
normalised for each of the VT_iso_vertices using the surrounding triangles.
==============================================================================*/
{
	int return_code, time_number;
	struct GT_voltex *voltex;

	ENTER(GT_object_normalise_GT_voltex_normals);
	if (graphics_object && (g_VOLTEX == graphics_object->object_type))
	{
		if (0 < (time_number = GT_object_get_time_number(graphics_object,
			/*time*/0)))
		{
			if (graphics_object->primitive_lists)
			{
				voltex = graphics_object->primitive_lists[time_number - 1].
					gt_voltex.first;
				while (voltex)
				{
					GT_voltex_normalise_normals(voltex);
					voltex = voltex->ptrnext;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"GT_object_normalise_GT_voltex_normals.  Invalid primitive_lists");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"GT_object_normalise_GT_voltex_normals.  Graphics object does not contain a voltex.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_normalise_GT_voltex_normals.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_normalise_GT_voltex_normals */

enum Graphics_select_mode GT_object_get_select_mode(
	struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Gets the default_select_mode of a GT_object.
==============================================================================*/
{
	enum Graphics_select_mode select_mode;

	ENTER(GT_object_get_select_mode);
	if (graphics_object)
	{
		select_mode = graphics_object->select_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_get_select_mode.  Invalid argument(s)");
		select_mode = GRAPHICS_NO_SELECT;
	}
	LEAVE;

	return (select_mode);
} /* GT_object_get_select_mode */

int GT_object_set_select_mode(struct GT_object *graphics_object,
	enum Graphics_select_mode select_mode)
/*******************************************************************************
LAST MODIFIED : 7 July 2000

DESCRIPTION :
Sets the select_mode of the <graphics_object>.
==============================================================================*/
{
	int return_code;

	ENTER(GT_object_set_select_mode);
	if (graphics_object && (
		(GRAPHICS_SELECT_ON == select_mode) ||
		(GRAPHICS_NO_SELECT == select_mode) ||
		(GRAPHICS_DRAW_SELECTED == select_mode) ||
		(GRAPHICS_DRAW_UNSELECTED == select_mode)))
	{
		graphics_object->select_mode=select_mode;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_set_select_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_set_select_mode */

int GT_object_get_glyph_mirror_mode(struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Gets the glyph_mirror_mode of a GT_object -- true or false.
???RC temporary until we have a separate struct Glyph.
==============================================================================*/
{
	int glyph_mirror_mode;

	ENTER(GT_object_get_glyph_mirror_mode);
	if (graphics_object)
	{
		glyph_mirror_mode = graphics_object->glyph_mirror_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_set_glyph_mirror_mode.  Invalid argument(s)");
		glyph_mirror_mode = 0;
	}
	LEAVE;

	return (glyph_mirror_mode);
} /* GT_object_get_glyph_mirror_mode */

int GT_object_set_glyph_mirror_mode(struct GT_object *graphics_object,
	int glyph_mirror_mode)
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Sets the glyph_mirror_mode of the <graphics_object> to true or false.
???RC temporary until we have a separate struct Glyph.
==============================================================================*/
{
	int return_code;

	ENTER(GT_object_set_glyph_mirror_mode);
	if (graphics_object)
	{
		graphics_object->glyph_mirror_mode = glyph_mirror_mode;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_set_glyph_mirror_mode.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_set_glyph_mirror_mode */

Graphics_object_glyph_labels_function Graphics_object_get_glyph_labels_function(
	struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 19 September 2005

DESCRIPTION :
Gets the glyph_labels_function of the <graphics_object>.
This function enables a custom, per compile, labelling for a graphics object 
==============================================================================*/
{
	Graphics_object_glyph_labels_function glyph_labels_function;

	ENTER(GT_object_get_glyph_labels_function);
	if (graphics_object)
	{
		glyph_labels_function = graphics_object->glyph_labels_function;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_set_glyph_labels_function.  Invalid argument(s)");
		glyph_labels_function = 0;
	}
	LEAVE;

	return (glyph_labels_function);
} /* GT_object_get_glyph_labels_function */

int Graphics_object_set_glyph_labels_function(struct GT_object *graphics_object,
	Graphics_object_glyph_labels_function glyph_labels_function)
/*******************************************************************************
LAST MODIFIED : 19 September 2005

DESCRIPTION :
Sets the glyph_labels_function of the <graphics_object>.
This function enables a custom, per compile, labelling for a graphics object 
==============================================================================*/
{
	int return_code;

	ENTER(GT_object_set_glyph_labels_function);
	if (graphics_object)
	{
		graphics_object->glyph_labels_function = glyph_labels_function;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_set_glyph_labels_function.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_set_glyph_labels_function */

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
LAST MODIFIED : 7 July 2000

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
	if (graphics_object&&(GRAPHICS_NO_SELECT != graphics_object->select_mode))
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
			"GT_object_select_graphic.  Invalid argument(s)");
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
		/* SAB Allow NULL material as this is what is required in glyphs where
			the material changes with the data */
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
			"set_GT_object_default_material.  Invalid graphics object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_GT_object_default_material */

struct Graphical_material *get_GT_object_secondary_material
	(struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Gets the secondary_material of a GT_object.
==============================================================================*/
{
	struct Graphical_material *material;

	ENTER(get_GT_object_secondary_material);
	if (graphics_object)
	{
		material = graphics_object->secondary_material;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_GT_object_secondary_material.  Invalid graphics object");
		material = (struct Graphical_material *)NULL;
	}
	LEAVE;

	return (material);
} /* get_GT_object_secondary_material */

int set_GT_object_secondary_material(struct GT_object *graphics_object,
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 20 February 2000

DESCRIPTION :
Sets the secondary_material of a GT_object.
==============================================================================*/
{
	int return_code;

	ENTER(set_GT_object_secondary_material);
	if (graphics_object)
	{
		/* SAB Allow NULL material as this is what is required in glyphs where
			the material changes with the data */
		if (material != graphics_object->secondary_material)
		{
			REACCESS(Graphical_material)(&(graphics_object->secondary_material),
				material);
			GT_object_changed(graphics_object);
		}
	  return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_GT_object_secondary_material.  Invalid graphics object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_GT_object_secondary_material */

int GT_object_set_name(struct GT_object *graphics_object, char *name)
/*******************************************************************************
LAST MODIFIED : 30 April 2003

DESCRIPTION :
Changes the name of <graphics_object> to a copy of <name>.
==============================================================================*/
{
	char *temp_name;
	int return_code;

	ENTER(GT_object_set_name);
	if (graphics_object && name)
	{
		if (temp_name = duplicate_string(name))
		{
			DEALLOCATE(graphics_object->name);
			graphics_object->name = temp_name;
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_set_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_set_name */

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
		spectrum=(struct Spectrum *)spectrum_void;
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

enum GT_coordinate_system GT_object_get_coordinate_system(
	struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 9 June 2005

DESCRIPTION :
Gets the graphical coordinate system of a GT_object.
==============================================================================*/
{
	enum GT_coordinate_system coordinate_system;

	ENTER(GT_object_get_coordinate_system);
	if (graphics_object)
	{
		coordinate_system = graphics_object->coordinate_system;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_get_coordinate_system.  Invalid graphics object");
		coordinate_system = g_MODEL_COORDINATES;
	}
	LEAVE;

	return (coordinate_system);
} /* GT_object_get_coordinate_system */

int GT_object_set_coordinate_system(struct GT_object *graphics_object,
	enum GT_coordinate_system coordinate_system)
/*******************************************************************************
LAST MODIFIED : 9 June 2005

DESCRIPTION :
Sets the graphical coordinate system of a GT_object.
==============================================================================*/
{
	int return_code;

	ENTER(GT_object_set_coordinate_system);
	if (graphics_object)
	{
		graphics_object->coordinate_system = coordinate_system;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_set_coordinate_system.  Invalid graphics object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_object_set_coordinate_system */

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

int resolve_glyph_axes(Triple point, Triple axis1, Triple axis2,
	Triple axis3, Triple scale, int mirror, int reverse, Triple final_point,
	Triple final_axis1, Triple final_axis2, Triple final_axis3)
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Multiplies the three axes by their <scale> to give the final axes, reversing
<final_axis3> if necessary to produce a right handed coordinate system.
If <mirror> is true, then the axes are pointed in the opposite direction.
If <reverse> is true, then the point is shifted to the end of each axis if the
scale is negative for that axis.
==============================================================================*/
{
	int j, return_code;

	ENTER(resolve_glyph_axes);
	if (point && axis1 && axis2 && axis3 && scale &&
		final_point && final_axis1 && final_axis2 && final_axis3)
	{
		return_code = 1;
		for (j = 0; j < 3; j++)
		{
			final_axis1[j] = axis1[j] * scale[0];
			final_axis2[j] = axis2[j] * scale[1];
			final_axis3[j] = axis3[j] * scale[2];
			if (mirror)
			{
				final_axis1[j] = -final_axis1[j];
				final_axis2[j] = -final_axis2[j];
				final_axis3[j] = -final_axis3[j];
			}
			final_point[j] = point[j];
			if (reverse)
			{
				/* shift glyph centre to end of axes if scale negative */
				if (0.0 > scale[0])
				{
					final_point[j] -= final_axis1[j];
				}
				if (0.0 > scale[1])
				{
					final_point[j] -= final_axis2[j];
				}
				if (0.0 > scale[2])
				{
					final_point[j] -= final_axis3[j];
				}
			}
		}
		/* if required, reverse axis3 to maintain right-handed coordinate system */
		if (0.0 > (
			final_axis3[0]*(final_axis1[1]*final_axis2[2] -
				final_axis1[2]*final_axis2[1]) +
			final_axis3[1]*(final_axis1[2]*final_axis2[0] -
				final_axis1[0]*final_axis2[2]) +
			final_axis3[2]*(final_axis1[0]*final_axis2[1] -
				final_axis1[1]*final_axis2[0])))
		{
			final_axis3[0] = -final_axis3[0];
			final_axis3[1] = -final_axis3[1];
			final_axis3[2] = -final_axis3[2];
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "resolve_glyph_axes. Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* resolve_glyph_axes */

struct GT_object_compile_context *CREATE(GT_object_compile_context)(
	float time, struct Graphics_buffer *graphics_buffer
#if defined (OPENGL_API)
	, unsigned int ndc_display_list, unsigned int end_ndc_display_list
#endif /* defined (OPENGL_API) */
)
/*******************************************************************************
LAST MODIFIED : 22 November 2005

DESCRIPTION :
Creates a GT_object_compile_context structure.
==============================================================================*/
{
	struct GT_object_compile_context *context;

	ENTER(CREATE(GT_object_compile_context));
	if (ALLOCATE(context,struct GT_object_compile_context,1))
	{
		context->time = time;
		context->graphics_buffer = graphics_buffer;
		context->draw_selected = 0;
#if defined (OPENGL_API)
		context->ndc_display_list = ndc_display_list;
		context->end_ndc_display_list = end_ndc_display_list;
#endif /* defined (OPENGL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(GT_object_compile_context).  Not enough memory");
	}
	LEAVE;

	return (context);
} /* CREATE(GT_object_compile_context) */

int DESTROY(GT_object_compile_context)(struct GT_object_compile_context **context)
/*******************************************************************************
LAST MODIFIED : 12 October 2005

DESCRIPTION :
Frees the memory for <**context> and sets <*context> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(GT_object_compile_context));
	if (context)
	{
		if (*context)
		{
			DEALLOCATE(*context);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(GT_object_compile_context).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(GT_object_compile_context) */

