/***************************************************************************//**
 * tessellation.hpp
 *
 * Objects for describing how elements / continuous field domains are
 * tessellated or sampled into graphics.
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
 * Portions created by the Initial Developer are Copyright (C) 2010
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

#if !defined (TESSELLATION_HPP)
#define TESSELLATION_HPP

#include "zinc/tessellation.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"

struct Cmiss_graphics_module;

class Cmiss_tessellation_change_detail
{
	bool circleDivisionsChanged;
	bool elementDivisionsChanged;

public:

	Cmiss_tessellation_change_detail() :
		circleDivisionsChanged(false),
		elementDivisionsChanged(false)
	{ }

	void clear()
	{
		circleDivisionsChanged = false;
		elementDivisionsChanged = false;
	}

	bool isCircleDivisionsChanged() const
	{
		return circleDivisionsChanged;
	}

	void setCircleDivisionsChanged()
	{
		circleDivisionsChanged = true;
	}

	bool isElementDivisionsChanged() const
	{
		return elementDivisionsChanged;
	}

	void setElementDivisionsChanged()
	{
		elementDivisionsChanged = true;
	}
};

/***************************************************************************//**
 * Object describing how elements / continuous field domains are  tessellated
 * or sampled into graphics.
 */
struct Cmiss_tessellation;

DECLARE_LIST_TYPES(Cmiss_tessellation);
DECLARE_MANAGER_TYPES(Cmiss_tessellation);

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_tessellation);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Cmiss_tessellation);

PROTOTYPE_LIST_FUNCTIONS(Cmiss_tessellation);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Cmiss_tessellation,name,const char *);

PROTOTYPE_MANAGER_FUNCTIONS(Cmiss_tessellation);
PROTOTYPE_MANAGER_IDENTIFIER_WITHOUT_MODIFY_FUNCTIONS(Cmiss_tessellation,name,const char *);


/**
 * Create and return a handle to a new tessellation module.
 * Private; only to be called from graphics_module.
 *
 * @return  Handle to the newly created tessellation module if successful,
 * otherwise NULL.
 */
Cmiss_tessellation_module_id Cmiss_tessellation_module_create();

struct MANAGER(Cmiss_tessellation) *Cmiss_tessellation_module_get_manager(
	Cmiss_tessellation_module_id tessellation_module);

/***************************************************************************//**
 * Private; only to be called from graphics_module.
 */
int Cmiss_tessellation_manager_set_owner_private(struct MANAGER(Cmiss_tessellation) *manager,
	struct Cmiss_tessellation_module *tessellation_module);

/**
 * Same as MANAGER_MESSAGE_GET_OBJECT_CHANGE(Cmiss_tessellation) but also returns
 * change_detail for tessellation, if any.
 *
 * @param message  The tessellation manager change message.
 * @param tessellation  The tessellation to query about.
 * @param change_detail_address  Address to put const change detail in.
 * @return  manager change flags for the object.
 */
int Cmiss_tessellation_manager_message_get_object_change_and_detail(
	struct MANAGER_MESSAGE(Cmiss_tessellation) *message, Cmiss_tessellation *tessellation,
	const Cmiss_tessellation_change_detail **change_detail_address);

/**
 * Internal function finding or creating a tessellation with coarse and fine
 * divisions equal to the fixed divisions supplied, and with matching circle
 * divisions. Match for either can be omitted.
 *
 * @param tessellationModule  The tessellation module to search or create in.
 * @param elementDivisionsCount  The size of the fixed_divisions array. 0 to omit.
 * @param elementDivisions  Array of divisions to match. 0 to omit.
 * @param circleDivisions  Number of circle divisions. 0 to omit.
 * @param defaultTessellation  Tessellation to get defaults from where element
 * or circle divisions are omitted.
 * @return  Handle to tessellation or 0 if error. Up to caller to destroy.
 */
Cmiss_tessellation_id Cmiss_tessellation_module_find_or_create_fixed_tessellation(
	Cmiss_tessellation_module_id tessellationModule,
	int elementDivisionsCount, int *elementDivisions, int circleDivisions,
	Cmiss_tessellation_id defaultTessellation);

/***************************************************************************//**
 * Function to process the string to be passed into an tessellation object.
 *
 * @param input  string to be passed into the function..
 * @param values_in  pointer of an array of int in which values will be written
 * 	 into.
 * @param size_in  pointer to an int which value will be assigned in this function.
 */
int string_to_divisions(const char *input, int **values_in, int *size_in);

void list_divisions(int size, int *divisions);

int list_Cmiss_tessellation_iterator(struct Cmiss_tessellation *tessellation, void *dummy_void);

#endif
