/*******************************************************************************
FILE : import_finite_element.c

LAST MODIFIED : 4 September 2001

DESCRIPTION :
The function for importing finite element data, from a file or CMISS (via a
socket) into the graphical interface to CMISS.
???DB.  Not accessing and deaccessing FE_basis's properly.
==============================================================================*/
#include <ctype.h>
#include "finite_element/finite_element.h"
#include "finite_element/import_finite_element.h"
#include "general/debug.h"
#include "general/multi_range.h"
#include "general/myio.h"
#include "general/mystring.h"
#include "general/object.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#if defined (SGI)
/* For finite so that we can check for Nans which some compilers seem
	to accept as valid in an fscanf */
#include <ieeefp.h>
#endif /* defined (SGI) */

/*
Module types
------------
*/

/*
Module variables
----------------
*/

/*
Module functions
----------------
*/
#if defined (OLD_CODE)
static int read_FE_value_array(FILE *input_file,int *number_of_values,
	FE_value **values)
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Reads an FE_VALUE_ARRAY_VALUE from <input_file>. The first number read is the
number_of_values in the array; the real values follow.
==============================================================================*/
{
	int i,return_code;

	ENTER(read_FE_value_array);
	return_code=0;
	if (input_file&&number_of_values&&values)
	{
		if (1==fscanf(input_file," %d",number_of_values))
		{
			if (0< *number_of_values)
			{
				if (ALLOCATE(*values,FE_value,*number_of_values))
				{
					return_code=1;
					for (i=0;(i<(*number_of_values))&&return_code;i++)
					{
						if (1!=fscanf(input_file,FE_VALUE_INPUT_STRING,&((*values)[i])))
						{
							display_message(ERROR_MESSAGE,
								"read_FE_value_array.  Missing array value(s)");
							DEALLOCATE(*values);
							return_code=0;
						}
						if (!finite((*values)[i]))
						{
							display_message(ERROR_MESSAGE,"read_FE_value_array.  "
								"Infinity or NAN value read from file.");
							return_code=0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_FE_value_array.  Not enough memory");
				}
			}
			else if (0== *number_of_values)
			{
				/* array with no values is a valid return */
				*values=(FE_value *)NULL;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_FE_value_array.  negative number of values %d",
					number_of_values);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_FE_value_array.  Missing number of values and array");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_FE_value_array.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* read_FE_value_array */
#endif /* defined (OLD_CODE) */

static int read_element_xi_value(FILE *input_file,
	struct MANAGER(FE_element) *element_manager,struct FE_element **element,
	FE_value *xi)
/*******************************************************************************
LAST MODIFIED : 6 March 2000

DESCRIPTION :
Reads an element:xi position in from the <input_file> in the format:
E<lement>/F<ace>/L<ine> ELEMENT_NUMBER DIMENSION xi1 xi2... xiDIMENSION
<xi> should have space for up to MAXIMUM_ELEMENT_XI_DIMENSIONS FE_values.
==============================================================================*/
{
	char element_type[8];
	int dimension,k,return_code;
	struct CM_element_information cm;

	ENTER(read_element_xi_value);
	if (input_file&&element_manager&&element&&xi)
	{
		return_code=1;
		if (1==fscanf(input_file,"%7s",element_type))
		{
			if (fuzzy_string_compare(element_type,"element"))
			{
				cm.type=CM_ELEMENT;
			}
			else if (fuzzy_string_compare(element_type,"face"))
			{
				cm.type=CM_FACE;
			}
			else if (fuzzy_string_compare(element_type,"line"))
			{
				cm.type=CM_LINE;
			}
			else
			{
				display_message(ERROR_MESSAGE,"read_element_xi_value.  "
					"Unknown element type %s for element_xi value",
					element_type);
				return_code=0;
			}
			if (return_code)
			{
				if (1==fscanf(input_file," %d",&(cm.number)))
				{
					if (((*element) = FIND_BY_IDENTIFIER_IN_MANAGER(
						FE_element,identifier)(&cm,element_manager))&&(*element)->shape)
					{
						if ((1==fscanf(input_file," %d",&dimension))&&(0<dimension)&&
							(dimension<=MAXIMUM_ELEMENT_XI_DIMENSIONS)&&
							(dimension == (*element)->shape->dimension))
						{
							k=0;
							while (return_code&&(k<dimension))
							{
								if (1==fscanf(input_file,FE_VALUE_INPUT_STRING,&(xi[k])))
								{
									if (finite(xi[k]))
									{
										k++;
									}
									else
									{
										display_message(ERROR_MESSAGE,"read_element_xi_value.  "
											"Infinity or NAN xi coordinates read from file.");
										return_code=0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_element_xi_value.  Missing %d xi value(s).  Line %d",
										k,get_line_number(input_file));
									return_code=0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"read_element_xi_value.  "
								"Invalid dimension (%d) for %s %d - should be %d.  Line %d",
								dimension,CM_element_type_string(cm.type),cm.number,
								(*element)->shape->dimension,get_line_number(input_file));
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"read_element_xi_value.  "
							"Could not find %s %d.  Line %d",
							CM_element_type_string(cm.type),cm.number,
							get_line_number(input_file));
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"read_element_xi_value.  "
						"Missing %s number.  Line %d",
						CM_element_type_string(cm.type),get_line_number(input_file));
					return_code=0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"read_element_xi_value.  "
				"Missing element type.  Line %d",
				get_line_number(input_file));
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_element_xi_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* read_element_xi_value */

static int read_string_value(FILE *input_file,char **string_address)
/*******************************************************************************
LAST MODIFIED : 6 March 2000

DESCRIPTION :
Returns an allocated string with the next contiguous block (length>0) of
characters from input_file not containing whitespace (space, formfeed, newline,
carriage return, tab, vertical tab). If the string begins with EITHER a single
or double quote, ' or ", then the string must end in the same quote mark
followed by whitespace or EOF. Repeat that quote mark to put it once in the
string.
==============================================================================*/
{
	char *the_string;
	int allocated_length,length,quote_mark,reading_token,return_code,this_char;

	ENTER(read_string_value);
	if (input_file&&string_address)
	{
		the_string=(char *)NULL;
		allocated_length=0; /* including space for \0 at end */
		length=0;
		/* pass over leading white space */
		while (isspace(this_char=fgetc(input_file)));
		/* determine if string is in quotes and which quote_mark is in use */
		if (((int)'\'' == this_char) || ((int)'\"' == this_char))
		{
			quote_mark=this_char;
			this_char=fgetc(input_file);
		}
		else
		{
			quote_mark=0;
		}
		reading_token=1;
		/* read token until [quote_mark+]EOF/whitespace */
		while (reading_token)
		{
			if ((EOF==this_char)||(!quote_mark&&isspace(this_char)))
			{
				if (EOF==this_char)
				{
					if (quote_mark)
					{
						display_message(ERROR_MESSAGE,
							"read_string_value.  End of file before end quote mark");
						return_code=0;
					}
					if (!the_string)
					{
						display_message(ERROR_MESSAGE,
							"read_string_value.  Missing string");
						return_code=0;
					}
				}
				reading_token=0;
			}
			else
			{
				if (quote_mark&&(quote_mark==this_char))
				{
					this_char=fgetc(input_file);
					if ((EOF==this_char)||isspace(this_char))
					{
						if (!the_string)
						{
							/* for empty string "" or '' */
							if (ALLOCATE(the_string,char,1))
							{
								*string_address=the_string;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_string_value.  Not enough memory");
							}
						}
						reading_token=0;
					}
					else if (quote_mark!=this_char)
					{
						display_message(ERROR_MESSAGE,
							"read_string_value.  Must have white space after end quote");
						DEALLOCATE(the_string);
						reading_token=0;
					}
				}
				if (reading_token)
				{
					length++;
					/* is the current string big enough (including \0 at end)? */
					if (allocated_length < length+1)
					{
						allocated_length += 50;
						if (REALLOCATE(*string_address,the_string,char,
							allocated_length))
						{
							the_string = *string_address;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_string_value.  Not enough memory");
							DEALLOCATE(the_string);
							reading_token=0;
						}
					}
					if (the_string)
					{
						the_string[length-1]=(char)this_char;
					}
					this_char=fgetc(input_file);
				}
			}
		}
		if (the_string)
		{
			the_string[length]='\0';
			return_code=1;
		}
		*string_address=the_string;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_string_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* read_string_value */

static struct FE_field *read_FE_field(FILE *input_file,
	struct MANAGER(FE_field) *fe_field_manager)
/*******************************************************************************
LAST MODIFIED : 30 August 2001

DESCRIPTION :
Reads a field from its descriptor in <input_file>. Note that the same format is
used for node and element field headers. The field is returned. It is up to the
calling function to match the field against existing ones in the manager, and
destroy or use the returned field as necessary. Also note that the field will
not have any component names; these must be set by the calling function.
==============================================================================*/
{
	char *field_name,*next_block;
	enum CM_field_type cm_field_type;
	enum FE_field_type fe_field_type;
	enum Value_type value_type;
	float focus;
	int i,number_of_components,number_of_indexed_values,return_code;
	struct Coordinate_system coordinate_system;
	struct FE_field *field,*indexer_field;

	ENTER(read_FE_field);
	field = (struct FE_field *)NULL;
	if (input_file&&fe_field_manager)
	{
		return_code=1;
		field_name=(char *)NULL;
		/* read the field information */
		fscanf(input_file," %*d) ");
		/* read the field name */
		if (return_code)
		{
			if (read_string(input_file,"[^,]",&field_name))
			{
				fscanf(input_file,", ");
				/* remove trailing blanks off field name */
				i=strlen(field_name);
				while ((0<i)&&(isspace(field_name[i-1])))
				{
					i--;
				}
				field_name[i]='\0';
				if (0==i)
				{
					display_message(ERROR_MESSAGE,"read_FE_field.  "
						"No field name.  Line %d",get_line_number(input_file));
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"read_FE_field.  "
					"Missing field name.  Line %d",get_line_number(input_file));
				return_code=0;
			}
		}
		next_block=(char *)NULL;
		if (return_code)
		{
			/* next string required for value_type, below */
			return_code=read_string(input_file,"[^,]",&next_block);
			fscanf(input_file,", ");
		}
		/* read the CM_field_type */
		if (return_code && next_block)
		{
			if (!STRING_TO_ENUMERATOR(CM_field_type)(next_block, &cm_field_type))
			{
				display_message(ERROR_MESSAGE,"read_FE_field.  "
					"Field '%s' has unknown CM field type '%s'.  Line %d",field_name,
					next_block,get_line_number(input_file));
				return_code=0;
			}
		}
		if (next_block)
		{
			DEALLOCATE(next_block);
		}
		/* read the FE_field_information */
		if (return_code)
		{
			/* next string required for value_type, below */
			return_code=read_string(input_file,"[^,]",&next_block);
			fscanf(input_file,", ");
		}
		/* read the optional modifier: constant|indexed */
		if (return_code&&next_block)
		{
			if (fuzzy_string_compare_same_length(next_block,"constant"))
			{
				fe_field_type=CONSTANT_FE_FIELD;
			}
			else if (fuzzy_string_compare_same_length(next_block,"indexed"))
			{
				fe_field_type=INDEXED_FE_FIELD;
				DEALLOCATE(next_block);
				if (!((EOF != fscanf(input_file," Index_field = "))&&
					read_string(input_file,"[^,]",&next_block)&&
					(indexer_field=FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(
						next_block,fe_field_manager))&&
					(1==fscanf(input_file,", #Values=%d",&number_of_indexed_values))&&
					(0<number_of_indexed_values)))
				{
					display_message(ERROR_MESSAGE,"read_FE_field.  "
						"Field '%s' missing indexing information.  Line %d",field_name,
						get_line_number(input_file));
					return_code=0;
				}
				fscanf(input_file,", ");
			}
			else
			{
				fe_field_type=GENERAL_FE_FIELD;
			}
			if (GENERAL_FE_FIELD != fe_field_type)
			{
				DEALLOCATE(next_block);
				if (return_code)
				{
					/* next string required for coordinate system or value_type */
					return_code=read_string(input_file,"[^,]",&next_block);
					fscanf(input_file,", ");
				}
			}
		}
		else
		{
			if (next_block)
			{
				DEALLOCATE(next_block);
			}
		}
		/* read the coordinate system (optional) */
		coordinate_system.type=NOT_APPLICABLE;
		if (return_code&&next_block)
		{
			if (fuzzy_string_compare_same_length(next_block,"rectangular cartesian"))
			{
				coordinate_system.type=RECTANGULAR_CARTESIAN;
			}
			else if (fuzzy_string_compare_same_length(next_block,"cylindrical polar"))
			{
				coordinate_system.type=CYLINDRICAL_POLAR;
			}
			else if (fuzzy_string_compare_same_length(next_block,"spherical polar"))
			{
				coordinate_system.type=SPHERICAL_POLAR;
			}
			else if (fuzzy_string_compare_same_length(next_block,
				"prolate spheroidal"))
			{
				coordinate_system.type=PROLATE_SPHEROIDAL;
				fscanf(input_file," focus=");
				if ((1!=fscanf(input_file,FE_VALUE_INPUT_STRING,&focus))||
					(!finite(focus)))
				{
					focus=1.0;
				}
				coordinate_system.parameters.focus=focus;
				fscanf(input_file," ,");
			}
			else if (fuzzy_string_compare_same_length(next_block,"oblate spheroidal"))
			{
				coordinate_system.type=OBLATE_SPHEROIDAL;
				fscanf(input_file," focus=");
				if ((1!=fscanf(input_file,FE_VALUE_INPUT_STRING,&focus))||
					(!finite(focus)))
				{
					focus=1.0;
				}
				coordinate_system.parameters.focus=focus;
				fscanf(input_file," ,");
			}
			else if (fuzzy_string_compare_same_length(next_block,"fibre"))
			{
				coordinate_system.type=FIBRE;
				value_type=FE_VALUE_VALUE;
			}
			if (NOT_APPLICABLE!=coordinate_system.type)
			{
				DEALLOCATE(next_block);
				if (return_code)
				{
					/* next string required for value_type, below */
					return_code=read_string(input_file,"[^,\n]",&next_block);
					fscanf(input_file,", ");
				}
			}
		}
		else
		{
			if (next_block)
			{
				DEALLOCATE(next_block);
			}
		}
		/* read the value_type */
		if (return_code&&next_block)
		{
			value_type=Value_type_from_string(next_block);
			if (UNKNOWN_VALUE == value_type)
			{
				if (coordinate_system.type != NOT_APPLICABLE)
				{
					/* for backwards compatibility default to FE_VALUE_VALUE if
						coordinate system specified */
					value_type=FE_VALUE_VALUE;
				}
				else
				{
					display_message(ERROR_MESSAGE,"read_FE_field.  "
						"Field '%s' has unknown value_type %s.  Line %d",field_name,
						next_block,get_line_number(input_file));
					return_code=0;
				}
			}
			else
			{
				DEALLOCATE(next_block);
				/* next string required for value_type, below */
				return_code=read_string(input_file,"[^,\n]",&next_block);
			}
		}
		else
		{
			if (next_block)
			{
				DEALLOCATE(next_block);
			}
		}
		if (return_code&&next_block)
		{
			if (!((1==sscanf(next_block," #Components=%d",
				&number_of_components))&&(0<number_of_components)))
			{
				display_message(ERROR_MESSAGE,"read_FE_field.  "
					"Field '%s' missing #Components.  Line %d",field_name,
					get_line_number(input_file));
				return_code=0;
			}
		}
		if (next_block)
		{
			DEALLOCATE(next_block);
		}
		if (return_code)
		{
			/* create the field */
			if (!((field=CREATE(FE_field)())&&
				set_FE_field_name(field,field_name)&&
				set_FE_field_value_type(field,value_type)&&
				set_FE_field_number_of_components(field,number_of_components)&&
				((CONSTANT_FE_FIELD != fe_field_type)||
					set_FE_field_type_constant(field))&&
				((GENERAL_FE_FIELD != fe_field_type)||
					set_FE_field_type_general(field))&&
				((INDEXED_FE_FIELD != fe_field_type)||set_FE_field_type_indexed(field,
					indexer_field,number_of_indexed_values))&&
				set_FE_field_CM_field_type(field,cm_field_type)&&
				set_FE_field_coordinate_system(field,&coordinate_system)))
			{
				display_message(ERROR_MESSAGE,
					"read_FE_field.  Could not create field '%s'",field_name);
				DESTROY(FE_field)(&field);
			}
		}
		DEALLOCATE(field_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,"read_FE_field.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* read_FE_field */

static int read_FE_field_values(FILE *input_file,
	struct MANAGER(FE_element) *element_manager,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 14 September 1999

DESCRIPTION :
Reads in from <input_file> the values for the constant and indexed fields in
the <field_order_info>.
==============================================================================*/
{
	char *rest_of_line;
	enum Value_type value_type;
	int i,k,number_of_fields,number_of_values,return_code;
	struct FE_field *field;

	ENTER(read_FE_field_values);
	return_code=0;
	if (input_file&&field_order_info)
	{
		rest_of_line=(char *)NULL;
		read_string(input_file,"[^\n]",&rest_of_line);
		return_code=string_matches_without_whitespace(rest_of_line,"alues : ");
		DEALLOCATE(rest_of_line);
		if (return_code)
		{
			return_code=1;
			number_of_fields=
				get_FE_field_order_info_number_of_fields(field_order_info);
			for (i=0;(i<number_of_fields)&&return_code;i++)
			{
				if (field=get_FE_field_order_info_field(field_order_info,i))
				{
					number_of_values=get_FE_field_number_of_values(field);
					if (0<number_of_values)
					{
						value_type=get_FE_field_value_type(field);
						switch (value_type)
						{
							case ELEMENT_XI_VALUE:
							{
								FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
								struct FE_element *element;

								for (k=0;(k<number_of_values)&&return_code;k++)
								{
									if (!(read_element_xi_value(input_file,element_manager,
										&element,xi)&&
										set_FE_field_element_xi_value(field,k,element,xi)))
									{
										display_message(ERROR_MESSAGE,
											"read_FE_field_values.  Error getting element_xi value");
										return_code=0;
									}
								}
							} break;
							case FE_VALUE_VALUE:
							{
								FE_value value;

								for (k=0;(k<number_of_values)&&return_code;k++)
								{
									if (!((1==fscanf(input_file,FE_VALUE_INPUT_STRING,&value))&&
										finite(value)&&set_FE_field_FE_value_value(field,k,value)))
									{
										display_message(ERROR_MESSAGE,
											"read_FE_field_values.  Error getting FE_value");
										return_code=0;
									}
								}
							} break;
							case INT_VALUE:
							{
								int value;

								for (k=0;(k<number_of_values)&&return_code;k++)
								{
									if (!((1==fscanf(input_file,"%d",&value))&&
										set_FE_field_int_value(field,k,value)))
									{
										display_message(ERROR_MESSAGE,
											"read_FE_field_values.  Error getting int");
										return_code=0;
									}
								}
							} break;
							case STRING_VALUE:
							{
								char *the_string;

								for (k=0;(k<number_of_values)&&return_code;k++)
								{
									if (read_string_value(input_file,&the_string))
									{
										if (!set_FE_field_string_value(field,k,the_string))
										{
											display_message(ERROR_MESSAGE,
												"read_FE_field_values.  Error setting string");
											return_code=0;
										}
										DEALLOCATE(the_string);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"read_FE_field_values.  Error reading string");
										return_code=0;
									}
								}
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,"read_FE_field_values.  "
									"Unsupported value_type %s",Value_type_string(value_type));
								return_code=0;
							} break;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_FE_field_values.  Invalid field #%d",i+1);
					return_code=0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_FE_field_values.  Invalid Values: line text");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"read_FE_field_values.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* read_FE_field_values */

static int read_FE_node_field(FILE *input_file,
	struct MANAGER(FE_field) *fe_field_manager,struct FE_node *node, 
	struct FE_field **field)
/*******************************************************************************
LAST MODIFIED : 27 March 2001

DESCRIPTION :
Reads a node field from an <input_file>, adding it to the fields defined at
<node>. <field> is returned.
==============================================================================*/
{
	char *component_name, *derivative_type_name, *field_name,
		*nodal_value_type_string, *rest_of_line;
	enum FE_field_type fe_field_type;
	enum FE_nodal_value_type **components_nodal_value_types, *derivative_type;	
	int component_number, *components_number_of_derivatives,
		*components_number_of_versions, end_of_names, end_of_string, i,
		number_of_components,	return_code, temp_int;
	struct FE_field *the_field; 
	
	ENTER(read_FE_node_field);
	*field=(struct FE_field *)NULL;
	return_code=0;
	if (input_file&&fe_field_manager&&node&&field)
	{
		if ((the_field=read_FE_field(input_file,fe_field_manager))&&
			(number_of_components=get_FE_field_number_of_components(the_field))&&
			GET_NAME(FE_field)(the_field,&field_name))
		{
			fe_field_type=get_FE_field_FE_field_type(the_field);
			return_code=1;
			/* allocate memory for the components */
			component_name=(char *)NULL;
			ALLOCATE(components_number_of_derivatives,int,
				number_of_components);
			ALLOCATE(components_number_of_versions,int,
				number_of_components);
			if (ALLOCATE(components_nodal_value_types,
				enum FE_nodal_value_type *,number_of_components))
			{
				for (i=0;i<number_of_components;i++)
				{
					components_nodal_value_types[i]=(enum FE_nodal_value_type *)NULL;
				}
			}
			if (components_number_of_derivatives&&
				components_number_of_versions&&components_nodal_value_types)
			{
				/* read the components */
				component_number=0;
				while (return_code&&(component_number<number_of_components))
				{
					fscanf(input_file," ");
					/* read the component name */
					if (component_name)
					{
						DEALLOCATE(component_name);
					}
					if (read_string(input_file,"[^.]",&component_name))
					{
						/* strip trailing blanks from component name */
						i=strlen(component_name);
						while ((0<i)&&(isspace(component_name[i-1])))
						{
							i--;
						}
						component_name[i]='\0';
						return_code=(0<i)&&set_FE_field_component_name(the_field,
							component_number,component_name);
					}
					if (return_code)
					{
						/* component name is sufficient for non-GENERAL_FE_FIELD */
						if (GENERAL_FE_FIELD==fe_field_type)
						{
							/* ignore value index */
							if ((2==fscanf(input_file,
								".  Value index=%d, #Derivatives=%d",&temp_int,
								components_number_of_derivatives+component_number))&&
								(0<=temp_int)&&
								(0<=components_number_of_derivatives[component_number]))
							{
								if (ALLOCATE(components_nodal_value_types[component_number],
									enum FE_nodal_value_type,
									1+components_number_of_derivatives[component_number]))
								{
									/* CMISS starts from 1 */
									temp_int--;
									/* first number is the value */
									(components_nodal_value_types[component_number])[0]=
										FE_NODAL_VALUE;
									/* clear derivative value types to FE_NODAL_UNKNOWN */
									for (i = 1; i <=
										components_number_of_derivatives[component_number]; i++)
									{
										(components_nodal_value_types[component_number])[i] =
											FE_NODAL_UNKNOWN;
									}
									if (read_string(input_file, "[^\n]", &rest_of_line))
									{
										derivative_type_name = rest_of_line;
										derivative_type =
											components_nodal_value_types[component_number];
										derivative_type++;
										/* skip leading spaces */
										while (' '== *derivative_type_name)
										{
											derivative_type_name++;
										}
										if (0 < (i =
											components_number_of_derivatives[component_number]))
										{
											/* optional derivative names will be in brackets */
											if ('(' == *derivative_type_name)
											{
												derivative_type_name++;
												/* skip leading spaces */
												while (' ' == *derivative_type_name)
												{
													derivative_type_name++;
												}
												end_of_names = (')' == *derivative_type_name);
												end_of_string = ('\0' == *derivative_type_name);
												while (0 < i)
												{
													if (!(end_of_names || end_of_string))
													{
														nodal_value_type_string = derivative_type_name;
														while (('\0' != *derivative_type_name) &&
															(',' != *derivative_type_name) &&
															(' ' != *derivative_type_name) &&
															(')' != *derivative_type_name))
														{
															derivative_type_name++;
														}
														end_of_names = (')' == *derivative_type_name);
														end_of_string = ('\0' == *derivative_type_name);
														*derivative_type_name = '\0';
														if (!(end_of_names || end_of_string))
														{
															derivative_type_name++;
															while ((',' == *derivative_type_name) ||
																(' ' == *derivative_type_name))
															{
																derivative_type_name++;
															}
															end_of_names = (')' == *derivative_type_name);
															end_of_string = ('\0' == *derivative_type_name);
														}
														if (!STRING_TO_ENUMERATOR(FE_nodal_value_type)(
															nodal_value_type_string, derivative_type))
														{
															display_message(WARNING_MESSAGE,
																"Unknown derivative type '%s' for field "
																"component %s.%s", nodal_value_type_string,
																field_name, component_name);
														}
													}
													else
													{
														display_message(WARNING_MESSAGE,
															"Missing derivative type for field component "
															"%s.%s", field_name, component_name);
													}
													derivative_type++;
													i--;
												}
												if (end_of_names)
												{
													derivative_type_name++;
												}
											}
											else
											{
												display_message(WARNING_MESSAGE,
													"Derivative types missing for field component %s.%s",
													field_name, component_name);
											}
										}
										/* read in the number of versions (if present) */
										if (1!=sscanf(derivative_type_name,", #Versions=%d",
											components_number_of_versions+component_number))
										{
											components_number_of_versions[component_number]=1;
										}
										DEALLOCATE(rest_of_line);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"read_FE_node_field.  Could not read rest_of_line");
										return_code=0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,"read_FE_node_field.  "
										"Could not allocate nodal_value_types array");
									return_code=0;
								}
							}
						}
						else
						{
							/* non GENERAL_FE_FIELD */
							/* check there is nothing on remainder of line */
							if (read_string(input_file,"[^\n]",&rest_of_line))
							{
								if (fuzzy_string_compare(rest_of_line,"."))
								{
									/* set number_of_derivatives to 0, versions to 1 and single
										 nodal value types to FE_NODAL_VALUE for all components */
									components_number_of_derivatives[component_number]=0;
									components_number_of_versions[component_number]=1;
									if (ALLOCATE(components_nodal_value_types[component_number],
										enum FE_nodal_value_type,
										1+components_number_of_derivatives[component_number]))
									{
										/* first number is the value */
										(components_nodal_value_types[component_number])[0]=
											FE_NODAL_VALUE;
									}
									else
									{
										display_message(ERROR_MESSAGE,"read_FE_node_field.  "
											"Could not allocate nodal_value_types array");
										return_code=0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,"read_FE_node_field.  "
										"Unexpected text on field '%s' component '%s' line: %s",
										field_name,component_name,rest_of_line);
									return_code=0;
								}
								DEALLOCATE(rest_of_line);
							}
							else
							{
								display_message(ERROR_MESSAGE,"read_FE_node_field.  "
									"Unexpected end of field '%s' component '%s' line",
									field_name,component_name);
								return_code=0;
							}
						}
						component_number++;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_FE_node_field.  Error establishing component name");
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_FE_node_field.  Could not allocate component information");
				return_code=0;
			}
			if (return_code)
			{
				/* first try to retrieve matching field from manager */
				if ((*field) = FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(
					field_name,fe_field_manager))
				{
					if (!FE_fields_match(*field,the_field))
					{
						display_message(ERROR_MESSAGE,"read_FE_node_field.  "
							"Field '%s' inconsistent with existing field of same name",
							field_name);
						*field = (struct FE_field *)NULL;
						return_code=0;
					}
				}
				else
				{
					if (ADD_OBJECT_TO_MANAGER(FE_field)(the_field,fe_field_manager))
					{
						*field = the_field;
						/* clear the_field so not destroyed below */
						the_field=(struct FE_field *)NULL;
					}
					else
					{
						display_message(ERROR_MESSAGE,"read_FE_node_field.  "
							"Could not add field to manager");
						return_code=0;
					}
				}
			}
			DESTROY(FE_field)(&the_field);
			if (return_code)
			{
				if (!define_FE_field_at_node(node,*field,
					components_number_of_derivatives,components_number_of_versions,
					components_nodal_value_types))
				{
					display_message(ERROR_MESSAGE,
						"read_FE_node_field.  Could not define field at node");
					*field = (struct FE_field *)NULL;
					return_code=0;
				}
			}
			if (component_name)
			{
				DEALLOCATE(component_name);
			}
			if (components_nodal_value_types)
			{
				for (i=0;i<number_of_components;i++)
				{
					DEALLOCATE(components_nodal_value_types[i]);
				}
				DEALLOCATE(components_nodal_value_types);
			}
			DEALLOCATE(components_number_of_derivatives);
			DEALLOCATE(components_number_of_versions);
			DEALLOCATE(field_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_FE_node_field.  Could not read field");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_FE_node_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* read_FE_node_field */

static struct FE_node *read_FE_node_field_info(FILE *input_file,
	struct MANAGER(FE_field) *fe_field_manager,
	struct FE_field_order_info **field_order_info)
/*******************************************************************************
LAST MODIFIED : 4 September 2001

DESCRIPTION :
Creates a node with the field information read from the <input_file>.
Creates, fills in and returns field_order_info.
<*field_order_info> is reallocated here so should be either NULL or returned
from a previous call to this function.
==============================================================================*/
{
	int number_of_fields,return_code,i;
	struct FE_node *node;
	struct FE_field *field;

	ENTER(read_FE_node_field_info);
	node = (struct FE_node *)NULL;
	if (input_file && fe_field_manager && field_order_info)
	{
		if (*field_order_info)
		{
			DESTROY(FE_field_order_info)(field_order_info);
		}
		/* create the node */
		if (node = CREATE(FE_node)(0, (struct FE_node *)NULL))
		{
			return_code = 1;
			if ((1==fscanf(input_file,"Fields=%d",&number_of_fields))&&
				(0<=number_of_fields))
			{
				*field_order_info = CREATE(FE_field_order_info)();
				/* read in the node fields */
				for (i = 0; (i < number_of_fields) && return_code; i++)
				{
					field = (struct FE_field *)NULL;
					if (read_FE_node_field(input_file,fe_field_manager,node,&field))
					{
						if (!add_FE_field_order_info_field(*field_order_info, field))
						{
							display_message(ERROR_MESSAGE,
								"read_FE_node_field_info.  Could not add field to list");
							return_code = 0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_FE_node_field_info.  Could not read node field");
						return_code = 0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_FE_node_field_info.  Error reading number of fields from file");
				return_code = 0;
			}
			if (!return_code)
			{
				DESTROY(FE_node)(&node);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_FE_node_field_info.  Could not create node");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_FE_node_field_info.  Invalid argument(s)");
	}
	LEAVE;

	return (node);
} /* read_FE_node_field_info */

static struct FE_node *read_FE_node(FILE *input_file,
	struct FE_node *template_node,struct MANAGER(FE_node) *node_manager,
	struct MANAGER(FE_element) *element_manager,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 17 September 1999

DESCRIPTION :
Reads in a node from an <input_file>.
==============================================================================*/
{
	char *field_name;
	enum Value_type value_type;
	int i,j,k,length,node_number,number_of_components,number_of_fields,
		number_of_values,return_code;
	struct FE_field *field;
	struct FE_node *existing_node,*node;

	ENTER(read_FE_node);
	node=(struct FE_node *)NULL;
	if (input_file&&template_node&&node_manager&&element_manager&&
		field_order_info)
	{
		if (1==fscanf(input_file,"ode :%d",&node_number))
		{
			return_code=1;
			/* create node based on template node; read and fill in contents */
			if (node=CREATE(FE_node)(node_number,template_node))
			{
				number_of_fields=
					get_FE_field_order_info_number_of_fields(field_order_info);
				for (i=0;(i<number_of_fields)&&return_code;i++)
				{
					if (field=get_FE_field_order_info_field(field_order_info,i))
					{
						/* only GENERAL_FE_FIELD can store values at nodes */
						if (GENERAL_FE_FIELD==get_FE_field_FE_field_type(field))
						{
							GET_NAME(FE_field)(field,&field_name);
							number_of_components=get_FE_field_number_of_components(field);
							number_of_values=0;
							for (j=0;j<number_of_components;j++)
							{
								number_of_values +=
									get_FE_node_field_component_number_of_versions(node,field,j)*
									(1+get_FE_node_field_component_number_of_derivatives(node,
										field,j));
							}
							value_type=get_FE_field_value_type(field);
							if (0<number_of_values)
							{
								switch (value_type)
								{
									case ELEMENT_XI_VALUE:
									{
										FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
										struct FE_element *element;

										if (number_of_values==number_of_components)
										{
											for (k=0;(k<number_of_values)&&return_code;k++)
											{
												if (!(read_element_xi_value(input_file,element_manager,
													&element,xi)&&set_FE_nodal_element_xi_value(node,
														field,/*component_number*/k,/*version*/0,
														FE_NODAL_VALUE,element,xi)))
												{
													display_message(ERROR_MESSAGE,"read_FE_node.  "
														"Error getting element_xi value for field '%s'",
														field_name);
													return_code=0;
												}
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,"read_FE_node.  "
												"Derivatives/versions not supported for element_xi");
											return_code=0;
										}
									} break;
									case FE_VALUE_VALUE:
									{
										FE_value *values;

										if (ALLOCATE(values,FE_value,number_of_values))
										{
											for (k=0;(k<number_of_values)&&return_code;k++)
											{
												if (1 != fscanf(input_file,FE_VALUE_INPUT_STRING,
													&(values[k])))
												{
													display_message(ERROR_MESSAGE,"read_FE_node.  "
														"Error reading nodal value from file");
													return_code=0;
												}
												if (!finite(values[k]))
												{
													display_message(ERROR_MESSAGE,"read_FE_node.  "
														"Infinity or NAN read from node file.");
													return_code=0;
												}
											}
											if (return_code)
											{
												if (return_code=set_FE_nodal_field_FE_value_values(
													field,node,values,&length))
												{
													if (length != number_of_values)
													{
														display_message(ERROR_MESSAGE,"read_FE_node.  "
															"node %d field '%s' took %d values from %d expected",
															node_number,field_name,length,number_of_values);
														return_code=0;
													}
												}
											}
											DEALLOCATE(values);
										}
										else
										{
											display_message(ERROR_MESSAGE,"read_FE_node.  "
												"Insufficient memory for FE_value_values");
											return_code=0;
										}
									} break;
									case INT_VALUE:
									{
										int *values;

										if (ALLOCATE(values,int,number_of_values))
										{
											for (k=0;(k<number_of_values)&&return_code;k++)
											{
												if (1 != fscanf(input_file,"%d",&(values[k])))
												{
													display_message(ERROR_MESSAGE,
														"read_FE_node.  Error reading nodal value from file");
													return_code=0;
												}
											}
											if (return_code)
											{
												if (return_code=set_FE_nodal_field_int_values(field,node,
													values,&length))
												{
													if (length != number_of_values)
													{
														display_message(ERROR_MESSAGE,"read_FE_node.  "
															"node %d field '%s' took %d values from %d expected",
															node_number,field_name,length,number_of_values);
														return_code=0;
													}
												}
											}
											DEALLOCATE(values);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"read_FE_node.  Insufficient memory for int_values");
											return_code=0;
										}
									} break;
									case STRING_VALUE:
									{
										char *the_string;

										if (number_of_values==number_of_components)
										{
											for (k=0;(k<number_of_values)&&return_code;k++)
											{
												if (read_string_value(input_file,&the_string))
												{
													if (!set_FE_nodal_string_value(node,field,
														/*component_number*/k,/*version*/0,FE_NODAL_VALUE,
														the_string))
													{
														display_message(ERROR_MESSAGE,"read_FE_node.  "
															"Error setting string value for field '%s'",
															field_name);
														return_code=0;
													}
													DEALLOCATE(the_string);
												}
												else
												{
													display_message(ERROR_MESSAGE,"read_FE_node.  "
														"Error reading string value for field '%s'",
														field_name);
													return_code=0;
												}
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,"read_FE_node.  "
												"Derivatives/versions not supported for string");
											return_code=0;
										}
									} break;
									default:
									{
										display_message(ERROR_MESSAGE,
											"read_FE_node.  Unsupported value_type %s",
											Value_type_string(value_type));
										return_code=0;
									} break;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_FE_node.  No nodal values for field '%s'",field_name);
								return_code=0;
							}
							DEALLOCATE(field_name);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_FE_node.  Invalid field #%d",i+1);
						return_code=0;
					}
				}
				if (return_code)
				{
					if (existing_node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,
						cm_node_identifier)(node_number,node_manager))
					{
						/* merge the values from the existing to the new */
						if (merge_FE_node(node,existing_node))
						{
							if (MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)(
								existing_node,node,node_manager))
							{
								DESTROY(FE_node)(&node);
								node=existing_node;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_FE_node.  Error modifying node %d in node_manager", 
									node_number);
								DESTROY(FE_node)(&node);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_FE_node.  Error merging new information for node %d",
								node_number);
							DESTROY(FE_node)(&node);
						}
					}
					else
					{
						if (!ADD_OBJECT_TO_MANAGER(FE_node)(node,node_manager))
						{				
							display_message(ERROR_MESSAGE,
								"read_FE_node.  Error adding node %d to node_manager",
								node_number);
							DESTROY(FE_node)(&node);
						}
					}
				}
				else
				{
					DESTROY(FE_node)(&node);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"read_FE_node.  Could not create node");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_FE_node.  Error reading node number from file");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"read_FE_node.  Invalid argument(s)");
	}
	LEAVE;

	return (node);
} /* read_FE_node */

static struct FE_element_shape *read_FE_element_shape(FILE *input_file)
/*******************************************************************************
LAST MODIFIED : 26 April 2001

DESCRIPTION :
Reads element shape information from an <input_file> or the socket (if
<input_file> is NULL).
==============================================================================*/
{
	char *end_description,*shape_description_string,*start_description;
	int component,dimension,*first_simplex,i,j,number_of_polygon_vertices,
		previous_component,return_code,*temp_entry,*type,*type_entry,
		xi_number;
	struct FE_element_shape *shape;

	ENTER(read_FE_element_shape);
	/* check argument */
	if (input_file)
	{
		/* file input */
		if ((1==fscanf(input_file,"hape.  Dimension=%d",&dimension))&&
			(0<dimension))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_FE_element_shape.  Error reading element dimension from file");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"read_FE_element_shape.  Missing input_file");
		return_code=0;
	}
	if (return_code)
	{
		if (ALLOCATE(type,int,(dimension*(dimension+1))/2))
		{
			if (input_file)
			{
				/* file input */
				fscanf(input_file,",");
				/* read the shape description string */
				if (read_string(input_file,"[^\n]",&shape_description_string))
				{
					if (shape_description_string)
					{
						/* trim the shape description string */
						end_description=shape_description_string+
							(strlen(shape_description_string)-1);
						while ((end_description>shape_description_string)&&
							(' '== *end_description))
						{
							end_description--;
						}
						end_description[1]='\0';
						start_description=shape_description_string;
						while (' '== *start_description)
						{
							start_description++;
						}
						if ('\0'== *start_description)
						{
							DEALLOCATE(shape_description_string);
						}
					}
					if (shape_description_string)
					{
						/* decipher the shape description */
						xi_number=0;
						type_entry=type;
						while (type&&(xi_number<dimension))
						{
							xi_number++;
							if (xi_number<dimension)
							{
								if (end_description=strchr(start_description,'*'))
								{
									*end_description='\0';
								}
								else
								{
									DEALLOCATE(type);
								}
							}
							if (type)
							{
								if (0==strncmp(start_description,"line",4))
								{
									start_description += 4;
									*type_entry=LINE_SHAPE;
									while (' '== *start_description)
									{
										start_description++;
									}
									if ('\0'== *start_description)
									{
										type_entry++;
										for (i=dimension-xi_number;i>0;i--)
										{
											*type_entry=0;
											type_entry++;
										}
									}
									else
									{
										DEALLOCATE(type);
									}
								}
								else
								{
									if (0==strncmp(start_description,"polygon",7))
									{
										start_description += 7;
										while (' '== *start_description)
										{
											start_description++;
										}
										if ('\0'== *start_description)
										{
											/* check for link to first polygon coordinate */
											temp_entry=type_entry;
											i=xi_number-1;
											j=dimension-xi_number;
											number_of_polygon_vertices=0;
											while (type&&(i>0))
											{
												j++;
												temp_entry -= j;
												if (*temp_entry)
												{
													if (0<number_of_polygon_vertices)
													{
														DEALLOCATE(type);
													}
													else
													{
														if (!((POLYGON_SHAPE==temp_entry[i-xi_number])&&
															((number_of_polygon_vertices= *temp_entry)>=3)))
														{
															DEALLOCATE(type);
														}
													}
												}
												i--;
											}
											if (type&&(3<=number_of_polygon_vertices))
											{
												*type_entry=POLYGON_SHAPE;
												type_entry++;
												for (i=dimension-xi_number;i>0;i--)
												{
													*type_entry=0;
													type_entry++;
												}
											}
											else
											{
												DEALLOCATE(type);
											}
										}
										else
										{
											/* assign link to second polygon coordinate */
											if ((2==sscanf(start_description,"(%d ;%d )%n",
												&number_of_polygon_vertices,&component,&i))&&
												(3<=number_of_polygon_vertices)&&
												(xi_number<component)&&(component<=dimension)&&
												('\0'==start_description[i]))
											{
												*type_entry=POLYGON_SHAPE;
												type_entry++;
												i=xi_number+1;
												while (i<component)
												{
													*type_entry=0;
													type_entry++;
													i++;
												}
												*type_entry=number_of_polygon_vertices;
												type_entry++;
												while (i<dimension)
												{
													*type_entry=0;
													type_entry++;
													i++;
												}
											}
											else
											{
												DEALLOCATE(type);
											}
										}
									}
									else
									{
										if (0==strncmp(start_description,"simplex",7))
										{
											start_description += 7;
											while (' '== *start_description)
											{
												start_description++;
											}
											if ('\0'== *start_description)
											{
												/* check for link to previous simplex coordinate */
												temp_entry=type_entry;
												i=xi_number-1;
												j=dimension-xi_number;
												first_simplex=(int *)NULL;
												while (type&&(i>0))
												{
													j++;
													temp_entry -= j;
													if (*temp_entry)
													{
														if (SIMPLEX_SHAPE==temp_entry[i-xi_number])
														{
															first_simplex=temp_entry;
														}
														else
														{
															DEALLOCATE(type);
														}
													}
													i--;
												}
												if (type&&first_simplex)
												{
													*type_entry=SIMPLEX_SHAPE;
													type_entry++;
													first_simplex++;
													for (i=dimension-xi_number;i>0;i--)
													{
														*type_entry= *first_simplex;
														type_entry++;
														first_simplex++;
													}
												}
												else
												{
													DEALLOCATE(type);
												}
											}
											else
											{
												/* assign link to succeeding simplex coordinate */
												previous_component=xi_number+1;
												if ((1 == sscanf(start_description, "(%d %n",
													&component, &i)) &&
													(previous_component <= component) &&
													(component <= dimension))
												{
													*type_entry=SIMPLEX_SHAPE;
													type_entry++;
													do
													{
														start_description += i;
														while (previous_component<component)
														{
															*type_entry=0;
															type_entry++;
															previous_component++;
														}
														*type_entry=1;
														type_entry++;
														previous_component++;
													} while ((')'!=start_description[0])&&
														(1==sscanf(start_description,"%*[; ]%d %n",
														&component,&i))&&(previous_component<=component)&&
														(component<=dimension));
													if (')'==start_description[0])
													{
														/* fill rest of shape_type row with zeroes */
														while (previous_component <= dimension)
														{
															*type_entry=0;
															type_entry++;
															previous_component++;
														}
													}
													else
													{
														DEALLOCATE(type);
													}
												}
												else
												{
													DEALLOCATE(type);
												}
											}
										}
										else
										{
											DEALLOCATE(type);
										}
									}
								}
								if (type&&(xi_number<dimension))
								{
									start_description=end_description+1;
								}
							}
						}
						DEALLOCATE(shape_description_string);
					}
					else
					{
						/* retrieve a "square" element of the specified dimension */
						type_entry=type;
						for (i=dimension-1;i>=0;i--)
						{
							*type_entry=LINE_SHAPE;
							type_entry++;
							for (j=i;j>0;j--)
							{
								*type_entry=0;
								type_entry++;
							}
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
					"read_FE_element_shape.  Error reading shape description from file");
					shape=(struct FE_element_shape *)NULL;
					DEALLOCATE(type);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_FE_element_shape.  Missing input_file");
				shape=(struct FE_element_shape *)NULL;
				DEALLOCATE(type);
			}
			if (type)
			{
				if (!(shape=CREATE(FE_element_shape)(dimension,type)))
				{
					display_message(ERROR_MESSAGE,
						"read_FE_element_shape.  Error creating shape");
				}
				DEALLOCATE(type);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_FE_element_shape.  Invalid shape description");
				shape=(struct FE_element_shape *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_FE_element_shape.  Insufficient memory for type");
			shape=(struct FE_element_shape *)NULL;
		}
	}
	else
	{
		shape=(struct FE_element_shape *)NULL;
	}
	LEAVE;

	return (shape);
} /* read_FE_element_shape */

static struct FE_basis *read_FE_basis(FILE *input_file,
	int number_of_xi_coordinates,int *basis_type,
	struct MANAGER(FE_basis) *basis_manager)
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Reads a basis description from an <input_file> or the socket (if <input_file> is
NULL).  If the basis does not exist, it is created.  The basis is returned.
<basis_type> should be allocated outside the function and on exit will contain
the a copy of the type for the basis.  Some examples of basis descriptions in an
input file are
1. c.Hermite*c.Hermite*l.Lagrange  This has cubic variation in xi1 and xi2 and
	linear variation in xi3.
2. c.Hermite*l.simplex(3)*l.simplex  This has cubic variation in xi1 and 2-D
	linear simplex variation for xi2 and xi3.
3. polygon(5,3)*l.Lagrange*polygon  This has linear variation in xi2 and a 2-D
	5-gon for xi1 and xi3.
==============================================================================*/
{
	char *basis_description_string,*end_basis_name,*start_basis_name;
	int component,i,j,*first_simplex,no_error,number_of_polygon_vertices,
		previous_component,*temp_basis_type,*xi_basis_type,xi_number;
	struct FE_basis *basis;

	ENTER(read_FE_basis);
	/* check the arguments */
	if ((number_of_xi_coordinates>0)&&basis_type)
	{
		if (input_file)
		{
			/* file input */
			/* read the basis type */
			if (read_string(input_file,"[^,]",&basis_description_string))
			{
				/* decipher the basis description */
				xi_number=0;
				start_basis_name=basis_description_string;
				/* skip leading blanks */
				while (' '== *start_basis_name)
				{
					start_basis_name++;
				}
				/* remove trailing blanks */
				end_basis_name=start_basis_name+(strlen(start_basis_name)-1);
				while (' '== *end_basis_name)
				{
					end_basis_name--;
				}
				end_basis_name[1]='\0';
				xi_basis_type=basis_type;
				*xi_basis_type=number_of_xi_coordinates;
				xi_basis_type++;
				no_error=1;
				while (no_error&&(xi_number<number_of_xi_coordinates))
				{
					xi_number++;
					/* determine the interpolation in the xi direction */
					if (xi_number<number_of_xi_coordinates)
					{
						if (end_basis_name=strchr(start_basis_name,'*'))
						{
							*end_basis_name='\0';
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_FE_basis.  Invalid basis description");
							no_error=0;
						}
					}
					if (no_error)
					{
						if ((0==strncmp(start_basis_name,"l.simplex",9))||
							(0==strncmp(start_basis_name,"q.simplex",9)))
						{
/*???debug */
/*printf("simplex\n");*/
							if (0==strncmp(start_basis_name,"l.simplex",9))
							{
								*xi_basis_type=LINEAR_SIMPLEX;
							}
							else
							{
								*xi_basis_type=QUADRATIC_SIMPLEX;
							}
/*???debug */
/*printf("%p %d %d\n",xi_basis_type,*xi_basis_type,LINEAR_SIMPLEX);*/
							start_basis_name += 9;
							/* skip blanks */
							while (' '== *start_basis_name)
							{
								start_basis_name++;
							}
							/* check for links to other simplex components */
							if ('('== *start_basis_name)
							{
/*???debug */
/*printf("first simplex component\n");*/
								xi_basis_type++;
								/* assign links to other simplex components */
								previous_component=xi_number+1;
								if ((1==sscanf(start_basis_name,"(%d %n",&component,&i))&&
									(previous_component<=component)&&
									(component<=number_of_xi_coordinates))
								{
									do
									{
										start_basis_name += i;
										while (previous_component<component)
										{
											*xi_basis_type=NO_RELATION;
											xi_basis_type++;
											previous_component++;
										}
										*xi_basis_type=1;
										xi_basis_type++;
										previous_component++;
									} while ((')'!=start_basis_name[0])&&
										(1==sscanf(start_basis_name,"%*[; ]%d %n",&component,&i))&&
										(previous_component<=component)&&
										(component<=number_of_xi_coordinates));
									if (')'==start_basis_name[0])
									{
										/* fill rest of basis_type row with NO_RELATION */
										while (previous_component <= number_of_xi_coordinates)
										{
											*xi_basis_type=NO_RELATION;
											xi_basis_type++;
											previous_component++;
										}
									}
									else
									{
										/* have no links to succeeding xi directions */
										display_message(ERROR_MESSAGE,
											"read_FE_basis.  Invalid simplex component");
										no_error=0;
									}
								}
								else
								{
									/* have no links to succeeding xi directions */
									display_message(ERROR_MESSAGE,
										"read_FE_basis.  Invalid simplex component");
									no_error=0;
								}
							}
							else
							{
/*???debug */
/*printf("not first simplex component\n");*/
								/* check that links have been assigned */
								temp_basis_type=xi_basis_type;
								i=xi_number-1;
								j=number_of_xi_coordinates-xi_number;
								first_simplex=(int *)NULL;
								while (no_error&&(i>0))
								{
									j++;
									temp_basis_type -= j;
									if (NO_RELATION!= *temp_basis_type)
									{
/*???debug */
/*printf("%p %p\n",xi_basis_type,(temp_basis_type-(xi_number-i)));
printf("%d %d\n",*xi_basis_type,*(temp_basis_type-(xi_number-i)));*/
										if (*xi_basis_type== *(temp_basis_type-(xi_number-i)))
										{
											first_simplex=temp_basis_type;
										}
										else
										{
											no_error=0;
										}
									}
									i--;
								}
/*???debug */
/*printf("%d %p\n",no_error,first_simplex);*/
								if (no_error&&first_simplex)
								{
									xi_basis_type++;
									first_simplex++;
									i=xi_number;
									while (i<number_of_xi_coordinates)
									{
										*xi_basis_type= *first_simplex;
										xi_basis_type++;
										first_simplex++;
										i++;
									}
								}
								else
								{
									no_error=0;
								}
							}
						}
						else
						{
							if (0==strncmp(start_basis_name,"polygon",7))
							{
								*xi_basis_type=POLYGON;
								start_basis_name += 7;
								/* skip blanks */
								while (' '== *start_basis_name)
								{
									start_basis_name++;
								}
								/* check for link to other polygon component */
								if ('('== *start_basis_name)
								{
									/* assign link to other polygon component */
									if ((2==sscanf(start_basis_name,"(%d ;%d )%n",
										&number_of_polygon_vertices,&component,&i))&&
										(3<=number_of_polygon_vertices)&&
										(xi_number<component)&&
										(component<=number_of_xi_coordinates)&&
										('\0'== start_basis_name[i]))
									{
										/* assign link */
										xi_basis_type++;
										i=xi_number+1;
										while (i<component)
										{
											*xi_basis_type=NO_RELATION;
											xi_basis_type++;
											i++;
										}
										*xi_basis_type=number_of_polygon_vertices;
										xi_basis_type++;
										while (i<number_of_xi_coordinates)
										{
											*xi_basis_type=NO_RELATION;
											xi_basis_type++;
											i++;
										}
									}
									else
									{
										/* have no links to succeeding xi directions */
										display_message(ERROR_MESSAGE,
											"read_FE_basis.  Invalid polygon");
										no_error=0;
									}
								}
								else
								{
									/* check that link has been assigned */
									temp_basis_type=xi_basis_type;
									i=xi_number-1;
									j=number_of_xi_coordinates-xi_number;
									number_of_polygon_vertices=0;
									while (no_error&&(i>0))
									{
										j++;
										temp_basis_type -= j;
										if (NO_RELATION!= *temp_basis_type)
										{
											if (0<number_of_polygon_vertices)
											{
												no_error=0;
											}
											else
											{
												if ((number_of_polygon_vertices= *temp_basis_type)<3)
												{
													no_error=0;
												}
											}
										}
										i--;
									}
									if (no_error&&(3<=number_of_polygon_vertices))
									{
										xi_basis_type++;
										i=xi_number;
										while (i<number_of_xi_coordinates)
										{
											*xi_basis_type=NO_RELATION;
											xi_basis_type++;
											i++;
										}
									}
									else
									{
										no_error=0;
									}
								}
							}
							else
							{
								if (0==strncmp(start_basis_name,"l.Lagrange",10))
								{
									*xi_basis_type=LINEAR_LAGRANGE;
									start_basis_name += 10;
								}
								else
								{
									if (0==strncmp(start_basis_name,"q.Lagrange",10))
									{
										*xi_basis_type=QUADRATIC_LAGRANGE;
										start_basis_name += 10;
									}
									else
									{
										if (0==strncmp(start_basis_name,"c.Lagrange",10))
										{
											*xi_basis_type=CUBIC_LAGRANGE;
											start_basis_name += 10;
										}
										else
										{
											if (0==strncmp(start_basis_name,"c.Hermite",9))
											{
												*xi_basis_type=CUBIC_HERMITE;
												start_basis_name += 9;
											}
											else
											{
												if (0==strncmp(start_basis_name,"LagrangeHermite",15))
												{
													*xi_basis_type=LAGRANGE_HERMITE;
													start_basis_name += 15;
												}
												else
												{
													if (0==strncmp(start_basis_name,"HermiteLagrange",15))
													{
														*xi_basis_type=HERMITE_LAGRANGE;
														start_basis_name += 15;
													}
													else
													{
														display_message(ERROR_MESSAGE,
															"read_FE_basis.  Invalid basis type");
														no_error=0;
													}
												}
											}
										}
									}
								}
								if (no_error)
								{
									/* skip blanks */
									while (' '== *start_basis_name)
									{
										start_basis_name++;
									}
									/* check for simplex elements */
									if ('('== *start_basis_name)
									{
										/* assign links to succeeding simplex xi directions */
										temp_basis_type=xi_basis_type;
										i=xi_number;
										while (no_error&&(i<number_of_xi_coordinates)&&
											(')'!= *start_basis_name))
										{
											temp_basis_type++;
											if (';'== *start_basis_name)
											{
												*temp_basis_type=NO_RELATION;
												start_basis_name++;
											}
											else
											{
												if (0==strncmp(start_basis_name,"l.Lagrange",10))
												{
													*temp_basis_type=LINEAR_LAGRANGE;
													start_basis_name += 10;
												}
												else
												{
													if (0==strncmp(start_basis_name,"q.Lagrange",10))
													{
														*temp_basis_type=QUADRATIC_LAGRANGE;
														start_basis_name += 10;
													}
													else
													{
														if (0==strncmp(start_basis_name,"c.Lagrange",10))
														{
															*temp_basis_type=CUBIC_LAGRANGE;
															start_basis_name += 10;
														}
														else
														{
															if (0==strncmp(start_basis_name,"c.Hermite",9))
															{
																*temp_basis_type=CUBIC_HERMITE;
																start_basis_name += 9;
															}
															else
															{
																if (0==strncmp(start_basis_name,
																	"LagrangeHermite",15))
																{
																	*temp_basis_type=LAGRANGE_HERMITE;
																	start_basis_name += 15;
																}
																else
																{
																	if (0==strncmp(start_basis_name,
																		"HermiteLagrange",15))
																	{
																		*temp_basis_type=HERMITE_LAGRANGE;
																		start_basis_name += 15;
																	}
																	else
																	{
																		display_message(ERROR_MESSAGE,
																			"read_FE_basis.  Invalid basis type");
																		no_error=0;
																	}
																}
															}
														}
													}
												}
												if (';'== *start_basis_name)
												{
													start_basis_name++;
												}
											}
											i++;
										}
										if (no_error)
										{
											while (i<number_of_xi_coordinates)
											{
												temp_basis_type++;
												*temp_basis_type=NO_RELATION;
												i++;
											}
										}
									}
									else
									{
										if ('\0'== *start_basis_name)
										{
											/* have no links to succeeding xi directions */
											temp_basis_type=xi_basis_type;
											for (i=xi_number;i<number_of_xi_coordinates;i++)
											{
												temp_basis_type++;
												*temp_basis_type=NO_RELATION;
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"read_FE_basis.  Invalid basis type");
											no_error=0;
										}
									}
									if (no_error&&(xi_number<number_of_xi_coordinates))
									{
										xi_basis_type += number_of_xi_coordinates-xi_number+1;
									}
								}
							}
						}
						start_basis_name=end_basis_name+1;
					}
				}
				DEALLOCATE(basis_description_string);
				if (!no_error)
				{
					display_message(ERROR_MESSAGE,
						"read_FE_basis.  Invalid basis description");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_FE_basis.  Error reading basis description from file");
				no_error=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"read_FE_basis.  Missing input_file");
			no_error=0;
		}
		if (no_error)
		{
			basis = make_FE_basis(basis_type,basis_manager);
		}
		else
		{
			basis=(struct FE_basis *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"read_FE_basis.  Invalid argument(s)");
		basis=(struct FE_basis *)NULL;
	}
	LEAVE;

	return (basis);
} /* read_FE_basis */

static int read_FE_element_field(FILE *input_file,
	struct FE_element_shape *shape,struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_basis) *basis_manager,struct FE_element *element,
	struct FE_field **field)
/*******************************************************************************
LAST MODIFIED : 18 October 1999

DESCRIPTION :
Reads an element field from an <input_file>, adding it to the fields defined at
<element>. <field> is returned.
==============================================================================*/
{
	char *component_name,*field_name,*global_to_element_map_string,
		*modify_function_name,*rest_of_line;
	enum FE_field_type fe_field_type;
	FE_element_field_component_modify modify;
	int *basis_type,component_number,i,*index,j,node_index,number_of_components,
		number_of_nodes,number_of_values,*number_in_xi,return_code;
	struct FE_basis *basis;
	struct FE_field *the_field;
	struct FE_element_field_component **component,**components;
	struct Standard_node_to_element_map **standard_node_map;

	ENTER(read_FE_element_field);
	*field=(struct FE_field *)NULL;
	return_code=0;
	if (input_file&&shape&&fe_field_manager&&basis_manager&&element&&field)
	{
		if ((the_field=read_FE_field(input_file,fe_field_manager))&&
			(number_of_components=get_FE_field_number_of_components(the_field))&&
			GET_NAME(FE_field)(the_field,&field_name))
		{
			fe_field_type=get_FE_field_FE_field_type(the_field);
			return_code=1;
			/* allocate memory for the components */
			component_name=(char *)NULL;
			if (ALLOCATE(components,struct FE_element_field_component *,
				number_of_components))
			{
				for (i=0;i<number_of_components;i++)
				{
					components[i]=(struct FE_element_field_component *)NULL;
				}
			}
			ALLOCATE(basis_type,int,1+((shape->dimension)*(1+shape->dimension))/2);
			if (components&&basis_type)
			{
				/* read the components */
				component_number=0;
				number_of_values=0;
				component=components;
				while (return_code&&(component_number<number_of_components))
				{
					fscanf(input_file," ");
					/* read the component name */
					if (component_name)
					{
						DEALLOCATE(component_name);
					}
					if (read_string(input_file,"[^.]",&component_name))
					{
						/* strip trailing blanks from component name */
						i=strlen(component_name);
						while ((0<i)&&(isspace(component_name[i-1])))
						{
							i--;
						}
						component_name[i]='\0';
						return_code=(0<i)&&set_FE_field_component_name(the_field,
							component_number,component_name);
					}
					if (return_code)
					{
						/* component name is sufficient for non-GENERAL_FE_FIELD */
						if (GENERAL_FE_FIELD==fe_field_type)
						{
							fscanf(input_file,". ");
							/* read the basis */
							if (basis=read_FE_basis(input_file,shape->dimension,
								basis_type,basis_manager))
							{
								fscanf(input_file,", ");
								/* read the modify function name */
								if (read_string(input_file,"[^,]",
									&modify_function_name))
								{
									/* determine the modify function */
									if (strcmp("no modify",modify_function_name))
									{
										if (strcmp("increasing in xi1",
											modify_function_name))
										{
											if (strcmp("decreasing in xi1",
												modify_function_name))
											{
												if (strcmp("non-increasing in xi1",
													modify_function_name))
												{
													if (strcmp("non-decreasing in xi1",
														modify_function_name))
													{
														display_message(ERROR_MESSAGE,
															"read_FE_element_field.  Invalid modify function from file");
														return_code=0;
													}
													else
													{
														modify=theta_non_decreasing_in_xi1;
													}
												}
												else
												{
													modify=theta_non_increasing_in_xi1;
												}
											}
											else
											{
												modify=theta_decreasing_in_xi1;
											}
										}
										else
										{
											modify=theta_increasing_in_xi1;
										}
									}
									else
									{
										modify=(FE_element_field_component_modify)NULL;
									}
									if (return_code)
									{
										fscanf(input_file,", ");
										/* read the global to element map type */
										if (read_string(input_file,"[^.]",
											&global_to_element_map_string))
										{
											fscanf(input_file,". ");
																/* determine the global to element map type */
											if (strcmp("standard node based",
												global_to_element_map_string))
											{
												if (strcmp("grid based",
													global_to_element_map_string))
												{
													if (strcmp("general node based",
														global_to_element_map_string))
													{
														if (strcmp("field based",
															global_to_element_map_string))
														{
															display_message(ERROR_MESSAGE,
																"read_FE_element_field.  Invalid global to element map type from file");
															return_code=0;
														}
														else
														{
															/* FIELD_TO_ELEMENT_MAP */
															/*???DB.  Not yet implemented */
															display_message(ERROR_MESSAGE,
																"read_FE_element_field.  Invalid global to element map type from file");
															return_code=0;
														}
													}
													else
													{
														/* GENERAL_NODE_TO_ELEMENT_MAP */
														/*???DB.  Not yet implemented */
														display_message(ERROR_MESSAGE,
															"read_FE_element_field.  Invalid global to element map type from file");
														return_code=0;
													}
												}
												else
												{
													/* element grid based */
													if (components[component_number]=
														CREATE(FE_element_field_component)(
															ELEMENT_GRID_MAP,1,basis,modify))
													{
														/* read the number of divisions in each xi
															 direction */
														number_in_xi=
															(components[component_number]->map).
															element_grid_based.number_in_xi;
														i=0;
														while (return_code&&(i<shape->dimension))
														{
															if ((2==fscanf(input_file,"#xi%d = %d",
																&j,number_in_xi+i))&&(j==i+1)&&
																(0<number_in_xi[i]))
															{
																fscanf(input_file," , ");
																i++;
															}
															else
															{
																display_message(ERROR_MESSAGE,
																	"read_FE_element_field.  Error reading #xi%d",i+1);
																return_code=0;
															}
														}
														if (return_code)
														{
															/* only allow linear bases */
															index=basis->type;
															i= *index;
															while (return_code&&(i>0))
															{
																index++;
																if (LINEAR_LAGRANGE== *index)
																{
																	i--;
																	j=i;
																	while (return_code&&(j>0))
																	{
																		index++;
																		if (0== *index)
																		{
																			j--;
																		}
																		else
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
															if (return_code)
															{
																/* CMISS starts from 1 */
																(components[component_number]->
																	map).element_grid_based.
																	value_index=0;
															}
															else
															{
																display_message(ERROR_MESSAGE,
																	"read_FE_element_field.  Grid based must be linear");
																return_code=0;
															}
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
															"read_FE_element_field.  Error creating component from file");
														return_code=0;
													}
												}
											}
											else
											{
												/* standard node to element map */
												/* read the number of nodes */
												if ((1==fscanf(input_file,"#Nodes=%d",
													&number_of_nodes))&&(0<number_of_nodes))
												{
													if (components[component_number]=
														CREATE(FE_element_field_component)(
															STANDARD_NODE_TO_ELEMENT_MAP,
															number_of_nodes,basis,modify))
													{
														standard_node_map=
															(components[component_number])->
															map.standard_node_based.
															node_to_element_maps;
														i=number_of_nodes;
														while (return_code&&(i>0))
														{
															if ((2==fscanf(input_file,
																"%d .  #Values=%d",&node_index,
																&number_of_values))&&
																(0<node_index--)&&
																(0<number_of_values)&&
																(*standard_node_map=
																	CREATE(Standard_node_to_element_map)(
																		node_index,number_of_values)))
															{
																/* read the value indices */
																fscanf(input_file,
																	" Value indices:");
																index=(*standard_node_map)->
																	nodal_value_indices;
																j=number_of_values;
																while (return_code&&(j>0))
																{
																	if (1==fscanf(input_file,"%d",
																		index))
																	{
																		/* CMISS arrays start at 1 */
																		(*index)--;
																		index++;
																		j--;
																	}
																	else
																	{
																		display_message(ERROR_MESSAGE,
																			"read_FE_element_field.  Error reading nodal value index from file");
																		return_code=0;
																	}
																}
																if (return_code)
																{
																/* read the scale factor indices */
																	fscanf(input_file,
																		" Scale factor indices:");
																	index=(*standard_node_map)->
																		scale_factor_indices;
																	j=number_of_values;
																	while (return_code&&(j>0))
																	{
																		if (1==fscanf(input_file,"%d",
																			index))
																		{
																			/* CMISS arrays start at 1 */
																			(*index)--;
																			index++;
																			j--;
																		}
																		else
																		{
																			display_message(ERROR_MESSAGE,
																				"read_FE_element_field.  Error reading scale factor index from file");
																			return_code=0;
																		}
																	}
																	if (return_code)
																	{
																		standard_node_map++;
																		i--;
																	}
																}
															}
															else
															{
																display_message(ERROR_MESSAGE,
																	"read_FE_element_field.  Error creating standard node to element map from file"
																								);
																return_code=0;
															}
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
															"read_FE_element_field.  Error creating component from file");
														return_code=0;
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"read_FE_element_field.  Error reading component number of nodes from file");
													return_code=0;
												}
											}
											DEALLOCATE(global_to_element_map_string);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"read_FE_element_field.  Error reading global to element map type from file");
											return_code=0;
										}
										DEALLOCATE(modify_function_name);
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_FE_element_field.  Error reading modify function name from file");
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_FE_element_field.  Invalid basis from file");
								return_code=0;
							}
						}
						else
						{
							/* non GENERAL_FE_FIELD */
							/* check there is nothing on remainder of line */
							if (read_string(input_file,"[^\n]",&rest_of_line))
							{
								if (fuzzy_string_compare(rest_of_line,"."))
								{
									/* components are all NULL */
									return_code=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,"read_FE_element_field.  "
										"Unexpected text on field '%s' component '%s' line: %s",
										field_name,component_name,rest_of_line);
									return_code=0;
								}
								DEALLOCATE(rest_of_line);
							}
							else
							{
								display_message(ERROR_MESSAGE,"read_FE_element_field.  "
									"Unexpected end of field '%s' component '%s' line",
									field_name,component_name);
								return_code=0;
							}
						}
						component_number++;
						component++;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_FE_element_field.  Error reading component name from file");
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_FE_element_field.  Could not allocate component information");
				return_code=0;
			}
			if (return_code)
			{
				/* first try to retrieve matching field from manager */
				if (*field = FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(
					field_name,fe_field_manager))
				{
					if (!FE_fields_match(*field,the_field))
					{
						display_message(ERROR_MESSAGE,"read_FE_element_field.  "
							"Field '%s' inconsistent with existing field of same name",
							field_name);
						*field = (struct FE_field *)NULL;
						return_code=0;
					}
				}
				else
				{
					if (ADD_OBJECT_TO_MANAGER(FE_field)(the_field,fe_field_manager))
					{
						*field = the_field;
						/* clear the_field so not destroyed below */
						the_field=(struct FE_field *)NULL;
					}
					else
					{
						display_message(ERROR_MESSAGE,"read_FE_element_field.  "
							"Could not add field to manager");
						return_code=0;
					}
				}
			}
			DESTROY(FE_field)(&the_field);
			if (return_code)
			{
				if (!define_FE_field_at_element(element,*field,components))
				{
					display_message(ERROR_MESSAGE,
						"read_FE_element_field.  Could not define field at element");
					*field = (struct FE_field *)NULL;
					return_code=0;
				}
			}
			component_number=number_of_components;
			if (component_name)
			{
				DEALLOCATE(component_name);
			}
			if (components)
			{
				for (i=0;i<number_of_components;i++)
				{
					DESTROY(FE_element_field_component)(&(components[i]));
				}
				DEALLOCATE(components);
			}
			DEALLOCATE(basis_type);
			DEALLOCATE(field_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_FE_element_field.  Could not read field");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_FE_element_field.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* read_FE_element_field */

static struct FE_element *read_FE_element_field_info(
	FILE *input_file,struct FE_element_shape *shape,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_basis) *basis_manager,
	struct FE_field_order_info **field_order_info)
/*******************************************************************************
LAST MODIFIED : 4 September 2001

DESCRIPTION :
Returns a template element with the element field information read in from the
<input_file>.
If this functions encounters a heading with #Fields=0, then there is no
node_scale_field information and NULL is returned. Previously it did not
appear to be possible to go from a state where there were fields to where they
were inherited in the same file, now you just need the header:
 #Scale factor sets=0
 #Nodes=0
 #Fields=0
It is also possible to have no scale factors and no nodes but a field - this
would be the case for grid-based fields.
<*field_order_info> is reallocated here so should be either NULL or returned
from a previous call to this function.
==============================================================================*/
{
	int *basis_type,dimension,i,number_of_fields,number_of_nodes,
		number_of_scale_factor_sets,*numbers_in_scale_factor_sets,return_code;
	struct CM_element_information element_identifier;
	struct FE_element *element;
	struct FE_field *field;
	void **scale_factor_set_identifiers;

	ENTER(read_FE_element_field_info);
	element=(struct FE_element *)NULL;
	if (input_file&&shape&&fe_field_manager&&basis_manager&&field_order_info)
	{
		if (*field_order_info)
		{
			DESTROY(FE_field_order_info)(field_order_info);
		}
		/* create the element */
		element_identifier.number=0;
		element_identifier.type=CM_ELEMENT;
		if (element=CREATE(FE_element)(&element_identifier,
			(struct FE_element *)NULL))
		{
			return_code=1;
			if (!set_FE_element_shape(element,shape))
			{
				display_message(ERROR_MESSAGE,"read_FE_element_field_info.  "
					"Error setting element shape");
				return_code=0;
			}
			dimension=shape->dimension;
			/* read in the scale factor information */
			if (!((1==fscanf(input_file,"Scale factor sets=%d ",
				&number_of_scale_factor_sets))&&(0<=number_of_scale_factor_sets)))
			{
				display_message(ERROR_MESSAGE,"read_FE_element_field_info.  "
					"Error reading #scale sets from file");
				return_code=0;
			}
			if (return_code)
			{
				scale_factor_set_identifiers=(void **)NULL;
				numbers_in_scale_factor_sets=(int *)NULL;
				basis_type=(int *)NULL;
				/* note can have no scale factor sets */
				if ((0==number_of_scale_factor_sets)||(
					ALLOCATE(scale_factor_set_identifiers,void *,
						number_of_scale_factor_sets)&&
					ALLOCATE(numbers_in_scale_factor_sets,int,
						number_of_scale_factor_sets)&&
					ALLOCATE(basis_type,int,1+(dimension*(1+dimension))/2)))
				{
					/* read in the scale factor set information */
					for (i=0;(i<number_of_scale_factor_sets)&&return_code;i++)
					{
						if (scale_factor_set_identifiers[i]=(void *)read_FE_basis(
							input_file,dimension,basis_type,basis_manager))
						{
							if (!((1==fscanf(input_file,", #Scale factors=%d ",
								&(numbers_in_scale_factor_sets[i])))&&
								(0<numbers_in_scale_factor_sets[i])))
							{
								display_message(ERROR_MESSAGE,"read_FE_element_field_info.  "
									"Error reading #Scale factors from file");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"read_FE_element_field_info.  "
								"Error reading scale factor set identifier (basis) from file");
							return_code=0;
						}
					}
					/* read in the node information */
					if (!((1==fscanf(input_file,"#Nodes=%d ",&number_of_nodes))&&
						(0<=number_of_nodes)))
					{
						display_message(ERROR_MESSAGE,"read_FE_element_field_info.  "
							"Error reading #Nodes from file");
						return_code=0;
					}
					/* read in the field information */
					if (!((1==fscanf(input_file,"#Fields=%d ",&number_of_fields))&&
						(0<=number_of_fields)))
					{
						display_message(ERROR_MESSAGE,"read_FE_element_field_info.  "
							"Error reading #fields from file");
						return_code=0;
					}
					if (return_code&&(0<number_of_fields))
					{
						if (!set_FE_element_node_scale_field_info(element,
							number_of_scale_factor_sets,scale_factor_set_identifiers,
							numbers_in_scale_factor_sets,number_of_nodes))
						{
							display_message(ERROR_MESSAGE,"read_FE_element_field_info.  "
								"Error establishing node scale field information");
							return_code=0;
						}
					}
					if (return_code)
					{
						*field_order_info = CREATE(FE_field_order_info)();
						/* read in the element fields */
						for (i = 0; (i < number_of_fields) && return_code; i++)
						{
							field = (struct FE_field *)NULL;
							if (read_FE_element_field(input_file, shape, fe_field_manager,
								basis_manager, element, &field))
							{
								if (!add_FE_field_order_info_field(*field_order_info, field))
								{
									display_message(ERROR_MESSAGE,
										"read_FE_element_field_info.  Could not add field to list");
									return_code = 0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_FE_element_field_info.  Could not read element field");
								return_code = 0;
							}
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_FE_element_field_info.  Not enough memory");
					return_code=0;
				}
				if (basis_type)
				{
					DEALLOCATE(basis_type);
				}
				if (numbers_in_scale_factor_sets)
				{
					DEALLOCATE(numbers_in_scale_factor_sets);
				}
				if (scale_factor_set_identifiers)
				{
					DEALLOCATE(scale_factor_set_identifiers);
				}
			}
			if (!return_code)
			{
				DESTROY(FE_element)(&element);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_FE_element_field_info.  Could not create element");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_FE_element_field_info.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (element);
} /* read_FE_element_field_info */

static struct FE_element *read_FE_element(FILE *input_file,
	struct FE_element *template_element,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(FE_node) *node_manager,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 12 October 1999

DESCRIPTION :
Reads in an element from an <input_file>.
???RC element info may now have no nodes and no scale factors - eg. for reading
in a grid field.
==============================================================================*/
{
	enum Value_type value_type;
	FE_value *scale_factor;
	int face_token_length,i,j,k,node_number,number_of_components,number_of_faces,
		number_of_fields,number_of_nodes,number_of_scale_factors,number_of_values,
		return_code,element_num,face_num,line_num;
	struct CM_element_information element_identifier,face_identifier;
	struct FE_element *element,*existing_element,*face_element;
	struct FE_field *field;
	struct FE_node *node;

	ENTER(read_FE_element);
	element=(struct FE_element *)NULL;
	if (input_file&&template_element&&template_element->shape&&
		(0<=(number_of_faces=template_element->shape->number_of_faces))&&
		element_manager&&node_manager&&field_order_info)
	{
		/* read the element identifier */
		if (3==fscanf(input_file,"lement :%d %d %d",
			&element_num,&face_num,&line_num))
		{
			if (element_num)
			{
				element_identifier.number = element_num;
				element_identifier.type = CM_ELEMENT;
			}
			else if(face_num) 
			{
				element_identifier.number = face_num;
				element_identifier.type = CM_FACE;
			}
			else /* line_num */
			{
				element_identifier.number = line_num;
				element_identifier.type = CM_LINE;
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_FE_element.  Error reading element identifier from file");
			return_code=0;
		}
		if (return_code)
		{
			/* create element based on template element; read and fill in contents */
			if (element=CREATE(FE_element)(&element_identifier,template_element))
			{
				/* if face_token_length > 0, then faces being read */
				face_token_length=0;
				fscanf(input_file," Faces:%n",&face_token_length);
				if (0<face_token_length)
				{
					for (i=0;(i<number_of_faces)&&return_code;i++)
					{
						/* file input */			 
						if (3==fscanf(input_file,"%d %d %d",
							&element_num,&face_num,&line_num))
						{
							if (element_num)
							{
								face_identifier.number = element_num;
								face_identifier.type = CM_ELEMENT;
							}
							else if (face_num) 
							{
								face_identifier.number = face_num;
								face_identifier.type = CM_FACE;
							}
							else /* line_num */
							{
								face_identifier.number = line_num;
								face_identifier.type = CM_LINE;
							}								
							return_code =1;
						}
						else
						{
							display_message(ERROR_MESSAGE,"read_FE_element.  "
								"Error reading face identifier from file");
							return_code=0;
						}
						if (return_code)
						{
							/* face number of 0 means no face */
							if (0 != face_identifier.number)
							{
								if (face_element=
									FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,identifier)(
										&face_identifier,element_manager))
								{
									if (!set_FE_element_face(element,i,face_element))
									{
										display_message(ERROR_MESSAGE,"read_FE_element.  "
											"Could not set face %d of %s %d",i,
											CM_element_type_string(element_identifier.type),
											element_identifier.number);
										return_code=0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,"read_FE_element.  "
										"Could not find face %d of %s %d",i,
										CM_element_type_string(element_identifier.type),
										element_identifier.number);
									return_code=0;
								}
							}
						}
					}
				}
				if (return_code)
				{
					/* check for field information */
					if (element->information)
					{
						/* check whether element has any grid values */
						/*???RC Should have better check than this! */
						if (0<element->information->values_storage_size)
						{
							/* read the values */
							fscanf(input_file," Values :");
							number_of_fields=
								get_FE_field_order_info_number_of_fields(field_order_info);
							for (i=0;(i<number_of_fields)&&return_code;i++)
							{
								if (field=get_FE_field_order_info_field(field_order_info,i))
								{
									if (0<(number_of_values=
										get_FE_element_field_number_of_grid_values(element,field)))
									{
										value_type=get_FE_field_value_type(field);
										number_of_components=
											get_FE_field_number_of_components(field);
										switch (value_type)
										{
											case FE_VALUE_VALUE:
											{
												FE_value *values;

												if (ALLOCATE(values,FE_value,number_of_values))
												{
													for (j=0;(j<number_of_components)&&return_code;j++)
													{
														for (k=0;(k<number_of_values)&&return_code;k++)
														{
															if (1 != fscanf(input_file,FE_VALUE_INPUT_STRING,
																&(values[k])))
															{
																display_message(ERROR_MESSAGE,
																	"read_FE_element.  "
																	"Error reading grid FE_value value from file");
																return_code=0;
															}
															if (!finite(values[k]))
															{
																display_message(ERROR_MESSAGE,"read_FE_element.  "
																	"Infinity or NAN element value read from element file.");
																return_code=0;
															}
														}
														if (return_code)
														{
															if (!set_FE_element_field_component_grid_FE_value_values(
																element,field,/*component_number*/j,values))
															{
																display_message(ERROR_MESSAGE,
																	"read_FE_element.  "
																	"Could not set grid FE_value values");
															}
														}
													}
													DEALLOCATE(values);
												}
												else
												{
													display_message(ERROR_MESSAGE,"read_FE_element.  "
														"Insufficient memory for FE_value values");
													return_code=0;
												}
											} break;
											case INT_VALUE:
											{
												int *values;

												if (ALLOCATE(values,int,number_of_values))
												{
													for (j=0;(j<number_of_components)&&return_code;j++)
													{
														for (k=0;(k<number_of_values)&&return_code;k++)
														{
															if (1 != fscanf(input_file,"%d",&(values[k])))
															{
																display_message(ERROR_MESSAGE,
																	"read_FE_element.  "
																	"Error reading grid int value from file");
																return_code=0;
															}
														}
														if (return_code)
														{
															if (!set_FE_element_field_component_grid_int_values(
																element,field,/*component_number*/j,values))
															{
																display_message(ERROR_MESSAGE,
																	"read_FE_element.  "
																	"Could not set grid int values");
															}
														}
													}
													DEALLOCATE(values);
												}
												else
												{
													display_message(ERROR_MESSAGE,"read_FE_element.  "
														"Insufficient memory for int values");
													return_code=0;
												}
											} break;
											default:
											{
												display_message(ERROR_MESSAGE,
													"read_FE_element.  Unsupported value_type %s",
													Value_type_string(value_type));
												return_code=0;
											} break;
										}
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_FE_element.  Invalid field #%d",i+1);
									return_code=0;
								}
							}
						}
						if ((0 < (number_of_nodes = element->information->number_of_nodes))
							&& return_code)
						{
							/* read the nodes */
							fscanf(input_file," Nodes:");
							for (i=0;(i<number_of_nodes)&&return_code;i++)
							{
								if (1==fscanf(input_file,"%d",&node_number))
								{
									if (node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,
										cm_node_identifier)(node_number,node_manager))
									{
										if (!set_FE_element_node(element,i,node))
										{
											display_message(ERROR_MESSAGE,
												"read_FE_element.  Could not set node");
											return_code=0;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"read_FE_element.  No such node %d in %s %d",node_number,
											CM_element_type_string(element_identifier.type),
											element_identifier.number);
										return_code=0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_FE_element.  Error reading node number from file");
									return_code=0;
								}
							}
						}
						if ((0 < (number_of_scale_factors =
							element->information->number_of_scale_factors)) &&
							return_code)
						{
							/*???RC scale_factors array in element_info should be private */
							/* read the scale factors */
							fscanf(input_file," Scale factors:");
							scale_factor=element->information->scale_factors;
							for (i=0;(i<number_of_scale_factors)&&return_code;i++)
							{
								if (1!=fscanf(input_file,FE_VALUE_INPUT_STRING,
									&(scale_factor[i])))
								{
									display_message(ERROR_MESSAGE,
										"read_FE_element.  Error reading scale factor from file");
									return_code=0;
								}
								if (!finite(scale_factor[i]))
								{
									display_message(ERROR_MESSAGE,"read_FE_element.  "
										"Infinity or NAN scale factor read from element file.");
									return_code=0;
								}
							}
						}
					}
				}
				if (return_code)
				{
					if (existing_element=FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,
						identifier)(&element_identifier,element_manager))
					{
						/* if there is no element->information, the shape of the new element
							 and the existing_element match and no faces were read in,
							 then the existing_element as is. This allows element files to
							 consist of just lines like "Element: # # #"
							 to build groups of elements irrespective of the fields in them */
						if ((!element->information)&&(existing_element->shape==
							element->shape)&&(0==face_token_length))
						{
							DESTROY(FE_element)(&element);
							element=existing_element;
						}
						else
						{
							/* merge the values from the existing to the new */
							if (merge_FE_element(element,existing_element))
							{
								if (MANAGER_MODIFY_NOT_IDENTIFIER(FE_element,identifier)(
									existing_element,element,element_manager))
								{
									DESTROY(FE_element)(&element);
									element=existing_element;
								}
								else
								{
									display_message(ERROR_MESSAGE,"read_FE_element.  "
										"Error modifying element in element_manager");
									DESTROY(FE_element)(&element);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_FE_element.  Error merging new information");
								DESTROY(FE_element)(&element);
							}
						}
					}
					else
					{
						if (!ADD_OBJECT_TO_MANAGER(FE_element)(element,element_manager))
						{
							display_message(ERROR_MESSAGE,"read_FE_element.  "
								"Error adding element to element_manager");
							DESTROY(FE_element)(&element);
						}
					}
				}
				else
				{
					DESTROY(FE_element)(&element);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_FE_element.  Could not create element");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"read_FE_element.  Invalid argument(s)");
	}
	LEAVE;

	return (element);
} /* read_FE_element */

/*
Global functions
----------------
*/
int read_FE_node_group_with_order(FILE *input_file,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct FE_node_order_info *node_order_info)
/*******************************************************************************
LAST MODIFIED : 3 September 2001

DESCRIPTION :
Reads node groups from an <input_file> or the socket (if <input_file> is NULL).
???RC Needs data_group_manager, node_group_manager and element_group_manager
since groups of the same name in each manager are created simultaneously, and
node groups are updated to contain nodes used by elements in associated group.
If the <node_order_info> is non NULL then each node is added to this object
in the order of the file.
==============================================================================*/
{
	char group_field_node_flag,*group_name,*last_character,*temp_string;
	int cm_node_identifier,existing_group_empty,input_result,return_code;
	struct FE_node *node,*template_node;
	struct GROUP(FE_element) *same_name_element_group;
	struct GROUP(FE_node) *existing_group,*nodes_in_file,*same_name_data_group;
	struct FE_field_order_info *field_order_info;
	struct Multi_range *new_cmiss_numbers;

	ENTER(read_FE_node_group_with_order);
	field_order_info = (struct FE_field_order_info *)NULL;
	group_name=(char *)NULL;
	template_node=(struct FE_node *)NULL;
	existing_group=(struct GROUP(FE_node) *)NULL;
	nodes_in_file=(struct GROUP(FE_node) *)NULL;
	/* use Multi_range to store cmiss numbers of nodes added to existing group */
	new_cmiss_numbers = CREATE(Multi_range)();
	return_code=1;
	input_result=1;
	/* don't need to cache all node groups, since the groups themselves are
		cached */
	MANAGER_BEGIN_CACHE(FE_node)(node_manager);
	while (return_code&&(1==input_result))
	{
		/* get the flag to say what to read next */
		if (input_file)
		{
			/* file input */
			fscanf(input_file," ");
			input_result=fscanf(input_file,"%c",&group_field_node_flag);
			/*???DB.  On the alphas input_result is 0 at the end of file when the
				fscanfs are combined " %c" ? */
			if (EOF==input_result)
			{
				/* make it look like the group is changing to re-use code for
					 establishing the group */
				group_field_node_flag='G';
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_FE_node_group_with_order.  Missing input_file");
			return_code=0;
		}
		if (return_code)
		{
			switch (group_field_node_flag)
			{
				case 'G':
				{
					/* do we currently have a group? If yes, establish it */
					if (nodes_in_file)
					{
						if (existing_group)
						{
							if (!existing_group_empty &&
								(0 < Multi_range_get_number_of_ranges(new_cmiss_numbers)))
							{
								/* report node/data numbers added to existing group */
								temp_string = (char *)NULL;
								GET_NAME(GROUP(FE_node))(existing_group, &temp_string);
								display_message(WARNING_MESSAGE, "The following nodes/data "
									"were added to group %s during read:", temp_string);
								DEALLOCATE(temp_string);
								Multi_range_display_ranges(new_cmiss_numbers);
								Multi_range_clear(new_cmiss_numbers);
							}
							if (NUMBER_IN_GROUP(FE_node)(existing_group) !=
								NUMBER_IN_GROUP(FE_node)(nodes_in_file))
							{
								temp_string = (char *)NULL;
								GET_NAME(GROUP(FE_node))(existing_group, &temp_string);
								display_message(WARNING_MESSAGE,
									"File does not contain all nodes in group %s", temp_string);
								DEALLOCATE(temp_string);
							}
							MANAGED_GROUP_END_CACHE(FE_node)(existing_group);
							DESTROY_GROUP(FE_node)(&nodes_in_file);
						}
						else
						{
							if (!(return_code=ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(
								nodes_in_file,node_group_manager)))
							{
								DESTROY(GROUP(FE_node))(&nodes_in_file);
							}
							/* make sure there are data and element groups of the same name
								 so that GT_element_group automatically created with them */
							same_name_data_group=CREATE(GROUP(FE_node))(group_name);
							if (!ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(same_name_data_group,
								data_group_manager))
							{
								DESTROY(GROUP(FE_node))(&same_name_data_group);
							}
							same_name_element_group=CREATE(GROUP(FE_element))(group_name);
							if (!ADD_OBJECT_TO_MANAGER(GROUP(FE_element))(
								same_name_element_group,element_group_manager))
							{
								DESTROY(GROUP(FE_element))(&same_name_element_group);
							}
						}
					}
					/* read the name of the group of nodes */
					if (group_name)
					{
						DEALLOCATE(group_name);
					}
					if (EOF != input_result)
					{
						if (input_file)
						{
							/* file input */
							fscanf(input_file,"roup name : "); /* the 'G' has gone */
							if (read_string(input_file,"[^\n]",&group_name))
							{
								/* trim trailing blanks */
								last_character=group_name+(strlen(group_name)-1);
								while ((last_character>group_name)&&(' '== *last_character))
								{
									last_character--;
								}
								*(last_character+1)='\0';
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Error reading node group name from file.  Line %d",
									get_line_number(input_file));
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_FE_node_group_with_order.  Missing input_file");
							return_code=0;
						}
						if (return_code)
						{
							/* determine if the group already exists */
							if (existing_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),
								name)(group_name,node_group_manager))
							{
								/* if the existing group is empty do not write warnings for
									 every new node added to it - see later */
								existing_group_empty=
									((struct FE_node *)NULL==FIRST_OBJECT_IN_GROUP_THAT(FE_node)(
										(GROUP_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
										existing_group));
								nodes_in_file=CREATE_GROUP(FE_node)((char *)NULL);
								MANAGED_GROUP_BEGIN_CACHE(FE_node)(existing_group);
							}
							else
							{
								nodes_in_file=CREATE_GROUP(FE_node)(group_name);
							}
							if (template_node)
							{
								DEACCESS(FE_node)(&template_node);
							}
						}
					}
				} break;
				case '#':
				{
					/* ensure we have a node group */
					if (nodes_in_file)
					{
						/* clear node field information */
						if (template_node)
						{
							DEACCESS(FE_node)(&template_node);
						}
						/* read new node field information and field_order_info */
						if (template_node=read_FE_node_field_info(input_file,
							fe_field_manager,&field_order_info))
						{
							ACCESS(FE_node)(template_node);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Error reading node field information");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"No current group for node field info");
						return_code=0;
					}
				} break;
				case 'N':
				{
					/* ensure we have node field information */
					if (template_node)
					{	
						/* read node */
						if (node=read_FE_node(input_file,template_node,node_manager,
							element_manager,field_order_info))
						{		
							if (node_order_info)
							{
								fill_FE_node_order_info(node, (void *)node_order_info);
							}
							/* check for repeated nodes in file */
							cm_node_identifier=get_FE_node_cm_node_identifier(node);
							if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
								cm_node_identifier,nodes_in_file))
							{
								display_message(WARNING_MESSAGE,"Node %d repeated in file",
									cm_node_identifier);
							}
							else
							{
								/* add node to the list of the nodes in the file */
								if (ADD_OBJECT_TO_GROUP(FE_node)(node,nodes_in_file))
								{
									if (existing_group)
									{
										/* check that node is in the existing group */
										if (existing_group_empty ||
											(!FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)
												(cm_node_identifier,existing_group)))
										{
											/* collate cmiss numbers of nodes added to group */
											Multi_range_add_range(new_cmiss_numbers,
												cm_node_identifier, cm_node_identifier);
											if (!ADD_OBJECT_TO_GROUP(FE_node)(node,existing_group))
											{
												display_message(ERROR_MESSAGE,
													"read_FE_node_group_with_order.  "
													"Could not add node to existing_group");
												REMOVE_OBJECT_FROM_GROUP(FE_node)(node,nodes_in_file);
												REMOVE_OBJECT_FROM_MANAGER(FE_node)(node,node_manager);
												node=(struct FE_node *)NULL;
												return_code=0;
											}
										}
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_FE_node_group_with_order.  "
										"Could not add node to nodes_in_file");
									REMOVE_OBJECT_FROM_MANAGER(FE_node)(node,node_manager);
									node=(struct FE_node *)NULL;
									return_code=0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_FE_node_group_with_order.  Error reading node");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_FE_node_group_with_order.  No current node field info for node");
						return_code=0;
					}
				} break;
				case 'V':
				{
					/* read in field values */
					if (template_node&&field_order_info)
					{
						if (!read_FE_field_values(input_file,element_manager,
							field_order_info))
						{
							display_message(ERROR_MESSAGE,
								"read_FE_node_group_with_order.  Error reading field values");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_FE_node_group_with_order.  Unexpected V[alues] token in file");
						return_code=0;
					}
				} break;
				default:
				{
					if (read_string(input_file,"[^\n]",&temp_string))
					{
						display_message(ERROR_MESSAGE,
							"Invalid flag \'%c\' in node file.  Line %d \'%c%s\'",
							group_field_node_flag,get_line_number(input_file),
							group_field_node_flag,temp_string);
						DEALLOCATE(temp_string);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Invalid flag \'%c\' in node file.  Line %d",
							group_field_node_flag,get_line_number(input_file));
					}
					return_code=0;
				} break;
			}
		}
	}
	/* clear node field information */
	if (template_node)
	{
		DEACCESS(FE_node)(&template_node);
	}
	DESTROY(Multi_range)(&new_cmiss_numbers);
	MANAGER_END_CACHE(FE_node)(node_manager);
#if defined (OLD_CODE)
	MANAGER_END_CACHE(GROUP(FE_node))(node_group_manager);
#endif /* defined (OLD_CODE)*/
	if (!return_code)
	{
		if (existing_group)
		{
			MANAGED_GROUP_END_CACHE(FE_node)(existing_group);
		}
		DESTROY_GROUP(FE_node)(&nodes_in_file);
	}
	if (group_name)
	{
		DEALLOCATE(group_name);
	}
	if (field_order_info)
  {
		DESTROY(FE_field_order_info)(&field_order_info);
  }
	LEAVE;

	return (return_code);
} /* read_FE_node_group_with_order */

int read_FE_node_group(FILE *input_file,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager)
/*******************************************************************************
LAST MODIFIED : 18 November 1999

DESCRIPTION :
Reads node groups from an <input_file> or the socket (if <input_file> is NULL).
???RC Needs data_group_manager, node_group_manager and element_group_manager
since groups of the same name in each manager are created simultaneously, and
node groups are updated to contain nodes used by elements in associated group.
==============================================================================*/
{
	int return_code;

	ENTER(read_FE_node_group);
	return_code=read_FE_node_group_with_order(input_file,fe_field_manager,
		node_manager,element_manager,node_group_manager,data_group_manager,
		element_group_manager,(struct FE_node_order_info *)NULL);
	LEAVE;

	return (return_code);
} /* read_FE_node_group */

int file_read_FE_node_group(char *file_name,void *data_void)
/*******************************************************************************
LAST MODIFIED : 6 September 1999

DESCRIPTION :
Reads a node group from a file.
==============================================================================*/
{
	FILE *input_file;
	int return_code;
	struct File_read_FE_node_group_data *data;

	ENTER(file_read_FE_node_group);
	if (data=(struct File_read_FE_node_group_data *)data_void)
	{
		/* open the input file */
		if (file_name)
		{
			if (input_file=fopen(file_name,"r"))
			{
				return_code=read_FE_node_group(input_file,data->fe_field_manager,
					data->node_manager,data->element_manager,data->node_group_manager,
					data->data_group_manager,data->element_group_manager);
				fclose(input_file);
			}
			else
			{
				display_message(ERROR_MESSAGE,"Could not open node group file: %s",
					file_name);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing node group file name");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"file_read_FE_node_group.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* file_read_FE_node_group */

int read_FE_element_group(FILE *input_file,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct MANAGER(FE_basis) *basis_manager)
/*******************************************************************************
LAST MODIFIED : 4 September 2001

DESCRIPTION :
Reads an element group from an <input_file> or the socket (if <input_file> is
NULL).
???RC Needs data_group_manager, node_group_manager and element_group_manager
since groups of the same name in each manager are created simultaneously, and
node groups are updated to contain nodes used by elements in associated group.
==============================================================================*/
{
	char group_field_shape_element_flag,*group_name,*last_character,*temp_string;
	int existing_group_empty,input_result,return_code;
	struct CM_element_information element_identifier;
	struct FE_element *element,*template_element;
	struct FE_element_list_CM_element_type_data element_list_type_data;
	struct GROUP(FE_element) *elements_in_file,*existing_group;
	struct FE_element_shape *element_shape;
	struct GROUP(FE_node) *same_name_data_group,*same_name_node_group;
	struct LIST(FE_node) *nodes_to_add;
	struct LIST(FE_element) *temp_element_list;
	struct FE_field_order_info *field_order_info;
	enum CM_element_type last_cm_element_type;
	struct Multi_range *new_cmiss_numbers;

	ENTER(read_FE_element_group);
	field_order_info = (struct FE_field_order_info *)NULL;
	group_name=(char *)NULL;
	template_element=(struct FE_element *)NULL;
	existing_group=(struct GROUP(FE_element) *)NULL;
	elements_in_file=(struct GROUP(FE_element) *)NULL;
	element_shape=(struct FE_element_shape *)NULL;
	/* use Multi_range to store cmiss numbers of elements added to existing group */
	last_cm_element_type = CM_ELEMENT_TYPE_INVALID;
	new_cmiss_numbers = CREATE(Multi_range)();
	return_code=1;
	input_result=1;
	/* don't need to cache all element groups since each individual group read
		 is cached */
#if defined (OLD_CODE)
	MANAGER_BEGIN_CACHE(GROUP(FE_element))(element_group_manager);
#endif /* defined (OLD_CODE) */
	MANAGER_BEGIN_CACHE(FE_element)(element_manager);
	while (return_code&&(1==input_result))
	{
		/* get the flag to say what to read next */
		if (input_file)
		{
			/* file input */
			fscanf(input_file," ");
			input_result=fscanf(input_file,"%c",&group_field_shape_element_flag);
			/*???DB.  On the alphas input_result is 0 at the end of file when the
				fscanfs are combined " %c" ? */
			if (EOF==input_result)
			{
				/* make it look like the group is changing to re-use code for
					 establishing the group */
				group_field_shape_element_flag='G';
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_FE_element_group.  Missing input_file");
			return_code=0;
		}
		if (return_code)
		{
			switch (group_field_shape_element_flag)
			{
				case 'G':
				{
					/* do we currently have a group? If yes, establish it */
					if (elements_in_file)
					{
						/* make sure faces and lines of elements are also in the group. Make
							 a temporary list of top-level elements since group may change. */
						if (temp_element_list = CREATE(LIST(FE_element))())
						{
							element_list_type_data.cm_element_type = CM_ELEMENT;
							element_list_type_data.element_list = temp_element_list;
							FOR_EACH_OBJECT_IN_GROUP(FE_element)(
								add_FE_element_of_CM_element_type_to_list,
								(void *)&element_list_type_data, elements_in_file);
							FOR_EACH_OBJECT_IN_LIST(FE_element)(
								ensure_FE_element_and_faces_are_in_group,
								(void *)elements_in_file,temp_element_list);
							if (existing_group)
							{
								/* make sure existing group also has faces and lines added */
								FOR_EACH_OBJECT_IN_LIST(FE_element)(
									ensure_FE_element_and_faces_are_in_group,
									(void *)existing_group,temp_element_list);
							}
							DESTROY(LIST(FE_element))(&temp_element_list);
						}
						if (existing_group)
						{
							/* update the node_group of the same name to contain at least the
								 nodes in the elements of the existing_group */
							if (same_name_node_group=FIND_BY_IDENTIFIER_IN_MANAGER(
								GROUP(FE_node),name)(group_name,node_group_manager))
							{
								nodes_to_add=CREATE(LIST(FE_node))();
								FOR_EACH_OBJECT_IN_GROUP(FE_element)(
									ensure_top_level_FE_element_nodes_are_in_list,
									(void *)nodes_to_add,existing_group);
								FOR_EACH_OBJECT_IN_GROUP(FE_node)(
									ensure_FE_node_is_not_in_list,(void *)nodes_to_add,
									same_name_node_group);
								/* only make change if any new nodes to be added */
								if (FIRST_OBJECT_IN_LIST_THAT(FE_node)(
									(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
									nodes_to_add))
								{
									MANAGED_GROUP_BEGIN_CACHE(FE_node)(same_name_node_group);
									FOR_EACH_OBJECT_IN_LIST(FE_node)(ensure_FE_node_is_in_group,
										(void *)same_name_node_group,nodes_to_add);
									MANAGED_GROUP_END_CACHE(FE_node)(same_name_node_group);
								}
								DESTROY(LIST(FE_node))(&nodes_to_add);
							}
							else
							{
								display_message(WARNING_MESSAGE,"read_FE_element_group.  "
									"No node group of same name as element group %s",group_name);
							}
							if (!existing_group_empty &&
								(0 < Multi_range_get_number_of_ranges(new_cmiss_numbers)))
							{
								/* report element numbers added to non-empty existing group */
								temp_string = (char *)NULL;
								GET_NAME(GROUP(FE_element))(existing_group, &temp_string);
								display_message(WARNING_MESSAGE,
									"The following %s(s) were added to group %s during read:",
									CM_element_type_string(last_cm_element_type), temp_string);
								DEALLOCATE(temp_string);
								Multi_range_display_ranges(new_cmiss_numbers);
								Multi_range_clear(new_cmiss_numbers);
								last_cm_element_type = CM_ELEMENT_TYPE_INVALID;
							}
							if (NUMBER_IN_GROUP(FE_element)(existing_group)!=
								NUMBER_IN_GROUP(FE_element)(elements_in_file))
							{
								display_message(WARNING_MESSAGE,"read_FE_element_group.  "
									"File does not contain complete element group");
							}
							MANAGED_GROUP_END_CACHE(FE_element)(existing_group);
							DESTROY_GROUP(FE_element)(&elements_in_file);
						}
						else
						{
							/* make sure there are data and node groups of the same name
								 so that GT_element_group automatically created with them.
								 Also fill node_group with all the nodes referred to by elements
								 in the element group */
							same_name_data_group=CREATE(GROUP(FE_node))(group_name);
							if (!ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(same_name_data_group,
								data_group_manager))
							{
								DESTROY(GROUP(FE_node))(&same_name_data_group);
							}
							same_name_node_group=CREATE(GROUP(FE_node))(group_name);
							nodes_to_add=CREATE(LIST(FE_node))();
							FOR_EACH_OBJECT_IN_GROUP(FE_element)(
								ensure_top_level_FE_element_nodes_are_in_list,
								(void *)nodes_to_add,elements_in_file);
							FOR_EACH_OBJECT_IN_LIST(FE_node)(ensure_FE_node_is_in_group,
								(void *)same_name_node_group,nodes_to_add);
							DESTROY(LIST(FE_node))(&nodes_to_add);
							if (!ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(same_name_node_group,
								node_group_manager))
							{
								DESTROY(GROUP(FE_node))(&same_name_node_group);
							}
							if (!(return_code=ADD_OBJECT_TO_MANAGER(GROUP(FE_element))(
								elements_in_file,element_group_manager)))
							{
								DESTROY(GROUP(FE_element))(&elements_in_file);
							}
						}
					}
					/* read the name of the group of elements */
					if (group_name)
					{
						DEALLOCATE(group_name);
					}
					if (EOF != input_result)
					{
						if (input_file)
						{
							/* file input */
							fscanf(input_file,"roup name : ");
							if (read_string(input_file,"[^\n]",&group_name))
							{
								/* trim trailing blanks */
								last_character=group_name+(strlen(group_name)-1);
								while ((last_character>group_name)&&(' '== *last_character))
								{
									last_character--;
								}
								*(last_character+1)='\0';
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,"read_FE_element_group.  "
									"Error reading element group name from file.  Line %d",
									get_line_number(input_file));
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_FE_element_group.  Missing input_file");
							return_code=0;
						}
						if (return_code)
						{
							/* determine if the group already exists */
							if (existing_group=FIND_BY_IDENTIFIER_IN_MANAGER(
								GROUP(FE_element),name)(group_name,element_group_manager))
							{
								/* if the existing group is empty do not write warnings for
									 every new element added to it - see later */
								existing_group_empty=((struct FE_element *)NULL==
									FIRST_OBJECT_IN_GROUP_THAT(FE_element)(
										(GROUP_CONDITIONAL_FUNCTION(FE_element) *)NULL,(void *)NULL,
										existing_group));
								elements_in_file=CREATE_GROUP(FE_element)((char *)NULL);
								MANAGED_GROUP_BEGIN_CACHE(FE_element)(existing_group);
							}
							else
							{
								elements_in_file=CREATE_GROUP(FE_element)(group_name);
							}
							if (template_element)
							{
								DEACCESS(FE_element)(&template_element);
							}
						}
					}
				} break;
				case '#':
				{
					/* ensure we have an element group */
					if (elements_in_file)
					{
						/* clear element field information */
						if (template_element)
						{
							DEACCESS(FE_element)(&template_element);
						}
						/* read new element field information and field_order_info */
						if (template_element=read_FE_element_field_info(input_file,
							element_shape,fe_field_manager,basis_manager,&field_order_info))
						{
							ACCESS(FE_element)(template_element);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Error reading element field information");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"No current group for element field info");
						return_code=0;
					}
				} break;
				case 'S':
				{
					/* clear element shape information */
					DEACCESS(FE_element_shape)(&element_shape);
					/* read element shape information */
					if (element_shape=read_FE_element_shape(input_file))
					{
						ACCESS(FE_element_shape)(element_shape);
						/* clear element field information */
						if (template_element)
						{
							DEACCESS(FE_element)(&template_element);
						}
						/* create the initial template element for no fields */
						element_identifier.type=CM_ELEMENT;
						element_identifier.number=0;
						template_element=CREATE(FE_element)(&element_identifier,
							(struct FE_element *)NULL);
						ACCESS(FE_element)(template_element);
						set_FE_element_shape(template_element,element_shape);
						/* clear field_order_info */
						if (field_order_info)
						{
							DESTROY(FE_field_order_info)(&field_order_info);
						}
						field_order_info = CREATE(FE_field_order_info)();
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_FE_element_group.  Error reading element shape");
						return_code=0;
					}
				} break;
				case 'E':
				{
					/* ensure we have element field information */
					if (template_element)
					{	
						/* read element */
						if (element=read_FE_element(input_file,template_element,
							element_manager,node_manager,field_order_info))
						{
							/* check for repeated elements in file */
							if (FIND_BY_IDENTIFIER_IN_GROUP(FE_element,identifier)(
								element->identifier,elements_in_file))
							{
								display_message(WARNING_MESSAGE,
									"read_FE_element_group.  Element repeated in file");
							}
							else
							{
								/* add element to the list of the elements in the file */
								if (ADD_OBJECT_TO_GROUP(FE_element)(element,elements_in_file))
								{
									if (existing_group)
									{
										/* check that element is in the existing group */
										if (existing_group_empty||
											(!FIND_BY_IDENTIFIER_IN_GROUP(FE_element,identifier)(
												element->identifier,existing_group)))
										{
											if (!existing_group_empty)
											{
												/* report on elements of different type added thus far */
												if ((0 <
													Multi_range_get_number_of_ranges(new_cmiss_numbers)) &&
													(element->identifier->type != last_cm_element_type))
												{
													/* report element numbers added to existing group */
													temp_string = (char *)NULL;
													GET_NAME(GROUP(FE_element))(existing_group, &temp_string);
													display_message(WARNING_MESSAGE, "The following %s(s) "
														"were added to group %s during read:",
														CM_element_type_string(last_cm_element_type),
														temp_string);
													DEALLOCATE(temp_string);
													Multi_range_display_ranges(new_cmiss_numbers);
													Multi_range_clear(new_cmiss_numbers);
												}
												/* collate cmiss numbers of elements added to group */
												Multi_range_add_range(new_cmiss_numbers,
													element->identifier->number, element->identifier->number);
												last_cm_element_type = element->identifier->type;
											}
											if (!ADD_OBJECT_TO_GROUP(FE_element)(element,
												existing_group))
											{
												display_message(ERROR_MESSAGE,"read_FE_element_group.  "
													"Could not add element to existing_group");
												REMOVE_OBJECT_FROM_GROUP(FE_element)(element,
													elements_in_file);
												REMOVE_OBJECT_FROM_MANAGER(FE_element)(element,
													element_manager);
												element=(struct FE_element *)NULL;
												return_code=0;
											}
										}
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_FE_element_group.  Could not add element to group");
									DESTROY(FE_element)(&element);
									return_code=0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_FE_element_group.  Error reading element");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"read_FE_element_group.  "
							"No current element field info for element");
						return_code=0;
					}
				} break;
				case 'V':
				{
					/* read in field values */
					if (template_element&&field_order_info)
					{
						if (!read_FE_field_values(input_file,element_manager,
							field_order_info))
						{
							display_message(ERROR_MESSAGE,
								"read_FE_element_group.  Error reading field values");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_FE_element_group.  Unexpected V[alues] token in file");
						return_code=0;
					}
				} break;
				default:
				{
					if (read_string(input_file,"[^\n]",&temp_string))
					{
						display_message(ERROR_MESSAGE,
							"Invalid flag \'%c\' in element file.  Line %d \'%c%s\'",
							group_field_shape_element_flag,get_line_number(input_file),
							group_field_shape_element_flag,temp_string);
						DEALLOCATE(temp_string);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Invalid flag \'%c\' in element file.  Line %d",
							group_field_shape_element_flag,get_line_number(input_file));
					}
					return_code=0;
				} break;
			}
		}
	}
	/* deaccess shape and clear element field information */
	DEACCESS(FE_element_shape)(&element_shape);
	if (template_element)
	{
		DEACCESS(FE_element)(&template_element);
	}
	DESTROY(Multi_range)(&new_cmiss_numbers);
	if (!return_code)
	{
		if (existing_group)
		{
			MANAGED_GROUP_END_CACHE(FE_element)(existing_group);
		}
		DESTROY_GROUP(FE_element)(&elements_in_file);
	}
	if (group_name)
	{
		DEALLOCATE(group_name);
	}
	MANAGER_END_CACHE(FE_element)(element_manager);
#if defined (OLD_CODE)
	MANAGER_END_CACHE(GROUP(FE_element))(element_group_manager);
#endif /* defined (OLD_CODE)*/
	if (field_order_info)
  {
		DESTROY(FE_field_order_info)(&field_order_info);
  }
	LEAVE;

	return (return_code);
} /* read_FE_element_group */

int file_read_FE_element_group(char *file_name,void *data_void)
/*******************************************************************************
LAST MODIFIED : 22 April 1999

DESCRIPTION :
Reads an element group from a file.
==============================================================================*/
{
	FILE *input_file;
	int return_code;
	struct File_read_FE_element_group_data *data;

	ENTER(file_read_FE_element_group);
	if (data=(struct File_read_FE_element_group_data *)data_void)
	{
		/* open the input file */
		if (file_name)
		{
			if (input_file=fopen(file_name,"r"))
			{
				return_code=read_FE_element_group(input_file,data->element_manager,
					data->element_group_manager,data->fe_field_manager,
					data->node_manager,data->node_group_manager,
					data->data_group_manager,data->basis_manager);
				fclose(input_file);
			}
			else
			{
				display_message(ERROR_MESSAGE,"Could not open element group file: %s",
					file_name);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing element group file name");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"file_read_FE_element_group.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* file_read_FE_element_group */

int read_exnode_or_exelem_file_from_string(char *exnode_string,char *exelem_string,
	char *name,struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_node))*node_group_manager,
	struct MANAGER(GROUP(FE_node))*data_group_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_basis) *basis_manager)
/*******************************************************************************
LAST MODIFIED :9 October 2000

DESCRIPTION : given a string <exnode_string> containing an entire exnode file,
XOR a string <exelem_string> containing an entire exelem file, reads in the node 
or element group(s). Renames the first node or element group <name> 
(i.e ignores the first node or element group name in <exnode_string>/<exelem_string>)
Does all this by writing out <exnode_string>/<exelem_string> to a temporary file, 
and reading it back in with read_FE_node_group/read_FE_element_group
This is generally done so we can statically include an exnode or exelem file (in
<exnode_string>/<exelem_string>)
==============================================================================*/
{
	int string_len;
	char group_name_str[]=" Group name: ";
	char *exnode_or_exelem_string,*line_str,*outstr;		
	FILE *input_file,*output_file;
	int return_code;

	ENTER(read_exnode_or_exelem_file_from_string);
	input_file=(FILE *)NULL;
	output_file=(FILE *)NULL;
	outstr=(char *)NULL;
	line_str=(char *)NULL;		
	return_code=0;
	/* exnode_string XOR exelem_string defined, other NULL*/
	if(((exnode_string&&!exelem_string)||(!exnode_string&&exelem_string))&&
		fe_field_manager&&node_manager&&element_manager&&node_group_manager&&
		data_group_manager&&element_group_manager)
	{	
		if(exnode_string)
		{
			exnode_or_exelem_string=exnode_string;
		}
		else
		/* exelem_string */
		{
			exnode_or_exelem_string=exelem_string;
		}
		/* set up the new node group name */	
		string_len = strlen(group_name_str);
		string_len++;
		string_len+=strlen(name);
		string_len++;
		if(ALLOCATE(line_str,char,string_len))
		{
			strcpy(line_str,group_name_str);
			strcat(line_str,name);
			strcat(line_str,"\n");	
			/* open a temp file to write to */
			if(output_file=fopen("temp_exnode_or_exelem_file", "w"))
			{
				/*write out the new group name */
				if(fwrite(line_str,1,strlen(line_str),output_file))
				{
					/*move the start of the string to to the start of the second line (after the \n) */
					/*(The first line is the group name and we've  just replaced this)*/
					outstr=exnode_or_exelem_string;
					while(*outstr!='\n')
					{		
						outstr++;				
					}
					/*move past the \n */
					outstr++;
					/*write the string to the temp file*/
					if(fwrite(outstr,1,strlen(outstr),output_file))
					{
						fclose(output_file);
						/* read the temp file in to a node or element group */
						if (input_file=fopen("temp_exnode_or_exelem_file","r"))
						{
							if(exnode_string)
							{
								return_code=read_FE_node_group(input_file,fe_field_manager,
									node_manager,element_manager,node_group_manager,data_group_manager,
									element_group_manager);			
							}
							else
							/* exelem_string */
							{
								return_code=read_FE_element_group(input_file,element_manager,
									element_group_manager,fe_field_manager,node_manager,
									node_group_manager,data_group_manager,basis_manager);	
							}
							fclose(input_file);
						}
					}
					else
					{
						return_code=0;
						fclose(output_file);
						display_message(ERROR_MESSAGE,"read_exnode_or_exelem_file_from_string."
							" failed to write file");
					}
				}
				else
				{
					return_code=0;
					fclose(output_file);
					display_message(ERROR_MESSAGE,"read_exnode_or_exelem_file_from_string."
						" failed to write name to file");
				}			
				/*remove the temp file*/						
				remove("temp_exnode_or_exelem_file");				
			}
			else
			{
				return_code=0;
				display_message(ERROR_MESSAGE,"read_exnode_or_exelem_file_from_string."
					" failed to open file");
			}
		}
		else
		{
			return_code=0;
			display_message(ERROR_MESSAGE,"read_exnode_or_exelem_file_from_string. out of memory");
		}
		DEALLOCATE(line_str);
	}
	else
	{
		display_message(ERROR_MESSAGE,"read_exnode_or_exelem_file_from_string. Invalid arguments");
	}
	LEAVE;
	return(return_code);
} /* read_exnode_or_exelem_file_from_string */

int read_exnode_and_exelem_file_from_string_and_offset(
	char *exnode_string,char *exelem_string,
	char *name,int offset,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_node))*node_group_manager,
	struct MANAGER(GROUP(FE_node))*data_group_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_basis) *basis_manager)
/*******************************************************************************
LAST MODIFIED :3 November 2000

DESCRIPTION :
Given a string <exnode_string> containing an entire exnode file,and a string 
<exelem_string> containing an entire exelem file, reads in the node and 
element group(s), names them <name>, and shifts the node and element identifier 
numbers to <offset>
==============================================================================*/
{	
	int return_code;

	ENTER(read_exnode_and_exelem_file_from_string_and_offset);
	if(exnode_string&&exelem_string&&fe_field_manager&&node_manager&&element_manager&&
		node_group_manager&&data_group_manager&&element_group_manager&&basis_manager)
	{				
		/* read in the default torso mesh nodes and elements */
		/* (cleaned up when the program shuts down) */		
		if((return_code=read_exnode_or_exelem_file_from_string(exnode_string,
			(char *)NULL,name,fe_field_manager,node_manager,element_manager,
			node_group_manager,data_group_manager,element_group_manager,basis_manager))&&
			(return_code=read_exnode_or_exelem_file_from_string((char *)NULL,
				exelem_string,name,fe_field_manager,node_manager,
				element_manager,node_group_manager,data_group_manager,element_group_manager
				,basis_manager)))
		{
			/* offset the nodea and elements */
			return_code=offset_FE_node_and_element_identifiers_in_group(name,offset,
				node_manager,element_manager,node_group_manager,
				element_group_manager);
		}								
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,"read_exnode_and_exelem_file_from_string_and_offset."
			" Invalid arguments");
	}
	LEAVE;
	return(return_code);
}

char *get_first_group_name_from_FE_node_file(char *group_file_name)
/*******************************************************************************
LAST MODIFIED :2 November 2000

DESCRIPTION :
Allocates and returns the name of the first node group, in the exnode file refered 
to by <group_file_name>. It is up to the user to DEALLOCATE the returned 
group name.	
Read in the group name by peering into the node file. This is a bit inelegant,
and will only get the name of the FIRST group, so semi-assuming the exnode file 
only has one group (although it'd be posible to get others). Do this as 
read_FE_node_group doesn't (yet?) return info about the nodes, groups or 
fields .
==============================================================================*/
{	
	char *group_name,input_str[13];
	FILE *input_file;

	ENTER(get_first_group_name_from_FE_node_file);
	group_name=(char *)NULL;
	input_file=(FILE *)NULL;
	if(group_file_name)
	{
		if (input_file=fopen(group_file_name,"r"))
		{										
			fscanf(input_file," ");
			fscanf(input_file,"%12c",input_str);
			/* NULL terminate for use with strcmp */	
			input_str[12]='\0';
			if(!strcmp("Group name: ",input_str))
			{
				read_string(input_file,"[^\n]",&group_name);			
			}	
			else
			{			
				display_message(ERROR_MESSAGE,
					"get_first_group_name_from_FE_node_file."
					" group name corrupt");
			}	
			fclose(input_file);
		}
		else
		{		
			display_message(ERROR_MESSAGE,
				"get_first_group_name_from_FE_node_file. "
				"failed to read group name from exnode file ");
		}
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"get_first_group_name_from_FE_node_file. "
			"invalid arguments ");
	}
	LEAVE;
	return(group_name);
}/* get_first_group_name_from_FE_node_file*/

int read_FE_node_and_elem_groups_and_return_name_given_file_name(char *group_file_name,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_node))*node_group_manager,
	struct MANAGER(GROUP(FE_node))*data_group_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_basis) *basis_manager,
	char **group_name)
/*******************************************************************************
LAST MODIFIED :2 November 2000

DESCRIPTION :
Given a string <group_file_name>  reads in the corresponding node and 
element group(s)
Eg if <group_file_name> is "/usr/bob/default_torso", reads in 
"/usr/bob/default_torso.exnode" and "/usr/bob/default_torso.exelem".
Also returns the name of the first node and element group, in <group_name>.
It is up to the user to DEALLOCATE <group_name>.
==============================================================================*/
{	
	char exnode_ext[]=".exnode";
	char exelem_ext[]=".exelem";
	char *exnode_name_str,*exelem_name_str;
	FILE *input_file;
	int exnode_name_str_len,exelem_name_str_len,return_code,string_len;

	ENTER(read_FE_node_and_elem_groups_and_return_name_given_file_name);
	input_file=(FILE *)NULL;
	exnode_name_str=(char *)NULL;
	exelem_name_str=(char *)NULL;
	if(group_file_name&&fe_field_manager&&node_manager&&element_manager&&
		node_group_manager&&data_group_manager&&element_group_manager&&
		basis_manager)
	{
		/*construct full names for exnode and exelem files*/
		string_len = strlen(group_file_name);
		exnode_name_str_len = string_len +strlen(exnode_ext); 
		exelem_name_str_len = string_len +strlen(exelem_ext);
		if(ALLOCATE(exnode_name_str,char,exnode_name_str_len)&&
			ALLOCATE(exelem_name_str,char,exelem_name_str_len))
		{
			return_code=1;
			/* add the extensions to the file name */
			strcpy(exnode_name_str,group_file_name);
			strcat(exnode_name_str,exnode_ext);
			strcpy(exelem_name_str,group_file_name);
			strcat(exelem_name_str,exelem_ext);
			/*get thenode and element  group name*/
			*group_name=get_first_group_name_from_FE_node_file(exnode_name_str);
			/* read in the node file */
			if(*group_name)
			{
				if (input_file=fopen(exnode_name_str,"r"))
				{
					return_code=read_FE_node_group(input_file,fe_field_manager,node_manager,
						element_manager,node_group_manager,data_group_manager,element_group_manager);
				}
				else
				{
					return_code=0;
					display_message(ERROR_MESSAGE,
						"read_FE_node_and_elem_groups_and_return_name_given_file_name."
						" failed to open file exnode_name_str  ");
				}
				fclose(input_file);
			}
			else
			{
				return_code=0;
			}
			/* read in the element file */
			if(return_code)
			{
				if (input_file=fopen(exelem_name_str,"r"))
				{
					return_code=read_FE_element_group(input_file,element_manager,
						element_group_manager,fe_field_manager,node_manager,
						node_group_manager,data_group_manager,basis_manager);	
				}
				else
				{
					return_code=0;
					display_message(ERROR_MESSAGE,
						"read_FE_node_and_elem_groups_and_return_name_given_file_name. "
						"failed to open file exnode_name_str  ");
				}
				fclose(input_file);
			}		
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_FE_node_and_elem_groups_and_return_name_given_file_name. "
				"out of memory for exnode_name_str ");
			return_code=0;
		}									
		DEALLOCATE(exnode_name_str);
		DEALLOCATE(exelem_name_str);	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_FE_node_and_elem_groups_and_return_name_given_file_name. "
			"invalid arguments ");
		return_code=0;
	}	
	LEAVE;
	return(return_code);
} /* read_FE_node_and_elem_groups_and_return_name_given_file_name */
