/*******************************************************************************
FILE : callback.h

LAST MODIFIED : 23 March 2000

DESCRIPTION :
Macro definition for lists of callbacks between objects.
==============================================================================*/
#if !defined (CALLBACK_H)
#define CALLBACK_H
#if defined (MOTIF)
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */
#include "general/list.h"
#include "general/object.h"

/*
Global Types
------------
*/

#if defined (MOTIF)
/*???DB.  Should have a return_code ? */
typedef void Callback_procedure(Widget,void *,void *);

struct Callback_data
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Contains all information necessary for a callback.
==============================================================================*/
{
	Callback_procedure *procedure;
	void *data;
}; /* struct Callback_data */
#endif /* defined (MOTIF) */

#if ! defined (SHORT_NAMES)
#define CMISS_CALLBACK_FUNCTION( callback_type ) callback_function_ ## callback_type
#else
#define CMISS_CALLBACK_FUNCTION( callback_type ) cbf_ ## callback_type
#endif

#define TYPEDEF_CMISS_CALLBACK_FUNCTION( callback_type , object_type , \
	call_data_type ) \
typedef void CMISS_CALLBACK_FUNCTION(callback_type)(object_type,call_data_type,void *)

#if ! defined (SHORT_NAMES)
#define CMISS_CALLBACK_ITEM( callback_type ) callback_item_ ## callback_type
#else
#define CMISS_CALLBACK_ITEM( callback_type ) cbi_ ## callback_type
#endif

#define DECLARE_CMISS_CALLBACK_TYPE( callback_type ) \
struct CMISS_CALLBACK_ITEM(callback_type) \

/*
Global functions
----------------
*/

#if ! defined (SHORT_NAMES)
#define CMISS_CALLBACK_LIST_CALL( callback_type ) callback_list_call_ ## callback_type
#else
#define CMISS_CALLBACK_LIST_CALL( callback_type ) cblc_ ## callback_type
#endif

#define PROTOTYPE_CMISS_CALLBACK_LIST_CALL_FUNCTION( callback_type , object_type , \
	call_data_type ) \
int CMISS_CALLBACK_LIST_CALL(callback_type)( \
	struct LIST(CMISS_CALLBACK_ITEM(callback_type)) *callback_list, \
	object_type object,call_data_type call_data)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Calls every callback in <callback_list> with <object> and <call_data>.
==============================================================================*/

#if ! defined (SHORT_NAMES)
#define CMISS_CALLBACK_LIST_ADD_CALLBACK( callback_type ) \
	callback_list_add_callback_ ## callback_type
#else
#define CMISS_CALLBACK_LIST_ADD_CALLBACK( callback_type ) cblac_ ## callback_type
#endif

#define PROTOTYPE_CMISS_CALLBACK_LIST_ADD_CALLBACK_FUNCTION(callback_type ) \
int CMISS_CALLBACK_LIST_ADD_CALLBACK(callback_type)( \
	struct LIST(CMISS_CALLBACK_ITEM(callback_type)) *callback_list, \
	CMISS_CALLBACK_FUNCTION(callback_type) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Adds a callback = <function> + <user_data> to <callback_list>.
==============================================================================*/

#if ! defined (SHORT_NAMES)
#define CMISS_CALLBACK_LIST_REMOVE_CALLBACK( callback_type ) \
	callback_list_remove_callback_ ## callback_type
#else
#define CMISS_CALLBACK_LIST_REMOVE_CALLBACK( callback_type ) cblrc_ ## callback_type
#endif

#define PROTOTYPE_CMISS_CALLBACK_LIST_REMOVE_CALLBACK_FUNCTION(callback_type) \
int CMISS_CALLBACK_LIST_REMOVE_CALLBACK(callback_type)( \
	struct LIST(CMISS_CALLBACK_ITEM(callback_type)) *callback_list, \
	CMISS_CALLBACK_FUNCTION(callback_type) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Removes a callback = <function> + <user_data> from <callback_list>.
=============================================================================*/

#define DECLARE_CMISS_CALLBACK_TYPES( callback_type , object_type , call_data_type ) \
TYPEDEF_CMISS_CALLBACK_FUNCTION(callback_type,object_type,call_data_type); \
DECLARE_CMISS_CALLBACK_TYPE(callback_type); \
DECLARE_LIST_TYPES(CMISS_CALLBACK_ITEM(callback_type))

/* note that most modules will not need to prototype functions in headers as
	 will use the following in wrappers or module functions */
#define PROTOTYPE_CMISS_CALLBACK_FUNCTIONS( callback_type , object_type , \
	call_data_type ) \
PROTOTYPE_LIST_FUNCTIONS(CMISS_CALLBACK_ITEM(callback_type)); \
PROTOTYPE_CMISS_CALLBACK_LIST_CALL_FUNCTION(callback_type,object_type, \
	call_data_type); \
PROTOTYPE_CMISS_CALLBACK_LIST_ADD_CALLBACK_FUNCTION(callback_type); \
PROTOTYPE_CMISS_CALLBACK_LIST_REMOVE_CALLBACK_FUNCTION(callback_type)

#endif /* !defined (CALLBACK_H) */
