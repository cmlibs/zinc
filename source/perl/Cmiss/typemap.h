#if ! defined (PERL_CMISS_TYPEMAP_H)
#define PERL_CMISS_TYPEMAP_H

/* SAB Currently in cmgui it's command data is called struct Cmiss_command_data,
	in perl I am giving it a more specific name */
typedef struct Cmiss_command_data * Cmiss__cmgui_command_data;
typedef struct FE_element * Cmiss__FE_element;
typedef struct FE_field * Cmiss__FE_field;
typedef struct FE_node * Cmiss__FE_node;

#endif /* ! defined (PERL_CMISS_TYPEMAP_H) */
