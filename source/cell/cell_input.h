/*******************************************************************************
FILE : cell_input.h

LAST MODIFIED : 18 November 2000

DESCRIPTION :
Input routines for the cell interface.
==============================================================================*/
#if !defined (CELL_INPUT_H)
#define CELL_INPUT_H

#include "cell/cell_calculate.h"
#include "cell/cell_cmgui_interface.h"
#include "cell/cell_component.h"
#include "cell/cell_graphic.h"
#include "cell/cell_interface.h"
#include "cell/cell_variable.h"

/*
Module objects
--------------
*/
struct Cell_input;
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
A data object used to input cell models into the Cell interface data objects.
==============================================================================*/


/*
Global functions
----------------
*/
struct Cell_input *CREATE(Cell_input)(void);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Creates a Cell_input object.
==============================================================================*/
int DESTROY(Cell_input)(struct Cell_input **cell_input_address);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Destroys the Cell_input object.
==============================================================================*/
int Cell_input_close(struct Cell_input *cell_input);
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Closes the Cell_input object.
==============================================================================*/
char *Cell_input_read_model(struct Cell_input *cell_input,char *filename,
  int *XMLParser_initialised,struct LIST(Cell_component) *component_list,
  struct LIST(Cell_variable) *variable_list,
  struct LIST(Cell_graphic) *graphic_list,struct Cell_calculate *cell_calculate,
  struct Cell_cmgui_interface *cmgui_interface);
/*******************************************************************************
LAST MODIFIED : 01 November 2000

DESCRIPTION :
Reads in the cell model specified in <filename>. <XMLParser_initialised> is a
pointer to the value in the Cell interface object, used to determine if the
XML parser has been initialised or not.

Returns the display name of the model.
==============================================================================*/
void Cell_input_list_XMLParser_properties(struct Cell_input *cell_input,
  int XMLParser_initialised);
/*******************************************************************************
LAST MODIFIED : 30 June 2000

DESCRIPTION :
Lists out the XML parser's properties.
==============================================================================*/
int *Cell_input_get_XMLParser_properties(struct Cell_input *cell_input);
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
Gets the XML parser's properties.
==============================================================================*/
int Cell_input_set_XMLParser_properties(struct Cell_input *cell_input,
  int *properties);
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
Sets the XML parser's properties.
==============================================================================*/
void Cell_input_list_copy_tags(struct Cell_input *cell_input);
/*******************************************************************************
LAST MODIFIED : 30 June 2000

DESCRIPTION :
Lists out the current copy tags.
==============================================================================*/
char **Cell_input_get_copy_tags(struct Cell_input *cell_input);
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Returns a copy of the current copy tags.
==============================================================================*/
void Cell_input_list_ref_tags(struct Cell_input *cell_input);
/*******************************************************************************
LAST MODIFIED : 30 June 2000

DESCRIPTION :
Lists out the current ref tags.
==============================================================================*/
void *Cell_input_get_crim_document(struct Cell_input *cell_input);
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION :
Returns a pointer to the current CRIM document.
==============================================================================*/

#endif /* !defined (CELL_INPUT_H) */
