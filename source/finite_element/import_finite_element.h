/*******************************************************************************
FILE : import_finite_element.h

LAST MODIFIED : 6 March 2000

DESCRIPTION :
The function prototypes for importing finite element data, from a file or cmiss
(via a socket) into the graphical interface to CMISS.
==============================================================================*/
#if !defined (IMPORT_FINITE_ELEMENT_H)
#define IMPORT_FINITE_ELEMENT_H

#include <stdio.h>
#include "finite_element/finite_element.h"

/*
Global types
------------
*/
struct Node_time_index
/*******************************************************************************
LAST MODIFIED : 31 October 2001

DESCRIPTION :
Points to a specific time value which the nodes represent
==============================================================================*/
{
	float time;
}; /* Node_time_index */

struct FE_node_order_info;

struct File_read_FE_node_group_data
/*******************************************************************************
LAST MODIFIED : 6 September 1999

DESCRIPTION :
Data needed by file_read_FE_node_group.
???RC Need data_group_manager, node_group_manager and element_group_manager
since groups of the same name in each manager are created simultaneously, and
node groups are updated to contain nodes used by elements in associated group.
==============================================================================*/
{
	struct MANAGER(FE_field) *fe_field_manager;
	struct FE_time *fe_time;
	struct MANAGER(GROUP(FE_element))	*element_group_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager,*node_group_manager;
	struct MANAGER(FE_element) *element_manager;
}; /* File_read_FE_node_group_data */

struct File_read_FE_element_group_data
/*******************************************************************************
LAST MODIFIED : 22 April 1999

DESCRIPTION :
Data needed by file_read_FE_element_group.
???RC Need data_group_manager, node_group_manager and element_group_manager
since groups of the same name in each manager are created simultaneously, and
node groups are updated to contain nodes used by elements in associated group.
==============================================================================*/
{
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	struct FE_time *fe_time;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager,*node_group_manager;
	struct MANAGER(FE_basis) *basis_manager;
}; /* File_read_FE_element_group_data */

/*
Global functions
----------------
*/
int read_FE_node_group_with_order(FILE *input_file,
	struct MANAGER(FE_field) *fe_field_manager, struct FE_time *fe_time,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct FE_node_order_info *node_order_info,
	struct Node_time_index *node_time_index);
/*******************************************************************************
LAST MODIFIED : 31 October 2001

DESCRIPTION :
Reads node groups from an <input_file> or the socket (if <input_file> is NULL).
???RC Needs data_group_manager, node_group_manager and element_group_manager
since groups of the same name in each manager are created simultaneously, and
node groups are updated to contain nodes used by elements in associated group.
If the <node_order_info> is non NULL then each node is added to this object
in the order of the file.
If the <node_time_index> is non NULL then the values in this read are assumed
to belong to the specified time.  This means that the nodal values will be read
into an array and the correct index put into the corresponding time array.
==============================================================================*/

int read_FE_node_group(FILE *input_file,
	struct MANAGER(FE_field) *fe_field_manager, struct FE_time *fe_time,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager);
/*******************************************************************************
LAST MODIFIED : 6 September 1999

DESCRIPTION :
Reads node groups from an <input_file> or the socket (if <input_file> is NULL).
???RC Needs data_group_manager, node_group_manager and element_group_manager
since groups of the same name in each manager are created simultaneously, and
node groups are updated to contain nodes used by elements in associated group.
==============================================================================*/

int file_read_FE_node_group(char *file_name,void *data_void);
/*******************************************************************************
LAST MODIFIED : 22 April 1999

DESCRIPTION :
Reads a node group from a file.
==============================================================================*/

int read_FE_element_group(FILE *input_file,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_field) *fe_field_manager, struct FE_time *fe_time,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct MANAGER(FE_basis) *basis_manager);
/*******************************************************************************
LAST MODIFIED : 22 April 1999

DESCRIPTION :
Reads an element group from an <input_file> or the socket (if <input_file> is
NULL).
???RC Needs data_group_manager, node_group_manager and element_group_manager
since groups of the same name in each manager are created simultaneously, and
node groups are updated to contain nodes used by elements in associated group.
==============================================================================*/

int file_read_FE_element_group(char *file_name,void *data_void);
/*******************************************************************************
LAST MODIFIED : 19 June 1996

DESCRIPTION :
Reads an element group from a file.
==============================================================================*/

int read_exnode_or_exelem_file_from_string(char *exnode_string,char *exelem_string,
	char *name,struct MANAGER(FE_field) *fe_field_manager, struct FE_time *fe_time,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_node))*node_group_manager,
	struct MANAGER(GROUP(FE_node))*data_group_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_basis) *basis_manager);
/*******************************************************************************
LAST MODIFIED :9 October 2000

DESCRIPTION : given a string <exnode_string> containing an entire exnode file,
XOR a string <exelem_string> containing an entire exelem file, reads in the node 
or element group(s). Renames the first node or element group <name> 
(i.e ignores the first node or element group name in <exnode_string>/<exelem_string>)
Does all this by writing out <exnode_string>/<exelem_string> to a temporary file, 
and reading it back in with read_FE_node_group/read_FE_element_group
This is generally done so we can statically include an exnode or exelem file (in
<exnode_string>/<exelem_string>)
==============================================================================*/

int read_exnode_and_exelem_file_from_string_and_offset(
	char *exnode_string,char *exelem_string,
	char *name,int offset,struct MANAGER(FE_field) *fe_field_manager,
	struct FE_time *fe_time, struct MANAGER(FE_node) *node_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_node))*node_group_manager,
	struct MANAGER(GROUP(FE_node))*data_group_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_basis) *basis_manager);
/*******************************************************************************
LAST MODIFIED : 3 November 2000

DESCRIPTION :
Given a string <exnode_string> containing an entire exnode file,and a string 
<exelem_string> containing an entire exelem file, reads in the node and 
element group(s), names them <name>, and shifts the node and element identifier 
numbers to <offset>.
==============================================================================*/

char *get_first_group_name_from_FE_node_file(char *group_file_name);
/*******************************************************************************
LAST MODIFIED :2 November 2000

DESCRIPTION :
Allocates and returns the name of the first node group, in the exnode file refered 
to by <group_file_name>. It is up to the user to DEALLOCATE the returned 
group name.	
Read in the group name by peering into the node file. This is a bit inelegant,
and will only get the name of the FIRST group, so semi-assuming the exnode file 
only has one group (although it'd be posible to get others). Do this as 
read_FE_node_group doesn't (yet?) return info about the nodes, groups or 
fields .
==============================================================================*/

int read_FE_node_and_elem_groups_and_return_name_given_file_name(
	char *group_file_name,
	struct MANAGER(FE_field) *fe_field_manager, struct FE_time *fe_time,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_node))*node_group_manager,
	struct MANAGER(GROUP(FE_node))*data_group_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_basis) *basis_manager,
	char **group_name);
/*******************************************************************************
LAST MODIFIED :2 November 2000

DESCRIPTION :
Given a string <group_file_name>  reads in the corresponding node and 
element group(s)
Eg if <group_file_name> is "/usr/bob/default_torso", reads in 
"/usr/bob/default_torso.exnode" and "/usr/bob/default_torso.exelem".
Also returns the name of the first node and element group, in <group_name>.
It is up to the user to DEALLOCATE <group_name>.
==============================================================================*/

#endif /* !defined (IMPORT_FINITE_ELEMENT_H) */
