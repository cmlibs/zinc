/*******************************************************************************
FILE : computed_field_trigonometry.h

LAST MODIFIED : 10 June 2004

DESCRIPTION :
Implements a number of basic trigonometry operations on computed fields.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMPUTED_FIELD_TRIGONOMETRY_H)
#define COMPUTED_FIELD_TRIGONOMETRY_H

#include "zinc/fieldtrigonometry.h"

/* API functions are prefixed with cmzn */
#define Computed_field_create_sin cmzn_fieldmodule_create_field_sin
#define Computed_field_create_cos cmzn_fieldmodule_create_field_cos
#define Computed_field_create_tan cmzn_fieldmodule_create_field_tan
#define Computed_field_create_asin cmzn_fieldmodule_create_field_asin
#define Computed_field_create_acos cmzn_fieldmodule_create_field_acos
#define Computed_field_create_atan cmzn_fieldmodule_create_field_atan
#define Computed_field_create_atan2 cmzn_fieldmodule_create_field_atan2

#endif /* !defined (COMPUTED_FIELD_TRIGONOMETRY_H) */
