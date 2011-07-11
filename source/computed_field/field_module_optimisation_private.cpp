/**
 * @file field_module_optimisation.cpp
 *
 * The factory method for creating a Cmiss_optimisation using a field module.
 * We want this to be separate to the general computed field code to ensure
 * that the core cmgui field library is not dependent upon optimisation.
 *
 * @see-also api/cmiss_optimisation.h
 *
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

extern "C"
{
	#include "minimise/cmiss_optimisation_private.h"
	#include "general/debug.h"
	#include "user_interface/message.h"
}

Cmiss_optimisation_id Cmiss_field_module_create_optimisation(Cmiss_field_module_id field_module)
{
	Cmiss_optimisation_id optimisation = 0;
	ENTER(Cmiss_field_module_create_optimisation);
	if (field_module)
	{
		optimisation = Cmiss_optimisation_create_private();
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_field_module_create_optimisation.  Invalid argument(s)");
	}
	LEAVE;
	return(optimisation);
}
