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

/*#define USE_ACPI*/

/*#define TEST_SOCKETS 1*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>

#include "service.h"


#if !defined (TEST_SOCKETS)
// internal variables
SERVICE_STATUS          ssStatus;       // current status of the service
SERVICE_STATUS_HANDLE   sshStatusHandle;
DWORD                   dwErr = 0;
BOOL                    bDebug = FALSE;
TCHAR                   szErr[256];

// internal function prototypes
#if defined (USE_ACPI)
DWORD WINAPI service_ctrl(DWORD dwCtrlCode,DWORD dwEventType,LPVOID lpEventData,
	LPVOID lpContext);
#else /* defined (USE_ACPI) */
VOID WINAPI service_ctrl(DWORD dwCtrlCode);
#endif /* defined (USE_ACPI) */
VOID WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv);
VOID CmdInstallService();
VOID CmdRemoveService();
VOID CmdDebugService(int argc, char **argv);
BOOL WINAPI ControlHandler ( DWORD dwCtrlType );
LPTSTR GetLastErrorText( LPTSTR lpszBuf, DWORD dwSize );

/*???DB.  unemap hardware specific start */
/* to add a version to the displayed name */
int unemap_get_software_version(int *software_version);

#define SERVICE_DISPLAY_NAME_LENGTH 120
char service_display_name[SERVICE_DISPLAY_NAME_LENGTH];
/*???DB.  unemap hardware specific end */
#if defined (UNEMAP_HARDWARE_SPECIFIC)
#endif /* defined (UNEMAP_HARDWARE_SPECIFIC) */

/*???DB.  unemap hardware specific start */
FILE *fopen_UNEMAP_HARDWARE(char *file_name,char *type);

VOID AddToMessageLog(LPTSTR lpszMsg)
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"%s\n",lpszMsg);
		fclose(unemap_debug);
	}
	else
	{
	    TCHAR   szMsg[256];
		HANDLE  hEventSource;
		LPTSTR  lpszStrings[2];

 /*      dwErr = GetLastError();*/

        // Use event logging to log the error.
        //
        hEventSource = RegisterEventSource(NULL, TEXT(SZSERVICENAME));
 
 /*       _stprintf(szMsg, TEXT("%s error: %d"), TEXT(SZSERVICENAME), dwErr);*/
        lpszStrings[0] = /*szMsg;
        lpszStrings[1] = */lpszMsg;

        if (hEventSource != NULL) {
            ReportEvent(hEventSource, // handle of event source
                EVENTLOG_ERROR_TYPE,  // event type
                0,                    // event category
                0,                    // event ID
                NULL,                 // current user's SID
                1,                    // strings in lpszStrings
                0,                    // no bytes of raw data
                lpszStrings,          // array of error strings
                NULL);                // no raw data

            (VOID) DeregisterEventSource(hEventSource);
        }
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
	int software_version;

	software_version=0;
	unemap_get_software_version(&software_version);
	sprintf(service_display_name,"Unemap Hardware Service. v%d. %s",
		software_version,__DATE__);
	/*???DB.  unemap hardware specific end */
#if defined (UNEMAP_HARDWARE_SPECIFIC)
#endif /* defined (UNEMAP_HARDWARE_SPECIFIC) */

#if defined (UNEMAP_HARDWARE_SPECIFIC)
	/*???DB.  unemap hardware specific start */
	/* clear the error log */
	{
		FILE *unemap_debug;

		if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","w"))
		{
			fclose(unemap_debug);
		}
	}
	/*???DB.  unemap hardware specific end */
#endif /* defined (UNEMAP_HARDWARE_SPECIFIC) */
#if defined (DEBUG)
	/*???debug */
	AddToMessageLog(TEXT("enter main"));
#endif /* defined (DEBUG) */
    if ( (argc > 1) &&
         ((*argv[1] == '-') || (*argv[1] == '/')) )
    {
        if ( _stricmp( "install", argv[1]+1 ) == 0 )
        {
#if defined (DEBUG)
 	/*???debug */
	AddToMessageLog(TEXT("install"));
#endif /* defined (DEBUG) */
           CmdInstallService();
        }
        else if ( _stricmp( "remove", argv[1]+1 ) == 0 )
        {
#if defined (DEBUG)
	/*???debug */
	AddToMessageLog(TEXT("remove"));
#endif /* defined (DEBUG) */
            CmdRemoveService();
        }
        else if ( _stricmp( "debug", argv[1]+1 ) == 0 )
        {
#if defined (DEBUG)
	/*???debug */
	AddToMessageLog(TEXT("debug"));
#endif /* defined (DEBUG) */
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
#if defined (DEBUG)
	/*???debug */
	AddToMessageLog(TEXT("dispatch"));
#endif /* defined (DEBUG) */
        // this is just to be friendly
        printf( "%s -install          to install the service\n", SZAPPNAME );
        printf( "%s -remove           to remove the service\n", SZAPPNAME );
        printf( "%s -debug <params>   to run as a console app for debugging\n", SZAPPNAME );
        printf( "\nStartServiceCtrlDispatcher being called.\n" );
        printf( "This may take several seconds.  Please wait.\n" );

#if defined (DEBUG)
	/*???debug */
	AddToMessageLog(TEXT("StartServiceCtrlDispatcher"));
#endif /* defined (DEBUG) */
        if (!StartServiceCtrlDispatcher(dispatchTable))
            AddToMessageLog(TEXT("StartServiceCtrlDispatcher failed."));
#if defined (DEBUG)
	/*???debug */
	AddToMessageLog(TEXT("leave main"));
#endif /* defined (DEBUG) */
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

#if defined (DEBUG)
	/*???debug */
	AddToMessageLog(TEXT("enter service_main"));
#endif /* defined (DEBUG) */

    // register our service control handler:
    //
#if defined (USE_ACPI)
    sshStatusHandle = RegisterServiceCtrlHandlerEx( TEXT(SZSERVICENAME), service_ctrl, (LPVOID)NULL);
#else /* defined (USE_ACPI) */
    sshStatusHandle = RegisterServiceCtrlHandler( TEXT(SZSERVICENAME), service_ctrl);
#endif /* defined (USE_ACPI) */

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


#if defined (DEBUG)
		/* rename the error log */
		rename_UNEMAP_HARDWARE("unemap.deb","unemapsv.deb");
#endif /* defined (DEBUG) */
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
#if defined (USE_ACPI)
DWORD WINAPI service_ctrl(DWORD dwCtrlCode,DWORD dwEventType,LPVOID lpEventData,
	LPVOID lpContext)
#else /* defined (USE_ACPI) */
VOID WINAPI service_ctrl(DWORD dwCtrlCode)
#endif /* defined (USE_ACPI) */
{
#if defined (DEBUG)
	/*???debug */
	AddToMessageLog(TEXT("enter service_ctrl"));
#endif /* defined (DEBUG) */
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
#if defined (DEBUG)
	/*???debug */
	AddToMessageLog(TEXT("SERVICE_CONTROL_STOP|SERVICE_CONTROL_SHUTDOWN"));
#endif /* defined (DEBUG) */
            ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 0);
            ServiceStop();
#if defined (USE_ACPI)
            return (NO_ERROR);
#else /* defined (USE_ACPI) */
            return;
#endif /* defined (USE_ACPI) */

        // Update the service status.
        //
        case SERVICE_CONTROL_INTERROGATE:
#if defined (DEBUG)
	/*???debug */
	AddToMessageLog(TEXT("SERVICE_CONTROL_INTERROGATE"));
#endif /* defined (DEBUG) */
            break;

#if defined (USE_ACPI)
				case SERVICE_CONTROL_POWEREVENT:
#if defined (DEBUG)
	/*???debug */
  AddToMessageLog(TEXT("SERVICE_CONTROL_POWEREVENT"));
#endif /* defined (DEBUG) */
						ReportStatusToSCMgr(ssStatus.dwCurrentState, NO_ERROR, 0);
						return (ERROR_CONTINUE);
#endif /* defined (USE_ACPI) */
        // invalid control code
        //
        default:
#if defined (DEBUG)
	/*???debug */
  AddToMessageLog(TEXT("default"));
#endif /* defined (DEBUG) */
            break;

    }

    ReportStatusToSCMgr(ssStatus.dwCurrentState, NO_ERROR, 0);
#if defined (USE_ACPI)
		return (NO_ERROR);
#endif /* defined (USE_ACPI) */
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
#if defined (USE_ACPI)
							|SERVICE_ACCEPT_POWEREVENT
#endif /* defined (USE_ACPI) */
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
        _tprintf(TEXT("Unable to install %s - %s\n"), TEXT(service_display_name), GetLastErrorText(szErr, 256));
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
            TEXT(service_display_name), // name to display
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
            _tprintf(TEXT("%s installed.\n"), TEXT(service_display_name) );
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

/*???DB.  unemap hardware specific start */
		DWORD display_name_length;
		TCHAR display_name[SERVICE_DISPLAY_NAME_LENGTH];
/*???DB.  unemap hardware specific end */
#if defined (UNEMAP_HARDWARE_SPECIFIC)
#endif /* defined (UNEMAP_HARDWARE_SPECIFIC) */

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
/*???DB.  unemap hardware specific start */
					display_name_length=SERVICE_DISPLAY_NAME_LENGTH;
					GetServiceDisplayName(schService,TEXT(SZSERVICENAME),display_name,
						&display_name_length);
/*???DB.  unemap hardware specific end */
#if defined (UNEMAP_HARDWARE_SPECIFIC)
#endif /* defined (UNEMAP_HARDWARE_SPECIFIC) */
            // try to stop the service
            if ( ControlService( schService, SERVICE_CONTROL_STOP, &ssStatus ) )
            {
                _tprintf(TEXT("Stopping %s."), TEXT(display_name));
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
                    _tprintf(TEXT("\n%s stopped.\n"), TEXT(display_name) );
                else
                    _tprintf(TEXT("\n%s failed to stop.\n"), TEXT(display_name) );

            }

            // now remove the service
            if( DeleteService(schService) )
                _tprintf(TEXT("%s removed.\n"), TEXT(display_name) );
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

    _tprintf(TEXT("Debugging %s.\n"), TEXT(service_display_name));

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
            _tprintf(TEXT("Stopping %s.\n"), TEXT(service_display_name));
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
