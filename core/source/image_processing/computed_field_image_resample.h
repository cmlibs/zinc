/*******************************************************************************
FILE : computed_field_image_resample.h

LAST MODIFIED : 7 March 2007

DESCRIPTION :
Field that changes the native resolution of a computed field.
Image processing fields use the native resolution to determine their image size.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMPUTED_FIELD_IMAGE_RESAMPLE_H)
#define COMPUTED_FIELD_IMAGE_RESAMPLE_H

/***************************************************************************//**
 * Create a field which resamples the image in source field, and overrides the
 * sizes that will be used for subsequent image processing operations.
 * The texture_coordinate field could also be overridden.  The minimums and
 * maximums are not implemented at all, which would allow a windowing, and could
 * also be overridden here.
 * 
 * @param field_module  Region field module which will own new field.
 * @return  Newly created field.
 */
Computed_field *cmzn_field_module_create_image_resample(
	struct cmzn_field_module *field_module,
	struct Computed_field *source_field, int dimension, int *sizes);

#endif /* !defined (COMPUTED_FIELD_IMAGE_RESAMPLE_H) */
