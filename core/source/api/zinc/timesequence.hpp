/***************************************************************************//**
 * FILE : timesequence.hpp
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
#ifndef CMZN_TIMESEQUENCE_HPP__
#define CMZN_TIMESEQUENCE_HPP__

#include "zinc/timesequence.h"

namespace OpenCMISS
{
namespace Zinc
{

class TimeSequence
{
protected:
	cmzn_time_sequence_id id;

public:

	TimeSequence() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit TimeSequence(cmzn_time_sequence_id in_time_sequence_id) :
		id(in_time_sequence_id)
	{  }

	TimeSequence(const TimeSequence& timeSequence) :
		id(cmzn_time_sequence_access(timeSequence.id))
	{  }

	TimeSequence& operator=(const TimeSequence& timeSequence)
	{
		cmzn_time_sequence_id temp_id = cmzn_time_sequence_access(timeSequence.id);
		if (0 != id)
		{
			cmzn_time_sequence_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~TimeSequence()
	{
		if (0 != id)
		{
			cmzn_time_sequence_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_time_sequence_id getId()
	{
		return id;
	}

	int setValue(int timeIndex, double time)
	{
		return cmzn_time_sequence_set_value(id, timeIndex, time);

	}

};

}  // namespace Zinc
}

#endif
