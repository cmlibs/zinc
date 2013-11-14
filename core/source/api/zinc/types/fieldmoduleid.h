/**
 * FILE : fieldmoduleid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_FIELDMODULEID_H__
#define CMZN_FIELDMODULEID_H__

/**
 * Container/manager of fields and domains within a region.
 */
struct cmzn_fieldmodule;
typedef struct cmzn_fieldmodule *cmzn_fieldmodule_id;

/**
 * Manages individual user notification of changes with a field module.
 */
struct cmzn_fieldmodulenotifier;
typedef struct cmzn_fieldmodulenotifier *cmzn_fieldmodulenotifier_id;

/**
 * Information about changes to fields and other objects in the field module.
 */
struct cmzn_fieldmoduleevent;
typedef struct cmzn_fieldmoduleevent *cmzn_fieldmoduleevent_id;

typedef void (*cmzn_fieldmodulenotifier_callback_function)(
	cmzn_fieldmoduleevent_id event, void *client_data);

/**
 * Bit flags summarising changes to fields in the field module.
 */
enum cmzn_fieldmoduleevent_change_flag
{
	CMZN_FIELDMODULEEVENT_CHANGE_FLAG_NONE = 0,
		/*!< object not changed */
	CMZN_FIELDMODULEEVENT_CHANGE_FLAG_ADD = 1,
		/*!< one or more objects added */
	CMZN_FIELDMODULEEVENT_CHANGE_FLAG_REMOVE = 2,
		/*!< one or more objects removed */
	CMZN_FIELDMODULEEVENT_CHANGE_FLAG_IDENTIFIER = 4,
		/*!< object identifier changed */
	CMZN_FIELDMODULEEVENT_CHANGE_FLAG_DEFINITION = 8,
		/*!< definition of object change which affects result */
	CMZN_FIELDMODULEEVENT_CHANGE_FLAG_DEPENDENCY = 16,
		/*!< change to a related object this has a dependency on which affects result */
	CMZN_FIELDMODULEEVENT_CHANGE_FLAG_METADATA = 32,
		/*!< change to object attributes not affecting result and not identifier */
	CMZN_FIELDMODULEEVENT_CHANGE_FLAG_FINAL = 64,
		/*!< final notification: owning field module i.e. region has been destroyed */
	CMZN_FIELDMODULEEVENT_CHANGE_FLAG_RESULT =
		CMZN_FIELDMODULEEVENT_CHANGE_FLAG_DEFINITION | CMZN_FIELDMODULEEVENT_CHANGE_FLAG_DEPENDENCY
		/*!< convenient value representing any change affecting result */
};

/**
 * Type for passing logical OR of #cmzn_fieldmoduleevent_change_flag
 */
typedef int cmzn_fieldmoduleevent_change_flags;

#endif /* CMZN_FIELD_MODULE_ID_H */
