/*******************************************************************************
FILE : execute_command.i

LAST MODIFIED : 16 December 2008

DESCRIPTION :
Swig interface file for wrapping the Cmiss_command_data_execute_command 
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
 * Portions created by the Initial Developer are Copyright (C) 2008
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
 /* execute_command.i */

struct Cmiss_command_data;
%module execute_command

%{
#include "api/cmiss_command_data.h"
extern int Cmiss_command_data_execute_command(struct Cmiss_command_data 
	*command_data, const char *command);
extern struct Cmiss_command_data *create_Cmiss_command_data(int argc, 
	char *argv[], char *version_string);
extern struct Cmiss_region *Cmiss_command_data_get_root_region(
	struct Cmiss_command_data *command_data);
extern struct Cmiss_region *Cmiss_region_create(void);
extern int Cmiss_region_get_number_of_nodes_in_region(struct Cmiss_region 
	*region);
extern struct Cmiss_region *Cmiss_region_get_sub_region(struct Cmiss_region *region,
	const char *path);
%}

%inline %{
	static void *Cmiss_main_loop_run(void *command_data_void)
	{
		Cmiss_command_data_id command_data = 
			(Cmiss_command_data_id)command_data_void;
	
        	Cmiss_command_data_main_loop(command_data);
	}

	/* Create a new cmiss_command_data */
    	struct Cmiss_command_data *new_Cmiss_command_data()
    	{
		Cmiss_command_data_id command_data;
		           
		char* cmgui_argv[] = {strdup("cmgui"), NULL};

		char **string_ptr;
	    	pthread_t thread;
	
		command_data = create_Cmiss_command_data(
			(sizeof(cmgui_argv)/sizeof(char *) - 1),
			cmgui_argv, "0.0"); 

        /* Need to fix up the memory leaks from the strdup sometime */        
	 
		if (pthread_create(&thread, NULL, Cmiss_main_loop_run,
		    (void *)command_data) != 0)
	    	{
	        	return NULL;
	    	}
        
        	return command_data;
      	}

	int Cmiss_region_get_number_of_nodes_in_region(struct Cmiss_region *region)
	/*******************************************************************************
	LAST MODIFIED : 4 November 2004

	DESCRIPTION :
	Returns the number of nodes in the <region>.
	==============================================================================*/
	{
		int number_of_nodes;
		struct FE_region *fe_region;

		//ENTER(Cmiss_region_get_number_of_nodes_in_region);
		number_of_nodes = 0;
		if (region)
		{
			if (fe_region = Cmiss_region_get_FE_region(region))
			{
				number_of_nodes = FE_region_get_number_of_FE_nodes(fe_region);
			}
		}
		//LEAVE;

		return (number_of_nodes);
	} /* Cmiss_region_get_number_of_elements */
%}

extern int Cmiss_command_data_execute_command(struct Cmiss_command_data 
	*command_data, const char *command);
extern struct Cmiss_command_data *create_Cmiss_command_data(int argc, 
	char *argv[], char *version_string);
extern struct Cmiss_region *Cmiss_command_data_get_root_region(
	struct Cmiss_command_data *command_data);
extern struct Cmiss_region *Cmiss_region_create(void);
extern int Cmiss_region_get_number_of_nodes_in_region(struct Cmiss_region 
	*region);
extern struct Cmiss_region *Cmiss_region_get_sub_region(struct Cmiss_region *region,
	const char *path);
