/***************************************************************************//**
 * context.cpp
 *
 * The main root structure of cmgui.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (CONTEXT_H)
#define CONTEXT_H

#include "zinc/context.h"
#include "general/manager.h"

#define Context cmzn_context

struct Context
{
	int access_count;
	const char *id;
	struct cmzn_region *root_region;
	/* Always want the entry for graphics_buffer_package even if it will
		not be available on this implementation */
	struct cmzn_graphics_module *graphics_module;
	struct Any_object_selection *any_object_selection;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	//-- struct Event_dispatcher *event_dispatcher;
	struct IO_stream_package *io_stream_package;
	cmzn_timekeepermodule *timekeepermodule;
	struct MANAGER(Curve) *curve_manager;
};

/***************************************************************************//**
 * Returns a handle to the default graphics module.
 *
 * @param context  Handle to a context object.
 * @return  The handle to the default graphics module of the context if
 *    successfully called, otherwise 0.
 */
cmzn_graphics_module * cmzn_context_get_graphics_module(
	cmzn_context_id context);

/***************************************************************************//**
 * Return the any object selection in context.
 *
 * @param context  Pointer to a cmiss_context object.
 * @return  the any_object_selection if successfully, otherwise NULL.
 */
struct Any_object_selection *cmzn_context_get_any_object_selection(
	struct Context *context);

/***************************************************************************//**
 * Return the element point ranges selection in context.
 *
 * @param context  Pointer to a cmiss_context object.
 * @return  the Element_point_ranges_selection if successfully, otherwise NULL.
 */
struct Element_point_ranges_selection *cmzn_context_get_element_point_ranges_selection(
	struct Context *context);

/***************************************************************************//**
 * Return the IO_stream_package in context.
 *
 * @param context  Pointer to a cmiss_context object.
 * @return  the default IO_stream_package if successfully, otherwise NULL.
 */
struct IO_stream_package *cmzn_context_get_default_IO_stream_package(
	struct Context *context);

/***************************************************************************//**
 * Return the default curve manager in context.
 *
 * @param context  Pointer to a cmiss_context object.
 * @return  the default curve_manager if successfully, otherwise NULL.
 */
struct MANAGER(Curve) *cmzn_context_get_default_curve_manager(
	cmzn_context_id context);

#endif /* !defined (CONTEXT_H) */
