
#include "api/cmiss_field.h"
#include "api/cmiss_field_module.h"
#include "api/cmiss_field_group.h"
#include "api/cmiss_region.h"
#include "api/cmiss_optimisation.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "general/enumerator.h"
#include "general/enumerator_private.hpp"
#include "region/cmiss_region_app.h"
#include "minimise/minimise.h"

int gfx_minimise(struct Parse_state *state, void *dummy_to_be_modified,
	void *root_region_void)
{
	int return_code;
	USE_PARAMETER(dummy_to_be_modified);
	Cmiss_region_id root_region = reinterpret_cast<Cmiss_region_id>(root_region_void);
	if (state && root_region)
	{
		enum Cmiss_optimisation_method optimisation_method = CMISS_OPTIMISATION_METHOD_QUASI_NEWTON;
		int maxIters = 100; // default value
		int showReport = 1; // output solution report by default
		const char *optimisation_method_string = 0;
		Multiple_strings independentFieldNames;
		Multiple_strings objectiveFieldNames;
		Cmiss_region_id region = Cmiss_region_access(root_region);

		Option_table *option_table = CREATE(Option_table)();
		/* independent field(s) */
		independentFieldNames.number_of_strings = 0;
		independentFieldNames.strings = (char **)NULL;
		Option_table_add_multiple_strings_entry(option_table, "independent_fields",
			&independentFieldNames, "FIELD_NAME [& FIELD_NAME [& ...]]");
		/* limit the number of iterations (really the number of objective
		   function evaluations) */
		Option_table_add_entry(option_table, "maximum_iterations", &maxIters,
			NULL, set_int_positive);
		/* method */
		optimisation_method_string =
			ENUMERATOR_STRING(Cmiss_optimisation_method)(optimisation_method);
		int number_of_valid_strings = 0;
		const char **valid_strings;
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Cmiss_optimisation_method)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(Cmiss_optimisation_method) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table, number_of_valid_strings,
			valid_strings, &optimisation_method_string);
		DEALLOCATE(valid_strings);
		/* objective field(s) */
		objectiveFieldNames.number_of_strings = 0;
		objectiveFieldNames.strings = (char **)NULL;
		Option_table_add_multiple_strings_entry(option_table, "objective_fields",
			&objectiveFieldNames, "FIELD_NAME [& FIELD_NAME [& ...]]");
		/* region */
		Option_table_add_set_Cmiss_region(option_table, "region", root_region, &region);
		/* flag whether to show or hide the optimisation output */
		Option_table_add_switch(option_table, "show_output", "hide_output", &showReport);
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code)
		{
			Cmiss_field_module_id fieldModule = Cmiss_region_get_field_module(region);
			Cmiss_optimisation_id optimisation = Cmiss_field_module_create_optimisation(fieldModule);
			STRING_TO_ENUMERATOR(Cmiss_optimisation_method)(
				optimisation_method_string, &optimisation_method);
			if (!Cmiss_optimisation_set_method(optimisation, optimisation_method))
			{
				display_message(ERROR_MESSAGE, "gfx minimise:  Invalid optimisation method");
				return_code = 0;
			}
			for (int i = 0; i < independentFieldNames.number_of_strings; i++)
			{
				Cmiss_field_id independentField = Cmiss_field_module_find_field_by_name(
					fieldModule, independentFieldNames.strings[i]);
				if (!Cmiss_optimisation_add_independent_field(optimisation, independentField))
				{
					display_message(ERROR_MESSAGE, "gfx minimise:  Invalid or unrecognised independent field '%s'",
						independentFieldNames.strings[i]);
					return_code = 0;
				}
				Cmiss_field_destroy(&independentField);
			}
			for (int i = 0; i < objectiveFieldNames.number_of_strings; i++)
			{
				Cmiss_field_id objectiveField = Cmiss_field_module_find_field_by_name(
					fieldModule, objectiveFieldNames.strings[i]);
				if (!Cmiss_optimisation_add_objective_field(optimisation, objectiveField))
				{
					display_message(ERROR_MESSAGE, "gfx minimise:  Invalid or unrecognised objective field '%s'",
						objectiveFieldNames.strings[i]);
					return_code = 0;
				}
				Cmiss_field_destroy(&objectiveField);
			}
			if (!Cmiss_optimisation_set_attribute_integer(optimisation,
				CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_ITERATIONS, maxIters))
			{
				display_message(ERROR_MESSAGE, "gfx minimise:  Invalid maximum_iterations %d", maxIters);
				return_code = 0;
			}
			if (return_code)
			{
				return_code = Cmiss_optimisation_optimise(optimisation);
				if (showReport)
				{
					char *report = Cmiss_optimisation_get_solution_report(optimisation);
					if (report)
					{
						display_message_string(INFORMATION_MESSAGE, report);
						DEALLOCATE(report);
					}
				}
				if (!return_code)
				{
					display_message(ERROR_MESSAGE, "gfx minimise.  Optimisation failed.");
				}
			}
			Cmiss_optimisation_destroy(&optimisation);
			Cmiss_field_module_destroy(&fieldModule);
		}
		DESTROY(Option_table)(&option_table);
		if (independentFieldNames.strings)
		{
			for (int i = 0; i < independentFieldNames.number_of_strings; i++)
			{
				DEALLOCATE(independentFieldNames.strings[i]);
			}
			DEALLOCATE(independentFieldNames.strings);
		}
		if (objectiveFieldNames.strings)
		{
			for (int i = 0; i < objectiveFieldNames.number_of_strings; i++)
			{
				DEALLOCATE(objectiveFieldNames.strings[i]);
			}
			DEALLOCATE(objectiveFieldNames.strings);
		}
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_minimise.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}
