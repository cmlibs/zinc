/*******************************************************************************
FILE : export_finite_element.h

LAST MODIFIED : 12 November 2002

DESCRIPTION :
Functions for exporting finite element data to a file.
==============================================================================*/
#if !defined (EXPORT_FINITE_ELEMENT_H)
#define EXPORT_FINITE_ELEMENT_H

#include <stdio.h>
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "general/enumerator.h"
#include "region/cmiss_region.h"

/*
Global/Public types
-------------------
*/

enum FE_write_criterion
/*******************************************************************************
LAST MODIFIED : 7 November 2002

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
	FE_WRITE_NO_FIELDS,
	FE_WRITE_WITH_ALL_LISTED_FIELDS,
	FE_WRITE_WITH_ANY_LISTED_FIELDS
};

/*
Global/Public functions
-----------------------
*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(FE_write_criterion);

int write_exregion_file(FILE *output_file,
	struct Cmiss_region *root_region, char *write_path,
	int write_elements, int write_nodes, enum FE_write_criterion write_criterion,
	struct FE_field_order_info *field_order_info);
/*******************************************************************************
LAST MODIFIED : 7 March 2003

DESCRIPTION :
Writes an exregion file to <output_file> with <root_region> at the top level of
the file. If <write_path> is supplied, only its contents are written, except
that all paths from <root_region> to it are complete in the file.
If the structure of the file permits it to be written in the old exnode/exelem
format this is done so; this is only possible if the output hierarchy is
two-deep and the second level contains only regions which use their parent's
name space for fields etc. In all other cases, <region> and </region> elements
are used to start and end regions so that they can be nested.
The <write_elements>, <write_nodes>, <write_criterion> and <field_order_info>
control what part of the regions are written:
If <field_order_info> is NULL, all element fields are written in the default,
alphabetical order.
If <field_order_info> is empty, only object identifiers are output.
If <field_order_info> contains fields, they are written in that order.
Additionally, the <write_criterion> controls output as follows:
FE_WRITE_COMPLETE_GROUP = write all object in the group (the default);
FE_WRITE_WITH_ALL_LISTED_FIELDS =
  write only objects with all listed fields defined;
FE_WRITE_WITH_ANY_LISTED_FIELDS =
  write only objects with any listed fields defined.
==============================================================================*/

int write_exregion_file_of_name(char *file_name,
	struct Cmiss_region *root_region, char *write_path,
	int write_elements, int write_nodes, enum FE_write_criterion write_criterion,
	struct FE_field_order_info *field_order_info);
/*******************************************************************************
LAST MODIFIED : 18 March 2003

DESCRIPTION :
Opens <file_name> for writing, calls write_exregion_file and then closes the file.
==============================================================================*/

#endif /* !defined (EXPORT_FINITE_ELEMENT_H) */
