/*******************************************************************************
FILE : computed_field_time.h

LAST MODIFIED : 19 September 2003

DESCRIPTION :
Implements computed fields that control the time behaviour.
==============================================================================*/
#if !defined (COMPUTED_FIELD_TIME_H)
#define COMPUTED_FIELD_TIME_H

int Computed_field_register_types_time(
	struct Computed_field_package *computed_field_package,
	struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
==============================================================================*/

int Computed_field_set_type_time_lookup(struct Computed_field *field,
	struct Computed_field *source_field, struct Computed_field *time_field);
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_TIME_LOOKUP with the supplied
fields, <source_field> is the field the values are returned from but rather
than using the current time, the scalar <time_field> is evaluated and its value
is used for the time.
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_TIME_H) */
