/*******************************************************************************
FILE : computed_field_control_curve.h

LAST MODIFIED : 31 May 2001

DESCRIPTION :
==============================================================================*/
#if !defined (COMPUTED_FIELD_CONTROL_CURVE_H)
#define COMPUTED_FIELD_CONTROL_CURVE_H

#include "curve/control_curve.h"

int Computed_field_register_types_control_curve(
	struct Computed_field_package *computed_field_package, 
	struct MANAGER(Control_curve) *control_curve_manager);
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
==============================================================================*/

int Computed_field_set_type_curve_lookup(struct Computed_field *field,
	struct Computed_field *source_field, struct Control_curve *curve,
	struct MANAGER(Computed_field) *computed_field_manager,
	struct MANAGER(Control_curve) *control_curve_manager);
/*******************************************************************************
LAST MODIFIED : 24 May 2001

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CURVE_LOOKUP, returning the value of
<curve> at the time/parameter value given by scalar <source_field>.
Sets number of components to same number as <curve>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
???RC In future may not need to pass computed_field_manager it all fields
maintain pointer to it. Only have it to invoke computed field manager messages
in response to changes in the curve from the control curve manager.
==============================================================================*/

int Computed_field_get_type_curve_lookup(struct Computed_field *field,
	struct Computed_field **source_field, struct Control_curve **curve);
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CURVE_LOOKUP, the source_field and curve
used by it are returned - otherwise an error is reported.
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_CONTROL_CURVE_H) */
