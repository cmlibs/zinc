/*******************************************************************************
FILE : manager_private.h

LAST MODIFIED : 5 March 2002

DESCRIPTION :
Managers oversee the creation, deletion and modification of global objects -
such as object_types, lights and cameras.  All registered clients of the manager
are sent a message when an object has changed.

This file contains the details of the internal workings of the manager and
should be included in the module that declares the manager for a particular
object type.  This allows changes to the internal structure to be made without
affecting other parts of the program.
???DB.  Remove Manager_type and have a separate Manager_message and
	Manager_callback_procedure for each object ?
???DB.  Name not general enough ?
???DB.  Use B-trees instead of lists ?
???DB.  Include manager in message ?
???DB.  Should there be another field, like access_count, that makes it so that
	only the manager can delete/modify ?  (locked ?)
???GH.  The MANAGER_COPY_NOT_IDENTIFIER functions should be put in object.h,
	and the 'MANAGER' bit removed from the name.
???GH.  The specialised group functions in this file should be in group.h
==============================================================================*/
#if !defined (MANAGER_PRIVATE_H)
#define MANAGER_PRIVATE_H

#include "general/manager.h"

/*
Macros
======
*/

/*
Local types
-----------
*/
#if ! defined (SHORT_NAMES)
#define MANAGER_CALLBACK_ITEM( object_type ) \
	manager_callback_item_ ## object_type
#else
#define MANAGER_CALLBACK_ITEM( object_type )  mi ## object_type
#endif

#define FULL_DECLARE_MANAGER_TYPE( object_type ) \
struct MANAGER_CALLBACK_ITEM(object_type) \
/***************************************************************************** \
LAST MODIFIED : 27 September 1995 \
\
DESCRIPTION : \
An item in the manager's list of callbacks to make when changes are made to \
the objects being managed \
============================================================================*/ \
{ \
	MANAGER_CALLBACK_FUNCTION(object_type) *callback; \
	void *user_data; \
	struct MANAGER_CALLBACK_ITEM(object_type) *next; \
}; /* struct MANAGER_CALLBACK_ITEM(object_type) */ \
\
DECLARE_MANAGER_TYPE(object_type) \
/***************************************************************************** \
LAST MODIFIED : 5 June 2001 \
\
DESCRIPTION : \
The structure for the manager. \
============================================================================*/ \
{ \
	/* the list of objects in the manager */ \
	struct LIST(object_type) *object_list; \
	/* the list of callbacks invoked when manager has changed */ \
	struct MANAGER_CALLBACK_ITEM(object_type) *callback_list; \
	int locked; \
	/* the type of changes since the last update message */ \
	struct MANAGER_MESSAGE(object_type) *message; \
	/* flag indicating whether caching is on */ \
	int cache; \
} /* struct MANAGER(object_type) */

/*
Local functions
---------------
*/

#if ! defined (SHORT_NAMES)
#define MANAGER_UPDATE( object_type )  manager_update_ ## object_type
#else
#define MANAGER_UPDATE( object_type )  mu ## object_type
#endif

#define DECLARE_MANAGER_UPDATE_FUNCTION( object_type ) \
static void MANAGER_UPDATE(object_type)(struct MANAGER(object_type) *manager) \
/***************************************************************************** \
LAST MODIFIED : 16 January 2002 \
\
DESCRIPTION : \
Send a manager message listing all changes to date of the <change> type in the
<changed_object_list> of <manager> to registered clients. \
Once <change> and <changed_object_list> are put in the <message>, they are \
cleared from the manager, then the message is sent. \
Copies the message, leaving the message in the manager blank before sending \
since it is sometimes possible to have messages sent while others are out. \
This is currently the case for heirarchical scenes but this will be changed \
once we use object-to-client callbacks instead of manager messages for \
inter-object dependencies. \
============================================================================*/ \
{ \
	struct LIST(object_type) *new_changed_object_list; \
	struct MANAGER_CALLBACK_ITEM(object_type) *item; \
	struct MANAGER_MESSAGE(object_type) *temp_message; \
\
	ENTER(MANAGER_UPDATE(object_type)); \
	if (manager) \
	{ \
		if (MANAGER_CHANGE_NONE(object_type) != manager->message->change) \
		{ \
			if (ALLOCATE(temp_message, struct MANAGER_MESSAGE(object_type), 1) && \
				(new_changed_object_list = CREATE_LIST(object_type)())) \
			{ \
				/* copy message from manager into temp_message */ \
				temp_message->change = manager->message->change; \
				temp_message->changed_object_list = \
					manager->message->changed_object_list; \
				/* clear manager->message so messages can be build up while previous \
					 temp_message is sent out to clients */ \
				manager->message->change = MANAGER_CHANGE_NONE(object_type); \
				manager->message->changed_object_list = new_changed_object_list; \
				/* send message to the clients */ \
				item = manager->callback_list; \
				while (item) \
				{ \
					(item->callback)(temp_message, item->user_data); \
					item = item->next; \
				} \
				/* discard the changed_object_list in the message just sent */ \
				DESTROY(LIST(object_type))(&(temp_message->changed_object_list)); \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"MANAGER_UPDATE(" #object_type ").  Could not build message"); \
			} \
			/* deallocate the message just sent */ \
			DEALLOCATE(temp_message); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"MANAGER_UPDATE(" #object_type ").  Invalid argument(s)"); \
	} \
	LEAVE; \
} /* MANAGER_UPDATE(object_type) */

#if ! defined (SHORT_NAMES)
#define MANAGER_FIND_CLIENT( object_type )  manager_find_client ## object_type
#else
#define MANAGER_FIND_CLIENT( object_type )  bm ## object_type
#endif

#define DECLARE_MANAGER_FIND_CLIENT_FUNCTION( object_type ) \
struct MANAGER_CALLBACK_ITEM(object_type) *MANAGER_FIND_CLIENT(object_type)( \
	void *callback_id,struct MANAGER(object_type) *manager) \
/***************************************************************************** \
LAST MODIFIED : 27 September 1995 \
\
DESCRIPTION : \
Finds a client based on its callback id. \
???DB.  Is this needed ? \
============================================================================*/ \
{ \
	struct MANAGER_CALLBACK_ITEM(object_type) *item,*return_callback; \
\
	ENTER(MANAGER_FIND_CLIENT(object_type)); \
	if (manager&&callback_id) \
	{ \
		item=manager->callback_list; \
		return_callback=(struct MANAGER_CALLBACK_ITEM(object_type) *)NULL; \
		while (item&&!return_callback) \
		{ \
			if (callback_id==item) \
			{ \
				return_callback=item; \
			} \
			else \
			{ \
				item=item->next; \
			} \
		} \
		if (!return_callback) \
		{ \
			display_message(ERROR_MESSAGE, \
				"MANAGER_FIND_CLIENT(" #object_type ").  Could not find client"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"MANAGER_FIND_CLIENT(" #object_type ").  Invalid argument(s)"); \
		return_callback=(struct MANAGER_CALLBACK_ITEM(object_type) *)NULL; \
	} \
	LEAVE; \
\
	return (return_callback); \
} /* MANAGER_FIND_CLIENT(object_type) */

#if ! defined (SHORT_NAMES)
#define MANAGED_OBJECT_NOT_IN_USE_CONDITIONAL( object_type ) \
	managed_object_not_in_use_conditional_ ## object_type
#else
#define MANAGED_OBJECT_NOT_IN_USE_CONDITIONAL( object_type ) \
	moniuc ## object_type
#endif

#define DECLARE_MANAGED_OBJECT_NOT_IN_USE_CONDITIONAL_FUNCTION( object_type ) \
static int MANAGED_OBJECT_NOT_IN_USE_CONDITIONAL(object_type)( \
	struct object_type *object, void *manager_void) \
/***************************************************************************** \
LAST MODIFIED : 18 January 2002 \
\
DESCRIPTION : \
List conditional version of MANAGED_OBJECT_NOT_IN_USE function. \
============================================================================*/ \
{ \
	int return_code; \
\
	ENTER(MANAGED_OBJECT_NOT_IN_USE_CONDITIONAL(object_type)); \
	return_code = MANAGED_OBJECT_NOT_IN_USE(object_type)(object, \
		(struct MANAGER(object_type) *)manager_void); \
	LEAVE; \
\
	return (return_code); \
} /* MANAGED_OBJECT_NOT_IN_USE_CONDITIONAL(object_type) */

#if ! defined (SHORT_NAMES)
#define OBJECT_WITH_MANAGER_REMOVE_MANAGER( object_type ) \
	object_with_manager_remove_manager ## object_type
#else
#define OBJECT_WITH_MANAGER_REMOVE_MANAGER( object_type )  owmrm ## object_type
#endif

#define DECLARE_OBJECT_WITH_MANAGER_REMOVE_MANAGER_FUNCTION( object_type , object_manager ) \
static int OBJECT_WITH_MANAGER_REMOVE_MANAGER(object_type)(struct object_type *object, \
	void *dummy_user_data) \
/***************************************************************************** \
LAST MODIFIED : 10 August 2000 \
\
DESCRIPTION : \
Remove the reference to the manager from the object \
============================================================================*/ \
{ \
	int return_code; \
\
	ENTER(OBJECT_WITH_MANAGER_REMOVE_MANAGER(object_type)); \
   USE_PARAMETER(dummy_user_data); \
	if (object) \
	{ \
      object->object_manager = (struct MANAGER(object_type) *)NULL; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"OBJECT_WITH_MANAGER_REMOVE_MANAGER(" #object_type ").  Missing object"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* OBJECT_WITH_MANAGER_REMOVE_MANAGER(object_type) */

/*
Global functions
----------------
*/
#define DECLARE_CREATE_MANAGER_FUNCTION( object_type ) \
PROTOTYPE_CREATE_MANAGER_FUNCTION(object_type) \
{ \
	struct MANAGER(object_type) *manager; \
\
	ENTER(CREATE_MANAGER(object_type)); \
	if (ALLOCATE(manager, struct MANAGER(object_type), 1)) \
	{ \
		manager->object_list = CREATE_LIST(object_type)(); \
		if (ALLOCATE(manager->message, struct MANAGER_MESSAGE(object_type), 1)) \
		{ \
			manager->message->change = MANAGER_CHANGE_NONE(object_type); \
			manager->message->changed_object_list = CREATE_LIST(object_type)(); \
		} \
		if (manager->object_list && manager->message && \
			manager->message->changed_object_list) \
		{ \
			manager->callback_list = \
				(struct MANAGER_CALLBACK_ITEM(object_type) *)NULL; \
			manager->locked = 0; \
			manager->cache = 0; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"MANAGER_CREATE(" #object_type ").  Could not create object lists"); \
			if (manager->message) \
			{ \
				DESTROY(LIST(object_type))(&(manager->message->changed_object_list)); \
				DEALLOCATE(manager->message); \
			} \
			DESTROY(LIST(object_type))(&(manager->object_list)); \
			DEALLOCATE(manager); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"MANAGER_CREATE(" #object_type ").  Could not allocate memory"); \
	} \
	LEAVE; \
\
	return (manager); \
} /* MANAGER_CREATE(object_type) */

/* Note!!! there are 2 DESTROY functions - the extra one is for destroying
	 objects that store a pointer to their manager! Keep them consistent! */
#define DECLARE_DESTROY_MANAGER_FUNCTION( object_type ) \
PROTOTYPE_DESTROY_MANAGER_FUNCTION(object_type) \
{ \
	int return_code; \
	struct MANAGER(object_type) *manager; \
	struct MANAGER_CALLBACK_ITEM(object_type) *current, *next; \
\
	ENTER(DESTROY_MANAGER(object_type)); \
	if (manager_address && (manager = *manager_address)) \
	{ \
		return_code = 1; \
		/* warn if cache should has not to zero */ \
		if (0 != manager->cache) \
		{ \
			display_message(ERROR_MESSAGE, "DESTROY(MANAGER(" #object_type \
				")).  manager->cache = %d != 0", manager->cache); \
		} \
		/* destroy the manager message */ \
		if (manager->message) \
		{ \
			DESTROY(LIST(object_type))(&(manager->message->changed_object_list)); \
			DEALLOCATE(manager->message); \
		} \
		/* destroy the list of objects in the manager */ \
		DESTROY_LIST(object_type)(&(manager->object_list)); \
		/* destroy the callback list, after the list of objects as some Computed fields get 
		 Computed_field_manager callbacks */ \
		current=manager->callback_list; \
		while (current) \
		{ \
			next = current->next; \
			DEALLOCATE(current); \
			current = next; \
		} \
      DEALLOCATE(manager); \
	} \
	else \
	{ \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* DESTROY_MANAGER(object_type) */

/* Note!!! there are 2 DESTROY functions - the extra one is for destroying
	 objects that store a pointer to their manager! Keep them constistent! */
#define DECLARE_DESTROY_OBJECT_WITH_MANAGER_MANAGER_FUNCTION( object_type , \
	object_manager ) \
PROTOTYPE_DESTROY_MANAGER_FUNCTION(object_type) \
{ \
	int return_code; \
	struct MANAGER(object_type) *manager; \
	struct MANAGER_CALLBACK_ITEM(object_type) *current,*next; \
\
	ENTER(DESTROY_OBJECT_WITH_MANAGER_MANAGER(object_type)); \
	if (manager_address && (manager= *manager_address)) \
	{ \
		return_code = 1; \
		/* warn if cache should has not to zero */ \
		if (0 != manager->cache) \
		{ \
			display_message(ERROR_MESSAGE, "DESTROY(MANAGER(" #object_type \
				")).  manager->cache = %d != 0", manager->cache); \
		} \
		/* destroy the manager message */ \
		if (manager->message) \
		{ \
			DESTROY(LIST(object_type))(&(manager->message->changed_object_list)); \
			DEALLOCATE(manager->message); \
		} \
		/* remove the manager_pointer from each object */ \
		FOR_EACH_OBJECT_IN_LIST(object_type)( \
			OBJECT_WITH_MANAGER_REMOVE_MANAGER(object_type), \
			(void *)NULL, manager->object_list); \
		/* destroy the list of objects in the manager */ \
		DESTROY_LIST(object_type)(&(manager->object_list)); \
		/* destroy the callback list, after the list of objects as some Computed fields get 
		 Computed_field_manager callbacks */ \
		current=manager->callback_list; \
		while (current) \
		{ \
			next = current->next; \
			DEALLOCATE(current); \
			current = next; \
		} \
      DEALLOCATE(manager); \
	} \
	else \
	{ \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* DESTROY_OBJECT_WITH_MANAGER_MANAGER(object_type) */

#define DECLARE_ADD_OBJECT_TO_MANAGER_FUNCTION( object_type , \
	identifier_field_name ) \
PROTOTYPE_ADD_OBJECT_TO_MANAGER_FUNCTION(object_type) \
{ \
	int return_code; \
\
	ENTER(ADD_OBJECT_TO_MANAGER(object_type)); \
	if (manager && object) \
	{ \
		if (!(manager->locked)) \
		{ \
			/* can only add if new identifier not already in use in manager */ \
			if ((struct object_type *)NULL == \
				FIND_BY_IDENTIFIER_IN_LIST(object_type, identifier_field_name)( \
					object->identifier_field_name, manager->object_list)) \
			{ \
				MANAGER_BEGIN_CHANGE(object_type)(manager, \
					MANAGER_CHANGE_ADD(object_type), object); \
				/* object_list will access the object */ \
				if (ADD_OBJECT_TO_LIST(object_type)(object, manager->object_list)) \
				{ \
					return_code = 1; \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
						"ADD_OBJECT_TO_MANAGER(" #object_type \
						").  Could not add object to list"); \
					return_code = 0; \
				} \
				MANAGER_END_CHANGE(object_type)(manager); \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"ADD_OBJECT_TO_MANAGER(" #object_type \
					").  Object with that identifier already in manager"); \
				return_code = 0; \
			} \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"ADD_OBJECT_TO_MANAGER(" #object_type ").  Manager locked"); \
			return_code = 0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"ADD_OBJECT_TO_MANAGER(" #object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* ADD_OBJECT_TO_MANAGER(object_type) */

/* Special version of ADD_OBJECT_TO_MANAGER for objects that maintain a */
/* pointer to their manager, eg. Graphics_window, Scene and GROUP(). */
/* NOTE: MUST clear the objects pointer to manager in its CREATE function! */
#define DECLARE_ADD_OBJECT_WITH_MANAGER_TO_MANAGER_FUNCTION( object_type , \
	identifier_field_name , object_manager ) \
PROTOTYPE_ADD_OBJECT_TO_MANAGER_FUNCTION(object_type) \
{ \
	int return_code; \
\
	ENTER(ADD_OBJECT_WITH_MANAGER_TO_MANAGER(object_type)); \
	if (manager && object) \
	{ \
		if (!object->object_manager) \
		{ \
			if (!(manager->locked)) \
			{ \
				/* can only add if new identifier not already in use in manager */ \
				if ((struct object_type *)NULL == \
					FIND_BY_IDENTIFIER_IN_LIST(object_type, identifier_field_name)( \
						object->identifier_field_name, manager->object_list)) \
				{ \
					MANAGER_BEGIN_CHANGE(object_type)(manager, \
						MANAGER_CHANGE_ADD(object_type), object); \
					/* object_list will access the object */ \
					if (ADD_OBJECT_TO_LIST(object_type)(object, manager->object_list)) \
					{ \
						/* object keeps a pointer to its manager */ \
						object->object_manager = manager; \
						return_code = 1; \
					} \
					else \
					{ \
						display_message(ERROR_MESSAGE, \
							"ADD_OBJECT_WITH_MANAGER_TO_MANAGER(" #object_type \
							").  Could not add object to list"); \
						return_code = 0; \
					} \
					MANAGER_END_CHANGE(object_type)(manager); \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
						"ADD_OBJECT_WITH_MANAGER_TO_MANAGER(" #object_type \
						").  Object with that identifier already in manager"); \
					return_code = 0; \
				} \
			} \
			else \
			{ \
				display_message(WARNING_MESSAGE, \
					"ADD_OBJECT_WITH_MANAGER_TO_MANAGER(" #object_type \
					").  Manager locked"); \
				return_code = 0; \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"ADD_OBJECT_WITH_MANAGER_TO_MANAGER(" #object_type \
				").  Object already managed"); \
			return_code = 0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"ADD_OBJECT_WITH_MANAGER_TO_MANAGER(" #object_type \
			").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* ADD_OBJECT_WITH_MANAGER_TO_MANAGER(object_type) */

#define DECLARE_REMOVE_OBJECT_FROM_MANAGER_FUNCTION( object_type ) \
PROTOTYPE_REMOVE_OBJECT_FROM_MANAGER_FUNCTION(object_type) \
{ \
	int return_code; \
\
	ENTER(REMOVE_OBJECT_FROM_MANAGER(object_type)); \
	if (manager && object) \
	{ \
		if (!(manager->locked)) \
		{ \
			if (MANAGED_OBJECT_NOT_IN_USE(object_type)(object, manager)) \
			{ \
				MANAGER_BEGIN_CHANGE(object_type)(manager, \
					MANAGER_CHANGE_REMOVE(object_type), object); \
				return_code = REMOVE_OBJECT_FROM_LIST(object_type)(object, \
					manager->object_list); \
				MANAGER_END_CHANGE(object_type)(manager); \
			} \
			else \
			{ \
				if (IS_OBJECT_IN_LIST(object_type)(object, manager->object_list)) \
				{ \
					display_message(ERROR_MESSAGE, \
						"REMOVE_OBJECT_FROM_MANAGER(" #object_type \
						").  Object is in use"); \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
						"REMOVE_OBJECT_FROM_MANAGER(" #object_type \
						").  Object is not managed"); \
				} \
				return_code = 0; \
			} \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"REMOVE_OBJECT_FROM_MANAGER(" #object_type ").  Manager locked"); \
			return_code = 0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"REMOVE_OBJECT_FROM_MANAGER(" #object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* REMOVE_OBJECT_FROM_MANAGER(object_type) */

/* Special version of REMOVE_OBJECT_FROM_MANAGER for objects that maintain a */
/* pointer to their manager, eg. Graphics_window, Scene and GROUP(). */
#define DECLARE_REMOVE_OBJECT_WITH_MANAGER_FROM_MANAGER_FUNCTION( \
	object_type , object_manager ) \
PROTOTYPE_REMOVE_OBJECT_FROM_MANAGER_FUNCTION(object_type) \
{ \
	int return_code; \
\
	ENTER(REMOVE_OBJECT_WITH_MANAGER_FROM_MANAGER(object_type)); \
	if (manager && object) \
	{ \
		if (!(manager->locked)) \
		{ \
			if (MANAGED_OBJECT_NOT_IN_USE(object_type)(object, manager)) \
			{ \
				MANAGER_BEGIN_CHANGE(object_type)(manager, \
					MANAGER_CHANGE_REMOVE(object_type), object); \
				/* clear object's pointer to manager */ \
				object->object_manager = (struct MANAGER(object_type) *)NULL; \
				return_code = REMOVE_OBJECT_FROM_LIST(object_type)(object, \
					manager->object_list); \
				MANAGER_END_CHANGE(object_type)(manager); \
			} \
			else \
			{ \
				if (IS_OBJECT_IN_LIST(object_type)(object, manager->object_list)) \
				{ \
					display_message(ERROR_MESSAGE, \
						"REMOVE_OBJECT_FROM_MANAGER(" #object_type \
						").  Object is in use"); \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
						"REMOVE_OBJECT_FROM_MANAGER(" #object_type \
						").  Object is not managed"); \
				} \
				return_code = 0; \
			} \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"REMOVE_OBJECT_WITH_MANAGER_FROM_MANAGER(" #object_type \
				").  Manager locked"); \
			return_code = 0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"REMOVE_OBJECT_WITH_MANAGER_FROM_MANAGER(" #object_type \
			").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* REMOVE_OBJECT_WITH_MANAGER_FROM_MANAGER(object_type) */

#define DECLARE_REMOVE_ALL_OBJECTS_FROM_MANAGER_FUNCTION( object_type ) \
PROTOTYPE_REMOVE_ALL_OBJECTS_FROM_MANAGER_FUNCTION(object_type) \
{ \
	int return_code; \
	struct object_type *object; \
\
	ENTER(REMOVE_ALL_OBJECTS_FROM_MANAGER(object_type)); \
	if (manager) \
	{ \
		if (!(manager->locked)) \
		{ \
			return_code = 1; \
			/* this function uses "temporary" caching by having many BEGIN_CHANGE \
				 calls followed by a single END_CHANGE */ \
			while (return_code && (object = FIRST_OBJECT_IN_LIST_THAT(object_type)( \
				MANAGED_OBJECT_NOT_IN_USE_CONDITIONAL(object_type), (void *)manager, \
				manager->object_list))) \
			{ \
				MANAGER_BEGIN_CHANGE(object_type)(manager, \
					MANAGER_CHANGE_REMOVE(object_type), object); \
				return_code = \
					REMOVE_OBJECT_FROM_LIST(object_type)(object, manager->object_list); \
			} \
			if (0 != NUMBER_IN_MANAGER(object_type)(manager)) \
			{ \
				display_message(ERROR_MESSAGE, \
					"REMOVE_ALL_OBJECTS_FROM_MANAGER(" #object_type \
					").  %d items could not be removed", \
					NUMBER_IN_MANAGER(object_type)(manager)); \
				return_code = 0; \
			} \
			MANAGER_END_CHANGE(object_type)(manager); \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"REMOVE_ALL_OBJECTS_FROM_MANAGER(" #object_type ").  Manager locked"); \
			return_code = 0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"REMOVE_ALL_OBJECTS_FROM_MANAGER(" #object_type \
			").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* REMOVE_ALL_OBJECTS_FROM_MANAGER(object_type) */

/* Special version of REMOVE_ALL_OBJECTS_FROM_MANAGER for objects that maintain
	 a pointer to their manager, eg. Graphics_window, Scene and GROUP(). */
#define DECLARE_REMOVE_ALL_OBJECTS_WITH_MANAGER_FROM_MANAGER_FUNCTION( \
	object_type , object_manager ) \
PROTOTYPE_REMOVE_ALL_OBJECTS_FROM_MANAGER_FUNCTION(object_type) \
{ \
	int return_code; \
	struct object_type *object; \
\
	ENTER(REMOVE_ALL_OBJECTS_WITH_MANAGER_FROM_MANAGER(object_type)); \
	if (manager) \
	{ \
		if (!(manager->locked)) \
		{ \
			return_code = 1; \
			/* this function uses "temporary" caching by having many BEGIN_CHANGE \
				 calls followed by a single END_CHANGE */ \
			while (return_code && (object = FIRST_OBJECT_IN_LIST_THAT(object_type)( \
				MANAGED_OBJECT_NOT_IN_USE_CONDITIONAL(object_type), (void *)manager, \
				manager->object_list))) \
			{ \
				MANAGER_BEGIN_CHANGE(object_type)(manager, \
					MANAGER_CHANGE_REMOVE(object_type), object); \
				/* clear object's pointer to manager */ \
				object->object_manager = (struct MANAGER(object_type) *)NULL; \
				return_code = \
					REMOVE_OBJECT_FROM_LIST(object_type)(object, manager->object_list); \
			} \
			if (0 != NUMBER_IN_MANAGER(object_type)(manager)) \
			{ \
				display_message(ERROR_MESSAGE, \
					"REMOVE_ALL_OBJECTS_WITH_MANAGER_FROM_MANAGER(" #object_type \
					").  %d items could not be removed", \
					NUMBER_IN_MANAGER(object_type)(manager)); \
				return_code = 0; \
			} \
			MANAGER_END_CHANGE(object_type)(manager); \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"REMOVE_ALL_OBJECTS_WITH_MANAGER_FROM_MANAGER(" #object_type \
				").  Manager locked"); \
			return_code = 0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"REMOVE_ALL_OBJECTS_WITH_MANAGER_FROM_MANAGER(" #object_type \
			").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* REMOVE_ALL_OBJECTS_WITH_MANAGER_FROM_MANAGER(object_type) */

#define DECLARE_NUMBER_IN_MANAGER_FUNCTION( object_type ) \
PROTOTYPE_NUMBER_IN_MANAGER_FUNCTION(object_type) \
{ \
	int return_code; \
\
	ENTER(NUMBER_IN_MANAGER(object_type)); \
	if (manager) \
	{ \
		if (!(manager->locked)) \
		{ \
			return_code = NUMBER_IN_LIST(object_type)(manager->object_list); \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"NUMBER_IN_MANAGER(" #object_type ").  Manager is locked"); \
			return_code = 0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"NUMBER_IN_MANAGER(" #object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* NUMBER_IN_MANAGER(object_type) */

#define DECLARE_MANAGER_MODIFY_FUNCTION( object_type , identifier_field_name ) \
PROTOTYPE_MANAGER_MODIFY_FUNCTION(object_type,identifier_field_name) \
{ \
	int return_code; \
	struct LIST_IDENTIFIER_CHANGE_DATA(object_type, \
		identifier_field_name) *identifier_change_data; \
	struct object_type *tmp_object; \
\
	ENTER(MANAGER_MODIFY(object_type,identifier_field_name)); \
	if (manager && object && new_data) \
	{ \
		if (!(manager->locked)) \
		{ \
			if (IS_OBJECT_IN_LIST(object_type)(object, manager->object_list)) \
			{ \
				if (tmp_object = \
					FIND_BY_IDENTIFIER_IN_LIST(object_type, identifier_field_name)( \
						new_data->identifier_field_name, manager->object_list)) \
				{ \
					if (tmp_object == object) \
					{ \
						/* don't need to copy object over itself */ \
						return_code = 1; \
					} \
					else \
					{ \
						display_message(ERROR_MESSAGE, \
							"MANAGER_MODIFY(" #object_type "," #identifier_field_name \
							").  Source object is also in manager"); \
						return_code = 0; \
					} \
				} \
				else \
				{ \
					/* must perform IDENTIFIER_CHANGE stuff between BEGIN_CHANGE and \
						 END_CHANGE calls; manager message must not be sent while object \
						 is part changed and/or temporarily out of the manager! */ \
					MANAGER_BEGIN_CHANGE(object_type)(manager, \
						MANAGER_CHANGE_OBJECT(object_type), object); \
					if (identifier_change_data = \
						LIST_BEGIN_IDENTIFIER_CHANGE(object_type, \
							identifier_field_name)(object)) \
					{ \
						if (!(return_code = MANAGER_COPY_WITH_IDENTIFIER(object_type, \
							identifier_field_name)(object, new_data))) \
						{ \
							display_message(ERROR_MESSAGE, \
								"MANAGER_MODIFY(" #object_type "," #identifier_field_name \
								").  Could not copy object"); \
						} \
						if (!LIST_END_IDENTIFIER_CHANGE(object_type, \
							identifier_field_name)(&identifier_change_data)) \
						{ \
							display_message(ERROR_MESSAGE, \
								"MANAGER_MODIFY(" #object_type "," #identifier_field_name \
								").  Could not restore object to all indexed lists"); \
						} \
					} \
					else \
					{ \
						display_message(ERROR_MESSAGE, \
							"MANAGER_MODIFY(" #object_type "," #identifier_field_name \
							").  Could not safely change identifier in indexed lists"); \
						return_code = 0; \
					} \
					MANAGER_END_CHANGE(object_type)(manager); \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"MANAGER_MODIFY(" #object_type "," #identifier_field_name \
					").  Object is not managed"); \
				return_code = 0; \
			} \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"MANAGER_MODIFY(" #object_type "," #identifier_field_name \
				").  Manager locked"); \
			return_code = 0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"MANAGER_MODIFY(" #object_type "," #identifier_field_name \
			").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* MANAGER_MODIFY(object_type,identifier_field_name) */

#define DECLARE_MANAGER_MODIFY_NOT_IDENTIFIER_FUNCTION( object_type , \
	identifier_field_name ) \
PROTOTYPE_MANAGER_MODIFY_NOT_IDENTIFIER_FUNCTION(object_type, \
	identifier_field_name) \
{ \
	int return_code; \
\
	ENTER(MANAGER_MODIFY_NOT_IDENTIFIER(object_type,identifier_field_name)); \
	if (manager && object && new_data) \
	{ \
		if (!(manager->locked)) \
		{ \
			if (IS_OBJECT_IN_LIST(object_type)(object,manager->object_list)) \
			{ \
				MANAGER_BEGIN_CHANGE(object_type)(manager, \
					MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(object_type), object); \
				if (MANAGER_COPY_WITHOUT_IDENTIFIER(object_type, \
					identifier_field_name)(object, new_data)) \
				{ \
					return_code = 1; \
				} \
				else  \
				{ \
					display_message(ERROR_MESSAGE, \
						"MANAGER_MODIFY_NOT_IDENTIFIER(" #object_type "," \
						#identifier_field_name ").  Could not copy object"); \
					return_code = 0; \
				} \
				MANAGER_END_CHANGE(object_type)(manager); \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"MANAGER_MODIFY_NOT_IDENTIFIER(" #object_type "," \
					#identifier_field_name ").  Object is not managed"); \
				return_code = 0; \
			} \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"MANAGER_MODIFY_NOT_IDENTIFIER(" #object_type "," \
				#identifier_field_name ").  Manager is locked"); \
			return_code = 0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"MANAGER_MODIFY_NOT_IDENTIFIER(" #object_type "," \
			#identifier_field_name ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* MANAGER_MODIFY_NOT_IDENTIFIER(object_type,identifier_field_name) */

#define DECLARE_MANAGER_MODIFY_IDENTIFIER_FUNCTION( object_type , \
	identifier_field_name , identifier_type ) \
PROTOTYPE_MANAGER_MODIFY_IDENTIFIER_FUNCTION(object_type, \
	identifier_field_name,identifier_type) \
{ \
	int return_code; \
	struct LIST_IDENTIFIER_CHANGE_DATA(object_type, \
		identifier_field_name) *identifier_change_data; \
	struct object_type *tmp_object; \
\
	ENTER(MANAGER_MODIFY_IDENTIFIER(object_type,identifier_field_name)); \
	if (manager && object && new_identifier) \
	{ \
		if (!(manager->locked)) \
		{ \
			if (IS_OBJECT_IN_LIST(object_type)(object,manager->object_list)) \
			{ \
				if (tmp_object = \
					FIND_BY_IDENTIFIER_IN_LIST(object_type, identifier_field_name)( \
						new_identifier, manager->object_list)) \
				{ \
					if (tmp_object == object) \
					{ \
						/* don't need to copy identifier of object over itself */ \
						return_code = 1; \
					} \
					else \
					{ \
						display_message(ERROR_MESSAGE,"MANAGER_MODIFY_IDENTIFIER(" \
							#object_type "," #identifier_field_name \
							").  Object with new identifier already exists"); \
						return_code = 0; \
					} \
				} \
				else \
				{ \
					/* must perform IDENTIFIER_CHANGE stuff between BEGIN_CHANGE and \
						 END_CHANGE calls; manager message must not be sent while object \
						 is part changed and/or temporarily out of the manager! */ \
					MANAGER_BEGIN_CHANGE(object_type)(manager, \
						MANAGER_CHANGE_IDENTIFIER(object_type), object); \
					if (identifier_change_data = \
						LIST_BEGIN_IDENTIFIER_CHANGE(object_type, \
							identifier_field_name)(object)) \
					{ \
						if (!(return_code = MANAGER_COPY_IDENTIFIER(object_type, \
							identifier_field_name)(object, new_identifier))) \
						{ \
							display_message(ERROR_MESSAGE, \
								"MANAGER_MODIFY_IDENTIFIER(" #object_type "," \
								#identifier_field_name ").  Could not copy identifier"); \
						} \
						if (!LIST_END_IDENTIFIER_CHANGE(object_type, \
							identifier_field_name)(&identifier_change_data)) \
						{ \
							display_message(ERROR_MESSAGE, \
								"MANAGER_MODIFY_IDENTIFIER(" #object_type "," \
								#identifier_field_name \
								").  Could not restore object to all indexed lists"); \
						} \
					} \
					else  \
					{ \
						display_message(ERROR_MESSAGE, \
							"MANAGER_MODIFY_IDENTIFIER(" #object_type "," \
							#identifier_field_name \
							").  Could not safely change identifier in indexed lists"); \
						return_code = 0; \
					} \
					MANAGER_END_CHANGE(object_type)(manager); \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"MANAGER_MODIFY_IDENTIFIER(" #object_type "," #identifier_field_name \
					").  Object is not managed"); \
				return_code=0; \
			} \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"MANAGER_MODIFY_IDENTIFIER(" #object_type "," #identifier_field_name \
				").  Manager is locked"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"MANAGER_MODIFY_IDENTIFIER(" #object_type "," #identifier_field_name \
			").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* MANAGER_MODIFY_IDENTIFIER(object_type,identifier_field_name) */

#define DECLARE_MANAGER_REGISTER_FUNCTION( object_type ) \
PROTOTYPE_MANAGER_REGISTER_FUNCTION(object_type) \
{ \
	struct MANAGER_CALLBACK_ITEM(object_type) *new_callback; \
	void *callback_id; \
\
	ENTER(MANAGER_REGISTER(object_type)); \
	if (manager&&callback) \
	{ \
		if (ALLOCATE(new_callback,struct MANAGER_CALLBACK_ITEM(object_type),1)) \
		{ \
			new_callback->next=manager->callback_list; \
			new_callback->callback=callback; \
			new_callback->user_data=user_data; \
			manager->callback_list=new_callback; \
			callback_id=(void *)new_callback; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
			"MANAGER_REGISTER(" #object_type ").  Could not allocate new callback"); \
			callback_id=(void *)NULL; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"MANAGER_REGISTER(" #object_type ").  Invalid argument(s)"); \
		callback_id=(void *)NULL; \
	} \
	LEAVE; \
\
	return (callback_id); \
} /* MANAGER_REGISTER(object_type) */

#define DECLARE_MANAGER_DEREGISTER_FUNCTION( object_type ) \
PROTOTYPE_MANAGER_DEREGISTER_FUNCTION(object_type) \
{ \
	int return_code; \
	struct MANAGER_CALLBACK_ITEM(object_type) *item,**item_address; \
\
	ENTER(MANAGER_DEREGISTER(object_type)); \
	if (manager&&callback_id) \
	{ \
		item_address= &(manager->callback_list); \
		while ((*item_address)&&(*item_address!=callback_id)) \
		{ \
				item_address= &((*item_address)->next); \
		} \
		if (item= *item_address) \
		{ \
			/* found it */ \
			*item_address=item->next; \
			DEALLOCATE(item); \
			return_code=1; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"MANAGER_DEREGISTER(" #object_type ").  Could not find callback"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"MANAGER_DEREGISTER(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* MANAGER_DEREGISTER(object_type) */

#define DECLARE_IS_MANAGED_FUNCTION( object_type ) \
PROTOTYPE_IS_MANAGED_FUNCTION(object_type) \
{ \
	int return_code; \
\
	ENTER(IS_MANAGED(object_type)); \
	if (manager&&object) \
	{ \
		if (!(manager->locked)) \
		{ \
			return_code=IS_OBJECT_IN_LIST(object_type)(object,manager->object_list); \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"IS_MANAGED(" #object_type ").  Manager is locked"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"IS_MANAGED(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* IS_MANAGED(object_type) */

#define DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION( object_type ) \
PROTOTYPE_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 21 January 2002 \
\
DESCRIPTION : \
Returns true if <object> is only accessed by the manager or other managed \
objects. In general, a true result is sufficient to indicate the object may be \
removed from the manager or modified. \
This default version may be used for any object that cannot be accessed by \
other objects in the manager. It only checks the object is accessed just by \
the manager's object_list and changed_object_list. \
FE_element type requires a special version due to face/parent accessing. \
============================================================================*/ \
{ \
	int return_code; \
\
	ENTER(MANAGED_OBJECT_NOT_IN_USE(object_type)); \
	return_code = 0; \
	if (manager && object) \
	{ \
		if (!(manager->locked)) \
		{ \
			if (IS_OBJECT_IN_LIST(object_type)(object, manager->object_list)) \
			{ \
				if ((1 == object->access_count) || \
					((2 == object->access_count) && \
						IS_OBJECT_IN_LIST(object_type)(object, \
							manager->message->changed_object_list))) \
				{ \
					return_code = 1; \
				} \
			} \
			else \
			{ \
				display_message(WARNING_MESSAGE, \
					"MANAGED_OBJECT_NOT_IN_USE(" #object_type \
					").  Object is not managed"); \
			} \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"MANAGED_OBJECT_NOT_IN_USE(" #object_type ").  Manager is locked"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"MANAGED_OBJECT_NOT_IN_USE(" #object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* MANAGED_OBJECT_NOT_IN_USE(object_type) */

#define DECLARE_FIND_BY_IDENTIFIER_IN_MANAGER_FUNCTION( object_type , \
	identifier , identifier_type ) \
PROTOTYPE_FIND_BY_IDENTIFIER_IN_MANAGER_FUNCTION(object_type,identifier, \
	identifier_type) \
{ \
	struct object_type *object; \
\
	ENTER(FIND_BY_IDENTIFIER_IN_MANAGER(object_type,identifier)); \
	if (manager) \
	{ \
		if (!(manager->locked)) \
		{ \
			object=FIND_BY_IDENTIFIER_IN_LIST(object_type,identifier)( \
				identifier,manager->object_list); \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"FIND_BY_IDENTIFIER_IN_LIST(" #object_type "," #identifier \
				").  Manager is locked"); \
			object=(struct object_type *)NULL; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"FIND_BY_IDENTIFIER_IN_LIST(" #object_type "," #identifier \
			").  Invalid argument(s)"); \
		object=(struct object_type *)NULL; \
	} \
	LEAVE; \
\
	return (object); \
} /* FIND_BY_IDENTIFIER_IN_MANAGER(object_type,identifier) */

#define DECLARE_FIRST_OBJECT_IN_MANAGER_THAT_FUNCTION( object_type ) \
PROTOTYPE_FIRST_OBJECT_IN_MANAGER_THAT_FUNCTION(object_type) \
{ \
	struct object_type *object; \
\
	ENTER(FIRST_OBJECT_IN_MANAGER_THAT(object_type)); \
	if (manager) \
	{ \
		object=FIRST_OBJECT_IN_LIST_THAT(object_type)(conditional,user_data, \
			manager->object_list); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"FIRST_OBJECT_IN_MANAGER_THAT(" #object_type ").  Invalid argument(s)"); \
		object=(struct object_type *)NULL; \
	} \
	LEAVE; \
\
	return (object); \
} /* FIRST_OBJECT_IN_MANAGER_THAT(object_type) */

#define DECLARE_FOR_EACH_OBJECT_IN_MANAGER_FUNCTION( object_type ) \
PROTOTYPE_FOR_EACH_OBJECT_IN_MANAGER_FUNCTION(object_type) \
{ \
	int return_code; \
\
	ENTER(FOR_EACH_OBJECT_IN_MANAGER(object_type)); \
	if (manager&&iterator) \
	{ \
		return_code=FOR_EACH_OBJECT_IN_LIST(object_type)(iterator,user_data, \
			manager->object_list); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"FOR_EACH_OBJECT_IN_MANAGER(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* FOR_EACH_OBJECT_IN_MANAGER(object_type) */

#define DECLARE_MANAGER_BEGIN_CHANGE_FUNCTION( object_type ) \
PROTOTYPE_MANAGER_BEGIN_CHANGE_FUNCTION(object_type) \
{ \
	ENTER(MANAGER_BEGIN_CHANGE(object_type)); \
	if (manager && (MANAGER_CHANGE_NONE(object_type) != change) && object) \
	{ \
		if ((MANAGER_CHANGE_NONE(object_type) != manager->message->change) && \
			(change != manager->message->change)) \
		{ \
			/* new type of change; inform clients of changes to date */ \
			MANAGER_UPDATE(object_type)(manager); \
		} \
		manager->message->change = change; \
		/* ensure object is in changed_object_list for future messages */ \
		if (!IS_OBJECT_IN_LIST(object_type)(object, \
			manager->message->changed_object_list)) \
		{ \
			ADD_OBJECT_TO_LIST(object_type)(object, \
				manager->message->changed_object_list); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"MANAGER_BEGIN_CHANGE(" #object_type ").  Invalid argument(s)"); \
	} \
	LEAVE; \
} /* MANAGER_BEGIN_CHANGE(object_type) */

#define DECLARE_MANAGER_END_CHANGE_FUNCTION( object_type ) \
PROTOTYPE_MANAGER_END_CHANGE_FUNCTION(object_type) \
{ \
	ENTER(MANAGER_END_CHANGE(object_type)); \
	if (manager) \
	{ \
		if (!manager->cache) \
		{ \
			/* inform clients of changes to date */ \
			MANAGER_UPDATE(object_type)(manager); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"MANAGER_END_CHANGE(" #object_type ").  Invalid argument(s)"); \
	} \
	LEAVE; \
} /* MANAGER_END_CHANGE(object_type) */

#define DECLARE_MANAGER_BEGIN_CACHE_FUNCTION( object_type ) \
PROTOTYPE_MANAGER_BEGIN_CACHE_FUNCTION(object_type) \
{ \
	int return_code; \
\
	ENTER(MANAGER_BEGIN_CACHE(object_type)); \
	if (manager) \
	{ \
		/* increment cache so we can safely nest caching */ \
		(manager->cache)++; \
		return_code = 1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"MANAGER_BEGIN_CACHE(" #object_type ").  Invalid argument"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* MANAGER_BEGIN_CACHE(object_type) */

#define DECLARE_MANAGER_END_CACHE_FUNCTION( object_type ) \
PROTOTYPE_MANAGER_END_CACHE_FUNCTION(object_type) \
{ \
	int return_code; \
\
	ENTER(MANAGER_END_CACHE(object_type)); \
	if (manager) \
	{ \
		if (0 < manager->cache) \
		{ \
			/* decrement cache so we can safely nest caching */ \
			(manager->cache)--; \
			if (0 == manager->cache) \
			{ \
				/* once cache has run out, inform clients of all changes to date */ \
				MANAGER_UPDATE(object_type)(manager); \
			} \
			return_code = 1; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"MANAGER_END_CACHE(" #object_type ").  Caching not enabled"); \
			return_code = 0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"MANAGER_END_CACHE(" #object_type ").  Invalid argument"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* MANAGER_END_CACHE(object_type) */

#define DECLARE_GROUP_MANAGER_COPY_WITHOUT_IDENTIFIER( object_type ) \
PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(GROUP(object_type),name) \
{ \
	int return_code; \
\
	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(GROUP(object_type),name)); \
	/* check arguments */ \
	if (source&&destination) \
	{ \
		if (DESTROY_LIST(object_type)(&(destination->object_list))) \
		{ \
			if (destination->object_list=CREATE_LIST(object_type)()) \
			{ \
				return_code=COPY_LIST(object_type)(destination->object_list, \
					source->object_list); \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE,"MANAGER_COPY_WITHOUT_IDENTIFIER(" \
				#object_type ",name).  Could not create new destination object list"); \
				return_code=0; \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE,"MANAGER_COPY_WITHOUT_IDENTIFIER(" \
				#object_type ",name).  Could not destroy destination object list"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"MANAGER_COPY_WITHOUT_IDENTIFIER(" \
			#object_type ",name).  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(GROUP(object_type),name) */

#define DECLARE_GROUP_MANAGER_COPY_IDENTIFIER( object_type ) \
PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(GROUP(object_type),name,char *) \
{ \
	int return_code; \
\
	ENTER(MANAGER_COPY_IDENTIFIER(GROUP(object_type),name)); \
	/* check arguments */ \
	if (destination) \
	{ \
		DEALLOCATE(destination->name); \
		if (name) \
		{ \
			if (ALLOCATE(destination->name,char,strlen(name)+1)) \
			{ \
				strcpy(destination->name,name); \
				return_code=1; \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE,"MANAGER_COPY_IDENTIFIER(" \
					#object_type ",name).  Insufficient memory for name"); \
				return_code=0; \
			} \
		} \
		else \
		{ \
			return_code=1; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"MANAGER_COPY_IDENTIFIER(" \
			#object_type ",name).  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* MANAGER_COPY_IDENTIFIER(GROUP(object_type),name) */

#define DECLARE_GROUP_MANAGER_COPY_WITH_IDENTIFIER( object_type ) \
PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(GROUP(object_type),name) \
{ \
	int return_code; \
\
	ENTER(MANAGER_COPY_WITH_IDENTIFIER(GROUP(object_type),name)); \
	if (source&&destination) \
	{ \
		if (return_code=MANAGER_COPY_WITHOUT_IDENTIFIER(GROUP(object_type),name)( \
			destination,source)) \
		{ \
			return_code=MANAGER_COPY_IDENTIFIER(GROUP(object_type),name)( \
				destination,source->name); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"MANAGER_COPY_WITH_IDENTIFIER(" \
			#object_type ",name).  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* MANAGER_COPY_WITH_IDENTIFIER(GROUP(object_type),name) */

#define DECLARE_LOCAL_MANAGER_FUNCTIONS(object_type) \
DECLARE_MANAGER_UPDATE_FUNCTION(object_type) \
DECLARE_MANAGER_FIND_CLIENT_FUNCTION(object_type) \
DECLARE_MANAGED_OBJECT_NOT_IN_USE_CONDITIONAL_FUNCTION(object_type)

#define DECLARE_MANAGER_FUNCTIONS( object_type ) \
DECLARE_CREATE_MANAGER_FUNCTION(object_type) \
DECLARE_DESTROY_MANAGER_FUNCTION(object_type) \
DECLARE_REMOVE_OBJECT_FROM_MANAGER_FUNCTION(object_type) \
DECLARE_REMOVE_ALL_OBJECTS_FROM_MANAGER_FUNCTION(object_type) \
DECLARE_NUMBER_IN_MANAGER_FUNCTION(object_type) \
DECLARE_MANAGER_REGISTER_FUNCTION(object_type) \
DECLARE_MANAGER_DEREGISTER_FUNCTION(object_type) \
DECLARE_IS_MANAGED_FUNCTION(object_type) \
DECLARE_FIRST_OBJECT_IN_MANAGER_THAT_FUNCTION(object_type) \
DECLARE_FOR_EACH_OBJECT_IN_MANAGER_FUNCTION(object_type) \
DECLARE_MANAGER_BEGIN_CHANGE_FUNCTION(object_type) \
DECLARE_MANAGER_END_CHANGE_FUNCTION(object_type) \
DECLARE_MANAGER_BEGIN_CACHE_FUNCTION(object_type) \
DECLARE_MANAGER_END_CACHE_FUNCTION(object_type)

#define DECLARE_MANAGER_IDENTIFIER_FUNCTIONS( object_type , \
	identifier_field_name , identifier_type ) \
DECLARE_ADD_OBJECT_TO_MANAGER_FUNCTION(object_type,identifier_field_name) \
DECLARE_MANAGER_MODIFY_FUNCTION(object_type,identifier_field_name) \
DECLARE_MANAGER_MODIFY_NOT_IDENTIFIER_FUNCTION(object_type, \
	identifier_field_name) \
DECLARE_MANAGER_MODIFY_IDENTIFIER_FUNCTION(object_type,identifier_field_name, \
	identifier_type) \
DECLARE_FIND_BY_IDENTIFIER_IN_MANAGER_FUNCTION(object_type, \
	identifier_field_name,identifier_type)

/* special set of functions for object containing pointer to manager */
#define DECLARE_OBJECT_WITH_MANAGER_MANAGER_FUNCTIONS( object_type , object_manager ) \
DECLARE_CREATE_MANAGER_FUNCTION(object_type) \
DECLARE_OBJECT_WITH_MANAGER_REMOVE_MANAGER_FUNCTION(object_type,object_manager) \
DECLARE_DESTROY_OBJECT_WITH_MANAGER_MANAGER_FUNCTION(object_type,object_manager) \
DECLARE_REMOVE_OBJECT_WITH_MANAGER_FROM_MANAGER_FUNCTION(object_type,object_manager) \
DECLARE_REMOVE_ALL_OBJECTS_WITH_MANAGER_FROM_MANAGER_FUNCTION(object_type,object_manager) \
DECLARE_NUMBER_IN_MANAGER_FUNCTION(object_type) \
DECLARE_MANAGER_REGISTER_FUNCTION(object_type) \
DECLARE_MANAGER_DEREGISTER_FUNCTION(object_type) \
DECLARE_IS_MANAGED_FUNCTION(object_type) \
DECLARE_FIRST_OBJECT_IN_MANAGER_THAT_FUNCTION(object_type) \
DECLARE_FOR_EACH_OBJECT_IN_MANAGER_FUNCTION(object_type) \
DECLARE_MANAGER_BEGIN_CHANGE_FUNCTION(object_type) \
DECLARE_MANAGER_END_CHANGE_FUNCTION(object_type) \
DECLARE_MANAGER_BEGIN_CACHE_FUNCTION(object_type) \
DECLARE_MANAGER_END_CACHE_FUNCTION(object_type)

#define DECLARE_OBJECT_WITH_MANAGER_MANAGER_IDENTIFIER_FUNCTIONS( \
	object_type , identifier_field_name , identifier_type , object_manager ) \
DECLARE_ADD_OBJECT_WITH_MANAGER_TO_MANAGER_FUNCTION(object_type, \
	identifier_field_name, object_manager) \
DECLARE_MANAGER_MODIFY_FUNCTION(object_type,identifier_field_name) \
DECLARE_MANAGER_MODIFY_NOT_IDENTIFIER_FUNCTION(object_type, \
	identifier_field_name) \
DECLARE_MANAGER_MODIFY_IDENTIFIER_FUNCTION(object_type,identifier_field_name, \
	identifier_type) \
DECLARE_FIND_BY_IDENTIFIER_IN_MANAGER_FUNCTION(object_type, \
	identifier_field_name,identifier_type)

#define DECLARE_GROUP_MANAGER_COPY_FUNCTIONS( object_type ) \
DECLARE_GROUP_MANAGER_COPY_WITHOUT_IDENTIFIER(object_type) \
DECLARE_GROUP_MANAGER_COPY_IDENTIFIER(object_type) \
DECLARE_GROUP_MANAGER_COPY_WITH_IDENTIFIER(object_type)

#endif /* !defined (MANAGER_PRIVATE_H) */
