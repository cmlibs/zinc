/***************************************************************************//**
 * context.h
 *
 * API to access the main root structure of cmgui.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_CONTEXT_H__
#define CMZN_CONTEXT_H__

#include "types/contextid.h"
#include "graphicsmodule.h"
#include "sceneviewer.h"
#include "types/regionid.h"
#include "timekeeper.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Create a new cmgui context with an id <id>.
 *
 * @param id  The identifier given to the new context.
 * @return  a handle to a new cmzn_context if successfully create, otherwise NULL.
 */
ZINC_API cmzn_context_id cmzn_context_create(const char *id);

/*******************************************************************************
 * Returns a new reference to the context with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param context  The context to obtain a new reference to.
 * @return  New region reference with incremented reference count.
 */
ZINC_API cmzn_context_id cmzn_context_access(cmzn_context_id context);

/***************************************************************************//**
 * Destroy a context.
 *
 * @param context_address  The address to the handle of the context
 *    to be destroyed.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_context_destroy(cmzn_context_id *context_address);

/***************************************************************************//**
 * Returns a handle to the default graphics module.
 *
 * @param context  Handle to a context object.
 * @return  The handle to the default graphics module of the context if
 *    successfully called, otherwise 0.
 */
ZINC_API cmzn_graphics_module_id cmzn_context_get_graphics_module(
	cmzn_context_id context);

/***************************************************************************//**
 * Returns the default region in the context.
 *
 * @param context  Handle to a context object.
 * @return  The handle to the default region of the context if successfully
 *    called, otherwise 0.
 */
ZINC_API cmzn_region_id cmzn_context_get_default_region(cmzn_context_id context);

/***************************************************************************//**
 * Create a new region and return a reference to it. Use this function to create
 * a region forming the root of an independent region tree. To create regions
 * for addition to an existing region tree, use cmzn_region_create_region.
 *
 * @see cmzn_region_create_region
 * @param context  Handle to a context object.
 * @return  Reference to newly created region if successful, otherwise NULL.
 */
ZINC_API cmzn_region_id cmzn_context_create_region(cmzn_context_id context);

/***************************************************************************//**
 * Returns the handle to time keeper and also increments the access count of
 * the returned time keeper by one.
 * User interface must be enabled before this function can be called successfully.
 *
 * @param context  Handle to a context object.
 * @return  The time keeper if successfully called otherwise NULL.
 */
ZINC_API cmzn_time_keeper_id cmzn_context_get_default_time_keeper(
	cmzn_context_id context);

/***************************************************************************//**
 * Process idle event in cmgui. Use this function to update idle event in cmgui
 * (such as redrawing the scene viewer) if a main loop is provided for cmgui.
 * This function does not trigger any time event.
 *
 * @param context  Handle to a context object.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_context_process_idle_event(cmzn_context_id context);

#ifdef __cplusplus
}
#endif

#endif /* CMZN_CONTEXT_H__ */
