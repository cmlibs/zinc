/**
 * FILE : minimise.cpp
 *
 * Optimisation/minimisation routines.
 */
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

extern "C" {
#include <stdio.h>
#include <math.h>
#include "api/cmiss_field.h"
#include "api/cmiss_field_module.h"
#include "api/cmiss_optimisation.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_finite_element.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_private.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_region_private.h"
#include "general/any_object_private.h"
#include "general/any_object_definition.h"
#include "general/callback_private.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/object.h"
#include "general/message.h"
#include "minimise/minimise.h"
}
#include "general/enumerator_private_cpp.hpp"

PROTOTYPE_ENUMERATOR_FUNCTIONS(Cmiss_optimisation_method);

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Cmiss_optimisation_method)
{
	const char *enumerator_string = 0;
	switch (enumerator_value)
	{
	case CMISS_OPTIMISATION_METHOD_QUASI_NEWTON:
		enumerator_string = "QUASI_NEWTON";
		break;
	case CMISS_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON:
		enumerator_string = "LEAST_SQUARES_QUASI_NEWTON";
		break;
	default:
		break;
	}
	return enumerator_string;
}

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS( Cmiss_optimisation_method);

