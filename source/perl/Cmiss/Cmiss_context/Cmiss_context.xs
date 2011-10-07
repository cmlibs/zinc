#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "api/zn_context.h"
#include "context/context.h"
#include "finite_element/finite_element.h"
#include "general/debug.h"
#include "selection/element_selection.h"
#include "selection/node_selection.h"
#include "typemap.h"

/* I have had to expand this out to a plain list and then extend the stack 
	in the XS code below because I could not get the stack to manipulate properly
	in the for_each_element routine. */
struct Element_list_data
{
	int count;
	struct FE_element **list;
};

static int XS_Cmiss_Cmiss_context_add_element_to_list(struct FE_element *element,
	void *element_list_data_void)
{
	struct Element_list_data *data;

	if (data = (struct Element_list_data *)element_list_data_void)
	{
		data->list[data->count] = element;
		data->count++;
	}
	return(1);
}

/* I have had to expand this out to a plain list and then extend the stack 
	in the XS code below because I could not get the stack to manipulate properly
	in the for_each_element routine. */
struct Node_list_data
{
	int count;
	struct FE_node **list;
};

static int XS_Cmiss_Cmiss_context_add_node_to_list(struct FE_node *node,
	void *node_list_data_void)
{
	struct Node_list_data *data;

	if (data = (struct Node_list_data *)node_list_data_void)
	{
		data->list[data->count] = node;
		data->count++;
	}
	return(1);
}

MODULE = Cmiss::Cmiss_context		PACKAGE = Cmiss::Cmiss_context	PREFIX = Cmiss_context_

PROTOTYPES: DISABLE

Cmiss::Cmiss_context
create(argv)
   AV *argv;
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_value structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_value_2=$cmiss_value_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
	   {
	      char **c_argv;
	      unsigned int argc, i;
			SV **elem;

			argc = av_len(argv) + 1;
			New(0, c_argv, argc, char *);
			for (i = 0 ; i < argc ; i++)
			{
				elem = av_fetch(argv, i, 0);
				c_argv[i] = SvPV(*elem, PL_na);
			}
			RETVAL=Cmiss_context_create("cmgui perl");
			Cmiss_context_enable_user_interface(RETVAL, argc, c_argv);
			Safefree(c_argv);
	   }
	OUTPUT:
		RETVAL

int
destroy(Cmiss::Cmiss_context value)
	CODE:
		{
			struct Cmiss_context *temp_value;

			temp_value=value;
			RETVAL=Cmiss_context_destroy(&temp_value);
		}
	OUTPUT:
		RETVAL

int
execute_command(Cmiss::Cmiss_context cmiss_context, char *name)
	CODE:
	{
	   int quit = 0;
		execute_command(name, (void *)cmiss_context, &quit, &RETVAL);
	}
	OUTPUT:
	RETVAL

Cmiss::Region
Cmiss_context_get_default_region(Cmiss::Cmiss_context cmiss_context)

void
Cmiss_context_get_element_selection(Cmiss::Cmiss_context cmiss_context)
	PPCODE:
	{
		int i;
		struct FE_element_selection *element_selection;
		struct LIST(FE_element) *selected_element_list;
		struct Element_list_data data;
		SV *sv;
		if ((element_selection = Cmiss_context_get_element_selection(
			cmiss_context)) &&
			(selected_element_list = FE_element_selection_get_element_list(
			element_selection)))
		{
			data.count = 0;
			data.list = malloc(sizeof(struct FE_element *) * NUMBER_IN_LIST(FE_element)(selected_element_list));
			FOR_EACH_OBJECT_IN_LIST(FE_element)(XS_Cmiss_Cmiss_context_add_element_to_list,
				(void *)&data, selected_element_list);
			EXTEND(SP, data.count);
			for (i = 0 ; i < data.count ; i++)
			{
				sv = sv_newmortal();
				sv_setref_pv(sv, "Cmiss::Element", (void *)(ACCESS(FE_element)(data.list[i])));
				PUSHs(sv);
			}
			free(data.list);
		}
	}

void
Cmiss_context_get_node_selection(Cmiss::Cmiss_context cmiss_context)
	PPCODE:
	{
		int i;
		struct FE_node_selection *node_selection;
		struct LIST(FE_node) *selected_node_list;
		struct Node_list_data data;
		SV *sv;
		if ((node_selection = Cmiss_context_get_node_selection(
			cmiss_context)) &&
			(selected_node_list = FE_node_selection_get_node_list(
			node_selection)))
		{
			data.count = 0;
			data.list = malloc(sizeof(struct FE_node *) * NUMBER_IN_LIST(FE_node)(selected_node_list));
			FOR_EACH_OBJECT_IN_LIST(FE_node)(XS_Cmiss_Cmiss_context_add_node_to_list,
				(void *)&data, selected_node_list);
			EXTEND(SP, data.count);
			for (i = 0 ; i < data.count ; i++)
			{
				sv = sv_newmortal();
				sv_setref_pv(sv, "Cmiss::Node", (void *)(ACCESS(FE_node)(data.list[i])));
				PUSHs(sv);
			}
			free(data.list);
		}
	}
