/*******************************************************************************
FILE : write_fieldml.h

LAST MODIFIED : 28 January 2003

DESCRIPTION :
Functions for exporting finite element data to a fieldml file.
==============================================================================*/
#if !defined (WRITE_FIELDML_H)
#define WRITE_FIELDML_H

#include <stdio.h>
#include "finite_element/finite_element.h"
/*#include "finite_element/finite_element_region.h"*/
#include "general/enumerator.h"

/* Temporary declarations until this is merged into the real Cmiss_region code
   till end */
struct FE_region
/*******************************************************************************
LAST MODIFIED : 29 January 2003

DESCRIPTION :
A quick FE_region for the time being.

AUTHOR : Shane Blackett
==============================================================================*/
{
	struct MANAGER(FE_basis) *basis_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	struct GROUP(FE_node) *node_group;
	struct GROUP(FE_element) *element_group;
};

struct Cmiss_region
{
	struct FE_region *fe_region;
};
/* end Temporary declarations until this is merged into the real Cmiss_region code */

/*
Global/Public functions
-----------------------
*/

int write_fieldml_file(FILE *output_file,
	struct Cmiss_region *root_region, char *write_path,
	int write_elements, int write_nodes, struct FE_field_order_info *field_order_info);
/*******************************************************************************
LAST MODIFIED : 28 January 2003

DESCRIPTION :
Writes an exregion file to <output_file> with <root_region> at the top level of
the file. If <write_region> is supplied, only its contents are written, except
that all paths from <root_region> to it are complete in the file.
If the structure of the file permits it to be written in the old exnode/exelem
format this is done so; this is only possible if the output hierarchy is
two-deep and the second level contains only regions which use their parent's
name space for fields etc. In all other cases, <region> and </region> elements
are used to start and end regions so that they can be nested.
The <write_elements>, <write_nodes> and <field_order_info>
control what part of the regions are written:
If <field_order_info> is NULL, all element fields are written in the default,
alphabetical order.
If <field_order_info> is empty, only object identifiers are output.
If <field_order_info> contains fields, they are written in that order.
==============================================================================*/

#endif /* !defined (WRITE_FIELDML_H) */
