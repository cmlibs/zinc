/*******************************************************************************
FILE : cmiss_time_keeper.c

LAST MODIFIED : 2 Mar 2009

DESCRIPTION :
The public interface of Cmiss_time_keeper which defines a relationship between
a bunch of time objects, keeps them in sync and allows control such as play,
rewind and fast forward.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "api/cmiss_time_keeper.h"
#include "general/debug.h"
#include "time/time_keeper.h"
#include "user_interface/message.h"

enum Cmiss_time_keeper_play_direction Cmiss_time_keeper_get_play_direction(
	Cmiss_time_keeper_id time_keeper)
{
	enum Time_keeper_play_direction play_direction;
	enum Cmiss_time_keeper_play_direction cmiss_time_keeper_play_direction;
	
	ENTER(Cmiss_time_keeper_get_play_direction);
	if (time_keeper)
	{
		play_direction = Time_keeper_get_play_direction(time_keeper);
		switch(play_direction)
		{
			case TIME_KEEPER_PLAY_FORWARD:
			{
				cmiss_time_keeper_play_direction = CMISS_TIME_KEEPER_PLAY_FORWARD;
			} break;
			case TIME_KEEPER_PLAY_BACKWARD:
			{
				cmiss_time_keeper_play_direction = CMISS_TIME_KEEPER_PLAY_BACKWARD;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_time_keeper_get_play_direction.  Unknown play direction.");
				cmiss_time_keeper_play_direction = CMISS_TIME_KEEPER_PLAY_FORWARD;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_time_keeper_get_play_direction.  Invalid time keeper.");
		cmiss_time_keeper_play_direction = CMISS_TIME_KEEPER_PLAY_FORWARD;
	}
	
	LEAVE;
	
	return(cmiss_time_keeper_play_direction);
}

int Cmiss_time_keeper_play(Cmiss_time_keeper_id time_keeper,
	enum Cmiss_time_keeper_play_direction play_direction)
{
	int return_code;

	ENTER(Cmiss_time_keeper_play);
	if (time_keeper)
	{
		switch(play_direction)
		{
			case CMISS_TIME_KEEPER_PLAY_FORWARD:
			{
				return_code = Time_keeper_play(time_keeper, 
					TIME_KEEPER_PLAY_FORWARD);
			} break;
			case CMISS_TIME_KEEPER_PLAY_BACKWARD:
			{
				return_code = Time_keeper_play(time_keeper, 
					TIME_KEEPER_PLAY_BACKWARD);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_time_keeper_play.  Unknown play direction.");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_time_keeper_play.  Invalid time keeper.");
		return_code=0;
	}
	
	LEAVE;
	
	return(return_code);
}

enum Cmiss_time_keeper_frame_mode Cmiss_time_keeper_get_frame_mode(
	Cmiss_time_keeper_id time_keeper)
{
	enum Cmiss_time_keeper_frame_mode frame_mode;

	ENTER(Cmiss_time_keeper_get_frame_mode);
	if (time_keeper)
	{
		if (Time_keeper_get_play_every_frame(time_keeper))
		{
			frame_mode = CMISS_TIME_KEEPER_PLAY_EVERY_FRAME;
		}
		else
		{
			frame_mode = CMISS_TIME_KEEPER_PLAY_REAL_TIME;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_time_keeper_get_frame_mode.  Invalid time keeper.");
		frame_mode = CMISS_TIME_KEEPER_INVALID_FRAME_MODE;
	}
	
	LEAVE;
	
	return(frame_mode);
}

int Cmiss_time_keeper_set_frame_mode(Cmiss_time_keeper_id time_keeper,
	enum Cmiss_time_keeper_frame_mode frame_mode)
{
	int return_code;

	ENTER(Cmiss_time_keeper_set_frame_mode);
	if (time_keeper)
	{
		switch(frame_mode)
		{
			case CMISS_TIME_KEEPER_PLAY_REAL_TIME:
			{
				return_code = Time_keeper_set_play_skip_frames(time_keeper);
			} break;
			case CMISS_TIME_KEEPER_PLAY_EVERY_FRAME:
			{
				return_code = Time_keeper_set_play_every_frame(time_keeper);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_time_keeper_set_frame_mode.  Unknown frame mode.");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_time_keeper_set_frame_mode.  Invalid time keeper.");
		return_code=0;
	}
	
	LEAVE;
	
	return(return_code);
}

enum Cmiss_time_keeper_repeat_mode Cmiss_time_keeper_get_repeat_mode(
	Cmiss_time_keeper_id time_keeper)
{
	enum Time_keeper_play_mode play_mode;
	enum Cmiss_time_keeper_repeat_mode repeat_mode;
	
	ENTER(Cmiss_time_keeper_repeat_mode);
	if (time_keeper)
	{
		play_mode = Time_keeper_get_play_mode(time_keeper);
		switch(play_mode)
		{
			case TIME_KEEPER_PLAY_ONCE:
			{
				repeat_mode = CMISS_TIME_KEEPER_PLAY_ONCE;
			} break;
			case TIME_KEEPER_PLAY_LOOP:
			{
				repeat_mode = CMISS_TIME_KEEPER_PLAY_LOOP;
			} break;
			case TIME_KEEPER_PLAY_SWING:
			{
				repeat_mode = CMISS_TIME_KEEPER_PLAY_SWING;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_time_keeper_get_repeat_mode.  Unknown repeat mode.");
				repeat_mode = CMISS_TIME_KEEPER_INVALID_REPEAT_MODE;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_time_keeper_get_repeat_mode.  Invalid time keeper.");
		repeat_mode = CMISS_TIME_KEEPER_INVALID_REPEAT_MODE;
	}

	return (repeat_mode);
}

int Cmiss_time_keeper_set_repeat_mode(Cmiss_time_keeper_id time_keeper,
	enum Cmiss_time_keeper_repeat_mode repeat_mode)
{
	int return_code;

	ENTER(Cmiss_time_keeper_repeat_mode);
	if (time_keeper)
	{
		switch(repeat_mode)
		{
			case CMISS_TIME_KEEPER_PLAY_ONCE:
			{
				return_code = Time_keeper_set_play_once(time_keeper);
			} break;
			case CMISS_TIME_KEEPER_PLAY_LOOP:
			{
				return_code = Time_keeper_set_play_loop(time_keeper);
			} break;
			case CMISS_TIME_KEEPER_PLAY_SWING:
			{
				return_code = Time_keeper_set_play_swing(time_keeper);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_time_keeper_set_repeat_mode.  Unknown repeat mode.");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_time_keeper_set_repeat_mode.  Invalid time keeper.");
		return_code=0;
	}
	
	LEAVE;
	
	return(return_code);
}

int Cmiss_time_keeper_destroy(Cmiss_time_keeper_id *time_keeper_address)
{
	return (DEACCESS(Time_keeper)(time_keeper_address));
}
