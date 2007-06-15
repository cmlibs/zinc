/*******************************************************************************
FILE : cmiss.c

LAST MODIFIED : 12 April 2006

DESCRIPTION :
Functions for executing cmiss commands.
==============================================================================*/
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
#include "command/parser.h"
#include "computed_field/computed_field.h"
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
#include <iostream>
using namespace std;

struct Minimisation_package
{
	struct Time_keeper *time_keeper;
	struct Computed_field_package *computed_field_package;
	struct Cmiss_region *root_region;
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
	if (time_keeper)
	{
		ALLOCATE(package, struct Minimisation_package, 1);
		package->time_keeper = time_keeper;
		package->computed_field_package = computed_field_package;
		package->root_region = root_region;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(package).  Invalid arguments.");
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
	if (*package)
	{
		DEALLOCATE(*package);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(package).  Missing package");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Minimisation_package) */




namespace {

class Minimisation
{
//These parameters are protected to this file using a NULL namespace
public:
  Computed_field *objective_field;
  Computed_field *independent_field;
	FE_region *fe_region;
	FE_value current_time;
	int total_dof;

	Minimisation(Computed_field* objective_field, Computed_field* independent_field,
		FE_region* fe_region) : 
		objective_field(objective_field), independent_field(independent_field),
		fe_region(fe_region)
	{
		current_time = 0.0;
		total_dof = 0;
		dof_storage_array = static_cast<FE_value**>(NULL);
		dof_initial_values = static_cast<FE_value*>(NULL);
	};

	~Minimisation();

private:
	
	FE_value **dof_storage_array;
	FE_value *dof_initial_values;
	
public:
	static int construct_dof_arrays(struct FE_node *node,
		void *minimisation_object_void);

	void set_dof_value(int dof_index, FE_value new_value);
	
	void list_dof_values();
	
	FE_value evaluate_objective_function();

	void minimise();
	
};

Minimisation::~Minimisation()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	ENTER(Minimisation::~Minimisation);
	
	if (dof_storage_array)
	{
		DEALLOCATE(dof_storage_array);
	}
	if (dof_initial_values)
	{
		DEALLOCATE(dof_initial_values);
	}
	LEAVE;
} /* Minimisation::~Minimisation */

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
	int i, component_number, return_code, number_of_components, number_of_values;
	Minimisation *minimisation;

	ENTER(Minimisation::construct_dof_arrays);
	if (node && (minimisation = 
		static_cast<Minimisation *>(minimisation_object_void)))
	{
		
		Computed_field_get_type_finite_element(minimisation->independent_field,
			&fe_field);
		
		//Get number of components
		number_of_components = get_FE_field_number_of_components(fe_field);
		
		//for each component in field
		for(component_number=0; component_number<number_of_components; 
				component_number++)
		{
			number_of_values=1+get_FE_node_field_component_number_of_derivatives(node,
				fe_field, component_number);
		
			nodal_value_types=get_FE_node_field_component_nodal_value_types(node,
			fe_field,component_number);
			
			for(i=0;i<number_of_values;i++)
			{
				
				// increment total_dof
				minimisation->total_dof++;
				
				// reallocate arrays
				REALLOCATE(minimisation->dof_storage_array, minimisation->dof_storage_array,
					FE_value *, minimisation->total_dof);
				REALLOCATE(minimisation->dof_initial_values, minimisation->dof_initial_values,
					FE_value, minimisation->total_dof);
				
				// get value storage pointer
				get_FE_nodal_FE_value_storage(node,
					fe_field,/*component_number*/component_number, /*version_number*/0,
					/*nodal_value_type*/nodal_value_types[i], minimisation->current_time,
					&(minimisation->dof_storage_array[minimisation->total_dof-1]));
					
				// get initial value from value storage pointer
				minimisation->dof_initial_values[minimisation->total_dof-1] =
					*minimisation->dof_storage_array[minimisation->total_dof-1];
				
				cout << minimisation->dof_storage_array[minimisation->total_dof-1]<<"   "<<
				  minimisation->dof_initial_values[minimisation->total_dof-1] << endl;
			}
			
			DEALLOCATE(nodal_value_types);
			
		}
	
	  return_code = 1;		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Minimisation::count_dof.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Minimisation::construct_dof_arrays */

void Minimisation::set_dof_value(int dof_index, FE_value new_value)
/*******************************************************************************
LAST MODIFIED : 8 May 2007

DESCRIPTION :
Sets the dof value to <new_value>
==============================================================================*/
{
  
	ENTER(Minimisation::set_dof_value);
	
	*dof_storage_array[dof_index] = new_value;	
	
	LEAVE;
	
} /* Minimisation::set_dof_value */

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
	
	for (i = 0 ; i < total_dof ; i++)
	{
		cout << "dof["<<i<<"] = "<<*dof_storage_array[i] << endl;
	}
	
	LEAVE;
	
} /* Minimisation::list_dof_values */


FE_value Minimisation::evaluate_objective_function()
/*******************************************************************************
LAST MODIFIED : 8 May 2007

DESCRIPTION :
Evalulates the objective function value given the currents dof values.
==============================================================================*/
{
  FE_value objective_value[3];
	FE_node *node;

	ENTER(Minimisation::evaluate_objective_function);
	
	node = FE_region_get_first_FE_node_that(fe_region, NULL, NULL);
	
	Computed_field_clear_cache(objective_field); 
	
	Computed_field_evaluate_at_node(objective_field, node, 
		current_time, objective_value);
		
	LEAVE;
	
	return(sqrt(objective_value[0]/3.0));
	
} /* Minimisation::evaluate_objective_function */


void Minimisation::minimise()
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
	FE_value F0, Fa, Fb, Fi, Ff;
	
	ALLOCATE(dFdx,FE_value,total_dof);

	ENTER(Minimisation::minimise);
	
	// Initialisation
	h = 1E-5; // independent variable perturbation value
	tol = 1E-5; // outside loop tolerance
	lstol = 0.1*tol; // inside loop (line search) tolerance
	line_size = 0.5; // line size for line search
	max_iterations = 10000; // for outside loop
		
	phi = (1.0 + sqrt(5.0))/2.0;
	LFa = 1.0/(1.0+phi);
	LFb = 1-LFa;
	
	// initialise independent values to initial values
	for (i=0;i<total_dof;i++)
	{
		set_dof_value(i,dof_initial_values[i]);
	}
	
	iteration=0;
	ds = tol+1.0;
	stop = 0;
	Fi = evaluate_objective_function();
	// Outside loop searching for the overall minimum
	while((ds>tol) && (iteration<max_iterations) && (stop==0))
	{
		iteration++;
		
		// Construct dFdx, the steepest decent vector
		F0 = evaluate_objective_function();
		for (i=0;i<total_dof;i++)
		{
			set_dof_value(i,dof_initial_values[i]-h);
			Fa = F0-evaluate_objective_function();
			set_dof_value(i,dof_initial_values[i]+h);
			Fb = evaluate_objective_function()-F0;
			dFdx[i] = 0.5*(Fb+Fa);
			set_dof_value(i,dof_initial_values[i]);
		}
		
		// Normalise dFdx
		sum = 0.0;
		for (i=0;i<total_dof;i++)
		{
			sum += dFdx[i]*dFdx[i];
		}
		sum = sqrt(sum);
		
		if(sum==0.0)
		{
			stop = 1;
		}
		else // Line search
		{
			for (i=0;i<total_dof;i++)
			{
				dFdx[i] = dFdx[i]/sum;
			}
			
			s0 = 0;
			F0 = evaluate_objective_function();
			
			s1 = line_size;
			for (i=0;i<total_dof;i++)
			{
				set_dof_value(i, (dof_initial_values[i] - dFdx[i]*s1));
			}
			// F1 = evaluate_objective_function();
			
			sa = s0 + LFa*(s1-s0);
			for (i=0;i<total_dof;i++)
			{
				set_dof_value(i, (dof_initial_values[i] - dFdx[i]*sa));
			}
			Fa = evaluate_objective_function();
	
			sb = s0 + LFb*(s1-s0);
			for (i=0;i<total_dof;i++)
			{
				set_dof_value(i, (dof_initial_values[i] - dFdx[i]*sb));
			}
			Fb = evaluate_objective_function();
			
			// Golden Section Line Search, searching for line minimum
			while (((s1-s0) > lstol)&&(stop==0)){
			
				if (Fb > Fa)
				{
					s1 = sb;
					// F1 = Fb;
					sb = sa;
					Fb = Fa;
					sa = s0 + LFa*(s1-s0);
					for (i=0;i<total_dof;i++)
					{
						set_dof_value(i, (dof_initial_values[i] - dFdx[i]*sa));
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
					sb = s0 + LFb*(s1-s0);
					for (i=0;i<total_dof;i++)
					{
						set_dof_value(i, (dof_initial_values[i] - dFdx[i]*sb));
					}
					Fb = evaluate_objective_function();
					ds = sa;
					Ff = Fa;
				}
		
			} // Line Search
			
			// Update dof
			for (i=0;i<total_dof;i++)
			{
				dof_initial_values[i] -= dFdx[i]*ds;
				set_dof_value(i,dof_initial_values[i]);
			}
			
			// Precision of FE_value
			if(fabs(Fi-Ff)<1E-7) stop = 1;
			Fi = Ff;
			
		}
		
		cout << "Objective Value = " << evaluate_objective_function() <<endl;
		
	} // Outside loop
		
	if(iteration>=max_iterations) cout << "Max number of iterations reached" << endl;
	list_dof_values();

	DEALLOCATE(dFdx);

	LEAVE;
	
} /* Minimisation::minimise */


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
	char *region_path;
	// pjb: temporarily removed declaration of time as it is not yet used,
	// otherwise irix compilation will fail
	//	FE_value time;
	int return_code;
	struct Cmiss_region *region;
	struct Computed_field *objective_field, *independent_field;
	struct Minimisation_package *package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_objective_field_data,
		set_independent_field_data;

	ENTER(gfx_minimise);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (package = (struct Minimisation_package *)package_void))
	{
		region_path = (char *)NULL;
		objective_field = (struct Computed_field *)NULL;
		independent_field = (struct Computed_field *)NULL;
		if (package->time_keeper)
		{
			// time = Time_keeper_get_time(package->time_keeper);
		}
		else
		{
			// time = 0;
		}
		
		option_table = CREATE(Option_table)();
		/* objective function */
		set_objective_field_data.conditional_function =
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_objective_field_data.conditional_function_user_data = (void *)NULL;
		set_objective_field_data.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				package->computed_field_package);
		Option_table_add_entry(option_table, "objective_field", &objective_field,
			&set_objective_field_data, set_Computed_field_conditional);
		/* independent field */
		set_independent_field_data.conditional_function =
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_independent_field_data.conditional_function_user_data = (void *)NULL;
		set_independent_field_data.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				package->computed_field_package);
		Option_table_add_entry(option_table, "independent_field", &independent_field,
			&set_independent_field_data, set_Computed_field_conditional);
		/* region */
		Option_table_add_entry(option_table, "region", &region_path,
			package->root_region, set_Cmiss_region_path);

		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (objective_field && independent_field)
			{
				if (region_path)
				{
					if (Cmiss_region_get_region_from_path(package->root_region, region_path, &region))
					{
						FE_region *fe_region = Cmiss_region_get_FE_region(region);
						
						FE_region_begin_change(fe_region);
						
						// Create minimisation object
						Minimisation minimisation(objective_field, independent_field, fe_region);
						
						// Populate the dof pointers and initial values for each node
						if (Computed_field_is_type_finite_element(independent_field))
						{
							FE_region_for_each_FE_node(fe_region, Minimisation::construct_dof_arrays,
								&minimisation);
						}
						
						// Minimise the objective function
						minimisation.minimise();
						
						FE_region_end_change(fe_region);
						
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_minimise"
							"Invalid argument(s)");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_minimise.  Must specify a region");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_minimise.  Must specify objective function and independent field");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
		if (independent_field)
		{
			DEACCESS(Computed_field)(&independent_field);
		}
		if (objective_field)
		{
			DEACCESS(Computed_field)(&objective_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_minimise.  Invalid argument(s)");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* gfx_minimise */

