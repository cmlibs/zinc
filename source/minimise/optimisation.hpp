/**
 * @file optimisation.hpp
 *
 * The private methods for the Cmiss_optimisation object. Required to allow creation
 * of optimisation objects.
 *
 * @see-also api/cmiss_optimisation.h
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

#ifndef OPTIMISATION_HPP_
#define OPTIMISATION_HPP_

#include <vector>

extern "C"
{
	#include "api/cmiss_optimisation.h"
}

typedef std::vector<Cmiss_field_id> FieldList;

struct Cmiss_optimisation
{
private:
	~Cmiss_optimisation()
	{
		if (objectiveField) Cmiss_field_destroy(&objectiveField);
		if (meshField) Cmiss_field_destroy(&meshField);
		if (dataField) Cmiss_field_destroy(&dataField);
		size_t i;
		for (i=0;i<independentFields.size();i++)
		{
			Cmiss_field_destroy(&(independentFields[i]));
		}
		if (meshRegion) Cmiss_region_destroy(&meshRegion);
		if (dataRegion) Cmiss_region_destroy(&dataRegion);
	}

public:
	int dimension;
	int maximumIterations;
	int maximumNumberFunctionEvaluations;
	double absoluteTolerance;
	double relativeTolerance;
	enum Cmiss_optimisation_method method;
	Cmiss_field_id objectiveField;
	Cmiss_field_id meshField;
	Cmiss_field_id dataField;
	FieldList independentFields;
	bool independentFieldsConstant;
	Cmiss_region_id meshRegion;
	Cmiss_region_id dataRegion;

	Cmiss_optimisation()
	{
		// initialise to default values
		dimension = 2;
		maximumIterations = 100;
		maximumNumberFunctionEvaluations = 1000;
		absoluteTolerance = 1.0e-8;
		relativeTolerance = 1.0e-8;
		method = CMISS_OPTIMISATION_METHOD_QUASI_NEWTON;
		objectiveField = NULL;
		meshField = NULL;
		dataField = NULL;
		independentFieldsConstant = false;
		meshRegion = NULL;
		dataRegion = NULL;
	}

	static int destroy(Cmiss_optimisation_id &optimisation)
	{
		if (!optimisation) return 0;
		delete optimisation;
		optimisation = 0;
		return 1;
	}

	int runOptimisation();
};

#endif /* OPTIMISATION_HPP_ */
