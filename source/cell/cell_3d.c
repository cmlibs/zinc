/*******************************************************************************
FILE : cell_3d.c

LAST MODIFIED : 28 August 2000

DESCRIPTION :
Functions for Cell 3d.
==============================================================================*/
#include <string.h>
#include "cell/cell_3d.h"
#include "cell/cell_component.h"
#include "cell/parameter_dialog.h"
#include "general/any_object.h"
#include "graphics/material.h"
#include "graphics/colour.h"
#include "graphics/import_graphics_object.h"
#include "graphics/glyph.h"
#include "finite_element/finite_element_to_graphics_object.h"

/*
Local types
===========
*/
struct Scene_picked_cell_object
/*******************************************************************************
LAST MODIFIED : 18 August 2000

DESCRIPTION :
Used to find the nearest cell object when a mouse button is pressed in the
cell scene
==============================================================================*/
{
  struct Scene_object *scene_object;
  double nearest;
}; /* Scene_picked_cell_object */

/*
Local functions
===============
*/

struct Cell_graphic *create_cell_graphic(char *type)
/*******************************************************************************
LAST MODIFIED : 28 August 2000

DESCRIPTION :
Create a Cell_graphic with the given <type>.
==============================================================================*/
{
  struct Cell_graphic *graphic = (struct Cell_graphic *)NULL;

  ENTER(create_cell_graphic);
  if (ALLOCATE(graphic,struct Cell_graphic,1))
  {
    if (ALLOCATE(graphic->type,char,strlen(type)+1))
    {
			strcpy(graphic->type,type);
      graphic->graphics_object = (struct GT_object *)NULL;
      graphic->graphical_material = (struct Graphical_material *)NULL;
      graphic->next = (struct Cell_graphic *)NULL;
    }
    else
    {
      display_message(ERROR_MESSAGE,"create_cell_graphic. "
        "Unable to allocate memory for the Cell_graphics type field");
      DEALLOCATE(graphic);
      graphic = (struct Cell_graphic *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_cell_graphic. "
      "Unable to allocate memory for the Cell_graphic");
    graphic = (struct Cell_graphic *)NULL;
  }
  LEAVE;
  return(graphic);
} /* END create_cell_graphic() */

struct Cell_graphic *assign_cell_graphic(struct Cell_window *cell,char *type)
/*******************************************************************************
LAST MODIFIED : 08 September 1999

DESCRIPTION :
If can't find a graphic of the given <type>, creates it.
==============================================================================*/
{
  struct Cell_graphic *graphic = (struct Cell_graphic *)NULL;
  struct Cell_graphic *current = (struct Cell_graphic *)NULL;
  int found;
  
  ENTER(assign_cell_graphic);
  if (cell != (struct Cell_window *)NULL)
  {
    found = 0;
    graphic = cell->graphics;
    while (!found && (graphic != (struct Cell_graphic *)NULL))
    {
      if (!strcmp(graphic->type,type))
      {
        found = 1;
      }
      else
      {
        graphic = graphic->next;
      }
    } /* while */
    if (!found)
    {
      /* required graphic does not exist, so create it */
      if (graphic = create_cell_graphic(type))
      {
        /* now add it to the end of the list */
        current = cell->graphics;
        while ((current != (struct Cell_graphic *)NULL) &&
          (current->next != (struct Cell_graphic *)NULL))
        {
          current = current->next;
        }
        if (current != (struct Cell_graphic *)NULL)
        {
          current->next = graphic;
        }
        else
        {
          cell->graphics = graphic;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"assign_cell_graphic. "
          "Unable to create a new Cell_graphic");
        graphic = (struct Cell_graphic *)NULL;
      }
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"assign_cell_graphic. "
      "Invalid arguments");
    graphic = (struct Cell_graphic *)NULL;
  }
  LEAVE;
  return(graphic);
} /* END assign_cell_graphic() */

/*
Global functions
================
*/
int set_component_graphical_information(struct Cell_window *cell,
  char *type,char *pos_x,char *pos_y,char *pos_z,char *dir_x,char *dir_y,
  char *dir_z,char *scale_x,char *scale_y,char *scale_z,char *name)
/*******************************************************************************
LAST MODIFIED : 08 September 1999

DESCRIPTION :
Find all Cell_components with their component field the same as the given <name>
and set their transformation matrix using the specified values. Also asigns
the Cell_graphic field of the matching components, creating a Cell_graphic of
the given <type> if one does not already exist.
==============================================================================*/
{
  int return_code = 0,i,found;
  float x,y,z;
  struct Cell_component *component = (struct Cell_component *)NULL;
  FE_value a[3],b[3],c[3],orientation[3],size[3],size1,size2,size3;
  
  ENTER(set_component_graphical_information);
  if (cell != (struct Cell_window *)NULL)
  {
    component = cell->components;
    found = 0;
    /* loop through all components and modify those with the same name */
    while (component != (struct Cell_component *)NULL)
    {
      if (!strcmp(component->name,name))
      {
        found = 1;
        /* set the data which will be used to create the transformation
        matrix */
        sscanf(dir_x,"%f",&x);
        sscanf(dir_y,"%f",&y);
        sscanf(dir_z,"%f",&z);
        orientation[0] = (FE_value)x;
        orientation[1] = (FE_value)y;
        orientation[2] = (FE_value)z;
        if (make_glyph_orientation_scale_axes(3,orientation,a,b,c,size))
        {
          sscanf(scale_x,"%f",&x);
          sscanf(scale_y,"%f",&y);
          sscanf(scale_z,"%f",&z);
          size1 = size[0] * (FE_value)x;
          size2 = size[1] * (FE_value)y;
          size3 = size[2] * (FE_value)z;
          for (i=0;i<3;i++)
          {
            a[i] *= size1;
            b[i] *= size2;
            c[i] *= size3;
            component->axis1[i] = a[i];
            component->axis2[i] = b[i];
            component->axis3[i] = c[i];
          }
          sscanf(pos_x,"%f",&x);
          sscanf(pos_y,"%f",&y);
          sscanf(pos_z,"%f",&z);
          component->point[0] = (FE_value)x;
          component->point[1] = (FE_value)y;
          component->point[2] = (FE_value)z;
          return_code = 1;
        }
        else
        {
          display_message(ERROR_MESSAGE,"set_component_graphical_information. "
            "Unable to make transformation information");
          return_code = 0;
        }
        /* assign the Cell_graphic */
        if (return_code &&
          (component->graphic = assign_cell_graphic(cell,type)))
        {
          return_code = 1;
        }
        else
        {
          display_message(ERROR_MESSAGE,"set_component_graphical_information. "
            "Unable to assign cell graphic");
          return_code = 0;
        }
      }
      component = component->next;
    }
    if (!found)
    {
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"set_component_graphical_information. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END set_component_graphical_information() */

int set_graphics_information(struct Cell_window *cell,char *filename,char *type,
  char *diffuse_red,char *diffuse_green,char *diffuse_blue,
  char *ambient_red,char *ambient_green,char *ambient_blue,
  char *emission_red,char *emission_green,char *emission_blue,
  char *specular_red,char *specular_green,char *specular_blue,
  char *alpha,char *shininess)
/*******************************************************************************
LAST MODIFIED : 08 September 1999

DESCRIPTION :
If a Cell_graphic exists of the same <type>, create the graphics object from
the given wavefront obj file <filename> and give it the specified material
properties. If <filename> is NULL, then a simple arrow is used.
==============================================================================*/
{
  int return_code = 0,found;
  float red,blue,green;
  struct Cell_graphic *graphic = (struct Cell_graphic *)NULL;
  struct File_read_graphics_object_from_obj_data *obj_data;
  struct Colour *colour;
  
  ENTER(set_graphics_information);
  if (cell && type)
  {
    graphic = cell->graphics;
    found = 0;
    while (!found && (graphic != (struct Cell_graphic *)NULL))
    {
      if (!strcmp(graphic->type,type))
      {
        found = 1;
      }
      else
      {
        graphic = graphic->next;
      }
    }
    /* only do something if the graphic already exists, i.e. the xml may define
       graphical objects which are not required for the current model */
    if (found)
    {
      if (filename)
      {
        /* create the graphics object from a wavefront obj file */
        if (ALLOCATE(obj_data,struct File_read_graphics_object_from_obj_data,1))
        {
          obj_data->object_list=(cell->cell_3d).graphics_object_list;
          obj_data->graphical_material_manager=
            (cell->cell_3d).graphical_material_manager;
          obj_data->time = 0.0;
          obj_data->graphics_object_name = type;
          obj_data->render_type = RENDER_TYPE_SHADED;
          return_code = file_read_voltex_graphics_object_from_obj(filename,
            (void *)obj_data);
          if (return_code)
          {
            graphic->graphics_object = FIND_BY_IDENTIFIER_IN_LIST(GT_object,
              name)(type,(cell->cell_3d).graphics_object_list);
          }
          else
          {
            display_message(ERROR_MESSAGE,"set_graphics_information. "
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
        /* just create an arrow */
        graphic->graphics_object = make_glyph_arrow_solid(type,12,2.0/3.0,
          1.0/6.0);
        return_code = 1;
      }
      if (return_code)
      {
        /* create the material */
        if (graphic->graphical_material = CREATE(Graphical_material)(type))
        {
          sscanf(diffuse_red,"%f",&red);
          sscanf(diffuse_green,"%f",&green);
          sscanf(diffuse_blue,"%f",&blue);
          colour = create_Colour(red,green,blue);
          Graphical_material_set_diffuse(graphic->graphical_material,colour);
          sscanf(ambient_red,"%f",&red);
          sscanf(ambient_green,"%f",&green);
          sscanf(ambient_blue,"%f",&blue);
          colour = create_Colour(red,green,blue);
          Graphical_material_set_ambient(graphic->graphical_material,colour);
          sscanf(emission_red,"%f",&red);
          sscanf(emission_green,"%f",&green);
          sscanf(emission_blue,"%f",&blue);
          colour = create_Colour(red,green,blue);
          Graphical_material_set_emission(graphic->graphical_material,colour);
          sscanf(specular_red,"%f",&red);
          sscanf(specular_green,"%f",&green);
          sscanf(specular_blue,"%f",&blue);
          colour = create_Colour(red,green,blue);
          Graphical_material_set_specular(graphic->graphical_material,colour);
          destroy_Colour(&colour);
          sscanf(alpha,"%f",&red);
          Graphical_material_set_alpha(graphic->graphical_material,red);
          sscanf(shininess,"%f",&red);
          Graphical_material_set_shininess(graphic->graphical_material,red);
          return_code = compile_Graphical_material(graphic->graphical_material,
            (void *)NULL);
          if (return_code)
          {
            return_code = set_GT_object_default_material(
              graphic->graphics_object,graphic->graphical_material);
            if (return_code)
            {
              if (graphic->graphics_object->object_type == g_VOLTEX)
              {
                /* need to update each triangle's material (since using a
                voltex graphics object) */
                update_GT_voltex_materials_to_default(graphic->graphics_object);
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,"set_graphics_information. "
                "Unable to update the material");
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"set_graphics_information. "
              "Unable to compile the material");
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"set_graphics_information. "
            "Unable to create the material");
          return_code = 0;
        }
      } /* if (return_code) */
    } /* if (found) */
    else
    {
      return_code = 1;
      display_message(INFORMATION_MESSAGE,"set_graphics_information. "
        "Ignoring graphic object: **%s**\n",type);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"set_graphics_information. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END set_graphics_information() */

int draw_cell_3d(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 25 August 2000

DESCRIPTION :
Draws all defined components into the Cell 3D scene and adds the manager
callbacks.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_component *component = (struct Cell_component *)NULL;
  struct Scene_object *scene_object;
  gtMatrix transformation;

  ENTER(draw_cell_3d);
  if (cell != (struct Cell_window *)NULL)
  {
    component = cell->components;
    return_code = 1;
    while ((component != (struct Cell_component *)NULL) && return_code)
    {
      if ((component->graphic != (struct Cell_graphic *)NULL) &&
        (component->graphic->graphics_object != (struct GT_object *)NULL))
      {
        /* draw the component into the scene */
        Scene_add_graphics_object((cell->cell_3d).scene,
          component->graphic->graphics_object,0,component->name,
					/*fast_changing*/0);
        /* grab the scene object */
        if (scene_object = Scene_get_scene_object_by_name(
          (cell->cell_3d).scene,component->name))
        {
          /* construct the transformation matrix */
          transformation[0][0] = component->axis1[0];
          transformation[0][1] = component->axis1[1];
          transformation[0][2] = component->axis1[2];
          transformation[0][3] = 0.0;
          transformation[1][0] = component->axis2[0];
          transformation[1][1] = component->axis2[1];
          transformation[1][2] = component->axis2[2];
          transformation[1][3] = 0.0;
          transformation[2][0] = component->axis3[0];
          transformation[2][1] = component->axis3[1];
          transformation[2][2] = component->axis3[2];
          transformation[2][3] = 0.0;
          transformation[3][0] = component->point[0];
          transformation[3][1] = component->point[1];
          transformation[3][2] = component->point[2];
          transformation[3][3] = 1.0;
          /* apply the transformation */
          return_code = Scene_object_set_transformation(scene_object,
            &transformation);
					/* make the scene_object represent the cell component */
					Scene_object_set_represented_object(scene_object,
						CREATE(ANY_OBJECT(Cell_component))(component));
        }
      }
      component = component->next;
    } /* while component */
  }
  else
  {
    display_message(ERROR_MESSAGE,"draw_cell_3d. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END draw_cell_3d() */

void destroy_cell_graphic(struct Cell_component *component)
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
Destroys the <component>'s cell graphic, if it has one.
==============================================================================*/
{
  ENTER(destroy_cell_graphic);
  if (component->graphic != (struct Cell_graphic *)NULL)
  {
    DESTROY(GT_object)(&(component->graphic->graphics_object));
    DESTROY(Graphical_material)(&(component->graphic->graphical_material));
    DEALLOCATE(component->graphic);
  }
  LEAVE;
} /* END destroy_cell_graphic() */

void cell_3d_component_selection_change(
	struct Any_object_selection *any_object_selection,
	struct Any_object_selection_changes *changes,
	void *cell_window_void)
/*******************************************************************************
LAST MODIFIED : 28 August 2000

DESCRIPTION :
Callback for change in the global selection of Any_objects - from which
Cell_component selections are interpreted.
==============================================================================*/
{
	ENTER(cell_3d_component_selection_change);
	if (any_object_selection&&changes&&cell_window_void)
	{
		FOR_EACH_OBJECT_IN_LIST(ANY_OBJECT(Cell_component))(
			bring_up_parameter_dialog_iterator,cell_window_void,
			changes->newly_selected_any_object_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cell_3d_component_selection_change.  Invalid argument(s)");
	}
	LEAVE;
} /* cell_3d_component_selection_change */

