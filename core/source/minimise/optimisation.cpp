/***************************************************************************//**
 * @file optimisation.cpp
 *
 * Implementation of Minimisation object for performing optimisation algorithm
 * from description in Cmiss_optimisation.
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

//-- extern "C" {
	#include <stdio.h>
	#include <math.h>
	#include "api/cmiss_field.h"
	#include "api/cmiss_field_module.h"
	#include "computed_field/computed_field.h"
	#include "computed_field/computed_field_composite.h"
	#include "computed_field/computed_field_set.h"
	#include "computed_field/computed_field_finite_element.h"
	#include "finite_element/finite_element.h"
	#include "finite_element/finite_element_private.h"
	#include "finite_element/finite_element_region.h"
	#include "finite_element/finite_element_region_private.h"
	#include "general/any_object_private.h"
	#include "general/any_object_definition.h"
	#include "general/callback_private.h"
	#include "general/compare.h"
	#include "general/debug.h"
	#include "general/indexed_list_private.h"
	#include "general/object.h"
	#include "time/time_keeper.h"
	#include "general/message.h"
//-- }
#include "computed_field/computed_field_private.hpp"
#include "minimise/optimisation.hpp"
#include "general/enumerator_private.hpp"
#include "mesh/cmiss_element_private.hpp"
#include "computed_field/field_module.hpp"
#include <iostream>
#include <sstream>
using namespace std;

// OPT++ includes and namespaces
#include <LSQNLF.h>
#include <NLF.h>
#include <NLP.h>
#include <OptQNewton.h>
#include <OptNewton.h>
using NEWMAT::ColumnVector;
using namespace ::OPTPP;

// global variable needed to pass minimisation object to Opt++ init functions.
static void* GlobalVariableMinimisation = NULL;

int ObjectiveFieldData::prepareTerms()
{
	Cmiss_field_module_id field_module = Cmiss_field_get_field_module(field);
	Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
	numTerms = field->get_number_of_sum_square_terms(*field_cache);
	Cmiss_field_cache_destroy(&field_cache);
	Cmiss_field_module_destroy(&field_module);
	bufferSize = numComponents;
	if (numTerms > 0)
		bufferSize *= numTerms;
	buffer = new FE_value[bufferSize];
	return (0 != buffer);
}

Minimisation::~Minimisation()
{
	delete[] objectiveValues;
	if (dof_storage_array) DEALLOCATE(dof_storage_array);
	if (dof_initial_values) DEALLOCATE(dof_initial_values);
	Cmiss_field_cache_destroy(&field_cache);
	Cmiss_field_module_destroy(&field_module);
	for (FieldVector::iterator iter = independentFields.begin();
		iter != independentFields.end(); ++iter)
	{
		Cmiss_field_destroy(&(*iter));
	}
	for (ObjectiveFieldDataVector::iterator iter = objectiveFields.begin();
		iter != objectiveFields.end(); ++iter)
	{
		delete *iter;
	}
}

int Minimisation::prepareOptimisation()
{
	int return_code = 1;
	Cmiss_field_module_begin_change(field_module);
	if (optimisation.independentFields.size() != independentFields.size())
		return_code = 0;
	if (optimisation.objectiveFields.size() != objectiveFields.size())
		return_code = 0;
	return_code = return_code && construct_dof_arrays();
	if (optimisation.method == CMISS_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON)
	{
		totalLeastSquaresTerms = 0;
		for (ObjectiveFieldDataVector::iterator iter = objectiveFields.begin();
			iter != objectiveFields.end(); ++iter)
		{
			ObjectiveFieldData *objective = *iter;
			if (!objective->prepareTerms())
			{
				return_code = 0;
				break;
			}
			totalLeastSquaresTerms += objective->bufferSize;
		}
	}
	if (!return_code)
	{
		display_message(ERROR_MESSAGE, "Minimisation::prepareOptimisation() Failed");
	}
	Cmiss_field_module_end_change(field_module);
	return return_code;
}

/***************************************************************************//**
 * Ensures independent fields are marked as changed, so graphics update
 */
void Minimisation::touch_independent_fields()
{
	for (FieldList::iterator iter = optimisation.independentFields.begin();
		iter != optimisation.independentFields.end(); ++iter)
	{
		Computed_field_changed(*iter);
	}
}

int Minimisation::runOptimisation()
{
	Cmiss_field_module_begin_change(field_module);
	// Minimise the objective function
	int return_code = 0;
	switch (optimisation.method)
	{
	case CMISS_OPTIMISATION_METHOD_QUASI_NEWTON:
		return_code = minimise_QN();
		break;
	case CMISS_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON:
		return_code = minimise_LSQN();
		break;
	default:
		display_message(ERROR_MESSAGE, "Cmiss_optimisation::runOptimisation. "
			"Unknown minimisation method.");
		break;
	}
	touch_independent_fields();
	Cmiss_field_module_end_change(field_module);
	if (!return_code)
	{
		display_message(ERROR_MESSAGE, "Minimisation::runOptimisation() Failed");
	}
	return return_code;
}

/***************************************************************************//**
 *  Populates the array of pointers to the dof values and the array of the dof
 *  initial values. Notice the population includes both nodal values and
 *  derivatives but does NOT handle versions - this needs to be added by someone
 *  who understands and can test versions.
 */
int Minimisation::construct_dof_arrays()
{
	int return_code = 1;
	if (dof_storage_array)
	{
		DEALLOCATE(dof_storage_array);
		dof_storage_array = 0;
	}
	if (dof_initial_values)
	{
		DEALLOCATE(dof_initial_values);
		dof_initial_values = 0;
	}
	total_dof = 0;
	for (FieldVector::iterator iter = independentFields.begin();
		iter != independentFields.end(); ++iter)
	{
		Cmiss_field *independentField = *iter;
		int number_of_components = Cmiss_field_get_number_of_components(independentField);
		if (Computed_field_is_type_finite_element(independentField))
		{
			// should only have one independent field
			FE_field *fe_field;
			Computed_field_get_type_finite_element(independentField, &fe_field);
			Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
			Cmiss_node_iterator_id iterator = Cmiss_nodeset_create_node_iterator(nodeset);
			Cmiss_node_id node = 0;
			while ((0 != (node = Cmiss_node_iterator_next_non_access(iterator))) && return_code)
			{
				if (FE_field_is_defined_at_node(fe_field, node))
				{
					for (int component_number = 0; component_number < number_of_components; component_number++)
					{
						int number_of_values = 1 + get_FE_node_field_component_number_of_derivatives(node,
							fe_field, component_number);
						int number_of_versions = get_FE_node_field_component_number_of_versions(node,
							fe_field, component_number);
						int total_number_of_values = number_of_versions*number_of_values;

						// FIXME: tmp remove derivatives
						//number_of_values = 1;

						enum FE_nodal_value_type *nodal_value_types =
							get_FE_node_field_component_nodal_value_types(node, fe_field, component_number);
						if (nodal_value_types)
						{
							FE_value **temp_dof_storage_array;
							if (REALLOCATE(temp_dof_storage_array, dof_storage_array, FE_value *, total_dof + total_number_of_values))
							{
								dof_storage_array = temp_dof_storage_array;
							}
							else
							{
								return_code = 0;
							}
							FE_value *temp_dof_initial_values;
							if (REALLOCATE(temp_dof_initial_values, dof_initial_values, FE_value, total_dof + total_number_of_values))
							{
								dof_initial_values = temp_dof_initial_values;
							}
							else
							{
								return_code = 0;
							}
							for (int version_number = 0; (version_number < number_of_versions) && return_code; ++version_number)
							{
								for (int i = 0; i < number_of_values; i++)
								{
									if (get_FE_nodal_FE_value_storage(node, fe_field, component_number,
										version_number, /*nodal_value_type*/nodal_value_types[i],
										current_time, &(dof_storage_array[total_dof])))
									{
										// get initial value from value storage pointer
										dof_initial_values[total_dof] = *dof_storage_array[total_dof];
										/*cout << dof_storage_array[total_dof - 1] << "   "
												<< dof_initial_values[total_dof - 1] << endl;*/
										total_dof++;
									}
									else
									{
										display_message(ERROR_MESSAGE, "Cmiss_optimisation::construct_dof_arrays. "
											"get_FE_nodal_FE_value_storage failed.");
										return_code = 0;
										break;
									}
								}
							}
							DEALLOCATE(nodal_value_types);
						}
					}
				}
			}
			Cmiss_node_iterator_destroy(&iterator);
			Cmiss_nodeset_destroy(&nodeset);
		}
		else if (Computed_field_is_constant(independentField))
		{
			FE_value *constant_values_storage = Computed_field_constant_get_values_storage(independentField);
			if (constant_values_storage)
			{
				REALLOCATE(dof_storage_array, dof_storage_array,
					FE_value *, total_dof + number_of_components);
				REALLOCATE(dof_initial_values, dof_initial_values,
					FE_value, total_dof + number_of_components);
				for (int component_number = 0; component_number < number_of_components; component_number++)
				{
					dof_storage_array[total_dof] = constant_values_storage + component_number;
					dof_initial_values[total_dof] = *dof_storage_array[total_dof];
					/*cout << dof_storage_array[total_dof] << "   "
							<< dof_initial_values[total_dof] << endl;*/
					total_dof++;
				}
			}
			else
			{
				char *field_name = Cmiss_field_get_name(independentField);
				display_message(WARNING_MESSAGE, "Minimisation::construct_dof_arrays.  "
					"Independent field '%s' is not a constant. Skipping.", field_name);
				DEALLOCATE(field_name);
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Cmiss_optimisation::construct_dof_arrays. "
				"Invalid independent field type.");
			return_code = 0;
		}
	}
	return return_code;
}

/***************************************************************************//**
 * Simple function to list the dof values. This is mainly for debugging purposes
 * and may be removed later.
 */
void Minimisation::list_dof_values()
{
	for (int i = 0; i < total_dof; i++) {
		cout << "dof[" << i << "] = " << *dof_storage_array[i] << endl;
	}
}

/***************************************************************************//**
 * Must call this function after updating independent field DOFs to ensure
 * dependent field caches are fully recalculated with the DOF changes.
 */
void Minimisation::invalidate_independent_field_caches()
{
	for (FieldVector::iterator iter = independentFields.begin();
		iter != independentFields.end(); ++iter)
	{
		Cmiss_field_id independentField = *iter;
		independentField->clearCaches();
	}
}

/***************************************************************************//**
 * Evaluates the scalar objective function value given the current DOF values.
 * Equals sum of all objective field components.
 */
int Minimisation::evaluate_objective_function(FE_value *valueAddress)
{
	int return_code = 1;
	*valueAddress = 0.0;
	invalidate_independent_field_caches();
	int offset = 0;
	for (ObjectiveFieldDataVector::iterator iter = objectiveFields.begin();
		iter != objectiveFields.end(); ++iter)
	{
		ObjectiveFieldData *objective = *iter;
		if (!Cmiss_field_evaluate_real(objective->field, field_cache,
			objective->numComponents, objectiveValues + offset))
		{
			display_message(ERROR_MESSAGE, "Failed to evaluate objective field %s", objective->field->name);
			return_code = 0;
			break;
		}
		offset += objective->numComponents;
	}
	for (int i = 0; i < totalObjectiveFieldComponents; ++i)
	{
		*valueAddress += objectiveValues[i];
	}
	return return_code;
}

/***************************************************************************//**
 * One time initialisation code required by the Opt++ quasi-Newton and least-
 * squares quasi-Newton minimisation algorithms.
 * Copies initial DOF values.
 */
static void init_dof_initial_values(int ndim, ColumnVector& x)
{
	int i;
	Minimisation* minimisation = static_cast<Minimisation*> (GlobalVariableMinimisation);
	FE_value *dof_initial_values = minimisation->get_dof_initial_values();
	for (i = 0; i < ndim; i++)
	{
		x(i+1) = static_cast<double>(dof_initial_values[i]);
	}
}

/***************************************************************************//**
 * The objective function for the Opt++ quasi-Newton minimisation.
 */
void objective_function_QN(int ndim, const ColumnVector& x, double& fx,
		int& result)
{
	int i;
	Minimisation* minimisation = static_cast<Minimisation*> (GlobalVariableMinimisation);
	// ColumnVector's index'd from 1...
	for (i = 0; i < ndim; i++)
	{
		minimisation->set_dof_value(i, x(i + 1));
	}
	//minimisation->list_dof_values();

	FE_value objectiveFunctionValue = 0.0;
	minimisation->evaluate_objective_function(&objectiveFunctionValue);
	fx = static_cast<double>(objectiveFunctionValue);
	//cout << "Objective Value = " << fx << endl;
	result = NLPFunction;
}

/***************************************************************************//**
 * Naive wrapper around a quasi-Newton minimisation.
 */
int Minimisation::minimise_QN()
{
	char message[] = { "Solution from quasi-newton" };
	// need a handle on this object...
	// FIXME: need to find and use "user data" in the Opt++ methods.
	GlobalVariableMinimisation = static_cast<void*> (this);

	FDNLF1 nlp(total_dof, objective_function_QN, init_dof_initial_values);
	OptQNewton objfcn(&nlp);
	objfcn.setSearchStrategy(LineSearch);
	objfcn.setFcnTol(optimisation.functionTolerance);
	objfcn.setGradTol(optimisation.gradientTolerance);
	objfcn.setStepTol(optimisation.stepTolerance);
	objfcn.setMaxIter(optimisation.maximumIterations);
	objfcn.setMaxFeval(optimisation.maximumNumberFunctionEvaluations);
	objfcn.setMaxStep(optimisation.maximumStep);
	objfcn.setMinStep(optimisation.minimumStep);
	objfcn.setLineSearchTol(optimisation.linesearchTolerance);
	objfcn.setMaxBacktrackIter(optimisation.maximumBacktrackIterations);
	/* @todo Add in support for trust region methods
	objfcn.setTRSize(optimisation.trustRegionSize);
	*/

	// send Opt++ log text to string buffer
	if (!objfcn.setOutputFile(optppMessageStream))
		cerr << "main: output file open failed" << endl;
	objfcn.optimize();
	objfcn.printStatus(message);
	objfcn.cleanup();

	ColumnVector solution = nlp.getXc();
	int i;
	for (i = 0; i < total_dof; i++)
		this->set_dof_value(i, solution(i + 1));
	//list_dof_values();
	return 1;
}

/***************************************************************************//**
 * The objective function for the Opt++ least-squares quasi-Newton minimisation.
 */
void objective_function_LSQ(int ndim, const ColumnVector& x, ColumnVector& fx,
		int& result, void* iterationCounterVoid)
{
	//int* iterationCounter = static_cast<int*>(iterationCounterVoid);
	//std::cout << "objective function called " << ++(*iterationCounter) << " times." << std::endl;
	USE_PARAMETER(iterationCounterVoid);
	int i;
	Minimisation* minimisation = static_cast<Minimisation*> (GlobalVariableMinimisation);
	// ColumnVector's index'd from 1...
	for (i = 0; i < ndim; i++)
	{
		minimisation->set_dof_value(i, x(i + 1));
	}
	//minimisation->list_dof_values();
	minimisation->invalidate_independent_field_caches();
	int return_code = 1;
	Field_time_location location;
	int termIndex = 0;
	for (ObjectiveFieldDataVector::iterator iter = minimisation->objectiveFields.begin();
		iter != minimisation->objectiveFields.end(); ++iter)
	{
		ObjectiveFieldData *objective = *iter;
		const int bufferSize = objective->bufferSize;
		FE_value *buffer = objective->buffer;
		if (objective->numTerms > 0)
			return_code = objective->field->evaluate_sum_square_terms(*(minimisation->field_cache), bufferSize, buffer);
		else
			return_code = Cmiss_field_evaluate_real(objective->field, minimisation->field_cache, objective->bufferSize, objective->buffer);
		if (!return_code)
		{
			// GRC: should record failure properly
			display_message(ERROR_MESSAGE, "Failed to evaluate least squares terms for objective field %s", objective->field->name);
			break;
		}
		for (i = 0; i < bufferSize; ++i)
			fx.element(termIndex++) = buffer[i];
	}
	result = NLPFunction;
}

/***************************************************************************//**
 * Least-squares minimisation using Opt++
 */
int Minimisation::minimise_LSQN()
{
	int iterationCounter = 0;
	char message[] = { "Solution from newton least squares" };
	// need a handle on this object...
	GlobalVariableMinimisation = static_cast<void*> (this);
	LSQNLF nlp(total_dof, totalLeastSquaresTerms,
		objective_function_LSQ, init_dof_initial_values, (OPTPP::INITCONFCN)NULL,
		(void*)(&iterationCounter));
	OptNewton objfcn(&nlp);
	objfcn.setSearchStrategy(LineSearch);
	// send Opt++ log text to string buffer
	if (!objfcn.setOutputFile(optppMessageStream))
		cerr << "main: output file open failed" << endl;
	objfcn.setFcnTol(optimisation.functionTolerance);
	objfcn.setGradTol(optimisation.gradientTolerance);
	objfcn.setStepTol(optimisation.stepTolerance);
	objfcn.setMaxIter(optimisation.maximumIterations);
	objfcn.setMaxFeval(optimisation.maximumNumberFunctionEvaluations);
	objfcn.setMaxStep(optimisation.maximumStep);
	objfcn.setMinStep(optimisation.minimumStep);
	objfcn.setLineSearchTol(optimisation.linesearchTolerance);
	objfcn.setMaxBacktrackIter(optimisation.maximumBacktrackIterations);
	/* @todo Add in support for trust region methods
	objfcn.setTRSize(optimisation.trustRegionSize);
	*/
	//nlp.setIsExpensive(false);
	//nlp.setDebug();
	objfcn.optimize();
	objfcn.printStatus(message);
	ColumnVector solution = nlp.getXc();
	int i;
	for (i = 0; i < total_dof; i++)
		this->set_dof_value(i, solution(i + 1));
	//list_dof_values();
	return 1;
}

