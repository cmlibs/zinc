/*******************************************************************************
FILE : timer.h

LAST MODIFIED : 11 April 2005

DESCRIPTION :
The private interface to the timer callback functions of cmgui.
==============================================================================*/
#ifndef __TIMER_H__
#define __TIMER_H__

#include "event_dispatcher.h"
#include "api/cmiss_timer.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define Timer_package Cmiss_timer_package
#define Timer_package_id Cmiss_timer_package_id
#define Timer_callback Cmiss_timer_callback
#define Timer_callback_id Cmiss_timer_callback_id
#define Timer_package_add_callback Cmiss_timer_package_add_callback
#define Timer_callback_function Cmiss_timer_callback_function

Timer_package_id CREATE(Timer_package)(struct Event_dispatcher *event_dispatcher);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
#endif /* __TIMER_H__ */
