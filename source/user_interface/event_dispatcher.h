/*******************************************************************************
FILE : event_dispatcher.h

LAST MODIFIED : 5 March 2002

DESCRIPTION :
Routines for managing the main event loop in cmiss and dispatching events on
registered file handles to the correct handlers.
==============================================================================*/
#if !defined (EVENT_DISPATCHER_H)
#define EVENT_DISPATCHER_H

#include "general/object.h"
#if defined (USE_XTAPP_CONTEXT)
#include "Xm/Xm.h"
#endif /* defined (USE_XTAPP_CONTEXT) */

/*
Global types
------------
*/

struct Event_dispatcher;

struct Event_dispatcher_file_descriptor_handler;

struct Event_dispatcher_timeout_callback;

struct Event_dispatcher_idle_callback;

typedef int Event_dispatcher_handler_function(int file_descriptor,
	void *user_data);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

typedef int Event_dispatcher_idle_function(void *user_data);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

enum Event_dispatcher_idle_priority
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/
{
	EVENT_DISPATCHER_X_PRIORITY,
	EVENT_DISPATCHER_TRACKING_EDITOR_PRIORITY,
	EVENT_DISPATCHER_IDLE_UPDATE_SCENE_VIEWER_PRIORITY,
	EVENT_DISPATCHER_SYNC_SCENE_VIEWERS_PRIORITY,
	EVENT_DISPATCHER_TUMBLE_SCENE_VIEWER_PRIORITY
};

typedef int Event_dispatcher_timeout_function(void *user_data);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

/*
Global functions
----------------
*/

struct Event_dispatcher *CREATE(Event_dispatcher)(void);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

int DESTROY(Event_dispatcher)(struct Event_dispatcher **event_dispatcher);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Destroys an Event_dispatcher object
==============================================================================*/

struct Event_dispatcher_file_descriptor_handler *Event_dispatcher_add_file_descriptor_handler(
	struct Event_dispatcher *event_dispatcher, int file_descriptor,
	Event_dispatcher_handler_function *handler_function, void *user_data); 
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
If successful returns the callback ID for the handler.
==============================================================================*/

int Event_dispatcher_remove_file_descriptor_handler(
	struct Event_dispatcher *event_dispatcher, 
	struct Event_dispatcher_file_descriptor_handler *callback_id);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

struct Event_dispatcher_timeout_callback *Event_dispatcher_add_timeout_callback_at_time(
	struct Event_dispatcher *event_dispatcher, unsigned long timeout_s, unsigned long timeout_ns,
	Event_dispatcher_timeout_function *timeout_function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

struct Event_dispatcher_timeout_callback *Event_dispatcher_add_timeout_callback(
	struct Event_dispatcher *event_dispatcher, unsigned long timeout_s, unsigned long timeout_ns,
	Event_dispatcher_timeout_function *timeout_function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

int Event_dispatcher_remove_timeout_callback(
	struct Event_dispatcher *event_dispatcher, 
	struct Event_dispatcher_timeout_callback *callback_id);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

struct Event_dispatcher_idle_callback *Event_dispatcher_add_idle_event_callback(
	struct Event_dispatcher *event_dispatcher, 
	Event_dispatcher_idle_function *idle_function, void *user_data,
	enum Event_dispatcher_idle_priority priority);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

int Event_dispatcher_remove_idle_event_callback(
	struct Event_dispatcher *event_dispatcher, 
	struct Event_dispatcher_idle_callback *callback_id);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

struct Event_dispatcher_idle_callback *Event_dispatcher_set_special_idle_callback(
	struct Event_dispatcher *event_dispatcher, 
	Event_dispatcher_idle_function *idle_function, void *user_data,
	enum Event_dispatcher_idle_priority priority);
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
==============================================================================*/

int Event_dispatcher_do_one_event(struct Event_dispatcher *event_dispatcher); 
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

int Event_dispatcher_main_loop(struct Event_dispatcher *event_dispatcher); 
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

int Event_dispatcher_end_main_loop(struct Event_dispatcher *event_dispatcher);
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
==============================================================================*/

#if defined (USE_XTAPP_CONTEXT)
int Event_dispatcher_set_application_context(struct Event_dispatcher *event_dispatcher,
	XtAppContext application_context);
/*******************************************************************************
LAST MODIFIED : 4 June 2002

DESCRIPTION :
==============================================================================*/
#endif /* defined (USE_XTAPP_CONTEXT) */
#endif /* !defined (EVENT_DISPATCHER_H) */
