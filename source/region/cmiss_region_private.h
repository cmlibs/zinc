/*******************************************************************************
FILE : cmiss_region_private.h

LAST MODIFIED : 1 October 2002

DESCRIPTION :
Private interface for attaching any object type to Cmiss_region objects.
==============================================================================*/
#if !defined (CMISS_REGION_PRIVATE_H)
#define CMISS_REGION_PRIVATE_H

#include "general/any_object.h"
#include "region/cmiss_region.h"

/*
Global functions
----------------
*/

int Cmiss_region_private_attach_any_object(struct Cmiss_region *region,
	struct Any_object *any_object);
/*******************************************************************************
LAST MODIFIED : 1 October 2002

DESCRIPTION :
Adds <any_object> to the list of objects attached to <region>.
This function is only externally visible to context objects.
==============================================================================*/

int Cmiss_region_private_detach_any_object(struct Cmiss_region *region,
	struct Any_object *any_object);
/*******************************************************************************
LAST MODIFIED : 1 October 2002

DESCRIPTION :
Removes <any_object> from the list of objects attached to <region>.
Note that only in the case that <any_object> is the exact Any_object stored in
<region> may it be cleaned up. In any other case the <any_object> passed in
must be cleaned up by the calling function.
This function is only externally visible to context objects.
==============================================================================*/

struct LIST(Any_object) *
Cmiss_region_private_get_any_object_list(struct Cmiss_region *region);
/*******************************************************************************
LAST MODIFIED : 1 October 2002

DESCRIPTION :
Returns the list of objects, abstractly stored as struct Any_object from
<region>. It is important that this list not be modified directly.
This function is only externally visible to context objects.
==============================================================================*/

#endif /* !defined (CMISS_REGION_PRIVATE_H) */
