/*******************************************************************************
FILE : computed_field_threshold_image_filter.h

LAST MODIFIED : 16 May 2008

DESCRIPTION :
Wraps itk::ThresholdImageFilter
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMPUTED_FIELD_THRESHOLD_IMAGE_FILTER_H)
#define COMPUTED_FIELD_THRESHOLD_IMAGE_FILTER_H

#include "zinc/field.h"
#include "zinc/fieldimageprocessing.h"

cmzn_field_threshold_image_filter_id cmzn_field_cast_threshold_image_filter(cmzn_field_id field);

PROTOTYPE_ENUMERATOR_FUNCTIONS(General_threshold_filter_mode);

#endif /* !defined (COMPUTED_FIELD_THRESHOLD_IMAGE_FILTER_H) */
