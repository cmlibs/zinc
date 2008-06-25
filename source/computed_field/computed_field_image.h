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
#if !defined (COMPUTED_FIELD_SAMPLE_TEXTURE_H)
#define COMPUTED_FIELD_SAMPLE_TEXTURE_H

#include "api/cmiss_field_image.h"

#define Computed_field_create_image Cmiss_field_create_image

/*****************************************************************************//**
 * Creates a new image based field.  This constructor does not define the
 * actual image data, which should then be set using a Cmiss_field_image_set_*
 * function.
 * 
 * @param domain_field  The field in which the image data will be embedded.
 * @return Newly created field
*/
struct Computed_field *Computed_field_create_image(struct Computed_field *domain_field);

int Computed_field_register_type_image(
	struct Computed_field_package *computed_field_package, 
	struct MANAGER(Texture) *texture_manager);
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
==============================================================================*/

int Computed_field_depends_on_texture(struct Computed_field *field,
	struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 4 September 2007

DESCRIPTION :
Returns true if the field or recursively any source fields are sample
texture fields which reference <texture>.
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_SAMPLE_TEXTURE_H) */
