/*******************************************************************************
FILE : unemap_hardware_service.c

LAST MODIFIED : 30 September 1999

DESCRIPTION :
The unemap service which runs under NT and talks to unemap via sockets.

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

???DB.  unemap_hardware_service -remove gives message about not being able to
	initialize the nidaq DLL because the DLL is already in use by the service.
	Could have a separate remove program that didn't link to nidaq.
==============================================================================*/

/*???DB.  Assume that running on Intel machine */
#define __BYTE_ORDER 1234

#define USE_SOCKETS
/*#define USE_WORMHOLES*/

#include <stdio.h>
#include <stdlib.h>
#if defined (USE_SOCKETS)
#include <winsock2.h>
#include <process.h>
#include <tchar.h>
#endif /* defined (USE_SOCKETS) */
#if defined (USE_WORMHOLES)
#include <windows.h>
#include "wormhole.h"
#endif /* defined (USE_WORMHOLES) */

#include "service.h"
#include "unemap/unemap_hardware.h"
#include "unemap_hardware_service/unemap_hardware_service.h"

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

/*
Constants
---------
*/
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

/*
Module variables
----------------
*/
/* for error messages */
char error_string[501];
/* this event is signalled when the service should end */
HANDLE hServerStopEvent=NULL;
#if defined (USE_SOCKETS)
SOCKET calibration_socket=INVALID_SOCKET,command_socket=INVALID_SOCKET,
	scrolling_socket=INVALID_SOCKET;
#endif /* defined (USE_SOCKETS) */
unsigned char calibration_big_endian,scrolling_big_endian;

/*
Module functions
----------------
*/
static int close_connection(void)
/*******************************************************************************
LAST MODIFIED : 21 May 1999

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

	return (return_code);
} /* close_connection */

#if defined (USE_SOCKETS)
static int socket_recv(SOCKET socket,unsigned char *buffer,int buffer_length,
	int flags)
/*******************************************************************************
LAST MODIFIED : 27 May 1999

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
		AddToMessageLog(TEXT(error_string));
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
static int socket_send(
#if defined (WIN32)
	SOCKET socket,
#endif /* defined (WIN32) */
#if defined (UNIX)
	int socket,
#endif /* defined (UNIX) */
	unsigned char *buffer,int buffer_length,int flags)
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
			sprintf(error_string,"socket_recv.  Connection closed");
		}
		else
		{
			sprintf(error_string,"socket_recv.  recv() failed: error %d",
				last_error);
		}
		AddToMessageLog(TEXT(error_string));
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
#if defined (__BYTE_ORDER) && (1234==__BYTE_ORDER)
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
#else /* defined (__BYTE_ORDER) && (1234==__BYTE_ORDER) */
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
#endif /* defined (__BYTE_ORDER) && (1234==__BYTE_ORDER) */
	}

	return (return_code);
} /* copy_byte_swapped */

#if defined (DEBUG)
/*???debug */
int number_of_scrolling_callbacks=0;
#endif /* defined (DEBUG) */

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

#if defined (DEBUG)
/*???debug */
{
	number_of_scrolling_callbacks++;
	if (number_of_scrolling_callbacks<=5)
	{
		sprintf(error_string,"scrolling_callback %d.  %d",
			number_of_scrolling_callbacks,number_of_channels);
		for (i=0;i<number_of_channels;i++)
		{
			sprintf(error_string+strlen(error_string)," %d",channel_numbers[i]);
		}
		sprintf(error_string+strlen(error_string)," %d %p %d",
			number_of_values_per_channel,values,scrolling_socket);
		AddToMessageLog(TEXT(error_string));
	}
}
#endif /* defined (DEBUG) */
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
			if (out_buffer=(unsigned char *)malloc(out_buffer_size))
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
#if defined (DEBUG)
/*???debug */
{
	int j;

	if (number_of_scrolling_callbacks<=5)
	{
		sprintf(error_string,"  %d %d",out_buffer_size,retval);
		AddToMessageLog(TEXT(error_string));
		for (i=0;i<number_of_channels;i++)
		{
			sprintf(error_string,"  %d ",channel_numbers[i]);
			for (j=0;j<number_of_values_per_channel;j++)
			{
				sprintf(error_string+strlen(error_string)," %d",
					values[i*number_of_values_per_channel+j]);
			}
			AddToMessageLog(TEXT(error_string));
		}
	}
}
#endif /* defined (DEBUG) */
				free(out_buffer);
			}
		}
	}
	/* free the arrays */
	if (channel_numbers)
	{
		free(channel_numbers);
	}
	if (values)
	{
		free(values);
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
			if (out_buffer=(unsigned char *)malloc(out_buffer_size))
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
				free(out_buffer);
			}
		}
		else
		{
			/* failed */
			message_size=0;
			out_buffer_size=2+sizeof(long)+message_size;
			if (out_buffer=(unsigned char *)malloc(out_buffer_size))
			{
				out_buffer[0]=(unsigned char)0x0;
				out_buffer[1]=calibration_big_endian;
				copy_byte_swapped(out_buffer+2,sizeof(long),(char *)&message_size,
					calibration_big_endian);
				retval=socket_send(calibration_socket,out_buffer,out_buffer_size,0);
				/*???debug */
				sprintf(error_string,
					"calibration_callback.  Failed message sent %d %ld",retval,
					out_buffer_size);
				AddToMessageLog(TEXT(error_string));
#if defined (DEBUG)
#endif /* defined (DEBUG) */
				free(out_buffer);
			}
		}
	}
} /* calibration_callback */

#if defined (USE_SOCKETS)
static int process_message(const unsigned char operation_code,
	const long message_size,const unsigned char big_endian,
	unsigned char **out_buffer_address,long *out_buffer_size_address)
/*******************************************************************************
LAST MODIFIED : 13 September 1999

DESCRIPTION :
==============================================================================*/
{
	int return_code,retval;
	long size,unread_size;
	unsigned char buffer[BUFFER_SIZE],*out_buffer;

	return_code=0;
#if defined (DEBUG)
	/*???debug */
	sprintf(error_string,"enter process_message %d %d %ld %p %p",
		(int)operation_code,(int)UNEMAP_GET_NUMBER_OF_CHANNELS_CODE,
		message_size,out_buffer_address,out_buffer_size_address);
	AddToMessageLog(TEXT(error_string));
#endif /* defined (DEBUG) */
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
					AddToMessageLog(TEXT(error_string));
				}
			} break;
			case UNEMAP_CHANNEL_VALID_FOR_STIMULATOR_CODE:
			{
				int buffer_position,channel_number,stimulator_number;

				return_code=0;
				size=sizeof(stimulator_number)+sizeof(channel_number);
#if defined (DEBUG)
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,"UNEMAP_CHANNEL_VALID_FOR_STIMULATOR_CODE %d %d\n",size,
			message_size);
		fclose(debug_nidaq);
	}
}
#endif /* defined (DEBUG) */
				if (size==message_size)
				{
					retval=socket_recv(command_socket,buffer,message_size,0);
#if defined (DEBUG)
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,"retval %d %d\n",retval,SOCKET_ERROR);
		if (SOCKET_ERROR==retval)
		{
			fprintf(debug_nidaq,"WSAGetLastError %d\n",WSAGetLastError());
		}
		fclose(debug_nidaq);
	}
}
#endif /* defined (DEBUG) */
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
					AddToMessageLog(TEXT(error_string));
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
					AddToMessageLog(TEXT(error_string));
				}
			} break;
			case UNEMAP_CONFIGURE_CODE:
			{
				float sampling_frequency,scrolling_refresh_frequency;
				int buffer_position,number_of_samples_in_buffer;

				return_code=0;
				size=sizeof(sampling_frequency)+sizeof(number_of_samples_in_buffer)+
					sizeof(scrolling_refresh_frequency);
#if defined (DEBUG)
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,"UNEMAP_CONFIGURE_CODE %d %d\n",size,message_size);
		fclose(debug_nidaq);
	}
}
#endif /* defined (DEBUG) */
				if (size==message_size)
				{
					retval=socket_recv(command_socket,buffer,message_size,0);
#if defined (DEBUG)
/*???debug */
{
	FILE *debug_nidaq;

	if (debug_nidaq=fopen("nidaq.deb","a"))
	{
		fprintf(debug_nidaq,"retval %d %d\n",retval,SOCKET_ERROR);
		if (SOCKET_ERROR==retval)
		{
			fprintf(debug_nidaq,"WSAGetLastError %d\n",WSAGetLastError());
		}
		fclose(debug_nidaq);
	}
}
#endif /* defined (DEBUG) */
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
						scrolling_big_endian=big_endian;
						return_code=unemap_configure(sampling_frequency,
							number_of_samples_in_buffer,(HWND)NULL,(UINT)0,scrolling_callback,
							(void *)NULL,scrolling_refresh_frequency);
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_configure.  Incorrect message size %d %d",message_size,
						size);
					AddToMessageLog(TEXT(error_string));
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
					AddToMessageLog(TEXT(error_string));
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
					AddToMessageLog(TEXT(error_string));
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
					retval=socket_recv(command_socket,buffer,message_size,0);
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
							if (out_buffer=(unsigned char *)malloc(size))
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
					AddToMessageLog(TEXT(error_string));
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
					retval=socket_recv(command_socket,buffer,message_size,0);
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
							if (out_buffer=(unsigned char *)malloc(size))
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
					AddToMessageLog(TEXT(error_string));
				}
			} break;
			case UNEMAP_GET_CHANNEL_STIMULATING_CODE:
			{
				int channel_number,stimulating;

				return_code=0;
				size=sizeof(channel_number);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,buffer,message_size,0);
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
							if (out_buffer=(unsigned char *)malloc(size))
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
					AddToMessageLog(TEXT(error_string));
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
					retval=socket_recv(command_socket,buffer,message_size,0);
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
							if (out_buffer=(unsigned char *)malloc(size))
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
					AddToMessageLog(TEXT(error_string));
				}
			} break;
			case UNEMAP_GET_HARDWARE_VERSION_CODE:
			{
				enum UNEMAP_hardware_version hardware_version;

				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_get_hardware_version(&hardware_version);
					if (return_code)
					{
						size=sizeof(hardware_version);
						if (out_buffer=(unsigned char *)malloc(size))
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
						"unemap_get_power.  Incorrect message size %d 0",message_size);
					AddToMessageLog(TEXT(error_string));
				}
			} break;
			case UNEMAP_GET_ISOLATE_RECORD_MODE_CODE:
			{
				int channel_number,isolate;

				return_code=0;
				size=sizeof(channel_number);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,buffer,message_size,0);
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
							if (out_buffer=(unsigned char *)malloc(size))
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
					AddToMessageLog(TEXT(error_string));
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
						if (out_buffer=(unsigned char *)malloc(size))
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
					AddToMessageLog(TEXT(error_string));
				}
			} break;
			case UNEMAP_GET_NUMBER_OF_CHANNELS_CODE:
			{
				unsigned long number_of_channels;

				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_get_number_of_channels(&number_of_channels);
					if (return_code)
					{
						size=sizeof(number_of_channels);
						if (out_buffer=(unsigned char *)malloc(size))
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
					AddToMessageLog(TEXT(error_string));
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
						if (out_buffer=(unsigned char *)malloc(size))
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
					AddToMessageLog(TEXT(error_string));
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
						if (out_buffer=(unsigned char *)malloc(size))
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
					AddToMessageLog(TEXT(error_string));
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
						if (out_buffer=(unsigned char *)malloc(size))
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
					AddToMessageLog(TEXT(error_string));
				}
			} break;
			case UNEMAP_GET_SAMPLE_RANGE_CODE:
			{
				long int maximum_sample_value,minimum_sample_value;

				return_code=0;
				if (0==message_size)
				{
					return_code=unemap_get_sample_range(&minimum_sample_value,
						&maximum_sample_value);
					if (return_code)
					{
						size=sizeof(minimum_sample_value)+sizeof(maximum_sample_value);
						if (out_buffer=(unsigned char *)malloc(size))
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
				else
				{
					sprintf(error_string,
						"unemap_get_sample_range.  Incorrect message size %d 0",
						message_size);
					AddToMessageLog(TEXT(error_string));
				}
			} break;
			case UNEMAP_GET_SAMPLES_ACQUIRED_CODE:
			{
				int channel_number;
				unsigned char *out_buffer_entry,temp;
				unsigned long i,number_of_channels,number_of_samples;

				return_code=0;
				size=sizeof(channel_number);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,buffer,message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						if (return_code=unemap_get_number_of_samples_acquired(
							&number_of_samples))
						{
							if (0==channel_number)
							{
								if (return_code=unemap_get_number_of_channels(
									&number_of_channels))
								{
									number_of_samples *= number_of_channels;
								}
								else
								{
									sprintf(error_string,
					"unemap_get_samples_acquired.  unemap_get_number_of_channels failed");
									AddToMessageLog(TEXT(error_string));
								}
							}
							size=number_of_samples*sizeof(short int);
							if (return_code&&(out_buffer=(unsigned char *)malloc(size)))
							{
								return_code=unemap_get_samples_acquired(channel_number,
									(short int *)out_buffer);
								if (return_code)
								{
									/* in-line for speed */
#if defined (__BYTE_ORDER) && (1234==__BYTE_ORDER)
									if (!big_endian)
									{
										out_buffer_entry=out_buffer;
										for (i=number_of_samples;i>0;i--)
										{
											temp=out_buffer_entry[0];
											out_buffer_entry[0]=out_buffer_entry[1];
											out_buffer_entry[1]=temp;
											out_buffer_entry += 2;
										}
									}
#else /* defined (__BYTE_ORDER) && (1234==__BYTE_ORDER) */
									if (!big_endian)
									{
										out_buffer_entry=out_buffer;
										for (i=number_of_samples;i>0;i--)
										{
											temp=out_buffer_entry[0];
											out_buffer_entry[0]=out_buffer_entry[1];
											out_buffer_entry[1]=temp;
											out_buffer_entry +=2;
										}
									}
#endif /* defined (__BYTE_ORDER) && (1234==__BYTE_ORDER) */
									*out_buffer_address=out_buffer;
									*out_buffer_size_address=size;
								}
							}
							else
							{
								if (return_code)
								{
									return_code=0;
									sprintf(error_string,
							"unemap_get_samples_acquired.  Could not allocate out_buffer %ld",
										size);
									AddToMessageLog(TEXT(error_string));
								}
							}
						}
						else
						{
							sprintf(error_string,
	"unemap_get_samples_acquired.  unemap_get_number_of_samples_acquired failed");
							AddToMessageLog(TEXT(error_string));
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_get_samples_acquired.  Incorrect message size %d %d",
						message_size,size);
					AddToMessageLog(TEXT(error_string));
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
						if (out_buffer=(unsigned char *)malloc(size))
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
					AddToMessageLog(TEXT(error_string));
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
					retval=socket_recv(command_socket,buffer,message_size,0);
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
							if (out_buffer=(unsigned char *)malloc(size))
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
					AddToMessageLog(TEXT(error_string));
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
					retval=socket_recv(command_socket,buffer,message_size,0);
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
					AddToMessageLog(TEXT(error_string));
				}
			} break;
			case UNEMAP_SET_CHANNEL_STIMULATING_CODE:
			{
				int channel_number,stimulating;

				return_code=0;
				size=sizeof(channel_number)+sizeof(stimulating);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,buffer,message_size,0);
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
					AddToMessageLog(TEXT(error_string));
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
					retval=socket_recv(command_socket,buffer,message_size,0);
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
					AddToMessageLog(TEXT(error_string));
				}
			} break;
			case UNEMAP_SET_ISOLATE_RECORD_MODE_CODE:
			{
				int channel_number,isolate;

				return_code=0;
				size=sizeof(channel_number)+sizeof(isolate);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,buffer,message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer,big_endian);
						copy_byte_swapped((unsigned char *)&isolate,
							sizeof(isolate),buffer+sizeof(channel_number),big_endian);
#if defined (DEBUG)
						/*???debug */
						sprintf(error_string,"unemap_set_isolate_record_mode.  %d %d",
							channel_number,isolate);
						AddToMessageLog(TEXT(error_string));
#endif /* defined (DEBUG) */
						return_code=unemap_set_isolate_record_mode(channel_number,isolate);
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_set_isolate_record_mode.  Incorrect message size %d %d",
						message_size,size);
					AddToMessageLog(TEXT(error_string));
				}
			} break;
			case UNEMAP_SET_POWER_CODE:
			{
				int on;

				return_code=0;
				size=sizeof(on);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,buffer,message_size,0);
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
					AddToMessageLog(TEXT(error_string));
				}
			} break;
			case UNEMAP_SET_POWERUP_ANTIALIASING_FILTER_FREQUENCY_CODE:
			{
				int channel_number;

				return_code=0;
				size=sizeof(channel_number);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,buffer,message_size,0);
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
"unemap_set_powerup_antialiasing_filter_frequency.  Incorrect message size %d %d",
						message_size,size);
					AddToMessageLog(TEXT(error_string));
				}
			} break;
			case UNEMAP_SET_SCROLLING_CHANNEL_CODE:
			{
				int channel_number;

				return_code=0;
				size=sizeof(channel_number);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,buffer,message_size,0);
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
					AddToMessageLog(TEXT(error_string));
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
					AddToMessageLog(TEXT(error_string));
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
					retval=socket_recv(command_socket,buffer,size,0);
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
								if (out_buffer=(unsigned char *)malloc(size+sizeof(float)))
								{
									retval=socket_recv(command_socket,out_buffer,size,0);
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
										free(out_buffer);
										out_buffer=(unsigned char *)NULL;
									}
								}
								else
								{
									sprintf(error_string,
										"unemap_start_calibrating.  Could not allocate voltages %d",
										number_of_voltages);
									AddToMessageLog(TEXT(error_string));
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
										free(out_buffer);
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
							AddToMessageLog(TEXT(error_string));
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_start_calibrating.  Incorrect message size %d>=%d",
						message_size,size);
					AddToMessageLog(TEXT(error_string));
				}
			} break;
#if defined (OLD_CODE)
			case UNEMAP_START_CALIBRATING_CODE:
			{
				int channel_number,i,number_of_voltages;
				float *voltage,*voltages,voltages_per_second;

				return_code=0;
				size=2*sizeof(int)+sizeof(float);
				if (size<=message_size)
				{
					retval=socket_recv(command_socket,buffer,size,0);
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
							if (0<number_of_voltages)
							{
								size=number_of_voltages*sizeof(float);
								if (voltages=(float *)malloc(size+sizeof(float)))
								{
									retval=socket_recv(command_socket,
										(unsigned char *)(voltages+1),size,0);
									if (SOCKET_ERROR!=retval)
									{
										unread_size -= retval;
										voltage=voltages;
										for (i=number_of_voltages;i>0;i--)
										{
											copy_byte_swapped((unsigned char *)voltage,sizeof(float),
												(unsigned char *)(voltage+1),big_endian);
											voltage++;
										}
										return_code=1;
									}
									else
									{
										free(voltages);
									}
								}
								else
								{
									sprintf(error_string,
										"unemap_start_calibrating.  Could not allocate voltages %d",
										number_of_voltages);
									AddToMessageLog(TEXT(error_string));
								}
							}
							else
							{
								voltages=(float *)NULL;
								return_code=1;
							}
							if (return_code)
							{
								return_code=unemap_start_calibrating(channel_number,
									number_of_voltages,voltages_per_second,voltages);
								if (voltages)
								{
									free(voltages);
								}
							}
						}
						else
						{
							sprintf(error_string,
								"unemap_start_calibrating.  Incorrect message size %d %d",
								message_size,size+number_of_voltages*sizeof(float));
							AddToMessageLog(TEXT(error_string));
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_start_calibrating.  Incorrect message size %d>=%d",
						message_size,size);
					AddToMessageLog(TEXT(error_string));
				}
			} break;
#endif /* defined (OLD_CODE) */
			case UNEMAP_START_CURRENT_STIMULATING_CODE:
			{
				int channel_number,i,number_of_currents;
				float *current,*currents,currents_per_second;

				return_code=0;
				size=2*sizeof(int)+sizeof(float);
				if (size<=message_size)
				{
					retval=socket_recv(command_socket,buffer,size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						size=0;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer+size,big_endian);
						size += sizeof(channel_number);
						copy_byte_swapped((unsigned char *)&number_of_currents,
							sizeof(number_of_currents),buffer+size,big_endian);
						size += sizeof(number_of_currents);
						copy_byte_swapped((unsigned char *)&currents_per_second,
							sizeof(currents_per_second),buffer+size,big_endian);
						size += sizeof(currents_per_second);
						size += number_of_currents*sizeof(float);
						if (size==message_size)
						{
							out_buffer=(unsigned char *)NULL;
							if (0<number_of_currents)
							{
								size=number_of_currents*sizeof(float);
								if (out_buffer=(unsigned char *)malloc(size+sizeof(float)))
								{
									retval=socket_recv(command_socket,out_buffer,size,0);
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
										return_code=1;
									}
									else
									{
										free(out_buffer);
										out_buffer=(unsigned char *)NULL;
									}
								}
								else
								{
									sprintf(error_string,
						"unemap_start_current_stimulating.  Could not allocate currents %d",
										number_of_currents);
									AddToMessageLog(TEXT(error_string));
								}
							}
							else
							{
								currents=(float *)NULL;
								return_code=1;
							}
							if (return_code)
							{
								if (return_code=unemap_start_current_stimulating(channel_number,
									number_of_currents,currents_per_second,currents))
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
										return_code=1;
										*out_buffer_address=out_buffer;
										*out_buffer_size_address=size;
									}
								}
								else
								{
									if (out_buffer)
									{
										free(out_buffer);
										out_buffer=(unsigned char *)NULL;
									}
								}
							}
						}
						else
						{
							sprintf(error_string,
							"unemap_start_current_stimulating.  Incorrect message size %d %d",
								message_size,size+number_of_currents*sizeof(float));
							AddToMessageLog(TEXT(error_string));
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_start_current_stimulating.  Incorrect message size %d>=%d",
						message_size,size);
					AddToMessageLog(TEXT(error_string));
				}
			} break;
#if defined (OLD_CODE)
			case UNEMAP_START_CURRENT_STIMULATING_CODE:
			{
				int channel_number,i,number_of_currents;
				float *current,*currents,currents_per_second;

				return_code=0;
				size=2*sizeof(int)+sizeof(float);
				if (size<=message_size)
				{
					retval=socket_recv(command_socket,buffer,size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
						size=0;
						copy_byte_swapped((unsigned char *)&channel_number,
							sizeof(channel_number),buffer+size,big_endian);
						size += sizeof(channel_number);
						copy_byte_swapped((unsigned char *)&number_of_currents,
							sizeof(number_of_currents),buffer+size,big_endian);
						size += sizeof(number_of_currents);
						copy_byte_swapped((unsigned char *)&currents_per_second,
							sizeof(currents_per_second),buffer+size,big_endian);
						size += sizeof(currents_per_second);
						size += number_of_currents*sizeof(float);
						if (size==message_size)
						{
							if (0<number_of_currents)
							{
								size=number_of_currents*sizeof(float);
								if (currents=(float *)malloc(size+sizeof(float)))
								{
									retval=socket_recv(command_socket,
										(unsigned char *)(currents+1),size,0);
									if (SOCKET_ERROR!=retval)
									{
										unread_size -= retval;
										current=currents;
										for (i=number_of_currents;i>0;i--)
										{
											copy_byte_swapped((unsigned char *)current,sizeof(float),
												(unsigned char *)(current+1),big_endian);
											current++;
										}
										return_code=1;
									}
									else
									{
										free(currents);
									}
								}
								else
								{
									sprintf(error_string,
						"unemap_start_current_stimulating.  Could not allocate currents %d",
										number_of_currents);
									AddToMessageLog(TEXT(error_string));
								}
							}
							else
							{
								currents=(float *)NULL;
								return_code=1;
							}
							if (return_code)
							{
								return_code=unemap_start_current_stimulating(channel_number,
									number_of_currents,currents_per_second,currents);
								if (currents)
								{
									free(currents);
								}
							}
						}
						else
						{
							sprintf(error_string,
							"unemap_start_current_stimulating.  Incorrect message size %d %d",
								message_size,size+number_of_currents*sizeof(float));
							AddToMessageLog(TEXT(error_string));
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_start_current_stimulating.  Incorrect message size %d>=%d",
						message_size,size);
					AddToMessageLog(TEXT(error_string));
				}
			} break;
#endif /* defined (OLD_CODE) */
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
					AddToMessageLog(TEXT(error_string));
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
					AddToMessageLog(TEXT(error_string));
				}
			} break;
			case UNEMAP_START_VOLTAGE_STIMULATING_CODE:
			{
				int channel_number,i,number_of_voltages;
				float *voltage,*voltages,voltages_per_second;

				return_code=0;
				size=2*sizeof(int)+sizeof(float);
				if (size<=message_size)
				{
					retval=socket_recv(command_socket,buffer,size,0);
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
								if (out_buffer=(unsigned char *)malloc(size+sizeof(float)))
								{
									retval=socket_recv(command_socket,out_buffer,size,0);
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
										free(out_buffer);
										out_buffer=(unsigned char *)NULL;
									}
								}
								else
								{
									sprintf(error_string,
						"unemap_start_voltage_stimulating.  Could not allocate voltages %d",
										number_of_voltages);
									AddToMessageLog(TEXT(error_string));
								}
							}
							else
							{
								voltages=(float *)NULL;
								return_code=1;
							}
							if (return_code)
							{
								if (return_code=unemap_start_voltage_stimulating(channel_number,
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
										free(out_buffer);
										out_buffer=(unsigned char *)NULL;
									}
								}
							}
						}
						else
						{
							sprintf(error_string,
							"unemap_start_voltage_stimulating.  Incorrect message size %d %d",
								message_size,size+number_of_voltages*sizeof(float));
							AddToMessageLog(TEXT(error_string));
						}
					}
				}
				else
				{
					sprintf(error_string,
						"unemap_start_voltage_stimulating.  Incorrect message size %d>=%d",
						message_size,size);
					AddToMessageLog(TEXT(error_string));
				}
			} break;
			case UNEMAP_STOP_CALIBRATING_CODE:
			{
				int channel_number;

				return_code=0;
				size=sizeof(channel_number);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,buffer,message_size,0);
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
					AddToMessageLog(TEXT(error_string));
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
					AddToMessageLog(TEXT(error_string));
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
					AddToMessageLog(TEXT(error_string));
				}
			} break;
			case UNEMAP_STOP_STIMULATING_CODE:
			{
				int channel_number;

				return_code=0;
				size=sizeof(channel_number);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,buffer,message_size,0);
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
					AddToMessageLog(TEXT(error_string));
				}
			} break;
			case UNEMAP_TOGGLE_SHIFT_REGISTER_CODE:
			{
				int channel_number,register_number;

				return_code=0;
				size=2*sizeof(int);
				if (size==message_size)
				{
					retval=socket_recv(command_socket,buffer,message_size,0);
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
					AddToMessageLog(TEXT(error_string));
				}
			} break;
			default:
			{
				/* ignore the message */
				return_code=1;
				size=message_size;
				while (return_code&&(0<size))
				{
					retval=socket_recv(command_socket,buffer,message_size,0);
					if (SOCKET_ERROR!=retval)
					{
						unread_size -= retval;
					}
					else
					{
						return_code=0;
					}
				}
#if defined (DEBUG)
				/* bounce the message back */
				return_code=1;
				if (out_buffer=(unsigned char *)malloc(message_size))
				{
					out_buffer_size=0;
					while (return_code&&(0<message_size))
					{
						do
						{
							retval=recv(command_socket,out_buffer+out_buffer_size,
								message_size,0);
						} while ((SOCKET_ERROR==retval)&&
							(WSAEWOULDBLOCK==WSAGetLastError()));
						if ((SOCKET_ERROR!=retval)&&(0<retval))
						{
							message_size -= retval;
							out_buffer_size += retval;
						}
						else
						{
							return_code=0;
							free(out_buffer);
							out_buffer_size=0;
						}
					}
				}
				else
				{
					while (return_code&&(0<message_size))
					{
						do
						{
							retval=recv(command_socket,buffer,BUFFER_SIZE,0);
						} while ((SOCKET_ERROR==retval)&&
							(WSAEWOULDBLOCK==WSAGetLastError()));
						if ((SOCKET_ERROR!=retval)&&(0<retval))
						{
							message_size -= retval;
						}
						else
						{
							return_code=0;
						}
					}
					out_buffer_size=0;
				}
#endif /* defined (DEBUG) */
			} break;
		}
	}
	else
	{
		sprintf(error_string,"process_message.  Invalid arguments %p %p",
			out_buffer_address,out_buffer_size_address);
		AddToMessageLog(TEXT(error_string));
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
		retval=socket_recv(command_socket,buffer,size,0);
		if (SOCKET_ERROR!=retval)
		{
			unread_size -= size;
		}
	}
#if defined (DEBUG)
	/*???debug */
	sprintf(error_string,"leave process_message %d",return_code);
	AddToMessageLog(TEXT(error_string));
#endif /* defined (DEBUG) */

	return (return_code);
} /* process_message */
#endif /* defined (USE_SOCKETS) */

VOID ServiceStart(DWORD dwArgc,LPTSTR *lpszArgv)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Actual code of the service that does the work.
	dwArgc - number of command line arguments
	lpszArgv - array of command line arguments
==============================================================================*/
{
	DWORD dwWait;
	HANDLE hEvents[2]={NULL,NULL};
	unsigned long number_of_channels;
#if defined (USE_SOCKETS)
#if defined (OLD_CODE)
	fd_set readfds;
#endif /* defined (OLD_CODE) */
	int fromlen,last_error,return_code,retval,running,socket_type=SOCK_STREAM;
	long message_size,out_buffer_size;
	SOCKET calibration_socket_listen,command_socket_listen,
		scrolling_socket_listen;
	struct sockaddr_in from,local;
	struct timeval timeout;
	unsigned char big_endian,buffer[BUFFER_SIZE],operation_code,*out_buffer;
	unsigned short port=DEFAULT_PORT;
	WORD wVersionRequested;
	WSADATA wsaData;
	WSANETWORKEVENTS network_events;
#endif /* defined (USE_SOCKETS) */
#if defined (USE_WORMHOLES)
	Wh_input *command_in,*scrolling_in;
	Wh_output *command_out,*scrolling_out;
#endif /* defined (USE_WORMHOLES) */

#if defined (DEBUG)
	/*???debug */
	sprintf(error_string,"Entering ServiceStart");
	AddToMessageLog(TEXT(error_string));
#endif /* defined (DEBUG) */
	/* when the NIDAQ DLL is loaded (when the service starts), the NI cards and
		hence the unemap cards are in an unknown state.  This call to
		unemap_get_number_of_channels forces a call to search_for_NI_cards which
		in turn configures the NI cards */
	unemap_get_number_of_channels(&number_of_channels);
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
				hEvents[1]=CreateEvent(/*no security attributes*/NULL,
					/*manual reset event*/TRUE,/*not-signalled*/FALSE,
					/*no name*/NULL);
				if (hEvents[1])
				{
					/* report the status to the service control manager */
					if (ReportStatusToSCMgr(/*service state*/SERVICE_START_PENDING,
						/*exit code*/NO_ERROR,/*wait hint*/3000))
					{
#if defined (USE_SOCKETS)
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
							if ((INVALID_SOCKET!=calibration_socket_listen)&&
								(INVALID_SOCKET!=command_socket_listen)&&
								(INVALID_SOCKET!=scrolling_socket_listen))
							{
								/* can't have different event objects for different network
									events */
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
												if ((SOCKET_ERROR!=listen(command_socket_listen,5))&&
													(SOCKET_ERROR!=listen(calibration_socket_listen,5))&&
													(SOCKET_ERROR!=listen(scrolling_socket_listen,5)))
												{
													/* report the status to the service control manager */
													if (ReportStatusToSCMgr(
														/*service state*/SERVICE_RUNNING,
														/*exit code*/NO_ERROR,/*wait hint*/0))
													{
														running=1;
														/*???debug */
														sprintf(error_string,"Starting main loop");
														AddToMessageLog(TEXT(error_string));
														command_socket=INVALID_SOCKET;
														timeout.tv_sec=0;
														timeout.tv_usec=0;
														while (1==running)
														{
#if defined (DEBUG)
															/*???debug */
															sprintf(error_string,"waiting for event");
															AddToMessageLog(TEXT(error_string));
#endif /* defined (DEBUG) */
															dwWait=WaitForMultipleObjects(2,hEvents,FALSE,
																INFINITE);
#if defined (DEBUG)
															/*???debug */
															sprintf(error_string,"received event %x %x",
																dwWait,WAIT_OBJECT_0+1);
															AddToMessageLog(TEXT(error_string));
#endif /* defined (DEBUG) */
															if (WAIT_OBJECT_0+1==dwWait)
															{
																WSAEnumNetworkEvents(command_socket,
																	NULL,&network_events);
#if defined (DEBUG)
																/*???debug */
																sprintf(error_string,"network_events %lx %lx",
																	network_events.lNetworkEvents,FD_CLOSE);
																AddToMessageLog(TEXT(error_string));
#endif /* defined (DEBUG) */
																if (network_events.lNetworkEvents&FD_CLOSE)
																{
																	close_connection();
																}
																WSAEnumNetworkEvents(command_socket_listen,
																	NULL,&network_events);
#if defined (DEBUG)
																/*???debug */
																sprintf(error_string,"network_events %lx %lx",
																	network_events.lNetworkEvents,FD_CLOSE);
																AddToMessageLog(TEXT(error_string));
#endif /* defined (DEBUG) */
																if (network_events.lNetworkEvents&FD_ACCEPT)
																{
																	if (INVALID_SOCKET==command_socket)
																	{
#if defined (DEBUG)
																		/*???debug */
																		sprintf(error_string,
																			"Making command connection");
																		AddToMessageLog(TEXT(error_string));
#endif /* defined (DEBUG) */
																		/* make connection */
																		fromlen=sizeof(from);
																		do
																		{
																			command_socket=accept(
																				command_socket_listen,
																				(struct sockaddr *)&from,&fromlen);
																		} while ((INVALID_SOCKET==command_socket)&&
																			(WSAEWOULDBLOCK==
																			(last_error=WSAGetLastError())));
																		if (INVALID_SOCKET!=command_socket)
																		{
#if defined (DEBUG)
																			/*???debug */
																			sprintf(error_string,
																				"Making scrolling connection");
																			AddToMessageLog(TEXT(error_string));
#endif /* defined (DEBUG) */
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
#if defined (DEBUG)
																				/*???debug */
																				sprintf(error_string,
																					"Making calibration connection");
																				AddToMessageLog(TEXT(error_string));
#endif /* defined (DEBUG) */
																				fromlen=sizeof(from);
																				do
																				{
																					calibration_socket=accept(
																						calibration_socket_listen,
																						(struct sockaddr *)&from,&fromlen);
																				} while ((INVALID_SOCKET==
																					calibration_socket)&&(WSAEWOULDBLOCK==
																					(last_error=WSAGetLastError())));
																				if (INVALID_SOCKET==calibration_socket)
																				{
																					sprintf(error_string,
																				"Making calibration connection failed");
																					AddToMessageLog(TEXT(error_string));
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
																				AddToMessageLog(TEXT(error_string));
																				closesocket(command_socket);
																				command_socket=INVALID_SOCKET;
																			}
																		}
																		else
																		{
																			sprintf(error_string,
																				"Making command connection failed");
																			AddToMessageLog(TEXT(error_string));
																		}
																	}
																}
																if (INVALID_SOCKET!=command_socket)
																{
#if defined (OLD_CODE)
																	/* check if socket is ready for reading */
																		/*???DB.  After being closed at other end,
																			still ready for reading */
																	FD_ZERO(&readfds);
																	FD_SET(command_socket,&readfds);
																	if (1==select(1,&readfds,(fd_set *)NULL,
																		(fd_set *)NULL,&timeout))
																	{
#endif /* defined (OLD_CODE) */
																		retval=socket_recv(command_socket,buffer,
																			MESSAGE_HEADER_SIZE,0);
																		if (SOCKET_ERROR!=retval)
																		{
																			/* decode message header */
																			operation_code=buffer[0];
																			big_endian=buffer[1];
#if defined (DEBUG)
																			/*???debug */
																			sprintf(error_string,
																				"operation_code=%d",
																				(int)operation_code);
																			AddToMessageLog(TEXT(error_string));
#endif /* defined (DEBUG) */
																			copy_byte_swapped((char *)&message_size,
																				sizeof(long),buffer+2,big_endian);
																			out_buffer=(unsigned char *)NULL;
																			out_buffer_size=0;
																			return_code=process_message(
																				operation_code,message_size,
																				big_endian,&out_buffer,
																				&out_buffer_size);
																			ResetEvent(hEvents[1]);
#if defined (DEBUG)
																			/*???debug */
																			sprintf(error_string,"return_code=%d",
																				(int)return_code);
																			AddToMessageLog(TEXT(error_string));
#endif /* defined (DEBUG) */
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
#if defined (DEBUG)
																			/*???debug */
																			sprintf(error_string,
																				"Sent message header %d %d",
																				SOCKET_ERROR,retval);
																			AddToMessageLog(TEXT(error_string));
#endif /* defined (DEBUG) */
																			if (0<out_buffer_size)
																			{
																				retval=socket_send(command_socket,
																					out_buffer,out_buffer_size,0);
#if defined (DEBUG)
																				/*???debug */
																				sprintf(error_string,
																					"Sent message body %d %d",
																					SOCKET_ERROR,retval);
																				AddToMessageLog(TEXT(error_string));
#endif /* defined (DEBUG) */
																				free(out_buffer);
																				out_buffer=(unsigned char *)NULL;
																			}
																		}
																		else
																		{
																			ResetEvent(hEvents[1]);
																		}
#if defined (OLD_CODE)
																	}
																	else
																	{
																		sprintf(error_string,
																			"Command socket not ready for reading");
																		AddToMessageLog(TEXT(error_string));
																		close_connection();
																		ResetEvent(hEvents[1]);
																	}
#endif /* defined (OLD_CODE) */
																}
																else
																{
																	ResetEvent(hEvents[1]);
																}
															}
															else
															{
																running=0;
															}
#if defined (DEBUG)
															/*???debug */
															sprintf(error_string,"command_socket %d\n",
																command_socket);
															AddToMessageLog(TEXT(error_string));
#endif /* defined (DEBUG) */
														}
													}
												}
												else
												{
													sprintf(error_string,
														"listen() failed with error %d",WSAGetLastError());
													AddToMessageLog(TEXT(error_string));
												}
											}
											else
											{
												sprintf(error_string,
										"bind() for calibration_socket_listen failed with error %d",
													WSAGetLastError());
												AddToMessageLog(TEXT(error_string));
											}
										}
										else
										{
											sprintf(error_string,
											"bind() for scrolling_socket_listen failed with error %d",
												WSAGetLastError());
											AddToMessageLog(TEXT(error_string));
										}
									}
									else
									{
										sprintf(error_string,
											"bind() failed for command_socket_listen with error %d",
											WSAGetLastError());
										AddToMessageLog(TEXT(error_string));
									}
								}
								else
								{
									sprintf(error_string,"WSAEventSelect() failed with error %d",
										WSAGetLastError());
									AddToMessageLog(TEXT(error_string));
								}
							}
							else
							{
								sprintf(error_string,"Error creating listening sockets.  %d",
									WSAGetLastError());
								AddToMessageLog(TEXT(error_string));
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
							AddToMessageLog(TEXT(error_string));
						}
						WSACleanup();
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
							/*???debug */
							sprintf(error_string,"Could not create wormholes");
							AddToMessageLog(TEXT(error_string));
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
#if defined (DEBUG)
	/*???debug */
	sprintf(error_string,"Leaving ServiceStart");
	AddToMessageLog(TEXT(error_string));
#endif /* defined (DEBUG) */
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
