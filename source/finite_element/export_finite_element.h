/*******************************************************************************
FILE : export_finite_element.h

LAST MODIFIED : 12 November 2002

DESCRIPTION :
Functions for exporting finite element data to a file.
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
#if !defined (EXPORT_FINITE_ELEMENT_H)
#define EXPORT_FINITE_ELEMENT_H

#include <stdio.h>
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "general/enumerator.h"
#include "region/cmiss_region.h"

/*
Global/Public types
-------------------
*/

enum FE_write_criterion
/*******************************************************************************
LAST MODIFIED : 7 November 2002

DESCRIPTION :
Controls output of nodes/elements to exnode/exelem files as follows:
FE_WRITE_COMPLETE_GROUP = write all object in the group (the default);
FE_WRITE_WITH_ALL_LISTED_FIELDS =
  write only objects with all listed fields defined;
FE_WRITE_WITH_ANY_LISTED_FIELDS =
  write only objects with any listed fields defined.
==============================================================================*/
{
	FE_WRITE_COMPLETE_GROUP,
	FE_WRITE_NO_FIELDS,
	FE_WRITE_WITH_ALL_LISTED_FIELDS,
	FE_WRITE_WITH_ANY_LISTED_FIELDS
};

/*
Global/Public functions
-----------------------
*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(FE_write_criterion);

int write_exregion_file(FILE *output_file,
	struct Cmiss_region *root_region, char *write_path,
	int write_elements, int write_nodes, enum FE_write_criterion write_criterion,
	struct FE_field_order_info *field_order_info);
/*******************************************************************************
LAST MODIFIED : 7 March 2003

DESCRIPTION :
Writes an exregion file to <output_file> with <root_region> at the top level of
the file. If <write_path> is supplied, only its contents are written, except
that all paths from <root_region> to it are complete in the file.
If the structure of the file permits it to be written in the old exnode/exelem
format this is done so; this is only possible if the output hierarchy is
two-deep and the second level contains only regions which use their parent's
name space for fields etc. In all other cases, <region> and </region> elements
are used to start and end regions so that they can be nested.
The <write_elements>, <write_nodes>, <write_criterion> and <field_order_info>
control what part of the regions are written:
If <field_order_info> is NULL, all element fields are written in the default,
alphabetical order.
If <field_order_info> is empty, only object identifiers are output.
If <field_order_info> contains fields, they are written in that order.
Additionally, the <write_criterion> controls output as follows:
FE_WRITE_COMPLETE_GROUP = write all object in the group (the default);
FE_WRITE_WITH_ALL_LISTED_FIELDS =
  write only objects with all listed fields defined;
FE_WRITE_WITH_ANY_LISTED_FIELDS =
  write only objects with any listed fields defined.
==============================================================================*/

int write_exregion_file_of_name(char *file_name,
	struct Cmiss_region *root_region, char *write_path,
	int write_elements, int write_nodes, enum FE_write_criterion write_criterion,
	struct FE_field_order_info *field_order_info);
/*******************************************************************************
LAST MODIFIED : 18 March 2003

DESCRIPTION :
Opens <file_name> for writing, calls write_exregion_file and then closes the file.
==============================================================================*/

#endif /* !defined (EXPORT_FINITE_ELEMENT_H) */
