/*******************************************************************************
FILE : cell_graphic.c

LAST MODIFIED : 8 March 2002

DESCRIPTION :
Routines for using the Cell_graphic objects
==============================================================================*/

#include <stdio.h>
#include <string.h>

#include "cell/cell_component.h"
#include "cell/cell_graphic.h"
#include "cell/cell_interface.h"

#include "graphics/graphics_library.h"
#include "graphics/material.h"
#include "graphics/colour.h"
#include "graphics/import_graphics_object.h"
#include "graphics/glyph.h"
#include "graphics/scene.h"

#include "selection/any_object_selection.h"

#include "general/indexed_list_private.h"
#include "general/compare.h"

/*
Module objects
--------------
*/
struct Cell_graphic
/*******************************************************************************
LAST MODIFIED : 18 November 2000

DESCRIPTION :
A data object used to store information about the graphics used to represent
a Cell_component object in the Cell 3D scene.
==============================================================================*/
{
  /* The graphic's ID, used for the indexed list because the name is
   * not guaranteed to be unique ??? probably is for graphics ???
   */
  int id;
  /* The access counter */
  int access_count;
  /* The graphics's name */
  char *name;
  /* The graphical object */
  struct GT_object *graphics_object;
  /* The graphic's material */
  struct Graphical_material *graphical_material;
}; /* struct Cell_graphic */

FULL_DECLARE_INDEXED_LIST_TYPE(Cell_graphic);

/*
Module functions
----------------
*/
/*DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Cell_component,id,int,compare_int)*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Cell_graphic,name,char *,strcmp)
  
/*
Global functions
----------------
*/

DECLARE_OBJECT_FUNCTIONS(Cell_graphic)
DECLARE_INDEXED_LIST_FUNCTIONS(Cell_graphic)
  /*DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Cell_component,id, \
    int,compare_int)*/
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Cell_graphic,name, \
  char *,strcmp)

struct Cell_graphic *CREATE(Cell_graphic)(char *name)
/*******************************************************************************
LAST MODIFIED : 18 November 2000

DESCRIPTION :
Creates a Cell_graphic object.
==============================================================================*/
{
  static int current_graphic_number = 0; /* used to assign id's to each
                                          * component as it is created */
  struct Cell_graphic *cell_graphic;

  ENTER(CREATE(Cell_graphic));
  if (name)
  {
    if (ALLOCATE(cell_graphic,struct Cell_graphic,1))
    {
      /* Assign an ID to each graphic as it is created to guarantee a unique
       * reference to each graphic object ????
       */
      current_graphic_number++;
      cell_graphic->id = current_graphic_number;
      /* initialise data objects */
      cell_graphic->access_count = 0;
      cell_graphic->name = (char *)NULL;
      cell_graphic->graphics_object = (struct GT_object *)NULL;
      cell_graphic->graphical_material = (struct Graphical_material *)NULL;
      /* Set the graphic's name */
      if (ALLOCATE(cell_graphic->name,char,strlen(name)+1))
      {
        strcpy(cell_graphic->name,name);
      }
      else
      {
        display_message(ERROR_MESSAGE,"CREATE(Cell_graphic).  "
          "Unable to allocate memory for the Cell graphic's name");
        DESTROY(Cell_graphic)(&cell_graphic);
        cell_graphic = (struct Cell_graphic *)NULL;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"CREATE(Cell_graphic).  "
        "Unable to allocate memory for the Cell graphic object");
      cell_graphic = (struct Cell_graphic *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Cell_graphic).  "
      "Invalid argument(s)");
    cell_graphic = (struct Cell_graphic *)NULL;
  }
  LEAVE;
  return(cell_graphic);
} /* CREATE(Cell_graphic)() */

int DESTROY(Cell_graphic)(struct Cell_graphic **cell_graphic_address)
/*******************************************************************************
LAST MODIFIED : 18 November 2000

DESCRIPTION :
Destroys a Cell_graphic object.
==============================================================================*/
{
	int return_code;
  struct Cell_graphic *cell_graphic;

	ENTER(DESTROY(Cell_graphic));
	if (cell_graphic_address && (cell_graphic = *cell_graphic_address))
	{
    if (cell_graphic->access_count == 0)
    {
      if (cell_graphic->name)
      {
        DEALLOCATE(cell_graphic->name);
      }
      if (cell_graphic->graphics_object)
      {
        DEACCESS(GT_object)(&(cell_graphic->graphics_object));
      }
      if (cell_graphic->graphical_material)
      {
        DEACCESS(Graphical_material)(&(cell_graphic->graphical_material));
      }
      DEALLOCATE(*cell_graphic_address);
      *cell_graphic_address = (struct Cell_graphic *)NULL;
      return_code=1;
    }
    else
    {
      display_message(WARNING_MESSAGE,"DESTROY(Cell_graphic).  "
        "Access count is not zero - cannot destroy object");
      return_code = 0;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cell_graphic).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* DESTROY(Cell_graphic)() */

int Cell_graphic_create_graphical_object(struct Cell_graphic *cell_graphic,
  struct Cell_cmgui_interface *cmgui_interface,char *obj_file)
/*******************************************************************************
LAST MODIFIED : 18 November 2000

DESCRIPTION :
Creates the <cell_graphic>'s graphical object. If <obj_file> is non-NULL, then
that OBJ file is used to create the graphical object - otherwise a sime arrow
is created.
==============================================================================*/
{
  int return_code = 0;
  struct File_read_graphics_object_from_obj_data *obj_data;
  struct LIST(GT_object) *graphics_list;

  ENTER(Cell_graphic_create_graphical_object);
  if (cell_graphic && cmgui_interface)
  {
    if (obj_file)
    {
      /* Create the graphics object from a wavefront obj file */
      if (ALLOCATE(obj_data,struct File_read_graphics_object_from_obj_data,1))
      {
        graphics_list = Cell_cmgui_interface_get_graphics_object_list(
          cmgui_interface);
        obj_data->object_list = graphics_list;
        obj_data->graphical_material_manager =
          Cell_cmgui_interface_get_graphical_material_manager(cmgui_interface);
        obj_data->time = 0.0;
        obj_data->graphics_object_name = cell_graphic->name;
        obj_data->render_type = RENDER_TYPE_SHADED;
        return_code = file_read_voltex_graphics_object_from_obj(obj_file,
          (void *)obj_data);
        if (return_code)
        {
          cell_graphic->graphics_object =
            ACCESS(GT_object)(FIND_BY_IDENTIFIER_IN_LIST(GT_object,
              name)(cell_graphic->name,graphics_list));
          REMOVE_OBJECT_FROM_LIST(GT_object)(cell_graphic->graphics_object,
            graphics_list);
        }
        else
        {
          display_message(ERROR_MESSAGE,"Cell_graphic_create_graphics_object.  "
            "Unable to read the wavefront obj file");
        }
        DEALLOCATE(obj_data);
      }
      else
      {
        display_message(ERROR_MESSAGE,"set_graphics_information. "
          "Unable to allocate memory for the wavefront obj");
        return_code = 0;
      }
    }
    else
    {
      /* Just create an arrow */
      cell_graphic->graphics_object = ACCESS(GT_object)(make_glyph_arrow_solid(
        cell_graphic->name,12,2.0/3.0,1.0/6.0));
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_graphic_create_graphical_object.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_graphic_create_graphical_object() */

int Cell_graphic_create_graphical_material(struct Cell_graphic *cell_graphic,
  struct Cell_cmgui_interface *cmgui_interface,
  char *diffuse_red,char *diffuse_green,char *diffuse_blue,
  char *ambient_red,char *ambient_green,char *ambient_blue,
  char *emission_red,char *emission_green,char *emission_blue,
  char *specular_red,char *specular_green,char *specular_blue,
  char *alpha,char *shininess)
/*******************************************************************************
LAST MODIFIED : 8 March 2002

DESCRIPTION :
Creates the material for the given <cell_graphic> and assigns it to the
graphic's graphical object.
==============================================================================*/
{
  int return_code = 0;
  float red,blue,green;
  struct Colour *colour;
  
  ENTER(Cell_graphic_create_graphical_material);
  if (cell_graphic && cmgui_interface)
  {
    /* create the material - if it doesn't already exist */
    cell_graphic->graphical_material =
      FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(cell_graphic->name,
        Cell_cmgui_interface_get_graphical_material_manager(cmgui_interface));
    if (cell_graphic->graphical_material || ((cell_graphic->graphical_material =
      ACCESS(Graphical_material)(CREATE(Graphical_material)(
        cell_graphic->name))) &&
      ADD_OBJECT_TO_MANAGER(Graphical_material)(
        cell_graphic->graphical_material,
        Cell_cmgui_interface_get_graphical_material_manager(cmgui_interface))))
    {
      /* Diffuse component */
      sscanf(diffuse_red,"%f",&red);
      sscanf(diffuse_green,"%f",&green);
      sscanf(diffuse_blue,"%f",&blue);
      colour = create_Colour(red,green,blue);
      Graphical_material_set_diffuse(cell_graphic->graphical_material,colour);
      destroy_Colour(&colour);
      /* Ambient component */
      sscanf(ambient_red,"%f",&red);
      sscanf(ambient_green,"%f",&green);
      sscanf(ambient_blue,"%f",&blue);
      colour = create_Colour(red,green,blue);
      Graphical_material_set_ambient(cell_graphic->graphical_material,colour);
      destroy_Colour(&colour);
      /* Emission component */
      sscanf(emission_red,"%f",&red);
      sscanf(emission_green,"%f",&green);
      sscanf(emission_blue,"%f",&blue);
      colour = create_Colour(red,green,blue);
      Graphical_material_set_emission(cell_graphic->graphical_material,colour);
      destroy_Colour(&colour);
      /* Emission component */
      sscanf(specular_red,"%f",&red);
      sscanf(specular_green,"%f",&green);
      sscanf(specular_blue,"%f",&blue);
      colour = create_Colour(red,green,blue);
      Graphical_material_set_specular(cell_graphic->graphical_material,colour);
      destroy_Colour(&colour);
      /* Alpha */
      sscanf(alpha,"%f",&red);
      Graphical_material_set_alpha(cell_graphic->graphical_material,red);
      /* Shininess */
      sscanf(shininess,"%f",&red);
      Graphical_material_set_shininess(cell_graphic->graphical_material,red);
      /* Compile the material */
      return_code = compile_Graphical_material(cell_graphic->graphical_material,
        (void *)NULL);
      if (return_code && cell_graphic->graphics_object)
      {
        /* Set the material of the graphical object */
        return_code = set_GT_object_default_material(
          cell_graphic->graphics_object,cell_graphic->graphical_material);
				/*???RC Note there should be no need to update materials in a voltex
					since they now use a NULL material to denote the use of the above
					default_material */
        if (!return_code)
				{
           display_message(ERROR_MESSAGE,
            "Cell_graphic_create_graphical_material.  "
            "Unable to update the material");
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "Cell_graphic_create_graphical_material.  "
          "Unable to compile the material");
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_graphic_create_graphical_material.  "
        "Unable to create the material");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_graphic_create_graphical_material.  "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_graphic_create_graphical_material() */

struct GT_object *Cell_graphic_get_graphics_object(
  struct Cell_graphic *cell_graphic)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Returns the graphics object for the given <cell_graphic>
==============================================================================*/
{
  struct GT_object *graphics_object;

  ENTER(Cell_graphic_get_graphics_object);
  if (cell_graphic)
  {
    graphics_object = cell_graphic->graphics_object;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_graphic_get_graphics_object.  "
      "Invalid argument(s)");
    graphics_object = (struct GT_object *)NULL;
  }
  LEAVE;
  return(graphics_object);
} /* Cell_graphic_get_graphics_object() */
