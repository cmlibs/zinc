/*******************************************************************************
FILE : model_dialog.h

LAST MODIFIED : 16 September 1999

DESCRIPTION :
Functions and structures for using the model dialog box.
==============================================================================*/
#if !defined (CELL_MODEL_DIALOG_H)
#define CELL_MODEL_DIALOG_H

#if defined (MOTIF)
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "cell/cell_window.h"
#include "cell/input.h"

/*
Global types
============
*/
enum Model_type
/*******************************************************************************
LAST MODIFIED 15 February 1999

DESCRIPTION :
The different types of models allowed
==============================================================================*/
{
  MEMBRANE_MODEL,
  MECHANICS_MODEL,
  METABOLISM_MODEL,
  COUPLED_MODEL,
  UNKNOWN_MODEL
};

struct Model_name
/*******************************************************************************
LAST MODIFIED : 14 September 1999

DESCRIPTION :
To hold the model names
==============================================================================*/
{
  char *name;
  char *id; /* used to identify with a model specified via a node */
  struct URI *uri;
  enum Model_type type;
  struct Model_name *next;
};

struct Model_dialog
/*******************************************************************************
LAST MODIFIED : 28 February 1999

DESCRIPTION :
Structure containing information related to the "Model" dialog.
==============================================================================*/
{
  struct Model_name *model_names;
  Widget shell;
  Widget window;
  Widget current_model_label;
  Widget membrane_list;
  Widget mechanics_list;
  Widget metabolism_list;
  Widget coupled_list;
  char *current_model_string;
}; /* struct Model_dialog */

/*
Global functions
================
*/
int bring_up_model_dialog(struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 03 February 1999

DESCRIPTION :
If there is a model dialog in existence, then bring it to the front, else
create a new one.
==============================================================================*/
void set_model_name(struct Cell_window *cell,int type,char *name,
  struct URI *uri,char *id);
/*******************************************************************************
LAST MODIFIED : 27 February 1999

DESCRIPTION :
Adds the model <name> to the model_names array in <cell>. <type> = 1 for a
membrane model, 2 for a mechanics model, 3 for a metabolism model, and 4 for a
coupled model. <uri> is the corresponding model file. <id> is used to identify
a model specified via a node.
==============================================================================*/
void close_model_dialog(struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
If there is a model dialog in existence, destroy it.
==============================================================================*/

#endif /* !defined (CELL_MODEL_DIALOG_H) */
