#if !defined (XVG_INTERFACE_H)
#define XVG_INTERFACE_H

#include "finite_element/finite_element.h"
#include "user_interface/user_interface.h"

void xvg_popup_interface(struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct User_interface *user_interface);

/* whether to show the node number when digitising */
extern int data_draw_number;

int LoadFile(char *name);
#endif /* !defined (XVG_INTERFACE_H) */
