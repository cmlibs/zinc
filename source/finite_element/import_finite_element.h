/*******************************************************************************
FILE : import_finite_element.h

LAST MODIFIED : 6 September 1999

DESCRIPTION :
The function prototypes for importing finite element data, from a file or cmiss
(via a socket) into the graphical interface to CMISS.
==============================================================================*/
#if !defined (IMPORT_FINITE_ELEMENT_H)
#define IMPORT_FINITE_ELEMENT_H

#include "finite_element/finite_element.h"

/*
Global types
------------
*/
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
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager,*node_group_manager;
	struct MANAGER(FE_basis) *basis_manager;
}; /* File_read_FE_element_group_data */

/*
Global functions
----------------
*/
int read_FE_node_group_with_order(FILE *input_file,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct FE_node_order_info *node_order_info);
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Reads node groups from an <input_file> or the socket (if <input_file> is NULL).
???RC Needs data_group_manager, node_group_manager and element_group_manager
since groups of the same name in each manager are created simultaneously, and
node groups are updated to contain nodes used by elements in associated group.
If the <node_order_info> is non NULL then each node is added to this object
in the order of the file.
==============================================================================*/

int read_FE_node_group(FILE *input_file,
	struct MANAGER(FE_field) *fe_field_manager,
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
	struct MANAGER(FE_field) *fe_field_manager,
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

#endif /* !defined (IMPORT_FINITE_ELEMENT_H) */
