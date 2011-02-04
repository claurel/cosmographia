/*
 * $Revision: 432 $ $Date: 2010-08-16 09:05:19 -0700 (Mon, 16 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_ATOMIC_INT_H_
#define _VESTA_ATOMIC_INT_H_

#if defined(__GNUC__)
#define USE_GCC_ATOMIC_INTRINSICS 1
#elif defined(_MSC_VER)
#define USE_MSVC_ATOMIC_INTRINSICS 0
#else
// Microsoft VC++ and g++ are the only supported compilers right now
#error Unsupported compiler!
#endif

#if USE_MSVC_ATOMIC_INTRINSICS
#include <intrin.h>
#endif


namespace vesta
{

/**  Wrapper around an integer that implements atomic (thread safe and lock free)
  *  read/modify/write operations.
  */
class AtomicInt
{
public:
    AtomicInt(unsigned int value) :
        m_value(value)
    {
    }

    /** Atomic pre-increment.
      */
    inline int operator++()
    {
#if USE_GCC_ATOMIC_INTRINSICS
        return __sync_add_and_fetch(&m_value, 1);
#elif USE_MSVC_ATOMIC_INTRINSICS
        return _InterlockedIncrement(&m_value);
#else
        // NOT THREAD SAFE
        return ++m_value;
#endif
    }

    /** Atomic pre-decrement.
      */
    inline int operator--()
    {
#if USE_GCC_ATOMIC_INTRINSICS
        return __sync_sub_and_fetch(&m_value, 1);
#elif USE_MSVC_ATOMIC_INTRINSICS
        return _InterlockedDecrement(&m_value);
#else
        // NOT THREAD SAFE
        return --m_value;
#endif
    }

    int value() const
    {
        return m_value;
    }

private:
    volatile int m_value;
};

}

#endif // _VESTA_ATOMIC_INT_H_
