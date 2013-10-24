/**
 * FILE : selectionid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_SELECTIONID_H__
#define CMZN_SELECTIONID_H__

struct cmzn_selectionnotifier;
typedef struct cmzn_selectionnotifier * cmzn_selectionnotifier_id;

struct cmzn_selectionevent;
typedef struct cmzn_selectionevent * cmzn_selectionevent_id;

/**
 * Bit flags summarising changes to the selection.
 */
enum cmzn_selectionevent_change_flags
{
	CMZN_SELECTIONEVENT_CHANGE_NONE = 0,
	CMZN_SELECTIONEVENT_CHANGE_ADD = 1,       /*!< one or more objects added */
	CMZN_SELECTIONEVENT_CHANGE_REMOVE = 2,    /*!< one or more objects removed */
	CMZN_SELECTIONEVENT_CHANGE_FINAL = 4      /*!< final notification: owning object destroyed */
};

#endif /* CMZN_SELECTION_ID_H */
