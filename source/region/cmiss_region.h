/*******************************************************************************
FILE : cmiss_region.h

LAST MODIFIED : 4 December 2003

DESCRIPTION :
Definition of Cmiss_region, used to create hierarchical data structures.
Object can be structured into directed-acyclic-graphs or DAGs, which contain a
list of child regions, each with their own name as seen by that parent.
Advanced hierarchical functionality is built up by attaching context objects
to regions, but hiding their details from the core Cmiss_region object.
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
#if !defined (CMISS_REGION_H)
#define CMISS_REGION_H

#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "general/callback.h"
#include "general/object.h"


/*
Global constants
----------------
*/

/* separator character for Cmiss_region in path strings */
#define CMISS_REGION_PATH_SEPARATOR_CHAR '/'
#define CMISS_REGION_PATH_SEPARATOR_STRING "/"

/*
Global types
------------
*/

struct Cmiss_region;
/*******************************************************************************
LAST MODIFIED : 30 September 2002

DESCRIPTION :
Object responsible for building directed acyclic graph hierarchies in Cmiss.
Implements hierarchies by contains a list of Cmiss_region_child objects, each
with their own name as seen by this parent.
Implements advanced hierarchical functionality through context objects stored
within it. Type and role of context objects are not known to the Cmiss_region.
==============================================================================*/

struct Cmiss_region_changes
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Data broadcast with callbacks from <Cmiss_region> describing the changes.
==============================================================================*/
{
	/* true if children added, removed or reordered in Cmiss_region */
	int children_changed;
	/* If a single child has been added then identify it here for efficiency */
	struct Cmiss_region *child_added;
	/* If a single child has been removed then identify it here for efficiency */
	struct Cmiss_region *child_removed;
	/* The name by which the parent used to name the child */
	char *child_removed_name;
	/* true if objects added to or removed from Cmiss_region */
	int objects_changed;
}; /* struct Cmiss_region_changes */

DECLARE_CMISS_CALLBACK_TYPES(Cmiss_region_change, \
	struct Cmiss_region *, struct Cmiss_region_changes *, void);

/*
Global functions
----------------
*/

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_region);

struct Cmiss_region *Cmiss_region_create(void);
/*******************************************************************************
LAST MODIFIED : 23 May 2008

DESCRIPTION :
Creates a full Cmiss_region, able to have its own fields.
Region is created with an access_count of 1; DEACCESS to destroy.
==============================================================================*/

struct Cmiss_region *Cmiss_region_create_group(
		struct Cmiss_region *master_region);
/*******************************************************************************
LAST MODIFIED : 23 May 2008

DESCRIPTION :
Creates an empty Cmiss_region that shares the fields with master_region but
uses a subset of its nodes and element (i.e. a group).
Region is created with an access_count of 1; DEACCESS to destroy.
Consider as private: NOT approved for exposing in API.
==============================================================================*/

struct Cmiss_region *Cmiss_region_create_share_globals(
		struct Cmiss_region *source_region);
/*******************************************************************************
LAST MODIFIED : 23 May 2008

DESCRIPTION :
Equivalent to Cmiss_region_create(), except the new region uses
global basis_manager and shape_list from <source_region>.
Region is created with an access_count of 1; DEACCESS to destroy.
Consider as private: NOT approved for exposing in API.
==============================================================================*/

struct MANAGER(Computed_field) *Cmiss_region_get_Computed_field_manager(
	struct Cmiss_region *cmiss_region);
/*******************************************************************************
LAST MODIFIED : 20 May 2008

DESCRIPTION :
Returns the field manager containing all the fields for this region.
==============================================================================*/

int set_Cmiss_region_path(struct Parse_state *state, void *path_address_void,
	void *root_region_void);
/*******************************************************************************
LAST MODIFIED : 13 January 2003

DESCRIPTION :
Modifier function for entering a path to a Cmiss_region, starting at
<root_region>.
==============================================================================*/

int Option_table_add_set_Cmiss_region_path(struct Option_table *option_table,
	char *entry_name, struct Cmiss_region *root_region, char **path_address);
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Adds an entry to the <option_table> with name <entry_name> that returns a 
region path in <path_address> relative to the <root_region>.
==============================================================================*/

int Cmiss_region_begin_change(struct Cmiss_region *region);
/*******************************************************************************
LAST MODIFIED : 10 December 2002

DESCRIPTION :
Increments the change counter in <region>. No update callbacks will occur until
change counter is restored to zero by calls to Cmiss_region_end_change.
???RC Make recursive/hierarchical?
==============================================================================*/

int Cmiss_region_end_change(struct Cmiss_region *region);
/*******************************************************************************
LAST MODIFIED : 10 December 2002

DESCRIPTION :
Decrements the change counter in <region>. No update callbacks occur until the
change counter is restored to zero by this function.
???RC Make recursive/hierarchical?
==============================================================================*/

int Cmiss_region_add_callback(struct Cmiss_region *region,
	CMISS_CALLBACK_FUNCTION(Cmiss_region_change) *function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Adds a callback to <region> so that when it changes <function> is called with
<user_data>. <function> has 3 arguments, a struct Cmiss_region *, a
struct Cmiss_region_changes * and the void *user_data.
==============================================================================*/

int Cmiss_region_remove_callback(struct Cmiss_region *region,
	CMISS_CALLBACK_FUNCTION(Cmiss_region_change) *function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Removes the callback calling <function> with <user_data> from <region>.
==============================================================================*/

int Cmiss_region_add_child_region(struct Cmiss_region *region,
	struct Cmiss_region *child_region, char *child_name, int child_position);
/*******************************************************************************
LAST MODIFIED : 22 October 2002

DESCRIPTION :
Adds <child_region> to <region> at <child_position> under the <child_name>.
<child_position> must be from -1 to the regions number_of_child_regions; if it
is -1 it is added at the end of the list.
Ensures:
- <child_name> passes is_standard_identifier_name;
- <child_name> is not already used in <region>;
- <child_region> is not already in <region>;
- <child_region> is not the same as or contains <region>;
==============================================================================*/

int Cmiss_region_remove_child_region(struct Cmiss_region *region,
	struct Cmiss_region *child_region);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Removes <child_region> from <region>.
???RC what if this is the master region for the child?
==============================================================================*/

int Cmiss_region_contains_Cmiss_region(struct Cmiss_region *region,
	struct Cmiss_region *other_region);
/*******************************************************************************
LAST MODIFIED : 12 November 2002

DESCRIPTION :
Returns true if <region> contains <other_region>, this is the case if
<region> equals <other_region> or any of child regions satisfy this function.
==============================================================================*/

struct Cmiss_region *Cmiss_region_get_child_region_from_name(
	struct Cmiss_region *region, const char *child_name);
/*******************************************************************************
LAST MODIFIED : 23 September 2003

DESCRIPTION :
Returns a pointer to the child of <region> known by <child_name>.
Returns NULL without error messages if no child of that name exists in <region>.
==============================================================================*/

int Cmiss_region_get_child_region_number(struct Cmiss_region *region,
	struct Cmiss_region *child_region, int *child_number_address);
/*******************************************************************************
LAST MODIFIED : 9 January 2003

DESCRIPTION :
Returns in <child_number_address> the position of <child_region> in <region>,
with numbering starting at 0.
Returns 0 without error messages if <child_region> is not in <region>.
==============================================================================*/

int Cmiss_region_set_child_region_number(struct Cmiss_region *region,
	struct Cmiss_region *child_region, int child_number);
/*******************************************************************************
LAST MODIFIED : 24 March 2003

DESCRIPTION :
Changes the position of <child_region> in the list of children of <region> to
<child_number>, with the first child being 0. A <child_number> of -1 puts the
child at the bottom of the list.
==============================================================================*/

int Cmiss_region_get_region_from_path(struct Cmiss_region *root_region,
	const char *path, struct Cmiss_region **region_address);
/*******************************************************************************
LAST MODIFIED : 8 November 2002

DESCRIPTION :
Returns the region described relative to the <root_region> in <path>.
<path> is of the format CHILD_NAME/CHILD_NAME/...
where '/' is actually the CMISS_REGION_PATH_SEPARATOR_CHAR.
Single leading and trailing separator characters are ignored, hence:
- "/heart/coronaries/" = "heart/coronaries" -- the latter is preferred.
- both "/" and "" refer to the root_region itself.
If no region can be identified from path, returns successfully but with NULL
in <region_address> 
Note <path> is modified within this function but returned in its original state.
For safety, returns NULL in <region_address> on any error.
==============================================================================*/

int Cmiss_region_get_child_region_from_path(struct Cmiss_region *root_region,
	char *path, struct Cmiss_region **child_region_address,
	char **remaining_path_address);
/*******************************************************************************
LAST MODIFIED : 11 November 2002

DESCRIPTION :
Returns in <child_region> the child region of <root_region> named by the first
part of <path> up to the CMISS_REGION_PATH_SEPARATOR_CHAR.
<path> is allowed to have zero or one leading or trailing separators, except
that "//" is illegal.
<remaining_path> is advanced to the next child region name in <path> or NULL
if none.
Special case: "/" and "" both return <root_region>.
Returns 0 with error message if no child region found.
For safety, returns NULL in <child_region_address> and either NULL or a valid
pointer within path in <remaining_path_address> on any error.
==============================================================================*/

#if defined (OLD_CODE)
int Cmiss_region_resolve_path(struct Cmiss_region *root_region, char *path,
	struct Cmiss_region **region_address, char **resolved_path_address);
/*******************************************************************************
LAST MODIFIED : 8 January 2003

DESCRIPTION :
Returns in <region> the first whole Cmiss_region identified by <path> relative
to <root_region>. The resolved path is returned.
For safety, returns NULL in <region_address> and NULL in <resolved_path_address>
on any error.
==============================================================================*/
#endif /* defined (OLD_CODE) */

int Cmiss_region_get_number_of_child_regions(struct Cmiss_region *region,
	int *number_of_child_regions_address);
/*******************************************************************************
LAST MODIFIED : 7 November 2002

DESCRIPTION :
Gets the number of child regions in <region>.
==============================================================================*/

struct Cmiss_region *Cmiss_region_get_child_region(struct Cmiss_region *region,
	int child_number);
/*******************************************************************************
LAST MODIFIED : 26 May 2008

DESCRIPTION :
Returns the child <child_number> of <region> if within the range from 0 to
one less than number_of_child_regions, otherwise NULL.
==============================================================================*/

int Cmiss_region_get_child_region_name(struct Cmiss_region *region,
	int child_number, char **child_region_name_address);
/*******************************************************************************
LAST MODIFIED : 7 November 2002

DESCRIPTION :
Returns at <child_region_name_address> the name of child region <child_number>
as known by <region>.
<child_number> ranges from 0 to one less than number_of_child_regions.
For safety, returns NULL in <child_region_name_address> on any error.
Up to the calling function to deallocate the returned name.
==============================================================================*/

int Cmiss_region_get_root_region_path(char **path_address);
/*******************************************************************************
LAST MODIFIED : 17 January 2003

DESCRIPTION :
Allocates and returns the path to the root_region.
==============================================================================*/

int Cmiss_region_list(struct Cmiss_region *region,
	int indent, int indent_increment);
/*******************************************************************************
LAST MODIFIED : 5 March 2003

DESCRIPTION :
Lists the Cmiss_region hierarchy starting from <region>. Contents are listed
indented from the left margin by <indent> spaces; this is incremented by
<indent_increment> for each child level.
==============================================================================*/

#endif /* !defined (CMISS_REGION_H) */
