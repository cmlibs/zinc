/*******************************************************************************
FILE : unemap_hardware_client.c

LAST MODIFIED : 19 November 2003

DESCRIPTION :
Code for talking to the unemap hardware service (running under NT).  This is an
alternative to unemap_hardware.c that talks directly to the hardware.

NOTES :
19 September 2002.  DB
	Looked at timing out if the hardware service is already in use.  I thought
	that the wait would be in connect, but it is in socket_recv.  Tried adding
	a timeout using a select before the recv.  Had a few problems to do with
	- first parameter of select does need to be biggest file descriptor plus 1
	- select does return if already data waiting
	- mutex deadlocking when timed out and closing connection (changed to using
		recursive mutexs for unix, under Windows are all recursive - see
		CREATE_MUTEX)
	- hardware service can be away for a long time during configuration (>20
		seconds) and probably longer when getting samples back, so gave up on this
		plan
24 September 2002.  DB
	Changed the hardware service so that if it already has a command_socket and
		it gets another connection on command_socket_listen it accepts and then
		closes immediately.  Client will get a connection reset error (104 for
		Linux and 10054 for Windows)
	Added destroy_mutex
	Only did create_mutex and destroy_mutex for connection_mutex
	Switched to recursive mutexes and fixed hanging problem (when service already
		connected) for linux
28 May 2003.  DB
	Added stimulation end callback support (new socket)
		- will still work (but no callbacks) if the service does not have the
			stimulation socket (hardware service version less than 3).  Does this by
			closing the un'connect'd stimulation_socket
2 June 2003.  DB
	Changed from SERVICE_VERSION to SOFTWARE_VERSION (now in
		unemap/unemap_hardware.c)
???DB.  Finish propagating create_mutex and destroy_mutex

QUESTIONS :
1 How much information should the client store ?  Start with none.
2 Can wormholes be used ?

???DB.  Sort out __BYTE_ORDER and BIG_ENDIAN.  See /usr/include/sys/endian.h
???DB.  Sort out WSACleanup/WSAStartup
	???DB.  Where I'm up to.  Will have to clean up properly for client ?
				WaitForMultipleObjects for WIN32_SYSTEM

TO DO :
1 Speed up save/transfer of data.  Needs to be made thread safe for this
1.1 Sort out BACKGROUND_SAVING/UNIX/WIN32
2 Synchronize stimulating between cards and then crates
3 Subset of channels.  Look for
3.1 samples.  DONE
3.2 number_of_channels.  DONE
3.3 unemap_get_number_of_channels.  DONE
3.4 module_number_of_channels.  DONE
3.5 module_number_of_configured_channels.  DONE
==============================================================================*/

/*???testing */
#define CREATE_MUTEX

#define USE_SOCKETS
/*#define USE_WORMHOLES*/

#define CACHE_CLIENT_INFORMATION

#define BACKGROUND_SAVING
#define UNEMAP_THREAD_SAFE

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if defined (WIN32_SYSTEM)
#if defined (USE_SOCKETS)
#include <winsock2.h>
#include <process.h>
#include <tchar.h>
#endif /* defined (USE_SOCKETS) */
#if defined (USE_WORMHOLES)
#include <windows.h>
#include "wormhole.h"
#endif /* defined (USE_WORMHOLES) */
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
extern int errno;
#endif /* defined (UNIX) */
#if defined (BACKGROUND_SAVING) && defined (UNIX)
#include <pthread.h>
#endif /* defined (BACKGROUND_SAVING) && defined (UNIX) */
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/myio.h"
#include "general/mystring.h"
#include "general/object.h"
#include "user_interface/event_dispatcher.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "unemap/unemap_hardware.h"
#include "unemap_hardware_service/unemap_hardware_service.h"

/*
Module constants
----------------
*/
#define NUMBER_OF_CHANNELS_ON_NI_CARD 64

#if defined (UNIX)
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#endif /* defined (UNIX) */

#if defined (BYTE_ORDER) && (1234==BYTE_ORDER)
#define BIG_ENDIAN_CODE (unsigned char)0x01
#else /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
#define BIG_ENDIAN_CODE (unsigned char)0x00
#endif /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */

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
Module types
------------
*/
struct Stimulation_end_callback_info
/*******************************************************************************
LAST MODIFIED : 29 May 2003

DESCRIPTION :
Used for keeping track of the stimulation end callbacks.
==============================================================================*/
{
	/* <crate_ids> are the cards/channels that were initially loaded for the
		crate.  <crate_cards> are the current cards/channels the <callback> is for
		(subsequent loads can overwrite) */
	int *crate_cards,*crate_ids;
	Unemap_stimulation_end_callback *callback;
	void *callback_data;
	int access_count;
}; /* struct Stimulation_end_callback_info */

DECLARE_LIST_TYPES(Stimulation_end_callback_info);

FULL_DECLARE_INDEXED_LIST_TYPE(Stimulation_end_callback_info);

#if defined (CACHE_CLIENT_INFORMATION)
struct Unemap_card
/*******************************************************************************
LAST MODIFIED : 17 August 2003

DESCRIPTION :
Used for caching information on this side of the connection to speed up some
functions.
==============================================================================*/
{
	int channel_number;
	float post_filter_gain,pre_filter_gain;
	/* for a gain of 1 */
	float maximum_voltage,minimum_voltage;
	long int maximum_sample_value,minimum_sample_value;
}; /* struct Unemap_card */
#endif /* defined (CACHE_CLIENT_INFORMATION) */

struct Unemap_crate
/*******************************************************************************
LAST MODIFIED : 11 August 2003

DESCRIPTION :
Information needed for each SCU crate/NI computer pair.
==============================================================================*/
{
	char *server_name;
	int software_version;
#if defined (USE_SOCKETS)
#if defined (WIN32_SYSTEM)
	SOCKET acquired_socket;
	HANDLE acquired_socket_thread_stop_event,
		acquired_socket_thread_stopped_event;
	HANDLE acquired_socket_mutex;
	SOCKET calibration_socket;
	HANDLE calibration_socket_thread_stop_event,
		calibration_socket_thread_stopped_event;
	SOCKET command_socket;
	HANDLE command_socket_mutex;
	SOCKET scrolling_socket;
	HANDLE scrolling_socket_thread_stop_event,
		scrolling_socket_thread_stopped_event;
	SOCKET stimulation_socket;
	HANDLE stimulation_socket_thread_stop_event,
		stimulation_socket_thread_stopped_event;
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
	int acquired_socket;
	pthread_mutex_t *acquired_socket_mutex;
#if !defined (CREATE_MUTEX)
	pthread_mutex_t acquired_socket_mutex_storage;
#endif /* !defined (CREATE_MUTEX) */
	int calibration_socket;
	int command_socket;
	pthread_mutex_t *command_socket_mutex;
#if !defined (CREATE_MUTEX)
	pthread_mutex_t command_socket_mutex_storage;
#endif /* !defined (CREATE_MUTEX) */
	int scrolling_socket;
	int stimulation_socket;
#endif /* defined (UNIX) */
#if defined (UNIX)
	struct Event_dispatcher *event_dispatcher;
	pthread_mutex_t *event_dispatcher_mutex;
#if !defined (CREATE_MUTEX)
	pthread_mutex_t event_dispatcher_mutex_storage;
#endif /* !defined (CREATE_MUTEX) */
	struct Event_dispatcher_descriptor_callback *acquired_socket_xid;
	struct Event_dispatcher_descriptor_callback *calibration_socket_xid;
	struct Event_dispatcher_descriptor_callback *scrolling_socket_xid;
	struct Event_dispatcher_descriptor_callback *stimulation_socket_xid;
#endif /* defined (UNIX) */
#endif /* defined (USE_SOCKETS) */
	unsigned short acquired_port,calibration_port,command_port,scrolling_port,
		stimulation_port;
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
	int *configured_channel_offsets,*configured_channels,number_of_channels,
		number_of_configured_channels;
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
#if defined (WIN32_USER_INTERFACE)
UINT module_scrolling_message=(UINT)0;
HWND module_scrolling_window=(HWND)NULL;
#endif /* defined (WIN32_USER_INTERFACE) */

/* calibration information set by unemap_calibrate */
Unemap_calibration_end_callback
	*module_calibration_end_callback=(Unemap_calibration_end_callback *)NULL;
void *module_calibration_end_callback_data=(void *)NULL;

int allow_open_connection=1;
int module_number_of_channels=0,module_number_of_configured_channels=0,
	module_number_of_unemap_crates=0;
struct Unemap_crate *module_unemap_crates=(struct Unemap_crate *)NULL;
struct LIST(Stimulation_end_callback_info)
	*module_stimulation_end_callback_info_list=
	(struct LIST(Stimulation_end_callback_info) *)NULL;
#if defined (WIN32_SYSTEM)
HANDLE connection_mutex=(HANDLE)NULL;
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
pthread_mutex_t *connection_mutex=(pthread_mutex_t *)NULL;
#if !defined (CREATE_MUTEX)
pthread_mutex_t connection_mutex_storage;
#endif /* !defined (CREATE_MUTEX) */
#endif /* defined (UNIX) */
#if defined (WIN32_SYSTEM)
HANDLE stimulation_mutex=(HANDLE)NULL;
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
pthread_mutex_t *stimulation_mutex=(pthread_mutex_t *)NULL;
#if !defined (CREATE_MUTEX)
pthread_mutex_t stimulation_mutex_storage;
#endif /* !defined (CREATE_MUTEX) */
#endif /* defined (UNIX) */

#if defined (CACHE_CLIENT_INFORMATION)
int module_force_connection=0,module_get_cache_information_failed=0,
	module_number_of_stimulators=0;
#endif /* defined (CACHE_CLIENT_INFORMATION) */

/* information set by unemap_get_samples_acquired_background */
Unemap_acquired_data_callback *module_acquired_callback=
	(Unemap_acquired_data_callback *)NULL;
int module_acquired_channel_number= -1;
void *module_acquired_callback_data=(void *)NULL;

/*
Module functions
----------------
*/
static struct Stimulation_end_callback_info
	*CREATE(Stimulation_end_callback_info)(int *crate_ids,
	Unemap_stimulation_end_callback *callback,void *callback_data)
/*******************************************************************************
LAST MODIFIED : 29 May 2003

DESCRIPTION :
Creates a struct Stimulation_end_callback_info.  ALLOCATEs memory for
<crate_ids> and copies.
==============================================================================*/
{
	int i;
	struct Stimulation_end_callback_info *stimulation_end_callback_info;

	ENTER(CREATE(Stimulation_end_callback_info));
	stimulation_end_callback_info=(struct Stimulation_end_callback_info *)NULL;
	if (callback&&crate_ids)
	{
		if (ALLOCATE(stimulation_end_callback_info,
			struct Stimulation_end_callback_info,1)&&
			(0<module_number_of_unemap_crates)&&
			ALLOCATE(stimulation_end_callback_info->crate_ids,int,
			module_number_of_unemap_crates)&&
			ALLOCATE(stimulation_end_callback_info->crate_cards,int,
			module_number_of_unemap_crates))
		{
			for (i=0;i<module_number_of_unemap_crates;i++)
			{
				(stimulation_end_callback_info->crate_ids)[i]=crate_ids[i];
				(stimulation_end_callback_info->crate_cards)[i]=crate_ids[i];
			}
			stimulation_end_callback_info->callback=callback;
			stimulation_end_callback_info->callback_data=callback_data;
			stimulation_end_callback_info->access_count=0;
		}
		else
		{
			if (stimulation_end_callback_info)
			{
				if ((0<module_number_of_unemap_crates)&&
					stimulation_end_callback_info->crate_ids)
				{
					DEALLOCATE(stimulation_end_callback_info->crate_ids);
				}
				DEALLOCATE(stimulation_end_callback_info);
			}
			display_message(ERROR_MESSAGE,"CREATE(Stimulation_end_callback_info).  "
				"Could not allocate");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Stimulation_end_callback_info).  "
			"Invalid argument(s).  %p %p",callback,crate_ids);
	}
	LEAVE;

	return (stimulation_end_callback_info);
} /* CREATE(Stimulation_end_callback_info) */

static int DESTROY(Stimulation_end_callback_info)(
	struct Stimulation_end_callback_info **stimulation_end_callback_info_address)
/*******************************************************************************
LAST MODIFIED : 29 May 2003

DESCRIPTION :
Destroys <*stimulation_end_callback_info_address>.
==============================================================================*/
{
	int return_code;
	struct Stimulation_end_callback_info *stimulation_end_callback_info;

	ENTER(DESTROY(Stimulation_end_callback_info));
	return_code=0;
	if (stimulation_end_callback_info_address&&
		(stimulation_end_callback_info= *stimulation_end_callback_info_address))
	{
		DEALLOCATE(stimulation_end_callback_info->crate_ids);
		DEALLOCATE(stimulation_end_callback_info->crate_cards);
		DEALLOCATE(stimulation_end_callback_info);
		*stimulation_end_callback_info_address=
			(struct Stimulation_end_callback_info *)NULL;
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Stimulation_end_callback_info) */

#if defined (OLD_CODE)
static int Stimulation_end_callback_info_execute(
	struct Stimulation_end_callback_info *stimulation_end_callback_info,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 29 May 2003

DESCRIPTION :
Executes the callback.
==============================================================================*/
{
	int return_code;

	ENTER(Stimulation_end_callback_info_execute);
	return_code=1;
	if (stimulation_end_callback_info&&(stimulation_end_callback_info->callback))
	{
		(*(stimulation_end_callback_info->callback))(
			stimulation_end_callback_info->callback_data);
	}
	LEAVE;

	return (return_code);
} /* Stimulation_end_callback_info_execute */
#endif /* defined (OLD_CODE) */

/*???DB.  Would like to be static */
DECLARE_OBJECT_FUNCTIONS(Stimulation_end_callback_info)

static int Stimulation_end_callback_info_compare(int *crate_ids_1,
	int *crate_ids_2)
/*******************************************************************************
LAST MODIFIED : 29 May 2003

DESCRIPTION :
Returns -1 if crate_ids_1 < crate_ids_2, 0 if crate_ids_1 = crate_ids_2 and 1 if
crate_ids_1 > crate_ids_2.
==============================================================================*/
{
	int *crate_id_1,*crate_id_2,i,return_code;

	ENTER(Stimulation_end_callback_info_compare);
	return_code=0;
	if ((crate_id_1=crate_ids_1)&&(crate_id_2=crate_ids_2))
	{
		i=module_number_of_unemap_crates;
		while ((0==return_code)&&(i>0))
		{
			i--;
			if (*crate_id_1 < *crate_id_2)
			{
				return_code= -1;
			}
			else
			{
				if (*crate_id_1 > *crate_id_2)
				{
					return_code=1;
				}
			}
			crate_id_1++;
			crate_id_2++;
		}
	}
	LEAVE;

	return (return_code);
} /* Stimulation_end_callback_info_compare */

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Stimulation_end_callback_info,crate_ids, \
	int *,Stimulation_end_callback_info_compare)

/*???DB.  Would like to be static */
DECLARE_INDEXED_LIST_FUNCTIONS(Stimulation_end_callback_info)

static int Stimulation_end_callback_info_no_cards(
	struct Stimulation_end_callback_info *info,void *info_new_void)
/*******************************************************************************
LAST MODIFIED : 29 May 2003

DESCRIPTION :
Updates the <info->crate_cards>, based on <info_new->crate_ids> and then checks
if the <info->crate_cards> are all zero (remove).
==============================================================================*/
{
	int i,return_code;
	struct Stimulation_end_callback_info *info_new;

	ENTER(Stimulation_end_callback_info_no_cards);
	return_code=0;
	if (info&&(info->crate_cards)&&
		(info_new=(struct Stimulation_end_callback_info *)info_new_void)&&
		(info_new->crate_ids))
	{
		return_code=1;
		for (i=0;i<module_number_of_unemap_crates;i++)
		{
			(info->crate_cards)[i] &= ~((info_new->crate_ids)[i]);
			if (0!=(info->crate_cards)[i])
			{
				return_code=0;
			}
		}
	}
	LEAVE;

	return (return_code);
} /* Stimulation_end_callback_info_no_cards */

struct Stimulation_end_callback_info_execute_data
/*******************************************************************************
LAST MODIFIED : 29 May 2003

DESCRIPTION :
Data fof Stimulation_end_callback_info_execute.
==============================================================================*/
{
	int callback_id;
	struct Unemap_crate *crate;
	Unemap_stimulation_end_callback *callback;
	void *callback_data;
}; /* struct Stimulation_end_callback_info_execute_data */

static int Stimulation_end_callback_info_execute(
	struct Stimulation_end_callback_info *info,
	void *stimulation_end_callback_info_execute_data_void)
/*******************************************************************************
LAST MODIFIED : 2 June 2003

DESCRIPTION :
Checks if this <info> has the <callback_id> and <crate>.  If it is, sets the
<crate_cards> to zero and checks if this is the last <crate> for the
<callback_id>.  If it is, sets the callback and callback_data (used to call the
callback, but the callback may try and add to the list again which could cause
problems), and sets return code to 1 to remove from list.
==============================================================================*/
{
	int crate_index,return_code;
	struct Stimulation_end_callback_info_execute_data *data;

	ENTER(Stimulation_end_callback_info_execute);
	return_code=0;
	if (info&&(info->crate_cards)&&
		(data=(struct Stimulation_end_callback_info_execute_data *)
		stimulation_end_callback_info_execute_data_void)&&(0<data->callback_id)&&
		(data->crate))
	{
		crate_index=(data->crate)-module_unemap_crates;
		if (data->callback_id==(info->crate_ids)[crate_index])
		{
			(info->crate_cards)[crate_index]=0;
			return_code=1;
			crate_index=0;
			while (return_code&&(crate_index<module_number_of_unemap_crates))
			{
				if (0!=(info->crate_cards)[crate_index])
				{
					return_code=0;
				}
				crate_index++;
			}
			if (return_code)
			{
				data->callback=info->callback;
				data->callback_data=info->callback_data;
			}
		}
	}
	LEAVE;

	return (return_code);
} /* Stimulation_end_callback_info_execute */

#if defined (CREATE_MUTEX)
#if defined (UNIX) && defined (GENERIC_PC)
#if !defined (CYGWIN)
/*???DB.  Don't get prototype from pthread.h but in library for linux */
#define PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#define PTHREAD_MUTEX_DEFAULT PTHREAD_MUTEX_FAST_NP
#endif /* !defined (CYGWIN) */

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
#endif /* defined (UNIX) && defined (GENERIC_PC) */

static
#if defined (WIN32_SYSTEM)
	HANDLE
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
	pthread_mutex_t *
#endif /* defined (UNIX) */
	create_mutex(void)
/*******************************************************************************
LAST MODIFIED : 23 September 2002

DESCRIPTION :
Creates a mutex.
==============================================================================*/
{
#if defined (WIN32_SYSTEM)
	HANDLE mutex;
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
	int error_code;
	pthread_mutexattr_t attribute;
	pthread_mutex_t *mutex;
#endif /* defined (UNIX) */

	ENTER(create_mutex);
#if defined (WIN32_SYSTEM)
	mutex=CreateMutex(/*no security attributes*/NULL,
		/*do not initially own*/FALSE,/*no name*/(LPCTSTR)NULL);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
	if (ALLOCATE(mutex,pthread_mutex_t,1))
	{
		pthread_mutexattr_init(&attribute);
/*		if (0==pthread_mutexattr_settype(&attribute,PTHREAD_MUTEX_DEFAULT))*/
		/* for recursive */
		if (0==pthread_mutexattr_settype(&attribute,PTHREAD_MUTEX_RECURSIVE))
		{
			if (0!=(error_code=pthread_mutex_init(mutex,&attribute)))
			{
				DEALLOCATE(mutex);
				mutex=(pthread_mutex_t *)NULL;
				display_message(ERROR_MESSAGE,"create_mutex.  "
					"pthread_mutex_init failed.  Error code %d",error_code);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"create_mutex.  Could not set recursive");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_mutex.  Could not allocate");
	}
#endif /* defined (UNIX) */
	LEAVE;

	return (mutex);
} /* create_mutex */

static int destroy_mutex(
#if defined (WIN32_SYSTEM)
	HANDLE *
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
	pthread_mutex_t **
#endif /* defined (UNIX) */
	mutex_address)
/*******************************************************************************
LAST MODIFIED : 24 September 2002

DESCRIPTION :
Destroys a mutex.
==============================================================================*/
{
	int return_code;

	ENTER(destroy_mutex);
	return_code=0;
	/* check argument */
	if (mutex_address&&(*mutex_address))
	{
#if defined (WIN32_SYSTEM)
		CloseHandle(*mutex_address);
		*mutex_address=NULL;
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
		pthread_mutex_destroy(*mutex_address);
		DEALLOCATE(*mutex_address);
		*mutex_address=(pthread_mutex_t *)NULL;
#endif /* defined (UNIX) */
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"destroy_mutex.  Invalid mutex_address");
	}
	LEAVE;

	return (return_code);
} /* destroy_mutex */
#endif /* defined (CREATE_MUTEX) */

static int lock_mutex(
#if defined (WIN32_SYSTEM)
	HANDLE mutex
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
	pthread_mutex_t *mutex
#endif /* defined (UNIX) */
	)
/*******************************************************************************
LAST MODIFIED : 27 July 2000

DESCRIPTION :
Locks the <mutex>.
==============================================================================*/
{
	int return_code;

	ENTER(lock_mutex);
	return_code=1;
#if defined (UNEMAP_THREAD_SAFE)
	if (mutex)
	{
#if defined (WIN32_SYSTEM)
		if (WAIT_FAILED==WaitForSingleObject(mutex,INFINITE))
		{
			display_message(ERROR_MESSAGE,
				"lock_mutex.  WaitForSingleObject failed.  Error code %d",
				GetLastError());
			return_code=0;
		}
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
		pthread_mutex_lock(mutex);
#endif /* defined (UNIX) */
	}
#endif /* defined (UNEMAP_THREAD_SAFE) */
	LEAVE;

	return (return_code);
} /* lock_mutex */

static int unlock_mutex(
#if defined (WIN32_SYSTEM)
	HANDLE mutex
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
	pthread_mutex_t *mutex
#endif /* defined (UNIX) */
	)
/*******************************************************************************
LAST MODIFIED : 27 July 2000

DESCRIPTION :
Unlocks the <mutex>.
==============================================================================*/
{
	int return_code;

	ENTER(unlock_mutex);
	return_code=1;
#if defined (UNEMAP_THREAD_SAFE)
	if (mutex)
	{
#if defined (WIN32_SYSTEM)
		ReleaseMutex(mutex);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
		pthread_mutex_unlock(mutex);
#endif /* defined (UNIX) */
	}
#endif /* defined (UNEMAP_THREAD_SAFE) */
	LEAVE;

	return (return_code);
} /* unlock_mutex */

#if defined (WIN32_SYSTEM)
static void sleep(unsigned seconds)
{
	Sleep((DWORD)seconds*(DWORD)1000);
}
#endif /* defined (WIN32_SYSTEM) */

static int crate_deconfigure(struct Unemap_crate *crate);

static int close_crate_connection(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 11 August 2003

DESCRIPTION :
Closes the connection with the unemap hardware service for the <crate>.
==============================================================================*/
{
	int return_code;
#if defined (OLD_CODE)
/* moved to crate_deconfigure */
#if defined (WIN32_SYSTEM)
	DWORD status;
	HANDLE hEvents[3];
	int number_of_events;
#endif /* defined (WIN32_SYSTEM) */
#endif /* defined (OLD_CODE) */

	ENTER(close_crate_connection);
	return_code=0;
#if defined (DEBUG)
	/*???debug */
	if (crate)
	{
		display_message(INFORMATION_MESSAGE,"enter close_crate_connection %p.  "
			"command %d.  calibration %d.  acquired %d.  scrolling %d"
			".  stimulation %d\n",crate,crate->command_socket,
			crate->calibration_socket,crate->acquired_socket,crate->scrolling_socket,
			crate->stimulation_socket);
	}
	else
	{
		display_message(INFORMATION_MESSAGE,"enter close_crate_connection %p\n",
			crate);
	}
#endif /* defined (DEBUG) */
	if (crate)
	{
		return_code=1;
		if (INVALID_SOCKET!=crate->command_socket)
		{
			lock_mutex(crate->command_socket_mutex);
#if defined (WIN32_SYSTEM)
			closesocket(crate->command_socket);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
			close(crate->command_socket);
#endif /* defined (UNIX) */
			crate->command_socket=INVALID_SOCKET;
			unlock_mutex(crate->command_socket_mutex);
		}
		crate_deconfigure(crate);
		if (INVALID_SOCKET!=crate->scrolling_socket)
		{
#if defined (WIN32_SYSTEM)
			closesocket(crate->scrolling_socket);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
			close(crate->scrolling_socket);
#endif /* defined (UNIX) */
			crate->scrolling_socket=INVALID_SOCKET;
		}
		if (INVALID_SOCKET!=crate->calibration_socket)
		{
#if defined (WIN32_SYSTEM)
			closesocket(crate->calibration_socket);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
			close(crate->calibration_socket);
#endif /* defined (UNIX) */
			crate->calibration_socket=INVALID_SOCKET;
		}
		if (INVALID_SOCKET!=crate->acquired_socket)
		{
#if defined (WIN32_SYSTEM)
			closesocket(crate->acquired_socket);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
			close(crate->acquired_socket);
#endif /* defined (UNIX) */
			crate->acquired_socket=INVALID_SOCKET;
		}
		if (INVALID_SOCKET!=crate->stimulation_socket)
		{
#if defined (WIN32_SYSTEM)
			closesocket(crate->stimulation_socket);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
			close(crate->stimulation_socket);
#endif /* defined (UNIX) */
			crate->stimulation_socket=INVALID_SOCKET;
		}
		/* free mutex's */
		if (crate->command_socket_mutex)
		{
#if defined (CREATE_MUTEX)
			destroy_mutex(&(crate->command_socket_mutex));
#else /* defined (CREATE_MUTEX) */
#if defined (WIN32_SYSTEM)
			CloseHandle(crate->command_socket_mutex);
			crate->command_socket_mutex=(HANDLE)NULL;
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
			pthread_mutex_destroy(crate->command_socket_mutex);
			crate->command_socket_mutex=(pthread_mutex_t *)NULL;
#endif /* defined (UNIX) */
#endif /* defined (CREATE_MUTEX) */
		}
		if (crate->acquired_socket_mutex)
		{
#if defined (CREATE_MUTEX)
			destroy_mutex(&(crate->acquired_socket_mutex));
#else /* defined (CREATE_MUTEX) */
#if defined (WIN32_SYSTEM)
			CloseHandle(crate->acquired_socket_mutex);
			crate->acquired_socket_mutex=(HANDLE)NULL;
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
			pthread_mutex_destroy(crate->acquired_socket_mutex);
			crate->acquired_socket_mutex=(pthread_mutex_t *)NULL;
#endif /* defined (UNIX) */
#endif /* defined (CREATE_MUTEX) */
		}
		DEALLOCATE(crate->configured_channel_offsets);
		DEALLOCATE(crate->configured_channels);
		crate->number_of_configured_channels=0;
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
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave close_crate_connection\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* close_crate_connection */

static int close_connection(void)
/*******************************************************************************
LAST MODIFIED : 29 May 2003

DESCRIPTION :
Closes the connection with the unemap hardware service.
==============================================================================*/
{
	int return_code;
	struct Unemap_crate *crate;

	ENTER(close_connection);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter close_connection\n");
#endif /* defined (DEBUG) */
	if (connection_mutex)
	{
		lock_mutex(connection_mutex);
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"after lock_mutex\n");
#endif /* defined (DEBUG) */
	return_code=1;
	if (crate=module_unemap_crates)
	{
		while (module_number_of_unemap_crates>0)
		{
			close_crate_connection(crate);
			module_number_of_unemap_crates--;
			crate++;
		}
#if defined (WIN32_SYSTEM)
		WSACleanup();
#endif /* defined (WIN32_SYSTEM) */
#if defined (OLD_CODE)
		/*???DB.  May not even have started */
		if (module_stimulation_end_callback_info_list)
		{
			FOR_EACH_OBJECT_IN_LIST(Stimulation_end_callback_info)(
				Stimulation_end_callback_info_execute,(void *)NULL,
				module_stimulation_end_callback_info_list);
		}
#endif /* defined (OLD_CODE) */
		DESTROY_LIST(Stimulation_end_callback_info)(
			&module_stimulation_end_callback_info_list);
		DEALLOCATE(module_unemap_crates);
		module_number_of_unemap_crates=0;
		module_number_of_channels=0;
		/*???DB.  seems to need time to settle down.  Something to do with
			close_connection in service */
		sleep(1);
	}
	allow_open_connection=1;
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"before unlock_mutex\n");
#endif /* defined (DEBUG) */
	if (stimulation_mutex)
	{
#if defined (CREATE_MUTEX)
		destroy_mutex(&stimulation_mutex);
#else /* defined (CREATE_MUTEX) */
#if defined (WIN32_SYSTEM)
		CloseHandle(stimulation_mutex);
		stimulation_mutex=(HANDLE)NULL;
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
		pthread_mutex_destroy(stimulation_mutex);
		stimulation_mutex=(pthread_mutex_t *)NULL;
#endif /* defined (UNIX) */
#endif /* defined (CREATE_MUTEX) */
	}
	if (connection_mutex)
	{
		unlock_mutex(connection_mutex);
#if defined (CREATE_MUTEX)
		destroy_mutex(&connection_mutex);
#else /* defined (CREATE_MUTEX) */
#if defined (WIN32_SYSTEM)
		CloseHandle(connection_mutex);
		connection_mutex=(HANDLE)NULL;
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
		pthread_mutex_destroy(connection_mutex);
		connection_mutex=(pthread_mutex_t *)NULL;
#endif /* defined (UNIX) */
#endif /* defined (CREATE_MUTEX) */
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave close_connection\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* close_connection */

#if defined (USE_SOCKETS)
static int socket_recv(
#if defined (WIN32_SYSTEM)
	SOCKET socket,HANDLE read_event,
#endif /* defined (WIN32_SYSTEM) */
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
#if defined (WIN32_SYSTEM)
			if (read_event)
			{
				ResetEvent(read_event);
			}
#endif /* defined (WIN32_SYSTEM) */
			return_code=recv(socket,buffer+buffer_received,
				buffer_length-buffer_received,flags);
		} while ((SOCKET_ERROR==return_code)&&
#if defined (WIN32_SYSTEM)
			(WSAEWOULDBLOCK==WSAGetLastError())
#endif /* defined (WIN32_SYSTEM) */
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
#if defined (WIN32_SYSTEM)
			"socket_recv.  recv() failed %d",WSAGetLastError()
#endif /* defined (WIN32_SYSTEM) */
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
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"socket_recv:close_crate_connection %d\n",socket);
#if defined (DEBUG)
#endif /* defined (DEBUG) */
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
#if defined (WIN32_SYSTEM)
	SOCKET socket,
#endif /* defined (WIN32_SYSTEM) */
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
#if defined (WIN32_SYSTEM)
	int last_error;
#endif /* defined (WIN32_SYSTEM) */

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
#if defined (WIN32_SYSTEM)
			(WSAEWOULDBLOCK==(last_error=WSAGetLastError()))
#endif /* defined (WIN32_SYSTEM) */
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
#if defined (WIN32_SYSTEM)
			"socket_send.  send() failed %d",last_error
#endif /* defined (WIN32_SYSTEM) */
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
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"socket_send:close_crate_connection\n");
#if defined (DEBUG)
#endif /* defined (DEBUG) */
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
#if defined (BACKGROUND_SAVING) && defined (UNIX)
static int acquired_socket_callback(int file_descriptor,void *crate_void);

static void *acquired_socket_callback_process(void *crate_void)
#else /* defined (BACKGROUND_SAVING) && defined (UNIX) */
static int acquired_socket_callback(
#if defined (WIN32_SYSTEM)
	LPVOID *crate_void,HANDLE read_event
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
	int file_descriptor,void *crate_void
#endif /* defined (UNIX) */
	)
#endif /* defined (BACKGROUND_SAVING) && defined (UNIX) */
/*******************************************************************************
LAST MODIFIED : 21 August 2003

DESCRIPTION :
Called when there is input on the acquired socket.

The <module_acquired_callback> is responsible for deallocating the samples.
==============================================================================*/
{
	int acquired_number_of_channels,i,k,number_of_channels,
		number_of_configured_channels;
	long message_size;
	short *sample,*samples;
	struct Unemap_crate *crate;
	unsigned char message_header[2+sizeof(long)];
	unsigned long j,number_of_samples,offset;
#if !(defined (BACKGROUND_SAVING) && defined (UNIX))
	int return_code;
#endif /* !(defined (BACKGROUND_SAVING) && defined (UNIX)) */

#if defined (BACKGROUND_SAVING) && defined (UNIX)
	ENTER(acquired_socket_callback_process);
#else /* defined (BACKGROUND_SAVING) && defined (UNIX) */
	ENTER(acquired_socket_callback);
	return_code=0;
#endif /* defined (BACKGROUND_SAVING) && defined (UNIX) */
#if defined (DEBUG)
	/*???debug */
#if defined (BACKGROUND_SAVING) && defined (UNIX)
	display_message(INFORMATION_MESSAGE,
		"enter acquired_socket_callback_process.  %p\n",crate_void);
#else /* defined (BACKGROUND_SAVING) && defined (UNIX) */
	display_message(INFORMATION_MESSAGE,"enter acquired_socket_callback.  %p\n",
		crate_void);
#endif /* defined (BACKGROUND_SAVING) && defined (UNIX) */
#endif /* defined (DEBUG) */
#if defined (UNIX)
#if !defined (BACKGROUND_SAVING)
	USE_PARAMETER(file_descriptor);
#endif /* !defined (BACKGROUND_SAVING) */
#endif /* defined (UNIX) */
	if (crate=(struct Unemap_crate *)crate_void)
	{
		lock_mutex(crate->acquired_socket_mutex);
		DEALLOCATE((crate->acquired).samples);
		(crate->acquired).number_of_channels=0;
		(crate->acquired).number_of_samples=0;
		/* get the header back */
		if (SOCKET_ERROR!=socket_recv(crate->acquired_socket,
#if defined (WIN32_SYSTEM)
			read_event,
#endif /* defined (WIN32_SYSTEM) */
			message_header,2+sizeof(long),0))
		{
			if (message_header[0])
			{
				/* succeeded */
				memcpy(&message_size,message_header+2,sizeof(message_size));
				if ((int)sizeof(number_of_channels)<=message_size)
				{
					if (SOCKET_ERROR!=socket_recv(crate->acquired_socket,
#if defined (WIN32_SYSTEM)
						read_event,
#endif /* defined (WIN32_SYSTEM) */
						(unsigned char *)&number_of_channels,sizeof(number_of_channels),0))
					{
						(crate->acquired).number_of_channels=number_of_channels;
						message_size -= sizeof(number_of_channels);
#if defined (DEBUG)
						/*???debug */
						display_message(INFORMATION_MESSAGE,"number_of_channels.  %d\n",
							number_of_channels);
#endif /* defined (DEBUG) */
						if ((int)sizeof(number_of_samples)<=message_size)
						{
							if (SOCKET_ERROR!=socket_recv(crate->acquired_socket,
#if defined (WIN32_SYSTEM)
								read_event,
#endif /* defined (WIN32_SYSTEM) */
								(unsigned char *)&number_of_samples,sizeof(number_of_samples),
								0))
							{
								message_size -= sizeof(number_of_samples);
#if defined (DEBUG)
								/*???debug */
								display_message(INFORMATION_MESSAGE,"number_of_samples.  %lu\n",
									number_of_samples);
#endif /* defined (DEBUG) */
								if (number_of_channels*(long)(number_of_samples*sizeof(short))==
									message_size)
								{
									if (0<message_size)
									{
										ALLOCATE(samples,short,
											number_of_channels*number_of_samples);
										if (samples)
										{
#if defined (DEBUG)
											/*???debug */
											int retval;
#endif /* defined (DEBUG) */

											if (SOCKET_ERROR!=
#if defined (DEBUG)
												/*???debug */
												(retval=
#endif /* defined (DEBUG) */
												socket_recv(crate->acquired_socket,
#if defined (WIN32_SYSTEM)
												read_event,
#endif /* defined (WIN32_SYSTEM) */
												(unsigned char *)samples,
												number_of_channels*(int)number_of_samples*sizeof(short),
												0)
#if defined (DEBUG)
												/*???debug */
												)
#endif /* defined (DEBUG) */
												)
											{
#if defined (DEBUG)
												/*???debug */
#if defined (WIN32_SYSTEM)
												display_message(INFORMATION_MESSAGE,
													"received.  %ld %lu %ld %ld %ld\n",retval,
													number_of_channels*(int)number_of_samples*
													sizeof(short),WaitForSingleObject(read_event,
													(DWORD)0),WAIT_OBJECT_0,WAIT_TIMEOUT);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
												display_message(INFORMATION_MESSAGE,
													"received.  %d %d\n",retval,number_of_channels*
													(int)number_of_samples*sizeof(short));
#endif /* defined (UNIX) */
#endif /* defined (DEBUG) */
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
				}
			}
		}
		/*???DB.  Should this be thread protected? */
		(crate->acquired).complete=1;
		i=module_number_of_unemap_crates;
		crate=module_unemap_crates;
		acquired_number_of_channels=0;
		number_of_channels=0;
		number_of_configured_channels=0;
		number_of_samples=0;
		while ((i>0)&&((crate->acquired).complete))
		{
			number_of_configured_channels += crate->number_of_configured_channels;
			if (4<=crate->software_version)
			{
				number_of_channels += crate->number_of_configured_channels;
			}
			else
			{
				number_of_channels += crate->number_of_channels;
			}
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"%d) number_of_channels=%d (%d), number_of_samples=%lu\n",i,
				crate->number_of_configured_channels,
				(crate->acquired).number_of_channels,
				(crate->acquired).number_of_samples);
#endif /* defined (DEBUG) */
			if (0<(crate->acquired).number_of_channels)
			{
				acquired_number_of_channels += (crate->acquired).number_of_channels;
				if (number_of_samples<(crate->acquired).number_of_samples)
				{
					number_of_samples=(crate->acquired).number_of_samples;
				}
			}
			crate++;
			i--;
		}
		if (0==i)
		{
			if (module_acquired_callback)
			{
				if ((0<number_of_samples)&&(((0!=module_acquired_channel_number)&&
					(1==acquired_number_of_channels))||
					((0==module_acquired_channel_number)&&
					(number_of_channels==acquired_number_of_channels))))
				{
					ALLOCATE(samples,short,
						number_of_configured_channels*number_of_samples);
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,"acquired_number_of_channels=%d, "
						"number_of_samples=%lu, samples=%p, %d\n",
						acquired_number_of_channels,number_of_samples,samples,
						acquired_number_of_channels*number_of_samples);
#endif /* defined (DEBUG) */
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
									if (j<(crate->acquired).number_of_samples)
									{
										offset=j*((crate->acquired).number_of_channels);
										if (4<=crate->software_version)
										{
											for (k=0;k<crate->number_of_configured_channels;k++)
											{
												sample[(crate->configured_channel_offsets)[k]]=
													((crate->acquired).samples)[offset];
												offset++;
											}
										}
										else
										{
											for (k=0;k<crate->number_of_configured_channels;k++)
											{
												if ((crate->configured_channels)[k]<=
													(crate->acquired).number_of_channels)
												{
													sample[(crate->configured_channel_offsets)[k]]=
														((crate->acquired).samples)[offset+
														(crate->configured_channels)[k]-1];
												}
											}
										}
									}
									crate++;
								}
								sample += number_of_configured_channels;
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
					}
					else
					{
						number_of_samples=0;
					}
				}
				else
				{
					samples=(short *)NULL;
					number_of_samples=0;
				}
				(*module_acquired_callback)(module_acquired_channel_number,
					(int)number_of_samples,samples,module_acquired_callback_data);
				module_acquired_callback=(Unemap_acquired_data_callback *)NULL;
				module_acquired_callback_data=(void *)NULL;
			}
		}
		crate=(struct Unemap_crate *)crate_void;
#if defined (BACKGROUND_SAVING) && defined (UNIX)
		lock_mutex(crate->event_dispatcher_mutex);
		crate->acquired_socket_xid=Event_dispatcher_add_simple_descriptor_callback(
			crate->event_dispatcher,crate->acquired_socket,
			acquired_socket_callback,(void *)crate);
		unlock_mutex(crate->event_dispatcher_mutex);
#endif /* defined (BACKGROUND_SAVING) && defined (UNIX) */
		unlock_mutex(crate->acquired_socket_mutex);
	}
#if defined (DEBUG)
	/*???debug */
#if defined (BACKGROUND_SAVING) && defined (UNIX)
	display_message(INFORMATION_MESSAGE,
		"leave acquired_socket_callback_process.  %p\n",crate);
#else /* defined (BACKGROUND_SAVING) && defined (UNIX) */
	display_message(INFORMATION_MESSAGE,"leave acquired_socket_callback.  %p\n",
		crate);
#endif /* defined (BACKGROUND_SAVING) && defined (UNIX) */
#endif /* defined (DEBUG) */
	LEAVE;
#if defined (BACKGROUND_SAVING) && defined (UNIX)

	return (NULL);
} /* acquired_socket_callback_process */

static int acquired_socket_callback(int file_descriptor,void *crate_void)
/*******************************************************************************
LAST MODIFIED : 1 September 2002

DESCRIPTION :
Called when there is input on the acquired socket.

The <module_acquired_callback> is responsible for deallocating the samples.
==============================================================================*/
{
	int return_code;
	pthread_t thread_id;
	struct Unemap_crate *crate;

	ENTER(acquired_socket_callback);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter acquired_socket_callback.  %p\n",
		crate_void);
#endif /* defined (DEBUG) */
	USE_PARAMETER(file_descriptor);
	return_code = 1;
	if ((crate=(struct Unemap_crate *)crate_void)&&(crate->acquired_socket_xid))
	{
#if defined (DEBUG)
		/*???debug */
		display_message(INFORMATION_MESSAGE,"crate->acquired_socket_xid=%lx\n",
			crate->acquired_socket_xid);
#endif /* defined (DEBUG) */
		/* so that don't get multiple acquired_socket_callback calls */
		lock_mutex(crate->event_dispatcher_mutex);
		Event_dispatcher_remove_descriptor_callback(crate->event_dispatcher,
			crate->acquired_socket_xid);
		unlock_mutex(crate->event_dispatcher_mutex);
		crate->acquired_socket_xid=
			(struct Event_dispatcher_descriptor_callback *)NULL;
		if (pthread_create(&thread_id,(pthread_attr_t *)NULL,
			acquired_socket_callback_process,(void *)crate_void))
		{
			acquired_socket_callback_process((void *)crate_void);
			display_message(ERROR_MESSAGE,
				"acquired_socket_callback.  pthread_create failed");
		}
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave acquired_socket_callback.  %p\n",
		crate_void);
#endif /* defined (DEBUG) */
	LEAVE;
#endif /* defined (BACKGROUND_SAVING) && defined (UNIX) */

	return (return_code);
} /* acquired_socket_callback */
#endif /* defined (USE_SOCKETS) */

#if defined (WIN32_SYSTEM) && defined (USE_SOCKETS)
DWORD WINAPI acquired_thread_function(LPVOID crate_void)
/*******************************************************************************
LAST MODIFIED : 30 August 2002

DESCRIPTION :
Thread to watch the acquired socket.
???DB.  Change hEvents[1] to automatic ?
==============================================================================*/
{
	DWORD dwWait,return_code;
	HANDLE hEvents[2]={NULL,NULL};
	int running;
#if defined (OLD_CODE)
	int i;
#endif /* defined (OLD_CODE) */
	struct Unemap_crate *crate;

	ENTER(acquired_thread_function);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter acquired_thread_function\n");
#endif /* defined (DEBUG) */
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
							/* for a FD_READ event, recv re-enables signalling and if there is
								still data to be read at completion the event is set again.
								This means that the event has to be reset before each recv */
							acquired_socket_callback(crate_void,hEvents[1]);
						}
						else
						{
							running=0;
#if defined (DEBUG)
							/*???debug */
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
#if defined (OLD_CODE)
		if (INVALID_SOCKET!=crate->acquired_socket)
		{
			closesocket(crate->acquired_socket);
			crate->acquired_socket=INVALID_SOCKET;
		}
#endif /* defined (OLD_CODE) */
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
		/* indicate that thread has finished */
		SetEvent(crate->acquired_socket_thread_stopped_event);
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave acquired_thread_function\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* acquired_thread_function */
#endif /* defined (WIN32_SYSTEM) && defined (USE_SOCKETS) */

#if defined (USE_SOCKETS)
int calibration_socket_callback(
#if defined (WIN32_SYSTEM)
	LPVOID *crate_void,HANDLE read_event
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
	int file_descriptor,void *crate_void
#endif /* defined (UNIX) */
	)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

DESCRIPTION :
Called when there is input on the calibration socket.
==============================================================================*/
{
	float *channel_gains,*channel_offsets;
	int *channel_numbers,i,number_of_channels,return_code;
	long message_size;
	struct Unemap_crate *crate;
	unsigned char message_header[2+sizeof(long)];

	ENTER(calibration_socket_callback);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"calibration_socket_callback enter.  %p\n",module_calibration_end_callback);
#endif /* defined (DEBUG) */
#if defined (UNIX)
	USE_PARAMETER(file_descriptor);
#endif /* defined (UNIX) */
	return_code = 1;
	if (crate=(struct Unemap_crate *)crate_void)
	{
		(crate->calibration).complete=1;
		(crate->calibration).number_of_channels=0;
		DEALLOCATE((crate->calibration).channel_numbers);
		DEALLOCATE((crate->calibration).channel_offsets);
		DEALLOCATE((crate->calibration).channel_gains);
		/* get the header back */
		if (SOCKET_ERROR!=socket_recv(crate->calibration_socket,
#if defined (WIN32_SYSTEM)
			read_event,
#endif /* defined (WIN32_SYSTEM) */
			message_header,2+sizeof(long),0))
		{
			if (message_header[0])
			{
				/* succeeded */
				memcpy(&message_size,message_header+2,sizeof(message_size));
				if ((int)sizeof(number_of_channels)<=message_size)
				{
					if (SOCKET_ERROR!=socket_recv(crate->calibration_socket,
#if defined (WIN32_SYSTEM)
						read_event,
#endif /* defined (WIN32_SYSTEM) */
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
#if defined (WIN32_SYSTEM)
								read_event,
#endif /* defined (WIN32_SYSTEM) */
								(unsigned char *)channel_numbers,number_of_channels*sizeof(int),
								0))&&(SOCKET_ERROR!=socket_recv(crate->calibration_socket,
#if defined (WIN32_SYSTEM)
								read_event,
#endif /* defined (WIN32_SYSTEM) */
								(unsigned char *)channel_offsets,
								number_of_channels*sizeof(float),0))&&
								(SOCKET_ERROR!=socket_recv(crate->calibration_socket,
#if defined (WIN32_SYSTEM)
								read_event,
#endif /* defined (WIN32_SYSTEM) */
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
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"calibration_socket_callback leave %d\n",
		return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* calibration_socket_callback */
#endif /* defined (USE_SOCKETS) */

#if defined (WIN32_SYSTEM) && defined (USE_SOCKETS)
DWORD WINAPI calibration_thread_function(LPVOID crate_void)
/*******************************************************************************
LAST MODIFIED : 30 August 2002

DESCRIPTION :
Thread to watch the calibration socket.
???DB.  Change hEvents[1] to automatic ?
==============================================================================*/
{
	DWORD dwWait,return_code;
	HANDLE hEvents[2]={NULL,NULL};
	int running;
	struct Unemap_crate *crate;

	ENTER(calibration_thread_function);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter calibration_thread_function\n");
#endif /* defined (DEBUG) */
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
							/* for a FD_READ event, recv re-enables signalling and if there is
								still data to be read at completion the event is set again.
								This means that the event has to be reset before each recv */
							calibration_socket_callback(crate_void,hEvents[1]);
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
#if defined (OLD_CODE)
		if (INVALID_SOCKET!=crate->calibration_socket)
		{
			closesocket(crate->calibration_socket);
			crate->calibration_socket=INVALID_SOCKET;
		}
#endif /* defined (OLD_CODE) */
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
		/* indicate that thread has finished */
		SetEvent(crate->calibration_socket_thread_stopped_event);
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave calibration_thread_function\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* calibration_thread_function */
#endif /* defined (WIN32_SYSTEM) && defined (USE_SOCKETS) */

#if defined (USE_SOCKETS)
int scrolling_socket_callback(
#if defined (WIN32_SYSTEM)
	LPVOID crate_void,HANDLE read_event
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
	int file_descriptor,void *crate_void
#endif /* defined (UNIX) */
	)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

DESCRIPTION :
Called when there is input on the scrolling socket.
???DB.  What if scrolling callback for next event comes back before all crates
	have sent back current?
==============================================================================*/
{
	int *channel_numbers,i,number_of_channels,number_of_values_per_channel,
		return_code,*temp_channel_numbers;
	long message_size;
	short *temp_values,*values;
	struct Unemap_crate *crate;
	unsigned char *byte_array,message_header[2+sizeof(long)];
#if defined (WIN32_SYSTEM)
	int length;
	unsigned char *temp_byte_array;
#endif /* defined (WIN32_SYSTEM) */

	ENTER(scrolling_socket_callback);
#if defined (UNIX)
	USE_PARAMETER(file_descriptor);
#endif /* defined (UNIX) */
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
	return_code=1;
	if (crate=(struct Unemap_crate *)crate_void)
	{
		/* get the header back */
		if (SOCKET_ERROR!=socket_recv(crate->scrolling_socket,
#if defined (WIN32_SYSTEM)
			read_event,
#endif /* defined (WIN32_SYSTEM) */
			message_header,2+sizeof(long),0))
		{
			memcpy(&message_size,message_header+2,sizeof(message_size));
			if (ALLOCATE(byte_array,unsigned char,message_size))
			{
				if (SOCKET_ERROR!=socket_recv(crate->scrolling_socket,
#if defined (WIN32_SYSTEM)
					read_event,
#endif /* defined (WIN32_SYSTEM) */
					byte_array,message_size,0))
				{
					if (module_scrolling_callback
#if defined (WIN32_USER_INTERFACE)
						||module_scrolling_window
#endif /* defined (WIN32_USER_INTERFACE) */
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
#if defined (WIN32_USER_INTERFACE)
			||module_scrolling_window
#endif /* defined (WIN32_USER_INTERFACE) */
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
#if defined (WIN32_USER_INTERFACE)
					if (module_scrolling_callback)
					{
#endif /* defined (WIN32_USER_INTERFACE) */
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
#if defined (WIN32_USER_INTERFACE)
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
#endif /* defined (WIN32_USER_INTERFACE) */
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

	return (return_code);
} /* scrolling_socket_callback */
#endif /* defined (USE_SOCKETS) */

#if defined (WIN32_SYSTEM) && defined (USE_SOCKETS)
DWORD WINAPI scrolling_thread_function(LPVOID crate_void)
/*******************************************************************************
LAST MODIFIED : 30 August 2002

DESCRIPTION :
Thread to watch the scrolling socket.
???DB.  Change hEvents[1] to automatic ?
==============================================================================*/
{
	DWORD dwWait,return_code;
	HANDLE hEvents[2]={NULL,NULL};
	int running;
#if defined (OLD_CODE)
	int i;
#endif /* defined (OLD_CODE) */
	struct Unemap_crate *crate;

	ENTER(scrolling_thread_function);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter scrolling_thread_function.  %p\n",
		crate_void);
#endif /* defined (DEBUG) */
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
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,"running %p\n",hEvents[0]);
#endif /* defined (DEBUG) */
					running=1;
					while (1==running)
					{
						dwWait=WaitForMultipleObjects(2,hEvents,FALSE,INFINITE);
						if (WAIT_OBJECT_0+1==dwWait)
						{
							/* for a FD_READ event, recv re-enables signalling and if there is
								still data to be read at completion the event is set again.
								This means that the event has to be reset before each recv */
							scrolling_socket_callback(crate_void,hEvents[1]);
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
#if defined (OLD_CODE)
		if (INVALID_SOCKET!=crate->scrolling_socket)
		{
			closesocket(crate->scrolling_socket);
			crate->scrolling_socket=INVALID_SOCKET;
		}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
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
#endif /* defined (OLD_CODE) */
		/* indicate that thread has finished */
		SetEvent(crate->scrolling_socket_thread_stopped_event);
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave scrolling_thread_function\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* scrolling_thread_function */
#endif /* defined (WIN32_SYSTEM) && defined (USE_SOCKETS) */

#if defined (USE_SOCKETS)
int stimulation_socket_callback(
#if defined (WIN32_SYSTEM)
	LPVOID crate_void,HANDLE read_event
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
	int file_descriptor,void *crate_void
#endif /* defined (UNIX) */
	)
/*******************************************************************************
LAST MODIFIED : 2 June 2003

DESCRIPTION :
Called when there is input on the stimulation socket.
==============================================================================*/
{
	int stimulation_end_callback_id,return_code;
	long message_size;
	struct Stimulation_end_callback_info_execute_data
		stimulation_end_callback_info_execute_data;
	struct Unemap_crate *crate;
	unsigned char *byte_array,message_header[2+sizeof(long)];

	ENTER(stimulation_socket_callback);
#if defined (UNIX)
	USE_PARAMETER(file_descriptor);
#endif /* defined (UNIX) */
	return_code=1;
	if ((crate=(struct Unemap_crate *)crate_void)&&
		(INVALID_SOCKET!=crate->stimulation_socket))
	{
		if (stimulation_mutex)
		{
			lock_mutex(stimulation_mutex);
		}
		/* get the header back */
		if (SOCKET_ERROR!=socket_recv(crate->stimulation_socket,
#if defined (WIN32_SYSTEM)
			read_event,
#endif /* defined (WIN32_SYSTEM) */
			message_header,2+sizeof(long),0))
		{
			memcpy(&message_size,message_header+2,sizeof(message_size));
			if (ALLOCATE(byte_array,unsigned char,message_size))
			{
				if (SOCKET_ERROR!=socket_recv(crate->stimulation_socket,
#if defined (WIN32_SYSTEM)
					read_event,
#endif /* defined (WIN32_SYSTEM) */
					byte_array,message_size,0))
				{
					stimulation_end_callback_id= *((int *)byte_array);
					if ((0<stimulation_end_callback_id)&&
						module_stimulation_end_callback_info_list)
					{
						stimulation_end_callback_info_execute_data.callback_id=
							stimulation_end_callback_id;
						stimulation_end_callback_info_execute_data.crate=crate;
						stimulation_end_callback_info_execute_data.callback=
							(Unemap_stimulation_end_callback *)NULL;
						stimulation_end_callback_info_execute_data.callback_data=NULL;
						REMOVE_OBJECTS_FROM_LIST_THAT(Stimulation_end_callback_info)(
							Stimulation_end_callback_info_execute,
							(void *)&stimulation_end_callback_info_execute_data,
							module_stimulation_end_callback_info_list);
						if (stimulation_end_callback_info_execute_data.callback)
						{
							(*(stimulation_end_callback_info_execute_data.callback))(
								stimulation_end_callback_info_execute_data.callback_data);
						}
					}
				}
				DEALLOCATE(byte_array);
			}
		}
		if (stimulation_mutex)
		{
			unlock_mutex(stimulation_mutex);
		}
	}
	LEAVE;

	return (return_code);
} /* stimulation_socket_callback */
#endif /* defined (USE_SOCKETS) */

#if defined (WIN32_SYSTEM) && defined (USE_SOCKETS)
DWORD WINAPI stimulation_thread_function(LPVOID crate_void)
/*******************************************************************************
LAST MODIFIED : 1 June 2003

DESCRIPTION :
Thread to watch the stimulation socket.
???DB.  Change hEvents[1] to automatic ?
==============================================================================*/
{
	DWORD dwWait,return_code;
	HANDLE hEvents[2]={NULL,NULL};
	int running;
	struct Unemap_crate *crate;

	ENTER(stimulation_thread_function);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"enter stimulation_thread_function.  %p\n",crate_void);
#endif /* defined (DEBUG) */
	if ((crate=(struct Unemap_crate *)crate_void)&&
		(INVALID_SOCKET!=crate->stimulation_socket))
	{
		hEvents[0]=NULL;
		hEvents[1]=NULL;
		return_code=0;
		crate->stimulation_socket_thread_stop_event=CreateEvent(
			/*no security attributes*/NULL,
			/*manual reset event*/TRUE,/*not-signalled*/FALSE,/*no name*/NULL);
		if (crate->stimulation_socket_thread_stop_event)
		{
			hEvents[0]=crate->stimulation_socket_thread_stop_event;
			/* create the event object object use in overlapped i/o */
			hEvents[1]=CreateEvent(/*no security attributes*/NULL,
				/*manual reset event*/TRUE,/*not-signalled*/FALSE,/*no name*/NULL);
			if (hEvents[1])
			{
				if (0==WSAEventSelect(crate->stimulation_socket,hEvents[1],FD_READ))
				{
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,"running %p\n",hEvents[0]);
#endif /* defined (DEBUG) */
					running=1;
					while (1==running)
					{
						dwWait=WaitForMultipleObjects(2,hEvents,FALSE,INFINITE);
						if (WAIT_OBJECT_0+1==dwWait)
						{
							/* for a FD_READ event, recv re-enables signalling and if there is
								still data to be read at completion the event is set again.
								This means that the event has to be reset before each recv */
							stimulation_socket_callback(crate_void,hEvents[1]);
						}
						else
						{
							running=0;
#if defined (DEBUG)
							display_message(INFORMATION_MESSAGE,
								"stimulation_thread_function.  Stop %d %d\n",dwWait,
								WAIT_OBJECT_0+1);
#endif /* defined (DEBUG) */
						}
					}
				}
			}
		}
		/* cleanup */
		if (crate->stimulation_socket_thread_stop_event)
		{
			CloseHandle(crate->stimulation_socket_thread_stop_event);
			crate->stimulation_socket_thread_stop_event=NULL;
		}
		/* overlapped i/o event */
		if (hEvents[1])
		{
			CloseHandle(hEvents[1]);
		}
		/* indicate that thread has finished */
		SetEvent(crate->stimulation_socket_thread_stopped_event);
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave stimulation_thread_function\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* stimulation_thread_function */
#endif /* defined (WIN32_SYSTEM) && defined (USE_SOCKETS) */

static int crate_get_software_version(struct Unemap_crate *crate,
	int *software_version)
/*******************************************************************************
LAST MODIFIED : 2 June 2003

DESCRIPTION :
Used for maintaining backward compatability when the service is changed.

The function does not need the hardware to be configured.

Returns the unemap <software_version> for the <crate>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_get_software_version);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"enter crate_get_software_version %p %p\n",crate,software_version);
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if (crate&&software_version)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			lock_mutex(crate->command_socket_mutex);
			buffer[0]=UNEMAP_GET_SOFTWARE_VERSION_CODE;
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
				retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
					(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
					buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
							(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
							buffer,sizeof(int),0))
						{
							memcpy(software_version,buffer,sizeof(int));
#if defined (DEBUG)
							display_message(INFORMATION_MESSAGE," %d",*software_version);
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
						"crate_get_software_version.  socket_recv() failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_software_version.  socket_send() failed");
			}
			unlock_mutex(crate->command_socket_mutex);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_software_version.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_get_software_version.  "
			"Missing crate (%p) or software_version (%p)",crate,software_version);
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave crate_get_software_version %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_get_software_version */

static int crate_configure_start(struct Unemap_crate *crate,int slave,
	float sampling_frequency,int number_of_samples_in_buffer,
#if defined (WIN32_USER_INTERFACE)
	HWND scrolling_window,UINT scrolling_message,
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (UNIX)
	struct Event_dispatcher *event_dispatcher,
#endif /* defined (UNIX) */
	Unemap_hardware_callback *scrolling_callback,void *scrolling_callback_data,
	float scrolling_frequency,float scrolling_callback_frequency,
	int synchronization_card)
/*******************************************************************************
LAST MODIFIED : 11 August 2003

DESCRIPTION :
Configures the <crate> for sampling at the specified <sampling_frequency> and
with the specified <number_of_samples_in_buffer>. <sampling_frequency>,
<number_of_samples_in_buffer>, <scrolling_frequency> and
<scrolling_callback_frequency> are requests only.  If <slave> is non-zero, then
the sampling control signal comes from another crate, otherwise it is generated
by the crate.

See <unemap_configure> for more details.

???DB.  To be done.  Implement slave sampling for hardware.
==============================================================================*/
{
	float sampling_frequency_slave,temp_scrolling_callback_frequency,
		temp_scrolling_frequency;
	int number_of_channels,return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+3*sizeof(float)+3*sizeof(int)+sizeof(long)];
#if defined (WIN32_SYSTEM)
	DWORD acquired_thread_id;
	HANDLE acquired_thread;
		/*???DB.  Global variable ? */
	DWORD calibration_thread_id;
	HANDLE calibration_thread;
		/*???DB.  Global variable ? */
	DWORD scrolling_thread_id;
	HANDLE scrolling_thread;
		/*???DB.  Global variable ? */
	DWORD stimulation_thread_id;
	HANDLE stimulation_thread;
		/*???DB.  Global variable ? */
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
#if !defined (CREATE_MUTEX)
	int error_code;
#endif /* !defined (CREATE_MUTEX) */
#endif /* defined (UNIX) */

	ENTER(crate_configure_start);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter crate_configure_start %p\n",crate);
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if (crate&&(0<sampling_frequency)&&(0<number_of_samples_in_buffer)&&
		(!scrolling_callback||((0<scrolling_callback_frequency)&&
		(scrolling_frequency>=scrolling_callback_frequency)))
#if defined (UNIX)
		&&event_dispatcher
#endif /* defined (UNIX) */
		)
	{
		/* if the hardware service version is less than 3 then it won't have a
			stimulation_socket */
		if ((INVALID_SOCKET!=crate->scrolling_socket)&&
			(INVALID_SOCKET!=crate->calibration_socket)&&
			(INVALID_SOCKET!=crate->acquired_socket)&&
			(INVALID_SOCKET!=crate->command_socket)
#if defined (UNIX)
			&&!(crate->event_dispatcher)
#endif /* defined (UNIX) */
			)
		{
			(crate->scrolling).complete=SCROLLING_NO_CHANNELS_FOR_CRATE;
#if defined (UNIX)
#if defined (CREATE_MUTEX)
			if (crate->event_dispatcher_mutex=create_mutex())
			{
#else /* defined (CREATE_MUTEX) */
			if (0==(error_code=pthread_mutex_init(
				&(crate->event_dispatcher_mutex_storage),
				(pthread_mutexattr_t *)NULL)))
			{
				crate->event_dispatcher_mutex=
					&(crate->event_dispatcher_mutex_storage);
#endif /* defined (CREATE_MUTEX) */
				crate->event_dispatcher=event_dispatcher;
				lock_mutex(crate->event_dispatcher_mutex);
#endif /* defined (UNIX) */
#if defined (WIN32_SYSTEM)
				if (scrolling_thread=CreateThread(
					/*no security attributes*/NULL,/*use default stack size*/0,
					scrolling_thread_function,(LPVOID)crate,
					/*use default creation flags*/0,&scrolling_thread_id))
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
				if (!(crate->scrolling_socket_xid)&&(crate->scrolling_socket_xid=
					Event_dispatcher_add_simple_descriptor_callback(
					event_dispatcher,crate->scrolling_socket,
					scrolling_socket_callback,(void *)crate)))
#endif /* defined (UNIX) */
				{
#if defined (WIN32_SYSTEM)
					if (calibration_thread=CreateThread(
						/*no security attributes*/NULL,/*use default stack size*/0,
						calibration_thread_function,(LPVOID)crate,
						/*use default creation flags*/0,&calibration_thread_id))
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
					if (!(crate->calibration_socket_xid)&&(crate->calibration_socket_xid=
						Event_dispatcher_add_simple_descriptor_callback(
						event_dispatcher,crate->calibration_socket,
						calibration_socket_callback,(void *)crate)))
#endif /* defined (UNIX) */
					{
#if defined (WIN32_SYSTEM)
						if (acquired_thread=CreateThread(
							/*no security attributes*/NULL,/*use default stack size*/0,
							acquired_thread_function,(LPVOID)crate,
							/*use default creation flags*/0,&acquired_thread_id))
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
						if (!(crate->acquired_socket_xid)&&(crate->acquired_socket_xid=
							Event_dispatcher_add_simple_descriptor_callback(
							event_dispatcher,crate->acquired_socket,
							acquired_socket_callback,(void *)crate)))
#endif /* defined (UNIX) */
						{
							if ((INVALID_SOCKET==crate->stimulation_socket)||
#if defined (WIN32_SYSTEM)
								(stimulation_thread=CreateThread(
								/*no security attributes*/NULL,/*use default stack size*/0,
								stimulation_thread_function,(LPVOID)crate,
								/*use default creation flags*/0,&stimulation_thread_id))
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
								(!(crate->stimulation_socket_xid)&&
								(crate->stimulation_socket_xid=
								Event_dispatcher_add_simple_descriptor_callback(
								event_dispatcher,crate->stimulation_socket,
								stimulation_socket_callback,(void *)crate)))
#endif /* defined (UNIX) */
								)
							{
								lock_mutex(crate->command_socket_mutex);
								buffer[0]=UNEMAP_CONFIGURE_CODE;
								buffer[1]=BIG_ENDIAN_CODE;
								buffer_size=2+sizeof(message_size);
								if (4<=crate->software_version)
								{
									if (0==crate->number_of_configured_channels)
									{
										/* no channels configured */
										number_of_channels= -1;
									}
									else
									{
										number_of_channels=crate->number_of_configured_channels;
									}
									memcpy(buffer+buffer_size,&number_of_channels,
										sizeof(number_of_channels));
									buffer_size += sizeof(number_of_channels);
								}
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
								if ((0<scrolling_callback_frequency)&&
									(scrolling_callback_frequency<=scrolling_frequency)&&
									(scrolling_callback
#if defined (WIN32_USER_INTERFACE)
									||scrolling_window
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (UNIX)
									&&event_dispatcher
#endif /* defined (UNIX) */
									))
								{
									temp_scrolling_frequency=scrolling_frequency;
									temp_scrolling_callback_frequency=
										scrolling_callback_frequency;
									module_scrolling_callback=scrolling_callback;
									module_scrolling_callback_data=scrolling_callback_data;
#if defined (WIN32_USER_INTERFACE)
									module_scrolling_message=scrolling_message;
									module_scrolling_window=scrolling_window;
#endif /* defined (WIN32_USER_INTERFACE) */
								}
								else
								{
									temp_scrolling_frequency=(float)0;
									temp_scrolling_callback_frequency=(float)0;
									module_scrolling_callback=(Unemap_hardware_callback *)NULL;
									module_scrolling_callback_data=(void *)NULL;
#if defined (WIN32_USER_INTERFACE)
									module_scrolling_message=(UINT)0;
									module_scrolling_window=(HWND)NULL;
#endif /* defined (WIN32_USER_INTERFACE) */
								}
								if (3<=crate->software_version)
								{
									memcpy(buffer+buffer_size,&temp_scrolling_frequency,
										sizeof(temp_scrolling_frequency));
									buffer_size += sizeof(temp_scrolling_frequency);
								}
								memcpy(buffer+buffer_size,&temp_scrolling_callback_frequency,
									sizeof(temp_scrolling_callback_frequency));
								buffer_size += sizeof(temp_scrolling_callback_frequency);
								memcpy(buffer+buffer_size,&synchronization_card,
									sizeof(synchronization_card));
								buffer_size += sizeof(synchronization_card);
								message_size=buffer_size-(2+(long)sizeof(message_size));
								if (4<=crate->software_version)
								{
									if (0<number_of_channels)
									{
										message_size += number_of_channels*sizeof(int);
									}
								}
								memcpy(buffer+2,&message_size,sizeof(message_size));
#if defined (DEBUG)
								/*???debug */
								display_message(INFORMATION_MESSAGE,
									"crate_configure_start.  %g %g %d %g %g %d %d\n",
									sampling_frequency,sampling_frequency_slave,
									number_of_samples_in_buffer,temp_scrolling_callback_frequency,
									scrolling_callback_frequency,
									crate->number_of_configured_channels,
									crate->software_version);
#endif /* defined (DEBUG) */
								retval=socket_send(crate->command_socket,buffer,buffer_size,0);
								if (SOCKET_ERROR!=retval)
								{
#if defined (DEBUG)
									/*???debug */
									display_message(INFORMATION_MESSAGE,
										"Sent data %x %x %ld\n",buffer[0],buffer[1],message_size);
#endif /* defined (DEBUG) */
									if (4<=crate->software_version)
									{
										if (0<number_of_channels)
										{
											retval=socket_send(crate->command_socket,
												(unsigned char *)(crate->configured_channels),
												number_of_channels*sizeof(int),0);
											if (SOCKET_ERROR!=retval)
											{
												return_code=1;
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"crate_configure_start.  socket_send() failed");
												unlock_mutex(crate->command_socket_mutex);
#if defined (WIN32_SYSTEM)
												ResetEvent(crate->acquired_socket_thread_stopped_event);
												SetEvent(crate->acquired_socket_thread_stop_event);
												ResetEvent(
													crate->calibration_socket_thread_stopped_event);
												SetEvent(crate->calibration_socket_thread_stop_event);
												ResetEvent(
													crate->scrolling_socket_thread_stopped_event);
												SetEvent(crate->scrolling_socket_thread_stop_event);
												if (INVALID_SOCKET!=crate->stimulation_socket)
												{
													ResetEvent(
														crate->stimulation_socket_thread_stopped_event);
													SetEvent(crate->stimulation_socket_thread_stop_event);
												}
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
												Event_dispatcher_remove_descriptor_callback(
													crate->event_dispatcher,crate->acquired_socket_xid);
												crate->acquired_socket_xid=
													(struct Event_dispatcher_descriptor_callback *)NULL;
												Event_dispatcher_remove_descriptor_callback(
													crate->event_dispatcher,
													crate->calibration_socket_xid);
												crate->calibration_socket_xid=
													(struct Event_dispatcher_descriptor_callback *)NULL;
												Event_dispatcher_remove_descriptor_callback(
													crate->event_dispatcher,crate->scrolling_socket_xid);
												crate->scrolling_socket_xid=
													(struct Event_dispatcher_descriptor_callback *)NULL;
												if (INVALID_SOCKET!=crate->stimulation_socket)
												{
													Event_dispatcher_remove_descriptor_callback(
														crate->event_dispatcher,
														crate->stimulation_socket_xid);
													crate->stimulation_socket_xid=
														(struct Event_dispatcher_descriptor_callback *)NULL;
												}
#endif /* defined (UNIX) */
#if defined (UNIX)
												unlock_mutex(crate->event_dispatcher_mutex);
#if defined (CREATE_MUTEX)
												destroy_mutex(&(crate->event_dispatcher_mutex));
#else /* defined (CREATE_MUTEX) */
												pthread_mutex_destroy(crate->event_dispatcher_mutex);
												crate->event_dispatcher_mutex=(pthread_mutex_t *)NULL;
#endif /* defined (CREATE_MUTEX) */
												crate->event_dispatcher=(struct Event_dispatcher *)NULL;
#endif /* defined (UNIX) */
											}
										}
										else
										{
											return_code=1;
										}
									}
									else
									{
										return_code=1;
									}
#if defined (UNIX)
									unlock_mutex(crate->event_dispatcher_mutex);
#endif /* defined (UNIX) */
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"crate_configure_start.  socket_send() failed");
									unlock_mutex(crate->command_socket_mutex);
#if defined (WIN32_SYSTEM)
									ResetEvent(crate->acquired_socket_thread_stopped_event);
									SetEvent(crate->acquired_socket_thread_stop_event);
									ResetEvent(crate->calibration_socket_thread_stopped_event);
									SetEvent(crate->calibration_socket_thread_stop_event);
									ResetEvent(crate->scrolling_socket_thread_stopped_event);
									SetEvent(crate->scrolling_socket_thread_stop_event);
									if (INVALID_SOCKET!=crate->stimulation_socket)
									{
										ResetEvent(crate->stimulation_socket_thread_stopped_event);
										SetEvent(crate->stimulation_socket_thread_stop_event);
									}
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
									Event_dispatcher_remove_descriptor_callback(
										crate->event_dispatcher,crate->acquired_socket_xid);
									crate->acquired_socket_xid=
										(struct Event_dispatcher_descriptor_callback *)NULL;
									Event_dispatcher_remove_descriptor_callback(
										crate->event_dispatcher,crate->calibration_socket_xid);
									crate->calibration_socket_xid=
										(struct Event_dispatcher_descriptor_callback *)NULL;
									Event_dispatcher_remove_descriptor_callback(
										crate->event_dispatcher,crate->scrolling_socket_xid);
									crate->scrolling_socket_xid=
										(struct Event_dispatcher_descriptor_callback *)NULL;
									if (INVALID_SOCKET!=crate->stimulation_socket)
									{
										Event_dispatcher_remove_descriptor_callback(
											crate->event_dispatcher,crate->stimulation_socket_xid);
										crate->stimulation_socket_xid=
											(struct Event_dispatcher_descriptor_callback *)NULL;
									}
#endif /* defined (UNIX) */
#if defined (UNIX)
									unlock_mutex(crate->event_dispatcher_mutex);
#if defined (CREATE_MUTEX)
									destroy_mutex(&(crate->event_dispatcher_mutex));
#else /* defined (CREATE_MUTEX) */
									pthread_mutex_destroy(crate->event_dispatcher_mutex);
									crate->event_dispatcher_mutex=(pthread_mutex_t *)NULL;
#endif /* defined (CREATE_MUTEX) */
									crate->event_dispatcher=(struct Event_dispatcher *)NULL;
#endif /* defined (UNIX) */
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"crate_configure_start.  "
#if defined (WIN32_SYSTEM)
									"CreateThread failed for stimulation socket");
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
									"Event_dispatcher_add_simple_descriptor_callback failed for "
									"stimulation socket.  %p",crate->stimulation_socket_xid);
#endif /* defined (UNIX) */
#if defined (WIN32_SYSTEM)
								ResetEvent(crate->acquired_socket_thread_stopped_event);
								SetEvent(crate->acquired_socket_thread_stop_event);
								ResetEvent(crate->calibration_socket_thread_stopped_event);
								SetEvent(crate->calibration_socket_thread_stop_event);
								ResetEvent(crate->scrolling_socket_thread_stopped_event);
								SetEvent(crate->scrolling_socket_thread_stop_event);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
								Event_dispatcher_remove_descriptor_callback(
									crate->event_dispatcher,crate->acquired_socket_xid);
								crate->acquired_socket_xid=
									(struct Event_dispatcher_descriptor_callback *)NULL;
								Event_dispatcher_remove_descriptor_callback(
									crate->event_dispatcher,crate->calibration_socket_xid);
								crate->calibration_socket_xid=
									(struct Event_dispatcher_descriptor_callback *)NULL;
								Event_dispatcher_remove_descriptor_callback(
									crate->event_dispatcher,crate->scrolling_socket_xid);
								crate->scrolling_socket_xid=
									(struct Event_dispatcher_descriptor_callback *)NULL;
#endif /* defined (UNIX) */
#if defined (UNIX)
								unlock_mutex(crate->event_dispatcher_mutex);
#if defined (CREATE_MUTEX)
								destroy_mutex(&(crate->event_dispatcher_mutex));
#else /* defined (CREATE_MUTEX) */
								pthread_mutex_destroy(crate->event_dispatcher_mutex);
								crate->event_dispatcher_mutex=(pthread_mutex_t *)NULL;
#endif /* defined (CREATE_MUTEX) */
								crate->event_dispatcher=(struct Event_dispatcher *)NULL;
#endif /* defined (UNIX) */
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"crate_configure_start.  "
#if defined (WIN32_SYSTEM)
								"CreateThread failed for acquired socket");
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
								"Event_dispatcher_add_simple_descriptor_callback failed for "
								"acquired socket.  %p",crate->acquired_socket_xid);
#endif /* defined (UNIX) */
#if defined (WIN32_SYSTEM)
							ResetEvent(crate->calibration_socket_thread_stopped_event);
							SetEvent(crate->calibration_socket_thread_stop_event);
							ResetEvent(crate->scrolling_socket_thread_stopped_event);
							SetEvent(crate->scrolling_socket_thread_stop_event);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
							Event_dispatcher_remove_descriptor_callback(
								crate->event_dispatcher,crate->calibration_socket_xid);
							crate->calibration_socket_xid=
								(struct Event_dispatcher_descriptor_callback *)NULL;
							Event_dispatcher_remove_descriptor_callback(
								crate->event_dispatcher,crate->scrolling_socket_xid);
							crate->scrolling_socket_xid=
								(struct Event_dispatcher_descriptor_callback *)NULL;
#endif /* defined (UNIX) */
#if defined (UNIX)
							unlock_mutex(crate->event_dispatcher_mutex);
#if defined (CREATE_MUTEX)
							destroy_mutex(&(crate->event_dispatcher_mutex));
#else /* defined (CREATE_MUTEX) */
							pthread_mutex_destroy(crate->event_dispatcher_mutex);
							crate->event_dispatcher_mutex=(pthread_mutex_t *)NULL;
#endif /* defined (CREATE_MUTEX) */
							crate->event_dispatcher=(struct Event_dispatcher *)NULL;
#endif /* defined (UNIX) */
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"crate_configure_start.  "
#if defined (WIN32_SYSTEM)
							"CreateThread failed for calibration socket");
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
							"Event_dispatcher_add_simple_descriptor_callback failed for "
							"calibration socket.  %p",crate->calibration_socket_xid);
#endif /* defined (UNIX) */
#if defined (WIN32_SYSTEM)
						ResetEvent(crate->scrolling_socket_thread_stopped_event);
						SetEvent(crate->scrolling_socket_thread_stop_event);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
						Event_dispatcher_remove_descriptor_callback(
							crate->event_dispatcher,crate->scrolling_socket_xid);
						crate->scrolling_socket_xid=
							(struct Event_dispatcher_descriptor_callback *)NULL;
#endif /* defined (UNIX) */
#if defined (UNIX)
						unlock_mutex(crate->event_dispatcher_mutex);
#if defined (CREATE_MUTEX)
						destroy_mutex(&(crate->event_dispatcher_mutex));
#else /* defined (CREATE_MUTEX) */
						pthread_mutex_destroy(crate->event_dispatcher_mutex);
						crate->event_dispatcher_mutex=(pthread_mutex_t *)NULL;
#endif /* defined (CREATE_MUTEX) */
						crate->event_dispatcher=(struct Event_dispatcher *)NULL;
#endif /* defined (UNIX) */
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"crate_configure_start.  "
#if defined (WIN32_SYSTEM)
						"CreateThread failed for scrolling socket");
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
						"Event_dispatcher_add_simple_descriptor_callback failed for scrolling "
						"socket.  %p",crate->scrolling_socket_xid);
#endif /* defined (UNIX) */
#if defined (UNIX)
					unlock_mutex(crate->event_dispatcher_mutex);
#if defined (CREATE_MUTEX)
					destroy_mutex(&(crate->event_dispatcher_mutex));
#else /* defined (CREATE_MUTEX) */
					pthread_mutex_destroy(crate->event_dispatcher_mutex);
					crate->event_dispatcher_mutex=(pthread_mutex_t *)NULL;
#endif /* defined (CREATE_MUTEX) */
					crate->event_dispatcher=(struct Event_dispatcher *)NULL;
#endif /* defined (UNIX) */
				}
#if defined (UNIX)
#if defined (CREATE_MUTEX)
			}
			else
			{
				display_message(ERROR_MESSAGE,"crate_configure_start.  "
					"create_mutex failed for event_dispatcher_mutex");
			}
#else /* defined (CREATE_MUTEX) */
			}
			else
			{
				display_message(ERROR_MESSAGE,"crate_configure_start.  "
					"pthread_mutex_init failed for event_dispatcher_mutex.  "
					"Error code %d",error_code);
			}
#endif /* defined (CREATE_MUTEX) */
#endif /* defined (UNIX) */
		}
		else
		{
			display_message(ERROR_MESSAGE,"crate_configure_start.  "
				"Invalid command_socket (%p) or calibration_socket (%p) or "
				"scrolling_socket (%p) or acquired_socket (%p)"
#if defined (UNIX)
				" or event_dispatcher (%p)"
#endif /* defined (UNIX) */
				,crate->command_socket,crate->calibration_socket,
				crate->scrolling_socket,crate->acquired_socket
#if defined (UNIX)
				,crate->event_dispatcher
#endif /* defined (UNIX) */
				);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_configure_start.  Invalid argument(s).  %p %g %d"
#if defined (UNIX)
			" %p"
#endif /* defined (UNIX) */
			,crate,sampling_frequency,number_of_samples_in_buffer
#if defined (UNIX)
			,event_dispatcher
#endif /* defined (UNIX) */
			);
		return_code=0;
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave crate_configure_start\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_configure_start */

static int crate_configure_end(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code,retval;
	long buffer_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(crate_configure_end);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter crate_configure_end\n");
#endif /* defined (DEBUG) */
	return_code=0;
	/* check argument */
	if (crate)
	{
		/* get the header back */
		retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
			(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
			buffer,2+sizeof(long),0);
		if (SOCKET_ERROR!=retval)
		{
			memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,
				"Received %d (%d) bytes, data %x %x %ld\n",retval,2+sizeof(long),
				buffer[0],buffer[1],buffer_size);
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
		unlock_mutex(crate->command_socket_mutex);
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_configure_end.  Invalid argument");
		return_code=0;
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave crate_configure_end\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_configure_end */

static int crate_configured(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

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
		lock_mutex(crate->command_socket_mutex);
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
			retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
		unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 13 August 2003

DESCRIPTION :
Stops acquisition and signal generation for the <crate>.  Frees buffers
associated with the hardware.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];
#if defined (WIN32_SYSTEM)
	DWORD status;
	HANDLE hEvents[4];
	int number_of_events;
#endif /* defined (WIN32_SYSTEM) */

	ENTER(crate_deconfigure);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter crate_deconfigure %p.  "
		"command %d.  calibration %d.  acquired %d.  scrolling %d"
		".  stimulation %d\n",crate,crate->command_socket,crate->calibration_socket,
		crate->acquired_socket,crate->scrolling_socket,crate->stimulation_socket);
#endif /* defined (DEBUG) */
	return_code=0;
	if (crate)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			lock_mutex(crate->command_socket_mutex);
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
				retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
					(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
					buffer,2+sizeof(long),0);
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
			unlock_mutex(crate->command_socket_mutex);
		}
#if defined (WIN32_SYSTEM)
		number_of_events=0;
#if defined (DEBUG)
		/*???debug */
		display_message(INFORMATION_MESSAGE,"scrolling thread stop event %p\n",
			crate->scrolling_socket_thread_stop_event);
#endif /* defined (DEBUG) */
		if (crate->scrolling_socket_thread_stop_event)
		{
			hEvents[number_of_events]=crate->scrolling_socket_thread_stopped_event;
			number_of_events++;
			ResetEvent(crate->scrolling_socket_thread_stopped_event);
			SetEvent(crate->scrolling_socket_thread_stop_event);
		}
#if defined (DEBUG)
		/*???debug */
		display_message(INFORMATION_MESSAGE,"calibration thread stop event %p\n",
			crate->calibration_socket_thread_stop_event);
#endif /* defined (DEBUG) */
		if (crate->calibration_socket_thread_stop_event)
		{
			hEvents[number_of_events]=crate->calibration_socket_thread_stopped_event;
			number_of_events++;
			ResetEvent(crate->calibration_socket_thread_stopped_event);
			SetEvent(crate->calibration_socket_thread_stop_event);
		}
#if defined (DEBUG)
		/*???debug */
		display_message(INFORMATION_MESSAGE,"acquired thread stop event %p\n",
			crate->acquired_socket_thread_stop_event);
#endif /* defined (DEBUG) */
		if (crate->acquired_socket_thread_stop_event)
		{
			hEvents[number_of_events]=crate->acquired_socket_thread_stopped_event;
			number_of_events++;
			ResetEvent(crate->acquired_socket_thread_stopped_event);
			SetEvent(crate->acquired_socket_thread_stop_event);
		}
#if defined (DEBUG)
		/*???debug */
		display_message(INFORMATION_MESSAGE,"stimulation thread stop event %p\n",
			crate->stimulation_socket_thread_stop_event);
#endif /* defined (DEBUG) */
		if (crate->stimulation_socket_thread_stop_event)
		{
			hEvents[number_of_events]=crate->stimulation_socket_thread_stopped_event;
			number_of_events++;
			ResetEvent(crate->stimulation_socket_thread_stopped_event);
			SetEvent(crate->stimulation_socket_thread_stop_event);
		}
		/* wait for threads to end before deallocating crate memory */
#if defined (DEBUG)
		/*???debug */
		display_message(INFORMATION_MESSAGE,
			"before WaitForMultipleObjects.  %d\n",number_of_events);
#endif /* defined (DEBUG) */
		if (number_of_events>0)
		{
			status=WaitForMultipleObjects(number_of_events,hEvents,TRUE,INFINITE);
		}
#if defined (DEBUG)
		/*???debug */
		display_message(INFORMATION_MESSAGE,
			"after WaitForMultipleObjects.  %p %p\n",status,WAIT_FAILED);
#endif /* defined (DEBUG) */
		if (crate->scrolling_socket_thread_stopped_event)
		{
			CloseHandle(crate->scrolling_socket_thread_stopped_event);
			crate->scrolling_socket_thread_stopped_event=NULL;
		}
		if (crate->calibration_socket_thread_stopped_event)
		{
			CloseHandle(crate->calibration_socket_thread_stopped_event);
			crate->calibration_socket_thread_stopped_event=NULL;
		}
		if (crate->acquired_socket_thread_stopped_event)
		{
			CloseHandle(crate->acquired_socket_thread_stopped_event);
			crate->acquired_socket_thread_stopped_event=NULL;
		}
		if (crate->stimulation_socket_thread_stopped_event)
		{
			CloseHandle(crate->stimulation_socket_thread_stopped_event);
			crate->stimulation_socket_thread_stopped_event=NULL;
		}
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
		if ((crate->event_dispatcher)&&(crate->event_dispatcher_mutex))
		{
			if (crate->scrolling_socket_xid)
			{
				lock_mutex(crate->event_dispatcher_mutex);
				Event_dispatcher_remove_descriptor_callback(crate->event_dispatcher,
					crate->scrolling_socket_xid);
				unlock_mutex(crate->event_dispatcher_mutex);
				crate->scrolling_socket_xid=
					(struct Event_dispatcher_descriptor_callback *)NULL;
			}
			if (crate->calibration_socket_xid)
			{
				lock_mutex(crate->event_dispatcher_mutex);
				Event_dispatcher_remove_descriptor_callback(crate->event_dispatcher,
					crate->calibration_socket_xid);
				unlock_mutex(crate->event_dispatcher_mutex);
				crate->calibration_socket_xid=
					(struct Event_dispatcher_descriptor_callback *)NULL;
			}
			if (crate->acquired_socket_xid)
			{
				lock_mutex(crate->event_dispatcher_mutex);
				Event_dispatcher_remove_descriptor_callback(crate->event_dispatcher,
					crate->acquired_socket_xid);
				unlock_mutex(crate->event_dispatcher_mutex);
				crate->acquired_socket_xid=
					(struct Event_dispatcher_descriptor_callback *)NULL;
			}
			if (crate->stimulation_socket_xid)
			{
				lock_mutex(crate->event_dispatcher_mutex);
				Event_dispatcher_remove_descriptor_callback(crate->event_dispatcher,
					crate->stimulation_socket_xid);
				unlock_mutex(crate->event_dispatcher_mutex);
				crate->stimulation_socket_xid=
					(struct Event_dispatcher_descriptor_callback *)NULL;
			}
#if defined (CREATE_MUTEX)
			destroy_mutex(&(crate->event_dispatcher_mutex));
#else /* defined (CREATE_MUTEX) */
			pthread_mutex_destroy(crate->event_dispatcher_mutex);
#endif /* defined (CREATE_MUTEX) */
		}
		crate->event_dispatcher_mutex=(pthread_mutex_t *)NULL;
		crate->event_dispatcher=(struct Event_dispatcher *)NULL;
#endif /* defined (UNIX) */
		DEALLOCATE(crate->configured_channels);
		DEALLOCATE(crate->configured_channel_offsets);
		crate->number_of_configured_channels=0;
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
	display_message(INFORMATION_MESSAGE,"leave crate_deconfigure %d\n",
		return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_deconfigure */

static int crate_get_hardware_version(struct Unemap_crate *crate,
	int *hardware_version)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

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
	display_message(INFORMATION_MESSAGE,
		"enter crate_get_hardware_version %p %p\n",crate,hardware_version);
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if (crate&&hardware_version)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			lock_mutex(crate->command_socket_mutex);
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
				retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
					(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
					buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
							(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
							buffer,sizeof(int),0))
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
			unlock_mutex(crate->command_socket_mutex);
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
	display_message(INFORMATION_MESSAGE,"leave crate_get_hardware_version %d\n",
		return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_get_hardware_version */

static int crate_shutdown(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

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
	display_message(INFORMATION_MESSAGE,"enter crate_shutdown %p\n",crate);
#endif /* defined (DEBUG) */
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		lock_mutex(crate->command_socket_mutex);
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
			retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
					display_message(INFORMATION_MESSAGE,"before recv\n");
					return_code=recv(crate->command_socket,buffer,1,0);
					display_message(INFORMATION_MESSAGE,"after recv %d\n",return_code);
				}
				while ((SOCKET_ERROR==return_code)&&
#if defined (WIN32_SYSTEM)
					(WSAEWOULDBLOCK==WSAGetLastError())
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
					(EWOULDBLOCK==errno)
#endif /* defined (UNIX) */
					);
			}
			while ((SOCKET_ERROR!=return_code)&&(0!=return_code));
		}
#endif /* defined (OLD_CODE) */
		unlock_mutex(crate->command_socket_mutex);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_shutdown.  Missing crate (%p) or invalid command_socket",crate);
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave crate_shutdown %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_shutdown */

static int crate_set_scrolling_channel(struct Unemap_crate *crate,
	int channel_number)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

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
		lock_mutex(crate->command_socket_mutex);
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
			retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
		unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 5 August 2002

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
		lock_mutex(crate->command_socket_mutex);
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
			retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
		unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 5 August 2002

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
		lock_mutex(crate->command_socket_mutex);
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
			retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
		unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 5 August 2002

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
		lock_mutex(crate->command_socket_mutex);
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
			retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
		unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 5 August 2002

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
		lock_mutex(crate->command_socket_mutex);
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
			retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
		unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 5 August 2002

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
		lock_mutex(crate->command_socket_mutex);
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
			retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
		unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 5 August 2002

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
		lock_mutex(crate->command_socket_mutex);
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
			return_code=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
		unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 27 July 2000

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
	display_message(INFORMATION_MESSAGE,
		"enter crate_set_isolate_record_mode_start %p\n",crate);
#endif /* defined (DEBUG) */
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		lock_mutex(crate->command_socket_mutex);
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
		display_message(ERROR_MESSAGE,"crate_set_isolate_record_mode_start.  "
			"Missing crate (%p) or invalid command_socket",crate);
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave crate_set_isolate_record_mode_start %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_set_isolate_record_mode_start */

static int crate_set_isolate_record_mode_end(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

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
	display_message(INFORMATION_MESSAGE,
		"enter crate_set_isolate_record_mode_end %p\n",crate);
#endif /* defined (DEBUG) */
	return_code=0;
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		/* get the header back */
		retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
			(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
			buffer,2+sizeof(long),0);
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
		unlock_mutex(crate->command_socket_mutex);
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_set_isolate_record_mode_end.  "
			"Missing crate (%p) or invalid command_socket",crate);
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave crate_set_isolate_record_mode_end %d\n",return_code);
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
	display_message(INFORMATION_MESSAGE,
		"enter crate_set_isolate_record_mode %p\n",crate);
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
	display_message(INFORMATION_MESSAGE,
		"leave crate_set_isolate_record_mode %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_set_isolate_record_mode */
#endif /* defined (OLD_CODE) */

static int crate_get_isolate_record_mode(struct Unemap_crate *crate,
	int channel_number,int *isolate)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

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
			lock_mutex(crate->command_socket_mutex);
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
				retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
					(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
					buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
							(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
							buffer,sizeof(int),0))
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
			unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 5 August 2002

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
		lock_mutex(crate->command_socket_mutex);
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
			retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
		unlock_mutex(crate->command_socket_mutex);
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_set_antialiasing_filter_frequency.  "
			"Missing crate (%p) or invalid command_socket",crate);
	}
	LEAVE;

	return (return_code);
} /* crate_set_antialiasing_filter_frequency */

static int crate_set_powerup_antialiasing_filter_frequency(
	struct Unemap_crate *crate,int channel_number)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

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
		lock_mutex(crate->command_socket_mutex);
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
			retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
					"crate_set_powerup_antialiasing_filter_frequency.  "
					"socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_set_powerup_antialiasing_filter_frequency.  "
				"socket_send() failed");
		}
		unlock_mutex(crate->command_socket_mutex);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_set_powerup_antialiasing_filter_frequency.  "
			"Missing crate (%p) or invalid command_socket",crate);
	}
	LEAVE;

	return (return_code);
} /* crate_set_powerup_antialiasing_filter_frequency */

static int crate_get_antialiasing_filter_frequency(struct Unemap_crate *crate,
	int channel_number,float *frequency)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

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
			lock_mutex(crate->command_socket_mutex);
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
				retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
					(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
					buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
							(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
							buffer,sizeof(float),0))
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
			unlock_mutex(crate->command_socket_mutex);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_antialiasing_filter_frequency.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_get_antialiasing_filter_frequency.  "
			"Missing crate (%p) or frequency (%p)",crate,frequency);
	}
	LEAVE;

	return (return_code);
} /* crate_get_antialiasing_filter_frequency */

static int crate_get_number_of_channels(struct Unemap_crate *crate,
	int *number_of_channels)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

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
			lock_mutex(crate->command_socket_mutex);
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
				retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
					(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
					buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
							(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
							buffer,sizeof(int),0))
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
			unlock_mutex(crate->command_socket_mutex);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_number_of_channels.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_get_number_of_channels.  "
			"Missing crate (%p) or number_of_channels (%p)",crate,number_of_channels);
	}
	LEAVE;

	return (return_code);
} /* crate_get_number_of_channels */

static int crate_get_sample_range(struct Unemap_crate *crate,int channel_number,
	long int *minimum_sample_value,long int *maximum_sample_value)
/*******************************************************************************
LAST MODIFIED : 2 June 2003

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels for the
<crate> inclusive, then the function applies to the group
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.
Otherwise, the function fails.

The sample range is returned via <*minimum_sample_value> and
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
			lock_mutex(crate->command_socket_mutex);
			buffer[0]=UNEMAP_GET_SAMPLE_RANGE_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			if (crate->software_version>1)
			{
				memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
				buffer_size += sizeof(channel_number);
			}
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
				retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
					(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
					buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
							(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
							buffer,2*sizeof(long),0))
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
			unlock_mutex(crate->command_socket_mutex);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_sample_range.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_get_sample_range.  Missing crate (%p)"
			"or minimum_sample_value (%p) or maximum_sample_value (%p)",
			crate,minimum_sample_value,maximum_sample_value);
	}
	LEAVE;

	return (return_code);
} /* crate_get_sample_range */

static int crate_get_voltage_range_start(struct Unemap_crate *crate,
	int channel_number)
/*******************************************************************************
LAST MODIFIED : 27 July 2000

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
			lock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 5 August 2002

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
			retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
					if (SOCKET_ERROR!=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
						(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
						buffer,2*sizeof(float),0))
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
			unlock_mutex(crate->command_socket_mutex);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_voltage_range_end.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_get_voltage_range_end.  "
			"Missing crate (%p) or minimum_voltage (%p) or maximum_voltage (%p)",
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
LAST MODIFIED : 5 August 2002

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
			lock_mutex(crate->command_socket_mutex);
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
				retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
					(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
					buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
							(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
							buffer,sizeof(long),0))
						{
							memcpy(number_of_samples,buffer,sizeof(long));
#if defined (DEBUG)
							/*???debug */
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
			unlock_mutex(crate->command_socket_mutex);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_number_of_samples_acquired.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_get_number_of_samples_acquired.  "
			"Missing crate (%p) or number_of_samples (%p)",crate,number_of_samples);
	}
	LEAVE;

	return (return_code);
} /* crate_get_number_of_samples_acquired */

static int crate_get_samples_acquired(struct Unemap_crate *crate,
	int channel_number,int number_of_samples,short int *samples,
	int *number_of_samples_got)
/*******************************************************************************
LAST MODIFIED : 13 August 2003

DESCRIPTION :
The function fails if the hardware is not configured.

For the <crate>.  If <channel_number> is valid (between 1 and the total number
of channels inclusive), then the <samples> for that channel are returned.  If
<channel_number> is 0 then the <samples> for all channels are returned.
Otherwise the function fails.

<samples> is for all crates.
==============================================================================*/
{
	int k,number_of_channels,return_code,retval;
	long block_size,buffer_size,message_size;
	short int *crate_samples,*module_samples;
	unsigned char buffer[2+sizeof(long)+sizeof(int)+sizeof(unsigned long)];
	unsigned long local_number_of_samples;

	ENTER(crate_get_samples_acquired);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"enter crate_get_samples_acquired %p %d %p\n",crate,channel_number,
		samples);
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if (crate&&samples)
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			lock_mutex(crate->command_socket_mutex);
			buffer[0]=UNEMAP_GET_SAMPLES_ACQUIRED_CODE;
			buffer[1]=BIG_ENDIAN_CODE;
			buffer_size=2+sizeof(message_size);
			memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
			buffer_size += sizeof(channel_number);
			if (crate->software_version>0)
			{
				memcpy(buffer+buffer_size,&number_of_samples,sizeof(number_of_samples));
				buffer_size += sizeof(number_of_samples);
			}
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
				/* get the header/acknowledgement back */
				retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
					(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
					buffer,2+sizeof(long),0);
				if (SOCKET_ERROR!=retval)
				{
					memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"Received %d bytes, data %x %x %ld\n",retval,buffer[0],
						buffer[1],buffer_size);
#endif /* defined (DEBUG) */
					/*???DB.  Go back to reading back data through command socket because
						have a separate thread watching the acquired socket and don't want
						it activated for this */
					if (crate->software_version>0)
					{
						if ((2+sizeof(long)==retval)&&(0==buffer_size)&&(buffer[0]))
						{
							/* read back from command socket */
							/* get the header back */
							retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
								(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
								buffer,2+sizeof(long)+sizeof(number_of_channels)+
								sizeof(local_number_of_samples),0);
							if (SOCKET_ERROR!=retval)
							{
								memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
								/*???debug */
								display_message(INFORMATION_MESSAGE,
									"Received command %d bytes, data %x %x %ld\n",retval,
									buffer[0],buffer[1],buffer_size);
#endif /* defined (DEBUG) */
								memcpy(&number_of_channels,buffer+2+sizeof(buffer_size),
									sizeof(number_of_channels));
								memcpy(&local_number_of_samples,buffer+2+sizeof(buffer_size)+
									sizeof(number_of_channels),sizeof(local_number_of_samples));
#if defined (DEBUG)
								/*???debug */
								display_message(INFORMATION_MESSAGE,
									"number_of_channels=%d, local_number_of_samples=%ld.  "
									"%ld\n",number_of_channels,local_number_of_samples,
									sizeof(number_of_channels)+sizeof(local_number_of_samples)+
									local_number_of_samples*number_of_channels*
									sizeof(short int));
#endif /* defined (DEBUG) */
								if ((2+sizeof(long)+sizeof(number_of_channels)+
									sizeof(local_number_of_samples)==retval)&&
									(sizeof(number_of_channels)+sizeof(local_number_of_samples)+
									local_number_of_samples*number_of_channels*
									sizeof(short int)==(unsigned)buffer_size)&&(buffer[0]))
								{
									buffer_size -= sizeof(number_of_channels)+
										sizeof(local_number_of_samples);
									if (0==channel_number)
									{
										if (0<buffer_size)
										{
											if (4<=crate->software_version)
											{
												block_size=crate->number_of_configured_channels;
											}
											else
											{
												block_size=crate->number_of_channels;
											}
											if ((0<block_size)&&ALLOCATE(crate_samples,short int,
												block_size))
											{
												block_size *= (long)sizeof(short int);
												module_samples=samples;
												while ((buffer_size>0)&&(SOCKET_ERROR!=socket_recv(
													crate->command_socket,
#if defined (WIN32_SYSTEM)
													(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
													(unsigned char *)crate_samples,block_size,0)))
												{
													/* have to stagger to allow room for other crates */
													if (4<=crate->software_version)
													{
														for (k=0;k<crate->number_of_configured_channels;k++)
														{
															module_samples[
																(crate->configured_channel_offsets)[k]]=
																crate_samples[k];
														}
													}
													else
													{
														for (k=0;k<crate->number_of_configured_channels;k++)
														{
															module_samples[
																(crate->configured_channel_offsets)[k]]=
																crate_samples[
																(crate->configured_channels)[k]-1];
														}
													}
													module_samples +=
														module_number_of_configured_channels;
													buffer_size -= block_size;
												}
												DEALLOCATE(crate_samples);
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"crate_get_samples_acquired.  "
													"Could not allocate crate_samples 1");
											}
										}
										if (0==buffer_size)
										{
											return_code=1;
										}
									}
									else
									{
										if (SOCKET_ERROR!=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
											(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
											(unsigned char *)samples,buffer_size,0))
										{
											return_code=1;
										}
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"crate_get_samples_acquired.  socket_recv() 1 failed");
							}
							/* get the header back */
							retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
								(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
								buffer,2+sizeof(long),0);
							if (SOCKET_ERROR!=retval)
							{
								memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
								if (!((2+sizeof(long)==retval)&&(0==buffer_size)&&
									(buffer[0])))
								{
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"crate_get_samples_acquired.  socket_recv() 2 failed");
							}
						}
					}
					else
					{
						if ((2+sizeof(long)==retval)&&(0<=buffer_size)&&(buffer[0]))
						{
							if (0==buffer_size)
							{
								/* read back from acquired socket */
								lock_mutex(crate->acquired_socket_mutex);
								/* get the header back */
								retval=socket_recv(crate->acquired_socket,
#if defined (WIN32_SYSTEM)
									(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
									buffer,2+sizeof(long)+sizeof(number_of_channels)+
									sizeof(local_number_of_samples),0);
								if (SOCKET_ERROR!=retval)
								{
									memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
#if defined (DEBUG)
									/*???debug */
									display_message(INFORMATION_MESSAGE,
										"Received acquired %d bytes, data %x %x %ld\n",retval,
										buffer[0],buffer[1],buffer_size);
#endif /* defined (DEBUG) */
									memcpy(&number_of_channels,buffer+2+sizeof(buffer_size),
										sizeof(number_of_channels));
									memcpy(&local_number_of_samples,buffer+2+sizeof(buffer_size)+
										sizeof(number_of_channels),sizeof(local_number_of_samples));
#if defined (DEBUG)
									/*???debug */
									display_message(INFORMATION_MESSAGE,
										"number_of_channels=%d, local_number_of_samples=%ld.  "
										"%ld\n",number_of_channels,local_number_of_samples,
										sizeof(number_of_channels)+sizeof(local_number_of_samples)+
										local_number_of_samples*number_of_channels*
										sizeof(short int));
#endif /* defined (DEBUG) */
									if ((2+sizeof(long)+sizeof(number_of_channels)+
										sizeof(local_number_of_samples)==retval)&&
										(sizeof(number_of_channels)+sizeof(local_number_of_samples)+
										local_number_of_samples*number_of_channels*
										sizeof(short int)==(unsigned)buffer_size)&&(buffer[0]))
									{
										buffer_size -= sizeof(number_of_channels)+
											sizeof(local_number_of_samples);
										if (0==channel_number)
										{
											if (0<buffer_size)
											{
												block_size=crate->number_of_channels;
												if ((0<block_size)&&ALLOCATE(crate_samples,short int,
													block_size))
												{
													block_size *= (long)sizeof(short int);
													module_samples=samples;
													while ((buffer_size>0)&&(SOCKET_ERROR!=socket_recv(
														crate->acquired_socket,
#if defined (WIN32_SYSTEM)
														(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
														(unsigned char *)crate_samples,block_size,0)))
													{
														/* have to stagger to allow room for other crates */
														for (k=0;k<crate->number_of_configured_channels;k++)
														{
															module_samples[
																(crate->configured_channel_offsets)[k]]=
																crate_samples[
																(crate->configured_channels)[k]-1];
														}
														module_samples +=
															module_number_of_configured_channels;
														buffer_size -= block_size;
													}
													DEALLOCATE(crate_samples);
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"crate_get_samples_acquired.  "
														"Could not allocate crate_samples 2");
												}
											}
											if (0==buffer_size)
											{
												return_code=1;
											}
										}
										else
										{
											if (SOCKET_ERROR!=socket_recv(crate->acquired_socket,
#if defined (WIN32_SYSTEM)
												(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
												(unsigned char *)samples,buffer_size,0))
											{
												return_code=1;
											}
										}
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"crate_get_samples_acquired.  socket_recv() 3 failed");
								}
								unlock_mutex(crate->acquired_socket_mutex);
								/* get the header back */
								retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
									(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
									buffer,2+sizeof(long),0);
								if (SOCKET_ERROR!=retval)
								{
									memcpy(&buffer_size,buffer+2,sizeof(buffer_size));
									if (!((2+sizeof(long)==retval)&&(0==buffer_size)&&
										(buffer[0])))
									{
										return_code=0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"crate_get_samples_acquired.  socket_recv() 4 failed");
								}
							}
							else
							{
								/* read back from command socket (kept for compatability with
									older versions of the hardware service) */
								if (0==channel_number)
								{
									if (0<buffer_size)
									{
										block_size=crate->number_of_channels;
										if ((0<block_size)&&ALLOCATE(crate_samples,short int,
											block_size))
										{
											block_size *= (long)sizeof(short int);
											module_samples=samples;
											while ((buffer_size>0)&&(SOCKET_ERROR!=socket_recv(
												crate->command_socket,
#if defined (WIN32_SYSTEM)
												(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
												(unsigned char *)crate_samples,block_size,0)))
											{
												/* have to stagger to allow room for other crates */
												for (k=0;k<crate->number_of_configured_channels;k++)
												{
													module_samples[
														(crate->configured_channel_offsets)[k]]=
														crate_samples[
														(crate->configured_channels)[k]-1];
												}
												module_samples +=
													module_number_of_configured_channels;
												buffer_size -= block_size;
											}
											DEALLOCATE(crate_samples);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"crate_get_samples_acquired.  "
												"Could not allocate crate_samples 3");
										}
									}
									if (0==buffer_size)
									{
										local_number_of_samples=buffer_size/block_size;
										return_code=1;
									}
								}
								else
								{
									if (SOCKET_ERROR!=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
										(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
										(unsigned char *)samples,buffer_size,0))
									{
										local_number_of_samples=buffer_size/(long)sizeof(short int);
										return_code=1;
									}
								}
							}
						}
					}
					if (return_code&&number_of_samples_got)
					{
						*number_of_samples_got=(int)local_number_of_samples;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_get_samples_acquired.  socket_recv() 5 failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_samples_acquired.  socket_send() failed");
			}
			unlock_mutex(crate->command_socket_mutex);
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
	display_message(INFORMATION_MESSAGE,"leave crate_get_samples_acquired %d\n",
		return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_get_samples_acquired */

static int crate_get_samples_acquired_background_start(
	struct Unemap_crate *crate,int channel_number,int number_of_samples)
/*******************************************************************************
LAST MODIFIED : 2 June 2003

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
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)];

	ENTER(crate_get_samples_acquired_background_start);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"enter crate_get_samples_acquired_background_start %p %d\n",crate,
		channel_number);
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		lock_mutex(crate->command_socket_mutex);
		buffer[0]=UNEMAP_GET_SAMPLES_ACQUIRED_BACKGROUND_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		memcpy(buffer+buffer_size,&channel_number,sizeof(channel_number));
		buffer_size += sizeof(channel_number);
		if (crate->software_version>0)
		{
			memcpy(buffer+buffer_size,&number_of_samples,sizeof(number_of_samples));
			buffer_size += sizeof(number_of_samples);
		}
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
			"crate_get_samples_acquired_background_start.  "
			"Missing crate (%p) or invalid command_socket",crate);
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave crate_get_samples_acquired_background_start %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_get_samples_acquired_background_start */

static int crate_get_samples_acquired_background_end(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 29 August 2002

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
	unsigned char buffer[2+sizeof(long)+sizeof(int)];

	ENTER(crate_get_samples_acquired_background_end);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"enter crate_get_samples_acquired_background_end %p %d\n",crate);
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if (crate&&(INVALID_SOCKET!=crate->command_socket))
	{
		/* get the header back */
		retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
			buffer,2+sizeof(long),0);
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
		unlock_mutex(crate->command_socket_mutex);
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_get_samples_acquired_background_end.  "
			"Missing crate (%p) or invalid command_socket",crate);
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave crate_get_samples_acquired_background_end %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_get_samples_acquired_background_end */

static int crate_get_maximum_number_of_samples(struct Unemap_crate *crate,
	unsigned long *number_of_samples)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

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
			lock_mutex(crate->command_socket_mutex);
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
				retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
					(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
					buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
							(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
							buffer,sizeof(long),0))
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
			unlock_mutex(crate->command_socket_mutex);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_maximum_number_of_samples.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_get_maximum_number_of_samples.  "
			"Missing crate (%p) or number_of_samples (%p)",crate,number_of_samples);
	}
	LEAVE;

	return (return_code);
} /* crate_get_maximum_number_of_samples */

static int crate_get_sampling_frequency(struct Unemap_crate *crate,
	float *frequency)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

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
			lock_mutex(crate->command_socket_mutex);
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
				retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
					(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
					buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
							(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
							buffer,sizeof(float),0))
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
			unlock_mutex(crate->command_socket_mutex);
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

static int crate_get_scrolling_frequency(struct Unemap_crate *crate,
	float *frequency)
/*******************************************************************************
LAST MODIFIED : 31 October 2003

DESCRIPTION :
The function fails if the hardware is not configured.

The scrolling frequency is assigned to <*frequency>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(float)];

	ENTER(crate_get_scrolling_frequency);
	return_code=0;
	/* check arguments */
	if (crate&&frequency)
	{
		if (3<=crate->software_version)
		{
			if (INVALID_SOCKET!=crate->command_socket)
			{
				lock_mutex(crate->command_socket_mutex);
				buffer[0]=UNEMAP_GET_SCROLLING_FREQUENCY_CODE;
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
					retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
						(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
						buffer,2+sizeof(long),0);
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
							if (SOCKET_ERROR!=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
								(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
								buffer,sizeof(float),0))
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
							"crate_get_scrolling_frequency.  socket_recv() failed");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_get_scrolling_frequency.  socket_send() failed");
				}
				unlock_mutex(crate->command_socket_mutex);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_scrolling_frequency.  Invalid command_socket");
			}
		}
		else
		{
			*frequency=(float)100;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"crate_get_scrolling_frequency.  Missing crate (%p) or frequency (%p)",
			crate,frequency);
	}
	LEAVE;

	return (return_code);
} /* crate_get_scrolling_frequency */

static int crate_get_scrolling_callback_frequency(struct Unemap_crate *crate,
	float *frequency)
/*******************************************************************************
LAST MODIFIED : 31 October 2003

DESCRIPTION :
The function fails if the hardware is not configured.

The scrolling callback frequency is assigned to <*frequency>.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+sizeof(float)];

	ENTER(crate_get_scrolling_callback_frequency);
	return_code=0;
	/* check arguments */
	if (crate&&frequency)
	{
		if (3<=crate->software_version)
		{
			if (INVALID_SOCKET!=crate->command_socket)
			{
				lock_mutex(crate->command_socket_mutex);
				buffer[0]=UNEMAP_GET_SCROLLING_CALLBACK_FREQUENCY_CODE;
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
					retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
						(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
						buffer,2+sizeof(long),0);
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
							if (SOCKET_ERROR!=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
								(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
								buffer,sizeof(float),0))
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
							"crate_get_scrolling_callback_frequency.  socket_recv() failed");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"crate_get_scrolling_callback_frequency.  socket_send() failed");
				}
				unlock_mutex(crate->command_socket_mutex);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"crate_get_scrolling_callback_frequency.  Invalid command_socket");
			}
		}
		else
		{
			*frequency=(float)25;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_get_scrolling_callback_frequency.  "
			"Missing crate (%p) or frequency (%p)",crate,frequency);
	}
	LEAVE;

	return (return_code);
} /* crate_get_scrolling_callback_frequency */

static int crate_get_gain_start(struct Unemap_crate *crate,int channel_number)
/*******************************************************************************
LAST MODIFIED : 27 July 2000

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
			lock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 5 August 2002

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
			retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
					if (SOCKET_ERROR!=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
						(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
						buffer,2*sizeof(float),0))
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
			unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 19 November 2003

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
		lock_mutex(crate->command_socket_mutex);
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
			retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
				display_message(ERROR_MESSAGE,"crate_set_gain.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"crate_set_gain.  socket_send() failed");
		}
		unlock_mutex(crate->command_socket_mutex);
#if defined (CACHE_CLIENT_INFORMATION)
		/*???DB.  Has to be here because of mutexs */
		if (return_code)
		{
			if (crate->unemap_cards)
			{
				module_force_connection=1;
				if (0==channel_number)
				{
					unemap_card=crate->unemap_cards;
					for (i=0;i<crate->number_of_unemap_cards;i++)
					{
						if (0<unemap_card->channel_number)
						{
							crate_get_gain_start(crate,unemap_card->channel_number);
							crate_get_gain_end(crate,&(unemap_card->pre_filter_gain),
								&(unemap_card->post_filter_gain));
						}
						unemap_card++;
					}
				}
				else
				{
					unemap_card=(crate->unemap_cards)+
						((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
					crate_get_gain_start(crate,channel_number);
					crate_get_gain_end(crate,&(unemap_card->pre_filter_gain),
						&(unemap_card->post_filter_gain));
				}
				module_force_connection=0;
			}
		}
#endif /* defined (CACHE_CLIENT_INFORMATION) */
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
	float voltages_per_second,float *voltages,unsigned int number_of_cycles)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

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

If <number_of_cycles> is zero then the waveform is repeated until
<crate_stop_stimulating>, otherwise the waveform is repeated the
<number_of_cycles> times or until <crate_stop_stimulating>.

Use unemap_set_channel_stimulating to make a channel into a stimulating channel.
Use <unemap_start_stimulating> to start the stimulating.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)+sizeof(float)+
		sizeof(unsigned int)];

	ENTER(crate_load_voltage_stimulating);
	return_code=0;
	if (crate&&((0==number_of_channels)||((0<number_of_channels)&&
		channel_numbers))&&((0==number_of_voltages)||((0<number_of_voltages)&&
		voltages)))
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			lock_mutex(crate->command_socket_mutex);
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
			memcpy(buffer+buffer_size,&number_of_cycles,
				sizeof(number_of_cycles));
			buffer_size += sizeof(number_of_cycles);
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
				retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
					(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
					buffer,2+sizeof(long),0);
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
						((int)sizeof(float)*number_of_voltages==buffer_size)&&(buffer[0]))
					{
						if (0<buffer_size)
						{
							retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
								(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
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
			unlock_mutex(crate->command_socket_mutex);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_load_voltage_stimulating.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_load_voltage_stimulating.  "
			"Missing crate (%p) or invalid number_of_channels (%d) or "
			"channel_numbers (%p) or number_of_voltages (%d) or voltages (%p)",
			crate,number_of_channels,channel_numbers,number_of_voltages,voltages);
	}
	LEAVE;

	return (return_code);
} /* crate_load_voltage_stimulating */

static int crate_load_current_stimulating(struct Unemap_crate *crate,
	int number_of_channels,int *channel_numbers,int number_of_currents,
	float currents_per_second,float *currents,unsigned int number_of_cycles)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

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

If <number_of_cycles> is zero then the waveform is repeated until
<crate_stop_stimulating>, otherwise the waveform is repeated the
<number_of_cycles> times or until <crate_stop_stimulating>.

The <currents> are those desired as a proportion of the maximum (dependent on
the impedance being driven).  The function sets <currents> to the actual values
used.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)+2*sizeof(int)+sizeof(float)+
		sizeof(unsigned int)];

	ENTER(crate_load_current_stimulating);
	return_code=0;
	if (crate&&((0==number_of_channels)||((0<number_of_channels)&&
		channel_numbers))&&((0==number_of_currents)||((0<number_of_currents)&&
		currents)))
	{
		if (INVALID_SOCKET!=crate->command_socket)
		{
			lock_mutex(crate->command_socket_mutex);
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
			memcpy(buffer+buffer_size,&number_of_cycles,
				sizeof(number_of_cycles));
			buffer_size += sizeof(number_of_cycles);
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
				retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
					(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
					buffer,2+sizeof(long),0);
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
#if defined (WIN32_SYSTEM)
								(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
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
			unlock_mutex(crate->command_socket_mutex);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_load_current_stimulating.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_load_current_stimulating.  "
			"Missing crate (%p) or invalid number_of_channels (%d) or "
			"channel_numbers (%p) or number_of_currents (%d) or currents (%p)",
			crate,number_of_channels,channel_numbers,number_of_currents,currents);
	}
	LEAVE;

	return (return_code);
} /* crate_load_current_stimulating */

static int crate_start_stimulating(struct Unemap_crate *crate)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

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
			lock_mutex(crate->command_socket_mutex);
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
				retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
					(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
					buffer,2+sizeof(long),0);
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
			unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 5 August 2002

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
		lock_mutex(crate->command_socket_mutex);
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
			retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
		unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 5 August 2002

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
		lock_mutex(crate->command_socket_mutex);
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
			retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
		unlock_mutex(crate->command_socket_mutex);
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_set_channel_stimulating.  "
			"Missing crate (%p) or invalid command_socket",crate);
	}
	LEAVE;

	return (return_code);
} /* crate_set_channel_stimulating */

static int crate_get_channel_stimulating(struct Unemap_crate *crate,
	int channel_number,int *stimulating)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

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
			lock_mutex(crate->command_socket_mutex);
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
				retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
					(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
					buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
							(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
							buffer,sizeof(int),0))
						{
							memcpy(stimulating,buffer,sizeof(int));
#if defined (DEBUG)
							/*???debug */
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
			unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 5 August 2002

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
			lock_mutex(crate->command_socket_mutex);
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
				retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
					(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
					buffer,2+sizeof(long),0);
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
#if defined (WIN32_SYSTEM)
								(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
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
			unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 5 August 2002

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
		lock_mutex(crate->command_socket_mutex);
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
			retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
		unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 27 July 2000

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
		lock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 5 August 2002

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
		retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
			(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
			buffer,2+sizeof(long),0);
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
		unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 5 August 2002

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
			lock_mutex(crate->command_socket_mutex);
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
				retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
					(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
					buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
							(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
							buffer,sizeof(int),0))
						{
							memcpy(on,buffer,sizeof(int));
#if defined (DEBUG)
							/*???debug */
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
			unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 27 July 2000

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
			lock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 5 August 2002

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
			retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
					if (SOCKET_ERROR!=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
						(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
						buffer,sizeof(int),0))
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
			unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 27 July 2000

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
			lock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 5 August 2002

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
			retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
			unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 5 August 2002

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
			lock_mutex(crate->command_socket_mutex);
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
				retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
					(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
					buffer,2+sizeof(long),0);
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
						if (SOCKET_ERROR!=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
							(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
							buffer,buffer_size,0))
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
			unlock_mutex(crate->command_socket_mutex);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"crate_get_card_state.  Invalid command_socket");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"crate_get_card_state.  "
			"Invalid argument(s).  %p %d %p %p %p %p %p %p %p %p %p %p %p %p %p",
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
LAST MODIFIED : 5 August 2002

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
		lock_mutex(crate->command_socket_mutex);
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
			retval=socket_recv(crate->command_socket,
#if defined (WIN32_SYSTEM)
				(HANDLE)NULL,
#endif /* defined (WIN32_SYSTEM) */
				buffer,2+sizeof(long),0);
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
		unlock_mutex(crate->command_socket_mutex);
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
LAST MODIFIED : 13 August 2003

DESCRIPTION :
Sets up the connection with the unemap hardware service for the <crate>.
==============================================================================*/
{
#if defined (USE_SOCKETS)
	int return_code,socket_type;
	struct hostent *internet_host_data;
	struct sockaddr_in server_socket;
#if defined (UNIX)
#if !defined (CREATE_MUTEX)
	int error_code;
#endif /* !defined (CREATE_MUTEX) */
#endif /* defined (UNIX) */
#endif /* defined (USE_SOCKETS) */

	ENTER(crate_initialize_connection);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"enter crate_initialize_connection %p\n",crate);
#endif /* defined (DEBUG) */
	return_code=0;
	if (crate&&(crate->server_name))
	{
#if defined (DEBUG)
		/*???debug */
		display_message(INFORMATION_MESSAGE,
			"server_name=/%s/, command_socket=%d\n",crate->server_name,
			crate->command_socket);
#endif /* defined (DEBUG) */
#if defined (USE_SOCKETS)
		if (INVALID_SOCKET==crate->command_socket)
		{
			/* create mutex's */
				/*???DB.  When do others, free if failure */
#if defined (CREATE_MUTEX)
			if (crate->command_socket_mutex=create_mutex())
			{
				if (crate->acquired_socket_mutex=create_mutex())
				{
#if defined (WIN32_SYSTEM)
					/* create stopped events */
					if (crate->scrolling_socket_thread_stopped_event=CreateEvent(
						/*no security attributes*/NULL,/*manual reset event*/TRUE,
						/*not-signalled*/FALSE,/*no name*/NULL))
					{
						if (crate->acquired_socket_thread_stopped_event=CreateEvent(
							/*no security attributes*/NULL,/*manual reset event*/TRUE,
							/*not-signalled*/FALSE,/*no name*/NULL))
						{
							if (crate->calibration_socket_thread_stopped_event=CreateEvent(
								/*no security attributes*/NULL,/*manual reset event*/TRUE,
								/*not-signalled*/FALSE,/*no name*/NULL))
							{
								if ((INVALID_SOCKET==crate->stimulation_socket)||
									(crate->stimulation_socket_thread_stopped_event=
									CreateEvent(/*no security attributes*/NULL,
									/*manual reset event*/TRUE,/*not-signalled*/FALSE,
									/*no name*/NULL)))
								{
									return_code=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"crate_initialize_connection.  "
										"CreateEvent failed for "
										"stimulation_socket_thread_stopped_event.  Error code %d",
										GetLastError());
									CloseHandle(crate->calibration_socket_thread_stopped_event);
									crate->calibration_socket_thread_stopped_event=NULL;
									CloseHandle(crate->acquired_socket_thread_stopped_event);
									crate->acquired_socket_thread_stopped_event=NULL;
									CloseHandle(crate->scrolling_socket_thread_stopped_event);
									crate->scrolling_socket_thread_stopped_event=NULL;
									destroy_mutex(&(crate->acquired_socket_mutex));
									destroy_mutex(&(crate->command_socket_mutex));
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"crate_initialize_connection.  "
									"CreateEvent failed for "
									"calibration_socket_thread_stopped_event.  Error code %d",
									GetLastError());
								CloseHandle(crate->acquired_socket_thread_stopped_event);
								crate->acquired_socket_thread_stopped_event=NULL;
								CloseHandle(crate->scrolling_socket_thread_stopped_event);
								crate->scrolling_socket_thread_stopped_event=NULL;
								destroy_mutex(&(crate->acquired_socket_mutex));
								destroy_mutex(&(crate->command_socket_mutex));
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"crate_initialize_connection.  "
								"CreateEvent failed for "
								"acquired_socket_thread_stopped_event.  "
								"Error code %d",GetLastError());
							CloseHandle(crate->scrolling_socket_thread_stopped_event);
							crate->scrolling_socket_thread_stopped_event=NULL;
							destroy_mutex(&(crate->acquired_socket_mutex));
							destroy_mutex(&(crate->command_socket_mutex));
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"crate_initialize_connection.  "
							"CreateEvent failed for "
							"scrolling_socket_thread_stopped_event.  "
							"Error code %d",GetLastError());
						destroy_mutex(&(crate->acquired_socket_mutex));
						destroy_mutex(&(crate->command_socket_mutex));
					}
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
					return_code=1;
#endif /* defined (UNIX) */
				}
				else
				{
					display_message(ERROR_MESSAGE,"crate_initialize_connection.  "
						"create_mutex failed for acquired_socket_mutex");
					destroy_mutex(&(crate->command_socket_mutex));
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"crate_initialize_connection.  "
					"create_mutex failed for command_socket_mutex");
			}
#else /* defined (CREATE_MUTEX) */
#if defined (WIN32_SYSTEM)
			if (crate->command_socket_mutex=CreateMutex(
				/*no security attributes*/NULL,/*do not initially own*/FALSE,
				/*no name*/(LPCTSTR)NULL))
			{
				if (crate->acquired_socket_mutex=CreateMutex(
					/*no security attributes*/NULL,/*do not initially own*/FALSE,
					/*no name*/(LPCTSTR)NULL))
				{
					/* create stopped events */
					if (crate->scrolling_socket_thread_stopped_event=CreateEvent(
						/*no security attributes*/NULL,/*manual reset event*/TRUE,
						/*not-signalled*/FALSE,/*no name*/NULL))
					{
						if (crate->acquired_socket_thread_stopped_event=CreateEvent(
							/*no security attributes*/NULL,/*manual reset event*/TRUE,
							/*not-signalled*/FALSE,/*no name*/NULL))
						{
							if (crate->calibration_socket_thread_stopped_event=CreateEvent(
								/*no security attributes*/NULL,/*manual reset event*/TRUE,
								/*not-signalled*/FALSE,/*no name*/NULL))
							{
								if ((INVALID_SOCKET==crate->stimulation_socket)||
									(crate->stimulation_socket_thread_stopped_event=
									CreateEvent(/*no security attributes*/NULL,
									/*manual reset event*/TRUE,/*not-signalled*/FALSE,
									/*no name*/NULL)))
								{
									return_code=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"crate_initialize_connection.  "
										"CreateEvent failed for "
										"stimulation_socket_thread_stopped_event.  Error code %d",
										GetLastError());
									CloseHandle(crate->calibration_socket_thread_stopped_event);
									crate->calibration_socket_thread_stopped_event=NULL;
									CloseHandle(crate->acquired_socket_thread_stopped_event);
									crate->acquired_socket_thread_stopped_event=NULL;
									CloseHandle(crate->scrolling_socket_thread_stopped_event);
									crate->scrolling_socket_thread_stopped_event=NULL;
									CloseHandle(crate->acquired_socket_mutex);
									crate->acquired_socket_mutex=NULL;
									CloseHandle(crate->command_socket_mutex);
									crate->command_socket_mutex=NULL;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"crate_initialize_connection.  "
									"CreateEvent failed for "
									"calibration_socket_thread_stopped_event.  Error code %d",
									GetLastError());
								CloseHandle(crate->acquired_socket_thread_stopped_event);
								crate->acquired_socket_thread_stopped_event=NULL;
								CloseHandle(crate->scrolling_socket_thread_stopped_event);
								crate->scrolling_socket_thread_stopped_event=NULL;
								CloseHandle(crate->acquired_socket_mutex);
								crate->acquired_socket_mutex=NULL;
								CloseHandle(crate->command_socket_mutex);
								crate->command_socket_mutex=NULL;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"crate_initialize_connection.  "
								"CreateEvent failed for "
								"acquired_socket_thread_stopped_event.  "
								"Error code %d",GetLastError());
							CloseHandle(crate->scrolling_socket_thread_stopped_event);
							crate->scrolling_socket_thread_stopped_event=NULL;
							CloseHandle(crate->acquired_socket_mutex);
							crate->acquired_socket_mutex=NULL;
							CloseHandle(crate->command_socket_mutex);
							crate->command_socket_mutex=NULL;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"crate_initialize_connection.  "
							"CreateEvent failed for "
							"scrolling_socket_thread_stopped_event.  "
							"Error code %d",GetLastError());
						CloseHandle(crate->acquired_socket_mutex);
						crate->acquired_socket_mutex=NULL;
						CloseHandle(crate->command_socket_mutex);
						crate->command_socket_mutex=NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"crate_initialize_connection.  "
						"CreateMutex failed for acquired_socket_mutex.  Error code %d",
						GetLastError());
					CloseHandle(crate->command_socket_mutex);
					crate->command_socket_mutex=NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"crate_initialize_connection.  "
					"CreateMutex failed for command_socket_mutex.  Error code %d",
					GetLastError());
			}
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
			if (0==(error_code=pthread_mutex_init(
				&(crate->command_socket_mutex_storage),
				(pthread_mutexattr_t *)NULL)))
			{
				crate->command_socket_mutex=
					&(crate->command_socket_mutex_storage);
				if (0==(error_code=pthread_mutex_init(
					&(crate->acquired_socket_mutex_storage),
					(pthread_mutexattr_t *)NULL)))
				{
					crate->acquired_socket_mutex=
						&(crate->acquired_socket_mutex_storage);
					return_code=1;
				}
				else
				{
					return_code=0;
					display_message(ERROR_MESSAGE,"crate_initialize_connection.  "
						"pthread_mutex_init failed for acquired_socket_mutex.  "
						"Error code %d",error_code);
					crate->acquired_socket_mutex=(pthread_mutex_t *)NULL;
					pthread_mutex_destroy(crate->command_socket_mutex);
					crate->command_socket_mutex=(pthread_mutex_t *)NULL;
				}
			}
			else
			{
				return_code=0;
				display_message(ERROR_MESSAGE,"crate_initialize_connection.  "
					"pthread_mutex_init failed for command_socket_mutex.  Error code %d",
					error_code);
				crate->acquired_socket_mutex=(pthread_mutex_t *)NULL;
				crate->command_socket_mutex=(pthread_mutex_t *)NULL;
			}
#endif /* defined (UNIX) */
#endif /* defined (CREATE_MUTEX) */
			if (return_code)
			{
				return_code=0;
				internet_host_data=gethostbyname(crate->server_name);
				if (internet_host_data)
				{
					/* create the acquired socket.  Do this before the command
						socket because the service responds to the command socket
						connection */
					socket_type=DEFAULT_SOCKET_TYPE;
					crate->acquired_socket=socket(AF_INET,socket_type,0);
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"acquired_socket=%d\n",crate->acquired_socket);
#endif /* defined (DEBUG) */
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
						display_message(INFORMATION_MESSAGE,
							"acquired_port=%d\n",crate->acquired_port);
#endif /* defined (DEBUG) */
						if (SOCKET_ERROR!=connect(crate->acquired_socket,
							(struct sockaddr *)&server_socket,sizeof(server_socket)))
						{
							/* create the calibration socket.  Do this before the command
								socket because the service responds to the command socket
								connection */
							socket_type=DEFAULT_SOCKET_TYPE;
							crate->calibration_socket=socket(AF_INET,socket_type,0);
#if defined (DEBUG)
							/*???debug */
							display_message(INFORMATION_MESSAGE,
								"calibration_socket=%d\n",crate->calibration_socket);
#endif /* defined (DEBUG) */
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
								display_message(INFORMATION_MESSAGE,
									"calibration_port=%d\n",crate->calibration_port);
#endif /* defined (DEBUG) */
								if (SOCKET_ERROR!=connect(crate->calibration_socket,
									(struct sockaddr *)&server_socket,sizeof(server_socket)))
								{
									/* create the scrolling socket.  Do this before the command
										socket because the service responds to the command socket
										connection */
									socket_type=DEFAULT_SOCKET_TYPE;
									crate->scrolling_socket=socket(AF_INET,socket_type,0);
#if defined (DEBUG)
									/*???debug */
									display_message(INFORMATION_MESSAGE,
										"scrolling_socket=%d\n",crate->scrolling_socket);
#endif /* defined (DEBUG) */
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
										display_message(INFORMATION_MESSAGE,
											"scrolling_port=%d\n",crate->scrolling_port);
#endif /* defined (DEBUG) */
										if (SOCKET_ERROR!=connect(crate->scrolling_socket,
											(struct sockaddr *)&server_socket,sizeof(server_socket)))
										{
											/* create the stimulation socket.  Do this before the
												command socket because the service responds to the
												command socket connection */
											socket_type=DEFAULT_SOCKET_TYPE;
											crate->stimulation_socket=socket(AF_INET,socket_type,0);
#if defined (DEBUG)
											/*???debug */
											display_message(INFORMATION_MESSAGE,
												"stimulation_socket=%d\n",crate->stimulation_socket);
#endif /* defined (DEBUG) */
											if (INVALID_SOCKET!=crate->stimulation_socket)
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
												server_socket.sin_port=htons(crate->stimulation_port);
#if defined (DEBUG)
												/*???debug */
												display_message(INFORMATION_MESSAGE,
													"stimulation_port=%d\n",crate->stimulation_port);
#endif /* defined (DEBUG) */
												/* if the service has version less than 3, it will not
													have a stimulation_socket and the connect will fail
													leaving an un'connect'ed socket (no traffic) */
												if (INVALID_SOCKET==connect(crate->stimulation_socket,
													(struct sockaddr *)&server_socket,
													sizeof(server_socket)))
												{
#if defined (WIN32_SYSTEM)
													closesocket(crate->stimulation_socket);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
													close(crate->stimulation_socket);
#endif /* defined (UNIX) */
													crate->stimulation_socket=INVALID_SOCKET;
												}
												/* create the command socket */
												socket_type=DEFAULT_SOCKET_TYPE;
												crate->command_socket=socket(AF_INET,socket_type,0);
#if defined (DEBUG)
												/*???debug */
												display_message(INFORMATION_MESSAGE,
													"command_socket=%d\n",crate->command_socket);
#endif /* defined (DEBUG) */
												if (INVALID_SOCKET!=crate->command_socket)
												{
													/* connect to the server */
													memset(&server_socket,0,sizeof(server_socket));
														/*???DB.  Have to use memset because some
															implementations of struct sockaddr_in don't have
															the sin_len field */
													memcpy(&(server_socket.sin_addr),
														internet_host_data->h_addr,
														internet_host_data->h_length);
													server_socket.sin_family=
														internet_host_data->h_addrtype;
													server_socket.sin_port=htons(crate->command_port);
#if defined (DEBUG)
													/*???debug */
													display_message(INFORMATION_MESSAGE,
														"command_port=%d\n",crate->command_port);
#endif /* defined (DEBUG) */
													if (SOCKET_ERROR!=connect(crate->command_socket,
														(struct sockaddr *)&server_socket,
														sizeof(server_socket)))
													{
														if (crate_get_number_of_channels(crate,
															&(crate->number_of_channels)))
														{
															crate->number_of_configured_channels=0;
															crate_get_software_version(crate,
																&(crate->software_version));
															return_code=1;
														}
														else
														{
															display_message(ERROR_MESSAGE,
																"crate_initialize_connection.  "
																"Could not get the number of channels");
#if defined (WIN32_SYSTEM)
															closesocket(crate->acquired_socket);
															closesocket(crate->calibration_socket);
															closesocket(crate->scrolling_socket);
															if (INVALID_SOCKET!=crate->stimulation_socket)
															{
																closesocket(crate->stimulation_socket);
															}
															closesocket(crate->command_socket);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
															close(crate->acquired_socket);
															close(crate->calibration_socket);
															close(crate->scrolling_socket);
															if (INVALID_SOCKET!=crate->stimulation_socket)
															{
																close(crate->stimulation_socket);
															}
															close(crate->command_socket);
#endif /* defined (UNIX) */
															crate->acquired_socket=INVALID_SOCKET;
															crate->calibration_socket=INVALID_SOCKET;
															crate->scrolling_socket=INVALID_SOCKET;
															crate->stimulation_socket=INVALID_SOCKET;
															crate->command_socket=INVALID_SOCKET;
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
															"crate_initialize_connection.  "
															"Could not connect command_socket.  "
															"Port %hu.  Error code %d",crate->command_port,
#if defined (WIN32_SYSTEM)
															WSAGetLastError()
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
															errno
#endif /* defined (UNIX) */
															);
#if defined (WIN32_SYSTEM)
														closesocket(crate->acquired_socket);
														closesocket(crate->calibration_socket);
														closesocket(crate->scrolling_socket);
														if (INVALID_SOCKET!=crate->stimulation_socket)
														{
															closesocket(crate->stimulation_socket);
														}
														closesocket(crate->command_socket);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
														close(crate->acquired_socket);
														close(crate->calibration_socket);
														close(crate->scrolling_socket);
														if (INVALID_SOCKET!=crate->stimulation_socket)
														{
															close(crate->stimulation_socket);
														}
														close(crate->command_socket);
#endif /* defined (UNIX) */
														crate->acquired_socket=INVALID_SOCKET;
														crate->calibration_socket=INVALID_SOCKET;
														crate->scrolling_socket=INVALID_SOCKET;
														crate->stimulation_socket=INVALID_SOCKET;
														crate->command_socket=INVALID_SOCKET;
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"crate_initialize_connection.  "
														"Could not create command_socket.  Error code %d",
#if defined (WIN32_SYSTEM)
														WSAGetLastError()
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
														errno
#endif /* defined (UNIX) */
														);
#if defined (WIN32_SYSTEM)
													closesocket(crate->acquired_socket);
													closesocket(crate->calibration_socket);
													closesocket(crate->scrolling_socket);
													if (INVALID_SOCKET!=crate->stimulation_socket)
													{
														closesocket(crate->stimulation_socket);
													}
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
													close(crate->acquired_socket);
													close(crate->calibration_socket);
													close(crate->scrolling_socket);
													if (INVALID_SOCKET!=crate->stimulation_socket)
													{
														close(crate->stimulation_socket);
													}
#endif /* defined (UNIX) */
													crate->acquired_socket=INVALID_SOCKET;
													crate->calibration_socket=INVALID_SOCKET;
													crate->scrolling_socket=INVALID_SOCKET;
													crate->stimulation_socket=INVALID_SOCKET;
												}
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"crate_initialize_connection.  "
													"Could not create stimulation_socket.  Error code %d",
#if defined (WIN32_SYSTEM)
													WSAGetLastError()
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
													errno
#endif /* defined (UNIX) */
													);
#if defined (WIN32_SYSTEM)
												closesocket(crate->acquired_socket);
												closesocket(crate->calibration_socket);
												closesocket(crate->scrolling_socket);
#endif /* defined (WIN32_SYSTEM) */
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
												"crate_initialize_connection.  "
												"Could not connect scrolling_socket.  Port %hu.  "
												"Error code %d",crate->scrolling_port,
#if defined (WIN32_SYSTEM)
												WSAGetLastError()
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
												errno
#endif /* defined (UNIX) */
												);
#if defined (WIN32_SYSTEM)
											closesocket(crate->acquired_socket);
											closesocket(crate->calibration_socket);
											closesocket(crate->scrolling_socket);
#endif /* defined (WIN32_SYSTEM) */
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
											"crate_initialize_connection.  "
											"Could not create scrolling_socket.  Error code %d",
#if defined (WIN32_SYSTEM)
											WSAGetLastError()
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
											errno
#endif /* defined (UNIX) */
											);
#if defined (WIN32_SYSTEM)
										closesocket(crate->acquired_socket);
										closesocket(crate->calibration_socket);
#endif /* defined (WIN32_SYSTEM) */
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
									display_message(ERROR_MESSAGE,"crate_initialize_connection.  "
										"Could not connect calibration_socket.  Port %hu.  "
										"Error code %d",crate->calibration_port,
#if defined (WIN32_SYSTEM)
										WSAGetLastError()
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
										errno
#endif /* defined (UNIX) */
										);
#if defined (WIN32_SYSTEM)
									closesocket(crate->acquired_socket);
									closesocket(crate->calibration_socket);
#endif /* defined (WIN32_SYSTEM) */
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
								display_message(ERROR_MESSAGE,"crate_initialize_connection.  "
									"Could not create calibration_socket.  Error code %d",
#if defined (WIN32_SYSTEM)
									WSAGetLastError()
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
									errno
#endif /* defined (UNIX) */
									);
#if defined (WIN32_SYSTEM)
								closesocket(crate->acquired_socket);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
								close(crate->acquired_socket);
#endif /* defined (UNIX) */
								crate->acquired_socket=INVALID_SOCKET;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"crate_initialize_connection.  "
								"Could not connect acquired_socket.  Port %hu.  Error code %d",
								crate->acquired_port,
#if defined (WIN32_SYSTEM)
								WSAGetLastError()
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
								errno
#endif /* defined (UNIX) */
								);
#if defined (WIN32_SYSTEM)
							closesocket(crate->acquired_socket);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
							close(crate->acquired_socket);
#endif /* defined (UNIX) */
							crate->acquired_socket=INVALID_SOCKET;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"crate_initialize_connection.  "
							"Could not create acquired_socket.  Error code %d",
#if defined (WIN32_SYSTEM)
							WSAGetLastError()
#endif /* defined (WIN32_SYSTEM) */
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
			if (!return_code)
			{
				/* free mutex's */
				if (crate->command_socket_mutex)
				{
#if defined (CREATE_MUTEX)
					destroy_mutex(&(crate->command_socket_mutex));
#else /* defined (CREATE_MUTEX) */
#if defined (WIN32_SYSTEM)
					CloseHandle(crate->command_socket_mutex);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
					pthread_mutex_destroy(crate->command_socket_mutex);
#endif /* defined (UNIX) */
#endif /* defined (CREATE_MUTEX) */
				}
				if (crate->acquired_socket_mutex)
				{
#if defined (CREATE_MUTEX)
					destroy_mutex(&(crate->acquired_socket_mutex));
#else /* defined (CREATE_MUTEX) */
#if defined (WIN32_SYSTEM)
					CloseHandle(crate->acquired_socket_mutex);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
					pthread_mutex_destroy(crate->acquired_socket_mutex);
#endif /* defined (UNIX) */
#endif /* defined (CREATE_MUTEX) */
				}
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
	display_message(INFORMATION_MESSAGE,
		"leave crate_initialize_connection %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* crate_initialize_connection */

static int initialize_connection(void)
/*******************************************************************************
LAST MODIFIED : 11 August 2003

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
#if defined (WIN32_SYSTEM)
	WORD wVersionRequested;
	WSADATA wsaData;
#endif /* defined (WIN32_SYSTEM) */
#if !defined (CREATE_MUTEX)
#if defined (UNIX)
	int error_code;
#endif /* defined (UNIX) */
#endif /* !defined (CREATE_MUTEX) */
#endif /* defined (USE_SOCKETS) */

	ENTER(initialize_connection);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"enter initialize_connection %p %d %d\n",module_unemap_crates,
		module_number_of_unemap_crates,allow_open_connection);
#endif /* defined (DEBUG) */
	return_code=0;
#if defined (USE_SOCKETS)
	if (!connection_mutex)
	{
#if defined (CREATE_MUTEX)
		if (!(connection_mutex=create_mutex()))
		{
			display_message(ERROR_MESSAGE,"initialize_connection.  "
				"create_mutex failed for connection_mutex");
		}
#else /* defined (CREATE_MUTEX) */
#if defined (WIN32_SYSTEM)
		if (!(connection_mutex=CreateMutex(/*no security attributes*/NULL,
			/*do not initially own*/FALSE,/*no name*/(LPCTSTR)NULL)))
		{
			display_message(ERROR_MESSAGE,"initialize_connection.  "
				"CreateMutex failed for connection_mutex.  Error code %d",
				GetLastError());
		}
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
		if (0==(error_code=pthread_mutex_init(&connection_mutex_storage,
			(pthread_mutexattr_t *)NULL)))
		{
			connection_mutex= &connection_mutex_storage;
			return_code=1;
		}
		else
		{
			return_code=0;
			display_message(ERROR_MESSAGE,"initialize_connection.  "
				"pthread_mutex_init failed for connection_mutex.  Error code %d",
				error_code);
		}
#endif /* defined (UNIX) */
#endif /* defined (CREATE_MUTEX) */
	}
	if (!stimulation_mutex)
	{
#if defined (CREATE_MUTEX)
		if (!(stimulation_mutex=create_mutex()))
		{
			display_message(ERROR_MESSAGE,"initialize_connection.  "
				"create_mutex failed for stimulation_mutex");
		}
#else /* defined (CREATE_MUTEX) */
#if defined (WIN32_SYSTEM)
		if (!(stimulation_mutex=CreateMutex(/*no security attributes*/NULL,
			/*do not initially own*/FALSE,/*no name*/(LPCTSTR)NULL)))
		{
			display_message(ERROR_MESSAGE,"initialize_connection.  "
				"CreateMutex failed for stimulation_mutex.  Error code %d",
				GetLastError());
		}
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
		if (0==(error_code=pthread_mutex_init(&stimulation_mutex_storage,
			(pthread_mutexattr_t *)NULL)))
		{
			stimulation_mutex= &stimulation_mutex_storage;
			return_code=1;
		}
		else
		{
			return_code=0;
			display_message(ERROR_MESSAGE,"initialize_connection.  "
				"pthread_mutex_init failed for stimulation_mutex.  Error code %d",
				error_code);
		}
#endif /* defined (UNIX) */
#endif /* defined (CREATE_MUTEX) */
	}
	if (connection_mutex)
	{
		lock_mutex(connection_mutex);
	}
	if (!module_unemap_crates)
	{
		/* only allow one attempt to open a closed connection */
		if (allow_open_connection)
		{
			number_of_servers=0;
			if (hardware_directory=getenv("UNEMAP_HARDWARE"))
			{
				if (ALLOCATE(server_file_name,char,strlen(hardware_directory)+
					strlen(SERVER_FILE_NAME)+2))
				{
					strcpy(server_file_name,hardware_directory);
#if defined (WIN32_SYSTEM)
					if ('\\'!=server_file_name[strlen(server_file_name)-1])
					{
						strcat(server_file_name,"\\");
					}
#else /* defined (WIN32_SYSTEM) */
					if ('/'!=server_file_name[strlen(server_file_name)-1])
					{
						strcat(server_file_name,"/");
					}
#endif /* defined (WIN32_SYSTEM) */
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
				module_stimulation_end_callback_info_list=
					CREATE_LIST(Stimulation_end_callback_info)();
#if defined (WIN32_SYSTEM)
				wVersionRequested=MAKEWORD(2,2);
				if (SOCKET_ERROR!=WSAStartup(wVersionRequested,&wsaData))
				{
#endif /* defined (WIN32_SYSTEM) */
					port=DEFAULT_PORT;
					module_number_of_unemap_crates=0;
					module_number_of_channels=0;
					crate=module_unemap_crates;
					return_code=1;
					while (return_code&&(number_of_servers>0))
					{
						crate->software_version=0;
#if defined (WIN32_SYSTEM)
						crate->acquired_socket=(SOCKET)INVALID_SOCKET;
						crate->acquired_socket_thread_stop_event=NULL;
						crate->acquired_socket_thread_stopped_event=NULL;
						crate->acquired_socket_mutex=(HANDLE)NULL;
						crate->calibration_socket=(SOCKET)INVALID_SOCKET;
						crate->calibration_socket_thread_stop_event=NULL;
						crate->calibration_socket_thread_stopped_event=NULL;
						crate->command_socket=(SOCKET)INVALID_SOCKET;
						crate->command_socket_mutex=(HANDLE)NULL;
						crate->scrolling_socket=(SOCKET)INVALID_SOCKET;
						crate->scrolling_socket_thread_stop_event=NULL;
						crate->scrolling_socket_thread_stopped_event=NULL;
						crate->stimulation_socket=(SOCKET)INVALID_SOCKET;
						crate->stimulation_socket_thread_stop_event=NULL;
						crate->stimulation_socket_thread_stopped_event=NULL;
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
						crate->acquired_socket=(int)INVALID_SOCKET;
						crate->acquired_socket_mutex=(pthread_mutex_t *)NULL;
						crate->calibration_socket=(int)INVALID_SOCKET;
						crate->command_socket=(int)INVALID_SOCKET;
						crate->command_socket_mutex=(pthread_mutex_t *)NULL;
						crate->scrolling_socket=(int)INVALID_SOCKET;
						crate->stimulation_socket=(int)INVALID_SOCKET;
#endif /* defined (UNIX) */
#if defined (UNIX)
						crate->event_dispatcher=(struct Event_dispatcher *)NULL;
						crate->acquired_socket_xid=
							(struct Event_dispatcher_descriptor_callback *)NULL;
						crate->calibration_socket_xid=
							(struct Event_dispatcher_descriptor_callback *)NULL;
						crate->scrolling_socket_xid=
							(struct Event_dispatcher_descriptor_callback *)NULL;
						crate->stimulation_socket_xid=
							(struct Event_dispatcher_descriptor_callback *)NULL;
#endif /* defined (UNIX) */
						/*???DB.  The port doesn't have to be unique, only the port and
							machine being connected to */
						port=DEFAULT_PORT;
						crate->command_port=port;
						port++;
						crate->scrolling_port=port;
						port++;
						crate->calibration_port=port;
						port++;
						crate->acquired_port=port;
						port++;
						crate->stimulation_port=port;
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
						crate->number_of_configured_channels=0;
						crate->configured_channels=(int *)NULL;
						crate->configured_channel_offsets=(int *)NULL;
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
					}
					if (!return_code)
					{
#if defined (DEBUG)
						/*???debug */
						display_message(INFORMATION_MESSAGE,
							"initialize_connection:close_crate_connection\n");
#endif /* defined (DEBUG) */
						close_connection();
					}
#if defined (WIN32_SYSTEM)
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"initialize_connection.  WSAStartup failed.  Error code %d",
						WSAGetLastError());
				}
#endif /* defined (WIN32_SYSTEM) */
			}
			else
			{
				return_code=0;
			}
			allow_open_connection=0;
		}
	}
	else
	{
		/* already initialized */
		return_code=1;
		allow_open_connection=1;
	}
#endif /* defined (USE_SOCKETS) */
	if (connection_mutex)
	{
		unlock_mutex(connection_mutex);
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave initialize_connection %p %d %d %d\n",module_unemap_crates,
		module_number_of_unemap_crates,allow_open_connection,return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* initialize_connection */

#if defined (CACHE_CLIENT_INFORMATION)
static int get_cache_information(void)
/*******************************************************************************
LAST MODIFIED : 18 August 2003

DESCRIPTION :
Retrieves the unemap information that is cached with the client.
==============================================================================*/
{
	int i,j,maximum_number_of_unemap_cards,return_code,*stimulator_number,
		*stimulator_numbers;
	struct Unemap_crate *crate;

	ENTER(get_cache_information);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"enter get_cache_information\n");
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
						crate=module_unemap_crates;
						for (i=module_number_of_unemap_crates;i>0;i--)
						{
							for (j=0;j<crate->number_of_unemap_cards;j++)
							{
								((crate->unemap_cards)[j]).channel_number=0;
							}
							for (j=0;j<crate->number_of_configured_channels;j++)
							{
								(crate->unemap_cards)[((crate->configured_channels)[j]-1)/
									NUMBER_OF_CHANNELS_ON_NI_CARD].channel_number=
									(crate->configured_channels)[j];
							}
							crate++;
						}
						for (j=0;j<maximum_number_of_unemap_cards;j++)
						{
							crate=module_unemap_crates;
							stimulator_number=stimulator_numbers;
							for (i=module_number_of_unemap_crates;i>0;i--)
							{
								if ((*stimulator_number)&&(j<(crate->number_of_unemap_cards))&&
									(0<(crate->unemap_cards)[j].channel_number))
								{
									crate_get_voltage_range_start(crate,
										(crate->unemap_cards)[j].channel_number);
								}
								stimulator_number++;
								crate++;
							}
							crate=module_unemap_crates;
							stimulator_number=stimulator_numbers;
							for (i=module_number_of_unemap_crates;i>0;i--)
							{
								if ((*stimulator_number)&&(j<(crate->number_of_unemap_cards))&&
									(0<(crate->unemap_cards)[j].channel_number))
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
								if ((*stimulator_number)&&(j<(crate->number_of_unemap_cards))&&
									(0<(crate->unemap_cards)[j].channel_number))
								{
									crate_get_gain_start(crate,
										(crate->unemap_cards)[j].channel_number);
								}
								stimulator_number++;
								crate++;
							}
							crate=module_unemap_crates;
							stimulator_number=stimulator_numbers;
							for (i=module_number_of_unemap_crates;i>0;i--)
							{
								if ((*stimulator_number)&&(j<(crate->number_of_unemap_cards))&&
									(0<(crate->unemap_cards)[j].channel_number))
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
								if ((*stimulator_number)&&(j<(crate->number_of_unemap_cards))&&
									(0<(crate->unemap_cards)[j].channel_number))
								{
									crate_channel_valid_for_stimulator_start(crate,
										*stimulator_number,(crate->unemap_cards)[j].channel_number);
								}
								stimulator_number++;
								crate++;
							}
							crate=module_unemap_crates;
							stimulator_number=stimulator_numbers;
							for (i=module_number_of_unemap_crates;i>0;i--)
							{
								if ((*stimulator_number)&&(j<(crate->number_of_unemap_cards))&&
									(0<(crate->unemap_cards)[j].channel_number))
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
							crate=module_unemap_crates;
							stimulator_number=stimulator_numbers;
							for (i=module_number_of_unemap_crates;i>0;i--)
							{
								if ((*stimulator_number)&&(j<(crate->number_of_unemap_cards))&&
									(0<(crate->unemap_cards)[j].channel_number))
								{
									crate_get_sample_range(crate,
										(crate->unemap_cards)[j].channel_number,
										&((crate->unemap_cards[j]).minimum_sample_value),
										&((crate->unemap_cards[j]).maximum_sample_value));
								}
								stimulator_number++;
								crate++;
							}
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
	display_message(INFORMATION_MESSAGE,
		"leave get_cache_information %d %d\n",return_code,
		module_number_of_stimulators);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* get_cache_information */
#endif /* defined (CACHE_CLIENT_INFORMATION) */

/*
Global functions
----------------
*/
int unemap_configure(int number_of_channels,int *channel_numbers,
	float sampling_frequency,int number_of_samples_in_buffer,
#if defined (WIN32_USER_INTERFACE)
	HWND scrolling_window,UINT scrolling_message,
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (UNIX)
	struct Event_dispatcher *event_dispatcher,
#endif /* defined (UNIX) */
	Unemap_hardware_callback *scrolling_callback,void *scrolling_callback_data,
	float scrolling_frequency,float scrolling_callback_frequency,
	int synchronization_card)
/*******************************************************************************
LAST MODIFIED : 31 October 2003

DESCRIPTION :
Configures the hardware for sampling the specified channels
(<number_of_channels> and <channel_numbers>) at the specified
<sampling_frequency> and with the specified <number_of_samples_in_buffer>.
<sampling_frequency>, <number_of_samples_in_buffer>, <scrolling_frequency> and
<scrolling_callback_frequency> are requests and the actual values used should
be determined using unemap_get_sampling_frequency,
unemap_get_maximum_number_of_samples, unemap_get_scrolling_frequency and
unemap_get_scrolling_callback_frequency.

If <number_of_channels> is
- -1 then the hardware is configured with no channels
- 0 then all channels are configured
- otherwise, the <number_of_channels> listed in <channel_numbers> are
	configured.

Every <sampling_frequency>/<scrolling_callback_frequency> samples (one sample
	is a measurement on every channel)
- if <scrolling_callback> is not NULL, <scrolling_callback> is called with
	- first argument is the number of scrolling channels
	- second argument is an array of channel numbers (in the order they were added
		using unemap_set_scrolling_channel)
	- third argument is the number of values for each channel
		(<scrolling_frequency>/<scrolling_callback_frequency>)
	- fourth argument is an array of channel values (all the values for the first
		channel, then all the values for the second channel and so on)
	- fifth argument is the user supplied callback data
	- it is the responsibilty of the callback to free the channel numbers and
		values arrays
- for WIN32_USER_INTERFACE, if <scrolling_window> is not NULL, a
	<scrolling_message> for <scrolling_window> is added to the message queue with
	- WPARAM is an array of unsigned char amalgamating the five arguments above
		- first sizeof(int) bytes is the number of scrolling channels
		- next (number of scrolling channels)*sizeof(int) bytes are the scrolling
			channel numbers
		- next sizeof(int) bytes is the number of values for each channel
		- next (number of scrolling channels)*(number of values)*sizeof(short) bytes
			are the channel values
		- last sizeof(void *) bytes are the user supplied callback data
	- LPARAM is a ULONG specifying the number of bytes in the WPARAM array
		over the <scrolling_callback_period>) in the array
	- it is the responsibility of the window message handler to free the WPARAM
		array.

Initially there are no scrolling channels.

<synchronization_card> is the number of the NI card, starting from 1 on the
left, for which the synchronization signal is input if the crate is a slave, via
the 5-way cable, to the attached SCU.  All other SCUs output the synchronization
signal.
==============================================================================*/
{
	float crate_sampling_frequency,crate_scrolling_callback_frequency,
		crate_scrolling_frequency,master_sampling_frequency,
		master_scrolling_callback_frequency,master_scrolling_frequency;
	int channel_number,*configured_channels,*configured_channel_offsets,
		different_configurations,i,j,k,master_number_of_samples_in_buffer,
		return_code,retry;
	struct Unemap_crate *crate;
	unsigned long crate_number_of_samples_in_buffer;

	ENTER(unemap_configure);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"enter unemap_configure.  %d\n",synchronization_card);
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if (((0==number_of_channels)||((0<number_of_channels)&&channel_numbers))&&
		(0<sampling_frequency)&&(0<number_of_samples_in_buffer)&&
		(!scrolling_callback||((0<scrolling_callback_frequency)&&
		(scrolling_frequency>=scrolling_callback_frequency))))
	{
		if (initialize_connection()&&module_unemap_crates&&
			(0<module_number_of_unemap_crates))
		{
			return_code=0;
			/* configure crates and if they end up with different sampling
				frequencies or buffer sizes retry with the minima */
			master_sampling_frequency=sampling_frequency;
			master_scrolling_frequency=scrolling_frequency;
			master_scrolling_callback_frequency=scrolling_callback_frequency;
			master_number_of_samples_in_buffer=number_of_samples_in_buffer;
			retry=0;
			do
			{
				/* distribute the channels among the crates */
				if (-1==number_of_channels)
				{
					crate=module_unemap_crates;
					for (i=module_number_of_unemap_crates;i>0;i--)
					{
						crate->number_of_configured_channels=0;
						crate++;
					}
					return_code=1;
				}
				else if (0==number_of_channels)
				{
					return_code=1;
					crate=module_unemap_crates;
					i=module_number_of_unemap_crates;
					k=0;
					while (return_code&&(i>0))
					{
						crate->number_of_configured_channels=crate->number_of_channels;
						ALLOCATE(crate->configured_channels,int,crate->number_of_channels);
						ALLOCATE(crate->configured_channel_offsets,int,
							crate->number_of_channels);
						if ((crate->configured_channels)&&
							(crate->configured_channel_offsets))
						{
							for (j=0;j<crate->number_of_channels;j++)
							{
								(crate->configured_channels)[j]=j+1;
								(crate->configured_channel_offsets)[j]=k;
								k++;
							}
							crate++;
							i--;
						}
						else
						{
							return_code=0;
							DEALLOCATE(crate->configured_channels);
							DEALLOCATE(crate->configured_channel_offsets);
						}
					}
					if (!return_code)
					{
						channel_number=0;
						while (i<module_number_of_unemap_crates)
						{
							crate--;
							i++;
							DEALLOCATE(crate->configured_channels);
							DEALLOCATE(crate->configured_channel_offsets);
						}
					}
				}
				else
				{
					return_code=1;
					crate=module_unemap_crates;
					for (i=module_number_of_unemap_crates;i>0;i--)
					{
						crate->number_of_configured_channels=0;
						crate++;
					}
					i=0;
					while (return_code&&(i<number_of_channels))
					{
						channel_number=channel_numbers[i];
						if (channel_number>0)
						{
							crate=module_unemap_crates;
							j=module_number_of_unemap_crates;
							while ((j>0)&&(channel_number>crate->number_of_channels))
							{
								channel_number -= crate->number_of_channels;
								crate++;
								j--;
							}
							if (j>0)
							{
								if (REALLOCATE(configured_channels,crate->configured_channels,int,
									(crate->number_of_configured_channels)+1))
								{
									crate->configured_channels=configured_channels;
								}
								if (REALLOCATE(configured_channel_offsets,
									crate->configured_channel_offsets,int,
									(crate->number_of_configured_channels)+1))
								{
									crate->configured_channel_offsets=configured_channel_offsets;
								}
								if (configured_channels&&configured_channel_offsets)
								{
									(crate->configured_channel_offsets)[
										crate->number_of_configured_channels]=i;
									(crate->configured_channels)[
										crate->number_of_configured_channels]=channel_number;
									(crate->number_of_configured_channels)++;
								}
								else
								{
									return_code=0;
								}
							}
							else
							{
								return_code=0;
							}
						}
						else
						{
							return_code=0;
						}
						i++;
					}
					if (!return_code)
					{
						crate=module_unemap_crates;
						for (i=module_number_of_unemap_crates;i>0;i--)
						{
							crate->number_of_configured_channels=0;
							DEALLOCATE(crate->configured_channels);
							DEALLOCATE(crate->configured_channel_offsets);
							crate++;
						}
					}
				}
				if (return_code)
				{
					crate=module_unemap_crates;
					i=module_number_of_unemap_crates;
					if (crate_configure_start(crate,0,master_sampling_frequency,
						master_number_of_samples_in_buffer,
#if defined (WIN32_USER_INTERFACE)
						scrolling_window,scrolling_message,
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (UNIX)
						event_dispatcher,
#endif /* defined (UNIX) */
						scrolling_callback,scrolling_callback_data,
						master_scrolling_frequency,master_scrolling_callback_frequency,
						synchronization_card))
					{
						do
						{
							crate++;
							i--;
						} while ((i>0)&&crate_configure_start(crate,1,
							master_sampling_frequency,master_number_of_samples_in_buffer,
#if defined (WIN32_USER_INTERFACE)
							scrolling_window,scrolling_message,
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (UNIX)
							event_dispatcher,
#endif /* defined (UNIX) */
							scrolling_callback,scrolling_callback_data,
							master_scrolling_frequency,master_scrolling_callback_frequency,
							synchronization_card));
					}
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"  start completed %d\n",i);
#endif /* defined (DEBUG) */
					if (i>0)
					{
						return_code=0;
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
						different_configurations=0;
						i=module_number_of_unemap_crates;
						crate=module_unemap_crates;
						i--;
						if (return_code=crate_configure_end(crate))
						{
							if (i>0)
							{
								if (crate_get_sampling_frequency(crate,
									&master_sampling_frequency)&&
									crate_get_maximum_number_of_samples(crate,
									&crate_number_of_samples_in_buffer)&&
									crate_get_scrolling_frequency(crate,
									&master_scrolling_frequency)&&
									crate_get_scrolling_callback_frequency(crate,
									&master_scrolling_callback_frequency))
								{
									master_number_of_samples_in_buffer=
										(int)crate_number_of_samples_in_buffer;
									do
									{
										crate++;
										i--;
										if (crate_configure_end(crate)&&
											crate_get_sampling_frequency(crate,
											&crate_sampling_frequency)&&
											crate_get_maximum_number_of_samples(crate,
											&crate_number_of_samples_in_buffer)&&
											crate_get_scrolling_frequency(crate,
											&crate_scrolling_frequency)&&
											crate_get_scrolling_callback_frequency(crate,
											&crate_scrolling_callback_frequency))
										{
											if ((master_sampling_frequency!=
												crate_sampling_frequency)||
												(master_number_of_samples_in_buffer!=
												(int)crate_number_of_samples_in_buffer)||
												(master_scrolling_frequency!=
												crate_scrolling_frequency)||
												(master_scrolling_callback_frequency!=
												crate_scrolling_callback_frequency))
											{
												different_configurations=1;
												if (retry)
												{
													return_code=0;
												}
												else
												{
													if (crate_sampling_frequency<
														master_sampling_frequency)
													{
														master_sampling_frequency=crate_sampling_frequency;
													}
													if ((int)crate_number_of_samples_in_buffer<
														master_number_of_samples_in_buffer)
													{
														master_number_of_samples_in_buffer=
															(int)crate_number_of_samples_in_buffer;
													}
												}
											}
										}
										else
										{
											return_code=0;
										}
									} while ((i>0)&&return_code);
									if (!retry&&different_configurations)
									{
										retry=1;
									}
									else
									{
										retry=0;
									}
								}
								else
								{
									return_code=0;
									retry=0;
								}
							}
							else
							{
								retry=0;
							}
						}
						else
						{
							retry=0;
						}
#if defined (DEBUG)
						/*???debug */
						display_message(INFORMATION_MESSAGE,
							"  end completed %d %d %d\n",i,return_code,retry);
#endif /* defined (DEBUG) */
						while (i>0)
						{
							crate++;
							i--;
							crate_configure_end(crate);
						}
						if (!return_code||retry)
						{
							crate=module_unemap_crates;
							for (i=module_number_of_unemap_crates;i>0;i--)
							{
								crate_deconfigure(crate);
								crate++;
							}
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"unemap_configure.  Invalid number_of_channels %d",
						number_of_channels);
				}
			} while (return_code&&retry);
			if (return_code)
			{
				return_code=1;
				module_number_of_configured_channels=0;
				crate=module_unemap_crates;
				for (i=module_number_of_unemap_crates;i>0;i--)
				{
					module_number_of_configured_channels +=
						crate->number_of_configured_channels;
					crate++;
				}
#if defined (CACHE_CLIENT_INFORMATION)
				get_cache_information();
#endif /* defined (CACHE_CLIENT_INFORMATION) */
			}
		}
#if defined (OLD_CODE)
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_configure.  Could not initialize_connection");
		}
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_configure.  Invalid argument(s).  %g %d %g %g",sampling_frequency,
			number_of_samples_in_buffer,scrolling_frequency,
			scrolling_callback_frequency);
		return_code=0;
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave unemap_configure.  %d\n",return_code);
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
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_configured.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
	LEAVE;

	return (return_code);
} /* unemap_configured */

int unemap_deconfigure(void)
/*******************************************************************************
LAST MODIFIED : 13 August 2003

DESCRIPTION :
Stops acquisition and signal generation.  Frees buffers associated with the
hardware.
==============================================================================*/
{
	int i,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_deconfigure);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter unemap_deconfigure\n");
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
#if defined (DEBUG)
		/*???debug */
		display_message(INFORMATION_MESSAGE,
			"unemap_deconfigure:close_crate_connection\n");
#endif /* defined (DEBUG) */
		close_connection();
	}
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_deconfigure.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
	module_number_of_configured_channels=0;
#if defined (CACHE_CLIENT_INFORMATION)
	module_number_of_stimulators=0;
	module_get_cache_information_failed=0;
#endif /* defined (CACHE_CLIENT_INFORMATION) */
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave unemap_deconfigure\n");
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
#if defined (OLD_CODE)
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_hardware_version.  Could not initialize_connection");
		}
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_hardware_version.  Missing hardware_version");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_hardware_version */

int unemap_get_software_version(int *software_version)
/*******************************************************************************
LAST MODIFIED : 2 June 2003

DESCRIPTION :
The function does not need the software to be configured.

Returns the minimum of the crate software versions.
==============================================================================*/
{
	int i,return_code,temp_software_version;
	struct Unemap_crate *crate;

	ENTER(unemap_get_software_version);
	return_code=0;
	/* check arguments */
	if (software_version)
	{
		if (initialize_connection()&&(crate=module_unemap_crates)&&
			(0<(i=module_number_of_unemap_crates)))
		{
			temp_software_version=crate->software_version;
			i--;
			while (0<i)
			{
				crate++;
				if (crate->software_version<temp_software_version)
				{
					temp_software_version=crate->software_version;
				}
				i--;
			}
			*software_version=temp_software_version;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_software_version.  Missing software_version");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_software_version */

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
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_shutdown.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_scrolling_channel.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_clear_scrolling_channels.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
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
	display_message(INFORMATION_MESSAGE,
		"enter unemap_start_scrolling\n");
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
			display_message(INFORMATION_MESSAGE,
				"%p %d %d\n",crate,(crate->scrolling).number_of_channels,
				(crate->scrolling).complete);
#endif /* defined (DEBUG) */
			crate++;
		}
	}
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_start_scrolling.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave unemap_start_scrolling\n");
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
	display_message(INFORMATION_MESSAGE,
		"enter unemap_stop_scrolling\n");
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
			display_message(INFORMATION_MESSAGE,
				"%p %d %d\n",crate,(crate->scrolling).number_of_channels,
				(crate->scrolling).complete);
#endif /* defined (DEBUG) */
			crate++;
		}
	}
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_stop_scrolling.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave unemap_stop_scrolling\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_stop_scrolling */

int unemap_calibrate(Unemap_calibration_end_callback *calibration_end_callback,
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
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_calibrate.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_start_sampling.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_stop_sampling.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
	LEAVE;

	return (return_code);
} /* unemap_stop_sampling */

int unemap_get_sampling(void)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

DESCRIPTION :
Returns a non-zero if unemap is sampling and zero otherwise.
==============================================================================*/
{
	int return_code,retval;
	long buffer_size,message_size;
	unsigned char buffer[2+sizeof(long)];

	ENTER(unemap_get_sampling);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&module_unemap_crates&&
		(INVALID_SOCKET!=module_unemap_crates->command_socket)&&
		(0<module_number_of_unemap_crates))
	{
		lock_mutex(module_unemap_crates->command_socket_mutex);
		buffer[0]=UNEMAP_GET_SAMPLING_CODE;
		buffer[1]=BIG_ENDIAN_CODE;
		buffer_size=2+sizeof(message_size);
		message_size=0;
		memcpy(buffer+2,&message_size,sizeof(message_size));
		retval=socket_send(module_unemap_crates->command_socket,buffer,buffer_size,
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
			retval=socket_recv(module_unemap_crates->command_socket,
#if defined (WIN32_USER_INTERFACE)
				(HANDLE)NULL,
#endif /* defined (WIN32_USER_INTERFACE) */
				buffer,2+sizeof(long),0);
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
					"unemap_get_sampling.  socket_recv() failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_sampling.  socket_send() failed");
		}
		unlock_mutex(module_unemap_crates->command_socket_mutex);
	}
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_sampling.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
	LEAVE;

	return (return_code);
} /* unemap_get_sampling */

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
	display_message(INFORMATION_MESSAGE,
		"enter unemap_set_isolate_record_mode\n");
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
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_isolate_record_mode.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave unemap_set_isolate_record_mode %d\n",return_code);
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
	display_message(INFORMATION_MESSAGE,
		"enter unemap_set_isolate_record_mode\n");
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
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_isolate_record_mode.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave unemap_set_isolate_record_mode %d\n",return_code);
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
#if defined (OLD_CODE)
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_isolate_record_mode.  Could not initialize_connection");
		}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
	"unemap_set_antialiasing_filter_frequency.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
"unemap_set_powerup_antialiasing_filter_frequency.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
		else
		{
			display_message(ERROR_MESSAGE,
	"unemap_get_antialiasing_filter_frequency.  Could not initialize_connection");
		}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_number_of_channels.  Could not initialize_connection");
		}
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_number_of_channels.  Missing number_of_channels");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_number_of_channels */

int unemap_get_number_of_configured_channels(int *number_of_channels)
/*******************************************************************************
LAST MODIFIED : 9 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

The number of configured channels is assigned to <*number_of_channels>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_number_of_configured_channels);
	return_code=0;
	/* check arguments */
	if (number_of_channels)
	{
		*number_of_channels=module_number_of_configured_channels;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_number_of_configured_channels.  Missing number_of_channels");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_number_of_configured_channels */

int unemap_get_sample_range(int channel_number,long int *minimum_sample_value,
	long int *maximum_sample_value)
/*******************************************************************************
LAST MODIFIED : 18 August 2003

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

The minimum possible sample value is assigned to <*minimum_sample_value> and the
maximum possible sample value is assigned to <*maximum_sample_value>.
==============================================================================*/
{
	int crate_channel_number,i,return_code;
#if defined (CACHE_CLIENT_INFORMATION)
	struct Unemap_card *unemap_card;
#endif /* defined (CACHE_CLIENT_INFORMATION) */
	struct Unemap_crate *crate;

	ENTER(unemap_get_sample_range);
	return_code=0;
	/* check arguments */
	if (minimum_sample_value&&maximum_sample_value)
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
						if (0<(unemap_card->channel_number))
						{
							*minimum_sample_value=unemap_card->minimum_sample_value;
							*maximum_sample_value=unemap_card->maximum_sample_value;
							return_code=1;
						}
					}
				}
				else
				{
#endif /* defined (CACHE_CLIENT_INFORMATION) */
					return_code=crate_get_sample_range(crate,crate_channel_number,
						minimum_sample_value,maximum_sample_value);
#if defined (CACHE_CLIENT_INFORMATION)
				}
#endif /* defined (CACHE_CLIENT_INFORMATION) */
			}
			else
			{
				display_message(ERROR_MESSAGE,"unemap_get_sample_range.  "
					"Invalid channel_number %d",channel_number);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"unemap_get_sample_range.  "
			"Missing minimum_sample_value or maximum_sample_value");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_sample_range */

int unemap_get_voltage_range(int channel_number,float *minimum_voltage,
	float *maximum_voltage)
/*******************************************************************************
LAST MODIFIED : 18 August 2003

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
						if (0<(unemap_card->channel_number))
						{
							*minimum_voltage=(unemap_card->minimum_voltage)/
								((unemap_card->pre_filter_gain)*
								(unemap_card->post_filter_gain));
							*maximum_voltage=(unemap_card->maximum_voltage)/
								((unemap_card->pre_filter_gain)*
								(unemap_card->post_filter_gain));
							return_code=1;
						}
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
			else
			{
				display_message(ERROR_MESSAGE,"unemap_get_voltage_range.  "
					"Invalid channel_number %d",channel_number);
			}
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
	display_message(INFORMATION_MESSAGE,
		"enter unemap_get_number_of_samples_acquired\n");
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
					display_message(INFORMATION_MESSAGE,
						"crate_number_of_samples %lu\n",crate_number_of_samples);
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
	display_message(INFORMATION_MESSAGE,
		"leave unemap_get_number_of_samples_acquired %d %lu\n",return_code,
		*number_of_samples);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_get_number_of_samples_acquired */

int unemap_transfer_samples_acquired(int channel_number,int number_of_samples,
	Unemap_transfer_samples_function *transfer_samples_function,
	void *transfer_samples_function_data,int *number_of_samples_transferred)
/*******************************************************************************
LAST MODIFIED : 13 August 2003

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is valid (between 1 and the total number of channels
inclusive, then the <samples> for that channel are transferred.  If
<channel_number> is 0 then the <samples> for all channels are transferred.
Otherwise the function fails.

If <number_of_samples> is not positive or greater than the number of samples
available, then the number of samples available are transferred.  Otherwise,
<number_of_samples> are transferred. If <number_of_samples_transferred> is not
NULL, then it is set to the number of samples transferred.

The <transfer_samples_function> is used to transfer the samples.  It is called
with samples (short int *), number_of_samples (int) and
<transfer_samples_function_data>.  It should return the number of samples
transferred.

???DB.  Should be able to get away without allocating a samples array, but this
	requires interleaving the crates and the initial application for this function
	is on the server side (unemap_hardware_service).  Once re-written, it can be
	used in unemap_get_samples_acquired.
==============================================================================*/
{
	int number_of_channels,number_of_samples_got,number_transferred,return_code;
	short *samples;
	unsigned long local_number_of_samples;

	ENTER(unemap_transfer_samples_acquired);
	return_code=0;
	number_transferred=0;
	/* check arguments */
	if (transfer_samples_function&&(0<=channel_number)&&
		(channel_number<=module_number_of_channels))
	{
		if (unemap_get_number_of_samples_acquired(&local_number_of_samples))
		{
			if (number_of_samples>0)
			{
				if ((unsigned long)number_of_samples<local_number_of_samples)
				{
					local_number_of_samples=(unsigned long)number_of_samples;
				}
			}
			if (0==channel_number)
			{
				if (unemap_get_number_of_configured_channels(&number_of_channels))
				{
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,"unemap_transfer_samples_acquired.  "
						"Could not get number_of_channels");
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
				"unemap_transfer_samples_acquired.  Could not get number_of_samples");
		}
		if (return_code)
		{
			if ((0<number_of_channels)&&(0<local_number_of_samples))
			{
				if (ALLOCATE(samples,short,number_of_channels*local_number_of_samples))
				{
					if (!unemap_get_samples_acquired(channel_number,number_of_samples,
						samples,&number_of_samples_got))
					{
						display_message(ERROR_MESSAGE,
							"unemap_transfer_samples_acquired.  Could not get samples");
						DEALLOCATE(samples);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"unemap_transfer_samples_acquired.  Could not allocate samples");
					return_code=0;
				}
			}
			else
			{
				samples=(short *)NULL;
				number_of_samples_got=0;
			}
			if (return_code)
			{
				number_transferred=(*transfer_samples_function)(samples,
					number_of_channels*number_of_samples_got,
					transfer_samples_function_data);
				if (number_of_channels*number_of_samples_got!=number_transferred)
				{
					display_message(ERROR_MESSAGE,
						"unemap_transfer_samples_acquired.  Error transferring samples");
					return_code=0;
				}
				if (0<number_of_channels)
				{
					number_transferred /= number_of_channels;
				}
				DEALLOCATE(samples);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_transfer_samples_acquired.  Invalid argument(s).  %p %d",
			transfer_samples_function,channel_number);
	}
	if (number_of_samples_transferred)
	{
		*number_of_samples_transferred=number_transferred;
	}
	LEAVE;

	return (return_code);
} /* unemap_transfer_samples_acquired */

int unemap_write_samples_acquired(int channel_number,int number_of_samples,
	FILE *file,int *number_of_samples_written)
/*******************************************************************************
LAST MODIFIED : 13 August 2003

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is valid (between 1 and the total number of channels
inclusive, then the <samples> for that channel are written to <file>.  If
<channel_number> is 0 then the <samples> for all channels are written to <file>.
Otherwise the function fails.

If <number_of_samples> is not positive or greater than the number of samples
available, then the number of samples available are written.  Otherwise,
<number_of_samples> are written. If <number_of_samples_written> is not NULL,
then it is set to the number of samples written.

Needed for the local hardware version.  Not needed in this version, but included
for completeness.
==============================================================================*/
{
	int number_of_channels,return_code;
	short *samples;
	unsigned long local_number_of_samples;

	ENTER(unemap_write_samples_acquired);
	return_code=0;
	/* check arguments */
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"unemap_write_samples_acquired.  %p %d %f\n",file,channel_number,
		module_number_of_channels);
#endif /* defined (DEBUG) */
	if (file&&(0<=channel_number)&&(channel_number<=module_number_of_channels))
	{
		if (unemap_get_number_of_samples_acquired(&local_number_of_samples))
		{
			if (number_of_samples>0)
			{
				if ((unsigned long)number_of_samples<local_number_of_samples)
				{
					local_number_of_samples=(unsigned long)number_of_samples;
				}
			}
			if (0==channel_number)
			{
				if (unemap_get_number_of_configured_channels(&number_of_channels))
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
			if ((0<number_of_channels)&&(0<local_number_of_samples))
			{
				if (ALLOCATE(samples,short,number_of_channels*local_number_of_samples))
				{
					if (!unemap_get_samples_acquired(channel_number,number_of_samples,
						samples,number_of_samples_written))
					{
						display_message(ERROR_MESSAGE,
							"unemap_write_samples_acquired.  Could not get samples");
						DEALLOCATE(samples);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"unemap_write_samples_acquired.  Could not allocate samples");
					return_code=0;
				}
			}
			else
			{
				*number_of_samples_written=0;
			}
			if (return_code)
			{
				fwrite((char *)&channel_number,sizeof(channel_number),1,file);
				fwrite((char *)&number_of_channels,sizeof(number_of_channels),1,file);
				fwrite((char *)&local_number_of_samples,
					sizeof(local_number_of_samples),1,file);
				if (samples)
				{
					fwrite((char *)samples,sizeof(short int),
						number_of_channels*local_number_of_samples,file);
					DEALLOCATE(samples);
				}
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

int unemap_get_samples_acquired(int channel_number,int number_of_samples,
	short int *samples,int *number_of_samples_got)
/*******************************************************************************
LAST MODIFIED : 13 August 2003

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is valid (between 1 and the total number of channels
inclusive), then the <samples> for that channel are returned.  If
<channel_number> is 0 then the <samples> for all channels are returned.
Otherwise the function fails.

If <number_of_samples> is not positive or greater than the number of samples
available, then the number of samples available are got.  Otherwise,
<number_of_samples> are got. If <number_of_samples_got> is not NULL, then it is
set to the number of samples got.
==============================================================================*/
{
	int i,crate_channel_number,crate_number_of_samples_got,
		local_number_of_samples_got,return_code;
	struct Unemap_crate *crate;

	ENTER(unemap_get_samples_acquired);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"enter unemap_get_samples_acquired\n");
#endif /* defined (DEBUG) */
	return_code=0;
	local_number_of_samples_got=0;
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
					crate,channel_number,number_of_samples,samples,
					&crate_number_of_samples_got)))
				{
					if (crate_number_of_samples_got>local_number_of_samples_got)
					{
						local_number_of_samples_got=crate_number_of_samples_got;
					}
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
						number_of_samples,samples,&local_number_of_samples_got);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_samples_acquired.  Missing samples");
	}
	if (number_of_samples_got)
	{
		*number_of_samples_got=local_number_of_samples_got;
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave unemap_get_samples_acquired\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_get_samples_acquired */

int unemap_get_samples_acquired_background(int channel_number,
	int number_of_samples,Unemap_acquired_data_callback *callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 29 August 2002

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
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"enter unemap_get_samples_acquired_background\n");
#endif /* defined (DEBUG) */
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
						crate_get_samples_acquired_background_start(crate,0,
						number_of_samples)))
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
							crate_channel_number,number_of_samples))
						{
							return_code=crate_get_samples_acquired_background_end(crate);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"unemap_get_samples_acquired_background.  "
							"Invalid channel_number %d",channel_number);
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
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave unemap_get_samples_acquired_background\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_get_samples_acquired_background */

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
#if defined (OLD_CODE)
		else
		{
			display_message(ERROR_MESSAGE,
			"unemap_get_maximum_number_of_samples.  Could not initialize_connection");
		}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_sampling_frequency.  Could not initialize_connection");
		}
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_sampling_frequency.  Missing frequency");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_sampling_frequency */

int unemap_get_scrolling_frequency(float *frequency)
/*******************************************************************************
LAST MODIFIED : 4 June 2003

DESCRIPTION :
The function fails if the hardware is not configured.

The scrolling frequency is assigned to <*frequency>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_scrolling_frequency);
	return_code=0;
	/* check arguments */
	if (frequency)
	{
		if (initialize_connection())
		{
			/* unemap_configure makes sure that all crates have the same scrolling
				frequency */
			return_code=crate_get_scrolling_frequency(module_unemap_crates,frequency);
		}
#if defined (OLD_CODE)
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_scrolling_frequency.  Could not initialize_connection");
		}
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_scrolling_frequency.  Missing frequency");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_scrolling_frequency */

int unemap_get_scrolling_callback_frequency(float *frequency)
/*******************************************************************************
LAST MODIFIED : 4 June 2003

DESCRIPTION :
The function fails if the hardware is not configured.

The scrolling frequency is assigned to <*frequency>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_scrolling_callback_frequency);
	return_code=0;
	/* check arguments */
	if (frequency)
	{
		if (initialize_connection())
		{
			/* unemap_configure makes sure that all crates have the same scrolling
				frequency */
			return_code=crate_get_scrolling_callback_frequency(module_unemap_crates,
				frequency);
		}
#if defined (OLD_CODE)
		else
		{
			display_message(ERROR_MESSAGE,"unemap_get_scrolling_callback_frequency.  "
				"Could not initialize_connection");
		}
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_scrolling_callback_frequency.  Missing frequency");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_scrolling_callback_frequency */

int unemap_set_gain(int channel_number,float pre_filter_gain,
	float post_filter_gain)
/*******************************************************************************
LAST MODIFIED : 16 July 2000

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
	int crate_channel_number,i,return_code,sampling_on;
	struct Unemap_crate *crate;

	ENTER(unemap_set_gain);
	return_code=0;
	/* unemap may have been started by another process, so can't just check that
		the command_socket is valid */
	if (initialize_connection()&&(crate=module_unemap_crates)&&
		(0<module_number_of_unemap_crates))
	{
		/* since sampling is controlled by the first crate, have to make sure that
			it is stopped for all crates while changing gain */
		if (sampling_on=unemap_get_sampling())
		{
			unemap_stop_sampling();
		}
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
		if (sampling_on)
		{
			unemap_start_sampling();
		}
	}
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_gain.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
	LEAVE;

	return (return_code);
} /* unemap_set_gain */

int unemap_get_gain(int channel_number,float *pre_filter_gain,
	float *post_filter_gain)
/*******************************************************************************
LAST MODIFIED : 18 August 2003

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
						if (0<(unemap_card->channel_number))
						{
							*pre_filter_gain=unemap_card->pre_filter_gain;
							*post_filter_gain=unemap_card->post_filter_gain;
							return_code=1;
						}
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
#if defined (OLD_CODE)
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_gain.  Could not initialize_connection");
		}
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_gain.  Missing pre_filter_gain or post_filter_gain");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_gain */

int unemap_load_voltage_stimulating(int number_of_channels,int *channel_numbers,
	int number_of_voltages,float voltages_per_second,float *voltages,
	unsigned int number_of_cycles,
	Unemap_stimulation_end_callback *stimulation_end_callback,
	void *stimulation_end_callback_data)
/*******************************************************************************
LAST MODIFIED : 2 June 2003

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

If <number_of_cycles> is zero then the waveform is repeated until
<unemap_stop_stimulating>, otherwise the waveform is repeated the
<number_of_cycles> times or until <unemap_stop_stimulating>.

The <stimulation_end_callback> is called with <stimulation_end_callback_data>
when the stimulation ends.

Use unemap_set_channel_stimulating to make a channel into a stimulating channel.
Use <unemap_start_stimulating> to start the stimulating.
==============================================================================*/
{
	int all_channels,*channel_number,crate_channel_number,*crate_channel_numbers,
		crate_number_of_channels,crate_stimulation_id,*crate_stimulation_ids,i,j,k,
		l,return_code;
	struct Stimulation_end_callback_info *stimulation_end_callback_info;
	struct Unemap_crate *crate;

	ENTER(unemap_load_voltage_stimulating);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"enter unemap_load_voltage_stimulating\n");
#endif /* defined (DEBUG) */
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
			if (ALLOCATE(crate_stimulation_ids,int,module_number_of_unemap_crates))
			{
				if (all_channels)
				{
					return_code=1;
					for (i=0;i<module_number_of_unemap_crates;i++)
					{
						if (crate_load_voltage_stimulating(crate,0,(int *)NULL,
							number_of_voltages,voltages_per_second,voltages,number_of_cycles))
						{
							crate_stimulation_id=0;
							if (crate->software_version>=3)
							{
								/* has stimulation socket */
								k=1;
								for (l=((crate->number_of_channels)-1)/
									NUMBER_OF_CHANNELS_ON_NI_CARD;l>=0;l--)
								{
									crate_stimulation_id |= k;
									k *= 2;
								}
							}
							crate_stimulation_ids[i]=crate_stimulation_id;
						}
						else
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
						for (i=0;i<module_number_of_unemap_crates;i++)
						{
							crate_number_of_channels=0;
							crate_stimulation_id=0;
							for (j=0;j<number_of_channels;j++)
							{
								if ((crate_channel_number<channel_numbers[j])&&
									(channel_numbers[j]<=crate_channel_number+
									(crate->number_of_channels)))
								{
									crate_channel_numbers[crate_number_of_channels]=
										channel_numbers[j]-crate_channel_number;
									if (crate->software_version>=3)
									{
										/* has stimulation socket */
										k=1;
										for (l=(crate_channel_numbers[crate_number_of_channels]-1)/
											NUMBER_OF_CHANNELS_ON_NI_CARD;l>0;l--)
										{
											k *= 2;
										}
										crate_stimulation_id |= k;
									}
									crate_number_of_channels++;
								}
							}
							crate_stimulation_ids[i]=crate_stimulation_id;
							if (0<crate_number_of_channels)
							{
								if (!crate_load_voltage_stimulating(crate,
									crate_number_of_channels,crate_channel_numbers,
									number_of_voltages,voltages_per_second,voltages,
									number_of_cycles))
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
						display_message(ERROR_MESSAGE,"unemap_load_voltage_stimulating.  "
							"Could not allocate crate_channel_numbers");
					}
				}
				if (return_code&&module_stimulation_end_callback_info_list&&
					stimulation_end_callback)
				{
					/* check for no stimulation sockets */
					i=0;
					while ((i<module_number_of_unemap_crates)&&
						(0==crate_stimulation_ids[i]))
					{
						i++;
					}
					if (i<module_number_of_unemap_crates)
					{
						if (stimulation_end_callback_info=
							CREATE(Stimulation_end_callback_info)(crate_stimulation_ids,
							stimulation_end_callback,stimulation_end_callback_data))
						{
							REMOVE_OBJECTS_FROM_LIST_THAT(Stimulation_end_callback_info)(
								Stimulation_end_callback_info_no_cards,
								(void *)stimulation_end_callback_info,
								module_stimulation_end_callback_info_list);
							if (!ADD_OBJECT_TO_LIST(Stimulation_end_callback_info)(
								stimulation_end_callback_info,
								module_stimulation_end_callback_info_list))
							{
								DESTROY(Stimulation_end_callback_info)(
									&stimulation_end_callback_info);
								display_message(ERROR_MESSAGE,
									"unemap_load_voltage_stimulating.  "
									"Could not add stimulation_end_callback_info");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"unemap_load_voltage_stimulating.  "
								"Could not create stimulation_end_callback_info");
						}
					}
				}
				DEALLOCATE(crate_stimulation_ids);
			}
			else
			{
				display_message(ERROR_MESSAGE,"unemap_load_voltage_stimulating.  "
					"Could not allocate crate_stimulation_ids");
			}
		}
#if defined (OLD_CODE)
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_load_voltage_stimulating.  Could not initialize_connection");
		}
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"unemap_load_voltage_stimulating.  "
			"Invalid number_of_channels (%d) or channel_numbers (%p) or "
			"number_of_voltages (%d) or voltages (%p) or voltages_per_second (%g) or "
			"return_code (%d)",number_of_channels,channel_numbers,number_of_voltages,
			voltages,voltages_per_second,return_code);
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave unemap_load_voltage_stimulating\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_load_voltage_stimulating */

int unemap_load_current_stimulating(int number_of_channels,int *channel_numbers,
	int number_of_currents,float currents_per_second,float *currents,
	unsigned int number_of_cycles,
	Unemap_stimulation_end_callback *stimulation_end_callback,
	void *stimulation_end_callback_data)
/*******************************************************************************
LAST MODIFIED : 2 June 2003

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

If <number_of_cycles> is zero then the waveform is repeated until
<unemap_stop_stimulating>, otherwise the waveform is repeated the
<number_of_cycles> times or until <unemap_stop_stimulating>.

The <stimulation_end_callback> is called with <stimulation_end_callback_data>
when the stimulation ends.

Use unemap_set_channel_stimulating to make a channel into a stimulating channel.
Use <unemap_start_stimulating> to start the stimulating.
==============================================================================*/
{
	int all_channels,*channel_number,crate_channel_number,*crate_channel_numbers,
		crate_number_of_channels,crate_stimulation_id,*crate_stimulation_ids,i,j,k,
		l,return_code;
	struct Stimulation_end_callback_info *stimulation_end_callback_info;
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
			if (ALLOCATE(crate_stimulation_ids,int,module_number_of_unemap_crates))
			{
				if (all_channels)
				{
					return_code=1;
					for (i=0;i<module_number_of_unemap_crates;i++)
					{
						if (crate_load_current_stimulating(crate,0,(int *)NULL,
							number_of_currents,currents_per_second,currents,number_of_cycles))
						{
							crate_stimulation_id=0;
							if (crate->software_version>=3)
							{
								/* has stimulation socket */
								k=1;
								for (l=((crate->number_of_channels)-1)/
									NUMBER_OF_CHANNELS_ON_NI_CARD;l>=0;l--)
								{
									crate_stimulation_id |= k;
									k *= 2;
								}
							}
							crate_stimulation_ids[i]=crate_stimulation_id;
						}
						else
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
						for (i=0;i<module_number_of_unemap_crates;i++)
						{
							crate_number_of_channels=0;
							crate_stimulation_id=0;
							for (j=0;j<number_of_channels;j++)
							{
								if ((crate_channel_number<channel_numbers[j])&&
									(channel_numbers[j]<=crate_channel_number+
									(crate->number_of_channels)))
								{
									crate_channel_numbers[crate_number_of_channels]=
										channel_numbers[j]-crate_channel_number;
									if (crate->software_version>=3)
									{
										/* has stimulation socket */
										k=1;
										for (l=(crate_channel_numbers[crate_number_of_channels]-1)/
											NUMBER_OF_CHANNELS_ON_NI_CARD;l>0;l--)
										{
											k *= 2;
										}
										crate_stimulation_id |= k;
									}
									crate_number_of_channels++;
								}
							}
							if (0<crate_number_of_channels)
							{
								if (!crate_load_current_stimulating(crate,
									crate_number_of_channels,crate_channel_numbers,
									number_of_currents,currents_per_second,currents,
									number_of_cycles))
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
						display_message(ERROR_MESSAGE,"unemap_load_current_stimulating.  "
							"Could not allocate crate_channel_numbers");
					}
				}
				if (return_code&&module_stimulation_end_callback_info_list&&
					stimulation_end_callback)
				{
					/* check for no stimulation sockets */
					i=0;
					while ((j<module_number_of_unemap_crates)&&
						(0==crate_stimulation_ids[i]))
					{
						i++;
					}
					if (i==module_number_of_unemap_crates)
					{
						if (stimulation_end_callback_info=
							CREATE(Stimulation_end_callback_info)(crate_stimulation_ids,
							stimulation_end_callback,stimulation_end_callback_data))
						{
							REMOVE_OBJECTS_FROM_LIST_THAT(Stimulation_end_callback_info)(
								Stimulation_end_callback_info_no_cards,
								(void *)stimulation_end_callback_info,
								module_stimulation_end_callback_info_list);
							if (!ADD_OBJECT_TO_LIST(Stimulation_end_callback_info)(
								stimulation_end_callback_info,
								module_stimulation_end_callback_info_list))
							{
								DESTROY(Stimulation_end_callback_info)(
									&stimulation_end_callback_info);
								display_message(ERROR_MESSAGE,
									"unemap_load_current_stimulating.  "
									"Could not add stimulation_end_callback_info");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"unemap_load_current_stimulating.  "
								"Could not create stimulation_end_callback_info");
						}
					}
				}
				DEALLOCATE(crate_stimulation_ids);
			}
			else
			{
				display_message(ERROR_MESSAGE,"unemap_load_current_stimulating.  "
					"Could not allocate crate_stimulation_ids");
			}
		}
#if defined (OLD_CODE)
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_load_current_stimulating.  Could not initialize_connection");
		}
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"unemap_load_current_stimulating.  "
			"Invalid number_of_channels (%d) or channel_numbers (%p) or "
			"number_of_currents (%d) or currents (%p)",number_of_channels,
			channel_numbers,number_of_currents,currents);
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
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_start_voltage_stimulating.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_start_voltage_stimulating.  Could not initialize_connection");
		}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_start_current_stimulating.  Could not initialize_connection");
		}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_stop_stimulating.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_channel_stimulating.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_channel_stimulating.  Could not initialize_connection");
		}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_start_calibrating.  Could not initialize_connection");
		}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_stop_calibrating.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_power.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_power.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_power.  Could not initialize_connection");
		}
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"unemap_get_power.  Missing on");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_power */

int unemap_read_waveform_file(FILE *in_file,char *waveform_file_name,
	int *number_of_values,float *values_per_second,float **values,
	int *constant_voltage)
/*******************************************************************************
LAST MODIFIED : 7 January 2000

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

	ENTER(unemap_read_waveform_file);
	return_code=0;
	/* check arguments */
	if (((in_file&&!waveform_file_name)||(!in_file&&waveform_file_name))&&
		number_of_values&&values&&values_per_second&&constant_voltage)
	{
		if (in_file)
		{
			waveform_file=in_file;
		}
		else
		{
			waveform_file=fopen(waveform_file_name,"r");
		}
		if (waveform_file)
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
						"unemap_read_waveform_file.  Could not allocate memory for values");
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
			if (!in_file)
			{
				fclose(waveform_file);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Could not open waveform file %s",
				waveform_file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_read_waveform_file.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* unemap_read_waveform_file */

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
#if defined (OLD_CODE)
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_number_of_stimulators.  Could not initialize_connection");
			}
#endif /* defined (OLD_CODE) */
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
LAST MODIFIED : 17 August 2003

DESCRIPTION :
The function does not need the hardware to be configured.

Returns non-zero if <stimulator_number> can stimulate down <channel_number>.
==============================================================================*/
{
	int crate_channel_number,crate_stimulator_number,i,return_code;
	struct Unemap_crate *crate;
#if !defined (CACHE_CLIENT_INFORMATION)
	int crate_number_of_stimulators;
#endif /* !defined (CACHE_CLIENT_INFORMATION) */

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
		return_code=1;
		while (return_code&&(i>0)&&crate&&
			(crate_channel_number>crate->number_of_channels))
		{
			crate_channel_number -= crate->number_of_channels;
#if defined (CACHE_CLIENT_INFORMATION)
			crate_stimulator_number -= crate->number_of_stimulators;
#else /* defined (CACHE_CLIENT_INFORMATION) */
			if ((return_code=crate_get_number_of_stimulators_start(crate))&&
				(return_code=crate_get_number_of_stimulators_end(crate,
				&crate_number_of_stimulators)))
			{
				crate_stimulator_number -= crate_number_of_stimulators;
			}
#endif /* defined (CACHE_CLIENT_INFORMATION) */
			i--;
			crate++;
		}
		if (return_code&&(i>0)&&crate)
		{
			return_code=0;
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
		else
		{
			return_code=0;
		}
	}
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_channel_valid_for_stimulator.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_card_state.  Could not initialize_connection");
		}
#endif /* defined (OLD_CODE) */
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
#if defined (OLD_CODE)
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_toggle_shift_register.  Could not initialize_connection");
	}
#endif /* defined (OLD_CODE) */
	LEAVE;

	return (return_code);
} /* unemap_toggle_shift_register */
