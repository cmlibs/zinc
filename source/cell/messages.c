/*******************************************************************************
FILE : messages.c

LAST MODIFIED : 25 January 1999

DESCRIPTION :
Functions for the display of messages.
==============================================================================*/
#include <stdio.h>
/* need the XML header for the XML_Error_return structure */
#include "xml.h"
#include "cell/messages.h"
#include "general/debug.h"

/*
Local functions
===============
*/

/*
Global functions
================
*/
void display_xml_error_message(struct XML_Error_return *er)
/*******************************************************************************
LAST MODIFIED : 19 January 1999

DESCRIPTION :
Writes out the error defined by <er> (returned from the XML parsing library).
==============================================================================*/
{
  ENTER(display_XML_error_message);
  if ((er->message != (char *)NULL) && (er->filename != (char *)NULL))
  {
    printf("\nERROR (%s) occurred at line %d, column %d in file %s\n\n",
      er->message,er->line_number,er->column_number,er->filename);
  }
  else
  {
    printf("\nERROR - unknown\n\n");
  }
  LEAVE;
} /* END display_xml_error_message() */

void cell_display_message(char *message,char *title)
/*****************************************************************************
LAST MODIFIED : 19 january 1999

DESCRIPTION :
Used to display messages by stand-alone CELL.

DPN TODO - needs to be updated to using dialog boxes
==============================================================================*/
{
  if (title[0] == 'I')
  {
    printf("%s -> %s",title,message);
  }
  else
  {
    printf("%s -> %s\n",title,message);
  }
} /* END cell_display_message() */
