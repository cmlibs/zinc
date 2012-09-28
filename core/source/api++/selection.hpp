/***************************************************************************//**
 * FILE : selection.hpp
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
#ifndef __ZN_SELECTION_HPP__
#define __ZN_SELECTION_HPP__

#include "api/cmiss_selection.h"

namespace Zn
{

class SelectionHandler
{
protected:
	Cmiss_selection_handler_id id;

public:

	SelectionHandler() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit SelectionHandler(Cmiss_selection_handler_id in_selection_handler_id) :
		id(in_selection_handler_id)
	{  }

	SelectionHandler(const SelectionHandler& selectionHandler) :
		id(Cmiss_selection_handler_access(selectionHandler.id))
	{  }

	SelectionHandler& operator=(const SelectionHandler& selectionHandler)
	{
		Cmiss_selection_handler_id temp_id = Cmiss_selection_handler_access(selectionHandler.id);
		if (0 != id)
		{
			Cmiss_selection_handler_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~SelectionHandler()
	{
		if (0 != id)
		{
			Cmiss_selection_handler_destroy(&id);
		}
	}

	Cmiss_selection_handler_id getId()
	{
		return id;
	}

	bool getHierarchical()
	{
		return Cmiss_selection_handler_get_hierarchical(id);
	}

	int setHierarchical(bool hierarchicalFlag)
	{
		return Cmiss_selection_handler_set_hierarchical(id, (int)hierarchicalFlag);
	}

};


class SelectionEvent
{
protected:
	Cmiss_selection_event_id id;

public:

	SelectionEvent() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit SelectionEvent(Cmiss_selection_event_id in_selection_event_id) :
		id(in_selection_event_id)
	{  }

	SelectionEvent(const SelectionEvent& selectionEvent) :
		id(Cmiss_selection_event_access(selectionEvent.id))
	{  }

	enum ChangeType
	{
		SELECTION_NO_CHANGE = CMISS_SELECTION_NO_CHANGE,
		SELECTION_CLEAR = CMISS_SELECTION_CLEAR,
		SELECTION_ADD = CMISS_SELECTION_ADD,
		SELECTION_REMOVE = CMISS_SELECTION_REMOVE,
		SELECTION_REPLACE = CMISS_SELECTION_REPLACE,
	};

	SelectionEvent& operator=(const SelectionEvent& selectionEvent)
	{
		Cmiss_selection_event_id temp_id = Cmiss_selection_event_access(selectionEvent.id);
		if (0 != id)
		{
			Cmiss_selection_event_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~SelectionEvent()
	{
		if (0 != id)
		{
			Cmiss_selection_event_destroy(&id);
		}
	}

	Cmiss_selection_event_id getId()
	{
		return id;
	}

	ChangeType getChangeType()
	{
		return static_cast<ChangeType>(Cmiss_selection_event_get_change_type(id));
	}

	int owningRenditionIsDestroyed()
	{
		return Cmiss_selection_event_owning_rendition_is_destroyed(id);
	}

};

}  // namespace Zn

#endif /* __ZN_SELECTION_HPP__ */
