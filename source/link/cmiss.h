/*******************************************************************************
FILE : cmiss.h

LAST MODIFIED : 15 January 2003

DESCRIPTION :
This module keeps the data structures within CMISS and CMGUI 'in sync'.  It does
this by creating two wormhole connections.

The first connection handles commands to and from CMISS.

The second connection handles data about nodes, elements etc.

Note:  A connection comprises two wormholes - input and output.
==============================================================================*/
#if !defined (CMISS_H)
#define CMISS_H

#include "finite_element/finite_element.h"
#include "prompt/prompt_window.h"
#include "region/cmiss_region.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/

struct CMISS_connection;
/*******************************************************************************
LAST MODIFIED : 3 September 2001

DESCRIPTION :
Private structure representing the connection between cm and cmgui.
==============================================================================*/

/*
Global Functions
----------------
*/

struct CMISS_connection *CREATE(CMISS_connection)(char *machine,
	enum Machine_type type, int attach, double wormhole_timeout, char mycm_flag,
	char asynchronous_commands, struct Cmiss_region *root_region,
	struct Cmiss_region *data_root_region,
	struct Prompt_window **prompt_window_address,char *parameters_file_name,
	char *examples_directory_path,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 15 January 2003

DESCRIPTION :
Creates a connection to the machine specified in <machine>.  If <attach> is
not zero then cm already exists and <attach> is the base port number to connect
on, otherwise a new cm is spawned.  If <asynchronous_commands> is set then cmgui
does not wait for cm commands to complete, otherwise it does.
==============================================================================*/

int DESTROY(CMISS_connection)(struct CMISS_connection **connection_address);
/*******************************************************************************
LAST MODIFIED : 3 December 1996

DESCRIPTION :
Frees the memory for the connection, sets <*node_address> to NULL.
==============================================================================*/

int CMISS_connection_process_command(
	struct CMISS_connection **connection_address,char *command,
	Widget modal_widget);
/*******************************************************************************
LAST MODIFIED : 3 October 2001

DESCRIPTION :
Executes the given command within CMISS.  This routine may destroy the
connection.
==============================================================================*/

int CMISS_connection_process_prompt_reply(struct CMISS_connection *connection,
	char *reply);
/*******************************************************************************
LAST MODIFIED : 3 December 1996

DESCRIPTION :
Sends the given text in response to the prompt.
==============================================================================*/

int CMISS_connection_update(struct CMISS_connection **connection_address);
/*******************************************************************************
LAST MODIFIED : 3 October 2001

DESCRIPTION :
Performs any updating necessary.  This routine may destroy the connection.
==============================================================================*/
#endif /* CMISS_H */
