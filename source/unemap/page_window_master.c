/*******************************************************************************
FILE : page_window.c

LAST MODIFIED : 21 March 1998

DESCRIPTION :

???DB.  General design of tools
- have a create_tool that is passed a parent and returns a tool structure
- have an open_tool that is passed the address of a pointer to a tool structure.
	If the point is not NULL then it is displayed/brought to the front/opened
	(assumes that the parent is a shell).  Otherwise a tool and a shell are
	created and it displayed/brought to the front/opened.
==============================================================================*/
#include <stddef.h>
#include <limits.h>
#if defined (MOTIF)
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Composite.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
#include <windows.h>
#include <commctrl.h>
#if defined (NI_DAQ)
#include "nidaq.h"
/* for RTSI */
#include "nidaqcns.h"
#endif /* defined (NI_DAQ) */
#endif /* defined (WINDOWS) */
#include "general/debug.h"
#include "unemap/page_window.h"
#if defined (MOTIF)
#include "unemap/page_window.uid64"
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
#include "unemap/page_window.rc"
#endif /* defined (WINDOWS) */
#include "unemap/rig.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#if defined (WINDOWS)
#include "unemap/vunemapd.h"
#endif /* defined (WINDOWS) */

/*
Module constants
----------------
*/
#if defined (NI_DAQ)
#define NUMBER_OF_CHANNELS_ON_NI_CARD ((i16)64)
#endif /* defined (NI_DAQ) */

/*
Module types
------------
*/
#if defined (NI_DAQ)
struct NI_card
{
	i16 device_number;
	HGLOBAL memory_object;
	i16 *hardware_buffer;
	i16 time_base;
	u16 sampling_interval;
	u32 hardware_buffer_size;
}; /* struct NI_card */
#endif /* defined (NI_DAQ) */

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int page_window_hierarchy_open=0;
static MrmHierarchy page_window_hierarchy;
#endif /* defined (MOTIF) */

#if defined (NI_DAQ)
int number_of_ni_cards=0;
struct NI_card *ni_cards=(struct NI_card *)NULL;
#endif /* defined (NI_DAQ) */

/*
Module functions
----------------
*/
static int acquire_write_signal_file(char *file_name,void *rig_pointer)
/*******************************************************************************
LAST MODIFIED : 21 April 1997

DESCRIPTION :
This function writes the rig configuration and interval of signal data to the
named file.
==============================================================================*/
{
	FILE *output_file;
	int return_code;
	struct Rig *rig;

	ENTER(acquire_write_signal_file);
	/* check that the rig exists */
	if (rig= *((struct Rig **)rig_pointer))
	{
		/* open the output file */
		if (output_file=fopen(file_name,"wb"))
		{
			return_code=write_signal_file(output_file,rig);
			fclose(output_file);
			if (!return_code)
			{
				remove(file_name);
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"acquire_write_signal_file.  Invalid file: %s",
				file_name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"acquire_write_signal_file.  Missing rig");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* acquire_write_signal_file */

#if defined (MOTIF)
static void identify_page_calibrate_button(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 31 December 1996

DESCRIPTION :
Finds the id of the page calibrate button.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_calibrate_button);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->calibrate_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_calibrate_button.  page window missing");
	}
	LEAVE;
} /* identify_page_calibrate_button */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_reset_button(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 31 December 1996

DESCRIPTION :
Finds the id of the page reset button.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_reset_button);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->reset_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_reset_button.  page window missing");
	}
	LEAVE;
} /* identify_page_reset_button */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_close_button(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 31 December 1996

DESCRIPTION :
Finds the id of the page close button.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_close_button);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->close_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_close_button.  page window missing");
	}
	LEAVE;
} /* identify_page_close_button */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_page_drawing_area(Widget *widget_id,
	XtPointer page_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 31 December 1996

DESCRIPTION :
Finds the id of the page drawing area.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(identify_page_drawing_area);
	if (page=(struct Page_window *)page_window_structure)
	{
		page->drawing_area= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_page_drawing_area.  page window missing");
	}
	LEAVE;
} /* identify_page_drawing_area */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void destroy_Page_window(Widget widget,XtPointer page_window_structure,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 31 December 1996

DESCRIPTION :
This function expects <page_window_structure> to be a pointer to an page window
structure.  If the <address> field of the page window is not NULL, <*address> is
set to NULL.  If the <activation> field is not NULL, the <activation> widget is
unghosted.  The function frees the memory associated with the fields of the
page window and frees the memory associated with the page window.
==============================================================================*/
{
	struct Page_window *page;

	ENTER(destroy_Page_window);
	if (page=(struct Page_window *)page_window_structure)
	{
		/* set the pointer to the page window to NULL */
		if (page->address)
		{
			*(page->address)=(struct Page_window *)NULL;
		}
		/* unghost the activation button */
		if (page->activation)
		{
			XtSetSensitive(page->activation,True);
		}
		/* free the page window memory */
		DEALLOCATE(page);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_Page_window.  page_window_structure missing");
	}
	LEAVE;
} /* destroy_Page_window */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static Widget create_page_window_shell(Widget *address,Widget parent,
	int screen_width,int screen_height)
		/*???DB.  Position and size should be passed ? */
/*******************************************************************************
LAST MODIFIED : 31 December 1996

DESCRIPTION :
Creates a popup shell widget for an page window.  If <address> is not NULL,
<*address> is set to the id of the created shell and on destruction <*address>
will be set to NULL.  The id of the created widget is returned.
???If address is NULL, it won't create an entry in the shell list ?
==============================================================================*/
{
	Widget shell;

	ENTER(create_page_window_shell);
	/* create and place the page window shell */
	if (shell=XtVaCreatePopupShell("page_window_shell",
		topLevelShellWidgetClass,parent,
		XmNallowShellResize,True,
		XmNx,screen_width/2,
		XmNy,0,
		XmNwidth,screen_width/2,
		XmNheight,screen_height/2,
		NULL))
	{
		if (address)
		{
			*address=shell;
			/* add the destroy callback */
			XtAddCallback(shell,XmNdestroyCallback,destroy_window_shell,
				(XtPointer)create_Shell_list_item(address));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_page_window_shell.  Could create the page window shell");
	}
	LEAVE;

	return (shell);
} /* create_page_window_shell */
#endif /* defined (MOTIF) */

#if defined (WINDOWS)
static LRESULT CALLBACK Page_window_drawing_area_class_proc(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 6 March 1998

DESCRIPTION :
???DB.  Should return 0 if it processes the message.
==============================================================================*/
{
	CREATESTRUCT *wm_create_structure;
	static HBRUSH fill_brush;
	struct Page_window *page_window;
	HDC device_context;
	LRESULT return_code;
#if defined (NI_DAQ)
	i16 status;
#endif /* defined (NI_DAQ) */
/*???debug */
static POINT scroll_line[2],signal_line[5],x_axis[2];
static RECT fill_rectangle;

	ENTER(Page_window_drawing_area_class_proc);
	switch (message_identifier)
	{
		case WM_CLOSE:
		{
			/* destroy fill brush */
			DeleteObject(fill_brush);
			DestroyWindow(window);
			return_code=0;
		} break;
		case WM_CREATE:
		{
#if defined (NI_DAQ)
			f64 sampling_rate;
			i16 channel_vector[NUMBER_OF_CHANNELS_ON_NI_CARD],device_code,
				device_number,gain_vector[NUMBER_OF_CHANNELS_ON_NI_CARD],i,time_base;
			struct NI_card *temp_ni_cards;
			struct Signal_buffer *signal_buffer;
			u16 sampling_interval;
			u32 hardware_buffer_size;
#endif /* defined (NI_DAQ) */

			/* set the fill colour and type */
			fill_brush=CreateSolidBrush(0x00ffffff);
			/* retrieve the acquisition window out of <window> */
			wm_create_structure=(CREATESTRUCT *)second_message;
			page_window=(struct Page_window *)(wm_create_structure->lpCreateParams);
				/*???DB.  Check for NT - see WM_CREATE */
			/* set the data */
			SetWindowLong(window,0,(LONG)page_window);
#if defined (MIRADA)
			/* start the interrupting */
			if (INVALID_HANDLE_VALUE!=page_window->device_driver)
			{
				start_interrupting(page_window->device_driver,window,WM_USER,
					(LPARAM)page_window);
			}
#endif /* defined (MIRADA) */
#if defined (NI_DAQ)
			signal_buffer=(((*(page_window->rig_address))->devices)[0])->signal->
				buffer;
			/* convert sampling rate (S/s) into timebase and sampling
				interval */
			sampling_rate=(f64)(signal_buffer->frequency)*
				(f64)(page_window->number_of_channels);
			status=DAQ_Rate(sampling_rate,/* rate is S/s */(i16)0,
				&time_base,&sampling_interval);
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","w"))
	{
		fprintf(debug_nidaq,"DAQ_Rate=%d %g %d %d %x %x\n",status,sampling_rate,
			time_base,sampling_interval,time_base,sampling_interval);
		fclose(debug_nidaq);
	}
}
			if (0==status)
			{
				for (i=0;i<NUMBER_OF_CHANNELS_ON_NI_CARD;i++)
				{
					channel_vector[i]=i;
					gain_vector[i]=(i16)1;
				}
				hardware_buffer_size=(u32)(NUMBER_OF_CHANNELS_ON_NI_CARD*
					(signal_buffer->number_of_samples));
				/* search for NI cards */
				number_of_ni_cards=0;
				ni_cards=(struct NI_card *)NULL;
				device_number=1;
				while ((0==Init_DA_Brds(device_number,&device_code))&&(0<device_number))
				{
					if ((220/*PCI-6031E*/==device_code)||(222/*PCI-6033E*/==device_code))
					{
						if (REALLOCATE(temp_ni_cards,ni_cards,struct NI_card,
							number_of_ni_cards+1))
						{
							ni_cards=temp_ni_cards;
							(ni_cards[number_of_ni_cards]).device_number=device_number;
							(ni_cards[number_of_ni_cards]).memory_object=(HGLOBAL)NULL;
							(ni_cards[number_of_ni_cards]).hardware_buffer=(i16 *)NULL;
							(ni_cards[number_of_ni_cards]).time_base=time_base;
							(ni_cards[number_of_ni_cards]).sampling_interval=
								sampling_interval;
							(ni_cards[number_of_ni_cards]).hardware_buffer_size=
								hardware_buffer_size;
							number_of_ni_cards++;
						}
						else
						{
							display_message(ERROR_MESSAGE,
					"Page_window_drawing_area_class_proc.  Could not allocate ni_cards");
							number_of_ni_cards=0;
							DEALLOCATE(ni_cards);
							device_number=0;
						}
					}
					device_number++;
				}
					/*???DB.  To be done */
				/* allocate memory for cards */
				if (ni_cards)
				{
					/* configure cards */
					i=0;
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,"number_of_ni_cards=%d\n",number_of_ni_cards);
		fprintf(debug_nidaq,"hardware_buffer_size=%d\n",hardware_buffer_size);
		fclose(debug_nidaq);
	}
}
					while ((0==status)&&(i<number_of_ni_cards))
					{
						/* configure the card */
						status=AI_Configure((ni_cards[i]).device_number,
							(i16)(-1),/* all channels */
							(i16)2,/* non-referenced single-ended */
							(i16)0,/* ignored (input range) */
							(i16)0,/* bipolar */
							(i16)0/* do not drive AISENSE to ground */
							);
/*???debug */
{
	FILE *debug_nidaq;
	u32 mode;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,"AI_Configure=%d\n",status);
		status=Get_DAQ_Device_Info((ni_cards[i]).device_number,ND_DATA_XFER_MODE_AI,
			&mode);
		fprintf(debug_nidaq,"ND_DATA_XFER_MODE_AI %d %d %d %d\n",mode,
			ND_UP_TO_1_DMA_CHANNEL,ND_UP_TO_2_DMA_CHANNELS,ND_INTERRUPTS);
		fclose(debug_nidaq);
	}
}
						if (0==status)
						{
							/* allocate buffer */
							if ((ni_cards[i]).memory_object=GlobalAlloc(GMEM_MOVEABLE,
								(DWORD)(((ni_cards[i]).hardware_buffer_size)*sizeof(i16))))
							{
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,"GlobalAlloc successful\n");
		fclose(debug_nidaq);
	}
}
								if ((ni_cards[i]).hardware_buffer=
									(i16 *)GlobalLock((ni_cards[i]).memory_object))
								{
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,"GlobalLock successful\n");
		fclose(debug_nidaq);
	}
}
									/* working from "Building Blocks" section, p.3-25 (pdf 65) in
										the "NI-DAQ User Manual" */
									/* configuration block.  Start with default settings except
										for changing to double buffering mode (DAQ_DB_Config).
										Could also use AI_Configure, AI_Max_Config, DAQ_Config,
										DAQ_StopTrigger_Config */
									status=DAQ_DB_Config((ni_cards[i]).device_number,
										/* enable */(i16)1);
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,"DAQ_DB_Config=%d\n",status);
		fclose(debug_nidaq);
	}
}
									if (0==status)
									{
										/* set the scan sequence and gain */
										status=SCAN_Setup((ni_cards[i]).device_number,
											NUMBER_OF_CHANNELS_ON_NI_CARD,channel_vector,gain_vector);
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,"SCAN_Setup=%d\n",status);
		fclose(debug_nidaq);
	}
}
										if (0==status)
										{
											/* first card is the "master */
											if (0==i)
											{
												/* set up the update messages */
													/*???DB.  DAQTrigVal0 (N) must be an even divisor of
														the buffer size in scans */
												status=Config_DAQ_Event_Message(
													(ni_cards[i]).device_number,
													/* add message */(i16)1,/* channel string */"AI0",
													/* send message every N scans */(i16)1,/* N */(i32)40,
													(i32)0,(u32)0,(u32)0,(u32)0,window,(i16)WM_USER,
													(u32)0);
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,"Config_DAQ_Event_Message=%d\n",status);
		fclose(debug_nidaq);
	}
}
												if (0==status)
												{
													/* set up the completion message */
													status=Config_DAQ_Event_Message(
														(ni_cards[i]).device_number,
														/* add message */(i16)1,/* channel string */"AI0",
														/* send message at completion */(i16)2,(i32)0,
														(i32)0,(u32)0,(u32)0,(u32)0,window,(i16)(WM_USER+1),
														(u32)0);
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,"Config_DAQ_Event_Message=%d\n",status);
		fclose(debug_nidaq);
	}
}
													if (0==status)
													{
														if (1<number_of_ni_cards)
														{
															/* configure RTSI */
																/*???DB.  To be done */
															/* set the first card to output the A/D conversion
																trigger */
															status=Select_Signal((ni_cards[i]).device_number,
																ND_RTSI_0,ND_IN_CONVERT,ND_HIGH_TO_LOW);
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,
			"Select_Signal(0,ND_RTSI_0,ND_IN_CONVERT,ND_HIGH_TO_LOW)=%d\n",status);
		fclose(debug_nidaq);
	}
}
															if (0!=status)
															{
																display_message(ERROR_MESSAGE,
"Page_window_drawing_area_class_proc.  Select_Signal(0,ND_RTSI_0,ND_IN_CONVERT,ND_HIGH_TO_LOW) failed");
															}
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
			"Page_window_drawing_area_class_proc.  Config_DAQ_Event_Message failed");
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
			"Page_window_drawing_area_class_proc.  Config_DAQ_Event_Message failed");
												}
											}
											else
											{
												if (1<number_of_ni_cards)
												{
													/* configure RTSI */
														/*???DB.  To be done */
													/* set the other cards to input the A/D conversion
														trigger */
													status=Select_Signal((ni_cards[i]).device_number,
														ND_IN_CONVERT,ND_RTSI_0,ND_HIGH_TO_LOW);
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,
			"Select_Signal(>0,ND_IN_CONVERT,ND_RTSI_0,ND_HIGH_TO_LOW)=%d\n",status);
		fclose(debug_nidaq);
	}
}
													if (0==status)
													{
														/* scanning won't actually start until it gets the
															conversion trigger */
														status=SCAN_Start((ni_cards[i]).device_number,
															(i16 *)((ni_cards[i]).hardware_buffer),
															(u32)((ni_cards[i]).hardware_buffer_size),
															(ni_cards[i]).time_base,
															(ni_cards[i]).sampling_interval,(i16)0,(u16)0);
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,"SCAN_Start(>0)=%d\n",status);
		fclose(debug_nidaq);
	}
}
														if (0!=status)
														{
															display_message(ERROR_MESSAGE,
								"Page_window_drawing_area_class_proc.  SCAN_Start(>0) failed");
															GlobalUnlock((ni_cards[i]).memory_object);
															GlobalFree((ni_cards[i]).memory_object);
															DAQ_DB_Config((ni_cards[i]).device_number,
																/* disable */(i16)0);
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
"Page_window_drawing_area_class_proc.  Select_Signal(>0,ND_IN_CONVERT,ND_RTSI_0,ND_HIGH_TO_LOW) failed");
													}
												}
											}
											if (0==status)
											{
												i++;
											}
											else
											{
												GlobalUnlock((ni_cards[i]).memory_object);
												GlobalFree((ni_cards[i]).memory_object);
												DAQ_DB_Config((ni_cards[i]).device_number,
													/* disable */(i16)0);
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
										"Page_window_drawing_area_class_proc.  SCAN_Setup failed");
											GlobalUnlock((ni_cards[i]).memory_object);
											GlobalFree((ni_cards[i]).memory_object);
											DAQ_DB_Config((ni_cards[i]).device_number,
												/* disable */(i16)0);
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
									"Page_window_drawing_area_class_proc.  DAQ_DB_Config failed");
										GlobalUnlock((ni_cards[i]).memory_object);
										GlobalFree((ni_cards[i]).memory_object);
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Page_window_drawing_area_class_proc.  GlobalLock failed");
									GlobalFree((ni_cards[i]).memory_object);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Page_window_drawing_area_class_proc.  GlobalAlloc failed");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Page_window_drawing_area_class_proc.  AI_Configure failed");
						}
					}
					if (0==status)
					{
/*???debug */
for (i=0;i<number_of_ni_cards;i++)
{
	{
		FILE *debug_nidaq;
		i16 stopped;
		u32 retrieved;

		if (debug_nidaq=fopen("nidaq.deb","a"))
		{
			status=DAQ_Check((ni_cards[i]).device_number,&stopped,&retrieved);
			fprintf(debug_nidaq,"WM_CREATE %d %d %d %d\n",i,status,stopped,
				retrieved);
			fclose(debug_nidaq);
		}
	}
}
						/* start the data acquisition */
						status=SCAN_Start((ni_cards[0]).device_number,
							(i16 *)((ni_cards[0]).hardware_buffer),
							(u32)((ni_cards[0]).hardware_buffer_size),(ni_cards[0]).time_base,
							(ni_cards[0]).sampling_interval,(i16)0,(u16)0);
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,"SCAN_Start(0)=%d\n",status);
/*		SCAN_Sequence_Retrieve((ni_cards[0]).device_number,
			NUMBER_OF_CHANNELS_ON_NI_CARD,channel_vector);
		for (i=0;i<NUMBER_OF_CHANNELS_ON_NI_CARD;i++)
		{
			fprintf(debug_nidaq,"  %d %d\n",i,channel_vector[i]);
		}*/
		fclose(debug_nidaq);
	}
}
						if (0!=status)
						{
							display_message(ERROR_MESSAGE,
								"Page_window_drawing_area_class_proc.  SCAN_Start(0) failed");
						}
					}
					if (0!=status)
					{
						while (i>0)
						{
							i--;
							GlobalUnlock((ni_cards[i]).memory_object);
							GlobalFree((ni_cards[i]).memory_object);
							DAQ_DB_Config((ni_cards[i]).device_number,/* disable */(i16)0);
						}
						DEALLOCATE(ni_cards);
						number_of_ni_cards=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
					"Page_window_drawing_area_class_proc.  Could not allocate ni_cards");
					number_of_ni_cards=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Page_window_drawing_area_class_proc.  DAQ_Rate failed");
				number_of_ni_cards=0;
				ni_cards=(struct NI_card *)NULL;
			}
#endif /* defined (NI_DAQ) */
			/*???DB.  Any other processing ? */
			return_code=DefWindowProc(window,message_identifier,first_message,
				second_message);
		} break;
		case WM_DESTROY:
		{
#if defined (MIRADA)
/*???debug */
unsigned long interrupt_count,start;

page_window=(struct Page_window *)GetWindowLong(window,0);
if (INVALID_HANDLE_VALUE!=page_window->device_driver)
{
	stop_interrupting(page_window->device_driver,&interrupt_count,&start);
}
#endif /* defined (MIRADA) */
#if defined (NI_DAQ)
			int i;

			if (ni_cards)
			{
/*???debug */
for (i=0;i<number_of_ni_cards;i++)
{
	{
		FILE *debug_nidaq;
		i16 stopped;
		u32 retrieved;

		if (debug_nidaq=fopen("nidaq.deb","a"))
		{
			status=DAQ_Check((ni_cards[i]).device_number,&stopped,&retrieved);
			fprintf(debug_nidaq,"WM_DESTROY %d %d %d %d\n",i,status,stopped,
				retrieved);
			fclose(debug_nidaq);
		}
	}
}
				/* stop continuous sampling */
				status=DAQ_Clear((ni_cards[0]).device_number);
/*???debug */
for (i=0;i<number_of_ni_cards;i++)
{
	{
		FILE *debug_nidaq;
		i16 stopped;
		u32 retrieved;

		if (debug_nidaq=fopen("nidaq.deb","a"))
		{
			status=DAQ_Check((ni_cards[i]).device_number,&stopped,&retrieved);
			fprintf(debug_nidaq,"WM_DESTROY %d %d %d %d\n",i,status,stopped,
				retrieved);
			fclose(debug_nidaq);
		}
	}
}
/*???debug */
for (i=0;i<number_of_ni_cards;i++)
{
	{
		FILE *debug_nidaq;
		i16 stopped;
		u32 retrieved;

		if (debug_nidaq=fopen("nidaq.deb","a"))
		{
			status=DAQ_Check((ni_cards[i]).device_number,&stopped,&retrieved);
			fprintf(debug_nidaq,"WM_DESTROY %d %d %d %d\n",i,status,stopped,
				retrieved);
			fclose(debug_nidaq);
		}
	}
}
/*???debug */
/*for (i=number_of_ni_cards-1;i>=0;i--)*/
				for (i=0;i<number_of_ni_cards;i++)
				{
/*???debug */
{
	FILE *debug_nidaq;
	i16 stopped;
	u32 retrieved;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		status=DAQ_Check((ni_cards[i]).device_number,&stopped,&retrieved);
		fprintf(debug_nidaq,"WM_DESTROY %d %d %d %d\n",i,status,stopped,retrieved);
		fclose(debug_nidaq);
	}
}
					/* stop continuous sampling */
					status=DAQ_Clear((ni_cards[i]).device_number);
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,"DAQ_Clear(%d)=%d\n",i,status);
		fclose(debug_nidaq);
	}
}
					GlobalUnlock((ni_cards[i]).memory_object);
					GlobalFree((ni_cards[i]).memory_object);
					/* turn off double buffering */
					status=DAQ_DB_Config((ni_cards[i]).device_number,/* disable */(i16)0);
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,"DAQ_DB_Config(%d)=%d\n",i,status);
		fclose(debug_nidaq);
	}
}
				}
/*???debug */
for (i=0;i<number_of_ni_cards;i++)
{
	{
		FILE *debug_nidaq;
		i16 stopped;
		u32 retrieved;

		if (debug_nidaq=fopen("nidaq.deb","a"))
		{
			status=DAQ_Check((ni_cards[i]).device_number,&stopped,&retrieved);
			fprintf(debug_nidaq,"WM_DESTROY %d %d %d %d\n",i,status,stopped,
				retrieved);
			fclose(debug_nidaq);
		}
	}
}
#endif /* defined (NI_DAQ) */
			}
			PostQuitMessage(0);
			return_code=0;
		} break;
		case WM_PAINT:
		{
			int i;
			PAINTSTRUCT paint_struct;

			/* check for update rectangle */
			if (TRUE==GetUpdateRect(window,(LPRECT)NULL,FALSE))
			{
				if (page_window=(struct Page_window *)GetWindowLong(window,0))
				{
					/* redraw the window */
					device_context=BeginPaint(window,&paint_struct);
						/*???DB.  I was using GetDC instead of BeginPaint, but this meant
							that WM_PAINT messages were being continuously sent to the window.
							BeginPaint/EndPaint should only be called in response to WM_PAINT
							(see Win32 SDK).  They must have something to do with removing the
							WM_PAINT message from the message queue */
						/*???DB.  BeginPaint/EndPaint should not be called if GetUpdateRect
							fails */
					GetClientRect(window,&fill_rectangle);
					(fill_rectangle.right)++;
					(fill_rectangle.bottom)++;
						/*???DB.  Seems to return a nonzero on success (not TRUE) */
					FillRect(device_context,&fill_rectangle,fill_brush);
					scroll_line[1].y=fill_rectangle.bottom;
					fill_rectangle.bottom -= 5;
					scroll_line[0].y=fill_rectangle.bottom;
					for (i=100;i<fill_rectangle.right;i += 100)
					{
						scroll_line[0].x=i;
						scroll_line[1].x=i;
						Polyline(device_context,scroll_line,2);
					}
					x_axis[0].x=0;
					x_axis[0].y=((page_window->display_maximum)*fill_rectangle.bottom)/
						((page_window->display_maximum)-(page_window->display_minimum));
					x_axis[1].x=fill_rectangle.right;
					x_axis[1].y=x_axis[0].y;
					Polyline(device_context,x_axis,2);
					fill_rectangle.left=1;
					fill_rectangle.right=5;
					x_axis[1].x=4;
					scroll_line[0].x=5;
					scroll_line[0].y=0;
					scroll_line[1].x=5;
					scroll_line[1].y=fill_rectangle.bottom;
#if defined (OLD_CODE)
					x_axis[0].x=0;
					x_axis[0].y=((page_window->display_maximum)*fill_rectangle.bottom)/
						((page_window->display_maximum)-(page_window->display_minimum));
					x_axis[1].x=fill_rectangle.right;
					x_axis[1].y=x_axis[0].y;
					Polyline(device_context,x_axis,2);
					fill_rectangle.right=2;
					x_axis[1].x=2;
					scroll_line[0].x=1;
					scroll_line[0].y=0;
					scroll_line[1].x=1;
					scroll_line[1].y=fill_rectangle.bottom;
#endif /* defined (OLD_CODE) */
					EndPaint(window,&paint_struct);
					return_code=0;
				}
				else
				{
					return_code=DefWindowProc(window,message_identifier,first_message,
						second_message);
				}
			}
			else
			{
				return_code=DefWindowProc(window,message_identifier,first_message,
					second_message);
			}
		} break;
		case WM_USER:
		{
			int i,j;
			long sum;
			RECT drawing_rectangle;
			short signal_value;
#if defined (MIRADA)
			short *hardware_buffer;
#endif /* defined (MIRADA) */
#if defined (NI_DAQ)
			i16 *hardware_buffer;
			int ni_card;
#endif /* defined (NI_DAQ) */
			unsigned long offset,number_of_channels,number_of_samples,sample_number;

/*???debug */
/*{
	FILE *debug_nidaq;
	i16 stopped;
	u32 retrieved;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		status=DAQ_Check(device,&stopped,&retrieved);
		fprintf(debug_nidaq,"WM_USER %d %d %d %d\n",status,stopped,retrieved,
			second_message);
		if (page_window=(struct Page_window *)GetWindowLong(window,0))
		{
			fprintf(debug_nidaq,"page_window %p %p\n",page_window,
				page_window->display_device);
		}
		else
		{
			fprintf(debug_nidaq,"No page_window\n");
		}
		fclose(debug_nidaq);
	}
}*/
			if ((page_window=(struct Page_window *)GetWindowLong(window,0))&&
				(page_window->display_device)&&
#if defined (MIRADA)
				(hardware_buffer=page_window->mirada_buffer)&&
#endif /* defined (MIRADA) */
#if defined (NI_DAQ)
				ni_cards&&(0<=(ni_card=(page_window->display_device->signal->index)/
				NUMBER_OF_CHANNELS_ON_NI_CARD))&&(ni_card<number_of_ni_cards)&&
				(hardware_buffer=(ni_cards[ni_card]).hardware_buffer)
#endif /* defined (NI_DAQ) */
				)
			{
				device_context=GetDC(window);
				GetClientRect(window,&drawing_rectangle);
					/*???DB.  Seems to return a nonzero on success (not TRUE) */
				sample_number=(unsigned long)second_message;
				number_of_channels=page_window->number_of_channels;
				number_of_samples=page_window->number_of_samples;
#if defined (NI_DAQ)
				sample_number %= number_of_samples;
#endif /* defined (NI_DAQ) */
				offset=number_of_channels*sample_number+
					((page_window->display_device->signal->index)%
					NUMBER_OF_CHANNELS_ON_NI_CARD);
				drawing_rectangle.bottom -= 5;
				signal_line[4].x=x_axis[0].x;
				signal_line[3].x=(x_axis[0].x)+1;
				signal_line[2].x=(x_axis[0].x)+2;
				signal_line[1].x=(x_axis[0].x)+3;
				signal_line[0].x=(x_axis[0].x)+4;
				signal_line[4].y=signal_line[0].y;
				for (j=0;j<4;j++)
				{
					sum=0;
					for (i=10;i>0;i--)
					{
						sum += (long)(hardware_buffer[offset]);
						if (offset<number_of_channels)
						{
							offset += number_of_channels*(number_of_samples-1);
						}
						else
						{
							offset -= number_of_channels;
						}
					}
					signal_value=(short)(sum/10);
					if (page_window->signal_maximum<page_window->signal_minimum)
					{
						page_window->signal_maximum=signal_value;
						page_window->signal_minimum=signal_value;
					}
					else
					{
						if (signal_value<page_window->signal_minimum)
						{
							page_window->signal_minimum=signal_value;
						}
						else
						{
							if (signal_value>page_window->signal_maximum)
							{
								page_window->signal_maximum=signal_value;
							}
						}
					}
					signal_line[j].y=(((page_window->display_maximum)-signal_value)*
						drawing_rectangle.bottom)/((page_window->display_maximum)-
						(page_window->display_minimum));
				}
				FillRect(device_context,&fill_rectangle,fill_brush);
				Polyline(device_context,x_axis,2);
				Polyline(device_context,scroll_line,2);
				fill_rectangle.left += 4;
				fill_rectangle.right += 4;
				scroll_line[0].x += 4;
				scroll_line[1].x += 4;
				if (x_axis[0].x>0)
				{
				  Polyline(device_context,signal_line,5);
				}
				else
				{
				  Polyline(device_context,signal_line,4);
				}
				x_axis[0].x += 4;
				x_axis[1].x += 4;
				if (x_axis[1].x>drawing_rectangle.right)
				{
					x_axis[0].x=0;
					x_axis[1].x=4;
					scroll_line[0].x=5;
					scroll_line[1].x=5;
					fill_rectangle.left=1;
					fill_rectangle.right=5;
				}
#if defined (OLD_CODE)
				sum=(long)(hardware_buffer[offset]);
				for (i=39;i>0;i--)
				{
					if (offset<number_of_channels)
					{
						offset += number_of_channels*(number_of_samples-1);
					}
					else
					{
						offset -= number_of_channels;
					}
					sum += (long)(hardware_buffer[offset]);
				}
				signal_value=(short)(sum/40);
/*???debug */
/*signal_value=(short)sample_number;
if (0==sample_number%3)
{
	signal_value=(short)number_of_channels;
}
else
{
	signal_value=(short)number_of_samples;
}*/
				if (page_window->signal_maximum<page_window->signal_minimum)
				{
					page_window->signal_maximum=signal_value;
					page_window->signal_minimum=signal_value;
				}
				else
				{
					if (signal_value<page_window->signal_minimum)
					{
						page_window->signal_minimum=signal_value;
					}
					else
					{
						if (signal_value>page_window->signal_maximum)
						{
							page_window->signal_maximum=signal_value;
						}
					}
				}
				device_context=GetDC(window);
				GetClientRect(window,&drawing_rectangle);
					/*???DB.  Seems to return a nonzero on success (not TRUE) */
				FillRect(device_context,&fill_rectangle,fill_brush);
				Polyline(device_context,x_axis,2);
				Polyline(device_context,scroll_line,2);
				(fill_rectangle.left)++;
				(fill_rectangle.right)++;
				(x_axis[1].x)++;
				(scroll_line[0].x)++;
				(scroll_line[1].x)++;
				signal_line[0].x=signal_line[1].x;
				signal_line[0].y=signal_line[1].y;
				signal_line[1].x=x_axis[0].x;
				signal_line[1].y=(((page_window->display_maximum)-signal_value)*
					drawing_rectangle.bottom)/((page_window->display_maximum)-
					(page_window->display_minimum));
				if (x_axis[0].x>0)
				{
				  Polyline(device_context,signal_line,2);
				}
				(x_axis[0].x)++;
				if (x_axis[0].x>drawing_rectangle.right)
				{
					x_axis[0].x=0;
					x_axis[1].x=2;
					scroll_line[0].x=1;
					scroll_line[1].x=1;
					fill_rectangle.left=0;
					fill_rectangle.right=2;
				}
#endif /* defined (OLD_CODE) */
			}
			return_code=0;
		} break;
		case WM_USER+1:
		{
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,"WM_USER+1 %d\n",second_message);
		fclose(debug_nidaq);
	}
}
		} break;
		default:
		{
			return_code=DefWindowProc(window,message_identifier,first_message,
				second_message);
		} break;
	}
	LEAVE;

	return (return_code);
} /* Page_window_drawing_area_class_proc */
#endif /* defined (WINDOWS) */

static int update_display_channel(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 8 September 1997

DESCRIPTION :
Reads the string in the channel field of the <page_window> and updates the
scrolling display.  Returns 1 if it is able to update, otherwise it returns 0
==============================================================================*/
{
#if defined (WINDOWS)
	int working_string_length;
#endif /* defined (WINDOWS) */
	char *working_string;
	int device_number,i,number_of_devices,return_code;
	struct Device **device_address,*display_device;

	if (page_window)
	{
		if (page_window->display_device)
		{
#if defined (MOTIF)
			/*???DB.  To be done */
			working_string=(char *)NULL;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			working_string_length=Edit_GetTextLength((page_window->channel).edit)+1;
				/*???DB.  GetWindowTextLength seems to give 1 less than the number of
					characters */
			if (ALLOCATE(working_string,char,working_string_length+1))
			{
				Edit_GetText((page_window->channel).edit,working_string,
					working_string_length);
#endif /* defined (WINDOWS) */
				/* set the display device */
				display_device=(struct Device *)NULL;
				device_address=(*(page_window->rig_address))->devices;
				number_of_devices=(*(page_window->rig_address))->number_of_devices;
				for (i=0;i<number_of_devices;i++)
				{
					if ((*device_address)&&((*device_address)->signal)&&
						((*device_address)->channel)&&
						(0<(*device_address)->channel->number)&&
#if defined (WINDOWS)
#if defined (MIRADA)
						((*device_address)->channel->number<=
						(int)page_window->number_of_channels)&&
#endif /* defined (MIRADA) */
#endif /* defined (WINDOWS) */
						((*device_address)->description)&&
						(0==strcmp(working_string,(*device_address)->description->name)))
					{
						display_device= *device_address;
						device_number=i;
					}
					device_address++;
				}
				if (display_device)
				{
					if (display_device!=page_window->display_device)
					{
/*???debug */
#if defined (NI_DAQ)
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,"change display_device %s, index %d, offset %d\n",
			display_device->description->name,display_device->signal->index,
			(display_device->signal->index)%NUMBER_OF_CHANNELS_ON_NI_CARD);
		fprintf(debug_nidaq,"number_of_channels=%d, number_of_samples=%d\n",
			page_window->number_of_channels,page_window->number_of_samples);
		fclose(debug_nidaq);
	}
}
#endif /* defined (NI_DAQ) */
						page_window->display_device=display_device;
						page_window->device_number=device_number;
#if defined (OLD_CODE)
/*???DB.  Don't want range reset when change channel */
						page_window->signal_maximum=0;
						page_window->signal_minimum=1;
#endif /* defined (OLD_CODE) */
#if defined (WINDOWS)
						InvalidateRect(page_window->drawing_area,(CONST RECT *)NULL,
							FALSE);
						Edit_SetText((page_window->channel).edit,
							page_window->display_device->description->name);
#endif /* defined (WINDOWS) */
					}
				}
				else
				{
#if defined (WINDOWS)
					Edit_SetText((page_window->channel).edit,
						page_window->display_device->description->name);
#endif /* defined (WINDOWS) */
				}
#if defined (OLD_CODE)
				if (channel_number!=page_window->channel_number)
				{
#if defined (WINDOWS)
#if defined (MIRADA)
					if (PCI_SUCCESSFUL==get_mirada_information(page_window->device_driver,
						&number_of_cards,&bus,&device_function,&number_of_channels,
						&number_of_samples,&mirada_buffer))
#endif /* defined (MIRADA) */
					{
						if ((0<channel_number)&&(channel_number<=(int)number_of_channels))
						{
#endif /* defined (WINDOWS) */
							page_window->channel_number=channel_number;
							channel_number--;
							page_window->channel_index=16*(channel_number/16)+
								8*(channel_number%2)+(channel_number%16)/2;
							page_window->signal_maximum=0;
							page_window->signal_minimum=1;
#if defined (WINDOWS)
							InvalidateRect(page_window->drawing_area,(CONST RECT *)NULL,
								FALSE);
						}
					}
#endif /* defined (WINDOWS) */
				}
#endif /* defined (OLD_CODE) */
#if defined (WINDOWS)
				DEALLOCATE(working_string);
			}
#endif /* defined (WINDOWS) */
		}
		else
		{
#if defined (WINDOWS)
			Edit_SetText((page_window->channel).edit," ");
#endif /* defined (WINDOWS) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_display_channel.  Missing page_window");
		return_code=0;
	}

	return (return_code);
} /* update_display_channel */

static int update_display_maximum(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 25 June 1997

DESCRIPTION :
Reads the string in the maximum field of the <page_window> and updates the
scrolling display.  Returns 1 if it is able to update, otherwise it returns 0
==============================================================================*/
{
#if defined (WINDOWS)
	char number_string[21],*working_string;
	int working_string_length;
#endif /* defined (WINDOWS) */
	float channel_gain,channel_offset,temp;
	int return_code;
	long int display_maximum;

	if (page_window)
	{
		if (page_window->display_device)
		{
			channel_gain=page_window->display_device->channel->gain;
			channel_offset=page_window->display_device->channel->offset;
#if defined (MOTIF)
			/*???DB.  To be done */
			display_maximum=page_window->display_maximum;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			working_string_length=Edit_GetTextLength((page_window->maximum).edit)+1;
				/*???DB.  GetWindowTextLength seems to give 1 less than the number of
					characters */
			if (ALLOCATE(working_string,char,working_string_length+1))
			{
				Edit_GetText((page_window->maximum).edit,working_string,
					working_string_length);
				if (1==sscanf(working_string,"%f",&temp))
				{
					display_maximum=(long int)(channel_offset+temp/channel_gain);
#endif /* defined (WINDOWS) */
					if (display_maximum!=page_window->display_maximum)
					{
						if (display_maximum>SHRT_MAX)
						{
							display_maximum=SHRT_MAX;
						}
						if (display_maximum>page_window->display_minimum)
						{
							page_window->display_maximum=display_maximum;
#if defined (WINDOWS)
							InvalidateRect(page_window->drawing_area,(CONST RECT *)NULL,
								FALSE);
#endif /* defined (WINDOWS) */
						}
					}
#if defined (WINDOWS)
				}
				DEALLOCATE(working_string);
			}
			sprintf(number_string,"%7.2f",
				channel_gain*((float)(page_window->display_maximum))-channel_offset);
			Edit_SetText((page_window->maximum).edit,number_string);
#endif /* defined (WINDOWS) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_display_maximum.  Missing page_window");
		return_code=0;
	}

	return (return_code);
} /* update_display_maximum */

static int update_display_minimum(struct Page_window *page_window)
/*******************************************************************************
LAST MODIFIED : 25 June 1997

DESCRIPTION :
Reads the string in the minimum field of the <page_window> and updates the
scrolling display.  Returns 1 if it is able to update, otherwise it returns 0
==============================================================================*/
{
#if defined (WINDOWS)
	char number_string[21],*working_string;
	int working_string_length;
#endif /* defined (WINDOWS) */
	float channel_gain,channel_offset,temp;
	int return_code;
	long int display_minimum;

	if (page_window)
	{
		if (page_window->display_device)
		{
			channel_gain=page_window->display_device->channel->gain;
			channel_offset=page_window->display_device->channel->offset;
#if defined (MOTIF)
			/*???DB.  To be done */
			display_minimum=page_window->display_minimum;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			working_string_length=Edit_GetTextLength((page_window->minimum).edit)+1;
				/*???DB.  GetWindowTextLength seems to give 1 less than the number of
					characters */
			if (ALLOCATE(working_string,char,working_string_length+1))
			{
				Edit_GetText((page_window->minimum).edit,working_string,
					working_string_length);
				if (1==sscanf(working_string,"%f",&temp))
				{
					display_minimum=(long int)(channel_offset+temp/channel_gain);
					if (display_minimum!=page_window->display_minimum)
					{
#endif /* defined (WINDOWS) */
						if (display_minimum<SHRT_MIN+1)
						{
							display_minimum=SHRT_MIN+1;
						}
						if (display_minimum<page_window->display_maximum)
						{
							page_window->display_minimum=display_minimum;
#if defined (WINDOWS)
							InvalidateRect(page_window->drawing_area,(CONST RECT *)NULL,
								FALSE);
#endif /* defined (WINDOWS) */
						}
#if defined (WINDOWS)
					}
				}
				DEALLOCATE(working_string);
			}
			sprintf(number_string,"%7.2f",
				channel_gain*((float)(page_window->display_minimum))-channel_offset);
			Edit_SetText((page_window->minimum).edit,number_string);
#endif /* defined (WINDOWS) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_display_minimum.  Missing page_window");
		return_code=0;
	}

	return (return_code);
} /* update_display_minimum */

#if defined (WINDOWS)
static void Page_window_WM_COMMAND_handler(HWND window,
	int item_control_accelerator_id,HWND control_window,UINT notify_code)
/*******************************************************************************
LAST MODIFIED : 21 March 1998

DESCRIPTION :
==============================================================================*/
{
	ENTER(Page_window_WM_COMMAND_handler);
	switch (item_control_accelerator_id)
		/*???DB.  Switch on <control_window> ? (when have no dialogs) */
	{
		/*???DB.  Will eventually call functions */
		case ACQUIRE_BUTTON:
		{
			struct Device **device;
			struct Page_window *page_window;
			struct Rig *rig;
#if defined (MIRADA)
			int i,index;
			short int *destination,*mirada_buffer,*source;
			struct Signal_buffer *signal_buffer;
			unsigned char *bus,*device_function;
			unsigned long interrupt_count,mirada_start,number_of_cards,
				number_of_channels,number_of_samples;
#endif /* defined (MIRADA) */
#if defined (NI_DAQ)
	int i;
	i16 status;
#endif /* defined (NI_DAQ) */

			if ((page_window=(struct Page_window *)GetWindowLong(window,
				DLGWINDOWEXTRA))&&(page_window->rig_address)&&
				(rig= *(page_window->rig_address))&&(device=rig->devices))
			{
#if defined (MIRADA)
				if (PCI_SUCCESSFUL==stop_interrupting(page_window->device_driver,
					&interrupt_count,&mirada_start))
				{
					if (PCI_SUCCESSFUL==get_mirada_information(page_window->device_driver,
						&number_of_cards,&bus,&device_function,&number_of_channels,
						&number_of_samples,&mirada_buffer))
					{
						/* copy the buffer */
						i=rig->number_of_devices;
						signal_buffer=(struct Signal_buffer *)NULL;
						while ((i>0)&&!signal_buffer)
						{
							if ((*device)&&((*device)->signal))
							{
								signal_buffer=(*device)->signal->buffer;
							}
							device++;
							i--;
						}
						if (signal_buffer)
						{
							destination=(signal_buffer->signals).short_int_values;
							signal_buffer->start=0;
							if (interrupt_count>number_of_samples)
							{
								source=mirada_buffer+
									(number_of_channels*(int)mirada_start);
								for (index=number_of_channels*
									(number_of_samples-(int)mirada_start);index>0;
									index--)
								{
									*destination= *source;
									destination++;
									source++;
								}
								signal_buffer->end=(int)number_of_samples-1;
							}
							else
							{
								signal_buffer->end=(int)mirada_start-1;
							}
							source=mirada_buffer;
							for (index=number_of_channels*(int)mirada_start;index>0;
								index--)
							{
								*destination= *source;
								destination++;
								source++;
							}
							/* write out the signal file */
							open_file_and_write(
#if defined (MOTIF)
								(Widget)NULL,(XtPointer)(page_window->acquire_file_open_data),
								(XtPointer)NULL
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
								page_window->acquire_file_open_data
#endif /* defined (WINDOWS) */
							);
						}
					}
					InvalidateRect(page_window->drawing_area,(CONST RECT *)NULL,FALSE);
					start_interrupting(page_window->device_driver,
						page_window->drawing_area,WM_USER,(LPARAM)page_window);
				}
#endif /* defined (MIRADA) */
#if defined (NI_DAQ)
				if (ni_cards)
				{
					/* stop continuous sampling */
					status=DAQ_Clear((ni_cards[0]).device_number);
/*???debug */
for (i=0;i<number_of_ni_cards;i++)
{
	{
		FILE *debug_nidaq;
		i16 stopped;
		u32 retrieved;

		if (debug_nidaq=fopen("nidaq.deb","a"))
		{
			status=DAQ_Check((ni_cards[i]).device_number,&stopped,&retrieved);
			fprintf(debug_nidaq,"ACQUIRE %d %d %d %d\n",i,status,stopped,
				retrieved);
			fclose(debug_nidaq);
		}
	}
}
#endif /* defined (NI_DAQ) */
				}
			}
		} break;
		case CHANNEL_EDIT:
		{
			switch (notify_code)
			{
				case EN_KILLFOCUS:
					/*???DB.  Pick up enter key press ? */
				{
					update_display_channel((struct Page_window *)GetWindowLong(window,
						DLGWINDOWEXTRA));
				} break;
			}
		} break;
		case EXIT_BUTTON:
		{
			DestroyWindow(window);
		} break;
		case MAXIMUM_EDIT:
		{
			switch (notify_code)
			{
				case EN_KILLFOCUS:
					/*???DB.  Pick up enter key press ? */
				{
					update_display_maximum((struct Page_window *)GetWindowLong(window,
						DLGWINDOWEXTRA));
				} break;
			}
		} break;
		case MINIMUM_EDIT:
		{
			switch (notify_code)
			{
				case EN_KILLFOCUS:
					/*???DB.  Pick up enter key press ? */
				{
					update_display_minimum((struct Page_window *)GetWindowLong(window,
						DLGWINDOWEXTRA));
				} break;
			}
		} break;
		case RESET_SCALE_BUTTON:
		{
			struct Page_window *page_window;

			if (page_window=(struct Page_window *)GetWindowLong(window,
				DLGWINDOWEXTRA))
			{
				page_window->signal_maximum=0;
				page_window->signal_minimum=1;
			}
		} break;
		case SCALE_BUTTON:
		{
			char number_string[21];
			float channel_gain,channel_offset;
			struct Page_window *page_window;

			if ((page_window=(struct Page_window *)GetWindowLong(window,
				DLGWINDOWEXTRA))&&(page_window->display_device))
			{
				if (page_window->signal_minimum<page_window->signal_maximum)
				{
					page_window->display_maximum=page_window->signal_maximum;
					page_window->display_minimum=page_window->signal_minimum;
					channel_gain=page_window->display_device->channel->gain;
					channel_offset=page_window->display_device->channel->offset;
					sprintf(number_string,"%7.2f",channel_gain*
						((float)(page_window->display_minimum)-channel_offset));
					Edit_SetText((page_window->minimum).edit,number_string);
					sprintf(number_string,"%7.2f",channel_gain*
						((float)(page_window->display_maximum)-channel_offset));
					Edit_SetText((page_window->maximum).edit,number_string);
					InvalidateRect(page_window->drawing_area,(CONST RECT *)NULL,FALSE);
				}
			}
		} break;
	}
	LEAVE;
} /* Page_window_WM_COMMAND_handler */
#endif /* defined (WINDOWS) */

#if defined (WINDOWS)
WNDPROC old_channel_edit_wndproc;

static LRESULT CALLBACK channel_edit_pick_up_enter(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 30 May 1997

DESCRIPTION :
???DB.  Should return 0 if it processes the message.
==============================================================================*/
{
	LRESULT return_code;

	ENTER(channel_edit_pick_up_enter);
	switch (message_identifier)
	{
		case WM_KEYDOWN:
		{
			switch ((UINT)first_message)
			{
				case VK_RETURN:
				{
					update_display_channel((struct Page_window *)GetWindowLong(window,
						GWL_USERDATA));
				} break;
			}
		} break;
	}
	return_code=CallWindowProc(old_channel_edit_wndproc,window,message_identifier,
		first_message,second_message);
	LEAVE;

	return (return_code);
} /* channel_edit_pick_up_enter */
#endif /* defined (WINDOWS) */

#if defined (WINDOWS)
WNDPROC old_maximum_edit_wndproc;

static LRESULT CALLBACK maximum_edit_pick_up_enter(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 30 May 1997

DESCRIPTION :
???DB.  Should return 0 if it processes the message.
==============================================================================*/
{
	LRESULT return_code;

	ENTER(maximum_edit_pick_up_enter);
	switch (message_identifier)
	{
		case WM_KEYDOWN:
		{
			switch ((UINT)first_message)
			{
				case VK_RETURN:
				{
					update_display_maximum((struct Page_window *)GetWindowLong(window,
						GWL_USERDATA));
				} break;
			}
		} break;
	}
	return_code=CallWindowProc(old_maximum_edit_wndproc,window,message_identifier,
		first_message,second_message);
	LEAVE;

	return (return_code);
} /* maximum_edit_pick_up_enter */
#endif /* defined (WINDOWS) */

#if defined (WINDOWS)
WNDPROC old_minimum_edit_wndproc;

static LRESULT CALLBACK minimum_edit_pick_up_enter(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 30 May 1997

DESCRIPTION :
???DB.  Should return 0 if it processes the message.
==============================================================================*/
{
	LRESULT return_code;

	ENTER(minimum_edit_pick_up_enter);
	switch (message_identifier)
	{
		case WM_KEYDOWN:
		{
			switch ((UINT)first_message)
			{
				case VK_RETURN:
				{
					update_display_minimum((struct Page_window *)GetWindowLong(window,
						GWL_USERDATA));
				} break;
			}
		} break;
	}
	return_code=CallWindowProc(old_minimum_edit_wndproc,window,message_identifier,
		first_message,second_message);
	LEAVE;

	return (return_code);
} /* minimum_edit_pick_up_enter */
#endif /* defined (WINDOWS) */

#if defined (WINDOWS)
static LRESULT CALLBACK Page_window_class_proc(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 8 September 1997

DESCRIPTION :
???DB.  Should return 0 if it processes the message.
==============================================================================*/
{
	LRESULT return_code;

	ENTER(Page_window_class_proc);
	switch (message_identifier)
	{
		case WM_CLOSE:
		{
			DestroyWindow(window);
			return_code=0;
		} break;
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return_code=0;
		} break;
		case WM_COMMAND:
		{
			return_code=(BOOL)HANDLE_WM_COMMAND(window,first_message,second_message,
				Page_window_WM_COMMAND_handler);
		} break;
		case WM_INITDIALOG:
		{
			BOOL win32_return_code;
			char working_string[21];
			float channel_gain,channel_offset;
			int widget_spacing;
			RECT rectangle;
			static char *class_name="Page_window_drawing_area";
			struct Page_window *page_window;
			WNDCLASSEX class_information;

			/*???DB.  Need to create drawing area */
			if (page_window=(struct Page_window *)second_message)
			{
				SetWindowLong(window,DLGWINDOWEXTRA,(LONG)page_window);
				/* retrieve the control windows and do any setup required */
				widget_spacing=page_window->user_interface->widget_spacing;
				page_window->acquire_button=GetDlgItem(window,ACQUIRE_BUTTON);
				GetWindowRect(page_window->acquire_button,&rectangle);
				page_window->acquire_button_width=rectangle.right-rectangle.left;
				page_window->acquire_button_height=rectangle.bottom-rectangle.top;
				(page_window->channel).edit=GetDlgItem(window,CHANNEL_EDIT);
				GetWindowRect((page_window->channel).edit,&rectangle);
				(page_window->channel).edit_width=rectangle.right-rectangle.left;
				(page_window->channel).edit_height=rectangle.bottom-rectangle.top;
#if defined (OLD_CODE)
				sprintf(working_string,"%d",page_window->channel_number);
				Edit_SetText((page_window->channel).edit,working_string);
#endif /* defined (OLD_CODE) */
				if (page_window->display_device)
				{
					Edit_SetText((page_window->channel).edit,
						page_window->display_device->description->name);
				}
				/* replace the window procedure so that enters can be picked up */
				old_channel_edit_wndproc=(WNDPROC)SetWindowLong(
					(page_window->channel).edit,GWL_WNDPROC,
					(DWORD)channel_edit_pick_up_enter);
				SetWindowLong((page_window->channel).edit,GWL_USERDATA,
					(LONG)page_window);
				/* add up and down arrows for the channel number */
				(page_window->channel).arrows_width=
					((page_window->channel).edit_width)/3;
				(page_window->channel).arrows_height=(page_window->channel).edit_height;
				(page_window->channel).arrows=CreateUpDownControl(
					WS_CHILD|WS_BORDER|WS_VISIBLE|UDS_ALIGNLEFT,0,0,
					(page_window->channel).arrows_width,
					(page_window->channel).arrows_height,window,CHANNEL_ARROWS,
					page_window->instance,(HWND)NULL,page_window->number_of_channels,1,1);
				(page_window->channel).text=GetDlgItem(window,CHANNEL_TEXT);
				GetWindowRect((page_window->channel).text,&rectangle);
				(page_window->channel).text_width=rectangle.right-rectangle.left;
				(page_window->channel).text_height=rectangle.bottom-rectangle.top;
				page_window->exit_button=GetDlgItem(window,EXIT_BUTTON);
				GetWindowRect(page_window->exit_button,&rectangle);
				page_window->exit_button_width=rectangle.right-rectangle.left;
				page_window->exit_button_height=rectangle.bottom-rectangle.top;
				(page_window->maximum).edit=GetDlgItem(window,MAXIMUM_EDIT);
				GetWindowRect((page_window->maximum).edit,&rectangle);
				(page_window->maximum).edit_width=rectangle.right-rectangle.left;
				(page_window->maximum).edit_height=rectangle.bottom-rectangle.top;
				if (page_window->display_device)
				{
					channel_gain=page_window->display_device->channel->gain;
					channel_offset=page_window->display_device->channel->offset;
					sprintf(working_string,"%7.2f",channel_gain*
						((float)(page_window->display_maximum)-channel_offset));
					Edit_SetText((page_window->maximum).edit,working_string);
				}
				/* replace the window procedure so that enters can be picked up */
				old_maximum_edit_wndproc=(WNDPROC)SetWindowLong(
					(page_window->maximum).edit,GWL_WNDPROC,
					(DWORD)maximum_edit_pick_up_enter);
				SetWindowLong((page_window->maximum).edit,GWL_USERDATA,
					(LONG)page_window);
				(page_window->maximum).text=GetDlgItem(window,MAXIMUM_TEXT);
				GetWindowRect((page_window->maximum).text,&rectangle);
				(page_window->maximum).text_width=rectangle.right-rectangle.left;
				(page_window->maximum).text_height=rectangle.bottom-rectangle.top;
				(page_window->minimum).edit=GetDlgItem(window,MINIMUM_EDIT);
				GetWindowRect((page_window->minimum).edit,&rectangle);
				(page_window->minimum).edit_width=rectangle.right-rectangle.left;
				(page_window->minimum).edit_height=rectangle.bottom-rectangle.top;
				if (page_window->display_device)
				{
					sprintf(working_string,"%7.2f",channel_gain*
						((float)(page_window->display_minimum)-channel_offset));
					Edit_SetText((page_window->minimum).edit,working_string);
				}
				/* replace the window procedure so that enters can be picked up */
				old_minimum_edit_wndproc=(WNDPROC)SetWindowLong(
					(page_window->minimum).edit,GWL_WNDPROC,
					(DWORD)minimum_edit_pick_up_enter);
				SetWindowLong((page_window->minimum).edit,GWL_USERDATA,
					(LONG)page_window);
				(page_window->minimum).text=GetDlgItem(window,MINIMUM_TEXT);
				GetWindowRect((page_window->minimum).text,&rectangle);
				(page_window->minimum).text_width=rectangle.right-rectangle.left;
				(page_window->minimum).text_height=rectangle.bottom-rectangle.top;
				page_window->reset_scale_button=GetDlgItem(window,RESET_SCALE_BUTTON);
				GetWindowRect(page_window->reset_scale_button,&rectangle);
				page_window->reset_scale_button_width=rectangle.right-rectangle.left;
				page_window->reset_scale_button_height=rectangle.bottom-rectangle.top;
				page_window->scale_button=GetDlgItem(window,SCALE_BUTTON);
				GetWindowRect(page_window->scale_button,&rectangle);
				page_window->scale_button_width=rectangle.right-rectangle.left;
				page_window->scale_button_height=rectangle.bottom-rectangle.top;
				page_window->menu_bar_width=
					page_window->acquire_button_width+widget_spacing+
					(page_window->channel).text_width+
					(page_window->channel).arrows_width+
					(page_window->channel).edit_width+widget_spacing+
					page_window->scale_button_width+widget_spacing+
					page_window->reset_scale_button_width+widget_spacing+
					(page_window->minimum).text_width+
					(page_window->minimum).edit_width+widget_spacing+
					(page_window->maximum).text_width+
					(page_window->maximum).edit_width+widget_spacing+
					page_window->exit_button_width;
				/* check if the drawing area class is registered */
				if (TRUE!=(win32_return_code=GetClassInfoEx(page_window->instance,
					class_name,&class_information)))
				{
					class_information.cbClsExtra=0;
					class_information.cbWndExtra=sizeof(struct Page_window *);
					class_information.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
					class_information.hCursor=LoadCursor(NULL,IDC_ARROW);
					class_information.hIcon=(HICON)NULL;
					class_information.hInstance=page_window->instance;
					class_information.lpfnWndProc=Page_window_drawing_area_class_proc;
					class_information.lpszClassName=class_name;
					class_information.style=CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
					/* allow resource to specify the menu */
					class_information.lpszMenuName=NULL;
					/*???DB.  Extra in WNDCLASSEX over WNDCLASS */
					class_information.cbSize=sizeof(WNDCLASSEX);
					class_information.hIconSm=(HICON)NULL;
					if (RegisterClassEx(&class_information))
					{
						win32_return_code=TRUE;
					}
				}
				/* create the window */
				if (TRUE==win32_return_code)
				{
					GetClientRect(window,&rectangle);
					if (page_window->drawing_area=CreateWindowEx(
						WS_EX_OVERLAPPEDWINDOW,
							/* extended window styles */
						class_name,
							/* class name */
						"",
							/* window name */
							/*???DB.  Is this what goes in the title bar ? */
						WS_CHILD|WS_VISIBLE,
							/* window styles */
						0,
							/* horizontal position */
						25,
							/* vertical position */
						rectangle.right-rectangle.left,
							/* width */
						rectangle.bottom-rectangle.top-25,
							/* height */
						window,
							/* parent or owner window */
						(HMENU)NULL,
							/* menu to be used - use class menu */
						page_window->instance,
							/* instance handle */
						page_window
							/* window creation data */
							/*???DB.  Like to have page_window */
						))
					{
						/* make the page window fill the whole screen */
						RECT screen_rectangle;

						if (TRUE==SystemParametersInfo(SPI_GETWORKAREA,0,&screen_rectangle,
							0))
						{
							MoveWindow(window,0,0,
								screen_rectangle.right-screen_rectangle.left,
								screen_rectangle.bottom-screen_rectangle.top,
								TRUE);
#if defined (OLD_CODE)
/*???debug */
{
	FILE *debug;
	LONG base_units,base_unit_x,base_unit_y;

	if (debug=fopen("debug.out","a"))
	{
		base_units=GetDialogBaseUnits();
		base_unit_x=base_units&0x0000ffff;
		base_unit_y=(base_units>>16)&0x0000ffff;
		fprintf(debug,"base_unit_x=%d, base_unit_y=%d\n",base_unit_x,base_unit_y);
		fprintf(debug,"Screen size %d %d %d %d\n",screen_rectangle.left,
			screen_rectangle.right,screen_rectangle.top,screen_rectangle.bottom);
		fprintf(debug,"dialog width=%d, dialog height=%d\n",
			((screen_rectangle.right-screen_rectangle.left)*4)/base_unit_x,
			((screen_rectangle.bottom-screen_rectangle.top)*8)/base_unit_y);
		fclose(debug);
	}
}
#endif /* defined (OLD_CODE) */
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Page_window_class_proc.  Could not get system info");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Page_window_class_proc.  Could not create drawing area");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Page_window_class_proc.  Unable to register class information");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Page_window_class_proc.  Missing page_window");
			}
			return_code=DefDlgProc(window,message_identifier,first_message,
				second_message);
		} break;
#if defined (OLD_CODE)
/*???DB.  Doesn't work.  Try if decide to create controls individually */
		case WM_KEYDOWN:
			/*???DB.  Don't know which control it is for.  May not even come here */
		{
			switch ((UINT)first_message)
			{
				case VK_RETURN:
				{
					update_display_maximum((struct Page_window *)GetWindowLong(window,
						DLGWINDOWEXTRA));
					return_code=0;
				} break;
				default:
				{
					return_code=DefDlgProc(window,message_identifier,first_message,
						second_message);
				} break;
			}
		} break;
#endif /* defined (OLD_CODE) */
		case WM_NOTIFY:
		{
			int device_number;
			struct Device *display_device;
			struct Page_window *page_window;

			if ((CHANNEL_ARROWS==(int)first_message)&&
				(UDN_DELTAPOS==((LPNMHDR)second_message)->code))
			{
				if ((page_window=(struct Page_window *)GetWindowLong(window,
					DLGWINDOWEXTRA))&&(page_window->display_device))
				{
					device_number=page_window->device_number;
					if (0<((NM_UPDOWN FAR *)second_message)->iDelta)
					{
						device_number++;
					}
					else
					{
						device_number--;
					}
					if ((0<=device_number)&&(device_number<
						((*(page_window->rig_address))->number_of_devices))&&
						(display_device=((*(page_window->rig_address))->devices)[
						device_number])&&(display_device->signal)&&
						(display_device->channel)&&(0<display_device->channel->number)&&
						(display_device->channel->number<=
						(int)(page_window->number_of_channels))&&
						(display_device->description)&&(display_device->description->name))
					{
						page_window->display_device=display_device;
						page_window->device_number=device_number;
#if defined (OLD_CODE)
/*???DB.  Don't want range reset when change channel */
						page_window->signal_maximum=0;
						page_window->signal_minimum=1;
#endif /* defined (OLD_CODE) */
						Edit_SetText((page_window->channel).edit,
							display_device->description->name);
						InvalidateRect(page_window->drawing_area,(CONST RECT *)NULL,
							FALSE);
					}
				}
			}
		} break;
		case WM_PAINT:
		{
			int acquire_button_width,channel_arrows_width,channel_edit_width,
				channel_text_width,exit_button_width,maximum_edit_width,
				maximum_text_width,menu_bar_width,minimum_edit_width,minimum_text_width,
				reset_scale_button_width,scale_button_width,widget_spacing;
			RECT rectangle;
			struct Page_window *page_window;

			if (page_window=(struct Page_window *)GetWindowLong(window,
				DLGWINDOWEXTRA))
			{
				GetClientRect(window,&rectangle);
				/* resize menu bar */
				widget_spacing=page_window->user_interface->widget_spacing;
				acquire_button_width=page_window->acquire_button_width;
				channel_arrows_width=(page_window->channel).arrows_width;
				channel_edit_width=(page_window->channel).edit_width;
				channel_text_width=(page_window->channel).text_width;
				exit_button_width=page_window->exit_button_width;
				maximum_edit_width=(page_window->maximum).edit_width;
				maximum_text_width=(page_window->maximum).text_width;
				minimum_edit_width=(page_window->minimum).edit_width;
				minimum_text_width=(page_window->minimum).text_width;
				reset_scale_button_width=page_window->reset_scale_button_width;
				scale_button_width=page_window->scale_button_width;
				menu_bar_width=page_window->menu_bar_width;
				if (menu_bar_width>rectangle.right-rectangle.left)
				{
					if (menu_bar_width-6*widget_spacing>rectangle.right-rectangle.left)
					{
						menu_bar_width -= 6*widget_spacing;
						widget_spacing=0;
						acquire_button_width=((rectangle.right-rectangle.left)*
							acquire_button_width)/menu_bar_width;
						channel_text_width=((rectangle.right-rectangle.left)*
							(channel_arrows_width+channel_edit_width+channel_text_width))/
							menu_bar_width-(channel_arrows_width+channel_edit_width);
						if (channel_text_width<0)
						{
							channel_arrows_width += channel_text_width;
							channel_text_width=0;
							if (channel_arrows_width<0)
							{
								channel_edit_width += channel_arrows_width;
								channel_arrows_width=0;
							}
						}
						scale_button_width=((rectangle.right-rectangle.left)*
							scale_button_width)/menu_bar_width;
						reset_scale_button_width=((rectangle.right-rectangle.left)*
							reset_scale_button_width)/menu_bar_width;
						maximum_text_width=((rectangle.right-rectangle.left)*
							(maximum_edit_width+maximum_text_width))/menu_bar_width-
							maximum_edit_width;
						if (maximum_text_width<0)
						{
							maximum_edit_width += maximum_text_width;
							maximum_text_width=0;
						}
						minimum_text_width=((rectangle.right-rectangle.left)*
							(minimum_edit_width+minimum_text_width))/menu_bar_width-
							minimum_edit_width;
						if (minimum_text_width<0)
						{
							minimum_edit_width += minimum_text_width;
							minimum_text_width=0;
						}
						exit_button_width=((rectangle.right-rectangle.left)*
							exit_button_width)/menu_bar_width;
					}
					else
					{
						widget_spacing=(rectangle.right-rectangle.left+
							5*widget_spacing-menu_bar_width)/5;
					}
				}
				menu_bar_width=0;
				MoveWindow(page_window->acquire_button,menu_bar_width,2,
					acquire_button_width,page_window->acquire_button_height,TRUE);
				menu_bar_width += widget_spacing+acquire_button_width;
				MoveWindow((page_window->channel).text,menu_bar_width,3,
					channel_text_width,(page_window->channel).text_height,TRUE);
				menu_bar_width += channel_text_width;
				MoveWindow((page_window->channel).edit,menu_bar_width,2,
					channel_edit_width,(page_window->channel).edit_height,TRUE);
				menu_bar_width += channel_edit_width;
				MoveWindow((page_window->channel).arrows,menu_bar_width,2,
					channel_arrows_width,(page_window->channel).arrows_height,TRUE);
				menu_bar_width += widget_spacing+channel_arrows_width;
				MoveWindow(page_window->scale_button,menu_bar_width,2,
					scale_button_width,page_window->scale_button_height,TRUE);
				menu_bar_width += widget_spacing+scale_button_width;
				MoveWindow(page_window->reset_scale_button,menu_bar_width,2,
					reset_scale_button_width,page_window->reset_scale_button_height,TRUE);
				menu_bar_width += widget_spacing+reset_scale_button_width;
				MoveWindow((page_window->minimum).text,menu_bar_width,3,
					minimum_text_width,(page_window->minimum).text_height,TRUE);
				menu_bar_width += minimum_text_width;
				MoveWindow((page_window->minimum).edit,menu_bar_width,2,
					minimum_edit_width,(page_window->minimum).edit_height,TRUE);
				menu_bar_width += widget_spacing+minimum_edit_width;
				MoveWindow((page_window->maximum).text,menu_bar_width,3,
					maximum_text_width,(page_window->maximum).text_height,TRUE);
				menu_bar_width += maximum_text_width;
				MoveWindow((page_window->maximum).edit,menu_bar_width,2,
					maximum_edit_width,(page_window->maximum).edit_height,TRUE);
				MoveWindow(page_window->exit_button,
					rectangle.right-rectangle.left-exit_button_width,2,
					exit_button_width,page_window->exit_button_height,TRUE);
				/* resize drawing area */
				MoveWindow(page_window->drawing_area,0,25,
					rectangle.right-rectangle.left,
					rectangle.bottom-rectangle.top-25,TRUE);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Page_window_class_proc.  Missing page_window");
			}
			return_code=DefDlgProc(window,message_identifier,first_message,
				second_message);
		} break;
		default:
		{
			return_code=DefDlgProc(window,message_identifier,first_message,
				second_message);
		} break;
	}
	LEAVE;

	return (return_code);
} /* Page_window_class_proc */
#endif /* defined (WINDOWS) */

#if defined (WINDOWS)
static BOOL CALLBACK Page_window_dialog_proc(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 19 April 1997

DESCRIPTION :
???DB.  All in the class proc
==============================================================================*/
{
	BOOL return_code;

	ENTER(Page_window_dialog_proc);
	return_code=FALSE;
	LEAVE;

	return (return_code);
} /* Page_window_dialog_proc */
#endif /* defined (WINDOWS) */

/*
Global functions
----------------
*/
struct Page_window *create_Page_window(struct Page_window **address,
#if defined (MOTIF)
	Widget activation,Widget parent,
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	HWND parent,
#if defined (MIRADA)
	HANDLE device_driver,
#endif /* defined (MIRADA) */
#endif /* defined (WINDOWS) */
	struct Page **page_address,struct Rig **rig_address,
#if defined (MOTIF)
	Pixel identifying_colour,Pixel background_colour,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 6 February 1998

DESCRIPTION :
This function allocates the memory for a page window and sets the fields to the
specified values (<address>, <activation>, <page_address>).  It then retrieves
a page window widget with the specified <parent> and assigns the widget ids to
the appropriate fields of the structure.  If successful it returns a pointer to
the created page window and, if <address> is not NULL, makes <*address> point to
the created page window.  If unsuccessful, NULL is returned.
==============================================================================*/
{
	int channel_number,device_number,i,number_of_devices;
	struct Device **device_address,*display_device;
	struct Page_window *page_window;
#if defined (MOTIF)
	static MrmRegisterArg callback_list[]={
		{"identify_page_calibrate_button",
			(XtPointer)identify_page_calibrate_button},
		{"identify_page_reset_button",(XtPointer)identify_page_reset_button},
		{"identify_page_close_button",(XtPointer)identify_page_close_button},
		{"identify_page_drawing_area",(XtPointer)identify_page_drawing_area},
		{"destroy_Page_window",(XtPointer)destroy_Page_window}};
	static MrmRegisterArg identifier_list[]=
	{
		{"page_window_structure",(XtPointer)NULL},
		{"identifying_colour",(XtPointer)NULL}
	};
	MrmType page_window_class;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	BOOL win32_return_code;
	static char *class_name="Page_window";
#if defined (MIRADA)
	unsigned char *bus,*device_function;
	unsigned long number_of_cards;
#endif /* defined (MIRADA) */
	WNDCLASSEX class_information;
#endif /* defined (WINDOWS) */

	ENTER(create_Page_window);
	/* check arguments */
	if (rig_address&&user_interface)
	{
#if defined (MOTIF)
		if (MrmOpenHierarchy_base64_string(page_window_uid64,
			&page_window_hierarchy,&page_window_hierarchy_open))
		{
#endif /* defined (MOTIF) */
			/* allocate memory */
			if (ALLOCATE(page_window,struct Page_window,1))
			{
				/* assign fields */
				page_window->address=address;
				page_window->page_address=page_address;
				page_window->user_interface=user_interface;
				page_window->display_maximum=SHRT_MAX;
				page_window->display_minimum=SHRT_MIN+1;
				page_window->signal_maximum=0;
				page_window->signal_minimum=1;
				page_window->rig_address=rig_address;
				page_window->acquire_file_open_data=create_File_open_data(".sig",
					REGULAR,acquire_write_signal_file,rig_address,0,user_interface);
#if defined (MOTIF)
				page_window->activation=activation;
				page_window->shell=parent;
				page_window->window=(Widget)NULL;
				page_window->calibrate_button=(Widget)NULL;
				page_window->reset_button=(Widget)NULL;
				page_window->close_button=(Widget)NULL;
				page_window->drawing_area=(Widget)NULL;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				page_window->window=(HWND)NULL;
				page_window->acquire_button=(HWND)NULL;
				(page_window->channel).edit=(HWND)NULL;
				(page_window->channel).text=(HWND)NULL;
				page_window->exit_button=(HWND)NULL;
				(page_window->maximum).edit=(HWND)NULL;
				(page_window->maximum).text=(HWND)NULL;
				(page_window->minimum).edit=(HWND)NULL;
				(page_window->minimum).text=(HWND)NULL;
				page_window->reset_scale_button=(HWND)NULL;
				page_window->scale_button=(HWND)NULL;
				page_window->drawing_area=(HWND)NULL;
				page_window->instance=user_interface->instance;
#if defined (MIRADA)
				page_window->device_driver=device_driver;
				if (INVALID_HANDLE_VALUE!=device_driver)
				{
					if (PCI_SUCCESSFUL!=get_mirada_information(device_driver,
						&number_of_cards,&bus,&device_function,
						&(page_window->number_of_channels),
						&(page_window->number_of_samples),&(page_window->mirada_buffer)))
					{
						page_window->mirada_buffer=(short int *)NULL;
						page_window->number_of_channels=0;
						page_window->number_of_samples=0;
						display_message(ERROR_MESSAGE,
							"create_Page_window.  Could not get Mirada information");
					}
				}
				else
				{
					page_window->mirada_buffer=(short int *)NULL;
					page_window->number_of_channels=0;
					page_window->number_of_samples=0;
				}
#endif /* defined (MIRADA) */
#if defined (NI_DAQ)
				if (*rig_address)
				{
					page_window->number_of_channels=(*((*rig_address)->devices))->signal->
						buffer->number_of_signals;
					page_window->number_of_samples=(*((*rig_address)->devices))->signal->
						buffer->number_of_samples;
				}
				else
				{
					page_window->number_of_channels=0;
					page_window->number_of_samples=0;
				}
#endif /* defined (NI_DAQ) */
#endif /* defined (WINDOWS) */
				/* set the display device */
				display_device=(struct Device *)NULL;
				if ((*rig_address)&&
					(0<(number_of_devices=(*rig_address)->number_of_devices))&&
					(device_address=(*rig_address)->devices))
				{
#if defined (WINDOWS)
					channel_number=(page_window->number_of_channels)+1;
#else
					channel_number=0;
#endif /* defined (WINDOWS) */
					for (i=0;i<number_of_devices;i++)
					{
						if ((*device_address)&&((*device_address)->signal)&&
							((*device_address)->channel)&&
							(0<(*device_address)->channel->number)&&
							((*device_address)->channel->number<channel_number)&&
							((*device_address)->description)&&
							((*device_address)->description->name))
						{
							display_device= *device_address;
							device_number=i;
							channel_number=display_device->channel->number;
						}
						device_address++;
					}
				}
				if (page_window->display_device=display_device)
				{
					page_window->device_number=device_number;
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"No device with a valid channel number in the rig");
					page_window->device_number= -1;
				}
#if defined (MOTIF)
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(page_window_hierarchy,
					callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)page_window;
					identifier_list[1].value=(XtPointer)identifying_colour;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(page_window_hierarchy,
						identifier_list,XtNumber(identifier_list)))
					{
						/* fetch the page window widget */
						if (MrmSUCCESS==MrmFetchWidget(page_window_hierarchy,"page_window",
							parent,&(page_window->window),&page_window_class))
						{
							/* set the background colour for the page drawing area */
							XtVaSetValues(page_window->drawing_area,XmNbackground,
								background_colour,NULL);
							/*??? set ghosting for the buttons */
							if (address)
							{
								*address=page_window;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_Page_window.  Could not fetch page window widget");
							DEALLOCATE(page_window);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Page_window.  Could not register the identifiers");
						DEALLOCATE(page_window);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Page_window.  Could not register the callbacks");
					DEALLOCATE(page_window);
				}
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				/* check if the class is registered */
				if (TRUE!=(win32_return_code=GetClassInfoEx(user_interface->instance,
					class_name,&class_information)))
				{
					class_information.cbClsExtra=0;
					class_information.cbWndExtra=
						DLGWINDOWEXTRA+sizeof(struct Page_window *);
					class_information.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
					class_information.hCursor=LoadCursor(NULL,IDC_ARROW);
					class_information.hIcon=(HICON)NULL;
/*					class_information.hIcon=LoadIcon(user_interface->instance,class_name);*/
						/*???DB.  Do I need an icon ? */
					class_information.hInstance=user_interface->instance;
					class_information.lpfnWndProc=Page_window_class_proc;
					class_information.lpszClassName=class_name;
					class_information.style=CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
					/* allow resource to specify the menu */
					class_information.lpszMenuName=NULL;
					/*???DB.  Extra in WNDCLASSEX over WNDCLASS */
					class_information.cbSize=sizeof(WNDCLASSEX);
					class_information.hIconSm=(HICON)NULL;
/*					class_information.hIconSm=LoadIcon(user_interface->instance,
						"Page_window" "_small");*/
						/*???DB.  Do I need an icon ? */
					if (RegisterClassEx(&class_information))
					{
						win32_return_code=TRUE;
					}
				}
				/* create the window */
				if (TRUE==win32_return_code)
				{
					if (page_window->window=CreateDialogParam(
						user_interface->instance,"Page_window",parent,
						Page_window_dialog_proc,(LPARAM)page_window))
					{
						if (address)
						{
							*address=page_window;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Page_window.  Could not create dialog");
						DEALLOCATE(page_window);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Page_window.  Unable to register class information");
					DEALLOCATE(page_window);
				}
#endif /* defined (WINDOWS) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Page_window.  Could not allocate page window structure");
			}
#if defined (MOTIF)
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Page_window.  Could not open hierarchy");
			page_window=(struct Page_window *)NULL;
		}
#endif /* defined (MOTIF) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Page_window.  Missing user_interface");
		page_window=(struct Page_window *)NULL;
	}
	LEAVE;

	return (page_window);
} /* create_Page_window */

int open_Page_window(struct Page_window **address,struct Page **page_address,
	struct Rig **rig_address,
#if defined (MOTIF)
	Pixel identifying_colour,Pixel background_colour,int screen_width,
	int screen_height,
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
#if defined (MIRADA)
	HANDLE device_driver,
#endif /* defined (MIRADA) */
#endif /* defined (WINDOWS) */
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 31 January 1998

DESCRIPTION :
If <*address> is NULL, a page window with the specified <parent> and 
<identifying_colour> is created.  The page window is opened.
???DB.  Change rig_address on the fly ?
==============================================================================*/
{
	int return_code;
	struct Page_window *page_window;
#if defined (MOTIF)
	Widget page_window_shell;
#endif /* defined (MOTIF) */

	ENTER(open_Page_window);
	if (address&&user_interface)
	{
		if (!(page_window= *address))
		{
#if defined (MOTIF)
			if (page_window_shell=create_page_window_shell((Widget *)NULL,
				user_interface->application_shell,screen_width,screen_height))
			{
#endif /* defined (MOTIF) */
				if (page_window=create_Page_window(address,
#if defined (MOTIF)
					(Widget)NULL,page_window_shell,
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
					(HWND)NULL,
#if defined (MIRADA)
					device_driver,
#endif /* defined (MIRADA) */
#endif /* defined (WINDOWS) */
					page_address,rig_address,
#if defined (MOTIF)
					identifying_colour,background_colour,
#endif /* defined (MOTIF) */
					user_interface))
				{
#if defined (MOTIF)
					/* manage the page window */
					XtManageChild(page_window->window);
					/* realize the page shell */
					XtRealizeWidget(page_window->shell);
#endif /* defined (MOTIF) */
					*address=page_window;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"open_Page_window.  Could not create window");
#if defined (MOTIF)
					XtDestroyWidget(page_window_shell);
#endif /* defined (MOTIF) */
					return_code=0;
				}
#if defined (MOTIF)
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"open_Page_window.  Could not create shell");
				return_code=0;
			}
#endif /* defined (MOTIF) */
		}
		if (page_window
#if defined (MOTIF)
			&&(page_window->shell)
#endif /* defined (MOTIF) */
			)
		{
#if defined (MOTIF)
			/* pop up the page window shell */
			XtPopup(page_window->shell,XtGrabNone);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
			ShowWindow(page_window->window,SW_SHOWDEFAULT);
				/*???DB.  SW_SHOWDEFAULT needs thinking about */
#endif /* defined (WINDOWS) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"open_Page_window.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* open_Page_window */
