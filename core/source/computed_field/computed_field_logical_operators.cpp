/*******************************************************************************
FILE : computed_field_logical_operators.c

LAST MODIFIED : 16 May 2008

DESCRIPTION :
Implements a number of logical operations on computed fields.
Three methods are developed here: AND, OR, XOR, EQUAL_TO, LESS_THAN, GREATER_THAN
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
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"
#include "computed_field/computed_field_logical_operators.h"

class Computed_field_logical_operators_package : public Computed_field_type_package
{
};

namespace {

const char computed_field_or_type_string[] = "or";

class Computed_field_or : public Computed_field_core
{
public:
	Computed_field_or() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_or();
	}

	const char *get_type_string()
	{
		return(computed_field_or_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_or*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

};

int Computed_field_or::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (source1Cache && source2Cache)
	{
		for (int i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] = (0.0 != source1Cache->values[i]) || (0.0 != source2Cache->values[i]);
		}
		valueCache.derivatives_valid = 0;
		return 1;
	}
	return 0;
}

} //namespace

Computed_field *Cmiss_field_module_create_or(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
{
	Computed_field *field = NULL;
	/* Access and broadcast before checking components match,
		the local source_field_one and source_field_two will
		get replaced if necessary. */
	ACCESS(Computed_field)(source_field_one);
	ACCESS(Computed_field)(source_field_two);
	if (field_module && source_field_one && source_field_one->isNumerical() &&
		source_field_two && source_field_two->isNumerical() &&
		Computed_field_broadcast_field_components(field_module,
			&source_field_one, &source_field_two) &&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		Computed_field *source_fields[2];
		source_fields[0] = source_field_one;
		source_fields[1] = source_field_two;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field_one->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_or());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_or.  Invalid argument(s)");
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);

	return (field);
}

int Computed_field_get_type_or(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_XYZ, the
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_or);
	if (field&&(dynamic_cast<Computed_field_or*>(field->core)))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_or.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_or */

namespace {

const char computed_field_and_type_string[] = "and";

class Computed_field_and : public Computed_field_core
{
public:
	Computed_field_and() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_and();
	}

	const char *get_type_string()
	{
		return(computed_field_and_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_and*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

};

int Computed_field_and::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (source1Cache && source2Cache)
	{
		for (int i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] = (0.0 != source1Cache->values[i]) && (0.0 != source2Cache->values[i]);
		}
		valueCache.derivatives_valid = 0;
		return 1;
	}
	return 0;
}

} //namespace

Computed_field *Cmiss_field_module_create_and(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
{
	Computed_field *field = NULL;
	/* Access and broadcast before checking components match,
		the local source_field_one and source_field_two will
		get replaced if necessary. */
	ACCESS(Computed_field)(source_field_one);
	ACCESS(Computed_field)(source_field_two);
	if (field_module && source_field_one && source_field_one->isNumerical() &&
		source_field_two && source_field_two->isNumerical() &&
		Computed_field_broadcast_field_components(field_module,
			&source_field_one, &source_field_two) &&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		Computed_field *source_fields[2];
		source_fields[0] = source_field_one;
		source_fields[1] = source_field_two;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field_one->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_and());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_or.  Invalid argument(s)");
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);

	return (field);
}

int Computed_field_get_type_and(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_AND, the
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_and);
	if (field&&(dynamic_cast<Computed_field_and*>(field->core)))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_and.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_and */

namespace {

const char computed_field_xor_type_string[] = "xor";

class Computed_field_xor : public Computed_field_core
{
public:
	Computed_field_xor() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_xor();
	}

	const char *get_type_string()
	{
		return(computed_field_xor_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_xor*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);
};

int Computed_field_xor::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (source1Cache && source2Cache)
	{
		for (int i = 0 ; i < field->number_of_components ; i++)
		{
			if (0.0 != source1Cache->values[i])
			{
				valueCache.values[i] = (0.0 == source2Cache->values[i]);
			}
			else
			{
				valueCache.values[i] = (0.0 != source2Cache->values[i]);
			}
		}
		valueCache.derivatives_valid = 0;
		return 1;
	}
	return 0;
}

} //namespace

Computed_field *Cmiss_field_module_create_xor(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
{
	Computed_field *field = NULL;
	/* Access and broadcast before checking components match,
		the local source_field_one and source_field_two will
		get replaced if necessary. */
	ACCESS(Computed_field)(source_field_one);
	ACCESS(Computed_field)(source_field_two);
	if (field_module && source_field_one && source_field_one->isNumerical() &&
		source_field_two && source_field_two->isNumerical() &&
		Computed_field_broadcast_field_components(field_module,
			&source_field_one, &source_field_two) &&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		Computed_field *source_fields[2];
		source_fields[0] = source_field_one;
		source_fields[1] = source_field_two;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field_one->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_xor());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_or.  Invalid argument(s)");
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);

	return (field);
}

int Computed_field_get_type_xor(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_XOR, the
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_xor);
	if (field&&(dynamic_cast<Computed_field_xor*>(field->core)))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_xor.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_xor */

namespace {

const char computed_field_equal_to_type_string[] = "equal_to";

class Computed_field_equal_to : public Computed_field_core
{
public:
	Computed_field_equal_to() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_equal_to();
	}

	const char *get_type_string()
	{
		return(computed_field_equal_to_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_equal_to*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);
};

int Computed_field_equal_to::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	switch (Cmiss_field_get_value_type(getSourceField(0)))
	{
		case CMISS_FIELD_VALUE_TYPE_REAL:
		{
			RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
			RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
			if (source1Cache && source2Cache)
			{
				for (int i = 0 ; i < field->number_of_components ; i++)
				{
					valueCache.values[i] = (source1Cache->values[i] == source2Cache->values[i]) ? 1.0 : 0.0;
				}
				valueCache.derivatives_valid = 0;
				return 1;
			}
		} break;
		case CMISS_FIELD_VALUE_TYPE_STRING:
		{
			StringFieldValueCache *source1Cache = StringFieldValueCache::cast(getSourceField(0)->evaluate(cache));
			StringFieldValueCache *source2Cache = StringFieldValueCache::cast(getSourceField(1)->evaluate(cache));
			if (source1Cache && source2Cache)
			{
				FE_value result = (FE_value)(0 == strcmp(source1Cache->stringValue, source2Cache->stringValue));
				// should only be one component if string arguments
				for (int i = 0 ; i < field->number_of_components ; i++)
				{
					valueCache.values[i] = result;
				}
				return 1;
			}
		} break;
		default:
		{
			// unsupported type
		} break;
	}
	return 0;
}

} //namespace

Computed_field *Cmiss_field_module_create_equal_to(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
{
	Computed_field *field = NULL;
	/* Access and broadcast before checking components match,
		the local source_field_one and source_field_two will
		get replaced if necessary. */
	ACCESS(Computed_field)(source_field_one);
	ACCESS(Computed_field)(source_field_two);
	if (field_module && source_field_one && source_field_two &&
		(Cmiss_field_get_value_type(source_field_one) == Cmiss_field_get_value_type(source_field_two)) &&
		Computed_field_broadcast_field_components(field_module,
			&source_field_one, &source_field_two) &&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		Computed_field *source_fields[2];
		source_fields[0] = source_field_one;
		source_fields[1] = source_field_two;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field_one->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_equal_to());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_or.  Invalid argument(s)");
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);

	return (field);
}

int Computed_field_get_type_equal_to(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_EQUAL_TO, the
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_equal_to);
	if (field&&(dynamic_cast<Computed_field_equal_to*>(field->core)))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_equal_to.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_equal_to */

namespace {

const char computed_field_less_than_type_string[] = "less_than";

class Computed_field_less_than : public Computed_field_core
{
public:
	Computed_field_less_than() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_less_than();
	}

	const char *get_type_string()
	{
		return(computed_field_less_than_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_less_than*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);
};

int Computed_field_less_than::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (source1Cache && source2Cache)
	{
		for (int i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] = (source1Cache->values[i] < source2Cache->values[i]) ? 1.0 : 0.0;
		}
		valueCache.derivatives_valid = 0;
		return 1;
	}
	return 0;
}

} //namespace

Computed_field *Computed_field_create_less_than(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
{
	Computed_field *field = NULL;
	/* Access and broadcast before checking components match,
		the local source_field_one and source_field_two will
		get replaced if necessary. */
	ACCESS(Computed_field)(source_field_one);
	ACCESS(Computed_field)(source_field_two);
	if (field_module && source_field_one  && source_field_one->isNumerical() &&
		source_field_two && source_field_two->isNumerical() &&
		Computed_field_broadcast_field_components(field_module,
			&source_field_one, &source_field_two) &&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		Computed_field *source_fields[2];
		source_fields[0] = source_field_one;
		source_fields[1] = source_field_two;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field_one->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_less_than());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_or.  Invalid argument(s)");
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);

	return (field);
}

int Computed_field_get_type_less_than(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_LESS_THAN, the
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_less_than);
	if (field&&(dynamic_cast<Computed_field_less_than*>(field->core)))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_less_than.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_less_than */

namespace {

const char computed_field_greater_than_type_string[] = "greater_than";

class Computed_field_greater_than : public Computed_field_core
{
public:
	Computed_field_greater_than() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_greater_than();
	}

	const char *get_type_string()
	{
		return(computed_field_greater_than_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_greater_than*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);
};

int Computed_field_greater_than::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (source1Cache && source2Cache)
	{
		for (int i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] = (source1Cache->values[i] > source2Cache->values[i]) ? 1.0 : 0.0;
		}
		valueCache.derivatives_valid = 0;
		return 1;
	}
	return 0;
}

} //namespace

Computed_field *Computed_field_create_greater_than(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
{
	Computed_field *field = NULL;
	/* Access and broadcast before checking components match,
		the local source_field_one and source_field_two will
		get replaced if necessary. */
	ACCESS(Computed_field)(source_field_one);
	ACCESS(Computed_field)(source_field_two);
	if (field_module && source_field_one  && source_field_one->isNumerical() &&
		source_field_two && source_field_two->isNumerical() &&
		Computed_field_broadcast_field_components(field_module,
			&source_field_one, &source_field_two) &&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		Computed_field *source_fields[2];
		source_fields[0] = source_field_one;
		source_fields[1] = source_field_two;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field_one->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_greater_than());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_or.  Invalid argument(s)");
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);

	return (field);
}

int Computed_field_get_type_greater_than(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_GREATER_THAN, the
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_greater_than);
	if (field&&(dynamic_cast<Computed_field_greater_than*>(field->core)))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_greater_than.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_greater_than */

namespace {

const char computed_field_is_defined_type_string[] = "is_defined";

class Computed_field_is_defined : public Computed_field_core
{
public:
	Computed_field_is_defined() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_is_defined();
	}

	const char *get_type_string()
	{
		return(computed_field_is_defined_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_is_defined*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	virtual bool is_defined_at_location(Cmiss_field_cache&)
	{
		return true;
	}
};

int Computed_field_is_defined::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
	valueCache.values[0] = getSourceField(0)->core->is_defined_at_location(cache);
	return 1;
}

} //namespace

/*****************************************************************************//**
 * Creates a scalar field whose value is 1 wherever the source field is defined,
 * and 0 elsewhere (without error).
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  Source field to check whether defined.
 * @return Newly created field
 */
Computed_field *Computed_field_create_is_defined(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field)
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/true,
		/*number_of_components*/1,
		/*number_of_source_fields*/1, &source_field,
		/*number_of_source_values*/0, NULL,
		new Computed_field_is_defined());

	return (field);
}

int Computed_field_get_type_is_defined(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_IS_DEFINED, the
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_is_defined);
	if (field&&(dynamic_cast<Computed_field_is_defined*>(field->core)))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_is_defined.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_is_defined */

namespace {

const char computed_field_not_type_string[] = "not";

class Computed_field_not : public Computed_field_core
{
public:
	Computed_field_not() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_not();
	}

	const char *get_type_string()
	{
		return (computed_field_not_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		return (0 != dynamic_cast<Computed_field_not*>(other_field));
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);
};

int Computed_field_not::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		for (int i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] = (0.0 != sourceCache->values[i]) ? 0.0 : 1.0;
		}
		return 1;
	}
	return 0;
}

} //namespace

Cmiss_field_id Cmiss_field_module_create_not(Cmiss_field_module_id field_module,
	Cmiss_field_id source_field)
{
	Cmiss_field_id field = 0;
	if (source_field && source_field->isNumerical())
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_not());
	}
	return field;
}

