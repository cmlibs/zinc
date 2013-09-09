/***************************************************************************//**
 * FILE : computed_field_group_base.hpp
 * 
 * Common base class for group fields & common group change detail types.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (COMPUTED_FIELD_GROUP_BASE_HPP)
#define COMPUTED_FIELD_GROUP_BASE_HPP

#include "computed_field/computed_field_private.hpp"

/***************************************************************************//**
 * Summary of changes to a group.
 */
enum cmzn_field_group_change_type
{
	CMZN_FIELD_GROUP_NO_CHANGE,
	CMZN_FIELD_GROUP_CLEAR,     /*!< group is empty, but wasn't before */
	CMZN_FIELD_GROUP_ADD,       /*!< objects have been added only */
	CMZN_FIELD_GROUP_REMOVE,    /*!< objects have been removed only */
	CMZN_FIELD_GROUP_REPLACE    /*!< contents replaced: clear+add, add+remove */
};

/***************************************************************************//**
 * Base class for all group field change details.
 */
struct cmzn_field_group_base_change_detail : public cmzn_field_change_detail
{
	virtual cmzn_field_group_change_type getChange() const = 0;

	virtual cmzn_field_group_change_type getLocalChange() const = 0;
};

/***************************************************************************//**
 * Base class for all group fields.
 */
class Computed_field_group_base : public Computed_field_core
{
public:

	Computed_field_group_base() :
		Computed_field_core()
	{
	}

	virtual char *get_command_string()
	{
		return 0;
	}

	virtual int isEmpty() const = 0;

	virtual int clear() = 0;
};

#endif /* COMPUTED_FIELD_GROUP_BASE_HPP */

