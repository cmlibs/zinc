/*******************************************************************************
FILE : idle.h

LAST MODIFIED : 21 March 2005

DESCRIPTION :
The private interface to idle callback functions of cmgui.
==============================================================================*/
#ifndef __IDLE_H__
#define __IDLE_H__

#include "event_dispatcher.h"
#include "api/cmiss_idle.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define Idle_package Cmiss_idle_package
#define Idle_package_id Cmiss_idle_package_id
#define Idle_callback Cmiss_idle_callback
#define Idle_callback_id Cmiss_idle_callback_id
#define Idle_package_add_callback Cmiss_idle_package_add_callback
#define Idle_callback_function Cmiss_idle_callback_function

Idle_package_id CREATE(Idle_package)(struct Event_dispatcher *event_dispatcher);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
#endif /* __IDLE_H__ */
