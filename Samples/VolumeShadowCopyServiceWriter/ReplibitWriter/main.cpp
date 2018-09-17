/*
**++
**
** Copyright (c) 2018 eFolder Inc
**
**
** Module Name:
**
**	main.cpp
**
*/

///////////////////////////////////////////////////////////////////////////////
// Includes

#include "stdafx.h"
#include "main.h"
#include "ReplibitWriter.h"
#include <vld.h>

///////////////////////////////////////////////////////////////////////////////
// Declarations

HANDLE g_quitEvent = NULL;
BOOL g_bStop = FALSE;

///////////////////////////////////////////////////////////////////////////////

extern "C" int __cdecl wmain(__in int argc, __in_ecount(argc) wchar_t **) {
    UNREFERENCED_PARAMETER(argc);

    static const WCHAR pwcBuildDate[] = TEXT(__DATE__ " " __TIME__);
    _tprintf(TEXT("%s %s\r\n"), pwcWriterName, pwcBuildDate);

    HRESULT hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        _tprintf(TEXT("CoInitializeEx failed. (0x%08lx)\r\n"), GetLastError());
        return 1;
    }

    hr = ::CoInitializeSecurity(NULL,                           // IN PSECURITY_DESCRIPTOR        pSecDesc,
                                -1,                             // IN LONG                        cAuthSvc,
                                NULL,                           // IN SOLE_AUTHENTICATION_SERVICE *asAuthSvc,
                                NULL,                           // IN void                        *pReserved1,
                                RPC_C_AUTHN_LEVEL_PKT_PRIVACY,  // IN DWORD                       dwAuthnLevel,
                                RPC_C_IMP_LEVEL_IDENTIFY,       // IN DWORD                       dwImpLevel,
                                NULL,                           // IN void                        *pAuthList,
                                EOAC_NONE,                      // IN DWORD                       dwCapabilities,
                                NULL                            // IN void                        *pReserved3
    );
    if (FAILED(hr)) {
        _tprintf(TEXT("CoInitializeSecurity failed. (0x%08lx)\r\n"), GetLastError());
        return 1;
    }

    g_quitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_quitEvent == NULL) {
        _tprintf(TEXT("CreateEvent failed. (0x%08lx)\r\n"), GetLastError());
        return 1;
    }

    // set a control handler that allows the writer to be shut down
    if (!::SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        _tprintf(TEXT("SetConsoleSecurityHandler failed. (0x%08lx)\r\n"), GetLastError());
        return 1;
    }

    CReplibitWriter::StaticInitialize();

    // We want the writer to go out of scope before the return statement
    {
        CReplibitWriter writer;
        hr = writer.Initialize();
        if (FAILED(hr)) {
            _tprintf(TEXT("Writer init failed. (0x%08lx)\r\n"), GetLastError());
            return 1;
        }

        if (::WaitForSingleObject(g_quitEvent, INFINITE) != WAIT_OBJECT_0) {
            _tprintf(TEXT("WaitForSingleObject failed. (0x%08lx)\r\n"), GetLastError());
            return 1;
        }
        writer.Uninitialize();
    }

    return 0;
}

BOOL WINAPI ConsoleHandler(DWORD dwSignal) {
    if (dwSignal == CTRL_BREAK_EVENT) {
        _tprintf(TEXT("\nCTRL_BREAK_EVENT received \n"));
        g_bStop = TRUE;
        ::SetEvent(g_quitEvent);
        return TRUE;
    }
    return FALSE;
}
