// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef WINVER
#undef WINVER
#endif

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#ifdef EFSVSS_2008
#define WINVER _WIN32_WINNT_VISTA  
#define _WIN32_WINNT _WIN32_WINNT_VISTA  
#elif EFSVSS_2008_R2
#define WINVER _WIN32_WINNT_WIN8  
#define _WIN32_WINNT _WIN32_WINNT_WIN8  
#else
#error("Need to specify EFSVSS_2008 or EFSVSS_2008_R2")
// Check: Project properties -> C/C++ -> Preprocessor -> Preprocessor definitions
#endif

#pragma warning( disable: 4091 )

#define VSS_SERVER

// General includes
#include <windows.h>
#include <winbase.h>

// _ASSERTE declaration (used by ATL) and otehr macros
#include "macros.h"



#include <iostream>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be explicit

// ATL includes
#pragma warning( disable: 4189 )    // disable local variable is initialized but not referenced
#include <atlbase.h>

// STL includes
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <fstream>
using namespace std;   

// Used for safe string manipulation
#include <strsafe.h>

#include "shadow.h"

