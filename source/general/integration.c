/*******************************************************************************
FILE : integration.c

LAST MODIFIED : 29 December 2002

DESCRIPTION :
Structures and functions for integerating a computed field over a group of
elements.

???DB.  What about time integrals?
==============================================================================*/

#include "general/debug.h"
#include "general/integration.h"
#include "general/mystring.h"

/*
Module types
------------
*/
struct Integration_scheme
/*******************************************************************************
LAST MODIFIED : 26 December 2002

DESCRIPTION :
An object which when given an element will return the weights and abscissae to
be used when integrating over the element.

???DB.  cm has weights and abscissae for each basis.  A computed field may use
	FE fields with different bases for the same element - if weights and abscissae
	were stored with basis it wouldn't be clear which to use
???DB.  May want to have many ways of setting the weights and abscissae for
	elements eg shape, basis for a particular field.  Start with always same
==============================================================================*/
{
	/* the name of the scheme */
	char *name;
	/* the dimension of the elements that can be integrated by this scheme */
	int dimension;
	/* default weights and abscissae.  NB there are
		dimension*number_of_weights_abscissae values is abscissae with xi varying
		fastest */
	int default_number_of_weights_abscissae;
	FE_value *default_abscissae,*default_weights;
}; /* struct Integration_scheme */

struct Integrate_Computed_field_over_element_data
{
	FE_value *result,time,*values;
	int error_code,number_of_components;
	struct Computed_field *field;
	struct Integration_scheme *scheme;
}; /* struct Integrate_Computed_field_over_element_data */

/*
Module functions
----------------
*/
/*???DB.  These functions are part of Richard's update of finite_element and
	will be removed when the update is checked in.  Begin */
static int FE_basis_get_dimension(struct FE_basis *basis,
	int *dimension_address)
/*******************************************************************************
LAST MODIFIED : 6 November 2002

DESCRIPTION :
Returns the dimension of <basis>.
If fails, puts zero at <dimension_address>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_basis_get_dimension);
	if (basis && basis->type && dimension_address)
	{
		*dimension_address = basis->type[0];
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_basis_get_dimension.  Invalid argument(s)");
		if (dimension_address)
		{
			*dimension_address = 0;
		}
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_basis_get_dimension */

static int FE_basis_get_xi_basis_type(struct FE_basis *basis,
	int xi_number, enum FE_basis_type *basis_type_address)
/*******************************************************************************
LAST MODIFIED : 29 December 2002

DESCRIPTION :
Returns the basis type of <basis> on <xi_number> -- on main diagonal of
type array. The first xi_number is 0.
==============================================================================*/
{
	int i, offset, return_code;

	ENTER(FE_basis_get_xi_basis_type);
	if (basis && basis->type && (0 <= xi_number) &&
		(xi_number < basis->type[0]) && basis_type_address)
	{
		/* first value in basis->type is the dimension */
		offset = 1;
		for (i = 0; i < xi_number; i++)
		{
			offset += basis->type[0] - i;
		}
		*basis_type_address=(enum FE_basis_type)(basis->type[offset]);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_basis_get_xi_basis_type.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_basis_get_xi_basis_type */

static int FE_basis_get_next_linked_xi_number(
	struct FE_basis *basis, int xi_number,
	int *next_xi_number_address, int *xi_link_number_address)
/*******************************************************************************
LAST MODIFIED : 29 December 2002

DESCRIPTION :
Returns in <next_xi_number_address> the next xi number higher than <xi_number>
which is linked in basis with it, plus in <xi_link_number_address> the number
denoting how it is linked; currently used only for polygon basiss to denote the
number of polygon sides.
If there is no remaining linked dimension, 0 is returned in both addresses.
<xi_number> is from 0 to one less than the basis dimension.
Also checks that the linked xi numbers have the same basis type.
==============================================================================*/
{
	int basis_type,dimension,i,limit,offset,return_code;

	ENTER(FE_basis_get_next_linked_xi_number);
	if (basis && basis->type &&
		(0 <= xi_number) && (xi_number < (dimension=basis->type[0])) &&
		next_xi_number_address && xi_link_number_address)
	{
		return_code = 1;
		offset = 0;
		for (i = 0; i < xi_number; i++)
		{
			offset += dimension - i;
		}
		basis_type = basis->type[offset];
		limit = dimension - xi_number;
		offset++;
		for (i = 1; (i < limit) && (0 == basis->type[offset]); i++)
		{
			offset++;
		}
		if (i < limit)
		{
			*next_xi_number_address = i;
			*xi_link_number_address = basis->type[offset];
			/* finally check the basis type matches */
			offset = 0;
			for (i = 0; i < *next_xi_number_address; i++)
			{
				offset += dimension - i;
			}
			if (basis->type[offset] != basis_type)
			{
				display_message(ERROR_MESSAGE,
					"FE_basis_get_next_linked_xi_number.  "
					"Basis has linked xi directions with different basis type");
				return_code = 0;
			}
		}
		else
		{
			*next_xi_number_address = 0;
			*xi_link_number_address = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_basis_get_next_linked_xi_number.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_basis_get_next_linked_xi_number */
/*???DB.  These functions are part of Richard's update of finite_element and
	will be removed when the update is checked in.  End */

static int Integration_scheme_get_dimension(struct Integration_scheme *scheme,
	int *dimension_address)
/*******************************************************************************
LAST MODIFIED : 27 December 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Integration_scheme_get_dimension);
	return_code=0;
	if (scheme&&dimension_address)
	{
		*dimension_address=scheme->dimension;
	}
	LEAVE;

	return (return_code);
} /* Integration_scheme_get_dimension */

static int Integration_scheme_get_weights_and_abscissae(
	struct Integration_scheme *scheme,struct FE_element *element,
	int *number_of_weights_abscissae_address,
	FE_value **weights_address,FE_value **abscissae_address)
/*******************************************************************************
LAST MODIFIED : 27 December 2002

DESCRIPTION :
Returns the number of weights/abscissae, the weights and the abscissae for the
<element>.

NB.  The <scheme> handles the memory for the weights and abscissae.
<*weights_address> and <*abscissae_address> should not be DEALLOCATE'd/free'd.
==============================================================================*/
{
	int return_code;

	ENTER(Integration_scheme_get_weights_and_abscissae);
	return_code=0;
	if (scheme&&element&&number_of_weights_abscissae_address&&weights_address&&
		abscissae_address)
	{
		*number_of_weights_abscissae_address=
			scheme->default_number_of_weights_abscissae;
		*weights_address=scheme->default_weights;
		*abscissae_address=scheme->default_abscissae;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Integration_scheme_get_weights_and_abscissae.  "
			"Invalid argument(s).  %p %p %p %p %p",scheme,element,
			number_of_weights_abscissae_address,weights_address,abscissae_address);
	}
	LEAVE;

	return (return_code);
} /* Integration_scheme_get_weights_and_abscissae */

static int integrate_Computed_field_over_element(struct FE_element *element,
	void *integrate_Computed_field_over_element_data_void)
/*******************************************************************************
LAST MODIFIED : 27 December 2002

DESCRIPTION :
Calculates the integral of the field over the <element> at the specified time
using the given scheme.  The result array needs to be big enough to hold one
value for each component of the field.
==============================================================================*/
{
	FE_value *abscissa,*result,time,*values,*weight;
	int dimension,i,j,number_of_components,return_code;
	struct Computed_field *field;
	struct Integrate_Computed_field_over_element_data *data;
	struct Integration_scheme *scheme;

	ENTER(integrate_Computed_field_over_element);
	return_code=0;
	if (element&&(data=(struct Integrate_Computed_field_over_element_data *)
		integrate_Computed_field_over_element_data_void)&&(field=data->field)&&
		(scheme=data->scheme)&&(values=data->values)&&(result=data->result))
	{
		if (Computed_field_is_defined_in_element(field,element))
		{
			if (Integration_scheme_get_dimension(scheme,&dimension)&&
				(dimension==get_FE_element_dimension(element))&&
				Integration_scheme_get_weights_and_abscissae(scheme,element,&i,&weight,
				&abscissa))
			{
				time=data->time;
				number_of_components=data->number_of_components;
				return_code=1;
				while (return_code&&(i>0))
				{
					if (Computed_field_evaluate_in_element(field,element,abscissa,time,
						(struct FE_element *)NULL,values,(FE_value *)NULL))
					{
						for (j=0;j<number_of_components;j++)
						{
							result[j]=(*weight)*values[j];
						}
						weight++;
						abscissa += dimension;
						i--;
					}
					else
					{
						data->error_code=3;
						return_code=0;
					}
				}
				Computed_field_clear_cache(field);
			}
			else
			{
				data->error_code=2;
				return_code=0;
			}
		}
		else
		{
			data->error_code=1;
			return_code=0;
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* integrate_Computed_field_over_element */

/*
Global functions
----------------
*/
struct Integration_scheme *CREATE(Integration_scheme)(char *name,
	struct FE_basis *basis)
/*******************************************************************************
LAST MODIFIED : 26 December 2002

DESCRIPTION :
Creates an integration scheme with the given <name> that will integrate the
<basis> exactly.  Unless further information is added, the created scheme will
return the weights and abscissae for <basis> for every element.  The <basis>
also defines the dimension of the integration scheme.
==============================================================================*/
{
	enum FE_basis_type basis_type;
	FE_value *abscissa,*weight;
	int dimension,i,j,k,link_type,number_of_weights;
	struct Integration_scheme *scheme;

	ENTER(CREATE(Integration_scheme));
	scheme=(struct Integration_scheme *)NULL;
	/* check arguments */
	if (name&&basis)
	{
		if (ALLOCATE(scheme,struct Integration_scheme,1))
		{
			if (ALLOCATE(scheme->name,char,strlen(name)+1))
			{
				strcpy(scheme->name,name);
				if (FE_basis_get_dimension(basis,&dimension))
				{
					scheme->dimension=dimension;
					/* determine the number of weights */
					number_of_weights=1;
					i=0;
					k=1;
					while (k&&(i<dimension)&&FE_basis_get_xi_basis_type(basis,i,
						&basis_type)&&FE_basis_get_next_linked_xi_number(basis,i,&j,
						&link_type)
						/*???DB.  Starting with tensor products */
						&&(0==j)&&(0==link_type)
						)
					{
						switch (basis_type)
						{
							case LINEAR_LAGRANGE:
							{
								/* 1 weight.  Do nothing */
							} break;
							case QUADRATIC_LAGRANGE:
							{
								number_of_weights *= 2;
							} break;
							case CUBIC_HERMITE:
							case CUBIC_LAGRANGE:
							{
								number_of_weights *= 3;
							} break;
							default:
							{
								k=0;
							} break;
						}
						i++;
					}
					if (k&&(i>=dimension))
					{
						weight=(FE_value *)NULL;
						abscissa=(FE_value *)NULL;
						if (ALLOCATE(weight,FE_value,number_of_weights)&&
							ALLOCATE(abscissa,FE_value,number_of_weights*dimension))
						{
							scheme->default_number_of_weights_abscissae=number_of_weights;
							scheme->default_weights=weight;
							scheme->default_abscissae=abscissa;
							*weight=(FE_value)1;
							for (i=dimension;i>0;i--)
							{
								*abscissa=(FE_value)0.5;
								abscissa++;
							}
							number_of_weights=1;
							for (i=0;i<dimension;i++)
							{
								FE_basis_get_xi_basis_type(basis,i,&basis_type);
								weight=scheme->default_weights;
								abscissa=scheme->default_abscissae;
								switch (basis_type)
								{
									case LINEAR_LAGRANGE:
									{
										/* 1 weight.  Do nothing */
									} break;
									case QUADRATIC_LAGRANGE:
									{
										for (j=number_of_weights;j>0;j--)
										{
											*weight *= (FE_value)0.5;
											weight[number_of_weights]= *weight;
											weight++;
											for (k=i-1;k>0;k--)
											{
												abscissa[dimension*number_of_weights]= *abscissa;
												abscissa++;
											}
											*abscissa=(FE_value)0.5-(FE_value)0.288675134594813;
											abscissa[dimension*number_of_weights]=
												(FE_value)0.5+(FE_value)0.288675134594813;
											abscissa += dimension-i+1;
										}
										number_of_weights *= 2;
									} break;
									case CUBIC_HERMITE:
									case CUBIC_LAGRANGE:
									{
										for (j=number_of_weights;j>0;j--)
										{
											weight[number_of_weights]=
												(*weight)*(FE_value)0.444444444444444;
											*weight *= (FE_value)0.277777777777778;
											weight[2*number_of_weights]= *weight;
											weight++;
											for (k=i-1;k>0;k--)
											{
												abscissa[dimension*number_of_weights]= *abscissa;
												abscissa[2*dimension*number_of_weights]= *abscissa;
												abscissa++;
											}
											*abscissa=(FE_value)0.5-(FE_value)0.387298334620741;
											abscissa[dimension*number_of_weights]=(FE_value)0.5;
											abscissa[dimension*2*number_of_weights]=
												(FE_value)0.5+(FE_value)0.387298334620741;
											abscissa += dimension-i+1;
										}
										number_of_weights *= 3;
									} break;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"CREATE(Integration_scheme).  "
								"Invalid basis");
							DEALLOCATE(weight);
							DEALLOCATE(abscissa);
							DEALLOCATE(scheme->name);
							DEALLOCATE(scheme);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"CREATE(Integration_scheme).  "
							"Invalid basis");
						DEALLOCATE(scheme->name);
						DEALLOCATE(scheme);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"CREATE(Integration_scheme).  "
						"Could not get basis dimension");
					DEALLOCATE(scheme->name);
					DEALLOCATE(scheme);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"CREATE(Integration_scheme).  "
					"Could not allocate scheme->name");
				DEALLOCATE(scheme);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(Integration_scheme).  "
				"Could not allocate scheme");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Integration_scheme).  "
			"Invalid argument(s).  %p %p",name,basis);
	}
	LEAVE;

	return (scheme);
} /* CREATE(Integration_scheme) */

int DESTROY(Integration_scheme)(struct Integration_scheme **scheme_address)
/*******************************************************************************
LAST MODIFIED : 27 December 2002

DESCRIPTION :
Frees memory/deaccess objects in scheme at <*scheme_address>.
==============================================================================*/
{
	int return_code;
	struct Integration_scheme *scheme;

	ENTER(DESTROY(Integration_scheme));
	return_code=0;
	if (scheme_address&&(scheme= *scheme_address))
	{
		DEALLOCATE(scheme->name);
		DEALLOCATE(scheme->default_weights);
		DEALLOCATE(scheme->default_abscissae);
		DEALLOCATE(scheme);
		*scheme_address=(struct Integration_scheme *)NULL;
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Integration_scheme) */

int integrate(struct Computed_field *field,struct GROUP(FE_element) *domain,
	struct Integration_scheme *scheme,FE_value time,FE_value *result)
/*******************************************************************************
LAST MODIFIED : 27 December 2002

DESCRIPTION :
Calculates the integral of the <field> over the <domain> at the specified <time>
using the given <scheme>.  The <result> array needs to be big enough to hold one
value for each component of the <field>.

???DB.  "integrate" may be to general a name?  Time integrals (solving ODE?)?
==============================================================================*/
{
	int i,number_of_components,return_code;
	struct Integrate_Computed_field_over_element_data
		integrate_Computed_field_over_element_data;

	ENTER(integrate);
	return_code=0;
	/* check arguments */
	if (field&&domain&&scheme&&result)
	{
		/* check that field is numeric */
		if (Computed_field_has_numerical_components(field,(void *)NULL))
		{
			/* evaluate the integral */
			number_of_components=Computed_field_get_number_of_components(field);
			if ((0<number_of_components)&&
				ALLOCATE(integrate_Computed_field_over_element_data.values,FE_value,
				number_of_components))
			{
				integrate_Computed_field_over_element_data.field=field;
				integrate_Computed_field_over_element_data.scheme=scheme;
				integrate_Computed_field_over_element_data.time=time;
				integrate_Computed_field_over_element_data.result=result;
				integrate_Computed_field_over_element_data.number_of_components=
					number_of_components;
				integrate_Computed_field_over_element_data.error_code=0;
				for (i=0;i<number_of_components;i++)
				{
					result[i]=(FE_value)0;
				}
				if (!(return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
					integrate_Computed_field_over_element,
					&integrate_Computed_field_over_element_data,domain)))
				{
					display_message(ERROR_MESSAGE,"integrate.  "
						"Failed for an element.  %d",
						integrate_Computed_field_over_element_data.error_code);
				}
				DEALLOCATE(integrate_Computed_field_over_element_data.values);
			}
			else
			{
				display_message(ERROR_MESSAGE,"integrate.  "
					"Could not ALLOCATE values.  %d",number_of_components);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"integrate.  "
				"Field is not numeric");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"integrate.  "
			"Invalid argument(s).  %p %p %p %p",field,domain,scheme,result);
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* integrate */
