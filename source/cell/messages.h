/*******************************************************************************
FILE : messages.h

LAST MODIFIED : 25 January 1999

DESCRIPTION :
Functions for the display of messages.
==============================================================================*/
#if !defined (CELL_MESSAGES_H)
#define CELL_MESSAGES_H

#include "user_interface/message.h"
#include "xml.h"

/*
Global functions
================
*/
void display_xml_error_message(struct XML_Error_return *er);
/*******************************************************************************
LAST MODIFIED : 19 January 1999

DESCRIPTION :
Writes out the error defined by <er> (returned from the XML parsing library).
==============================================================================*/
void cell_display_message(char *message,char *title);
/*****************************************************************************
LAST MODIFIED : 19 january 1999

DESCRIPTION :
Used to display messages by stand-alone CELL.

DPN TODO - needs to be updated to using dialog boxes
==============================================================================*/

#endif /* if !defined (CELL_MESSAGES_H) */
