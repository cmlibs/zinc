/*******************************************************************************
FILE : read_fieldml.h

LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
#if !defined (READ_FIELDML_H)
#define READ_FIELDML_H

int parse_fieldml_file(char *filename, struct MANAGER(FE_basis) *basis_manager,
	struct MANAGER(FE_node) *node_manager, struct MANAGER(FE_element) *element_manager,
	struct MANAGER(FE_field) *fe_field_manager);
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
#endif /* !defined (READ_FIELDML_H) */

