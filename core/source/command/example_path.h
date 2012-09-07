/*******************************************************************************
FILE : example_path.h

LAST MODIFIED : 17 April 2000

DESCRIPTION :
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
#if !defined (EXAMPLE_PATH_H)
#define EXAMPLE_PATH_H


/*
Global functions
----------------
*/

char *resolve_example_path(char *example_path, char *directory_name,
	char **comfile_name, char **requirements);
/*******************************************************************************
LAST MODIFIED : 17 April 2000

DESCRIPTION :
Uses the executable $example_path/common/resolve_example_path to demangle
a short example name into a full path.  The returned string is ALLOCATED.
Code is basically a repeat from general/child_process but don't want to create
an object and want to encapsulate the whole process in one file for calling
from the back end.
<*comfile_name> is allocated and returned as well if the resolve function
returns a string for it.  This too must be DEALLOCATED by the calling function.
<*requirements> is either set to NULL or is an allocated string specifying 
the features required to run this example, i.e. whether the example 
needs cmgui and/or cm. The requirements are comma separated.  This too must 
be DEALLOCATED by the calling function.
==============================================================================*/

#endif /* !defined (EXAMPLE_PATH_H) */
