/*******************************************************************************
FILE : rig_node.c

LAST MODIFIED : 12 May 2003

DESCRIPTION :
Essentially the same functionality as rig.c, but using nodes and fields to store
the rig and signal information. rather than special structures.
==============================================================================*/
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>
/*ieeefp.h doesn't exist for Linux. Needed for finite() for Irix. */
/*finite() in math.h in Linux */
#if defined (NOT_ANSI)
#include <ieeefp.h>
#endif /* defined (NOT_ANSI) */
#include "finite_element/finite_element.h"
#include "finite_element/import_finite_element.h"
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


#if defined (UNEMAP_USE_3D)

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
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Used by  rig_node_group_map_add_electrode_position_field
to store info when iterating 
==============================================================================*/
{
	struct FE_field *map_electrode_position_field;	
	struct FE_region *fe_region;
};

struct Position_min_max_iterator
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
used by get_node_position_min_max
==============================================================================*/
{
	FE_value max_x,min_x,max_y,min_y,max_z,min_z;
	int count;
	struct FE_field *position_field;
};

struct Set_map_electrode_position_info
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION 	
Used by rig_node_group_set_map_electrode_position_lambda_r
to store info when iterating
==============================================================================*/
{	
	enum Region_type region_type;
	FE_value value1;
	FE_value value2;
	struct FE_field *map_electrode_position_field;
	struct FE_region *fe_region;
};

#endif /* defined (UNEMAP_USE_3D) */


/*
Module variables
----------------
*/


/*
Module functions
----------------
*/

#if defined (UNEMAP_USE_3D)
static struct FE_node *create_config_template_node(enum Config_node_type 
	config_node_type,FE_value focus,struct FE_field_order_info **field_order_info,
	struct Unemap_package *unemap_package,
	struct FE_field **electrode_position_field)
/*******************************************************************************
LAST MODIFIED : 8 May 2003

DESCRIPTION :
Creates and returns a configuration template node for read_text_config_FE_node
and read_binary_config_FE_node.  The type of the configuration template node is
determined by Config_node_type.  Cannot create a template node of type
MIXED_TYPE, this doesn't make sense.  focus is relevant only for
config_node_type=SOCK_ELECTRODE_TYPE, for other types, focus is ignored.
Creates, fills in and returns and returns field_order_info, containing the
node fields.  Therefore, field_order_info should be an unassigned pointer when
passed to this function, and must be freed by the calling function.  The fields
of the field_order_info are named using the node_type, therefore beware of name
clashes if loading multiple rigs of the same node_type.  Also returns the
created electrode_position_field.
==============================================================================*/
{	
#if defined (UNEMAP_USE_NODES)	
  #define AUXILIARY_NUM_CONFIG_FIELDS 5 /*,device_name_field,*/
	/* device_type_field channel_number,read_order_field, highlight_field */
  #define PATCH_NUM_CONFIG_FIELDS 6 /* electrode_position_field,*/ 
	/*device_name_field,device_type_field,channel_number,read_order_field,*/
	/*highlight_field */	
  #define SOCK_NUM_CONFIG_FIELDS 6 /* electrode_position_field,*/
	/*device_name_field,device_type_field,channel_number,read_order_field,*/
	/*highlight_field */
  #define TORSO_NUM_CONFIG_FIELDS 6/* electrode_position_field,*/ 
  /*device_name_field,device_type_field,channel_number,read_order_field,*/
	/*highlight_field */
#else
  #define AUXILIARY_NUM_CONFIG_FIELDS 4 /*,device_name_field,*/
	/* device_type_field channel_number,read_order_field */
  #define PATCH_NUM_CONFIG_FIELDS 5 /* electrode_position_field,*/ 
	/*device_name_field,device_type_field,channel_number,read_order_field,*/		
  #define SOCK_NUM_CONFIG_FIELDS 5 /* electrode_position_field,*/
	/*device_name_field,device_type_field,channel_number,read_order_field,*/
  #define TORSO_NUM_CONFIG_FIELDS 5/* electrode_position_field,*/ 
  /*device_name_field,device_type_field,channel_number,read_order_field,*/
#endif /* (UNEMAP_USE_NODES) */

	char *electrode_name;
	char *rig_type_str;
	int number_of_fields,success;
	struct Cmiss_region *root_cmiss_region;	
	struct Coordinate_system coordinate_system;	
	struct FE_field *channel_number_field;
	struct FE_field *device_name_field;
	struct FE_field *device_type_field;
	struct FE_field *the_electrode_position_field;
	struct FE_field *read_order_field;
#if  defined (UNEMAP_USE_NODES)
	struct FE_field *highlight_field;
#endif /*  defined (UNEMAP_USE_NODES) */
	struct FE_field_order_info *the_field_order_info;
	struct FE_node *node;
	struct FE_node_field_creator *node_field_creator;
	struct FE_region *root_fe_region;
	
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
		"channel number"
	};
#if  defined (UNEMAP_USE_NODES)	
	char *highlight_component_names[1]=
	{
		"highlight"
	};		
#endif /*  defined (UNEMAP_USE_NODES)*/
	char *read_order_component_names[1]=
	{
		"read order"
	};

	ENTER(create_config_template_node);
	if (unemap_package)
	{		
		electrode_name =(char *)NULL;
		rig_type_str =(char *)NULL;	
		channel_number_field=(struct FE_field *)NULL;	
#if  defined (UNEMAP_USE_NODES)
		highlight_field=(struct FE_field *)NULL;
#endif /* defined (UNEMAP_USE_NODES) */
		read_order_field=(struct FE_field *)NULL;
		device_name_field=(struct FE_field *)NULL;
		device_type_field=(struct FE_field *)NULL;			
		the_electrode_position_field=(struct FE_field *)NULL;
		the_field_order_info = 
			(struct FE_field_order_info *)NULL;
		node=(struct FE_node *)NULL;
		root_cmiss_region = get_unemap_package_root_Cmiss_region(unemap_package);
		root_fe_region = Cmiss_region_get_FE_region(root_cmiss_region);
		FE_region_begin_change(root_fe_region);
		success = 1;			
		electrode_name = (char *)NULL;
		/* create the node */		
		if (node = CREATE(FE_node)(0, root_fe_region, (struct FE_node *)NULL))
		{
			/* get info that depends on config_node_type */
			switch (config_node_type)
			{
				case SOCK_ELECTRODE_TYPE:
				{	
					number_of_fields = SOCK_NUM_CONFIG_FIELDS; 
					rig_type_str = (char *)sock_electrode_position_str;
				} break;
				case PATCH_ELECTRODE_TYPE:
				{		
					number_of_fields = PATCH_NUM_CONFIG_FIELDS;				
					rig_type_str = (char *)patch_electrode_position_str;
				} break;
				case TORSO_ELECTRODE_TYPE:
				{	
					number_of_fields = TORSO_NUM_CONFIG_FIELDS;
					rig_type_str = (char *)torso_electrode_position_str;
				} break;
				case AUXILIARY_TYPE:
				{						
					number_of_fields = AUXILIARY_NUM_CONFIG_FIELDS;
					rig_type_str = (char *)auxiliary_str;
				} break;
			}
			/* create the FE_field_order_info for all the fields */
			if (!(the_field_order_info = CREATE(FE_field_order_info)()))
			{
				display_message(ERROR_MESSAGE,
					"create_config_template_node.  Could not create field_order_info");
				success = 0;
			}
			/* create the fields which depend on template types. */		
			if (success)
			{
				if (electrode_name = duplicate_string(rig_type_str))
				{
					if (node_field_creator = CREATE(FE_node_field_creator)(
						/*number_of_components*/3))
					{
						switch (config_node_type)
						{
							case SOCK_ELECTRODE_TYPE:
							{										
								/* set up the info needed to create the electrode position
									field */
								coordinate_system.type=PROLATE_SPHEROIDAL;								
								coordinate_system.parameters.focus = focus;								
								/* create the electrode position field, add it to the node and
									create the FE_field_order_info with enough fields */
								if ((the_electrode_position_field=
									FE_region_get_FE_field_with_properties(root_fe_region,
									electrode_name,GENERAL_FE_FIELD,
									/*indexer_field*/(struct FE_field *)NULL,
									/*number_of_indexed_values*/0,CM_COORDINATE_FIELD,
									&coordinate_system,FE_VALUE_VALUE,
									/*number_of_components*/3,sock_electrode_component_names,
									/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
									(struct FE_field_external_information *)NULL)))
								{
									success =define_node_field_and_field_order_info(node,
										the_electrode_position_field, node_field_creator,
										the_field_order_info);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"create_config_template_node.  "
										"Could not retrieve sock electrode position field");
									success=0;			
								}
							} break;			
							case TORSO_ELECTRODE_TYPE:
							{				
								/* set up the info needed to create the electrode position
									field */
								coordinate_system.type=RECTANGULAR_CARTESIAN;
								if ((the_electrode_position_field=
									FE_region_get_FE_field_with_properties(root_fe_region,
									electrode_name,GENERAL_FE_FIELD,
									/*indexer_field*/(struct FE_field *)NULL,
									/*number_of_indexed_values*/0,CM_COORDINATE_FIELD,
									&coordinate_system,FE_VALUE_VALUE,
									/*number_of_components*/3,electrode_component_names,
									/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
									(struct FE_field_external_information *)NULL)))
								{
									success =define_node_field_and_field_order_info(node,
										the_electrode_position_field, node_field_creator,
										the_field_order_info);								 				
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"create_config_template_node.  "
										"Could not torso retrieve electrode position field");
									success=0;			
								}
							} break;		
							case PATCH_ELECTRODE_TYPE:
							{
								/* set up the info needed to create the electrode position
									field */
								coordinate_system.type=RECTANGULAR_CARTESIAN;						
								/* create the electrode position field, add it to the node */
								if ((the_electrode_position_field=
									FE_region_get_FE_field_with_properties(root_fe_region,
									electrode_name,GENERAL_FE_FIELD,
									/*indexer_field*/(struct FE_field *)NULL,
									/*number_of_indexed_values*/0,CM_COORDINATE_FIELD,
									&coordinate_system,FE_VALUE_VALUE,
									/*number_of_components*/2,electrode_component_names,
									/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
									(struct FE_field_external_information *)NULL)))
								{
									success=define_node_field_and_field_order_info(node,
										the_electrode_position_field, node_field_creator,
										the_field_order_info);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"create_config_template_node.  "
										"Could not retrieve patch electrode position field");
									success=0;			
								}
							} break;				
						} /* switch */
						DESTROY(FE_node_field_creator)(&node_field_creator);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_config_template_node.  "
							"Could not make node field creator.");
						success=0;			
					}
				}
				else
				{	
					display_message(ERROR_MESSAGE,
						"create_config_template_node. Could not allocate memory for "
						"electrode_name");
					success =0;
				}
			} /* if (success) */
			DEALLOCATE(electrode_name);		
			/* create fields common to all node */
			if (success)
			{							
				/* All the rest have 1 component with no versions or derivatives */
				if (node_field_creator = CREATE(FE_node_field_creator)(
					/*number_of_components*/1))
				{
					/* set up the info needed to create the device name field */			
					coordinate_system.type=NOT_APPLICABLE;				
					/* create the device name field, add it to the node */			
					if (device_name_field=FE_region_get_FE_field_with_properties(
						root_fe_region,"device_name",GENERAL_FE_FIELD,
						/*indexer_field*/(struct FE_field *)NULL,
						/*number_of_indexed_values*/0,CM_GENERAL_FIELD,
						&coordinate_system,STRING_VALUE,
						/*number_of_components*/1,device_name_component_names,
						/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
						(struct FE_field_external_information *)NULL))
					{ 
						set_unemap_package_device_name_field(unemap_package,
							device_name_field);
						success =define_node_field_and_field_order_info(node,
							device_name_field, node_field_creator,
							the_field_order_info); 
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_config_template_node.  "
							"Could not retrieve device name field");
						success=0;
					}				
					/* set up the info needed to create the device type field */			
					coordinate_system.type=NOT_APPLICABLE;				
					/* create the device type field, add it to the node */			
					if (device_type_field=FE_region_get_FE_field_with_properties(
							 root_fe_region,"device_type",GENERAL_FE_FIELD,
							 /*indexer_field*/(struct FE_field *)NULL,
							 /*number_of_indexed_values*/0,CM_GENERAL_FIELD,
							 &coordinate_system,STRING_VALUE,
							 /*number_of_components*/1,device_type_component_names,
							 /*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
							 (struct FE_field_external_information *)NULL))
					{ 
						set_unemap_package_device_type_field(unemap_package,device_type_field);
						success =define_node_field_and_field_order_info(node,
							device_type_field, node_field_creator,
							the_field_order_info); 						
					}
					else
					{
						display_message(ERROR_MESSAGE,"create_config_template_node.  "
							"Could not retrieve device type field");
						success=0;
					}					
					/* set up the info needed to create the channel number field */			
					coordinate_system.type=NOT_APPLICABLE;				
					/* create the channel number field, add it to the node */			
					if (channel_number_field=FE_region_get_FE_field_with_properties(
							 root_fe_region,"channel_number",GENERAL_FE_FIELD,
							 /*indexer_field*/(struct FE_field *)NULL,
							 /*number_of_indexed_values*/0,CM_GENERAL_FIELD,
							 &coordinate_system,INT_VALUE,
							 /*number_of_components*/1,channel_number_component_names,
							 /*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
							 (struct FE_field_external_information *)NULL))
					{	
						set_unemap_package_channel_number_field(unemap_package,channel_number_field);
						success =define_node_field_and_field_order_info(node,
							channel_number_field, node_field_creator,
							the_field_order_info); 
					}
					else
					{
						display_message(ERROR_MESSAGE,"create_config_template_node.  "
							"Could not retrieve channel number field");
						success=0;
					}
					/* set up the info needed to create the read_order field */			
					coordinate_system.type=NOT_APPLICABLE;				
					/* create the channel number field, add it to the node */			
					if (read_order_field=FE_region_get_FE_field_with_properties(
							 root_fe_region,"read_order",GENERAL_FE_FIELD,
							 /*indexer_field*/(struct FE_field *)NULL,
							 /*number_of_indexed_values*/0,CM_GENERAL_FIELD,
							 &coordinate_system,INT_VALUE,
							 /*number_of_components*/1,read_order_component_names,
							 /*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
							 (struct FE_field_external_information *)NULL))
					{	
						set_unemap_package_read_order_field(unemap_package,read_order_field);
						success =define_node_field_and_field_order_info(node,
							read_order_field, node_field_creator,
							the_field_order_info); 
					}
					else
					{
						display_message(ERROR_MESSAGE,"create_config_template_node.  "
							"Could not retrieve read order field");
						success=0;
					}
#if defined (UNEMAP_USE_NODES)	
					/* set up the info needed to create the highlight field */			
					coordinate_system.type=NOT_APPLICABLE;				
					/* create the channel number field, add it to the node */			
					if (highlight_field=FE_region_get_FE_field_with_properties(
							 root_fe_region,"highlight",GENERAL_FE_FIELD,
							 /*indexer_field*/(struct FE_field *)NULL,
							 /*number_of_indexed_values*/0,CM_GENERAL_FIELD,
							 &coordinate_system,INT_VALUE,
							 /*number_of_components*/1,highlight_component_names,
							 /*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
							 (struct FE_field_external_information *)NULL))
					{	
						set_unemap_package_highlight_field(unemap_package,highlight_field);					
						success =define_node_field_and_field_order_info(node,
							highlight_field, node_field_creator,
							the_field_order_info); 
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_config_template_node. Could not retrieve highlight field");
						success=0;
					}			
#endif /*  defined (UNEMAP_USE_NODES) */
					/* check the number of fields matched what was expected */
					if (get_FE_field_order_info_number_of_fields(the_field_order_info) !=
						number_of_fields)
					{
						display_message(ERROR_MESSAGE, "create_config_template_node.  "
							"Wrong number of fields for config_node_type");
						success = 0;
					}
					DESTROY(FE_node_field_creator)(&node_field_creator);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_config_template_node.  "
						"Could not make node field creator.");
					success=0;			
				}
			} /* if (success) */
		}	
		else
		{
			display_message(ERROR_MESSAGE,
				"create_config_template_node.  Could not create node");
			success = 0;
		}
		*field_order_info = the_field_order_info;
		*electrode_position_field = the_electrode_position_field;
		if (!success)
		{
			DESTROY(FE_node)(&node);
			node = (struct FE_node *)NULL;			
		}		
		FE_region_end_change(root_fe_region);
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
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static struct FE_node *set_config_FE_node(struct FE_node *template_node,
	struct FE_region *fe_region, enum Config_node_type	config_node_type,
	int node_number,int read_order_number,
#if  defined (UNEMAP_USE_NODES)
	int highlight,
#endif /* defined (UNEMAP_USE_NODES) */
	struct FE_field_order_info *field_order_info,char *name,
	int channel_number,FE_value xpos,FE_value ypos,FE_value zpos)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
sets a node's values storage with the passed, name, channel_number,
read_order_number, highlight  xpos, ypos, zpos using the template node created
in  create_config_template_node, and the field_order_info. 
==============================================================================*/
{
	FE_value lambda,mu,theta;	
	int component_number,field_number;
	struct Coordinate_system *coordinate_system;
	struct FE_field *field;
	struct FE_field_component component;
	struct FE_node *node;

	ENTER(set_config_FE_node);
	node = (struct FE_node *)NULL;
	field_number = 0;
	if (fe_region && template_node && field_order_info)
	{
		if (FE_region_get_FE_node_from_identifier(fe_region, node_number))
		{
			display_message(ERROR_MESSAGE,
				"set_config_FE_node.  Node already exists!");	
		}				
		else
		{
			if (node = CREATE(FE_node)(node_number, (struct FE_region *)NULL,
				template_node))
			{								
				/*	set up the type dependent values	*/
				switch (config_node_type)
				{
					case SOCK_ELECTRODE_TYPE:
					{	
						/* convert to prolate */	 
						/* 1st field contains electrode coords */
						component.field=get_FE_field_order_info_field(
							field_order_info,field_number);
						field_number++;
						coordinate_system =	get_FE_field_coordinate_system(component.field);
					
#if defined (DEBUG)
						/* for IUPS heart usr/people/williams/unemap_notes/iups2001/poster/epi_trsf_new.signal */
						{
							FE_value a,b,c;
							Linear_transformation *lt;
					
							if (ALLOCATE(lt,Linear_transformation,1))
							{						
								lt->translate_x=-19.0514;
								lt->translate_y=-71.9462;
								lt->translate_z=3.1975;

								lt->txx=0.6457;
								lt->txy= 0.6298;
								lt->txz= 0.4316;

								lt->tyx= 0.2155;
								lt->tyy= -0.6927;
								lt->tyz= 0.6883;

								lt->tzx= -0.7325;
								lt->tzy=   0.3514;
								lt->tzz= 0.5830;

								linear_transformation(lt,xpos,ypos,zpos,&a,&b,&c);	
								cartesian_to_prolate_spheroidal(a,b,c,
									coordinate_system->parameters.focus,&lambda,&mu,&theta,
									(float *)NULL);
							}
						}
#else
						cartesian_to_prolate_spheroidal(xpos,ypos,zpos,
							coordinate_system->parameters.focus,&lambda,&mu,&theta,
							(float *)NULL);
#endif/*  defined (DEBUG)*/
						component.number = 0;					
						set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,/*time*/0,lambda);
						component.number = 1;
						set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,/*time*/0,mu);
						component.number = 2;
						set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,/*time*/0,theta);
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
							FE_NODAL_VALUE,/*time*/0,channel_number);	
						/* 5th field contains read_order_number */
						component.field=get_FE_field_order_info_field(
							field_order_info,field_number);
						field_number++;
						component.number=0;
						set_FE_nodal_int_value(node,&component,/*version*/0,
							FE_NODAL_VALUE,/*time*/0,read_order_number);	
#if  defined (UNEMAP_USE_NODES)	
						/* 6th field contains highlight */
						component.field=get_FE_field_order_info_field(
							field_order_info,field_number);
						field_number++;
						component.number=0;
						set_FE_nodal_int_value(node,&component,/*version*/0,
							FE_NODAL_VALUE,/*time*/0,highlight);
#endif /*  defined (UNEMAP_USE_NODES) */
					} break;
					case TORSO_ELECTRODE_TYPE:
					{
						/* 1st field contains electrode coords */	
						component.field=get_FE_field_order_info_field(
							field_order_info,field_number);
						field_number++;
						component.number = 0;					
						set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,/*time*/0,xpos);
						component.number = 1;	
						set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,/*time*/0,ypos);
						component.number = 2;	
						set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,/*time*/0,zpos);
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
							FE_NODAL_VALUE,/*time*/0,channel_number);	
						/* 5th field contains read_order_number */
						component.field=get_FE_field_order_info_field(
							field_order_info,field_number);
						field_number++;
						component.number=0;
						set_FE_nodal_int_value(node,&component,/*version*/0,
							FE_NODAL_VALUE,/*time*/0,read_order_number);
#if  defined (UNEMAP_USE_NODES)		
						/* 6th field contains highlight */
						component.field=get_FE_field_order_info_field(
							field_order_info,field_number);
						field_number++;
						component.number=0;
						set_FE_nodal_int_value(node,&component,/*version*/0,
							FE_NODAL_VALUE,/*time*/0,highlight);
#endif /*  defined (UNEMAP_USE_NODES)	*/
					} break;		
					case PATCH_ELECTRODE_TYPE:
					{	 
						/* 1st field contains electrode coords */
						component.field=get_FE_field_order_info_field(
							field_order_info,field_number);
						field_number++;
						component.number = 0;					
						set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,/*time*/0,xpos);
						component.number = 1;	
						set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,/*time*/0,ypos);
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
							FE_NODAL_VALUE,/*time*/0,channel_number);	
						/* 5th field contains read_order_number */
						component.field=get_FE_field_order_info_field(
							field_order_info,field_number);
						field_number++;
						component.number=0;
						set_FE_nodal_int_value(node,&component,/*version*/0,
							FE_NODAL_VALUE,/*time*/0,read_order_number);	
#if  defined (UNEMAP_USE_NODES)	
						/* 6th field contains highlight */
						component.field=get_FE_field_order_info_field(
							field_order_info,field_number);
						field_number++;
						component.number=0;
						set_FE_nodal_int_value(node,&component,/*version*/0,
							FE_NODAL_VALUE,/*time*/0,highlight);
#endif /*  defined (UNEMAP_USE_NODES)	*/
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
							FE_NODAL_VALUE,"AUXILIARY");
						/* 3rd field contains channel number */					
						component.field=get_FE_field_order_info_field(
							field_order_info,field_number);
						field_number++;
						component.number=0;
						set_FE_nodal_int_value(node,&component,/*version*/0,
							FE_NODAL_VALUE,/*time*/0,channel_number);	
						/* 4th field contains read_order_number */
						component.field=get_FE_field_order_info_field(
							field_order_info,field_number);
						field_number++;
						component.number=0;
						set_FE_nodal_int_value(node,&component,/*version*/0,
							FE_NODAL_VALUE,/*time*/0,read_order_number);
#if  defined (UNEMAP_USE_NODES)		
						/* 5th field contains highlight */
						component.field=get_FE_field_order_info_field(
							field_order_info,field_number);
						field_number++;
						component.number=0;
						set_FE_nodal_int_value(node,&component,/*version*/0,
							FE_NODAL_VALUE,/*time*/0,highlight);
#endif /*  defined (UNEMAP_USE_NODES)	*/
					} break;
				}	/* switch () */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_config_FE_node.  Error creating node");	
			}
		}
	}	
	else
	{
		display_message(ERROR_MESSAGE, "set_config_FE_node.  Invalid argument(s)");
	}
	LEAVE;

	return (node);
}/* set_config_FE_node*/
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static struct FE_node *read_text_config_FE_node(FILE *input_file,
	struct FE_node *template_node,struct FE_region *fe_region,
	enum Config_node_type	config_node_type,int node_number,int read_order_number,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 6 July 2004

DESCRIPTION :
Reads a node  from a text configuration file, using the template node created in
create_config_template_node.
cf read_FE_node() in import_finite_element.c
==============================================================================*/
{	
	char *dummy_str,*electrode_name,*name,separator,string[10],*trimmed_string;
	FE_value coefficient,*coefficients,*coefficients_temp,sign,xpos,ypos,zpos;	
	int channel_number,found,electrode_number,*electrode_numbers,
		*electrode_numbers_temp,number_of_electrodes,string_length,success;
	struct FE_node *node;

	ENTER(read_text_config_FE_node);
	node=(struct FE_node *)NULL;
	success=0;
	if (input_file&&template_node&& fe_region &&field_order_info)
	{
		fscanf(input_file," : ");
		/* read in name */
		dummy_str=(char *)NULL;
		if (read_string(input_file,"[^\n]",&name))
		{
			if (trimmed_string=trim_string(name))
			{
				DEALLOCATE(name);
				name=trimmed_string;
			}
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
											while (isspace(*electrode_name))
											{
												electrode_name++;
											}
											/* trim trailing space */
											string_length=strlen(electrode_name);
											while ((0<string_length)&&
												(isspace(electrode_name[string_length-1])))
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
				if (!(node=set_config_FE_node(template_node, fe_region,
					config_node_type,node_number,read_order_number,
#if  defined (UNEMAP_USE_NODES)
					0/*highlight*/,
#endif /* defined (UNEMAP_USE_NODES)*/
					field_order_info,name,channel_number,xpos,ypos,zpos)))
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
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static struct FE_node *read_binary_config_FE_node(FILE *input_file,
	struct FE_node *template_node,struct FE_region *fe_region,
	enum Config_node_type	config_node_type,int node_number,int read_order_number,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

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
	if (input_file&&template_node&& fe_region &&field_order_info)
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
						/*??JW Don't handle the linear combiation auxiliaries as nodes yet*/
						/* so don't do anything with these coefficients,node numbers */
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
				if (!(node=set_config_FE_node(template_node, fe_region,
					config_node_type,node_number,read_order_number,
#if  defined (UNEMAP_USE_NODES)
					0/*highlight*/,
#endif /*  defined (UNEMAP_USE_NODES) */
					field_order_info,name,channel_number,xpos,ypos,zpos)))
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
			DEALLOCATE(name);
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
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int read_binary_config_FE_node_group(FILE *input_file,
	struct Unemap_package *package,enum Region_type rig_type,
	struct FE_node_order_info **the_node_order_info,struct Rig *rig) 
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Reads  a node group from a configuration file into the rig . An FE_node_order_info
is created, and so must be destroyed by the calling function.
cf read_FE_node_group() in import_finite_element.c, and read_configuration()
in rig.c
==============================================================================*/
{	
	char *page_name,*rig_name,*region_name;
	char region_num_string[10];	
	enum Config_node_type config_node_type;	
	enum Device_type device_type;
	enum Region_type region_type;
	FE_value focus;	
	int count,device_count,device_number,last_node_number,node_number,
		number_of_pages,number_of_page_devices,number_of_regions,read_order_number,
		region_number,region_number_of_devices,return_code,string_length;
	int string_error =0;
	struct Cmiss_region *root_cmiss_region;
	struct FE_field *electrode_position_field;	
	struct FE_field_order_info *field_order_info;	
	struct FE_node *node,*template_node;
	struct FE_node_order_info *all_devices_node_order_info;	
	struct FE_region *node_group, *all_devices_node_group, *root_fe_region;	
	struct Region *region=(struct Region *)NULL;
	struct Region_list_item *region_item=(struct Region_list_item *)NULL;

	ENTER(read_binary_config_FE_node_group);	
	return_code = 1;
	rig_name =(char *)NULL;	
	region_name =(char *)NULL;
	page_name =(char *)NULL;	
	template_node = (struct FE_node *)NULL;	
	field_order_info = (struct FE_field_order_info *)NULL;
	electrode_position_field = (struct FE_field *)NULL;
	node_group = (struct FE_region *)NULL;
	all_devices_node_group = (struct FE_region *)NULL;
	all_devices_node_order_info =(struct FE_node_order_info *)NULL;	
	device_count =0;
	if (input_file&&package) 
	{			
		root_cmiss_region = get_unemap_package_root_Cmiss_region(package);
		/* we've read in the rig type, now read in the rig name */
		/* read the rig name */
		BINARY_FILE_READ((char *)&string_length,sizeof(int),1,input_file);
		if (ALLOCATE(rig_name,char,string_length))
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
					/* do nothing for mixed case, need to read in more info to */
					/*decide node_type*/
					;
				} break;
				case SOCK:
				{			
					/* read in focus */
					BINARY_FILE_READ((char *)&focus,sizeof(float),1,input_file);
					config_node_type = SOCK_ELECTRODE_TYPE;
				} break;
				case PATCH:
				{
					config_node_type = PATCH_ELECTRODE_TYPE;
				} break;
				case TORSO:	
				{
					config_node_type = TORSO_ELECTRODE_TYPE;
				} break;
			}
			root_fe_region = Cmiss_region_get_FE_region(root_cmiss_region);
			if (all_devices_node_group = Cmiss_region_get_or_create_child_FE_region(
				root_cmiss_region, "all_devices", /*master_fe_region*/root_fe_region,
				(struct MANAGER(FE_basis) *)NULL, (struct LIST(FE_element_shape) *)NULL))
			{
				FE_region_begin_change(all_devices_node_group);
				FE_region_clear(all_devices_node_group, /*destroy_in_master*/1);
				set_Rig_all_devices_rig_node_group(rig,all_devices_node_group);
				last_node_number = 1;	
				/* read the number of regions */
				BINARY_FILE_READ((char *)&number_of_regions,sizeof(int),1,input_file);
				region_number=0;			
				if (region_item=get_Rig_region_list(rig))
				{
					while((region_number<number_of_regions)&&return_code)
					{
						if (!(region=get_Region_list_item_region(region_item)))
						{
							display_message(ERROR_MESSAGE,"read_binary_config_FE_node_group. "
								"region_item->region is NULL ");
							return_code=0;
						}
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
										/*destroy any existing template,field_order_info*/
										/* create the template node */								
										DESTROY(FE_node)(&template_node);
										DESTROY(FE_field_order_info)(&field_order_info);	
										if (node_group) /* stop caching the current group,(if any) */
										{
											FE_region_end_change(node_group);
											FE_region_end_change(node_group);
										}			
										template_node = create_config_template_node(
											SOCK_ELECTRODE_TYPE,focus,&field_order_info,package,
											&electrode_position_field);
										node_group = Cmiss_region_get_or_create_child_FE_region(
											root_cmiss_region, region_name,
											/*master_fe_region*/root_fe_region,
											(struct MANAGER(FE_basis) *)NULL, (struct LIST(FE_element_shape) *)NULL);
										FE_region_begin_change(node_group);
										FE_region_clear(node_group, /*destroy_in_master*/1);
										set_Region_electrode_position_field(region,electrode_position_field);
										set_Region_rig_node_group(region,node_group);								
									} break;
									case TORSO:
									{	
										config_node_type = TORSO_ELECTRODE_TYPE;
										/*destroy any existing template,field_order_info*/
										/*create the template node */								
										DESTROY(FE_node)(&template_node);
										DESTROY(FE_field_order_info)(&field_order_info);
										if (node_group) /* stop caching the current group,(if any) */
										{
											FE_region_end_change(node_group);
										}	
										template_node = create_config_template_node(
											TORSO_ELECTRODE_TYPE,0,&field_order_info,package,
											&electrode_position_field);
										node_group = Cmiss_region_get_or_create_child_FE_region(
											root_cmiss_region, region_name,
											/*master_fe_region*/root_fe_region,
											(struct MANAGER(FE_basis) *)NULL, (struct LIST(FE_element_shape) *)NULL);
										FE_region_begin_change(node_group);
										FE_region_clear(node_group, /*destroy_in_master*/1);
										set_Region_electrode_position_field(region,electrode_position_field);
										set_Region_rig_node_group(region,node_group);
									} break;
									case PATCH:
									{
										config_node_type = PATCH_ELECTRODE_TYPE;
										/*destroy any existing template,field_order_info*/
										/* create the template node */								
										DESTROY(FE_node)(&template_node);
										DESTROY(FE_field_order_info)(&field_order_info);	
										if (node_group) /* stop caching the current group,(if any) */
										{
											FE_region_end_change(node_group);
										}		
										template_node = create_config_template_node(
											PATCH_ELECTRODE_TYPE,0,&field_order_info,package,
											&electrode_position_field);
										node_group = Cmiss_region_get_or_create_child_FE_region(
											root_cmiss_region, region_name,
											/*master_fe_region*/root_fe_region,
											(struct MANAGER(FE_basis) *)NULL, (struct LIST(FE_element_shape) *)NULL);
										FE_region_begin_change(node_group);
										FE_region_clear(node_group, /*destroy_in_master*/1);
										set_Region_electrode_position_field(region,electrode_position_field);
										set_Region_rig_node_group(region,node_group);
									} break;
								}/*	switch (region_type) */	
							}	/*	if (MIXED==rig_type)  */
							else
							{						
								if (node_group) /* stop caching the current group,(if any) */
								{
									FE_region_end_change(node_group);
								}								
								/* Get group */
								node_group = Cmiss_region_get_or_create_child_FE_region(
									root_cmiss_region, region_name,
									/*master_fe_region*/root_fe_region,
									(struct MANAGER(FE_basis) *)NULL, (struct LIST(FE_element_shape) *)NULL);
								FE_region_begin_change(node_group);
								FE_region_clear(node_group, /*destroy_in_master*/1);
								set_Region_rig_node_group(region,node_group);
								/*destroy any exisiting template,field_order_info*/
								/* create the template node,(but may change it to auxiliary later*/	
								DESTROY(FE_node)(&template_node);
								DESTROY(FE_field_order_info)(&field_order_info);
								switch (config_node_type)
								{							
									case SOCK_ELECTRODE_TYPE:
									{
										template_node = create_config_template_node(
											SOCK_ELECTRODE_TYPE,focus,&field_order_info,package,
											&electrode_position_field);
										set_Region_electrode_position_field(region,electrode_position_field);
									} break;
									case TORSO_ELECTRODE_TYPE:
									{
										template_node = create_config_template_node(
											TORSO_ELECTRODE_TYPE,0,&field_order_info,package,
											&electrode_position_field);
										set_Region_electrode_position_field(region,electrode_position_field);
									} break;
									case PATCH_ELECTRODE_TYPE:
									{
										template_node = create_config_template_node(
											PATCH_ELECTRODE_TYPE,0,&field_order_info,package,
											&electrode_position_field);
										set_Region_electrode_position_field(region,electrode_position_field);
									} break;
								}	/* switch (config_node_type) */
							}			
							/* read the devices for the region */
							/* read the number of inputs */
							BINARY_FILE_READ((char *)&region_number_of_devices,
								sizeof(int),1,input_file);	
							/* create or reallocate the all_devices_node_order_info */
							if (!all_devices_node_order_info)
							{
								all_devices_node_order_info = CREATE(FE_node_order_info)
									(region_number_of_devices);

								*the_node_order_info = all_devices_node_order_info;
								if (!all_devices_node_order_info)
								{
									display_message(ERROR_MESSAGE,
										"read_binary_config_FE_node_group.  "
										"CREATE(FE_node_order_info) failed");
									return_code = 0;						
									node_group = (struct FE_region *)NULL;					
								}
							}
							else
							{ 
								if (!(return_code = add_nodes_FE_node_order_info(
									region_number_of_devices,all_devices_node_order_info)))
								{							
									display_message(ERROR_MESSAGE,
										"read_binary_config_FE_node_group.  "
										"add_nodes_FE_node_order_info failed");
									return_code = 0;						
									node_group = (struct FE_region *)NULL;	
								}
							}
							/* read in the inputs */
							read_order_number=0;
							while((region_number_of_devices>0)&&return_code)
							{
								BINARY_FILE_READ((char *)&device_type,sizeof(enum Device_type),1,
									input_file);
								/* if valid device type */
								if ((ELECTRODE==device_type)||(AUXILIARY==device_type))
								{													
									/* read the device number */
									BINARY_FILE_READ((char *)&device_number,sizeof(int),1,
										input_file);
									node_number = FE_region_get_next_FE_node_identifier(
										root_fe_region, last_node_number);
									/* read device type dependent properties */
									switch (device_type)
									{
										case ELECTRODE:
										{
											node=read_binary_config_FE_node(input_file, template_node,
												root_fe_region, config_node_type, node_number,
												read_order_number, field_order_info);
											last_node_number = node_number;	
										} break;
										case AUXILIARY:
										{	
											config_node_type = AUXILIARY_TYPE;									
											DESTROY(FE_node)(&template_node);
											DESTROY(FE_field_order_info)(&field_order_info);	
											template_node = create_config_template_node(config_node_type,
												0,&field_order_info,package,&electrode_position_field);
											node = read_binary_config_FE_node(input_file,
												template_node, root_fe_region, config_node_type,
												node_number, read_order_number, field_order_info);
											last_node_number = node_number;										
										} break;
									}	/* switch (device_type) */
									if (node)
									{ 
										if (FE_region_merge_FE_node(node_group, node))
										{
											if (FE_region_merge_FE_node(all_devices_node_group, node))
											{
												/* add the node to the node_order_info*/
												set_FE_node_order_info_node(all_devices_node_order_info,
													device_count, node);
												device_count++;
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"read_binary_config_FE_node_group.  "
													"Could not add node to all_devices_node_group");
												/* remove node from its ultimate master FE_region */
												FE_region_remove_FE_node(root_fe_region, node);
												node = (struct FE_node *)NULL;
												return_code = 0;
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"read_binary_config_FE_node_group.  "
												"Could not add node to node_group");
											/*???RC This was an existing potential leak */
											node = (struct FE_node *)NULL;
											return_code = 0;
										}
									}/*	if (node) */
								}/* if ((ELECTRODE==device_type)||(AUXILIARY==device_type)) */
								else
								{
									display_message(ERROR_MESSAGE,
										"read_binary_config_FE_node_group. Invalid device type");
									return_code = 0;
								}
								region_number_of_devices--;
								read_order_number++;
							} /*	while (region_number_of_devices>0)&&return_code) */					
						}	/* if (ALLOCATE(region_name */
						else
						{
							display_message(ERROR_MESSAGE,
								"read_binary_config_FE_node_group. Could not create region name");
							return_code =0;		
						}	
						if (return_code)
						{
							/* move to the next region */
							region_number++;				
						}
						region_item=get_Region_list_item_next(region_item);
						if (region_name)
						{
							DEALLOCATE(region_name);
						}				
					}/*	while((region_number<number_of_regions)&&return_code) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_binary_config_FE_node_group. rig->region_list is NULL");
					return_code = 0;
				}
				if (return_code)
				{
					/* read the number of pages */
					BINARY_FILE_READ((char *)&number_of_pages,sizeof(int),1,input_file);
					/* read in the pages */
					count=0;		
					while ((count<number_of_pages)&&return_code)
					{									
						/* read the page name */
						BINARY_FILE_READ((char *)&string_length,sizeof(int),1,input_file);
						if (ALLOCATE(page_name,char,string_length+1))
						{
							BINARY_FILE_READ(page_name,sizeof(char),string_length,input_file);
							page_name[string_length]='\0';
							/* read the number of devices in the device list */
							BINARY_FILE_READ((char *)&number_of_page_devices,sizeof(int),1,
								input_file);
							/* read the device list */					
							while ((number_of_page_devices>0)&&return_code)
							{
								BINARY_FILE_READ((char *)&device_number,sizeof(int),1,input_file);
								number_of_page_devices--;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_binary_config_FE_node_group.  Could not create page name");
							return_code = 0;
						}			 
						if (page_name)
						{
							DEALLOCATE(page_name);
						}
						count++;
					} /*	while ((count<number_of_pages)&&return_code) */
				}	/* if (success) */
				FE_region_end_change(all_devices_node_group);
			}
			else
			{
				display_message(ERROR_MESSAGE,"read_binary_config_FE_node_group.  "
					"Can't create all_devices_node_group");	
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"read_binary_config_FE_node_group. "
				"Can't read rig name");	
			return_code =0;		
		}	
		/* We've finished. Clean up */	
		if (template_node)
		{
			DESTROY(FE_node)(&template_node);
		}			
		if (node_group)
		{
			FE_region_end_change(node_group);
		}		
		if (field_order_info)
		{
			DESTROY(FE_field_order_info)(&field_order_info);
		}			
		if (rig_name)
		{
			DEALLOCATE(rig_name);
		}		
		if (region_name)
		{
			DEALLOCATE(region_name);
		}
		if (page_name)
		{
			DEALLOCATE(page_name);
		}	
	}
	else
	{
		return_code = 0;
		display_message(ERROR_MESSAGE,
			"read_binary_config_FE_node_group. Invalid arguments");
	}
	LEAVE;
	return (return_code);
} /* read_binary_config_FE_node_group */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int read_text_config_FE_node_group(FILE *input_file,
	struct Unemap_package *package,enum Region_type region_type,struct Rig *rig)
/*******************************************************************************
LAST MODIFIED : 6 July 2004

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
		*rig_name,separator,*trimmed_string;
	enum Config_node_type config_node_type;
	FE_value focus;
	int device_number,finished,is_auxiliary,is_electrode,last_node_number,
		node_number,read_order_number,region_number,return_code;
	int string_error =0;
	struct Cmiss_region *root_cmiss_region;
	struct FE_node *node, *template_node;
	struct FE_field *electrode_position_field;				
	struct FE_field_order_info *field_order_info;	
	struct FE_region *node_group, *all_devices_node_group, *root_fe_region;
	struct Region *region=(struct Region *)NULL;
	struct Region_list_item *region_item=(struct Region_list_item *)NULL;

	ENTER(read_text_config_FE_node_group);	
	region_name=(char *)NULL;
	template_node=(struct FE_node *)NULL;
	electrode_position_field = (struct FE_field *)NULL;	
	field_order_info=(struct FE_field_order_info *)NULL;
	dummy_str=(char *)NULL;
	node_group=(struct FE_region *)NULL; 	
	all_devices_node_group=(struct FE_region *)NULL;
	read_order_number=0;
	device_number=0;
	region_number=0;
	return_code=1;
	if (input_file&&package) 
	{
		root_cmiss_region = get_unemap_package_root_Cmiss_region(package);
		/* we've read in the rig type, now read in the rig name */
		fscanf(input_file,"%*[ :\n]");
		/* rig name */		 
		if (read_string(input_file,"[^\n]",&rig_name))
		{
			if (trimmed_string=trim_string(rig_name))
			{
				DEALLOCATE(rig_name);
				rig_name=trimmed_string;
			}
			fscanf(input_file," ");						
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
			root_fe_region = Cmiss_region_get_FE_region(root_cmiss_region);
			if (all_devices_node_group = Cmiss_region_get_or_create_child_FE_region(
				root_cmiss_region, "all_devices", /*master_fe_region*/root_fe_region,
				(struct MANAGER(FE_basis) *)NULL, (struct LIST(FE_element_shape) *)NULL))
			{
				FE_region_begin_change(all_devices_node_group);
				FE_region_clear(all_devices_node_group, /*destroy_in_master*/1);
				set_Rig_all_devices_rig_node_group(rig,all_devices_node_group);
				last_node_number=1;	 
				fscanf(input_file," ");				
				finished=0;
				if (region_item=get_Rig_region_list(rig))
				{
					while ((!feof(input_file))&&(!finished)&&return_code)
					{
						if (region_item)
							/* not an error if it isn't set*/
						{
							region=get_Region_list_item_region(region_item);
						}					
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
							if (read_string(input_file,"[^\n]",&rig_name))
							{
								if (trimmed_string=trim_string(rig_name))
								{
									DEALLOCATE(rig_name);
									rig_name=trimmed_string;
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
									" Can't read torso rig name");	
								return_code =0;	
							}
							config_node_type=TORSO_ELECTRODE_TYPE;
							/*destroy any existing template,field_order_info*/						
							/* create the template node */						
							DESTROY(FE_node)(&template_node);
							DESTROY(FE_field_order_info)(&field_order_info);
							/* stop caching the current group,(if any) */
							if (node_group)
							{
								FE_region_end_change(node_group);
							}	
							template_node=create_config_template_node(TORSO_ELECTRODE_TYPE,0,
								&field_order_info,package,&electrode_position_field);
							node_group = Cmiss_region_get_or_create_child_FE_region(
								root_cmiss_region, rig_name, /*master_fe_region*/root_fe_region,
								(struct MANAGER(FE_basis) *)NULL, (struct LIST(FE_element_shape) *)NULL);
							FE_region_begin_change(node_group);
							FE_region_clear(node_group, /*destroy_in_master*/1);
							set_Region_electrode_position_field(region,electrode_position_field);
							set_Region_rig_node_group(region,node_group);
							/* new group, new rig, so reset device number */
							device_number=0;
						}
						else if ((!strcmp("patch",input_str))&&(region_type==MIXED))
						{	
							/* get the rig name */	
							fscanf(input_file,"%*[ :\n]");
							if (read_string(input_file,"[^\n]",&rig_name))
							{
								if (trimmed_string=trim_string(rig_name))
								{
									DEALLOCATE(rig_name);
									rig_name=trimmed_string;
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
									" Can't read patch rig name");	
								return_code=0;	
							}
							config_node_type=PATCH_ELECTRODE_TYPE;
							/*destroy any existing template,field_order_info*/	
							/* create the template node */					
							DESTROY(FE_node)(&template_node);
							DESTROY(FE_field_order_info)(&field_order_info);	
							/* stop caching the current group,(if any) */
							if (node_group)
							{
								FE_region_end_change(node_group);
							}		
							template_node = create_config_template_node(PATCH_ELECTRODE_TYPE,
								0,&field_order_info,package,&electrode_position_field);
							node_group = Cmiss_region_get_or_create_child_FE_region(
								root_cmiss_region, rig_name, /*master_fe_region*/root_fe_region,
								(struct MANAGER(FE_basis) *)NULL, (struct LIST(FE_element_shape) *)NULL);
							FE_region_begin_change(node_group);
							FE_region_clear(node_group, /*destroy_in_master*/1);
							set_Region_electrode_position_field(region,electrode_position_field);
							set_Region_rig_node_group(region,node_group);
							/* new group, new rig, so reset device number */
							device_number=0;
						}
						else if ((!strcmp("sock ",input_str))&&(region_type==MIXED))
						{
							/* shorten string, 'sock' only 4 chars long*/
							input_str[4]='\0';
							/* get the rig name */
							fscanf(input_file,"%*[ :\n]");
							if (read_string(input_file,"[^\n]",&rig_name))
							{
								if (trimmed_string=trim_string(rig_name))
								{
									DEALLOCATE(rig_name);
									rig_name=trimmed_string;
								}
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
								return_code=0;	
							}
							config_node_type=SOCK_ELECTRODE_TYPE;
							/*destroy any existing template,field_order_info*/	
							/* create the template node */						
							DESTROY(FE_node)(&template_node);
							DESTROY(FE_field_order_info)(&field_order_info);	
							/* stop caching the current group,(if any) */
							if (node_group)
							{
								FE_region_end_change(node_group);
							}			
							template_node=create_config_template_node(SOCK_ELECTRODE_TYPE,focus,
								&field_order_info,package,&electrode_position_field);
							node_group = Cmiss_region_get_or_create_child_FE_region(
								root_cmiss_region, rig_name, /*master_fe_region*/root_fe_region,
								(struct MANAGER(FE_basis) *)NULL, (struct LIST(FE_element_shape) *)NULL);
							FE_region_begin_change(node_group);
							FE_region_clear(node_group, /*destroy_in_master*/1);
							set_Region_electrode_position_field(region,electrode_position_field);
							set_Region_rig_node_group(region,node_group);
							/* new group, new rig, so reset device number */
							device_number=0;
						}				
						else if (!strcmp("regio",input_str))
						{
							/* line is "region" */
							fscanf(input_file,"n : "); 
							region_name=(char *)NULL;
							if (read_string(input_file,"[^\n]",&region_name))
							{
								if (trimmed_string=trim_string(region_name))
								{
									DEALLOCATE(region_name);
									region_name=trimmed_string;
								}
							}
							/*append the region number to the name, to ensure it's unique*/
							sprintf(region_num_string,"%d",region_number);
							append_string(&region_name,region_num_string,&string_error);
							region_number++;						
							/* stop caching the current group,(if any) */
							if (node_group)
							{
								FE_region_end_change(node_group);
							}								
							/* get group */
							node_group = Cmiss_region_get_or_create_child_FE_region(
								root_cmiss_region, region_name,
								/*master_fe_region*/root_fe_region,
								(struct MANAGER(FE_basis) *)NULL, (struct LIST(FE_element_shape) *)NULL);
							FE_region_begin_change(node_group);
							FE_region_clear(node_group, /*destroy_in_master*/1);
							set_Region_rig_node_group(region,node_group);											
							/*destroy any existing template,field_order_info*/	
							/* create the template node (but may change it to auxiliary later) */					
							DESTROY(FE_node)(&template_node);
							DESTROY(FE_field_order_info)(&field_order_info);
							switch (config_node_type)
							{							
								case SOCK_ELECTRODE_TYPE:
								{
									template_node=create_config_template_node(SOCK_ELECTRODE_TYPE,
										focus,&field_order_info,package,&electrode_position_field);
									set_Region_electrode_position_field(region,electrode_position_field);
								} break;
								case TORSO_ELECTRODE_TYPE:
								{
									template_node=create_config_template_node(TORSO_ELECTRODE_TYPE,0,
										&field_order_info,package,&electrode_position_field);
									set_Region_electrode_position_field(region,electrode_position_field);
								} break;
								case PATCH_ELECTRODE_TYPE:
								{
									template_node=create_config_template_node(PATCH_ELECTRODE_TYPE,0,
										&field_order_info,package,&electrode_position_field);
									set_Region_electrode_position_field(region,electrode_position_field);
								} break;
							}
							/* new group, new rig, so reset device number */
							device_number=0;
						}				
						else if ((is_electrode = !strcmp("elect",input_str))||
							(is_auxiliary=!strcmp("auxil",input_str)))
						{
							/* node, line is "electrode" or "auxiliary" */
							/* complete the word */
							fscanf(input_file,"%4c",input_str);
							node_number = FE_region_get_next_FE_node_identifier(
								root_fe_region, last_node_number);
							fscanf(input_file," ");							
							/* have we got an auxiliary node? */
							if (is_auxiliary)
							{
								config_node_type=AUXILIARY_TYPE;							
								DESTROY(FE_node)(&template_node);
								DESTROY(FE_field_order_info)(&field_order_info);	
								template_node=create_config_template_node(config_node_type,0,
									&field_order_info,package,&electrode_position_field);
								node = read_text_config_FE_node(input_file, template_node,
									root_fe_region, config_node_type, node_number,
									read_order_number, field_order_info);
								last_node_number=node_number;	
								device_number++;
								read_order_number++;					
							}
							else if (is_electrode)
							{							
								/* not an auxiliary node,electrode node! */
								node = read_text_config_FE_node(input_file, template_node,
									root_fe_region, config_node_type, node_number,
									read_order_number, field_order_info);	
								last_node_number=node_number;
								device_number++;
								read_order_number++;
							}						
							if (node)
							{ 
								if (FE_region_merge_FE_node(node_group, node))
								{
									if (!FE_region_merge_FE_node(all_devices_node_group, node))
									{
										display_message(ERROR_MESSAGE,
											"read_text_config_FE_node_group.  "
											"Could not add node to all_devices_node_group");
										/* remove node from its ultimate master FE_region */
										FE_region_remove_FE_node(root_fe_region, node);
										node = (struct FE_node *)NULL;
										return_code = 0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_text_config_FE_node_group.  "
										"Could not add node to node_group");
									/*???RC This was an existing potential leak */
									node = (struct FE_node *)NULL;
									return_code = 0;
								}
							}/*	if (node) */
						} /* else if ((is_electrode = */
						else
						{
							finished=1;
						}
						fscanf(input_file," ");
						DEALLOCATE(dummy_str); 
						if (region_item)
							/* not an error if it isn't set*/
						{
							/* move to the next region */
							region_item=get_Region_list_item_next(region_item);					
						}						
					} /* while((!feof(input_file))&&(!finished)) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_text_config_FE_node_group. rig->region_list is NULL");
					return_code =0;		
				}
				if (return_code&&!feof(input_file))
				{
					if (!strcmp("pages",input_str))			
					{				
						/* read the first page name */
						if (read_string(input_file,"s",&name))
						{
							fscanf(input_file," %c",&separator);
							while (!feof(input_file)&&return_code&&(':'==separator))
							{													
								/* read the list of devices */
								fscanf(input_file," ");
								separator=',';					
								while (!feof(input_file)&&return_code&&(separator!=':'))
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
											return_code=0;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"read_text_config_FE_node_group."
											"  Could not create device name for page");
										return_code=0;
									}
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_text_config_FE_node_group.  Could not create page name");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_text_config_FE_node_group.  Invalid device type");
						return_code=0;
					}
				}
				FE_region_end_change(all_devices_node_group);
			}
			else
			{
				display_message(ERROR_MESSAGE,"read_text_config_FE_node_group.  "
					"Can't create all_devices_node_group");	
				return_code = 0;
			}
		}	
		else
		{	
			display_message(ERROR_MESSAGE,
				"read_text_config_FE_node_group. Can't read rig name");	
			return_code=0;	
		}					
		/* we've finished. Clean up */
		/* clear template node */			
		if (template_node)
		{
			DESTROY(FE_node)(&template_node);
		}
		if (node_group)
		{
			FE_region_end_change(node_group);
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
		return_code=0;
		display_message(ERROR_MESSAGE,
			"read_text_config_FE_node_group. Invalid arguments");
	}
	LEAVE;

	return (return_code);
} /* read_text_config_FE_node_group */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int read_config_FE_node_group(FILE *input_file,
	struct Unemap_package *unemap_package,
	enum Region_type region_type,	enum Rig_file_type file_type,
	struct FE_node_order_info **node_order_info,struct Rig *rig)
/*******************************************************************************
LAST MODIFIED : 21 July 2000

DESCRIPTION :
Reads a node group from a configuration file into the rig
cf read_FE_node_group() in import_finite_element.c
==============================================================================*/
{		
	int return_code;

	ENTER(read_config_FE_node_group);	
	return_code=0;
	if (input_file&&unemap_package)
	{				
		if (TEXT==file_type)
		{
			return_code=read_text_config_FE_node_group(input_file,unemap_package,region_type,
				rig);
		}
		else
		{
			/* must be binary */
			return_code=read_binary_config_FE_node_group(input_file,unemap_package,
				region_type,node_order_info,rig);
		}		
	}
	else
	{	
		return_code=0;
		display_message(ERROR_MESSAGE,
			"read_config_FE_node_group. Invalid arguments");
	}
	LEAVE;

	return (return_code);
} /* read_config_FE_node_group */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int read_signal_FE_node_group(FILE *input_file,
	struct Unemap_package *package,
	struct FE_node_order_info *node_order_info)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Reads signals from a signal file, stores them in a node group, via 
FE_node_order_info.  Performs the same loading functions as read_signal_file,
but using the rig_node group.  Doesn't load in auxilliary devices that are
linear combinations of other channels.
==============================================================================*/
{
	char
	*channel_gain_component_names[1]=
	{
		"gain_value"
	},	
	*channel_offset_component_names[1]=
	{
		"offset_value"
	},
#if defined (UNEMAP_USE_NODES)	
	*display_start_time_component_names[1]=
	{
		"display start time"
	},	
	*display_end_time_component_names[1]=
	{
		"display end time"
	},
#endif /* defined (UNEMAP_USE_NODES)*/	
	*signal_maximum_component_names[1]=
	{
		"maximum_value"
	},
	*signal_minimum_component_names[1]=
	{
		"minimum_value"
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
#if defined (UNEMAP_USE_NODES)	
	FE_value end_time,start_time;
#endif /* defined (UNEMAP_USE_NODES)*/
	float *buffer_signals_float,*buffer_value,*channel_gains,*channel_offsets,frequency;
	int *buffer_times,channel_number,count,fread_result,i,j,number_of_nodes,
		number_of_samples,number_of_devices,
		number_of_signals,return_code,temp_int;
	short int *buffer_signals_short,*node_signals_short;
	struct Cmiss_region *root_cmiss_region;
	struct Coordinate_system coordinate_system;		
	struct FE_field *channel_gain_field,*channel_offset_field,
#if defined (UNEMAP_USE_NODES)
		*display_start_time_field,*display_end_time_field,
#endif /* defined (UNEMAP_USE_NODES)*/
		*signal_field,*signal_maximum_field,
		*signal_minimum_field,*signal_status_field;
	struct FE_field_component component;
	struct FE_node *device_node,*node,*node_managed;
	struct FE_node_field_creator *node_field_creator;
	struct FE_region *root_fe_region;
	struct FE_time_version *fe_time_version;
#if defined(NEW_CODE) /* not using yet, may use later*/
	int index;
#endif

	ENTER(read_signal_FE_node_group);
	return_code=1;
	fread_result=0;
	buffer_signals_short=(short int *)NULL;
	buffer_times=(int *)NULL;
	buffer_signals_float=(float *)NULL;
	fe_time_version = (struct FE_time_version *)NULL;
	node_signals_short=(short int *)NULL;
	node_signals_fe_value=(FE_value *)NULL;
	node=(struct FE_node *)NULL;
	node_managed=(struct FE_node *)NULL;
	signal_field=(struct FE_field *)NULL;
	channel_gain_field=(struct FE_field *)NULL;
	channel_offset_field=(struct FE_field *)NULL;
#if defined (UNEMAP_USE_NODES)
	display_start_time_field=(struct FE_field *)NULL;
	display_end_time_field=(struct FE_field *)NULL;
#endif /* defined (UNEMAP_USE_NODES)*/
	signal_minimum_field=(struct FE_field *)NULL;
	signal_maximum_field=(struct FE_field *)NULL;
	times=(FE_value *)NULL;
	if (input_file && package && node_order_info &&
		(root_cmiss_region = get_unemap_package_root_Cmiss_region(package)) &&
		(root_fe_region = Cmiss_region_get_FE_region(root_cmiss_region)))
	{
		number_of_devices = get_FE_node_order_info_number_of_nodes(
			node_order_info);
		coordinate_system.type=NOT_APPLICABLE;
		if (ALLOCATE(channel_gains,float,number_of_devices)&&
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
						if (ALLOCATE(buffer_times,int,number_of_samples))
						{
							if (number_of_samples==(int)BINARY_FILE_READ((char *)buffer_times,
								sizeof(int),number_of_samples,input_file))
							{
								/* read the signals */
								switch (signal_value_type)
								{
									case SHORT_INT_VALUE:
									{
										if (ALLOCATE(buffer_signals_short,short int,number_of_samples*
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
										if (ALLOCATE(buffer_signals_float,float,number_of_samples*
											number_of_signals))
										{
											fread_result=BINARY_FILE_READ((char *)buffer_signals_float,
												sizeof(float),number_of_samples*number_of_signals,input_file);
											/* check signal values.  If it's non a valid float, set it to 0  */
											buffer_value=buffer_signals_float;
											for(i=0;i<number_of_samples*number_of_signals;i++)
											{	
												/* check if data is valid finite() checks inf and nan*/
												if (!finite( (double)(*buffer_value)  ))
												{
													*buffer_value=0.0;
													display_message(ERROR_MESSAGE,
														"read_signal_file.Signal value is infinite or not a number "
														"Set to 0 ");
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
											/*time*/0,&channel_number))&&(channel_number>0))
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
									if (return_code)
									{	
										period = 1/frequency;									
										/* allocate memory for times, and fill in */
										if (ALLOCATE(times,FE_value,number_of_samples))
										{								
											for(j=0;j<number_of_samples;j++)
											{
												times[j] = buffer_times[j]*period;										
											}	
											/*used a little later in fields display_start_time display_start_time*/
#if defined (UNEMAP_USE_NODES)
											start_time=times[0];
											end_time=times[number_of_samples-1];											
#endif /* defined (UNEMAP_USE_NODES)*/
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
												if (ALLOCATE(node_signals_short,short int,number_of_samples))
												{
													if (!(signal_field=FE_region_get_FE_field_with_properties(
														root_fe_region,"signal",GENERAL_FE_FIELD,
														/*indexer_field*/(struct FE_field *)NULL,
														/*number_of_indexed_values*/0,CM_GENERAL_FIELD,
														&coordinate_system,SHORT_VALUE,
														/*number_of_components*/1,signal_component_names,
														/*number_of_times*/0,/*time_value_type*/
														UNKNOWN_VALUE,
														(struct FE_field_external_information *)NULL)))
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
												if (ALLOCATE(node_signals_fe_value,float,number_of_samples))
												{
													if (!(signal_field=FE_region_get_FE_field_with_properties(
														root_fe_region,"signal",GENERAL_FE_FIELD,
														/*indexer_field*/(struct FE_field *)NULL,
														/*number_of_indexed_values*/0,CM_GENERAL_FIELD,
														&coordinate_system,FE_VALUE_VALUE,
														/*number_of_components*/1,signal_component_names,
														/*number_of_values*/0,/*time_value_type*/UNKNOWN_VALUE,
														(struct FE_field_external_information *)NULL)))
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
										if (signal_field)
										{
											for(j=0;j<number_of_samples;j++)
											{
												set_FE_field_time_FE_value(signal_field,j,times[j]);
											}
											if (!(fe_time_version =
												FE_region_get_FE_time_version_matching_series(
													root_fe_region, number_of_samples, times)))
											{
												return_code = 0;
											}
										}
									}							
									if (return_code)
									{
										/* All these fields use the same node_field_creator
											as they have one component with no derivative
											and no versions */
										if (node_field_creator = CREATE(FE_node_field_creator)(
											/*number_of_components*/1))
										{
											number_of_nodes=get_FE_node_order_info_number_of_nodes(
												node_order_info);
											FE_region_begin_change(root_fe_region);
											for(i=0;i<number_of_nodes;i++)
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
												/* create a copy of the node to work with */
												if (node = CREATE(FE_node)(
													get_FE_node_identifier(node_managed),
													(struct FE_region *)root_fe_region, node_managed))
												{
													/* add channel_gain and channel_offset fields to node */
													if (channel_gain_field=FE_region_get_FE_field_with_properties(
														root_fe_region,"channel_gain",GENERAL_FE_FIELD,
														/*indexer_field*/(struct FE_field *)NULL,
														/*number_of_indexed_values*/0,CM_GENERAL_FIELD,
														&coordinate_system,FE_VALUE_VALUE,
														/*number_of_components*/1,channel_gain_component_names,
														/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
														(struct FE_field_external_information *)NULL))
													{
														if (define_FE_field_at_node(node,channel_gain_field,
															(struct FE_time_version *)NULL,
															node_field_creator))
														{	
															/* add it to the unemap package */
															set_unemap_package_channel_gain_field(package,
																channel_gain_field);
															/* set the values*/
															component.number = 0;
															component.field = channel_gain_field; 
															set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
																/*time*/0,channel_gains[i]);
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
													if (channel_offset_field=FE_region_get_FE_field_with_properties(
														root_fe_region,"channel_offset",GENERAL_FE_FIELD,
														/*indexer_field*/(struct FE_field *)NULL,
														/*number_of_indexed_values*/0,CM_GENERAL_FIELD,
														&coordinate_system,FE_VALUE_VALUE,
														/*number_of_components*/1,channel_offset_component_names,
														/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
														(struct FE_field_external_information *)NULL))
													{ 
														if (define_FE_field_at_node(node,channel_offset_field,
																 (struct FE_time_version *)NULL,
																 node_field_creator))
														{	
															/* add it to the unemap package */
															set_unemap_package_channel_offset_field(package,
																channel_offset_field);
															/* set the values*/
															component.number = 0;
															component.field = channel_offset_field; 
															set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
																/*time*/0,channel_offsets[i]);
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
													if (signal_minimum_field=FE_region_get_FE_field_with_properties(
														root_fe_region,"signal_minimum",GENERAL_FE_FIELD,
														/*indexer_field*/(struct FE_field *)NULL,
														/*number_of_indexed_values*/0,CM_GENERAL_FIELD,
														&coordinate_system,FE_VALUE_VALUE,
														/*number_of_components*/1,signal_minimum_component_names,
														/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
														(struct FE_field_external_information *)NULL))
													{
														if (define_FE_field_at_node(node,signal_minimum_field,
															(struct FE_time_version *)NULL,
															node_field_creator))
														{	
															/* add it to the unemap package */
															set_unemap_package_signal_minimum_field(package,
																signal_minimum_field);
															/*set the signal_minimum_field and signal_maximum_field fields */
															component.number = 0;
															component.field = signal_minimum_field;												
															set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
																/*time*/0,1.0);
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
													if (signal_maximum_field=FE_region_get_FE_field_with_properties(
														root_fe_region,"signal_maximum",GENERAL_FE_FIELD,
														/*indexer_field*/(struct FE_field *)NULL,
														/*number_of_indexed_values*/0,CM_GENERAL_FIELD,
														&coordinate_system,FE_VALUE_VALUE,
														/*number_of_components*/1,signal_maximum_component_names,
														/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
														(struct FE_field_external_information *)NULL))
													{
														if (define_FE_field_at_node(node,signal_maximum_field,
															(struct FE_time_version *)NULL,
															node_field_creator))
														{	
															/* add it to the unemap package */
															set_unemap_package_signal_maximum_field(package,
																signal_maximum_field);
															component.number = 0;
															component.field = signal_maximum_field; 
															set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
																/*time*/0,0.0);
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
													if (signal_status_field=FE_region_get_FE_field_with_properties(
														root_fe_region,"signal_status",GENERAL_FE_FIELD,
														/*indexer_field*/(struct FE_field *)NULL,
														/*number_of_indexed_values*/0,CM_GENERAL_FIELD,
														&coordinate_system,STRING_VALUE,
														/*number_of_components*/1,signal_status_component_names,
														/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
														(struct FE_field_external_information *)NULL))
													{ 
														if (define_FE_field_at_node(node,signal_status_field,
																 (struct FE_time_version *)NULL,
																 node_field_creator))
														{	
															/* add it to the unemap package */													
															set_unemap_package_signal_status_field(package,
																signal_status_field);
															/* need to set status based upon the nodal device_type */ 
															/* status may be 'loaded over' later */ 
															get_FE_nodal_string_value(node,
																get_unemap_package_device_type_field(package),
																0,0,FE_NODAL_VALUE,&device_type_string);
															if (strcmp(device_type_string,"ELECTRODE"))
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
													if (define_FE_field_at_node(node,signal_field,
														fe_time_version, node_field_creator))
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
																for (j=0;j<number_of_samples;j++)
																{
																	set_FE_nodal_short_value(node,&component,0,
																		FE_NODAL_VALUE,times[j],node_signals_short[j]);
																}
															} break;
															case FLOAT_VALUE:
															{
																for (j=0;j<number_of_samples;j++)
																{
																	set_FE_nodal_FE_value_value(node,&component,0,
																		FE_NODAL_VALUE,times[j],node_signals_fe_value[j]);
																}
															} break;
														}	/* switch (signal_value_type) */											
													}
													else
													{
														display_message(ERROR_MESSAGE,"read_signal_FE_node_group."
															" define_field_at_node failed");							
														return_code=0;
													}
													/* NOT for UNEMAP_USE_3D */
#if defined (UNEMAP_USE_NODES)
													/* create the display start time field, add it to the node */	
													/*NOTE: it's a CONSTANT_FE_FIELD, values stored t field, not node */
													if (display_start_time_field=FE_region_get_FE_field_with_properties(
														root_fe_region,"display_start_time",CONSTANT_FE_FIELD,
														/*indexer_field*/(struct FE_field *)NULL,
														/*number_of_indexed_values*/0,CM_GENERAL_FIELD,
														&coordinate_system,FE_VALUE_VALUE,
														/*number_of_components*/1,display_start_time_component_names,
														/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
														(struct FE_field_external_information *)NULL))
													{
														if (define_FE_field_at_node(node,display_start_time_field,
															(struct FE_time_version *)NULL,
															node_field_creator))
														{	
															/* add it to the unemap package */
															set_unemap_package_display_start_time_field(package,
																display_start_time_field);													
															set_FE_field_FE_value_value(display_start_time_field,0,start_time);
														}
														else
														{
															display_message(ERROR_MESSAGE,"read_signal_FE_node_group."
																"error defining display_start_time_field");	
															return_code =0;
														}	
													}
													else
													{
														display_message(ERROR_MESSAGE,"read_signal_FE_node_group."
															"error getting  display_start_time_field");	
														return_code =0;
													}	
													/* create the display start time field, add it to the node */	
													/*NOTE: it's a CONSTANT_FE_FIELD, values stored t field, not node */
													if (display_end_time_field=FE_region_get_FE_field_with_properties(
														root_fe_region,"display_end_time",CONSTANT_FE_FIELD,
														/*indexer_field*/(struct FE_field *)NULL,
														/*number_of_indexed_values*/0,CM_GENERAL_FIELD,
														&coordinate_system,FE_VALUE_VALUE,
														/*number_of_components*/1,display_end_time_component_names,
														/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
														(struct FE_field_external_information *)NULL))
													{
														if (define_FE_field_at_node(node,display_end_time_field,
															(struct FE_time_version *)NULL,
															node_field_creator))
														{	
															/* add it to the unemap package */
															set_unemap_package_display_end_time_field(package,
																display_end_time_field);													
															set_FE_field_FE_value_value(display_end_time_field,0,end_time);
														}
														else
														{
															display_message(ERROR_MESSAGE,"read_signal_FE_node_group."
																"error defining display_end_time_field");	
															return_code =0;
														}	
													}
													else
													{
														display_message(ERROR_MESSAGE,"read_signal_FE_node_group."
															"error getting  display_end_time_field");	
														return_code =0;
													}	
#endif /* (UNEMAP_USE_NODES) */
													/* merge it back into the region */
													FE_region_merge_FE_node(root_fe_region, node);
													/* destroy the working copy */
													DESTROY(FE_node)(&node);
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"read_signal_FE_node_group.  "
														"CREATE(FE_node) failed");
													return_code=0;
												}
											}
											FE_region_end_change(root_fe_region);
											DESTROY(FE_node_field_creator)(
												&node_field_creator);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"read_signal_FE_node_group.  "
												"Unable to make node field creator.");
											return_code=0;
										}
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
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int read_event_settings_and_signal_status_FE_node_group(FILE *input_file,
	struct Unemap_package *package,struct FE_node_order_info *node_order_info)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

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
	struct Cmiss_region *root_cmiss_region;
	struct FE_node *node,*node_managed;
	struct FE_field *signal_maximum_field,*signal_minimum_field,*signal_status_field;
	struct FE_field_component component;
	struct FE_region *root_fe_region;

	ENTER(read_event_settings_and_signal_status_FE_node_group);
	return_code = 0;
	node=(struct FE_node *)NULL;
	node_managed=(struct FE_node *)NULL;
	signal_maximum_field=(struct FE_field *)NULL;
	signal_minimum_field=(struct FE_field *)NULL;
	signal_status_field=(struct FE_field *)NULL;
	if (input_file && package && node_order_info &&
		(root_cmiss_region = get_unemap_package_root_Cmiss_region(package)) &&
		(root_fe_region = Cmiss_region_get_FE_region(root_cmiss_region)))
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
			signal_maximum_field=
				get_unemap_package_signal_maximum_field(package);	
			signal_minimum_field=
				get_unemap_package_signal_minimum_field(package);
			signal_status_field=
				get_unemap_package_signal_status_field(package);	
			number_of_devices = 
				get_FE_node_order_info_number_of_nodes(node_order_info);
			/* for each signal read the status, range and events */
			if (((number_of_devices)>0)&& signal_maximum_field&&
				signal_minimum_field&&signal_status_field)
			{
				i=0;
				return_code=1;
				FE_region_begin_change(root_fe_region);
				while (return_code&&(i<number_of_devices))
				{
					/* read the status and range */
					/* if no signal_status,  a linear comb auxiliary device. Do nothing*/
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
									number_of_events--;
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

						/* this */
						/* put (some of!) the read info into the node */
						node_managed = get_FE_node_order_info_node(node_order_info,i);
						/* create a copy of the node to work with */
						if (node = CREATE(FE_node)(get_FE_node_identifier(node_managed),
							(struct FE_region *)NULL, node_managed))
						{
							component.number=0;
							component.field=signal_minimum_field;
							/* fields have already been defined at the node in read_signal_FE_node_group*/
							set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
								/*time*/0,signal_minimum);
							component.field=signal_maximum_field;
							set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
								/*time*/0,signal_maximum);
							switch (signal_status)
							{
								case ACCEPTED:
								{
									set_FE_nodal_string_value(node,signal_status_field,0,0,FE_NODAL_VALUE,
										"ACCEPTED");
								} break;	
								case REJECTED:
								{
									set_FE_nodal_string_value(node,signal_status_field,0,0,FE_NODAL_VALUE,
										"REJECTED");
								} break;	
								case UNDECIDED:						
								{
									set_FE_nodal_string_value(node,signal_status_field,0,0,FE_NODAL_VALUE,
										"UNDECIDED");
								} break;		
								default:	
								{								
									display_message(ERROR_MESSAGE,
										"read_event_settings_and_signal_status_FE_node_group."
										"  incorrect signal_status");
									return_code=0;							
								} break;					
							}
							/* merge node back into fe_region */
							FE_region_merge_FE_node(root_fe_region, node);
							/* destroy the working copy */
							DESTROY(FE_node)(&node);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_event_settings_and_signal_status_FE_node_group.  "
								"CREATE(FE_node) failed");
							return_code = 0;
						}
					}
					i++;
				}/* while (return_code&&(i<number_of_devices))*/
				FE_region_end_change(root_fe_region);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_event_settings_and_signal_status_FE_node_group.  "
					"node or field info incorrect");
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

	return (return_code);
}/*read_event_settings_and_signal_status_FE_node_group. */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int get_node_position_min_max(struct FE_node *node,
	void *position_min_max_iterator_void)
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
Finds the min and max coordinates of the  <position_field>
in the <node>.
==============================================================================*/
{	
	struct FE_field_component component;
	FE_value c0,c1,c2,dest_coordinates[3],source_coordinates[3],x_value,
		y_value,z_value;
	int return_code;			
	struct Position_min_max_iterator *position_min_max_iterator;
	struct Coordinate_system *source_coordinate_system,dest_coordinate_system;

	return_code=1;
	ENTER(get_node_position_min_max);
	source_coordinate_system=(struct Coordinate_system *)NULL;
	if ((node)&&(position_min_max_iterator_void))
	{
		position_min_max_iterator=(struct Position_min_max_iterator *)
			position_min_max_iterator_void;	
		if (position_min_max_iterator&&position_min_max_iterator->position_field)
		{													 
			component.field=position_min_max_iterator->position_field;
			component.number=0;
			get_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,/*time*/0,&c0);
			component.number=1;
			get_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,/*time*/0,&c1);
			component.number=2;
			get_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,/*time*/0,&c2);
			source_coordinate_system=get_FE_field_coordinate_system(component.field);
			dest_coordinate_system.type=RECTANGULAR_CARTESIAN;
			source_coordinates[0]=c0;
			source_coordinates[1]=c1;
			source_coordinates[2]=c2;
			convert_Coordinate_system(source_coordinate_system,3,source_coordinates,
				&dest_coordinate_system,3,dest_coordinates,(float *)NULL);
			x_value=dest_coordinates[0];
			y_value=dest_coordinates[1];
			z_value=dest_coordinates[2];

			if (x_value>position_min_max_iterator->max_x)
			{
				position_min_max_iterator->max_x=x_value;
			}
			if (x_value<position_min_max_iterator->min_x)
			{
				position_min_max_iterator->min_x=x_value;
			}									
			if (y_value>position_min_max_iterator->max_y)
			{
				position_min_max_iterator->max_y=y_value;
			}
			if (y_value<position_min_max_iterator->min_y)
			{
				position_min_max_iterator->min_y=y_value;
			}			
			if (z_value>position_min_max_iterator->max_z)
			{
				position_min_max_iterator->max_z=z_value;
			}
			if (z_value<position_min_max_iterator->min_z)
			{
				position_min_max_iterator->min_z=z_value;
			}			
			position_min_max_iterator->count++;
		}	/* if (min_max_iterator */
		else
		{
			display_message(ERROR_MESSAGE,"get_node_position_min_max."
				" min_max_iterator NULL ");
			return_code=0;
		}	
	}/* if ((node)&&(position_min_max_iterator_void)) */
	else
	{
		display_message(ERROR_MESSAGE,"get_node_position_min_max."
			" Invalid argument");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* get_node_position_min_max*/
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int iterative_get_rig_node_signal_min_max_at_time(struct FE_node *node,
	void *min_max_iterator_void)
/*******************************************************************************
LAST MODIFIED : 19 February 2002

DESCRIPTION :
Get the signal <value> stored at the <node>, <min_max_iterator->component->field>
at time <min_max_iterator->time>.
Compares this value to <min_max_iterator->max>,<min_max_iterator->min>, and adjusts 
these accordingly.
This function is called iteratively by get_rig_node_group_signal_min_max_at_time
Does nothing with REJECTED signals.
Note: up to calling function to call Computed_field_clear_cache for
scaled_offset_signal_value_at_time_field
==============================================================================*/
{	
	char *signal_status_string;	
	FE_value fe_value;
	int return_code;		
	struct Min_max_iterator *min_max_iterator;
	
	return_code=1;
	ENTER(iterative_get_rig_node_signal_min_max_at_time);
	if ((node)&&(min_max_iterator_void))
	{
		min_max_iterator=(struct Min_max_iterator *)min_max_iterator_void;	
		if (min_max_iterator&&min_max_iterator->signal_status_field)
		{
			/* need to check if signal is accepted/rejected/undecided */	
			if (get_FE_nodal_string_value(node,min_max_iterator->signal_status_field,0,0,
				FE_NODAL_VALUE,&signal_status_string))
			{
				/* do nothing if signal is rejected */
				if (strcmp(signal_status_string,"REJECTED")) /* strcmp rets 0 for match*/
				{															
					Computed_field_evaluate_at_node(
						min_max_iterator->scaled_offset_signal_value_at_time_field,
						node,min_max_iterator->time,&fe_value/*assume 1 component*/);				
					if (return_code)
					{
						min_max_iterator->count++;
						if (fe_value>min_max_iterator->max)
						{
							min_max_iterator->max=fe_value;
						}
						if (fe_value<min_max_iterator->min)
						{
							min_max_iterator->min=fe_value;
						}			
					}/* if (return_code) */
				} /* if (strcmp(signal_status_string,*/
				DEALLOCATE(signal_status_string);
			}	/* if (get_FE_nodal_string_value */
			else
			{
				display_message(ERROR_MESSAGE,"iterative_get_rig_node_signal_min_max_at_time."
					"Failed to get signal_status");
				return_code=0;	
			}				
		}	/* if (min_max_iterator */
		else
		{
			display_message(ERROR_MESSAGE,"iterative_get_rig_node_signal_min_max_at_time."
			" min_max_iterator NULL ");
			return_code=0;
		}	
	}/* if ((node)&&(min_max_iterator_void)) */
	else
	{
		display_message(ERROR_MESSAGE,"iterative_get_rig_node_signal_min_max_at_time."
			" Invalid argument");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* iterative_get_rig_node_signal_min_max_at_time*/
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int iterative_rig_node_set_map_electrode_position_lambda_r(struct FE_node *node,
	void *set_map_electrode_position_info_void)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Sets the node's nodal map_electrode_postions from the nodal electrode_positions, 
and changes the node's map_electrode_postions lambda or r  values

This function is called iteratively by 
rig_node_group_set_map_electrode_position_lambda_r
==============================================================================*/
{
	FE_value lambda,mu,theta,r,x,y,z_cp,z_rc;	
	int return_code;	
	struct FE_field_component component;	
	struct FE_field *electrode_position_field,*map_electrode_position_field;
	struct FE_node *node_unmanaged;	
	struct FE_region *fe_region;
	struct Set_map_electrode_position_info *set_map_electrode_position_info;
	/* ROUND_TORSO defined in rig_node.h */
#if !defined (ROUND_TORSO) /*FOR AJP*/
	FE_value c,torso_x,s,t,torso_major_r,torso_minor_r,torso_y;
#endif /* defined (ROUND_TORSO)*/

	ENTER(iterative_rig_node_set_map_electrode_position_lambda_r);
	return_code=1;
	node_unmanaged=(struct FE_node *)NULL;
	if (node && (set_map_electrode_position_info =
		(struct Set_map_electrode_position_info *)
		set_map_electrode_position_info_void))
	{
		if ((fe_region = set_map_electrode_position_info->fe_region) &&
			(map_electrode_position_field=
				set_map_electrode_position_info->map_electrode_position_field))
		{
			if ((electrode_position_field=
				FE_region_get_FE_field_from_name(fe_region,
					(char *)sock_electrode_position_str)) &&
				(FE_field_is_defined_at_node(electrode_position_field,node)) ||
				(electrode_position_field =
					FE_region_get_FE_field_from_name(fe_region,
						(char *)torso_electrode_position_str)) &&
				(FE_field_is_defined_at_node(electrode_position_field,node)) ||
				(electrode_position_field=
					FE_region_get_FE_field_from_name(fe_region,
						(char *)patch_electrode_position_str)) &&
				(FE_field_is_defined_at_node(electrode_position_field,node)))
				/* if we don't have an electrode_position_field, it's an auxilliary
					 node -- do nothing */
			{
				if (node_unmanaged = CREATE(FE_node)(get_FE_node_identifier(node),
					(struct FE_region *)NULL, node))
				{
					switch (set_map_electrode_position_info->region_type)
					{
						case SOCK:
						{	
							/* get the source data*/
							component.field=electrode_position_field;
							component.number=0;/* lambda */
							get_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,/*time*/0,&lambda);
							component.number=1;/* mu */
							get_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,/*time*/0,&mu);
							component.number=2;/* theta */
							get_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,/*time*/0,&theta);
							/* change lambda*/
							lambda=set_map_electrode_position_info->value1;
							/* set the dest data*/
							component.field=map_electrode_position_field;
							component.number=0;/* lambda */
							set_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,/*time*/0,lambda);
							component.number=1;/* mu */
							set_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,/*time*/0,mu);
							component.number=2;/* theta */
							set_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,/*time*/0,theta);							
						} break;	
						case TORSO:
						{				
							/* Note that the torso map is in cp coords, but the torso */
							/* electrode_positions are in rc, so torso map_electrode_positions are in cp */
							/* get the source data stored as cartesian */
							component.field=electrode_position_field;
							component.number=0;/* x*/
							get_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,/*time*/0,&x);	
							component.number=1;/* y */
							get_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,/*time*/0,&y);
							component.number=2;/* z */
							get_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,/*time*/0,&z_rc);
							/* convert coords to cylindrical polar */
							cartesian_to_cylindrical_polar(x,y,z_rc,&r,&theta,&z_cp,(float *)NULL);
							/* change the r */
							/* ROUND_TORSO defined in rig_node.h */
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
								FE_NODAL_VALUE,/*time*/0,r); 
							component.number=1;/* theta */	
							set_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,/*time*/0,theta);
							component.number=2;/* z_cp */	
							set_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,/*time*/0,z_cp);							
						} break;
						case PATCH:
						{		
							/* Data to copy, but none to change */
							/* get the source data*/
							component.field=electrode_position_field;
							component.number=0;/* x */
							get_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,/*time*/0,&x);
							component.number=1;/* y */
							get_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,/*time*/0,&y);
							/* set the dest data*/
							component.field=map_electrode_position_field;
							component.number=0;/* x */
							set_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,/*time*/0,x);
							component.number=1;/* y */
							set_FE_nodal_FE_value_value(node_unmanaged,&component,0,
								FE_NODAL_VALUE,/*time*/0,y);															
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"iterative_rig_node_set_map_electrode_position_lambda_r"
								"  invalid region_type");
							return_code=0;
						} break;
					}
					/* merge node back into the fe_region */
					FE_region_merge_FE_node(fe_region, node_unmanaged);
					DESTROY(FE_node)(&node_unmanaged);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"iterative_rig_node_set_map_electrode_position_lambda_r  "
						"CREATE(FE_node) failed");
					return_code=0;
				}
			}
		}	/* if (set_map_electrode_position_info */
		else
		{
			display_message(ERROR_MESSAGE,"iterative_rig_node_set_map_electrode_position_lambda_r"
				" set_map_electrode_position_info NULL ");
			return_code=0;
		}	
	}/* if ((node)&&(set_map_electrode_position_info_void)) */
	else
	{
		display_message(ERROR_MESSAGE,"iterative_rig_node_set_map_electrode_position_lambda_r"
			" Invalid argument");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* rig_node_set_map_electrode_position_lambda_r */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int rig_node_add_map_electrode_position_field(struct FE_node *node,
	void *add_map_electrode_position_info_void)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Add a map  electrode_position_field  to the rig_node.

Called by rig_node_group_add_map_electrode_position_field
==============================================================================*/
{	
	int return_code;	
	struct Add_map_electrode_position_info *add_map_electrode_position_info;	
	struct FE_field *map_electrode_position_field,*electrode_position_field;
	struct FE_node *node_unmanaged;
	struct FE_node_field_creator *node_field_creator;
	struct FE_region *fe_region;

	ENTER(rig_node_add_map_electrode_position_field);
	return_code=1;
	node_unmanaged=(struct FE_node *)NULL;
	electrode_position_field=(struct FE_field *)NULL;
	map_electrode_position_field=(struct FE_field *)NULL;	
	if (node && (add_map_electrode_position_info =
		(struct Add_map_electrode_position_info *)
		add_map_electrode_position_info_void))
	{
		if ((map_electrode_position_field=
			add_map_electrode_position_info->map_electrode_position_field)&&
			(fe_region = add_map_electrode_position_info->fe_region))
		{					
			/* if we have a electrode_position_field, want a map_electrode_position_field */
			/* if we don't it's an auxilliary node-do nothing. */
			if ((electrode_position_field=
				FE_region_get_FE_field_from_name(fe_region,
					(char *)sock_electrode_position_str)) &&
				(FE_field_is_defined_at_node(electrode_position_field,node)) ||
				(electrode_position_field=
					FE_region_get_FE_field_from_name(fe_region,
						(char *)torso_electrode_position_str)) &&
				(FE_field_is_defined_at_node(electrode_position_field,node)) ||
				(electrode_position_field=
					FE_region_get_FE_field_from_name(fe_region,
						(char *)patch_electrode_position_str)) &&
				(FE_field_is_defined_at_node(electrode_position_field,node)))
			{	
				/* if its already deined, have nothing to do */
				if (!FE_field_is_defined_at_node(map_electrode_position_field,node))
				{
					if (node_unmanaged = CREATE(FE_node)(get_FE_node_identifier(node),
						(struct FE_region *)NULL, node))
					{
						if (node_field_creator = CREATE(FE_node_field_creator)(
							/*number_of_components*/3))
						{
							if (!define_FE_field_at_node(node_unmanaged,map_electrode_position_field,
								(struct FE_time_version *)NULL, node_field_creator))
							{
								display_message(ERROR_MESSAGE,"rig_node_add_map_electrode_position_field."
									"error defining electrode_position_field");	
								return_code =0;
							}	
							/* copy node back into the manager */
							FE_region_merge_FE_node(fe_region, node_unmanaged);
							DESTROY(FE_node_field_creator)(&node_field_creator);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"rig_node_add_map_electrode_position_field.  "
								"Unable to make node field creator.");
							return_code=0;
						}
						DESTROY(FE_node)(&node_unmanaged);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"rig_node_add_map_electrode_position_field.  "
							"CREATE(FE_node) failed");
						return_code=0;
					}
				}
			}/* if ((electrode_position_field= */
		}	/* if (add_map_electrode_position_info */
		else
		{
			display_message(ERROR_MESSAGE,"rig_node_add_map_electrode_position_field"
				" add_map_electrode_position_info NULL ");
			return_code=0;
		}	
	}/* if ((node)&&(add_map_electrode_position_info_void)) */
	else
	{
		display_message(ERROR_MESSAGE,"rig_node_add_map_electrode_position_field"
			" Invalid argument");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* rig_node_add_map_electrode_position_field */
#endif /* defined (UNEMAP_USE_3D) */


/*
Global functions
----------------
*/

#if defined (UNEMAP_USE_3D)
struct Min_max_iterator *CREATE(Min_max_iterator)(void)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION:
Create a Min_max_iterator, set fields to NULL/0.
==============================================================================*/
{	
	struct  Min_max_iterator *min_max_iterator;

	ENTER(CREATE(Min_max_iterator));
	min_max_iterator=(struct  Min_max_iterator *)NULL;
	if (ALLOCATE(min_max_iterator,struct Min_max_iterator,1))
	{
		min_max_iterator->max=0;
		min_max_iterator->min=0;
		min_max_iterator->time=0;
		min_max_iterator->count=0;
		min_max_iterator->started=0; /*have we started accumulating info yet? */	
#if defined (UNEMAP_USE_NODES)
		min_max_iterator->display_start_time_field=(struct FE_field *)NULL;
		min_max_iterator->display_end_time_field=(struct FE_field *)NULL;
#endif /* defined (UNEMAP_USE_NODES) */
		min_max_iterator->signal_minimum_field=(struct FE_field *)NULL;
		min_max_iterator->signal_maximum_field=(struct FE_field *)NULL;
		min_max_iterator->signal_status_field=(struct FE_field *)NULL;	
		min_max_iterator->scaled_offset_signal_value_at_time_field=
			(struct Computed_field *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Unemap_package).  Could not allocate memory ");
		DEALLOCATE(min_max_iterator);
	}
	LEAVE;
	return(min_max_iterator);
}

int DESTROY(Min_max_iterator)(struct Min_max_iterator **iterator_address)
/*******************************************************************************
LAST MODIFIED : 8 August 2000 

DESCRIPTION :
Destroy a Min_max_iterator. Don't DEACCESS fields, 'cos never accessed them.
This is only temp references for iteration.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Min_max_iterator));
	if (iterator_address)
	{				
		DEALLOCATE(*iterator_address);		
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Min_max_iterator) */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int get_Min_max_iterator_started(struct Min_max_iterator *min_max_iterator, 
	int *started)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the started of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator&&started)
	{
		return_code=1;
		*started=min_max_iterator->started;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"get_Min_max_iterator_started. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* get_Min_max_iterator_started */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int set_Min_max_iterator_started(struct Min_max_iterator *min_max_iterator, int started)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the started of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator)
	{
		return_code=1;
		min_max_iterator->started=started;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"set_Min_max_iterator_started. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* set_Min_max_iterator_started */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_3D)
int get_Min_max_iterator_count(struct Min_max_iterator *min_max_iterator, 
	int *count)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the count of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator&&count)
	{
		return_code=1;
		*count=min_max_iterator->count;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"get_Min_max_iterator_count. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* get_Min_max_iterator_count */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int set_Min_max_iterator_count(struct Min_max_iterator *min_max_iterator, int count)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the count of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator)
	{
		return_code=1;
		min_max_iterator->count=count;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"set_Min_max_iterator_count. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* set_Min_max_iterator_count */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int get_Min_max_iterator_time(struct Min_max_iterator *min_max_iterator, 
	FE_value *time)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the time of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator&&time)
	{
		return_code=1;
		*time=min_max_iterator->time;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"get_Min_max_iterator_time. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* get_Min_max_iterator_time */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int set_Min_max_iterator_time(struct Min_max_iterator *min_max_iterator, FE_value time)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the time of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator)
	{
		return_code=1;
		min_max_iterator->time=time;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"set_Min_max_iterator_time. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* set_Min_max_iterator_time */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int get_Min_max_iterator_min(struct Min_max_iterator *min_max_iterator, 
	FE_value *min)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the min of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator&&min)
	{
		return_code=1;
		*min=min_max_iterator->min;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"get_Min_max_iterator_min. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* get_Min_max_iterator_min */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int set_Min_max_iterator_min(struct Min_max_iterator *min_max_iterator, FE_value min)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the min of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator)
	{
		return_code=1;
		min_max_iterator->min=min;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"set_Min_max_iterator_min. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* set_Min_max_iterator_min */

int get_Min_max_iterator_max(struct Min_max_iterator *min_max_iterator, 
	FE_value *max)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the max of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator&&max)
	{
		return_code=1;
		*max=min_max_iterator->max;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"get_Min_max_iterator_max. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* get_Min_max_iterator_max */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int set_Min_max_iterator_max(struct Min_max_iterator *min_max_iterator, FE_value max)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the max of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator)
	{
		return_code=1;
		min_max_iterator->max=max;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"set_Min_max_iterator_max. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* set_Min_max_iterator_max */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_NODES)
int get_Min_max_iterator_display_start_time_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *display_start_time_field)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the display_start_time_field of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator&&display_start_time_field)
	{
		return_code=1;
		display_start_time_field=min_max_iterator->display_start_time_field;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"get_Min_max_iterator_display_start_time_field. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* get_Min_max_iterator_display_start_time_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Min_max_iterator_display_start_time_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *display_start_time_field)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the display_start_time_field of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator)
	{
		return_code=1;
		min_max_iterator->display_start_time_field=display_start_time_field;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"set_Min_max_iterator_display_start_time_field. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* set_Min_max_iterator_display_start_time_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int get_Min_max_iterator_display_end_time_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *display_end_time_field)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the display_end_time_field of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator&&display_end_time_field)
	{
		return_code=1;
		display_end_time_field=min_max_iterator->display_end_time_field;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"get_Min_max_iterator_display_end_time_field. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* get_Min_max_iterator_display_end_time_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Min_max_iterator_display_end_time_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *display_end_time_field)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the display_end_time_field of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator)
	{
		return_code=1;
		min_max_iterator->display_end_time_field=display_end_time_field;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"set_Min_max_iterator_display_end_time_field. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* set_Min_max_iterator_display_end_time_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int get_Min_max_iterator_signal_minimum_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *signal_minimum_field)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the signal_minimum_field of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator&&signal_minimum_field)
	{
		return_code=1;
		signal_minimum_field=min_max_iterator->signal_minimum_field;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"get_Min_max_iterator_signal_minimum_field. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* get_Min_max_iterator_signal_minimum_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Min_max_iterator_signal_minimum_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *signal_minimum_field)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the signal_minimum_field of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator)
	{
		return_code=1;
		min_max_iterator->signal_minimum_field=signal_minimum_field;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"set_Min_max_iterator_signal_minimum_field. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* set_Min_max_iterator_signal_minimum_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int get_Min_max_iterator_signal_maximum_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *signal_maximum_field)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the signal_maximum_field of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator&&signal_maximum_field)
	{
		return_code=1;
		signal_maximum_field=min_max_iterator->signal_maximum_field;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"get_Min_max_iterator_signal_maximum_field. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* get_Min_max_iterator_signal_maximum_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Min_max_iterator_signal_maximum_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *signal_maximum_field)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the signal_maximum_field of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator)
	{
		return_code=1;
		min_max_iterator->signal_maximum_field=signal_maximum_field;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"set_Min_max_iterator_signal_maximum_field. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* set_Min_max_iterator_signal_maximum_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_3D)
int get_Min_max_iterator_signal_status_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *signal_status_field)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the signal_status_field of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator&&signal_status_field)
	{
		return_code=1;
		signal_status_field=min_max_iterator->signal_status_field;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"get_Min_max_iterator_signal_status_field. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* get_Min_max_iterator_signal_status_field */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int set_Min_max_iterator_signal_status_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *signal_status_field)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the signal_status_field of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator)
	{
		return_code=1;
		min_max_iterator->signal_status_field=signal_status_field;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"set_Min_max_iterator_signal_status_field. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* set_Min_max_iterator_signal_status_field */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int get_Min_max_iterator_scaled_offset_signal_value_at_time_field(
	struct Min_max_iterator *min_max_iterator, 
	struct Computed_field *scaled_offset_signal_value_at_time_field)
/*******************************************************************************
LAST MODIFIED :19 February 2002

DESCRIPTION :
gets the scaled_offset_signal_value_at_time_field of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator&&scaled_offset_signal_value_at_time_field)
	{
		return_code=1;
		scaled_offset_signal_value_at_time_field=
			min_max_iterator->scaled_offset_signal_value_at_time_field;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"get_Min_max_iterator_scaled_offset_signal_value_at_time_field. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* get_Min_max_iterator_scaled_offset_signal_value_at_time_field */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int set_Min_max_iterator_scaled_offset_signal_value_at_time_field(
	struct Min_max_iterator *min_max_iterator, 
	struct Computed_field *scaled_offset_signal_value_at_time_field)
/*******************************************************************************
LAST MODIFIED :19 February 2002

DESCRIPTION :
Sets the scaled_offset_signal_value_at_time_field of Min_max_iterato
==============================================================================*/
{
	int return_code;

	if (min_max_iterator)
	{
		return_code=1;
		min_max_iterator->scaled_offset_signal_value_at_time_field=
			scaled_offset_signal_value_at_time_field;
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"set_Min_max_iterator_scaled_offset_signal_value_at_time_field. Invalid arguments");	
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* set_Min_max_iterator_scaled_offset_signal_value_at_time_field */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_NODES)
DECLARE_OBJECT_FUNCTIONS(Signal_drawing_package)
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct Signal_drawing_package *CREATE(Signal_drawing_package)(void)
/*******************************************************************************
LAST MODIFIED :  9 July 1999

DESCRIPTION :
Create a Signal_drawing_package, set all it's fields to NULL.
==============================================================================*/
{
	struct Signal_drawing_package *package;

	ENTER(CREATE(Signal_drawing_package));	
	if (ALLOCATE(package,struct Signal_drawing_package,1))
	{
		/* fields of the rig_nodes */
		package->device_name_field=(struct FE_field *)NULL;		
		package->device_type_field=(struct FE_field *)NULL;	
		package->channel_number_field=(struct FE_field *)NULL;
		package->display_start_time_field=(struct FE_field *)NULL;
		package->display_end_time_field=(struct FE_field *)NULL;
		package->read_order_field=(struct FE_field *)NULL;
		package->highlight_field=(struct FE_field *)NULL;
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
			"CREATE(Signal_drawing_package).  Could not allocate memory");
		DEALLOCATE(package);
	}
	LEAVE;

	return (package);
} /* CREATE(Signal_drawing_package) */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int DESTROY(Signal_drawing_package)(struct Signal_drawing_package **package_address)
/*******************************************************************************
LAST MODIFIED : 9 July 1999

DESCRIPTION :
Frees the memory for the Signal_drawing_package node  and 
sets <*package_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct Signal_drawing_package *package;

	ENTER(DESTROY(Signal_drawing_package));
	if ((package_address)&&(package=*package_address))
	{	
		DEACCESS(FE_field)(&(package->device_name_field));	
		DEACCESS(FE_field)(&(package->device_type_field));
		DEACCESS(FE_field)(&(package->channel_number_field));
		DEACCESS(FE_field)(&(package->display_start_time_field));
		DEACCESS(FE_field)(&(package->display_end_time_field));
		DEACCESS(FE_field)(&(package->read_order_field));
		DEACCESS(FE_field)(&(package->highlight_field));
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
} /* DESTROY(Signal_drawing_package) */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_device_name_field(
	struct Signal_drawing_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
gets the field of the Signal_drawing_package.
==============================================================================*/
{	
	struct FE_field *device_name_field;

	ENTER(get_Signal_drawing_package_device_name_field);
	if (package)
	{	
		device_name_field=package->device_name_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Signal_drawing_package_device_name_field.  Invalid argument");
		device_name_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (device_name_field);
} /* get_Signal_drawing_package_device_name_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_device_name_field(struct Signal_drawing_package *package,
	struct FE_field *device_name_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the Signal_drawing_package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Signal_drawing_package_device_name_field);
	if (package)
	{
		return_code=1;	
		REACCESS(FE_field)(&(package->device_name_field),device_name_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Signal_drawing_package_device_name_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Signal_drawing_package_device_name__field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_device_type_field(
	struct Signal_drawing_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Gets the field of the Signal_drawing_package.
==============================================================================*/
{	
	struct FE_field *device_type_field;

	ENTER(get_Signal_drawing_package_device_type_field);
	if (package)
	{	
		device_type_field=package->device_type_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Signal_drawing_package_device_type_field.  Invalid argument");
		device_type_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (device_type_field);
} /* get_Signal_drawing_package_device_type_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_device_type_field(struct Signal_drawing_package *package,
	struct FE_field *device_type_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the Signal_drawing_package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Signal_drawing_package_device_type_field);
	if (package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->device_type_field),device_type_field);	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Signal_drawing_package_device_type_field.  Invalid argument");
		return_code =0;
	}
	LEAVE;

	return (return_code);
} /* set_Signal_drawing_package_device_type_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_channel_number_field(
	struct Signal_drawing_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Gets the field of the Signal_drawing_package.
==============================================================================*/
{	
	struct FE_field *channel_number_field;

	ENTER(get_Signal_drawing_package_channel_number_field);
	if (package)
	{	
		channel_number_field=package->channel_number_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Signal_drawing_package_channel_number_field.  Invalid argument");
		channel_number_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (channel_number_field);
} /* get_Signal_drawing_package_channel_number_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_channel_number_field(struct Signal_drawing_package *package,
	struct FE_field *channel_number_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the Signal_drawing_package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Signal_drawing_package_channel_number_field);
	if (package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->channel_number_field),channel_number_field);		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Signal_drawing_package_channel_number_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Signal_drawing_package_channel_number_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_display_start_time_field(
	struct Signal_drawing_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Gets the field of the Signal_drawing_package.
==============================================================================*/
{	
	struct FE_field *display_start_time_field;

	ENTER(get_Signal_drawing_package_display_start_time_field);
	if (package)
	{	
		display_start_time_field=package->display_start_time_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Signal_drawing_package_display_start_time_field.  Invalid argument");
		display_start_time_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (display_start_time_field);
} /* get_Signal_drawing_package_display_start_time_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_display_start_time_field(struct Signal_drawing_package *package,
	struct FE_field *display_start_time_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the Signal_drawing_package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Signal_drawing_package_display_start_time_field);
	if (package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->display_start_time_field),display_start_time_field);		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Signal_drawing_package_display_start_time_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Signal_drawing_package_display_start_time_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_display_end_time_field(
	struct Signal_drawing_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Gets the field of the Signal_drawing_package.
==============================================================================*/
{	
	struct FE_field *display_end_time_field;

	ENTER(get_Signal_drawing_package_display_end_time_field);
	if (package)
	{	
		display_end_time_field=package->display_end_time_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Signal_drawing_package_display_end_time_field.  Invalid argument");
		display_end_time_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (display_end_time_field);
} /* get_Signal_drawing_package_display_end_time_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_display_end_time_field(struct Signal_drawing_package *package,
	struct FE_field *display_end_time_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the Signal_drawing_package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Signal_drawing_package_display_end_time_field);
	if (package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->display_end_time_field),display_end_time_field);		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Signal_drawing_package_display_end_time_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Signal_drawing_package_display_end_time_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_read_order_field(
	struct Signal_drawing_package *package)
/*******************************************************************************
LAST MODIFIED : 26 July 2000

DESCRIPTION :
Gets the field of the Signal_drawing_package.
==============================================================================*/
{	
	struct FE_field *read_order_field;

	ENTER(get_Signal_drawing_package_read_order_field);
	if (package)
	{	
		read_order_field=package->read_order_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Signal_drawing_package_read_order_field.  Invalid argument");
		read_order_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (read_order_field);
} /* get_Signal_drawing_package_read_order_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_read_order_field(struct Signal_drawing_package *package,
	struct FE_field *read_order_field)
/*******************************************************************************
LAST MODIFIED : 26 July 2000

DESCRIPTION :
Sets the field of the Signal_drawing_package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Signal_drawing_package_read_order_field);
	if (package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->read_order_field),read_order_field);		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Signal_drawing_package_read_order_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Signal_drawing_package_read_order_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_highlight_field(
	struct Signal_drawing_package *package)
/*******************************************************************************
LAST MODIFIED : 26 July 2000

DESCRIPTION :
Gets the field of the Signal_drawing_package.
==============================================================================*/
{	
	struct FE_field *highlight_field;

	ENTER(get_Signal_drawing_package_highlight_field);
	if (package)
	{	
		highlight_field=package->highlight_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Signal_drawing_package_highlight_field.  Invalid argument");
		highlight_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (highlight_field);
} /* get_Signal_drawing_package_highlight_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_highlight_field(struct Signal_drawing_package *package,
	struct FE_field *highlight_field)
/*******************************************************************************
LAST MODIFIED : 26 July 2000

DESCRIPTION :
Sets the field of the Signal_drawing_package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Signal_drawing_package_highlight_field);
	if (package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->highlight_field),highlight_field);		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Signal_drawing_package_highlight_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Signal_drawing_package_highlight_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_signal_field(struct Signal_drawing_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Gets the field of the Signal_drawing_package.
==============================================================================*/
{	
	struct FE_field *signal_field;

	ENTER(get_Signal_drawing_package_signal_field);
	if (package)
	{	
		signal_field=package->signal_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Signal_drawing_package_signal_field.  Invalid argument");
		signal_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (signal_field);
} /* get_Signal_drawing_package_signal_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_signal_field(struct Signal_drawing_package *package,
	struct FE_field *signal_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the Signal_drawing_package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Signal_drawing_package_signal_field);
	if (package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->signal_field),signal_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Signal_drawing_package_signal_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Signal_drawing_package_signal_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_signal_status_field
 (struct Signal_drawing_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Gets the field of the Signal_drawing_package.
==============================================================================*/
{	
	struct FE_field *signal_status_field;

	ENTER(get_Signal_drawing_package_signal_status_field);
	if (package)
	{	
		signal_status_field=package->signal_status_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Signal_drawing_package_signal_status_field.  Invalid argument");
		signal_status_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (signal_status_field);
} /* get_Signal_drawing_package_signal_status_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_signal_status_field(struct Signal_drawing_package *package,
	struct FE_field *signal_status_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the Signal_drawing_package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Signal_drawing_package_signal_status_field);
	if (package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->signal_status_field),signal_status_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Signal_drawing_package_signal_status_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Signal_drawing_package_signal_status_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_signal_minimum_field
 (struct Signal_drawing_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Gets the field of the Signal_drawing_package.
==============================================================================*/
{	
	struct FE_field *signal_minimum_field;

	ENTER(get_Signal_drawing_package_signal_minimum_field);
	if (package)
	{	
		signal_minimum_field=package->signal_minimum_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Signal_drawing_package_signal_minimum_field.  Invalid argument");
		signal_minimum_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (signal_minimum_field);
} /* get_Signal_drawing_package_signal_minimum_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_signal_minimum_field(struct Signal_drawing_package *package,
	struct FE_field *signal_minimum_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the Signal_drawing_package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Signal_drawing_package_signal_minimum_field);
	if (package)
	{
		return_code=1;	
		REACCESS(FE_field)(&(package->signal_minimum_field),signal_minimum_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Signal_drawing_package_signal_minimum_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Signal_drawing_package_signal_minimum_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_signal_maximum_field
 (struct Signal_drawing_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Gets the field of the Signal_drawing_package.
==============================================================================*/
{	
	struct FE_field *signal_maximum_field;

	ENTER(get_Signal_drawing_package_signal_maximum_field);
	if (package)
	{	
		signal_maximum_field=package->signal_maximum_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Signal_drawing_package_signal_maximum_field.  Invalid argument");
		signal_maximum_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (signal_maximum_field);
} /* get_Signal_drawing_package_signal_maximum_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_signal_maximum_field(struct Signal_drawing_package *package,
	struct FE_field *signal_maximum_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the Signal_drawing_package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Signal_drawing_package_signal_maximum_field);
	if (package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->signal_maximum_field),signal_maximum_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Signal_drawing_package_signal_maximum_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Signal_drawing_package_signal_maximum_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_channel_offset_field(
	struct Signal_drawing_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Gets the field of the Signal_drawing_package.
==============================================================================*/
{	
	struct FE_field *channel_offset_field;

	ENTER(get_Signal_drawing_package_channel_offset_field);
	if (package)
	{	
		channel_offset_field=package->channel_offset_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Signal_drawing_package_channel_offset_field.  Invalid argument");
		channel_offset_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (channel_offset_field);
} /* get_Signal_drawing_package_channel_offset_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_channel_offset_field(struct Signal_drawing_package *package,
	struct FE_field *channel_offset_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the Signal_drawing_package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Signal_drawing_package_channel_offset_field);
	if (package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->channel_offset_field),channel_offset_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Signal_drawing_package_channel_offset_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Signal_drawing_package_channel_offset_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_channel_gain_field(
	struct Signal_drawing_package *package)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Gets the field of the Signal_drawing_package.
==============================================================================*/
{	
	struct FE_field *channel_gain_field;

	ENTER(get_Signal_drawing_package_fields);
	if (package)
	{	
		channel_gain_field=package->channel_gain_field; 	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Signal_drawing_package_channel_gain_field.  Invalid argument");
		channel_gain_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (channel_gain_field);
} /* get_Signal_drawing_package_channel_gain_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_channel_gain_field(struct Signal_drawing_package *package,
	struct FE_field *channel_gain_field)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the field of the Signal_drawing_package.
==============================================================================*/
{
	int return_code;	

	ENTER(set_Signal_drawing_package_channel_gain_field);
	if (package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->channel_gain_field),channel_gain_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Signal_drawing_package_channel_gain_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Signal_drawing_package_channel_gain_field */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_3D)
struct FE_region *make_unrejected_node_group(
	struct Cmiss_region *root_cmiss_region,
	struct FE_region *rig_node_group,struct FE_field *signal_status_field)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
makes and returns unrejected_node_group, consisting of all the unrejected 
(ie accepted or undecided) nodes in <rig_node_group>. Adds the group to the 
manager.
==============================================================================*/
{
	char *rig_node_group_name;
	char *unrejected_name;
	char *unrejected_str = "_unrejected";
	int child_number;
	struct Cmiss_region *rig_node_cmiss_region;
	struct FE_region *unrejected_node_group;

	ENTER(make_unrejected_node_group);
	rig_node_group_name = (char *)NULL;
	unrejected_node_group = (struct FE_region *)NULL;
	if (root_cmiss_region && rig_node_group && signal_status_field &&
		FE_region_get_Cmiss_region(rig_node_group,
			&rig_node_cmiss_region) &&
		Cmiss_region_get_child_region_number(root_cmiss_region,
			rig_node_cmiss_region, &child_number) &&
		Cmiss_region_get_child_region_name(root_cmiss_region,
			child_number, &rig_node_group_name))
	{
		/* construct the unrejected group name */
		if (ALLOCATE(unrejected_name, char,
			strlen(rig_node_group_name) + strlen(unrejected_str) + 1))
		{
			strcpy(unrejected_name, rig_node_group_name);
			strcat(unrejected_name, unrejected_str);
			/* make the group */
			if (unrejected_node_group = Cmiss_region_get_or_create_child_FE_region(
				root_cmiss_region, unrejected_name,
				/*master_fe_region*/Cmiss_region_get_FE_region(root_cmiss_region),
				(struct MANAGER(FE_basis) *)NULL, (struct LIST(FE_element_shape) *)NULL))
			{
				FE_region_begin_change(unrejected_node_group);
				FE_region_clear(unrejected_node_group, /*destroy_in_master*/1);
				FE_region_for_each_FE_node_conditional(rig_node_group,
					node_signal_is_unrejected, (void *)signal_status_field,
					FE_region_merge_FE_node_iterator, (void *)unrejected_node_group);
				FE_region_end_change(unrejected_node_group);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"make_unrejected_node_group. Couldn't CREATE unrejected_node_group");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "make_unrejected_node_group.  "
				"Could not allocate memory for unrejected_name");		
		}
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"make_unrejected_node_group.  Invalid argument");	
	}
	if (rig_node_group_name)
	{
		DEALLOCATE(rig_node_group_name);
	}
	LEAVE;

	return(unrejected_node_group);
} /* make_unrejected_node_group */
#endif /* defined (UNEMAP_USE_3D) */

int extract_signal_information(struct FE_node *device_node,
	struct Signal_drawing_package *signal_drawing_package,struct Device *device,
	int signal_number,int first_data,int last_data,
	int *number_of_signals_address,int *number_of_values_address,
	float **times_address,float **values_address,
	enum Event_signal_status **status_address,char **name_address,
	int *highlight_address,float *signal_minimum_address,
	float *signal_maximum_address)
/*******************************************************************************
LAST MODIFIED : 6 September 2002

DESCRIPTION :
Extracts the specified signal information.  The specification arguments are:
- <device_node>, <signal_drawing_package>, <device> identify where the
	information is stored. Either supply a <device>, with <device_node>,
	<signal_drawing_package> NULL to extract info from <device>, or supply a
	<device_node> and a <signal_drawing_package> with <device> = NULL, to extract
	info from <device_node>.
- <signal_number> specifies which signal (zero indicates all)	
- if device is set (device_node is NULL) <first_data>, <last_data> specify the 
  part of the signal required. If <first_data> is greater than <last_data> then 
  return the whole signal
- if device_node is set (device is NULL)<first_data>, <last_data> not used,
  <device_node> fields display_start(end)_time used instead
The extraction arguments are:
- <number_of_signals_address> if not NULL, <*number_of_signals_address> is set
	to the number of signals extracted
- <number_of_values_address> if not NULL, <*number_of_values_address> is set to
	the number of values extracted for each signal
- <times_address> if not NULL, an array with number of values entries is
	allocated, filled in with the times and assigned to <*times_address>
- <values_address> if not NULL, an array with (number of signals)*(number of
	values) entries is allocated, filled in with the values and assigned to
	<*values_address>.  In the extracted <*values_address> array the signal varies
	fastest ie the values for all signals at a particular time are sequential
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
	float signal_minimum,signal_maximum;
	float	*signals_values=(float *)NULL;
	float	*times=(float *)NULL;
	int highlight,number_of_signals,number_of_values,return_code;

	ENTER(extract_signal_information);
	return_code=0;
	if (
#if defined (UNEMAP_USE_NODES)
		((device_node&&signal_drawing_package)&&(!device))||
#endif /* defined (UNEMAP_USE_NODES) */
		(device&&(device->description)&&(!device_node)&&(!signal_drawing_package)))
	{
#if defined (UNEMAP_USE_NODES)
		if (device)
		{
#endif /* defined (UNEMAP_USE_NODES) */
#if defined (OLD_CODE)
			float buffer_frequency,channel_offset,channel_gain,*data_value_float,
				*signal_value;
			int buffer_offset,first_data_local,i,j,k,length,last_data_local;
			short int *data_value_short_int=(short int *)NULL;
			struct Signal *signal=(struct Signal *)NULL;
			struct Signal_buffer *buffer=(struct Signal_buffer *)NULL;
#if defined (DEVICE_EXPRESSIONS)
			float *channel_value,*channel_values;
			int number_of_channel_devices;
			struct Device **channel_devices=(struct Device **)NULL;
#else /* defined (DEVICE_EXPRESSIONS) */
			float	*electrode_coefficients=(float *)NULL;
			int linear_combination,number_of_electrodes;
			struct Device **electrodes=(struct Device **)NULL;
#endif /* defined (DEVICE_EXPRESSIONS) */

#if defined (DEVICE_EXPRESSIONS)
			number_of_channel_devices=0;
			channel_devices=(struct Device **)NULL;
			if (calculate_device_channels(device,&number_of_channel_devices,
				&channel_devices)&&(0<number_of_channel_devices))
			{
				/* check that electodes have same number of signals and that all
					signals are from the same buffer */
				if ((signal=(*channel_devices)->signal)&&(buffer=signal->buffer))
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
						while (return_code&&(i<number_of_channel_devices))
						{
							if (signal=channel_devices[i]->signal)
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
								if (j<number_of_signals)
								{
									number_of_signals=j;
								}
							}
							i++;
						}
						if (number_of_signals<=0)
						{
							return_code=0;
						}
					}
				}
			}
#else /* defined (DEVICE_EXPRESSIONS) */
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
					(0<(number_of_electrodes=(device->description->properties).auxiliary.
					number_of_electrodes)))
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
#endif /* defined (DEVICE_EXPRESSIONS) */
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
#if defined (DEVICE_EXPRESSIONS)
								if (ALLOCATE(channel_values,float,number_of_signals*
									number_of_channel_devices))
								{
									signal_value=signals_values;
									for (i=0;i<number_of_values;i++)
									{
										for (j=0;j<number_of_channel_devices;j++)
										{
											channel_offset=channel_devices[j]->channel->offset;
											channel_gain=channel_devices[j]->channel->gain;
											signal=channel_devices[j]->signal;
											channel_value=channel_values+j;
											for (k=number_of_signals;k>0;k--)
											{
												switch (buffer->value_type)
												{													
													case SHORT_INT_VALUE:
													{
														data_value_short_int=
															(buffer->signals.short_int_values)+
															((first_data_local+i)*buffer_offset+
															(signal->index));
														*channel_value=
															channel_gain*((float)(*data_value_short_int)-
															channel_offset);
													} break;
													case FLOAT_VALUE:
													{
														data_value_float=(buffer->signals.float_values)+
															((first_data_local+i)*buffer_offset+
															(signal->index));
														*channel_value=
															channel_gain*((float)(*data_value_float)-
															channel_offset);
													} break;
												}
												signal=signal->next;
												channel_value += number_of_channel_devices;
											}
										}
										channel_value=channel_values;
										data_value_float=signal_value;
										for (k=number_of_signals;k>0;k--)
										{
											evaluate_device(device,&channel_value,data_value_float);
											data_value_float += number_of_values;
										}
										signal_value++;
									}
									DEALLOCATE(channel_values);
								}
								else
								{
									display_message(ERROR_MESSAGE,"extract_signal_information.  "
										"Could not allocate channel_values");
									return_code=0;
								}
#else /* defined (DEVICE_EXPRESSIONS) */
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
#endif /* defined (DEVICE_EXPRESSIONS) */
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
#if defined (DEVICE_EXPRESSIONS)
									if (1<number_of_channel_devices)
									{
										for (i=0;i<number_of_signals;i++)
										{
											signals_status[i]=REJECTED;
										}
									}
									else
									{
										signal=(*channel_devices)->signal;
										i=0;
										for (k=1;k<=number_of_signals;k++)
										{
											if ((0==signal_number)||(k==signal_number))
											{
												signals_status[i]=signal->status;
												i++;
											}
											signal=signal->next;
										}
									}
#else /* defined (DEVICE_EXPRESSIONS) */
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
#endif /* defined (DEVICE_EXPRESSIONS) */
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
						signal_minimum=device->signal_display_minimum;
						signal_maximum=device->signal_display_maximum;					
					}
				}
			}
			else
			{			
				display_message(ERROR_MESSAGE,
					"extract_signal_information.  Missing signal(s) for device");
				return_code=0;
 			}
#if defined (DEVICE_EXPRESSIONS)
			DEALLOCATE(channel_devices);
#endif /* defined (DEVICE_EXPRESSIONS) */
#endif /* defined (OLD_CODE) */
			times=(float *)NULL;
			signals_values=(float *)NULL;
			signals_status=(enum Event_signal_status *)NULL;
			number_of_signals=0;
			number_of_values=0;
			name=(char *)NULL;
			highlight=0;
			signal_minimum=0;
			signal_maximum=0;
			return_code=extract_Device_signal_information(device,signal_number,
				first_data,last_data,&times,&signals_values,&signals_status,
				&number_of_signals,&number_of_values,&name,&highlight,&signal_minimum,
				&signal_maximum);
#if defined (UNEMAP_USE_NODES)
		}
		else
		{
			enum Value_type value_type;
			float channel_gain,channel_offset;
			FE_value *FE_value_signal_data=(FE_value *)NULL;
			FE_value 	end_time,start_time,time,time_high,time_low;
			int end_array_index,first_data_local,i,index_high,index_low,
				last_data_local,number_of_nodal_values,start_array_index;
			short int *short_signal_data=(short int *)NULL;
			struct FE_field_component component;						
			struct FE_field *display_start_time_field=(struct FE_field *)NULL;
			struct FE_field *display_end_time_field=(struct FE_field *)NULL;	
			struct FE_field *signal_field=(struct FE_field *)NULL;	
		
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
			component.field=signal_drawing_package->signal_field;
			component.number=0;
			number_of_nodal_values=get_FE_nodal_array_number_of_elements(device_node,
				&component,0,FE_NODAL_VALUE);
			value_type= get_FE_nodal_value_type(device_node,&component,0);
			/*get the fields for time information */
			display_start_time_field=
							get_Signal_drawing_package_display_start_time_field(signal_drawing_package);
			display_end_time_field=
							get_Signal_drawing_package_display_end_time_field(signal_drawing_package);
			signal_field=
							get_Signal_drawing_package_signal_field(signal_drawing_package);					
			if ((number_of_nodal_values>0)&&(value_type!=UNKNOWN_VALUE)&&
				display_start_time_field&&display_end_time_field&&signal_field)
			{
				/*get the display start time, end time*/
				return_code=get_FE_field_FE_value_value(display_start_time_field,0,
					&start_time);
				return_code=get_FE_field_FE_value_value(display_end_time_field,0,
					&end_time);
				/*get the array indices for start time, end time*/
				return_code=get_FE_field_time_array_index_at_FE_value_time(
					signal_field,start_time,&time_high,&time_low,&start_array_index,
					&index_high,&index_low);	
				return_code=get_FE_field_time_array_index_at_FE_value_time(
					signal_field,end_time,&time_high,&time_low,&end_array_index,
					&index_high,&index_low);						
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
					first_data_local=start_array_index;
					last_data_local=end_array_index;
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
							if (get_FE_field_time_FE_value(signal_drawing_package->signal_field,
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
						component.field=signal_drawing_package->channel_offset_field;
						component.number=0;	
						if (get_FE_nodal_FE_value_value(device_node,&component,0,FE_NODAL_VALUE,
							/*time*/0,&channel_offset))
						{
							component.field=signal_drawing_package->channel_gain_field;
							if (get_FE_nodal_FE_value_value(device_node,&component,0,FE_NODAL_VALUE,
								/*time*/0,&channel_gain))
							{
								if (ALLOCATE(signals_values,float,
									number_of_signals*number_of_values))
								{
									component.field=signal_drawing_package->signal_field;
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
										signal_drawing_package->signal_status_field,/*component_number*/0,
										/*version*/0,FE_NODAL_VALUE,&signal_status_str))
									{
										if (!(strcmp("ACCEPTED",signal_status_str)))
										{
											signals_status[i]=ACCEPTED;										
										}
										else if (!(strcmp("REJECTED",signal_status_str)))
										{
											signals_status[i]=REJECTED;											
										}
										else if (!(strcmp("UNDECIDED",signal_status_str)))
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
									signal_drawing_package->device_name_field,/*component_number*/0,
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
					/*get highligh, signal_minimum,signal_maximum from fields*/
					component.number=0;				
					component.field=signal_drawing_package->highlight_field;
					get_FE_nodal_int_value(device_node,&component,0,FE_NODAL_VALUE,
						/*time*/0,&highlight);									
					component.field=signal_drawing_package->signal_minimum_field;
					get_FE_nodal_FE_value_value(device_node,&component,0,FE_NODAL_VALUE,
						/*time*/0,&signal_minimum);
					component.field=signal_drawing_package->signal_maximum_field;
					get_FE_nodal_FE_value_value(device_node,&component,0,FE_NODAL_VALUE,
						/*time*/0,&signal_maximum);					
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
			"extract_signal_information.  Invalid arguments %p %p %p",device,
			device_node,signal_drawing_package);
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* extract_signal_information */

#if defined (UNEMAP_USE_3D)
int file_read_signal_FE_node_group(char *file_name,struct Rig *rig)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Reads  node group(s) from a signal file into rig . Signal file includes the
configuration info.
==============================================================================*/
{
	enum Region_type region_type;	
	FILE *input_file;
	int return_code;
	struct Cmiss_region *root_cmiss_region;
	struct FE_node_order_info *node_order_info;
	struct FE_field *signal_status_field;
	struct FE_region *unrejected_node_group, *rig_node_group;
	struct Region *region;
	struct Region_list_item *region_item;
	struct Unemap_package *unemap_package;

	ENTER(file_read_signal_FE_node_group);	
	unemap_package=(struct Unemap_package *)NULL;
	signal_status_field = (struct FE_field *)NULL;
	unrejected_node_group= (struct FE_region *)NULL;
	rig_node_group= (struct FE_region *)NULL;	
	node_order_info=(struct FE_node_order_info *)NULL;
	region=(struct Region *)NULL;
	region_item=(struct Region_list_item *)NULL;
	if (file_name && (unemap_package = rig->unemap_package) &&
		(root_cmiss_region = get_unemap_package_root_Cmiss_region(unemap_package)))
	{					
		if (input_file=fopen(file_name,"rb"))
		{
			if (1==BINARY_FILE_READ((char *)&region_type,sizeof(enum Region_type),1,
				input_file))
			{
				if ((SOCK==region_type)||(PATCH==region_type)||(MIXED==region_type)||
					(TORSO==region_type))
				{										
					if (return_code=read_config_FE_node_group(input_file,unemap_package,
						region_type,BINARY,&node_order_info,rig))	
					{
						if (return_code=read_signal_FE_node_group(input_file,unemap_package,
							node_order_info))
						{
							if (!read_event_settings_and_signal_status_FE_node_group
								(input_file,unemap_package,node_order_info))
							{	
								display_message(ERROR_MESSAGE,
									"file_read_signal_FE_node_group."
									" read_event_settings_FE_node_group failed");
								return_code=0;
							}							
							/* set up the unrejected_node_groups*/
							region_item=get_Rig_region_list(rig);
							while (region_item)
							{
								region = get_Region_list_item_region(region_item);
								rig_node_group = get_Region_rig_node_group(region);
								if ((signal_status_field =
									get_unemap_package_signal_status_field(unemap_package)) &&
									(unrejected_node_group = make_unrejected_node_group(
										root_cmiss_region, rig_node_group,
										signal_status_field)))
								{									
									set_Region_unrejected_node_group(region,
										unrejected_node_group);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_binary_config_FE_node_group.  "
										"Can't set_Region_unrejected_node_group");
								}
								region_item=get_Region_list_item_next(region_item);							
							}/* while (region_item) */													
						}
						else
						{	
							display_message(ERROR_MESSAGE,
								"file_read_signal_FE_node_group.  "
								"read_signal_FE_node_group failed");
							return_code=0;
						}
						/* no longer needed */
						DEACCESS(FE_node_order_info)(&node_order_info);
					}
					else
					{	
						return_code=0;
						display_message(ERROR_MESSAGE,
					"file_read_signal_FE_node_group.  read_config_FE_node_group failed");
					}
				}
				else
				{	
					return_code=0;
					display_message(ERROR_MESSAGE,
						"file_read_signal_FE_node_group.  Invalid rig type");
				}
			}	
			else
			{
				return_code=0;
				display_message(ERROR_MESSAGE,
					"file_read_signal_FE_node_group.  Error reading file - rig type");
			}
		}
		else
		{
			return_code=0;
			display_message(ERROR_MESSAGE,
				"file_read_signal_FE_node_group.  Could not open %s",file_name);
			fclose(input_file);
		}	
	}
	else
	{	
		return_code=0;
		display_message(ERROR_MESSAGE,
			"file_read_signal_FE_node_group.  Invalid arguments");
	}
	LEAVE;	

	return (return_code);
} /* file_read_signal_FE_node_group */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int file_read_config_FE_node_group(char *file_name,
	struct Unemap_package *unemap_package,struct Rig *rig)
/*******************************************************************************
LAST MODIFIED : 21 July 2000

DESCRIPTION :
Reads  configuration file into  a node group.
cf file_read_FE_node_group() in import_finite_element.c
==============================================================================*/
{	
	char rig_type_text[6];
	enum Region_type region_type;
	enum Rig_file_type file_type;
	FILE *input_file = (FILE *)NULL;
	int return_code;	
	struct FE_node_order_info *node_order_info;

	ENTER(file_read_config_FE_node_group);
	input_file=(FILE *)NULL;
	return_code=0;	
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
				return_code=1;	
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
					return_code=1;		
				}
				else if (!strcmp("torso",rig_type_text))
				{	
					region_type=TORSO;	
					file_type=TEXT;		
					return_code=1;		
				}
				else if (!strcmp("patch",rig_type_text))
				{
					region_type=PATCH;
					file_type=TEXT;	
					return_code=1;					
				}
				else if (!strcmp("mixed",rig_type_text))
				{
					region_type=MIXED;
					file_type=TEXT;		
					return_code=1;				
				}	
				else
				{	
					display_message(ERROR_MESSAGE,
						"file_read_config_FE_node_group. Can't read region type");
					fclose(input_file);
					input_file=(FILE *)NULL;
				}
			} /* if (input_file=fopen(file_name,"r")) */
		} /* if (!input_file) */
		if (input_file&&return_code)
		{		
			return_code=read_config_FE_node_group(input_file,unemap_package,
				region_type,file_type,&node_order_info,rig);
		
			fclose(input_file);		
			/* no longer needed */
			DEACCESS(FE_node_order_info)(&node_order_info);
		}	
		else
		{	
			display_message(ERROR_MESSAGE,
				"file_read_config_FE_node_group.  Invalid file: %s",file_name);
			return_code= 0;
		}		
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"file_read_config_FE_node_group. Invalid arguments");
	}
	LEAVE;

	return (return_code);
} /* file_read_config_FE_node_group */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int get_node_group_position_min_max(struct FE_region *node_group,
	struct FE_field *position_field,FE_value *min_x,FE_value *max_x,
  FE_value *min_y,FE_value *max_y,FE_value *min_z,FE_value *max_z)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Finds the min and max coordinates of the  <position_field>
in the <node_group>. Note: Not necessarily rectangular catresian coords!
==============================================================================*/
{	

	FE_value dest_coordinates[3],source_coordinates[3];
	int return_code;
	struct Coordinate_system *source_coordinate_system,dest_coordinate_system;
	struct Position_min_max_iterator position_min_max_iterator;
	struct FE_node *node;
	struct FE_field_component component;

	ENTER(get_node_group_position_min_max);
	if (node_group&&min_x&&max_x&&min_y&&max_y&&min_z&&max_z
		&&position_field)
	{	
		position_min_max_iterator.count=0;
		position_min_max_iterator.position_field=position_field;	
		component.number=0;
		component.field=position_field;
		if (node=FE_region_get_first_FE_node_that(node_group,
			(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL, NULL))
		{
			source_coordinates[0]=0;
			component.number=0;
			get_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
				/*time*/0,&source_coordinates[0]);
			source_coordinates[1]=0;
			component.number=1;		
			get_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
				/*time*/0,&source_coordinates[1]);
			source_coordinates[2]=0;
			component.number=2;
			get_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
				/*time*/0,&source_coordinates[2]);
			source_coordinate_system=get_FE_field_coordinate_system(position_field);
			dest_coordinate_system.type=RECTANGULAR_CARTESIAN;
			convert_Coordinate_system(source_coordinate_system,3,source_coordinates,
				&dest_coordinate_system,3,dest_coordinates,(float *)NULL);
			position_min_max_iterator.min_x=dest_coordinates[0];
			position_min_max_iterator.max_x=position_min_max_iterator.min_x;
			component.number=1;				
			position_min_max_iterator.min_y=dest_coordinates[1];
			position_min_max_iterator.max_y=position_min_max_iterator.min_y;
			component.number=2;
			position_min_max_iterator.min_z=dest_coordinates[2];
			position_min_max_iterator.max_z=position_min_max_iterator.min_z;

			return_code = FE_region_for_each_FE_node(node_group,
				get_node_position_min_max, (void *)&position_min_max_iterator);	
			*min_x= position_min_max_iterator.min_x;
			*max_x= position_min_max_iterator.max_x;
			*min_y= position_min_max_iterator.min_y;
			*max_y= position_min_max_iterator.max_y;	
			*min_z= position_min_max_iterator.min_z;
			*max_z= position_min_max_iterator.max_z;
		}
		else
		{
			display_message(ERROR_MESSAGE,"get_node_group_position_min_max.  "
				"can't get node");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_node_group_position_min_max."
			" Invalid argument");
		return_code=0;
	}
	if (return_code==0)
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
} /* get_node_group_position_min_max */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_NODES)
int iterative_get_rig_node_accepted_undecided_signal_min_max(struct FE_node *node,
	void *min_max_iterator_void)
/*******************************************************************************
LAST MODIFIED : 7 August 2000

DESCRIPTION :
Sets the the <min_max_iterator>, min, max based upon the node's signal properties.
Only proceses a node if it's signal status filed is accepted/undecided.
This function is called iteratively by anal_set_range_all_accep_undec
==============================================================================*/
{		
	FE_value min,max;
	int return_code;
	struct Min_max_iterator *min_max_iterator;
	enum Event_signal_status status;

	return_code=1;
	ENTER(iterative_get_rig_node_accepted_undecided_signal_min_max);
	if (node&&min_max_iterator_void)
	{
		min_max_iterator=(struct Min_max_iterator *)min_max_iterator_void;	
		if (min_max_iterator&&min_max_iterator->display_start_time_field
			&&min_max_iterator->display_end_time_field		
			&&min_max_iterator->signal_status_field)
		{
			if (FE_field_is_defined_at_node(min_max_iterator->display_start_time_field,node)
				&&FE_field_is_defined_at_node(min_max_iterator->display_end_time_field,node)
				&&FE_field_is_defined_at_node(min_max_iterator->signal_status_field,node))
				/* nothing to do, but NOT an error if no signal at node*/			
			{	
				/* get the minimum,maximum */			
				get_rig_node_signal_min_max(node,
					min_max_iterator->display_start_time_field,
					min_max_iterator->display_end_time_field,
					min_max_iterator->signal_status_field,				
					&min,&max,&status,1/* time_range*/);
				/* do nothing with rejected signals*/
				if ((status==ACCEPTED)||(status==UNDECIDED))
				{
					if (!min_max_iterator->started)
					{
						/*initialise the min_max_iterator->min,min_max_iterator->max*/
						min_max_iterator->min=min;
						min_max_iterator->max=max;
						min_max_iterator->started=1;
					}
					else
					{
						/*check/set min/max*/
						if (max>min_max_iterator->max)
						{
							min_max_iterator->max=max;
						}
						if (min<min_max_iterator->min)
						{
							min_max_iterator->min=min;			
						}
					}		
					min_max_iterator->count++; /* don't really use, but may as well count*/
				}						
			}/* if (FE_field_is_defined_at_node*/
		}	/* if (min_max_iterator */
		else
		{
			display_message(ERROR_MESSAGE,
				"iterative_get_rig_node_accepted_undecided_signal_min_max."
				" min_max_iterator NULL ");
			return_code=0;
		}	
	}/* if ((node)&&(min_max_iterator_void)) */
	else
	{
		display_message(ERROR_MESSAGE,
			"iterative_get_rig_node_accepted_undecided_signal_min_max."
			" Invalid argument");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* iterative_get_rig_node_accepted_undecided_signal_min_max*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int get_rig_node_signal_min_max(struct FE_node *node,
	struct FE_field *display_start_time_field,
	struct FE_field *display_end_time_field,struct FE_field *signal_status_field,	
	FE_value *min,FE_value *max,enum Event_signal_status *status,int time_range)
/*******************************************************************************
LAST MODIFIED : 20 September 2000

DESCRIPTION : Determines and returns <min> and <max the minimum and maximum
value of <signal_field> at <node>. If <time_range> >0 AND <display_start_time_field>,
<display_end_time_field>  are set, then determines the min, max over this range.
If <time_range> =0, determines the min, max over entire signal time  range.
If <signal_status_field> and <status> set, return the node signal's 
Event_signal_status in <status>

Needs rewriting with ew time stuff.
CF
iterative_get_rig_node_signal_min_max_at_time
==============================================================================*/
{	
	int return_code;

	if(node)
	{
		USE_PARAMETER(node);
		USE_PARAMETER(display_start_time_field);
		USE_PARAMETER(display_end_time_field);
		USE_PARAMETER(signal_status_field);
		USE_PARAMETER(min);
		USE_PARAMETER(max);
		USE_PARAMETER(status);
		USE_PARAMETER(time_range);
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_rig_node_signal_min_max."
			" Invalid argument");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* get_rig_node_signal_min_max */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int iterative_unrange_rig_node_signal(struct FE_node *node,	void *min_max_iterator_void)
/*******************************************************************************
LAST MODIFIED : 7 August 2000

DESCRIPTION :
Set the <nodes> signal_minimum,signal_maximum by determining the signals's
actual min and max.
This function is called iteratively by analysis_unrange_all
==============================================================================*/
{		
	FE_value signal_minimum,signal_maximum;
	int return_code;		
	struct FE_field_component component;
	struct Min_max_iterator *min_max_iterator;

	return_code=1;
	ENTER(iterative_unrange_rig_node_signal);
	if (node&&min_max_iterator_void)
	{
		min_max_iterator=(struct Min_max_iterator *)min_max_iterator_void;	
		if (min_max_iterator&&min_max_iterator->signal_minimum_field
			&&min_max_iterator->signal_maximum_field&&min_max_iterator->display_start_time_field
			&&min_max_iterator->display_end_time_field)
		{
			if (FE_field_is_defined_at_node(min_max_iterator->signal_minimum_field,node)
				&&FE_field_is_defined_at_node(min_max_iterator->signal_maximum_field,node)
				&&FE_field_is_defined_at_node(min_max_iterator->display_start_time_field,node)
				&&FE_field_is_defined_at_node(min_max_iterator->display_end_time_field,node))
				/* nothing to do, but NOT an error if no signal at node*/			
			{										
				/* calculate the  new signal_minimum,signal_maximum */
				get_rig_node_signal_min_max(node,
					min_max_iterator->display_start_time_field,
					min_max_iterator->display_end_time_field,(struct FE_field *)NULL,					
					&signal_minimum,&signal_maximum,(enum Event_signal_status *)NULL,					
					1/*int time_range*/);					
				/* set the new signal_minimum,signal_maximum*/
				component.number=0;	
				component.field = min_max_iterator->signal_minimum_field;	
				/*??JW should be copying to/from node with MANAGER_MODIFY, but this func not yet used */
				set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,/*time*/0,signal_minimum);
				component.field = min_max_iterator->signal_maximum_field;
				set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,/*time*/0,signal_maximum);
				min_max_iterator->count++; /* don't really use,but  may as well count*/
			}/* if (FE_field_is_defined_at_node*/
		}	/* if (min_max_iterator */
		else
		{
			display_message(ERROR_MESSAGE,"iterative_unrange_rig_node_signal."
				" min_max_iterator NULL ");
			return_code=0;
		}	
	}/* if ((node)&&(min_max_iterator_void)) */
	else
	{
		display_message(ERROR_MESSAGE,"iterative_unrange_rig_node_signal."
			" Invalid argument");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* iterative_unrange_rig_node_signal*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int iterative_set_rig_node_signal_min_max(struct FE_node *node,	
	void *min_max_iterator_void)
/*******************************************************************************
LAST MODIFIED : 7 August 2000

DESCRIPTION :
Set the <nodes> signal_minimum,signal_maximum from the <min_max_iterator>.
This function is called iteratively by anal_set_range_all_accep_undec
==============================================================================*/
{		
	FE_value signal_minimum,signal_maximum;
	int return_code;		
	struct FE_field_component component;
	struct Min_max_iterator *min_max_iterator;

	return_code=1;
	ENTER(iterative_set_rig_node_signal_min_max);
	if (node&&min_max_iterator_void)
	{
		min_max_iterator=(struct Min_max_iterator *)min_max_iterator_void;	
		if (min_max_iterator&&min_max_iterator->signal_minimum_field
			&&min_max_iterator->signal_maximum_field)
		{
			if (FE_field_is_defined_at_node(min_max_iterator->signal_minimum_field,node)
				&&FE_field_is_defined_at_node(min_max_iterator->signal_maximum_field,node))
				/* nothing to do, but NOT an error if no signal at node*/			
			{
				component.number=0;	
				signal_minimum=min_max_iterator->min;
				signal_maximum=min_max_iterator->max;
				/* set the new signal_minimum,signal_maximum*/
				component.field = min_max_iterator->signal_minimum_field;
				/*??JW should be copying to/from node with MANAGER_MODIFY, but this func not yet used */
				set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,/*time*/0,signal_minimum);
				component.field = min_max_iterator->signal_maximum_field;
				set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,/*time*/0,signal_maximum);
				min_max_iterator->count++; /* don't really use,but  may as well count*/
			}/* if (FE_field_is_defined_at_node*/
		}	/* if (min_max_iterator */
		else
		{
			display_message(ERROR_MESSAGE,"iterative_set_rig_node_signal_min_max."
				" min_max_iterator NULL ");
			return_code=0;
		}	
	}/* if ((node)&&(min_max_iterator_void)) */
	else
	{
		display_message(ERROR_MESSAGE,"set_rig_node_signal_min_max."
			" Invalid argument");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* iterative_set_rig_node_signal_min_max*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_3D)
int get_rig_node_group_signal_min_max_at_time(struct FE_region *node_group,
	struct Computed_field *scaled_offset_signal_value_at_time_field,
	struct FE_field *signal_status_field,
	FE_value time,FE_value *min,FE_value *max)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Returns the <min> and <max>  signal values at the rig nodes in the rig_node_group
<node_group>, field <signal_field>, time <time>
==============================================================================*/
{	
	FE_value fe_value,maximum,minimum;
	int return_code;
	struct Min_max_iterator *min_max_iterator;
	struct FE_node *node;
	
	ENTER(get_rig_node_group_signal_min_max_at_time);
	if (node_group&&min&&max)
	{	
		return_code=1;	
		if (min_max_iterator=CREATE(Min_max_iterator)())
		{	
			set_Min_max_iterator_count(min_max_iterator,0);
			set_Min_max_iterator_time(min_max_iterator,time);	
			node=FE_region_get_first_FE_node_that(node_group,
				(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL, NULL);				
			if (return_code)
			{			
				Computed_field_evaluate_at_node(scaled_offset_signal_value_at_time_field,
						node,time,&fe_value/*assume 1 component*/);
				set_Min_max_iterator_min(min_max_iterator,fe_value);
				set_Min_max_iterator_max(min_max_iterator,fe_value);			
				set_Min_max_iterator_signal_status_field(min_max_iterator,signal_status_field);
				set_Min_max_iterator_scaled_offset_signal_value_at_time_field(min_max_iterator,
					scaled_offset_signal_value_at_time_field);				
				return_code = FE_region_for_each_FE_node(node_group,
					iterative_get_rig_node_signal_min_max_at_time,
					(void *)min_max_iterator);					
				get_Min_max_iterator_min(min_max_iterator,&minimum);
				*min= minimum;
				get_Min_max_iterator_max(min_max_iterator,&maximum);
				*max= maximum;				
				Computed_field_clear_cache(scaled_offset_signal_value_at_time_field);
			}
			else
			{
				*min= 0;
				*max= 0;
			}
			DESTROY(Min_max_iterator)(&min_max_iterator);	
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_rig_node_group_signal_min_max_at_time.  "
				"CREATE(Min_max_iterator failed)");
			return_code=0;
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
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int rig_node_group_set_map_electrode_position_lambda_r(
	struct Unemap_package *package,struct FE_region *rig_node_group,
	struct Region *region,FE_value sock_lambda,FE_value torso_major_r,
	FE_value torso_minor_r)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Sets the node group's nodal map_electrode_postions from the nodal electrode_positions, 
and changes the <rig_node_group>'s map_electrode_postions lambda or r values to 
<value>
==============================================================================*/
{	
	enum Region_type region_type;
	FE_value value1,value2;
	int return_code;	
	struct FE_field *map_electrode_position_field;
	struct Set_map_electrode_position_info set_map_electrode_position_info;

	ENTER(rig_node_group_set_map_electrode_position_lambda_r);
	map_electrode_position_field=(struct FE_field *)NULL;
	return_code=1;
	if (package&&rig_node_group)
	{
		region_type=region->type;
		switch (region_type)
		{
			case SOCK:
			{
				value1=sock_lambda;
				value2=0;/* not used*/
			} break;																												
			case TORSO:	
			{
				value1=torso_major_r;
				value2=torso_minor_r;
			} break;
			case PATCH:
			{									
				value1=0;/* not used*/
				value2=0;/* not used*/
			} break;	
			case MIXED:	
			{
				display_message(ERROR_MESSAGE,
					"rig_node_group_set_map_electrode_position_lambda_r.  "
					"MIXED region type");
				return_code=0;
			} break;
			case UNKNOWN:	
			default:	
			{
				display_message(ERROR_MESSAGE,
					"rig_node_group_set_map_electrode_position_lambda_r.  "
					"UNKNOWN region type");
				return_code=0;	
			} break;
		}/* switch (current_region->type) */	
		if (return_code)
		{
			map_electrode_position_field=
			  get_Region_map_electrode_position_field(region);
			set_map_electrode_position_info.map_electrode_position_field=
				map_electrode_position_field;	
			set_map_electrode_position_info.fe_region = rig_node_group;
			set_map_electrode_position_info.value1=value1;
			set_map_electrode_position_info.value2=value2;
			set_map_electrode_position_info.region_type=region_type;
			FE_region_begin_change(rig_node_group);
			return_code=FE_region_for_each_FE_node(rig_node_group,
				iterative_rig_node_set_map_electrode_position_lambda_r,
				(void *)&set_map_electrode_position_info);
			FE_region_end_change(rig_node_group);
		}	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"rig_node_group_set_map_electrode_position_lambda_r.  "
			"Invalid argument");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* rig_node_group_set_map_electrode_position_lambda_r */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int rig_node_group_add_map_electrode_position_field(
	struct Unemap_package *package,struct FE_region *rig_node_group,
	struct FE_field *map_electrode_position_field)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Add an electrode_position_field  to the <rig_node_group> nodes,
in addition to the one created with create_config_template_node.
==============================================================================*/
{	
	int return_code;
	struct Add_map_electrode_position_info add_map_electrode_position_info;	
	
	ENTER(rig_node_group_add_map_electrode_position_field);
	if (package && rig_node_group && map_electrode_position_field)
	{
		add_map_electrode_position_info.map_electrode_position_field=
			map_electrode_position_field;
		add_map_electrode_position_info.fe_region = rig_node_group;		
		FE_region_begin_change(rig_node_group);
		return_code=FE_region_for_each_FE_node(rig_node_group,
			rig_node_add_map_electrode_position_field,
			(void *)&add_map_electrode_position_info);
		FE_region_end_change(rig_node_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"rig_node_group_add_map_electrode_position_field.  "
			"Invalid argument");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* rig_node_group_add_map_electrode_position_field*/
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_NODES)
int sort_rig_node_sorts_by_read_order(void *first,void *second)
/*******************************************************************************
LAST MODIFIED : 25 July 2000

DESCRIPTION :
Returns whether the <first> rig_node_sort has a smaller (< 0), the same (0) or a
larger (> 0) read_order than the <second> device.
==============================================================================*/
{
	int first_number,return_code,second_number;

	ENTER(sort_rig_node_sorts_by_read_order);
	first_number=(*((struct Rig_node_sort **)first))->read_order;
	second_number=(*((struct Rig_node_sort **)second))->read_order;
	if (first_number<second_number)
	{
		return_code=-1;
	}
	else
	{
		if (first_number>second_number)
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	LEAVE;

	return (return_code);
} /*sort_rig_node_sorts_by_read_order(  */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int sort_rig_node_sorts_by_event_time(void *first,void *second)
/*******************************************************************************
LAST MODIFIED : 26 July 2000

DESCRIPTION :
Returns whether the <first> rig_node_sort has a smaller (< 0), the same (0) or a
larger (> 0) event_time than the <second> device.
==============================================================================*/
{
	FE_value first_number,second_number;
	int return_code;

	ENTER(sort_rig_node_sorts_by_event_time);
	first_number=(*((struct Rig_node_sort **)first))->event_time;
	second_number=(*((struct Rig_node_sort **)second))->event_time;
	if (first_number<second_number)
	{
		return_code=-1;
	}
	else
	{
		if (first_number>second_number)
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	LEAVE;

	return (return_code);
} /*sort_rig_node_sorts_by_event_time(  */
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_3D)
struct FE_node *find_rig_node_given_device(struct Device *device,
	struct FE_region *rig_node_group,struct FE_field *device_name_field)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Given a <device>, finds the corresponding FE_node in <rig_node_group>.
Does by matching the names. Therefore assume's device/node names are unique.
If they're not, you'll get the first match.
==============================================================================*/
{
	char *name;
	struct Device_description *description;
	struct FE_node *rig_node;
	struct FE_field_and_string_data field_and_string_data;
	
	ENTER(find_rig_node_given_device);
	rig_node=(struct FE_node *)NULL;
	name=(char *)NULL;
	description=(struct Device_description *)NULL;
	if (device_name_field&&rig_node_group&&device&&(description=
		get_Device_description(device))&&(name=get_Device_description_name(description)))
	{
		field_and_string_data.fe_field=device_name_field;
		field_and_string_data.string=name;
		rig_node = FE_region_get_first_FE_node_that(rig_node_group,
			FE_node_has_FE_field_and_string_data, (void *)(&field_and_string_data));
	}
	else
	{
		display_message(ERROR_MESSAGE,"find_rig_node_given_device. Invalid argument");
	}
 LEAVE;
 return(rig_node);
}/* find_rig_node_given_device */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
struct Device **find_device_given_rig_node(struct FE_node *node,
	struct FE_field *device_name_field,struct Rig *rig)
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Given a <node>, finds the corresponding Device in <rig>.
returns a (struct Device **) rather than a (struct Device *) as pointer
arithmetic is done on it later.
Currently matches the names.
Does by matching the names. Therefore assume's device/node names are unique.
If they're not, you'll get the first match.
==============================================================================*/
{
	char *device_name,*nodal_name;
	int i,number_of_devices,success;
	struct Device *device,**devices;	
	struct Device_description *description;
 
	ENTER(find_device_given_rig_node);
	device_name=(char *)NULL;
	nodal_name=(char *)NULL;
	device=(struct Device *)NULL;
	devices=(struct Device **)NULL;
	description=(struct Device_description *)NULL;
	success=0;
	if (device_name_field&&rig&&rig->devices)
	{	
		if (FE_field_is_defined_at_node(device_name_field,node))
		{
			success=get_FE_nodal_string_value(node,device_name_field,0,0,FE_NODAL_VALUE,&nodal_name);
			if (!success)
			{
				display_message(WARNING_MESSAGE,"find_rig_node_given_device. " 
					"field not defined at node");							
			}
		}
		if (success)
		{	
			device=*(rig->devices);
			devices=rig->devices;
			number_of_devices=rig->number_of_devices;
			success=0;
			i=0;
			while((!success)&&(i<number_of_devices))
			{		
				device=*devices;			
				description=get_Device_description(device);
				device_name=get_Device_description_name(description);
				if (!strcmp(device_name,nodal_name))
				{
					success=1;
				}
				else
				{
					devices++;
					i++;
				} /* if (!strcmp(required_stri */
			}/* while(!success)&&(i<number_of_devices)*/ 
			if (!success)
			{
				device=(struct Device *)NULL;
			}
		}/* if (success) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"find_rig_node_given_devic. Invalid argument");
	}
 LEAVE;
 return(devices);
}/* find_device_given_rig_node */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int node_signal_is_unrejected(struct FE_node *node,void *signal_status_field_void)
/*******************************************************************************
LAST MODIFIED : 5 December 2000 

DESCRIPTION : 
An iterator function.
Returns 1 if the <signal_status_field> at the <node> does NOT return the string
"REJECTED".
==============================================================================*/
{
	char *signal_status_string;		
	int return_code;
	struct FE_field *signal_status_field;

	ENTER(node_signal_is_unrejected);
	return_code=0;
	if (node&&(signal_status_field=(struct FE_field *)signal_status_field_void))
	{
		if (get_FE_nodal_string_value(node,signal_status_field,
			/*component_number*/0,/*version*/0,FE_NODAL_VALUE,&signal_status_string))
		{
			if (!strcmp(signal_status_string,"REJECTED")) /* strcmp rets 0 for match*/
			{
				return_code=0;
			}
			else
			{
				return_code=1;
			}
			DEALLOCATE(signal_status_string);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"node_signal_is_unrejected. failed to get signal status");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_signal_is_unrejected.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* node_signal_is_unrejected  */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static struct FE_node *create_config_FE_node_from_device(struct Device *device,
	struct FE_node *template_node,struct FE_region *fe_region,
	enum Config_node_type	config_node_type,int node_number,int read_order_number,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Creates a config FE_node from <device>, <template_node> and other passed 
information.
cf read_binary_config_FE_node
==============================================================================*/
{
	char *name;
	int channel_number,number_of_electrodes,success;
	FE_value xpos,ypos,zpos;
	struct FE_node *node;
	struct Device_description *description;

	ENTER(create_config_FE_node_from_device);
	node=(struct FE_node *)NULL;
	description=(struct Device_description *)NULL;
	success=0;
	if (device&&(description=device->description)&&template_node&&
		fe_region &&field_order_info)
	{
		/* read in name */
		name=description->name;
		/* read channel number */
		if(device->channel)
		{
			channel_number=device->channel->number;
		}
		else
		{ 
			/* linear combiation auxiliaries */
			channel_number=-1;
		}
		number_of_electrodes=0;
		switch (config_node_type)
		{
			case SOCK_ELECTRODE_TYPE:
			case TORSO_ELECTRODE_TYPE:
			{
				xpos=(description->properties).electrode.position.x;
				ypos=(description->properties).electrode.position.y;
				zpos=(description->properties).electrode.position.z;
				success=1;
			} break;
			case PATCH_ELECTRODE_TYPE:
			{
				xpos=(description->properties).electrode.position.x;
				ypos=(description->properties).electrode.position.y;	
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
					success=1;
					/*??JW Don't handle the linear combiation auxiliaries as nodes yet*/
					USE_PARAMETER(number_of_electrodes);
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
			if (!(node=set_config_FE_node(template_node, fe_region,
				config_node_type,node_number,read_order_number,
#if  defined (UNEMAP_USE_NODES)
				0/*highlight*/,
#endif /*  defined (UNEMAP_USE_NODES) */
				field_order_info,name,channel_number,xpos,ypos,zpos)))
			{
				display_message(ERROR_MESSAGE,
					"create_config_FE_node_from_device.  Error creating node");
				node=(struct FE_node *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_config_FE_node_from_device.  Error reading device type specific information from file ");
			node=(struct FE_node *)NULL;
		}	 
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_config_FE_node_from_device.  Invalid argument(s)");
		node=(struct FE_node *)NULL;
	}
	LEAVE;

	return (node);
} /* create_config_FE_node_from_device */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int convert_config_rig_to_nodes(struct Rig *rig,
	struct FE_node_order_info **the_all_devices_node_order_info)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION : 
Convert a the configuration file information of a rig into nodes/elements/fields.
Creates and returns <the_all_devices_node_order_info>, with the created nodes.
Creates nodes/elements/fields.
cf read_binary_config_FE_node_group.
==============================================================================*/
{
	char *region_name,region_num_string[10];
	enum Config_node_type config_node_type;	
	enum Region_type region_type;
	FE_value focus;
	int count,device_count,last_node_number,node_number,number_of_regions,
		read_order_number,region_number,region_number_of_devices,return_code,
		rig_number_of_devices,string_error,string_length;
	struct Cmiss_region *root_cmiss_region;
	struct Device **device;	
	struct Device_description *description;
	struct FE_field *electrode_position_field;	
	struct FE_field_order_info *field_order_info;	
	struct FE_node *node,*template_node;
	struct FE_node_order_info *all_devices_node_order_info;		
	struct FE_region *all_devices_node_group, *node_group, *root_fe_region;
	struct Region *region;
	struct Region_list_item *region_item;
	struct Unemap_package *package;

	ENTER(convert_config_rig_to_nodes);
	description=(struct Device_description *)NULL;
	device=(struct Device **)NULL;
	electrode_position_field = (struct FE_field *)NULL;
	field_order_info = (struct FE_field_order_info *)NULL;
	template_node = (struct FE_node *)NULL;
	node= (struct FE_node *)NULL;
	region_name=(char *)NULL;
	region=(struct Region *)NULL;
	region_item=(struct Region_list_item *)NULL;
	package=(struct Unemap_package *)NULL;
	all_devices_node_group=(struct FE_region *)NULL;
	all_devices_node_order_info =(struct FE_node_order_info *)NULL;	
	node_group=(struct FE_region *)NULL;
	if (rig && (device=rig->devices)&&(package=rig->unemap_package)&&
		(root_cmiss_region = get_unemap_package_root_Cmiss_region(package)) &&
		(root_fe_region = Cmiss_region_get_FE_region(root_cmiss_region)))
	{	
		return_code=1;
		device_count =0;
		rig_number_of_devices=rig->number_of_devices;
		number_of_regions=rig->number_of_regions;
		FE_region_begin_change(root_fe_region);
		/*make all_devices_node_group */
		if (all_devices_node_group = Cmiss_region_get_or_create_child_FE_region(
			root_cmiss_region, "all_devices", /*master_fe_region*/root_fe_region,
			(struct MANAGER(FE_basis) *)NULL, (struct LIST(FE_element_shape) *)NULL))
		{
			FE_region_begin_change(all_devices_node_group);
			FE_region_clear(all_devices_node_group, /*destroy_in_master*/1);
			set_Rig_all_devices_rig_node_group(rig,all_devices_node_group);
			last_node_number = 1;	
			region_number=0;
			/* for all the regions..*/
			if (region_item=get_Rig_region_list(rig))
			{
				while((region_number<number_of_regions)&&return_code)
				{
					if (!(region=get_Region_list_item_region(region_item)))
					{
						display_message(ERROR_MESSAGE,
							"convert_config_rig_to_nodes region_item->region is NULL ");
						return_code=0;
					}
					region_type=region->type;
					/*get the focus*/
					if(region_type==SOCK)
					{
						focus=(region->properties).sock.focus;
					}
					else
					{
						focus=0.0;
					}
					/*get the name */
					string_length=strlen(region->name);
					string_length++;
					if (ALLOCATE(region_name,char,string_length+1))
					{
						strcpy(region_name,region->name);
						region_name[string_length]='\0';
						/*append the region number to the name, to ensure it's unique*/
						sprintf(region_num_string,"%d",region_number);
						append_string(&region_name,region_num_string,&string_error);
						if (node_group) /* stop caching the current group,(if any) */
						{
							FE_region_end_change(node_group);
						}								
						/* Get group */
						node_group = Cmiss_region_get_or_create_child_FE_region(
							root_cmiss_region, region_name, /*master_fe_region*/root_fe_region,
							(struct MANAGER(FE_basis) *)NULL, (struct LIST(FE_element_shape) *)NULL);
						FE_region_begin_change(node_group);
						FE_region_clear(node_group, /*destroy_in_master*/1);
						set_Region_rig_node_group(region,node_group);
						/*destroy any exisiting template,field_order_info*/
						/* create the template node,(but may change it to auxiliary later*/	
						DESTROY(FE_node)(&template_node);
						DESTROY(FE_field_order_info)(&field_order_info);
						switch (region_type)
						{							
							case SOCK:
							{
								config_node_type = SOCK_ELECTRODE_TYPE;
								template_node = create_config_template_node(
									config_node_type,focus,&field_order_info,package,
									&electrode_position_field);
								set_Region_electrode_position_field(region,
									electrode_position_field);
							} break;
							case TORSO:
							{
								config_node_type = TORSO_ELECTRODE_TYPE;
								template_node = create_config_template_node(
									config_node_type,0,&field_order_info,package,
									&electrode_position_field);
								set_Region_electrode_position_field(region,electrode_position_field);
							} break;
							case PATCH:
							{
								config_node_type = PATCH_ELECTRODE_TYPE;
								template_node = create_config_template_node(
									config_node_type,0,&field_order_info,package,
									&electrode_position_field);
								set_Region_electrode_position_field(region,electrode_position_field);
							} break;
							case MIXED:
							default :
							{
								display_message(ERROR_MESSAGE,"convert_config_rig_to_nodes."
									"invalid region type ");
								return_code=0;
							} break;
						}	/* switch (config_node_type) */
						region_number_of_devices=region->number_of_devices;
						/* create or reallocate the all_devices_node_order_info */
						if (!all_devices_node_order_info)
						{
							all_devices_node_order_info =CREATE(FE_node_order_info)
								(region_number_of_devices);
							*the_all_devices_node_order_info = all_devices_node_order_info;						
							if (!all_devices_node_order_info)
							{
								display_message(ERROR_MESSAGE,"convert_config_rig_to_nodes."
									"CREATE(FE_node_order_info) failed ");
								return_code=0;						
								node_group = (struct FE_region *)NULL;					
							}
						}
						else
						{ 
							if (!(return_code =add_nodes_FE_node_order_info(
								region_number_of_devices,all_devices_node_order_info)))
							{			
								return_code=0;					
								node_group = (struct FE_region *)NULL;	
							}
						}/* if (!all_devices_node_order_info) */
						/* read in the inputs */
						read_order_number=0;					
						count=rig_number_of_devices;
						/* for all the devices in the rig*/
						device=rig->devices;
						while((count>0)&&return_code)
						{	
							description=(*device)->description;
							/*if the device is in the current region */
							if(description->region==region)
							{
								node_number = FE_region_get_next_FE_node_identifier(
									root_fe_region, last_node_number);
								/* create a node from the device */
								switch (description->type)
								{
									case ELECTRODE:
									{
										node=create_config_FE_node_from_device(*device,template_node,
											root_fe_region,config_node_type,node_number,read_order_number,
											field_order_info);
										last_node_number = node_number;	
									} break;
									case AUXILIARY:
									{	
										config_node_type = AUXILIARY_TYPE;									
										DESTROY(FE_node)(&template_node);
										DESTROY(FE_field_order_info)(&field_order_info);	
										template_node = create_config_template_node(config_node_type,
											0,&field_order_info,package,&electrode_position_field);
										node=create_config_FE_node_from_device(*device,template_node,
											root_fe_region,config_node_type,node_number,read_order_number,
											field_order_info);
										last_node_number = node_number;										
									} break;
								}	/* switch (device_type) */
								if (node)
								{ 
									if (FE_region_merge_FE_node(node_group, node))
									{
										if (FE_region_merge_FE_node(all_devices_node_group, node))
										{
											/* add the node to the node_order_info*/
											set_FE_node_order_info_node(all_devices_node_order_info,
												device_count,node);
											device_count++;
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"convert_config_rig_to_nodes.  "
												"Could not add node to all_devices_node_group");
											/* remove node from its ultimate master FE_region */
											FE_region_remove_FE_node(root_fe_region, node);
											node = (struct FE_node *)NULL;
											return_code = 0;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"convert_config_rig_to_nodes.  "
											"Could not add node to node_group");
										node = (struct FE_node *)NULL;
										return_code = 0;
									}
								} /*	if (node) */								
								read_order_number++;
							}
							device++;
							count--;						
						} /*	while (count>0)&&return_code) */		
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"convert_config_rig_to_nodes. failed to allocate region_name");
						return_code =0;	
					}		
					if (return_code)
					{
						/* move to the next region */
						region_number++;				
					}
					if (region_name)
					{
						DEALLOCATE(region_name);
					}			
					region_item=get_Region_list_item_next(region_item);							
				}/*	while((region_number<number_of_regions)&&return_code) */	
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"convert_config_rig_to_nodes. rig->region_list is NULL");
				return_code =0;		
			}	
			/* We've finished. Clean up */	
			if (template_node)
			{
				DESTROY(FE_node)(&template_node);
			}	
			if (node_group)
			{
				FE_region_end_change(node_group);
			}	
			FE_region_end_change(all_devices_node_group);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"convert_config_rig_to_nodes.  Could not create node_group");
			return_code = 0;
		}
		FE_region_end_change(root_fe_region);
		if (field_order_info)
		{
			DESTROY(FE_field_order_info)(&field_order_info);
		}		
		if (region_name)
		{
			DEALLOCATE(region_name);
		}			
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"convert_config_rig_to_nodes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* convert_config_rig_to_nodes*/
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int convert_signal_rig_to_nodes(struct Rig *rig,
	struct FE_node_order_info *all_devices_node_order_info)
/*******************************************************************************
LAST MODIFIED :18 October 2001

DESCRIPTION : 
Converts the signals of a rig into nodes/fields.
Assumes that convert_config_rig_to_nodes has been called first.
==============================================================================*/
{
	char
	*channel_gain_component_names[1]=
	{
		"gain_value"
	},	
	*channel_offset_component_names[1]=
	{
		"offset_value"
	},
	*signal_maximum_component_names[1]=
	{
		"maximum_value"
	},
	*signal_minimum_component_names[1]=
	{
		"minimum_value"
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
	FE_value channel_gain,channel_offset,*node_signals_fe_value,period,*times;
	float *buffer_signals_float,frequency;
	int *buffer_times,i,j,number_of_nodes,number_of_samples,
		number_of_signals,return_code;
	short int *buffer_signals_short,*node_signals_short;
	struct Cmiss_region *root_cmiss_region;
	struct Coordinate_system coordinate_system;	
	struct Device **device;	
	struct FE_field *channel_gain_field,*channel_offset_field,*signal_field,
		*signal_maximum_field,*signal_minimum_field,*signal_status_field;
	struct FE_field_component component;
	struct FE_node *node,*node_managed;
	struct FE_node_field_creator *node_field_creator;
	struct FE_region *root_fe_region;
	struct FE_time_version *fe_time_version;
	struct Signal_buffer *buffer;
	struct Unemap_package *package;

	ENTER(convert_signal_rig_to_nodes);
	device_type_string=(char *)NULL;
	node=(struct FE_node *)NULL;
	node_managed=(struct FE_node *)NULL;
	buffer_signals_float=(float *)NULL;
	signal_field=(struct FE_field *)NULL;
	channel_gain_field=(struct FE_field *)NULL;
	channel_offset_field=(struct FE_field *)NULL;
	signal_minimum_field=(struct FE_field *)NULL;
	signal_maximum_field=(struct FE_field *)NULL;
	buffer_signals_short=(short int *)NULL;
	node_signals_short=(short int *)NULL;	
	buffer_times=(int *)NULL;
	times=(FE_value *)NULL;
	node_signals_fe_value=(FE_value *)NULL;
	device=(struct Device **)NULL;
	fe_time_version=(struct FE_time_version *)NULL;
	package=(struct Unemap_package *)NULL;
	/*assumes all devices share the same signal buffer*/
	/*is this necessarily true for multiple regions? */
	/*read_signal_file makes this assumption.*/
	if (rig&&all_devices_node_order_info&&(device=rig->devices)&&
		((*device)->signal)&&(buffer=(*device)->signal->buffer)&&
		(package=rig->unemap_package) &&
		(root_cmiss_region = get_unemap_package_root_Cmiss_region(package)) &&
		(root_fe_region = Cmiss_region_get_FE_region(root_cmiss_region)))
	{
		return_code=1;					
		coordinate_system.type=NOT_APPLICABLE;	
		number_of_signals=buffer->number_of_signals;
		number_of_samples=buffer->number_of_samples;
		frequency=buffer->frequency;
		signal_value_type=buffer->value_type;
		buffer_times=buffer->times;																
		/* Do node stuff here */
		if (return_code)
		{	
			period = 1/frequency;									
			/* allocate memory for times, and fill in */
			if (ALLOCATE(times,FE_value,number_of_samples))
			{								
				for(j=0;j<number_of_samples;j++)
				{
					times[j] = buffer_times[j]*period;										
				}											
				if (!(fe_time_version = FE_region_get_FE_time_version_matching_series(
					root_fe_region, number_of_samples, times)))
				{
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"convert_signal_rig_to_nodes. No memory for times ");	
				return_code =0;
			}
			/* allocate memory for signals to be stored at node, 
				 create signal field */
			switch (signal_value_type)
			{
				case SHORT_INT_VALUE:
				{
					buffer_signals_short=buffer->signals.short_int_values;
					if (ALLOCATE(node_signals_short,short int,number_of_samples))
					{
						if (!(signal_field=FE_region_get_FE_field_with_properties(
							root_fe_region,"signal",GENERAL_FE_FIELD,
							/*indexer_field*/(struct FE_field *)NULL,
							/*number_of_indexed_values*/0,CM_GENERAL_FIELD,
							&coordinate_system,SHORT_VALUE,
							/*number_of_components*/1,signal_component_names,
							/*number_of_times*/0,/*time_value_type*/
							UNKNOWN_VALUE,
							(struct FE_field_external_information *)NULL)))
						{
							display_message(ERROR_MESSAGE,"convert_signal_rig_to_nodes."
								"error getting signal field");	
							return_code =0;
						}													
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"convert_signal_rig_to_nodes. No memory for node_signals_short ");
						return_code =0;
					}
				} break;
				case FLOAT_VALUE:
				{
					buffer_signals_float=buffer->signals.float_values;
					if (ALLOCATE(node_signals_fe_value,float,number_of_samples))
					{
						if (!(signal_field=FE_region_get_FE_field_with_properties(
							root_fe_region,"signal",GENERAL_FE_FIELD,
							/*indexer_field*/(struct FE_field *)NULL,
							/*number_of_indexed_values*/0,CM_GENERAL_FIELD,
							&coordinate_system,FE_VALUE_VALUE,
							/*number_of_components*/1,signal_component_names,
							/*number_of_samples*/0,/*time_value_type*/UNKNOWN_VALUE,
							(struct FE_field_external_information *)NULL)))
						{
							display_message(ERROR_MESSAGE,"convert_signal_rig_to_nodes."
								"error getting signal field");	
							return_code =0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"convert_signal_rig_to_nodes. No memory for "
							"node_signals_fe_value");
						return_code =0;
					}
				} break;
			}	/* switch (signal_value_type) */									
		}							
		if (return_code)
		{		
			/* As the each field has one component with one version and no
				derivatives we can use the same node_field_creator for all fields */
			if (node_field_creator = CREATE(FE_node_field_creator)(
					 /*number_of_components*/1))
			{
				number_of_nodes=get_FE_node_order_info_number_of_nodes(
					all_devices_node_order_info);
				device=rig->devices;
				FE_region_begin_change(root_fe_region);
				for(i=0;i<number_of_nodes;i++)
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
					node_managed =get_FE_node_order_info_node(all_devices_node_order_info,i);
					/* create a copy of the node to work with */
					if (node = CREATE(FE_node)(get_FE_node_identifier(node_managed),
						(struct FE_region *)NULL, node_managed))
					{
						/* add channel_gain and channel_offset fields to node*/
						if (channel_gain_field=FE_region_get_FE_field_with_properties(
							root_fe_region,"channel_gain",GENERAL_FE_FIELD,
							/*indexer_field*/(struct FE_field *)NULL,
							/*number_of_indexed_values*/0,CM_GENERAL_FIELD,
							&coordinate_system,FE_VALUE_VALUE,
							/*number_of_components*/1,channel_gain_component_names,
							/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
							(struct FE_field_external_information *)NULL))
						{
							if (define_FE_field_at_node(node,channel_gain_field,
								(struct FE_time_version *)NULL, node_field_creator))
							{	
								/* add it to the unemap package */
								set_unemap_package_channel_gain_field(package,
									channel_gain_field);
								/* set the values*/
								component.number = 0;
								component.field = channel_gain_field; 
								if((*device)->channel)
								{
									channel_gain=(*device)->channel->gain;
								}
								else
								{
									/*a linear combination auxiliary. Not handled properly yet*/
									channel_gain=1;
								}
								set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
									/*time*/0,channel_gain);
							}
							else
							{
								display_message(ERROR_MESSAGE,"convert_signal_rig_to_nodes."
									"error defining channel_gain_field");	
								return_code =0;
							}	
						}
						else
						{
							display_message(ERROR_MESSAGE,"convert_signal_rig_to_nodes."
								"error getting  channel_gain_field");	
							return_code =0;
						}	
						if (channel_offset_field=FE_region_get_FE_field_with_properties(
							root_fe_region,"channel_offset",GENERAL_FE_FIELD,
							/*indexer_field*/(struct FE_field *)NULL,
							/*number_of_indexed_values*/0,CM_GENERAL_FIELD,
							&coordinate_system,FE_VALUE_VALUE,
							/*number_of_components*/1,channel_offset_component_names,
							/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
							(struct FE_field_external_information *)NULL))
						{ 
							if (define_FE_field_at_node(node,channel_offset_field,
								(struct FE_time_version *)NULL, node_field_creator))
							{	
								/* add it to the unemap package */
								set_unemap_package_channel_offset_field(package,
									channel_offset_field);
								/* set the values*/
								component.number = 0;
								component.field = channel_offset_field; 
								if((*device)->channel)
								{
									channel_offset=(*device)->channel->offset;
								}
								else
								{
									/*a linear combination auxiliary. Not handled properly yet*/
									channel_offset=0;
								}
								set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
									/*time*/0,channel_offset);
							}
							else
							{
								display_message(ERROR_MESSAGE,"convert_signal_rig_to_nodes."
									"error defining channel_offset_field");	
								return_code =0;
							}											
						}
						else
						{
							display_message(ERROR_MESSAGE,"convert_signal_rig_to_nodes."
								"error getting channel_offset_field");	
							return_code =0;
						}	
						/* add signal_minimum field to node. Set to default (1.0)*/
						/* Value set up in draw_signal()*/
						if (signal_minimum_field=FE_region_get_FE_field_with_properties(
							root_fe_region,"signal_minimum",GENERAL_FE_FIELD,
							/*indexer_field*/(struct FE_field *)NULL,
							/*number_of_indexed_values*/0,CM_GENERAL_FIELD,
							&coordinate_system,FE_VALUE_VALUE,
							/*number_of_components*/1,signal_minimum_component_names,
							/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
							(struct FE_field_external_information *)NULL))
						{
							if (define_FE_field_at_node(node,signal_minimum_field,
								(struct FE_time_version *)NULL, node_field_creator))
							{	
								/* add it to the unemap package */
								set_unemap_package_signal_minimum_field(package,
									signal_minimum_field);
								/*set the signal_minimum_field and signal_maximum_field fields */
								component.number = 0;
								component.field = signal_minimum_field;												
								set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
									/*time*/0,(*device)->signal_display_minimum);
							}
							else
							{
								display_message(ERROR_MESSAGE,"convert_signal_rig_to_nodes."
									"error defining signal_minimum_field");	
								return_code =0;
							}	
						}
						else
						{
							display_message(ERROR_MESSAGE,"convert_signal_rig_to_nodes."
								"error getting  signal_minimum_field");	
							return_code =0;
						}	
						/* add signal_maximum field to node.Set to default (0.0)*/
						/* Value set up in draw_signal()*/
						if (signal_maximum_field=FE_region_get_FE_field_with_properties(
							root_fe_region,"signal_maximum",GENERAL_FE_FIELD,
							/*indexer_field*/(struct FE_field *)NULL,
							/*number_of_indexed_values*/0,CM_GENERAL_FIELD,
							&coordinate_system,FE_VALUE_VALUE,
							/*number_of_components*/1,signal_maximum_component_names,
							/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
							(struct FE_field_external_information *)NULL))
						{
							if (define_FE_field_at_node(node,signal_maximum_field,
								(struct FE_time_version *)NULL, node_field_creator))
							{	
								/* add it to the unemap package */
								set_unemap_package_signal_maximum_field(package,
									signal_maximum_field);
								component.number = 0;
								component.field = signal_maximum_field; 
								set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
									/*time*/0,(*device)->signal_display_maximum);
							}
							else
							{
								display_message(ERROR_MESSAGE,"convert_signal_rig_to_nodes."
									"error defining signal_maximum_field");	
								return_code =0;
							}	
						}
						else
						{
							display_message(ERROR_MESSAGE,"convert_signal_rig_to_nodes."
								"error getting  signal_maximum_field");	
							return_code =0;
						}	
						/* add signal_status field to node. The contents of this may be */
						/*overwritten if loaded in in */
						/*read_event_settings_and_signal_status_FE_node_group*/	
						if (signal_status_field=FE_region_get_FE_field_with_properties(
							root_fe_region,"signal_status",GENERAL_FE_FIELD,
							/*indexer_field*/(struct FE_field *)NULL,
							/*number_of_indexed_values*/0,CM_GENERAL_FIELD,
							&coordinate_system,STRING_VALUE,
							/*number_of_components*/1,signal_status_component_names,
							/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
							(struct FE_field_external_information *)NULL))
						{ 
							if (define_FE_field_at_node(node,signal_status_field,
								(struct FE_time_version *)NULL, node_field_creator))
							{	
								/* add it to the unemap package */													
								set_unemap_package_signal_status_field(package,
									signal_status_field);
								/* need to set status based upon the nodal device_type */ 							
								get_FE_nodal_string_value(node,
									get_unemap_package_device_type_field(package),
									0,0,FE_NODAL_VALUE,&device_type_string);
								if (strcmp(device_type_string,"ELECTRODE"))
								{
									set_FE_nodal_string_value(node,signal_status_field,0,0,
										FE_NODAL_VALUE,
										"REJECTED");
								}
								else
								{
									switch((*device)->signal->status)
									{
										case ACCEPTED: 
										{
											set_FE_nodal_string_value(node,signal_status_field,0,0,
												FE_NODAL_VALUE,
												"UNDECIDED");
										}break;
										case REJECTED: 
										{
											set_FE_nodal_string_value(node,signal_status_field,0,0,
												FE_NODAL_VALUE,
												"REJECTED");
										}break;
										default:
										case UNDECIDED: 
										{
											set_FE_nodal_string_value(node,signal_status_field,0,0,
												FE_NODAL_VALUE,
												"UNDECIDED");
										}break;
									}
								}
								DEALLOCATE(device_type_string);
							}
							else
							{
								display_message(ERROR_MESSAGE,"convert_signal_rig_to_nodes."
									"error defining signal_status_field");	
								return_code =0;
							}					
						}
						else
						{
							display_message(ERROR_MESSAGE,"convert_signal_rig_to_nodes."
								"error getting  signal_status_field");	
							return_code =0;
						}				
						/* add signal field to the node */
						if (define_FE_field_at_node(node,signal_field,
							fe_time_version, node_field_creator))
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
									for (j=0;j<number_of_samples;j++)
									{
										set_FE_nodal_short_value(node,&component,0,
											FE_NODAL_VALUE,times[j],node_signals_short[j]);
									}
								} break;
								case FLOAT_VALUE:
								{
									for (j=0;j<number_of_samples;j++)
									{
										set_FE_nodal_FE_value_value(node,&component,0,
											FE_NODAL_VALUE,times[j],node_signals_fe_value[j]);
									}
								} break;
							}	/* switch (signal_value_type) */											
						}
						else
						{
							display_message(ERROR_MESSAGE,"convert_signal_rig_to_nodes."
								" define_field_at_node failed");							
							return_code=0;
						}
						/* merge node back into the fe_region */
						FE_region_merge_FE_node(root_fe_region, node);
						/* destroy the working copy */
						DESTROY(FE_node)(&node);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"convert_signal_rig_to_nodes.  CREATE(FE_node) failed");
						return_code=0;
					}
					device++;
				} /* for(i=0;i<number_of_nodes;i++) */
				FE_region_end_change(root_fe_region);
				DESTROY(FE_node_field_creator)(&node_field_creator);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"rig_node_add_map_electrode_position_field"
					"  Unable to make node field creator.");
				return_code=0;
			}
		}	/* if (return_code)	*/																		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"convert_signal_rig_to_nodes.  Invalid argument(s)");
		return_code=0;
	}	
	/* clean up */
	DEALLOCATE(node_signals_fe_value);
	DEALLOCATE(node_signals_short);	
	DEALLOCATE(times);	
	LEAVE;
	return (return_code);
}/* convert_signal_rig_to_nodes */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int convert_rig_to_nodes(struct Rig *rig)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION : 
Converts a rig, including it's signals to nodes and fields.
cf file_read_signal_FE_node_group
==============================================================================*/
{
	int return_code;
	struct Cmiss_region *root_cmiss_region;
	struct FE_field *signal_status_field;
	struct FE_node_order_info *all_devices_node_order_info;
	struct FE_region *unrejected_node_group,*rig_node_group;	
	struct Region *region;
	struct Region_list_item *region_item;
	struct Unemap_package *unemap_package;

	ENTER(convert_rig_to_nodes);	
	unemap_package=(struct Unemap_package *)NULL;
	signal_status_field = (struct FE_field *)NULL;
	unrejected_node_group= (struct FE_region *)NULL;
	rig_node_group= (struct FE_region *)NULL;	
	region=(struct Region *)NULL;
	region_item=(struct Region_list_item *)NULL;
	all_devices_node_order_info=(struct FE_node_order_info *)NULL;
	return_code=0;
	if (rig && (unemap_package=rig->unemap_package) &&
		(root_cmiss_region = get_unemap_package_root_Cmiss_region(unemap_package)))
	{
		if(return_code=convert_config_rig_to_nodes(rig,&all_devices_node_order_info))
		{										
		  if(return_code=convert_signal_rig_to_nodes(rig,all_devices_node_order_info))
		  {				
				/* set up the unrejected_node_groups*/
				region_item=get_Rig_region_list(rig);
				while (region_item)
				{	
					region = get_Region_list_item_region(region_item);
					rig_node_group = get_Region_rig_node_group(region);
					if ((signal_status_field =
						get_unemap_package_signal_status_field(unemap_package)) &&
						(unrejected_node_group = make_unrejected_node_group(
							root_cmiss_region, rig_node_group,
							signal_status_field)))
					{									
						set_Region_unrejected_node_group(region,unrejected_node_group);
					}
					else
					{
						display_message(ERROR_MESSAGE,"convert_signal_rig_to_nodes failed."
							"can't set_Region_unrejected_node_group");
					}
					region_item=get_Region_list_item_next(region_item);							
				}/* while (region_item) */		
			}
			else
			{
				display_message(ERROR_MESSAGE,
					" convert_rig_to_nodes. convert_signal_rig_to_nodes failed");	
			}
		}			
		else
		{
			display_message(ERROR_MESSAGE,
				"convert_rig_to_nodes. convert_signal_rig_to_nodes failed");	
		}					
		/* no longer needed */
		DEACCESS(FE_node_order_info)(&all_devices_node_order_info);
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"convert_rig_to_nodes. Invalid argument(s)");		
	}
	LEAVE;

	return(return_code);
}/* convert_rig_to_nodes */
#endif /* defined (UNEMAP_USE_3D) */
