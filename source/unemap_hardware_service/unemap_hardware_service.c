/*******************************************************************************
FILE : unemap_hardware_service.c

LAST MODIFIED : 24 September 2002

DESCRIPTION :
The unemap service which runs under NT and talks to unemap via sockets.

SERVICE_VERSION :
13 January 2002.  Added so that can maintain compatability between clients and
	servers
1 Added the ability to specify the number_of_samples for
	unemap_get_samples_acquired and related functions
2 Change unemap_get_sample_range to include the channel_number for systems with
	mixed 12 and 16 bit cards

NOTES :
1 Started out as :
	c:\program files\devstudio\samples\sdk\winnt\service - for service
	c:\mstools\samples\win32\winsock2\simple - for sockets
2 Usage (test version)
	unemap_hardware_service -install
		installs the service "Unemap Hardware Service"
	sc start UnemapHardwareService
		starts the service (sc is a SDK/NT tool, can also use Services from the
		control panel)
	sc query UnemapHardwareService
		queries the state of the service
	unemap_hardware_service -remove
		removes the service
3 Had to add esp20 to /etc/hosts for esu24 for the socket client to work
4 To create service, compile service.c , compile unemap_hardware_service.c and
	link the object files

PROBLEMS :
1 If "simplec -n esp20" is the first connection then fails (sends data but then
	has a recv() failed) and subsequent connections (esp20 or 130.216.5.170) fail.
	If "simplec -n 130.216.5.170" is the first connection then OK.
2 Problem with NT crashing (blue screen) when using a large acquisition buffer
	(Also see unemap_hardware.c)
2.1 Problem description - see unemap_hardware.c
2.2 Tried many things, mostly using #defines,
2.2.1 WIN32_IO
			Thought that may be something wrong with the generic file I/O (fopen,
			etc), so swapped to the Microsoft ones.  Doesn't fix the problem
2.2.2 USE_MEMORY_FOR_BACKGROUND
			Instead of writing to disk, copy into a separate memory buffer.  If the
			sampling buffer is big enough, then virtual memory will make this
			equivalent to writing to disk.  Doesn't fix the problem
2.2.3 NO_ACQUIRED_THREAD
			Thought that may be problems with locking memory between threads.  Doesn't
			fix the problem
2.2.4 USE_UNEMAP_HARDWARE
			Problems with loading service at start up under W2K.  Trying not using
			unemap (NI-DAQ DLL)

TO DO :
1 Clear unemap.deb
==============================================================================*/

#define USE_SOCKETS
/*#define USE_WORMHOLES*/
#define USE_UNEMAP_HARDWARE

#include <stdio.h>
#include <stdlib.h>
#if defined (USE_UNEMAP_HARDWARE)
#if defined (USE_SOCKETS)
#include <winsock2.h>
#include <process.h>
#include <tchar.h>
#endif /* defined (USE_SOCKETS) */
#if defined (USE_WORMHOLES)
#include <windows.h>
#include "wormhole.h"
#endif /* defined (USE_WORMHOLES) */
#endif /* defined (USE_UNEMAP_HARDWARE) */

#include "service.h"
#include "general/debug.h"
#include "general/myio.h"
#if defined (USE_UNEMAP_HARDWARE)
#include "unemap/unemap_hardware.h"
#endif /* defined (USE_UNEMAP_HARDWARE) */
#include "unemap_hardware_service/unemap_hardware_service.h"
#include "user_interface/message.h"

/*
Module constants
----------------
*/
#if defined (USE_UNEMAP_HARDWARE)
/* Used so that the client can deal with old versions */
#define SERVICE_VERSION 2
#if defined (USE_SOCKETS)
#define BUFFER_SIZE 128
#define DEFAULT_PORT 5001
#define MESSAGE_HEADER_SIZE 6
#endif /* defined (USE_SOCKETS) */
#if defined (USE_WORMHOLES)
#define COMMAND_IN_CONNECTION_ID "sock:esp20:5001"
#define COMMAND_OUT_CONNECTION_ID "sock:esp20:5002"
#define SCROLLING_IN_CONNECTION_ID "sock:esp20:5003"
#define SCROLLING_OUT_CONNECTION_ID "sock:esp20:5004"
#endif /* defined (USE_WORMHOLES) */
#endif /* defined (USE_UNEMAP_HARDWARE) */

/*
Module variables
----------------
*/
/* for error messages */
char error_string[501];
/* this event is signalled when the service should end */
HANDLE hServerStopEvent=NULL;
#if defined (USE_UNEMAP_HARDWARE)
#if defined (USE_SOCKETS)
HANDLE acquired_socket_mutex=NULL;
HANDLE command_socket_read_event=NULL;
SOCKET acquired_socket=INVALID_SOCKET,calibration_socket=INVALID_SOCKET,
	command_socket=INVALID_SOCKET,scrolling_socket=INVALID_SOCKET;
#endif /* defined (USE_SOCKETS) */
unsigned char acquired_big_endian,calibration_big_endian,scrolling_big_endian;
#endif /* defined (USE_UNEMAP_HARDWARE) */

/*
Prototypes for unemap "hidden" functions
----------------------------------------
*/
#if defined (USE_UNEMAP_HARDWARE)
int unemap_get_card_state(int channel_number,int *battA_state,
	int *battGood_state,float *filter_frequency,int *filter_taps,
	unsigned char shift_registers[10],int *GA0_state,int *GA1_state,
	int *NI_gain,int *input_mode,int *polarity,float *tol_settling,
	int *sampling_interval,int *settling_step_max);
/*******************************************************************************
LAST MODIFIED : 30 June 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Returns the current state of the signal conditioning card containing the
<channel_number>.

Intended for diagnostic use only.
==============================================================================*/

int unemap_toggle_shift_register(int channel_number,int register_number);
/*******************************************************************************
LAST MODIFIED : 19 February 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Toggles the <shift_register> of the signal conditioning card containing the
<channel_number>.

Intended for diagnostic use only.
==============================================================================*/
#endif /* defined (USE_UNEMAP_HARDWARE) */

/*
Module functions
----------------
*/
FILE *fopen_UNEMAP_HARDWARE(char *file_name,char *type)
/*******************************************************************************
LAST MODIFIED : 11 January 1999

DESCRIPTION :
Opens a file in the UNEMAP_HARDWARE directory.
==============================================================================*/
{
	char *hardware_directory,*hardware_file_name;
	FILE *hardware_file;

	hardware_file=(FILE *)NULL;
	if (file_name&&type)
	{
		hardware_file_name=(char *)NULL;
		if (hardware_directory=getenv("UNEMAP_HARDWARE"))
		{
			if (hardware_file_name=(char *)malloc(strlen(hardware_directory)+
				strlen(file_name)+2))
			{
				strcpy(hardware_file_name,hardware_directory);
#if defined (WIN32_SYSTEM)
				if ('\\'!=hardware_file_name[strlen(hardware_file_name)-1])
				{
					strcat(hardware_file_name,"\\");
				}
#else /* defined (WIN32_SYSTEM) */
				if ('/'!=hardware_file_name[strlen(hardware_file_name)-1])
				{
					strcat(hardware_file_name,"/");
				}
#endif /* defined (WIN32_SYSTEM) */
			}
		}
		else
		{
			if (hardware_file_name=(char *)malloc(strlen(file_name)+1))
			{
				hardware_file_name[0]='\0';
			}
		}
		if (hardware_file_name)
		{
			strcat(hardware_file_name,file_name);
			hardware_file=fopen(hardware_file_name,type);
			free(hardware_file_name);
		}
	}

	return (hardware_file);
} /* fopen_UNEMAP_HARDWARE */

int rename_UNEMAP_HARDWARE(char *old_name,char *new_name)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Renames a file in the UNEMAP_HARDWARE directory.
==============================================================================*/
{
	char *hardware_directory,*hardware_new_name,*hardware_old_name;
	int return_code;

	return_code= -1;
	if (old_name&&new_name)
	{
		hardware_new_name=(char *)NULL;
		hardware_old_name=(char *)NULL;
		if (hardware_directory=getenv("UNEMAP_HARDWARE"))
		{
			hardware_new_name=(char *)malloc(strlen(hardware_directory)+
				strlen(new_name)+2);
			hardware_old_name=(char *)malloc(strlen(hardware_directory)+
				strlen(old_name)+2);
			if (hardware_new_name&&hardware_old_name)
			{
				strcpy(hardware_new_name,hardware_directory);
				strcpy(hardware_old_name,hardware_directory);
#if defined (WIN32_SYSTEM)
				if ('\\'!=hardware_new_name[strlen(hardware_new_name)-1])
				{
					strcat(hardware_new_name,"\\");
					strcat(hardware_old_name,"\\");
				}
#else /* defined (WIN32_SYSTEM) */
				if ('/'!=hardware_new_name[strlen(hardware_new_name)-1])
				{
					strcat(hardware_new_name,"/");
					strcat(hardware_old_name,"/");
				}
#endif /* defined (WIN32_SYSTEM) */
			}
		}
		else
		{
			hardware_new_name=(char *)malloc(strlen(new_name)+1);
			hardware_old_name=(char *)malloc(strlen(old_name)+1);
			if (hardware_new_name&&hardware_old_name)
			{
				hardware_new_name[0]='\0';
				hardware_old_name[0]='\0';
			}
		}
		if (hardware_new_name&&hardware_old_name)
		{
			strcat(hardware_new_name,new_name);
			strcat(hardware_old_name,old_name);
			return_code=rename(hardware_old_name,hardware_new_name);
		}
		if (hardware_new_name)
		{
			free(hardware_new_name);
		}
		if (hardware_old_name)
		{
			free(hardware_old_name);
		}
	}

	return (return_code);
} /* rename_UNEMAP_HARDWARE */

static int display_error_message(char *message,void *dummy)
/*******************************************************************************
LAST MODIFIED : 10 August 1999

DESCRIPTION :
==============================================================================*/
{
	FILE *unemap_debug;
	int return_code;

	return_code=1;
	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"ERROR: %s\n",message);
		fclose(unemap_debug);
	}
	else
	{
		printf("ERROR: %s\n",message);
		return_code=1;
	}

	return (return_code);
} /* display_error_message */

static int display_information_message(char *message,void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 10 August 1999

DESCRIPTION :
==============================================================================*/
{
	FILE *unemap_debug;
	int return_code;

	return_code=1;
	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"INFORMATION: %s",message);
		fclose(unemap_debug);
	}
	else
	{
		printf("INFORMATION: %s",message);
		return_code=1;
	}

	return (return_code);
} /* display_information_message */

static int display_warning_message(char *message,void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 10 August 1999

DESCRIPTION :
==============================================================================*/
{
	FILE *unemap_debug;
	int return_code;

	return_code=1;
	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"WARNING: %s\n",message);
		fclose(unemap_debug);
	}
	else
	{
		printf("WARNING: %s\n",message);
		return_code=1;
	}

	return (return_code);
} /* display_warning_message */

#if defined (USE_UNEMAP_HARDWARE)
static int close_connection(void)
/*******************************************************************************
LAST MODIFIED : 2 July 2000

DESCRIPTION :
Closes the sockets connecting the service with the client.
==============================================================================*/
{
	int return_code;

	return_code=1;
	closesocket(command_socket);
	command_socket=INVALID_SOCKET;
	closesocket(scrolling_socket);
	scrolling_socket=INVALID_SOCKET;
	closesocket(calibration_socket);
	calibration_socket=INVALID_SOCKET;
	closesocket(acquired_socket);
	acquired_socket=INVALID_SOCKET;

	return (return_code);
} /* close_connection */

#if defined (USE_SOCKETS)
static int socket_recv(SOCKET socket,HANDLE socket_read_event,
	unsigned char *buffer,int buffer_length,int flags)
/*******************************************************************************
LAST MODIFIED : 2 August 2002

DESCRIPTION :
Wrapper function for recv.
==============================================================================*/
{
	int buffer_received,last_error,return_code;

	buffer_received=0;
	return_code=0;
	do
	{
		do
		{
			if (socket_read_event)
			{
				ResetEvent(socket_read_event);
			}
			return_code=recv(socket,buffer+buffer_received,
				buffer_length-buffer_received,flags);
		} while ((SOCKET_ERROR==return_code)&&
			(WSAEWOULDBLOCK==(last_error=WSAGetLastError())));
		if (SOCKET_ERROR!=return_code)
		{
			buffer_received += return_code;
		}
	} while ((SOCKET_ERROR!=return_code)&&(0!=return_code)&&
		(buffer_received<buffer_length));
	if ((SOCKET_ERROR==return_code)||(0==return_code))
	{
		if ((WSAECONNRESET==last_error)||(0==return_code))
		{
			sprintf(error_string,"socket_recv.  Connection closed");
		}
		else
		{
			sprintf(error_string,"socket_recv.  recv() failed: error %d",
				last_error);
		}
		display_message(ERROR_MESSAGE,error_string);
		close_connection();
		return_code=SOCKET_ERROR;
	}
	else
	{
		return_code=buffer_length;
	}

	return (return_code);
} /* socket_recv */
#endif /* defined (USE_SOCKETS) */

#define SEND_BLOCK_SIZE 4096

#if defined (USE_SOCKETS)
static int socket_send(SOCKET socket,unsigned char *buffer,int buffer_length,
	int flags)
/*******************************************************************************
LAST MODIFIED : 14 July 1999

DESCRIPTION :
Wrapper function for send.
==============================================================================*/
{
	unsigned char *local_buffer;
	int last_error,local_buffer_length,return_code;

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
			(WSAEWOULDBLOCK==(last_error=WSAGetLastError())));
		if (SOCKET_ERROR!=return_code)
		{
			local_buffer_length -= return_code;
			local_buffer += return_code;
		}
	} while ((SOCKET_ERROR!=return_code)&&(0!=return_code)&&
		(local_buffer_length>0));
	if ((SOCKET_ERROR==return_code)||(0==return_code))
	{
		if ((WSAECONNRESET==last_error)||(0==return_code))
		{
			sprintf(error_string,"socket_send.  Connection closed");
		}
		else
		{
			sprintf(error_string,"socket_send.  send() failed: error %d",
				last_error);
		}
		display_message(ERROR_MESSAGE,error_string);
		close_connection();
		return_code=SOCKET_ERROR;
	}
	else
	{
		return_code=buffer_length;
	}

	return (return_code);
} /* socket_send */
#endif /* defined (USE_SOCKETS) */

static int copy_byte_swapped(unsigned char *destination,int number_of_bytes,
	unsigned char *source,unsigned char big_endian)
/*******************************************************************************
LAST MODIFIED : 1 March 1999

DESCRIPTION :
Copies the bytes from <source> into <destination>, swapping the byte order if
required.
==============================================================================*/
{
	unsigned char *destination_byte,*source_byte;
	int i,return_code;

	return_code=0;
	if (destination&&(0<number_of_bytes)&&source)
	{
#if defined (BYTE_ORDER) && (1234==BYTE_ORDER)
		if (big_endian)
		{
			source_byte=source;
			destination_byte=destination;
			for (i=number_of_bytes;i>0;i--)
			{
				*destination_byte= *source_byte;
				source_byte++;
				destination_byte++;
			}
		}
		else
		{
			source_byte=source;
			destination_byte=destination+number_of_bytes;
			for (i=number_of_bytes;i>0;i--)
			{
				destination_byte--;
				*destination_byte= *source_byte;
				source_byte++;
			}
		}
#else /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
		if (big_endian)
		{
			source_byte=source;
			destination_byte=destination+number_of_bytes;
			for (i=number_of_bytes;i>0;i--)
			{
				destination_byte--;
				*destination_byte= *source_byte;
				source_byte++;
			}
		}
		else
		{
			source_byte=source;
			destination_byte=destination;
			for (i=number_of_bytes;i>0;i--)
			{
				*destination_byte= *source_byte;
				source_byte++;
				destination_byte++;
			}
		}
#endif /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
	}

	return (return_code);
} /* copy_byte_swapped */

static void scrolling_callback(int number_of_channels,int *channel_numbers,
	int number_of_values_per_channel,short *values,void *dummy)
/*******************************************************************************
LAST MODIFIED : 21 July 1999

DESCRIPTION :
Called by unemap hardware.  Sends the information down the scrolling socket and
frees the channel_numbers and values arrays.
==============================================================================*/
{
	int i,retval,number_of_values;
	long message_size,out_buffer_size;
	unsigned char *out_buffer;

	if ((0<number_of_channels)&&channel_numbers&&
		(0<number_of_values_per_channel)&&values)
	{
		if (INVALID_SOCKET!=scrolling_socket)
		{
			/* send message down scrolling socket */
			number_of_values=number_of_channels*number_of_values_per_channel;
			message_size=(number_of_channels+2)*sizeof(int)+
				number_of_values*sizeof(short);
			out_buffer_size=2+sizeof(long)+message_size;
			if (ALLOCATE(out_buffer,unsigned char,out_buffer_size))
			{
				out_buffer[0]=(unsigned char)0x01;
					/*???DB.  Not sure if codes are needed */
				out_buffer[1]=scrolling_big_endian;
				copy_byte_swapped(out_buffer+2,sizeof(long),(char *)&message_size,
					scrolling_big_endian);
				out_buffer_size=2+sizeof(long);
				copy_byte_swapped(out_buffer+out_buffer_size,sizeof(int),
					(char *)&number_of_channels,scrolling_big_endian);
				out_buffer_size += sizeof(int);
				for (i=0;i<number_of_channels;i++)
				{
					copy_byte_swapped(out_buffer+out_buffer_size,sizeof(int),
						(char *)(channel_numbers+i),scrolling_big_endian);
					out_buffer_size += sizeof(int);
				}
				copy_byte_swapped(out_buffer+out_buffer_size,sizeof(int),
					(char *)&number_of_values_per_channel,scrolling_big_endian);
				out_buffer_size += sizeof(int);
				for (i=0;i<number_of_values;i++)
				{
					copy_byte_swapped(out_buffer+out_buffer_size,sizeof(short),
						(char *)(values+i),scrolling_big_endian);
					out_buffer_size += sizeof(short);
				}
				retval=socket_send(scrolling_socket,out_buffer,out_buffer_size,0);
				DEALLOCATE(out_buffer);
			}
		}
	}
	/* free the arrays */
	if (channel_numbers)
	{
		DEALLOCATE(channel_numbers);
	}
	if (values)
	{
		DEALLOCATE(values);
	}
} /* scrolling_callback */

static void calibration_callback(const int number_of_channels,
	const int *channel_numbers,const float *channel_offsets,
	const float *channel_gains,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 27 May 1999

DESCRIPTION :
Called by unemap hardware.  Sends the information down the calibration socket.
==============================================================================*/
{
	int i,retval;
	long message_size,out_buffer_size;
	unsigned char *out_buffer;

	if (INVALID_SOCKET!=calibration_socket)
	{
		/* send message down calibration socket */
		if ((0<number_of_channels)&&channel_numbers&&channel_offsets&&channel_gains)
		{
			/* succeeded */
			message_size=sizeof(number_of_channels)+number_of_channels*
				(sizeof(int)+2*sizeof(float));
			out_buffer_size=2+sizeof(long)+message_size;
			if (ALLOCATE(out_buffer,unsigned char,out_buffer_size))
			{
				out_buffer[0]=(unsigned char)0x01;
				out_buffer[1]=calibration_big_endian;
				copy_byte_swapped(out_buffer+2,sizeof(long),(char *)&message_size,
					calibration_big_endian);
				out_buffer_size=2+sizeof(long);
				copy_byte_swapped(out_buffer+out_buffer_size,sizeof(number_of_channels),
					(char *)&number_of_channels,calibration_big_endian);
				out_buffer_size += sizeof(number_of_channels);
				for (i=0;i<number_of_channels;i++)
				{
					copy_byte_swapped(out_buffer+out_buffer_size,sizeof(int),
						(char *)(channel_numbers+i),calibration_big_endian);
					out_buffer_size += sizeof(int);
				}
				for (i=0;i<number_of_channels;i++)
				{
					copy_byte_swapped(out_buffer+out_buffer_size,sizeof(float),
						(char *)(channel_offsets+i),calibration_big_endian);
					out_buffer_size += sizeof(float);
				}
				for (i=0;i<number_of_channels;i++)
				{
					copy_byte_swapped(out_buffer+out_buffer_size,sizeof(float),
						(char *)(channel_gains+i),calibration_big_endian);
					out_buffer_size += sizeof(float);
				}
				retval=socket_send(calibration_socket,out_buffer,out_buffer_size,0);
				DEALLOCATE(out_buffer);
			}
		}
		else
		{
			/* failed */
			message_size=0;
			out_buffer_size=2+sizeof(long)+message_size;
			if (ALLOCATE(out_buffer,unsigned char,out_buffer_size))
			{
				out_buffer[0]=(unsigned char)0x0;
				out_buffer[1]=calibration_big_endian;
				copy_byte_swapped(out_buffer+2,sizeof(long),(char *)&message_size,
					calibration_big_endian);
				retval=socket_send(calibration_socket,out_buffer,out_buffer_size,0);
				DEALLOCATE(out_buffer);
			}
		}
	}
} /* calibration_callback */

#if defined (USE_SOCKETS)
#if defined (USE_MEMORY_FOR_BACKGROUND)
struct Acquired_samples_information
{
	int number_of_channels;
	unsigned char big_endian,*out_buffer;
	unsigned long number_of_samples;
}; /* struct Acquired_samples_information */

DWORD WINAPI acquired_thread_function(LPVOID acquired_samples_information_void)
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
Sends samples down the acquired socket.
==============================================================================*/
{
	DWORD return_code;
	int i,number_of_channels,retval;
	long message_size,out_buffer_size;
	struct Acquired_samples_information *acquired_samples_information;
	unsigned char big_endian,message_header[2+sizeof(long)],*out_buffer,
		*out_buffer_entry,temp;
	unsigned long number_of_samples;

	ENTER(acquired_thread_function);
	return_code=0;
	if ((acquired_samples_information=
		(struct Acquired_samples_information *)acquired_samples_information_void)&&
		(out_buffer=acquired_samples_information->out_buffer))
	{
		big_endian=acquired_samples_information->big_endian;
		number_of_channels=acquired_samples_information->number_of_channels;
		number_of_samples=acquired_samples_information->number_of_samples;
		if (acquired_socket_mutex)
		{
			if (WAIT_FAILED==WaitForSingleObject(acquired_socket_mutex,INFINITE))
			{
				sprintf(error_string,"acquired_thread_function.  "
					"WaitForSingleObject failed.  Error code %d",WSAGetLastError());
				display_message(ERROR_MESSAGE,error_string);
			}
		}
		if (INVALID_SOCKET!=acquired_socket)
		{
			/* send message down acquired socket */
			message_size=sizeof(number_of_channels)+sizeof(number_of_samples)+
				number_of_channels*number_of_samples*sizeof(short);
			message_header[0]=(unsigned char)0x1;
			message_header[1]=big_endian;
			copy_byte_swapped(message_header+2,sizeof(long),(char *)&message_size,
				big_endian);
			retval=socket_send(acquired_socket,message_header,2+sizeof(long),0);
			out_buffer_size=0;
			copy_byte_swapped(out_buffer+out_buffer_size,sizeof(number_of_channels),
				(char *)&number_of_channels,big_endian);
			out_buffer_size += sizeof(number_of_channels);
			copy_byte_swapped(out_buffer+out_buffer_size,sizeof(number_of_samples),
				(char *)&number_of_samples,big_endian);
			out_buffer_size += sizeof(number_of_samples);
			retval=socket_send(acquired_socket,out_buffer,out_buffer_size,0);
			out_buffer_size=number_of_samples*number_of_channels*sizeof(short);
			/* in-line for speed */
#if defined (BYTE_ORDER) && (1234==BYTE_ORDER)
			if (!big_endian)
#else /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
			if (big_endian)
#endif /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
			{
				out_buffer_entry=out_buffer;
				for (i=number_of_samples*number_of_channels;i>0;i--)
				{
					temp=out_buffer_entry[0];
					out_buffer_entry[0]=out_buffer_entry[1];
					out_buffer_entry[1]=temp;
					out_buffer_entry += 2;
				}
			}
			retval=socket_send(acquired_socket,out_buffer,out_buffer_size,0);
		}
		if (acquired_socket_mutex)
		{
			ReleaseMutex(acquired_socket_mutex);
		}
		DEALLOCATE(out_buffer);
		DEALLOCATE(acquired_samples_information);
	}
	LEAVE;

	return (return_code);
} /* acquired_thread_function */
#else /* defined (USE_MEMORY_FOR_BACKGROUND) */
DWORD WINAPI acquired_thread_function(LPVOID acquired_file_void)
/*******************************************************************************
LAST MODIFIED : 15 November 2001

DESCRIPTION :
Reads samples out of the file and sends them down the acquired socket.
==============================================================================*/
{
	DWORD return_code;
	FILE *acquired_file;
	int channel_number,i,j,number_of_channels,retval;
	long message_size,out_buffer_size;
	short *sample;
	unsigned char message_header[2+sizeof(long)],*out_buffer;
	unsigned long number_of_samples;

	ENTER(acquired_thread_function);
	return_code=0;
	if (acquired_file=(FILE *)acquired_file_void)
	{
		if (acquired_socket_mutex)
		{
			if (WAIT_FAILED==WaitForSingleObject(acquired_socket_mutex,INFINITE))
			{
				sprintf(error_string,
				"acquired_thread_function.  WaitForSingleObject failed.  Error code %d",
					WSAGetLastError());
				display_message(ERROR_MESSAGE,error_string);
			}
		}
		rewind(acquired_file);
		fread((char *)&channel_number,sizeof(channel_number),1,acquired_file);
		fread((char *)&number_of_channels,sizeof(number_of_channels),1,
			acquired_file);
		fread((char *)&number_of_samples,sizeof(number_of_samples),1,acquired_file);
		if (INVALID_SOCKET!=acquired_socket)
		{
			/* send message down acquired socket */
			message_size=sizeof(number_of_channels)+sizeof(number_of_samples)+
				number_of_channels*number_of_samples*sizeof(short);
			out_buffer_size=sizeof(number_of_channels)+sizeof(number_of_samples);
			if ((number_of_channels+1)*sizeof(short)>(unsigned)out_buffer_size)
			{
				out_buffer_size=(number_of_channels+1)*sizeof(short);
			}
			if (ALLOCATE(out_buffer,unsigned char,out_buffer_size))
			{
				message_header[0]=(unsigned char)0x1;
				message_header[1]=acquired_big_endian;
				copy_byte_swapped(message_header+2,sizeof(long),(char *)&message_size,
					acquired_big_endian);
				retval=socket_send(acquired_socket,message_header,2+sizeof(long),0);
				out_buffer_size=0;
				copy_byte_swapped(out_buffer+out_buffer_size,sizeof(number_of_channels),
					(char *)&number_of_channels,acquired_big_endian);
				out_buffer_size += sizeof(number_of_channels);
				copy_byte_swapped(out_buffer+out_buffer_size,sizeof(number_of_samples),
					(char *)&number_of_samples,acquired_big_endian);
				out_buffer_size += sizeof(number_of_samples);
				retval=socket_send(acquired_socket,out_buffer,out_buffer_size,0);
				out_buffer_size=number_of_channels*sizeof(short);
				for (i=number_of_samples;i>0;i--)
				{
					fread(out_buffer+sizeof(short),sizeof(short),number_of_channels,
						acquired_file);
					sample=(short *)out_buffer;
					for (j=number_of_channels;j>0;j--)
					{
						copy_byte_swapped((char *)sample,sizeof(short),(char *)(sample+1),
							acquired_big_endian);
						sample++;
					}
					retval=socket_send(acquired_socket,out_buffer,out_buffer_size,0);
				}
				DEALLOCATE(out_buffer);
			}
			else
			{
				message_header[0]=(unsigned char)0x0;
				message_header[1]=acquired_big_endian;
				message_size=0;
				copy_byte_swapped(message_header+2,sizeof(long),(char *)&message_size,
					acquired_big_endian);
				retval=socket_send(acquired_socket,message_header,2+sizeof(long),0);
			}
		}
		/* acquired_file is temporary so it is automatically deleted on closing */
#if defined (WIN32_IO)
		CloseHandle((HANDLE)acquired_file_void);
#else /* defined (WIN32_IO) */
		fclose(acquired_file);
#endif /* defined (WIN32_IO) */
		if (acquired_socket_mutex)
		{
			ReleaseMutex(acquired_socket_mutex);
		}
	}
	LEAVE;

	return (return_code);
} /* acquired_thread_function */
#endif /* defined (USE_MEMORY_FOR_BACKGROUND) */
#endif /* defined (USE_SOCKETS) */

#if defined (USE_SOCKETS)
struct Socket_send_samples_acquired_data
{
	int buffer_size;
	SOCKET send_socket;
	unsigned char big_endian,*buffer;
}; /* struct Socket_send_samples_acquired_data */

static int socket_send_samples_acquired(short int *samples,
	int number_of_samples,void *socket_send_samples_acquired_data_void)
/*******************************************************************************
LAST MODIFIED : 15 November 2001

DESCRIPTION :
Used in conjunction with <unemap_transfer_samples_acquired> when sending samples
back.
==============================================================================*/
{
	int i,number_left,number_to_send,return_code,retval;
	struct Socket_send_samples_acquired_data *socket_send_samples_acquired_data;
	unsigned char *buffer_entry,*sample;

	return_code=0;
	if (samples&&(0<number_of_samples)&&(socket_send_samples_acquired_data=
		(struct Socket_send_samples_acquired_data *)
		socket_send_samples_acquired_data_void))
	{
		/* in-line for speed */
#if defined (BYTE_ORDER) && (1234==BYTE_ORDER)
		if (!(socket_send_samples_acquired_data->big_endian))
#else /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
		if (socket_send_samples_acquired_data->big_endian)
#endif /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
		{
			sample=(unsigned char *)samples;
			number_left=number_of_samples;
			return_code=0;
			do
			{
				number_to_send=number_left;
				if (2*number_to_send>socket_send_samples_acquired_data->buffer_size)
				{
					number_to_send=(socket_send_samples_acquired_data->buffer_size)/2;
				}
				number_left -= number_to_send;
				buffer_entry=socket_send_samples_acquired_data->buffer;
				for (i=number_to_send;i>0;i--)
				{
					buffer_entry[0]=sample[1];
					buffer_entry[1]=sample[0];
					buffer_entry += 2;
					sample += 2;
				}
				retval=socket_send(socket_send_samples_acquired_data->send_socket,
					(unsigned char *)samples,2*number_to_send,0);
				if (SOCKET_ERROR!=retval)
				{
					return_code += retval/2;
				}
				else
				{
					return_code=0;
				}
			} while ((2*number_to_send==retval)&&(number_left>0));
		}
		else
		{
			return_code=socket_send(socket_send_samples_acquired_data->send_socket,
				(unsigned char *)samples,2*number_of_samples,0);
			if (SOCKET_ERROR!=return_code)
			{
				return_code /= 2;
			}
			else
			{
				return_code=0;
			}
		}
	}

	return (return_code);
} /* socket_send_samples_acquired */
#endif /* defined (USE_SOCKETS) */

#if defined (NO_ACQUIRED_THREAD)
FILE *acquired_file;
#endif /* defined (NO_ACQUIRED_THREAD) */

#if defined (USE_SOCKETS)
static int process_message(const unsigned char operation_code,
	const long message_size,const unsigned char big_endian,
	unsigned char **out_buffer_address,long *out_buffer_size_address)
/*******************************************************************************
LAST MODIFIED : 6 August 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code,retval;
	long size,unread_size;
	unsigned char buffer[BUFFER_SIZE],*out_buffer;

	return_code=0;
	unread_size=message_size;
	/* check arguments */
	if (out_buffer_address&&out_buffer_size_address)
	{
		switch (operation_code)
		{
			case UNEMAP_CALIBRATE_CODE:
			{
				return_code=0;
				if (0==message_size)
				{
					calibration_big_endian=big_endian;
					return_code=unemap_calibrate(calibration_callback,(void *)NULL);
				}
				else
				{
					sprintf(error_string,
						"unemap_calibrate.  Incorrect message size %d 0",message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_CHANNEL_VALID_FOR_STIMULATOR_CODE:
			{
				int buffer_position,channel_number,stimulator_number;

				return_code=0;
				size=sizeof(stimulator_number)+sizeof(channel_number);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						buffer_position=0;
						copy_byte_swapped((unsigned char *)&stimulator_number,
							sizeof(stimulator_number),buffer+buffer_position,big_endian);
						buffer_position += sizeof(stimulator_number);
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer+buffer_position,big_endian);
						scrolling_big_endian=big_endian;
						return_code=unemap_channel_valid_for_stimulator(stimulator_number,
							channel_number);
					}
				}
				else
				{
					sprintf(error_string,
					"unemap_channel_valid_for_stimulator.  Incorrect message size %d %d",
						message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_CLEAR_SCROLLING_CHANNELS_CODE:
			{
				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_clear_scrolling_channels();
				}
				else
				{
					sprintf(error_string,
						"unemap_clear_scrolling_channels.  Incorrect message size %d 0",
						message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_CONFIGURE_CODE:
			{
				float sampling_frequency,scrolling_refresh_frequency;
				int buffer_position,number_of_samples_in_buffer,synchronization_card;

				return_code=0;
				size=sizeof(sampling_frequency)+sizeof(number_of_samples_in_buffer)+
					sizeof(scrolling_refresh_frequency)+sizeof(synchronization_card);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						buffer_position=0;
						copy_byte_swapped((unsigned char *)&sampling_frequency,
							sizeof(sampling_frequency),buffer+buffer_position,big_endian);
						buffer_position += sizeof(sampling_frequency);
						copy_byte_swapped((unsigned char *)&number_of_samples_in_buffer,
							sizeof(number_of_samples_in_buffer),buffer+buffer_position,
							big_endian);
						buffer_position += sizeof(number_of_samples_in_buffer);
						copy_byte_swapped((unsigned char *)&scrolling_refresh_frequency,
							sizeof(scrolling_refresh_frequency),buffer+buffer_position,
							big_endian);
						buffer_position += sizeof(scrolling_refresh_frequency);
						copy_byte_swapped((unsigned char *)&synchronization_card,
							sizeof(synchronization_card),buffer+buffer_position,big_endian);
						scrolling_big_endian=big_endian;
						return_code=unemap_configure(sampling_frequency,
							number_of_samples_in_buffer,(HWND)NULL,(UINT)0,scrolling_callback,
							(void *)NULL,scrolling_refresh_frequency,synchronization_card);
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_configure.  Incorrect message size %d %d",message_size,
						size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_CONFIGURED_CODE:
			{
				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_configured();
				}
				else
				{
					sprintf(error_string,
						"unemap_configured.  Incorrect message size %d 0",message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_DECONFIGURE_CODE:
			{
				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_deconfigure();
/*					close_connection();*/
				}
				else
				{
					sprintf(error_string,
						"unemap_deconfigure.  Incorrect message size %d 0",message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_GET_ANTIALIASING_FILTER_FREQUENCY_CODE:
			{
				float frequency;
				int channel_number;

				return_code=0;
				size=sizeof(channel_number);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						return_code=unemap_get_antialiasing_filter_frequency(channel_number,
							&frequency);
						if (return_code)
						{
							size=sizeof(frequency);
							if (ALLOCATE(out_buffer,unsigned char,size))
							{
								copy_byte_swapped(out_buffer,sizeof(frequency),
									(char *)&frequency,big_endian);
								*out_buffer_address=out_buffer;
								*out_buffer_size_address=size;
							}
							else
							{
								return_code=0;
							}
						}
					}
				}
				else
				{
					sprintf(error_string,
			"unemap_get_antialiasing_filter_frequency.  Incorrect message size %d %d",
						message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_GET_CARD_STATE_CODE:
			{
				float filter_frequency,tol_settling;
				int battA_state,battGood_state,channel_number,filter_taps,GA0_state,
					GA1_state,i,input_mode,NI_gain,polarity,sampling_interval,
					settling_step_max;
				unsigned char shift_registers[10];

				return_code=0;
				size=sizeof(channel_number);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						return_code=unemap_get_card_state(channel_number,&battA_state,
							&battGood_state,&filter_frequency,&filter_taps,shift_registers,
							&GA0_state,&GA1_state,&NI_gain,&input_mode,&polarity,
							&tol_settling,&sampling_interval,&settling_step_max);
						if (return_code)
						{
							size=sizeof(battA_state)+sizeof(battGood_state)+
								sizeof(filter_frequency)+sizeof(filter_taps)+10+
								sizeof(GA0_state)+sizeof(GA1_state)+sizeof(NI_gain)+
								sizeof(input_mode)+sizeof(polarity)+sizeof(tol_settling)+
								sizeof(sampling_interval)+sizeof(settling_step_max);
							if (ALLOCATE(out_buffer,unsigned char,size))
							{
								size=0;
								copy_byte_swapped(out_buffer+size,sizeof(battA_state),
									(char *)&battA_state,big_endian);
								size += sizeof(battA_state);
								copy_byte_swapped(out_buffer+size,sizeof(battGood_state),
									(char *)&battGood_state,big_endian);
								size += sizeof(battGood_state);
								copy_byte_swapped(out_buffer+size,sizeof(filter_frequency),
									(char *)&filter_frequency,big_endian);
								size += sizeof(filter_frequency);
								copy_byte_swapped(out_buffer+size,sizeof(filter_taps),
									(char *)&filter_taps,big_endian);
								size += sizeof(filter_taps);
								for (i=0;i<10;i++)
								{
									out_buffer[size]=shift_registers[i];
									size++;
								}
								copy_byte_swapped(out_buffer+size,sizeof(GA0_state),
									(char *)&GA0_state,big_endian);
								size += sizeof(GA0_state);
								copy_byte_swapped(out_buffer+size,sizeof(GA1_state),
									(char *)&GA1_state,big_endian);
								size += sizeof(GA1_state);
								copy_byte_swapped(out_buffer+size,sizeof(NI_gain),
									(char *)&NI_gain,big_endian);
								size += sizeof(NI_gain);
								copy_byte_swapped(out_buffer+size,sizeof(input_mode),
									(char *)&input_mode,big_endian);
								size += sizeof(input_mode);
								copy_byte_swapped(out_buffer+size,sizeof(polarity),
									(char *)&polarity,big_endian);
								size += sizeof(polarity);
								copy_byte_swapped(out_buffer+size,sizeof(tol_settling),
									(char *)&tol_settling,big_endian);
								size += sizeof(tol_settling);
								copy_byte_swapped(out_buffer+size,sizeof(sampling_interval),
									(char *)&sampling_interval,big_endian);
								size += sizeof(sampling_interval);
								copy_byte_swapped(out_buffer+size,sizeof(settling_step_max),
									(char *)&settling_step_max,big_endian);
								size += sizeof(settling_step_max);
								*out_buffer_address=out_buffer;
								*out_buffer_size_address=size;
							}
							else
							{
								return_code=0;
							}
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_get_card_state.  Incorrect message size %d %d",
						message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_GET_CHANNEL_STIMULATING_CODE:
			{
				int channel_number,stimulating;

				return_code=0;
				size=sizeof(channel_number);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						return_code=unemap_get_channel_stimulating(channel_number,
							&stimulating);
						if (return_code)
						{
							size=sizeof(stimulating);
							if (ALLOCATE(out_buffer,unsigned char,size))
							{
								copy_byte_swapped(out_buffer,sizeof(stimulating),
									(char *)&stimulating,big_endian);
								*out_buffer_address=out_buffer;
								*out_buffer_size_address=size;
							}
							else
							{
								return_code=0;
							}
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_get_channel_stimulating.  Incorrect message size %d %d",
						message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_GET_GAIN_CODE:
			{
				float post_filter_gain,pre_filter_gain;
				int channel_number;

				return_code=0;
				size=sizeof(channel_number);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						return_code=unemap_get_gain(channel_number,&pre_filter_gain,
							&post_filter_gain);
						if (return_code)
						{
							size=sizeof(pre_filter_gain)+sizeof(post_filter_gain);
							if (ALLOCATE(out_buffer,unsigned char,size))
							{
								copy_byte_swapped(out_buffer,sizeof(pre_filter_gain),
									(char *)&pre_filter_gain,big_endian);
								copy_byte_swapped(out_buffer+sizeof(pre_filter_gain),
									sizeof(post_filter_gain),(char *)&post_filter_gain,
									big_endian);
								*out_buffer_address=out_buffer;
								*out_buffer_size_address=size;
							}
							else
							{
								return_code=0;
							}
						}
					}
				}
				else
				{
					sprintf(error_string,"unemap_get_gain.  Incorrect message size %d %d",
						message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_GET_HARDWARE_VERSION_CODE:
			{
				int hardware_version;

				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_get_hardware_version(&hardware_version);
					if (return_code)
					{
						size=sizeof(hardware_version);
						if (ALLOCATE(out_buffer,unsigned char,size))
						{
							copy_byte_swapped(out_buffer,sizeof(hardware_version),
								(char *)&hardware_version,big_endian);
							*out_buffer_address=out_buffer;
							*out_buffer_size_address=size;
						}
						else
						{
							return_code=0;
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_get_hardware_version.  Incorrect message size %d 0",
						message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_GET_ISOLATE_RECORD_MODE_CODE:
			{
				int channel_number,isolate;

				return_code=0;
				size=sizeof(channel_number);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						return_code=unemap_get_isolate_record_mode(channel_number,
							&isolate);
						if (return_code)
						{
							size=sizeof(isolate);
							if (ALLOCATE(out_buffer,unsigned char,size))
							{
								copy_byte_swapped(out_buffer,sizeof(isolate),
									(char *)&isolate,big_endian);
								*out_buffer_address=out_buffer;
								*out_buffer_size_address=size;
							}
							else
							{
								return_code=0;
							}
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_get_isolate_record_mode.  Incorrect message size %d %d",
						message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_GET_MAXIMUM_NUMBER_OF_SAMPLES_CODE:
			{
				unsigned long number_of_samples;

				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_get_maximum_number_of_samples(&number_of_samples);
					if (return_code)
					{
						size=sizeof(number_of_samples);
						if (ALLOCATE(out_buffer,unsigned char,size))
						{
							copy_byte_swapped(out_buffer,sizeof(number_of_samples),
								(char *)&number_of_samples,big_endian);
							*out_buffer_address=out_buffer;
							*out_buffer_size_address=size;
						}
						else
						{
							return_code=0;
						}
					}
				}
				else
				{
					sprintf(error_string,
					"unemap_get_maximum_number_of_samples.  Incorrect message size %d 0",
						message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_GET_NUMBER_OF_CHANNELS_CODE:
			{
				int number_of_channels;

				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_get_number_of_channels(&number_of_channels);
					if (return_code)
					{
						size=sizeof(number_of_channels);
						if (ALLOCATE(out_buffer,unsigned char,size))
						{
							copy_byte_swapped(out_buffer,sizeof(number_of_channels),
								(char *)&number_of_channels,big_endian);
							*out_buffer_address=out_buffer;
							*out_buffer_size_address=size;
						}
						else
						{
							return_code=0;
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_get_number_of_channels.  Incorrect message size %d 0",
						message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_GET_NUMBER_OF_SAMPLES_ACQUIRED_CODE:
			{
				unsigned long number_of_samples;

				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_get_number_of_samples_acquired(&number_of_samples);
					if (return_code)
					{
						size=sizeof(number_of_samples);
						if (ALLOCATE(out_buffer,unsigned char,size))
						{
							copy_byte_swapped(out_buffer,sizeof(number_of_samples),
								(char *)&number_of_samples,big_endian);
							*out_buffer_address=out_buffer;
							*out_buffer_size_address=size;
						}
						else
						{
							return_code=0;
						}
					}
				}
				else
				{
					sprintf(error_string,
					"unemap_get_number_of_samples_acquired.  Incorrect message size %d 0",
						message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_GET_NUMBER_OF_STIMULATORS_CODE:
			{
				int number_of_stimulators;

				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_get_number_of_stimulators(&number_of_stimulators);
					if (return_code)
					{
						size=sizeof(number_of_stimulators);
						if (ALLOCATE(out_buffer,unsigned char,size))
						{
							copy_byte_swapped(out_buffer,sizeof(number_of_stimulators),
								(char *)&number_of_stimulators,big_endian);
							*out_buffer_address=out_buffer;
							*out_buffer_size_address=size;
						}
						else
						{
							return_code=0;
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_get_number_of_stimulators.  Incorrect message size %d 0",
						message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_GET_POWER_CODE:
			{
				int on;

				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_get_power(&on);
					if (return_code)
					{
						size=sizeof(on);
						if (ALLOCATE(out_buffer,unsigned char,size))
						{
							copy_byte_swapped(out_buffer,sizeof(on),(char *)&on,big_endian);
							*out_buffer_address=out_buffer;
							*out_buffer_size_address=size;
						}
						else
						{
							return_code=0;
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_get_power.  Incorrect message size %d 0",message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_GET_SAMPLE_RANGE_CODE:
			{
				long int maximum_sample_value,minimum_sample_value;
				int channel_number;

				return_code=0;
				size=sizeof(channel_number);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						return_code=unemap_get_sample_range(channel_number,
							&minimum_sample_value,&maximum_sample_value);
						if (return_code)
						{
							size=sizeof(minimum_sample_value)+sizeof(maximum_sample_value);
							if (ALLOCATE(out_buffer,unsigned char,size))
							{
								copy_byte_swapped(out_buffer,sizeof(minimum_sample_value),
									(char *)&minimum_sample_value,big_endian);
								copy_byte_swapped(out_buffer+sizeof(minimum_sample_value),
									sizeof(maximum_sample_value),(char *)&maximum_sample_value,
									big_endian);
								*out_buffer_address=out_buffer;
								*out_buffer_size_address=size;
							}
							else
							{
								return_code=0;
							}
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_get_sample_range.  Incorrect message size %d 0",
						message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_GET_SAMPLES_ACQUIRED_CODE:
			{
				int channel_number,number_of_channels,number_of_samples_written,
					requested_number_of_samples,specify_number_of_samples;
				struct Socket_send_samples_acquired_data
					socket_send_samples_acquired_data;
				unsigned char message_header[2+sizeof(long)+sizeof(int)+
					sizeof(unsigned long)];
				unsigned long acknowledgement,number_of_samples;

				/*???DB.  Go back to using command socket for data transfer because
					client has a separate thread watching the acquired socket and don't
					want it activated for this */
				return_code=0;
				if ((sizeof(channel_number)==message_size)||(sizeof(channel_number)+
					sizeof(requested_number_of_samples)==message_size))
				{
					if (sizeof(channel_number)==message_size)
					{
						specify_number_of_samples=0;
					}
					else
					{
						specify_number_of_samples=1;
					}
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						if (return_code=unemap_get_number_of_samples_acquired(
							&number_of_samples))
						{
							if (specify_number_of_samples)
							{
								copy_byte_swapped((unsigned char *)&requested_number_of_samples,
									sizeof(requested_number_of_samples),
									buffer+sizeof(channel_number),big_endian);
								if ((0<requested_number_of_samples)&&
									((unsigned long)requested_number_of_samples<
									number_of_samples))
								{
									number_of_samples=(unsigned long)requested_number_of_samples;
								}
							}
							if (0==channel_number)
							{
								if (!(return_code=unemap_get_number_of_channels(
									&number_of_channels)))
								{
									sprintf(error_string,
					"unemap_get_samples_acquired.  unemap_get_number_of_channels failed");
									display_message(ERROR_MESSAGE,error_string);
								}
							}
							else
							{
								number_of_channels=1;
							}
							/* send acknowledgement so that client can start retrieving */
							message_header[0]=(unsigned char)0x1;
							message_header[1]=(unsigned char)0x0;
							acknowledgement=0;
							copy_byte_swapped(message_header+2,sizeof(acknowledgement),
								(char *)&acknowledgement,big_endian);
							retval=socket_send(command_socket,message_header,
								2+sizeof(acknowledgement),0);
							/* send down the command socket */
							socket_send_samples_acquired_data.buffer_size=
								number_of_channels*sizeof(short int);
							if (ALLOCATE(socket_send_samples_acquired_data.buffer,
								unsigned char,socket_send_samples_acquired_data.buffer_size))
							{
								socket_send_samples_acquired_data.send_socket=command_socket;
								socket_send_samples_acquired_data.big_endian=big_endian;
								message_header[0]=(unsigned char)0x1;
								message_header[1]=big_endian;
								size=sizeof(number_of_channels)+sizeof(number_of_samples)+
									number_of_samples*number_of_channels*sizeof(short int);
								copy_byte_swapped(message_header+2,sizeof(long),(char *)&size,
									big_endian);
								copy_byte_swapped(message_header+2+sizeof(long),
									sizeof(number_of_channels),(char *)&number_of_channels,
									big_endian);
								copy_byte_swapped(message_header+2+sizeof(long)+
									sizeof(number_of_channels),sizeof(number_of_samples),
									(char *)&number_of_samples,big_endian);
								retval=socket_send(
									socket_send_samples_acquired_data.send_socket,message_header,
									2+sizeof(long)+sizeof(number_of_channels)+
									sizeof(number_of_samples),0);
								return_code=unemap_transfer_samples_acquired(channel_number,
									(int)number_of_samples,socket_send_samples_acquired,
									&socket_send_samples_acquired_data,
									&number_of_samples_written);
								DEALLOCATE(socket_send_samples_acquired_data.buffer);
							}
							else
							{
								return_code=0;
								sprintf(error_string,"unemap_get_samples_acquired.  "
									"Could not allocate samples buffer %ld",
									socket_send_samples_acquired_data.buffer_size);
								display_message(ERROR_MESSAGE,error_string);
							}
						}
						else
						{
							sprintf(error_string,
	"unemap_get_samples_acquired.  unemap_get_number_of_samples_acquired failed");
							display_message(ERROR_MESSAGE,error_string);
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_get_samples_acquired.  Incorrect message size %d",
						message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
#if defined (USE_MEMORY_FOR_BACKGROUND)
			case UNEMAP_GET_SAMPLES_ACQUIRED_BACKGROUND_CODE:
			{
#if !defined (NO_ACQUIRED_THREAD)
				DWORD acquired_thread_id;
				HANDLE acquired_thread;
#endif /* !defined (NO_ACQUIRED_THREAD) */
				int channel_number,number_of_channels,requested_number_of_samples,
					specify_number_of_samples;
				struct Acquired_samples_information *acquired_samples_information;
				unsigned long number_of_samples;

				return_code=0;
				if ((sizeof(channel_number)==message_size)||(sizeof(channel_number)+
					sizeof(requested_number_of_samples)==message_size))
				{
					if (sizeof(channel_number)==message_size)
					{
						specify_number_of_samples=0;
					}
					else
					{
						specify_number_of_samples=1;
					}
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						if (return_code=unemap_get_number_of_samples_acquired(
							&number_of_samples))
						{
							if (specify_number_of_samples)
							{
								copy_byte_swapped((unsigned char *)&requested_number_of_samples,
									sizeof(requested_number_of_samples),
									buffer+sizeof(channel_number),big_endian);
								if ((0<requested_number_of_samples)&&
									((unsigned long)requested_number_of_samples<
									number_of_samples))
								{
									number_of_samples=(unsigned long)requested_number_of_samples;
								}
							}
							if (0==channel_number)
							{
								if (!(return_code=unemap_get_number_of_channels(
									&number_of_channels)))
								{
									sprintf(error_string,
					"unemap_get_samples_acquired.  unemap_get_number_of_channels failed");
									display_message(ERROR_MESSAGE,error_string);
								}
							}
							else
							{
								number_of_channels=1;
							}
							size=number_of_samples*number_of_channels*sizeof(short int);
							if (return_code)
							{
								ALLOCATE(acquired_samples_information,
									struct Acquired_samples_information,1);
								ALLOCATE(out_buffer,unsigned char,size);
								if (acquired_samples_information&&out_buffer)
								{
									acquired_samples_information->out_buffer=out_buffer;
									acquired_samples_information->number_of_samples=
										number_of_samples;
									acquired_samples_information->number_of_channels=
										number_of_channels;
									acquired_samples_information->big_endian=big_endian;
									return_code=unemap_get_samples_acquired(channel_number,
										number_of_samples,(short int *)out_buffer,(int *)NULL);
									if (return_code)
									{
										acquired_big_endian=big_endian;
#if defined (NO_ACQUIRED_THREAD)
										return_code=1;
#else /* defined (NO_ACQUIRED_THREAD) */
										/* spawn the background process for sending the samples
											back */
										if (acquired_thread=CreateThread(
											/*no security attributes*/NULL,
											/*use default stack size*/0,
											acquired_thread_function,
											(LPVOID)acquired_samples_information,
											/*use default creation flags*/0,&acquired_thread_id))
										{
											return_code=1;
										}
										else
										{
											return_code=0;
											DEALLOCATE(acquired_samples_information);
											DEALLOCATE(out_buffer);
											sprintf(error_string,
												"unemap_get_samples_acquired_background.  "
												"Could not create thread");
											display_message(ERROR_MESSAGE,error_string);
										}
#endif /* defined (NO_ACQUIRED_THREAD) */
									}
								}
								else
								{
									return_code=0;
									DEALLOCATE(acquired_samples_information);
									DEALLOCATE(out_buffer);
									sprintf(error_string,
										"unemap_get_samples_acquired_background.  "
										"Could not allocate out_buffer %ld",size);
									display_message(ERROR_MESSAGE,error_string);
								}
							}
						}
						else
						{
							sprintf(error_string,"unemap_get_samples_acquired_background.  "
								"unemap_get_number_of_samples_acquired failed");
							display_message(ERROR_MESSAGE,error_string);
						}
					}
				}
				else
				{
					sprintf(error_string,"unemap_get_samples_acquired_background.  "
						"Incorrect message size %d %d",message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
#else /* defined (USE_MEMORY_FOR_BACKGROUND) */
			case UNEMAP_GET_SAMPLES_ACQUIRED_BACKGROUND_CODE:
			{
#if !defined (NO_ACQUIRED_THREAD)
				DWORD acquired_thread_id;
				HANDLE acquired_thread;
#if defined (WIN32_IO)
				HANDLE acquired_file;
#else /* defined (WIN32_IO) */
				FILE *acquired_file;
#endif /* defined (WIN32_IO) */
#endif /* !defined (NO_ACQUIRED_THREAD) */
				int channel_number,number_of_samples,specify_number_of_samples;

				return_code=0;
				if ((sizeof(channel_number)==message_size)||(sizeof(channel_number)+
					sizeof(number_of_samples)==message_size))
				{
					if (sizeof(channel_number)==message_size)
					{
						specify_number_of_samples=0;
					}
					else
					{
						specify_number_of_samples=1;
					}
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						if (specify_number_of_samples)
						{
							copy_byte_swapped((unsigned char *)&number_of_samples,
								sizeof(number_of_samples),buffer+sizeof(channel_number),
								big_endian);
						}
						else
						{
							number_of_samples=0;
						}
#if defined (WIN32_IO)
						if (acquired_file=CreateFile("c:\\unemap\\bin\\save.sig",
							(DWORD)(GENERIC_READ|GENERIC_WRITE),(DWORD)0,
							(LPSECURITY_ATTRIBUTES)NULL,(DWORD)CREATE_ALWAYS,
							(DWORD)FILE_ATTRIBUTE_NORMAL,(HANDLE)NULL))
#else /* defined (WIN32_IO) */
						/*???debug */
/*						if (acquired_file=fopen_UNEMAP_HARDWARE("save.sig","wb"))*/
						if (acquired_file=tmpfile())
#endif /* defined (WIN32_IO) */
						{
							if (unemap_write_samples_acquired(channel_number,
								number_of_samples,acquired_file,(int *)NULL))
							{
								acquired_big_endian=big_endian;
#if defined (NO_ACQUIRED_THREAD)
								return_code=1;
#else /* defined (NO_ACQUIRED_THREAD) */
								/* spawn the background process for sending the samples back */
								if (acquired_thread=CreateThread(
									/*no security attributes*/NULL,/*use default stack size*/0,
									acquired_thread_function,(LPVOID)acquired_file,
									/*use default creation flags*/0,&acquired_thread_id))
								{
									return_code=1;
								}
								else
								{
									sprintf(error_string,
						"unemap_get_samples_acquired_background.  Could not create thread");
									display_message(ERROR_MESSAGE,error_string);
								}
#endif /* defined (NO_ACQUIRED_THREAD) */
							}
							else
							{
								sprintf(error_string,"unemap_get_samples_acquired_background.  "
									"unemap_write_samples_acquired failed");
								display_message(ERROR_MESSAGE,error_string);
							}
						}
						else
						{
							sprintf(error_string,"unemap_get_samples_acquired_background.  "
								"Could not open temporary file");
							display_message(ERROR_MESSAGE,error_string);
						}
					}
				}
				else
				{
					sprintf(error_string,"unemap_get_samples_acquired_background.  "
						"Incorrect message size %d",message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
#endif /* defined (USE_MEMORY_FOR_BACKGROUND) */
			case UNEMAP_GET_SAMPLING_CODE:
			{
				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_get_sampling();
				}
				else
				{
					sprintf(error_string,
						"unemap_get_sampling.  Incorrect message size %d 0",message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_GET_SAMPLING_FREQUENCY_CODE:
			{
				float frequency;

				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_get_sampling_frequency(&frequency);
					if (return_code)
					{
						size=sizeof(frequency);
						if (ALLOCATE(out_buffer,unsigned char,size))
						{
							copy_byte_swapped(out_buffer,sizeof(frequency),
								(char *)&frequency,big_endian);
							*out_buffer_address=out_buffer;
							*out_buffer_size_address=size;
						}
						else
						{
							return_code=0;
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_get_sampling_frequency.  Incorrect message size %d 0",
						message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_GET_SERVICE_VERSION_CODE:
			{
				int service_version;

				return_code=0;
				if (0==message_size)
				{
					service_version=(int)SERVICE_VERSION;
					size=sizeof(service_version);
					if (ALLOCATE(out_buffer,unsigned char,size))
					{
						copy_byte_swapped(out_buffer,sizeof(service_version),
							(char *)&service_version,big_endian);
						*out_buffer_address=out_buffer;
						*out_buffer_size_address=size;
						return_code=1;
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_get_service_version.  Incorrect message size %d 0",
						message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_GET_VOLTAGE_RANGE_CODE:
			{
				float maximum_voltage,minimum_voltage;
				int channel_number;

				return_code=0;
				size=sizeof(channel_number);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						return_code=unemap_get_voltage_range(channel_number,
							&minimum_voltage,&maximum_voltage);
						if (return_code)
						{
							size=sizeof(minimum_voltage)+sizeof(maximum_voltage);
							if (ALLOCATE(out_buffer,unsigned char,size))
							{
								copy_byte_swapped(out_buffer,sizeof(minimum_voltage),
									(char *)&minimum_voltage,big_endian);
								copy_byte_swapped(out_buffer+sizeof(minimum_voltage),
									sizeof(maximum_voltage),(char *)&maximum_voltage,big_endian);
								*out_buffer_address=out_buffer;
								*out_buffer_size_address=size;
							}
							else
							{
								return_code=0;
							}
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_get_voltage_range.  Incorrect message size %d %d",
						message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_LOAD_CURRENT_STIMULATING_CODE:
			{
				float *current,*currents,currents_per_second;
				int *channel_number,*channel_numbers,i,number_of_currents,
					number_of_channels;
				unsigned int number_of_cycles;

				return_code=0;
				size=2*sizeof(int)+sizeof(float)+sizeof(unsigned int);
				if (size<=message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						size=0;
						copy_byte_swapped((unsigned char *)&number_of_channels,
							sizeof(number_of_channels),buffer+size,big_endian);
						size += sizeof(number_of_channels);
						copy_byte_swapped((unsigned char *)&number_of_currents,
							sizeof(number_of_currents),buffer+size,big_endian);
						size += sizeof(number_of_currents);
						copy_byte_swapped((unsigned char *)&currents_per_second,
							sizeof(currents_per_second),buffer+size,big_endian);
						size += sizeof(currents_per_second);
						copy_byte_swapped((unsigned char *)&number_of_cycles,
							sizeof(number_of_cycles),buffer+size,big_endian);
						size += sizeof(number_of_cycles);
						size += number_of_channels*sizeof(int);
						size += number_of_currents*sizeof(float);
						if (size==message_size)
						{
							return_code=1;
							if (0<number_of_channels)
							{
								size=number_of_channels;
								if (ALLOCATE(channel_numbers,int,size+1))
								{
									size *= sizeof(int);
									channel_number=channel_numbers+1;
									retval=socket_recv(command_socket,command_socket_read_event,
										(unsigned char *)channel_number,size,0);
									if (SOCKET_ERROR!=retval)
									{
										unread_size -= retval;
										channel_number=channel_numbers;
										for (i=number_of_channels;i>0;i--)
										{
											copy_byte_swapped((unsigned char *)channel_number,
												sizeof(int),(unsigned char *)(channel_number+1),
												big_endian);
											channel_number++;
										}
									}
									else
									{
										DEALLOCATE(channel_numbers);
										channel_numbers=(int *)NULL;
										return_code=0;
									}
								}
								else
								{
									sprintf(error_string,
			"unemap_load_current_stimulating.  Could not allocate channel numbers %d",
										number_of_channels);
									display_message(ERROR_MESSAGE,error_string);
									return_code=0;
								}
							}
							else
							{
								channel_numbers=(int *)NULL;
							}
							out_buffer=(unsigned char *)NULL;
							if (return_code&&(0<number_of_currents))
							{
								size=number_of_currents*sizeof(float);
								if (ALLOCATE(out_buffer,unsigned char,size+sizeof(float)))
								{
									retval=socket_recv(command_socket,command_socket_read_event,
										out_buffer,size,0);
									if (SOCKET_ERROR!=retval)
									{
										unread_size -= retval;
										currents=((float *)out_buffer)+1;
										current=currents+(number_of_currents-1);
										for (i=number_of_currents;i>0;i--)
										{
											copy_byte_swapped((unsigned char *)current,sizeof(float),
												(unsigned char *)(current-1),big_endian);
											current--;
										}
									}
									else
									{
										DEALLOCATE(out_buffer);
										out_buffer=(unsigned char *)NULL;
										return_code=0;
									}
								}
								else
								{
									sprintf(error_string,
						"unemap_load_current_stimulating.  Could not allocate currents %d",
										number_of_currents);
									display_message(ERROR_MESSAGE,error_string);
									return_code=0;
								}
							}
							else
							{
								currents=(float *)NULL;
							}
							if (return_code)
							{
								if (return_code=unemap_load_current_stimulating(
									number_of_channels,channel_numbers,number_of_currents,
									currents_per_second,currents,number_of_cycles,
									(Unemap_stimulation_end_callback *)NULL,(void *)NULL))
								{
									if (out_buffer)
									{
										current=(float *)out_buffer;
										for (i=number_of_currents;i>0;i--)
										{
											copy_byte_swapped((unsigned char *)current,sizeof(float),
												(unsigned char *)(current+1),big_endian);
											current++;
										}
										*out_buffer_address=out_buffer;
										*out_buffer_size_address=size;
									}
								}
								else
								{
									if (out_buffer)
									{
										DEALLOCATE(out_buffer);
										out_buffer=(unsigned char *)NULL;
									}
								}
							}
							if (channel_numbers)
							{
								DEALLOCATE(channel_numbers);
								channel_numbers=(int *)NULL;
							}
						}
						else
						{
							sprintf(error_string,
							"unemap_load_current_stimulating.  Incorrect message size %d %d",
								message_size,size);
							display_message(ERROR_MESSAGE,error_string);
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_load_current_stimulating.  Incorrect message size %d>=%d",
						message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_LOAD_VOLTAGE_STIMULATING_CODE:
			{
				float *voltage,*voltages,voltages_per_second;
				int *channel_number,*channel_numbers,i,number_of_voltages,
					number_of_channels;
				unsigned int number_of_cycles;

				return_code=0;
				size=2*sizeof(int)+sizeof(float)+sizeof(unsigned int);
				if (size<=message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						size=0;
						copy_byte_swapped((unsigned char *)&number_of_channels,
							sizeof(number_of_channels),buffer+size,big_endian);
						size += sizeof(number_of_channels);
						copy_byte_swapped((unsigned char *)&number_of_voltages,
							sizeof(number_of_voltages),buffer+size,big_endian);
						size += sizeof(number_of_voltages);
						copy_byte_swapped((unsigned char *)&voltages_per_second,
							sizeof(voltages_per_second),buffer+size,big_endian);
						size += sizeof(voltages_per_second);
						copy_byte_swapped((unsigned char *)&number_of_cycles,
							sizeof(number_of_cycles),buffer+size,big_endian);
						size += sizeof(number_of_cycles);
						size += number_of_channels*sizeof(int);
						size += number_of_voltages*sizeof(float);
						if (size==message_size)
						{
							return_code=1;
							if (0<number_of_channels)
							{
								size=number_of_channels;
								if (ALLOCATE(channel_numbers,int,size+1))
								{
									size *= sizeof(int);
									channel_number=channel_numbers+1;
									retval=socket_recv(command_socket,command_socket_read_event,
										(unsigned char *)channel_number,size,0);
									if (SOCKET_ERROR!=retval)
									{
										unread_size -= retval;
										channel_number=channel_numbers;
										for (i=number_of_channels;i>0;i--)
										{
											copy_byte_swapped((unsigned char *)channel_number,
												sizeof(int),(unsigned char *)(channel_number+1),
												big_endian);
											channel_number++;
										}
									}
									else
									{
										DEALLOCATE(channel_numbers);
										channel_numbers=(int *)NULL;
										return_code=0;
									}
								}
								else
								{
									sprintf(error_string,
		"unemap_load_voltage_stimulating.  Could not allocate channel numbers %d",
										number_of_channels);
									display_message(ERROR_MESSAGE,error_string);
									return_code=0;
								}
							}
							else
							{
								channel_numbers=(int *)NULL;
							}
							out_buffer=(unsigned char *)NULL;
							if (return_code&&(0<number_of_voltages))
							{
								size=number_of_voltages*sizeof(float);
								if (ALLOCATE(out_buffer,unsigned char,size+sizeof(float)))
								{
									retval=socket_recv(command_socket,command_socket_read_event,
										out_buffer,size,0);
									if (SOCKET_ERROR!=retval)
									{
										unread_size -= retval;
										voltages=((float *)out_buffer)+1;
										voltage=voltages+(number_of_voltages-1);
										for (i=number_of_voltages;i>0;i--)
										{
											copy_byte_swapped((unsigned char *)voltage,sizeof(float),
												(unsigned char *)(voltage-1),big_endian);
											voltage--;
										}
									}
									else
									{
										DEALLOCATE(out_buffer);
										out_buffer=(unsigned char *)NULL;
										return_code=0;
									}
								}
								else
								{
									sprintf(error_string,
						"unemap_load_voltage_stimulating.  Could not allocate voltages %d",
										number_of_voltages);
									display_message(ERROR_MESSAGE,error_string);
									return_code=0;
								}
							}
							else
							{
								voltages=(float *)NULL;
							}
							if (return_code)
							{
								if (return_code=unemap_load_voltage_stimulating(
									number_of_channels,channel_numbers,number_of_voltages,
									voltages_per_second,voltages,number_of_cycles,
									(Unemap_stimulation_end_callback *)NULL,(void *)NULL))
								{
									if (out_buffer)
									{
										voltage=(float *)out_buffer;
										for (i=number_of_voltages;i>0;i--)
										{
											copy_byte_swapped((unsigned char *)voltage,sizeof(float),
												(unsigned char *)(voltage+1),big_endian);
											voltage++;
										}
										*out_buffer_address=out_buffer;
										*out_buffer_size_address=size;
									}
								}
								else
								{
									if (out_buffer)
									{
										DEALLOCATE(out_buffer);
										out_buffer=(unsigned char *)NULL;
									}
								}
							}
							if (channel_numbers)
							{
								DEALLOCATE(channel_numbers);
								channel_numbers=(int *)NULL;
							}
						}
						else
						{
							sprintf(error_string,
							"unemap_load_voltage_stimulating.  Incorrect message size %d %d",
								message_size,size);
							display_message(ERROR_MESSAGE,error_string);
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_load_voltage_stimulating.  Incorrect message size %d>=%d",
						message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_SET_ANTIALIASING_FILTER_FREQUENCY_CODE:
			{
				float frequency;
				int channel_number;

				return_code=0;
				size=sizeof(channel_number)+sizeof(frequency);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						copy_byte_swapped((unsigned char *)&frequency,
							sizeof(frequency),buffer+sizeof(channel_number),big_endian);
						return_code=unemap_set_antialiasing_filter_frequency(channel_number,
							frequency);
					}
				}
				else
				{
					sprintf(error_string,
			"unemap_set_antialiasing_filter_frequency.  Incorrect message size %d %d",
						message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_SET_CHANNEL_STIMULATING_CODE:
			{
				int channel_number,stimulating;

				return_code=0;
				size=sizeof(channel_number)+sizeof(stimulating);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						copy_byte_swapped((unsigned char *)&stimulating,
							sizeof(stimulating),buffer+sizeof(channel_number),big_endian);
						return_code=unemap_set_channel_stimulating(channel_number,
							stimulating);
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_set_channel_stimulating.  Incorrect message size %d %d",
						message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_SET_GAIN_CODE:
			{
				float post_filter_gain,pre_filter_gain;
				int channel_number;

				return_code=0;
				size=sizeof(channel_number)+sizeof(pre_filter_gain)+
					sizeof(post_filter_gain);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						copy_byte_swapped((unsigned char *)&pre_filter_gain,
							sizeof(pre_filter_gain),buffer+sizeof(channel_number),big_endian);
						copy_byte_swapped((unsigned char *)&post_filter_gain,
							sizeof(post_filter_gain),buffer+sizeof(channel_number)+
							sizeof(pre_filter_gain),big_endian);
						return_code=unemap_set_gain(channel_number,pre_filter_gain,
							post_filter_gain);
					}
				}
				else
				{
					sprintf(error_string,"unemap_set_gain.  Incorrect message size %d %d",
						message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_SET_ISOLATE_RECORD_MODE_CODE:
			{
				int channel_number,isolate;

				return_code=0;
				size=sizeof(channel_number)+sizeof(isolate);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						copy_byte_swapped((unsigned char *)&isolate,
							sizeof(isolate),buffer+sizeof(channel_number),big_endian);
						return_code=unemap_set_isolate_record_mode(channel_number,isolate);
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_set_isolate_record_mode.  Incorrect message size %d %d",
						message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_SET_POWER_CODE:
			{
				int on;

				return_code=0;
				size=sizeof(on);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&on,sizeof(on),buffer,
							big_endian);
						return_code=unemap_set_power(on);
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_set_power.  Incorrect message size %d %d",message_size,
						size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_SET_POWERUP_ANTIALIASING_FILTER_FREQUENCY_CODE:
			{
				int channel_number;

				return_code=0;
				size=sizeof(channel_number);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						return_code=unemap_set_powerup_antialiasing_filter_frequency(
							channel_number);
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_set_powerup_antialiasing_filter_frequency.  "
						"Incorrect message size %d %d",message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_SET_SCROLLING_CHANNEL_CODE:
			{
				int channel_number;

				return_code=0;
				size=sizeof(channel_number);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						return_code=unemap_set_scrolling_channel(channel_number);
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_set_scrolling_channel.  Incorrect message size %d %d",
						message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_SHUTDOWN_CODE:
			{
				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_shutdown();
				}
				else
				{
					sprintf(error_string,
						"unemap_shutdown.  Incorrect message size %d 0",message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_START_CALIBRATING_CODE:
			{
				int channel_number,i,number_of_voltages;
				float *voltage,*voltages,voltages_per_second;

				return_code=0;
				size=2*sizeof(int)+sizeof(float);
				if (size<=message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						size=0;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer+size,big_endian);
						size += sizeof(channel_number);
						copy_byte_swapped((unsigned char *)&number_of_voltages,
							sizeof(number_of_voltages),buffer+size,big_endian);
						size += sizeof(number_of_voltages);
						copy_byte_swapped((unsigned char *)&voltages_per_second,
							sizeof(voltages_per_second),buffer+size,big_endian);
						size += sizeof(voltages_per_second);
						size += number_of_voltages*sizeof(float);
						if (size==message_size)
						{
							out_buffer=(unsigned char *)NULL;
							if (0<number_of_voltages)
							{
								size=number_of_voltages*sizeof(float);
								if (ALLOCATE(out_buffer,unsigned char,size+sizeof(float)))
								{
									retval=socket_recv(command_socket,command_socket_read_event,
										out_buffer,size,0);
									if (SOCKET_ERROR!=retval)
									{
										unread_size -= retval;
										voltages=((float *)out_buffer)+1;
										voltage=voltages+(number_of_voltages-1);
										for (i=number_of_voltages;i>0;i--)
										{
											copy_byte_swapped((unsigned char *)voltage,sizeof(float),
												(unsigned char *)(voltage-1),big_endian);
											voltage--;
										}
										return_code=1;
									}
									else
									{
										DEALLOCATE(out_buffer);
										out_buffer=(unsigned char *)NULL;
									}
								}
								else
								{
									sprintf(error_string,
										"unemap_start_calibrating.  Could not allocate voltages %d",
										number_of_voltages);
									display_message(ERROR_MESSAGE,error_string);
								}
							}
							else
							{
								voltages=(float *)NULL;
								return_code=1;
							}
							if (return_code)
							{
								if (return_code=unemap_start_calibrating(channel_number,
									number_of_voltages,voltages_per_second,voltages))
								{
									if (out_buffer)
									{
										voltage=(float *)out_buffer;
										for (i=number_of_voltages;i>0;i--)
										{
											copy_byte_swapped((unsigned char *)voltage,sizeof(float),
												(unsigned char *)(voltage+1),big_endian);
											voltage++;
										}
										return_code=1;
										*out_buffer_address=out_buffer;
										*out_buffer_size_address=size;
									}
								}
								else
								{
									if (out_buffer)
									{
										DEALLOCATE(out_buffer);
										out_buffer=(unsigned char *)NULL;
									}
								}
							}
						}
						else
						{
							sprintf(error_string,
								"unemap_start_calibrating.  Incorrect message size %d %d",
								message_size,size+number_of_voltages*sizeof(float));
							display_message(ERROR_MESSAGE,error_string);
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_start_calibrating.  Incorrect message size %d>=%d",
						message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_START_SAMPLING_CODE:
			{
				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_start_sampling();
				}
				else
				{
					sprintf(error_string,
						"unemap_start_sampling.  Incorrect message size %d 0",message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_START_SCROLLING_CODE:
			{
				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_start_scrolling();
				}
				else
				{
					sprintf(error_string,
						"unemap_start_scrolling.  Incorrect message size %d 0",
						message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_START_STIMULATING_CODE:
			{
				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_start_stimulating();
				}
				else
				{
					sprintf(error_string,
						"unemap_start_stimulating.  Incorrect message size %d 0",
						message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_STOP_CALIBRATING_CODE:
			{
				int channel_number;

				return_code=0;
				size=sizeof(channel_number);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						return_code=unemap_stop_calibrating(channel_number);
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_stop_stimulating.  Incorrect message size %d %d",
						message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_STOP_SAMPLING_CODE:
			{
				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_stop_sampling();
				}
				else
				{
					sprintf(error_string,
						"unemap_stop_sampling.  Incorrect message size %d 0",message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_STOP_SCROLLING_CODE:
			{
				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_stop_scrolling();
				}
				else
				{
					sprintf(error_string,
						"unemap_stop_scrolling.  Incorrect message size %d 0",message_size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_STOP_STIMULATING_CODE:
			{
				int channel_number;

				return_code=0;
				size=sizeof(channel_number);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						return_code=unemap_stop_stimulating(channel_number);
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_stop_stimulating.  Incorrect message size %d %d",
						message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			case UNEMAP_TOGGLE_SHIFT_REGISTER_CODE:
			{
				int channel_number,register_number;

				return_code=0;
				size=2*sizeof(int);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						size=0;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer+size,big_endian);
						size += sizeof(channel_number);
						copy_byte_swapped((unsigned char *)&register_number,
							sizeof(register_number),buffer+size,big_endian);
						size += sizeof(register_number);
						return_code=unemap_toggle_shift_register(channel_number,
							register_number);
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_toggle_shift_register.  Incorrect message size %d %d",
						message_size,size);
					display_message(ERROR_MESSAGE,error_string);
				}
			} break;
			default:
			{
				/* ignore the message */
				return_code=1;
				size=message_size;
				while (return_code&&(0<size))
				{
					retval=socket_recv(command_socket,command_socket_read_event,buffer,
						message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
					}
					else
					{
						return_code=0;
					}
				}
			} break;
		}
	}
	else
	{
		sprintf(error_string,"process_message.  Invalid arguments %p %p",
			out_buffer_address,out_buffer_size_address);
		display_message(ERROR_MESSAGE,error_string);
	}
	/* clear unread message */
	retval=1;
	while ((SOCKET_ERROR!=retval)&&(unread_size>0))
	{
		if (unread_size>BUFFER_SIZE)
		{
			size=BUFFER_SIZE;
		}
		else
		{
			size=unread_size;
		}
		retval=socket_recv(command_socket,command_socket_read_event,buffer,size,0);
		if (SOCKET_ERROR!=retval)
		{
			unread_size -= size;
		}
	}

	return (return_code);
} /* process_message */
#endif /* defined (USE_SOCKETS) */
#endif /* defined (USE_UNEMAP_HARDWARE) */

/*
Global functions
----------------
*/
VOID ServiceStart(DWORD dwArgc,LPTSTR *lpszArgv)
/*******************************************************************************
LAST MODIFIED : 24 September 2002

DESCRIPTION :
Actual code of the service that does the work.
	dwArgc - number of command line arguments
	lpszArgv - array of command line arguments
==============================================================================*/
{
	DWORD dwWait;
	HANDLE hEvents[2]={NULL,NULL};
#if defined (USE_UNEMAP_HARDWARE)
	unsigned long number_of_channels;
#endif /* defined (USE_UNEMAP_HARDWARE) */
#if defined (USE_SOCKETS)
	int running;
	struct timeval timeout;
#if defined (USE_UNEMAP_HARDWARE)
	int fromlen,last_error,return_code,retval,socket_type=SOCK_STREAM;
	long message_size,out_buffer_size;
	SOCKET acquired_socket_listen,calibration_socket_listen,command_socket_listen,
		reject_socket,scrolling_socket_listen;
	struct sockaddr_in from,local;
	unsigned char big_endian,buffer[BUFFER_SIZE],operation_code,*out_buffer;
	unsigned short port=DEFAULT_PORT;
	WORD wVersionRequested;
	WSADATA wsaData;
	WSANETWORKEVENTS network_events;
#endif /* defined (USE_UNEMAP_HARDWARE) */
#endif /* defined (USE_SOCKETS) */
#if defined (USE_WORMHOLES)
	Wh_input *command_in,*scrolling_in;
	Wh_output *command_out,*scrolling_out;
#endif /* defined (USE_WORMHOLES) */

	/* set up messages */
	set_display_message_function(ERROR_MESSAGE,display_error_message,
		(void *)NULL);
	set_display_message_function(INFORMATION_MESSAGE,display_information_message,
		(void *)NULL);
	set_display_message_function(WARNING_MESSAGE,display_warning_message,
		(void *)NULL);
#if defined (USE_UNEMAP_HARDWARE)
	/* when the NIDAQ DLL is loaded (when the service starts), the NI cards and
		hence the unemap cards are in an unknown state.  This call to
		unemap_get_number_of_channels forces a call to search_for_NI_cards which
		in turn configures the NI cards */
	unemap_get_number_of_channels(&number_of_channels);
#endif /* defined (USE_UNEMAP_HARDWARE) */
	/* service initialization */
	hEvents[0]=NULL;
	hEvents[1]=NULL;
	/* report the status to the service control manager */
	if (ReportStatusToSCMgr(/*service state*/SERVICE_START_PENDING,
		/*exit code*/NO_ERROR,/*wait hint*/3000))
	{
		/* create the stop event object. The control handler function signals this
			event when it receives the "stop" control code */
		hServerStopEvent=CreateEvent(/*no security attributes*/NULL,
			/*manual reset event*/TRUE,/*not-signalled*/FALSE,/*no name*/NULL);
		if (hServerStopEvent)
		{
			hEvents[0]=hServerStopEvent;
			/* report the status to the service control manager */
			if (ReportStatusToSCMgr(/*service state*/SERVICE_START_PENDING,
				/*exit code*/NO_ERROR,/*wait hint*/3000))
			{
				/* create the event object used in overlapped i/o */
				command_socket_read_event=CreateEvent(/*no security attributes*/NULL,
					/*manual reset event*/TRUE,/*not-signalled*/FALSE,
					/*no name*/NULL);
				hEvents[1]=command_socket_read_event;
				if (hEvents[1])
				{
					/* report the status to the service control manager */
					if (ReportStatusToSCMgr(/*service state*/SERVICE_START_PENDING,
						/*exit code*/NO_ERROR,/*wait hint*/3000))
					{
#if defined (USE_SOCKETS)
#if defined (USE_UNEMAP_HARDWARE)
						wVersionRequested=MAKEWORD(2,2);
						if (SOCKET_ERROR!=WSAStartup(wVersionRequested,&wsaData))
						{
							local.sin_family=AF_INET;
							local.sin_addr.s_addr=INADDR_ANY;
								/*???DB.  Could use inet_addr("xxx.xxx.xxx.xxx") */
							/* Port MUST be in Network Byte Order */
								/*???DB.  What does this mean ? */
							command_socket_listen=socket(AF_INET,socket_type,0);
							scrolling_socket_listen=socket(AF_INET,socket_type,0);
							calibration_socket_listen=socket(AF_INET,socket_type,0);
							acquired_socket_listen=socket(AF_INET,socket_type,0);
							acquired_socket_mutex=CreateMutex(/*no security attributes*/NULL,
								/*do not initially own*/FALSE,/*no name*/(LPCTSTR)NULL);
							if ((INVALID_SOCKET!=acquired_socket_listen)&&
								(INVALID_SOCKET!=calibration_socket_listen)&&
								(INVALID_SOCKET!=command_socket_listen)&&
								(INVALID_SOCKET!=scrolling_socket_listen)&&
								acquired_socket_mutex)
							{
								/* can't have different event objects for different network
									events */
								/* this event also applies to command_socket which is created
									with accept from command_socket_listen */
								if (0==WSAEventSelect(command_socket_listen,hEvents[1],
									FD_ACCEPT|FD_READ|FD_CLOSE))
								{
									/* bind() associates a local address and port combination with
										the socket just created. This is most useful when the
										application is a server that has a well-known port that
										clients know about in advance */
									port=DEFAULT_PORT;
									local.sin_port=htons(port);
									if (SOCKET_ERROR!=bind(command_socket_listen,
										(struct sockaddr *)&local,sizeof(local)))
									{
										port=DEFAULT_PORT+1;
										local.sin_port=htons(port);
										if (SOCKET_ERROR!=bind(scrolling_socket_listen,
											(struct sockaddr *)&local,sizeof(local)))
										{
											port=DEFAULT_PORT+2;
											local.sin_port=htons(port);
											if (SOCKET_ERROR!=bind(calibration_socket_listen,
												(struct sockaddr *)&local,sizeof(local)))
											{
												port=DEFAULT_PORT+3;
												local.sin_port=htons(port);
												if (SOCKET_ERROR!=bind(acquired_socket_listen,
													(struct sockaddr *)&local,sizeof(local)))
												{
													if ((SOCKET_ERROR!=listen(command_socket_listen,5))&&
														(SOCKET_ERROR!=listen(scrolling_socket_listen,5))&&
														(SOCKET_ERROR!=listen(calibration_socket_listen,
														5))&&
														(SOCKET_ERROR!=listen(acquired_socket_listen,5)))
													{
#endif /* defined (USE_UNEMAP_HARDWARE) */
														/* report the status to the service control
															manager */
														if (ReportStatusToSCMgr(
															/*service state*/SERVICE_RUNNING,
															/*exit code*/NO_ERROR,/*wait hint*/0))
														{
															running=1;
#if defined (USE_UNEMAP_HARDWARE)
															command_socket=INVALID_SOCKET;
#endif /* defined (USE_UNEMAP_HARDWARE) */
															timeout.tv_sec=0;
															timeout.tv_usec=0;
															while (1==running)
															{
																dwWait=WaitForMultipleObjects(2,hEvents,FALSE,
																	INFINITE);
																if (WAIT_OBJECT_0+1==dwWait)
																{
#if defined (USE_UNEMAP_HARDWARE)
																	WSAEnumNetworkEvents(command_socket_listen,
																		NULL,&network_events);
																	if (network_events.lNetworkEvents&FD_ACCEPT)
																	{
																		if (INVALID_SOCKET==command_socket)
																		{
																			/* make connection */
																			fromlen=sizeof(from);
																			do
																			{
																				command_socket=accept(
																					command_socket_listen,
																					(struct sockaddr *)&from,&fromlen);
																			} while ((
																				INVALID_SOCKET==command_socket)&&
																				(WSAEWOULDBLOCK==
																				(last_error=WSAGetLastError())));
																			if (INVALID_SOCKET!=command_socket)
																			{
																				fromlen=sizeof(from);
																				do
																				{
																					scrolling_socket=accept(
																						scrolling_socket_listen,
																						(struct sockaddr *)&from,&fromlen);
																				} while ((INVALID_SOCKET==
																					scrolling_socket)&&(WSAEWOULDBLOCK==
																					(last_error=WSAGetLastError())));
																				if (INVALID_SOCKET!=scrolling_socket)
																				{
																					fromlen=sizeof(from);
																					do
																					{
																						calibration_socket=accept(
																							calibration_socket_listen,
																							(struct sockaddr *)&from,
																							&fromlen);
																					} while ((INVALID_SOCKET==
																						calibration_socket)&&
																						(WSAEWOULDBLOCK==
																						(last_error=WSAGetLastError())));
																					if (INVALID_SOCKET!=
																						calibration_socket)
																					{
																						fromlen=sizeof(from);
																						do
																						{
																							acquired_socket=accept(
																								acquired_socket_listen,
																								(struct sockaddr *)&from,
																								&fromlen);
																						} while ((INVALID_SOCKET==
																							acquired_socket)&&
																							(WSAEWOULDBLOCK==
																							(last_error=WSAGetLastError())));
																						if (INVALID_SOCKET==acquired_socket)
																						{
																							sprintf(error_string,
																					"Making acquired connection failed");
																							display_message(ERROR_MESSAGE,
																								error_string);
																							closesocket(calibration_socket);
																							calibration_socket=INVALID_SOCKET;
																							closesocket(scrolling_socket);
																							scrolling_socket=INVALID_SOCKET;
																							closesocket(command_socket);
																							command_socket=INVALID_SOCKET;
																						}
																					}
																					else
																					{
																						sprintf(error_string,
																				"Making calibration connection failed");
																						display_message(ERROR_MESSAGE,
																							error_string);
																						closesocket(scrolling_socket);
																						scrolling_socket=INVALID_SOCKET;
																						closesocket(command_socket);
																						command_socket=INVALID_SOCKET;
																					}
																				}
																				else
																				{
																					sprintf(error_string,
																					"Making scrolling connection failed");
																					display_message(ERROR_MESSAGE,
																						error_string);
																					closesocket(command_socket);
																					command_socket=INVALID_SOCKET;
																				}
																			}
																			else
																			{
																				sprintf(error_string,
																					"Making command connection failed");
																				display_message(ERROR_MESSAGE,
																					error_string);
																			}
																		}
																		else
																		{
																			/* reject connection */
																			fromlen=sizeof(from);
																			do
																			{
																				reject_socket=accept(
																					command_socket_listen,
																					(struct sockaddr *)&from,&fromlen);
																			} while ((
																				INVALID_SOCKET==reject_socket)&&
																				(WSAEWOULDBLOCK==
																				(last_error=WSAGetLastError())));
																			if (INVALID_SOCKET!=reject_socket)
																			{
																				closesocket(reject_socket);
																			}
																		}
																	}
																	if (INVALID_SOCKET!=command_socket)
																	{
																		WSAEnumNetworkEvents(command_socket,
																			NULL,&network_events);
																		if (network_events.lNetworkEvents&FD_READ)
																		{
																			retval=socket_recv(command_socket,
																				command_socket_read_event,buffer,
																				MESSAGE_HEADER_SIZE,0);
																			if (SOCKET_ERROR!=retval)
																			{
																				/* decode message header */
																				operation_code=buffer[0];
																				big_endian=buffer[1];
																				copy_byte_swapped((char *)&message_size,
																					sizeof(long),buffer+2,big_endian);
																				out_buffer=(unsigned char *)NULL;
																				out_buffer_size=0;
																				return_code=process_message(
																					operation_code,message_size,
																					big_endian,&out_buffer,
																					&out_buffer_size);
																				ResetEvent(hEvents[1]);
																				if (return_code)
																				{
																					buffer[0]=(unsigned char)0x1;
																				}
																				else
																				{
																					buffer[0]=(unsigned char)0x0;
																				}
																				buffer[1]=(unsigned char)0x0;
																				copy_byte_swapped(buffer+2,sizeof(long),
																					(char *)&out_buffer_size,big_endian);
																				retval=socket_send(command_socket,
																					buffer,MESSAGE_HEADER_SIZE,0);
																				if (0<out_buffer_size)
																				{
																					retval=socket_send(command_socket,
																						out_buffer,out_buffer_size,0);
																					DEALLOCATE(out_buffer);
																					out_buffer=(unsigned char *)NULL;
																				}
#if defined (NO_ACQUIRED_THREAD)
																				if (return_code&&(UNEMAP_GET_SAMPLES_ACQUIRED_BACKGROUND_CODE==operation_code))
																				{
																					acquired_thread_function(
																						(LPVOID)acquired_file);
																				}
#endif /* defined (NO_ACQUIRED_THREAD) */
																			}
																			else
																			{
																				ResetEvent(hEvents[1]);
																			}
																		}
																		else
																		{
																			if (network_events.lNetworkEvents&
																				FD_CLOSE)
																			{
																				close_connection();
																			}
																			ResetEvent(hEvents[1]);
																		}
																	}
																	else
																	{
																		ResetEvent(hEvents[1]);
																	}
#endif /* defined (USE_UNEMAP_HARDWARE) */
																}
																else
																{
																	running=0;
																}
															}
														}
#if defined (USE_UNEMAP_HARDWARE)
													}
													else
													{
														sprintf(error_string,
															"listen() failed with error %d",
															WSAGetLastError());
														display_message(ERROR_MESSAGE,error_string);
													}
												}
												else
												{
													sprintf(error_string,
											"bind() for acquired_socket_listen failed with error %d",
														WSAGetLastError());
													display_message(ERROR_MESSAGE,error_string);
												}
											}
											else
											{
												sprintf(error_string,
										"bind() for calibration_socket_listen failed with error %d",
													WSAGetLastError());
												display_message(ERROR_MESSAGE,error_string);
											}
										}
										else
										{
											sprintf(error_string,
											"bind() for scrolling_socket_listen failed with error %d",
												WSAGetLastError());
											display_message(ERROR_MESSAGE,error_string);
										}
									}
									else
									{
										sprintf(error_string,
											"bind() failed for command_socket_listen with error %d",
											WSAGetLastError());
										display_message(ERROR_MESSAGE,error_string);
									}
								}
								else
								{
									sprintf(error_string,"WSAEventSelect() failed with error %d",
										WSAGetLastError());
									display_message(ERROR_MESSAGE,error_string);
								}
							}
							else
							{
								sprintf(error_string,"Error creating listening sockets.  %d",
									WSAGetLastError());
								display_message(ERROR_MESSAGE,error_string);
							}
							if (acquired_socket_mutex)
							{
								CloseHandle(acquired_socket_mutex);
							}
							if (INVALID_SOCKET!=acquired_socket_listen)
							{
								closesocket(acquired_socket_listen);
							}
							if (INVALID_SOCKET!=calibration_socket_listen)
							{
								closesocket(calibration_socket_listen);
							}
							if (INVALID_SOCKET!=command_socket_listen)
							{
								closesocket(command_socket_listen);
							}
							if (INVALID_SOCKET!=scrolling_socket_listen)
							{
								closesocket(scrolling_socket_listen);
							}
						}
						else
						{
							sprintf(error_string,"WSAStartup failed with error %d",
								WSAGetLastError());
							display_message(ERROR_MESSAGE,error_string);
						}
						WSACleanup();
#endif /* defined (USE_UNEMAP_HARDWARE) */
#endif /* defined (USE_SOCKETS) */
#if defined (USE_WORMHOLES)
						if ((command_in=CREATE(Wh_input)(COMMAND_IN_CONNECTION_ID,
							(char *)NULL,(double)-1))&&(command_out=CREATE(Wh_output)(
							COMMAND_OUT_CONNECTION_ID,(char *)NULL,(double)-1))&&
							(scrolling_in=CREATE(Wh_input)(SCROLLING_IN_CONNECTION_ID,
							(char *)NULL,(double)-1))&&(scrolling_out=CREATE(Wh_output)(
							SCROLLING_OUT_CONNECTION_ID,(char *)NULL,(double)-1)))
						{
						}
						else
						{
							if (command_in)
							{
								DESTROY(Wh_input)(&command_in);
								if (command_out)
								{
									DESTROY(Wh_output)(&command_out);
									if (scrolling_in)
									{
										DESTROY(Wh_input)(&scrolling_in);
										if (scrolling_out)
										{
											DESTROY(Wh_output)(&scrolling_out);
										}
									}
								}
							}
						}
#endif /* defined (USE_WORMHOLES) */
					}
				}
			}
		}
	}
	/* cleanup */
	if (hServerStopEvent)
	{
		CloseHandle(hServerStopEvent);
	}
	/* overlapped i/o events */
	if (hEvents[1])
	{
		CloseHandle(hEvents[1]);
	}
	/* service initialization */
} /* ServiceStart */

VOID ServiceStop()
/*******************************************************************************
LAST MODIFIED : 22 February 1999

DESCRIPTION :
Stops the service.
???Microsoft.  If a ServiceStop procedure is going to take longer than 3 seconds
	to execute, it should spawn a thread to execute the stop code, and return.
	Otherwise, the ServiceControlManager will believe that the service has stopped
	responding.
==============================================================================*/
{
	if (hServerStopEvent)
	{
		SetEvent(hServerStopEvent);
	}
} /* ServiceStop */
