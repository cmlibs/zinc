/*******************************************************************************
FILE : export_finite_element.h

LAST MODIFIED : 7 September 2001

DESCRIPTION :
The function prototypes for importing finite element data, from a file or cmiss
(via a socket) into the graphical interface to CMISS.
==============================================================================*/
#if !defined (EXPORT_FINITE_ELEMENT_H)
#define EXPORT_FINITE_ELEMENT_H

#include <stdio.h>
#include "finite_element/finite_element.h"
#include "general/enumerator.h"

/*
Global/Public types
-------------------
*/

enum FE_write_criterion
/*******************************************************************************
LAST MODIFIED : 5 September 2001

DESCRIPTION :
Controls output of nodes/elements to exnode/exelem files as follows:
FE_WRITE_COMPLETE_GROUP = write all object in the group (the default);
FE_WRITE_WITH_ALL_LISTED_FIELDS =
  write only objects with all listed fields defined;
FE_WRITE_WITH_ANY_LISTED_FIELDS =
  write only objects with any listed fields defined.
==============================================================================*/
{
	FE_WRITE_COMPLETE_GROUP,
	FE_WRITE_WITH_ALL_LISTED_FIELDS,
	FE_WRITE_WITH_ANY_LISTED_FIELDS
};

struct Fwrite_FE_element_group_data
/*******************************************************************************
LAST MODIFIED : 7 September 2001

DESCRIPTION :
==============================================================================*/
{
	enum FE_write_criterion write_criterion;
	struct FE_field_order_info *field_order_info;
	struct GROUP(FE_element) *element_group;
}; /* struct File_write_FE_element_group_data */

struct Fwrite_all_FE_element_groups_data
/*******************************************************************************
LAST MODIFIED : 7 September 2001

DESCRIPTION :
==============================================================================*/
{
	enum FE_write_criterion write_criterion;
	struct FE_field_order_info *field_order_info;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
}; /* struct File_write_FE_element_group_data */

struct Fwrite_FE_node_group_data
/*******************************************************************************
LAST MODIFIED : 5 September 2001

DESCRIPTION :
==============================================================================*/
{
	enum FE_write_criterion write_criterion;
	struct FE_field_order_info *field_order_info;
	struct GROUP(FE_node) *node_group;
}; /* struct Fwrite_FE_node_group_data */

struct Fwrite_all_FE_node_groups_data
/*******************************************************************************
LAST MODIFIED : 5 September 2001

DESCRIPTION :
==============================================================================*/
{
	enum FE_write_criterion write_criterion;
	struct FE_field_order_info *field_order_info;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
}; /* struct Fwrite_all_FE_node_groups_data */

/*
Global/Public functions
-----------------------
*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(FE_write_criterion);

int write_FE_element_group(FILE *output_file,
	struct GROUP(FE_element) *element_group,
	enum FE_write_criterion write_criterion,
	struct FE_field_order_info *field_order_info);
/*******************************************************************************
LAST MODIFIED : 7 September 2001

DESCRIPTION :
Writes an element group to an <output_file> or the socket (if <output_file> is
NULL).
If <field_order_info> is NULL, all element fields are written in the default,
alphabetical order.
If <field_order_info> is empty, only element identifiers are output.
If <field_order_info> contains fields, they are written in that order.
Additionally, the <write_criterion> controls output as follows:
FE_WRITE_COMPLETE_GROUP = write all elements in the group (the default);
FE_WRITE_WITH_ALL_LISTED_FIELDS =
  write only elements with all listed fields defined;
FE_WRITE_WITH_ANY_LISTED_FIELDS =
  write only elements with any listed fields defined.
==============================================================================*/

int file_write_FE_element_group(char *file_name,void *data_void);
/*******************************************************************************
LAST MODIFIED : 7 September 2001

DESCRIPTION :
Writes an element group to a file.
<data_void> should point to a struct Fwrite_FE_element_group_data.
==============================================================================*/

int file_write_all_FE_element_groups(char *file_name,void *data_void);
/*******************************************************************************
LAST MODIFIED : 7 September 2001

DESCRIPTION :
Writes all existing element groups to a file.
<data_void> should point to a struct Fwrite_all_FE_element_groups_data.
==============================================================================*/

int write_FE_node_group(FILE *output_file, struct GROUP(FE_node) *node_group,
	enum FE_write_criterion write_criterion,
	struct FE_field_order_info *field_order_info);
/*******************************************************************************
LAST MODIFIED : 6 September 2001

DESCRIPTION :
Writes a node group to an <output_file>.
If <field_order_info> is NULL, all node fields are written in the default,
alphabetical order.
If <field_order_info> is empty, only node identifiers are output.
If <field_order_info> contains fields, they are written in that order.
Additionally, the <write_criterion> controls output as follows:
FE_WRITE_COMPLETE_GROUP = write all nodes in the group (the default);
FE_WRITE_WITH_ALL_LISTED_FIELDS =
  write only nodes with all listed fields defined;
FE_WRITE_WITH_ANY_LISTED_FIELDS =
  write only nodes with any listed fields defined.
==============================================================================*/

int file_write_FE_node_group(char *file_name, void *data_void);
/*******************************************************************************
LAST MODIFIED : 5 September 2001

DESCRIPTION :
Writes a node group to a file.
<data_void> should point to a struct Fwrite_FE_node_group_data.
==============================================================================*/

int file_write_all_FE_node_groups(char *file_name, void *data_void);
/*******************************************************************************
LAST MODIFIED : 5 September 2001

DESCRIPTION :
Writes all existing node groups to a file.
<data_void> should point to a struct Fwrite_all_FE_node_groups_data.
==============================================================================*/

#endif /* !defined (EXPORT_FINITE_ELEMENT_H) */
