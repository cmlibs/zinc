/*******************************************************************************
FILE : photoface_cmiss.c

LAST MODIFIED : 16 February 2001

DESCRIPTION :
The functions that interface Photoface to cmiss.  All functions have an integer
return code - zero is success, non-zero is failure.
==============================================================================*/
#ifdef _AFXDLL
#include "stdafx.h"
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h> /*???DB.  Contains definition of __BYTE_ORDER for Linux */

#if defined (WIN32)
#define __BYTE_ORDER 1234
#include <windows.h>
#endif

#define CMISSDLLEXPORT
#include "photoface_cmiss.h"

/*
Macros
------
*/
#define ALLOCATE( result , type , number ) \
( result = ( type *) malloc( ( number ) * sizeof( type ) ) )

#define DEALLOCATE( ptr ) \
{ free((char *) ptr ); ( ptr )=NULL;}

#define ENTER( function_name )

#define LEAVE

#define REALLOCATE( final , initial , type , number ) \
( final = ( type *) realloc( (void *)( initial ) , \
	( number ) * sizeof( type ) ) )

/*
Module types
------------
*/
struct Marker_struct
{
	int number_of_markers;
	char **marker_names;
	int *marker_indices;
	float *marker_positions;
}; /* struct Marker_struct */

struct Obj
{ 
	int number_of_vertices;
	float *vertex_3d_locations;
	int number_of_texture_vertices;
	float *texture_vertex_3d_locations;
	int number_of_triangles;
	int *triangle_vertices;
	int *triangle_texture_vertices;
}; /* struct Obj */

/*
Module variables
----------------
*/
static struct Marker_struct *markers = NULL;
static char *photoface_linux_path = NULL,*photoface_windows_path = NULL;
static float ndc_texture_scaling = 0, ndc_texture_offset = 0;

#if defined (MANUAL_CMISS)
static PF_display_message_function
	*display_error_message_function=(PF_display_message_function *)NULL,
	*display_information_message_function=(PF_display_message_function *)NULL,
	*display_warning_message_function=(PF_display_message_function *)NULL;
static void
	*display_error_message_data=(void *)NULL,
	*display_information_message_data=(void *)NULL,
	*display_warning_message_data=(void *)NULL;

#define MESSAGE_STRING_SIZE 1000
static char message_string[MESSAGE_STRING_SIZE];
#else /* defined (MANUAL_CMISS) */
#define COMMAND_STRING_SIZE 1000
static char command_string[COMMAND_STRING_SIZE];
#endif /* defined (MANUAL_CMISS) */

/*
Module functions
----------------
*/
#if defined (MANUAL_CMISS)
static int display_message(enum PF_message_type message_type,char *format, ... )
/*******************************************************************************
LAST MODIFIED : 13 February 2001

DESCRIPTION :
A function for displaying a message of the specified <message_type>.  The printf
form of arguments is used.
==============================================================================*/
{
	int return_code;
	va_list ap;

	ENTER(display_message);
	va_start(ap,format);
/*	return_code=vsnprintf(message_string,MESSAGE_STRING_SIZE,format,ap);*/
	return_code=vsprintf(message_string,format,ap);
	if (return_code >= (MESSAGE_STRING_SIZE-1))
	{
		char error_string[100];
		sprintf(error_string,"Overflow of message_string.  "
			"Following is truncated to %d characters:",return_code);
		if (display_error_message_function)
		{
			return_code=(*display_error_message_function)(error_string,
				display_error_message_data);
		}
		else
		{
			return_code=printf("ERROR: %s\n",error_string);
		}
	}
	switch (message_type)
	{
		case PF_ERROR_MESSAGE:
		{
			if (display_error_message_function)
			{
				return_code=(*display_error_message_function)(message_string,
					display_error_message_data);
			}
			else
			{
				return_code=printf("ERROR: %s\n",message_string);
			}
		} break;
		case PF_INFORMATION_MESSAGE:
		{
			if (display_information_message_function)
			{
				return_code=(*display_information_message_function)(message_string,
					display_information_message_data);
			}
			else
			{
				/* make sure we don't interpret % characters by printing the string */
				return_code=printf("%s",message_string);
			}
		} break;
		case PF_WARNING_MESSAGE:
		{
			if (display_warning_message_function)
			{
				return_code=(*display_warning_message_function)(message_string,
					display_warning_message_data);
			}
			else
			{
				return_code=printf("WARNING: %s\n",message_string);
			}
		} break;
		default:
		{
			return_code=printf("UNKNOWN: %s\n",message_string);
		} break;
	}
	va_end(ap);
	LEAVE;

	return (return_code);
} /* display_message */
#else /* defined (MANUAL_CMISS) */
static int linux_execute(char *format, ... )
/*******************************************************************************
LAST MODIFIED : 16 February 2001

DESCRIPTION :
A function for executing a command under linux.  The printf form of arguments is
used.
==============================================================================*/
{
	char *host,*user;
	int return_code;
	va_list ap;

	ENTER(linux_execute);
	return_code=0;
	va_start(ap,format);
	if ((user=getenv("PF_CMISS_USER"))&&(host=getenv("PF_CMISS_HOST")))
	{
#if defined (WIN32)
		sprintf(command_string,"rsh %s -l %s setenv DISPLAY :0.0;",host,user);
		vsprintf(command_string+strlen(command_string),format,ap);
#else /* defined (WIN32) */
		sprintf(command_string,"rsh %s -l %s 'setenv DISPLAY :0.0;",host,user);
		vsprintf(command_string+strlen(command_string),format,ap);
		sprintf(command_string+strlen(command_string)," '");
#if defined (DEBUG)
		vsprintf(command_string,format,ap);
#endif /* defined (DEBUG) */

#endif /* defined (WIN32) */
		if (-1!=system(command_string))
		{
			return_code=1;
		}
	}
	va_end(ap);
	LEAVE;

	return (return_code);
} /* linux_execute */
#endif /* defined (MANUAL_CMISS) */

static int read_line(FILE *file,char *buff,int buff_limit)
/*******************************************************************************
LAST MODIFIED : 13 February 2001

DESCRIPTION :
Get the next line in the data file.
==============================================================================*/
{
	char *c;
	int return_code;
 
 	ENTER(read_line);
	return_code=0;
	while ((0==return_code)&&
		((c = fgets(buff,buff_limit,file)) == NULL || buff[0] == '\0'))
	{
		if (c == NULL)
		{
			if (feof(file))
			{
				return_code=EOF;
			}
			else if (ferror(file))
			{
#if defined (MANUAL_CMISS)
				display_message(PF_ERROR_MESSAGE,"read_line.  Error encountered");
#endif /* defined (MANUAL_CMISS) */
				return_code= -2;
			}
		}
	}
	LEAVE;

	return (return_code);
} /* read_line */

static int reduce_fuzzy_string(char *reduced_string,char *string)
/*******************************************************************************
LAST MODIFIED : 16 September 1998

DESCRIPTION :
Copies <string> to <reduced_string> converting to upper case and removing
whitespace.
==============================================================================*/
{
	char *destination,*source;
	int return_code;

	ENTER(reduce_fuzzy_string);
	if ((source=string)&&(destination=reduced_string))
	{
		while (*source)
		{
			/* remove whitespace, -'s and _'s */
			/*???RC don't know why you want to exclude - and _. I had a case where
				I typed gfx create fibre_ (short for fibre_field) and it thought it was
				ambiguous since there is also a gfx create fibres commands. */
			if (!isspace(*source)/*&&(*source!='-')&&(*source!='_')*/)
			{
				*destination=toupper(*source);
				destination++;
			}
			source++;
		}
		/* terminate the string */
		*destination='\0';
		return_code=1;
	}
	else
	{
#if defined (MANUAL_CMISS)
		display_message(PF_ERROR_MESSAGE,"reduce_fuzzy_string.  Invalid argument(s)");
#endif /* defined (MANUAL_CMISS) */
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* reduce_fuzzy_string */

#if defined (OLD_CODE)
static int fuzzy_string_compare(char *first,char *second)
/*******************************************************************************
LAST MODIFIED : 30 August 2000

DESCRIPTION :
This is a case insensitive compare disregarding certain characters (whitespace,
dashes and underscore).  For example, "Ambient Colour" matches the following:

"AMBIENT_COLOUR", "ambient_colour", "ambientColour", "Ambient_Colour",
"ambient-colour", "AmBiEnTcOlOuR", "Ambient-- Colour"

and a large set of even louder versions.

Actually this has been changed so the underscore and dashes are not ignored.

Both strings are first reduced, which removes whitespace and converts to upper
case. Returns 1 iff the reduced second string is at least as long as the
reduced first string and starts with the characters in the first.
==============================================================================*/
{
	char *first_reduced,*second_reduced;
	int compare_length,first_length,return_code,second_length;

	ENTER(fuzzy_string_compare);
	if (first&&second)
	{
		/* allocate memory */
		if (ALLOCATE(first_reduced,char,strlen(first)+1)&&
			ALLOCATE(second_reduced,char,strlen(second)+1))
		{
			/* reduce strings */
			if (reduce_fuzzy_string(first_reduced,first)&&
				reduce_fuzzy_string(second_reduced,second))
			{
				first_length=strlen(first_reduced);
				second_length=strlen(second_reduced);
				/* first reduced string must not be longer than second */
				if (first_length <= second_length)
				{
					compare_length=first_length;
					if (strncmp(first_reduced,second_reduced,compare_length))
					{
						return_code=0;
					}
					else
					{
						return_code=1;
					}
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
#if defined (MANUAL_CMISS)
				display_message(PF_ERROR_MESSAGE,"fuzzy_string_compare.  Error reducing");
#endif /* defined (MANUAL_CMISS) */
				return_code=0;
			}
			DEALLOCATE(first_reduced);
			DEALLOCATE(second_reduced);
		}
		else
		{
			DEALLOCATE(first_reduced);
#if defined (MANUAL_CMISS)
			display_message(PF_ERROR_MESSAGE,
				"fuzzy_string_compare.  Insufficient memory");
#endif /* defined (MANUAL_CMISS) */
			return_code=0;
		}
	}
	else
	{
#if defined (MANUAL_CMISS)
		display_message(PF_ERROR_MESSAGE,"fuzzy_string_compare.  Invalid arguments");
#endif /* defined (MANUAL_CMISS) */
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* fuzzy_string_compare */
#endif /* defined (OLD_CODE) */

static int fuzzy_string_compare_same_length(char *first,char *second)
/*******************************************************************************
LAST MODIFIED : 14 August 1998

DESCRIPTION :
Same as fuzzy_string_compare except that the two reduced strings must be the
same length.
==============================================================================*/
{
	char *first_reduced,*second_reduced;
	int return_code;

	ENTER(fuzzy_string_compare_same_length);
	if (first&&second)
	{
		if (ALLOCATE(first_reduced,char,strlen(first)+1)&&
			ALLOCATE(second_reduced,char,strlen(second)+1))
		{
			/* reduce strings */
			if (reduce_fuzzy_string(first_reduced,first)&&
				reduce_fuzzy_string(second_reduced,second))
			{
				if (0==strcmp(first_reduced,second_reduced))
				{
					return_code=1;
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
#if defined (MANUAL_CMISS)
				display_message(PF_ERROR_MESSAGE,
					"fuzzy_string_compare_same_length.  Error reducing");
#endif /* defined (MANUAL_CMISS) */
				return_code=0;
			}
			DEALLOCATE(first_reduced);
			DEALLOCATE(second_reduced);
		}
		else
		{
			DEALLOCATE(first_reduced);
#if defined (MANUAL_CMISS)
			display_message(PF_ERROR_MESSAGE,
				"fuzzy_string_compare_same_length.  Insufficient memory");
#endif /* defined (MANUAL_CMISS) */
			return_code=0;
		}
	}
	else
	{
#if defined (MANUAL_CMISS)
		display_message(PF_ERROR_MESSAGE,
			"fuzzy_string_compare_same_length.  Invalid arguments");
#endif /* defined (MANUAL_CMISS) */
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* fuzzy_string_compare_same_length */

static int copy_file(char *source_filename, char *destination_filename)
/*******************************************************************************
LAST MODIFIED : 13 February 2001

DESCRIPTION :
Copy the <source> file to the <destination>
==============================================================================*/
{
#define COPY_FILE_BUFFER_SIZE (1000)
	char copy_buffer[COPY_FILE_BUFFER_SIZE];
	FILE *source, *destination;
	int return_code;

	ENTER(copy_file);
	return_code=0;
 	if (source = fopen(source_filename, "r"))
	{
		if (destination = fopen(destination_filename, "w"))
		{
			return_code = 0;
			while ((return_code == 0) && (!feof(source)) && 
				(!read_line(source, copy_buffer, COPY_FILE_BUFFER_SIZE)))
			{
				fprintf(destination, "%s", copy_buffer);
			}
			fclose(destination);
		}
		else
		{
#if defined (MANUAL_CMISS)
			display_message(PF_ERROR_MESSAGE,
				"copy_file.  Unable to open destination file %s",
				destination_filename);
#endif /* defined (MANUAL_CMISS) */
			return_code= -2;
		}
		fclose(source);
	}
	else
	{
#if defined (MANUAL_CMISS)
		display_message(PF_ERROR_MESSAGE,
			"copy_file.  Unable to open source file %s",source_filename);
#endif /* defined (MANUAL_CMISS) */
		return_code= -1;
	}
	LEAVE;

	return (return_code);
} /* copy_file */

#define MARKER_BUFFER_SIZE (500)

static struct Marker_struct *read_marker_file(char *filename)
/*******************************************************************************
LAST MODIFIED : 13 February 2001

DESCRIPTION :
Reads in a plain text marker file and creates the structure that represents that
data.
==============================================================================*/
{
	char buffer[MARKER_BUFFER_SIZE],marker_name[MARKER_BUFFER_SIZE],
		*new_marker_name,**new_marker_names;
	FILE *marker_file;
	float marker_x, marker_y, marker_z, *new_marker_positions;
	int marker_index, *new_marker_indices;
	struct Marker_struct *markers;
	
	ENTER(read_marker_file);
	if (ALLOCATE(markers,struct Marker_struct,1))
	{
		markers->number_of_markers = 0;
		markers->marker_names = (char **)NULL;
		markers->marker_indices = (int *)NULL;
		markers->marker_positions = (float *)NULL;
		marker_index = 200001; /* Need to offset from host mesh and obj mesh */
		
		if (marker_file = fopen(filename, "r"))
		{
			while (markers && (!feof(marker_file)) && 
				(!read_line(marker_file, buffer, MARKER_BUFFER_SIZE)))
			{
				if (4 == sscanf(buffer, "%s%f%f%f", marker_name,
					&marker_x, &marker_y, &marker_z))
				{
					if (REALLOCATE(new_marker_names,markers->marker_names,char *,
						markers->number_of_markers+1)&&
						REALLOCATE(new_marker_indices,markers->marker_indices,int,
						markers->number_of_markers+1)&&
						REALLOCATE(new_marker_positions,markers->marker_positions,float,
						3*(markers->number_of_markers+1))&&
						ALLOCATE(new_marker_name,char,strlen(marker_name)+1))
					{
						markers->marker_names = new_marker_names;
						markers->marker_indices = new_marker_indices;
						markers->marker_positions = new_marker_positions;
						new_marker_names[markers->number_of_markers] = new_marker_name;
						strcpy(new_marker_name, marker_name);
						new_marker_indices[markers->number_of_markers] = marker_index;
						marker_index++;
						new_marker_positions[3 * markers->number_of_markers] = marker_x;
						new_marker_positions[3 * markers->number_of_markers + 1] = marker_y;
						new_marker_positions[3 * markers->number_of_markers + 2] = marker_z;
						markers->number_of_markers++;
					}
					else
					{
						DEALLOCATE(markers);
#if defined (MANUAL_CMISS)
						display_message(PF_ERROR_MESSAGE,
							"Problem with reallocating marker storage");
#endif /* defined (MANUAL_CMISS) */
					}
				}
			}
			fclose(marker_file);
		}
		else
		{
			DEALLOCATE(markers);
#if defined (MANUAL_CMISS)
			display_message(PF_ERROR_MESSAGE,"Unable to open marker file %s",
				filename);
#endif /* defined (MANUAL_CMISS) */
		}
	}
#if defined (MANUAL_CMISS)
	else
	{
		display_message(PF_ERROR_MESSAGE,
			"Unable to allocate memory for marker structure");
	}
#endif /* defined (MANUAL_CMISS) */
	LEAVE;

	return (markers);
} /* read_marker_file */

static int write_exnode(char *filename, char *group_name, char *field_name,
	int number_of_components, int number_of_nodes, int *node_indices,
	float *values)
/*******************************************************************************
LAST MODIFIED : 13 February 2001

DESCRIPTION :
==============================================================================*/
{
	float *value_index;
	FILE *file;
	int i, j, return_code;
	
	ENTER(write_exnode);
	return_code = 0;
	if (file = fopen(filename, "w"))
	{
		fprintf(file,"Group name: %s\n", group_name);
		fprintf(file,"#Fields=1\n");

		fprintf(file,"1) %s, coordinate, rectangular cartesian, #Components=%d\n",
			field_name, number_of_components);
		for (j = 0 ; j < number_of_components ; j++)
		{
			fprintf(file," %d. Value index=%d, #Derivatives=0\n", j + 1, j + 1);
		}
		value_index = values;
		for (i = 0 ; i < number_of_nodes ; i++)
		{
			/* Ignore nodes with a zero index */
			if (node_indices[i])
			{
				fprintf(file,"Node: %d\n", node_indices[i]);
				for (j = 0 ; j < number_of_components ; j++)
				{
					fprintf(file,"%f ", *value_index);
					value_index++;
				}
				fprintf(file,"\n");
			}
			else
			{
				value_index+=number_of_components;
			}
		}
		fclose(file);
	}
	else
	{
#if defined (MANUAL_CMISS)
		display_message(PF_ERROR_MESSAGE, "Unable to open file %s for writing",
			filename);
#endif /* defined (MANUAL_CMISS) */
		return_code = -4;
	}
	LEAVE;

	return (return_code);
} /* write_exnode */

#define MAX_VERT 10000
#define MAX_TRI 10000
#define MAX_OBJ_VERTICES 4

static struct Obj *read_obj(char *filename)
/*******************************************************************************
LAST MODIFIED : 13 February 2001

DESCRIPTION :
==============================================================================*/
{
	char text[512], *word, face_word[MAX_OBJ_VERTICES][128];
	FILE *objfile;
	int i, n_face_vertices, nfiles = 0, n_v = 1, n_t=0, n_vt=1, n_vn=1, n_mat=0;
	int id = 0, return_code;
	double *v, *vn, *vt;
	int *t;
	int n_lines = 0;
	struct Obj *obj;

	ENTER(read_obj);
	obj=(struct Obj *)NULL;
	if (ALLOCATE(v, double, 3 * MAX_VERT) &&
		ALLOCATE(vn, double, 3 * MAX_VERT) &&
		ALLOCATE(vt, double, 3 * MAX_VERT * 3) && /* This is what it was */
		ALLOCATE(t, int, MAX_TRI * 3 * 3))
	{
		return_code = 0;
		/* print header to exnode file */
		if (objfile = fopen(filename, "r"))
		{
			n_vt = 0;
			n_vn = 0;
			n_v = 0;
			n_t = 0;
			if (ALLOCATE(obj,struct Obj,1))
			{
				/* scan input file into memory */
				while (NULL!=fgets(text, 512, objfile))
				{
					word=strtok(text," ");
					if (word)
					{
						if (0==strncmp(word,"vt", 2))
						{
							i=0;
							while (word=strtok(NULL," "))
							{
								vt[3 * n_vt + i]=atof(word);
								/*printf("word(%s) vt[%d][%d] = %lf\n", word, n_vt, i, vt[n_vt][i]);*/
								i++;
							}
							n_vt++;
						}			
						else if (0==strncmp(word,"vn", 2))
						{
							i=0;
							while (word=strtok(NULL," "))
							{
								vn[3 * n_vn + i]=atof(word);
								i++;
							}
							n_vn++;
						}
						else if (0==strcmp(word,"v"))
						{
							/* process vertices */
							i=0;
							while (word=strtok(NULL," "))
							{
								v[3 * n_v + i]=atof(word);
								i++;
							}				
							n_v++;
						}			
						else if (0==strcmp(word,"f"))
						{
							/* process faces */
							n_face_vertices=0;
							while (word=strtok(NULL," "))
							{
								if (n_face_vertices<MAX_OBJ_VERTICES)
								{
									strcpy(face_word[n_face_vertices],word);
								}
								n_face_vertices++;
							}

							if (3==n_face_vertices)
							{
								for (i=0;i<n_face_vertices;i++)
								{
									if (!strstr(face_word[i],"/"))
									{
										/* no vt or vn data (represent as 0) */
										t[n_t * 9 + i * 3 + 0]=atoi(face_word[i]);
										t[n_t * 9 + i * 3 + 1]=0;
										t[n_t * 9 + i * 3 + 2]=0;
									}
									else
									{
										if (strstr(face_word[i],"//"))
										{
											/* no vt data (represent as 0) */
											word=strtok(face_word[i],"//");
											t[n_t * 9 + i * 3 + 0]=atoi(word);
											t[n_t * 9 + i * 3 + 1]=0;
											if(word=strtok(NULL,"//"))
											{
												t[n_t * 9 + i * 3 + 2]=atoi(word);
												word=strtok(NULL,"/");
											}
											else
											{
												t[n_t * 9 + i * 3 + 2]=0;
											}
										}
										else
										{
											word=strtok(face_word[i], "/");
											t[n_t * 9 + i * 3 + 0]=atoi(word);
											if(word=strtok(NULL,"/"))
											{
												t[n_t * 9 + i * 3 + 1]=atoi(word);
												if(word=strtok(NULL,"/"))
												{
													t[n_t * 9 + i * 3 + 2]=atoi(word);
												}
												else
												{
													t[n_t * 9 + i * 3 + 2]=0;
												}
											}
											else
											{
												t[n_t * 9 + i * 3 + 1]=0;
												t[n_t * 9 + i * 3 + 2]=0;
											}
										}
									}
								}
								n_t++;
							}
							else
							{
#if defined (MANUAL_CMISS)
								display_message(PF_ERROR_MESSAGE,
									"Dis ain't no t-mesh baby - n_vertices = %d",
									n_face_vertices);
#endif /* defined (MANUAL_CMISS) */
								DEALLOCATE(obj);
							}
						}
					}
					n_lines++;
				}
				if (obj)
				{
					if (ALLOCATE(obj->vertex_3d_locations,float,3*n_v)&&
						ALLOCATE(obj->texture_vertex_3d_locations,float,3*n_vt)&&
						ALLOCATE(obj->triangle_vertices,int,3*n_t)&&
						ALLOCATE(obj->triangle_texture_vertices,int,3*n_t))
					{
						for (i = 0 ; i < n_v ; i++)
						{
							obj->vertex_3d_locations[3 * i] = (float)v[3 * i + 0];
							obj->vertex_3d_locations[3 * i + 1] = (float)v[3 * i + 1];
							obj->vertex_3d_locations[3 * i + 2] = (float)v[3 * i + 2];
						}
						obj->number_of_vertices = n_v;
						for (i = 0 ; i < n_vt ; i++)
						{
							obj->texture_vertex_3d_locations[3 * i] = (float)vt[3 * i + 0];
							obj->texture_vertex_3d_locations[3 * i + 1] =
								(float)vt[3 * i + 1];
							obj->texture_vertex_3d_locations[3 * i + 2] =
								(float)vt[3 * i + 2];
						}
						obj->number_of_texture_vertices = n_vt;
						for (i = 0 ; i < n_t ; i++)
						{
							obj->triangle_vertices[3 * i] = t[i * 9 + 0 * 3 + 0];
							obj->triangle_vertices[3 * i + 1] = t[i * 9 + 1 * 3 + 0];
							obj->triangle_vertices[3 * i + 2] = t[i * 9 + 2 * 3 + 0];
							obj->triangle_texture_vertices[3 * i] = t[i * 9 + 0 * 3 + 2];
							obj->triangle_texture_vertices[3 * i + 1] = t[i * 9 + 1 * 3 + 2];
							obj->triangle_texture_vertices[3 * i + 2] = t[i * 9 + 2 * 3 + 2];
						}
						obj->number_of_triangles = n_t;
					}
					else
					{
#if defined (MANUAL_CMISS)
						display_message(PF_ERROR_MESSAGE,"Unable to allocate Obj fields");
#endif /* defined (MANUAL_CMISS) */
						DEALLOCATE(obj);
					}
				}
			}
#if defined (MANUAL_CMISS)
			else
			{
				display_message(PF_ERROR_MESSAGE, "Unable to allocate Obj structure");
			}
#endif /* defined (MANUAL_CMISS) */
		}
#if defined (MANUAL_CMISS)
		else
		{
			display_message(PF_ERROR_MESSAGE, "Unable to open objfile %s",filename);
		}
#endif /* defined (MANUAL_CMISS) */
	}
#if defined (MANUAL_CMISS)
	else
	{
		display_message(PF_ERROR_MESSAGE,
			"Unable to ALLOCATE temporary vertex arrays");
	}
#endif /* defined (MANUAL_CMISS) */
	LEAVE;

	return (obj);
} /* read_obj */

static int read_and_byte_swap(unsigned char *byte_array,int value_size,
	int number_of_values,int least_to_most,FILE *input_file)
/*******************************************************************************
LAST MODIFIED : 13 February 2001

DESCRIPTION :
Performs the read and byte.
???DB.  Should be combined with functions in general/myio
==============================================================================*/
{
	int return_code;
#if defined (__BYTE_ORDER)
	int i,j;
	unsigned char *bottom_byte,byte,*element,*top_byte;
#endif /* defined (__BYTE_ORDER) */

	ENTER(read_and_byte_swap);
	return_code=0;
#if defined (__BYTE_ORDER)
#if (1234==__BYTE_ORDER)
	if (!least_to_most)
#else /* (1234==__BYTE_ORDER) */
	if (least_to_most)
#endif /* (1234==__BYTE_ORDER) */
	{
		if (number_of_values==(return_code=fread(byte_array,value_size,
			number_of_values,input_file)))
		{
			element=byte_array;
			for (j=number_of_values;j>0;j--)
			{
				bottom_byte=element;
				top_byte=element+value_size;
				for (i=value_size/2;i>0;i--)
				{
					top_byte--;
					byte= *bottom_byte;
					*bottom_byte= *top_byte;
					*top_byte=byte;
					bottom_byte++;
				}
				element += value_size;
			}
		}
	}
	else
	{
		return_code=fread(byte_array,value_size,number_of_values,input_file);
	}
#else /* defined (__BYTE_ORDER) */
	return_code=fread(byte_array,value_size,number_of_values,input_file);
#endif /* defined (__BYTE_ORDER) */
	LEAVE;

	return (return_code);
} /* read_and_byte_swap */

static int read_rgb_image_file(char *file_name,
	int *number_of_components_address, int *number_of_bytes_per_component,
	long int *height_address,long int *width_address,
	long unsigned **image_address)
/*******************************************************************************
LAST MODIFIED : 13 February 2001

DESCRIPTION :
Reads an image from a SGI rgb file.
number_of_components=1, I
number_of_components=2, IA
number_of_components=3, RGB
number_of_components=4, RGBA
???DB.  Need to find out more about images.  See 4Dgifts/iristools.
==============================================================================*/
{
	FILE *image_file;
	int i,j,k,least_to_most,max_row_size,number_of_bytes,number_of_rows,
		return_code,run_length;
	long row_size, row_start;
	unsigned char *image,*image_ptr,pixel,*row,*row_ptr;
	unsigned char *row_size_char,*row_sizes_char,*row_start_char,*row_starts_char;
	unsigned short dimension,height,image_file_type,magic_number,
		number_of_components,pixel2,width;

	ENTER(read_rgb_image_file);
	return_code=0;
	least_to_most = 0;
	/* check arguments */
	if (file_name&&number_of_components_address&&height_address&&width_address&&
		image_address)
	{
		if (image_file=fopen(file_name,"rb"))
		{
			if ((1==read_and_byte_swap((unsigned char *)&magic_number,2,1,
				least_to_most,image_file))&&
				(1==read_and_byte_swap((unsigned char *)&image_file_type,2,1,
				least_to_most,image_file))&&
				(1==read_and_byte_swap((unsigned char *)&dimension,2,1,least_to_most,
				image_file))&&
				(1==read_and_byte_swap((unsigned char *)&width,2,1,least_to_most,
				image_file))&&
				(1==read_and_byte_swap((unsigned char *)&height,2,1,least_to_most,
				image_file))&&
				(1==read_and_byte_swap((unsigned char *)&number_of_components,2,1,
				least_to_most,image_file)))
			{
#if defined (DEBUG)
				/*???debug */
				printf("magic_number %x\n",magic_number);
				printf("image_file_type %x\n",image_file_type);
				printf("dimension %d\n",dimension);
				printf("width %d\n",width);
				printf("height %d\n",height);
				printf("number_of_components %d\n",number_of_components);
#endif /* defined (DEBUG) */
				if ((0<width)&&(0<height)&&(1<=number_of_components)&&
					(number_of_components<=4))
				{
					number_of_rows=height*number_of_components;
					return_code=1;
					image=(unsigned char *)NULL;
					*number_of_bytes_per_component = (image_file_type&0x000000ff);
					number_of_bytes = number_of_components * *number_of_bytes_per_component;
					if (0x00000100==(image_file_type&0x0000ff00))
					{
						/* run length encoded */
						row_starts_char=(unsigned char *)NULL;
						row_sizes_char=(unsigned char *)NULL;
						/* SAB All this stuff needs to avoid the use of platform
							dependencies, like sizeof (long) etc. so I have used
							unsigned char throughout */
						if (ALLOCATE(row_starts_char,unsigned char,4*number_of_rows)&&
							ALLOCATE(row_sizes_char,unsigned char,4*number_of_rows)&&
							ALLOCATE(image,unsigned char,
							((width*height*number_of_bytes+3)/4)*4))
						{
							/* No longer doing a byte_swap as the use of << to add the
								bytes of the chars together will sort that out */
							if ((0==fseek(image_file,512,SEEK_SET))&&
								((unsigned)number_of_rows==fread(row_starts_char,4,
								number_of_rows,image_file))&&
								((unsigned)number_of_rows==fread(row_sizes_char,4,
								number_of_rows,image_file)))
							{
								/* find the maximum row size */
								row_size_char=row_sizes_char;
								row_size = ((long)row_size_char[3]) + 
									(((long)row_size_char[2]) << 8) +
									(((long)row_size_char[1]) << 16) +
									(((long)row_size_char[0]) << 24);
								max_row_size = row_size;
								for (i=number_of_rows-1;i>0;i--)
								{
									row_size_char += 4;
									row_size = ((long)row_size_char[3]) + 
										(((long)row_size_char[2]) << 8) +
										(((long)row_size_char[1]) << 16) +
										(((long)row_size_char[0]) << 24);
									if (row_size>max_row_size)
									{
										max_row_size = row_size;
									}
								}
								max_row_size *= *number_of_bytes_per_component;
								if (ALLOCATE(row,unsigned char,max_row_size))
								{
									row_start_char=row_starts_char;
									row_size_char=row_sizes_char;
									image_ptr=image;
									i=0;
									while (return_code&&(i<number_of_components))
									{
										image_ptr=image+i* *number_of_bytes_per_component;
										j=height;
										while (return_code&&(j>0))
										{
											row_start = ((long)row_start_char[3]) + 
												(((long)row_start_char[2]) << 8) +
												(((long)row_start_char[1]) << 16) +
												(((long)row_start_char[0]) << 24);
											row_size = ((long)row_size_char[3]) + 
												(((long)row_size_char[2]) << 8) +
												(((long)row_size_char[1]) << 16) +
												(((long)row_size_char[0]) << 24);
											switch( *number_of_bytes_per_component )
											{
												case 1:
												{
													if ((0==fseek(image_file,row_start,SEEK_SET))&&
														((unsigned)row_size==fread(row,1,row_size,image_file)))
													{
														row_ptr=row;
														pixel= *row_ptr;
														run_length=(int)(pixel&0x0000007F);
														while (run_length)
														{
															row_ptr++;
															if (pixel&0x00000080)
															{
																while (run_length>0)
																{
																	*image_ptr= *row_ptr;
																	image_ptr += number_of_components;
																	row_ptr++;
																	run_length--;
																}
															}
															else
															{
																pixel= *row_ptr;
																row_ptr++;
																while (run_length>0)
																{
																	*image_ptr=pixel;
																	image_ptr += number_of_components;
																	run_length--;
																}
															}
															pixel= *row_ptr;
															run_length=(int)(pixel&0x0000007F);
														}
														row_start_char+=4;
														row_size_char+=4;
														j--;
													}
													else
													{
#if defined (MANUAL_CMISS)
														display_message(PF_ERROR_MESSAGE,
													"read_rgb_image_file.  Error reading row component");
#endif /* defined (MANUAL_CMISS) */
														return_code=0;
													}
												} break;
												case 2:
												{
													if ((0==fseek(image_file,row_start,SEEK_SET))&&
														((unsigned)row_size==fread(row,1,row_size,
														image_file)))
													{
														row_ptr=row;
														pixel2 = (((unsigned short)*row_ptr) << 8) +
															((unsigned short)*(row_ptr + 1));
														run_length=(int)(pixel2&0x0000007F);
														while (run_length)
														{
															row_ptr+=2;
															if (pixel2&0x00000080)
															{
																while (run_length>0)
																{
																	*image_ptr= *row_ptr;
																	*(image_ptr + 1)= *(row_ptr + 1);
																	image_ptr += number_of_components * 2;
																	row_ptr+=2;
																	run_length--;
																}
															}
															else
															{
																pixel2= (((unsigned short)*row_ptr) << 8) +
																	((unsigned short)*(row_ptr + 1));
																row_ptr+=2;
																while (run_length>0)
																{
																	*image_ptr=(pixel2 & 0xff00)>>8;
																	*(image_ptr + 1)= pixel2 & 0x00ff;
																	image_ptr += number_of_components * 2;
																	run_length--;
																}
															}
															pixel2= (((unsigned short)*row_ptr) << 8) +
																((unsigned short)*(row_ptr + 1));
															run_length=(int)(pixel2&0x0000007F);
														}
														row_start_char+=4;
														row_size_char+=4;
														j--;
													}
													else
													{
#if defined (MANUAL_CMISS)
														display_message(PF_ERROR_MESSAGE,
													"read_rgb_image_file.  Error reading row component");
#endif /* defined (MANUAL_CMISS) */
														return_code=0;
													}
												} break;
												default:
												{
#if defined (MANUAL_CMISS)
													display_message(PF_ERROR_MESSAGE,
											"read_rgb_image_file.  Unsupported bytes per component");
#endif /* defined (MANUAL_CMISS) */
													return_code=0;
												} break;
											}
													
										}
										i++;
									}
									DEALLOCATE(row);
								}
								else
								{
#if defined (MANUAL_CMISS)
									display_message(PF_ERROR_MESSAGE,
										"read_rgb_image_file.  Could not allocate row array");
#endif /* defined (MANUAL_CMISS) */
									return_code=0;
								}
							}
							else
							{
#if defined (MANUAL_CMISS)
								display_message(PF_ERROR_MESSAGE,
									"read_rgb_image_file.  Error reading row start/size");
#endif /* defined (MANUAL_CMISS) */
								return_code=0;
							}
						}
						else
						{
#if defined (MANUAL_CMISS)
							display_message(PF_ERROR_MESSAGE,
								"read_rgb_image_file.  Could not allocate image arrays");
#endif /* defined (MANUAL_CMISS) */
							return_code=0;
						}
						DEALLOCATE(row_starts_char);
						DEALLOCATE(row_sizes_char);
					}
					else
					{
						/* verbatim */
						if (ALLOCATE(row,unsigned char,
							width*(*number_of_bytes_per_component))&&
							ALLOCATE(image,unsigned char,width*height*number_of_bytes))
						{
							if (0==fseek(image_file,512,SEEK_SET))
							{
								i=0;
								while (return_code&&(i<number_of_components))
								{
									image_ptr=image+i;
									j=height;
									while (return_code&&(j>0))
									{
										if (width==read_and_byte_swap(row,
											*number_of_bytes_per_component,width,least_to_most,
											image_file))
										{
											row_ptr=row;
											switch( *number_of_bytes_per_component )
											{
												case 1:
												{
													for (k=width;k>0;k--)
													{
														*image_ptr= *row_ptr;
														row_ptr++;
														image_ptr += number_of_components;
													}
												} break;
												case 2:
												{
													for (k=width;k>0;k--)
													{
														*image_ptr= *row_ptr;
														*(image_ptr + 1)= *(row_ptr + 1);
														row_ptr+=2;
														image_ptr += number_of_components * 2;
													}
												} break;
												default:
												{
#if defined (MANUAL_CMISS)
													display_message(PF_ERROR_MESSAGE,
											"read_rgb_image_file.  Unsupported bytes per component");
#endif /* defined (MANUAL_CMISS) */
													return_code=0;
												} break;
											}
											j--;
										}
										else
										{
#if defined (MANUAL_CMISS)
											display_message(PF_ERROR_MESSAGE,
												"read_rgb_image_file.  Error reading row component");
#endif /* defined (MANUAL_CMISS) */
											return_code=0;
										}
									}
									i++;
								}
							}
							else
							{
#if defined (MANUAL_CMISS)
								display_message(PF_ERROR_MESSAGE,
									"read_rgb_image_file.  Error positioning file");
#endif /* defined (MANUAL_CMISS) */
								return_code=0;
							}
						}
						else
						{
#if defined (MANUAL_CMISS)
							display_message(PF_ERROR_MESSAGE,
								"read_rgb_image_file.  Could not allocate image arrays");
#endif /* defined (MANUAL_CMISS) */
							return_code=0;
						}
						DEALLOCATE(row);
					}
					if (return_code)
					{
						*width_address=width;
						*height_address=height;
						*number_of_components_address=
							(int)number_of_components;
						*image_address=(unsigned long *)image;
					}
					else
					{
						DEALLOCATE(image);
					}
				}
				else
				{
#if defined (MANUAL_CMISS)
					display_message(PF_ERROR_MESSAGE,
						"read_rgb_image_file.  Invalid image size");
#endif /* defined (MANUAL_CMISS) */
					return_code=0;
				}
			}
			else
			{
#if defined (MANUAL_CMISS)
				display_message(PF_ERROR_MESSAGE,
					"read_rgb_image_file.  Error reading file information");
#endif /* defined (MANUAL_CMISS) */
				return_code=0;
			}
			fclose(image_file);
		}
#if defined (MANUAL_CMISS)
		else
		{
			display_message(PF_ERROR_MESSAGE,
				"read_rgb_image_file.  Could not open image file");
		}
#endif /* defined (MANUAL_CMISS) */
	}
#if defined (MANUAL_CMISS)
	else
	{
		display_message(PF_ERROR_MESSAGE,
			"read_rgb_image_file.  Invalid argument(s)");
	}
#endif /* defined (MANUAL_CMISS) */
	LEAVE;

	return (return_code);
} /* read_rgb_image_file */

static int read_basis_version1and2(char *filename,int *m,int *n,float **a,
	int verbose)
/*******************************************************************************
LAST MODIFIED : 13 February 2001

DESCRIPTION :
Read in a basis file of either version 1 or version 2, only keeping the
m, n and a parts as this is all that is stored in a version 2 file.
==============================================================================*/
{
#define MAGICSIZE (13)
	char magic1[] = "em basis data";
	char magic2[] = "em basis 2.0";
	char buff[MAGICSIZE + 2];
	FILE* file;
	int i, j, return_code;

	ENTER(read_basis_version1and2);
	return_code=0;
#if defined (MANUAL_CMISS)
	if (verbose)
	{
		display_message(PF_INFORMATION_MESSAGE,
			"read_basis: reading in the basis file \"%s\"",filename);
	}
#endif /* defined (MANUAL_CMISS) */
	if (file = fopen(filename,"rb"))
	{
		fread(buff,MAGICSIZE,1,file);
		buff[MAGICSIZE] = 0;

		if (strncmp(buff,magic1,MAGICSIZE) == 0)
		{
#if defined (MANUAL_CMISS)
			if (verbose)
			{
				display_message(PF_INFORMATION_MESSAGE,
					"read_basis: magic number \"%s\"",buff);
			}
#endif /* defined (MANUAL_CMISS) */
#if defined (MANUAL_CMISS)
			display_message(PF_ERROR_MESSAGE, "read_basis_version1and2:"
				"\"%s\" isn't an unsupported old basis file. Magic is \"%s\"",
				filename, buff);
#endif /* defined (MANUAL_CMISS) */
			return_code = -17;
			fclose(file);
		}
		else if (strncmp(buff,magic2,MAGICSIZE - 1) == 0)
		{
			buff[MAGICSIZE - 1] = '\0';
#if defined (MANUAL_CMISS)
			if (verbose)
			{
				display_message(PF_INFORMATION_MESSAGE,
					"read_basis: magic number \"%s\"",buff);
			}
#endif /* defined (MANUAL_CMISS) */
			/* Comment/title line */
			fscanf(file, "%*[^\n]%*[\n]");
			fscanf(file, "%d%d", m, n);
#if defined (MANUAL_CMISS)
			if (verbose)
			{
				display_message(PF_INFORMATION_MESSAGE,"read_basis: dim %d %d",*m,*n);
			}
#endif /* defined (MANUAL_CMISS) */
			if (ALLOCATE(*a,float,(*m)*(*n)))
			{
				for (i=0;i<*n;i++)
				{
					for (j=0;j<*m;j++)
					{
						fscanf(file, "%f", *a + i * *m + j);
					}
				}
				return_code = 0;
			}
			else
			{
#if defined (MANUAL_CMISS)
				display_message(PF_ERROR_MESSAGE, "read_basis_version1and2:"
					"Could not allocate array");
#endif /* defined (MANUAL_CMISS) */
				return_code= -2;
			}
		}
		else
		{
#if defined (MANUAL_CMISS)
			display_message(PF_ERROR_MESSAGE, "read_basis_version1and2:"
				"\"%s\" isn't a basis file. Magic is \"%s\"", filename, buff);
#endif /* defined (MANUAL_CMISS) */
			return_code = -16;
		}
	}
	else
	{
#if defined (MANUAL_CMISS)
		display_message(PF_ERROR_MESSAGE, "read_basis_version1and2:"
			"Unable to open basis file \"%s\"", filename);
#endif /* defined (MANUAL_CMISS) */
		return_code = -18;
	}
	LEAVE;

	return (return_code);
} /* read_basis_version1and2 */

static int convert_2d_to_ndc(int number_of_markers, float *twod_coordinates,
	float *ndc_coordinates)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
==============================================================================*/
{
	float maximum, minimum, *ndc_index, padding_factor = 1.1f, *twod_index;
	int i, return_code;
	
	ENTER(convert_2d_to_ndc);
	return_code = 0;

	/* Find the maximum value */
	twod_index = twod_coordinates;
	maximum = *twod_index;
	minimum = *twod_index;
	twod_index++;
	for (i = 1 ; i < 2 * number_of_markers ; i++)
	{
		if (*twod_index > maximum)
		{
			maximum = *twod_index;
		}
		if (*twod_index < minimum)
		{
			minimum = *twod_index;
		}
		twod_index++;
	}

	if (maximum != minimum)
	{
		/* Store the maximum plus a bit for later */
		ndc_texture_scaling = 2.0f / (padding_factor * (maximum - minimum));
		ndc_texture_offset = minimum - (padding_factor - 1.0f) / (padding_factor * ndc_texture_scaling);
	}
	else
	{
		ndc_texture_scaling = 1.0f;
		ndc_texture_offset = minimum;
	}

#if defined (CMISS_DEBUG)	
	printf("Minimum %f Maximum %f\n", minimum, maximum);
	printf("Scaling %f Offset %f\n", ndc_texture_scaling, ndc_texture_offset);
#endif /* defined (CMISS_DEBUG) */
	
	/* Scale all the coordinates by this maximum plus a bit */
	twod_index = twod_coordinates;
	ndc_index = ndc_coordinates;
	for (i = 0 ; i < number_of_markers ; i++)
	{
		*ndc_index = (*twod_index - ndc_texture_offset) * ndc_texture_scaling - 1.0f;
		twod_index++;
		ndc_index++;
		/* Y goes in the reverse direction */
		*ndc_index = - (*twod_index - ndc_texture_offset) * ndc_texture_scaling + 1.0f;
		twod_index++;
		ndc_index++;
#if defined (CMISS_DEBUG)	
		printf ("Coordinates %f %f -> %f %f\n", *(twod_index-2), *(twod_index-1), *(ndc_index-2), *(ndc_index-1));
#endif /* defined (CMISS_DEBUG) */
	}	

	LEAVE;

	return (return_code);
} /* convert_2d_to_ndc */

/*
Global functions
----------------
*/
#if defined (MANUAL_CMISS)
int pf_set_display_message_function(enum PF_message_type message_type,
	PF_display_message_function *display_message_function,void *data)
/*******************************************************************************
LAST MODIFIED : 13 February 2001

DESCRIPTION :
A function for setting the <display_message_function> to be used for displaying
a message of the specified <message_type>.
==============================================================================*/
{
	int return_code;

	ENTER(set_display_message_function);
	/* in case forget to set return_code */
	return_code=PF_GENERAL_FAILURE_RC;
	switch (message_type)
	{
		case PF_ERROR_MESSAGE:
		{
			display_error_message_function=display_message_function;
			display_error_message_data=data;
			return_code=PF_SUCCESS_RC;
		} break;
		case PF_INFORMATION_MESSAGE:
		{
			display_information_message_function=display_message_function;
			display_information_message_data=data;
			return_code=PF_SUCCESS_RC;
		} break;
		case PF_WARNING_MESSAGE:
		{
			display_warning_message_function=display_message_function;
			display_warning_message_data=data;
			return_code=PF_SUCCESS_RC;
		} break;
		default:
		{
			display_message(PF_ERROR_MESSAGE,
				"pf_set_display_message_function.  Unknown message_type");
		} break;
	}
	LEAVE;

	return (return_code);
} /* pf_set_display_message_function */
#endif /* defined (MANUAL_CMISS) */

#if defined (LINUX_CMISS)
int pf_specify_paths(char *photoface_main_windows_path,
	char *photoface_main_linux_path)
/*******************************************************************************
LAST MODIFIED : 13 February 2001

DESCRIPTION :
Specifies the main directory under which the model, working and cmiss directory
are expected to be located under Windows and Linux.  The path should end in a
delimiting /.  This function will be obsolete once a windows only version of the
Photoface cmiss library is built.

If either path is NULL the internal storage for that path is free'd.
==============================================================================*/
{
	int return_code;

	ENTER(pf_specify_paths);
	return_code=PF_SUCCESS_RC;
	/* Deallocate the old paths */
	DEALLOCATE(photoface_windows_path);
	DEALLOCATE(photoface_linux_path);
	if (photoface_main_windows_path)
	{
		if (ALLOCATE(photoface_windows_path,char,
			strlen(photoface_main_windows_path)+1))
		{
			strcpy(photoface_windows_path, photoface_main_windows_path);
		}
		else
		{
#if defined (MANUAL_CMISS)
			display_message(PF_ERROR_MESSAGE,
				"Unable to allocate windows path string");
#endif /* defined (MANUAL_CMISS) */
			return_code = PF_ALLOCATE_FAILURE_RC;
		}
	}
	if (photoface_main_linux_path)
	{
		if (ALLOCATE(photoface_linux_path,char,
			strlen(photoface_main_linux_path)+1))
		{
			strcpy(photoface_linux_path, photoface_main_linux_path);
		}
		else
		{
#if defined (MANUAL_CMISS)
			display_message(PF_ERROR_MESSAGE,"Unable to allocate linux path string");
#endif /* defined (MANUAL_CMISS) */
			return_code = PF_ALLOCATE_FAILURE_RC;
		}
	}
	LEAVE;

	return (return_code);
} /* pf_specify_paths */
#endif /* defined (LINUX_CMISS) */

#if defined (WIN32)
DWORD WINAPI start_cmgui_thread_function(LPVOID dummy)
{
	DWORD return_code;

	ENTER(start_cmgui_thread_function);
	return_code=0;
	linux_execute("%sbin/cmgui_bg",photoface_linux_path);
	LEAVE;

	return (return_code);
} /* start_cmgui_thread_function */
#endif /* defined (WIN32) */

int pf_setup(char *model_name,char *state)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
<model_name> is the name of head model.  cmiss will have a directory containing
the files (generic obj etc) need for each model.  <state> is additional
information about the face in the image, such as "smiling", which may allow
adjustment of the generic head.
==============================================================================*/
{
	char *model_path, *working_path;
	FILE *setup_comfile;
	int length, return_code;
	struct stat stat_buffer;
#if defined (WIN32)
	DWORD start_cmgui_thread_id;
#else /* defined (WIN32) */
	int pid;
#endif /* defined (WIN32) */

	ENTER(pf_setup);
	return_code=PF_GENERAL_FAILURE_RC;
	/* Check that we have the files available for the specified model and state */
	if (ALLOCATE(model_path,char,
		strlen(photoface_windows_path)+strlen(model_name)+30))
	{
#if defined (WIN32)
		/* Have to remove the trailing / unless it is the root of a drive specification */
		strcpy(model_path, photoface_windows_path);
		length = strlen(model_path);
		if ((length > 1) && (':' != model_path[length - 2]))
		{
				model_path[length - 1] = 0;
		}
#else /* defined (WIN32) */
		strcpy(model_path, photoface_windows_path);
#endif /* defined (WIN32) */
		if (0 == stat(model_path, &stat_buffer))
		{
			sprintf(model_path,"%smodels/%s/%s.obj",photoface_windows_path,model_name,
				model_name);
			if (0 == stat(model_path, &stat_buffer))
			{
				if (ALLOCATE(working_path,char,2*strlen(photoface_windows_path)+30))
				{
					sprintf(working_path, "%sworking", photoface_windows_path);
					if (0 == stat(working_path, &stat_buffer))
					{
						sprintf(working_path, "%scmiss", photoface_windows_path);
						if (0 == stat(working_path, &stat_buffer))
						{
							/* Create the source files that represent the selected model and
								state, source.obj, source.basis, source.markers */
							sprintf(working_path, "%sworking/source.obj",
								photoface_windows_path);
							copy_file(model_path, working_path);
							sprintf(model_path,"%smodels/%s/%s.basis",photoface_windows_path,
								model_name,model_name);
							sprintf(working_path,"%sworking/source.basis",
								photoface_windows_path);
							copy_file(model_path, working_path);
							sprintf(model_path,"%smodels/%s/%s.markers",
								photoface_windows_path,model_name,model_name);
							sprintf(working_path,"%sworking/source.markers",
								photoface_windows_path);
							copy_file(model_path, working_path);
							sprintf(model_path,"%smodels/%s/%s_eyes.exnode",
								photoface_windows_path,model_name,model_name);
							sprintf(working_path,"%sworking/source_eyes.exnode",
								photoface_windows_path);
							copy_file(model_path, working_path);


							/* Create the startup comfile */
							sprintf(working_path, "%sworking/pf_setup_main.com", photoface_windows_path);
							if (setup_comfile = fopen(working_path, "w"))
							{
								fprintf(setup_comfile, "$PHOTOFACE_WORKING = \"%sworking\"\n",
									photoface_linux_path);
								fprintf(setup_comfile, "$PHOTOFACE_CMISS = \"%scmiss\"\n",
									photoface_linux_path);
								fprintf(setup_comfile, "$PHOTOFACE_BIN = \"%sbin\"\n",
									photoface_linux_path);
								fprintf(setup_comfile, "open comfile $PHOTOFACE_CMISS/pf_setup.com exec\n");

								fclose(setup_comfile);

								/* Start cmgui in a forked thread and load this model */
#if defined (WIN32)
								CreateThread(/*no security attributes*/NULL,
									/*use default stack size*/0,start_cmgui_thread_function,
									(LPVOID)NULL,/*use default creation flags*/0,
									&start_cmgui_thread_id);
								/* Main process comes here */
								Sleep((DWORD)10000);
#else /* defined (WIN32) */
								if (!(pid = fork()))
								{
#if defined (MANUAL_CMISS)
									if (display_message(PF_INFORMATION_MESSAGE,
#else /* defined (MANUAL_CMISS) */
									if (linux_execute(
#endif /* defined (MANUAL_CMISS) */
										"%sbin/cmgui_bg", photoface_linux_path))
									{
										return_code=PF_SUCCESS_RC;
									}
									exit(0);
								}
								/* Main process comes here */
								sleep(10);
#endif /* defined (WIN32) */
#if defined (MANUAL_CMISS)
								if (display_message(PF_INFORMATION_MESSAGE,
#else /* defined (MANUAL_CMISS) */
								if (linux_execute(
#endif /* defined (MANUAL_CMISS) */
									"cmgui_control 'open comfile %sworking/pf_setup_main.com exec'",
									photoface_linux_path))
								{
									return_code=PF_SUCCESS_RC;
								}

							}
							else
							{
#if defined (MANUAL_CMISS)
								display_message(PF_ERROR_MESSAGE,"Unable to open file %s",
									working_path);
#endif /* defined (MANUAL_CMISS) */
								return_code = PF_OPEN_FILE_FAILURE_RC;
							}

						}
						else
						{
#if defined (MANUAL_CMISS)
							display_message(PF_ERROR_MESSAGE,
								"Unable to find cmiss directory %s",working_path);
#endif /* defined (MANUAL_CMISS) */
							return_code = PF_FIND_FILE_FAILURE_RC;
						}
					}
					else
					{
#if defined (MANUAL_CMISS)
						display_message(PF_ERROR_MESSAGE,
							"Unable to find model directory %s",working_path);
#endif /* defined (MANUAL_CMISS) */
						return_code = PF_FIND_FILE_FAILURE_RC;
					}
					DEALLOCATE(working_path);
				}
				else
				{
#if defined (MANUAL_CMISS)
					display_message(PF_ERROR_MESSAGE,
						"Could not allocate required string");
#endif /* defined (MANUAL_CMISS) */
					return_code = PF_ALLOCATE_FAILURE_RC;
				}
			}
			else
			{
#if defined (MANUAL_CMISS)
				display_message(PF_ERROR_MESSAGE,"Unable to find model directory %s",
					model_path);
#endif /* defined (MANUAL_CMISS) */
				return_code = PF_FIND_FILE_FAILURE_RC;
			}
			DEALLOCATE(model_path);
		}
		else
		{
#if defined (MANUAL_CMISS)
			display_message(PF_ERROR_MESSAGE, "Unable to find photoface directory %s",
				photoface_windows_path);
#endif /* defined (MANUAL_CMISS) */
			return_code = PF_FIND_FILE_FAILURE_RC;
		}
	}
	else
	{
#if defined (MANUAL_CMISS)
		display_message(PF_ERROR_MESSAGE,"Could not allocate required string");
#endif /* defined (MANUAL_CMISS) */
		return_code = PF_ALLOCATE_FAILURE_RC;
	}
	LEAVE;

	return (return_code);
} /* pf_setup */

int pf_close(void)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
Closes the cmiss library.  Freeing any internal memory and stopping any
internal processes.
==============================================================================*/
{
	char **name_address;
	int i,return_code;

	ENTER(pf_close);
	return_code=PF_GENERAL_FAILURE_RC;
#if defined (LINUX_CMISS)
	/* deallocate the paths */
	DEALLOCATE(photoface_windows_path);
	DEALLOCATE(photoface_linux_path);
#endif /* defined (LINUX_CMISS) */
	/* free markers */
	if (markers)
	{
		name_address=markers->marker_names;
		for (i=markers->number_of_markers;i>0;i--)
		{
			DEALLOCATE(*name_address);
			name_address++;
		}
		DEALLOCATE(markers->marker_names);
		DEALLOCATE(markers->marker_indices);
		DEALLOCATE(markers->marker_positions);
		DEALLOCATE(markers);
	}
	/* close cmgui */
#if defined (MANUAL_CMISS)
	if (display_message(PF_INFORMATION_MESSAGE,
#else /* defined (MANUAL_CMISS) */
	if (linux_execute(
#endif /* defined (MANUAL_CMISS) */
		"cmgui_control 'quit'"))
	{
		return_code=PF_SUCCESS_RC;
	}
	LEAVE;

	return (return_code);
} /* pf_close */

int pf_define_markers(int number_of_markers,char **marker_names,
	float *marker_3d_generic_positions)
/*******************************************************************************
LAST MODIFIED : 13 February 2001

DESCRIPTION :
Define <number_of_markers> using
<marker_names> an array of <number_of_markers> marker names
<marker_3d_generic_positions> an array of 3*<number_of_markers> floats giving
the locations for the markers in the generic model (marker number varying
slowest).
If a marker already exists for the model then its position in the generic head
model will be updated, other wise a new marker will be created.
Please note that
1.  These are the positions on the generic head not the one being created.  This
	is like in the pipeline where you click on the image and the corresponding
	point on the generic model.  Its the points on the generic head that would be
	passed to pf_define_markers.
2.  pf_define_markers involves cmiss doing an optimization to get the material
	points.
==============================================================================*/
{
	char filename[200], *new_marker_name, **new_marker_names;
	float *new_marker_positions;	
	int i, j, marker_index, return_code, *new_marker_indices;

	ENTER(pf_define_markers);
	return_code=PF_SUCCESS_RC;
	/* If we haven't already done so read the marker definition file */
	if (!markers)
	{
		sprintf(filename, "%sworking/source.markers", photoface_windows_path);
		markers = read_marker_file(filename);
	}
	if (markers)
	{
		/* Find the node number equivalents for each of the marker names */
		for(i = 0 ; i < number_of_markers ; i++)
		{
			marker_index = 0;
			j = 0;
			while ((j < markers->number_of_markers) &&
				(!fuzzy_string_compare_same_length(marker_names[i],
				markers->marker_names[j])))
			{
				if (markers->marker_indices[j] >= marker_index)
				{
					marker_index = markers->marker_indices[j] + 1;
				}
				j++;
			}
			if (j < markers->number_of_markers)
			{
#if defined (MANUAL_CMISS)
				display_message(PF_ERROR_MESSAGE,
					"Marker name \"%s\" already defined for current model,"
					" respecifying 3D position", marker_names[i]);
#endif /* defined (MANUAL_CMISS) */
				markers->marker_positions[3*j]=marker_3d_generic_positions[3 * i];
				markers->marker_positions[3*j+1]=marker_3d_generic_positions[3 * i + 1];
				markers->marker_positions[3*j+2]=marker_3d_generic_positions[3 * i + 2];
			}
			else
			{
				if (REALLOCATE(new_marker_names,markers->marker_names,char *,
					markers->number_of_markers+1)&&
					REALLOCATE(new_marker_indices,markers->marker_indices,int,
					markers->number_of_markers + 1)&&
					REALLOCATE(new_marker_positions,markers->marker_positions,float,
					3*(markers->number_of_markers+1))&&
					ALLOCATE(new_marker_name,char,strlen(marker_names[i])+1))
				{
					markers->marker_names = new_marker_names;
					markers->marker_indices = new_marker_indices;
					markers->marker_positions = new_marker_positions;
					new_marker_names[markers->number_of_markers] = new_marker_name;
					strcpy(new_marker_name, marker_names[i]);
					new_marker_indices[markers->number_of_markers] = marker_index;
					marker_index++;
					new_marker_positions[3 * markers->number_of_markers] = 
						marker_3d_generic_positions[3 * i];
					new_marker_positions[3 * markers->number_of_markers + 1] = 
						marker_3d_generic_positions[3 * i + 1];
					new_marker_positions[3 * markers->number_of_markers + 2] = 
						marker_3d_generic_positions[3 * i + 2];
					markers->number_of_markers++;
				}
				else
				{
#if defined (MANUAL_CMISS)
					display_message(PF_ERROR_MESSAGE,
						"Problem with reallocating marker storage");
#endif /* defined (MANUAL_CMISS) */
					return_code = PF_ALLOCATE_FAILURE_RC;
				}
			}
		}
	}
	else
	{
#if defined (MANUAL_CMISS)
		display_message(PF_ERROR_MESSAGE, "Unable to read markers file");
#endif /* defined (MANUAL_CMISS) */
		return_code = PF_READ_FILE_FAILURE_RC;
	}
	LEAVE;

	return (return_code);
} /* pf_define_markers */

int pf_specify_markers(int number_of_markers,char **marker_names,
	float *marker_2d_positions,float *marker_confidences)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
Specify <number_of_markers> using
<marker_names> an array of <number_of_markers> marker names.  The marker names
	will come from Ben's XML specification or MPEG4.  A particular model may not
	have material positions for all markers, in which case the markers it doesn't
	know about will be ignored.
<marker_2d_positions> an array of 2*<number_of_markers> floats giving the
	measured locations for the markers on the particular image (marker number
	varying slowest).
<marker_confidences> an array of <number_of_markers> positive floats giving
	the relative confidences for the marker positions.  For the two markers the
	one with the larger confidence has a better position estimate.
==============================================================================*/
{
	char *filename;
	float *marker_positions, *marker_ndc_positions;
	int i, j, *marker_indices, return_code;

	ENTER(pf_specify_markers);
	return_code=PF_GENERAL_FAILURE_RC;
	if (ALLOCATE(filename,char,strlen(photoface_windows_path)+50))
	{
		/* If we haven't already done so read the marker definition file */
		if (!markers)
		{
			sprintf(filename, "%sworking/source.markers", photoface_windows_path);
			markers = read_marker_file(filename);
		}
		if (markers)
		{
			ALLOCATE(marker_indices,int,number_of_markers);
			ALLOCATE(marker_ndc_positions,float,number_of_markers*2);
			ALLOCATE(marker_positions,float,number_of_markers*3);
			if (marker_indices&&marker_ndc_positions&&marker_positions)
			{
				/* Find the node number equivalents for each of the marker names */
				for(i = 0 ; i < number_of_markers ; i++)
				{
					/* Zero indicates that it is not defined and so is the default */
					marker_indices[i] = 0;
					marker_positions[3 * i] = 0.0;
					marker_positions[3 * i + 1] = 0.0;
					marker_positions[3 * i + 2] = 0.0;

					j = 0;
					while ((j < markers->number_of_markers) &&
						(!fuzzy_string_compare_same_length(marker_names[i],
						markers->marker_names[j])))
					{
						j++;
					}
					if (j < markers->number_of_markers)
					{
						marker_indices[i] = markers->marker_indices[j];
						marker_ndc_positions[2 * i] = marker_2d_positions[2 * j];
						marker_ndc_positions[2 * i + 1] = marker_2d_positions[2 * j + 1];
						marker_positions[3 * i] = markers->marker_positions[3 * j];
						marker_positions[3 * i + 1] = markers->marker_positions[3 * j + 1];
						marker_positions[3 * i + 2] = markers->marker_positions[3 * j + 2];
					}
#if defined (MANUAL_CMISS)
					else
					{
						display_message(PF_ERROR_MESSAGE,
							"Marker name \"%s\" not defined for current model,"
							" ignoring 2D position", marker_names[i]);
					}
#endif /* defined (MANUAL_CMISS) */
				}
				/* Write out the placed_2d_points.exnode that specifies the these 2D
					positions for calculate_ptm and fit.pl in ndc coordinates */
				sprintf(filename,"%sworking/specified_2D_positions.exnode",
					photoface_windows_path);
				convert_2d_to_ndc(number_of_markers, marker_2d_positions, marker_ndc_positions);
				if (!(return_code=write_exnode(filename,"fiducial","ndc_coordinates",
					2, number_of_markers, marker_indices, marker_ndc_positions)))
				{
					sprintf(filename, "%sworking/specified_marker_confidence.exnode",
						photoface_windows_path);
					if (!(return_code=write_exnode(filename,"fiducial",
						"marker_confidence",1,number_of_markers,marker_indices,
						marker_confidences)))
					{
						/* Write out the 3D model positions for these nodes */
						sprintf(filename, "%sworking/source_coordinates.exnode",
							photoface_windows_path);
						if (!(return_code=write_exnode(filename,"fiducial",
							"marker_coordinates",3,number_of_markers,marker_indices,
							marker_positions)))
						{
							/* Write the same values as the target_coordinates as we just want
								to define the field for all these nodes and then cmgui will
								overwrite the actual target positions */
							sprintf(filename, "%sworking/target_coordinates.exnode",
								photoface_windows_path);
							if (!(return_code = write_exnode(filename, "fiducial", 
								"target_coordinates", 3, number_of_markers, marker_indices, 
								marker_positions)))
							{
								/* Define these fields and nodes */
#if defined (MANUAL_CMISS)
								if (display_message(PF_INFORMATION_MESSAGE,
#else /* defined (MANUAL_CMISS) */
								if (linux_execute(
#endif /* defined (MANUAL_CMISS) */
									"cmgui_control 'open comfile %scmiss/pf_specify_markers.com exec'",
									photoface_linux_path))
								{
									return_code=PF_SUCCESS_RC;
								}
							}
							else
							{
#if defined (MANUAL_CMISS)
								display_message(PF_ERROR_MESSAGE,"Unable to write file %s",
									filename);
#endif /* defined (MANUAL_CMISS) */
								return_code = PF_WRITE_FILE_FAILURE_RC;
							}
						}
						else
						{
#if defined (MANUAL_CMISS)
							display_message(PF_ERROR_MESSAGE,"Unable to write file %s",
								filename);
#endif /* defined (MANUAL_CMISS) */
							return_code = PF_WRITE_FILE_FAILURE_RC;
						}
					}
					else
					{
#if defined (MANUAL_CMISS)
						display_message(PF_ERROR_MESSAGE,"Unable to write file %s",
							filename);
#endif /* defined (MANUAL_CMISS) */
						return_code = PF_WRITE_FILE_FAILURE_RC;
					}
				}
				else
				{
#if defined (MANUAL_CMISS)
					display_message(PF_ERROR_MESSAGE, "Unable to write file %s",
						filename);
#endif /* defined (MANUAL_CMISS) */
					return_code = PF_WRITE_FILE_FAILURE_RC;
				}
			}
			else
			{
#if defined (MANUAL_CMISS)
				display_message(PF_ERROR_MESSAGE,
					"Unable to allocate marker indices array");
#endif /* defined (MANUAL_CMISS) */
				return_code = PF_ALLOCATE_FAILURE_RC;
			}
			DEALLOCATE(marker_indices);
			DEALLOCATE(marker_ndc_positions);
			DEALLOCATE(marker_positions);
		}
		else
		{
#if defined (MANUAL_CMISS)
			display_message(PF_ERROR_MESSAGE,"Unable to read markers file");
#endif /* defined (MANUAL_CMISS) */
			return_code = PF_READ_FILE_FAILURE_RC;
		}
		DEALLOCATE(filename);
	}
	else
	{
#if defined (MANUAL_CMISS)
		display_message(PF_ERROR_MESSAGE,"Unable to allocate filename string");
#endif /* defined (MANUAL_CMISS) */
		return_code = PF_ALLOCATE_FAILURE_RC;
	}
	LEAVE;

	return (return_code);
} /* pf_specify_markers */

int pf_get_marker_fitted_positions(int number_of_markers,char **marker_names,
	float *marker_fitted_3d_positions)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
Returns the fitted positions of the markers in <marker_fitted_3d_positions>
which is assumed to be allocated large enough for 3*<number_of_markers> floats
(marker number varying slowest).
==============================================================================*/
{
	char *filename, buffer[MARKER_BUFFER_SIZE];
	FILE *warped_file;
	float *marker_positions, marker_x, marker_y, marker_z;
	int i, j, *marker_indices, return_code;

	ENTER(pf_get_marker_fitted_positions);
	return_code=PF_GENERAL_FAILURE_RC;
	if (ALLOCATE(filename,char,strlen(photoface_windows_path)+50))
	{
		/* If we haven't already done so read the marker definition file */
		if (!markers)
		{
			sprintf(filename, "%sworking/source.markers", photoface_windows_path);
			markers = read_marker_file(filename);
		}
		if (markers)
		{
			if (ALLOCATE(marker_indices,int,number_of_markers)&&
				ALLOCATE(marker_positions,float,number_of_markers*3))
			{
				/* Work out which marker names correspond to which nodes */
				for(i = 0 ; i < number_of_markers ; i++)
				{
					/* Zero indicates that it is not defined and so is the default */
					marker_indices[i] = 0;
					marker_positions[3 * i] = 0.0;
					marker_positions[3 * i + 1] = 0.0;
					marker_positions[3 * i + 2] = 0.0;

					j = 0;
					while ((j < markers->number_of_markers) &&
						(!fuzzy_string_compare_same_length(marker_names[i],
						markers->marker_names[j])))
					{
						j++;
					}
					if (j < markers->number_of_markers)
					{
						marker_indices[i] = markers->marker_indices[j];
						marker_positions[3 * i] = markers->marker_positions[3 * j];
						marker_positions[3 * i + 1] = markers->marker_positions[3 * j + 1];
						marker_positions[3 * i + 2] = markers->marker_positions[3 * j + 2];
					}
#if defined (MANUAL_CMISS)
					else
					{
						display_message(PF_WARNING_MESSAGE,
							"Marker name \"%s\" not defined for current model, ignoring",
							marker_names[i]);
					}
#endif /* defined (MANUAL_CMISS) */
				}

				/* Write out the original.exnode that specifies the the original
					3D positions for fit.pl */
				sprintf(filename, "%sworking/original.exnode", photoface_windows_path);
				if (!(return_code = write_exnode(filename, "get_marker_positions", 
					"marker_coordinates",
					3, number_of_markers, marker_indices, marker_positions)))
				{
					/* Define these fields and nodes */
#if defined (MANUAL_CMISS)
					if (display_message(PF_INFORMATION_MESSAGE,
#else /* defined (MANUAL_CMISS) */
					if (linux_execute(
#endif /* defined (MANUAL_CMISS) */
						"cmgui_control 'open comfile %scmiss/pf_get_marker_fitted_positions.com exec'",
						photoface_linux_path))
					{
						return_code=PF_SUCCESS_RC;
					}
					/* Read the generated exnode file and return the results */
					sprintf(filename, "%sworking/warped.00001.exnode",
						photoface_windows_path);
					if (warped_file = fopen(filename, "r"))
					{
						i = 0;
						return_code = PF_SUCCESS_RC;
						while ((return_code == PF_SUCCESS_RC) && (!feof(warped_file)) && 
							(!read_line(warped_file, buffer, MARKER_BUFFER_SIZE)))
						{
							if (3 == sscanf(buffer, "%f%f%f", &marker_x, &marker_y,
								&marker_z))
							{
								if (i >= number_of_markers)
								{
#if defined (MANUAL_CMISS)
									display_message(PF_ERROR_MESSAGE,
										"Unexpected marker positions in file %s",filename);
#endif /* defined (MANUAL_CMISS) */
									return_code = PF_READ_FILE_FAILURE_RC;
								}
								marker_fitted_3d_positions[3 * i] = marker_x;
								marker_fitted_3d_positions[3 * i + 1] = marker_y;
								marker_fitted_3d_positions[3 * i + 2] = marker_z;
								i++;
							}
						}
						if (i < number_of_markers)
						{
#if defined (MANUAL_CMISS)
							display_message(PF_ERROR_MESSAGE,
								"Insufficient marker positions in file %s",filename);
#endif /* defined (MANUAL_CMISS) */
							return_code = PF_READ_FILE_FAILURE_RC;
						}
						fclose(warped_file);
					}
					else
					{
#if defined (MANUAL_CMISS)
						display_message(PF_ERROR_MESSAGE,
							"Unable to open warped positions file %s",filename);
#endif /* defined (MANUAL_CMISS) */
						return_code = PF_OPEN_FILE_FAILURE_RC;
					}
				}
				else
				{
#if defined (MANUAL_CMISS)
					display_message(PF_ERROR_MESSAGE,
						"Unable to write original positions");
#endif /* defined (MANUAL_CMISS) */
					return_code = PF_WRITE_FILE_FAILURE_RC;
				}
				DEALLOCATE(marker_indices);
				DEALLOCATE(marker_positions);
			}
			else
			{
#if defined (MANUAL_CMISS)
				display_message(PF_ERROR_MESSAGE,
					"Unable to allocate marker indices array");
#endif /* defined (MANUAL_CMISS) */
				return_code = PF_ALLOCATE_FAILURE_RC;
			}
		}
		else
		{
#if defined (MANUAL_CMISS)
			display_message(PF_ERROR_MESSAGE,"Unable to read markers file");
#endif /* defined (MANUAL_CMISS) */
			return_code = PF_READ_FILE_FAILURE_RC;
		}
	}
	else
	{
#if defined (MANUAL_CMISS)
		display_message(PF_ERROR_MESSAGE,"Unable to allocate filename string");
#endif /* defined (MANUAL_CMISS) */
		return_code = PF_ALLOCATE_FAILURE_RC;
	}
	LEAVE;

	return (return_code);
} /* pf_get_marker_fitted_positions */

int pf_view_align(float *error_measure)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
Calculates the view that aligns the model to the specified markers and returns
an <error_measure>.
==============================================================================*/
{
	int return_code;

	ENTER(pf_view_align);
	return_code=PF_GENERAL_FAILURE_RC;
	/* For now this is all done by the comfile, until we have more direct control
		over cmgui */
#if defined (MANUAL_CMISS)
	if (display_message(PF_INFORMATION_MESSAGE,
#else /* defined (MANUAL_CMISS) */
	if (linux_execute(
#endif /* defined (MANUAL_CMISS) */
		"cmgui_control 'open comfile %scmiss/pf_view_align.com exec'",
		photoface_linux_path))
	{
		return_code=PF_SUCCESS_RC;
	}
	/* Run the filt program calculate_ptm */
	/* Load the projection into cmgui so that the fields are updated relative to
		it */
	LEAVE;

	return (return_code);
} /* pf_view_align */

int pf_get_view(float *eye_point,float *interest_point,float *up_vector,
	float *view_angle)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
Returns the current view as an <eye_point> (3 component vector), an
<interest_point> (3 component vector), an <up_vector> (3 component vector) and a
<view_angle> (scalar).  Assumes that all storage has been assigned large enough.
==============================================================================*/
{
#define GET_VIEW_LINE_LIMIT (500)
	char filename[100], line[GET_VIEW_LINE_LIMIT];
	FILE *window_commands_file;
	int return_code;

	ENTER(pf_get_view);
	return_code=PF_GENERAL_FAILURE_RC;
	/* Get the current viewing parameters from cmgui */
	/* Export the window projection from cmgui to a file */
#if defined (MANUAL_CMISS)
	if (display_message(PF_INFORMATION_MESSAGE,
#else /* defined (MANUAL_CMISS) */
	if (linux_execute(
#endif /* defined (MANUAL_CMISS) */
		"cmgui_control 'open comfile %scmiss/pf_get_view.com exec'",
		photoface_linux_path))
	{
		return_code=PF_SUCCESS_RC;
	}
	sprintf(filename, "%sworking/window_placement.com", photoface_windows_path);
	if (window_commands_file = fopen(filename, "r"))
	{
		return_code= PF_READ_FILE_FAILURE_RC;
		while ((return_code == PF_READ_FILE_FAILURE_RC) 
			&& (!read_line (window_commands_file, line, GET_VIEW_LINE_LIMIT)))
		{
			if (10 == sscanf(line, "gfx modify window 1 view perspective eye_point %f %f %f "
				"interest_point %f %f %f up_vector %f %f %f view_angle %f",
				eye_point, eye_point + 1, eye_point + 2,
				interest_point, interest_point + 1, interest_point + 2,
				up_vector, up_vector + 1, up_vector + 2, view_angle))
			{
				return_code = PF_SUCCESS_RC;
			}
		}
		fclose(window_commands_file);
#if defined (MANUAL_CMISS)
		if (return_code == PF_READ_FILE_FAILURE_RC)
		{
			display_message(PF_ERROR_MESSAGE,
				"Unable to find the window projection command in %s",filename);
		}
#endif /* defined (MANUAL_CMISS) */
	}
	else
	{
#if defined (MANUAL_CMISS)
		display_message(PF_ERROR_MESSAGE, "Unable to open file %s for reading",
			filename);
#endif /* defined (MANUAL_CMISS) */
		return_code= PF_OPEN_FILE_FAILURE_RC;
	}
	LEAVE;

	return (return_code);
} /* pf_get_view */

int pf_specify_view(float *eye_point,float *interest_point,float *up_vector,
	float view_angle)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
Sets the current view as an <eye_point> (3 component vector), an
<interest_point> (3 component vector), an <up_vector> (3 component vector) and a
<view_angle> (scalar).  It is an alternative/override for pf_view_align.
==============================================================================*/
{
	int return_code;

	ENTER(pf_specify_view);
	return_code=PF_GENERAL_FAILURE_RC;
	/* Set the viewing parameters in cmgui */
#if defined (MANUAL_CMISS)
	if (display_message(PF_INFORMATION_MESSAGE,
#else /* defined (MANUAL_CMISS) */
	if (linux_execute(
#endif /* defined (MANUAL_CMISS) */
		"cmgui_control 'gfx modify window 1 view eye %f %f %f "
		"interest %f %f %f up %f %f %f view_angle %f'",
		eye_point[0], eye_point[1], eye_point[2],
		interest_point[0], interest_point[1], interest_point[2],
		up_vector[0], up_vector[1], up_vector[2], view_angle))
	{
		return_code=PF_SUCCESS_RC;
	}
	LEAVE;

	return (return_code);
} /* pf_specify_view */

int pf_fit(float *error_measure)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
Fits the model to the specified markers, using the current transformation
matrix, and returns an <error_measure>.
==============================================================================*/
{
	int return_code;

	ENTER(pf_fit);
	return_code=PF_GENERAL_FAILURE_RC;
	/* For now this is all done by the comfile, until we have more direct
		control over cmgui */
#if defined (MANUAL_CMISS)
	if (display_message(PF_INFORMATION_MESSAGE,
#else /* defined (MANUAL_CMISS) */
	if (linux_execute(
#endif /* defined (MANUAL_CMISS) */
		"cmgui_control 'open comfile %scmiss/pf_fit.com exec'",
		photoface_linux_path))
	{
		return_code=PF_SUCCESS_RC;
	}

	/* Use cmgui to calculate 3D positions for markers where the 2D point is 
		specified and write these out */

	/* Run fit.pl */

	/* Load the fitted model into cmgui */

	/* Place the eyes */

	LEAVE;

	return (return_code);
} /* pf_fit */

int pf_get_head_model(int *number_of_vertices,float **vertex_3d_locations,
	int *number_of_texture_vertices,float **texture_vertex_3d_locations,
	int *number_of_triangles,int **triangle_vertices,
	int **triangle_texture_vertices)
/*******************************************************************************
LAST MODIFIED : 13 February 2001

DESCRIPTION :
Returns the current transformed generic head as
<vertex_3d_locations> a 1-D array of 3*<number_of_vertices> floats specifying
	the world locations of the vertices (vertex number varying slowest)
<texture_vertex_3d_locations> a 1-D array of 3*<number_of_texture_vertices>
	floats specifying the texture locations of the texture vertices (vertex number
	varying slowest)
<triangle_vertices> a 1-D array of 3*<number_of_triangles> ints giving the
	vertex numbers for each triangle
<triangle_texture_vertices> a 1-D array of 3*<number_of_triangles> ints giving
	the texture vertex numbers for each triangle
==============================================================================*/
{
	char filename[100];
	int return_code;
	struct Obj *obj;

	ENTER(pf_get_head_model);
	return_code=PF_SUCCESS_RC;
	/* Read the fitted obj file and return all the values */
	sprintf(filename, "%sworking/target.fit.obj", photoface_windows_path);
	if(obj = read_obj(filename))
	{
		*number_of_vertices = obj->number_of_vertices;
		*vertex_3d_locations = obj->vertex_3d_locations;
		*number_of_texture_vertices = obj->number_of_texture_vertices;
		*texture_vertex_3d_locations = obj->texture_vertex_3d_locations;
		*number_of_triangles = obj->number_of_triangles;
		*triangle_vertices = obj->triangle_vertices;
		*triangle_texture_vertices = obj->triangle_texture_vertices;
		DEALLOCATE(obj);
		return_code=PF_SUCCESS_RC;
	}
	else
	{
		return_code= PF_OPEN_FILE_FAILURE_RC;
	}
	LEAVE;

	return (return_code);
} /* pf_get_head_model */

int pf_get_basis(int *number_of_modes,int *number_of_vertices,
	float **vertex_3d_locations_or_offsets)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
Returns the basis for the current transformed model in
<vertex_3d_locations_or_offsets> which is a 1-D array of
3*<number_of_modes>*<number_of_vertices> floats with x,y,z varying fastest and
mode number fastest.
==============================================================================*/
{
	char filename[200];
	int return_code;

	ENTER(pf_get_basis);
	return_code=PF_GENERAL_FAILURE_RC;
	/* Calculate the basis based on the current host mesh transformation
		??? (or do this as a separate step, or as part of doing the fit) */
#if defined (MANUAL_CMISS)
	if (display_message(PF_INFORMATION_MESSAGE,
#else /* defined (MANUAL_CMISS) */
	if (linux_execute(
#endif /* defined (MANUAL_CMISS) */
		"cmgui_control 'open comfile %scmiss/pf_get_basis.com exec'",
		photoface_linux_path))
	{
		return_code=PF_SUCCESS_RC;
	}

	/* Read the fitted basis file and return all the values */
	sprintf(filename, "%sworking/target.fit.basis", photoface_windows_path);
	if (0 == (return_code = read_basis_version1and2(filename, number_of_vertices,
		number_of_modes, vertex_3d_locations_or_offsets, 1)))
	{
		*number_of_vertices /= 3; /* Each component is a separate number */
		return_code=PF_SUCCESS_RC;
	}
	else
	{
		return_code=PF_OPEN_FILE_FAILURE_RC;
	}
	LEAVE;

	return (return_code);
} /* pf_get_basis */

#define CMGUI_BUG_PAD_HACK

int pf_specify_image(int width,int height,enum PF_image_format image_format,
	char *image)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
Used to specify the image to be texture mapped onto the model.
==============================================================================*/
{
	char *image_ptr, filename[200];
	float texture_ndc_x, texture_ndc_y, texture_ndc_width, texture_ndc_height;
	FILE *image_file, *image_comfile;
	int i, return_code;

	ENTER(pf_specify_image);
	return_code=PF_GENERAL_FAILURE_RC;
	/* Create the rgb image for this image and make the image square by padding */
	sprintf(filename, "%sworking/source_image.raw", photoface_windows_path);
	if (image_file = fopen(filename, "wb"))
	{
#if defined CMGUI_BUG_PAD_HACK
		{
			char padding[3] = {(char)255,(char)255,(char)255};
			int j, pad;

			if (width % 4)
			{
				pad = 4 - (width % 4);
			}
			else
			{
				pad = 0;
			}
			image_ptr = image;
			for (i = 0 ; i < height ; i++)
			{
				for (j = 0 ; j < width ; j++)
				{
					fwrite(image_ptr, 1, 3, image_file);
					image_ptr += 3;
				}
				for (j = 0 ; j < pad ; j++)
				{
					fwrite(padding, 1, 3, image_file);		
				}
			}
			fclose(image_file);
			if (pad)
			{
				width += pad;
			}
		}
#else /* defined CMGUI_BUG_PAD_HACK */
		image_ptr = image;
		for (i = 0 ; i < width * height ; i++)
		{
			fwrite(image_ptr, 1, 3, image_file);
			image_ptr += 3;
		}
		fclose(image_file);
#endif /* defined CMGUI_BUG_PAD_HACK */

		/* Calculate the texture placement coordinates in ndc space */
		texture_ndc_x = - ndc_texture_offset * ndc_texture_scaling - 1.0f;
		texture_ndc_y = ndc_texture_offset * ndc_texture_scaling - 1.0f;
		texture_ndc_width = ((float)width - ndc_texture_offset) * ndc_texture_scaling - 1.0f - texture_ndc_x;
		/* OK This appears to be insane, i.e. texture_ndc_x is used where you might expect texture_ndc_y but that
			 is because the y position is already mangled */
		texture_ndc_height = ((float)height - ndc_texture_offset) * ndc_texture_scaling - 1.0f - texture_ndc_x;

		sprintf(filename, "%sworking/pf_specify_image.com", photoface_windows_path);
		if (image_comfile = fopen(filename, "w"))
		{
			fprintf(image_comfile, "gfx modify texture source_image image %sworking/source_image.raw specify_width %d raw_interleaved\n",
				photoface_linux_path, width);
			fprintf(image_comfile, "gfx modify window 1 background tex_placement %f %f %f %f\n",
				texture_ndc_x, texture_ndc_y, texture_ndc_width, texture_ndc_height);

			fclose(image_comfile);

#if defined (MANUAL_CMISS)
			if (display_message(PF_INFORMATION_MESSAGE,
#else /* defined (MANUAL_CMISS) */
			if (linux_execute(
#endif /* defined (MANUAL_CMISS) */
				"cmgui_control 'open comfile %sworking/pf_specify_image.com exec'",
				photoface_linux_path))
			{
				return_code=PF_SUCCESS_RC;
			}
		}
		else
		{
			return_code=PF_OPEN_FILE_FAILURE_RC;
		}
	}
	else
	{
		return_code=PF_OPEN_FILE_FAILURE_RC;
	}
	LEAVE;

	return (return_code);
} /* pf_specify_image */

int pf_get_texture(int width,int height,char *texture)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
The caller specifies the texture size and provides the storage.  The <texture>
is filled in based on the current model.
==============================================================================*/
{
	char filename[200], *src_image_ptr, *dest_image_ptr;
	FILE *texture_comfile;
	int i, number_of_bytes_per_component, number_of_components, return_code;
	long int file_height, file_width;
	long unsigned *image;

	ENTER(pf_get_texture);
	return_code=PF_GENERAL_FAILURE_RC;
	/* Use cmgui to calculate the texture based on the specified image */
	sprintf(filename, "%sworking/pf_get_texture.com", photoface_windows_path);
	if (texture_comfile = fopen(filename, "w"))
	{
		fprintf(texture_comfile, "$width = %d\n", width);
		fprintf(texture_comfile, "$height = %d\n", height);
		fprintf(texture_comfile, "gfx mod win 1 back texture source_image\n");
		fprintf(texture_comfile, "gfx def field projection window_projection field coordinates win 1\n");
		fprintf(texture_comfile, "gfx def field mapped_texture sample_texture coord projection texture source_image\n");
#if defined (BACKWARD_PROJECTION)
		/* This is the original precise texture calculation */
		fprintf(texture_comfile, "gfx modify texture face_mapped width 1 height 1 evaluate_image field mapped_texture spectrum rgba_spectrum width $width height $height texture_coord texture element_group objface format rgba\n");
		fprintf(texture_comfile, "gfx write texture face_mapped file %sworking/target.fit.rgb rgb\n",
			photoface_linux_path);
#else /* defined (BACKWARD_PROJECTION) */
		/* This is the projection just involving drawing the image in texture space which is 
			faster with a potentially small degradation in quality */
		/* Use a single buffer window to get the full colour depth if it draws onscreen */
		fprintf(texture_comfile, "gfx create material source_image texture source_image\n");
		fprintf(texture_comfile, "gfx create scene texture_projection manual\n");
		fprintf(texture_comfile, "gfx set vis axes off scene texture_projection\n");
		fprintf(texture_comfile, "gfx draw group objface scene texture_projection\n");
		/* Dither the graphics object around so that the colours near the edges bleed into
			the holes and so there are no blank pixels on our texture */
		fprintf(texture_comfile, "gfx draw group objface scene texture_projection as objface2\n");
		fprintf(texture_comfile, "gfx set transformation name objface2 scene texture_projection 1 0 0 0 0 1 0 0 0 0 1 0 0.001 0.001 -1 1\n");		
		fprintf(texture_comfile, "gfx draw group objface scene texture_projection as objface3\n");
		fprintf(texture_comfile, "gfx set transformation name objface3 scene texture_projection 1 0 0 0 0 1 0 0 0 0 1 0 -0.001 0.001 -2 1\n");		
		fprintf(texture_comfile, "gfx draw group objface scene texture_projection as objface4\n");
		fprintf(texture_comfile, "gfx set transformation name objface4 scene texture_projection 1 0 0 0 0 1 0 0 0 0 1 0 0.001 -0.001 -3 1\n");		
		fprintf(texture_comfile, "gfx draw group objface scene texture_projection as objface5\n");
		fprintf(texture_comfile, "gfx set transformation name objface5 scene texture_projection 1 0 0 0 0 1 0 0 0 0 1 0 -0.001 -0.001 -4 1\n");		
		fprintf(texture_comfile, "gfx modify g_element objface surfaces coordinate texture select_on material source_image texture_coordinates projection selected_material default_selected render_shaded scene texture_projection\n");

		fprintf(texture_comfile, "gfx create window texture_projection single\n");
		fprintf(texture_comfile, "gfx modify window texture_projection layout 2d ortho_axes z -y width $width height $height\n");
		fprintf(texture_comfile, "gfx modify window texture_projection image scene texture_projection\n");
		fprintf(texture_comfile, "gfx modify window texture_projection view parallel eye_point 0.5 0.5 3 interest_point 0.5 0.5 0 up_vector 0.00580574 0.999983 0 view_angle 26.525435202 near_clipping_plane 0.0288485 far_clipping_plane 10.3095 relative_viewport ndc_placement -1 -1 2 2 viewport_coordinates -1 -1 400 400\n");
		fprintf(texture_comfile, "gfx print window texture_projection rgb file %sworking/target.fit.rgb width $width height $height\n",
			photoface_linux_path);
		fprintf(texture_comfile, "gfx modify texture face_mapped image %sworking/target.fit.rgb\n",
			photoface_linux_path);
#endif /* defined (BACKWARD_PROJECTION) */
		fprintf(texture_comfile, "open comfile %scmiss/pf_make_standin_texture.com exec\n",
			photoface_linux_path);
		fprintf(texture_comfile, "gfx modify material skin texture face_mapped\n");
		fprintf(texture_comfile, "gfx modify g_element objface surfaces select_on material skin texture_coord texture\n");
		fprintf(texture_comfile, "gfx mod win 1 back texture none\n");
		fclose(texture_comfile);
#if defined (MANUAL_CMISS)
		if (display_message(PF_INFORMATION_MESSAGE,
#else /* defined (MANUAL_CMISS) */
		if (linux_execute(
#endif /* defined (MANUAL_CMISS) */
			"cmgui_control 'open comfile %sworking/pf_get_texture.com exec'",
			photoface_linux_path))
		{
			return_code=PF_SUCCESS_RC;
		}

		sprintf(filename, "%sworking/target.fit.rgb", photoface_windows_path);
		if (read_rgb_image_file(filename, &number_of_components,
			&number_of_bytes_per_component, &file_height, &file_width, &image))
		{
			if ((number_of_bytes_per_component == 1) && (number_of_components > 2)
				&& (number_of_components < 5))
			{
				if ((file_width == width) && (file_height == height))
				{
					src_image_ptr = (char *) image;
					dest_image_ptr = texture;
					for (i = 0 ; i < width * height ; i++)
					{
						*dest_image_ptr = *src_image_ptr;
						*(dest_image_ptr + 1) = *(src_image_ptr + 1);
						*(dest_image_ptr + 2) = *(src_image_ptr + 2);
						src_image_ptr += number_of_components;
						dest_image_ptr += 3;
					}
					return_code=PF_SUCCESS_RC;
				}
				else
				{
#if defined (MANUAL_CMISS)
					display_message(PF_ERROR_MESSAGE,
						"File image dimensions are not what was expected in file %s",
						filename);
#endif /* defined (MANUAL_CMISS) */
					return_code=PF_GENERAL_FAILURE_RC;
				}
			}
			else
			{
#if defined (MANUAL_CMISS)
				display_message(PF_ERROR_MESSAGE, "Unsupported image format in file %s",
					filename);
#endif /* defined (MANUAL_CMISS) */
				return_code=PF_GENERAL_FAILURE_RC;
			}
		}
		else
		{
#if defined (MANUAL_CMISS)
			display_message(PF_ERROR_MESSAGE, "Unable to read image from file %s",
				filename);
#endif /* defined (MANUAL_CMISS) */
			return_code=PF_READ_FILE_FAILURE_RC;
		}
	}
	else
	{
		return_code=PF_OPEN_FILE_FAILURE_RC;
	}
	LEAVE;

	return (return_code);
} /* pf_get_texture */
