/*******************************************************************************
FILE : manager_private.h

LAST MODIFIED : 1 December 1999

DESCRIPTION :
Managers oversee the creation, deletion and modification of global objects -
such as object_types, lights and cameras.  All registered clients of the manager
are sent a message when an object has changed.

This file contains the details of the internal workings of the manager and
should be included in the module that declares the manager for a particular
object type.  This allows changes to the internal structure to be made without
affecting other parts of the program.
???DB.  Have a create_Manager ?
???DB.  Remove Manager_type and have a separate Manager_message and
	Manager_callback_procedure for each object ?
???DB.  Name not general enough ?
???DB.  Use B-trees instead of lists ?
???DB.  Message for destroy ?
???DB.  Have to change to a List type rather than a List_item type.
???DB.  Should there be a concept of order (get_first etc) ?
???DB.  Include manager in message ?
???DB.  Should there be another field, like access_count, that makes it so that
	only the manager can delete/modify ?  (locked ?)
???GH.  The MANAGER_COPY_NOT_IDENTIFIER functions should be put in object.h,
	and the 'MANAGER' bit removed from the name.
???GH.  The specialised group functions in this file should be in group.h
???DB.  Problems with DELETEing objects.  Need to be able to take account of
	objects that can cope with the object being destroyed (don't ACCESS) but need
	to know before the object is destroyed
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
#if defined (FULL_NAMES)
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
LAST MODIFIED : 27 September 1995 \
\
DESCRIPTION : \
The structure for the manager. \
============================================================================*/ \
{ \
	struct LIST(object_type) *object_list; \
	struct MANAGER_CALLBACK_ITEM(object_type) *callback_list; \
	int locked; \
	int cache; \
} /* struct MANAGER(object_type) */

/*
Local functions
---------------
*/
#if defined (FULL_NAMES)
#define MANAGER_UPDATE( object_type )  manager_update_ ## object_type
#else
#define MANAGER_UPDATE( object_type )  um ## object_type
#endif

#define DECLARE_MANAGER_UPDATE_FUNCTION( object_type ) \
static void MANAGER_UPDATE(object_type)( \
	struct MANAGER_MESSAGE(object_type) *message, \
	struct MANAGER(object_type) *manager) \
/***************************************************************************** \
LAST MODIFIED : 27 September 1995 \
\
DESCRIPTION : \
Calls all the callbacks registered with the <manager> \
============================================================================*/ \
{ \
	struct MANAGER_CALLBACK_ITEM(object_type) *item; \
\
	ENTER(MANAGER_UPDATE(object_type)); \
	/* check the arguments */ \
	if (manager&&message) \
	{ \
		item=manager->callback_list; \
		while (item) \
		{ \
			(item->callback)(message,item->user_data); \
			item=item->next; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"MANAGER_UPDATE(" #object_type ").  Invalid argument(s)"); \
	} \
	LEAVE; \
} /* MANAGER_UPDATE(object_type) */

#if defined (FULL_NAMES)
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
	/* check the arguments */ \
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

#if defined (FULL_NAMES)
#define MANAGER_NOT_IN_USE_CONDITIONAL( object_type ) \
	manager_not_in_use_conditional ## object_type
#else
#define MANAGER_NOT_IN_USE_CONDITIONAL( object_type )  gm ## object_type
#endif

#define DECLARE_MANAGER_NOT_IN_USE_CONDITIONAL( object_type ) \
int MANAGER_NOT_IN_USE_CONDITIONAL(object_type)(struct object_type *object, \
	void *dummy_user_data) \
/***************************************************************************** \
LAST MODIFIED : 27 May 1999 \
\
DESCRIPTION : \
Returns true if the access count for the <object> is 1 (not in use) \
============================================================================*/ \
{ \
	int return_code; \
\
	ENTER(MANAGER_NOT_IN_USE_CONDITIONAL(object_type)); \
	/* check the arguments */ \
	if (object&&!dummy_user_data) \
	{ \
		if (1==object->access_count) \
		{ \
			return_code=1; \
		} \
		else \
		{ \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"MANAGER_NOT_IN_USE_CONDITIONAL(" #object_type ").  Missing object"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* MANAGER_NOT_IN_USE_CONDITIONAL(object_type) */

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
	if (ALLOCATE(manager,struct MANAGER(object_type),1)) \
	{ \
		if (manager->object_list=CREATE_LIST(object_type)()) \
		{ \
			manager->callback_list= \
				(struct MANAGER_CALLBACK_ITEM(object_type) *)NULL; \
			manager->locked=0; \
			manager->cache=0; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"MANAGER_CREATE(" #object_type ").  Could not create object list"); \
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

#define DECLARE_DESTROY_MANAGER_FUNCTION( object_type ) \
PROTOTYPE_DESTROY_MANAGER_FUNCTION(object_type) \
{ \
	int return_code; \
	struct MANAGER(object_type) *manager; \
	struct MANAGER_CALLBACK_ITEM(object_type) *current,*next; \
\
	ENTER(DESTROY_MANAGER(object_type)); \
	if (manager_address&&(manager= *manager_address)) \
	{ \
		/* destroy the object list */ \
		DESTROY_LIST(object_type)(&(manager->object_list)); \
		/* destroy the callback list */ \
		current=manager->callback_list; \
		while (current) \
		{ \
			next=current->next; \
			DEALLOCATE(current); \
			current=next; \
		} \
		return_code=1; \
	} \
	else \
	{ \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* DESTROY_MANAGER(object_type) */

#if defined (OLD_CODE)
/*???DB.  Doesn't work because object no longer exists when the
	MANAGER_CHANGE_DELETE message is sent */
/*???DB.  Brought on because I want objects such as GT_element_groups to be
	interested, but not to access so that the managed object can be destroyed
	and the interested object recovers */
#define DECLARE_REMOVE_OBJECT_FROM_MANAGER_FUNCTION( object_type ) \
PROTOTYPE_REMOVE_OBJECT_FROM_MANAGER_FUNCTION(object_type) \
{ \
	int return_code; \
	struct MANAGER_MESSAGE(object_type) *message; \
\
	ENTER(REMOVE_OBJECT_FROM_MANAGER(object_type)); \
	/* check the arguments */ \
	if (manager&&object) \
	{ \
		if (!(manager->locked)) \
		{ \
			if (IS_OBJECT_IN_LIST(object_type)(object,manager->object_list)) \
			{ \
				if (1==object->access_count) \
				{ \
					REMOVE_OBJECT_FROM_LIST(object_type)(object,manager->object_list); \
					if (!(manager->cache)) \
					{ \
						if (ALLOCATE(message,struct MANAGER_MESSAGE(object_type),1)) \
						{ \
							message->change=MANAGER_CHANGE_DELETE(object_type); \
							message->object_changed=object; \
							MANAGER_UPDATE(object_type)(message,manager); \
							DEALLOCATE(message); \
						} \
						else  \
						{ \
							display_message(ERROR_MESSAGE, \
"REMOVE_OBJECT_FROM_MANAGER(" #object_type ").  Could not allocate message"); \
						} \
					} \
					return_code=1; \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
					"REMOVE_OBJECT_FROM_MANAGER(" #object_type ").  Object is in use"); \
					return_code=0; \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
			"REMOVE_OBJECT_FROM_MANAGER(" #object_type ").  Object is not managed"); \
				return_code=0; \
			} \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"REMOVE_OBJECT_FROM_MANAGER(" #object_type ").  Manager locked"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"REMOVE_OBJECT_FROM_MANAGER(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* REMOVE_OBJECT_FROM_MANAGER(object_type) */

#define DECLARE_REMOVE_ALL_OBJECTS_FROM_MANAGER_FUNCTION( object_type ) \
PROTOTYPE_REMOVE_ALL_OBJECTS_FROM_MANAGER_FUNCTION(object_type) \
{ \
	int return_code; \
	struct MANAGER_MESSAGE(object_type) *message; \
	struct object_type *object; \
\
	ENTER(REMOVE_ALL_OBJECTS_FROM_MANAGER(object_type)); \
	/* check the arguments */ \
	if (manager) \
	{ \
		if (!(manager->locked)) \
		{ \
			while (object=FIRST_OBJECT_IN_LIST_THAT(object_type)( \
				MANAGER_NOT_IN_USE_CONDITIONAL(object_type),(void *)NULL, \
				manager->object_list)) \
			{ \
				REMOVE_OBJECT_FROM_LIST(object_type)(object,manager->object_list); \
				if (!(manager->cache)) \
				{ \
					if (ALLOCATE(message,struct MANAGER_MESSAGE(object_type),1)) \
					{ \
						message->change=MANAGER_CHANGE_DELETE(object_type); \
						message->object_changed=object; \
						MANAGER_UPDATE(object_type)(message,manager); \
						DEALLOCATE(message); \
					} \
					else  \
					{ \
						display_message(ERROR_MESSAGE, \
							"REMOVE_ALL_OBJECTS_FROM_MANAGER(" #object_type \
							").  Could not allocate message"); \
					} \
				} \
				return_code=1; \
			} \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"REMOVE_ALL_OBJECTS_FROM_MANAGER(" #object_type ").  Manager locked"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"REMOVE_ALL_OBJECTS_FROM_MANAGER(" #object_type \
			").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* REMOVE_OBJECT_FROM_MANAGER(object_type) */
#endif /* defined (OLD_CODE) */

/*???RC Must remove from list before sending manager delete message. */
/* But! That would entail passing around the address of a freed object, which */
/* is a no-no. Solution: ACCESS and DEACCESS the deleted object locally so */
/* that it is not freed until after the message. */
#define DECLARE_REMOVE_OBJECT_FROM_MANAGER_FUNCTION( object_type ) \
PROTOTYPE_REMOVE_OBJECT_FROM_MANAGER_FUNCTION(object_type) \
{ \
	int return_code; \
	struct object_type *temp_object; \
	struct MANAGER_MESSAGE(object_type) *message; \
\
	ENTER(REMOVE_OBJECT_FROM_MANAGER(object_type)); \
	/* check the arguments */ \
	if (manager&&object) \
	{ \
		if (!(manager->locked)) \
		{ \
			if (IS_OBJECT_IN_LIST(object_type)(object,manager->object_list)) \
			{ \
				if (1==object->access_count) \
				{ \
					temp_object=ACCESS(object_type)(object); \
					if (ALLOCATE(message,struct MANAGER_MESSAGE(object_type),1)) \
					{ \
						if (return_code=REMOVE_OBJECT_FROM_LIST(object_type)(object, \
							manager->object_list)) \
						{ \
							message->change=MANAGER_CHANGE_DELETE(object_type); \
							message->object_changed=object; \
							MANAGER_UPDATE(object_type)(message,manager); \
						} \
						DEALLOCATE(message); \
					} \
					else \
					{ \
						display_message(ERROR_MESSAGE, \
"REMOVE_OBJECT_FROM_MANAGER(" #object_type ").  Could not allocate message"); \
						return_code=0; \
					} \
					DEACCESS(object_type)(&temp_object); \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
					"REMOVE_OBJECT_FROM_MANAGER(" #object_type ").  Object is in use"); \
					return_code=0; \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
			"REMOVE_OBJECT_FROM_MANAGER(" #object_type ").  Object is not managed"); \
				return_code=0; \
			} \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"REMOVE_OBJECT_FROM_MANAGER(" #object_type ").  Manager locked"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"REMOVE_OBJECT_FROM_MANAGER(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* REMOVE_OBJECT_FROM_MANAGER(object_type) */

/*???RC comments as above REMOVE_OBJECT_FROM_MANAGER */
#define DECLARE_REMOVE_ALL_OBJECTS_FROM_MANAGER_FUNCTION( object_type ) \
PROTOTYPE_REMOVE_ALL_OBJECTS_FROM_MANAGER_FUNCTION(object_type) \
{ \
	int return_code; \
	struct MANAGER_MESSAGE(object_type) *message; \
	struct object_type *object,*temp_object; \
\
	ENTER(REMOVE_ALL_OBJECTS_FROM_MANAGER(object_type)); \
	/* check the arguments */ \
	if (manager) \
	{ \
		if (!(manager->locked)) \
		{ \
			while (object=FIRST_OBJECT_IN_LIST_THAT(object_type)( \
				MANAGER_NOT_IN_USE_CONDITIONAL(object_type),(void *)NULL, \
				manager->object_list)) \
			{ \
				temp_object=ACCESS(object_type)(object); \
				if (ALLOCATE(message,struct MANAGER_MESSAGE(object_type),1)) \
				{ \
					if (return_code=REMOVE_OBJECT_FROM_LIST(object_type)(object, \
						manager->object_list)) \
					{ \
						message->change=MANAGER_CHANGE_DELETE(object_type); \
						message->object_changed=object; \
						MANAGER_UPDATE(object_type)(message,manager); \
					} \
					DEALLOCATE(message); \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
"REMOVE_OBJECT_FROM_MANAGER(" #object_type ").  Could not allocate message"); \
					return_code=0; \
				} \
				DEACCESS(object_type)(&temp_object); \
			} \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"REMOVE_ALL_OBJECTS_FROM_MANAGER(" #object_type ").  Manager locked"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"REMOVE_ALL_OBJECTS_FROM_MANAGER(" #object_type \
			").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* REMOVE_ALL_OBJECTS_FROM_MANAGER(object_type) */

#define DECLARE_ADD_OBJECT_TO_MANAGER_FUNCTION( object_type , \
	identifier_field_name ) \
PROTOTYPE_ADD_OBJECT_TO_MANAGER_FUNCTION(object_type) \
{ \
	int return_code; \
	struct object_type *temp_object_ptr; \
	struct MANAGER_MESSAGE(object_type) *message; \
\
	ENTER(ADD_OBJECT_TO_MANAGER(object_type)); \
	/* check the arguments */ \
	if (manager&&object) \
	{ \
		if (!(manager->locked)) \
		{ \
			/* can only add if new identifier not already used by managed object */ \
			temp_object_ptr=FIND_BY_IDENTIFIER_IN_LIST(object_type, \
				identifier_field_name)(object->identifier_field_name, \
				manager->object_list); \
			if (!temp_object_ptr) \
			{ \
				/* will access the object */ \
				if (ADD_OBJECT_TO_LIST(object_type)(object,manager->object_list)) \
				{ \
					if (!(manager->cache)) \
					{ \
						if (ALLOCATE(message,struct MANAGER_MESSAGE(object_type),1)) \
						{ \
							message->change=MANAGER_CHANGE_ADD(object_type); \
							message->object_changed=object; \
							MANAGER_UPDATE(object_type)(message,manager); \
							DEALLOCATE(message); \
						} \
						else \
						{ \
							display_message(ERROR_MESSAGE, \
								"ADD_OBJECT_TO_MANAGER(" #object_type \
								").  Could not allocate message"); \
						} \
					} \
					return_code=1; \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
						"ADD_OBJECT_TO_MANAGER(" #object_type \
						").  Could not add object to list"); \
					return_code=0; \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"ADD_OBJECT_TO_MANAGER(" #object_type \
					").  Could not add object; identifier already in use"); \
				return_code=0; \
			} \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"ADD_OBJECT_TO_MANAGER(" #object_type ").  Manager locked"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"ADD_OBJECT_TO_MANAGER(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
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
	struct object_type *temp_object_ptr; \
	struct MANAGER_MESSAGE(object_type) *message; \
\
	ENTER(ADD_OBJECT_TO_MANAGER(object_type)); \
	/* check the arguments */ \
	if (manager&&object) \
	{ \
		if (!object->object_manager) \
		{ \
			if (!(manager->locked)) \
			{ \
				/* can only add if new identifier not used by managed object */ \
				temp_object_ptr=FIND_BY_IDENTIFIER_IN_LIST(object_type, \
					identifier_field_name)(object->identifier_field_name, \
					manager->object_list); \
				if (!temp_object_ptr) \
				{ \
					/* will access the object */ \
					if (ADD_OBJECT_TO_LIST(object_type)(object,manager->object_list)) \
					{ \
						if (!(manager->cache)) \
						{ \
							if (ALLOCATE(message,struct MANAGER_MESSAGE(object_type),1)) \
							{ \
								message->change=MANAGER_CHANGE_ADD(object_type); \
								message->object_changed=object; \
								MANAGER_UPDATE(object_type)(message,manager); \
								DEALLOCATE(message); \
							} \
							else \
							{ \
								display_message(ERROR_MESSAGE, \
									"ADD_OBJECT_TO_MANAGER(" #object_type \
									").  Could not allocate message"); \
							} \
						} \
						/* object keeps a pointer to its manager */ \
						object->object_manager=manager; \
						return_code=1; \
					} \
					else \
					{ \
						display_message(ERROR_MESSAGE, \
							"ADD_OBJECT_TO_MANAGER(" #object_type \
							").  Could not add object to list"); \
						return_code=0; \
					} \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
						"ADD_OBJECT_TO_MANAGER(" #object_type \
						").  Could not add object; identifier already in use"); \
					return_code=0; \
				} \
			} \
			else \
			{ \
				display_message(WARNING_MESSAGE, \
					"ADD_OBJECT_TO_MANAGER(" #object_type ").  Manager locked"); \
				return_code=0; \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"ADD_OBJECT_TO_MANAGER(" #object_type ").  Object already managed"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"ADD_OBJECT_TO_MANAGER(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* ADD_OBJECT_TO_MANAGER(object_type) */

#define DECLARE_NUMBER_IN_MANAGER_FUNCTION( object_type ) \
PROTOTYPE_NUMBER_IN_MANAGER_FUNCTION(object_type) \
{ \
	int return_code; \
\
	ENTER(NUMBER_IN_MANAGER(object_type)); \
	/* check the arguments */ \
	if (manager) \
	{ \
		if (!(manager->locked)) \
		{ \
			return_code=NUMBER_IN_LIST(object_type)(manager->object_list); \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"NUMBER_IN_MANAGER(" #object_type ").  Manager is locked."); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"NUMBER_IN_MANAGER(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* NUMBER_IN_MANAGER(object_type) */

#define DECLARE_MANAGER_MODIFY_FUNCTION( object_type , identifier_field_name ) \
PROTOTYPE_MANAGER_MODIFY_FUNCTION(object_type) \
{ \
	int return_code; \
	struct object_type *tmp_object; \
	struct MANAGER_MESSAGE(object_type) *message; \
\
	ENTER(MANAGER_MODIFY(object_type)); \
	/* check the arguments */ \
	if (manager&&object&&new_data) \
	{ \
		if (!(manager->locked)) \
		{ \
			if (IS_OBJECT_IN_LIST(object_type)(object,manager->object_list)) \
			{ \
				tmp_object= \
					FIND_BY_IDENTIFIER_IN_LIST(object_type,identifier_field_name)( \
					new_data->identifier_field_name,manager->object_list); \
				if ((!tmp_object)||(object==tmp_object)) \
				{ \
					/* remove and re-add object to list so ordering is correct */ \
					/* must use ACCESS/DEACCESS so that object is not DESTROYed! */ \
					/* Note: DEACCESS clears pointer so keep tmp_object as copy */ \
					tmp_object=ACCESS(object_type)(object); \
					if (REMOVE_OBJECT_FROM_LIST(object_type)(object, \
						manager->object_list)) \
					{ \
						if (MANAGER_COPY_WITH_IDENTIFIER(object_type,\
							identifier_field_name)(object,new_data)) \
						{ \
							if (ADD_OBJECT_TO_LIST(object_type)(object, \
								manager->object_list)) \
							{ \
								if (!(manager->cache)) \
								{ \
									if (ALLOCATE(message,struct MANAGER_MESSAGE(object_type),1)) \
									{ \
										message->change=MANAGER_CHANGE_OBJECT(object_type); \
										message->object_changed=object; \
										MANAGER_UPDATE(object_type)(message,manager); \
										DEALLOCATE(message); \
									} \
									else \
									{ \
										display_message(ERROR_MESSAGE, \
						"MANAGER_MODIFY(" #object_type ").  Could not allocate message"); \
									} \
								} \
								return_code=1; \
							} \
							else  \
							{ \
								display_message(ERROR_MESSAGE, \
									"MANAGER_MODIFY(" #object_type ").  Could not add object"); \
								return_code=0; \
							} \
						} \
						else  \
						{ \
							ADD_OBJECT_TO_LIST(object_type)(object,manager->object_list); \
							display_message(ERROR_MESSAGE, \
								"MANAGER_MODIFY(" #object_type ").  Could not copy object"); \
							return_code=0; \
						} \
					} \
					else  \
					{ \
						display_message(ERROR_MESSAGE, \
							"MANAGER_MODIFY(" #object_type ").  Could not remove object"); \
						return_code=0; \
					} \
					DEACCESS(object_type)(&tmp_object); \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
"MANAGER_MODIFY(" #object_type ").  Object with specified identifier already exists."); \
					return_code=0; \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"MANAGER_MODIFY(" #object_type ").  Object is not managed"); \
				return_code=0; \
			} \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"MANAGER_MODIFY(" #object_type ").  Manager locked"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"MANAGER_MODIFY(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* MANAGER_MODIFY(object_type) */

#define DECLARE_MANAGER_MODIFY_NOT_IDENTIFIER_FUNCTION( object_type , \
	identifier_field_name ) \
PROTOTYPE_MANAGER_MODIFY_NOT_IDENTIFIER_FUNCTION(object_type, \
	identifier_field_name) \
{ \
	int return_code; \
	struct MANAGER_MESSAGE(object_type) *message; \
\
	ENTER(MANAGER_MODIFY_NOT_IDENTIFIER(object_type,identifier_field_name)); \
	/* check the arguments */ \
	if (manager&&object&&new_data) \
	{ \
		if (!(manager->locked)) \
		{ \
			if (IS_OBJECT_IN_LIST(object_type)(object,manager->object_list)) \
			{ \
				if (MANAGER_COPY_WITHOUT_IDENTIFIER(object_type, \
					identifier_field_name)(object,new_data)) \
				{ \
					if (!(manager->cache)) \
					{ \
						if (ALLOCATE(message,struct MANAGER_MESSAGE(object_type),1)) \
						{ \
							message->change= \
								MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(object_type); \
							message->object_changed=object; \
							MANAGER_UPDATE(object_type)(message,manager); \
							DEALLOCATE(message); \
						} \
						else  \
						{ \
							display_message(ERROR_MESSAGE, \
"MANAGER_MODIFY_NOT_IDENTIFIER(" #object_type "," #identifier_field_name ").  Could not allocate message"); \
						} \
					} \
					return_code=1; \
				} \
				else  \
				{ \
					display_message(ERROR_MESSAGE, \
"MANAGER_MODIFY_NOT_IDENTIFIER(" #object_type "," #identifier_field_name ").  Could not copy object"); \
					return_code=0; \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
"MANAGER_MODIFY_NOT_IDENTIFIER(" #object_type "," #identifier_field_name ").  Object is not managed"); \
				return_code=0; \
			} \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
"MANAGER_MODIFY_NOT_IDENTIFIER(" #object_type "," #identifier_field_name ").  Manager is locked"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
"MANAGER_MODIFY_NOT_IDENTIFIER(" #object_type "," #identifier_field_name ").  Invalid argument(s)"); \
		return_code=0; \
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
	struct MANAGER_MESSAGE(object_type) *message; \
	struct object_type *tmp_object; \
\
	ENTER(MANAGER_MODIFY_IDENTIFIER(object_type,identifier_field_name)); \
	/* check the arguments */ \
	if (manager&&object&&new_identifier) \
	{ \
		if (!(manager->locked)) \
		{ \
			if (IS_OBJECT_IN_LIST(object_type)(object,manager->object_list)) \
			{ \
				tmp_object= \
					FIND_BY_IDENTIFIER_IN_LIST(object_type,identifier_field_name)( \
					new_identifier,manager->object_list); \
				if ((!tmp_object)||(object==tmp_object)) \
				{ \
					/* remove and re-add object to list so ordering is correct */ \
					/* must use ACCESS/DEACCESS so that object is not DESTROYed! */ \
					/* Note: DEACCESS clears pointer so keep tmp_object as copy */ \
					tmp_object=ACCESS(object_type)(object); \
					if (REMOVE_OBJECT_FROM_LIST(object_type)(object, \
						manager->object_list)) \
					{ \
						if (MANAGER_COPY_IDENTIFIER(object_type,identifier_field_name)( \
							object,new_identifier)) \
						{ \
							if (ADD_OBJECT_TO_LIST(object_type)(object, \
								manager->object_list)) \
							{ \
								if (!(manager->cache)) \
								{ \
									if (ALLOCATE(message,struct MANAGER_MESSAGE(object_type),1)) \
									{ \
										message->change=MANAGER_CHANGE_IDENTIFIER(object_type); \
										message->object_changed=object; \
										MANAGER_UPDATE(object_type)(message,manager); \
										DEALLOCATE(message); \
									} \
									else  \
									{ \
										display_message(ERROR_MESSAGE, \
"MANAGER_MODIFY_IDENTIFIER(" #object_type "," #identifier_field_name ").  Could not allocate message"); \
									} \
								} \
								return_code=1; \
							} \
							else  \
							{ \
								display_message(ERROR_MESSAGE, \
"MANAGER_MODIFY_IDENTIFIER(" #object_type "," #identifier_field_name ").  Could not add object"); \
								return_code=0; \
							} \
						} \
						else  \
						{ \
							ADD_OBJECT_TO_LIST(object_type)(object,manager->object_list); \
							display_message(ERROR_MESSAGE, \
"MANAGER_MODIFY_IDENTIFIER(" #object_type "," #identifier_field_name ").  Could not copy identifier"); \
							return_code=0; \
						} \
					} \
					else  \
					{ \
						display_message(ERROR_MESSAGE, \
"MANAGER_MODIFY_IDENTIFIER(" #object_type "," #identifier_field_name ").  Could not remove object"); \
						return_code=0; \
					} \
					DEACCESS(object_type)(&tmp_object); \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
"MANAGER_MODIFY_IDENTIFIER(" #object_type "," #identifier_field_name ").  Object with new identifier already exists."); \
					return_code=0; \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
"MANAGER_MODIFY_IDENTIFIER(" #object_type "," #identifier_field_name ").  Object is not managed"); \
				return_code=0; \
			} \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
"MANAGER_MODIFY_IDENTIFIER(" #object_type "," #identifier_field_name ").  Manager is locked."); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
"MANAGER_MODIFY_IDENTIFIER(" #object_type "," #identifier_field_name ").  Invalid argument(s)"); \
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
	/* check the arguments */ \
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
	/* check the arguments */ \
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
	/* check the arguments */ \
	if (manager&&object) \
	{ \
		if (!(manager->locked)) \
		{ \
			return_code=IS_OBJECT_IN_LIST(object_type)(object,manager->object_list); \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"IS_MANAGED(" #object_type ").  Manager is locked."); \
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

#define DECLARE_FIND_BY_IDENTIFIER_IN_MANAGER_FUNCTION( object_type , \
	identifier , identifier_type ) \
PROTOTYPE_FIND_BY_IDENTIFIER_IN_MANAGER_FUNCTION(object_type,identifier, \
	identifier_type) \
{ \
	struct object_type *object; \
\
	ENTER(FIND_BY_IDENTIFIER_IN_MANAGER(object_type,identifier)); \
	/* check the arguments */ \
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
"FIND_BY_IDENTIFIER_IN_LIST(" #object_type "," #identifier ").  Manager is locked."); \
			object=(struct object_type *)NULL; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
"FIND_BY_IDENTIFIER_IN_LIST(" #object_type "," #identifier ").  Invalid argument(s)"); \
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
	/* check the arguments */ \
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
	/* check the arguments */ \
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

#define DECLARE_MANAGER_BEGIN_CACHE_FUNCTION( object_type ) \
PROTOTYPE_MANAGER_BEGIN_CACHE_FUNCTION(object_type) \
{ \
	int return_code; \
\
	ENTER(MANAGER_BEGIN_CACHE(object_type)); \
	/* check the arguments */ \
	if (manager) \
	{ \
		if (manager->cache) \
		{ \
			display_message(ERROR_MESSAGE, \
				"MANAGER_BEGIN_CACHE(" #object_type ").  Caching already enabled."); \
			return_code=0; \
		} \
		else \
		{ \
			manager->cache=1; \
			return_code=1; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"MANAGER_BEGIN_CACHE(" #object_type ").  Invalid argument"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* MANAGER_BEGIN_CACHE(object_type) */

#define DECLARE_MANAGER_END_CACHE_FUNCTION( object_type ) \
PROTOTYPE_MANAGER_END_CACHE_FUNCTION(object_type) \
{ \
	int return_code; \
	struct MANAGER_MESSAGE(object_type) *message; \
\
	ENTER(MANAGER_END_CACHE(object_type)); \
	/* check the arguments */ \
	if (manager) \
	{ \
		if (manager->cache) \
		{ \
			if (ALLOCATE(message,struct MANAGER_MESSAGE(object_type),1)) \
			{ \
				message->change=MANAGER_CHANGE_ALL(object_type); \
				message->object_changed=(struct object_type *)NULL; \
				MANAGER_UPDATE(object_type)(message,manager); \
				DEALLOCATE(message); \
				return_code=1; \
			} \
			else  \
			{ \
				display_message(ERROR_MESSAGE, \
					"MANAGER_END_CACHE(" #object_type ").  Could not allocate message"); \
				return_code=0; \
			} \
			manager->cache=0; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"MANAGER_END_CACHE(" #object_type ").  Caching not enabled"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"MANAGER_END_CACHE(" #object_type ").  Invalid argument"); \
		return_code=0; \
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
DECLARE_MANAGER_NOT_IN_USE_CONDITIONAL(object_type)

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
#endif
