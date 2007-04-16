/*******************************************************************************
FILE : comfile.h

LAST MODIFIED : 29 June 2002

DESCRIPTION :
Commands and functions for comfiles.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#if !defined (COMFILE_H)
#define COMFILE_H

#if defined (MOTIF)
#include "comfile/comfile_window.h"
#elif defined (WX_USER_INTERFACE)
#include "comfile/comfile_window_wx.h"
#endif /* defined (MOTIF) */
#include "command/parser.h"
#include "general/io_stream.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/

struct Open_comfile_data
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION :
==============================================================================*/
{
	char example_flag,*examples_directory,*example_symbol,*file_extension,
		*file_name;
	int execute_count;
	struct Execute_command *execute_command,*set_command;
	struct IO_stream_package *io_stream_package;
#if defined (MOTIF) || (WX_USER_INTERFACE)
	struct MANAGER(Comfile_window) *comfile_window_manager;
#endif /* defined (MOTIF) || (WX_USER_INTERFACE) */
	struct User_interface *user_interface;
}; /* struct Open_comfile_data */

/*
Global functions
----------------
*/

int open_comfile(struct Parse_state *state,void *dummy_to_be_modified,
	void *open_comfile_data_void);
/*******************************************************************************
LAST MODIFIED : 29 June 2002

DESCRIPTION 
Opens a comfile, and a window if it is to be executed.  If a comfile is not
specified on the command line, a file selection box is presented to the user.
==============================================================================*/
#endif /* !defined (COMFILE_H) */
