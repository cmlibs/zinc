/*******************************************************************************
FILE : unemap_package.c

LAST MODIFIED : 28 April 2000

DESCRIPTION :
Contains function definitions for unemap package.
==============================================================================*/
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "finite_element/finite_element.h"
#include "finite_element/computed_field.h"
#include "general/debug.h"
#include "graphics/colour.h"
#include "graphics/graphics_window.h"
#include "graphics/graphical_element.h"
#include "unemap/unemap_package.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

#if defined (UNEMAP_USE_NODES)

#define map_element_discretization_size 6

struct Map_info	
/* Info about the last map. Has it's own structure as we can have more than one */
/* per unemap_package, i.e more than one per map_scene.*/
{	
	char *fit_name;
	enum Electrodes_option electrodes_option;
	enum Region_type region_type;	
	FE_value electrode_size;
	int access_count,number_of_contours,number_of_map_columns,number_of_map_rows;	
	int rig_node_group_number;/* the corresponding rig_node_group in unmap_package */
	struct GT_element_settings **contour_settings;
	struct FE_field *map_position_field,*map_fit_field;	
	struct FE_node_order_info *node_order_info;	
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *node_group;		
	struct GT_object *electrode_glyph;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_element))	*element_group_manager;	
	struct MANAGER(GROUP(FE_node)) *data_group_manager,*node_group_manager;  

};

static struct Map_info *CREATE(Map_info)(	int number_of_map_rows,
	int number_of_map_columns,int rig_node_group_number,
	enum Region_type region_type,char *fit_name,
	struct FE_node_order_info *node_order_info,
	struct FE_field *map_position_field,struct FE_field *map_fit_field,
	struct GROUP(FE_node) *node_group,struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(GROUP(FE_element))	*element_group_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager, 
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(Computed_field) *computed_field_manager)
/*******************************************************************************
LAST MODIFIED : 8 September 1999

DESCRIPTION:
Create and  and set it's components 
==============================================================================*/
{
	int string_length;
	struct Map_info *map_info;

	ENTER(CREATE(Map_info));
	if(fit_name&&node_order_info&&map_position_field&&map_fit_field)
	{
		if (ALLOCATE(map_info,struct Map_info,1))
		{
			string_length = strlen(fit_name);	
			string_length++;			
			if(ALLOCATE(map_info->fit_name,char,string_length))
			{	
				strcpy(map_info->fit_name,fit_name);
				map_info->rig_node_group_number=rig_node_group_number;
				map_info->number_of_map_rows=number_of_map_rows;
				map_info->number_of_map_columns=number_of_map_columns;
				map_info->number_of_contours=0;
				map_info->contour_settings=(struct GT_element_settings **)NULL;
				map_info->region_type=region_type;				
				map_info->node_order_info=ACCESS(FE_node_order_info)
					(node_order_info);
				map_info->map_position_field=ACCESS(FE_field)(map_position_field);			
				map_info->map_fit_field=ACCESS(FE_field)(map_fit_field);
				map_info->node_group=ACCESS(GROUP(FE_node))(node_group);
				map_info->fe_field_manager=fe_field_manager;
				map_info->element_group=(struct GROUP(FE_element) *)NULL;
				map_info->element_group_manager=element_group_manager;
				map_info->node_manager=node_manager;
				map_info->data_group_manager=data_group_manager;
				map_info->node_group_manager=node_group_manager; 
				map_info->element_manager=element_manager;
				map_info->computed_field_manager=computed_field_manager;
				map_info->electrode_glyph=(struct GT_object *)NULL;
				map_info->electrode_size=0;
				map_info->electrodes_option=HIDE_ELECTRODES;
				map_info->access_count=0.0;
			}
			else
			{
				display_message(ERROR_MESSAGE,"CREATE(Map_info)."
					"Out of memory");	
				if (map_info)
				{
					DEALLOCATE(map_info);
				}		
			}			
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(Map_info).  Not enough memory");
			if (map_info)
			{
				DEALLOCATE(map_info);
			}
		}
	}
	LEAVE;
	return (map_info);	
}

static int free_map_info_map_contours(struct Map_info *map_info)
/*******************************************************************************
LAST MODIFIED : 16 May 2000

DESCRIPTION :
Frees the array of map contour GT_element_settings stored in the <map_info>
==============================================================================*/
{
	int i,number_of_contours,return_code;	

	ENTER(free_map_info_map_contours);
	if(map_info)
	{
		return_code=1;
		number_of_contours=map_info->number_of_contours;
		if(number_of_contours)
		{					
			for(i=0;i<number_of_contours;i++)
			{										
				DEACCESS(GT_element_settings)(&(map_info->contour_settings[i]));
			}								
			DEALLOCATE(map_info->contour_settings);
			map_info->contour_settings=(struct GT_element_settings **)NULL;
		}	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"free_map_info_map_contours Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
}/*free_map_info_map_contours */

static int DESTROY(Map_info)(struct Map_info **map_info_address)
/*******************************************************************************
LAST MODIFIED : 8 September 1999

DESCRIPTION :
Frees the memory for the Map_info and sets <*package_address>
to NULL.
==============================================================================*/
{
	int return_code,success;
	struct FE_field *temp_field;
	struct Map_info *map_info;
	struct FE_element *element_to_destroy;
	struct GROUP(FE_element) *map_element_group,*temp_map_element_group;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(GROUP(FE_element))	*element_group_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager,*node_group_manager; 
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(DESTROY(Map_info));
	if ((map_info_address)&&(map_info= *map_info_address))
	{
		element_manager=map_info->element_manager;
		element_group_manager=map_info->element_group_manager;
		node_manager=map_info->node_manager;
		data_group_manager=map_info->data_group_manager;
		node_group_manager=map_info->node_group_manager;
		fe_field_manager=map_info->fe_field_manager;
		computed_field_manager=map_info->computed_field_manager;
		return_code=1;
		/* electrode_glyph*/
		DEACCESS(GT_object)(&(map_info->electrode_glyph));
		/* node_order_info */
		DEACCESS(FE_node_order_info)(&(map_info->node_order_info));
		/*element_group */
		success=1;
		/* contours*/
		free_map_info_map_contours(map_info);
		map_element_group=map_info->element_group;
		while(success&&(element_to_destroy=FIRST_OBJECT_IN_GROUP_THAT(FE_element)
			((GROUP_CONDITIONAL_FUNCTION(FE_element) *)NULL, NULL,map_element_group)))
		{
			success = REMOVE_OBJECT_FROM_GROUP(FE_element)(
				element_to_destroy,map_element_group);				
			if (FE_element_can_be_destroyed(element_to_destroy))
			{				
				success = REMOVE_OBJECT_FROM_MANAGER(FE_element)(element_to_destroy,
					element_manager);					
			}					
		}		
		temp_map_element_group=map_element_group;
		DEACCESS(GROUP(FE_element))(&temp_map_element_group);
		if(MANAGED_GROUP_CAN_BE_DESTROYED(FE_element)(map_element_group))
		{
			REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_element))(map_element_group,
				element_group_manager);
		}
		/* map_group */
		free_node_and_element_and_data_groups(map_info->node_group,
			element_manager,element_group_manager,node_manager,
			data_group_manager,node_group_manager);
		/* computed_fields */
		remove_computed_field_from_manager_given_FE_field(
			computed_field_manager,map_info->map_fit_field);		
		remove_computed_field_from_manager_given_FE_field(computed_field_manager,
			map_info->map_position_field);		
		/* map_fit_field */
		if(map_info->map_fit_field)
		{
			temp_field=map_info->map_fit_field; 		
			DEACCESS(FE_field)(&temp_field);
			destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				map_info->map_fit_field);
			map_info->map_fit_field=(struct FE_field *)NULL;
		}
		/* map_position_field */
		if(map_info->map_position_field)
		{
			temp_field=map_info->map_position_field; 		
			DEACCESS(FE_field)(&temp_field);
			destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				map_info->map_position_field);
			map_info->map_position_field=(struct FE_field *)NULL;
		}
		DEALLOCATE(map_info->contour_settings);
		/* fit_name */
		DEALLOCATE(map_info->fit_name);
		DEALLOCATE(*map_info_address);
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Map_info) */

static struct Map_info *ACCESS(Map_info)(struct Map_info *map_info) 
/***************************************************************************** 
LAST MODIFIED : 8 September 1999

DESCRIPTION :
Increases the <access_count> for the <map_info> by 1.  Returns the <map_info>.
==============================================================================*/ 
{
	ENTER(ACCESS(Map_info));
	if (map_info)
	{
		(map_info->access_count)++;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ACCESS(Map_info) .  Invalid argument");
	}
	LEAVE;

	return (map_info); 
} /* ACCESS(Map_info) */


static int DEACCESS(Map_info)(struct Map_info **map_info_address)
/*****************************************************************************
LAST MODIFIED : 12 March 1999

DESCRIPTION :
Decreases the <access_count> of the map_info by 1. If the access_count becomes
less than or equal to 0, the map_info is destroyed.
In all cases, sets <*map_info_address> to NULL.
Note that the REACCESS function will also destroy map_infos if it reduces their
access_count below 1.
==============================================================================*/
{
	int return_code;
	struct Map_info *map_info;

	ENTER(DEACCESS(Map_info));
	if (map_info_address&&(map_info= *map_info_address))
	{
		(map_info->access_count)--;
		if (map_info->access_count<=0)
		{
			return_code=DESTROY(Map_info)(map_info_address);
		}
		else
		{
			*map_info_address=(struct Map_info *)NULL;
			return_code=1;
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DEACCESS(Map_info) */

static int REACCESS(Map_info)(struct Map_info **map_info_address,
	struct Map_info *new_map_info)
/*****************************************************************************
LAST MODIFIED : 12 March 1999

DESCRIPTION :
Makes <*map_info_address> point to <new_map_info> with safe ACCESS transfer.
If <new_map_info> is not NULL, its access_count is first increased by 1.
If <*map_info_address> is not NULL, its access_count is decreased by 1, and if
it is now less than or equal to 0, the map_info is destroyed.
Finally, <*map_info_address> is set to <new_map_info>.
==============================================================================*/
{
	int return_code;
	struct Map_info *current_map_info;

	ENTER(REACCESS(Map_info));
	if (map_info_address)
	{
		return_code=1;
		if (new_map_info)
		{
			/* access the new map_info */
			(new_map_info->access_count)++;
		}
		if (current_map_info= *map_info_address)
		{
			/* deaccess the current map_info */
			(current_map_info->access_count)--;
			if (current_map_info->access_count<=0)
			{
				DESTROY(Map_info)(map_info_address);
			}
		}
		/* point to the new map_info */
		*map_info_address=new_map_info;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"REACCESS( Map_info ).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* REACCESS(Map_info) */

DECLARE_OBJECT_FUNCTIONS(Unemap_package)

struct Unemap_package *CREATE(Unemap_package)(
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(FE_basis) *fe_basis_manager,
	struct MANAGER(FE_element) *element_manager,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct FE_node_selection *data_selection,
	struct MANAGER(Computed_field) *computed_field_manager,
	struct MANAGER(Graphics_window) *graphics_window_manager,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct MANAGER(Scene) *scene_manager,
	struct MANAGER(Light_model) *light_model_manager,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(FE_node) *data_manager,
	struct LIST(GT_object) *glyph_list)
/*******************************************************************************
LAST MODIFIED : 28 April 2000

DESCRIPTION:
Create a Unemap package, and fill in the managers.
The fields are filed in with set_unemap_package_fields()
==============================================================================*/
{
	struct Unemap_package *package;

	ENTER(CREATE(Unemap_package));
	if (fe_field_manager&&element_group_manager&&node_manager&&
		data_group_manager&&node_group_manager&&fe_basis_manager&&element_manager&&
		element_point_ranges_selection&&element_selection&&node_selection&&
		data_selection&&
		computed_field_manager&&graphics_window_manager&&texture_manager&&
		interactive_tool_manager&&
		scene_manager&&light_model_manager&&light_manager&&spectrum_manager&&
		graphical_material_manager&&data_manager)
	{
		if (ALLOCATE(package,struct Unemap_package,1))
		{
			package->fe_field_manager=fe_field_manager;
			package->element_group_manager=element_group_manager;
			package->node_manager=node_manager;
			package->data_group_manager=data_group_manager;
			package->node_group_manager=node_group_manager;
			package->fe_basis_manager=fe_basis_manager;	
			package->element_manager=element_manager;
			package->element_point_ranges_selection=element_point_ranges_selection;
			package->element_selection=element_selection;
			package->node_selection=node_selection;
			package->data_selection=data_selection;
			package->computed_field_manager=computed_field_manager;			
			/* fields of the rig_nodes */
			package->number_of_electrode_position_fields=0;
			package->electrode_position_fields=(struct FE_field **)NULL;			
			package->number_of_map_electrode_position_fields=0;
			package->map_electrode_position_fields=(struct FE_field **)NULL;
			package->device_name_field=(struct FE_field *)NULL;
			package->device_type_field=(struct FE_field *)NULL;
			package->channel_number_field=(struct FE_field *)NULL;
			package->signal_field=(struct FE_field *)NULL;
			package->signal_minimum_field=(struct FE_field *)NULL;
			package->signal_maximum_field=(struct FE_field *)NULL;	
			package->signal_status_field=(struct FE_field *)NULL;
			package->channel_gain_field=(struct FE_field *)NULL;
			package->channel_offset_field=(struct FE_field *)NULL;
			package->signal_value_at_time_field=(struct Computed_field *)NULL;
			package->time_field=(struct Computed_field *)NULL;		
			package->number_of_maps=0;
			package->maps_info=(struct Map_info **)NULL;
			package->number_of_rig_node_groups=0;
			package->rig_node_groups=(struct GROUP(FE_node) **)NULL;								
			/* for the cmgui graphics window */
			package->viewed_scene=0; 
			package->no_interpolation_colour=create_Colour(0.65,0.65,0.65);
			package->window=(struct Graphics_window *)NULL;
			package->background_colour=create_Colour(0,0,0);
			package->light=(struct Light *)NULL;			
			package->light_model=(struct Light_model *)NULL;			
			package->scene=(struct Scene *)NULL;		
			package->user_interface=(struct User_interface *)NULL;		
			package->computed_field_package=(struct Computed_field_package *)NULL;
			package->time_keeper = (struct Time_keeper *)NULL;	
			package->graphical_material=(struct Graphical_material *)NULL;
			package->graphics_window_manager=graphics_window_manager;
			package->light_manager=light_manager;
			package->texture_manager=texture_manager;
			package->interactive_tool_manager=interactive_tool_manager;
			package->scene_manager=scene_manager;
			package->light_model_manager=light_model_manager;		
			package->spectrum_manager=spectrum_manager;
			package->graphical_material_manager=graphical_material_manager;
			package->data_manager=data_manager;
			package->glyph_list=glyph_list; /* like a manager, so don't access*/
			package->map_element_discretization.number_in_xi1=map_element_discretization_size;
			package->map_element_discretization.number_in_xi2=map_element_discretization_size;
			package->map_element_discretization.number_in_xi3=map_element_discretization_size;
			package->access_count=0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Unemap_package).  Could not allocate memory for node field");
			DEALLOCATE(package);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Unemap_package).  Invalid argument(s)");
		package=(struct Unemap_package *)NULL;
	}
	LEAVE;

	return (package);
} /* CREATE(Unemap_package) */

int DESTROY(Unemap_package)(struct Unemap_package **package_address)
/*******************************************************************************
LAST MODIFIED : 26 April 1999

DESCRIPTION :
Frees the memory for the Unemap_package node field and sets <*package_address>
to NULL.
==============================================================================*/
{
	int return_code,count;
	struct Unemap_package *package;

	ENTER(DESTROY(Unemap_package));
	if ((package_address)&&(package= *package_address))
	{
		for(count=0;count<package->number_of_electrode_position_fields;count++)
		{
			DEACCESS(FE_field)(&(package->electrode_position_fields[count]));		
		}
		for(count=0;count<package->number_of_map_electrode_position_fields;count++)
		{
			DEACCESS(FE_field)(&(package->map_electrode_position_fields[count]));		
		}
		destroy_Colour(&package->no_interpolation_colour);
		destroy_Colour(&package->background_colour);
		package->number_of_electrode_position_fields=0;	
		package->number_of_map_electrode_position_fields=0;
		DEACCESS(FE_field)(&(package->device_name_field));
		DEACCESS(FE_field)(&(package->device_type_field));
		DEACCESS(FE_field)(&(package->channel_number_field));
		DEACCESS(FE_field)(&(package->signal_field));
		DEACCESS(FE_field)(&(package->signal_minimum_field));
		DEACCESS(FE_field)(&(package->signal_maximum_field));	
		DEACCESS(FE_field)(&(package->signal_status_field));
		DEACCESS(FE_field)(&(package->channel_gain_field));
		DEACCESS(FE_field)(&(package->channel_offset_field));	
		DEACCESS(Computed_field)(&(package->signal_value_at_time_field));
		DEACCESS(Computed_field)(&(package->time_field));		
		for(count=0;count<package->number_of_rig_node_groups;count++)
		{		
			DEACCESS(GROUP(FE_node))(&(package->rig_node_groups[count]));
		}
		package->number_of_rig_node_groups=0;
		DEALLOCATE(package->rig_node_groups);			
		for(count=0;count<package->number_of_maps;count++)
		{
			DEACCESS(Map_info)(&(package->maps_info[count]));
		}
		DEALLOCATE(package->maps_info);
		package->number_of_maps=0;
		DEACCESS(Graphics_window )(&package->window);		
		DEACCESS(Light)(&package->light);
		DEACCESS(Light_model)(&package->light_model);		
		DEACCESS(Scene)(&package->scene);		
		DEACCESS(Graphical_material)(&package->graphical_material);	
		DEACCESS(Time_keeper)(&package->time_keeper);	
		DEALLOCATE(*package_address);		
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Unemap_package) */


int get_unemap_package_viewed_scene(struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
gets the viewed_scene flag of the unemap package.
==============================================================================*/
{
	int viewed_scene;

	ENTER(get_unemap_package_viewed_scene)
	if(package)
	{
		viewed_scene=package->viewed_scene;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_viewed_scene."
			" invalid arguments");
		viewed_scene=0;
	}
	LEAVE;
	return(viewed_scene);
}/* get_unemap_package_viewed_scene */

int set_unemap_package_viewed_scene(struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
sets the viewed_scene flag of the unemap package to 1.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_viewed_scene)
	if(package)
	{
		package->viewed_scene=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_viewed_scene."
			" invalid arguments");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* set_unemap_package_viewed_scene */

struct Colour *get_unemap_package_no_interpolation_colour(struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
gets the no_interpolation_colour of the unemap package.
==============================================================================*/
{
	struct Colour *no_interpolation_colour;

	ENTER(get_unemap_package_no_interpolation_colour)
	if(package)
	{
		no_interpolation_colour=package->no_interpolation_colour;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_no_interpolation_colour."
			" invalid arguments");
		no_interpolation_colour=(struct Colour *)NULL;
	}
	LEAVE;
	return(no_interpolation_colour);
}/* get_unemap_package_no_interpolation_colour */

struct FE_field *get_unemap_package_electrode_position_field(
	struct Unemap_package *package,int field_number)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *electrode_position_field;
	ENTER(get_unemap_package_electrode_position_field);
	if(package&&(field_number>=0)&&
		(field_number<=package->number_of_electrode_position_fields))
	{		
		electrode_position_field=package->electrode_position_fields[field_number];
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_electrode_position_field."
				" invalid arguments");
		electrode_position_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (electrode_position_field);
} /* get_unemap_package_electrode_position_field */

int set_unemap_package_electrode_position_field(struct Unemap_package *package,
	struct FE_field *electrode_position_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;
	struct FE_field **fields;

	ENTER(set_unemap_package_electrode_position_field);
	if(package)
	{
		return_code =1;		
		package->number_of_electrode_position_fields++;
		if(REALLOCATE(fields,package->electrode_position_fields,struct FE_field *,
			package->number_of_electrode_position_fields))
		{									
			package->electrode_position_fields=fields;
			package->electrode_position_fields[package->number_of_electrode_position_fields-1]
				=ACCESS(FE_field)(electrode_position_field);
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_unemap_package_electrode_position."
				" out of memory");
			return_code =0;
		}	
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_electrode_position_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_electrode_position_field */

struct Element_discretization *get_unemap_package_map_element_discretization(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Get the Element_discretization for <package>
??JW may want to put this into the Map_info, &/or and read in from Cmgui file
==============================================================================*/
{
	struct Element_discretization *map_element_discretization;
	ENTER(get_unemap_package_map_element_discretization);
	if(package)
	{
		map_element_discretization=&(package->map_element_discretization);
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_map_element_discretization."
				" invalid arguments");
		map_element_discretization = (struct Element_discretization *)NULL;
	}
	LEAVE;
	return (map_element_discretization);
} /* get_unemap_package_map_element_discretization */

struct FE_field *get_unemap_package_map_electrode_position_field(
	struct Unemap_package *package,int map_number)
/*******************************************************************************
LAST MODIFIED : October 19 1999

DESCRIPTION :
gets the map_electrode_position_field  in <package>.
==============================================================================*/
{
	struct FE_field *map_electrode_position_field;

	ENTER(get_unemap_package_map_electrode_position_field);
	if(package&&(map_number>=0)&&
		(map_number<=package->number_of_map_electrode_position_fields))
	{		
		if(package->number_of_map_electrode_position_fields==map_number)
		{			
			/* No map_info,map_electrode_position_field =NULL */	
			map_electrode_position_field=(struct FE_field *)NULL;		
		}
		else	
		{
			map_electrode_position_field=package->map_electrode_position_fields[map_number];
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_map_electrode_position_field."
				" invalid arguments");
		map_electrode_position_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (map_electrode_position_field);
} /* get_unemap_package_map_electrode_position_field */

int set_unemap_package_map_electrode_position_field(struct Unemap_package *package,
	struct FE_field *map_electrode_position_field)
/*******************************************************************************
LAST MODIFIED : October 19 1999

DESCRIPTION :
Sets the map_electrode_position_field  in <package>.
==============================================================================*/
{
	int return_code;
	struct FE_field **fields;

	ENTER(set_unemap_package_map_electrode_position_field);
	if(package)
	{
		return_code =1;		
		package->number_of_map_electrode_position_fields++;
		if(REALLOCATE(fields,package->map_electrode_position_fields,struct FE_field *,
			package->number_of_map_electrode_position_fields))
		{									
			package->map_electrode_position_fields=fields;
			package->map_electrode_position_fields[package->number_of_map_electrode_position_fields-1]
				=ACCESS(FE_field)(map_electrode_position_field);
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_unemap_package_map_electrode_position."
				" out of memory");
			return_code =0;
		}	
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_map_electrode_position_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_map_electrode_position_field */

struct GT_object *get_unemap_package_map_electrode_glyph(
	struct Unemap_package *package,int map_number)
/*******************************************************************************
LAST MODIFIED : October 21 1999

DESCRIPTION :
gets the map_electrode_glyph for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/
{
	struct GT_object *electrode_glyph;

	ENTER(get_unemap_package_map_electrode_glyph);
	if((package)&&(map_number>-1)&&(map_number<=package->number_of_maps))	
	{
		if(package->number_of_maps==map_number)
		{			
			/* No map_info,map_electrode_glyph =NULL */	
			electrode_glyph=(struct GT_object *)NULL;		
		}
		else
		{
			electrode_glyph=package->maps_info[map_number]->electrode_glyph;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_map_electrode_glyph."
			" invalid arguments");
		electrode_glyph=(struct GT_object *)NULL;
	}
	LEAVE;
	return (electrode_glyph);	
} /* get_unemap_package_map_electrode_glyph */

int set_unemap_package_map_electrode_glyph(struct Unemap_package *package,
	struct GT_object *electrode_glyph,int map_number)
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
Sets the electrode_glyph  for map_info <map_number> in <package>.
Set (and get) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3...
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_map_electrode_glyph);
	if(package&&(map_number>-1)&&
		(map_number<package->number_of_maps))
	{		
		return_code =1;
		REACCESS(GT_object)
			(&(package->maps_info[map_number]->electrode_glyph),
				electrode_glyph);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_map_electrode_glyph ."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_map_electrode_glyph */


FE_value get_unemap_package_map_electrode_size(struct Unemap_package *package,
	int map_number)
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
gets the map_electrode_size for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/
{
	FE_value electrode_size;

	ENTER(get_unemap_package_map_electrode_size);
	if((package)&&(map_number>-1)&&(map_number<=package->number_of_maps))	
	{
		if(package->number_of_maps==map_number)
		{			
			/* No map_info,map_electrode_size */	
			electrode_size=0;		
		}
		else
		{
			electrode_size=package->maps_info[map_number]->electrode_size;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_map_electrode_size."
			" invalid arguments");
		electrode_size=0;
	}
	LEAVE;
	return (electrode_size);	
} /* get_unemap_package_map_electrode_size */

int set_unemap_package_map_electrode_size(struct Unemap_package *package,
	FE_value electrode_size,int map_number)
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
Sets the electrode_size  for map_info <map_number> in <package>.
Set (and get) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3...
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_map_electrode_size);
	if(package&&(map_number>-1)&&
		(map_number<package->number_of_maps))
	{		
		return_code =1;
		package->maps_info[map_number]->electrode_size=electrode_size;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_map_electrode_size ."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_map_electrode_size */

enum Electrodes_option get_unemap_package_map_electrodes_option(
	struct Unemap_package *package,int map_number)
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
gets the map_electrodes_option for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/
{
	enum Electrodes_option electrodes_option;

	ENTER(get_unemap_package_map_electrodes_option);
	if((package)&&(map_number>-1)&&(map_number<=package->number_of_maps))	
	{
		if(package->number_of_maps==map_number)
		{			
			/* No map_info,map_electrodes_option */	
			electrodes_option=HIDE_ELECTRODES;		
		}
		else
		{
			electrodes_option=package->maps_info[map_number]->electrodes_option;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_map_electrodes_option."
			" invalid arguments");
		electrodes_option=HIDE_ELECTRODES;
	}
	LEAVE;
	return (electrodes_option);	
} /* get_unemap_package_map_electrodes_option */

int set_unemap_package_map_electrodes_option(struct Unemap_package *package,
	enum Electrodes_option electrodes_option,int map_number)
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
Sets the electrodes_option  for map_info <map_number> in <package>.
Set (and get) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3...
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_map_electrodes_option);
	if(package&&(map_number>-1)&&
		(map_number<package->number_of_maps))
	{		
		return_code =1;
		package->maps_info[map_number]->electrodes_option=electrodes_option;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_map_electrodes_option ."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_map_electrodes_option */

struct FE_field *get_unemap_package_device_name_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *device_name_field;
	ENTER(get_unemap_package_device_name_field);
	if(package)
	{
		device_name_field=package->device_name_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_device_name_field."
				" invalid arguments");
		device_name_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (device_name_field);
} /* get_unemap_package_device_name_field */

int set_unemap_package_device_name_field(struct Unemap_package *package,
	struct FE_field *device_name_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_device_name_field);
	if(package)
	{
		return_code =1;	
		REACCESS(FE_field)(&(package->device_name_field),device_name_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_device_name_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_device_name_field */

struct FE_field *get_unemap_package_device_type_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *device_type_field;
	ENTER(get_unemap_package_device_type_field);
	if(package)
	{
		device_type_field=package->device_type_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_device_type_field."
				" invalid arguments");
		device_type_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (device_type_field);
} /* get_unemap_package_device_type_field */

int set_unemap_package_device_type_field(struct Unemap_package *package,
	struct FE_field *device_type_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_device_type_field);
	if(package)
	{
		return_code =1;
		REACCESS(FE_field)(&(package->device_type_field),device_type_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_device_type_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_device_type_field */

struct FE_field *get_unemap_package_channel_number_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *channel_number_field;
	ENTER(get_unemap_package_channel_number_field);
	if(package)
	{
		channel_number_field=package->channel_number_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_channel_number_field."
				" invalid arguments");
		channel_number_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (channel_number_field);
} /* get_unemap_package_channel_number_field */

int set_unemap_package_channel_number_field(struct Unemap_package *package,
	struct FE_field *channel_number_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_channel_number_field);
	if(package)
	{
		return_code =1;		
		REACCESS(FE_field)(&(package->channel_number_field),channel_number_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_channel_number_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_channel_number_field */

struct FE_field *get_unemap_package_signal_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *signal_field;
	ENTER(get_unemap_package_signal_field);
	if(package)
	{
		signal_field=package->signal_field; 
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_signal_field."
				" invalid arguments");
		signal_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (signal_field);
} /* get_unemap_package_signal_field */

int set_unemap_package_signal_field(struct Unemap_package *package,
	struct FE_field *signal_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_signal_field);
	if(package)
	{
		return_code =1;	
		REACCESS(FE_field)(&(package->signal_field),signal_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_signal_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_signal_field */

struct FE_field *get_unemap_package_signal_minimum_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *signal_minimum_field;
	ENTER(get_unemap_package_signal_minimum_field);
	if(package)
	{
		signal_minimum_field=package->signal_minimum_field; 
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_signal_minimum_field."
				" invalid arguments");
		signal_minimum_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (signal_minimum_field);
} /* get_unemap_package_signal_minimum_field */

int set_unemap_package_signal_minimum_field(struct Unemap_package *package,
	struct FE_field *signal_minimum_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_signal_minimum_field);
	if(package)
	{
		return_code =1;	
		REACCESS(FE_field)(&(package->signal_minimum_field),signal_minimum_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_signal_minimum_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_signal_minimum_field */

struct FE_field *get_unemap_package_signal_maximum_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *signal_maximum_field;
	ENTER(get_unemap_package_signal_maximum_field);
	if(package)
	{
		signal_maximum_field=package->signal_maximum_field; 
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_signal_maximum_field."
				" invalid arguments");
		signal_maximum_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (signal_maximum_field);
} /* get_unemap_package_signal_maximum_field */

int set_unemap_package_signal_maximum_field(struct Unemap_package *package,
	struct FE_field *signal_maximum_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_signal_maximum_field);
	if(package)
	{
		return_code =1;	
		REACCESS(FE_field)(&(package->signal_maximum_field),signal_maximum_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_signal_maximum_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_signal_maximum_field */

struct FE_field *get_unemap_package_signal_status_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : October 8 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *signal_status_field;
	ENTER(get_unemap_package_signal_status_field);
	if(package)
	{
		signal_status_field=package->signal_status_field; 
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_signal_status_field."
				" invalid arguments");
		signal_status_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (signal_status_field);
} /* get_unemap_package_signal_status_field */

int set_unemap_package_signal_status_field(struct Unemap_package *package,
	struct FE_field *signal_status_field)
/*******************************************************************************
LAST MODIFIED : October 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_signal_status_field);
	if(package)
	{
		return_code =1;	
		REACCESS(FE_field)(&(package->signal_status_field),signal_status_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_signal_status_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_signal_status_field */

struct Computed_field *get_unemap_package_signal_value_at_time_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct Computed_field *signal_value_at_time_field;
	ENTER(get_unemap_package_signal_value_at_time_field);
	if(package)
	{
		signal_value_at_time_field=package->signal_value_at_time_field; 
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_signal_value_at_time_field."
				" invalid arguments");
		signal_value_at_time_field = (struct Computed_field *)NULL;
	}
	LEAVE;
	return (signal_value_at_time_field);
} /* get_unemap_package_signal_value_at_time_field */

int set_unemap_package_signal_value_at_time_field(struct Unemap_package *package,
	struct Computed_field *signal_value_at_time_field)
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_signal_value_at_time_field);
	if(package)
	{
		return_code =1;	
		REACCESS(Computed_field)(&(package->signal_value_at_time_field),
			signal_value_at_time_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_signal_value_at_time_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_signal_value_at_time_field */

struct Computed_field *get_unemap_package_time_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct Computed_field *time_field;
	ENTER(get_unemap_package_time_field);
	if(package)
	{
		time_field=package->time_field; 
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_time_field."
				" invalid arguments");
		time_field = (struct Computed_field *)NULL;
	}
	LEAVE;
	return (time_field);
} /* get_unemap_package_time_field */

int set_unemap_package_time_field(struct Unemap_package *package,
	struct Computed_field *time_field)
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_time_field);
	if(package)
	{
		return_code =1;	
		REACCESS(Computed_field)(&(package->time_field),
			time_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_time_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_time_field */

struct FE_field *get_unemap_package_channel_offset_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *channel_offset_field;
	ENTER(get_unemap_package_channel_offset_field);
	if(package)
	{
		channel_offset_field=package->channel_offset_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_channel_offset_field."
				" invalid arguments");
		channel_offset_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (channel_offset_field);
} /* get_unemap_package_channel_offset_field */

int set_unemap_package_channel_offset_field(struct Unemap_package *package,
	struct FE_field *channel_offset_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_channel_offset_field);
	if(package)
	{
		return_code =1;	
		REACCESS(FE_field)(&(package->channel_offset_field),channel_offset_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_channel_offset_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_channel_offset_field */

struct FE_field *get_unemap_package_channel_gain_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *channel_gain_field;
	ENTER(get_unemap_package_channel_gain_field);
	if(package)
	{
		channel_gain_field=package->channel_gain_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_channel_gain_field."
				" invalid arguments");
		channel_gain_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (channel_gain_field);
} /* get_unemap_package_channel_gain_field */

int set_unemap_package_channel_gain_field(struct Unemap_package *package,
	struct FE_field *channel_gain_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_channel_gain_field);
	if(package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->channel_gain_field),channel_gain_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_channel_gain_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_channel_gain_field */

struct MANAGER(FE_field) *get_unemap_package_FE_field_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(FE_field) *fe_field_manager;

	ENTER(get_unemap_package_FE_field_manager);
	if(package)
	{
		fe_field_manager=package->fe_field_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_FE_field_manager."
			" invalid arguments");
		fe_field_manager = (struct MANAGER(FE_field) *)NULL;
	}
	LEAVE;
	return(fe_field_manager);
}/* get_unemap_package_FE_field_manager */

struct MANAGER(Computed_field) *get_unemap_package_Computed_field_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 3 September 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(get_unemap_package_Computed_field_manager);
	if(package)
	{
		computed_field_manager=package->computed_field_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_Computed_field_manager."
			" invalid arguments");
		computed_field_manager = (struct MANAGER(Computed_field) *)NULL;
	}
	LEAVE;
	return(computed_field_manager);
}/* get_unemap_package_Computed_field_manager */

struct MANAGER(GROUP(FE_element)) *get_unemap_package_element_group_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(GROUP(FE_element)) *element_group_manager;

	ENTER(get_unemap_package_element_group_manager);
	if(package)
	{
		element_group_manager=package->element_group_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_element_group_manager."
			" invalid arguments");
		element_group_manager = (struct MANAGER(GROUP(FE_element)) *)NULL;
	}
	LEAVE;
	return(element_group_manager);
}/* get_unemap_package_element_group_manager */

struct MANAGER(FE_node) *get_unemap_package_node_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(FE_node) *node_manager;

	ENTER(get_unemap_package_node_manager);
	if(package)
	{
		node_manager=package->node_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_node_manager."
			" invalid arguments");
		node_manager = (struct MANAGER(FE_node) *)NULL;
	}
	LEAVE;
	return(node_manager);
}/* get_unemap_package_node_manager */

struct MANAGER(FE_element) *get_unemap_package_element_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(FE_element) *element_manager;

	ENTER(get_unemap_package_element_manager);
	if(package)
	{
		element_manager=package->element_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_element_manager."
			" invalid arguments");
		element_manager = (struct MANAGER(FE_element) *)NULL;
	}
	LEAVE;
	return(element_manager);
}/* get_unemap_package_element_manager */

struct MANAGER(FE_basis) *get_unemap_package_basis_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(FE_basis) *basis_manager;

	ENTER(get_unemap_package_basis_manager);
	if(package)
	{
		basis_manager=package->fe_basis_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_basis_manager."
			" invalid arguments");
		basis_manager = (struct MANAGER(FE_basis) *)NULL;
	}
	LEAVE;
	return(basis_manager);
}/* get_unemap_package_basis_manager */


struct MANAGER(GROUP(FE_node)) *get_unemap_package_data_group_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(GROUP(FE_node)) *data_group_manager;

	ENTER(get_unemap_package_data_group_manager);
	if(package)
	{
		data_group_manager=package->data_group_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_data_group_manager."
			" invalid arguments");
		data_group_manager = (struct MANAGER(GROUP(FE_node)) *)NULL;
	}
	LEAVE;
	return(data_group_manager);
}/* get_unemap_package_data_group_manager */

struct MANAGER(GROUP(FE_node)) *get_unemap_package_node_group_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(GROUP(FE_node)) *node_group_manager;

	ENTER(get_unemap_package_node_group_manager);
	if(package)
	{
		node_group_manager=package->node_group_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_node_group_manager."
			" invalid arguments");
		node_group_manager = (struct MANAGER(GROUP(FE_node)) *)NULL;
	}
	LEAVE;
	return(node_group_manager);
}/* get_unemap_package_node_group_manager */

int get_unemap_package_map_number_of_map_rows(
	struct Unemap_package *package,int map_number)
/*******************************************************************************
LAST MODIFIED : September 8 1999

DESCRIPTION :
gets the number_of_map_rows for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
Returns -1 on error
==============================================================================*/
{
	int number_of_map_rows;

	ENTER(get_unemap_package_map_number_of_map_rows);		
	if((package)&&(map_number>-1)&&(map_number<=package->number_of_maps))	
	{
		if(package->number_of_maps==map_number)
		{	
			number_of_map_rows=0; /* No map_info, say number_of_map_rows=0 */
		}
		else
		{		
			number_of_map_rows=package->maps_info[map_number]->number_of_map_rows;	
		}
	}	
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_map_number_of_map_rows."
			" invalid arguments");
		number_of_map_rows=-1;
	}
	LEAVE;
	return (number_of_map_rows);
} /* get_unemap_package_map_number_of_map_rows */

int get_unemap_package_map_number_of_map_columns(
	struct Unemap_package *package,int map_number)
/*******************************************************************************
LAST MODIFIED : September 8 1999

DESCRIPTION :
gets the number_of_map_columns for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3...
Returns -1 on error
==============================================================================*/
{
	int number_of_map_columns;

	ENTER(get_unemap_package_map_number_of_map_columns);
	if((package)&&(map_number>-1)&&(map_number<=package->number_of_maps))	
	{
		if(package->number_of_maps==map_number)
		{
			number_of_map_columns=0; /* No map_info, say number_of_map_columns =0 */			
		}
		else
		{
			number_of_map_columns=package->maps_info[map_number]->number_of_map_columns;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_map_number_of_map_columns."
				" invalid arguments");
		number_of_map_columns=-1;
	}
	LEAVE;
	return (number_of_map_columns);
} /* get_unemap_package_map_number_of_map_columns */

int free_unemap_package_map_contours(struct Unemap_package *package,int map_number)
/*******************************************************************************
LAST MODIFIED : 16 May 2000

DESCRIPTION :
Frees the array of map contour GT_element_settings stored in the <package>
<map_number> map_info.
==============================================================================*/
{
	int return_code;	
	struct Map_info *map_info=(struct Map_info *)NULL;

	ENTER(free_unemap_package_map_contours);
	if(package)
	{
		return_code=1;		
		if((package->maps_info)&&(map_info=package->maps_info[map_number]))
		{
			free_map_info_map_contours(map_info);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"free_unemap_package_map_contours Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
}/* free_unemap_package_map_contours */

int get_unemap_package_map_contours(struct Unemap_package *package,
	int map_number,int *number_of_contours,
	struct GT_element_settings ***contour_settings)
/*******************************************************************************
LAST MODIFIED : 16 May 2000

DESCRIPTION :
gets the <number_of_contours> and <contour_settings> for map_info <map_number> 
in <package>.
get with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... Returns -1 on error
==============================================================================*/
{
	int return_code;

	ENTER(get_unemap_package_map_contours);
	return_code=0;
	if((package)&&(map_number>-1)&&(map_number<package->number_of_maps))	
	{				
		*number_of_contours=package->maps_info[map_number]->number_of_contours;
		*contour_settings=package->maps_info[map_number]->contour_settings;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_map_contours."
			" invalid arguments");
		return_code=0; /* No map_info */
		*number_of_contours=0;
		*contour_settings=(struct GT_element_settings **)NULL;
	}
	LEAVE;
	return (return_code);
} /* get_unemap_package_map_contours */

int set_unemap_package_map_contours(struct Unemap_package *package,int map_number,
	int number_of_contours,struct GT_element_settings **contour_settings)
/*******************************************************************************
LAST MODIFIED : 16 May 2000

DESCRIPTION :
sets the <number_of_contours> and <contour_settings> for map_info <map_number> 
in <package>.
set with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3...
==============================================================================*/
{
	int i,return_code;

	ENTER(set_unemap_package_map_contours)
	return_code=0;
	if((package)&&(map_number>-1)&&(map_number<package->number_of_maps))	
	{
	
		return_code=1;	
		package->maps_info[map_number]->number_of_contours=number_of_contours;
		package->maps_info[map_number]->contour_settings=contour_settings;	
		for(i=0;i<number_of_contours;i++)
		{			
			package->maps_info[map_number]->contour_settings[i]=
				ACCESS(GT_element_settings)(contour_settings[i]);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_map_contours."
			" invalid arguments");	
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_map_contours */

int get_unemap_package_map_rig_node_group_number(
	struct Unemap_package *package,int map_number)
/*******************************************************************************
LAST MODIFIED :October 19 1999

DESCRIPTION :
gets the rig_node_group_number for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3...
Returns -1 on error
==============================================================================*/
{
	int rig_node_group_number;

	ENTER(get_unemap_package_map_rig_node_group_number);
	if((package)&&(map_number>-1)&&(map_number<=package->number_of_maps))	
	{
		if(package->number_of_maps==map_number)
		{
			rig_node_group_number=-1;	
			display_message(ERROR_MESSAGE,"get_unemap_package_map_rig_node_group_number."
				" mo map defined!");	
		}
		else
		{
			rig_node_group_number=package->maps_info[map_number]->rig_node_group_number;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_map_rig_node_group_number."
				" invalid arguments");
		rig_node_group_number=-1;
	}
	LEAVE;
	return (rig_node_group_number);
} /* get_unemap_package_map_rig_node_group_number */

enum Region_type get_unemap_package_map_region_type(
	struct Unemap_package *package,int map_number)
/*******************************************************************************
LAST MODIFIED : September 8 1999

DESCRIPTION :
gets the region_type for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/
{
	enum Region_type region_type;

	ENTER(get_unemap_package_map_region_type);
	if((package)&&(map_number>-1)&&(map_number<=package->number_of_maps))		
	{
		if(package->number_of_maps==map_number)
		{
			region_type=UNKNOWN;/* No map_info, region_type=UNKNOWN */
		}
		else
		{
			region_type=package->maps_info[map_number]->region_type;
			 
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_map_region_type."
				" invalid arguments");
		region_type=UNKNOWN;
	}
	LEAVE;
	return (region_type);
} /* get_unemap_package_map_region_type */

char *get_unemap_package_map_fit_name(
	struct Unemap_package *package,int map_number)
/*******************************************************************************
LAST MODIFIED : September 8 1999

DESCRIPTION :
gets the fit_name for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/
{
	char *fit_name;

	ENTER(get_unemap_package_map_fit_name);
	if((package)&&(map_number>-1)&&(map_number<=package->number_of_maps))			
	{
		if(package->number_of_maps==map_number)
		{		
			fit_name=(char *)NULL;/* No map_info,ast_fit_name=(char *)NULL */
		}
		else
		{
			fit_name=package->maps_info[map_number]->fit_name;	
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_map_fit_name."
				" invalid arguments");
		fit_name=(char *)NULL;
	}
	LEAVE;
	return (fit_name);
} /* get_unemap_package_map_fit_name */

struct FE_node_order_info *get_unemap_package_map_node_order_info(
	struct Unemap_package *package,int map_number)
/*******************************************************************************
LAST MODIFIED : September 8 1999

DESCRIPTION :
gets the node_order_info for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/
{
	struct FE_node_order_info *node_order_info;

	ENTER(get_unemap_package_map_node_order_info);	
	if((package)&&(map_number>-1)&&(map_number<=package->number_of_maps))	
	{
		if(package->number_of_maps==map_number)
		{	
			/* No map_info,node_order_info=NULL */
			node_order_info=(struct FE_node_order_info *)NULL;		
		}
		else
		{
			node_order_info=package->maps_info[map_number]->node_order_info;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_map_node_order_info."
				" invalid arguments");
		node_order_info=(struct FE_node_order_info *)NULL;
	}
	LEAVE;
	return (node_order_info);
} /* get_unemap_package_map_node_order_info */

struct FE_field *get_unemap_package_map_position_field(
	struct Unemap_package *package,int map_number)
/*******************************************************************************
LAST MODIFIED : September 8 1999

DESCRIPTION :
gets the map_position_field for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/
{
	struct FE_field *map_position_field;

	ENTER(get_unemap_package_map_position_field);	
	if((package)&&(map_number>-1)&&(map_number<=package->number_of_maps))		
	{
		if(package->number_of_maps==map_number)
		{			
			/* No map_info, map_position_field=NULL */	
			map_position_field=(struct FE_field *)NULL;		
		}
		else
		{
			map_position_field=package->maps_info[map_number]->map_position_field;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_map_position_field."
				" invalid arguments");
		map_position_field=(struct FE_field *)NULL;
	}
	LEAVE;
	return (map_position_field);
} /* get_unemap_package_map_position_field */

struct FE_field *get_unemap_package_map_fit_field(
	struct Unemap_package *package,int map_number)
/*******************************************************************************
LAST MODIFIED : September 8 1999

DESCRIPTION :
gets the map_fit_field for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/
{
	struct FE_field *map_fit_field;

	ENTER(get_unemap_package_map_fit_field);
	if((package)&&(map_number>-1)&&(map_number<=package->number_of_maps))	
	{
		if(package->number_of_maps==map_number)
		{			
			/* No map_info, map_fit_field=NULL */	
			map_fit_field=(struct FE_field *)NULL;		
		}
		else
		{
			map_fit_field=package->maps_info[map_number]->map_fit_field;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_map_fit_field."
				" invalid arguments");
		map_fit_field=(struct FE_field *)NULL;
	}
	LEAVE;
	return (map_fit_field);
} /* get_unemap_package_map_fit_field */

struct GROUP(FE_node) *get_unemap_package_rig_node_group(
	struct Unemap_package *package,int node_number)
/*******************************************************************************
LAST MODIFIED : August 16 1999

DESCRIPTION :
gets the rig_node_group of the unemap package.
==============================================================================*/
{
	struct GROUP(FE_node) *rig_node_group;

	ENTER(get_unemap_package_rig_node_group);
	if(package)
	{		
		rig_node_group=package->rig_node_groups[node_number];
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_rig_node_group."
			" invalid arguments");
		rig_node_group=(struct GROUP(FE_node) *)NULL;
	}
	LEAVE;
	return (rig_node_group);	
}/* get_unemap_package_rig_node_group */

int set_unemap_package_rig_node_group(struct Unemap_package *package,
	struct GROUP(FE_node) *rig_node_group)
/*******************************************************************************
LAST MODIFIED : August 16 1999

DESCRIPTION :
Allocates and sets the latest rig_node_group pointer of the unemap package.
==============================================================================*/
{
	int return_code;
	struct GROUP(FE_node) **rig_node_groups;
	ENTER(set_unemap_package_rig_node_group);
	if(package&&rig_node_group)
	{		
		package->number_of_rig_node_groups++;
		if(REALLOCATE(rig_node_groups,package->rig_node_groups,struct GROUP(FE_node) *,
			package->number_of_rig_node_groups))
		{									
			package->rig_node_groups=rig_node_groups;
			package->rig_node_groups[package->number_of_rig_node_groups-1]
				=ACCESS(GROUP(FE_node))(rig_node_group);
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_unemap_package_rig_node_group."
				" out of memory");
			return_code =0;
		}	
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_rig_node_group."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
}/* set_unemap_package_rig_node_group */

struct GROUP(FE_node) *get_unemap_package_map_node_group(
	struct Unemap_package *package,int map_number)
/*******************************************************************************
LAST MODIFIED : 9 September 1999

DESCRIPTION :
gets the node_group for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/
{
	struct GROUP(FE_node) *map_node_group;

	ENTER(get_unemap_package_map_node_group);
	if((package)&&(map_number>-1)&&(map_number<=package->number_of_maps))	
	{	
		if(package->number_of_maps==map_number)
		{
			/* No map_info, map_node_group=NULL */	
			map_node_group=(struct GROUP(FE_node) *)NULL;
		}
		else
		{
			map_node_group=package->maps_info[map_number]->node_group;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_map_node_group."
			" invalid arguments");
		map_node_group=(struct GROUP(FE_node) *)NULL;
	}
	LEAVE;
	return (map_node_group);	
}/* get_unemap_package_map_node_group */

struct GROUP(FE_element) *get_unemap_package_map_element_group(
	struct Unemap_package *package,int map_number)
/*******************************************************************************
LAST MODIFIED : September 10 1999

DESCRIPTION :
gets the element_group for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3...
==============================================================================*/
{
	struct GROUP(FE_element) *map_element_group;

	ENTER(get_unemap_package_map_element_group);
	if((package)&&(map_number>-1)&&(map_number<=package->number_of_maps))
	{			
		if(package->number_of_maps==map_number)
		{
			/* No map_info, map_element_group=NULL */	
			map_element_group=(struct GROUP(FE_element) *)NULL;
		}
		else
		{
			map_element_group=package->maps_info[map_number]->element_group;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_map_element_group."
			" invalid arguments");
		map_element_group=(struct GROUP(FE_element) *)NULL;
	}
	LEAVE;
	return (map_element_group);	
}/* get_unemap_package_map_element_group */

int set_unemap_package_map_element_group(struct Unemap_package *package,
	struct GROUP(FE_element) *map_element_group,int map_number)
/*******************************************************************************
LAST MODIFIED : September 10 1999

DESCRIPTION :
Sets the element_group for map_info <map_number> in <package>.
Set (and get) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3...
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_map_element_group);
	if(package&&map_element_group&&(map_number>-1)&&(map_number<package->number_of_maps))
	{		
		return_code =1;
		REACCESS(GROUP(FE_element))
			(&(package->maps_info[map_number]->element_group),map_element_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_map_element_group."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
}/* set_unemap_package_map_element_group */

#if defined(OLD_CODE)
int free_unemap_package_rig_computed_fields(struct Unemap_package *unemap_package)
/*******************************************************************************
LAST MODIFIED : August 20 1999

DESCRIPTION :
Frees the computed fields associated with the rid fields from the computed 
field manager
==============================================================================*/
{
	int count,return_code;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(free_unemap_package_rig_computed_fields)
	if(unemap_package)
	{
		return_code=1;
		computed_field_manager=unemap_package->computed_field_manager;	
		if(unemap_package->electrode_position_fields)
		{
			for(count=0;count<unemap_package->number_of_electrode_position_fields;count++)
			{
				remove_computed_field_from_manager_given_FE_field(
					computed_field_manager,unemap_package->electrode_position_fields[count]);
			}
		}		
		if(unemap_package->map_electrode_position_fields)
		{
			for(count=0;count<unemap_package->number_of_map_electrode_position_fields;count++)
			{
				remove_computed_field_from_manager_given_FE_field(
					computed_field_manager,unemap_package->map_electrode_position_fields[count]);
			}
		}		

		if(unemap_package->device_name_field)
		{
			remove_computed_field_from_manager_given_FE_field(
				computed_field_manager,unemap_package->device_name_field);
		}
		if(unemap_package->device_type_field)
		{
			remove_computed_field_from_manager_given_FE_field(
				computed_field_manager,unemap_package->device_type_field);
		}
		if(unemap_package->channel_number_field)
		{
			remove_computed_field_from_manager_given_FE_field(
				computed_field_manager,unemap_package->channel_number_field);
		}	
		if(unemap_package->signal_field)
		{
			remove_computed_field_from_manager_given_FE_field(
				computed_field_manager,unemap_package->signal_field);
		}
		if(unemap_package->signal_minimum_field)
		{
			remove_computed_field_from_manager_given_FE_field(
				computed_field_manager,unemap_package->signal_minimum_field);
		}
		if(unemap_package->signal_maximum_field)
		{
			remove_computed_field_from_manager_given_FE_field(
				computed_field_manager,unemap_package->signal_maximum_field);
		}	
		if(unemap_package->signal_status_field)
		{
			remove_computed_field_from_manager_given_FE_field(
				computed_field_manager,unemap_package->signal_status_field);
		}
		if(unemap_package->channel_gain_field)
		{
			remove_computed_field_from_manager_given_FE_field(
				computed_field_manager,unemap_package->channel_gain_field);
		}
		if(unemap_package->channel_offset_field)
		{
			remove_computed_field_from_manager_given_FE_field(
				computed_field_manager,unemap_package->channel_offset_field);
		}	
	}
	else
	{	
		display_message(ERROR_MESSAGE,"free_unemap_package_rig_computed_fields."
			" invalid arguments");
		return_code=0;
	}
	LEAVE;
	return(return_code);		
}/* free_unemap_package_rig_computed_fields*/
#endif /* defined(OLD_CODE) */

int free_unemap_package_rig_fields(struct Unemap_package *unemap_package)
/*******************************************************************************
LAST MODIFIED : 17 May 2000

DESCRIPTION :
Frees the <unemap_package> rig's computed and fe fields
==============================================================================*/
{
	int count,return_code;
	struct MANAGER(Computed_field) *computed_field_manager=
		(struct MANAGER(Computed_field) *)NULL;
	struct MANAGER(FE_field) *fe_field_manager=
		(struct MANAGER(FE_field) *)NULL;
	struct FE_field *temp_field=(struct FE_field *)NULL;

	ENTER(free_unemap_package_rig_fields)
	if(unemap_package)
	{
		return_code=1;
		computed_field_manager=unemap_package->computed_field_manager;
		fe_field_manager=unemap_package->fe_field_manager;
	
		if(unemap_package->electrode_position_fields)
		{
			for(count=0;count<unemap_package->number_of_electrode_position_fields;count++)
			{
				temp_field=unemap_package->electrode_position_fields[count];
				DEACCESS(FE_field)(&temp_field);
				destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
					unemap_package->electrode_position_fields[count]);
				unemap_package->electrode_position_fields[count]=(struct FE_field *)NULL;
			}	
			unemap_package->number_of_electrode_position_fields=0;
		}		
		if(unemap_package->map_electrode_position_fields)
		{
			for(count=0;count<unemap_package->number_of_map_electrode_position_fields;count++)
			{
				temp_field=unemap_package->map_electrode_position_fields[count];
				DEACCESS(FE_field)(&temp_field);
				destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
					unemap_package->map_electrode_position_fields[count]);
				unemap_package->map_electrode_position_fields[count]=(struct FE_field *)NULL;
			}	
			unemap_package->number_of_map_electrode_position_fields=0;
		}		
		if(unemap_package->device_name_field)
		{
			temp_field=unemap_package->device_name_field;
			DEACCESS(FE_field)(&temp_field);
			destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->device_name_field);
			unemap_package->device_name_field=(struct FE_field *)NULL;
		}
		if(unemap_package->device_type_field)
		{
			temp_field=unemap_package->device_type_field;
			DEACCESS(FE_field)(&temp_field);
			destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->device_type_field);
			unemap_package->device_type_field=(struct FE_field *)NULL;
		}
		if(unemap_package->channel_number_field)
		{
			temp_field=unemap_package->channel_number_field;
			DEACCESS(FE_field)(&temp_field);
			destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->channel_number_field);
			unemap_package->channel_number_field=(struct FE_field *)NULL;
		}	
		if(unemap_package->signal_field)
		{
			temp_field=unemap_package->signal_field;
			DEACCESS(FE_field)(&temp_field);
			destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->signal_field);
			unemap_package->signal_field=(struct FE_field *)NULL;
		}
		if(unemap_package->signal_minimum_field)
		{
			temp_field=unemap_package->signal_minimum_field;
			DEACCESS(FE_field)(&temp_field);
			destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->signal_minimum_field);
			unemap_package->signal_minimum_field=(struct FE_field *)NULL;
		}
		if(unemap_package->signal_maximum_field)
		{
			temp_field=unemap_package->signal_maximum_field;
			DEACCESS(FE_field)(&temp_field);
			destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->signal_maximum_field);
			unemap_package->signal_maximum_field=(struct FE_field *)NULL;
		}	
		if(unemap_package->signal_status_field)
		{
			temp_field=unemap_package->signal_status_field;
			DEACCESS(FE_field)(&temp_field);
			destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->signal_status_field);
			unemap_package->signal_status_field=(struct FE_field *)NULL;
		}
		if(unemap_package->channel_gain_field)
		{
			temp_field=unemap_package->channel_gain_field;
			DEACCESS(FE_field)(&temp_field);
			destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->channel_gain_field);
			unemap_package->channel_gain_field=(struct FE_field *)NULL;
		}
		if(unemap_package->channel_offset_field)
		{
			temp_field=unemap_package->channel_offset_field;
			DEACCESS(FE_field)(&temp_field);
			destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->channel_offset_field);
			unemap_package->channel_offset_field=(struct FE_field *)NULL;
		}
	}
	else
	{	
		display_message(ERROR_MESSAGE,"free_unemap_package_rig_fields."
			" invalid arguments");
		return_code=0;
	}
	LEAVE;
	return(return_code);		
}/* free_unemap_package_rig_fields*/

#if defined(OLD_CODE)
int free_unemap_package_rig_fields(struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : August 20 1999

DESCRIPTION :
Frees the fields stored in the unemap_package that are used by the rid_node_group
Do this last, as it attempts to remove the fields from the managers.
==============================================================================*/
{
	int count,return_code;
	struct FE_field *temp_field;	
	struct MANAGER(FE_field) *fe_field_manager;

	ENTER(free_unemap_package_rig_fields);
	if(package)
	{
		return_code=1;
		fe_field_manager=package->fe_field_manager;
		if(package->electrode_position_fields)
		{
			for(count=0;count<package->number_of_electrode_position_fields;count++)
			{
				temp_field=package->electrode_position_fields[count]; 
				DEACCESS(FE_field)(&temp_field);
				if (FE_field_can_be_destroyed(package->electrode_position_fields[count]))
				{
					if(REMOVE_OBJECT_FROM_MANAGER(FE_field)
						(package->electrode_position_fields[count],fe_field_manager))
					{
						package->electrode_position_fields[count]	=(struct FE_field *)NULL;
					}
					else
					{
						display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
							" Couldn't remove electrode_position_field from manager");
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
						" Couldn't destroy electrode_position_field");
				}
			}
			package->number_of_electrode_position_fields=0;
		}		
		if(package->map_electrode_position_fields)
		{
			for(count=0;count<package->number_of_map_electrode_position_fields;count++)
			{
				temp_field=package->map_electrode_position_fields[count]; 
				DEACCESS(FE_field)(&temp_field);
				if (FE_field_can_be_destroyed(package->map_electrode_position_fields[count]))
				{
					if(REMOVE_OBJECT_FROM_MANAGER(FE_field)
						(package->map_electrode_position_fields[count],fe_field_manager))
					{
						package->map_electrode_position_fields[count]	=(struct FE_field *)NULL;
					}
					else
					{
						display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
							" Couldn't remove map_electrode_position_field from manager");
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
						" Couldn't destroy map_electrode_position_field");
				}
			}
			package->number_of_map_electrode_position_fields=0;
		}

		temp_field=package->device_name_field; 
		DEACCESS(FE_field)(&temp_field);
		if(package->device_name_field)
		{	
			if (FE_field_can_be_destroyed(package->device_name_field))
			{
				if(REMOVE_OBJECT_FROM_MANAGER(FE_field)(package->device_name_field,
					fe_field_manager))
				{
					package->device_name_field=(struct FE_field *)NULL;
				}
				else
				{
					display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
						" Couldn't remove device_name_field from manager");
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
					" Couldn't destroy device_name_field");
			}
		}
		temp_field=package->device_type_field;
		DEACCESS(FE_field)(&temp_field);
		if(package->device_type_field)
		{
			if (FE_field_can_be_destroyed(package->device_type_field))
			{
				if(REMOVE_OBJECT_FROM_MANAGER(FE_field)(package->device_type_field,
					fe_field_manager))
				{
					package->device_type_field=(struct FE_field *)NULL;
				}
				else
				{
					display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
						" Couldn't remove device_type_field from manager");
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
					" Couldn't destroy device_type_field");
			}	
		}
		temp_field=package->channel_number_field;
		DEACCESS(FE_field)(&temp_field);
		if(package->channel_number_field)
		{
			if (FE_field_can_be_destroyed(package->channel_number_field))
			{
				if(REMOVE_OBJECT_FROM_MANAGER(FE_field)(package->channel_number_field,
					fe_field_manager))
				{
					package->channel_number_field=(struct FE_field *)NULL;
				}	
				else
				{
					display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
						" Couldn't remove channel_number_field from manager");
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
					" Couldn't destroy channel_number_field");
			}	
		}
		temp_field=package->signal_field;
		DEACCESS(FE_field)(&temp_field);
		if(package->signal_field)
		{	
			if (FE_field_can_be_destroyed(package->signal_field))
			{
				if(REMOVE_OBJECT_FROM_MANAGER(FE_field)(package->signal_field,
					fe_field_manager))
				{
					package->signal_field=(struct FE_field *)NULL;
				}	
				else
				{
					display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
						" Couldn't remove signal_field from manager");
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
					" Couldn't destroy signal_field");
			}
		}
		temp_field=package->signal_minimum_field;
		DEACCESS(FE_field)(&temp_field);
		if(package->signal_minimum_field)
		{	
			if (FE_field_can_be_destroyed(package->signal_minimum_field))
			{
				if(REMOVE_OBJECT_FROM_MANAGER(FE_field)(package->signal_minimum_field,
					fe_field_manager))
				{
					package->signal_minimum_field=(struct FE_field *)NULL;
				}	
				else
				{
					display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
						" Couldn't remove signal_minimum_field from manager");
				}
			}	
			else
			{
				display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
					" Couldn't destroy signal_minimum_field");
			}
		}
		temp_field=package->signal_maximum_field;
		DEACCESS(FE_field)(&temp_field);
		if(package->signal_maximum_field)
		{	
			if (FE_field_can_be_destroyed(package->signal_maximum_field))
			{
				if(REMOVE_OBJECT_FROM_MANAGER(FE_field)(package->signal_maximum_field,
					fe_field_manager))
				{
					package->signal_maximum_field=(struct FE_field *)NULL;
				}	
				else
				{
					display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
						" Couldn't remove signal_maximum_field from manager");
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
					" Couldn't destroy signal_maximum_field");
			}
		}
		temp_field=package->signal_status_field;
		DEACCESS(FE_field)(&temp_field);
		if(package->signal_status_field)
		{	
			if (FE_field_can_be_destroyed(package->signal_status_field))
			{
				if(REMOVE_OBJECT_FROM_MANAGER(FE_field)(package->signal_status_field,
					fe_field_manager))
				{
					package->signal_status_field=(struct FE_field *)NULL;
				}	
				else
				{
					display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
						" Couldn't remove signal_status_field from manager");
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
					" Couldn't destroy signal_status_field");
			}
		}
		temp_field=package->channel_gain_field;
		DEACCESS(FE_field)(&temp_field);
		if(package->channel_gain_field)
		{	
			if (FE_field_can_be_destroyed(package->channel_gain_field))
			{
				if(REMOVE_OBJECT_FROM_MANAGER(FE_field)(package->channel_gain_field,
					fe_field_manager))
				{
					package->channel_gain_field=(struct FE_field *)NULL;
				}	
				else
				{
					display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
						" Couldn't remove channel_gain_field from manager");
				}
			}	
			else
			{
				display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
					" Couldn't channel_gain_destroy field");
			}	
		}
		temp_field=package->channel_offset_field;
		DEACCESS(FE_field)(&temp_field);
		if(package->channel_offset_field)
		{	
			if (FE_field_can_be_destroyed(package->channel_offset_field))
			{
				if(REMOVE_OBJECT_FROM_MANAGER(FE_field)(package->channel_offset_field,
					fe_field_manager))
				{
					package->channel_offset_field=(struct FE_field *)NULL;
				}	
				else
				{
					display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
						" Couldn't remove channel_offset_field from manager");
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,"free_unemap_package_rig_fields."
					" Couldn't destroy channel_offset_field");
			}	
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"free_unemap_package_rig_fields."
			" invalid arguments");
		return_code =0;	
	}		
	LEAVE;
	return (return_code);
}/* free_unemap_package_rig_fields */
#endif /* defined(OLD_CODE) */

int free_unemap_package_time_computed_fields(struct Unemap_package *unemap_package)
/*******************************************************************************
LAST MODIFIED : 4 May 2000

DESCRIPTION :
Frees the time related computed fields (used by the map electrode glyphs) 
stored in the unemap package. Also frees any associated fe_fields
==============================================================================*/
{
	int return_code;
	struct Computed_field *computed_field,*temp_field;
	struct FE_field *fe_field;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct MANAGER(FE_field) *fe_field_manager;

	ENTER(free_unemap_package_time_computed_fields);
	if(unemap_package)
	{
		computed_field=(struct Computed_field *)NULL;
		temp_field=(struct Computed_field *)NULL;
		temp_field=(struct Computed_field *)NULL;
		fe_field=(struct FE_field *)NULL;
		computed_field_manager=(struct MANAGER(Computed_field) *)NULL;
		fe_field_manager=(struct MANAGER(FE_field) *)NULL;
		return_code=1;
		computed_field_manager=get_unemap_package_Computed_field_manager(unemap_package);
		computed_field=get_unemap_package_signal_value_at_time_field(unemap_package);
		temp_field=computed_field;
		DEACCESS(Computed_field)(&temp_field);
		if(computed_field)
		{	
			if (Computed_field_can_be_destroyed
				(computed_field))
			{
				/* also want to destroy any wrapped FE_field */
				fe_field=(struct FE_field *)NULL;
				switch (Computed_field_get_type(computed_field))
				{
					case COMPUTED_FIELD_FINITE_ELEMENT:
					{
						Computed_field_get_type_finite_element(computed_field,
							&fe_field);
					}
				}
				if(REMOVE_OBJECT_FROM_MANAGER(Computed_field)
					(computed_field,computed_field_manager))
				{
					computed_field=(struct Computed_field *)NULL;
					set_unemap_package_signal_value_at_time_field
						(unemap_package,(struct Computed_field *)NULL);	
					if (fe_field)
					{
						return_code=REMOVE_OBJECT_FROM_MANAGER(FE_field)(
							fe_field,fe_field_manager);
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"free_unemap_package_time_computed_fields"
						" Couldn't remove signal_value_at_time_field from manager");
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,"free_unemap_package_time_computed_fields"
					"Couldn't destroy signal_value_at_time_field");
			}
		}/* if(computed_field) */

		computed_field=get_unemap_package_time_field(unemap_package);
		temp_field=computed_field;
		DEACCESS(Computed_field)(&temp_field);
		if(computed_field)
		{	
			if (Computed_field_can_be_destroyed
				(computed_field))
			{
				fe_field=(struct FE_field *)NULL;
				switch (Computed_field_get_type(computed_field))
				{
					case COMPUTED_FIELD_FINITE_ELEMENT:
					{
						Computed_field_get_type_finite_element(computed_field,
							&fe_field);
					}
				}
				if(REMOVE_OBJECT_FROM_MANAGER(Computed_field)
					(computed_field,computed_field_manager))
				{
					computed_field=(struct Computed_field *)NULL;
					set_unemap_package_time_field
						(unemap_package,(struct Computed_field *)NULL);
					if (fe_field)
					{
						return_code=REMOVE_OBJECT_FROM_MANAGER(FE_field)(
							fe_field,fe_field_manager);
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"free_unemap_package_time_computed_fields"
						" Couldn't remove time_field from manager");
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,"free_unemap_package_time_computed_fields. "
					"Couldn't destroy time_field ");
			}
		}/* if(computed_field) */
	}/* if(unemap_package) */
	else
	{
		display_message(WARNING_MESSAGE,"free_unemap_package_time_computed_fields. "
				"Invalid arguments ");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/*free_unemap_package_time_computed_fields */

int free_unemap_package_rig_node_group_glyphs(struct Unemap_package *package,
	int rig_node_group_number)
/*******************************************************************************
LAST MODIFIED : 22 October 1999

DESCRIPTION :
Frees up any glyphs used by the nodes in the rig_node_group
==============================================================================*/
{
	char *group_name;
	int return_code;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *settings;
	struct Scene *scene;
	struct GROUP(FE_element) *rig_element_group;
	struct GROUP(FE_node) *rig_node_group;
	struct MANAGER(GROUP(FE_element))	*element_group_manager;

	ENTER(free_unemap_package_rig_node_group_glyphs);	
	gt_element_group=(struct GT_element_group *)NULL;
	settings=(struct GT_element_settings *)NULL;
	scene=(struct Scene *)NULL;
	rig_element_group=(struct GROUP(FE_element) *)NULL;
	rig_node_group=(struct GROUP(FE_node) *)NULL;
	element_group_manager=(struct MANAGER(GROUP(FE_element)) *)NULL;
	if (package)
	{
		if((scene=get_unemap_package_scene(package))&&
			(rig_node_group=get_unemap_package_rig_node_group(package,
			rig_node_group_number))&&(element_group_manager=
			get_unemap_package_element_group_manager(package)))
		{
			GET_NAME(GROUP(FE_node))(rig_node_group,&group_name);	 
			rig_element_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)
				(group_name,element_group_manager);
			if (rig_element_group&&(gt_element_group=
				Scene_get_graphical_element_group(scene,rig_element_group)))
			{
				return_code=1;
				while (return_code&&(settings=first_settings_in_GT_element_group_that(
					gt_element_group,GT_element_settings_type_matches,
					(void *)GT_ELEMENT_SETTINGS_NODE_POINTS)))
				{
					if(!(return_code=GT_element_group_remove_settings(gt_element_group,settings)))
					{
						display_message(ERROR_MESSAGE,
							"free_unemap_package_rig_node_group_glyphs. couldn't remove settings");
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"free_unemap_package_rig_node_group_glyphs. rig_element_group/gt_element_group"
						"not found");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"free_unemap_package_rig_node_group_glyphs.  Invalid argument(s)");
		return_code=0;
	}
	DEALLOCATE(group_name);	
	LEAVE;

	return (return_code);
} /* free_unemap_package_rig_node_group_glyphs */

int free_unemap_package_rig_node_groups(struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : August 17 1999

DESCRIPTION :
Frees the rig node groups and associated element and data groups stored in 
the unemap_package
==============================================================================*/
{
	int count,return_code;	
	struct GROUP(FE_node) *rig_node_group;

	ENTER(free_unemap_package_rig_node_groups);
	if(package)
	{
		for(count=0;count<package->number_of_rig_node_groups;count++)
		{

			free_unemap_package_rig_node_group_glyphs(package,count);

			rig_node_group=package->rig_node_groups[count];

			return_code=free_node_and_element_and_data_groups(
				rig_node_group,package->element_manager,
				package->element_group_manager,package->node_manager,
				package->data_group_manager,package->node_group_manager);				
		}	
		DEALLOCATE(package->rig_node_groups);
		package->number_of_rig_node_groups=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"free_unemap_package_rig_node_groups."
			" invalid arguments");
		return_code =0;	
	}		
	LEAVE;
	return (return_code);
}/* free_unemap_package_rig_node_groups */

int free_unemap_package_rig_info(struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : August 27 1999

DESCRIPTION :
Frees the fields, nodes, elements stored in unemap_package that are 
associated with the rig
==============================================================================*/		 
{
	int return_code;

	if(package)
	{	
		return_code=(return_code=free_unemap_package_rig_node_groups(package)&&
		free_unemap_package_time_computed_fields(package)&&
		free_unemap_package_rig_fields(package));
	}
	else
	{	
		display_message(ERROR_MESSAGE,"free_unemap_package_rig_info."
			" invalid arguments");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* free_unemap_package_rig_info.*/

struct MANAGER(Graphics_window) *get_unemap_package_Graphics_window_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(Graphics_window) *graphics_window_manager;

	ENTER(get_unemap_package_Graphics_window_manager);
	if(package)
	{
		graphics_window_manager=package->graphics_window_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_Graphics_window_manager."
			" invalid arguments");
		graphics_window_manager = (struct MANAGER(Graphics_window) *)NULL;
	}
	LEAVE;
	return(graphics_window_manager);
}/* get_unemap_package_Graphics_window_manager */

struct MANAGER(Light) *get_unemap_package_Light_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(Light) *light_manager;

	ENTER(get_unemap_package_Light_manager);
	if(package)
	{
		light_manager=package->light_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_Light_manager."
			" invalid arguments");
		light_manager = (struct MANAGER(Light) *)NULL;
	}
	LEAVE;
	return(light_manager);
}/* get_unemap_package_Light_manager */

struct MANAGER(Texture) *get_unemap_package_Texture_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(Texture) *texture_manager;

	ENTER(get_unemap_package_Texture_manager);
	if(package)
	{
		texture_manager=package->texture_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_Texture_manager."
			" invalid arguments");
		texture_manager = (struct MANAGER(Texture) *)NULL;
	}
	LEAVE;
	return(texture_manager);
}/* get_unemap_package_Texture_manager */

struct MANAGER(Scene) *get_unemap_package_Scene_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(Scene) *scene_manager;

	ENTER(get_unemap_package_Scene_manager);
	if(package)
	{
		scene_manager=package->scene_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_Scene_manager."
			" invalid arguments");
		scene_manager = (struct MANAGER(Scene) *)NULL;
	}
	LEAVE;
	return(scene_manager);
}/* get_unemap_package_Scene_manager */

struct MANAGER(Light_model) *get_unemap_package_Light_model_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(Light_model) *light_model_manager;

	ENTER(get_unemap_package_Light_model_manager);
	if(package)
	{
		light_model_manager=package->light_model_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_Light_model_manager."
			" invalid arguments");
		light_model_manager = (struct MANAGER(Light_model) *)NULL;
	}
	LEAVE;
	return(light_model_manager);
}/* get_unemap_package_Light_model_manager */

struct MANAGER(Graphical_material) *get_unemap_package_Graphical_material_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(Graphical_material) *graphical_material_manager;

	ENTER(get_unemap_package_Graphical_material_manager);
	if(package)
	{
		graphical_material_manager=package->graphical_material_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_Graphical_material_manager."
			" invalid arguments");
		graphical_material_manager = (struct MANAGER(Graphical_material) *)NULL;
	}
	LEAVE;
	return(graphical_material_manager);
}/* get_unemap_package_Graphical_material_manager */

struct Graphics_window *get_unemap_package_window(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED :  September 2 1999

DESCRIPTION :
gets the Graphics_window of the unemap package.
==============================================================================*/
{
	struct Graphics_window *window;
	ENTER(get_unemap_package_window);
	if(package)
	{
		window=package->window;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_window."
				" invalid arguments");
		window = (struct Graphics_window *)NULL;
	}
	LEAVE;
	return (window);
} /* get_unemap_package_window */

int set_unemap_package_window(struct Unemap_package *package,
	struct Graphics_window *window)
/*******************************************************************************
LAST MODIFIED : September 2 1999

DESCRIPTION :
Sets the Graphics_window of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_window);
	if(package)
	{
		return_code =1;	
		REACCESS(Graphics_window)(&(package->window),window);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_window."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_window */

struct Colour *get_unemap_package_background_colour(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED :  September 2 1999

DESCRIPTION :
gets the Colour of the unemap package.
==============================================================================*/
{
	struct Colour *background_colour;
	ENTER(get_unemap_package_background_colour);
	if(package)
	{
		background_colour=package->background_colour;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_background_colour."
				" invalid arguments");
		background_colour = (struct Colour *)NULL;
	}
	LEAVE;
	return (background_colour);
} /* get_unemap_package_background_colour */

int set_unemap_package_background_colour(struct Unemap_package *package,
	struct Colour *background_colour)
/*******************************************************************************
LAST MODIFIED : September 2 1999

DESCRIPTION :
Sets the Colour of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_background_colour);
	if(package)
	{
		return_code =1;
		package->background_colour->red=background_colour->red;
		package->background_colour->green=background_colour->green;
		package->background_colour->blue=background_colour->blue;
		
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_background_colour."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_background_colour */

struct Light *get_unemap_package_light(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED :  September 2 1999

DESCRIPTION :
gets the Light of the unemap package.
==============================================================================*/
{
	struct Light *light;
	ENTER(get_unemap_package_light);
	if(package)
	{
		light=package->light;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_light."
				" invalid arguments");
		light = (struct Light *)NULL;
	}
	LEAVE;
	return (light);
} /* get_unemap_package_light */

int set_unemap_package_light(struct Unemap_package *package,
	struct Light *light)
/*******************************************************************************
LAST MODIFIED : September 2 1999

DESCRIPTION :
Sets the Light of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_light);
	if(package)
	{
		return_code =1;	
		REACCESS(Light)(&(package->light),light);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_light."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_light */

struct Light_model *get_unemap_package_light_model(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED :  September 2 1999

DESCRIPTION :
gets the Light_model of the unemap package.
==============================================================================*/
{
	struct Light_model *light_model;
	ENTER(get_unemap_package_light_model);
	if(package)
	{
		light_model=package->light_model;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_light_model."
				" invalid arguments");
		light_model = (struct Light_model *)NULL;
	}
	LEAVE;
	return (light_model);
} /* get_unemap_package_light_model */

int set_unemap_package_light_model(struct Unemap_package *package,
	struct Light_model *light_model)
/*******************************************************************************
LAST MODIFIED : September 2 1999

DESCRIPTION :
Sets the Light_model of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_light_model);
	if(package)
	{
		return_code =1;	
		REACCESS(Light_model)(&(package->light_model),light_model);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_light_model."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_light_model */

struct Scene *get_unemap_package_scene(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED :  September 2 1999

DESCRIPTION :
gets the Scene of the unemap package.
==============================================================================*/
{
	struct Scene *scene;
	ENTER(get_unemap_package_scene);
	if(package)
	{
		scene=package->scene;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_scene."
				" invalid arguments");
		scene = (struct Scene *)NULL;
	}
	LEAVE;
	return (scene);
} /* get_unemap_package_scene */

int set_unemap_package_scene(struct Unemap_package *package,
	struct Scene *scene)
/*******************************************************************************
LAST MODIFIED : September 2 1999

DESCRIPTION :
Sets the Scene of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_scene);
	if(package)
	{
		return_code =1;	
		REACCESS(Scene)(&(package->scene),scene);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_scene."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_scene */

struct User_interface *get_unemap_package_user_interface(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED :  September 2 1999

DESCRIPTION :
gets the User_interface of the unemap package.
==============================================================================*/
{
	struct User_interface *user_interface;
	ENTER(get_unemap_package_user_interface);
	if(package)
	{
		user_interface=package->user_interface;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_user_interface."
				" invalid arguments");
		user_interface = (struct User_interface *)NULL;
	}
	LEAVE;
	return (user_interface);
} /* get_unemap_package_user_interface */

int set_unemap_package_user_interface(struct Unemap_package *package,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : September 2 1999

DESCRIPTION :
Sets the User_interface of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_user_interface);
	if(package)
	{
		return_code =1;	
		/* don't ACCESS as there's only ever one user_interface */
		package->user_interface=user_interface;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_user_interface."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_user_interface */

struct Graphical_material *get_unemap_package_graphical_material(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED :  September 3 1999

DESCRIPTION :
gets the Graphical_material of the unemap package.
==============================================================================*/
{
	struct Graphical_material *graphical_material;
	ENTER(get_unemap_package_graphical_material);
	if(package)
	{
		graphical_material=package->graphical_material;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_graphical_material."
				" invalid arguments");
		graphical_material = (struct Graphical_material *)NULL;
	}
	LEAVE;
	return (graphical_material);
} /* get_unemap_package_graphical_material */

int set_unemap_package_graphical_material(struct Unemap_package *package,
	struct Graphical_material *graphical_material)
/*******************************************************************************
LAST MODIFIED : September 3 1999

DESCRIPTION :
Sets the Graphical_material of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_graphical_material);
	if(package)
	{
		return_code =1;		
		REACCESS(Graphical_material)(&(package->graphical_material),graphical_material);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_graphical_material."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_graphical_material */

struct Computed_field_package *get_unemap_package_computed_field_package(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED :  September 7 1999

DESCRIPTION :
gets the Computed_field_package of the unemap package.
==============================================================================*/
{
	struct Computed_field_package *computed_field_package;
	ENTER(get_unemap_package_computed_field_package);
	if(package)
	{
		computed_field_package=package->computed_field_package;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_computed_field_package."
				" invalid arguments");
		computed_field_package = (struct Computed_field_package *)NULL;
	}
	LEAVE;
	return (computed_field_package);
} /* get_unemap_package_computed_field_package */

int set_unemap_package_computed_field_package(struct Unemap_package *package,
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : September 7 1999

DESCRIPTION :
Sets the Computed_field_package of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_computed_field_package);
	if(package)
	{
		return_code =1;
		/* don't ACCESS as only one*/
		package->computed_field_package=computed_field_package;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_computed_field_package."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_computed_field_package */

struct Time_keeper *get_unemap_package_time_keeper(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED :  September 3 1999

DESCRIPTION :
gets the Time_keeper of the unemap package.
==============================================================================*/
{
	struct Time_keeper *time_keeper;
	ENTER(get_unemap_package_time_keeper);
	if(package)
	{
		time_keeper=package->time_keeper;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_time_keeper."
				" invalid arguments");
		time_keeper = (struct Time_keeper *)NULL;
	}
	LEAVE;
	return (time_keeper);
} /* get_unemap_package_time_keeper */

int set_unemap_package_time_keeper(struct Unemap_package *package,
	struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : September 3 1999

DESCRIPTION :
Sets the Time_keeper of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_time_keeper);
	if(package)
	{
		return_code =1;			
		REACCESS(Time_keeper)(&(package->time_keeper),time_keeper);		
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_time_keeper."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_time_keeper */

struct LIST(GT_object) *get_unemap_package_glyph_list(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 21 October 1999

DESCRIPTION :
gets the glyph_list of the unemap package.
==============================================================================*/
{
	struct LIST(GT_object) *glyph_list;
	ENTER(get_unemap_package_glyph_list);
	if(package)
	{
		glyph_list=package->glyph_list;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_glyph_list."
				" invalid arguments");
		glyph_list = (struct LIST(GT_object) *)NULL;
	}
	LEAVE;
	return (glyph_list);
} /* get_unemap_package_glyph_list */

int unemap_package_make_map_scene(struct Unemap_package *package,
	struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 28 April 2000

DESCRIPTION :
Creates the unemap_package scene, if isn't already present.
==============================================================================*/
{
	int return_code;
	struct Scene *map_scene;

	ENTER(unemap_package_make_map_scene);
	map_scene=(struct Scene *)NULL;
	if(package)
	{
		/* create the map scene, if haven't already got one */	
		if(!(package->scene))
		{
			if (map_scene=CREATE(Scene)("map"))
			{
				Scene_enable_graphics(map_scene,package->glyph_list,
					package->graphical_material_manager,
					package->graphical_material,package->light_manager,
					package->spectrum_manager,spectrum,
					package->texture_manager);

				Scene_set_graphical_element_mode(map_scene,GRAPHICAL_ELEMENT_EMPTY,
					package->computed_field_package,package->element_manager,
					package->element_group_manager,package->fe_field_manager,
					package->node_manager,package->node_group_manager,
					package->data_manager,package->data_group_manager,
					package->element_point_ranges_selection,
					package->element_selection,package->node_selection,
					package->data_selection,package->user_interface);
				
				/*???RC.  May want to use functions to modify default_scene here */
				/* eg. to add model lights, etc. */
				/* ACCESS so can never be destroyed */
				/*???RC.  Should be able to change: eg. gfx set default scene NAME */

				ACCESS(Scene)(map_scene);
				if (!ADD_OBJECT_TO_MANAGER(Scene)(map_scene,
					package->scene_manager))
				{
					DEACCESS(Scene)(&(map_scene));
				}
			}			
			Scene_enable_time_behaviour(map_scene,package->time_keeper);
			set_unemap_package_scene(package,map_scene);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"unemap_package_make_map_spectrum_and_scene."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return(return_code);
}/* unemap_package_make_map_scene */

int free_unemap_package_maps(struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : September 10 1999

DESCRIPTION :
Attempt to destroy all the package's Map_infos.
Set the package number_of_maps to 0.
==============================================================================*/		 
{
	int count,return_code;
	
	ENTER(free_unemap_package_maps);
	if(package)
	{
		return_code=1;
		for(count=0;count<package->number_of_maps;count++)
		{
			if((package->maps_info)&&(package->maps_info[count]))
			{
				DEACCESS(Map_info)(&(package->maps_info[count]));
			}
		}
		DEALLOCATE(package->maps_info);
		package->number_of_maps=0;	
		package->viewed_scene=0; 
	}
	else
	{
		display_message(ERROR_MESSAGE,"free_unemap_package_maps."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return(return_code);
} /* free_unemap_package_maps */

int free_unemap_package_map_info(struct Unemap_package *package,int map_number)
/*******************************************************************************
LAST MODIFIED : September 16 1999

DESCRIPTION :
Attempt to destroy  the package's map_number Map_infos.
DOESN'T alter  the package number_of_maps. 
==============================================================================*/		 
{
	int return_code;
	
	ENTER(free_unemap_package_map_info);
	if(package)
	{
		return_code=1;		
		if((package->maps_info)&&(package->maps_info[map_number]))
		{
			DEACCESS(Map_info)(&(package->maps_info[map_number]));		
			package->viewed_scene=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"free_unemap_package_map_info."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return(return_code);
} /* free_unemap_package_map_info */

int set_unemap_package_map_info(struct Unemap_package *package,int map_number,
	int rig_node_group_number,int number_of_map_rows,int number_of_map_columns,
	enum Region_type region_type,char *name,
	struct FE_node_order_info *node_order_info,
	struct FE_field *map_position_field,struct FE_field *map_fit_field,
	struct GROUP(FE_node) *node_group)
/*******************************************************************************
LAST MODIFIED : September 8 1999

DESCRIPTION :
Sets the <package> <map_number> Map_info.
If (map_number<package->number_of_maps), i.e. the map_info already exists, 
free the existing map_info, create a new one and set it to the 
package->maps_info[map_number]
If (map_number>package->number_of_maps) give an error, as package->maps_info[]
map_info elements must be consecutive.
Else (map_number==package->number_of_maps), increment package->number_of_maps,
increment the number of elements in package->maps_info[], crate a new
Map_info, add it as the new  package->maps_info[] element
Note: map_number is the index into package->maps_info[], so will be 0,1,2..
package->number_of_maps is the number of elements in package->maps_info[]
so will be 1,2,3...
==============================================================================*/		 
{
	int return_code;
	struct Map_info **maps_info,*map_info;

	ENTER(set_unemap_package_map_info);
	if(name&&node_order_info&&map_position_field&&map_fit_field&&(map_number>-1))
	{
		if(map_number<package->number_of_maps)			
		{
			/* Create the new Map_info*/
			map_info=CREATE(Map_info)(number_of_map_rows,number_of_map_columns,
				rig_node_group_number,region_type,name,node_order_info,
				map_position_field,map_fit_field,node_group,package->fe_field_manager,
				package->element_group_manager,package->node_manager,
				package->data_group_manager,package->node_group_manager,
				package->element_manager,package->computed_field_manager);	
			/* free the existing maps_info and replace with a new one*/				
			REACCESS(Map_info)(&(package->maps_info[map_number]),map_info);
		}
		else
		{
			/* map_number 0,1,2.. , package->number_of_maps 1,2,3..*/
			if(map_number>package->number_of_maps)
			{
				display_message(ERROR_MESSAGE,"set_unemap_package_map_info."
					"package->maps_info members must be consecutive ");
				return_code =0;				
			}
			else			
			{	
				/* create an additional maps_info */
				package->number_of_maps++;
				if(REALLOCATE(maps_info,package->maps_info,struct Map_info *,
					package->number_of_maps))
				{
					package->maps_info=maps_info;
					map_info=CREATE(Map_info)(number_of_map_rows,number_of_map_columns,
						rig_node_group_number,region_type,name,node_order_info,
						map_position_field,map_fit_field,node_group,
						package->fe_field_manager,package->element_group_manager,
						package->node_manager,package->data_group_manager,package->node_group_manager,
						package->element_manager,package->computed_field_manager);
					package->maps_info[package->number_of_maps-1]=ACCESS(Map_info)
						(map_info);
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_unemap_package_map_info."
						"Out of memory ");
					return_code =0;
				}				
			}/*	if(map_number>package->number_of_maps)*/
		}/* if(map_number<package->number_of_maps) */		
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_map_info."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return(return_code);
}/* set_unemap_package_map_info */

struct MANAGER(Spectrum) *get_unemap_package_spectrum_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : April 18 2000

DESCRIPTION :
gets the spectrum_manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(Spectrum) *spectrum_manager;
	ENTER(get_unemap_package_spectrum_manager);
	if(package)
	{
		spectrum_manager=package->spectrum_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_spectrum_manager."
				" invalid arguments");
		spectrum_manager = (struct MANAGER(Spectrum) *)NULL;
	}
	LEAVE;
	return (spectrum_manager);
} /* get_unemap_package_ spectrum_manager*/

#endif /* #if defined (UNEMAP_USE_NODES) */
