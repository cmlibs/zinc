/*******************************************************************************
FILE : computed_field_image.h

LAST MODIFIED : 4 July 2000

DESCRIPTION :
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMPUTED_FIELD_IMAGE_H)
#define COMPUTED_FIELD_IMAGE_H

#include "opencmiss/zinc/fieldimage.h"
#include "computed_field/computed_field.h"
#include "graphics/texture.h"

/***************************************************************************//**
 * A convenient function to get texture out from a computed_field if it is of
 * type image. This function should not be made available to the external API.
 *
 * @param field  Compited_field to get the texture from.
 * @return  Returns texture if successfully get a texture from the provided
 *   field, otherwise NULL.
 */
struct Texture *cmzn_field_image_get_texture(cmzn_field_image_id image_field);

int Computed_field_depends_on_texture(struct Computed_field *field,
	struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 4 September 2007

DESCRIPTION :
Returns true if the field or recursively any source fields are sample
texture fields which reference <texture>.
==============================================================================*/

/***************************************************************************//**
 * A function to identify an image field.
 *
 * @param field  Compited_field to be identified.
 * @return  Returns 1 if field is an image field, otherwise 0.
 */
int Computed_field_is_image_type(struct Computed_field *field,
	void *dummy_void);

int cmzn_field_image_set_texture(cmzn_field_image_id image_field,
		struct Texture *texture);

/*****************************************************************************//**
 * A get function that gets the internal cmiss_texture from the image field.
 * This is an internal function.
 *
 * @param image_field  The image field to get the texture from.
 * @return Returns the handle to cmiss texture.
 */
cmzn_texture_id cmzn_field_image_get_texture(cmzn_field_image_id image_field);

/***************************************************************************//**
 * A function to list information of the texture in an image field.
 */
int list_image_field(struct Computed_field *field,void *dummy_void);

/***************************************************************************//**
 * A function to list the command to create an image field.
 */
int list_image_field_commands(struct Computed_field *field,void *command_prefix_void);

/***************************************************************************//**
 * A function to evaluate a field into a texture, the texture must allocate the
 * image before passing into this function.
 */
int Set_cmiss_field_value_to_texture(struct cmzn_field *field,
	struct cmzn_field *texture_coordinate_field, struct Texture *texture,
	struct cmzn_spectrum *spectrum,	struct cmzn_material *fail_material,
	int image_height, int image_width, int image_depth, int bytes_per_pixel,
	int number_of_bytes_per_component, int use_pixel_location,
	enum Texture_storage_type specify_format, int propagate_field,
	struct Graphics_buffer_package *graphics_buffer_package,
	cmzn_mesh_id search_mesh);

#endif /* !defined (COMPUTED_FIELD_IMAGE_H) */
