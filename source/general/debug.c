/*******************************************************************************
FILE : debug.c

LAST MODIFIED : 10 June 1999

DESCRIPTION :
Function definitions for debugging.
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "general/debug.h"
#include "user_interface/message.h"

/* controls whether or not memory checking is included */
/* #define MEMORY_CHECKING */

/*
Module types
------------
*/
#if defined (MEMORY_CHECKING)
struct Memory_block
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
Keeps a record of where a block of memory was allocated
==============================================================================*/
{
	char *filename_line;
	int count,size;
	struct Memory_block *next;
	void *ptr;
}; /* struct Memory_block */
#endif /* defined (MEMORY_CHECKING) */

/*
Module variables
----------------
*/
#if defined (MEMORY_CHECKING)
static int maximum_count = 1;
static struct Memory_block *memory_list=NULL;
#endif /* defined (MEMORY_CHECKING) */

/*
Global variables
----------------
*/
/* temporary storage string */
char global_temp_string[GLOBAL_TEMP_STRING_SIZE];

/*
Global functions
----------------
*/
#if defined (USE_PARAMETER_ON)
void use_parameter(int dummy, ... )
/*******************************************************************************
LAST MODIFIED : 10 June 1999

DESCRIPTION :
Definition of function which is called in the development stage (when
USE_PARAMETER_ON is defined) to swallow unused parameters to functions which
would otherwise cause compiler warnings. For example, parameter <dummy_void>
is swallowed with the call USE_PARAMETER(dummy_void); at the start of function.
==============================================================================*/
{
} /* use_parameter */
#endif /* defined (USE_PARAMETER_ON) */

char *allocate(unsigned size,char *filename,int line)
/*******************************************************************************
LAST MODIFIED : 25 July 1998

DESCRIPTION :
Wrapper for allocate which keeps track of allocated memory.
==============================================================================*/
{
	char *result;
#if defined (MEMORY_CHECKING)
	char *filename_line;
	struct Memory_block *new_block;
#endif /* defined (MEMORY_CHECKING) */

	ENTER(allocate);
	if (0<size)
	{
		result=malloc(size);
		if (!result)
		{
			display_message(ERROR_MESSAGE,"allocate.  Insufficient memory.  Size=%d",
				size);
		}
#if defined (MEMORY_CHECKING)
		else
		{
			if (filename_line=(char *)malloc(sizeof(char)*(strlen(filename)+20)))
			{
				sprintf(filename_line,"%s : %10d",filename,line);
				if (new_block=
					(struct Memory_block *)malloc(sizeof(struct Memory_block)))
				{
					new_block->ptr=(void *)result;
					new_block->filename_line=filename_line;
					new_block->size=size;
					new_block->count=maximum_count;
					new_block->next=memory_list;
					memory_list=new_block;
				}
				else
				{
					display_message(ERROR_MESSAGE,
				"allocate.  Unable to allocate memory for memory_block list structure");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
		"allocate.  Unable to allocate memory for memory_block string identifier");
			}
		}
#endif /* defined (MEMORY_CHECKING) */
	}
	else
	{
		display_message(WARNING_MESSAGE,"allocate.  Zero size");
		result=(char *)NULL;
	}
	LEAVE;

	return (result);
} /* allocate */

void deallocate(char *ptr,char *filename,int line)
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
Wrapper for deallocate which keeps track of allocated memory.
==============================================================================*/
{
#if defined (MEMORY_CHECKING)
	struct Memory_block *list,*previous;
#endif /* defined (MEMORY_CHECKING) */

	ENTER(deallocate);
	if (ptr)
	{
#if defined (MEMORY_CHECKING)
		list=memory_list;
		previous=(struct Memory_block *)NULL;
		while (list&&(list->ptr!=(void *)ptr))
		{
			previous=list;
			list=list->next;
		}
		if (list)
		{
			if (previous)
			{
				previous->next=list->next;
			}
			else
			{
				memory_list=list->next;
			}
			free(list->filename_line);
			free((char *)list);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"deallocate.  Could not find ptr %x in memory block list",ptr);
			display_message(ERROR_MESSAGE,
				"deallocate.  called from %s at line %d",filename,line);
			printf("deallocate.  Could not find ptr %x in memory block list\n",ptr);
			printf("deallocate.  called from %s at line %d\n",filename,line);
		}
#endif /* defined (MEMORY_CHECKING) */
		free(ptr);
	}
	LEAVE;
} /* deallocate */

char *reallocate(char *ptr,unsigned size,char *filename,int line)
/*******************************************************************************
LAST MODIFIED : 25 July 1998

DESCRIPTION :
Wrapper for reallocate which keeps track of allocated memory.
==============================================================================*/
{
	char *result;
#if defined (MEMORY_CHECKING)
	char *filename_line;
	struct Memory_block *list,*new_block;
#endif /* defined (MEMORY_CHECKING) */

	ENTER(reallocate);
	if (0<size)
	{
#if defined (MEMORY_CHECKING)
		if (ptr)
		{
			list=memory_list;
			while (list&&(list->ptr!=(void *)ptr))
			{
				list=list->next;
			}
			if (!list)
			{
				display_message(ERROR_MESSAGE,
					"reallocate.  Could not find ptr %x in memory block list", ptr);
				display_message(ERROR_MESSAGE,
					"reallocate.  called from %s at line %d.", filename, line);
				printf("reallocate.  Could not find ptr %x in memory block list\n", ptr);
				printf("reallocate.  called from %s at line %d.\n", filename, line);
			}
		}
		else
		{
			list=(struct Memory_block *)NULL;
		}
#endif /* defined (MEMORY_CHECKING) */
		result=realloc(ptr,size);
		if (!result)
		{
			display_message(ERROR_MESSAGE,
				"reallocate.  Insufficient memory.  Size=%d",size);
		}
#if defined (MEMORY_CHECKING)
		else
		{
			if (list)
			{
				if (filename_line=(char *)realloc(list->filename_line,
					sizeof(char)*(strlen(filename)+20)))
				{
					sprintf(filename_line,"%s : %10d",filename,line);
					list->ptr=(void *)result;
					list->filename_line=filename_line;
					list->size=size;
				}
				else
				{
					display_message(ERROR_MESSAGE,
"reallocate.  Unable to reallocate memory for memory_block string identifier");
				}
			}
			else
			{
				if (filename_line=(char *)malloc(sizeof(char)*(strlen(filename)+20)))
				{
					sprintf(filename_line,"%s : %10d",filename,line);
					if (new_block=
						(struct Memory_block *)malloc(sizeof(struct Memory_block)))
					{
						new_block->ptr=(void *)result;
						new_block->filename_line=filename_line;
						new_block->size=size;
						new_block->count=1;
						new_block->next=memory_list;
						memory_list=new_block;
					}
					else
					{
						display_message(ERROR_MESSAGE,
			"reallocate.  Unable to allocate memory for memory_block list structure");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
	"reallocate.  Unable to allocate memory for memory_block string identifier");
				}
			}
		}
#endif /* defined (MEMORY_CHECKING) */
	}
	else
	{
		display_message(WARNING_MESSAGE,"reallocate.  Zero size requested ");
#if defined (MEMORY_CHECKING)
		display_message(WARNING_MESSAGE,"%s : %10d",filename,line);
#endif
		result=ptr;
	}
	LEAVE;

	return (result);
} /* reallocate */

int list_memory(int count, int show_pointers, int increment_counter)
/*******************************************************************************
LAST MODIFIED : 20 October 1999

DESCRIPTION :
Writes out memory blocks currently allocated.  Each time this is called an internal
counter is incremented and all subsequent ALLOCATIONS marked with this new
count_number.  i.e. To find a leak run till before the leak (all these allocations
will be marked count 1), call list_memory increment, 
do the leaky thing several times (these have count 2), call list_memory increment, 
do the leaky thing once (so that any old stuff with count 2 should have been deallocated 
and recreated with count 3) and then list_memory 2.  This should list no memory.
If <count_number> is zero all the memory allocated is written out.
If <count_number> is negative no memory is written out, just the total.
If <count_number> is positive only the memory with that count is written out.
???DB.  printf used because want to make sure that no allocation is going on
	while printing.
<show_pointers> toggles the output format to include the actual memory addresses
or not.  (It isn't useful for testing and output to record the changing addresses).
==============================================================================*/
{
	int return_code;
#if defined (MEMORY_CHECKING)
	int *count_total, i, total;
	struct Memory_block *list;
#endif /* defined (MEMORY_CHECKING) */

	ENTER(list_memory);
	return_code=0;
#if defined (MEMORY_CHECKING)
	total=0;
	list=memory_list;
	if (count_total = (int *)malloc((maximum_count + 1) * sizeof(int)))
	{
		for (i = 0 ; i < (maximum_count + 1) ; i++)
		{
			count_total[i] = 0;
		}
		if (show_pointers)
		{
			while (list)
			{
				if (!count||(list->count==count))
				{
					printf("%s @ %x size %d\n",list->filename_line,list->ptr,list->size);
				}
				count_total[list->count] += list->size;
				total += list->size;
				list=list->next;
			}
		}
		else
		{
			while (list)
			{
				if (!count||(list->count==count))
				{
					printf("%s size %d\n",list->filename_line,list->size);
					count_total += list->size;
				}
				count_total[list->count] += list->size;
				total += list->size;
				list=list->next;
			}
		}
		printf("\n\n\n\n\n");
		for (i = 1 ; i < maximum_count + 1 ; i++)
		{
			printf("Allocated memory with count %3d: %12d\n", i, count_total[i]);
		}
		free(count_total);
		printf("Total allocated memory:          %12d\n", total);
		if (increment_counter)
		{
			maximum_count++;
		}
		printf("\nNow allocating with count: %d\n", maximum_count);
		return_code = 1;
	}
	else
	{
		printf("Unable to allocate memory total array\n");
		return_code = 0;
	}
#endif /* defined (MEMORY_CHECKING) */
	LEAVE;

	return (return_code);
} /* list_memory */
