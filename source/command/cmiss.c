/*******************************************************************************
FILE : cmiss.c

LAST MODIFIED : 31 May 2000

DESCRIPTION :
Functions for executing cmiss commands.
==============================================================================*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#if defined (MOTIF)
#include <Xm/List.h>
#endif /* defined (MOTIF) */
#if !defined (WINDOWS_DEV_FLAG)
#include "application/application.h"
#if defined (CELL)
#include "cell/cell_window.h"
#include "cell/cmgui_connection.h"
#endif /* defined (CELL) */
#include "comfile/comfile_window.h"
#endif /* !defined (WINDOWS_DEV_FLAG) */
#include "command/cmiss.h"
#if !defined (WINDOWS_DEV_FLAG)
#include "command/command_window.h"
#include "command/example_path.h"
#endif /* !defined (WINDOWS_DEV_FLAG) */
#include "command/parser.h"
#if !defined (WINDOWS_DEV_FLAG)
#include "data/data_grabber_dialog.h"
#include "data/node_transform.h"
#include "data/sync_2d_3d.h"
#include "element/element_creator.h"
#include "element/element_point_viewer.h"
#include "finite_element/computed_field.h"
#include "finite_element/export_finite_element.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_to_graphics_object.h"
#include "finite_element/finite_element_to_iso_lines.h"
#include "finite_element/finite_element_to_streamlines.h"
#include "finite_element/grid_field_calculator.h"
#include "finite_element/import_finite_element.h"
#endif /* !defined (WINDOWS_DEV_FLAG) */
#include "general/debug.h"
#if !defined (WINDOWS_DEV_FLAG)
#include "general/image_utilities.h"
#include "general/matrix_vector.h"
#include "general/multi_range.h"
#include "general/mystring.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/environment_map.h"
#include "graphics/graphical_element.h"
#include "graphics/graphical_element_editor_dialog.h"
#include "graphics/graphics_object.h"
#include "graphics/graphics_window.h"
#include "graphics/import_graphics_object.h"
#include "graphics/interactive_streamlines_dialog.h"
#include "graphics/iso_field_calculation.h"
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "graphics/material.h"
#include "graphics/movie_graphics.h"
#if defined (NEW_ALIAS)
#include "graphics/renderalias.h"
#endif /* defined (NEW_ALIAS) */
#include "graphics/renderbinarywavefront.h"
#include "graphics/rendervrml.h"
#include "graphics/renderwavefront.h"
#include "graphics/scene.h"
#include "graphics/spectrum.h"
#include "graphics/spectrum_editor.h"
#include "graphics/spectrum_editor_dialog.h"
#include "graphics/spectrum_settings.h"
#include "graphics/texture.h"
#include "graphics/texturemap.h"
#include "graphics/userdef_objects.h"
#include "graphics/volume_texture.h"
#include "graphics/volume_texture_editor.h"
#include "graphics/write_to_video.h"
#include "help/help_interface.h"
#if defined (HAPTIC)
#include "io_devices/haptic_input_module.h"
#endif /* defined (HAPTIC) */
#include "io_devices/input_module_dialog.h"
#if defined (LINK_CMISS)
#include "link/cmiss.h"
#endif /* defined (LINK_CMISS) */
#include "material/material_editor_dialog.h"
#include "menu/menu_window.h"
#if defined (MIRAGE)
/*#include "mirage/movie.h"*/
#include "mirage/tracking_editor_dialog.h"
#endif /* defined (MIRAGE) */
#include "node/interactive_node_editor_dialog.h"
#include "node/node_tool.h"
#include "node/node_viewer.h"
#include "projection/projection_window.h"
#include "slider/emoter_dialog.h"
#include "slider/node_group_slider_dialog.h"
#if defined (OLD_CODE)
#include "socket/socket.h"
#endif /* defined (OLD_CODE) */
#include "three_d_drawing/movie_extensions.h"
#include "three_d_drawing/ThreeDDraw.h"
#include "time/time_editor_dialog.h"
#include "time/time_keeper.h"
#include "transformation/transformation_editor_dialog.h"
#if defined (UNEMAP)
#include "unemap/system_window.h"
#endif /* defined (UNEMAP) */
#include "user_interface/filedir.h"
#endif /* !defined (WINDOWS_DEV_FLAG) */
#include "user_interface/confirmation.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "curve/control_curve.h"
#include "curve/control_curve_editor_dialog.h"
#if !defined (WINDOWS_DEV_FLAG)
#include "view/coord_trans.h"
#include "xvg/include/xvg_interface.h"
#endif /* !defined (WINDOWS_DEV_FLAG) */
#if defined (F90_INTERPRETER)
#include "command/f90_interpreter.h"
#else /* defined (F90_INTERPRETER) */
#if defined (PERL_INTERPRETER)
#include "command/perl_interpreter.h"
#endif /* defined (PERL_INTERPRETER) */
#endif /* defined (F90_INTERPRETER) */

/*
Module variables
----------------
*/
#if defined (LINK_CMISS)
/*???GMH.  This is a hack - when we register it will disappear (used in
	user_interface.c) */
struct CMISS_connection *CMISS = (struct CMISS_connection *)NULL;
#endif /* LINK_CMISS */

/*
Module functions
----------------
*/

struct Add_FE_element_to_list_if_in_range_data
/*******************************************************************************
LAST MODIFIED : 21 April 1999

DESCRIPTION :
Data for iterator function add_FE_element_to_list_if_in_range.
==============================================================================*/
{
	struct Multi_range *element_ranges;
	struct LIST(FE_element) *element_list;
}; /* struct Add_FE_element_to_list_if_in_range_data */

static int add_FE_element_to_list_if_in_range(struct FE_element *element,
	void *element_in_range_data_void)
/*******************************************************************************
LAST MODIFIED : 21 April 1999

DESCRIPTION :
Adds the <element> to the <element_list> if it is a top-level element and
either <multi_range> is NULL or contains the element cm.number.
Does not report any errors if the element is already in the list.
==============================================================================*/
{
	int return_code;
	struct Add_FE_element_to_list_if_in_range_data *element_in_range_data;
	
	ENTER(add_FE_element_to_list_if_in_range);
	if (element&&(element_in_range_data=
		(struct Add_FE_element_to_list_if_in_range_data *)
		element_in_range_data_void)&&element_in_range_data->element_list)
	{
		if ((CM_ELEMENT==element->cm.type)&&
			((!element_in_range_data->element_ranges)||Multi_range_is_value_in_range(
				element_in_range_data->element_ranges,element->cm.number))&&
			(!FIND_BY_IDENTIFIER_IN_LIST(FE_element,identifier)(
				element->identifier,element_in_range_data->element_list)))
		{
			return_code=ADD_OBJECT_TO_LIST(FE_element)(element,
				element_in_range_data->element_list);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"add_FE_element_to_list_if_in_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* add_FE_element_to_list_if_in_range */

struct Add_FE_node_to_list_if_in_range_data
/*******************************************************************************
LAST MODIFIED : 21 April 1999

DESCRIPTION :
Data for iterator function add_FE_node_to_list_if_in_range.
==============================================================================*/
{
	struct Multi_range *node_ranges;
	struct LIST(FE_node) *node_list;
}; /* struct Add_FE_node_to_list_if_in_range_data */

static int add_FE_node_to_list_if_in_range(struct FE_node *node,
	void *node_in_range_data_void)
/*******************************************************************************
LAST MODIFIED : 21 April 1999

DESCRIPTION :
Adds the <node> to the <node_list> if either the <multi_range> is NULL or
contains the node cm_node_number.
Does not report any errors if the node is already in the list.
==============================================================================*/
{
	int cm_node_identifier,return_code;
	struct Add_FE_node_to_list_if_in_range_data *node_in_range_data;

	ENTER(add_FE_node_to_list_if_in_range);
	if (node&&(node_in_range_data=
		(struct Add_FE_node_to_list_if_in_range_data *)node_in_range_data_void)&&
		node_in_range_data->node_list)
	{
		return_code=1;
		cm_node_identifier=get_FE_node_cm_node_identifier(node);
		if (((!node_in_range_data->node_ranges)||Multi_range_is_value_in_range(
			node_in_range_data->node_ranges,cm_node_identifier))&&
			(!FIND_BY_IDENTIFIER_IN_LIST(FE_node,cm_node_identifier)(
				cm_node_identifier,node_in_range_data->node_list)))
		{
			return_code=ADD_OBJECT_TO_LIST(FE_node)(node,
				node_in_range_data->node_list);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"add_FE_node_to_list_if_in_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* add_FE_node_to_list_if_in_range */

struct Change_identifier_data
/*******************************************************************************
LAST MODIFIED : 7 October 1999

DESCRIPTION :
==============================================================================*/
{
	int count, element_offset, face_offset, line_offset, node_offset;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(FE_node) *node_manager;
};

#if !defined (WINDOWS_DEV_FLAG)
static int FE_element_change_identifier_sub(struct FE_element *element,
	void *data_void)
/*******************************************************************************
LAST MODIFIED : 7 October 1999

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct CM_element_information cm_id;
	struct Change_identifier_data *data;

	ENTER(FE_element_change_identifier_sub);
	if (element && 
		(data=(struct Change_identifier_data *)data_void)
		&& data->element_manager)
	{
		return_code = 1;
		cm_id.type = element->cm.type;
		cm_id.number = element->cm.number;
		switch( cm_id.type )
		{
			case CM_ELEMENT:
			{
				cm_id.number += data->element_offset;
			} break;
			case CM_FACE:
			{
				cm_id.number += data->element_offset;
			} break;
			case CM_LINE:
			{
				cm_id.number += data->element_offset;
			} break;
		}
		if (MANAGER_MODIFY_IDENTIFIER(FE_element, identifier)(element,
			&cm_id, data->element_manager))
		{
			data->count++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_change_identifier_sub.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_change_identifier_sub */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int FE_node_change_identifier_sub(struct FE_node *node, void *data_void)
/*******************************************************************************
LAST MODIFIED : 7 October 1999

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Change_identifier_data *data;

	ENTER(FE_node_change_identifier_sub);
	if (node && 
		(data=(struct Change_identifier_data *)data_void)
		&& data->node_manager)
	{
		return_code = 1;
		if (MANAGER_MODIFY_IDENTIFIER(FE_node, cm_node_identifier)(node,
			get_FE_node_cm_node_identifier(node) + data->node_offset,
			data->node_manager))
		{
			data->count++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_change_identifier_sub.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_change_identifier_sub */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_change_identifier(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 October 1999

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"dgroup",NULL,NULL,set_FE_node_group},
		{"egroup",NULL,NULL,set_FE_element_group},
		{"element_offset",NULL,NULL,set_int},
		{"face_offset",NULL,NULL,set_int},
		{"line_offset",NULL,NULL,set_int},
		{"ngroup",NULL,NULL,set_FE_node_group},
		{"node_offset",NULL,NULL,set_int},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Change_identifier_data data;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *data_group, *node_group;

	ENTER(gfx_change_identifier);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data = (struct Cmiss_command_data *)command_data_void)
		{
			data_group = (struct GROUP(FE_node) *) NULL;
			element_group = (struct GROUP(FE_element) *) NULL;
			node_group = (struct GROUP(FE_node) *) NULL;
			data.node_manager = (struct MANAGER(FE_node) *)NULL;
			data.element_manager = (struct MANAGER(FE_element) *)NULL;
			data.element_offset = 0;
			data.face_offset = 0;
			data.line_offset = 0;
			data.node_offset = 0;
			(option_table[0]).to_be_modified= &data_group;
			(option_table[0]).user_data=command_data->data_group_manager;
			(option_table[1]).to_be_modified= &element_group;
			(option_table[1]).user_data=command_data->element_group_manager;
			(option_table[2]).to_be_modified= &(data.element_offset);
			(option_table[3]).to_be_modified= &(data.face_offset);
			(option_table[4]).to_be_modified= &(data.line_offset);
			(option_table[5]).to_be_modified= &node_group;
			(option_table[5]).user_data=command_data->node_group_manager;
			(option_table[6]).to_be_modified= &(data.node_offset);
			if (return_code=process_multiple_options(state,option_table))
			{
				if ((!node_group)&&(!data_group)&&(!element_group))
				{
					display_message(ERROR_MESSAGE,
						"gfx_change_identifier."
						"  Specify at least one of dgroup, egroup or ngroup");
					return_code = 1;
				}
				if (return_code)
				{
					if (node_group)
					{
						data.count = 0;
						data.node_manager = command_data->node_manager;
						MANAGER_BEGIN_CACHE(FE_node)(command_data->node_manager);
						FOR_EACH_OBJECT_IN_GROUP(FE_node)(
							FE_node_change_identifier_sub,
							(void *)&data, node_group);
						MANAGER_END_CACHE(FE_node)(command_data->node_manager);
						if (data.count != NUMBER_IN_GROUP(FE_node)(node_group))
						{
							display_message(ERROR_MESSAGE,
								"gfx_change_identifier."
								"  Only able to update node numbers for %d nodes out of %d\n",
								data.count, NUMBER_IN_GROUP(FE_node)(node_group));
						}
					}
					if (data_group)
					{
						data.count = 0;
						data.node_manager = command_data->data_manager;
						MANAGER_BEGIN_CACHE(FE_node)(command_data->data_manager);
						FOR_EACH_OBJECT_IN_GROUP(FE_node)(
							FE_node_change_identifier_sub,
							(void *)&data, data_group);
						MANAGER_END_CACHE(FE_node)(command_data->data_manager);
						if (data.count != NUMBER_IN_GROUP(FE_node)(data_group))
						{
							display_message(ERROR_MESSAGE,
								"gfx_change_identifier."
								"  Only able to update node numbers for %d nodes out of %d\n",
								data.count, NUMBER_IN_GROUP(FE_node)(data_group));
						}
					}
					if (element_group)
					{
						data.count = 0;
						data.element_manager = command_data->element_manager;
						MANAGER_BEGIN_CACHE(FE_element)(command_data->element_manager);
						FOR_EACH_OBJECT_IN_GROUP(FE_element)(
							FE_element_change_identifier_sub,
							(void *)&data, element_group);
						MANAGER_END_CACHE(FE_element)(command_data->element_manager);
						if (data.count != NUMBER_IN_GROUP(FE_element)(element_group))
						{
							display_message(ERROR_MESSAGE,
								"gfx_change_identifier."
								"  Only able to update element numbers for %d elements out of %d\n",
								data.count, NUMBER_IN_GROUP(FE_element)(element_group));
						}
					}
				}
			}
			/* must deaccess computed_field since accessed by set_FE_node_group */
			if (node_group)
			{
				DEACCESS(GROUP(FE_node))(&node_group);
			}				
			if (data_group)
			{
				DEACCESS(GROUP(FE_node))(&data_group);
			}				
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}				
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_change_identifier.  Missing command data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_change_identifier.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* gfx_change_identifier */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_create_annotation(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 January 2000

DESCRIPTION :
Executes a GFX CREATE ANNOTATION command. Creates a graphics object containing
a single point in 3-D space with a text string drawn beside it.
==============================================================================*/
{
	char *annotation_text,*graphics_object_name,**text;
	float time;
	int number_of_components,return_code;
	static char default_name[]="annotation";
	static char default_annotation_text[]="\"annotation text\"";
	struct Cmiss_command_data *command_data;
	struct Graphical_material *material;
	struct GT_object *graphics_object;
	struct GT_pointset *point_set;
	struct Option_table *option_table;
	Triple *pointlist,position;

	ENTER(gfx_create_annotation);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			if (ALLOCATE(graphics_object_name,char,strlen(default_name)+1))
			{
				strcpy(graphics_object_name,default_name);
			}
			else
			{
				graphics_object_name=(char *)NULL;
			}
			/* must access it now, because we deaccess it later */
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			position[0]=0.0;
			position[1]=0.0;
			position[2]=0.0;
			if (ALLOCATE(annotation_text,char,strlen(default_annotation_text)+1))
			{
				strcpy(annotation_text,default_annotation_text);
			}
			else
			{
				annotation_text=(char *)NULL;
			}
			time=0.0;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* material */
			Option_table_add_entry(option_table,"material",&material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* position */
			number_of_components=3;
			Option_table_add_entry(option_table,"position",position,
				&number_of_components,set_float_vector);
			/* text */
			Option_table_add_entry(option_table,"text",&annotation_text,
				(void *)1,set_name);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			return_code=Option_table_multi_parse(option_table,state);
			if (return_code)
			{
				if (annotation_text)
				{
					if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
						graphics_object_name,command_data->graphics_object_list))
					{
						if (g_POINTSET==graphics_object->object_type)
						{
							if (GT_object_has_time(graphics_object,time))
							{
								display_message(WARNING_MESSAGE,
									"Overwriting time %g in graphics object '%s'",time,
									graphics_object_name);
								return_code=GT_object_delete_time(graphics_object,time);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Object of different type named '%s' already exists",
								graphics_object_name);
							return_code=0;
						}
					}
					else
					{
						if (!((graphics_object=CREATE(GT_object)(graphics_object_name,
							g_POINTSET,material))&&
							ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
								command_data->graphics_object_list)))
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_points.  Could not create graphics object");
							DESTROY(GT_object)(&graphics_object);
							return_code=0;
						}
					}
					if (return_code)
					{
						/* create the pointset used to display the annotation */
						pointlist=(Triple *)NULL;
						text=(char **)NULL;
						point_set=(struct GT_pointset *)NULL;
						if (ALLOCATE(pointlist,Triple,1)&&ALLOCATE(text,char *,1))
						{
							*text = annotation_text;
							(*pointlist)[0]=position[0];
							(*pointlist)[1]=position[1];
							(*pointlist)[2]=position[2];
							if ((point_set=CREATE(GT_pointset)(1,pointlist,text,g_NO_MARKER,
								0.0,g_NO_DATA,(GTDATA *)NULL,(int *)NULL))&&
								GT_OBJECT_ADD(GT_pointset)(graphics_object,time,point_set))
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
							return_code=0;
						}
						if (return_code)
						{
							/* annotation string now owned by point set */
							annotation_text=(char *)NULL;
						}
						else
						{
							display_message(WARNING_MESSAGE,"Could not create annotation");
							if (point_set)
							{
								DESTROY(GT_pointset)(&point_set);
							}
							else
							{
								DEALLOCATE(pointlist);
								DEALLOCATE(text);
							}
							if (0==GT_object_get_number_of_times(graphics_object))
							{
								REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
									command_data->graphics_object_list);
							}
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing annotation text");
					return_code=0;
				}
			}
			DESTROY(Option_table)(&option_table);
			if (annotation_text)
			{
				DEALLOCATE(annotation_text);
			}
			DEACCESS(Graphical_material)(&material);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_annotation.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_annotation.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_annotation */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_colour_bar(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 January 2000

DESCRIPTION :
Executes a GFX CREATE COLOUR_BAR command. Creates a colour bar graphics object
with tick marks and labels for showing the scale of a spectrum.
==============================================================================*/
{
	char *graphics_object_name,number_format[16],number_string[48];
	float bar_length,bar_radius,extend_length,tick_length;
	int number_of_components,return_code,significant_figures,tick_divisions;
	static char default_name[]="colour_bar";
	struct Cmiss_command_data *command_data;
	struct Graphical_material *label_material,*material;
	struct GT_object *graphics_object;
	struct Option_table *option_table;
	struct Spectrum *spectrum;
	Triple bar_axis,bar_centre,side_axis;

	ENTER(gfx_create_colour_bar);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			if (ALLOCATE(graphics_object_name,char,strlen(default_name)+1))
			{
				strcpy(graphics_object_name,default_name);
			}
			else
			{
				graphics_object_name=(char *)NULL;
			}
			/* must access it now, because we deaccess it later */
			label_material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
			number_of_components=3;
			bar_centre[0]=-0.9;
			bar_centre[1]=0.0;
			bar_centre[2]=0.5;
			bar_axis[0]=0.0;
			bar_axis[1]=1.0;
			bar_axis[2]=0.0;
			side_axis[0]=1.0;
			side_axis[1]=0.0;
			side_axis[2]=0.0;
			bar_length=1.6;
			extend_length=0.06;
			bar_radius=0.06;
			significant_figures=4;
			tick_length=0.04;
			tick_divisions=10;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* axis */
			Option_table_add_entry(option_table,"axis",bar_axis,
				&number_of_components,set_float_vector);
			/* centre */
			Option_table_add_entry(option_table,"centre",bar_centre,
				&number_of_components,set_float_vector);
			/* divisions */
			Option_table_add_entry(option_table,"divisions",&tick_divisions,
				NULL,set_int_non_negative);
			/* extend_length */
			Option_table_add_entry(option_table,"extend_length",&extend_length,
				NULL,set_float_non_negative);
			/* label_material */
			Option_table_add_entry(option_table,"label_material",&label_material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* length */
			Option_table_add_entry(option_table,"length",&bar_length,
				NULL,set_float_positive);
			/* material */
			Option_table_add_entry(option_table,"material",&material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* radius */
			Option_table_add_entry(option_table,"radius",&bar_radius,
				NULL,set_float_positive);
			/* significant_figures */
			Option_table_add_entry(option_table,"significant_figures",
				&significant_figures,NULL,set_int_positive);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			/* tick_direction */
			Option_table_add_entry(option_table,"tick_direction",side_axis,
				&number_of_components,set_float_vector);
			/* tick_length */
			Option_table_add_entry(option_table,"tick_length",&tick_length,
				NULL,set_float_non_negative);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				if (20 < significant_figures)
				{
					display_message(WARNING_MESSAGE,"Limited to 20 significant figures");
					significant_figures=20;
				}
				if (100 < tick_divisions)
				{
					display_message(WARNING_MESSAGE,"Limited to 100 tick_divisions");
					tick_divisions=100;
				}
				sprintf(number_format," %%+.%de",significant_figures-1);
				if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list))
				{
					display_message(ERROR_MESSAGE,
						"Graphics object named '%s' already exists",graphics_object_name);
					return_code=0;
				}
				else
				{
					if (!((graphics_object=create_Spectrum_colour_bar(
						graphics_object_name,spectrum,bar_centre,bar_axis,side_axis,
						bar_length,bar_radius,extend_length,tick_divisions,tick_length,
						number_format,number_string,material,label_material))&&
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list)))
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_colour_bar.  Could not create graphics object");
						if (graphics_object)
						{
							DESTROY(GT_object)(&graphics_object);
						}
						return_code=0;
					}
				}
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			DEACCESS(Graphical_material)(&label_material);
			DEACCESS(Graphical_material)(&material);
			DEACCESS(Spectrum)(&spectrum);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_colour_bar.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_colour_bar.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_colour_bar */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_cylinders(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Executes a GFX CREATE CYLINDERS command.
==============================================================================*/
{
	char exterior_flag,*graphics_object_name;
	float constant_radius,scale_factor,time;
	gtObject *graphics_object;
	int face_number,return_code;
	static char default_name[]="cylinders";
	struct Cmiss_command_data *command_data;
	struct Element_discretization discretization;
	struct Element_to_cylinder_data element_to_cylinder_data;
	struct Computed_field *coordinate_field,*data_field,*radius_field;
	struct Graphical_material *material;
	struct GROUP(FE_element) *element_group;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data,set_radius_field_data;
	struct Spectrum *spectrum;

	ENTER(gfx_create_cylinders);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			if (ALLOCATE(graphics_object_name,char,strlen(default_name)+1))
			{
				strcpy(graphics_object_name,default_name);
			}
			else
			{
				graphics_object_name=(char *)NULL;
			}
			element_group=(struct GROUP(FE_element) *)NULL;
			constant_radius=0.0;
			scale_factor=1.0;
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			data_field=(struct Computed_field *)NULL;
			radius_field=(struct Computed_field *)NULL;
			time=0;
			/* must access it now, because we deaccess it later */
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
			discretization.number_in_xi1=4;
			discretization.number_in_xi2=6;
			discretization.number_in_xi3=0;
			exterior_flag=0;
			face_number=0;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* constant_radius */
			Option_table_add_entry(option_table,"constant_radius",&constant_radius,
				NULL,set_float);
			/* coordinate */
			set_coordinate_field_data.computed_field_package=
				command_data->computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_package=
				command_data->computed_field_package;
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&data_field,
				&set_data_field_data,set_Computed_field_conditional);
			/* exterior */
			Option_table_add_entry(option_table,"exterior",&exterior_flag,
				NULL,set_char_flag);
			/* face */
			Option_table_add_entry(option_table,"face",&face_number,
				NULL,set_exterior);
			/* from */
			Option_table_add_entry(option_table,"from",&element_group,
				command_data->element_group_manager,set_FE_element_group);
			/* material */
			Option_table_add_entry(option_table,"material",&material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* radius_scalar */
			set_radius_field_data.computed_field_package=
				command_data->computed_field_package;
			set_radius_field_data.conditional_function=Computed_field_is_scalar;
			set_radius_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"radius_scalar",&radius_field,
				&set_radius_field_data,set_Computed_field_conditional);
			/* scale_factor */
			Option_table_add_entry(option_table,"scale_factor",&scale_factor,
				NULL,set_float);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* with */
			Option_table_add_entry(option_table,"with",&discretization,
				command_data->user_interface,set_Element_discretization);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors, not asking for help */
			if (return_code)
			{
				face_number -= 2;
				if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list))
				{
					if (g_SURFACE==graphics_object->object_type)
					{
						if (GT_object_has_time(graphics_object,time))
						{
							display_message(WARNING_MESSAGE,
								"Overwriting time %g in graphics object '%s'",time,
								graphics_object_name);
							return_code=GT_object_delete_time(graphics_object,time);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Object of different type named '%s' already exists",
							graphics_object_name);
						return_code=0;
					}
				}
				else
				{
					if ((graphics_object=CREATE(GT_object)(graphics_object_name,
						g_SURFACE,material))&&
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_cylinders.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
				if (return_code)
				{
					element_to_cylinder_data.coordinate_field=
						Computed_field_begin_wrap_coordinate_field(coordinate_field);
					element_to_cylinder_data.element_group=element_group;
					element_to_cylinder_data.exterior=exterior_flag;
					element_to_cylinder_data.face_number=face_number;
					/* radius = constant_radius + scale_factor*radius_scalar */
					element_to_cylinder_data.constant_radius=constant_radius;
					element_to_cylinder_data.radius_field=radius_field;
					element_to_cylinder_data.scale_factor=scale_factor;
					element_to_cylinder_data.graphics_object=graphics_object;
					element_to_cylinder_data.time=time;
					element_to_cylinder_data.material=material;
					element_to_cylinder_data.data_field=data_field;
					element_to_cylinder_data.number_of_segments_along=
						discretization.number_in_xi1;
					element_to_cylinder_data.number_of_segments_around=
						discretization.number_in_xi2;
					if (element_group)
					{
						return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
							element_to_cylinder,(void *)&element_to_cylinder_data,
							element_group);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							element_to_cylinder,(void *)&element_to_cylinder_data,
							command_data->element_manager);
					}
					if (return_code&&GT_object_has_time(graphics_object,time))
					{
						if (data_field)
						{
							return_code=set_GT_object_Spectrum(graphics_object,spectrum);
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"No cylinders created");
						if (0==GT_object_get_number_of_times(graphics_object))
						{
							REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
								command_data->graphics_object_list);
						}
					}
					Computed_field_end_wrap(&(element_to_cylinder_data.coordinate_field));
				}
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (data_field)
			{
				DEACCESS(Computed_field)(&data_field);
			}
			if (radius_field)
			{
				DEACCESS(Computed_field)(&radius_field);
			}
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
			DEACCESS(Graphical_material)(&material);
			DEACCESS(Spectrum)(&spectrum);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_cylinders.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_cylinders.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_cylinders */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_create_element_creator(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Executes a GFX CREATE ELEMENT_CREATOR command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_element_creator);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (command_data->element_creator)
				{
					return_code=Element_creator_bring_window_to_front(
						command_data->element_creator);
				}
				else
				{
					if (CREATE(Element_creator)(&(command_data->element_creator),
						command_data->basis_manager,command_data->element_manager,
						command_data->fe_field_manager,command_data->node_manager,
						command_data->element_selection,command_data->node_selection,
						command_data->user_interface))
					{
						return_code=1;
					}
					else
					{
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_element_creator.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_element_creator.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_element_creator */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_element_points(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 January 2000

DESCRIPTION :
Executes a GFX CREATE ELEMENT_POINTS command.
==============================================================================*/
{
	char exterior_flag,*graphics_object_name,*use_element_type_string,
		**valid_strings,*xi_discretization_mode_string;
	float time;
	int face_number,number_of_components,number_of_valid_strings,return_code;
	static char default_name[]="element_points";
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field,*data_field,*label_field,
		*orientation_scale_field;
	struct Element_discretization discretization;
	struct Element_to_glyph_set_data element_to_glyph_set_data;
	struct FE_field *native_discretization_field;
	struct GROUP(FE_element) *element_group;
	struct Graphical_material *material;
	struct GT_object *glyph,*graphics_object;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Spectrum *spectrum;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data,set_label_field_data,set_orientation_scale_field_data;
	Triple glyph_centre,glyph_scale_factors,glyph_size;

	ENTER(gfx_create_element_points);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			if (ALLOCATE(graphics_object_name,char,strlen(default_name)+1))
			{
				strcpy(graphics_object_name,default_name);
			}
			else
			{
				graphics_object_name=(char *)NULL;
			}
			number_of_components=3;
			glyph_centre[0]=0.0;
			glyph_centre[1]=0.0;
			glyph_centre[2]=0.0;
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			ACCESS(Computed_field)(coordinate_field);
			data_field=(struct Computed_field *)NULL;
			exterior_flag=0;
			face_number=0;
			element_group=(struct GROUP(FE_element) *)NULL;
			if (glyph=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("point",
				command_data->glyph_list))
			{
				ACCESS(GT_object)(glyph);
			}
			label_field=(struct Computed_field *)NULL;
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			native_discretization_field=(struct FE_field *)NULL;
			orientation_scale_field=(struct Computed_field *)NULL;
			glyph_scale_factors[0]=1.0;
			glyph_scale_factors[1]=1.0;
			glyph_scale_factors[2]=1.0;
			glyph_size[0]=1.0;
			glyph_size[1]=1.0;
			glyph_size[2]=1.0;
			spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
			time=0.0;
			discretization.number_in_xi1=1;
			discretization.number_in_xi2=1;
			discretization.number_in_xi3=1;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* cell_centres/cell_corners/cell_random */ 
			xi_discretization_mode_string=
				Xi_discretization_mode_string(XI_DISCRETIZATION_CELL_CENTRES);
			valid_strings=
				Xi_discretization_mode_get_valid_strings(&number_of_valid_strings);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&xi_discretization_mode_string);
			DEALLOCATE(valid_strings);
			/* centre [of glyph] */
			Option_table_add_entry(option_table,"centre",glyph_centre,
				&(number_of_components),set_float_vector);
			/* coordinate */
			set_coordinate_field_data.computed_field_package=
				command_data->computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_package=
				command_data->computed_field_package;
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&data_field,
				&set_data_field_data,set_Computed_field_conditional);
			/* exterior */
			Option_table_add_entry(option_table,"exterior",&exterior_flag,
				NULL,set_char_flag);
			/* face */
			Option_table_add_entry(option_table,"face",&face_number,
				NULL,set_exterior);
			/* from [element_group] */
			Option_table_add_entry(option_table,"from",&element_group,
				command_data->element_group_manager,set_FE_element_group);
			/* glyph */
			Option_table_add_entry(option_table,"glyph",&glyph,
				command_data->glyph_list,set_Graphics_object);
			/* label */
			set_label_field_data.computed_field_package=
				command_data->computed_field_package;
			set_label_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_label_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"label",&label_field,
				&set_label_field_data,set_Computed_field_conditional);
			/* material */
			Option_table_add_entry(option_table,"material",&material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* native_discretization */
			Option_table_add_entry(option_table,"native_discretization",
				&native_discretization_field,command_data->fe_field_manager,
				set_FE_field);
			/* orientation */
			set_orientation_scale_field_data.computed_field_package=
				command_data->computed_field_package;
			set_orientation_scale_field_data.conditional_function=
				Computed_field_is_orientation_scale_capable;
			set_orientation_scale_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"orientation",
				&orientation_scale_field,&set_orientation_scale_field_data,
				set_Computed_field_conditional);
			/* scale_factors */
			Option_table_add_entry(option_table,"scale_factors",
				glyph_scale_factors,"*",set_special_float3);
			/* size [of glyph] */
			Option_table_add_entry(option_table,"size",
				glyph_size,"*",set_special_float3);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",
				&spectrum,command_data->spectrum_manager,set_Spectrum);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* with */
			Option_table_add_entry(option_table,"with",&discretization,
				command_data->user_interface,set_Element_discretization);
			/* use_elements/use_faces/use_lines */
			use_element_type_string=Use_element_type_string(USE_ELEMENTS);
			valid_strings=
				Use_element_type_get_valid_strings(&number_of_valid_strings);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&use_element_type_string);
			DEALLOCATE(valid_strings);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				face_number -= 2;
				if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list))
				{
					if (g_GLYPH_SET == graphics_object->object_type)
					{
						if (GT_object_has_time(graphics_object,time))
						{
							display_message(WARNING_MESSAGE,
								"Overwriting time %g in graphics object '%s'",time,
								graphics_object_name);
							return_code=GT_object_delete_time(graphics_object,time);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Object of different type named '%s' already exists",
							graphics_object_name);
						return_code=0;
					}
				}
				else
				{
					if ((graphics_object=CREATE(GT_object)(graphics_object_name,
						g_GLYPH_SET,material))&&
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_element_points.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
				if (return_code)
				{
					element_to_glyph_set_data.time=time;
					element_to_glyph_set_data.xi_discretization_mode=
						Xi_discretization_mode_from_string(xi_discretization_mode_string);
					element_to_glyph_set_data.coordinate_field=
						Computed_field_begin_wrap_coordinate_field(coordinate_field);
					if (orientation_scale_field)
					{
						element_to_glyph_set_data.orientation_scale_field=
							Computed_field_begin_wrap_orientation_scale_field(
								orientation_scale_field,
								element_to_glyph_set_data.coordinate_field);
					}
					else
					{
						element_to_glyph_set_data.orientation_scale_field=
							(struct Computed_field *)NULL;
					}
					element_to_glyph_set_data.use_element_type=
						Use_element_type_from_string(use_element_type_string);
					element_to_glyph_set_data.element_group=element_group;
					element_to_glyph_set_data.exterior=exterior_flag;
					element_to_glyph_set_data.face_number=face_number;
					element_to_glyph_set_data.number_in_xi[0]=
						discretization.number_in_xi1;
					element_to_glyph_set_data.number_in_xi[1]=
						discretization.number_in_xi2;
					element_to_glyph_set_data.number_in_xi[2]=
						discretization.number_in_xi3;
					element_to_glyph_set_data.data_field=data_field;
					element_to_glyph_set_data.label_field=label_field;
					element_to_glyph_set_data.native_discretization_field=
						native_discretization_field;
					element_to_glyph_set_data.glyph=glyph;
					element_to_glyph_set_data.graphics_object=graphics_object;
					element_to_glyph_set_data.glyph_centre[0]=glyph_centre[0];
					element_to_glyph_set_data.glyph_centre[1]=glyph_centre[1];
					element_to_glyph_set_data.glyph_centre[2]=glyph_centre[2];
					element_to_glyph_set_data.glyph_size[0]=glyph_size[0];
					element_to_glyph_set_data.glyph_size[1]=glyph_size[1];
					element_to_glyph_set_data.glyph_size[2]=glyph_size[2];
					element_to_glyph_set_data.glyph_scale_factors[0]=
						glyph_scale_factors[0];
					element_to_glyph_set_data.glyph_scale_factors[1]=
						glyph_scale_factors[1];
					element_to_glyph_set_data.glyph_scale_factors[2]=
						glyph_scale_factors[2];
					element_to_glyph_set_data.select_mode=GRAPHICS_NO_SELECT;
					if (element_group)
					{
						return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
							element_to_glyph_set,(void *)&element_to_glyph_set_data,
							element_group);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							element_to_glyph_set,(void *)&element_to_glyph_set_data,
							command_data->element_manager);
					}
					if (return_code&&GT_object_has_time(graphics_object,time))
					{
						if (data_field)
						{
							return_code=set_GT_object_Spectrum(graphics_object,spectrum);
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"No element_points created");
						if (0==GT_object_get_number_of_times(graphics_object))
						{
							REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
								command_data->graphics_object_list);
						}
					}
					if (element_to_glyph_set_data.orientation_scale_field)
					{
						Computed_field_end_wrap(
							&(element_to_glyph_set_data.orientation_scale_field));
					}
					Computed_field_end_wrap(
						&(element_to_glyph_set_data.coordinate_field));
				}
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (data_field)
			{
				DEACCESS(Computed_field)(&data_field);
			}
			DEACCESS(Spectrum)(&spectrum);
			DEACCESS(Graphical_material)(&material);
			DEALLOCATE(graphics_object_name);
			if (glyph)
			{
				DEACCESS(GT_object)(&glyph);
			}
			if (orientation_scale_field)
			{
				DEACCESS(Computed_field)(&orientation_scale_field);
			}
			if (native_discretization_field)
			{
				DEACCESS(FE_field)(&native_discretization_field);
			}
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_element_points.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_element_points.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_element_points */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_element_group(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 23 April 1999

DESCRIPTION :
Executes a GFX CREATE EGROUP command.
==============================================================================*/
{
	char *name;
	int return_code;
	struct Add_FE_element_to_list_if_in_range_data element_in_range_data;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group,*from_element_group;
	struct GROUP(FE_node) *data_group,*node_group;
	struct LIST(FE_node) *node_list;
	static struct Modifier_entry option_table[]=
	{
		{"add_ranges",NULL,NULL,set_Multi_range},
		{"from",NULL,NULL,set_FE_element_group},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_create_element_group);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		name=(char *)NULL;
		if (set_name(state,(void *)&name,(void *)1))
		{
			/* initialise defaults */
			from_element_group=(struct GROUP(FE_element) *)NULL;
			element_in_range_data.element_list=CREATE(LIST(FE_element))();
			element_in_range_data.element_ranges=CREATE(Multi_range)();
			(option_table[0]).to_be_modified=element_in_range_data.element_ranges;
			(option_table[1]).to_be_modified=&from_element_group;
			(option_table[1]).user_data=command_data->element_group_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (name&&!FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(name,
					command_data->data_group_manager)&&
					!FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(name,
						command_data->node_group_manager)&&
					!FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)(name,
						command_data->element_group_manager))
				{
					/* create node and element groups of same name simultaneously */
					data_group=CREATE_GROUP(FE_node)(name);
					node_group=CREATE_GROUP(FE_node)(name);
					element_group=CREATE_GROUP(FE_element)(name);
					if (data_group&&node_group&&element_group)
					{
						if (0<Multi_range_get_number_of_ranges(
							element_in_range_data.element_ranges))
						{
							/* make list of elements to add to group, then add to group */
							if (from_element_group)
							{
								return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
									add_FE_element_to_list_if_in_range,
									(void *)&element_in_range_data,from_element_group);
							}
							else
							{
								return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
									add_FE_element_to_list_if_in_range,
									(void *)&element_in_range_data,command_data->element_manager);
							}
							if (return_code)
							{
								/* also fill node_group with nodes used by these elements */
								node_list=CREATE(LIST(FE_node))();
								return_code=FOR_EACH_OBJECT_IN_LIST(FE_element)(
									ensure_FE_element_and_faces_are_in_group,
									(void *)element_group,element_in_range_data.element_list)&&
									FOR_EACH_OBJECT_IN_LIST(FE_element)(
										ensure_top_level_FE_element_nodes_are_in_list,
										(void *)node_list,element_in_range_data.element_list)&&
									FOR_EACH_OBJECT_IN_LIST(FE_node)(ensure_FE_node_is_in_group,
										(void *)node_group,node_list);
								DESTROY(LIST(FE_node))(&node_list);
							}
						}
						if (return_code)
						{
							/* must add node group before element group so the node group
								 exists when a GT_element_group is made for the element group */
							if (ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(data_group,
								command_data->data_group_manager)&&
								ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(node_group,
									command_data->node_group_manager)&&
								ADD_OBJECT_TO_MANAGER(GROUP(FE_element))(element_group,
									command_data->element_group_manager))
							{
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,"gfx_create_element_group.  "
									"Could not add data/node/element group(s) to manager");
								DESTROY_GROUP(FE_element)(&element_group);
								if (FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
									name,command_data->node_group_manager))
								{
									REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(node_group,
										command_data->node_group_manager);
								}
								else
								{
									DESTROY_GROUP(FE_node)(&node_group);
								}
								if (FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
									name,command_data->data_group_manager))
								{
									REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(data_group,
										command_data->data_group_manager);
								}
								else
								{
									DESTROY_GROUP(FE_node)(&data_group);
								}
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"gfx_create_element_group.  "
								"Could not add fill node/element group(s)");
							DESTROY(GROUP(FE_element))(&element_group);
							DESTROY(GROUP(FE_node))(&node_group);
							DESTROY(GROUP(FE_node))(&data_group);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"gfx_create_element_group.  "
							"Could not create data/node/element group(s)");
						DESTROY(GROUP(FE_element))(&element_group);
						DESTROY(GROUP(FE_node))(&node_group);
						DESTROY(GROUP(FE_node))(&data_group);
						return_code=0;
					}
				}
				else
				{
					if (name)
					{
						display_message(ERROR_MESSAGE,"gfx_create_element_group.  "
							"Data/node/element group '%s' already exists",name);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_element_group.  Must specify a name for the group");
					}
					return_code=0;
				}
			}
			DESTROY(Multi_range)(&element_in_range_data.element_ranges);
			DESTROY(LIST(FE_element))(&element_in_range_data.element_list);
			if (from_element_group)
			{
				DEACCESS(GROUP(FE_element))(&from_element_group);
			}
		}
		if (name)
		{
			DEALLOCATE(name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_element_group.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_element_group */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_environment_map(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 2 October 1996

DESCRIPTION :
Executes a GFX CREATE ENVIRONMENT_MAP command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Environment_map *environment_map;
	struct Modify_environment_map_data modify_environment_map_data;

	ENTER(gfx_create_environment_map);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(Environment_map,name)(
						current_token,command_data->environment_map_manager))
					{
						if (environment_map=CREATE(Environment_map)(current_token))
						{
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								modify_environment_map_data.graphical_material_manager=
									command_data->graphical_material_manager;
								modify_environment_map_data.environment_map_manager=
									command_data->environment_map_manager;
								return_code=modify_Environment_map(state,
									(void *)environment_map,
									(void *)(&modify_environment_map_data));
							}
							else
							{
								return_code=1;
							}
							ADD_OBJECT_TO_MANAGER(Environment_map)(environment_map,
								command_data->environment_map_manager);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_environment_map.  Error creating environment_map");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Environment map already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					modify_environment_map_data.graphical_material_manager=
						command_data->graphical_material_manager;
					modify_environment_map_data.environment_map_manager=
						command_data->environment_map_manager;
					return_code=modify_Environment_map(state,
						(void *)NULL,(void *)(&modify_environment_map_data));
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_environment_map.  Missing command_data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing environment_map_name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_environment_map.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_environment_map */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if defined (OLD_CODE)
#if !defined (WINDOWS_DEV_FLAG)
static int set_intersect(struct Parse_state *state,
	void *intersect_mu_theta_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 27 June 1996

DESCRIPTION :
A modifier function for setting mu and theta (prolate spheroidal coordinates)
for intersect - look at how fibre angle varies through the heart wall.
???DB.  Specialist
==============================================================================*/
{
	char *current_token;
	float *intersect_mu_theta;
	int return_code;

	ENTER(set_intersect);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (intersect_mu_theta=(float *)intersect_mu_theta_void)
				{
					/* turn on intersect */
					*intersect_mu_theta=1;
					if (1==sscanf(current_token,"%f",intersect_mu_theta+1))
					{
						shift_Parse_state(state,1);
						if (current_token=state->current_token)
						{
							if (1==sscanf(current_token,"%f",intersect_mu_theta+2))
							{
								shift_Parse_state(state,1);
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,"Invalid theta: %s",
									current_token);
								display_parse_state_location(state);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Missing theta");
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid mu: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_intersect.  Missing intersect_mu_theta");
					return_code=0;
				}
			}
			else
			{
				if (intersect_mu_theta=(float *)intersect_mu_theta_void)
				{
					display_message(INFORMATION_MESSAGE," MU[%g] THETA[%g]",
						intersect_mu_theta[1],intersect_mu_theta[2]);
				}
				else
				{
					display_message(INFORMATION_MESSAGE," MU THETA");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing mu");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_intersect.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_intersect */
#endif /* !defined (WINDOWS_DEV_FLAG) */
#endif /* defined (OLD_CODE) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_flow_particles(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE FLOW_PARTICLES command.
==============================================================================*/
{
	static char default_name[]="particles";
	char *graphics_object_name;
	float time,xi[3];
	gtObject *graphics_object;
	struct GT_pointset *pointset;
	int element_number,i,number_of_particles,return_code,
		vector_components;
	struct Cmiss_command_data *command_data;
	struct Element_to_particle_data element_to_particle_data;
	struct Computed_field *coordinate_field,*stream_vector_field;
	struct GROUP(FE_element) *element_group;
	struct Graphical_material *material;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Spectrum *spectrum;
	Triple *particle_positions,*final_particle_positions;
	static struct Modifier_entry option_table[]=
	{
		{"as",NULL,(void *)1,set_name},
		{"coordinate",NULL,NULL,set_Computed_field_conditional},
		{"element",NULL,NULL,set_int_non_negative},
		{"from",NULL,NULL,set_FE_element_group},
		{"initial_xi",NULL,NULL,set_FE_value_array},
		{"material",NULL,NULL,set_Graphical_material},
		{"spectrum",NULL,NULL,set_Spectrum},
		{"time",NULL,NULL,set_float},
		{"vector",NULL,NULL,set_Computed_field_conditional},
		{NULL,NULL,NULL,NULL}
	};
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_stream_vector_field_data;

	ENTER(gfx_create_flow_particles);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			if (ALLOCATE(graphics_object_name,char,strlen(default_name)+1))
			{
				strcpy(graphics_object_name,default_name);
			}
			else
			{
				graphics_object_name=(char *)NULL;
			}
			element_group=(struct GROUP(FE_element) *)NULL;
			element_number=0;  /* Zero gives all elements in group */
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			if (stream_vector_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_is_stream_vector_capable,(void *)NULL,
				computed_field_manager))
			{
				ACCESS(Computed_field)(stream_vector_field);
			}
			vector_components=3;
			xi[0]=0.5;
			xi[1]=0.5;
			xi[2]=0.5;
			time=0;
			/* must access it now,because we deaccess it later */
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			spectrum=
				ACCESS(Spectrum)(command_data->default_spectrum);
			i=0;
			/* as */
			(option_table[i]).to_be_modified= &graphics_object_name;
			i++;
			/* coordinate */
			set_coordinate_field_data.computed_field_package=
				command_data->computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[i]).to_be_modified= &coordinate_field;
			(option_table[i]).user_data= &set_coordinate_field_data;
			i++;
			/* element */
			(option_table[i]).to_be_modified= &element_number;
			i++;
			/* from */
			(option_table[i]).to_be_modified= &element_group;
			(option_table[i]).user_data=command_data->element_group_manager;
			i++;
			/* initial_xi */
			(option_table[i]).to_be_modified=xi;
			(option_table[i]).user_data= &(vector_components);
			i++;
			/* material */
			(option_table[i]).to_be_modified= &material;
			(option_table[i]).user_data=command_data->graphical_material_manager;
			i++;
			/* spectrum */
			(option_table[i]).to_be_modified= &spectrum;
			(option_table[i]).user_data=command_data->spectrum_manager;
			i++;
			/* time */
			(option_table[i]).to_be_modified= &time;
			i++;
			/* vector */
			set_stream_vector_field_data.computed_field_package=
				command_data->computed_field_package;
			set_stream_vector_field_data.conditional_function=
				Computed_field_is_stream_vector_capable;
			set_stream_vector_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[i]).to_be_modified= &stream_vector_field;
			(option_table[i]).user_data= &set_stream_vector_field_data;
			i++;
			return_code=process_multiple_options(state,option_table);
			/* no errors,not asking for help */
			if (return_code)
			{
				if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list))
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_flow_particles.  Object already exists");
					return_code=0;
				}
				else
				{
					if ((graphics_object=CREATE(GT_object)(graphics_object_name,
						g_POINTSET,material))&&
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_flow_particles.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
				if (return_code)
				{
					if (element_group)
					{
						number_of_particles=NUMBER_IN_GROUP(FE_element)(element_group);
					}
					else
					{
						number_of_particles=
							NUMBER_IN_MANAGER(FE_element)(command_data->element_manager);
					}
					if (ALLOCATE(particle_positions,Triple,number_of_particles))
					{
						if (pointset=CREATE(GT_pointset)(number_of_particles,
							particle_positions,(char **)NULL,	g_POINT_MARKER,
							1,g_NO_DATA,(GTDATA *) NULL,(int *)NULL))
						{
							element_to_particle_data.coordinate_field=coordinate_field;
							element_to_particle_data.element_number=element_number;
							element_to_particle_data.stream_vector_field=stream_vector_field;
							element_to_particle_data.pointlist= &(pointset->pointlist);
							element_to_particle_data.index=0;
							element_to_particle_data.graphics_object=graphics_object;
							element_to_particle_data.xi[0]=xi[0];
							element_to_particle_data.xi[1]=xi[1];
							element_to_particle_data.xi[2]=xi[2];
							element_to_particle_data.number_of_particles=0;
							element_to_particle_data.list= &(command_data->streampoint_list);
							if (element_group)
							{
								return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
									element_to_particle,(void *)&element_to_particle_data,
									element_group);
							}
							else
							{
								return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
									element_to_particle,(void *)&element_to_particle_data,
									command_data->element_manager);
							}
							number_of_particles=element_to_particle_data.number_of_particles;
							if (return_code && number_of_particles &&
								REALLOCATE(final_particle_positions,particle_positions,
								Triple,number_of_particles))
							{
								pointset->pointlist=final_particle_positions;
								pointset->n_pts=number_of_particles;
								if (GT_OBJECT_ADD(GT_pointset)(graphics_object,time,pointset))
								{
									return_code=set_GT_object_Spectrum(graphics_object,spectrum);
								}
								else
								{
									DESTROY(GT_pointset)(&pointset);
									display_message(ERROR_MESSAGE,
			"gfx_create_flow_particles.  Could not add pointset to graphics object");
									return_code=0;
								}
							}
							else
							{
								DESTROY(GT_pointset)(&pointset);
								if (return_code)
								{
									display_message(WARNING_MESSAGE,"No particles created");
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"gfx_create_flow_particles.  Error creating particles");
								}
							}
						}
						else
						{
							DEALLOCATE (particle_positions);
							display_message(ERROR_MESSAGE,
								"gfx_create_flow_particles.  Unable to create pointset");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
"gfx_create_flow_particles.  Unable to allocate memory for particle positions");
						return_code=0;
					}
				}
			} /* parse error,help */
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (stream_vector_field)
			{
				DEACCESS(Computed_field)(&stream_vector_field);
			}
			DEACCESS(Spectrum)(&spectrum);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_flow_particles.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_flow_particles.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* gfx_create_flow_particles */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_more_flow_particles(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE MORE_FLOW_PARTICLES command.
==============================================================================*/
{
	static char default_name[]="particles";
	char *graphics_object_name;
	float time,xi[3];
	gtObject *graphics_object;
	int current_number_of_particles,element_number,i,number_of_particles,
		return_code,vector_components;
	struct Cmiss_command_data *command_data;
	struct Element_to_particle_data element_to_particle_data;
	struct Computed_field *coordinate_field,*stream_vector_field;
	struct GROUP(FE_element) *element_group;
	struct Graphical_material *material;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Spectrum *spectrum;
	Triple *new_particle_positions,*particle_positions,
		*final_particle_positions;
	static struct Modifier_entry option_table[]=
	{
		{"as",NULL,(void *)1,set_name},
		{"coordinate",NULL,NULL,set_Computed_field_conditional},
		{"element",NULL,NULL,set_int_non_negative},
		{"from",NULL,NULL,set_FE_element_group},
		{"initial_xi",NULL,NULL,set_float_vector},
		{"material",NULL,NULL,set_Graphical_material},
		{"spectrum",NULL,NULL,set_Spectrum},
		{"time",NULL,NULL,set_float},
		{"vector",NULL,NULL,set_Computed_field_conditional},
		{NULL,NULL,NULL,NULL}
	};
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_stream_vector_field_data;

	ENTER(gfx_create_flow_particles);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			if (ALLOCATE(graphics_object_name,char,strlen(default_name)+1))
			{
				strcpy(graphics_object_name,default_name);
			}
			else
			{
				graphics_object_name=(char *)NULL;
			}
			element_group=(struct GROUP(FE_element) *)NULL;
			element_number=0;  /* Zero gives all elements in group */
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			if (stream_vector_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_is_stream_vector_capable,(void *)NULL,
				computed_field_manager))
			{
				ACCESS(Computed_field)(stream_vector_field);
			}
			vector_components=3;
			xi[0]=0.5;
			xi[1]=0.5;
			xi[2]=0.5;
			time=0;
			/* must access it now,because we deaccess it later */
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			spectrum=
				ACCESS(Spectrum)(command_data->default_spectrum);
			i=0;
			/* as */
			(option_table[i]).to_be_modified= &graphics_object_name;
			i++;
			/* coordinate */
			set_coordinate_field_data.computed_field_package=
				command_data->computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[i]).to_be_modified= &coordinate_field;
			(option_table[i]).user_data= &set_coordinate_field_data;
			i++;
			/* element */
			(option_table[i]).to_be_modified= &element_number;
			i++;
			/* from */
			(option_table[i]).to_be_modified= &element_group;
			(option_table[i]).user_data=command_data->element_group_manager;
			i++;
			/* initial_xi */
			(option_table[i]).to_be_modified=xi;
			(option_table[i]).user_data= &(vector_components);
			i++;
			/* material */
			(option_table[i]).to_be_modified= &material;
			(option_table[i]).user_data=command_data->graphical_material_manager;
			i++;
			/* spectrum */
			(option_table[i]).to_be_modified= &spectrum;
			(option_table[i]).user_data=command_data->spectrum_manager;
			i++;
			/* time */
			(option_table[i]).to_be_modified= &time;
			i++;
			/* vector */
			set_stream_vector_field_data.computed_field_package=
				command_data->computed_field_package;
			set_stream_vector_field_data.conditional_function=
				Computed_field_is_stream_vector_capable;
			set_stream_vector_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[i]).to_be_modified= &stream_vector_field;
			(option_table[i]).user_data= &set_stream_vector_field_data;
			i++;
			return_code=process_multiple_options(state,option_table);
			/* no errors,not asking for help */
			if (return_code)
			{
				if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list))
				{
					particle_positions=(*((graphics_object->gu).gt_pointset))->pointlist;
					current_number_of_particles=
						(*((graphics_object->gu).gt_pointset))->n_pts;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_flow_particles.  Graphics object does not exist");
					return_code=0;
				}
				if (return_code)
				{
					if (element_group)
					{
						number_of_particles=current_number_of_particles +
							NUMBER_IN_GROUP(FE_element)(element_group);
					}
					else
					{
						number_of_particles=current_number_of_particles +
							NUMBER_IN_MANAGER(FE_element)(command_data->element_manager);
					}
					if (REALLOCATE(new_particle_positions,particle_positions,Triple,
						number_of_particles))
					{
						(*((graphics_object->gu).gt_pointset))->pointlist=
							new_particle_positions;
						element_to_particle_data.coordinate_field=coordinate_field;
						element_to_particle_data.element_number=element_number;
						element_to_particle_data.stream_vector_field=stream_vector_field;
						element_to_particle_data.index=current_number_of_particles;
						element_to_particle_data.pointlist=
							&((*((graphics_object->gu).gt_pointset))->pointlist);
						element_to_particle_data.graphics_object=graphics_object;
						element_to_particle_data.xi[0]=xi[0];
						element_to_particle_data.xi[1]=xi[1];
						element_to_particle_data.xi[2]=xi[2];
						element_to_particle_data.number_of_particles=0;
						element_to_particle_data.list= &(command_data->streampoint_list);
						if (element_group)
						{
							return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
								element_to_particle,(void *)&element_to_particle_data,
								element_group);
						}
						else
						{
							return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
								element_to_particle,(void *)&element_to_particle_data,
								command_data->element_manager);
						}
						number_of_particles=current_number_of_particles +
							element_to_particle_data.number_of_particles;
						if (return_code && number_of_particles &&
							REALLOCATE(final_particle_positions,new_particle_positions,
								Triple,number_of_particles))
						{
							(*((graphics_object->gu).gt_pointset))->pointlist=
								final_particle_positions;
							(*((graphics_object->gu).gt_pointset))->n_pts=number_of_particles;
							GT_object_changed(graphics_object);
						}
						else
						{
							DEALLOCATE(particle_positions);
							if (return_code)
							{
								display_message(WARNING_MESSAGE,"No particles created");
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_create_more_flow_particles.  Error creating particles");
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
"gfx_create_more_flow_particles.  Unable to allocate memory for particle positions");
					}
				}
			} /* parse error,help */
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (stream_vector_field)
			{
				DEACCESS(Computed_field)(&stream_vector_field);
			}
			DEACCESS(Spectrum)(&spectrum);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_more_flow_particles.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_more_flow_particles.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* gfx_create_more_flow_particles */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_modify_flow_particles(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 January 2000

DESCRIPTION :
Executes a GFX MODIFY FLOW_PARTICLES command.
==============================================================================*/
{
	int return_code;
	float stepsize,time;
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field,*stream_vector_field;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_stream_vector_field_data;

	ENTER(gfx_modify_flow_particles);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			if (stream_vector_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_is_stream_vector_capable,(void *)NULL,
				computed_field_manager))
			{
				ACCESS(Computed_field)(stream_vector_field);
			}
			stepsize=1;
			/* If time of 0 is sent the previous points are updated at the previous
				time value */
			time=0;

			option_table=CREATE(Option_table)();
			/* coordinate */
			set_coordinate_field_data.computed_field_package=
				command_data->computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* stepsize */
			Option_table_add_entry(option_table,"stepsize",&stepsize,
				NULL,set_float);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* vector */
			set_stream_vector_field_data.computed_field_package=
				command_data->computed_field_package;
			set_stream_vector_field_data.conditional_function=
				Computed_field_is_stream_vector_capable;
			set_stream_vector_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"vector",&stream_vector_field,
				&set_stream_vector_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code=update_flow_particle_list(
					command_data->streampoint_list,coordinate_field,stream_vector_field,
					stepsize,time);
			}
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (stream_vector_field)
			{
				DEACCESS(Computed_field)(&stream_vector_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_flow_particles.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_flow_particles.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_flow_particles */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_create_g_element_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 31 August 1999

DESCRIPTION :
Executes a GFX CREATE G_ELEMENT_EDITOR command.
Invokes the graphical element group editor.
==============================================================================*/
{
	int return_code;
	struct GROUP(FE_element) *element_group;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"from",NULL,NULL,set_FE_element_group},
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,NULL}
	};
	struct Scene *scene;

	ENTER(gfx_create_g_element_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults - try to get existing ones or at least not the
				 active group if possible */
			element_group=FIRST_OBJECT_IN_MANAGER_THAT(GROUP(FE_element))(
				(MANAGER_CONDITIONAL_FUNCTION(GROUP(FE_element)) *)NULL,(void *)NULL,
				command_data->element_group_manager);
			scene=ACCESS(Scene)(command_data->default_scene);
			if (command_data->element_group_editor_dialog)
			{
				graphical_element_editor_dialog_get_element_group_and_scene(
					command_data->element_group_editor_dialog,
					&element_group,&scene);
			}
			if (element_group)
			{
				ACCESS(GROUP(FE_element))(element_group);
			}
			if (scene)
			{
				ACCESS(Scene)(scene);
			}
			(option_table[0]).to_be_modified= &element_group;
			(option_table[0]).user_data=command_data->element_group_manager;
			(option_table[1]).to_be_modified= &scene;
			(option_table[1]).user_data=command_data->scene_manager;
			if (return_code=process_multiple_options(state,option_table))
			{
				return_code=bring_up_graphical_element_editor_dialog(
					&(command_data->element_group_editor_dialog),
					command_data->user_interface->application_shell,
					command_data->computed_field_package,
					command_data->element_manager,
					command_data->element_group_manager,element_group,
					command_data->fe_field_manager,
					command_data->graphical_material_manager,
					command_data->default_graphical_material,command_data->glyph_list,
					command_data->scene_manager,scene,command_data->spectrum_manager,
					command_data->default_spectrum,command_data->volume_texture_manager,
					command_data->user_interface);
			} /* parse error, help */
			DEACCESS(Scene)(&scene);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_create_g_element_editor.  "
				"Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_g_element_editor.  "
			"Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_g_element_editor */

static int gfx_create_graphical_material_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE GRAPHICAL_MATERIAL_EDITOR command.
If there is a material editor dialog in existence, then bring it to the front,
otherwise it creates a new one.  Assumes we will only ever want one material
editor at a time.  This implementation may be changed later.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_graphical_material_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				return_code=bring_up_material_editor_dialog(
					&(command_data->material_editor_dialog),
					command_data->user_interface->application_shell,
					command_data->graphical_material_manager,
					command_data->texture_manager,(struct Graphical_material *)NULL,
					command_data->user_interface);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_graphical_material_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_graphical_material_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_graphical_material_editor */

static int gfx_create_grid_field_calculator(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 December 1999

DESCRIPTION :
Executes a GFX CREATE GRID_FIELD_CALCULATOR command.
Invokes the grid field calculator dialog.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_grid_field_calculator);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				return_code=bring_up_grid_field_calculator(
					&(command_data->grid_field_calculator_dialog),
					command_data->user_interface->application_shell,
					command_data->computed_field_package,
					&(command_data->control_curve_editor_dialog),
					command_data->control_curve_manager,
					command_data->element_manager,
					command_data->user_interface);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_grid_field_calculator.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_grid_field_calculator.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_grid_field_calculator */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_input_module_control(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE IM_CONTROL command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_input_module_control);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
#if defined (EXT_INPUT)
				return_code=bring_up_input_module_dialog(
					&(command_data->input_module_dialog),
					command_data->user_interface->application_shell,
					command_data->default_graphical_material,
					command_data->graphical_material_manager,command_data->default_scene,
					command_data->scene_manager);
					/*???DB.  commmand_data should not be used outside of command.c */
#else /* defined (EXT_INPUT) */
				display_message(ERROR_MESSAGE,"External input module was not linked");
#endif /* defined (EXT_INPUT) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_input_module_control.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_input_module_control.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_input_module_control */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_interactive_data_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE INTERACTIVE_DATA_EDITOR command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_interactive_data_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				return_code=bring_up_interactive_node_editor_dialog(
					&(command_data->interactive_data_editor_dialog),
					command_data->user_interface->application_shell,
					command_data->data_manager,command_data->execute_command,
					(struct FE_node *)NULL);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_interactive_data_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_interactive_data_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_interactive_data_editor */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_interactive_node_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE INTERACTIVE_NODE_EDITOR command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_interactive_node_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				return_code=bring_up_interactive_node_editor_dialog(
					&(command_data->interactive_node_editor_dialog),
					command_data->user_interface->application_shell,
					command_data->node_manager,command_data->execute_command,
					(struct FE_node *)NULL);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_interactive_node_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_interactive_node_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_interactive_node_editor */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if defined (OLD_CODE)
static int set_iso_field_with_floats(struct Parse_state *state,
	void *iso_field_data_ptr_void,void *type_user_data)
/*******************************************************************************
LAST MODIFIED : 6 January 1998

DESCRIPTION :
A modifier function to set the iso_field_calculation_type and the associated
float parameters.
==============================================================================*/
{
	char *current_token;
	float coeffs[3], *trace_coeffs;
	int return_code;
	struct Iso_field_calculation_data *data;
	enum Iso_field_calculation_type type;
	int number_of_points, i;

	ENTER(set_iso_field_with_floats);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (data=
					*((struct Iso_field_calculation_data **)iso_field_data_ptr_void))
				{
					if (type= *((enum Iso_field_calculation_type *)type_user_data))
					{
						switch (type)
						{
							case COORDINATE_PLANE:
							case COORDINATE_SPHERE:
							{
								if ((1==sscanf(current_token," %f ",&(coeffs[0])))&&
									shift_Parse_state(state,1)&&(state->current_token)&&
									(1==sscanf(state->current_token," %f ",&(coeffs[1])))&&
									shift_Parse_state(state,1)&&(state->current_token)&&
									(1==sscanf(state->current_token," %f ",&(coeffs[2]))))
								{
									set_Iso_field_calculation_with_floats(data,type,3,coeffs);
									return_code=shift_Parse_state(state,1);
								}
								else
								{
									display_message(WARNING_MESSAGE,
										"Missing/invalid coeff value(s)");
									display_parse_state_location(state);
									return_code=0;
								}
							} break;
							case VERTICAL_POINT_TRACE:
							{
								if ((1==sscanf(current_token," %d ",&(number_of_points)))&&
									(number_of_points>1))
								{
									return_code=1;
									if (ALLOCATE(trace_coeffs,FE_value,number_of_points*2))
									{
										i=0;
										while (return_code&&(i<number_of_points*2))
										{
											if (!(shift_Parse_state(state,1)&&(state->current_token)&&
												(1==sscanf(state->current_token," %f ",
												&(trace_coeffs[i])))))
											{
												display_message(WARNING_MESSAGE,
													"Missing/invalid trace point value(s)");
												display_parse_state_location(state);
												return_code=0;
											}
											i++;
										}
										if (return_code)
										{
											set_Iso_field_calculation_with_floats(data,type,
												number_of_points*2,trace_coeffs);
											return_code=shift_Parse_state(state,1);
										}
										DEALLOCATE(trace_coeffs);
									}
								}
								else
								{
									display_message(WARNING_MESSAGE,
										"Missing/invalid number of points.");
									display_parse_state_location(state);
									return_code=0;
								}
							} break;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_iso_field_with_floats.  Invalid iso_field_calculation_type");
						return_code=0;
					}

				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_iso_field_with_floats.  Missing iso_field_data structure");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," Value 1# Value 2# Value 3#");
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing value(s) for plane");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_iso_field_with_floats.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_iso_field_with_floats */
#endif /* defined (OLD_CODE) */

static int gfx_create_iso_surfaces(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 1 February 2000

DESCRIPTION :
Executes a GFX CREATE ISO_SURFACES command.
==============================================================================*/
{
	char exterior_flag,*graphics_object_name,*render_type_string,
		*use_element_type_string, **valid_strings;
	enum GT_object_type graphics_object_type;
	float time;
	int face_number,number_of_valid_strings,return_code;
	static char default_name[]="iso_surfaces";
	struct Clipping *clipping;
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field, *data_field, *scalar_field,
		*surface_data_density_field;
	struct Element_discretization discretization;
	struct Element_to_iso_scalar_data element_to_iso_scalar_data;
	struct FE_element *first_element;
	struct FE_field *native_discretization_field;
	struct FE_node *data_to_destroy;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *surface_data_group;
	struct GT_object *graphics_object;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_scalar_field_data,
		set_surface_data_density_field_data;
	struct Spectrum *spectrum;

	ENTER(gfx_create_iso_surfaces);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			if (ALLOCATE(graphics_object_name,char,strlen(default_name)+1))
			{
				strcpy(graphics_object_name,default_name);
			}
			else
			{
				graphics_object_name=(char *)NULL;
			}
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			data_field=(struct Computed_field *)NULL;
			surface_data_group = (struct GROUP(FE_node) *)NULL;
			surface_data_density_field=(struct Computed_field *)NULL;
			time=0;
			element_to_iso_scalar_data.material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			element_group=(struct GROUP(FE_element) *)NULL;
			clipping=(struct Clipping *)NULL;
			scalar_field=(struct Computed_field *)NULL;
			element_to_iso_scalar_data.iso_value=0;
			native_discretization_field=(struct FE_field *)NULL;
			spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
			discretization.number_in_xi1=4;
			discretization.number_in_xi2=4;
			discretization.number_in_xi3=4;
			exterior_flag=0;
			face_number=0;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* clipping */
			Option_table_add_entry(option_table,"clipping",&clipping,
				NULL,set_Clipping);
			/* coordinate */
			set_coordinate_field_data.computed_field_package=
				command_data->computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_package=
				command_data->computed_field_package;
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&data_field,
				&set_data_field_data,set_Computed_field_conditional);
			/* exterior */
			Option_table_add_entry(option_table,"exterior",&exterior_flag,
				NULL,set_char_flag);
			/* face */
			Option_table_add_entry(option_table,"face",&face_number,
				NULL,set_exterior);
			/* from */
			Option_table_add_entry(option_table,"from",&element_group,
				command_data->element_group_manager,set_FE_element_group);
			/* iso_scalar */
			set_scalar_field_data.computed_field_package=
				command_data->computed_field_package;
			set_scalar_field_data.conditional_function=Computed_field_is_scalar;
			set_scalar_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"iso_scalar",&scalar_field,
				&set_scalar_field_data,set_Computed_field_conditional);
			/* iso_value */
			Option_table_add_entry(option_table,"iso_value",
				&(element_to_iso_scalar_data.iso_value),NULL,set_double);
			/* material */
			Option_table_add_entry(option_table,"material",
				&(element_to_iso_scalar_data.material),
				command_data->graphical_material_manager,set_Graphical_material);
			/* native_discretization */
			Option_table_add_entry(option_table,"native_discretization",
				&native_discretization_field,command_data->fe_field_manager,
				set_FE_field);
			/* render_type */
			render_type_string=
				Render_type_string(RENDER_TYPE_SHADED);
			valid_strings=Render_type_get_valid_strings(
				&number_of_valid_strings);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&render_type_string);
			DEALLOCATE(valid_strings);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			/* surface_data_density */
			set_surface_data_density_field_data.computed_field_package=
				command_data->computed_field_package;
			set_surface_data_density_field_data.conditional_function=
				Computed_field_has_up_to_4_numerical_components;
			set_surface_data_density_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"surface_data_density",
				&surface_data_density_field,&set_surface_data_density_field_data,
				set_Computed_field_conditional);
			/* surface_data_group */
			Option_table_add_entry(option_table,"surface_data_group",
				&surface_data_group,command_data->data_group_manager,set_FE_node_group);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* use_elements/use_faces/use_lines */
			use_element_type_string=Use_element_type_string(USE_ELEMENTS);
			valid_strings=
				Use_element_type_get_valid_strings(&number_of_valid_strings);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&use_element_type_string);
			DEALLOCATE(valid_strings);
			/* with */
			Option_table_add_entry(option_table,"with",&discretization,
				command_data->user_interface,set_Element_discretization);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				face_number -= 2;
				if (surface_data_group&&(!surface_data_density_field))
				{
					display_message(ERROR_MESSAGE,"gfx_create_volumes.  Must supply "
						"a surface_data_density_field with a surface_data_group");
					return_code=0;
				}
				if ((!surface_data_group)&&surface_data_density_field)
				{
					display_message(ERROR_MESSAGE,"gfx_create_volumes.  Must supply "
						"a surface_data_group with a surface_data_density_field");
					return_code=0;
				}
				if (!scalar_field)
				{
					display_message(WARNING_MESSAGE,"Missing iso_scalar field");
					return_code=0;
				}
				element_to_iso_scalar_data.use_element_type=
					Use_element_type_from_string(use_element_type_string);
				switch (element_to_iso_scalar_data.use_element_type)
				{
					case USE_ELEMENTS:
					{
						graphics_object_type=g_VOLTEX;
						if (element_group)
						{
							first_element=FIRST_OBJECT_IN_GROUP_THAT(FE_element)(
								FE_element_is_top_level,(void *)NULL,element_group);
						}
						else
						{
							first_element=FIRST_OBJECT_IN_MANAGER_THAT(FE_element)(
								FE_element_is_top_level,(void *)NULL,
								command_data->element_manager);
						}
						if (first_element&&(2==get_FE_element_dimension(first_element)))
						{
							graphics_object_type=g_POLYLINE;
						}
					} break;
					case USE_FACES:
					{
						graphics_object_type=g_POLYLINE;
					} break;
					case USE_LINES:
					{
						display_message(ERROR_MESSAGE,
							".  "
							"USE_LINES not supported for iso_scalar");
						return_code=0;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							".  "
							"Unknown use_element_type");
						return_code=0;
					} break;
				}
				if (!graphics_object_name)
				{
					display_message(WARNING_MESSAGE,"Missing name");
					return_code=0;
				}
				if (return_code)
				{
					if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
						graphics_object_name,command_data->graphics_object_list))
					{
						if (graphics_object_type==graphics_object->object_type)
						{
							if (GT_object_has_time(graphics_object,time))
							{
								display_message(WARNING_MESSAGE,
									"Overwriting time %g in graphics object '%s'",time,
									graphics_object_name);
								return_code=GT_object_delete_time(graphics_object,time);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Object of different type named '%s' already exists",
								graphics_object_name);
							return_code=0;
						}
					}
					else
					{
						if (!((graphics_object=CREATE(GT_object)(graphics_object_name,
							graphics_object_type,element_to_iso_scalar_data.material))&&
							ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
								command_data->graphics_object_list)))
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_iso_surfaces.  Could not create graphics object");
							DESTROY(GT_object)(&graphics_object);
							return_code=0;
						}
					}
				}
				if (return_code && surface_data_group)
				{
					return_code = 1;
					MANAGED_GROUP_BEGIN_CACHE(FE_node)(surface_data_group);

					/* Remove all the current data from the group */
					while(return_code&&(data_to_destroy=FIRST_OBJECT_IN_GROUP_THAT(FE_node)
						((GROUP_CONDITIONAL_FUNCTION(FE_node) *)NULL, NULL, surface_data_group)))
					{
						return_code = REMOVE_OBJECT_FROM_GROUP(FE_node)(
							data_to_destroy, surface_data_group);
						if (FE_node_can_be_destroyed(data_to_destroy))
						{
							return_code = REMOVE_OBJECT_FROM_MANAGER(FE_node)(data_to_destroy,
								command_data->data_manager);
						}
					}
					if(!return_code)
					{
						display_message(ERROR_MESSAGE,"gfx_create_iso_surfaces.  "
							"Unable to remove all data from data_group");
						MANAGED_GROUP_END_CACHE(FE_node)(surface_data_group);
					}
				}
				if (return_code)
				{
					element_to_iso_scalar_data.coordinate_field=
						Computed_field_begin_wrap_coordinate_field(coordinate_field);
					element_to_iso_scalar_data.data_field=data_field;
					element_to_iso_scalar_data.scalar_field=scalar_field;
					element_to_iso_scalar_data.graphics_object=graphics_object;
					element_to_iso_scalar_data.clipping=clipping;
					element_to_iso_scalar_data.surface_data_density_field = 
						surface_data_density_field;
					element_to_iso_scalar_data.surface_data_group = 
						surface_data_group;
					element_to_iso_scalar_data.data_manager = 
						command_data->data_manager;
					element_to_iso_scalar_data.fe_field_manager =
						command_data->fe_field_manager;
					element_to_iso_scalar_data.time=time;
					element_to_iso_scalar_data.number_in_xi[0]=
						discretization.number_in_xi1;
					element_to_iso_scalar_data.number_in_xi[1]=
						discretization.number_in_xi2;
					element_to_iso_scalar_data.number_in_xi[2]=
						discretization.number_in_xi3;
					element_to_iso_scalar_data.render_type=
						Render_type_from_string(render_type_string);
					element_to_iso_scalar_data.element_group=element_group;
					element_to_iso_scalar_data.exterior=exterior_flag;
					element_to_iso_scalar_data.face_number=face_number;
					element_to_iso_scalar_data.native_discretization_field=
						native_discretization_field;
					if (element_group)
					{
						return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
							element_to_iso_scalar,(void *)&element_to_iso_scalar_data,
							element_group);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							element_to_iso_scalar,(void *)&element_to_iso_scalar_data,
							command_data->element_manager);
					}
					if (return_code&&GT_object_has_time(graphics_object,time))
					{
						if (data_field)
						{
							return_code=set_GT_object_Spectrum(
								element_to_iso_scalar_data.graphics_object,spectrum);
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"No iso_surface created");
						if (0==GT_object_get_number_of_times(graphics_object))
						{
							REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
								command_data->graphics_object_list);
						}
					}
					Computed_field_end_wrap(
						&(element_to_iso_scalar_data.coordinate_field));
					if (surface_data_group)
					{
						MANAGED_GROUP_END_CACHE(FE_node)(surface_data_group);
					}
				}
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (data_field)
			{
				DEACCESS(Computed_field)(&data_field);
			}
			if (scalar_field)
			{
				DEACCESS(Computed_field)(&scalar_field);
			}
			if (surface_data_density_field)
			{
				DEACCESS(Computed_field)(&surface_data_density_field);
			}
			if (surface_data_group)
			{
				DEACCESS(GROUP(FE_node))(&surface_data_group);
			}
			if (clipping)
			{
				DESTROY(Clipping)(&clipping);
			}
			if (native_discretization_field)
			{
				DEACCESS(FE_field)(&native_discretization_field);
			}
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
			DEACCESS(Spectrum)(&spectrum);
			DEACCESS(Graphical_material)(
				&(element_to_iso_scalar_data.material));
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_iso_surfaces.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_iso_surfaces.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_iso_surfaces */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_light(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE LIGHT command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Light *light;
	struct Modify_light_data modify_light_data;

	ENTER(gfx_create_light);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(Light,name)(current_token,
						command_data->light_manager))
					{
						if (light=CREATE(Light)(current_token))
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(Light,name)(light,
								command_data->default_light);
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								modify_light_data.default_light=command_data->default_light;
								modify_light_data.light_manager=command_data->light_manager;
								return_code=modify_Light(state,(void *)light,
									(void *)(&modify_light_data));
							}
							else
							{
								return_code=1;
							}
							ADD_OBJECT_TO_MANAGER(Light)(light,command_data->light_manager);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_light.  Could not create light");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Light already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					modify_light_data.default_light=command_data->default_light;
					modify_light_data.light_manager=command_data->light_manager;
					return_code=modify_Light(state,(void *)NULL,
						(void *)(&modify_light_data));
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_light.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing light name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_light.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_light */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_light_model(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE LMODEL command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Light_model *light_model;
	struct Modify_light_model_data modify_light_model_data;

	ENTER(gfx_create_light_model);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(Light_model,name)(current_token,
						command_data->light_model_manager))
					{
						if (light_model=CREATE(Light_model)(current_token))
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(Light_model,name)(light_model,
								command_data->default_light_model);
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								modify_light_model_data.default_light_model=
									command_data->default_light_model;
								modify_light_model_data.light_model_manager=
									command_data->light_model_manager;
								return_code=modify_Light_model(state,(void *)light_model,
									(void *)(&modify_light_model_data));
							}
							else
							{
								return_code=1;
							}
							ADD_OBJECT_TO_MANAGER(Light_model)(light_model,
								command_data->light_model_manager);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_light_model.  Could not create light model");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Light_model already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					modify_light_model_data.default_light_model=
						command_data->default_light_model;
					modify_light_model_data.light_model_manager=
						command_data->light_model_manager;
					return_code=modify_Light_model(state,(void *)NULL,
						(void *)(&modify_light_model_data));
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_light_model.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing light model name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_light_model.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_light_model */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_create_lines(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 January 2000

DESCRIPTION :
Executes a GFX CREATE LINES command.
==============================================================================*/
{
	static char default_name[]="lines";
	char exterior_flag,*graphics_object_name;
	float time;
	struct GT_object *graphics_object;
	int face_number,return_code;
	struct Cmiss_command_data *command_data;
	struct Element_discretization discretization;
	struct Element_to_polyline_data element_to_polyline_data;
	struct Computed_field *coordinate_field,*data_field;
	struct Graphical_material *material;
	struct GROUP(FE_element) *element_group;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data;
	struct Spectrum *spectrum;

	ENTER(gfx_create_lines);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			if (ALLOCATE(graphics_object_name,char,strlen(default_name)+1))
			{
				strcpy(graphics_object_name,default_name);
			}
			else
			{
				graphics_object_name=(char *)NULL;
			}
			element_group=(struct GROUP(FE_element) *)NULL;
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			data_field=(struct Computed_field *)NULL;
			time=0;
			/* must access it now, because we deaccess it later */
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
			discretization.number_in_xi1=4;
			discretization.number_in_xi2=0;
			discretization.number_in_xi3=0;
			exterior_flag=0;
			face_number=0;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* coordinate */
			set_coordinate_field_data.computed_field_package=
				command_data->computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_package=
				command_data->computed_field_package;
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&data_field,
				&set_data_field_data,set_Computed_field_conditional);
			/* exterior */
			Option_table_add_entry(option_table,"exterior",&exterior_flag,
				NULL,set_char_flag);
			/* face */
			Option_table_add_entry(option_table,"face",&face_number,
				NULL,set_exterior);
			/* from */
			Option_table_add_entry(option_table,"from",&element_group,
				command_data->element_group_manager,set_FE_element_group);
			/* material */
			Option_table_add_entry(option_table,"material",&material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* with */
			Option_table_add_entry(option_table,"with",&discretization,
				command_data->user_interface,set_Element_discretization);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors, not asking for help */
			if (return_code)
			{
				face_number -= 2;
				if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list))
				{
					if (g_POLYLINE==graphics_object->object_type)
					{
						if (GT_object_has_time(graphics_object,time))
						{
							display_message(WARNING_MESSAGE,
								"Overwriting time %g in graphics object '%s'",time,
								graphics_object_name);
							return_code=GT_object_delete_time(graphics_object,time);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Object of different type named '%s' already exists",
							graphics_object_name);
						return_code=0;
					}
				}
				else
				{
					if ((graphics_object=CREATE(GT_object)(graphics_object_name,
						g_POLYLINE,material))&&
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_lines.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
				if (return_code)
				{
					element_to_polyline_data.coordinate_field=
						Computed_field_begin_wrap_coordinate_field(coordinate_field);
					element_to_polyline_data.element_group=element_group;
					element_to_polyline_data.exterior=exterior_flag;
					element_to_polyline_data.face_number=face_number;
					element_to_polyline_data.data_field=data_field;
					element_to_polyline_data.graphics_object=graphics_object;
					element_to_polyline_data.time=time;
					element_to_polyline_data.number_of_segments_in_xi1=
						discretization.number_in_xi1;
					if (element_group)
					{
						return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
							element_to_polyline,(void *)&element_to_polyline_data,
							element_group);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							element_to_polyline,(void *)&element_to_polyline_data,
							command_data->element_manager);
					}
					if (return_code&&GT_object_has_time(graphics_object,time))
					{
						if (data_field)
						{
							return_code=set_GT_object_Spectrum(graphics_object,spectrum);
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"No lines created");
						if (0==GT_object_get_number_of_times(graphics_object))
						{
							REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
								command_data->graphics_object_list);
						}
					}
					Computed_field_end_wrap(&(element_to_polyline_data.coordinate_field));
				}
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (data_field)
			{
				DEACCESS(Computed_field)(&data_field);
			}
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
			DEACCESS(Spectrum)(&spectrum);
			DEACCESS(Graphical_material)(&material);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_create_lines.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_lines.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_lines */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_material(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE MATERIAL command.
If the material already exists, then behaves like gfx modify material.
==============================================================================*/
{
	char *current_token;
	int material_is_new,return_code;
	struct Cmiss_command_data *command_data;
	struct Graphical_material *material;
	struct Modify_graphical_material_data modify_graphical_material_data;

	ENTER(gfx_create_material);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					/* if there is an existing material of that name, just modify it */
					if (!(material=FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
						current_token,command_data->graphical_material_manager)))
					{
						if (material=CREATE(Graphical_material)(current_token))
						{
							/*???DB.  Temporary */
							MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name)(material,
								command_data->default_graphical_material);
						}
						material_is_new=1;
					}
					else
					{
						material_is_new=0;
					}
					if (material)
					{
						shift_Parse_state(state,1);
						if (state->current_token)
						{
							/* modify the material with the rest of the parse state */
							modify_graphical_material_data.default_graphical_material=
								command_data->default_graphical_material;
							modify_graphical_material_data.graphical_material_manager=
								command_data->graphical_material_manager;
							modify_graphical_material_data.texture_manager=
								command_data->texture_manager;
							return_code=modify_Graphical_material(state,(void *)material,
								(void *)(&modify_graphical_material_data));
						}
						else
						{
							return_code=1;
						}
						if (material_is_new)
						{
							ADD_OBJECT_TO_MANAGER(Graphical_material)(material,
								command_data->graphical_material_manager);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_material.  Error creating material");
						return_code=0;
					}
				}
				else
				{
					modify_graphical_material_data.default_graphical_material=
						command_data->default_graphical_material;
					modify_graphical_material_data.graphical_material_manager=
						command_data->graphical_material_manager;
					modify_graphical_material_data.texture_manager=
						command_data->texture_manager;
					return_code=modify_Graphical_material(state,(void *)NULL,
						(void *)(&modify_graphical_material_data));
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_material.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing material name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_material.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_material */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_morph(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE MORPH command.  This command interpolates between two
graphics objects, and produces a new object
==============================================================================*/
{
	static char default_name[]="morph";
	char *graphics_object_name;
	float proportion;
	gtObject *graphics_object;
	gtObject *initial,*final;
	int return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"as",NULL,(void *)1,set_name},
		{"final",NULL,NULL,set_Graphics_object},
		{"initial",NULL,NULL,set_Graphics_object},
		{"proportion",NULL,NULL,set_float_0_to_1_inclusive},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_create_morph);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			if (ALLOCATE(graphics_object_name,char,strlen(default_name)+1))
			{
				strcpy(graphics_object_name,default_name);
			}
			else
			{
				graphics_object_name=(char *)NULL;
			}
			proportion=0.0;
			initial=(gtObject *)NULL;
			final=(gtObject *)NULL;
			(option_table[0]).to_be_modified= &graphics_object_name;
			(option_table[1]).to_be_modified= &final;
			(option_table[1]).user_data= (void *)command_data->graphics_object_list;
			(option_table[2]).to_be_modified= &initial;
			(option_table[2]).user_data= (void *)command_data->graphics_object_list;
			(option_table[3]).to_be_modified= &proportion;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				/* check for valid arguments */
				if (initial&&final)
				{
					if (graphics_object=morph_gtObject(graphics_object_name,
						proportion,initial,final))
					{
						if (!ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list))
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_morph.  Could not add graphics object to list");
							DESTROY(GT_object)(&graphics_object);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_morph.  Could not create morph from surface");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Must specify initial and final graphics objects for morph");
					return_code=0;
				}
			} /* parse error, help */
			DEALLOCATE(graphics_object_name);
			if (initial)
			{
				DEACCESS(GT_object)(&initial);
			}
			if (final)
			{
				DEACCESS(GT_object)(&final);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_create_morph.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_morph.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_morph */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if defined (OLD_CODE)
#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_node_group_slider(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE SLIDER command.  If there is a node group slider dialog
in existence, then bring it to the front, otherwise create new one.  If the
fixed node and node group don't have a slider in the slider dialog then add a
new slider.
???DB.  Temporary command ?
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_node_group_slider);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			return_code=create_node_group_slider_dialog(state,
				&(command_data->node_group_slider_dialog),
				command_data->user_interface->application_shell,
				command_data->node_manager,command_data->node_group_manager,
				command_data->execute_command,command_data->user_interface);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_node_group_slider.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_node_group_slider.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_node_group_slider */
#endif /* !defined (WINDOWS_DEV_FLAG) */
#endif /* defined (OLD_CODE) */

static int gfx_create_node_viewer(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 April 2000

DESCRIPTION :
Executes a GFX CREATE NODE_VIEWER command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_node_viewer);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (command_data->node_viewer)
				{
					return_code=Node_viewer_bring_window_to_front(
						command_data->node_viewer);
				}
				else
				{
					if (command_data->node_viewer=CREATE(Node_viewer)(
						&(command_data->node_viewer),"Node Viewer",
						command_data->node_manager,(struct FE_node *)NULL,
						command_data->node_selection,command_data->computed_field_package,
						command_data->user_interface))
					{
						return_code=1;
					}
					else
					{
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_node_viewer.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_node_viewer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_node_viewer */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_data_viewer(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Executes a GFX CREATE DATA_VIEWER command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_data_viewer);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (command_data->data_viewer)
				{
					return_code=Node_viewer_bring_window_to_front(
						command_data->data_viewer);
				}
				else
				{
					if (command_data->data_viewer=CREATE(Node_viewer)(
						&(command_data->data_viewer),"Data Viewer",
						command_data->data_manager,(struct FE_node *)NULL,
						command_data->data_selection,command_data->computed_field_package,
						command_data->user_interface))
					{
						return_code=1;
					}
					else
					{
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_data_viewer.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_data_viewer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_data_viewer */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_create_element_point_viewer(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
Executes a GFX CREATE ELEMENT_POINT_VIEWER command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_element_point_viewer);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (command_data->element_point_viewer)
				{
					return_code=Element_point_viewer_bring_window_to_front(
						command_data->element_point_viewer);
				}
				else
				{
					if (command_data->element_point_viewer=CREATE(Element_point_viewer)(
						&(command_data->element_point_viewer),
						command_data->element_manager,
						command_data->node_manager,
						command_data->element_point_ranges_selection,
						command_data->computed_field_package,
						command_data->fe_field_manager,
						command_data->user_interface))
					{
						return_code=1;
					}
					else
					{
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_element_point_viewer.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_element_point_viewer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_element_point_viewer */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_node_points(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 January 2000

DESCRIPTION :
Executes a GFX CREATE NODE_POINTS command.
==============================================================================*/
{
	static char default_name[]="node_glyphs";
	char *graphics_object_name;
	float time;
	int number_of_components,return_code;
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field,*data_field,*label_field,
		*orientation_scale_field,*rc_coordinate_field,
		*wrapper_orientation_scale_field;
	struct GROUP(FE_node) *node_group;
	struct Graphical_material *material;
	struct GT_glyph_set *glyph_set;
	struct GT_object *glyph,*graphics_object;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Option_table *option_table;
	struct Spectrum *spectrum;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data,set_label_field_data,set_orientation_scale_field_data;
	Triple glyph_centre,glyph_scale_factors,glyph_size;

	ENTER(gfx_create_node_points);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			/* default to point glyph for fastest possible display */
			if (glyph=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("point",
				command_data->glyph_list))
			{
				ACCESS(GT_object)(glyph);
			}
			if (ALLOCATE(graphics_object_name,char,strlen(default_name)+1))
			{
				strcpy(graphics_object_name,default_name);
			}
			else
			{
				graphics_object_name=(char *)NULL;
			}
			node_group=(struct GROUP(FE_node) *)NULL;
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			data_field=(struct Computed_field *)NULL;
			/* must access it now, because we deaccess it later */
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			label_field=(struct Computed_field *)NULL;
			orientation_scale_field=(struct Computed_field *)NULL;
			spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
			time=0.0;
			/* final_size = size + scale_factors*magnitude */
			glyph_scale_factors[0]=1.0;
			glyph_scale_factors[1]=1.0;
			glyph_scale_factors[2]=1.0;
			glyph_size[0]=1.0;
			glyph_size[1]=1.0;
			glyph_size[2]=1.0;
			number_of_components=3;
			glyph_centre[0]=0.0;
			glyph_centre[1]=0.0;
			glyph_centre[2]=0.0;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* centre [of glyph] */
			Option_table_add_entry(option_table,"centre",glyph_centre,
				&(number_of_components),set_float_vector);
			/* coordinate */
			set_coordinate_field_data.computed_field_package=
				command_data->computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_package=
				command_data->computed_field_package;
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&data_field,
				&set_data_field_data,set_Computed_field_conditional);
			/* from [node_group] */
			Option_table_add_entry(option_table,"from",&node_group,
				command_data->node_group_manager,set_FE_node_group);
			/* glyph */
			Option_table_add_entry(option_table,"glyph",&glyph,
				command_data->glyph_list,set_Graphics_object);
			/* label */
			set_label_field_data.computed_field_package=
				command_data->computed_field_package;
			set_label_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_label_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"label",&label_field,
				&set_label_field_data,set_Computed_field_conditional);
			/* material */
			Option_table_add_entry(option_table,"material",&material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* orientation */
			set_orientation_scale_field_data.computed_field_package=
				command_data->computed_field_package;
			set_orientation_scale_field_data.conditional_function=
				Computed_field_is_orientation_scale_capable;
			set_orientation_scale_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"orientation",
				&orientation_scale_field,&set_orientation_scale_field_data,
				set_Computed_field_conditional);
			/* scale_factors */
			Option_table_add_entry(option_table,"scale_factors",
				glyph_scale_factors,"*",set_special_float3);
			/* size [of glyph] */
			Option_table_add_entry(option_table,"size",
				glyph_size,"*",set_special_float3);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",
				&spectrum,command_data->spectrum_manager,set_Spectrum);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list))
				{
					if (g_GLYPH_SET==graphics_object->object_type)
					{
						if (GT_object_has_time(graphics_object,time))
						{
							display_message(WARNING_MESSAGE,
								"Overwriting time %g in graphics object '%s'",time,
								graphics_object_name);
							return_code=GT_object_delete_time(graphics_object,time);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Object of different type named '%s' already exists",
							graphics_object_name);
						return_code=0;
					}
				}
				else
				{
					if ((graphics_object=CREATE(GT_object)(graphics_object_name,
						g_GLYPH_SET,material))&&
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_node_points.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
				if (return_code)
				{
					rc_coordinate_field=
						Computed_field_begin_wrap_coordinate_field(coordinate_field);
					if (orientation_scale_field)
					{
						wrapper_orientation_scale_field=
							Computed_field_begin_wrap_orientation_scale_field(
								orientation_scale_field,rc_coordinate_field);
					}
					else
					{
						wrapper_orientation_scale_field=(struct Computed_field *)NULL;
					}
					if (glyph_set=create_GT_glyph_set_from_FE_node_group(
						node_group,command_data->node_manager,rc_coordinate_field,
						glyph,glyph_centre,glyph_size,wrapper_orientation_scale_field,
						glyph_scale_factors,data_field,label_field,GRAPHICS_NO_SELECT,
						(struct LIST(FE_node) *)NULL))
					{
						if (!GT_OBJECT_ADD(GT_glyph_set)(graphics_object,time,glyph_set))
						{
							DESTROY(GT_glyph_set)(&glyph_set);
						}
					}
					else
					{
						return_code=0;
					}
					if (wrapper_orientation_scale_field)
					{
						Computed_field_end_wrap(&wrapper_orientation_scale_field);
					}
					Computed_field_end_wrap(&rc_coordinate_field);
					if (return_code&&GT_object_has_time(graphics_object,time))
					{
						if (data_field)
						{
							return_code=set_GT_object_Spectrum(graphics_object,spectrum);
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"No node_glyphs created");
						if (0==GT_object_get_number_of_times(graphics_object))
						{
							REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
								command_data->graphics_object_list);
						}
					}
				} /* not duplicate name */
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (data_field)
			{
				DEACCESS(Computed_field)(&data_field);
			}
			if (orientation_scale_field)
			{
				DEACCESS(Computed_field)(&orientation_scale_field);
			}
			DEACCESS(Spectrum)(&spectrum);
			DEACCESS(Graphical_material)(&material);
			DEALLOCATE(graphics_object_name);
			if (glyph)
			{
				DEACCESS(GT_object)(&glyph);
			}
			if (node_group)
			{
				DEACCESS(GROUP(FE_node))(&node_group);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_node_points.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_node_points.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_node_points */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_data_points(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 23 February 2000

DESCRIPTION :
Executes a GFX CREATE DATA_POINTS command.
==============================================================================*/
{
	static char default_name[]="data_glyphs";
	char *graphics_object_name;
	float time;
	int number_of_components,return_code;
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field,*data_field,*label_field,
		*orientation_scale_field,*rc_coordinate_field,
		*wrapper_orientation_scale_field;
	struct GROUP(FE_node) *data_group;
	struct Graphical_material *material;
	struct GT_glyph_set *glyph_set;
	struct GT_object *glyph,*graphics_object;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Option_table *option_table;
	struct Spectrum *spectrum;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data,set_label_field_data,set_orientation_scale_field_data;
	Triple glyph_centre,glyph_scale_factors,glyph_size;

	ENTER(gfx_create_data_points);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			/* default to point glyph for fastest possible display */
			if (glyph=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("point",
				command_data->glyph_list))
			{
				ACCESS(GT_object)(glyph);
			}
			if (ALLOCATE(graphics_object_name,char,strlen(default_name)+1))
			{
				strcpy(graphics_object_name,default_name);
			}
			else
			{
				graphics_object_name=(char *)NULL;
			}
			data_group=(struct GROUP(FE_node) *)NULL;
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			data_field=(struct Computed_field *)NULL;
			/* must access it now, because we deaccess it later */
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			label_field=(struct Computed_field *)NULL;
			orientation_scale_field=(struct Computed_field *)NULL;
			spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
			time=0.0;
			/* final_size = size + scale_factors*magnitude */
			glyph_scale_factors[0]=1.0;
			glyph_scale_factors[1]=1.0;
			glyph_scale_factors[2]=1.0;
			glyph_size[0]=1.0;
			glyph_size[1]=1.0;
			glyph_size[2]=1.0;
			number_of_components=3;
			glyph_centre[0]=0.0;
			glyph_centre[1]=0.0;
			glyph_centre[2]=0.0;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* centre [of glyph] */
			Option_table_add_entry(option_table,"centre",glyph_centre,
				&(number_of_components),set_float_vector);
			/* coordinate */
			set_coordinate_field_data.computed_field_package=
				command_data->computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_package=
				command_data->computed_field_package;
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&data_field,
				&set_data_field_data,set_Computed_field_conditional);
			/* from [data_group] */
			Option_table_add_entry(option_table,"from",&data_group,
				command_data->data_group_manager,set_FE_node_group);
			/* glyph */
			Option_table_add_entry(option_table,"glyph",&glyph,
				command_data->glyph_list,set_Graphics_object);
			/* label */
			set_label_field_data.computed_field_package=
				command_data->computed_field_package;
			set_label_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_label_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"label",&label_field,
				&set_label_field_data,set_Computed_field_conditional);
			/* material */
			Option_table_add_entry(option_table,"material",&material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* orientation */
			set_orientation_scale_field_data.computed_field_package=
				command_data->computed_field_package;
			set_orientation_scale_field_data.conditional_function=
				Computed_field_is_orientation_scale_capable;
			set_orientation_scale_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"orientation",
				&orientation_scale_field,&set_orientation_scale_field_data,
				set_Computed_field_conditional);
			/* scale_factors */
			Option_table_add_entry(option_table,"scale_factors",
				glyph_scale_factors,"*",set_special_float3);
			/* size [of glyph] */
			Option_table_add_entry(option_table,"size",
				glyph_size,"*",set_special_float3);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",
				&spectrum,command_data->spectrum_manager,set_Spectrum);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list))
				{
					if (g_GLYPH_SET==graphics_object->object_type)
					{
						if (GT_object_has_time(graphics_object,time))
						{
							display_message(WARNING_MESSAGE,
								"Overwriting time %g in graphics object '%s'",time,
								graphics_object_name);
							return_code=GT_object_delete_time(graphics_object,time);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Object of different type named '%s' already exists",
							graphics_object_name);
						return_code=0;
					}
				}
				else
				{
					if ((graphics_object=CREATE(GT_object)(graphics_object_name,
						g_GLYPH_SET,material))&&
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_data_points.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
				if (return_code)
				{
					rc_coordinate_field=
						Computed_field_begin_wrap_coordinate_field(coordinate_field);
					if (orientation_scale_field)
					{
						wrapper_orientation_scale_field=
							Computed_field_begin_wrap_orientation_scale_field(
								orientation_scale_field,rc_coordinate_field);
					}
					else
					{
						wrapper_orientation_scale_field=(struct Computed_field *)NULL;
					}
					if (glyph_set=create_GT_glyph_set_from_FE_node_group(
						data_group,command_data->data_manager,rc_coordinate_field,
						glyph,glyph_centre,glyph_size,wrapper_orientation_scale_field,
						glyph_scale_factors,data_field,label_field,GRAPHICS_NO_SELECT,
						(struct LIST(FE_node) *)NULL))
					{
						if (!GT_OBJECT_ADD(GT_glyph_set)(graphics_object,time,glyph_set))
						{
							DESTROY(GT_glyph_set)(&glyph_set);
						}
					}
					else
					{
						return_code=0;
					}
					if (wrapper_orientation_scale_field)
					{
						Computed_field_end_wrap(&wrapper_orientation_scale_field);
					}
					Computed_field_end_wrap(&rc_coordinate_field);
					if (return_code&&GT_object_has_time(graphics_object,time))
					{
						if (data_field)
						{
							return_code=set_GT_object_Spectrum(graphics_object,spectrum);
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"No data_glyphs created");
						if (0==GT_object_get_number_of_times(graphics_object))
						{
							REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
								command_data->graphics_object_list);
						}
					}
				} /* not duplicate name */
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (data_field)
			{
				DEACCESS(Computed_field)(&data_field);
			}
			if (orientation_scale_field)
			{
				DEACCESS(Computed_field)(&orientation_scale_field);
			}
			DEACCESS(Spectrum)(&spectrum);
			DEACCESS(Graphical_material)(&material);
			DEALLOCATE(graphics_object_name);
			if (glyph)
			{
				DEACCESS(GT_object)(&glyph);
			}
			if (data_group)
			{
				DEACCESS(GROUP(FE_node))(&data_group);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_data_points.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_data_points.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_data_points */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_data_group(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE DGROUP command.
==============================================================================*/
{
	char *name;
	int return_code;
	struct Add_FE_node_to_list_if_in_range_data node_in_range_data;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *data_group,*from_data_group,*node_group;
	static struct Modifier_entry option_table[]=
	{
		{"add_ranges",NULL,NULL,set_Multi_range},
		{"from",NULL,NULL,set_FE_node_group},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_create_data_group);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		name=(char *)NULL;
		if (set_name(state,(void *)&name,(void *)1))
		{
			/* initialise defaults */
			from_data_group=(struct GROUP(FE_node) *)NULL;
			node_in_range_data.node_list=CREATE(LIST(FE_node))();
			node_in_range_data.node_ranges=CREATE(Multi_range)();
			(option_table[0]).to_be_modified=node_in_range_data.node_ranges;
			(option_table[1]).to_be_modified= &from_data_group;
			(option_table[1]).user_data=command_data->data_group_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (name&&!FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(name,
					command_data->data_group_manager)&&
					!FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(name,
						command_data->node_group_manager)&&
					!FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)(name,
						command_data->element_group_manager))
				{
					/* create node and element groups of same name simultaneously */
					data_group=CREATE_GROUP(FE_node)(name);
					node_group=CREATE_GROUP(FE_node)(name);
					element_group=CREATE_GROUP(FE_element)(name);
					if (data_group&&node_group&&element_group)
					{
						if (0<Multi_range_get_number_of_ranges(
							node_in_range_data.node_ranges))
						{
							/* make list of data to add to group, then add to group */
							if (from_data_group)
							{
								return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(
									add_FE_node_to_list_if_in_range,(void *)&node_in_range_data,
									from_data_group);
							}
							else
							{
								return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_node)(
									add_FE_node_to_list_if_in_range,(void *)&node_in_range_data,
									command_data->data_manager);
							}
							if (return_code)
							{
								return_code=FOR_EACH_OBJECT_IN_LIST(FE_node)(
									ensure_FE_node_is_in_group,(void *)data_group,
									node_in_range_data.node_list);
							}
						}
						if (return_code)
						{
							/* must add node group before element group so the node group
								 exists when a GT_element_group is made for the element group */
							if (ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(data_group,
								command_data->data_group_manager)&&
								ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(node_group,
									command_data->node_group_manager)&&
								ADD_OBJECT_TO_MANAGER(GROUP(FE_element))(element_group,
									command_data->element_group_manager))
							{
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,"gfx_create_data_group.  "
									"Could not add data/node/element group(s) to manager");
								DESTROY_GROUP(FE_element)(&element_group);
								if (FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
									name,command_data->node_group_manager))
								{
									REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(node_group,
										command_data->node_group_manager);
								}
								else
								{
									DESTROY_GROUP(FE_node)(&node_group);
								}
								if (FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
									name,command_data->data_group_manager))
								{
									REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(data_group,
										command_data->data_group_manager);
								}
								else
								{
									DESTROY_GROUP(FE_node)(&data_group);
								}
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_data_group.  Could not add data ranges to group");
							DESTROY(GROUP(FE_element))(&element_group);
							DESTROY_GROUP(FE_node)(&node_group);
							DESTROY(GROUP(FE_node))(&data_group);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"gfx_create_data_group.  "
							"Could not create data/node/element group(s)");
						DESTROY(GROUP(FE_element))(&element_group);
						DESTROY(GROUP(FE_node))(&node_group);
						DESTROY(GROUP(FE_node))(&data_group);
						return_code=0;
					}
				}
				else
				{
					if (name)
					{
						display_message(ERROR_MESSAGE,"gfx_create_data_group.  "
							"Data/node/element group '%s' already exists",name);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_data_group.  Must specify a name for the group");
					}
					return_code=0;
				}
			}
			DESTROY(Multi_range)(&node_in_range_data.node_ranges);
			DESTROY(LIST(FE_node))(&node_in_range_data.node_list);
			if (from_data_group)
			{
				DEACCESS(GROUP(FE_node))(&from_data_group);
			}
		}
		if (name)
		{
			DEALLOCATE(name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_data_group.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_data_group */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_node_group(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE NGROUP command.
==============================================================================*/
{
	char *name;
	int return_code;
	struct Add_FE_node_to_list_if_in_range_data node_in_range_data;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_node) *data_group,*from_node_group,*node_group;
	struct GROUP(FE_element) *element_group;
	static struct Modifier_entry option_table[]=
	{
		{"add_ranges",NULL,NULL,set_Multi_range},
		{"from",NULL,NULL,set_FE_node_group},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_create_node_group);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		name=(char *)NULL;
		if (set_name(state,(void *)&name,(void *)1))
		{
			/* initialise defaults */
			from_node_group=(struct GROUP(FE_node) *)NULL;
			node_in_range_data.node_list=CREATE(LIST(FE_node))();
			node_in_range_data.node_ranges=CREATE(Multi_range)();
			(option_table[0]).to_be_modified=node_in_range_data.node_ranges;
			(option_table[1]).to_be_modified= &from_node_group;
			(option_table[1]).user_data=command_data->node_group_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (name&&!FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(name,
					command_data->data_group_manager)&&
					!FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(name,
						command_data->node_group_manager)&&
					!FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)(name,
						command_data->element_group_manager))
				{
					/* create node and element groups of same name simultaneously */
					data_group=CREATE_GROUP(FE_node)(name);
					node_group=CREATE_GROUP(FE_node)(name);
					element_group=CREATE_GROUP(FE_element)(name);
					if (data_group&&node_group&&element_group)
					{
						if (0<Multi_range_get_number_of_ranges(
							node_in_range_data.node_ranges))
						{
							/* make list of nodes to add to group, then add to group */
							if (from_node_group)
							{
								return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(
									add_FE_node_to_list_if_in_range,(void *)&node_in_range_data,
									from_node_group);
							}
							else
							{
								return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_node)(
									add_FE_node_to_list_if_in_range,(void *)&node_in_range_data,
									command_data->node_manager);
							}
							if (return_code)
							{
								return_code=FOR_EACH_OBJECT_IN_LIST(FE_node)(
									ensure_FE_node_is_in_group,(void *)node_group,
									node_in_range_data.node_list);
							}
						}
						if (return_code)
						{
							/* must add node group before element group so the node group
								 exists when a GT_element_group is made for the element group */
							if (ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(data_group,
								command_data->data_group_manager)&&
								ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(node_group,
									command_data->node_group_manager)&&
								ADD_OBJECT_TO_MANAGER(GROUP(FE_element))(element_group,
									command_data->element_group_manager))
							{
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,"gfx_create_node_group.  "
									"Could not add data/node/element group(s) to manager");
								DESTROY_GROUP(FE_element)(&element_group);
								if (FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
									name,command_data->node_group_manager))
								{
									REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(node_group,
										command_data->node_group_manager);
								}
								else
								{
									DESTROY_GROUP(FE_node)(&node_group);
								}
								if (FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
									name,command_data->data_group_manager))
								{
									REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(data_group,
										command_data->data_group_manager);
								}
								else
								{
									DESTROY_GROUP(FE_node)(&data_group);
								}
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_node_group.  Could not add node ranges to group");
							DESTROY(GROUP(FE_element))(&element_group);
							DESTROY_GROUP(FE_node)(&node_group);
							DESTROY(GROUP(FE_node))(&data_group);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"gfx_create_node_group.  "
							"Could not create data/node/element group(s)");
						DESTROY(GROUP(FE_element))(&element_group);
						DESTROY(GROUP(FE_node))(&node_group);
						DESTROY(GROUP(FE_node))(&data_group);
						return_code=0;
					}
				}
				else
				{
					if (name)
					{
						display_message(ERROR_MESSAGE,"gfx_create_node_group.  "
							"Data/node/element group '%s' already exists",name);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_node_group.  Must specify a name for the group");
					}
					return_code=0;
				}
			}
			DESTROY(Multi_range)(&node_in_range_data.node_ranges);
			DESTROY(LIST(FE_node))(&node_in_range_data.node_list);
			if (from_node_group)
			{
				DEACCESS(GROUP(FE_node))(&from_node_group);
			}
		}
		if (name)
		{
			DEALLOCATE(name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_node_group.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_node_group */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_scene(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 April 2000

DESCRIPTION :
Executes a GFX CREATE SCENE command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Modify_scene_data modify_scene_data;
	struct Scene *scene;

	ENTER(gfx_create_scene);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				/* set up data for modify_Scene since called in two places */
				modify_scene_data.light_manager=command_data->light_manager;
				modify_scene_data.scene_manager=command_data->scene_manager;
				modify_scene_data.default_scene=command_data->default_scene;
				/* following used for enabling GFEs */
				modify_scene_data.computed_field_package=
					command_data->computed_field_package;
				modify_scene_data.element_manager=command_data->element_manager;
				modify_scene_data.element_group_manager=
					command_data->element_group_manager;
				modify_scene_data.fe_field_manager=
					command_data->fe_field_manager;
				modify_scene_data.node_manager=command_data->node_manager;
				modify_scene_data.node_group_manager=command_data->node_group_manager;
				modify_scene_data.data_manager=command_data->data_manager;
				modify_scene_data.data_group_manager=command_data->data_group_manager;
				modify_scene_data.element_point_ranges_selection=
					command_data->element_point_ranges_selection;
				modify_scene_data.element_selection=command_data->element_selection;
				modify_scene_data.node_selection=command_data->node_selection;
				modify_scene_data.data_selection=command_data->data_selection;
				modify_scene_data.user_interface=command_data->user_interface;
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(Scene,name)(
						current_token,command_data->scene_manager))
					{
						if (scene=CREATE(Scene)(current_token))
						{
							Scene_enable_graphics(scene,command_data->glyph_list,
								command_data->graphical_material_manager,
								command_data->default_graphical_material,
								command_data->light_manager,
								command_data->spectrum_manager,
								command_data->default_spectrum,
								command_data->texture_manager);
							Scene_enable_time_behaviour(scene,
								command_data->default_time_keeper);
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								return_code=modify_Scene(state,(void *)scene,
									(void *)(&modify_scene_data));
							}
							else
							{
								return_code=1;
							}
							ADD_OBJECT_TO_MANAGER(Scene)(scene,
								command_data->scene_manager);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_scene.  Error creating scene");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Scene already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					return_code=
						modify_Scene(state,(void *)NULL,(void *)(&modify_scene_data));
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_scene.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing scene name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_scene.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_scene */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_modify_Spectrum(struct Parse_state *state,void *spectrum_void,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 30 April 1999

DESCRIPTION :
Modifier function that parses all the command line options for creating or
modifying a spectrum.
I would put this with the other gfx modify routines but then it can't be
static and referred to by gfx_create_Spectrum
==============================================================================*/
{
	char autorange, blue_to_red, blue_white_red, clear, *current_token, lg_blue_to_red,
		lg_red_to_blue, overlay_colour, overwrite_colour, red_to_blue;
	int process, range_set, return_code;
	float maximum, minimum;
	struct Cmiss_command_data *command_data;
	struct Modify_spectrum_data modify_spectrum_data;
	struct Option_table *option_table;
	struct Scene *autorange_scene;
	struct Spectrum *spectrum_to_be_modified,*spectrum_to_be_modified_copy;
	struct Spectrum_command_data spectrum_command_data;

	ENTER(gfx_modify_Spectrum);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
				process=0;
				if (spectrum_to_be_modified=(struct Spectrum *)spectrum_void)
				{
					if (IS_MANAGED(Spectrum)(spectrum_to_be_modified,
						command_data->spectrum_manager))
					{
						if (spectrum_to_be_modified_copy=CREATE(Spectrum)(
							"spectrum_modify_temp"))
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)(
								spectrum_to_be_modified_copy,spectrum_to_be_modified);
							process=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Spectrum.  Could not create spectrum copy.");
							return_code=0;
						}
					}
					else
					{
						spectrum_to_be_modified_copy=spectrum_to_be_modified;
						spectrum_to_be_modified=(struct Spectrum *)NULL;
						process=1;
					}
				}
				else
				{
					if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
					{
						if (spectrum_to_be_modified=FIND_BY_IDENTIFIER_IN_MANAGER(
							Spectrum,name)(current_token,
							command_data->spectrum_manager))
						{
							if (return_code=shift_Parse_state(state,1))
							{
								if (spectrum_to_be_modified_copy=CREATE(Spectrum)(
									"spectrum_modify_temp"))
								{
									MANAGER_COPY_WITH_IDENTIFIER(Spectrum,name)(
										spectrum_to_be_modified_copy,spectrum_to_be_modified);
									process=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
									"gfx_modify_Spectrum.  Could not create spectrum copy");
									return_code=0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown spectrum : %s",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						if (spectrum_to_be_modified=CREATE(Spectrum)("dummy"))
						{
							option_table=CREATE(Option_table)();
							Option_table_add_entry(option_table,"SPECTRUM_NAME",
								(void *)spectrum_to_be_modified,command_data_void,
								gfx_modify_Spectrum);
							return_code=Option_table_parse(option_table,state);
							DESTROY(Option_table)(&option_table);
							DESTROY(Spectrum)(&spectrum_to_be_modified);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Spectrum.  Could not create dummy spectrum");
							return_code=0;
						}
					}
				}
				if (process)
				{
					autorange = 0;
					autorange_scene = ACCESS(Scene)(command_data->default_scene);
					blue_to_red = 0;
					clear = 0;
					lg_blue_to_red = 0;
					lg_red_to_blue = 0;
					overlay_colour = 0;
					overwrite_colour = 0;
					red_to_blue = 0;
					blue_white_red = 0;
					modify_spectrum_data.position = 0;
					modify_spectrum_data.settings = (struct Spectrum_settings *)NULL;
					modify_spectrum_data.spectrum_minimum = get_Spectrum_minimum(
						spectrum_to_be_modified_copy);
					modify_spectrum_data.spectrum_maximum = get_Spectrum_maximum(
						spectrum_to_be_modified_copy);
					spectrum_command_data.spectrum_manager 
						= command_data->spectrum_manager;
					option_table=CREATE(Option_table)();
					Option_table_add_entry(option_table,"autorange",&autorange,NULL,
						set_char_flag);					
					Option_table_add_entry(option_table,"blue_to_red",&blue_to_red,NULL,
						set_char_flag);
					Option_table_add_entry(option_table,"blue_white_red",&blue_white_red,NULL,
						set_char_flag);
					Option_table_add_entry(option_table,"clear",&clear,NULL,
						set_char_flag);
					Option_table_add_entry(option_table,"linear",&modify_spectrum_data,
						&spectrum_command_data,gfx_modify_spectrum_settings_linear);
					Option_table_add_entry(option_table,"log",&modify_spectrum_data,
						&spectrum_command_data,gfx_modify_spectrum_settings_log);
					Option_table_add_entry(option_table,"lg_blue_to_red",&lg_blue_to_red,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"lg_red_to_blue",&lg_red_to_blue,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"maximum",&spectrum_to_be_modified_copy,
						NULL,set_Spectrum_maximum_command);
					Option_table_add_entry(option_table,"minimum",&spectrum_to_be_modified_copy,
						NULL,set_Spectrum_minimum_command);
					Option_table_add_entry(option_table,"overlay_colour",&overlay_colour,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"overwrite_colour",&overwrite_colour,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"scene_for_autorange",&autorange_scene,
						command_data->scene_manager,set_Scene);
					Option_table_add_entry(option_table,"red_to_blue",&red_to_blue,
						NULL,set_char_flag);									
					if (return_code=Option_table_multi_parse(option_table,state))
					{
						if (return_code)
						{
							if ( clear )
							{
								Spectrum_remove_all_settings(spectrum_to_be_modified_copy);
							}
							if (blue_to_red + blue_white_red +red_to_blue + lg_red_to_blue + 
								lg_blue_to_red > 1 )
							{
								display_message(ERROR_MESSAGE,
									"gfx_modify_Spectrum.  Specify only one simple spectrum type\n "
									"   (blue_to_red, blue_white_red, red_to_blue, lg_red_to_blue, lg_blue_to_red)");
								return_code=0;
							}
							else if (red_to_blue)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									RED_TO_BLUE_SPECTRUM);
							}
							else if (blue_to_red)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									BLUE_TO_RED_SPECTRUM);
							}
							else if (blue_white_red)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									BLUE_WHITE_RED_SPECTRUM);
							}
							else if (lg_red_to_blue)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									LOG_RED_TO_BLUE_SPECTRUM);
							}
							else if (lg_blue_to_red)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									LOG_BLUE_TO_RED_SPECTRUM);
							}
							if ( modify_spectrum_data.settings )
							{
								/* add new settings */
								return_code=Spectrum_add_settings(spectrum_to_be_modified_copy,
									modify_spectrum_data.settings,
									modify_spectrum_data.position);
							}
							if (overlay_colour && overwrite_colour)
							{
								display_message(ERROR_MESSAGE,
									"gfx_modify_Spectrum.  Specify only one colour mode, overwrite_colour or overlay_colour");
								return_code=0;
							}
							else if (overlay_colour)
							{
								Spectrum_set_opaque_colour_flag(spectrum_to_be_modified_copy,
									0);
							}
							else if (overwrite_colour)
							{
								Spectrum_set_opaque_colour_flag(spectrum_to_be_modified_copy,
									1);
							}
							if (autorange)
							{
								/* Could also do all scenes */
								range_set = 0;
								Scene_get_data_range_for_spectrum(autorange_scene,
									spectrum_to_be_modified
									/* Not spectrum_to_be_modified_copy as this ptr 
										identifies the valid graphics objects */,
									&minimum, &maximum, &range_set);
								if ( range_set )
								{
									Spectrum_set_minimum_and_maximum(spectrum_to_be_modified_copy,
										minimum, maximum );
								}
							}
							if (spectrum_to_be_modified)
							{

								MANAGER_MODIFY_NOT_IDENTIFIER(Spectrum,name)(
									spectrum_to_be_modified,spectrum_to_be_modified_copy,
									command_data->spectrum_manager);
								DESTROY(Spectrum)(&spectrum_to_be_modified_copy);
							}
							else
							{
								spectrum_to_be_modified=spectrum_to_be_modified_copy;
							}
						}
						else
						{
							DESTROY(Spectrum)(&spectrum_to_be_modified_copy);
						}
					}
					if(option_table)
					{
						DESTROY(Option_table)(&option_table);
					}
					if ( modify_spectrum_data.settings )
					{
						DEACCESS(Spectrum_settings)(&(modify_spectrum_data.settings));
					}
					DEACCESS(Scene)(&autorange_scene);
				}
			}
			else
			{
				if (spectrum_void)
				{
					display_message(ERROR_MESSAGE,"Missing spectrum modifications");
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing spectrum name");
				}
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_Spectrum.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
				"gfx_modify_Spectrum.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_Spectrum */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE SPECTRUM command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Spectrum *spectrum;

	ENTER(gfx_create_spectrum);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(Spectrum,name)(
						current_token,command_data->spectrum_manager))
					{
						if (spectrum=CREATE(Spectrum)(current_token))
						{
							/*???DB.  Temporary */
							MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)(spectrum,
								command_data->default_spectrum);
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								return_code=gfx_modify_Spectrum(state,(void *)spectrum,
									command_data_void);
							}
							else
							{
								return_code=1;
							}
							if (return_code)
							{
								ADD_OBJECT_TO_MANAGER(Spectrum)(spectrum,
									command_data->spectrum_manager);
							}
							else
								DESTROY(Spectrum)(&spectrum);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_spectrum.  Error creating spectrum");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Spectrum already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					return_code=gfx_modify_Spectrum(state,(void *)NULL,command_data_void);
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_spectrum.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing spectrum name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_spectrum.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_spectrum */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_streamlines(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 January 2000

DESCRIPTION :
Executes a GFX CREATE STREAMLINES command.
==============================================================================*/
{
	static char default_name[]="streamlines";
	char *graphics_object_name,reverse_track,*streamline_data_type_string,
		*streamline_type_string,**valid_strings;
	enum Streamline_type streamline_type;
	enum Streamline_data_type streamline_data_type;
	FE_value seed_xi[3];
	float length,time,width;
	int number_of_components,number_of_valid_strings,return_code;
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field,*data_field,*stream_vector_field;
	struct Element_to_streamline_data element_to_streamline_data;
	struct FE_element *seed_element;
	struct FE_field *seed_data_field;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *seed_data_group;
	struct Graphical_material *material;
	struct GT_object *graphics_object;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Node_to_streamline_data node_to_streamline_data;
	struct Spectrum *spectrum;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data,set_stream_vector_field_data;
	struct Set_FE_field_conditional_data set_seed_data_field_data;

	ENTER(gfx_create_streamlines);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			if (ALLOCATE(graphics_object_name,char,strlen(default_name)+1))
			{
				strcpy(graphics_object_name,default_name);
			}
			else
			{
				graphics_object_name=(char *)NULL;
			}
			element_group=(struct GROUP(FE_element) *)NULL;
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			if (stream_vector_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_is_stream_vector_capable,(void *)NULL,
				computed_field_manager))
			{
				ACCESS(Computed_field)(stream_vector_field);
			}
			data_field=(struct Computed_field *)NULL;
			seed_element=(struct FE_element *)NULL;
			seed_data_group = (struct GROUP(FE_node) *)NULL;
			seed_data_field = (struct FE_field *)NULL;
			time=0;
			length=1;
			width = 1;
			reverse_track = 0;
			number_of_components = 3;
			seed_xi[0] = 0.5;
			seed_xi[1] = 0.5;
			seed_xi[2] = 0.5;
			/* must access it now, because we deaccess it later */
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			spectrum=
				ACCESS(Spectrum)(command_data->default_spectrum);

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* coordinate */
			set_coordinate_field_data.computed_field_package=
				command_data->computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_package=
				command_data->computed_field_package;
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&data_field,
				&set_data_field_data,set_Computed_field_conditional);
			/* ellipse/line/rectangle/ribbon */
			streamline_type_string=Streamline_type_string(STREAM_LINE);
			valid_strings=Streamline_type_get_valid_strings(&number_of_valid_strings);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&streamline_type_string);
			DEALLOCATE(valid_strings);
			/* from */
			Option_table_add_entry(option_table,"from",&element_group,
				command_data->element_group_manager,set_FE_element_group);
			/* length */
			Option_table_add_entry(option_table,"length",&length,NULL,set_float);
			/* material */
			Option_table_add_entry(option_table,"material",&material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* no_data/field_scalar/magnitude_scalar/travel_scalar */
			streamline_data_type_string=Streamline_data_type_string(STREAM_NO_DATA);
			valid_strings=
				Streamline_data_type_get_valid_strings(&number_of_valid_strings);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&streamline_data_type_string);
			DEALLOCATE(valid_strings);
			/* reverse */
			/*???RC use negative length to denote reverse track instead? */
			Option_table_add_entry(option_table,"reverse",
				&reverse_track,NULL,set_char_flag);
			/* seed_data_field */
			set_seed_data_field_data.fe_field_manager=command_data->fe_field_manager;
			set_seed_data_field_data.conditional_function=FE_field_has_value_type;
			set_seed_data_field_data.conditional_function_user_data=
				(void *)ELEMENT_XI_VALUE;
			Option_table_add_entry(option_table,"seed_data_field",&seed_data_field,
				&set_seed_data_field_data,set_Computed_field_conditional);
			/* seed_data_group */
			Option_table_add_entry(option_table,"seed_data_group",&seed_data_group,
				command_data->data_group_manager,set_FE_node_group);
			/* seed_element */
			Option_table_add_entry(option_table,"seed_element",
				&seed_element,command_data->element_manager,
				set_FE_element_dimension_3);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* vector */
			set_stream_vector_field_data.computed_field_package=
				command_data->computed_field_package;
			set_stream_vector_field_data.conditional_function=
				Computed_field_is_stream_vector_capable;
			set_stream_vector_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"vector",&stream_vector_field,
				&set_stream_vector_field_data,set_Computed_field_conditional);
			/* width */
			Option_table_add_entry(option_table,"width",&width,NULL,set_float);
			/* xi */
			Option_table_add_entry(option_table,"xi",
				seed_xi,&number_of_components,set_float_vector);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				if (element_group&&seed_element&&(!IS_OBJECT_IN_GROUP(FE_element)(
					seed_element,element_group)))
				{
					display_message(ERROR_MESSAGE,
						"seed_element is not in element_group");
					return_code=0;
				}
				if (!stream_vector_field)
				{
					display_message(ERROR_MESSAGE,"Must specify a vector");
					return_code=0;
				}
				streamline_type=Streamline_type_from_string(streamline_type_string);
				streamline_data_type=
					Streamline_data_type_from_string(streamline_data_type_string);
				if (data_field)
				{
					if (STREAM_FIELD_SCALAR != streamline_data_type)
					{
						display_message(WARNING_MESSAGE,
							"Must use field_scalar option with data; ensuring this");
						streamline_data_type=STREAM_FIELD_SCALAR;
					}
				}
				else
				{
					if (STREAM_FIELD_SCALAR == streamline_data_type)
					{
						display_message(WARNING_MESSAGE,
							"Must specify data field with field_scalar option");
						streamline_data_type=STREAM_NO_DATA;
					}
				}
				if (seed_data_field&&(!seed_data_group))
				{
					display_message(ERROR_MESSAGE,
						"If you specify a seed_data_field then you must also specity a seed_data_group");
					return_code=0;					
				}
				if ((!seed_data_field)&&seed_data_group)
				{
					display_message(ERROR_MESSAGE,
						"If you specify a seed_data_group then you must also specity a seed_data_field");
					return_code=0;
				}
				if (return_code)
				{
					if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
						graphics_object_name,command_data->graphics_object_list))
					{
						if ((g_POLYLINE == graphics_object->object_type &&
							(STREAM_LINE == streamline_type)) ||
							(g_SURFACE == graphics_object->object_type &&
							(STREAM_LINE != streamline_type)))
						{
							if (GT_object_has_time(graphics_object,time))
							{
								display_message(WARNING_MESSAGE,
									"Overwriting time %g in graphics object '%s'",time,
									graphics_object_name);
								return_code=GT_object_delete_time(graphics_object,time);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Object of different type named '%s' already exists",
								graphics_object_name);
							return_code=0;
						}
					}
					else
					{
						if (STREAM_LINE==streamline_type)
						{
							graphics_object=CREATE(GT_object)(graphics_object_name,
								g_POLYLINE,material);
						}
						else
						{
							graphics_object=CREATE(GT_object)(graphics_object_name,
								g_SURFACE,material);
						}
						if (graphics_object&&ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list))
						{
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_streamlines.  Could not create graphics object");
							DESTROY(GT_object)(&graphics_object);
							return_code=0;
						}
					}
					if (return_code)
					{
						if (seed_data_group)
						{
							node_to_streamline_data.coordinate_field=
								Computed_field_begin_wrap_coordinate_field(coordinate_field);
							node_to_streamline_data.stream_vector_field=
								Computed_field_begin_wrap_orientation_scale_field(
									stream_vector_field,
									node_to_streamline_data.coordinate_field);
							node_to_streamline_data.type = streamline_type;
							node_to_streamline_data.data_type = streamline_data_type;
							node_to_streamline_data.data_field = data_field;
							node_to_streamline_data.element_group = element_group;
							node_to_streamline_data.graphics_object = graphics_object;
							node_to_streamline_data.length = length;
							node_to_streamline_data.width = width;
							node_to_streamline_data.time = time;
							/* reverse_track = track -vector and store -travel_scalar */
							node_to_streamline_data.reverse_track=(int)reverse_track;
							node_to_streamline_data.seed_data_field=seed_data_field;
							node_to_streamline_data.seed_element = seed_element;
							return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(
								node_to_streamline,
								(void *)&node_to_streamline_data,seed_data_group);
							Computed_field_end_wrap(
								&(node_to_streamline_data.stream_vector_field));
							Computed_field_end_wrap(
								&(node_to_streamline_data.coordinate_field));
						}
						else
						{
							element_to_streamline_data.coordinate_field=
								Computed_field_begin_wrap_coordinate_field(coordinate_field);
							element_to_streamline_data.stream_vector_field=
								Computed_field_begin_wrap_orientation_scale_field(
									stream_vector_field,
									element_to_streamline_data.coordinate_field);
							element_to_streamline_data.type = streamline_type;
							element_to_streamline_data.data_type = streamline_data_type;
							element_to_streamline_data.data_field = data_field;
							element_to_streamline_data.graphics_object = graphics_object;
							element_to_streamline_data.length = length;
							element_to_streamline_data.width = width;
							element_to_streamline_data.time = time;
							/* reverse_track = track -vector and store -travel_scalar */
							element_to_streamline_data.reverse_track=(int)reverse_track;
							element_to_streamline_data.seed_xi[0]=seed_xi[0];
							element_to_streamline_data.seed_xi[1]=seed_xi[1];
							element_to_streamline_data.seed_xi[2]=seed_xi[2];
							if (seed_element)
							{
								element_to_streamline(seed_element,
									(void *)&element_to_streamline_data);
							}
							else
							{
								if (element_group)
								{
									return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
										element_to_streamline,
										(void *)&element_to_streamline_data,element_group);
								}
								else
								{
									return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
										element_to_streamline,
										(void *)&element_to_streamline_data,
										command_data->element_manager);
								}
							}
							Computed_field_end_wrap(
								&(element_to_streamline_data.stream_vector_field));
							Computed_field_end_wrap(
								&(element_to_streamline_data.coordinate_field));
						}
						if (return_code&&GT_object_has_time(graphics_object,time))
						{
							if (STREAM_NO_DATA != streamline_data_type)
							{
								return_code=set_GT_object_Spectrum(graphics_object,spectrum);
							}
						}
						else
						{
							display_message(WARNING_MESSAGE,"No streamlines created");
							if (0==GT_object_get_number_of_times(graphics_object))
							{
								REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
									command_data->graphics_object_list);
							}
						}
					}
				}
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (stream_vector_field)
			{
				DEACCESS(Computed_field)(&stream_vector_field);
			}
			if (data_field)
			{
				DEACCESS(Computed_field)(&data_field);
			}
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
			if (seed_data_field)
			{
				DEACCESS(FE_field)(&seed_data_field);
			}
			if (seed_data_group)
			{
				DEACCESS(GROUP(FE_node))(&seed_data_group);
			}
			if (seed_element)
			{
				DEACCESS(FE_element)(&seed_element);
			}
			DEACCESS(Graphical_material)(&material);
			DEACCESS(Spectrum)(&spectrum);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_streamlines.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_streamlines.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_streamlines */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_interactive_streamline(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 January 2000

DESCRIPTION :
Executes a GFX CREATE INTERACTIVE_STREAMLINE command.
==============================================================================*/
{
	static char default_name[]="interactive_streamline";
	char *graphics_object_name,*line_graphics_object_name,reverse_track,
		*streamline_data_type_string,*streamline_type_string,**valid_strings;
	enum Streamline_type streamline_type;
	enum Streamline_data_type streamline_data_type;
	FE_value seed_xi[3];
	float length,width;
	int number_of_components,number_of_valid_strings,return_code;
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field,*data_field,*rc_coordinate_field,
		*stream_vector_field,*wrapper_stream_vector_field;
	struct FE_element *seed_element;
	struct GROUP(FE_element) *element_group;
	struct Graphical_material *material;
	struct GT_object *graphics_object,*line_graphics_object;
	struct GT_pointset *point_set;
	struct GT_polyline *polyline;
	struct GT_surface *surface;
	struct Interactive_streamline *streamline;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Spectrum *spectrum;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data,set_stream_vector_field_data;

	ENTER(gfx_create_interactive_streamline);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			if (ALLOCATE(graphics_object_name,char,strlen(default_name)+1))
			{
				strcpy(graphics_object_name,default_name);
			}
			else
			{
				graphics_object_name=(char *)NULL;
			}
			element_group=(struct GROUP(FE_element) *)NULL;
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			if (stream_vector_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_is_stream_vector_capable,(void *)NULL,
				computed_field_manager))
			{
				ACCESS(Computed_field)(stream_vector_field);
			}
			data_field=(struct Computed_field *)NULL;
			seed_element=(struct FE_element *)NULL;
			length=1;
			width = 1;
			reverse_track = 0;
			number_of_components = 3;
			seed_xi[0] = 0.5;
			seed_xi[1] = 0.5;
			seed_xi[2] = 0.5;
			graphics_object=(struct GT_object *)NULL;
			line_graphics_object=(struct GT_object *)NULL;
			/* must access it now, because we deaccess it later */
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			spectrum=
				ACCESS(Spectrum)(command_data->default_spectrum);

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* coordinate */
			set_coordinate_field_data.computed_field_package=
				command_data->computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_package=
				command_data->computed_field_package;
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&data_field,
				&set_data_field_data,set_Computed_field_conditional);
			/* ellipse/line/rectangle/ribbon */
			streamline_type_string=Streamline_type_string(STREAM_LINE);
			valid_strings=Streamline_type_get_valid_strings(&number_of_valid_strings);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&streamline_type_string);
			DEALLOCATE(valid_strings);
			/* from */
			Option_table_add_entry(option_table,"from",&element_group,
				command_data->element_group_manager,set_FE_element_group);
			/* initial_xi */
			Option_table_add_entry(option_table,"initial_xi",
				seed_xi,&number_of_components,set_float_vector);
			/* length */
			Option_table_add_entry(option_table,"length",&length,NULL,set_float);
			/* material */
			Option_table_add_entry(option_table,"material",&material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* no_data/field_scalar/magnitude_scalar/travel_scalar */
			streamline_data_type_string=Streamline_data_type_string(STREAM_NO_DATA);
			valid_strings=
				Streamline_data_type_get_valid_strings(&number_of_valid_strings);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&streamline_data_type_string);
			DEALLOCATE(valid_strings);
			/* reverse */
			/*???RC use negative length to denote reverse track instead? */
			Option_table_add_entry(option_table,"reverse",
				&reverse_track,NULL,set_char_flag);
			/* seed_element */
			Option_table_add_entry(option_table,"seed_element",
				&seed_element,command_data->element_manager,
				set_FE_element_dimension_3);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			/* vector */
			set_stream_vector_field_data.computed_field_package=
				command_data->computed_field_package;
			set_stream_vector_field_data.conditional_function=
				Computed_field_is_stream_vector_capable;
			set_stream_vector_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"vector",&stream_vector_field,
				&set_stream_vector_field_data,set_Computed_field_conditional);
			/* width */
			Option_table_add_entry(option_table,"width",&width,NULL,set_float);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				if (seed_element)
				{
					if (element_group&&(!IS_OBJECT_IN_GROUP(FE_element)(
						seed_element,element_group)))
					{
						display_message(ERROR_MESSAGE,
							"seed_element is not in element_group");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Must specify a seed_element");
					return_code=0;
				}
				if (!stream_vector_field)
				{
					display_message(ERROR_MESSAGE,"Must specify a vector");
					return_code=0;
				}
				streamline_type=Streamline_type_from_string(streamline_type_string);
				streamline_data_type=
					Streamline_data_type_from_string(streamline_data_type_string);
				if (data_field)
				{
					if (STREAM_FIELD_SCALAR != streamline_data_type)
					{
						display_message(WARNING_MESSAGE,
							"Must use field_scalar option with data; ensuring this");
						streamline_data_type=STREAM_FIELD_SCALAR;
					}
				}
				else
				{
					if (STREAM_FIELD_SCALAR == streamline_data_type)
					{
						display_message(WARNING_MESSAGE,
							"Must specify data field with field_scalar option");
						streamline_data_type=STREAM_NO_DATA;
					}
				}
				if (return_code)
				{
					if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
						graphics_object_name,command_data->graphics_object_list))
					{
						display_message(ERROR_MESSAGE,
							"Object of different type named '%s' already exists",
							graphics_object_name);
						return_code=0;
					}
					else
					{
						if ((graphics_object=CREATE(GT_object)(graphics_object_name,
							g_POINTSET,material)))
						{
							if (ALLOCATE(line_graphics_object_name,char,
								strlen(graphics_object_name)+15))
							{
								strcpy(line_graphics_object_name,graphics_object_name);
								strcat(line_graphics_object_name,"_streamline");
								if (STREAM_LINE==streamline_type)
								{
									line_graphics_object=CREATE(GT_object)(
										line_graphics_object_name,g_POLYLINE,material);
								}
								else
								{
									line_graphics_object=CREATE(GT_object)(
										line_graphics_object_name,g_SURFACE,material);
								}
								if (line_graphics_object)
								{
									graphics_object->nextobject=
										ACCESS(GT_object)(line_graphics_object);
									return_code=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,"gfx_create_streamlines.  "
										"Could not create line_graphics_object");
									DESTROY(GT_object)(&line_graphics_object);
									DESTROY(GT_object)(&graphics_object);
									return_code=0;
								}
								DEALLOCATE(line_graphics_object_name);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_create_interactive_streamline.  "
									"Could not allocate memory for line_graphics_object_name");
								DESTROY(GT_object)(&graphics_object);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_interactive_streamline.  "
								"Could not create main graphics object");
							DESTROY(GT_object)(&graphics_object);
							return_code=0;
						}
					}
					if (return_code)
					{
						if (point_set=create_interactive_streampoint(seed_element,
							coordinate_field,length,seed_xi))
						{
							GT_OBJECT_ADD(GT_pointset)(graphics_object,/*time*/0.0,point_set);
							rc_coordinate_field=
								Computed_field_begin_wrap_coordinate_field(coordinate_field);
							wrapper_stream_vector_field=
								Computed_field_begin_wrap_orientation_scale_field(
									stream_vector_field,rc_coordinate_field);
							/* build the streamline */
							if (STREAM_LINE==streamline_type)
							{
								if (polyline=create_GT_polyline_streamline_FE_element(
									seed_element,seed_xi,rc_coordinate_field,
									wrapper_stream_vector_field,reverse_track,
									length,streamline_data_type,data_field))
								{
									if (!(return_code=GT_OBJECT_ADD(GT_polyline)(
										line_graphics_object,/*time*/0.0,polyline)))
									{
										DESTROY(GT_polyline)(&polyline);
									}
								}
								else
								{
									return_code=0;
								}
							}
							else if ((streamline_type == STREAM_RIBBON)||
								(streamline_type == STREAM_EXTRUDED_RECTANGLE)||
								(streamline_type == STREAM_EXTRUDED_ELLIPSE))
							{
								if (surface=create_GT_surface_streamribbon_FE_element(
									seed_element,seed_xi,rc_coordinate_field,
									wrapper_stream_vector_field,reverse_track,
									length,width,streamline_type,streamline_data_type,data_field))
								{
									if (!(return_code=GT_OBJECT_ADD(GT_surface)(
										line_graphics_object,/*time*/0.0,surface)))
									{
										DESTROY(GT_surface)(&surface);
									}
								}
								else
								{
									return_code=0;
								}
							}
							if (return_code)
							{
								if (STREAM_NO_DATA != streamline_data_type)
								{
									return_code=set_GT_object_Spectrum(graphics_object,spectrum)&&
										set_GT_object_Spectrum(line_graphics_object,spectrum);
								}
								streamline=CREATE(Interactive_streamline)(
									graphics_object_name,streamline_type,seed_element,seed_xi,
									rc_coordinate_field,wrapper_stream_vector_field,reverse_track,
									length,width,streamline_data_type,data_field,graphics_object,
									line_graphics_object);
								ADD_OBJECT_TO_MANAGER(Interactive_streamline)(streamline,
									command_data->interactive_streamline_manager);
								/* bring up interactive_streamline dialog */
								bring_up_interactive_streamline_dialog(
									&(command_data->interactive_streamlines_dialog),
									command_data->user_interface->application_shell,
									command_data->interactive_streamline_manager,
									streamline,command_data->user_interface,
									command_data->scene_manager);
							}
							Computed_field_end_wrap(&wrapper_stream_vector_field);
							Computed_field_end_wrap(&rc_coordinate_field);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_interactive_streamline.  "
								"Unable to create streampoint");
							return_code=0;
						}
						if (!return_code)
						{
							display_message(WARNING_MESSAGE,"No streamline created");
							if (0==GT_object_get_number_of_times(graphics_object))
							{
								REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
									command_data->graphics_object_list);
							}
						}
					}
				}
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (stream_vector_field)
			{
				DEACCESS(Computed_field)(&stream_vector_field);
			}
			if (data_field)
			{
				DEACCESS(Computed_field)(&data_field);
			}
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
			if (seed_element)
			{
				DEACCESS(FE_element)(&seed_element);
			}
			DEACCESS(Spectrum)(&spectrum);
			DEACCESS(Graphical_material)(&material);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_interactive_streamline.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_interactive_streamline.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_interactive_streamline */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_surfaces(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 January 2000

DESCRIPTION :
Executes a GFX CREATE SURFACES command.
==============================================================================*/
{
	char exterior_flag,*graphics_object_name,nurb,*render_type_string,
		reverse_normals,**valid_strings;
	enum GT_object_type object_type;
	float time;
	gtObject *graphics_object;
	int face_number,number_of_valid_strings,return_code;
	static char default_name[]="surfaces";
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field,*data_field,*texture_coordinate_field;
	struct Element_discretization discretization;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Element_to_surface_data element_to_surface_data;
	struct Graphical_material *material;
	struct GROUP(FE_element) *element_group;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_texture_coordinate_field_data;
	struct Spectrum *spectrum;

	ENTER(gfx_create_surfaces);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			if (ALLOCATE(graphics_object_name,char,strlen(default_name)+1))
			{
				strcpy(graphics_object_name,default_name);
			}
			else
			{
				graphics_object_name=(char *)NULL;
			}
			element_group=(struct GROUP(FE_element) *)NULL;
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			data_field=(struct Computed_field *)NULL;
			texture_coordinate_field=(struct Computed_field *)NULL;
			time=0;
			/* must access it now, because we deaccess it later */
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
			discretization.number_in_xi1=4;
			discretization.number_in_xi2=4;
			discretization.number_in_xi3=0;
			exterior_flag=0;
			face_number=0;
			nurb=0;
			reverse_normals=0;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* coordinate */
			set_coordinate_field_data.computed_field_package=
				command_data->computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_package=
				command_data->computed_field_package;
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&data_field,
				&set_data_field_data,set_Computed_field_conditional);
			/* exterior */
			Option_table_add_entry(option_table,"exterior",&exterior_flag,
				NULL,set_char_flag);
			/* face */
			Option_table_add_entry(option_table,"face",&face_number,
				NULL,set_exterior);
			/* from */
			Option_table_add_entry(option_table,"from",&element_group,
				command_data->element_group_manager,set_FE_element_group);
			/* material */
			Option_table_add_entry(option_table,"material",&material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* nurb */
			Option_table_add_entry(option_table,"nurb",&nurb,NULL,set_char_flag);
			/* reverse_normals */
			Option_table_add_entry(option_table,"reverse_normals",
				&reverse_normals,NULL,set_char_flag);
			/* render_type */
			render_type_string=
				Render_type_string(RENDER_TYPE_SHADED);
			valid_strings=Render_type_get_valid_strings(
				&number_of_valid_strings);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&render_type_string);
			DEALLOCATE(valid_strings);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			/* texture_coordinates */
			set_texture_coordinate_field_data.computed_field_package=
				command_data->computed_field_package;
			set_texture_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_texture_coordinate_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"texture_coordinates",
				&texture_coordinate_field,&set_texture_coordinate_field_data,
				set_Computed_field_conditional);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* with */
			Option_table_add_entry(option_table,"with",&discretization,
				command_data->user_interface,set_Element_discretization);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (nurb)
				{
					object_type = g_NURBS;
				}
				else
				{
					object_type = g_SURFACE;
				}
				face_number -= 2;
				if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list))
				{
					if (object_type==graphics_object->object_type)
					{
						if (GT_object_has_time(graphics_object,time))
						{
							display_message(WARNING_MESSAGE,
								"Overwriting time %g in graphics object '%s'",time,
								graphics_object_name);
							return_code=GT_object_delete_time(graphics_object,time);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Object of different type named '%s' already exists",
							graphics_object_name);
						return_code=0;
					}
				}
				else
				{
					if ((graphics_object=CREATE(GT_object)(graphics_object_name,
						object_type,material))&&
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_surfaces.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
			}
			if (return_code)
			{
				element_to_surface_data.coordinate_field=
					Computed_field_begin_wrap_coordinate_field(coordinate_field);
				element_to_surface_data.element_group=element_group;
				element_to_surface_data.exterior=exterior_flag;
				element_to_surface_data.face_number=face_number;
				element_to_surface_data.object_type = object_type;
				element_to_surface_data.data_field=data_field;
				element_to_surface_data.reverse_normals=reverse_normals;
				element_to_surface_data.graphics_object=graphics_object;
				element_to_surface_data.render_type=
					Render_type_from_string(render_type_string);
				element_to_surface_data.texture_coordinate_field=
					texture_coordinate_field;
				element_to_surface_data.time=time;
				element_to_surface_data.number_of_segments_in_xi1=
					discretization.number_in_xi1;
				element_to_surface_data.number_of_segments_in_xi2=
					discretization.number_in_xi2;
				if (element_group)
				{
					return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
						element_to_surface,(void *)&element_to_surface_data,
						element_group);
				}
				else
				{
					return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
						element_to_surface,(void *)&element_to_surface_data,
						command_data->element_manager);
				}
				if (return_code&&GT_object_has_time(graphics_object,time))
				{
					if (data_field)
					{
						return_code=set_GT_object_Spectrum(graphics_object,spectrum);
					}
				}
				else
				{
					if (return_code)
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_surfaces.  No surfaces created");
						return_code=0;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_surfaces.  Error creating surfaces");
					}
					if (0==GT_object_get_number_of_times(graphics_object))
					{
						REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list);
					}
				}
				Computed_field_end_wrap(&(element_to_surface_data.coordinate_field));
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (data_field)
			{
				DEACCESS(Computed_field)(&data_field);
			}
			if (texture_coordinate_field)
			{
				DEACCESS(Computed_field)(&texture_coordinate_field);
			}
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
			DEACCESS(Graphical_material)(&material);
			DEACCESS(Spectrum)(&spectrum);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_surfaces.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_surfaces.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_surfaces */
#endif /* !defined (WINDOWS_DEV_FLAG) */

int gfx_modify_Texture(struct Parse_state *state,void *texture_void,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Modifies the properties of a texture.
==============================================================================*/
{
	auto struct Modifier_entry
		help_option_table[]=
		{
			{"TEXTURE_NAME",NULL,NULL,gfx_modify_Texture},
			{NULL,NULL,NULL,NULL}
		},
		combine_option_table[]=
		{
			{"blend",NULL,NULL,set_Texture_combine_blend},
			{"decal",NULL,NULL,set_Texture_combine_decal},
			{"modulate",NULL,NULL,set_Texture_combine_modulate},
			{NULL,NULL,NULL,NULL}
		},
		filter_option_table[]=
		{
			{"linear_filter",NULL,NULL,set_Texture_filter_linear},
			{"nearest_filter",NULL,NULL,set_Texture_filter_nearest},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{"alpha",NULL,NULL,set_float_0_to_1_inclusive},
			{NULL,NULL,NULL,NULL},
			{NULL,NULL,NULL,NULL},
			{"colour",NULL,NULL,set_Colour},
			{"distortion",NULL,NULL,set_double_vector_with_help},
			{"height",NULL,NULL,set_float_positive},
			{"image",NULL,NULL,set_Texture_image},
			{NULL,NULL,NULL,NULL},
#if defined (SGI_MOVIE_FILE)
			{"movie",NULL,NULL,set_Movie_graphics},
#endif /* defined (SGI_MOVIE_FILE) */
			{"source_field",NULL,NULL,set_Computed_field_conditional},
			{"source_height_texels",NULL,NULL,set_int_positive},
			{"source_spectrum",NULL,NULL,set_Spectrum},
			{"source_width_texels",NULL,NULL,set_int_positive},
			{"source_texture_coordinates",NULL,NULL,set_Computed_field_conditional},
			{"width",NULL,NULL,set_float_positive},
			{NULL,NULL,NULL,NULL}
		},
		wrap_option_table[]=
		{
			{"clamp_wrap",NULL,NULL,set_Texture_wrap_clamp},
			{"repeat_wrap",NULL,NULL,set_Texture_wrap_repeat},
			{NULL,NULL,NULL,NULL}
		};
	char *current_token;
	double texture_distortion[3];
	float alpha,distortion_centre_x,distortion_centre_y,distortion_factor_k1,
		height,width;
	int i, process,return_code, source_height, source_width;
	struct Cmiss_command_data *command_data;
	struct Colour colour;
	struct Computed_field *field, *texture_coordinates_field;
#if defined (SGI_MOVIE_FILE)
	struct Movie_graphics *movie,*old_movie;
#endif /* defined (SGI_MOVIE_FILE) */
	struct Set_Computed_field_conditional_data set_field_data,
		set_texture_coordinates_field_data;
	struct Spectrum *spectrum;
	struct Texture *texture_to_be_modified,*texture_to_be_modified_copy;
	/* do not make the following static as 'set' flag must start at 0 */
	struct Set_vector_with_help_data texture_distortion_data=
		{3," DISTORTION_CENTRE_X DISTORTION_CENTRE_Y DISTORTION_FACTOR_K1",0};
#if defined (SGI_MOVIE_FILE)
	struct X3d_movie *x3d_movie;
#endif /* defined (SGI_MOVIE_FILE) */

	ENTER(gfx_modify_Texture);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				process=0;
				if (texture_to_be_modified=(struct Texture *)texture_void)
				{
					if (IS_MANAGED(Texture)(texture_to_be_modified,
						command_data->texture_manager))
					{
						if (texture_to_be_modified_copy=CREATE(Texture)((char *)NULL))
						{
							MANAGER_COPY_WITH_IDENTIFIER(Texture,name)(
								texture_to_be_modified_copy,texture_to_be_modified);
							process=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Texture.  Could not create texture copy");
							return_code=0;
						}
					}
					else
					{
						texture_to_be_modified_copy=texture_to_be_modified;
						texture_to_be_modified=(struct Texture *)NULL;
						process=1;
					}
				}
				else
				{
					if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
					{
						if (command_data->texture_manager)
						{
							if (texture_to_be_modified=FIND_BY_IDENTIFIER_IN_MANAGER(Texture,
								name)(current_token,command_data->texture_manager))
							{
								if (return_code=shift_Parse_state(state,1))
								{
									if (texture_to_be_modified_copy=CREATE(Texture)((char *)NULL))
									{
										MANAGER_COPY_WITH_IDENTIFIER(Texture,name)(
											texture_to_be_modified_copy,texture_to_be_modified);
										process=1;
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"gfx_modify_Texture.  Could not create texture copy");
										return_code=0;
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"Unknown texture : %s",
									current_token);
								display_parse_state_location(state);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Texture.  Missing texture manager");
							return_code=0;
						}
					}
					else
					{
						if (texture_to_be_modified=CREATE(Texture)((char *)NULL))
						{
							(help_option_table[0]).to_be_modified=
								(void *)texture_to_be_modified;
							(help_option_table[0]).user_data=command_data_void;
							return_code=process_option(state,help_option_table);
								/*???DB.  return_code will be 0 ? */
							DESTROY(Texture)(&texture_to_be_modified);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Texture.  Could not create dummy texture");
							return_code=0;
						}
					}
				}
				if (process)
				{
#if defined (SGI_MOVIE_FILE)
					if (x3d_movie=Texture_get_movie(texture_to_be_modified_copy))
					{
						if (movie=FIRST_OBJECT_IN_MANAGER_THAT(Movie_graphics)(
							Movie_graphics_has_X3d_movie,(void *)x3d_movie,
							command_data->movie_graphics_manager))
						{
							ACCESS(Movie_graphics)(movie);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Texture.  Missing Movie_graphics for X3d_movie");
						}
					}
					else
					{
						movie = (struct Movie_graphics *)NULL;
					}
					old_movie=movie;
#endif /* defined (SGI_MOVIE_FILE) */
					Texture_get_combine_alpha(texture_to_be_modified_copy, &alpha);
					Texture_get_combine_colour(texture_to_be_modified_copy, &colour);
					Texture_get_physical_size(texture_to_be_modified_copy,&width,&height);
					field = (struct Computed_field *)NULL;
					spectrum = (struct Spectrum *)NULL;
					source_height = 100;
					source_width = 100;
					texture_coordinates_field = (struct Computed_field *)NULL;
					Texture_get_distortion_info(texture_to_be_modified_copy,
						&distortion_centre_x,&distortion_centre_y,&distortion_factor_k1);
					texture_distortion[0]=(double)distortion_centre_x;
					texture_distortion[1]=(double)distortion_centre_y;
					texture_distortion[2]=(double)distortion_factor_k1;
					i=0;
					/* alpha */
					(option_table[i]).to_be_modified=&alpha;
					i++;
					/* blend/decal/modulate */
					(combine_option_table[0]).to_be_modified=texture_to_be_modified_copy;
					(combine_option_table[1]).to_be_modified=texture_to_be_modified_copy;
					(combine_option_table[2]).to_be_modified=texture_to_be_modified_copy;
					(option_table[i]).user_data=combine_option_table;
					i++;
					/* clamp_wrap/repeat_wrap */
					(wrap_option_table[0]).to_be_modified=texture_to_be_modified_copy;
					(wrap_option_table[1]).to_be_modified=texture_to_be_modified_copy;
					(option_table[i]).user_data=wrap_option_table;
					i++;
					/* colour */
					(option_table[i]).to_be_modified=&colour;
					i++;
					/* distortion */
					(option_table[i]).to_be_modified= (void *)texture_distortion;
					(option_table[i]).user_data= &texture_distortion_data;
					i++;
					/* height */
					(option_table[i]).to_be_modified=&height;
					i++;
					/* image */
					(option_table[i]).to_be_modified=texture_to_be_modified_copy;
					(option_table[i]).user_data=
						command_data->set_file_name_option_table;
					i++;
					/* linear_filter/nearest_filter */
					(filter_option_table[0]).to_be_modified=texture_to_be_modified_copy;
					(filter_option_table[1]).to_be_modified=texture_to_be_modified_copy;
					(option_table[i]).user_data=filter_option_table;
					i++;
#if defined (SGI_MOVIE_FILE)
					/* movie */
					(option_table[i]).to_be_modified=&movie;
					(option_table[i]).user_data=command_data->movie_graphics_manager;
					i++;
#endif /* defined (SGI_MOVIE_FILE) */
					/* source_field */
					set_field_data.computed_field_package=
						command_data->computed_field_package;
					set_field_data.conditional_function=
						(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
					set_field_data.conditional_function_user_data=(void *)NULL;
					(option_table[i]).to_be_modified= &field;
					(option_table[i]).user_data= &set_field_data;
					i++;
					/* source_height_texels */
					(option_table[i]).to_be_modified= &source_height;
					i++;
					/* source_spectrum */
					(option_table[i]).to_be_modified= &spectrum;
					(option_table[i]).user_data= command_data->spectrum_manager;
					i++;
					/* source_width_texels */
					(option_table[i]).to_be_modified= &source_width;
					i++;
					/* source_texture_coordinates */
					set_texture_coordinates_field_data.computed_field_package=
						command_data->computed_field_package;
					set_texture_coordinates_field_data.conditional_function=
						Computed_field_is_find_element_xi_capable;
					set_texture_coordinates_field_data.conditional_function_user_data=
						(void *)NULL;
					(option_table[i]).to_be_modified= &texture_coordinates_field;
					(option_table[i]).user_data= &set_texture_coordinates_field_data;
					i++;
					/* width */
					(option_table[i]).to_be_modified=&width;
					i++;
					return_code=process_multiple_options(state,option_table);
					if (field || spectrum || texture_coordinates_field)
					{
						if((!field) || (!spectrum) || (!texture_coordinates_field))
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Texture.  To source a texture from a field you must specify\n"
								"a source_field, source_spectrum and source_texture_coordinates");
							return_code = 0;
						}
					}
					if (return_code)
					{
						Texture_set_combine_alpha(texture_to_be_modified_copy, alpha);
						Texture_set_combine_colour(texture_to_be_modified_copy, &colour);
						Texture_set_physical_size(texture_to_be_modified_copy,width,height);
						if (texture_distortion_data.set)
						{
							distortion_centre_x=(float)texture_distortion[0];
							distortion_centre_y=(float)texture_distortion[1];
							distortion_factor_k1=(float)texture_distortion[2];
							Texture_set_distortion_info(texture_to_be_modified_copy,
								distortion_centre_x,distortion_centre_y,distortion_factor_k1);
						}
						if (field && spectrum && texture_coordinates_field)
						{
							set_Texture_image_from_field(texture_to_be_modified_copy, field,
								texture_coordinates_field, spectrum, TEXTURE_RGB,
								source_width, source_height, "computed_field",
								0, 0, 0, 0);
						}
						if (texture_to_be_modified)
						{
							MANAGER_MODIFY_NOT_IDENTIFIER(Texture,name)(
								texture_to_be_modified,texture_to_be_modified_copy,
								command_data->texture_manager);
							DESTROY(Texture)(&texture_to_be_modified_copy);
						}
						else
						{
							texture_to_be_modified=texture_to_be_modified_copy;
						}
#if defined (SGI_MOVIE_FILE)
						if ( movie != old_movie )
						{
							/* Movie is outside manager copy so that is updates
								the correct texture based on movie events */
							Texture_set_movie(texture_to_be_modified,
								Movie_graphics_get_X3d_movie(movie),
								command_data->user_interface, "movie");
						}
#endif /* defined (SGI_MOVIE_FILE) */
#if defined (GL_API)
						texture_to_be_modified->index= -(texture_to_be_modified->index);
#endif
#if defined (MS_22AUG96)
#if defined (OPENGL_API)
						if (texture_to_be_modified->list_index)
						{
							glDeleteLists(texture_to_be_modified->list_index,1);
							texture_to_be_modified->list_index=0;
						}
#endif
#endif /* defined (MS_22AUG96) */
					}
#if defined (SGI_MOVIE_FILE)
					if (movie)
					{
						DEACCESS(Movie_graphics)(&movie);
					}
#endif /* defined (SGI_MOVIE_FILE) */
					if (spectrum)
					{
						DEACCESS(Spectrum)(&spectrum);
					}
					if (field)
					{
						DEACCESS(Computed_field)(&field);
					}
					if (texture_coordinates_field)
					{
						DEACCESS(Computed_field)(&texture_coordinates_field);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_Texture.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			if (texture_void)
			{
				display_message(WARNING_MESSAGE,"Missing texture modifications");
			}
			else
			{
				display_message(WARNING_MESSAGE,"Missing texture name");
			}
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_Texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_Texture */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE TEXTURE command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Texture *texture;

	ENTER(gfx_create_texture);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (command_data->texture_manager)
					{
						if (!FIND_BY_IDENTIFIER_IN_MANAGER(Texture,name)(current_token,
							command_data->texture_manager))
						{
							if (texture=CREATE(Texture)(current_token))
							{
								shift_Parse_state(state,1);
								if (state->current_token)
								{
									return_code=gfx_modify_Texture(state,(void *)texture,
										command_data_void);
								}
								else
								{
									return_code=1;
								}
								if (return_code)
								{
									ADD_OBJECT_TO_MANAGER(Texture)(texture,
										command_data->texture_manager);
								}
								else
								{
									DESTROY(Texture)(&texture);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_create_texture.  Error creating texture");
								return_code=0;
							}
						}
						else
						{
							display_message(WARNING_MESSAGE,"Texture '%s' already exists",
								current_token);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_texture.  Missing texture_manager");
						return_code=0;
					}
				}
				else
				{
					return_code=gfx_modify_Texture(state,(void *)NULL,
						command_data_void);
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_texture.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing texture name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_texture */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_texture_map(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Executes a GFX CREATE TEXMAP command.
==============================================================================*/
{
	char *in_file_name,*out_file_name;
	double xi_max[3];
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"as",NULL,(void *)1,set_name},
		{"coordinate",NULL,NULL,set_Computed_field_conditional},
		{"extent",NULL,NULL,set_Element_discretization},
		{"from",NULL,(void *)1,set_name},
		{"seed_element",NULL,NULL,set_FE_element_dimension_3},
		{"window",NULL,NULL,set_Graphics_window},
		{NULL,NULL,NULL,NULL}
	};
	struct Computed_field *coordinate_field;
	struct Cmiss_command_data *command_data;
	struct Element_discretization extent;
	struct FE_element *seed_element;
	struct Graphics_window *window;
	struct Set_Computed_field_conditional_data set_coordinate_field_data;

	ENTER(gfx_create_texture_map);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			if (ALLOCATE(out_file_name,char,11))
			{
				strcpy(out_file_name,"texmap.rgb");
			}
			in_file_name=(char *)NULL;
			seed_element=(struct FE_element *)NULL;
			extent.number_in_xi1=1;
			extent.number_in_xi2=1;
			extent.number_in_xi3=1;
			if (window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,command_data->graphics_window_manager))
			{
				ACCESS(Graphics_window)(window);
			}
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package));
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);;
			(option_table[0]).to_be_modified= &out_file_name;
			/* coordinate */
			set_coordinate_field_data.computed_field_package=
				command_data->computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[1]).to_be_modified= &coordinate_field;
			(option_table[1]).user_data=&set_coordinate_field_data;
			(option_table[2]).to_be_modified= &extent;
			(option_table[2]).user_data=(void *)(command_data->user_interface);
			(option_table[3]).to_be_modified= &in_file_name;
			(option_table[4]).to_be_modified= &seed_element;
			(option_table[4]).user_data=command_data->element_manager;
			(option_table[5]).to_be_modified= &window;
			(option_table[5]).user_data=command_data->graphics_window_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (window)
				{
					if (seed_element)
					{
						xi_max[0]=(double)(extent.number_in_xi1);
						xi_max[1]=(double)(extent.number_in_xi2);
						xi_max[2]=(double)(extent.number_in_xi3);
						if (in_file_name&&out_file_name&&coordinate_field)
						{
							generate_textureimage_from_FE_element(window,in_file_name,
								out_file_name,seed_element,xi_max,coordinate_field);
						}
						else
						{
							display_message(WARNING_MESSAGE,"Missing option(s)");
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"Missing seed element");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Must have a graphics window");
					return_code=0;
				}
			} /* parse error, help */
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
			if (in_file_name)
			{
				DEALLOCATE(in_file_name);
			}
			if (out_file_name)
			{
				DEALLOCATE(out_file_name);
			}
			if (seed_element)
			{
				DEACCESS(FE_element)(&seed_element);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_texture_map.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_texture_map.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_texture_map */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_time_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 15 June 1999

DESCRIPTION :
Executes a GFX CREATE TIME_EDITOR command.
If there is a time editor dialog in existence, then bring it to the front,
otherwise it creates a new one.  Assumes we will only ever want one time
editor at a time.  This implementation may be changed later.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_graphical_time_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				return_code=bring_up_time_editor_dialog(
					&(command_data->time_editor_dialog),
					command_data->user_interface->application_shell,
					command_data->default_time_keeper, 
					command_data->user_interface);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_graphical_time_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_graphical_time_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_graphical_time_editor */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_control_curve_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Executes a GFX CREATE CONTROL_CURVE_EDITOR command.
If there is a variable editor dialog in existence, then bring it to the front,
otherwise it creates a new one.  Assumes we will only ever want one variable
editor at a time.  This implementation may be changed later.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_control_curve_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				return_code=bring_up_control_curve_editor_dialog(
					&(command_data->control_curve_editor_dialog),
					command_data->user_interface->application_shell,
					command_data->control_curve_manager,
					(struct Control_curve *)NULL,
					command_data->user_interface);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_control_curve_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_control_curve_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_control_curve_editor */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_tracking_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 April 2000

DESCRIPTION :
Executes a GFX CREATE TRACKING_EDITOR command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_tracking_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
#if defined (MIRAGE)
				return_code=open_tracking_editor_dialog(
					&(command_data->tracking_editor_dialog),
					tracking_editor_close_cb,
					&(command_data->background_colour),
					command_data->basis_manager,
					command_data->computed_field_package,
					command_data->element_manager,
					command_data->element_group_manager,
					command_data->fe_field_manager,
					command_data->glyph_list,
					command_data->graphical_material_manager,
					command_data->default_graphical_material,
					command_data->graphics_window_manager,
					command_data->light_manager,
					command_data->default_light,
					command_data->light_model_manager,
					command_data->default_light_model,
					command_data->node_manager,
					command_data->node_group_manager,
					command_data->data_manager,
					command_data->data_group_manager,
					command_data->element_point_ranges_selection,
					command_data->element_selection,
					command_data->node_selection,
					command_data->data_selection,
					command_data->scene_manager,
					command_data->default_scene,
					command_data->spectrum_manager,
					command_data->default_spectrum,
					command_data->texture_manager,
					command_data->interactive_tool_manager,
					command_data->user_interface);
#else /* defined (MIRAGE) */
				display_message(ERROR_MESSAGE,"Tracking editor is not available");
				return_code=0;
#endif /* defined (MIRAGE) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_tracking_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_tracking_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_tracking_editor */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_robot_7dof(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE OBJECT HEART_ROBOT command.
==============================================================================*/
{
	static char default_name[]="robot_7dof";
	char *graphics_object_name;
	gtObject *graphics_object;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Graphical_material *material;
	struct GT_userdef *robot_7dof;
	static struct Modifier_entry option_table[]=
	{
		{"as",NULL,(void *)1,set_name},
		{"material",NULL,NULL,set_Graphical_material},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_create_robot_7dof);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			if (ALLOCATE(graphics_object_name,char,strlen(default_name)))
			{
				strcpy(graphics_object_name,default_name);
			}
			else
			{
				graphics_object_name=(char *)NULL;
			}
			/* must access it now, because we deaccess it later */
			material=ACCESS(Graphical_material)(command_data->
				default_graphical_material);
			(option_table[0]).to_be_modified= &graphics_object_name;
			(option_table[1]).to_be_modified= &material;
			(option_table[1]).user_data=command_data->graphical_material_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(graphics_object_name,
					command_data->graphics_object_list))
				{
					display_message(ERROR_MESSAGE,"Object '%s' already exists",
						graphics_object_name);
					return_code=0;
				}
				else
				{
					if ((graphics_object=CREATE(GT_object)(graphics_object_name,g_USERDEF,
						material))&&
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list))
					{
						if (!((robot_7dof=create_robot_7dof())&&
							GT_OBJECT_ADD(GT_userdef)(graphics_object,0.0,robot_7dof)))
						{
							if (robot_7dof)
							{
								display_message(ERROR_MESSAGE,
									"gfx_create_robot_7dof.  Could not add userdef primitive");
								display_message(ERROR_MESSAGE,"gfx_create_robot_7dof.  "
									"No destroy function for userdef objects - memory lost");
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_create_robot_7dof.  Could not create 7dof robot");
							}
							/* the object will be automatically destroyed when removed */
							REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
								command_data->graphics_object_list);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_robot_7dof.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
			} /* parse error, help */
			DEACCESS(Graphical_material)(&material);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_robot_7dof.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_robot_7dof.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_robot_7dof */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_userdef(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE OBJECT command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"robot_7dof",NULL,NULL,gfx_create_robot_7dof},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_userdef);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (state->current_token)
		{
			(option_table[0]).user_data=command_data_void;
			return_code=process_option(state,option_table);
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				set_command_prompt("gfx create object",command_data->command_window);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_userdef.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_robot_7dof.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_userdef */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_volume_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE VOLUME_EDITOR command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_volume_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				create_texture_edit_window(command_data->default_graphical_material,
					command_data->graphical_material_manager,
					command_data->environment_map_manager,command_data->texture_manager,
					&(command_data->material_editor_dialog),
					&(command_data->transformation_editor_dialog),
					command_data->user_interface);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_volume_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_volume_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_volume_editor */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_volumes(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 January 2000

DESCRIPTION :
Executes a GFX CREATE VOLUMES command.
==============================================================================*/
{
	char *graphics_object_name,*render_type_string,**valid_strings;
	float time;
	int displacement_map_xi_direction,number_of_valid_strings, return_code;
	static char default_name[]="volumes";
	struct Clipping *clipping;
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field, *data_field,
		*displacement_map_field, *surface_data_density_field,
		*blur_field;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Element_to_volume_data element_to_volume_data;
	struct FE_element *seed_element;
	struct FE_node *data_to_destroy;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *surface_data_group;
	struct GT_object *graphics_object;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_displacement_map_field_data, 
		set_surface_data_density_field_data, set_blur_field_data;
	struct Spectrum *spectrum;
	struct VT_volume_texture *volume_texture;

	ENTER(gfx_create_volumes);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			if (ALLOCATE(graphics_object_name,char,strlen(default_name)+1))
			{
				strcpy(graphics_object_name,default_name);
			}
			else
			{
				graphics_object_name=(char *)NULL;
			}
			element_group=(struct GROUP(FE_element) *)NULL;
			clipping=(struct Clipping *)NULL;
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			data_field=(struct Computed_field *)NULL;
			seed_element=(struct FE_element *)NULL;
			spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
			surface_data_group = (struct GROUP(FE_node) *)NULL;
			surface_data_density_field=(struct Computed_field *)NULL;
			time=0;
			volume_texture=(struct VT_volume_texture *)NULL;
			displacement_map_field = (struct Computed_field *)NULL;
			displacement_map_xi_direction=3;
			blur_field = (struct Computed_field *)NULL;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* clipping */
			Option_table_add_entry(option_table,"clipping",&clipping,
				NULL,set_Clipping);
			/* coordinate */
			set_coordinate_field_data.computed_field_package=
				command_data->computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_package=
				command_data->computed_field_package;
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&data_field,
				&set_data_field_data,set_Computed_field_conditional);
			/* displacement_map_field */
			set_displacement_map_field_data.computed_field_package=
				command_data->computed_field_package;
			set_displacement_map_field_data.conditional_function=
				Computed_field_has_up_to_4_numerical_components;
			set_displacement_map_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"displacement_map_field",
				&displacement_map_field,&set_displacement_map_field_data,
				set_Computed_field_conditional);
			/* displacement_map_xi_direction */
			Option_table_add_entry(option_table,"displacement_map_xi_direction",
				&displacement_map_xi_direction,NULL,set_int_positive);
			/* from */
			Option_table_add_entry(option_table,"from",&element_group,
				command_data->element_group_manager,set_FE_element_group);
			/* render_type */
			render_type_string=
				Render_type_string(RENDER_TYPE_SHADED);
			valid_strings=Render_type_get_valid_strings(
				&number_of_valid_strings);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&render_type_string);
			DEALLOCATE(valid_strings);
			/* seed_element */
			Option_table_add_entry(option_table,"seed_element",
				&seed_element,command_data->element_manager,set_FE_element_dimension_3);
			/* smooth_field */
			set_blur_field_data.computed_field_package=
				command_data->computed_field_package;
			set_blur_field_data.conditional_function=
				Computed_field_has_up_to_4_numerical_components;
			set_blur_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"smooth_field",
				&blur_field,&set_blur_field_data,set_Computed_field_conditional);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			/* surface_data_density */
			set_surface_data_density_field_data.computed_field_package=
				command_data->computed_field_package;
			set_surface_data_density_field_data.conditional_function=
				Computed_field_has_up_to_4_numerical_components;
			set_surface_data_density_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"surface_data_density",
				&surface_data_density_field,&set_surface_data_density_field_data,
				set_Computed_field_conditional);
			/* surface_data_group */
			Option_table_add_entry(option_table,"surface_data_group",
				&surface_data_group,command_data->data_group_manager,set_FE_node_group);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* vtexture */
			Option_table_add_entry(option_table,"vtexture",
				&volume_texture,command_data->volume_texture_manager,
				set_VT_volume_texture);
			return_code=Option_table_multi_parse(option_table,state);
			if(surface_data_group&&(!surface_data_density_field))
			{
				display_message(ERROR_MESSAGE,"gfx_create_volumes.  "
					"Must supply a surface_data_density_field with a surface_data_group");
				return_code=0;
			}
			if((!surface_data_group)&&surface_data_density_field)
			{
				display_message(ERROR_MESSAGE,"gfx_create_volumes.  "
					"Must supply a surface_data_group with a surface_data_density_field");
				return_code=0;
			}

			/* no errors, not asking for help */
			if (return_code)
			{
				if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list))
				{
					if (g_VOLTEX==graphics_object->object_type)
					{
						if (GT_object_has_time(graphics_object,time))
						{
							display_message(WARNING_MESSAGE,
								"Overwriting time %g in graphics object '%s'",time,
								graphics_object_name);
							return_code=GT_object_delete_time(graphics_object,time);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Object of different type named '%s' already exists",
							graphics_object_name);
						return_code=0;
					}
				}
				else
				{
					if ((graphics_object=CREATE(GT_object)(graphics_object_name,
						g_VOLTEX,(struct Graphical_material *)NULL))&&
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_volumes.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
				if (return_code && surface_data_group)
				{
					return_code = 1;
					MANAGED_GROUP_BEGIN_CACHE(FE_node)(surface_data_group);

					/* Remove all the current data from the group */
					while(return_code&&(data_to_destroy=FIRST_OBJECT_IN_GROUP_THAT(FE_node)
						((GROUP_CONDITIONAL_FUNCTION(FE_node) *)NULL, NULL, surface_data_group)))
					{
						return_code = REMOVE_OBJECT_FROM_GROUP(FE_node)(
							data_to_destroy, surface_data_group);
						if (FE_node_can_be_destroyed(data_to_destroy))
						{
							return_code = REMOVE_OBJECT_FROM_MANAGER(FE_node)(data_to_destroy,
								command_data->data_manager);
						}
					}
					if(!return_code)
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_volumes.  Unable to remove all data from data_group");
						MANAGED_GROUP_END_CACHE(FE_node)(surface_data_group);
					}
				}
				if (return_code)
				{
					element_to_volume_data.coordinate_field=
						Computed_field_begin_wrap_coordinate_field(coordinate_field);
					element_to_volume_data.data_field=data_field;
					element_to_volume_data.graphics_object=graphics_object;
					element_to_volume_data.time=time;
					element_to_volume_data.render_type=
						Render_type_from_string(render_type_string);
					element_to_volume_data.volume_texture=volume_texture;
					element_to_volume_data.displacement_map_field = displacement_map_field;
					element_to_volume_data.displacement_map_xi_direction =
						displacement_map_xi_direction;
					element_to_volume_data.surface_data_density_field = 
						surface_data_density_field;
					element_to_volume_data.surface_data_group = 
						surface_data_group;
					element_to_volume_data.data_manager = 
						command_data->data_manager;
					element_to_volume_data.fe_field_manager =
						command_data->fe_field_manager;
					element_to_volume_data.blur_field= blur_field;
					element_to_volume_data.clipping=clipping;
					if (seed_element)
					{
						return_code=element_to_volume(seed_element,
							(void *)&element_to_volume_data);
					}
					else
					{
						/* no seed element specified, use all elements in the group as
							seeds*/
						if (element_group)
						{
							return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
								element_to_volume,(void *)&element_to_volume_data,
								element_group);
						}
						else
						{
							return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
								element_to_volume,(void *)&element_to_volume_data,
								command_data->element_manager);
						}
					}
					if (return_code)
					{
						if (!GT_object_has_time(graphics_object,time))
						{
							/* add a NULL voltex to make an empty time */
							GT_OBJECT_ADD(GT_voltex)(graphics_object,time,
								(struct GT_voltex *)NULL);
						}
						if (data_field)
						{
							return_code=set_GT_object_Spectrum(graphics_object,spectrum);
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"No volumes created");
						if (0==GT_object_get_number_of_times(graphics_object))
						{
							REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
								command_data->graphics_object_list);
						}
					}
					Computed_field_end_wrap(
						&(element_to_volume_data.coordinate_field));
					if (surface_data_group)
					{
						MANAGED_GROUP_END_CACHE(FE_node)(surface_data_group);
					}
				}
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			/* DEACCESS blur and displacement map stuff ? */
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (data_field)
			{
				DEACCESS(Computed_field)(&data_field);
			}
			DEACCESS(Spectrum)(&spectrum);
			if (clipping)
			{
				DESTROY(Clipping)(&clipping);
			}
			DEALLOCATE(graphics_object_name);
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
			if (seed_element)
			{
				DEACCESS(FE_element)(&seed_element);
			}
			if (surface_data_density_field)
			{
				DEACCESS(Computed_field)(&surface_data_density_field);
			}
			if (surface_data_group)
			{
				DEACCESS(GROUP(FE_node))(&surface_data_group);
			}
			if (volume_texture)
			{
				DEACCESS(VT_volume_texture)(&volume_texture);
			}
			if (displacement_map_field)
			{
				DEACCESS(Computed_field)(&displacement_map_field);
			}
			if (blur_field)
			{
				DEACCESS(Computed_field)(&blur_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_volumes.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_volumes.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_volumes */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_volume_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE VTEXTURE command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Modify_VT_volume_texture_data modify_VT_volume_texture_data;
	struct VT_volume_texture *volume_texture;

	ENTER(gfx_create_volume_texture);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (command_data=(struct Cmiss_command_data *)command_data_void)
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(VT_volume_texture,name)(
						current_token,command_data->volume_texture_manager))
					{
						if (volume_texture=CREATE(VT_volume_texture)(current_token))
						{
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								modify_VT_volume_texture_data.graphical_material_manager=
									command_data->graphical_material_manager;
								modify_VT_volume_texture_data.environment_map_manager=
									command_data->environment_map_manager;
								modify_VT_volume_texture_data.volume_texture_manager=
									command_data->volume_texture_manager;
								modify_VT_volume_texture_data.set_file_name_option_table=
									command_data->set_file_name_option_table;
								return_code=modify_VT_volume_texture(state,
									(void *)volume_texture,
									(void *)(&modify_VT_volume_texture_data));
							}
							else
							{
								return_code=1;
							}
							ADD_OBJECT_TO_MANAGER(VT_volume_texture)(volume_texture,
								command_data->volume_texture_manager);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_volume_texture.  Error creating volume texture");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Volume texture already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_volume_texture.  Missing command_data_void");
					return_code=0;
				}
			}
			else
			{
				if (command_data=(struct Cmiss_command_data *)command_data_void)
				{
					modify_VT_volume_texture_data.graphical_material_manager=
						command_data->graphical_material_manager;
					modify_VT_volume_texture_data.environment_map_manager=
						command_data->environment_map_manager;
					modify_VT_volume_texture_data.volume_texture_manager=
						command_data->volume_texture_manager;
					modify_VT_volume_texture_data.set_file_name_option_table=
						command_data->set_file_name_option_table;
					return_code=modify_VT_volume_texture(state,(void *)NULL,
						(void *)(&modify_VT_volume_texture_data));
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_volume_texture.  Missing command_data_void");
					return_code=0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing volume texture name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_volume_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_volume_texture */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_window(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX CREATE WINDOW command.
==============================================================================*/
{
	char double_buffer_flag,*name,single_buffer_flag;
	enum Scene_viewer_buffer_mode buffer_mode;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;
	static struct Modifier_entry
		buffer_option_table[]=
		{
			{"single_buffer",NULL,NULL,set_char_flag},
			{"double_buffer",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{"name",NULL,(void *)1,set_name},
			{NULL,NULL,NULL,NULL},
			{NULL,NULL,NULL,set_name}
		};

	ENTER(gfx_create_window);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			return_code=1;
			/* set_defaults */
			name=Graphics_window_manager_get_new_name(
				command_data->graphics_window_manager);
			buffer_mode=SCENE_VIEWER_DOUBLE_BUFFER;
			if (state->current_token)
			{
				/* change defaults */
				single_buffer_flag=0;
				double_buffer_flag=0;
				(option_table[0]).to_be_modified= &name;
				(buffer_option_table[0]).to_be_modified= &single_buffer_flag;
				(buffer_option_table[1]).to_be_modified= &double_buffer_flag;
				(option_table[1]).user_data=buffer_option_table;
				(option_table[2]).to_be_modified= &name;
				if (return_code=process_multiple_options(state,option_table))
				{
					if (single_buffer_flag)
					{
						buffer_mode=SCENE_VIEWER_SINGLE_BUFFER;
						if (double_buffer_flag)
						{
							display_message(ERROR_MESSAGE,
								"Only one of single_buffer/double_buffer");
							return_code=0;
						}
					}
					else
					{
						buffer_mode=SCENE_VIEWER_DOUBLE_BUFFER;
					}
				}
			}
			if (!name)
			{
				display_message(ERROR_MESSAGE,"gfx_create_window.  Missing name");
				return_code=0;
			}
			if (return_code)
			{
				if (window=FIND_BY_IDENTIFIER_IN_MANAGER(Graphics_window,name)(name,
					command_data->graphics_window_manager))
				{
					display_message(WARNING_MESSAGE,
						"Graphics window '%s' already exists",name);
					return_code=0;
				}
				else
				{
					if (window=CREATE(Graphics_window)(name,
						buffer_mode,&(command_data->background_colour),
						command_data->light_manager,command_data->default_light,
						command_data->light_model_manager,command_data->default_light_model,
						command_data->scene_manager,command_data->default_scene,
						command_data->texture_manager,
						command_data->interactive_tool_manager,
						command_data->user_interface))
					{
						if (!ADD_OBJECT_TO_MANAGER(Graphics_window)(window,
							command_data->graphics_window_manager))
						{
							DESTROY(Graphics_window)(&window);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_window.  Could not create graphics window");
						return_code=0;
					}
				}
			}
			if (name)
			{
				DEALLOCATE(name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_window.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_window.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_window */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if defined (HAPTIC)
static int gfx_create_haptic(struct Parse_state *state,
	void *dummy__to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 12 May 1998

DESCRIPTION :
Executes a GFX CREATE HAPTIC command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	float dynamic_friction, static_friction, damping,
		spring_k;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	static struct Modifier_entry
		option_table[]=
		{
			{"scene",NULL,NULL,set_Scene},
			{"dynamic_surface_friction",NULL,NULL,set_float_0_to_1_inclusive},
			{"static_surface_friction",NULL,NULL,set_float_0_to_1_inclusive},
			{"damping_surface",NULL,NULL,set_float_0_to_1_inclusive},
			{"spring_k_surface",NULL,NULL,set_float /* SAB set_float_0_to_1_inclusive */},
			{NULL,NULL,NULL,NULL}
		};

	ENTER(gfx_create_haptic);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			return_code=1;
			/* set_defaults */
			scene = command_data->default_scene;
			dynamic_friction = 0.2;
			static_friction = 0.35;
			damping = 0.0;
			spring_k = 1.0;
			if (current_token=state->current_token)
			{
				(option_table[0]).to_be_modified= &scene;
				(option_table[0]).user_data=command_data->scene_manager;
				(option_table[1]).to_be_modified= &dynamic_friction;
				(option_table[2]).to_be_modified= &static_friction;
				(option_table[3]).to_be_modified= &damping;
				(option_table[4]).to_be_modified= &spring_k;
				if (return_code=process_multiple_options(state,option_table))
				{
					/* SAB Damping is really in Kg/1000.0*sec and ranges from
						0 to 0.005, so the parameter is scaled here */
					damping *= 0.005;
					haptic_set_surface_defaults( dynamic_friction, static_friction,
						damping, spring_k );
				}
			}
			if ( return_code )
			{
				haptic_create_scene( scene );
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_haptic.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_haptic.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_haptic */
#endif /* defined (HAPTIC) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_3d_digitizer(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Executes a GFX CREATE 3D_DIGITIZER command.
If there is a data_grabber dialog in existence, then bring it to the front,
else create a new one.  Assumes we will only ever want one data_grabber at
a time.  This implementation may be changed later.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_3d_digitizer);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				return_code=bring_up_data_grabber_dialog(
					&(command_data->data_grabber_dialog),
					command_data->user_interface->application_shell,
					command_data->execute_command,command_data->user_interface,
					command_data->fe_field_manager,
					command_data->node_manager, command_data->data_manager,
					command_data->node_group_manager, command_data->data_group_manager);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_3d_digitizer.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_3d_digitizer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_3d_digitizer */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_data_sync(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 15 June 1999

DESCRIPTION :
Executes a GFX CREATE DATA_SYNC command.
If there is a data_sync dialog in existence, then bring it to the front,
else create a new one.  Assumes we will only ever want one data_sync at
a time.  This implementation may be changed later.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_data_sync);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				return_code=bring_up_sync_2d_3d_dialog(
					&(command_data->sync_2d_3d_dialog),
					command_data->user_interface->application_shell,
					command_data->execute_command,command_data->fe_field_manager,
					command_data->data_manager,command_data->data_group_manager);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_data_sync.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_data_sync.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_data_sync */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_cmiss(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX CREATE CMISS_CONNECTION command.
==============================================================================*/
{
	char *examples_directory,*host_name,mycm_flag,*parameters_file_name;
	enum Machine_type host_type;
	int connection_number,return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"connection_number",NULL,NULL,set_int_non_negative},
		{"examples_directory",NULL,(void *)1,set_name},
		{"host",NULL,(void *)1,set_name},
		{"mycm",NULL,NULL,set_char_flag},
		{"parameters",NULL,(void *)1,set_name},
		{"type",NULL,NULL,set_machine_type},
		{NULL,NULL,NULL,set_name}
	};
/*???DB.  Not sure if this is quite the right place */
	double wormhole_timeout;
#if defined (MOTIF)
#define XmNwormholeTimeoutSeconds "wormholeTimeoutSeconds"
#define XmCWormholeTimeoutSeconds "WormholeTimeoutSeconds"
	static XtResource resources[]=
	{
		{
			XmNwormholeTimeoutSeconds,
			XmCWormholeTimeoutSeconds,
			XmRInt,
			sizeof(int),
			0,
			XmRString,
			"300"
		}
	};
	int wormhole_timeout_seconds;
#endif /* defined (MOTIF) */

	ENTER(gfx_create_cmiss);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			if (command_data->cm_examples_directory)
			{
				if (ALLOCATE(examples_directory,char,
					strlen(command_data->cm_examples_directory)+1))
				{
					strcpy(examples_directory,command_data->cm_examples_directory);
				}
				else
				{
					examples_directory=(char *)NULL;
				}
			}
			else
			{
				examples_directory=(char *)NULL;
			}
			if (command_data->cm_parameters_file_name)
			{
				if (ALLOCATE(parameters_file_name,char,
					strlen(command_data->cm_parameters_file_name)+1))
				{
					strcpy(parameters_file_name,command_data->cm_parameters_file_name);
				}
				else
				{
					parameters_file_name=(char *)NULL;
				}
			}
			else
			{
				parameters_file_name=(char *)NULL;
			}
			if ((command_data->user_interface)&&
				(command_data->user_interface->local_machine_info)&&
				(command_data->user_interface->local_machine_info->name))
			{
				if (ALLOCATE(host_name,char,
					strlen(command_data->user_interface->local_machine_info->name)+1))
				{
					strcpy(host_name,
						command_data->user_interface->local_machine_info->name);
				}
				else
				{
					host_name=(char *)NULL;
				}
			}
			else
			{
				host_name=(char *)NULL;
			}
			connection_number=0;
			host_type=MACHINE_UNKNOWN;
			mycm_flag=0;
			wormhole_timeout=300;
#if defined (MOTIF)
			if (command_data->user_interface)
			{
				XtVaGetApplicationResources(command_data->user_interface->
					application_shell,&wormhole_timeout_seconds,resources,
					XtNumber(resources),NULL);
				wormhole_timeout=(double)wormhole_timeout_seconds;
			}
#endif /* defined (MOTIF) */
			(option_table[0]).to_be_modified= &connection_number;
			(option_table[1]).to_be_modified= &examples_directory;
			(option_table[2]).to_be_modified= &host_name;
			(option_table[3]).to_be_modified= &mycm_flag;
			(option_table[4]).to_be_modified= &parameters_file_name;
			(option_table[5]).to_be_modified= &host_type;
			(option_table[6]).to_be_modified= &host_name;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
#if defined (LINK_CMISS)
				if (!CMISS)
				{
					if (MACHINE_UNKNOWN==host_type)
					{
						/* have a stab at identifying it */
						if (fuzzy_string_compare(host_name,"esu"))
						{
							host_type=MACHINE_UNIX;
						}
						else
						{
							if (fuzzy_string_compare(host_name,"esv"))
							{
								host_type=MACHINE_VMS;
							}
						}
					}
					if (CMISS=CREATE(CMISS_connection)(host_name,host_type,
						connection_number,wormhole_timeout,mycm_flag,
						command_data->element_manager,
						command_data->element_group_manager,
						command_data->fe_field_manager,command_data->node_manager,
						command_data->data_manager,command_data->node_group_manager,
						command_data->data_group_manager,&(command_data->prompt_window),
						parameters_file_name,examples_directory,
						command_data->execute_command,command_data->user_interface))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_cmiss.  Could not create CMISS connection");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_cmiss.  CMISS connection already exists");
					return_code=0;
				}
#else /* defined (LINK_CMISS) */
			display_message(ERROR_MESSAGE,"gfx_create_cmiss.  Define LINK_CMISS");
			return_code=0;
#endif /* defined (LINK_CMISS) */
			} /* parse error, help */
			DEALLOCATE(examples_directory);
			DEALLOCATE(host_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_create_cmiss.  Missing command data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_cmiss.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_cmiss */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int execute_command_gfx_create(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 24 May 2000

DESCRIPTION :
Executes a GFX CREATE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Create_emoter_slider_data create_emoter_slider_data;
	struct Create_node_group_slider_data create_node_group_slider_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_create);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				create_node_group_slider_data.execute_command=
					command_data->execute_command;
				create_node_group_slider_data.fe_field_manager=
					command_data->fe_field_manager;
				create_node_group_slider_data.node_manager=
					command_data->node_manager;
				create_node_group_slider_data.node_group_manager=
					command_data->node_group_manager;
				create_node_group_slider_data.node_group_slider_dialog_address=
					&(command_data->node_group_slider_dialog);
				create_node_group_slider_data.control_curve_manager=
					command_data->control_curve_manager;
				create_node_group_slider_data.user_interface=
					command_data->user_interface;
				if (command_data->user_interface)
				{
					create_node_group_slider_data.parent=
						command_data->user_interface->application_shell;
				}
				else
				{
					create_node_group_slider_data.parent=(Widget)NULL;
				}
				create_emoter_slider_data.execute_command=command_data->execute_command;
				create_emoter_slider_data.fe_field_manager=
					command_data->fe_field_manager;
				create_emoter_slider_data.node_manager=command_data->node_manager;
				create_emoter_slider_data.element_manager=command_data->element_manager;
				create_emoter_slider_data.graphics_window_manager=
					command_data->graphics_window_manager;
				create_emoter_slider_data.data_group_manager=
					command_data->data_group_manager;
				create_emoter_slider_data.element_group_manager=
					command_data->element_group_manager;
				create_emoter_slider_data.node_group_manager=
					command_data->node_group_manager;
				create_emoter_slider_data.control_curve_manager=
					command_data->control_curve_manager;
				create_emoter_slider_data.viewer_scene=command_data->default_scene;
				create_emoter_slider_data.viewer_light=command_data->default_light;
				create_emoter_slider_data.viewer_light_model=
					command_data->default_light_model;
				create_emoter_slider_data.viewer_background_colour=
					command_data->background_colour;
				create_emoter_slider_data.emoter_slider_dialog_address=
					&(command_data->emoter_slider_dialog);
				if (command_data->user_interface)
				{
					create_emoter_slider_data.parent=
						command_data->user_interface->application_shell;
				}
				else
				{
					create_emoter_slider_data.parent=(Widget)NULL;
				}
				create_emoter_slider_data.control_curve_editor_dialog_address=
					&(command_data->control_curve_editor_dialog);
				create_emoter_slider_data.user_interface=
					command_data->user_interface;
#if defined (SCENE_IN_EMOTER)
				create_emoter_slider_data.command_data=command_data;
					/*???DB.  command_data shouldn't leave this module */
#endif /* defined (SCENE_IN_EMOTER) */

				option_table=CREATE(Option_table)();
				Option_table_add_entry(option_table,"annotation",NULL,
					command_data_void,gfx_create_annotation);
				Option_table_add_entry(option_table,"cmiss_connection",NULL,
					command_data_void,gfx_create_cmiss);
				Option_table_add_entry(option_table,"colour_bar",NULL,
					command_data_void,gfx_create_colour_bar);
				Option_table_add_entry(option_table,"curve_editor",NULL,
					command_data_void,gfx_create_control_curve_editor);
				Option_table_add_entry(option_table,"cylinders",NULL,
					command_data_void,gfx_create_cylinders);
				Option_table_add_entry(option_table,"data_viewer",NULL,
					command_data_void,gfx_create_data_viewer);
				Option_table_add_entry(option_table,"data_points",NULL,
					command_data_void,gfx_create_data_points);
				Option_table_add_entry(option_table,"data_sync",NULL,
					command_data_void,gfx_create_data_sync);
				Option_table_add_entry(option_table,"dgroup",NULL,
					command_data_void,gfx_create_data_group);
				Option_table_add_entry(option_table,"egroup",NULL,
					command_data_void,gfx_create_element_group);
				Option_table_add_entry(option_table,"element_creator",NULL,
					command_data_void,gfx_create_element_creator);
				Option_table_add_entry(option_table,"element_point_viewer",NULL,
					command_data_void,gfx_create_element_point_viewer);
				Option_table_add_entry(option_table,"element_points",NULL,
					command_data_void,gfx_create_element_points);
#if defined (MIRAGE)
				Option_table_add_entry(option_table,"emoter",NULL,
					(void *)&create_emoter_slider_data,gfx_create_emoter);
				Option_table_add_entry(option_table,"em_sliders",NULL,
					(void *)&create_node_group_slider_data,create_em_sliders);
#endif /* defined (MIRAGE) */
				Option_table_add_entry(option_table,"environment_map",NULL,
					command_data_void,gfx_create_environment_map);
				Option_table_add_entry(option_table,"flow_particles",NULL,
					command_data_void,gfx_create_flow_particles);
				Option_table_add_entry(option_table,"g_element_editor",NULL,
					command_data_void,gfx_create_g_element_editor);
				Option_table_add_entry(option_table,"graphical_material_editor",NULL,
					command_data_void,gfx_create_graphical_material_editor);
				Option_table_add_entry(option_table,"grid_field_calculator",NULL,
					command_data_void,gfx_create_grid_field_calculator);
#if defined (HAPTIC)
				Option_table_add_entry(option_table,"haptic",NULL,
					command_data_void,gfx_create_haptic);
#endif /* defined (HAPTIC) */
				Option_table_add_entry(option_table,"im_control",NULL,
					command_data_void,gfx_create_input_module_control);
				Option_table_add_entry(option_table,"interactive_data_editor",NULL,
					command_data_void,gfx_create_interactive_data_editor);
				Option_table_add_entry(option_table,"interactive_node_editor",NULL,
					command_data_void,gfx_create_interactive_node_editor);
				Option_table_add_entry(option_table,"iso_surfaces",NULL,
					command_data_void,gfx_create_iso_surfaces);
				Option_table_add_entry(option_table,"light",NULL,
					command_data_void,gfx_create_light);
				Option_table_add_entry(option_table,"lmodel",NULL,
					command_data_void,gfx_create_light_model);
				Option_table_add_entry(option_table,"lines",NULL,
					command_data_void,gfx_create_lines);
				Option_table_add_entry(option_table,"material",NULL,
					command_data_void,gfx_create_material);
				Option_table_add_entry(option_table,"more_flow_particles",NULL,
					command_data_void,gfx_create_more_flow_particles);
				Option_table_add_entry(option_table,"morph",NULL,
					command_data_void,gfx_create_morph);
				Option_table_add_entry(option_table,"muscle_slider",NULL,
					(void *)&create_node_group_slider_data,create_muscle_slider);
				Option_table_add_entry(option_table,"ngroup",NULL,
					command_data_void,gfx_create_node_group);
				Option_table_add_entry(option_table,"node_points",NULL,
					command_data_void,gfx_create_node_points);
				Option_table_add_entry(option_table,"node_viewer",NULL,
					command_data_void,gfx_create_node_viewer);
				Option_table_add_entry(option_table,"object",NULL,
					command_data_void,gfx_create_userdef);
				Option_table_add_entry(option_table,"pivot_slider",NULL,
					(void *)&create_node_group_slider_data,create_pivot_slider);
				Option_table_add_entry(option_table,"scene",NULL,
					command_data_void,gfx_create_scene);
				Option_table_add_entry(option_table,"spectrum",NULL,
					command_data_void,gfx_create_spectrum);
				Option_table_add_entry(option_table,"streamlines",NULL,
					command_data_void,gfx_create_streamlines);
				Option_table_add_entry(option_table,"strline_interactive",NULL,
					command_data_void,gfx_create_interactive_streamline);
				Option_table_add_entry(option_table,"surfaces",NULL,
					command_data_void,gfx_create_surfaces);
				Option_table_add_entry(option_table,"texmap",NULL,
					command_data_void,gfx_create_texture_map);
				Option_table_add_entry(option_table,"texture",NULL,
					command_data_void,gfx_create_texture);
				Option_table_add_entry(option_table,"time_editor",NULL,
					command_data_void,gfx_create_time_editor);
				Option_table_add_entry(option_table,"tracking_editor",NULL,
					command_data_void,gfx_create_tracking_editor);
				Option_table_add_entry(option_table,"volumes",NULL,
					command_data_void,gfx_create_volumes);
				Option_table_add_entry(option_table,"vt_editor",NULL,
					command_data_void,gfx_create_volume_editor);
				Option_table_add_entry(option_table,"vtexture",NULL,
					command_data_void,gfx_create_volume_texture);
				Option_table_add_entry(option_table,"window",NULL,
					command_data_void,gfx_create_window);
				Option_table_add_entry(option_table,"3d_digitizer",NULL,
					command_data_void,gfx_create_3d_digitizer);
				return_code=Option_table_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("gfx create",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_create.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_create.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_create */

static int gfx_define_faces(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 30 April 1999

DESCRIPTION :
Executes a GFX DEFINE FACES command.
==============================================================================*/
{
	int return_code;
	struct Add_FE_element_and_faces_to_manager_data *add_element_data;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group;
	struct LIST(FE_element) *element_list;
	static struct Modifier_entry option_table[]=
	{
		{"egroup",NULL,NULL,set_FE_element_group},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_define_faces);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		element_group=(struct GROUP(FE_element) *)NULL;
		(option_table[0]).to_be_modified=&element_group;
		(option_table[0]).user_data=command_data->element_group_manager;
		return_code=process_multiple_options(state,option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (element_list=CREATE(LIST(FE_element))())
			{
				if (element_group)
				{
					return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
						ensure_top_level_FE_element_is_in_list,(void *)element_list,
						element_group);
				}
				else
				{
					return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
						ensure_top_level_FE_element_is_in_list,(void *)element_list,
						command_data->element_manager);
				}
				if (return_code)
				{
					/* create user_data for add_FE_element_and_faces_to_manager, which
						 helps efficiently find existing faces to fit new element. Don't
						 forget to destroy it afterwards as it can be huge! */
					if (add_element_data=CREATE(Add_FE_element_and_faces_to_manager_data)(
						command_data->element_manager))
					{
						MANAGER_BEGIN_CACHE(FE_element)(command_data->element_manager);
						if (element_group)
						{
							MANAGED_GROUP_BEGIN_CACHE(FE_element)(element_group);
						}
						return_code=FOR_EACH_OBJECT_IN_LIST(FE_element)(
							add_FE_element_and_faces_to_manager,
							(void *)add_element_data,element_list);
						/* Destroy add_element_data without fail - it can be huge! */
						DESTROY(Add_FE_element_and_faces_to_manager_data)(
							&add_element_data);
						if (element_group)
						{
							/* make sure new faces are in element_group */
							FOR_EACH_OBJECT_IN_LIST(FE_element)(
								ensure_FE_element_and_faces_are_in_group,
								(void *)element_group,element_list);
							MANAGED_GROUP_END_CACHE(FE_element)(element_group);
						}
						MANAGER_END_CACHE(FE_element)(command_data->element_manager);
					}
				}
				DESTROY(LIST(FE_element))(&element_list);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_define_faces.  Could not create element list");
				return_code=0;
			}
		}
		if (element_group)
		{
			DEACCESS(GROUP(FE_element))(&element_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_define_faces.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_define_faces */

static int execute_command_gfx_define(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Executes a GFX DEFINE command.
==============================================================================*/
{
	int i,return_code;
	struct Cmiss_command_data *command_data;
	auto struct Modifier_entry option_table[]=
	{
		{"curve",NULL,NULL,gfx_define_Control_curve},
		{"faces",NULL,NULL,gfx_define_faces},
		{"field",NULL,NULL,define_Computed_field},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_command_gfx_define);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				i=0;
				/* curve */
				(option_table[i]).user_data=command_data->control_curve_manager;
				i++;
				/* faces */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* field */
				(option_table[i]).user_data=command_data->computed_field_package;
				i++;
				return_code=process_option(state,option_table);
			}
			else
			{
				set_command_prompt("gfx define",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_define.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_define.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_define */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_destroy_cmiss(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX DESTROY CMISS_CONNECTION command.
==============================================================================*/
{
	char *current_token;
	int return_code;

	ENTER(gfx_destroy_cmiss);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
#if defined (LINK_CMISS)
		if (CMISS)
		{
			return_code=DESTROY(CMISS_connection)(&CMISS);
		}
		else
		{
			if (!(current_token=state->current_token)||
				(strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
			{
				display_message(ERROR_MESSAGE,
					"gfx_destroy_cmiss.  No CMISS connection");
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
#else /* defined (LINK_CMISS) */
			display_message(ERROR_MESSAGE,"gfx_destroy_cmiss.  Define LINK_CMISS");
#endif /* defined (LINK_CMISS) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_cmiss.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_cmiss */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int FE_node_is_in_Multi_range_and_can_be_destroyed(struct FE_node *node,
	void *multi_range_void)
/*******************************************************************************
LAST MODIFIED : 16 April 1999

DESCRIPTION :
Returns true if the <node> cm_node_identifier is in the <multi_range> and
Fe_node_can_be_destroyed returns true for it.
==============================================================================*/
{
	int return_code;
	struct Multi_range *multi_range;

	ENTER(FE_node_is_in_Multi_range_and_can_be_destroyed);
	if (node&&(multi_range=(struct Multi_range *)multi_range_void))
	{
		return_code=FE_node_can_be_destroyed(node)&&
			Multi_range_is_value_in_range(multi_range,
				get_FE_node_cm_node_identifier(node));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_is_in_Multi_range_and_can_be_destroyed.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_is_in_Multi_range_and_can_be_destroyed */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_destroy_data(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 April 1999

DESCRIPTION :
Executes a GFX DESTROY DATA command.
==============================================================================*/
{
	int number_of_data_destroyed,return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{NULL,NULL,NULL,set_Multi_range}
	};
	struct FE_node *data_to_destroy;
	struct Multi_range *data_ranges;

	ENTER(gfx_destroy_data);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		data_ranges=CREATE(Multi_range)();
		(option_table[0]).to_be_modified=data_ranges;
		return_code=process_multiple_options(state,option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			number_of_data_destroyed=0;
			while (return_code&&(data_to_destroy=
				FIRST_OBJECT_IN_MANAGER_THAT(FE_node)(
					FE_node_is_in_Multi_range_and_can_be_destroyed,
					(void *)data_ranges,command_data->data_manager)))
			{
				if (REMOVE_OBJECT_FROM_MANAGER(FE_node)(data_to_destroy,
					command_data->data_manager))
				{
					number_of_data_destroyed++;
				}
				else
				{
					return_code=0;
				}
			}
			display_message(INFORMATION_MESSAGE,"%d data destroyed\n",
				number_of_data_destroyed);
		}
		DESTROY(Multi_range)(&data_ranges);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_data.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_data */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_destroy_data_group(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 23 April 1999

DESCRIPTION :
Executes a GFX DESTROY DGROUP command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *data_group,*node_group;

	ENTER(gfx_destroy_data_group);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					node_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
						current_token,command_data->node_group_manager);
					element_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)(
						current_token,command_data->element_group_manager);
					if (data_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
						current_token,command_data->data_group_manager))
					{
						/* must remove element_group before node group because the
							 GT_element_groups for it access the node and data groups */
						return_code=((!element_group)||
							REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_element))(element_group,
								command_data->element_group_manager))&&((!node_group)||
							REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(node_group,
								command_data->node_group_manager))&&
							REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(data_group,
								command_data->data_group_manager);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown data group: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," NAME");
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing data group name");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_data_group.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_data_group.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_data_group */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_destroy_element_group(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 23 April 1999

DESCRIPTION :
Executes a GFX DESTROY EGROUP command.
???RC.  Essentially the same code as gfx_destroy_node_group since node and
element groups are destroyed together.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *data_group,*node_group;

	ENTER(gfx_destroy_element_group);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					data_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
						current_token,command_data->data_group_manager);
					node_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
						current_token,command_data->node_group_manager);
					if (element_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),
						name)(current_token,command_data->element_group_manager))
					{
						/* must remove element_group before node group because the
							 GT_element_groups for it access the node and data groups */
						return_code=
							REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_element))(element_group,
								command_data->element_group_manager)&&((!node_group)||
							REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(node_group,
								command_data->node_group_manager))&&((!data_group)||
							REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(data_group,
								command_data->data_group_manager));
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown element group: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," NAME");
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing element group name");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_element_group.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_element_group.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_element_group */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int FE_element_is_top_level_in_Multi_range_and_can_be_destroyed(
	struct FE_element *element,void *multi_range_void)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Returns true if the <element> cm.number is in the <multi_range> and
Fe_element_can_be_destroyed returns true for it.
==============================================================================*/
{
	int return_code;
	struct Multi_range *multi_range;

	ENTER(FE_element_is_top_level_in_Multi_range_and_can_be_destroyed);
	if (element&&(multi_range=(struct Multi_range *)multi_range_void))
	{
		return_code=(CM_ELEMENT==element->cm.type)&&
			Multi_range_is_value_in_range(multi_range,element->cm.number)&&
			FE_element_can_be_destroyed(element);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_is_top_level_in_Multi_range_and_can_be_destroyed.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_is_top_level_in_Multi_range_and_can_be_destroyed */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_destroy_elements(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Executes a GFX DESTROY ELEMENTS command.
==============================================================================*/
{
	int number_of_elements_destroyed,return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{NULL,NULL,NULL,set_Multi_range}
	};
	struct FE_element *element_to_destroy;
	struct Multi_range *element_ranges;

	ENTER(gfx_destroy_elements);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		element_ranges=CREATE(Multi_range)();
		(option_table[0]).to_be_modified=element_ranges;
		return_code=process_multiple_options(state,option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			number_of_elements_destroyed=0;
			while (return_code&&(element_to_destroy=
				FIRST_OBJECT_IN_MANAGER_THAT(FE_element)(
					FE_element_is_top_level_in_Multi_range_and_can_be_destroyed,
					(void *)element_ranges,command_data->element_manager)))
			{
				if (remove_FE_element_and_faces_from_manager(element_to_destroy,
					command_data->element_manager))
				{
					number_of_elements_destroyed++;
				}
				else
				{
					return_code=0;
				}
			}
			display_message(INFORMATION_MESSAGE,"%d element(s) destroyed\n",
				number_of_elements_destroyed);
		}
		DESTROY(Multi_range)(&element_ranges);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_elements.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_elements */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_destroy_Computed_field(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Executes a GFX DESTROY FIELD command.
==============================================================================*/
{
	char *current_token;
	struct Computed_field *computed_field;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct FE_field *fe_field;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(gfx_destroy_Computed_field);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void)&&
		(computed_field_manager=Computed_field_package_get_computed_field_manager(
			command_data->computed_field_package)))
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (computed_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
					current_token,computed_field_manager))
				{
					if (Computed_field_can_be_destroyed(computed_field))
					{
						/* also want to destroy wrapped FE_field */
						fe_field=(struct FE_field *)NULL;
						switch (Computed_field_get_type(computed_field))
						{
							case COMPUTED_FIELD_FINITE_ELEMENT:
							{
								Computed_field_get_type_finite_element(computed_field,
									&fe_field);
							}
						}
						if (return_code=REMOVE_OBJECT_FROM_MANAGER(Computed_field)(
							computed_field,computed_field_manager))
						{
							if (fe_field)
							{
								return_code=REMOVE_OBJECT_FROM_MANAGER(FE_field)(
									fe_field,command_data->fe_field_manager);
							}
						}
						if (!return_code)
						{
							display_message(ERROR_MESSAGE,
								"gfx_destroy_Computed_field.  Could not destroy field");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cannot destroy field in use : %s",current_token);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Field does not exist: %s",
						current_token);
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," FIELD_NAME");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing field name");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_destroy_Computed_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_Computed_field */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int Scene_remove_graphics_object_iterator(struct Scene *scene,
	void *graphics_object_void)
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Removes all instances of the <graphics_object> from <scene>.
???RC Move to scene.c?
==============================================================================*/
{
	int return_code;
	struct GT_object *graphics_object;

	ENTER(Scene_remove_graphics_object_iterator);
	if (scene&&(graphics_object=(struct GT_object *)graphics_object_void))
	{
		while (Scene_has_graphics_object(scene,graphics_object))
		{
			Scene_remove_graphics_object(scene,graphics_object);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_remove_graphics_object_iterator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_remove_graphics_object_iterator */

static int gfx_destroy_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Executes a GFX DESTROY GRAPHICS_OBJECT command.
==============================================================================*/
{
	char *current_token;
	gtObject *graphics_object;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_destroy_graphics_object);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
						current_token,command_data->graphics_object_list))
					{
						/* remove all instances of the graphics object from all scenes */
						FOR_EACH_OBJECT_IN_MANAGER(Scene)(
							Scene_remove_graphics_object_iterator,(void *)graphics_object,
							command_data->scene_manager);
						/* remove graphics object from the global list. Object is destroyed
							 when deaccessed by list */
						REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list);
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"Graphics object does not exist: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," GRAPHICS_OBJECT_NAME");
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing graphics object name");
				return_code=0;
#if defined (OLD_CODE)
				/* destroy all objects in list */
				return_code=FOR_EACH_OBJECT_IN_LIST(GT_object)(
					remove_graphics_object_from_all_windows,(void *)NULL,list);
				REMOVE_ALL_OBJECTS_FROM_LIST(GT_object)(list);
					/*???DB.  Assuming that don't need destroy because of access_count */
				return_code=1;
#endif /* defined (OLD_CODE) */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_graphics_object.  Missing graphics_object_list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_destroy_graphics_object.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_graphics_object */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_destroy_node_group(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 23 April 1999

DESCRIPTION :
Executes a GFX DESTROY NGROUP command.
???RC.  Essentially the same code as gfx_destroy_element_group since node and
element groups are destroyed together.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *data_group,*node_group;

	ENTER(gfx_destroy_node_group);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					data_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
						current_token,command_data->data_group_manager);
					element_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)(
						current_token,command_data->element_group_manager);
					if (node_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
						current_token,command_data->node_group_manager))
					{
						/* must remove element_group before node group because the
							 GT_element_groups for it access the node and data groups */
						return_code=((!element_group)||
							REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_element))(element_group,
								command_data->element_group_manager))&&
							REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(node_group,
								command_data->node_group_manager)&&((!data_group)||
							REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(data_group,
								command_data->data_group_manager));
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown node group: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," NAME");
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing node group name");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_node_group.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_node_group.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_node_group */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_destroy_nodes(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 April 1999

DESCRIPTION :
Executes a GFX DESTROY NODES command.
==============================================================================*/
{
	int number_of_nodes_destroyed,return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{NULL,NULL,NULL,set_Multi_range}
	};
	struct FE_node *node_to_destroy;
	struct Multi_range *node_ranges;

	ENTER(gfx_destroy_nodes);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		node_ranges=CREATE(Multi_range)();
		(option_table[0]).to_be_modified=node_ranges;
		return_code=process_multiple_options(state,option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			number_of_nodes_destroyed=0;
			while (return_code&&(node_to_destroy=
				FIRST_OBJECT_IN_MANAGER_THAT(FE_node)(
					FE_node_is_in_Multi_range_and_can_be_destroyed,
					(void *)node_ranges,command_data->node_manager)))
			{
				if (REMOVE_OBJECT_FROM_MANAGER(FE_node)(node_to_destroy,
					command_data->node_manager))
				{
					number_of_nodes_destroyed++;
				}
				else
				{
					return_code=0;
				}
			}
			display_message(INFORMATION_MESSAGE,"%d node(s) destroyed\n",
				number_of_nodes_destroyed);
		}
		DESTROY(Multi_range)(&node_ranges);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_nodes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_nodes */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_destroy_vtextures(struct Parse_state *state,
	void *dummy_to_be_modified,void *volume_texture_manager_void)
/*******************************************************************************
LAST MODIFIED : 7 October 1996

DESCRIPTION :
Executes a GFX DESTROY VTEXTURES command.
???DB.  Could merge with destroy_graphics_objects if graphics_objects used the
	new list structures.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct MANAGER(VT_volume_texture) *volume_texture_manager;
	struct VT_volume_texture *volume_texture;

	ENTER(gfx_destroy_vtextures);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (volume_texture_manager=
			(struct MANAGER(VT_volume_texture) *)volume_texture_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (volume_texture=FIND_BY_IDENTIFIER_IN_MANAGER(VT_volume_texture,
						name)(current_token,volume_texture_manager))
					{
						/* remove object from list (destroys automatically) */
						return_code=REMOVE_OBJECT_FROM_MANAGER(VT_volume_texture)(
							volume_texture,volume_texture_manager);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Volume texture does not exist: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," <NAME[all]>");
					return_code=1;
				}
			}
			else
			{
				/* destroy all objects in list */
				return_code=REMOVE_ALL_OBJECTS_FROM_MANAGER(VT_volume_texture)(
					volume_texture_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_vtextures.  Missing volume texture manager");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_vtextures.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_vtextures */

#if defined (OLD_CODE)
static int gfx_destroy_vtextures(struct Parse_state *state,
	void *dummy_to_be_modified,void *volume_texture_manager_void)
/*******************************************************************************
LAST MODIFIED : 27 September 1996

DESCRIPTION :
Executes a GFX DESTROY VTEXTURES command.
???DB.  Could merge with destroy_graphics_objects if graphics_objects used the
	new list structures.
==============================================================================*/
{
	char *name;
	int return_code;
	struct MANAGER(VT_volume_texture) *volume_texture_manager;
	struct Modifier_entry option_table[]=
	{
		{"name",NULL,(void *)1,set_name},
		{NULL,NULL,NULL,NULL}
	};
	struct VT_volume_texture *volume_texture;

	ENTER(gfx_destroy_vtextures);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (volume_texture_manager=
			(struct MANAGER(VT_volume_texture) *)volume_texture_manager_void)
		{
			/* initialize defaults */
			name=(char *)NULL;
			(option_table[0]).to_be_modified = &name;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (name)
				{
					if (volume_texture=FIND_BY_IDENTIFIER_IN_MANAGER(VT_volume_texture,
						name)(name,volume_texture_manager))
					{
						/* remove object from list (destroys automatically) */
						return_code=REMOVE_OBJECT_FROM_MANAGER(VT_volume_texture)(
							volume_texture,volume_texture_manager);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Volume texture does not exist: %s",
							name);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					/* destroy all objects in list */
					return_code=REMOVE_ALL_OBJECTS_FROM_MANAGER(VT_volume_texture)(
						volume_texture_manager);
				}
			} /* parse error,help */
			if (name)
			{
				DEALLOCATE(name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_vtextures.  Missing volume texture manager");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_vtextures.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_vtextures */
#endif /* defined (OLD_CODE) */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_destroy(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Executes a GFX DESTROY command.
==============================================================================*/
{
	int i,return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"cmiss_connection",NULL,NULL,gfx_destroy_cmiss},
		{"curve",NULL,NULL,gfx_destroy_Control_curve},
		{"data",NULL,NULL,gfx_destroy_data},
		{"dgroup",NULL,NULL,gfx_destroy_data_group},
		{"egroup",NULL,NULL,gfx_destroy_element_group},
		{"elements",NULL,NULL,gfx_destroy_elements},
		{"field",NULL,NULL,gfx_destroy_Computed_field},
		{"graphics_object",NULL,NULL,gfx_destroy_graphics_object},
		{"ngroup",NULL,NULL,gfx_destroy_node_group},
		{"nodes",NULL,NULL,gfx_destroy_nodes},
		{"spectrum",NULL,NULL,gfx_destroy_spectrum},
		{"vtextures",NULL,NULL,gfx_destroy_vtextures},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_command_gfx_destroy);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				i=0;
				/* cmiss_connection */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* curve */
				(option_table[i]).user_data=command_data->control_curve_manager;
				i++;
				/* data */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* dgroup */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* egroup */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* elements */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* field */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* graphics_object */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* ngroup */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* nodes */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* spectrum */
				(option_table[i]).user_data=command_data->spectrum_manager;
				i++;
				/* vtextures */
				(option_table[i]).user_data=command_data->volume_texture_manager;
				i++;
				return_code=process_option(state,option_table);
			}
			else
			{
				set_command_prompt("gfx destroy",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_destroy.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_destroy.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_destroy */
#endif /* !defined (WINDOWS_DEV_FLAG) */

struct Scene_add_graphics_object_iterator_data
{
	struct Scene *scene;
};

#if !defined (WINDOWS_DEV_FLAG)
static int Scene_add_graphics_object_iterator(struct GT_object *graphics_object,
	void *data_void)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Scene *scene;
	struct Scene_add_graphics_object_iterator_data *data;

	ENTER(Scene_add_graphics_object_iterator);
	return_code=0;
	/* check arguments */
	if (graphics_object&&
		(data=(struct Scene_add_graphics_object_iterator_data *)data_void)&&
		(scene=data->scene))
	{
		if (Scene_has_graphics_object(scene,graphics_object))
		{
			return_code=1;
		}
		else
		{
			return_code=Scene_add_graphics_object(scene,graphics_object,0,
				graphics_object->name);
		}
		if (1<GT_object_get_number_of_times(graphics_object))
		{
			Scene_update_time_behaviour(data->scene,graphics_object);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_add_graphics_object_iterator.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Scene_add_graphics_object_iterator */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_draw(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 April 2000

DESCRIPTION :
Executes a GFX DRAW command.
==============================================================================*/
{
	char *scene_object_name,*time_object_name;
	struct GT_object *graphics_object;
	int return_code,position;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group;
	struct Scene *child_scene,*scene;
	struct Scene_add_graphics_object_iterator_data data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_draw);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			graphics_object=(struct GT_object *)NULL;
			element_group=(struct GROUP(FE_element) *)NULL;
			scene_object_name=(char *)NULL;
			time_object_name=(char *)NULL;
			position=0;
			scene=ACCESS(Scene)(command_data->default_scene);
			child_scene=(struct Scene *)NULL;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&scene_object_name,
				(void *)1,set_name);
			/* child_scene */
			Option_table_add_entry(option_table,"child_scene",&child_scene,
				command_data->scene_manager,set_Scene);
			/* graphics_object */
			Option_table_add_entry(option_table,"graphics_object",&graphics_object,
				command_data->graphics_object_list,set_Graphics_object);
			/* group */
			Option_table_add_entry(option_table,"group",&element_group,
				command_data->element_group_manager,set_FE_element_group);
			/* position */
			Option_table_add_entry(option_table,"position",&position,
				(void *)1,set_int);
			/* scene */
			Option_table_add_entry(option_table,"scene",&scene,
				command_data->scene_manager,set_Scene);
			/* time_object */
			Option_table_add_entry(option_table,"time_object",&time_object_name,
				(void *)1,set_name);
			/* default when token omitted (graphics_object) */
			Option_table_add_entry(option_table,(char *)NULL,&graphics_object,
				command_data->graphics_object_list,set_Graphics_object);
			return_code=Option_table_multi_parse(option_table,state);
			if ((child_scene&&graphics_object) || (graphics_object&&element_group) ||
				(element_group&&child_scene))
			{
				display_message(ERROR_MESSAGE,"execute_command_gfx_draw.  "
					"Specify only one of child_scene|graphics_object|group");
				return_code=0;
			}
			if (child_scene&&time_object_name)
			{
				display_message(ERROR_MESSAGE,"execute_command_gfx_draw.  "
					"Time objects may not be associated with a child_scene");
				return_code=0;
			}
			/* no errors, not asking for help */
			if (return_code)
			{
				data.scene=scene;
				if (graphics_object)
				{
					if (scene)
					{
						if (!scene_object_name)
						{
							ALLOCATE(scene_object_name,char,
								strlen(graphics_object->name)+1);
							strcpy(scene_object_name,graphics_object->name);
						}
						return_code=Scene_add_graphics_object(scene,graphics_object,0,
							scene_object_name);
						if (time_object_name)
						{
							/* SAB A new time_object is created and associated with the named
								 scene_object, the time_keeper is supplied so that the
							    default could be overridden */
							Scene_set_time_behaviour(scene,scene_object_name, time_object_name,
								command_data->default_time_keeper);
						}
						if (1<GT_object_get_number_of_times(graphics_object))
						{
							/* any scene_objects referring to this graphics_object which do
								 not already have a time_object all are associated with a
								 single common time_object */
							Scene_update_time_behaviour(scene,graphics_object);
						}
					}
				}
				else if (child_scene)
				{
					if (!scene_object_name)
					{
						GET_NAME(Scene)(child_scene,&scene_object_name);
					}
					return_code=Scene_add_child_scene(scene,child_scene,0,
						scene_object_name,command_data->scene_manager);
				}
				else if (element_group)
				{
					return_code=Scene_add_graphical_finite_element(scene,element_group,
						scene_object_name);
				}
				else
				{
					return_code=FOR_EACH_OBJECT_IN_LIST(GT_object)(
						Scene_add_graphics_object_iterator,(void *)&data,
						command_data->graphics_object_list);
				}
			} /* parse error,help */
			DESTROY(Option_table)(&option_table);
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
			if (scene)
			{
				DEACCESS(Scene)(&scene);
			}
			if (child_scene)
			{
				DEACCESS(Scene)(&child_scene);
			}
			if (graphics_object)
			{
				DEACCESS(GT_object)(&graphics_object);
			}
			if (scene_object_name)
			{
				DEALLOCATE(scene_object_name);
			}
			if (time_object_name)
			{
				DEALLOCATE(time_object_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_draw.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_draw.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_draw */
#endif /* !defined (WINDOWS_DEV_FLAG) */

struct Edit_graphics_object_data
{
	char *graphics_object_name;
	struct Scene *scene;
	struct Scene_object *scene_object;
}; /* struct Edit_graphics_object_data */

#if !defined (WINDOWS_DEV_FLAG)
static void edit_graphics_object(Widget widget,void *user_data,void *call_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Callback from transformation editor dialog when transformation changes.
==============================================================================*/
{
	gtMatrix transformation_matrix;
	struct Cmgui_coordinate *new_transformation;
	struct Edit_graphics_object_data *edit_graphics_object_data;
	struct Scene_object *scene_object;

	ENTER(edit_graphics_object);
	USE_PARAMETER(widget);
	if ((edit_graphics_object_data=
		(struct Edit_graphics_object_data *)user_data)&&
		(scene_object=edit_graphics_object_data->scene_object)&&
		(new_transformation=(struct Cmgui_coordinate *)call_data))
	{
#if defined (DEBUG)
		/*???debug */
		printf("%s\n",new_transformation->name);
		printf("position : %g %g %g\n",
			((new_transformation->origin).position.data)[0],
			((new_transformation->origin).position.data)[1],
			((new_transformation->origin).position.data)[2]);
		printf("direction : %g %g %g\n",
			((new_transformation->origin).direction.data)[0],
			((new_transformation->origin).direction.data)[1],
			((new_transformation->origin).direction.data)[2]);
#endif /* defined (DEBUG) */
#if defined (GL_API)
		loadmatrix(identity_transform_matrix);
		translate(((new_transformation->origin).position.data)[0],
			((new_transformation->origin).position.data)[1],
			((new_transformation->origin).position.data)[2]);
#if defined (OLD_CODE)
/*???GMH.  Testing out transformations */
		rot(-((new_transformation->origin).direction.data)[2],'z');
		rot(((new_transformation->origin).direction.data)[1],'x');
		rot(-((new_transformation->origin).direction.data)[0],'y');
#endif /* defined (OLD_CODE) */
		rot(((new_transformation->origin).direction.data)[0],'z');
		rot(((new_transformation->origin).direction.data)[1],'y');
		rot(((new_transformation->origin).direction.data)[2],'x');
		getmatrix(transformation_matrix);
#endif /* defined (GL_API) */
#if defined (OPENGL_API)
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(((new_transformation->origin).position.data)[0],
			((new_transformation->origin).position.data)[1],
			((new_transformation->origin).position.data)[2]);
#if defined (OLD_CODE)
/*???GMH.  Testing out transformations */
		glRotatef(-((new_transformation->origin).direction.data)[2],0.,0.,1.);
		glRotatef(((new_transformation->origin).direction.data)[1],1.,0.,0.);
		glRotatef(-((new_transformation->origin).direction.data)[0],0.,1.,0.);
#endif /* defined (OLD_CODE) */
		glRotatef(((new_transformation->origin).direction.data)[0],0.,0.,1.);
		glRotatef(((new_transformation->origin).direction.data)[1],0.,1.,0.);
		glRotatef(((new_transformation->origin).direction.data)[2],1.,0.,0.);
		wrapperReadMatrix(GL_MODELVIEW_MATRIX,&(transformation_matrix));
#endif /* defined (OPENGL_API) */
		Scene_object_set_transformation(scene_object, &transformation_matrix);
	}
	else
	{
		display_message(ERROR_MESSAGE,"edit_graphics_object.  Invalid argument(s)");
#if defined (DEBUG)
		/*???debug */
		printf("user_data=%p call_data=%p\n",user_data,call_data);
		edit_graphics_object_data=
			(struct Edit_graphics_object_data *)user_data;
		printf("gra=%p\n",edit_graphics_object_data->scene_object);
#endif /* defined (DEBUG) */
	}
	LEAVE;
} /* edit_graphics_object */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static void end_edit_graphics_object(Widget widget,void *user_data,
	void *call_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Callback for when transformation editor for gfx edit graphics_object is closed.
Cleans up <user_data> == edit_graphics_object_data.
==============================================================================*/
{
	struct Edit_graphics_object_data *edit_graphics_object_data;

	ENTER(end_edit_graphics_object);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((edit_graphics_object_data=
		(struct Edit_graphics_object_data *)user_data))
	{
		if (edit_graphics_object_data->graphics_object_name)
		{
			DEALLOCATE(edit_graphics_object_data->graphics_object_name);
		}
		/* deaccess graphics object and scene - see gfx_edit_graphics_object */
		DEACCESS(Scene_object)(&edit_graphics_object_data->scene_object);
		DEACCESS(Scene)(&edit_graphics_object_data->scene);
		DEALLOCATE(edit_graphics_object_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"end_edit_graphics_object.  Invalid argument(s)");
	}
	LEAVE;
} /* end_edit_graphics_object */
#endif /* !defined (WINDOWS_DEV_FLAG) */

struct Apply_transformation_data
{
	gtMatrix *transformation;
	struct MANAGER(FE_node) *node_manager;
}; /* struct Apply_transformation_data */

static int apply_transformation_to_node(struct FE_node *node,
	void *apply_transformation_data)
/*******************************************************************************
LAST MODIFIED : 6 May 1999

DESCRIPTION :
Iterator that modifies the position of each node according to the 
transformation in the transformation data.
==============================================================================*/
{
	FE_value x, x2, y, y2, z, z2, h2;
	gtMatrix *transformation;
	int return_code;
	struct Apply_transformation_data *data;
	struct FE_node *copy_node;
	struct FE_field *coordinate_field;

	ENTER(apply_transformation_to_node);
	if(node&&(data=(struct Apply_transformation_data *)apply_transformation_data)
		&&(transformation=data->transformation))
	{
		if (coordinate_field=FE_node_get_position_cartesian(node,
			(struct FE_field *)NULL,&x,&y,&z,(FE_value *)NULL))
		{
			/* Get the new position */
			h2 = (*transformation)[0][3] * x
				+ (*transformation)[1][3] * y
				+ (*transformation)[2][3] * z
				+ (*transformation)[3][3];
			x2 = ((*transformation)[0][0] * x
				+ (*transformation)[1][0] * y
				+ (*transformation)[2][0] * z
				+ (*transformation)[3][0]) / h2;
			y2 = ((*transformation)[0][1] * x
				+ (*transformation)[1][1] * y
				+ (*transformation)[2][1] * z
				+ (*transformation)[3][1]) / h2;
			z2 = ((*transformation)[0][2] * x
				+ (*transformation)[1][2] * y
				+ (*transformation)[2][2] * z
				+ (*transformation)[3][2]) / h2;

			/* create a copy of the node: */
			if ((copy_node=CREATE(FE_node)(0,(struct FE_node *)NULL))&&
				COPY(FE_node)(copy_node,node))
			{
				if (FE_node_set_position_cartesian(copy_node,coordinate_field,x2,y2,z2))
				{
					return_code = MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)(
						node,copy_node,data->node_manager);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_node_translate.  Could not make move node");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_node_translate.  Could not make copy of node");
				return_code = 0;
			}
			DESTROY(FE_node)(&copy_node);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_node_translate.  Could not calculate coordinate field");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"end_edit_graphics_object.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_edit_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EDIT GRAPHICS_OBJECT command.
==============================================================================*/
{
	char apply_flag,*graphics_object_name;
	int return_code;
	struct Callback_data callback,*callback_data;
	struct Cmiss_command_data *command_data;
	struct Cmgui_coordinate initial_transformation;
	struct Edit_graphics_object_data *edit_graphics_object_data;
	struct Scene_object *scene_object;
	struct Scene *scene;
	static struct Modifier_entry option_table[]=
	{
		{"apply_transformation",NULL,NULL,set_char_flag},
		{"name",NULL,(void *)1,set_name},
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,set_name}
	};

	ENTER(gfx_edit_graphics_object);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			apply_flag=0;
			graphics_object_name=(char *)NULL;
			scene=ACCESS(Scene)(command_data->default_scene);
			(option_table[0]).to_be_modified = &apply_flag;
			(option_table[1]).to_be_modified = &graphics_object_name;
			(option_table[2]).to_be_modified= &scene;
			(option_table[2]).user_data=command_data->scene_manager;
			(option_table[3]).to_be_modified= &graphics_object_name;
			return_code=process_multiple_options(state,option_table);
			if (return_code)
			{
				if (scene&&graphics_object_name&&(scene_object=
					Scene_get_scene_object_by_name(
					scene,graphics_object_name)))
				{
					if (apply_flag)
					{							
						/* SAB Temporary place for this command cause I really
							need to use it, not very general, doesn't work in prolate or
							rotate derivatives */
						struct Apply_transformation_data data;
						struct GT_element_group *gt_element_group;
						struct GROUP(FE_node) *data_group, *node_group;
						gtMatrix identity={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
							transformation;

						if (Scene_object_has_transformation(scene_object)&&
							Scene_object_get_transformation(scene_object,
								&transformation))
						{
							if(Scene_object_has_graphical_element_group(scene_object,NULL) &&
								(gt_element_group = 
									Scene_object_get_graphical_element_group(scene_object)))
							{
								data.transformation = &transformation;
								if (node_group=GT_element_group_get_node_group(
									gt_element_group))
								{
									MANAGER_BEGIN_CACHE(FE_node)(command_data->node_manager);
									data.node_manager = command_data->node_manager;
									FOR_EACH_OBJECT_IN_GROUP(FE_node)
										(apply_transformation_to_node, 
										(void *)&data, node_group);
									MANAGER_END_CACHE(FE_node)(command_data->node_manager);
								}
								else
								{
									display_message(WARNING_MESSAGE,
										"Could not get GT_element_group");
									return_code=0;
								}
								if (data_group=GT_element_group_get_data_group(
									gt_element_group))
								{
									MANAGER_BEGIN_CACHE(FE_node)(command_data->data_manager);
									data.node_manager = command_data->data_manager;
									FOR_EACH_OBJECT_IN_GROUP(FE_node)
										(apply_transformation_to_node, 
										(void *)&data, data_group);
									MANAGER_END_CACHE(FE_node)(command_data->data_manager);
								}
								else
								{
									display_message(WARNING_MESSAGE,
										"Could not get GT_element_group");
									return_code=0;
								}
								Scene_object_set_transformation(scene_object,
									&identity);
							}
						}
						else
						{
							return_code = 1;
						}
					}
					else
					{
						initial_transformation.name=graphics_object_name;
						initial_transformation.access_count=1;
						/*???DB.  Temp.  Need to be able to convert between gtMatrix and
						  position/direction */
						(initial_transformation.origin.position.data)[0]=0;
						(initial_transformation.origin.position.data)[1]=0;
						(initial_transformation.origin.position.data)[2]=0;
						(initial_transformation.origin.direction.data)[0]=0;
						(initial_transformation.origin.direction.data)[1]=0;
						(initial_transformation.origin.direction.data)[2]=0;
						bring_up_transformation_editor_dialog(
							&(command_data->transformation_editor_dialog),
							command_data->user_interface->application_shell,
							&initial_transformation,command_data->user_interface);
						edit_graphics_object_data=(struct Edit_graphics_object_data *)NULL;
						if ((callback_data=(struct Callback_data *)
							transformation_editor_dialog_get_data(
								command_data->transformation_editor_dialog,
								TRANSFORMATION_EDITOR_DIALOG_DESTROY_CB))&&
							(edit_graphics_object_data=
								(struct Edit_graphics_object_data *)callback_data->data))
						{
							/* deaccess graphics object and scene for existing editor */
							DEACCESS(Scene_object)(&edit_graphics_object_data->scene_object);
							DEACCESS(Scene)(&edit_graphics_object_data->scene);
						}
						/* create data to be passed to transformation editor callbacks */
						if (edit_graphics_object_data||ALLOCATE(edit_graphics_object_data,
							struct Edit_graphics_object_data,1))
						{
							/* access graphics object and scene in case either destroyed while
								editing tranformation */
							edit_graphics_object_data->scene_object=
								ACCESS(Scene_object)(scene_object);
							edit_graphics_object_data->scene=ACCESS(Scene)(scene);
							edit_graphics_object_data->graphics_object_name =
								graphics_object_name;
							/* set callback for when transformation changes */
							callback.data=(void *)edit_graphics_object_data;
							callback.procedure=edit_graphics_object;
							transformation_editor_dialog_set_data(
								command_data->transformation_editor_dialog,
								TRANSFORMATION_EDITOR_DIALOG_UPDATE_CB,&callback);
							/* set callback for when transformation editor is closed */
							callback.procedure=end_edit_graphics_object;
							transformation_editor_dialog_set_data(
								command_data->transformation_editor_dialog,
								TRANSFORMATION_EDITOR_DIALOG_DESTROY_CB,&callback);
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"No callbacks from transformation editor");
							return_code=0;
						}
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"Must specify name of graphics object in scene");
					return_code=0;
				}
			}
			DEACCESS(Scene)(&scene);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_edit_graphics_object.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_edit_graphics_object.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_edit_graphics_object */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_edit_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EDIT SPECTRUM command.
Invokes the graphical spectrum group editor.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Spectrum *spectrum;
	static struct Modifier_entry option_table[]=
	{
		{NULL,NULL,NULL,set_Spectrum},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_edit_spectrum);
	USE_PARAMETER(dummy_to_be_modified);
	/* check arguments */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			spectrum=(struct Spectrum *)NULL;
			(option_table[0]).to_be_modified= &spectrum;
			(option_table[0]).user_data=command_data->spectrum_manager;
			if (return_code=process_multiple_options(state,option_table))
			{
				return_code=bring_up_spectrum_editor_dialog(
					&(command_data->spectrum_editor_dialog),
					command_data->user_interface->application_shell,
					command_data->spectrum_manager,spectrum,command_data->user_interface,
					command_data->glyph_list,
					command_data->graphical_material_manager,command_data->light_manager,
					command_data->texture_manager,command_data->scene_manager);
			} /* parse error, help */
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_edit_spectrum.  "
				"Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_edit_spectrum.  "
			"Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_edit_spectrum */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_edit(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 2 August 1998

DESCRIPTION :
Executes a GFX EDIT command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"graphics_object",NULL,NULL,gfx_edit_graphics_object},
		{"spectrum",NULL,NULL,gfx_edit_spectrum},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_command_gfx_edit);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				(option_table[0]).user_data=command_data_void;
				(option_table[1]).user_data=command_data_void;
				return_code=process_option(state,option_table);
			}
			else
			{
				set_command_prompt("gfx edit",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_edit.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_edit.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_edit */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int execute_command_gfx_element_creator(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Executes a GFX ELEMENT_CREATOR command.
==============================================================================*/
{
	char *group_name;
	int element_dimension,return_code;
	struct Cmiss_command_data *command_data;
	struct Element_creator *element_creator;
	struct FE_field *coordinate_field;
	struct GROUP(FE_element) *element_group,*old_element_group;
	struct GROUP(FE_node) *node_group,*old_node_group;
	struct Option_table *option_table;
	struct Set_FE_field_conditional_data set_coordinate_field_data;

	ENTER(execute_command_gfx_element_creator);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		/* initialize defaults */
		element_group=(struct GROUP(FE_element) *)NULL;
		node_group=(struct GROUP(FE_node) *)NULL;
		if (element_creator=command_data->element_creator)
		{
			coordinate_field=Element_creator_get_coordinate_field(element_creator);
			element_dimension=Element_creator_get_element_dimension(element_creator);
			Element_creator_get_groups(element_creator,&element_group,&node_group);
		}
		else
		{
			coordinate_field=(struct FE_field *)NULL;
			element_dimension=2;
		}
		if (coordinate_field)
		{
			ACCESS(FE_field)(coordinate_field);
		}
		if (element_group)
		{
			ACCESS(GROUP(FE_element))(element_group);
		}
		old_element_group=element_group;
		old_node_group=node_group;

		option_table=CREATE(Option_table)();
		/* coordinate_field */
		set_coordinate_field_data.fe_field_manager=command_data->fe_field_manager;
		set_coordinate_field_data.conditional_function=FE_field_is_coordinate_field;
		set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"coordinate_field",
			&coordinate_field,&set_coordinate_field_data,set_FE_field_conditional);
		/* dimension */
		Option_table_add_entry(option_table,"dimension",
			&element_dimension,NULL,set_int_non_negative);
		/* group */
		Option_table_add_entry(option_table,"group",&element_group,
			command_data->element_group_manager,set_FE_element_group);
		if (return_code=Option_table_multi_parse(option_table,state))
		{
			if (element_creator)
			{
				Element_creator_set_coordinate_field(element_creator,coordinate_field);
				Element_creator_set_element_dimension(element_creator,
					element_dimension);
				if (element_group)
				{
					if (element_group != old_element_group)
					{
						if (GET_NAME(GROUP(FE_element))(element_group,&group_name))
						{
							if (!(node_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),
								name)(group_name,command_data->node_group_manager)))
							{
								node_group=old_node_group;
							}
							DEALLOCATE(group_name);
						}
					}
					Element_creator_set_groups(element_creator,element_group,node_group);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Must create element_creator before modifying it");
				return_code=0;
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
		if (element_group)
		{
			DEACCESS(GROUP(FE_element))(&element_group);
		}
		if (coordinate_field)
		{
			DEACCESS(FE_field)(&coordinate_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_element_creator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_element_creator */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_erase(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX ERASE command.
==============================================================================*/
{
	char *child_scene_name;
	int i,return_code;
	struct Cmiss_command_data *command_data;
	struct GT_object *graphics_object;
	struct Scene *child_scene,*scene;
	static struct Modifier_entry option_table[]=
	{
		{"child_scene",NULL,NULL,set_Scene},
		{"graphics_object",NULL,NULL,set_Graphics_object},
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,set_Graphics_object}
	};

	ENTER(execute_command_gfx_erase);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			child_scene=(struct Scene *)NULL;
			graphics_object=(struct GT_object *)NULL;
			scene=ACCESS(Scene)(command_data->default_scene);
			i=0;
			/* child_scene */
			(option_table[i]).to_be_modified= &child_scene;
			(option_table[i]).user_data=command_data->scene_manager;
			i++;
			/* graphics_object */
			(option_table[i]).to_be_modified= &graphics_object;
			(option_table[i]).user_data=command_data->graphics_object_list;
			i++;
			/* scene */
			(option_table[i]).to_be_modified= &scene;
			(option_table[i]).user_data=command_data->scene_manager;
			i++;
			/* default (graphics_object) */
			(option_table[i]).to_be_modified= &graphics_object;
			(option_table[i]).user_data=command_data->graphics_object_list;
			i++;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (scene&&(graphics_object||child_scene))
				{
					if (graphics_object)
					{
						if (Scene_has_graphics_object(scene,graphics_object))
						{
							if (!Scene_remove_graphics_object(scene,graphics_object))
							{
								return_code=0;
							}
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"No graphics_object named '%s' in scene",graphics_object->name);
							return_code=0;
						}
					}
					if (child_scene)
					{
						if (Scene_has_child_scene(scene,child_scene))
						{
							if (!Scene_remove_child_scene(scene,child_scene))
							{
								return_code=0;
							}
						}
						else
						{
							GET_NAME(Scene)(child_scene,&child_scene_name);
							display_message(WARNING_MESSAGE,
								"No child_scene named '%s' in scene",child_scene_name);
							DEALLOCATE(child_scene_name);
							return_code=0;
						}
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"Must specify scene, and graphics_object or child_scene to erase");
					return_code=0;
				}
			} /* parse error,help */
			if (child_scene)
			{
				DEACCESS(Scene)(&child_scene);
			}
			if (graphics_object)
			{
				DEACCESS(GT_object)(&graphics_object);
			}
			if (scene)
			{
				DEACCESS(Scene)(&scene);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_erase.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_erase.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_erase */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_export_alias(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EXPORT ALIAS command.
==============================================================================*/
{
#if defined (NEW_ALIAS)
	char destroy_when_saved,*default_filename="cmgui_wire",*file_name,
		*retrieve_filename,save_now,write_sdl;
	float frame_in,frame_out,view_frame;
#endif /* defined (NEW_ALIAS) */
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	static struct Modifier_entry option_table[]=
	{
#if defined (NEW_ALIAS)
		{"dont_save_now",NULL,NULL,unset_char_flag},
		{"frame_in",NULL,NULL,set_float},
		{"frame_out",NULL,NULL,set_float},
		{"keep",NULL,NULL,unset_char_flag},
		{"retrieve",NULL,NULL,set_file_name},
#endif /* defined (NEW_ALIAS) */
		{"scene",NULL,NULL,set_Scene},
#if defined (NEW_ALIAS)
		{"sdl",NULL,NULL,set_char_flag},
		{"viewframe",NULL,NULL,set_float},
		{NULL,NULL,NULL,set_file_name}
#else /* defined (NEW_ALIAS) */
		{NULL,NULL,NULL,NULL}
#endif /* defined (NEW_ALIAS) */
	};

	ENTER(gfx_export_alias);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
#if defined (NEW_ALIAS)
			file_name=(char *)NULL;
			frame_in=0;
			frame_out=0;
			retrieve_filename=(char *)NULL;
			save_now=1;
			view_frame=0;
			write_sdl=0;
			destroy_when_saved=1;
#endif /* defined (NEW_ALIAS) */
			scene=command_data->default_scene;
#if defined (NEW_ALIAS)
			(option_table[0]).to_be_modified= &save_now;
			(option_table[1]).to_be_modified= &frame_in;
			(option_table[2]).to_be_modified= &frame_out;
			(option_table[3]).to_be_modified= &destroy_when_saved;
			(option_table[4]).to_be_modified= &retrieve_filename;
			(option_table[5]).to_be_modified= &scene;
			(option_table[5]).user_data=command_data->scene_manager;
			(option_table[6]).to_be_modified= &write_sdl;
			(option_table[7]).to_be_modified= &view_frame;
			(option_table[8]).to_be_modified= &file_name;
#else /* defined (NEW_ALIAS) */
			(option_table[0]).to_be_modified= &scene;
			(option_table[0]).user_data=command_data->scene_manager;
#endif /* defined (NEW_ALIAS) */
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (scene)
				{
#if defined (NEW_ALIAS)
					if (!file_name)
					{
						file_name=default_filename;
					}
					if (write_sdl)
					{
						export_to_alias_sdl(scene,file_name,retrieve_filename,view_frame);
					}
					else
					{
						export_to_alias_frames(scene,file_name,frame_in,frame_out,save_now,
							destroy_when_saved);
					}
#else /* defined (NEW_ALIAS) */
					display_message(ERROR_MESSAGE,"gfx_export_alias.  The old gfx export alias is superseeded by gfx export wavefront");
					return_code=0;
#endif /* defined (NEW_ALIAS) */
				}
			} /* parse error,help */
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_export_alias.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_alias.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_alias */
#endif /* !defined (WINDOWS_DEV_FLAG) */

int gfx_export_node(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 November 1998

DESCRIPTION :
Executes a GFX EXPORT NODE command.  This command exports nodes to cmiss as
data.
==============================================================================*/
{
	int base_number,return_code;
	struct Cmiss_command_data *command_data;
	struct LIST(GROUP(FE_node)) *groups;
	static struct Modifier_entry option_table[]=
	{
		{"base_number",NULL,NULL,set_int},
		{"groups",NULL,NULL,set_FE_node_group_list},
		{NULL,NULL,NULL}
	};

	ENTER(gfx_export_node);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			groups=CREATE(LIST(GROUP(FE_node)))();
			base_number=1;
			/* initialise defaults */
			(option_table[0]).to_be_modified= &base_number;
			(option_table[1]).to_be_modified=groups;
			(option_table[1]).user_data=command_data->node_group_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				export_nodes(groups,base_number,command_data->execute_command);
			} /* parse error, help */
			DESTROY(LIST(GROUP(FE_node)))(&groups);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_export_node.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_node.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_node */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_export_vrml(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EXPORT VRML command.
==============================================================================*/
{
	char *file_name;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"file",NULL,(void *)1,set_name},
		{"scene",NULL,NULL,set_Scene_including_sub_objects},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Scene *scene;

	ENTER(gfx_export_vrml);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			file_name=(char *)NULL;
			scene=ACCESS(Scene)(command_data->default_scene);
			(option_table[0]).to_be_modified= &file_name;
			(option_table[1]).to_be_modified= &scene;
			(option_table[1]).user_data=command_data->scene_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (!file_name)
				{
					file_name=confirmation_get_write_filename(".wrl",
						command_data->user_interface);
				}
				if (file_name)
				{
					if (return_code=check_suffix(&file_name,".wrl"))
					{
						return_code=export_to_vrml(file_name,scene);
					}
				}
			} /* parse error,help */
			DEACCESS(Scene)(&scene);
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_export_vrml.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_vrml.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_vrml */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_export_wavefront(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EXPORT WAVEFRONT command.
==============================================================================*/
{
	char binary,*file_name,full_comments,*scene_object_name,*temp_filename;
	int frame_number, number_of_frames, return_code, version;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	struct Scene_object *scene_object;
	static struct Modifier_entry option_table[]=
	{
 		{"binary",NULL,NULL,set_char_flag},
		{"file",NULL,(void *)1,set_name},
		{"frame_number",NULL,NULL,set_int_non_negative},
		{"full_comments",NULL,NULL,set_char_flag},
		{"graphics_object",NULL,(void *)1,set_name},
 		{"number_of_frames",NULL,NULL,set_int_positive},
		{"scene",NULL,NULL,set_Scene_including_sub_objects},
 		{"version",NULL,NULL,set_int_positive},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_export_wavefront);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
 			binary=0;
			file_name=(char *)NULL;
			frame_number = 0;
			full_comments=0;
 			number_of_frames=100;
			scene=ACCESS(Scene)(command_data->default_scene);
			scene_object_name=(char *)NULL;
 			version=3;
 			(option_table[0]).to_be_modified= &binary;
			(option_table[1]).to_be_modified= &file_name;
			(option_table[2]).to_be_modified= &frame_number;
			(option_table[3]).to_be_modified= &full_comments;
			(option_table[4]).to_be_modified= &scene_object_name;
 			(option_table[5]).to_be_modified= &number_of_frames;
			(option_table[6]).to_be_modified= &scene;
			(option_table[6]).user_data=command_data->scene_manager;
 			(option_table[7]).to_be_modified= &version;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (scene_object_name)
				{
					if (!(scene_object=Scene_get_scene_object_by_name(scene,
						scene_object_name)))
					{
						display_message(ERROR_MESSAGE,
							"gfx_export_wavefront.  Unable to find object %s in scene",
							scene_object_name);
						return_code=0;						
					}
				}
				else
				{
					scene_object=(struct Scene_object *)NULL;
				}
			}
			if (return_code)
			{
				if (!file_name)
				{
					if (scene_object_name)
					{
						if (ALLOCATE(file_name,char,strlen(scene_object_name)+5))
						{
							sprintf(file_name,"%s.obj",scene_object_name);
						}
					}
					else
					{
						if (GET_NAME(Scene)(scene,&file_name))
						{
							if (REALLOCATE(temp_filename,file_name,char,strlen(file_name)+5))
							{
								file_name=temp_filename;
								strcat(file_name,".obj");
							}
						}
					}
				}
				if (file_name)
				{
					if (binary)
 					{
						if (!scene_object)
						{
							if (1==Scene_get_number_of_scene_objects(scene))
							{
								scene_object = first_Scene_object_in_Scene_that(scene,
									(LIST_CONDITIONAL_FUNCTION(Scene_object) *)NULL, NULL);
								if (!Scene_object_has_gt_object(scene_object,
									(struct GT_object *)NULL))
								{
									scene_object = (struct Scene_object *)NULL;
									display_message(ERROR_MESSAGE,"gfx_export_wavefront."
										"Can only export one object or settings at a time with binary wavefront");
									return_code=0;									
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"gfx_export_wavefront."
									"Can only export one object or settings at a time with binary wavefront");
								return_code=0;								
							}
						}
 						return_code=export_to_binary_wavefront(file_name,scene_object,
							number_of_frames,version,frame_number);
 					}
 					else
 					{
 						return_code=export_to_wavefront(file_name,scene,scene_object,
							full_comments);
 					}
				}
			} /* parse error,help */
			DEACCESS(Scene)(&scene);
			if (scene_object_name)
			{
				DEALLOCATE(scene_object_name);
			}
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_export_wavefront.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_wavefront.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_wavefront */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_export(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EXPORT command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"alias",NULL,NULL,gfx_export_alias},
		{"node",NULL,NULL,gfx_export_node},
		{"vrml",NULL,NULL,gfx_export_vrml},
		{"wavefront",NULL,NULL,gfx_export_wavefront},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_command_gfx_export);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data_void)
		{
			(option_table[0]).user_data=command_data_void;
			(option_table[1]).user_data=command_data_void;
			(option_table[2]).user_data=command_data_void;
			(option_table[3]).user_data=command_data_void;
			return_code=process_option(state,option_table);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_export.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_export.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_export */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
int gfx_evaluate(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 1 November 1999

DESCRIPTION :
==============================================================================*/
{
	int i,return_code;
	static struct Modifier_entry option_table[]=
	{
		{"destination",NULL,NULL,set_Computed_field_conditional},
		{"dgroup",NULL,NULL,set_FE_node_group},
		{"egroup",NULL,NULL,set_FE_element_group},
		{"ngroup",NULL,NULL,set_FE_node_group},
		{"source",NULL,NULL,set_Computed_field_conditional},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Computed_field *destination_computed_field,
		*source_computed_field;
	struct Set_Computed_field_conditional_data set_destination_field_data,
		set_source_field_data;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *data_group, *node_group;

	ENTER(gfx_evaluate);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data = (struct Cmiss_command_data *)command_data_void)
		{
			data_group = (struct GROUP(FE_node) *) NULL;
			element_group = (struct GROUP(FE_element) *) NULL;
			node_group = (struct GROUP(FE_node) *) NULL;
			i=0;
			/* destination */
			/* Could have better conditional functions */
			destination_computed_field=(struct Computed_field *)NULL;
			set_destination_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_destination_field_data.conditional_function_user_data=(void *)NULL;
			set_destination_field_data.computed_field_package =
				command_data->computed_field_package;
			(option_table[i]).to_be_modified= &destination_computed_field;
			(option_table[i]).user_data= &set_destination_field_data;
			i++;
			/* dgroup */
			(option_table[i]).to_be_modified= &data_group;
			(option_table[i]).user_data=command_data->data_group_manager;
			i++;
			/* egroup */
			(option_table[i]).to_be_modified= &element_group;
			(option_table[i]).user_data=command_data->element_group_manager;
			i++;
			/* ngroup */
			(option_table[i]).to_be_modified= &node_group;
			(option_table[i]).user_data=command_data->node_group_manager;
			i++;
			/* source */
			/* Could have better conditional functions */
			source_computed_field=(struct Computed_field *)NULL;
			set_source_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			set_source_field_data.computed_field_package =
				command_data->computed_field_package;
			(option_table[i]).to_be_modified= &source_computed_field;
			(option_table[i]).user_data= &set_source_field_data;
			i++;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (destination_computed_field&&source_computed_field)
				{
					if (data_group&&(!element_group)&&(!node_group))
					{
						Computed_field_update_nodal_values_from_source(
							destination_computed_field,source_computed_field,
							data_group,command_data->data_manager);
					}
					else if (element_group&&(!data_group)&&(!node_group))
					{
						Computed_field_update_element_values_from_source(
							destination_computed_field,source_computed_field,
							element_group,command_data->element_manager);
					}
					else if (node_group&&(!data_group)&&(!element_group))
					{
						Computed_field_update_nodal_values_from_source(
							destination_computed_field,source_computed_field,
							node_group,command_data->node_manager);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_evaluate.  Must specify one of dgroup/egroup/ngroup");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_evaluate.  Must specify destination and source fields");
					return_code=0;
				}
			}
			if (data_group)
			{
				DEACCESS(GROUP(FE_node))(&data_group);
			}				
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}				
			if (node_group)
			{
				DEACCESS(GROUP(FE_node))(&node_group);
			}				
			if (source_computed_field)
			{
				DEACCESS(Computed_field)(&source_computed_field);
			}
			if (destination_computed_field)
			{
				DEACCESS(Computed_field)(&destination_computed_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_evaluate.  Missing command data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_evaluate.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* gfx_evaluate */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_filter(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 26 June 1997

DESCRIPTION :
Executes a GFX FILTER command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"node",NULL,NULL,gfx_filter_node},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_gfx_filter);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			(option_table[0]).user_data=(void *)(command_data->node_group_manager);
			return_code=process_option(state,option_table);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_filter.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_filter.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_filter */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_grab_frame(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX GRAB_FRAME command.
==============================================================================*/
{
	int number_of_frames,return_code;
	static struct Modifier_entry option_table[]=
	{
		{NULL,NULL,NULL,set_int_positive}
	};

	ENTER(execute_command_gfx_grab_frame);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	/* check argument */
	if (state)
	{
		/* initialize defaults */
		number_of_frames=1;
		(option_table[0]).to_be_modified= &number_of_frames;
		return_code=process_multiple_options(state,option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			grab_frame(number_of_frames);
		} /* parse error, help */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_grab_frame.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_grab_frame */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_environment_map(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 June 1996

DESCRIPTION :
Executes a GFX LIST ENVIRONMENT_MAP.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Environment_map *environment_map;

	ENTER(gfx_list_environment_map);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (command_data=(struct Cmiss_command_data *)command_data_void)
				{
					if (environment_map=FIND_BY_IDENTIFIER_IN_MANAGER(Environment_map,
						name)(current_token,command_data->environment_map_manager))
					{
						return_code=list_Environment_map(environment_map);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown environment map: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_list_environment_map.  Missing command_data");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," ENVIRONMENT_MAP_NAME");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing environment map name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_environment_map.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_environment_map */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_Computed_field(struct Parse_state *state,
	void *dummy_to_be_modified,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 12 March 1999

DESCRIPTION :
Executes a GFX LIST FIELD.
???RC Could be moved to computed_field.c.
==============================================================================*/
{
	static char	*command_prefix="gfx define field ";
	char commands_flag;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"name",NULL,NULL,set_Computed_field_conditional},
		{NULL,NULL,NULL,set_Computed_field_conditional}
	};
	struct Computed_field *computed_field;
	struct Computed_field_package *computed_field_package;
	struct LIST(Computed_field) *list_of_fields;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(gfx_list_Computed_field);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((computed_field_package=
			(struct Computed_field_package *)computed_field_package_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				computed_field_package)))
		{
			commands_flag=0;
			/* if no computed_field specified, list all computed_fields */
			computed_field=(struct Computed_field *)NULL;
			set_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_field_data.conditional_function_user_data=(void *)NULL;
			set_field_data.computed_field_package=computed_field_package;
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &computed_field;
			(option_table[1]).user_data= &set_field_data;
			(option_table[2]).to_be_modified= &computed_field;
			(option_table[2]).user_data= &set_field_data;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (commands_flag)
				{
					if (computed_field)
					{
						return_code=list_Computed_field_commands(computed_field,
							(void *)command_prefix);
					}
					else
					{
						if (list_of_fields=CREATE(LIST(Computed_field))())
						{
							while (return_code&&(computed_field=FIRST_OBJECT_IN_MANAGER_THAT(
								Computed_field)(Computed_field_commands_ready_to_list,
								(void *)list_of_fields,computed_field_manager)))
							{
								return_code=list_Computed_field_commands(computed_field,
									(void *)command_prefix)&&
									ADD_OBJECT_TO_LIST(Computed_field)(computed_field,
										list_of_fields);
							}
							DESTROY(LIST(Computed_field))(&list_of_fields);
						}
						else
						{
							return_code=0;
						}
						if (!return_code)
						{
							display_message(ERROR_MESSAGE,
								"gfx_list_Computed_field.  Could not list field commands");
						}
					}
				}
				else
				{
					if (computed_field)
					{
						return_code=list_Computed_field(computed_field,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
							list_Computed_field_name,(void *)NULL,computed_field_manager);
					}
				}
			}
			/* must deaccess computed_field since accessed by set_Computed_field */
			if (computed_field)
			{
				DEACCESS(Computed_field)(&computed_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_list_Computed_field.  "
				"Missing computed_field_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_Computed_field.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_Computed_field */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_FE_element(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 February 1997

DESCRIPTION :
Executes a GFX LIST ELEMENT.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct FE_element *element;
	struct CM_element_information cm;
	int element_num,face_num,line_num;

	ENTER(gfx_list_FE_element);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (1==sscanf(current_token,"%d",&(element_num)))
					{
						shift_Parse_state(state,1);
						if (current_token=state->current_token)
						{
							if (1==sscanf(current_token,"%d",&(face_num)))
							{
								shift_Parse_state(state,1);
								if (current_token=state->current_token)
								{
									if (1==sscanf(current_token,"%d",&(line_num)))
									{
#if defined (OLD_CODE)
										if (element=FIND_BY_IDENTIFIER_IN_LIST(FE_element,
											identifier)(&cm,all_FE_element))
#endif /* defined (OLD_CODE) */
										if(element_num)
										{
											cm.number = element_num;
											cm.type = CM_ELEMENT;
										}
										else if(face_num)
										{
											cm.number = face_num;
											cm.type = CM_FACE;
										}
										else
										{
											cm.number = line_num;
											cm.type = CM_LINE;
										}

										if (element=FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,
											identifier)(&cm,command_data->element_manager))
										{
											list_FE_element(element,(void *)NULL);
										}
										else
										{
											display_message(ERROR_MESSAGE,"Unknown %d-D element: %d ",
												4-(cm.type+1),cm.number);
											return_code=0;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,"Invalid line number: %s",
											current_token);
										display_parse_state_location(state);
										return_code=0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,"Missing line number");
									display_parse_state_location(state);
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"Invalid face number: %s",
									current_token);
								display_parse_state_location(state);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Missing face number");
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid element number: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
						" ELEMENT#{integer} FACE#{integer} LINE#{integer}");
					return_code=1;
				}
			}
			else
			{
#if defined (OLD_CODE)
				return_code=FOR_EACH_OBJECT_IN_LIST(FE_element)(list_FE_element,
					(void *)NULL,all_FE_element);
#endif /* defined (OLD_CODE) */
				return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(list_FE_element,
					(void *)NULL,command_data->element_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_FE_element.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_FE_element.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_FE_element */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_FE_node(struct Parse_state *state,
	void *dummy_to_be_modified,void *node_manager_void)
/*******************************************************************************
LAST MODIFIED : 26 May 1998

DESCRIPTION :
Executes a GFX LIST NODE.
==============================================================================*/
{
	char *current_token;
	int node_number,return_code;
	struct FE_node *node;
	struct MANAGER(FE_node) *node_manager;

	ENTER(gfx_list_FE_node);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (node_manager=(struct MANAGER(FE_node) *)node_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (1==sscanf(current_token,"%d",&node_number))
					{
						if (node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,cm_node_identifier)(
							node_number,node_manager))
						{
							return_code=list_FE_node(node,(void *)1);
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown node: %d",node_number);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid node number: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," NODE#{integer}");
					return_code=1;
				}
			}
			else
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_node)(list_FE_node,
					(void *)1,node_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_FE_node.  Missing node_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_FE_node.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_FE_node */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_data(struct Parse_state *state,
	void *dummy_to_be_modified,void *data_manager_void)
/*******************************************************************************
LAST MODIFIED : 26 May 1998

DESCRIPTION :
Executes a GFX LIST DATA.
==============================================================================*/
{
	char *current_token;
	int data_number,return_code;
	struct FE_node *data;
	struct MANAGER(FE_node) *data_manager;

	ENTER(gfx_list_data);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (data_manager=(struct MANAGER(FE_node) *)data_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (1==sscanf(current_token,"%d",&data_number))
					{
						if (data=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,cm_node_identifier)(
							data_number,data_manager))
						{
							return_code=list_FE_node(data,(void *)1);
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown data: %d",data_number);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid data number: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," NODE#{integer}");
					return_code=1;
				}
			}
			else
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_node)(list_FE_node,
					(void *)1,data_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_data.  Missing data_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_data.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_data */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_list_graphical_material(struct Parse_state *state,
	void *dummy_to_be_modified,void *graphical_material_manager_void)
/*******************************************************************************
LAST MODIFIED : 22 September 1998

DESCRIPTION :
Executes a GFX LIST MATERIAL.
???RC Could be moved to material.c.
==============================================================================*/
{
	static char	*command_prefix="gfx create material ";
	char commands_flag;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"name",NULL,NULL,set_Graphical_material},
		{NULL,NULL,NULL,set_Graphical_material}
	};
	struct Graphical_material *material;
	struct MANAGER(Graphical_material) *graphical_material_manager;

	ENTER(gfx_list_graphical_material);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (graphical_material_manager=
			(struct MANAGER(Graphical_material) *)graphical_material_manager_void)
		{
			commands_flag=0;
			/* if no material specified, list all materials */
			material=(struct Graphical_material *)NULL;
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &material;
			(option_table[1]).user_data= graphical_material_manager_void;
			(option_table[2]).to_be_modified= &material;
			(option_table[2]).user_data= graphical_material_manager_void;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (commands_flag)
				{
					if (material)
					{
						return_code=list_Graphical_material_commands(material,
							(void *)command_prefix);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphical_material)(
							list_Graphical_material_commands,(void *)command_prefix,
							graphical_material_manager);
					}
				}
				else
				{
					if (material)
					{
						return_code=list_Graphical_material(material,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphical_material)(
							list_Graphical_material,(void *)NULL,
							graphical_material_manager);
					}
				}
			}
			/* must deaccess material since accessed by set_Graphical_material */
			if (material)
			{
				DEACCESS(Graphical_material)(&material);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_list_graphical_material.  "
				"Missing graphical_material_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_graphical_material.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_graphical_material */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_group_FE_element(struct Parse_state *state,
	void *dummy_to_be_modified,void *element_group_manager_void)
/*******************************************************************************
LAST MODIFIED : 2 October 1996

DESCRIPTION :
Executes a GFX LIST EGROUP.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct GROUP(FE_element) *element_group;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;

	ENTER(gfx_list_group_FE_element);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (element_group_manager=
			(struct MANAGER(GROUP(FE_element)) *)element_group_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (element_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),
						name)(current_token,element_group_manager))
					{
						return_code=list_group_FE_element(element_group,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown element group: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," <ELEMENT_GROUP_NAME[all]>");
					return_code=1;
				}
			}
			else
			{
				if (0<NUMBER_IN_MANAGER(GROUP(FE_element))(element_group_manager))
				{
					display_message(INFORMATION_MESSAGE,"element groups:\n");
					return_code=FOR_EACH_OBJECT_IN_MANAGER(GROUP(FE_element))(
						list_group_FE_element_name,(void *)NULL,element_group_manager);
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
						"There are no element groups defined\n");
					return_code=1;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_group_FE_element.  Missing element_group_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_group_FE_element.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_group_FE_element */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_g_element(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 26 July 1998

DESCRIPTION :
Executes a GFX LIST G_ELEMENT.
==============================================================================*/
{
	static char	*command_prefix="gfx modify g_element";
	char commands_flag,*command_suffix,*group_name,*scene_name;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"group",NULL,NULL,set_FE_element_group},
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,set_FE_element_group}
	};
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group;
	struct GT_element_group *gt_element_group;
	struct Scene *scene;

	ENTER(gfx_list_g_element);
	USE_PARAMETER(dummy_to_be_modified);
	/* check arguments */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			commands_flag=0;
			element_group=(struct GROUP(FE_element) *)NULL;
			scene=ACCESS(Scene)(command_data->default_scene);
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &element_group;
			(option_table[1]).user_data= command_data->element_group_manager;
			(option_table[2]).to_be_modified= &scene;
			(option_table[2]).user_data=command_data->scene_manager;
			(option_table[3]).to_be_modified= &element_group;
			(option_table[3]).user_data= command_data->element_group_manager;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (GET_NAME(Scene)(scene,&scene_name))
				{
					command_suffix=(char *)NULL;
					if ((scene != command_data->default_scene)&&
						ALLOCATE(command_suffix,char,strlen(scene_name)+8))
					{
						sprintf(command_suffix," scene %s",scene_name);
					}
					if (element_group&&(gt_element_group=
						Scene_get_graphical_element_group(scene,element_group)))
					{
						if (GET_NAME(GROUP(FE_element))(element_group,&group_name))
						{
							if (commands_flag)
							{
								display_message(INFORMATION_MESSAGE,
									"Commands for reproducing group %s on scene %s:\n",
									group_name,scene_name);
								return_code=GT_element_group_list_commands(gt_element_group,
									command_prefix,command_suffix);
							}
							else
							{
								display_message(INFORMATION_MESSAGE,
									"Contents of group %s on scene %s:\n",group_name,scene_name);
								return_code=GT_element_group_list_contents(gt_element_group);
							}
							DEALLOCATE(group_name);
						}
					}
					else
					{
						display_message(INFORMATION_MESSAGE,
							"Must specify element group on scene %s\n",scene_name);
						return_code=0;
					}
					if (command_suffix)
					{
						DEALLOCATE(command_suffix);
					}
					DEALLOCATE(scene_name);
				}
			}
			DEACCESS(Scene)(&scene);
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_g_element.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_g_element.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_g_element */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *object_list_void)
/*******************************************************************************
LAST MODIFIED : 26 July 1998

DESCRIPTION :
Executes a GFX LIST GLYPH/GRAPHICS_OBJECT command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct GT_object *object;
	struct LIST(GT_object) *list;

	ENTER(gfx_list_graphics_object);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (list=(struct LIST(GT_object) *)object_list_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
						current_token,list))
					{
						return_code=GT_object_list_contents(object,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Could not find object named '%s'",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," OBJECT_NAME[ALL]");
					return_code=1;
				}
			}
			else
			{
				/* list contents of all objects in list */
				return_code=FOR_EACH_OBJECT_IN_LIST(GT_object)(
					GT_object_list_contents,(void *)NULL,list);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_graphics_object.  Missing graphics object list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_graphics_object.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_graphics_object */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_group_FE_node(struct Parse_state *state,
	void *dummy_to_be_modified,void *node_group_manager_void)
/*******************************************************************************
LAST MODIFIED : 16 December 1997

DESCRIPTION :
Executes a GFX LIST NGROUP.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct GROUP(FE_node) *node_group;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;

	ENTER(gfx_list_group_FE_node);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (node_group_manager=
			(struct MANAGER(GROUP(FE_node)) *)node_group_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (node_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
						current_token,node_group_manager))
					{
						return_code=list_group_FE_node(node_group,(void *)1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown node group: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," <NODE_GROUP_NAME{all}>");
					return_code=1;
				}
			}
			else
			{
				if (0<NUMBER_IN_MANAGER(GROUP(FE_node))(node_group_manager))
				{
					display_message(INFORMATION_MESSAGE,"node groups:\n");
					return_code=FOR_EACH_OBJECT_IN_MANAGER(GROUP(FE_node))(
						list_group_FE_node,(void *)NULL,node_group_manager);
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
						"There are no node groups defined\n");
					return_code=1;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_group_FE_node.  Missing node_group_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_group_FE_node.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_group_FE_node */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_group_data(struct Parse_state *state,
	void *dummy_to_be_modified,void *data_group_manager_void)
/*******************************************************************************
LAST MODIFIED : 26 May 1998

DESCRIPTION :
Executes a GFX LIST DGROUP.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct GROUP(FE_node) *data_group;
	struct MANAGER(GROUP(FE_node)) *data_group_manager;

	ENTER(gfx_list_group_data);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (data_group_manager=
			(struct MANAGER(GROUP(FE_node)) *)data_group_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (data_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
						current_token,data_group_manager))
					{
						return_code=list_group_FE_node(data_group,(void *)1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown data group: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," <NODE_GROUP_NAME{all}>");
					return_code=1;
				}
			}
			else
			{
				if (0<NUMBER_IN_MANAGER(GROUP(FE_node))(data_group_manager))
				{
					display_message(INFORMATION_MESSAGE,"data groups:\n");
					return_code=FOR_EACH_OBJECT_IN_MANAGER(GROUP(FE_node))(
						list_group_FE_node,(void *)NULL,data_group_manager);
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
						"There are no data groups defined\n");
					return_code=1;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_group_data.  Missing data_group_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_group_data.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_group_data */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_interest_point(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Executes a GFX LIST INTEREST_POINT.
==============================================================================*/
{
	double eyex,eyey,eyez,lookatx,lookaty,lookatz,upx,upy,upz;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"window",NULL,NULL,set_Graphics_window},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;

	ENTER(gfx_list_interest_point);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,command_data->graphics_window_manager))
			{
				ACCESS(Graphics_window)(window);
			}
			(option_table[0]).to_be_modified=(void *)&window;
			(option_table[0]).user_data=command_data->graphics_window_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (window)
				{
					if (Scene_viewer_get_lookat_parameters(
						Graphics_window_get_Scene_viewer(window,0),&eyex,&eyey,&eyez,
						&lookatx,&lookaty,&lookatz,&upx,&upy,&upz))
					{
						display_message(INFORMATION_MESSAGE,"interest point: %g %g %g\n",
							lookatx,lookaty,lookatz);
/*???DB.  Temp ? */
printf("set interest_point position %g %g %g\n",lookatx,lookaty,lookatz);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Must have a graphics window");
					return_code=0;
				}
			} /* parse error, help */
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_interest_point.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_interest_point.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_interest_point */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_light(struct Parse_state *state,
	void *dummy_to_be_modified,void *light_manager_void)
/*******************************************************************************
LAST MODIFIED : 2 September 1996

DESCRIPTION :
Executes a GFX LIST LIGHT.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Light *light;
	struct MANAGER(Light) *light_manager;

	ENTER(gfx_list_light);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (light_manager=(struct MANAGER(Light) *)light_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (light=FIND_BY_IDENTIFIER_IN_MANAGER(Light,name)(current_token,
						light_manager))
					{
						return_code=list_Light(light,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown light: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," LIGHT_NAME");
					return_code=1;
				}
			}
			else
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Light)(list_Light,(void *)NULL,
					light_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_light.  Missing light_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_light.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_light */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_light_model(struct Parse_state *state,
	void *dummy_to_be_modified,void *light_model_manager_void)
/*******************************************************************************
LAST MODIFIED : 3 September 1996

DESCRIPTION :
Executes a GFX LIST LMODEL.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Light_model *light_model;
	struct MANAGER(Light_model) *light_model_manager;

	ENTER(gfx_list_light_model);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (light_model_manager=
			(struct MANAGER(Light_model) *)light_model_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (light_model=FIND_BY_IDENTIFIER_IN_MANAGER(Light_model,name)(
						current_token,light_model_manager))
					{
						return_code=list_Light_model(light_model,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown light model: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," LIGHT_MODEL_NAME");
					return_code=1;
				}
			}
			else
			{
				FOR_EACH_OBJECT_IN_MANAGER(Light_model)(list_Light_model,(void *)NULL,
					light_model_manager);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_light_model.  Missing light_model_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_light_model.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_light_model */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_list_scene(struct Parse_state *state,
	void *dummy_to_be_modified,void *scene_manager_void)
/*******************************************************************************
LAST MODIFIED : 21 September 1998

DESCRIPTION :
Executes a GFX LIST SCENE.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Scene *scene;
	struct MANAGER(Scene) *scene_manager;

	ENTER(gfx_list_scene);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (scene_manager=(struct MANAGER(Scene) *)scene_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (scene=FIND_BY_IDENTIFIER_IN_MANAGER(Scene,name)(current_token,
						scene_manager))
					{
						return_code=list_Scene(scene,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown scene: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," SCENE_NAME");
					return_code=1;
				}
			}
			else
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Scene)(list_Scene,(void *)NULL,
					scene_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_scene.  Missing scene_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_scene.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_scene */

static int gfx_list_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *spectrum_manager_void)
/*******************************************************************************
LAST MODIFIED : 15 October 1996

DESCRIPTION :
Executes a GFX LIST SPECTRUM.
==============================================================================*/
{
	static char	*command_prefix="gfx modify spectrum";
	int commands_flag,return_code;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct Spectrum *spectrum;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,set_Spectrum}
	};

	ENTER(gfx_list_spectrum);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (spectrum_manager=
			(struct MANAGER(Spectrum) *)spectrum_manager_void)
		{
			commands_flag=0;
			spectrum = (struct Spectrum *)NULL;
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &spectrum;
			(option_table[1]).user_data= spectrum_manager_void;

			if (return_code=process_multiple_options(state,option_table))
			{
				if ( commands_flag )
				{
					if ( spectrum )
					{
						display_message(INFORMATION_MESSAGE,
							"Commands for reproducing spectrum:\n");
						return_code=Spectrum_list_commands(spectrum,
							command_prefix,(char *)NULL);
					}
					else
					{
						display_message(INFORMATION_MESSAGE," SPECTRUM_NAME\n");
						return_code = 1;
					}
				}
				else
				{
					if ( spectrum )
					{
						return_code=Spectrum_list_contents(spectrum,(void *)NULL);
					}
					else
					{
						return_code = FOR_EACH_OBJECT_IN_MANAGER(Spectrum)(Spectrum_list_contents,
							(void *)NULL, spectrum_manager);
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_spectrum.  Missing spectrum_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_spectrum.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_spectrum */

static int gfx_list_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *texture_manager_void)
/*******************************************************************************
LAST MODIFIED : 19 May 1999

DESCRIPTION :
Executes a GFX LIST TEXTURE.
???RC Could be moved to texture.c.
==============================================================================*/
{
	static char	*command_prefix="gfx create texture ";
	char commands_flag;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"name",NULL,NULL,set_Texture},
		{NULL,NULL,NULL,set_Texture}
	};
	struct Texture *texture;
	struct MANAGER(Texture) *texture_manager;

	ENTER(gfx_list_texture);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (texture_manager=(struct MANAGER(Texture) *)texture_manager_void)
		{
			commands_flag=0;
			/* if no texture specified, list all textures */
			texture=(struct Texture *)NULL;
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &texture;
			(option_table[1]).user_data= texture_manager_void;
			(option_table[2]).to_be_modified= &texture;
			(option_table[2]).user_data= texture_manager_void;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (commands_flag)
				{
					if (texture)
					{
						return_code=list_Texture_commands(texture,(void *)command_prefix);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Texture)(
							list_Texture_commands,(void *)command_prefix,texture_manager);
					}
				}
				else
				{
					if (texture)
					{
						return_code=list_Texture(texture,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Texture)(
							list_Texture,(void *)NULL,texture_manager);
					}
				}
			}
			/* must deaccess texture since accessed by set_Texture */
			if (texture)
			{
				DEACCESS(Texture)(&texture);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_list_texture.  "
				"Missing texture_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_texture */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_transformation(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX LIST TRANSFORMATION.
==============================================================================*/
{
	char *command_prefix,commands_flag,*scene_name,*scene_object_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	struct Scene_object *scene_object;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"name",NULL,(void *)1,set_name},
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_list_transformation);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			commands_flag=0;
			scene_object_name=(char *)NULL;
			scene=ACCESS(Scene)(command_data->default_scene);
			/* parse the command line */
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &scene_object_name;
			(option_table[2]).to_be_modified= &scene;
			(option_table[2]).user_data=command_data->scene_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (GET_NAME(Scene)(scene,&scene_name))
				{
					if ((!scene_object_name)||(scene_object=
						Scene_get_scene_object_by_name(scene,scene_object_name)))
					{
						if (commands_flag)
						{
							if (ALLOCATE(command_prefix,char,40+strlen(scene_name)))
							{
								sprintf(command_prefix,"gfx set transformation scene %s name",
									scene_name);
								if (scene_object_name)
								{
									return_code=list_Scene_object_transformation_commands(
										scene_object,(void *)command_prefix);
								}
								else
								{
									return_code=for_each_Scene_object_in_Scene(scene,
										list_Scene_object_transformation_commands,
										(void *)command_prefix);
								}
								DEALLOCATE(command_prefix);
							}
							else
							{
								return_code=0;
							}
						}
						else
						{
							if (scene_object_name)
							{
								return_code=
									list_Scene_object_transformation(scene_object,(void *)NULL);
							}
							else
							{
								return_code=for_each_Scene_object_in_Scene(scene,
									list_Scene_object_transformation,(void *)NULL);
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"No object named '%s' in scene %s",scene_object_name,scene_name);
						return_code=0;
					}
					DEALLOCATE(scene_name);
				}
				else
				{
					return_code=0;
				}
			} /* parse error, help */
			DEACCESS(Scene)(&scene);
			if (scene_object_name)
			{
				DEALLOCATE(scene_object_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_transformation.  Missing command_data_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_transformation.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_transformation */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_view_point(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Executes a GFX LIST VIEW_POINT.
==============================================================================*/
{
	double eyex,eyey,eyez,lookatx,lookaty,lookatz,upx,upy,upz;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"window",NULL,NULL,set_Graphics_window},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;

	ENTER(gfx_list_view_point);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,command_data->graphics_window_manager))
			{
				ACCESS(Graphics_window)(window);
			}
			(option_table[0]).to_be_modified=(void *)&window;
			(option_table[0]).user_data=command_data->graphics_window_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (window)
				{
					if (Scene_viewer_get_lookat_parameters(
						Graphics_window_get_Scene_viewer(window,0),&eyex,&eyey,&eyez,
						&lookatx,&lookaty,&lookatz,&upx,&upy,&upz))
					{
						display_message(INFORMATION_MESSAGE,"view point: %g %g %g\n",eyex,
							eyey,eyez);
/*???DB.  Temp ? */
printf("set view_point position %g %g %g\n",eyex,eyey,eyez);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Must have a graphics window");
					return_code=0;
				}
			} /* parse error, help */
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_view_point.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_view_point.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_view_point */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int iterator_list_VT_volume_texture(struct VT_volume_texture *texture,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Iterator function version of list_VT_volume_texture
???DB.  Move to volume_texture.c ?
==============================================================================*/
{
	int return_code;

	ENTER(iterator_list_VT_volume_texture);
	USE_PARAMETER(dummy_user_data);
	return_code=list_VT_volume_texture(texture);
	LEAVE;

	return (return_code);
} /* iterator_list_VT_volume_texture */

static int gfx_list_volume_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *volume_texture_manager_void)
/*******************************************************************************
LAST MODIFIED : 2 August 1998

DESCRIPTION :
Executes a GFX LIST VTEXTURE.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct VT_volume_texture *texture;
	struct MANAGER(VT_volume_texture) *volume_texture_manager;

	ENTER(gfx_list_volume_texture);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (volume_texture_manager=
			(struct MANAGER(VT_volume_texture) *)volume_texture_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (texture=FIND_BY_IDENTIFIER_IN_MANAGER(VT_volume_texture,name)(
						current_token,volume_texture_manager))
					{
						return_code=list_VT_volume_texture(texture);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown volume texture: %s",
							current_token);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," VOLUME_TEXTURE_NAME");
					return_code=1;
				}
			}
			else
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(VT_volume_texture)(
					iterator_list_VT_volume_texture,(void *)NULL,
					volume_texture_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_volume_texture.  Missing volume_texture_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_volume_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_volume_texture */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_list_movie_graphics(struct Parse_state *state,
	void *dummy_to_be_modified,void *movie_graphics_manager_void)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Executes a GFX LIST MOVIE.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Movie_graphics *movie;
	struct MANAGER(Movie_graphics) *movie_graphics_manager;

	ENTER(gfx_list_movie_graphics);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (movie_graphics_manager=
			(struct MANAGER(Movie_graphics) *)movie_graphics_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (movie=FIND_BY_IDENTIFIER_IN_MANAGER(Movie_graphics,name)(
						current_token,movie_graphics_manager))
					{
						return_code=list_Movie_graphics(movie,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown volume movie: %s",
							current_token);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," MOVIE_NAME");
					return_code=1;
				}
			}
			else
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Movie_graphics)(
					list_Movie_graphics,(void *)NULL,movie_graphics_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_movie_graphics.  Missing movie_graphics_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_movie_graphics.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_movie_graphics */

#if defined (OLD_CODE)
#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_graphics_window(struct Parse_state *state,
	void *dummy_to_be_modified,void *graphics_window_manager_void)
/*******************************************************************************
LAST MODIFIED : 21 September 1998

DESCRIPTION :
Executes a GFX LIST WINDOW.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Graphics_window *graphics_window;
	struct MANAGER(Graphics_window) *graphics_window_manager;

	ENTER(gfx_list_graphics_window);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (graphics_window_manager=
			(struct MANAGER(Graphics_window) *)graphics_window_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (graphics_window=
						FIND_BY_IDENTIFIER_IN_MANAGER(Graphics_window,name)(current_token,
						graphics_window_manager))
					{
						return_code=list_Graphics_window(graphics_window,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown graphics_window: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," GRAPHICS_WINDOW_NAME");
					return_code=1;
				}
			}
			else
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
					list_Graphics_window,(void *)NULL,graphics_window_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_graphics_window.  Missing graphics_window_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_graphics_window.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_graphics_window */
#endif /* !defined (WINDOWS_DEV_FLAG) */
#endif /* defined (OLD_CODE) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_graphics_window(struct Parse_state *state,
	void *dummy_to_be_modified,void *graphics_window_manager_void)
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
Executes a GFX LIST WINDOW.
==============================================================================*/
{
	char commands_flag;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"name",NULL,NULL,set_Graphics_window},
		{NULL,NULL,NULL,set_Graphics_window}
	};
	struct Graphics_window *window;
	struct MANAGER(Graphics_window) *graphics_window_manager;

	ENTER(gfx_list_graphics_window);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (graphics_window_manager=
			(struct MANAGER(Graphics_window) *)graphics_window_manager_void)
		{
			commands_flag=0;
			/* if no window specified, list all windows */
			window=(struct Graphics_window *)NULL;
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &window;
			(option_table[1]).user_data= graphics_window_manager_void;
			(option_table[2]).to_be_modified= &window;
			(option_table[2]).user_data= graphics_window_manager_void;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (commands_flag)
				{
					if (window)
					{
						return_code=list_Graphics_window_commands(window,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
							list_Graphics_window_commands,(void *)NULL,
							graphics_window_manager);
					}
				}
				else
				{
					if (window)
					{
						return_code=list_Graphics_window(window,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
							list_Graphics_window,(void *)NULL,
							graphics_window_manager);
					}
				}
			}
			/* must deaccess window since accessed by set_Graphics_window */
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_list_graphics_window.  "
				"Missing graphics_window_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_graphics_window.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_graphics_window */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_list(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 February 2000

DESCRIPTION :
Executes a GFX LIST command.
==============================================================================*/
{
	int i,return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"curve",NULL,NULL,gfx_list_Control_curve},
		{"data",NULL,NULL,gfx_list_data},
		{"dgroup",NULL,NULL,gfx_list_group_data},
		{"egroup",NULL,NULL,gfx_list_group_FE_element},
		{"element",NULL,NULL,gfx_list_FE_element},
		{"environment_map",NULL,NULL,gfx_list_environment_map},
		{"field",NULL,NULL,gfx_list_Computed_field},
		{"g_element",NULL,NULL,gfx_list_g_element},
		{"glyph",NULL,NULL,gfx_list_graphics_object},
		{"graphics_object",NULL,NULL,gfx_list_graphics_object},
		{"interest_point",NULL,NULL,gfx_list_interest_point},
		{"light",NULL,NULL,gfx_list_light},
		{"lmodel",NULL,NULL,gfx_list_light_model},
		{"material",NULL,NULL,gfx_list_graphical_material},
#if defined (SGI_MOVIE_FILE)
		{"movie",NULL,NULL,gfx_list_movie_graphics},
#endif /* defined (SGI_MOVIE_FILE) */
		{"ngroup",NULL,NULL,gfx_list_group_FE_node},
		{"node",NULL,NULL,gfx_list_FE_node},
		{"slider",NULL,NULL,list_node_group_slider},
		{"scene",NULL,NULL,gfx_list_scene},
		{"spectrum",NULL,NULL,gfx_list_spectrum},
		{"texture",NULL,NULL,gfx_list_texture},
		{"transformation",NULL,NULL,gfx_list_transformation},
		{"view_point",NULL,NULL,gfx_list_view_point},
		{"vtexture",NULL,NULL,gfx_list_volume_texture},
		{"window",NULL,NULL,gfx_list_graphics_window},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_command_gfx_list);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				i=0;
				/* curve */
				(option_table[i]).user_data=
					(void *)(command_data->control_curve_manager);
				i++;
				/* data */
				(option_table[i]).user_data=command_data->data_manager;
				i++;
				/* dgroup */
				(option_table[i]).user_data=command_data->data_group_manager;
				i++;
				/* egroup */
				(option_table[i]).user_data=command_data->element_group_manager;
				i++;
				/* element */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* environment_map */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* field */
				(option_table[i]).user_data=command_data->computed_field_package;
				i++;
				/* g_element */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* glyph */
				(option_table[i]).user_data=command_data->glyph_list;
				i++;
				/* graphics_object */
				(option_table[i]).user_data=command_data->graphics_object_list;
				i++;
				/* interest_point */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* light */
				(option_table[i]).user_data=command_data->light_manager;
				i++;
				/* lmodel */
				(option_table[i]).user_data=command_data->light_model_manager;
				i++;
				/* material */
				(option_table[i]).user_data=command_data->graphical_material_manager;
				i++;
#if defined (SGI_MOVIE_FILE)
				/* movie */
				(option_table[i]).user_data=command_data->movie_graphics_manager;
				i++;
#endif /* defined (SGI_MOVIE_FILE) */
				/* ngroup */
				(option_table[i]).user_data=command_data->node_group_manager;
				i++;
				/* node */
				(option_table[i]).user_data=command_data->node_manager;
				i++;
				/* slider */
				(option_table[i]).user_data=command_data->node_group_slider_dialog;
				i++;
				/* scene */
				(option_table[i]).user_data=command_data->scene_manager;
				i++;
				/* spectrum */
				(option_table[i]).user_data=command_data->spectrum_manager;
				i++;
				/* texture */
				(option_table[i]).user_data=command_data->texture_manager;
				i++;
				/* transformation */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* view_point */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* volume texture */
				(option_table[i]).user_data=command_data->volume_texture_manager;
				i++;
				/* graphics_window */
				(option_table[i]).user_data=command_data->graphics_window_manager;
				i++;
				return_code=process_option(state,option_table);
			}
			else
			{
				set_command_prompt("gfx list",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_list.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_list.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_list */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_modify_data_group(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 21 April 1999

DESCRIPTION :
Executes a GFX MODIFY DGROUP command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"add_ranges",NULL,NULL,set_Multi_range},
		{"from",NULL,NULL,set_FE_node_group},
		{"remove_ranges",NULL,NULL,set_Multi_range},
		{NULL,NULL,NULL,NULL}
	};
	struct Add_FE_node_to_list_if_in_range_data add_node_data,remove_node_data;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_node) *from_data_group,*data_group;

	ENTER(gfx_modify_data_group);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		data_group=(struct GROUP(FE_node) *)NULL;
		if (set_FE_node_group(state,(void *)&data_group,
			(void *)command_data->data_group_manager))
		{
			/* initialise defaults */
			from_data_group=(struct GROUP(FE_node) *)NULL;
			add_node_data.node_ranges=CREATE(Multi_range)();
			add_node_data.node_list=CREATE(LIST(FE_node))();
			remove_node_data.node_ranges=CREATE(Multi_range)();
			remove_node_data.node_list=CREATE(LIST(FE_node))();
			(option_table[0]).to_be_modified=add_node_data.node_ranges;
			(option_table[1]).to_be_modified= &from_data_group;
			(option_table[1]).user_data=command_data->data_group_manager;
			(option_table[2]).to_be_modified=remove_node_data.node_ranges;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (data_group)
				{
					if ((0<Multi_range_get_number_of_ranges(add_node_data.node_ranges))||
						(0<Multi_range_get_number_of_ranges(remove_node_data.node_ranges)))
					{
						if (!Multi_ranges_overlap(add_node_data.node_ranges,
							remove_node_data.node_ranges))
						{
							/* make list of data to add and remove from data group */
							if (0<Multi_range_get_number_of_ranges(add_node_data.node_ranges))
							{
								if (from_data_group)
								{
									return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(
										add_FE_node_to_list_if_in_range,(void *)&add_node_data,
										from_data_group);
								}
								else
								{
									return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_node)(
										add_FE_node_to_list_if_in_range,(void *)&add_node_data,
										command_data->data_manager);
								}
							}
							if (0<Multi_range_get_number_of_ranges(
								remove_node_data.node_ranges))
							{
								return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(
									add_FE_node_to_list_if_in_range,(void *)&remove_node_data,
									data_group);
							}
							if (return_code)
							{
								MANAGED_GROUP_BEGIN_CACHE(FE_node)(data_group);
								return_code=
									FOR_EACH_OBJECT_IN_LIST(FE_node)(ensure_FE_node_is_in_group,
										(void *)data_group,add_node_data.node_list)&&
									FOR_EACH_OBJECT_IN_LIST(FE_node)(
										ensure_FE_node_is_not_in_group,(void *)data_group,
										remove_node_data.node_list);
								MANAGED_GROUP_END_CACHE(FE_node)(data_group);
							}
							if (!return_code)
							{
								display_message(ERROR_MESSAGE,"gfx modify dgroup. Failed");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx modify dgroup.  Add and remove ranges overlap");
							return_code=0;
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,
							"gfx modify dgroup.  No modifications requested");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx modify dgroup.  Missing group name");
					return_code=0;
				}
			}
			DESTROY(Multi_range)(&add_node_data.node_ranges);
			DESTROY(LIST(FE_node))(&add_node_data.node_list);
			DESTROY(Multi_range)(&remove_node_data.node_ranges);
			DESTROY(LIST(FE_node))(&remove_node_data.node_list);
			if (from_data_group)
			{
				DEACCESS(GROUP(FE_node))(&from_data_group);
			}
		}
		if (data_group)
		{
			DEACCESS(GROUP(FE_node))(&data_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_data_group.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_data_group */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_modify_element_group(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 21 April 1999

DESCRIPTION :
Executes a GFX MODIFY EGROUP command.
==============================================================================*/
{
	char *group_name;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"add_ranges",NULL,NULL,set_Multi_range},
		{"from",NULL,NULL,set_FE_element_group},
		{"remove_ranges",NULL,NULL,set_Multi_range},
		{NULL,NULL,NULL,NULL}
	};
	struct Add_FE_element_to_list_if_in_range_data add_element_data,
		remove_element_data;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group,*from_element_group;
	struct GROUP(FE_node) *node_group;
	struct LIST(FE_node) *add_node_list,*remove_node_list;

	ENTER(gfx_modify_element_group);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		element_group=(struct GROUP(FE_element) *)NULL;
		if (set_FE_element_group(state,(void *)&element_group,
			(void *)command_data->element_group_manager))
		{
			/* initialise defaults */
			from_element_group=(struct GROUP(FE_element) *)NULL;
			add_element_data.element_ranges=CREATE(Multi_range)();
			add_element_data.element_list=CREATE(LIST(FE_element))();
			remove_element_data.element_ranges=CREATE(Multi_range)();
			remove_element_data.element_list=CREATE(LIST(FE_element))();
			(option_table[0]).to_be_modified=add_element_data.element_ranges;
			(option_table[1]).to_be_modified= &from_element_group;
			(option_table[1]).user_data=command_data->element_group_manager;
			(option_table[2]).to_be_modified=remove_element_data.element_ranges;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (element_group&&
					GET_NAME(GROUP(FE_element))(element_group,&group_name))
				{
					if ((0<Multi_range_get_number_of_ranges(
						add_element_data.element_ranges))||
						(0<Multi_range_get_number_of_ranges(
							remove_element_data.element_ranges)))
					{
						if (!Multi_ranges_overlap(add_element_data.element_ranges,
							remove_element_data.element_ranges))
						{
							/* get node group of the same name to synchronize nodes from
								 elements added and removed */
							if (node_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
								group_name,command_data->node_group_manager))
							{
								if (0<Multi_range_get_number_of_ranges(
									add_element_data.element_ranges))
								{
									if (from_element_group)
									{
										return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
											add_FE_element_to_list_if_in_range,
											(void *)&add_element_data,from_element_group);
									}
									else
									{
										return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
											add_FE_element_to_list_if_in_range,
											(void *)&add_element_data,command_data->element_manager);
									}
								}
								if (0<Multi_range_get_number_of_ranges(
									remove_element_data.element_ranges))
								{
									return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
										add_FE_element_to_list_if_in_range,
										(void *)&remove_element_data,element_group);
								}
								if (return_code)
								{
									/* make sure we add any nodes referred to by new elements to
										 the node group of the same name, and that nodes only
										 referred to by elements we are removing are removed */
									add_node_list=CREATE(LIST(FE_node))();
									remove_node_list=CREATE(LIST(FE_node))();
									MANAGED_GROUP_BEGIN_CACHE(FE_element)(element_group);
									MANAGED_GROUP_BEGIN_CACHE(FE_node)(node_group);
									return_code=
										FOR_EACH_OBJECT_IN_LIST(FE_element)(
											ensure_FE_element_and_faces_are_in_group,
											(void *)element_group,add_element_data.element_list)&&
										FOR_EACH_OBJECT_IN_LIST(FE_element)(
											ensure_FE_element_and_faces_are_not_in_group,
											(void *)element_group,remove_element_data.element_list)&&
										FOR_EACH_OBJECT_IN_LIST(FE_element)(
											ensure_top_level_FE_element_nodes_are_in_list,
											(void *)add_node_list,add_element_data.element_list)&&
										FOR_EACH_OBJECT_IN_LIST(FE_element)(
											ensure_top_level_FE_element_nodes_are_in_list,
											(void *)remove_node_list,
											remove_element_data.element_list)&&
										FOR_EACH_OBJECT_IN_GROUP(FE_element)(
											ensure_top_level_FE_element_nodes_are_not_in_list,
											(void *)remove_node_list,element_group)&&
										FOR_EACH_OBJECT_IN_LIST(FE_node)(
											ensure_FE_node_is_in_group,(void *)node_group,
											add_node_list)&&
										FOR_EACH_OBJECT_IN_LIST(FE_node)(
											ensure_FE_node_is_not_in_group,(void *)node_group,
											remove_node_list);
									MANAGED_GROUP_END_CACHE(FE_node)(node_group);
									MANAGED_GROUP_END_CACHE(FE_element)(element_group);
									DESTROY(LIST(FE_node))(&add_node_list);
									DESTROY(LIST(FE_node))(&remove_node_list);
								}
								if (!return_code)
								{
									display_message(ERROR_MESSAGE,"gfx modify ngroup. Failed");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"gfx_modify_element_group.  "
									"Could not find node group %s",group_name);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx modify egroup.  Add and remove ranges overlap");
							return_code=0;
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,
							"gfx modify egroup.  No modifications requested");
					}
					DEALLOCATE(group_name);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx modify egroup.  Missing group name");
					return_code=0;
				}
			}
			DESTROY(LIST(FE_element))(&add_element_data.element_list);
			DESTROY(Multi_range)(&add_element_data.element_ranges);
			DESTROY(Multi_range)(&remove_element_data.element_ranges);
			DESTROY(LIST(FE_element))(&remove_element_data.element_list);
			if (from_element_group)
			{
				DEACCESS(GROUP(FE_element))(&from_element_group);
			}
		}
		if (element_group)
		{
			DEACCESS(GROUP(FE_element))(&element_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_element_group.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_element_group */
#endif /* !defined (WINDOWS_DEV_FLAG) */

int iterator_modify_g_element(struct GROUP(FE_element) *element_group,
	void *modify_g_element_data_void)
/*******************************************************************************
LAST MODIFIED : 13 December 1999

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Modify_g_element_data *modify_g_element_data;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *same_settings;

	ENTER(iterator_modify_g_element);
	if (element_group&&(modify_g_element_data=
		(struct Modify_g_element_data *)modify_g_element_data_void))
	{
		if (gt_element_group=Scene_get_graphical_element_group(
			modify_g_element_data->scene,element_group))
		{
			/* get settings describing same geometry in list */
			same_settings=first_settings_in_GT_element_group_that(
				gt_element_group,GT_element_settings_same_geometry,
				(void *)(modify_g_element_data->settings));
			if (modify_g_element_data->delete_flag)
			{
				/* delete */
				if (same_settings)
				{
					return_code=GT_element_group_remove_settings(
						gt_element_group,same_settings);
				}
				else
				{
					return_code=1;
				}
			}
			else
			{
				/* add/modify */
				if (same_settings)
				{
					ACCESS(GT_element_settings)(same_settings);
					if (-1 != modify_g_element_data->position)
					{
						/* move same_settings to new position */
						GT_element_group_remove_settings(gt_element_group,same_settings);
						GT_element_group_add_settings(gt_element_group,same_settings,
							modify_g_element_data->position);
					}
					/* modify same_settings to match new ones */
					return_code=GT_element_group_modify_settings(gt_element_group,
						same_settings,modify_g_element_data->settings);
					DEACCESS(GT_element_settings)(&same_settings);
				}
				else
				{
					return_code=0;
					if (same_settings=CREATE(GT_element_settings)(
						GT_element_settings_get_settings_type(
							modify_g_element_data->settings)))
					{
						ACCESS(GT_element_settings)(same_settings);
						if (COPY(GT_element_settings)(same_settings,
							modify_g_element_data->settings))
						{
							return_code=GT_element_group_add_settings(gt_element_group,
								same_settings,modify_g_element_data->position);
						}
						DEACCESS(GT_element_settings)(&same_settings);
					}
				}
				/* rebuild graphics for changed settings */
				GT_element_group_build_graphics_objects(gt_element_group,
					(struct FE_element *)NULL,(struct FE_node *)NULL);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"iterator_modify_g_element.  g_element not in scene");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"iterator_modify_g_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* iterator_modify_g_element */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_modify_g_element(struct Parse_state *state,
	void *help_mode,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 December 1999

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT command.
Parameter <help_mode> should be NULL when calling this function.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry
		help_option_table[]=
		{
			{"ELEMENT_GROUP_NAME|ALL",NULL,NULL,gfx_modify_g_element},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{NULL,NULL,NULL,set_FE_element_group_or_all}
		},
		valid_group_option_table[]=
		{
			{"cylinders",NULL,NULL,gfx_modify_g_element_cylinders},
			{"data_points",NULL,NULL,gfx_modify_g_element_data_points},
			{"element_points",NULL,NULL,gfx_modify_g_element_element_points},
			{"general",NULL,NULL,gfx_modify_g_element_general},
			{"iso_surfaces",NULL,NULL,gfx_modify_g_element_iso_surfaces},
			{"lines",NULL,NULL,gfx_modify_g_element_lines},
			{"node_points",NULL,NULL,gfx_modify_g_element_node_points},
			{"streamlines",NULL,NULL,gfx_modify_g_element_streamlines},
			{"surfaces",NULL,NULL,gfx_modify_g_element_surfaces},
			{"volumes",NULL,NULL,gfx_modify_g_element_volumes},
			{NULL,NULL,NULL,NULL}
		};
	struct Cmiss_command_data *command_data;
	struct G_element_command_data g_element_command_data;
	struct GROUP(FE_element) *element_group;
	struct Modify_g_element_data modify_g_element_data;

	ENTER(gfx_modify_g_element);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			return_code=1;
			/* initialize defaults */
			element_group=(struct GROUP(FE_element) *)NULL;
			if (!help_mode)
			{
				if (!state->current_token||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* read element group */
					(option_table[0]).to_be_modified= (void *)(&element_group);
					(option_table[0]).user_data=
						(void *)(command_data->element_group_manager);
					return_code=process_option(state,option_table);
				}
				else
				{
					/* write help - use help_mode flag to get correct behaviour */
					(help_option_table[0]).to_be_modified= (void *)1;
					(help_option_table[0]).user_data=command_data_void;
					return_code=process_option(state,help_option_table);
				}
			}
			if (return_code)
			{
				/* set defaults */
				modify_g_element_data.delete_flag=0;
				modify_g_element_data.position=-1;
				modify_g_element_data.scene=ACCESS(Scene)(command_data->default_scene);
				modify_g_element_data.settings=(struct GT_element_settings *)NULL;
				g_element_command_data.default_material=
					command_data->default_graphical_material;
				g_element_command_data.glyph_list=command_data->glyph_list;
				g_element_command_data.computed_field_package=
					command_data->computed_field_package;
				g_element_command_data.element_manager=command_data->element_manager;
				g_element_command_data.fe_field_manager=command_data->fe_field_manager;
				g_element_command_data.graphical_material_manager=
					command_data->graphical_material_manager;
				g_element_command_data.scene_manager=command_data->scene_manager;
				g_element_command_data.spectrum_manager=command_data->spectrum_manager;
				g_element_command_data.default_spectrum=command_data->default_spectrum;
				g_element_command_data.user_interface=command_data->user_interface;
				g_element_command_data.texture_manager=command_data->texture_manager;
				g_element_command_data.volume_texture_manager=
					command_data->volume_texture_manager;
				(valid_group_option_table[0]).to_be_modified=
					(void *)(&modify_g_element_data);
				(valid_group_option_table[0]).user_data=
					(void *)&g_element_command_data;
				(valid_group_option_table[1]).to_be_modified=
					(void *)(&modify_g_element_data);
				(valid_group_option_table[1]).user_data=
					(void *)&g_element_command_data;
				(valid_group_option_table[2]).to_be_modified=
					(void *)(&modify_g_element_data);
				(valid_group_option_table[2]).user_data=
					(void *)&g_element_command_data;
				(valid_group_option_table[3]).to_be_modified=(void *)element_group;
				(valid_group_option_table[3]).user_data=
					(void *)command_data->default_scene;
				(valid_group_option_table[4]).to_be_modified=
					(void *)(&modify_g_element_data);
				(valid_group_option_table[4]).user_data=
					(void *)&g_element_command_data;
				(valid_group_option_table[5]).to_be_modified=
					(void *)(&modify_g_element_data);
				(valid_group_option_table[5]).user_data=
					(void *)&g_element_command_data;
				(valid_group_option_table[6]).to_be_modified=
					(void *)(&modify_g_element_data);
				(valid_group_option_table[6]).user_data=
					(void *)&g_element_command_data;
				(valid_group_option_table[7]).to_be_modified=
					(void *)(&modify_g_element_data);
				(valid_group_option_table[7]).user_data=
					(void *)&g_element_command_data;
				(valid_group_option_table[8]).to_be_modified=
					(void *)(&modify_g_element_data);
				(valid_group_option_table[8]).user_data=
					(void *)&g_element_command_data;
				(valid_group_option_table[9]).to_be_modified=
					(void *)(&modify_g_element_data);
				(valid_group_option_table[9]).user_data=
					(void *)&g_element_command_data;
				return_code=process_option(state,valid_group_option_table);
				if (return_code&&(modify_g_element_data.settings))
				{
					if (element_group)
					{
						return_code=iterator_modify_g_element(element_group,
							(void *)&modify_g_element_data);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_modify_g_element.  Must specify element group");
						return_code=0;
#if defined (OLD_CODE)
						return_code=FOR_EACH_OBJECT_IN_MANAGER(GROUP(FE_element))(
							iterator_modify_g_element,(void *)&modify_g_element_data,
							command_data->element_group_manager);
#endif /* defined (OLD_CODE) */
					}
				} /* parse error,help */
				if (modify_g_element_data.settings)
				{
					DEACCESS(GT_element_settings)(&(modify_g_element_data.settings));
				}
				DEACCESS(Scene)(&modify_g_element_data.scene);
			} /* parse error,help */
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_g_element.  Missing command data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_g_element.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_g_element */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_modify_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Executes a GFX MODIFY GRAPHICS_OBJECT command.
==============================================================================*/
{
	char glyph_flag;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"glyph",NULL,NULL,set_char_flag},
		{"material",NULL,NULL,set_Graphical_material},
		{"spectrum",NULL,NULL,set_Spectrum},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct GT_object *graphics_object;
	struct Graphical_material *material;
	struct Spectrum *spectrum;

	ENTER(gfx_modify_graphics_object);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		graphics_object=(struct GT_object *)NULL;
		if (strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
		{
			if(graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)
				(state->current_token,command_data->graphics_object_list))
			{
				shift_Parse_state(state,1);				
				/* initialise defaults */
				glyph_flag = 0;
				material = get_GT_object_default_material(graphics_object);
				spectrum = get_GT_object_spectrum(graphics_object);
				(option_table[0]).to_be_modified=&glyph_flag;
				(option_table[1]).to_be_modified=&material;
				(option_table[1]).user_data=command_data->graphical_material_manager;
				(option_table[2]).to_be_modified=&spectrum;
				(option_table[2]).user_data=command_data->spectrum_manager;
				return_code=process_multiple_options(state,option_table);
				/* no errors, not asking for help */
				if (return_code)
				{
					set_GT_object_default_material(graphics_object, material);
					set_GT_object_Spectrum(graphics_object, spectrum);
					if (glyph_flag)
					{
						if (!(IS_OBJECT_IN_LIST(GT_object)(graphics_object, 
							command_data->glyph_list)))
						{
							ADD_OBJECT_TO_LIST(GT_object)(graphics_object, 
							command_data->glyph_list);
						}
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Could not find object named '%s'",
					state->current_token);
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(INFORMATION_MESSAGE," GRAPHICS_OBJECT_NAME");
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_graphics_object.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_graphics_object */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_modify_node_group(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 21 April 1999

DESCRIPTION :
Executes a GFX MODIFY NGROUP command.
==============================================================================*/
{
	char *group_name;
	int number_to_remove,original_number_to_remove,return_code;
	static struct Modifier_entry option_table[]=
	{
		{"add_ranges",NULL,NULL,set_Multi_range},
		{"from",NULL,NULL,set_FE_node_group},
		{"remove_ranges",NULL,NULL,set_Multi_range},
		{NULL,NULL,NULL,NULL}
	};
	struct Add_FE_node_to_list_if_in_range_data add_node_data,remove_node_data;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *from_node_group,*node_group;

	ENTER(gfx_modify_node_group);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		node_group=(struct GROUP(FE_node) *)NULL;
		if (set_FE_node_group(state,(void *)&node_group,
			(void *)command_data->node_group_manager))
		{
			/* initialise defaults */
			from_node_group=(struct GROUP(FE_node) *)NULL;
			add_node_data.node_ranges=CREATE(Multi_range)();
			add_node_data.node_list=CREATE(LIST(FE_node))();
			remove_node_data.node_ranges=CREATE(Multi_range)();
			remove_node_data.node_list=CREATE(LIST(FE_node))();
			(option_table[0]).to_be_modified=add_node_data.node_ranges;
			(option_table[1]).to_be_modified= &from_node_group;
			(option_table[1]).user_data=command_data->node_group_manager;
			(option_table[2]).to_be_modified=remove_node_data.node_ranges;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (node_group&&GET_NAME(GROUP(FE_node))(node_group,&group_name))
				{
					if ((0<Multi_range_get_number_of_ranges(add_node_data.node_ranges))||
						(0<Multi_range_get_number_of_ranges(remove_node_data.node_ranges)))
					{
						if (!Multi_ranges_overlap(add_node_data.node_ranges,
							remove_node_data.node_ranges))
						{
							/* make list of nodes to add and remove from node group */
							if (0<Multi_range_get_number_of_ranges(add_node_data.node_ranges))
							{
								if (from_node_group)
								{
									return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(
										add_FE_node_to_list_if_in_range,(void *)&add_node_data,
										from_node_group);
								}
								else
								{
									return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_node)(
										add_FE_node_to_list_if_in_range,(void *)&add_node_data,
										command_data->node_manager);
								}
							}
							original_number_to_remove=number_to_remove=0;
							if (0<Multi_range_get_number_of_ranges(
								remove_node_data.node_ranges))
							{
								if (element_group=
									FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)(
										group_name,command_data->element_group_manager))
								{
									if (return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(
										add_FE_node_to_list_if_in_range,(void *)&remove_node_data,
										node_group))
									{
										/* do not allow nodes to be removed if they are still used
											 by elements in the element group of the same name */
										original_number_to_remove=
											NUMBER_IN_LIST(FE_node)(remove_node_data.node_list);
										return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
											ensure_top_level_FE_element_nodes_are_not_in_list,
											(void *)remove_node_data.node_list,element_group);
										number_to_remove=
											NUMBER_IN_LIST(FE_node)(remove_node_data.node_list);
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,"gfx_modify_node_group.  "
										"Could not find element group %s",group_name);
									return_code=0;
								}
							}
							if (return_code)
							{
								MANAGED_GROUP_BEGIN_CACHE(FE_node)(node_group);
								return_code=
									FOR_EACH_OBJECT_IN_LIST(FE_node)(ensure_FE_node_is_in_group,
										(void *)node_group,add_node_data.node_list)&&
									FOR_EACH_OBJECT_IN_LIST(FE_node)(
										ensure_FE_node_is_not_in_group,(void *)node_group,
										remove_node_data.node_list);
								MANAGED_GROUP_END_CACHE(FE_node)(node_group);
							}
							if (return_code)
							{
								if (number_to_remove<original_number_to_remove)
								{
									display_message(WARNING_MESSAGE,
										"gfx modify ngroup.  %d nodes could not be removed because "
										"they are still in use by element group %s",
										original_number_to_remove-number_to_remove,group_name);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"gfx modify ngroup. Failed");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx modify ngroup.  Add and remove ranges overlap");
							return_code=0;
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,
							"gfx modify ngroup.  No modifications requested");
					}
					DEALLOCATE(group_name);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx modify ngroup.  Missing group name");
					return_code=0;
				}
			}
			DESTROY(LIST(FE_node))(&add_node_data.node_list);
			DESTROY(Multi_range)(&add_node_data.node_ranges);
			DESTROY(Multi_range)(&remove_node_data.node_ranges);
			DESTROY(LIST(FE_node))(&remove_node_data.node_list);
			if (from_node_group)
			{
				DEACCESS(GROUP(FE_node))(&from_node_group);
			}
		}
		if (node_group)
		{
			DEACCESS(GROUP(FE_node))(&node_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_node_group.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_node_group */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_modify(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 April 2000

DESCRIPTION :
Executes a GFX MODIFY command.
???DB.  Part of GFX EDIT ?
==============================================================================*/
{
	int i,return_code;
	struct Cmiss_command_data *command_data;
	struct Modify_environment_map_data modify_environment_map_data;
	struct Modify_graphical_material_data modify_graphical_material_data;
	struct Modify_graphics_window_data modify_graphics_window_data;
	struct Modify_light_data modify_light_data;
	struct Modify_light_model_data modify_light_model_data;
	struct Modify_scene_data modify_scene_data;
	struct Modify_VT_volume_texture_data modify_VT_volume_texture_data;
	static struct Modifier_entry option_table[]=
	{
		{"dgroup",NULL,NULL,gfx_modify_data_group},
		{"egroup",NULL,NULL,gfx_modify_element_group},
#if defined (MIRAGE)
		{"emoter",NULL,NULL,gfx_modify_emoter},
#endif /* defined (MIRAGE) */
		{"environment_map",NULL,NULL,modify_Environment_map},
		{"flow_particles",NULL,NULL,gfx_modify_flow_particles},
		{"g_element",NULL,NULL,gfx_modify_g_element},
		{"graphics_object",NULL,NULL,gfx_modify_graphics_object},
		{"light",NULL,NULL,modify_Light},
		{"lmodel",NULL,NULL,modify_Light_model},
		{"material",NULL,NULL,modify_Graphical_material},
		{"ngroup",NULL,NULL,gfx_modify_node_group},
		{"scene",NULL,NULL,modify_Scene},
		{"spectrum",NULL,NULL,gfx_modify_Spectrum},
		{"texture",NULL,NULL,gfx_modify_Texture},
		{"vtexture",NULL,NULL,modify_VT_volume_texture},
		{"window",NULL,NULL,modify_Graphics_window},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_command_gfx_modify);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				i=0;
				/* dgroup */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* egroup */
				(option_table[i]).user_data=command_data_void;
				i++;
#if defined (MIRAGE)
				/* emoter */
				(option_table[i]).user_data=command_data->emoter_slider_dialog;
				i++;
#endif /* defined (MIRAGE) */
				/* environment_map */
				modify_environment_map_data.graphical_material_manager=
					command_data->graphical_material_manager;
				modify_environment_map_data.environment_map_manager=
					command_data->environment_map_manager;
				(option_table[i]).user_data=(void *)(&modify_environment_map_data);
				i++;
				/* flow_particles */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* g_element */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* graphics_object */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* light */
				modify_light_data.default_light=command_data->default_light;
				modify_light_data.light_manager=command_data->light_manager;
				(option_table[i]).user_data=(void *)(&modify_light_data);
				i++;
				/* lmodel */
				modify_light_model_data.default_light_model=
					command_data->default_light_model;
				modify_light_model_data.light_model_manager=
					command_data->light_model_manager;
				(option_table[i]).user_data=(void *)(&modify_light_model_data);
				i++;
				/* material */
				modify_graphical_material_data.default_graphical_material=
					command_data->default_graphical_material;
				modify_graphical_material_data.graphical_material_manager=
					command_data->graphical_material_manager;
				modify_graphical_material_data.texture_manager=
					command_data->texture_manager;
				(option_table[i]).user_data=(void *)(&modify_graphical_material_data);
				i++;
				/* ngroup */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* scene */
				modify_scene_data.light_manager=command_data->light_manager;
				modify_scene_data.scene_manager=command_data->scene_manager;
				modify_scene_data.default_scene=command_data->default_scene;
				/* following used for enabling GFEs */
				modify_scene_data.computed_field_package=
					command_data->computed_field_package;
				modify_scene_data.element_manager=command_data->element_manager;
				modify_scene_data.element_group_manager=
					command_data->element_group_manager;
				modify_scene_data.fe_field_manager=command_data->fe_field_manager;
				modify_scene_data.node_manager=command_data->node_manager;
				modify_scene_data.node_group_manager=command_data->node_group_manager;
				modify_scene_data.data_manager=command_data->data_manager;
				modify_scene_data.data_group_manager=command_data->data_group_manager;
				modify_scene_data.element_point_ranges_selection=
					command_data->element_point_ranges_selection;
				modify_scene_data.element_selection=command_data->element_selection;
				modify_scene_data.node_selection=command_data->node_selection;
				modify_scene_data.data_selection=command_data->data_selection;
				modify_scene_data.user_interface=command_data->user_interface;
				(option_table[i]).user_data=(void *)(&modify_scene_data);
				i++;
				/* spectrum */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* texture */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* vtexture */
				modify_VT_volume_texture_data.graphical_material_manager=
					command_data->graphical_material_manager;
				modify_VT_volume_texture_data.environment_map_manager=
					command_data->environment_map_manager;
				modify_VT_volume_texture_data.volume_texture_manager=
					command_data->volume_texture_manager;
				modify_VT_volume_texture_data.set_file_name_option_table=
					command_data->set_file_name_option_table;
				(option_table[i]).user_data=(void *)(&modify_VT_volume_texture_data);
				i++;
				/* window */
				modify_graphics_window_data.graphics_window_manager=
					command_data->graphics_window_manager;
				modify_graphics_window_data.light_manager=command_data->light_manager;
				modify_graphics_window_data.light_model_manager=
					command_data->light_model_manager;
				modify_graphics_window_data.scene_manager=command_data->scene_manager;
				modify_graphics_window_data.texture_manager=
					command_data->texture_manager;
				(option_table[i]).user_data=(void *)(&modify_graphics_window_data);
				i++;
				return_code=process_option(state,option_table);
			}
			else
			{
				set_command_prompt("gfx modify",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_modify.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_modify.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_modify */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
int gfx_movie(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 2 February 2000

DESCRIPTION :
???RC Movie should ACCESS the graphics window so that it cannot be closed while
movie is being created.
==============================================================================*/
{
	static char *default_movie_name="default";
	char add_frame,avi,cinepak_avi,cinepak_quicktime,*create_file_name,end,
		every_frame,force_onscreen,indeo_avi,indeo_quicktime,loop,*movie_name,
		mvc1_sgi_movie3,once,*open_file_name,play,quicktime,rle24_sgi_movie3,
		skip_frames,sgi_movie3,stop;
	double speed;
	int height, return_code, width;
	struct Option_table *option_table;
	struct Cmiss_command_data *command_data;
	struct Movie_graphics *movie;
	struct X3d_movie *x3d_movie;
	struct Graphics_window *graphics_window;

	ENTER(gfx_movie);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data = (struct Cmiss_command_data *)command_data_void)
		{
#if defined (SGI_MOVIE_FILE)
			/* initialise defaults */
			if (ALLOCATE(movie_name,char,strlen(default_movie_name)+1))
			{
				strcpy(movie_name,default_movie_name);
			}
			else
			{
				movie_name=(char *)NULL;
			}
			add_frame = 0;
			avi = 0;
			cinepak_avi = 0;
			cinepak_quicktime = 0;
			create_file_name=(char *)NULL;
			end = 0;
			every_frame = 0;
			force_onscreen = 0;
			height = 0;
			indeo_avi = 0;
			indeo_quicktime = 0;
			loop = 0;
			mvc1_sgi_movie3 = 0;
			once = 0;
			open_file_name=(char *)NULL;
			play = 0;
			quicktime = 0;
			rle24_sgi_movie3 = 0;
			sgi_movie3 = 0;
			skip_frames = 0;
			speed = 0;
			stop = 0;
			width = 0;
			graphics_window=(struct Graphics_window *)NULL;

			option_table=CREATE(Option_table)();
			/* add_frame */
			Option_table_add_entry(option_table,"add_frame",&add_frame,
				NULL,set_char_flag);
			/* avi */
			Option_table_add_entry(option_table,"avi",&avi,
				NULL,set_char_flag);
			/* cinepak_avi */
			Option_table_add_entry(option_table,"cinepak_avi",&cinepak_avi,
				NULL,set_char_flag);
			/* cinepak_quicktime */
			Option_table_add_entry(option_table,"cinepak_quicktime",
				&cinepak_quicktime,NULL,set_char_flag);
			/* create */
			Option_table_add_entry(option_table,"create",&create_file_name,
				(void *)1,set_name);
			/* end */
			Option_table_add_entry(option_table,"end",&end,
				NULL,set_char_flag);
			/* every_frame */
			Option_table_add_entry(option_table,"every_frame",&every_frame,
				NULL,set_char_flag);
			/* force_onscreen */
			Option_table_add_entry(option_table,"force_onscreen",&force_onscreen,
				NULL,set_char_flag);
			/* height */
			Option_table_add_entry(option_table,"height",&height,
				NULL,set_int_non_negative);
			/* indeo_avi */
			Option_table_add_entry(option_table,"indeo_avi",&indeo_avi,
				NULL,set_char_flag);
			/* indeo_quicktime */
			Option_table_add_entry(option_table,"indeo_quicktime",&indeo_quicktime,
				NULL,set_char_flag);
			/* loop */
			Option_table_add_entry(option_table,"loop",&loop,
				NULL,set_char_flag);
			/* mvc1_sgi_movie3 */
			Option_table_add_entry(option_table,"mvc1_sgi_movie3",&mvc1_sgi_movie3,
				NULL,set_char_flag);
			/* name */
			Option_table_add_entry(option_table,"name",&movie_name,
				(void *)1,set_name);
			/* once */
			Option_table_add_entry(option_table,"once",&once,
				NULL,set_char_flag);
			/* open */
			Option_table_add_entry(option_table,"open",&open_file_name,
				(void *)1,set_name);
			/* play */
			Option_table_add_entry(option_table,"play",&play,
				NULL,set_char_flag);
			/* quicktime */
			Option_table_add_entry(option_table,"quicktime",&quicktime,
				NULL,set_char_flag);
			/* rle24_sgi_movie3 */
			Option_table_add_entry(option_table,"rle24_sgi_movie3",&rle24_sgi_movie3,
				NULL,set_char_flag);
			/* sgi_movie3 */
			Option_table_add_entry(option_table,"sgi_movie3",&sgi_movie3,
				NULL,set_char_flag);
			/* skip_frames */
			Option_table_add_entry(option_table,"skip_frames",&skip_frames,
				NULL,set_char_flag);
			/* speed */
			Option_table_add_entry(option_table,"speed",&speed,
				NULL,set_double);
			/* stop */
			Option_table_add_entry(option_table,"stop",&stop,
				NULL,set_char_flag);
			/* width */
			Option_table_add_entry(option_table,"width",&width,
				NULL,set_int_non_negative);
			/* window */
			Option_table_add_entry(option_table,"window",&graphics_window,
				command_data->graphics_window_manager,set_Graphics_window);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				movie=(struct Movie_graphics *)NULL;
				if (movie_name)
				{
					movie=FIND_BY_IDENTIFIER_IN_MANAGER(Movie_graphics,name)(
						movie_name,command_data->movie_graphics_manager);
				}
				if ((avi + cinepak_avi + cinepak_quicktime + indeo_quicktime + indeo_avi + 
					mvc1_sgi_movie3 + quicktime + rle24_sgi_movie3 + sgi_movie3) > 1)
				{
					display_message(ERROR_MESSAGE,
						"gfx_movie.  Can only specify one movie format, avi,  cinepak_quicktime, "
						"indeo_quicktime, quicktime, rle24_sgi_movie3 or sgi_movie3");
					return_code = 0;
				}
				if (open_file_name && (avi + cinepak_avi + cinepak_quicktime + indeo_avi + 
					indeo_quicktime + mvc1_sgi_movie3 + quicktime + rle24_sgi_movie3 + sgi_movie3))
				{
					display_message(ERROR_MESSAGE,
						"gfx_movie.  Cannot specify a format"
						" (avi, cinepak_quicktime, indeo_quicktime, quicktime or sgi_movie3) "
						"when opening an existing movie (open)");
					return_code = 0;
				}
			}
			if (return_code)
			{
				if (movie)
				{
					if (open_file_name || create_file_name)
					{
						display_message(ERROR_MESSAGE,
							"gfx_movie.  Movie %s is already open and must be ended before "
							"another opened or create",movie_name);
					}
				}
				else
				{
					if (movie_name)
					{
						if (open_file_name)
						{
							if (create_file_name)
							{
								display_message(ERROR_MESSAGE,
									"gfx_movie.  Specify only one of open and create");
							}
							else
							{
								if (!(movie=CREATE(Movie_graphics)(movie_name,open_file_name,
									X3D_MOVIE_OPEN_FILE)))
								{
									display_message(ERROR_MESSAGE,
										"gfx_movie.  Could not create movie.");
								}
							}
						}
						else
						{
							if (create_file_name)
							{
								if(avi)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_UNCOMPRESSED_AVI);
								}
								else if(cinepak_avi)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_CINEPAK_AVI);
								}
								else if(cinepak_quicktime)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_CINEPAK_QUICKTIME);
								}
								else if(indeo_avi)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_INDEO_AVI);
								}
								else if(indeo_quicktime)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_INDEO_QUICKTIME);
								}
								else if(quicktime)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_APPLE_ANIMATION_QUICKTIME);
								}
								else if(rle24_sgi_movie3)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_RLE24_SGI_MOVIE3);
								}
								else if(mvc1_sgi_movie3)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_MVC1_SGI_MOVIE3);
								}
								else
								{
									/* Default to this if no format is given */
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_UNCOMPRESSED_SGI_MOVIE3);
								}
								if (!movie)
								{
									display_message(ERROR_MESSAGE,
										"gfx_movie.  Could not create movie.");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_movie.  Need to specify 'open' or 'create' FILENAME");
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_movie.  No name given for new movie");
					}
					if (movie)
					{
						if (!ADD_OBJECT_TO_MANAGER(Movie_graphics)(movie,
							command_data->movie_graphics_manager))
						{
							display_message(ERROR_MESSAGE,
								"gfx_movie.  Could not add movie to manager");
							DESTROY(Movie_graphics)(&movie);
							movie = (struct Movie_graphics *)NULL;
						}
					}
					if (movie)
					{
						/* attach the time object of the new movie to the default time
							keeper */
						Time_object_set_time_keeper(
							X3d_movie_get_time_object(Movie_graphics_get_X3d_movie(movie)),
							command_data->default_time_keeper);
					}
				}
				if (movie && (x3d_movie = Movie_graphics_get_X3d_movie(movie)))
				{
					if (graphics_window)
					{
						Movie_graphics_set_Graphics_window(movie, graphics_window);
					}
					if ( add_frame )
					{
						Movie_graphics_add_frame_to_movie(movie, width, height, force_onscreen);
					}
					if ( every_frame )
					{
						X3d_movie_set_play_every_frame(x3d_movie, 1);
					}
					if ( loop )
					{
						X3d_movie_set_play_loop(x3d_movie, 1);
					}
					if ( once )
					{
						X3d_movie_set_play_loop(x3d_movie, 0);
					}
					if ( play )
					{
						X3d_movie_play(x3d_movie);
					}
					if ( skip_frames )
					{
						X3d_movie_set_play_every_frame(x3d_movie, 0);
					}
					if ( speed != 0.0 )
					{
						X3d_movie_set_play_speed(x3d_movie, speed);
					}
					if ( stop )
					{
						X3d_movie_stop(x3d_movie);
					}
					if ( end )
					{
						return_code=REMOVE_OBJECT_FROM_MANAGER(Movie_graphics)(movie,
							command_data->movie_graphics_manager);
						movie=(struct Movie_graphics *)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_movie.  Invalid movie object");
					return_code=0;
				}
			}
			DESTROY(Option_table)(&option_table);
			if (graphics_window)
			{
				DEACCESS(Graphics_window)(&graphics_window);
			}
#else /* defined (SGI_MOVIE_FILE) */
			display_message(ERROR_MESSAGE,
				"gfx_movie.  Movie extensions not available in this compilation");
			return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_movie.  Missing command data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_movie.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* gfx_movie */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int execute_command_gfx_node_tool(struct Parse_state *state,
	void *data_tool_flag,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Executes a GFX NODE_TOOL or GFX_DATA_TOOL command. If <data_tool_flag> is set,
then the <data_tool> is being modified, otherwise the <node_tool>.
Which tool that is being modified is passed in <node_tool_void>.
==============================================================================*/
{
	int create_enabled,edit_enabled,motion_update_enabled,return_code,
		select_enabled;
	struct Cmiss_command_data *command_data;
	struct FE_field *coordinate_field;
	struct Node_tool *node_tool;
	struct GROUP(FE_node) *node_group;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	struct Option_table *option_table;
	struct Set_FE_field_conditional_data set_coordinate_field_data;

	ENTER(execute_command_gfx_node_tool);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (data_tool_flag)
		{
			node_tool=command_data->data_tool;
			node_group_manager=command_data->data_group_manager;
		}
		else
		{
			node_tool=command_data->node_tool;
			node_group_manager=command_data->node_group_manager;
		}
		/* initialize defaults */
		node_group=(struct GROUP(FE_node) *)NULL;
		if (node_tool)
		{
			coordinate_field=Node_tool_get_coordinate_field(node_tool);
			create_enabled=Node_tool_get_create_enabled(node_tool);
			edit_enabled=Node_tool_get_edit_enabled(node_tool);
			motion_update_enabled=Node_tool_get_motion_update_enabled(node_tool);
			select_enabled=Node_tool_get_select_enabled(node_tool);
			node_group=Node_tool_get_node_group(node_tool);
		}
		else
		{
			coordinate_field=(struct FE_field *)NULL;
			create_enabled=0;
			edit_enabled=0;
			motion_update_enabled=0;
			select_enabled=0;
			node_group=(struct GROUP(FE_node) *)NULL;
		}
		if (coordinate_field)
		{
			ACCESS(FE_field)(coordinate_field);
		}
		if (node_group)
		{
			ACCESS(GROUP(FE_node))(node_group);
		}

		option_table=CREATE(Option_table)();
		/* coordinate_field */
		set_coordinate_field_data.fe_field_manager=command_data->fe_field_manager;
		set_coordinate_field_data.conditional_function=FE_field_is_coordinate_field;
		set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"coordinate_field",
			&coordinate_field,&set_coordinate_field_data,set_FE_field_conditional);
		/* create/no_create */
		Option_table_add_switch(option_table,"create","no_create",&create_enabled);
		/* edit/no_edit */
		Option_table_add_switch(option_table,"edit","no_edit",&edit_enabled);
		/* group */
		Option_table_add_entry(option_table,"group",&node_group,
			node_group_manager,set_FE_node_group);
		/* motion_update/no_motion_update */
		Option_table_add_switch(option_table,"motion_update","no_motion_update",
			&motion_update_enabled);
		/* select/no_select */
		Option_table_add_switch(option_table,"select","no_select",&select_enabled);
		if (return_code=Option_table_multi_parse(option_table,state))
		{
			if (node_tool)
			{
				Node_tool_set_coordinate_field(node_tool,coordinate_field);
				Node_tool_set_create_enabled(node_tool,create_enabled);
				Node_tool_set_edit_enabled(node_tool,edit_enabled);
				Node_tool_set_motion_update_enabled(node_tool,motion_update_enabled);
				Node_tool_set_select_enabled(node_tool,select_enabled);
				Node_tool_set_node_group(node_tool,node_group);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Must create node_tool before modifying it");
				return_code=0;
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
		if (node_group)
		{
			DEACCESS(GROUP(FE_node))(&node_group);
		}
		if (coordinate_field)
		{
			DEACCESS(FE_field)(&coordinate_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_node_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_node_tool */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_print(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Executes a GFX PRINT command.
==============================================================================*/
{
	auto struct Modifier_entry
		landscape_portrait_option_table[]=
		{
			{"landscape",NULL,NULL,set_char_flag},
			{"portrait",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{"file",NULL,(void *)1,set_name},
			{"force_onscreen",NULL,NULL,set_char_flag},
			{"height",NULL,NULL,set_int_non_negative},
			{NULL,NULL,NULL,NULL},
			{NULL,NULL,NULL,NULL},
			{"width",NULL,NULL,set_int_non_negative},
			{"window",NULL,NULL,set_Graphics_window},
			{NULL,NULL,NULL,NULL}
		},
		postscript_rgb_tiff_option_table[]=
		{
			{"postscript",NULL,NULL,set_char_flag},
			{"rgb",NULL,NULL,set_char_flag},
			{"tiff",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL}
		};
	char *file_name,force_onscreen_flag,landscape_flag,portrait_flag,
		postscript_flag,rgb_flag,tiff_flag;
	int return_code;
	static struct File_open_data *file_open_data=(struct File_open_data *)NULL;
	static struct Write_graphics_window_data write_graphics_window_data;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_gfx_print);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			file_name=(char *)NULL;
			write_graphics_window_data.height = 0;
			force_onscreen_flag = 0;
			landscape_flag=0;
			portrait_flag=0;
			write_graphics_window_data.width = 0;
			/* default is postscript (see later) */
			postscript_flag=0;
			rgb_flag=0;
			tiff_flag=0;
			/* must have at least one graphics_window to print */
			if (write_graphics_window_data.window=FIRST_OBJECT_IN_MANAGER_THAT(
				Graphics_window)((MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
					(void *)NULL,command_data->graphics_window_manager))
			{
				ACCESS(Graphics_window)(write_graphics_window_data.window);
			}
			(option_table[0]).to_be_modified= &file_name;
			(option_table[1]).to_be_modified= &force_onscreen_flag;
			(option_table[2]).to_be_modified= &write_graphics_window_data.height;
			(landscape_portrait_option_table[0]).to_be_modified= &landscape_flag;
			(landscape_portrait_option_table[1]).to_be_modified= &portrait_flag;
			(option_table[3]).user_data=landscape_portrait_option_table;
			(postscript_rgb_tiff_option_table[0]).to_be_modified= &postscript_flag;
			(postscript_rgb_tiff_option_table[1]).to_be_modified= &rgb_flag;
			(postscript_rgb_tiff_option_table[2]).to_be_modified= &tiff_flag;
			(option_table[4]).user_data=postscript_rgb_tiff_option_table;
			(option_table[5]).to_be_modified= &write_graphics_window_data.width;
			(option_table[6]).to_be_modified= &write_graphics_window_data.window;
			(option_table[6]).user_data=command_data->graphics_window_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				write_graphics_window_data.force_onscreen = force_onscreen_flag;
				if (write_graphics_window_data.window)
				{
					/*???DB.  Do better for postscript|rgb|tiff */
					if (rgb_flag)
					{
						if (postscript_flag||tiff_flag)
						{
							return_code=0;
						}
						else
						{
							write_graphics_window_data.image_file_format=RGB_FILE_FORMAT;
						}
					}
					else
					{
						if (tiff_flag)
						{
							if (postscript_flag)
							{
								return_code=0;
							}
							else
							{
								write_graphics_window_data.image_file_format=TIFF_FILE_FORMAT;
							}
						}
						else
						{
							write_graphics_window_data.image_file_format=
								POSTSCRIPT_FILE_FORMAT;
						}
					}
					if (return_code)
					{
						if (landscape_flag)
						{
							if (portrait_flag)
							{
								return_code=0;
								display_message(ERROR_MESSAGE,
									"gfx print.  Only one of portrait|landscape");
							}
							else
							{
								write_graphics_window_data.image_orientation=
									LANDSCAPE_ORIENTATION;
							}
						}
						else
						{
							write_graphics_window_data.image_orientation=PORTRAIT_ORIENTATION;
						}
						if (return_code)
						{
							if (file_name)
							{
								write_Graphics_window_to_file(file_name,
									&write_graphics_window_data);
							}
							else
							{
								if (!file_open_data)
								{
									file_open_data=create_File_open_data(".ps",REGULAR,
										write_Graphics_window_to_file,
										(void *)&write_graphics_window_data,0,
										command_data->user_interface);
								}
								if (file_open_data)
								{
									open_file_and_write((Widget)NULL,(XtPointer)file_open_data,
										(XtPointer)NULL);
								}
								else
								{
									display_message(ERROR_MESSAGE,
								"execute_command_gfx_print.  Could not create file_open_data");
									return_code=0;
								}
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Only one of postscript|rgb|tiff");
					}
					if (file_name)
					{
						DEALLOCATE(file_name);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"No graphics windows to print");
					return_code=0;
				}
			} /* parse error, help */
			if (write_graphics_window_data.window)
			{
				DEACCESS(Graphics_window)(&write_graphics_window_data.window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_print.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_print.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_print */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_read_Control_curve(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Reads a curve from 3 files: ~.curve.com, ~.curve.exnode, ~.curve.exelem, where
~ is the name of the curve/file specified here.
Works by executing the .curve.com file, which should have a gfx define curve
instruction to read in the mesh.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Modifier_entry *entry;

	ENTER(gfx_read_Control_curve);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(entry=command_data->set_file_name_option_table))
		{
			file_name=(char *)NULL;
			while (entry->option)
			{
				entry->to_be_modified= &file_name;
				entry++;
			}
			entry->to_be_modified= &file_name;
			if (return_code=process_multiple_options(state,
				command_data->set_file_name_option_table))
			{
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".curve.com",
						command_data->user_interface)))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					/* open the file */
					return_code=check_suffix(&file_name,".curve.com")&&
						execute_comfile(file_name,command_data->execute_command);
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_read_Control_curve.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_Control_curve.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_Control_curve */

static int gfx_read_data(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 September 1999

DESCRIPTION :
If a data file is not specified a file selection box is presented to the user,
otherwise the data file is read.
???DB.  Almost identical to gfx_read_nodes.  Could set up struct Read_nodes_data
	to combine, but will probably be adding ipdata format
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct File_read_FE_node_group_data data;
	struct Modifier_entry *entry;

	ENTER(gfx_read_data);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(entry=command_data->set_file_name_option_table))
		{
			file_name=(char *)NULL;
			while (entry->option)
			{
				entry->to_be_modified= &file_name;
				entry++;
			}
			entry->to_be_modified= &file_name;
			if (return_code=process_multiple_options(state,
				command_data->set_file_name_option_table))
			{
				data.fe_field_manager=command_data->fe_field_manager;
				data.element_group_manager=command_data->element_group_manager;
				/*???RC note swapping node and data manager stuff - extends to
				  also creating a node_group for the new data group! */
				data.node_manager=command_data->data_manager;
				data.element_manager=command_data->element_manager;
				data.node_group_manager=command_data->data_group_manager;
				data.data_group_manager=command_data->node_group_manager;
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".exdata",
						command_data->user_interface)))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					/* open the file */
					if (return_code=check_suffix(&file_name,".exdata"))
					{
						return_code=file_read_FE_node_group(file_name,(void *)&data);
					}
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_read_data.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_data.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_data */

static int gfx_read_elements(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 22 April 1999

DESCRIPTION :
If an element file is not specified a file selection box is presented to the
user, otherwise the elements file is read.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct File_read_FE_element_group_data data;
	struct Modifier_entry *entry;

	ENTER(gfx_read_elements);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(entry=command_data->set_file_name_option_table))
		{
			file_name=(char *)NULL;
			while (entry->option)
			{
				entry->to_be_modified= &file_name;
				entry++;
			}
			entry->to_be_modified= &file_name;
			if (return_code=process_multiple_options(state,
				command_data->set_file_name_option_table))
			{
				data.element_manager=command_data->element_manager;
				data.element_group_manager=command_data->element_group_manager;
				data.fe_field_manager=command_data->fe_field_manager;
				data.node_manager=command_data->node_manager;
				data.node_group_manager=command_data->node_group_manager;
				data.data_group_manager=command_data->data_group_manager;
				data.basis_manager=command_data->basis_manager;
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".exelem",
						command_data->user_interface)))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					/* open the file */
					if (return_code=check_suffix(&file_name,".exelem"))
					{
						return_code=file_read_FE_element_group(file_name,(void *)&data);
					}
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_read_elements.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_elements.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_elements */

static int gfx_read_nodes(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 September 1999

DESCRIPTION :
If a nodes file is not specified a file selection box is presented to the user,
otherwise the nodes file is read.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct File_read_FE_node_group_data data;
	struct Modifier_entry *entry;

	ENTER(gfx_read_nodes);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(entry=command_data->set_file_name_option_table))
		{
			file_name=(char *)NULL;
			while (entry->option)
			{
				entry->to_be_modified= &file_name;
				entry++;
			}
			entry->to_be_modified= &file_name;
			if (return_code=process_multiple_options(state,
				command_data->set_file_name_option_table))
			{
				data.fe_field_manager=command_data->fe_field_manager;
				data.element_group_manager=command_data->element_group_manager;
				data.node_manager=command_data->node_manager;
				data.element_manager=command_data->element_manager;
				data.node_group_manager=command_data->node_group_manager;
				data.data_group_manager=command_data->data_group_manager;
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".exnode",
						command_data->user_interface)))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					/* open the file */
					if (return_code=check_suffix(&file_name,".exnode"))
					{
						return_code=file_read_FE_node_group(file_name,(void *)&data);
					}
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_read_nodes.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_nodes.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_nodes */

static int gfx_read_objects(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
If a file is not specified a file selection box is presented to the user,
otherwise the file of graphics objects is read.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct File_read_graphics_object_data data;
	struct Cmiss_command_data *command_data;
	struct Modifier_entry *entry;

	ENTER(gfx_read_objects);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(entry=command_data->set_file_name_option_table))
		{
			file_name=(char *)NULL;
			while (entry->option)
			{
				entry->to_be_modified= &file_name;
				entry++;
			}
			entry->to_be_modified= &file_name;
			if (return_code=process_multiple_options(state,
				command_data->set_file_name_option_table))
			{
				data.object_list=command_data->graphics_object_list;
				data.graphical_material_manager=
					command_data->graphical_material_manager;
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".exgobj",
						command_data->user_interface)))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					/* open the file */
					if (return_code=check_suffix(&file_name,".exgobj"))
					{
						return_code=file_read_graphics_objects(file_name,(void *)&data);
					}
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_read_objects.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_objects.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_objects */

static int gfx_read_wavefront_obj(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
If a file is not specified a file selection box is presented to the user,
otherwise the wavefront obj file is read.
==============================================================================*/
{
	char *file_name, *graphics_object_name;
	float time;
	int return_code;
	struct File_read_graphics_object_from_obj_data data;
	struct Cmiss_command_data *command_data;
	struct Modifier_entry option_table[]=
	{
		{CMGUI_EXAMPLE_DIRECTORY_SYMBOL,NULL,NULL,set_file_name},
		{"as",NULL,(void *)1,set_name},
		{"time",NULL,NULL,set_float},
		{NULL,NULL,NULL,set_file_name}
	};

	ENTER(gfx_read_wavefront_obj);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void))
		{
			graphics_object_name=0;
			time = 0;
			file_name=(char *)NULL;
			(option_table[0]).to_be_modified= &file_name;
			(option_table[1]).to_be_modified= &graphics_object_name;
			(option_table[2]).to_be_modified= &time;
			(option_table[3]).to_be_modified= &file_name;
			if (return_code=process_multiple_options(state,option_table))
			{
				data.object_list=command_data->graphics_object_list;
				data.graphical_material_manager=
					command_data->graphical_material_manager;
				data.time = time;
				if(graphics_object_name)
				{
					data.graphics_object_name = graphics_object_name;
				}
				else
				{
					data.graphics_object_name = file_name;
				}
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".obj",
						command_data->user_interface)))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					/* open the file */
					if (return_code=check_suffix(&file_name,".obj"))
					{
						return_code=file_read_voltex_graphics_object_from_obj(file_name,(void *)&data);
					}
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_read_wavefront_obj.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_wavefront_obj.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_wavefront_obj */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_read(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Executes a GFX READ command.
==============================================================================*/
{
	int i,return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"curve",NULL,NULL,gfx_read_Control_curve},
		{"data",NULL,NULL,gfx_read_data},
#if !defined (WINDOWS_DEV_FLAG)
		{"elements",NULL,NULL,gfx_read_elements},
#endif /* !defined (WINDOWS_DEV_FLAG) */
		{"nodes",NULL,NULL,gfx_read_nodes},
#if !defined (WINDOWS_DEV_FLAG)
		{"objects",NULL,NULL,gfx_read_objects},
#endif /* !defined (WINDOWS_DEV_FLAG) */
#if !defined (WINDOWS_DEV_FLAG)
		{"wavefront_obj",NULL,NULL,gfx_read_wavefront_obj},
#endif /* !defined (WINDOWS_DEV_FLAG) */
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_command_gfx_read);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			i=0;
			(option_table[i]).user_data=command_data_void;
			i++;
			(option_table[i]).user_data=command_data_void;
			i++;
#if !defined (WINDOWS_DEV_FLAG)
			(option_table[i]).user_data=command_data_void;
			i++;
#endif /* !defined (WINDOWS_DEV_FLAG) */
			(option_table[i]).user_data=command_data_void;
			i++;
#if !defined (WINDOWS_DEV_FLAG)
			(option_table[i]).user_data=command_data_void;
			i++;
#endif /* !defined (WINDOWS_DEV_FLAG) */
#if !defined (WINDOWS_DEV_FLAG)
			(option_table[i]).user_data=command_data_void;
			i++;
#endif /* !defined (WINDOWS_DEV_FLAG) */
			return_code=process_option(state,option_table);
		}
		else
		{
			set_command_prompt("gfx read",command_data->command_window);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_read.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_read */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int execute_command_gfx_select(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 26 May 2000

DESCRIPTION :
Executes a GFX SELECT command.
==============================================================================*/
{
	char *ranges_string;
	int i,j,number_of_ranges,number_selected,return_code,start,stop,
		total_number_in_ranges;
	struct CM_element_information cm;
	struct Cmiss_command_data *command_data;
	struct Element_point_ranges *element_point_ranges;
	struct FE_element *element;
	struct FE_element_grid_to_Element_point_ranges_list_data grid_to_list_data;
	struct FE_field *grid_field;
	struct FE_node *node;
	struct Multi_range *data_ranges,*element_ranges,*face_ranges,
		*grid_point_ranges,*line_ranges,*multi_range,*node_ranges;
	struct Option_table *option_table;
	struct Set_FE_field_conditional_data set_grid_field_data;

	ENTER(execute_command_gfx_select);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			element_point_ranges=(struct Element_point_ranges *)NULL;
			data_ranges=CREATE(Multi_range)();
			element_ranges=CREATE(Multi_range)();
			face_ranges=CREATE(Multi_range)();
			grid_point_ranges=CREATE(Multi_range)();
			line_ranges=CREATE(Multi_range)();
			node_ranges=CREATE(Multi_range)();
			if ((grid_field=FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(
				"grid_point_number",command_data->fe_field_manager))&&
				FE_field_is_1_component_integer(grid_field,(void *)NULL))
			{
				ACCESS(FE_field)(grid_field);
			}
			else
			{
				grid_field=(struct FE_field *)NULL;
			}
			option_table=CREATE(Option_table)();
			Option_table_add_entry(option_table,"data",data_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"elements",element_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"faces",face_ranges,
				(void *)NULL,set_Multi_range);
			set_grid_field_data.fe_field_manager=command_data->fe_field_manager;
			set_grid_field_data.conditional_function=FE_field_is_1_component_integer;
			set_grid_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"grid_field",
				&grid_field,&set_grid_field_data,set_FE_field_conditional);
			Option_table_add_entry(option_table,"grid_points",grid_point_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"lines",line_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"nodes",node_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"points",&element_point_ranges,
				(void *)command_data->element_manager,set_Element_point_ranges);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				/* nodes */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(data_ranges)))
				{
					number_selected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_node)(command_data->data_manager)<
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string=Multi_range_get_ranges_string(data_ranges);
						/* take numbers not in the manager away from data_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						multi_range=CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_node)(
							add_FE_node_number_to_Multi_range,(void *)multi_range,
							command_data->data_manager))
						{
							Multi_range_intersect(data_ranges,multi_range);
						}
						DESTROY(Multi_range)(&multi_range);
					}
					FE_node_selection_begin_cache(command_data->data_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(data_ranges);
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(data_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							if (node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,
								cm_node_identifier)(j,command_data->data_manager))
							{
								if (FE_node_selection_select_node(
									command_data->data_selection,node))
								{
									number_selected++;
								}
							}
						}
					}
					if (number_selected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(data_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d data points selected from %s",number_selected,ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_node_selection_end_cache(command_data->data_selection);
				}
				/* element_points */
				if (element_point_ranges)
				{
					Element_point_ranges_selection_select_element_point_ranges(
						command_data->element_point_ranges_selection,element_point_ranges);
				}
				/* elements */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(element_ranges)))
				{
					number_selected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_element)(command_data->element_manager)<
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string=Multi_range_get_ranges_string(element_ranges);
						/* take numbers not in the manager away from element_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						multi_range=CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							add_FE_element_element_number_to_Multi_range,(void *)multi_range,
							command_data->element_manager))
						{
							Multi_range_intersect(element_ranges,multi_range);
						}
						DESTROY(Multi_range)(&multi_range);
					}
					FE_element_selection_begin_cache(command_data->element_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(element_ranges);
					cm.type=CM_ELEMENT;
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(element_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							cm.number=j;
							if (element=FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,identifier)(
								&cm,command_data->element_manager))
							{
								if (FE_element_selection_select_element(
									command_data->element_selection,element))
								{
									number_selected++;
								}
							}
						}
					}
					if (number_selected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(element_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d element(s) selected from %s",number_selected,ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_element_selection_end_cache(command_data->element_selection);
				}
				/* faces */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(face_ranges)))
				{
					number_selected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_element)(command_data->element_manager)<
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string=Multi_range_get_ranges_string(face_ranges);
						/* take numbers not in the manager away from face_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						multi_range=CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							add_FE_element_face_number_to_Multi_range,(void *)multi_range,
							command_data->element_manager))
						{
							Multi_range_intersect(face_ranges,multi_range);
						}
						DESTROY(Multi_range)(&multi_range);
					}
					FE_element_selection_begin_cache(command_data->element_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(face_ranges);
					cm.type=CM_FACE;
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(face_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							cm.number=j;
							if (element=FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,identifier)(
								&cm,command_data->element_manager))
							{
								if (FE_element_selection_select_element(
									command_data->element_selection,element))
								{
									number_selected++;
								}
							}
						}
					}
					if (number_selected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(face_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d face(s) selected from %s",number_selected,ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_element_selection_end_cache(command_data->element_selection);
				}
				/* grid_points */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(grid_point_ranges)))
				{
					if (grid_field)
					{
						if (grid_to_list_data.element_point_ranges_list=
							CREATE(LIST(Element_point_ranges))())
						{
							grid_to_list_data.grid_fe_field=grid_field;
							grid_to_list_data.grid_value_ranges=grid_point_ranges;
							/* inefficient: go through every element in manager */
							FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
								FE_element_grid_to_Element_point_ranges_list,
								(void *)&grid_to_list_data,command_data->element_manager);
							if (0<NUMBER_IN_LIST(Element_point_ranges)(
								grid_to_list_data.element_point_ranges_list))
							{
								Element_point_ranges_selection_begin_cache(
									command_data->element_point_ranges_selection);
								FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
									Element_point_ranges_select,
									(void *)command_data->element_point_ranges_selection,
									grid_to_list_data.element_point_ranges_list);
								Element_point_ranges_selection_end_cache(
									command_data->element_point_ranges_selection);
							}
							DESTROY(LIST(Element_point_ranges))(
								&(grid_to_list_data.element_point_ranges_list));
						}
						else
						{
							display_message(ERROR_MESSAGE,"execute_command_gfx_select.  "
								"Could not create grid_point list");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"To select grid_points, "
							"need integer grid_field (eg. grid_point_number)");
					}
				}
				/* lines */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(line_ranges)))
				{
					number_selected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_element)(command_data->element_manager)<
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string=Multi_range_get_ranges_string(line_ranges);
						/* take numbers not in the manager away from line_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						multi_range=CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							add_FE_element_line_number_to_Multi_range,(void *)multi_range,
							command_data->element_manager))
						{
							Multi_range_intersect(line_ranges,multi_range);
						}
						DESTROY(Multi_range)(&multi_range);
					}
					FE_element_selection_begin_cache(command_data->element_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(line_ranges);
					cm.type=CM_LINE;
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(line_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							cm.number=j;
							if (element=FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,identifier)(
								&cm,command_data->element_manager))
							{
								if (FE_element_selection_select_element(
									command_data->element_selection,element))
								{
									number_selected++;
								}
							}
						}
					}
					if (number_selected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(line_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d line(s) selected from %s",number_selected,ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_element_selection_end_cache(command_data->element_selection);
				}
				/* nodes */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(node_ranges)))
				{
					number_selected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_node)(command_data->node_manager)<
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string=Multi_range_get_ranges_string(node_ranges);
						/* take numbers not in the manager away from node_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						multi_range=CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_node)(
							add_FE_node_number_to_Multi_range,(void *)multi_range,
							command_data->node_manager))
						{
							Multi_range_intersect(node_ranges,multi_range);
						}
						DESTROY(Multi_range)(&multi_range);
					}
					FE_node_selection_begin_cache(command_data->node_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(node_ranges);
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(node_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							if (node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,
								cm_node_identifier)(j,command_data->node_manager))
							{
								if (FE_node_selection_select_node(
									command_data->node_selection,node))
								{
									number_selected++;
								}
							}
						}
					}
					if (number_selected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(node_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d node(s) selected from %s",number_selected,ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_node_selection_end_cache(command_data->node_selection);
				}
			}
			DESTROY(Option_table)(&option_table);
			if (grid_field)
			{
				DEACCESS(FE_field)(&grid_field);
			}
			DESTROY(Multi_range)(&node_ranges);
			DESTROY(Multi_range)(&line_ranges);
			DESTROY(Multi_range)(&grid_point_ranges);
			DESTROY(Multi_range)(&face_ranges);
			DESTROY(Multi_range)(&element_ranges);
			DESTROY(Multi_range)(&data_ranges);
			if (element_point_ranges)
			{
				DESTROY(Element_point_ranges)(&element_point_ranges);
			}
		}
		else
		{
			set_command_prompt("gfx select",command_data->command_window);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_select.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_select */

static int execute_command_gfx_unselect(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 26 May 2000

DESCRIPTION :
Executes a GFX UNSELECT command.
==============================================================================*/
{
	char *ranges_string;
	int i,j,number_of_ranges,number_unselected,return_code,start,stop,
		total_number_in_ranges;
	struct CM_element_information cm;
	struct Cmiss_command_data *command_data;
	struct Element_point_ranges *element_point_ranges;
	struct FE_element *element;
	struct FE_element_grid_to_Element_point_ranges_list_data grid_to_list_data;
	struct FE_field *grid_field;
	struct FE_node *node;
	struct Multi_range *data_ranges,*element_ranges,*face_ranges,
		*grid_point_ranges,*line_ranges,*multi_range,*node_ranges;
	struct Option_table *option_table;
	struct Set_FE_field_conditional_data set_grid_field_data;

	ENTER(execute_command_gfx_unselect);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			element_point_ranges=(struct Element_point_ranges *)NULL;
			data_ranges=CREATE(Multi_range)();
			element_ranges=CREATE(Multi_range)();
			face_ranges=CREATE(Multi_range)();
			grid_point_ranges=CREATE(Multi_range)();
			line_ranges=CREATE(Multi_range)();
			node_ranges=CREATE(Multi_range)();
			if ((grid_field=FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(
				"grid_point_number",command_data->fe_field_manager))&&
				FE_field_is_1_component_integer(grid_field,(void *)NULL))
			{
				ACCESS(FE_field)(grid_field);
			}
			else
			{
				grid_field=(struct FE_field *)NULL;
			}
			option_table=CREATE(Option_table)();
			Option_table_add_entry(option_table,"data",data_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"elements",element_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"faces",face_ranges,
				(void *)NULL,set_Multi_range);
			set_grid_field_data.fe_field_manager=command_data->fe_field_manager;
			set_grid_field_data.conditional_function=FE_field_is_1_component_integer;
			set_grid_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"grid_field",
				&grid_field,&set_grid_field_data,set_FE_field_conditional);
			Option_table_add_entry(option_table,"grid_points",grid_point_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"lines",line_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"nodes",node_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"points",&element_point_ranges,
				(void *)command_data->element_manager,set_Element_point_ranges);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				/* nodes */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(data_ranges)))
				{
					number_unselected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_node)(command_data->data_manager)<
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string=Multi_range_get_ranges_string(data_ranges);
						/* take numbers not in the manager away from data_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						multi_range=CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_node)(
							add_FE_node_number_to_Multi_range,(void *)multi_range,
							command_data->data_manager))
						{
							Multi_range_intersect(data_ranges,multi_range);
						}
						DESTROY(Multi_range)(&multi_range);
					}
					FE_node_selection_begin_cache(command_data->data_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(data_ranges);
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(data_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							if (node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,
								cm_node_identifier)(j,command_data->data_manager))
							{
								/* only unselect those that are currently selected */
								if (FE_node_selection_is_node_selected(
									command_data->data_selection,node)&&
									FE_node_selection_unselect_node(
										command_data->data_selection,node))
								{
									number_unselected++;
								}
							}
						}
					}
					if (number_unselected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(data_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d data unselected from %s",number_unselected,
							ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_node_selection_end_cache(command_data->data_selection);
				}

				/* element_points */
				if (element_point_ranges)
				{
					Element_point_ranges_selection_unselect_element_point_ranges(
						command_data->element_point_ranges_selection,element_point_ranges);
				}
				/* elements */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(element_ranges)))
				{
					number_unselected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_element)(command_data->element_manager)<
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string=Multi_range_get_ranges_string(element_ranges);
						/* take numbers not in the manager away from element_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						multi_range=CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							add_FE_element_element_number_to_Multi_range,(void *)multi_range,
							command_data->element_manager))
						{
							Multi_range_intersect(element_ranges,multi_range);
						}
						DESTROY(Multi_range)(&multi_range);
					}
					FE_element_selection_begin_cache(command_data->element_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(element_ranges);
					cm.type=CM_ELEMENT;
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(element_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							cm.number=j;
							if (element=FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,identifier)(
								&cm,command_data->element_manager))
							{
								/* only unselect those that are currently selected */
								if (FE_element_selection_is_element_selected(
									command_data->element_selection,element)&&
									FE_element_selection_unselect_element(
										command_data->element_selection,element))
								{
									number_unselected++;
								}
							}
						}
					}
					if (number_unselected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(element_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d element(s) unselected from %s",number_unselected,
							ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_element_selection_end_cache(command_data->element_selection);
				}
				/* faces */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(face_ranges)))
				{
					number_unselected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_element)(command_data->element_manager)<
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string=Multi_range_get_ranges_string(face_ranges);
						/* take numbers not in the manager away from face_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						multi_range=CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							add_FE_element_face_number_to_Multi_range,(void *)multi_range,
							command_data->element_manager))
						{
							Multi_range_intersect(face_ranges,multi_range);
						}
						DESTROY(Multi_range)(&multi_range);
					}
					FE_element_selection_begin_cache(command_data->element_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(face_ranges);
					cm.type=CM_FACE;
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(face_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							cm.number=j;
							if (element=FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,identifier)(
								&cm,command_data->element_manager))
							{
								/* only unselect those that are currently selected */
								if (FE_element_selection_is_element_selected(
									command_data->element_selection,element)&&
									FE_element_selection_unselect_element(
										command_data->element_selection,element))
								{
									number_unselected++;
								}
							}
						}
					}
					if (number_unselected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(face_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d face(s) unselected from %s",number_unselected,
							ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_element_selection_end_cache(command_data->element_selection);
				}
				/* grid_points */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(grid_point_ranges)))
				{
					if (grid_field)
					{
						if (grid_to_list_data.element_point_ranges_list=
							CREATE(LIST(Element_point_ranges))())
						{
							grid_to_list_data.grid_fe_field=grid_field;
							grid_to_list_data.grid_value_ranges=grid_point_ranges;
							/* inefficient: go through every element in manager */
							FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
								FE_element_grid_to_Element_point_ranges_list,
								(void *)&grid_to_list_data,command_data->element_manager);
							if (0<NUMBER_IN_LIST(Element_point_ranges)(
								grid_to_list_data.element_point_ranges_list))
							{
								Element_point_ranges_selection_begin_cache(
									command_data->element_point_ranges_selection);
								FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
									Element_point_ranges_unselect,
									(void *)command_data->element_point_ranges_selection,
									grid_to_list_data.element_point_ranges_list);
								Element_point_ranges_selection_end_cache(
									command_data->element_point_ranges_selection);
							}
							DESTROY(LIST(Element_point_ranges))(
								&(grid_to_list_data.element_point_ranges_list));
						}
						else
						{
							display_message(ERROR_MESSAGE,"execute_command_gfx_unselect.  "
								"Could not create grid_point list");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"To unselect grid_points, "
							"need integer grid_field (eg. grid_point_number)");
					}
				}
				/* lines */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(line_ranges)))
				{
					number_unselected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_element)(command_data->element_manager)<
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string=Multi_range_get_ranges_string(line_ranges);
						/* take numbers not in the manager away from line_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						multi_range=CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							add_FE_element_line_number_to_Multi_range,(void *)multi_range,
							command_data->element_manager))
						{
							Multi_range_intersect(line_ranges,multi_range);
						}
						DESTROY(Multi_range)(&multi_range);
					}
					FE_element_selection_begin_cache(command_data->element_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(line_ranges);
					cm.type=CM_LINE;
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(line_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							cm.number=j;
							if (element=FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,identifier)(
								&cm,command_data->element_manager))
							{
								/* only unselect those that are currently selected */
								if (FE_element_selection_is_element_selected(
									command_data->element_selection,element)&&
									FE_element_selection_unselect_element(
										command_data->element_selection,element))
								{
									number_unselected++;
								}
							}
						}
					}
					if (number_unselected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(line_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d line(s) unselected from %s",number_unselected,
							ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_element_selection_end_cache(command_data->element_selection);
				}
				/* nodes */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(node_ranges)))
				{
					number_unselected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_node)(command_data->node_manager)<
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string=Multi_range_get_ranges_string(node_ranges);
						/* take numbers not in the manager away from node_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						multi_range=CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_node)(
							add_FE_node_number_to_Multi_range,(void *)multi_range,
							command_data->node_manager))
						{
							Multi_range_intersect(node_ranges,multi_range);
						}
						DESTROY(Multi_range)(&multi_range);
					}
					FE_node_selection_begin_cache(command_data->node_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(node_ranges);
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(node_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							if (node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,
								cm_node_identifier)(j,command_data->node_manager))
							{
								/* only unselect those that are currently selected */
								if (FE_node_selection_is_node_selected(
									command_data->node_selection,node)&&
									FE_node_selection_unselect_node(
										command_data->node_selection,node))
								{
									number_unselected++;
								}
							}
						}
					}
					if (number_unselected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(node_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d node(s) unselected from %s",number_unselected,
							ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_node_selection_end_cache(command_data->node_selection);
				}
			}
			DESTROY(Option_table)(&option_table);
			if (grid_field)
			{
				DEACCESS(FE_field)(&grid_field);
			}
			DESTROY(Multi_range)(&node_ranges);
			DESTROY(Multi_range)(&line_ranges);
			DESTROY(Multi_range)(&grid_point_ranges);
			DESTROY(Multi_range)(&face_ranges);
			DESTROY(Multi_range)(&element_ranges);
			DESTROY(Multi_range)(&data_ranges);
			if (element_point_ranges)
			{
				DESTROY(Element_point_ranges)(&element_point_ranges);
			}
		}
		else
		{
			set_command_prompt("gfx unselect",command_data->command_window);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_unselect.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_unselect */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_axis_length(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 9 May 1999

DESCRIPTION :
Sets the axis length from the command line.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	static struct Modifier_entry option_table[]=
	{
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,set_special_float3}
	};
	Triple axis_lengths,old_axis_lengths;

	ENTER(gfx_set_axis_length);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			scene=command_data->default_scene;
			ACCESS(Scene)(scene);
			Scene_get_axis_lengths(scene,axis_lengths);
			(option_table[0]).to_be_modified= &scene;
			(option_table[0]).user_data=command_data->scene_manager;
			(option_table[1]).to_be_modified= axis_lengths;
			(option_table[1]).user_data= "*";
			if (return_code=process_multiple_options(state,option_table))
			{
				/* update scene only if changed */
				Scene_get_axis_lengths(scene,old_axis_lengths);
				if ((axis_lengths[0] != old_axis_lengths[0])||
					(axis_lengths[1] != old_axis_lengths[1])||
					(axis_lengths[2] != old_axis_lengths[2]))
				{
					Scene_set_axis_lengths(scene,axis_lengths);
				}
			}
			DEACCESS(Scene)(&scene);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_axis_length.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_axis_length.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_axis_length */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_axis_material(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Sets the axis material from the command line.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Graphical_material *axis_material;
	struct Scene *scene;
	static struct Modifier_entry option_table[]=
	{
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,set_Graphical_material}
	};

	ENTER(gfx_set_axis_material);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			scene=command_data->default_scene;
			ACCESS(Scene)(scene);
			axis_material=Scene_get_axis_material(scene);
			ACCESS(Graphical_material)(axis_material);
			(option_table[0]).to_be_modified= &scene;
			(option_table[0]).user_data=command_data->scene_manager;
			(option_table[1]).to_be_modified= &axis_material;
			(option_table[1]).user_data=command_data->graphical_material_manager;
			if (return_code=process_multiple_options(state,option_table))
			{
				/* update scene only if changed */
				if (axis_material != Scene_get_axis_material(scene))
				{
					Scene_set_axis_material(scene,axis_material);
				}
			}
			DEACCESS(Graphical_material)(&axis_material);
			DEACCESS(Scene)(&scene);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_axis_material.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_axis_material.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_axis_material */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_axis_origin(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Sets the axis origin of a scene from the command line.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	static struct Modifier_entry option_table[]=
	{
		{"coordinate",NULL,NULL,set_Cmgui_coordinate},
		{"position",NULL,NULL,set_Dof3_position},
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,set_Dof3_position},
	};
	struct Cmgui_coordinate *coordinate;
	struct Dof3_data global_position,position;
	Triple axis_origin;

	ENTER(gfx_set_axis_origin);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			coordinate=ACCESS(Cmgui_coordinate)(global_coordinate_ptr);
			scene=command_data->default_scene;
			ACCESS(Scene)(scene);
			Scene_get_axis_origin(scene,axis_origin);
			position.data[0]=axis_origin[0];
			position.data[1]=axis_origin[1];
			position.data[2]=axis_origin[2];
			(option_table[0]).to_be_modified= &coordinate;
			(option_table[1]).to_be_modified= &position;
			(option_table[2]).to_be_modified= &scene;
			(option_table[2]).user_data=command_data->scene_manager;
			(option_table[3]).to_be_modified= &position;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				/* we have a rc position, but relative to a coordinate system we
					 must convert to global */
				get_global_position(&position,coordinate,&global_position);
				axis_origin[0]=global_position.data[0];
				axis_origin[1]=global_position.data[1];
				axis_origin[2]=global_position.data[2];
				Scene_set_axis_origin(scene,axis_origin);
				DEACCESS(Cmgui_coordinate)(&coordinate);
			}
			DEACCESS(Scene)(&scene);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_axis_origin.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_axis_origin.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_axis_origin */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if defined (OLD_CODE)
#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_axis_visibility(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Sets the axis visibility from the command line.
==============================================================================*/
{
	enum GT_visibility_type axis_visibility;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	static struct Modifier_entry option_table[]=
	{
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_set_axis_visibility);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			scene=command_data->default_scene;
			ACCESS(Scene)(scene);
			axis_visibility=Scene_get_axis_visibility(scene);
			(option_table[0]).to_be_modified= &scene;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (g_VISIBLE==Scene_get_axis_visibility(scene))
				{
					axis_visibility=g_INVISIBLE;
				}
				else
				{
					axis_visibility=g_VISIBLE;
				}
				Scene_set_axis_visibility(scene,axis_visibility);
			}
			DEACCESS(Scene)(&scene);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_axis_visibility.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_axis_visibility.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_axis_visibility */
#endif /* !defined (WINDOWS_DEV_FLAG) */
#endif /* defined (OLD_CODE) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_background(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Sets the background colour and/or texture from the command line.
???RC Obsolete.
==============================================================================*/
{
	int return_code,update;
	struct Cmiss_command_data *command_data;
	struct Colour background_colour,save_background_colour;
	struct Graphics_window *window;
	struct Scene_viewer *scene_viewer;
	struct Texture *background_texture,*save_background_texture;
	static struct Modifier_entry option_table[]=
	{
		{"texture",NULL,NULL,set_Texture},
		{"window",NULL,NULL,set_Graphics_window},
		{NULL,NULL,NULL,set_Colour}
	};

	ENTER(gfx_set_background);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/*???Debug */
			display_message(WARNING_MESSAGE,
				"gfx set background is obsolete, please use gfx modify window");
			if ((window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,command_data->graphics_window_manager))&&
				(scene_viewer=Graphics_window_get_Scene_viewer(window,0)))
			{
				ACCESS(Graphics_window)(window);
				return_code=Scene_viewer_get_background_colour(scene_viewer,
					&background_colour);
				if (background_texture=Scene_viewer_get_background_texture(
					scene_viewer))
				{
					ACCESS(Texture)(background_texture);
				}
			}
			else
			{
				background_colour.red=0.0;
				background_colour.green=0.0;
				background_colour.blue=0.0;
				background_texture=(struct Texture *)NULL;
				return_code=1;
			}
			if (return_code)
			{
				save_background_colour.red=background_colour.red;
				save_background_colour.green=background_colour.green;
				save_background_colour.blue=background_colour.blue;
				save_background_texture=background_texture;
				/* initialise defaults */
				(option_table[0]).to_be_modified= &background_texture;
				(option_table[0]).user_data=(void *)(command_data->texture_manager);
				(option_table[1]).to_be_modified= &window;
				(option_table[1]).user_data=command_data->graphics_window_manager;
				(option_table[2]).to_be_modified= &background_colour;
				return_code=process_multiple_options(state,option_table);
				/* no errors, not asking for help */
				if (return_code)
				{
					if (window)
					{
						if (scene_viewer=Graphics_window_get_Scene_viewer(window,0))
						{
							update=0;
							if ((save_background_colour.red!=background_colour.red)||
								(save_background_colour.green!=background_colour.green)||
								(save_background_colour.blue!=background_colour.blue))
							{
								update=1;
								return_code=Scene_viewer_set_background_colour(scene_viewer,
									&background_colour);
							}
							if (return_code&&(save_background_texture!=background_texture))
							{
								update=1;
								return_code=Scene_viewer_set_background_texture(scene_viewer,
									background_texture);
							}
							if (return_code&&update)
							{
								return_code=Graphics_window_update_now(window);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_set_background.  Could not get scene viewer 1");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Must have a graphics window");
						return_code=0;
					}
				} /* parse error, help */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_set_background.  Could not get background information");
				return_code=0;
			}
			if (background_texture)
			{
				DEACCESS(Texture)(&background_texture);
			}
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_background.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_background.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_background */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_far_clipping_plane(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Sets the distance to the far clipping plane from the command line.
==============================================================================*/
{
	double bottom,default_far,far,left,near,right,top;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;
	struct Scene_viewer *scene_viewer;
	static struct Modifier_entry option_table[]=
	{
		{"window",NULL,NULL,set_Graphics_window},/*???DB. "on" ? */
		{NULL,NULL,NULL,set_double}
	};

	ENTER(gfx_set_far_clipping_plane);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if ((window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,command_data->graphics_window_manager))&&
				(scene_viewer=Graphics_window_get_Scene_viewer(window,0)))
			{
				ACCESS(Graphics_window)(window);
				return_code=Scene_viewer_get_viewing_volume(scene_viewer,&left,&right,
					&bottom,&top,&near,&default_far);
			}
			else
			{
				left=right=bottom=top=near=default_far=0.0;
				return_code=1;
			}
			if (return_code)
			{
				/* initialise defaults */
				(option_table[0]).to_be_modified= &window;
				(option_table[0]).user_data=command_data->graphics_window_manager;
				(option_table[1]).to_be_modified= &default_far;
				return_code=process_multiple_options(state,option_table);
				/* no errors, not asking for help */
				if (return_code)
				{
					if (window)
					{
						if ((scene_viewer=Graphics_window_get_Scene_viewer(window,0))&&
							Scene_viewer_get_viewing_volume(scene_viewer,&left,&right,&bottom,
							&top,&near,&far)&&(default_far!=far))
							/*???DB.  Should not be changing directly because managed ? */
						{
							return_code=Scene_viewer_set_viewing_volume(scene_viewer,left,
								right,bottom,top,near,default_far);
							return_code=Graphics_window_update_now(window);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"No windows are available");
						return_code=0;
					}
				} /* parse error, help */
			}
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_far_clipping_plane`.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_set_far_clipping_plane.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_far_clipping_plane */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
int set_graphics_window_resolution(struct Parse_state *state,
	void *dummy_to_be_modified,void *graphics_window_manager_void)
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Modifier function to set the resolution of a graphics window from a command.
==============================================================================*/
{
	int height,old_height,old_width,return_code,width;
	struct Graphics_window *window;
	struct MANAGER(Graphics_window) *graphics_window_manager;
	static struct Modifier_entry option_table[]=
	{
		{"height",NULL,NULL,set_int_positive},
		{"width",NULL,NULL,set_int_positive},
		{"window",NULL,NULL,set_Graphics_window},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(set_graphics_window_resolution);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (graphics_window_manager=(struct MANAGER(Graphics_window) *)
			graphics_window_manager_void)
		{
			if (window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,graphics_window_manager))
			{
				ACCESS(Graphics_window)(window);
				Graphics_window_get_viewing_area_size(window,&width,&height);
				old_width=width;
				old_height=height;
			}
			else
			{
				height=0;
				width=0;
			}
			(option_table[0]).to_be_modified= &height;
			(option_table[1]).to_be_modified= &width;
			(option_table[2]).to_be_modified= &window;
			(option_table[2]).user_data=graphics_window_manager;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (window)
				{
					if ((width != old_width)||(height != old_height))
					{
						return_code=
							Graphics_window_set_viewing_area_size(window,width,height);
					}
					else
					{
						return_code=1;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Must have a graphics window");
					return_code=0;
				}
			} /* parse error, help */
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_graphics_window_resolution.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_graphics_window_resolution.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_graphics_window_resolution */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_interest_point(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Sets the current interest point from the command line. The view_point is also
adjusted so that the view direction and distance from it to the interest point
remains the same. The up-vector therefore does not change.
==============================================================================*/
{
	double eye[3],lookat[3],up[3],view[3];
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"coordinate",NULL,NULL,set_Cmgui_coordinate},
		{"position",NULL,NULL,set_Dof3_position},
		{"window",NULL,NULL,set_Graphics_window},
		{NULL,NULL,NULL,set_Dof3_position},
	};
	struct Cmiss_command_data *command_data;
	struct Cmgui_coordinate *coordinate;
	struct Dof3_data global_position,position;
	struct Graphics_window *window;

	ENTER(gfx_set_interest_point);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,command_data->graphics_window_manager))
			{
				ACCESS(Graphics_window)(window);
				return_code=Scene_viewer_get_lookat_parameters(
					Graphics_window_get_Scene_viewer(window,0),&eye[0],&eye[1],&eye[2],
					&lookat[0],&lookat[1],&lookat[2],&up[0],&up[1],&up[2]);
			}
			else
			{
				eye[0]=eye[1]=eye[2]=0.0;
				lookat[0]=lookat[1]=lookat[2]=0.0;
				up[0]=up[1]=up[2]=0.0;
				return_code=1;
			}
			if (return_code)
			{
				/* initialise defaults */
				coordinate=ACCESS(Cmgui_coordinate)(global_coordinate_ptr);
				position.data[0]=lookat[0];
				position.data[1]=lookat[1];
				position.data[2]=lookat[2];
				(option_table[0]).to_be_modified= &coordinate;
				(option_table[1]).to_be_modified= &position;
				(option_table[2]).to_be_modified= &window;
				(option_table[2]).user_data=command_data->graphics_window_manager;
				(option_table[3]).to_be_modified= &position;
				return_code=process_multiple_options(state,option_table);
				/* no errors, not asking for help */
				if (return_code)
				{
					if (window)
					{
						Scene_viewer_get_lookat_parameters(
							Graphics_window_get_Scene_viewer(window,0),
							&eye[0],&eye[1],&eye[2],&lookat[0],&lookat[1],&lookat[2],
							&up[0],&up[1],&up[2]);
						/* get the view direction/distance which will not change */
						view[0]=lookat[0]-eye[0];
						view[1]=lookat[1]-eye[1];
						view[2]=lookat[2]-eye[2];
						/* we have a rc position, but relative to a coordinate system we
							 must convert to global */
						get_global_position(&position,coordinate,&global_position);
						lookat[0]=global_position.data[0];
						lookat[1]=global_position.data[1];
						lookat[2]=global_position.data[2];
						/* make sure the view_point is still the same direction and distance
							 from the interest point. */
						eye[0]=lookat[0]-view[0];
						eye[1]=lookat[1]-view[1];
						eye[2]=lookat[2]-view[2];
						if (return_code=Scene_viewer_set_lookat_parameters(
							Graphics_window_get_Scene_viewer(window,0),eye[0],eye[1],eye[2],
							lookat[0],lookat[1],lookat[2],up[0],up[1],up[2]))
							/*???DB.  Should not be changing directly because managed ? */
						{
							return_code=Graphics_window_update_now(window);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Must have a graphics window");
						return_code=0;
					}
				} /* parse error, help */
				DEACCESS(Cmgui_coordinate)(&coordinate);
			}
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_interest_point.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_interest_point.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_interest_point */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_near_clipping_plane(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Sets the distance to the near clipping plane from the command line.
==============================================================================*/
{
	double bottom,default_near,far,left,near,right,top;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;
	struct Scene_viewer *scene_viewer;
	static struct Modifier_entry option_table[]=
	{
		{"window",NULL,NULL,set_Graphics_window},/*???DB. "on" ? */
		{NULL,NULL,NULL,set_double}
	};

	ENTER(gfx_set_near_clipping_plane);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if ((window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,command_data->graphics_window_manager))&&
				(scene_viewer=Graphics_window_get_Scene_viewer(window,0)))
			{
				ACCESS(Graphics_window)(window);
				return_code=Scene_viewer_get_viewing_volume(scene_viewer,&left,&right,
					&bottom,&top,&default_near,&far);
			}
			else
			{
				left=right=bottom=top=default_near=far=0.0;
				return_code=1;
			}
			if (return_code)
			{
				/* initialise defaults */
				(option_table[0]).to_be_modified= &window;
				(option_table[0]).user_data=command_data->graphics_window_manager;
				(option_table[1]).to_be_modified= &default_near;
				return_code=process_multiple_options(state,option_table);
				/* no errors, not asking for help */
				if (return_code)
				{
					if (window)
					{
						if ((scene_viewer=Graphics_window_get_Scene_viewer(window,0))&&
							Scene_viewer_get_viewing_volume(scene_viewer,&left,&right,&bottom,
							&top,&near,&far)&&(default_near!=near))
							/*???DB.  Should not be changing directly because managed ? */
						{
							return_code=Scene_viewer_set_viewing_volume(scene_viewer,left,
								right,bottom,top,default_near,far);
							return_code=Graphics_window_update_now(window);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"No windows are available");
						return_code=0;
					}
				} /* parse error, help */
			}
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_near_clipping_plane`.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_set_near_clipping_plane.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_near_clipping_plane */
#endif /* !defined (WINDOWS_DEV_FLAG) */

int gfx_set_FE_nodal_value(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 30 August 1999

DESCRIPTION :
Sets nodal field values from a command.
???DB.  Should it be here ?
==============================================================================*/
{
	char *current_token;
	enum FE_nodal_value_type fe_nodal_d_ds1,fe_nodal_d_ds2,fe_nodal_d_ds3,
		fe_nodal_d2_ds1ds2,fe_nodal_d2_ds1ds3,fe_nodal_d2_ds2ds3,
		fe_nodal_d3_ds1ds2ds3,fe_nodal_value,value_type;
	FE_value value;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"value",NULL,NULL,set_enum},
		{"d/ds1",NULL,NULL,set_enum},
		{"d/ds2",NULL,NULL,set_enum},
		{"d/ds3",NULL,NULL,set_enum},
		{"d2/ds1ds2",NULL,NULL,set_enum},
		{"d2/ds1ds3",NULL,NULL,set_enum},
		{"d2/ds2ds3",NULL,NULL,set_enum},
		{"d3/ds1ds2ds3",NULL,NULL,set_enum},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct FE_field_component component;
	struct FE_node *node,*node_copy;

	ENTER(gfx_set_FE_nodal_value);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		node=(struct FE_node *)NULL;
		if (return_code=set_FE_node(state,(void *)&node,
			(void *)(command_data->node_manager)))
		{
			component.field=(struct FE_field *)NULL;
			component.number=0;
			if (return_code=set_FE_field_component(state,(void *)&component,
				(void *)(command_data->fe_field_manager)))
			{
				value_type=FE_NODAL_UNKNOWN;
				option_table[0].to_be_modified= &value_type;
				fe_nodal_value=FE_NODAL_VALUE;
				option_table[0].user_data= &fe_nodal_value;
				option_table[1].to_be_modified= &value_type;
				fe_nodal_d_ds1=FE_NODAL_D_DS1;
				option_table[1].user_data= &fe_nodal_d_ds1;
				option_table[2].to_be_modified= &value_type;
				fe_nodal_d_ds2=FE_NODAL_D_DS2;
				option_table[2].user_data= &fe_nodal_d_ds2;
				option_table[3].to_be_modified= &value_type;
				fe_nodal_d_ds3=FE_NODAL_D_DS3;
				option_table[3].user_data= &fe_nodal_d_ds3;
				option_table[4].to_be_modified= &value_type;
				fe_nodal_d2_ds1ds2=FE_NODAL_D2_DS1DS2;
				option_table[4].user_data= &fe_nodal_d2_ds1ds2;
				option_table[5].to_be_modified= &value_type;
				fe_nodal_d2_ds1ds3=FE_NODAL_D2_DS1DS3;
				option_table[5].user_data= &fe_nodal_d2_ds1ds3;
				option_table[6].to_be_modified= &value_type;
				fe_nodal_d2_ds2ds3=FE_NODAL_D2_DS2DS3;
				option_table[6].user_data= &fe_nodal_d2_ds2ds3;
				option_table[7].to_be_modified= &value_type;
				fe_nodal_d3_ds1ds2ds3=FE_NODAL_D3_DS1DS2DS3;
				option_table[7].user_data= &fe_nodal_d3_ds1ds2ds3;
				if (return_code=process_option(state,option_table))
				{
					if (current_token=state->current_token)
					{
						if (1==sscanf(current_token,FE_VALUE_INPUT_STRING,&value))
						{
							if ((node_copy=CREATE(FE_node)(0,(struct FE_node *)NULL))
								&&COPY(FE_node)(node_copy,node))
							{
								if (return_code=set_FE_nodal_FE_value_value(node_copy,
									&component,0,value_type,value))
								{
									return_code=MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,
										cm_node_identifier)(node,node_copy,command_data->node_manager);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_set_FE_nodal_value.  Could not duplicate node");
								return_code=0;
							}
							DESTROY(FE_node)(&node_copy);
						}
						else
						{
							display_message(ERROR_MESSAGE,"Invalid nodal value %s",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"Missing value for node");
						display_parse_state_location(state);
						return_code=1;
					}
				}
				else
				{
					if ((current_token=state->current_token)&&
						!(strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
					{
						display_message(INFORMATION_MESSAGE," #\n");
						return_code=1;
					}
				}
			}
			DEACCESS(FE_node)(&node);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_FE_nodal_value.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_FE_nodal_value */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_scene_order(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Sets the ordering of graphics objects on scene(s) from the command line.
==============================================================================*/
{
	char *name;
	int return_code,position;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	struct Scene_object *scene_object;
	static struct Modifier_entry option_table[]=
	{
		{"object",NULL,(void *)1,set_name},
		{"position",NULL,NULL,set_int},
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,set_name}
	};

	ENTER(gfx_set_scene_order);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			name=(char *)NULL;
			position=0;
			scene=command_data->default_scene;
			(option_table[0]).to_be_modified= &name;
			(option_table[1]).to_be_modified= &position;
			(option_table[2]).to_be_modified= &scene;
			(option_table[2]).user_data=command_data->scene_manager;
			(option_table[3]).to_be_modified= &name;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (name)
				{
					if (scene_object=Scene_get_scene_object_by_name(scene,name))
					{
						return_code=Scene_set_scene_object_position(scene,scene_object,
							position);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"No graphics object named '%s' in scene",name);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing graphics object name");
					return_code=0;
				}
			} /* parse error,help */
			if (name)
			{
				DEALLOCATE(name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_scene_order.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_set_scene_order.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_scene_order */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_time(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Sets the time from the command line.
==============================================================================*/
{
	char *timekeeper_name;
	float time;
	int return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"timekeeper",NULL,NULL,set_name},
		{NULL,NULL,NULL,set_float}
	};

	ENTER(gfx_set_time);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			ALLOCATE(timekeeper_name,char,10);
			/* there is only a default timekeeper at the moment but I am making the
				commands with a timekeeper manager in mind */
			strcpy(timekeeper_name,"default");
			if (command_data->default_time_keeper)
			{
				time=Time_keeper_get_time(command_data->default_time_keeper);
			}
			else
			{
				time = 0;
			}
			(option_table[0]).to_be_modified= &timekeeper_name;
			(option_table[1]).to_be_modified= &time;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				/* the old routine only use to call this if the time wasn't the
					same as the default time, but the timekeeper might not be the
					default */
				Time_keeper_request_new_time(command_data->default_time_keeper,time);
			} /* parse error, help */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_time.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_time.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_time */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int set_transformation_matrix(struct Parse_state *state,
	void *transformation_matrix_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Sets a transformation matrix from the command line.
==============================================================================*/
{
	char *current_token;
	gtMatrix *transformation_matrix;
	int i,j,return_code;

	ENTER(set_transformation_matrix);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if ((current_token=state->current_token)&&(
			!strcmp(PARSER_HELP_STRING,current_token)||
			!strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
		{
			display_message(INFORMATION_MESSAGE,"# # # # # # # # # # # # # # # #");
			if (transformation_matrix=(gtMatrix *)transformation_matrix_void)
			{
				display_message(INFORMATION_MESSAGE,
					" [%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g]",
					(*transformation_matrix)[0][0],(*transformation_matrix)[0][1],
					(*transformation_matrix)[0][2],(*transformation_matrix)[0][3],
					(*transformation_matrix)[1][0],(*transformation_matrix)[1][1],
					(*transformation_matrix)[1][2],(*transformation_matrix)[1][3],
					(*transformation_matrix)[2][0],(*transformation_matrix)[2][1],
					(*transformation_matrix)[2][2],(*transformation_matrix)[2][3],
					(*transformation_matrix)[3][0],(*transformation_matrix)[3][1],
					(*transformation_matrix)[3][2],(*transformation_matrix)[3][3]);
			}
			return_code=1;
		}
		else
		{
			if (transformation_matrix=(gtMatrix *)transformation_matrix_void)
			{
				return_code=1;
				i=0;
				while ((i<4)&&return_code&&current_token)
				{
					j=0;
					while ((j<4)&&return_code&&current_token)
					{
						if (1==sscanf(current_token,"%f",&((*transformation_matrix)[i][j])))
						{
							shift_Parse_state(state,1);
							current_token=state->current_token;
						}
						else
						{
							return_code=0;
						}
						j++;
					}
					i++;
				}
				if (!return_code||(i<4)||(j<4))
				{
					if (current_token)
					{
						display_message(ERROR_MESSAGE,
							"Error reading transformation matrix: %s",current_token);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Error reading transformation matrix");
					}
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_transformation_matrix.  Missing transformation_matrix");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_transformation_matrix.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_transformation_matrix */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_transformation(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Sets the transformation for a graphics object from the command line.
==============================================================================*/
{
	char *scene_object_name;
	gtMatrix transformation_matrix;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	struct Scene_object *scene_object;
	static struct Modifier_entry option_table[]=
	{
		{"name",NULL,(void *)1,set_name},
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,set_transformation_matrix}
	};

	ENTER(gfx_set_transformation);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			scene_object_name=(char *)NULL;
			scene=ACCESS(Scene)(command_data->default_scene);
			transformation_matrix[0][0]=1;
			transformation_matrix[0][1]=0;
			transformation_matrix[0][2]=0;
			transformation_matrix[0][3]=0;
			transformation_matrix[1][0]=0;
			transformation_matrix[1][1]=1;
			transformation_matrix[1][2]=0;
			transformation_matrix[1][3]=0;
			transformation_matrix[2][0]=0;
			transformation_matrix[2][1]=0;
			transformation_matrix[2][2]=1;
			transformation_matrix[2][3]=0;
			transformation_matrix[3][0]=0;
			transformation_matrix[3][1]=0;
			transformation_matrix[3][2]=0;
			transformation_matrix[3][3]=1;
			/* parse the command line */
			(option_table[0]).to_be_modified= &scene_object_name;
			(option_table[1]).to_be_modified= &scene;
			(option_table[1]).user_data=command_data->scene_manager;
			(option_table[2]).to_be_modified= &transformation_matrix;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (scene_object_name)
				{
					if (scene_object=Scene_get_scene_object_by_name(scene,
						scene_object_name))
					{
						Scene_object_set_transformation(scene_object,
							&transformation_matrix);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"No object named '%s' in scene",scene_object_name);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing graphics object name");
					return_code=0;
				}
			} /* parse error, help */
			DEACCESS(Scene)(&scene);
			if (scene_object_name)
			{
				DEALLOCATE(scene_object_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_transformation.  Missing command_data_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_transformation.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_transformation */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_up_vector(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Sets the current view point from the command line.
==============================================================================*/
{
	double dot_prod,eye[3],lookat[3],norm[3],up[3],view[3];
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"coordinate",NULL,NULL,set_Cmgui_coordinate},
		{"position",NULL,NULL,set_Dof3_position},
		{"window",NULL,NULL,set_Graphics_window},
		{NULL,NULL,NULL,set_Dof3_position},
	};
	struct Cmiss_command_data *command_data;
	struct Cmgui_coordinate *coordinate;
	struct Dof3_data global_position,position;
	struct Graphics_window *window;

	ENTER(gfx_set_up_vector);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,command_data->graphics_window_manager))
			{
				ACCESS(Graphics_window)(window);
				return_code=Scene_viewer_get_lookat_parameters(
					Graphics_window_get_Scene_viewer(window,0),
					&eye[0],&eye[1],&eye[2],&lookat[0],&lookat[1],&lookat[2],
					&up[0],&up[1],&up[2]);
			}
			else
			{
				eye[0]=eye[1]=eye[2]=0.0;
				lookat[0]=lookat[1]=lookat[2]=0.0;
				up[0]=up[1]=up[2]=0.0;
				return_code=1;
			}
			if (return_code)
			{
				/* initialise defaults */
				coordinate=ACCESS(Cmgui_coordinate)(global_coordinate_ptr);
				position.data[0]=up[0];
				position.data[1]=up[1];
				position.data[2]=up[2];
				(option_table[0]).to_be_modified= &coordinate;
				(option_table[1]).to_be_modified= &position;
				(option_table[2]).to_be_modified= &window;
				(option_table[2]).user_data=command_data->graphics_window_manager;
				(option_table[3]).to_be_modified= &position;
				return_code=process_multiple_options(state,option_table);
				/* no errors, not asking for help */
				if (return_code)
				{
					if (window)
					{
						Scene_viewer_get_lookat_parameters(
							Graphics_window_get_Scene_viewer(window,0),
							&eye[0],&eye[1],&eye[2],&lookat[0],&lookat[1],&lookat[2],
							&up[0],&up[1],&up[2]);
						/* we have a rc position, but relative to a coordinate system we
							must convert to global */
						get_global_position(&position,coordinate,&global_position);
						up[0]=global_position.data[0];
						up[1]=global_position.data[1];
						up[2]=global_position.data[2];
						/* get unit vector in view direction */
						view[0]=lookat[0]-eye[0];
						view[1]=lookat[1]-eye[1];
						view[2]=lookat[2]-eye[2];
						normalize3(view);
						normalize3(up);
						/* make sure view direction not colinear with up-vector */
						dot_prod=fabs(dot_product3(view,up));
						if (dot_prod<0.99999)
						{
							/* ensure up vector is normal to view direction */
							if (dot_prod>0.00001)
							{
								cross_product3(up,view,norm);
								cross_product3(view,norm,up);
								normalize3(up);
								display_message(WARNING_MESSAGE,
									"gfx set up_vector.  Adjusting up_vector to (%g,%g,%g) so it "
									"remains normal to view direction",up[0],up[1],up[2]);
							}
							if (return_code=Scene_viewer_set_lookat_parameters(
								Graphics_window_get_Scene_viewer(window,0),eye[0],eye[1],eye[2],
								lookat[0],lookat[1],lookat[2],up[0],up[1],up[2]))
								/*???DB.  Should not be changing directly because managed ? */
							{
								return_code=Graphics_window_update_now(window);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx set up_vector.  Up vector colinear with view direction. "
								"Set interest_point and view_point first.");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Must have a graphics window");
						return_code=0;
					}
				} /* parse error, help */
				DEACCESS(Cmgui_coordinate)(&coordinate);
			}
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_up_vector.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_up_vector.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_up_vector */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static void update_callback(struct MANAGER_MESSAGE(FE_node) *message,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes "open comfile update execute"
???RC Is this used?
==============================================================================*/
{
	struct Cmiss_command_data *command_data;

	ENTER(update_callback);
	USE_PARAMETER(message);
	if (command_data=(struct Cmiss_command_data *)command_data_void)
	{
		Execute_command_execute_string(command_data->execute_command, "open comfile update execute");
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_callback.  Missing command_data_void");
	}
} /* update_callback */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_update_callback(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 20 January 1998

DESCRIPTION :
Sets the update callback on/off.  When the update callback is on, the command
	open comfile update execute
is issued whenever a node is changed.
==============================================================================*/
{
	char *current_token;
	int return_code;
	static void *callback_id=NULL;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_set_update_callback);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((current_token=state->current_token)&&(
			!strcmp(PARSER_HELP_STRING,current_token)||
			!strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
		{
			if (callback_id)
			{
				display_message(INFORMATION_MESSAGE,"on");
			}
			else
			{
				display_message(INFORMATION_MESSAGE,"off");
			}
			display_message(INFORMATION_MESSAGE,
" ! when on, \"open comfile update execute\" is issued whenever a node is changed");
		}
		else
		{
			display_message(WARNING_MESSAGE,"This is a temporary command");
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (callback_id)
				{
					if (MANAGER_DEREGISTER(FE_node)(callback_id,
						command_data->node_manager))
					{
						callback_id=NULL;
					}
				}
				else
				{
					callback_id=MANAGER_REGISTER(FE_node)(update_callback,
						command_data_void,command_data->node_manager);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_set_update_callback.  Missing command_data_void");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_update_callback.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_update_callback */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_video(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 12 August 1998

DESCRIPTION :
Sets the video on/off for a window from the command line.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;
	static struct Modifier_entry option_table[]=
	{
		{"window",NULL,NULL,set_Graphics_window},/*???DB. "on" ? */
		{NULL,NULL,NULL,NULL} /*???DB.  set_name ? */
	};

	ENTER(gfx_set_video);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			/* must have at least one graphics_window to print */
			if (window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,(void *)NULL,
				command_data->graphics_window_manager))
			{
				ACCESS(Graphics_window)(window);
			}
			/* parse the command line */
			(option_table[0]).to_be_modified= &window;
			(option_table[0]).user_data=command_data->graphics_window_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (window)
				{
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"gfx set video command temporarily disabled\n");
					return_code=0;
#if defined (OLD_GFX_WINDOW)
					return_code=set_video_on_off(window);
#endif /* defined (OLD_GFX_WINDOW) */
				}
				else
				{
					display_message(ERROR_MESSAGE,"Must have a graphics window");
					return_code=0;
				}
			} /* parse error, help */
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_video.  Missing command_data_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_video.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_video */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_view_angle(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Sets the view angle from the command line.
==============================================================================*/
{
	float view_angle;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;
	static struct Modifier_entry option_table[]=
	{
		{"window",NULL,NULL,set_Graphics_window},/*???DB. "on" ? */
		{NULL,NULL,NULL,set_float}
	};

	ENTER(gfx_set_view_angle);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,command_data->graphics_window_manager))
			{
				ACCESS(Graphics_window)(window);
			}
			view_angle=90;
			/* parse the command line */
			(option_table[0]).to_be_modified= &window;
			(option_table[1]).to_be_modified= &view_angle;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (window)
				{
					view_angle *= PI/180.0;
					if (return_code=Scene_viewer_set_view_angle(
						Graphics_window_get_Scene_viewer(window,0),view_angle))
					{
						return_code=Graphics_window_update_now(window);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Must have a graphics window");
					return_code=0;
				}
			} /* parse error, help */
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_view_angle.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_view_angle.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_view_angle */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_view_point(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Sets the current view point from the command line.
==============================================================================*/
{
	double eye[3],lookat[3],norm[3],oldview[3],up[3],view[3];
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"coordinate",NULL,NULL,set_Cmgui_coordinate},
		{"position",NULL,NULL,set_Dof3_position},
		{"window",NULL,NULL,set_Graphics_window},
		{NULL,NULL,NULL,set_Dof3_position},
	};
	struct Cmiss_command_data *command_data;
	struct Cmgui_coordinate *coordinate;
	struct Dof3_data global_position,position;
	struct Graphics_window *window;

	ENTER(gfx_set_view_point);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,command_data->graphics_window_manager))
			{
				ACCESS(Graphics_window)(window);
				return_code=Scene_viewer_get_lookat_parameters(
					Graphics_window_get_Scene_viewer(window,0),
					&eye[0],&eye[1],&eye[2],&lookat[0],&lookat[1],&lookat[2],
					&up[0],&up[1],&up[2]);
			}
			else
			{
				eye[0]=eye[1]=eye[2]=0.0;
				lookat[0]=lookat[1]=lookat[2]=0.0;
				up[0]=up[1]=up[2]=0.0;
				return_code=1;
			}
			if (return_code)
			{
				/* initialise defaults */
				coordinate=ACCESS(Cmgui_coordinate)(global_coordinate_ptr);
				position.data[0]=eye[0];
				position.data[1]=eye[1];
				position.data[2]=eye[2];
				(option_table[0]).to_be_modified= &coordinate;
				(option_table[1]).to_be_modified= &position;
				(option_table[2]).to_be_modified= &window;
				(option_table[2]).user_data=command_data->graphics_window_manager;
				(option_table[3]).to_be_modified= &position;
				return_code=process_multiple_options(state,option_table);
				/* no errors, not asking for help */
				if (return_code)
				{
					if (window)
					{
						Scene_viewer_get_lookat_parameters(
							Graphics_window_get_Scene_viewer(window,0),
							&eye[0],&eye[1],&eye[2],&lookat[0],&lookat[1],&lookat[2],
							&up[0],&up[1],&up[2]);
						/* save the current view direction to give to the up-vector in case
							 the new view direction is colinear with the up-vector */
						oldview[0]=lookat[0]-eye[0];
						oldview[1]=lookat[1]-eye[1];
						oldview[2]=lookat[2]-eye[2];
						/* we have a rc position, but relative to a coordinate system we
							must convert to global */
						get_global_position(&position,coordinate,&global_position);
						eye[0]=global_position.data[0];
						eye[1]=global_position.data[1];
						eye[2]=global_position.data[2];
						/* check eye and lookat points at different positions */
						if ((eye[0]!=lookat[0])||(eye[1]!=lookat[1])||(eye[2]!=lookat[2]))
						{
							/* get unit vector in view direction */
							view[0]=lookat[0]-eye[0];
							view[1]=lookat[1]-eye[1];
							view[2]=lookat[2]-eye[2];
							normalize3(view);
							normalize3(up);
							/* is view direction colinear with up-vector? */
							if (fabs(dot_product3(view,up))>0.99999)
							{
								/* set the up vector to the old view direction (normalized) */
								up[0]=oldview[0];
								up[1]=oldview[1];
								up[2]=oldview[2];
								normalize3(up);
								display_message(WARNING_MESSAGE,
									"gfx set view_point.  View direction colinear with up_vector."
									" Changing up_vector to (%g,%g,%g)",up[0],up[1],up[2]);
							}
							/* ensure up vector is normal to view direction */
							if (fabs(dot_product3(view,up))>0.00001)
							{
								cross_product3(up,view,norm);
								cross_product3(view,norm,up);
								normalize3(up);
								display_message(WARNING_MESSAGE,
									"gfx set view_point.  View direction not normal to up_vector."
									" Changing up_vector to (%g,%g,%g)",up[0],up[1],up[2]);
							}
							if (return_code=Scene_viewer_set_lookat_parameters(
								Graphics_window_get_Scene_viewer(window,0),eye[0],eye[1],eye[2],
								lookat[0],lookat[1],lookat[2],up[0],up[1],up[2]))
								/*???DB.  Should not be changing directly because managed ? */
							{
								return_code=Graphics_window_update_now(window);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx set view_point.  View_point same as interest_point. "
								"Set the interest_point first");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Must have a graphics window");
						return_code=0;
					}
				} /* parse error, help */
				DEACCESS(Cmgui_coordinate)(&coordinate);
			}
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_view_point.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_view_point.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_view_point */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_visibility(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Toggles the visibility of graphics objects on scenes from the command line.
==============================================================================*/
{
	char *name,off_flag,on_flag;
	enum GT_visibility_type current_visibility,new_visibility;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	struct Scene_object *scene_object;
	static struct Modifier_entry
		off_on_option_table[]=
		{
			{"off",NULL,NULL,set_char_flag},
			{"on",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{"name",NULL,(void *)1,set_name},
			{NULL,NULL,NULL,NULL},
			{"scene",NULL,NULL,set_Scene},
			{NULL,NULL,NULL,set_name}
		};

	ENTER(gfx_set_visibility);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			name=(char *)NULL;
			scene=ACCESS(Scene)(command_data->default_scene);
			off_flag=0;
			on_flag=0;
			(option_table[0]).to_be_modified = &name;
			(off_on_option_table[0]).to_be_modified= &off_flag;
			(off_on_option_table[1]).to_be_modified= &on_flag;
			(option_table[1]).user_data= &off_on_option_table;
			(option_table[2]).to_be_modified= &scene;
			(option_table[2]).user_data=command_data->scene_manager;
			(option_table[3]).to_be_modified= &name;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (off_flag&&on_flag)
				{
					display_message(ERROR_MESSAGE,"Only one of off/on");
					return_code=0;
				}
				if (return_code)
				{
					if (name)
					{
						if (scene_object=Scene_get_scene_object_by_name(scene,name))
						{
							current_visibility=Scene_object_get_visibility(scene_object);
							if (on_flag)
							{
								new_visibility=g_VISIBLE;
							}
							else
							{
								if (off_flag)
								{
									new_visibility=g_INVISIBLE;
								}
								else
								{
									if (g_VISIBLE == current_visibility)
									{
										new_visibility=g_INVISIBLE;
									}
									else
									{
										new_visibility=g_VISIBLE;
									}
								}
							}
							if (new_visibility!=current_visibility)
							{
								Scene_object_set_visibility(scene_object,new_visibility);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"No graphics object named '%s' in scene",name);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Missing graphics object name");
						return_code=0;
					}
				}
			} /* parse error,help */
			DEACCESS(Scene)(&scene);
			if (name)
			{
				DEALLOCATE(name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_visibility.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_set_visibility.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_visibility */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int execute_command_gfx_set(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX SET command.
==============================================================================*/
{
	int i,return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"application_parameters",NULL,NULL,set_application_parameters},
		{"axis_length",NULL,NULL,gfx_set_axis_length},
		{"axis_material",NULL,NULL,gfx_set_axis_material},
		{"axis_origin",NULL,NULL,gfx_set_axis_origin},
		{"background",NULL,NULL,gfx_set_background},
		{"far_clipping_plane",NULL,NULL,gfx_set_far_clipping_plane},
		{"interest_point",NULL,NULL,gfx_set_interest_point},
		{"line_width",NULL,NULL,set_float_positive},
		{"near_clipping_plane",NULL,NULL,gfx_set_near_clipping_plane},
		{"node_value",NULL,NULL,gfx_set_FE_nodal_value},
		{"order",NULL,NULL,gfx_set_scene_order},
		{"point_size",NULL,NULL,set_float_positive},
		{"resolution",NULL,NULL,set_graphics_window_resolution},
		{"slider",NULL,NULL,set_node_group_slider_value},
		{"transformation",NULL,NULL,gfx_set_transformation},
		{"time",NULL,NULL,gfx_set_time},
		{"up_vector",NULL,NULL,gfx_set_up_vector},
		{"update_callback",NULL,NULL,gfx_set_update_callback},
		{"video",NULL,NULL,gfx_set_video},
		{"view_angle",NULL,NULL,gfx_set_view_angle},
		{"view_point",NULL,NULL,gfx_set_view_point},
		{"visibility",NULL,NULL,gfx_set_visibility},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_command_gfx_set);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				i=0;
				/* application_parameters */
				i++;
				/* axis_length */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* axis_material */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* axis_origin */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* background */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* far_clipping_plane */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* interest_point */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* line_width */
				(option_table[i]).to_be_modified= &global_line_width;
				i++;
				/* near_clipping_plane */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* node_value */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* order */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* point_size */
				(option_table[i]).to_be_modified= &global_point_size;
				i++;
				/* resolution */
				(option_table[i]).user_data=command_data->graphics_window_manager;
				i++;
				/* slider */
				(option_table[i]).user_data=command_data->node_group_slider_dialog;
				i++;
				/* transformation */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* time */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* up_vector */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* update_callback */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* video */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* view_angle */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* view_point */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* visibility */
				(option_table[i]).user_data=command_data_void;
				i++;
				return_code=process_option(state,option_table);
			}
			else
			{
				set_command_prompt("gfx set",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_set.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_set.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_set */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_smooth(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX SMOOTH command.
==============================================================================*/
{
	int i,j,number_of_components,return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"egroup",NULL,NULL,set_FE_element_group},
		{"field",NULL,NULL,set_FE_field},
		{"smoothing",NULL,NULL,set_float_positive},
		{NULL,NULL,NULL,NULL}
	};
	struct FE_field *field;
	struct GROUP(FE_element) *element_group;
	struct LIST(FE_node) **node_lists;
	struct Smooth_field_over_element_data smooth_field_over_element_data;
	struct Smooth_field_over_node_data smooth_field_over_node_data;

	ENTER(execute_command_gfx_smooth);
	USE_PARAMETER(dummy_to_be_modified);
/*???debug */
printf("enter execute_command_gfx_smooth\n");
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			display_message(WARNING_MESSAGE,"gfx smooth  is temporary/not general");
			element_group=(struct GROUP(FE_element) *)NULL;
			smooth_field_over_element_data.field=(struct FE_field *)NULL;
			smooth_field_over_element_data.smoothing=1;
			smooth_field_over_element_data.node_manager=command_data->node_manager;
			smooth_field_over_element_data.element_manager=
				command_data->element_manager;
			(option_table[0]).to_be_modified= &element_group;
			(option_table[0]).user_data=command_data->element_group_manager;
			(option_table[1]).to_be_modified= &(smooth_field_over_element_data.field);
			(option_table[1]).user_data=command_data->fe_field_manager;
			(option_table[2]).to_be_modified=
				&(smooth_field_over_element_data.smoothing);
			return_code=process_multiple_options(state,option_table);
			if (return_code)
			{
				MANAGER_BEGIN_CACHE(FE_node)(command_data->node_manager);
				MANAGER_BEGIN_CACHE(FE_element)(command_data->element_manager);
				smooth_field_over_element_data.node_lists=(struct LIST(FE_node) **)NULL;
				if (element_group)
				{
					return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
						smooth_field_over_element,(void *)&smooth_field_over_element_data,
						element_group);
				}
				else
				{
					return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
						smooth_field_over_element,(void *)&smooth_field_over_element_data,
						command_data->element_manager);
				}
				if (return_code&&(field=smooth_field_over_element_data.field))
				{
					node_lists=smooth_field_over_element_data.node_lists;
					smooth_field_over_node_data.node_manager=command_data->node_manager;
					smooth_field_over_node_data.field_component.field=field;
					smooth_field_over_node_data.number_of_elements=0;
					for (j=0;j<smooth_field_over_element_data.
						maximum_number_of_elements_per_node;j++)
					{
						(smooth_field_over_node_data.number_of_elements)++;
						number_of_components=get_FE_field_number_of_components(field);
						for (i=0;i<number_of_components;i++)
						{
							smooth_field_over_node_data.field_component.number=i;
							FOR_EACH_OBJECT_IN_LIST(FE_node)(smooth_field_over_node,
								(void *)&smooth_field_over_node_data,*node_lists);
							node_lists++;
						}
					}
				}
				if (node_lists=smooth_field_over_element_data.node_lists)
				{
					number_of_components=get_FE_field_number_of_components(
						smooth_field_over_element_data.field);
					for (i=(number_of_components)*(smooth_field_over_element_data.
						maximum_number_of_elements_per_node);i>0;i--)
					{
						DESTROY_LIST(FE_node)(node_lists);
						node_lists++;
					}
					DEALLOCATE(smooth_field_over_element_data.node_lists);
				}
				MANAGER_END_CACHE(FE_element)(command_data->element_manager);
				MANAGER_END_CACHE(FE_node)(command_data->node_manager);
			}
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
			if (smooth_field_over_element_data.field)
			{
				DEACCESS(FE_field)(&smooth_field_over_element_data.field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_smooth.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_smooth.  Missing state");
		return_code=0;
	}
/*???debug */
printf("leave execute_command_gfx_smooth\n");
	LEAVE;

	return (return_code);
} /* execute_command_gfx_smooth */
#endif /* !defined (WINDOWS_DEV_FLAG) */

int gfx_timekeeper(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
==============================================================================*/
{
	char every, loop, maximum_flag, minimum_flag, once, play, set_time_flag,
		skip, speed_flag, stop, swing;
	double maximum, minimum, set_time, speed;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"every_frame",NULL,NULL,set_char_flag},
		{"loop",NULL,NULL,set_char_flag},
		{"maximum",NULL,NULL,set_double_and_char_flag},
		{"minimum",NULL,NULL,set_double_and_char_flag},
		{"once",NULL,NULL,set_char_flag},
		{"play",NULL,NULL,set_char_flag},
		{"set_time",NULL,NULL,set_double_and_char_flag},
		{"skip_frames",NULL,NULL,set_char_flag},
		{"speed",NULL,NULL,set_double_and_char_flag},
		{"stop",NULL,NULL,set_char_flag},
		{"swing",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Time_keeper *time_keeper;

	ENTER(gfx_timekeeper);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data = (struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
				{
					if (!strcmp(state->current_token, "default"))
					{
						/* Continue */
						return_code = shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Only a default timekeeper at the moment");
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
						"\n      TIMEKEEPER_NAME");
					/* By not shifting the parse state the rest of the help should come out */
					return_code = 1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_timekeeper.  Missing timekeeper name");
				return_code=0;
			}
			if (return_code)
			{
				/* initialise defaults */
				if (time_keeper = command_data->default_time_keeper)
				{
					maximum = Time_keeper_get_maximum(time_keeper);
					minimum = Time_keeper_get_minimum(time_keeper);
					set_time = Time_keeper_get_time(time_keeper);
					speed = Time_keeper_get_speed(time_keeper);
				}
				else
				{
					maximum = 0.0;
					minimum = 0.0;
					set_time = 0.0;
					speed = 30;
				}
				every = 0;
				loop = 0;
				maximum_flag = 0;
				minimum_flag = 0;
				once = 0;
				play = 0;
				set_time_flag = 0;
				skip = 0;
				speed_flag = 0;
				stop = 0;
				swing = 0;

				(option_table[0]).to_be_modified = &every;
				(option_table[1]).to_be_modified = &loop;
				(option_table[2]).to_be_modified = &maximum;
				(option_table[2]).user_data = &maximum_flag;
				(option_table[3]).to_be_modified = &minimum;				
				(option_table[3]).user_data = &minimum_flag;
				(option_table[4]).to_be_modified = &once;
				(option_table[5]).to_be_modified = &play;
				(option_table[6]).to_be_modified = &set_time;
				(option_table[6]).user_data = &set_time_flag;
				(option_table[7]).to_be_modified = &skip;
				(option_table[8]).to_be_modified = &speed;
				(option_table[8]).user_data = &speed_flag;
				(option_table[9]).to_be_modified = &stop;
				(option_table[10]).to_be_modified = &swing;
				return_code=process_multiple_options(state,option_table);

				if(return_code)
				{
					if((loop + once + swing) > 1)
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Specify only one of loop, swing or once");
						return_code = 0;
					}
					if(every && skip)
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Specify only one of every_frame or skip_frames");
						return_code = 0;
					}
					if(play && stop)
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Specify only one of play or stop");
						return_code = 0;
					}
				}
				if (return_code)
				{
					if ( time_keeper )
					{
						if ( set_time_flag )
						{
							Time_keeper_request_new_time(time_keeper, set_time);
						}
						if ( speed_flag )
						{
							Time_keeper_set_speed(time_keeper, speed);
						}
						if ( maximum_flag )
						{
							Time_keeper_set_maximum(time_keeper, maximum);
						}
						if ( minimum_flag )
						{
							Time_keeper_set_minimum(time_keeper, minimum);
						}
						if ( loop )
						{
							Time_keeper_set_play_loop(time_keeper);
						}
						if ( swing )
						{
							Time_keeper_set_play_swing(time_keeper);
						}
						if ( once )
						{
							Time_keeper_set_play_once(time_keeper);
						}
						if ( every )
						{
							Time_keeper_set_play_every_frame(time_keeper);
						}
						if ( skip )
						{
							Time_keeper_set_play_skip_frames(time_keeper);
						}
						if ( play )
						{
							Time_keeper_play(time_keeper, TIME_KEEPER_PLAY_FORWARD);
						}
						if ( stop )
						{
							Time_keeper_stop(time_keeper);
						}
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_timekeeper.  Missing command data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_timekeeper.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* gfx_timekeeper */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_transform(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Executes a GFX TRANSFORM command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"node",NULL,NULL,gfx_transform_node},
			/*???DB.  Change gfx_transform_node because not in this module ? */
		{NULL,NULL,NULL,NULL}
	};
	struct Node_transform_data node_transform_data;

	ENTER(execute_command_gfx_transform);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			node_transform_data.fe_field_manager=command_data->fe_field_manager;
			node_transform_data.node_manager=command_data->node_manager;
			(option_table[0]).user_data= &node_transform_data;
			return_code=process_option(state,option_table);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_transform.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_transform.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_transform */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_update(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 October 1998

DESCRIPTION :
Executes a GFX UPDATE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;
	static struct Modifier_entry option_table[]=
	{
		{"window",NULL,NULL,set_Graphics_window},/*???DB. "on" ? */
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_command_gfx_update);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			window=(struct Graphics_window *)NULL;
			(option_table[0]).to_be_modified= &window;
			(option_table[0]).user_data=command_data->graphics_window_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (window)
				{
					return_code=Graphics_window_update_now(window);
				}
				else
				{
					return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
						Graphics_window_update_now_iterator,(void *)NULL,
						command_data->graphics_window_manager);
				}
			} /* parse error,help */
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_update.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_update.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_update */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_warp_node(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Executes a GFX WARP NODE command. This warps the coordinate nodal values (not
the derivatives).
???DB.  Don't pass all of command data ?
==============================================================================*/
{
	double ximax[3];
	int return_code,xi_order;
	static struct Modifier_entry option_table[]=
	{
		{"coordinate_field",NULL,NULL,set_FE_field},
		{"extent",NULL,NULL,set_Element_discretization},
		{"from",NULL,NULL,set_FE_node_group},
		{"seed_element",NULL,NULL,set_FE_element_dimension_3},
		{"to_coordinate_field",NULL,NULL,set_FE_field},
		{"values",NULL,NULL,set_Warp_values},
		{"warp_field",NULL,NULL,set_FE_field},
		{"xi_order",NULL,NULL,set_int_positive},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Element_discretization extent;
	struct FE_element *seed_element;
	struct FE_field *coordinate_field,*to_coordinate_field,*warp_field;
	struct GROUP(FE_node) *node_group;
	struct Warp_values warp_values;

	ENTER(gfx_warp_node);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			coordinate_field=(struct FE_field *)NULL;
			to_coordinate_field=(struct FE_field *)NULL;
			warp_field=(struct FE_field *)NULL;
			seed_element=(struct FE_element *)NULL;
			extent.number_in_xi1=1;
			extent.number_in_xi2=1;
			extent.number_in_xi3=1;
			xi_order=123;
			node_group=(struct GROUP(FE_node) *)NULL;
			(option_table[0]).to_be_modified= &coordinate_field;
			(option_table[0]).user_data=command_data->fe_field_manager;
			(option_table[1]).to_be_modified= &extent;
			(option_table[1]).user_data=command_data->user_interface;
			(option_table[2]).to_be_modified= &node_group;
			(option_table[2]).user_data=command_data->node_group_manager;
			(option_table[3]).to_be_modified= &seed_element;
			(option_table[3]).user_data=command_data->element_manager;
			(option_table[4]).to_be_modified= &to_coordinate_field;
			(option_table[4]).user_data=command_data->fe_field_manager;
			(option_table[5]).to_be_modified= &warp_values;
			(option_table[6]).to_be_modified= &warp_field;
			(option_table[6]).user_data=command_data->fe_field_manager;
			(option_table[7]).to_be_modified= &xi_order;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				ximax[0]=extent.number_in_xi1;
				ximax[1]=extent.number_in_xi2;
				ximax[2]=extent.number_in_xi3;
				return_code=warp_FE_node_group_with_FE_element(node_group,
					command_data->node_manager,coordinate_field,to_coordinate_field,
					seed_element,warp_field,ximax,warp_values.value,xi_order);
			}
			if (node_group)
			{
				DEACCESS(GROUP(FE_node))(&node_group);
			}
			if (seed_element)
			{
				DEACCESS(FE_element)(&seed_element);
			}
			if (coordinate_field)
			{
				DEACCESS(FE_field)(&coordinate_field);
			}
			if (to_coordinate_field)
			{
				DEACCESS(FE_field)(&to_coordinate_field);
			}
			if (warp_field)
			{
				DEACCESS(FE_field)(&warp_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_warp_node.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_warp_node.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_warp_node */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_warp_voltex(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Executes a GFX WARP VOLTEX command. This warps the vertex and normals of a
gtvoltex according to calculated
???DB.  Don't pass all of command data ?
==============================================================================*/
{
	char *graphics_object_name1,*graphics_object_name2;
	double ximax[3];
	gtObject *graphics_object1,*graphics_object2;
	int itime1,itime2,return_code,xi_order;
	static char default_name[]="volume";
	static struct Modifier_entry option_table[]=
	{
		{"extent",NULL,NULL,set_Element_discretization},
		{"field",NULL,NULL,set_FE_field},
		{"from",NULL,(void *)1,set_name},
		{"seed_element",NULL,NULL,set_FE_element_dimension_3},
		{"to",NULL,(void *)1,set_name},
		{"values",NULL,NULL,set_Warp_values},
		{"xi_order",NULL,NULL,set_int_positive},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Element_discretization extent;
	struct FE_element *seed_element;
	struct FE_field *warp_field;
	struct Warp_values warp_values;

	ENTER(gfx_warp_voltex);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			if (ALLOCATE(graphics_object_name1,char,strlen(default_name)+1))
			{
				strcpy(graphics_object_name1,default_name);
			}
			else
			{
				graphics_object_name1=(char *)NULL;
			}
			if (ALLOCATE(graphics_object_name2,char,strlen(default_name)+1))
			{
				strcpy(graphics_object_name2,default_name);
			}
			else
			{
				graphics_object_name2=(char *)NULL;
			}
			warp_field=(struct FE_field *)NULL;
			seed_element=(struct FE_element *)NULL;
			extent.number_in_xi1=1;
			extent.number_in_xi2=1;
			extent.number_in_xi3=1;
			xi_order=123;
			(option_table[0]).to_be_modified= &extent;
			(option_table[0]).user_data=command_data->user_interface;
			(option_table[1]).to_be_modified= &warp_field;
			(option_table[1]).user_data=command_data->fe_field_manager;
			(option_table[2]).to_be_modified= &graphics_object_name1;
			(option_table[3]).to_be_modified= &seed_element;
			(option_table[3]).user_data=command_data->element_manager;
			(option_table[4]).to_be_modified= &graphics_object_name2;
			(option_table[5]).to_be_modified= &warp_values;
			(option_table[6]).to_be_modified= &xi_order;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if ((graphics_object1=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name1,command_data->graphics_object_list))&&
					(graphics_object2=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name2,command_data->graphics_object_list)))
				{
					if ((g_VOLTEX==graphics_object1->object_type)&&
						(g_VOLTEX==graphics_object1->object_type))
					{
						if (seed_element&&(seed_element->shape)&&
							(3==seed_element->shape->dimension))
						{
							ximax[0]=extent.number_in_xi1;
							ximax[1]=extent.number_in_xi2;
							ximax[2]=extent.number_in_xi3;
							itime1=graphics_object1->number_of_times;
							itime2=graphics_object2->number_of_times;
							return_code=warp_GT_voltex_with_FE_element(
								(graphics_object1->gu.gt_voltex)[itime1-1],
								(graphics_object2->gu.gt_voltex)[itime2-1],seed_element,
								warp_field,ximax,warp_values.value,xi_order);
#if defined (DEBUG)
							/*???debug */
							printf("Warp called: volume1 = %s, volume2 = %s,  element = %d, coordinates = %s, extent = %d %d %d, values = %f %f,  %f %f,  %f %f xi_order = %d\n",
								graphics_object1->name,graphics_object2->name,
								(seed_element->cmiss).element_number,warp_field->name,
								extent.number_in_xi1,extent.number_in_xi2,extent.number_in_xi3,
								(warp_values.value)[0],(warp_values.value)[1],
								(warp_values.value)[2],(warp_values.value)[3],
								(warp_values.value)[4],(warp_values.value)[5],xi_order);
#endif /* defined (DEBUG) */
						}
						else
						{
							display_message(ERROR_MESSAGE,"Missing or non 3-D seed element");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Specified graphics object(s) not of voltex type");
						return_code=0;
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"Volumes don't exist");
					return_code=0;
				}
			}
			DEALLOCATE(graphics_object_name1);
			DEALLOCATE(graphics_object_name2);
			if (seed_element)
			{
				DEACCESS(FE_element)(&seed_element);
			}
			if (warp_field)
			{
				DEACCESS(FE_field)(&warp_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_warp_voltex.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_warp_voltex.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_warp_voltex */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_warp(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 March 1998

DESCRIPTION :
Executes a GFX command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"node",NULL,NULL,gfx_warp_node},
		{"voltex",NULL,NULL,gfx_warp_voltex},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_gfx_warp);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				(option_table[0]).user_data=command_data_void;
				(option_table[1]).user_data=command_data_void;
				return_code=process_option(state,option_table);
			}
			else
			{
				set_command_prompt("gfx warp",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_warp.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_write_Control_curve(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Writes an individual curve or all curves to filename(s) stemming from the name
of the curve, eg. "name" -> name.curve.com name.curve.exnode name.curve.exelem
==============================================================================*/
{
	char write_all_curves_flag;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Modifier_entry option_table[]=
	{
		{"all",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,set_Control_curve}
	};
	struct Control_curve *curve;

	ENTER(gfx_write_Control_curve);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		return_code=1;
		write_all_curves_flag=0;
		curve=(struct Control_curve *)NULL;
		(option_table[0]).to_be_modified= &write_all_curves_flag;
		(option_table[1]).to_be_modified= &curve;
		(option_table[1]).user_data=command_data->control_curve_manager;
		if (return_code=process_multiple_options(state,option_table))
		{
			if (write_all_curves_flag&&!curve)
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Control_curve)(
					write_Control_curve,(void *)NULL,
					command_data->control_curve_manager);
			}
			else if (curve&&!write_all_curves_flag)
			{
				return_code=write_Control_curve(curve,(void *)NULL);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_write_Control_curve.  Specify either a curve name or 'all'");
				return_code=0;
			}
		}
		if (curve)
		{
			DEACCESS(Control_curve)(&curve);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_write_Control_curve.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_Control_curve */

static int gfx_write_data(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
If a data file is not specified a file selection box is presented to the user,
otherwise the data file is written.
Can now specify individual node groups to write with the <group> option.
???DB.  Almost identical to gfx_write_nodes.  Could set up struct
	Write_nodes_data to combine, but will probably be adding ipdata format
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct FE_field *field;
	struct Fwrite_all_FE_node_groups_data fwrite_all_FE_node_groups_data;
	struct Fwrite_FE_node_group_data fwrite_FE_node_group_data;
	struct GROUP(FE_node) *node_group;
	static struct Modifier_entry option_table[]=
	{
		{"field",NULL,NULL,set_FE_field},
		{"group",NULL,NULL,set_FE_node_group},
		{NULL,NULL,NULL,set_name}
	};

	ENTER(gfx_write_data);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			return_code=1;
			node_group=(struct GROUP(FE_node) *)NULL;
			field=(struct FE_field *)NULL;
			file_name=(char *)NULL;
			if (state->current_token)
			{
				(option_table[0]).to_be_modified= &field;
				(option_table[0]).user_data=command_data->fe_field_manager;
				(option_table[1]).to_be_modified= &node_group;
				(option_table[1]).user_data=command_data->data_group_manager;
				(option_table[2]).to_be_modified= &file_name;
				return_code=process_multiple_options(state,option_table);
			}
			if (return_code)
			{
				if (!file_name)
				{
					if (!(file_name = confirmation_get_write_filename(".exdata",
						command_data->user_interface)))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					/* open the file */
					if (return_code=check_suffix(&file_name,".exdata"))
					{
						if (node_group)
						{
							fwrite_FE_node_group_data.node_group=node_group;
							fwrite_FE_node_group_data.field=field;
							return_code=file_write_FE_node_group(file_name,
								(void *)&fwrite_FE_node_group_data);
						}
						else
						{
							fwrite_all_FE_node_groups_data.node_group_manager=
								command_data->data_group_manager;
							fwrite_all_FE_node_groups_data.field=field;
							return_code=file_write_all_FE_node_groups(file_name,
								(void *)&fwrite_all_FE_node_groups_data);
						}
					}
				}
			}
			if (field)
			{
				DEACCESS(FE_field)(&field);
			}
			if (node_group)
			{
				DEACCESS(GROUP(FE_node))(&node_group);
			}
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_write_data.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_write_data.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_data */

static int gfx_write_elements(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 30 March 1999

DESCRIPTION :
If an element file is not specified a file selection box is presented to the
user, otherwise the element file is written.
Can also write individual element groups with the <group> option.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct FE_field *field;
	struct Fwrite_all_FE_element_groups_data fwrite_all_FE_element_groups_data;
	struct Fwrite_FE_element_group_data fwrite_FE_element_group_data;
	struct GROUP(FE_element) *element_group;
	static struct Modifier_entry option_table[]=
	{
		{"field",NULL,NULL,set_FE_field},
		{"group",NULL,NULL,set_FE_element_group},
		{NULL,NULL,NULL,set_name}
	};

	ENTER(gfx_write_elements);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			return_code=1;
			element_group=(struct GROUP(FE_element) *)NULL;
			field=(struct FE_field *)NULL;
			file_name=(char *)NULL;
			if (state->current_token)
			{
				(option_table[0]).to_be_modified= &field;
				(option_table[0]).user_data=command_data->fe_field_manager;
				(option_table[1]).to_be_modified= &element_group;
				(option_table[1]).user_data=command_data->element_group_manager;
				(option_table[2]).to_be_modified= &file_name;
				return_code=process_multiple_options(state,option_table);
			}
			if (return_code)
			{
				if (!file_name)
				{
					if (!(file_name = confirmation_get_write_filename(".exelem",
						command_data->user_interface)))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					/* open the file */
					if (return_code=check_suffix(&file_name,".exelem"))
					{
						if (element_group)
						{
							fwrite_FE_element_group_data.element_group=element_group;
							fwrite_FE_element_group_data.field=field;
							return_code=file_write_FE_element_group(file_name,
								(void *)&fwrite_FE_element_group_data);
						}
						else
						{
							fwrite_all_FE_element_groups_data.element_group_manager=
								command_data->element_group_manager;
							fwrite_all_FE_element_groups_data.field=field;
							return_code=file_write_all_FE_element_groups(file_name,
								(void *)&fwrite_all_FE_element_groups_data);
						}
					}
				}
			}
			if (field)
			{
				DEACCESS(FE_field)(&field);
			}
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_write_elements.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_write_elements.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_elements */

static int gfx_write_nodes(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
If a nodes file is not specified a file selection box is presented to the user,
otherwise the nodes file is written.
Can now specify individual node groups to write with the <group> option.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct FE_field *field;
	struct Fwrite_all_FE_node_groups_data fwrite_all_FE_node_groups_data;
	struct Fwrite_FE_node_group_data fwrite_FE_node_group_data;
	struct GROUP(FE_node) *node_group;
	static struct Modifier_entry option_table[]=
	{
		{"field",NULL,NULL,set_FE_field},
		{"group",NULL,NULL,set_FE_node_group},
		{NULL,NULL,NULL,set_name}
	};

	ENTER(gfx_write_nodes);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			return_code=1;
			node_group=(struct GROUP(FE_node) *)NULL;
			field=(struct FE_field *)NULL;
			file_name=(char *)NULL;
			if (state->current_token)
			{
				(option_table[0]).to_be_modified= &field;
				(option_table[0]).user_data=command_data->fe_field_manager;
				(option_table[1]).to_be_modified= &node_group;
				(option_table[1]).user_data=command_data->node_group_manager;
				(option_table[2]).to_be_modified= &file_name;
				return_code=process_multiple_options(state,option_table);
			}
			if (return_code)
			{
				if (!file_name)
				{
					if (!(file_name = confirmation_get_write_filename(".exnode",
						command_data->user_interface)))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					/* open the file */
					if (return_code=check_suffix(&file_name,".exnode"))
					{
						if (node_group)
						{
							fwrite_FE_node_group_data.node_group=node_group;
							fwrite_FE_node_group_data.field=field;
							return_code=file_write_FE_node_group(file_name,
								(void *)&fwrite_FE_node_group_data);
						}
						else
						{
							fwrite_all_FE_node_groups_data.node_group_manager=
								command_data->node_group_manager;
							fwrite_all_FE_node_groups_data.field=field;
							return_code=file_write_all_FE_node_groups(file_name,
								(void *)&fwrite_all_FE_node_groups_data);
						}
					}
				}
			}
			if (field)
			{
				DEACCESS(FE_field)(&field);
			}
			if (node_group)
			{
				DEACCESS(GROUP(FE_node))(&node_group);
			}
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_write_nodes.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_write_nodes.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_nodes */

static int gfx_write_element_layout(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Executes a GFX WRITE_ELEMENT_LAYOUT command.
==============================================================================*/
{
	double ximax[3];
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"extent",NULL,NULL,set_Element_discretization},
		{"seed_element",NULL,NULL,set_FE_element_dimension_3},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Element_discretization extent;
	struct FE_element *seed_element;

	ENTER(gfx_write_element_layout);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			seed_element=(struct FE_element *)NULL;
			extent.number_in_xi1=1;
			extent.number_in_xi2=1;
			extent.number_in_xi3=1;
			(option_table[0]).to_be_modified= &extent;
			(option_table[0]).user_data=(void *)(command_data->user_interface);
			(option_table[1]).to_be_modified= &seed_element;
			(option_table[1]).user_data=command_data->element_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				ximax[0]=extent.number_in_xi1;
				ximax[1]=extent.number_in_xi2;
				ximax[2]=extent.number_in_xi3;
				return_code=write_FE_element_layout(ximax, seed_element);
			}
			if (seed_element)
			{
				DEACCESS(FE_element)(&seed_element);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_write_FE_element_layout.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_write_FE_element_layout.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_write_element_layout */

static int gfx_write_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 May 1999

DESCRIPTION :
Executes a GFX WRITE TEXTURE command.
==============================================================================*/
{
	auto struct Modifier_entry
		landscape_portrait_option_table[]=
		{
			{"landscape",NULL,NULL,set_char_flag},
			{"portrait",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{"file",NULL,(void *)1,set_name},
			{NULL,NULL,NULL,NULL},
			{NULL,NULL,NULL,NULL},
			{NULL,NULL,NULL,NULL}
		},
		postscript_rgb_tiff_option_table[]=
		{
			{"postscript",NULL,NULL,set_char_flag},
			{"rgb",NULL,NULL,set_char_flag},
			{"tiff",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL}
		};
	char *current_token,*file_name,landscape_flag,portrait_flag,postscript_flag,
		rgb_flag,tiff_flag;
	enum Image_file_format file_format;
	enum Image_orientation orientation;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Texture *texture;

	ENTER(gfx_write_texture);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			texture = (struct Texture *)NULL;
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (command_data->texture_manager)
					{
						if (texture=FIND_BY_IDENTIFIER_IN_MANAGER(Texture,name)
							(current_token,command_data->texture_manager))
						{
							return_code=shift_Parse_state(state,1);
						}
						else
						{
							display_message(ERROR_MESSAGE,"gfx_write_texture.  Unknown texture : %s",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_write_Texture.  Missing texture manager");
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
						" TEXTURE_NAME");
					return_code = 1;
					/* By not shifting the parse state the rest of the help should come out */
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_Texture.  Missing texture name");
				return_code=0;
			}

			if (return_code)
			{
				/* initialize defaults */
				file_name=(char *)NULL;
				landscape_flag=0;
				portrait_flag=0;
				/* default is postscript (see later) */
				postscript_flag=0;
				rgb_flag=0;
				tiff_flag=0;

				(option_table[0]).to_be_modified= &file_name;
				(landscape_portrait_option_table[0]).to_be_modified= &landscape_flag;
				(landscape_portrait_option_table[1]).to_be_modified= &portrait_flag;
				(option_table[1]).user_data=landscape_portrait_option_table;
				(postscript_rgb_tiff_option_table[0]).to_be_modified= &postscript_flag;
				(postscript_rgb_tiff_option_table[1]).to_be_modified= &rgb_flag;
				(postscript_rgb_tiff_option_table[2]).to_be_modified= &tiff_flag;
				(option_table[2]).user_data=postscript_rgb_tiff_option_table;
				return_code=process_multiple_options(state,option_table);
				/* no errors, not asking for help */
				if (return_code)
				{
					if (postscript_flag + rgb_flag + tiff_flag > 1)
					{
						display_message(ERROR_MESSAGE,"gfx_write_texture.  Specify only one of postscript, rgb or tiff");
						return_code = 0;
					}
					if (landscape_flag && portrait_flag)
					{
						display_message(ERROR_MESSAGE,"gfx_write_texture.  Specify only one of landscape or portrait");
						return_code = 0;
					}
					if (postscript_flag + rgb_flag + tiff_flag == 0)
					{
						display_message(ERROR_MESSAGE,"gfx_write_texture.  Must specify one of postscript, rgb or tiff");
						return_code = 0;
					}				
					if(!texture)
					{
						display_message(ERROR_MESSAGE,"gfx_write_texture.  Specify texture to write");
						return_code=0;
					}
				}
				if (return_code)
				{
					if (rgb_flag)
					{
						file_format = RGB_FILE_FORMAT;
					}
					else if (tiff_flag)
					{
						file_format = TIFF_FILE_FORMAT;
					}
					else if (postscript_flag)
					{
						file_format = POSTSCRIPT_FILE_FORMAT;
					}
					if (landscape_flag)
					{
						orientation = LANDSCAPE_ORIENTATION;
					}
					else
					{
						/* Default */
						orientation = PORTRAIT_ORIENTATION;
					}
					if (!file_name)
					{
						switch (file_format)
						{
							case RGB_FILE_FORMAT:
							{
								if (!(file_name = confirmation_get_write_filename(".rgb",
									command_data->user_interface)))
								{
									return_code = 0;
								}
							} break;
							case TIFF_FILE_FORMAT:
							{
								if (!(file_name = confirmation_get_write_filename(".tiff",
									command_data->user_interface)))
								{
									return_code = 0;
								}					
							} break;
							case POSTSCRIPT_FILE_FORMAT:
							{
								if (!(file_name = confirmation_get_write_filename(".ps",
									command_data->user_interface)))
								{
									return_code = 0;
								}					
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,"gfx_write_texture.  Error setting file extension");
								return_code = 0;							
							} break;
						}
					}
				}
				if (return_code)
				{
					Texture_write_to_file(texture, file_name, file_format, orientation);
				} /* parse error, help */
				if (file_name)
				{
					DEALLOCATE(file_name);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_write_texture.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_write_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_texture */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_write(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Executes a GFX WRITE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"curve",NULL,NULL,gfx_write_Control_curve},
		{"data",NULL,NULL,gfx_write_data},
		{"element_layout",NULL,NULL,gfx_write_element_layout},
		{"elements",NULL,NULL,gfx_write_elements},
		{"nodes",NULL,NULL,gfx_write_nodes},
		{"texture",NULL,NULL,gfx_write_texture},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_command_gfx_write);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			(option_table[0]).user_data=command_data_void;
			(option_table[1]).user_data=command_data_void;
			(option_table[2]).user_data=command_data_void;
			(option_table[3]).user_data=command_data_void;
			(option_table[4]).user_data=command_data_void;
			(option_table[5]).user_data=command_data_void;
			return_code=process_option(state,option_table);
		}
		else
		{
			set_command_prompt("gfx write",command_data->command_window);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_write.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_write */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 12 May 2000

DESCRIPTION :
Executes a GFX command.
==============================================================================*/
{
	int i,return_code;
	static struct Modifier_entry option_table[]=
	{
		{"change_identifier",NULL,NULL,gfx_change_identifier},
		{"create",NULL,NULL,execute_command_gfx_create},
#if !defined (WINDOWS_DEV_FLAG)
		{"data_tool",NULL,NULL,execute_command_gfx_node_tool},
		{"define",NULL,NULL,execute_command_gfx_define},
		{"destroy",NULL,NULL,execute_command_gfx_destroy},
		{"draw",NULL,NULL,execute_command_gfx_draw},
		{"edit",NULL,NULL,execute_command_gfx_edit},
		{"element_creator",NULL,NULL,execute_command_gfx_element_creator},
		{"erase",NULL,NULL,execute_command_gfx_erase},
		{"evaluate",NULL,NULL,gfx_evaluate},
		{"export",NULL,NULL,execute_command_gfx_export},
		{"filter",NULL,NULL,execute_command_gfx_filter},
		{"grab_frame",NULL,NULL,execute_command_gfx_grab_frame},
		{"list",NULL,NULL,execute_command_gfx_list},
		{"modify",NULL,NULL,execute_command_gfx_modify},
		{"movie",NULL,NULL,gfx_movie},
		{"node_tool",NULL,NULL,execute_command_gfx_node_tool},
		{"print",NULL,NULL,execute_command_gfx_print},
		{"project",NULL,NULL,open_projection_window},
#endif /* !defined (WINDOWS_DEV_FLAG) */
		{"read",NULL,NULL,execute_command_gfx_read},
#if !defined (WINDOWS_DEV_FLAG)
		{"select",NULL,NULL,execute_command_gfx_select},
		{"set",NULL,NULL,execute_command_gfx_set},
		{"smooth",NULL,NULL,execute_command_gfx_smooth},
		{"timekeeper",NULL,NULL,gfx_timekeeper},
		{"transform",NULL,NULL,execute_command_gfx_transform},
		{"unselect",NULL,NULL,execute_command_gfx_unselect},
		{"update",NULL,NULL,execute_command_gfx_update},
		{"warp",NULL,NULL,execute_command_gfx_warp},
		{"write",NULL,NULL,execute_command_gfx_write},
#endif /* !defined (WINDOWS_DEV_FLAG) */
		{NULL,NULL,NULL,NULL}
	};
	struct Open_projection_window_data open_projection_window_data;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_gfx);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				i=0;
				/* change_identifier */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* create */
				(option_table[i]).user_data=command_data_void;
				i++;
#if !defined (WINDOWS_DEV_FLAG)
				/* data_tool */
				(option_table[i]).to_be_modified=(void *)1;
				(option_table[i]).user_data=command_data_void;
				i++;
				/* define */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* destroy */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* draw */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* edit */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* element_creator */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* erase */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* evaluate */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* export */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* filter */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* grab_frame */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* list */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* modify */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* movie */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* node_tool */
				(option_table[i]).to_be_modified=(void *)0;
				(option_table[i]).user_data=command_data_void;
				i++;
				/* print */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* project */
				(option_table[i]).to_be_modified= &(command_data->projection_window);
				open_projection_window_data.user_interface=command_data->user_interface;
				open_projection_window_data.fe_field_manager=
					command_data->fe_field_manager;
				open_projection_window_data.element_manager=
					command_data->element_manager;
				open_projection_window_data.element_group_manager=
					command_data->element_group_manager;
				open_projection_window_data.spectrum_manager=
					command_data->spectrum_manager;
				open_projection_window_data.default_spectrum=
					command_data->default_spectrum;
				(option_table[i]).user_data= &open_projection_window_data;
				i++;
#endif /* !defined (WINDOWS_DEV_FLAG) */
				/* read */
				(option_table[i]).user_data=command_data_void;
				i++;
#if !defined (WINDOWS_DEV_FLAG)
				/* select */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* set */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* smooth */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* timekeeper */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* transform */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* unselect */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* update */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* warp */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* write */
				(option_table[i]).user_data=command_data_void;
				i++;
#endif /* !defined (WINDOWS_DEV_FLAG) */
				return_code=process_option(state,option_table);
			}
			else
			{
				set_command_prompt("gfx",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_cm(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 27 May 1997

DESCRIPTION :
Executes a cm (back end) command.
==============================================================================*/
{
	char *current_token,*prompt;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_cm);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
#if defined (LINK_CMISS)
				if (CMISS)
				{
					/* somehow extract the whole command */
					return_code=CMISS_connection_process_command(CMISS,
						state->command_string);
				}
				else
				{
					if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
					{
						display_message(ERROR_MESSAGE,
							"execute_command_cm.	No CMISS connection");
						return_code=0;
					}
					else
					{
						return_code=1;
					}
				}
#else /* defined (LINK_CMISS) */
				display_message(ERROR_MESSAGE,"execute_command_cm.	Define LINK_CMISS");
				return_code=0;
#endif /* defined (LINK_CMISS) */
#if defined (OLD_CODE)
				if (open_socket(command_data->command_window,
					command_data->basis_manager,command_data->node_manager,
					command_data->element_group_manager,command_data->node_group_manager,
					&(command_data->prompt_window),command_data->user_interface))
				{
#if !defined (WINDOWS_DEV_FLAG)
					write_socket(state->command_string,CONN_ID1);
#endif /* !defined (WINDOWS_DEV_FLAG) */
					return_code=1;
				}
				else
				{
					return_code=0;
				}
#endif /* defined (OLD_CODE) */
			}
			else
			{
				if (prompt=(char *)prompt_void)
				{
					set_command_prompt(prompt,command_data->command_window);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_cm.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_cm.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cm */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if defined (OLD_CODE)
#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_connection(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 3 October 1996

DESCRIPTION :
Executes a CONNECTION command.
???RC Obsolete.
==============================================================================*/
{
	char *current_token,*host_name;
	int port_number,return_code;

	ENTER(execute_command_connection);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	/* check argument */
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				set_host(current_token);
				shift_Parse_state(state,1);
				if (current_token=state->current_token)
				{
					set_port(atoi(current_token));
				}
				return_code=1;
			}
			else
			{
				if (get_host(&host_name))
				{
					display_message(INFORMATION_MESSAGE," HOSTNAME[%s]",host_name);
					DEALLOCATE(host_name);
				}
				else
				{
					display_message(INFORMATION_MESSAGE," HOSTNAME[none]");
				}
				if (get_port(&port_number))
				{
					display_message(INFORMATION_MESSAGE," PORT_ADDRESS[%d]",port_number);
				}
				else
				{
					display_message(INFORMATION_MESSAGE," PORT_ADDRESS[none]");
				}
				return_code=1;
			}
		}
		else
		{
			if (get_port(&port_number)&&get_host(&host_name))
			{
				display_message(INFORMATION_MESSAGE,"host name : %s\n",host_name);
				display_message(INFORMATION_MESSAGE,"port address : %d\n",port_number);
				DEALLOCATE(host_name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"Could not get connection information");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_cm.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_connection */
#endif /* !defined (WINDOWS_DEV_FLAG) */
#endif /* defined (OLD_CODE) */

#if !defined (WINDOWS_DEV_FLAG)
#if defined (UNEMAP)
static int execute_command_unemap_open(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Executes a UNEMAP OPEN command.
==============================================================================*/
{
	char *current_token;
	Dimension window_width=0,window_height=0;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct System_window *system;
	struct System_window_data
	{
		Position x;
		Position y;
	} system_window_data;
	static XtResource resources[]=
	{
		{
			XmNx,
			XmCPosition,
			XmRPosition,
			sizeof(Position),
			XtOffsetOf(struct System_window_data,x),
			XmRImmediate,
			(XtPointer) -1
		},
		{
			XmNy,
			XmCPosition,
			XmRPosition,
			sizeof(Position),
			XtOffsetOf(struct System_window_data,y),
			XmRImmediate,
			(XtPointer) -1
		}
	};
	Widget shell;

	ENTER(execute_command_unemap_open);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (!((current_token=state->current_token)&&
				!(strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				if (!(system=command_data->unemap_system_window))
				{
					/* create a shell */
					if (shell=XtVaCreatePopupShell("system_window_shell",
						topLevelShellWidgetClass,
						command_data->user_interface->application_shell,
						XmNallowShellResize,False,NULL))
					{
						if (system=create_System_window(shell,close_emap,
							command_data->default_time_keeper,command_data->user_interface,
							command_data->unemap_package))
						{
							command_data->unemap_system_window=system;
							create_Shell_list_item(&(system->window_shell),
								command_data->user_interface);
							XtAddCallback(system->window_shell,XmNdestroyCallback,
								close_emap,(XtPointer)system);
							/* manage the system window */
							XtManageChild(system->window);
							/* realize the system window shell */
							XtRealizeWidget(system->window_shell);
							/* determine placement */
							XtVaGetValues(system->window_shell,
								XmNwidth,&window_width,
								XmNheight,&window_height,
								NULL);
							/* Do all this to allow backward compatibility but still allow the
								resources to be set */
							system_window_data.x = -1; /* These defaults match with the */
							system_window_data.y = -1; /* default resources above */
							XtVaGetApplicationResources(system->window_shell,
								&system_window_data,resources,XtNumber(resources),NULL);
							if (system_window_data.x == -1)
							{
								system_window_data.x = ((command_data->user_interface->
									screen_width)-window_width)/2;
							}
							if (system_window_data.y == -1)
							{
								system_window_data.y = ((command_data->user_interface->
									screen_height)-window_height)/2;
							}
							XtVaSetValues(system->window_shell,
								XmNx, system_window_data.x,
								XmNy, system_window_data.y,
								XmNmappedWhenManaged, True,
								NULL);
						}
						else
						{
							display_message(ERROR_MESSAGE,
				"execute_command_unemap_open.  Could not create unemap_system_window");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
	"execute_command_unemap_open.  Could not create unemap_system_window shell");
					}
				}
				if (system)
				{
					/* pop up the system window shell */
					XtPopup(system->window_shell,XtGrabNone);
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				/* no help */
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_unemap_open.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_unemap_open.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_unemap_open */
#endif /* defined (UNEMAP) */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_unemap(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 9 December 1996

DESCRIPTION :
Executes a UNEMAP command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
#if defined (UNEMAP)
		{"open",NULL,NULL,execute_command_unemap_open},
#endif /* defined (UNEMAP) */
		{NULL,NULL,NULL,execute_command_cm}
	};

	ENTER(execute_command_unemap);
	/* check argument */
	if (state)
	{
#if defined (UNEMAP)
		(option_table[0]).user_data=command_data_void;
		(option_table[1]).user_data=command_data_void;
		(option_table[1]).to_be_modified=prompt_void;
#else
		(option_table[0]).user_data=command_data_void;
		(option_table[0]).to_be_modified=prompt_void;
#endif /* defined (UNEMAP) */
		return_code=process_option(state,option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_unemap.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_unemap */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
#if defined (CELL)
static int execute_command_cell_open(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a CELL OPEN command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cell_window *cell_window;
	struct Cmiss_command_data *command_data;
	struct Execute_command *execute_command;
	
	ENTER(execute_command_cell_open);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (!((current_token=state->current_token)&&
				!(strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				if (!(cell_window=command_data->cell_window))
				{
					if (execute_command = CREATE(Execute_command)(cmiss_execute_command,
						(void *)(command_data)))
					{
						/* create a shell */
						if (XtVaCreatePopupShell("cell_window_shell",
							topLevelShellWidgetClass,
							command_data->user_interface->application_shell,
							XmNallowShellResize,False,NULL))
						{
							/* add Cell 3D */
							if (cell_window=create_Cell_window(command_data->user_interface,
								(char *)NULL,command_data->control_curve_manager,
								command_data->unemap_package,
								&(command_data->background_colour),command_data->light_manager,
								command_data->default_light,command_data->light_model_manager,
								command_data->default_light_model,command_data->scene_manager,
								command_data->default_scene,command_data->texture_manager,
								command_data->graphics_object_list,
								command_data->graphical_material_manager,
								command_data->glyph_list,command_data->fe_field_manager,
								command_data->element_group_manager,command_data->node_manager,
								command_data->element_manager,command_data->node_group_manager,
								command_data->data_group_manager,
								command_data->default_graphical_material,
								command_data->spectrum_manager,command_data->default_spectrum,
								execute_command))
#if defined (OLD_CODE)
#if defined (CELL_CONTROL_CURVE)
							if (cell_window=create_Cell_window(shell,close_cell,
								command_data->user_interface,
								command_data->control_curve_manager,
								command_data->unemap_package))
#else
							if (cell_window=create_Cell_window(shell,close_cell,
								command_data->user_interface))
#endif /* defined (CELL_CONTROL_CURVE) */
#endif /* defined (OLD_CODE) */
							{
								command_data->cell_window=cell_window;
#if defined (CELL)
								/*create_Shell_list_item(&(cell_window->window_shell),
								  command_data->user_interface);
								  XtAddCallback(cell_window->window_shell,XmNdestroyCallback,
								  close_cell,(XtPointer)cell_window);*/
								/* manage the cell window */
								/*XtManageChild(cell_window->shell);*/
								/* realize the cell window shell */
								/*XtRealizeWidget(cell_window->window_shell);*/
#endif /* defined (CELL) */
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"execute_command_cell_open.  Could not create cell_window");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"execute_command_cell_open.  Could not create cell_window shell");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"execute_command_cell_open.  Could not ALLOCATE Execute_command structure");
					}	
				}
					if (cell_window->shell)
				{
#if defined (CELL)
					/* pop up the CELL window shell */
					XtPopup(cell_window->shell,XtGrabNone);
#endif /* defined (CELL) */
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				/* no help */
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_cell_open.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_cell_open.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cell_open */
#endif /* !defined (WINDOWS_DEV_FLAG) */
#endif /* defined (CELL) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_cell(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 30 September 1998

DESCRIPTION :
Executes a CELL command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
#if defined (CELL)
		{"open",NULL,NULL,execute_command_cell_open},
#endif /* defined (CELL) */
		{NULL,NULL,NULL,execute_command_cm}
	};

	ENTER(execute_command_cell);
	/* check argument */
	if (state)
	{
#if defined (CELL)
		(option_table[0]).user_data=command_data_void;
		(option_table[1]).user_data=command_data_void;
		(option_table[1]).to_be_modified=prompt_void;
#else
		(option_table[0]).user_data=command_data_void;
		(option_table[0]).to_be_modified=prompt_void;
#endif /* defined (CELL) */
		return_code=process_option(state,option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_cell.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cell */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_assign(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 

DESCRIPTION :
Executes an ASSIGN command.
First implementation of a small subset of the assign command which exists 
in the "new interpreter".
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_assign);
	USE_PARAMETER(prompt_void);
	if (state)
	{
		if (state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
			{
				if (fuzzy_string_compare(state->current_token, "variable"))
				{
					shift_Parse_state(state,1);
					return_code=execute_assign_variable(state,(void *)NULL,
						(void *)NULL);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_command_assign:  Only \"assign variable VARIABLE_NAME value\" implemented currently");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					"\n      variable");
				/* By not shifting the parse state the rest of the help should come out */
				return_code=execute_assign_variable(state,(void *)NULL,
					(void *)NULL);
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				set_command_prompt("assign",command_data->command_window);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_assign.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_assign.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_assign */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_create(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 1 October 1998

DESCRIPTION :
Executes a CREATE command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"cm",NULL,NULL,gfx_create_cmiss},
		{NULL,NULL,NULL,execute_command_cm}
	};

	ENTER(execute_command_cell);
	/* check argument */
	if (state)
	{
		(option_table[0]).user_data=command_data_void;
		(option_table[1]).user_data=command_data_void;
		(option_table[1]).to_be_modified=prompt_void;
		return_code=process_option(state,option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_create.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_create */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (NO_HELP)
static int execute_command_help(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 24 November 1997

DESCRIPTION :
Executes a HELP command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_help);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
#if !defined (HYPERTEXT_HELP) && !defined (NETSCAPE_HELP)
#if !defined (WINDOWS_DEV_FLAG)
					do_help(current_token);
#endif /* !defined (WINDOWS_DEV_FLAG) */
#else
					strcpy(global_temp_string,command_data->help_url);
					strcat(global_temp_string,current_token);
					strcat(global_temp_string,"/");
					do_help(global_temp_string,command_data->examples_directory,
						command_data->execute_command,command_data->user_interface);
#endif
				}
				else
				{
					display_message(INFORMATION_MESSAGE," WORD");
					return_code=1;
				}
			}
			else
			{
#if !defined (HYPERTEXT_HELP) && !defined (NETSCAPE_HELP)
#if !defined (WINDOWS_DEV_FLAG)
				do_help(" ",command_data->execute_command,command_data->user_interface);
#endif /* !defined (WINDOWS_DEV_FLAG) */
#else
				do_help(command_data->help_url,command_data->examples_directory,
					command_data->execute_command,command_data->user_interface);
#endif
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_help.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_help.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_help */
#endif /* !defined (NO_HELP) */

#if !defined (WINDOWS_DEV_FLAG)
#if defined (INCLUDE_XVG)
static int execute_command_imp_draw_number(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Executes a IMP DRAW_NUMBER command.
???DB.  Should be set_char_flag ?
???RC.  Is this used?
==============================================================================*/
{
	char *current_token;
	int return_code;

	ENTER(execute_command_imp_draw_number);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (!((current_token=state->current_token)&&
			!(strcmp(PARSER_HELP_STRING,current_token)&&
			strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
		{
			if (data_draw_number)
			{
				data_draw_number=0;
			}
			else
			{
				data_draw_number=1;
			}
			return_code=1;
		}
		else
		{
			/* no help */
			return_code=1;
		}
	}
	else
	{
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* execute_command_imp_draw_number */

static int execute_command_imp_load(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a IMP LOAD command.
???RC.  Is this used?
==============================================================================*/
{
	char *current_token;
	int return_code;

	ENTER(execute_command_imp_load);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (!((current_token=state->current_token)&&
			!(strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
		{
			return_code=LoadFile(current_token);
		}
		else
		{
			/* no help */
			return_code=1;
		}
	}
	else
	{
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* execute_command_imp_load */

static int execute_command_imp_open(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 2 February 1999

DESCRIPTION :
Executes a IMP OPEN command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_imp_open);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (!((current_token=state->current_token)&&
				!(strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				/*???DB.  Does this have a return code */
				xvg_popup_interface(command_data->fe_field_manager,
					command_data->data_manager,command_data->data_group_manager,
					command_data->user_interface);
				return_code=1;
			}
			else
			{
				/* no help */
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_imp_open.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_imp_open.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_imp_open */
#endif /* defined (INCLUDE_XVG) */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_imp(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a IMP command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
#if defined (INCLUDE_XVG)
		{"draw_number",NULL,NULL,execute_command_imp_draw_number},
		{"load",NULL,NULL,execute_command_imp_load},
		{"open",NULL,NULL,execute_command_imp_open},
#endif /* defined (INCLUDE_XVG) */
		{NULL,NULL,NULL,execute_command_cm}
	};

	ENTER(execute_command_imp);
	/* check argument */
	if (state)
	{
#if defined (INCLUDE_XVG)
		(option_table[0]).user_data=NULL;
		(option_table[1]).user_data=NULL;
		(option_table[2]).user_data=command_data_void;
		(option_table[3]).user_data=command_data_void;
		(option_table[3]).to_be_modified=prompt_void;
#else
		(option_table[0]).user_data=command_data_void;
		(option_table[0]).to_be_modified=prompt_void;
#endif /* defined (INCLUDE_XVG) */
		return_code=process_option(state,option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_imp.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_imp */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int execute_command_list_memory(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a LIST_MEMORY command.
==============================================================================*/
{
	char increment_counter, suppress_pointers;
	int count_number,return_code;
	static struct Modifier_entry option_table[]=
	{
		{"increment_counter",NULL,NULL,set_char_flag},
		{"suppress_pointers",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,set_int}
	};

	ENTER(execute_command_list_memory);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		count_number=0;
		increment_counter = 0;
		suppress_pointers = 0;
		(option_table[0]).to_be_modified= &increment_counter;
		(option_table[1]).to_be_modified= &suppress_pointers;
		(option_table[2]).to_be_modified= &count_number;
		return_code=process_multiple_options(state,option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (suppress_pointers)
			{
				return_code=list_memory(count_number, /*show_pointers*/0,
					increment_counter, /*show_structures*/0);
			}
			else
			{
				return_code=list_memory(count_number, /*show_pointers*/1,
					increment_counter, /*show_structures*/1);
			}
		} /* parse error, help */
		else
		{
			/* no help */
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_list_memory.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_list_memory */

#if defined (OLD_CODE)
#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_open_comfile(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 24 September 1996

DESCRIPTION :
Executes a OPEN COMFILE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_open_comfile);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			return_code=open_comfile(state,command_data->execute_command,
				command_data->set_file_name_option_table,command_data->user_interface);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_open_comfile.  Missing command_data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_open_comfile.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_open_comfile */
#endif /* !defined (WINDOWS_DEV_FLAG) */
#endif /* defined (OLD_CODE) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_open_menu(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 24 September 1996

DESCRIPTION :
Executes a OPEN MENU command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_open_menu);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			return_code=open_menu(state,command_data->execute_command,
				command_data->set_file_name_option_table,command_data->user_interface);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_open_menu.  Missing command_data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_open_menu.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_open_menu */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_read(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
Executes a READ command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"comfile",NULL,NULL,open_comfile},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Open_comfile_data open_comfile_data;

	ENTER(execute_command_read);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				open_comfile_data.file_name=(char *)NULL;
				open_comfile_data.example_flag=0;
				open_comfile_data.execute_count=1;
				open_comfile_data.examples_directory=command_data->example_directory;
				open_comfile_data.example_symbol=CMGUI_EXAMPLE_DIRECTORY_SYMBOL;
				open_comfile_data.execute_command=command_data->execute_command;
				open_comfile_data.set_command=command_data->set_command;
				open_comfile_data.file_extension=".com";
				open_comfile_data.user_interface=command_data->user_interface;
				(option_table[0]).user_data=(void *)&open_comfile_data;
				return_code=process_option(state,option_table);
			}
			else
			{
				set_command_prompt("read",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_read.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_read.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_read */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_open(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
Executes a OPEN command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"comfile",NULL,NULL,open_comfile},
		{"menu",NULL,NULL,execute_command_open_menu},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Open_comfile_data open_comfile_data;

	ENTER(execute_command_open);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				open_comfile_data.file_name=(char *)NULL;
				open_comfile_data.example_flag=0;
				open_comfile_data.execute_count=0;
				open_comfile_data.examples_directory=command_data->example_directory;
				open_comfile_data.example_symbol=CMGUI_EXAMPLE_DIRECTORY_SYMBOL;
				open_comfile_data.execute_command=command_data->execute_command;
				open_comfile_data.set_command=command_data->set_command;
				open_comfile_data.file_extension=".com";
				open_comfile_data.user_interface=command_data->user_interface;
				(option_table[0]).user_data=(void *)&open_comfile_data;
				(option_table[1]).user_data=command_data_void;
				return_code=process_option(state,option_table);
			}
			else
			{
				set_command_prompt("open",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_open.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_open.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_open */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_quit(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a QUIT command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_quit);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (!((current_token=state->current_token)&&
			!(strcmp(PARSER_HELP_STRING,current_token)&&
			strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				REMOVE_ALL_OBJECTS_FROM_MANAGER(Graphics_window)(
					command_data->graphics_window_manager);
				REMOVE_ALL_OBJECTS_FROM_MANAGER(Scene)(command_data->scene_manager);
				close_user_interface(command_data->user_interface);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_quit.  Invalid command_data");
				return_code=0;
			}
		}
		else
		{
			/* no help */
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_quit.	Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_quit */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if defined (OLD_CODE)
#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_set_dir_example(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Executes a SET DIR #CMGUI_EXAMPLE_DIRECTORY_SYMBOL command.
==============================================================================*/
{
	char *current_token,*example_directory,*temp_char;
	int current_token_length,file_name_length,i,return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_set_dir_example);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (command_data=(struct Cmiss_command_data *)command_data_void)
				{
					/* construct the example directory path */
					if ((current_token_length=strlen(current_token))>0)
					{
						file_name_length=
							1+(current_token_length*(current_token_length+3))/2;
						if (command_data->examples_directory)
						{
							file_name_length += strlen(command_data->examples_directory);
						}
						if (ALLOCATE(example_directory,char,file_name_length))
						{
							*example_directory='\0';
							if (command_data->examples_directory)
							{
								strcat(example_directory,command_data->examples_directory);
							}
							temp_char=example_directory+strlen(example_directory);
							for (i=1;i<=current_token_length;i++)
							{
								strncpy(temp_char,current_token,i);
								temp_char += i;
								*temp_char='/';
								temp_char++;
							}
							*temp_char='\0';
							DEALLOCATE(command_data->example_directory);
							command_data->example_directory=example_directory;
							/* send command to the back end */
							return_code=execute_command_cm(state,(void *)NULL,
								command_data_void);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"execute_command_set_dir_example.  Insufficient memory");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"execute_command_set_dir_example.  Invalid example name");
					}
#if defined (OLD_CODE)
					file_name_length=strlen(current_token)+2;
					if (command_data->examples_directory)
					{
						file_name_length += strlen(command_data->examples_directory);
					}
					if (ALLOCATE(example_directory,char,file_name_length))
					{
						*example_directory='\0';
						if (command_data->examples_directory)
						{
							strcat(example_directory,command_data->examples_directory);
						}
						strcat(example_directory,current_token);
						strcat(example_directory,"/");
						DEALLOCATE(command_data->example_directory);
						command_data->example_directory=example_directory;
						return_code=execute_command_cm(state,(void *)NULL,
							command_data_void);
					}
					else
					{
						display_message(ERROR_MESSAGE,
"execute_command_set_dir_example.  Insufficient memory for relative example directory");
						return_code=0;
					}
#endif /* defined (OLD_CODE) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_command_set_dir_example.  Missing command_data");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," RELATIVE_EXAMPLE_DIRECTORY");
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing graphics object name");
			display_parse_state_location(state);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_set_dir_example.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_set_dir_example */
#endif /* !defined (WINDOWS_DEV_FLAG) */
#endif /* defined (OLD_CODE) */

#if !defined (WINDOWS_DEV_FLAG)
static int set_dir(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 November 1999

DESCRIPTION :
Executes a SET DIR command.
==============================================================================*/
{
	char *directory_name, *example_directory, example_flag, *token;
	int file_name_length, return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{CMGUI_EXAMPLE_DIRECTORY_SYMBOL,CMGUI_EXAMPLE_DIRECTORY_SYMBOL,NULL,
			set_char_flag},
		{NULL,NULL,NULL,set_name}
	};

	ENTER(set_dir);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (token = state->current_token)
			{
				directory_name = (char *)NULL;
				example_flag = 0;
				(option_table[0]).to_be_modified = &example_flag;
				(option_table[1]).to_be_modified = &directory_name;
				return_code=process_multiple_options(state,option_table);
				if (return_code)
				{
					if (example_flag)
					{
						if (directory_name)
						{							
							/* Lookup the example path */
							if (example_directory = 
								resolve_example_path(command_data->examples_directory, directory_name))
							{
								if (command_data->example_directory)
								{
									DEALLOCATE(command_data->example_directory);
								}
								command_data->example_directory=example_directory;
								/* send command to the back end */
								/* have to reset the token position to get it to
									export the command */
								state->current_token = token;
								return_code=execute_command_cm(state,(void *)NULL,
								  command_data_void);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_dir.  Unable to resolve example path.");
								return_code = 0;
							}
#if defined (OLD_CODE)
							/* construct the example directory path */
							if ((directory_name_length=strlen(directory_name))>0)
							{
								file_name_length=
									1+(directory_name_length*(directory_name_length+3))/2;
								if (command_data->examples_directory)
								{
									file_name_length += strlen(command_data->examples_directory);
								}
								if (ALLOCATE(example_directory,char,file_name_length))
								{
									*example_directory='\0';
									if (command_data->examples_directory)
									{
										strcat(example_directory,command_data->examples_directory);
									}
									temp_char=example_directory+strlen(example_directory);
									for (i=1;i<=directory_name_length;i++)
									{
										strncpy(temp_char,directory_name,i);
										temp_char += i;
										*temp_char='/';
										temp_char++;
									}
									*temp_char='\0';
									DEALLOCATE(command_data->example_directory);
									command_data->example_directory=example_directory;
									/* send command to the back end */
									/* have to reset the token position to get it to
										export the command */
									state->current_token = token;
									return_code=execute_command_cm(state,(void *)NULL,
										command_data_void);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"set_dir.  Insufficient memory");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_dir.  Invalid example name");
							}
#endif /* defined (OLD_CODE) */
						}
						else
						{
							file_name_length = 1;
							if (command_data->examples_directory)
							{
								file_name_length += strlen(command_data->examples_directory);
							}
							if (ALLOCATE(example_directory,char,file_name_length))
							{
								*example_directory='\0';
								if (command_data->examples_directory)
								{
									strcat(example_directory,command_data->examples_directory);
								}
								DEALLOCATE(command_data->example_directory);
								command_data->example_directory=example_directory;
#if defined (LINK_CMISS)
								if (CMISS)
								{
									/* send command to the back end */
									/* have to reset the token position to get it to
										export the command */
									state->current_token = token;
									return_code=execute_command_cm(state,(void *)NULL,
										command_data_void);
								}
#endif /* defined (LINK_CMISS) */
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_dir.  Insufficient memory");
							}
						}
					} 
					else
					{
						if(chdir(directory_name))
						{
							display_message(ERROR_MESSAGE,
								"set_dir.  Unable to change to directory %s",
								directory_name);
							
						}
						return_code=execute_command_cm(state,(void *)NULL,
							command_data_void);
					}
				}
				if (directory_name)
				{
					DEALLOCATE(directory_name);
				}
			}
			else
			{
				set_command_prompt("set dir",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_set_dir.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_set_dir.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_dir */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_set(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Executes a SET command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"directory","dir",NULL,set_dir},
		{NULL,NULL,NULL,execute_command_cm}
	};

	ENTER(execute_command_set);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				(option_table[0]).user_data=command_data_void;
				(option_table[1]).user_data=command_data_void;
				return_code=process_option(state,option_table);
			}
			else
			{
				set_command_prompt("set",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_set.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_set.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_set */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_system(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 19 February 1998

DESCRIPTION :
Executes a SET DIR #CMGUI_EXAMPLE_DIRECTORY_SYMBOL command.
???RC Obsolete?
==============================================================================*/
{
	char *command,*current_token,*system_command;
	int return_code;

	ENTER(execute_command_system);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	return_code=0;
	/* check argument */
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				command=strstr(state->command_string,current_token);
				if (ALLOCATE(system_command,char,strlen(command)+1))
				{
					strcpy(system_command,command);
					parse_variable(&system_command);
					system(system_command);
					DEALLOCATE(system_command);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_command_system.  Insufficient memory");
					return_code=0;
				}
			}
			else
			{
				/* no help */
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing graphics object name");
			display_parse_state_location(state);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_system.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_system */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_variable(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 June 1996

DESCRIPTION :
Executes a VARIABLE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_variable);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (state->current_token)
		{
			return_code=execute_variable_command(state,(void *)NULL,
				(void *)NULL);
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				set_command_prompt("var",command_data->command_window);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_variable.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_variable.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_variable */
#endif /* !defined (WINDOWS_DEV_FLAG) */

/*
Global functions
----------------
*/
#if defined (F90_INTERPRETER) || defined (PERL_INTERPRETER)
int cmiss_actually_execute_command(char *command_string,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 March 2000

DESCRIPTION:
==============================================================================*/
{
#if !defined (WINDOWS_DEV_FLAG)
	char **token;
#endif /* !defined (WINDOWS_DEV_FLAG) */
	int i,return_code;
	struct Cmiss_command_data *command_data;
#if !defined (WINDOWS_DEV_FLAG)
	static struct Modifier_entry option_table[]=
	{
		{"assign","assign",NULL,execute_command_assign},
		{"cell","cell",NULL,execute_command_cell},
		{"command_window","command_window",NULL,modify_Command_window},
		{"create","create",NULL,execute_command_create},
		{"fem","fem",NULL,execute_command_cm},
		{"gen","gen",NULL,execute_command_cm},
		{"gfx",NULL,NULL,execute_command_gfx},
#if !defined (NO_HELP)
		{"help",NULL,NULL,execute_command_help},
#endif /* !defined (NO_HELP) */
		{"imp","imp",NULL,execute_command_imp},
		{"open",NULL,NULL,execute_command_open},
		{"quit",NULL,NULL,execute_command_quit},
		{"list_memory",NULL,NULL,execute_command_list_memory},
		{"read",NULL,NULL,execute_command_read},
		{"set","set",NULL,execute_command_set},
		{"system","system",NULL,execute_command_system},
		{"unemap","unemap",NULL,execute_command_unemap},
		{"var","var",NULL,execute_command_variable},
		{NULL,NULL,NULL,execute_command_cm}
	};
	struct Parse_state *state;
#endif /* !defined (WINDOWS_DEV_FLAG) */

	ENTER(cmiss_execute_command);
	if (command_data=(struct Cmiss_command_data *)command_data_void)
	{
#if !defined (WINDOWS_DEV_FLAG)
		if (state=create_Parse_state(command_string))
			/*???DB.  create_Parse_state has to be extended */
		{
			i=state->number_of_tokens;
			/* check for comment */
			if (i>0)
			{
				/* add command to command history */
				/*???RC put out processed tokens instead? */
				display_message(INFORMATION_MESSAGE,
					"%s\n", command_string);
				/* check for a "<" as one of the of the tokens */
					/*???DB.  Include for backward compatability.  Remove ? */
				token=state->tokens;
				while ((i>0)&&strcmp(*token,"<"))
				{
					i--;
					token++;
				}
				if (i>0)
				{
					/* return to tree root */
					return_code=set_command_prompt("",command_data->command_window);
				}
				else
				{
					/* assign */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* cell */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* command_window */
					(option_table[i]).user_data=command_data->command_window;
					i++;
					/* create */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* fem */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* gen */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* gfx */
					(option_table[i]).user_data=command_data_void;
					i++;
#if !defined (NO_HELP)
					/* help */
					(option_table[i]).user_data=command_data_void;
					i++;
#endif /* !defined (NO_HELP) */
					/* imp */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* open */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* quit */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* list_memory */
					(option_table[i]).user_data=NULL;
					i++;
					/* read */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* set */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* system */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* unemap */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* var */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* default */
					(option_table[i]).user_data=command_data_void;
					i++;
					return_code=process_option(state,option_table);
				}
			}
			if (command_data->command_window)
			{
				reset_command_box(command_data->command_window);
			}
			destroy_Parse_state(&state);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmiss_execute_command.  Could not create parse state");
			return_code=0;
		}
#else
		return_code=1;
#endif /* !defined (WINDOWS_DEV_FLAG) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmiss_execute_command.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmiss_execute_command */

int cmiss_execute_command(char *command_string,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 24 March 2000

DESCRIPTION:
Takes a <command_string>, processes this through the F90 interpreter
and then executes the returned strings
==============================================================================*/
{
	int i,return_code;
	struct Cmiss_command_data *command_data;

	ENTER(cmiss_execute_command);
	if (command_data=(struct Cmiss_command_data *)command_data_void)
	{
		if (command_data->command_window)
		{
			add_to_command_list(command_string,command_data->command_window);
		}
		cmiss_interpreter_execute_command(command_string, command_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmiss_execute_command.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmiss_execute_command */
#else /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */
int cmiss_execute_command(char *command_string,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION:
Execute a <command_string>. If there is a command
==============================================================================*/
{
#if !defined (WINDOWS_DEV_FLAG)
	char **token;
#endif /* !defined (WINDOWS_DEV_FLAG) */
	int i,return_code;
	struct Cmiss_command_data *command_data;
#if !defined (WINDOWS_DEV_FLAG)
	static struct Modifier_entry option_table[]=
	{
		{"assign","assign",NULL,execute_command_assign},
		{"cell","cell",NULL,execute_command_cell},
		{"command_window","command_window",NULL,modify_Command_window},
		{"create","create",NULL,execute_command_create},
		{"fem","fem",NULL,execute_command_cm},
		{"gen","gen",NULL,execute_command_cm},
		{"gfx",NULL,NULL,execute_command_gfx},
#if !defined (NO_HELP)
		{"help",NULL,NULL,execute_command_help},
#endif /* !defined (NO_HELP) */
		{"imp","imp",NULL,execute_command_imp},
		{"open",NULL,NULL,execute_command_open},
		{"quit",NULL,NULL,execute_command_quit},
		{"list_memory",NULL,NULL,execute_command_list_memory},
		{"read",NULL,NULL,execute_command_read},
		{"set","set",NULL,execute_command_set},
		{"system","system",NULL,execute_command_system},
		{"unemap","unemap",NULL,execute_command_unemap},
		{"var","var",NULL,execute_command_variable},
		{NULL,NULL,NULL,execute_command_cm}
	};
	struct Parse_state *state;
#endif /* !defined (WINDOWS_DEV_FLAG) */

	ENTER(cmiss_execute_command);
	if (command_data=(struct Cmiss_command_data *)command_data_void)
	{
#if !defined (WINDOWS_DEV_FLAG)
		if (state=create_Parse_state(command_string))
			/*???DB.  create_Parse_state has to be extended */
		{
			i=state->number_of_tokens;
			/* check for comment */
			if (i>0)
			{
				/* add command to command history */
				/*???RC put out processed tokens instead? */
				if (command_data->command_window)
				{
					add_to_command_list(command_string,command_data->command_window);
				}
				/* check for a "<" as one of the of the tokens */
					/*???DB.  Include for backward compatability.  Remove ? */
				token=state->tokens;
				while ((i>0)&&strcmp(*token,"<"))
				{
					i--;
					token++;
				}
				if (i>0)
				{
					/* return to tree root */
					return_code=set_command_prompt("",command_data->command_window);
				}
				else
				{
					/* assign */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* cell */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* command_window */
					(option_table[i]).user_data=command_data->command_window;
					i++;
					/* create */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* fem */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* gen */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* gfx */
					(option_table[i]).user_data=command_data_void;
					i++;
#if !defined (NO_HELP)
					/* help */
					(option_table[i]).user_data=command_data_void;
					i++;
#endif /* !defined (NO_HELP) */
					/* imp */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* open */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* quit */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* list_memory */
					(option_table[i]).user_data=NULL;
					i++;
					/* read */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* set */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* system */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* unemap */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* var */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* default */
					(option_table[i]).user_data=command_data_void;
					i++;
					return_code=process_option(state,option_table);
				}
			}
			if (command_data->command_window)
			{
				reset_command_box(command_data->command_window);
			}
			destroy_Parse_state(&state);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmiss_execute_command.  Could not create parse state");
			return_code=0;
		}
#else
		return_code=1;
#endif /* !defined (WINDOWS_DEV_FLAG) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmiss_execute_command.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmiss_execute_command */
#endif  /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */

int cmiss_set_command(char *command_string,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION:
Sets the <command_string> in the command box of the CMISS command_window, ready
for editing and entering. If there is no command_window, does nothing.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(cmiss_set_command);
	if (command_string&&
		(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (command_data->command_window)
		{
			return_code=Command_window_set_command_string(
				command_data->command_window,command_string);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"cmiss_set_command.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmiss_set_command */
