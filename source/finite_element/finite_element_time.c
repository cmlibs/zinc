/*******************************************************************************
FILE : finite_element_time.c

LAST MODIFIED : 21 January 2002

DESCRIPTION :
Representing time in finite elements.
==============================================================================*/
#include <math.h>
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_time.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/list_private.h"
#include "general/manager_private.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct FE_time
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
==============================================================================*/
{
	struct MANAGER(FE_time_version) *fe_time_version_manager;

	int access_count;
}; /* struct FE_time */

struct FE_time_version
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
==============================================================================*/
{
	enum FE_time_version_type type;

	/* For FE_TIME_SERIES */
	int number_of_times;
	FE_value *times;

	/* A pointer to itself so that we can make the INDEX functions work with
		multiple parts of the object as the identifier */
	struct FE_time_version *self;

	int access_count;
}; /* struct FE_time_version */

FULL_DECLARE_INDEXED_LIST_TYPE(FE_time_version);

FULL_DECLARE_MANAGER_TYPE(FE_time_version);

/*
Module functions
----------------
*/
DECLARE_OBJECT_FUNCTIONS(FE_time)
 
static int compare_FE_time_version(struct FE_time_version *fe_time_version_1,
	struct FE_time_version *fe_time_version_2)
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Returns -1 if fe_time_version_1 < fe_time_version_2, 
0 if fe_time_version_1 = fe_time_version_2 and 1 if
fe_time_version_1 > fe_time_version_2.
==============================================================================*/
{
	int return_code;

	ENTER(compare_FE_time_version);
	if (fe_time_version_1 && fe_time_version_2)
	{
		if (fe_time_version_1->type == fe_time_version_2->type)
		{
			switch(fe_time_version_1->type)
			{
				case FE_TIME_SERIES:
				{
					if (fe_time_version_1->number_of_times == 
						fe_time_version_2->number_of_times)
					{
						/* Using a char based memcmp for speed.  I think that
							it is OK to misrepresent these FE_values as chars
							as so long as we have a consistent order it is OK,
							if we wanted different representations of the same
							number to match we would have to compare FE_values
							instead. */
						return_code = memcmp(fe_time_version_1->times,
							fe_time_version_2->times, fe_time_version_1->number_of_times
							* sizeof(FE_value));
						if (return_code != 0)
						{
							/* Convert to -1 or 1 in case it is something else */
							if (return_code > 0)
							{
								return_code = 1;
							}
							else
							{
								return_code = -1;
							}
						}
					}
					else
					{
						if (fe_time_version_1->number_of_times > 
							fe_time_version_2->number_of_times)
						{
							return_code = 1;
						}
						else
						{
							return_code = -1;
						}
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"compare_FE_time_version.  Unimplemented FE_time_version type");
					return_code = 0;
				} break;
			}
		}
		else
		{
			if (fe_time_version_1->type > fe_time_version_2->type)
			{
				return_code = 1;
			}
			else
			{
				return_code = -1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"compare_FE_time_version.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* compare_FE_time_version */

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(FE_time_version,self,struct FE_time_version *,
	compare_FE_time_version)

DECLARE_LOCAL_MANAGER_FUNCTIONS(FE_time_version)

/*
Global functions
----------------
*/

struct FE_time *CREATE(FE_time)(void)
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Creates FE_time.
==============================================================================*/
{
	struct FE_time *fe_time;

	ENTER(CREATE(FE_time));

	if (ALLOCATE(fe_time,struct FE_time,1) && 
		(fe_time->fe_time_version_manager = CREATE(MANAGER(FE_time_version))()))
	{
		fe_time->access_count=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(FE_time).  Not enough memory");
		DEALLOCATE(fe_time);
	}
	LEAVE;

	return (fe_time);
} /* CREATE(FE_time) */

int DESTROY(FE_time)(struct FE_time **fe_time_address)
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Frees memory/deaccess objects in FE_time at <*fe_time_address>.
==============================================================================*/
{
	int return_code;
	struct FE_time *fe_time;

	ENTER(DESTROY(FE_time));
	if (fe_time_address&&(fe_time= *fe_time_address))
	{
		if (0 >= fe_time->access_count)
		{
			DESTROY(MANAGER(FE_time_version))(&(fe_time->fe_time_version_manager));
			DEALLOCATE(*fe_time_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(FE_time).  Positive access_count");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(FE_time).  Missing fe_time");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_time) */

struct FE_time_version *CREATE(FE_time_version)(void)
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Creates a basic FE_time_version.
==============================================================================*/
{
	struct FE_time_version *fe_time_version;

	ENTER(CREATE(FE_time_version));
	if (ALLOCATE(fe_time_version,struct FE_time_version,1))
	{
		/* initialise all members of computed_fe_time_version */
		fe_time_version->type = FE_TIME_SERIES;
		fe_time_version->number_of_times = 0;
		fe_time_version->times = (FE_value *)NULL;

		fe_time_version->self = fe_time_version;

		fe_time_version->access_count=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(FE_time_version).  Not enough memory");
		DEALLOCATE(fe_time_version);
	}
	LEAVE;

	return (fe_time_version);
} /* CREATE(FE_time_version) */

int DESTROY(FE_time_version)(struct FE_time_version **fe_time_version_address)
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Frees memory/deaccess objects in fe_time_version at <*fe_time_version_address>.
==============================================================================*/
{
	int return_code;
	struct FE_time_version *fe_time_version;

	ENTER(DESTROY(FE_time_version));
	if (fe_time_version_address&&(fe_time_version= *fe_time_version_address))
	{
		if (0 >= fe_time_version->access_count)
		{
			if (fe_time_version->times)
			{
				DEALLOCATE(fe_time_version->times);
			}
			DEALLOCATE(*fe_time_version_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(FE_time_version).  Positive access_count");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(FE_time_version).  Missing fe_time_version");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_time_version) */

DECLARE_OBJECT_FUNCTIONS(FE_time_version)

DECLARE_INDEXED_LIST_FUNCTIONS(FE_time_version)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(FE_time_version,self,
	struct FE_time_version *, compare_FE_time_version)

DECLARE_INDEXED_LIST_IDENTIFIER_CHANGE_FUNCTIONS(FE_time_version,self)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(FE_time_version,self)
{
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(FE_time_version,self));
	USE_PARAMETER(destination);
	USE_PARAMETER(source);
	display_message(ERROR_MESSAGE,
		"MANAGER_COPY_WITH_IDENTIFIER(FE_time_version,self).  "
		"You cannot do this on an FE_time_version, make a new one");
	return_code=0;
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(FE_time_version,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(FE_time_version,self)
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Do not allow copy if:
- it creates a self-referencing fe_time_version (one that depends on itself) which will
  result in an infinite loop;
- it changes the number of components of a fe_time_version in use;
==============================================================================*/
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(FE_time_version,self));
	USE_PARAMETER(destination);
	USE_PARAMETER(source);
	display_message(ERROR_MESSAGE,
		"MANAGER_COPY_WITHOUT_IDENTIFIER(FE_time_version,self).  "
		"You cannot do this on an FE_time_version, make a new one");
	return_code=0;
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(FE_time_version,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(FE_time_version,self,struct FE_time_version *)
{
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(FE_time_version,self));
	USE_PARAMETER(destination);
	USE_PARAMETER(self);
	display_message(ERROR_MESSAGE,
		"MANAGER_COPY_IDENTIFIER(FE_time_version,self).  "
		"You cannot do this on an FE_time_version, make a new one");
	return_code=0;
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(FE_time_version,name) */

DECLARE_MANAGER_FUNCTIONS(FE_time_version)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(FE_time_version)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(FE_time_version,self,struct FE_time_version *)

int FE_time_version_get_number_of_times(
	struct FE_time_version *fe_time_version)
/*******************************************************************************
LAST MODIFIED : 15 November 2001

DESCRIPTION :
Returns the number of times that a particular FE_time_version references to.
==============================================================================*/
{
	int number_of_times;

	ENTER(FE_time_version_get_number_of_times);

	if (fe_time_version)
	{
		number_of_times = fe_time_version->number_of_times;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_time_version_get_number_of_times.  Invalid arguments");
		number_of_times = 0;
	}
	LEAVE;
	
	return (number_of_times);
} /* FE_time_version_get_number_of_times */

int FE_time_version_get_index_for_time(
	struct FE_time_version *fe_time_version, FE_value time, int *time_index)
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
Returns the integer <time_index> into the time array contained in this version 
that corresponds to the <time>.  Returns 0 if that exact time is not found
and 1 if it is.
==============================================================================*/
{
	int return_code, time_index_one, time_index_two;
	FE_value xi;
	
	ENTER(FE_time_version_get_index_for_time);
	
	if (fe_time_version)
	{
		return_code = 0;
		if (FE_time_version_get_interpolation_for_time(fe_time_version, 
			time, &time_index_one, &time_index_two, &xi))
		{
			if (0 == xi)
			{
				*time_index = time_index_one;
				return_code = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_time_version_get_index_for_time.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_time_version_get_index_for_time */

int FE_time_version_get_interpolation_for_time(
	struct FE_time_version *fe_time_version, FE_value time, int *time_index_one,
	int *time_index_two, FE_value *xi)
/*******************************************************************************
LAST MODIFIED : 20 November 2001

DESCRIPTION :
Returns the two integers <time_index_one> and <time_index_two> which index into
the time array bracketing the supplied <time>, the <xi> value is set between 0
and 1 to indicate what fraction of the way between <time_index_one> and 
<time_index_two> the value is found.  Returns 0 if time is outside the range
of the time index array.
==============================================================================*/
{
	int array_index,done,index_high,index_low,number_of_times,return_code,step;	
	FE_value first_time,last_time,this_time,fe_value_index,time_high,time_low;
	
	ENTER(FE_time_version_get_index_for_time);
	
	if (fe_time_version)
	{
		return_code = 0;
		number_of_times = fe_time_version->number_of_times;

		first_time = fe_time_version->times[0];
		last_time = fe_time_version->times[number_of_times-1];
		/*Initial est. of the array index, assuming times evenly spaced, no gaps */	
		/*This assumption and hence estimate is true for most signal files. */
		if (last_time>first_time)
		{
			fe_value_index=((time-first_time)/(last_time-first_time))*(number_of_times-1);
			fe_value_index+=0.5;/*round float to nearest int */
			array_index=floor(fe_value_index);
		}
		else
		{
			array_index = 0;
		}
		time_low=0;
		time_high=0;
		done=0;
		index_low=0;
		index_high=number_of_times-1;
		/* do binary search for <time>'s array index. Also look at time of */
		/* adjacent array element, as index estimate may be slightly off due to*/
		/* rounding error. This avoids unnecessarily long search from end of array */
		while(!done)
		{	
			if ((array_index >= 0) && (array_index < number_of_times))
			{
				this_time = fe_time_version->times[array_index];
				if (this_time>time)
				{ 
					index_high=array_index;					
					if (array_index>0)
					{
						/* get adjacent array element*/
						time_low = fe_time_version->times[array_index - 1];
						/* are we between elements?*/
						if (time_low<time)
						{			
							index_low=array_index-1;
							return_code=1;
							done=1;
						}	
						else
						{
							time_low=0;
						}
					}
					else
					{
						/* can't get lower adjacent array element when array_index=0. Finished*/
						time_low = fe_time_version->times[array_index];
						index_low=array_index;
						return_code=1;
						done=1;
					}
				}
				else if (this_time<time)
				{
					index_low=array_index;
					if (array_index<(number_of_times-1))
					{
						/* get adjacent array element*/
						time_high = fe_time_version->times[array_index + 1];
						/* are we between elements?*/
						if (time_high>time)
						{		
							index_high=array_index+1;
							return_code=1;
							done=1;
						}	
						else
						{
							time_high=0;
						}
					}
					else
					{
						/* can't get higher adjacent array element when */
						/*array_index=(number_of_times-1). Finished*/
						time_high = fe_time_version->times[array_index];
						index_high=array_index;
						return_code=1;
						done=1;
					}
				}
				else /* (this_time == time) */
				{
					index_low=array_index;
					index_high=array_index;
					time_high=this_time;
					time_low=this_time;
					return_code=1;
					done=1;
				}
				if (!done)
				{	
					step=(index_high-index_low)/2;	
					/* No exact match, can't subdivide further, must do interpolation.*/
					if (step==0)												
					{	
						done=1;	
						return_code=1;														
					}
					else
					{
						array_index=index_low+step;
					}
						
				}/* if (!done)	*/
			}
			else
			{
				if (array_index < 0)
				{
					/* Before start of times, don't write an error as
						we want the calling routine to decide how to treat this */
					done = 1;
					index_low = 0;
					index_high = 0;
					return_code = 0;
				}
				else
				{
					/* After end of times, don't write an error as
						we want the calling routine to decide how to treat this */
					done = 1;
					index_low = number_of_times-1;
					index_high = number_of_times-1;
					return_code = 0;
				}
			}
		}	/* while(!done)	*/
		/* index_low and index_high should now be adjacent */
		if (!time_low)
		{
			time_low = fe_time_version->times[index_low];
		}
		if (!time_high)
		{
			time_high = fe_time_version->times[index_high];
		}
		*time_index_one = index_low;
		*time_index_two = index_high;
		if (time_high > time_low)
		{
			*xi = (time - time_low) / (time_high - time_low);
		}
		else
		{
			*xi = 0.0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_time_version_get_interpolation_for_time.  "
			"Invalid arguments time out of range");
		return_code=0;
	}	
	LEAVE;

	return (return_code);
} /* FE_time_version_get_interpolation_for_time */

int FE_time_version_get_time_for_index(
	struct FE_time_version *fe_time_version, int time_index, FE_value *time)
/*******************************************************************************
LAST MODIFIED : 15 November 2001

DESCRIPTION :
If the <time_index> is valid returns the corresponding <time>.
==============================================================================*/
{
	int return_code;
	
	ENTER(FE_time_version_get_index_for_time);
	
	if (fe_time_version)
	{
		if ((time_index >= 0) && (time_index < fe_time_version->number_of_times))
		{
			*time = fe_time_version->times[time_index];
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_time_version_get_time_for_index.  Time index out of range");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_time_version_get_time_for_index.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_time_version_get_index_for_time */

struct FE_time_version *get_FE_time_version_matching_time_series(
	struct FE_time *fe_time, int number_of_times, FE_value *times)
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Searches <fe_time> for a fe_time_version which has the time series specified. 
If no equivalent fe_time_version is found one is created and returned.
==============================================================================*/
{
	struct FE_time_version *fe_time_version, *local_fe_time_version;

	ENTER(get_FE_time_version_matching_time_series);
	fe_time_version=(struct FE_time_version *)NULL;
	if (fe_time&&fe_time->fe_time_version_manager&&number_of_times&&times)
	{
		/* Create a FE_time_version into which we will poke a reference to this 
			number of times and times array so that we can look for another one the
			same.  I really want to avoid copying the array unnecessarily but if 
			the CREATE(FE_time_version) routine was changed to allocate the times array
			by default then this would leak.*/
		local_fe_time_version = CREATE(FE_time_version)();
		local_fe_time_version->type = FE_TIME_SERIES;
		local_fe_time_version->number_of_times = number_of_times;
		local_fe_time_version->times = times;
		/* search the manager for a fe_time_version of that name */
		if (fe_time_version=
			FIND_BY_IDENTIFIER_IN_MANAGER(FE_time_version,self)(local_fe_time_version,
				fe_time->fe_time_version_manager))
		{
			/* Found so get rid of our local one. */
			local_fe_time_version->number_of_times = 0;
			local_fe_time_version->times = (FE_value *)NULL;
			DESTROY(FE_time_version)(&local_fe_time_version);
		}
		else
		{
			/* Finish establishing our local one correctly. */
			local_fe_time_version->times = (FE_value *)NULL;
			if (ALLOCATE(local_fe_time_version->times, FE_value, number_of_times))
			{
				memcpy(local_fe_time_version->times, times, number_of_times *
					sizeof(FE_value));
				if (ADD_OBJECT_TO_MANAGER(FE_time_version)(local_fe_time_version,
					fe_time->fe_time_version_manager))
				{
					fe_time_version = local_fe_time_version;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"get_FE_time_version_matching_time_series.  "
						"Unable to add object to manager");
					fe_time_version=(struct FE_time_version *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"get_FE_time_version_matching_time_series.  "
					"Unable to allocate memory");
				fe_time_version=(struct FE_time_version *)NULL;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_time_version_matching_time_series.  Invalid argument(s)");
		fe_time_version=(struct FE_time_version *)NULL;
	}
	LEAVE;

	return (fe_time_version);
} /* get_FE_time_version_matching_time_series */

struct FE_time_version *get_FE_time_version_merging_two_time_series(
	struct FE_time *fe_time, struct FE_time_version *time_version_one,
	struct FE_time_version *time_version_two)
/*******************************************************************************
LAST MODIFIED : 20 November 2001

DESCRIPTION :
Searches <fe_time> for a fe_time_version which has the list of times formed
by merging the two time_versions supplied.
==============================================================================*/
{
	int maximum_number_of_times, number_of_times;
	FE_value *end_one, *end_two, *index_one, *index_two, *merged_index, *times;
	struct FE_time_version *fe_time_version;

	ENTER(get_FE_time_version_matching_time_series);
	fe_time_version=(struct FE_time_version *)NULL;
	if (fe_time&&fe_time->fe_time_version_manager&&time_version_one&&
		time_version_two)
	{
		if (time_version_one == time_version_two)
		{
			fe_time_version = time_version_one;
		}
		else
		{
			maximum_number_of_times = time_version_one->number_of_times + 
				time_version_two->number_of_times;
			if (ALLOCATE(times, FE_value, maximum_number_of_times))
			{
				number_of_times = 0;
				merged_index = times;
				index_one = time_version_one->times;
				index_two = time_version_two->times;
				end_one = time_version_one->times + time_version_one->number_of_times;
				end_two = time_version_two->times + time_version_two->number_of_times;

				while ((index_one < end_one) && (index_two < end_two))
				{
					if (*index_one < *index_two)
					{
						*merged_index = *index_one;
						index_one++;
						number_of_times++;
						merged_index++;
					}
					else if (*index_one > *index_two)
					{
						*merged_index = *index_two;
						index_two++;
						number_of_times++;
						merged_index++;
					}
					else
					{
						*merged_index = *index_one;
						index_one++;
						index_two++;
						number_of_times++;
						merged_index++;
					}
				}
				if (index_one < end_one)
				{
					/* Copy over the end of array one */
					while(index_one < end_one)
					{
						*merged_index = *index_one;
						index_one++;
						number_of_times++;
						merged_index++;
					}
				}
				else if (index_two < end_two)
				{
					/* Copy over the end of array two */
					while(index_two < end_two)
					{
						*merged_index = *index_two;
						index_two++;
						number_of_times++;
						merged_index++;
					}
				}

				fe_time_version = get_FE_time_version_matching_time_series(
					fe_time, number_of_times, times);

				DEALLOCATE(times);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"get_FE_time_version_matching_time_series.  "
					"Could not ALLOCATE temporary time array.");
				fe_time_version=(struct FE_time_version *)NULL;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_time_version_matching_time_series.  Invalid argument(s)");
		fe_time_version=(struct FE_time_version *)NULL;
	}
	LEAVE;

	return (fe_time_version);
} /* get_FE_time_version_matching_time_series */

