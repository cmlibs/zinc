/*******************************************************************************
FILE : parser.h

LAST MODIFIED : 15 February 1999

DESCRIPTION :
Functions for handling all file parsing for CELL.
==============================================================================*/
#if !defined (CELL_PARSER_H)
#define CELL_PARSER_H

#include "xml.h"
#include "cell/cell_window.h"
#include "cell/input.h"
#include "cell/messages.h"
#include "user_interface/message.h"

/*
Global functions
================
*/
struct XML_Tree parse_cell_uri(struct URI *uri,int *er);
/*******************************************************************************
LAST MODIFIED : 28 January 1999

DESCRIPTION :
Parses the file specified by the <uri> and returns the corresponding tree.
<er> is returned 0 if an error occured (need some check on whether the tree is
good).
==============================================================================*/
void display_cell_tree(struct XML_Tree *tree);
/*******************************************************************************
LAST MODIFIED : 20 January 1999

DESCRIPTION :
Displays a CELL XML tree.

Currently only writes out tree to the prompt, but could be modified to display
in some form of dialog box.
==============================================================================*/
struct XML_Element *find_element(struct XML_Element *start,char **tags,
  int num_tags);
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
Returns the element corresponding to the final tag in the <tags> array in the
children of the <start> element. The <tags> array specifies the element tags
from the top level <start> down to the required level.
==============================================================================*/
char *cell_get_content(struct XML_Tree *tree,char **tags,int num_tags);
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
Returns the character content of of the element of the <tree> which corresponds
to the final tag in the <tags> array, which lists the tags from the top level
of the <tree> down to the required level. Returns a NULL character array if any
tag is not found in the tree or there is no character content in the element.
==============================================================================*/
char *cell_get_attribute(struct XML_Tree *tree,char **tags,int num_tags,
  char *name);
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
Returns the character content of of the attribute <name> of the element of the
<tree> which corresponds to the final tag in the <tags> array, which lists the
tags from the top level of the <tree> down to the required level. Returns a
NULL character array if any tag is not found in the tree or there is no
character content in the element.
==============================================================================*/

#endif /* if !defined (CELL_PARSER_H) */
