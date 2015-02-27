/***************************************************************************//**
 * tessellation.hpp
 *
 * Objects for describing how elements / continuous field domains are
 * tessellated or sampled into graphics.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (TESSELLATION_HPP)
#define TESSELLATION_HPP

#include "zinc/tessellation.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"

struct cmzn_graphics_module;

class cmzn_tessellation_change_detail
{
	bool circleDivisionsChanged;
	bool elementDivisionsChanged;

public:

	cmzn_tessellation_change_detail() :
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
struct cmzn_tessellation;

DECLARE_LIST_TYPES(cmzn_tessellation);
DECLARE_MANAGER_TYPES(cmzn_tessellation);

PROTOTYPE_OBJECT_FUNCTIONS(cmzn_tessellation);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(cmzn_tessellation);

PROTOTYPE_LIST_FUNCTIONS(cmzn_tessellation);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(cmzn_tessellation,name,const char *);

PROTOTYPE_MANAGER_FUNCTIONS(cmzn_tessellation);
PROTOTYPE_MANAGER_IDENTIFIER_WITHOUT_MODIFY_FUNCTIONS(cmzn_tessellation,name,const char *);


/**
 * Create and return a handle to a new tessellation module.
 * Private; only to be called from graphics_module.
 *
 * @return  Handle to the newly created tessellation module if successful,
 * otherwise NULL.
 */
cmzn_tessellationmodule_id cmzn_tessellationmodule_create();

struct MANAGER(cmzn_tessellation) *cmzn_tessellationmodule_get_manager(
	cmzn_tessellationmodule_id tessellationmodule);

/***************************************************************************//**
 * Private; only to be called from graphics_module.
 */
int cmzn_tessellation_manager_set_owner_private(struct MANAGER(cmzn_tessellation) *manager,
	struct cmzn_tessellationmodule *tessellationmodule);

/**
 * Same as MANAGER_MESSAGE_GET_OBJECT_CHANGE(cmzn_tessellation) but also returns
 * change_detail for tessellation, if any.
 *
 * @param message  The tessellation manager change message.
 * @param tessellation  The tessellation to query about.
 * @param change_detail_address  Address to put const change detail in.
 * @return  manager change flags for the object.
 */
int cmzn_tessellation_manager_message_get_object_change_and_detail(
	struct MANAGER_MESSAGE(cmzn_tessellation) *message, cmzn_tessellation *tessellation,
	const cmzn_tessellation_change_detail **change_detail_address);

/**
 * Find or create a tessellation with divisions equal to the supplied element
 * and circle divisions and no refinement factors. Either divisions argument
 * can be omitted in which case values of the fallback tessellation apply (and
 * (if not supplied, default to element divisions = 1, circle divisions = 12).
 *
 * @param tessellationModule  The tessellation module to search or create in.
 * @param elementDivisionsCount  The size of the fixed_divisions array. 0 to omit.
 * @param elementDivisions  Array of divisions to match. 0 to omit.
 * @param circleDivisions  Number of circle divisions. 0 to omit.
 * @param fallbackTessellation  Optional tessellation to get defaults from where
 * element or circle divisions are omitted. If not supplied, element divisions
 * default to 1, and circleDivisions to 12.
 * @return  Handle to tessellation or 0 if error. Up to caller to destroy.
 */
cmzn_tessellation_id cmzn_tessellationmodule_find_or_create_fixed_tessellation(
	cmzn_tessellationmodule_id tessellationmodule,
	int elementDivisionsCount, int *elementDivisions, int circleDivisions,
	cmzn_tessellation_id fallbackTessellation);

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

int list_cmzn_tessellation_iterator(struct cmzn_tessellation *tessellation, void *dummy_void);

#endif
