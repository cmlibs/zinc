/*******************************************************************************
FILE : input.h

LAST MODIFIED : 14 September 1999

DESCRIPTION :
Functions for handling all file input for CELL.
==============================================================================*/
#if !defined (CELL_INPUT_H)
#define CELL_INPUT_H

#include <stdio.h>
#if defined (MOTIF)
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */
#include "cell/cell_window.h"

/*
Global types
============
*/
struct URI
/*******************************************************************************
LAST MODIFIED : 27 January 1999

DESCRIPTION :
Stores a URI.
==============================================================================*/
{
  char *path; /* either an absolute or relative path or a URL */
  char *filename;
}; /* struct URI */

/*
Global functions
================
*/
#if defined (OLD_CODE)
struct XML_Tree create_cell_tree(char *filename,int *return_code);
/*******************************************************************************
LAST MODIFIED : 21 January 1999

DESCRIPTION :
Creates the main XML tree from the cell specification file <filename>.
Returns <return_code> as 1 if tree successfully created, otherwise returns 0

(1) Parse the cell specification file which should give
    - an experimental specification URI and/or elements which should
      replace those given in the file
    - a parameter specification URI and/or elements which should replace those
      given in the file
(2) Parse the experimental specification file and add to the tree, with
    appropriate elements replaced by those given in the cell spec. file
(3) Parse the parameter specification file and add to the tree, with
    appropriate elements replaced by those given in the cell spec. file
==============================================================================*/
#endif /* defined (OLD_CODE) */
char **cell_create_tag_list(int num, ... );
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
Takes a list of character arrays and generates a single array of character
pointers containing each of th individual tags. <num> gives the number of tags
to be added to the array.
==============================================================================*/
void cell_destroy_tag_list(char **tags,int num_tags);
/*******************************************************************************
LAST MODIFIED : 26 January 1999

DESCRIPTION :
Frees the memory used by a list of element tags.
==============================================================================*/
#if defined (OLD_CODE)
int read_variables_file(char *filename,XtPointer cell_window);
/*******************************************************************************
LAST MODIFIED : 06 February 1999

DESCRIPTION :
The function called when a variables file is selected via the file selection
box.
==============================================================================*/
#endif /* defined (OLD_CODE) */
int read_model_file(char *filename,XtPointer cell_window);
/*******************************************************************************
LAST MODIFIED : 06 February 1999

DESCRIPTION :
Read the model file <filename>, and set-up the <cell_window>.
==============================================================================*/
#if defined (OLD_CODE)
int read_parameters_file(char *filename,XtPointer cell_window);
/*******************************************************************************
LAST MODIFIED : 06 February 1999

DESCRIPTION :
The function called when a parameters file is selected via the file selection
box.
==============================================================================*/
#endif /* defined (OLD_CODE) */
void verify_text_field_modification(Widget widget,XtPointer unused,
  XtPointer callback_struct);
/*******************************************************************************
LAST MODIFIED : 13 February 1999

DESCRIPTION :
Callback for when a text field's value is changed. Ensures that only numbers
are entered.
==============================================================================*/
int extract_float_from_text_field(Widget text,float *value);
/*******************************************************************************
LAST MODIFIED : 13 February 1999

DESCRIPTION :
Extracts a float <value> from the text field widget <text>.
==============================================================================*/
int read_model_names(struct Cell_window *cell,char *filename);
/*******************************************************************************
LAST MODIFIED : 15 February 1999

DESCRIPTION :
Reads and sets the model names from <filename>
==============================================================================*/

#endif /* if !defined (CELL_INPUT_H) */
