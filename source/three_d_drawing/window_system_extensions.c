/*******************************************************************************
FILE : graphics_buffer.cpp

LAST MODIFIED : 10 March 2008

DESCRIPTION :
This provides a Cmgui interface to the window system specific OpenGL binding
(i.e. glx or wgl) extensions.
******************************************************************************/
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

#if defined (WIN32_USER_INTERFACE)
#include <GL/gl.h>
#include <windows.h>
#endif /* defined (WIN32_USER_INTERFACE) */
#include "general/debug.h"

#define WINDOW_SYSTEM_EXTENSIONS_C
#include "three_d_drawing/window_system_extensions.h"

#if defined (WIN32_USER_INTERFACE)
int Window_system_extensions_load_wgl_extension(char *extension_name)
/*******************************************************************************
LAST MODIFIED : 3 October 2007

DESCRIPTION :
Attempts to load a single wgl openGL extension.
If the extension symbol and all the functions used in cmgui from that extension are
found then returns WGLEXTENSION_AVAILABLE.
If the extension list is defined but this extension is not available then
it returns WGLEXTENSION_UNAVAILABLE.
If the extension string is not yet defined then the test is not definitive and
so it returns WGLEXTENSION_UNSURE, allowing the calling procedure to react
appropriately.
==============================================================================*/
{
	USE_PARAMETER(extension_name);
	int return_code = 0;

	/* Could also check wglGetExtensionStringARB but if the functions are available
		I'm going to try and use them */
	if (0)
	{
	}
#if defined WGL_ARB_pixel_format
	else if (!strcmp(extension_name, "WGL_ARB_pixel_format"))
	{
		if (WGLEXTENSION_UNSURE != WGLEXTENSIONFLAG(WGL_ARB_pixel_format))
		{
			return_code = WGLEXTENSIONFLAG(WGL_ARB_pixel_format);
		}
		else
		{
			if ((WINDOW_SYSTEM_EXTENSIONS_ASSIGN_HANDLE(wglGetPixelFormatAttribivARB, PFNWGLGETPIXELFORMATATTRIBIVARBPROC)
					wglGetProcAddress("wglGetPixelFormatAttribivARB")) &&
				(WINDOW_SYSTEM_EXTENSIONS_ASSIGN_HANDLE(wglGetPixelFormatAttribfvARB, PFNWGLGETPIXELFORMATATTRIBFVARBPROC)
					wglGetProcAddress("wglGetPixelFormatAttribfvARB")) &&
				(WINDOW_SYSTEM_EXTENSIONS_ASSIGN_HANDLE(wglChoosePixelFormatARB, PFNWGLCHOOSEPIXELFORMATARBPROC)
					wglGetProcAddress("wglChoosePixelFormatARB")))
			{
				return_code = WGLEXTENSION_AVAILABLE;
			}
			else
			{
				return_code = WGLEXTENSION_UNAVAILABLE;
			}
#if defined (DEBUG)
			printf("Handles for WGL_ARB_pixel_format %p %p %p\n",
				WGLHANDLE(wglGetPixelFormatAttribivARB),
				WGLHANDLE(wglGetPixelFormatAttribfvARB),
				WGLHANDLE(wglChoosePixelFormatARB));
#endif /* defined (DEBUG) */
			WGLEXTENSIONFLAG(WGL_ARB_pixel_format) = return_code;
		}
	}
#endif /* WGL_ARB_pixel_format */
#if defined WGL_ARB_pbuffer
	else if (!strcmp(extension_name, "WGL_ARB_pbuffer"))
	{
		if (WGLEXTENSION_UNSURE != WGLEXTENSIONFLAG(WGL_ARB_pbuffer))
		{
			return_code = WGLEXTENSIONFLAG(WGL_ARB_pbuffer);
		}
		else
		{
			if ((WINDOW_SYSTEM_EXTENSIONS_ASSIGN_HANDLE(wglCreatePbufferARB, PFNWGLCREATEPBUFFERARBPROC)
					wglGetProcAddress("wglCreatePbufferARB")) &&
				(WINDOW_SYSTEM_EXTENSIONS_ASSIGN_HANDLE(wglGetPbufferDCARB, PFNWGLGETPBUFFERDCARBPROC)
					wglGetProcAddress("wglGetPbufferDCARB")) &&
				(WINDOW_SYSTEM_EXTENSIONS_ASSIGN_HANDLE(wglReleasePbufferDCARB, PFNWGLRELEASEPBUFFERDCARBPROC)
					wglGetProcAddress("wglReleasePbufferDCARB")) &&
				(WINDOW_SYSTEM_EXTENSIONS_ASSIGN_HANDLE(wglDestroyPbufferARB, PFNWGLDESTROYPBUFFERARBPROC)
					wglGetProcAddress("wglDestroyPbufferARB")) &&
				(WINDOW_SYSTEM_EXTENSIONS_ASSIGN_HANDLE(wglQueryPbufferARB, PFNWGLQUERYPBUFFERARBPROC)
					wglGetProcAddress("wglQueryPbufferARB")))
			{
				return_code = WGLEXTENSION_AVAILABLE;
			}
			else
			{
				return_code = WGLEXTENSION_UNAVAILABLE;
			}
#if defined (DEBUG)
			printf("Handles for WGL_ARB_pbuffer %p %p %p %p %p\n",
				WGLHANDLE(wglCreatePbufferARB),
				WGLHANDLE(wglGetPbufferDCARB),
				WGLHANDLE(wglReleasePbufferDCARB),
				WGLHANDLE(wglDestroyPbufferARB),
				WGLHANDLE(wglQueryPbufferARB));
#endif /* defined (DEBUG) */
			WGLEXTENSIONFLAG(WGL_ARB_pbuffer) = return_code;
		}
	}
#endif /* WGL_ARB_pbuffer */
	else
	{
		return_code = 0;
	}

	return (return_code);
}
#endif /* defined (WIN32_USER_INTERFACE) */
