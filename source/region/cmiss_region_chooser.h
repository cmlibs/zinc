/*******************************************************************************
FILE : cmiss_region_chooser.h

LAST MODIFIED : 13 January 2003

DESCRIPTION :
==============================================================================*/
#if !defined (CMISS_REGION_CHOOSER_H)
#define CMISS_REGION_CHOOSER_H

/*
Global variables
----------------
*/

#include "general/callback_motif.h"

struct Cmiss_region_chooser;
/*******************************************************************************
LAST MODIFIED : 9 January 2003

DESCRIPTION :
==============================================================================*/

/*
Global functions
----------------
*/

struct Cmiss_region_chooser *CREATE(Cmiss_region_chooser)(Widget parent,
	struct Cmiss_region *root_region, char *initial_path);
/*******************************************************************************
LAST MODIFIED : 8 January 2003

DESCRIPTION :
Creates a dialog from which a region may be chosen.
<parent> must be an XmForm.
==============================================================================*/

int DESTROY(Cmiss_region_chooser)(
	struct Cmiss_region_chooser **chooser_address);
/*******************************************************************************
LAST MODIFIED : 9 January 2003

DESCRIPTION :
==============================================================================*/

int Cmiss_region_chooser_set_callback(struct Cmiss_region_chooser *chooser,
	Callback_procedure *procedure, void *data);
/*******************************************************************************
LAST MODIFIED : 8 January 2003

DESCRIPTION :
Sets the callback <procedure> and user <data> in the <chooser>.
==============================================================================*/

int Cmiss_region_chooser_get_path(struct Cmiss_region_chooser *chooser,
	char **path_address);
/*******************************************************************************
LAST MODIFIED : 13 January 2003

DESCRIPTION :
Gets <path> of chosen region in the <chooser>.
Up to the calling function to DEALLOCATE the returned path.
==============================================================================*/

int Cmiss_region_chooser_get_region(struct Cmiss_region_chooser *chooser,
	struct Cmiss_region **region_address);
/*******************************************************************************
LAST MODIFIED : 13 January 2003

DESCRIPTION :
Returns to <region_address> the region chosen in the <chooser>.
==============================================================================*/

int Cmiss_region_chooser_set_path(struct Cmiss_region_chooser *chooser,
	char *path);
/*******************************************************************************
LAST MODIFIED : 9 January 2003

DESCRIPTION :
Sets <path> of chosen region in the <chooser>.
==============================================================================*/

#endif /* !defined (CMISS_REGION_CHOOSER_H) */
