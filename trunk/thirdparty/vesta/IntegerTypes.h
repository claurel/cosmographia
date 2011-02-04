/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_INTEGER_TYPES_H_
#define _VESTA_INTEGER_TYPES_H_

#ifndef _MSC_VER
#include <stdint.h>
#endif

namespace vesta
{
    /** Unsigned integer with 8 bits */
    typedef unsigned char v_uint8;

    /** Signed integer with 8 bits */
    typedef signed char v_int8;

    /** Unsigned integer with 16 bits */
    typedef unsigned short v_uint16;

    /** Signed integer with 16 bits */
    typedef short v_int16;

    /** Unsigned integer with 32 bits */
    typedef unsigned int v_uint32;

    /** Signed integer with 32 bits */
    typedef int v_int32;

#ifdef _MSC_VER
    typedef __int64 v_int64;
    typedef unsigned __int64 v_uint64;
#else
    typedef int64_t v_int64;
    typedef uint64_t v_uint64;
#endif
}

#endif // _VESTA_INTEGER_TYPES_H_
