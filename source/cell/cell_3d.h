/*******************************************************************************
FILE : cell_3d.h

LAST MODIFIED : 16 September 1999

DESCRIPTION :
Functions for Cell 3d.
==============================================================================*/
#if !defined (CELL_3D_H)
#define CELL_3D_H

#include "cell/cell_window.h"
#include "graphics/graphics_library.h"
#include "graphics/scene.h"

/*
Global types
============
*/
struct Cell_graphic
/*******************************************************************************
LAST MODIFIED : 08 September 1999

DESCRIPTION :
Stores information for a Cell graphical object
==============================================================================*/
{
  char *type;
  struct GT_object *graphics_object;
  struct Graphical_material *graphical_material;
  struct Cell_graphic *next;
}; /* struct Cell_graphic */

/*
Global functions
================
*/
int set_component_graphical_information(struct Cell_window *cell,
  char *type,char *pos_x,char *pos_y,char *pos_z,char *dir_x,char *dir_y,
  char *dir_z,char *scale_x,char *scale_y,char *scale_z,char *name);
/*******************************************************************************
LAST MODIFIED : 08 September 1999

DESCRIPTION :
Find all Cell_components with their component field the same as the given <name>
and set their transformation matrix using the specified values. Also asigns
the Cell_graphic field of the matching components, creating a Cell_graphic of
the given <type> if one does not already exist.
==============================================================================*/
int set_graphics_information(struct Cell_window *cell,char *filename,char *type,
  char *diffuse_red,char *diffuse_blue,char *diffuse_green,
  char *ambient_red,char *ambient_blue,char *ambient_green,
  char *emission_red,char *emission_blue,char *emission_green,
  char *specular_red,char *specular_blue,char *specular_green,
  char *alpha,char *shininess);
/*******************************************************************************
LAST MODIFIED : 08 September 1999

DESCRIPTION :
If a Cell_graphic exists of the same <type>, create the graphics object from
the given wavefront obj file <filename> and give it the specified material
properties. If <filename> is NULL, then a simple arrow is used.
==============================================================================*/
int draw_cell_3d(struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 08 September 1999

DESCRIPTION :
Draws all defined components into the Cell 3D scene and adds the manager
callbacks.
==============================================================================*/
void destroy_cell_graphic(struct Cell_component *component);
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
Destroys the <component>'s cell graphic, if it has one.
==============================================================================*/
void cell_3d_picking_callback(struct Scene *scene,void *cell_window,
  struct Scene_input_callback_data *scene_input_callback_data);
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
Receives mouse button press, motion and release events from <scene>, and
processes them to bring up the appropriate dialog box.
==============================================================================*/

#endif /* !defined (CELL_3D_H) */
