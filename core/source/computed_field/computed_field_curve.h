/*******************************************************************************
FILE : computed_field_curve.h

LAST MODIFIED : 31 May 2001

DESCRIPTION :
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMPUTED_FIELD_CURVE_H)
#define COMPUTED_FIELD_CURVE_H

#include "curve/curve.h"

/*****************************************************************************//**
 * Creates a field which returns the value of curve at the time/parameter value
 * given by scalar <source_field>.
 * Field has number of components to same number as <curve>.
 * 
 * @param field_module  Region field module which will own new field.
 * @param source_field  Field providing time/parameter values for curve lookup.
 * @param curve  Parametric curve object.
 * @param curve_manager  Manager of curve objects.
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_curve_lookup(
	cmzn_fieldmodule *field_module, struct Computed_field *source_field,
	struct Curve *curve, struct MANAGER(Curve) *curve_manager);

int Computed_field_get_type_curve_lookup(struct Computed_field *field,
	struct Computed_field **source_field, struct Curve **curve);
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CURVE_LOOKUP, the source_field and curve
used by it are returned - otherwise an error is reported.
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_CURVE_H) */
