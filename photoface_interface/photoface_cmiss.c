/*******************************************************************************
FILE : photoface_cmiss.c

LAST MODIFIED : 16 February 2001

DESCRIPTION :
The functions that interface Photoface to cmiss.  All functions have an integer
return code - zero is success, non-zero is failure.
==============================================================================*/
#include <errno.h>
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

#define PF_LOCK_DIRNAME "pflock"
#define PF_STRUCTURE_FILENAME "pf_job.struct"

#define WITH_OMNISECURE

#define MAX_ATTEMPTS_AT_FIRST_CMISS_CONTROL (50)

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

struct Pf_job
{
	int pf_job_id;
	char *working_path;
	char *remote_working_path;
	float ndc_texture_scaling, ndc_texture_offset_x, ndc_texture_offset_y;
}; /* struct Pf_job */
/*
Module variables
----------------
*/
static char *photoface_remote_path = NULL,*photoface_local_path = NULL;
struct Marker_struct *markers;

#define COMMAND_STRING_SIZE 10000
static char command_string[COMMAND_STRING_SIZE];
static char command_string2[COMMAND_STRING_SIZE];

#if defined (ERROR_MESSAGE)
#define ERROR_MESSAGE_SIZE 10000
static char error_message[ERROR_MESSAGE_SIZE] = "";

enum PF_message_type
{
	PF_INFORMATION_MESSAGE,
	PF_WARNING_MESSAGE,
	PF_ERROR_MESSAGE
};
#endif /* defined (ERROR_MESSAGE) */

/*
Module functions
----------------
*/
#if defined (ERROR_MESSAGE)
static int set_error_message(enum PF_message_type message_type,char *format, ... )
/*******************************************************************************
LAST MODIFIED : 19 October 2001

DESCRIPTION :
A function for setting an error message of the specified <message_type>.  The printf
form of arguments is used.
==============================================================================*/
{
	int offset, return_code;
	va_list ap;

	ENTER(set_error_message);
	va_start(ap,format);
	switch (message_type)
	{
		case PF_ERROR_MESSAGE:
		{
			strcpy (error_message, "ERROR: ");
		} break;
		case PF_INFORMATION_MESSAGE:
		{
			strcpy (error_message, "");
		} break;
		case PF_WARNING_MESSAGE:
		{
			strcpy (error_message, "WARNING: ");
		} break;
		default:
		{
			strcpy (error_message, "UNKNOWN: ");
		} break;
	}
	offset = strlen(error_message);

#if defined (WIN32)
	return_code= _vsnprintf(error_message+offset,ERROR_MESSAGE_SIZE-offset,format,ap);
#else /* defined (WIN32) */
	return_code= vsnprintf(error_message+offset,ERROR_MESSAGE_SIZE-offset,format,ap);
#endif /* defined (WIN32) */
	/*	return_code=vsprintf(error_message,format,ap); */
	if (return_code >= (ERROR_MESSAGE_SIZE-1-offset))
	{
		sprintf(error_message + ERROR_MESSAGE_SIZE - 50, 
			":Overflow of error string.  Truncated.");
	}
	va_end(ap);
	LEAVE;

	return (return_code);
} /* set_error_message */
#endif /* ERROR_MESSAGE */

static int linux_execute(char *format, ... )
/*******************************************************************************
LAST MODIFIED : 16 February 2001

DESCRIPTION :
A function for executing a command under linux.  The printf form of arguments is
used.
==============================================================================*/
{
	char *connection;
	int return_code;
	va_list ap;

	ENTER(linux_execute);
	return_code=0;
	va_start(ap,format);
	if (connection=getenv("PF_CMISS_CONNECTION"))
	{
//		sprintf(command_string,"\"c:\\program files\\putty\\plink\" %s setenv DISPLAY :3.0;",host);
//		sprintf(command_string,"\"c:\\program files\\putty\\plink\" %s setenv DISPLAY esu48:0.0;",host);
		strcpy(command_string, connection);
	}
	else
	{
		strcpy(command_string, "");
	}

#if ! defined (WIN32)
	if (connection)
	{
		sprintf(command_string+strlen(command_string)," '");
	}
#endif /* ! defined (WIN32) */

		vsprintf(command_string+strlen(command_string),format,ap);

#if ! defined (WIN32)
	if (connection)
	{
		sprintf(command_string+strlen(command_string)," '");
	}
#endif /* ! defined (WIN32) */

#if defined (DEBUG)
	printf (command_string);
	printf ("\n");
#endif /* defined (DEBUG) */

	return_code = system(command_string);
	/* Translate this into a internal return code */
	if (0==return_code)
	{
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}

	va_end(ap);
	LEAVE;

	return (return_code);
} /* linux_execute */

static int linux_execute2(char *format, ... )
/*******************************************************************************
LAST MODIFIED : 16 February 2001

DESCRIPTION :
A function for executing a command under linux.  The printf form of arguments is
used.
==============================================================================*/
{
	char *connection;
	int return_code;
	va_list ap;

	ENTER(linux_execute);
	return_code=0;
	va_start(ap,format);
	if (connection=getenv("PF_CMISS_CONNECTION"))
	{
//		sprintf(command_string2,"\"c:\\program files\\putty\\plink\" %s setenv DISPLAY :3.0;",host);
//		sprintf(command_string2,"\"c:\\program files\\putty\\plink\" %s setenv DISPLAY esu48:0.0;",host);
		strcpy(command_string2, connection);
	}
	else
	{
		strcpy(command_string2, "");
	}

#if ! defined (WIN32)
	if (connection)
	{
		sprintf(command_string2+strlen(command_string2)," '");
	}
#endif /* ! defined (WIN32) */

		vsprintf(command_string2+strlen(command_string2),format,ap);

#if ! defined (WIN32)
	if (connection)
	{
		sprintf(command_string2+strlen(command_string2)," '");
	}
#endif /* ! defined (WIN32) */

#if defined (DEBUG)
	printf (command_string2);
	printf ("\n");
#endif /* defined (DEBUG) */

	return_code = system(command_string2);
	/* Translate this into a internal return code */
	if (0==return_code)
	{
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}

	va_end(ap);
	LEAVE;

	return (return_code);
} /* linux_execute2 */

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
#if defined (ERROR_MESSAGE)
				set_error_message(PF_ERROR_MESSAGE,"read_line.  File IO error encountered.");
#endif /* defined (ERROR_MESSAGE) */
				return_code = PF_READ_FILE_FAILURE_RC;
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
#if defined (ERROR_MESSAGE)
		set_error_message(PF_ERROR_MESSAGE,"reduce_fuzzy_string.  Invalid arguments.");
#endif /* defined (ERROR_MESSAGE) */
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
#if defined (ERROR_MESSAGE)
				set_error_message(PF_ERROR_MESSAGE,"fuzzy_string_compare.  Error reducing");
#endif /* defined (ERROR_MESSAGE) */
				return_code=0;
			}
			DEALLOCATE(first_reduced);
			DEALLOCATE(second_reduced);
		}
		else
		{
			DEALLOCATE(first_reduced);
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE,
				"fuzzy_string_compare.  Unable to allocate memory.");
#endif /* defined (ERROR_MESSAGE) */
			return_code=0;
		}
	}
	else
	{
#if defined (ERROR_MESSAGE)
		set_error_message(PF_ERROR_MESSAGE,"fuzzy_string_compare.  Invalid arguments.");
#endif /* defined (ERROR_MESSAGE) */
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
#if defined (ERROR_MESSAGE)
				set_error_message(PF_ERROR_MESSAGE,
					"fuzzy_string_compare_same_length.  Error reducing.");
#endif /* defined (ERROR_MESSAGE) */
				return_code=0;
			}
			DEALLOCATE(first_reduced);
			DEALLOCATE(second_reduced);
		}
		else
		{
			DEALLOCATE(first_reduced);
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE,
				"fuzzy_string_compare_same_length.  Unable to allocate memory.");
#endif /* defined (ERROR_MESSAGE) */
			return_code=0;
		}
	}
	else
	{
#if defined (ERROR_MESSAGE)
		set_error_message(PF_ERROR_MESSAGE,
			"fuzzy_string_compare_same_length.  Invalid arguments");
#endif /* defined (ERROR_MESSAGE) */
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
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE,
				"copy_file.  Unable to open destination file %s",
				destination_filename);
#endif /* defined (ERROR_MESSAGE) */
			return_code= PF_OPEN_FILE_FAILURE_RC;
		}
		fclose(source);
	}
	else
	{
#if defined (ERROR_MESSAGE)
		set_error_message(PF_ERROR_MESSAGE,
			"copy_file.  Unable to open source file %s",source_filename);
#endif /* defined (ERROR_MESSAGE) */
		return_code= PF_OPEN_FILE_FAILURE_RC;
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
#if defined (ERROR_MESSAGE)
						set_error_message(PF_ERROR_MESSAGE,
							"Problem with reallocating marker storage");
#endif /* defined (ERROR_MESSAGE) */
					}
				}
			}
			fclose(marker_file);
		}
		else
		{
			DEALLOCATE(markers);
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE,"Unable to open marker file %s",
				filename);
#endif /* defined (ERROR_MESSAGE) */
		}
	}
#if defined (ERROR_MESSAGE)
	else
	{
		set_error_message(PF_ERROR_MESSAGE,
			"Unable to allocate memory for marker structure");
	}
#endif /* defined (ERROR_MESSAGE) */
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
#if defined (ERROR_MESSAGE)
		set_error_message(PF_ERROR_MESSAGE, "Unable to open file %s for writing",
			filename);
#endif /* defined (ERROR_MESSAGE) */
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
	int i, n_face_vertices, n_v = 1, n_t=0, n_vt=1, n_vn=1;
	int return_code;
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
							while (i < 3)
							{
								vt[3 * n_vt + i] = 0.0;
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
							while (i < 3)
							{
								vn[3 * n_vn + i] = 0.0;
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
							while (i < 3)
							{
								vt[3 * n_v + i] = 0.0;
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
#if defined (ERROR_MESSAGE)
								set_error_message(PF_ERROR_MESSAGE,
									"Only triangles are supported in the obj mesh: n_vertices %d",
									n_face_vertices);
#endif /* defined (ERROR_MESSAGE) */
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
							obj->triangle_texture_vertices[3 * i] = t[i * 9 + 0 * 3 + 1];
							obj->triangle_texture_vertices[3 * i + 1] = t[i * 9 + 1 * 3 + 1];
							obj->triangle_texture_vertices[3 * i + 2] = t[i * 9 + 2 * 3 + 1];
						}
						obj->number_of_triangles = n_t;
					}
					else
					{
#if defined (ERROR_MESSAGE)
						set_error_message(PF_ERROR_MESSAGE,"Unable to allocate obj fields");
#endif /* defined (ERROR_MESSAGE) */
						DEALLOCATE(obj);
					}
				}
			}
#if defined (ERROR_MESSAGE)
			else
			{
				set_error_message(PF_ERROR_MESSAGE, "Unable to allocate obj structure");
			}
#endif /* defined (ERROR_MESSAGE) */
			fclose(objfile);
		}
#if defined (ERROR_MESSAGE)
		else
		{
			set_error_message(PF_ERROR_MESSAGE, "Unable to open objfile %s",filename);
		}
#endif /* defined (ERROR_MESSAGE) */
		DEALLOCATE(v);
		DEALLOCATE(vn);
		DEALLOCATE(vt);
		DEALLOCATE(t);
	}
#if defined (ERROR_MESSAGE)
	else
	{
		set_error_message(PF_ERROR_MESSAGE,
			"Unable to allocate temporary vertex arrays");
	}
#endif /* defined (ERROR_MESSAGE) */
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

static int read_rgb_image_file(char *filename,
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
	if (filename&&number_of_components_address&&height_address&&width_address&&
		image_address)
	{
		if (image_file=fopen(filename,"rb"))
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
#if defined (ERROR_MESSAGE)
														set_error_message(PF_ERROR_MESSAGE,
													"read_rgb_image_file.  Error reading row component in %s.",
															filename);
#endif /* defined (ERROR_MESSAGE) */
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
#if defined (ERROR_MESSAGE)
														set_error_message(PF_ERROR_MESSAGE,
													"read_rgb_image_file.  Error reading row component in %s.",
															filename);
#endif /* defined (ERROR_MESSAGE) */
														return_code=0;
													}
												} break;
												default:
												{
#if defined (ERROR_MESSAGE)
													set_error_message(PF_ERROR_MESSAGE,
											"read_rgb_image_file.  Unsupported bytes per component in %s.",
														filename);
#endif /* defined (ERROR_MESSAGE) */
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
#if defined (ERROR_MESSAGE)
									set_error_message(PF_ERROR_MESSAGE,
										"read_rgb_image_file.  Could not allocate row array for %s.",
										filename);
#endif /* defined (ERROR_MESSAGE) */
									return_code=0;
								}
							}
							else
							{
#if defined (ERROR_MESSAGE)
								set_error_message(PF_ERROR_MESSAGE,
									"read_rgb_image_file.  Error reading row start/size in %s.",
									filename);
#endif /* defined (ERROR_MESSAGE) */
								return_code=0;
							}
						}
						else
						{
#if defined (ERROR_MESSAGE)
							set_error_message(PF_ERROR_MESSAGE,
								"read_rgb_image_file.  Could not allocate image arrays for %s.",
								filename);
#endif /* defined (ERROR_MESSAGE) */
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
#if defined (ERROR_MESSAGE)
													set_error_message(PF_ERROR_MESSAGE,
											"read_rgb_image_file.  Unsupported bytes per component in %s.",
														filename);
#endif /* defined (ERROR_MESSAGE) */
													return_code=0;
												} break;
											}
											j--;
										}
										else
										{
#if defined (ERROR_MESSAGE)
											set_error_message(PF_ERROR_MESSAGE,
												"read_rgb_image_file.  Error reading row component in %s.",
												filename);
#endif /* defined (ERROR_MESSAGE) */
											return_code=0;
										}
									}
									i++;
								}
							}
							else
							{
#if defined (ERROR_MESSAGE)
								set_error_message(PF_ERROR_MESSAGE,
									"read_rgb_image_file.  Error positioning file %s.",
									filename);
#endif /* defined (ERROR_MESSAGE) */
								return_code=0;
							}
						}
						else
						{
#if defined (ERROR_MESSAGE)
							set_error_message(PF_ERROR_MESSAGE,
								"read_rgb_image_file.  Could not allocate image arrays for %s.",
								filename);
#endif /* defined (ERROR_MESSAGE) */
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
#if defined (ERROR_MESSAGE)
					set_error_message(PF_ERROR_MESSAGE,
						"read_rgb_image_file.  Invalid image size in %s.", filename);
#endif /* defined (ERROR_MESSAGE) */
					return_code=0;
				}
			}
			else
			{
#if defined (ERROR_MESSAGE)
				set_error_message(PF_ERROR_MESSAGE,
					"read_rgb_image_file.  Error reading file information in %s.", filename);
#endif /* defined (ERROR_MESSAGE) */
				return_code=0;
			}
			fclose(image_file);
		}
#if defined (ERROR_MESSAGE)
		else
		{
			set_error_message(PF_ERROR_MESSAGE,
				"read_rgb_image_file.  Could not open image file %s.", filename);
		}
#endif /* defined (ERROR_MESSAGE) */
	}
#if defined (ERROR_MESSAGE)
	else
	{
		set_error_message(PF_ERROR_MESSAGE,
			"read_rgb_image_file.  Invalid arguments.");
	}
#endif /* defined (ERROR_MESSAGE) */
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
#if defined (ERROR_MESSAGE)
	if (verbose)
	{
		set_error_message(PF_INFORMATION_MESSAGE,
			"read_basis: reading in the basis file \"%s\"",filename);
	}
#endif /* defined (ERROR_MESSAGE) */
	if (file = fopen(filename,"rb"))
	{
		fread(buff,MAGICSIZE,1,file);
		buff[MAGICSIZE] = 0;

		if (strncmp(buff,magic1,MAGICSIZE) == 0)
		{
#if defined (ERROR_MESSAGE)
			if (verbose)
			{
				set_error_message(PF_INFORMATION_MESSAGE,
					"read_basis: magic number \"%s\"",buff);
			}
#endif /* defined (ERROR_MESSAGE) */
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE, "read_basis_version1and2:"
				"\"%s\" is an unsupported old basis file. Magic is \"%s\".",
				filename, buff);
#endif /* defined (ERROR_MESSAGE) */
			return_code = PF_READ_FILE_FAILURE_RC;
			fclose(file);
		}
		else if (strncmp(buff,magic2,MAGICSIZE - 1) == 0)
		{
			buff[MAGICSIZE - 1] = '\0';
#if defined (ERROR_MESSAGE)
			if (verbose)
			{
				set_error_message(PF_INFORMATION_MESSAGE,
					"read_basis: magic number \"%s\"",buff);
			}
#endif /* defined (ERROR_MESSAGE) */
			/* Comment/title line */
			fscanf(file, "%*[^\n]%*[\n]");
			fscanf(file, "%d%d", m, n);
#if defined (ERROR_MESSAGE)
			if (verbose)
			{
				set_error_message(PF_INFORMATION_MESSAGE,"read_basis: dim %d %d",*m,*n);
			}
#endif /* defined (ERROR_MESSAGE) */
			if (ALLOCATE(*a,float,(*m)*(*n)))
			{
				for (i=0;i<*n;i++)
				{
					for (j=0;j<*m;j++)
					{
						fscanf(file, "%f", *a + i * *m + j);
					}
				}
				return_code = PF_SUCCESS_RC;
			}
			else
			{
#if defined (ERROR_MESSAGE)
				set_error_message(PF_ERROR_MESSAGE, "read_basis_version1and2:"
					"Could not allocate array.");
#endif /* defined (ERROR_MESSAGE) */
				return_code = PF_ALLOCATE_FAILURE_RC;
			}
		}
		else
		{
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE, "read_basis_version1and2:"
				"\"%s\" isn't a basis file. Magic is \"%s\"", filename, buff);
#endif /* defined (ERROR_MESSAGE) */
			return_code = PF_READ_FILE_FAILURE_RC;
		}
		fclose(file);
	}
	else
	{
#if defined (ERROR_MESSAGE)
		set_error_message(PF_ERROR_MESSAGE, "read_basis_version1and2:"
			"Unable to open basis file \"%s\"", filename);
#endif /* defined (ERROR_MESSAGE) */
		return_code = PF_OPEN_FILE_FAILURE_RC;
	}
	LEAVE;

	return (return_code);
} /* read_basis_version1and2 */

static int convert_2d_to_ndc(int number_of_markers, float *twod_coordinates,
	float *ndc_coordinates, struct Pf_job *pf_job)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
==============================================================================*/
{
	float maximum_x, minimum_x, maximum_y, minimum_y, *ndc_index, padding_factor = 1.2f,
		range, *twod_index;
	int i, return_code;
	
	ENTER(convert_2d_to_ndc);
	return_code = 0;

	/* Find the maximum value */
	twod_index = twod_coordinates;
	maximum_x = *twod_index;
	minimum_x = *twod_index;
	twod_index++;
	maximum_y = *twod_index;
	minimum_y = *twod_index;
	twod_index++;
	for (i = 1 ; i < number_of_markers ; i++)
	{
		if (*twod_index > maximum_x)
		{
			maximum_x = *twod_index;
		}
		if (*twod_index < minimum_x)
		{
			minimum_x = *twod_index;
		}
		twod_index++;
		if (*twod_index > maximum_y)
		{
			maximum_y = *twod_index;
		}
		if (*twod_index < minimum_y)
		{
			minimum_y = *twod_index;
		}
		twod_index++;
	}

	range = 0.0;
	if (maximum_x != minimum_x)
	{
		range = maximum_x - minimum_x;
	}
	if (maximum_y != minimum_y)
	{
		if (maximum_y - minimum_y > range)
		{
			range = maximum_y - minimum_y;
		}
	}
	if (range > 0.0)
	{
		/* Store the maximum plus a bit for later */
		pf_job->ndc_texture_scaling = 2.0f / (padding_factor * range);
		pf_job->ndc_texture_offset_x = (maximum_x + minimum_x - range) / 2.0f
			- (padding_factor - 1.0f) / 
			(padding_factor * pf_job->ndc_texture_scaling);
		pf_job->ndc_texture_offset_y = (maximum_y + minimum_y - range) / 2.0f
			- (padding_factor - 1.0f) / 
			(padding_factor * pf_job->ndc_texture_scaling);
	}
	else
	{
		pf_job->ndc_texture_scaling = 1.0f;
		pf_job->ndc_texture_offset_x = minimum_x;
		pf_job->ndc_texture_offset_y = minimum_y;
	}

#if defined (CMISS_DEBUG)	
	printf("Minimum %f Maximum %f\n", minimum, maximum);
	printf("Scaling %f Offset x %f y %f\n", pf_job->ndc_texture_scaling, 
		pf_job->ndc_texture_offset_x, pf_job->ndc_texture_offset_y);
#endif /* defined (CMISS_DEBUG) */
	
	/* Scale all the coordinates by this maximum plus a bit */
	twod_index = twod_coordinates;
	ndc_index = ndc_coordinates;
	for (i = 0 ; i < number_of_markers ; i++)
	{
		*ndc_index = (*twod_index - pf_job->ndc_texture_offset_x) * 
			pf_job->ndc_texture_scaling - 1.0f;
		twod_index++;
		ndc_index++;
		/* Y goes in the reverse direction */
		*ndc_index = - (*twod_index - pf_job->ndc_texture_offset_y) * 
			pf_job->ndc_texture_scaling + 1.0f;
		twod_index++;
		ndc_index++;
#if defined (CMISS_DEBUG)	
		printf ("Coordinates %f %f -> %f %f\n", *(twod_index-2), 
			*(twod_index-1), *(ndc_index-2), *(ndc_index-1));
#endif /* defined (CMISS_DEBUG) */
	}	

	LEAVE;

	return (return_code);
} /* convert_2d_to_ndc */

static int create_Pf_job(int *pf_job_id_address)
/*******************************************************************************
LAST MODIFIED : 29 May 2001

DESCRIPTION :
This routine creates a new pf_job, creating the required directories and
initialising the pf_job data.  This function returns the pf_job_id if 
successful.
==============================================================================*/
{
	char *pf_cmiss_force_job_id, *working_path;
	FILE *pf_job_file;
	int pf_job_id, return_code;

	ENTER(create_Pf_job);

	*pf_job_id_address = 0;
	return_code = PF_SUCCESS_RC;
	if (ALLOCATE(working_path, char, strlen(photoface_local_path) + 100))
	{
		/* Create a new directory */
		if ((pf_cmiss_force_job_id=getenv("PF_CMISS_FORCE_JOB_ID")) &&
			sscanf(pf_cmiss_force_job_id, "%d", &pf_job_id))
		{
			return_code = PF_SUCCESS_RC;
#if defined (WIN32)
			sprintf(working_path, "%sworking/job%06d/", photoface_local_path,
				pf_job_id);
			CreateDirectory(working_path, NULL);
#else /* defined (WIN32) */
			sprintf(working_path, "%sworking/job%06d/", photoface_local_path,
				pf_job_id);
			mkdir(working_path, S_IRWXU);
#endif /* defined (WIN32) */
		}
		else
		{
#if defined (WIN32)
			pf_job_id = rand();
			sprintf(working_path, "%sworking/job%06d/", photoface_local_path,
				pf_job_id);
			while ((return_code == PF_SUCCESS_RC) &&
				(!CreateDirectory(working_path, NULL)))
			{
				if (ERROR_ALREADY_EXISTS == GetLastError())
				{
					/* Try another directory */
					pf_job_id = rand();
					sprintf(working_path, "%sworking/job%06d/", photoface_local_path,
						pf_job_id);
				}
				else
				{
#if defined (ERROR_MESSAGE)
					set_error_message(PF_ERROR_MESSAGE, "create_Pf_job: "
						"Unable to create working directory \"%s\".  "
						"Check that working directory exists and that you have permission to write into it.", working_path);
#endif /* defined (ERROR_MESSAGE) */
					return_code = PF_OPEN_FILE_FAILURE_RC;
				}
			}
#else /* defined (WIN32) */
			pf_job_id = random() / 2386;
			sprintf(working_path, "%sworking/job%06d/", photoface_local_path,
				pf_job_id);
			while ((return_code == PF_SUCCESS_RC) &&
				(0 != mkdir(working_path, S_IRWXU)))
			{
				if (EEXIST == errno)
				{
					/* Try another directory */
					pf_job_id = random() / 2386;
					sprintf(working_path, "%sworking/job%06d/", photoface_local_path,
						pf_job_id);
				}
				else
				{
#if defined (ERROR_MESSAGE)
					set_error_message(PF_ERROR_MESSAGE, "create_PF_job: "
						"Unable to create working directory \"%s\".  "
						"Check that working directory exists and that you have permission to write into it.", working_path);
#endif /* defined (ERROR_MESSAGE) */
					return_code = PF_OPEN_FILE_FAILURE_RC;
				}
			}
#endif /* defined (WIN32) */
		}
		if (PF_SUCCESS_RC == return_code)
		{
			/* Create a new pf_job data file */
			sprintf(working_path, "%sworking/job%06d/%s", 
				photoface_local_path, pf_job_id, PF_STRUCTURE_FILENAME);
			if (pf_job_file = fopen(working_path, "w"))
			{
				fprintf(pf_job_file, "%f\n", 1.0f);
				fprintf(pf_job_file, "%f\n", 0.0f);
				fprintf(pf_job_file, "%f\n", 0.0f);
				fclose(pf_job_file);
				*pf_job_id_address = pf_job_id;
			}
			else
			{
#if defined (ERROR_MESSAGE)
				set_error_message(PF_ERROR_MESSAGE, "create_Pf_job:"
					"Unable to open structure data file for writing \"%s\".", working_path);
#endif /* defined (ERROR_MESSAGE) */
				return_code= PF_OPEN_FILE_FAILURE_RC;
			}
		}
		DEALLOCATE(working_path);
	}
	else
	{
#if defined (ERROR_MESSAGE)
		set_error_message(PF_ERROR_MESSAGE, "create_Pf_job:"
			"Could not allocate working string.");
#endif /* defined (ERROR_MESSAGE) */
		return_code= PF_ALLOCATE_FAILURE_RC;
	}

	LEAVE;

	return (return_code);
} /* create_Pf_job */

static int delete_Pf_job(int pf_job_id)
/*******************************************************************************
LAST MODIFIED : 29 May 2001

DESCRIPTION :
This routine cleans up the working directory and destroys the specified job.
==============================================================================*/
{
	char *pf_cmiss_keep_working_dir, *pf_cmiss_wait_delay, *slash_ptr,
		*working_path, *working_path2;
	int delay, return_code;
	struct stat stat_buffer;

	ENTER(delete_Pf_job);

	return_code = PF_SUCCESS_RC;
	if (ALLOCATE(working_path, char, strlen(photoface_local_path) + 100)
		&& ALLOCATE(working_path2, char, strlen(photoface_local_path) + 100))
	{
		/* Look for the correct working directory */
		sprintf(working_path, "%sworking/job%06d", photoface_local_path,
			pf_job_id);
		if (0 == stat(working_path, &stat_buffer))
		{
			/* Lock it or wait */
			strcat(working_path, "/");
			strcat(working_path, PF_LOCK_DIRNAME);
			if ((pf_cmiss_wait_delay = getenv("PF_CMISS_WAIT_DELAY"))
				&& (sscanf(pf_cmiss_wait_delay, "%d", &delay)))
			{
				/* Do nothing */
			}
			else
			{
				delay = 2;
			}
#if defined (WIN32)
			while (!CreateDirectory(working_path, NULL))
			{
				if (ERROR_ALREADY_EXISTS == GetLastError())
				{
					/* Wait */
					Sleep(delay * 1000);
				}
				else
				{
#if defined (ERROR_MESSAGE)
					set_error_message(PF_ERROR_MESSAGE, "delete_Pf_job: "
						"Unable to lock working directory \"%s\".", working_path);
#endif /* defined (ERROR_MESSAGE) */
					return_code = PF_OPEN_FILE_FAILURE_RC;
				}
			}
#else /* defined (WIN32) */
			while (mkdir (working_path, S_IRWXU))
			{
				if (EEXIST == errno)
				{
					/* Wait */
					sleep (delay);
				}
				else
				{
#if defined (ERROR_MESSAGE)
					set_error_message(PF_ERROR_MESSAGE, "delete_Pf_job: "
						"Unable to lock working directory \"%s\".", working_path);
#endif /* defined (ERROR_MESSAGE) */
					return_code = PF_OPEN_FILE_FAILURE_RC;
				}
			}
#endif /* defined (WIN32) */
			if (PF_SUCCESS_RC == return_code)
			{
				sprintf(working_path, "%sworking/job%06d", photoface_local_path,
					pf_job_id);
				sprintf(working_path2, "%sworking/job%06dx", photoface_local_path,
					pf_job_id);
				
				if ((pf_cmiss_keep_working_dir=getenv("PF_CMISS_KEEP_WORKING_DIRECTORY")) &&
					*pf_cmiss_keep_working_dir)
				{
#if defined (WITH_OMNISECURE)
					/* Release the omnisecure lock */
					sprintf(working_path, "%sworking/job%06d/%s/.vpdlock", photoface_local_path,
						pf_job_id, PF_LOCK_DIRNAME);
					unlink(working_path);
#endif /* defined (WITH_OMNISECURE) */

					sprintf(working_path, "%sworking/job%06d/%s", photoface_local_path,
						pf_job_id, PF_LOCK_DIRNAME);
	 				/* Unlock the directory so it can be reused */
#if defined (WIN32)
					RemoveDirectory(working_path);
#else /* defined (WIN32) */
					rmdir(working_path);
#endif /* defined (WIN32) */
					return_code = PF_SUCCESS_RC;
				}
				else
				{
					/* Delete the directory */
#if defined (WIN32)
					/* Move the directory away so no other processes find it */
					MoveFile(working_path, working_path2);

					sprintf(working_path, working_path2);
					/* As we are going to use a system command we need to reverse all the / separators */
					slash_ptr = working_path;
					while (slash_ptr = strchr(slash_ptr, '/'))
					{
						*slash_ptr = '\\';
					}
					sprintf(working_path2, "rmdir /s /q %s", working_path);
#else /* defined (WIN32) */
					/* Move the directory away so no other processes find it */
					rename(working_path, working_path2);

					sprintf(working_path2, "rm -r %sworking/job%06dx", photoface_local_path,
						pf_job_id);
#endif /* defined (WIN32) */
					system(working_path2);
				}
			}
		}
		else
		{
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE, "delete_pf_job:"
				"Working directory \"%s\" does not exist.", working_path);
#endif /* defined (ERROR_MESSAGE) */
			return_code= PF_FIND_FILE_FAILURE_RC;
		}
		DEALLOCATE(working_path);
		DEALLOCATE(working_path2);
	}
	else
	{
#if defined (ERROR_MESSAGE)
		set_error_message(PF_ERROR_MESSAGE, "delete_pf_job:"
			"Could not allocate working string.");
#endif /* defined (ERROR_MESSAGE) */
		return_code= PF_ALLOCATE_FAILURE_RC;
	}

	LEAVE;

	return (return_code);
} /* delete_Pf_job */

static int get_Pf_job_from_id_and_lock(int pf_job_id, struct Pf_job **pf_job_address)
/*******************************************************************************
LAST MODIFIED : 29 May 2001

DESCRIPTION :
This routine uses the <pf_job_id> to find the correct working directory, returns
the corresponding Pf_job structure and locks the directory for that stucture
giving this process exclusive access.
==============================================================================*/
{
	char *working_path;
	FILE *pf_job_file;
	int return_code;
	struct Pf_job *pf_job;
	struct stat stat_buffer;

	ENTER(get_Pf_job_from_id_and_lock);

	return_code = PF_SUCCESS_RC;
	if (ALLOCATE(working_path, char, strlen(photoface_local_path) + 100))
	{
		/* Look for the correct working directory */
		sprintf(working_path, "%sworking/job%06d", photoface_local_path,
			pf_job_id);
		if (0 == stat(working_path, &stat_buffer))
		{
			/* Lock it or wait */
			strcat(working_path, "/");
			strcat(working_path, PF_LOCK_DIRNAME);
#if defined (WIN32)
			while (!CreateDirectory(working_path, NULL))
			{
				if (ERROR_ALREADY_EXISTS == GetLastError())
				{
					/* Wait */
					Sleep(2000);
				}
				else
				{
					return_code = PF_OPEN_FILE_FAILURE_RC;
				}
			}
#else /* defined (WIN32) */
			while (mkdir (working_path, S_IRWXU))
			{
				if (EEXIST == errno)
				{
					/* Wait */
					sleep (2);
					printf("Waiting on lock %s\n", working_path);
				}
				else
				{
#if defined (ERROR_MESSAGE)
					set_error_message(PF_ERROR_MESSAGE, "delete_Pf_job: "
						"Unable to lock working directory \"%s\".", working_path);
#endif /* defined (ERROR_MESSAGE) */
					return_code = PF_OPEN_FILE_FAILURE_RC;
				}
			}
#endif /* defined (WIN32) */
			if (PF_SUCCESS_RC == return_code)
			{
				/* Allocate a Pf_job structure */
				if (ALLOCATE(pf_job, struct Pf_job, 1) && ALLOCATE(pf_job->working_path,
					char, strlen(photoface_local_path) + 25)
					&& ALLOCATE(pf_job->remote_working_path,
					char, strlen(photoface_remote_path) + 25))
				{
					*pf_job_address = pf_job;
					pf_job->pf_job_id = pf_job_id;
					sprintf(pf_job->working_path, "%sworking/job%06d", 
						photoface_local_path, pf_job_id);
					sprintf(pf_job->remote_working_path, "%sworking/job%06d", 
						photoface_remote_path, pf_job_id);
					/* Read the data for this job into the structure */
					sprintf(working_path, "%sworking/job%06d/%s", 
						photoface_local_path, pf_job_id, PF_STRUCTURE_FILENAME);
					if (pf_job_file = fopen(working_path, "r"))
					{
						fscanf(pf_job_file, "%f", &(pf_job->ndc_texture_scaling));
						fscanf(pf_job_file, "%f", &(pf_job->ndc_texture_offset_x));
						fscanf(pf_job_file, "%f", &(pf_job->ndc_texture_offset_y));
						fclose(pf_job_file);
					}
					else
					{
#if defined (ERROR_MESSAGE)
						set_error_message(PF_ERROR_MESSAGE, "get_Pf_job_from_id_and_lock:"
							"Unable to open structure data \"%s\".", working_path);
#endif /* defined (ERROR_MESSAGE) */
						return_code= PF_OPEN_FILE_FAILURE_RC;
					}					
				}
				else
				{
#if defined (ERROR_MESSAGE)
					set_error_message(PF_ERROR_MESSAGE, "get_Pf_job_from_id_and_lock:"
						"Could not allocate Pf_job structure.");
#endif /* defined (ERROR_MESSAGE) */
					return_code= PF_ALLOCATE_FAILURE_RC;
				}
			}
		}
		else
		{
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE, "get_Pf_job_from_id_and_lock:"
				"Working directory \"%s\"for specified job does not exist.", working_path);
#endif /* defined (ERROR_MESSAGE) */
			return_code= PF_FIND_FILE_FAILURE_RC;
		}
		DEALLOCATE(working_path);
	}
	else
	{
#if defined (ERROR_MESSAGE)
		set_error_message(PF_ERROR_MESSAGE, "get_Pf_job_from_id_and_lock:"
			"Could not allocate working string.");
#endif /* defined (ERROR_MESSAGE) */
		return_code= PF_ALLOCATE_FAILURE_RC;
	}

	LEAVE;

	return (return_code);
} /* get_Pf_job_from_id_and_lock */

static int save_state_Pf_job_and_unlock(struct Pf_job **pf_job_address)
/*******************************************************************************
LAST MODIFIED : 29 May 2001

DESCRIPTION :
This routine saves the state corresponding to the specified <pf_job>, deallocates
the memory associated with it and unlocks the access to that job.
==============================================================================*/
{
	char *working_path;
	FILE *pf_job_file;
	int return_code;
	struct Pf_job *pf_job;
	struct stat stat_buffer;

	ENTER(get_Pf_job_from_id_and_lock);

	return_code = PF_SUCCESS_RC;
	if (pf_job = *pf_job_address)
	{
		if (ALLOCATE(working_path, char, strlen(pf_job->working_path) + 100))
		{
			/* Look for the correct working directory */
			sprintf(working_path, "%s/%s", pf_job->working_path, PF_LOCK_DIRNAME);
			if (0 == stat(working_path, &stat_buffer))
			{
				/* Save the data for the pf_job */
				sprintf(working_path, "%s/%s", 
					pf_job->working_path, PF_STRUCTURE_FILENAME);
				if (pf_job_file = fopen(working_path, "w"))
				{
					fprintf(pf_job_file, "%f\n", pf_job->ndc_texture_scaling);
					fprintf(pf_job_file, "%f\n", pf_job->ndc_texture_offset_x);
					fprintf(pf_job_file, "%f\n", pf_job->ndc_texture_offset_y);
					fclose(pf_job_file);
				}
				else
				{
#if defined (ERROR_MESSAGE)
					set_error_message(PF_ERROR_MESSAGE, "save_state_Pf_job_and_unlock:"
						"Unable to open structure data \"%s\".", working_path);
#endif /* defined (ERROR_MESSAGE) */
					return_code= PF_OPEN_FILE_FAILURE_RC;
				}
#if defined (WITH_OMNISECURE)
				/* Release the omnisecure lock */
				sprintf(working_path, "%s/%s/.vpdlock", pf_job->working_path,
					PF_LOCK_DIRNAME);
				unlink(working_path);
#endif /* defined (WITH_OMNISECURE) */

				/* Set the lock filename now before we deallocate everything */
				sprintf(working_path, "%s/%s", pf_job->working_path, PF_LOCK_DIRNAME);
				DEALLOCATE(pf_job->working_path);
				DEALLOCATE(pf_job->remote_working_path);
				/* Deallocate the pf_job structure */
				DEALLOCATE(*pf_job_address);

 				/* Unlock the directory */
#if defined (WIN32)
				RemoveDirectory(working_path);
#else /* defined (WIN32) */
				rmdir(working_path);
#endif /* defined (WIN32) */
			}
			else
			{
#if defined (ERROR_MESSAGE)
				set_error_message(PF_ERROR_MESSAGE, "get_Pf_job_from_id_and_lock:"
					"Lock does not exist in correct place \"%s\".", working_path);
#endif /* defined (ERROR_MESSAGE) */
				return_code= PF_FIND_FILE_FAILURE_RC;
			}
			DEALLOCATE(working_path);
		}
		else
		{
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE, "save_state_Pf_job_and_unlock:"
				"Could not allocate working string.");
#endif /* defined (ERROR_MESSAGE) */
			return_code= PF_ALLOCATE_FAILURE_RC;
		}
	}
	else
	{
#if defined (ERROR_MESSAGE)
		set_error_message(PF_ERROR_MESSAGE, "save_state_Pf_job_and_unlock:"
			"Invalid arguments.");
#endif /* defined (ERROR_MESSAGE) */
		return_code= PF_INVALID_ARGUMENTS_FAILURE_RC;
	}

	LEAVE;

	return (return_code);
} /* save_state_Pf_job_and_unlock */

/*
Global functions
----------------
*/
#if defined (ERROR_MESSAGE)
int pf_get_error_message(char **message)
/*******************************************************************************
LAST MODIFIED : 19 October 2001

DESCRIPTION :
Allocates and returns the reason recorded for the last error returned by
photoface_cmiss.
The string should be freed when no longer required.
==============================================================================*/
{
	int return_code;

	ENTER(pf_get_error_message);

	/* in case forget to set return_code */
	return_code=PF_GENERAL_FAILURE_RC;
	if (ALLOCATE(*message, char, strlen(error_message) + 1))
	{
		strcpy(*message, error_message);
	}
	else
	{
		return_code = PF_ALLOCATE_FAILURE_RC;
	}
	LEAVE;

	return (return_code);
} /* pf_get_error_message */
#endif /* defined (ERROR_MESSAGE) */

int pf_specify_paths(char *photoface_local_path_in, char *photoface_remote_path_in)
/*******************************************************************************
LAST MODIFIED : 19 October 2001

DESCRIPTION :
Specifies the main directory under which the model, working and cmiss directory
are expected to be located.  The paths should end in a delimiting /.
The local path specifies where the files are found on this local operating
system (in a mixed OS system Windows) and the remote path specifies
where the same directory is on the remote fitting machine (at time of writing,
a linux machine).
If the photoface_library and this interface are on the same machine then these
two strings should still be set but they will be identical.

If either path is NULL the internal storage for that path is free'd.
==============================================================================*/
{
	int return_code;

	ENTER(pf_specify_paths);
	return_code=PF_SUCCESS_RC;
	/* Deallocate the old paths */
	if (photoface_local_path)
	{
		DEALLOCATE(photoface_local_path);
	}
	if (photoface_remote_path)
	{
		DEALLOCATE(photoface_remote_path);
	}
	if (photoface_local_path_in)
	{
		if (ALLOCATE(photoface_local_path,char,
			strlen(photoface_local_path_in)+1))
		{
			strcpy(photoface_local_path, photoface_local_path_in);
		}
		else
		{
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE,
				"Unable to allocate local path string.");
#endif /* defined (ERROR_MESSAGE) */
			return_code = PF_ALLOCATE_FAILURE_RC;
		}
	}
	if (photoface_remote_path_in)
	{
		if (ALLOCATE(photoface_remote_path,char,
			strlen(photoface_remote_path_in)+1))
		{
			strcpy(photoface_remote_path, photoface_remote_path_in);
		}
		else
		{
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE,
				"Unable to allocate remote path string.");
#endif /* defined (ERROR_MESSAGE) */
			return_code = PF_ALLOCATE_FAILURE_RC;
		}
	}
	LEAVE;

	return (return_code);
} /* pf_specify_paths */

#if defined (WIN32)
DWORD WINAPI start_cmgui_thread_function(LPVOID pf_job_id_ptr)
{
	DWORD return_code;
	int pf_job_id;

	ENTER(start_cmgui_thread_function);
	return_code=0;
	pf_job_id = *((int *)pf_job_id_ptr);
	linux_execute2("%sbin/cmgui_bg -id %06d",photoface_remote_path,pf_job_id);
	LEAVE;

	return (return_code);
} /* start_cmgui_thread_function */
#endif /* defined (WIN32) */

int pf_setup(char *model_name,char *state,int *pf_job_id)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
<model_name> is the name of head model.  cmiss will have a directory containing
the files (generic obj etc) need for each model.  <state> is additional
information about the face in the image, such as "smiling", which may allow
adjustment of the generic head.  On success, the <pf_job_id> is set.
==============================================================================*/
{
	char *pf_cmiss_wait_delay, *model_path, *working_path;
	char *force_model_name = "sifit_mode0";
	FILE *setup_comfile;
	int attempts, delay, length, return_code;
	struct Pf_job *pf_job;
	struct stat stat_buffer;
#if defined (WIN32)
	DWORD start_cmgui_thread_id;
#else /* defined (WIN32) */
	int pid;
#endif /* defined (WIN32) */

	ENTER(pf_setup);

	return_code=PF_GENERAL_FAILURE_RC;
	/* Start a new job and lock it */
	if (PF_SUCCESS_RC == (return_code = create_Pf_job(pf_job_id)) &&
		(PF_SUCCESS_RC == (return_code = get_Pf_job_from_id_and_lock
			(*pf_job_id, &pf_job))))
	{
		/* Check that we have the files available for the specified model and state */
		if (ALLOCATE(model_path,char,
			strlen(photoface_local_path)+strlen(model_name)+100))
		{
#if defined (WIN32)
			/* Have to remove the trailing / unless it is the root of a drive specification */
			strcpy(model_path, photoface_local_path);
			length = strlen(model_path);
			if ((length > 1) && (':' != model_path[length - 2]))
			{
				model_path[length - 1] = 0;
			}
#else /* defined (WIN32) */
			strcpy(model_path, photoface_local_path);
#endif /* defined (WIN32) */
			if (0 == stat(model_path, &stat_buffer))
			{
				sprintf(model_path,"%smodels/%s/%s.obj",photoface_local_path,force_model_name,
					force_model_name);
				if (0 == stat(model_path, &stat_buffer))
				{
					if (ALLOCATE(working_path,char,2*strlen(photoface_local_path)+100))
					{
						if (0 == stat(pf_job->working_path, &stat_buffer))
						{
							sprintf(working_path, "%scmiss", photoface_local_path);
							if (0 == stat(working_path, &stat_buffer))
							{
								/* Create the source files that represent the selected model and
									state, source.obj, source.basis, source.markers */
								sprintf(working_path, "%s/source.obj",
									pf_job->working_path);
								copy_file(model_path, working_path);
								sprintf(model_path,"%smodels/%s/%s.markers",
									photoface_local_path,force_model_name,force_model_name);
								sprintf(working_path,"%s/source.markers",
									pf_job->working_path);
								copy_file(model_path, working_path);

								/* Create the startup comfile */
								sprintf(working_path, "%s/pf_setup_main.com",
									pf_job->working_path);
								if (setup_comfile = fopen(working_path, "w"))
								{
									fprintf(setup_comfile, "$PHOTOFACE_WORKING = \"%s\"\n",
										pf_job->remote_working_path);
									fprintf(setup_comfile, "$PHOTOFACE_CMISS = \"%scmiss\"\n",
										photoface_remote_path);
									fprintf(setup_comfile, "$PHOTOFACE_BIN = \"%sbin\"\n",
										photoface_remote_path);
									fprintf(setup_comfile, "$ANTHRO_FITTING_CMISS = \"%s../anthro_fitting/cmiss\"\n",
										photoface_remote_path);
									fprintf(setup_comfile, "$LFX_MODELER_CMISS = \"%s../lfx_modeler/cmiss\"\n",
										photoface_remote_path);
									fprintf(setup_comfile, "$PHOTOFACE_MODEL = \"%s\"\n",
										model_name);
									fprintf(setup_comfile, "open comfile $PHOTOFACE_CMISS/pf_setup.com exec\n");

									fclose(setup_comfile);
									if ((pf_cmiss_wait_delay = getenv("PF_CMISS_WAIT_DELAY"))
										&& (sscanf(pf_cmiss_wait_delay, "%d", &delay)))
									{
										/* Do nothing */
									}
									else
									{
#if defined (WIN32)
										delay = 15;
#else /* defined (WIN32) */
										delay = 2;
#endif /* defined (WIN32) */
									}

									/* Start cmgui in a forked thread and load this model */
#if defined (WIN32)
									CreateThread(/*no security attributes*/NULL,
										/*use default stack size*/0,start_cmgui_thread_function,
										(LPVOID)pf_job_id,/*use default creation flags*/0,
										&start_cmgui_thread_id);
									/* Main process comes here */
#else /* defined (WIN32) */
									if (!(pid = fork()))
									{
										if (linux_execute2("%sbin/cmgui_bg -id %06d", 
											photoface_remote_path, *pf_job_id))
										{
											return_code=PF_SUCCESS_RC;
										}
										exit(0);
									}
									/* Main process comes here */
#endif /* defined (WIN32) */
									return_code = PF_GENERAL_FAILURE_RC;
									attempts = 0;
									while ((return_code != PF_SUCCESS_RC) &&
										(attempts < MAX_ATTEMPTS_AT_FIRST_CMISS_CONTROL))
									{
#if defined (WIN32)
										Sleep((DWORD)(delay * 1000));
#else /* defined (WIN32) */
										sleep(delay);
#endif /* defined (WIN32) */										
										attempts++;
										if (linux_execute("%sbin/cmgui_control -id %06d 'open comfile %s/pf_setup_main.com exec'",
												 photoface_remote_path, *pf_job_id, pf_job->remote_working_path))
										{
											return_code=PF_SUCCESS_RC;
										}
									}
#if defined (ERROR_MESSAGE)
									if (return_code != PF_SUCCESS_RC)
									{
										set_error_message(PF_ERROR_MESSAGE,"Unable to connect to cmiss after %d attempts.",
											"  Check that your rsh connection is enabled, that your "
											"offscreen display is valid and your window manager suitable.");
									}
#endif /* defined (ERROR_MESSAGE) */
								}
								else
								{
#if defined (ERROR_MESSAGE)
									set_error_message(PF_ERROR_MESSAGE,"Unable to open file %s.",
										working_path);
#endif /* defined (ERROR_MESSAGE) */
									return_code = PF_OPEN_FILE_FAILURE_RC;
								}

							}
							else
							{
#if defined (ERROR_MESSAGE)
								set_error_message(PF_ERROR_MESSAGE,
									"Unable to find cmiss directory %s.",working_path);
#endif /* defined (ERROR_MESSAGE) */
								return_code = PF_FIND_FILE_FAILURE_RC;
							}
						}
						else
						{
#if defined (ERROR_MESSAGE)
							set_error_message(PF_ERROR_MESSAGE,
								"Unable to find model directory %s.",working_path);
#endif /* defined (ERROR_MESSAGE) */
							return_code = PF_FIND_FILE_FAILURE_RC;
						}
						DEALLOCATE(working_path);
					}
					else
					{
#if defined (ERROR_MESSAGE)
						set_error_message(PF_ERROR_MESSAGE,
							"Could not allocate temporary string.");
#endif /* defined (ERROR_MESSAGE) */
						return_code = PF_ALLOCATE_FAILURE_RC;
					}
				}
				else
				{
#if defined (ERROR_MESSAGE)
					set_error_message(PF_ERROR_MESSAGE,"Unable to find model directory %s.",
						model_path);
#endif /* defined (ERROR_MESSAGE) */
					return_code = PF_FIND_FILE_FAILURE_RC;
				}
				DEALLOCATE(model_path);
			}
			else
			{
#if defined (ERROR_MESSAGE)
				set_error_message(PF_ERROR_MESSAGE, "Unable to find photoface directory %s.",
					photoface_local_path);
#endif /* defined (ERROR_MESSAGE) */
				return_code = PF_FIND_FILE_FAILURE_RC;
			}
		}
		else
		{
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE,"Could not allocate temporary string.");
#endif /* defined (ERROR_MESSAGE) */
			return_code = PF_ALLOCATE_FAILURE_RC;
		}
		save_state_Pf_job_and_unlock(&pf_job);
	}
	LEAVE;

	return (return_code);
} /* pf_setup */

int pf_close(int pf_job_id)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
Closes the cmiss library.  Freeing any internal memory and stopping any
internal processes.
==============================================================================*/
{
	char **name_address;
	int i;
	int return_code;
	struct Pf_job *pf_job;

	ENTER(pf_close);
	return_code=PF_GENERAL_FAILURE_RC;

	/* free markers, bad if we aren't working in a linear fashion, but if they
		are needed then they will just get reloaded. */
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
		markers = (struct Marker_struct *)NULL;
	}

	if(PF_SUCCESS_RC == (return_code = 
		get_Pf_job_from_id_and_lock(pf_job_id, &pf_job)))
	{
		/* close cmgui */
		if (linux_execute("%sbin/cmgui_control -id %06d  'quit'", photoface_remote_path, pf_job_id))
		{
			return_code=PF_SUCCESS_RC;
		}
		save_state_Pf_job_and_unlock(&pf_job);
		delete_Pf_job(pf_job_id);
	}

	LEAVE;

	return (return_code);
} /* pf_close */

int pf_define_markers(int pf_job_id,int number_of_markers,char **marker_names,
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
	struct Pf_job *pf_job;

	ENTER(pf_define_markers);

	return_code=PF_SUCCESS_RC;
	/* If we haven't already done so read the marker definition file */
	if(PF_SUCCESS_RC == (return_code = 
		get_Pf_job_from_id_and_lock(pf_job_id, &pf_job)))
	{
		if (!markers)
		{
			sprintf(filename, "%s/source.markers", pf_job->working_path);
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
#if defined (ERROR_MESSAGE)
					set_error_message(PF_ERROR_MESSAGE,
						"Marker name \"%s\" already defined for current model,"
						" respecifying 3D position", marker_names[i]);
#endif /* defined (ERROR_MESSAGE) */
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
#if defined (ERROR_MESSAGE)
						set_error_message(PF_ERROR_MESSAGE,
							"Problem with reallocating marker storage");
#endif /* defined (ERROR_MESSAGE) */
						return_code = PF_ALLOCATE_FAILURE_RC;
					}
				}
			}
		}
		else
		{
#if defined (OLD_CODE)
			/* Leave the error message from the read_marker_file routine */
			set_error_message(PF_ERROR_MESSAGE, "Unable to read markers file");
#endif /* defined (OLD_CODE) */
			return_code = PF_READ_FILE_FAILURE_RC;
		}
		save_state_Pf_job_and_unlock(&pf_job);
	}
	LEAVE;

	return (return_code);
} /* pf_define_markers */

int pf_specify_markers(int pf_job_id,int number_of_markers,char **marker_names,
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
	struct Pf_job *pf_job;

	ENTER(pf_specify_markers);
	return_code=PF_GENERAL_FAILURE_RC;
	if(PF_SUCCESS_RC == (return_code = 
		get_Pf_job_from_id_and_lock(pf_job_id, &pf_job)))
	{
		if (ALLOCATE(filename,char,strlen(photoface_local_path)+100))
		{
#if defined (WRITE_SPECIFY_MARKER_FILE)
			{
				FILE *file;
				
				/* write out input markers so can reproduce in test case */
				sprintf(filename, "%s/pf_specify_markers.c", pf_job->working_path);
				if (file = fopen(filename, "w"))
				{
					fprintf(file,"  /* input supplied to pf_specify_markers */\n");
					fprintf(file,"  int number_of_markers = %d;\n",number_of_markers);
					fprintf(file,"  char *marker_names[] = {\n");
					for (i = 0 ; i < number_of_markers ; i++)
					{
						if (i < (number_of_markers - 1))
						{
							fprintf(file, "    \"%s\",\n", marker_names[i]);
						}
						else
						{
							fprintf(file, "    \"%s\"};\n", marker_names[i]);
						}
					}
					fprintf(file,"  float marker_2d_positions[] = {\n");
					for (i = 0 ; i < number_of_markers ; i++)
					{
						if (i < (number_of_markers - 1))
						{
							fprintf(file, "    %g, %g, /* %s */\n",
								marker_2d_positions[i*2], marker_2d_positions[i*2 + 1],
								marker_names[i]);
						}
						else
						{
							fprintf(file, "    %g, %g /* %s */ };\n",
								marker_2d_positions[i*2], marker_2d_positions[i*2 + 1],
								marker_names[i]);
						}
					}
					fprintf(file,"  float marker_confidences[] = {\n");
					for (i = 0 ; i < number_of_markers ; i++)
					{
						if (i < (number_of_markers - 1))
						{
							fprintf(file, "    %g, /* %s */\n", marker_confidences[i],
								marker_names[i]);
						}
						else
						{
							fprintf(file, "    %g /* %s */ };\n", marker_confidences[i],
								marker_names[i]);
						}
					}
					fclose(file);
				}
			}
#endif /* defined (WRITE_SPECIFY_MARKER_FILE) */
			/* If we haven't already done so read the marker definition file */
			if (!markers)
			{
				sprintf(filename, "%s/source.markers", pf_job->working_path);
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
							marker_positions[3 * i] = markers->marker_positions[3 * j];
							marker_positions[3 * i + 1] = markers->marker_positions[3 * j + 1];
							marker_positions[3 * i + 2] = markers->marker_positions[3 * j + 2];
						}
#if defined (ERROR_MESSAGE)
						else
						{
							set_error_message(PF_ERROR_MESSAGE,
								"Marker name \"%s\" not defined for current model,"
								" ignoring 2D position", marker_names[i]);
						}
#endif /* defined (ERROR_MESSAGE) */
					}
					/* Write out the placed_2d_points.exnode that specifies the these 2D
						positions for calculate_ptm and fit.pl in ndc coordinates */
					sprintf(filename,"%s/specified_2D_positions.exnode",
						pf_job->working_path);
					convert_2d_to_ndc(number_of_markers, marker_2d_positions,
						marker_ndc_positions, pf_job);
					if (!(return_code=write_exnode(filename,"fiducial","ndc_coordinates",
						2, number_of_markers, marker_indices, marker_ndc_positions)))
					{
						sprintf(filename, "%s/specified_marker_confidence.exnode",
							pf_job->working_path);
						if (!(return_code=write_exnode(filename,"fiducial",
							"marker_confidence",1,number_of_markers,marker_indices,
							marker_confidences)))
						{
							/* Write out the 3D model positions for these nodes */
							sprintf(filename, "%s/source_coordinates.exnode",
								pf_job->working_path);
							if (!(return_code=write_exnode(filename,"fiducial",
								"marker_coordinates",3,number_of_markers,marker_indices,
								marker_positions)))
							{
								/* Write the same values as the target_coordinates as we just want
									to define the field for all these nodes and then cmgui will
									overwrite the actual target positions */
								sprintf(filename, "%s/target_coordinates.exnode",
									pf_job->working_path);
								if (!(return_code = write_exnode(filename, "fiducial", 
									"target_coordinates", 3, number_of_markers, marker_indices, 
									marker_positions)))
								{
									/* Define these fields and nodes */
									if (linux_execute(
										"%sbin/cmgui_control -id %06d  'open comfile %scmiss/pf_specify_markers.com exec'",
										photoface_remote_path, pf_job_id, photoface_remote_path))
									{
										return_code=PF_SUCCESS_RC;
									}
								}
								else
								{
#if defined (ERROR_MESSAGE)
									set_error_message(PF_ERROR_MESSAGE,"Unable to write file %s",
										filename);
#endif /* defined (ERROR_MESSAGE) */
									return_code = PF_WRITE_FILE_FAILURE_RC;
								}
							}
							else
							{
#if defined (ERROR_MESSAGE)
								set_error_message(PF_ERROR_MESSAGE,"Unable to write file %s",
									filename);
#endif /* defined (ERROR_MESSAGE) */
								return_code = PF_WRITE_FILE_FAILURE_RC;
							}
						}
						else
						{
#if defined (ERROR_MESSAGE)
							set_error_message(PF_ERROR_MESSAGE,"Unable to write file %s",
								filename);
#endif /* defined (ERROR_MESSAGE) */
							return_code = PF_WRITE_FILE_FAILURE_RC;
						}
					}
					else
					{
#if defined (ERROR_MESSAGE)
						set_error_message(PF_ERROR_MESSAGE, "Unable to write file %s",
							filename);
#endif /* defined (ERROR_MESSAGE) */
						return_code = PF_WRITE_FILE_FAILURE_RC;
					}
				}
				else
				{
#if defined (ERROR_MESSAGE)
					set_error_message(PF_ERROR_MESSAGE,
						"Unable to allocate marker indices array");
#endif /* defined (ERROR_MESSAGE) */
					return_code = PF_ALLOCATE_FAILURE_RC;
				}
				DEALLOCATE(marker_indices);
				DEALLOCATE(marker_ndc_positions);
				DEALLOCATE(marker_positions);
			}
			else
			{
#if defined (OLD_CODE)
				/* Leave the error message from read_marker_file routine */
				set_error_message(PF_ERROR_MESSAGE,"Unable to read markers file");
#endif /* defined (OLD_CODE) */
				return_code = PF_READ_FILE_FAILURE_RC;
			}
			DEALLOCATE(filename);
		}
		else
		{
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE,"Unable to allocate filename string");
#endif /* defined (ERROR_MESSAGE) */
			return_code = PF_ALLOCATE_FAILURE_RC;
		}
		save_state_Pf_job_and_unlock(&pf_job);
	}
	LEAVE;

	return (return_code);
} /* pf_specify_markers */

int pf_get_marker_fitted_positions(int pf_job_id,int number_of_markers,
	char **marker_names,float *marker_fitted_3d_positions)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
Returns the fitted positions of the markers in <marker_fitted_3d_positions>
which is assumed to be allocated large enough for 3*<number_of_markers> floats
(marker number varying slowest).
==============================================================================*/
{
	char *filename, buffer[MARKER_BUFFER_SIZE], 
		*special_marker_names[] = {"eyeball_origin_left",
									  "eyeball_origin_right"};
	FILE *eye_file, *warped_file;
	float *marker_positions, marker_x, marker_y, marker_z;
	int call_cmiss, i, j, load_eyeballs, *marker_indices, return_code;
	int number_of_special_markers = 2;
	struct Pf_job *pf_job;

	ENTER(pf_get_marker_fitted_positions);
	return_code=PF_GENERAL_FAILURE_RC;
	if(PF_SUCCESS_RC == (return_code = 
		get_Pf_job_from_id_and_lock(pf_job_id, &pf_job)))
	{
		if (ALLOCATE(filename,char,strlen(photoface_local_path)+100))
		{
			call_cmiss = 0;
			load_eyeballs = 0;
			/* If we haven't already done so read the marker definition file */
			if (!markers)
			{
				sprintf(filename, "%s/source.markers", pf_job->working_path);
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
						while ((j < number_of_special_markers) &&
							(!fuzzy_string_compare_same_length(marker_names[i],
								special_marker_names[j])))
						{
							j++;
						}
						if (j < markers->number_of_markers)
						{
							/* Special marker name, currently forces an eyeball load */
							marker_indices[i] = -j;
							load_eyeballs = 1;
						}
						else
						{
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
								call_cmiss = 1;
							}
#if defined (ERROR_MESSAGE)
							else
							{
								set_error_message(PF_WARNING_MESSAGE,
									"Marker name \"%s\" not defined for current model, ignoring",
									marker_names[i]);
							}
#endif /* defined (ERROR_MESSAGE) */
						}
					}
					if (load_eyeballs)
					{
						sprintf(filename, "%s/standin_eyes.exnode", pf_job->working_path);
						if (eye_file = fopen(filename, "r"))
						{
							i = 0;
							return_code = PF_SUCCESS_RC;
							while ((return_code == PF_SUCCESS_RC) && (!feof(eye_file)) && 
								(i < 2) && (!read_line(eye_file, buffer, MARKER_BUFFER_SIZE)))
							{
								if (3 == sscanf(buffer, "%f%f%f", &marker_x, &marker_y,
									&marker_z))
								{
									for (j = 0 ; j < number_of_markers ; j++)
									{
										if (marker_indices[j] == -i)
										{
											marker_fitted_3d_positions[3 * j] = marker_x;
											marker_fitted_3d_positions[3 * j + 1] = marker_y;
											marker_fitted_3d_positions[3 * j + 2] = marker_z;
										}
									}
									i++;									
								}
							}
							fclose(eye_file);
						}
						else
						{
#if defined (ERROR_MESSAGE)
							set_error_message(PF_ERROR_MESSAGE,
								"Unable to open eye positions file %s",filename);
#endif /* defined (ERROR_MESSAGE) */
							return_code = PF_OPEN_FILE_FAILURE_RC;
						}
					}

					if (call_cmiss)
					{
						/* Write out the original.exnode that specifies the the original
							3D positions for fit.pl */
						sprintf(filename, "%s/original.exnode", pf_job->working_path);
						if (!(return_code = write_exnode(filename, "get_marker_positions", 
									"marker_coordinates",
									3, number_of_markers, marker_indices, marker_positions)))
						{
							/* Define these fields and nodes */
							if (linux_execute(
									 "%sbin/cmgui_control -id %06d  'open comfile %scmiss/pf_get_marker_fitted_positions.com exec'",
									 photoface_remote_path, pf_job_id, photoface_remote_path))
							{
								return_code=PF_SUCCESS_RC;
							}
							/* Read the generated exnode file and return the results */
							sprintf(filename, "%s/warped.exnode",
								pf_job->working_path);
							if (warped_file = fopen(filename, "r"))
							{
								i = 0;
								return_code = PF_SUCCESS_RC;
								while ((return_code == PF_SUCCESS_RC) && (!feof(warped_file)) && 
									(i < number_of_markers) &&
									(!read_line(warped_file, buffer, MARKER_BUFFER_SIZE)))
								{
									if (3 == sscanf(buffer, "%f%f%f", &marker_x, &marker_y,
											 &marker_z))
									{
										while (marker_indices[i] < 1)
										{
											i++;
										}
										if (i >= number_of_markers)
										{
#if defined (ERROR_MESSAGE)
											set_error_message(PF_ERROR_MESSAGE,
												"Unexpected marker positions in file %s",filename);
#endif /* defined (ERROR_MESSAGE) */
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
#if defined (ERROR_MESSAGE)
									set_error_message(PF_ERROR_MESSAGE,
										"Insufficient marker positions in file %s",filename);
#endif /* defined (ERROR_MESSAGE) */
									return_code = PF_READ_FILE_FAILURE_RC;
								}
								fclose(warped_file);
							}
							else
							{
#if defined (ERROR_MESSAGE)
								set_error_message(PF_ERROR_MESSAGE,
									"Unable to open warped positions file %s",filename);
#endif /* defined (ERROR_MESSAGE) */
								return_code = PF_OPEN_FILE_FAILURE_RC;
							}
						}
						else
						{
#if defined (ERROR_MESSAGE)
							set_error_message(PF_ERROR_MESSAGE,
								"Unable to write original positions file %s",filename);
#endif /* defined (ERROR_MESSAGE) */
							return_code = PF_WRITE_FILE_FAILURE_RC;
						}
					}
					DEALLOCATE(marker_indices);
					DEALLOCATE(marker_positions);
				}
				else
				{
#if defined (ERROR_MESSAGE)
					set_error_message(PF_ERROR_MESSAGE,
						"Unable to allocate marker indices array");
#endif /* defined (ERROR_MESSAGE) */
					return_code = PF_ALLOCATE_FAILURE_RC;
				}
			}
			else
			{
				/* Leave the read_marker_file function error */
#if defined (OLD_CODE)
				set_error_message(PF_ERROR_MESSAGE,"Unable to read markers file %s",
					filename);
#endif /* defined (OLD_CODE) */
				return_code = PF_READ_FILE_FAILURE_RC;
			}
			DEALLOCATE(filename);
		}
		else
		{
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE,"Unable to allocate filename string");
#endif /* defined (ERROR_MESSAGE) */
			return_code = PF_ALLOCATE_FAILURE_RC;
		}
		save_state_Pf_job_and_unlock(&pf_job);
	}
	LEAVE;

	return (return_code);
} /* pf_get_marker_fitted_positions */

int pf_view_align(int pf_job_id,float *error_measure)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
Calculates the view that aligns the model to the specified markers and returns
an <error_measure>.
==============================================================================*/
{
	int return_code;
	struct Pf_job *pf_job;

	ENTER(pf_view_align);
	return_code=PF_GENERAL_FAILURE_RC;
	if(PF_SUCCESS_RC == (return_code = 
		get_Pf_job_from_id_and_lock(pf_job_id, &pf_job)))
	{
		/* For now this is all done by the comfile, until we have more direct control
			over cmgui */
		if (linux_execute(
			"%sbin/cmgui_control -id %06d  'open comfile %scmiss/pf_view_align.com exec'",
			photoface_remote_path, pf_job_id, photoface_remote_path))
		{
			return_code=PF_SUCCESS_RC;
		}
		/* Run the filt program calculate_ptm */
		/* Load the projection into cmgui so that the fields are updated relative to
			it */
		save_state_Pf_job_and_unlock(&pf_job);
	}
	LEAVE;

	return (return_code);
} /* pf_view_align */

int pf_get_view(int pf_job_id,float *eye_point,float *interest_point,
	float *up_vector,float *view_angle)
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
	struct Pf_job *pf_job;

	ENTER(pf_get_view);
	return_code=PF_GENERAL_FAILURE_RC;
	if(PF_SUCCESS_RC == (return_code = 
		get_Pf_job_from_id_and_lock(pf_job_id, &pf_job)))
	{
		/* Get the current viewing parameters from cmgui */
		/* Export the window projection from cmgui to a file */
		if (linux_execute(
			"%sbin/cmgui_control -id %06d  'open comfile %scmiss/pf_get_view.com exec'",
			photoface_remote_path, pf_job_id, photoface_remote_path))
		{
			return_code=PF_SUCCESS_RC;
		}
		sprintf(filename, "%s/window_placement.com", pf_job->working_path);
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
					// Convert from diagonal to horizontal view angle
					*view_angle = (float)(2.0 * 180.0 / 3.141592654 * atan(tan((double)*view_angle 
						* 3.141592654 / (180.0 * 2.0)) / 1.414213562));
					return_code = PF_SUCCESS_RC;
				}
			}
			fclose(window_commands_file);
#if defined (ERROR_MESSAGE)
			if (return_code == PF_READ_FILE_FAILURE_RC)
			{
				set_error_message(PF_ERROR_MESSAGE,
					"Unable to find the window projection command in file %s",filename);
			}
#endif /* defined (ERROR_MESSAGE) */
		}
		else
		{
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE, "Unable to open file %s for reading",
				filename);
#endif /* defined (ERROR_MESSAGE) */
			return_code= PF_OPEN_FILE_FAILURE_RC;
		}
		save_state_Pf_job_and_unlock(&pf_job);
	}
	LEAVE;

	return (return_code);
} /* pf_get_view */

int pf_specify_view(int pf_job_id,float *eye_point,float *interest_point,
	float *up_vector,float view_angle)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
Sets the current view as an <eye_point> (3 component vector), an
<interest_point> (3 component vector), an <up_vector> (3 component vector) and a
<view_angle> (scalar).  It is an alternative/override for pf_view_align.
==============================================================================*/
{
	int return_code;
	struct Pf_job *pf_job;

	ENTER(pf_specify_view);
	return_code=PF_GENERAL_FAILURE_RC;
	if(PF_SUCCESS_RC == (return_code = 
		get_Pf_job_from_id_and_lock(pf_job_id, &pf_job)))
	{
		/* Set the viewing parameters in cmgui */
		view_angle = (float)(2.0 * 180.0 / 3.141592654 * atan(tan(view_angle 
			* 3.141592654 / (180.0 * 2.0)) * 1.414213562));
		if (linux_execute(
			"%sbin/cmgui_control -id %06d  'gfx modify window 1 view eye %f %f %f "
			"interest %f %f %f up %f %f %f view_angle %f'",
			photoface_remote_path, pf_job_id, eye_point[0], eye_point[1], eye_point[2],
			interest_point[0], interest_point[1], interest_point[2],
			up_vector[0], up_vector[1], up_vector[2], view_angle))
		{
			return_code=PF_SUCCESS_RC;
		}
		save_state_Pf_job_and_unlock(&pf_job);
	}
	LEAVE;

	return (return_code);
} /* pf_specify_view */

int pf_fit(int pf_job_id,float *error_measure)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
Fits the model to the specified markers, using the current transformation
matrix, and returns an <error_measure>.
==============================================================================*/
{
	int return_code;
	struct Pf_job *pf_job;

	ENTER(pf_fit);
	return_code=PF_GENERAL_FAILURE_RC;
	if(PF_SUCCESS_RC == (return_code = 
		get_Pf_job_from_id_and_lock(pf_job_id, &pf_job)))
	{
		/* For now this is all done by the comfile, until we have more direct
			control over cmgui */
		if (linux_execute(
			"%sbin/cmgui_control -id %06d  'open comfile %scmiss/pf_fit.com exec'",
			photoface_remote_path, pf_job_id, photoface_remote_path))
		{
			return_code=PF_SUCCESS_RC;
		}

		/* Use cmgui to calculate 3D positions for markers where the 2D point is 
			specified and write these out */

		/* Run fit.pl */

		/* Load the fitted model into cmgui */

		/* Place the eyes */
		save_state_Pf_job_and_unlock(&pf_job);
	}
	LEAVE;

	return (return_code);
} /* pf_fit */

int pf_get_head_model(int pf_job_id,int *number_of_vertices,
	float **vertex_3d_locations,int *number_of_texture_vertices,
	float **texture_vertex_3d_locations,int *number_of_triangles,
	int **triangle_vertices,int **triangle_texture_vertices)
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
	struct Pf_job *pf_job;

	ENTER(pf_get_head_model);
	if(PF_SUCCESS_RC == (return_code = 
		get_Pf_job_from_id_and_lock(pf_job_id, &pf_job)))
	{
		return_code=PF_SUCCESS_RC;
		/* Read the fitted obj file and return all the values */
		sprintf(filename, "%s/standin.obj", pf_job->working_path);
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
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE, "Unable to open file %s for reading",
				filename);
#endif /* defined (ERROR_MESSAGE) */
			return_code= PF_OPEN_FILE_FAILURE_RC;
		}
		save_state_Pf_job_and_unlock(&pf_job);
	}
	LEAVE;

	return (return_code);
} /* pf_get_head_model */

int pf_get_basis(int pf_job_id,int *number_of_modes,int *number_of_vertices,
	float **vertex_3d_locations_or_offsets)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
Returns the basis for the current transformed model in
<vertex_3d_locations_or_offsets> which is a 1-D array of
3*<number_of_modes>*<number_of_vertices> floats with x,y,z varying fastest and
mode number slowest.
==============================================================================*/
{
	char filename[200];
	int return_code;
	struct Pf_job *pf_job;


	ENTER(pf_get_basis);
	if(PF_SUCCESS_RC == (return_code = 
		get_Pf_job_from_id_and_lock(pf_job_id, &pf_job)))
	{
		return_code=PF_GENERAL_FAILURE_RC;
		/* Calculate the basis based on the current host mesh transformation
			??? (or do this as a separate step, or as part of doing the fit) */
		if (linux_execute(
			"%sbin/cmgui_control -id %06d  'open comfile %scmiss/pf_get_basis.com exec'",
			photoface_remote_path, pf_job_id, photoface_remote_path))
		{
			return_code=PF_SUCCESS_RC;
		}

		/* Read the fitted basis file and return all the values */
		sprintf(filename, "%s/standin.basis", pf_job->working_path);
		if (PF_SUCCESS_RC == (return_code = read_basis_version1and2(filename, number_of_vertices,
						 number_of_modes, vertex_3d_locations_or_offsets, /*verbose*/0)))
		{
			*number_of_vertices /= 3; /* Each component is a separate number */
		}
		save_state_Pf_job_and_unlock(&pf_job);
	}
	LEAVE;

	return (return_code);
} /* pf_get_basis */

int pf_specify_image(int pf_job_id,int width,int height,
	enum PF_image_format image_format,char *image)
/*******************************************************************************
LAST MODIFIED : 15 February 2001

DESCRIPTION :
Used to specify the image to be texture mapped onto the model.
==============================================================================*/
{
	char *pf_cmiss_wait_delay, *image_ptr, filename[200];
	float texture_ndc_x, temp_texture_ndc_y, texture_ndc_y, texture_ndc_width,
		texture_ndc_height;
	FILE *image_file, *image_comfile;
	int delay, i, image_components, return_code;
	struct Pf_job *pf_job;

	ENTER(pf_specify_image);
	if(PF_SUCCESS_RC == (return_code = 
		get_Pf_job_from_id_and_lock(pf_job_id, &pf_job)))
	{
		return_code=PF_GENERAL_FAILURE_RC;
		/* Create the rgb image for this image and make the image square by padding */
		sprintf(filename, "%s/source_image.raw", pf_job->working_path);
		switch (image_format)
		{
			case PF_RGB_IMAGE:
			{
				image_components = 3;
			} break;
			case PF_RGBA_IMAGE:
			{
				image_components = 4;
			} break;
		}
		if (image_file = fopen(filename, "wb"))
		{
			image_ptr = image;
			for (i = 0 ; i < width * height ; i++)
			{
				fwrite(image_ptr, 1, 3, image_file);
				image_ptr += image_components;
			}
			fclose(image_file);

			/* Calculate the texture placement coordinates in ndc space */
			texture_ndc_x = - pf_job->ndc_texture_offset_x * pf_job->ndc_texture_scaling
				- 1.0f;
			temp_texture_ndc_y = - pf_job->ndc_texture_offset_y * pf_job->ndc_texture_scaling
				- 1.0f;
			texture_ndc_y = pf_job->ndc_texture_offset_y * pf_job->ndc_texture_scaling
				- 1.0f;
			texture_ndc_width = ((float)width - pf_job->ndc_texture_offset_x) *
				pf_job->ndc_texture_scaling - 1.0f - texture_ndc_x;
			texture_ndc_height = ((float)height - pf_job->ndc_texture_offset_y)
				* pf_job->ndc_texture_scaling - 1.0f - temp_texture_ndc_y;

			sprintf(filename, "%s/pf_specify_image.com", pf_job->working_path);
			if (image_comfile = fopen(filename, "w"))
			{
				fprintf(image_comfile, "print (\"Running pf_specify_image.com\\n\");\n");
				fprintf(image_comfile, "gfx modify texture source_image image rgb:%s/source_image.raw specify_width %d specify_height %d raw_interleaved\n",
					pf_job->remote_working_path, width, height);
				fprintf(image_comfile, "gfx modify window 1 background tex_placement %f %f %f %f\n",
					texture_ndc_x, texture_ndc_y, texture_ndc_width, texture_ndc_height);

				fclose(image_comfile);

				if ((pf_cmiss_wait_delay = getenv("PF_CMISS_WAIT_DELAY"))
					&& (sscanf(pf_cmiss_wait_delay, "%d", &delay)))
				{
					/* Do nothing */
				}
				else
				{
					delay = 2;
				}

#if defined (WIN32)
				Sleep((DWORD)(delay * 1000));
#else /* defined (WIN32) */
				sleep(delay);
#endif /* defined (WIN32) */

				if (linux_execute(
					"%sbin/cmgui_control -id %06d  'open comfile %s/pf_specify_image.com exec'",
					photoface_remote_path, pf_job_id, pf_job->remote_working_path))
				{
					return_code=PF_SUCCESS_RC;
				}
			}
			else
			{
#if defined (ERROR_MESSAGE)
				set_error_message(PF_ERROR_MESSAGE, "Unable to open file %s for writing",
					filename);
#endif /* defined (ERROR_MESSAGE) */
				return_code=PF_OPEN_FILE_FAILURE_RC;
			}
		}
		else
		{
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE, "Unable to open file %s for writing",
				filename);
#endif /* defined (ERROR_MESSAGE) */
			return_code=PF_OPEN_FILE_FAILURE_RC;
		}
		save_state_Pf_job_and_unlock(&pf_job);
	}
	LEAVE;

	return (return_code);
} /* pf_specify_image */

int pf_get_texture(int pf_job_id,int width,int height,
	enum PF_image_format image_format, char *texture)
/*******************************************************************************
LAST MODIFIED : 26 June 2001

DESCRIPTION :
The caller specifies the texture size and provides the storage.  The <texture>
is filled in based on the current model.
==============================================================================*/
{
	char filename[200], *src_image_ptr, *dest_image_ptr;
	int destination_components, i, j, number_of_bytes_per_component, number_of_components, return_code;
	long int file_height, file_width;
	long unsigned *image;
	struct Pf_job *pf_job;

	ENTER(pf_get_texture);
	if(PF_SUCCESS_RC == (return_code = 
		get_Pf_job_from_id_and_lock(pf_job_id, &pf_job)))
	{
		return_code=PF_GENERAL_FAILURE_RC;
		/* Use cmgui to calculate the texture based on the specified image */
		if (linux_execute(
				 "%sbin/cmgui_control -id %06d  '$width=%d;$height=%d;cmiss(qq(open comfile %scmiss/pf_get_texture.com exec))'",
				 photoface_remote_path, pf_job_id, width, height, photoface_remote_path))
		{
			return_code=PF_SUCCESS_RC;
		}
		
		sprintf(filename, "%s/standin.rgb", pf_job->working_path);
		if (read_rgb_image_file(filename, &number_of_components,
			&number_of_bytes_per_component, &file_height, &file_width, &image))
		{
			switch (image_format)
			{
				case PF_RGB_IMAGE:
				{
					destination_components = 3;
				} break;
				case PF_RGBA_IMAGE:
				{
					destination_components = 4;
				} break;
			}
			if ((number_of_bytes_per_component == 1) && (number_of_components >= destination_components))
			{
				if ((file_width == width) && (file_height == height))
				{
					src_image_ptr = (char *) image;
					dest_image_ptr = texture;
					for (i = 0 ; i < width * height ; i++)
					{
						for (j = 0 ; j < destination_components ; j++)
						{
							*(dest_image_ptr + j) = *(src_image_ptr + j);
						}
						dest_image_ptr += destination_components;
						src_image_ptr += number_of_components;
					}
					return_code=PF_SUCCESS_RC;
				}
				else
				{
#if defined (ERROR_MESSAGE)
					set_error_message(PF_ERROR_MESSAGE,
						"File image dimensions are not what was expected in file %s",
						filename);
#endif /* defined (ERROR_MESSAGE) */
					return_code=PF_GENERAL_FAILURE_RC;
				}
			}
			else
			{
#if defined (ERROR_MESSAGE)
				set_error_message(PF_ERROR_MESSAGE, "Unsupported image format in file %s",
					filename);
#endif /* defined (ERROR_MESSAGE) */
				return_code=PF_GENERAL_FAILURE_RC;
			}
			DEALLOCATE(image);
		}
		else
		{
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE, "Unable to read image from file %s",
				filename);
#endif /* defined (ERROR_MESSAGE) */
			return_code=PF_READ_FILE_FAILURE_RC;
		}
		save_state_Pf_job_and_unlock(&pf_job);
	}
	LEAVE;

	return (return_code);
} /* pf_get_texture */

int pf_get_hair_model(int pf_job_id,int *number_of_vertices,
	float **vertex_3d_locations,int *number_of_texture_vertices,
	float **texture_vertex_3d_locations,int *number_of_triangles,
	int **triangle_vertices,int **triangle_texture_vertices)
/*******************************************************************************
LAST MODIFIED : 21 June 2001

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
	struct Pf_job *pf_job;

	ENTER(pf_get_head_model);
	if(PF_SUCCESS_RC == (return_code = 
		get_Pf_job_from_id_and_lock(pf_job_id, &pf_job)))
	{
		return_code=PF_SUCCESS_RC;
		/* Read the fitted obj file and return all the values */
		sprintf(filename, "%s/standin_hair_halo.obj", pf_job->working_path);
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
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE, "Unable to open file %s for reading",
				filename);
#endif /* defined (ERROR_MESSAGE) */
			return_code= PF_OPEN_FILE_FAILURE_RC;
		}
		save_state_Pf_job_and_unlock(&pf_job);
	}
	LEAVE;

	return (return_code);
} /* pf_get_head_model */

int pf_specify_hair_mask(int pf_job_id,int width,int height,
	enum PF_image_format image_format,char *image)
/*******************************************************************************
LAST MODIFIED : 21 June 2001

DESCRIPTION :
Used to specify the image to be texture mapped onto the model.
==============================================================================*/
{
	char *image_ptr, filename[200];
	float texture_ndc_x, temp_texture_ndc_y, texture_ndc_y, texture_ndc_width,
		texture_ndc_height;
	FILE *image_file;
	int i, image_components, return_code;
	struct Pf_job *pf_job;

	ENTER(pf_specify_hair_mask);
	if(PF_SUCCESS_RC == (return_code = 
		get_Pf_job_from_id_and_lock(pf_job_id, &pf_job)))
	{
		return_code=PF_GENERAL_FAILURE_RC;
		/* Create the rgb image for this image and make the image square by padding */
		sprintf(filename, "%s/source_hair_mask.raw", pf_job->working_path);
		switch (image_format)
		{
			case PF_RGB_IMAGE:
			{
				image_components = 3;
			} break;
			case PF_RGBA_IMAGE:
			{
				image_components = 4;
			} break;
		}
		if (image_file = fopen(filename, "wb"))
		{
			image_ptr = image;
			for (i = 0 ; i < width * height ; i++)
			{
				fwrite(image_ptr, 1, 3, image_file);
				image_ptr += image_components;
			}
			fclose(image_file);

			/* Calculate the texture placement coordinates in ndc space */
			texture_ndc_x = - pf_job->ndc_texture_offset_x * pf_job->ndc_texture_scaling
				- 1.0f;
			temp_texture_ndc_y = - pf_job->ndc_texture_offset_y * pf_job->ndc_texture_scaling
				- 1.0f;
			texture_ndc_y = pf_job->ndc_texture_offset_y * pf_job->ndc_texture_scaling
				- 1.0f;
			texture_ndc_width = ((float)width - pf_job->ndc_texture_offset_x) *
				pf_job->ndc_texture_scaling - 1.0f - texture_ndc_x;
			texture_ndc_height = ((float)height - pf_job->ndc_texture_offset_y)
				* pf_job->ndc_texture_scaling - 1.0f - temp_texture_ndc_y;

			if (linux_execute(
				"%sbin/cmgui_control -id %06d  '$hair_width=%d;$hair_height=%d;cmiss(qq(open comfile %scmiss/pf_specify_hair_mask.com exec))'",
				photoface_remote_path, pf_job_id, width, height, photoface_remote_path))
			{
				return_code=PF_SUCCESS_RC;
			}
		}
		else
		{
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE, "Unable to open file %s for writing",
				filename);
#endif /* defined (ERROR_MESSAGE) */
			return_code=PF_OPEN_FILE_FAILURE_RC;
		}
		save_state_Pf_job_and_unlock(&pf_job);
	}
	LEAVE;

	return (return_code);
} /* pf_specify_hair_mask */

int pf_get_hair_texture(int pf_job_id,int width,int height,
	enum PF_image_format image_format, char *texture)
/*******************************************************************************
LAST MODIFIED : 26 June 2001

DESCRIPTION :
The caller specifies the texture size and provides the storage.  The <texture>
is filled in based on the current model.
==============================================================================*/
{
	char filename[200], *src_image_ptr, *dest_image_ptr;
	int destination_components, i, j, number_of_bytes_per_component, number_of_components, return_code;
	long int file_height, file_width;
	long unsigned *image;
	struct Pf_job *pf_job;

	ENTER(pf_get_hair_texture);
	if(PF_SUCCESS_RC == (return_code = 
		get_Pf_job_from_id_and_lock(pf_job_id, &pf_job)))
	{
		return_code=PF_GENERAL_FAILURE_RC;
		/* Use cmgui to calculate the texture based on the specified image */
		if (linux_execute(
				 "%sbin/cmgui_control -id %06d  '$width=%d;$height=%d;cmiss(qq(open comfile %scmiss/pf_get_hair_texture.com exec))'",
				 photoface_remote_path, pf_job_id, width, height, photoface_remote_path))
		{
			return_code=PF_SUCCESS_RC;
		}
		
		sprintf(filename, "%s/hair.rgb", pf_job->working_path);
		if (read_rgb_image_file(filename, &number_of_components,
			&number_of_bytes_per_component, &file_height, &file_width, &image))
		{
			switch (image_format)
			{
				case PF_RGB_IMAGE:
				{
					destination_components = 3;
				} break;
				case PF_RGBA_IMAGE:
				{
					destination_components = 4;
				} break;
			}
			if ((number_of_bytes_per_component == 1) && (number_of_components >= destination_components))
			{
				if ((file_width == width) && (file_height == height))
				{
					src_image_ptr = (char *) image;
					dest_image_ptr = texture;
					for (i = 0 ; i < width * height ; i++)
					{
						for (j = 0 ; j < destination_components ; j++)
						{
							*(dest_image_ptr + j) = *(src_image_ptr + j);
						}
						dest_image_ptr += destination_components;
						src_image_ptr += number_of_components;
					}
					return_code=PF_SUCCESS_RC;
				}
				else
				{
#if defined (ERROR_MESSAGE)
					set_error_message(PF_ERROR_MESSAGE,
						"File image dimensions are not what was expected in file %s",
						filename);
#endif /* defined (ERROR_MESSAGE) */
					return_code=PF_GENERAL_FAILURE_RC;
				}
			}
			else
			{
#if defined (ERROR_MESSAGE)
				set_error_message(PF_ERROR_MESSAGE, "Unsupported image format in file %s",
					filename);
#endif /* defined (ERROR_MESSAGE) */
				return_code=PF_GENERAL_FAILURE_RC;
			}
			DEALLOCATE(image);
		}
		else
		{
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE, "Unable to read image from file %s",
				filename);
#endif /* defined (ERROR_MESSAGE) */
			return_code=PF_READ_FILE_FAILURE_RC;
		}
		save_state_Pf_job_and_unlock(&pf_job);
	}
	LEAVE;

	return (return_code);
} /* pf_get_hair_texture */

int pf_get_distorted_background(int pf_job_id,int width,int height,
	enum PF_image_format image_format, char *texture)
/*******************************************************************************
LAST MODIFIED : 26 June 2001

DESCRIPTION :
The caller specifies the texture size and provides the storage.  The <texture>
is filled in based on the current model.
==============================================================================*/
{
	char filename[200], *src_image_ptr, *dest_image_ptr;
	int destination_components, i, j, number_of_bytes_per_component, number_of_components, return_code;
	long int file_height, file_width;
	long unsigned *image;
	struct Pf_job *pf_job;

	ENTER(pf_get_distorted_background);
	if(PF_SUCCESS_RC == (return_code = 
		get_Pf_job_from_id_and_lock(pf_job_id, &pf_job)))
	{
		return_code=PF_GENERAL_FAILURE_RC;
		/* Use image magick to distort the background image */
		if (linux_execute(
				 "%sbin/cmgui_control -id %06d '$width=%d;$height=%d;cmiss(qq(open comfile %scmiss/pf_get_distorted_background.com exec))'",
				 photoface_remote_path, pf_job_id, width, height, photoface_remote_path))
		{
			return_code=PF_SUCCESS_RC;
		}
		
		sprintf(filename, "%s/source_image_distorted.rgb", pf_job->working_path);
		if (read_rgb_image_file(filename, &number_of_components,
			&number_of_bytes_per_component, &file_height, &file_width, &image))
		{
			switch (image_format)
			{
				case PF_RGB_IMAGE:
				{
					destination_components = 3;
				} break;
				case PF_RGBA_IMAGE:
				{
					destination_components = 4;
				} break;
			}
			if ((number_of_bytes_per_component == 1) && (number_of_components >= destination_components))
			{
				if ((file_width == width) && (file_height == height))
				{
					src_image_ptr = (char *) image;
					dest_image_ptr = texture;
					for (i = 0 ; i < width * height ; i++)
					{
						for (j = 0 ; j < destination_components ; j++)
						{
							*(dest_image_ptr + j) = *(src_image_ptr + j);
						}
						dest_image_ptr += destination_components;
						src_image_ptr += number_of_components;
					}
					return_code=PF_SUCCESS_RC;
				}
				else
				{
#if defined (ERROR_MESSAGE)
					set_error_message(PF_ERROR_MESSAGE,
						"File image dimensions are not what was expected in file %s",
						filename);
#endif /* defined (ERROR_MESSAGE) */
					return_code=PF_GENERAL_FAILURE_RC;
				}
			}
			else
			{
#if defined (ERROR_MESSAGE)
				set_error_message(PF_ERROR_MESSAGE, "Unsupported image format in file %s",
					filename);
#endif /* defined (ERROR_MESSAGE) */
				return_code=PF_GENERAL_FAILURE_RC;
			}
			DEALLOCATE(image);
		}
		else
		{
#if defined (ERROR_MESSAGE)
			set_error_message(PF_ERROR_MESSAGE, "Unable to read image from file %s",
				filename);
#endif /* defined (ERROR_MESSAGE) */
			return_code=PF_READ_FILE_FAILURE_RC;
		}
		save_state_Pf_job_and_unlock(&pf_job);
	}
	LEAVE;

	return (return_code);
} /* pf_get_distorted_background */
