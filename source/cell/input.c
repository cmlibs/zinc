/*******************************************************************************
FILE : input.c

LAST MODIFIED : 16 September 1999

DESCRIPTION :
Functions for handling all input for CELL.
==============================================================================*/
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <Xm/Xm.h>
#include "cell/cell_component.h"
#include "cell/cell_window.h"
#include "cell/input.h"
#include "cell/parser.h"
#include "cell/model_dialog.h"
#include "cell/cell_parameter.h"
#include "cell/cell_variable.h"
#include "cell/calculate.h"

#define URL_STRING "http://"
#define ABS_PATH_STRING "/"
#define FILENAME_START_CHAR '/'

/*
Local types
===========
*/

/*
Local functions
===============
*/
static struct URI *get_uri(char *full_name,char *parent_path)
/*******************************************************************************
LAST MODIFIED : 27 January 1999

DESCRIPTION :
Takes the <full_name> of a file and separates it into the path and the file
name. <parent_path> is added to any relative paths.
==============================================================================*/
{
  struct URI *uri = (struct URI *)NULL;
  int i,j;
  char *tmp_string;
  
  ENTER(get_uri);
  if (ALLOCATE(uri,struct URI,1))
  {
    uri->path = (char *)NULL;
    uri->filename = (char *)NULL;
    i = strlen(full_name);
    while ((full_name[i-1] != FILENAME_START_CHAR) && (i > 1))
    {
      i--;
    }
    if (i > 1)
    {
      /* some path information included in the full_name */
      if ((ALLOCATE(uri->path,char,i+1)) &&
        ALLOCATE(uri->filename,char,strlen(full_name)-i+1))
      {
        for (j=0;j<i;j++)
        {
          uri->path[j] = full_name[j];
        }
        uri->path[i] = '\0';
        for (j=i;j<strlen(full_name);j++)
        {
          uri->filename[j-i] = full_name[j];
        }
        uri->filename[strlen(full_name)-i] = '\0';
      }
      else
      {
        display_message(ERROR_MESSAGE,"get_uri. "
          "Unable to allocate memmory for the path or filename (%s)",full_name);
        DEALLOCATE(uri);
        uri = (struct URI *)NULL;
      }
    }
    else
    {
      /* no path information */
      uri->path = (char *)NULL;
      if (ALLOCATE(uri->filename,char,strlen(full_name)))
      {
        for (j=0;j<strlen(full_name);j++)
        {
          uri->filename[j] = full_name[j];
        }
        uri->filename[strlen(full_name)] = '\0';
      }
      else
      {
        display_message(ERROR_MESSAGE,"get_uri. "
          "Unable to allocate memmory for the filename (%s)",full_name);
        DEALLOCATE(uri);
        uri = (struct URI *)NULL;
      }
    }
    if ((uri != (struct URI *)NULL) && (parent_path != (char *)NULL))
    {
      /* add the parent path to any relative paths */
      if ((uri->path == (char *)NULL) && (uri->filename != (char *)NULL))
      {
        /* only a file name specified so add parent path as the whole path */
        if (ALLOCATE(uri->path,char,strlen(parent_path)+1))
        {
          sprintf(uri->path,"%s\0",parent_path);
        }
        else
        {
          display_message(ERROR_MESSAGE,"get_uri. "
            "Unable to allocate memory for the parent path (%s)",parent_path);
          DEALLOCATE(uri);
          uri = (struct URI *)NULL;
        }
      }
      else if ((strncmp(uri->path,URL_STRING,strlen(URL_STRING))) &&
        (strncmp(uri->path,ABS_PATH_STRING,strlen(ABS_PATH_STRING))))
      {
        /* the path exists, but does not begin with either the URL_STRING or
        the ABS_PATH_STRING, so add the parent path to the relative path. */
        if (ALLOCATE(tmp_string,char,strlen(uri->path)))
        {
          strcpy(tmp_string,uri->path);
          DEALLOCATE(uri->path);
          if (ALLOCATE(uri->path,char,strlen(tmp_string)+strlen(parent_path)+1))
          {
            sprintf(uri->path,"%s%s\0",parent_path,tmp_string);
            DEALLOCATE(tmp_string);
          }
        }
      }
    } /* if ((uri != (struct URI *)NULL) && (parent_path != (char *)NULL)) */
    if ((uri != (struct URI *)NULL) && (uri->path == (char *)NULL))
    {
      /* need to set the path to a zero length string to avoid errors
      with strlen() */
      uri->path = "\0";
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"get_uri. "
      "Unable to allocate memory for the URI (%s)",full_name);
    uri = (struct URI *)NULL;
  }
  LEAVE;
  return(uri);
} /* END get_uri() */

#if defined (OLD_CODE)
static char *get_experiment_filename(struct XML_Tree *tree)
/*******************************************************************************
LAST MODIFIED : 27 January 1999

DESCRIPTION :
Returns the experimental specification URI from the given <tree>.
==============================================================================*/
{
  int num_tags;
  char **tags,*full_name;
  char *return_filename;
  
  ENTER(get_experiment_filename);
  num_tags = 1;
  tags = cell_create_tag_list(num_tags,"cell-experiment-specification");
  if (tags != (char **)NULL)
  {
    return_filename = cell_get_attribute(tree,tags,num_tags,"uri");
    cell_destroy_tag_list(tags,num_tags);
  }
  else
  {
    display_message(ERROR_MESSAGE,"get_experiment_filename. "
      "Unable to create tag list.");
  }
  LEAVE;
  return(return_filename);
} /* END get_experiment_filename() */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static char *get_parameter_filename(struct XML_Tree *tree)
/*******************************************************************************
LAST MODIFIED : 27 January 1999

DESCRIPTION :
Returns the parameter specification URI from the given <tree>.
==============================================================================*/
{
  int num_tags;
  char **tags,*full_name;
  char *return_filename;
  
  ENTER(get_parameter_filename);
  num_tags = 1;
  tags = cell_create_tag_list(num_tags,"cell-parameter-specification");
  if (tags != (char **)NULL)
  {
    return_filename = cell_get_attribute(tree,tags,num_tags,"uri");
  }
  else
  {
    display_message(ERROR_MESSAGE,"get_parameter_filename. "
      "Unable to create tag list.");
  }
  LEAVE;
  return(return_filename);
} /* END get_parameter_filename() */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static char *get_version(struct XML_Tree *tree)
/*******************************************************************************
LAST MODIFIED : 28 January 1999

DESCRIPTION :
Returns a pointer to the version field of the given <tree> assuming that the
version is always found in the top level of the <tree>.
==============================================================================*/
{
  char *version;
  int num_tags;
  char **tags;
  
  ENTER(get_version);
  num_tags = 1;
  tags = cell_create_tag_list(num_tags,"version");
  if (tags != (char **)NULL)
  {
    version = cell_get_content(tree,tags,num_tags);
  }
  else
  {
    display_message(ERROR_MESSAGE,"get_version. "
      "Unable to set the element tags");
    version = (char *)NULL;
  }
  LEAVE;
  return(version);
} /* END get_version() */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static int check_versions(int num, ... )
/*******************************************************************************
LAST MODIFIED : 28 January 1999

DESCRIPTION :
Checks each of the trees given for the same version. <num> specifies the number
of trees to be checked. ( ... represents a variable length list of XML_Tree *
structures)
==============================================================================*/
{
  int return_code,i;
  va_list ap;
  char *version1,*version2;
  struct XML_Tree *tree1,*tree2;

  ENTER(check_versions);
  if (num > 1)
  {
    /* initialise the va_list (see man va_arg) */
    va_start(ap,num);
    tree1 = va_arg(ap,struct XML_Tree *);
    tree2 = va_arg(ap,struct XML_Tree *);
    return_code = 1;
    for (i=2;(i<=num) && return_code;i++)
    {
      version1 = get_version(tree1);
      version2 = get_version(tree2);
      if (version1 && version2)
      {
        if (strcmp(version1,version2))
        {
          return_code = 0;
        }
        else
        {
          tree1 = tree2;
          tree2 = va_arg(ap,struct XML_Tree *);
        }
      }
    }
    /* close the va_list */
    va_end(ap);
  }
  else
  {
    return_code = 1;
  }
  LEAVE;
  return(return_code);
} /* END check_versions() */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static int merge_trees(struct XML_Tree *root_tree,int num, ... )
/*******************************************************************************
LAST MODIFIED : 28 January 1999

DESCRIPTION :
Merges the given list of trees <...> and merges them into the <root_tree>. Any
elements already in the <root_tree> are dropped from the sub-trees. <num>
specifies the number of trees given in the list <...> of trees. The trees are
assumed to correspond to elements in the top level of the tree.
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(merge_trees);
  return_code = 1;
  LEAVE;
  return(return_code);
} /* END merge_trees() */
#endif /* defined (OLD_CODE) */

/*
Global functions
================
*/
#if defined (OLD_CODE)
struct XML_Tree create_cell_tree(char *filename,int *return_code)
/*******************************************************************************
LAST MODIFIED : 28 January 1999

DESCRIPTION :
Creates the main XML tree from the cell specification file <filename>.
Returns <return_code> as 1 if tree successfully created, otherwise returns 0

(1) Parse the cell specification file which should give
    - an experimental specification URI and/or elements which should
      replace those given in the file
    - a parameter specification URI and/or elements which should replace those
      given in the file
(2) Parse the experimental specification file and add to the tree, with
    appropriate elements replaced by those given in the cell spec. file
(3) Parse the parameter specification file and add to the tree, with
    appropriate elements replaced by those given in the cell spec. file
==============================================================================*/
{
  struct XML_Tree cell_tree,parameter_tree,experiment_tree;
  struct URI *experiment_uri,*parameter_uri,*cell_uri;
  char *tmp_filename;
  
  ENTER(create_cell_tree);
  *return_code = 0;
  /* set the cell URI for use in the parameter and experiment URI's if
     required to resolve relative paths */
  cell_uri = get_uri(filename,(char *)NULL);
  if (cell_uri)
  {
    cell_tree = parse_cell_uri(cell_uri,return_code);
    if (*return_code)
    {
      tmp_filename = get_experiment_filename(&cell_tree);
      if (tmp_filename != (char *)NULL)
      {
        experiment_uri = get_uri(tmp_filename,cell_uri->path);
      }
      tmp_filename = get_parameter_filename(&cell_tree);
      if (tmp_filename != (char *)NULL)
      {
        parameter_uri = get_uri(tmp_filename,cell_uri->path);
      }
      if (experiment_uri && parameter_uri)
      {
        experiment_tree = parse_cell_uri(experiment_uri,return_code);
        if (*return_code)
        {
          parameter_tree = parse_cell_uri(parameter_uri,return_code);
          if (*return_code)
          {
            /* check the versions of the three trees */
            if (check_versions(3,&cell_tree,&experiment_tree,&parameter_tree))
            {
              if (!merge_trees(&cell_tree,2,&experiment_tree,&parameter_tree))
              {
                display_message(ERROR_MESSAGE,"create_cell_tree. "
                  "Cannot merge the trees");
                *return_code = 0;
              }
              else
              {
                *return_code = 1;
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,"create_cell_tree. "
                "File versions do not match");
              *return_code = 0;
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"create_cell_tree. "
              "Unable to create the parameter tree");
            *return_code = 0;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"create_cell_tree. "
            "Unable to create the experiment tree");
          *return_code = 0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"create_cell_tree. "
          "URI's not found");
        *return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"create_cell_window. "
        "Unable to create the main cell tree");
      *return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_cell_tree. "
      "Cell URI not found");
    *return_code = 0;
  }
  LEAVE;
  return(cell_tree);
} /* END create_cell_tree() */
#endif /* defined (OLD_CODE) */

char **cell_create_tag_list(int num, ... )
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
Takes a list of character arrays and generates a single array of character
pointers containing each of the individual tags. <num> gives the number of tags
to be added to the array.
==============================================================================*/
{
  va_list ap;
  char **tag_list = (char **)NULL;
  int i;
  
  ENTER(cell_create_tag_list);
  /* initialise the va_list (see man va_arg) */
  va_start(ap,num);
  /* now add each of the tags to the tag_list */
  if (ALLOCATE(tag_list,char *,num))
  {
    for (i=0;i<num;i++)
    {
      tag_list[i] = va_arg(ap,char *);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"cell_create_tag_list. "
      "Unable to allocate memory for the tag_list");
    tag_list = (char **)NULL;
  }
  /* close the va_list */
  va_end(ap);
  LEAVE;
  return(tag_list);
} /* END cell_create_tag_list() */

void cell_destroy_tag_list(char **tags,int num_tags)
/*******************************************************************************
LAST MODIFIED : 26 January 1999

DESCRIPTION :
Frees the memory used by a list of element tags.
==============================================================================*/
{
  int i;
  
  ENTER(cell_destroy_tag_list);
  for (i=0;i<num_tags;i++)
  {
    /* DPN ??? this deallocation gives a segmentation fault in free() under
       Linux on my PC but appears to work fine under IRIX (6.52) ??? */ 
#if defined (SGI)
    DEALLOCATE(tags[i]);
#endif /* if defined (SGI) */
    tags[i] = (char *)NULL;
  }
  DEALLOCATE(tags);
  tags = (char **)NULL;
  LEAVE;
} /* END cell_destroy_tag_list() */

#if defined (OLD_CODE)
int read_variables_file(char *filename,XtPointer cell_window)
/*******************************************************************************
LAST MODIFIED : 27 February 1999

DESCRIPTION :
The function called when a variables file is selected via the file selection
box.
==============================================================================*/
{
  int return_code = 0,er = 0,i;
  struct Cell_window *cell = (struct Cell_window *)NULL;
  struct URI *uri = (struct URI *)NULL;
  struct XML_Tree tree;
  struct XML_Element *current_element = (struct XML_Element *)NULL;
  
  ENTER(read_variables_file);
  if ((cell = (struct Cell_window *)cell_window) && filename)
  {
    if (uri = get_uri(filename,(char *)NULL))
    {
      tree = parse_cell_uri(uri,&er);
      if (er)
      {
        /*display_message(INFORMATION_MESSAGE,"read_variables_file. "
          "Parsed the file **%s**\n",filename);*/
        /* first, reset the list of variables */
        destroy_cell_variables(cell);
        return_code = 1;
        for (i=0;(i<(tree.root)->number_of_children) && return_code;i++)
        {
          current_element = (tree.root)->children[i];
          return_code = set_variable_information(cell,
            current_element->children[0]->content,                 /* name */
            current_element->children[1]->content,                 /* label */
            current_element->children[2]->attributes[0].content,  /* units */
            current_element->children[2]->attributes[1].content, /* spatial */
            current_element->children[2]->attributes[2].content, /* T.V. */
            current_element->children[2]->content,                /* value */
            1);
        }
        if (return_code)
        {
          /* some variable information was set, so want to recreate the
             variables dialog */
          close_variables_dialog(cell);
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"read_variables_file. "
          "Unable to parse the file");
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"read_variables_file. "
        "Unable to set the URI");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"read_variables_file. "
      "Incorrect arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END read_variables_file() */
#endif /* defined (OLD_CODE) */

int read_model_file(char *filename,XtPointer cell_window)
/*******************************************************************************
LAST MODIFIED : 08 September 1999

DESCRIPTION :
Read the model file <filename>, and set-up the <cell_window>.
==============================================================================*/
{
  int return_code = 0,er = 0,i,j;
  struct Cell_window *cell = (struct Cell_window *)NULL;
  struct URI *uri = (struct URI *)NULL;
  struct XML_Tree tree;
  struct XML_Element *current_element = (struct XML_Element *)NULL;
  char *file_name;
  
  ENTER(read_model_file);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (filename)
    {
      if (uri = get_uri(filename,(char *)NULL))
      {
        tree = parse_cell_uri(uri,&er);
        if (er)
        {
          /*display_message(INFORMATION_MESSAGE,"read_model_file. "
            "Parsed the file **%s**\n",filename);*/
          /* first set the variables */
          /* reset the variables array if reading the default values, else
             want to add/overwrite variables */
          if (cell->default_values)
          {
            destroy_cell_variables(cell);
            destroy_cell_outputs(cell);
          }
          return_code = 1;
          for (i=0;(i<(tree.root)->children[0]->number_of_children) &&
                 return_code;i++)
          {
            current_element = (tree.root)->children[0]->children[i];
            return_code = set_variable_information(cell,
              current_element->children[0]->attributes[0].content, /* array*/
              current_element->children[0]->attributes[1].content, /*post'n*/
              current_element->children[0]->content,               /* name */
              current_element->children[1]->content,              /* label */
              current_element->children[2]->attributes[0].content,/* units */
              current_element->children[2]->attributes[1].content,/*spatial*/
              current_element->children[2]->attributes[2].content,/* T.V. */
              current_element->children[2]->content,              /* value */
              cell->default_values);
            if (return_code && cell->default_values)
            {
              /* set each variable as an output */
              return_code = set_output_information(cell,
                current_element->children[0]->content,"Variable");
            }
          }
          if (return_code)
          {
            /* some variable information was set, so want to recreate the
               variables dialog */
            close_variables_dialog(cell);
            /* now read in the parameters file */
            /* assume that the default file contains all parameters, in the
               right order */
            if (cell->default_values)
            {
              destroy_cell_parameters(cell);
            }
            return_code = 1;
            for (i=0;(i<(tree.root)->children[1]->number_of_children)
                   && return_code;i++)
            {
              current_element = (tree.root)->children[1]->children[i];
              return_code = set_parameter_information(cell,
                current_element->children[0]->attributes[0].content, /* array*/
                current_element->children[0]->attributes[1].content, /*post'n*/
                current_element->children[0]->content,               /* name */
                current_element->children[1]->content,              /* label */
                current_element->children[2]->attributes[0].content,/* units */
                current_element->children[2]->attributes[1].content,/*spatial*/
                current_element->children[2]->attributes[2].content, /* T.V. */
                current_element->children[2]->content,              /* value */
                cell->default_values);
              if (cell->default_values)
              {
                /* only set the components when reading the default values,
                   as these are assumed to be the complete set */
                /* set the cell components */
                for (j=3;(j<current_element->number_of_children)
                       &&return_code;j++)
                {
                  return_code = set_parameter_cell_component(cell,
                    current_element->children[j]->content);
                }
              }
            }
            if (return_code)
            {
              /* some parameter information was set so need to destroy
                 the current list of cell components and recreate it */
              destroy_cell_component_list(cell);
              return_code = create_cell_component_list(cell);
              if (return_code)
              {
                /* now define the cell components */
                return_code = 1;
                for (i=0;(i<(tree.root)->children[2]->number_of_children)
                       && return_code;i++)
                {
                  current_element = (tree.root)->children[2]->children[i];
                  return_code = set_component_graphical_information(cell,
                    current_element->attributes[0].content,/*type*/
                    current_element->attributes[1].content,/*posx*/
                    current_element->attributes[2].content,/*posy*/
                    current_element->attributes[3].content,/*posz*/
                    current_element->attributes[4].content,/*dirx*/
                    current_element->attributes[5].content,/*diry*/
                    current_element->attributes[6].content,/*dirz*/
                    current_element->attributes[7].content,/*sca*/
                    current_element->attributes[8].content,/*sca*/
                    current_element->attributes[9].content,/*sca*/
                    current_element->content); /* name */
                } /* for - cell components */
                if (return_code)
                {
                  /* now define the cell graphics */
                  return_code = 1;
                  for (i=0;(i<(tree.root)->children[3]->number_of_children)
                         && return_code;i++)
                  {
                    current_element = (tree.root)->children[3]->children[i];
                    if (strlen(
                      current_element->children[0]->attributes[0].content) > 0)
                    {
                      if (current_element->children[0]->attributes[0].content[0]
                        != '/')
                      {
                        if (ALLOCATE(file_name,char,strlen(uri->path)+strlen(
                          current_element->children[0]->attributes[0].content)))
                        {
                          sprintf(file_name,"%s%s\0",uri->path,
                           current_element->children[0]->attributes[0].content);
                        }
                      }
                      else
                      {
                        if (ALLOCATE(file_name,char,strlen(
                          current_element->children[0]->attributes[0].content)))
                        {
                          sprintf(file_name,"%s\0",
                           current_element->children[0]->attributes[0].content);
                        }
                      }
                    }
                    else
                    {
                      file_name = (char *)NULL;
                    }
                    return_code = set_graphics_information(cell,file_name,
                      current_element->children[0]->content, /* name/type */
                      current_element->children[1]->attributes[0].content,
                      current_element->children[1]->attributes[1].content,
                      current_element->children[1]->attributes[2].content,
                      current_element->children[1]->attributes[3].content,
                      current_element->children[1]->attributes[4].content,
                      current_element->children[1]->attributes[5].content,
                      current_element->children[1]->attributes[6].content,
                      current_element->children[1]->attributes[7].content,
                      current_element->children[1]->attributes[8].content,
                      current_element->children[1]->attributes[9].content,
                      current_element->children[1]->attributes[10].content,
                      current_element->children[1]->attributes[11].content,
                      current_element->children[1]->attributes[12].content,
                      current_element->children[1]->attributes[13].content);
                  } /* for - cell graphics */
                  /* *** */
                  /* Now set-up the Cell 3D scene */
                  /* *** */
                  return_code = draw_cell_3d(cell);
                }
#if defined (OLD_CODE)
                /* redraw the image map */
                draw_image_map(cell);
                /* now want to set-up additional outputs */
                if (cell->default_values)
                {
                  if (tree.root->number_of_children > 2)
                  {
                    return_code = 1;
                    for (i=0;(i<(tree.root)->children[2]->number_of_children)
                           && return_code;i++)
                    {
                      current_element = (tree.root)->children[2]->children[i];
                      return_code = set_output_information(cell,
                        current_element->content,    /* name */
                        current_element->attributes[0].content);
                    }
                  }
                }
#endif /* defined (OLD_CODE) */
              }
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"read_model_file. "
              "No variables found");
            return_code = 0;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"read_model_file. "
            "Unable to parse the file : **%s**",filename);
          return_code = 0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"read_model_file. "
          "Unable to get the URI for the filename : **%s**",filename);
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"read_model_file. "
        "Missing filename");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"read_model_file. "
      "Missing Cell window");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END read_model_file() */

#if defined (OLD_CODE)
int read_parameters_file(char *filename,XtPointer cell_window)
/*******************************************************************************
LAST MODIFIED : 06 February 1999

DESCRIPTION :
The function called when a parameters file is selected via the file selection
box.
==============================================================================*/
{
  int return_code = 0,er = 0,i,j;
  struct Cell_window *cell = (struct Cell_window *)NULL;
  struct URI *uri = (struct URI *)NULL;
  struct XML_Tree tree;
  struct XML_Element *current_element = (struct XML_Element *)NULL;
  
  ENTER(read_parameters_file);
  if ((cell = (struct Cell_window *)cell_window) && filename)
  {
    if (uri = get_uri(filename,(char *)NULL))
    {
      tree = parse_cell_uri(uri,&er);
      if (er)
      {
        /*display_message(INFORMATION_MESSAGE,"read_parameters_file. "
          "Parsed the file **%s**\n",filename);*/
        /* first, reset the list of variables */
        destroy_cell_parameters(cell);
        return_code = 1;
        for (i=0;(i<(tree.root)->number_of_children) && return_code;i++)
        {
          current_element = (tree.root)->children[i];
          return_code = set_parameter_information(cell,
            current_element->children[0]->content,                  /* name */
            current_element->children[1]->content,                 /* label */
            current_element->children[2]->attributes[0].content,   /* units */
            current_element->children[2]->attributes[1].content, /* spatial */
            current_element->children[2]->content,1);              /* value */
          /* set the cell components */
          for (j=3;(j<current_element->number_of_children)&&return_code;j++)
          {
            return_code = set_parameter_cell_component(cell,
              current_element->children[j]->content);
          }
        }
        if (return_code)
        {
          /* some parameter information was set so need to destroy the current
          list of cell components and recreate it */
          destroy_cell_component_list(cell);
          return_code = create_cell_component_list(cell);
          if (return_code)
          {
#if defined (OLD_CODE)
            /* redraw the image map */
            draw_image_map(cell);
#endif /* defined (OLD_CODE) */
          }
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"read_parameters_file. "
          "Unable to parse the file");
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"read_parameters_file. "
        "Unable to set the URI");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"read_parameters_file. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END read_parameters_file() */
#endif /* defined (OLD_CODE) */

void verify_text_field_modification(Widget widget,XtPointer unused,
  XtPointer callback_struct)
/*******************************************************************************
LAST MODIFIED : 13 February 1999

DESCRIPTION :
Callback for when a text field's value is changed. Ensures that only numbers
are entered.
==============================================================================*/
{
  XmTextVerifyCallbackStruct *cbs;
  int len = 0;
  int i;
  
  ENTER(verify_text_field_modification);
  USE_PARAMETER(widget);
  USE_PARAMETER(unused);
  if (cbs = (XmTextVerifyCallbackStruct *)callback_struct)
  {
    if (!(cbs->startPos < cbs->currInsert)) /* if not delete */
    {
      /* go through the additional text making sure it is acceptable */
      for (len=0;len<(cbs->text->length);len++)
      {
        if ((!isdigit(cbs->text->ptr[len])) && (cbs->text->ptr[len] != 'e') &&
          (cbs->text->ptr[len] != 'E') && (cbs->text->ptr[len] != '+') &&
          (cbs->text->ptr[len] != '-') && (cbs->text->ptr[len] != '.'))
        {
          for (i=len;(i+1)<(cbs->text->length);i++)
          {
            cbs->text->ptr[i] = cbs->text->ptr[i+1];
          }
          cbs->text->length--;
          len--;
        }
      }
      if ((cbs->text->length) == 0)
      {
        cbs->doit = False;
      }
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"check_text_field_modification. "
      "Missing callback structure");
  }
  LEAVE;
} /* END check_text_field_modification() */

int extract_float_from_text_field(Widget text,float *value)
/*******************************************************************************
LAST MODIFIED : 13 February 1999

DESCRIPTION :
Extracts a float <value> from the text field widget <text>.
==============================================================================*/
{
  int return_code = 0;
  char *string;

  ENTER(extract_float_from_text_field);
  if (text)
  {
    XtVaGetValues(text,
      XmNvalue,&string,
      NULL);
    /* check that a float is found in the string */
    if (sscanf(string,"%f",value) == 1)
    {
      return_code = 1;
    }
    XtFree(string);
  }
  else
  {
    display_message(ERROR_MESSAGE,"extract_float_from_text_field. "
      "Missing text field widget");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END extract_float_from_text_field() */

int read_model_names(struct Cell_window *cell,char *filename)
/*******************************************************************************
LAST MODIFIED : 27 February 1999

DESCRIPTION :
Reads and sets the model names from <filename>
==============================================================================*/
{
  int return_code = 0;
  int error = 0,i,j;
  struct URI *uri = (struct URI *)NULL;
  struct URI *model_uri = (struct URI *)NULL;
  struct XML_Tree tree;
  struct XML_Element *current_element = (struct XML_Element *)NULL;

  ENTER(read_model_names);
  if (filename)
  {
    if (uri = get_uri(filename,(char *)NULL))
    {
      tree = parse_cell_uri(uri,&error);
      if (error)
      {
        /*display_message(INFORMATION_MESSAGE,"read_model_names. "
          "Parsed the file **%s**\n",filename);*/
        /* set the model names */
        for (i=0;i<tree.root->number_of_children;i++)
        {
          current_element = tree.root->children[i];
          for (j=0;j<current_element->number_of_children;j++)
          {
            if (model_uri =
              get_uri(current_element->children[j]->attributes[0].content,
                uri->path))
            {
              set_model_name(cell,i+1,current_element->children[j]->content,
                model_uri,current_element->children[j]->attributes[1].content);
              return_code = 1;
            }
            else
            {
              display_message(ERROR_MESSAGE,"read_model_names. "
                "Unable to get the model URI for the model %s",
                current_element->children[j]->content);
              return_code = 0;
            }
          } /* for (j...) */
        } /* for (i...) */
      }
      else
      {
        display_message(ERROR_MESSAGE,"read_model_names. "
          "Unable to parse the file: %s",filename);
        return_code = 0;
      }
      DEALLOCATE(uri);
    }
    else
    {
      display_message(ERROR_MESSAGE,"read_model_names. "
        "Unable to set the URI");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"read_model_names. "
      "Missing file name");
    return_code = 0;
  }  
  LEAVE;
  return(return_code);
} /* END read_model_names() */
