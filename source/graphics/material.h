/*******************************************************************************
FILE : material.h

LAST MODIFIED : 6 December 2004

DESCRIPTION :
The data structures used for representing graphical materials.
???RC Only OpenGL is supported now.
==============================================================================*/
#if !defined (MATERIAL_H)
#define MATERIAL_H

#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/colour.h"
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

struct Material_package *CREATE(Material_package)(struct MANAGER(Texture) *texture_manager);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

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

struct Graphical_material *CREATE(Graphical_material)(char *name);
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
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Graphical_material,name,char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Graphical_material,name,char *);
PROTOTYPE_MANAGER_FUNCTIONS(Graphical_material);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Graphical_material,name,char *);

int direct_render_Graphical_material(struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Directly outputs the graphics library commands for activating <material>.
==============================================================================*/

char *Graphical_material_name(struct Graphical_material *material);
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

struct Texture *Graphical_material_get_texture(
	struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Returns the texture member of the material.
==============================================================================*/

int Graphical_material_set_texture(struct Graphical_material *material,
	struct Texture *texture);
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Sets the texture member of the material.
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

int Graphical_material_set_spectrum_flag(struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the spectrum_flag of a material giving it the cue to intelligently issue
commands from direct_render_Graphical_material.
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

#if defined (OLD_CODE)
int activate_Graphical_material(struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 17 November 1994

DESCRIPTION :
Activates the <material> as part of the rendering loop.
==============================================================================*/
#endif /* defined (OLD_CODE) */

int compile_Graphical_material(struct Graphical_material *material,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Graphical_material list/manager iterator function.
Rebuilds the display_list for <material> if it is not current. If <material>
does not have a display list, first attempts to give it one. The display list
created here may be called using execute_Graphical_material, below.
???RC Graphical_materials must be compiled before they are executed since openGL
cannot start writing to a display list when one is currently being written to.
???RC The behaviour of materials is set up to take advantage of pre-computed
display lists. To switch to direct rendering make this routine do nothing and
execute_Graphical_material should just call direct_render_Graphical_material.
==============================================================================*/

int execute_Graphical_material(struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 7 September 2000

DESCRIPTION :
Activates <material> by calling its display list. If the display list is not
current, an error is reported.
Passing a NULL material will deactivate any textures or material parameters
that get set up with materials.
???RC The behaviour of materials is set up to take advantage of pre-computed
display lists. To switch to direct rendering this routine should just call
direct_render_Graphical_material.
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
