/*******************************************************************************
FILE : cmiss_region.h

LAST MODIFIED : 1 April 2004

DESCRIPTION :
The public interface to the Cmiss_regions.
==============================================================================*/
#ifndef __CMISS_REGION_H__
#define __CMISS_REGION_H__

#include "api/cmiss_finite_element.h"
#include "general/object.h"

/*
Global types
------------
*/
struct Cmiss_region;
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
==============================================================================*/

/*
Global functions
----------------
*/

struct Cmiss_region *CREATE(Cmiss_region_API)(void);
/*******************************************************************************
LAST MODIFIED : 19 August 2003

DESCRIPTION :
Creates a blank Cmiss_region.
SAB Temporarily mangled from the internal version
==============================================================================*/

int Cmiss_region_read_file(struct Cmiss_region *region, char *file_name);
/*******************************************************************************
LAST MODIFIED : 19 August 2002

DESCRIPTION :
Returns the sub_region specified by the string <path> in <region>.
==============================================================================*/

struct Cmiss_region *Cmiss_region_get_sub_region(struct Cmiss_region *region,
	char *path);
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
Returns the sub_region specified by the string <path> in <region>.
==============================================================================*/

struct Cmiss_FE_field *Cmiss_region_get_field(struct Cmiss_region *region,
	char *path, char *name);
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
Returns field with <name> in sub_region <path> of <region> if it exists.
==============================================================================*/

struct Cmiss_element *Cmiss_region_get_element(struct Cmiss_region *region,
	char *path, char *name);
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
Returns element with <name> in sub_region <path> of <region> if it exists.
==============================================================================*/

struct Cmiss_node *Cmiss_region_get_node(struct Cmiss_region *region,
	char *path, char *name);
/*******************************************************************************
LAST MODIFIED : 19 August 2002

DESCRIPTION :
Returns node with <name> in sub_region <path> of <region> if it exists.
==============================================================================*/

int Cmiss_region_get_number_of_nodes_in_region(struct Cmiss_region *region,
	char *path);
/*******************************************************************************
LAST MODIFIED : 1 April 2004

DESCRIPTION :
Returns the number of nodes in sub_region <path> of <region> if it exists.
==============================================================================*/

int Cmiss_region_for_each_node_in_region(struct Cmiss_region *region,
	char *path, Cmiss_node_iterator_function iterator_function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 31 March 2004

DESCRIPTION :
Iterates over each node in the subregion specified by <region> and <path>.
==============================================================================*/
#endif /* __CMISS_REGION_H__ */
