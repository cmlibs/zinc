/*******************************************************************************
FILE : parser.c

LAST MODIFIED : 28 August 2000

DESCRIPTION :
Functions for handling all file parsing for CELL.
==============================================================================*/
#include <stdio.h>
#include <string.h>
/* include the XML parsing header file, include directory defined in
   the make file */
#include "cell/parser.h"
#include "general/debug.h"

/*
Local functions
===============
*/
static char *get_attribute_content(struct XML_Element *element,char *name)
/*******************************************************************************
LAST MODIFIED : 26 January 1999

DESCRIPTION :
Returns the content of the attribute <name> in the given <element>, if it
exists, otherwise returns a NULL string.
==============================================================================*/
{
  char *content;
  int i;
  
  ENTER(get_attribute_content);
  i = 0;
  while ((i<element->number_of_attributes) &&
    (strcmp((element->attributes[i]).name,name)))
  {
    i++;
  }
  if (i == element->number_of_attributes)
  {
    content = (char *)NULL;
  }
  else
  {
    content = (element->attributes[i]).content;
  }
  LEAVE;
  return(content);
} /* END get_attribute_content() */

static struct XML_Tree parse_cell_file(char *filename,int *er)
/*******************************************************************************
LAST MODIFIED : 28 January 1999

DESCRIPTION :
Parses the file specified by <filename> and returns the corresponding tree.
<er> is returned 0 if an error occured (need some check on whether the tree is
good).
==============================================================================*/
{
  struct XML_Tree tree;
  struct XML_Error_return *error_return = (struct XML_Error_return *)NULL;

  ENTER(parse_cell_file);
  /* check the file for well-formedness */
  error_return = XML_wf(filename);
  if (error_return == (struct XML_Error_return *)NULL)
  {
    /* document is well-formed */
    tree = XML_build_tree(filename,error_return);
    if (tree.root != tree.current_element)
    {
      /* The pointer to the current element should always be returned
      pointing to the root element.
      This assignment operation only seems to be required when building
      library under IRIX (6.52???), not required for Linux */
      tree.root = tree.current_element;
    }
    if (error_return != (struct XML_Error_return *)NULL)
    {
      display_xml_error_message(error_return);
      *er = 0;
    }
    else
    {
      *er = 1;
    }
  }
  else
  {
    display_xml_error_message(error_return);
    *er = 0;
  }
  LEAVE;
  return (tree);
} /* END parse_cell_file() */

/*
Global functions
================
*/
struct XML_Tree parse_cell_uri(struct URI *uri,int *er)
/*******************************************************************************
LAST MODIFIED : 25 August 2000

DESCRIPTION :
Parses the file specified by the <uri> and returns the corresponding tree.
<er> is returned 0 if an error occured (need some check on whether the tree is
good).
==============================================================================*/
{
  struct XML_Tree tree;
  char *filename;

  ENTER(parse_cell_uri);
  /* assemble the full file name */
  if (ALLOCATE(filename,char,strlen(uri->path)+strlen(uri->filename)+1))
  {
		strcpy(filename,uri->path);
		strcat(filename,uri->filename);
    tree = parse_cell_file(filename,er);
  }
  else
  {
    display_message(ERROR_MESSAGE,"parse_cell_uri. "
      "Unable to allocate memory for the full file name");
    *er = 0;
  }
  LEAVE;
  return (tree);
} /* END parse_cell_uri() */

void display_cell_tree(struct XML_Tree *tree)
/*******************************************************************************
LAST MODIFIED : 20 January 1999

DESCRIPTION :
Displays a CELL XML tree.

Currently only writes out tree to the prompt, but could be modified to display
in some form of dialog box.
==============================================================================*/
{
  ENTER(display_cell_tree);
  /* use function from XML library */
  XML_write_tree(tree);
  LEAVE;
} /* END display_cell_tree() */

struct XML_Element *find_element(struct XML_Element *start,char **tags,
  int num_tags)
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
Returns the element corresponding to the final tag in the <tags> array in the
children of the <start> element. The <tags> array specifies the element tags
from the top level <start> down to the required level.
==============================================================================*/
{
  int i,cont = 1,j;
  struct XML_Element *element = (struct XML_Element *)NULL;
  
  ENTER(find_element);
  /* start the found element at the root of the tree */
  element = start;
  /* loop through the tags, decending the tree */
  for (i=0;(i<num_tags) && cont;i++)
  {
    j = 0;
    while ((j<element->number_of_children) &&
      (strcmp(element->children[j]->element_tag,tags[i])))
    {
      j++;
    }
    if (j == element->number_of_children)
    {
      /* tag not found */
      cont = 0;
    }
    else
    {
      /* tag found, so reset the element */
      element = element->children[j];
    }
  } /* for (i=0;(i<num_tags) && cont;i++) */
  if (!cont)
  {
    element = (struct XML_Element *)NULL;
  }
  LEAVE;
  return(element);
} /* END find_element() */

char *cell_get_content(struct XML_Tree *tree,char **tags,int num_tags)
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
Returns the character content of of the element of the <tree> which corresponds
to the final tag in the <tags> array, which lists the tags from the top level
of the <tree> down to the required level. Returns a NULL character array if any
tag is not found in the tree or there is no character content in the element.
==============================================================================*/
{
  struct XML_Element *element = (struct XML_Element *)NULL;
  char *content = (char *)NULL;
  
  ENTER(cell_get_content);
  /* first find the corresponding element in the tree */
  element = find_element(tree->root,tags,num_tags);
  if (element != (struct XML_Element *)NULL)
  {
    content = element->content;
  }
  else
  {
    display_message(ERROR_MESSAGE,"cell_get_content. "
      "Unable to find element.");
    content = (char *)NULL;
  }
  LEAVE;
  return(content);
} /* END cell_get_content() */

char *cell_get_attribute(struct XML_Tree *tree,char **tags,int num_tags,
  char *name)
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
Returns the character content of of the attribute <name> of the element of the
<tree> which corresponds to the final tag in the <tags> array, which lists the
tags from the top level of the <tree> down to the required level. Returns a
NULL character array if any tag is not found in the tree or there is no
character content in the element.
==============================================================================*/
{
  struct XML_Element *element = (struct XML_Element *)NULL;
  char *content = (char *)NULL;
  
  ENTER(cell_get_attribute);
  /* first find the corresponding element in the tree */
  element = find_element(tree->root,tags,num_tags);
  if (element != (struct XML_Element *)NULL)
  {
    content = get_attribute_content(element,name);
  }
  else
  {
    display_message(ERROR_MESSAGE,"cell_get_attribute. "
      "Unable to find element.");
    content = (char *)NULL;
  }
  LEAVE;
  return(content);
} /* END cell_get_attribute() */
