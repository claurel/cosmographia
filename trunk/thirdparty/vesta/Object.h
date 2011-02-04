/*
 * $Revision: 504 $ $Date: 2010-09-13 16:25:03 -0700 (Mon, 13 Sep 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_OBJECT_H_
#define _VESTA_OBJECT_H_

// Set to 1 when using atomic increment and decrement operations for
// thread safety. Currently disabled while the code for atomic operations
// are being tested with the GNU and Microsoft C++ compilers.
#define USE_ATOMICS 0

#include "CountedPtr.h"
#if USE_ATOMICS
#include "internal/AtomicInt.h"
#endif
#include <cassert>

/** Object is the base class for all complex classes (i.e. not plain-old-data)
  * in VESTA. It implements the reference counting methods addRef() and release()
  * so that it can be used with VESTA's counted_ptr smart pointer template class.
  */
namespace vesta
{

class Object
{
public:
    Object() :
        m_refCount(0)
    {
    }

    virtual ~Object()
    {
    }

public:
    /** Add a reference to this object.
      */
    int addRef()
    {
        return ++m_refCount;
    }

    /** Remove a reference to this object. The object is deleted if the
      * reference count reaches zero.
      */
    int release()
    {
        assert(refCount() > 0);

        if (--m_refCount == 0)
        {
            delete this;
            return 0;
        }
        else
        {
            return refCount();
        }
    }

    /** Return the current number of references to this object.
      */
    int refCount() const
    {
#if USE_ATOMICS
        return m_refCount.value();
#else
        return m_refCount;
#endif
    }

private:
#if USE_ATOMICS
    mutable AtomicInt m_refCount;
#else
    mutable int m_refCount;
#endif
};

}

#endif // _VESTA_OBJECT_H_
