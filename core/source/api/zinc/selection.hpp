/***************************************************************************//**
 * FILE : selection.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_SELECTION_HPP__
#define CMZN_SELECTION_HPP__

#include "zinc/selection.h"

namespace OpenCMISS
{
namespace Zinc
{

class SelectionHandler
{
protected:
	cmzn_selection_handler_id id;

public:

	SelectionHandler() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit SelectionHandler(cmzn_selection_handler_id in_selection_handler_id) :
		id(in_selection_handler_id)
	{  }

	SelectionHandler(const SelectionHandler& selectionHandler) :
		id(cmzn_selection_handler_access(selectionHandler.id))
	{  }

	SelectionHandler& operator=(const SelectionHandler& selectionHandler)
	{
		cmzn_selection_handler_id temp_id = cmzn_selection_handler_access(selectionHandler.id);
		if (0 != id)
		{
			cmzn_selection_handler_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~SelectionHandler()
	{
		if (0 != id)
		{
			cmzn_selection_handler_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_selection_handler_id getId()
	{
		return id;
	}

	bool getHierarchical()
	{
		return (0 != cmzn_selection_handler_get_hierarchical(id));
	}

	int setHierarchical(bool hierarchicalFlag)
	{
		return cmzn_selection_handler_set_hierarchical(id, (int)hierarchicalFlag);
	}

};


class SelectionEvent
{
protected:
	cmzn_selection_event_id id;

public:

	SelectionEvent() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit SelectionEvent(cmzn_selection_event_id in_selection_event_id) :
		id(in_selection_event_id)
	{  }

	SelectionEvent(const SelectionEvent& selectionEvent) :
		id(cmzn_selection_event_access(selectionEvent.id))
	{  }

	enum ChangeType
	{
		SELECTION_NO_CHANGE = CMZN_SELECTION_NO_CHANGE,
		SELECTION_CLEAR = CMZN_SELECTION_CLEAR,
		SELECTION_ADD = CMZN_SELECTION_ADD,
		SELECTION_REMOVE = CMZN_SELECTION_REMOVE,
		SELECTION_REPLACE = CMZN_SELECTION_REPLACE,
	};

	SelectionEvent& operator=(const SelectionEvent& selectionEvent)
	{
		cmzn_selection_event_id temp_id = cmzn_selection_event_access(selectionEvent.id);
		if (0 != id)
		{
			cmzn_selection_event_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~SelectionEvent()
	{
		if (0 != id)
		{
			cmzn_selection_event_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_selection_event_id getId()
	{
		return id;
	}

	ChangeType getChangeType()
	{
		return static_cast<ChangeType>(cmzn_selection_event_get_change_type(id));
	}

	int owningSceneIsDestroyed()
	{
		return cmzn_selection_event_owning_scene_is_destroyed(id);
	}

};

}  // namespace Zinc
}

#endif
