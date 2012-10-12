



Computed_field *Computed_field_create_sin(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field);

Computed_field *Computed_field_create_cos(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field);

Computed_field *Computed_field_create_tan(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field);

Computed_field *Computed_field_create_asin(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field);

Computed_field *Computed_field_create_acos(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field);

Computed_field *Computed_field_create_atan(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field);

Computed_field *Computed_field_create_atan2(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);

int Computed_field_register_types_trigonometry(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
==============================================================================*/
