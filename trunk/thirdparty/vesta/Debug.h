/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_DEBUG_H_
#define _VESTA_DEBUG_H_

namespace vesta
{
    void VESTA_LOG(const char* format, ...);
    void VESTA_WARNING(const char* format, ...);
}

#endif // _VESTA_DEBUG_H_
