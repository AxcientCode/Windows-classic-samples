#pragma once

// Including SDKDDKVer.h defines the highest available Windows platform.

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.

#ifdef __cplusplus
#define ECHO_CONFIG
#endif

// Global warning disable defines
//#define _CRT_SECURE_NO_WARNINGS
//#define _CRT_NONSTDC_NO_WARNINGS
//#define _CRT_SECURE_NO_DEPRECATE
//#define _CRT_NON_CONFORMING_WCSTOK
//#define NO_WARN_MBCS_MFC_DEPRECATION
//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#define EFS_GETVERSION_DEPRECATED_NO_WARNINGS

// Global warning disables. Provide justifications.
// warning C4091: 'typedef ': ignored on left of 'XYZ' when no variable is declared - only appears in SDK or third part code
// warning C4206: nonstandard extension used: translation unit is empty
// warning C4456: declaration of 'err' hides previous local declaration - we initially have to assume that this behavior works
// warning C4457 : declaration of 'nIndex' hides function parameter - we initially have to assume that this behavior works
// warning C4702: unreachable code - most (but not all) of these are from try/catch blocks trying to catch SEH exceptions and not
// C++ exceptions. We currently compile with EHsc which won't catch SEH exceptions. We might change this to EHa but we need to
// research the ramifications.
//#pragma warning(disable : 4091 4206 4456 4457 4702)

#include <SDKDDKVer.h>

//Server 2003 SP2, 2003 R2 SP2 and XP Pro x64 SP2
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT  _WIN32_WINNT_WS03

#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif
#define NTDDI_VERSION NTDDI_WS03SP2

// _WIN32_WINDOWS really only applies to the old Win 9x family. However declare it
// the same as the _WIN32_WINNT just in case it's used somewhere.
#define _WIN32_WINDOWS _WIN32_WINNT

#ifdef ECHO_CONFIG

#if (_WIN32_WINNT <= _WIN32_WINNT_NT4)
#pragma message("Targeting Windows NT 4.0 or later")
#elif (_WIN32_WINNT <= _WIN32_WINNT_WIN2K)
#pragma message("Targeting Windows 2000 or later")
#elif (_WIN32_WINNT <= _WIN32_WINNT_WINXP)
#pragma message("Targeting Windows XP or later")
#elif (_WIN32_WINNT <= _WIN32_WINNT_WS03)
#pragma message("Targeting Windows XP x64/Windows Server 2003 or later")
#elif (_WIN32_WINNT <= _WIN32_WINNT_VISTA)
#pragma message("Targeting Windows Vista or later")
#elif (_WIN32_WINNT <= _WIN32_WINNT_WIN7)
#pragma message("Targeting Windows 7 or later")
#elif (_WIN32_WINNT <= _WIN32_WINNT_WIN8)
#pragma message("Targeting Windows 8 or later")
#elif (_WIN32_WINNT <= _WIN32_WINNT_WINBLUE)
#pragma message("Targeting Windows 8.1 or later")
#elif (_WIN32_WINNT <= 0x0A00)
#pragma message("Targeting Windows 10 or later")
#else
#pragma message("Targeting unknown modern Windows version. Update " __FILE__)
#endif

#if (_WIN32_IE <= _WIN32_IE_IE20)
#pragma message("Targeting Internet Explorer 2.0 or later")
#elif (_WIN32_IE <= _WIN32_IE_IE30)
#pragma message("Targeting Internet Explorer 3.0 or later")
#elif (_WIN32_IE <= _WIN32_IE_IE302)
#pragma message("Targeting Internet Explorer 3.02 or later")
#elif (_WIN32_IE <= _WIN32_IE_IE40)
#pragma message("Targeting Internet Explorer 4.0 or later")
#elif (_WIN32_IE <= _WIN32_IE_IE401)
#pragma message("Targeting Internet Explorer 4.01 or later")
#elif (_WIN32_IE <= _WIN32_IE_IE50)
#pragma message("Targeting Internet Explorer 5 or later")
#elif (_WIN32_IE <= _WIN32_IE_IE501)
#pragma message("Targeting Internet Explorer 5.01 or later")
#elif (_WIN32_IE <= _WIN32_IE_IE55)
#pragma message("Targeting Internet Explorer 5.5 or later")
#elif (_WIN32_IE <= _WIN32_IE_IE60)
#pragma message("Targeting Internet Explorer 6 or later")
#elif (_WIN32_IE <= _WIN32_IE_IE60SP1)
#pragma message("Targeting Internet Explorer 6.1 or later")
#elif (_WIN32_IE <= _WIN32_IE_IE60SP2)
#pragma message("Targeting Internet Explorer 6.2 or later")
#elif (_WIN32_IE <= _WIN32_IE_IE70)
#pragma message("Targeting Internet Explorer 7 or later")
#elif (_WIN32_IE <= _WIN32_IE_IE80)
#pragma message("Targeting Internet Explorer 8 or later")
#elif (_WIN32_IE <= _WIN32_IE_IE90)
#pragma message("Targeting Internet Explorer 9 or later")
#elif (_WIN32_IE <= _WIN32_IE_IE100)
#pragma message("Targeting Internet Explorer 10 or later")
#else
#pragma message("Targeting Internet Explorer 11")
#endif

#if (_MSC_VER < 1200)
#pragma message("Where did you find this compiler?")
#elif (_MSC_VER == 1200)
#pragma message("Microsoft C++ version 6 (VC 6)")
#elif (_MSC_VER == 1300)
#pragma message("Microsoft C++ version 7 (VC.NET 2002)")
#elif (_MSC_VER == 1310)
#pragma message("Microsoft C++ version 7.1 (VC.NET 2003)")
#elif (_MSC_VER == 1400)
#pragma message("Microsoft C++ version 8 (VC 2005)")
#elif (_MSC_VER == 1500)
#pragma message("Microsoft C++ version 9 (VC 2008)")
#elif (_MSC_VER == 1600)
#pragma message("Microsoft C++ version 10 (VC 2010)")
#elif (_MSC_VER == 1700)
#pragma message("Microsoft C++ version 11 (VC 2012)")
#elif (_MSC_VER == 1800)
#pragma message("Microsoft C++ version 12 (VC 2013)")
#elif (_MSC_VER == 1900)
#pragma message("Microsoft C++ version 14 (VC 2015)")
#elif (_MSC_VER == 1910)
#pragma message("Microsoft C++ version 14.1 (VC 2017)")
#else
#pragma message("Targeting unknown modern Windows version. Update " __FILE__)
#endif

#endif //ECHO_CONFIG

#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// As of the build tools upgrade VS2015 & .NET 4.0 the earliest
// possible supported Windows versions are:
// Windows XP, SP3
// Windows XP64, SP2
// Windows Server 2003 & 2003 R2, SP2
// What this means is that there is no 95/98/ME support and all
// versions are running VER_PLATFORM_WIN32_NT instead of VER_PLATFORM_WIN32_WINDOWS
#define PLATFORM_WIN32_NT
