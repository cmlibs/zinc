/*******************************************************************************
FILE : api/cmiss_timer.h

LAST MODIFIED : 11 April, 2005

DESCRIPTION :
The public interface to time callbacks
==============================================================================*/
#ifndef __API_CMISS_TIMER_H__
#define __API_CMISS_TIMER_H__

#include "general/object.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct Cmiss_timer_package * Cmiss_timer_package_id;
/*******************************************************************************
LAST MODIFIED : 11 April 2005

DESCRIPTION :
An identifier for a timer package object.
==============================================================================*/

typedef struct Cmiss_timer_callback * Cmiss_timer_callback_id;
/*******************************************************************************
LAST MODIFIED : 11 April 2005

DESCRIPTION :
An identifier for a timer callback object.
==============================================================================*/

typedef int Cmiss_timer_callback_function(void *user_data);
/*******************************************************************************
LAST MODIFIED : 11 April 2005

DESCRIPTION :
The type used for timer callback function.
==============================================================================*/

Cmiss_timer_callback_id Cmiss_timer_package_add_callback
(
 Cmiss_timer_package_id pkg,
 unsigned long secs, unsigned long nsecs,
 Cmiss_timer_callback_function *callback,
 void *user_data
);
/*******************************************************************************
LAST MODIFIED : 11 April 2005

DESCRIPTION :
Sets a timer callback.
==============================================================================*/

int DESTROY(Cmiss_timer_callback)(Cmiss_timer_package_id pkg,
	Cmiss_timer_callback_id *callback);
/*******************************************************************************
LAST MODIFIED : 11 April 2005

DESCRIPTION :
Destroys a timer callback, causing it to not be called any longer.
==============================================================================*/

int DESTROY(Cmiss_timer_package)(Cmiss_timer_package_id *pkg);
/*******************************************************************************
LAST MODIFIED : 11 April 2005

DESCRIPTION :
Destroys the timer package object.
==============================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
#endif /* __API_CMISS_TIMER_H__ */
