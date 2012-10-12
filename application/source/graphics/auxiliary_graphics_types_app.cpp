


#include "command/parser.h"

#include "general/enumerator_private.h"

#include "general/message.h"
#include "general/mystring.h"
#include "general/enumerator_private.hpp"
#include "general/message.h"
#include "general/debug.h"
#include "user_interface/user_interface.h"
#include "graphics/auxiliary_graphics_types.h"

int set_exterior(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 8 June 2004

DESCRIPTION :
A modifier function for setting exterior flag and face number.
==============================================================================*/
{
	const char *current_token;
	int return_code,*value_address;

	ENTER(set_exterior);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				value_address=(int *)value_address_void;
				if (value_address)
				{
					return_code=1;
					/*???DB.  Only valid for cubes (not polygons) */
					if (fuzzy_string_compare(current_token,"XI1_0"))
					{
						*value_address=0;
						shift_Parse_state(state,1);
					}
					else
					{
						if (fuzzy_string_compare(current_token,"XI1_1"))
						{
							*value_address=1;
							shift_Parse_state(state,1);
						}
						else
						{
							if (fuzzy_string_compare(current_token,"XI2_0"))
							{
								*value_address=2;
								shift_Parse_state(state,1);
							}
							else
							{
								if (fuzzy_string_compare(current_token,"XI2_1"))
								{
									*value_address=3;
									shift_Parse_state(state,1);
								}
								else
								{
									if (fuzzy_string_compare(current_token,"XI3_0"))
									{
										*value_address=4;
										shift_Parse_state(state,1);
									}
									else
									{
										if (fuzzy_string_compare(current_token,"XI3_1"))
										{
											*value_address=5;
											shift_Parse_state(state,1);
										}
										else
										{
											*value_address=-1;
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
			value_address=(int *)value_address_void;
			if (value_address)
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


int set_Circle_discretization(struct Parse_state *state,
	void *circle_discretization_void,void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
A modifier function for setting number of segments used to draw circles.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	int *circle_discretization;

	USE_PARAMETER(user_interface_void);
	ENTER(set_Circle_discretization);
	if (state)
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				circle_discretization = (int *)circle_discretization_void;
				if (circle_discretization)
				{
					if (1==sscanf(current_token,"%d",circle_discretization))
					{
						shift_Parse_state(state,1);
						return_code=check_Circle_discretization(circle_discretization);
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
				circle_discretization = (int *)circle_discretization_void;
				if (circle_discretization)
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



int set_Element_discretization(struct Parse_state *state,
	void *element_discretization_void,void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
A modifier function for setting discretization in each element direction.
==============================================================================*/
{
	const char *current_token;
	int return_code,multiple_default;
	struct Element_discretization *element_discretization;

	USE_PARAMETER(user_interface_void);
	ENTER(set_Element_discretization);
	if (state)
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				element_discretization=
					(struct Element_discretization *)element_discretization_void;
				if (element_discretization)
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
						return_code=check_Element_discretization(element_discretization);
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
				element_discretization=
					(struct Element_discretization *)element_discretization_void;
				if (element_discretization)
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
