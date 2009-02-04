/*******************************************************************************
FILE : cmiss_region.i

LAST MODIFIED : 23 January 2008

DESCRIPTION :
Swig interface file for wrapping api header api/cmiss_region
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

#if defined SWIGPYTHON
#  define MODULE_PREFIX(modulename) modulename
#else
#  define MODULE_PREFIX(modulename) cmiss_ ## modulename
#endif

%module MODULE_PREFIX(region)

%include carrays.i
%array_functions(float, float_array);

%{
#include "api/cmiss_region.h"
%}

//%include "api/cmiss_region.h"

%nodefaultctor;
%nodefaultdtor;

typedef struct Cmiss_region
{
	%extend {
	    Cmiss_region()
	    {
	       return Cmiss_region_create();
	    }
	    ~Cmiss_region()
	    {
	       Cmiss_region_destroy(&$self);
	    }
	    int read_file(char *filename);
	    int add_child_region(struct Cmiss_region *child_region,
	       const char *child_name, int child_position);
	    int get_number_of_nodes_in_region();
	    int get_number_of_elements_in_region();
	    struct Cmiss_field *find_field_by_name(const char *field_name);
	    struct Cmiss_node *get_node(const char *name);
	    struct Cmiss_field *add_field(struct Cmiss_field *field);
	    struct Cmiss_region *get_sub_region(const char *name);
	}
} *Cmiss_region_id;


