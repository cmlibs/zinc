/*******************************************************************************
FILE : computed_field_fastMarchingImageFilter.h

LAST MODIFIED : 31 May 2001

DESCRIPTION :
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMPUTED_FIELD_FAST_MARCHING_IMAGE_FILTER_H)
#define COMPUTED_FIELD_FAST_MARCHING_IMAGE_FILTER_H

#include "zinc/field.h"

int cmzn_field_get_type_fast_marching_image_filter(struct Computed_field *field,
	struct Computed_field **source_field,double *stopping_value,
  int *num_seed_points, int *dimension, double **seed_points, double **seed_values, 
  int **output_size);
/*******************************************************************************
LAST MODIFIED : 18 Nov 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_FAST_MARCHING_IMAGE_FILTER, the source_field and fast_marching_image_filter
used by it are returned - otherwise an error is reported.
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_FAST_MARCHING_IMAGE_FILTER_H) */
