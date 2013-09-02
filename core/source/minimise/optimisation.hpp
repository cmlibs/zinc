/***************************************************************************//**
 * @file optimisation.hpp
 *
 * Minimisation object for performing optimisation algorithm from description
 * in cmzn_optimisation.
 *
 * @see-also api/zinc/optimisation.h
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
#include "minimise/cmiss_optimisation_private.hpp"

class ObjectiveFieldData
{
public:
	cmzn_field_id field;
	int numComponents;
	int numTerms;
	int bufferSize;
	FE_value *buffer;

	ObjectiveFieldData(cmzn_field_id objectiveField) :
		field(cmzn_field_access(objectiveField)),
		numComponents(cmzn_field_get_number_of_components(field)),
		numTerms(0),
		bufferSize(0),
		buffer(0)
	{
	}

	~ObjectiveFieldData()
	{
		cmzn_field_destroy(&field);
		delete[] buffer;
	}

	int prepareTerms();
};

typedef std::vector<cmzn_field_id> FieldVector;
typedef std::vector<ObjectiveFieldData*> ObjectiveFieldDataVector;

class Minimisation
{
private:
	cmzn_optimisation& optimisation;

public:
	cmzn_field_module_id field_module;
	cmzn_field_cache_id field_cache;
	FE_value current_time;
	int total_dof;
	ObjectiveFieldDataVector objectiveFields;

private:
	FE_value **dof_storage_array;
	FE_value *dof_initial_values;
	FieldVector independentFields;
	int totalObjectiveFieldComponents;
	int totalLeastSquaresTerms;
	FE_value *objectiveValues;
	std::ostream optppMessageStream;

public:
	Minimisation(cmzn_optimisation& optimisation) :
		optimisation(optimisation),
		field_module(cmzn_field_module_access(optimisation.fieldModule)),
		field_cache(cmzn_field_module_create_cache(field_module)),
		current_time(0.0),
		total_dof(0),
		dof_storage_array(0),
		dof_initial_values(0),
		optppMessageStream(&optimisation.solution_report)
	{
		for (FieldList::iterator iter = optimisation.independentFields.begin();
			iter != optimisation.independentFields.end(); ++iter)
		{
			cmzn_field_access(*iter);
			independentFields.push_back(*iter);
		}
		totalObjectiveFieldComponents = 0;
		for (FieldList::iterator iter = optimisation.objectiveFields.begin();
			iter != optimisation.objectiveFields.end(); ++iter)
		{
			cmzn_field_id objectiveField = *iter;
			totalObjectiveFieldComponents += cmzn_field_get_number_of_components(objectiveField);
			objectiveFields.push_back(new ObjectiveFieldData(objectiveField));
		}
		objectiveValues = new FE_value[totalObjectiveFieldComponents];
		totalLeastSquaresTerms = 0;
	};

	~Minimisation();

	/** Prepare data structures for optimisation
	 * @return  1 on success, 0 on failure */
	int prepareOptimisation();

	/** Perform optimisation until ending condition met or error
	 * Must have successfully called prepareOptimisation() first.
	 * @return  1 on success, 0 on failure */
	int runOptimisation();

	FE_value *get_dof_initial_values()
	{
		return dof_initial_values;
	}

	inline void set_dof_value(int dof_index, FE_value new_value)
	{
		*dof_storage_array[dof_index] = new_value;
	}

	void list_dof_values();

	void invalidate_independent_field_caches();

	/** @return  1 on success, 0 on failure */
	int evaluate_objective_function(FE_value *valueAddress);

private:

	int construct_dof_arrays();

	void touch_independent_fields();

	int minimise_QN();

	int minimise_LSQN();

};

#endif /* OPTIMISATION_HPP_ */
