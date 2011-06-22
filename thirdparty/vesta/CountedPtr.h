/*
 * $Revision: 432 $ $Date: 2010-08-16 09:05:19 -0700 (Mon, 16 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_COUNTED_PTR_H_
#define _VESTA_COUNTED_PTR_H_

namespace vesta
{

/** counted_ptr is a template class for intrusive reference counted smart pointers.
  * The class T must implement two methods: addRef() to increment the reference count
  * and release() to decrement it.
  */
template <class T> class counted_ptr
{
public:
    typedef T element_type;

    explicit counted_ptr(T* p = 0) :
        m_p(p)
    {
        if (m_p)
        {
            m_p->addRef();
        }
    }

    ~counted_ptr()
    {
        if (m_p)
        {
            m_p->release();
        }
    }

    counted_ptr(const counted_ptr& cp) :
        m_p(cp.m_p)
    {
        if (m_p)
        {
            m_p->addRef();
        }
    }

    counted_ptr& operator=(T* p)
    {
        if (m_p != p)
        {
            if (m_p)
            {
                m_p->release();
            }
            m_p = p;
            if (m_p)
            {
                m_p->addRef();
            }
        }

        return *this;
    }

    counted_ptr& operator=(const counted_ptr& cp)
    {
        if (this != &cp)
        {
            if (m_p)
            {
                m_p->release();
            }
            m_p = cp.m_p;
            if (m_p)
            {
                m_p->addRef();
            }
        }

        return *this;
    }

    bool operator==(const counted_ptr& other)
    {
        return this->m_p == other.m_p;
    }

    bool operator!=(const counted_ptr& other)
    {
        return this->m_p != other.m_p;
    }

    bool operator<(const counted_ptr& other)
    {
        return this->m_p < other.m_p;
    }

    bool operator>(const counted_ptr& other)
    {
        return this->m_p > other.m_p;
    }

    bool operator<=(const counted_ptr& other)
    {
        return this->m_p <= other.m_p;
    }

    bool operator>=(const counted_ptr& other)
    {
        return this->m_p >= other.m_p;
    }

    T& operator*() const
    {
        return *m_p;
    }

    T* operator->() const
    {
        return m_p;
    }

    bool isNull() const
    {
        return !m_p;
    }

    bool isValid() const
    {
        return m_p != 0;
    }

    T* ptr() const
    {
        return m_p;
    }

private:
    T* m_p;
};

}

#endif // _VESTA_COUNTED_PTR_H_
