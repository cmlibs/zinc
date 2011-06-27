/***************************************************************************//**
 * FILE : minimise.cpp
 *
 * Optimisation/minimisation routines.
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
#include "command/parser.h"
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
#include "minimise/minimise.h"
}
#include "general/enumerator_private_cpp.hpp"
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

enum MinimisationMethod {
	NSDGSL, // Duane's original steepest descent with golden section line search
	OPTPP_QuasiNewton, // Opt++ with quasi-Newton - naive approach based on Duane's code
	OPTPP_LSQ_Newton, // Opt++ least squares with Newton
	UNKNOWN_MINIMISATION_METHOD
};

PROTOTYPE_ENUMERATOR_FUNCTIONS(MinimisationMethod)
;

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(MinimisationMethod) {
	const char *enumerator_string;
	ENTER(ENUMERATOR_STRING(MinimisationMethod));
	switch (enumerator_value) {
	case NSDGSL: {
		enumerator_string = "NSDGSL";
	}
		break;
	case OPTPP_QuasiNewton: {
		enumerator_string = "QN";
	}
		break;
	case OPTPP_LSQ_Newton: {
		enumerator_string = "LSQN";
	}
		break;
	default: {
		enumerator_string = (const char *) NULL;
	}
		break;
	}LEAVE;
	return (enumerator_string);
}

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS( MinimisationMethod)
;

struct Minimisation_package {
	struct Time_keeper *time_keeper;
	struct Computed_field_package *computed_field_package;
	struct Cmiss_region *root_region;
	enum MinimisationMethod method;
};

struct Minimisation_package *CREATE(Minimisation_package)(
		struct Time_keeper *time_keeper,
		struct Computed_field_package *computed_field_package,
		struct Cmiss_region *root_region)
/*******************************************************************************
 LAST MODIFIED : 7 May 2007

 DESCRIPTION :
 Creates the package required for minimisation.
 ==============================================================================*/
{
	struct Minimisation_package *package;

	ENTER(CREATE(Minimisation_package));
	if (time_keeper) {
		ALLOCATE(package, struct Minimisation_package, 1);
		package->time_keeper = time_keeper;
		package->computed_field_package = computed_field_package;
		package->root_region = root_region;
		package->method = UNKNOWN_MINIMISATION_METHOD;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Minimisation_package).  Invalid arguments.");
		package = (struct Minimisation_package *)NULL;
	}
	LEAVE;

	return (package);
} /* CREATE(Minimisation_package) */

int DESTROY(Minimisation_package)(struct Minimisation_package **package)
/*******************************************************************************
 LAST MODIFIED : 7 May 2007

 DESCRIPTION :
 Destroys the package required for minimisation.
 ==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Minimisation_package));
	if (*package) {
		DEALLOCATE(*package);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Minimisation_package).  Missing package");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Minimisation_package) */

namespace {

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
	//These parameters are protected to this file using a NULL namespace
public:
	Computed_field *objective_field;
	Computed_field **independent_fields;
	int number_of_independent_fields;
	int independent_fields_are_constant;
	Computed_field *data_field;
	Computed_field *mesh_field;
	FE_region *fe_region;
	Cmiss_region *region;
	Cmiss_region* data_group;
	Cmiss_field_module_id field_module;
	Cmiss_field_cache_id field_cache;
	FE_value current_time;
	int maximum_iterations;
	int dimension;
	int total_dof;
	int number_of_data_components,number_of_data_points;
	int currentDataPointIndex;
	std::map< const FE_node*, Projection > projections;

	Minimisation(Computed_field* objective_field,
			Computed_field* data_field,
			Computed_field* mesh_field, FE_region* fe_region,
			Cmiss_region* region, Cmiss_region* data_group,
			int maximum_iterations, int dimension) :
				objective_field(objective_field),
				data_field(data_field), mesh_field(mesh_field),
				fe_region(fe_region), region(region), data_group(data_group),
				field_module(Cmiss_region_get_field_module(region)),
				field_cache(Cmiss_field_module_create_cache(field_module)),
				maximum_iterations(maximum_iterations), dimension(dimension)
	{
		current_time = 0.0;
		total_dof = 0;
		number_of_data_points = 0;
		number_of_independent_fields = 0;
		independent_fields_are_constant = 0;
		independent_fields = static_cast<Computed_field**> (NULL);
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
	static int calculate_projection(struct FE_node *node,
			void *minimisation_object_void);

	inline void set_dof_value(int dof_index, FE_value new_value)
	{
		*dof_storage_array[dof_index] = new_value;
	}

	void list_dof_values();

	FE_value evaluate_objective_function();

	/*void init_QN(int ndim, ColumnVector& x);
	 void objective_function_QN(int ndim, const ColumnVector& x, double& fx,
	 int& result);*/

	void touch_constant_independent_fields();

	void minimise_NSDGSL();
	void minimise_QN();
	void minimise_LSQN();

	// FIXME: should allow for different types of projection calculations?
	void calculate_data_projections();

	int add_independent_field(const char* name);
};

Minimisation::~Minimisation()
/*******************************************************************************
 LAST MODIFIED : 24 August 2006

 DESCRIPTION :
 Clear the type specific data used by this type.
 ==============================================================================*/
{
	int i;
	ENTER(Minimisation::~Minimisation);

	if (dof_storage_array) {
		DEALLOCATE(dof_storage_array);
	}
	if (dof_initial_values) {
		DEALLOCATE(dof_initial_values);
	}
	for (i=0;i<number_of_independent_fields;i++) {
		Cmiss_field_destroy(independent_fields+i);
	}
	DEALLOCATE(independent_fields);
	Cmiss_field_cache_destroy(&field_cache);
	Cmiss_field_module_end_change(field_module);
	Cmiss_field_module_destroy(&field_module);
	LEAVE;
} /* Minimisation::~Minimisation */

int Minimisation::add_independent_field(const char* name)
{
	int return_code = 1;
	// FIXME: need to check if adding the same field more than once.
	Cmiss_field_id field = Cmiss_field_module_find_field_by_name(field_module,name);
	if (independent_fields_are_constant && !Computed_field_is_constant(field))
	{
		display_message(ERROR_MESSAGE, "Minimisation::add_independent_field.  "
				"All independent fields must be constant if any are.");
		Cmiss_field_destroy(&field);
		return_code = 0;
	}
	else if (Computed_field_is_type_finite_element(field) && (number_of_independent_fields > 0))
	{
		display_message(ERROR_MESSAGE, "Minimisation::add_independent_field.  "
				"Can only have one finite element type independent field.");
		Cmiss_field_destroy(&field);
		return_code = 0;
	}
	if (return_code)
	{
		number_of_independent_fields++;
		REALLOCATE(independent_fields, independent_fields, Computed_field *, number_of_independent_fields);
		independent_fields[number_of_independent_fields - 1] = field;
		if (Computed_field_is_constant(field)) independent_fields_are_constant = 1;
	}
	return return_code;
}

int Minimisation::calculate_projection(struct FE_node *node,
		void *minimisation_object_void)
{
	Minimisation *minimisation;
	minimisation = static_cast<Minimisation *> (minimisation_object_void);
	FE_value* values = 0;
	ALLOCATE(values,FE_value,minimisation->number_of_data_components);
	Projection projection = minimisation->projections[node];
	Computed_field_evaluate_at_node(minimisation->data_field, node,
			minimisation->current_time, values);
	Computed_field_find_element_xi(minimisation->mesh_field,
		values, minimisation->number_of_data_components, minimisation->current_time,
		&(projection.element), projection.xi,
		/*element_dimension*/minimisation->dimension, \
		/*search_region*/minimisation->region, /*propagate_field*/0,
		/*find_nearest_location*/1);
	DEALLOCATE(values);
	minimisation->projections[node] = projection;
	//std::cout << "Element: " << FE_element_get_cm_number(projection.element) << ";";
	//std::cout << " xi: " << projection.xi[0] << "," << projection.xi[1] << ",";
	//std::cout << projection.xi[2] << std::endl;
	return 1;
}

void Minimisation::calculate_data_projections()
{
	//std::cout << "Minimisation::calculate_data_projections" << std::endl;
	struct Cmiss_region* dRegion = Computed_field_get_region(this->data_field);
	struct FE_region* fe_region = Cmiss_region_get_FE_region(dRegion);
	FE_region_for_each_FE_node(fe_region,Minimisation::calculate_projection,this);
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

		if (node) {
			// should only have one independent field
			Computed_field_get_type_finite_element(minimisation->independent_fields[0],
					&fe_field);

			//Get number of components
			number_of_components = get_FE_field_number_of_components(fe_field);

			//for each component in field
			for (component_number = 0; component_number < number_of_components; component_number++) {
				number_of_values = 1
						+ get_FE_node_field_component_number_of_derivatives(node,
								fe_field, component_number);

				// FIXME: tmp remove derivatives
				//number_of_values = 1;

				nodal_value_types = get_FE_node_field_component_nodal_value_types(
						node, fe_field, component_number);

				for (i = 0; i < number_of_values; i++) {

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
							node,
							fe_field,/*component_number*/
							component_number, /*version_number*/ 0,
							/*nodal_value_type*/nodal_value_types[i],
							minimisation->current_time,
							&(minimisation->dof_storage_array[minimisation->total_dof
									- 1]));

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
			for (i=0;i<minimisation->number_of_independent_fields;i++)
			{
				number_of_components = Cmiss_field_get_number_of_components(minimisation->independent_fields[i]);
				FE_value *constant_values_storage =
					Computed_field_constant_get_values_storage(minimisation->independent_fields[i]);
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
					char *field_name = Cmiss_field_get_name(minimisation->independent_fields[i]);
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

/***************************************************************************//**
 * Evaluates the objective function value given the currents dof values.
 */
FE_value Minimisation::evaluate_objective_function()
{
	FE_value objective_value[3];
	Computed_field_clear_cache(objective_field);
	Cmiss_field_evaluate_real(objective_field, field_cache,
		/*number_of_values*/3, objective_value);
	// GRC why do we need sqrt and / 3.0 in following ?
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
	max_iterations = 10000; // for outside loop

	phi = (1.0 + sqrt(5.0)) / 2.0;
	LFa = 1.0 / (1.0 + phi);
	LFb = 1 - LFa;

	// initialise independent values to initial values
	for (i = 0; i < total_dof; i++) {
		set_dof_value(i, dof_initial_values[i]);
	}

	iteration = 0;
	ds = tol + 1.0;
	stop = 0;
	Fi = evaluate_objective_function();
	// Outside loop searching for the overall minimum
	while ((ds > tol) && (iteration < max_iterations) && (stop == 0)) {
		iteration++;

		// Construct dFdx, the steepest decent vector
		F0 = evaluate_objective_function();
		for (i = 0; i < total_dof; i++) {
			set_dof_value(i, dof_initial_values[i] - h);
			Fa = F0 - evaluate_objective_function();
			set_dof_value(i, dof_initial_values[i] + h);
			Fb = evaluate_objective_function() - F0;
			dFdx[i] = 0.5 * (Fb + Fa);
			set_dof_value(i, dof_initial_values[i]);
		}

		// Normalise dFdx
		sum = 0.0;
		for (i = 0; i < total_dof; i++) {
			sum += dFdx[i] * dFdx[i];
		}
		sum = sqrt(sum);

		if (sum == 0.0) {
			stop = 1;
		} else // Line search
		{
			for (i = 0; i < total_dof; i++) {
				dFdx[i] = dFdx[i] / sum;
			}

			s0 = 0;
			F0 = evaluate_objective_function();

			s1 = line_size;
			for (i = 0; i < total_dof; i++) {
				set_dof_value(i, (dof_initial_values[i] - dFdx[i] * s1));
			}
			// F1 = evaluate_objective_function();

			sa = s0 + LFa * (s1 - s0);
			for (i = 0; i < total_dof; i++) {
				set_dof_value(i, (dof_initial_values[i] - dFdx[i] * sa));
			}
			Fa = evaluate_objective_function();

			sb = s0 + LFb * (s1 - s0);
			for (i = 0; i < total_dof; i++) {
				set_dof_value(i, (dof_initial_values[i] - dFdx[i] * sb));
			}
			Fb = evaluate_objective_function();

			// Golden Section Line Search, searching for line minimum
			while (((s1 - s0) > lstol) && (stop == 0)) {

				if (Fb > Fa) {
					s1 = sb;
					// F1 = Fb;
					sb = sa;
					Fb = Fa;
					sa = s0 + LFa * (s1 - s0);
					for (i = 0; i < total_dof; i++) {
						set_dof_value(i, (dof_initial_values[i] - dFdx[i] * sa));
					}
					Fa = evaluate_objective_function();
					ds = sb;
					Ff = Fb;
				} else {
					s0 = sa;
					F0 = Fa;
					sa = sb;
					Fa = Fb;
					sb = s0 + LFb * (s1 - s0);
					for (i = 0; i < total_dof; i++) {
						set_dof_value(i, (dof_initial_values[i] - dFdx[i] * sb));
					}
					Fb = evaluate_objective_function();
					ds = sa;
					Ff = Fa;
				}

			} // Line Search

			// Update dof
			for (i = 0; i < total_dof; i++) {
				dof_initial_values[i] -= dFdx[i] * ds;
				set_dof_value(i, dof_initial_values[i]);
			}

			// Precision of FE_value
			if (fabs(Fi - Ff) < 1E-7)
				stop = 1;
			Fi = Ff;

		}

		//cout << "Objective Value = " << evaluate_objective_function() << endl;

	} // Outside loop

	if (iteration >= max_iterations)
		cout << "Max number of iterations reached" << endl;
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
	for (i = 0; i < ndim; i++) {
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
	for (i = 0; i < ndim; i++) {
		minimisation->set_dof_value(i, x(i + 1));
	}
	//minimisation->list_dof_values();
	// need to clear the cache of the independent field for the DOF changes to take effect.
	for (i=0;i<minimisation->number_of_independent_fields;i++) {
		Computed_field_clear_cache(minimisation->independent_fields[i]);
	}
	fx = static_cast<double> (minimisation->evaluate_objective_function());
	//cout << "Objective Value = " << fx << endl;
	result = NLPFunction;
}

/** ensures constant independent fields are marked as changed, so graphics update
 * FIXME: Should only need to do this once, after optimisation.
 */
void Minimisation::touch_constant_independent_fields()
{
	if (independent_fields_are_constant)
	{
		for (int i = 0; i < number_of_independent_fields; i++)
		{
			int number_of_components = Computed_field_get_number_of_components(independent_fields[i]);
			FE_value *values = new FE_value[number_of_components];
			Cmiss_field_evaluate_real(independent_fields[i], field_cache, number_of_components, values);
			Cmiss_field_assign_real(independent_fields[i], field_cache, number_of_components, values);
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
	UserData = static_cast<void*> (this);

	FDNLF1 nlp(total_dof, objective_function_QN, init_QN);
	OptQNewton objfcn(&nlp);
	objfcn.setSearchStrategy(TrustRegion);
	//objfcn.setMaxFeval(200);
//	objfcn.setFcnTol(1.e-4);
//	objfcn.setMinStep(1.0e-4);
//	objfcn.setStepTol(1.0e-4);
//	objfcn.setConTol(1.0e-4);
//	objfcn.setGradTol(1.0e-4);
//	objfcn.setLineSearchTol(1.0e-4);

	// create a string stream to store messages from Opt++ and set that for all output.
	std::stringbuf optppMessages;
	std::ostream optppMessageStream(&optppMessages);
	if (!objfcn.setOutputFile(optppMessageStream))
		cerr << "main: output file open failed" << endl;
	objfcn.setMaxIter(this->maximum_iterations);
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
	for (i = 0; i < ndim; i++) {
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
			minimisation->mesh_field);
	FE_value* actualValues = static_cast<FE_value*>(NULL);
	FE_value* currentValues = static_cast<FE_value*>(NULL);
	ALLOCATE(actualValues,FE_value,minimisation->number_of_data_components);
	ALLOCATE(currentValues,FE_value,number_of_mesh_components);
	Projection projection = minimisation->projections[node];
	// FIXME: really should check return values!
	Computed_field_evaluate_at_node(minimisation->data_field, node,
			minimisation->current_time, actualValues);
	Computed_field_evaluate_in_element(minimisation->mesh_field,projection.element,
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
#if 0
	UDP ud;
	Minimisation* minimisation = static_cast<Minimisation*> (UserData);
	struct Cmiss_region* dRegion = Computed_field_get_region(minimisation->objective_field);
	struct FE_region* fe_region = Cmiss_region_get_FE_region(dRegion);
	ud.first = &fx;
	ud.second = minimisation;
	minimisation->currentDataPointIndex = 0;
	minimisation->list_dof_values();
	FE_region_for_each_FE_node(fe_region,calculate_LS_term,&ud);
	minimisation->set_dof_value(0, -100);
	minimisation->currentDataPointIndex = 0;
	minimisation->list_dof_values();
	FE_region_for_each_FE_node(fe_region,calculate_LS_term,&ud);
	Computed_field_clear_cache(minimisation->objective_field);
	Computed_field_clear_cache(minimisation->independent_field);
	minimisation->currentDataPointIndex = 0;
	FE_region_for_each_FE_node(fe_region,calculate_LS_term,&ud);
	exit(1);
#endif
	//int* iterationCounter = static_cast<int*>(iterationCounterVoid);
	//std::cout << "objective function called " << ++(*iterationCounter) << " times." << std::endl;
	USE_PARAMETER(iterationCounterVoid);
	int i;
	UDP ud;
	Minimisation* minimisation = static_cast<Minimisation*> (UserData);
	// ColumnVector's index'd from 1...
	for (i = 0; i < ndim; i++) {
		minimisation->set_dof_value(i, x(i + 1));
	}
	//minimisation->list_dof_values();
	// need to clear the cache of the independent field for the DOF changes to take effect.
	for (i=0;i<minimisation->number_of_independent_fields;i++) {
		Computed_field_clear_cache(minimisation->independent_fields[i]);
	}
	//minimisation->calculate_data_projections();

	// evaluate the least-squares terms
	ud.first = &fx;
	ud.second = minimisation;
	minimisation->currentDataPointIndex = 0;
	FE_region_for_each_FE_node(Cmiss_region_get_FE_region(minimisation->data_group),
			calculate_LS_term,&ud);
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
	// FIXME: using objective field as the data field...
	// GRC: should allow alternative nodeset "cmiss_data" to be used:
	// Need to get the field_module for the data_group to get the nodeset for the group region
	Cmiss_field_module_id data_group_field_module = Cmiss_region_get_field_module(this->data_group);
	Cmiss_nodeset_id datapointset = Cmiss_field_module_get_nodeset_by_name(data_group_field_module, "cmiss_nodes");
	this->number_of_data_points = Cmiss_nodeset_get_size(datapointset);
	Cmiss_nodeset_destroy(&datapointset);
	Cmiss_field_module_destroy(&data_group_field_module);
	this->number_of_data_components = Computed_field_get_number_of_components(
			this->data_field);
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
	objfcn.setMaxIter(this->maximum_iterations);
	//objfcn.setMaxFeval(this->maximum_iterations);
	//objfcn.setTRSize(1.0e3);
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

} //namespace


/*=====================================================*/
/*=====================================================*/
/*=====================================================*/

int gfx_minimise(struct Parse_state *state, void *dummy_to_be_modified,
		void *package_void)
/*******************************************************************************
 LAST MODIFIED : 04 May 2007

 DESCRIPTION : Minimises the <objective_field> by changing the <independent_field>
 over a <region>
 ==============================================================================*/
{
	int dimension, number_of_valid_strings, maxIters, return_code, i;
	struct Computed_field *objective_field, *data_field, *mesh_field;
	struct Multiple_strings field_names;
	struct Minimisation_package *package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_objective_field_data,
		set_data_field_data,set_mesh_field_data;
	const char *minimisation_method_string, **valid_strings;

	ENTER(gfx_minimise);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (package = (struct Minimisation_package *) package_void))
	{
		struct Cmiss_region *region = NULL;
		struct Cmiss_region *data_group = NULL;
		objective_field = (struct Computed_field *) NULL;
		data_field = (struct Computed_field *) NULL;
		mesh_field = (struct Computed_field *) NULL;
		if (package->time_keeper) {
			// time = Time_keeper_get_time(package->time_keeper);
		} else {
			// time = 0;
		}

		/* the default maximum number of iterations to take */
		maxIters = 100;

		/* the default dimension of the elements to be fit */
		dimension = 2;

		option_table = CREATE(Option_table)();
		/* objective function */
		set_objective_field_data.conditional_function
				= (MANAGER_CONDITIONAL_FUNCTION(Computed_field) *) NULL;
		set_objective_field_data.conditional_function_user_data = (void *) NULL;
		set_objective_field_data.computed_field_manager
				= Computed_field_package_get_computed_field_manager(
						package->computed_field_package);
		Option_table_add_entry(option_table, "objective_field",
				&objective_field, &set_objective_field_data,
				set_Computed_field_conditional);
		/* independent field(s) */
		field_names.number_of_strings = 0;
		field_names.strings = (char **)NULL;
		Option_table_add_multiple_strings_entry(option_table, "independent_fields",
			&field_names, "FIELD_NAME [& FIELD_NAME [& ...]]");
		/* data field */
		set_data_field_data.conditional_function
				= (MANAGER_CONDITIONAL_FUNCTION(Computed_field) *) NULL;
		set_data_field_data.conditional_function_user_data
				= (void *) NULL;
		set_data_field_data.computed_field_manager
				= Computed_field_package_get_computed_field_manager(
						package->computed_field_package);
		Option_table_add_entry(option_table, "data_field",
				&data_field, &set_data_field_data,
				set_Computed_field_conditional);
		/* mesh field */
		set_mesh_field_data.conditional_function
				= (MANAGER_CONDITIONAL_FUNCTION(Computed_field) *) NULL;
		set_mesh_field_data.conditional_function_user_data
				= (void *) NULL;
		set_mesh_field_data.computed_field_manager
				= Computed_field_package_get_computed_field_manager(
						package->computed_field_package);
		Option_table_add_entry(option_table, "mesh_field",
				&mesh_field, &set_mesh_field_data,
				set_Computed_field_conditional);
		/* region/groups */
		Option_table_add_set_Cmiss_region(option_table, "group",
			package->root_region, &region);
		Option_table_add_set_Cmiss_region(option_table, "data_group",
			package->root_region, &data_group);
		/* method to use */
		minimisation_method_string = ENUMERATOR_STRING(MinimisationMethod)(
				package->method);
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(MinimisationMethod)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(MinimisationMethod) *) NULL,
				(void *) NULL);
		Option_table_add_enumerator(option_table, number_of_valid_strings,
				valid_strings, &minimisation_method_string);
		DEALLOCATE(valid_strings);
		/* limit the number of iterations (really the number of objective
		   function evaluations) */
		Option_table_add_entry(option_table, "maximum_iterations", &maxIters,
			NULL, set_int_positive);
		/* the dimension of the elements to be fit */
		// FIXME: should be restricted to 1 or 2?
		Option_table_add_entry(option_table, "dimension", &dimension,
					NULL, set_int_positive);

		return_code = Option_table_multi_parse(option_table, state);
		if (return_code)
		{
			if ((field_names.number_of_strings > 0) && (objective_field || (data_field && mesh_field))) {
				if (region && data_group)
				{
					if (minimisation_method_string)
					{
						STRING_TO_ENUMERATOR(MinimisationMethod)(
							minimisation_method_string, &(package->method));
					}

					FE_region *fe_region = Cmiss_region_get_FE_region(
							region);

					// Create minimisation object
					Minimisation minimisation(objective_field,
							data_field, mesh_field, fe_region, region,
							data_group, maxIters, dimension);

					for (i=0;i<field_names.number_of_strings;i++) {
						minimisation.add_independent_field(field_names.strings[i]);
					}

					// Populate the dof pointers and initial values for each node
					if (Computed_field_is_type_finite_element(minimisation.independent_fields[0])) {
						FE_region_for_each_FE_node(fe_region,
								Minimisation::construct_dof_arrays,
								&minimisation);
					} else if (minimisation.independent_fields_are_constant) {
						Minimisation::construct_dof_arrays(NULL,&minimisation);
					}

					// Minimise the objective function
					switch (package->method) {
					case NSDGSL:
						minimisation.minimise_NSDGSL();
						break;
					case OPTPP_QuasiNewton:
						minimisation.minimise_QN();
						break;
					case OPTPP_LSQ_Newton:
						minimisation.minimise_LSQN();
						break;
					default:
						display_message(ERROR_MESSAGE, "gfx minimise. "
							"Unknown minimisation method.");
						return_code = 0;
						break;
					}
				} else {
					display_message(ERROR_MESSAGE,
							"gfx_minimise.  Must specify a region and data group");
					return_code = 0;
				}
			} else {
				display_message(ERROR_MESSAGE,
						"gfx_minimise.  Must specify independent field and either an objective field or a data and a mesh field");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (region)
		{
			Cmiss_region_destroy(&region);
		}
		if (data_group)
		{
			Cmiss_region_destroy(&data_group);
		}
		if (objective_field)
		{
			Cmiss_field_destroy(&objective_field);
		}
		if (data_field)
		{
			Cmiss_field_destroy(&data_field);
		}
		if (mesh_field)
		{
			Cmiss_field_destroy(&mesh_field);
		}
		if (field_names.strings)
		{
			for (int i = 0; i < field_names.number_of_strings; i++)
			{
				DEALLOCATE(field_names.strings[i]);
			}
			DEALLOCATE(field_names.strings);
		}
	} else {
		display_message(ERROR_MESSAGE, "gfx_minimise.  Invalid argument(s)");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* gfx_minimise */

