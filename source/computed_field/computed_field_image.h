/*******************************************************************************
FILE : computed_field_image.h

LAST MODIFIED : 4 July 2000

DESCRIPTION :
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#if !defined (COMPUTED_FIELD_IMAGE_H)
#define COMPUTED_FIELD_IMAGE_H

#include "api/cmiss_field_image.h"
#include "computed_field/computed_field.h"
#include "graphics/texture.h"

#define Computed_field_create_image Cmiss_field_module_create_image

/*****************************************************************************//**
 * Creates a new image based field.  This constructor does not define the
 * actual image data, which should then be set using a Cmiss_field_image_set_*
 * function.
 * 
 * @param field_module  Region field module which will own new field.
 * @param domain_field  The field in which the image data will be embedded.
 * @param source_field  Optional source field to automatically provides pixel
 * values to the image.
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_image(
	struct Cmiss_field_module *field_module,
	struct Computed_field *domain_field,
	struct Computed_field *source_field);

int Computed_field_register_type_image(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
==============================================================================*/

/***************************************************************************//**
 * A convenient function to get texture out from a computed_field if it is of
 * type image. This function should not be made available to the external API.
 *
 * @param field  Compited_field to get the texture from.
 * @return  Returns texture if successfully get a texture from the provided
 *   field, otherwise NULL.
 */
struct Texture *Computed_field_get_texture(struct Computed_field *field);

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

int Cmiss_field_image_set_texture(Cmiss_field_image_id image_field,
		struct Texture *texture);

/*****************************************************************************//**
 * A get function that gets the internal cmiss_texture from the image field.
 * This is an internal function.
 *
 * @param image_field  The image field to get the texture from.
 * @return Returns the handle to cmiss texture.
 */
Cmiss_texture_id Cmiss_field_image_get_texture(Cmiss_field_image_id image_field);

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
int Set_cmiss_field_value_to_texture(struct Cmiss_field *field,
		struct Cmiss_field *texture_coordinate_field,	struct Texture *texture,
		struct Cmiss_spectrum *spectrum,	struct Cmiss_graphics_material *fail_material,
		int image_height, int image_width, int image_depth, int bytes_per_pixel,
		int number_of_bytes_per_component, int use_pixel_location,
		enum Texture_storage_type specify_format, int propagate_field,
		struct Graphics_buffer_package *graphics_buffer_package, int element_dimension,
		struct Cmiss_region *search_region);

#endif /* !defined (COMPUTED_FIELD_IMAGE_H) */
