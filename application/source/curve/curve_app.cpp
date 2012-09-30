
#include <stdio.h>

#include "general/debug.h"
#include "general/message.h"
#include "general/enumerator_private.hpp"
#include "general/mystring.h"
#include "command/parser.h"
#include "curve/curve.h"

//DEFINE_DEFAULT_ENUMERATOR_PREFIX_INCREMENT_OPERATOR(Curve_extend_mode)
//DEFINE_DEFAULT_ENUMERATOR_POSTFIX_INCREMENT_OPERATOR(Curve_extend_mode)

int define_Curve_information(struct Parse_state *state,
	void *curve_definition_void,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
==============================================================================*/
{
	const char *extend_mode_string,*file_name,**valid_strings;
	enum Curve_extend_mode extend_mode;
	enum FE_basis_type new_fe_basis_type;
	FE_value *max_value,*min_value,parameter_grid,value_grid;
	int comp_no,existing_number_of_components,new_number_of_components = 0,
		number_of_components,number_of_valid_strings,return_code;
	struct Curve *temp_curve;
	struct Curve_definition *curve_definition;
	struct Option_table *option_table;

	ENTER(define_Curve_information);
	USE_PARAMETER(dummy_void);
	if (state&&
		(curve_definition=(struct Curve_definition *)curve_definition_void))
	{
		file_name=(char *)NULL;
		min_value=(FE_value *)NULL;
		max_value=(FE_value *)NULL;
		number_of_components=curve_definition->number_of_components;
		if (ALLOCATE(min_value,FE_value,number_of_components)&&
			ALLOCATE(max_value,FE_value,number_of_components))
		{
			curve_definition->curve = CREATE(Curve)(curve_definition->name,
				curve_definition->fe_basis_type,curve_definition->number_of_components);
			if (NULL != curve_definition->curve)
			{
				ACCESS(Curve)(curve_definition->curve);
				if (curve_definition->curve_to_be_modified)
				{
					existing_number_of_components=Curve_get_number_of_components(
						curve_definition->curve_to_be_modified);
					for (comp_no=0;comp_no<number_of_components;comp_no++)
					{
						if (comp_no < existing_number_of_components)
						{
							Curve_get_edit_component_range(
								curve_definition->curve_to_be_modified,comp_no,
								&(min_value[comp_no]),&(max_value[comp_no]));
						}
						else
						{
							Curve_get_edit_component_range(curve_definition->curve,
								comp_no,&(min_value[comp_no]),&(max_value[comp_no]));
						}
					}
					Curve_get_parameter_grid(
						curve_definition->curve_to_be_modified,&parameter_grid);
					Curve_get_value_grid(
						curve_definition->curve_to_be_modified,&value_grid);
					extend_mode=Curve_get_extend_mode(
						curve_definition->curve_to_be_modified);
				}
				else
				{
					for (comp_no=0;comp_no<number_of_components;comp_no++)
					{
						Curve_get_edit_component_range(curve_definition->curve,
							comp_no,&(min_value[comp_no]),&(max_value[comp_no]));
					}
					Curve_get_parameter_grid(curve_definition->curve,
						&parameter_grid);
					Curve_get_value_grid(curve_definition->curve,&value_grid);
					extend_mode=Curve_get_extend_mode(curve_definition->curve);
				}

				option_table=CREATE(Option_table)();
				/* extend mode */
				extend_mode_string=Curve_extend_mode_string(extend_mode);
				valid_strings=Curve_extend_mode_get_valid_strings(
					&number_of_valid_strings);
				Option_table_add_enumerator(option_table,number_of_valid_strings,
					valid_strings,&extend_mode_string);
				DEALLOCATE(valid_strings);
				/* file */
				Option_table_add_entry(option_table,"file",&file_name,
					(void *)1,set_name);
				/* max_value */
				Option_table_add_FE_value_vector_entry(option_table, "max_value",
					max_value, &number_of_components);
				/* min_value */
				Option_table_add_FE_value_vector_entry(option_table, "min_value",
					min_value, &number_of_components);
				/* parameter_grid */
				Option_table_add_non_negative_double_entry(option_table,
					"parameter_grid", &parameter_grid);
				/* value_grid */
				Option_table_add_non_negative_double_entry(option_table,
					"value_grid", &value_grid);
				return_code = Option_table_multi_parse(option_table, state);
				if (return_code)
				{
					if (file_name)
					{
						temp_curve = create_Curve_from_file(
							curve_definition->curve->name, file_name,
							curve_definition->io_stream_package);
						if (NULL != temp_curve)
						{
							if (curve_definition->fe_basis_type_set)
							{
								new_fe_basis_type=curve_definition->fe_basis_type;
							}
							else
							{
								new_fe_basis_type=Curve_get_fe_basis_type(temp_curve);
							}
							if (curve_definition->number_of_components_set)
							{
								new_number_of_components=curve_definition->number_of_components;
							}
							else
							{
								new_number_of_components=
									Curve_get_number_of_components(temp_curve);
							}
							if ((Curve_get_number_of_components(temp_curve)==
								new_number_of_components)&&(new_fe_basis_type ==
								(Curve_get_fe_basis_type(temp_curve))))
							{
								/* use this instead of curve created earlier */
								REACCESS(Curve)(&(curve_definition->curve),temp_curve);
							}
							else
							{
								if (!(return_code=cc_copy_convert_without_name(
									curve_definition->curve,new_fe_basis_type,
									new_number_of_components,temp_curve)))
								{
									display_message(ERROR_MESSAGE,
										"define_Curve_information.  "
										"Could not copy curve from file %s",file_name);
									DEACCESS(Curve)(&(curve_definition->curve));
									return_code=0;
								}
								DESTROY(Curve)(&temp_curve);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"define_Curve_information.  "
								"Could not read curve from file %s",file_name);
							DEACCESS(Curve)(&(curve_definition->curve));
							return_code=0;
						}
					}
					else if (curve_definition->curve_to_be_modified)
					{
						if (!(return_code=cc_copy_convert_without_name(
							curve_definition->curve,curve_definition->fe_basis_type,
							number_of_components,curve_definition->curve_to_be_modified)))
						{
							display_message(ERROR_MESSAGE,
								"define_Curve_information.  Could not copy curve");
						}
						new_number_of_components=number_of_components;
					}
					if (return_code)
					{
						for (comp_no=0;comp_no<number_of_components;comp_no++)
						{
							if (comp_no<new_number_of_components)
							{
								Curve_set_edit_component_range(curve_definition->curve,
									comp_no,min_value[comp_no],max_value[comp_no]);
							}
						}
						Curve_set_parameter_grid(curve_definition->curve,
							parameter_grid);
						Curve_set_value_grid(curve_definition->curve,
							value_grid);
						Curve_set_extend_mode(curve_definition->curve,
							Curve_extend_mode_from_string(extend_mode_string));
					}
				}
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Curve_information.  Could not create curve");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Curve_information.  Not enough memory");
			return_code=0;
		}
		if (min_value)
		{
			DEALLOCATE(min_value);
		}
		if (max_value)
		{
			DEALLOCATE(max_value);
		}
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Curve_information.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Curve_information */

int define_Curve_number_of_components(struct Parse_state *state,
	void *curve_definition_void,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 29 November 1999

DESCRIPTION :
==============================================================================*/
{
	const char *current_token;
	int return_code = 0;
	struct Curve_definition *curve_definition;
	struct Modifier_entry
		help_option_table[]=
		{
			{"<number_of_components #>",NULL,NULL,define_Curve_information},
			{NULL,NULL,NULL,NULL}
		};

	ENTER(define_Curve_number_of_components);
	USE_PARAMETER(dummy_void);
	if (state&&
		(curve_definition=(struct Curve_definition *)curve_definition_void))
	{
		current_token = state->current_token;
		if (NULL != current_token)
		{
			/* read the optional number_of_components parameter */
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (fuzzy_string_compare(current_token,"number_of_components"))
				{
					shift_Parse_state(state,1);
					current_token = state->current_token;
					if (NULL != current_token)
					{
						if ((1==sscanf(current_token," %d ",
							&(curve_definition->number_of_components)))&&
							(0<curve_definition->number_of_components))
						{
							curve_definition->number_of_components_set=1;
							shift_Parse_state(state,1);
							return_code=1;
						}
						else
						{
							return_code=0;
						}
					}
				}
				if (return_code)
				{
					return_code=define_Curve_information(state,
						curve_definition_void,(void *)NULL);
				}
			}
			else
			{
				/* write help */
				(help_option_table[0]).to_be_modified=curve_definition_void;
				return_code=process_option(state,help_option_table);
			}
		}
		else
		{
			return_code=define_Curve_information(state,
				curve_definition_void,(void *)NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Curve_number_of_components.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Curve_number_of_components */

int define_Curve_fe_basis_type(struct Parse_state *state,
	void *curve_definition_void,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 29 November 1999

DESCRIPTION :
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Curve_definition *curve_definition;
	struct Modifier_entry
		help_option_table[]=
		{
			{"<c.Hermite|c.Lagrange|l.Lagrange|q.Lagrange>",NULL,NULL,
			 define_Curve_number_of_components},
			{NULL,NULL,NULL,NULL}
		};

	ENTER(define_Curve_fe_basis_type);
	USE_PARAMETER(dummy_void);
	if (state&&
		(curve_definition=(struct Curve_definition *)curve_definition_void))
	{
		current_token = state->current_token;
		if (NULL != current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
#if defined (OLD_CODE)
				/*???RC Currently can't handle these optional tokens that also know if
					set, hence leave with old parsing stuff for now */
				/* basis type */
				fe_basis_type_string=
					FE_basis_type_string(curve_definition->fe_basis_type);
				valid_strings=Curve_FE_basis_type_get_valid_strings(
					&number_of_valid_strings);
				Option_table_add_enumerator(option_table,number_of_valid_strings,
					valid_strings,&fe_basis_type_string);
				DEALLOCATE(valid_strings);
#endif /* defined (OLD_CODE) */
				/* read the optional fe_basis_type parameter */
				if (fuzzy_string_compare(current_token,"c.Hermite"))
				{
					curve_definition->fe_basis_type=CUBIC_HERMITE;
					curve_definition->fe_basis_type_set=1;
					shift_Parse_state(state,1);
				}
				else if (fuzzy_string_compare(current_token,"c.Lagrange"))
				{
					curve_definition->fe_basis_type=CUBIC_LAGRANGE;
					curve_definition->fe_basis_type_set=1;
					shift_Parse_state(state,1);
				}
				else if (fuzzy_string_compare(current_token,"l.Lagrange"))
				{
					curve_definition->fe_basis_type=LINEAR_LAGRANGE;
					curve_definition->fe_basis_type_set=1;
					shift_Parse_state(state,1);
				}
				else if (fuzzy_string_compare(current_token,"q.Lagrange"))
				{
					curve_definition->fe_basis_type=QUADRATIC_LAGRANGE;
					curve_definition->fe_basis_type_set=1;
					shift_Parse_state(state,1);
				}
				return_code=define_Curve_number_of_components(state,
					curve_definition_void,(void *)NULL);
			}
			else
			{
				/* write help */
				(help_option_table[0]).to_be_modified=curve_definition_void;
				return_code=process_option(state,help_option_table);
			}
		}
		else
		{
			return_code=define_Curve_information(state,
				curve_definition_void,(void *)NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Curve_fe_basis_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Curve_fe_basis_type */

int gfx_define_Curve(struct Parse_state *state,
	void *dummy_to_be_modified,void *curve_command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 November 1999

DESCRIPTION :
==============================================================================*/
{
	struct Modifier_entry
		help_option_table[]=
		{
			{"CURVE_NAME",NULL,NULL,define_Curve_fe_basis_type},
			{NULL,NULL,NULL,NULL}
		};
	const char *current_token;
	int return_code;
	struct Curve_definition curve_definition;
	struct Curve_command_data *command_data;

	ENTER(gfx_define_Curve);
	USE_PARAMETER(dummy_to_be_modified);
	return_code=0;
	if (state)
	{
		command_data = (struct Curve_command_data *)curve_command_data_void;
		if (NULL != command_data)
		{
			current_token = state->current_token;
			if (NULL != current_token)
			{
				return_code=1;
				curve_definition.name=(char *)NULL;
				curve_definition.fe_basis_type=LINEAR_LAGRANGE;
				curve_definition.fe_basis_type_set=0;
				curve_definition.number_of_components=1;
				curve_definition.number_of_components_set=0;
				curve_definition.curve=(struct Curve *)NULL;
				curve_definition.curve_to_be_modified=(struct Curve *)NULL;
				curve_definition.io_stream_package=command_data->io_stream_package;
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					curve_definition.name=duplicate_string(current_token);
					curve_definition.curve_to_be_modified =
						FIND_BY_IDENTIFIER_IN_MANAGER(Curve,name)(
							curve_definition.name, command_data->curve_manager);
					if (NULL != curve_definition.curve_to_be_modified)
					{
						curve_definition.fe_basis_type=
							Curve_get_fe_basis_type(
								curve_definition.curve_to_be_modified);
						curve_definition.number_of_components=
							Curve_get_number_of_components(
								curve_definition.curve_to_be_modified);
					}
					shift_Parse_state(state,1);
					if (define_Curve_fe_basis_type(state,
						(void *)&curve_definition,(void *)NULL))
					{
						if (curve_definition.curve_to_be_modified)
						{
							return_code=MANAGER_MODIFY_NOT_IDENTIFIER(Curve,name)(
								curve_definition.curve_to_be_modified,curve_definition.curve,
								command_data->curve_manager);
						}
						else
						{
							return_code=ADD_OBJECT_TO_MANAGER(Curve)(
								curve_definition.curve,command_data->curve_manager);
						}
					}
					else
					{
						return_code=0;
					}
				}
				else
				{
					curve_definition.name=duplicate_string("CURVE_NAME");
					(help_option_table[0]).to_be_modified=(void *)&curve_definition;
					return_code=process_option(state,help_option_table);
				}
				if (curve_definition.name)
				{
					DEALLOCATE(curve_definition.name);
				}
				if (curve_definition.curve)
				{
					DEACCESS(Curve)(&(curve_definition.curve));
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing curve name");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_define_Curve.  Missing curve_command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_define_Curve.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_define_Curve */

int gfx_destroy_Curve(struct Parse_state *state,
	void *dummy_to_be_modified,void *curve_manager_void)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Executes a GFX DESTROY CURVE command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct MANAGER(Curve) *curve_manager;
	struct Curve *curve;

	ENTER(gfx_destroy_Curve);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		curve_manager = (struct MANAGER(Curve) *)curve_manager_void;
		if (NULL != curve_manager)
		{
			current_token = state->current_token;
			if (NULL != current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					curve = FIND_BY_IDENTIFIER_IN_MANAGER(Curve,name)(
						current_token,curve_manager);
					if (NULL != curve)
					{
						return_code=REMOVE_OBJECT_FROM_MANAGER(Curve)(curve,
							curve_manager);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown curve: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," CURVE_NAME");
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing curve name");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_Curve.  Missing curve_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_Curve.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_Curve */


int set_Curve(struct Parse_state *state,void *curve_address_void,
	void *curve_manager_void)
/*******************************************************************************
LAST MODIFIED : 5 November 1999

DESCRIPTION :
Modifier function to set the curve from a command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct MANAGER(Curve) *curve_manager;
	struct Curve *temp_curve,**curve_address;

	ENTER(set_Curve);
	if (state)
	{
		current_token = state->current_token;
		if (NULL != current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((curve_address=(struct Curve **)curve_address_void)&&
					(curve_manager=
						(struct MANAGER(Curve) *)curve_manager_void))
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*curve_address)
						{
							DEACCESS(Curve)(curve_address);
							*curve_address=(struct Curve *)NULL;
						}
						return_code=1;
					}
					else
					{
						temp_curve = FIND_BY_IDENTIFIER_IN_MANAGER(Curve,name)(
							current_token, curve_manager);
						if (NULL != temp_curve)
						{
							if (*curve_address!=temp_curve)
							{
								DEACCESS(Curve)(curve_address);
								*curve_address=ACCESS(Curve)(temp_curve);
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_Curve.  Curve '%s' does not exist",
								current_token);
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Curve.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," CURVE_NAME|none");
				curve_address = (struct Curve **)curve_address_void;
				if (NULL != curve_address)
				{
					temp_curve= *curve_address;
					if (NULL != temp_curve)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",temp_curve->name);
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
			display_message(ERROR_MESSAGE,"Missing curve name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Curve.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Curve */

int gfx_list_Curve(struct Parse_state *state,
	void *dummy_to_be_modified,void *curve_manager_void)
/*******************************************************************************
LAST MODIFIED : 28 November 2000

DESCRIPTION :
Executes a GFX LIST CURVE.
==============================================================================*/
{
	int return_code;
	struct MANAGER(Curve) *curve_manager;
	struct Curve *curve;
	struct Option_table *option_table;

	ENTER(gfx_list_Curve);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (curve_manager=
		(struct MANAGER(Curve) *)curve_manager_void))
	{
		curve = (struct Curve *)NULL;
		option_table=CREATE(Option_table)();
		/* default option: curve name */
		Option_table_add_entry(option_table, (char *)NULL, &curve,
			curve_manager_void, set_Curve);
		return_code = Option_table_multi_parse(option_table,state);
		if (return_code)
		{
			if (curve)
			{
				return_code = list_Curve(curve, (void *)NULL);
			}
			else
			{
				display_message(INFORMATION_MESSAGE,"Control curves:\n");
				return_code = FOR_EACH_OBJECT_IN_MANAGER(Curve)(
					list_Curve, (void *)NULL, curve_manager);
			}
		}
		DESTROY(Option_table)(&option_table);
		if (curve)
		{
			DEACCESS(Curve)(&curve);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_Curve.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_Curve */


