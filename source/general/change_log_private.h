/*******************************************************************************
FILE : change_log_private.h

LAST MODIFIED : 15 April 2003

DESCRIPTION :
Structure for storing information about objects added, removed or modified
with and/or without identifier. Used for change messages throughout Cmgui.
Internally the objects are stored in a list indexed by the address of the
object, and containing an enumerator that describes the change. Objects in the
change_log are accessed so that removed objects exist until after clients
get to hear about their imminent demise. Change enumerator logic is slightly
involved -- refer to function CHANGE_LOG_ADD for details.
==============================================================================*/
#if !defined (CHANGE_LOG_PRIVATE_H)
#define CHANGE_LOG_PRIVATE_H

#include "general/change_log.h"
#include "general/compare.h"
#include "general/indexed_list_private.h"
#include "general/object.h"

/*
Macros
======
*/

/*
Local types
-----------
*/

#if defined (FULL_NAMES)
#define CHANGE_LOG_ENTRY( object_type ) change_log_entry_ ## object_type
#else
#define CHANGE_LOG_ENTRY( object_type ) cle_ ## object_type
#endif

#define FULL_DECLARE_CHANGE_LOG_ENTRY_TYPE( object_type ) \
struct CHANGE_LOG_ENTRY(object_type) \
/***************************************************************************** \
LAST MODIFIED : 9 December 2002 \
\
DESCRIPTION : \
Record of an object and what has changed about it, used internally by the \
change_log. \
============================================================================*/ \
{ \
	/* ACCESSed pointer to changed object */ \
	struct object_type *the_object; \
	/* what has changed about the object */ \
	enum CHANGE_LOG_CHANGE(object_type) change; \
	/* access_count for using list macros */ \
	int access_count; \
}; /* struct CHANGE_LOG_ENTRY(object_type) */ \
\
DECLARE_LIST_TYPES(CHANGE_LOG_ENTRY(object_type)); \
\
FULL_DECLARE_INDEXED_LIST_TYPE(CHANGE_LOG_ENTRY(object_type))

#define FULL_DECLARE_CHANGE_LOG_TYPE( object_type ) \
DECLARE_CHANGE_LOG_TYPE(object_type) \
/***************************************************************************** \
LAST MODIFIED : 4 February 2003 \
\
DESCRIPTION : \
The structure for the change_log. \
???RC could typedef the entry_list to be the change log structure, but may \
find more info is needed later so leave for now. \
============================================================================*/ \
{ \
	/* bitwise OR of all change enumerators in entry_list */ \
	enum CHANGE_LOG_CHANGE(object_type) change_summary; \
	/* all_change indicates that a large number of objects have changed and the \
		 log is no longer remembering individual changes; all objects are \
		 considered as having undergone change_summary except for remove, which \
		 can still be determined from the current object_list. all_change can be \
		 set by calling CHANGE_LOG_SET_CHANGE_ALL or by the log exceeding \
		 max_changes */ \
	int all_change; \
	/* number_of_changes is incremented with each logged change. When all_change \
		 is flagged, the number in the object_list is added to it */ \
	int number_of_changes; \
	/* number indicating the maximum length the entry_list may become before \
		 all_change is set. Negative numbers indicate there is no limit */ \
	int max_changes; \
	/* the list of changed objects with change descriptions */ \
	struct LIST(CHANGE_LOG_ENTRY(object_type)) *entry_list; \
	/* the list of global objects */ \
	struct LIST(object_type) *object_list; \
} /* struct CHANGE_LOG(object_type) */

/*
Module functions
----------------
*/

#define DESTROY_CHANGE_LOG_ENTRY( object_type ) \
	DESTROY(CHANGE_LOG_ENTRY(object_type))

#define DECLARE_DESTROY_CHANGE_LOG_ENTRY_FUNCTION( object_type ) \
static int DESTROY_CHANGE_LOG_ENTRY(object_type)( \
	struct CHANGE_LOG_ENTRY(object_type) **change_log_entry_address) \
/***************************************************************************** \
LAST MODIFIED : 9 December 2002 \
\
DESCRIPTION : \
Note the CREATE for CHANGE_LOG_ENTRY is in the ADD function. \
============================================================================*/ \
{ \
	int return_code; \
	struct CHANGE_LOG_ENTRY(object_type) *change_log_entry; \
\
	ENTER(DESTROY_CHANGE_LOG_ENTRY(object_type)); \
	if (change_log_entry_address && \
		(change_log_entry = *change_log_entry_address)) \
	{ \
		if (0 == change_log_entry->access_count) \
		{ \
			DEACCESS(object_type)(&(change_log_entry->the_object)); \
			DEALLOCATE(*change_log_entry_address); \
			return_code = 1; \
		} \
		else \
		{ \
			return_code = 0; \
		} \
		*change_log_entry_address = (struct CHANGE_LOG_ENTRY(object_type) *)NULL; \
	} \
	else \
	{ \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* DESTROY_CHANGE_LOG_ENTRY(object_type) */

#if defined (FULL_NAMES)
#define CHANGE_LOG_ENTRY_OBJECT_CHANGE( object_type ) \
	change_log_entry_object_change_ ## object_type
#else
#define CHANGE_LOG_ENTRY_OBJECT_CHANGE( object_type ) cleoc_ ## object_type
#endif

#define DECLARE_CHANGE_LOG_ENTRY_OBJECT_CHANGE_FUNCTION( object_type ) \
static int CHANGE_LOG_ENTRY_OBJECT_CHANGE(object_type)( \
	struct CHANGE_LOG_ENTRY(object_type) *entry, void *change_log_void) \
/***************************************************************************** \
LAST MODIFIED : 25 March 2003 \
\
DESCRIPTION : \
Calls CHANGE_LOG_OBJECT_CHANGE with <change_log> and the object in <entry>. \
Used for merging change_log entry_list. \
If <change_log> has a pointer to the object_list it is logging changes for, \
only merges changes for objects that are in the object_list. \
============================================================================*/ \
{ \
	int return_code; \
	struct CHANGE_LOG(object_type) *change_log; \
\
	ENTER(CHANGE_LOG_ENTRY_OBJECT_CHANGE(object_type)); \
	if (entry && \
		(change_log = (struct CHANGE_LOG(object_type) *)change_log_void)) \
	{ \
		if ((!(change_log->object_list)) || IS_OBJECT_IN_LIST(object_type)( \
			entry->the_object, change_log->object_list)) \
		{ \
			return_code = CHANGE_LOG_OBJECT_CHANGE(object_type)(change_log, \
				entry->the_object, entry->change); \
		} \
		else \
		{ \
			return_code = 1; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, "CHANGE_LOG_ENTRY_OBJECT_CHANGE(" \
			#object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHANGE_LOG_ENTRY_OBJECT_CHANGE(object_type) */

#if defined (OLD_CODE_TO_KEEP)

#if defined (FULL_NAMES)
#define CHANGE_LOG_CONDITIONAL_DATA( object_type ) change_log_conditional_data_ ## object_type
#else
#define CHANGE_LOG_CONDITIONAL_DATA( object_type ) clcd_ ## object_type
#endif

#if defined (FULL_NAMES)
#define CHANGE_LOG_CONDITIONAL( object_type ) change_log_conditional_ ## object_type
#else
#define CHANGE_LOG_CONDITIONAL( object_type ) clcn_ ## object_type
#endif

#define DEFINE_CHANGE_LOG_CONDITIONAL_DATA_AND_FUNCTION( object_type ) \
struct CHANGE_LOG_CONDITIONAL_DATA(object_type) \
{ \
	CHANGE_LOG_CONDITIONAL_FUNCTION(object_type) *conditional_function; \
	void *user_data; \
}; \
\
static int CHANGE_LOG_CONDITIONAL(object_type)( \
	struct CHANGE_LOG_ENTRY(object_type) *entry, void *data_void) \
/***************************************************************************** \
LAST MODIFIED : 9 December 2002 \
\
DESCRIPTION : \
Returns, for the object in <entry>, the return value from the change_log \
conditional_function with user_data from the <data>. \
Returns 1 if no conditional_function is supplied. \
<data_void> points at a struct CHANGE_LOG_CONDITIONAL_DATA(object_type). \
============================================================================*/ \
{ \
	int return_code; \
	struct CHANGE_LOG_CONDITIONAL_DATA(object_type) *data; \
\
	ENTER(CHANGE_LOG_CONDITIONAL(object_type)); \
	if (entry && \
		(data = (struct CHANGE_LOG_CONDITIONAL_DATA(object_type) *)data_void)) \
	{ \
		if (data->conditional_function) \
		{ \
			return_code = (data->conditional_function)(entry->the_object, \
				entry->change, data->user_data); \
		} \
		else \
		{ \
			return_code = 1; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHANGE_LOG_CONDITIONAL(" #object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHANGE_LOG_CONDITIONAL(object_type) */

#if defined (FULL_NAMES)
#define CHANGE_LOG_ITERATOR_DATA( object_type ) change_log_iterator_data_ ## object_type
#else
#define CHANGE_LOG_ITERATOR_DATA( object_type ) clid_ ## object_type
#endif

#if defined (FULL_NAMES)
#define CHANGE_LOG_ITERATOR( object_type ) change_log_iterator_ ## object_type
#else
#define CHANGE_LOG_ITERATOR( object_type ) clit_ ## object_type
#endif

#define DEFINE_CHANGE_LOG_ITERATOR_DATA_AND_FUNCTION( object_type ) \
struct CHANGE_LOG_ITERATOR_DATA(object_type) \
{ \
	CHANGE_LOG_ITERATOR_FUNCTION(object_type) *iterator_function; \
	void *user_data; \
}; \
\
static int CHANGE_LOG_ITERATOR(object_type)( \
	struct CHANGE_LOG_ENTRY(object_type) *entry, void *data_void) \
/***************************************************************************** \
LAST MODIFIED : 9 December 2002 \
\
DESCRIPTION : \
Calls, for the object in <entry>, the iterator_function with user_data from the \
<data>. Returns its return value to the caller of this function. \
<data_void> points at a struct CHANGE_LOG_ITERATOR_DATA(object_type). \
============================================================================*/ \
{ \
	int return_code; \
	struct CHANGE_LOG_ITERATOR_DATA(object_type) *data; \
\
	ENTER(CHANGE_LOG_ITERATOR(object_type)); \
	if (entry && \
		(data = (struct CHANGE_LOG_ITERATOR_DATA(object_type) *)data_void) && \
		data->iterator_function) \
	{ \
		return_code = (data->iterator_function)(entry->the_object, entry->change, \
			data->user_data); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHANGE_LOG_ITERATOR(" #object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHANGE_LOG_ITERATOR(object_type) */

#endif /* defined (OLD_CODE_TO_KEEP) */

/*
Global functions
----------------
*/

#define DECLARE_CREATE_CHANGE_LOG_FUNCTION( object_type ) \
PROTOTYPE_CREATE_CHANGE_LOG_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 17 February 2003 \
\
DESCRIPTION : \
Creates and returns a new change_log for recording changes to objects in the \
<object_list>. The change_log remembers whether objects were added, removed \
or otherwise modified. Since it is expensive to remember large lists of \
changed objects the change_log remembers individual changes to objects until \
<max_changes> is exceeded, then internal flags indicate that all objects in \
<object_list> have changed. A negative <max_changes> places no limit on the \
maximum number of changes; <object_list> need not be supplied in this case, \
but it should be supplied if it is a different list to that logged by the \
super_change_log passed to CHANGE_LOG_MERGE. \
Note that the change_log maintains a pointer to the <object_list> which will \
be owned by another object. Make sure the object_list is not destroyed first! \
============================================================================*/ \
{ \
	struct CHANGE_LOG(object_type) *change_log; \
\
	ENTER(CREATE_CHANGE_LOG(object_type)); \
	change_log = (struct CHANGE_LOG(object_type) *)NULL; \
	if ((max_changes < 0) || object_list) \
	{ \
		if (ALLOCATE(change_log, struct CHANGE_LOG(object_type), 1)) \
		{ \
			change_log->change_summary = CHANGE_LOG_OBJECT_UNCHANGED(object_type); \
			change_log->all_change = 0; \
			change_log->number_of_changes = 0; \
			change_log->max_changes = max_changes; \
			change_log->entry_list = CREATE_LIST(CHANGE_LOG_ENTRY(object_type))(); \
			change_log->object_list = object_list; \
			if (!(change_log->entry_list)) \
			{ \
				display_message(ERROR_MESSAGE, "CREATE_CHANGE_LOG(" #object_type \
					").  Could not create entry_list"); \
				DEALLOCATE(change_log); \
				change_log = (struct CHANGE_LOG(object_type) *)NULL; \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CREATE_CHANGE_LOG(" #object_type ").  Could not allocate log"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CREATE_CHANGE_LOG(" #object_type ").  Could not allocate log"); \
	} \
	LEAVE; \
\
	return (change_log); \
} /* CREATE_CHANGE_LOG(object_type) */

#define DECLARE_DESTROY_CHANGE_LOG_FUNCTION( object_type ) \
PROTOTYPE_DESTROY_CHANGE_LOG_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 9 December 2002 \
\
DESCRIPTION : \
Destroys the <**change_log> and sets <*change_log> to NULL. \
============================================================================*/ \
{ \
	int return_code; \
	struct CHANGE_LOG(object_type) *change_log; \
\
	ENTER(DESTROY_CHANGE_LOG(object_type)); \
	if (change_log_address && (change_log = *change_log_address)) \
	{ \
		return_code = 1; \
		DESTROY_LIST(CHANGE_LOG_ENTRY(object_type))(&(change_log->entry_list)); \
    DEALLOCATE(*change_log_address); \
		*change_log_address = (struct CHANGE_LOG(object_type) *)NULL; \
	} \
	else \
	{ \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* DESTROY_CHANGE_LOG(object_type) */

#define DECLARE_CHANGE_LOG_CLEAR_FUNCTION( object_type ) \
PROTOTYPE_CHANGE_LOG_CLEAR_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 20 February 2003 \
\
DESCRIPTION : \
Clears all entries/flags in the change_log. \
============================================================================*/ \
{ \
	int return_code; \
\
	ENTER(CHANGE_LOG_CLEAR(object_type)); \
	if (change_log) \
	{ \
		change_log->change_summary = CHANGE_LOG_OBJECT_UNCHANGED(object_type); \
		change_log->all_change = 0; \
		change_log->number_of_changes = 0; \
		REMOVE_ALL_OBJECTS_FROM_LIST(CHANGE_LOG_ENTRY(object_type))( \
			change_log->entry_list); \
		return_code = 1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHANGE_LOG_CLEAR(" #object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHANGE_LOG_CLEAR(object_type) */

#define DECLARE_CHANGE_LOG_ALL_CHANGE_FUNCTION( object_type ) \
PROTOTYPE_CHANGE_LOG_ALL_CHANGE_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 20 February 2003 \
\
DESCRIPTION : \
Tells <change_log> that all objects it monitors have undergone the <change>. \
Not to be used when max_changes is negative. \
============================================================================*/ \
{ \
	int return_code; \
\
	ENTER(CHANGE_LOG_ALL_CHANGE(object_type)); \
	if (change_log && (0 <= change_log->max_changes)) \
	{ \
		change_log->change_summary |= change; \
		/* if this change includes ADD, and REMOVE has been set, must set \
			 IDENTIFIER_CHANGED and NOT_IDENTIFIER_CHANGED in change_summary */ \
		if ((change & CHANGE_LOG_OBJECT_ADDED(object_type)) && \
			(change_log->change_summary & CHANGE_LOG_OBJECT_REMOVED(object_type))) \
		{ \
			change_log->change_summary |= \
				CHANGE_LOG_OBJECT_CHANGED(object_type); \
		} \
		change_log->number_of_changes += \
			NUMBER_IN_LIST(object_type)(change_log->object_list); \
		if (!change_log->all_change) \
		{ \
			change_log->all_change = 1; \
			REMOVE_ALL_OBJECTS_FROM_LIST(CHANGE_LOG_ENTRY(object_type))( \
				change_log->entry_list); \
		} \
		return_code = 1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHANGE_LOG_ALL_CHANGE(" #object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHANGE_LOG_ALL_CHANGE(object_type) */

#define DECLARE_CHANGE_LOG_OBJECT_CHANGE_FUNCTION( object_type ) \
PROTOTYPE_CHANGE_LOG_OBJECT_CHANGE_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 15 April 2003 \
\
DESCRIPTION : \
Tells the <change_log> that <object> has undergone the <change>. \
============================================================================*/ \
{ \
	int return_code; \
	struct CHANGE_LOG_ENTRY(object_type) *entry; \
\
	ENTER(CHANGE_LOG_OBJECT_CHANGE(object_type)); \
	if (change_log && object) \
	{ \
		return_code = 1; \
		change_log->change_summary |= change; \
		/* if this change includes ADD, and REMOVE has been set, must set \
			 IDENTIFIER_CHANGED and NOT_IDENTIFIER_CHANGED in change_summary */ \
		if ((change & CHANGE_LOG_OBJECT_ADDED(object_type)) && \
			(change_log->change_summary & CHANGE_LOG_OBJECT_REMOVED(object_type))) \
		{ \
			change_log->change_summary |= \
				CHANGE_LOG_OBJECT_CHANGED(object_type); \
		} \
		(change_log->number_of_changes)++; \
		if (!change_log->all_change) \
		{ \
			if ((0 <= change_log->max_changes) && \
				(change_log->number_of_changes > change_log->max_changes)) \
			{ \
				return_code = CHANGE_LOG_ALL_CHANGE(object_type)(change_log, change); \
			} \
			else \
			{ \
				if (entry = FIND_BY_IDENTIFIER_IN_LIST(CHANGE_LOG_ENTRY(object_type), \
					the_object)(object, change_log->entry_list)) \
				{ \
					switch (change) \
					{ \
						case CHANGE_LOG_OBJECT_ADDED(object_type): \
						{ \
							if (entry->change == CHANGE_LOG_OBJECT_REMOVED(object_type)) \
							{ \
								/* adding after removed is OK but assume it was modified */ \
								entry->change = CHANGE_LOG_OBJECT_CHANGED(object_type); \
							} \
							else \
							{ \
								entry->change = CHANGE_LOG_OBJECT_ADDED(object_type); \
							} \
						} break; \
						case CHANGE_LOG_OBJECT_REMOVED(object_type): \
						{ \
							if (entry->change == CHANGE_LOG_OBJECT_ADDED(object_type)) \
							{ \
								/* if just been added, no change needs to be noted */ \
								REMOVE_OBJECT_FROM_LIST(CHANGE_LOG_ENTRY(object_type))(entry, \
									change_log->entry_list); \
							} \
							else \
							{ \
								entry->change = CHANGE_LOG_OBJECT_REMOVED(object_type); \
							} \
						} break; \
						case CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(object_type): \
						case CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(object_type): \
						case CHANGE_LOG_OBJECT_CHANGED(object_type): \
						case CHANGE_LOG_RELATED_OBJECT_CHANGED(object_type): \
						{ \
							/* no change if object added or removed */ \
							if ((entry->change != CHANGE_LOG_OBJECT_ADDED(object_type)) && \
								(entry->change != CHANGE_LOG_OBJECT_REMOVED(object_type))) \
							{ \
								/* bitwise OR */ \
								entry->change |= change; \
							} \
						} break; \
						default: \
						{ \
							/* don't want this to be called with UNCHANGED so that case is \
								 handled here too */ \
							display_message(ERROR_MESSAGE, \
								"CHANGE_LOG_OBJECT_CHANGE(" #object_type \
								").  Invalid change type"); \
							return_code = 0; \
						} \
					} \
				} \
				else \
				{ \
					/* create a new CHANGE_LOG_ENTRY */ \
					if (change == CHANGE_LOG_OBJECT_UNCHANGED(object_type)) \
					{ \
						/* cannot create an entry marked as OBJECT_UNCHANGED */ \
						display_message(ERROR_MESSAGE, \
							"CHANGE_LOG_OBJECT_CHANGE(" #object_type \
							").  Cannot note OBJECT_UNCHANGED"); \
						return_code = 0; \
					} \
					else \
					{ \
						/* note CREATE CHANGE_LOG_ENTRY incorporated in this function */ \
						if (ALLOCATE(entry, struct CHANGE_LOG_ENTRY(object_type), 1)) \
						{ \
							/* objects ACCESSed internally to ensure not destroyed */ \
							entry->the_object = ACCESS(object_type)(object); \
							entry->change = change; \
							entry->access_count = 0; \
							if (!ADD_OBJECT_TO_LIST(CHANGE_LOG_ENTRY(object_type))(entry, \
								change_log->entry_list)) \
							{ \
								display_message(ERROR_MESSAGE, \
									"CHANGE_LOG_OBJECT_CHANGE(" #object_type \
									").  Could not add change entry to list"); \
								DESTROY(CHANGE_LOG_ENTRY(object_type))(&entry); \
								return_code = 0; \
							} \
						} \
						else \
						{ \
							display_message(ERROR_MESSAGE, \
								"CHANGE_LOG_OBJECT_CHANGE(" #object_type \
								").  Could not add create change entry"); \
							return_code = 0; \
						} \
					} \
				} \
			} \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHANGE_LOG_OBJECT_CHANGE(" #object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHANGE_LOG_OBJECT_CHANGE(object_type) */

#define DECLARE_CHANGE_LOG_GET_CHANGE_SUMMARY_FUNCTION( object_type ) \
PROTOTYPE_CHANGE_LOG_GET_CHANGE_SUMMARY_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 4 February 2003 \
\
DESCRIPTION : \
Returns a bitwise OR of all the changes enumerators in the change_log. Check \
against a particular change by bitwise ANDing with ADD, REMOVE etc. \
============================================================================*/ \
{ \
	int return_code; \
\
	ENTER(CHANGE_LOG_GET_CHANGE_SUMMARY(object_type)); \
	if (change_log && change_summary_address) \
	{ \
		*change_summary_address = change_log->change_summary; \
		return_code = 1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHANGE_LOG_GET_CHANGE_SUMMARY(" #object_type \
			").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHANGE_LOG_GET_CHANGE_SUMMARY(object_type) */

#define DECLARE_CHANGE_LOG_GET_NUMBER_OF_CHANGES_FUNCTION( object_type ) \
PROTOTYPE_CHANGE_LOG_GET_NUMBER_OF_CHANGES_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 3 February 2003 \
\
DESCRIPTION : \
Returns the number of changes that have been logged in <change_log>. \
Note that the returned number may not exactly match the number of objects \
changed since some objects may have been changed more than once. \
============================================================================*/ \
{ \
	int return_code; \
\
	ENTER(CHANGE_LOG_GET_NUMBER_OF_CHANGES(object_type)); \
	if (change_log && number_of_changes_address) \
	{ \
		*number_of_changes_address = change_log->number_of_changes; \
		return_code = 1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHANGE_LOG_GET_NUMBER_OF_CHANGES(" #object_type \
			").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHANGE_LOG_GET_NUMBER_OF_CHANGES(object_type) */

#define DECLARE_CHANGE_LOG_QUERY_FUNCTION( object_type ) \
PROTOTYPE_CHANGE_LOG_QUERY_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 20 February 2003 \
\
DESCRIPTION : \
Returns in <change_address> the status of <object> from <change_log>. \
Note that it is possible for the change to be a union of any of \
OBJECT_ADDED, OBJECT_IDENTIFIER_CHANGED and OBJECT_NOT_IDENTIFIER_CHANGED, \
meaning that any of them may have occured. Hence, the calling function should \
test by performing logical AND against any or all of these states. \
The OBJECT_REMOVED bit always occurs alone. \
Unchanged objects are returned as OBJECT_UNCHANGED. \
============================================================================*/ \
{ \
	int return_code; \
	struct CHANGE_LOG_ENTRY(object_type) *entry; \
\
	ENTER(CHANGE_LOG_QUERY(object_type)); \
	if (change_log && object && change_address) \
	{ \
		if (change_log->all_change) \
		{ \
			if (change_log->change_summary & \
				CHANGE_LOG_OBJECT_REMOVED(object_type)) \
			{ \
				if (IS_OBJECT_IN_LIST(object_type)(object, change_log->object_list)) \
				{ \
					/* clear REMOVED bit with bitwise XOR */ \
					*change_address = (enum CHANGE_LOG_CHANGE(object_type))( \
						 change_log->change_summary ^ \
						 CHANGE_LOG_OBJECT_REMOVED(object_type)); \
				} \
				else \
				{ \
					/* object has been removed */ \
					*change_address = CHANGE_LOG_OBJECT_REMOVED(object_type); \
				} \
			} \
			else \
			{ \
				*change_address = change_log->change_summary; \
			} \
		} \
		else \
		{ \
			if (entry = FIND_BY_IDENTIFIER_IN_LIST(CHANGE_LOG_ENTRY(object_type), \
				the_object)(object, change_log->entry_list)) \
			{ \
				*change_address = entry->change; \
			} \
			else \
			{ \
				*change_address = CHANGE_LOG_OBJECT_UNCHANGED(object_type); \
			} \
		} \
		return_code = 1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHANGE_LOG_QUERY(" #object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHANGE_LOG_QUERY(object_type) */

#define DECLARE_CHANGE_LOG_MERGE_FUNCTION( object_type ) \
PROTOTYPE_CHANGE_LOG_MERGE_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 20 February 2003 \
\
DESCRIPTION : \
Incorporates into <change_log> the changes from <super_change_log> that are \
relevant to it. <super_change_log> should be a change log for the superset of \
objects logged in <change_log>. \
Note that both change logs should be set up identically; either both use the \
max_changes/change_all capability or both do not. \
============================================================================*/ \
{ \
	int return_code; \
\
	ENTER(CHANGE_LOG_MERGE(object_type)); \
	if (change_log && super_change_log && ( \
		((change_log->max_changes < 0) && (super_change_log->max_changes < 0)) || \
		((change_log->max_changes >= 0) && (super_change_log->max_changes >= 0)))) \
	{ \
		if (super_change_log->all_change || change_log->all_change) \
		{ \
			return_code = CHANGE_LOG_ALL_CHANGE(object_type)(change_log, \
				super_change_log->change_summary); \
		} \
		else \
		{ \
			return_code = FOR_EACH_OBJECT_IN_LIST(CHANGE_LOG_ENTRY(object_type))( \
				CHANGE_LOG_ENTRY_OBJECT_CHANGE(object_type), (void *)change_log, \
				super_change_log->entry_list); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHANGE_LOG_MERGE(" #object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHANGE_LOG_MERGE(object_type) */

#if defined (OLD_CODE_TO_KEEP)

#if defined (FULL_NAMES)
#define CHANGE_LOG_FIRST_OBJECT_THAT_( object_type ) \
	change_log_first_object_that_ ## object_type
#else
#define CHANGE_LOG_FIRST_OBJECT_THAT_( object_type ) clfot_ ## object_type
#endif
#define CHANGE_LOG_FIRST_OBJECT_THAT( object_type ) \
	CHANGE_LOG_FIRST_OBJECT_THAT_(object_type)

#define PROTOTYPE_CHANGE_LOG_FIRST_OBJECT_THAT_FUNCTION( object_type ) \
struct object_type *CHANGE_LOG_FIRST_OBJECT_THAT(object_type)( \
	struct CHANGE_LOG(object_type) *change_log, \
	CHANGE_LOG_CONDITIONAL_FUNCTION(object_type) *conditional_function, \
	void *user_data) \
/***************************************************************************** \
LAST MODIFIED : 9 December 2002 \
\
DESCRIPTION : \
Returns the first object in <change_log> for which <conditional_function> \
with <user_data> returns true, or the first object in <change_log> if \
<conditional_function> is NULL. Otherwise NULL object. \
Note the special format of the CHANGE_LOG_CONDITIONAL_FUNCTION(object_type), \
which has an additional middle argument of the object's change status. \
==============================================================================*/

#define DECLARE_CHANGE_LOG_FIRST_OBJECT_THAT_FUNCTION( object_type ) \
PROTOTYPE_CHANGE_LOG_FIRST_OBJECT_THAT_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 9 December 2002 \
\
DESCRIPTION : \
Returns the first object in <change_log> for which <conditional_function> \
with <user_data> returns true, or the first object in <change_log> if \
<conditional_function> is NULL. Otherwise NULL object. \
Note the special format of the CHANGE_LOG_CONDITIONAL_FUNCTION(object_type), \
which has an additional middle argument of the object's change status. \
============================================================================*/ \
{ \
	struct CHANGE_LOG_CONDITIONAL_DATA(object_type) data; \
	struct CHANGE_LOG_ENTRY(object_type) *entry; \
	struct object_type *object; \
\
	ENTER(CHANGE_LOG_FIRST_OBJECT_THAT(object_type)); \
	object = (struct object_type *)NULL; \
	if (change_log) \
	{ \
		data.conditional_function = conditional_function; \
		data.user_data = user_data; \
		if (entry = FIRST_OBJECT_IN_LIST_THAT(CHANGE_LOG_ENTRY(object_type))( \
			CHANGE_LOG_CONDITIONAL(object_type), (void *)&data, \
			change_log->entry_list)) \
		{ \
			object = entry->the_object; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHANGE_LOG_FIRST_OBJECT_THAT(" #object_type \
			").  Invalid argument(s)"); \
	} \
	LEAVE; \
\
	return (object); \
} /* CHANGE_LOG_FIRST_OBJECT_THAT(object_type) */

#if defined (FULL_NAMES)
#define CHANGE_LOG_FOR_EACH_OBJECT_( object_type ) \
	change_log_for_each_object_ ## object_type
#else
#define CHANGE_LOG_FOR_EACH_OBJECT_( object_type ) clfeo_ ## object_type
#endif
#define CHANGE_LOG_FOR_EACH_OBJECT( object_type ) \
	CHANGE_LOG_FOR_EACH_OBJECT_(object_type)

#define PROTOTYPE_CHANGE_LOG_FOR_EACH_OBJECT_FUNCTION( object_type ) \
int CHANGE_LOG_FOR_EACH_OBJECT(object_type)( \
	struct CHANGE_LOG(object_type) *change_log, \
	CHANGE_LOG_ITERATOR_FUNCTION(object_type) *iterator_function, \
	void *user_data) \
/***************************************************************************** \
LAST MODIFIED : 9 December 2002 \
\
DESCRIPTION : \
Calls the <iterator_function> with <user_data> for each object in \
<change_log>. \
Note the special format of the CHANGE_LOG_ITERATOR_FUNCTION(object_type), \
which has an additional middle argument of the object's change status. \
==============================================================================*/

#define DECLARE_CHANGE_LOG_FOR_EACH_OBJECT_FUNCTION( object_type ) \
PROTOTYPE_CHANGE_LOG_FOR_EACH_OBJECT_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 9 December 2002 \
\
DESCRIPTION : \
Calls the <iterator_function> with <user_data> for each object in \
<change_log>. \
Note the special format of the CHANGE_LOG_ITERATOR_FUNCTION(object_type), \
which has an additional middle argument of the object's change status. \
============================================================================*/ \
{ \
	int return_code; \
	struct CHANGE_LOG_ITERATOR_DATA(object_type) data; \
\
	ENTER(CHANGE_LOG_FOR_EACH_OBJECT(object_type)); \
	if (change_log && iterator_function) \
	{ \
		data.iterator_function = iterator_function; \
		data.user_data = user_data; \
		return_code = FOR_EACH_OBJECT_IN_LIST(CHANGE_LOG_ENTRY(object_type))( \
			CHANGE_LOG_ITERATOR(object_type), (void *)&data, \
			change_log->entry_list); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHANGE_LOG_FOR_EACH_OBJECT(" #object_type \
			").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHANGE_LOG_FOR_EACH_OBJECT(object_type) */

#endif /* defined (OLD_CODE_TO_KEEP) */

#define FULL_DECLARE_CHANGE_LOG_TYPES( object_type ) \
FULL_DECLARE_CHANGE_LOG_ENTRY_TYPE(object_type); \
FULL_DECLARE_CHANGE_LOG_TYPE(object_type)

#define DECLARE_CHANGE_LOG_MODULE_FUNCTIONS( object_type ) \
PROTOTYPE_OBJECT_FUNCTIONS(CHANGE_LOG_ENTRY(object_type)); \
PROTOTYPE_LIST_FUNCTIONS(CHANGE_LOG_ENTRY(object_type)); \
DECLARE_DESTROY_CHANGE_LOG_ENTRY_FUNCTION(object_type) \
DECLARE_OBJECT_FUNCTIONS(CHANGE_LOG_ENTRY(object_type)) \
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(CHANGE_LOG_ENTRY(object_type), \
	the_object, struct object_type *, compare_pointer) \
DECLARE_INDEXED_LIST_FUNCTIONS(CHANGE_LOG_ENTRY(object_type)) \
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION( \
	CHANGE_LOG_ENTRY(object_type), the_object, struct object_type *, \
	compare_pointer) \
DECLARE_CHANGE_LOG_ENTRY_OBJECT_CHANGE_FUNCTION(object_type)

#define DECLARE_CHANGE_LOG_FUNCTIONS( object_type ) \
DECLARE_CREATE_CHANGE_LOG_FUNCTION(object_type) \
DECLARE_DESTROY_CHANGE_LOG_FUNCTION(object_type) \
DECLARE_CHANGE_LOG_CLEAR_FUNCTION(object_type) \
DECLARE_CHANGE_LOG_ALL_CHANGE_FUNCTION(object_type) \
DECLARE_CHANGE_LOG_OBJECT_CHANGE_FUNCTION(object_type) \
DECLARE_CHANGE_LOG_GET_CHANGE_SUMMARY_FUNCTION(object_type) \
DECLARE_CHANGE_LOG_GET_NUMBER_OF_CHANGES_FUNCTION(object_type) \
DECLARE_CHANGE_LOG_QUERY_FUNCTION(object_type) \
DECLARE_CHANGE_LOG_MERGE_FUNCTION(object_type)

#endif /* !defined (CHANGE_LOG_PRIVATE_H) */
