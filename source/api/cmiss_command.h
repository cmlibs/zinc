/*******************************************************************************
FILE : cmiss_command.h

LAST MODIFIED : 13 August 2003

DESCRIPTION :
The public interface to the some of the internal functions of cmiss.
==============================================================================*/
#ifndef __CMISS_COMMAND_H__
#define __CMISS_COMMAND_H__

/* If this is going to be in the API then it needs to have an interface there */
#include "general/object.h"

/*
Global types
------------
*/
struct Cmiss_command_data;
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
Shifted the Cmiss_command_data to be internal to cmiss.c
==============================================================================*/

/*
Global functions
----------------
*/

struct Cmiss_command_data *CREATE(Cmiss_command_data)(int argc,char *argv[],
	char *version_string);
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
Initialise all the subcomponents of cmgui and create the Cmiss_command_data
==============================================================================*/

int DESTROY(Cmiss_command_data)(struct Cmiss_command_data **command_data_address);
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
Clean up the command_data, deallocating all the associated memory and resources.
==============================================================================*/

int Cmiss_command_data_execute(struct Cmiss_command_data *command_data,
	char *command);
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
Parses the supplied <command> using cmiss on command parser, therefore the
string should only contain valid "gfx" or "fem" syntax.
==============================================================================*/

struct Cmiss_region *Cmiss_command_data_get_root_region(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 18 April 2003

DESCRIPTION :
Returns the root region from the <command_data>.
==============================================================================*/
#endif /* __CMISS_COMMAND_H__ */
