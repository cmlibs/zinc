/*******************************************************************************
FILE : debug.c

LAST MODIFIED : 26 November 2001

DESCRIPTION :
Function definitions for debugging.
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user_interface/message.h"
#include "general/debug.h"

#if defined (MEMORY_CHECKING)
/* Must override all these Macros before we use them to ensure we don't get
	stuck in an infinite loop of allocating */
#define ALLOCATE( result , type , number ) \
( result = ( type *) malloc( ( number ) * sizeof( type ) ))

#define DEALLOCATE( ptr ) \
{ free((char *) ptr ); ( ptr )=NULL;}

#define ENTER( function_name )

#define LEAVE

#define REALLOCATE( final , initial , type , number ) \
( final = ( type *) realloc( (char *)( initial ) , \
	( number ) * sizeof( type )))
#include "general/compare.h"
#include "general/indexed_list_private.h"
#endif

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
	char *filename_line, *type;
	int count,size;
	void *ptr;
	int access_count;
}; /* struct Memory_block */
#endif /* defined (MEMORY_CHECKING) */

/*
Module functions
----------------
*/

#if defined (MEMORY_CHECKING)
static struct Memory_block *CREATE(Memory_block)(void *pointer,
	char *filename, int line, char *type_string, int size,
	int count)
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
==============================================================================*/
{
	struct Memory_block *block;
	
	if (ALLOCATE(block, struct Memory_block, 1))
	{
		block->ptr = pointer;
		block->size = size;
		block->count = count;
		block->access_count = 0;

		if (ALLOCATE(block->filename_line, char, strlen(filename)+20))
		{
			sprintf(block->filename_line,"%s : %10d",filename,line);
			if (ALLOCATE(block->type,char,strlen(type_string)+1))
			{
				strcpy(block->type, type_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Memory_block).  Unable to allocate memory for memory_block type string identifier");
				DEALLOCATE(block);
				DEALLOCATE(block->filename_line);
				block = (struct Memory_block *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Memory_block).  Unable to allocate memory for memory_block string identifier");
			DEALLOCATE(block);
			block = (struct Memory_block *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Memory_block).  Unable to allocate memory for memory_block list structure");
		block = (struct Memory_block *)NULL;
	}
	
	return(block);
}

int DESTROY(Memory_block)(struct Memory_block **block_address)
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Memory_block *block;

	if (block_address && (block = *block_address))
	{
		if (block->access_count <= 0)
		{
			if (block->filename_line)
			{
				DEALLOCATE(block->filename_line);
			}
			if (block->type)
			{
				DEALLOCATE(block->type);
			}
			DEALLOCATE(*block_address);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Memory_block).  Destroy called when access count > 0.");
			*block_address = (struct Memory_block *)NULL;
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Memory_block).  Invalid arguments.");
		return_code = 0;
	}

	return (return_code);
}

DECLARE_LIST_TYPES(Memory_block);

PROTOTYPE_OBJECT_FUNCTIONS(Memory_block);
PROTOTYPE_LIST_FUNCTIONS(Memory_block);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Memory_block,ptr,void *);

DECLARE_OBJECT_FUNCTIONS(Memory_block)
FULL_DECLARE_INDEXED_LIST_TYPE(Memory_block);
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Memory_block,ptr,void *,compare_pointer)
DECLARE_INDEXED_LIST_FUNCTIONS(Memory_block)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Memory_block,ptr,void *,
	compare_pointer)

#if defined (MEMORY_CHECKING)
static int maximum_count = 1;
static struct LIST(Memory_block) *memory_block_list =
   (struct LIST(Memory_block) *)NULL;
#endif /* defined (MEMORY_CHECKING) */

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
LAST MODIFIED : 26 November 2001

DESCRIPTION :
Definition of function which is called in the development stage (when
USE_PARAMETER_ON is defined) to swallow unused parameters to functions which
would otherwise cause compiler warnings. For example, parameter <dummy_void>
is swallowed with the call USE_PARAMETER(dummy_void); at the start of function.
==============================================================================*/
{
	if (dummy)
	{
	}
} /* use_parameter */
#endif /* defined (USE_PARAMETER_ON) */

#if !defined (OPTIMISED)

char *allocate(unsigned size,char *filename,int line, char *type)
/*******************************************************************************
LAST MODIFIED : 26 November 2001

DESCRIPTION :
Wrapper for allocate which keeps track of allocated memory.
==============================================================================*/
{
	char *result;
#if defined (MEMORY_CHECKING)
	struct Memory_block *new_block;
#endif /* defined (MEMORY_CHECKING) */

	ENTER(allocate);
#if !defined (MEMORY_CHECKING)
	USE_PARAMETER(filename);
	USE_PARAMETER(line);
	USE_PARAMETER(type);
#endif /* !defined (MEMORY_CHECKING) */
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
			if (!memory_block_list)
			{
				memory_block_list = CREATE(LIST(Memory_block))();
			}
			if (new_block=CREATE(Memory_block)((void *)result,
				filename, line, type, size, maximum_count))
			{
				if (!ADD_OBJECT_TO_LIST(Memory_block)(new_block,memory_block_list))
				{
					display_message(ERROR_MESSAGE,
						"allocate.  Unable to add Memory_block to list");
					DESTROY(Memory_block)(&new_block);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"allocate.  Unable to create Memory_block");
			}
		}
#endif /* defined (MEMORY_CHECKING) */
	}
	else
	{
		display_message(WARNING_MESSAGE,"allocate.  Zero size");
#if defined (MEMORY_CHECKING)
		display_message(WARNING_MESSAGE,"%s : %10d",filename,line);
#endif
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
	struct Memory_block *block;
#endif /* defined (MEMORY_CHECKING) */

	ENTER(deallocate);
#if !defined (MEMORY_CHECKING)
	USE_PARAMETER(filename);
	USE_PARAMETER(line);
#endif /* !defined (MEMORY_CHECKING) */
	if (ptr)
	{
#if defined (MEMORY_CHECKING)
		if (memory_block_list && (block = 
			FIND_BY_IDENTIFIER_IN_LIST(Memory_block,ptr)(ptr, memory_block_list)))
		{
			REMOVE_OBJECT_FROM_LIST(Memory_block)(block, memory_block_list);
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

char *reallocate(char *ptr,unsigned size,char *filename,int line, char *type)
/*******************************************************************************
LAST MODIFIED : 25 July 1998

DESCRIPTION :
Wrapper for reallocate which keeps track of allocated memory.
==============================================================================*/
{
	char *result;
#if defined (MEMORY_CHECKING)
	struct Memory_block *block,*new_block;
#endif /* defined (MEMORY_CHECKING) */

	ENTER(reallocate);
#if !defined (MEMORY_CHECKING)
	USE_PARAMETER(filename);
	USE_PARAMETER(line);
	USE_PARAMETER(type);
#endif /* !defined (MEMORY_CHECKING) */
	if (0<size)
	{
#if defined (MEMORY_CHECKING)
		if (!memory_block_list)
		{
			memory_block_list = CREATE(LIST(Memory_block))();
		}
		if (ptr)
		{
			if (!(block = FIND_BY_IDENTIFIER_IN_LIST(Memory_block,ptr)(ptr, memory_block_list)))
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
			block=(struct Memory_block *)NULL;
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
			if (block)
			{
				if (strcmp(type, block->type))
				{
					display_message(ERROR_MESSAGE,
						"reallocate.  Allocation types don't match %s realloced at %s : %10d",
						block->filename_line, filename, line);
				}
				REMOVE_OBJECT_FROM_LIST(Memory_block)(block, memory_block_list);
			}
			if (new_block=CREATE(Memory_block)((void *)result,
				filename, line, type, size, maximum_count))
			{
				if (!ADD_OBJECT_TO_LIST(Memory_block)(new_block,memory_block_list))
				{
					display_message(ERROR_MESSAGE,
						"reallocate.  Unable to add Memory_block to list");
					DESTROY(Memory_block)(&new_block);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"reallocate.  Unable to create Memory_block");
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

#endif /* !defined (OPTIMISED) */

#if defined (MEMORY_CHECKING)
struct List_memory_data
{
	int count;
	int show_pointers;
	int show_structures;
	int total;
	int *count_total;
};

static int list_memory_block(struct Memory_block *block, void *list_memory_data_void)
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct List_memory_data *list_memory_data;

	ENTER(list_memory_block);
	if (block && (list_memory_data = (struct List_memory_data *)list_memory_data_void))
	{
		if (list_memory_data->show_pointers)
		{
			if (!list_memory_data->count||(block->count==list_memory_data->count))
			{
				printf("%s @ %x size %d type %s\n",block->filename_line,block->ptr,
					block->size, block->type);
			}
			list_memory_data->count_total[block->count] += block->size;
			list_memory_data->total += block->size;
		}
		else
		{
			if (!list_memory_data->count||(block->count==list_memory_data->count))
			{
				printf("%s size %d type %s\n",block->filename_line,block->size,block->type);
			}
			list_memory_data->count_total[block->count] += block->size;
			list_memory_data->total += block->size;
		}
#if defined (DEBUG)
		/* This code, while useful for debugging, causes this file to be dependent
			on parts of cmgui it shouldn't. */
		if (list_memory_data->show_structures)
		{
			if (!strcmp(block->type, "struct FE_field"))
			{
				list_FE_field(block->ptr, NULL);
			}
			else if (!strcmp(block->type, "struct FE_element"))
			{
				list_FE_element(block->ptr, NULL);
			}
		}
#endif /* defined (DEBUG) */
		return_code = 1;
	}
	else
	{
		printf("Unable to allocate memory total array\n");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_memory_block */
#endif /* defined (MEMORY_CHECKING) */

int list_memory(int count,int show_pointers,int increment_counter,
	int show_structures)
/*******************************************************************************
LAST MODIFIED : 26 November 2001

DESCRIPTION :
Writes out memory blocks currently allocated.  Each time this is called an
internal counter is incremented and all subsequent ALLOCATIONS marked with this
new <count>.  i.e. To find a leak run till before the leak (all these
allocations will be marked count 1), call list_memory increment, do the leaky
thing several times (these have count 2), call list_memory increment, do the
leaky thing once (so that any old stuff with count 2 should have been
deallocated and recreated with count 3) and then list_memory 2.  This should
list no memory.
If <count> is zero all the memory allocated is written out.
If <count> is negative no memory is written out, just the total.
If <count> is positive only the memory with that count is written out.
<show_pointers> toggles the output format to include the actual memory addresses
or not.  (It isn't useful for testing and output to record the changing
addresses).
If <show_structures> is set then for known types the objects are cast to the
actual object type and then the appropriate list function is called.
???DB.  printf used because want to make sure that no allocation is going on
	while printing.
==============================================================================*/
{
	int return_code;
#if defined (MEMORY_CHECKING)
	int i;
	struct List_memory_data list_memory_data;
#endif /* defined (MEMORY_CHECKING) */

	ENTER(list_memory);
#if defined (MEMORY_CHECKING)
	if (ALLOCATE(list_memory_data.count_total, int, maximum_count + 1))
	{
		for (i = 0 ; i < (maximum_count + 1) ; i++)
		{
			list_memory_data.count_total[i] = 0;
		}
		list_memory_data.count = count;
		list_memory_data.show_pointers = show_pointers;
		list_memory_data.show_structures = show_structures;
		list_memory_data.total = 0;

		if (memory_block_list)
		{
			FOR_EACH_OBJECT_IN_LIST(Memory_block)(list_memory_block, 
				(void *)&list_memory_data, memory_block_list);
		}

		printf("\n\n\n\n\n");
		for (i = 1 ; i < maximum_count + 1 ; i++)
		{
			printf("Allocated memory with count %3d: %12d\n", i, 
				list_memory_data.count_total[i]);
		}
		DEALLOCATE(list_memory_data.count_total);
		printf("Total allocated memory:          %12d\n", list_memory_data.total);
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
#else /* defined (MEMORY_CHECKING) */
	USE_PARAMETER(count);
	USE_PARAMETER(show_pointers);
	USE_PARAMETER(increment_counter);
	USE_PARAMETER(show_structures);
	return_code = 0;
#endif /* defined (MEMORY_CHECKING) */
	LEAVE;

	return (return_code);
} /* list_memory */
