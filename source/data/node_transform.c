/*******************************************************************************
FILE : node_transform.c

LAST MODIFIED : 31 August 2001

DESCRIPTION :
???DB.  Conditions need tidying up, generalizing and put in their own module
- list rather than array of conditions ?
- more than just logical and ?
- part of expression analyser ?
- in general ?
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include "command/command.h"
#include "data/node_transform.h"
#include "finite_element/finite_element.h"
#include "general/debug.h"
#include "io_devices/matrix.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module constants
----------------
*/
#define COORDINATES_2D_FIELD_NAME "coordinates_2d_rc"
#define COORDINATES_3D_FIELD_NAME "coordinates_3d_rc"

/*
Module types
------------
*/
struct FE_node_coord_pair
/*******************************************************************************
LAST MODIFIED : 23 May 1996

DESCRIPTION :
Used to bind a node with a physical position, irrespective of what coordinate
it thinks it is at.
==============================================================================*/
{
	int node_number;
	int dimension;
	FE_value *values;
}; /* FE_node_coord_pair */

enum Condition_type
/*******************************************************************************
LAST MODIFIED : 23 May 1996

DESCRIPTION :
Details the type of condition.
???DB.  See comments in module heading
==============================================================================*/
{
	CONDITION_ALL,    /* always true */
	CONDITION_NONE,   /* always false */
	CONDITION_EQ,     /* == */
	CONDITION_NE,     /* != */
	CONDITION_GE,     /* >= */
	CONDITION_GT,     /* >  */
	CONDITION_LE,     /* <= */
	CONDITION_LT      /* <  */
}; /* enum Condition_type */

struct Condition
/*******************************************************************************
LAST MODIFIED : 23 May 1996

DESCRIPTION :
A logical condition - eg >= 10 or !=0.9 etc
???DB.  See comments in module heading
==============================================================================*/
{
	enum Condition_type type;
	FE_value value;
}; /* struct Condition */

struct Transform_nodes_2d_to_3d_sub_sub
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Used by function transform_nodes_2d_to_3d_sub_sub to pass information to a list
iterator.
==============================================================================*/
{
	int base_number;
	FE_value elevation;
	FE_value pixel_origin[2];
	FE_value world_origin[2];
	FE_value world_delta_per_x_pixel[2];
	FE_value world_delta_per_y_pixel[2];
	struct LIST(GROUP(FE_node)) *destination;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_node) *node_manager;
}; /* struct Transform_nodes_2d_to_3d_sub_sub */

struct Transform_nodes_2d_scale_sub_sub
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Used by function transform_nodes_2d_scale_sub_sub to pass information to a list
iterator.
==============================================================================*/
{
	int base_number;
	FE_value *origin;
	FE_value *scale;
	struct LIST(GROUP(FE_node)) *destination;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_node) *node_manager;
}; /* struct Transform_nodes_2d_scale_sub_sub */

struct Export_nodes_sub_sub
/*******************************************************************************
LAST MODIFIED : 17 June 1996

DESCRIPTION :
Used by function export_nodes_sub_sub to pass information to a list iterator.
==============================================================================*/
{
	int base_number;
	struct Execute_command *execute_command;
}; /* struct Export_nodes_sub_sub */

struct Filter_nodes_sub_sub
/*******************************************************************************
LAST MODIFIED : 23 May 1996

DESCRIPTION :
Used by function filter_nodes_sub_sub to pass information to a list iterator.
==============================================================================*/
{
	struct Condition *conditions;
	int num_conditions;
	struct LIST(GROUP(FE_node)) *destination;
}; /* struct Filter_nodes_sub_sub */

/*
Module functions
----------------
*/
static struct FE_node_coord_pair *CREATE(FE_node_coord_pair)(int node_number,
	int dimension)
/*******************************************************************************
LAST MODIFIED : 23 May 1996

DESCRIPTION :
Creates a FE_node_coord_pair of the specified dimension.
==============================================================================*/
{
	int i;
	struct FE_node_coord_pair *return_node_coord;

	ENTER(CREATE(FE_node_coord_pair));
	return_node_coord=(struct FE_node_coord_pair *)NULL;
	if (ALLOCATE(return_node_coord,struct FE_node_coord_pair,1))
	{
		return_node_coord->node_number=node_number;
		return_node_coord->dimension=dimension;
		if (ALLOCATE(return_node_coord->values,FE_value,
			return_node_coord->dimension))
		{
			for (i=0;i<return_node_coord->dimension;i++)
			{
				return_node_coord->values[i]=0.0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(FE_node_coord_pair).  Could not allocate values");
			DEALLOCATE(return_node_coord);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(FE_node_coord_pair).  Could not create FE_node_coord_pair");
	}
	LEAVE;

	return return_node_coord;
} /* CREATE(FE_node_coord_pair) */

static int DESTROY(FE_node_coord_pair)(
	struct FE_node_coord_pair **node_coord_address)
/*******************************************************************************
LAST MODIFIED : 23 May 1996

DESCRIPTION :
Destroys the FE_node_coord_pair.
==============================================================================*/
{
	int return_code;
	struct FE_node_coord_pair *temp_node_coord;

	ENTER(DESTROY(FE_node_coord_pair));
	return_code=0;
	if (node_coord_address)
	{
		if (temp_node_coord= *node_coord_address)
		{
			if (temp_node_coord->values)
			{
				DEALLOCATE(temp_node_coord->values);
				DEALLOCATE(temp_node_coord);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"DESTROY(FE_node_coord_pair).  Invalid FE_node_coord_pair values");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
			"DESTROY(FE_node_coord_pair).  Invalid FE_node_coord_pair");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(FE_node_coord_pair).  Invalid FE_node_coord_pair address");
	}
	LEAVE;

	return return_code;
} /* DESTROY(FE_node_coord_pair) */

static int set_FE_node_coord_pair(struct Parse_state *state,
	void *node_coord_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 29 June 1996

DESCRIPTION :
A modifier function for setting a FE_node_coord_pair.
==============================================================================*/
{
	char *current_token;
	int i,return_code;
	struct FE_node_coord_pair *node_coord;

	ENTER(set_FE_node_coord_pair);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (node_coord=(struct FE_node_coord_pair *)node_coord_void)
				{
					if (1==sscanf(current_token," %i ",&node_coord->node_number))
					{
						if (node_coord->values)
						{
							return_code=shift_Parse_state(state,1);
							i=0;
							while (return_code&&(i<node_coord->dimension))
							{
								if (current_token=state->current_token)
								{
									if (1==sscanf(current_token," %f ",&(node_coord->values[i])))
									{
										return_code=shift_Parse_state(state,1);
										i++;
									}
									else
									{
										display_message(WARNING_MESSAGE,"Invalid value: %s",
											current_token);
										display_parse_state_location(state);
									}
								}
								else
								{
									display_message(WARNING_MESSAGE,"Missing value");
									display_parse_state_location(state);
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_FE_node_coord_pair.  Missing node_coord->values");
							return_code=0;
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"Invalid node number: %s",
							current_token);
						display_parse_state_location(state);
						return_code=1;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_FE_node_coord_pair.  Missing node_coord");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				if (node_coord=(struct FE_node_coord_pair *)node_coord_void)
				{
					display_message(INFORMATION_MESSAGE,"[%i]",node_coord->node_number);
				}
				display_message(INFORMATION_MESSAGE,"{integer} COORDINATES");
				if (node_coord&&node_coord->values)
				{
					display_message(INFORMATION_MESSAGE,"[");
					for (i=0;i<node_coord->dimension;i++)
					{
						display_message(INFORMATION_MESSAGE,FE_VALUE_INPUT_STRING,
							node_coord->values[i]);
						if (i<node_coord->dimension-1)
						{
							display_message(INFORMATION_MESSAGE," ");
						}
					}
					display_message(INFORMATION_MESSAGE,"]");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing node number");
			display_parse_state_location(state);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_FE_node_coord_pair.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_node_coord_pair */

#if defined (OLD_CODE)
static struct Condition *CREATE(Condition)(enum Condition_type type,
	FE_value value)
/*******************************************************************************
LAST MODIFIED : 23 May 1996

DESCRIPTION :
Creates a condition.
==============================================================================*/
{
	struct Condition *condition;

	ENTER(CREATE(Condition));
	if (ALLOCATE(condition,struct Condition,1))
	{
		condition->type=type;
		condition->value=value;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Condition).  Could not create Condition");
	}
	LEAVE;

	return (condition);
} /* CREATE(Condition) */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static int DESTROY(Condition)(struct Condition **condition_address)
/*******************************************************************************
LAST MODIFIED : 23 May 1996

DESCRIPTION :
Destroys the Condition.
???DB.  Doesn't free memory.  Because using array instead of list for a set of
	conditions ?
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Condition));
	return_code=0;
	if (condition_address)
	{
		if (*condition_address)
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"DESTROY(Condition).  Invalid Condition");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Condition).  Invalid Condition address");
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Condition) */
#endif /* defined (OLD_CODE) */

static int set_Condition(struct Parse_state *state,void *condition_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 7 October 1996

DESCRIPTION :
A modifier function for setting a Condition.
==============================================================================*/
{
	char *current_token;
	int read_value,return_code;
	struct Condition *condition;

	ENTER(set_Condition);
	USE_PARAMETER(dummy_user_data);
	/* default return_code if forget to set */
	return_code=0;
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (condition=(struct Condition *)condition_void)
				{
					/* default read_value if forget to set */
					read_value=0;
					if (fuzzy_string_compare(current_token,"ALL"))
					{
						shift_Parse_state(state,1);
						condition->type=CONDITION_ALL;
						return_code=1;
						read_value=0;
					}
					else
					{
						if (fuzzy_string_compare(current_token,"NONE"))
						{
							shift_Parse_state(state,1);
							condition->type=CONDITION_NONE;
							return_code=1;
							read_value=0;
						}
						else
						{
							if (strcmp(current_token,"=="))
							{
								if (strcmp(current_token,"!="))
								{
									if (strcmp(current_token,">="))
									{
										if (strcmp(current_token,">"))
										{
											if (strcmp(current_token,"<="))
											{
												if (strcmp(current_token,"<"))
												{
													display_message(ERROR_MESSAGE,
														"set_Condition.  Invalid condition type");
													return_code=0;
													read_value=0;
												}
												else
												{
													shift_Parse_state(state,1);
													condition->type=CONDITION_LT;
													return_code=1;
													read_value=1;
												}
											}
											else
											{
												shift_Parse_state(state,1);
												condition->type=CONDITION_LE;
												return_code=1;
												read_value=1;
											}
										}
										else
										{
											shift_Parse_state(state,1);
											condition->type=CONDITION_GT;
											return_code=1;
											read_value=1;
										}
									}
									else
									{
										shift_Parse_state(state,1);
										condition->type=CONDITION_GE;
										return_code=1;
										read_value=1;
									}
								}
								else
								{
									shift_Parse_state(state,1);
									condition->type=CONDITION_NE;
									return_code=1;
									read_value=1;
								}
							}
							else
							{
								shift_Parse_state(state,1);
								condition->type=CONDITION_EQ;
								return_code=1;
								read_value=1;
							}
							if (return_code&&read_value)
							{
								if (current_token=state->current_token)
								{
									if (1==sscanf(current_token," "FE_VALUE_INPUT_STRING" ",
										&(condition->value)))
									{
										return_code=shift_Parse_state(state,1);
									}
									else
									{
										display_message(ERROR_MESSAGE,"Invalid condition value: %s",
											current_token);
										display_parse_state_location(state);
										return_code=0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,"Missing condition value");
									display_parse_state_location(state);
									return_code=0;
								}
							}
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Condition.  Missing condition_type");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," CONDITION");
				if (condition=(struct Condition *)condition_void)
				{
					switch (condition->type)
					{
						case CONDITION_ALL:
						{
							display_message(INFORMATION_MESSAGE,"[all]");
						} break;
						case CONDITION_NONE:
						{
							display_message(INFORMATION_MESSAGE,"[none]");
						} break;
						case CONDITION_EQ:
						{
							display_message(INFORMATION_MESSAGE,"[== %"FE_VALUE_STRING"]",
								condition->value);
						} break;
						case CONDITION_NE:
						{
							display_message(INFORMATION_MESSAGE,"[!= %"FE_VALUE_STRING"]",
								condition->value);
						} break;
						case CONDITION_GE:
						{
							display_message(INFORMATION_MESSAGE,"[>= %"FE_VALUE_STRING"]",
								condition->value);
						} break;
						case CONDITION_GT:
						{
							display_message(INFORMATION_MESSAGE,"[> %"FE_VALUE_STRING"]",
								condition->value);
						} break;
						case CONDITION_LE:
						{
							display_message(INFORMATION_MESSAGE,"[<= %"FE_VALUE_STRING"]",
								condition->value);
						} break;
						case CONDITION_LT:
						{
							display_message(INFORMATION_MESSAGE,"[< %"FE_VALUE_STRING"]",
								condition->value);
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"set_Condition.  Invalid condition type");
						} break;
					}
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing condition type");
			display_parse_state_location(state);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Condition.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Condition */

static int meet_Conditions(struct Condition *conditions,
	int number_of_conditions,FE_value *values,int number_of_values)
/*******************************************************************************
LAST MODIFIED : 23 May 1996

DESCRIPTION :
Tests if the values meet all the given conditions.
???DB.  Assumes value for ALL and NONE.  Should this be ?
==============================================================================*/
{
	FE_value *value;
	int i,return_code;
	struct Condition *condition;

	ENTER(meet_Conditions);
	if ((condition=conditions)&&(value=values))
	{
		return_code=1;
		if (number_of_conditions<number_of_values)
		{
			i=number_of_conditions;
		}
		else
		{
			i=number_of_values;
		}
		while (return_code&&(i>0))
		{
			switch (condition->type)
			{
				case CONDITION_ALL:
				{
					/* do nothing */
				} break;
				case CONDITION_NONE:
				{
					return_code=0;
				} break;
				case CONDITION_EQ:
				{
					if (*value!=condition->value)
					{
						return_code=0;
					}
				} break;
				case CONDITION_NE:
				{
					if (*value==condition->value)
					{
						return_code=0;
					}
				} break;
				case CONDITION_GE:
				{
					if (*value<condition->value)
					{
						return_code=0;
					}
				} break;
				case CONDITION_GT:
				{
					if (*value<=condition->value)
					{
						return_code=0;
					}
				} break;
				case CONDITION_LE:
				{
					if (*value>condition->value)
					{
						return_code=0;
					}
				} break;
				case CONDITION_LT:
				{
					if (*value>=condition->value)
					{
						return_code=0;
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"meet_Conditions.  Invalid condition type");
					return_code=0;
				} break;
			}
			i--;
			condition++;
			value++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"meet_Conditions.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* meet_Conditions */

static int transform_nodes_2d_to_3d_sub_sub_sub(
	struct GROUP(FE_node) *node_group,void *user_data)
/*******************************************************************************
LAST MODIFIED : 23 May 1996

DESCRIPTION :
Transforms a given node to 3d.
==============================================================================*/
{
	int return_code;
	struct FE_node *node;

	ENTER(transform_nodes_2d_to_3d_sub_sub_sub);
	return_code=0;
	if (node_group&&(node=(struct FE_node *)user_data))
	{
		return_code=ADD_OBJECT_TO_GROUP(FE_node)(node,node_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transform_nodes_2d_to_3d_sub_sub_sub.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* transform_nodes_2d_to_3d_sub_sub_sub */

static int transform_nodes_2d_to_3d_sub_sub(struct FE_node *node,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 31 August 2001

DESCRIPTION :
Transforms a given node to 3d.
==============================================================================*/
{
	char *component_names[3]=
	{
		"X","Y","Z"
	};
	FE_value image_values[3],temp_values[3];
	int node_number,return_code;
	struct FE_field *new_field;
	struct FE_node *new_node,*template_node;
	struct FE_node_field_creator *node_field_creator;
	struct Transform_nodes_2d_to_3d_sub_sub *temp_data;
	struct Coordinate_system rect_cart_coords;

	rect_cart_coords.type =  RECTANGULAR_CARTESIAN;

	ENTER(transform_nodes_2d_to_3d_sub_sub);
	return_code=0;
	if (node&&(temp_data=(struct Transform_nodes_2d_to_3d_sub_sub *)user_data))
	{
		if (FE_node_get_position_cartesian(node,(struct FE_field *)NULL,
			image_values,image_values+1,image_values+2,(FE_value *)NULL))
		{
			image_values[0] -= temp_data->pixel_origin[0];
			image_values[1] -= temp_data->pixel_origin[1];
			/* transform the coordinates to correct 3d */
			temp_values[0]=temp_data->world_origin[0]+
				temp_data->world_delta_per_x_pixel[0]*image_values[0]+
				temp_data->world_delta_per_y_pixel[0]*image_values[1];
			temp_values[1]=temp_data->world_origin[1]+
				temp_data->world_delta_per_x_pixel[1]*image_values[0]+
				temp_data->world_delta_per_y_pixel[1]*image_values[1];
			temp_values[2]=temp_data->elevation;
			/* and add a new node */
			node_number=get_next_FE_node_number(temp_data->node_manager,
				temp_data->base_number);
			temp_data->base_number=node_number+1;
			if (new_field=get_FE_field_manager_matched_field(
				temp_data->fe_field_manager,COORDINATES_3D_FIELD_NAME,
				GENERAL_FE_FIELD,(struct FE_time *)NULL,
				/*indexer_field*/(struct FE_field *)NULL,
				/*number_of_indexed_values*/0,CM_COORDINATE_FIELD,
				&rect_cart_coords,FE_VALUE_VALUE,
				/*number_of_components*/3,component_names,
				/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
				(struct FE_field_external_information *)NULL))
			{
				if ((node_field_creator = CREATE(FE_node_field_creator)(
					/*number_of_components*/3))&&
					(template_node=CREATE(FE_node)(0,(struct FE_node *)NULL))
					&&define_FE_field_at_node(template_node,new_field,
					(struct FE_time_version *)NULL,node_field_creator))
				{
					if (new_node=CREATE(FE_node)(node_number,template_node))
					{	
						int length;
						set_FE_nodal_field_FE_value_values(new_field,
							new_node,temp_values,&length);

						if (ADD_OBJECT_TO_MANAGER(FE_node)(new_node,
							temp_data->node_manager))
						{
							return_code=FOR_EACH_OBJECT_IN_LIST(GROUP(FE_node))(
								transform_nodes_2d_to_3d_sub_sub_sub,new_node,
								temp_data->destination);
						}
						else
						{
							display_message(ERROR_MESSAGE,
				"transform_nodes_2d_to_3d_sub_sub.  Could not add FE_node to manager");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"transform_nodes_2d_to_3d_sub_sub.  Could not create FE_node");
					}
					DESTROY(FE_node_field_creator)(&(node_field_creator));
					DESTROY(FE_node)(&template_node);
				}
				else
				{
					display_message(ERROR_MESSAGE,
					"transform_nodes_2d_to_3d_sub_sub.  Could not create template_node");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"transform_nodes_2d_to_3d_sub_sub.  Could not create FE_field");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
		"transform_nodes_2d_to_3d_sub_sub.  Could not calculate coordinate field");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transform_nodes_2d_to_3d_sub_sub.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* transform_nodes_2d_to_3d_sub_sub */

static int transform_nodes_2d_to_3d_sub(struct GROUP(FE_node) *node_group,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 23 May 1996

DESCRIPTION :
Transforms the group of nodes to 3d.
==============================================================================*/
{
	int return_code;

	ENTER(transform_nodes_2d_to_3d_sub);
	if (node_group&&user_data)
	{
		return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(
			transform_nodes_2d_to_3d_sub_sub,user_data,node_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transform_nodes_2d_to_3d_sub.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* transform_nodes_2d_to_3d_sub */

static int transform_nodes_2d_to_3d(struct LIST(GROUP(FE_node)) *source,
	struct FE_node_coord_pair *origin,struct FE_node_coord_pair *right,
	struct FE_node_coord_pair *top,FE_value elevation,
	struct LIST(GROUP(FE_node)) *destination,int base_number,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Transforms nodes in the <source> node groups to 3d using the information
contained in the origin, right and top node-coordinate pairs.  Nodes are
added to the global list (node number >= base_number) and to any groups
specified in <destination>.
==============================================================================*/
{
	FE_value new_coords[3];
	Gmatrix global_delta,pixel_delta,pixel_delta_inverse,conversion;
	int return_code;
	struct Transform_nodes_2d_to_3d_sub_sub temp_data;
	struct FE_node *origin_node,*right_node,*top_node;

	ENTER(transform_nodes_2d_to_3d);
	return_code=0;
	if (source&&origin&&right&&top&&destination)
	{
		origin_node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,cm_node_identifier)(
			origin->node_number,node_manager);
		right_node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,cm_node_identifier)(
			right->node_number,node_manager);
		top_node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,cm_node_identifier)(
			top->node_number,node_manager);
		if (origin_node&&right_node&&top_node)
		{
			return_code=1;
			matrix_I(&global_delta);
			matrix_I(&pixel_delta);
			/* get the coordinate field */
			if (FE_node_get_position_cartesian(origin_node,(struct FE_field *)NULL,
				new_coords,new_coords+1,new_coords+2,(FE_value *)NULL))
			{
				temp_data.pixel_origin[0]=new_coords[0];
				temp_data.pixel_origin[1]=new_coords[1];
				temp_data.world_origin[0]=origin->values[0];
				temp_data.world_origin[1]=origin->values[1];
			}
			else
			{
				display_message(ERROR_MESSAGE,
			"transform_nodes_2d_to_3d.  Could not calculate origin coordinate field");
				return_code=0;
			}
			/* get the coordinate field for right */
			if (FE_node_get_position_cartesian(right_node,(struct FE_field *)NULL,
				new_coords,new_coords+1,new_coords+2,(FE_value *)NULL))
			{
				global_delta.data[0][0]=right->values[0]-temp_data.world_origin[0];
				global_delta.data[1][0]=right->values[1]-temp_data.world_origin[1];
				pixel_delta.data[0][0]=new_coords[0]-temp_data.pixel_origin[0];
				pixel_delta.data[1][0]=new_coords[1]-temp_data.pixel_origin[1];
			}
			else
			{
				display_message(ERROR_MESSAGE,
			"transform_nodes_2d_to_3d.  Could not calculate right coordinate field");
				return_code=0;
			}
			/* get the coordinate field for top */
			if (FE_node_get_position_cartesian(top_node,(struct FE_field *)NULL,
				new_coords,new_coords+1,new_coords+2,(FE_value *)NULL))
			{
				global_delta.data[0][1]=top->values[0]-temp_data.world_origin[0];
				global_delta.data[1][1]=top->values[1]-temp_data.world_origin[1];
				pixel_delta.data[0][1]=new_coords[0]-temp_data.pixel_origin[0];
				pixel_delta.data[1][1]=new_coords[1]-temp_data.pixel_origin[1];
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"transform_nodes_2d_to_3d.  Could not calculate top coordinate field");
				return_code=0;
			}
			if (return_code)
			{
				temp_data.fe_field_manager=fe_field_manager;
				temp_data.node_manager=node_manager;
				temp_data.destination=destination;
				temp_data.base_number=base_number;
				temp_data.elevation=elevation;
				/* work out the inverse of the pixel matrix */
				matrix_inverse(&pixel_delta,&pixel_delta_inverse);
				/* multiply the global delta by this inverse to get the conversion
					matrix */
				matrix_mult(&global_delta,&pixel_delta_inverse,&conversion);
				/* now extract the conversion factors */
				temp_data.world_delta_per_x_pixel[0]=conversion.data[0][0];
				temp_data.world_delta_per_x_pixel[1]=conversion.data[1][0];
				temp_data.world_delta_per_y_pixel[0]=conversion.data[0][1];
				temp_data.world_delta_per_y_pixel[1]=conversion.data[1][1];
				FOR_EACH_OBJECT_IN_LIST(GROUP(FE_node))(transform_nodes_2d_to_3d_sub,
					&temp_data,source);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"transform_nodes_2d_to_3d.  Origin node(s) not found");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transform_nodes_2d_to_3d.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* transform_nodes_2d_to_3d */

static int gfx_transform_node_2d_to_3d(struct Parse_state *state,
	void *dummy_to_be_modified,void *node_transform_data_void)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Executes a GFX TRANSFORM NODE 2D_TO_3D command.  This command transforms nodes
(ie 2d) to 3d versions (ie used when digitising image data)
==============================================================================*/
{
	FE_value elevation;
	int return_code,base_number;
	static struct Modifier_entry option_table[]=
	{
		{"base_number",NULL,NULL,set_int},
		{"destination_groups",NULL,NULL,set_FE_node_group_list},
		{"elevation",NULL,NULL,set_float},
		{"origin",NULL,NULL,set_FE_node_coord_pair},
		{"right",NULL,NULL,set_FE_node_coord_pair},
		{"source_groups",NULL,NULL,set_FE_node_group_list},
		{"top",NULL,NULL,set_FE_node_coord_pair},
		{NULL,NULL,NULL,NULL}
	};
	struct FE_node_coord_pair *origin,*right,*top;
	struct Node_transform_data *node_transform_data;
	struct LIST(GROUP(FE_node)) *destination_groups,*source_groups;

	ENTER(gfx_transform_node_2d_to_3d);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(node_transform_data=
		(struct Node_transform_data *)node_transform_data_void))
	{
		source_groups=CREATE(LIST(GROUP(FE_node)))();
		origin=CREATE(FE_node_coord_pair)(1,2);
		right=CREATE(FE_node_coord_pair)(2,2);
		top=CREATE(FE_node_coord_pair)(3,2);
		elevation=0.0;
		destination_groups=CREATE(LIST(GROUP(FE_node)))();
		base_number=10000;
		/* initialise defaults */
		(option_table[0]).to_be_modified= &base_number;
		(option_table[1]).to_be_modified=destination_groups;
		(option_table[2]).to_be_modified= &elevation;
		(option_table[3]).to_be_modified=origin;
		(option_table[4]).to_be_modified=right;
		(option_table[5]).to_be_modified=source_groups;
		(option_table[6]).to_be_modified=top;
		return_code=process_multiple_options(state,option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			transform_nodes_2d_to_3d(source_groups,origin,right,top,elevation,
				destination_groups,base_number,node_transform_data->fe_field_manager,
				node_transform_data->node_manager);
		} /* parse error, help */
		DESTROY(LIST(GROUP(FE_node)))(&source_groups);
		DESTROY(FE_node_coord_pair)(&origin);
		DESTROY(FE_node_coord_pair)(&right);
		DESTROY(FE_node_coord_pair)(&top);
		DESTROY(LIST(GROUP(FE_node)))(&destination_groups);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_transform_node_2d_to_3d.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_transform_node_2d_to_3d */

static int transform_nodes_2d_scale_sub_sub(struct FE_node *node,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 31 August 2001

DESCRIPTION :
Transforms a given node in 2d space - adds an origin and scales the pixel
coordinates.
==============================================================================*/
{
	char *component_names[2]=
	{
		"X","Y"
	};
	FE_value global_values[2],image_values[3];
	int node_number,return_code;
	struct FE_field *new_field;
	struct FE_node *new_node,*template_node;
	struct FE_node_field_creator *node_field_creator;
	struct Transform_nodes_2d_scale_sub_sub *temp_data;
	struct Coordinate_system rect_cart_coords;

	ENTER(transform_nodes_2d_scale_sub_sub);
	return_code=0;
	rect_cart_coords.type=RECTANGULAR_CARTESIAN;
	if (node&&(temp_data=(struct Transform_nodes_2d_scale_sub_sub *)user_data))
	{
		if (FE_node_get_position_cartesian(node,(struct FE_field *)NULL,
			image_values,image_values+1,image_values+2,(FE_value *)NULL))
		{
			global_values[0]=temp_data->origin[0]+
				image_values[0]*temp_data->scale[0];
			global_values[1]=temp_data->origin[1]+
				image_values[1]*temp_data->scale[1];
			/* and add a new node */
			node_number=get_next_FE_node_number(temp_data->node_manager,
				temp_data->base_number);
			temp_data->base_number=node_number+1;
			if (new_field=get_FE_field_manager_matched_field(
				temp_data->fe_field_manager,COORDINATES_2D_FIELD_NAME,
				GENERAL_FE_FIELD,(struct FE_time *)NULL,
				/*indexer_field*/(struct FE_field *)NULL,
				/*number_of_indexed_values*/0,CM_COORDINATE_FIELD,
				&rect_cart_coords,FE_VALUE_VALUE,
				/*number_of_components*/2,component_names,
				/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
				(struct FE_field_external_information *)NULL))
			{
				if ((node_field_creator = CREATE(FE_node_field_creator)(
					/*number_of_components*/2))&&
					(template_node=CREATE(FE_node)(0,(struct FE_node *)NULL))&&
					define_FE_field_at_node(template_node,new_field,
					(struct FE_time_version *)NULL,node_field_creator))
				{
					if (new_node=CREATE(FE_node)(node_number,template_node))
					{					
						int length;
						set_FE_nodal_field_FE_value_values(new_field,new_node,
							global_values,&length);

						if (ADD_OBJECT_TO_MANAGER(FE_node)(new_node,
							temp_data->node_manager))
						{
							/* add to the destination groups */
							return_code = FOR_EACH_OBJECT_IN_LIST(GROUP(FE_node))(
								transform_nodes_2d_to_3d_sub_sub_sub,new_node,
								temp_data->destination);
						}
						else
						{
							display_message(ERROR_MESSAGE,
				"transform_nodes_2d_scale_sub_sub.  Could not add FE_node to manager");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"transform_nodes_2d_scale_sub_sub.  Could not create new_node");
					}
					DESTROY(FE_node_field_creator)(&(node_field_creator));
					DESTROY(FE_node)(&template_node);
				}
				else
				{
					display_message(ERROR_MESSAGE,
					"transform_nodes_2d_scale_sub_sub.  Could not create template_node");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"transform_nodes_2d_scale_sub_sub.  Could not create FE_field");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
		"transform_nodes_2d_scale_sub_sub.  Could not calculate coordinate field");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transform_nodes_2d_scale_sub_sub.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* transform_nodes_2d_scale_sub_sub */

static int transform_nodes_2d_scale_sub(struct GROUP(FE_node) *node_group,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 23 May 1996

DESCRIPTION :
Transforms the group of nodes to 3d.
==============================================================================*/
{
	int return_code;

	ENTER(transform_nodes_2d_scale_sub);
	if (node_group&&user_data)
	{
		return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(
			transform_nodes_2d_scale_sub_sub,(void *)user_data,node_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transform_nodes_2d_scale_sub.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* transform_nodes_2d_scale_sub */

static int transform_nodes_2d_scale(struct LIST(GROUP(FE_node)) *source,
	FE_value *origin,FE_value *scale,struct LIST(GROUP(FE_node)) *destination,
	int base_number,struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Transform_nodes_2d_scale_sub_sub temp_data;

	ENTER(transform_nodes_2d_scale);
	if (source&&origin&&scale&&destination)
	{
		temp_data.destination=destination;
		temp_data.base_number=base_number;
		temp_data.origin=origin;
		temp_data.scale=scale;
		temp_data.fe_field_manager=fe_field_manager;
		temp_data.node_manager=node_manager;
		return_code=FOR_EACH_OBJECT_IN_LIST(GROUP(FE_node))(
			transform_nodes_2d_scale_sub,(void *)(&temp_data),source);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transform_nodes_2d_scale.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* transform_nodes_2d_scale */

static int gfx_transform_node_2d_scale(struct Parse_state *state,
	void *dummy_to_be_modified,void *node_transform_data_void)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Executes a GFX TRANSFORM NODE 2D_SCALE command.  This command transforms nodes
in 2d space.
==============================================================================*/
{
	FE_value origin[2],scale[2];
	int base_number,return_code;
	static struct Modifier_entry option_table[]=
	{
		{"base_number",NULL,NULL,set_int},
		{"destination_groups",NULL,NULL,set_FE_node_group_list},
		{"origin_x",NULL,NULL,set_float},
		{"origin_y",NULL,NULL,set_float},
		{"scale_x",NULL,NULL,set_float},
		{"scale_y",NULL,NULL,set_float},
		{"source_groups",NULL,NULL,set_FE_node_group_list},
		{NULL,NULL,NULL,NULL}
	};
	struct LIST(GROUP(FE_node)) *source_groups,*destination_groups;
	struct Node_transform_data *node_transform_data;

	ENTER(gfx_transform_node_2d_scale);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(node_transform_data=
		(struct Node_transform_data *)node_transform_data_void))
	{
		source_groups=CREATE(LIST(GROUP(FE_node)))();
		origin[0]=0.0;
		origin[1]=0.0;
		scale[0]=1.0;
		scale[1]=1.0;
		destination_groups=CREATE(LIST(GROUP(FE_node)))();
		base_number=10000;
		/* initialise defaults */
		(option_table[0]).to_be_modified= &base_number;
		(option_table[1]).to_be_modified=destination_groups;
		(option_table[2]).to_be_modified= &origin[0];
		(option_table[3]).to_be_modified= &origin[1];
		(option_table[4]).to_be_modified= &scale[0];
		(option_table[5]).to_be_modified= &scale[1];
		(option_table[6]).to_be_modified=source_groups;
		return_code=process_multiple_options(state,option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			transform_nodes_2d_scale(source_groups,origin,scale,destination_groups,
				base_number,node_transform_data->fe_field_manager,
				node_transform_data->node_manager);
		} /* parse error, help */
		DESTROY(LIST(GROUP(FE_node)))(&source_groups);
		DESTROY(LIST(GROUP(FE_node)))(&destination_groups);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_transform_node_2d_scale.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_transform_node_2d_scale */

static int export_nodes_sub_sub(struct FE_node *node,void *user_data)
/*******************************************************************************
LAST MODIFIED : 2 November 1998

DESCRIPTION :
Exports a given node to 3d.
==============================================================================*/
{
	int return_code;
	FE_value image_values[3];
	struct Export_nodes_sub_sub *temp_data;

	ENTER(export_nodes_sub_sub);
	return_code=0;
	if (node&&(temp_data=(struct Export_nodes_sub_sub *)user_data))
	{
		if (FE_node_get_position_cartesian(node,(struct FE_field *)NULL,
			image_values,image_values+1,image_values+2,(FE_value *)NULL))
		{
			sprintf(global_temp_string, "FEM define data number %d"
				" x="FE_VALUE_INPUT_STRING
				" y="FE_VALUE_INPUT_STRING
				" z="FE_VALUE_INPUT_STRING,
				temp_data->base_number,
				image_values[0],image_values[1],image_values[2]);
			Execute_command_execute_string(temp_data->execute_command,
				global_temp_string);
			/* increment the data number */
			temp_data->base_number++;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"export_nodes_sub_sub.  Could not calculate coordinate field");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"export_nodes_sub_sub.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* export_nodes_sub_sub */

static int export_nodes_sub(struct GROUP(FE_node) *node_group,void *user_data)
/*******************************************************************************
LAST MODIFIED : 7 May 1996

DESCRIPTION :
Exports the group of nodes to 3d.
==============================================================================*/
{
	int return_code;

	ENTER(export_nodes_sub);
	if (node_group&&user_data)
	{
		return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(export_nodes_sub_sub,
			user_data,node_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,"export_nodes_sub.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* export_nodes_sub */

static int filter_nodes_sub_sub(struct FE_node *node,void *user_data)
/*******************************************************************************
LAST MODIFIED : 2 November 1998

DESCRIPTION :
Tests to see if the given node satisfies the conditions.
==============================================================================*/
{
	FE_value image_values[3];
	int return_code;
	struct Filter_nodes_sub_sub *temp_data;

	ENTER(filter_nodes_sub_sub);
	return_code=0;
	if (node&&(temp_data=(struct Filter_nodes_sub_sub *)user_data))
	{
		if (FE_node_get_position_cartesian(node,(struct FE_field *)NULL,
			image_values,image_values+1,image_values+2,(FE_value *)NULL))
		{
			if (meet_Conditions(temp_data->conditions,temp_data->num_conditions,
				image_values,3))
			{
				return_code=FOR_EACH_OBJECT_IN_LIST(GROUP(FE_node))(
					transform_nodes_2d_to_3d_sub_sub_sub,node,temp_data->destination);
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"filter_nodes_sub_sub.  Could not calculate coordinate field");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"filter_nodes_sub_sub.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* filter_nodes_sub_sub */

static int filter_nodes_sub(struct GROUP(FE_node) *node_group,void *user_data)
/*******************************************************************************
LAST MODIFIED : 29 December 1995

DESCRIPTION :
Transforms the group of nodes to 3d.
==============================================================================*/
{
	int return_code;

	ENTER(filter_nodes_sub);
	if (node_group&&user_data)
	{
		return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(filter_nodes_sub_sub,
			user_data,node_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,"filter_nodes_sub.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* filter_nodes_sub */

static int filter_nodes(struct LIST(GROUP(FE_node)) *source,
	struct Condition *conditions,int num_conditions,
	struct LIST(GROUP(FE_node)) *destination)
/*******************************************************************************
LAST MODIFIED : 23 May 1996

DESCRIPTION :
==============================================================================*/
{
	struct Filter_nodes_sub_sub temp_data;
	int return_code;

	ENTER(filter_nodes);
	if (source&&conditions&&destination)
	{
		temp_data.destination=destination;
		temp_data.conditions=conditions;
		temp_data.num_conditions=num_conditions;
		return_code=FOR_EACH_OBJECT_IN_LIST(GROUP(FE_node))(filter_nodes_sub,
			&temp_data,source);
	}
	else
	{
		display_message(ERROR_MESSAGE,"filter_nodes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* filter_nodes */

/*
Global functions
---------------
*/
int gfx_transform_node(struct Parse_state *state,
	void *dummy_to_be_modified,void *node_transform_data_void)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Executes a GFX TRANSFORM NODE command.  This command transforms nodes (ie 2d)
to 3d versions (ie used when digitising image data)
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"2d_scale",NULL,NULL,gfx_transform_node_2d_scale},
		{"2d_to_3d",NULL,NULL,gfx_transform_node_2d_to_3d},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_transform_node);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		(option_table[0]).user_data=node_transform_data_void;
		(option_table[1]).user_data=node_transform_data_void;
		return_code=process_option(state,option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_transform_node.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_transform_node */

int export_nodes(struct LIST(GROUP(FE_node)) *groups,int base_number,
	struct Execute_command *execute_command)
/*******************************************************************************
LAST MODIFIED : 7 November 1998

DESCRIPTION :
Executes the command FEM define data number %d x=%f y=%f z=%f
==============================================================================*/
{
	struct Export_nodes_sub_sub temp_data;
	int return_code;

	ENTER(export_nodes);
	return_code=0;
	if (groups)
	{
		temp_data.base_number=base_number;
		temp_data.execute_command=execute_command;
		return_code=FOR_EACH_OBJECT_IN_LIST(GROUP(FE_node))(export_nodes_sub,
			&temp_data,groups);
	}
	else
	{
		display_message(ERROR_MESSAGE,"export_nodes.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* export_nodes */

#if defined (OLD_CODE)
/*???DB.  Shifted to command/command.c */
int gfx_export_node(struct Parse_state *state,void *dummy_to_be_modified,
	void *execute_command_void)
/*******************************************************************************
LAST MODIFIED : 27 September 1996

DESCRIPTION :
Executes a GFX EXPORT NODE command.  This command exports nodes to cmiss as
data.
???DB.  Should this be in command.c ?
==============================================================================*/
{
	int base_number,return_code;
	struct Execute_command *execute_command;
	struct LIST(GROUP(FE_node)) *groups;
	static struct Modifier_entry option_table[]=
	{
		{"base_number",NULL,NULL,set_int},
		{"groups",NULL,NULL,set_FE_node_group_list},
		{NULL,NULL,NULL}
	};

	ENTER(gfx_export_node);
	if (state)
	{
		groups=CREATE(LIST(GROUP(FE_node)))();
		base_number=0;
		/* initialise defaults */
		(option_table[0]).to_be_modified= &base_number;
		(option_table[1]).to_be_modified=groups;
		return_code=process_multiple_options(state,option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (execute_command=(struct Execute_command *)execute_command_void)
			{
				export_nodes(groups,base_number,execute_command);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_export_node.  Missing execute_command");
				return_code=0;
			}
		} /* parse error, help */
		DESTROY(LIST(GROUP(FE_node)))(&groups);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_node.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_node */
#endif /* defined (OLD_CODE) */

int gfx_filter_node(struct Parse_state *state,void *dummy_to_be_modified,
	void *node_group_manager_void)
/*******************************************************************************
LAST MODIFIED : 26 June 1997

DESCRIPTION :
Executes a GFX FILTER NODE command.  This command tests nodes against a set
criteria and sends them to the destination groups.
==============================================================================*/
{
	int return_code;
	struct Condition conditions[3];
	struct LIST(GROUP(FE_node)) *source_groups,*destination_groups;
	static struct Modifier_entry option_table[]=
	{
		{"destination_groups",NULL,NULL,set_FE_node_group_list},
		{"source_groups",NULL,NULL,set_FE_node_group_list},
		{"x",NULL,NULL,set_Condition},
		{"y",NULL,NULL,set_Condition},
		{"z",NULL,NULL,set_Condition},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_filter_node);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		source_groups=CREATE(LIST(GROUP(FE_node)))();
		conditions[0].type=CONDITION_ALL;
		conditions[0].value=0.0;
		conditions[1].type=CONDITION_ALL;
		conditions[1].value=0.0;
		conditions[2].type=CONDITION_ALL;
		conditions[2].value=0.0;
		destination_groups=CREATE(LIST(GROUP(FE_node)))();
		/* initialise defaults */
		(option_table[0]).to_be_modified=destination_groups;
		(option_table[0]).user_data=node_group_manager_void;
		(option_table[1]).to_be_modified=source_groups;
		(option_table[1]).user_data=node_group_manager_void;
		(option_table[2]).to_be_modified= &conditions[0];
		(option_table[3]).to_be_modified= &conditions[1];
		(option_table[4]).to_be_modified= &conditions[2];
		return_code=process_multiple_options(state,option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			filter_nodes(source_groups,conditions,3,destination_groups);
		} /* parse error, help */
		DESTROY(LIST(GROUP(FE_node)))(&source_groups);
		DESTROY(LIST(GROUP(FE_node)))(&destination_groups);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_filter_node.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_filter_node */
