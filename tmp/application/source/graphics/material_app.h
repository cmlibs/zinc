
#include "api/cmiss_field.h"
#include "api/cmiss_graphics_module.h"
/*???DB.  Make consistent with finite_element.h ? */
//#define MATERIAL_PRECISION float
#define MATERIAL_PRECISION_STRING "lf"
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
struct Graphical_material;
int set_material_program_type(struct Graphical_material *material_to_be_modified,
	int bump_mapping_flag, int colour_lookup_red_flag, int colour_lookup_green_flag,
	int colour_lookup_blue_flag,  int colour_lookup_alpha_flag,
	int lit_volume_intensity_normal_texture_flag, int lit_volume_finite_difference_normal_flag,
	int lit_volume_scale_alpha_flag, int return_code);
/******************************************************************************
from the modify_graphical_material.
******************************************************************************/

int compile_Graphical_material_for_order_independent_transparency(
	struct Graphical_material *material,
	void *material_order_independent_data_void);
/*will work with order_independent_transparency.
==============================================================================*/

int set_Graphical_material(struct Parse_state *state,
	void *material_address_void,void *graphical_material_manager_void);
/*******************************************************************************
LAST MODIFIED : 20 June 1996

DESCRIPTION :
Modifier function to set the material from a command.
==============================================================================*/

int Option_table_add_set_Material_entry(
	struct Option_table *option_table, const char *token,
	struct Graphical_material **material, struct Material_package *material_package);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Adds the given <token> to the <option_table>.  The <material> is selected from
the <material_package> by name.
	struct Graphical_material* material, const char *uniform_name, float value);*/

