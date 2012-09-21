/***************************************************************************//**
 * FILE : rendition.hpp
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
 * The Original Code is libZinc.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2012
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
#ifndef __CMISS_RENDITION_HPP__
#define __CMISS_RENDITION_HPP__

extern "C" {
#include "api/cmiss_rendition.h"
}
#include "api++/graphic.hpp"
#include "api++/fieldtypesgroup.hpp"
#include "api++/selection.hpp"

namespace Zn
{

class Rendition
{

protected:
	Cmiss_rendition_id id;

public:

	Rendition() : id(NULL)
	{  }

	// takes ownership of C-style rendition reference
	Rendition(Cmiss_rendition_id rendition_id) : id(rendition_id)
	{  }

	Rendition(const Rendition& rendition) : id(Cmiss_rendition_access(rendition.id))
	{  }

	Rendition& operator=(const Rendition& rendition)
	{
		Cmiss_rendition_id temp_id = Cmiss_rendition_access(rendition.id);
		if (NULL != id)
		{
			Cmiss_rendition_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Rendition()
	{
		if (NULL != id)
		{
			Cmiss_rendition_destroy(&id);
		}
	}

	enum GraphicType
	{
		GRAPHIC_TYPE_INVALID = CMISS_GRAPHIC_TYPE_INVALID ,
		GRAPHIC_NODE_POINTS = CMISS_GRAPHIC_NODE_POINTS,
		GRAPHIC_DATA_POINTS = CMISS_GRAPHIC_DATA_POINTS,
		GRAPHIC_LINES = CMISS_GRAPHIC_LINES,
		GRAPHIC_CYLINDERS = CMISS_GRAPHIC_CYLINDERS,
		GRAPHIC_SURFACES = CMISS_GRAPHIC_SURFACES,
		GRAPHIC_ISO_SURFACES = CMISS_GRAPHIC_ISO_SURFACES,
		CMISS_GRAPHIC_ELEMENT_POINTS,
		GRAPHIC_ELEMENT_POINTS = CMISS_GRAPHIC_STREAMLINES,
		GRAPHIC_POINT = CMISS_GRAPHIC_POINT,
	};

	// needed for casting constructors: see RenditionImage(Rendition&)
	Cmiss_rendition_id getId()
	{
		return id;
	}

	int beginChange()
	{
		return Cmiss_rendition_begin_change(id);
	}

	int endChange()
	{
		return Cmiss_rendition_end_change(id);
	}

	Graphic createGraphic(GraphicType graphicType)
	{
		return Graphic(Cmiss_rendition_create_graphic(id,
			static_cast<Cmiss_graphic_type>(graphicType)));
	}

	SelectionHandler createSelectionHandler()
	{
		return SelectionHandler(Cmiss_rendition_create_selection_handler(id));
	}

	/***************************************************************************//**
	 * Returns the graphic of the specified name from the rendition. Beware that
	 * graphics in the same rendition may have the same name and this function will
	 * only return the first graphic found with the specified name;
	 *
	 * @param rendition  Rendition in which to find the graphic.
	 * @param graphic_name  The name of the graphic to find.
	 * @return  New reference to graphic of specified name, or NULL if not found.
	 */
	Graphic findGraphicByName(const char *graphicName)
	{
		return Graphic(Cmiss_rendition_find_graphic_by_name(id, graphicName));
	}

	Graphic getFirstGraphic()
	{
		return Graphic(Cmiss_rendition_get_first_graphic(id));
	}

	Graphic getNextGraphic(Graphic& refGraphic)
	{
		return Graphic(Cmiss_rendition_get_next_graphic(id, refGraphic.getId()));
	}

	Graphic getPreviousGraphic(Graphic& refGraphic)
	{
		return Graphic(Cmiss_rendition_get_previous_graphic(id, refGraphic.getId()));
	}

	int getNumberOfGraphics()
	{
		return Cmiss_rendition_get_number_of_graphics(id);
	}

	FieldGroup getSelectionGroup()
	{
		return FieldGroup(Cmiss_rendition_get_selection_group(id));
	}

	int setSelectionGroup(FieldGroup& fieldGroup)
	{
		return Cmiss_rendition_set_selection_group(id,
			(Cmiss_field_group_id)(fieldGroup.getId()));
	}

	int getVisibilityFlag()
	{
		return Cmiss_rendition_get_visibility_flag(id);
	}

	int setVisibilityFlag(int visibilityFlag)
	{
		return Cmiss_rendition_set_visibility_flag(id, visibilityFlag);
	}

	int moveGraphicBefore(Graphic& graphic, Graphic& refGraphic)
	{
		return Cmiss_rendition_move_graphic_before(id, graphic.getId(), refGraphic.getId());
	}

	int removeAllGraphics()
	{
		return Cmiss_rendition_remove_all_graphics(id);
	}

	int removeGraphic(Graphic& graphic)
	{
		return Cmiss_rendition_remove_graphic(id, graphic.getId());
	}

	int executeCommand(const char *command_string)
	{
		return Cmiss_rendition_execute_command(id, command_string);
	}
};

} // namespace Cmiss

#endif /* __CMISS_RENDITION_HPP__ */
