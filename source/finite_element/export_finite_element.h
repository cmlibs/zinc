/*******************************************************************************
FILE : export_finite_element.h

LAST MODIFIED : 4 October 1999

DESCRIPTION :
The function prototypes for importing finite element data, from a file or cmiss
(via a socket) into the graphical interface to CMISS.
==============================================================================*/
#if !defined (EXPORT_FINITE_ELEMENT_H)
#define EXPORT_FINITE_ELEMENT_H

#include <stdio.h>
#include "finite_element/finite_element.h"

/*
Global/Public types
-------------------
*/
struct Fwrite_FE_element_group_data
/*******************************************************************************
LAST MODIFIED : 30 March 1999

DESCRIPTION :
==============================================================================*/
{
	struct FE_field *field;
	struct GROUP(FE_element) *element_group;
}; /* struct File_write_FE_element_group_data */

struct Fwrite_all_FE_element_groups_data
/*******************************************************************************
LAST MODIFIED : 4 October 1999

DESCRIPTION :
The active group is not written out unless asked for explicitly
==============================================================================*/
{
	struct FE_field *field;
	struct GROUP(FE_element) *active_element_group;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
}; /* struct File_write_FE_element_group_data */

struct Fwrite_FE_node_group_data
/*******************************************************************************
LAST MODIFIED : 30 April 1998

DESCRIPTION :
==============================================================================*/
{
	struct FE_field *field;
	struct GROUP(FE_node) *node_group;
}; /* struct Fwrite_FE_node_group_data */

struct Fwrite_all_FE_node_groups_data
/*******************************************************************************
LAST MODIFIED : 4 October 1999

DESCRIPTION :
==============================================================================*/
{
	struct FE_field *field;
	struct GROUP(FE_node) *active_node_group;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
}; /* struct Fwrite_all_FE_node_groups_data */

/*
Global/Public functions
-----------------------
*/
int write_FE_element_group(FILE *output_file,
	struct GROUP(FE_element) *element_group,struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 30 March 1999

DESCRIPTION :
Writes an element group to an <output_file> or the socket (if <output_file> is
NULL).
==============================================================================*/

int file_write_FE_element_group(char *file_name,void *data_void);
/*******************************************************************************
LAST MODIFIED : 30 March 1999

DESCRIPTION :
Writes an element group to a file.
==============================================================================*/

int file_write_all_FE_element_groups(char *file_name,void *data_void);
/*******************************************************************************
LAST MODIFIED : 30 March 1999

DESCRIPTION :
Writes all existing element groups to a file.
==============================================================================*/

int write_FE_node_group(FILE *output_file,struct GROUP(FE_node) *node_group,
	struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 30 April 1998

DESCRIPTION :
Writes a node group to an <output_file> or the socket (if <output_file> is
NULL).
==============================================================================*/

int file_write_FE_node_group(char *file_name,void *data_void);
/*******************************************************************************
LAST MODIFIED : 30 April 1998

DESCRIPTION :
Writes a node group to a file.
==============================================================================*/

int file_write_all_FE_node_groups(char *file_name,void *data_void);
/*******************************************************************************
LAST MODIFIED : 30 April 1998

DESCRIPTION :
Writes all existing node groups to a file.
==============================================================================*/
#endif /* !defined (EXPORT_FINITE_ELEMENT_H) */
