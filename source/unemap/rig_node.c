/*******************************************************************************
FILE : rig_node.c

LAST MODIFIED : 14 June 2000

DESCRIPTION :
Essentially the same functionality as rig.c, but using nodes and fields to store
the rig and signal information. rather than special structures.
==============================================================================*/
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "finite_element/finite_element.h"
#include "general/debug.h"
#include "general/geometry.h"
#include "general/myio.h"
#include "general/mystring.h"
#include "unemap/analysis.h"
#include "unemap/analysis_window.h"
#include "unemap/rig.h"
#include "unemap/rig_node.h"
#include "unemap/trace_window.h"
#include "unemap/unemap_package.h"
#include "user_interface/message.h"


#if defined (UNEMAP_USE_NODES)

/*
Module Constants
----------------
*/
const char auxiliary_str[] = "auxiliary";
const char patch_electrode_position_str[] = "patch_electrode_position";
const char sock_electrode_position_str[] = "sock_electrode_position";
const char torso_electrode_position_str[] = "torso_electrode_position";

/*
Module types
------------
*/

enum Config_node_type
/*******************************************************************************
LAST MODIFIED : 4 May 1999

DESCRIPTION :
The type of the rig.
==============================================================================*/
{
	AUXILIARY_TYPE,
	PATCH_ELECTRODE_TYPE,
	SOCK_ELECTRODE_TYPE,
	TORSO_ELECTRODE_TYPE
}; /* enum Config_node_type */

struct Add_map_electrode_position_info
/*******************************************************************************
LAST MODIFIED : 3 April 2000

DESCRIPTION :
Used by  rig_node_group_map_add_electrode_position_field
to store info when iterating 
==============================================================================*/
{
	struct FE_field *map_electrode_position_field;	
	struct MANAGER(FE_field) *field_manager;
	struct MANAGER(FE_node) *node_manager;
};

struct Min_max_info
/*******************************************************************************
LAST MODIFIED : 3 April 2000

DESCRIPTION :
Used by get_rig_node_group_signal_min_max_at_time to store info
about the minimum and maximum signal values at a rig node group at time
==============================================================================*/
{
	FE_value max,min,time;
	int count;
	struct FE_field_component *signal_component;
	struct FE_field *signal_status_field;
};

struct Position_min_max_info
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
used by get_rig_node_map_electrode_position_min_max
==============================================================================*/
{
	FE_value max_x,min_x,max_y,min_y,max_z,min_z;
	int count;
	struct FE_field *map_electrode_position_field;
};

struct Set_map_electrode_position_info
/*******************************************************************************
LAST MODIFIED : 3 April 2000

DESCRIPTION 	
Used by rig_node_group_set_map_electrode_position_lambda_r
to store info when iterating
==============================================================================*/
{	
	enum Region_type region_type;
	FE_value value1;
	FE_value value2;
	struct FE_field *map_electrode_position_field;
	struct MANAGER(FE_field) *field_manager;
	struct MANAGER(FE_node) *node_manager;
};

#endif /* defined (UNEMAP_USE_NODES) */

/*
Module variables
----------------
*/


/*
Module functions
----------------
*/
#if defined (UNEMAP_USE_NODES)
static struct FE_node *create_config_template_node(enum Config_node_type 
	config_node_type,FE_value focus,struct FE_field_order_info **field_order_info,
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 31 May 1999

DESCRIPTION :
Creates and returns a configuration template node for read_text_config_FE_node
and read_binary_config_FE_node.
The type of the configuration template node is determined by Config_node_type.
Cannot create a template node of type MIXED_TYPE, this doesn't make sense.
focus is relevant only for config_node_type=SOCK_ELECTRODE_TYPE, for other
types,focus is ignored. Creates, fills in and returns and returns  field_order_info,
containing the nodes fields. Therefore, field_order_info should be an unassigned 
pointer when passed to this function, and must be freed by the calling function.
The fields of the field_order_info are named using the node_type, therefore
beware of name clashes if loading multiple rigs of the same node_type.
==============================================================================*/
{		
  #define AUXILIARY_NUM_CONFIG_FIELDS 3 /*,device_name_field,*/
	/* device_type_field channel_number */
  #define PATCH_NUM_CONFIG_FIELDS 4 /* electrode_position_field,*/ 
	  /*device_name_field,device_type_field,channel_number*/
  #define SOCK_NUM_CONFIG_FIELDS 4 /* electrode_position_field,*/
	  /*device_name_field,device_type_field,channel_number */
  #define TORSO_NUM_CONFIG_FIELDS 4/* electrode_position_field,*/ 
    /*device_name_field,device_type_field,channel_number */

	char *electrode_name =(char *)NULL;
	char *rig_type_str =(char *)NULL;
	int field_number,number_of_fields,success,string_length;
	struct CM_field_information field_info;	
	struct Coordinate_system coordinate_system;	
	struct FE_field *channel_number_field=(struct FE_field *)NULL;
	struct FE_field *device_name_field=(struct FE_field *)NULL;
	struct FE_field *device_type_field=(struct FE_field *)NULL;
	struct FE_field *electrode_position_field=(struct FE_field *)NULL;
	struct FE_field_order_info *the_field_order_info = 
		(struct FE_field_order_info *)NULL;
	struct FE_node *node=(struct FE_node *)NULL;
	struct MANAGER(FE_field) *fe_field_manager=(struct MANAGER(FE_field) *)NULL;
	
	char *device_name_component_names[1]=
	{
		"name"
	};	
	char *device_type_component_names[1]=
	{
		"type"
	};
	char *electrode_component_names[3]=
	{
		"x","y","z"
	};
	char *sock_electrode_component_names[3]=
	{
		"lambda","mu","theta"
	};	
	char *channel_number_component_names[1]=
	{
		"number"
	};

	enum FE_nodal_value_type *device_name_components_nodal_value_types[1]=
	{
		{
			FE_NODAL_VALUE
		}
	};		
	enum FE_nodal_value_type *device_type_components_nodal_value_types[1]=
	{
		{
			FE_NODAL_VALUE
		}
	};

	enum FE_nodal_value_type *electrode_components_nodal_value_types[3]=
	{
		{
			FE_NODAL_VALUE
		},
		{
			FE_NODAL_VALUE
		},
		{
			FE_NODAL_VALUE
		}
	};

	enum FE_nodal_value_type *channel_number_components_nodal_value_types[1]=
	{
		{
			FE_NODAL_VALUE
		}
	};	
	int device_name_components_number_of_derivatives[1]={0},
	  device_name_components_number_of_versions[1]={1};	
	int device_type_components_number_of_derivatives[1]={0},
	  device_type_components_number_of_versions[1]={1};
	int electrode_components_number_of_derivatives[3]={0,0,0},
		  electrode_components_number_of_versions[3]={1,1,1};
	int channel_number_components_number_of_derivatives[1]={0},
	  channel_number_components_number_of_versions[1]={1};
 
	ENTER(create_config_template_node);
	if(package)
	{	
		fe_field_manager=get_unemap_package_FE_field_manager(package);	
		success = 1;			
		field_number = 0;
		electrode_name = (char *)NULL;
		/* create the node */		
		if (node=CREATE(FE_node)(0,(struct FE_node *)NULL))
		{		
			/* create the FE_field_order_info for all the fields */
			switch(config_node_type)
			{
				case SOCK_ELECTRODE_TYPE:
				{	
					the_field_order_info=
						CREATE(FE_field_order_info)(SOCK_NUM_CONFIG_FIELDS);
					number_of_fields = SOCK_NUM_CONFIG_FIELDS; 
					rig_type_str = (char *)sock_electrode_position_str;
				}break;
				case PATCH_ELECTRODE_TYPE:
				{		
					the_field_order_info=
						CREATE(FE_field_order_info)(PATCH_NUM_CONFIG_FIELDS);	
					number_of_fields = PATCH_NUM_CONFIG_FIELDS;				
					rig_type_str = (char *)patch_electrode_position_str;
				}break;
				case TORSO_ELECTRODE_TYPE:
				{	
					the_field_order_info=
						CREATE(FE_field_order_info)(TORSO_NUM_CONFIG_FIELDS);
					number_of_fields = TORSO_NUM_CONFIG_FIELDS;
					rig_type_str = (char *)torso_electrode_position_str;
				}break;
				case AUXILIARY_TYPE:
				{						
					the_field_order_info=
						CREATE(FE_field_order_info)(AUXILIARY_NUM_CONFIG_FIELDS);
					number_of_fields = AUXILIARY_NUM_CONFIG_FIELDS;
					rig_type_str = (char *)auxiliary_str;
				}break;
			}
			if(!the_field_order_info)
			{
				display_message(ERROR_MESSAGE,
					"create_config_template_node. Could not create field_order_info");
				success=0;
			}
			/* create the fields which depend on template types. */		
			if(success)
			{
				string_length = strlen(rig_type_str);				
				string_length++;
				if(ALLOCATE(electrode_name,char,string_length))
				{	
					strcpy(electrode_name,rig_type_str);				
					switch(config_node_type)
					{
						case SOCK_ELECTRODE_TYPE:
						{										
							/* set up the info needed to create the  electrode position field */
							set_CM_field_information(&field_info,CM_COORDINATE_FIELD,(int *)NULL);	
							coordinate_system.type=PROLATE_SPHEROIDAL;								
							coordinate_system.parameters.focus = focus;								
							/* create the electrode position field, add it to the node and create */
							/* the FE_field_order_info with enough fields */
							if ((electrode_position_field=get_FE_field_manager_matched_field(
								fe_field_manager,electrode_name,
								GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
								/*number_of_indexed_values*/0,&field_info,
								&coordinate_system,FE_VALUE_VALUE,
								/*number_of_components*/3,sock_electrode_component_names,
								/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE)))
							{
								set_unemap_package_electrode_position_field(package,
									electrode_position_field);
								success =define_node_field_and_field_order_info(node,
									electrode_position_field,electrode_components_number_of_derivatives
									,electrode_components_number_of_versions,&field_number,
									number_of_fields,electrode_components_nodal_value_types,
									the_field_order_info);															 				
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_config_template_node.Could not retrieve sock electrode"
									" position field");
								success=0;			
							}
						}break;			
						case TORSO_ELECTRODE_TYPE:
						{				
							/* set up the info needed to create the  electrode position field */
							set_CM_field_information(&field_info,CM_COORDINATE_FIELD,(int *)NULL);	
							coordinate_system.type=RECTANGULAR_CARTESIAN;																			
							if ((electrode_position_field=get_FE_field_manager_matched_field(
								fe_field_manager,electrode_name,
								GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
								/*number_of_indexed_values*/0,&field_info,
								&coordinate_system,FE_VALUE_VALUE,
								/*number_of_components*/3,electrode_component_names,
								/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE)))
							{
								set_unemap_package_electrode_position_field(package,
									electrode_position_field);								
								success =define_node_field_and_field_order_info(node,
									electrode_position_field,electrode_components_number_of_derivatives
									,electrode_components_number_of_versions,&field_number,
									number_of_fields,electrode_components_nodal_value_types,
									the_field_order_info);								 				
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_config_template_node. Could not torso retrieve electrode"
									" position field");
								success=0;			
							}
						}break;		
						case PATCH_ELECTRODE_TYPE:
						{
							/* set up the info needed to create the  electrode position field */
							set_CM_field_information(&field_info,CM_COORDINATE_FIELD,(int *)NULL);	
							coordinate_system.type=RECTANGULAR_CARTESIAN;
							/* create the electrode position field, add it to the node */
							/* create the electrode position field, add it to the node */
							if ((electrode_position_field=get_FE_field_manager_matched_field(
								fe_field_manager,electrode_name,
								GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
								/*number_of_indexed_values*/0,&field_info,
								&coordinate_system,FE_VALUE_VALUE,
								/*number_of_components*/2,electrode_component_names,
								/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE)))
							{
								set_unemap_package_electrode_position_field(package,
									electrode_position_field);								
								success =define_node_field_and_field_order_info(node,
									electrode_position_field,electrode_components_number_of_derivatives
									,electrode_components_number_of_versions,&field_number,
									number_of_fields,electrode_components_nodal_value_types,
									the_field_order_info);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_config_template_node. Could not retrieve patch "
									"electrode position field");
								success=0;			
							}
						}break;				
					} /* switch */
				}
				else
				{	
					display_message(ERROR_MESSAGE,
								"create_config_template_node. Could not allocate memory for "
								"electrode_name");
					success =0;
				}
			} /* if(success) */
			DEALLOCATE(electrode_name);		
			/* create fields common to all node */
			if(success)
			{							
				/* set up the info needed to create the device name field */			
				set_CM_field_information(&field_info,CM_FIELD,(int *)NULL);		
				coordinate_system.type=NOT_APPLICABLE;				
				/* create the device name field, add it to the node */			
				if (device_name_field=get_FE_field_manager_matched_field(
					fe_field_manager,"device_name",
					GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
					/*number_of_indexed_values*/0,&field_info,
					&coordinate_system,STRING_VALUE,
					/*number_of_components*/1,device_name_component_names,
					/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE))
				{ 
					set_unemap_package_device_name_field(package,device_name_field);				
					success =define_node_field_and_field_order_info(node,
						device_name_field,device_name_components_number_of_derivatives
						,device_name_components_number_of_versions,&field_number,
						number_of_fields,device_name_components_nodal_value_types,
						the_field_order_info); 
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_config_template_node. Could not retrieve device name field");
					success=0;
				}				
				/* set up the info needed to create the device type field */			
				set_CM_field_information(&field_info,CM_FIELD,(int *)NULL);		
				coordinate_system.type=NOT_APPLICABLE;				
				/* create the device type field, add it to the node */			
				if (device_type_field=get_FE_field_manager_matched_field(
					fe_field_manager,"device_type",
					GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
					/*number_of_indexed_values*/0,&field_info,
					&coordinate_system,STRING_VALUE,
					/*number_of_components*/1,device_type_component_names,
					/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE))
				{ 
					set_unemap_package_device_type_field(package,device_type_field);				
					success =define_node_field_and_field_order_info(node,
						device_type_field,device_type_components_number_of_derivatives
						,device_type_components_number_of_versions,&field_number,
						number_of_fields,device_type_components_nodal_value_types,
						the_field_order_info); 						
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_config_template_node. Could not retrieve  device type field");
					success=0;
				}					
				/* set up the info needed to create the channel number field */			
				set_CM_field_information(&field_info,CM_FIELD,(int *)NULL);		
				coordinate_system.type=NOT_APPLICABLE;				
				/* create the channel number field, add it to the node */			
				if (channel_number_field=get_FE_field_manager_matched_field(
					fe_field_manager,"channel_number",
					GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
					/*number_of_indexed_values*/0,&field_info,
					&coordinate_system,INT_VALUE,
					/*number_of_components*/1,channel_number_component_names,
					/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE))
				{	
					set_unemap_package_channel_number_field(package,channel_number_field);					
					success =define_node_field_and_field_order_info(node,
						channel_number_field,channel_number_components_number_of_derivatives
						,channel_number_components_number_of_versions,&field_number,
						number_of_fields,channel_number_components_nodal_value_types,
						the_field_order_info); 
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_config_template_node. Could not retrieve channel number field");
					success=0;
				}							
			} /* if(success) */		
		}	
		else
		{
			display_message(ERROR_MESSAGE,
				"create_config_template_node.  Could not create node");
			success=0;
		}			
		*field_order_info = the_field_order_info;
		if(!success)
		{
			DESTROY(FE_node)(&node);
			node = (struct FE_node *)NULL;			
		}		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_config_template_node. Invalid arguments");
		node = (struct FE_node *)NULL;
	}		
	LEAVE;
	return(node);
}/* create_config_template_node */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
static struct FE_node *set_config_FE_node(FILE *input_file,
	struct FE_node *template_node,struct MANAGER(FE_node) *node_manager,
	enum Config_node_type	config_node_type,int node_number,
	struct FE_field_order_info *field_order_info,char *name,
	int channel_number,FE_value xpos,FE_value ypos,FE_value zpos)
/*******************************************************************************
LAST MODIFIED : 3 September 1999

DESCRIPTION :
sets a node's values storage with the passed, name, channel_number,
xpos, ypos, zpos using the template node created in create_config_template_node,
and the field_order_info
==============================================================================*/
{
	FE_value lambda,mu,theta;	
	int component_number,field_number;
	struct Coordinate_system *coordinate_system;
	struct FE_field *field;
	struct FE_field_component component;
	struct FE_node *node;

	ENTER(set_config_FE_node);
	field_number = 0;
	if (input_file&&template_node&&node_manager&&field_order_info)
	{	 							
		if (node=CREATE(FE_node)(node_number,template_node))
		{								
			/*	set up the type dependent values	*/
			switch(config_node_type)
			{
				case SOCK_ELECTRODE_TYPE:
				{	
					/* convert to prolate */	 
					/* 1st field contains electrode coords */
					component.field=get_FE_field_order_info_field(
						field_order_info,field_number);
					field_number++;
					coordinate_system =	get_FE_field_coordinate_system(component.field);
					cartesian_to_prolate_spheroidal(xpos,ypos,zpos,
						coordinate_system->parameters.focus,&lambda,&mu,&theta,
						(float *)NULL);
					component.number = 0;					
					set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,lambda);
					component.number = 1;
					set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,mu);
					component.number = 2;
					set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,theta);
					/* 2nd field contains device name */
					field=get_FE_field_order_info_field(field_order_info,
						field_number);
					field_number++;
					component_number=0;
					set_FE_nodal_string_value(node,field,component_number,/*version*/0,
						FE_NODAL_VALUE,name);
					/* 3rd field contains device type */
					field=get_FE_field_order_info_field(field_order_info,
						field_number);
					field_number++;
					component_number=0;
					set_FE_nodal_string_value(node,field,component_number,/*version*/0,
						FE_NODAL_VALUE,"ELECTRODE");
					/* 4th field contains channel number */
					component.field=get_FE_field_order_info_field(
						field_order_info,field_number);
					field_number++;
					component.number=0;
					set_FE_nodal_int_value(node,&component,/*version*/0,
						FE_NODAL_VALUE,channel_number);
				} break;
				case TORSO_ELECTRODE_TYPE:
				{
					/* 1st field contains electrode coords */	
					component.field=get_FE_field_order_info_field(
						field_order_info,field_number);
					field_number++;
					component.number = 0;					
					set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,xpos);
					component.number = 1;	
					set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,ypos);
					component.number = 2;	
					set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,zpos);
					/* 2nd field contains device name */
					field=get_FE_field_order_info_field(field_order_info,
						field_number);
					field_number++;
					component_number=0;
					set_FE_nodal_string_value(node,field,component_number,/*version*/0,
						FE_NODAL_VALUE,name);
					/* 3rd field contains device type */
					field=get_FE_field_order_info_field(field_order_info,
						field_number);
					field_number++;
					component_number=0;
					set_FE_nodal_string_value(node,field,component_number,/*version*/0,
						FE_NODAL_VALUE,"ELECTRODE");
					/* 4th field contains channel number */					
					component.field=get_FE_field_order_info_field(
						field_order_info,field_number);
					field_number++;
					component.number=0;
					set_FE_nodal_int_value(node,&component,/*version*/0,
						FE_NODAL_VALUE,channel_number);
				} break;		
				case PATCH_ELECTRODE_TYPE:
				{	 
					/* 1st field contains electrode coords */
					component.field=get_FE_field_order_info_field(
						field_order_info,field_number);
					field_number++;
					component.number = 0;					
					set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,xpos);
					component.number = 1;	
					set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,ypos);
					/* 2nd field contains device name */
					field=get_FE_field_order_info_field(field_order_info,
						field_number);
					field_number++;
					component_number=0;
					set_FE_nodal_string_value(node,field,component_number,/*version*/0,
						FE_NODAL_VALUE,name);
					/* 3rd field contains device type */
					field=get_FE_field_order_info_field(field_order_info,
						field_number);
					field_number++;
					component_number=0;
					set_FE_nodal_string_value(node,field,component_number,/*version*/0,
						FE_NODAL_VALUE,"ELECTRODE");
					/* 4th field contains channel number */					
					component.field=get_FE_field_order_info_field(
						field_order_info,field_number);
					field_number++;
					component.number=0;
					set_FE_nodal_int_value(node,&component,/*version*/0,
						FE_NODAL_VALUE,channel_number);
				} break;
				case AUXILIARY_TYPE:
				{					
					/* 1st field contains device name */
					field=get_FE_field_order_info_field(field_order_info,
						field_number);
					field_number++;
					component_number=0;
					set_FE_nodal_string_value(node,field,component_number,/*version*/0,
						FE_NODAL_VALUE,name);
					/* 2nd field contains device type */
					field=get_FE_field_order_info_field(field_order_info,
						field_number);
					field_number++;
					component_number=0;
					set_FE_nodal_string_value(node,field,component_number,/*version*/0,
						FE_NODAL_VALUE,name);
					/* 3rd field contains channel number */					
					component.field=get_FE_field_order_info_field(
						field_order_info,field_number);
					field_number++;
					component.number=0;
					set_FE_nodal_int_value(node,&component,/*version*/0,
						FE_NODAL_VALUE,channel_number);
				} break;
			}	/* switch() */
			if (FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,
				cm_node_identifier)(node_number,node_manager))
			{									
				display_message(ERROR_MESSAGE,
					"set_config_FE_node.  Node already exists!");	
				node=(struct FE_node *)NULL;
			}				
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_config_FE_node.  Error creating node");	
			node=(struct FE_node *)NULL;
		}					
	}	
	else
	{
		display_message(ERROR_MESSAGE,"set_config_FE_node.  Invalid argument(s)");
		node=(struct FE_node *)NULL;
	}
	LEAVE;
	return (node);
}/* set_config_FE_node*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
static struct FE_node *read_text_config_FE_node(FILE *input_file,
	struct FE_node *template_node,struct MANAGER(FE_node) *node_manager,
	enum Config_node_type	config_node_type,int node_number,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Reads a node  from a text configuration file, using the template node created in
create_config_template_node.
cf read_FE_node() in import_finite_element.c
==============================================================================*/
{	
	char *dummy_str,*electrode_name,*name,separator,string[10];
	FE_value coefficient,*coefficients,*coefficients_temp,sign,xpos,ypos,zpos;	
	int channel_number,found,electrode_number,*electrode_numbers,
		*electrode_numbers_temp,number_of_electrodes,string_length,success;
	struct FE_node *node;

	ENTER(read_text_config_FE_node);
	node=(struct FE_node *)NULL;
	success=0;
	if (input_file&&template_node&&node_manager&&field_order_info)
	{
		fscanf(input_file," : ");
		/* read in name */
		dummy_str=(char *)NULL;
		if (read_string(input_file,"[^\n]",&dummy_str)&&
			(name=trim_string(dummy_str)))
		{
			DEALLOCATE(dummy_str);						
			number_of_electrodes=0;
			electrode_numbers=(int *)NULL;
			coefficients=(FE_value *)NULL;
			fscanf(input_file," ");
			switch (config_node_type)
			{
				case SOCK_ELECTRODE_TYPE:
				case TORSO_ELECTRODE_TYPE:
				{											
					/* read in channel number */ 
					if (1==fscanf(input_file,"channel : %d",&channel_number))
					{
						fscanf(input_file," ");
						if (3==fscanf(input_file,"position :  x = %f, y = %f, z = %f",
							&xpos,&ypos,&zpos))
						{
							success=1;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_text_config_FE_node. Error reading channel from file)");
						node=(struct FE_node *)NULL;				
					}
				} break;		
				case PATCH_ELECTRODE_TYPE:
				{
					/* read in channel number */ 
					if (1==fscanf(input_file,"channel : %d",&channel_number))
					{
						fscanf(input_file," ");
						if (2==fscanf(input_file,"position :  x = %f, y = %f",&xpos,&ypos))
						{
							success=1;
						}
					}
				} break;
				case AUXILIARY_TYPE:
				{
					/* check if single channel or linear combination of devices */
					string[0]='\0';
					fscanf(input_file," %7[^ \n]",string);
					if (('c'==string[0])&&('h'==string[1])&&('a'==string[2])&&
						('n'==string[3])&&('n'==string[4])&&('e'==string[5])&&
						('l'==string[6])&&('\0'==string[7]))
					{
						/* single channel */
						if (1==fscanf(input_file," : %d ",&channel_number))
						{
							success=1;
						}
					}
					else
					{
						if (('s'==string[0])&&('u'==string[1])&&
							('m'==string[2])&&('\0'==string[3]))
						{
							/* linear combination */
							/*???DB.  Haven't decided how to store at node (could have an
								array of node numbers and an array of coefficients).  Put
								channel number 0 at node for now */
							channel_number=0;
							fscanf(input_file," : ");
							sign=1;
							success=1;
							do
							{
								fscanf(input_file," ");
								if (read_string(input_file,"[^+-\n]",&dummy_str))
								{
									if (0<strlen(dummy_str))
									{
										if (electrode_name=strchr(dummy_str,'*'))
										{
											*electrode_name='\0';
											electrode_name++;
											if (1!=sscanf(dummy_str,"%f",&coefficient))
											{
												success=0;
											}
										}
										else
										{
											electrode_name=dummy_str;
											coefficient=(FE_value)1;
										}
										if (success)
										{
											coefficient *= sign;
											/* trim leading space */
											while (' '== *electrode_name)
											{
												electrode_name++;
											}
											/* trim trailing space */
											string_length=strlen(electrode_name);
											while ((0<string_length)&&
												(' '==electrode_name[string_length-1]))
											{
												string_length--;
											}
											electrode_name[string_length]='\0';
											/* find the node in the device list with electrode_name */
												/*???DB.  To be done */
											electrode_number=0;
											found=1;
#if defined (OLD_CODE)
											device_item=device_list;
											found=0;
											while (device_item&&!found)
											{
												if ((device_item->device)&&
													(device_item->device->
													description)&&
													(ELECTRODE==device_item->
													device->description->type)&&
													(device_item->device->
													description->name)&&
													(0==strcmp(name,device_item->
													device->description->name)))
												{
													found=1;
												}
												else
												{
													device_item=device_item->next;
												}
											}
#endif /* defined (OLD_CODE) */
											if (found)
											{
												if (REALLOCATE(coefficients_temp,coefficients,FE_value,
													number_of_electrodes+1))
												{
													coefficients=coefficients_temp;
												}
												if (REALLOCATE(electrode_numbers_temp,
													electrode_numbers,int,number_of_electrodes+1))
												{
													electrode_numbers=electrode_numbers_temp;
												}
												if (coefficients&&electrode_numbers)
												{
													coefficients[number_of_electrodes]=coefficient;
													electrode_numbers[number_of_electrodes]=
														electrode_number;
													number_of_electrodes++;
												}
												else
												{
													success=0;
												}
											}
											else
											{
												success=0;
											}
										}
									}
									fscanf(input_file,"%c",&separator);
									if ('-'==separator)
									{
										sign=(float)-1;
									}
									else
									{
										sign=(float)1;
									}
									DEALLOCATE(dummy_str);
								}
								else
								{
									success=0;
								}
							} while (success&&('\n'!=separator));
						}
					}
				} break;				
			}
			if (success)
			{
				if (!(node=set_config_FE_node(input_file,template_node,node_manager,
					config_node_type,node_number,field_order_info,name,
					channel_number,xpos,ypos,zpos)))
				{		
					display_message(ERROR_MESSAGE,
						"read_text_config_FE_node.  Error creating node");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
"read_text_config_FE_node.  Error reading device type specific information from file ");
				node=(struct FE_node *)NULL;
			}
			DEALLOCATE(coefficients);
			DEALLOCATE(electrode_numbers);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_text_config_FE_node. Error reading name from file)");
			node=(struct FE_node *)NULL;		
		}	
		DEALLOCATE(dummy_str);						
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_text_config_FE_node.  Invalid argument(s)");
		node=(struct FE_node *)NULL;
	}
	LEAVE;

	return (node);
} /* read_text_config_FE_node */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
static struct FE_node *read_binary_config_FE_node(FILE *input_file,
	struct FE_node *template_node,struct MANAGER(FE_node) *node_manager,
	enum Config_node_type	config_node_type,int node_number,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
Reads a node from a binary configuration file, using the template node created
in create_config_template_node.
cf read_FE_node() in import_finite_element.c
==============================================================================*/
{
	char *name;
	int channel_number,*node_numbers,number_of_electrodes,string_length,success;
	FE_value *coefficients,xpos,ypos,zpos;
	struct FE_node *node;

	ENTER(read_binary_config_FE_node);
	node=(struct FE_node *)NULL;
	success=0;
	if (input_file&&template_node&&node_manager&&field_order_info)
	{
		BINARY_FILE_READ((char *)&string_length,sizeof(int),1,input_file);
		if (ALLOCATE(name,char,string_length+1))
		{
			/* read in name */
			BINARY_FILE_READ(name,sizeof(char),string_length,input_file);
			name[string_length]='\0';
			/* read channel number */
			BINARY_FILE_READ((char *)&channel_number,sizeof(int),1,input_file);
			number_of_electrodes=0;
			coefficients=(FE_value *)NULL;
			node_numbers=(int *)NULL;
			switch (config_node_type)
			{
				case SOCK_ELECTRODE_TYPE:
				case TORSO_ELECTRODE_TYPE:
				{
					BINARY_FILE_READ((char *)&xpos,sizeof(float),1,input_file);
					BINARY_FILE_READ((char *)&ypos,sizeof(float),1,input_file);
					BINARY_FILE_READ((char *)&zpos,sizeof(float),1,input_file);	
					success=1;
				} break;
				case PATCH_ELECTRODE_TYPE:
				{
					BINARY_FILE_READ((char *)&xpos,sizeof(float),1,input_file);
					BINARY_FILE_READ((char *)&ypos,sizeof(float),1,input_file);
					success=1;
				} break;
				case AUXILIARY_TYPE:
				{
					if (channel_number<0)
					{
						/* linear combination */
						number_of_electrodes= -channel_number;
						/*???DB.  Haven't decided how to store at node (could have an
							array of node numbers and an array of coefficients).  Put
							channel number 0 at node for now */
						channel_number=0;
						ALLOCATE(coefficients,FE_value,number_of_electrodes);
						ALLOCATE(node_numbers,int,number_of_electrodes);
						if (coefficients&&node_numbers)
						{
							BINARY_FILE_READ((char *)coefficients,sizeof(FE_value),
								number_of_electrodes,input_file);
							BINARY_FILE_READ((char *)node_numbers,sizeof(int),
								number_of_electrodes,input_file);
							/*???DB.  Haven't done conversion to node numbers (should be
								similar to that for pages */
							success=1;
						}
					}
					else
					{
						/* single channel */
						success=1;
					}
				} break;
			}
			if (success)
			{
				if (!(node=set_config_FE_node(input_file,template_node,node_manager,
					config_node_type,node_number,field_order_info,name,
					channel_number,xpos,ypos,zpos)))
				{
					display_message(ERROR_MESSAGE,
						"read_binary_config_FE_node.  Error creating node");
					node=(struct FE_node *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
"read_binary_config_FE_node.  Error reading device type specific information from file ");
				node=(struct FE_node *)NULL;
			}
			DEALLOCATE(coefficients);
			DEALLOCATE(node_numbers);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_binary_config_FE_node.  Could not read name");
			node=(struct FE_node *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_binary_config_FE_node.  Invalid argument(s)");
		node=(struct FE_node *)NULL;
	}
	LEAVE;

	return (node);
} /* read_binary_config_FE_node */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (OLD_CODE)
#if defined (UNEMAP_USE_NODES)
static struct GROUP(FE_node) *get_config_node_group(
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_element))	*element_group_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,char *group_name)
/*******************************************************************************
LAST MODIFIED : 18 May 1999

DESCRIPTION :
Gets a node group ,struct GROUP(FE_node), for the configuration, using the supplied
group_name. The group is cached, with MANAGED_GROUP_BEGIN_CACHE(FE_node).
The caching must be ended by the calling function.
==============================================================================*/
{

	struct FE_node *node_to_destroy;
	struct GROUP(FE_node) *existing_group,*node_group,*same_name_data_group;
	struct GROUP(FE_element) *same_name_element_group;	
	int success;

	ENTER(get_config_node_group);
	if(node_group_manager&&node_manager&&element_group_manager&&data_group_manager&&
		 group_name)
	{
		node_group = (struct GROUP(FE_node) *)NULL;
		success = 1;

		if ((existing_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
			group_name,node_group_manager)))
		{
			node_group = existing_group;

			/* now need to clean out the group */
			while(success&&(node_to_destroy=FIRST_OBJECT_IN_GROUP_THAT(FE_node)
				((GROUP_CONDITIONAL_FUNCTION(FE_node) *)NULL, NULL, node_group)))
			{
				success = REMOVE_OBJECT_FROM_GROUP(FE_node)(
					node_to_destroy, node_group);
				if (FE_node_can_be_destroyed(node_to_destroy))
				{
					success = REMOVE_OBJECT_FROM_MANAGER(FE_node)(node_to_destroy,
						node_manager);
				}
			}				
			/* do we have anything in same_name_data_group or same_name_element_group? */
			/* if we do, fail the function, set node_group to NULL */
			/* if we don't,sucess!, do MANAGED_GROUP_BEGIN_CACHE(FE_node)(node_group) */
			same_name_data_group=
				FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(group_name,data_group_manager);
			if((same_name_data_group)&&!(NUMBER_IN_GROUP(FE_node)(same_name_data_group)))
			{					
				success = 1;										
			}
			else
			{
				success = 0;
			}				
			same_name_element_group=
				FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)(group_name,element_group_manager);
			if((same_name_element_group)&&!(NUMBER_IN_GROUP(FE_element)(same_name_element_group)))
			{					
				success = 1;										
			}
			else
			{
				success = 0;
			}
			if(success)
			{	
				MANAGED_GROUP_BEGIN_CACHE(FE_node)(node_group);
			}	
			else
			{				
				node_group = (struct GROUP(FE_node) *)NULL;	 
			}									 
		}
		else
		{
			node_group = CREATE_GROUP(FE_node)(group_name);
			/* now need to add the group to manager */

			if (!(success=ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(
				node_group,node_group_manager)))
			{
				DESTROY(GROUP(FE_node))(&node_group);
			}
			/* make sure there are data and element groups of the same name
				 so that GT_element_group automatically created with them */
			same_name_data_group=CREATE(GROUP(FE_node))(group_name);
			if (!ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(same_name_data_group,
				data_group_manager))
			{
				DESTROY(GROUP(FE_node))(&same_name_data_group);
			}
			same_name_element_group=CREATE(GROUP(FE_element))(group_name);
			if (!ADD_OBJECT_TO_MANAGER(GROUP(FE_element))(
				same_name_element_group,element_group_manager))
			{
				DESTROY(GROUP(FE_element))(&same_name_element_group);
			}
			MANAGED_GROUP_BEGIN_CACHE(FE_node)(node_group);
		}
	}
	else
	{	
		display_message(ERROR_MESSAGE,"get_config_node_group. Invalid arguments");	
		node_group = (struct GROUP(FE_node) *)NULL;
	}					
	LEAVE;
	return(node_group);
}/* get_config_node_group */
#endif /* defined (UNEMAP_USE_NODES) */
#endif /* defined (OLD_CODE) */

#if defined (UNEMAP_USE_NODES)
static struct GROUP(FE_node) *read_binary_config_FE_node_group(FILE *input_file,
	struct Unemap_package *package,enum Region_type rig_type,
	struct FE_node_order_info **the_node_order_info) 
/*******************************************************************************
LAST MODIFIED : 27 April 2000

DESCRIPTION :
Reads and returns a node group from a configuration file. An FE_node_order_info
is created, and so must be destroyed by the calling function.
cf read_FE_node_group() in import_finite_element.c, and read_configuration()
in rig.c
==============================================================================*/
{	
	char *device_name,*page_name,*rig_name,*region_name;
	char region_num_string[10];	
	enum Config_node_type config_node_type;	
	enum Device_type device_type;
	enum Region_type region_type;
	FE_value focus;	
	int count,device_count,device_number,last_node_number,node_number,
		number_of_pages,number_of_page_devices,number_of_regions,region_number,
		region_number_of_devices,string_length,success;
	int string_error =0;
	struct FE_field_order_info *field_order_info;	
	struct FE_node *node,*template_node;
	struct FE_node_order_info *node_order_info;	
	struct GROUP(FE_node) *node_group,*all_devices_node_group;	
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_element))	*element_group_manager;	
	struct MANAGER(GROUP(FE_node)) *data_group_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;	

	ENTER(read_binary_config_FE_node_group);	
	success = 1;
	rig_name =(char *)NULL;	
	region_name =(char *)NULL;
	device_name =(char *)NULL;
	page_name =(char *)NULL;	
	template_node = (struct FE_node *)NULL;	
	field_order_info = (struct FE_field_order_info *)NULL;
	node_group = (struct GROUP(FE_node) *)NULL;
	all_devices_node_group = (struct GROUP(FE_node) *)NULL;
	node_order_info =(struct FE_node_order_info *)NULL;	
	element_manager= (struct MANAGER(FE_element) *)NULL;
	device_count =0;
	if(input_file&&package) 
	{			
		element_group_manager=get_unemap_package_element_group_manager(package);
		node_manager=get_unemap_package_node_manager(package);
		element_manager=get_unemap_package_element_manager(package);
		data_group_manager=get_unemap_package_data_group_manager(package);
		node_group_manager= get_unemap_package_node_group_manager(package);
		/* we've read in the rig type, now read in the rig name */
		/* read the rig name */
		BINARY_FILE_READ((char *)&string_length,sizeof(int),1,input_file);
		if(ALLOCATE(rig_name,char,string_length))
		{
			if (string_length>0)
			{
				BINARY_FILE_READ(rig_name,sizeof(char),string_length,input_file);
			}
			(rig_name)[string_length]='\0';
			
			/* read in the initial rig name and type */	
			focus=0.0;
			switch (rig_type)
			{				
				case MIXED:
				{	
					;/* do nothing for mixed case, need to read in more info to decide node_type*/
				}break;
				case SOCK:
				{			
					/* read in focus */
					BINARY_FILE_READ((char *)&focus,sizeof(float),1,input_file);															
					config_node_type = SOCK_ELECTRODE_TYPE;
				}break;
				case PATCH:
				{
					config_node_type = PATCH_ELECTRODE_TYPE;
				}break;
				case TORSO:	
				{
					config_node_type = TORSO_ELECTRODE_TYPE;
				}break;
			}		
			MANAGER_BEGIN_CACHE(FE_node)(node_manager);	
			all_devices_node_group	= make_node_and_element_and_data_groups(
				node_group_manager,node_manager,element_manager,element_group_manager,
				data_group_manager,"all_devices");
			set_unemap_package_rig_node_group(package,all_devices_node_group);
			last_node_number = 1;	
			/* read the number of regions */
			BINARY_FILE_READ((char *)&number_of_regions,sizeof(int),1,input_file);	
			region_number=0;		
			while((region_number<number_of_regions)&&success)
			{
				/* read the region type */
				if (MIXED==rig_type)
				{
					BINARY_FILE_READ((char *)&region_type,sizeof(enum Region_type),
						1,input_file);
				}
				else
				{
					region_type=rig_type;
				}
				/* read the region name */
				BINARY_FILE_READ((char *)&string_length,sizeof(int),1,input_file);
				if (ALLOCATE(region_name,char,string_length+1))
				{					
					BINARY_FILE_READ(region_name,sizeof(char),string_length,input_file);	
					region_name[string_length]='\0';
					/*append the region number to the name, to ensure it's unique*/
					sprintf(region_num_string,"%d",region_number);
					append_string(&region_name,region_num_string,&string_error);
					if (MIXED==rig_type)
					{
						switch (region_type)
						{
							case SOCK:
							{	
								BINARY_FILE_READ((char *)&focus,sizeof(float),1,input_file);
								config_node_type = SOCK_ELECTRODE_TYPE;
								/* destroy any existing template and field_order_info*/
								/* create the template node */
								DESTROY(FE_node)(&template_node);
								DESTROY(FE_field_order_info)(&field_order_info);	
								if(node_group) /* stop caching the current group,(if any) */
								{
									MANAGED_GROUP_END_CACHE(FE_node)(node_group);
								}			
								template_node = create_config_template_node(SOCK_ELECTRODE_TYPE,	
									focus,&field_order_info,package);						
								node_group = make_node_and_element_and_data_groups(node_group_manager,
									node_manager,element_manager,element_group_manager,data_group_manager,
									region_name);
								set_unemap_package_rig_node_group(package,node_group);
							} break;
							case TORSO:
							{	
								config_node_type = TORSO_ELECTRODE_TYPE;
								/* destroy any existing template and field_order_info*/
								/* create the template node */
								DESTROY(FE_node)(&template_node);
								DESTROY(FE_field_order_info)(&field_order_info);
								if(node_group) /* stop caching the current group,(if any) */
								{
									MANAGED_GROUP_END_CACHE(FE_node)(node_group);
								}	
								template_node = create_config_template_node(TORSO_ELECTRODE_TYPE,
									0,&field_order_info,package);
								node_group = make_node_and_element_and_data_groups(node_group_manager,
									node_manager,element_manager,element_group_manager,data_group_manager,
									region_name);
								set_unemap_package_rig_node_group(package,node_group);
							} break;
							case PATCH:
							{
								config_node_type = PATCH_ELECTRODE_TYPE;
								/* destroy any existing template and field_order_info*/
								/* create the template node */
								DESTROY(FE_node)(&template_node);
								DESTROY(FE_field_order_info)(&field_order_info);	
								if(node_group) /* stop caching the current group,(if any) */
								{
									MANAGED_GROUP_END_CACHE(FE_node)(node_group);
								}		
								template_node = create_config_template_node(PATCH_ELECTRODE_TYPE,
									0,&field_order_info,package);
								node_group = make_node_and_element_and_data_groups(node_group_manager,
									node_manager,element_manager,element_group_manager,data_group_manager,
									region_name);
								set_unemap_package_rig_node_group(package,node_group);
							}break;
						}/*	switch (region_type) */	
					}	/*	if (MIXED==rig_type)  */
					else
					{						
						if(node_group) /* stop caching the current group,(if any) */
						{
							MANAGED_GROUP_END_CACHE(FE_node)(node_group);
						}								
						/* Get group */
						node_group = make_node_and_element_and_data_groups(node_group_manager,
							node_manager,element_manager,element_group_manager,data_group_manager,
							region_name);
						set_unemap_package_rig_node_group(package,node_group);
						/* destroy any exisiting template and field_order_info*/
						/* create the template node,( but may change it to auxiliary later) */					
						DESTROY(FE_node)(&template_node);
						DESTROY(FE_field_order_info)(&field_order_info);
						switch(config_node_type)
						{							
							case SOCK_ELECTRODE_TYPE:
							{
								template_node = create_config_template_node(SOCK_ELECTRODE_TYPE,
									focus,&field_order_info,package);
							}break;
							case TORSO_ELECTRODE_TYPE:
							{
								template_node = create_config_template_node(TORSO_ELECTRODE_TYPE,
									0,&field_order_info,package);
							}break;
							case PATCH_ELECTRODE_TYPE:
							{
								template_node = create_config_template_node(PATCH_ELECTRODE_TYPE,
									0,&field_order_info,package);
							}break;
						}	/* switch(config_node_type) */
					}			
					/* read the devices for the region */
					/* read the number of inputs */
					BINARY_FILE_READ((char *)&region_number_of_devices,sizeof(int),1,input_file);	
					/* create or reallocate the node_order_info */
					if(!node_order_info)
					{
						node_order_info =CREATE(FE_node_order_info)
							(region_number_of_devices);
						*the_node_order_info = node_order_info;
						if(!node_order_info)
						{
							display_message(ERROR_MESSAGE,"read_binary_config_FE_node_group. "
								"CREATE(FE_node_order_info) failed ");
							success =0;						
							DESTROY_GROUP(FE_node)(&node_group);
							node_group = (struct GROUP(FE_node) *)NULL;					
						}
					}
					else
					{ 
						if(!(success =add_nodes_FE_node_order_info(region_number_of_devices,
							node_order_info)))
						{							
							DESTROY_GROUP(FE_node)(&node_group);
							node_group = (struct GROUP(FE_node) *)NULL;	
						}
					}
					/* read in the inputs */
					while((region_number_of_devices>0)&&success)
					{
						BINARY_FILE_READ((char *)&device_type,sizeof(enum Device_type),1,
							input_file);
						/* if valid device type */
						if ((ELECTRODE==device_type)||(AUXILIARY==device_type))
						{													
							/* read the device number */
							BINARY_FILE_READ((char *)&device_number,sizeof(int),1,input_file);
							node_number = get_next_FE_node_number(node_manager,last_node_number);	
							/* read device type dependent properties */
							switch (device_type)
							{
								case ELECTRODE:
								{
									node=read_binary_config_FE_node(input_file,template_node,node_manager,
										config_node_type,node_number,field_order_info);	
									last_node_number = node_number;	
								} break;
								case AUXILIARY:
								{	
									config_node_type = AUXILIARY_TYPE;	
									DESTROY(FE_node)(&template_node);
									DESTROY(FE_field_order_info)(&field_order_info);	
									template_node = create_config_template_node(config_node_type,
										0,&field_order_info,package);
									node=read_binary_config_FE_node(input_file,template_node,node_manager,
										config_node_type,node_number,field_order_info);
									last_node_number = node_number;										
								}break;
							}	/* switch (device_type) */
							if(node)
							{ 
								if(ADD_OBJECT_TO_MANAGER(FE_node)(node,node_manager))
								{						
									if(ADD_OBJECT_TO_GROUP(FE_node)(node,node_group))
									{
										if(ADD_OBJECT_TO_GROUP(FE_node)(node,all_devices_node_group))
										{
											/* add the node to the node_order_info*/											
											set_FE_node_order_info_node(node_order_info,device_count,node);
											device_count++;
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"read_binary_config_FE_node_group."
												" Could not add node to all_devices_node_group");
											REMOVE_OBJECT_FROM_GROUP(FE_node)(node,all_devices_node_group);
											REMOVE_OBJECT_FROM_MANAGER(FE_node)(node,node_manager);
											node=(struct FE_node *)NULL;
											success=0;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"read_binary_config_FE_node_group.  Could not add node to node_group");
										REMOVE_OBJECT_FROM_GROUP(FE_node)(node,node_group);
										REMOVE_OBJECT_FROM_MANAGER(FE_node)(node,node_manager);
										node=(struct FE_node *)NULL;
										success=0;
									}							
								}/* if(!ADD_OBJECT_TO_MANAGER(FE_node) */
								else
								{
									display_message(ERROR_MESSAGE,
										"read_binary_config_FE_node_group.  Could not add node to node_manager");
									REMOVE_OBJECT_FROM_MANAGER(FE_node)(node,node_manager);
									node=(struct FE_node *)NULL;
									success=0;
								}
							}/*	if(node) */		
							if(device_name)
							{
								DEALLOCATE(device_name);
							}
						}/* if ((ELECTRODE==device_type)||(AUXILIARY==device_type)) */
						else
						{
							display_message(ERROR_MESSAGE,
								"read_binary_config_FE_node_group. Invalid device type");
							success = 0;
						}
						region_number_of_devices--;
					} /*	while (region_number_of_devices>0)&&success) */
				}	/* if (ALLOCATE(region_name */
				else
				{
					display_message(ERROR_MESSAGE,
						"read_binary_config_FE_node_group. Could not create region name");
					success =0;		
				}	
				if (success)
				{
					/* move to the next region */
					region_number++;				
				}
				if(region_name)
				{
					DEALLOCATE(region_name);
				}			
			}/*	while((region_number<number_of_regions)&&success) */		
			if (success)
			{
				/* read the number of pages */
				BINARY_FILE_READ((char *)&number_of_pages,sizeof(int),1,input_file);
				/* read in the pages */
				count=0;		
				while ((count<number_of_pages)&&success)
				{									
					/* read the page name */
					BINARY_FILE_READ((char *)&string_length,sizeof(int),1,input_file);
					if (ALLOCATE(page_name,char,string_length+1))
					{
						BINARY_FILE_READ(page_name,sizeof(char),string_length,input_file);
						page_name[string_length]='\0';
						/* read the number of devices in the device list */
						BINARY_FILE_READ((char *)&number_of_page_devices,sizeof(int),1,input_file);
						/* read the device list */					
						while ((number_of_page_devices>0)&&success)
						{
							BINARY_FILE_READ((char *)&device_number,sizeof(int),1,input_file);						
							number_of_page_devices--;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_binary_config_FE_node_group.  Could not create page name");
						success = 0;
					}			 
					if(page_name)
					{
						DEALLOCATE(page_name);
					}
					count++;
				} /*	while ((count<number_of_pages)&&success) */
			}	/* if (success) */
		}
		else
		{
			display_message(ERROR_MESSAGE,"read_binary_config_FE_node_group. "
				"Can't read rig name");	
			success =0;		
		}	
		/* We've finished. Clean up */	
		if (template_node)
		{
			DESTROY(FE_node)(&template_node);
		}			
		MANAGER_END_CACHE(FE_node)(node_manager);
		if(node_group)
		{
			MANAGED_GROUP_END_CACHE(FE_node)(node_group);
		}		
		if(all_devices_node_group)
		{
			MANAGED_GROUP_END_CACHE(FE_node)(all_devices_node_group);
		}	
		if (field_order_info)
		{
			DESTROY(FE_field_order_info)(&field_order_info);
		}			
		if(rig_name)
		{
			DEALLOCATE(rig_name);
		}		
		if(region_name)
		{
			DEALLOCATE(region_name);
		}
		if(device_name)
		{
			DEALLOCATE(device_name);

		}		
		if(page_name)
		{
			DEALLOCATE(device_name);
		}	
	}
	else
	{
		success = 0;
		display_message(ERROR_MESSAGE,
			"read_binary_config_FE_node_group. Invalid arguments");
	}
	LEAVE;
	return (all_devices_node_group);
} /* read_binary_config_FE_node_group */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
static struct GROUP(FE_node) *read_text_config_FE_node_group(FILE *input_file,
	struct Unemap_package *package,enum Region_type region_type)
/*******************************************************************************
LAST MODIFIED : 27 April 2000

DESCRIPTION :
Reads a node group from a configuration file.
cf read_FE_node_group() in import_finite_element.c, and read_configuration()
in rig.c and read_binary_config_FE_node_group above.
Unlike read_binary_config_FE_node_group above, don't set up a
FE_node_order_info, as this is used to pass info when reading signal
files, and there are no text signal files.
==============================================================================*/
{	
	char *dummy,*dummy_str,input_str[6],*name,*region_name,region_num_string[10],
		*rig_name,separator;
	enum Config_node_type config_node_type;
	FE_value focus;
	int device_number,finished,is_auxiliary,is_electrode,last_node_number,
		node_number,success,region_number;
	int string_error =0;
	struct FE_node *node,*template_node;			
	struct FE_field_order_info *field_order_info;	
	struct GROUP(FE_node) *node_group,*all_devices_node_group;
	struct MANAGER(FE_element) *element_manager;			
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_element))	*element_group_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;	

	ENTER(read_text_config_FE_node_group);	
	region_name=(char *)NULL;
	template_node=(struct FE_node *)NULL;	
	field_order_info=(struct FE_field_order_info *)NULL;
	dummy_str=(char *)NULL;
	node_group=(struct GROUP(FE_node) *)NULL; 	
	all_devices_node_group=(struct GROUP(FE_node) *)NULL;
	element_manager=(struct MANAGER(FE_element) *)NULL;
	device_number=0;
	region_number=0;
	if (input_file&&package) 
	{
		element_group_manager=get_unemap_package_element_group_manager(package);
		node_manager=get_unemap_package_node_manager(package);
		data_group_manager=get_unemap_package_data_group_manager(package);
		node_group_manager= get_unemap_package_node_group_manager(package);					
		/* we've read in the rig type, now read in the rig name */
		fscanf(input_file,"%*[ :\n]");
		/* rig name */		 
		if (read_string(input_file,"[^\n]",&dummy_str)&&
			(rig_name=trim_string(dummy_str))) 
		{	
			fscanf(input_file," ");						
			DEALLOCATE(dummy_str);
			/* read in the initial rig name and type */	
			focus=(float)0;							
			switch (region_type)
			{
				case MIXED:
				{	
					/* do nothing for mixed case, need to read in more info to decide
						node_type */
				} break;
				case SOCK:
				{				
					/* read in focus */						
					if (!fscanf(input_file,"focus for Hammer projection : %f",&focus))
					{
						display_message(WARNING_MESSAGE,"read_text_config_FE_node_group."
							" sock has no focus. Set focus =0.0");								
					}	
					fscanf(input_file," ");			
					config_node_type=SOCK_ELECTRODE_TYPE;
				} break;
				case PATCH:
				{
					config_node_type=PATCH_ELECTRODE_TYPE;
				} break;
				case TORSO:	
				{
					config_node_type=TORSO_ELECTRODE_TYPE;
				} break;
			}
			MANAGER_BEGIN_CACHE(FE_node)(node_manager);		
			all_devices_node_group=make_node_and_element_and_data_groups(
				node_group_manager,node_manager,element_manager,element_group_manager,
				data_group_manager,"all_devices");
			set_unemap_package_rig_node_group(package,all_devices_node_group);
			last_node_number=1;	 
			fscanf(input_file," ");				
			finished=0;
			while ((!feof(input_file))&&(!finished))
			{
				is_electrode=0;
				is_auxiliary=0;
				fscanf(input_file," ");
				fscanf(input_file,"%5c",input_str); 
				/* NULL terminate for use with strcmp */	
				input_str[5]='\0';
				/* for a mixed rig, may load in the actual rig information */
				if ((!strcmp("torso",input_str))&&(region_type==MIXED))
				{
					/* get the rig name */	
					fscanf(input_file,"%*[ :\n]");
					if (read_string(input_file,"[^\n]",&dummy_str)&&
						(rig_name=trim_string(dummy_str)))
					{	
						fscanf(input_file," ");
						/*append the region number to the name, to ensure it's unique*/
						sprintf(region_num_string,"%d",region_number);
						append_string(&rig_name,region_num_string,&string_error);					
						region_number++;
					}	
					else
					{	
						display_message(ERROR_MESSAGE,"read_text_config_FE_node_group."
							" Can't read torso rig name");	
						success =0;	
					}
					config_node_type=TORSO_ELECTRODE_TYPE;
					/* destroy any existing template and field_order_info */
					/* create the template node */
					DESTROY(FE_node)(&template_node);
					DESTROY(FE_field_order_info)(&field_order_info);
					/* stop caching the current group,(if any) */
					if (node_group)
					{
						MANAGED_GROUP_END_CACHE(FE_node)(node_group);
					}	
					template_node=create_config_template_node(TORSO_ELECTRODE_TYPE,0,
						&field_order_info,package);
					node_group=make_node_and_element_and_data_groups(node_group_manager,
						node_manager,element_manager,element_group_manager,
						data_group_manager,rig_name);					
					set_unemap_package_rig_node_group(package,node_group);
					/* new group, new rig, so reset device number */
					device_number=0;
				}
				else if ((!strcmp("patch",input_str))&&(region_type==MIXED))
				{	
					/* get the rig name */	
					fscanf(input_file,"%*[ :\n]");
					if (read_string(input_file,"[^\n]",&dummy_str)&&
						(rig_name=trim_string(dummy_str))) 
					{	
						fscanf(input_file," ");		
						/*append the region number to the name, to ensure it's unique*/
						sprintf(region_num_string,"%d",region_number);
						append_string(&rig_name,region_num_string,&string_error);						
						region_number++;										
					}	
					else
					{	
						display_message(ERROR_MESSAGE,"read_text_config_FE_node_group."
							" Can't read patch rig name");	
						success=0;	
					}
					config_node_type=PATCH_ELECTRODE_TYPE;
					/* destroy any existing template and field_order_info */
					/* create the template node */
					DESTROY(FE_node)(&template_node);
					DESTROY(FE_field_order_info)(&field_order_info);	
					/* stop caching the current group,(if any) */
					if (node_group)
					{
						MANAGED_GROUP_END_CACHE(FE_node)(node_group);
					}		
					template_node = create_config_template_node(PATCH_ELECTRODE_TYPE,
						0,&field_order_info,package);
					node_group=make_node_and_element_and_data_groups(node_group_manager,
						node_manager,element_manager,element_group_manager,
						data_group_manager,rig_name);					
					set_unemap_package_rig_node_group(package,node_group);
					/* new group, new rig, so reset device number */
					device_number=0;
				}
				else if((!strcmp("sock ",input_str))&&(region_type==MIXED))
				{
					/* shorten string, 'sock' only 4 chars long*/
					input_str[4]='\0';
					/* get the rig name */
					fscanf(input_file,"%*[ :\n]");
					if (read_string(input_file,"[^\n]",&dummy_str)&&
						(rig_name = trim_string(dummy_str))) 
					{	
						fscanf(input_file," ");				
						/* read in focus (if any)*/						
						if (!fscanf(input_file,"focus for Hammer projection : %f",&focus))
						{
							focus=(float)0;
						}	
						fscanf(input_file," ");	
						/*append the region number to the name, to ensure it's unique*/
						sprintf(region_num_string,"%d",region_number);
						append_string(&rig_name,region_num_string,&string_error);						
						region_number++;		
					}	
					else
					{	
						display_message(ERROR_MESSAGE,"read_text_config_FE_node_group."
							" Can't read sock rig name");	
						success=0;	
					}
					config_node_type=SOCK_ELECTRODE_TYPE;
					/* destroy any existing template and field_order_info */
					/* create the template node */
					DESTROY(FE_node)(&template_node);
					DESTROY(FE_field_order_info)(&field_order_info);	
					/* stop caching the current group,(if any) */
					if (node_group)
					{
						MANAGED_GROUP_END_CACHE(FE_node)(node_group);
					}			
					template_node=create_config_template_node(SOCK_ELECTRODE_TYPE,focus,
						&field_order_info,package);
					node_group=make_node_and_element_and_data_groups(node_group_manager,
						node_manager,element_manager,element_group_manager,
						data_group_manager,rig_name);
					set_unemap_package_rig_node_group(package,node_group);
					/* new group, new rig, so reset device number */
					device_number=0;
				}				
				else if(!strcmp("regio",input_str))
				{
					/* line is "region" */
					fscanf(input_file,"n : "); 
					read_string(input_file,"[^\n]",&dummy_str);
					region_name=trim_string(dummy_str);	
					/*append the region number to the name, to ensure it's unique*/
					sprintf(region_num_string,"%d",region_number);
					append_string(&region_name,region_num_string,&string_error);
					region_number++;						
					/* stop caching the current group,(if any) */
					if (node_group)
					{
						MANAGED_GROUP_END_CACHE(FE_node)(node_group);
					}								
					/* get group */
					node_group=make_node_and_element_and_data_groups(node_group_manager,
						node_manager,element_manager,element_group_manager,
						data_group_manager,region_name);
					set_unemap_package_rig_node_group(package,node_group);
					/* destroy any exisiting template and field_order_info */
					/* create the template node (but may change it to auxiliary later) */
					DESTROY(FE_node)(&template_node);
					DESTROY(FE_field_order_info)(&field_order_info);
					switch (config_node_type)
					{							
						case SOCK_ELECTRODE_TYPE:
						{
							template_node=create_config_template_node(SOCK_ELECTRODE_TYPE,
								focus,&field_order_info,package);
						} break;
						case TORSO_ELECTRODE_TYPE:
						{
							template_node=create_config_template_node(TORSO_ELECTRODE_TYPE,0,
								&field_order_info,package);
						} break;
						case PATCH_ELECTRODE_TYPE:
						{
							template_node=create_config_template_node(PATCH_ELECTRODE_TYPE,0,
								&field_order_info,package);
						}break;
					}
					/* new group, new rig, so reset device number */
					device_number=0;
				}				
				else if((is_electrode = !strcmp("elect",input_str))||
					(is_auxiliary=!strcmp("auxil",input_str)))
				{
					/* node, line is "electrode" or "auxiliary" */
					/* complete the word */
					fscanf(input_file,"%4c",input_str);
					node_number=get_next_FE_node_number(node_manager,last_node_number);
					fscanf(input_file," ");							
					/* have we got an auxiliary node? */
					if (is_auxiliary)
					{
						config_node_type=AUXILIARY_TYPE;	
						DESTROY(FE_node)(&template_node);
						DESTROY(FE_field_order_info)(&field_order_info);	
						template_node=create_config_template_node(config_node_type,0,
							&field_order_info,package);
						node=read_text_config_FE_node(input_file,template_node,node_manager,
							config_node_type,node_number,field_order_info);
						last_node_number=node_number;	
						device_number++;					
					}
					else if (is_electrode)
					{							
						/* not an auxiliary node,electrode node! */
						node=read_text_config_FE_node(input_file,template_node,node_manager,
							config_node_type,node_number,field_order_info);	
						last_node_number=node_number;
						device_number++;
					}						
					if (node)
					{ 
						if (ADD_OBJECT_TO_MANAGER(FE_node)(node,node_manager))
						{						
							if (ADD_OBJECT_TO_GROUP(FE_node)(node,node_group))
							{
								if (!ADD_OBJECT_TO_GROUP(FE_node)(node,all_devices_node_group))
								{
									display_message(ERROR_MESSAGE,
										"read_text_config_FE_node_group."
										" Could not add node to all_devices_node_group");
									REMOVE_OBJECT_FROM_GROUP(FE_node)(node,
										all_devices_node_group);
									REMOVE_OBJECT_FROM_MANAGER(FE_node)(node,node_manager);
									node=(struct FE_node *)NULL;
									success=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
					"read_text_config_FE_node_group.  Could not add node to node_group");
								REMOVE_OBJECT_FROM_GROUP(FE_node)(node,node_group);
								REMOVE_OBJECT_FROM_MANAGER(FE_node)(node,node_manager);
								node=(struct FE_node *)NULL;
								success=0;
							}							
						} /* if(!ADD_OBJECT_TO_MANAGER(FE_node) */
						else
						{
							display_message(ERROR_MESSAGE,
				"read_text_config_FE_node_group.  Could not add node to node_manager");
							REMOVE_OBJECT_FROM_MANAGER(FE_node)(node,node_manager);
							node=(struct FE_node *)NULL;
							success=0;
						}
					} /*if (node) */					
				} /* else if ((is_electrode = */
				else
				{
					finished=1;
				}
				fscanf(input_file," ");
				DEALLOCATE(dummy_str);			
			} /* while((!feof(input_file))&&(!finished)) */
			if (success&&!feof(input_file))
			{
				if (!strcmp("pages",input_str))			
				{				
					/* read the first page name */
					if (read_string(input_file,"s",&name))
					{
						fscanf(input_file," %c",&separator);
						while (!feof(input_file)&&success&&(':'==separator))
						{													
							/* read the list of devices */
							fscanf(input_file," ");
							separator=',';					
							while (!feof(input_file)&&success&&(separator!=':'))
							{
								if (read_string(input_file,"[^:, \n]",&dummy))
								{
									if (separator!=',')
									{
										if (ALLOCATE(name,char,strlen(dummy)+2))
										{
											name[0]=separator;
											name[1]='\0';
											strcat(name,dummy);
											DEALLOCATE(dummy);
										}
									}
									else
									{
										name=dummy;
									}
									if (name)
									{
										fscanf(input_file," %c ",&separator);
										if (separator!=':')
										{
											
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"read_text_config_FE_node_group."
											"  Could not create device name for page");
										success=0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_text_config_FE_node_group."
										"  Could not create device name for page");
									success=0;
								}
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_text_config_FE_node_group.  Could not create page name");
						success=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_text_config_FE_node_group.  Invalid device type");
					success=0;
				}
			}
		}	
		else
		{	
			display_message(ERROR_MESSAGE,
				"read_text_config_FE_node_group. Can't read rig name");	
			success=0;	
		}					
		/* we've finished. Clean up */
		/* clear template node */			
		if (template_node)
		{
			DESTROY(FE_node)(&template_node);
		}
		MANAGER_END_CACHE(FE_node)(node_manager);
		if (node_group)
		{
			MANAGED_GROUP_END_CACHE(FE_node)(node_group);
		}
		if (all_devices_node_group)
		{
			MANAGED_GROUP_END_CACHE(FE_node)(all_devices_node_group);
		}	
		if (rig_name)
		{
			DEALLOCATE(rig_name);					
		}
		if (region_name)
		{
			DEALLOCATE(region_name);						
		}	
		if (dummy_str)
		{
			DEALLOCATE(dummy_str);						
		}					
		if (field_order_info)
		{
			DESTROY(FE_field_order_info)(&field_order_info);
		}			
	}
	else
	{
		success=0;
		display_message(ERROR_MESSAGE,
			"read_text_config_FE_node_group. Invalid arguments");
	}
	LEAVE;

	return (all_devices_node_group);
} /* read_text_config_FE_node_group */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
static struct GROUP(FE_node) *read_config_FE_node_group(FILE *input_file,
	struct Unemap_package *unemap_package,
	enum Region_type region_type,	enum Rig_file_type file_type,
	struct FE_node_order_info **node_order_info)
/*******************************************************************************
LAST MODIFIED : 28 July 1999

DESCRIPTION :
Reads a node group from a configuration file.
cf read_FE_node_group() in import_finite_element.c
==============================================================================*/
{		
	struct GROUP(FE_node) *node_group;

	ENTER(read_config_FE_node_group);	
	node_group=(struct GROUP(FE_node) *)NULL;	
	if (input_file&&unemap_package)
	{		
		/* clear out the old node and element groups, and asscoiated fields */ 	
		free_unemap_package_rig_info(unemap_package);	
		free_unemap_package_maps(unemap_package);
		if (TEXT==file_type)
		{
			node_group=read_text_config_FE_node_group(input_file,unemap_package,region_type);
		}
		else
		{
			/* must be binary */
			node_group=read_binary_config_FE_node_group(input_file,unemap_package,
				region_type,node_order_info); 		
		}		
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"read_config_FE_node_group. Invalid arguments");
	}
	LEAVE;

	return (node_group);
} /* read_config_FE_node_group */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
static int read_signal_FE_node_group(FILE *input_file,
	struct Unemap_package *package,
	struct FE_node_order_info *node_order_info)
/*******************************************************************************
LAST MODIFIED : 2 August 1999

DESCRIPTION :
Reads signals from a signal file, stores them in a node group, via 
FE_node_order_info.
Performs the same loading functions as read_signal_file, but using the rig_node group
Doesn't load in auxilliary devices that are linear combinations of other channels.
==============================================================================*/
{

		enum FE_nodal_value_type
		*signal_components_nodal_value_types[1]=
		{
			{
				FE_NODAL_VALUE
			}
		},	
		*channel_gain_components_nodal_value_types[1]=
		{
			{
				FE_NODAL_VALUE
			}
		},	
		*signal_maximum_components_nodal_value_types[1]=
		{
			{
				FE_NODAL_VALUE
			}
		},
		*signal_minimum_components_nodal_value_types[1]=
		{
			{
				FE_NODAL_VALUE
			}
		},	
		*channel_offset_components_nodal_value_types[1]=
		{
			{
				FE_NODAL_VALUE
			}
		},
		*signal_status_components_nodal_value_types[1]=
		{
			{
				FE_NODAL_VALUE
			}
		};

	  char
		*channel_gain_component_names[1]=
		{
			"gain_value"
		},	
		*signal_maximum_component_names[1]=
		{
			"maximum_value"
		},
		*signal_minimum_component_names[1]=
		{
			"minimum_value"
		},
		*channel_offset_component_names[1]=
		{
			"offset_value"
		},
		*signal_component_names[1]=
		{
			"signal_value"
		},	
		*signal_status_component_names[1]=
		{
			"status"
		};
	

	char *device_type_string;
	enum Signal_value_type signal_value_type;	
	FE_value *node_signals_fe_value,period,*times;
	float *buffer_signals_float,*buffer_value,*channel_gains,*channel_offsets,frequency;
	int *buffer_times,channel_number,count,fread_result,i,j,number_of_samples,number_of_devices,
		number_of_signals,return_code,temp_int,		
	  channel_gain_components_number_of_derivatives[1]={0},
	  channel_gain_components_number_of_versions[1]={1},
		channel_offset_components_number_of_derivatives[1]={0},
	  channel_offset_components_number_of_versions[1]={1},
		signal_components_number_of_derivatives[1]={0},
	  signal_components_number_of_versions[1]={1},
		signal_minimum_components_number_of_derivatives[1]={0},
	  signal_minimum_components_number_of_versions[1]={1},
		signal_maximum_components_number_of_derivatives[1]={0},
	  signal_maximum_components_number_of_versions[1]={1},	
		signal_status_components_number_of_derivatives[1]={0},
	  signal_status_components_number_of_versions[1]={1};
		short int *buffer_signals_short,*node_signals_short;
	struct CM_field_information field_info;	
	struct Coordinate_system coordinate_system;		
	struct FE_field *channel_gain_field,*channel_offset_field,*signal_field,
		*signal_maximum_field,*signal_minimum_field,*signal_status_field;
	struct FE_field_component component;
	struct FE_node *device_node,*node,*node_managed;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_node) *node_manager;
#if defined(NEW_CODE) /* not using yet, may use later*/
	int index;
#endif

	ENTER(read_signal_FE_node_group);
	return_code=1;
	fread_result=0;
	buffer_signals_short=(short int *)NULL;
	buffer_times=(int *)NULL;
	buffer_signals_float=(float *)NULL;
	node_signals_short=(short int *)NULL;
	node_signals_fe_value=(FE_value *)NULL;
	node=(struct FE_node *)NULL;
	node_managed=(struct FE_node *)NULL;
	signal_field=(struct FE_field *)NULL;
	channel_gain_field=(struct FE_field *)NULL;
	channel_offset_field=(struct FE_field *)NULL;
	signal_minimum_field=(struct FE_field *)NULL;
	signal_maximum_field=(struct FE_field *)NULL;
	times=(FE_value *)NULL;
	if (input_file&&package&&node_order_info) 
	{
		fe_field_manager = get_unemap_package_FE_field_manager(package);	
		node_manager = get_unemap_package_node_manager(package);			
		number_of_devices = get_FE_node_order_info_number_of_nodes(
			node_order_info);
		set_CM_field_information(&field_info,CM_FIELD,(int *)NULL);		
		coordinate_system.type=NOT_APPLICABLE;
		if(ALLOCATE(channel_gains,float,number_of_devices)&&
			ALLOCATE(channel_offsets,float,number_of_devices))
		{
			/* read the number of signals */
			if (1==BINARY_FILE_READ((char *)&number_of_signals,sizeof(int),1,input_file))
			{
				/*???DB.  Done this way to maintain backward compatibility */
				if (number_of_signals<0)
				{
					signal_value_type=FLOAT_VALUE;
					number_of_signals= -number_of_signals;
				}
				else
				{
					signal_value_type=SHORT_INT_VALUE;
				}
				/* read the number of samples */
				if (1==BINARY_FILE_READ((char *)&number_of_samples,sizeof(int),1,
					input_file))
				{
					/* read the sampling frequency (Hz) */
					if (1==BINARY_FILE_READ((char *)&frequency,sizeof(float),1,
						input_file))
					{					
						/* read the sample times */
						if(ALLOCATE(buffer_times,int,number_of_samples))
						{
							if (number_of_samples==(int)BINARY_FILE_READ((char *)buffer_times,
								sizeof(int),number_of_samples,input_file))
							{
								/* read the signals */
								switch (signal_value_type)
								{
									case SHORT_INT_VALUE:
									{
										if(ALLOCATE(buffer_signals_short,short int,number_of_samples*
											number_of_signals))
										{
											fread_result=BINARY_FILE_READ((char *)buffer_signals_short,
												sizeof(short int),number_of_samples*number_of_signals,input_file);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"read_signal_FE_node_group. No memory for buffer_signals_short ");	
											return_code =0;
										}
									} break;
									case FLOAT_VALUE:
									{
										if(ALLOCATE(buffer_signals_float,float,number_of_samples*
											number_of_signals))
										{
											fread_result=BINARY_FILE_READ((char *)buffer_signals_float,
												sizeof(float),number_of_samples*number_of_signals,input_file);
											/* check signal values.  If it's non a valid float, set it to 0  */
											buffer_value=buffer_signals_float;
											for(i=0;i<number_of_samples*number_of_signals;i++)
											{											
												if(isnan((double)(*buffer_value)))
												{
													*buffer_value=0.0;	
													display_message(ERROR_MESSAGE,
														"read_signal_FE_node_group. signal value is not "
														"a number. Set to 0 ");											
												}
												buffer_value++;
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"read_signal_FE_node_group. No memory for buffer_signals_float ");	
											return_code =0;
										}
									} break;
								}	/* switch (signal_value_type) */
								if (fread_result==number_of_samples*number_of_signals)
								{
									/* read the device signal indicies and channel
										 characteristics */								
									count=0;
									while (return_code&&(number_of_devices>0))
									{
										/* get the channel number for this node */	
										/*???DB. If channel number zero skip, as is auxiliary
												device that is a linear combination see 
												read_binary_config_FE_node,read_text_config_FE_node*/
										device_node =get_FE_node_order_info_node(node_order_info,count);
										component.number=0;										
										component.field=package->channel_number_field;										
										if ((get_FE_nodal_int_value(device_node,&component,0,FE_NODAL_VALUE,
											&channel_number))&&(channel_number>0))
										{																				
											if ((1==BINARY_FILE_READ((char *)&temp_int,sizeof(int),1,
												input_file))&&(1==BINARY_FILE_READ((char *)&channel_offsets[count],
													sizeof(float),1,input_file))&&(1==BINARY_FILE_READ(
														(char *)&channel_gains[count],sizeof(float),1,input_file)))
											{																		
												/* allow multiple signals for each device */
												if (return_code)
												{										
													while (return_code&&(temp_int<0))
													{
														if (1==BINARY_FILE_READ((char *)&temp_int,
															sizeof(int),1,input_file))
														{										
															/*???DB.  Need to read indices even if not using */
#if defined(NEW_CODE) /* not using yet, may use later*/
															if (temp_int<0)
															{
																index= -(temp_int+1);
															}
															else
															{
																index=temp_int;
															}	
#endif												
														}
														else
														{
															display_message(ERROR_MESSAGE,
																"read_signal_FE_node_group. Error reading file -"
																" signal index");
															return_code=0;
														}
													}/* while (return_code&&(temp_int<0)) */
												}	/* if (return_code)	*/											
											}	/* if ((1==BINARY_FILE_READ( */
											else
											{
												display_message(ERROR_MESSAGE,
													"read_signal_FE_node_group. Error reading file - "
													"offsets and gains");
												return_code=0;
											}
										}
										number_of_devices--;
										count++;
									}	/* while (return_code&&(number_of_devices>0)) */
									if (return_code&&(number_of_devices>0))
									{
										display_message(ERROR_MESSAGE,
											"read_signal_FE_node_group. Missing devices");								
										return_code=0;
									}
									/* Do node stuff here */
									if(return_code)
									{								
										period = 1/frequency;									
										/* allocate memory for times, and fill in */
										if(ALLOCATE(times,FE_value,number_of_samples))
										{								
											for(j=0;j<number_of_samples;j++)
											{
												times[j] = buffer_times[j]*period;										
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"read_signal_FE_node_group. No memory for times ");	
											return_code =0;
										}
										/* allocate memory for signals to be stored at node, 
											 create signal field */
										switch (signal_value_type)
										{
											case SHORT_INT_VALUE:
											{
												if(ALLOCATE(node_signals_short,short int,number_of_samples))
												{
													if (!(signal_field=get_FE_field_manager_matched_field(
														fe_field_manager,"signal",
														GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
														/*number_of_indexed_values*/0,&field_info,
														&coordinate_system,SHORT_ARRAY_VALUE,
														/*number_of_components*/1,signal_component_names,
														/*number_of_times*/number_of_samples,/*time_value_type*/
														FE_VALUE_VALUE)))
													{
														display_message(ERROR_MESSAGE,"read_signal_FE_node_group."
															"error getting signal field");	
														return_code =0;
													}													
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"read_signal_FE_node_group. No memory for node_signals_short ");
													return_code =0;
												}
											} break;
											case FLOAT_VALUE:
											{
												if(ALLOCATE(node_signals_fe_value,float,number_of_samples))
												{
													if (!(signal_field=get_FE_field_manager_matched_field(
														fe_field_manager,"signal",
														GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
														/*number_of_indexed_values*/0,&field_info,
														&coordinate_system,FE_VALUE_ARRAY_VALUE,
														/*number_of_components*/1,signal_component_names,
														number_of_samples,/*time_value_type*/FE_VALUE_VALUE)))
													{
														display_message(ERROR_MESSAGE,"read_signal_FE_node_group."
															"error getting signal field");	
														return_code =0;
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"read_signal_FE_node_group. No memory for "
														"node_signals_fe_value");
													return_code =0;
												}
											} break;
										}	/* switch (signal_value_type) */									
										/* fill in field based time values*/
										if(signal_field)
										{
											for(j=0;j<number_of_samples;j++)
											{
												set_FE_field_time_FE_value(signal_field,j,times[j]);
											}
										}
									}							
									if(return_code)
									{										
										MANAGER_BEGIN_CACHE(FE_node)(node_manager);	
										for(i=0;i<get_FE_node_order_info_number_of_nodes(
											node_order_info);i++)
										{								
											/* copy data from buffer to node_signals. Channels are interlaced */
											/* assumes that all nodes (devices) have the same number of signals */
											for(j=0;j<number_of_samples;j++)
											{
												switch (signal_value_type)
												{
													case SHORT_INT_VALUE:
													{
														node_signals_short[j] = 
															buffer_signals_short[(j*number_of_signals)+i];
													} break;
													case FLOAT_VALUE:
													{
														node_signals_fe_value[j] = 
															buffer_signals_float[(j*number_of_signals)+i];
													} break;
												}	/* switch (signal_value_type) */
											}	/* for(j=0;j<number_of_samples) */
											/* convert short to FE_value if necessary*/
										 	node_managed =get_FE_node_order_info_node(
												node_order_info,i);
											/* create a node to work with */
											node=CREATE(FE_node)(0,(struct FE_node *)NULL);
											/*copy it from the manager */
											if (MANAGER_COPY_WITH_IDENTIFIER(FE_node,cm_node_identifier)
												(node,node_managed))
											{
												/* add channel_gain and channel_offset fields to node*/
												if (channel_gain_field=get_FE_field_manager_matched_field(
													fe_field_manager,"channel_gain",
													GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
													/*number_of_indexed_values*/0,&field_info,
													&coordinate_system,FE_VALUE_VALUE,
													/*number_of_components*/1,channel_gain_component_names,
													/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE))
												{
													if(define_FE_field_at_node(node,channel_gain_field,
														channel_gain_components_number_of_derivatives
														,channel_gain_components_number_of_versions,
														channel_gain_components_nodal_value_types))
													{	
														/* add it to the unemap package */
														set_unemap_package_channel_gain_field(package,
															channel_gain_field);
														/* set the values*/
														component.number = 0;
														component.field = channel_gain_field; 
														set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
															channel_gains[i]);
													}
													else
													{
														display_message(ERROR_MESSAGE,"read_signal_FE_node_group."
															"error defining channel_gain_field");	
														return_code =0;
													}	
												}
												else
												{
													display_message(ERROR_MESSAGE,"read_signal_FE_node_group."
														"error getting  channel_gain_field");	
													return_code =0;
												}	
												if(channel_offset_field=get_FE_field_manager_matched_field(
													fe_field_manager,"channel_offset",
													GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
													/*number_of_indexed_values*/0,&field_info,
													&coordinate_system,FE_VALUE_VALUE,
													/*number_of_components*/1,channel_offset_component_names,
													/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE))
												{ 
													if(define_FE_field_at_node(node,channel_offset_field,
														channel_offset_components_number_of_derivatives
														,channel_offset_components_number_of_versions,
														channel_offset_components_nodal_value_types))
													{	
														/* add it to the unemap package */
														set_unemap_package_channel_offset_field(package,
															channel_offset_field);
														/* set the values*/
														component.number = 0;
														component.field = channel_offset_field; 
														set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
															channel_offsets[i]);
													}
													else
													{
														display_message(ERROR_MESSAGE,"read_signal_FE_node_group."
															"error defining channel_offset_field");	
														return_code =0;
													}											
												}
												else
												{
													display_message(ERROR_MESSAGE,"read_signal_FE_node_group."
														"error getting channel_offset_field");	
													return_code =0;
												}	
												/* add signal_minimum field to node. Set to default (1.0)*/
												/* Value set up in draw_signal()*/
												if (signal_minimum_field=get_FE_field_manager_matched_field(
													fe_field_manager,"signal_minimum",
													GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
													/*number_of_indexed_values*/0,&field_info,
													&coordinate_system,FE_VALUE_VALUE,
													/*number_of_components*/1,signal_minimum_component_names,
													/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE))
												{
													if(define_FE_field_at_node(node,signal_minimum_field,
														signal_minimum_components_number_of_derivatives
														,signal_minimum_components_number_of_versions,
														signal_minimum_components_nodal_value_types))
													{	
														/* add it to the unemap package */
														set_unemap_package_signal_minimum_field(package,
															signal_minimum_field);
														/*set the signal_minimum_field and signal_maximum_field fields */
														component.number = 0;
														component.field = signal_minimum_field;												
														set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
															1.0);
													}
													else
													{
														display_message(ERROR_MESSAGE,"read_signal_FE_node_group."
															"error defining signal_minimum_field");	
														return_code =0;
													}	
												}
												else
												{
													display_message(ERROR_MESSAGE,"read_signal_FE_node_group."
														"error getting  signal_minimum_field");	
													return_code =0;
												}	
												/* add signal_maximum field to node.Set to default (0.0)*/
												/* Value set up in draw_signal()*/
												if (signal_maximum_field=get_FE_field_manager_matched_field(
													fe_field_manager,"signal_maximum",
													GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
													/*number_of_indexed_values*/0,&field_info,
													&coordinate_system,FE_VALUE_VALUE,
													/*number_of_components*/1,signal_maximum_component_names,
													/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE))
												{
													if(define_FE_field_at_node(node,signal_maximum_field,
														signal_maximum_components_number_of_derivatives
														,signal_maximum_components_number_of_versions,
														signal_maximum_components_nodal_value_types))
													{	
														/* add it to the unemap package */
														set_unemap_package_signal_maximum_field(package,
															signal_maximum_field);
														component.number = 0;
														component.field = signal_maximum_field; 
														set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
															0.0);
													}
													else
													{
														display_message(ERROR_MESSAGE,"read_signal_FE_node_group."
															"error defining signal_maximum_field");	
														return_code =0;
													}	
												}
												else
												{
													display_message(ERROR_MESSAGE,"read_signal_FE_node_group."
														"error getting  signal_maximum_field");	
													return_code =0;
												}	
												/* add signal_status field to node. The contents of this may be */
												/*overwritten if loaded in in */
												/*read_event_settings_and_signal_status_FE_node_group*/	
												if (signal_status_field=get_FE_field_manager_matched_field(
													fe_field_manager,"signal_status",
													GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
													/*number_of_indexed_values*/0,&field_info,
													&coordinate_system,STRING_VALUE,
													/*number_of_components*/1,signal_status_component_names,
													/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE))
												{ 
													if(define_FE_field_at_node(node,signal_status_field,
														signal_status_components_number_of_derivatives
														,signal_status_components_number_of_versions,
														signal_status_components_nodal_value_types))
													{	
														/* add it to the unemap package */													
														set_unemap_package_signal_status_field(package,
															signal_status_field);
														/* need to set status based upon the nodal device_type */ 
														/* status may be 'loaded over' later */ 
														get_FE_nodal_string_value(node,
															get_unemap_package_device_type_field(package),
															0,0,FE_NODAL_VALUE,&device_type_string);
														if(strcmp(device_type_string,"ELECTRODE"))
														{
															set_FE_nodal_string_value(node,signal_status_field,0,0,
																FE_NODAL_VALUE,
																"REJECTED");
														}
														else
														{
															set_FE_nodal_string_value(node,signal_status_field,0,0,
																FE_NODAL_VALUE,
																"UNDECIDED");														
														}
														DEALLOCATE(device_type_string);
													}
													else
													{
														display_message(ERROR_MESSAGE,"read_signal_FE_node_group."
															"error defining signal_status_field");	
														return_code =0;
													}					
												}
												else
												{
													display_message(ERROR_MESSAGE,"read_signal_FE_node_group."
														"error getting  signal_status_field");	
													return_code =0;
												}				
												/* add signal field to the node */
												if(define_FE_field_at_node(node,signal_field,
													signal_components_number_of_derivatives
													,signal_components_number_of_versions,
													signal_components_nodal_value_types))
												{	
													/* add it to the unemap package */
													set_unemap_package_signal_field(package,signal_field);
													/* set the values */
													component.number = 0;
													component.field = signal_field; 
													switch (signal_value_type)
													{
														case SHORT_INT_VALUE:
														{
															set_FE_nodal_short_array(node,&component,0,FE_NODAL_VALUE,
																node_signals_short,number_of_samples);
														} break;
														case FLOAT_VALUE:
														{
															set_FE_nodal_FE_value_array(node,&component,0,FE_NODAL_VALUE,
																node_signals_fe_value,number_of_samples);
														} break;
													}	/* switch (signal_value_type) */											
												}
												else
												{
													display_message(ERROR_MESSAGE,"read_signal_FE_node_group."
														" define_field_at_node failed");							
													return_code=0;
												}
												/* copy node back into the manager */
												MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)
													(node_managed,node,node_manager);
											}  	/* if (MANAGER_COPY_WITH_IDENTIFIER */
											else
											{
												display_message(ERROR_MESSAGE,
													"read_signal_FE_node_group.  MANAGER_COPY_WITH_IDENTIFIER failed");
												return_code=0;
											}
											/* destroy the working copy */
											DESTROY(FE_node)(&node);
										}
										MANAGER_END_CACHE(FE_node)(node_manager);								
									}												
								}/*	if (fread_result==number_of_samples*number_of_signals) */
								else
								{
									display_message(ERROR_MESSAGE,
										"read_signal_FE_node_group. Error reading file - signals");
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_signal_FE_node_group. Error reading file - times");
								return_code=0;
							}				
						}
						else
						{	
							display_message(ERROR_MESSAGE,
								"read_signal_FE_node_group. No memory for buffer_times");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_signal_FE_node_group. Error reading file - frequency");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_signal_FE_node_group. Error reading file - number of samples");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_signal_FE_node_group. Error reading file - number of signals");		
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_signal_FE_node_group. Out of memory for channel_gain,offset");		
			return_code=0;
		}
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"read_signal_FE_node_group. Invalid arguments");
	}	
	/* clean up */
	DEALLOCATE(buffer_signals_short);
	DEALLOCATE(buffer_times);
	DEALLOCATE(buffer_signals_float);
	DEALLOCATE(node_signals_fe_value);
	DEALLOCATE(node_signals_short);	
	DEALLOCATE(times);	
	DEALLOCATE(channel_offsets);	
	DEALLOCATE(channel_gains);
				
	LEAVE;

	return(return_code);
} /* read_signal_FE_node_group */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
static int read_event_settings_and_signal_status_FE_node_group(FILE *input_file,
	struct Unemap_package *package,
	struct FE_node_order_info *node_order_info)
/*******************************************************************************
LAST MODIFIED :  11 October 1999

DESCRIPTION :
If they are present, reads the event settings, and the signal_status,
signal_minimum,signal_maximum from the signal file.
Puts the signal_status,signal_minimum,signal_maximum into the rig_node_group.
Note that signal_status,signal_minimum,signal_maximum have already been set up
by read_signal_FE_node_group.
Performs the same loading functions as analysis_read_signal_file,
but using the rig_node group, and doens't yet do anything with the event 
setting information.
Must call after read_signal_FE_node_group
==============================================================================*/
{	
	char calculate_events;
	enum Edit_order edit_order;
	enum Event_detection_algorithm detection;
	enum Event_signal_status signal_status,event_status;
	enum Datum_type datum_type;
	enum Signal_order signal_order;
	float signal_maximum,signal_minimum;
	int datum,end_search_interval,event_number,event_time,i,number_of_devices,
		number_of_events,potential_time,minimum_separation,start_search_interval,
		threshold,return_code;
	struct FE_node *node,*node_managed;
	struct FE_field *signal_maximum_field,*signal_minimum_field,*signal_status_field;
	struct FE_field_component component;
	struct MANAGER(FE_node) *node_manager;

	ENTER(read_event_settings_and_signal_status_FE_node_group);
	node=(struct FE_node *)NULL;
	node_managed=(struct FE_node *)NULL;
	node_manager=(struct MANAGER(FE_node) *)NULL;
	signal_maximum_field=(struct FE_field *)NULL;
	signal_minimum_field=(struct FE_field *)NULL;
	signal_status_field=(struct FE_field *)NULL;
	if(input_file&&package&&node_order_info)
	{
		return_code=1;
		if ((1==BINARY_FILE_READ((char *)&datum,sizeof(int),1,input_file))&&
			(1==BINARY_FILE_READ((char *)&calculate_events,sizeof(char),1,
			input_file))&&(1==BINARY_FILE_READ((char *)&detection,
			sizeof(enum Event_detection_algorithm),1,input_file))&&
			(1==BINARY_FILE_READ((char *)&event_number,sizeof(int),1,input_file))&&
			(1==BINARY_FILE_READ((char *)&number_of_events,sizeof(int),1,
			input_file))&&(1==BINARY_FILE_READ((char *)&potential_time,sizeof(int),
			1,input_file))&&(1==BINARY_FILE_READ((char *)&minimum_separation,
			sizeof(int),1,input_file))&&(1==BINARY_FILE_READ((char *)&threshold,
			sizeof(int),1,input_file))&&(1==BINARY_FILE_READ((char *)&datum_type,
			sizeof(enum Datum_type),1,input_file))&&(1==BINARY_FILE_READ(
			(char *)&edit_order,sizeof(enum Edit_order),1,input_file))&&
			(1==BINARY_FILE_READ((char *)&signal_order,sizeof(enum Signal_order),1,
			input_file))&&(1==BINARY_FILE_READ((char *)&start_search_interval,
			sizeof(int),1,input_file))&&(1==BINARY_FILE_READ(
			(char *)&end_search_interval,sizeof(int),1,input_file)))
		{	
			node_manager = get_unemap_package_node_manager(package);	
			signal_maximum_field=
				get_unemap_package_signal_maximum_field(package);	
			signal_minimum_field=
				get_unemap_package_signal_minimum_field(package);
			signal_status_field=
				get_unemap_package_signal_status_field(package);	
			number_of_devices = 
				get_FE_node_order_info_number_of_nodes(node_order_info);
			/* for each signal read the status, range and events */
			if(((number_of_devices)>0)&&node_manager&&signal_maximum_field&&
				signal_minimum_field&&signal_status_field)
			{
				i=0;
				return_code=1;	
				MANAGER_BEGIN_CACHE(FE_node)(node_manager);
				while (return_code&&(i<number_of_devices))
				{				
					/* read the status and range */
					if ((1==BINARY_FILE_READ((char *)&(signal_status),
						sizeof(enum Event_signal_status),1,input_file))&&
						(1==BINARY_FILE_READ((char *)&(signal_minimum),
						sizeof(float),1,input_file))&&(1==BINARY_FILE_READ(
						(char *)&(signal_maximum),sizeof(float),1,input_file)))
					{
						/* read the events */
						if (1==BINARY_FILE_READ((char *)&number_of_events,sizeof(int),1,
							input_file))
						{						
							while (return_code&&(number_of_events>0))
							{
								if ((1==BINARY_FILE_READ((char *)&(event_time),sizeof(int),1,
									input_file))&&(1==BINARY_FILE_READ((char *)&(event_number),
										sizeof(int),1,input_file))&&
									(1==BINARY_FILE_READ((char *)&(event_status),
										sizeof(enum Event_signal_status),1,input_file)))
								{
									number_of_events--;;
								}
								else
								{
									return_code=0;
									display_message(ERROR_MESSAGE,
										"read_event_settings_and_signal_status_FE_node_group."
										"  Error reading event");
								}
							}
						}
						else
						{
							return_code=0;
							display_message(ERROR_MESSAGE,
								"read_event_settings_and_signal_status_FE_node_group."
								"  Error reading number of events");
						}
					}
					else
					{
						return_code=0;
						display_message(ERROR_MESSAGE,
							"read_event_settings_and_signal_status_FE_node_group."
							"  Error reading signal range/status");
					}
					/* put (some of!) the read info into the node */
					node_managed =get_FE_node_order_info_node(node_order_info,i);
					/* create a node to work with */
					node=CREATE(FE_node)(0,(struct FE_node *)NULL);
					if (MANAGER_COPY_WITH_IDENTIFIER(FE_node,cm_node_identifier)
						(node,node_managed))
					{
						component.number=0;
						component.field=signal_minimum_field;
						/* fields have already been defined at the node in read_signal_FE_node_group*/
						set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
							signal_minimum);
						component.field=signal_maximum_field;
						set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
							signal_maximum);
						switch(signal_status)
						{
							case ACCEPTED:
							{
								set_FE_nodal_string_value(node,signal_status_field,0,0,FE_NODAL_VALUE,
									"ACCEPTED");
							}break;	
							case REJECTED:
							{
								set_FE_nodal_string_value(node,signal_status_field,0,0,FE_NODAL_VALUE,
									"REJECTED");
							}break;	
							case UNDECIDED:						
							{
								set_FE_nodal_string_value(node,signal_status_field,0,0,FE_NODAL_VALUE,
									"UNDECIDED");
							}break;		
							default:	
							{								
								display_message(ERROR_MESSAGE,
									"read_event_settings_and_signal_status_FE_node_group."
									"  incorrect signal_status");
								return_code=0;							
							}break;					
						}
						/* copy node back into the manager */
						MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)
							(node_managed,node,node_manager);
					}  	/* if (MANAGER_COPY_WITH_IDENTIFIER */
					else
					{
						display_message(ERROR_MESSAGE,
							"read_event_settings_and_signal_status_FE_node_group."
							"  MANAGER_COPY_WITH_IDENTIFIER failed");
						return_code=0;
					}
					/* destroy the working copy */
					DESTROY(FE_node)(&node);
					i++;
				}
				MANAGER_END_CACHE(FE_node)(node_manager);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_event_settings_and_signal_status_FE_node_group."
							"  node or field info incorrect");
				return_code=0;
			}
		}/* if ((1==BINARY_FILE_READ((char *)&datum,  <else nothing to do>*/
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_event_settings_and_signal_status_FE_node_group. Invalid arguments");
	}
	LEAVE;
	return(return_code);
}/*read_event_settings_and_signal_status_FE_node_group. */
#endif /* defined (UNEMAP_USE_NODES) */

/*
Global functions
----------------
*/
#if defined (UNEMAP_USE_NODES)
DECLARE_OBJECT_FUNCTIONS(Draw_package)
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct Draw_package *CREATE(Draw_package)(void)
/*******************************************************************************
LAST MODIFIED :  9 July 1999

DESCRIPTION :
Create a Draw_package, set all it's fields to NULL.
==============================================================================*/
{
	struct Draw_package *package;

	ENTER(CREATE(Draw_package));	
	if (ALLOCATE(package,struct Draw_package,1))
	{
		/* fields of the rig_nodes */
		package->device_name_field=(struct FE_field *)NULL;		
		package->device_type_field=(struct FE_field *)NULL;	
		package->channel_number_field=(struct FE_field *)NULL;
		package->signal_field=(struct FE_field *)NULL;		
		package->signal_status_field=(struct FE_field *)NULL;	
		package->signal_minimum_field=(struct FE_field *)NULL;
		package->signal_maximum_field=(struct FE_field *)NULL;
		package->channel_gain_field=(struct FE_field *)NULL;
		package->channel_offset_field=(struct FE_field *)NULL;
		package->access_count=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Draw_package).  Could not allocate memory");
		DEALLOCATE(package);
	}
	LEAVE;

	return (package);
} /* CREATE(Draw_package) */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int DESTROY(Draw_package)(struct Draw_package **package_address)
/*******************************************************************************
LAST MODIFIED : 9 July 1999

DESCRIPTION :
Frees the memory for the Draw_package node  and 
sets <*package_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct Draw_package *package;

	ENTER(DESTROY(Draw_package));
	if ((package_address)&&(package=*package_address))
	{	
		DEACCESS(FE_field)(&(package->device_name_field));	
		DEACCESS(FE_field)(&(package->device_type_field));
		DEACCESS(FE_field)(&(package->channel_number_field)); 
		DEACCESS(FE_field)(&(package->signal_field));	
		DEACCESS(FE_field)(&(package->signal_status_field)); 
		DEACCESS(FE_field)(&(package->signal_minimum_field));
		DEACCESS(FE_field)(&(package->signal_maximum_field));
		DEACCESS(FE_field)(&(package->channel_gain_field)); 
		DEACCESS(FE_field)(&(package->channel_offset_field)); 
		DEALLOCATE(*package_address);
		return_code=1;	
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Draw_package) */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_device_name_field(
	struct Draw_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{	
	struct FE_field *device_name_field;

	ENTER(get_Draw_package_device_name_field);
	if (package)
	{	
		device_name_field=package->device_name_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Draw_package_device_name_field.  Invalid argument");
		device_name_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (device_name_field);
} /* get_Draw_package_device_name_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Draw_package_device_name_field(struct Draw_package *package,
	struct FE_field *device_name_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the draw package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Draw_package_device_name_field);
	if (package)
	{
		return_code=1;	
		REACCESS(FE_field)(&(package->device_name_field),device_name_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Draw_package_device_name_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Draw_package_device_name__field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_device_type_field(
	struct Draw_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Gets the field of the unemap package.
==============================================================================*/
{	
	struct FE_field *device_type_field;

	ENTER(get_Draw_package_device_type_field);
	if (package)
	{	
		device_type_field=package->device_type_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Draw_package_device_type_field.  Invalid argument");
		device_type_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (device_type_field);
} /* get_Draw_package_device_type_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Draw_package_device_type_field(struct Draw_package *package,
	struct FE_field *device_type_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the draw package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Draw_package_device_type_field);
	if (package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->device_type_field),device_type_field);	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Draw_package_device_type_field.  Invalid argument");
		return_code =0;
	}
	LEAVE;

	return (return_code);
} /* set_Draw_package_device_type_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_channel_number_field(
	struct Draw_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Gets the field of the unemap package.
==============================================================================*/
{	
	struct FE_field *channel_number_field;

	ENTER(get_Draw_package_channel_number_field);
	if (package)
	{	
		channel_number_field=package->channel_number_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Draw_package_channel_number_field.  Invalid argument");
		channel_number_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (channel_number_field);
} /* get_Draw_package_channel_number_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Draw_package_channel_number_field(struct Draw_package *package,
	struct FE_field *channel_number_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the draw package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Draw_package_channel_number_field);
	if (package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->channel_number_field),channel_number_field);		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Draw_package_channel_number_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Draw_package_channel_number_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_signal_field(struct Draw_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Gets the field of the unemap package.
==============================================================================*/
{	
	struct FE_field *signal_field;

	ENTER(get_Draw_package_signal_field);
	if (package)
	{	
		signal_field=package->signal_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Draw_package_signal_field.  Invalid argument");
		signal_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (signal_field);
} /* get_Draw_package_signal_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Draw_package_signal_field(struct Draw_package *package,
	struct FE_field *signal_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the draw package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Draw_package_signal_field);
	if (package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->signal_field),signal_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Draw_package_signal_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Draw_package_signal_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_signal_status_field(struct Draw_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Gets the field of the unemap package.
==============================================================================*/
{	
	struct FE_field *signal_status_field;

	ENTER(get_Draw_package_signal_status_field);
	if (package)
	{	
		signal_status_field=package->signal_status_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Draw_package_signal_status_field.  Invalid argument");
		signal_status_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (signal_status_field);
} /* get_Draw_package_signal_status_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Draw_package_signal_status_field(struct Draw_package *package,
	struct FE_field *signal_status_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the draw package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Draw_package_signal_status_field);
	if (package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->signal_status_field),signal_status_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Draw_package_signal_status_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Draw_package_signal_status_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_signal_minimum_field(struct Draw_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Gets the field of the unemap package.
==============================================================================*/
{	
	struct FE_field *signal_minimum_field;

	ENTER(get_Draw_package_signal_minimum_field);
	if (package)
	{	
		signal_minimum_field=package->signal_minimum_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Draw_package_signal_minimum_field.  Invalid argument");
		signal_minimum_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (signal_minimum_field);
} /* get_Draw_package_signal_minimum_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Draw_package_signal_minimum_field(struct Draw_package *package,
	struct FE_field *signal_minimum_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the draw package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Draw_package_signal_minimum_field);
	if (package)
	{
		return_code=1;	
		REACCESS(FE_field)(&(package->signal_minimum_field),signal_minimum_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Draw_package_signal_minimum_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Draw_package_signal_minimum_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_signal_maximum_field(struct Draw_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Gets the field of the unemap package.
==============================================================================*/
{	
	struct FE_field *signal_maximum_field;

	ENTER(get_Draw_package_signal_maximum_field);
	if (package)
	{	
		signal_maximum_field=package->signal_maximum_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Draw_package_signal_maximum_field.  Invalid argument");
		signal_maximum_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (signal_maximum_field);
} /* get_Draw_package_signal_maximum_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Draw_package_signal_maximum_field(struct Draw_package *package,
	struct FE_field *signal_maximum_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the draw package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Draw_package_signal_maximum_field);
	if (package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->signal_maximum_field),signal_maximum_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Draw_package_signal_maximum_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Draw_package_signal_maximum_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_channel_offset_field(
	struct Draw_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Gets the field of the unemap package.
==============================================================================*/
{	
	struct FE_field *channel_offset_field;

	ENTER(get_Draw_package_channel_offset_field);
	if (package)
	{	
		channel_offset_field=package->channel_offset_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Draw_package_channel_offset_field.  Invalid argument");
		channel_offset_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (channel_offset_field);
} /* get_Draw_package_channel_offset_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Draw_package_channel_offset_field(struct Draw_package *package,
	struct FE_field *channel_offset_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Draw_package_channel_offset_field);
	if (package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->channel_offset_field),channel_offset_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Draw_package_channel_offset_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Draw_package_channel_offset_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_channel_gain_field(
	struct Draw_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Gets the field of the unemap package.
==============================================================================*/
{	
	struct FE_field *channel_gain_field;

	ENTER(get_Draw_package_fields);
	if (package)
	{	
		channel_gain_field=package->channel_gain_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Draw_package_channel_gain_field.  Invalid argument");
		channel_gain_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (channel_gain_field);
} /* get_Draw_package_channel_gain_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Draw_package_channel_gain_field(struct Draw_package *package,
	struct FE_field *channel_gain_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Draw_package_channel_gain_field);
	if (package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->channel_gain_field),channel_gain_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Draw_package_channel_gain_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Draw_package_channel_gain_field */
#endif /* defined (UNEMAP_USE_NODES) */

int extract_signal_information(struct FE_node *device_node,
	struct Draw_package *draw_package,struct Device *device,
	int signal_number,int first_data,int last_data,
	int *number_of_signals_address,int *number_of_values_address,
	float **times_address,float **values_address,
	enum Event_signal_status **status_address,char **name_address,
	int *highlight_address,float *signal_minimum_address,
	float *signal_maximum_address)
/*******************************************************************************
LAST MODIFIED : 13 August 1999

DESCRIPTION :
Extracts the specified signal information.  The specification arguments are:
- <device_node>, <draw_package>, <device> identify where the information is
	stored. Either supply a <device>, with <device_node>, <draw_package> NULL
	to extract info from <device>, or supply a <device_node> and a <draw_package>
	with <device> = NULL, to extract info from <device_node>.
- <signal_number> specifies which signal (zero indicates all)
- <first_data>, <last_data> specify the part of the signal required.  If
	<first_data> is greater than <last_data> then return the whole signal
The extraction arguments are:
- <number_of_signals_address> if not NULL, <*number_of_signals_address> is set
	to the number of signals extracted
- <number_of_values_address> if not NULL, <*number_of_values_address> is set to
	the number of values extracted for each signal
- <times_address> if not NULL, an array with number of values entries is
	allocated, filled in with the times and assigned to <*times_address>
- <values_address> if not NULL, an array with (number of signals)*(number of
	values) entries is allocated, filled in with the values and assigned to
	<*values_address>
- <status_address> if not NULL, an array with number of signals entries is
	allocated, filled in with the signal statuses and assigned to
	<*status_address>
- <name_address> if not NULL, a copy of the name is made and assigned to
	<*name_address>
- <highlight_address> if not NULL <*highlight_address> is set to zero if the
	signals are not highlighted and a non-zero if they are highlighted
- <signal_minimum_address> if not NULL <*signal_minimum_address> is set to
	the minimum value to be displayed (not necessarily the minimum of the values)
- <signal_maximum_address> if not NULL <*signal_maximum_address> is set to
	the maximum value to be displayed (not necessarily the maximum of the values)
==============================================================================*/
{
	char *name=(char *)NULL;
#if defined (UNEMAP_USE_NODES)
	char *signal_status_str=(char *)NULL;
#endif /* defined (UNEMAP_USE_NODES) */
	enum Event_signal_status *signals_status=(enum Event_signal_status *)NULL;
	float channel_offset,channel_gain,signal_minimum,signal_maximum;
	float	*signal_value=(float *)NULL;
	float	*signals_values=(float *)NULL;
	float	*times=(float *)NULL;
	int first_data_local,highlight,last_data_local,number_of_signals,
		number_of_values,return_code;

	ENTER(extract_signal_information);
	return_code=0;
	if (
#if defined (UNEMAP_USE_NODES)
		((device_node&&draw_package)&&(!device))||
#endif /* defined (UNEMAP_USE_NODES) */
		(device&&((!device_node)&&(!draw_package))))
	{
#if defined (UNEMAP_USE_NODES)
		if (device)
		{
#endif /* defined (UNEMAP_USE_NODES) */
			float buffer_frequency,*data_value_float;
			float	*electrode_coefficients=(float *)NULL;
			int buffer_offset,i,j,k,length,linear_combination,number_of_electrodes;
			short int *data_value_short_int=(short int *)NULL;
			struct Device **electrodes=(struct Device **)NULL;
			struct Signal *signal=(struct Signal *)NULL;
			struct Signal_buffer *buffer=(struct Signal_buffer *)NULL;

			if ((signal=device->signal)&&(buffer=signal->buffer))
			{
				linear_combination=0;
				number_of_signals=1;
				return_code=1;
				while (return_code&&(signal=signal->next))
				{
					/* only handle devices with signals all in the same buffer */
					if (buffer==signal->buffer)
					{
						number_of_signals++;
					}
					else
					{
						return_code=0;
					}
				}
			}
			else
			{
				if ((device->description)&&(AUXILIARY==device->description->type)&&
					(0<(number_of_electrodes=(device->description->properties).
					auxiliary.number_of_electrodes)))
				{
					/* auxiliary device that is a linear combination of electrodes */
					linear_combination=1;
					electrodes=(device->description->properties).auxiliary.electrodes;
					electrode_coefficients=(device->description->properties).auxiliary.
						electrode_coefficients;
					/* check that electodes have same number of signals and that all
						signals are from the same buffer */
					if ((signal=(*electrodes)->signal)&&(buffer=signal->buffer))
					{
						number_of_signals=1;
						return_code=1;
						while (return_code&&(signal=signal->next))
						{
							if (buffer==signal->buffer)
							{
								number_of_signals++;
							}
							else
							{
								return_code=0;
							}
						}
						if (return_code)
						{
							i=1;
							while (return_code&&(i<number_of_electrodes))
							{
								if (signal=electrodes[i]->signal)
								{
									j=1;
									while (return_code&&(signal=signal->next))
									{
										if (buffer==signal->buffer)
										{
											j++;
										}
										else
										{
											return_code=0;
										}
									}
									if (j!=number_of_signals)
									{
										return_code=0;
									}
								}
								i++;
							}
						}
					}
				}
			}
			if (return_code)
			{
				if (0!=signal_number)
				{
					if ((0<signal_number)&&(signal_number<=number_of_signals))
					{
						number_of_signals=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"extract_signal_information.  Invalid signal_number");
						return_code=0;
					}
				}
				if (first_data>last_data)
				{
					first_data_local=buffer->start;
					last_data_local=buffer->end;
				}
				else
				{
					first_data_local=first_data;
					last_data_local=last_data;
				}
				if (!((0<=first_data_local)&&(first_data_local<=last_data_local)&&
					(last_data_local<buffer->number_of_samples)))
				{
					display_message(ERROR_MESSAGE,
				"extract_signal_information.  Invalid signal range.  Device.  %d %d %d",
						first_data_local,last_data_local,buffer->number_of_samples);
					return_code=0;
				}
				if (return_code)
				{
					number_of_values=last_data_local-first_data_local+1;	
					buffer_offset=buffer->number_of_signals;	
					buffer_frequency=buffer->frequency;
					if (times_address)
					{
						if (ALLOCATE(times,float,number_of_values))
						{
							/* fill in times */
							for (i=0;i<number_of_values;i++)
							{	
								times[i]=(buffer->times[first_data_local+i])/buffer_frequency;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"extract_signal_information.  Could not allocate times");
							return_code=0;
						}
					}
					else
					{
						times=(float *)NULL;
					}
					if (return_code)
					{
						if (values_address)
						{
							if (ALLOCATE(signals_values,float,
								number_of_signals*number_of_values))
							{
								/* fill in signal values */
								if (linear_combination)
								{
									signal_value=signals_values;
									for (i=number_of_signals*number_of_values;i>0;i--)
									{
										*signal_value=(float)0;
										signal_value++;
									}
									for (i=0;i<number_of_electrodes;i++)
									{
										signal_value=signals_values;
										signal=electrodes[i]->signal;
										channel_offset=electrodes[i]->channel->offset;
										channel_gain=electrodes[i]->channel->gain;
										channel_gain *= electrode_coefficients[i];
										switch (buffer->value_type)
										{													
											case SHORT_INT_VALUE:
											{
												k=0;
												while (signal)
												{
													k++;
													if ((0==signal_number)||(k==signal_number))
													{
														data_value_short_int=
															(buffer->signals.short_int_values)+
															(first_data_local*buffer_offset+signal->index);
														for (j=number_of_values;j>0;j--)
														{
															*signal_value +=
																channel_gain*((float)(*data_value_short_int)-
																channel_offset);
															data_value_short_int += buffer_offset;
															signal_value++;
														}
													}
													signal=signal->next;
												}
											} break;
											case FLOAT_VALUE:
											{
												k=0;
												while (signal)
												{
													k++;
													if ((0==signal_number)||(k==signal_number))
													{
														data_value_float=(buffer->signals.float_values)+
															(first_data_local*buffer_offset+signal->index);
														for (j=number_of_values;j>0;j--)
														{
															*signal_value +=
																channel_gain*((float)(*data_value_float)-
																channel_offset);
															data_value_float += buffer_offset;
															signal_value++;
														}
													}
													signal=signal->next;
												}
											} break;
										}
									}
								}
								else
								{
									signal_value=signals_values;
									signal=device->signal;
									channel_offset=device->channel->offset;
									channel_gain=device->channel->gain;		
									switch (buffer->value_type)
									{													
										case SHORT_INT_VALUE:
										{
											k=0;
											while (signal)
											{
												k++;
												if ((0==signal_number)||(k==signal_number))
												{
													data_value_short_int=
														(buffer->signals.short_int_values)+
														(first_data_local*buffer_offset+signal->index);
													for (i=number_of_values;i>0;i--)
													{
														*signal_value=
															channel_gain*((float)(*data_value_short_int)-
															channel_offset);
														data_value_short_int += buffer_offset;
														signal_value++;
													}											
												}											
												signal=signal->next;
											}
										} break;
										case FLOAT_VALUE:
										{
											k=0;
											while (signal)
											{
												k++;
												if ((0==signal_number)||(k==signal_number))
												{
													data_value_float=(buffer->signals.float_values)+
														(first_data_local*buffer_offset+signal->index);
													for (i=number_of_values;i>0;i--)
													{
														*signal_value=
															channel_gain*((float)(*data_value_float)-
															channel_offset);
														data_value_float += buffer_offset;
														signal_value++;
													}
												}
												signal=signal->next;
											}
										} break;
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"extract_signal_information.  Could not allocate values");
								return_code=0;
							}
						}
						else
						{
							signals_values=(float *)NULL;
						}
						if (return_code)
						{
							if (status_address)
							{
								if (ALLOCATE(signals_status,enum Event_signal_status,
									number_of_signals))
								{
									/* fill in statuses */
									if (linear_combination)
									{
										for (i=0;i<number_of_signals;i++)
										{
											signals_status[i]=REJECTED;
										}
									}
									else
									{
										signal=device->signal;
										i=0;
										k=0;
										while (signal)
										{
											k++;
											if ((0==signal_number)||(k==signal_number))
											{
												signals_status[i]=signal->status;
												i++;
											}
											signal=signal->next;
										}
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"extract_signal_information.  Could not allocate statuses");
									return_code=0;
								}
							}
							else
							{
								signals_status=(enum Event_signal_status *)NULL;
							}
							if (return_code)
							{
								if (name_address)
								{
									if (device->description->name)
									{
										length=strlen(device->description->name)+1;
										if (ALLOCATE(name,char,length))
										{
											strcpy(name,device->description->name);
										}
										else
										{	
											display_message(WARNING_MESSAGE,
												"extract_signal_information.  Could not allocate name");
											return_code=0;
										}
									}
									else
									{
										name=(char *)NULL;
									}
								}
								else
								{
									name=(char *)NULL;
								}
								if (!return_code)
								{
									DEALLOCATE(signals_status);
									DEALLOCATE(signals_values);
									DEALLOCATE(times);
								}
							}
							else
							{
								DEALLOCATE(signals_values);
								DEALLOCATE(times);
							}
						}
						else
						{
							DEALLOCATE(times);
						}
					}
					if (return_code)
					{
						highlight=device->highlight;
						signal_minimum=device->signal_minimum;
						signal_maximum=device->signal_maximum;
					}
				}
			}
			else
			{			
				display_message(ERROR_MESSAGE,
					"extract_signal_information.  Missing signal(s) for device");
				return_code=0;
 			}
#if defined (UNEMAP_USE_NODES)
		}
		else
		{
			enum Value_type value_type;
			FE_value *FE_value_signal_data=(FE_value *)NULL;
			FE_value 	time;
			int i,number_of_nodal_values;
			short int *short_signal_data=(short int *)NULL;
			struct FE_field_component component;						
			
			return_code=1;
			/*???DB.  Currently only storing one signal at node */
			number_of_signals=1;
			if (0!=signal_number)
			{
				if ((0<signal_number)&&(signal_number<=number_of_signals))
				{
					number_of_signals=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"extract_signal_information.  Invalid signal_number");
					return_code=0;
				}
			}
			component.field=draw_package->signal_field;
			component.number=0;
			number_of_nodal_values=get_FE_nodal_array_number_of_elements(device_node,
				&component,0,FE_NODAL_VALUE);
			value_type= get_FE_nodal_value_type(device_node,&component,0);
			if((number_of_nodal_values>0)&&(value_type!=UNKNOWN_VALUE))
			{
				return_code=1;
			}
			else
			{			
				return_code=0;
			}
			if (return_code)
			{
				if (first_data>last_data)
				{
					first_data_local=0;
					last_data_local=number_of_nodal_values-1;
				}
				else
				{
					first_data_local=first_data;
					last_data_local=last_data;
				}
				if ((0<=first_data_local)&&(first_data_local<=last_data_local)&&
					(last_data_local<number_of_nodal_values))
				{
					number_of_values=last_data_local-first_data_local+1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
					"extract_signal_information.  Invalid signal range.  Node.  %d %d %d",
						first_data_local,last_data_local,number_of_nodal_values);
					return_code=0;
				}
			}
			if (return_code)
			{
				if (times_address)
				{
					if (ALLOCATE(times,float,number_of_values))
					{
						/* fill in times */
							/*???DB.  Can they be got all at once? */
						i=0;
						while (return_code&&(i<number_of_values))
						{	
							if (get_FE_field_time_FE_value(draw_package->signal_field,
								first_data_local+i,&time))
							{
								times[i]=(float)time;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"extract_signal_information.  Could not get time");
								DEALLOCATE(times);
								return_code=0;
							}
							i++;
						} 						
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"extract_signal_information.  Could not allocate times");
						return_code=0;
					}
				}
				else
				{
					times=(float *)NULL;
				}
				if (return_code)
				{
					if (values_address)
					{
						/* get the channel gain and offset */
						component.field=draw_package->channel_offset_field;
						component.number=0;	
						if (get_FE_nodal_FE_value_value(device_node,&component,0,FE_NODAL_VALUE,
							&channel_offset))
						{
							component.field=draw_package->channel_gain_field;
							if (get_FE_nodal_FE_value_value(device_node,&component,0,FE_NODAL_VALUE,
								&channel_gain))
							{
								if (ALLOCATE(signals_values,float,
									number_of_signals*number_of_values))
								{
									component.field=draw_package->signal_field;
									component.number=0;
									switch (value_type)
									{
										case FE_VALUE_ARRAY_VALUE:
										{
											if (ALLOCATE(FE_value_signal_data,FE_value,
												number_of_nodal_values))
											{
												if (get_FE_nodal_FE_value_array(device_node,
													&component,0,FE_NODAL_VALUE,FE_value_signal_data,
													number_of_nodal_values))
												{
													for (i=0;i<number_of_values;i++)
													{
														signals_values[i]=channel_gain*
															((float)FE_value_signal_data[first_data_local+i]-
															channel_offset);
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
		"extract_signal_information.  Could not get_FE_nodal_FE_value_array");
													return_code=0;
												}
												DEALLOCATE(FE_value_signal_data);
											}
											else
											{
												display_message(ERROR_MESSAGE,
				"extract_signal_information.  Could not allocate FE_value_signal_data");
												return_code=0;
											}
										} break;
										case SHORT_ARRAY_VALUE:
										{
											if (ALLOCATE(short_signal_data,short int,
												number_of_nodal_values))
											{
												if (get_FE_nodal_short_array(device_node,
													&component,0,FE_NODAL_VALUE,short_signal_data,
													number_of_nodal_values))
												{
													for (i=0;i<number_of_values;i++)
													{
														signals_values[i]=channel_gain*
															((float)short_signal_data[first_data_local+i]-
															channel_offset);
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
			"extract_signal_information.  Could not get_FE_nodal_short_array");
													return_code=0;
												}
												DEALLOCATE(short_signal_data);
											}
											else
											{
												display_message(ERROR_MESSAGE,
					"extract_signal_information.  Could not allocate short_signal_data");
												return_code=0;
											}
										} break;
										default:
										{
											display_message(ERROR_MESSAGE,
												"extract_signal_information.  Invalid value type");
											return_code=0;
										} break;
									}
									if (!return_code)
									{
										DEALLOCATE(signals_values);
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"extract_signal_information.  Could not allocate values");
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"extract_signal_information.  Could not get channel gain");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"extract_signal_information.  Could not get channel offset");
							return_code=0;
						}
					}
					else
					{
						signals_values=(float *)NULL;
					}
					if (return_code)
					{
						if (status_address)
						{
							if (ALLOCATE(signals_status,enum Event_signal_status,
								number_of_signals))
							{									
								/* fill in statuses */							
								for (i=0;i<number_of_signals;i++)
								{
									if (get_FE_nodal_string_value(device_node,
										draw_package->signal_status_field,/*component_number*/0,
										/*version*/0,FE_NODAL_VALUE,&signal_status_str))
									{
										if(!(strcmp("ACCEPTED",signal_status_str)))
										{
											signals_status[i]=ACCEPTED;										
										}
										else if(!(strcmp("REJECTED",signal_status_str)))
										{
											signals_status[i]=REJECTED;											
										}
										else if(!(strcmp("UNDECIDED",signal_status_str)))
										{
											signals_status[i]=UNDECIDED;										
										}
										else
										{
											signals_status[i]=UNDECIDED;										
										}									
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"extract_signal_information.  Could not get signal_status");
										return_code=0;
										signals_status[i]=UNDECIDED;
									}								
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"extract_signal_information.  Could not allocate statuses");
								return_code=0;
							}
						}
						else
						{
							signals_status=(enum Event_signal_status *)NULL;
						}
						if (return_code)
						{
							if (name_address)
							{
								if (!get_FE_nodal_string_value(device_node,
									draw_package->device_name_field,/*component_number*/0,
									/*version*/0,FE_NODAL_VALUE,&name))
								{
									display_message(ERROR_MESSAGE,
										"extract_signal_information.  Could not get name");
									return_code=0;
								}
							}
							else
							{
								name=(char *)NULL;
							}
							if (!return_code)
							{
								DEALLOCATE(signals_status);
								DEALLOCATE(signals_values);
								DEALLOCATE(times);
							}
						}
						else
						{
							DEALLOCATE(signals_values);
							DEALLOCATE(times);
						}
					}
					else
					{
						DEALLOCATE(times);
					}
				}
				if (return_code)
				{
					/*???DB.  In future get highlight from active group */
					highlight=0;
					component.number=0;										
					component.field=draw_package->signal_minimum_field;
					get_FE_nodal_FE_value_value(device_node,&component,0,FE_NODAL_VALUE,
						&signal_minimum);
					component.field=draw_package->signal_maximum_field;
					get_FE_nodal_FE_value_value(device_node,&component,0,FE_NODAL_VALUE,
						&signal_maximum);					
				}
			}
		}
#endif /* defined (UNEMAP_USE_NODES) */
		if (return_code)
		{
			if (number_of_signals_address)
			{
				*number_of_signals_address=number_of_signals;
			}
			if (number_of_values_address)
			{
				*number_of_values_address=number_of_values;
			}
			if (times_address)
			{
				*times_address=times;
			}
			if (values_address)
			{
				*values_address=signals_values;
			}
			if (status_address)
			{
				*status_address=signals_status;
			}
			if (name_address)
			{
				*name_address=name;
			}
			if (highlight_address)
			{
				*highlight_address=highlight;
			}
			if (signal_minimum_address)
			{
				*signal_minimum_address=signal_minimum;
			}
			if (signal_maximum_address)
			{
				*signal_maximum_address=signal_maximum;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"extract_signal_information.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* extract_signal_information */

#if defined (UNEMAP_USE_NODES)
struct GROUP(FE_node) *file_read_signal_FE_node_group(char *file_name,
	struct Unemap_package *unemap_package)
/*******************************************************************************
LAST MODIFIED : 2 August 1999

DESCRIPTION :
Reads and returns node group from a signal file. Signal file includes the
configuration info.
==============================================================================*/
{
	enum Region_type region_type;	
	FILE *input_file;
	struct FE_node_order_info *node_order_info;
	struct GROUP(FE_node) *node_group;

	ENTER(file_read_signal_FE_node_group);
	node_group=(struct GROUP(FE_node) *)NULL;
	if (file_name&&unemap_package)
	{		
		node_order_info=(struct FE_node_order_info *)NULL;		
		if (input_file=fopen(file_name,"rb"))
		{
			if (1==BINARY_FILE_READ((char *)&region_type,sizeof(enum Region_type),1,
				input_file))
			{
				if ((SOCK==region_type)||(PATCH==region_type)||(MIXED==region_type)||
					(TORSO==region_type))
				{										
					if (node_group=read_config_FE_node_group(input_file,unemap_package,
						region_type,BINARY,&node_order_info))	
					{
						if (read_signal_FE_node_group(input_file,unemap_package,
							node_order_info))
						{
							if(!read_event_settings_and_signal_status_FE_node_group
								(input_file,unemap_package,node_order_info))
							{	
								display_message(ERROR_MESSAGE,
									"file_read_signal_FE_node_group."
									" read_event_settings_FE_node_group failed");
							}
						}
						else
						{	
							display_message(ERROR_MESSAGE,
								"file_read_signal_FE_node_group.  read_signal_FE_node_group failed");
						}
						/* no longer needed */
						DEACCESS(FE_node_order_info)(&node_order_info);
					}
					else
					{	
						display_message(ERROR_MESSAGE,
					"file_read_signal_FE_node_group.  read_config_FE_node_group failed");
					}
				}
				else
				{	
					display_message(ERROR_MESSAGE,
						"file_read_signal_FE_node_group.  Invalid rig type");
				}
			}	
			else
			{
				display_message(ERROR_MESSAGE,
					"file_read_signal_FE_node_group.  Error reading file - rig type");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"file_read_signal_FE_node_group.  Could not open %s",file_name);
			fclose(input_file);
		}	
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"file_read_signal_FE_node_group.  Invalid arguments");
	}
	LEAVE;	

	return (node_group);
} /* file_read_signal_FE_node_group */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct GROUP(FE_node) *file_read_config_FE_node_group(char *file_name,
	struct Unemap_package *unemap_package)
/*******************************************************************************
LAST MODIFIED : 28 July 1999

DESCRIPTION :
Reads and returns configuration file into  a node group.
cf file_read_FE_node_group() in import_finite_element.c
==============================================================================*/
{	
	char rig_type_text[6];
	enum Region_type region_type;
	enum Rig_file_type file_type;
	FILE *input_file = (FILE *)NULL;
	int success;	
	struct FE_node_order_info *node_order_info;
	struct GROUP(FE_node) *node_group;

	ENTER(file_read_config_FE_node_group);
	node_group=(struct GROUP(FE_node) *)NULL;
	input_file=(FILE *)NULL;
	success=0;	
	if (file_name&&unemap_package)
	{
		node_order_info=(struct FE_node_order_info *)NULL;
		/* try binary */
		if (input_file=fopen(file_name,"rb"))
		{
			BINARY_FILE_READ((char *)&region_type,sizeof(enum Region_type),1,
				input_file);
			if ((SOCK==region_type)||(PATCH==region_type)||(MIXED==region_type)||
				(TORSO==region_type))
			{
				file_type=BINARY;
				success=1;	
			}
			else
			{
				fclose(input_file);
				input_file=(FILE *)NULL;		
			}
		}
		if (!input_file)
		{
			/* try text */			
			if (input_file=fopen(file_name,"r"))
			{
				/* read the rig name in the first line in the CNFG file */	
				/* 1st 5 chars are type */
				fscanf(input_file,"%5c",rig_type_text);
				/* NULL terminate for use with strcmp*/
				rig_type_text[5]='\0';
				/* set the template based on the rig type */
				if (!strcmp("sock ",rig_type_text))
				{
					region_type=SOCK;
					/* shorten string, 'sock' only 4 chars long*/
					rig_type_text[4]='\0';
					file_type=TEXT;	
					success=1;		
				}
				else if (!strcmp("torso",rig_type_text))
				{	
					region_type=TORSO;	
					file_type=TEXT;		
					success=1;		
				}
				else if (!strcmp("patch",rig_type_text))
				{
					region_type=PATCH;
					file_type=TEXT;	
					success=1;					
				}
				else if (!strcmp("mixed",rig_type_text))
				{
					region_type=MIXED;
					file_type=TEXT;		
					success=1;				
				}	
				else
				{	
					display_message(ERROR_MESSAGE,
						"file_read_config_FE_node_group. Can't read region type");
					fclose(input_file);
					input_file=(FILE *)NULL;
				}
			} /* if(input_file=fopen(file_name,"r")) */
		} /* if(!input_file) */
		if (input_file&&success)
		{		
			node_group=read_config_FE_node_group(input_file,unemap_package,
				region_type,file_type,&node_order_info);
			if (node_group)
			{
				success=1;
			}
			else
			{	
				success=0;
			}
			fclose(input_file);		
			/* no longer needed */
			DEACCESS(FE_node_order_info)(&node_order_info);
		}	
		else
		{	
			display_message(ERROR_MESSAGE,
				"file_read_config_FE_node_group.  Invalid file: %s",file_name);
			success = 0;
		}		
	}
	else
	{
		success=0;
		display_message(ERROR_MESSAGE,
			"file_read_config_FE_node_group. Invalid arguments");
	}
	LEAVE;

	return (node_group);
} /* file_read_config_FE_node_group */

static int get_rig_node_map_electrode_position_min_max(struct FE_node *node,
	void *position_min_max_info_void)
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
Finds the min and max coordinates of the  <map_electrode_position_field>
in the <node>.
Note: map_electrode_position_field  is cylindrical polar, convert to rc.
==============================================================================*/
{	
	struct FE_field_component component;
	FE_value c0,c1,c2,x_value,y_value,z_value;
	int return_code;			
	struct Position_min_max_info *position_min_max_info;
	
	return_code=1;
	ENTER(get_rig_node_map_electrode_position_min_max);
	if((node)&&(position_min_max_info_void))
	{
		position_min_max_info=(struct Position_min_max_info *)position_min_max_info_void;	
		if(position_min_max_info&&position_min_max_info->map_electrode_position_field)
		{													 
			component.field=position_min_max_info->map_electrode_position_field;
			component.number=0;
			get_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,&c0);
			component.number=1;
			get_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,&c1);
			component.number=2;
			get_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,&c2);
			/*map_electrode_position_field is cylindrical polar */
			/*perhaps should perform check*/
			cylindrical_polar_to_cartesian(c0,c1,c2,&x_value,&y_value,&z_value,
				(FE_value *)NULL);						
			if(x_value>position_min_max_info->max_x)
			{
				position_min_max_info->max_x=x_value;
			}
			if(x_value<position_min_max_info->min_x)
			{
				position_min_max_info->min_x=x_value;
			}									
			if(y_value>position_min_max_info->max_y)
			{
				position_min_max_info->max_y=y_value;
			}
			if(y_value<position_min_max_info->min_y)
			{
				position_min_max_info->min_y=y_value;
			}			
			if(z_value>position_min_max_info->max_z)
			{
				position_min_max_info->max_z=z_value;
			}
			if(z_value<position_min_max_info->min_z)
			{
				position_min_max_info->min_z=z_value;
			}			
			position_min_max_info->count++;
		}	/* if(min_max_info */
		else
		{
			display_message(ERROR_MESSAGE,"get_rig_node_map_electrode_position_min_max."
				" min_max_info NULL ");
			return_code=0;
		}	
	}/* if((node)&&(position_min_max_info_void)) */
	else
	{
		display_message(ERROR_MESSAGE,"get_rig_node_map_electrode_position_min_max."
			" Invalid argument");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* get_rig_node_map_electrode_position_min_max*/

int get_rig_node_group_map_electrode_position_min_max(struct GROUP(FE_node) *node_group,
	struct FE_field *map_electrode_position_field,FE_value *min_x,FE_value *max_x,
  FE_value *min_y,FE_value *max_y,FE_value *min_z,FE_value *max_z)
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
Finds the min and max coordinates of the  <map_electrode_position_field>
in the <node_group>. Note: Not necessarily rectangular catresian coords!
==============================================================================*/
{	

	FE_value value;
	int return_code;
	struct Position_min_max_info position_min_max_info;
	struct FE_node *node;
	struct FE_field_component component;

	ENTER(get_rig_node_group_map_electrode_position_min_max);
	if (node_group&&min_x&&max_x&&min_y&&max_y&&min_z&&max_z
		&&map_electrode_position_field)
	{	
		position_min_max_info.count=0;
		position_min_max_info.map_electrode_position_field=
			map_electrode_position_field;	
		component.number=0;
		component.field=map_electrode_position_field;
		if(node=FIRST_OBJECT_IN_GROUP_THAT(FE_node)(
			(GROUP_CONDITIONAL_FUNCTION(FE_node) *)NULL, NULL, node_group))
		{
			value=0;
			get_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,&value);
			position_min_max_info.min_x=value;
			position_min_max_info.max_x=position_min_max_info.min_x;
			component.number=1;	
			value=0;			
			get_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,&value);
			position_min_max_info.min_y=value;
			position_min_max_info.max_y=position_min_max_info.min_y;
			component.number=2;
			value=0;
			get_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,&value);
			position_min_max_info.min_z=value;
			position_min_max_info.max_z=position_min_max_info.min_z;
			return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)
				(get_rig_node_map_electrode_position_min_max,
					(void *)&position_min_max_info,node_group);	
			*min_x= position_min_max_info.min_x;
			*max_x= position_min_max_info.max_x;
			*min_y= position_min_max_info.min_y;
			*max_y= position_min_max_info.max_y;	
			*min_z= position_min_max_info.min_z;
			*max_z= position_min_max_info.max_z;
		}
		else
		{
			display_message(ERROR_MESSAGE,"get_rig_node_group_map_electrode_position_min_max."
			" can't get node");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_rig_node_group_map_electrode_position_min_max."
			" Invalid argument");
		return_code=0;
	}
	if(return_code==0)
	{
		*min_x= 0;
		*max_x= 0;
		*min_y= 0;
		*max_y= 0;
		*min_z= 0;
		*max_z= 0;
	}
	LEAVE;

	return (return_code);
} /* get_rig_node_group_map_electrode_position_min_max */

static int get_rig_node_signal_min_max_at_time(struct FE_node *node,
	void *min_max_info_void)
/*******************************************************************************
LAST MODIFIED : 6 October 1999

DESCRIPTION :
Get the signal <value> stored at the <node>, <min_max_info->component->field>
at time <min_max_info->time>.
Compares this value to <min_max_info->max>,<min_max_info->min>, and adjusts these
accordingly.
This function is called iteratively by get_rig_node_group_signal_min_max_at_time
==============================================================================*/
{	
	char *signal_status_string;	
	enum Value_type value_type;
	FE_value fe_value;
	int return_code;		
	short short_value;
	struct Min_max_info *min_max_info;
	
	return_code=1;
	ENTER(get_rig_node_signal_min_max_at_time);
	if((node)&&(min_max_info_void))
	{
		min_max_info=(struct Min_max_info *)min_max_info_void;	
		if(min_max_info&&min_max_info->signal_component&&min_max_info->signal_status_field)
		{
			/* need to check if signal is accepted/rejected/undecided */	
			if(get_FE_nodal_string_value(node,min_max_info->signal_status_field,0,0,
				FE_NODAL_VALUE,&signal_status_string))
			{
				/* do nothing if signal is rejected */
				if(strcmp(signal_status_string,"REJECTED")) /* strcmp rets 0 for match*/
				{
					value_type=get_FE_field_value_type(min_max_info->signal_component->field);
					switch(value_type)
					{
						case FE_VALUE_ARRAY_VALUE:
						{
							return_code=get_FE_nodal_FE_value_array_value_at_FE_value_time(node,
								min_max_info->signal_component,0,FE_NODAL_VALUE,min_max_info->time,
								&fe_value);
						}break;
						case SHORT_ARRAY_VALUE:
						{
							return_code=get_FE_nodal_short_array_value_at_FE_value_time(node,
								min_max_info->signal_component,0,FE_NODAL_VALUE,min_max_info->time,
								&short_value);
							fe_value=short_value;
						}break;
						default :
						{
							display_message(ERROR_MESSAGE,"get_rig_node_signal_min_max_at_time."
								" Incorrect signal field value type");
							return_code=0;
						}break;
					}
					if(return_code)
					{
						min_max_info->count++;
						if(fe_value>min_max_info->max)
						{
							min_max_info->max=fe_value;
						}
						if(fe_value<min_max_info->min)
						{
							min_max_info->min=fe_value;
						}			
					}/* if(return_code) */
				} /* if(strcmp(signal_status_string,*/
				DEALLOCATE(signal_status_string);
			}	/* if(get_FE_nodal_string_value */
			else
			{
				display_message(ERROR_MESSAGE,"get_rig_node_signal_min_max_at_time."
					"Failed to get signal_status");
				return_code=0;	
			}				
		}	/* if(min_max_info */
		else
		{
			display_message(ERROR_MESSAGE,"get_rig_node_signal_min_max_at_time."
			" min_max_info NULL ");
			return_code=0;
		}	
	}/* if((node)&&(min_max_info_void)) */
	else
	{
		display_message(ERROR_MESSAGE,"get_rig_node_signal_min_max_at_time."
			" Invalid argument");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}

int get_rig_node_group_signal_min_max_at_time(struct GROUP(FE_node) *node_group,
	struct FE_field *signal_field,struct FE_field *signal_status_field,FE_value time,
	FE_value *min,FE_value *max)
/*******************************************************************************
LAST MODIFIED : 6 October 1999

DESCRIPTION :
Returns the <min> and <max>  signal values at the rig nodes in the rig_node_group
<node_group>, field <signal_field>, time <time>
==============================================================================*/
{	
	enum Value_type value_type;
	FE_value fe_value;
	int return_code;
	short short_value;	
	struct Min_max_info min_max_info;
	struct FE_node *node;
	struct FE_field_component component;
	
	ENTER(get_rig_node_group_signal_min_max_at_time);
	if (node_group&&min&&max&&signal_field)
	{	
		min_max_info.count=0;
		min_max_info.time=time;	
		component.number=0;
		component.field=signal_field;

		node=FIRST_OBJECT_IN_GROUP_THAT(FE_node)(
			(GROUP_CONDITIONAL_FUNCTION(FE_node) *)NULL, NULL, node_group);		
		value_type=get_FE_field_value_type(signal_field);
		switch(value_type)
		{
			case FE_VALUE_ARRAY_VALUE:
			{
				return_code=get_FE_nodal_FE_value_array_value_at_FE_value_time(node,
					&component,0,FE_NODAL_VALUE,time,&fe_value);
			}break;
			case SHORT_ARRAY_VALUE:
			{
				return_code=get_FE_nodal_short_array_value_at_FE_value_time(node,
					&component,0,FE_NODAL_VALUE,time,&short_value);
				fe_value=short_value;
			}break;
			default :
			{
				display_message(ERROR_MESSAGE,"get_rig_node_group_signal_min_max_at_time."
					" Incorrect signal field value type");
				return_code=0;
			}break;
		}
		if(return_code)
		{
			min_max_info.min=fe_value;
			min_max_info.max=min_max_info.min;
			min_max_info.signal_component=&component;
			min_max_info.signal_status_field=signal_status_field;
			return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(get_rig_node_signal_min_max_at_time,
				(void *)&min_max_info,node_group);	
			*min= min_max_info.min;
			*max= min_max_info.max;
		}
		else
		{
			*min= 0;
			*max= 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_rig_node_group_signal_min_max_at_time."
			" Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_rig_node_group_signal_min_max_at_time */

static int rig_node_set_map_electrode_position_lambda_r(struct FE_node *node,
	void *set_map_electrode_position_info_void)
/*******************************************************************************
LAST MODIFIED : 14 June 2000

DESCRIPTION :
Sets the node's nodal map_electrode_postions from the nodal electrode_positions, 
and changes the node's map_electrode_postions lambda or r  values

This function is called iteratively by 
rig_node_group_set_map_electrode_position_lambda_r
==============================================================================*/
{
	FE_value c,torso_x,lambda,mu,s,t,theta,torso_major_r,torso_minor_r,
		r,torso_y,x,y,z_cp,z_rc;	
	int return_code;	
	struct FE_field_component component;	
	struct FE_field *electrode_position_field,*map_electrode_position_field;
	struct FE_node *node_unmanaged;	
	struct MANAGER(FE_field) *field_manager;
	struct MANAGER(FE_node) *node_manager;
	struct Set_map_electrode_position_info *set_map_electrode_position_info;

	ENTER(rig_node_set_map_electrode_position_lambda_r);
	return_code=1;
	node_unmanaged=(struct FE_node *)NULL;
	if((node)&&(set_map_electrode_position_info_void))
	{
		set_map_electrode_position_info=
			(struct Set_map_electrode_position_info *)set_map_electrode_position_info_void;
		if(set_map_electrode_position_info&&(field_manager=
			set_map_electrode_position_info->field_manager)&&
			(node_manager=set_map_electrode_position_info->node_manager)&&
			(map_electrode_position_field=
				set_map_electrode_position_info->map_electrode_position_field))
		{						
			if ((electrode_position_field=
				FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)((char *)sock_electrode_position_str,
				field_manager))&&(FE_field_is_defined_at_node(electrode_position_field,node))||
				(electrode_position_field=
				FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)((char *)torso_electrode_position_str,
				field_manager))&&(FE_field_is_defined_at_node(electrode_position_field,node))||
				(electrode_position_field=
				FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)((char *)patch_electrode_position_str,
				field_manager))&&(FE_field_is_defined_at_node(electrode_position_field,node)))
				/* if we don't have an electrode_position_field, it's an auxilliary node-do 
					 nothing */
			{				
				node_unmanaged=CREATE(FE_node)(0,(struct FE_node *)NULL);
				if (MANAGER_COPY_WITH_IDENTIFIER(FE_node,cm_node_identifier)
					(node_unmanaged,node))
				{												
					switch(set_map_electrode_position_info->region_type)
					{
						case SOCK:
						{	
							/* get the source data*/
							component.field=electrode_position_field;
							component.number=0;/* lambda */
							get_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,&lambda);
							component.number=1;/* mu */
							get_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,&mu);
							component.number=2;/* theta */
							get_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,&theta);
							/* change lambda*/
							lambda=set_map_electrode_position_info->value1;
							/* set the dest data*/
							component.field=map_electrode_position_field;
							component.number=0;/* lambda */
							set_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,lambda);
							component.number=1;/* mu */
							set_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,mu);
							component.number=2;/* theta */
							set_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,theta);							
						}break;	
						case TORSO:
						{				
							/* Note that the torso map is in cp coords, but the torso */
							/* electrode_positions are in rc, so torso map_electrode_positions are in cp */
							/* get the source data stored as cartesian */
							component.field=electrode_position_field;
							component.number=0;/* x*/
							get_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,&x);	
							component.number=1;/* y */
							get_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,&y);
							component.number=2;/* z */
							get_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,&z_rc);
							/* convert coords to cylindrical polar */
							cartesian_to_cylindrical_polar(x,y,z_rc,&r,&theta,&z_cp,(float *)NULL);
							/* change the r */
#if defined (ROUND_TORSO) /*FOR AJP*/
							r=set_map_electrode_position_info->value1;/* r_major */
#else		
							torso_major_r=set_map_electrode_position_info->value1;	
							torso_minor_r=set_map_electrode_position_info->value2;
							t=theta; /*parametric T*/
							c=cos(t);
							s=sin(t);								
							torso_x=torso_major_r*c;
							torso_y=torso_minor_r*s;	
#if defined (NEW_CODE2)
							theta=atan2(torso_y,torso_x); /*new theta*/
#else /* defined (NEW_CODE2) */
							theta=t;
#endif /* defined (NEW_CODE2) */
							r=sqrt((torso_x*torso_x)+(torso_y*torso_y));
#endif /* defined (ROUND_TORSO) */
							/* set the dest data,stored as cylindrical polar  */
							component.field=map_electrode_position_field;
							component.number=0;/* r */	
							set_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,r); 
							component.number=1;/* theta */	
							set_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,theta);
							component.number=2;/* z_cp */	
							set_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,z_cp);							
						}break;
						case PATCH:
						{		
							/* Data to copy, but none to change */
							/* get the source data*/
							component.field=electrode_position_field;
							component.number=0;/* x */
							get_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,&x);
							component.number=1;/* y */
							get_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,&y);						
							/* set the dest data*/
							component.field=map_electrode_position_field;
							component.number=0;/* x */
							set_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,x);
							component.number=1;/* y */
							set_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,y);															
						}break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"rig_node_set_map_electrode_position_lambda_r"
								"  invalid region_type");
							return_code=0;
						}break;
					}
					/* copy node back into the manager */
					MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)
						(node,node_unmanaged,node_manager);
				}  	/* if (MANAGER_COPY_WITH_IDENTIFIER */
				else
				{
					display_message(ERROR_MESSAGE,
						"rig_node_set_map_electrode_position_lambda_r"
						"  MANAGER_COPY_WITH_IDENTIFIER failed");
					return_code=0;
				}
				DESTROY(FE_node)(&node_unmanaged);
			}
		}	/* if(set_map_electrode_position_info */
		else
		{
			display_message(ERROR_MESSAGE,"rig_node_set_map_electrode_position_lambda_r"
				" set_map_electrode_position_info NULL ");
			return_code=0;
		}	
	}/* if((node)&&(set_map_electrode_position_info_void)) */
	else
	{
		display_message(ERROR_MESSAGE,"rig_node_set_map_electrode_position_lambda_r"
			" Invalid argument");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* rig_node_set_map_electrode_position_lambda_r */

int rig_node_group_set_map_electrode_position_lambda_r(int map_number,
	struct Unemap_package *package,FE_value sock_lambda,FE_value torso_major_r,
	FE_value torso_minor_r)
/*******************************************************************************
LAST MODIFIED : 14 June 2000

DESCRIPTION :
Sets the node group's nodal map_electrode_postions from the nodal electrode_positions, 
and changes the node group's map_electrode_postions lambda or r values to <value>
==============================================================================*/
{	
	enum Region_type region_type;
	FE_value value1,value2;
	int node_group_number,return_code;	
	struct FE_field *map_electrode_position_field;
	struct GROUP(FE_node) *node_group;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(FE_field) *field_manager;
	struct Set_map_electrode_position_info set_map_electrode_position_info;

	ENTER(rig_node_group_set_map_electrode_position_lambda_r);
	node_group=(struct GROUP(FE_node) *)NULL;
	node_manager=(struct MANAGER(FE_node) *)NULL;
	field_manager=(struct MANAGER(FE_field) *)NULL;
	map_electrode_position_field=(struct FE_field *)NULL;
	return_code=1;
	if (package)
	{		
		region_type=get_unemap_package_map_region_type(package,map_number);
		switch(region_type)
		{
			case SOCK:
			{
				value1=sock_lambda;
				value2=0;/* not used*/
			}break;																												
			case TORSO:	
			{
				value1=torso_major_r;
				value2=torso_minor_r;
			}break;
			case PATCH:
			{									
				value1=0;/* not used*/
				value2=0;/* not used*/
			}break;	
			case MIXED:	
			{
				display_message(ERROR_MESSAGE,"rig_node_group_set_map_electrode_position_lambda_r."
					" MIXED region type");
				return_code=0;
			}break;
			case UNKNOWN:	
			default:	
			{
				display_message(ERROR_MESSAGE,"rig_node_group_set_map_electrode_position_lambda_r"
					"UNKNOWN region type");
				return_code=0;	
			}break;
		}/* switch(current_region->type) */	
		if(return_code)
		{
			map_electrode_position_field=
				get_unemap_package_map_electrode_position_field(package,map_number);		
			field_manager=get_unemap_package_FE_field_manager(package);			
			node_group_number=get_unemap_package_map_rig_node_group_number(package,map_number);
			node_group=get_unemap_package_rig_node_group(package,node_group_number);
			node_manager=	get_unemap_package_node_manager(package);
			set_map_electrode_position_info.map_electrode_position_field=
				map_electrode_position_field;	
			set_map_electrode_position_info.field_manager=field_manager;
			set_map_electrode_position_info.node_manager=node_manager;
			set_map_electrode_position_info.value1=value1;
			set_map_electrode_position_info.value2=value2;
			set_map_electrode_position_info.region_type=region_type;
			MANAGER_BEGIN_CACHE(FE_node)(node_manager);				
			return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(
				rig_node_set_map_electrode_position_lambda_r,
				(void *)&set_map_electrode_position_info,node_group);
			MANAGER_END_CACHE(FE_node)(node_manager);	
		}	
	}
	else
	{
		display_message(ERROR_MESSAGE,"rig_node_group_set_map_electrode_position_lambda_r."
			" Invalid argument");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* rig_node_group_set_map_electrode_position_lambda_r */
 
static int rig_node_add_map_electrode_position_field(struct FE_node *node,
	void *add_map_electrode_position_info_void)
/*******************************************************************************
LAST MODIFIED : 13 October 1999

DESCRIPTION :
Add a map  electrode_position_field  to the rig_node.

Called by rig_node_group_add_map_electrode_position_field
==============================================================================*/
{	
	enum FE_nodal_value_type *map_electrode_components_nodal_value_types[3]=
	{
		{
			FE_NODAL_VALUE
		},
		{
			FE_NODAL_VALUE
		},
		{
			FE_NODAL_VALUE
		}
	};
	int  map_electrode_components_number_of_derivatives[3]={0,0,0},
		 map_electrode_components_number_of_versions[3]={1,1,1},return_code;	
	struct Add_map_electrode_position_info *add_map_electrode_position_info;	
	struct FE_field *map_electrode_position_field,*electrode_position_field;
	struct FE_node *node_unmanaged;
	struct MANAGER(FE_field) *field_manager;
	struct MANAGER(FE_node) *node_manager;

	ENTER(rig_node_add_map_electrode_position_field);
	return_code=1;
	node_unmanaged=(struct FE_node *)NULL;
	electrode_position_field=(struct FE_field *)NULL;
	map_electrode_position_field=(struct FE_field *)NULL;	
	node_manager=(struct MANAGER(FE_node) *)NULL;
	field_manager=(struct MANAGER(FE_field) *)NULL;
	if((node)&&(add_map_electrode_position_info_void))
	{
		add_map_electrode_position_info=
			(struct Add_map_electrode_position_info *)add_map_electrode_position_info_void;
		if(add_map_electrode_position_info&&(map_electrode_position_field=
			add_map_electrode_position_info->map_electrode_position_field)&&
			(node_manager=add_map_electrode_position_info->node_manager)&&		
			(field_manager=add_map_electrode_position_info->field_manager))
		{					
			/* if we have a electrode_position_field, want a map_electrode_position_field */
			/* if we don't it's an auxilliary node-do nothing. */
			if ((electrode_position_field=
				FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)((char *)sock_electrode_position_str,
				field_manager))&&(FE_field_is_defined_at_node(electrode_position_field,node))||
				(electrode_position_field=
				FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)((char *)torso_electrode_position_str,
				field_manager))&&(FE_field_is_defined_at_node(electrode_position_field,node))||
				(electrode_position_field=
				FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)((char *)patch_electrode_position_str,
				field_manager))&&(FE_field_is_defined_at_node(electrode_position_field,node)))
			{	
				/* if its already deined, have nothing to do */
				if(!FE_field_is_defined_at_node(map_electrode_position_field,node))
				{
					node_unmanaged=CREATE(FE_node)(0,(struct FE_node *)NULL);
					if (MANAGER_COPY_WITH_IDENTIFIER(FE_node,cm_node_identifier)
						(node_unmanaged,node))
					{						
						if(!define_FE_field_at_node(node_unmanaged,map_electrode_position_field,
							map_electrode_components_number_of_derivatives
							,map_electrode_components_number_of_versions,
							map_electrode_components_nodal_value_types))					
						{
							display_message(ERROR_MESSAGE,"rig_node_add_map_electrode_position_field."
								"error defining electrode_position_field");	
							return_code =0;
						}	
						/* copy node back into the manager */
						MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)
							(node,node_unmanaged,node_manager);
					}  	/* if (MANAGER_COPY_WITH_IDENTIFIER */
					else
					{
						display_message(ERROR_MESSAGE,
							"rig_node_add_map_electrode_position_field"
							"  MANAGER_COPY_WITH_IDENTIFIER failed");
						return_code=0;
					}
					DESTROY(FE_node)(&node_unmanaged);
				}
			}/* if ((electrode_position_field= */
		}	/* if(add_map_electrode_position_info */
		else
		{
			display_message(ERROR_MESSAGE,"rig_node_add_map_electrode_position_field"
				" add_map_electrode_position_info NULL ");
			return_code=0;
		}	
	}/* if((node)&&(add_map_electrode_position_info_void)) */
	else
	{
		display_message(ERROR_MESSAGE,"rig_node_add_map_electrode_position_field"
			" Invalid argument");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* rig_node_add_map_electrode_position_field */

int rig_node_group_add_map_electrode_position_field(int map_number,
	struct Unemap_package *package,struct FE_field *map_electrode_position_field)
/*******************************************************************************
LAST MODIFIED : 13 October 1999

DESCRIPTION :
Add an electrode_position_field  to the rig_node_group nodes,
in addition to the one created with create_config_template_node.

==============================================================================*/
{	
	int node_group_number,return_code;
	struct Add_map_electrode_position_info add_map_electrode_position_info;
	struct GROUP(FE_node) *node_group;
	struct MANAGER(FE_field) *field_manager;
	struct MANAGER(FE_node) *node_manager;
	
	ENTER(rig_node_group_change_electrode_position_focus);
	node_group=(struct GROUP(FE_node) *)NULL;
	node_manager=(struct MANAGER(FE_node) *)NULL;
	if (package&&map_electrode_position_field)
	{				
		node_group_number=get_unemap_package_map_rig_node_group_number(package,map_number);
		node_group=get_unemap_package_rig_node_group(package,node_group_number);
		node_manager=	get_unemap_package_node_manager(package);
		field_manager=get_unemap_package_FE_field_manager(package);
		add_map_electrode_position_info.field_manager=field_manager;	
		add_map_electrode_position_info.map_electrode_position_field=
			map_electrode_position_field;
		add_map_electrode_position_info.node_manager=node_manager;		
		MANAGER_BEGIN_CACHE(FE_node)(node_manager);				
		return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(rig_node_add_map_electrode_position_field,
			(void *)&add_map_electrode_position_info,node_group);
		MANAGER_END_CACHE(FE_node)(node_manager);
	}
	else
	{
		display_message(ERROR_MESSAGE,"rig_node_group_change_electrode_position_focus."
			" Invalid argument");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* rig_node_group_change_electrode_position_focus */
#endif /* defined (UNEMAP_USE_NODES) */
