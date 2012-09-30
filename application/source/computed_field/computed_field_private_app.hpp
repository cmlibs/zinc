

#if !defined (COMPUTED_FIELD_PRIVATE_APP_H_)
#define COMPUTED_FIELD_PRIVATE_APP_H_

/* Used by the register_type_function, Computed_field_type_data and
	Computed_field_add_type_to_option_table*/
typedef int (*Define_Computed_field_type_function)(
	struct Parse_state *state,void *field_void,void *computed_field_package_void);


int Computed_field_package_add_type(
	struct Computed_field_package *computed_field_package, const char *name,
	Define_Computed_field_type_function define_Computed_field_type_function,
	Computed_field_type_package *define_type_user_data);
/*******************************************************************************
LAST MODIFIED : 24 January 2007

DESCRIPTION :
Adds the type of Computed_field described by <name> and
<define_Computed_field_type_function> to those in the LIST held by the
<computed_field_package>.  This type is then added to the
define_Computed_field_type option table when parsing commands.
==============================================================================*/


#endif
