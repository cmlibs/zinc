/*******************************************************************************
FILE : integration.h

LAST MODIFIED : 26 December 2002

DESCRIPTION :
Structures and functions for integerating a computed field over a group of
elements.
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
#if !defined (INTEGRATION_H)
#define INTEGRATION_H

#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "general/object.h"

/*
Global types
------------
*/
struct Integration_scheme;
/*******************************************************************************
LAST MODIFIED : 26 December 2002

DESCRIPTION :
An object which when given an element will return the weights and abscissae to
be used when integrating over the element.

???DB.  cm has weights and abscissae for each basis.  A computed field may use
	FE fields with different bases for the same element - if weights and abscissae
	were stored with basis it wouldn't be clear which to use
???DB.  May want to have many ways of setting the weights and abscissae for
	elements eg shape, basis for a particular field.  Start with always same
==============================================================================*/

/*
Global functions
----------------
*/
struct Integration_scheme *CREATE(Integration_scheme)(char *name,
	struct FE_basis *basis);
/*******************************************************************************
LAST MODIFIED : 26 December 2002

DESCRIPTION :
Creates an integration scheme with the given <name> that will integrate the
<basis> exactly.  Unless further information is added, the created scheme will
return the weights and abscissae for <basis> for every element.  The <basis>
also defines the dimension of the integration scheme.
==============================================================================*/

int DESTROY(Integration_scheme)(struct Integration_scheme **scheme_address);
/*******************************************************************************
LAST MODIFIED : 26 December 2002

DESCRIPTION :
Frees memory/deaccess objects in scheme at <*scheme_address>.
==============================================================================*/

int integrate(struct Computed_field *field, struct FE_region *domain,
	struct Integration_scheme *scheme,FE_value time,FE_value *result);
/*******************************************************************************
LAST MODIFIED : 26 December 2002

DESCRIPTION :
Calculates the integral of the <field> over the <domain> at the specified <time>
using the given <scheme>.  The <result> array needs to be big enough to hold one
value for each component of the <field>.
==============================================================================*/

#endif /* !defined (INTEGRATION_H) */
