/*******************************************************************************
FILE : computed_field_window_projection.h

LAST MODIFIED : 4 July 2000

DESCRIPTION :
==============================================================================*/
#if !defined (COMPUTED_FIELD_WINDOW_PROJECTION_H)
#define COMPUTED_FIELD_WINDOW_PROJECTION_H

char *Computed_field_window_projection_type_string(void);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Return the static type string which identifies this type.
==============================================================================*/

int Computed_field_register_type_window_projection(
	struct Computed_field_package *computed_field_package, 
	struct MANAGER(Graphics_window) *graphics_window_manager);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_WINDOW_PROJECTION_H) */
