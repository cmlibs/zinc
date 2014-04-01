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
 * Information about changes to fields and other objects in the field module,
 * sent with each callback from the fieldmodule notifier.
 */
struct cmzn_fieldmoduleevent;
typedef struct cmzn_fieldmoduleevent *cmzn_fieldmoduleevent_id;

typedef void (*cmzn_fieldmodulenotifier_callback_function)(
	cmzn_fieldmoduleevent_id event, void *client_data);

#endif /* CMZN_FIELD_MODULE_ID_H */
