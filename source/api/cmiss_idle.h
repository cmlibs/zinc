/*******************************************************************************
FILE : api/cmiss_idle.h

LAST MODIFIED : 21 March, 2005

DESCRIPTION :
The public interface to idle callbacks
==============================================================================*/
#ifndef __API_CMISS_IDLE_H__
#define __API_CMISS_IDLE_H__

#include "general/object.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct Cmiss_idle_package * Cmiss_idle_package_id;
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
An identifier for an idle package object.
==============================================================================*/

typedef struct Cmiss_idle_callback * Cmiss_idle_callback_id;
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
An identifier for an idle callback object.
==============================================================================*/

typedef int Cmiss_idle_callback_function(void *user_data);
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
The type used for idle callback data.
==============================================================================*/

Cmiss_idle_callback_id Cmiss_idle_package_add_callback(Cmiss_idle_package_id pkg,
 Cmiss_idle_callback_function *callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
Sets an idle callback.
==============================================================================*/

int DESTROY(Cmiss_idle_callback)(Cmiss_idle_package_id pkg,
	Cmiss_idle_callback_id *callback);
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
Destroys an idle callback, causing it to not be called any longer.
==============================================================================*/

int DESTROY(Cmiss_idle_package)(Cmiss_idle_package_id *pkg);
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
Destroys the idle package object.
==============================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
#endif /* __API_CMISS_IDLE_H__ */
