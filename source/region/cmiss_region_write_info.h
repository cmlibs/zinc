/*******************************************************************************
FILE : cmiss_region_write_info.h

LAST MODIFIED : 16 May 2003

DESCRIPTION :
Data structure shared by several region export modules.
==============================================================================*/
#if !defined (CMISS_REGION_WRITE_INFO_H)
#define CMISS_REGION_WRITE_INFO_H

#include "region/cmiss_region.h"
#include "general/list.h"
#include "general/object.h"

/*
Module types
------------
*/

enum Cmiss_region_write_status
{
	CMISS_REGION_NOT_WRITTEN,
	CMISS_REGION_DECLARED,
	CMISS_REGION_WRITTEN
};

struct Cmiss_region_write_info;
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
Data structure shared by several region export modules.
==============================================================================*/

DECLARE_LIST_TYPES(Cmiss_region_write_info);

/*
Module functions
----------------
*/

struct Cmiss_region_write_info *CREATE(Cmiss_region_write_info)(void);
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
==============================================================================*/

int DESTROY(Cmiss_region_write_info)(
	struct Cmiss_region_write_info **write_info_address);
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_region_write_info);

PROTOTYPE_LIST_FUNCTIONS(Cmiss_region_write_info);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Cmiss_region_write_info, \
	region, struct Cmiss_region *);

int set_Cmiss_region_write_info(
	struct LIST(Cmiss_region_write_info) *write_info_list,
	struct Cmiss_region *region, enum Cmiss_region_write_status write_status,
	char *path);
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
==============================================================================*/

int get_Cmiss_region_write_info(
	struct LIST(Cmiss_region_write_info) *write_info_list,
	struct Cmiss_region *region,
	enum Cmiss_region_write_status *write_status_address,
	char **path_address);
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
The returned path is not to be deallocated.
==============================================================================*/

#endif /* !defined (CMISS_REGION_WRITE_INFO_H) */
