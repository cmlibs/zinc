// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 1993-1996  Microsoft Corporation.  All Rights Reserved.
//
//  MODULE:   service.c
//
//  PURPOSE:  Implements functions required by all services
//            windows.
//
//  FUNCTIONS:
//    main(int argc, char **argv);
//    service_ctrl(DWORD dwCtrlCode);
//    service_main(DWORD dwArgc, LPTSTR *lpszArgv);
//    CmdInstallService();
//    CmdRemoveService();
//    CmdDebugService(int argc, char **argv);
//    ControlHandler ( DWORD dwCtrlType );
//    GetLastErrorText( LPTSTR lpszBuf, DWORD dwSize );
//
//  COMMENTS:
//
//  AUTHOR: Craig Link - Microsoft Developer Support
//

/*#define TEST_SOCKETS 1*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>

#include "service.h"
/*???DB.  unemap hardware specific start */
#include "unemap/unemap_hardware.h"
#include "user_interface/message.h"
/*???DB.  unemap hardware specific end */
#if defined (UNEMAP_HARDWARE_SPECIFIC)
#endif /* defined (UNEMAP_HARDWARE_SPECIFIC) */



#if !defined (TEST_SOCKETS)
// internal variables
SERVICE_STATUS          ssStatus;       // current status of the service
SERVICE_STATUS_HANDLE   sshStatusHandle;
DWORD                   dwErr = 0;
BOOL                    bDebug = FALSE;
TCHAR                   szErr[256];

// internal function prototypes
VOID WINAPI service_ctrl(DWORD dwCtrlCode);
VOID WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv);
VOID CmdInstallService();
VOID CmdRemoveService();
VOID CmdDebugService(int argc, char **argv);
BOOL WINAPI ControlHandler ( DWORD dwCtrlType );
LPTSTR GetLastErrorText( LPTSTR lpszBuf, DWORD dwSize );

/*???DB.  unemap hardware specific start */
static FILE *fopen_UNEMAP_HARDWARE(char *file_name,char *type)
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
#if defined (WIN32)
				if ('\\'!=hardware_file_name[strlen(hardware_file_name)-1])
				{
					strcat(hardware_file_name,"\\");
				}
#else /* defined (WIN32) */
				if ('/'!=hardware_file_name[strlen(hardware_file_name)-1])
				{
					strcat(hardware_file_name,"/");
				}
#endif /* defined (WIN32) */
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

VOID AddToMessageLog(LPTSTR lpszMsg)
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"%s\n",lpszMsg);
		fclose(unemap_debug);
	}
} /* AddToMessageLog */
/*???DB.  unemap hardware specific end */
#if defined (UNEMAP_HARDWARE_SPECIFIC)
#endif /* defined (UNEMAP_HARDWARE_SPECIFIC) */

//
//  FUNCTION: main
//
//  PURPOSE: entrypoint for service
//
//  PARAMETERS:
//    argc - number of command line arguments
//    argv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    main() either performs the command line task, or
//    call StartServiceCtrlDispatcher to register the
//    main service thread.  When the this call returns,
//    the service has stopped, so exit.
//
void _CRTAPI1 main(int argc, char **argv)
{
    SERVICE_TABLE_ENTRY dispatchTable[] =
    {
        { TEXT(SZSERVICENAME), (LPSERVICE_MAIN_FUNCTION)service_main },
        { NULL, NULL }
    };
	/*???DB.  unemap hardware specific start */
	unsigned long number_of_channels;

	/*???debug */
	/* clear the error log */
	{
		FILE *unemap_debug;

		if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","w"))
		{
			fclose(unemap_debug);
		}
	}
	/* set up messages */
	set_display_message_function(ERROR_MESSAGE,display_error_message,
		(void *)NULL);
	set_display_message_function(INFORMATION_MESSAGE,display_information_message,
		(void *)NULL);
	set_display_message_function(WARNING_MESSAGE,display_warning_message,
		(void *)NULL);
	/* when the NIDAQ DLL is loaded (when the service starts), the NI cards and
		hence the unemap cards are in an unknown state.  This call to
		unemap_get_number_of_channels forces a call to search_for_NI_cards which in
		turn configures the NI cards */
	unemap_get_number_of_channels(&number_of_channels);
		/*???DB.  unemap hardware specific end */
#if defined (UNEMAP_HARDWARE_SPECIFIC)
#endif /* defined (UNEMAP_HARDWARE_SPECIFIC) */
    if ( (argc > 1) &&
         ((*argv[1] == '-') || (*argv[1] == '/')) )
    {
        if ( _stricmp( "install", argv[1]+1 ) == 0 )
        {
            CmdInstallService();
        }
        else if ( _stricmp( "remove", argv[1]+1 ) == 0 )
        {
            CmdRemoveService();
        }
        else if ( _stricmp( "debug", argv[1]+1 ) == 0 )
        {
            bDebug = TRUE;
            CmdDebugService(argc, argv);
        }
        else
        {
            goto dispatch;
        }
        exit(0);
    }

    // if it doesn't match any of the above parameters
    // the service control manager may be starting the service
    // so we must call StartServiceCtrlDispatcher
    dispatch:
        // this is just to be friendly
        printf( "%s -install          to install the service\n", SZAPPNAME );
        printf( "%s -remove           to remove the service\n", SZAPPNAME );
        printf( "%s -debug <params>   to run as a console app for debugging\n", SZAPPNAME );
        printf( "\nStartServiceCtrlDispatcher being called.\n" );
        printf( "This may take several seconds.  Please wait.\n" );

        if (!StartServiceCtrlDispatcher(dispatchTable))
            AddToMessageLog(TEXT("StartServiceCtrlDispatcher failed."));
}



//
//  FUNCTION: service_main
//
//  PURPOSE: To perform actual initialization of the service
//
//  PARAMETERS:
//    dwArgc   - number of command line arguments
//    lpszArgv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    This routine performs the service initialization and then calls
//    the user defined ServiceStart() routine to perform majority
//    of the work.
//
void WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv)
{

    // register our service control handler:
    //
    sshStatusHandle = RegisterServiceCtrlHandler( TEXT(SZSERVICENAME), service_ctrl);

    if (!sshStatusHandle)
        goto cleanup;

    // SERVICE_STATUS members that don't change in example
    //
    ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ssStatus.dwServiceSpecificExitCode = 0;


    // report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state
        NO_ERROR,              // exit code
        3000))                 // wait hint
        goto cleanup;


    ServiceStart( dwArgc, lpszArgv );

cleanup:

    // try to report the stopped status to the service control manager.
    //
    if (sshStatusHandle)
        (VOID)ReportStatusToSCMgr(
                            SERVICE_STOPPED,
                            dwErr,
                            0);

    return;
}



//
//  FUNCTION: service_ctrl
//
//  PURPOSE: This function is called by the SCM whenever
//           ControlService() is called on this service.
//
//  PARAMETERS:
//    dwCtrlCode - type of control requested
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
VOID WINAPI service_ctrl(DWORD dwCtrlCode)
{
    // Handle the requested control code.
    //
    switch(dwCtrlCode)
    {
        // Stop the service.
        //
        // SERVICE_STOP_PENDING should be reported before
        // setting the Stop Event - hServerStopEvent - in
        // ServiceStop().  This avoids a race condition
        // which may result in a 1053 - The Service did not respond...
        // error.
        case SERVICE_CONTROL_STOP:
/*???DB.  unemap hardware specific start */
        case SERVICE_CONTROL_SHUTDOWN:
/*???DB.  unemap hardware specific end */
#if defined (UNEMAP_HARDWARE_SPECIFIC)
#endif /* defined (UNEMAP_HARDWARE_SPECIFIC) */
            ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 0);
            ServiceStop();
            return;

        // Update the service status.
        //
        case SERVICE_CONTROL_INTERROGATE:
            break;

        // invalid control code
        //
        default:
            break;

    }

    ReportStatusToSCMgr(ssStatus.dwCurrentState, NO_ERROR, 0);
}
#endif /* !defined (TEST_SOCKETS) */



//
//  FUNCTION: ReportStatusToSCMgr()
//
//  PURPOSE: Sets the current status of the service and
//           reports it to the Service Control Manager
//
//  PARAMETERS:
//    dwCurrentState - the state of the service
//    dwWin32ExitCode - error code to report
//    dwWaitHint - worst case estimate to next checkpoint
//
//  RETURN VALUE:
//    TRUE  - success
//    FALSE - failure
//
//  COMMENTS:
//
BOOL ReportStatusToSCMgr(DWORD dwCurrentState,
                         DWORD dwWin32ExitCode,
                         DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;
    BOOL fResult = TRUE;


#if !defined (TEST_SOCKETS)
    if ( !bDebug ) // when debugging we don't report to the SCM
    {
        if (dwCurrentState == SERVICE_START_PENDING)
            ssStatus.dwControlsAccepted = 0;
        else
            ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP
/*???DB.  unemap hardware specific start */
							|SERVICE_ACCEPT_SHUTDOWN
/*???DB.  unemap hardware specific end */
#if defined (UNEMAP_HARDWARE_SPECIFIC)
#endif /* defined (UNEMAP_HARDWARE_SPECIFIC) */
							;

        ssStatus.dwCurrentState = dwCurrentState;
        ssStatus.dwWin32ExitCode = dwWin32ExitCode;
        ssStatus.dwWaitHint = dwWaitHint;

        if ( ( dwCurrentState == SERVICE_RUNNING ) ||
             ( dwCurrentState == SERVICE_STOPPED ) )
            ssStatus.dwCheckPoint = 0;
        else
            ssStatus.dwCheckPoint = dwCheckPoint++;


        // Report the status of the service to the service control manager.
        //
        if (!(fResult = SetServiceStatus( sshStatusHandle, &ssStatus))) {
            AddToMessageLog(TEXT("SetServiceStatus"));
        }
    }
#endif /* !defined (TEST_SOCKETS) */
    return fResult;
}


#if defined (OLD_CODE)
/*???DB.  Moved to unemap_hardware_service */
//
//  FUNCTION: AddToMessageLog(LPTSTR lpszMsg)
//
//  PURPOSE: Allows any thread to log an error message
//
//  PARAMETERS:
//    lpszMsg - text for message
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
VOID AddToMessageLog(LPTSTR lpszMsg)
{
#if defined (TEST_SOCKETS)
	printf("%s\n",lpszMsg);
#else /* defined (TEST_SOCKETS) */
#if defined (OLD_CODE)
    TCHAR   szMsg[256];
    HANDLE  hEventSource;
    LPTSTR  lpszStrings[2];
#endif /* defined (OLD_CODE) */


/*???debug */
{
	FILE *debug_file;

	if (debug_file=fopen("d:\\bullivan\\test\\service.deb","a"))
	{
		fprintf(debug_file,"%s\n",lpszMsg);

#if defined (OLD_CODE)
    if ( !bDebug )
    {
        dwErr = GetLastError();

        // Use event logging to log the error.
        //
        hEventSource = RegisterEventSource(NULL, TEXT(SZSERVICENAME));

        _stprintf(szMsg, TEXT("%s error: %d"), TEXT(SZSERVICENAME), dwErr);
        lpszStrings[0] = szMsg;
        lpszStrings[1] = lpszMsg;

        if (hEventSource != NULL) {
            ReportEvent(hEventSource, // handle of event source
                EVENTLOG_ERROR_TYPE,  // event type
                0,                    // event category
                0,                    // event ID
                NULL,                 // current user's SID
                2,                    // strings in lpszStrings
                0,                    // no bytes of raw data
                lpszStrings,          // array of error strings
                NULL);                // no raw data

            (VOID) DeregisterEventSource(hEventSource);
        }
    }
#endif /* defined (OLD_CODE) */
/*???debug */
		fclose(debug_file);
	}
}
#endif /* defined (TEST_SOCKETS) */
}
#endif /* defined (OLD_CODE) */




#if !defined (TEST_SOCKETS)
///////////////////////////////////////////////////////////////////
//
//  The following code handles service installation and removal
//


//
//  FUNCTION: CmdInstallService()
//
//  PURPOSE: Installs the service
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
void CmdInstallService()
{
    SC_HANDLE   schService;
    SC_HANDLE   schSCManager;

    TCHAR szPath[512];

    if ( GetModuleFileName( NULL, szPath, 512 ) == 0 )
    {
        _tprintf(TEXT("Unable to install %s - %s\n"), TEXT(SZSERVICEDISPLAYNAME), GetLastErrorText(szErr, 256));
        return;
    }

    schSCManager = OpenSCManager(
                        NULL,                   // machine (NULL == local)
                        NULL,                   // database (NULL == default)
                        SC_MANAGER_ALL_ACCESS   // access required
                        );
    if ( schSCManager )
    {
        schService = CreateService(
            schSCManager,               // SCManager database
            TEXT(SZSERVICENAME),        // name of service
            TEXT(SZSERVICEDISPLAYNAME), // name to display
            SERVICE_ALL_ACCESS,         // desired access
            SERVICE_WIN32_OWN_PROCESS,  // service type
            SERVICE_AUTO_START,         // start type
            SERVICE_ERROR_NORMAL,       // error control type
            szPath,                     // service's binary
            NULL,                       // no load ordering group
            NULL,                       // no tag identifier
            TEXT(SZDEPENDENCIES),       // dependencies
            NULL,                       // LocalSystem account
            NULL);                      // no password

        if ( schService )
        {
            _tprintf(TEXT("%s installed.\n"), TEXT(SZSERVICEDISPLAYNAME) );
            CloseServiceHandle(schService);
        }
        else
        {
            _tprintf(TEXT("CreateService failed - %s\n"), GetLastErrorText(szErr, 256));
        }

        CloseServiceHandle(schSCManager);
    }
    else
        _tprintf(TEXT("OpenSCManager failed - %s\n"), GetLastErrorText(szErr,256));
}



//
//  FUNCTION: CmdRemoveService()
//
//  PURPOSE: Stops and removes the service
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
void CmdRemoveService()
{
    SC_HANDLE   schService;
    SC_HANDLE   schSCManager;

    schSCManager = OpenSCManager(
                        NULL,                   // machine (NULL == local)
                        NULL,                   // database (NULL == default)
                        SC_MANAGER_ALL_ACCESS   // access required
                        );
    if ( schSCManager )
    {
        schService = OpenService(schSCManager, TEXT(SZSERVICENAME), SERVICE_ALL_ACCESS);

        if (schService)
        {
            // try to stop the service
            if ( ControlService( schService, SERVICE_CONTROL_STOP, &ssStatus ) )
            {
                _tprintf(TEXT("Stopping %s."), TEXT(SZSERVICEDISPLAYNAME));
                Sleep( 1000 );

                while( QueryServiceStatus( schService, &ssStatus ) )
                {
                    if ( ssStatus.dwCurrentState == SERVICE_STOP_PENDING )
                    {
                        _tprintf(TEXT("."));
                        Sleep( 1000 );
                    }
                    else
                        break;
                }

                if ( ssStatus.dwCurrentState == SERVICE_STOPPED )
                    _tprintf(TEXT("\n%s stopped.\n"), TEXT(SZSERVICEDISPLAYNAME) );
                else
                    _tprintf(TEXT("\n%s failed to stop.\n"), TEXT(SZSERVICEDISPLAYNAME) );

            }

            // now remove the service
            if( DeleteService(schService) )
                _tprintf(TEXT("%s removed.\n"), TEXT(SZSERVICEDISPLAYNAME) );
            else
                _tprintf(TEXT("DeleteService failed - %s\n"), GetLastErrorText(szErr,256));


            CloseServiceHandle(schService);
        }
        else
            _tprintf(TEXT("OpenService failed - %s\n"), GetLastErrorText(szErr,256));

        CloseServiceHandle(schSCManager);
    }
    else
        _tprintf(TEXT("OpenSCManager failed - %s\n"), GetLastErrorText(szErr,256));
}




///////////////////////////////////////////////////////////////////
//
//  The following code is for running the service as a console app
//


//
//  FUNCTION: CmdDebugService(int argc, char ** argv)
//
//  PURPOSE: Runs the service as a console application
//
//  PARAMETERS:
//    argc - number of command line arguments
//    argv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
void CmdDebugService(int argc, char ** argv)
{
    DWORD dwArgc;
    LPTSTR *lpszArgv;

#ifdef UNICODE
    lpszArgv = CommandLineToArgvW(GetCommandLineW(), &(dwArgc) );
#else
    dwArgc   = (DWORD) argc;
    lpszArgv = argv;
#endif

    _tprintf(TEXT("Debugging %s.\n"), TEXT(SZSERVICEDISPLAYNAME));

    SetConsoleCtrlHandler( ControlHandler, TRUE );

    ServiceStart( dwArgc, lpszArgv );
}


//
//  FUNCTION: ControlHandler ( DWORD dwCtrlType )
//
//  PURPOSE: Handled console control events
//
//  PARAMETERS:
//    dwCtrlType - type of control event
//
//  RETURN VALUE:
//    True - handled
//    False - unhandled
//
//  COMMENTS:
//
BOOL WINAPI ControlHandler ( DWORD dwCtrlType )
{
    switch( dwCtrlType )
    {
        case CTRL_BREAK_EVENT:  // use Ctrl+C or Ctrl+Break to simulate
        case CTRL_C_EVENT:      // SERVICE_CONTROL_STOP in debug mode
            _tprintf(TEXT("Stopping %s.\n"), TEXT(SZSERVICEDISPLAYNAME));
            ServiceStop();
            return TRUE;
            break;

    }
    return FALSE;
}

//
//  FUNCTION: GetLastErrorText
//
//  PURPOSE: copies error message text to string
//
//  PARAMETERS:
//    lpszBuf - destination buffer
//    dwSize - size of buffer
//
//  RETURN VALUE:
//    destination buffer
//
//  COMMENTS:
//
LPTSTR GetLastErrorText( LPTSTR lpszBuf, DWORD dwSize )
{
    DWORD dwRet;
    LPTSTR lpszTemp = NULL;

    dwRet = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           NULL,
                           GetLastError(),
                           LANG_NEUTRAL,
                           (LPTSTR)&lpszTemp,
                           0,
                           NULL );

    // supplied buffer is not long enough
    if ( !dwRet || ( (long)dwSize < (long)dwRet+14 ) )
        lpszBuf[0] = TEXT('\0');
    else
    {
        lpszTemp[lstrlen(lpszTemp)-2] = TEXT('\0');  //remove cr and newline character
        _stprintf( lpszBuf, TEXT("%s (0x%x)"), lpszTemp, GetLastError() );
    }

    if ( lpszTemp )
        LocalFree((HLOCAL) lpszTemp );

    return lpszBuf;
}
#endif /* !defined (TEST_SOCKETS) */
