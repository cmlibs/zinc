/*******************************************************************************
FILE : change_log.h

LAST MODIFIED : 25 March 2003

DESCRIPTION :
Structure for storing information about objects added, removed or modified
with and/or without identifier. Used for change messages throughout Cmgui.
Internally the objects are stored in a list indexed by the address of the
object, and containing an enumerator that describes the change. Objects in the
change_log are accessed so that removed objects exist until after clients
get to hear about their imminent demise. Change enumerator logic is slightly
involved -- refer to function CHANGE_LOG_ADD for details.
==============================================================================*/
#if !defined (CHANGE_LOG_H)
#define CHANGE_LOG_H

/*
Macros
======
*/

/*
Global types
------------
*/

#if defined (FULL_NAMES)
#define CHANGE_LOG( object_type ) Change_log_ ## object_type
#else
#define CHANGE_LOG( object_type ) Cl_ ## object_type
#endif

#define DECLARE_CHANGE_LOG_TYPE( object_type ) \
	struct CHANGE_LOG(object_type)

#if defined (FULL_NAMES)
#define CHANGE_LOG_OBJECT_UNCHANGED( object_type ) \
	change_log_object_unchanged_ ## object_type
#else
#define CHANGE_LOG_OBJECT_UNCHANGED( object_type ) CLOU_ ## object_type
#endif

#if defined (FULL_NAMES)
#define CHANGE_LOG_OBJECT_ADDED( object_type ) \
	change_log_object_added_ ## object_type
#else
#define CHANGE_LOG_OBJECT_ADDED( object_type ) CLOA_ ## object_type
#endif

#if defined (FULL_NAMES)
#define CHANGE_LOG_OBJECT_REMOVED( object_type ) \
	change_log_object_removed_ ## object_type
#else
#define CHANGE_LOG_OBJECT_REMOVED( object_type ) CLOR_ ## object_type
#endif

#if defined (FULL_NAMES)
#define CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED( object_type ) \
	change_log_object_identifier_changed_ ## object_type
#else
#define CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED( object_type ) \
	CLOIC_ ## object_type
#endif

#if defined (FULL_NAMES)
#define CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED( object_type ) \
	change_log_object_not_identifier_changed_ ## object_type
#else
#define CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED( object_type )  \
	CLONIC ## object_type
#endif

#if defined (FULL_NAMES)
#define CHANGE_LOG_OBJECT_CHANGED( object_type ) \
	change_log_object_changed_ ## object_type
#else
#define CHANGE_LOG_OBJECT_CHANGED( object_type ) CLOC_ ## object_type
#endif

#if defined (FULL_NAMES)
#define CHANGE_LOG_RELATED_OBJECT_CHANGED( object_type ) \
	change_log_related_object_changed_ ## object_type
#else
#define CHANGE_LOG_RELATED_OBJECT_CHANGED( object_type ) CLROC_ ## object_type
#endif

#if defined (FULL_NAMES)
#define CHANGE_LOG_CHANGE( object_type ) \
	change_log_change_ ## object_type
#else
#define CHANGE_LOG_CHANGE( object_type ) Clct_ ## object_type
#endif

#define DECLARE_CHANGE_LOG_CHANGE_TYPE( object_type ) \
enum CHANGE_LOG_CHANGE(object_type) \
/***************************************************************************** \
LAST MODIFIED : 14 February 2003 \
\
DESCRIPTION : \
Emumeration describing what has change about a particular object. Values are \
attributed so that bitwise logical operations can be used to treat different \
states in the same way. Note in particular that OBJECT_CHANGED equals \
OBJECT_IDENTIFIER_CHANGED | OBJECT_NOT_IDENTIFIER_CHANGED, a logical OR. \
============================================================================*/ \
{ \
	/* object is unchanged; no object will be listed in the change_log with \
		 this enumeration */ \
	CHANGE_LOG_OBJECT_UNCHANGED(object_type) = 0, \
	/* object has been added */ \
	CHANGE_LOG_OBJECT_ADDED(object_type) = 1, \
	/* object has been removed */ \
	CHANGE_LOG_OBJECT_REMOVED(object_type) = 2, \
	/* identifier of object has been changed */ \
	CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(object_type) = 4, \
	/* contents but not identifier of object has been changed */ \
	CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(object_type) = 8, \
	/* identifier and contents of object has been changed */ \
	CHANGE_LOG_OBJECT_CHANGED(object_type) = 12, \
	/* related object that affects the representation of this object has been \
		 changed */ \
	CHANGE_LOG_RELATED_OBJECT_CHANGED(object_type) = 16 \
} /* enum CHANGE_LOG_CHANGE(object_type) */

#if defined (FULL_NAMES)
#define CHANGE_LOG_CONDITIONAL_FUNCTION( object_type ) \
	change_log_conditional_function_ ## object_type
#else
#define CHANGE_LOG_CONDITIONAL_FUNCTION( object_type ) clcf_ ## object_type
#endif

/* Note: conditional functions are passed the object change status as their
	 second argument */
#define DECLARE_CHANGE_LOG_CONDITIONAL_FUNCTION( object_type ) \
typedef int (CHANGE_LOG_CONDITIONAL_FUNCTION(object_type))( \
	struct object_type *object, enum CHANGE_LOG_CHANGE(object_type) change, \
	void *user_data)

#if defined (FULL_NAMES)
#define CHANGE_LOG_ITERATOR_FUNCTION( object_type ) \
	change_log_iterator_function_ ## object_type
#else
#define CHANGE_LOG_ITERATOR_FUNCTION( object_type ) clif_ ## object_type
#endif

/* Note: iterator functions are passed the object change status as their
	 second argument */
#define DECLARE_CHANGE_LOG_ITERATOR_FUNCTION( object_type ) \
typedef int (CHANGE_LOG_ITERATOR_FUNCTION(object_type))( \
	struct object_type *object, enum CHANGE_LOG_CHANGE(object_type) change, \
	void *user_data)

/*
Global functions
----------------
*/

#define CREATE_CHANGE_LOG( object_type ) CREATE(CHANGE_LOG(object_type))

#define PROTOTYPE_CREATE_CHANGE_LOG_FUNCTION( object_type ) \
struct CHANGE_LOG(object_type) *CREATE_CHANGE_LOG(object_type)( \
	struct LIST(object_type) *object_list, int max_changes) \
/***************************************************************************** \
LAST MODIFIED : 25 March 2003 \
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
==============================================================================*/

#define DESTROY_CHANGE_LOG( object_type ) DESTROY(CHANGE_LOG(object_type))

#define PROTOTYPE_DESTROY_CHANGE_LOG_FUNCTION( object_type ) \
int DESTROY_CHANGE_LOG(object_type)( \
	struct CHANGE_LOG(object_type) **change_log_address) \
/***************************************************************************** \
LAST MODIFIED : 9 December 2002 \
\
DESCRIPTION : \
Destroys the <**change_log> and sets <*change_log> to NULL. \
==============================================================================*/

#if defined (FULL_NAMES)
#define CHANGE_LOG_CLEAR_( object_type ) change_log_clear_ ## object_type
#else
#define CHANGE_LOG_CLEAR_( object_type ) clc_ ## object_type
#endif
#define CHANGE_LOG_CLEAR( object_type ) CHANGE_LOG_CLEAR_(object_type)

#define PROTOTYPE_CHANGE_LOG_CLEAR_FUNCTION( object_type ) \
int CHANGE_LOG_CLEAR(object_type)(struct CHANGE_LOG(object_type) *change_log) \
/***************************************************************************** \
LAST MODIFIED : 3 February 2003 \
\
DESCRIPTION : \
Clears all entries/flags in the change_log. \
==============================================================================*/

#if defined (FULL_NAMES)
#define CHANGE_LOG_ALL_CHANGE_( object_type ) \
	change_log_all_change_ ## object_type
#else
#define CHANGE_LOG_ALL_CHANGE_( object_type ) clac_ ## object_type
#endif
#define CHANGE_LOG_ALL_CHANGE( object_type ) \
	CHANGE_LOG_ALL_CHANGE_(object_type)

#define PROTOTYPE_CHANGE_LOG_ALL_CHANGE_FUNCTION( object_type ) \
int CHANGE_LOG_ALL_CHANGE(object_type)( \
	struct CHANGE_LOG(object_type) *change_log, \
	enum CHANGE_LOG_CHANGE(object_type) change) \
/***************************************************************************** \
LAST MODIFIED : 17 February 2003 \
\
DESCRIPTION : \
Tells <change_log> that all objects it monitors have undergone the <change>. \
Not to be used when max_changes is negative. \
==============================================================================*/

#if defined (FULL_NAMES)
#define CHANGE_LOG_OBJECT_CHANGE_( object_type ) \
	change_log_object_change_ ## object_type
#else
#define CHANGE_LOG_OBJECT_CHANGE_( object_type ) cloc_ ## object_type
#endif
#define CHANGE_LOG_OBJECT_CHANGE( object_type ) \
	CHANGE_LOG_OBJECT_CHANGE_(object_type)

#define PROTOTYPE_CHANGE_LOG_OBJECT_CHANGE_FUNCTION( object_type ) \
int CHANGE_LOG_OBJECT_CHANGE(object_type)( \
	struct CHANGE_LOG(object_type) *change_log, \
	struct object_type *object, enum CHANGE_LOG_CHANGE(object_type) change) \
/***************************************************************************** \
LAST MODIFIED : 3 February 2003 \
\
DESCRIPTION : \
Tells the <change_log> that <object> has undergone the <change>. \
==============================================================================*/

#if defined (FULL_NAMES)
#define CHANGE_LOG_GET_CHANGE_SUMMARY_( object_type ) \
	change_log_get_change_summary_ ## object_type
#else
#define CHANGE_LOG_GET_CHANGE_SUMMARY_( object_type ) clgcs_ ## object_type
#endif
#define CHANGE_LOG_GET_CHANGE_SUMMARY( object_type ) \
	CHANGE_LOG_GET_CHANGE_SUMMARY_(object_type)

#define PROTOTYPE_CHANGE_LOG_GET_CHANGE_SUMMARY_FUNCTION( object_type ) \
int CHANGE_LOG_GET_CHANGE_SUMMARY(object_type)( \
	struct CHANGE_LOG(object_type) *change_log, \
	enum CHANGE_LOG_CHANGE(object_type) *change_summary_address) \
/***************************************************************************** \
LAST MODIFIED : 4 February 2003 \
\
DESCRIPTION : \
Returns a bitwise OR of all the changes enumerators in the change_log. Check \
against a particular change by bitwise ANDing with ADD, REMOVE etc. \
==============================================================================*/

#if defined (FULL_NAMES)
#define CHANGE_LOG_GET_NUMBER_OF_CHANGES_( object_type ) \
	change_log_get_number_of_changes_ ## object_type
#else
#define CHANGE_LOG_GET_NUMBER_OF_CHANGES_( object_type ) clgnc_ ## object_type
#endif
#define CHANGE_LOG_GET_NUMBER_OF_CHANGES( object_type ) \
	CHANGE_LOG_GET_NUMBER_OF_CHANGES_(object_type)

#define PROTOTYPE_CHANGE_LOG_GET_NUMBER_OF_CHANGES_FUNCTION( object_type ) \
int CHANGE_LOG_GET_NUMBER_OF_CHANGES(object_type)( \
	struct CHANGE_LOG(object_type) *change_log, int *number_of_changes_address) \
/***************************************************************************** \
LAST MODIFIED : 3 February 2003 \
\
DESCRIPTION : \
Returns the number of changes that have been logged in <change_log>. \
Note that the returned number may not exactly match the number of objects \
changed since some objects may have been changed more than once. \
==============================================================================*/

#if defined (FULL_NAMES)
#define CHANGE_LOG_QUERY_( object_type ) \
	change_log_query_ ## object_type
#else
#define CHANGE_LOG_QUERY_( object_type ) clq_ ## object_type
#endif
#define CHANGE_LOG_QUERY( object_type ) CHANGE_LOG_QUERY_(object_type)

#define PROTOTYPE_CHANGE_LOG_QUERY_FUNCTION( object_type ) \
int CHANGE_LOG_QUERY(object_type)( \
	struct CHANGE_LOG(object_type) *change_log, struct object_type *object, \
	enum CHANGE_LOG_CHANGE(object_type) *change_address) \
/***************************************************************************** \
LAST MODIFIED : 3 February 2003 \
\
DESCRIPTION : \
Returns in <change_address> the status of <object> from <change_log>. \
Note that it is possible for the change to be a union of any of \
OBJECT_ADDED, OBJECT_IDENTIFIER_CHANGED and OBJECT_NOT_IDENTIFIER_CHANGED, \
meaning that any of them may have occured. Hence, the calling function should \
test by performing logical AND against any or all of these states. \
The OBJECT_REMOVED bit always occurs alone. \
Unchanged objects are returned as OBJECT_UNCHANGED. \
==============================================================================*/

#if defined (FULL_NAMES)
#define CHANGE_LOG_MERGE_( object_type ) change_log_merge_ ## object_type
#else
#define CHANGE_LOG_MERGE_( object_type ) clm_ ## object_type
#endif
#define CHANGE_LOG_MERGE( object_type ) CHANGE_LOG_MERGE_(object_type)

#define PROTOTYPE_CHANGE_LOG_MERGE_FUNCTION( object_type ) \
int CHANGE_LOG_MERGE(object_type)( \
	struct CHANGE_LOG(object_type) *change_log, \
	struct CHANGE_LOG(object_type) *super_change_log) \
/***************************************************************************** \
LAST MODIFIED : 17 February 2003 \
\
DESCRIPTION : \
Incorporates into <change_log> the changes from <super_change_log> that are \
relevant to it. <super_change_log> should be a change log for the superset of \
objects logged in <change_log>. \
Note that both change logs should be set up identically; either both use the \
max_changes/change_all capability or both do not. \
==============================================================================*/

#define DECLARE_CHANGE_LOG_TYPES( object_type ) \
DECLARE_CHANGE_LOG_TYPE(object_type); \
DECLARE_CHANGE_LOG_CHANGE_TYPE(object_type); \
DECLARE_CHANGE_LOG_CONDITIONAL_FUNCTION(object_type); \
DECLARE_CHANGE_LOG_ITERATOR_FUNCTION(object_type)

#define PROTOTYPE_CHANGE_LOG_FUNCTIONS( object_type ) \
PROTOTYPE_CREATE_CHANGE_LOG_FUNCTION(object_type); \
PROTOTYPE_DESTROY_CHANGE_LOG_FUNCTION(object_type); \
PROTOTYPE_CHANGE_LOG_CLEAR_FUNCTION(object_type); \
PROTOTYPE_CHANGE_LOG_ALL_CHANGE_FUNCTION(object_type); \
PROTOTYPE_CHANGE_LOG_OBJECT_CHANGE_FUNCTION(object_type); \
PROTOTYPE_CHANGE_LOG_GET_CHANGE_SUMMARY_FUNCTION(object_type); \
PROTOTYPE_CHANGE_LOG_GET_NUMBER_OF_CHANGES_FUNCTION(object_type); \
PROTOTYPE_CHANGE_LOG_QUERY_FUNCTION(object_type); \
PROTOTYPE_CHANGE_LOG_MERGE_FUNCTION(object_type) \

#endif
