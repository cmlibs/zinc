/***************************************************************************//**
 * FILE : Cmiss_field_image_stream.hpp
 *
 * The private interface to Cmiss_field_image_stream.
 *
 */
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
 * Portions created by the Initial Developer are Copyright (C) 2005-2011
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

#if !defined (CMISS_REGION_STREAM_HPP)
#define CMISS_REGION_STREAM_HPP

//-- extern "C" {
#include "api/cmiss_field.h"
#include "api/cmiss_field_image.h"
#include "general/image_utilities.h"
//-- }
#include "stream/cmiss_stream_private.hpp"

struct Cmiss_stream_information_image : Cmiss_stream_information
{
public:

	Cmiss_stream_information_image(Cmiss_field_image_id image_field_in) :
		image_field(image_field_in)
	{
		Cmiss_field_access(Cmiss_field_image_base_cast(image_field_in));
		image_information = CREATE(Cmgui_image_information)();
	}

	virtual ~Cmiss_stream_information_image()
	{
		Cmiss_field_image_destroy(&image_field);
		DESTROY(Cmgui_image_information)(&image_information);
	}

	Cmgui_image_information *getImageInformation()
	{
		return image_information;
	}

private:
	Cmiss_field_image_id image_field;
	struct Cmgui_image_information *image_information;
};


#endif /* CMISS_REGION_STREAM_HPP */
