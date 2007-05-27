/*******************************************************************************
FILE : cmiss_texture.c

LAST MODIFIED : 24 May 2007

DESCRIPTION :
The public interface to the Cmiss_texture object.
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
 * Contributor(s):
 * Shane Blackett shane at blackett.co.nz
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

#include "api/cmiss_texture.h"
#include "general/debug.h"
#include "graphics/texture.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

struct Cmiss_texture *Cmiss_texture_manager_get_texture(
	struct Cmiss_texture_manager *manager, const char *name)
/*******************************************************************************
LAST MODIFIED : 24 May 2007

DESCRIPTION :
Returns the texture of <name> from the <manager> if it is defined.
==============================================================================*/
{
	struct Cmiss_texture *texture;

	ENTER(Cmiss_texture_manager_get_field);
	if (manager && name)
	{
		texture=FIND_BY_IDENTIFIER_IN_MANAGER(Texture,name)(
			(char *)name, manager);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_texture_manager_get_field.  Invalid argument(s)");
		texture = (struct Cmiss_texture *)NULL;
	}
	LEAVE;

	return (texture);
} /* Cmiss_texture_manager_get_field */

enum Cmiss_texture_combine_mode Cmiss_texture_get_combine_mode(Cmiss_texture_id texture)
/*******************************************************************************
LAST MODIFIED : 24 May 2007

DESCRIPTION :
Returns how the texture is combined with the material: blend, decal or modulate.
==============================================================================*/
{
	enum Texture_combine_mode texture_combine_mode;
	enum Cmiss_texture_combine_mode combine_mode;

	ENTER(Cmiss_texture_get_combine_mode);
	if (texture)
	{
      texture_combine_mode = Texture_get_combine_mode(texture);
      switch(texture_combine_mode)
		{
			case TEXTURE_BLEND:
			{
				combine_mode = CMISS_TEXTURE_COMBINE_BLEND;
			} break;
			case TEXTURE_DECAL:
			{
				combine_mode = CMISS_TEXTURE_COMBINE_DECAL;
			} break;
			case TEXTURE_MODULATE:
			{
				combine_mode = CMISS_TEXTURE_COMBINE_MODULATE;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_texture_get_combine_mode.  "
					"Combine mode not supported in public interface.");
				combine_mode = CMISS_TEXTURE_COMBINE_DECAL;
			} break;
		}
	}
	else
	{
      display_message(ERROR_MESSAGE,
			"Cmiss_texture_get_combine_mode.  Invalid argument(s)");
		combine_mode = CMISS_TEXTURE_COMBINE_DECAL;
	}
	LEAVE;

	return (combine_mode);
} /* Cmiss_texture_get_combine_mode */

int Cmiss_texture_set_combine_mode(Cmiss_texture_id texture,
   enum Cmiss_texture_combine_mode combine_mode)
/*******************************************************************************
LAST MODIFIED : 24 May 2007

DESCRIPTION :
Sets how the texture is combined with the material: blend, decal or modulate.
==============================================================================*/
{
  int return_code;

  ENTER(Cmiss_texture_set_combine_mode);
  if (texture)
  {
	  switch(combine_mode)
	  {
		  case CMISS_TEXTURE_COMBINE_BLEND:
		  {
			  return_code = Texture_set_combine_mode(texture, 
				  TEXTURE_BLEND);
		  } break;
		  case CMISS_TEXTURE_COMBINE_DECAL:
		  {
			  return_code = Texture_set_combine_mode(texture, 
				  TEXTURE_DECAL);
		  } break;
		  case CMISS_TEXTURE_COMBINE_MODULATE:
		  {
			  return_code = Texture_set_combine_mode(texture, 
				  TEXTURE_MODULATE);
		  } break;
		  default:
		  {
			  display_message(ERROR_MESSAGE,
				  "Cmiss_texture_set_combine_mode.  "
				  "Unknown combine mode.");
			  return_code = 0;
		  } break;
	  }
  }
  else
    {
      display_message(ERROR_MESSAGE,
		      "Cmiss_texture_set_combine_mode.  Invalid argument(s)");
      return_code=0;
    }
  LEAVE;

  return (return_code);
} /* Cmiss_texture_set_combine_mode */

enum Cmiss_texture_compression_mode Cmiss_texture_get_compression_mode(
   Cmiss_texture_id texture)
/*******************************************************************************
LAST MODIFIED : 24 May 2007

DESCRIPTION :
==============================================================================*/
{
	enum Texture_compression_mode texture_compression_mode;
	enum Cmiss_texture_compression_mode compression_mode;

	ENTER(Cmiss_texture_get_compression_mode);
	if (texture)
	{
      texture_compression_mode = Texture_get_compression_mode(texture);
      switch(texture_compression_mode)
		{
			case TEXTURE_UNCOMPRESSED:
			{
				compression_mode = CMISS_TEXTURE_COMPRESSION_UNCOMPRESSED;
			} break;
			case TEXTURE_COMPRESSED_UNSPECIFIED:
			{
				compression_mode = CMISS_TEXTURE_COMPRESSION_COMPRESSED_UNSPECIFIED;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_texture_get_compression_mode.  "
					"Compression mode not supported in public interface.");
				compression_mode = CMISS_TEXTURE_COMPRESSION_UNCOMPRESSED;
			} break;
		}
	}
	else
	{
      display_message(ERROR_MESSAGE,
			"Cmiss_texture_get_compression_mode.  Invalid argument(s)");
		compression_mode = CMISS_TEXTURE_COMPRESSION_UNCOMPRESSED;
	}
	LEAVE;

	return (compression_mode);
} /* Cmiss_texture_get_compression_mode */

int Cmiss_texture_set_compression_mode(Cmiss_texture_id texture,
   enum Cmiss_texture_compression_mode compression_mode)
/*******************************************************************************
LAST MODIFIED : 24 May 2007

DESCRIPTION :
Indicate to the graphics hardware how you would like the texture stored in
graphics memory.
==============================================================================*/
{
  int return_code;

  ENTER(Cmiss_texture_set_compression_mode);
  if (texture)
  {
	  switch(compression_mode)
	  {
		  case CMISS_TEXTURE_COMPRESSION_UNCOMPRESSED:
		  {
			  return_code = Texture_set_compression_mode(texture, 
				  TEXTURE_UNCOMPRESSED);
		  } break;
		  case CMISS_TEXTURE_COMPRESSION_COMPRESSED_UNSPECIFIED:
		  {
			  return_code = Texture_set_compression_mode(texture, 
				  TEXTURE_COMPRESSED_UNSPECIFIED);
		  } break;
		  default:
		  {
			  display_message(ERROR_MESSAGE,
				  "Cmiss_texture_set_compression_mode.  "
				  "Unknown compression mode.");
			  return_code = 0;
		  } break;
	  }
  }
  else
    {
      display_message(ERROR_MESSAGE,
		      "Cmiss_texture_set_compression_mode.  Invalid argument(s)");
      return_code=0;
    }
  LEAVE;

  return (return_code);
} /* Cmiss_texture_set_compression_mode */

enum Cmiss_texture_filter_mode Cmiss_texture_get_filter_mode(
   Cmiss_texture_id texture)
/*******************************************************************************
LAST MODIFIED : 24 May 2007

DESCRIPTION :
==============================================================================*/
{
	enum Texture_filter_mode texture_filter_mode;
	enum Cmiss_texture_filter_mode filter_mode;

	ENTER(Cmiss_texture_get_filter_mode);
	if (texture)
	{
      texture_filter_mode = Texture_get_filter_mode(texture);
      switch(texture_filter_mode)
		{
			case TEXTURE_LINEAR_FILTER:
			{
				filter_mode = CMISS_TEXTURE_FILTER_LINEAR;
			} break;
			case TEXTURE_NEAREST_FILTER:
			{
				filter_mode = CMISS_TEXTURE_FILTER_NEAREST;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_texture_get_filter_mode.  "
					"Filter mode not supported in public interface.");
				filter_mode = CMISS_TEXTURE_FILTER_NEAREST;
			} break;
		}
	}
	else
	{
      display_message(ERROR_MESSAGE,
			"Cmiss_texture_get_filter_mode.  Invalid argument(s)");
		filter_mode = CMISS_TEXTURE_FILTER_NEAREST;
	}
	LEAVE;

	return (filter_mode);
} /* Cmiss_texture_get_filter_mode */


int Cmiss_texture_set_filter_mode(Cmiss_texture_id texture,
   enum Cmiss_texture_filter_mode filter_mode)
/*******************************************************************************
LAST MODIFIED : 24 May 2007

DESCRIPTION :
Specfiy how the graphics hardware rasterises the texture onto the screen.
==============================================================================*/
{
  int return_code;

  ENTER(Cmiss_texture_set_filter_mode);
  if (texture)
  {
	  switch(filter_mode)
	  {
		  case CMISS_TEXTURE_FILTER_NEAREST:
		  {
			  return_code = Texture_set_filter_mode(texture, 
				  TEXTURE_NEAREST_FILTER);
		  } break;
		  case CMISS_TEXTURE_FILTER_LINEAR:
		  {
			  return_code = Texture_set_filter_mode(texture, 
				  TEXTURE_LINEAR_FILTER);
		  } break;
		  default:
		  {
			  display_message(ERROR_MESSAGE,
				  "Cmiss_texture_set_filter_mode.  "
				  "Unknown filter mode.");
			  return_code = 0;
		  } break;
	  }
  }
  else
    {
      display_message(ERROR_MESSAGE,
		      "Cmiss_texture_set_filter_mode.  Invalid argument(s)");
      return_code=0;
    }
  LEAVE;

  return (return_code);
} /* Cmiss_texture_set_filter_mode */
