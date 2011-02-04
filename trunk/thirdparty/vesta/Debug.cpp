/*
 * $Revision: 333 $ $Date: 2010-07-07 17:47:06 -0700 (Wed, 07 Jul 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "Debug.h"
#include <cstdio>
#include <cstdarg>

#ifdef _WIN32
//#define _WIN32_WINNT 0x0501
#include <windows.h>
#endif

using namespace vesta;
using namespace std;

#ifdef _WIN32

// Enabled because IsDebuggerPresent isn't returning true on
static const bool AlwaysOutputToDebugger = true;

static void Win32DebugPrint(const char* format, va_list args)
{
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer) - 1, format, args);

    // Make sure that the string is null terminated even if we ran out of
    // space in the buffer.
    buffer[sizeof(buffer) - 1] = '\0';

    OutputDebugStringA(buffer);
    OutputDebugStringA("\n");
}

static bool DebuggerAttached()
{
    if (IsDebuggerPresent() == TRUE)
    {
        return true;
    }

    BOOL remotePresent = FALSE;

    // Disabled for now; requires _WIN32_WINNT to be 0x0501 or greater
    // CheckRemoteDebuggerPresent(GetCurrentProcess(), &remotePresent);

    return remotePresent == TRUE;
}

#endif

/** Emit an error message to Vesta's log
  */
void vesta::VESTA_LOG(const char* format, ...)
{
    va_list args;
    va_start(args, format);

#ifdef _WIN32
    if (DebuggerAttached() || AlwaysOutputToDebugger)
    {
        Win32DebugPrint(format, args);
    }
    else
#endif
    {
        // By default, write to standard out
        vprintf(format, args);
        printf("\n");
        fflush(stdout);
    }
}


/** Emit an error message to Vesta's warning log
  */
void vesta::VESTA_WARNING(const char* format, ...)
{
    va_list args;
    va_start(args, format);

#ifdef _WIN32
    if (DebuggerAttached() || AlwaysOutputToDebugger)
    {
        Win32DebugPrint(format, args);
    }
    else
#endif
    {
        // By default, write to standard error
        vfprintf(stderr, format, args);
        fprintf(stderr, "\n");
    }
}
