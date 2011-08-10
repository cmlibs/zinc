/**
 * @file optimisation.cpp
 *
 * The private methods for the Cmiss_optimisation object.
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

extern "C" {
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
	#include "region/cmiss_region_private.h"
	#include "time/time_keeper.h"
	#include "user_interface/message.h"
}
#include "minimise/optimisation.hpp"
#include "general/enumerator_private_cpp.hpp"
#include "mesh/cmiss_element_private.hpp"
#include "computed_field/field_module.hpp"
#include <iostream>
#include <sstream>
#include <map>
using namespace std;

// OPT++ includes and namespaces
#include <LSQNLF.h>
#include <NLF.h>
#include <NLP.h>
#include <OptQNewton.h>
#include <OptNewton.h>
using NEWMAT::ColumnVector;
using namespace ::OPTPP;

static void* UserData = NULL;

void display_chunked_message(std::string& message)
{
	unsigned int i,chunkSize=512;
	for (i=0;i<message.length();i+=chunkSize)
	{
		std::string m = message.substr(i,chunkSize);
		display_message(INFORMATION_MESSAGE,"%s",m.c_str());
	}
}

typedef std::pair<ColumnVector*,class Minimisation*> UDP;

struct Projection {
	FE_value_tuple xi;
	struct FE_element* element;
};

class Minimisation {

public:
	struct Cmiss_optimisation* optimisation;
	FE_region *fe_region;
	Cmiss_region_id meshRegion;
	Cmiss_field_module_id field_module;
	Cmiss_field_cache_id field_cache;
	FE_value current_time;
	int total_dof;
	int number_of_data_components,number_of_data_points;
	int currentDataPointIndex;
	std::map< const FE_node*, Projection > projections;

	Minimisation(struct Cmiss_optimisation* optimisation) :
		optimisation(optimisation)
	{
		fe_region = Cmiss_mesh_get_FE_region(optimisation->feMesh);
		// the meshRegion is not accessed, so make sure not to destroy.
		FE_region_get_Cmiss_region(fe_region, &meshRegion);
		field_module = Cmiss_region_get_field_module(meshRegion);
		field_cache = Cmiss_field_module_create_cache(field_module);
		current_time = 0.0;
		total_dof = 0;
		number_of_data_components = 0;
		number_of_data_points = 0;
		dof_storage_array = static_cast<FE_value**> (NULL);
		dof_initial_values = static_cast<FE_value*> (NULL);
		Cmiss_field_module_begin_change(field_module);
	};

	~Minimisation();

private:

	FE_value **dof_storage_array;

public:
	// FIXME: andre tmp make public until function pointers sorted out.
	FE_value *dof_initial_values;

	static int construct_dof_arrays(struct FE_node *node,
			void *minimisation_object_void);

	int calculate_projection(Cmiss_node_id node);

	inline void set_dof_value(int dof_index, FE_value new_value)
	{
		*dof_storage_array[dof_index] = new_value;
	}

	void list_dof_values();

	void invalidate_independent_field_caches();

	FE_value evaluate_objective_function();

	void touch_constant_independent_fields();

	void minimise_NSDGSL();
	void minimise_QN();
	void minimise_LSQN();

	// FIXME: should allow for different types of projection calculations?
	void calculate_data_projections();

};

Minimisation::~Minimisation()
/*******************************************************************************
 LAST MODIFIED : 24 August 2006

 DESCRIPTION :
 Clear the type specific data used by this type.
 ==============================================================================*/
{
	ENTER(Minimisation::~Minimisation);

	if (dof_storage_array) DEALLOCATE(dof_storage_array);
	if (dof_initial_values) DEALLOCATE(dof_initial_values);
	Cmiss_field_cache_destroy(&field_cache);
	Cmiss_field_module_end_change(field_module);
	Cmiss_field_module_destroy(&field_module);
	LEAVE;
} /* Minimisation::~Minimisation */

int Minimisation::calculate_projection(Cmiss_node_id node)
{
	FE_value* values = 0;
	ALLOCATE(values, FE_value, number_of_data_components);
	Projection projection = projections[node];
	Computed_field_evaluate_at_node(optimisation->dataField, node, current_time, values);
	Computed_field_find_element_xi(optimisation->meshField,
		values, number_of_data_components, current_time,
		&(projection.element), projection.xi,
		/*element_dimension*/Cmiss_mesh_get_dimension(optimisation->feMesh),
		/*search_region*/meshRegion, /*propagate_field*/0,
		/*find_nearest_location*/1);
	DEALLOCATE(values);
	projections[node] = projection;
	//std::cout << "Element: " << FE_element_get_cm_number(projection.element) << ";";
	//std::cout << " xi: " << projection.xi[0] << "," << projection.xi[1] << ",";
	//std::cout << projection.xi[2] << std::endl;
	return 1;
}

void Minimisation::calculate_data_projections()
{
	Cmiss_node_iterator_id iterator = Cmiss_nodeset_create_node_iterator(optimisation->dataNodeset);
	Cmiss_node_id node = Cmiss_node_iterator_next(iterator);
	// FIXME: must be a better way to set the search region for the find_element_xi.
	int code = 1;
	while (node && code)
	{
		code = calculate_projection(node);
		Cmiss_node_destroy(&node);
		if (code) node = Cmiss_node_iterator_next(iterator);
	}
	Cmiss_node_iterator_destroy(&iterator);
	//std::cout << "Minimisation::calculate_data_projections" << std::endl;
}

int Minimisation::construct_dof_arrays(struct FE_node *node,
		void *minimisation_object_void)
/*******************************************************************************
 LAST MODIFIED : 7 June 2007

 DESCRIPTION :
 Populates the array of pointers to the dof values and the array of the dof
 initial values. Notice the population includes both nodal values and derivatives
 but does NOT handle versions - this needs to be added by someone who understands
 and can test versions.
 ==============================================================================*/
{
	enum FE_nodal_value_type *nodal_value_types;
	FE_field *fe_field;
	int i, component_number, return_code, number_of_components,
			number_of_values;
	Minimisation *minimisation;

	ENTER(Minimisation::construct_dof_arrays);
	minimisation = static_cast<Minimisation *> (minimisation_object_void);
	if (minimisation != 0)
	{
		if (node)
		{
			// should only have one independent field
			Computed_field_get_type_finite_element(minimisation->optimisation->independentFields[0],
					&fe_field);
			//Get number of components
			number_of_components = get_FE_field_number_of_components(fe_field);
			//for each component in field
			for (component_number = 0; component_number < number_of_components; component_number++)
			{
				number_of_values = 1 + get_FE_node_field_component_number_of_derivatives(node,
						fe_field, component_number);

				// FIXME: tmp remove derivatives
				//number_of_values = 1;

				nodal_value_types = get_FE_node_field_component_nodal_value_types(
						node, fe_field, component_number);
				for (i = 0; i < number_of_values; i++)
				{
					// increment total_dof
					minimisation->total_dof++;
					// reallocate arrays
					REALLOCATE(minimisation->dof_storage_array, minimisation->dof_storage_array,
							FE_value *, minimisation->total_dof);
					REALLOCATE(minimisation->dof_initial_values, minimisation->dof_initial_values,
							FE_value, minimisation->total_dof);
					// get value storage pointer
					// FIXME: need to handle versions for the femur mesh?
					get_FE_nodal_FE_value_storage(
							node,fe_field,/*component_number*/component_number,
							/*version_number*/ 0, /*nodal_value_type*/nodal_value_types[i],
							minimisation->current_time,
							&(minimisation->dof_storage_array[minimisation->total_dof - 1]));
					// get initial value from value storage pointer
					minimisation->dof_initial_values[minimisation->total_dof - 1]
							= *minimisation->dof_storage_array[minimisation->total_dof
									- 1];
					/*cout << minimisation->dof_storage_array[minimisation->total_dof
							- 1] << "   "
							<< minimisation->dof_initial_values[minimisation->total_dof
									- 1] << endl;*/
				}
				DEALLOCATE(nodal_value_types);
			}
		}
		else
		{
			// constant field(s) - could have many
			size_t i;
			for (i=0;i<minimisation->optimisation->independentFields.size();i++)
			{
				number_of_components = Cmiss_field_get_number_of_components(
						minimisation->optimisation->independentFields[i]);
				FE_value *constant_values_storage =
					Computed_field_constant_get_values_storage(minimisation->optimisation->independentFields[i]);
				if (constant_values_storage)
				{
					// reallocate arrays
					REALLOCATE(minimisation->dof_storage_array, minimisation->dof_storage_array,
						FE_value *, minimisation->total_dof + number_of_components);
					REALLOCATE(minimisation->dof_initial_values, minimisation->dof_initial_values,
						FE_value, minimisation->total_dof + number_of_components);
					for (component_number = 0; component_number < number_of_components; component_number++)
					{
						// get value storage pointer
						minimisation->dof_storage_array[minimisation->total_dof] =
							constant_values_storage + component_number;
						// get initial value from value storage pointer
						minimisation->dof_initial_values[minimisation->total_dof] =
							*minimisation->dof_storage_array[minimisation->total_dof];
						/*cout << minimisation->dof_storage_array[minimisation->total_dof] << "   "
								<< minimisation->dof_initial_values[minimisation->total_dof] << endl;*/
						minimisation->total_dof++;
					}
				}
				else
				{
					char *field_name = Cmiss_field_get_name(minimisation->optimisation->independentFields[i]);
					display_message(WARNING_MESSAGE, "Minimisation::construct_dof_arrays.  "
						"Independent field '%s' is not a constant. Skipping.", field_name);
					DEALLOCATE(field_name);
				}
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Minimisation::construct_dof_arrays.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Minimisation::construct_dof_arrays */

void Minimisation::list_dof_values()
/*******************************************************************************
 LAST MODIFIED : 8 May 2007

 DESCRIPTION :
 Simple function to list the dof values. This is mainly for debugging purposes
 and may be removed later.
 ==============================================================================*/
{
	int i;

	ENTER(Minimisation::list_dof_values);

	for (i = 0; i < total_dof; i++) {
		cout << "dof[" << i << "] = " << *dof_storage_array[i] << endl;
	}

	LEAVE;

} /* Minimisation::list_dof_values */

/** Must call this function after updating independent field DOFs to ensure
 * dependent field caches are fully recalculated with the DOF changes.
 */
void Minimisation::invalidate_independent_field_caches()
{
	for (size_t j = 0; j < optimisation->independentFields.size(); ++j)
	{
		Cmiss_field_cache_invalidate_field(field_cache, optimisation->independentFields[j]);
	}
}

/***************************************************************************//**
 * Evaluates the objective function value given the currents dof values.
 */
FE_value Minimisation::evaluate_objective_function()
{
	invalidate_independent_field_caches();
	FE_value objective_value[3];
	Cmiss_field_evaluate_real(optimisation->objectiveField, field_cache,
		/*number_of_values*/3, objective_value);
	// GRC why do we need sqrt and / 3.0 in following ?
	// DPN not sure, was in Duane's original code. Removing it makes little difference but probably more correct...
	return (sqrt(objective_value[0] / 3.0));
}

void Minimisation::minimise_NSDGSL()
/*******************************************************************************
 LAST MODIFIED : 8 May 2007

 DESCRIPTION :
 Calculates the normalised steepest decent vector and does a Golden section line
 search in the direction of the steepest decent vector to find the minimum along
 the line. This is repeated until the stopping criteria is met and overall
 minimum is found. The intention is to replace this minimisations function with
 the non-linear minimisation function in PetSc. This should be more efficient
 and may be parallelised.
 ==============================================================================*/
{
	int i, iteration, max_iterations, stop;
	FE_value h, tol, lstol, line_size;
	FE_value phi, LFa, LFb;
	FE_value *dFdx, sum;
	FE_value s0, s1, sa, sb, ds;
	// pjb: temporarily removed declaration of F1 as it is not yet used,
	// otherwise irix compilation will fail
	//	FE_value F0, F1, Fa, Fb, Fi, Ff;
	FE_value F0, Fa, Fb, Fi, Ff = 0.0;

	ALLOCATE(dFdx,FE_value,total_dof);

	ENTER(Minimisation::minimise_NSDGSL);

	// Initialisation
	h = 1E-5; // independent variable perturbation value
	tol = 1E-5; // outside loop tolerance
	lstol = 0.1 * tol; // inside loop (line search) tolerance
	line_size = 0.5; // line size for line search
	max_iterations = optimisation->maximumIterations; // for outside loop

	phi = (1.0 + sqrt(5.0)) / 2.0;
	LFa = 1.0 / (1.0 + phi);
	LFb = 1 - LFa;

	// initialise independent values to initial values
	for (i = 0; i < total_dof; i++)
	{
		set_dof_value(i, dof_initial_values[i]);
	}

	iteration = 0;
	ds = tol + 1.0;
	stop = 0;
	Fi = evaluate_objective_function();
	// Outside loop searching for the overall minimum
	while ((ds > tol) && (iteration < max_iterations) && (stop == 0))
	{
		iteration++;

		// Construct dFdx, the steepest decent vector
		F0 = evaluate_objective_function();
		for (i = 0; i < total_dof; i++)
		{
			set_dof_value(i, dof_initial_values[i] - h);
			Fa = F0 - evaluate_objective_function();
			set_dof_value(i, dof_initial_values[i] + h);
			Fb = evaluate_objective_function() - F0;
			dFdx[i] = 0.5 * (Fb + Fa);
			set_dof_value(i, dof_initial_values[i]);
		}

		// Normalise dFdx
		sum = 0.0;
		for (i = 0; i < total_dof; i++)
		{
			sum += dFdx[i] * dFdx[i];
		}
		sum = sqrt(sum);

		if (sum == 0.0)
		{
			stop = 1;
		}
		else // Line search
		{
			for (i = 0; i < total_dof; i++)
			{
				dFdx[i] = dFdx[i] / sum;
			}

			s0 = 0;
			F0 = evaluate_objective_function();

			s1 = line_size;
			for (i = 0; i < total_dof; i++)
			{
				set_dof_value(i, (dof_initial_values[i] - dFdx[i] * s1));
			}
			// F1 = evaluate_objective_function();

			sa = s0 + LFa * (s1 - s0);
			for (i = 0; i < total_dof; i++)
			{
				set_dof_value(i, (dof_initial_values[i] - dFdx[i] * sa));
			}
			Fa = evaluate_objective_function();

			sb = s0 + LFb * (s1 - s0);
			for (i = 0; i < total_dof; i++)
			{
				set_dof_value(i, (dof_initial_values[i] - dFdx[i] * sb));
			}
			Fb = evaluate_objective_function();

			// Golden Section Line Search, searching for line minimum
			while (((s1 - s0) > lstol) && (stop == 0))
			{
				if (Fb > Fa) {
					s1 = sb;
					// F1 = Fb;
					sb = sa;
					Fb = Fa;
					sa = s0 + LFa * (s1 - s0);
					for (i = 0; i < total_dof; i++)
					{
						set_dof_value(i, (dof_initial_values[i] - dFdx[i] * sa));
					}
					Fa = evaluate_objective_function();
					ds = sb;
					Ff = Fb;
				}
				else
				{
					s0 = sa;
					F0 = Fa;
					sa = sb;
					Fa = Fb;
					sb = s0 + LFb * (s1 - s0);
					for (i = 0; i < total_dof; i++)
					{
						set_dof_value(i, (dof_initial_values[i] - dFdx[i] * sb));
					}
					Fb = evaluate_objective_function();
					ds = sa;
					Ff = Fa;
				}
			} // Line Search

			// Update dof
			for (i = 0; i < total_dof; i++)
			{
				dof_initial_values[i] -= dFdx[i] * ds;
				set_dof_value(i, dof_initial_values[i]);
			}

			// Precision of FE_value
			if (fabs(Fi - Ff) < 1E-7) stop = 1;
			Fi = Ff;
		}
		//cout << "Objective Value = " << evaluate_objective_function() << endl;
	} // Outside loop

	if (iteration >= max_iterations) cout << "Max number of iterations reached" << endl;
	//list_dof_values();

	DEALLOCATE(dFdx);

	LEAVE;
}

void init_QN(int ndim, ColumnVector& x)
/**
 * One time initialisation code required by the Opt++ quasi-Newton minimisation.
 */
{
	// copy over the initial DOFs
	int i;
	Minimisation* minimisation = static_cast<Minimisation*> (UserData);
	for (i = 0; i < ndim; i++)
	{
		x(i+1) = static_cast<double>(minimisation->dof_initial_values[i]);
	}
}

void objective_function_QN(int ndim, const ColumnVector& x, double& fx,
		int& result)
/**
 * The objective function for the Opt++ quasi-Newton minimisation.
 */
{
	int i;
	Minimisation* minimisation = static_cast<Minimisation*> (UserData);
	// ColumnVector's index'd from 1...
	for (i = 0; i < ndim; i++)
	{
		minimisation->set_dof_value(i, x(i + 1));
	}
	//minimisation->list_dof_values();

	fx = static_cast<double> (minimisation->evaluate_objective_function());
	//cout << "Objective Value = " << fx << endl;
	result = NLPFunction;
}

/** ensures constant independent fields are marked as changed, so graphics update
 * FIXME: Should only need to do this once, after optimisation.
 */
void Minimisation::touch_constant_independent_fields()
{
	if (optimisation->independentFieldsConstant)
	{
		for (size_t i = 0; i < optimisation->independentFields.size(); i++)
		{
			int number_of_components = Computed_field_get_number_of_components(optimisation->independentFields[i]);
			FE_value *values = new FE_value[number_of_components];
			Cmiss_field_evaluate_real(optimisation->independentFields[i], field_cache, number_of_components, values);
			Cmiss_field_assign_real(optimisation->independentFields[i], field_cache, number_of_components, values);
			delete [] values;
		}
	}
}

void Minimisation::minimise_QN()
/**
 * Naive wrapper around a quasi-Newton minimisation.
 */
{
	char message[] = { "Solution from quasi-newton" };
	ENTER(Minimisation::minimise_QN);
	// need a handle on this object...
	// FIXME: need to find and use "user data" in the Opt++ methods.
	UserData = static_cast<void*> (this);

	FDNLF1 nlp(total_dof, objective_function_QN, init_QN);
	OptQNewton objfcn(&nlp);
	objfcn.setSearchStrategy(LineSearch);
	objfcn.setFcnTol(optimisation->functionTolerance);
	objfcn.setGradTol(optimisation->gradientTolerance);
	objfcn.setStepTol(optimisation->stepTolerance);
	objfcn.setMaxIter(optimisation->maximumIterations);
	objfcn.setMaxFeval(optimisation->maximumNumberFunctionEvaluations);
	objfcn.setMaxStep(optimisation->maximumStep);
	objfcn.setMinStep(optimisation->minimumStep);
	objfcn.setLineSearchTol(optimisation->linesearchTolerance);
	objfcn.setMaxBacktrackIter(optimisation->maximumBacktrackIterations);
	/* @todo Add in support for trust region methods
	objfcn.setTRSize(optimisation->trustRegionSize);
	*/

	// create a string stream to store messages from Opt++ and set that for all output.
	std::stringbuf optppMessages;
	std::ostream optppMessageStream(&optppMessages);
	if (!objfcn.setOutputFile(optppMessageStream))
		cerr << "main: output file open failed" << endl;
	objfcn.optimize();
	objfcn.printStatus(message);
	objfcn.cleanup();

	ColumnVector solution = nlp.getXc();
	int i;
	for (i = 0; i < total_dof; i++)
		this->set_dof_value(i, solution(i + 1));
	touch_constant_independent_fields();
	//list_dof_values();
	// write out messages?
	std::string m = optppMessages.str();
	display_chunked_message(m);
	LEAVE;
}

void init_LSQ(int ndim, ColumnVector& x)
/**
 * One time initialisation code required by the Opt++ least-squares minimisation.
 */
{
	// copy over the initial DOFs
	int i;
	Minimisation* minimisation = static_cast<Minimisation*> (UserData);
	for (i = 0; i < ndim; i++)
	{
		x(i+1) = static_cast<double>(minimisation->dof_initial_values[i]);
	}
	// compute the initial data projections
	minimisation->calculate_data_projections();
}

int calculate_LS_term(struct FE_node *node,void *user_data_void)
{
	UDP* ud = static_cast<UDP*>(user_data_void);
	Minimisation *minimisation = ud->second;
	int i,number_of_mesh_components = Computed_field_get_number_of_components(
			minimisation->optimisation->meshField);
	FE_value* actualValues = static_cast<FE_value*>(NULL);
	FE_value* currentValues = static_cast<FE_value*>(NULL);
	ALLOCATE(actualValues,FE_value,minimisation->number_of_data_components);
	ALLOCATE(currentValues,FE_value,number_of_mesh_components);
	Projection projection = minimisation->projections[node];
	// FIXME: really should check return values!
	Computed_field_evaluate_at_node(minimisation->optimisation->dataField, node,
			minimisation->current_time, actualValues);
	Computed_field_evaluate_in_element(minimisation->optimisation->meshField,projection.element,
			projection.xi, minimisation->current_time, (struct FE_element*)NULL,
			currentValues, (FE_value *)NULL);
	for (i=0;i<number_of_mesh_components;i++)
	{
		//std::cout << "current: " << currentValues[i] << "; actual: " << actualValues[i] << std::endl;
		double diff = (double)(currentValues[i] - actualValues[i]);
		ud->first->element(minimisation->currentDataPointIndex) = diff;
		//std::cout << "LS term " << minimisation->currentDataPointIndex << ": "
		//		<< diff << std::endl;
		minimisation->currentDataPointIndex ++;
	}
	//diff = sqrt(diff);
	DEALLOCATE(actualValues);
	DEALLOCATE(currentValues);
	return 1;
}

void objective_function_LSQ(int ndim, const ColumnVector& x, ColumnVector& fx,
		int& result, void* iterationCounterVoid)
/**
 * The objective function for the Opt++ quasi-Newton minimisation.
 */
{
	//int* iterationCounter = static_cast<int*>(iterationCounterVoid);
	//std::cout << "objective function called " << ++(*iterationCounter) << " times." << std::endl;
	USE_PARAMETER(iterationCounterVoid);
	int i;
	UDP ud;
	Minimisation* minimisation = static_cast<Minimisation*> (UserData);
	// ColumnVector's index'd from 1...
	for (i = 0; i < ndim; i++)
	{
		minimisation->set_dof_value(i, x(i + 1));
	}
	//minimisation->list_dof_values();
	minimisation->invalidate_independent_field_caches();
	//minimisation->calculate_data_projections();

	// evaluate the least-squares terms
	ud.first = &fx;
	ud.second = minimisation;
	minimisation->currentDataPointIndex = 0;
	Cmiss_node_iterator_id iterator =
			Cmiss_nodeset_create_node_iterator(minimisation->optimisation->dataNodeset);
	Cmiss_node_id node = Cmiss_node_iterator_next(iterator);
	int code = 1;
	while (node && code)
	{
		code = calculate_LS_term(node, &ud);
		Cmiss_node_destroy(&node);
		if (code) node = Cmiss_node_iterator_next(iterator);
	}
	Cmiss_node_iterator_destroy(&iterator);

	//FE_region_for_each_FE_node(Cmiss_region_get_FE_region(minimisation->optimisation->dataRegion),
	//		calculate_LS_term,&ud);
	result = NLPFunction;
}

/**
 * Dummy for Opt++ least squares...
 */
void update_model_dummy(int, int, ColumnVector) {}

void Minimisation::minimise_LSQN()
/*
 * Least-squares minimisation using Opt++
 */
{
	int iterationCounter = 0;
	char message[] = { "Solution from newton least squares" };
	ENTER(Minimisation::minimise_LSQN);
	// need a handle on this object...
	UserData = static_cast<void*> (this);
	this->number_of_data_points = Cmiss_nodeset_get_size(optimisation->dataNodeset);
	this->number_of_data_components = Computed_field_get_number_of_components(
			this->optimisation->dataField);
	LSQNLF nlp(total_dof, this->number_of_data_points * this->number_of_data_components,
			objective_function_LSQ, init_LSQ, (OPTPP::INITCONFCN)NULL,
			(void*)(&iterationCounter));
	OptNewton objfcn(&nlp);
	objfcn.setSearchStrategy(LineSearch);
	// create a string stream to store messages from Opt++ and set that for all output.
	std::stringbuf optppMessages;
	std::ostream optppMessageStream(&optppMessages);
	if (!objfcn.setOutputFile(optppMessageStream))
		cerr << "main: output file open failed" << endl;
	objfcn.setFcnTol(optimisation->functionTolerance);
	objfcn.setGradTol(optimisation->gradientTolerance);
	objfcn.setStepTol(optimisation->stepTolerance);
	objfcn.setMaxIter(optimisation->maximumIterations);
	objfcn.setMaxFeval(optimisation->maximumNumberFunctionEvaluations);
	objfcn.setMaxStep(optimisation->maximumStep);
	objfcn.setMinStep(optimisation->minimumStep);
	objfcn.setLineSearchTol(optimisation->linesearchTolerance);
	objfcn.setMaxBacktrackIter(optimisation->maximumBacktrackIterations);
	/* @todo Add in support for trust region methods
	objfcn.setTRSize(optimisation->trustRegionSize);
	*/
	//nlp.setIsExpensive(false);
	//nlp.setDebug();
	objfcn.optimize();
	objfcn.printStatus(message);
	ColumnVector solution = nlp.getXc();
	int i;
	for (i = 0; i < total_dof; i++)
		this->set_dof_value(i, solution(i + 1));
	touch_constant_independent_fields();
	//list_dof_values();
	// write out messages?
	std::string m = optppMessages.str();
	display_chunked_message(m);
	LEAVE;
}


/*=====================================================*/
/*=====================================================*/
/*=====================================================*/

int Cmiss_optimisation::runOptimisation()
{
	int return_code = 1;

	ENTER(Cmiss_optimisation::runOptimisation);
	// FIXME: must be a better way to iterate over all the nodes in the mesh?
	// Create minimisation object
	Minimisation minimisation(this);
	// Populate the dof pointers and initial values for each node
	if (Computed_field_is_type_finite_element(independentFields[0]))
	{
		FE_region_for_each_FE_node(minimisation.fe_region,
				Minimisation::construct_dof_arrays,
				&minimisation);
	}
	else if (independentFieldsConstant)
	{
		Minimisation::construct_dof_arrays(NULL,&minimisation);
	}
	else
	{
		// should never get this far?
		display_message(ERROR_MESSAGE,"Cmiss_optimisation::runOptimisation. "
				"Invalid independent field type.");
		return 0;
	}

	// Minimise the objective function
	switch (this->method)
	{
	case CMISS_OPTIMISATION_METHOD_NSDGSL:
		minimisation.minimise_NSDGSL();
		break;
	case CMISS_OPTIMISATION_METHOD_QUASI_NEWTON:
		minimisation.minimise_QN();
		break;
	case CMISS_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON:
		minimisation.minimise_LSQN();
		break;
	default:
		display_message(ERROR_MESSAGE, "Cmiss_optimisation::runOptimisation. "
			"Unknown minimisation method.");
		return_code = 0;
		break;
	}
	LEAVE;
	return (return_code);
}

