/*******************************************************************************
FILE : example_path.c

LAST MODIFIED : 8 October 2002

DESCRIPTION :
==============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include "general/debug.h"
#include "user_interface/message.h"
#include "command/example_path.h"

/*
Global functions
----------------
*/

char *resolve_example_path(char *example_path, char *directory_name,
	char **comfile_name, char **requirements)
/*******************************************************************************
LAST MODIFIED : 17 April 2000

DESCRIPTION :
Uses the executable $example_path/common/resolve_example_path to demangle
a short example name into a full path.  The returned string is ALLOCATED.
Code is basically a repeat from general/child_process but don't want to create
an object and want to encapsulate the whole process in one file for calling
from the back end.
<*comfile_name> is allocated and returned as well if the resolve function
returns a string for it.  This too must be DEALLOCATED by the calling function.
<*requirements> is either set to NULL or is an allocated string specifying 
the features required to run this example, i.e. whether the example 
needs cmgui and/or cm. The requirements are comma separated.  This too must 
be DEALLOCATED by the calling function.
==============================================================================*/
{
	char *return_string;
#if defined (UNIX)
#define BLOCKSIZE (100)
	char end[3] = {04, 10, 00}, *error_message, *filename, last_char, *new_string,
		*comfile_offset, *end_offset, *requirements_offset;
	fd_set readfds;
	int flags, stdin_filedes[2], stdout_filedes[2], 
		timeout;
	pid_t process_id;
	size_t index, string_size;
	ssize_t number_read;
	struct stat buf;
	struct timeval timeout_struct;
#endif /* defined (UNIX) */

	ENTER(resolve_example_path);

	if (example_path && directory_name)
	{
#if defined (UNIX)
	   timeout = 20; /* seconds */

		if (ALLOCATE(filename, char, strlen(example_path) + 
			strlen(directory_name) +50))
		{
			sprintf(filename, "%s/common/resolve_example_path", example_path);

			if (-1 == stat(filename,&buf))
			  {
			    if (!ALLOCATE(error_message, char, strlen(filename) + strlen("resolve_example_path. File %s does not exist")))
			      {
				display_message(ERROR_MESSAGE,"resolve_example_path. Unable to allocate string");
			      }
			    else
			      {
				sprintf(error_message, "resolve_example_path. File %s does not exist", filename);
				display_message(ERROR_MESSAGE,error_message);
				DEALLOCATE(error_message);
			      }
			    return_string = (char *)NULL;
			  }
 
			else if ((!pipe(stdin_filedes)) && (!pipe(stdout_filedes)))
			{
				if (-1 != (process_id = fork()))
				{
					if (process_id)
					{
						/* Parent process comes here */
						write(stdin_filedes[1], directory_name, strlen(directory_name) + 1);

						FD_ZERO(&readfds);
						FD_SET(stdout_filedes[0], &readfds);
						string_size = 2 * BLOCKSIZE;
						last_char = 0xff;
						index = 0;
						if (return_string = ALLOCATE(return_string, char,
							2 * BLOCKSIZE))
						{
							while((last_char != 0) && return_string)
							{
								if (index + BLOCKSIZE > string_size)
								{
									if(REALLOCATE(new_string, return_string, char,
										index + 2 * BLOCKSIZE))
									{
										return_string = new_string;
										string_size = index + 2 * BLOCKSIZE;
									}
									else
									{
										display_message(ERROR_MESSAGE,"resolve_example_path."
											"  Unable to reallocate string");
										DEALLOCATE(return_string);
										return_string = (char *)NULL;
									}
								}
								if (return_string)
								{
									flags = fcntl (stdout_filedes[0], F_GETFL);
									/*???DB.  Wouldn't compile at home.  O_NDELAY is equivalent to
									  FNDELAY */
									/*					flags &= FNDELAY;*/
									flags &= O_NDELAY;
									fcntl (stdout_filedes[0], F_SETFL, flags);
									timeout_struct.tv_sec = timeout;
									timeout_struct.tv_usec = 10000;
									if (select(FD_SETSIZE, &readfds, NULL, NULL, &timeout_struct))
									{
										number_read = read(stdout_filedes[0],
											(void *)(return_string + index), BLOCKSIZE);
										while((last_char != 0) && number_read)
										{
											last_char = *(return_string + index);
											number_read--;
											index++;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,"resolve_example_path."
											"  Timed out waiting for response from child");
										DEALLOCATE(return_string);
										return_string = (char *)NULL;
									}
								}
							}
						}
						else
						{
							DEALLOCATE(return_string);
							display_message(ERROR_MESSAGE,"resolve_example_path."
								"  Unable to allocate string");
							return_string = (char *)NULL;
						}
						write(stdin_filedes[1], end, 3);
						close(stdin_filedes[1]);
						close(stdout_filedes[0]);

						if (return_string)
						{
						   comfile_offset = (char *)NULL;
						   requirements_offset = (char *)NULL;
						   /* Look for the first space separator in the 
							  returned string */
						   if (comfile_offset = strchr(return_string, ' '))
						   {
							  /* Terminate the example path string */
							  *comfile_offset = 0;
							  comfile_offset++;

							  /* Look for the next space */
							  if (requirements_offset = strchr(comfile_offset, ' '))
							  {
								 /* Terminate the comfile string */
								 *requirements_offset = 0;
								 requirements_offset++;

								 /* Look for the end of this word */
								 if (end_offset = strchr(requirements_offset, ' '))
								 {
									/* Terminate the requirements string */
									*end_offset = 0;
								 }
							  }
						   }
						   if (comfile_name)
						   {
							  if (comfile_offset && ALLOCATE(*comfile_name, char,
								 strlen (comfile_offset) + 1))
							  {
								 strcpy (*comfile_name, comfile_offset);
							  }
							  else
							  {
								 *comfile_name = (char *)NULL;
							  }
						   }
						   if (requirements)
						   {
							  if (requirements_offset && ALLOCATE(*requirements, 
								 char, strlen (requirements_offset) + 1))
							  {
								 strcpy (*requirements, requirements_offset);
							  }
							  else
							  {
								 *requirements = (char *)NULL;
							  }
						   }
						   if (ALLOCATE(new_string, char,
							  strlen(return_string) + strlen(example_path) + 5))
						   {
							  sprintf(new_string, "%s/%s/", example_path,
								 return_string);
							  DEALLOCATE(return_string);
							  return_string = new_string;
						   }
						   else
						   {
							  DEALLOCATE(return_string);
							  display_message(ERROR_MESSAGE,"resolve_example_path."
								 "  Unable to make final reallocate of string");
							  return_string = (char *)NULL;
						   }
						}
					}
					else
					{
						/* Child process comes here */
						/* Remap the stdin and stdout */
						dup2(stdin_filedes[0],STDIN_FILENO);
						dup2(stdout_filedes[1],STDOUT_FILENO);
						/* Execute the filename */
						execlp(filename, filename, directory_name, (char *)0);
						/* The execlp should never return as the process gets overlayed */
						display_message(ERROR_MESSAGE,"CREATE(Child_process). Exec error!");
						exit(EXIT_FAILURE);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"resolve_example_path. Unable to fork child process");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"resolve_example_path. Unable to create pipes");
			}
			DEALLOCATE(filename);
		}
		else
		{
			display_message(ERROR_MESSAGE,"resolve_example_path. Unable to allocate program string");
			return_string = (char *)NULL;
		}
#else /* defined (UNIX) */
		display_message(ERROR_MESSAGE,"resolve_example_path.  "
			"Not implemented yet.");
		return_string = (char *)NULL;		
#endif /* defined (UNIX) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"resolve_example_path.  Invalid argument(s).");
		return_string = (char *)NULL;
	}

	LEAVE;

	return (return_string);
} /* resolve_example_path */

void destroy_example_path(char **example_path, char**comfile_name)
/*******************************************************************************
CREATED : KAT 19 April 2000
LAST MODIFIED : 19 April 2000

DESCRIPTION :
Destroys the example path created by resolve_example_path.
==============================================================================*/
{
  ENTER(destroy_example_path);

  if(example_path && comfile_name)
    {
      if(*example_path)
	{
	  DEALLOCATE(*example_path);
	  *example_path = NULL;
	}
      else
	{
	  display_message(WARNING_MESSAGE,"destroy_example_path.  Example path does not exist");
	}
      if(*comfile_name)
	{
	  DEALLOCATE(*comfile_name);
	  *comfile_name = NULL;
	}
    }
  else
    {
      display_message(ERROR_MESSAGE,"destroy_example_path.  Invalid argument(s).");
    }

  LEAVE;
}

/* Local Variables:  */
/* tab-width: 4 */
/* End:  */
