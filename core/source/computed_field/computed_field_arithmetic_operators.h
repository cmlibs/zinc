/*******************************************************************************
FILE : computed_field_arithmetic_operators.h

LAST MODIFIED : 13 July 2000

DESCRIPTION :
Implements a number of basic component wise operators on computed fields.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMPUTED_FIELD_ARITHMETIC_OPERATORS_H)
#define COMPUTED_FIELD_ARITHMETIC_OPERATORS_H

#include "general/value.h"
#include "opencmiss/zinc/field.h"
#include "opencmiss/zinc/fieldarithmeticoperators.h"

/* API functions are prefixed with cmzn */
#define Computed_field_create_add cmzn_fieldmodule_create_field_add
#define Computed_field_create_subtract cmzn_fieldmodule_create_field_subtract
#define Computed_field_create_multiply cmzn_fieldmodule_create_field_multiply
#define Computed_field_create_divide cmzn_fieldmodule_create_field_divide
#define Computed_field_create_exp cmzn_fieldmodule_create_field_exp
#define Computed_field_create_log cmzn_fieldmodule_create_field_log
#define Computed_field_create_power cmzn_fieldmodule_create_field_power
#define Computed_field_create_sqrt cmzn_fieldmodule_create_field_sqrt
#define Computed_field_create_abs cmzn_fieldmodule_create_field_abs

struct Computed_field *Computed_field_create_power(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_POWER with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of
components equal to the source_fields.
Automatic scalar broadcast will apply, see cmiss_field.h.
==============================================================================*/

struct Computed_field *Computed_field_create_multiply(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);
/*******************************************************************************
LAST MODIFIED : 14 May 2008

DESCRIPTION :
Creates a field of type COMPUTED_FIELD_MULTIPLY with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of
components equal to the source_fields.
Automatic scalar broadcast will apply, see cmiss_field.h.
==============================================================================*/

struct Computed_field *Computed_field_create_divide(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DIVIDE with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of
components equal to the source_fields.
Automatic scalar broadcast will apply, see cmiss_field.h.
==============================================================================*/

struct Computed_field *Computed_field_create_weighted_add(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field_one, double scale_factor1,
	struct Computed_field *source_field_two, double scale_factor2);
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ADD with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of
components equal to the source_fields.
Automatic scalar broadcast will apply, see cmiss_field.h.
==============================================================================*/

struct Computed_field *Computed_field_create_add(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ADD with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of
components equal to the source_fields.
Automatic scalar broadcast will apply, see cmiss_field.h.
==============================================================================*/

struct Computed_field *Computed_field_create_subtract(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ADD with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of
components equal to the source_fields.
Automatic scalar broadcast will apply, see cmiss_field.h.
==============================================================================*/

struct Computed_field *Computed_field_create_log(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field);
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_LOG with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
Automatic scalar broadcast will apply, see cmiss_field.h.
==============================================================================*/

struct Computed_field *Computed_field_create_sqrt(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field);
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SQRT with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/

struct Computed_field *Computed_field_create_exp(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field);
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_EXP with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/

struct Computed_field *Computed_field_create_abs(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field);
/*******************************************************************************
LAST MODIFIED : 1 December 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ABS with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_ARITHMETIC_OPERATORS_H) */
