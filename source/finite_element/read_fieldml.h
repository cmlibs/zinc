/*******************************************************************************
FILE : read_fieldml.h

LAST MODIFIED : 15 May 2003

DESCRIPTION :
==============================================================================*/
#if !defined (READ_FIELDML_H)
#define READ_FIELDML_H

struct Cmiss_region *parse_fieldml_file(char *filename,
	struct MANAGER(FE_basis) *basis_manager,
	struct LIST(FE_element_shape) *element_shape_list);
/*******************************************************************************
LAST MODIFIED : 15 May 2003

DESCRIPTION :
Reads fieldml file <filename> and returns a Cmiss_region containing its
contents. A NULL object return indicates an error.
Up to the calling function to check, merge and destroy the returned
Cmiss_region.
==============================================================================*/
#endif /* !defined (READ_FIELDML_H) */

