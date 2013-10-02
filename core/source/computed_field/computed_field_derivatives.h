/*******************************************************************************
FILE : computed_field_derivatives.h

LAST MODIFIED : 1 November 2000

DESCRIPTION :
Implements computed_fields for calculating various derivative quantities such
as derivatives w.r.t. Xi, gradient, curl, divergence etc.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMPUTED_FIELD_DERIVATIVES_H)
#define COMPUTED_FIELD_DERIVATIVES_H

#include "zinc/fieldderivatives.h"

#define Computed_field_create_gradient cmzn_fieldmodule_create_field_gradient
#define Computed_field_create_divergence cmzn_fieldmodule_create_field_divergence
#define Computed_field_create_curl cmzn_fieldmodule_create_field_curl
#define Computed_field_create_derivative cmzn_fieldmodule_create_field_derivative

#endif /* !defined (COMPUTED_FIELD_DERIVATIVES_H) */
