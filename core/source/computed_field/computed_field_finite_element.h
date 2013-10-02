/*******************************************************************************
FILE : computed_field_finite_element.h

LAST MODIFIED : 25 May 2006

DESCRIPTION :
Implements computed fields which interface to finite element fields.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMPUTED_FIELD_FINITE_ELEMENT_H)
#define COMPUTED_FIELD_FINITE_ELEMENT_H

#include "zinc/fieldfiniteelement.h"

/*
Global functions
----------------
*/
int Computed_field_is_type_finite_element(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/

int Computed_field_is_type_finite_element_iterator(
	struct Computed_field *field, void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 16 March 2007

DESCRIPTION :
Iterator/conditional function returning true if <field> is read only and a
wrapper for an FE_field.
==============================================================================*/

/**
 * If the field is of type COMPUTED_FIELD_FINITE_ELEMENT, the FE_field being
 * wrapped by it is returned, or 0 with no errors if not of correct type.
 * The returned fe_field is NOT accessed.
 */
int Computed_field_get_type_finite_element(struct Computed_field *field,
	struct FE_field **fe_field);

/*******************************************************************************
 * Iterator/conditional function returning true if <field> is the finite_element
 * computed field wrapper for the FE_field.
 * @param fe_field_void  Void pointer to a struct FE_field.
 */
int Computed_field_wraps_fe_field(struct Computed_field *field,
	void *fe_field_void);

int Computed_field_contains_changed_FE_field(
	struct Computed_field *field, void *fe_field_change_log_void);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Returns true if <field> directly contains an FE_field and it is listed as
changed, added or removed in <fe_field_change_log>.
<fe_field_change_log_void> must point at a struct CHANGE_LOG<FE_field>.
==============================================================================*/

struct LIST(FE_field)
	*Computed_field_get_defining_FE_field_list(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Returns the list of FE_fields that <field> depends on.
==============================================================================*/

struct LIST(FE_field)
	*Computed_field_array_get_defining_FE_field_list(
		int number_of_fields, struct Computed_field **field_array);
/*******************************************************************************
LAST MODIFIED : 5 April 2006

DESCRIPTION :
Returns the compiled list of FE_fields that are required by any of
the <number_of_fields> fields in <field_array>.
==============================================================================*/

int Computed_field_is_type_cmiss_number(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/

/*****************************************************************************//**
 * Creates a cmiss_number type fields whose value is the number of the element
 * or node location.
 *
 * @param field_module  Region field module which will own new field.
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_cmiss_number(
	struct cmzn_fieldmodule *field_module);

int Computed_field_has_coordinate_fe_field(struct Computed_field *field,
	void *dummy);
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Iterator/conditional function returning true if <field> is a wrapper for a
coordinate type fe_field.
==============================================================================*/

int Computed_field_has_element_xi_fe_field(struct Computed_field *field,
	void *dummy);
/*******************************************************************************
LAST MODIFIED : 2 June 2006

DESCRIPTION :
Iterator/conditional function returning true if <field> is a wrapper for an
element_xi type fe_field.
==============================================================================*/

int Computed_field_is_scalar_integer(struct Computed_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Returns true if <field> is a 1 integer component FINITE_ELEMENT wrapper.
==============================================================================*/

int Computed_field_is_scalar_integer_grid_in_element(
	struct Computed_field *field,void *element_void);
/*******************************************************************************
LAST MODIFIED : 26 May 2000

DESCRIPTION :
Returns true if <field> is a 1 integer component FINITE_ELEMENT wrapper which
is defined in <element> AND is grid-based.
Used for choosing field suitable for identifying grid points.
==============================================================================*/

int Computed_field_is_type_xi_coordinates(struct Computed_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
==============================================================================*/

/*****************************************************************************//**
 * Creates a field whose return values are the xi coordinate location.
 * Currently fixed at 3 components, padding with zeros for lower dimension
 * elements.
 *
 * @param field_module  Region field module which will own new field.
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_xi_coordinates(
	struct cmzn_fieldmodule *field_module);

int Computed_field_is_type_node_value(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
==============================================================================*/

struct FE_time_sequence *Computed_field_get_FE_node_field_FE_time_sequence(
	 struct Computed_field *computed_field, struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 9 Oct 2007

DESCRIPTION :
==============================================================================*/

struct FE_region_changes;

/***************************************************************************//**
 * Callback for changes to FE_region attached to a cmiss_region.
 * Updates definitions of Computed_field wrappers for changed FE_fields in the
 * region.
 * Also ensures region has cmiss_number and xi fields, at the appropriate time.
 * @private  Should only be called from cmiss_region.cpp!
 */
void cmzn_region_FE_region_change(struct FE_region *fe_region,
	struct FE_region_changes *changes, void *cmiss_region_void);

#endif /* !defined (COMPUTED_FIELD_FINITE_ELEMENT_H) */
