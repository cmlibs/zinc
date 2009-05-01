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

/**
 * Enumeration controlling which fields are be output to the EX file.
 */
enum FE_write_fields_mode
{
	FE_WRITE_ALL_FIELDS, /**< write all fields in the default, alphabetical order */
	FE_WRITE_NO_FIELDS, /**< write no fields, only object (node, element) identifiers */
	FE_WRITE_LISTED_FIELDS /**< write listed fields in the supplied order */
};

/**
 * Enumeration controlling which objects (nodes, elements) will be output to
 * the EX file.
 */
enum FE_write_criterion
{
	FE_WRITE_COMPLETE_GROUP, /**< write all objects in the group */
	FE_WRITE_WITH_ALL_LISTED_FIELDS, /**< write only objects with all listed fields defined */
	FE_WRITE_WITH_ANY_LISTED_FIELDS /**< write only objects with any listed fields defined */
};

/**
 * Enumeration controlling which objects (nodes, elements) will be output to
 * the EX file.
 */
enum FE_write_recursion
{
	FE_WRITE_RECURSIVE, /**< recursively write all sub-regions and sub-groups */
	FE_WRITE_RECURSE_SUBGROUPS, /**< write sub-groups but not sub-regions */
	FE_WRITE_NON_RECURSIVE /**< write only the selected region with no sub-groups */
};

/*
Global/Public functions
-----------------------
*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(FE_write_criterion);
PROTOTYPE_ENUMERATOR_FUNCTIONS(FE_write_recursion);

/***************************************************************************//**
 * Writes an EX file with supplied root_region at the top level of the file.
 *
 * @param output_file  The file to write region and field data to.
 * @param region  The region/group to output.
 * @param root_region  The region which will become the root region in the EX
 *   file. Need not be the true root of region hierarchy, but must contain
 *   <region>. 
 * @param write_elements  If set, write elements and element fields to file.
 * @param write_nodes  If set, write nodes and node fields to file.
 * @param write_data  If set, write data and data fields to file. May only use
 *   if write_elements and write_nodes are 0.
 * @param write_fields_mode  Controls which fields are written to file.
 *   If mode is FE_WRITE_LISTED_FIELDS then:
 *   - Number/list of field_names must be supplied;
 *   - Field names not used in a region are ignored;
 *   - Warnings are given for any field names not used in any output region.
 * @param number_of_field_names  The number of names in the field_names array.
 * @param field_names  Array of field names.
 * @param write_criterion  Controls which objects are written. Some modes
 *   limit output to nodes or objects with any or all listed fields defined.
 * @param write_recursion  Controls whether sub-regions and sub-groups are
 *   recursively written.
 */
int write_exregion_file(FILE *output_file,
	struct Cmiss_region *region, struct Cmiss_region *root_region,
	int write_elements, int write_nodes, int write_data,
	enum FE_write_fields_mode write_fields_mode,
	int number_of_field_names, char **field_names,
	enum FE_write_criterion write_criterion,
	enum FE_write_recursion write_recursion);

/***************************************************************************//**
 * Opens file with supplied name, calls write_exregion_file with it and closes
 * file. 
 * 
 * @param file_name  Name of file. 
 * @see write_exregion_file.
 */
int write_exregion_file_of_name(const char *file_name,
	struct Cmiss_region *region, struct Cmiss_region *root_region,
	int write_elements, int write_nodes, int write_data,
	enum FE_write_fields_mode write_fields_mode,
	int number_of_field_names, char **field_names,
	enum FE_write_criterion write_criterion,
	enum FE_write_recursion write_recursion);	

#endif /* !defined (EXPORT_FINITE_ELEMENT_H) */
