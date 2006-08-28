/*******************************************************************************
FILE : computed_field_matrix_operations.c

LAST MODIFIED : 25 August 2006

DESCRIPTION :
Implements a number of basic vector operations on computed fields.
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
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_matrix_operations.h"
}
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "user_interface/message.h"
}

struct Computed_field_matrix_operations_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
};

int Computed_field_get_square_matrix_size(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If <field> can represent a square matrix with numerical components, the number
of rows = number of columns is returned.
==============================================================================*/
{
	int n, return_code, size;

	ENTER(Computed_field_get_square_matrix_size);
	return_code = 0;
	if (field)
	{
		size = field->number_of_components;
		n = 1;
		while (n * n < size)
		{
			n++;
		}
		if (n * n == size)
		{
			return_code = n;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_square_matrix_size.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_square_matrix_size */

int Computed_field_is_square_matrix(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns true if <field> can represent a square matrix, on account of having n*n
components, where n is a positive integer. If matrix is square, n is returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_square_matrix);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code =
			Computed_field_has_numerical_components(field, (void *)NULL) &&
			(0 != Computed_field_get_square_matrix_size(field));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_square_matrix.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_square_matrix */

namespace {

char computed_field_eigenvalues_type_string[] = "eigenvalues";

class Computed_field_eigenvalues : public Computed_field_core
{
public:
	/* cache for matrix, eigenvalues and eigenvectors */
	double *a, *d, *v;

	Computed_field_eigenvalues(Computed_field *field) : Computed_field_core(field)
	{
		a = (double*)NULL;
		d = (double*)NULL;
		v = (double*)NULL;
	};

	~Computed_field_eigenvalues();

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_eigenvalues(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_eigenvalues_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_eigenvalues*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int clear_cache();

	int evaluate();
};

Computed_field_eigenvalues::~Computed_field_eigenvalues()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	ENTER(Computed_field_eigenvalues::~Computed_field_eigenvalues);
	if (field)
	{
		if (a)
		{
			DEALLOCATE(a);
		}
		if (d)
		{
			DEALLOCATE(d);
		}
		if (v)
		{
			DEALLOCATE(v);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_eigenvalues::~Computed_field_eigenvalues.  "
			"Invalid argument(s)");
	}
	LEAVE;

} /* Computed_field_eigenvalues::~Computed_field_eigenvalues */

int Computed_field_eigenvalues::clear_cache()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_eigenvalues::clear_cache);
	if (field)
	{
		if (a)
		{
			DEALLOCATE(a);
		}
		if (d)
		{
			DEALLOCATE(d);
		}
		if (v)
		{
			DEALLOCATE(v);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_eigenvalues::clear_cache(.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_eigenvalues::clear_cache( */

int Computed_field_eigenvalues::evaluate()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Evaluates the eigenvalues and eigenvectors of the source field of <field> in
double precision in the type_specific_data then copies the eigenvalues to the
field->values.
==============================================================================*/
{
	int i, matrix_size, n, nrot, return_code;
	struct Computed_field *source_field;
	
	ENTER(Computed_field_eigenvalues::evaluate);
	if (field)
	{
		n = field->number_of_components;
		matrix_size = n * n;
		if ((a || ALLOCATE(a, double, matrix_size)) &&
			(d || ALLOCATE(d, double, n)) &&
			(v || ALLOCATE(v, double, matrix_size)))
		{
			source_field = field->source_fields[0];
			for (i = 0; i < matrix_size; i++)
			{
				a[i] = (double)(source_field->values[i]);
			}
			if (!matrix_is_symmetric(n, a, 1.0E-6))
			{
				display_message(WARNING_MESSAGE,
					"Eigenanalysis of field %s may be wrong as matrix not symmetric",
					source_field->name);
			}
			/* get eigenvalues and eigenvectors sorted from largest to smallest */
			if (Jacobi_eigenanalysis(n, a, d, v, &nrot) &&
				eigensort(n, d, v))
			{
				/* d now contains the eigenvalues, v the eigenvectors in columns, while
					 values of a above the main diagonal are destroyed */
				/* copy the eigenvalues into the field->values */
				for (i = 0; i < n; i++)
				{
					field->values[i] = (FE_value)(d[i]);
				}
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_eigenvalues::evaluate.  Eigenanalysis failed");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_eigenvalues::evaluate.  Could not allocate cache");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_eigenvalues::evaluate.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_eigenvalues::evaluate */

int Computed_field_eigenvalues::evaluate_cache_at_location(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Evaluate the fields cache in the element.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_eigenvalues::evaluate_cache_at_location);
	if (field && location)
	{
		if (0 == location->get_number_of_derivatives())
		{
			field->derivatives_valid = 0;
			return_code = Computed_field_evaluate_source_fields_cache_at_location(
				field, location)
				&& evaluate();
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_eigenvalues::evaluate_cache_at_location.  "
				"Cannot calculate derivatives of eigenvalues");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_eigenvalues::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_eigenvalues::evaluate_cache_at_location */


int Computed_field_eigenvalues::list()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_eigenvalues);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    source field : %s\n",
			field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_eigenvalues.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_eigenvalues */

char *Computed_field_eigenvalues::get_command_string()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_eigenvalues::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_eigenvalues_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_eigenvalues::get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_eigenvalues::get_command_string */

int Computed_field_is_type_eigenvalues(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_eigenvalues);
	if (field)
	{
		if (dynamic_cast<Computed_field_eigenvalues*>(field->core))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_eigenvalues.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_eigenvalues */

int Computed_field_is_type_eigenvalues_conditional(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
List conditional function version of Computed_field_is_type_eigenvalues.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_eigenvalues_conditional);
	USE_PARAMETER(dummy_void);
	return_code = Computed_field_is_type_eigenvalues(field);
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_eigenvalues_conditional */

} //namespace

int Computed_field_set_type_eigenvalues(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> to type 'eigenvalues' which performs a full eigenanalysis on
the symmetric matrix in <source_field> and returns the n eigenvalues, where
<source_field> must have n x n components.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_eigenvalues);
	if (field && source_field)
	{
		if (Computed_field_is_square_matrix(source_field, (void *)NULL))
		{
			/* 1. make dynamic allocations for any new type-specific data */
			number_of_source_fields = 1;
			if (ALLOCATE(source_fields,struct Computed_field *,
				number_of_source_fields))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->number_of_components =
					Computed_field_get_square_matrix_size(source_field);
				source_fields[0] = ACCESS(Computed_field)(source_field);
				field->source_fields = source_fields;
				field->number_of_source_fields = number_of_source_fields;
				field->core = new Computed_field_eigenvalues(field);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_eigenvalues.  Not enough memory");
				DEALLOCATE(source_fields);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_field_set_type_eigenvalues.  "
				"Field %s cannot hold a square matrix",source_field->name);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_eigenvalues.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_eigenvalues */

int Computed_field_get_type_eigenvalues(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type 'eigenvalues', the <source_field> it calculates the
eigenvalues of is returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_eigenvalues);
	if (field && (dynamic_cast<Computed_field_eigenvalues*>(field->core)) &&
		source_field)
	{
		*source_field = field->source_fields[0];
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_eigenvalues.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_eigenvalues */

int define_Computed_field_type_eigenvalues(struct Parse_state *state,
	void *field_void, void *computed_field_matrix_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type 'eigenvalues' (if it is not already) and allows its
contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field, *source_field;
	struct Computed_field_matrix_operations_package 
		*computed_field_matrix_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_eigenvalues);
	if (state && (field = (struct Computed_field *)field_void) &&
		(computed_field_matrix_operations_package=
			(struct Computed_field_matrix_operations_package *)
			computed_field_matrix_operations_package_void))
	{
		return_code=1;
		source_field = (struct Computed_field *)NULL;
		if (computed_field_eigenvalues_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_eigenvalues(field, &source_field);
		}
		if (return_code)
		{
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			option_table = CREATE(Option_table)();
			set_source_field_data.conditional_function =
				Computed_field_is_square_matrix;
			set_source_field_data.conditional_function_user_data = (void *)NULL;
			set_source_field_data.computed_field_manager =
				computed_field_matrix_operations_package->computed_field_manager;
			Option_table_add_entry(option_table, "field", &source_field,
				&set_source_field_data, set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table, state) &&
				Computed_field_set_type_eigenvalues(field, source_field);
			DESTROY(Option_table)(&option_table);
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_eigenvalues.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_eigenvalues */

namespace {

char computed_field_eigenvectors_type_string[] = "eigenvectors";

class Computed_field_eigenvectors : public Computed_field_core
{
public:
	Computed_field_eigenvectors(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_eigenvectors(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_eigenvectors_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_eigenvectors*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int evaluate();
};

int Computed_field_eigenvectors::evaluate()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Extracts the eigenvectors out of the source eigenvalues field.
Note the source field should already have been evaluated.
==============================================================================*/
{
	Computed_field_eigenvalues* eigenvalue_core;
	double *v;
	int i, j, n, return_code;
	struct Computed_field *source_field;
	
	ENTER(Computed_field_eigenvectors::evaluate);
	if (field)
	{
		source_field = field->source_fields[0];
		if (eigenvalue_core = dynamic_cast<Computed_field_eigenvalues*>
				(source_field->core))
		{
			if (v = eigenvalue_core->v)
			{
				n = source_field->number_of_components;
				/* return the vectors across the rows of the field values */
				for (i = 0; i < n; i++)
				{
					for (j = 0; j < n; j++)
					{
						field->values[i*n + j] = (FE_value)(v[j*n + i]);
					}
				}
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_eigenvectors::evaluate.  Missing eigenvalues cache");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_field_eigenvectors::evaluate.  "
				"Source field is not an eigenvalues field");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_eigenvectors::evaluate.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_eigenvectors::evaluate */

int Computed_field_eigenvectors::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_eigenvectors::evaluate_cache_at_location);
	if (field && location)
	{
		if (0 == location->get_number_of_derivatives())
		{
			field->derivatives_valid = 0;
			return_code = 
				Computed_field_evaluate_source_fields_cache_at_location(field, location) &&
				evaluate();
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_eigenvectors::evaluate_cache_at_location.  "
				"Cannot calculate derivatives of eigenvectors");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_eigenvectors::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_eigenvectors::evaluate_cache_at_location */


int Computed_field_eigenvectors::list()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_eigenvectors);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    eigenvalues field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_eigenvectors.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_eigenvectors */

char *Computed_field_eigenvectors::get_command_string()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_eigenvectors::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_eigenvectors_type_string, &error);
		append_string(&command_string, " eigenvalues ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_eigenvectors::get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_eigenvectors::get_command_string */

} //namespace

int Computed_field_set_type_eigenvectors(struct Computed_field *field,
	struct Computed_field *eigenvalues_field)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> to type 'eigenvectors' extracting the eigenvectors out of the
source <eigenvalues_field>. Sets the number of components equal to n x n, where
n is the number of components in the <eigenvalues_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int n, number_of_source_fields, return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_eigenvectors);
	if (field && eigenvalues_field)
	{
		if (Computed_field_is_type_eigenvalues(eigenvalues_field))
		{
			/* 1. make dynamic allocations for any new type-specific data */
			number_of_source_fields = 1;
			if (ALLOCATE(source_fields, struct Computed_field *,
				number_of_source_fields))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				n = eigenvalues_field->number_of_components;
				field->number_of_components = n * n;
				source_fields[0] = ACCESS(Computed_field)(eigenvalues_field);
				field->source_fields = source_fields;
				field->number_of_source_fields = number_of_source_fields;			
			field->core = new Computed_field_eigenvectors(field);
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_eigenvectors.  Not enough memory");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_field_set_type_eigenvectors.  "
				"Must be given an eigenvalues source field");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_eigenvectors.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_eigenvectors */

int Computed_field_get_type_eigenvectors(struct Computed_field *field,
	struct Computed_field **eigenvalues_field)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type 'eigenvectors', the <eigenvalues_field> used by it is
returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_eigenvectors);
	if (field && (dynamic_cast<Computed_field_eigenvectors*>(field->core)) &&
		eigenvalues_field)
	{
		*eigenvalues_field = field->source_fields[0];
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_eigenvectors.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_eigenvectors */

int define_Computed_field_type_eigenvectors(struct Parse_state *state,
	void *field_void,void *computed_field_matrix_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type 'eigenvectors' (if it is not  already) and allows
its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field, *source_field;
	struct Computed_field_matrix_operations_package 
		*computed_field_matrix_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_eigenvectors);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_matrix_operations_package=
		(struct Computed_field_matrix_operations_package *)
		computed_field_matrix_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		if (computed_field_eigenvectors_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_eigenvectors(field, &source_field);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			option_table = CREATE(Option_table)();
			/* field */
			set_source_field_data.computed_field_manager =
				computed_field_matrix_operations_package->computed_field_manager;
			set_source_field_data.conditional_function =
				Computed_field_is_type_eigenvalues_conditional;
			set_source_field_data.conditional_function_user_data = (void *)NULL;
			Option_table_add_entry(option_table, "eigenvalues", &source_field,
				&set_source_field_data, set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table,state) &&
				Computed_field_set_type_eigenvectors(field, source_field);
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_eigenvectors.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_eigenvectors */

namespace {

char computed_field_matrix_invert_type_string[] = "matrix_invert";

class Computed_field_matrix_invert : public Computed_field_core
{
public:
	/* cache for LU decomposed matrix, RHS vector and pivots */
	double *a, *b;
	int *indx;

	Computed_field_matrix_invert(Computed_field *field) : Computed_field_core(field)
	{
		a = (double*)NULL;
		b = (double*)NULL;
		indx = (int*)NULL;
	};

	~Computed_field_matrix_invert();

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_matrix_invert(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_matrix_invert_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_matrix_invert*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int clear_cache();

	int evaluate();
};

Computed_field_matrix_invert::~Computed_field_matrix_invert()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{

	ENTER(Computed_field_matrix_invert::~Computed_field_matrix_invert);
	if (field)
	{
		if (a)
		{
			DEALLOCATE(a);
		}
		if (b)
		{
			DEALLOCATE(b);
		}
		if (indx)
		{
			DEALLOCATE(indx);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_matrix_invert::~Computed_field_matrix_invert.  Invalid argument(s)");
	}
	LEAVE;

} /* Computed_field_matrix_invert::~Computed_field_matrix_invert */

int Computed_field_matrix_invert::clear_cache()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_matrix_invert::clear_cache);
	if (field)
	{
		if (a)
		{
			DEALLOCATE(a);
		}
		if (b)
		{
			DEALLOCATE(b);
		}
		if (indx)
		{
			DEALLOCATE(indx);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_matrix_invert::clear_cache.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_matrix_invert::clear_cache */

int Computed_field_matrix_invert::evaluate()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Evaluates the inverse of the matrix held in the <source_field>. The cache is
used to store intermediate LU-decomposed matrix and RHS vector in double
precision, as well as the integer pivot indx. Expects that source_field has
been pre-calculated before calling this.
==============================================================================*/
{
	double d;
	int i, j, matrix_size, n, return_code;
	struct Computed_field *source_field;
	
	ENTER(Computed_field_matrix_invert::evaluate);
	if (field)
	{
		source_field = field->source_fields[0];
		n = Computed_field_get_square_matrix_size(source_field);
		matrix_size = n * n;
		if ((a || ALLOCATE(a, double, matrix_size)) &&
			(b || ALLOCATE(b, double, n)) &&
			(indx || ALLOCATE(indx, int, n)))
		{
			for (i = 0; i < matrix_size; i++)
			{
				a[i] = (double)(source_field->values[i]);
			}
			if (LU_decompose(n, a, indx, &d,/*singular_tolerance*/1.0e-12))
			{
				return_code = 1;
				for (i = 0; (i < n) && return_code; i++)
				{
					/* take a column of the identity matrix */
					for (j = 0; j < n; j++)
					{
						b[j] = 0.0;
					}
					b[i] = 1.0;
					if (LU_backsubstitute(n, a, indx, b))
					{
						/* extract a column of the inverse matrix */
						for (j = 0; j < n; j++)
						{
							field->values[j*n + i] = (FE_value)b[j];
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_matrix_invert::evaluate.  "
							"Could not LU backsubstitute matrix");
						return_code = 0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_matrix_invert::evaluate.  "
					"Could not LU decompose matrix");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_matrix_invert::evaluate.  Could not allocate cache");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_matrix_invert::evaluate.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_matrix_invert::evaluate */

int Computed_field_matrix_invert::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Evaluate the fields cache in the element.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_matrix_invert::evaluate_cache_at_location);
	if (field && location)
	{
		if (0 == location->get_number_of_derivatives())
		{
			field->derivatives_valid = 0;
			return_code = Computed_field_evaluate_source_fields_cache_at_location(
				field, location)
				&& evaluate();
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_matrix_invert::evaluate_cache_at_location.  "
				"Cannot calculate derivatives of matrix_invert");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_matrix_invert::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_matrix_invert::evaluate_cache_at_location */


int Computed_field_matrix_invert::list()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_matrix_invert);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    source field : %s\n",
			field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_matrix_invert.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_matrix_invert */

char *Computed_field_matrix_invert::get_command_string()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_matrix_invert::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_matrix_invert_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_matrix_invert::get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_matrix_invert::get_command_string */

} //namespace

int Computed_field_set_type_matrix_invert(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> to type 'matrix_invert' which performs a full eigenanalysis on
the symmetric matrix in <source_field> and returns the n matrix_invert, where
<source_field> must have n x n components.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_matrix_invert);
	if (field && source_field)
	{
		if (Computed_field_is_square_matrix(source_field, (void *)NULL))
		{
			/* 1. make dynamic allocations for any new type-specific data */
			number_of_source_fields = 1;
			if (ALLOCATE(source_fields,struct Computed_field *,
				number_of_source_fields))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->number_of_components =
					Computed_field_get_number_of_components(source_field);
				source_fields[0] = ACCESS(Computed_field)(source_field);
				field->source_fields = source_fields;
				field->number_of_source_fields = number_of_source_fields;
				field->core = new Computed_field_matrix_invert(field);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_matrix_invert.  Not enough memory");
				DEALLOCATE(source_fields);
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_field_set_type_matrix_invert.  "
				"Field %s cannot hold a square matrix", source_field->name);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_matrix_invert.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_matrix_invert */

int Computed_field_get_type_matrix_invert(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type 'matrix_invert', the <source_field> it calculates the
matrix_invert of is returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_matrix_invert);
	if (field && (dynamic_cast<Computed_field_matrix_invert*>(field->core)) &&
		source_field)
	{
		*source_field = field->source_fields[0];
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_matrix_invert.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_matrix_invert */

int define_Computed_field_type_matrix_invert(struct Parse_state *state,
	void *field_void, void *computed_field_matrix_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type 'matrix_invert' (if it is not already) and allows its
contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field, *source_field;
	struct Computed_field_matrix_operations_package 
		*computed_field_matrix_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_matrix_invert);
	if (state && (field = (struct Computed_field *)field_void) &&
		(computed_field_matrix_operations_package=
			(struct Computed_field_matrix_operations_package *)
			computed_field_matrix_operations_package_void))
	{
		return_code = 1;
		source_field = (struct Computed_field *)NULL;
		if (computed_field_matrix_invert_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_matrix_invert(field, &source_field);
		}
		if (return_code)
		{
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			option_table = CREATE(Option_table)();
			set_source_field_data.conditional_function =
				Computed_field_is_square_matrix;
			set_source_field_data.conditional_function_user_data = (void *)NULL;
			set_source_field_data.computed_field_manager =
				computed_field_matrix_operations_package->computed_field_manager;
			Option_table_add_entry(option_table, "field", &source_field,
				&set_source_field_data, set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table, state) &&
				Computed_field_set_type_matrix_invert(field, source_field);
			DESTROY(Option_table)(&option_table);
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_matrix_invert.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_matrix_invert */

namespace {

char computed_field_matrix_multiply_type_string[] = "matrix_multiply";

class Computed_field_matrix_multiply : public Computed_field_core
{
public:
	int number_of_rows;

	Computed_field_matrix_multiply(Computed_field *field,
		int number_of_rows) :
		Computed_field_core(field), number_of_rows(number_of_rows)
									 
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_matrix_multiply(new_parent, number_of_rows);
	}

	char *get_type_string()
	{
		return(computed_field_matrix_multiply_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

int Computed_field_matrix_multiply::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	Computed_field_matrix_multiply* other;
	int return_code;

	ENTER(Computed_field_matrix_multiply::compare);
	if (field && (other = dynamic_cast<Computed_field_matrix_multiply*>(other_core)))
	{
		if (number_of_rows == other->number_of_rows)
		{
			return_code = 1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_matrix_multiply::compare */

int Computed_field_matrix_multiply::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Evaluate the fields cache in the element.
==============================================================================*/
{
	FE_value *a,*ad,*b,*bd,sum;
	int d, i, j, k, m, n, number_of_derivatives, return_code, s;

	ENTER(Computed_field_matrix_multiply::evaluate_cache_at_location);
	if (field && location && (m = number_of_rows)&&
		(n = field->source_fields[0]->number_of_components) &&
		(0 == (n % m)) && (s = n/m) &&
		(n = field->source_fields[1]->number_of_components) &&
		(0 == (n % s)) && (n /= s))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			a=field->source_fields[0]->values;
			b=field->source_fields[1]->values;
			for (i=0;i<m;i++)
			{
				for (j=0;j<n;j++)
				{
					sum=0.0;
					for (k=0;k<s;k++)
					{
						sum += a[i*s+k] * b[k*n+j];
					}
					field->values[i*n+j]=sum;
				}
			}
			number_of_derivatives = location->get_number_of_derivatives();
			if (0 < number_of_derivatives)
			{
				for (d=0;d<number_of_derivatives;d++)
				{
					/* use the product rule */
					a = field->source_fields[0]->values;
					ad = field->source_fields[0]->derivatives+d;
					b = field->source_fields[1]->values;
					bd = field->source_fields[1]->derivatives+d;
					for (i=0;i<m;i++)
					{
						for (j=0;j<n;j++)
						{
							sum=0.0;
							for (k=0;k<s;k++)
							{
								sum += a[i*s+k] * bd[number_of_derivatives*(k*n+j)] +
									ad[number_of_derivatives*(i*s+k)] * b[k*n+j];
							}
							field->derivatives[number_of_derivatives*(i*n+j)+d]=sum;
						}
					}
				}
				field->derivatives_valid=1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_matrix_multiply::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_matrix_multiply::evaluate_cache_at_location */


int Computed_field_matrix_multiply::list()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_matrix_multiply);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    number of rows : %d\n",number_of_rows);
		display_message(INFORMATION_MESSAGE,"    source fields : %s %s\n",
			field->source_fields[0]->name,field->source_fields[1]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_matrix_multiply.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_matrix_multiply */

char *Computed_field_matrix_multiply::get_command_string()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error;

	ENTER(Computed_field_matrix_multiply::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_matrix_multiply_type_string, &error);
		sprintf(temp_string, " number_of_rows %d", number_of_rows);
		append_string(&command_string, temp_string, &error);
		append_string(&command_string, " fields ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_matrix_multiply::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_matrix_multiply::get_command_string */

} //namespace

int Computed_field_set_type_matrix_multiply(struct Computed_field *field,
	int number_of_rows,struct Computed_field *source_field1,
	struct Computed_field *source_field2)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_MATRIX_MULTIPLY producing the result
<source_field1> x <source_field2>, with <number_of_rows> rows in both
<source_field1> and the result. From the <number_of_rows> the columns in
<source_field1>, rows in <source_field2> and then columns in <source_field2>
are implied and checked.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int n, number_of_source_fields, return_code, s;
	struct Computed_field **temp_source_fields;

	ENTER(Computed_field_set_type_matrix_multiply);
	if (field && (0 < number_of_rows) && source_field1 && source_field2)
	{
		if ((n = source_field1->number_of_components) &&
			(0 == (n % number_of_rows)) && (s = n/number_of_rows))
		{
			if ((n = source_field2->number_of_components) &&
				(0 == (n % s)) && (n /= s))
			{
				/* 1. make dynamic allocations for any new type-specific data */
				number_of_source_fields=2;
				if (ALLOCATE(temp_source_fields,struct Computed_field *,
					number_of_source_fields))
				{
					/* 2. free current type-specific data */
					Computed_field_clear_type(field);
					/* 3. establish the new type */
					field->number_of_components = number_of_rows*n;
					temp_source_fields[0]=ACCESS(Computed_field)(source_field1);
					temp_source_fields[1]=ACCESS(Computed_field)(source_field2);
					field->source_fields=temp_source_fields;
					field->number_of_source_fields=number_of_source_fields;
					field->core = new Computed_field_matrix_multiply(field,
						number_of_rows);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_set_type_matrix_multiply.  "
						"Unable to allocate memory");
					DEALLOCATE(temp_source_fields);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_matrix_multiply.  "
					"Field %s has wrong number of components",source_field2->name);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_matrix_multiply.  "
				"Field %s has wrong number of components",source_field1->name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_matrix_multiply.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_matrix_multiply */

int Computed_field_get_type_matrix_multiply(struct Computed_field *field,
	int *number_of_rows, struct Computed_field **source_field1,
	struct Computed_field **source_field2)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_MATRIX_MULTIPLY, the 
<number_of_rows> and <source_fields> used by it are returned.
==============================================================================*/
{
	Computed_field_matrix_multiply* core;
	int return_code;

	ENTER(Computed_field_get_type_matrix_multiply);
	if (field && (core=dynamic_cast<Computed_field_matrix_multiply*>(field->core)) &&
		source_field1 && source_field2)
	{
		*number_of_rows = core->number_of_rows;
		*source_field1 = field->source_fields[0];
		*source_field2 = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_matrix_multiply.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_matrix_multiply */

int define_Computed_field_type_matrix_multiply(struct Parse_state *state,
	void *field_void,void *computed_field_matrix_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_MATRIX_MULTIPLY (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	int i, number_of_rows, return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_matrix_operations_package 
		*computed_field_matrix_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_field_array_data;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(define_Computed_field_type_matrix_multiply);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_matrix_operations_package=
		(struct Computed_field_matrix_operations_package *)
		computed_field_matrix_operations_package_void))
	{
		return_code=1;
		if (ALLOCATE(source_fields,struct Computed_field *,2))
		{
			/* get valid parameters for matrix_multiply field */
			number_of_rows = 1;
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if (computed_field_matrix_multiply_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_matrix_multiply(field,
					&number_of_rows,&(source_fields[0]),&(source_fields[1]));
			}
			if (return_code)
			{
				/* ACCESS the source fields for set_Computed_field_array */
				for (i=0;i<2;i++)
				{
					if (source_fields[i])
					{
						ACCESS(Computed_field)(source_fields[i]);
					}
				}
				/* try to handle help first */
				if (current_token=state->current_token)
				{
					if (!(strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
					{
						option_table = CREATE(Option_table)();
						Option_table_add_entry(option_table,"number_of_rows",
							&number_of_rows,NULL,set_int_positive);
						set_field_data.conditional_function=
							Computed_field_has_numerical_components;
						set_field_data.conditional_function_user_data=(void *)NULL;
						set_field_data.computed_field_manager=
							computed_field_matrix_operations_package->computed_field_manager;
						set_field_array_data.number_of_fields=2;
						set_field_array_data.conditional_data= &set_field_data;
						Option_table_add_entry(option_table,"fields",source_fields,
							&set_field_array_data,set_Computed_field_array);
						return_code=Option_table_multi_parse(option_table,state);
						DESTROY(Option_table)(&option_table);
					}
					else
					{
						/* if the "number_of_rows" token is next, read it */
						if (fuzzy_string_compare(current_token,"number_of_rows"))
						{
							option_table = CREATE(Option_table)();
							/* number_of_rows */
							Option_table_add_entry(option_table,"number_of_rows",
								&number_of_rows,NULL,set_int_positive);
							return_code = Option_table_parse(option_table,state);
							DESTROY(Option_table)(&option_table);
						}
						if (return_code)
						{
							option_table = CREATE(Option_table)();
							set_field_data.conditional_function=
								Computed_field_has_numerical_components;
							set_field_data.conditional_function_user_data=(void *)NULL;
							set_field_data.computed_field_manager=
								computed_field_matrix_operations_package->
								computed_field_manager;
							set_field_array_data.number_of_fields=2;
							set_field_array_data.conditional_data= &set_field_data;
							Option_table_add_entry(option_table,"fields",source_fields,
								&set_field_array_data,set_Computed_field_array);
							if (return_code=Option_table_multi_parse(option_table,state))
							{
								return_code = Computed_field_set_type_matrix_multiply(field,
									number_of_rows,source_fields[0],source_fields[1]);
							}
							DESTROY(Option_table)(&option_table);
						}
						if (!return_code)
						{
							/* error */
							display_message(ERROR_MESSAGE,
								"define_Computed_field_type_matrix_multiply.  Failed");
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "Missing command options");
					return_code = 0;
				}
				for (i=0;i<2;i++)
				{
					if (source_fields[i])
					{
						DEACCESS(Computed_field)(&(source_fields[i]));
					}
				}
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_matrix_multiply.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_matrix_multiply.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_matrix_multiply */

namespace {

char computed_field_projection_type_string[] = "projection";

class Computed_field_projection : public Computed_field_core
{
public:
	double *projection_matrix;

	Computed_field_projection(Computed_field *field,
		double* projection_matrix_in) : Computed_field_core(field)
	{
		int i;
		int number_of_projection_values = 
			(field->source_fields[0]->number_of_components + 1)
			* (field->number_of_components + 1);
		projection_matrix = new double[number_of_projection_values];
		for (i = 0 ; i < number_of_projection_values ; i++)
		{
			projection_matrix[i] = projection_matrix_in[i];
		}
	};

	~Computed_field_projection();

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_projection(new_parent, projection_matrix);
	}

	char *get_type_string()
	{
		return(computed_field_projection_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int evaluate(int number_of_derivatives);
};

Computed_field_projection::~Computed_field_projection()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{

	ENTER(Computed_field_projection::~Computed_field_projection);
	if (field)
	{
		DEALLOCATE(projection_matrix);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_projection::~Computed_field_projection.  Invalid argument(s)");
	}
	LEAVE;

} /* Computed_field_projection::~Computed_field_projection */

int Computed_field_projection::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Compare the type specific data.
==============================================================================*/
{
	Computed_field_projection* other;
	int i, number_of_projection_values, return_code;

	ENTER(Computed_field_projection::compare);
	if (field && (other = dynamic_cast<Computed_field_projection*>(other_core)))
	{
		number_of_projection_values =
			(field->source_fields[0]->number_of_components + 1) *
			(field->number_of_components + 1);
		if (number_of_projection_values ==
			((other->field->source_fields[0]->number_of_components + 1) *
				(other->field->number_of_components + 1)))
		{
			return_code=1;
			for (i=0;return_code&&(i<number_of_projection_values);i++)
			{
				if (projection_matrix[i] != other->projection_matrix[i])
				{
					return_code=0;
				}
			}
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_projection::compare */

int Computed_field_projection::evaluate(int number_of_derivatives)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Function called by Computed_field_evaluate_in_element to compute a field
transformed by a projection matrix.
NOTE: Assumes that values and derivatives arrays are already allocated in
<field>, and that its source_fields are already computed (incl. derivatives)
for the same element, with the given <number_of_derivatives> = number of Xi coords.
==============================================================================*/
{
	double dhdxi, dh1dxi, perspective;
	int coordinate_components, i, j, k,return_code;

	ENTER(Computed_field_projection::evaluate);
	if (field)
	{
		if (number_of_derivatives > 0)
		{
			field->derivatives_valid=1;
		}
		else
		{
			field->derivatives_valid=0;
		}

		/* Calculate the transformed coordinates */
		coordinate_components=field->source_fields[0]->number_of_components;
		for (i = 0 ; i < field->number_of_components ; i++)
		{
			field->values[i] = 0.0;
			for (j = 0 ; j < coordinate_components ; j++)
			{
 				field->values[i] +=
					projection_matrix[i * (coordinate_components + 1) + j] *
					field->source_fields[0]->values[j];
			}
			/* The last source value is fixed at 1 */
			field->values[i] +=  projection_matrix[
				i * (coordinate_components + 1) + coordinate_components];
		}

		/* The last calculated value is the perspective value which divides through
			all the other components */
		perspective = 0.0;
		for (j = 0 ; j < coordinate_components ; j++)
		{
			perspective += projection_matrix[field->number_of_components
				* (coordinate_components + 1) + j] * field->source_fields[0]->values[j];
		}
		perspective += projection_matrix[field->number_of_components 
			* (coordinate_components + 1) + coordinate_components];
		
		if (number_of_derivatives > 0)
		{
			for (k=0;k<number_of_derivatives;k++)
			{
				/* Calculate the coordinate derivatives without perspective */
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					field->derivatives[i * number_of_derivatives + k] = 0.0;
					for (j = 0 ; j < coordinate_components ; j++)
					{
						field->derivatives[i * number_of_derivatives + k] += 
							projection_matrix[i * (coordinate_components + 1) + j]
							* field->source_fields[0]->derivatives[j * number_of_derivatives + k];
					}
				}

				/* Calculate the perspective derivative */
				dhdxi = 0.0;
				for (j = 0 ; j < coordinate_components ; j++)
				{
					dhdxi += projection_matrix[field->number_of_components 
						* (coordinate_components + 1) + j]
						* field->source_fields[0]->derivatives[j *number_of_derivatives + k];
				}

				/* Calculate the perspective reciprocal derivative using chaing rule */
				dh1dxi = (-1.0) / (perspective * perspective) * dhdxi;

				/* Calculate the derivatives of the perspective scaled transformed
					 coordinates, which is ultimately what we want */
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					field->derivatives[i * number_of_derivatives + k] = 
						field->derivatives[i * number_of_derivatives + k] / perspective
						+ field->values[i] * dh1dxi;
				}
			}
		}

		/* Now apply the perspective scaling to the non derivative transformed
			 coordinates */
		for (i = 0 ; i < field->number_of_components ; i++)
		{
			field->values[i] /= perspective;
		}

		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_projection::evaluate.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_projection::evaluate */

int Computed_field_projection::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Evaluate the fields cache in the element.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_projection::evaluate_cache_at_location);
	if (field && location)
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			return_code=evaluate(location->get_number_of_derivatives());
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_projection::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_projection::evaluate_cache_at_location */


int Computed_field_projection::list()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int i,number_of_projection_values,return_code;

	ENTER(List_Computed_field_projection);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    source field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    projection_matrix : ");
		number_of_projection_values =
			(field->source_fields[0]->number_of_components + 1)
			* (field->number_of_components + 1);
		for (i=0;i<number_of_projection_values;i++)
		{
			display_message(INFORMATION_MESSAGE," %g",projection_matrix[i]);
		}
		display_message(INFORMATION_MESSAGE,"\n");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_projection.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_projection */

char *Computed_field_projection::get_command_string()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error, i, number_of_projection_values;

	ENTER(Computed_field_projection::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_projection_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		sprintf(temp_string, " number_of_components %d",
			field->number_of_components);
		append_string(&command_string, temp_string, &error);
		append_string(&command_string, " projection_matrix", &error);
		number_of_projection_values =
			(field->source_fields[0]->number_of_components + 1)
			* (field->number_of_components + 1);
		for (i = 0; i < number_of_projection_values; i++)
		{
			sprintf(temp_string, " %g", projection_matrix[i]);
			append_string(&command_string, temp_string, &error);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_projection::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_projection::get_command_string */

} //namespace

int Computed_field_set_type_projection(struct Computed_field *field,
	struct Computed_field *source_field, int number_of_components, 
	double *projection_matrix)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_PROJECTION, returning the <source_field>
with each component multiplied by the perspective <projection_matrix>.
The <projection_matrix> array must be of size
<source_field->number_of_components + 1> * <field->number_of_components + 1>.
The source vector is appended with a 1 to make
source_field->number_of_components + 1 components. The extra calculated value
is a perspective value which divides through each of the other components.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_projection);
	if (field&&source_field&&projection_matrix)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components=number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			field->core = new Computed_field_projection(field, projection_matrix);
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_projection.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_projection */

int Computed_field_get_type_projection(struct Computed_field *field,
	struct Computed_field **source_field, int *number_of_components,
	double **projection_matrix)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_PROJECTION, the source_field and
projection matrix used by it are returned. Since the number of projections is
equal to the number of components in the source_field (and you don't know this
yet), this function returns in *projection_matrix a pointer to an allocated 
array containing the values.
It is up to the calling function to DEALLOCATE returned <*projection_matrix>.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	Computed_field_projection* core;
	int number_of_projection_values,return_code;

	ENTER(Computed_field_get_type_projection);
	if (field && (core = dynamic_cast<Computed_field_projection*>(field->core)) &&
		source_field && number_of_components && projection_matrix)
	{
		number_of_projection_values =
			(field->source_fields[0]->number_of_components + 1) *
			(field->number_of_components + 1);
		if (ALLOCATE(*projection_matrix,double,number_of_projection_values))
		{
			*number_of_components = field->number_of_components;
			*source_field=field->source_fields[0];
			memcpy(*projection_matrix,core->projection_matrix,
				number_of_projection_values*sizeof(double));
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_projection.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_projection.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_projection */

int define_Computed_field_type_projection(struct Parse_state *state,
	void *field_void,void *computed_field_matrix_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_PROJECTION (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	double *projection_matrix, *temp_projection_matrix;
	int i, number_of_components, number_of_projection_values, return_code,
		temp_number_of_projection_values;
	struct Computed_field *field, *source_field;
	struct Computed_field_matrix_operations_package 
		*computed_field_matrix_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_projection);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_matrix_operations_package=
			(struct Computed_field_matrix_operations_package *)
			computed_field_matrix_operations_package_void))
	{
		return_code = 1;
		set_source_field_data.conditional_function =
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		set_source_field_data.computed_field_manager =
			computed_field_matrix_operations_package->computed_field_manager;
		source_field = (struct Computed_field *)NULL;
		projection_matrix = (double *)NULL;
		if (dynamic_cast<Computed_field_projection*>(field->core))
		{
			if (return_code = Computed_field_get_type_projection(field,
				&source_field, &number_of_components, &projection_matrix))
			{
				number_of_projection_values = (source_field->number_of_components + 1)
					* (number_of_components + 1);
			}
		}
		else
		{
			/* ALLOCATE and fill array of projection_matrix - with identity */
			number_of_components = 0;
			number_of_projection_values = 1;
			if (ALLOCATE(projection_matrix, double, 1))
			{
				projection_matrix[0] = 0.0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_projection.  Not enough memory");
				return_code = 0;
			}
		}
		if (return_code)
		{
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}

			/* try to handle help first */
			if ((current_token = state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				option_table = CREATE(Option_table)();					
				Option_table_add_entry(option_table, "field", &source_field,
					&set_source_field_data, set_Computed_field_conditional);
				Option_table_add_entry(option_table, "number_of_components",
					&number_of_components, NULL, set_int_positive);
				Option_table_add_entry(option_table, "projection_matrix",
					projection_matrix, &number_of_projection_values,
					set_double_vector);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}

			/* keep the number_of_projection_values to maintain any current ones */
			temp_number_of_projection_values = number_of_projection_values;
			/* parse the field... */
			if (return_code)
			{
				/* ... only if the "field" token is next or no source_field */
				if ((current_token = state->current_token) &&
					fuzzy_string_compare(current_token, "field"))
				{
					option_table = CREATE(Option_table)();
					/* field */
					Option_table_add_entry(option_table, "field", &source_field,
						&set_source_field_data, set_Computed_field_conditional);
					return_code = Option_table_parse(option_table, state);
					DESTROY(Option_table)(&option_table);
				}
				if (return_code && (!source_field))
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_projection.  Must specify field first");
					return_code = 0;
				}
			}

			/* parse the number_of_components... */
			if (return_code && (current_token = state->current_token))
			{
				/* ... only if the "number_of_components" token is next */
				if (fuzzy_string_compare(current_token, "number_of_components"))
				{
					option_table = CREATE(Option_table)();					
					Option_table_add_entry(option_table, "number_of_components",
						&number_of_components, NULL, set_int_positive);
					return_code = Option_table_parse(option_table, state);
					DESTROY(Option_table)(&option_table);
				}
			}

			/* ensure projection matrix is correct size; set new values to zero */
			if (return_code)
			{
				number_of_projection_values = (source_field->number_of_components + 1)
					* (number_of_components + 1);
				if (temp_number_of_projection_values != number_of_projection_values)
				{
					if (REALLOCATE(temp_projection_matrix, projection_matrix, double,
						number_of_projection_values))
					{
						projection_matrix = temp_projection_matrix;
						/* clear any new projection_matrix to zero */
						for (i = temp_number_of_projection_values;
							i < number_of_projection_values; i++)
						{
							projection_matrix[i] = 0.0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_projection.  "
							"Could not reallocate projection matrix");
						return_code = 0;
					}
				}
			}

			/* parse the projection_matrix */
			if (return_code && state->current_token)
			{
				option_table = CREATE(Option_table)();					
				Option_table_add_entry(option_table, "projection_matrix",
					projection_matrix, &number_of_projection_values,
					set_double_vector);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			if (return_code)
			{
				if (Computed_field_set_type_projection(field, source_field,
					number_of_components, projection_matrix))
				{
					return_code = 1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_projection.  Failed");
					return_code = 0;
				}
			}
			/* clean up the projection_matrix array */
			DEALLOCATE(projection_matrix);
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_projection.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_projection */

namespace {

char computed_field_transpose_type_string[] = "transpose";

class Computed_field_transpose : public Computed_field_core
{
public:
	int source_number_of_rows;

	Computed_field_transpose(Computed_field *field, int source_number_of_rows) : 
		Computed_field_core(field), source_number_of_rows(source_number_of_rows)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_transpose(new_parent, source_number_of_rows);
	}

	char *get_type_string()
	{
		return(computed_field_transpose_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

int Computed_field_transpose::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	Computed_field_transpose *other;
	int return_code;

	ENTER(Computed_field_transpose::compare);
	if (field && (other = dynamic_cast<Computed_field_transpose*>(other_core)))
	{
		if (source_number_of_rows == other->source_number_of_rows)
		{
			return_code = 1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_transpose::compare */

int Computed_field_transpose::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Evaluate the fields cache in the element.
==============================================================================*/
{
	FE_value *destination_derivatives, *source_derivatives, *source_values;
	int d, number_of_derivatives, i, j, m, n, return_code;

	ENTER(Computed_field_transpose::evaluate_cache_at_location);
	if (field && location && (m = source_number_of_rows) &&
		(n = (field->source_fields[0]->number_of_components / m)))
	{
		/* returns n row x m column tranpose of m row x n column source field,
			 where values always change along rows fastest */
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = Computed_field_evaluate_source_fields_cache_at_location(
			field, location))
		{
			/* 2. Calculate the field */
			source_values = field->source_fields[0]->values;
			for (i = 0; i < n; i++)
			{
				for (j = 0; j < m; j++)
				{
					field->values[i*m + j] = source_values[j*n + i];
				}
			}
			number_of_derivatives = location->get_number_of_derivatives();
			if (0 < number_of_derivatives)
			{
				/* transpose derivatives in same way as values */
				for (i = 0; i < n; i++)
				{
					for (j = 0; j < m; j++)
					{
						source_derivatives = field->source_fields[0]->derivatives +
							number_of_derivatives*(j*n + i);
						destination_derivatives = field->derivatives +
							number_of_derivatives*(i*m + j);
						for (d = 0; d < number_of_derivatives; d++)
						{
							destination_derivatives[d] = source_derivatives[d];
						}
					}
				}
				field->derivatives_valid=1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_transpose::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_transpose::evaluate_cache_at_location */


int Computed_field_transpose::list()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_transpose);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source number of rows : %d\n",source_number_of_rows);
		display_message(INFORMATION_MESSAGE,"    source field : %s\n",
			field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_transpose.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_transpose */

char *Computed_field_transpose::get_command_string()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error;

	ENTER(Computed_field_transpose::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_transpose_type_string, &error);
		sprintf(temp_string, " source_number_of_rows %d",
			source_number_of_rows);
		append_string(&command_string, temp_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_transpose::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_transpose::get_command_string */

} //namespace

int Computed_field_set_type_transpose(struct Computed_field *field,
	int source_number_of_rows,struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_TRANSPOSE which computes the transpose
of the <source_number_of_rows> by source_number_of_columns <source_field>. The
source_number_of_columns is computed as source_field->number_of_components
divided by <source_number_of_rows>, and this division must have no remainder.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **temp_source_fields;

	ENTER(Computed_field_set_type_transpose);
	if (field && (0 < source_number_of_rows) && source_field)
	{
		if (0 == (source_field->number_of_components % source_number_of_rows))
		{
			/* 1. make dynamic allocations for any new type-specific data */
			number_of_source_fields=1;
			if (ALLOCATE(temp_source_fields,struct Computed_field *,
				number_of_source_fields))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->number_of_components = source_field->number_of_components;
				temp_source_fields[0] = ACCESS(Computed_field)(source_field);
				field->source_fields = temp_source_fields;
				field->number_of_source_fields = number_of_source_fields;
				field->core = new Computed_field_transpose(field, source_number_of_rows);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_transpose.  Unable to allocate memory");
				DEALLOCATE(temp_source_fields);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_field_set_type_transpose.  "
				"Source field %s has %d components, hence cannot have %d rows",
				source_field->name,source_field->number_of_components,
				source_number_of_rows);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_transpose.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_transpose */

int Computed_field_get_type_transpose(struct Computed_field *field,
	int *source_number_of_rows, struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_TRANSPOSE, the 
<source_number_of_rows> and <source_field> used by it are returned.
==============================================================================*/
{
	Computed_field_transpose* core;
	int return_code;

	ENTER(Computed_field_get_type_transpose);
	if (field && (core = dynamic_cast<Computed_field_transpose*>(field->core)) &&
		source_field)
	{
		*source_number_of_rows = core->source_number_of_rows;
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_transpose.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_transpose */

int define_Computed_field_type_transpose(struct Parse_state *state,
	void *field_void,void *computed_field_matrix_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_TRANSPOSE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	int source_number_of_rows, return_code;
	struct Computed_field *field,*source_field;
	struct Computed_field_matrix_operations_package 
		*computed_field_matrix_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_transpose);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_matrix_operations_package=
		(struct Computed_field_matrix_operations_package *)
		computed_field_matrix_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for transpose field */
		source_number_of_rows = 1;
		source_field = (struct Computed_field *)NULL;
		if (computed_field_transpose_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_transpose(field,
				&source_number_of_rows,&source_field);
		}
		if (return_code)
		{
			/* ACCESS the source fields for set_Computed_field_conditional */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			/* try to handle help first */
			if (current_token = state->current_token)
			{
				set_source_field_data.conditional_function =
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data = (void *)NULL;
				set_source_field_data.computed_field_manager =
					computed_field_matrix_operations_package->computed_field_manager;
				if (!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
				{
					option_table = CREATE(Option_table)();
					Option_table_add_entry(option_table,"source_number_of_rows",
						&source_number_of_rows,NULL,set_int_positive);
					Option_table_add_entry(option_table,"field",&source_field,
						&set_source_field_data,set_Computed_field_conditional);
					return_code=Option_table_multi_parse(option_table,state);
					DESTROY(Option_table)(&option_table);
				}
				else
				{
					/* if the "source_number_of_rows" token is next, read it */
					if (fuzzy_string_compare(current_token,"source_number_of_rows"))
					{
						option_table = CREATE(Option_table)();
						/* source_number_of_rows */
						Option_table_add_entry(option_table,"source_number_of_rows",
							&source_number_of_rows,NULL,set_int_positive);
						return_code = Option_table_parse(option_table,state);
						DESTROY(Option_table)(&option_table);
					}
					if (return_code)
					{
						option_table = CREATE(Option_table)();
						Option_table_add_entry(option_table,"field",&source_field,
							&set_source_field_data,set_Computed_field_conditional);
						return_code = Option_table_multi_parse(option_table,state) &&
							Computed_field_set_type_transpose(field,
								source_number_of_rows,source_field);
						DESTROY(Option_table)(&option_table);
					}
					if (!return_code)
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_transpose.  Failed");
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "Missing command options");
				return_code = 0;
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_transpose.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_transpose */

int Computed_field_register_types_matrix_operations(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_matrix_operations_package 
		computed_field_matrix_operations_package;

	ENTER(Computed_field_register_types_matrix_operations);
	if (computed_field_package)
	{
		computed_field_matrix_operations_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);

		return_code =
			Computed_field_package_add_type(computed_field_package,
				computed_field_eigenvalues_type_string,
				define_Computed_field_type_eigenvalues,
				&computed_field_matrix_operations_package) &&
			Computed_field_package_add_type(computed_field_package,
				computed_field_eigenvectors_type_string,
				define_Computed_field_type_eigenvectors,
				&computed_field_matrix_operations_package) &&
			Computed_field_package_add_type(computed_field_package,
				computed_field_matrix_invert_type_string,
				define_Computed_field_type_matrix_invert,
				&computed_field_matrix_operations_package) &&
			Computed_field_package_add_type(computed_field_package,
				computed_field_matrix_multiply_type_string,
				define_Computed_field_type_matrix_multiply,
				&computed_field_matrix_operations_package) &&
			Computed_field_package_add_type(computed_field_package,
				computed_field_projection_type_string,
				define_Computed_field_type_projection,
				&computed_field_matrix_operations_package) &&
			Computed_field_package_add_type(computed_field_package,
				computed_field_transpose_type_string,
				define_Computed_field_type_transpose,
				&computed_field_matrix_operations_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_matrix_operations.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_matrix_operations */

