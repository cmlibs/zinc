/*******************************************************************************
FILE : unemap_hardware_client.c

LAST MODIFIED : 30 September 1999

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

/*
Module variables
----------------
*/
#if defined (CACHE_CLIENT_INFORMATION)
struct Unemap_card
/*******************************************************************************
LAST MODIFIED : 14 July 1999

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

/*
Module variables
----------------
*/
#if defined (USE_SOCKETS)
int first_invalid_socket_call=1;
#if defined (WIN32)
SOCKET calibration_socket=(SOCKET)INVALID_SOCKET;
HANDLE calibration_socket_thread_stop_event=NULL;
SOCKET command_socket=(SOCKET)INVALID_SOCKET;
SOCKET scrolling_socket=(SOCKET)INVALID_SOCKET;
HANDLE scrolling_socket_thread_stop_event=NULL;
#endif /* defined (WIN32) */
#if defined (UNIX)
int calibration_socket=(int)INVALID_SOCKET;
int command_socket=(int)INVALID_SOCKET;
int scrolling_socket=(int)INVALID_SOCKET;
#endif /* defined (UNIX) */
#if defined (MOTIF)
XtInputId calibration_socket_xid=0;
XtInputId scrolling_socket_xid=0;
#endif /* defined (MOTIF) */
#endif /* defined (USE_SOCKETS) */

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

#if defined (CACHE_CLIENT_INFORMATION)
int module_force_connection=0,module_get_cache_information_failed=0,
	module_number_of_stimulators=0,
	*module_stimulator_unemap_card_indices=(int *)NULL,
	module_number_of_unemap_cards=0;
struct Unemap_card *module_unemap_cards=(struct Unemap_card *)NULL;
unsigned long module_number_of_channels=0;
#endif /* defined (CACHE_CLIENT_INFORMATION) */

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

static int initialize_connection(void)
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
Sets up the connection with the unemap hardware service.
==============================================================================*/
{
#if defined (USE_SOCKETS)
	char *server_name,*server_file_name,*hardware_directory;
	FILE *server_file;
	int return_code,server_name_allocated,socket_type;
	struct hostent *internet_host_data;
	struct sockaddr_in server_socket;
	unsigned long internet_address;
	unsigned short port;
#if defined (WIN32)
	WORD wVersionRequested;
	WSADATA wsaData;
#endif /* defined (WIN32) */
#endif /* defined (USE_SOCKETS) */

	ENTER(initialize_connection);
#if defined (DEBUG)
	/*???debug */
	printf("enter initialize_connection %d %d\n",command_socket,
		first_invalid_socket_call);
#endif /* defined (DEBUG) */
	return_code=0;
#if defined (USE_SOCKETS)
	if (INVALID_SOCKET==command_socket)
	{
		/* only allow one attempt to open a closed connection */
		if (first_invalid_socket_call)
		{
			first_invalid_socket_call=0;
#if defined (WIN32)
			wVersionRequested=MAKEWORD(2,2);
			if (SOCKET_ERROR!=WSAStartup(wVersionRequested,&wsaData))
			{
#endif /* defined (WIN32) */
				server_name=(char *)NULL;
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
						read_string(server_file,"s",&server_name);
						fclose(server_file);
					}
					DEALLOCATE(server_file_name);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"initialize_connection.  Could not allocate server_file_name");
				}
				if (server_name)
				{
					server_name_allocated=1;
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"Using default unemap server name (%s)",DEFAULT_SERVER_NAME);
					server_name_allocated=0;
					server_name=DEFAULT_SERVER_NAME;
				}
				if (server_name)
				{
#if defined (DEBUG)
					/*???debug */
					printf("server_name %s\n",server_name);
#endif /* defined (DEBUG) */
					if (isalpha(server_name[0]))
					{
						/* server address is a name */
						internet_host_data=gethostbyname(server_name);
					}
					else
					{
						/* server address is a "." format IP address */
						internet_address=inet_addr(server_name);
						internet_host_data=gethostbyaddr((char *)&internet_address,
							sizeof(internet_address),AF_INET);
					}
					if (internet_host_data)
					{
						/* create the calibration socket.  Do this before the command socket
							because the service responds to the command socket connection */
						socket_type=DEFAULT_SOCKET_TYPE;
						calibration_socket=socket(AF_INET,socket_type,0);
						if (INVALID_SOCKET!=calibration_socket)
						{
							/* connect to the server */
							port=DEFAULT_PORT+2;
							memset(&server_socket,0,sizeof(server_socket));
								/*???DB.  Have to use memset because some implementations of
									struct sockaddr_in don't have the sin_len field */
							memcpy(&(server_socket.sin_addr),internet_host_data->h_addr,
								internet_host_data->h_length);
							server_socket.sin_family=internet_host_data->h_addrtype;
							server_socket.sin_port=htons(port);
							if (SOCKET_ERROR!=connect(calibration_socket,
								(struct sockaddr *)&server_socket,sizeof(server_socket)))
							{
								/* create the scrolling socket.  Do this before the command
									socket because the service responds to the command socket
									connection */
								socket_type=DEFAULT_SOCKET_TYPE;
								scrolling_socket=socket(AF_INET,socket_type,0);
								if (INVALID_SOCKET!=scrolling_socket)
								{
									/* connect to the server */
									port=DEFAULT_PORT+1;
									memset(&server_socket,0,sizeof(server_socket));
										/*???DB.  Have to use memset because some implementations of
											struct sockaddr_in don't have the sin_len field */
									memcpy(&(server_socket.sin_addr),internet_host_data->h_addr,
										internet_host_data->h_length);
									server_socket.sin_family=internet_host_data->h_addrtype;
									server_socket.sin_port=htons(port);
									if (SOCKET_ERROR!=connect(scrolling_socket,
										(struct sockaddr *)&server_socket,sizeof(server_socket)))
									{
										/* create the command socket */
										socket_type=DEFAULT_SOCKET_TYPE;
										command_socket=socket(AF_INET,socket_type,0);
										if (INVALID_SOCKET!=command_socket)
										{
											/* connect to the server */
											port=DEFAULT_PORT;
											memset(&server_socket,0,sizeof(server_socket));
												/*???DB.  Have to use memset because some
													implementations of struct sockaddr_in don't have the
													sin_len field */
											memcpy(&(server_socket.sin_addr),
												internet_host_data->h_addr,
												internet_host_data->h_length);
											server_socket.sin_family=internet_host_data->h_addrtype;
											server_socket.sin_port=htons(port);
											if (SOCKET_ERROR!=connect(command_socket,
												(struct sockaddr *)&server_socket,
												sizeof(server_socket)))
											{
												/*???DB.  Handshake ? */
												return_code=1;
											}
											else
											{
												display_message(ERROR_MESSAGE,
		"initialize_connection.  Could not connect command_socket.  Error code %d",
#if defined (WIN32)
													WSAGetLastError()
#endif /* defined (WIN32) */
#if defined (UNIX)
													errno
#endif /* defined (UNIX) */
													);
#if defined (WIN32)
												closesocket(calibration_socket);
												closesocket(scrolling_socket);
												closesocket(command_socket);
												WSACleanup();
#endif /* defined (WIN32) */
#if defined (UNIX)
												close(calibration_socket);
												close(scrolling_socket);
												close(command_socket);
#endif /* defined (UNIX) */
												calibration_socket=INVALID_SOCKET;
												scrolling_socket=INVALID_SOCKET;
												command_socket=INVALID_SOCKET;
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
			"initialize_connection.  Could not create command_socket.  Error code %d",
#if defined (WIN32)
												WSAGetLastError()
#endif /* defined (WIN32) */
#if defined (UNIX)
												errno
#endif /* defined (UNIX) */
												);
#if defined (WIN32)
											closesocket(calibration_socket);
											closesocket(scrolling_socket);
											WSACleanup();
#endif /* defined (WIN32) */
#if defined (UNIX)
											close(calibration_socket);
											close(scrolling_socket);
#endif /* defined (UNIX) */
											calibration_socket=INVALID_SOCKET;
											scrolling_socket=INVALID_SOCKET;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
	"initialize_connection.  Could not connect scrolling_socket.  Error code %d",
#if defined (WIN32)
											WSAGetLastError()
#endif /* defined (WIN32) */
#if defined (UNIX)
											errno
#endif /* defined (UNIX) */
											);
#if defined (WIN32)
										closesocket(calibration_socket);
										closesocket(scrolling_socket);
										WSACleanup();
#endif /* defined (WIN32) */
#if defined (UNIX)
										close(calibration_socket);
										close(scrolling_socket);
#endif /* defined (UNIX) */
										calibration_socket=INVALID_SOCKET;
										scrolling_socket=INVALID_SOCKET;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
			"initialize_connection.  Could not create scrolling_socket.  Error code %d",
#if defined (WIN32)
										WSAGetLastError()
#endif /* defined (WIN32) */
#if defined (UNIX)
										errno
#endif /* defined (UNIX) */
										);
#if defined (WIN32)
									closesocket(calibration_socket);
									WSACleanup();
#endif /* defined (WIN32) */
#if defined (UNIX)
									close(calibration_socket);
#endif /* defined (UNIX) */
									calibration_socket=INVALID_SOCKET;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
"initialize_connection.  Could not connect calibration_socket.  Error code %d",
#if defined (WIN32)
									WSAGetLastError()
#endif /* defined (WIN32) */
#if defined (UNIX)
									errno
#endif /* defined (UNIX) */
									);
#if defined (WIN32)
								closesocket(calibration_socket);
								WSACleanup();
#endif /* defined (WIN32) */
#if defined (UNIX)
								close(calibration_socket);
#endif /* defined (UNIX) */
								calibration_socket=INVALID_SOCKET;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
	"initialize_connection.  Could not create calibration_socket.  Error code %d",
#if defined (WIN32)
								WSAGetLastError()
#endif /* defined (WIN32) */
#if defined (UNIX)
								errno
#endif /* defined (UNIX) */
								);
#if defined (WIN32)
							WSACleanup();
#endif /* defined (WIN32) */
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"initialize_connection.  Could not resolve server name [%s]",
							server_name);
#if defined (WIN32)
						WSACleanup();
#endif /* defined (WIN32) */
					}
					if (server_name_allocated)
					{
						DEALLOCATE(server_name);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"initialize_connection.  Could not get server_name");
#if defined (WIN32)
					WSACleanup();
#endif /* defined (WIN32) */
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
	else
	{
		/* already initialized */
		return_code=1;
		first_invalid_socket_call=1;
	}
#endif /* defined (USE_SOCKETS) */
#if defined (DEBUG)
	/*???debug */
	printf("leave initialize_connection %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* initialize_connection */

static int close_connection(void)
/*******************************************************************************
LAST MODIFIED : 14 July 1999

DESCRIPTION :
Closes the connection with the unemap hardware service.
==============================================================================*/
{
	int return_code;

	ENTER(close_connection);
	return_code=1;
	first_invalid_socket_call=1;
	if (INVALID_SOCKET!=command_socket)
	{
#if defined (WIN32)
		closesocket(command_socket);
#endif /* defined (WIN32) */
#if defined (UNIX)
		close(command_socket);
#endif /* defined (UNIX) */
		command_socket=INVALID_SOCKET;
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
#endif /* defined (MOTIF) */
	/*???DB.  seems to need time to settle down.  Something to do with
		close_connection in service */
	sleep(1);
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
			/*???debug */
			printf("socket_send.  Connection closed 2\n");
			/*???debug */
			printf("socket_send.  Connection closed 3\n");
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
void calibration_socket_callback(
#if defined (WINDOWS)
	void
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
	XtPointer dummy_client_data,int *source,XtInputId *id
#endif /* defined (MOTIF) */
	)
/*******************************************************************************
LAST MODIFIED : 5 August 1999

DESCRIPTION :
Called when there is input on the calibration socket.
==============================================================================*/
{
	float *channel_gains,*channel_offsets;
	int *channel_numbers,number_of_channels;
	long message_size;
	unsigned char message_header[2+sizeof(long)];

	ENTER(calibration_socket_callback);
#if defined (DEBUG)
	display_message(INFORMATION_MESSAGE,"calibration_socket_callback.  %p\n",
		module_calibration_end_callback);
#endif /* defined (DEBUG) */
#if defined (MOTIF)
	USE_PARAMETER(dummy_client_data);
	USE_PARAMETER(source);
	USE_PARAMETER(id);
#endif /* defined (MOTIF) */
	/* get the header back */
	if (SOCKET_ERROR!=socket_recv(calibration_socket,message_header,
		2+sizeof(long),0))
	{
		if (message_header[0])
		{
			/* succeeded */
			memcpy(&message_size,message_header+2,sizeof(message_size));
			if (sizeof(number_of_channels)<=message_size)
			{
				if (SOCKET_ERROR!=socket_recv(calibration_socket,
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
							(SOCKET_ERROR!=socket_recv(calibration_socket,
							(unsigned char *)channel_numbers,number_of_channels*sizeof(int),
							0))&&(SOCKET_ERROR!=socket_recv(calibration_socket,
							(unsigned char *)channel_offsets,number_of_channels*sizeof(float),
							0))&&(SOCKET_ERROR!=socket_recv(calibration_socket,
							(unsigned char *)channel_gains,number_of_channels*sizeof(float),
							0))&&module_calibration_end_callback)
						{
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
						}
					}
				}
			}
		}
		else
		{
			if (module_calibration_end_callback)
			{
				(*module_calibration_end_callback)(0,(const int *)NULL,
					(const float *)NULL,(const float *)NULL,
					module_calibration_end_callback_data);
			}
		}
	}
	else
	{
		if (module_calibration_end_callback)
		{
			(*module_calibration_end_callback)(0,(const int *)NULL,
				(const float *)NULL,(const float *)NULL,
				module_calibration_end_callback_data);
		}
	}
	LEAVE;
} /* calibration_socket_callback */
#endif /* defined (USE_SOCKETS) */

#if defined (WINDOWS) && defined (USE_SOCKETS)
DWORD WINAPI calibration_thread_function(LPVOID dummy)
/*******************************************************************************
LAST MODIFIED : 31 May 1999

DESCRIPTION :
Thread to watch the calibration socket.
???DB.  Change hEvents[1] to automatic ?
==============================================================================*/
{
	DWORD dwWait,return_code;
	HANDLE hEvents[2]={NULL,NULL};
	int running;

	ENTER(calibration_thread_function);
	hEvents[0]=NULL;
	hEvents[1]=NULL;
	return_code=0;
	calibration_socket_thread_stop_event=CreateEvent(
		/*no security attributes*/NULL,/*manual reset event*/TRUE,
		/*not-signalled*/FALSE,/*no name*/NULL);
	if (calibration_socket_thread_stop_event)
	{
		hEvents[0]=calibration_socket_thread_stop_event;
		/* create the event object object use in overlapped i/o */
		hEvents[1]=CreateEvent(/*no security attributes*/NULL,
			/*manual reset event*/TRUE,/*not-signalled*/FALSE,/*no name*/NULL);
		if (hEvents[1])
		{
			if (0==WSAEventSelect(calibration_socket,hEvents[1],FD_READ))
			{
				running=1;
				while (1==running)
				{
					dwWait=WaitForMultipleObjects(2,hEvents,FALSE,INFINITE);
					if (WAIT_OBJECT_0+1==dwWait)
					{
						ResetEvent(hEvents[1]);
						calibration_socket_callback();
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
	if (INVALID_SOCKET!=calibration_socket)
	{
		closesocket(calibration_socket);
		calibration_socket=INVALID_SOCKET;
	}
	if (calibration_socket_thread_stop_event)
	{
		CloseHandle(calibration_socket_thread_stop_event);
		calibration_socket_thread_stop_event=NULL;
	}
	/* overlapped i/o event */
	{
		CloseHandle(hEvents[1]);
	}
	if (!scrolling_socket)
	{
		WSACleanup();
	}
	LEAVE;

	return (return_code);
} /* calibration_thread_function */
#endif /* defined (WINDOWS) && defined (USE_SOCKETS) */

#if defined (USE_SOCKETS)
void scrolling_socket_callback(
#if defined (WINDOWS)
	void
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
	XtPointer dummy_client_data,int *source,XtInputId *id
#endif /* defined (MOTIF) */
	)
/*******************************************************************************
LAST MODIFIED : 5 August 1999

DESCRIPTION :
Called when there is input on the scrolling socket.
==============================================================================*/
{
	int *channel_numbers,number_of_channels,number_of_values_per_channel;
	long message_size;
	short *values;
	unsigned char *byte_array,message_header[2+sizeof(long)];

	ENTER(scrolling_socket_callback);
#if defined (MOTIF)
	USE_PARAMETER(dummy_client_data);
	USE_PARAMETER(source);
	USE_PARAMETER(id);
#endif /* defined (MOTIF) */
#if defined (DEBUG)
	/*???debug */
	{
		static int number_of_scrolling_socket_callbacks=0;

		if (number_of_scrolling_socket_callbacks<5)
		{
			display_message(INFORMATION_MESSAGE,"scrolling_socket_callback.  %d %p\n",
				number_of_scrolling_socket_callbacks,module_scrolling_callback);
			number_of_scrolling_socket_callbacks++;
		}
	}
#endif /* defined (DEBUG) */
	/* get the header back */
	if (SOCKET_ERROR!=socket_recv(scrolling_socket,message_header,2+sizeof(long),
		0))
	{
		memcpy(&message_size,message_header+2,sizeof(message_size));
		if (ALLOCATE(byte_array,unsigned char,message_size))
		{
			if (SOCKET_ERROR!=socket_recv(scrolling_socket,byte_array,message_size,0))
			{
				if (module_scrolling_callback)
				{
					number_of_channels= *((int *)byte_array);
					number_of_values_per_channel=
						*((int *)(byte_array+(number_of_channels+1)*sizeof(int)));
					if ((0<number_of_channels)&&(0<number_of_values_per_channel)&&
						(message_size==(long)((number_of_channels+2)*sizeof(int)+
						number_of_channels*number_of_values_per_channel*sizeof(short))))
					{
						ALLOCATE(channel_numbers,int,number_of_channels);
						ALLOCATE(values,short,number_of_channels*
							number_of_values_per_channel);
						if (channel_numbers&&values)
						{
							memcpy((char *)channel_numbers,(char *)(byte_array+sizeof(int)),
								number_of_channels*sizeof(int));
							memcpy((char *)values,(char *)(byte_array+
								(number_of_channels+2)*sizeof(int)),number_of_channels*
								number_of_values_per_channel*sizeof(short));
							(*module_scrolling_callback)(number_of_channels,channel_numbers,
								number_of_values_per_channel,values,
								module_scrolling_callback_data);
						}
						else
						{
							DEALLOCATE(channel_numbers);
							DEALLOCATE(values);
						}
					}
				}
#if defined (WIN32)
				if (module_scrolling_window)
				{
					PostMessage(module_scrolling_window,module_scrolling_message,
						(WPARAM)byte_array,(ULONG)message_size);
				}
				else
				{
#endif /* defined (WIN32) */
					DEALLOCATE(byte_array);
#if defined (WIN32)
				}
#endif /* defined (WIN32) */
			}
			else
			{
				DEALLOCATE(byte_array);
			}
		}
#if defined (OLD_CODE)
		number_of_values=(int)(message_size/sizeof(short));
		if (ALLOCATE(values,short,number_of_values))
		{
			if ((SOCKET_ERROR!=socket_recv(scrolling_socket,(unsigned char *)values,
				message_size,0))&&(module_scrolling_callback
#if defined (WIN32)
				||module_scrolling_window
#endif /* defined (WIN32) */
				))
			{
#if defined (WIN32)
				if (module_scrolling_window)
				{
					if (module_scrolling_callback)
					{
						if (ALLOCATE(values_2,short,number_of_values))
						{
							memcpy((char *)values_2,(char *)values,number_of_values*
								sizeof(short));
#if defined (OLD_CODE)
							(*module_scrolling_callback)((HWND)NULL,(UINT)0,(LPARAM)values,
								(WPARAM)number_of_values);
#endif /* defined (OLD_CODE) */
							(*module_scrolling_callback)(values_2,number_of_values,
								module_scrolling_callback_data);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"scrolling_socket_callback.  Could not allocate values_2");
							first_error=0;
						}
					}
					PostMessage(module_scrolling_window,module_scrolling_message,
						(WPARAM)values,(ULONG)number_of_values);
				}
				else
				{
#endif /* defined (WIN32) */
#if defined (OLD_CODE)
					(*module_scrolling_callback)(
#if defined (WIN32)
						(HWND)NULL,(UINT)0,(LPARAM)values,(WPARAM)number_of_values
#endif /* defined (WIN32) */
#if defined (UNIX)
						values,number_of_values
#endif /* defined (UNIX) */
						);
#endif /* defined (OLD_CODE) */
					(*module_scrolling_callback)(values,number_of_values,
						module_scrolling_callback_data);
#if defined (WIN32)
				}
#endif /* defined (WIN32) */
			}
			else
			{
				DEALLOCATE(values);
			}
		}
#endif /* defined (OLD_CODE) */
	}
	LEAVE;
} /* scrolling_socket_callback */
#endif /* defined (USE_SOCKETS) */

#if defined (WINDOWS) && defined (USE_SOCKETS)
DWORD WINAPI scrolling_thread_function(LPVOID dummy)
/*******************************************************************************
LAST MODIFIED : 31 May 1999

DESCRIPTION :
Thread to watch the scrolling socket.
???DB.  Change hEvents[1] to automatic ?
==============================================================================*/
{
	DWORD dwWait,return_code;
	HANDLE hEvents[2]={NULL,NULL};
	int running;

	ENTER(scrolling_thread_function);
	hEvents[0]=NULL;
	hEvents[1]=NULL;
	return_code=0;
	scrolling_socket_thread_stop_event=CreateEvent(/*no security attributes*/NULL,
		/*manual reset event*/TRUE,/*not-signalled*/FALSE,/*no name*/NULL);
	if (scrolling_socket_thread_stop_event)
	{
		hEvents[0]=scrolling_socket_thread_stop_event;
		/* create the event object object use in overlapped i/o */
		hEvents[1]=CreateEvent(/*no security attributes*/NULL,
			/*manual reset event*/TRUE,/*not-signalled*/FALSE,/*no name*/NULL);
		if (hEvents[1])
		{
			if (0==WSAEventSelect(scrolling_socket,hEvents[1],FD_READ))
			{
				running=1;
				while (1==running)
				{
					dwWait=WaitForMultipleObjects(2,hEvents,FALSE,INFINITE);
					if (WAIT_OBJECT_0+1==dwWait)
					{
						ResetEvent(hEvents[1]);
						scrolling_socket_callback();
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
	if (INVALID_SOCKET!=scrolling_socket)
	{
		closesocket(scrolling_socket);
		scrolling_socket=INVALID_SOCKET;
	}
	if (scrolling_socket_thread_stop_event)
	{
		CloseHandle(scrolling_socket_thread_stop_event);
		scrolling_socket_thread_stop_event=NULL;
	}
	/* overlapped i/o event */
	{
		CloseHandle(hEvents[1]);
	}
	if (!calibration_socket)
	{
		WSACleanup();
	}
	LEAVE;

	return (return_code);
} /* scrolling_thread_function */
#endif /* defined (WINDOWS) && defined (USE_SOCKETS) */

#if defined (CACHE_CLIENT_INFORMATION)
static int get_cache_information(void)
/*******************************************************************************
LAST MODIFIED : 6 August 1999

DESCRIPTION :
Retrieves the unemap information that is cached with the client.
==============================================================================*/
{
	int channel_number,i,return_code,stimulator_number;

	ENTER(get_cache_information);
	return_code=0;
	if (module_unemap_cards)
	{
		return_code=1;
	}
	else
	{
		if (!module_get_cache_information_failed)
		{
			module_force_connection=1;
#if defined (OLD_CODE)
			if (unemap_configured())
#endif /* defined (OLD_CODE) */
			{
				module_unemap_cards=(struct Unemap_card *)NULL;
				module_stimulator_unemap_card_indices=(int *)NULL;
				if (unemap_get_number_of_channels(&module_number_of_channels)&&
					(0<(module_number_of_unemap_cards=(int)(module_number_of_channels/
					NUMBER_OF_CHANNELS_ON_NI_CARD)))&&ALLOCATE(module_unemap_cards,
					struct Unemap_card,module_number_of_unemap_cards)&&
					unemap_get_number_of_stimulators(&module_number_of_stimulators)&&
					((0==module_number_of_stimulators)||ALLOCATE(
					module_stimulator_unemap_card_indices,int,
					module_number_of_stimulators)))
				{
					return_code=1;
					channel_number=1;
					stimulator_number=1;
					for (i=0;i<module_number_of_unemap_cards;i++)
					{
						unemap_get_voltage_range(channel_number,
							&((module_unemap_cards[i]).minimum_voltage),
							&((module_unemap_cards[i]).maximum_voltage));
						unemap_get_gain(channel_number,
							&((module_unemap_cards[i]).pre_filter_gain),
							&((module_unemap_cards[i]).post_filter_gain));
						(module_unemap_cards[i]).minimum_voltage *=
							((module_unemap_cards[i]).pre_filter_gain)*
							((module_unemap_cards[i]).post_filter_gain);
						(module_unemap_cards[i]).maximum_voltage *=
							((module_unemap_cards[i]).pre_filter_gain)*
							((module_unemap_cards[i]).post_filter_gain);
						if (unemap_channel_valid_for_stimulator(stimulator_number,
							channel_number))
						{
							module_stimulator_unemap_card_indices[stimulator_number-1]=i;
							stimulator_number++;
						}
						channel_number += NUMBER_OF_CHANNELS_ON_NI_CARD;
					}
				}
				else
				{
					module_number_of_unemap_cards=0;
					module_number_of_stimulators=0;
					DEALLOCATE(module_unemap_cards);
					DEALLOCATE(module_stimulator_unemap_card_indices);
					module_get_cache_information_failed=1;
				}
			}
			module_force_connection=0;
		}
	}
	LEAVE;

	return (return_code);
} /* get_cache_information */
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
	float scrolling_refresh_frequency)
/*******************************************************************************
LAST MODIFIED : 21 July 1999

DESCRIPTION :
Configures the hardware for sampling at the specified <sampling_frequency> and
with the specified <number_of_samples_in_buffer>. <sampling_frequency> and
<number_of_samples_in_buffer> are requests and the actual values used should
be determined using unemap_get_sampling_frequency and
unemap_get_number_of_samples_in_buffer.

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
==============================================================================*/
{
	float temp_scrolling_refresh_frequency;
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+2*sizeof(float)+sizeof(int)+sizeof(long)];
#if defined (WINDOWS)
	DWORD calibration_thread_id;
	HANDLE calibration_thread;
		/*???DB.  Global variable ? */
	DWORD scrolling_thread_id;
	HANDLE scrolling_thread;
		/*???DB.  Global variable ? */
#endif /* defined (WINDOWS) */

	ENTER(unemap_configure);
#if defined (DEBUG)
	/*???debug */
	printf("enter unemap_configure\n");
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if ((0<sampling_frequency)&&(0<number_of_samples_in_buffer))
	{
		if (initialize_connection())
		{
#if defined (WINDOWS)
			if (scrolling_thread=CreateThread(
				/*no security attributes*/NULL,/*use default stack size*/0,
				scrolling_thread_function,(DWORD)NULL,/*use default creation flags*/0,
				&scrolling_thread_id))
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
			if (!application_context||(scrolling_socket_xid=XtAppAddInput(
				application_context,scrolling_socket,(XtPointer)XtInputReadMask,
				scrolling_socket_callback,(XtPointer)NULL)))
#endif /* defined (MOTIF) */
			{
#if defined (WINDOWS)
				if (calibration_thread=CreateThread(
					/*no security attributes*/NULL,/*use default stack size*/0,
					calibration_thread_function,(DWORD)NULL,
					/*use default creation flags*/0,&calibration_thread_id))
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
				if (!application_context||(calibration_socket_xid=XtAppAddInput(
					application_context,calibration_socket,(XtPointer)XtInputReadMask,
					calibration_socket_callback,(XtPointer)NULL)))
#endif /* defined (MOTIF) */
				{
					buffer[0]=UNEMAP_CONFIGURE_CODE;
					buffer[1]=BIG_ENDIAN_CODE;
					buffer_size=2+sizeof(message_size);
					memcpy(buffer+buffer_size,&sampling_frequency,
						sizeof(sampling_frequency));
					buffer_size += sizeof(sampling_frequency);
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
					message_size=buffer_size-(2+(long)sizeof(message_size));
					memcpy(buffer+2,&message_size,sizeof(message_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"unemap_configure.  %g %d %g %g\n",sampling_frequency,
						number_of_samples_in_buffer,temp_scrolling_refresh_frequency,
						scrolling_refresh_frequency);
#endif /* defined (DEBUG) */
					retval=socket_send(command_socket,buffer,buffer_size,0);
					if (SOCKET_ERROR!=retval)
					{
#if defined (DEBUG)
						/*???debug */
						display_message(INFORMATION_MESSAGE,
							"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
						/* get the header back */
						retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
#if defined (CACHE_CLIENT_INFORMATION)
								get_cache_information();
#endif /* defined (CACHE_CLIENT_INFORMATION) */
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"unemap_configure.  socket_recv() failed");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"unemap_configure.  socket_send() failed");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
#if defined (WINDOWS)
						"unemap_configure.  CreateThread failed for calibration socket");
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
						"unemap_configure.  XtAppAddInput failed for calibration socket");
#endif /* defined (MOTIF) */
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
#if defined (WINDOWS)
					"unemap_configure.  CreateThread failed for scrolling socket");
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
					"unemap_configure.  XtAppAddInput failed for scrolling socket");
#endif /* defined (MOTIF) */
			}
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
#if defined (DEBUG)
	/*???debug */
	printf("leave unemap_configure\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_configure */

int unemap_configured(void)
/*******************************************************************************
LAST MODIFIED : 21 July 1999

DESCRIPTION :
Returns a non-zero if unemap is configured and zero otherwise.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(unemap_configured);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
	{
		buffer[0]=UNEMAP_CONFIGURED_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		message_size=0;
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&message_size,buffer+2,sizeof(message_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
					"unemap_configured.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_configured.  socket_send() failed");
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
LAST MODIFIED : 16 July 1999

DESCRIPTION :
Stops acquisition and signal generation.  Frees buffers associated with the
hardware.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(unemap_deconfigure);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
	{
		buffer[0]=UNEMAP_DECONFIGURE_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		message_size=0;
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&message_size,buffer+2,sizeof(message_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
					"unemap_deconfigure.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_deconfigure.  socket_send() failed");
		}
		close_connection();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_deconfigure.  Could not initialize_connection");
	}
#if defined (CACHE_CLIENT_INFORMATION)
	module_number_of_unemap_cards=0;
	module_number_of_stimulators=0;
	module_get_cache_information_failed=0;
	DEALLOCATE(module_unemap_cards);
	DEALLOCATE(module_stimulator_unemap_card_indices);
#endif /* defined (CACHE_CLIENT_INFORMATION) */
	LEAVE;

	return (return_code);
} /* unemap_deconfigure */

int unemap_get_hardware_version(enum UNEMAP_hardware_version *hardware_version)
/*******************************************************************************
LAST MODIFIED : 13 September 1999

DESCRIPTION :
The function does not need the hardware to be configured.

Returns the unemap <hardware_version>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(unemap_get_hardware_version);
	return_code=0;
	/* check arguments */
	if (hardware_version)
	{
		if (initialize_connection())
		{
			buffer[0]=UNEMAP_GET_HARDWARE_VERSION_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(command_socket,buffer,sizeof(int),0))
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
						"unemap_get_hardware_version.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_hardware_version.  socket_send() failed");
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
LAST MODIFIED : 2 July 1999

DESCRIPTION :
Shuts down NT running on the signal conditioning unit computer.
???DB.  Not really anything to do with unemap hardware ?
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(unemap_shutdown);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
	{
		buffer[0]=UNEMAP_SHUTDOWN_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		message_size=0;
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&message_size,buffer+2,sizeof(message_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
					"unemap_shutdown.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_shutdown.  socket_send() failed");
		}
		if (return_code)
		{
#if defined (OLD_CODE)
			fd_set acceptfds;
#endif /* defined (OLD_CODE) */

			/* wait for the command socket to be closed */
#if defined (OLD_CODE)
			FD_ZERO(&acceptfds);
			FD_SET(command_socket,&acceptfds);
			select(1,(fd_set *)NULL,(fd_set *)NULL,&acceptfds,(struct timeval *)NULL);
#endif /* defined (OLD_CODE) */
			do
			{
				do
				{
					printf("before recv\n");
					return_code=recv(command_socket,buffer,1,0);
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
#if defined (OLD_CODE)
#endif /* defined (OLD_CODE) */
		}
#if defined (NEW_CODE)
/*???DB.  To be done */
		first_invalid_socket_call=1;
		if (INVALID_SOCKET!=command_socket)
		{
#if defined (WIN32)
			closesocket(command_socket);
#endif /* defined (WIN32) */
#if defined (UNIX)
			close(command_socket);
#endif /* defined (UNIX) */
			command_socket=INVALID_SOCKET;
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
#endif /* defined (MOTIF) */
#endif /* defined (NEW_CODE) */
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
LAST MODIFIED : 20 July 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Adds the <channel_number> to the list of channels for which scrolling
information is sent via the scrolling_callback (see unemap_configure).
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(unemap_set_scrolling_channel);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
	{
		buffer[0]=UNEMAP_SET_SCROLLING_CHANNEL_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
		buffer_size += sizeof(channel_number);
		message_size=buffer_size-(2+(long)sizeof(message_size));
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
					"unemap_set_scrolling_channel.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_set_scrolling_channel.  socket_send() failed");
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
LAST MODIFIED : 21 July 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Clears the list of channels for which scrolling information is sent via the
scrolling_callback (see unemap_configure).
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(unemap_clear_scrolling_channels);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
	{
		buffer[0]=UNEMAP_CLEAR_SCROLLING_CHANNELS_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		message_size=0;
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&message_size,buffer+2,sizeof(message_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
					"unemap_clear_scrolling_channels.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_clear_scrolling_channels.  socket_send() failed");
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
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Starts scrolling messages/callbacks.  Also need to be sampling to get messages/
callbacks.  Allows sampling without scrolling.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(unemap_start_scrolling);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
	{
		buffer[0]=UNEMAP_START_SCROLLING_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		message_size=0;
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&message_size,buffer+2,sizeof(message_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
					"unemap_start_scrolling.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_start_scrolling.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_start_scrolling.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_start_scrolling */

int unemap_stop_scrolling(void)
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Stops scrolling messages/callbacks.  Also need to be sampling to get messages/
callbacks.  Allows sampling without scrolling.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(unemap_stop_scrolling);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
	{
		buffer[0]=UNEMAP_STOP_SCROLLING_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		message_size=0;
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&message_size,buffer+2,sizeof(message_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
					"unemap_stop_scrolling.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_stop_scrolling.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_stop_scrolling.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_stop_scrolling */

int unemap_calibrate(Calibration_end_callback *calibration_end_callback,
	void *calibration_end_callback_data)
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

When the calibration is completed <calibration_end_callback> is called with -
the number of channels calibrated, the channel numbers for the calibrated
channels, the offsets for the calibrated channels, the gains for the
calibrated channels and the <calibration_end_callback_data>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(unemap_calibrate);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
	{
		module_calibration_end_callback=calibration_end_callback;
		module_calibration_end_callback_data=calibration_end_callback_data;
		buffer[0]=UNEMAP_CALIBRATE_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		message_size=0;
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&message_size,buffer+2,sizeof(message_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
					"unemap_calibrate.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"unemap_calibrate.  socket_send() failed");
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
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Starts the sampling.
???DB.  Check if already going
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(unemap_start_sampling);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
	{
		buffer[0]=UNEMAP_START_SAMPLING_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		message_size=0;
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&message_size,buffer+2,sizeof(message_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
					"unemap_start_sampling.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_start_sampling.  socket_send() failed");
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
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Stops the sampling.  Use <unemap_get_number_of_samples_acquired> to find out how
many samples were acquired.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(unemap_stop_sampling);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
	{
		buffer[0]=UNEMAP_STOP_SAMPLING_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		message_size=0;
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&message_size,buffer+2,sizeof(message_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			return_code=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
					"unemap_stop_sampling.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_stop_sampling.  socket_send() failed");
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
LAST MODIFIED : 16 August 1999

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
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)];

	ENTER(unemap_set_isolate_record_mode);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
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
		retval=socket_send(command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
					"unemap_set_isolate_record_mode.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_set_isolate_record_mode.  socket_send() failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_isolate_record_mode.  Could not initialize_connection");
	}
	LEAVE;

	return (return_code);
} /* unemap_set_isolate_record_mode */

int unemap_get_isolate_record_mode(int channel_number,int *isolate)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

If the group is in isolate mode, then <*isolate> is set to 1.  Otherwise
<*isolate> is set to 0.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(unemap_get_isolate_record_mode);
	return_code=0;
	/* check arguments */
	if (isolate)
	{
		if (initialize_connection())
		{
			buffer[0]=UNEMAP_GET_ISOLATE_RECORD_MODE_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(command_socket,buffer,sizeof(int),0))
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
						"unemap_get_isolate_record_mode.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_isolate_record_mode.  socket_send() failed");
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
LAST MODIFIED : 26 May 1999

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
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)+sizeof(float)];

	ENTER(unemap_set_antialiasing_filter_frequency);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
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
		retval=socket_send(command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
					"unemap_set_antialiasing_filter_frequency.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_set_antialiasing_filter_frequency.  socket_send() failed");
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
LAST MODIFIED : 26 May 1999

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
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(unemap_set_powerup_antialiasing_filter_frequency);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
	{
		buffer[0]=UNEMAP_SET_POWERUP_ANTIALIASING_FILTER_FREQUENCY_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
		buffer_size += sizeof(channel_number);
		message_size=buffer_size-(2+(long)sizeof(message_size));
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
		"unemap_set_powerup_antialiasing_filter_frequency.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
		"unemap_set_powerup_antialiasing_filter_frequency.  socket_send() failed");
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
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

<*frequency> is set to the frequency for the anti-aliasing filter.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(float)];

	ENTER(unemap_get_antialiasing_filter_frequency);
	return_code=0;
	/* check arguments */
	if (frequency)
	{
		if (initialize_connection())
		{
			buffer[0]=UNEMAP_GET_ANTIALIASING_FILTER_FREQUENCY_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(command_socket,buffer,sizeof(float),
							0))
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
						"unemap_get_antialiasing_filter_frequency.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_antialiasing_filter_frequency.  socket_send() failed");
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

int unemap_get_number_of_channels(unsigned long *number_of_channels)
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function does not need the hardware to be configured.

The total number of hardware channels is assigned to <*number_of_channels>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(unemap_get_number_of_channels);
	return_code=0;
	/* check arguments */
	if (number_of_channels)
	{
		if (initialize_connection())
		{
			buffer[0]=UNEMAP_GET_NUMBER_OF_CHANNELS_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			message_size=0;
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(command_socket,buffer,sizeof(long),0))
						{
							memcpy(number_of_channels,buffer,sizeof(long));
#if defined (DEBUG)
							display_message(INFORMATION_MESSAGE," %ld",*number_of_channels);
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
						"unemap_get_number_of_channels.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_number_of_channels.  socket_send() failed");
			}
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
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+2*sizeof(long)];

	ENTER(unemap_get_sample_range);
	return_code=0;
	/* check arguments */
	if (minimum_sample_value&&maximum_sample_value)
	{
		if (initialize_connection())
		{
			buffer[0]=UNEMAP_GET_SAMPLE_RANGE_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			message_size=0;
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(command_socket,buffer,2*sizeof(long),
							0))
						{
							memcpy(minimum_sample_value,buffer,sizeof(long));
							memcpy(maximum_sample_value,buffer+sizeof(long),sizeof(long));
#if defined (DEBUG)
							display_message(INFORMATION_MESSAGE," %ld %ld",
								*minimum_sample_value,*maximum_sample_value);
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
						"unemap_get_sample_range.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_sample_range.  socket_send() failed");
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
LAST MODIFIED : 6 August 1999

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

The voltage range, allowing for gain, is returned via <*minimum_voltage> and
<*maximum_voltage>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
#if defined (CACHE_CLIENT_INFORMATION)
	struct Unemap_card *unemap_card;
#endif /* defined (CACHE_CLIENT_INFORMATION) */
	unsigned char buffer[2+sizeof(long)+2*sizeof(float)];

	ENTER(unemap_get_voltage_range);
	return_code=0;
	/* check arguments */
	if (minimum_voltage&&maximum_voltage)
	{
#if defined (CACHE_CLIENT_INFORMATION)
		/*???DB.  Eventually, it will fail of module_unemap_cards is not set ? */
		if (!module_force_connection&&get_cache_information())
		{
			if ((1<=channel_number)&&(channel_number<=(int)module_number_of_channels))
			{
				unemap_card=module_unemap_cards+
					((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
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
			if (initialize_connection())
			{
				buffer[0]=UNEMAP_GET_VOLTAGE_RANGE_CODE;
				buffer[1]=BIG_ENDIAN_CODE;
				buffer_size=2+sizeof(message_size);
				memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
				buffer_size += sizeof(channel_number);
				message_size=buffer_size-(2+(long)sizeof(message_size));
				memcpy(buffer+2,&message_size,sizeof(message_size));
				retval=socket_send(command_socket,buffer,buffer_size,0);
				if (SOCKET_ERROR!=retval)
				{
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
					/* get the header back */
					retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
					if (SOCKET_ERROR!=retval)
					{
						memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
						/*???debug */
						display_message(INFORMATION_MESSAGE,
							"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
							buffer_size);
#endif /* defined (DEBUG) */
						if ((2+sizeof(float)==retval)&&(2*sizeof(float)==buffer_size)&&
							(buffer[0]))
						{
							if (SOCKET_ERROR!=socket_recv(command_socket,buffer,
								2*sizeof(float),0))
							{
								memcpy(minimum_voltage,buffer,sizeof(float));
								memcpy(maximum_voltage,buffer+sizeof(float),sizeof(float));
#if defined (DEBUG)
								display_message(INFORMATION_MESSAGE," %g %g",*minimum_voltage,
									*maximum_voltage);
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
							"unemap_get_voltage_range.  socket_recv() failed");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"unemap_get_voltage_range.  socket_send() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_voltage_range.  Could not initialize_connection");
			}
#if defined (CACHE_CLIENT_INFORMATION)
		}
#endif /* defined (CACHE_CLIENT_INFORMATION) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_voltage_range.  Missing minimum_voltage or maximum_voltage");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_voltage_range */

int unemap_get_number_of_samples_acquired(unsigned long *number_of_samples)
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

The number of samples acquired per channel since <unemap_start_sampling> is
assigned to <*number_of_samples>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(unemap_get_number_of_samples_acquired);
	return_code=0;
	/* check arguments */
	if (number_of_samples)
	{
		if (initialize_connection())
		{
			buffer[0]=UNEMAP_GET_NUMBER_OF_SAMPLES_ACQUIRED_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			message_size=0;
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(command_socket,buffer,sizeof(long),0))
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
						"unemap_get_number_of_samples_acquired.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_number_of_samples_acquired.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
		"unemap_get_number_of_samples_acquired.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_number_of_samples_acquired.  Missing number_of_channels");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_number_of_samples_acquired */

int unemap_get_samples_acquired(int channel_number,short int *samples)
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is valid (between 1 and the total number of channels
inclusive), then the <samples> for that channel are returned.  If
<channel_number> is 0 then the <samples> for all channels are returned.
Otherwise the function fails.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(unemap_get_samples_acquired);
	return_code=0;
	/* check arguments */
	if (samples)
	{
		if (initialize_connection())
		{
			buffer[0]=UNEMAP_GET_SAMPLES_ACQUIRED_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
						buffer_size);
#endif /* defined (DEBUG) */
					if ((2+sizeof(float)==retval)&&(0<buffer_size)&&(buffer[0]))
					{
						if (SOCKET_ERROR!=socket_recv(command_socket,
							(unsigned char *)samples,buffer_size,0))
						{
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
						"unemap_get_samples_acquired.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_samples_acquired.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_samples_acquired.  Could not initialize_connection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"unemap_get_samples_acquired.  Missing minimum_voltage or maximum_voltage");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_samples_acquired */

int unemap_get_maximum_number_of_samples(unsigned long *number_of_samples)
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

The size of the rolling buffer, in number of samples per channel, is assigned to
<*number_of_samples>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(unemap_get_maximum_number_of_samples);
	return_code=0;
	/* check arguments */
	if (number_of_samples)
	{
		if (initialize_connection())
		{
			buffer[0]=UNEMAP_GET_MAXIMUM_NUMBER_OF_SAMPLES_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			message_size=0;
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(command_socket,buffer,sizeof(long),0))
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
						"unemap_get_maximum_number_of_samples.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_maximum_number_of_samples.  socket_send() failed");
			}
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
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(float)];

	ENTER(unemap_get_sampling_frequency);
	return_code=0;
	/* check arguments */
	if (frequency)
	{
		if (initialize_connection())
		{
			buffer[0]=UNEMAP_GET_SAMPLING_FREQUENCY_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			message_size=0;
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(command_socket,buffer,sizeof(float),
							0))
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
						"unemap_get_sampling_frequency.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_sampling_frequency.  socket_send() failed");
			}
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
			"unemap_get_sampling_frequency.  Missing number_of_channels");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_sampling_frequency */

int unemap_set_gain(int channel_number,float pre_filter_gain,
	float post_filter_gain)
/*******************************************************************************
LAST MODIFIED : 17 August 1999

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
	int return_code,retval;
	long buffer_size,message_size;
#if defined (CACHE_CLIENT_INFORMATION)
	int i;
	struct Unemap_card *unemap_card;
#endif /* defined (CACHE_CLIENT_INFORMATION) */
	unsigned char buffer[2+sizeof(long)+sizeof(int)+2*sizeof(float)];

	ENTER(unemap_set_gain);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
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
		retval=socket_send(command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
					if (module_unemap_cards)
					{
						module_force_connection=1;
						if (0==channel_number)
						{
							unemap_card=module_unemap_cards;
							for (i=module_number_of_unemap_cards*
								NUMBER_OF_CHANNELS_ON_NI_CARD;i>0;
								i -= NUMBER_OF_CHANNELS_ON_NI_CARD)
							{
								unemap_get_gain(i,&(unemap_card->pre_filter_gain),
									&(unemap_card->post_filter_gain));
							}
						}
						else
						{
							unemap_card=module_unemap_cards+
								((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
							unemap_get_gain(channel_number,&(unemap_card->pre_filter_gain),
								&(unemap_card->post_filter_gain));
						}
						module_force_connection=0;
					}
#endif /* defined (CACHE_CLIENT_INFORMATION) */
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"unemap_set_gain.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"unemap_set_gain.  socket_send() failed");
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
LAST MODIFIED : 6 August 1999

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

The <*pre_filter_gain> and <*post_filter_gain> for the group are assigned.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
#if defined (CACHE_CLIENT_INFORMATION)
	struct Unemap_card *unemap_card;
#endif /* defined (CACHE_CLIENT_INFORMATION) */
	unsigned char buffer[2+sizeof(long)+2*sizeof(float)];

	ENTER(unemap_get_gain);
	return_code=0;
	/* check arguments */
	if (pre_filter_gain&&post_filter_gain)
	{
#if defined (CACHE_CLIENT_INFORMATION)
#if defined (DEBUG)
		/*???debug */
		display_message(INFORMATION_MESSAGE,
			"module_force_connection=%d\n",module_force_connection);
#endif /* defined (DEBUG) */
		/*???DB.  Eventually, it will fail if module_unemap_cards is not set ? */
		if (!module_force_connection&&get_cache_information())
		{
			if ((1<=channel_number)&&(channel_number<=(int)module_number_of_channels))
			{
				unemap_card=module_unemap_cards+
					((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
				*pre_filter_gain=unemap_card->pre_filter_gain;
				*post_filter_gain=unemap_card->post_filter_gain;
				return_code=1;
			}
		}
		else
		{
#endif /* defined (CACHE_CLIENT_INFORMATION) */
			if (initialize_connection())
			{
				buffer[0]=UNEMAP_GET_GAIN_CODE;
				buffer[1]=BIG_ENDIAN_CODE;
				buffer_size=2+sizeof(message_size);
				memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
				buffer_size += sizeof(channel_number);
				message_size=buffer_size-(2+(long)sizeof(message_size));
				memcpy(buffer+2,&message_size,sizeof(message_size));
				retval=socket_send(command_socket,buffer,buffer_size,0);
				if (SOCKET_ERROR!=retval)
				{
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
					/* get the header back */
					retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
					if (SOCKET_ERROR!=retval)
					{
						memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
						/*???debug */
						display_message(INFORMATION_MESSAGE,
							"Received %d bytes, data %x %x %ld",retval,buffer[0],buffer[1],
							buffer_size);
#endif /* defined (DEBUG) */
						if ((2+sizeof(float)==retval)&&(2*sizeof(float)==buffer_size)&&
							(buffer[0]))
						{
							if (SOCKET_ERROR!=socket_recv(command_socket,buffer,
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
							"unemap_get_gain.  socket_recv() failed");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"unemap_get_gain.  socket_send() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_gain.  Could not initialize_connection");
			}
#if defined (CACHE_CLIENT_INFORMATION)
		}
#endif /* defined (CACHE_CLIENT_INFORMATION) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_gain.  Missing pre_filter_gain or post_filter_gain");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_gain */

int unemap_start_voltage_stimulating(int channel_number,int number_of_voltages,
	float voltages_per_second,float *voltages)
/*******************************************************************************
LAST MODIFIED : 30 September 1999

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
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)+sizeof(float)];

	ENTER(unemap_start_voltage_stimulating);
	return_code=0;
	if ((0==number_of_voltages)||((0<number_of_voltages)&&voltages))
	{
		if (initialize_connection())
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
			retval=socket_send(command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
				if (0<number_of_voltages)
				{
					retval=socket_send(command_socket,(unsigned char *)voltages,
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
				retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
							retval=socket_recv(command_socket,(unsigned char *)voltages,
								buffer_size,0);
							if (SOCKET_ERROR!=retval)
							{
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
						"unemap_start_voltage_stimulating.  socket_recv() voltages failed");
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
						"unemap_start_voltage_stimulating.  socket_recv() header failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_start_voltage_stimulating.  socket_send() failed");
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
LAST MODIFIED : 30 September 1999

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
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)+sizeof(float)];

	ENTER(unemap_start_current_stimulating);
	return_code=0;
	if ((0==number_of_currents)||((0<number_of_currents)&&currents))
	{
		if (initialize_connection())
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
			retval=socket_send(command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
				if (0<number_of_currents)
				{
					retval=socket_send(command_socket,(unsigned char *)currents,
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
				retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
							retval=socket_recv(command_socket,(unsigned char *)currents,
								buffer_size,0);
							if (SOCKET_ERROR!=retval)
							{
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
						"unemap_start_current_stimulating.  socket_recv() currents failed");
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
						"unemap_start_current_stimulating.  socket_recv() header failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_start_current_stimulating.  socket_send() failed");
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

int unemap_stop_stimulating(int channel_number)
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Stops stimulating for the channels in the group.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(unemap_stop_stimulating);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
	{
		buffer[0]=UNEMAP_STOP_STIMULATING_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
		buffer_size += sizeof(channel_number);
		message_size=buffer_size-(2+(long)sizeof(message_size));
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
					"unemap_stop_stimulating.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_stop_stimulating.  socket_send() failed");
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
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Zero <stimulating> means off.  Non-zero <stimulating> means on.  If
<channel_number> is valid (between 1 and the total number of channels
inclusive), then <channel_number> is set to <stimulating>.  If <channel_number>
is 0, then all channels are set to <stimulating>.  Otherwise the function fails.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)];

	ENTER(unemap_set_channel_stimulating);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
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
		retval=socket_send(command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
					"unemap_set_channel_stimulating.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_set_channel_stimulating.  socket_send() failed");
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
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is valid (between 1 and the total number of channels
inclusive), then <stimulating> is set to 1 if <channel_number> is stimulating
and 0 otherwise.  Otherwise the function fails.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(unemap_get_channel_stimulating);
	return_code=0;
	/* check arguments */
	if (stimulating)
	{
		if (initialize_connection())
		{
			buffer[0]=UNEMAP_GET_CHANNEL_STIMULATING_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(command_socket,buffer,sizeof(int),0))
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
						"unemap_get_channel_stimulating.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_channel_stimulating.  socket_send() failed");
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
LAST MODIFIED : 30 September 1999

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
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)+sizeof(float)];

	ENTER(unemap_start_calibrating);
	return_code=0;
	if ((0==number_of_voltages)||((0<number_of_voltages)&&voltages))
	{
		if (initialize_connection())
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
			retval=socket_send(command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
				if (0<number_of_voltages)
				{
					retval=socket_send(command_socket,(unsigned char *)voltages,
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
				retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
						(sizeof(float)*number_of_voltages==buffer_size)&&(buffer[0]))
					{
						if (0<buffer_size)
						{
							retval=socket_recv(command_socket,(unsigned char *)voltages,
								buffer_size,0);
							if (SOCKET_ERROR!=retval)
							{
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"unemap_start_calibrating.  socket_recv() voltages failed");
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
						"unemap_start_calibrating.  socket_recv() header failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_start_calibrating.  socket_send() failed");
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
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Stops generating the calibration signal for the channels in the group.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(unemap_stop_calibrating);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
	{
		buffer[0]=UNEMAP_STOP_CALIBRATING_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
		buffer_size += sizeof(channel_number);
		message_size=buffer_size-(2+(long)sizeof(message_size));
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
					"unemap_stop_calibrating.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_stop_calibrating.  socket_send() failed");
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
LAST MODIFIED : 16 August 1999

DESCRIPTION :
The function does not need the hardware to be configured.

If <on> is zero the hardware is powered off, otherwise the hardware is powered
on.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(unemap_set_power);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
	{
		buffer[0]=UNEMAP_SET_POWER_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&on,sizeof(on));
		buffer_size += sizeof(on);
		message_size=buffer_size-(2+(long)sizeof(message_size));
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
					"unemap_set_power.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"unemap_set_power.  socket_send() failed");
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

int unemap_get_power(int *on)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
The function does not need the hardware to be configured.

If the hardware power is on then <*on> is set to 1, otherwise <*on> is set to 0.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(unemap_get_power);
	return_code=0;
	/* check arguments */
	if (on)
	{
		if (initialize_connection())
		{
			buffer[0]=UNEMAP_GET_POWER_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(command_socket,buffer,sizeof(int),0))
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
						"unemap_get_power.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_power.  socket_send() failed");
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
LAST MODIFIED : 21 July 1999

DESCRIPTION :
The function does not need the hardware to be configured.

The total number of hardware stimulators is assigned to
<*number_of_stimulators>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

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
		}
		else
		{
#endif /* defined (CACHE_CLIENT_INFORMATION) */
			if (initialize_connection())
			{
				buffer[0]=UNEMAP_GET_NUMBER_OF_STIMULATORS_CODE;
				buffer[1]=BIG_ENDIAN_CODE;
				buffer_size=2+sizeof(message_size);
				message_size=0;
				memcpy(buffer+2,&message_size,sizeof(message_size));
				retval=socket_send(command_socket,buffer,buffer_size,0);
				if (SOCKET_ERROR!=retval)
				{
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
					/* get the header back */
					retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
							if (SOCKET_ERROR!=socket_recv(command_socket,buffer,sizeof(int),
								0))
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
							"unemap_get_number_of_stimulators.  socket_recv() failed");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"unemap_get_number_of_stimulators.  socket_send() failed");
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

int unemap_channel_valid_for_stimulator(int stimulator_number,
	int channel_number)
/*******************************************************************************
LAST MODIFIED : 6 August 1999

DESCRIPTION :
The function does not need the hardware to be configured.

Returns non-zero if <stimulator_number> can stimulate down <channel_number>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)];

	ENTER(unemap_channel_valid_for_stimulator);
	return_code=0;
#if defined (CACHE_CLIENT_INFORMATION)
	/*???DB.  Eventually, it will fail of module_unemap_cards is not set ? */
	if (!module_force_connection&&get_cache_information())
	{
		if ((1<=channel_number)&&(channel_number<=(int)module_number_of_channels))
		{
			if ((0<stimulator_number)&&
				(stimulator_number<=module_number_of_stimulators)&&
				(1<=channel_number)&&(channel_number<=(int)module_number_of_channels)&&
				(module_stimulator_unemap_card_indices[stimulator_number-1]==
				(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD))
			{
				return_code=1;
			}
		}
	}
	else
	{
#endif /* defined (CACHE_CLIENT_INFORMATION) */
		/* unemap may have been started by another process, so can't just check that
			the command_socket is valid */
		if (initialize_connection())
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
			retval=socket_send(command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
						"unemap_channel_valid_for_stimulator.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_channel_valid_for_stimulator.  socket_send() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
			"unemap_channel_valid_for_stimulator.  Could not initialize_connection");
		}
#if defined (CACHE_CLIENT_INFORMATION)
	}
#endif /* defined (CACHE_CLIENT_INFORMATION) */
	LEAVE;

	return (return_code);
} /* unemap_channel_valid_for_stimulator */

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
LAST MODIFIED : 30 June 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Returns the current state of the signal conditioning card containing the
<channel_number>.

Intended for diagnostic use only.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+10*sizeof(int)+2*sizeof(float)+10];

	ENTER(unemap_get_card_state);
	return_code=0;
	/* check arguments */
	if (battA_state&&battGood_state&&filter_frequency&&filter_taps&&
		shift_registers&&GA0_state&&GA1_state&&NI_gain&&input_mode&&polarity&&
		tol_settling&&sampling_interval&&settling_step_max)
	{
		if (initialize_connection())
		{
			buffer[0]=UNEMAP_GET_CARD_STATE_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			message_size=buffer_size-(2+(long)sizeof(message_size));
			memcpy(buffer+2,&message_size,sizeof(message_size));
			retval=socket_send(command_socket,buffer,buffer_size,0);
			if (SOCKET_ERROR!=retval)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
				/* get the header back */
				retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(command_socket,buffer,buffer_size,0))
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
						"unemap_get_card_state.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_card_state.  socket_send() failed");
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
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Toggles the <shift_register> of the signal conditioning card containing the
<channel_number>.

Intended for diagnostic use only.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)];

	ENTER(unemap_toggle_shift_register);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection())
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
		retval=socket_send(command_socket,buffer,buffer_size,0);
		if (SOCKET_ERROR!=retval)
		{
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
			/* get the header back */
			retval=socket_recv(command_socket,buffer,2+sizeof(long),0);
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
					"unemap_toggle_shift_register.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_toggle_shift_register.  socket_send() failed");
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
