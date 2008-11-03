/*******************************************************************************
FILE : finite_element_region.h

LAST MODIFIED : 8 August 2006

DESCRIPTION :
Object comprising a single finite element mesh including nodes, elements and
finite element fields defined on or interpolated over them.
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
#if !defined (FINITE_ELEMENT_REGION_H)
#define FINITE_ELEMENT_REGION_H

#include "finite_element/finite_element.h"
#include "general/any_object.h"
#include "general/any_object_prototype.h"
#include "general/callback.h"
#include "general/change_log.h"
#include "general/object.h"
#include "region/cmiss_region.h"

/*
Global types
------------
*/

struct FE_region;

struct FE_region_changes
/*******************************************************************************
LAST MODIFIED : 5 February 2003

DESCRIPTION :
Structure sent out with FE_region change callbacks describing the changes that
have taken place. <fe_field_changes> lists all the FE_fields added, removed and
modified in the region since the last callback. <node_changes> and
<element_changes> similarly list changes to nodes and elements, except that
the changes are limited to the fields listed in the <fe_field_changes>.
==============================================================================*/
{
	struct CHANGE_LOG(FE_field) *fe_field_changes;
	struct CHANGE_LOG(FE_node) *fe_node_changes;
	struct CHANGE_LOG(FE_element) *fe_element_changes;
}; /* struct FE_region_changes */

DECLARE_CMISS_CALLBACK_TYPES(FE_region_change, \
	struct FE_region *, struct FE_region_changes *, void);

/*
Global macros
-------------
*/

/* Need these macros to help the text_choose_from_fe_region macros. */
#define FE_region_changes_get_FE_field_changes(changes) changes->fe_field_changes
#define FE_region_changes_get_FE_node_changes(changes) changes->fe_node_changes
#define FE_region_changes_get_FE_element_changes(changes) changes->fe_element_changes

#define FE_region_FE_object_method_class( object_type ) FE_region_FE_object_method_class_FE_ ## object_type \

#define DEFINE_FE_region_FE_object_method_class( object_type )	\
class FE_region_FE_object_method_class( object_type ) \
{ \
 public: \
	 typedef enum CHANGE_LOG_CHANGE(FE_ ## object_type) change_log_change_object_type; \
	 typedef struct CHANGE_LOG(FE_ ## object_type) change_log_object_type;	\
	 static const enum CHANGE_LOG_CHANGE(FE_ ## object_type) change_log_object_unchanged = \
			CHANGE_LOG_OBJECT_UNCHANGED(FE_ ## object_type);	\
	 static const enum CHANGE_LOG_CHANGE(FE_ ## object_type) change_log_object_added = \
			CHANGE_LOG_OBJECT_ADDED(FE_ ## object_type); \
	 static const enum CHANGE_LOG_CHANGE(FE_ ## object_type) change_log_object_removed = \
			CHANGE_LOG_OBJECT_REMOVED(FE_ ## object_type); \
	 static const enum CHANGE_LOG_CHANGE(FE_ ## object_type) change_log_object_identifier_changed = \
			CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_ ## object_type);	\
	 static const enum CHANGE_LOG_CHANGE(FE_ ## object_type) change_log_object_not_identifier_changed = \
			CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_ ## object_type); \
	 static const enum CHANGE_LOG_CHANGE(FE_ ## object_type) change_log_object_changed = \
			CHANGE_LOG_OBJECT_CHANGED(FE_ ## object_type); \
	 static const enum CHANGE_LOG_CHANGE(FE_ ## object_type) change_log_related_object_changed = \
			CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_ ## object_type); \
	 static inline FE_ ## object_type* string_to_object(struct FE_region *fe_region, char *string) \
	 { \
			return FE_region_ ## object_type ## _string_to_FE_ ## object_type(fe_region, string); \
	 } \
\
    static inline FE_ ## object_type* get_first_object_that(struct FE_region *fe_region, \
			 LIST_CONDITIONAL_FUNCTION(FE_ ## object_type) *conditional_function, \
			 void *user_data_void) \
	 { \
			return FE_region_get_first_FE_ ## object_type ## _that(fe_region, \
				 conditional_function, user_data_void); \
	 } \
\
	 static inline int FE_region_contains_object(struct FE_region *fe_region, struct FE_## object_type *object) \
	 { \
			return FE_region_contains_FE_ ## object_type(fe_region, object); \
	 } \
\
 	 static inline int FE_object_to_string(struct FE_## object_type *object, char **string) \
 	 {	\
			return FE_ ##object_type ## _to_ ##object_type## _string(object, string); \
	 } \
\
	 static inline change_log_object_type *get_object_changes(struct FE_region_changes *changes) \
	 { \
			return changes->fe_ ## object_type ## _changes; \
	 } \
\
	 static inline int change_log_query(struct CHANGE_LOG(FE_ ##object_type) *change_log, \
			struct FE_ ##object_type *object,	enum CHANGE_LOG_CHANGE(FE_ ##object_type) *change_address )	\
	 { \
			return CHANGE_LOG_QUERY(FE_ ##object_type)( \
				 change_log, object, change_address); \
	 } \
\
}; 

/*
Global functions
----------------
*/

struct FE_region *CREATE(FE_region)(struct FE_region *master_fe_region,
	struct MANAGER(FE_basis) *basis_manager,
	struct LIST(FE_element_shape) *element_shape_list);
/*******************************************************************************
LAST MODIFIED : 8 August 2006

DESCRIPTION :
Creates a struct FE_region.
If <master_fe_region> is supplied, the <basis_manager> and <element_shape_list>
are ignored and along with all fields, nodes and elements the FE_region may address,
will belong to the master region, and this FE_region will be merely a container
for nodes and elements.
If <master_fe_region> is not supplied, the FE_region will own all its own nodes,
elements and fields.  If <basis_manager> or <element_shape_list> are not 
supplied then a default empty object will be created for this region.  (Allowing
them to be specified allows sharing across regions).
==============================================================================*/

struct FE_region *FE_region_get_data_FE_region(struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 24 October 2008

DESCRIPTION :
Gets the data FE_region for <fe_region>, which has an additional set
of discrete data objects, just like nodes, which we call "data". Data points
in the data_FE_region share the same fields as <fe_region>.
Returns NULL with error if <fe_region> is itself a data region.
==============================================================================*/

int DESTROY(FE_region)(struct FE_region **fe_region_address);
/*******************************************************************************
LAST MODIFIED : 19 October 2002

DESCRIPTION :
Frees the memory for the FE_region and sets <*fe_region_address> to NULL.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(FE_region);

PROTOTYPE_ANY_OBJECT(FE_region);

int FE_region_begin_change(struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 10 December 2002

DESCRIPTION :
Increments the change counter in <region>. No update callbacks will occur until
change counter is restored to zero by calls to FE_region_end_change.
Automatically calls the same function for any master_FE_region.
==============================================================================*/

int FE_region_end_change(struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 10 December 2002

DESCRIPTION :
Decrements the change counter in <region>. No update callbacks occur until the
change counter is restored to zero by this function.
Automatically calls the same function for any master_FE_region.
==============================================================================*/

int FE_region_add_callback(struct FE_region *fe_region,
	CMISS_CALLBACK_FUNCTION(FE_region_change) *function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 10 December 2002

DESCRIPTION :
Adds a callback to <region> so that when it changes <function> is called with
<user_data>. <function> has 3 arguments, a struct FE_region *, a
struct FE_region_changes * and the void *user_data.
==============================================================================*/

int FE_region_remove_callback(struct FE_region *fe_region,
	CMISS_CALLBACK_FUNCTION(FE_region_change) *function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 10 December 2002

DESCRIPTION :
Removes the callback calling <function> with <user_data> from <region>.
==============================================================================*/

int FE_region_clear(struct FE_region *fe_region, int destroy_in_master);
/*******************************************************************************
LAST MODIFIED : 13 May 2003

DESCRIPTION :
Removes all the fields, nodes and elements from <fe_region>.
If <fe_region> has a master FE_region, its fields, nodes and elements will
still be owned by the master, unless <destroy_in_master> is set, which causes
the nodes and elements in this <fe_region> to definitely be destroyed.
Note this function uses FE_region_begin/end_change so it sends a single change
message if not already in the middle of changes.
???RC This could be made faster.
==============================================================================*/

struct FE_field *FE_region_get_FE_field_from_name(struct FE_region *fe_region,
	char *field_name);
/*******************************************************************************
LAST MODIFIED : 15 October 2002

DESCRIPTION :
Returns the field of <field_name> in <fe_region>, or NULL without error if none.
==============================================================================*/

int FE_region_contains_FE_field(struct FE_region *fe_region,
	struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 10 March 2003

DESCRIPTION :
Returns true if <field> is in <fe_region>.
==============================================================================*/

struct FE_field *FE_region_get_first_FE_field_that(struct FE_region *fe_region,
	LIST_CONDITIONAL_FUNCTION(FE_field) *conditional_function,
	void *user_data_void);
/*******************************************************************************
LAST MODIFIED : 25 March 2003

DESCRIPTION :
Returns the first field in <fe_region> that satisfies <conditional_function>
with <user_data_void>.
A NULL <conditional_function> returns the first FE_field in <fe_region>, if any.
==============================================================================*/

int FE_region_for_each_FE_field(struct FE_region *fe_region,
	LIST_ITERATOR_FUNCTION(FE_field) iterator_function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 15 January 2003

DESCRIPTION :
Calls <iterator_function> with <user_data> for each FE_field in <region>.
==============================================================================*/

int FE_region_get_number_of_FE_fields(struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 10 January 2003

DESCRIPTION :
Returns the number of FE_fields in <fe_region>.
==============================================================================*/

struct FE_field *FE_region_get_FE_field_with_properties(
	struct FE_region *fe_region, char *name, enum FE_field_type fe_field_type,
	struct FE_field *indexer_field, int number_of_indexed_values,
	enum CM_field_type cm_field_type, struct Coordinate_system *coordinate_system,
	enum Value_type value_type, int number_of_components, char **component_names,
	int number_of_times, enum Value_type time_value_type,
	struct FE_field_external_information *external);
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Returns an FE_field with the given <name> merged into <fe_region> and with the
given properties. If a field of the given <name> already exists, checks that
it has the same properties -- if not an error is reported. If no field of that
name exists, one is created and FE_region_merge_FE_field called for it.
Hence, this function may result in change messages being sent, so use
begin/end change if several calls are to be made.
==============================================================================*/

struct FE_field *FE_region_merge_FE_field(struct FE_region *fe_region,
	struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
Checks <fe_field> is compatible with <fe_region> and any existing FE_field
using the same identifier, then merges it into <fe_region>.
If no FE_field of the same identifier exists in FE_region, <fe_field> is added
to <fe_region> and returned by this function, otherwise changes are merged into
the existing FE_field and it is returned.
A NULL value is returned on any error.
==============================================================================*/

int FE_region_is_FE_field_in_use(struct FE_region *fe_region,
	struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 13 May 2003

DESCRIPTION :
Returns true if <FE_field> is defined on any nodes and element in <fe_region>.
==============================================================================*/

int FE_region_remove_FE_field(struct FE_region *fe_region,
	struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Removes <fe_field> from <fe_region>.
Fields can only be removed if not defined on any nodes and element in
<fe_region>.
==============================================================================*/

int set_FE_field_component_FE_region(struct Parse_state *state,
	void *fe_field_component_address_void, void *fe_region_void);
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
FE_region wrapper for set_FE_field_component.
==============================================================================*/

struct Set_FE_field_conditional_FE_region_data
/*******************************************************************************
LAST MODIFIED : 27 February 2003

DESCRIPTION :
User data structure passed to set_FE_field_conditional_FE_region.
==============================================================================*/
{
	LIST_CONDITIONAL_FUNCTION(FE_field) *conditional_function;
	void *user_data;
	struct FE_region *fe_region;
}; /* struct Set_FE_field_conditional_FE_region_data */

int set_FE_field_conditional_FE_region(struct Parse_state *state,
	void *fe_field_address_void, void *parse_field_data_void);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
FE_region wrapper for set_FE_field_conditional. <parse_field_data_void> points
at a struct Set_FE_field_conditional_FE_region_data.
==============================================================================*/

int set_FE_fields_FE_region(struct Parse_state *state,
	void *fe_field_order_info_address_void, void *fe_region_void);
/*******************************************************************************
LAST MODIFIED : 7 March 2003

DESCRIPTION :
FE_region wrapper for set_FE_fields.
==============================================================================*/

int Option_table_add_set_FE_field_from_FE_region(
	struct Option_table *option_table, const char *entry_string,
	struct FE_field **fe_field_address, struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 11 March 2003

DESCRIPTION :
Adds an entry for selecting an FE_field.
==============================================================================*/

int FE_region_FE_field_has_multiple_times(struct FE_region *fe_region,
	struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 26 February 2003

DESCRIPTION :
Returns true if <fe_field> is defined with multiple times on any node or element
in the ultimate master FE_region of <fe_region>.
==============================================================================*/

int FE_region_get_default_coordinate_FE_field(struct FE_region *fe_region,
	struct FE_field **fe_field);
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Returns an <fe_field> which the <fe_region> considers to be its default
coordinate field or returns 0 and sets *<fe_field> to NULL if it has no 
"coordinate" fields.
==============================================================================*/

int FE_region_change_FE_node_identifier(struct FE_region *fe_region,
	struct FE_node *node, int new_identifier);
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Gets <fe_region>, or its master_fe_region if it has one, to change the
identifier of <node> to <new_identifier>. Fails if the identifier is already
in use by an node in the same ultimate master FE_region.
???RC Needs recoding if FE_node changed from using indexed list.
==============================================================================*/

struct FE_node *FE_region_get_FE_node_from_identifier(
	struct FE_region *fe_region, int identifier);
/*******************************************************************************
LAST MODIFIED : 25 February 2003

DESCRIPTION :
Returns the node of number <identifier> in <fe_region>, or NULL without error
if no such node found.
==============================================================================*/

struct FE_node *FE_region_get_or_create_FE_node_with_identifier(
	struct FE_region *fe_region, int identifier);
/*******************************************************************************
LAST MODIFIED : 27 May 2003

DESCRIPTION :
Convenience function returning an existing node with <identifier> from
<fe_region> or any of its ancestors. If none is found, a new node with the
given <identifier> is created.
If the returned node is not already in <fe_region> it is merged before return.
It is expected that the calling function has wrapped calls to this function
with FE_region_begin/end_change.
==============================================================================*/

int FE_region_get_next_FE_node_identifier(struct FE_region *fe_region,
	int start_identifier);
/*******************************************************************************
LAST MODIFIED : 25 February 2003

DESCRIPTION :
Returns the next unused node identifier for <fe_region> starting from
<start_identifier>. Search is performed on the ultimate master FE_region for
<fe_region> since they share the same FE_node namespace.
==============================================================================*/

int FE_region_contains_FE_node(struct FE_region *fe_region,
	struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 9 December 2002

DESCRIPTION :
Returns true if <node> is in <fe_region>.
==============================================================================*/

int FE_region_contains_FE_node_conditional(struct FE_node *node,
	void *fe_region_void);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
FE_node conditional function version of FE_region_contains_FE_node.
Returns true if <node> is in <fe_region>.
==============================================================================*/

int FE_region_or_data_FE_region_contains_FE_node(struct FE_region *fe_region,
	struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 26 April 2005

DESCRIPTION :
Returns true if <node> is in <fe_region> or if the <fe_region> has a
data_FE_region attached to it in that attached region.
==============================================================================*/

int FE_node_is_not_in_FE_region(struct FE_node *node, void *fe_region_void);
/*******************************************************************************
LAST MODIFIED : 3 April 2003

DESCRIPTION :
Returns true if <node> is not in <fe_region>.
==============================================================================*/

int FE_region_define_FE_field_at_FE_node(struct FE_region *fe_region,
	struct FE_node *node, struct FE_field *fe_field,
	struct FE_time_sequence *fe_time_sequence,
	struct FE_node_field_creator *node_field_creator);
/*******************************************************************************
LAST MODIFIED : 28 April 2003

DESCRIPTION :
Checks <fe_region> contains <node>. If <fe_field> is already defined on it,
returns successfully, otherwise defines the field at the node using optional
<fe_time_sequence> and <node_field_creator>. Change messages are broadcast for
the ultimate master FE_region of <fe_region>.
Should place multiple calls to this function between begin_change/end_change.
==============================================================================*/

int FE_region_undefine_FE_field_at_FE_node(struct FE_region *fe_region,
	struct FE_node *node, struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 28 April 2003

DESCRIPTION :
Checks <fe_region> contains <node>. If <fe_field> is not defined on it,
returns successfully, otherwise undefines the field at the node. Change messages
are broadcast for the ultimate master FE_region of <fe_region>.
Should place multiple calls to this function between begin_change/end_change.
Note it is more efficient to use FE_region_undefine_FE_field_in_FE_node_list
for more than one node.
==============================================================================*/

int FE_region_undefine_FE_field_in_FE_node_list(struct FE_region *fe_region,
	struct FE_field *fe_field, struct LIST(FE_node) *fe_node_list,
	int *number_in_elements_address);
/*******************************************************************************
LAST MODIFIED : 29 April 2003

DESCRIPTION :
Makes sure <fe_field> is not defined at any nodes in <fe_node_list> from
<fe_region>, unless that field at the node is in interpolated over an element
from <fe_region>.
Should wrap call to this function between begin_change/end_change.
<fe_node_list> is unchanged by this function.
On return, the integer at <number_in_elements_address> will be set to the
number of nodes for which <fe_field> could not be undefined because they are
used by element fields of <fe_field>.
==============================================================================*/

struct FE_node *FE_region_merge_FE_node(struct FE_region *fe_region,
	struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
Checks <node> is compatible with <fe_region> and any existing FE_node
using the same identifier, then merges it into <fe_region>.
If no FE_node of the same identifier exists in FE_region, <node> is added
to <fe_region> and returned by this function, otherwise changes are merged into
the existing FE_node and it is returned.
During the merge, any new fields from <node> are added to the existing node of
the same identifier. Where those fields are already defined on the existing
node, the existing structure is maintained, but the new values are added from
<node>. Fails if fields are not consistent in numbers of versions and
derivatives etc.
A NULL value is returned on any error.
==============================================================================*/

int FE_region_merge_FE_node_iterator(struct FE_node *node,
	void *fe_region_void);
/*******************************************************************************
LAST MODIFIED : 15 January 2003

DESCRIPTION :
FE_node iterator version of FE_region_merge_FE_node.
==============================================================================*/

struct FE_node *FE_region_get_first_FE_node_that(struct FE_region *fe_region,
	LIST_CONDITIONAL_FUNCTION(FE_node) *conditional_function,
	void *user_data_void);
/*******************************************************************************
LAST MODIFIED : 25 March 2003

DESCRIPTION :
Returns the first node in <fe_region> that satisfies <conditional_function>
with <user_data_void>.
A NULL <conditional_function> returns the first FE_node in <fe_region>, if any.
==============================================================================*/

int FE_region_for_each_FE_node(struct FE_region *fe_region,
	LIST_ITERATOR_FUNCTION(FE_node) iterator_function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 12 November 2002

DESCRIPTION :
Calls <iterator_function> with <user_data> for each FE_node in <region>.
==============================================================================*/

int FE_region_for_each_FE_node_conditional(struct FE_region *fe_region,
	LIST_CONDITIONAL_FUNCTION(FE_node) conditional_function,
	void *conditional_user_data,
	LIST_ITERATOR_FUNCTION(FE_node) iterator_function,
	void *iterator_user_data);
/*******************************************************************************
LAST MODIFIED : 15 January 2003

DESCRIPTION :
For each FE_node in <fe_region> satisfying <conditional_function> with
<conditional_user_data>, calls <iterator_function> with it and
<iterator_user_data> as arguments.
==============================================================================*/

int FE_region_remove_FE_node(struct FE_region *fe_region,
	struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Removes <node> from <fe_region>.
Nodes can only be removed if not in use by elements in <fe_region>.
Should enclose call between FE_region_begin_change and FE_region_end_change to
minimise messages.
Note it is more efficient to use FE_region_remove_FE_node_list for more than
one node.
==============================================================================*/

int FE_region_remove_FE_node_list(struct FE_region *fe_region,
	struct LIST(FE_node) *node_list);
/*******************************************************************************
LAST MODIFIED : 14 May 2003

DESCRIPTION :
Attempts to removes all the nodes in <node_list> from <fe_region>.
Nodes can only be removed if not in use by elements in <fe_region>.
Should enclose call between FE_region_begin_change and FE_region_end_change to
minimise messages.
On return, <node_list> will contain all the nodes that are still in
<fe_region> after the call.
A true return code is only obtained if all nodes from <node_list> are removed.
==============================================================================*/

int FE_region_get_number_of_FE_nodes(struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Returns the number of FE_nodes in <fe_region>.
==============================================================================*/

struct FE_node *FE_region_node_string_to_FE_node(
	struct FE_region *fe_region, char *node_string);
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Returns the node from <fe_region> whose number is in the string <name>.
==============================================================================*/

int set_FE_node_FE_region(struct Parse_state *state, void *node_address_void,
	void *fe_region_void);
/*******************************************************************************
LAST MODIFIED : 25 February 2003

DESCRIPTION :
Used in command parsing to translate a node name into an node from <fe_region>.
==============================================================================*/

struct MANAGER(FE_basis) *FE_region_get_basis_manager(
	struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 14 January 2003

DESCRIPTION :
Returns the FE_basis_manager used for bases in this <fe_region>.
==============================================================================*/

int FE_region_get_immediate_master_FE_region(struct FE_region *fe_region,
	struct FE_region **master_fe_region_address);
/*******************************************************************************
LAST MODIFIED : 18 February 2003

DESCRIPTION :
Returns the master_fe_region for this <fe_region>, which is NULL if the region
has its own namespace for fields, nodes and elements.
Function is not recursive; returns only the master FE_region for <fe_region>
without enquiring for that of its master.
See also FE_region_get_ultimate_master_FE_region.
==============================================================================*/

int FE_region_get_ultimate_master_FE_region(struct FE_region *fe_region,
	struct FE_region **master_fe_region_address);
/*******************************************************************************
LAST MODIFIED : 18 February 2003

DESCRIPTION :
Returns the FE_region that the fields, nodes and elements of <fe_region>
ultimately belong to. <fe_region> is returned if it has no immediate master.
==============================================================================*/

struct LIST(FE_element_shape) *FE_region_get_FE_element_shape_list(
	struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 7 July 2003

DESCRIPTION :
Returns the LIST of FE_element_shapes used for elements in this <fe_region>.
==============================================================================*/

int FE_region_change_FE_element_identifier(
	struct FE_region *fe_region, struct FE_element *element,
	struct CM_element_information *new_identifier);
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Gets <fe_region>, or its master_fe_region if it has one, to change the
identifier of <element> to <new_identifier>. Fails if the identifier is already
in use by an element in the same ultimate master FE_region.
==============================================================================*/

struct FE_element *FE_region_get_FE_element_from_identifier(
	struct FE_region *fe_region, struct CM_element_information *identifier);
/*******************************************************************************
LAST MODIFIED : 22 October 2002

DESCRIPTION :
Returns the element identified by <cm> in <fe_region>, or NULL without error if
no such element found.
==============================================================================*/

struct FE_element *FE_region_get_or_create_FE_element_with_identifier(
	struct FE_region *fe_region, struct CM_element_information *identifier,
	int dimension);
/*******************************************************************************
LAST MODIFIED : 27 May 2003

DESCRIPTION :
Convenience function returning an existing element with <identifier> from
<fe_region> or any of its ancestors. Existing elements are checked against the
<dimension> and no element is returned if the dimension is different.
If no existing element is found, a new element with the given <identifier> and
and unspecified shape of the given <dimension> is created.
If the returned element is not already in <fe_region> it is merged before
return.
It is expected that the calling function has wrapped calls to this function
with FE_region_begin/end_change.
???RC Could eventually allow the shape of newly created elements to be other
than 'unspecified'.
==============================================================================*/

int FE_region_get_next_FE_element_identifier(struct FE_region *fe_region,
	enum CM_element_type element_type, int start_identifier);
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Returns the next unused element number for elements of <element_type> in
<fe_region> starting from <start_identifier>.
Search is performed on the ultimate master FE_region for <fe_region> since they
share the same FE_element namespace.
==============================================================================*/

int FE_region_get_number_of_FE_elements(struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
Returns the number of elements in <fe_region>.
==============================================================================*/

int FE_region_contains_FE_element(struct FE_region *fe_region,
	struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 9 December 2002

DESCRIPTION :
Returns true if <element> is in <fe_region>.
==============================================================================*/

int FE_region_contains_FE_element_conditional(struct FE_element *element,
	void *fe_region_void);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
FE_element conditional function version of FE_region_contains_FE_element.
Returns true if <element> is in <fe_region>.
==============================================================================*/

int FE_element_is_not_in_FE_region(struct FE_element *element,
	void *fe_region_void);
/*******************************************************************************
LAST MODIFIED : 3 April 2003

DESCRIPTION :
Returns true if <element> is not in <fe_region>.
==============================================================================*/

int FE_region_define_FE_field_at_element(struct FE_region *fe_region,
	struct FE_element *element, struct FE_field *fe_field,
	struct FE_element_field_component **components);
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Checks <fe_region> contains <element>. If <fe_field> is already defined on it,
returns successfully, otherwise defines the field at the element using the
<components>. Change messages are broadcast for the ultimate master FE_region
of <fe_region>.
Should place multiple calls to this function between begin_change/end_change.
==============================================================================*/

struct FE_element *FE_region_merge_FE_element(struct FE_region *fe_region,
	struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
Checks <element> is compatible with <fe_region> and any existing FE_element
using the same identifier, then merges it into <fe_region>.
If no FE_element of the same identifier exists in FE_region, <element> is added
to <fe_region> and returned by this function, otherwise changes are merged into
the existing FE_element and it is returned.
During the merge, any new fields from <element> are added to the existing
element of the same identifier. Where those fields are already defined on the
existing element, the existing structure is maintained, but the new values are
added from <element>. Fails if fields are not consistently defined.
A NULL value is returned on any error.
==============================================================================*/

int FE_region_begin_define_faces(struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 13 January 2003

DESCRIPTION :
Sets up <fe_region> to automatically define faces on any elements merged into
it, and their faces recursively.
Call FE_region_end_define_faces as soon as face definition is finished.
Should put face definition calls between calls to begin_change/end_change.
==============================================================================*/

int FE_region_end_define_faces(struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 13 January 2003

DESCRIPTION :
Ends face definition in <fe_region>. Cleans up internal cache.
==============================================================================*/

int FE_region_merge_FE_element_and_faces_and_nodes(struct FE_region *fe_region,
	struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 14 May 2003

DESCRIPTION :
Version of FE_region_merge_FE_element that merges not only <element> into
<fe_region> but any of its faces that are defined.
<element> and any of its faces may already be in <fe_region>.
Also merges nodes referenced directly by <element> and its parents, if any.

- MUST NOT iterate over the list of elements in a region to add or define faces
as the list will be modified in the process; copy the list and iterate over
the copy.

FE_region_begin/end_change are called internally to reduce change messages to
one per call. User should place calls to the begin/end_change functions around
multiple calls to this function.

If calls to this function are placed between FE_region_begin/end_define_faces,
then any missing faces are created and also merged into <fe_region>.
Function ensures that elements share existing faces and lines in preference to
creating new ones if they have matching shape and nodes.

???RC Can only match faces correctly for coordinate fields with standard node
to element maps and no versions. A grid-based coordinate field would fail
utterly since it has no nodes. A possible future solution for all cases is to
match the geometry exactly either by using the FE_element_field_values
(coefficients of the monomial basis functions), although there is a problem with
xi-directions not matching up, or actual centre positions of the face being a
trivial rejection, narrowing down to a single face or list of faces to compare
against.
==============================================================================*/

int FE_region_merge_FE_element_and_faces_and_nodes_iterator(
	struct FE_element *element, void *fe_region_void);
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
FE_element iterator version of FE_region_merge_FE_element_and_faces_and_nodes.
- MUST NOT iterate over the list of elements in a region to add or define faces
as the list will be modified in the process; copy the list and iterate over
the copy.
==============================================================================*/

int FE_region_get_FE_element_discretization(struct FE_region *fe_region,
	struct FE_element *element, int face_number,
	struct FE_field *native_discretization_field,
	int *top_level_number_in_xi,struct FE_element **top_level_element,
	int *number_in_xi);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
FE_region wrapper for get_FE_element_discretization.
???RC Currently <fe_region> may be omitted to place no restriction on what
parent can be inherited from. This function will need to be tightened later.
==============================================================================*/

int FE_region_remove_FE_element(struct FE_region *fe_region,
	struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 14 May 2003

DESCRIPTION :
Removes <element> and all its faces that are not shared with other elements
from <fe_region>.
FE_region_begin/end_change are called internally to reduce change messages to
one per call. User should place calls to the begin/end_change functions around
multiple calls to this function.
Can only remove faces and lines if there is no master_fe_region or no parents
in this fe_region.
This function is recursive.
==============================================================================*/

int FE_region_remove_FE_element_list(struct FE_region *fe_region,
	struct LIST(FE_element) *element_list);
/*******************************************************************************
LAST MODIFIED : 14 May 2003

DESCRIPTION :
Attempts to removes all the elements in <element_list>, and all their faces that
are not shared with other elements from <fe_region>.
FE_region_begin/end_change are called internally to reduce change messages to
one per call. User should place calls to the begin/end_change functions around
multiple calls to this function.
Can only remove faces and lines if there is no master_fe_region or no parents
in this fe_region.
On return, <element_list> will contain all the elements that are still in
<fe_region> after the call.
A true return code is only obtained if all elements from <element_list> are
removed.
==============================================================================*/

int FE_region_element_or_parent_has_field(struct FE_region *fe_region,
	struct FE_element *element, struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 12 November 2002

DESCRIPTION :
Returns true if <element> is in <fe_region> and has <field> defined on it or
any parents also in <fe_region>.
==============================================================================*/

struct FE_element *FE_region_get_first_FE_element_that(
	struct FE_region *fe_region,
	LIST_CONDITIONAL_FUNCTION(FE_element) *conditional_function,
	void *user_data_void);
/*******************************************************************************
LAST MODIFIED : 25 March 2003

DESCRIPTION :
Returns the first element in <fe_region> that satisfies <conditional_function>
with <user_data_void>.
A NULL <conditional_function> returns the first FE_element in <fe_region>,
if any.
==============================================================================*/

int FE_region_for_each_FE_element(struct FE_region *fe_region,
	LIST_ITERATOR_FUNCTION(FE_element) iterator_function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 15 January 2003

DESCRIPTION :
Calls <iterator_function> with <user_data> for each FE_element in <region>.
==============================================================================*/

int FE_region_for_each_FE_element_conditional(struct FE_region *fe_region,
	LIST_CONDITIONAL_FUNCTION(FE_element) conditional_function,
	void *conditional_user_data,
	LIST_ITERATOR_FUNCTION(FE_element) iterator_function,
	void *iterator_user_data);
/*******************************************************************************
LAST MODIFIED : 15 January 2003

DESCRIPTION :
For each FE_element in <fe_region> satisfying <conditional_function> with
<conditional_user_data>, calls <iterator_function> with it and
<iterator_user_data> as arguments.
==============================================================================*/

int FE_region_FE_element_meets_topological_criteria(struct FE_region *fe_region,
	struct FE_element *element, int dimension,
	enum CM_element_type cm_element_type, int exterior, int face_number);
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Calls FE_element_meets_topological_criteria with the <element_list> private to
<fe_region>.
==============================================================================*/

struct FE_element *FE_region_element_string_to_FE_element(
	struct FE_region *fe_region, char *name);
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Calls element_string_to_FE_element with the <element_list> private to
<fe_region>.
==============================================================================*/

struct FE_element *FE_region_any_element_string_to_FE_element(
	struct FE_region *fe_region, char *name);
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Calls any_element_string_to_FE_element with the <element_list> private to
<fe_region>.
==============================================================================*/

int set_FE_element_dimension_3_FE_region(struct Parse_state *state,
	void *element_address_void, void *fe_region_void);
/*******************************************************************************
LAST MODIFIED : 26 February 2003

DESCRIPTION :
A modifier function for specifying a 3-D element (as opposed to a 2-D face or
1-D line number), used (eg.) to set the seed element for a volume texture.
==============================================================================*/

int set_FE_element_top_level_FE_region(struct Parse_state *state,
	void *element_address_void, void *fe_region_void);
/*******************************************************************************
LAST MODIFIED : 26 February 2003

DESCRIPTION :
A modifier function for specifying a top level element, used, for example, to
set the seed element for a xi_texture_coordinate computed_field.
==============================================================================*/

int FE_region_smooth_FE_field(struct FE_region *fe_region,
	struct FE_field *fe_field, FE_value time);
/*******************************************************************************
LAST MODIFIED : 12 March 2003

DESCRIPTION :
Smooths node-based <fe_field> over its nodes and elements in <fe_region>.
==============================================================================*/

struct FE_time_sequence *FE_region_get_FE_time_sequence_matching_series(
	struct FE_region *fe_region, int number_of_times, FE_value *times);
/*******************************************************************************
LAST MODIFIED : 20 February 2003

DESCRIPTION :
Finds or creates a struct FE_time_sequence in <fe_region> with the given
<number_of_times> and <times>.
==============================================================================*/

struct FE_time_sequence *FE_region_get_FE_time_sequence_merging_two_time_series(
	struct FE_region *fe_region, struct FE_time_sequence *time_sequence_one,
	struct FE_time_sequence *time_sequence_two);
/*******************************************************************************
LAST MODIFIED : 20 February 2003

DESCRIPTION :
Finds or creates a struct FE_time_sequence in <fe_region> which has the list of
times formed by merging the two time_sequences supplied.
==============================================================================*/

struct FE_basis *FE_region_get_FE_basis_matching_basis_type(
	struct FE_region *fe_region, int *basis_type);
/*******************************************************************************
LAST MODIFIED : 29 October 2002

DESCRIPTION :
Finds or creates a struct FE_basis in <fe_region> with the given <basis_type>.
Recursive if fe_region has a master_fe_region.
==============================================================================*/

int Cmiss_region_attach_FE_region(struct Cmiss_region *cmiss_region,
	struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 29 October 2002

DESCRIPTION :
Adds <fe_region> to the list of objects attached to <cmiss_region>.
To clean up, <fe_region> will be deaccessed when detached from the Cmiss_region.
If <cmiss_region> already has an FE_region, an error is reported.
This function returns it, or NULL if no FE_region found.
==============================================================================*/

struct FE_region *Cmiss_region_get_FE_region(struct Cmiss_region *cmiss_region);
/*******************************************************************************
LAST MODIFIED : 1 October 2002

DESCRIPTION :
Currently, a Cmiss_region may have at most one FE_region.
This function returns it, or NULL if no FE_region found.
==============================================================================*/

int FE_region_get_Cmiss_region(struct FE_region *fe_region,
	struct Cmiss_region **cmiss_region_address);
/*******************************************************************************
LAST MODIFIED : 12 November 2002

DESCRIPTION :
Returns in <cmiss_region_address> either the owning <cmiss_region> for
<fe_region> or NULL if none/error.
==============================================================================*/

int FE_regions_can_be_merged(struct FE_region *global_fe_region,
	struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 20 November 2002

DESCRIPTION :
Returns true if definitions of fields, nodes and elements in <fe_region> are
compatible with those in <global_fe_region>, such that FE_region_merge_FE_region
should succeed. Neither region is modified.
Note order: <fe_region> is to be merged into <global_fe_region>.
=============================================================================*/

int Cmiss_regions_FE_regions_can_be_merged(struct Cmiss_region *global_region,
	struct Cmiss_region *region);
/*******************************************************************************
LAST MODIFIED : 19 November 2002

DESCRIPTION :
Returns true if the FE_region in <region> can be merged into the FE_region for
<global_region>, and, recursively, if the same function returns true for all
child regions with the same name.
==============================================================================*/

int Cmiss_regions_merge_FE_regions(struct Cmiss_region *global_region,
	struct Cmiss_region *region);
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
Merges into <global_region> the fields, nodes and elements from <region>.
It is expected that Cmiss_regions_FE_regions_can_be_merged has already been
passed for the two regions; if so the merge should proceed without error.
The can_be_merged check should not be necessary if the members of <region> have
been extracted from <global_region> in the first place and only field values
changed or new objects added.

IMPORTANT NOTE:
Caller should destroy <region> after calling this function since the FE_regions
it contains will be significantly modified during the merge. It would be more
expensive to keep FE_regions unchanged during the merge, a behaviour not
required at this time for import functions. If this is required in future,
FE_regions_merge would have to be changed.
==============================================================================*/

#endif /* !defined (FINITE_ELEMENT_REGION_H) */
