/*******************************************************************************
FILE : graphics_object.cpp

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
 *   Shane Blackett <shane@blackett.co.nz>
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
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "zinc/zincconfigure.h"

#include "zinc/field.h"
#include "zinc/fieldsubobjectgroup.h"
#include "zinc/graphicsmaterial.h"
#include "zinc/status.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_group.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "general/octree.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/decimate_voltex.h"
#include "graphics/font.h"
#include "graphics/glyph.hpp"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
#include "graphics/volume_texture.h"
#include "general/message.h"
#if defined (USE_OPENCASCADE)
#include "zinc/fieldcad.h"
#endif /* defined (USE_OPENCASCADE) */
#include "graphics/render_gl.h"
#include "graphics/graphics_object.hpp"
#include "graphics/graphics_object_highlight.hpp"
#include "graphics/graphics_object_private.hpp"
#include "computed_field/computed_field_subobject_group_private.hpp"

/*
Module types
------------
*/

#define GRAPHICS_VERTEX_BUFFER_INITIAL_SIZE (50)

/*****************************************************************************//**
 * Holds the vertex buffer for a particular vertex_type.
*/
struct Graphics_vertex_buffer
{
	/** Number of vertices stored. */
	unsigned int vertex_count;
	/** Type of vertex. */
	Graphics_vertex_array_attribute_type type;
	/** Number of values per vertex */
	unsigned int values_per_vertex;
	/** Maximum number of vertices currently memory is allocated for. */
	unsigned int max_vertex_count;
	/** Vertex buffer memory */
	void *memory;
	/** Cmgui reference count. */
	int access_count;
};

DECLARE_LIST_TYPES(Graphics_vertex_buffer);

/*
Module functions
----------------
*/

int DESTROY(GT_polyline_vertex_buffers)(struct GT_polyline_vertex_buffers **polyline);

PROTOTYPE_DEFAULT_DESTROY_OBJECT_FUNCTION(Graphics_vertex_buffer);
PROTOTYPE_OBJECT_FUNCTIONS(Graphics_vertex_buffer);
PROTOTYPE_LIST_FUNCTIONS(Graphics_vertex_buffer);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Graphics_vertex_buffer,type,
	Graphics_vertex_array_attribute_type);
FULL_DECLARE_INDEXED_LIST_TYPE(Graphics_vertex_buffer);
DECLARE_OBJECT_FUNCTIONS(Graphics_vertex_buffer)
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Graphics_vertex_buffer,type,
	Graphics_vertex_array_attribute_type,compare_int)
DECLARE_INDEXED_LIST_FUNCTIONS(Graphics_vertex_buffer)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Graphics_vertex_buffer,
	type,Graphics_vertex_array_attribute_type,compare_int)

class Graphics_vertex_array_internal
{
public:
	Graphics_vertex_array_type type;
	LIST(Graphics_vertex_buffer) *buffer_list;

	Graphics_vertex_array_internal(Graphics_vertex_array_type type)
		: type(type)
	{
		buffer_list = CREATE(LIST(Graphics_vertex_buffer))();
	}

	~Graphics_vertex_array_internal()
	{
		DESTROY(LIST(Graphics_vertex_buffer))(&buffer_list);
	}

	/** Gets the buffer appropriate for storing this vertex data or
	* creates one in this array if it doesn't already exist.
	* If it does exist but the value_per_vertex does not match then
	* the method return NULL.
	*/
	Graphics_vertex_buffer *get_or_create_vertex_buffer(
		Graphics_vertex_array_attribute_type vertex_type,
		unsigned int values_per_vertex);

	/** Gets the buffer appropriate for storing this vertex data or
	* returns NULL.
	*/
	Graphics_vertex_buffer *get_vertex_buffer_for_attribute(
		Graphics_vertex_array_attribute_type vertex_type);

	/** Gets the buffer of the specified type if it exists or
	* returns NULL.
	*/
	Graphics_vertex_buffer *get_vertex_buffer(
		Graphics_vertex_array_attribute_type vertex_buffer_type);

	template <class value_type> int free_unused_buffer_memory( Graphics_vertex_array_attribute_type vertex_type, const value_type* dummy );

	template <class value_type> int add_attribute(
		Graphics_vertex_array_attribute_type vertex_type,
		const unsigned int values_per_vertex, const unsigned int number_of_values, const value_type *values);

	template <class value_type> int get_attribute(
		Graphics_vertex_array_attribute_type vertex_type,
		unsigned int vertex_index,
		unsigned int number_of_values, value_type *values);

	template <class value_type> int get_vertex_buffer(
		Graphics_vertex_array_attribute_type vertex_buffer_type,
		value_type **vertex_buffer, unsigned int *values_per_vertex,
		unsigned int *vertex_count);
};

static int GT_object_get_time_number(struct GT_object *graphics_object,
	ZnReal time)
/*******************************************************************************
LAST MODIFIED : 18 June 1998

DESCRIPTION :
The graphics object stores a list of times at which primitives/key frames are
kept. This function returns the index into this array that <time> is at,
starting with 1 for the first time, whereas 0 is returned if the time is not
found or there is an error.
==============================================================================*/
{
	ZnReal *times;
	int time_number;

	ENTER(GT_object_get_time_number);
	if (graphics_object)
	{
		if (0<(time_number=graphics_object->number_of_times))
		{
			times=graphics_object->times;
			if (times)
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
	struct GT_object *graphics_object,ZnReal time)
/*******************************************************************************
LAST MODIFIED : 7 July 1998

DESCRIPTION :
The graphics object stores a list of times at which primitives/key frames are
kept. This function returns the index into this array of the first time less
than or equal to <time>, starting with 1 for the first time. A value of 0 is
returned if <time> is lower than any times in the array or an error occurs.
==============================================================================*/
{
	ZnReal *times;
	int time_number;

	ENTER(GT_object_get_less_than_or_equal_time_number);
	if (graphics_object)
	{
		if (0<(time_number=graphics_object->number_of_times))
		{
			times=graphics_object->times;
			if (times)
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
	ZnReal *times; \
	int i, return_code; \
	struct primitive_type *last_primitive, *primitive, **primitive_ptr; \
	union GT_primitive_list *primitive_list; \
\
	ENTER(GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(primitive_type)); \
	if (graphics_object && (gt_object_type == graphics_object->object_type) && \
		(0 < time_number) && (time_number <= graphics_object->number_of_times)) \
	{ \
		times = graphics_object->times; \
		if (times) \
		{ \
			if (graphics_object->primitive_lists) \
			{ \
				primitive_list = graphics_object->primitive_lists + time_number - 1; \
				last_primitive = (struct primitive_type *)NULL; \
				primitive_ptr = &(primitive_list->primitive_var.first); \
				/* remove primitives at this time */ \
				while (NULL != (primitive = *primitive_ptr)) \
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
						graphics_object->times = (ZnReal *)NULL; \
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
	ZnReal *times; \
	int i, return_code; \
	struct primitive_type *last_primitive, *primitive, **primitive_ptr; \
	union GT_primitive_list *primitive_list; \
\
	ENTER(GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(primitive_type)); \
	if (graphics_object && (gt_object_type == graphics_object->object_type) && \
		(0 < time_number) && (time_number <= graphics_object->number_of_times)) \
	{ \
		times = graphics_object->times; \
		if (times) \
		{ \
			if (graphics_object->primitive_lists) \
			{ \
				primitive_list = graphics_object->primitive_lists + time_number - 1; \
				last_primitive = (struct primitive_type *)NULL; \
				primitive_ptr = &(primitive_list->primitive_var.first); \
				/* remove primitives at this time */ \
				while (NULL != (primitive = *primitive_ptr)) \
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
						graphics_object->times = (ZnReal *)NULL; \
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
	int return_code = 0;

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
			case g_POLYLINE_VERTEX_BUFFERS:
			{
				if (graphics_object->number_of_times &&
					graphics_object->primitive_lists)
				{
					graphics_object->number_of_times = 0;
					DESTROY(GT_polyline_vertex_buffers)(
						&graphics_object->primitive_lists->gt_polyline_vertex_buffers);
					DEALLOCATE(graphics_object->primitive_lists);
					DEALLOCATE(graphics_object->times);
				}
				if (graphics_object->vertex_array)
				{
					graphics_object->vertex_array->clear_buffers();
				}
				return_code = 1;
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
const char *GT_object_get_name(struct GT_object *gt_object)
{
	return gt_object->name;
}

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

const char *get_GT_object_type_string(enum GT_object_type object_type)
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Returns a string describing the object type, suitable for writing to file
(and reinterpreting it later).
==============================================================================*/
{
	const char *type_string;

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
		case g_POLYLINE_VERTEX_BUFFERS:
		{
			type_string="POLYLINE_VERTEX_BUFFERS";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"get_GT_object_type_string.  Unknown object type");
			type_string=(const char *)NULL;
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
	const char *compare_type_string;
	int return_code;
	enum GT_object_type temp_obj_type;

	ENTER(get_GT_object_type_from_string);
	if (type_string&&object_type)
	{
		return_code=1;
		temp_obj_type=g_OBJECT_TYPE_BEFORE_FIRST;
		temp_obj_type = (enum GT_object_type)((int)temp_obj_type + 1);
		while ((temp_obj_type<g_OBJECT_TYPE_AFTER_LAST)&&
			(compare_type_string=get_GT_object_type_string(temp_obj_type))&&
			(!fuzzy_string_compare_same_length(compare_type_string,type_string)))
		{
			temp_obj_type = (enum GT_object_type)((int)temp_obj_type + 1);
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

const char *get_GT_polyline_type_string(enum GT_polyline_type polyline_type)
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Returns a string describing the polyline type, suitable for writing to file
(and reinterpreting it later).
==============================================================================*/
{
	const char *type_string;

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
			type_string=(const char *)NULL;
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
	const char *compare_type_string;
	int return_code;
	enum GT_polyline_type temp_poly_type;

	ENTER(get_GT_polyline_type_from_string);
	if (type_string&&polyline_type)
	{
		return_code=1;
		temp_poly_type=g_POLYLINE_TYPE_BEFORE_FIRST;
		temp_poly_type = (enum GT_polyline_type)((int)temp_poly_type + 1);
		while ((temp_poly_type<g_POLYLINE_TYPE_AFTER_LAST)&&
			(compare_type_string=get_GT_polyline_type_string(temp_poly_type))&&
			(!fuzzy_string_compare_same_length(compare_type_string,type_string)))
		{
			temp_poly_type = (enum GT_polyline_type)((int)temp_poly_type + 1);
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

const char *get_GT_surface_type_string(enum GT_surface_type surface_type)
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Returns a string describing the surface type, suitable for writing to file
(and reinterpreting it later).
==============================================================================*/
{
	const char *type_string;

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
			type_string=(const char *)NULL;
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
	const char *compare_type_string;
	int return_code;
	enum GT_surface_type temp_surf_type;

	ENTER(get_GT_surface_type_from_string);
	if (type_string&&surface_type)
	{
		return_code=1;
		temp_surf_type=g_SURFACE_TYPE_BEFORE_FIRST;
		temp_surf_type = (enum GT_surface_type)((int)temp_surf_type + 1);
		while ((temp_surf_type<g_SURFACE_TYPE_AFTER_LAST)&&
			(compare_type_string=get_GT_surface_type_string(temp_surf_type))&&
			(!fuzzy_string_compare_same_length(compare_type_string,type_string)))
		{
			temp_surf_type = (enum GT_surface_type)((int)temp_surf_type + 1);
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

struct GT_glyph_set *morph_GT_glyph_set(ZnReal proportion,
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
	GLfloat *data;
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
		data = 0;
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
				if (ALLOCATE(data,GLfloat,number_of_points*initial->n_data_components))
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
				glyph_set = CREATE(GT_glyph_set)(number_of_points, point_list,
					axis1_list, axis2_list, axis3_list, scale_list, initial->glyph, initial->glyph_repeat_mode,
					initial->base_size, initial->scale_factors, initial->offset,
					initial->font, labels, initial->label_offset, initial->static_label_text,
					initial->n_data_components, data,
					/*label_bounds_dimension*/0, /*label_bounds_components*/0, /*label_bounds*/(ZnReal *)NULL,
					/*label_density_list*/(Triple *)NULL,
					nearest_glyph_set->object_name, names);
				if (glyph_set)
				{
					display_message(ERROR_MESSAGE,
						"morph_GT_glyph_set.  Not going recursive on possible linked list.");
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

struct GT_pointset *morph_GT_pointset(ZnReal proportion,
	struct GT_pointset *initial,struct GT_pointset *final)
/*******************************************************************************
LAST MODIFIED : 16 April 1999

DESCRIPTION :
Creates a new GT_pointset which is the interpolation of two GT_pointsets.
==============================================================================*/
{
	char **source_text,**text;
	ZnReal marker_size;
	GLfloat *data;
	struct GT_pointset *point_set = NULL;
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
				if (ALLOCATE(data,GLfloat,number_of_points*initial->n_data_components))
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
				data=0;
			}
			if (point&&source_text)
			{
				if (ALLOCATE(text,char *,number_of_points))
				{
					for (i=0;text&&(i<number_of_points);i++)
					{
						if (source_text[i])
						{
							if (ALLOCATE(text[i],char,static_cast<int>(strlen(source_text[i])+1)))
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
				point_set=CREATE(GT_pointset)(initial->n_pts,point,text,
					initial->marker_type,marker_size,initial->n_data_components,data,
					morph_names,initial->font);
				if (point_set)
				{
					display_message(ERROR_MESSAGE,
						"morph_GT_pointset.  Not going recursive on possible linked list.");
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

struct GT_polyline *morph_GT_polyline(ZnReal proportion,
	struct GT_polyline *initial,struct GT_polyline *final)
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Creates a new GT_polyline which is the interpolation of two GT_polylines.
==============================================================================*/
{
	GLfloat *data;
	struct GT_polyline *polyline = NULL;
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
					if (ALLOCATE(data,GLfloat,number_of_nodes*initial->n_data_components))
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
					data=0;
				}
			}
			if (point)
			{
				polyline=CREATE(GT_polyline)(initial->polyline_type,
					initial->n_pts,point,normallist,
					initial->n_data_components,data);
				if (polyline)
				{
					display_message(ERROR_MESSAGE,
						"morph_GT_polyline.  Not going recursive on possible linked list.");
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

struct GT_surface *morph_GT_surface(ZnReal proportion,
	struct GT_surface *initial,struct GT_surface *final)
/*******************************************************************************
LAST MODIFIED : 28 November 2003

DESCRIPTION :
Creates a new GT_surface which is the interpolation of two GT_surfaces.
==============================================================================*/
{
	GLfloat *data;
	struct GT_surface *surface = NULL;
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
					if (ALLOCATE(data,GLfloat,number_of_nodes*initial->n_data_components))
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
					data = 0;
				}
			}
			if (point)
			{
				surface=CREATE(GT_surface)(initial->surface_type,initial->render_polygon_mode,initial->polygon,
					initial->n_pts1,initial->n_pts2,point,normallist,tangentlist,texturelist,
					initial->n_data_components,data);
				if (surface)
				{
					/* go recursive in case it is a linked list */
					if (initial->ptrnext&&final->ptrnext)
					{
						display_message(ERROR_MESSAGE,
							"morph_GT_surface.  Going recusive not implemented.");
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

gtObject *morph_gtObject(char *name,ZnReal proportion,gtObject *initial,
	gtObject *final)
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Creates a new gtObject which is the interpolation of two gtObjects.
==============================================================================*/
{
	ZnReal time;
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
		graphics_object=CREATE(GT_object)(name,initial->object_type,
			initial->default_material);
		if (graphics_object)
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
								pointset = morph_GT_pointset(proportion, pointset_initial,
									pointset_final);
								if (pointset)
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
								polyline = morph_GT_polyline(proportion, polyline_initial,
									polyline_final);
								if (polyline)
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
								surface = morph_GT_surface(proportion, surface_initial,
									surface_final);
								if (surface)
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
				DEACCESS(GT_object)(&graphics_object);
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
	ZnReal *transformation)
/*******************************************************************************
LAST MODIFIED : 8 July 1999

DESCRIPTION :
Creates a new GT_surface which is the interpolation of two GT_surfaces.
==============================================================================*/
{
	GLfloat *data = 0;
	struct GT_surface *surface = NULL;
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
				if (ALLOCATE(data,GLfloat,number_of_nodes*initial->n_data_components))
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
				data=0;
			}
		}
		if (point)
		{
			surface=CREATE(GT_surface)(initial->surface_type,initial->render_polygon_mode,initial->polygon,
				initial->n_pts1,initial->n_pts2,point,normallist,tangentlist,texturelist,
				initial->n_data_components,data);
			if (surface)
			{
				/* go recursive in case it is a linked list */
				if (initial->ptrnext)
				{
					display_message(ERROR_MESSAGE,
						"transform_GT_surface.  Going recusive not implemented.");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"transform_GT_surface.  Could not create surface.");
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
	ZnReal *transformation)
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
		graphics_object=CREATE(GT_object)(object->name,object->object_type,
			object->default_material);
		if (graphics_object)
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
				DEACCESS(GT_object)(&graphics_object);
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

struct GT_glyph_set *CREATE(GT_glyph_set)(int number_of_points,
	Triple *point_list, Triple *axis1_list, Triple *axis2_list,
	Triple *axis3_list, Triple *scale_list, struct GT_object *glyph,
	enum Cmiss_glyph_repeat_mode glyph_repeat_mode,
	Triple base_size, Triple scale_factors, Triple offset,
	struct Cmiss_font *font, char **labels, Triple label_offset,
	char *static_label_text[3], int n_data_components, GLfloat *data,
	int label_bounds_dimension, int label_bounds_components, ZnReal *label_bounds,
	Triple *label_density_list,	int object_name, int *names)
{
	struct GT_glyph_set *glyph_set;

	ENTER(CREATE(GT_glyph_set));
	if ((0 < number_of_points) && point_list && ((glyph && axis1_list &&
		axis2_list && axis3_list && scale_list) || (!glyph))  && (!labels || font))
	{
		if (ALLOCATE(glyph_set,struct GT_glyph_set,1))
		{
			glyph_set->number_of_points = number_of_points;
			glyph_set->point_list = point_list;
			glyph_set->axis1_list = axis1_list;
			glyph_set->axis2_list = axis2_list;
			glyph_set->axis3_list = axis3_list;
			glyph_set->scale_list = scale_list;
			if (glyph)
				glyph_set->glyph = ACCESS(GT_object)(glyph);
			else
				glyph_set->glyph = (GT_object *)NULL;
			glyph_set->glyph_repeat_mode = glyph_repeat_mode;
			for (int i = 0; i < 3; ++i)
			{
				glyph_set->base_size[i] = base_size[i];
				glyph_set->scale_factors[i] = scale_factors[i];
				glyph_set->offset[i] = offset[i];
				glyph_set->label_offset[i] = label_offset[i];
				glyph_set->static_label_text[i] =
					(static_label_text && static_label_text[i]) ? duplicate_string(static_label_text[i]) : 0;
			}
			if (font)
				glyph_set->font = ACCESS(Cmiss_font)(font);
			else
				glyph_set->font = (Cmiss_font *)NULL;
			glyph_set->labels = labels;
			glyph_set->n_data_components = n_data_components;
			glyph_set->data = data;
			glyph_set->label_bounds_dimension = label_bounds_dimension;
			glyph_set->label_bounds_components = label_bounds_components;
			glyph_set->label_bounds = label_bounds;
			glyph_set->label_density_list = label_density_list;

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
		for (i = 0; i < 3; i++)
		{
			if (glyph_set->static_label_text[i])
			{
				DEALLOCATE(glyph_set->static_label_text[i]);
			}
		}
		if (glyph_set->font)
		{
			DEACCESS(Cmiss_font)(&(glyph_set->font));
		}
		labels = glyph_set->labels;
		if (labels)
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
		if (glyph_set->label_density_list)
		{
			DEALLOCATE(glyph_set->label_density_list);
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
	int return_code = 1;

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
	int return_code = 1;

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
	int return_code = 1;

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
	gtMarkerType marker_type,ZnReal marker_size,int n_data_components,
	int object_name, GLfloat *data, struct Cmiss_font *font)
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
				point->font = ACCESS(Cmiss_font)(font);
			}
			else
			{
				point->font = (struct Cmiss_font *)NULL;
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
			DEACCESS(Cmiss_font)(&((*point)->font));
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
	int return_code = 1;

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
	gtMarkerType marker_type,ZnReal marker_size,int n_data_components,GLfloat *data,
	int *names, struct Cmiss_font *font)
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
				point_set->font = ACCESS(Cmiss_font)(font);
			}
			else
			{
				point_set->font = (struct Cmiss_font *)NULL;
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
			text=(*pointset)->text;
			if (text)
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
				DEACCESS(Cmiss_font)(&((*pointset)->font));
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
	int n_pts,Triple *pointlist,Triple *normallist,
	int n_data_components,GLfloat *data)
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

/***************************************************************************//**
 * Creates the shared scene information for a GT_polyline_vertex_buffers.
 */
GT_polyline_vertex_buffers *CREATE(GT_polyline_vertex_buffers)(
	GT_polyline_type polyline_type)
{
	struct GT_polyline_vertex_buffers *polyline;

	ENTER(CREATE(GT_polyline_vertex_buffers));
	if (ALLOCATE(polyline,struct GT_polyline_vertex_buffers,1))
	{
		polyline->polyline_type=polyline_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(GT_polyline_vertex_buffers).  Not enough memory");
	}
	LEAVE;

	return (polyline);
} /* CREATE(GT_polyline_vertex_buffers) */

/***************************************************************************//**
 * Destroys the polyline.
 */
int DESTROY(GT_polyline_vertex_buffers)(GT_polyline_vertex_buffers **polyline)
{
	int return_code;

	ENTER(DESTROY(GT_polyline_vertex_buffers));
	if (polyline && *polyline)
	{
		DEALLOCATE(*polyline);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(GT_polyline_vertex_buffers).  "
			"Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(GT_polyline_vertex_buffers) */

int GT_polyline_set_integer_identifier(struct GT_polyline *polyline,
	int identifier)
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Sets the integer identifier used by the graphics to distinguish this object.
==============================================================================*/
{
	int return_code = 1;

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
	enum Cmiss_graphic_render_polygon_mode render_polygon_mode, gtPolygonType polytype,
	int n_pts1,int n_pts2,Triple *pointlist,
	Triple *normallist, Triple *tangentlist, Triple *texturelist,
	int n_data_components,GLfloat *data)
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
		surface->render_polygon_mode=render_polygon_mode;
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
		surface->tile_number=0;
		surface->allocated_size=0;
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
	int return_code = 0;

	ENTER(GT_surface_set_integer_identifier);
	if (surface)
	{
		surface->object_name = identifier;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_surface_set_integer_identifier.  Invalid argument(s)");
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
	int return_code = 0;

	ENTER(GT_voltex_set_integer_identifier);
	if (voltex)
	{
		voltex->object_name = identifier;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_voltex_set_integer_identifier.  Invalid argument(s)");
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

struct GT_object *CREATE(GT_object)(const char *name,enum GT_object_type object_type,
	struct Graphical_material *default_material)
{
	struct GT_object *object;
	int return_code;

	ENTER(CREATE(GT_object));
	if (name)
	{
		if (ALLOCATE(object, gtObject, 1) &&
			(object->name = duplicate_string(name)))
		{
			object->select_mode=CMISS_GRAPHIC_NO_SELECT;
			object->times = (ZnReal *)NULL;
			object->primitive_lists = (union GT_primitive_list *)NULL;
			object->glyph_labels_function = (Graphics_object_glyph_labels_function)NULL;
			object->glyph_type = CMISS_GLYPH_TYPE_INVALID;
			object->texture_tiling = (struct Texture_tiling *)NULL;
			object->vertex_array = (Graphics_vertex_array *)NULL;
			object->access_count = 1;
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
				case g_POLYLINE_VERTEX_BUFFERS:
				{
					object->vertex_array = new Graphics_vertex_array
						(GRAPHICS_VERTEX_ARRAY_TYPE_FLOAT_SEPARATE_DRAW_ARRAYS);
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
				object->position_vertex_buffer_object = 0;
				object->position_values_per_vertex = 0;
				object->colour_vertex_buffer_object = 0;
				object->colour_values_per_vertex = 0;
				object->normal_vertex_buffer_object = 0;
				object->texture_coordinate0_vertex_buffer_object = 0;
				object->vertex_array_object = 0;
				object->multipass_width = 0;
				object->multipass_height = 0;
				object->multipass_vertex_buffer_object = 0;
				object->multipass_frame_buffer_object = 0;
				object->multipass_frame_buffer_texture = 0;
#endif /* defined (OPENGL_API) */
				object->compile_status = GRAPHICS_NOT_COMPILED;
				object->object_type=object_type;
				if (default_material)
				{
					object->default_material=Cmiss_graphics_material_access(default_material);
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
				object->render_line_width = 0.0; // not set: inherit from current state
				object->render_point_size = 0.0; // not set: inherit from current state
			}
			else
			{
				if (0==return_code)
				{
					display_message(ERROR_MESSAGE,
						"CREATE(GT_object).  Insufficient memory");
				}
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
}

/**
 * Frees the memory for the fields of <**object>, frees the memory for <**object>
 * and sets <*object> to NULL.
 */
static int DESTROY(GT_object)(struct GT_object **object_ptr)
{
	int i,return_code;
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
			for (i=object->number_of_times;0<i;i--)
			{
				GT_object_remove_primitives_at_time_number(object, i,
					(GT_object_primitive_object_name_conditional_function *)NULL,
					(void *)NULL);
			}
			DEALLOCATE(object->name);
			if (object->default_material)
			{
				Cmiss_graphics_material_destroy(&(object->default_material));
			}
			if (object->selected_material)
			{
				Cmiss_graphics_material_destroy(&(object->selected_material));
			}
			if (object->secondary_material)
			{
				Cmiss_graphics_material_destroy(&(object->secondary_material));
			}
			if (object->spectrum)
			{
				DEACCESS(Spectrum)(&(object->spectrum));
			}
			if (object->vertex_array)
			{
				delete object->vertex_array;
			}
			if (object->texture_tiling)
			{
				DEACCESS(Texture_tiling)(&object->texture_tiling);
			}
#if defined (OPENGL_API)
			if (object->display_list)
			{
				glDeleteLists(object->display_list,1);
			}
#if defined (GL_VERSION_1_5)
			/* Assume this function is available the vertex buffer object
			 * values are set */
			if (object->position_vertex_buffer_object)
			{
				glDeleteBuffers(1, &object->position_vertex_buffer_object);
			}
			if (object->colour_vertex_buffer_object)
			{
				glDeleteBuffers(1, &object->colour_vertex_buffer_object);
			}
			if (object->normal_vertex_buffer_object)
			{
				glDeleteBuffers(1, &object->normal_vertex_buffer_object);
			}
			if (object->texture_coordinate0_vertex_buffer_object)
			{
				glDeleteBuffers(1, &object->texture_coordinate0_vertex_buffer_object);
			}
			if (object->multipass_vertex_buffer_object)
			{
				glDeleteBuffers(1, &object->multipass_vertex_buffer_object);
			}
			if (object->multipass_frame_buffer_object)
			{
#if defined GL_EXT_framebuffer_object
				glDeleteFramebuffersEXT(1, &object->multipass_frame_buffer_object);
#endif // defined GL_EXT_framebuffer_object
			}
			if (object->multipass_frame_buffer_texture)
			{
				glDeleteTextures(1, &object->multipass_frame_buffer_texture);
			}
#endif /* defined (GL_VERSION_1_5) */
#if defined (GL_VERSION_3_0)  && (NOT_IN_MESA_HEADER_YET)
			if (object->vertex_array_object)
			{
				/* Assume this is safe as the vertex array object is set */
				glDeleteVertexArrays(1, &object->vertex_array_object);
			}
#endif /* defined (GL_VERSION_3_0) */
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

DECLARE_OBJECT_FUNCTIONS(GT_object)

DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(GT_object)

void GT_object_changed(struct GT_object *graphics_object)
{
	while (graphics_object)
	{
		graphics_object->compile_status = GRAPHICS_NOT_COMPILED;
		graphics_object = graphics_object->nextobject;
	}
}

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
				else if ((graphics_object->object_type == g_POLYLINE_VERTEX_BUFFERS) &&
					(graphics_object->secondary_material))
				{
					/* vertex positions are calculated from the secondary material so need to be updated */
					graphics_object->compile_status = GRAPHICS_NOT_COMPILED;
				}
				else
				{
					if (GRAPHICS_NOT_COMPILED != graphics_object->compile_status)
					{
						graphics_object->compile_status = CHILD_GRAPHICS_NOT_COMPILED;
					}
				}
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

int GT_object_has_time(struct GT_object *graphics_object,ZnReal time)
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
	ZnReal time)
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
{
	if (graphics_object)
		return graphics_object->number_of_times;
	return 0;
}

void GT_object_time_change(struct GT_object *graphics_object)
{
	if (graphics_object)
	{
		if ((g_GLYPH_SET == graphics_object->object_type) && graphics_object->primitive_lists)
		{
			// assume only one time and one glyph
			GT_glyph_set *glyph_set = graphics_object->primitive_lists[0].gt_glyph_set.first;
			GT_object_time_change(glyph_set->glyph);
		}
	}
	if (1 < graphics_object->number_of_times)
		GT_object_changed(graphics_object);
}

Graphics_vertex_array *
	GT_object_get_vertex_set(GT_object *graphics_object)
{
	Graphics_vertex_array *set;

	ENTER(GT_object_get_vertex_set);
	if (graphics_object)
	{
		set=graphics_object->vertex_array;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_get_vertex_set.  Invalid arguments");
		set = (Graphics_vertex_array *)NULL;
	}
	LEAVE;

	return (set);
} /* GT_object_get_vertex_set */

ZnReal GT_object_get_time(struct GT_object *graphics_object,int time_number)
/*******************************************************************************
LAST MODIFIED : 18 June 1998

DESCRIPTION :
Returns the time at <time_number> from the graphics_object.
Note that time numbers range from 1 to number_of_times.
==============================================================================*/
{
	ZnReal return_time,*times;

	ENTER(GT_object_get_time);
	if (graphics_object)
	{
		if ((0 <= time_number)&&(time_number < graphics_object->number_of_times))
		{
			times=graphics_object->times;
			if (times)
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

ZnReal GT_object_get_nearest_time(struct GT_object *graphics_object,ZnReal time)
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
	ZnReal return_time,*times;
	int time_number;

	ENTER(GT_object_get_nearest_time);
	if (graphics_object)
	{
		if (0<(time_number=graphics_object->number_of_times))
		{
			times=graphics_object->times;
			if (times)
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
	ZnReal temp;
	GLfloat *maximum,*minimum;
	int *first, i, j, number_of_positions = 0, number_of_times = 0, position_offset = 0,
		return_code = 0;
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
		(maximum = graphics_object_range->maximum)&&
		(minimum = graphics_object_range->minimum)&&
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
							Triple *point = glyph_set->point_list;
							Triple *axis1 = glyph_set->axis1_list;
							Triple *axis2 = glyph_set->axis2_list;
							Triple *axis3 = glyph_set->axis3_list;
							Triple *scale = glyph_set->scale_list;
							Triple temp_axis1, temp_axis2, temp_axis3, temp_point;
							int number_of_points = glyph_set->number_of_points;
							for (i = 0; i < number_of_points; ++i)
							{
								resolve_glyph_axes(glyph_set->glyph_repeat_mode, /*glyph_number*/0,
									glyph_set->base_size, glyph_set->scale_factors, glyph_set->offset,
									*point, *axis1, *axis2, *axis3, *scale,
									temp_point, temp_axis1, temp_axis2, temp_axis3);
								if (*first)
								{
									for (int k = 0; k < 3; ++k)
									{
										maximum[k] = minimum[k] = temp_point[k];
									}
									*first = 0;
								}
								else
								{
									for (int k = 0; k < 3; ++k)
									{
										if (temp_point[k] < minimum[k])
										{
											minimum[k] = temp_point[k];
										}
										else if (temp_point[k] > maximum[k])
										{
											maximum[k] = temp_point[k];
										}
									}
								}
								point++;
								axis1++;
								axis2++;
								axis3++;
								scale++;
							}
							glyph_set = glyph_set->ptrnext;
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
					case g_POLYLINE_VERTEX_BUFFERS:
					{
						GLfloat *vertex_buffer;
						unsigned int values_per_vertex, vertex_count;
						if (graphics_object->vertex_array->get_float_vertex_buffer(
							GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_POSITION,
							&vertex_buffer, &values_per_vertex, &vertex_count))
						{
							if (*first)
							{
								minimum[0]=vertex_buffer[0];
								maximum[0]=minimum[0];
								minimum[1]=vertex_buffer[1];
								maximum[1]=minimum[1];
								minimum[2]=vertex_buffer[2];
								maximum[2]=minimum[2];
								vertex_count--;
								vertex_buffer += values_per_vertex;
								*first=0;
							}
							while (vertex_count>0)
							{
								int i;
								for (i = 0 ; i < 3 ; i++)
								{
									if (vertex_buffer[i]<minimum[i])
									{
										minimum[i]=vertex_buffer[i];
									}
									else if (vertex_buffer[i]>maximum[i])
									{
										maximum[i]=vertex_buffer[i];
									}
								}
								vertex_count--;
								vertex_buffer += values_per_vertex;
							}
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
									switch (surface->surface_type)
									{
									case g_SH_DISCONTINUOUS:
									case g_SH_DISCONTINUOUS_TEXMAP:
									case g_SH_DISCONTINUOUS_STRIP:
									case g_SH_DISCONTINUOUS_STRIP_TEXMAP:
										number_of_positions=(surface->n_pts1)*(surface->n_pts2);
										break;
									case g_SHADED:
									case g_SHADED_TEXMAP:
										number_of_positions=(surface->n_pts1)*(surface->n_pts1+1)/2;
										break;
									default:
										number_of_positions = 0;
										break;
									}
								} break;
								default:
								{
								} break;
							}
							position=surface->pointlist;
							if (position&&(0<number_of_positions))
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
	Graphics_object_data_range *range)
{
	if (graphics_object && range)
	{
		GLfloat *field_data = 0;
		union GT_primitive_list *primitive_list;
		int number_of_times = graphics_object->number_of_times;
		if ((0 == number_of_times) || graphics_object->primitive_lists)
		{
			int i;
			for (int j = 0; j < number_of_times; ++j)
			{
				primitive_list = graphics_object->primitive_lists + j;
				switch (graphics_object->object_type)
				{
					case g_GLYPH_SET:
					{
						struct GT_glyph_set *glyph_set = primitive_list->gt_glyph_set.first;
						while (glyph_set)
						{
							field_data = glyph_set->data;
							if (field_data)
							{
								for (i=glyph_set->number_of_points;i>0;--i)
								{
									range->addValues(glyph_set->n_data_components, field_data);
									field_data += glyph_set->n_data_components;
								}
							}
							glyph_set=glyph_set->ptrnext;
						}
					} break;
					case g_POINTSET:
					{
						struct GT_pointset *pointset = primitive_list->gt_pointset.first;
						while (pointset)
						{
							field_data=pointset->data;
							if (field_data)
							{
								for (i=pointset->n_pts;i>0;i--)
								{
									range->addValues(pointset->n_data_components, field_data);
									field_data += pointset->n_data_components;
								}
							}
							pointset=pointset->ptrnext;
						}
					} break;
					case g_POLYLINE:
					{
						struct GT_polyline *polyline = primitive_list->gt_polyline.first;
						while (polyline)
						{
							field_data=polyline->data;
							if (field_data)
							{
								for (i=polyline->n_pts;i>0;--i)
								{
									range->addValues(polyline->n_data_components, field_data);
									field_data += polyline->n_data_components;
								}
							}
							polyline=polyline->ptrnext;
						}
					} break;
					case g_POLYLINE_VERTEX_BUFFERS:
					{
						GLfloat *vertex_buffer = NULL;
						unsigned int values_per_vertex = 0, vertex_count = 0;
						graphics_object->vertex_array->get_float_vertex_buffer(
							GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_DATA,
							&vertex_buffer, &values_per_vertex, &vertex_count);
						while (vertex_count>0)
						{
							range->addValues(values_per_vertex, vertex_buffer);
							--vertex_count;
							vertex_buffer += values_per_vertex;
						}
					} break;
					case g_SURFACE:
					{
						struct GT_surface *surface = primitive_list->gt_surface.first;
						while (surface)
						{
							field_data=surface->data;
							if (field_data)
							{
								int n_pts = 0;
								switch (surface->surface_type)
								{
								case g_SH_DISCONTINUOUS:
								case g_SH_DISCONTINUOUS_TEXMAP:
								case g_SH_DISCONTINUOUS_STRIP:
								case g_SH_DISCONTINUOUS_STRIP_TEXMAP:
									n_pts = (surface->n_pts1)*(surface->n_pts2);
									break;
								// case g_SHADED:
								// case g_SHADED_TEXMAP:
								default:
									switch (surface->polygon)
									{
									case g_QUADRILATERAL:
										n_pts = (surface->n_pts1)*(surface->n_pts2);
										break;
									case g_TRIANGLE:
										n_pts = (((surface->n_pts1)+1)*(surface->n_pts1))/2;
										break;
									case g_GENERAL_POLYGON:
										// not supported for g_SHADED, g_SHADED_TEXMAP
										break;
									}
									break;
								}
								for (i=n_pts;i>0;--i)
								{
									range->addValues(surface->n_data_components, field_data);
									field_data += surface->n_data_components;
								}
							}
							surface=surface->ptrnext;
						}
					} break;
					case g_VOLTEX:
					{
						struct VT_iso_vertex **vertex;
						struct GT_voltex *voltex = primitive_list->gt_voltex.first;
						while (voltex)
						{
							vertex = voltex->vertex_list;
							for (i=voltex->number_of_vertices;i>0;--i)
							{
								range->addValues(voltex->n_data_components, (*vertex)->data);
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
						return 0;
					} break;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_graphics_object_data_range.  Invalid primitive_lists");
			return 0;
		}
		return 1;
	}
	return 0;
}

int get_graphics_object_time_range(struct GT_object *graphics_object,
	void *graphics_object_time_range_void)
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Enlarges the minimum and maximum time range by that of the graphics_object.
==============================================================================*/
{
	ZnReal *times;
	int return_code;
	struct Graphics_object_time_range_struct *graphics_object_time_range;

	ENTER(get_graphics_object_time_range);
	if (graphics_object&&(graphics_object_time_range=(struct
		Graphics_object_time_range_struct *)graphics_object_time_range_void))
	{
		return_code=1;
		if (0 < graphics_object->number_of_times)
		{
			times=graphics_object->times;
			if (times)
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
	ZnReal *times; \
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
			if (REALLOCATE(times, graphics_object->times, ZnReal, \
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

int GT_OBJECT_ADD(GT_polyline_vertex_buffers)(
	struct GT_object *graphics_object, struct GT_polyline_vertex_buffers *primitive)
{
	int return_code = 0;

	if (graphics_object && !graphics_object->primitive_lists)
	{
		if (ALLOCATE(graphics_object->primitive_lists, union GT_primitive_list, 1) &&
			ALLOCATE(graphics_object->times, ZnReal, 1))
		{
			graphics_object->primitive_lists->gt_polyline_vertex_buffers = primitive;
			graphics_object->times[0] = 0.0;
			graphics_object->number_of_times = 1;
			return_code = 1;
		}
	}
	else
	{
		return_code = 0;
	}
	return (return_code);
}

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
	struct GT_object *graphics_object, ZnReal time,
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
	int return_code = 0, time_number;

	ENTER(GT_object_remove_primitives_at_time);
	if (graphics_object)
	{
		if (graphics_object->object_type == g_POLYLINE_VERTEX_BUFFERS)
		{
			if (graphics_object->number_of_times &&
				graphics_object->primitive_lists)
			{
				graphics_object->number_of_times = 0;
				DESTROY(GT_polyline_vertex_buffers)(
					&graphics_object->primitive_lists->gt_polyline_vertex_buffers);
				DEALLOCATE(graphics_object->primitive_lists);
				DEALLOCATE(graphics_object->times);
			}
			if (graphics_object->vertex_array)
			{
				graphics_object->vertex_array->clear_buffers();
			}
		}
		else
		{
			if (0 < (time_number	= GT_object_get_time_number(graphics_object, time)))
			{
				return_code = GT_object_remove_primitives_at_time_number(
					graphics_object, time_number, conditional_function, user_data);
			}
			else
			{
				/* graphics_object does not have that time, so no need to remove */
				return_code = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_remove_primitives_at_time.  Invalid argument(s)");
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
	int time_number, ZnReal time) \
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
	struct GT_object *source, ZnReal time)
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
			primitive = primitive_list->primitive_var.first; \
			if (primitive) \
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
			primitive = primitive_list->primitive_var.first; \
			if (primitive) \
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

/***************************************************************************//**
 * Merge the vertices from <voltex> into <existing_voltex>. These can be the
 * same voltex in which case shared vertices will be merged.
 *
 * @param vertex_radius_tolerance  Absolute radius within which two vertices
 * are merged into one. Previous version had constant = 0.001.
 */
static int GT_voltex_merge_GT_voltex(struct GT_voltex *existing_voltex,
	struct GT_voltex *voltex, ZnReal vertex_radius_tolerance)
{
	int i, j, k, number_of_triangles, return_code, vertex_count;
	struct Octree_object *neighbour, *octree_vertex;
	struct LIST(Octree_object) *neighbours;
	struct VT_iso_vertex *existing_vertex, *vertex;
	FE_value coordinates[3];

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
		CAST_TO_FE_VALUE(coordinates,vertex->coordinates,3);
		Octree_add_objects_near_coordinate_to_list(
			existing_voltex->vertex_octree,
			/*dimension*/3, coordinates,
			vertex_radius_tolerance, neighbours);
		if (0 == NUMBER_IN_LIST(Octree_object)(neighbours))
		{
			octree_vertex = CREATE(Octree_object)(/*dimension*/3,
				coordinates);
			if ((existing_voltex != voltex) || (vertex_count < i))
			{
				existing_voltex->vertex_list[vertex_count] = vertex;
				vertex->index = vertex_count;
			}

#if defined (DEBUG_CODE)
			/* Move the points so that if they aren't using the same vertex from adjacent triangles
				then they won't stay connected */
			existing_voltex->vertex_list[vertex_count]->coordinates[0] += (ZnReal)vertex_count / 1000.0;
			existing_voltex->vertex_list[vertex_count]->coordinates[1] += (ZnReal)vertex_count / 1000.0;
			existing_voltex->vertex_list[vertex_count]->coordinates[2] += (ZnReal)vertex_count / 1000.0;
#endif /* defined (DEBUG_CODE) */

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
	int return_code = 0, time_number = 0;
	struct GT_voltex *existing_voltex = NULL;

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
					voltex, /*vertex_radius_tolerance*/0.001f);
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
				GT_voltex_merge_GT_voltex(voltex, voltex, /*vertex_radius_tolerance*/0.001f);
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
	int return_code = 0, time_number;
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
				return_code = 1;
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
	int i, return_code = 0;

	ENTER(GT_voltex_normalise_normals);
	if (voltex)
	{
		for (i = 0 ; i < voltex->number_of_vertices ; i++)
		{
			VT_iso_vertex_normalise_normal(voltex->vertex_list[i]);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_voltex_normalise_normals.  Invalid argument(s)");
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
	int return_code = 0, time_number;
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
				return_code = 1;
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
	}
	LEAVE;

	return (return_code);
} /* GT_object_normalise_GT_voltex_normals */

/***************************************************************************//**
 * Creates a GT_voltex representation of the triangles in the supplied surface
 * list.
 * @param surface_list  Linked list of GT_surface to convert. Note: only
 * supports surface types g_SH_DISCONTINUOUS and g_SH_DISCONTINUOUS_TEXMAP.
 * @return  The newly created GT_voltex.
 */
struct GT_voltex *GT_voltex_create_from_GT_surface(
	struct GT_surface *surface_list)
{
	ENTER(GT_voltex_create_from_GT_surface);
	int vertex_count = 0;
	VT_iso_vertex **vertex_list = NULL;
	int triangle_count = 0;
	VT_iso_triangle **triangle_list = NULL;
	GT_surface *surface = surface_list;
	int return_code = 1;
	while ((NULL != surface) && return_code)
	{
		switch (surface->surface_type)
		{
		case g_SH_DISCONTINUOUS:
		case g_SH_DISCONTINUOUS_TEXMAP:
			{
				if (surface->polygon == g_TRIANGLE)
				{
					triangle_count += surface->n_pts1;
					vertex_count += 3*(surface->n_pts1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"GT_voltex_create_from_GT_surface.  Unsupported polygon type");
					return_code = 0;
				}
			} break;
		default:
			{
				display_message(ERROR_MESSAGE,
					"GT_voltex_create_from_GT_surface.  Unsupported surface type");
				return_code = 0;
			} break;
		}
		surface = surface->ptrnext;
	}
	GT_voltex *voltex = NULL;
	int n_data_components = 0;
	int n_texture_coordinates = 0;
	if (return_code && (triangle_count > 0))
	{
		ALLOCATE(vertex_list, struct VT_iso_vertex *, vertex_count);
		if (NULL != vertex_list)
		{
			for (int i = 0; i < vertex_count; i++)
			{
				vertex_list[i] = NULL;
			}
		}
		ALLOCATE(triangle_list, struct VT_iso_triangle *, triangle_count);
		if (NULL != triangle_list)
		{
			for (int i = 0; i < triangle_count; i++)
			{
				triangle_list[i] = NULL;
			}
		}
		n_data_components = surface_list->n_data_components;
		if (NULL != surface_list->texturelist)
		{
			n_texture_coordinates = 3;
		}
		GT_voltex_type voltex_type = (surface_list->render_polygon_mode == CMISS_GRAPHIC_RENDER_POLYGON_SHADED) ?
			g_VOLTEX_SHADED_TEXMAP : g_VOLTEX_WIREFRAME_SHADED_TEXMAP;
		voltex = CREATE(GT_voltex)(vertex_count, vertex_list,
			triangle_count, triangle_list, n_data_components,
			n_texture_coordinates, voltex_type);
	}
	if (voltex)
	{
		surface = surface_list;
		int triangle_index = 0;
		int vertex_index = 0;
		while ((NULL != surface) && return_code)
		{
			int number_of_points = surface->n_pts1*surface->n_pts2;
			for (int i = 0; i < number_of_points; i++)
			{
				VT_iso_vertex *vertex = VT_iso_vertex_create_and_set(
					surface->pointlist[i], surface->normallist[i],
					n_texture_coordinates ? surface->texturelist[i] : NULL,
					n_data_components, surface->data + i*n_data_components);
				if (NULL != vertex)
				{
					vertex_list[vertex_index + i] = vertex;
				}
				else
				{
					return_code = 0;
					break;
				}
			}
			if (return_code)
			{
				switch (surface->surface_type)
				{
					case g_SH_DISCONTINUOUS:
					case g_SH_DISCONTINUOUS_TEXMAP:
					{
						for (int i = 0; i < surface->n_pts1; i++)
						{
							if (g_TRIANGLE == surface->polygon)
							{
								VT_iso_triangle *triangle =
									VT_iso_triangle_create_and_set(vertex_list + vertex_index);
								if (NULL != triangle)
								{
									triangle_list[triangle_index] = triangle;
								}
								else
								{
									return_code = 0;
									break;
								}
								vertex_index += 3;
								triangle_index++;
							}
						}
					} break;
					default:
					{
					} break;
				}
			}
			surface = surface->ptrnext;
		}
		if (!return_code)
		{
			DESTROY(GT_voltex)(&voltex);
		}
	}
	LEAVE;

	return (voltex);
}

/***************************************************************************//**
 * Creates a GT_surface representation of the triangles in the supplied voltex.
 * @param voltex  A single GT_voltex
 * @return  The newly created GT_surface.
 */
struct GT_surface *GT_surface_create_from_GT_voltex(
	struct GT_voltex *voltex)
{
	ENTER(GT_surface_create_from_GT_voltex);
	GT_surface *surface = NULL;
	// Does not support linked list of voltex so make it an error
	if ((NULL != voltex) && (NULL == voltex->ptrnext))
	{
		int number_of_triangles = voltex->number_of_triangles;
		Triple *points = NULL;
		Triple *normalpoints = NULL;
		Triple *texturepoints = NULL;
		Triple *tangentpoints = NULL;
		GLfloat *datavalues = 0;
		ALLOCATE(points, Triple, number_of_triangles*3);
		ALLOCATE(normalpoints, Triple, number_of_triangles*3);
		if (voltex->n_texture_coordinates)
		{
			ALLOCATE(texturepoints, Triple, number_of_triangles*3);
		}
		int n_data_components = voltex->n_data_components;
		if (n_data_components)
		{
			ALLOCATE(datavalues, GLfloat, number_of_triangles*3*n_data_components);
		}
		if ((NULL != points) && (NULL != normalpoints))
		{
			int index = 0;
			for (int i = 0; i < number_of_triangles; i++)
			{
				const VT_iso_triangle *triangle = voltex->triangle_list[i];
				for (int j = 0; j < 3; j++)
				{
					const VT_iso_vertex *vertex = triangle->vertices[j];
					points[index][0] = vertex->coordinates[0];
					points[index][1] = vertex->coordinates[1];
					points[index][2] = vertex->coordinates[2];
					normalpoints[index][0] = vertex->normal[0];
					normalpoints[index][1] = vertex->normal[1];
					normalpoints[index][2] = vertex->normal[2];
					if (voltex->n_texture_coordinates)
					{
						texturepoints[index][0] = vertex->texture_coordinates[0];
						texturepoints[index][1] = vertex->texture_coordinates[1];
						texturepoints[index][2] = vertex->texture_coordinates[2];
					}
					if (voltex->n_data_components)
					{
						for (int k = 0; k < n_data_components; k++)
						{
							datavalues[index*n_data_components + k] = vertex->data[k];
						}
					}
					index++;
				}
			}
			Cmiss_graphic_render_polygon_mode render_polygon_mode = (voltex->voltex_type == g_VOLTEX_SHADED_TEXMAP) ?
					CMISS_GRAPHIC_RENDER_POLYGON_SHADED : CMISS_GRAPHIC_RENDER_POLYGON_WIREFRAME;
			surface = CREATE(GT_surface)(g_SH_DISCONTINUOUS_TEXMAP, render_polygon_mode, g_TRIANGLE,
				/*number_of_points_in_xi1*/number_of_triangles, /*number_of_points_in_xi2*/3, points,
				normalpoints, tangentpoints, texturepoints, n_data_components, datavalues);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_surface_create_from_GT_voltex.  Invalid argument(s)");
	}
	LEAVE;

	return (surface);
}

int GT_object_decimate_GT_surface(struct GT_object *graphics_object,
	double threshold_distance)
{
	int return_code;

	ENTER(GT_object_decimate_GT_surface);
	return_code = 0;
	if (graphics_object && (g_SURFACE == graphics_object->object_type))
	{
		ZnReal time = 0.0;
		int time_number = GT_object_get_time_number(graphics_object, time);
		if (0 < time_number)
		{
			if (graphics_object->primitive_lists)
			{
				// get range of coordinates to choose vertex tolerance
				struct Graphics_object_range_struct range;
				range.first = 1;
				range.minimum[0] = range.minimum[1] = range.minimum[2] = 0.0;
				range.maximum[0] = range.maximum[1] = range.maximum[2] = 0.0;
				get_graphics_object_range(graphics_object, (void *)&range);
				ZnReal size[3];
				size[0] = range.maximum[0] - range.minimum[0];
				size[1] = range.maximum[1] - range.minimum[1];
				size[2] = range.maximum[2] - range.minimum[2];
				ZnReal mag = sqrt(size[0]*size[0] + size[1]*size[1] + size[2]*size[2]);
				ZnReal vertex_radius_tolerance = mag * 1.0e-5;
				if (0.1*threshold_distance < vertex_radius_tolerance)
				{
					vertex_radius_tolerance = 0.1*threshold_distance;
				}

				GT_surface *old_surface =
					graphics_object->primitive_lists[time_number - 1].gt_surface.first;
				GT_voltex *voltex = GT_voltex_create_from_GT_surface(old_surface);
				// merge GT_voltex into itself to share coincident vertices
				if (!voltex->vertex_octree)
				{
					voltex->vertex_octree = CREATE(Octree)();
				}
				GT_voltex_merge_GT_voltex(voltex, voltex, vertex_radius_tolerance);
				GT_voltex_decimate_triangles(voltex, threshold_distance);
				GT_voltex_normalise_normals(voltex);
				GT_surface *new_surface = GT_surface_create_from_GT_voltex(voltex);
				if (NULL != new_surface)
				{
					// replace old surface(s) with new decimated surface
					GT_OBJECT_REMOVE_PRIMITIVES_AT_TIME_NUMBER(GT_surface)(
						graphics_object, time_number,
						(GT_object_primitive_object_name_conditional_function *)NULL,
						(void *)NULL);
					return_code =
						GT_OBJECT_ADD(GT_surface)(graphics_object, time, new_surface);
				}
				DESTROY(GT_voltex)(&voltex);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"GT_object_decimate_GT_surface.  Invalid primitive_lists");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"GT_object_decimate_GT_surface.  Graphics object does not contain a voltex.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_object_decimate_GT_surface.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* GT_object_decimate_GT_surface */

enum Cmiss_graphic_select_mode GT_object_get_select_mode(
	struct GT_object *graphics_object)
{
	if (graphics_object)
		return graphics_object->select_mode;
	return CMISS_GRAPHIC_SELECT_MODE_INVALID;
}

int GT_object_set_select_mode(struct GT_object *graphics_object,
	enum Cmiss_graphic_select_mode select_mode)
{
	if (graphics_object && (0 != ENUMERATOR_STRING(Cmiss_graphic_select_mode)(select_mode)))
	{
		if (select_mode != graphics_object->select_mode)
		{
			graphics_object->select_mode=select_mode;
			GT_object_changed(graphics_object);
		}
		return CMISS_OK;
	}
	return CMISS_ERROR_ARGUMENT;
}

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

double get_GT_object_render_line_width(struct GT_object *graphics_object)
{
	if (graphics_object)
		return graphics_object->render_line_width;
	return 0.0;
}

int set_GT_object_render_line_width(struct GT_object *graphics_object,
	double width)
{
	if (graphics_object && (width >= 0.0))
	{
		if (graphics_object->render_line_width != width)
		{
			graphics_object->render_line_width = width;
			GT_object_changed(graphics_object);
		}
		return CMISS_OK;
	}
	return CMISS_ERROR_ARGUMENT;
}

double get_GT_object_render_point_size(struct GT_object *graphics_object)
{
	if (graphics_object)
		return graphics_object->render_point_size;
	return 0.0;
}

int set_GT_object_render_point_size(struct GT_object *graphics_object,
	double size)
{
	if (graphics_object && (size >= 0.0))
	{
		if (graphics_object->render_point_size != size)
		{
			graphics_object->render_point_size = size;
			GT_object_changed(graphics_object);
		}
		return CMISS_OK;
	}
	return CMISS_ERROR_ARGUMENT;
}

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

int GT_object_set_name(struct GT_object *gt_object, const char *name)
{
	if (gt_object)
	{
		if (gt_object->name)
			DEALLOCATE(gt_object->name);
		gt_object->name = name ? duplicate_string(name) : 0;
		return 1;
	}
	return 0;
}

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
	struct Spectrum *spectrum)
{
	int return_code;
	if (graphics_object)
	{
		if (spectrum != graphics_object->spectrum)
		{
			REACCESS(Spectrum)(&graphics_object->spectrum, spectrum);
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
	return (return_code);
}

struct Spectrum *get_GT_object_spectrum(struct GT_object *graphics_object)
{
	struct Spectrum *spectrum;
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
	return (spectrum);
}

struct Cmiss_font *get_GT_object_font(struct GT_object *graphics_object)
{
	if (graphics_object)
	{
		// assume only one time
		if ((g_GLYPH_SET == graphics_object->object_type) && graphics_object->primitive_lists)
		{
			GT_glyph_set *glyph_set = graphics_object->primitive_lists[0].gt_glyph_set.first;
			if (glyph_set)
				return glyph_set->font;
		}
		else if ((g_POINTSET == graphics_object->object_type) && graphics_object->primitive_lists)
		{
			GT_pointset *point_set = graphics_object->primitive_lists[0].gt_pointset.first;
			if (point_set)
				return point_set->font;
		}
	}
	return 0;
}

int set_GT_object_font(struct GT_object *graphics_object,
	struct Cmiss_font *font)
{
	int return_code = 0;
	if (graphics_object)
	{
		if ((g_GLYPH_SET == graphics_object->object_type) && graphics_object->primitive_lists)
		{
			// assume only one time
			GT_glyph_set *glyph_set = graphics_object->primitive_lists[0].gt_glyph_set.first;
			if (glyph_set && (glyph_set->font != font))
			{
				while (glyph_set)
				{
					REACCESS(Cmiss_font)(&glyph_set->font, font);
					glyph_set = glyph_set->ptrnext;
				}
				GT_object_changed(graphics_object);
			}
		}
		return_code = 1;
	}
	return (return_code);
}

struct GT_object *get_GT_object_glyph(struct GT_object *graphics_object)
{
	if (graphics_object && (g_GLYPH_SET == graphics_object->object_type) &&
		graphics_object->primitive_lists)
	{
		GT_glyph_set *glyph_set = graphics_object->primitive_lists[0].gt_glyph_set.first;
		if (glyph_set)
		{
			return glyph_set->glyph;
		}
	}
	return 0;
}

int set_GT_object_glyph(struct GT_object *graphics_object,
	struct GT_object *glyph)
{
	int return_code = 0;
	if (graphics_object)
	{
		if ((g_GLYPH_SET == graphics_object->object_type) && graphics_object->primitive_lists)
		{
			// assume only one time
			GT_glyph_set *glyph_set = graphics_object->primitive_lists[0].gt_glyph_set.first;
			if (glyph_set && (glyph_set->glyph != glyph))
			{
				while (glyph_set)
				{
					REACCESS(GT_object)(&glyph_set->glyph, glyph);
					glyph_set = glyph_set->ptrnext;
				}
				GT_object_changed(graphics_object);
			}
		}
		return_code = 1;
	}
	return (return_code);
}

int set_GT_object_glyph_repeat_mode(struct GT_object *graphics_object,
	enum Cmiss_glyph_repeat_mode glyph_repeat_mode)
{
	int return_code = 0;
	if (graphics_object)
	{
		if ((g_GLYPH_SET == graphics_object->object_type) && graphics_object->primitive_lists)
		{
			// assume only one time
			GT_glyph_set *glyph_set = graphics_object->primitive_lists[0].gt_glyph_set.first;
			if (glyph_set && (glyph_set->glyph_repeat_mode != glyph_repeat_mode))
			{
				while (glyph_set)
				{
					glyph_set->glyph_repeat_mode = glyph_repeat_mode;
					glyph_set = glyph_set->ptrnext;
				}
				GT_object_changed(graphics_object);
			}
		}
		return_code = 1;
	}
	return (return_code);
}

int set_GT_object_glyph_base_size(struct GT_object *graphics_object,
	Triple base_size)
{
	int return_code = 0;
	if (graphics_object)
	{
		if ((g_GLYPH_SET == graphics_object->object_type) && graphics_object->primitive_lists)
		{
			// assume only one time
			GT_glyph_set *glyph_set = graphics_object->primitive_lists[0].gt_glyph_set.first;
			if (glyph_set && (
				(glyph_set->base_size[0] != base_size[0]) ||
				(glyph_set->base_size[1] != base_size[1]) ||
				(glyph_set->base_size[2] != base_size[2])))
			{
				while (glyph_set)
				{
					glyph_set->base_size[0] = base_size[0];
					glyph_set->base_size[1] = base_size[1];
					glyph_set->base_size[2] = base_size[2];
					glyph_set = glyph_set->ptrnext;
				}
				GT_object_changed(graphics_object);
			}
		}
		return_code = 1;
	}
	return (return_code);
}

int set_GT_object_glyph_scale_factors(struct GT_object *graphics_object,
	Triple scale_factors)
{
	int return_code = 0;
	if (graphics_object)
	{
		if ((g_GLYPH_SET == graphics_object->object_type) && graphics_object->primitive_lists)
		{
			// assume only one time
			GT_glyph_set *glyph_set = graphics_object->primitive_lists[0].gt_glyph_set.first;
			if (glyph_set && (
				(glyph_set->scale_factors[0] != scale_factors[0]) ||
				(glyph_set->scale_factors[1] != scale_factors[1]) ||
				(glyph_set->scale_factors[2] != scale_factors[2])))
			{
				while (glyph_set)
				{
					glyph_set->scale_factors[0] = scale_factors[0];
					glyph_set->scale_factors[1] = scale_factors[1];
					glyph_set->scale_factors[2] = scale_factors[2];
					glyph_set = glyph_set->ptrnext;
				}
				GT_object_changed(graphics_object);
			}
		}
		return_code = 1;
	}
	return (return_code);
}

int set_GT_object_glyph_offset(struct GT_object *graphics_object,
	Triple offset)
{
	int return_code = 0;
	if (graphics_object)
	{
		if ((g_GLYPH_SET == graphics_object->object_type) && graphics_object->primitive_lists)
		{
			// assume only one time
			GT_glyph_set *glyph_set = graphics_object->primitive_lists[0].gt_glyph_set.first;
			if (glyph_set && (
				(glyph_set->offset[0] != offset[0]) ||
				(glyph_set->offset[1] != offset[1]) ||
				(glyph_set->offset[2] != offset[2])))
			{
				while (glyph_set)
				{
					glyph_set->offset[0] = offset[0];
					glyph_set->offset[1] = offset[1];
					glyph_set->offset[2] = offset[2];
					glyph_set = glyph_set->ptrnext;
				}
				GT_object_changed(graphics_object);
			}
		}
		return_code = 1;
	}
	return (return_code);
}

int set_GT_object_glyph_label_offset(struct GT_object *graphics_object,
	Triple label_offset)
{
	int return_code = 0;
	if (graphics_object)
	{
		if ((g_GLYPH_SET == graphics_object->object_type) && graphics_object->primitive_lists)
		{
			// assume only one time
			GT_glyph_set *glyph_set = graphics_object->primitive_lists[0].gt_glyph_set.first;
			if (glyph_set && (
				(glyph_set->label_offset[0] != label_offset[0]) ||
				(glyph_set->label_offset[1] != label_offset[1]) ||
				(glyph_set->label_offset[2] != label_offset[2])))
			{
				while (glyph_set)
				{
					glyph_set->label_offset[0] = label_offset[0];
					glyph_set->label_offset[1] = label_offset[1];
					glyph_set->label_offset[2] = label_offset[2];
					glyph_set = glyph_set->ptrnext;
				}
				GT_object_changed(graphics_object);
			}
		}
		return_code = 1;
	}
	return (return_code);
}

int set_GT_object_glyph_label_text(struct GT_object *graphics_object,
	char *static_label_text[3])
{
	int return_code = 0;
	if (graphics_object && static_label_text)
	{
		if ((g_GLYPH_SET == graphics_object->object_type) && graphics_object->primitive_lists)
		{
			// assume only one time
			GT_glyph_set *glyph_set = graphics_object->primitive_lists[0].gt_glyph_set.first;
			if (glyph_set)
			{
				bool different = false;
				for (int i = 0; i < 3; ++i)
				{
					if ((static_label_text[i] != glyph_set->static_label_text[i]) && (
						(0 == static_label_text[i]) || (0 == glyph_set->static_label_text[i]) ||
						(0 != strcmp(static_label_text[i], glyph_set->static_label_text[i]))))
					{
						different = true;
						break;
					}
				}
				if (different)
				{
					while (glyph_set)
					{
						for (int i = 0; i < 3; ++i)
						{
							if (glyph_set->static_label_text[i])
							{
								DEALLOCATE(glyph_set->static_label_text[i]);
							}
							glyph_set->static_label_text[i] =
								static_label_text[i] ? duplicate_string(static_label_text[i]) : 0;\
						}
						glyph_set = glyph_set->ptrnext;
					}
					GT_object_changed(graphics_object);
				}
			}
		}
		return_code = 1;
	}
	return (return_code);
}

int set_GT_object_render_polygon_mode(struct GT_object *graphics_object,
	enum Cmiss_graphic_render_polygon_mode render_polygon_mode)
{
	int return_code = 0;
	if (graphics_object)
	{
		if ((g_SURFACE == graphics_object->object_type) && graphics_object->primitive_lists)
		{
			// assume only one time
			GT_surface *surface = graphics_object->primitive_lists[0].gt_surface.first;
			if (surface && (surface->render_polygon_mode != render_polygon_mode))
			{
				while (surface)
				{
					surface->render_polygon_mode = render_polygon_mode;
					surface = surface->ptrnext;
				}
				GT_object_changed(graphics_object);
			}
		}
		return_code = 1;
	}
	return (return_code);
}

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

struct GT_object_compile_context *CREATE(GT_object_compile_context)(
	ZnReal time, struct Graphics_buffer *graphics_buffer
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
		context->texture_tiling = (struct Texture_tiling *)NULL;
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

/*****************************************************************************//**
 * Creates a new Graphics_vertex_buffer.  Initially no memory is allocated
 * and no vertices stored.
 *
 * @param type  Determines the format of this vertex buffers.
 * @return Newly created buffer.
*/
struct Graphics_vertex_buffer *
  CREATE(Graphics_vertex_buffer)
  (Graphics_vertex_array_attribute_type type,
  unsigned int values_per_vertex)
{
	struct Graphics_vertex_buffer *buffer;

	if (ALLOCATE(buffer, struct Graphics_vertex_buffer, 1))
	{
		buffer->type = type;
		buffer->values_per_vertex = values_per_vertex;
		buffer->max_vertex_count = 0;
		buffer->vertex_count = 0;
		buffer->memory = NULL;
		buffer->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Graphics_vertex_buffer)  "
			"Unable to allocate buffer memory.");
		buffer = (struct Graphics_vertex_buffer *)NULL;
	}
	return (buffer);
}

/*****************************************************************************//**
 * Destroys a Graphics_vertex_buffer.
 *
 * @param buffer_address  Pointer to a buffer to be destroyed.
 * @return return_code. 1 for Success, 0 for failure.
*/
int DESTROY(Graphics_vertex_buffer)(
	struct Graphics_vertex_buffer **buffer_address)
{
	int return_code = 0;
	struct Graphics_vertex_buffer *buffer;
	if (buffer_address && (buffer = *buffer_address))
	{
		if (buffer->max_vertex_count && buffer->memory)
		{
			DEALLOCATE(buffer->memory);
		}
		DEALLOCATE(*buffer_address);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Graphics_vertex_buffer)  "
			"Invalid object.");
	}
	return (return_code);
}

Graphics_vertex_array::Graphics_vertex_array(Graphics_vertex_array_type type)
{
	internal = new Graphics_vertex_array_internal(type);
}

Graphics_vertex_buffer *Graphics_vertex_array_internal::get_or_create_vertex_buffer(
	Graphics_vertex_array_attribute_type vertex_type,
	unsigned int values_per_vertex)
{
	Graphics_vertex_array_attribute_type vertex_buffer_type = GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_POSITION;
	Graphics_vertex_buffer *buffer;

	switch (type)
	{
		case GRAPHICS_VERTEX_ARRAY_TYPE_FLOAT_SEPARATE_DRAW_ARRAYS:
		{
			vertex_buffer_type = vertex_type;
		} break;
	}
	buffer = FIND_BY_IDENTIFIER_IN_LIST(Graphics_vertex_buffer,type)
		 (vertex_buffer_type, buffer_list);
	if (buffer)
	{
		if (buffer->values_per_vertex != values_per_vertex)
		{
			buffer = (Graphics_vertex_buffer *)NULL;
		}
	}
	else
	{
		buffer = CREATE(Graphics_vertex_buffer)(vertex_buffer_type,
			values_per_vertex);
		if (buffer)
		{
			if (!ADD_OBJECT_TO_LIST(Graphics_vertex_buffer)(buffer,
				buffer_list))
			{
				DESTROY(Graphics_vertex_buffer)(&buffer);
			}
		}
	}
	return (buffer);
}

Graphics_vertex_buffer *Graphics_vertex_array_internal::get_vertex_buffer_for_attribute(
	Graphics_vertex_array_attribute_type vertex_type)
{
	Graphics_vertex_array_attribute_type vertex_buffer_type = GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_POSITION;
	Graphics_vertex_buffer *buffer;

   switch (type)
   {
		case GRAPHICS_VERTEX_ARRAY_TYPE_FLOAT_SEPARATE_DRAW_ARRAYS:
		{
			vertex_buffer_type = vertex_type;
		} break;
	}
	buffer = FIND_BY_IDENTIFIER_IN_LIST(Graphics_vertex_buffer,type)
		(vertex_buffer_type, buffer_list);

	return (buffer);
}

Graphics_vertex_buffer *Graphics_vertex_array_internal::get_vertex_buffer(
	Graphics_vertex_array_attribute_type vertex_type)
{
	Graphics_vertex_buffer *buffer;

	buffer = FIND_BY_IDENTIFIER_IN_LIST(Graphics_vertex_buffer,type)
		(vertex_type, buffer_list);

	return (buffer);
}

template <class value_type> int Graphics_vertex_array_internal::add_attribute(
	Graphics_vertex_array_attribute_type vertex_type,
	const unsigned int values_per_vertex, const unsigned int number_of_values, const value_type *values)
{
	int return_code = 1;
	Graphics_vertex_buffer *buffer;

	buffer = get_or_create_vertex_buffer(vertex_type, values_per_vertex);
	if (buffer)
	{
		Graphics_vertex_array_attribute_type vertex_buffer_type = buffer->type;
		if (!buffer->memory)
		{
		// Allocate enough memory for what I am about to add plus some headroom
			if (ALLOCATE(buffer->memory, value_type,
				( GRAPHICS_VERTEX_BUFFER_INITIAL_SIZE + number_of_values ) * values_per_vertex))
			{
				buffer->max_vertex_count = GRAPHICS_VERTEX_BUFFER_INITIAL_SIZE;
			}
			else
			{
				return_code = 0;
			}
		}
		if (return_code)
		{
			if (buffer->max_vertex_count <= ( buffer->vertex_count + number_of_values ) )
			{
				// Reallocate enough memory for what I am about to add plus some headroom
				if (REALLOCATE(buffer->memory, buffer->memory, value_type,
					( 2 * buffer->max_vertex_count + number_of_values ) * values_per_vertex))
				{
					buffer->max_vertex_count = 2 * buffer->max_vertex_count + number_of_values;
				}
				else
				{
					return_code = 0;
				}
			}
		}
		if (return_code)
		{
			if (vertex_buffer_type == vertex_type)
			{
				memcpy((value_type*)buffer->memory + buffer->vertex_count * values_per_vertex,
					values, values_per_vertex * number_of_values * sizeof(value_type));
				buffer->vertex_count += number_of_values;
			}
			else
			{
				display_message(ERROR_MESSAGE,"Graphics_vertex_array::add_attribute.  "
					"Storage for this combination of vertex_buffer and vertex not implemented yet.");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_vertex_array::add_attribute.  "
			"Unable to create buffer.");
		return_code = 0;
	}

	return (return_code);
}

template <class value_type> int Graphics_vertex_array_internal::get_attribute(
	Graphics_vertex_array_attribute_type vertex_type,
	unsigned int vertex_index,
	unsigned int number_of_values, value_type *values)
{
	Graphics_vertex_buffer *buffer;
	int return_code = 0;

	buffer = get_vertex_buffer_for_attribute(vertex_type);
	if (buffer)
	{
		Graphics_vertex_array_attribute_type vertex_buffer_type = buffer->type;
		if (buffer->values_per_vertex == number_of_values)
		{
			if (vertex_buffer_type == vertex_type)
			{
				memcpy(values, (value_type*) buffer->memory + vertex_index
					* buffer->values_per_vertex, buffer->values_per_vertex
					* sizeof(value_type));
				return_code = 1;
			}
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		return_code = 0;
	}
	return (return_code);
}

template <class value_type> int Graphics_vertex_array_internal::get_vertex_buffer(
		Graphics_vertex_array_attribute_type vertex_buffer_type,
		value_type **vertex_buffer, unsigned int *values_per_vertex,
		unsigned int *vertex_count)
{
	Graphics_vertex_buffer *buffer;
	int return_code;

	buffer = get_vertex_buffer(vertex_buffer_type);
	if (buffer)
	{
		*vertex_buffer = static_cast<value_type*>(buffer->memory);
		*values_per_vertex = buffer->values_per_vertex;
		*vertex_count = buffer->vertex_count;
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}

	return return_code;
}

template <class value_type> int Graphics_vertex_array_internal::free_unused_buffer_memory(
	Graphics_vertex_array_attribute_type vertex_type, const value_type *dummy )
{
	int return_code = 0;
	Graphics_vertex_buffer *buffer = get_vertex_buffer(vertex_type);
	if (buffer)
	{
		if (REALLOCATE(buffer->memory, buffer->memory, value_type,
				(buffer->vertex_count  * buffer->values_per_vertex)))
		{
			return_code = 1;
			buffer->max_vertex_count = buffer->vertex_count;
		}
	}

	return return_code;
}

int Graphics_vertex_array::free_unused_buffer_memory(
	Graphics_vertex_array_attribute_type vertex_type )
{
	USE_PARAMETER(vertex_type);
	return 0;//internal->free_unused_buffer_memory( vertex_type );
}
/*
int Graphics_vertex_array::add_float_attribute(
	Graphics_vertex_array_attribute_type vertex_type,
	unsigned int values_per_vertex, unsigned int number_of_values, ZnReal *values)
{
	return internal->add_attribute(vertex_type, values_per_vertex, number_of_values, values);
}
*/
int Graphics_vertex_array::add_float_attribute(
	Graphics_vertex_array_attribute_type vertex_type,
	const unsigned int values_per_vertex, const unsigned int number_of_values, const GLfloat *values)
{
	return internal->add_attribute(vertex_type, values_per_vertex, number_of_values, values);
}


int Graphics_vertex_array::get_float_vertex_buffer(
		Graphics_vertex_array_attribute_type vertex_buffer_type,
		GLfloat **vertex_buffer, unsigned int *values_per_vertex,
		unsigned int *vertex_count)
{
	return internal->get_vertex_buffer(vertex_buffer_type,
		vertex_buffer, values_per_vertex, vertex_count);
}

int Graphics_vertex_array::add_unsigned_integer_attribute(
		Graphics_vertex_array_attribute_type vertex_type,
		unsigned int values_per_vertex, unsigned int number_of_values, unsigned int *values)
{
	return internal->add_attribute(vertex_type, values_per_vertex, number_of_values, values);
}

int Graphics_vertex_array::get_unsigned_integer_attribute(
		Graphics_vertex_array_attribute_type vertex_type,
		unsigned int vertex_index,
		unsigned int number_of_values, unsigned int *values)
{
	return internal->get_attribute(vertex_type,
		vertex_index, number_of_values, values);
}

int Graphics_vertex_array::get_unsigned_integer_vertex_buffer(
		Graphics_vertex_array_attribute_type vertex_buffer_type,
		unsigned int **vertex_buffer, unsigned int *values_per_vertex,
		unsigned int *vertex_count)
{
	return internal->get_vertex_buffer(vertex_buffer_type,
		vertex_buffer, values_per_vertex, vertex_count);
}

int Graphics_vertex_array::add_integer_attribute(
		Graphics_vertex_array_attribute_type vertex_type,
		unsigned int values_per_vertex, unsigned int number_of_values, int *values)
{
	return internal->add_attribute(vertex_type, values_per_vertex, number_of_values, values);
}

int Graphics_vertex_array::get_integer_attribute(
		Graphics_vertex_array_attribute_type vertex_type,
		unsigned int vertex_index,
		unsigned int number_of_values, int *values)
{
	return internal->get_attribute(vertex_type,
		vertex_index, number_of_values, values);
}

unsigned int Graphics_vertex_array::get_number_of_vertices(
	Graphics_vertex_array_attribute_type vertex_buffer_type)
{
	Graphics_vertex_buffer *buffer;
	unsigned int vertex_count;
	buffer = internal->get_vertex_buffer(vertex_buffer_type);
	if (buffer)
	{
		vertex_count = buffer->vertex_count;
	}
	else
	{
		vertex_count = 0;
	}

	return vertex_count;
}

/*****************************************************************************//**
 * Resets the number of vertices defined in the buffer to zero.  Does not actually
 * reset the allocated memory to zero as it is anticipated that the buffer will
 * recreated.
 *
 * @param buffer  Buffer to be cleared.
 * @return return_code. 1 for Success, 0 for failure.
*/
int Graphics_vertex_buffer_clear(
	struct Graphics_vertex_buffer *buffer, void *user_data_dummy)
{
	int return_code;
	USE_PARAMETER(user_data_dummy);

	if (buffer)
	{
		buffer->vertex_count = 0;
	}
	return_code = 1;

	return (return_code);
}

int Graphics_vertex_array::clear_buffers()
{
	return FOR_EACH_OBJECT_IN_LIST(Graphics_vertex_buffer)(
		Graphics_vertex_buffer_clear, NULL, internal->buffer_list);
}

Graphics_vertex_array::~Graphics_vertex_array()
{
	delete internal;
}

enum Cmiss_glyph_type GT_object_get_glyph_type(
	struct GT_object *gt_object)
{
	if (gt_object)
		return gt_object->glyph_type;
	return CMISS_GLYPH_TYPE_INVALID;
}

int GT_object_set_glyph_type(struct GT_object *gt_object,
	enum Cmiss_glyph_type glyph_type)
{
	if (gt_object)
	{
		gt_object->glyph_type = glyph_type;
		return 1;
	}
	return 0;
}

#if 0
int GT_object_get_overlay(struct GT_object *graphics_object)
{
	return graphics_object->overlay;
}

int set_GT_object_overlay(struct GT_object *graphics_object, int overlay)
{
	int return_code;

	ENTER(set_GT_object_overlay);
	if (graphics_object)
	{
		if (overlay != graphics_object->overlay)
		{
			graphics_object->overlay = overlay;
			GT_object_changed(graphics_object);
		}
	  return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_GT_object_overlay.  Invalid graphics object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}
#endif
