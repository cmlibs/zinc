/*******************************************************************************
FILE : cell_component.c

LAST MODIFIED : 19 June 2001

DESCRIPTION :
Routines for using the Cell_component objects
==============================================================================*/

#include <stdio.h>
#include <string.h>

#include "cell/cell_component.h"
#include "cell/cell_interface.h"
#include "cell/cell_variable.h"

#include "general/indexed_list_private.h"
#include "general/compare.h"
#include "general/any_object_definition.h"

/*
Module objects
--------------
*/
struct Cell_component
/*******************************************************************************
LAST MODIFIED : 18 November 2000

DESCRIPTION :
A data object used to store information about any object from the CellML which
the user may decide to display and interact with in the Cell interface.
==============================================================================*/
{
  /* The component's ID, used for the indexed list because the name is
   * not guaranteed to be unique
   */
  int id;
  /* The access counter */
  int access_count;
  /* The component's name */
  char *name;
  /* A more descriptive name for the component */
  char *display_name;
  /* A list of all the variables associated with the component */
  struct LIST(Cell_variable) *variable_list;
  /* A list of all the variables "exported" from the component */
  struct LIST(Cell_variable) *exported_variable_list;
  /* A list of all the variables "imported" from the component */
  struct LIST(Cell_variable) *imported_variable_list;
  /* The parent component */
  struct Cell_component *parent;
  /* The children components */
  struct LIST(Cell_component) *children_list;
  /* The information for placing the component's graphic */
  FE_value axis1[3],axis2[3],axis3[3],point[3];
  /* And the graphic */
  struct Cell_graphic *graphic;
}; /* struct Cell_component */

FULL_DECLARE_INDEXED_LIST_TYPE(Cell_component);

/*
Module functions
----------------
*/
/*DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Cell_component,id,int,compare_int)*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Cell_component,name,char *,strcmp)

static void Cell_component_list_full(struct Cell_component *cell_component)
/*******************************************************************************
LAST MODIFIED : 03 April 2001

DESCRIPTION :
Does a full listing of the <cell_component>.
==============================================================================*/
{
  ENTER(Cell_component_list_full);
  if (cell_component)
  {
    display_message(INFORMATION_MESSAGE,"Component: %s\n",
      cell_component->name);
    display_message(INFORMATION_MESSAGE,"  Display name: %s\n",
      cell_component->display_name);
    display_message(INFORMATION_MESSAGE,"  ID: %d\n",cell_component->id);
    display_message(INFORMATION_MESSAGE,"  access count: %d\n",
      cell_component->access_count);
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_component_list_full.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* Cell_component_list_full() */
  
static void Cell_component_list_brief(struct Cell_component *cell_component,
  void *indent_level_void)
/*******************************************************************************
LAST MODIFIED : 03 April 2001

DESCRIPTION :
Does a brief listing of the <cell_component>. Can be used as an iterator
function.
==============================================================================*/
{
  int *indent_level = (int *)indent_level_void;
  
  ENTER(Cell_component_list_brief);
  if (cell_component)
  {
    WRITE_INDENT(*indent_level);
    display_message(INFORMATION_MESSAGE,"Component: %s\n",
      cell_component->name);
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_component_list_brief.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* Cell_component_list_brief() */
  
/*
Global functions
----------------
*/

DEFINE_ANY_OBJECT(Cell_component)

DECLARE_OBJECT_FUNCTIONS(Cell_component)
DECLARE_INDEXED_LIST_FUNCTIONS(Cell_component)
  /*DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Cell_component,id, \
    int,compare_int)*/
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Cell_component,name, \
  char *,strcmp)

struct Cell_component *CREATE(Cell_component)(struct Cell_component *parent,
  char *name)
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
Creates a Cell_component object.
==============================================================================*/
{
  static int current_component_number; /* used to assign id's to each component
                                        * as it is created */
  struct Cell_component *cell_component;
  int i;

  ENTER(CREATE(Cell_component));
  if (name)
  {
    if (ALLOCATE(cell_component,struct Cell_component,1))
    {
      /* Assign an ID to each component as it is created to guarantee a unique
       * reference to each compnent object
       */
      current_component_number++;
      cell_component->id = current_component_number;
      /* initialise data objects */
      cell_component->access_count = 0;
      cell_component->name = (char *)NULL;
      cell_component->display_name = (char *)NULL;
      cell_component->parent = (struct Cell_component *)NULL;
      cell_component->variable_list = (struct LIST(Cell_variable) *)NULL;
      cell_component->exported_variable_list =
        (struct LIST(Cell_variable) *)NULL;
      cell_component->imported_variable_list =
        (struct LIST(Cell_variable) *)NULL;
      cell_component->children_list = (struct LIST(Cell_component) *)NULL;
      for (i=0;i<3;i++)
      {
        cell_component->axis1[i] = (FE_value)0.0;
        cell_component->axis2[i] = (FE_value)0.0;
        cell_component->axis3[i] = (FE_value)0.0;
        cell_component->point[i] = (FE_value)0.0;
      }
      cell_component->graphic = (struct Cell_graphic *)NULL;
      if (cell_component->variable_list = CREATE(LIST(Cell_variable))())
      {
        if (cell_component->children_list = CREATE(LIST(Cell_component))())
        {
          /* set the name of the component */
          Cell_component_set_name(cell_component,name);
          /* set the parent of the component */
          cell_component->parent = parent;
          /* add the new component to the parent's list of chilren */
          if (parent)
          {
            /* can have a NULL parent!! */
            Cell_component_add_child_to_children_list(parent,
              cell_component);
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"CREATE(Cell_component).  "
            "Unable to create the children list");
          DESTROY(Cell_component)(&cell_component);
          cell_component = (struct Cell_component *)NULL;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"CREATE(Cell_component).  "
          "Unable to create the variable list");
        DESTROY(Cell_component)(&cell_component);
        cell_component = (struct Cell_component *)NULL;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"CREATE(Cell_component).  "
        "Unable to allocate memory for the Cell component object");
      cell_component = (struct Cell_component *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Cell_component).  "
      "Invalid argument(s)");
    cell_component = (struct Cell_component *)NULL;
  }
  LEAVE;
  return(cell_component);
} /* CREATE(Cell_component) */

int DESTROY(Cell_component)(struct Cell_component **cell_component_address)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Destroys a Cell_component object.
==============================================================================*/
{
	int return_code;
  struct Cell_component *cell_component;

	ENTER(DESTROY(Cell_component));
	if (cell_component_address && (cell_component = *cell_component_address))
	{
    if (cell_component->access_count == 0)
    {
      if (cell_component->variable_list)
      {
        /* Removing the objects from the list will cause them to be
           destroy'ed */
        REMOVE_ALL_OBJECTS_FROM_LIST(Cell_variable)
          (cell_component->variable_list);
        /* and destroy the list */
        DESTROY(LIST(Cell_variable))(&(cell_component->variable_list));
      }
      if (cell_component->exported_variable_list)
      {
        /* Removing the objects from the list will cause them to be
           destroy'ed */
        REMOVE_ALL_OBJECTS_FROM_LIST(Cell_variable)
          (cell_component->exported_variable_list);
        /* and destroy the list */
        DESTROY(LIST(Cell_variable))(&(cell_component->exported_variable_list));
      }
      if (cell_component->imported_variable_list)
      {
        /* Removing the objects from the list will cause them to be
           destroy'ed */
        REMOVE_ALL_OBJECTS_FROM_LIST(Cell_variable)
          (cell_component->imported_variable_list);
        /* and destroy the list */
        DESTROY(LIST(Cell_variable))(&(cell_component->imported_variable_list));
      }
      if (cell_component->children_list)
      {
        /* Removing the objects from the list will cause them to be
           destroy'ed */
        REMOVE_ALL_OBJECTS_FROM_LIST(Cell_component)
          (cell_component->children_list);
        /* and destroy the list */
        DESTROY(LIST(Cell_component))(&(cell_component->children_list));
      }
      if (cell_component->name)
      {
        DEALLOCATE(cell_component->name);
      }
      if (cell_component->display_name)
      {
        DEALLOCATE(cell_component->display_name);
      }
      DEALLOCATE(*cell_component_address);
      *cell_component_address = (struct Cell_component *)NULL;
      return_code=1;
    }
    else
    {
      display_message(WARNING_MESSAGE,"DESTROY(Cell_component).  "
        "Access count is not zero - cannot destroy object");
      return_code = 0;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cell_component).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* DESTROY(Cell_component) */

int Cell_component_list(struct Cell_component *cell_component,void *full_void)
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Iterator function used to list out the current components. If <full> is not 0,
then a full listing of the <cell_component> is given, otherwise just a brief
listing.
==============================================================================*/
{
  int return_code = 0;
  int *full = (int *)full_void;
  int indent_level = 0;

  ENTER(Cell_component_list);
  if (cell_component)
  {
    if (*full)
    {
      /* do a full listing */
      Cell_component_list_full(cell_component);
    }
    else
    {
      /* do a brief listing */
      Cell_component_list_brief(cell_component,(void *)(&indent_level));
    }
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_component_list.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_component_list() */

int Cell_component_destroy_variable_list(struct Cell_component *cell_component,
  void *unused_void)
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Iterator function used to remove all the variable references in the variable
list of the <cell_component>
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_component_destroy_variable_list);
  USE_PARAMETER(unused_void);
  if (cell_component)
  {
    REMOVE_ALL_OBJECTS_FROM_LIST(Cell_variable)(cell_component->variable_list);
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_component_destroy_variable_list.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_component_destroy_variable_list() */

int Cell_component_set_name(struct Cell_component *cell_component,
  char *name)
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Sets the <name> of the <cell_component> - copies the <name> so the calling
routine should deallocate it.
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(Cell_component_set_name);
  if (cell_component && name)
  {
    if (ALLOCATE(cell_component->name,char,strlen(name)+1))
    {
      strcpy(cell_component->name,name);
/*        sprintf(cell_component->name,"%s\0",name); */
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_component_set_name.  "
        "Unable to allocate memory for the name");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_component_set_name.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_component_set_name() */

int Cell_component_set_display_name(struct Cell_component *cell_component,
  char *display_name)
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Sets the <display_name> of the <cell_component> - copies the <display_name>
so the calling routine should deallocate it.
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(Cell_component_set_display_name);
  if (cell_component && display_name)
  {
    if (ALLOCATE(cell_component->display_name,char,strlen(display_name)+1))
    {
      strcpy(cell_component->display_name,display_name);
/*        sprintf(cell_component->display_name,"%s\0",display_name); */
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_component_set_display_name.  "
        "Unable to allocate memory for the display name");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_component_set_display_name.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_component_set_display_name() */

int Cell_component_add_variable_to_variable_list(
  struct Cell_component *cell_component,
  struct Cell_variable *cell_variable)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
Adds the <cell_variable> to the <cell_component>'s variable list.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_component_add_variable_to_variable_list);
  if (cell_component && cell_variable)
  {
    if (ADD_OBJECT_TO_LIST(Cell_variable)(cell_variable,
      cell_component->variable_list))
    {
      return_code = 1;
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "Cell_component_add_variable_to_variable_list.  "
        "Unable to add the variable to the variable list");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_component_add_variable_to_variable_list.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_component_add_variable_to_variable_list() */

int Cell_component_add_variable_to_exported_variable_list(
  struct Cell_component *cell_component,
  struct Cell_variable *cell_variable)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Adds the <cell_variable> to the <cell_component>'s exported variable list.
Creates the list if this is the first variable to be added to the list.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_component_add_variable_to_exported_variable_list);
  if (cell_component && cell_variable)
  {
    if (cell_component->exported_variable_list ==
      (struct LIST(Cell_variable) *)NULL)
    {
      /* need to create the list */
      if (cell_component->exported_variable_list =
        CREATE(LIST(Cell_variable))())
      {
        /* do nothing */
      }
      else
      {
        cell_component->exported_variable_list =
          (struct LIST(Cell_variable) *)NULL;
      }
    }
    if (cell_component->exported_variable_list)
    {
      if (ADD_OBJECT_TO_LIST(Cell_variable)(cell_variable,
        cell_component->exported_variable_list))
      {
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "Cell_component_add_variable_to_exported_variable_list.  "
          "Unable to add the variable to the exported variable list");
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "Cell_component_add_variable_to_exported_variable_list.  "
        "Invalid exported variable list");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_component_add_variable_to_exported_variable_list.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_component_add_variable_to_exported_variable_list() */

int Cell_component_add_variable_to_imported_variable_list(
  struct Cell_component *cell_component,
  struct Cell_variable *cell_variable)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Adds the <cell_variable> to the <cell_component>'s imported variable list.
Creates the list if this is the first variable to be added to the list.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_component_add_variable_to_imported_variable_list);
  if (cell_component && cell_variable)
  {
    if (cell_component->imported_variable_list ==
      (struct LIST(Cell_variable) *)NULL)
    {
      /* need to create the list */
      if (cell_component->imported_variable_list =
        CREATE(LIST(Cell_variable))())
      {
        /* do nothing */
      }
      else
      {
        cell_component->imported_variable_list =
          (struct LIST(Cell_variable) *)NULL;
      }
    }
    if (cell_component->imported_variable_list)
    {
      if (ADD_OBJECT_TO_LIST(Cell_variable)(cell_variable,
        cell_component->imported_variable_list))
      {
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "Cell_component_add_variable_to_imported_variable_list.  "
          "Unable to add the variable to the imported variable list");
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "Cell_component_add_variable_to_imported_variable_list.  "
        "Invalid imported variable list");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_component_add_variable_to_imported_variable_list.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_component_add_variable_to_imported_variable_list() */

int Cell_component_add_child_to_children_list(
  struct Cell_component *cell_component,
  struct Cell_component *child)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Adds the <child> to the <cell_component>'s list of children
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_component_add_child_to_children_list);
  if (cell_component && child)
  {
    if (ADD_OBJECT_TO_LIST(Cell_component)(child,
      cell_component->children_list))
    {
      return_code = 1;
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "Cell_component_add_child_to_children_list.  "
        "Unable to add the child to the children list");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_component_add_child_to_children_list.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_component_add_child_to_children_list() */

int Cell_component_list_component_hierarchy(
  struct Cell_component *cell_component,void *indent_level_void)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Lists out the given <cell_component>'s hierarchy.
==============================================================================*/
{
  int return_code = 0;
  int *indent_level = (int *)indent_level_void;
  int indent;

  ENTER(Cell_component_list_component_hierarchy);
  if (cell_component)
  {
    indent = *indent_level;
    /* write out the current component */
    Cell_component_list_brief(cell_component,(void *)(&indent));
    indent += 2;
    /* and all of its children */
    FOR_EACH_OBJECT_IN_LIST(Cell_component)(
      Cell_component_list_component_hierarchy,(void *)(&indent),
      cell_component->children_list);
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_component_list_component_hierarchy.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_component_list_component_hierarchy() */

struct Cell_variable *Cell_component_get_cell_variable_by_name(
  struct Cell_component *cell_component,char *name)
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Returns the cell variable given by <name> from the <cell_component>'s variable
list - or NULL if a coresponding variable is not found.
==============================================================================*/
{
  struct Cell_variable *cell_variable = (struct Cell_variable *)NULL;
  
  ENTER(Cell_component_get_cell_variable_by_name);
  if (cell_component && name)
  {
    cell_variable =
      FIND_BY_IDENTIFIER_IN_LIST(Cell_variable,name)(
        name,cell_component->variable_list);
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_component_get_cell_variable_by_name.  "
      "Invalid argument(s)");
    cell_variable = (struct Cell_variable *)NULL;
  }
  LEAVE;
  return(cell_variable);
} /* Cell_component_get_cell_variable_by_name() */

char *Cell_component_get_name(struct Cell_component *cell_component)
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Returns a copy of the <cell_component>'s name.
==============================================================================*/
{
  char *name;
  
  ENTER(Cell_component_get_name);
  if (cell_component && cell_component->name)
  {
    if (ALLOCATE(name,char,strlen(cell_component->name)+1))
    {
      strcpy(name,cell_component->name);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_component_get_name.  "
        "Unable to allocate memory for the name");
      name = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_component_get_name.  "
      "Invalid argument(s)");
    name = (char *)NULL;
  }
  LEAVE;
  return(name);
} /* Cell_component_get_name() */

char *Cell_component_get_display_name(struct Cell_component *cell_component)
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Returns a copy of the <cell_component>'s display name.
==============================================================================*/
{
  char *display_name;
  
  ENTER(Cell_component_get_display_name);
  if (cell_component)
  {
    if (cell_component->display_name)
    {
      if (ALLOCATE(display_name,char,strlen(cell_component->display_name)+1))
      {
        strcpy(display_name,cell_component->display_name);
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_component_get_display_name.  "
          "Unable to allocate memory for the name");
        display_name = (char *)NULL;
      }
    }
    else
    {
      /* Not really an error to have no display name */
      display_name = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_component_get_display_name.  "
      "Invalid argument(s)");
    display_name = (char *)NULL;
  }
  LEAVE;
  return(display_name);
} /* Cell_component_get_display_name() */

int Cell_component_component_has_children(struct Cell_component *cell_component)
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Returns 1 if the <cell_component> has any children components, otherwise 0.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_component_component_has_children);
  if (cell_component)
  {
    if (NUMBER_IN_LIST(Cell_component)(cell_component->children_list) > 0)
    {
      return_code = 1;
    }
    else
    {
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_component_component_has_children.  "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_component_component_has_children() */

struct LIST(Cell_component) *Cell_component_get_children_list(
  struct Cell_component *cell_component)
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Returns the children list of the given <cell_component>
==============================================================================*/
{
  struct LIST(Cell_component) *children_list;

  ENTER(Cell_component_get_children_list);
  if (cell_component)
  {
    children_list = cell_component->children_list;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_component_get_children_list.  "
      "Invalid arguments");
    children_list = (struct LIST(Cell_component) *)NULL;
  }
  LEAVE;
  return(children_list);
} /* Cell_component_get_children_list() */

struct LIST(Cell_variable) *Cell_component_get_variable_list(
  struct Cell_component *cell_component)
/*******************************************************************************
LAST MODIFIED : 08 November 2000

DESCRIPTION :
Returns the variable list of the given <cell_component>
==============================================================================*/
{
  struct LIST(Cell_variable) *variable_list;

  ENTER(Cell_component_get_variable_list);
  if (cell_component)
  {
    variable_list = cell_component->variable_list;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_component_get_variable_list.  "
      "Invalid arguments");
    variable_list = (struct LIST(Cell_variable) *)NULL;
  }
  LEAVE;
  return(variable_list);
} /* Cell_component_get_variable_list() */

int Cell_component_set_graphical_transformation(
  struct Cell_component *component,char *pos_x,char *pos_y,char *pos_z,
  char *dir_x,char *dir_y,char *dir_z,char *scale_x,char *scale_y,
  char *scale_z)
/*******************************************************************************
LAST MODIFIED : 18 November 2000

DESCRIPTION :
Sets the transformation to be used for the graphical representation of the
given <component>
==============================================================================*/
{
  int return_code = 0,i;
  float x,y,z;
  FE_value a[3],b[3],c[3],orientation[3],size[3],size1,size2,size3;
  
  ENTER(Cell_component_set_graphical_transformation);
  if ((component != (struct Cell_component *)NULL) && pos_x && pos_y &&
    pos_z && dir_x && dir_y && dir_z && scale_x && scale_y && scale_z)
  {
    /* Set the data which will be used to create the transformation matrix */
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
      display_message(ERROR_MESSAGE,
        "Cell_component_set_graphical_transformation. "
        "Unable to make transformation information");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_component_set_graphical_transformation. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_component_set_graphical_transformation() */

int Cell_component_set_graphic(struct Cell_component *cell_component,
  struct Cell_graphic *cell_graphic)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Sets the <cell_graphic> for the given <cell_component>
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_component_set_graphic);
  if (cell_component && cell_graphic)
  {
    cell_component->graphic = cell_graphic;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_component_set_graphic.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_component_set_graphic() */

struct Cell_graphic *Cell_component_get_graphic(
  struct Cell_component *cell_component)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Gets the Cell graphic for the given <cell_component>
==============================================================================*/
{
  struct Cell_graphic *graphic;

  ENTER(Cell_component_get_graphic);
  if (cell_component)
  {
    graphic = cell_component->graphic;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_component_get_graphic.  "
      "Invalid argument(s)");
    graphic = (struct Cell_graphic *)NULL;
  }
  LEAVE;
  return(graphic);
} /* Cell_component_get_graphic() */

FE_value Cell_component_get_axis1(struct Cell_component *component,int index)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Returns the value of axis1[<index>]
==============================================================================*/
{
  FE_value value;

  ENTER(Cell_component_get_axis1);
  if (component && (index >= 0) && (index < 3))
  {
    value = component->axis1[index];
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_component_get_axis1.  "
      "Invalid argument(s)");
    value = (FE_value)0.0;
  }
  LEAVE;
  return(value);
} /* Cell_component_get_axis1() */

FE_value Cell_component_get_axis2(struct Cell_component *component,int index)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Returns the value of axis2[<index>]
==============================================================================*/
{
  FE_value value;

  ENTER(Cell_component_get_axis2);
  if (component && (index >= 0) && (index < 3))
  {
    value = component->axis2[index];
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_component_get_axis2.  "
      "Invalid argument(s)");
    value = (FE_value)0.0;
  }
  LEAVE;
  return(value);
} /* Cell_component_get_axis2() */

FE_value Cell_component_get_axis3(struct Cell_component *component,int index)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Returns the value of axis3[<index>]
==============================================================================*/
{
  FE_value value;

  ENTER(Cell_component_get_axis3);
  if (component && (index >= 0) && (index < 3))
  {
    value = component->axis3[index];
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_component_get_axis3.  "
      "Invalid argument(s)");
    value = (FE_value)0.0;
  }
  LEAVE;
  return(value);
} /* Cell_component_get_axis3() */

FE_value Cell_component_get_point(struct Cell_component *component,int index)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Returns the value of point[<index>]
==============================================================================*/
{
  FE_value value;

  ENTER(Cell_component_get_point);
  if (component && (index >= 0) && (index < 3))
  {
    value = component->point[index];
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_component_get_point.  "
      "Invalid argument(s)");
    value = (FE_value)0.0;
  }
  LEAVE;
  return(value);
} /* Cell_component_get_point() */

