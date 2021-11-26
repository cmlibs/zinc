/**
* FILE : export_finite_element.h
*
* Functions for exporting finite element data to EX format.
*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (EXPORT_FINITE_ELEMENT_H)
#define EXPORT_FINITE_ELEMENT_H

#include <stdio.h>
#include "opencmiss/zinc/fieldgroup.h"
#include "finite_element/finite_element.h"
#include "general/enumerator.h"
#include "region/cmiss_region.hpp"
#include "opencmiss/zinc/types/regionid.h"

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

/**
 * Opens file with supplied name, calls write_exregion_to_stream with it and closes
 * file.
 * 
 * @param file_name  Name of file.
 * @param rootRegion  The root region output paths are relative to.
 * @param region  The region to output nodes/elements/data from.
 * @param groupName  Optional name of group to limit output to.
 * @param writeDomainTypes  Bitwise OR of cmzn_field_domain_type flags controlling
 * which meshes or nodesets to write.
 * @param timeSet  True if following time is set to be output, otherwise false.
 * @param time  Time to output parameters at if timeSet is true. Nearest time in
 * time sequences is output if not an exact match.
 * @see write_exregion_to_stream.
 */
int write_exregion_file_of_name(
	const char *fileName,
	struct cmzn_region *rootRegion,
	struct cmzn_region *region, const char *groupName,
	bool timeSet, FE_value time,
	cmzn_field_domain_types writeDomainTypes,
	FE_write_fields_mode writeFieldsMode,
	int fieldNamesCount, const char * const *fieldNames,
	FE_write_criterion writeCriterion,
	cmzn_streaminformation_region_recursion_mode recursionMode);

int write_exregion_file_to_memory_block(
	void **memoryBlock, unsigned int *memoryBlockLength,
	struct cmzn_region *rootRegion,
	struct cmzn_region *region, const char *groupName,
	bool timeSet, FE_value time,
	cmzn_field_domain_types writeDomainTypes,
	FE_write_fields_mode writeFieldsMode,
	int fieldNamesCount, const char * const *fieldNames,
	FE_write_criterion writeCriterion,
	cmzn_streaminformation_region_recursion_mode recursionMode);

#endif /* !defined (EXPORT_FINITE_ELEMENT_H) */
