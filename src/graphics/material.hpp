/**
 * FILE : material.hpp
 *
 * Internal interfaces to graphical material and module.
 * Includes OpenGL rendering.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (MATERIAL_HPP)
#define MATERIAL_HPP

#include <list>
#include "opencmiss/zinc/zincconfigure.h"
#include "opencmiss/zinc/field.h"
#include "opencmiss/zinc/fieldimage.h"
#include "opencmiss/zinc/material.h"
#include "opencmiss/zinc/shader.h"
#include "general/callback_class.hpp"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/colour.h"
#include "graphics/spectrum.h"
#include "graphics/texture.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/shader_program.hpp"

/*
Global types
------------
*/

struct cmzn_context;
struct cmzn_graphics_module;

/** Pre-declared here to satisfy function prototype. */
struct IO_stream;

/**
 * A structure for storing related object for the texture of a material.
 * Each material consists four of this (multitexture support).
 */
struct Material_image_texture
{
	struct Texture *texture;
	cmzn_field_image_id field;
	void *callback_id;
	struct cmzn_material *material;
};

struct cmzn_material
/*******************************************************************************
LAST MODIFIED : 23 January 2004

DESCRIPTION :
The properties of a material.
==============================================================================*/
{
	/* the name of the material */
	const char *name;
	/* the colour of the background light (so scattered that its incident
		direction is unknown, comes from all directions) reflected */
	struct Colour ambient;
	/* the colour of the directional light scattered in all directions */
	struct Colour diffuse;
	/* the colour of the light emitted */
	struct Colour emission;
	/* the colour of the directional light which is reflected in a preferred
		direction */
	struct Colour specular;
	/* the transparency */
	MATERIAL_PRECISION alpha;
	/* how sharp and bright the glinting is */
	MATERIAL_PRECISION shininess;
#if defined (OPENGL_API)
	GLuint display_list;

	GLuint brightness_texture_id;
#endif /* defined (OPENGL_API) */
	/* enumeration indicates whether the graphics display list is up to date */
	enum Graphics_compile_status compile_status;
	/* the texture for this material */
	struct Material_image_texture image_texture;
	/* second stage multitexture (i.e. normals for bump mapping) */
	struct Material_image_texture second_image_texture;
	/* third stage multitexture */
	struct Material_image_texture third_image_texture;
	/* fourth stage multitexture */
	struct Material_image_texture fourth_image_texture;
	/* second stage multitexture (i.e. normals for bump mapping) */
	/* spectrum used to render this material */
	struct cmzn_spectrum *spectrum;
	/* callback if the spectrum changes */
	void *spectrum_manager_callback_id;
	/* the shared information for Graphical Materials, allowing them to share
	   cmzn_shaderprograms */
	struct cmzn_materialmodule *module;
	/* The normal calculated from the volume texture needs to be
		scaled similarly to how it is scaled into coordinate space,
		we do not take account of rotations or any other distortions.
		Four components as that is what ProgramEnvParameter4fvARB wants. */
	ZnReal lit_volume_normal_scaling[4];
	/* the graphics state program that represents this material */
	cmzn_shaderprogram_id program;
	int access_count, per_pixel_lighting_flag, bump_mapping_flag;
	/* this flag is for external API uses. If a material is set to be persistent
		 then this material will not be removed from the manager after destroy.
	 */
	bool isManagedFlag;
	int executed_as_order_independent;
	cmzn_shaderprogram_id order_program;
	cmzn_shaderuniforms_id uniforms;

public:

	/* after clearing in create, following to be modified only by manager */
	struct MANAGER(cmzn_material) *manager;
	int manager_change_status;

	inline cmzn_material *access()
	{
		++access_count;
		return this;
	}

	static void deaccess(cmzn_material*& material);

	int setName(const char *newName);
};

DECLARE_LIST_TYPES(cmzn_material);

DECLARE_MANAGER_TYPES(cmzn_material);

PROTOTYPE_OBJECT_FUNCTIONS(cmzn_material);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(cmzn_material);

PROTOTYPE_LIST_FUNCTIONS(cmzn_material);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(cmzn_material, name, const char*);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(cmzn_material, name, const char*);
PROTOTYPE_MANAGER_FUNCTIONS(cmzn_material);
PROTOTYPE_MANAGER_IDENTIFIER_WITHOUT_MODIFY_FUNCTIONS(cmzn_material, name, const char*);
PROTOTYPE_MANAGER_MODIFY_NOT_IDENTIFIER_FUNCTION(cmzn_material, name);

struct cmzn_materialmodulenotifier
{
private:
	cmzn_materialmodule_id module; // not accessed
	cmzn_materialmodulenotifier_callback_function function;
	void *user_data;
	int access_count;

	cmzn_materialmodulenotifier(cmzn_materialmodule *materialmodule);

	~cmzn_materialmodulenotifier();

public:

	/** private: external code must use cmzn_materialmodule_create_notifier */
	static cmzn_materialmodulenotifier *create(cmzn_materialmodule *materialmodule)
	{
		if (materialmodule)
			return new cmzn_materialmodulenotifier(materialmodule);
		return 0;
	}

	cmzn_materialmodulenotifier *access()
	{
		++(this->access_count);
		return this;
	}

	static int deaccess(cmzn_materialmodulenotifier* &notifier);

	int setCallback(cmzn_materialmodulenotifier_callback_function function_in,
		void *user_data_in);

	void *getUserData()
	{
		return this->user_data;
	}

	void clearCallback();

	void materialmoduleDestroyed();

	void notify(cmzn_materialmoduleevent *event)
	{
		if (this->function && event)
			(this->function)(event, this->user_data);
	}

};

struct cmzn_materialmoduleevent
{
private:
	cmzn_materialmodule *module;
	cmzn_material_change_flags changeFlags;
	struct MANAGER_MESSAGE(cmzn_material) *managerMessage;
	int access_count;

	cmzn_materialmoduleevent(cmzn_materialmodule *materialmoduleIn);
	~cmzn_materialmoduleevent();

public:

	/** @param materialmoduleIn  Owning materialmodule; can be NULL for FINAL event */
	static cmzn_materialmoduleevent *create(cmzn_materialmodule *materialmoduleIn)
	{
		return new cmzn_materialmoduleevent(materialmoduleIn);
	}

	cmzn_materialmoduleevent *access()
	{
		++(this->access_count);
		return this;
	}

	static int deaccess(cmzn_materialmoduleevent* &event);

	cmzn_material_change_flags getChangeFlags() const
	{
		return this->changeFlags;
	}

	void setChangeFlags(cmzn_material_change_flags changeFlagsIn)
	{
		this->changeFlags = changeFlagsIn;
	}

	cmzn_material_change_flags getMaterialChangeFlags(cmzn_material *material) const;

	struct MANAGER_MESSAGE(cmzn_material) *getManagerMessage();

	void setManagerMessage(struct MANAGER_MESSAGE(cmzn_material) *managerMessageIn);

	cmzn_materialmodule *getMaterialmodule()
	{
		return this->module;
	}
};

typedef std::list<cmzn_materialmodulenotifier *> cmzn_materialmodulenotifier_list;

struct cmzn_materialmodule
{

private:

	struct MANAGER(cmzn_material)* manager;
	void *manager_callback_id;
	cmzn_materialmodulenotifier_list notifier_list;
	cmzn_material* defaultMaterial;
	cmzn_material* defaultSelectedMaterial;
	cmzn_material* defaultSurfaceMaterial;
	cmzn_graphics_module* graphicsmodule;  // owning object - not accessed
	int access_count;

	cmzn_materialmodule();

	~cmzn_materialmodule();

	static void material_manager_change(
		struct MANAGER_MESSAGE(cmzn_material) *message, void *materialmodule_void);

public:

	/** first stage of creation; must also call setGraphicsmodule()
	 * @return  Accessed material module */
	static cmzn_materialmodule* create();

	/** define standard zinc materials beyond those made in create() */
	void defineStandardMaterials();

	/** second stage of creation */
	void setGraphicsmodule(cmzn_graphics_module* graphicsmoduleIn)
	{
		this->graphicsmodule = graphicsmoduleIn;
	}

	cmzn_materialmodule* access()
	{
		++access_count;
		return this;
	}

	static int deaccess(cmzn_materialmodule*& materialmodule);

	cmzn_context* getContext();

	struct MANAGER(cmzn_material)* getManager()
	{
		return this->manager;
	}

	struct MANAGER(cmzn_spectrum)* getSpectrumManager();

	int beginChange()
	{
		return MANAGER_BEGIN_CACHE(cmzn_material)(this->manager);
	}

	int endChange()
	{
		return MANAGER_END_CACHE(cmzn_material)(this->manager);
	}

	cmzn_material* createMaterial();

	cmzn_materialiterator* createMaterialiterator();

	/** @return  Non-accessed material, or nullptr if material with name not found */
	cmzn_material* findMaterialByName(const char* name)
	{
		return FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_material, name)(name, this->manager);
	}

	/** @return  Non-accessed material */
	cmzn_material* getDefaultMaterial() const
	{
		return this->defaultMaterial;
	}

	/** @param material  Must be valid. */
	void setDefaultMaterial(cmzn_material* material)
	{
		REACCESS(cmzn_material)(&this->defaultMaterial, material);
	}

	/** @return  Non-accessed material */
	cmzn_material* getDefaultSelectedMaterial() const
	{
		return this->defaultSelectedMaterial;
	}

	/** @param material  Must be valid. */
	void setDefaultSelectedMaterial(cmzn_material* material)
	{
		REACCESS(cmzn_material)(&this->defaultSelectedMaterial, material);
	}

	/** @return  Non-accessed material. Default is a nullptr. */
	cmzn_material* getDefaultSurfaceMaterial() const
	{
		return this->defaultSurfaceMaterial;
	}

	/** @param material  Can be nullptr. */
	void setDefaultSurfaceMaterial(cmzn_material* material)
	{
		REACCESS(cmzn_material)(&this->defaultSurfaceMaterial, material);
	}

	void addNotifier(cmzn_materialmodulenotifier *notifier);

	void removeNotifier(cmzn_materialmodulenotifier *notifier);

};

class Render_graphics_opengl;

/** Data for compiling materials specially for order independent transparency. */
struct Material_order_independent_transparency
{
	int layer;
	Render_graphics_opengl* renderer;
};

/*
Global functions
----------------
*/

cmzn_material *cmzn_material_create_private();

int DESTROY(cmzn_material)(struct cmzn_material **material_address);
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Frees the memory for the material and sets <*material_address> to NULL.
==============================================================================*/

const char *Graphical_material_name(cmzn_material *material);
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
While the GET_NAME macro returns a copy of the name of an object, this function
has been created for returning just a pointer to the material's name, or some
other string if the name is invalid, suitable for putting in printf statements.
Be careful with the returned value: esp. do not modify or DEALLOCATE it!
==============================================================================*/

int Graphical_material_get_ambient(cmzn_material *material,
	struct Colour *diffuse);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the ambient colour of the material.
==============================================================================*/

int Graphical_material_set_ambient(cmzn_material *material,
	struct Colour *diffuse);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the ambient colour of the material.
==============================================================================*/

int Graphical_material_get_diffuse(cmzn_material *material,
	struct Colour *diffuse);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the diffuse colour of the material.
==============================================================================*/

int Graphical_material_set_diffuse(cmzn_material *material,
	struct Colour *diffuse);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the diffuse colour of the material.
==============================================================================*/

int Graphical_material_get_emission(cmzn_material *material,
	struct Colour *emission);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the emission colour of the material.
==============================================================================*/

int Graphical_material_set_emission(cmzn_material *material,
	struct Colour *emission);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the emission colour of the material.
==============================================================================*/

int Graphical_material_get_specular(cmzn_material *material,
	struct Colour *emission);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the specular colour of the material.
==============================================================================*/

int Graphical_material_set_specular(cmzn_material *material,
	struct Colour *emission);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the specular colour of the material.
==============================================================================*/

int Graphical_material_get_alpha(cmzn_material *material,
	MATERIAL_PRECISION *alpha);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the alpha value of the material.
==============================================================================*/

int Graphical_material_set_alpha(cmzn_material *material,
	MATERIAL_PRECISION alpha);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the alpha value of the material.
==============================================================================*/

int Graphical_material_get_shininess(cmzn_material *material,
	MATERIAL_PRECISION *shininess);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the shininess value of the material.
==============================================================================*/

int Graphical_material_set_shininess(cmzn_material *material,
	MATERIAL_PRECISION shininess);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the shininess value of the material.
==============================================================================*/

int Graphical_material_set_colour_lookup_spectrum(cmzn_material *material,
	struct cmzn_spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 6 October 2006

DESCRIPTION :
Sets the spectrum member of the material.
==============================================================================*/

struct cmzn_spectrum *Graphical_material_get_colour_lookup_spectrum(
	cmzn_material *material);
/*******************************************************************************
LAST MODIFIED : 6 October 2006

DESCRIPTION :
Returns the spectrum member of the material.
==============================================================================*/

struct Texture *Graphical_material_get_texture(
	cmzn_material *material);
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Returns the texture member of the material.
==============================================================================*/

struct Texture *Graphical_material_get_second_texture(
	 cmzn_material *material);
/*******************************************************************************
LAST MODIFIED : 5 Dec 2007

DESCRIPTION :
Returns the second texture of the material.
==============================================================================*/

struct Texture *Graphical_material_get_third_texture(
	 cmzn_material *material);
/*******************************************************************************
LAST MODIFIED : 5 Dec 2007

DESCRIPTION :
Returns the third texture of the material.
==============================================================================*/


struct Texture *Graphical_material_get_fourth_texture(
	 cmzn_material *material);
/*******************************************************************************
LAST MODIFIED : 5 Dec 2007

DESCRIPTION :
Returns the fourth texture of the material.
==============================================================================*/


int Graphical_material_get_bump_mapping_flag(cmzn_material *material);
/*******************************************************************************
LAST MODIFIED : 5 December 2007

DESCRIPTION :
Returns the flag set for bump_mapping.
==============================================================================*/

int Graphical_material_get_per_pixel_lighting_flag(cmzn_material *material);
/*******************************************************************************
LAST MODIFIED : 5 December 2007

DESCRIPTION :
Returns the flag set for per_pixel_lighting.
==============================================================================*/

int list_Graphical_material(cmzn_material *material,void *dummy);
/*******************************************************************************
LAST MODIFIED : 22 September 1998

DESCRIPTION :
Writes the properties of the <material> to the command window.
==============================================================================*/

int list_Graphical_material_commands(cmzn_material *material,
	void *command_prefix_void);
/*******************************************************************************
LAST MODIFIED : 22 September 1998

DESCRIPTION :
Writes on the command window the commands needed to recreate the <material>.
The command is started with the string pointed to by <command_prefix>.
==============================================================================*/

int write_Graphical_material_commands_to_comfile(cmzn_material *material,
	 void *command_prefix_void);
/*******************************************************************************
LAST MODIFIED : 10 August 2007

DESCRIPTION :
Writes on the command window the command needed to recreate the <material>.
The command is started with the string pointed to by <command_prefix>.
==============================================================================*/

int file_read_Graphical_material_name(struct IO_stream *file,
	cmzn_material **material_address,
	struct MANAGER(cmzn_material) *graphical_material_manager);
/*******************************************************************************
LAST MODIFIED : 6 December 2004

DESCRIPTION :
Reads a material name from a <file>.  Searchs the list of all materials for one
with the specified name.  If one is not found a new one is created with the
specified name and the default properties.
==============================================================================*/

int set_shader_program_type(cmzn_shadermodule_id shadermodule, cmzn_material *material_to_be_modified,
	 int bump_mapping_flag, int colour_lookup_red_flag, int colour_lookup_green_flag,
	 int colour_lookup_blue_flag,  int colour_lookup_alpha_flag,
	 int lit_volume_intensity_normal_texture_flag, int lit_volume_finite_difference_normal_flag,
	 int lit_volume_scale_alpha_flag, int return_code);
/******************************************************************************
LAST MODIFIED : 4 Dec 2007

DESCRIPTION : Set up the shader program type for using the vertex
and fragment program. This and following functions are orginally
from the modify_graphical_material.
NOTE: I use the pointer to the materialmodule from the material.
==============================================================================*/

int material_copy_bump_mapping_and_per_pixel_lighting_flag(cmzn_material *material,
	 cmzn_material *material_to_be_modified);
/******************************************************************************
LAST MODIFIED : 5 Dec 2007

DESCRIPTION : This function will set the bump mapping and per
pixel_lighting_flag of the material_to_be_modified to be the same as
the one in material, it is used for setting up the GUI.
==============================================================================*/

int compile_Graphical_material_for_order_independent_transparency(cmzn_material *material,
	void *material_order_independent_data_void);
/*******************************************************************************
LAST MODIFIED : 2 May 2005

DESCRIPTION :
Recompile each of the <materials> which have already been compiled so that they
will work with order_independent_transparency.
==============================================================================*/

int Material_render_opengl(cmzn_material* material,
	Render_graphics_opengl* renderer);

int Material_compile_members_opengl(cmzn_material* material,
	Render_graphics_opengl* renderer);

int Material_compile_opengl_display_list(cmzn_material* material,
	Callback_base< cmzn_material* >* execute_function,
	Render_graphics_opengl* renderer);

int Material_execute_opengl_display_list(cmzn_material* material,
	Render_graphics_opengl* renderer);

int cmzn_material_shaderprogram_changed(cmzn_material *material, void *message_void);

int cmzn_material_shaderuniforms_changed(cmzn_material *material, void *message_void);

/***************************************************************************//**
 * Sets the Graphics_module object which will own this manager.
 * Private! Only to be called only from Graphics_module object.
 *
 * @param manager  Material manager.
 * @return  The owning Graphics_module object.
 */
int Material_manager_set_owner(struct MANAGER(cmzn_material) *manager,
	struct cmzn_materialmodule *materialmodule);

int material_deaccess_shader_program(cmzn_material *material_to_be_modified);

int Material_set_shader_program_strings(cmzn_shadermodule_id shadermodule,
	cmzn_material *material_to_be_modified, char *vertex_program_string,
	char *fragment_program_string, char *geometry_program_string);

struct cmzn_materialmodule *manager_get_owner_cmzn_material(manager_cmzn_material *manager);

/** non-accessing private variant of cmzn_materialiterator_next */
cmzn_material_id cmzn_materialiterator_next_non_access(cmzn_materialiterator_id iterator);

#endif
