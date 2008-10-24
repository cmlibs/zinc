/*******************************************************************************
FILE : material.h

LAST MODIFIED : 6 December 2004

DESCRIPTION :
The data structures used for representing graphical materials.
???RC Only OpenGL is supported now.
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
#if !defined (MATERIAL_H)
#define MATERIAL_H

#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/colour.h"
#include "graphics/spectrum.h"
#include "graphics/texture.h"

/*
Global constants
----------------
*/
/*???DB.  Make consistent with finite_element.h ? */
#define MATERIAL_PRECISION float
#define MATERIAL_PRECISION_STRING "f"

/*
Global types
------------
*/
struct IO_stream;
/*******************************************************************************
LAST MODIFIED : 6 December 2004

DESCRIPTION :
Declared here to satisfy function prototype.
==============================================================================*/


struct Material_package;
/*******************************************************************************
LAST MODIFIED : 20 November 2003;

DESCRIPTION :
==============================================================================*/

struct Graphical_material;
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
The contents of a graphical material are private.
==============================================================================*/

DECLARE_LIST_TYPES(Graphical_material);

DECLARE_MANAGER_TYPES(Graphical_material);

struct Graphical_material_Texture_change_data
{
	struct LIST(Texture) *changed_texture_list;
	struct LIST(Graphical_material) *changed_material_list;
};

/*
Global functions
----------------
*/

struct Material_package *CREATE(Material_package)(
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Spectrum) *spectrum_manager);
/*******************************************************************************
LAST MODIFIED : 20 May 2005

DESCRIPTION :
Create a shared information container for Materials.
==============================================================================*/

int DESTROY(Material_package)(struct Material_package **material_package_address);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Material_package);

int Material_package_manage_material(struct Material_package *material_package,
	struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
This puts the <material> into the manager connected with the <material_package>
and allows the OpenGL states within the materials mangaed by the package to be
shared.
==============================================================================*/

struct Graphical_material *Material_package_get_default_material(
	struct Material_package *material_package);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Returns the default material object.
==============================================================================*/

struct Graphical_material *Material_package_get_default_selected_material(
	struct Material_package *material_package);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Returns the default_selected material object.
==============================================================================*/

struct MANAGER(Graphical_material) *Material_package_get_material_manager(
	struct Material_package *material_package);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Returns the material manager.
==============================================================================*/

struct Graphical_material *CREATE(Graphical_material)(const char *name);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
==============================================================================*/

int DESTROY(Graphical_material)(struct Graphical_material **material_address);
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Frees the memory for the material and sets <*material_address> to NULL.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Graphical_material);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Graphical_material);

PROTOTYPE_LIST_FUNCTIONS(Graphical_material);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Graphical_material,name,const char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Graphical_material,name,const char *);
PROTOTYPE_MANAGER_FUNCTIONS(Graphical_material);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Graphical_material,name,const char *);

const char *Graphical_material_name(struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
While the GET_NAME macro returns a copy of the name of an object, this function
has been created for returning just a pointer to the material's name, or some
other string if the name is invalid, suitable for putting in printf statements.
Be careful with the returned value: esp. do not modify or DEALLOCATE it!
==============================================================================*/

int Graphical_material_get_ambient(struct Graphical_material *material,
	struct Colour *diffuse);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the ambient colour of the material.
==============================================================================*/

int Graphical_material_set_ambient(struct Graphical_material *material,
	struct Colour *diffuse);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the ambient colour of the material.
==============================================================================*/

int Graphical_material_get_diffuse(struct Graphical_material *material,
	struct Colour *diffuse);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the diffuse colour of the material.
==============================================================================*/

int Graphical_material_set_diffuse(struct Graphical_material *material,
	struct Colour *diffuse);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the diffuse colour of the material.
==============================================================================*/

int Graphical_material_get_emission(struct Graphical_material *material,
	struct Colour *emission);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the emission colour of the material.
==============================================================================*/

int Graphical_material_set_emission(struct Graphical_material *material,
	struct Colour *emission);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the emission colour of the material.
==============================================================================*/

int Graphical_material_get_specular(struct Graphical_material *material,
	struct Colour *emission);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the specular colour of the material.
==============================================================================*/

int Graphical_material_set_specular(struct Graphical_material *material,
	struct Colour *emission);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the specular colour of the material.
==============================================================================*/

int Graphical_material_get_alpha(struct Graphical_material *material,
	MATERIAL_PRECISION *alpha);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the alpha value of the material.
==============================================================================*/

int Graphical_material_set_alpha(struct Graphical_material *material,
	MATERIAL_PRECISION alpha);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the alpha value of the material.
==============================================================================*/

int Graphical_material_get_shininess(struct Graphical_material *material,
	MATERIAL_PRECISION *shininess);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the shininess value of the material.
==============================================================================*/

int Graphical_material_set_shininess(struct Graphical_material *material,
	MATERIAL_PRECISION shininess);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the shininess value of the material.
==============================================================================*/

int Graphical_material_set_colour_lookup_spectrum(struct Graphical_material *material,
	struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 6 October 2006

DESCRIPTION :
Sets the spectrum member of the material.
==============================================================================*/

struct Spectrum *Graphical_material_get_colour_lookup_spectrum(
	struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 6 October 2006

DESCRIPTION :
Returns the spectrum member of the material.
==============================================================================*/

struct Texture *Graphical_material_get_texture(
	struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Returns the texture member of the material.
==============================================================================*/

struct Texture *Graphical_material_get_second_texture(
	 struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 5 Dec 2007

DESCRIPTION :
Returns the second texture of the material.
==============================================================================*/

struct Texture *Graphical_material_get_third_texture(
	 struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 5 Dec 2007

DESCRIPTION :
Returns the third texture of the material.
==============================================================================*/


struct Texture *Graphical_material_get_fourth_texture(
	 struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 5 Dec 2007

DESCRIPTION :
Returns the fourth texture of the material.
==============================================================================*/


int Graphical_material_get_bump_mapping_flag(struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 5 December 2007

DESCRIPTION :
Returns the flag set for bump_mapping.
==============================================================================*/

int Graphical_material_get_per_pixel_lighting_flag(struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 5 December 2007

DESCRIPTION :
Returns the flag set for per_pixel_lighting.
==============================================================================*/

int Graphical_material_set_texture(struct Graphical_material *material,
	struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Sets the texture member of the material.
==============================================================================*/

int Graphical_material_set_second_texture(struct Graphical_material *material,
	 struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 5 December 2007

DESCRIPTION :
Sets the second texture member of the material.
==============================================================================*/

int Graphical_material_set_third_texture(struct Graphical_material *material,
	 struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 5 December 2007

DESCRIPTION :
Sets the third texture member of the material.
==============================================================================*/

int Graphical_material_set_fourth_texture(struct Graphical_material *material,
	 struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 5 December 2007

DESCRIPTION :
Sets the fourth texture member of the material.
==============================================================================*/

int Graphical_material_uses_texture_in_list(struct Graphical_material *material,
	void *texture_list_void);
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Returns true if the <material> uses a texture in the <texture_list>.
==============================================================================*/

int Graphical_material_Texture_change(struct Graphical_material *material,
	void *texture_change_data_void);
/*******************************************************************************
LAST MODIFIED : 13 March 2002

DESCRIPTION :
If the <material> uses a texture in the <changed_texture_list>, marks the
material compile_status as CHILD_GRAPHICS_NOT_COMPILED and adds the material
to the <changed_material_list>.
???RC Currently managed by Scene. This function should be replaced once messages
go directly from texture to material.
==============================================================================*/

int gfx_create_material(struct Parse_state *state,
	void *dummy_to_be_modified, void *material_package_void);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Shifted from command/cmiss.c now that there is a material package.
If the material already exists, then behaves like gfx modify material.
==============================================================================*/

int modify_Graphical_material(struct Parse_state *parse_state,void *material,
	void *material_package_void);
/*******************************************************************************
LAST MODIFIED : 5 September 1996

DESCRIPTION :
==============================================================================*/

int list_Graphical_material(struct Graphical_material *material,void *dummy);
/*******************************************************************************
LAST MODIFIED : 22 September 1998

DESCRIPTION :
Writes the properties of the <material> to the command window.
==============================================================================*/

int list_Graphical_material_commands(struct Graphical_material *material,
	void *command_prefix_void);
/*******************************************************************************
LAST MODIFIED : 22 September 1998

DESCRIPTION :
Writes on the command window the commands needed to recreate the <material>.
The command is started with the string pointed to by <command_prefix>.
==============================================================================*/

int write_Graphical_material_commands_to_comfile(struct Graphical_material *material,
	 void *command_prefix_void);
/*******************************************************************************
LAST MODIFIED : 10 August 2007

DESCRIPTION :
Writes on the command window the command needed to recreate the <material>.
The command is started with the string pointed to by <command_prefix>.
==============================================================================*/

int file_read_Graphical_material_name(struct IO_stream *file,
	struct Graphical_material **material_address,
	struct MANAGER(Graphical_material) *graphical_material_manager);
/*******************************************************************************
LAST MODIFIED : 6 December 2004

DESCRIPTION :
Reads a material name from a <file>.  Searchs the list of all materials for one
with the specified name.  If one is not found a new one is created with the
specified name and the default properties.
==============================================================================*/

int set_material_program_type(struct Graphical_material *material_to_be_modified,
	 int bump_mapping_flag, int colour_lookup_red_flag, int colour_lookup_green_flag, 
	 int colour_lookup_blue_flag,  int colour_lookup_alpha_flag, 
	 int lit_volume_intensity_normal_texture_flag, int lit_volume_finite_difference_normal_flag, 
	 int lit_volume_scale_alpha_flag, int return_code);
/****************************************************************************** 
LAST MODIFIED : 4 Dec 2007

DESCRIPTION : Set up the material program type for using the vertex
and fragment program. This and following functions are orginally
from the modify_graphical_material. 
NOTE: I use the pointer to the material_package from the material.
==============================================================================*/

int material_copy_bump_mapping_and_per_pixel_lighting_flag(struct Graphical_material *material,
	 struct Graphical_material *material_to_be_modified);
/****************************************************************************** 
LAST MODIFIED : 5 Dec 2007

DESCRIPTION : This function will set the bump mapping and per
pixel_lighting_flag of the material_to_be_modified to be the same as
the one in material, it is used for setting up the GUI.
==============================================================================*/

#if defined (WX_USER_INTERFACE)
int material_deaccess_material_program(struct Graphical_material *material_to_be_modified);
/****************************************************************************** 
LAST MODIFIED : 4 Dec 2007

DESCRIPTION : This function is to allow the material editor to
deaccess the material program from the material.
==============================================================================*/
#endif /*(WX_USER_INTERFACE)*/


#if defined (OLD_CODE)
int activate_Graphical_material(struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 17 November 1994

DESCRIPTION :
Activates the <material> as part of the rendering loop.
==============================================================================*/
#endif /* defined (OLD_CODE) */

int compile_Graphical_material_for_order_independent_transparency(
	struct Graphical_material *material, 
	void *material_order_independent_data_void);
/*******************************************************************************
LAST MODIFIED : 2 May 2005

DESCRIPTION :
Recompile each of the <materials> which have already been compiled so that they
will work with order_independent_transparency. 
==============================================================================*/

int set_Graphical_material(struct Parse_state *state,
	void *material_address_void,void *graphical_material_manager_void);
/*******************************************************************************
LAST MODIFIED : 20 June 1996

DESCRIPTION :
Modifier function to set the material from a command.
==============================================================================*/

int Option_table_add_set_Material_entry(
	struct Option_table *option_table, char *token,
	struct Graphical_material **material, struct Material_package *material_package);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Adds the given <token> to the <option_table>.  The <material> is selected from
the <material_package> by name.
==============================================================================*/
#endif
