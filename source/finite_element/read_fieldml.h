/*******************************************************************************
FILE : read_fieldml.h

LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
#if !defined (READ_FIELDML_H)
#define READ_FIELDML_H

#if defined (CMGUI_REGIONS)
int parse_fieldml_file(char *filename, struct Cmiss_region *root_region);
/*******************************************************************************
LAST MODIFIED : 4 March 2003

DESCRIPTION :
==============================================================================*/
#else /* defined (CMGUI_REGIONS) */
int parse_fieldml_file(char *filename, struct MANAGER(FE_basis) *basis_manager,
	struct MANAGER(FE_node) *node_manager, struct MANAGER(FE_element) *element_manager,
	struct MANAGER(FE_field) *fe_field_manager);
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
#endif /* defined (CMGUI_REGIONS) */
#endif /* !defined (READ_FIELDML_H) */

