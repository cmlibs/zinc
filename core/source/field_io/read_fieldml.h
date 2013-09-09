/***************************************************************************//**
 * FILE : read_fieldml.h
 * 
 * Functions for importing regions and fields from FieldML 0.4+ documents.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (READ_FIELDML_H)
#define READ_FIELDML_H

struct cmzn_region;

/***************************************************************************//**
 * Determines whether the named file is FieldML 0.4 format by the quick and
 * dirty method of finding a <Fieldml> tag near the beginning.
 * @return 1 if file is FieldML 0.4 format, 0 if not.
 */
int is_FieldML_file(const char *filename);

/***************************************************************************//**
 * Reads subregions and fields from FieldML 0.4 format file into the region.
 */
int parse_fieldml_file(struct cmzn_region *region, const char *filename);

#endif /* !defined (READ_FIELDML_H) */
