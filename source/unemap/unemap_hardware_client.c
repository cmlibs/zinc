/*******************************************************************************
FILE : unemap_hardware_client.c

LAST MODIFIED : 10 July 2000

DESCRIPTION :
Code for talking to the unemap hardware service (running under NT).  This is an
alternative to unemap_hardware.c that talks directly to the hardware.

QUESTIONS :
1 How much information should the client store ?  Start with none.
2 Can wormholes be used ?

???DB.  Sort out __BYTE_ORDER and BIG_ENDIAN.  See /usr/include/sys/endian.h
???DB.  Sort out WSACleanup/WSAStartup
	???DB.  Where I'm up to.  Will have to clean up properly for client ?
				WaitForMultipleObjects for WINDOWS

TO DO :
1 Extend to multiple crates
???DB.  Need master slave flag passed to configure?
2 Speed up save/transfer of data
3 Synchronize stimulating between cards and then crates
==============================================================================*/

#define USE_SOCKETS
/*#define USE_WORMHOLES*/

#define CACHE_CLIENT_INFORMATION

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if defined (WIN32)
/*???DB.  Assume that running on Intel machine */
#define __BYTE_ORDER 1234
#if defined (USE_SOCKETS)
#include <winsock2.h>
#include <process.h>
#include <tchar.h>
#endif /* defined (USE_SOCKETS) */
#if defined (USE_WORMHOLES)
#include <windows.h>
#include "wormhole.h"
#endif /* defined (USE_WORMHOLES) */
#endif /* defined (WIN32) */
#if defined (UNIX)
#include <ctype.h>
	/*???DB.  Contains definition of __BYTE_ORDER for Linux */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
extern int errno;
#endif /* defined (UNIX) */
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "unemap/unemap_hardware.h"
#include "unemap_hardware_service/unemap_hardware_service.h"

/*
Module constants
----------------
*/
#if defined (CACHE_CLIENT_INFORMATION)
#define NUMBER_OF_CHANNELS_ON_NI_CARD 64
#endif /* defined (CACHE_CLIENT_INFORMATION) */

#if defined (WIN32)
/*???DB.  Assume that running on Intel machine */
#define __BYTE_ORDER 1234
#endif /* defined (WIN32) */
#if defined (UNIX)
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#endif /* defined (UNIX) */

#if defined (__BYTE_ORDER) && (1234==__BYTE_ORDER)
#define BIG_ENDIAN_CODE (unsigned char)0x01
#else /* defined (__BYTE_ORDER) && (1234==__BYTE_ORDER) */
#define BIG_ENDIAN_CODE (unsigned char)0x00
#endif /* defined (__BYTE_ORDER) && (1234==__BYTE_ORDER) */

/*???DB.  Temporary ? */
#define DEFAULT_PORT 5001
#define DEFAULT_SERVER_NAME "localhost"
#define DEFAULT_SOCKET_TYPE SOCK_STREAM
#define SERVER_FILE_NAME "unemap_server.txt"

/* for scrolling */
#define SCROLLING_NO_CHANNELS_FOR_CRATE (0x1)
#define SCROLLING_CHANNELS_FAILED_FOR_CRATE (0x2)
#define SCROLLING_CHANNELS_SUCCEEDED_FOR_CRATE (0x4)

/*
Module variables
----------------
*/
#if defined (CACHE_CLIENT_INFORMATION)
struct Unemap_card
/*******************************************************************************
LAST MODIFIED : 6 March 2000

DESCRIPTION :
Used for caching information on this side of the connection to speed up some
functions.
==============================================================================*/
{
	float post_filter_gain,pre_filter_gain;
	/* for a gain of 1 */
	float maximum_voltage,minimum_voltage;
}; /* struct Unemap_card */
#endif /* defined (CACHE_CLIENT_INFORMATION) */

struct Unemap_crate
/*******************************************************************************
LAST MODIFIED : 2 July 2000

DESCRIPTION :
Information needed for each SCU crate/NI computer pair.
==============================================================================*/
{
	char *server_name;
#if defined (USE_SOCKETS)
#if defined (WIN32)
	SOCKET acquired_socket;
	HANDLE acquired_socket_thread_stop_event;
	SOCKET calibration_socket;
	HANDLE calibration_socket_thread_stop_event;
	SOCKET command_socket;
	SOCKET scrolling_socket;
	HANDLE scrolling_socket_thread_stop_event;
#endif /* defined (WIN32) */
#if defined (UNIX)
	int acquired_socket;
	int calibration_socket;
	int command_socket;
	int scrolling_socket;
#endif /* defined (UNIX) */
#if defined (MOTIF)
	XtInputId acquired_socket_xid;
	XtInputId calibration_socket_xid;
	XtInputId scrolling_socket_xid;
#endif /* defined (MOTIF) */
#endif /* defined (USE_SOCKETS) */
	unsigned short acquired_port,calibration_port,command_port,scrolling_port;
	struct
	{
		float *channel_gains,*channel_offsets;
		int *channel_numbers,complete,number_of_channels;
	} calibration;
	struct
	{
		int *channel_numbers,complete,number_of_channels,
			number_of_values_per_channel;
		short *values;
	} scrolling;
	struct
	{
		int complete,number_of_channels;
		short *samples;
		unsigned long number_of_samples;
	} acquired;
	int number_of_channels;
#if defined (CACHE_CLIENT_INFORMATION)
	int number_of_stimulators,*stimulator_card_indices,number_of_unemap_cards;
	struct Unemap_card *unemap_cards;
#endif /* defined (CACHE_CLIENT_INFORMATION) */
}; /* struct Unemap_crate */

/*
Module variables
----------------
*/
/* scrolling information set by configure_NI_cards and
	set_NI_scrolling_channel */
Unemap_hardware_callback
	*module_scrolling_callback=(Unemap_hardware_callback *)NULL;
void *module_scrolling_callback_data=(void *)NULL;
#if defined (WINDOWS)
UINT module_scrolling_message=(UINT)0;
HWND module_scrolling_window=(HWND)NULL;
#endif /* defined (WINDOWS) */

/* calibration information set by unemap_calibrate */
Calibration_end_callback
	*module_calibration_end_callback=(Calibration_end_callback *)NULL;
void *module_calibration_end_callback_data=(void *)NULL;

int allow_open_connection=1;
int module_number_of_channels=0,module_number_of_unemap_crates=0;
struct Unemap_crate *module_unemap_crates=(struct Unemap_crate *)NULL;

#if defined (CACHE_CLIENT_INFORMATION)
int module_force_connection=0,module_get_cache_information_failed=0,
	module_number_of_stimulators=0;
#endif /* defined (CACHE_CLIENT_INFORMATION) */

/* information set by unemap_get_samples_acquired_background */
Acquired_data_callback *module_acquired_callback=(Acquired_data_callback *)NULL;
int module_acquired_channel_number= -1;
void *module_acquired_callback_data=(void *)NULL;

/*
Module functions
----------------
*/
#if defined (WINDOWS)
static void sleep(unsigned seconds)
{
	Sleep((DWORD)seconds*(DWORD)1000);
}
#endif /* defined (WINDOWS) */

static int close_crate_connection(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 2 July 2000

DESCRIPTION :
Closes the connection with the unemap hardware service for the <crate>.
==============================================================================*/
{
	int return_code;

	ENTER(close_crate_connection);
	return_code=0;
	if (crate)
	{
		return_code=1;
		if (INVALID_SOCKET!=crate->command_socket)
		{
#if defined (WIN32)
			closesocket(crate->command_socket);
#endif /* defined (WIN32) */
#if defined (UNIX)
			close(crate->command_socket);
#endif /* defined (UNIX) */
			crate->command_socket=INVALID_SOCKET;
		}
#if defined (WIN32)
		if (crate->scrolling_socket_thread_stop_event)
		{
			SetEvent(crate->scrolling_socket_thread_stop_event);
		}
		else
		{
			if (INVALID_SOCKET!=crate->scrolling_socket)
			{
				closesocket(crate->scrolling_socket);
				crate->scrolling_socket=INVALID_SOCKET;
			}
		}
		if (crate->calibration_socket_thread_stop_event)
		{
			SetEvent(crate->calibration_socket_thread_stop_event);
		}
		else
		{
			if (INVALID_SOCKET!=crate->calibration_socket)
			{
				closesocket(crate->calibration_socket);
				crate->calibration_socket=INVALID_SOCKET;
			}
		}
		if (crate->acquired_socket_thread_stop_event)
		{
			SetEvent(crate->acquired_socket_thread_stop_event);
		}
		else
		{
			if (INVALID_SOCKET!=crate->acquired_socket)
			{
				closesocket(crate->acquired_socket);
				crate->acquired_socket=INVALID_SOCKET;
			}
		}
#endif /* defined (WIN32) */
#if defined (MOTIF)
		if (crate->scrolling_socket_xid)
		{
			XtRemoveInput(crate->scrolling_socket_xid);
			crate->scrolling_socket_xid=0;
		}
		if (INVALID_SOCKET!=crate->scrolling_socket)
		{
			close(crate->scrolling_socket);
			crate->scrolling_socket=INVALID_SOCKET;
		}
		if (crate->calibration_socket_xid)
		{
			XtRemoveInput(crate->calibration_socket_xid);
			crate->calibration_socket_xid=0;
		}
		if (INVALID_SOCKET!=crate->calibration_socket)
		{
			close(crate->calibration_socket);
			crate->calibration_socket=INVALID_SOCKET;
		}
		if (crate->acquired_socket_xid)
		{
			XtRemoveInput(crate->acquired_socket_xid);
			crate->acquired_socket_xid=0;
		}
		if (INVALID_SOCKET!=crate->acquired_socket)
		{
			close(crate->acquired_socket);
			crate->acquired_socket=INVALID_SOCKET;
		}
#endif /* defined (MOTIF) */
#if defined (CACHE_CLIENT_INFORMATION)
		DEALLOCATE(crate->unemap_cards);
		DEALLOCATE(crate->stimulator_card_indices);
#endif /* defined (CACHE_CLIENT_INFORMATION) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"close_crate_connection.  Invalid argument %p",crate);
	}
	LEAVE;

	return (return_code);
} /* close_crate_connection */

static int close_connection(void)
/*******************************************************************************
LAST MODIFIED : 2 July 2000

DESCRIPTION :
Closes the connection with the unemap hardware service.
==============================================================================*/
{
	int return_code;
	struct Unemap_crate *crate;
#if defined (WIN32)
	int i;
#endif /* defined (WIN32) */

	ENTER(close_connection);
	return_code=1;
	if (crate=module_unemap_crates)
	{
#if defined (WIN32)
		i=module_number_of_unemap_crates;
		while ((i>0)&&(NULL==crate->scrolling_socket_thread_stop_event)&&
			(NULL==crate->calibration_socket_thread_stop_event)&&
			(NULL==crate->acquired_socket_thread_stop_event))
		{
			i--;
			crate++;
		}
		crate=module_unemap_crates;
#endif /* defined (WIN32) */
		while (module_number_of_unemap_crates>0)
		{
			close_crate_connection(crate);
			module_number_of_unemap_crates--;
			crate++;
		}
#if defined (WIN32)
		if (0==i)
		{
			WSACleanup();
		}
#endif /* defined (WIN32) */
		DEALLOCATE(module_unemap_crates);
		module_number_of_unemap_crates=0;
		module_number_of_channels=0;
		/*???DB.  seems to need time to settle down.  Something to do with
			close_connection in service */
		sleep(1);
	}
	allow_open_connection=1;
	LEAVE;

	return (return_code);
} /* close_connection */

#if defined (USE_SOCKETS)
static int socket_recv(
#if defined (WIN32)
	SOCKET socket,
#endif /* defined (WIN32) */
#if defined (UNIX)
	int socket,
#endif /* defined (UNIX) */
	unsigned char *buffer,int buffer_length,int flags)
/*******************************************************************************
LAST MODIFIED : 8 October 1999

DESCRIPTION :
Wrapper function for recv.
==============================================================================*/
{
	int buffer_received,return_code;

	ENTER(socket_recv);
	buffer_received=0;
	return_code=0;
	do
	{
		do
		{
			return_code=recv(socket,buffer+buffer_received,
				buffer_length-buffer_received,flags);
		} while ((SOCKET_ERROR==return_code)&&
#if defined (WIN32)
			(WSAEWOULDBLOCK==WSAGetLastError())
#endif /* defined (WIN32) */
#if defined (UNIX)
			(EWOULDBLOCK==errno)
#endif /* defined (UNIX) */
			);
		if (SOCKET_ERROR!=return_code)
		{
			buffer_received += return_code;
		}
	} while ((SOCKET_ERROR!=return_code)&&(0!=return_code)&&
		(buffer_received<buffer_length));
	if (SOCKET_ERROR==return_code)
	{
		display_message(ERROR_MESSAGE,
#if defined (WIN32)
			"socket_recv.  recv() failed %d",WSAGetLastError()
#endif /* defined (WIN32) */
#if defined (UNIX)
			"socket_recv.  recv() failed %d %s",errno,strerror(errno)
#endif /* defined (UNIX) */
			);
	}
	else
	{
		if (0==return_code)
		{
			/* close_connection has to be before display_message, because
				display_message takes long enough that another message will come before
				it finishs */
			close_connection();
			display_message(WARNING_MESSAGE,"socket_recv.  Connection closed");
			return_code=SOCKET_ERROR;
		}
		else
		{
			return_code=buffer_received;
		}
	}
	LEAVE;

	return (return_code);
} /* socket_recv */
#endif /* defined (USE_SOCKETS) */

#define SEND_BLOCK_SIZE 4096

#if defined (USE_SOCKETS)
static int socket_send(
#if defined (WIN32)
	SOCKET socket,
#endif /* defined (WIN32) */
#if defined (UNIX)
	int socket,
#endif /* defined (UNIX) */
	unsigned char *buffer,int buffer_length,int flags)
/*******************************************************************************
LAST MODIFIED : 8 October 1999

DESCRIPTION :
Wrapper function for send.
==============================================================================*/
{
	unsigned char *local_buffer;
	int local_buffer_length,return_code;
#if defined (WIN32)
	int last_error;
#endif /* defined (WIN32) */

	/* send in blocks */
	local_buffer=buffer;
	local_buffer_length=buffer_length;
	return_code=0;
	do
	{
		do
		{
			if (local_buffer_length>SEND_BLOCK_SIZE)
			{
				return_code=send(socket,local_buffer,SEND_BLOCK_SIZE,flags);
			}
			else
			{
				return_code=send(socket,local_buffer,local_buffer_length,flags);
			}
		} while ((SOCKET_ERROR==return_code)&&
#if defined (WIN32)
			(WSAEWOULDBLOCK==(last_error=WSAGetLastError()))
#endif /* defined (WIN32) */
#if defined (UNIX)
			(EWOULDBLOCK==errno)
#endif /* defined (UNIX) */
			);
		if (SOCKET_ERROR!=return_code)
		{
			local_buffer_length -= return_code;
			local_buffer += return_code;
		}
	} while ((SOCKET_ERROR!=return_code)&&(0!=return_code)&&
		(local_buffer_length>0));
	if (SOCKET_ERROR==return_code)
	{
		display_message(ERROR_MESSAGE,
#if defined (WIN32)
			"socket_send.  send() failed %d",last_error
#endif /* defined (WIN32) */
#if defined (UNIX)
			"socket_send.  send() failed %d %s",errno,strerror(errno)
#endif /* defined (UNIX) */
			);
	}
	else
	{
		if (0==return_code)
		{
			/* close_connection has to be before display_message, because
				display_message takes long enough that another message will come before
				it finishs */
			close_connection();
			display_message(WARNING_MESSAGE,"socket_send.  Connection closed");
			return_code=SOCKET_ERROR;
		}
		else
		{
			return_code=buffer_length;
		}
	}

	return (return_code);
} /* socket_send */
#endif /* defined (USE_SOCKETS) */

#if defined (USE_SOCKETS)
void acquired_socket_callback(
#if defined (WINDOWS)
	LPVOID *crate_void
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
	XtPointer crate_void,int *source,XtInputId *id
#endif /* defined (MOTIF) */
	)
/*******************************************************************************
LAST MODIFIED : 2 July 2000

DESCRIPTION :
Called when there is input on the acquired socket.
???DB.  To be done
==============================================================================*/
{
	int i,j,number_of_channels;
	long message_size;
	short *sample,*samples;
	struct Unemap_crate *crate;
	unsigned char message_header[2+sizeof(long)];
	unsigned long number_of_samples;

	ENTER(acquired_socket_callback);
#if defined (DEBUG)
	display_message(INFORMATION_MESSAGE,"acquired_socket_callback.  %p\n",
		module_acquired_callback);
#endif /* defined (DEBUG) */
#if defined (MOTIF)
	USE_PARAMETER(source);
	USE_PARAMETER(id);
#endif /* defined (MOTIF) */
	if (crate=(struct Unemap_crate *)crate_void)
	{
		(crate->acquired).complete=1;
		/* get the header back */
		if (SOCKET_ERROR!=socket_recv(crate->acquired_socket,message_header,
			2+sizeof(long),0))
		{
			if (message_header[0])
			{
				/* succeeded */
				memcpy(&message_size,message_header+2,sizeof(message_size));
				if (sizeof(number_of_channels)<=message_size)
				{
					if (SOCKET_ERROR!=socket_recv(crate->acquired_socket,
						(unsigned char *)&number_of_channels,sizeof(number_of_channels),0))
					{
						message_size -= sizeof(number_of_channels);
						if (sizeof(number_of_samples)<=message_size)
						{
							if (SOCKET_ERROR!=socket_recv(crate->acquired_socket,
								(unsigned char *)&number_of_samples,sizeof(number_of_samples),
								0))
							{
								message_size -= sizeof(number_of_samples);
								if (number_of_channels*number_of_samples*sizeof(short)==
									message_size)
								{
									ALLOCATE(samples,short,number_of_channels*number_of_samples);
									if (samples&&(SOCKET_ERROR!=socket_recv(
										crate->acquired_socket,(unsigned char *)samples,
										number_of_channels*(int)number_of_samples*sizeof(short),0)))
									{
										(crate->acquired).number_of_channels=number_of_channels;
										(crate->acquired).number_of_samples=number_of_samples;
										(crate->acquired).samples=samples;
									}
									else
									{
										DEALLOCATE(samples);
									}
								}
							}
						}
					}
				}
			}
			else
			{
				DEALLOCATE((crate->acquired).samples);
				(crate->acquired).number_of_channels=0;
				(crate->acquired).number_of_samples=0;
			}
		}
		i=module_number_of_unemap_crates;
		crate=module_unemap_crates;
		number_of_channels=0;
		number_of_samples=0;
		while ((i>0)&&((crate->acquired).complete))
		{
			if (0<(crate->acquired).number_of_channels)
			{
				number_of_channels += (crate->acquired).number_of_channels;
				number_of_samples=(crate->acquired).number_of_samples;
			}
			crate++;
			i--;
		}
		if (0==i)
		{
			if ((0<number_of_channels)&&(0<number_of_samples))
			{
				ALLOCATE(samples,short,number_of_channels*number_of_samples);
				if (samples)
				{
					if (0==module_acquired_channel_number)
					{
						sample=samples;
						for (j=0;j<number_of_samples;j++)
						{
							crate=module_unemap_crates;
							for (i=module_number_of_unemap_crates;i>0;i--)
							{
								memcpy((char *)sample,((crate->acquired).samples)+
									(j*((crate->acquired).number_of_channels)),
									((crate->acquired).number_of_channels)*sizeof(short));
								sample += (crate->acquired).number_of_channels;
								crate++;
							}
						}
					}
					else
					{
						i=module_number_of_unemap_crates;
						crate=module_unemap_crates;
						while ((i>0)&&(0==(crate->acquired).number_of_channels))
						{
							crate++;
							i--;
						}
						if (0<i)
						{
							memcpy((char *)samples,(crate->acquired).samples,
								number_of_samples*sizeof(short));
						}
					}
					(*module_acquired_callback)(module_acquired_channel_number,
						(int)number_of_samples,samples,module_acquired_callback_data);
				}
				DEALLOCATE(samples);
			}
			module_acquired_callback=(Acquired_data_callback *)NULL;
			module_acquired_callback_data=(void *)NULL;
		}
	}
	LEAVE;
} /* acquired_socket_callback */
#endif /* defined (USE_SOCKETS) */

#if defined (WINDOWS) && defined (USE_SOCKETS)
DWORD WINAPI acquired_thread_function(LPVOID crate_void)
/*******************************************************************************
LAST MODIFIED : 3 July 2000

DESCRIPTION :
Thread to watch the acquired socket.
???DB.  Change hEvents[1] to automatic ?
==============================================================================*/
{
	DWORD dwWait,return_code;
	HANDLE hEvents[2]={NULL,NULL};
	int i,running;
	struct Unemap_crate *crate;

	ENTER(acquired_thread_function);
	if (crate=(struct Unemap_crate *)crate_void)
	{
		hEvents[0]=NULL;
		hEvents[1]=NULL;
		return_code=0;
		crate->acquired_socket_thread_stop_event=CreateEvent(
			/*no security attributes*/NULL,/*manual reset event*/TRUE,
			/*not-signalled*/FALSE,/*no name*/NULL);
		if (crate->acquired_socket_thread_stop_event)
		{
			hEvents[0]=crate->acquired_socket_thread_stop_event;
			/* create the event object object use in overlapped i/o */
			hEvents[1]=CreateEvent(/*no security attributes*/NULL,
				/*manual reset event*/TRUE,/*not-signalled*/FALSE,/*no name*/NULL);
			if (hEvents[1])
			{
				if (0==WSAEventSelect(crate->acquired_socket,hEvents[1],FD_READ))
				{
					running=1;
					while (1==running)
					{
						dwWait=WaitForMultipleObjects(2,hEvents,FALSE,INFINITE);
						if (WAIT_OBJECT_0+1==dwWait)
						{
							ResetEvent(hEvents[1]);
							acquired_socket_callback(crate_void);
						}
						else
						{
							running=0;
#if defined (DEBUG)
							display_message(INFORMATION_MESSAGE,
								"acquired_thread_function.  Stop %d %d\n",dwWait,
								WAIT_OBJECT_0+1);
#endif /* defined (DEBUG) */
						}
					}
				}
			}
		}
		/* cleanup */
		if (INVALID_SOCKET!=crate->acquired_socket)
		{
			closesocket(crate->acquired_socket);
			crate->acquired_socket=INVALID_SOCKET;
		}
		if (crate->acquired_socket_thread_stop_event)
		{
			CloseHandle(crate->acquired_socket_thread_stop_event);
			crate->acquired_socket_thread_stop_event=NULL;
		}
		/* overlapped i/o event */
		if (hEvents[1])
		{
			CloseHandle(hEvents[1]);
		}
		i=module_number_of_unemap_crates;
		crate=module_unemap_crates;
		while ((i>0)&&(INVALID_SOCKET==crate->scrolling_socket)&&
			(INVALID_SOCKET==crate->calibration_socket)&&
			(INVALID_SOCKET==crate->acquired_socket))
		{
			i--;
			crate++;
		}
		if (0==i)
		{
			WSACleanup();
		}
	}
	LEAVE;

	return (return_code);
} /* acquired_thread_function */
#endif /* defined (WINDOWS) && defined (USE_SOCKETS) */

#if defined (USE_SOCKETS)
void calibration_socket_callback(
#if defined (WINDOWS)
	LPVOID *crate_void
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
	XtPointer crate_void,int *source,XtInputId *id
#endif /* defined (MOTIF) */
	)
/*******************************************************************************
LAST MODIFIED : 29 February 2000

DESCRIPTION :
Called when there is input on the calibration socket.
==============================================================================*/
{
	float *channel_gains,*channel_offsets;
	int *channel_numbers,i,number_of_channels;
	long message_size;
	struct Unemap_crate *crate;
	unsigned char message_header[2+sizeof(long)];

	ENTER(calibration_socket_callback);
#if defined (DEBUG)
	display_message(INFORMATION_MESSAGE,"calibration_socket_callback.  %p\n",
		module_calibration_end_callback);
#endif /* defined (DEBUG) */
#if defined (MOTIF)
	USE_PARAMETER(source);
	USE_PARAMETER(id);
#endif /* defined (MOTIF) */
	if (crate=(struct Unemap_crate *)crate_void)
	{
		(crate->calibration).complete=1;
		(crate->calibration).number_of_channels=0;
		DEALLOCATE((crate->calibration).channel_numbers);
		DEALLOCATE((crate->calibration).channel_offsets);
		DEALLOCATE((crate->calibration).channel_gains);
		/* get the header back */
		if (SOCKET_ERROR!=socket_recv(crate->calibration_socket,message_header,
			2+sizeof(long),0))
		{
			if (message_header[0])
			{
				/* succeeded */
				memcpy(&message_size,message_header+2,sizeof(message_size));
				if (sizeof(number_of_channels)<=message_size)
				{
					if (SOCKET_ERROR!=socket_recv(crate->calibration_socket,
						(unsigned char *)&number_of_channels,sizeof(number_of_channels),0))
					{
						if (number_of_channels==
							(int)((message_size-sizeof(number_of_channels))/
							(sizeof(int)+2*sizeof(float))))
						{
							ALLOCATE(channel_numbers,int,number_of_channels);
							ALLOCATE(channel_gains,float,number_of_channels);
							ALLOCATE(channel_offsets,float,number_of_channels);
							if (channel_numbers&&channel_gains&&channel_offsets&&
								(SOCKET_ERROR!=socket_recv(crate->calibration_socket,
								(unsigned char *)channel_numbers,number_of_channels*sizeof(int),
								0))&&(SOCKET_ERROR!=socket_recv(crate->calibration_socket,
								(unsigned char *)channel_offsets,
								number_of_channels*sizeof(float),0))&&
								(SOCKET_ERROR!=socket_recv(crate->calibration_socket,
								(unsigned char *)channel_gains,number_of_channels*sizeof(float),
								0)))
							{
								(crate->calibration).number_of_channels=number_of_channels;
								(crate->calibration).channel_numbers=channel_numbers;
								(crate->calibration).channel_offsets=channel_offsets;
								(crate->calibration).channel_gains=channel_gains;
							}
							else
							{
								DEALLOCATE(channel_numbers);
								DEALLOCATE(channel_gains);
								DEALLOCATE(channel_offsets);
							}
						}
					}
				}
			}
		}
		if (module_calibration_end_callback)
		{
			i=module_number_of_unemap_crates;
			crate=module_unemap_crates;
			number_of_channels=0;
			while ((i>0)&&((crate->calibration).complete))
			{
				number_of_channels += (crate->calibration).number_of_channels;
				crate++;
				i--;
			}
			if (0==i)
			{
				if (0<number_of_channels)
				{
					ALLOCATE(channel_numbers,int,number_of_channels);
					ALLOCATE(channel_gains,float,number_of_channels);
					ALLOCATE(channel_offsets,float,number_of_channels);
					if (channel_numbers&&channel_gains&&channel_offsets)
					{
						i=module_number_of_unemap_crates;
						crate=module_unemap_crates;
						number_of_channels=0;
						while ((i>0)&&((crate->calibration).complete))
						{
							memcpy((char *)(channel_numbers+number_of_channels),
								(char *)((crate->calibration).channel_numbers),
								((crate->calibration).number_of_channels)*sizeof(int));
							memcpy((char *)(channel_gains+number_of_channels),
								(char *)((crate->calibration).channel_gains),
								((crate->calibration).number_of_channels)*sizeof(float));
							memcpy((char *)(channel_offsets+number_of_channels),
								(char *)((crate->calibration).channel_offsets),
								((crate->calibration).number_of_channels)*sizeof(float));
							number_of_channels +=
								(crate->calibration).number_of_channels;
							crate++;
							i--;
						}
						(*module_calibration_end_callback)(number_of_channels,
							(const int *)channel_numbers,(const float *)channel_offsets,
							(const float *)channel_gains,
							module_calibration_end_callback_data);
					}
					else
					{
						DEALLOCATE(channel_numbers);
						DEALLOCATE(channel_gains);
						DEALLOCATE(channel_offsets);
						(*module_calibration_end_callback)(0,(const int *)NULL,
							(const float *)NULL,(const float *)NULL,
							module_calibration_end_callback_data);
					}
				}
				else
				{
					(*module_calibration_end_callback)(0,(const int *)NULL,
						(const float *)NULL,(const float *)NULL,
						module_calibration_end_callback_data);
				}
			}
		}
	}
	LEAVE;
} /* calibration_socket_callback */
#endif /* defined (USE_SOCKETS) */

#if defined (WINDOWS) && defined (USE_SOCKETS)
DWORD WINAPI calibration_thread_function(LPVOID crate_void)
/*******************************************************************************
LAST MODIFIED : 2 March 2000

DESCRIPTION :
Thread to watch the calibration socket.
???DB.  Change hEvents[1] to automatic ?
==============================================================================*/
{
	DWORD dwWait,return_code;
	HANDLE hEvents[2]={NULL,NULL};
	int i,running;
	struct Unemap_crate *crate;

	ENTER(calibration_thread_function);
	if (crate=(struct Unemap_crate *)crate_void)
	{
		hEvents[0]=NULL;
		hEvents[1]=NULL;
		return_code=0;
		crate->calibration_socket_thread_stop_event=CreateEvent(
			/*no security attributes*/NULL,/*manual reset event*/TRUE,
			/*not-signalled*/FALSE,/*no name*/NULL);
		if (crate->calibration_socket_thread_stop_event)
		{
			hEvents[0]=crate->calibration_socket_thread_stop_event;
			/* create the event object object use in overlapped i/o */
			hEvents[1]=CreateEvent(/*no security attributes*/NULL,
				/*manual reset event*/TRUE,/*not-signalled*/FALSE,/*no name*/NULL);
			if (hEvents[1])
			{
				if (0==WSAEventSelect(crate->calibration_socket,hEvents[1],FD_READ))
				{
					running=1;
					while (1==running)
					{
						dwWait=WaitForMultipleObjects(2,hEvents,FALSE,INFINITE);
						if (WAIT_OBJECT_0+1==dwWait)
						{
							ResetEvent(hEvents[1]);
							calibration_socket_callback(crate_void);
						}
						else
						{
							running=0;
#if defined (DEBUG)
							display_message(INFORMATION_MESSAGE,
								"calibration_thread_function.  Stop %d %d\n",dwWait,
								WAIT_OBJECT_0+1);
#endif /* defined (DEBUG) */
						}
					}
				}
			}
		}
		/* cleanup */
		if (INVALID_SOCKET!=crate->calibration_socket)
		{
			closesocket(crate->calibration_socket);
			crate->calibration_socket=INVALID_SOCKET;
		}
		if (crate->calibration_socket_thread_stop_event)
		{
			CloseHandle(crate->calibration_socket_thread_stop_event);
			crate->calibration_socket_thread_stop_event=NULL;
		}
		/* overlapped i/o event */
		if (hEvents[1])
		{
			CloseHandle(hEvents[1]);
		}
		i=module_number_of_unemap_crates;
		crate=module_unemap_crates;
		while ((i>0)&&(INVALID_SOCKET==crate->scrolling_socket)&&
			(INVALID_SOCKET==crate->calibration_socket)&&
			(INVALID_SOCKET==crate->acquired_socket))
		{
			i--;
			crate++;
		}
		if (0==i)
		{
			WSACleanup();
		}
	}
	LEAVE;

	return (return_code);
} /* calibration_thread_function */
#endif /* defined (WINDOWS) && defined (USE_SOCKETS) */

#if defined (USE_SOCKETS)
void scrolling_socket_callback(
#if defined (WINDOWS)
	LPVOID crate_void
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
	XtPointer crate_void,int *source,XtInputId *id
#endif /* defined (MOTIF) */
	)
/*******************************************************************************
LAST MODIFIED : 2 April 2000

DESCRIPTION :
Called when there is input on the scrolling socket.
???DB.  What if scrolling callback for next event comes back before all crates
	have sent back current?
==============================================================================*/
{
	int *channel_numbers,i,number_of_channels,number_of_values_per_channel,
		*temp_channel_numbers;
	long message_size;
	short *temp_values,*values;
	struct Unemap_crate *crate;
	unsigned char *byte_array,message_header[2+sizeof(long)];
#if defined (WINDOWS)
	int length;
	unsigned char *temp_byte_array;
#endif /* defined (WINDOWS) */

	ENTER(scrolling_socket_callback);
#if defined (MOTIF)
	USE_PARAMETER(source);
	USE_PARAMETER(id);
#endif /* defined (MOTIF) */
#if defined (DEBUG)
	/*???debug */
	{
		static int number_of_scrolling_socket_callbacks=0;

		if (number_of_scrolling_socket_callbacks<5)
		{
/*			display_message(INFORMATION_MESSAGE,*/
			printf(
				"scrolling_socket_callback.  %p %d %p\n",crate_void,
				number_of_scrolling_socket_callbacks,module_scrolling_callback);
			number_of_scrolling_socket_callbacks++;
		}
	}
#endif /* defined (DEBUG) */
	if (crate=(struct Unemap_crate *)crate_void)
	{
		/* get the header back */
		if (SOCKET_ERROR!=socket_recv(crate->scrolling_socket,message_header,
			2+sizeof(long),0))
		{
			memcpy(&message_size,message_header+2,sizeof(message_size));
			if (ALLOCATE(byte_array,unsigned char,message_size))
			{
				if (SOCKET_ERROR!=socket_recv(crate->scrolling_socket,byte_array,
					message_size,0))
				{
					if (module_scrolling_callback
#if defined (WIN32)
						||module_scrolling_window
#endif /* defined (WIN32) */
						)
					{
						number_of_channels= *((int *)byte_array);
						number_of_values_per_channel=
							*((int *)(byte_array+(number_of_channels+1)*sizeof(int)));
						if ((0<number_of_channels)&&(0<number_of_values_per_channel)&&
							(message_size==(long)((number_of_channels+2)*sizeof(int)+
							number_of_channels*number_of_values_per_channel*sizeof(short))))
						{
							if ((number_of_channels!=(crate->scrolling).number_of_channels)||
								(number_of_values_per_channel!=
								(crate->scrolling).number_of_values_per_channel))
							{
								REALLOCATE(channel_numbers,(crate->scrolling).channel_numbers,
									int,number_of_channels);
								REALLOCATE(values,(crate->scrolling).values,short,
									number_of_channels*number_of_values_per_channel);
								if (channel_numbers&&values)
								{
									(crate->scrolling).number_of_channels=number_of_channels;
									(crate->scrolling).channel_numbers=channel_numbers;
									(crate->scrolling).number_of_values_per_channel=
										number_of_values_per_channel;
									(crate->scrolling).values=values;
								}
								else
								{
									if (channel_numbers)
									{
										DEALLOCATE(channel_numbers);
										(crate->scrolling).channel_numbers=(int *)NULL;
									}
									else
									{
										DEALLOCATE((crate->scrolling).channel_numbers);
									}
									(crate->scrolling).number_of_channels=0;
									if (values)
									{
										DEALLOCATE(values);
										(crate->scrolling).values=(short *)NULL;
									}
									else
									{
										DEALLOCATE((crate->scrolling).values);
									}
									(crate->scrolling).number_of_values_per_channel=0;
								}
							}
							else
							{
								channel_numbers=(crate->scrolling).channel_numbers;
								values=(crate->scrolling).values;
							}
							if (channel_numbers&&values)
							{
								memcpy((char *)channel_numbers,(char *)(byte_array+sizeof(int)),
									number_of_channels*sizeof(int));
								memcpy((char *)values,(char *)(byte_array+
									(number_of_channels+2)*sizeof(int)),number_of_channels*
									number_of_values_per_channel*sizeof(short));
								(crate->scrolling).complete |=
									SCROLLING_CHANNELS_SUCCEEDED_FOR_CRATE;
							}
							else
							{
								(crate->scrolling).complete |=
									SCROLLING_CHANNELS_FAILED_FOR_CRATE;
							}
						}
						else
						{
							(crate->scrolling).complete |=
								SCROLLING_CHANNELS_FAILED_FOR_CRATE;
						}
					}
					else
					{
						(crate->scrolling).complete |=
							SCROLLING_CHANNELS_SUCCEEDED_FOR_CRATE;
					}
				}
				else
				{
					(crate->scrolling).complete |= SCROLLING_CHANNELS_FAILED_FOR_CRATE;
				}
				DEALLOCATE(byte_array);
			}
			else
			{
				(crate->scrolling).complete |= SCROLLING_CHANNELS_FAILED_FOR_CRATE;
			}
		}
		else
		{
			(crate->scrolling).complete |= SCROLLING_CHANNELS_FAILED_FOR_CRATE;
		}
		if (module_scrolling_callback
#if defined (WIN32)
			||module_scrolling_window
#endif /* defined (WIN32) */
			)
		{
			i=module_number_of_unemap_crates;
			crate=module_unemap_crates;
			number_of_channels=0;
			number_of_values_per_channel=0;
			while ((i>0)&&((crate->scrolling).complete))
			{
				if ((crate->scrolling).complete&SCROLLING_CHANNELS_SUCCEEDED_FOR_CRATE)
				{
					number_of_channels += (crate->scrolling).number_of_channels;
					number_of_values_per_channel=
						(crate->scrolling).number_of_values_per_channel;
				}
				crate++;
				i--;
			}
			if (0==i)
			{
				if (0<number_of_channels)
				{
#if defined (WIN32)
					if (module_scrolling_callback)
					{
#endif /* defined (WIN32) */
						ALLOCATE(channel_numbers,int,number_of_channels);
						ALLOCATE(values,short,
							number_of_channels*number_of_values_per_channel);
						if (channel_numbers&&values)
						{
							temp_channel_numbers=channel_numbers;
							temp_values=values;
							i=module_number_of_unemap_crates;
							crate=module_unemap_crates;
							while ((i>0)&&((crate->scrolling).complete))
							{
								if ((crate->scrolling).complete&
									SCROLLING_CHANNELS_SUCCEEDED_FOR_CRATE)
								{
									memcpy((char *)temp_channel_numbers,
										(char *)((crate->scrolling).channel_numbers),
										((crate->scrolling).number_of_channels)*sizeof(int));
									temp_channel_numbers += (crate->scrolling).number_of_channels;
									memcpy((char *)temp_values,
										(char *)((crate->scrolling).values),
										((crate->scrolling).number_of_channels)*
										number_of_values_per_channel*sizeof(short));
									temp_values += ((crate->scrolling).number_of_channels)*
										number_of_values_per_channel;
								}
								crate++;
								i--;
							}
							(*module_scrolling_callback)(number_of_channels,channel_numbers,
								number_of_values_per_channel,values,
								module_scrolling_callback_data);
						}
#if defined (WIN32)
					}
					if (module_scrolling_window)
					{
						ALLOCATE(byte_array,char,number_of_channels*sizeof(int)+
							number_of_channels*number_of_values_per_channel*sizeof(short));
						if (byte_array)
						{
							i=module_number_of_unemap_crates;
							crate=module_unemap_crates;
							temp_byte_array=byte_array;
							while ((i>0)&&((crate->scrolling).complete))
							{
								if ((crate->scrolling).complete&
									SCROLLING_CHANNELS_SUCCEEDED_FOR_CRATE)
								{
									length=((crate->scrolling).number_of_channels)*sizeof(int);
									memcpy((char *)byte_array,
										(char *)((crate->scrolling).channel_numbers),length);
									byte_array += length;
									length=((crate->scrolling).number_of_channels)*
										number_of_values_per_channel*sizeof(short);
									byte_array += length;
								}
								crate++;
								i--;
							}
							message_size=number_of_channels*(sizeof(int)+
								number_of_values_per_channel*sizeof(short));
							PostMessage(module_scrolling_window,module_scrolling_message,
								(WPARAM)byte_array,(ULONG)message_size);
						}
					}
#endif /* defined (WIN32) */
				}
				/* reset for next scrolling */
				crate=module_unemap_crates;
				for (i=module_number_of_unemap_crates;i>0;i--)
				{
					(crate->scrolling).complete &=
						~(SCROLLING_CHANNELS_SUCCEEDED_FOR_CRATE|
						SCROLLING_CHANNELS_FAILED_FOR_CRATE);
					crate++;
				}
			}
		}
	}
	LEAVE;
} /* scrolling_socket_callback */
#endif /* defined (USE_SOCKETS) */

#if defined (WINDOWS) && defined (USE_SOCKETS)
DWORD WINAPI scrolling_thread_function(LPVOID crate_void)
/*******************************************************************************
LAST MODIFIED : 12 March 2000

DESCRIPTION :
Thread to watch the scrolling socket.
???DB.  Change hEvents[1] to automatic ?
==============================================================================*/
{
	DWORD dwWait,return_code;
	HANDLE hEvents[2]={NULL,NULL};
	int i,running;
	struct Unemap_crate *crate;

	ENTER(scrolling_thread_function);
	if (crate=(struct Unemap_crate *)crate_void)
	{
		hEvents[0]=NULL;
		hEvents[1]=NULL;
		return_code=0;
		crate->scrolling_socket_thread_stop_event=CreateEvent(
			/*no security attributes*/NULL,
			/*manual reset event*/TRUE,/*not-signalled*/FALSE,/*no name*/NULL);
		if (crate->scrolling_socket_thread_stop_event)
		{
			hEvents[0]=crate->scrolling_socket_thread_stop_event;
			/* create the event object object use in overlapped i/o */
			hEvents[1]=CreateEvent(/*no security attributes*/NULL,
				/*manual reset event*/TRUE,/*not-signalled*/FALSE,/*no name*/NULL);
			if (hEvents[1])
			{
				if (0==WSAEventSelect(crate->scrolling_socket,hEvents[1],FD_READ))
				{
					running=1;
					while (1==running)
					{
						dwWait=WaitForMultipleObjects(2,hEvents,FALSE,INFINITE);
						if (WAIT_OBJECT_0+1==dwWait)
						{
							ResetEvent(hEvents[1]);
							scrolling_socket_callback(crate_void);
						}
						else
						{
							running=0;
	#if defined (DEBUG)
							display_message(INFORMATION_MESSAGE,
								"scrolling_thread_function.  Stop %d %d\n",dwWait,
								WAIT_OBJECT_0+1);
	#endif /* defined (DEBUG) */
						}
					}
				}
			}
		}
		/* cleanup */
		if (INVALID_SOCKET!=crate->scrolling_socket)
		{
			closesocket(crate->scrolling_socket);
			crate->scrolling_socket=INVALID_SOCKET;
		}
		if (crate->scrolling_socket_thread_stop_event)
		{
			CloseHandle(crate->scrolling_socket_thread_stop_event);
			crate->scrolling_socket_thread_stop_event=NULL;
		}
		/* overlapped i/o event */
		if (hEvents[1])
		{
			CloseHandle(hEvents[1]);
		}
		i=module_number_of_unemap_crates;
		crate=module_unemap_crates;
		while ((i>0)&&(INVALID_SOCKET==crate->scrolling_socket)&&
			(INVALID_SOCKET==crate->calibration_socket)&&
			(INVALID_SOCKET==crate->acquired_socket))
		{
			i--;
			crate++;
		}
		if (0==i)
		{
			WSACleanup();
		}
	}
	LEAVE;

	return (return_code);
} /* scrolling_thread_function */
#endif /* defined (WINDOWS) && defined (USE_SOCKETS) */

static int crate_configure_start(struct Unemap_crate *crate,int slave,
	float sampling_frequency,int number_of_samples_in_buffer,
#if defined (WINDOWS)
	HWND scrolling_window,UINT scrolling_message,
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
	XtAppContext application_context,
#endif /* defined (MOTIF) */
	Unemap_hardware_callback *scrolling_callback,void *scrolling_callback_data,
	float scrolling_refresh_frequency,int synchronization_card)
/*******************************************************************************
LAST MODIFIED : 9 July 2000

DESCRIPTION :
Configures the <crate> for sampling at the specified <sampling_frequency> and
with the specified <number_of_samples_in_buffer>. <sampling_frequency> and
<number_of_samples_in_buffer> are requests only.  If <slave> is non-zero, then
the sampling control signal comes from another crate, otherwise it is generated
by the crate.

See <unemap_configure> for more details.

???DB.  To be done.  Implement slave sampling for hardware.
==============================================================================*/
{
	float sampling_frequency_slave,temp_scrolling_refresh_frequency;
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+2*sizeof(float)+2*sizeof(int)+sizeof(long)];
#if defined (WINDOWS)
	DWORD acquired_thread_id;
	HANDLE acquired_thread;
		/*???DB.  Global variable ? */
	DWORD calibration_thread_id;
	HANDLE calibration_thread;
		/*???DB.  Global variable ? */
	DWORD scrolling_thread_id;
	HANDLE scrolling_thread;
		/*???DB.  Global variable ? */
#endif /* defined (WINDOWS) */

	ENTER(crate_configure_start);
#if defined (DEBUG)
	/*???debug */
	printf("enter crate_configure_start\n");
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if (crate&&(0<sampling_frequency)&&(0<number_of_samples_in_buffer))
	{
		if ((INVALID_SOCKET!=crate->scrolling_socket)&&
			(INVALID_SOCKET!=crate->calibration_socket)&&
			(INVALID_SOCKET!=crate->acquired_socket)&&
			(INVALID_SOCKET!=crate->command_socket))
		{
			(crate->scrolling).complete=SCROLLING_NO_CHANNELS_FOR_CRATE;
#if defined (WINDOWS)
			if (scrolling_thread=CreateThread(
				/*no security attributes*/NULL,/*use default stack size*/0,
				scrolling_thread_function,(LPVOID)crate,/*use default creation flags*/0,
				&scrolling_thread_id))
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
			if (!application_context||(crate->scrolling_socket_xid=XtAppAddInput(
				application_context,crate->scrolling_socket,(XtPointer)XtInputReadMask,
				scrolling_socket_callback,(XtPointer)crate)))
#endif /* defined (MOTIF) */
			{
#if defined (WINDOWS)
				if (calibration_thread=CreateThread(
					/*no security attributes*/NULL,/*use default stack size*/0,
					calibration_thread_function,(LPVOID)crate,
					/*use default creation flags*/0,&calibration_thread_id))
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
				if (!application_context||(crate->calibration_socket_xid=XtAppAddInput(
					application_context,crate->calibration_socket,
					(XtPointer)XtInputReadMask,calibration_socket_callback,
					(XtPointer)crate)))
#endif /* defined (MOTIF) */
				{
#if defined (WINDOWS)
					if (acquired_thread=CreateThread(
						/*no security attributes*/NULL,/*use default stack size*/0,
						acquired_thread_function,(LPVOID)crate,
						/*use default creation flags*/0,&acquired_thread_id))
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
					if (!application_context||(crate->acquired_socket_xid=XtAppAddInput(
						application_context,crate->acquired_socket,
						(XtPointer)XtInputReadMask,acquired_socket_callback,
						(XtPointer)crate)))
#endif /* defined (MOTIF) */
					{
						buffer[0]=UNEMAP_CONFIGURE_CODE;
						buffer[1]=BIG_ENDIAN_CODE;
						buffer_size=2+sizeof(message_size);
						if (slave)
						{
							sampling_frequency_slave= -sampling_frequency;
						}
						else
						{
							sampling_frequency_slave=sampling_frequency;
						}
						memcpy(buffer+buffer_size,&sampling_frequency_slave,
							sizeof(sampling_frequency_slave));
						buffer_size += sizeof(sampling_frequency_slave);
						memcpy(buffer+buffer_size,&number_of_samples_in_buffer,
							sizeof(number_of_samples_in_buffer));
						buffer_size += sizeof(number_of_samples_in_buffer);
						if ((0<scrolling_refresh_frequency)&&(scrolling_callback
#if defined (WINDOWS)
							||scrolling_window
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
							&&application_context
#endif /* defined (MOTIF) */
							))
						{
							temp_scrolling_refresh_frequency=scrolling_refresh_frequency;
							module_scrolling_callback=scrolling_callback;
							module_scrolling_callback_data=scrolling_callback_data;
#if defined (WINDOWS)
							module_scrolling_message=scrolling_message;
							module_scrolling_window=scrolling_window;
#endif /* defined (WINDOWS) */
						}
						else
						{
							temp_scrolling_refresh_frequency=(float)0;
							module_scrolling_callback=(Unemap_hardware_callback *)NULL;
							module_scrolling_callback_data=(void *)NULL;
#if defined (WINDOWS)
							module_scrolling_message=(UINT)0;
							module_scrolling_window=(HWND)NULL;
#endif /* defined (WINDOWS) */
						}
						memcpy(buffer+buffer_size,&temp_scrolling_refresh_frequency,
							sizeof(temp_scrolling_refresh_frequency));
						buffer_size += sizeof(temp_scrolling_refresh_frequency);
						memcpy(buffer+buffer_size,&synchronization_card,
							sizeof(synchronization_card));
						buffer_size += sizeof(synchronization_card);
						message_size=buffer_size-(2+(long)sizeof(message_size));
						memcpy(buffer+2,&message_size,sizeof(message_size));
#if defined (DEBUG)
						/*???debug */
						display_message(INFORMATION_MESSAGE,
							"crate_configure_start.  %g %d %g %g\n",sampling_frequency,
							number_of_samples_in_buffer,temp_scrolling_refresh_frequency,
							scrolling_refresh_frequency);
#endif /* defined (DEBUG) */
						retval=socket_send(crate->command_socket,buffer,buffer_size,0);
						if (SOCKET_ERROR!=retval)
						{
#if defined (DEBUG)
							/*???debug */
							display_message(INFORMATION_MESSAGE,
								"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"crate_configure_start.  socket_send() failed");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
#if defined (WINDOWS)
						"crate_configure_start.  CreateThread failed for acquired socket");
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
						"crate_configure_start.  XtAppAddInput failed for acquired socket");
#endif /* defined (MOTIF) */
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
#if defined (WINDOWS)
					"crate_configure_start.  CreateThread failed for calibration socket");
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
				"crate_configure_start.  XtAppAddInput failed for calibration socket");
#endif /* defined (MOTIF) */
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
#if defined (WINDOWS)
					"crate_configure_start.  CreateThread failed for scrolling socket");
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
					"crate_configure_start.  XtAppAddInput failed for scrolling socket");
#endif /* defined (MOTIF) */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
"crate_configure_start.  Invalid command_socket (%p) or calibration_socket (%p) or scrolling_socket (%p) or acquired_socket (%p)",
				crate->command_socket,crate->calibration_socket,
				crate->scrolling_socket,crate->acquired_socket);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_configure_start.  Invalid argument(s).  %p %g %d",crate,
			sampling_frequency,number_of_samples_in_buffer);
		return_code=0;
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave crate_configure_start\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_configure_start */

static int crate_configure_end(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 9 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code,retval;
	long buffer_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_configure_end);
	return_code=0;
	/* check argument */
	if (crate)
	{
		/* get the header back */
		retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Received %d bytes, data %x %x %ld\n",retval,buffer[0],
				buffer[1],buffer_size);
#endif /* defined (DEBUG) */
			if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
			{
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_configure_end.  socket_recv() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_configure_end.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* crate_configure_end */

#if defined (OLD_CODE)
static int crate_configure(struct Unemap_crate *crate,int slave,
	float sampling_frequency,int number_of_samples_in_buffer,
#if defined (WINDOWS)
	HWND scrolling_window,UINT scrolling_message,
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
	XtAppContext application_context,
#endif /* defined (MOTIF) */
	Unemap_hardware_callback *scrolling_callback,void *scrolling_callback_data,
	float scrolling_refresh_frequency,int synchronization_card)
/*******************************************************************************
LAST MODIFIED : 9 July 2000

DESCRIPTION :
Configures the <crate> for sampling at the specified <sampling_frequency> and
with the specified <number_of_samples_in_buffer>. <sampling_frequency> and
<number_of_samples_in_buffer> are requests only.  If <slave> is non-zero, then
the sampling control signal comes from another crate, otherwise it is generated
by the crate.

See <unemap_configure> for more details.

???DB.  To be done.  Implement slave sampling for hardware.
==============================================================================*/
{
	float sampling_frequency_slave,temp_scrolling_refresh_frequency;
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+2*sizeof(float)+2*sizeof(int)+sizeof(long)];
#if defined (WINDOWS)
	DWORD acquired_thread_id;
	HANDLE acquired_thread;
		/*???DB.  Global variable ? */
	DWORD calibration_thread_id;
	HANDLE calibration_thread;
		/*???DB.  Global variable ? */
	DWORD scrolling_thread_id;
	HANDLE scrolling_thread;
		/*???DB.  Global variable ? */
#endif /* defined (WINDOWS) */

	ENTER(crate_configure);
#if defined (DEBUG)
	/*???debug */
	printf("enter crate_configure\n");
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if (crate&&(0<sampling_frequency)&&(0<number_of_samples_in_buffer))
	{
		if ((INVALID_SOCKET!=crate->scrolling_socket)&&
			(INVALID_SOCKET!=crate->calibration_socket)&&
			(INVALID_SOCKET!=crate->acquired_socket)&&
			(INVALID_SOCKET!=crate->command_socket))
		{
			(crate->scrolling).complete=SCROLLING_NO_CHANNELS_FOR_CRATE;
#if defined (WINDOWS)
			if (scrolling_thread=CreateThread(
				/*no security attributes*/NULL,/*use default stack size*/0,
				scrolling_thread_function,(LPVOID)crate,/*use default creation flags*/0,
				&scrolling_thread_id))
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
			if (!application_context||(crate->scrolling_socket_xid=XtAppAddInput(
				application_context,crate->scrolling_socket,(XtPointer)XtInputReadMask,
				scrolling_socket_callback,(XtPointer)crate)))
#endif /* defined (MOTIF) */
			{
#if defined (WINDOWS)
				if (calibration_thread=CreateThread(
					/*no security attributes*/NULL,/*use default stack size*/0,
					calibration_thread_function,(LPVOID)crate,
					/*use default creation flags*/0,&calibration_thread_id))
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
				if (!application_context||(crate->calibration_socket_xid=XtAppAddInput(
					application_context,crate->calibration_socket,
					(XtPointer)XtInputReadMask,calibration_socket_callback,
					(XtPointer)crate)))
#endif /* defined (MOTIF) */
				{
#if defined (WINDOWS)
					if (acquired_thread=CreateThread(
						/*no security attributes*/NULL,/*use default stack size*/0,
						acquired_thread_function,(LPVOID)crate,
						/*use default creation flags*/0,&acquired_thread_id))
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
					if (!application_context||(crate->acquired_socket_xid=XtAppAddInput(
						application_context,crate->acquired_socket,
						(XtPointer)XtInputReadMask,acquired_socket_callback,
						(XtPointer)crate)))
#endif /* defined (MOTIF) */
					{
						buffer[0]=UNEMAP_CONFIGURE_CODE;
						buffer[1]=BIG_ENDIAN_CODE;
						buffer_size=2+sizeof(message_size);
						if (slave)
						{
							sampling_frequency_slave= -sampling_frequency;
						}
						else
						{
							sampling_frequency_slave=sampling_frequency;
						}
						memcpy(buffer+buffer_size,&sampling_frequency_slave,
							sizeof(sampling_frequency_slave));
						buffer_size += sizeof(sampling_frequency_slave);
						memcpy(buffer+buffer_size,&number_of_samples_in_buffer,
							sizeof(number_of_samples_in_buffer));
						buffer_size += sizeof(number_of_samples_in_buffer);
						if ((0<scrolling_refresh_frequency)&&(scrolling_callback
#if defined (WINDOWS)
							||scrolling_window
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
							&&application_context
#endif /* defined (MOTIF) */
							))
						{
							temp_scrolling_refresh_frequency=scrolling_refresh_frequency;
							module_scrolling_callback=scrolling_callback;
							module_scrolling_callback_data=scrolling_callback_data;
#if defined (WINDOWS)
							module_scrolling_message=scrolling_message;
							module_scrolling_window=scrolling_window;
#endif /* defined (WINDOWS) */
						}
						else
						{
							temp_scrolling_refresh_frequency=(float)0;
							module_scrolling_callback=(Unemap_hardware_callback *)NULL;
							module_scrolling_callback_data=(void *)NULL;
#if defined (WINDOWS)
							module_scrolling_message=(UINT)0;
							module_scrolling_window=(HWND)NULL;
#endif /* defined (WINDOWS) */
						}
						memcpy(buffer+buffer_size,&temp_scrolling_refresh_frequency,
							sizeof(temp_scrolling_refresh_frequency));
						buffer_size += sizeof(temp_scrolling_refresh_frequency);
						memcpy(buffer+buffer_size,&synchronization_card,
							sizeof(synchronization_card));
						buffer_size += sizeof(synchronization_card);
						message_size=buffer_size-(2+(long)sizeof(message_size));
						memcpy(buffer+2,&message_size,sizeof(message_size));
#if defined (DEBUG)
						/*???debug */
						display_message(INFORMATION_MESSAGE,
							"crate_configure.  %g %d %g %g\n",sampling_frequency,
							number_of_samples_in_buffer,temp_scrolling_refresh_frequency,
							scrolling_refresh_frequency);
#endif /* defined (DEBUG) */
						retval=socket_send(crate->command_socket,buffer,buffer_size,0);
						if (SOCKET_ERROR!=retval)
						{
#if defined (DEBUG)
							/*???debug */
							display_message(INFORMATION_MESSAGE,
								"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
							/* get the header back */
							retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
							if (SOCKET_ERROR!=retval)
							{
								memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
								/*???debug */
								display_message(INFORMATION_MESSAGE,
									"Received %d bytes, data %x %x %ld\n",retval,buffer[0],
									buffer[1],buffer_size);
#endif /* defined (DEBUG) */
								if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
								{
									return_code=1;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"crate_configure.  socket_recv() failed");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"crate_configure.  socket_send() failed");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
#if defined (WINDOWS)
							"crate_configure.  CreateThread failed for acquired socket");
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
							"crate_configure.  XtAppAddInput failed for acquired socket");
#endif /* defined (MOTIF) */
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
#if defined (WINDOWS)
						"crate_configure.  CreateThread failed for calibration socket");
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
						"crate_configure.  XtAppAddInput failed for calibration socket");
#endif /* defined (MOTIF) */
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
#if defined (WINDOWS)
					"crate_configure.  CreateThread failed for scrolling socket");
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
					"crate_configure.  XtAppAddInput failed for scrolling socket");
#endif /* defined (MOTIF) */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
"crate_configure.  Invalid command_socket (%p) or calibration_socket (%p) or scrolling_socket (%p) or acquired_socket (%p)",
				crate->command_socket,crate->calibration_socket,
				crate->scrolling_socket,crate->acquired_socket);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_configure.  Invalid argument(s).  %p %g %d",crate,
			sampling_frequency,number_of_samples_in_buffer);
		return_code=0;
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave crate_configure\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_configure */
#endif /* defined (OLD_CODE) */

static int crate_configured(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
Returns a non-zero if <the crate> is configured and zero otherwise.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_configured);
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_CONFIGURED_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		message_size=0;
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,
			0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&message_size,buffer+2,sizeof(message_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,
				2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_configured.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_configured.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_configured.  Missing crate (%p) or invalid command_socket",crate);
	}
	LEAVE;

	return (return_code);
} /* crate_configured */

static int crate_deconfigure(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 12 March 2000

DESCRIPTION :
Stops acquisition and signal generation for the <crate>.  Frees buffers
associated with the hardware.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_deconfigure);
#if defined (DEBUG)
	/*???debug */
	printf("enter crate_deconfigure %p\n",crate);
#endif /* defined (DEBUG) */
	return_code=0;
	if (crate)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_DECONFIGURE_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			message_size=0;
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&message_size,buffer+2,sizeof(message_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
					{
						return_code=1;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_deconfigure.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_deconfigure.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_deconfigure.  Invalid command_socket");
		}
#if defined (CACHE_CLIENT_INFORMATION)
		crate->number_of_unemap_cards=0;
		crate->number_of_stimulators=0;
		DEALLOCATE(crate->unemap_cards);
		DEALLOCATE(crate->stimulator_card_indices);
#endif /* defined (CACHE_CLIENT_INFORMATION) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_deconfigure.  Missing crate");
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave crate_deconfigure %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_deconfigure */

static int crate_get_hardware_version(struct Unemap_crate *crate,
	int *hardware_version)
/*******************************************************************************
LAST MODIFIED : 8 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

Returns the unemap <hardware_version> for the <crate>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_get_hardware_version);
#if defined (DEBUG)
	/*???debug */
	printf("enter crate_get_hardware_version %p %p\n",crate,hardware_version);
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if (crate&&hardware_version)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_GET_HARDWARE_VERSION_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&(sizeof(int)==buffer_size)&&(buffer[0]))
					{
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,buffer,
							sizeof(int),0))
						{
							memcpy(hardware_version,buffer,sizeof(int));
#if defined (DEBUG)
							display_message(INFORMATION_MESSAGE," %d",*hardware_version);
#endif /* defined (DEBUG) */
							return_code=1;
						}
					}
#if defined (DEBUG)
					display_message(INFORMATION_MESSAGE,"\n");
#endif /* defined (DEBUG) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_get_hardware_version.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_hardware_version.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_hardware_version.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"crate_get_hardware_version.  Missing crate (%p) or hardware_version (%p)",
			crate,hardware_version);
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave crate_get_hardware_version %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_get_hardware_version */

static int crate_shutdown(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 8 March 2000

DESCRIPTION :
Shuts down NT running on the signal conditioning unit computer (<crate>).
???DB.  Not really anything to do with unemap hardware ?
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_shutdown);
#if defined (DEBUG)
	/*???debug */
	printf("enter crate_shutdown %p\n",crate);
#endif /* defined (DEBUG) */
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_SHUTDOWN_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		message_size=0;
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&message_size,buffer+2,sizeof(message_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_shutdown.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_shutdown.  socket_send() failed");
		}
#if defined (OLD_CODE)
/*???DB.  Waiting hangs */
		if (return_code)
		{
#if defined (OLD_CODE)
			fd_set acceptfds;
#endif /* defined (OLD_CODE) */

			/* wait for the command socket to be closed */
#if defined (OLD_CODE)
			FD_ZERO(&acceptfds);
			FD_SET(crate->command_socket,&acceptfds);
			select(1,(fd_set *)NULL,(fd_set *)NULL,&acceptfds,(struct timeval *)NULL);
#endif /* defined (OLD_CODE) */
			do
			{
				do
				{
					printf("before recv\n");
					return_code=recv(crate->command_socket,buffer,1,0);
					printf("after recv %d\n",return_code);
				}
				while ((SOCKET_ERROR==return_code)&&
#if defined (WIN32)
					(WSAEWOULDBLOCK==WSAGetLastError())
#endif /* defined (WIN32) */
#if defined (UNIX)
					(EWOULDBLOCK==errno)
#endif /* defined (UNIX) */
					);
			}
			while ((SOCKET_ERROR!=return_code)&&(0!=return_code));
		}
#endif /* defined (OLD_CODE) */
#if defined (NEW_CODE)
/*???DB.  To be done */
		first_invalid_socket_call=1;
		if (INVALID_SOCKET!=crate->command_socket)
		{
#if defined (WIN32)
			closesocket(crate->command_socket);
#endif /* defined (WIN32) */
#if defined (UNIX)
			close(crate->command_socket);
#endif /* defined (UNIX) */
			crate->command_socket=INVALID_SOCKET;
		}
#if defined (WIN32)
		if (scrolling_socket_thread_stop_event)
		{
			SetEvent(scrolling_socket_thread_stop_event);
		}
		else
		{
			if (INVALID_SOCKET!=scrolling_socket)
			{
				closesocket(scrolling_socket);
				scrolling_socket=INVALID_SOCKET;
			}
		}
		if (calibration_socket_thread_stop_event)
		{
			SetEvent(calibration_socket_thread_stop_event);
		}
		else
		{
			if (INVALID_SOCKET!=calibration_socket)
			{
				closesocket(calibration_socket);
				calibration_socket=INVALID_SOCKET;
			}
		}
		if (acquired_socket_thread_stop_event)
		{
			SetEvent(acquired_socket_thread_stop_event);
		}
		else
		{
			if (INVALID_SOCKET!=acquired_socket)
			{
				closesocket(acquired_socket);
				acquired_socket=INVALID_SOCKET;
			}
			WSACleanup();
		}
#endif /* defined (WIN32) */
#if defined (MOTIF)
		if (scrolling_socket_xid)
		{
			XtRemoveInput(scrolling_socket_xid);
			scrolling_socket_xid=0;
		}
		if (INVALID_SOCKET!=scrolling_socket)
		{
			close(scrolling_socket);
			scrolling_socket=INVALID_SOCKET;
		}
		if (calibration_socket_xid)
		{
			XtRemoveInput(calibration_socket_xid);
			calibration_socket_xid=0;
		}
		if (INVALID_SOCKET!=calibration_socket)
		{
			close(calibration_socket);
			calibration_socket=INVALID_SOCKET;
		}
		if (acquired_socket_xid)
		{
			XtRemoveInput(acquired_socket_xid);
			acquired_socket_xid=0;
		}
		if (INVALID_SOCKET!=acquired_socket)
		{
			close(acquired_socket);
			acquired_socket=INVALID_SOCKET;
		}
#endif /* defined (MOTIF) */
#endif /* defined (NEW_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_shutdown.  Missing crate (%p) or invalid command_socket",crate);
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave crate_shutdown %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_shutdown */

static int crate_set_scrolling_channel(struct Unemap_crate *crate,
	int channel_number)
/*******************************************************************************
LAST MODIFIED : 8 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Adds the <channel_number> to the list of channels for which scrolling
information is sent via the scrolling_callback from the <crate> (see
unemap_configure).
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(crate_set_scrolling_channel);
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_SET_SCROLLING_CHANNEL_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
		buffer_size += sizeof(channel_number);
		message_size=buffer_size-(2+(long)sizeof(message_size));
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_set_scrolling_channel.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_set_scrolling_channel.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
	"crate_set_scrolling_channel.  Missing crate (%p) or invalid command_socket",
			crate);
	}
	LEAVE;

	return (return_code);
} /* crate_set_scrolling_channel */

static int crate_clear_scrolling_channels(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Clears the list of channels for which scrolling information is sent via the
scrolling_callback by the <crate> (see unemap_configure).
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_clear_scrolling_channels);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_CLEAR_SCROLLING_CHANNELS_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		message_size=0;
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&message_size,buffer+2,sizeof(message_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_clear_scrolling_channels.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_clear_scrolling_channels.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_clear_scrolling_channels.  Missing crate (%p) or invalid command_socket",
			crate);
	}
	LEAVE;

	return (return_code);
} /* crate_clear_scrolling_channels */

static int crate_start_scrolling(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Starts scrolling messages/callbacks for the <crate>.  Also need to be sampling
to get messages/callbacks.  Allows sampling without scrolling.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_start_scrolling);
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_START_SCROLLING_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		message_size=0;
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&message_size,buffer+2,sizeof(message_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_start_scrolling.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_start_scrolling.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_start_scrolling.  Missing crate (%p) or invalid command_socket",
			crate);
	}
	LEAVE;

	return (return_code);
} /* crate_start_scrolling */

static int crate_stop_scrolling(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Stops scrolling messages/callbacks for the <crate>.  Also need to be sampling to
get messages/callbacks.  Allows sampling without scrolling.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_stop_scrolling);
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_STOP_SCROLLING_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		message_size=0;
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&message_size,buffer+2,sizeof(message_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_stop_scrolling.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_stop_scrolling.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_stop_scrolling.  Missing crate (%p) or invalid command_socket",
			crate);
	}
	LEAVE;

	return (return_code);
} /* crate_stop_scrolling */

static int crate_calibrate(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Starts calibration for the <crate>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_calibrate);
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_CALIBRATE_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		message_size=0;
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&message_size,buffer+2,sizeof(message_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_calibrate.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"crate_calibrate.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_calibrate.  Missing crate (%p) or invalid command_socket",
			crate);
	}
	LEAVE;

	return (return_code);
} /* crate_calibrate */

static int crate_start_sampling(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Starts sampling for the <crate>.
???DB.  Check if already going
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_start_sampling);
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_START_SAMPLING_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		message_size=0;
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&message_size,buffer+2,sizeof(message_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_start_sampling.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_start_sampling.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_start_sampling.  Missing crate (%p) or invalid command_socket",
			crate);
	}
	LEAVE;

	return (return_code);
} /* crate_start_sampling */

static int crate_stop_sampling(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Stops sampling for the <crate>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_stop_sampling);
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_STOP_SAMPLING_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		message_size=0;
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&message_size,buffer+2,sizeof(message_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			return_code=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_stop_sampling.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_stop_sampling.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_stop_sampling.  Missing crate (%p) or invalid command_socket",
			crate);
	}
	LEAVE;

	return (return_code);
} /* crate_stop_sampling */

static int crate_set_isolate_record_mode_start(struct Unemap_crate *crate,
	int channel_number,int isolate)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
The function does not need the hardware to be configured.

For the <crate>.  If <channel_number> is between 1 and the total number of
channels inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.  If
<channel_number> is 0 then the function applies to the group of all channels.
Otherwise, the function fails.

If <isolate> is not zero the group is put in isolate mode, otherwise the group
is put in record mode.  In isolate mode, the electrodes (recording, stimulation
and reference) are disconnected from the hardware and all channels are connected
to a calibration signal.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)];

	ENTER(crate_set_isolate_record_mode_start);
#if defined (DEBUG)
	/*???debug */
	printf("enter crate_set_isolate_record_mode_start %p\n",crate);
#endif /* defined (DEBUG) */
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_SET_ISOLATE_RECORD_MODE_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
		buffer_size += sizeof(channel_number);
		memcpy(buffer+buffer_size,&isolate,sizeof(isolate));
		buffer_size += sizeof(isolate);
		message_size=buffer_size-(2+(long)sizeof(message_size));
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_set_isolate_record_mode_start.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_set_isolate_record_mode_start.  Missing crate (%p) or invalid command_socket",
			crate);
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave crate_set_isolate_record_mode_start %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_set_isolate_record_mode_start */

static int crate_set_isolate_record_mode_end(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
The function does not need the hardware to be configured.

For the <crate>.  If <channel_number> is between 1 and the total number of
channels inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.  If
<channel_number> is 0 then the function applies to the group of all channels.
Otherwise, the function fails.

If <isolate> is not zero the group is put in isolate mode, otherwise the group
is put in record mode.  In isolate mode, the electrodes (recording, stimulation
and reference) are disconnected from the hardware and all channels are connected
to a calibration signal.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_set_isolate_record_mode_end);
#if defined (DEBUG)
	/*???debug */
	printf("enter crate_set_isolate_record_mode_end %p\n",crate);
#endif /* defined (DEBUG) */
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		/* get the header back */
		retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
				buffer_size);
#endif /* defined (DEBUG) */
			if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
			{
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_set_isolate_record_mode_end.  socket_recv() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_set_isolate_record_mode_end.  Missing crate (%p) or invalid command_socket",
			crate);
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave crate_set_isolate_record_mode_end %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_set_isolate_record_mode_end */

#if defined (OLD_CODE)
static int crate_set_isolate_record_mode(struct Unemap_crate *crate,
	int channel_number,int isolate)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

For the <crate>.  If <channel_number> is between 1 and the total number of
channels inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.  If
<channel_number> is 0 then the function applies to the group of all channels.
Otherwise, the function fails.

If <isolate> is not zero the group is put in isolate mode, otherwise the group
is put in record mode.  In isolate mode, the electrodes (recording, stimulation
and reference) are disconnected from the hardware and all channels are connected
to a calibration signal.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)];

	ENTER(crate_set_isolate_record_mode);
#if defined (DEBUG)
	/*???debug */
	printf("enter crate_set_isolate_record_mode %p\n",crate);
#endif /* defined (DEBUG) */
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_SET_ISOLATE_RECORD_MODE_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
		buffer_size += sizeof(channel_number);
		memcpy(buffer+buffer_size,&isolate,sizeof(isolate));
		buffer_size += sizeof(isolate);
		message_size=buffer_size-(2+(long)sizeof(message_size));
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_set_isolate_record_mode.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_set_isolate_record_mode.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_set_isolate_record_mode.  Missing crate (%p) or invalid command_socket",
			crate);
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave crate_set_isolate_record_mode %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_set_isolate_record_mode */
#endif /* defined (OLD_CODE) */

static int crate_get_isolate_record_mode(struct Unemap_crate *crate,
	int channel_number,int *isolate)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

For the <crate>.  If <channel_number> is between 1 and the total number of
channels inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.
Otherwise, the function fails.

If the group is in isolate mode, then <*isolate> is set to 1.  Otherwise
<*isolate> is set to 0.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(crate_get_isolate_record_mode);
	return_code=0;
	/* check arguments */
	if (crate&&isolate)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_GET_ISOLATE_RECORD_MODE_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&(sizeof(int)==buffer_size)&&(buffer[0]))
					{
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,buffer,
							sizeof(int),0))
						{
							memcpy(isolate,buffer,sizeof(int));
#if defined (DEBUG)
							display_message(INFORMATION_MESSAGE," %d",*isolate);
#endif /* defined (DEBUG) */
							return_code=1;
						}
					}
#if defined (DEBUG)
					display_message(INFORMATION_MESSAGE,"\n");
#endif /* defined (DEBUG) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_get_isolate_record_mode.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_isolate_record_mode.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_isolate_record_mode.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_get_isolate_record_mode.  Missing crate (%p) or isolate (%p)",
			crate,isolate);
	}
	LEAVE;

	return (return_code);
} /* crate_get_isolate_record_mode */

static int crate_set_antialiasing_filter_frequency(struct Unemap_crate *crate,
	int channel_number,float frequency)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

For the <crate>.  If <channel_number> is between 1 and the total number of
channels inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.  If
<channel_number> is 0 then the function applies to the group of all channels.
Otherwise, the function fails.

Sets the anti-aliasing filter to the specified <frequency> for all channels.
3dB cutoff frequency = 50*tap
begin OLD_CODE
3dB cutoff frequency = 1.07/(100*R*C)
C = 10 nF
R = (99-tap) * 101 ohm
dnd OLD_CODE
<frequency> is a request and the actual value used can be found with
unemap_get_antialiasing_filter.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)+sizeof(float)];

	ENTER(crate_set_antialiasing_filter_frequency);
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_SET_ANTIALIASING_FILTER_FREQUENCY_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
		buffer_size += sizeof(channel_number);
		memcpy(buffer+buffer_size,&frequency,sizeof(frequency));
		buffer_size += sizeof(frequency);
		message_size=buffer_size-(2+(long)sizeof(message_size));
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_set_antialiasing_filter_frequency.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_set_antialiasing_filter_frequency.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_set_antialiasing_filter_frequency.  Missing crate (%p) or invalid command_socket",
			crate);
	}
	LEAVE;

	return (return_code);
} /* crate_set_antialiasing_filter_frequency */

static int crate_set_powerup_antialiasing_filter_frequency(
	struct Unemap_crate *crate,int channel_number)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

For the <crate>.  If <channel_number> is between 1 and the total number of
channels inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.  If
<channel_number> is 0 then the function applies to the group of all channels.
Otherwise, the function fails.

Makes the current anti-aliasing filter frequency the power up value for the
group.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(crate_set_powerup_antialiasing_filter_frequency);
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_SET_POWERUP_ANTIALIASING_FILTER_FREQUENCY_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
		buffer_size += sizeof(channel_number);
		message_size=buffer_size-(2+(long)sizeof(message_size));
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
			"crate_set_powerup_antialiasing_filter_frequency.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
			"crate_set_powerup_antialiasing_filter_frequency.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_set_powerup_antialiasing_filter_frequency.  Missing crate (%p) or invalid command_socket",
			crate);
	}
	LEAVE;

	return (return_code);
} /* crate_set_powerup_antialiasing_filter_frequency */

static int crate_get_antialiasing_filter_frequency(struct Unemap_crate *crate,
	int channel_number,float *frequency)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

For the <crate>.  If <channel_number> is between 1 and the total number of
channels inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.
Otherwise, the function fails.

<*frequency> is set to the frequency for the anti-aliasing filter.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(float)];

	ENTER(crate_get_antialiasing_filter_frequency);
	return_code=0;
	/* check arguments */
	if (crate&&frequency)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_GET_ANTIALIASING_FILTER_FREQUENCY_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&(sizeof(float)==buffer_size)&&
						(buffer[0]))
					{
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,buffer,
							sizeof(float),0))
						{
							memcpy(frequency,buffer,sizeof(float));
#if defined (DEBUG)
							display_message(INFORMATION_MESSAGE," %g",*frequency);
#endif /* defined (DEBUG) */
							return_code=1;
						}
					}
#if defined (DEBUG)
					display_message(INFORMATION_MESSAGE,"\n");
#endif /* defined (DEBUG) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_get_antialiasing_filter_frequency.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_antialiasing_filter_frequency.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_antialiasing_filter_frequency.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_get_antialiasing_filter_frequency.  Missing crate (%p) or frequency (%p)",
			crate,frequency);
	}
	LEAVE;

	return (return_code);
} /* crate_get_antialiasing_filter_frequency */

static int crate_get_number_of_channels(struct Unemap_crate *crate,
	int *number_of_channels)
/*******************************************************************************
LAST MODIFIED : 6 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

The total number of hardware channels for the <crate> is assigned to
<*number_of_channels>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_get_number_of_channels);
	return_code=0;
	/* check arguments */
	if (crate&&number_of_channels)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_GET_NUMBER_OF_CHANNELS_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			message_size=0;
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&(sizeof(int)==buffer_size)&&
						(buffer[0]))
					{
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,buffer,
							sizeof(int),0))
						{
							memcpy(number_of_channels,buffer,sizeof(int));
#if defined (DEBUG)
							/*???debug */
							display_message(INFORMATION_MESSAGE," %d",*number_of_channels);
#endif /* defined (DEBUG) */
							return_code=1;
						}
					}
#if defined (DEBUG)
					display_message(INFORMATION_MESSAGE,"\n");
#endif /* defined (DEBUG) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_get_number_of_channels.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_number_of_channels.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_number_of_channels.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_get_number_of_channels.  Missing crate (%p) or number_of_channels (%p)",
			crate,number_of_channels);
	}
	LEAVE;

	return (return_code);
} /* crate_get_number_of_channels */

static int crate_get_sample_range(struct Unemap_crate *crate,
	long int *minimum_sample_value,long int *maximum_sample_value)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

For the <crate>.  The minimum possible sample value is assigned to
<*minimum_sample_value> and the maximum possible sample value is assigned to
<*maximum_sample_value>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+2*sizeof(long)];

	ENTER(crate_get_sample_range);
	return_code=0;
	/* check arguments */
	if (crate&&minimum_sample_value&&maximum_sample_value)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_GET_SAMPLE_RANGE_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			message_size=0;
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&(2*sizeof(long)==buffer_size)&&
						(buffer[0]))
					{
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,buffer,
							2*sizeof(long),0))
						{
							memcpy(minimum_sample_value,buffer,sizeof(long));
							memcpy(maximum_sample_value,buffer+sizeof(long),sizeof(long));
#if defined (DEBUG)
							/*???debug */
							display_message(INFORMATION_MESSAGE," %ld %ld",
								*minimum_sample_value,*maximum_sample_value);
#endif /* defined (DEBUG) */
							return_code=1;
						}
					}
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,"\n");
#endif /* defined (DEBUG) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_get_sample_range.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_sample_range.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_sample_range.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_get_sample_range.  Missing crate (%p) or minimum_sample_value (%p) or maximum_sample_value (%p)",
			crate,minimum_sample_value,maximum_sample_value);
	}
	LEAVE;

	return (return_code);
} /* crate_get_sample_range */

static int crate_get_voltage_range_start(struct Unemap_crate *crate,
	int channel_number)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels for the
<crate> inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.
Otherwise, the function fails.

The voltage range, allowing for gain, is returned via <*minimum_voltage> and
<*maximum_voltage>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(crate_get_voltage_range_start);
	return_code=0;
	/* check arguments */
	if (crate)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_GET_VOLTAGE_RANGE_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_voltage_range_start.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_voltage_range_start.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_get_voltage_range_start.  Missing crate");
	}
	LEAVE;

	return (return_code);
} /* crate_get_voltage_range_start */

static int crate_get_voltage_range_end(struct Unemap_crate *crate,
	float *minimum_voltage,float *maximum_voltage)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels for the
<crate> inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.
Otherwise, the function fails.

The voltage range, allowing for gain, is returned via <*minimum_voltage> and
<*maximum_voltage>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(float)];

	ENTER(crate_get_voltage_range_end);
	return_code=0;
	/* check arguments */
	if (crate&&minimum_voltage&&maximum_voltage)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(2*sizeof(float)==buffer_size)&&
					(buffer[0]))
				{
					if (SOCKET_ERROR!=socket_recv(crate->command_socket,buffer,
						2*sizeof(float),0))
					{
						memcpy(minimum_voltage,buffer,sizeof(float));
						memcpy(maximum_voltage,buffer+sizeof(float),sizeof(float));
#if defined (DEBUG)
						/*???debug */
						display_message(INFORMATION_MESSAGE," %g %g",*minimum_voltage,
							*maximum_voltage);
#endif /* defined (DEBUG) */
						return_code=1;
					}
				}
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,"\n");
#endif /* defined (DEBUG) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_voltage_range_end.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_voltage_range_end.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_get_voltage_range_end.  Missing crate (%p) or minimum_voltage (%p) or maximum_voltage (%p)",
			crate,minimum_voltage,maximum_voltage);
	}
	LEAVE;

	return (return_code);
} /* crate_get_voltage_range_end */

#if defined (OLD_CODE)
static int crate_get_voltage_range(struct Unemap_crate *crate,
	int channel_number,float *minimum_voltage,float *maximum_voltage)
/*******************************************************************************
LAST MODIFIED : 6 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels for the
<crate> inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.
Otherwise, the function fails.

The voltage range, allowing for gain, is returned via <*minimum_voltage> and
<*maximum_voltage>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(float)];

	ENTER(crate_get_voltage_range);
	return_code=0;
	/* check arguments */
	if (crate&&minimum_voltage&&maximum_voltage)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_GET_VOLTAGE_RANGE_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&(2*sizeof(float)==buffer_size)&&
						(buffer[0]))
					{
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,buffer,
							2*sizeof(float),0))
						{
							memcpy(minimum_voltage,buffer,sizeof(float));
							memcpy(maximum_voltage,buffer+sizeof(float),sizeof(float));
#if defined (DEBUG)
							/*???debug */
							display_message(INFORMATION_MESSAGE," %g %g",*minimum_voltage,
								*maximum_voltage);
#endif /* defined (DEBUG) */
							return_code=1;
						}
					}
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,"\n");
#endif /* defined (DEBUG) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_get_voltage_range.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_voltage_range.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_voltage_range.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_get_voltage_range.  Missing crate (%p) or minimum_voltage (%p) or maximum_voltage (%p)",
			crate,minimum_voltage,maximum_voltage);
	}
	LEAVE;

	return (return_code);
} /* crate_get_voltage_range */
#endif /* defined (OLD_CODE) */

static int crate_get_number_of_samples_acquired(struct Unemap_crate *crate,
	unsigned long *number_of_samples)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

For the <crate>.  The number of samples acquired per channel since
<unemap_start_sampling> is assigned to <*number_of_samples>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_get_number_of_samples_acquired);
	return_code=0;
	/* check arguments */
	if (crate&&number_of_samples)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_GET_NUMBER_OF_SAMPLES_ACQUIRED_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			message_size=0;
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&(sizeof(long)==buffer_size)&&
						(buffer[0]))
					{
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,buffer,
							sizeof(long),0))
						{
							memcpy(number_of_samples,buffer,sizeof(long));
#if defined (DEBUG)
							display_message(INFORMATION_MESSAGE," %ld",*number_of_samples);
#endif /* defined (DEBUG) */
							return_code=1;
						}
					}
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,"\n");
#endif /* defined (DEBUG) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_get_number_of_samples_acquired.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_number_of_samples_acquired.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_number_of_samples_acquired.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_get_number_of_samples_acquired.  Missing crate (%p) or number_of_samples (%p)",
			crate,number_of_samples);
	}
	LEAVE;

	return (return_code);
} /* crate_get_number_of_samples_acquired */

static int crate_get_samples_acquired(struct Unemap_crate *crate,
	int channel_number,short int *samples)
/*******************************************************************************
LAST MODIFIED : 12 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

For the <crate>.  If <channel_number> is valid (between 1 and the total number
of channels inclusive), then the <samples> for that channel are returned.  If
<channel_number> is 0 then the <samples> for all channels are returned.
Otherwise the function fails.
==============================================================================*/
{
	int return_code,retval;
	long block_size,buffer_size,message_size;
	short int *crate_samples;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(crate_get_samples_acquired);
#if defined (DEBUG)
	/*???debug */
	printf("enter crate_get_samples_acquired %p %d %p\n",crate,channel_number,
		samples);
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if (crate&&samples)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_GET_SAMPLES_ACQUIRED_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&(0<buffer_size)&&(buffer[0]))
					{
						if ((0==channel_number)&&(0<module_number_of_channels))
						{
							/* have to stagger to allow room for other crates */
							crate_samples=samples;
							block_size=(crate->number_of_channels)*(long)sizeof(short int);
							while ((buffer_size>0)&&(SOCKET_ERROR!=socket_recv(
								crate->command_socket,(unsigned char *)crate_samples,block_size,
								0)))
							{
								crate_samples += module_number_of_channels;
								buffer_size -= block_size;
							}
							if (0==buffer_size)
							{
								return_code=1;
							}
						}
						else
						{
							if (SOCKET_ERROR!=socket_recv(crate->command_socket,
								(unsigned char *)samples,buffer_size,0))
							{
								return_code=1;
							}
						}
					}
#if defined (DEBUG)
					display_message(INFORMATION_MESSAGE,"\n");
#endif /* defined (DEBUG) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_get_samples_acquired.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_samples_acquired.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_samples_acquired.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_get_samples_acquired.  Missing crate (%p) or samples (%p)",crate,
			samples);
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave crate_get_samples_acquired %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_get_samples_acquired */

static int crate_get_samples_acquired_background_start(
	struct Unemap_crate *crate,int channel_number)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
The function fails if the hardware is not configured.

For the <crate>.  If <channel_number> is valid (between 1 and the total number
of channels inclusive), then the <samples> for that channel are returned.  If
<channel_number> is 0 then the <samples> for all channels are returned.
Otherwise the function fails.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(crate_get_samples_acquired_background_start);
#if defined (DEBUG)
	/*???debug */
	printf("enter crate_get_samples_acquired_background_start %p %d\n",crate,
		channel_number);
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_GET_SAMPLES_ACQUIRED_BACKGROUND_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
		buffer_size += sizeof(channel_number);
		message_size=buffer_size-(2+(long)sizeof(message_size));
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_samples_acquired_background_start.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_get_samples_acquired_background_start.  Missing crate (%p) or invalid command_socket",
			crate);
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave crate_get_samples_acquired_background_start %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_get_samples_acquired_background_start */

static int crate_get_samples_acquired_background_end(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
The function fails if the hardware is not configured.

For the <crate>.  If <channel_number> is valid (between 1 and the total number
of channels inclusive), then the <samples> for that channel are returned.  If
<channel_number> is 0 then the <samples> for all channels are returned.
Otherwise the function fails.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_get_samples_acquired_background_end);
#if defined (DEBUG)
	/*???debug */
	printf("enter crate_get_samples_acquired_background_end %p %d\n",crate,
		channel_number);
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		/* get the header back */
		retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
				buffer_size);
#endif /* defined (DEBUG) */
			if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
			{
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_samples_acquired_background_end.  socket_recv() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_get_samples_acquired_background_end.  Missing crate (%p) or invalid command_socket",
			crate);
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave crate_get_samples_acquired_background_end %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_get_samples_acquired_background_end */

#if defined (OLD_CODE)
static int crate_get_samples_acquired_background(struct Unemap_crate *crate,
	int channel_number)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
The function fails if the hardware is not configured.

For the <crate>.  If <channel_number> is valid (between 1 and the total number
of channels inclusive), then the <samples> for that channel are returned.  If
<channel_number> is 0 then the <samples> for all channels are returned.
Otherwise the function fails.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(crate_get_samples_acquired_background);
#if defined (DEBUG)
	/*???debug */
	printf("enter crate_get_samples_acquired_background %p %d\n",crate,
		channel_number);
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_GET_SAMPLES_ACQUIRED_BACKGROUND_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
		buffer_size += sizeof(channel_number);
		message_size=buffer_size-(2+(long)sizeof(message_size));
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_samples_acquired_background.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_samples_acquired_background.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_get_samples_acquired_background.  Missing crate (%p) or invalid command_socket",
			crate);
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave crate_get_samples_acquired_background %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_get_samples_acquired_background */
#endif /* defined (OLD_CODE) */

static int crate_get_maximum_number_of_samples(struct Unemap_crate *crate,
	unsigned long *number_of_samples)
/*******************************************************************************
LAST MODIFIED : 6 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

The size of the rolling buffer for the <crate>, in number of samples per
channel, is assigned to <*number_of_samples>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_get_maximum_number_of_samples);
	return_code=0;
	/* check arguments */
	if (crate&&number_of_samples)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_GET_MAXIMUM_NUMBER_OF_SAMPLES_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			message_size=0;
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&(sizeof(long)==buffer_size)&&
						(buffer[0]))
					{
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,buffer,
							sizeof(long),0))
						{
							memcpy(number_of_samples,buffer,sizeof(long));
#if defined (DEBUG)
							display_message(INFORMATION_MESSAGE," %ld",*number_of_samples);
#endif /* defined (DEBUG) */
							return_code=1;
						}
					}
#if defined (DEBUG)
					display_message(INFORMATION_MESSAGE,"\n");
#endif /* defined (DEBUG) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_get_maximum_number_of_samples.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_maximum_number_of_samples.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_maximum_number_of_samples.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_get_maximum_number_of_samples.  Missing crate (%p) or number_of_samples (%p)",
			crate,number_of_samples);
	}
	LEAVE;

	return (return_code);
} /* crate_get_maximum_number_of_samples */

static int crate_get_sampling_frequency(struct Unemap_crate *crate,
	float *frequency)
/*******************************************************************************
LAST MODIFIED : 6 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

The sampling frequency is assigned to <*frequency>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(float)];

	ENTER(crate_get_sampling_frequency);
	return_code=0;
	/* check arguments */
	if (crate&&frequency)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_GET_SAMPLING_FREQUENCY_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			message_size=0;
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&(sizeof(float)==buffer_size)&&
						(buffer[0]))
					{
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,buffer,
							sizeof(float),0))
						{
							memcpy(frequency,buffer,sizeof(float));
#if defined (DEBUG)
							display_message(INFORMATION_MESSAGE," %g",*frequency);
#endif /* defined (DEBUG) */
							return_code=1;
						}
					}
#if defined (DEBUG)
					display_message(INFORMATION_MESSAGE,"\n");
#endif /* defined (DEBUG) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_get_sampling_frequency.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_sampling_frequency.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_sampling_frequency.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_get_sampling_frequency.  Missing crate (%p) or frequency (%p)",
			crate,frequency);
	}
	LEAVE;

	return (return_code);
} /* crate_get_sampling_frequency */

static int crate_get_gain_start(struct Unemap_crate *crate,int channel_number)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels for the
<crate> inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.
Otherwise, the function fails.

The <*pre_filter_gain> and <*post_filter_gain> for the group are assigned.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(crate_get_gain_start);
	return_code=0;
	/* check arguments */
	if (crate)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_GET_GAIN_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_gain_start.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_gain_start.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_get_gain_start.  Missing crate");
	}
	LEAVE;

	return (return_code);
} /* crate_get_gain_start */

static int crate_get_gain_end(struct Unemap_crate *crate,
	float *pre_filter_gain,float *post_filter_gain)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels for the
<crate> inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.
Otherwise, the function fails.

The <*pre_filter_gain> and <*post_filter_gain> for the group are assigned.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(float)];

	ENTER(crate_get_gain_end);
	return_code=0;
	/* check arguments */
	if (crate&&pre_filter_gain&&post_filter_gain)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(2*sizeof(float)==buffer_size)&&
					(buffer[0]))
				{
					if (SOCKET_ERROR!=socket_recv(crate->command_socket,buffer,
						2*sizeof(float),0))
					{
						memcpy(pre_filter_gain,buffer,sizeof(float));
						memcpy(post_filter_gain,buffer+sizeof(float),sizeof(float));
#if defined (DEBUG)
						/*???debug */
						display_message(INFORMATION_MESSAGE," %g %g",*pre_filter_gain,
							*post_filter_gain);
#endif /* defined (DEBUG) */
						return_code=1;
					}
				}
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,"\n");
#endif /* defined (DEBUG) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_gain_end.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_gain_end.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_get_gain_end.  Missing crate (%p) or pre_filter_gain (%p) or post_filter_gain (%p)",
			crate,pre_filter_gain,post_filter_gain);
	}
	LEAVE;

	return (return_code);
} /* crate_get_gain_end */

#if defined (OLD_CODE)
static int crate_get_gain(struct Unemap_crate *crate,int channel_number,
	float *pre_filter_gain,float *post_filter_gain)
/*******************************************************************************
LAST MODIFIED : 6 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels for the
<crate> inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.
Otherwise, the function fails.

The <*pre_filter_gain> and <*post_filter_gain> for the group are assigned.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(float)];

	ENTER(crate_get_gain);
	return_code=0;
	/* check arguments */
	if (crate&&pre_filter_gain&&post_filter_gain)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_GET_GAIN_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&(2*sizeof(float)==buffer_size)&&
						(buffer[0]))
					{
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,buffer,
							2*sizeof(float),0))
						{
							memcpy(pre_filter_gain,buffer,sizeof(float));
							memcpy(post_filter_gain,buffer+sizeof(float),sizeof(float));
#if defined (DEBUG)
							/*???debug */
							display_message(INFORMATION_MESSAGE," %g %g",*pre_filter_gain,
								*post_filter_gain);
#endif /* defined (DEBUG) */
							return_code=1;
						}
					}
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,"\n");
#endif /* defined (DEBUG) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_get_gain.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_gain.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"crate_get_gain.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_get_gain.  Missing crate (%p) or pre_filter_gain (%p) or post_filter_gain (%p)",
			crate,pre_filter_gain,post_filter_gain);
	}
	LEAVE;

	return (return_code);
} /* crate_get_gain */
#endif /* defined (OLD_CODE) */

static int crate_set_gain(struct Unemap_crate *crate,int channel_number,
	float pre_filter_gain,float post_filter_gain)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

For the <crate>.  If <channel_number> is between 1 and the total number of
channels inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.  If
<channel_number> is 0 then the function applies to the group of all channels.
Otherwise, the function fails.

Sets the gain before the band pass filter and the gain after the band pass
filter to the specified values.

For UNEMAP_1V1, there is no gain before the filter (<pre_filter_gain> ignored).
For UNEMAP_2V1 and UNEMAP_2V2, the gain before the filter can be 1, 2, 4 or 8.

For UNEMAP_1V1, the post filter gain can be 10, 20, 50, 100, 200, 500 or 1000
(fixed gain of 10)
For UNEMAP_2V1 and UNEMAP_2V2, the post filter gain can be 11, 22, 55, 110, 220,
550 or 1100 (fixed gain of 11).
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
#if defined (CACHE_CLIENT_INFORMATION)
	int i;
	struct Unemap_card *unemap_card;
#endif /* defined (CACHE_CLIENT_INFORMATION) */
	unsigned char buffer[2+sizeof(long)+sizeof(int)+2*sizeof(float)];

	ENTER(crate_set_gain);
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_SET_GAIN_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
		buffer_size += sizeof(channel_number);
		memcpy(buffer+buffer_size,&pre_filter_gain,sizeof(pre_filter_gain));
		buffer_size += sizeof(pre_filter_gain);
		memcpy(buffer+buffer_size,&post_filter_gain,sizeof(post_filter_gain));
		buffer_size += sizeof(post_filter_gain);
		message_size=buffer_size-(2+(long)sizeof(message_size));
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
#if defined (CACHE_CLIENT_INFORMATION)
					if (crate->unemap_cards)
					{
						module_force_connection=1;
						if (0==channel_number)
						{
							unemap_card=crate->unemap_cards;
							for (i=crate->number_of_unemap_cards*
								NUMBER_OF_CHANNELS_ON_NI_CARD;i>0;
								i -= NUMBER_OF_CHANNELS_ON_NI_CARD)
							{
								crate_get_gain_start(crate,i);
								crate_get_gain_end(crate,&(unemap_card->pre_filter_gain),
									&(unemap_card->post_filter_gain));
#if defined (OLD_CODE)
								crate_get_gain(crate,i,&(unemap_card->pre_filter_gain),
									&(unemap_card->post_filter_gain));
#endif /* defined (OLD_CODE) */
							}
						}
						else
						{
							unemap_card=(crate->unemap_cards)+
								((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
							crate_get_gain_start(crate,channel_number);
							crate_get_gain_end(crate,&(unemap_card->pre_filter_gain),
								&(unemap_card->post_filter_gain));
#if defined (OLD_CODE)
							crate_get_gain(crate,channel_number,
								&(unemap_card->pre_filter_gain),
								&(unemap_card->post_filter_gain));
#endif /* defined (OLD_CODE) */
						}
						module_force_connection=0;
					}
#endif /* defined (CACHE_CLIENT_INFORMATION) */
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"crate_set_gain.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"crate_set_gain.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_set_gain.  Missing crate (%p) or invalid command_socket");
	}
	LEAVE;

	return (return_code);
} /* crate_set_gain */

static int crate_load_voltage_stimulating(struct Unemap_crate *crate,
	int number_of_channels,int *channel_numbers,int number_of_voltages,
	float voltages_per_second,float *voltages)
/*******************************************************************************
LAST MODIFIED : 2 July 2000

DESCRIPTION :
The function fails if the hardware is not configured.

for the <crate>.  If <number_of_channels> is greater than 0 then the function
applies to the group that is the union of ((channel_number-1) div 64)*64+1 to
((channel_number-1) div 64)*64+64 for all the <channel_numbers>.  If
<number_of_channels> is 0 then the function applies to the group of all
channels.  Otherwise, the function fails.

Sets all the stimulating channels in the group to voltage stimulating and loads
the stimulating signal.  If <number_of_voltages> is 0 then the stimulating
signal is the zero signal.  If <number_of_voltages> is 1 then the stimulating
signal is constant at the <*voltages>.  If <number_of_voltages> is >1 then the
stimulating signal is that in <voltages> at the specified number of
<voltages_per_second>.

The <voltages> are those desired (in volts).  The function sets <voltages> to
the actual values used.

Use unemap_set_channel_stimulating to make a channel into a stimulating channel.
Use <unemap_start_stimulating> to start the stimulating.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)+sizeof(float)];

	ENTER(crate_load_voltage_stimulating);
	return_code=0;
	if (crate&&((0==number_of_channels)||((0<number_of_channels)&&
		channel_numbers))&&((0==number_of_voltages)||((0<number_of_voltages)&&
		voltages)))
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_LOAD_VOLTAGE_STIMULATING_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&number_of_channels,sizeof(number_of_channels));
			buffer_size += sizeof(number_of_channels);
			memcpy(buffer+buffer_size,&number_of_voltages,sizeof(number_of_voltages));
			buffer_size += sizeof(number_of_voltages);
			memcpy(buffer+buffer_size,&voltages_per_second,
				sizeof(voltages_per_second));
			buffer_size += sizeof(voltages_per_second);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			message_size += sizeof(int)*number_of_channels;
			message_size += sizeof(float)*number_of_voltages;
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
				if (0<number_of_channels)
				{
					retval=socket_send(crate->command_socket,
						(unsigned char *)channel_numbers,sizeof(int)*number_of_channels,0);
				}
				if (SOCKET_ERROR!=retval)
				{
					if (0<number_of_voltages)
					{
						retval=socket_send(crate->command_socket,(unsigned char *)voltages,
							sizeof(float)*number_of_voltages,0);
					}
				}
			}
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&
						(sizeof(float)*number_of_voltages==buffer_size)&&(buffer[0]))
					{
						if (0<buffer_size)
						{
							retval=socket_recv(crate->command_socket,
								(unsigned char *)voltages,buffer_size,0);
							if (SOCKET_ERROR!=retval)
							{
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
						"crate_load_voltage_stimulating.  socket_recv() voltages failed");
							}
						}
						else
						{
							return_code=1;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_load_voltage_stimulating.  socket_recv() header failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_load_voltage_stimulating.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_load_voltage_stimulating.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_load_voltage_stimulating.  Missing crate (%p) or invalid number_of_channels (%d) or channel_numbers (%p) or number_of_voltages (%d) or voltages (%p)",
			crate,number_of_channels,channel_numbers,number_of_voltages,voltages);
	}
	LEAVE;

	return (return_code);
} /* crate_load_voltage_stimulating */

static int crate_load_current_stimulating(struct Unemap_crate *crate,
	int number_of_channels,int *channel_numbers,int number_of_currents,
	float currents_per_second,float *currents)
/*******************************************************************************
LAST MODIFIED : 2 July 2000

DESCRIPTION :
The function fails if the hardware is not configured.

For the <crate>.  If <number_of_channels> is greater than 0 then the function
applies to the group that is the union of ((channel_number-1) div 64)*64+1 to
((channel_number-1) div 64)*64+64 for all the <channel_numbers>.  If
<number_of_channels> is 0 then the function applies to the group of all
channels.  Otherwise, the function fails.

Sets all the stimulating channels in the group to current stimulating and loads
the stimulating signal.  If <number_of_currents> is 0 then the stimulating
signal is the zero signal.  If <number_of_currents> is 1 then the stimulating
signal is constant at the <*currents>.  If <number_of_currents> is >1 then the
stimulating signal is that in <currents> at the specified number of
<currents_per_second>.

The <currents> are those desired as a proportion of the maximum (dependent on
the impedance being driven).  The function sets <currents> to the actual values
used.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)+sizeof(float)];

	ENTER(crate_load_current_stimulating);
	return_code=0;
	if (crate&&((0==number_of_channels)||((0<number_of_channels)&&
		channel_numbers))&&((0==number_of_currents)||((0<number_of_currents)&&
		currents)))
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_LOAD_CURRENT_STIMULATING_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&number_of_channels,sizeof(number_of_channels));
			buffer_size += sizeof(number_of_channels);
			memcpy(buffer+buffer_size,&number_of_currents,sizeof(number_of_currents));
			buffer_size += sizeof(number_of_currents);
			memcpy(buffer+buffer_size,&currents_per_second,
				sizeof(currents_per_second));
			buffer_size += sizeof(currents_per_second);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			message_size += sizeof(int)*number_of_channels;
			message_size += sizeof(float)*number_of_currents;
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
				if (0<number_of_channels)
				{
					retval=socket_send(crate->command_socket,
						(unsigned char *)channel_numbers,sizeof(int)*number_of_channels,0);
				}
				if (SOCKET_ERROR!=retval)
				{
					if (0<number_of_currents)
					{
						retval=socket_send(crate->command_socket,(unsigned char *)currents,
							sizeof(float)*number_of_currents,0);
					}
				}
			}
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&
						(sizeof(float)*number_of_currents==buffer_size)&&(buffer[0]))
					{
						if (0<buffer_size)
						{
							retval=socket_recv(crate->command_socket,
								(unsigned char *)currents,buffer_size,0);
							if (SOCKET_ERROR!=retval)
							{
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
						"crate_load_current_stimulating.  socket_recv() currents failed");
							}
						}
						else
						{
							return_code=1;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_load_current_stimulating.  socket_recv() header failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_load_current_stimulating.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_load_current_stimulating.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_load_current_stimulating.  Missing crate (%p) or invalid number_of_channels (%d) or channel_numbers (%p) or number_of_currents (%d) or currents (%p)",
			crate,number_of_channels,channel_numbers,number_of_currents,currents);
	}
	LEAVE;

	return (return_code);
} /* crate_load_current_stimulating */

static int crate_start_stimulating(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 2 July 2000

DESCRIPTION :
The function fails if the hardware is not configured.

For the <crate>.  Starts stimulation for all channels that have been loaded
(with unemap_load_voltage_stimulating or unemap_load_current_stimulating) and
have not yet started.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_start_stimulating);
	return_code=0;
	if (crate)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_START_STIMULATING_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			message_size=0;
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&message_size,buffer+2,sizeof(message_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
					{
						return_code=1;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_start_stimulating.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_start_stimulating.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_start_stimulating.  Missing command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_start_stimulating.  Missing crate");
	}
	LEAVE;

	return (return_code);
} /* crate_start_stimulating */

#if defined (OLD_CODE)
static int crate_start_voltage_stimulating(struct Unemap_crate *crate,
	int channel_number,int number_of_voltages,float voltages_per_second,
	float *voltages)
/*******************************************************************************
LAST MODIFIED : 13 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

For the <crate>.  If <channel_number> is between 1 and the total number of
channels inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.  If
<channel_number> is 0 then the function applies to the group of all channels.
Otherwise, the function fails.

Starts stimulating down the channels that are in the group and have been
specified as stimulating with <unemap_set_channel_stimulating>.  If
<number_of_voltages> is 0 then the stimulating signal is the zero signal.  If
<number_of_voltages> is 1 then the stimulating signal is constant at the
<*voltages>.  If <number_of_voltages> is >1 then the stimulating signal is that
in <voltages> at the specified number of <voltages_per_second>.

The <voltages> are those desired (in volts).  The function sets <voltages> to
the actual values used.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)+sizeof(float)];

	ENTER(crate_start_voltage_stimulating);
	return_code=0;
	if (crate&&((0==number_of_voltages)||((0<number_of_voltages)&&voltages)))
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_START_VOLTAGE_STIMULATING_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			memcpy(buffer+buffer_size,&number_of_voltages,sizeof(number_of_voltages));
			buffer_size += sizeof(number_of_voltages);
			memcpy(buffer+buffer_size,&voltages_per_second,
				sizeof(voltages_per_second));
			buffer_size += sizeof(voltages_per_second);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			message_size += sizeof(float)*number_of_voltages;
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
				if (0<number_of_voltages)
				{
					retval=socket_send(crate->command_socket,(unsigned char *)voltages,
						sizeof(float)*number_of_voltages,0);
				}
			}
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&
						((long)sizeof(float)*number_of_voltages==buffer_size)&&(buffer[0]))
					{
						if (0<buffer_size)
						{
							retval=socket_recv(crate->command_socket,
								(unsigned char *)voltages,buffer_size,0);
							if (SOCKET_ERROR!=retval)
							{
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
						"crate_start_voltage_stimulating.  socket_recv() voltages failed");
							}
						}
						else
						{
							return_code=1;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_start_voltage_stimulating.  socket_recv() header failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_start_voltage_stimulating.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_start_voltage_stimulating.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_start_voltage_stimulating.  Missing crate (%p) or invalid number_of_voltages (%d) or missing voltages (%p)",
			crate,number_of_voltages,voltages);
	}
	LEAVE;

	return (return_code);
} /* crate_start_voltage_stimulating */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static int crate_start_current_stimulating(struct Unemap_crate *crate,
	int channel_number,int number_of_currents,float currents_per_second,
	float *currents)
/*******************************************************************************
LAST MODIFIED : 13 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

For the <crate>.  If <channel_number> is between 1 and the total number of
channels inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.  If
<channel_number> is 0 then the function applies to the group of all channels.
Otherwise, the function fails.

Starts stimulating down the channels that are in the group and have been
specified as stimulating with <unemap_set_channel_stimulating>.  If
<number_of_currents> is 0 then the stimulating signal is the zero signal.  If
<number_of_currents> is 1 then the stimulating signal is constant at the
<*currents>.  If <number_of_currents> is >1 then the stimulating signal is that
in <currents> at the specified number of <currents_per_second>.

The <currents> are those desired as a proportion of the maximum (dependent on
the impedance being driven).  The function sets <currents> to the actual values
used.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)+sizeof(float)];

	ENTER(crate_start_current_stimulating);
	return_code=0;
	if (crate&&((0==number_of_currents)||((0<number_of_currents)&&currents)))
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_START_CURRENT_STIMULATING_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			memcpy(buffer+buffer_size,&number_of_currents,sizeof(number_of_currents));
			buffer_size += sizeof(number_of_currents);
			memcpy(buffer+buffer_size,&currents_per_second,
				sizeof(currents_per_second));
			buffer_size += sizeof(currents_per_second);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			message_size += sizeof(float)*number_of_currents;
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
				if (0<number_of_currents)
				{
					retval=socket_send(crate->command_socket,(unsigned char *)currents,
						sizeof(float)*number_of_currents,0);
				}
			}
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&
						((long)sizeof(float)*number_of_currents==buffer_size)&&(buffer[0]))
					{
						if (0<buffer_size)
						{
							retval=socket_recv(crate->command_socket,
								(unsigned char *)currents,buffer_size,0);
							if (SOCKET_ERROR!=retval)
							{
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
						"crate_start_current_stimulating.  socket_recv() currents failed");
							}
						}
						else
						{
							return_code=1;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_start_current_stimulating.  socket_recv() header failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_start_current_stimulating.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_start_current_stimulating.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_start_current_stimulating.  Missing crate (%p) or invalid number_of_currents (%d) or currents (%p)",
			crate,number_of_currents,currents);
	}
	LEAVE;

	return (return_code);
} /* crate_start_current_stimulating */
#endif /* defined (OLD_CODE) */

static int crate_stop_stimulating(struct Unemap_crate *crate,int channel_number)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

For the <crate>.  If <channel_number> is between 1 and the total number of
channels inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.  If
<channel_number> is 0 then the function applies to the group of all channels.
Otherwise, the function fails.

Stops stimulating for the channels in the group.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(crate_stop_stimulating);
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_STOP_STIMULATING_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
		buffer_size += sizeof(channel_number);
		message_size=buffer_size-(2+(long)sizeof(message_size));
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_stop_stimulating.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_stop_stimulating.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_stop_stimulating.  Missing crate (%p) or invalid command_socket",
			crate);
	}
	LEAVE;

	return (return_code);
} /* crate_stop_stimulating */

static int crate_set_channel_stimulating(struct Unemap_crate *crate,
	int channel_number,int stimulating)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

For the <crate>.  Zero <stimulating> means off.  Non-zero <stimulating> means
on.  If <channel_number> is valid (between 1 and the total number of channels
inclusive), then <channel_number> is set to <stimulating>.  If <channel_number>
is 0, then all channels are set to <stimulating>.  Otherwise the function fails.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)];

	ENTER(crate_set_channel_stimulating);
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_SET_CHANNEL_STIMULATING_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
		buffer_size += sizeof(channel_number);
		memcpy(buffer+buffer_size,&stimulating,sizeof(stimulating));
		buffer_size += sizeof(stimulating);
		message_size=buffer_size-(2+(long)sizeof(message_size));
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_set_channel_stimulating.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_set_channel_stimulating.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_set_channel_stimulating.  Missing crate (%p) or invalid command_socket",
			crate);
	}
	LEAVE;

	return (return_code);
} /* crate_set_channel_stimulating */

static int crate_get_channel_stimulating(struct Unemap_crate *crate,
	int channel_number,int *stimulating)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

For the <crate>.  If <channel_number> is valid (between 1 and the total number
of channels inclusive), then <stimulating> is set to 1 if <channel_number> is
stimulating and 0 otherwise.  Otherwise the function fails.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(crate_get_channel_stimulating);
	return_code=0;
	/* check arguments */
	if (crate&&stimulating)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_GET_CHANNEL_STIMULATING_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&(sizeof(int)==buffer_size)&&(buffer[0]))
					{
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,buffer,
							sizeof(int),0))
						{
							memcpy(stimulating,buffer,sizeof(int));
#if defined (DEBUG)
							display_message(INFORMATION_MESSAGE," %d",*stimulating);
#endif /* defined (DEBUG) */
							return_code=1;
						}
					}
#if defined (DEBUG)
					display_message(INFORMATION_MESSAGE,"\n");
#endif /* defined (DEBUG) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_get_channel_stimulating.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_channel_stimulating.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_channel_stimulating.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_get_channel_stimulating.  Missing crate (%p) or stimulating (%p)",
			crate,stimulating);
	}
	LEAVE;

	return (return_code);
} /* crate_get_channel_stimulating */

static int crate_start_calibrating(struct Unemap_crate *crate,
	int channel_number,int number_of_voltages,float voltages_per_second,
	float *voltages)
/*******************************************************************************
LAST MODIFIED : 13 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

For the <crate>.  If <channel_number> is between 1 and the total number of
channels inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.  If
<channel_number> is 0 then the function applies to the group of all channels.
Otherwise, the function fails.

Starts generating the calibration signal for the specified group.  If
<number_of_voltages> is 0 then the calibration signal is the zero signal.  If
<number_of_voltages> is 1 then the calibration signal is constant at the
<*voltages>.  If <number_of_voltages> is >1 then the calibration signal is that
in <voltages> at the specified number of <voltages_per_second>.

The <voltages> are those desired (in volts).  The function sets <voltages> to
the actual values used.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)+sizeof(float)];

	ENTER(crate_start_calibrating);
	return_code=0;
	if (crate&&((0==number_of_voltages)||((0<number_of_voltages)&&voltages)))
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_START_CALIBRATING_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			memcpy(buffer+buffer_size,&number_of_voltages,sizeof(number_of_voltages));
			buffer_size += sizeof(number_of_voltages);
			memcpy(buffer+buffer_size,&voltages_per_second,
				sizeof(voltages_per_second));
			buffer_size += sizeof(voltages_per_second);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			message_size += sizeof(float)*number_of_voltages;
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
				if (0<number_of_voltages)
				{
					retval=socket_send(crate->command_socket,(unsigned char *)voltages,
						sizeof(float)*number_of_voltages,0);
				}
			}
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
					{
						return_code=1;
					}
					if ((2+sizeof(long)==retval)&&
						((long)sizeof(float)*number_of_voltages==buffer_size)&&(buffer[0]))
					{
						if (0<buffer_size)
						{
							retval=socket_recv(crate->command_socket,
								(unsigned char *)voltages,buffer_size,0);
							if (SOCKET_ERROR!=retval)
							{
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"crate_start_calibrating.  socket_recv() voltages failed");
							}
						}
						else
						{
							return_code=1;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_start_calibrating.  socket_recv() header failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_start_calibrating.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_start_calibrating.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_start_calibrating.  Missing crate (%p) or invalid number_of_voltages (%d) or voltages (%p)",
			crate,number_of_voltages,voltages);
	}
	LEAVE;

	return (return_code);
} /* crate_start_calibrating */

static int crate_stop_calibrating(struct Unemap_crate *crate,int channel_number)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

For the <crate>.  If <channel_number> is between 1 and the total number of
channels inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.  If
<channel_number> is 0 then the function applies to the group of all channels.
Otherwise, the function fails.

Stops generating the calibration signal for the channels in the group.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(crate_stop_calibrating);
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_STOP_CALIBRATING_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
		buffer_size += sizeof(channel_number);
		message_size=buffer_size-(2+(long)sizeof(message_size));
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_stop_calibrating.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_stop_calibrating.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_stop_calibrating.  Missing crate (%p) or invalid command_socket",
			crate);
	}
	LEAVE;

	return (return_code);
} /* crate_stop_calibrating */

static int crate_set_power_start(struct Unemap_crate *crate,int on)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
The function does not need the hardware to be configured.

For the <crate>.  If <on> is zero the hardware is powered off, otherwise the
hardware is powered on.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(crate_set_power_start);
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_SET_POWER_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&on,sizeof(on));
		buffer_size += sizeof(on);
		message_size=buffer_size-(2+(long)sizeof(message_size));
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_set_power_start.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_set_power_start.  Missing crate (%p) or invalid command_socket",
			crate);
	}
	LEAVE;

	return (return_code);
} /* crate_set_power_start */

static int crate_set_power_end(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
The function does not need the hardware to be configured.

For the <crate>.  If <on> is zero the hardware is powered off, otherwise the
hardware is powered on.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_set_power_end);
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		/* get the header back */
		retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
				buffer_size);
#endif /* defined (DEBUG) */
			if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
			{
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_set_power_end.  socket_recv() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_set_power_end.  Missing crate (%p) or invalid command_socket",
			crate);
	}
	LEAVE;

	return (return_code);
} /* crate_set_power_end */

#if defined (OLD_CODE)
static int crate_set_power(struct Unemap_crate *crate,int on)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

For the <crate>.  If <on> is zero the hardware is powered off, otherwise the
hardware is powered on.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(crate_set_power);
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_SET_POWER_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&on,sizeof(on));
		buffer_size += sizeof(on);
		message_size=buffer_size-(2+(long)sizeof(message_size));
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_set_power.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"crate_set_power.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_set_power.  Missing crate (%p) or invalid command_socket",crate);
	}
	LEAVE;

	return (return_code);
} /* crate_set_power */
#endif /* defined (OLD_CODE) */

static int crate_get_power(struct Unemap_crate *crate,int *on)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

If the hardware power is on for the <crate> then <*on> is set to 1, otherwise
<*on> is set to 0.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_get_power);
	return_code=0;
	/* check arguments */
	if (crate&&on)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_GET_POWER_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&(sizeof(int)==buffer_size)&&(buffer[0]))
					{
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,buffer,
							sizeof(int),0))
						{
							memcpy(on,buffer,sizeof(int));
#if defined (DEBUG)
							display_message(INFORMATION_MESSAGE," %d",*on);
#endif /* defined (DEBUG) */
							return_code=1;
						}
					}
#if defined (DEBUG)
					display_message(INFORMATION_MESSAGE,"\n");
#endif /* defined (DEBUG) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_get_power.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_power.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_power.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_get_power.  Missing crate (%p) or on (%p)",crate,on);
	}
	LEAVE;

	return (return_code);
} /* crate_get_power */

static int crate_get_number_of_stimulators_start(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
The function does not need the hardware to be configured.

The total number of hardware stimulators for the <crate> is assigned to
<*number_of_stimulators>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_get_number_of_stimulators_start);
	return_code=0;
	/* check arguments */
	if (crate)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_GET_NUMBER_OF_STIMULATORS_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			message_size=0;
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_number_of_stimulators_start.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_number_of_stimulators_start.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_get_number_of_stimulators_start.  Missing crate");
	}
	LEAVE;

	return (return_code);
} /* crate_get_number_of_stimulators_start */

static int crate_get_number_of_stimulators_end(struct Unemap_crate *crate,
	int *number_of_stimulators)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
The function does not need the hardware to be configured.

The total number of hardware stimulators for the <crate> is assigned to
<*number_of_stimulators>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_get_number_of_stimulators_end);
	return_code=0;
	/* check arguments */
	if (crate&&number_of_stimulators)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(sizeof(int)==buffer_size)&&
					(buffer[0]))
				{
					if (SOCKET_ERROR!=socket_recv(crate->command_socket,buffer,
						sizeof(int),0))
					{
						memcpy(number_of_stimulators,buffer,sizeof(int));
#if defined (DEBUG)
						display_message(INFORMATION_MESSAGE," %ld",
							*number_of_stimulators);
#endif /* defined (DEBUG) */
						return_code=1;
					}
				}
#if defined (DEBUG)
				display_message(INFORMATION_MESSAGE,"\n");
#endif /* defined (DEBUG) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_number_of_stimulators_end.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_number_of_stimulators_end.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_get_number_of_stimulators_end.  Missing crate (%p) or number_of_stimulators (%p)",
			crate,number_of_stimulators);
	}
	LEAVE;

	return (return_code);
} /* crate_get_number_of_stimulators_end */

#if defined (OLD_CODE)
static int crate_get_number_of_stimulators(struct Unemap_crate *crate,
	int *number_of_stimulators)
/*******************************************************************************
LAST MODIFIED : 6 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

The total number of hardware stimulators for the <crate> is assigned to
<*number_of_stimulators>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_get_number_of_stimulators);
	return_code=0;
	/* check arguments */
	if (crate&&number_of_stimulators)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_GET_NUMBER_OF_STIMULATORS_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			message_size=0;
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&(sizeof(int)==buffer_size)&&
						(buffer[0]))
					{
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,buffer,
							sizeof(int),0))
						{
							memcpy(number_of_stimulators,buffer,sizeof(int));
#if defined (DEBUG)
							display_message(INFORMATION_MESSAGE," %ld",
								*number_of_stimulators);
#endif /* defined (DEBUG) */
							return_code=1;
						}
					}
#if defined (DEBUG)
					display_message(INFORMATION_MESSAGE,"\n");
#endif /* defined (DEBUG) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_get_number_of_stimulators.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_number_of_stimulators.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_number_of_stimulators.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_get_number_of_stimulators.  Missing crate (%p) or number_of_stimulators (%p)",
			crate,number_of_stimulators);
	}
	LEAVE;

	return (return_code);
} /* crate_get_number_of_stimulators */
#endif /* defined (OLD_CODE) */

static int crate_channel_valid_for_stimulator_start(struct Unemap_crate *crate,
	int stimulator_number,int channel_number)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
The function does not need the hardware to be configured.

Returns non-zero if <stimulator_number> can stimulate down <channel_number> for
<crate>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)];

	ENTER(crate_channel_valid_for_stimulator_start);
	return_code=0;
	if (crate)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_CHANNEL_VALID_FOR_STIMULATOR_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&stimulator_number,sizeof(stimulator_number));
			buffer_size += sizeof(stimulator_number);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_channel_valid_for_stimulator_start.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_channel_valid_for_stimulator_start.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_channel_valid_for_stimulator_start.  Missing crate");
	}
	LEAVE;

	return (return_code);
} /* crate_channel_valid_for_stimulator_start */

static int crate_channel_valid_for_stimulator_end(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
The function does not need the hardware to be configured.

Returns non-zero if <stimulator_number> can stimulate down <channel_number> for
<crate>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_channel_valid_for_stimulator_end);
	return_code=0;
	if (crate)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_channel_valid_for_stimulator_end.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_channel_valid_for_stimulator_end.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_channel_valid_for_stimulator_end.  Missing crate");
	}
	LEAVE;

	return (return_code);
} /* crate_channel_valid_for_stimulator_end */

#if defined (OLD_CODE)
static int crate_channel_valid_for_stimulator(struct Unemap_crate *crate,
	int stimulator_number,int channel_number)
/*******************************************************************************
LAST MODIFIED : 6 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

Returns non-zero if <stimulator_number> can stimulate down <channel_number> for
<crate>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)];

	ENTER(crate_channel_valid_for_stimulator);
	return_code=0;
	if (crate)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_CHANNEL_VALID_FOR_STIMULATOR_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&stimulator_number,sizeof(stimulator_number));
			buffer_size += sizeof(stimulator_number);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
					{
						return_code=1;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_channel_valid_for_stimulator.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_channel_valid_for_stimulator.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_channel_valid_for_stimulator.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_channel_valid_for_stimulator.  Missing crate");
	}
	LEAVE;

	return (return_code);
} /* crate_channel_valid_for_stimulator */
#endif /* defined (OLD_CODE) */

static int crate_get_card_state(struct Unemap_crate *crate,int channel_number,
	int *battA_state,int *battGood_state,float *filter_frequency,int *filter_taps,
	unsigned char shift_registers[10],int *GA0_state,int *GA1_state,
	int *NI_gain,int *input_mode,int *polarity,float *tol_settling,
	int *sampling_interval,int *settling_step_max)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Returns the current state of the signal conditioning card containing the
<channel_number> for the <crate>.

Intended for diagnostic use only.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+10*sizeof(int)+2*sizeof(float)+10];

	ENTER(crate_get_card_state);
	return_code=0;
	/* check arguments */
	if (crate&&battA_state&&battGood_state&&filter_frequency&&filter_taps&&
		shift_registers&&GA0_state&&GA1_state&&NI_gain&&input_mode&&polarity&&
		tol_settling&&sampling_interval&&settling_step_max)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			buffer[0]=UNEMAP_GET_CARD_STATE_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(crate->command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(long)==retval)&&
						(10*sizeof(int)+2*sizeof(float)+10==buffer_size)&&(buffer[0]))
					{
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,buffer,
							buffer_size,0))
						{
							buffer_size=0;
							memcpy(battA_state,buffer+buffer_size,sizeof(int));
							buffer_size += sizeof(int);
							memcpy(battGood_state,buffer+buffer_size,sizeof(int));
							buffer_size += sizeof(int);
							memcpy(filter_frequency,buffer+buffer_size,sizeof(float));
							buffer_size += sizeof(float);
							memcpy(filter_taps,buffer+buffer_size,sizeof(int));
							buffer_size += sizeof(int);
							memcpy(shift_registers,buffer+buffer_size,10);
							buffer_size += 10;
							memcpy(GA0_state,buffer+buffer_size,sizeof(int));
							buffer_size += sizeof(int);
							memcpy(GA1_state,buffer+buffer_size,sizeof(int));
							buffer_size += sizeof(int);
							memcpy(NI_gain,buffer+buffer_size,sizeof(int));
							buffer_size += sizeof(int);
							memcpy(input_mode,buffer+buffer_size,sizeof(int));
							buffer_size += sizeof(int);
							memcpy(polarity,buffer+buffer_size,sizeof(int));
							buffer_size += sizeof(int);
							memcpy(tol_settling,buffer+buffer_size,sizeof(float));
							buffer_size += sizeof(float);
							memcpy(sampling_interval,buffer+buffer_size,sizeof(int));
							buffer_size += sizeof(int);
							memcpy(settling_step_max,buffer+buffer_size,sizeof(int));
							buffer_size += sizeof(int);
							return_code=1;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_get_card_state.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_card_state.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_card_state.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"crate_get_card_state.  Invalid argument(s).  %p %d %p %p %p %p %p %p %p %p %p %p %p %p %p",
			crate,channel_number,battA_state,battGood_state,filter_frequency,
			filter_taps,shift_registers,GA0_state,GA1_state,NI_gain,input_mode,
			polarity,tol_settling,sampling_interval,settling_step_max);
	}
	LEAVE;

	return (return_code);
} /* crate_get_card_state */

static int crate_toggle_shift_register(struct Unemap_crate *crate,
	int channel_number,int register_number)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Toggles the <shift_register> of the signal conditioning card containing the
<channel_number> for the <crate>.

Intended for diagnostic use only.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)];

	ENTER(crate_toggle_shift_register);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		buffer[0]=UNEMAP_TOGGLE_SHIFT_REGISTER_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
		buffer_size += sizeof(channel_number);
		memcpy(buffer+buffer_size,&register_number,sizeof(register_number));
		buffer_size += sizeof(register_number);
		message_size=buffer_size-(2+(long)sizeof(message_size));
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(crate->command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(crate->command_socket,buffer,2+sizeof(long),0);
			if (SOCKET_ERROR!=retval)
			{
				memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Received %d bytes, data %x %x %ld\n",retval,buffer[0],buffer[1],
					buffer_size);
#endif /* defined (DEBUG) */
				if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_toggle_shift_register.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_toggle_shift_register.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
	"crate_toggle_shift_register.  Missing crate (%p) or invalid command_socket",
			crate);
	}
	LEAVE;

	return (return_code);
} /* crate_toggle_shift_register */

static int crate_initialize_connection(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 2 July 2000

DESCRIPTION :
Sets up the connection with the unemap hardware service for the <crate>.
==============================================================================*/
{
#if defined (USE_SOCKETS)
	int return_code,socket_type;
	struct hostent *internet_host_data;
	struct sockaddr_in server_socket;
	unsigned long internet_address;
#endif /* defined (USE_SOCKETS) */

	ENTER(crate_initialize_connection);
#if defined (DEBUG)
	/*???debug */
	printf("enter crate_initialize_connection %p\n",crate);
#endif /* defined (DEBUG) */
	return_code=0;
	if (crate&&(crate->server_name))
	{
#if defined (DEBUG)
		/*???debug */
		printf("server_name=/%s/, command_socket=%d\n",crate->server_name,
			crate->command_socket);
#endif /* defined (DEBUG) */
#if defined (USE_SOCKETS)
		if (INVALID_SOCKET==crate->command_socket)
		{
			if (isalpha((crate->server_name)[0]))
			{
				/* server address is a name */
				internet_host_data=gethostbyname(crate->server_name);
			}
			else
			{
				/* server address is a "." format IP address */
				internet_address=inet_addr(crate->server_name);
				internet_host_data=gethostbyaddr((char *)&internet_address,
					sizeof(internet_address),AF_INET);
			}
			if (internet_host_data)
			{
				/* create the acquired socket.  Do this before the command
					socket because the service responds to the command socket
					connection */
				socket_type=DEFAULT_SOCKET_TYPE;
				crate->acquired_socket=socket(AF_INET,socket_type,0);
				if (INVALID_SOCKET!=crate->acquired_socket)
				{
					/* connect to the server */
					memset(&server_socket,0,sizeof(server_socket));
						/*???DB.  Have to use memset because some implementations of
							struct sockaddr_in don't have the sin_len field */
					memcpy(&(server_socket.sin_addr),internet_host_data->h_addr,
						internet_host_data->h_length);
					server_socket.sin_family=internet_host_data->h_addrtype;
					server_socket.sin_port=htons(crate->acquired_port);
#if defined (DEBUG)
					/*???debug */
					printf("acquired_port=%d\n",crate->acquired_port);
#endif /* defined (DEBUG) */
					if (SOCKET_ERROR!=connect(crate->acquired_socket,
						(struct sockaddr *)&server_socket,sizeof(server_socket)))
					{
						/* create the calibration socket.  Do this before the command
							socket because the service responds to the command socket
							connection */
						socket_type=DEFAULT_SOCKET_TYPE;
						crate->calibration_socket=socket(AF_INET,socket_type,0);
						if (INVALID_SOCKET!=crate->calibration_socket)
						{
							/* connect to the server */
							memset(&server_socket,0,sizeof(server_socket));
								/*???DB.  Have to use memset because some implementations of
									struct sockaddr_in don't have the sin_len field */
							memcpy(&(server_socket.sin_addr),internet_host_data->h_addr,
								internet_host_data->h_length);
							server_socket.sin_family=internet_host_data->h_addrtype;
							server_socket.sin_port=htons(crate->calibration_port);
#if defined (DEBUG)
							/*???debug */
							printf("calibration_port=%d\n",crate->calibration_port);
#endif /* defined (DEBUG) */
							if (SOCKET_ERROR!=connect(crate->calibration_socket,
								(struct sockaddr *)&server_socket,sizeof(server_socket)))
							{
								/* create the scrolling socket.  Do this before the command
									socket because the service responds to the command socket
									connection */
								socket_type=DEFAULT_SOCKET_TYPE;
								crate->scrolling_socket=socket(AF_INET,socket_type,0);
								if (INVALID_SOCKET!=crate->scrolling_socket)
								{
									/* connect to the server */
									memset(&server_socket,0,sizeof(server_socket));
										/*???DB.  Have to use memset because some implementations
											of struct sockaddr_in don't have the sin_len field */
									memcpy(&(server_socket.sin_addr),internet_host_data->h_addr,
										internet_host_data->h_length);
									server_socket.sin_family=internet_host_data->h_addrtype;
									server_socket.sin_port=htons(crate->scrolling_port);
#if defined (DEBUG)
									/*???debug */
									printf("scrolling_port=%d\n",crate->scrolling_port);
#endif /* defined (DEBUG) */
									if (SOCKET_ERROR!=connect(crate->scrolling_socket,
										(struct sockaddr *)&server_socket,sizeof(server_socket)))
									{
										/* create the command socket */
										socket_type=DEFAULT_SOCKET_TYPE;
										crate->command_socket=socket(AF_INET,socket_type,0);
										if (INVALID_SOCKET!=crate->command_socket)
										{
											/* connect to the server */
											memset(&server_socket,0,sizeof(server_socket));
												/*???DB.  Have to use memset because some
													implementations of struct sockaddr_in don't have the
													sin_len field */
											memcpy(&(server_socket.sin_addr),
												internet_host_data->h_addr,
												internet_host_data->h_length);
											server_socket.sin_family=internet_host_data->h_addrtype;
											server_socket.sin_port=htons(crate->command_port);
#if defined (DEBUG)
											/*???debug */
											printf("command_port=%d\n",crate->command_port);
#endif /* defined (DEBUG) */
											if (SOCKET_ERROR!=connect(crate->command_socket,
												(struct sockaddr *)&server_socket,
												sizeof(server_socket)))
											{
												if (crate_get_number_of_channels(crate,
													&(crate->number_of_channels)))
												{
													return_code=1;
												}
												else
												{
													display_message(ERROR_MESSAGE,
					"crate_initialize_connection.  Could not get the number of channels");
#if defined (WIN32)
													closesocket(crate->acquired_socket);
													closesocket(crate->calibration_socket);
													closesocket(crate->scrolling_socket);
													closesocket(crate->command_socket);
#endif /* defined (WIN32) */
#if defined (UNIX)
													close(crate->acquired_socket);
													close(crate->calibration_socket);
													close(crate->scrolling_socket);
													close(crate->command_socket);
#endif /* defined (UNIX) */
													crate->acquired_socket=INVALID_SOCKET;
													crate->calibration_socket=INVALID_SOCKET;
													crate->scrolling_socket=INVALID_SOCKET;
													crate->command_socket=INVALID_SOCKET;
												}
											}
											else
											{
												display_message(ERROR_MESSAGE,
"crate_initialize_connection.  Could not connect command_socket.  Port %hu. Error code %d",
													crate->command_port,
#if defined (WIN32)
													WSAGetLastError()
#endif /* defined (WIN32) */
#if defined (UNIX)
													errno
#endif /* defined (UNIX) */
													);
#if defined (WIN32)
												closesocket(crate->acquired_socket);
												closesocket(crate->calibration_socket);
												closesocket(crate->scrolling_socket);
												closesocket(crate->command_socket);
#endif /* defined (WIN32) */
#if defined (UNIX)
												close(crate->acquired_socket);
												close(crate->calibration_socket);
												close(crate->scrolling_socket);
												close(crate->command_socket);
#endif /* defined (UNIX) */
												crate->acquired_socket=INVALID_SOCKET;
												crate->calibration_socket=INVALID_SOCKET;
												crate->scrolling_socket=INVALID_SOCKET;
												crate->command_socket=INVALID_SOCKET;
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
"crate_initialize_connection.  Could not create command_socket.  Error code %d",
#if defined (WIN32)
												WSAGetLastError()
#endif /* defined (WIN32) */
#if defined (UNIX)
												errno
#endif /* defined (UNIX) */
												);
#if defined (WIN32)
											closesocket(crate->acquired_socket);
											closesocket(crate->calibration_socket);
											closesocket(crate->scrolling_socket);
#endif /* defined (WIN32) */
#if defined (UNIX)
											close(crate->acquired_socket);
											close(crate->calibration_socket);
											close(crate->scrolling_socket);
#endif /* defined (UNIX) */
											crate->acquired_socket=INVALID_SOCKET;
											crate->calibration_socket=INVALID_SOCKET;
											crate->scrolling_socket=INVALID_SOCKET;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
"crate_initialize_connection.  Could not connect scrolling_socket.  Port %hu.  Error code %d",
											crate->scrolling_port,
#if defined (WIN32)
											WSAGetLastError()
#endif /* defined (WIN32) */
#if defined (UNIX)
											errno
#endif /* defined (UNIX) */
											);
#if defined (WIN32)
										closesocket(crate->acquired_socket);
										closesocket(crate->calibration_socket);
										closesocket(crate->scrolling_socket);
#endif /* defined (WIN32) */
#if defined (UNIX)
										close(crate->acquired_socket);
										close(crate->calibration_socket);
										close(crate->scrolling_socket);
#endif /* defined (UNIX) */
										crate->acquired_socket=INVALID_SOCKET;
										crate->calibration_socket=INVALID_SOCKET;
										crate->scrolling_socket=INVALID_SOCKET;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
"crate_initialize_connection.  Could not create scrolling_socket.  Error code %d",
#if defined (WIN32)
										WSAGetLastError()
#endif /* defined (WIN32) */
#if defined (UNIX)
										errno
#endif /* defined (UNIX) */
										);
#if defined (WIN32)
									closesocket(crate->acquired_socket);
									closesocket(crate->calibration_socket);
#endif /* defined (WIN32) */
#if defined (UNIX)
									close(crate->acquired_socket);
									close(crate->calibration_socket);
#endif /* defined (UNIX) */
									crate->acquired_socket=INVALID_SOCKET;
									crate->calibration_socket=INVALID_SOCKET;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
"crate_initialize_connection.  Could not connect calibration_socket.  Port %hu.  Error code %d",
									crate->calibration_port,
#if defined (WIN32)
									WSAGetLastError()
#endif /* defined (WIN32) */
#if defined (UNIX)
									errno
#endif /* defined (UNIX) */
									);
#if defined (WIN32)
								closesocket(crate->acquired_socket);
								closesocket(crate->calibration_socket);
#endif /* defined (WIN32) */
#if defined (UNIX)
								close(crate->acquired_socket);
								close(crate->calibration_socket);
#endif /* defined (UNIX) */
								crate->acquired_socket=INVALID_SOCKET;
								crate->calibration_socket=INVALID_SOCKET;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
"crate_initialize_connection.  Could not create calibration_socket.  Error code %d",
#if defined (WIN32)
								WSAGetLastError()
#endif /* defined (WIN32) */
#if defined (UNIX)
								errno
#endif /* defined (UNIX) */
								);
#if defined (WIN32)
							closesocket(crate->acquired_socket);
#endif /* defined (WIN32) */
#if defined (UNIX)
							close(crate->acquired_socket);
#endif /* defined (UNIX) */
							crate->acquired_socket=INVALID_SOCKET;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
"crate_initialize_connection.  Could not connect acquired_socket.  Port %hu.  Error code %d",
							crate->acquired_port,
#if defined (WIN32)
							WSAGetLastError()
#endif /* defined (WIN32) */
#if defined (UNIX)
							errno
#endif /* defined (UNIX) */
							);
#if defined (WIN32)
						closesocket(crate->acquired_socket);
#endif /* defined (WIN32) */
#if defined (UNIX)
						close(crate->acquired_socket);
#endif /* defined (UNIX) */
						crate->acquired_socket=INVALID_SOCKET;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
"crate_initialize_connection.  Could not create acquired_socket.  Error code %d",
#if defined (WIN32)
						WSAGetLastError()
#endif /* defined (WIN32) */
#if defined (UNIX)
						errno
#endif /* defined (UNIX) */
						);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"crate_initialize_connection.  Could not resolve server name [%s]",
					crate->server_name);
			}
		}
		else
		{
			/* already initialized */
			return_code=1;
		}
#endif /* defined (USE_SOCKETS) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_initialize_connection.  Invalid argument %p",crate);
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave crate_initialize_connection %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_initialize_connection */

static int initialize_connection(void)
/*******************************************************************************
LAST MODIFIED : 12 March 2000

DESCRIPTION :
Sets up the connections with the unemap crates.
==============================================================================*/
{
#if defined (USE_SOCKETS)
	char *hardware_directory,*server_file_name,*server_name;
	FILE *server_file;
	int number_of_servers,return_code;
	struct Unemap_crate *crate;
	unsigned short port;
#if defined (WIN32)
	WORD wVersionRequested;
	WSADATA wsaData;
#endif /* defined (WIN32) */
#endif /* defined (USE_SOCKETS) */

	ENTER(initialize_connection);
#if defined (DEBUG)
	/*???debug */
	printf("enter initialize_connection %p %d %d\n",module_unemap_crates,
		module_number_of_unemap_crates,allow_open_connection);
#endif /* defined (DEBUG) */
	return_code=0;
#if defined (USE_SOCKETS)
	if (!module_unemap_crates)
	{
		/* only allow one attempt to open a closed connection */
		if (allow_open_connection)
		{
			allow_open_connection=0;
			number_of_servers=0;
			if (hardware_directory=getenv("UNEMAP_HARDWARE"))
			{
				if (ALLOCATE(server_file_name,char,strlen(hardware_directory)+
					strlen(SERVER_FILE_NAME)+2))
				{
					strcpy(server_file_name,hardware_directory);
#if defined (WIN32)
					if ('\\'!=server_file_name[strlen(server_file_name)-1])
					{
						strcat(server_file_name,"\\");
					}
#else /* defined (WIN32) */
					if ('/'!=server_file_name[strlen(server_file_name)-1])
					{
						strcat(server_file_name,"/");
					}
#endif /* defined (WIN32) */
					strcat(server_file_name,SERVER_FILE_NAME);
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,
"Environment variable UNEMAP_HARDWARE is not defined.  Using current directory for %s",
					SERVER_FILE_NAME);
				if (ALLOCATE(server_file_name,char,strlen(SERVER_FILE_NAME)+1))
				{
					strcpy(server_file_name,SERVER_FILE_NAME);
				}
			}
			if (server_file_name)
			{
				if (server_file=fopen(server_file_name,"r"))
				{
					return_code=1;
					while (return_code&&(!feof(server_file)))
					{
						if (read_string(server_file,"s",&server_name)&&
							REALLOCATE(crate,module_unemap_crates,struct Unemap_crate,
							number_of_servers+1))
						{
							module_unemap_crates=crate;
							crate += number_of_servers;
							crate->server_name=server_name;
							number_of_servers++;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"initialize_connection.  Error reading %s",server_file_name);
							return_code=0;
						}
						fscanf(server_file," ");
					}
					fclose(server_file);
				}
				DEALLOCATE(server_file_name);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"initialize_connection.  Could not allocate server_file_name");
			}
			if (number_of_servers<=0)
			{
				display_message(WARNING_MESSAGE,
					"Using default unemap server name (%s)",DEFAULT_SERVER_NAME);
				if (ALLOCATE(module_unemap_crates,struct Unemap_crate,1)&&
					ALLOCATE(module_unemap_crates->server_name,char,
					strlen(DEFAULT_SERVER_NAME)+1))
				{
					strcpy(module_unemap_crates->server_name,DEFAULT_SERVER_NAME);
					number_of_servers=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"initialize_connection.  Error allocating unemap crate");
					return_code=0;
				}
				server_name=DEFAULT_SERVER_NAME;
			}
			if (number_of_servers>0)
			{
#if defined (WIN32)
				wVersionRequested=MAKEWORD(2,2);
				if (SOCKET_ERROR!=WSAStartup(wVersionRequested,&wsaData))
				{
#endif /* defined (WIN32) */
					port=DEFAULT_PORT;
					module_number_of_unemap_crates=0;
					module_number_of_channels=0;
					crate=module_unemap_crates;
					while (return_code&&(number_of_servers>0))
					{
						/*???DB.  The port doesn't have to be unique, only the port and
							machine being connected to */
						port=DEFAULT_PORT;
#if defined (WIN32)
						crate->acquired_socket=(SOCKET)INVALID_SOCKET;
						crate->acquired_socket_thread_stop_event=NULL;
						crate->calibration_socket=(SOCKET)INVALID_SOCKET;
						crate->calibration_socket_thread_stop_event=NULL;
						crate->command_socket=(SOCKET)INVALID_SOCKET;
						crate->scrolling_socket=(SOCKET)INVALID_SOCKET;
						crate->scrolling_socket_thread_stop_event=NULL;
#endif /* defined (WIN32) */
#if defined (UNIX)
						crate->acquired_socket=(int)INVALID_SOCKET;
						crate->calibration_socket=(int)INVALID_SOCKET;
						crate->command_socket=(int)INVALID_SOCKET;
						crate->scrolling_socket=(int)INVALID_SOCKET;
#endif /* defined (UNIX) */
#if defined (MOTIF)
						crate->acquired_socket_xid=0;
						crate->calibration_socket_xid=0;
						crate->scrolling_socket_xid=0;
#endif /* defined (MOTIF) */
						crate->command_port=port;
						port++;
						crate->scrolling_port=port;
						port++;
						crate->calibration_port=port;
						port++;
						crate->acquired_port=port;
						port++;
						(crate->acquired).complete=0;
						(crate->acquired).number_of_channels=0;
						(crate->acquired).number_of_samples=0;
						(crate->acquired).samples=(short *)NULL;
						(crate->calibration).complete=0;
						(crate->calibration).number_of_channels=0;
						(crate->calibration).channel_numbers=(int *)NULL;
						(crate->calibration).channel_offsets=(float *)NULL;
						(crate->calibration).channel_gains=(float *)NULL;
						(crate->scrolling).complete=0;
						(crate->scrolling).number_of_channels=0;
						(crate->scrolling).number_of_values_per_channel=0;
						(crate->scrolling).channel_numbers=(int *)NULL;
						(crate->scrolling).values=(short *)NULL;
						crate->number_of_channels=0;
#if defined (CACHE_CLIENT_INFORMATION)
						crate->number_of_stimulators=0;
						crate->stimulator_card_indices=(int *)NULL;
						crate->number_of_unemap_cards=0;
						crate->unemap_cards=(struct Unemap_card *)NULL;
#endif /* defined (CACHE_CLIENT_INFORMATION) */
						if (return_code=crate_initialize_connection(crate))
						{
							module_number_of_channels += crate->number_of_channels;
							module_number_of_unemap_crates++;
							number_of_servers--;
							crate++;
						}
						else
						{
							close_connection();
						}
					}
#if defined (WIN32)
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"initialize_connection.  WSAStartup failed.  Error code %d",
						WSAGetLastError());
				}
#endif /* defined (WIN32) */
			}
		}
	}
	else
	{
		/* already initialized */
		return_code=1;
		allow_open_connection=1;
	}
#endif /* defined (USE_SOCKETS) */
#if defined (DEBUG)
	/*???debug */
	printf("leave initialize_connection %p %d %d %d\n",module_unemap_crates,
		module_number_of_unemap_crates,allow_open_connection,return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* initialize_connection */

#if defined (CACHE_CLIENT_INFORMATION)
static int get_cache_information(void)
/*******************************************************************************
LAST MODIFIED : 26 March 2000

DESCRIPTION :
Retrieves the unemap information that is cached with the client.
==============================================================================*/
{
	int channel_number,i,j,maximum_number_of_unemap_cards,return_code,
		*stimulator_number,*stimulator_numbers;
	struct Unemap_crate *crate;

	ENTER(get_cache_information);
#if defined (DEBUG)
	/*???debug */
	printf("enter get_cache_information\n");
#endif /* defined (DEBUG) */
	return_code=0;
	if ((crate=module_unemap_crates)&&(0<(i=module_number_of_unemap_crates))&&
		(!module_get_cache_information_failed))
	{
		if (ALLOCATE(stimulator_numbers,int,module_number_of_unemap_crates))
		{
			return_code=1;
			module_force_connection=1;
			while (return_code&&(i>0))
			{
				if (!(crate->unemap_cards))
				{
					return_code=crate_get_number_of_stimulators_start(crate);
				}
				crate++;
				i--;
			}
			if (return_code)
			{
				crate=module_unemap_crates;
				i=module_number_of_unemap_crates;
				while (return_code&&(i>0))
				{
					if (!(crate->unemap_cards))
					{
						return_code=crate_get_number_of_stimulators_end(crate,
							&(crate->number_of_stimulators));
					}
					crate++;
					i--;
				}
				if (return_code)
				{
					crate=module_unemap_crates;
					i=module_number_of_unemap_crates;
					module_number_of_stimulators=0;
					stimulator_number=stimulator_numbers;
					maximum_number_of_unemap_cards=0;
					while (return_code&&(i>0))
					{
						if (!(crate->unemap_cards))
						{
							*stimulator_number=1;
							crate->unemap_cards=(struct Unemap_card *)NULL;
							crate->stimulator_card_indices=(int *)NULL;
							if ((0<(crate->number_of_unemap_cards=
								(int)((crate->number_of_channels)/
								NUMBER_OF_CHANNELS_ON_NI_CARD)))&&ALLOCATE(crate->unemap_cards,
								struct Unemap_card,crate->number_of_unemap_cards)&&
								((0==crate->number_of_stimulators)||ALLOCATE(
								crate->stimulator_card_indices,int,
								crate->number_of_stimulators)))
							{
								module_number_of_stimulators += crate->number_of_stimulators;
								if (maximum_number_of_unemap_cards<
									crate->number_of_unemap_cards)
								{
									maximum_number_of_unemap_cards=crate->number_of_unemap_cards;
								}
							}
							else
							{
								return_code=0;
							}
						}
						else
						{
							*stimulator_number=0;
							module_number_of_stimulators += crate->number_of_stimulators;
						}
						stimulator_number++;
						crate++;
						i--;
					}
					if (return_code)
					{
						channel_number=1;
						for (j=0;j<maximum_number_of_unemap_cards;j++)
						{
							crate=module_unemap_crates;
							stimulator_number=stimulator_numbers;
							for (i=module_number_of_unemap_crates;i>0;i--)
							{
								if ((*stimulator_number)&&(j<(crate->number_of_unemap_cards)))
								{
									crate_get_voltage_range_start(crate,channel_number);
								}
								stimulator_number++;
								crate++;
							}
							crate=module_unemap_crates;
							stimulator_number=stimulator_numbers;
							for (i=module_number_of_unemap_crates;i>0;i--)
							{
								if ((*stimulator_number)&&(j<(crate->number_of_unemap_cards)))
								{
									crate_get_voltage_range_end(crate,
										&((crate->unemap_cards[j]).minimum_voltage),
										&((crate->unemap_cards[j]).maximum_voltage));
								}
								stimulator_number++;
								crate++;
							}
							crate=module_unemap_crates;
							stimulator_number=stimulator_numbers;
							for (i=module_number_of_unemap_crates;i>0;i--)
							{
								if ((*stimulator_number)&&(j<(crate->number_of_unemap_cards)))
								{
									crate_get_gain_start(crate,channel_number);
								}
								stimulator_number++;
								crate++;
							}
							crate=module_unemap_crates;
							stimulator_number=stimulator_numbers;
							for (i=module_number_of_unemap_crates;i>0;i--)
							{
								if ((*stimulator_number)&&(j<(crate->number_of_unemap_cards)))
								{
									crate_get_gain_end(crate,
										&((crate->unemap_cards[j]).pre_filter_gain),
										&((crate->unemap_cards[j]).post_filter_gain));
									((crate->unemap_cards)[j]).minimum_voltage *=
										((crate->unemap_cards[j]).pre_filter_gain)*
										((crate->unemap_cards[j]).post_filter_gain);
									((crate->unemap_cards)[j]).maximum_voltage *=
										((crate->unemap_cards[j]).pre_filter_gain)*
										((crate->unemap_cards[j]).post_filter_gain);
								}
								stimulator_number++;
								crate++;
							}
							crate=module_unemap_crates;
							stimulator_number=stimulator_numbers;
							for (i=module_number_of_unemap_crates;i>0;i--)
							{
								if ((*stimulator_number)&&(j<(crate->number_of_unemap_cards)))
								{
									crate_channel_valid_for_stimulator_start(crate,
										*stimulator_number,channel_number);
								}
								stimulator_number++;
								crate++;
							}
							crate=module_unemap_crates;
							stimulator_number=stimulator_numbers;
							for (i=module_number_of_unemap_crates;i>0;i--)
							{
								if ((*stimulator_number)&&(j<(crate->number_of_unemap_cards)))
								{
									if (crate_channel_valid_for_stimulator_end(crate))
									{
										(crate->stimulator_card_indices)[(*stimulator_number)-1]=j;
										(*stimulator_number)++;
									}
								}
								stimulator_number++;
								crate++;
							}
							channel_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
						}
					}
					else
					{
						while (i<=module_number_of_unemap_crates)
						{
							crate->number_of_unemap_cards=0;
							crate->number_of_stimulators=0;
							DEALLOCATE(crate->unemap_cards);
							DEALLOCATE(crate->stimulator_card_indices);
							crate--;
							i++;
						}
					}
				}
				else
				{
					while (i>0)
					{
						if (!(crate->unemap_cards))
						{
							crate_get_number_of_stimulators_end(crate,
								&(crate->number_of_stimulators));
						}
						crate++;
						i--;
					}
				}
			}
			else
			{
				crate--;
				i++;
				while (i<module_number_of_unemap_crates)
				{
					crate--;
					i++;
					if (!(crate->unemap_cards))
					{
						crate_get_number_of_stimulators_end(crate,
							&(crate->number_of_stimulators));
					}
				}
			}
			module_force_connection=0;
			DEALLOCATE(stimulator_numbers);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_cache_information.  Could not allocate stimulator_numbers");
		}
		if (!return_code)
		{
			crate=module_unemap_crates;
			for (i=module_number_of_unemap_crates;i>0;i--)
			{
				crate->number_of_unemap_cards=0;
				crate->number_of_stimulators=0;
				DEALLOCATE(crate->unemap_cards);
				DEALLOCATE(crate->stimulator_card_indices);
				crate++;
			}
			module_get_cache_information_failed=1;
		}
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave get_cache_information %d %d\n",return_code,
		module_number_of_stimulators);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* get_cache_information */

#if defined (OLD_CODE)
static int get_cache_information(void)
/*******************************************************************************
LAST MODIFIED : 26 March 2000

DESCRIPTION :
Retrieves the unemap information that is cached with the client.
==============================================================================*/
{
	int channel_number,i,j,return_code,stimulator_number;
	struct Unemap_crate *crate;

	ENTER(get_cache_information);
#if defined (DEBUG)
	/*???debug */
	printf("enter get_cache_information\n");
#endif /* defined (DEBUG) */
	return_code=0;
	if ((crate=module_unemap_crates)&&(0<(i=module_number_of_unemap_crates))&&
		(!module_get_cache_information_failed))
	{
		return_code=1;
		module_number_of_stimulators=0;
		while (return_code&&(i>0))
		{
			if (!(crate->unemap_cards))
			{
				module_force_connection=1;
/*???DB.  Where I'm up to */
				crate->unemap_cards=(struct Unemap_card *)NULL;
				crate->stimulator_card_indices=(int *)NULL;
				if ((0<(crate->number_of_unemap_cards=
					(int)((crate->number_of_channels)/
					NUMBER_OF_CHANNELS_ON_NI_CARD)))&&ALLOCATE(crate->unemap_cards,
					struct Unemap_card,crate->number_of_unemap_cards)&&
					crate_get_number_of_stimulators(crate,
					&(crate->number_of_stimulators))&&
					((0==crate->number_of_stimulators)||ALLOCATE(
					crate->stimulator_card_indices,int,crate->number_of_stimulators)))
				{
					module_number_of_stimulators += crate->number_of_stimulators;
					channel_number=1;
					stimulator_number=1;
					for (j=0;j<crate->number_of_unemap_cards;j++)
					{
						crate_get_voltage_range(crate,channel_number,
							&((crate->unemap_cards[j]).minimum_voltage),
							&((crate->unemap_cards[j]).maximum_voltage));
						crate_get_gain(crate,channel_number,
							&((crate->unemap_cards[j]).pre_filter_gain),
							&((crate->unemap_cards[j]).post_filter_gain));
						((crate->unemap_cards)[j]).minimum_voltage *=
							((crate->unemap_cards[j]).pre_filter_gain)*
							((crate->unemap_cards[j]).post_filter_gain);
						((crate->unemap_cards)[j]).maximum_voltage *=
							((crate->unemap_cards[j]).pre_filter_gain)*
							((crate->unemap_cards[j]).post_filter_gain);
						if (crate_channel_valid_for_stimulator(crate,stimulator_number,
							channel_number))
						{
							(crate->stimulator_card_indices)[stimulator_number-1]=j;
							stimulator_number++;
						}
						channel_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
					}
				}
				else
				{
					return_code=0;
				}
				module_force_connection=0;
			}
			else
			{
				module_number_of_stimulators += crate->number_of_stimulators;
			}
			crate++;
			i--;
		}
		if (!return_code)
		{
			while (i<=module_number_of_unemap_crates)
			{
				crate->number_of_unemap_cards=0;
				crate->number_of_stimulators=0;
				DEALLOCATE(crate->unemap_cards);
				DEALLOCATE(crate->stimulator_card_indices);
				crate--;
				i++;
			}
			module_get_cache_information_failed=1;
		}
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave get_cache_information %d %d\n",return_code,
		module_number_of_stimulators);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* get_cache_information */
#endif /* defined (OLD_CODE) */
#endif /* defined (CACHE_CLIENT_INFORMATION) */

/*
Global functions
----------------
*/
int unemap_configure(float sampling_frequency,int number_of_samples_in_buffer,
#if defined (WINDOWS)
	HWND scrolling_window,UINT scrolling_message,
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
	XtAppContext application_context,
#endif /* defined (MOTIF) */
	Unemap_hardware_callback *scrolling_callback,void *scrolling_callback_data,
	float scrolling_refresh_frequency,int synchronization_card)
/*******************************************************************************
LAST MODIFIED : 9 July 2000

DESCRIPTION :
Configures the hardware for sampling at the specified <sampling_frequency> and
with the specified <number_of_samples_in_buffer>. <sampling_frequency> and
<number_of_samples_in_buffer> are requests and the actual values used should
be determined using unemap_get_sampling_frequency and
unemap_get_maximum_number_of_samples.

Every <sampling_frequency>/<scrolling_refresh_frequency> samples (one sample
	is a measument on every channel)
- if <scrolling_callback> is not NULL, <scrolling_callback> is called with
	- first argument is the number of scrolling channels
	- second argument is an array of channel numbers (in the order they were added
		using unemap_set_scrolling_channel)
	- third argument is the number of values for each channel
	- fourth argument is an array of channel values (all the values for the first
		channel, then all the values for the second channel and so on)
	- fifth argument is the user supplied callback data
	- it is the responsibilty of the callback to free the channel numbers and
		values arrays
- for WINDOWS, if <scrolling_window> is not NULL, a <scrolling_message> for
	<scrolling_window> is added to the message queue with
	- WPARAM is an array of unsigned char amalgamating the five arguments above
		- first sizeof(int) bytes is the number of scrolling channels
		- next (number of scrolling channels)*sizeof(int) bytes are the scrolling
			channel numbers
		- next sizeof(int) bytes is the number of values for each channel
		- next (number of scrolling channels)*(number of values)*sizeof(short) bytes
			are the channel values
		- last sizeof(void *) bytes are the user supplied callback data
	- LPARAM is a ULONG specifying the number of bytes ing the WPARAM array
		over the <scrolling_refresh_period>) in the array
	- it is the responsibility of the window message handler to free the WPARAM
		array.

Initially there are no scrolling channels.

<synchronization_card> is the number of the NI card, starting from 1 on the
left, for which the synchronization signal is input, via the 5-way cable, to the
attached SCU.  All other SCUs output the synchronization signal.
==============================================================================*/
{
	float crate_sampling_frequency,master_sampling_frequency;
	int i,return_code;
	struct Unemap_crate *crate;
	unsigned long crate_number_of_samples_in_buffer,
		master_number_of_samples_in_buffer;

	ENTER(unemap_configure);
	/*???debug */
	printf("enter unemap_configure.  %d\n",synchronization_card);
#if defined (DEBUG)
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if ((0<sampling_frequency)&&(0<number_of_samples_in_buffer))
	{
		if (initialize_connection()&&(crate=module_unemap_crates)&&
			(0<(i=module_number_of_unemap_crates)))
		{
			if (crate_configure_start(crate,0,sampling_frequency,
				number_of_samples_in_buffer,
#if defined (WINDOWS)
				scrolling_window,scrolling_message,
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
				application_context,
#endif /* defined (MOTIF) */
				scrolling_callback,scrolling_callback_data,
				scrolling_refresh_frequency,synchronization_card))
			{
				do
				{
					crate++;
					i--;
				} while ((i>0)&&crate_configure_start(crate,1,sampling_frequency,
					number_of_samples_in_buffer,
#if defined (WINDOWS)
					scrolling_window,scrolling_message,
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
					application_context,
#endif /* defined (MOTIF) */
					scrolling_callback,scrolling_callback_data,
					scrolling_refresh_frequency,synchronization_card));
			}
			/*???debug */
			printf("  start completed %d\n",i);
#if defined (DEBUG)
#endif /* defined (DEBUG) */
			if (i>0)
			{
				while (i<=module_number_of_unemap_crates)
				{
					crate_configure_end(crate);
					crate_deconfigure(crate);
					crate--;
					i++;
				}
			}
			else
			{
				i=module_number_of_unemap_crates;
				crate=module_unemap_crates;
				if (crate_configure_end(crate)&&
					crate_get_sampling_frequency(crate,&master_sampling_frequency)&&
					crate_get_maximum_number_of_samples(crate,
					&master_number_of_samples_in_buffer))
				{
					do
					{
						crate++;
						i--;
					} while ((i>0)&&crate_configure_end(crate)&&
						crate_get_sampling_frequency(crate,&crate_sampling_frequency)&&
						(master_sampling_frequency==crate_sampling_frequency)&&
						crate_get_maximum_number_of_samples(crate,
						&crate_number_of_samples_in_buffer)&&
						(master_number_of_samples_in_buffer==
						crate_number_of_samples_in_buffer));
				}
				/*???debug */
				printf("  end completed %d\n",i);
#if defined (DEBUG)
#endif /* defined (DEBUG) */
				if (i>0)
				{
					while (i>0)
					{
						crate++;
						i--;
						crate_configure_end(crate);
					}
					crate=module_unemap_crates;
					for (i=module_number_of_unemap_crates;i>0;i--)
					{
						crate_deconfigure(crate);
						crate--;
					}
				}
				else
				{
					return_code=1;
#if defined (CACHE_CLIENT_INFORMATION)
					get_cache_information();
#endif /* defined (CACHE_CLIENT_INFORMATION) */
				}
			}
#if defined (OLD_CODE)
			if (crate_configure(crate,0,sampling_frequency,
				number_of_samples_in_buffer,
#if defined (WINDOWS)
				scrolling_window,scrolling_message,
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
				application_context,
#endif /* defined (MOTIF) */
				scrolling_callback,scrolling_callback_data,
				scrolling_refresh_frequency)&&
				crate_get_sampling_frequency(crate,&master_sampling_frequency)&&
				crate_get_maximum_number_of_samples(crate,
				&master_number_of_samples_in_buffer))
			{
				do
				{
					crate++;
					i--;
				} while ((i>0)&&crate_configure(crate,1,sampling_frequency,
					number_of_samples_in_buffer,
#if defined (WINDOWS)
					scrolling_window,scrolling_message,
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
					application_context,
#endif /* defined (MOTIF) */
					scrolling_callback,scrolling_callback_data,
					scrolling_refresh_frequency,synchronization_card)&&
					crate_get_sampling_frequency(crate,&crate_sampling_frequency)&&
					(master_sampling_frequency==crate_sampling_frequency)&&
					crate_get_maximum_number_of_samples(crate,
					&crate_number_of_samples_in_buffer)&&
					(master_number_of_samples_in_buffer==
					crate_number_of_samples_in_buffer));
			}
			if (i>0)
			{
				while (i<=module_number_of_unemap_crates)
				{
					crate_deconfigure(crate);
					crate--;
					i++;
				}
			}
			else
			{
				return_code=1;
#if defined (CACHE_CLIENT_INFORMATION)
				get_cache_information();
#endif /* defined (CACHE_CLIENT_INFORMATION) */
			}
#endif /* defined (OLD_CODE) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_configure.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_configure.  Invalid argument(s).  %g %d",sampling_frequency,
			number_of_samples_in_buffer);
		return_code=0;
	}
	/*???debug */
	printf("leave unemap_configure\n");
#if defined (DEBUG)
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_configure */

int unemap_configured(void)
/*******************************************************************************
LAST MODIFIED : 8 March 2000

DESCRIPTION :
Returns a non-zero if unemap is configured and zero otherwise.
==============================================================================*/
{
	int i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_configured);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<(i=module_number_of_unemap_crates)))
	{
		return_code=crate_configured(crate);
		crate++;
		i--;
		while ((i>0)&&(return_code==crate_configured(crate)))
		{
			crate++;
			i--;
		}
		if (i>0)
		{
			unemap_deconfigure();
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_configured.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_configured */

int unemap_deconfigure(void)
/*******************************************************************************
LAST MODIFIED : 12 March 2000

DESCRIPTION :
Stops acquisition and signal generation.  Frees buffers associated with the
hardware.
==============================================================================*/
{
	int i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_deconfigure);
	/*???debug */
	printf("enter unemap_deconfigure\n");
#if defined (DEBUG)
#endif /* defined (DEBUG) */
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		return_code=1;
		for (i=module_number_of_unemap_crates;i>0;i--)
		{
			if (!crate_deconfigure(crate))
			{
				return_code=0;
			}
			crate++;
		}
		close_connection();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_deconfigure.  Could not initialize_connection");
	}
#if defined (CACHE_CLIENT_INFORMATION)
	module_number_of_stimulators=0;
	module_get_cache_information_failed=0;
#endif /* defined (CACHE_CLIENT_INFORMATION) */
	/*???debug */
	printf("leave unemap_deconfigure\n");
#if defined (DEBUG)
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_deconfigure */

int unemap_get_hardware_version(int *hardware_version)
/*******************************************************************************
LAST MODIFIED : 26 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

Returns the unemap <hardware_version>.
==============================================================================*/
{
	int crate_hardware_version,i,return_code,temp_hardware_version;
	struct Unemap_crate *crate;

	ENTER(unemap_get_hardware_version);
	return_code=0;
	/* check arguments */
	if (hardware_version)
	{
		if (initialize_connection()&&(crate=module_unemap_crates)&&
			(0<(i=module_number_of_unemap_crates)))
		{
			temp_hardware_version=0;
			while ((0<i)&&(return_code=crate_get_hardware_version(crate,
				&crate_hardware_version)))
			{
				temp_hardware_version |= crate_hardware_version;
				crate++;
				i--;
			}
			if (return_code)
			{
				*hardware_version=temp_hardware_version;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_hardware_version.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_hardware_version.  Missing hardware_version");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_hardware_version */

int unemap_shutdown(void)
/*******************************************************************************
LAST MODIFIED : 8 March 2000

DESCRIPTION :
Shuts down NT running on the signal conditioning unit computer.
???DB.  Not really anything to do with unemap hardware ?
==============================================================================*/
{
	int i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_shutdown);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		return_code=1;
		for (i=module_number_of_unemap_crates;i>0;i--)
		{
			if (!crate_shutdown(crate))
			{
				return_code=0;
			}
			crate++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_shutdown.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_shutdown */

int unemap_set_scrolling_channel(int channel_number)
/*******************************************************************************
LAST MODIFIED : 8 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Adds the <channel_number> to the list of channels for which scrolling
information is sent via the scrolling_callback (see unemap_configure).
==============================================================================*/
{
	int crate_channel_number,i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_set_scrolling_channel);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
	{
		crate=module_unemap_crates;
		i=module_number_of_unemap_crates;
		crate_channel_number=channel_number;
		while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
		{
			crate_channel_number -= crate->number_of_channels;
			crate++;
			i--;
		}
		if ((i>0)&&crate)
		{
			if (return_code=crate_set_scrolling_channel(crate,crate_channel_number))
			{
				(crate->scrolling).complete &= ~SCROLLING_NO_CHANNELS_FOR_CRATE;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_scrolling_channel.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_set_scrolling_channel */

int unemap_clear_scrolling_channels(void)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Clears the list of channels for which scrolling information is sent via the
scrolling_callback (see unemap_configure).
==============================================================================*/
{
	int i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_clear_scrolling_channels);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		return_code=1;
		for (i=module_number_of_unemap_crates;i>0;i--)
		{
			if (!crate_clear_scrolling_channels(crate))
			{
				return_code=0;
			}
			(crate->scrolling).complete=SCROLLING_NO_CHANNELS_FOR_CRATE;
			crate++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_clear_scrolling_channels.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_clear_scrolling_channels */

int unemap_start_scrolling(void)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Starts scrolling messages/callbacks.  Also need to be sampling to get messages/
callbacks.  Allows sampling without scrolling.
==============================================================================*/
{
	int i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_start_scrolling);
#if defined (DEBUG)
	/*???debug */
	printf("enter unemap_start_scrolling\n");
#endif /* defined (DEBUG) */
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		return_code=1;
		for (i=module_number_of_unemap_crates;i>0;i--)
		{
			if (!crate_start_scrolling(crate))
			{
				return_code=0;
			}
#if defined (DEBUG)
			/*???debug */
			printf("%p %d %d\n",crate,(crate->scrolling).number_of_channels,
				(crate->scrolling).complete);
#endif /* defined (DEBUG) */
			crate++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_start_scrolling.  Could not initialize_connection");
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave unemap_start_scrolling\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_start_scrolling */

int unemap_stop_scrolling(void)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Stops scrolling messages/callbacks.  Also need to be sampling to get messages/
callbacks.  Allows sampling without scrolling.
==============================================================================*/
{
	int i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_stop_scrolling);
#if defined (DEBUG)
	/*???debug */
	printf("enter unemap_stop_scrolling\n");
#endif /* defined (DEBUG) */
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		return_code=1;
		for (i=module_number_of_unemap_crates;i>0;i--)
		{
			if (!crate_stop_scrolling(crate))
			{
				return_code=0;
			}
#if defined (DEBUG)
			/*???debug */
			printf("%p %d %d\n",crate,(crate->scrolling).number_of_channels,
				(crate->scrolling).complete);
#endif /* defined (DEBUG) */
			crate++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_stop_scrolling.  Could not initialize_connection");
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave unemap_stop_scrolling\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_stop_scrolling */

int unemap_calibrate(Calibration_end_callback *calibration_end_callback,
	void *calibration_end_callback_data)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

When the calibration is completed <calibration_end_callback> is called with -
the number of channels calibrated, the channel numbers for the calibrated
channels, the offsets for the calibrated channels, the gains for the
calibrated channels and the <calibration_end_callback_data>.
==============================================================================*/
{
	int i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_calibrate);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		return_code=1;
		module_calibration_end_callback=calibration_end_callback;
		module_calibration_end_callback_data=calibration_end_callback_data;
		for (i=module_number_of_unemap_crates;i>0;i--)
		{
			(crate->calibration).complete=0;
			crate++;
		}
		crate=module_unemap_crates;
		for (i=module_number_of_unemap_crates;i>0;i--)
		{
			if (!crate_calibrate(crate))
			{
				return_code=0;
			}
			crate++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_calibrate.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_calibrate */

int unemap_start_sampling(void)
/*******************************************************************************
LAST MODIFIED : 30 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Starts the sampling.
???DB.  Check if already going
==============================================================================*/
{
	int i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_start_sampling);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		return_code=1;
		/* start in reverse order because the first crate actually starts the
			sampling */
		crate += module_number_of_unemap_crates;
		for (i=module_number_of_unemap_crates;i>0;i--)
		{
			crate--;
			if (!crate_start_sampling(crate))
			{
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_start_sampling.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_start_sampling */

int unemap_stop_sampling(void)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Stops the sampling.  Use <unemap_get_number_of_samples_acquired> to find out how
many samples were acquired.
==============================================================================*/
{
	int i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_stop_sampling);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		return_code=1;
		for (i=module_number_of_unemap_crates;i>0;i--)
		{
			if (!crate_stop_sampling(crate))
			{
				return_code=0;
			}
			crate++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_stop_sampling.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_stop_sampling */

int unemap_set_isolate_record_mode(int channel_number,int isolate)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

If <isolate> is not zero the group is put in isolate mode, otherwise the group
is put in record mode.  In isolate mode, the electrodes (recording, stimulation
and reference) are disconnected from the hardware and all channels are connected
to a calibration signal.
==============================================================================*/
{
	int i,crate_channel_number,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_set_isolate_record_mode);
#if defined (DEBUG)
	/*???debug */
	printf("enter unemap_set_isolate_record_mode\n");
#endif /* defined (DEBUG) */
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		if (0==channel_number)
		{
			return_code=1;
			for (i=module_number_of_unemap_crates;i>0;i--)
			{
				if (!crate_set_isolate_record_mode_start(crate,channel_number,isolate))
				{
					return_code=0;
				}
				crate++;
			}
			crate=module_unemap_crates;
			for (i=module_number_of_unemap_crates;i>0;i--)
			{
				if (!crate_set_isolate_record_mode_end(crate))
				{
					return_code=0;
				}
				crate++;
			}
		}
		else
		{
			i=module_number_of_unemap_crates;
			crate_channel_number=channel_number;
			while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
			{
				crate_channel_number -= crate->number_of_channels;
				crate++;
				i--;
			}
			if ((i>0)&&crate)
			{
				if (return_code=crate_set_isolate_record_mode_start(crate,
					crate_channel_number,isolate))
				{
					return_code=crate_set_isolate_record_mode_end(crate);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_isolate_record_mode.  Could not initialize_connection");
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave unemap_set_isolate_record_mode %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_set_isolate_record_mode */

#if defined (OLD_CODE)
int unemap_set_isolate_record_mode(int channel_number,int isolate)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

If <isolate> is not zero the group is put in isolate mode, otherwise the group
is put in record mode.  In isolate mode, the electrodes (recording, stimulation
and reference) are disconnected from the hardware and all channels are connected
to a calibration signal.
==============================================================================*/
{
	int i,crate_channel_number,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_set_isolate_record_mode);
#if defined (DEBUG)
	/*???debug */
	printf("enter unemap_set_isolate_record_mode\n");
#endif /* defined (DEBUG) */
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		if (0==channel_number)
		{
			return_code=1;
			for (i=module_number_of_unemap_crates;i>0;i--)
			{
				if (!crate_set_isolate_record_mode(crate,channel_number,isolate))
				{
					return_code=0;
				}
				crate++;
			}
		}
		else
		{
			i=module_number_of_unemap_crates;
			crate_channel_number=channel_number;
			while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
			{
				crate_channel_number -= crate->number_of_channels;
				crate++;
				i--;
			}
			if ((i>0)&&crate)
			{
				return_code=crate_set_isolate_record_mode(crate,crate_channel_number,
					isolate);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_isolate_record_mode.  Could not initialize_connection");
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave unemap_set_isolate_record_mode %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_set_isolate_record_mode */
#endif /* defined (OLD_CODE) */

int unemap_get_isolate_record_mode(int channel_number,int *isolate)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

If the group is in isolate mode, then <*isolate> is set to 1.  Otherwise
<*isolate> is set to 0.
==============================================================================*/
{
	int i,crate_channel_number,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_get_isolate_record_mode);
	return_code=0;
	/* check arguments */
	if (isolate)
	{
		/* unemap may have been started by another process, so can't just check that
			the command_socket is valid */
		if (initialize_connection()&&(crate=module_unemap_crates)&&
			(0<module_number_of_unemap_crates))
		{
			i=module_number_of_unemap_crates;
			crate_channel_number=channel_number;
			while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
			{
				crate_channel_number -= crate->number_of_channels;
				crate++;
				i--;
			}
			if ((i>0)&&crate)
			{
				return_code=crate_get_isolate_record_mode(crate,crate_channel_number,
					isolate);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_isolate_record_mode.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_isolate_record_mode.  Missing number_of_channels");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_isolate_record_mode */

int unemap_set_antialiasing_filter_frequency(int channel_number,
	float frequency)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Sets the anti-aliasing filter to the specified <frequency> for all channels.
3dB cutoff frequency = 50*tap
begin OLD_CODE
3dB cutoff frequency = 1.07/(100*R*C)
C = 10 nF
R = (99-tap) * 101 ohm
dnd OLD_CODE
<frequency> is a request and the actual value used can be found with
unemap_get_antialiasing_filter.
==============================================================================*/
{
	int i,crate_channel_number,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_set_antialiasing_filter_frequency);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		if (0==channel_number)
		{
			return_code=1;
			for (i=module_number_of_unemap_crates;i>0;i--)
			{
				if (!crate_set_antialiasing_filter_frequency(crate,channel_number,
					frequency))
				{
					return_code=0;
				}
				crate++;
			}
		}
		else
		{
			i=module_number_of_unemap_crates;
			crate_channel_number=channel_number;
			while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
			{
				crate_channel_number -= crate->number_of_channels;
				crate++;
				i--;
			}
			if ((i>0)&&crate)
			{
				return_code=crate_set_antialiasing_filter_frequency(crate,
					crate_channel_number,frequency);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
	"unemap_set_antialiasing_filter_frequency.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_set_antialiasing_filter_frequency */

int unemap_set_powerup_antialiasing_filter_frequency(int channel_number)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Makes the current anti-aliasing filter frequency the power up value for the
group.
==============================================================================*/
{
	int i,crate_channel_number,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_set_powerup_antialiasing_filter_frequency);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		if (0==channel_number)
		{
			return_code=1;
			for (i=module_number_of_unemap_crates;i>0;i--)
			{
				if (!crate_set_powerup_antialiasing_filter_frequency(crate,
					channel_number))
				{
					return_code=0;
				}
				crate++;
			}
		}
		else
		{
			i=module_number_of_unemap_crates;
			crate_channel_number=channel_number;
			while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
			{
				crate_channel_number -= crate->number_of_channels;
				crate++;
				i--;
			}
			if ((i>0)&&crate)
			{
				return_code=crate_set_powerup_antialiasing_filter_frequency(crate,
					crate_channel_number);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"unemap_set_powerup_antialiasing_filter_frequency.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_set_powerup_antialiasing_filter_frequency */

int unemap_get_antialiasing_filter_frequency(int channel_number,
	float *frequency)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

<*frequency> is set to the frequency for the anti-aliasing filter.
==============================================================================*/
{
	int i,crate_channel_number,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_get_antialiasing_filter_frequency);
	return_code=0;
	/* check arguments */
	if (frequency)
	{
		/* unemap may have been started by another process, so can't just check that
			the command_socket is valid */
		if (initialize_connection()&&(crate=module_unemap_crates)&&
			(0<module_number_of_unemap_crates))
		{
			i=module_number_of_unemap_crates;
			crate_channel_number=channel_number;
			while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
			{
				crate_channel_number -= crate->number_of_channels;
				crate++;
				i--;
			}
			if ((i>0)&&crate)
			{
				return_code=crate_get_antialiasing_filter_frequency(crate,
					crate_channel_number,frequency);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
	"unemap_get_antialiasing_filter_frequency.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_antialiasing_filter_frequency.  Missing number_of_channels");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_antialiasing_filter_frequency */

int unemap_get_number_of_channels(int *number_of_channels)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

The total number of hardware channels is assigned to <*number_of_channels>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_number_of_channels);
	return_code=0;
	/* check arguments */
	if (number_of_channels)
	{
		if (initialize_connection())
		{
			/* initialize_connection also determines the number of channels */
			*number_of_channels=module_number_of_channels;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_number_of_channels.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_number_of_channels.  Missing number_of_channels");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_number_of_channels */

int unemap_get_sample_range(long int *minimum_sample_value,
	long int *maximum_sample_value)
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function does not need the hardware to be configured.

The minimum possible sample value is assigned to <*minimum_sample_value> and the
maximum possible sample value is assigned to <*maximum_sample_value>.
==============================================================================*/
{
	int i,return_code;
	struct Unemap_crate *crate;
	long int crate_maximum_sample_value,crate_minimum_sample_value,
		temp_maximum_sample_value,temp_minimum_sample_value;

	ENTER(unemap_get_sample_range);
	return_code=0;
	/* check arguments */
	if (minimum_sample_value&&maximum_sample_value)
	{
		/* unemap may have been started by another process, so can't just check that
			the command_socket is valid */
		if (initialize_connection()&&(crate=module_unemap_crates)&&
			(0<module_number_of_unemap_crates))
		{
			return_code=1;
			temp_minimum_sample_value=1;
			temp_maximum_sample_value=0;
			for (i=module_number_of_unemap_crates;i>0;i--)
			{
				if (crate_get_sample_range(crate,&crate_minimum_sample_value,
					&crate_maximum_sample_value))
				{
					if (temp_minimum_sample_value<=temp_maximum_sample_value)
					{
						if (crate_minimum_sample_value<temp_minimum_sample_value)
						{
							temp_minimum_sample_value=crate_minimum_sample_value;
						}
						if (temp_maximum_sample_value<crate_maximum_sample_value)
						{
							temp_maximum_sample_value=crate_maximum_sample_value;
						}
					}
					else
					{
						temp_minimum_sample_value=crate_minimum_sample_value;
						temp_maximum_sample_value=crate_maximum_sample_value;
					}
				}
				else
				{
					return_code=0;
				}
				crate++;
			}
			if (return_code)
			{
				*minimum_sample_value=temp_minimum_sample_value;
				*maximum_sample_value=temp_maximum_sample_value;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_sample_range.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"unemap_get_sample_range.  Missing minimum_sample_value or maximum_sample_value");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_sample_range */

int unemap_get_voltage_range(int channel_number,float *minimum_voltage,
	float *maximum_voltage)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

The voltage range, allowing for gain, is returned via <*minimum_voltage> and
<*maximum_voltage>.
==============================================================================*/
{
	int crate_channel_number,i,return_code;
#if defined (CACHE_CLIENT_INFORMATION)
	struct Unemap_card *unemap_card;
#endif /* defined (CACHE_CLIENT_INFORMATION) */
	struct Unemap_crate *crate;

	ENTER(unemap_get_voltage_range);
	return_code=0;
	/* check arguments */
	if (minimum_voltage&&maximum_voltage)
	{
		if (initialize_connection())
		{
			crate=module_unemap_crates;
			i=module_number_of_unemap_crates;
			crate_channel_number=channel_number;
			while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
			{
				crate_channel_number -= crate->number_of_channels;
				crate++;
				i--;
			}
			if ((i>0)&&crate)
			{
#if defined (CACHE_CLIENT_INFORMATION)
				if (!module_force_connection&&get_cache_information())
				{
					if ((1<=crate_channel_number)&&
						(crate_channel_number<=crate->number_of_channels))
					{
						unemap_card=(crate->unemap_cards)+
							((crate_channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
						*minimum_voltage=(unemap_card->minimum_voltage)/
							((unemap_card->pre_filter_gain)*(unemap_card->post_filter_gain));
						*maximum_voltage=(unemap_card->maximum_voltage)/
							((unemap_card->pre_filter_gain)*(unemap_card->post_filter_gain));
						return_code=1;
					}
				}
				else
				{
#endif /* defined (CACHE_CLIENT_INFORMATION) */
					if (return_code=crate_get_voltage_range_start(crate,
						crate_channel_number))
					{
						return_code=crate_get_voltage_range_end(crate,
							minimum_voltage,maximum_voltage);
					}
#if defined (CACHE_CLIENT_INFORMATION)
				}
#endif /* defined (CACHE_CLIENT_INFORMATION) */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_voltage_range.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_voltage_range.  Missing minimum_voltage or maximum_voltage");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_voltage_range */

#if defined (OLD_CODE)
int unemap_get_voltage_range(int channel_number,float *minimum_voltage,
	float *maximum_voltage)
/*******************************************************************************
LAST MODIFIED : 6 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

The voltage range, allowing for gain, is returned via <*minimum_voltage> and
<*maximum_voltage>.
==============================================================================*/
{
	int crate_channel_number,i,return_code;
#if defined (CACHE_CLIENT_INFORMATION)
	struct Unemap_card *unemap_card;
#endif /* defined (CACHE_CLIENT_INFORMATION) */
	struct Unemap_crate *crate;

	ENTER(unemap_get_voltage_range);
	return_code=0;
	/* check arguments */
	if (minimum_voltage&&maximum_voltage)
	{
		if (initialize_connection())
		{
			crate=module_unemap_crates;
			i=module_number_of_unemap_crates;
			crate_channel_number=channel_number;
			while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
			{
				crate_channel_number -= crate->number_of_channels;
				crate++;
				i--;
			}
			if ((i>0)&&crate)
			{
#if defined (CACHE_CLIENT_INFORMATION)
				if (!module_force_connection&&get_cache_information())
				{
					if ((1<=crate_channel_number)&&
						(crate_channel_number<=crate->number_of_channels))
					{
						unemap_card=(crate->unemap_cards)+
							((crate_channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
						*minimum_voltage=(unemap_card->minimum_voltage)/
							((unemap_card->pre_filter_gain)*(unemap_card->post_filter_gain));
						*maximum_voltage=(unemap_card->maximum_voltage)/
							((unemap_card->pre_filter_gain)*(unemap_card->post_filter_gain));
						return_code=1;
					}
				}
				else
				{
#endif /* defined (CACHE_CLIENT_INFORMATION) */
					return_code=crate_get_voltage_range(crate,crate_channel_number,
						minimum_voltage,maximum_voltage);
#if defined (CACHE_CLIENT_INFORMATION)
				}
#endif /* defined (CACHE_CLIENT_INFORMATION) */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_voltage_range.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_voltage_range.  Missing minimum_voltage or maximum_voltage");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_voltage_range */
#endif /* defined (OLD_CODE) */

int unemap_get_number_of_samples_acquired(unsigned long *number_of_samples)
/*******************************************************************************
LAST MODIFIED : 12 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

The number of samples acquired per channel since <unemap_start_sampling> is
assigned to <*number_of_samples>.
==============================================================================*/
{
	int i,return_code;
	struct Unemap_crate *crate;
	unsigned long crate_number_of_samples,temp_number_of_samples;

	ENTER(unemap_get_number_of_samples_acquired);
#if defined (DEBUG)
	/*???debug */
	printf("enter unemap_get_number_of_samples_acquired\n");
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if (number_of_samples)
	{
		/* unemap may have been started by another process, so can't just check that
			the command_socket is valid */
		if (initialize_connection()&&(crate=module_unemap_crates)&&
			(0<module_number_of_unemap_crates))
		{
			temp_number_of_samples=0;
			return_code=1;
			i=module_number_of_unemap_crates;
			while (return_code&&(i>0))
			{
				if (crate_get_number_of_samples_acquired(crate,
					&crate_number_of_samples))
				{
#if defined (DEBUG)
					/*???debug */
					printf("crate_number_of_samples %lu\n",crate_number_of_samples);
#endif /* defined (DEBUG) */
					if (crate_number_of_samples>temp_number_of_samples)
					{
						temp_number_of_samples=crate_number_of_samples;
					}
				}
				else
				{
					return_code=0;
				}
				crate++;
				i--;
			}
			if (return_code)
			{
				*number_of_samples=temp_number_of_samples;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_number_of_samples_acquired.  Missing number_of_channels");
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave unemap_get_number_of_samples_acquired %d %lu\n",return_code,
		*number_of_samples);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_get_number_of_samples_acquired */

int unemap_write_samples_acquired(int channel_number,FILE *file)
/*******************************************************************************
LAST MODIFIED : 3 July 2000

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is valid (between 1 and the total number of channels
inclusive, then the <samples> for that channel are written to <file>.  If
<channel_number> is 0 then the <samples> for all channels are written to <file>.
Otherwise the function fails.

Needed for the local hardware version.  Not needed in this version, but included
for completeness.
==============================================================================*/
{
	int number_of_channels,return_code;
	short *samples;
	unsigned long number_of_samples;

	ENTER(unemap_write_samples_acquired);
	return_code=0;
	/* check arguments */
#if defined (DEBUG)
	/*???debug */
	printf("unemap_write_samples_acquired.  %p %d %f\n",file,channel_number,
		module_number_of_channels);
#endif /* defined (DEBUG) */
	if (file&&(0<=channel_number)&&(channel_number<=module_number_of_channels))
	{
		if (unemap_get_number_of_samples_acquired(&number_of_samples))
		{
			if (0==channel_number)
			{
				if (unemap_get_number_of_channels(&number_of_channels))
				{
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"unemap_write_samples_acquired.  Could not get number_of_channels");
				}
			}
			else
			{
				number_of_channels=1;
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_write_samples_acquired.  Could not get number_of_samples");
		}
		if (return_code)
		{
			if (ALLOCATE(samples,short,number_of_channels*number_of_samples))
			{
				if (unemap_get_samples_acquired(channel_number,samples))
				{
					fwrite((char *)&channel_number,sizeof(channel_number),1,file);
					fwrite((char *)&number_of_channels,sizeof(number_of_channels),1,file);
					fwrite((char *)&number_of_samples,sizeof(number_of_samples),1,file);
					fwrite((char *)samples,sizeof(short int),
						number_of_channels*number_of_samples,file);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"unemap_write_samples_acquired.  Could not get samples");
				}
				DEALLOCATE(samples);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_write_samples_acquired.  Could not allocate samples");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_write_samples_acquired.  Invalid argument(s).  %p %d",
			file,channel_number);
	}
	LEAVE;

	return (return_code);
} /* unemap_write_samples_acquired */

int unemap_get_samples_acquired(int channel_number,short int *samples)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is valid (between 1 and the total number of channels
inclusive), then the <samples> for that channel are returned.  If
<channel_number> is 0 then the <samples> for all channels are returned.
Otherwise the function fails.
==============================================================================*/
{
	int i,crate_channel_number,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_get_samples_acquired);
#if defined (DEBUG)
	/*???debug */
	printf("enter unemap_get_samples_acquired\n");
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if (samples)
	{
		/* unemap may have been started by another process, so can't just check that
			the command_socket is valid */
		if (initialize_connection()&&(crate=module_unemap_crates)&&
			(0<module_number_of_unemap_crates))
		{
			i=module_number_of_unemap_crates;
			crate_channel_number=channel_number;
			if (0==channel_number)
			{
				while ((i>0)&&(return_code=crate_get_samples_acquired(
					crate,channel_number,samples+crate_channel_number)))
				{
					crate_channel_number += crate->number_of_channels;
					crate++;
					i--;
				}
			}
			else
			{
				while ((i>0)&&(crate_channel_number>crate->number_of_channels))
				{
					crate_channel_number -= crate->number_of_channels;
					crate++;
					i--;
				}
				if ((i>0)&&crate)
				{
					return_code=crate_get_samples_acquired(crate,crate_channel_number,
						samples);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_samples_acquired.  Missing samples");
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave unemap_get_samples_acquired\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_get_samples_acquired */

int unemap_get_samples_acquired_background(int channel_number,
	Acquired_data_callback *callback,void *user_data)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
The function fails if the hardware is not configured.

The function gets the samples specified by the <channel_number> and calls the
<callback> with the <channel_number>, the number of samples, the samples and the
<user_data>.

When the function returns, it is safe to call any of the other functions
(including unemap_start_sampling), but the <callback> may not have finished or
even been called yet.  This function allows data to be transferred in the
background in a client/server arrangement.

???DB.  Saving a unemap signal file in background
1 Pop up dialog and get file name
2 Call unemap_get_samples_acquired_background
2.1 Sends message to server to unemap_get_samples_acquired_background
2.2 Server writes the data to disk
2.3 Server starts sub-process than opens socket and starts listening
2.4 Server sends back port number to client
2.5 Client starts sub-process that connects to server and transfers data
==============================================================================*/
{
	int crate_channel_number,i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_get_samples_acquired_background);
	return_code=0;
	/* check arguments */
	if (callback)
	{
		/* unemap may have been started by another process, so can't just check that
			the command_socket is valid */
		if (initialize_connection()&&(crate=module_unemap_crates)&&
			(0<module_number_of_unemap_crates))
		{
			if (!module_acquired_callback)
			{
				module_acquired_callback=callback;
				module_acquired_callback_data=user_data;
				module_acquired_channel_number=channel_number;
				if (0==channel_number)
				{
					for (i=module_number_of_unemap_crates;i>0;i--)
					{
						(crate->acquired).complete=0;
						crate++;
					}
					i=module_number_of_unemap_crates;
					crate=module_unemap_crates;
					while ((i>0)&&(return_code=
						crate_get_samples_acquired_background_start(crate,0)))
					{
						crate++;
						i--;
					}
					if (return_code)
					{
						i=module_number_of_unemap_crates;
						crate=module_unemap_crates;
						while ((i>0)&&(return_code=
							crate_get_samples_acquired_background_end(crate)))
						{
							crate++;
							i--;
						}
					}
					else
					{
						while (i<module_number_of_unemap_crates)
						{
							i++;
							crate--;
							crate_get_samples_acquired_background_end(crate);
						}
					}
				}
				else
				{
					for (i=module_number_of_unemap_crates;i>0;i--)
					{
						(crate->acquired).complete=1;
						crate++;
					}
					i=module_number_of_unemap_crates;
					crate=module_unemap_crates;
					crate_channel_number=channel_number;
					while ((i>0)&&(crate_channel_number>crate->number_of_channels))
					{
						crate_channel_number -= crate->number_of_channels;
						crate++;
						i--;
					}
					if ((i>0)&&crate)
					{
						(crate->acquired).complete=0;
						if (return_code=crate_get_samples_acquired_background_start(crate,
							crate_channel_number))
						{
							return_code=crate_get_samples_acquired_background_end(crate);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
					"unemap_get_samples_acquired_background.  Invalid channel_number %d",
							channel_number);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_samples_acquired_background.  Transfer in progress");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_samples_acquired_background.  Missing callback");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_samples_acquired_background */

#if defined (OLD_CODE)
int unemap_get_samples_acquired_background(int channel_number,
	Acquired_data_callback *callback,void *user_data)
/*******************************************************************************
LAST MODIFIED : 2 July 2000

DESCRIPTION :
The function fails if the hardware is not configured.

The function gets the samples specified by the <channel_number> and calls the
<callback> with the <channel_number>, the number of samples, the samples and the
<user_data>.

When the function returns, it is safe to call any of the other functions
(including unemap_start_sampling), but the <callback> may not have finished or
even been called yet.  This function allows data to be transferred in the
background in a client/server arrangement.

???DB.  Saving a unemap signal file in background
1 Pop up dialog and get file name
2 Call unemap_get_samples_acquired_background
2.1 Sends message to server to unemap_get_samples_acquired_background
2.2 Server writes the data to disk
2.3 Server starts sub-process than opens socket and starts listening
2.4 Server sends back port number to client
2.5 Client starts sub-process that connects to server and transfers data
==============================================================================*/
{
	int crate_channel_number,i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_get_samples_acquired_background);
	return_code=0;
	/* check arguments */
	if (callback)
	{
		/* unemap may have been started by another process, so can't just check that
			the command_socket is valid */
		if (initialize_connection()&&(crate=module_unemap_crates)&&
			(0<module_number_of_unemap_crates))
		{
			if (!module_acquired_callback)
			{
				module_acquired_callback=callback;
				module_acquired_callback_data=user_data;
				module_acquired_channel_number=channel_number;
				if (0==channel_number)
				{
					for (i=module_number_of_unemap_crates;i>0;i--)
					{
						(crate->acquired).complete=0;
						crate++;
					}
					i=module_number_of_unemap_crates;
					crate=module_unemap_crates;
					while ((i>0)&&(return_code=crate_get_samples_acquired_background(
						crate,0)))
					{
						crate++;
						i--;
					}
				}
				else
				{
					for (i=module_number_of_unemap_crates;i>0;i--)
					{
						(crate->acquired).complete=1;
						crate++;
					}
					i=module_number_of_unemap_crates;
					crate=module_unemap_crates;
					crate_channel_number=channel_number;
					while ((i>0)&&(crate_channel_number>crate->number_of_channels))
					{
						crate_channel_number -= crate->number_of_channels;
						crate++;
						i--;
					}
					if ((i>0)&&crate)
					{
						(crate->acquired).complete=0;
						return_code=crate_get_samples_acquired_background(crate,
							crate_channel_number);
					}
					else
					{
						display_message(ERROR_MESSAGE,
					"unemap_get_samples_acquired_background.  Invalid channel_number %d",
							channel_number);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_samples_acquired_background.  Transfer in progress");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_samples_acquired_background.  Missing callback");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_samples_acquired_background */
#endif /* defined (OLD_CODE) */

int unemap_get_maximum_number_of_samples(unsigned long *number_of_samples)
/*******************************************************************************
LAST MODIFIED : 6 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

The size of the rolling buffer, in number of samples per channel, is assigned to
<*number_of_samples>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_maximum_number_of_samples);
	return_code=0;
	/* check arguments */
	if (number_of_samples)
	{
		if (initialize_connection())
		{
			/* unemap_configure makes sure that all crates have the same sampling
				frequency */
			return_code=crate_get_maximum_number_of_samples(module_unemap_crates,
				number_of_samples);
		}
		else
		{
			display_message(ERROR_MESSAGE,
			"unemap_get_maximum_number_of_samples.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_maximum_number_of_samples.  Missing number_of_channels");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_maximum_number_of_samples */

int unemap_get_sampling_frequency(float *frequency)
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

The sampling frequency is assigned to <*frequency>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_sampling_frequency);
	return_code=0;
	/* check arguments */
	if (frequency)
	{
		if (initialize_connection())
		{
			/* unemap_configure makes sure that all crates have the same sampling
				frequency */
			return_code=crate_get_sampling_frequency(module_unemap_crates,frequency);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_sampling_frequency.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_sampling_frequency.  Missing frequency");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_sampling_frequency */

int unemap_set_gain(int channel_number,float pre_filter_gain,
	float post_filter_gain)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Sets the gain before the band pass filter and the gain after the band pass
filter to the specified values.

For UNEMAP_1V1, there is no gain before the filter (<pre_filter_gain> ignored).
For UNEMAP_2V1 and UNEMAP_2V2, the gain before the filter can be 1, 2, 4 or 8.

For UNEMAP_1V1, the post filter gain can be 10, 20, 50, 100, 200, 500 or 1000
(fixed gain of 10)
For UNEMAP_2V1 and UNEMAP_2V2, the post filter gain can be 11, 22, 55, 110, 220,
550 or 1100 (fixed gain of 11).
==============================================================================*/
{
	int crate_channel_number,i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_set_gain);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		if (0==channel_number)
		{
			return_code=1;
			for (i=module_number_of_unemap_crates;i>0;i--)
			{
				if (!crate_set_gain(crate,channel_number,pre_filter_gain,
					post_filter_gain))
				{
					return_code=0;
				}
				crate++;
			}
		}
		else
		{
			i=module_number_of_unemap_crates;
			crate_channel_number=channel_number;
			while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
			{
				crate_channel_number -= crate->number_of_channels;
				crate++;
				i--;
			}
			if ((i>0)&&crate)
			{
				return_code=crate_set_gain(crate,crate_channel_number,pre_filter_gain,
					post_filter_gain);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_gain.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_set_gain */

int unemap_get_gain(int channel_number,float *pre_filter_gain,
	float *post_filter_gain)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

The <*pre_filter_gain> and <*post_filter_gain> for the group are assigned.
==============================================================================*/
{
	int crate_channel_number,i,return_code;
#if defined (CACHE_CLIENT_INFORMATION)
	struct Unemap_card *unemap_card;
#endif /* defined (CACHE_CLIENT_INFORMATION) */
	struct Unemap_crate *crate;

	ENTER(unemap_get_gain);
	return_code=0;
	/* check arguments */
	if (pre_filter_gain&&post_filter_gain)
	{
		if (initialize_connection())
		{
			crate=module_unemap_crates;
			i=module_number_of_unemap_crates;
			crate_channel_number=channel_number;
			while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
			{
				crate_channel_number -= crate->number_of_channels;
				crate++;
				i--;
			}
			if ((i>0)&&crate)
			{
#if defined (CACHE_CLIENT_INFORMATION)
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"module_force_connection=%d\n",module_force_connection);
#endif /* defined (DEBUG) */
				/*???DB.  Eventually, it will fail if module_unemap_cards is not set ?*/
				if (!module_force_connection&&get_cache_information())
				{
					if ((1<=crate_channel_number)&&
						(crate_channel_number<=crate->number_of_channels))
					{
						unemap_card=(crate->unemap_cards)+
							((crate_channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
						*pre_filter_gain=unemap_card->pre_filter_gain;
						*post_filter_gain=unemap_card->post_filter_gain;
						return_code=1;
					}
				}
				else
				{
#endif /* defined (CACHE_CLIENT_INFORMATION) */
					if (return_code=crate_get_gain_start(crate,crate_channel_number))
					{
						return_code=crate_get_gain_end(crate,pre_filter_gain,
							post_filter_gain);
					}
#if defined (CACHE_CLIENT_INFORMATION)
				}
#endif /* defined (CACHE_CLIENT_INFORMATION) */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_gain.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_gain.  Missing pre_filter_gain or post_filter_gain");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_gain */

#if defined (OLD_CODE)
int unemap_get_gain(int channel_number,float *pre_filter_gain,
	float *post_filter_gain)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

The <*pre_filter_gain> and <*post_filter_gain> for the group are assigned.
==============================================================================*/
{
	int crate_channel_number,i,return_code;
#if defined (CACHE_CLIENT_INFORMATION)
	struct Unemap_card *unemap_card;
#endif /* defined (CACHE_CLIENT_INFORMATION) */
	struct Unemap_crate *crate;

	ENTER(unemap_get_gain);
	return_code=0;
	/* check arguments */
	if (pre_filter_gain&&post_filter_gain)
	{
		if (initialize_connection())
		{
			crate=module_unemap_crates;
			i=module_number_of_unemap_crates;
			crate_channel_number=channel_number;
			while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
			{
				crate_channel_number -= crate->number_of_channels;
				crate++;
				i--;
			}
			if ((i>0)&&crate)
			{
#if defined (CACHE_CLIENT_INFORMATION)
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"module_force_connection=%d\n",module_force_connection);
#endif /* defined (DEBUG) */
				/*???DB.  Eventually, it will fail if module_unemap_cards is not set ?*/
				if (!module_force_connection&&get_cache_information())
				{
					if ((1<=crate_channel_number)&&
						(crate_channel_number<=crate->number_of_channels))
					{
						unemap_card=(crate->unemap_cards)+
							((crate_channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
						*pre_filter_gain=unemap_card->pre_filter_gain;
						*post_filter_gain=unemap_card->post_filter_gain;
						return_code=1;
					}
				}
				else
				{
#endif /* defined (CACHE_CLIENT_INFORMATION) */
					return_code=crate_get_gain(crate,crate_channel_number,pre_filter_gain,
						post_filter_gain);
#if defined (CACHE_CLIENT_INFORMATION)
				}
#endif /* defined (CACHE_CLIENT_INFORMATION) */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_gain.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_gain.  Missing pre_filter_gain or post_filter_gain");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_gain */
#endif /* defined (OLD_CODE) */

int unemap_load_voltage_stimulating(int number_of_channels,int *channel_numbers,
	int number_of_voltages,float voltages_per_second,float *voltages)
/*******************************************************************************
LAST MODIFIED : 2 July 2000

DESCRIPTION :
The function fails if the hardware is not configured.

If <number_of_channels> is greater than 0 then the function applies to the group
that is the union of ((channel_number-1) div 64)*64+1 to
((channel_number-1) div 64)*64+64 for all the <channel_numbers>.  If
<number_of_channels> is 0 then the function applies to the group of all
channels.  Otherwise, the function fails.

Sets all the stimulating channels in the group to voltage stimulating and loads
the stimulating signal.  If <number_of_voltages> is 0 then the stimulating
signal is the zero signal.  If <number_of_voltages> is 1 then the stimulating
signal is constant at the <*voltages>.  If <number_of_voltages> is >1 then the
stimulating signal is that in <voltages> at the specified number of
<voltages_per_second>.

The <voltages> are those desired (in volts).  The function sets <voltages> to
the actual values used.

Use unemap_set_channel_stimulating to make a channel into a stimulating channel.
Use <unemap_start_stimulating> to start the stimulating.
==============================================================================*/
{
	int all_channels,*channel_number,crate_channel_number,*crate_channel_numbers,
		crate_number_of_channels,i,j,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_load_voltage_stimulating);
	return_code=0;
	/* check arguments */
	all_channels=0;
	if (0==number_of_channels)
	{
		return_code=1;
		all_channels=1;
	}
	else
	{
		if ((0<number_of_channels)&&channel_numbers)
		{
			return_code=1;
			i=number_of_channels;
			channel_number=channel_numbers;
			while (return_code&&(i>0))
			{
				if ((*channel_number>=0)&&(*channel_number<=module_number_of_channels))
				{
					if (0== *channel_number)
					{
						all_channels=1;
					}
					i--;
					channel_number++;
				}
				else
				{
					return_code=0;
				}
			}
		}
	}
	if (return_code&&((0==number_of_voltages)||
		((1==number_of_voltages)&&voltages)||((1<number_of_voltages)&&voltages&&
		(0<voltages_per_second))))
	{
		return_code=0;
		if (initialize_connection()&&(crate=module_unemap_crates)&&
			(0<module_number_of_unemap_crates))
		{
			if (all_channels)
			{
				return_code=1;
				for (i=module_number_of_unemap_crates;i>0;i--)
				{
					if (!crate_load_voltage_stimulating(crate,0,(int *)NULL,
						number_of_voltages,voltages_per_second,voltages))
					{
						return_code=0;
					}
					crate++;
				}
			}
			else
			{
				if (ALLOCATE(crate_channel_numbers,int,number_of_channels))
				{
					return_code=1;
					crate_channel_number=0;
					for (i=module_number_of_unemap_crates;i>0;i--)
					{
						crate_number_of_channels=0;
						for (j=0;j<number_of_channels;j++)
						{
							if ((crate_channel_number<channel_numbers[j])&&
								(channel_numbers[j]<=crate_channel_number+
								(crate->number_of_channels)))
							{
								crate_channel_numbers[crate_number_of_channels]=
									channel_numbers[j]-crate_channel_number;
								crate_number_of_channels++;
							}
						}
						if (0<crate_number_of_channels)
						{
							if (!crate_load_voltage_stimulating(crate,
								crate_number_of_channels,crate_channel_numbers,
								number_of_voltages,voltages_per_second,voltages))
							{
								return_code=0;
							}
						}
						crate_channel_number += crate->number_of_channels;
						crate++;
					}
					DEALLOCATE(crate_channel_numbers);
				}
				else
				{
					display_message(ERROR_MESSAGE,
	"unemap_load_voltage_stimulating.  Could not allocate crate_channel_numbers");
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_load_voltage_stimulating.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"unemap_load_voltage_stimulating.  Invalid number_of_channels (%d) or channel_numbers (%p) or number_of_voltages (%d) or voltages (%p)",
			number_of_channels,channel_numbers,number_of_voltages,voltages);
	}
	LEAVE;

	return (return_code);
} /* unemap_load_voltage_stimulating */

int unemap_load_current_stimulating(int number_of_channels,int *channel_numbers,
	int number_of_currents,float currents_per_second,float *currents)
/*******************************************************************************
LAST MODIFIED : 2 July 2000

DESCRIPTION :
The function fails if the hardware is not configured.

If <number_of_channels> is greater than 0 then the function applies to the group
that is the union of ((channel_number-1) div 64)*64+1 to
((channel_number-1) div 64)*64+64 for all the <channel_numbers>.  If
<number_of_channels> is 0 then the function applies to the group of all
channels.  Otherwise, the function fails.

Sets all the stimulating channels in the group to current stimulating and loads
the stimulating signal.  If <number_of_currents> is 0 then the stimulating
signal is the zero signal.  If <number_of_currents> is 1 then the stimulating
signal is constant at the <*currents>.  If <number_of_currents> is >1 then the
stimulating signal is that in <currents> at the specified number of
<currents_per_second>.

The <currents> are those desired as a proportion of the maximum (dependent on
the impedance being driven).  The function sets <currents> to the actual values
used.

Use unemap_set_channel_stimulating to make a channel into a stimulating channel.
Use <unemap_start_stimulating> to start the stimulating.
==============================================================================*/
{
	int all_channels,*channel_number,crate_channel_number,*crate_channel_numbers,
		crate_number_of_channels,i,j,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_load_current_stimulating);
	return_code=0;
	/* check arguments */
	all_channels=0;
	if (0==number_of_channels)
	{
		return_code=1;
		all_channels=1;
	}
	else
	{
		if ((0<number_of_channels)&&channel_numbers)
		{
			return_code=1;
			i=number_of_channels;
			channel_number=channel_numbers;
			while (return_code&&(i>0))
			{
				if ((*channel_number>=0)&&(*channel_number<=module_number_of_channels))
				{
					if (0== *channel_number)
					{
						all_channels=1;
					}
					i--;
					channel_number++;
				}
				else
				{
					return_code=0;
				}
			}
		}
	}
	if (return_code&&((0==number_of_currents)||
		((1==number_of_currents)&&currents)||((1<number_of_currents)&&currents&&
		(0<currents_per_second))))
	{
		return_code=0;
		if (initialize_connection()&&(crate=module_unemap_crates)&&
			(0<module_number_of_unemap_crates))
		{
			if (all_channels)
			{
				return_code=1;
				for (i=module_number_of_unemap_crates;i>0;i--)
				{
					if (!crate_load_current_stimulating(crate,0,(int *)NULL,
						number_of_currents,currents_per_second,currents))
					{
						return_code=0;
					}
					crate++;
				}
			}
			else
			{
				if (ALLOCATE(crate_channel_numbers,int,number_of_channels))
				{
					return_code=1;
					crate_channel_number=0;
					for (i=module_number_of_unemap_crates;i>0;i--)
					{
						crate_number_of_channels=0;
						for (j=0;j<number_of_channels;j++)
						{
							if ((crate_channel_number<channel_numbers[j])&&
								(channel_numbers[j]<=crate_channel_number+
								(crate->number_of_channels)))
							{
								crate_channel_numbers[crate_number_of_channels]=
									channel_numbers[j]-crate_channel_number;
								crate_number_of_channels++;
							}
						}
						if (0<crate_number_of_channels)
						{
							if (!crate_load_current_stimulating(crate,
								crate_number_of_channels,crate_channel_numbers,
								number_of_currents,currents_per_second,currents))
							{
								return_code=0;
							}
						}
						crate_channel_number += crate->number_of_channels;
						crate++;
					}
					DEALLOCATE(crate_channel_numbers);
				}
				else
				{
					display_message(ERROR_MESSAGE,
	"unemap_load_current_stimulating.  Could not allocate crate_channel_numbers");
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_load_current_stimulating.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"unemap_load_current_stimulating.  Invalid number_of_channels (%d) or channel_numbers (%p) or number_of_currents (%d) or currents (%p)",
			number_of_channels,channel_numbers,number_of_currents,currents);
	}
	LEAVE;

	return (return_code);
} /* unemap_load_current_stimulating */

int unemap_start_stimulating(void)
/*******************************************************************************
LAST MODIFIED : 2 July 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Starts stimulation for all channels that have been loaded (with
unemap_load_voltage_stimulating or unemap_load_current_stimulating) and have not
yet started.
==============================================================================*/
{
	int i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_start_stimulating);
	return_code=0;
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		return_code=1;
		for (i=module_number_of_unemap_crates;i>0;i--)
		{
			if (!crate_start_stimulating(crate))
			{
				return_code=0;
			}
			crate++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_start_voltage_stimulating.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_start_stimulating */

#if defined (OLD_CODE)
int unemap_start_voltage_stimulating(int channel_number,int number_of_voltages,
	float voltages_per_second,float *voltages)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Starts stimulating down the channels that are in the group and have been
specified as stimulating with <unemap_set_channel_stimulating>.  If
<number_of_voltages> is 0 then the stimulating signal is the zero signal.  If
<number_of_voltages> is 1 then the stimulating signal is constant at the
<*voltages>.  If <number_of_voltages> is >1 then the stimulating signal is that
in <voltages> at the specified number of <voltages_per_second>.

The <voltages> are those desired (in volts).  The function sets <voltages> to
the actual values used.
==============================================================================*/
{
	int crate_channel_number,i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_start_voltage_stimulating);
	return_code=0;
	if ((0==number_of_voltages)||((0<number_of_voltages)&&voltages))
	{
		if (initialize_connection()&&(crate=module_unemap_crates)&&
			(0<module_number_of_unemap_crates))
		{
			if (0==channel_number)
			{
				return_code=1;
				for (i=module_number_of_unemap_crates;i>0;i--)
				{
					if (!crate_start_voltage_stimulating(crate,channel_number,
						number_of_voltages,voltages_per_second,voltages))
					{
						return_code=0;
					}
					crate++;
				}
			}
			else
			{
				i=module_number_of_unemap_crates;
				crate_channel_number=channel_number;
				while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
				{
					crate_channel_number -= crate->number_of_channels;
					crate++;
					i--;
				}
				if ((i>0)&&crate)
				{
					return_code=crate_start_voltage_stimulating(crate,
						crate_channel_number,number_of_voltages,voltages_per_second,
						voltages);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_start_voltage_stimulating.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"unemap_start_voltage_stimulating.  Invalid number_of_voltages (%d) or voltages (%p)",
			number_of_voltages,voltages);
	}
	LEAVE;

	return (return_code);
} /* unemap_start_voltage_stimulating */

int unemap_start_current_stimulating(int channel_number,int number_of_currents,
	float currents_per_second,float *currents)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Starts stimulating down the channels that are in the group and have been
specified as stimulating with <unemap_set_channel_stimulating>.  If
<number_of_currents> is 0 then the stimulating signal is the zero signal.  If
<number_of_currents> is 1 then the stimulating signal is constant at the
<*currents>.  If <number_of_currents> is >1 then the stimulating signal is that
in <currents> at the specified number of <currents_per_second>.

The <currents> are those desired as a proportion of the maximum (dependent on
the impedance being driven).  The function sets <currents> to the actual values
used.
==============================================================================*/
{
	int crate_channel_number,i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_start_current_stimulating);
	return_code=0;
	if ((0==number_of_currents)||((0<number_of_currents)&&currents))
	{
		if (initialize_connection()&&(crate=module_unemap_crates)&&
			(0<module_number_of_unemap_crates))
		{
			if (0==channel_number)
			{
				return_code=1;
				for (i=module_number_of_unemap_crates;i>0;i--)
				{
					if (!crate_start_current_stimulating(crate,channel_number,
						number_of_currents,currents_per_second,currents))
					{
						return_code=0;
					}
					crate++;
				}
			}
			else
			{
				i=module_number_of_unemap_crates;
				crate_channel_number=channel_number;
				while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
				{
					crate_channel_number -= crate->number_of_channels;
					crate++;
					i--;
				}
				if ((i>0)&&crate)
				{
					return_code=crate_start_current_stimulating(crate,
						crate_channel_number,number_of_currents,currents_per_second,
						currents);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_start_current_stimulating.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"unemap_start_current_stimulating.  Invalid number_of_currents (%d) or currents (%p)",
			number_of_currents,currents);
	}
	LEAVE;

	return (return_code);
} /* unemap_start_current_stimulating */
#endif /* defined (OLD_CODE) */

int unemap_stop_stimulating(int channel_number)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Stops stimulating for the channels in the group.
==============================================================================*/
{
	int crate_channel_number,i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_stop_stimulating);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		if (0==channel_number)
		{
			return_code=1;
			for (i=module_number_of_unemap_crates;i>0;i--)
			{
				if (!crate_stop_stimulating(crate,channel_number))
				{
					return_code=0;
				}
				crate++;
			}
		}
		else
		{
			i=module_number_of_unemap_crates;
			crate_channel_number=channel_number;
			while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
			{
				crate_channel_number -= crate->number_of_channels;
				crate++;
				i--;
			}
			if ((i>0)&&crate)
			{
				return_code=crate_stop_stimulating(crate,channel_number);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_stop_stimulating.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_stop_stimulating */

int unemap_set_channel_stimulating(int channel_number,int stimulating)
/*******************************************************************************
LAST MODIFIED : 7 July 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Zero <stimulating> means off.  Non-zero <stimulating> means on.  If
<channel_number> is valid (between 1 and the total number of channels
inclusive), then <channel_number> is set to <stimulating>.  If <channel_number>
is 0, then all channels are set to <stimulating>.  Otherwise the function fails.
==============================================================================*/
{
	int crate_channel_number,i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_set_channel_stimulating);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		if (0==channel_number)
		{
			return_code=1;
			for (i=module_number_of_unemap_crates;i>0;i--)
			{
				if (!crate_set_channel_stimulating(crate,channel_number,stimulating))
				{
					return_code=0;
				}
				crate++;
			}
		}
		else
		{
			i=module_number_of_unemap_crates;
			crate_channel_number=channel_number;
			while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
			{
				crate_channel_number -= crate->number_of_channels;
				crate++;
				i--;
			}
			if ((i>0)&&crate)
			{
				return_code=crate_set_channel_stimulating(crate,crate_channel_number,
					stimulating);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_channel_stimulating.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_set_channel_stimulating */

int unemap_get_channel_stimulating(int channel_number,int *stimulating)
/*******************************************************************************
LAST MODIFIED : 7 July 2000

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is valid (between 1 and the total number of channels
inclusive), then <stimulating> is set to 1 if <channel_number> is stimulating
and 0 otherwise.  Otherwise the function fails.
==============================================================================*/
{
	int crate_channel_number,i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_get_channel_stimulating);
	return_code=0;
	/* check arguments */
	if (stimulating)
	{
		if (initialize_connection()&&(crate=module_unemap_crates)&&
			(0<module_number_of_unemap_crates))
		{
			i=module_number_of_unemap_crates;
			crate_channel_number=channel_number;
			while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
			{
				crate_channel_number -= crate->number_of_channels;
				crate++;
				i--;
			}
			if ((i>0)&&crate)
			{
				return_code=crate_get_channel_stimulating(crate,crate_channel_number,
					stimulating);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_channel_stimulating.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_channel_stimulating.  Missing number_of_channels");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_channel_stimulating */

int unemap_start_calibrating(int channel_number,int number_of_voltages,
	float voltages_per_second,float *voltages)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Starts generating the calibration signal for the specified group.  If
<number_of_voltages> is 0 then the calibration signal is the zero signal.  If
<number_of_voltages> is 1 then the calibration signal is constant at the
<*voltages>.  If <number_of_voltages> is >1 then the calibration signal is that
in <voltages> at the specified number of <voltages_per_second>.

The <voltages> are those desired (in volts).  The function sets <voltages> to
the actual values used.
==============================================================================*/
{
	int crate_channel_number,i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_start_calibrating);
	return_code=0;
	if ((0==number_of_voltages)||((0<number_of_voltages)&&voltages))
	{
		if (initialize_connection()&&(crate=module_unemap_crates)&&
			(0<module_number_of_unemap_crates))
		{
			if (0==channel_number)
			{
				return_code=1;
				for (i=module_number_of_unemap_crates;i>0;i--)
				{
					if (!crate_start_calibrating(crate,channel_number,number_of_voltages,
						voltages_per_second,voltages))
					{
						return_code=0;
					}
					crate++;
				}
			}
			else
			{
				i=module_number_of_unemap_crates;
				crate_channel_number=channel_number;
				while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
				{
					crate_channel_number -= crate->number_of_channels;
					crate++;
					i--;
				}
				if ((i>0)&&crate)
				{
					return_code=crate_start_calibrating(crate,channel_number,
						number_of_voltages,voltages_per_second,voltages);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_start_calibrating.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
	"unemap_start_calibrating.  Invalid number_of_voltages (%d) or voltages (%p)",
			number_of_voltages,voltages);
	}
	LEAVE;

	return (return_code);
} /* unemap_start_calibrating */

int unemap_stop_calibrating(int channel_number)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Stops generating the calibration signal for the channels in the group.
==============================================================================*/
{
	int crate_channel_number,i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_stop_calibrating);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		if (0==channel_number)
		{
			return_code=1;
			for (i=module_number_of_unemap_crates;i>0;i--)
			{
				if (!crate_stop_calibrating(crate,channel_number))
				{
					return_code=0;
				}
				crate++;
			}
		}
		else
		{
			i=module_number_of_unemap_crates;
			crate_channel_number=channel_number;
			while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
			{
				crate_channel_number -= crate->number_of_channels;
				crate++;
				i--;
			}
			if ((i>0)&&crate)
			{
				return_code=crate_stop_calibrating(crate,channel_number);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_stop_calibrating.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_stop_calibrating */

int unemap_set_power(int on)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
The function does not need the hardware to be configured.

If <on> is zero the hardware is powered off, otherwise the hardware is powered
on.
==============================================================================*/
{
	int i,return_code,step;
	struct Unemap_crate *crate;

	ENTER(unemap_set_power);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		return_code=1;
		if (on)
		{
			step=1;
		}
		else
		{
			crate += module_number_of_unemap_crates-1;
			step= -1;
		}
		for (i=module_number_of_unemap_crates;i>0;i--)
		{
			if (!crate_set_power_start(crate,on))
			{
				return_code=0;
			}
			crate += step;
		}
		crate=module_unemap_crates;
		if (!on)
		{
			crate += module_number_of_unemap_crates-1;
		}
		for (i=module_number_of_unemap_crates;i>0;i--)
		{
			if (!crate_set_power_end(crate))
			{
				return_code=0;
			}
			crate += step;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_power.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_set_power */

#if defined (OLD_CODE)
int unemap_set_power(int on)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

If <on> is zero the hardware is powered off, otherwise the hardware is powered
on.
==============================================================================*/
{
	int i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_set_power);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		return_code=1;
		if (on)
		{
			for (i=module_number_of_unemap_crates;i>0;i--)
			{
				if (!crate_set_power(crate,on))
				{
					return_code=0;
				}
				crate++;
			}
		}
		else
		{
			crate += module_number_of_unemap_crates;
			for (i=module_number_of_unemap_crates;i>0;i--)
			{
				crate--;
				if (!crate_set_power(crate,on))
				{
					return_code=0;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_power.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_set_power */
#endif /* defined (OLD_CODE) */

int unemap_get_power(int *on)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

If the hardware power is on then <*on> is set to 1, otherwise <*on> is set to 0.
==============================================================================*/
{
	int crate_on,i,return_code,temp_on;
	struct Unemap_crate *crate;

	ENTER(unemap_get_power);
	return_code=0;
	/* check arguments */
	if (on)
	{
		/* unemap may have been started by another process, so can't just check that
			the command_socket is valid */
		if (initialize_connection()&&(crate=module_unemap_crates)&&
			(0<(i=module_number_of_unemap_crates)))
		{
			return_code=crate_get_power(crate,&crate_on);
			temp_on=crate_on;
			i--;
			while (return_code&&(i>0)&&(crate_on==temp_on))
			{
				crate++;
				i--;
				return_code=crate_get_power(crate,&crate_on);
			}
			if (return_code&&(i==0)&&(crate_on==temp_on))
			{
				*on=temp_on;
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_power.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"unemap_get_power.  Missing on");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_power */

int unemap_read_waveform_file(char *waveform_file_name,int *number_of_values,
	float *values_per_second,float **values,int *constant_voltage)
/*******************************************************************************
LAST MODIFIED : 27 May 1999

DESCRIPTION :
The function does not need the hardware to be configured.

A waveform suitable for stimulation is read from a file.  If
<*constant_voltage>, then the <*values> are voltages otherwise the <*values> are
proportions of the maximum current.
???DB.  Same as in unemap_hardware.c .  Could read on remote machine ?
==============================================================================*/
{
	int return_code;
	FILE *waveform_file;
	float *value;
	int i;

	ENTER(read_waveform_file);
	return_code=0;
	/* check arguments */
	if (waveform_file_name&&number_of_values&&values&&values_per_second&&
		constant_voltage)
	{
		if (waveform_file=fopen(waveform_file_name,"r"))
		{
			fscanf(waveform_file,"Number of ");
			if (1==fscanf(waveform_file,"voltages = %d ",
				number_of_values))
			{
				*constant_voltage=1;
				return_code=1;
			}
			else
			{
				if (1==fscanf(waveform_file,"currents = %d ",
					number_of_values))
				{
					*constant_voltage=0;
					return_code=1;
				}
			}
			if (return_code)
			{
				return_code=0;
				if (0< *number_of_values)
				{
					if (1==fscanf(waveform_file,"Frequency (pts/s) = %f ",
						values_per_second))
					{
						if (0< *values_per_second)
						{
							if (ALLOCATE(*values,float,*number_of_values))
							{
								if (*constant_voltage)
								{
									fscanf(waveform_file," Voltages ");
								}
								else
								{
									fscanf(waveform_file," Currents ");
								}
								i= *number_of_values;
								return_code=1;
								value= *values;
								while (return_code&&(i>0))
								{
									if (1==fscanf(waveform_file," %f ",value))
									{
										i--;
										value++;
									}
									else
									{
										return_code=0;
									}
								}
								if (0==return_code)
								{
									display_message(ERROR_MESSAGE,"Error reading value from %s",
										waveform_file_name);
									DEALLOCATE(*values);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
								"read_waveform_file.  Could not allocate memory for values");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Invalid frequency (%g) from %s",
								*values_per_second,waveform_file_name);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Error reading frequency from %s",
							waveform_file_name);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Invalid number of values (%d) from %s",*number_of_values,
						waveform_file_name);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Error reading number of values from %s",waveform_file_name);
			}
			fclose(waveform_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Could not open waveform file %s",
				waveform_file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"read_waveform_file.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* read_waveform_file */

int unemap_get_number_of_stimulators(int *number_of_stimulators)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
The function does not need the hardware to be configured.

The total number of hardware stimulators is assigned to
<*number_of_stimulators>.
==============================================================================*/
{
	int crate_number_of_stimulators,i,return_code,total_number_of_stimulators;
	struct Unemap_crate *crate;

	ENTER(unemap_get_number_of_stimulators);
	return_code=0;
	/* check arguments */
	if (number_of_stimulators)
	{
#if defined (CACHE_CLIENT_INFORMATION)
		/*???DB.  Eventually, it will fail of module_unemap_cards is not set ? */
		if (!module_force_connection&&get_cache_information())
		{
			*number_of_stimulators=module_number_of_stimulators;
			return_code=1;
		}
		else
		{
#endif /* defined (CACHE_CLIENT_INFORMATION) */
			if (initialize_connection())
			{
				crate=module_unemap_crates;
				i=module_number_of_unemap_crates;
				while ((i>0)&&
					(return_code=crate_get_number_of_stimulators_start(crate)))
				{
					crate++;
					i--;
				}
				if (return_code)
				{
					crate=module_unemap_crates;
					i=module_number_of_unemap_crates;
					total_number_of_stimulators=0;
					while ((i>0)&&(return_code=crate_get_number_of_stimulators_end(crate,
						&crate_number_of_stimulators)))
					{
						total_number_of_stimulators += crate_number_of_stimulators;
						crate++;
						i--;
					}
					if (return_code)
					{
						*number_of_stimulators=total_number_of_stimulators;
					}
					else
					{
						crate++;
						i--;
						while (i>0)
						{
							crate_get_number_of_stimulators_end(crate,
								&crate_number_of_stimulators);
							crate++;
							i--;
						}
					}
				}
				else
				{
					while (i<module_number_of_unemap_crates)
					{
						crate--;
						i++;
						crate_get_number_of_stimulators_end(crate,
							&crate_number_of_stimulators);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_number_of_stimulators.  Could not initialize_connection");
			}
#if defined (CACHE_CLIENT_INFORMATION)
		}
#endif /* defined (CACHE_CLIENT_INFORMATION) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_number_of_stimulators.  Missing number_of_stimulators");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_number_of_stimulators */

#if defined (OLD_CODE)
int unemap_get_number_of_stimulators(int *number_of_stimulators)
/*******************************************************************************
LAST MODIFIED : 26 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

The total number of hardware stimulators is assigned to
<*number_of_stimulators>.
==============================================================================*/
{
	int crate_number_of_stimulators,i,return_code,total_number_of_stimulators;
	struct Unemap_crate *crate;

	ENTER(unemap_get_number_of_stimulators);
	return_code=0;
	/* check arguments */
	if (number_of_stimulators)
	{
#if defined (CACHE_CLIENT_INFORMATION)
		/*???DB.  Eventually, it will fail of module_unemap_cards is not set ? */
		if (!module_force_connection&&get_cache_information())
		{
			*number_of_stimulators=module_number_of_stimulators;
			return_code=1;
		}
		else
		{
#endif /* defined (CACHE_CLIENT_INFORMATION) */
			if (initialize_connection())
			{
				crate=module_unemap_crates;
				i=module_number_of_unemap_crates;
				total_number_of_stimulators=0;
				while ((i>0)&&(return_code=crate_get_number_of_stimulators(crate,
					&crate_number_of_stimulators)))
				{
					total_number_of_stimulators += crate_number_of_stimulators;
					crate++;
					i--;
				}
				if (return_code)
				{
					*number_of_stimulators=total_number_of_stimulators;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_number_of_stimulators.  Could not initialize_connection");
			}
#if defined (CACHE_CLIENT_INFORMATION)
		}
#endif /* defined (CACHE_CLIENT_INFORMATION) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_number_of_stimulators.  Missing number_of_stimulators");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_number_of_stimulators */
#endif /* defined (OLD_CODE) */

int unemap_channel_valid_for_stimulator(int stimulator_number,
	int channel_number)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
The function does not need the hardware to be configured.

Returns non-zero if <stimulator_number> can stimulate down <channel_number>.
==============================================================================*/
{
	int crate_channel_number,crate_stimulator_number,i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_channel_valid_for_stimulator);
	return_code=0;
	/* unemap may have been started by another process, so can't just check
		that the command_socket is valid */
	if (initialize_connection())
	{
		i=module_number_of_unemap_crates;
		crate=module_unemap_crates;
		crate_channel_number=channel_number;
		crate_stimulator_number=stimulator_number;
		while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
		{
			crate_channel_number -= crate->number_of_channels;
			crate_stimulator_number -= crate->number_of_stimulators;
			i--;
			crate++;
		}
		if ((i>0)&&crate)
		{
#if defined (CACHE_CLIENT_INFORMATION)
			/*???DB.  Eventually, it will fail of module_unemap_cards is not set ? */
			if (!module_force_connection&&get_cache_information())
			{
				if ((0<crate_stimulator_number)&&
					(crate_stimulator_number<=crate->number_of_stimulators)&&
					(1<=crate_channel_number)&&
					(crate_channel_number<=crate->number_of_channels)&&
					((crate->stimulator_card_indices)[crate_stimulator_number-1]==
					(crate_channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD))
				{
					return_code=1;
				}
			}
			else
			{
#endif /* defined (CACHE_CLIENT_INFORMATION) */
				if (return_code=crate_channel_valid_for_stimulator_start(crate,
					crate_stimulator_number,crate_channel_number))
				{
					return_code=crate_channel_valid_for_stimulator_end(crate);
				}
#if defined (CACHE_CLIENT_INFORMATION)
			}
#endif /* defined (CACHE_CLIENT_INFORMATION) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_channel_valid_for_stimulator.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_channel_valid_for_stimulator */

#if defined (OLD_CODE)
int unemap_channel_valid_for_stimulator(int stimulator_number,
	int channel_number)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

Returns non-zero if <stimulator_number> can stimulate down <channel_number>.
==============================================================================*/
{
	int crate_channel_number,crate_stimulator_number,i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_channel_valid_for_stimulator);
	return_code=0;
	/* unemap may have been started by another process, so can't just check
		that the command_socket is valid */
	if (initialize_connection())
	{
		i=module_number_of_unemap_crates;
		crate=module_unemap_crates;
		crate_channel_number=channel_number;
		crate_stimulator_number=stimulator_number;
		while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
		{
			crate_channel_number -= crate->number_of_channels;
			crate_stimulator_number -= crate->number_of_stimulators;
			i--;
			crate++;
		}
		if ((i>0)&&crate)
		{
#if defined (CACHE_CLIENT_INFORMATION)
			/*???DB.  Eventually, it will fail of module_unemap_cards is not set ? */
			if (!module_force_connection&&get_cache_information())
			{
				if ((0<crate_stimulator_number)&&
					(crate_stimulator_number<=crate->number_of_stimulators)&&
					(1<=crate_channel_number)&&
					(crate_channel_number<=crate->number_of_channels)&&
					((crate->stimulator_card_indices)[crate_stimulator_number-1]==
					(crate_channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD))
				{
					return_code=1;
				}
			}
			else
			{
#endif /* defined (CACHE_CLIENT_INFORMATION) */
				return_code=crate_channel_valid_for_stimulator(crate,
					crate_stimulator_number,crate_channel_number);
#if defined (CACHE_CLIENT_INFORMATION)
			}
#endif /* defined (CACHE_CLIENT_INFORMATION) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_channel_valid_for_stimulator.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_channel_valid_for_stimulator */
#endif /* defined (OLD_CODE) */

/*
Diagnostic functions
--------------------
*/
int unemap_get_card_state(int channel_number,int *battA_state,
	int *battGood_state,float *filter_frequency,int *filter_taps,
	unsigned char shift_registers[10],int *GA0_state,int *GA1_state,
	int *NI_gain,int *input_mode,int *polarity,float *tol_settling,
	int *sampling_interval,int *settling_step_max)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Returns the current state of the signal conditioning card containing the
<channel_number>.

Intended for diagnostic use only.
==============================================================================*/
{
	int crate_channel_number,i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_get_card_state);
	return_code=0;
	/* check arguments */
	if (battA_state&&battGood_state&&filter_frequency&&filter_taps&&
		shift_registers&&GA0_state&&GA1_state&&NI_gain&&input_mode&&polarity&&
		tol_settling&&sampling_interval&&settling_step_max)
	{
		if (initialize_connection()&&(crate=module_unemap_crates)&&
			(0<module_number_of_unemap_crates))
		{
			i=module_number_of_unemap_crates;
			crate_channel_number=channel_number;
			while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
			{
				crate_channel_number -= crate->number_of_channels;
				crate++;
				i--;
			}
			if ((i>0)&&crate)
			{
				return_code=crate_get_card_state(crate,channel_number,battA_state,
					battGood_state,filter_frequency,filter_taps,shift_registers,
					GA0_state,GA1_state,NI_gain,input_mode,polarity,tol_settling,
					sampling_interval,settling_step_max);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_card_state.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"unemap_get_card_state.  Invalid argument(s).  %d %p %p %p %p %p %p %p %p %p %p %p %p %p",
			channel_number,battA_state,battGood_state,filter_frequency,filter_taps,
			shift_registers,GA0_state,GA1_state,NI_gain,input_mode,polarity,
			tol_settling,sampling_interval,settling_step_max);
	}
	LEAVE;

	return (return_code);
} /* unemap_get_card_state */

int unemap_write_card_state(int channel_number,FILE *out_file)
/*******************************************************************************
LAST MODIFIED : 5 August 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Writes the current state of the signal conditioning card containing the
<channel_number> to the specified <out_file>.

Intended for diagnostic use only.
???DB.  Should this write on the remote machine ?
==============================================================================*/
{
	float filter_frequency,tol_settling;
	int battA_state,battGood_state,filter_taps,GA0_state,GA1_state,i,input_mode,
		NI_gain,polarity,return_code,settling_step_max,sampling_interval;
	unsigned char shift_registers[10];

	ENTER(unemap_write_card_state);
	return_code=0;
	if (out_file)
	{
		if (unemap_get_card_state(channel_number,&battA_state,&battGood_state,
			&filter_frequency,&filter_taps,shift_registers,&GA0_state,&GA1_state,
			&NI_gain,&input_mode,&polarity,&tol_settling,&sampling_interval,
			&settling_step_max))
		{
			fprintf(out_file,"filter = %g\n",filter_frequency);
			fprintf(out_file,"gain = %d\n",NI_gain);
			fprintf(out_file,"input_mode = %d\n",input_mode);
			fprintf(out_file,"polarity = %d\n",polarity);
			fprintf(out_file,"GA0 setting = ");
			if (GA0_state)
			{
				fprintf(out_file,"1\n");
			}
			else
			{
				fprintf(out_file,"0\n");
			}
			fprintf(out_file,"GA1 setting = ");
			if (GA1_state)
			{
				fprintf(out_file,"1\n");
			}
			else
			{
				fprintf(out_file,"0\n");
			}
			fprintf(out_file,"shift registers = ");
			for (i=0;i<10;i++)
			{
				fprintf(out_file,"%02x",(unsigned short)(shift_registers)[i]);
			}
			fprintf(out_file,"\n");
			fprintf(out_file,"settling magnitude = %g\n",tol_settling);
			fprintf(out_file,"BattA setting = %d\n",battA_state);
			fprintf(out_file,"sampling interval = %d\n",sampling_interval);
			fprintf(out_file,"max settling steps = %d\n",settling_step_max);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_write_card_state.  unemap_get_card_state failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_write_card_state.  Missing out_file");
	}
	LEAVE;

	return (return_code);
} /* unemap_write_card_state */

int unemap_toggle_shift_register(int channel_number,int register_number)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Toggles the <shift_register> of the signal conditioning card containing the
<channel_number>.

Intended for diagnostic use only.
==============================================================================*/
{
	int crate_channel_number,i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_toggle_shift_register);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		i=module_number_of_unemap_crates;
		crate_channel_number=channel_number;
		while ((i>0)&&crate&&(crate_channel_number>crate->number_of_channels))
		{
			crate_channel_number -= crate->number_of_channels;
			crate++;
			i--;
		}
		if ((i>0)&&crate)
		{
			return_code=crate_toggle_shift_register(crate,channel_number,
				register_number);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_toggle_shift_register.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_toggle_shift_register */
