/*******************************************************************************
FILE : cmiss_region.h

LAST MODIFIED : 4 November 2004

DESCRIPTION :
The public interface to the Cmiss_regions.
==============================================================================*/
#ifndef __CMISS_REGION_H__
#define __CMISS_REGION_H__

#include "general/object.h"

/*
Global types
------------
*/

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_node FE_node

struct Cmiss_node;
/*******************************************************************************
LAST MODIFIED : 14 August 2002

DESCRIPTION :
==============================================================================*/

typedef int (*Cmiss_node_iterator_function)(struct Cmiss_node *node, void *user_data);

typedef struct Cmiss_node * Cmiss_node_id;

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_element FE_element

struct Cmiss_element;
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
==============================================================================*/

typedef struct Cmiss_element * Cmiss_element_id;

struct Cmiss_region;
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
==============================================================================*/

typedef struct Cmiss_region * Cmiss_region_id;

/*
Global functions
----------------
*/

Cmiss_region_id CREATE(Cmiss_region_API)(void);
/*******************************************************************************
LAST MODIFIED : 19 August 2003

DESCRIPTION :
Creates a blank Cmiss_region.
SAB Temporarily mangled from the internal version
==============================================================================*/

int Cmiss_region_begin_change_API(Cmiss_region_id region);
/*******************************************************************************
LAST MODIFIED : 10 November 2004

DESCRIPTION :
Changes made to the <region> between Cmiss_region_begin_change and
Cmiss_region_end_change do not generate events in the rest of cmgui until
the change count returns to zero.  This allows many changes to be made 
efficiently, resulting in only one update of the dependent objects.
==============================================================================*/

int Cmiss_region_end_change_API(Cmiss_region_id region);
/*******************************************************************************
LAST MODIFIED : 10 November 2004

DESCRIPTION :
Changes made to the <region> between Cmiss_region_begin_change and
Cmiss_region_end_change do not generate events in the rest of cmgui until
the change count returns to zero.  This allows many changes to be made 
efficiently, resulting in only one update of the dependent objects.
==============================================================================*/

int Cmiss_region_read_file(struct Cmiss_region *region, char *file_name);
/*******************************************************************************
LAST MODIFIED : 19 August 2002

DESCRIPTION :
==============================================================================*/

struct Cmiss_region *Cmiss_region_get_sub_region(struct Cmiss_region *region,
	char *path);
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
Returns the sub_region specified by the string <path> in <region>.
==============================================================================*/

struct Cmiss_element *Cmiss_region_get_element(struct Cmiss_region *region,
	char *name);
/*******************************************************************************
LAST MODIFIED : 4 November 2004

DESCRIPTION :
Returns element with <name> in <region> if it exists.
==============================================================================*/

struct Cmiss_node *Cmiss_region_get_node(struct Cmiss_region *region,
	char *name);
/*******************************************************************************
LAST MODIFIED : 4 November 2004

DESCRIPTION :
Returns node with <name> in <region> if it exists.
==============================================================================*/

Cmiss_node_id Cmiss_region_merge_Cmiss_node(Cmiss_region_id region,
	Cmiss_node_id node);
/*******************************************************************************
LAST MODIFIED : 10 November 2004

DESCRIPTION :
Checks <node> is compatible with <region> and any existing Cmiss_node
using the same identifier, then merges it into <region>.
If no Cmiss_node of the same identifier exists in FE_region, <node> is added
to <region> and returned by this function, otherwise changes are merged into
the existing Cmiss_node and it is returned.
==============================================================================*/

int Cmiss_region_get_number_of_nodes_in_region(struct Cmiss_region *region);
/*******************************************************************************
LAST MODIFIED : 4 November 2004

DESCRIPTION :
Returns the number of nodes in <region> if it exists.
==============================================================================*/

int Cmiss_region_for_each_node_in_region(struct Cmiss_region *region,
	Cmiss_node_iterator_function iterator_function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 4 November 2004

DESCRIPTION :
Iterates over each node in <region>.
==============================================================================*/

Cmiss_element_id Cmiss_region_merge_Cmiss_element(Cmiss_region_id region,
	Cmiss_element_id element);
/*******************************************************************************
LAST MODIFIED : 10 November 2004

DESCRIPTION :
Checks <element> is compatible with <region> and any existing Cmiss_element
using the same identifier, then merges it into <region>.
If no Cmiss_element of the same identifier exists in Cmiss_region, <element> is added
to <region> and returned by this function, otherwise changes are merged into
the existing Cmiss_element and it is returned.
==============================================================================*/

int Cmiss_region_add_child_region(struct Cmiss_region *region,
	struct Cmiss_region *child_region, char *child_name, int child_position);
/*******************************************************************************
LAST MODIFIED : 22 October 2002

DESCRIPTION :
Adds <child_region> to <region> at <child_position> under the <child_name>.
<child_position> must be from -1 to the regions number_of_child_regions; if it
is -1 it is added at the end of the list.
Ensures:
- <child_name> passes is_standard_identifier_name;
- <child_name> is not already used in <region>;
- <child_region> is not already in <region>;
- <child_region> is not the same as or contains <region>;
==============================================================================*/
#endif /* __CMISS_REGION_H__ */
