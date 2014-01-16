/**
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
#include "types/fontid.h"
#include "types/glyphid.h"
#include "types/materialid.h"
#include "types/regionid.h"
#include "types/scenefilterid.h"
#include "types/sceneviewerid.h"
#include "types/spectrumid.h"
#include "types/tessellationid.h"
#include "types/timekeeperid.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a new cmgui context with an id <id>.
 *
 * @param id  The identifier given to the new context.
 * @return  a handle to a new cmzn_context if successfully create, otherwise NULL.
 */
ZINC_API cmzn_context_id cmzn_context_create(const char *id);

/**
 * Returns a new reference to the context with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param context  The context to obtain a new reference to.
 * @return  New region reference with incremented reference count.
 */
ZINC_API cmzn_context_id cmzn_context_access(cmzn_context_id context);

/**
 * Destroy a context.
 *
 * @param context_address  The address to the handle of the context
 *    to be destroyed.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_context_destroy(cmzn_context_id *context_address);

/**
 * Returns the default region in the context.
 *
 * @param context  Handle to a context object.
 * @return  The handle to the default region of the context if successfully
 *    called, otherwise 0.
 */
ZINC_API cmzn_region_id cmzn_context_get_default_region(cmzn_context_id context);

/**
 * Create a new region and return a reference to it. Use this function to create
 * a region forming the root of an independent region tree. To create regions
 * for addition to an existing region tree, use cmzn_region_create_region.
 *
 * @see cmzn_region_create_region
 * @param context  Handle to a context object.
 * @return  Reference to newly created region if successful, otherwise NULL.
 */
ZINC_API cmzn_region_id cmzn_context_create_region(cmzn_context_id context);

/**
* Get the font module which stores font object.
*
* @param context  The context to request the module from.
* @return  Handle to the font module, or 0 on error. Up to caller to destroy.
*/
ZINC_API cmzn_fontmodule_id cmzn_context_get_fontmodule(
	cmzn_context_id context);

/**
 * Get the glyph module which stores static graphics for visualising points,
 * vectors, axes etc. Note on startup no glyphs are defined and glyph module
 * functions need to be called to set up standard glyphs.
 *
 * @param context  The context to request the module from.
 * @return  Handle to the glyph module, or 0 on error. Up to caller to destroy.
 */
ZINC_API cmzn_glyphmodule_id cmzn_context_get_glyphmodule(
	cmzn_context_id context);

/**
 * Return the material module in context.
 *
 * @param context  The context to request module from.
 * @return  the material pacakage in context if successfully called,
 *    otherwise NULL.
 */
ZINC_API cmzn_materialmodule_id cmzn_context_get_materialmodule(
	cmzn_context_id context);

/**
 * Get the scene filter module which stores scenefilter objects.
 *
 * @param context  The context to request the module from.
 * @return  Handle to the context filter module, or 0 on error. Up to caller to destroy.
 */
ZINC_API cmzn_scenefiltermodule_id cmzn_context_get_scenefiltermodule(
	cmzn_context_id context);

/**
 * Returns a handle to a scene viewer module
 * User interface must be enabled before this function can be called successfully.
 *
 * @param context  Handle to a context object.
 * @return The scene viewer module if successfully called, otherwise NULL.
 */
ZINC_API cmzn_sceneviewermodule_id cmzn_context_get_sceneviewermodule(
	cmzn_context_id context);

/**
 * Get the spectrum module which stores spectrum objects.
 *
 * @param context  The context to request the module from.
 * @return  Handle to the spectrum module, or 0 on error. Up to caller to destroy.
 */
ZINC_API cmzn_spectrummodule_id cmzn_context_get_spectrummodule(
	cmzn_context_id context);

/**
 * Get the tessellation module which stores tessellation objects.
 *
 * @param context  The context to request the module from.
 * @return  Handle to the tesselation module, or 0 on error. Up to caller to destroy.
 */
ZINC_API cmzn_tessellationmodule_id cmzn_context_get_tessellationmodule(
	cmzn_context_id context);

/**
 * Get the timekeeper module which stores objects for synchronising time
 * across zinc objects.
 *
 * @param context  The context to request the module from.
 * @return  Handle to the timekeeper module, or 0 on error. Up to caller to destroy.
 */
ZINC_API cmzn_timekeepermodule_id cmzn_context_get_timekeepermodule(
	cmzn_context_id context);

#ifdef __cplusplus
}
#endif

#endif /* CMZN_CONTEXT_H__ */
