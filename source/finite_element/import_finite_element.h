/*******************************************************************************
FILE : import_finite_element.h

LAST MODIFIED : 3 September 2004

DESCRIPTION :
Functions for importing finite element data from a file into the graphical
interface to CMISS.
==============================================================================*/
#if !defined (IMPORT_FINITE_ELEMENT_H)
#define IMPORT_FINITE_ELEMENT_H

#include <stdio.h>
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "general/io_stream.h"
#include "region/cmiss_region.h"

/*
Global types
------------
*/

struct FE_import_time_index
/*******************************************************************************
LAST MODIFIED : 29 October 2002

DESCRIPTION :
Points to a specific time value which the nodes represent
==============================================================================*/
{
	float time;
}; /* FE_import_time_index */

/*
Global functions
----------------
*/

struct Cmiss_region *read_exregion_file(struct IO_stream *input_file,
	struct MANAGER(FE_basis) *basis_manager,
	struct LIST(FE_element_shape) *element_shape_list,
	struct FE_import_time_index *time_index);
/*******************************************************************************
LAST MODIFIED : 23 August 2004

DESCRIPTION :
Reads finite element groups in exnode/exelem format from <input_file>.
If successful, a Cmiss_region structure is returned which contains all the
groups as its named children. The fields, nodes and elements in each child
region are independent of one another; it is up to the calling function to
check and merge, clean up or otherwise deal with the returned Cmiss_region.
If the <node_time_index> is non NULL then the values in this read are assumed
to belong to the specified time.  This means that the nodal values will be read
into an array and the correct index put into the corresponding time array.
Where objects not within the file are referred to, such as nodes in a pure
exelem file or elements in embedded element:xi fields, local objects of the
correct type are made as placeholders and all checking is left to the merge.
Embedding elements are located by a region path starting at the root region
in the file; if no path is supplied they are placed in the root region.
If objects are repeated in the file, they are merged correctly.
==============================================================================*/

struct Cmiss_region *read_exregion_file_of_name(char *file_name,
	struct IO_stream_package *io_stream_package,
	struct MANAGER(FE_basis) *basis_manager,
	struct LIST(FE_element_shape) *element_shape_list,
	struct FE_import_time_index *time_index);
/*******************************************************************************
LAST MODIFIED : 3 September 2004

DESCRIPTION :
Version of read_exregion_file that opens and closes file <file_name>.
Up to the calling function to check and merge the returned cmiss_region.
==============================================================================*/

#endif /* !defined (IMPORT_FINITE_ELEMENT_H) */
