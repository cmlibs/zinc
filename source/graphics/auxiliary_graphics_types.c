/*******************************************************************************
FILE : auxiliary_graphics_types.c

LAST MODIFIED : 28 March 2000

DESCRIPTION :
Structures and enumerated types needed to produce graphics primitives but not
specific to any of them. Examples are:
- struct Element_discretization: stores the number of segments used to
represent curvesin three xi-directions;
- Triple;
==============================================================================*/
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "command/parser.h"
#include "general/debug.h"
#include "graphics/auxiliary_graphics_types.h"
#include "user_interface/user_interface.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/
char *Graphics_select_mode_string(enum Graphics_select_mode select_mode)
/*******************************************************************************
LAST MODIFIED : 23 February 2000

DESCRIPTION :
Returns a pointer to a static string describing the select_mode, eg.
GRAPHICS_NO_SELECT="no_select".This string should match the command
used to enact the mode. The returned string must not be DEALLOCATEd!
==============================================================================*/
{
	char *return_string;

	ENTER(Graphics_select_mode_string);
	switch (select_mode)
	{
		case GRAPHICS_SELECT_ON:
		{
			return_string="select_on";
		} break;
		case GRAPHICS_NO_SELECT:
		{
			return_string="no_select";
		} break;
		case GRAPHICS_DRAW_SELECTED:
		{
			return_string="draw_selected";
		} break;
		case GRAPHICS_DRAW_UNSELECTED:
		{
			return_string="draw_unselected";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Graphics_select_mode_string.  Unknown select_mode");
			return_string=(char *)NULL;
		} break;
	}
	LEAVE;

	return (return_string);
} /* Graphics_select_mode_string */

char **Graphics_select_mode_get_valid_strings(int *number_of_valid_strings)
/*******************************************************************************
LAST MODIFIED : 23 February 2000

DESCRIPTION :
Returns and allocated array of pointers to all static strings for valid
Graphics_select_modes - obtained from function Graphics_select_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/
{
	char **valid_strings;
	enum Graphics_select_mode select_mode;
	int i;

	ENTER(Graphics_select_mode_get_valid_strings);
	if (number_of_valid_strings)
	{
		*number_of_valid_strings=0;
		select_mode=GRAPHICS_SELECT_MODE_BEFORE_FIRST;
		select_mode++;
		while (select_mode<GRAPHICS_SELECT_MODE_AFTER_LAST)
		{
			(*number_of_valid_strings)++;
			select_mode++;
		}
		if (ALLOCATE(valid_strings,char *,*number_of_valid_strings))
		{
			select_mode=GRAPHICS_SELECT_MODE_BEFORE_FIRST;
			select_mode++;
			i=0;
			while (select_mode<GRAPHICS_SELECT_MODE_AFTER_LAST)
			{
				valid_strings[i]=Graphics_select_mode_string(select_mode);
				i++;
				select_mode++;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Graphics_select_mode_get_valid_strings.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_select_mode_get_valid_strings.  Invalid argument");
		valid_strings=(char **)NULL;
	}
	LEAVE;

	return (valid_strings);
} /* Graphics_select_mode_get_valid_strings */

enum Graphics_select_mode Graphics_select_mode_from_string(
	char *select_mode_string)
/*******************************************************************************
LAST MODIFIED : 23 February 2000

DESCRIPTION :
Returns the <Graphics_select_mode> described by <select_mode_string>,
or NULL if not recognized.
==============================================================================*/
{
	enum Graphics_select_mode select_mode;

	ENTER(Graphics_select_mode_from_string);
	if (select_mode_string)
	{
		select_mode=GRAPHICS_SELECT_MODE_BEFORE_FIRST;
		select_mode++;
		while ((select_mode<GRAPHICS_SELECT_MODE_AFTER_LAST)&&
			(!fuzzy_string_compare_same_length(select_mode_string,
				Graphics_select_mode_string(select_mode))))
		{
			select_mode++;
		}
		if (GRAPHICS_SELECT_MODE_AFTER_LAST==select_mode)
		{
			select_mode=GRAPHICS_SELECT_MODE_INVALID;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_select_mode_from_string.  Invalid argument");
		select_mode=GRAPHICS_SELECT_MODE_INVALID;
	}
	LEAVE;

	return (select_mode);
} /* Graphics_select_mode_from_string */

int set_exterior(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 13 July 1996

DESCRIPTION :
A modifier function for setting exterior flag and face number.
==============================================================================*/
{
	char *current_token;
	int return_code,*value_address;

	ENTER(set_exterior);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (value_address=(int *)value_address_void)
				{
					return_code=1;
					/*???DB.  Only valid for cubes (not polygons) */
					if (fuzzy_string_compare(current_token,"XI1_0"))
					{
						*value_address=2;
						shift_Parse_state(state,1);
					}
					else
					{
						if (fuzzy_string_compare(current_token,"XI1_1"))
						{
							*value_address=3;
							shift_Parse_state(state,1);
						}
						else
						{
							if (fuzzy_string_compare(current_token,"XI2_0"))
							{
								*value_address=4;
								shift_Parse_state(state,1);
							}
							else
							{
								if (fuzzy_string_compare(current_token,"XI2_1"))
								{
									*value_address=5;
									shift_Parse_state(state,1);
								}
								else
								{
									if (fuzzy_string_compare(current_token,"XI3_0"))
									{
										*value_address=6;
										shift_Parse_state(state,1);
									}
									else
									{
										if (fuzzy_string_compare(current_token,"XI3_1"))
										{
											*value_address=7;
											shift_Parse_state(state,1);
										}
										else
										{
											*value_address=1;
										}
									}
								}
							}
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_exterior.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" <xi1_0|xi1_1|xi2_0|xi2_1|xi3_0|xi3_1>");
				return_code=1;
			}
		}
		else
		{
			if (value_address=(int *)value_address_void)
			{
				*value_address=1;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"set_exterior.  Missing value_address");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_exterior.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_exterior */

int read_circle_discretization_defaults(int *default_value,
	int *maximum_value,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 11 December 1998

DESCRIPTION :
Reads that maximum and default number of line segments used to approximate
a circle. Minimum is always 2, but this does not look much like a circle!
???RC Changed so that does not report error if no user_interface - just returns
the defaults of 6 and 64. This is bad - should be able to read defaults without
any x application - incl. for compatibility with win32.
==============================================================================*/
{
#define XmNdefaultCircleDiscretization "defaultCircleDiscretization"
#define XmCDefaultCircleDiscretization "DefaultCircleDiscretization"
#define XmNmaximumCircleDiscretization "maximumCircleDiscretization"
#define XmCMaximumCircleDiscretization "MaximumCircleDiscretization"
	static int resources_read=0; /* flag so resources only read once */
	int return_code;
	static struct Discretization
	{
		int default_value;
		int maximum_value;
	} discretization;
	static XtResource resources[]=
	{
		{
			XmNdefaultCircleDiscretization,
			XmCDefaultCircleDiscretization,
			XmRInt,
			sizeof(int),
			XtOffsetOf(struct Discretization,default_value),
			XmRString,
			"6"
		},
		{
			XmNmaximumCircleDiscretization,
			XmCMaximumCircleDiscretization,
			XmRInt,
			sizeof(int),
			XtOffsetOf(struct Discretization,maximum_value),
			XmRString,
			"64"
		}
	};

	ENTER(read_circle_discretization_defaults);
	if (default_value&&maximum_value)
	{
		if (user_interface)
		{
			if (!resources_read)
			{
				/* retrieve the settings */
				XtVaGetApplicationResources(user_interface->application_shell,
					&discretization,resources,XtNumber(resources),NULL);
				resources_read=1;
			}
			*default_value=discretization.default_value;
			*maximum_value=discretization.maximum_value;
		}
		else
		{
			*default_value=6;
			*maximum_value=64;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_circle_discretization_defaults.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* read_circle_discretization_defaults */

int check_Circle_discretization(int *circle_discretization,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 11 December 1998

DESCRIPTION :
Ensures the <circle_discretization> is within the bounds of the minimum of 2
and the maximum read in from the defaults.
???RC user_interface argument not checked as may not be needed in
read_Circle_discretization_defaults().
==============================================================================*/
{
	int return_code;
	int default_value,maximum_value,initial_value;

	ENTER(check_Circle_discretization);
	if (circle_discretization)
	{
		if (return_code=read_circle_discretization_defaults(&default_value,
			&maximum_value,user_interface))
		{
			initial_value = *circle_discretization;
			if (*circle_discretization > maximum_value)
			{
				*circle_discretization=maximum_value;
			}
			if (2 > *circle_discretization)
			{
				*circle_discretization=2;
			}
			if (*circle_discretization != initial_value)
			{
				display_message(WARNING_MESSAGE,
					"Circle discretization values must be from 2 to %d\n"
					"%d changed to %d",maximum_value,initial_value,
					*circle_discretization);
			}
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"check_Circle_discretization.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return return_code;
} /* check_Circle_discretization */

int set_Circle_discretization(struct Parse_state *state,
	void *circle_discretization_void,void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
A modifier function for setting number of segments used to draw circles.
==============================================================================*/
{
	char *current_token;
	int return_code;
	int *circle_discretization;
	struct User_interface *user_interface;

	ENTER(set_Circle_discretization);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				user_interface=(struct User_interface *)user_interface_void;
				if (circle_discretization = (int *)circle_discretization_void)
				{
					if (1==sscanf(current_token,"%d",circle_discretization))
					{
						shift_Parse_state(state,1);
						return_code=check_Circle_discretization(circle_discretization,
							user_interface);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid discretization: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Circle_discretization.  Missing circle_discretization");
					return_code=0;
				}
			}
			else
			{
				if (circle_discretization = (int *)circle_discretization_void)
				{
					/* use negative value to signal that it is not determined which
						 current value is the default */
					if (0 > *circle_discretization)
					{
						display_message(INFORMATION_MESSAGE," #[CURRENT]{integer}");
					}
					else
					{
						display_message(INFORMATION_MESSAGE,
							" #[%d]{integer}",*circle_discretization);
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," #{integer}");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing circle discretization");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Circle_discretization.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Circle_discretization */

int read_element_discretization_defaults(int *default_value,
	int *maximum_value,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 11 December 1998

DESCRIPTION :
Reads that maximum and default number of line segments used to approximate
element curves. Minimum is always 1.
???RC Changed so that does not report error if no user_interface - just returns
the defaults of 4 and 50. This is bad - should be able to read defaults without
any x application - incl. for compatibility with win32.
==============================================================================*/
{
#define XmNdefaultElementDiscretization "defaultElementDiscretization"
#define XmCDefaultElementDiscretization "DefaultElementDiscretization"
#define XmNmaximumElementDiscretization "maximumElementDiscretization"
#define XmCMaximumElementDiscretization "MaximumElementDiscretization"
	static int resources_read=0; /* flag so resources only read once */
	int return_code;
	static struct Discretization
	{
		int default_value;
		int maximum_value;
	} discretization;
	static XtResource resources[]=
	{
		{
			XmNdefaultElementDiscretization,
			XmCDefaultElementDiscretization,
			XmRInt,
			sizeof(int),
			XtOffsetOf(struct Discretization,default_value),
			XmRString,
			"4"
		},
		{
			XmNmaximumElementDiscretization,
			XmCMaximumElementDiscretization,
			XmRInt,
			sizeof(int),
			XtOffsetOf(struct Discretization,maximum_value),
			XmRString,
			"50"
		}
	};

	ENTER(read_element_discretization_defaults);
	if (default_value&&maximum_value)
	{
		if (user_interface)
		{
			if (!resources_read)
			{
				/* retrieve the settings */
				XtVaGetApplicationResources(user_interface->application_shell,
					&discretization,resources,XtNumber(resources),NULL);
				resources_read=1;
			}
			*default_value=discretization.default_value;
			*maximum_value=discretization.maximum_value;
		}
		else
		{
			*default_value=4;
			*maximum_value=50;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_element_discretization_defaults.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* read_element_discretization_defaults */

int check_Element_discretization(struct Element_discretization
	*element_discretization,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 11 December 1998

DESCRIPTION :
Ensures the <element_discretization> is within the bounds of the minimum of 1
and the maximum read in from the defaults.
???DB.  Changed the lower bound to 1 because also used for elements.
???DB.  Need to make consistent.
???RC user_interface argument not checked as may not be needed in
read_Element_discretization_defaults().
==============================================================================*/
{
	int discretization_change,return_code;
	struct Element_discretization initial;
	int default_value,maximum_value;

	ENTER(check_Element_discretization);
	if (element_discretization)
	{
		if (return_code=read_element_discretization_defaults(&default_value,
			&maximum_value,user_interface))
		{
			discretization_change=0;
			initial.number_in_xi1=element_discretization->number_in_xi1;
			initial.number_in_xi2=element_discretization->number_in_xi2;
			initial.number_in_xi3=element_discretization->number_in_xi3;
			if (element_discretization->number_in_xi1 > maximum_value)
			{
				element_discretization->number_in_xi1=maximum_value;
				discretization_change=1;
			}
			if (1 > element_discretization->number_in_xi1)
			{
				element_discretization->number_in_xi1=1;
				discretization_change=1;
			}
			if (element_discretization->number_in_xi2 > maximum_value)
			{
				element_discretization->number_in_xi2=maximum_value;
				discretization_change=1;
			}
			if (1 > element_discretization->number_in_xi2)
			{
				element_discretization->number_in_xi2=1;
				discretization_change=1;
			}
			if (element_discretization->number_in_xi3 > maximum_value)
			{
				element_discretization->number_in_xi3=maximum_value;
				discretization_change=1;
			}
			if (1 > element_discretization->number_in_xi3)
			{
				element_discretization->number_in_xi3=1;
				discretization_change=1;
			}
			if (discretization_change)
			{
				display_message(WARNING_MESSAGE,
					"Element discretization values must be from 1 to %d\n"
					"%d*%d*%d changed to %d*%d*%d",maximum_value,
					initial.number_in_xi1,initial.number_in_xi2,initial.number_in_xi3,
					element_discretization->number_in_xi1,
					element_discretization->number_in_xi2,
					element_discretization->number_in_xi3);
			}
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"check_Element_discretization.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return return_code;
} /* check_Element_discretization */

int set_Element_discretization(struct Parse_state *state,
	void *element_discretization_void,void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
A modifier function for setting discretization in each element direction.
==============================================================================*/
{
	char *current_token;
	int return_code,multiple_default;
	struct Element_discretization *element_discretization;
	struct User_interface *user_interface;

	ENTER(set_Element_discretization);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				user_interface=(struct User_interface *)user_interface_void;
				if (element_discretization=
					(struct Element_discretization *)element_discretization_void)
				{
					if (1==sscanf(current_token,"%d",
						&(element_discretization->number_in_xi1)))
					{
						if (2==sscanf(current_token,"%d*%d",
							&(element_discretization->number_in_xi1),
							&(element_discretization->number_in_xi2)))
						{
							if (3!=sscanf(current_token,"%d*%d*%d",
								&(element_discretization->number_in_xi1),
								&(element_discretization->number_in_xi2),
								&(element_discretization->number_in_xi3)))
							{
								element_discretization->number_in_xi3=
									element_discretization->number_in_xi2;
							}
						}
						else
						{
							element_discretization->number_in_xi2=
								element_discretization->number_in_xi1;
							element_discretization->number_in_xi3=
								element_discretization->number_in_xi2;
						}
						shift_Parse_state(state,1);
						return_code=check_Element_discretization(element_discretization,
							user_interface);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid discretization: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Element_discretization.  Missing element_discretization");
					return_code=0;
				}
			}
			else
			{
				/* write help */
				if (element_discretization=
					(struct Element_discretization *)element_discretization_void)
				{
					multiple_default=
						(0>element_discretization->number_in_xi1)
						||(0>element_discretization->number_in_xi2)
						||(0>element_discretization->number_in_xi3);
					if (element_discretization->number_in_xi3)
					{
						if (multiple_default)
						{
							display_message(INFORMATION_MESSAGE,
								" #*#*#[CURRENT]{integer*integer*integer}");
						}
						else
						{
							display_message(INFORMATION_MESSAGE,
								" #*#*#[%d*%d*%d]{integer*integer*integer}",
								element_discretization->number_in_xi1,
								element_discretization->number_in_xi2,
								element_discretization->number_in_xi3);
						}
					}
					else
					{
						if (element_discretization->number_in_xi2)
						{
							if (multiple_default)
							{
								display_message(INFORMATION_MESSAGE,
									" #*#[CURRENT]{integer*integer}");
							}
							else
							{
								display_message(INFORMATION_MESSAGE,
									" #*#[%d*%d]{integer*integer}",
									element_discretization->number_in_xi1,
									element_discretization->number_in_xi2);
							}
						}
						else
						{
							if (multiple_default)
							{
								display_message(INFORMATION_MESSAGE," #[CURRENT]{integer}");
							}
							else
							{
								display_message(INFORMATION_MESSAGE," #[%d]{integer}",
									element_discretization->number_in_xi1);
							}
						}
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
						" #*#*#{integer*integer*integer}");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing element discretization");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Element_discretization.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Element_discretization */

char *Xi_discretization_mode_string(
	enum Xi_discretization_mode xi_discretization_mode)
/*******************************************************************************
LAST MODIFIED : 2 March 1999

DESCRIPTION :
Returns a pointer to a static string describing the <xi_discretization_mode>,
eg. XI_DISCRETIZATION_MODE_CELL_CENTRES == "cell_centres". This string should
match the command used to create that type of settings.
The returned string must not be DEALLOCATEd!
==============================================================================*/
{
	char *return_string;

	ENTER(Xi_discretization_mode_string);
	switch (xi_discretization_mode)
	{
		case XI_DISCRETIZATION_CELL_CENTRES:
		{
			return_string="cell_centres";
		} break;
		case XI_DISCRETIZATION_CELL_CORNERS:
		{
			return_string="cell_corners";
		} break;
		case XI_DISCRETIZATION_CELL_RANDOM:
		{
			return_string="cell_random";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Xi_discretization_mode_string.  Unknown xi_discretization_mode");
			return_string=(char *)NULL;
		} break;
	}
	LEAVE;

	return (return_string);
} /* Xi_discretization_mode_string */

int Xi_discretization_mode_get_number_of_xi_points(
	enum Xi_discretization_mode xi_discretization_mode,int dimension,
	int *number_in_xi)
/*******************************************************************************
LAST MODIFIED : 28 March 2000

DESCRIPTION :
Returns the number of points that should be created for <xi_discretization_mode>
in an element of the given <dimension> with <number_in_xi> cells in each
xi direction. Returns zero for invalid arguments, incl. if the number_in_xi are
less than 1 in any direction.
==============================================================================*/
{
	int i,number_of_xi_points;

	ENTER(Xi_discretization_mode_get_number_of_xi_points);
	if ((0<dimension)&&(3>=dimension)&&number_in_xi)
	{
		number_of_xi_points=1;
		for (i=0;(i<dimension)&&number_of_xi_points;i++)
		{
			if (1>number_in_xi[i])
			{
				display_message(ERROR_MESSAGE,
					"Xi_discretization_mode_get_number_of_xi_points.  "
					"non-positive number in xi");
				number_of_xi_points=0;
			}
		}
		if (0<number_of_xi_points)
		{
			switch (xi_discretization_mode)
			{
				case XI_DISCRETIZATION_CELL_CENTRES:
				case XI_DISCRETIZATION_CELL_RANDOM:
				{
					for (i=0;i<dimension;i++)
					{
						number_of_xi_points *= number_in_xi[i];
					}
				} break;
				case XI_DISCRETIZATION_CELL_CORNERS:
				{
					for (i=0;i<dimension;i++)
					{
						number_of_xi_points *= (number_in_xi[i]+1);
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Xi_discretization_mode_get_number_of_xi_points.  "
						"Unknown xi_discretization_mode");
					number_of_xi_points=0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Xi_discretization_mode_get_number_of_xi_points.  Invalid argument(s)");
		number_of_xi_points=0;
	}
	LEAVE;

	return (number_of_xi_points);
} /* Xi_discretization_mode_get_number_of_xi_points */

Triple *Xi_discretization_mode_get_xi_points(
	enum Xi_discretization_mode xi_discretization_mode,int dimension,
	int *number_in_xi,int *number_of_xi_points)
/*******************************************************************************
LAST MODIFIED : 28 March 2000

DESCRIPTION :
Allocates and returns the set of points for <xi_discretization_mode>
in an element of the given <dimension> with <number_in_xi> cells in each
xi direction. Layout of points is controlled by the <xi_discretization_mode>.
Function also returns <number_of_xi_points> calculated. Xi positions are always
returned as triples with remaining xi coordinates 0 for 1-D and 2-D cases.
Note: xi changes from 0 to 1 over each element direction.
==============================================================================*/
{
	float spread[MAXIMUM_ELEMENT_XI_DIMENSIONS],xi_j,xi_k;
	int i,j,k;
	Triple *xi_points,*xi;

	ENTER(Xi_discretization_mode_get_xi_points);
	if ((0<dimension)&&(3>=dimension)&&number_in_xi&&number_of_xi_points)
	{
		if (0<(*number_of_xi_points=Xi_discretization_mode_get_number_of_xi_points(
			xi_discretization_mode,dimension,number_in_xi)))
		{
			if (ALLOCATE(xi_points,Triple,*number_of_xi_points))
			{
				switch (xi_discretization_mode)
				{
					case XI_DISCRETIZATION_CELL_CENTRES:
					{
						xi=xi_points;
						switch (dimension)
						{
							case 1:
							{
								for (i=0;i<number_in_xi[0];i++)
								{
									(*xi)[0]=((float)i + 0.5)/(float)number_in_xi[0];
									(*xi)[1]=0.0;
									(*xi)[2]=0.0;
									xi++;
								}
							} break;
							case 2:
							{
								for (j=0;j<number_in_xi[1];j++)
								{
									xi_j=((float)j + 0.5)/(float)number_in_xi[1];
									for (i=0;i<number_in_xi[0];i++)
									{
										(*xi)[0]=((float)i + 0.5)/(float)number_in_xi[0];
										(*xi)[1]=xi_j;
										(*xi)[2]=0.0;
										xi++;
									}
								}
							} break;
							case 3:
							{
								for (k=0;k<number_in_xi[2];k++)
								{
									xi_k=((float)k + 0.5)/(float)number_in_xi[2];
									for (j=0;j<number_in_xi[1];j++)
									{
										xi_j=((float)j + 0.5)/(float)number_in_xi[1];
										for (i=0;i<number_in_xi[0];i++)
										{
											(*xi)[0]=((float)i + 0.5)/(float)number_in_xi[0];
											(*xi)[1]=xi_j;
											(*xi)[2]=xi_k;
											xi++;
										}
									}
								}
							} break;
						}
					} break;
					case XI_DISCRETIZATION_CELL_CORNERS:
					{
						xi=xi_points;
						switch (dimension)
						{
							case 1:
							{
								for (i=0;i<=number_in_xi[0];i++)
								{
									(*xi)[0]=(float)i/(float)number_in_xi[0];
									(*xi)[1]=0.0;
									(*xi)[2]=0.0;
									xi++;
								}
							} break;
							case 2:
							{
								for (j=0;j<=number_in_xi[1];j++)
								{
									xi_j=(float)j/(float)number_in_xi[1];
									for (i=0;i<=number_in_xi[0];i++)
									{
										(*xi)[0]=(float)i/(float)number_in_xi[0];
										(*xi)[1]=xi_j;
										(*xi)[2]=0.0;
										xi++;
									}
								}
							} break;
							case 3:
							{
								for (k=0;k<=number_in_xi[2];k++)
								{
									xi_k=(float)k/(float)number_in_xi[2];
									for (j=0;j<=number_in_xi[1];j++)
									{
										xi_j=(float)j/(float)number_in_xi[1];
										for (i=0;i<=number_in_xi[0];i++)
										{
											(*xi)[0]=(float)i/(float)number_in_xi[0];
											(*xi)[1]=xi_j;
											(*xi)[2]=xi_k;
											xi++;
										}
									}
								}
							} break;
						}
					} break;
					case XI_DISCRETIZATION_CELL_RANDOM:
					{
						xi=xi_points;
						for (i=0;i<dimension;i++)
						{
							spread[i] = 1.0 / (float)number_in_xi[i];
						}
						switch (dimension)
						{
							case 1:
							{
								for (i=0;i<number_in_xi[0];i++)
								{
									(*xi)[0]=(float)i/(float)number_in_xi[0] +
										(spread[0]*((float)(random()&0xFFFF))/65536.0);
									(*xi)[1]=0.0;
									(*xi)[2]=0.0;
									xi++;
								}
							} break;
							case 2:
							{
								for (j=0;j<number_in_xi[1];j++)
								{
									for (i=0;i<number_in_xi[0];i++)
									{
										(*xi)[0]=(float)i/(float)number_in_xi[0] +
											(spread[0]*((float)(random()&0xFFFF))/65536.0);
										(*xi)[1]=(float)j/(float)number_in_xi[1] +
											(spread[1]*((float)(random()&0xFFFF))/65536.0);
										(*xi)[2]=0.0;
										xi++;
									}
								}
							} break;
							case 3:
							{
								for (k=0;k<number_in_xi[2];k++)
								{
									for (j=0;j<number_in_xi[1];j++)
									{
										for (i=0;i<number_in_xi[0];i++)
										{
											(*xi)[0]=(float)i/(float)number_in_xi[0] +
												(spread[0]*((float)(random()&0xFFFF))/65536.0);
											(*xi)[1]=(float)j/(float)number_in_xi[1] +
												(spread[1]*((float)(random()&0xFFFF))/65536.0);
											(*xi)[2]=(float)k/(float)number_in_xi[2] +
												(spread[2]*((float)(random()&0xFFFF))/65536.0);
											xi++;
										}
									}
								}
							} break;
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Xi_discretization_mode_get_xi_points.  "
							"Unknown xi_discretization_mode");
						DEALLOCATE(xi_points);
						*number_of_xi_points=0;
					} break;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Xi_discretization_mode_get_xi_points.  Not enough memory");
				*number_of_xi_points=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Xi_discretization_mode_get_xi_points.  Invalid number of xi points");
			xi_points=(Triple *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Xi_discretization_mode_get_xi_points.  Invalid argument(s)");
		xi_points=(Triple *)NULL;
	}
	LEAVE;

	return (xi_points);
} /* Xi_discretization_mode_get_xi_points */

char **Xi_discretization_mode_get_valid_strings(int *number_of_valid_strings)
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Returns and allocated array of pointers to all static strings for valid
Xi_discretization_modes - obtained from function Xi_discretization_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/
{
	char **valid_strings;
	enum Xi_discretization_mode xi_discretization_mode;
	int i;

	ENTER(Xi_discretization_mode_get_valid_strings);
	if (number_of_valid_strings)
	{
		*number_of_valid_strings=0;
		xi_discretization_mode=XI_DISCRETIZATION_MODE_BEFORE_FIRST;
		xi_discretization_mode++;
		while (xi_discretization_mode<XI_DISCRETIZATION_MODE_AFTER_LAST)
		{
			(*number_of_valid_strings)++;
			xi_discretization_mode++;
		}
		if (ALLOCATE(valid_strings,char *,*number_of_valid_strings))
		{
			xi_discretization_mode=XI_DISCRETIZATION_MODE_BEFORE_FIRST;
			xi_discretization_mode++;
			i=0;
			while (xi_discretization_mode<XI_DISCRETIZATION_MODE_AFTER_LAST)
			{
				valid_strings[i]=Xi_discretization_mode_string(xi_discretization_mode);
				i++;
				xi_discretization_mode++;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Xi_discretization_mode_get_valid_strings.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Xi_discretization_mode_get_valid_strings.  Invalid argument");
		valid_strings=(char **)NULL;
	}
	LEAVE;

	return (valid_strings);
} /* Xi_discretization_mode_get_valid_strings */

enum Xi_discretization_mode Xi_discretization_mode_from_string(
	char *xi_discretization_mode_string)
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Returns the <Xi_discretization_mode> described by
<xi_discretization_mode_string>.
==============================================================================*/
{
	enum Xi_discretization_mode xi_discretization_mode;

	ENTER(Xi_discretization_mode_from_string);
	if (xi_discretization_mode_string)
	{
		xi_discretization_mode=XI_DISCRETIZATION_MODE_BEFORE_FIRST;
		xi_discretization_mode++;
		while ((xi_discretization_mode<XI_DISCRETIZATION_MODE_AFTER_LAST)&&
			(!fuzzy_string_compare_same_length(xi_discretization_mode_string,
				Xi_discretization_mode_string(xi_discretization_mode))))
		{
			xi_discretization_mode++;
		}
		if (XI_DISCRETIZATION_MODE_AFTER_LAST==xi_discretization_mode)
		{
			xi_discretization_mode=XI_DISCRETIZATION_INVALID;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Xi_discretization_mode_from_string.  Invalid argument");
		xi_discretization_mode=XI_DISCRETIZATION_INVALID;
	}
	LEAVE;

	return (xi_discretization_mode);
} /* Xi_discretization_mode_from_string */

char *Streamline_type_string(enum Streamline_type streamline_type)
/*******************************************************************************
LAST MODIFIED : 19 March 1999

DESCRIPTION :
Returns a pointer to a static string describing the streamline_type, eg.
STREAM_LINE == "line". This string should match the command used
to create that type of streamline. The returned string must not be DEALLOCATEd!
==============================================================================*/
{
	char *return_string;

	ENTER(Streamline_type_string);
	switch (streamline_type)
	{
		case STREAM_EXTRUDED_ELLIPSE:
		{
			return_string="ellipse";
		} break;
		case STREAM_LINE:
		{
			return_string="line";
		} break;
		case STREAM_EXTRUDED_RECTANGLE:
		{
			return_string="rectangle";
		} break;
		case STREAM_RIBBON:
		{
			return_string="ribbon";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Streamline_type_string.  Unknown streamline_type");
			return_string=(char *)NULL;
		} break;
	}
	LEAVE;

	return (return_string);
} /* Streamline_type_string */

char **Streamline_type_get_valid_strings(int *number_of_valid_strings)
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Returns and allocated array of pointers to all static strings for valid
Streamline_types - obtained from function Streamline_type_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/
{
	char **valid_strings;
	enum Streamline_type streamline_type;
	int i;

	ENTER(Streamline_type_get_valid_strings);
	if (number_of_valid_strings)
	{
		*number_of_valid_strings=0;
		streamline_type=STREAMLINE_TYPE_BEFORE_FIRST;
		streamline_type++;
		while (streamline_type<STREAMLINE_TYPE_AFTER_LAST)
		{
			(*number_of_valid_strings)++;
			streamline_type++;
		}
		if (ALLOCATE(valid_strings,char *,*number_of_valid_strings))
		{
			streamline_type=STREAMLINE_TYPE_BEFORE_FIRST;
			streamline_type++;
			i=0;
			while (streamline_type<STREAMLINE_TYPE_AFTER_LAST)
			{
				valid_strings[i]=Streamline_type_string(streamline_type);
				i++;
				streamline_type++;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Streamline_type_get_valid_strings.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Streamline_type_get_valid_strings.  Invalid argument");
		valid_strings=(char **)NULL;
	}
	LEAVE;

	return (valid_strings);
} /* Streamline_type_get_valid_strings */

enum Streamline_type Streamline_type_from_string(char *streamline_type_string)
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Returns the <Streamline_type> described by <streamline_type_string>.
==============================================================================*/
{
	enum Streamline_type streamline_type;

	ENTER(Streamline_type_from_string);
	if (streamline_type_string)
	{
		streamline_type=STREAMLINE_TYPE_BEFORE_FIRST;
		streamline_type++;
		while ((streamline_type<STREAMLINE_TYPE_AFTER_LAST)&&
			(!fuzzy_string_compare_same_length(streamline_type_string,
				Streamline_type_string(streamline_type))))
		{
			streamline_type++;
		}
		if (STREAMLINE_TYPE_AFTER_LAST==streamline_type)
		{
			streamline_type=STREAMLINE_TYPE_INVALID;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Streamline_type_from_string.  Invalid argument");
		streamline_type=STREAMLINE_TYPE_INVALID;
	}
	LEAVE;

	return (streamline_type);
} /* Streamline_type_from_string */

char *Streamline_data_type_string(
	enum Streamline_data_type streamline_data_type)
/*******************************************************************************
LAST MODIFIED : 24 March 1999

DESCRIPTION :
Returns a pointer to a static string describing the streamline_data_type, eg.
STREAM_FIELD_SCALAR == "field_scalar". This string should match the command used
to create that type of streamline. The returned string must not be DEALLOCATEd!
==============================================================================*/
{
	char *return_string;

	ENTER(Streamline_data_type_string);
	switch (streamline_data_type)
	{
		case STREAM_NO_DATA:
		{
			return_string="no_data";
		} break;
		case STREAM_FIELD_SCALAR:
		{
			return_string="field_scalar";
		} break;
		case STREAM_MAGNITUDE_SCALAR:
		{
			return_string="magnitude_scalar";
		} break;
		case STREAM_TRAVEL_SCALAR:
		{
			return_string="travel_scalar";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Streamline_data_type_string.  Unknown streamline_data_type");
			return_string=(char *)NULL;
		} break;
	}
	LEAVE;

	return (return_string);
} /* Streamline_data_type_string */

char **Streamline_data_type_get_valid_strings(int *number_of_valid_strings)
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Returns and allocated array of pointers to all static strings for valid
Streamline_data_types - obtained from function Streamline_data_type_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/
{
	char **valid_strings;
	enum Streamline_data_type streamline_data_type;
	int i;

	ENTER(Streamline_data_type_get_valid_strings);
	if (number_of_valid_strings)
	{
		*number_of_valid_strings=0;
		streamline_data_type=STREAMLINE_DATA_TYPE_BEFORE_FIRST;
		streamline_data_type++;
		while (streamline_data_type<STREAMLINE_DATA_TYPE_AFTER_LAST)
		{
			(*number_of_valid_strings)++;
			streamline_data_type++;
		}
		if (ALLOCATE(valid_strings,char *,*number_of_valid_strings))
		{
			streamline_data_type=STREAMLINE_DATA_TYPE_BEFORE_FIRST;
			streamline_data_type++;
			i=0;
			while (streamline_data_type<STREAMLINE_DATA_TYPE_AFTER_LAST)
			{
				valid_strings[i]=Streamline_data_type_string(streamline_data_type);
				i++;
				streamline_data_type++;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Streamline_data_type_get_valid_strings.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Streamline_data_type_get_valid_strings.  Invalid argument");
		valid_strings=(char **)NULL;
	}
	LEAVE;

	return (valid_strings);
} /* Streamline_data_type_get_valid_strings */

enum Streamline_data_type Streamline_data_type_from_string(
	char *streamline_data_type_string)
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Returns the <Streamline_data_type> described by <streamline_data_type_string>,
or NULL if not recognized.
==============================================================================*/
{
	enum Streamline_data_type streamline_data_type;

	ENTER(Streamline_data_type_from_string);
	if (streamline_data_type_string)
	{
		streamline_data_type=STREAMLINE_DATA_TYPE_BEFORE_FIRST;
		streamline_data_type++;
		while ((streamline_data_type<STREAMLINE_DATA_TYPE_AFTER_LAST)&&
			(!fuzzy_string_compare_same_length(streamline_data_type_string,
				Streamline_data_type_string(streamline_data_type))))
		{
			streamline_data_type++;
		}
		if (STREAMLINE_DATA_TYPE_AFTER_LAST==streamline_data_type)
		{
			streamline_data_type=STREAMLINE_DATA_TYPE_INVALID;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Streamline_data_type_from_string.  Invalid argument");
		streamline_data_type=STREAMLINE_DATA_TYPE_INVALID;
	}
	LEAVE;

	return (streamline_data_type);
} /* Streamline_data_type_from_string */

