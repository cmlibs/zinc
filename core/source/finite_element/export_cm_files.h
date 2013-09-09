/*******************************************************************************
FILE : export_cm_files.h

LAST MODIFIED : 12 November 2002

DESCRIPTION :
Functions for exporting finite element data to a file.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (EXPORT_CM_FILES_H)
#define EXPORT_CM_FILES_H

#include <stdio.h>
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "general/enumerator.h"
#include "region/cmiss_region.h"

/*
Global/Public functions
-----------------------
*/

int write_cm_files(FILE *ipcoor_file, FILE *ipbase_file,
	FILE *ipnode_file, FILE *ipelem_file, FILE *ipmap_file,
	struct cmzn_region *root_region, char *write_path,
	struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 21 April 2006

DESCRIPTION :
Writes the set of <ipcoor_file>, <ipbase_file>, <ipnode_file> and <ipelem_file>
that defines elements of <field> in <write_path>.  The <ipmap_file> is 
optional, all the others are required.
==============================================================================*/

#endif /* !defined (EXPORT_CM_FILES_H) */
