/*******************************************************************************
FILE : import_finite_element.h

LAST MODIFIED : 23 May 2008

DESCRIPTION :
Functions for importing finite element data from a file into the graphical
interface to CMISS.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
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

int read_exregion_file(struct Cmiss_region *region,
	struct IO_stream *input_file, struct FE_import_time_index *time_index);
/*******************************************************************************
LAST MODIFIED : 23 May 2008

DESCRIPTION :
Reads finite element groups in exnode/exelem format from <input_file> into
<region>. Groups in the file are created as child group regions of <region>,
sharing its definitions of fields, nodes and elements.
It is good practice to read the file into a newly created region and check it
can be merged into the global region before doing so, otherwise failure to
merge incompatible data will leave the global region in a compromised state.
If the <time_index> is non NULL then the values in this read are assumed
to belong to the specified time.  This means that the nodal values will be read
into an array and the correct index put into the corresponding time array.
Where objects not within the file are referred to, such as nodes in a pure
exelem file or elements in embedded element:xi fields, local objects of the
correct type are made as placeholders and all checking is left to the merge.
Embedding elements are located by a region path starting at the root region
in the file; if no path is supplied they are placed in the root region.
If objects are repeated in the file, they are merged correctly.
==============================================================================*/

int read_exregion_file_of_name(struct Cmiss_region *region, char *file_name,
	struct IO_stream_package *io_stream_package,
	struct FE_import_time_index *time_index);
/*******************************************************************************
LAST MODIFIED : 23 May 2008

DESCRIPTION :
Version of read_exregion_file that opens and closes file <file_name>.
Up to the calling function to check and merge the returned cmiss_region.
==============================================================================*/

#endif /* !defined (IMPORT_FINITE_ELEMENT_H) */
