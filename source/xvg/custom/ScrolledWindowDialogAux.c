/****************************************************************************
*
*    File:      ScrolledWindowAux.c
*    Author:    Paul Charette
*    Modified:  25 September 1995
*
*    Purpose:   various initialisation & utility routines
*                 ScrolledWindowOkCB()
*                 SelectFENodeGroup()
*****************************************************************************/
struct Find_selected_position_data
{
	int current_position,selected_position;
}; /* struct Find_selected_position_data */

static int find_selected_position(struct GROUP(FE_node) *node_group,
	void *void_find_selected_position_data)
{
	int return_code;
	struct Find_selected_position_data *find_selected_position_data;

	/* init return code */
	return_code=0;

#if defined (CMGUI)
	if (node_group&&(find_selected_position_data=
		(struct Find_selected_position_data *)void_find_selected_position_data))
	{
		if (find_selected_position_data->current_position<
			find_selected_position_data->selected_position)
		{
			(find_selected_position_data->current_position)++;
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
#endif

	return (return_code);
} /* find_selected_position */

static void ScrolledWindowOkCB(void)
{
  XmString *xs;
  struct GROUP(FE_node) *node_group;
  char *group_name,*s, cbuf[256];
  int *pos, selectedpos, cnt, i;
	struct Find_selected_position_data find_selected_position_data;

  /* remove interface */
  UxPopdownInterface(ScrolledWindowDialog);

#if defined (CMGUI)
  /* get the selected item index */
  XmListGetSelectedPos(ScrolledWindowDialogSLW, &pos, &cnt);
  /* check for no selection */
  if (cnt <= 0)
	{
		CurrentFENodeGroup = NULL;
	}
  else
	{
		/* item selected, check for consistency between group list and XmList */
		/* load selected pos index */
		selectedpos = pos[0];
		/* load the XmList string at the selected position */
		XtVaGetValues(ScrolledWindowDialogSLW, XmNitems, &xs, NULL);
		XmStringGetLtoR(xs[selectedpos-1], XmSTRING_DEFAULT_CHARSET, &s);
		/* find selected item in the linked list */
		find_selected_position_data.current_position=1;
		find_selected_position_data.selected_position=selectedpos;
		node_group=FIRST_OBJECT_IN_MANAGER_THAT(GROUP(FE_node))(
			find_selected_position,(void *)&find_selected_position_data,
			xvg_node_group_manager);
		/* check for list length exceeded */
		if (node_group == NULL)
		{
			ErrorMsg("ScrolledWindowOkCB() : List length mismatch, try again");
		}
		/* check that selected item names matches that in the group listing */
		else
		{
      if (GET_NAME(GROUP(FE_node))(node_group,&group_name))
      {
        if (strcmp(group_name,s) != 0)
        {
          sprintf(cbuf,
		"ScrolledWindowOkCB() : List inconsistency (%s not equal to %s), try again",
            group_name, s);
          ErrorMsg(cbuf);
        }
        else
        {
          /* everything ok, make this group the current one */
          /* set current group pointer */
          CurrentFENodeGroup = node_group;
          RedrawProcDrawingArea();
        }
        DEALLOCATE(group_name);
      }
    }
	}
#endif
}

static int add_FE_node_group_to_XmList(struct GROUP(FE_node) *node_group,
	void *void_position)
{
	char *group_name,*current_group_name;
	int *position,return_code;

	/* init return code */
	return_code=0;

#if defined (CMGUI)
	if (node_group&&(position=(int *)void_position))
	{
		if (return_code=GET_NAME(GROUP(FE_node))(node_group,&group_name))
    {
      XmListAddItem(ScrolledWindowDialogSLW,
        XmStringCreateSimple(group_name),*position);
      /* select current group if it exists */
      if (CurrentFENodeGroup)
      {
        /*???RC Why compare name of group, why not just the address? */
        if (GET_NAME(GROUP(FE_node))(CurrentFENodeGroup,&current_group_name))
        {
          if (0==strcmp(group_name,current_group_name))
          {
            XmListSelectPos(ScrolledWindowDialogSLW,*position,
              TRUE);
          }
          DEALLOCATE(current_group_name);
        }
      }
      (*position)++;
      DEALLOCATE(group_name);
    }
	}
	else
	{
		return_code=0;
	}
#endif
	return (return_code);
} /* add_FE_node_group_to_XmList */

static void ScanAllFENodeGroups(void)
{
  int position;

#if defined (CMGUI)
  /* clear Xm list */
  XmListDeleteAllItems(ScrolledWindowDialogSLW);
  /* build group list */
	position=1;
	FOR_EACH_OBJECT_IN_MANAGER(GROUP(FE_node))(add_FE_node_group_to_XmList,
		(void *)&position,xvg_node_group_manager);
#endif
}

void SelectFENodeGroup(void)
{
  ScanAllFENodeGroups();
  XtVaSetValues(ScrolledWindowDialog,
		XmNtitle, "FE node groups", NULL);
  UxPopupInterface(ScrolledWindowDialog, no_grab);
}
