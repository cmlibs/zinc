
#include <stdio.h>

#include "general/message.h"
#include "general/mystring.h"
#include "general/indexed_list_private.h"
#include "general/manager.h"
#include "command/parser.h"
#include "user_interface/process_list_or_write_command.hpp"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
// insert app headers here
#include "general/geometry_app.h"
#include "computed_field/computed_field_app.h"

struct Computed_field_type_data
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Stores information defining a type of computed field so that it can be
accessed by the rest of the program.
==============================================================================*/
{
	const char *name;
	Define_Computed_field_type_function define_Computed_field_type_function;
	Computed_field_type_package *define_type_user_data;
	int access_count;
};

PROTOTYPE_OBJECT_FUNCTIONS(Computed_field_type_data);

DECLARE_LIST_TYPES(Computed_field_type_data);
PROTOTYPE_LIST_FUNCTIONS(Computed_field_type_data);

struct Computed_field_package
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Contains data for gfx define field commands.
Also contains the computed_field_manger from the root region; this will be
removed once code using it has been converted to get the field manager directly
from the appropriate Cmiss_region.
==============================================================================*/
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct LIST(Computed_field_type_data) *computed_field_type_list;
	Computed_field_simple_package *simple_package;
}; /* struct Computed_field_package */

struct Computed_field_package *CREATE(Computed_field_package)(
	struct MANAGER(Computed_field) *computed_field_manager)
/*******************************************************************************
LAST MODIFIED : 20 May 2008

DESCRIPTION :
Creates a Computed_field_package which is used by the rest of the program to
access everything to do with computed fields.
The root_region's computed_field_manager is passed in to support old code that
expects it to be in the package. This is temporary until all code gets the true
manager from the respective Cmiss_region.
==============================================================================*/
{
	struct Computed_field_package *computed_field_package = NULL;

	ENTER(CREATE(Computed_field_package));
	if (computed_field_manager)
	{
		if (ALLOCATE(computed_field_package,struct Computed_field_package,1))
		{
			computed_field_package->computed_field_manager=computed_field_manager;
			computed_field_package->computed_field_type_list =
				CREATE(LIST(Computed_field_type_data))();
			computed_field_package->simple_package =
				new Computed_field_simple_package();
			computed_field_package->simple_package->addref();
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Computed_field_package).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Computed_field_package).  Invalid argument(s)");
	}
	LEAVE;

	return (computed_field_package);
} /* CREATE(Computed_field_package) */

int DESTROY(Computed_field_package)(
	struct Computed_field_package **package_address)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Frees memory/deaccess objects in computed_field_package at <*package_address>.
Cancels any further messages from managers.
==============================================================================*/
{
	int return_code = 0;
	struct Computed_field_package *computed_field_package;

	ENTER(DESTROY(Computed_field_package));
	if (package_address&&(computed_field_package= *package_address))
	{
		/* not destroying field manager as not owned by package */
		computed_field_package->simple_package->removeref();
		DEALLOCATE(*package_address);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Computed_field_package).  Missing field");
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Computed_field_package) */

struct MANAGER(Computed_field)
	*Computed_field_package_get_computed_field_manager(
		struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Extracts the computed_field_manager from the computed_field_package. Note that
the rest of the program should use this sparingly - it is really only here to
allow interfacing to the choose_object widgets.
==============================================================================*/
{
	struct MANAGER(Computed_field) *computed_field_manager

	ENTER(Computed_field_package_get_computed_field_manager);
	if (computed_field_package)
	{
		computed_field_manager=computed_field_package->computed_field_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_package_get_computed_field_manager.  "
			"Missing computed_field_package");
		computed_field_manager=(struct MANAGER(Computed_field) *)NULL;
	}
	LEAVE;

	return (computed_field_manager);
} /* Computed_field_package_get_computed_field_manager */

Computed_field_simple_package *Computed_field_package_get_simple_package(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 24 January 2007

DESCRIPTION :
Returns a pointer to a sharable simple type package which just contains a
function to access the Computed_field_package.
==============================================================================*/
{
	Computed_field_simple_package* return_package;

	ENTER(Computed_field_package_get_simple_package);
	if (computed_field_package)
	{
		return_package = computed_field_package->simple_package;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_package_get_simple_package.  Invalid arguments");
		return_package = (Computed_field_simple_package*)NULL;
	}
	LEAVE;

	return (return_package);
} /* Computed_field_package_get_simple_package */


struct Computed_field_type_data *CREATE(Computed_field_type_data)
   (const char *name, Define_Computed_field_type_function
	define_Computed_field_type_function,
	Computed_field_type_package *define_type_user_data)
/*******************************************************************************
LAST MODIFIED : 24 January 2007

DESCRIPTION :
Creates a structure representing a type of computed field.  The <name> should
point to a static string which is used as the identifier of that type
throughout the program.  The <define_Computed_field_type_function> is added to
the define_computed_field option table when needed.
==============================================================================*/
{
	struct Computed_field_type_data *type_data;

	ENTER(CREATE(Computed_field_type_data));

	if (name && define_Computed_field_type_function)
	{
		if (ALLOCATE(type_data,struct Computed_field_type_data,1))
		{
			type_data->name = name;
			type_data->define_Computed_field_type_function =
				define_Computed_field_type_function;
			type_data->define_type_user_data = define_type_user_data;
			type_data->access_count = 0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Computed_field_type_data).  Not enough memory");
			type_data = (struct Computed_field_type_data *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Computed_field_type_data).  Invalid arguments");
		type_data = (struct Computed_field_type_data *)NULL;
	}
	LEAVE;

	return (type_data);
} /* CREATE(Computed_field_type_data) */

int DESTROY(Computed_field_type_data)
	(struct Computed_field_type_data **data_address)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Frees memory/deaccess data at <*data_address>.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Computed_field_type_data));
	if (data_address&&*data_address)
	{
		if (0 >= (*data_address)->access_count)
		{
			DEALLOCATE(*data_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Computed_field_type_data).  Positive access_count");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Computed_field_type_data).  Missing mapping");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Computed_field_type_data) */

DECLARE_OBJECT_FUNCTIONS(Computed_field_type_data)
FULL_DECLARE_INDEXED_LIST_TYPE(Computed_field_type_data);
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Computed_field_type_data,
  name, const char *, strcmp)
DECLARE_INDEXED_LIST_FUNCTIONS(Computed_field_type_data)


struct Add_type_to_option_table_data
{
	struct Option_table *option_table;
	Computed_field_modify_data *field_modify;
	void *computed_field_package_void;
};

static int Computed_field_add_type_to_option_table(struct Computed_field_type_data *type,
	void *add_type_to_option_table_data_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Adds <type> to the <option_table> so it is available to the commands.
==============================================================================*/
{
	int return_code;
	struct Add_type_to_option_table_data *data;

	ENTER(Computed_field_add_type_to_option_table);
	if (type&&(data=(struct Add_type_to_option_table_data *)
		add_type_to_option_table_data_void))
	{
		Option_table_add_entry(data->option_table,type->name,
			(void *)data->field_modify,
			type->define_type_user_data,
			type->define_Computed_field_type_function);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_add_type_to_option_table.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_add_type_to_option_table */

static int define_Computed_field_type(struct Parse_state *state,
	void *field_modify_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 21 July 2008

DESCRIPTION :
Part of the group of define_Computed_field functions. Here, we already have the
<field> to be modified and have determined the number of components and
coordinate system, and must now determine the type of computed field function
and its parameter fields and values.
==============================================================================*/
{
	int return_code;
	struct Add_type_to_option_table_data data;
	Computed_field_modify_data *field_modify;
	struct Computed_field_package *computed_field_package;
	struct Option_table *option_table;

	ENTER(define_Computed_field_type);
	if (state && (field_modify=(Computed_field_modify_data *)field_modify_void) &&
		(computed_field_package=(struct Computed_field_package *)
			computed_field_package_void))
	{
		if (state->current_token)
		{
			option_table=CREATE(Option_table)();
			/* new_types */
			data.option_table = option_table;
			data.field_modify = field_modify;
			data.computed_field_package_void = computed_field_package_void;
			FOR_EACH_OBJECT_IN_LIST(Computed_field_type_data)(
				Computed_field_add_type_to_option_table, (void *)&data,
				computed_field_package->computed_field_type_list);

			return_code=Option_table_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			/* OK if no more modifications */
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"define_Computed_field_type.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type */

int define_Computed_field_coordinate_system(struct Parse_state *state,
	void *field_modify_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 21 July 2008

DESCRIPTION :
Modifier entry function acting as an optional prerequisite for settings the
coordinate_system. That means that if the token "coordinate_system" or part
thereof is found in the current token, then the coordinate system is read. In
each case, assuming no error has occurred, control passes to the next parsing
level, defining the type of computed field function.  Then, if the
coordinate_system was not explictly stated, it is set in accordance with the type.
Function assumes that <field> is not currently managed, as it would be illegal
to modify it if it was.
==============================================================================*/
{
	const char *current_token;
	struct Coordinate_system coordinate_system;
	int return_code;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;

	ENTER(define_Computed_field_coordinate_system);
	if (state && (field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		if (NULL != (current_token=state->current_token))
		{
			Cmiss_field_module *field_module = field_modify->get_field_module();
			coordinate_system = Cmiss_field_module_get_coordinate_system(field_module);
			/* read the optional cooordinate_system NAME [focus #] parameter */
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (fuzzy_string_compare(current_token,"coordinate_system"))
				{
					option_table = CREATE(Option_table)();
					Option_table_add_entry(option_table,"coordinate_system",
						&coordinate_system, NULL, set_Coordinate_system);
					return_code=Option_table_parse(option_table,state);
					DESTROY(Option_table)(&option_table);
					if (return_code)
					{
						Cmiss_field_module_set_coordinate_system(field_module, coordinate_system);
						return_code = define_Computed_field_type(state, field_modify_void,
							computed_field_package_void);
					}
				}
				else
				{
					/* Default coordinate system should be set when type is defined */
					return_code=define_Computed_field_type(state,field_modify_void,
						computed_field_package_void);
				}
			}
			else
			{
				/* Write out the help */
				option_table = CREATE(Option_table)();
				Option_table_add_entry(option_table,"[coordinate_system NAME]",
					field_modify_void, computed_field_package_void,
					define_Computed_field_type);
				return_code=Option_table_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
				return_code=1;
			}
		}
		else
		{
			/* OK if no more modifications */
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_coordinate_system.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_coordinate_system */

int define_Computed_field(struct Parse_state *state, void *root_region_void,
	void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 18 July 2008

DESCRIPTION :
Modifier entry function for creating and modifying Computed_fields. Format for
parameters from the parse state are:
  [REGION_PATH/]{FIELD_NAME|NEW_FIELD_NAME}
	rectangular_cartesian/cylindrical_polar/spherical_polar/prolate_sph...
	  component
		FIELD_NAME.COMPONENT_NAME
	  composite
		number_of_scalars #
			scalars FIELD_NAME FIELD_NAME... FIELD_NAME{number_of_scalars}
	  gradient
			  scalar FIELD_NAME
				coordinate FIELD_NAME
	  rc_coordinate
				coordinate FIELD_NAME
	  scale
			field FIELD_NAME
				values # # ... #{number of components in field}
	  ... (more as more types added)
Note that the above layout is used because:
1. The number_of_components is often prerequisite information for setting up
the modifier functions for certain types of computed field, eg. "composite"
requires as many scalar fields as there are components, while scale has as many
FE_values.
2. The number_of_components and coordinate system are options for all types of
computed field so it makes sense that they are set before splitting up into the
options for the various types.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Option_table *help_option_table;
	struct Cmiss_region *region, *root_region;

	ENTER(define_Computed_field);
	if (state && (root_region = (struct Cmiss_region *)root_region_void))
	{
		if (computed_field_package_void)
		{
			return_code=1;
			if (NULL != (current_token=state->current_token))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					char *field_name = NULL;
					char *region_path = NULL;
					if (Cmiss_region_get_partial_region_path(root_region,
						current_token, &region, &region_path, &field_name))
					{
						Cmiss_field_module *field_module = Cmiss_field_module_create(region);
						if (field_name && (strlen(field_name) > 0) &&
							(strchr(field_name, CMISS_REGION_PATH_SEPARATOR_CHAR)	== NULL))
						{
							shift_Parse_state(state,1);
							Cmiss_field_module_set_field_name(field_module, field_name);
							Computed_field *existing_field = Cmiss_field_module_find_field_by_name(field_module, field_name);
							if (existing_field)
							{
								Cmiss_field_module_set_replace_field(field_module, existing_field);
								Cmiss_field_module_set_coordinate_system(field_module, existing_field->coordinate_system);
							}
							Computed_field_modify_data field_modify(field_module);
							return_code = define_Computed_field_coordinate_system(state,
								(void *)&field_modify,computed_field_package_void);
							// set coordinate system if only it has changed
							if (existing_field)
							{
								struct Coordinate_system new_coordinate_system = Cmiss_field_module_get_coordinate_system(field_module);
								if (!Coordinate_systems_match(&(existing_field->coordinate_system), &new_coordinate_system))
								{
									Computed_field_set_coordinate_system(existing_field, &new_coordinate_system);
									Computed_field_changed(existing_field);
								}
								Cmiss_field_destroy(&existing_field);
							}
							Cmiss_field_module_destroy(&field_module);
						}
						else
						{
							if (field_name)
							{
								display_message(ERROR_MESSAGE,
									"gfx define field:  Invalid region path or field name '%s'", field_name);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx define field:  Missing field name or name matches child region '%s'", current_token);
							}
							display_parse_state_location(state);
							return_code = 0;
						}
						DEALLOCATE(region_path);
						DEALLOCATE(field_name);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx define field: Bad region_path/field_name '%s'", current_token);
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				else
				{
					/* Write out the help */
					Cmiss_field_module *field_module =
						Cmiss_field_module_create(root_region);
					Computed_field_modify_data field_modify(field_module);
					help_option_table = CREATE(Option_table)();
					Option_table_add_entry(help_option_table,
						"[REGION_PATH" CMISS_REGION_PATH_SEPARATOR_STRING "]FIELD_NAME",
						(void *)&field_modify, computed_field_package_void,
						define_Computed_field_coordinate_system);
					return_code=Option_table_parse(help_option_table,state);
					DESTROY(Option_table)(&help_option_table);
					Cmiss_field_module_destroy(&field_module);
					return_code = 1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing field name");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field.  Missing computed_field_package_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field */


int process_list_or_write_Computed_field_commands(struct Computed_field *field,
	 void *command_prefix_void, Process_list_or_write_command_class *process_message)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Writes the commands needed to reproduce <field> to the command window.
==============================================================================*/
{
	int return_code;
	char *command_prefix = reinterpret_cast<char *>(command_prefix_void);
	if (field && command_prefix)
	{
		char *command_string = field->core->get_command_string();
		if (command_string)
		{
			char *field_name = duplicate_string(field->name);
			make_valid_token(&field_name);
			char *coordinate_system_string = Coordinate_system_string(&field->coordinate_system);
			process_message->process_command(INFORMATION_MESSAGE,
				"%s%s coordinate_system %s %s;\n",
				command_prefix, field_name, coordinate_system_string, command_string);
			DEALLOCATE(field_name);
			DEALLOCATE(coordinate_system_string);
			DEALLOCATE(command_string);
		}
		else
		{
			process_message->process_command(INFORMATION_MESSAGE, "# field %s created by other commands\n", field->name);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"process_list_or_write_Computed_field_commands.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int list_Computed_field_commands(struct Computed_field *field,
	void *command_prefix_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Writes the commands needed to reproduce <field> to the command window.
==============================================================================*/
{
	 int return_code = 0;

	 ENTER(list_Computed_field_commands);
	 Process_list_command_class *list_message =
			new Process_list_command_class();
	 if (list_message != 0)
	 {
			return_code = process_list_or_write_Computed_field_commands(
				 field, command_prefix_void, list_message);
			delete list_message;
	 }
	 LEAVE;

	 return (return_code);
}

int list_Computed_field_commands_if_managed_source_fields_in_list(
	struct Computed_field *field, void *list_commands_data_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Calls list_Computed_field_commands if the field is not already in the list,
has no source fields, or all its source fields are either not managed or
already in the list. If the field is listed, it is added to the list.
Ensures field command list comes out in the order they need to be created.
Note, must be cycled through as many times as it takes till listed_fields -> 0.
Second argument is a struct List_Computed_field_commands_data.
==============================================================================*/
{
	int i, list_field, return_code;
	struct List_Computed_field_commands_data *list_commands_data;

	ENTER(list_Computed_field_commands_if_managed_source_fields_in_list);
	if (field && (list_commands_data =
		(struct List_Computed_field_commands_data *)list_commands_data_void))
	{
		return_code = 1;
		/* is the field not listed yet? */
		if (!IS_OBJECT_IN_LIST(Computed_field)(field,
			list_commands_data->computed_field_list))
		{
			list_field = 1;
			for (i = 0; list_field && (i < field->number_of_source_fields); i++)
			{
				if ((!IS_OBJECT_IN_LIST(Computed_field)(
					field->source_fields[i], list_commands_data->computed_field_list)) &&
					IS_MANAGED(Computed_field)(field->source_fields[i],
						list_commands_data->computed_field_manager))
				{
					list_field = 0;
				}
			}
			if (list_field)
			{
				/* do not list commands for read-only computed fields created
					 automatically by cmgui */
				return_code =
					list_Computed_field_commands(field,
						(void *)list_commands_data->command_prefix) &&
					ADD_OBJECT_TO_LIST(Computed_field)(field,
						list_commands_data->computed_field_list);
				list_commands_data->listed_fields++;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_commands_if_managed_source_fields_in_list.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_commands_if_managed_source_fields_in_list */

int write_Computed_field_commands_to_comfile(struct Computed_field *field,
	 void *command_prefix_void)
/*******************************************************************************
LAST MODIFIED : 10 August 2007

DESCRIPTION :
Writes the commands needed to reproduce <field> to the com file.
==============================================================================*/
{
	int return_code = 0;

	ENTER(write_Computed_field_commands_to_comfile);
	Process_write_command_class *write_message =
		 new Process_write_command_class();
	if (write_message)
	{
		 return_code = process_list_or_write_Computed_field_commands(
				field,  command_prefix_void, write_message);
		 delete write_message;
	}
	return (return_code);
	LEAVE;

	return (return_code);
} /* write_Computed_field_commands_to_comfile */

int write_Computed_field_commands_if_managed_source_fields_in_list_to_comfile(
	 struct Computed_field *field, void *list_commands_data_void)
/*******************************************************************************
LAST MODIFIED : 10 August 2007

DESCRIPTION :
Calls list_Computed_field_commands if the field is not already in the list,
has no source fields, or all its source fields are either not managed or
already in the list. If the field is listed, it is added to the list.
Ensures field command list comes out in the order they need to be created.
Note, must be cycled through as many times as it takes till listed_fields -> 0.
Second argument is a struct List_Computed_field_commands_data.
==============================================================================*/
{
	int i, list_field, return_code;
	struct List_Computed_field_commands_data *list_commands_data;

	ENTER(write_Computed_field_commands_if_managed_source_fields_in_list_to_comfile);
	if (field && (list_commands_data =
		(struct List_Computed_field_commands_data *)list_commands_data_void))
	{
		return_code = 1;
		/* is the field not listed yet? */
		if (!IS_OBJECT_IN_LIST(Computed_field)(field,
			list_commands_data->computed_field_list))
		{
			list_field = 1;
			for (i = 0; list_field && (i < field->number_of_source_fields); i++)
			{
				if ((!IS_OBJECT_IN_LIST(Computed_field)(
					field->source_fields[i], list_commands_data->computed_field_list)) &&
					IS_MANAGED(Computed_field)(field->source_fields[i],
						list_commands_data->computed_field_manager))
				{
					list_field = 0;
				}
			}
			if (list_field)
			{
				/* do not list commands for read-only computed fields created
					 automatically by cmgui */
				return_code =
					write_Computed_field_commands_to_comfile(field,
						 (void *)list_commands_data->command_prefix) &&
					ADD_OBJECT_TO_LIST(Computed_field)(field,
						list_commands_data->computed_field_list);
				list_commands_data->listed_fields++;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_commands_if_managed_source_fields_in_list.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_Computed_field_commands_if_managed_source_fields_in_list_to_comfile */


int Computed_field_package_add_type(
	struct Computed_field_package *computed_field_package, const char *name,
	Define_Computed_field_type_function define_Computed_field_type_function,
	Computed_field_type_package *define_type_user_data)
/*******************************************************************************
LAST MODIFIED : 24 January 2007

DESCRIPTION :
Adds the type of Computed_field described by <name> and
<define_Computed_field_type_function> to those in the LIST held by the
<computed_field_package>.  This type is then added to the
define_Computed_field_type option table when parsing commands.
==============================================================================*/
{
	int return_code;
	struct Computed_field_type_data *data;

	ENTER(Computed_field_package_add_type);
	if (computed_field_package && name && define_Computed_field_type_function &&
		 define_type_user_data)
	{
		if(NULL != (data = CREATE(Computed_field_type_data)(name,
			define_Computed_field_type_function, define_type_user_data)))
		{
			data->define_type_user_data->addref();
			return_code = ADD_OBJECT_TO_LIST(Computed_field_type_data)(data,
				computed_field_package->computed_field_type_list);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_package_add_type.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_package_add_type */

int Computed_field_package_remove_types(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 24 January 2007

DESCRIPTION :
Unregisters each of the computed field types added.
==============================================================================*/
{
	int return_code = 0;
	struct Computed_field_type_data *data;

	ENTER(Computed_field_package_remove_types);
	if (computed_field_package)
	{
		while(NULL != (data = FIRST_OBJECT_IN_LIST_THAT(Computed_field_type_data)(
			(LIST_CONDITIONAL_FUNCTION(Computed_field_type_data) *)NULL, (void *)NULL,
			computed_field_package->computed_field_type_list)))
		{
			data->define_type_user_data->removeref();

			REMOVE_OBJECT_FROM_LIST(Computed_field_type_data)(data,
				computed_field_package->computed_field_type_list);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_package_add_type.  Invalid arguments");
	}
	LEAVE;

	return (return_code);
} /* Computed_field_package_add_type */

