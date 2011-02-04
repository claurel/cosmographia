/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "InputDataStream.h"
#include <iostream>
#include <sstream>
#include <cassert>

using namespace vesta;
using namespace std;


// Unions used for loading and converting binary data
union FloatBytes
{
    float f;
    char bytes[4];
};

union Int16Bytes
{
    v_int16 i;
    char bytes[2];
};

union Uint16Bytes
{
    v_uint16 i;
    char bytes[2];
};

union Int32Bytes
{
    v_int32 i;
    char bytes[4];
};

union Uint32Bytes
{
    v_uint32 i;
    char bytes[4];
};
// End unions


/** Wrap a C++ istream with an InputDataStream. The InputDataStream does not
  * take ownership of the istream, so the caller must take care that the istream
  * isn't destroyed before the InputDataStream.
  */
InputDataStream::InputDataStream(istream& in) :
    m_in(&in),
    m_byteOrder(nativeByteOrder()),
    m_nativeByteOrder(nativeByteOrder()),
    m_ownStream(false)
{
}


/** Wrap a C++ string with an InputDataStream.
  */
InputDataStream::InputDataStream(const std::string& s) :
    m_in(new istringstream(s, istringstream::in | istringstream::binary)),
    m_byteOrder(nativeByteOrder()),
    m_nativeByteOrder(nativeByteOrder()),
    m_ownStream(true)
{
}


/** Destructor. */
InputDataStream::~InputDataStream()
{
    if (m_ownStream)
    {
        delete m_in;
    }
}


/** Get the status of the stream. It will be one of three values:
  *   Good        - the stream is OK
  *   EndOfFile   - a read past the end of the file was attempted
  *   StreamError - an error other than EOF occurred while reading the file
  */
InputDataStream::StreamStatus
InputDataStream::status() const
{
    if (m_in->eof())
    {
        return EndOfFile;
    }
    else if (m_in->good())
    {
        return Good;
    }
    else
    {
        return StreamError;
    }
}


/** Read an unsigned byte from the stream.
  */
v_uint8
InputDataStream::readUbyte()
{
    return static_cast<v_uint8>(m_in->get());
}


/** Read a signed byte from the stream.
  */
v_int8
InputDataStream::readByte()
{
    return static_cast<v_int8>(m_in->get());
}


/** Read an IEEE754 single precision floating point number from the stream.
  */
float
InputDataStream::readFloat()
{
    FloatBytes data;
    m_in->read(data.bytes, 4);
    if (m_byteOrder != m_nativeByteOrder)
    {
        swap(data.bytes[0], data.bytes[3]);
        swap(data.bytes[1], data.bytes[2]);
    }

    return data.f;
}


/** Read a 32-bit unsigned integer.
  */
v_uint32
InputDataStream::readUint32()
{
    Uint32Bytes data;
    m_in->read(data.bytes, 4);
    if (m_byteOrder != m_nativeByteOrder)
    {
        swap(data.bytes[0], data.bytes[3]);
        swap(data.bytes[1], data.bytes[2]);
    }

    return data.i;
}


/** Read a 32-bit signed integer.
  */
v_int32
InputDataStream::readInt32()
{
    Int32Bytes data;
    m_in->read(data.bytes, 4);
    if (m_byteOrder != m_nativeByteOrder)
    {
        swap(data.bytes[0], data.bytes[3]);
        swap(data.bytes[1], data.bytes[2]);
    }

    return data.i;
}


/** Read a 16-bit unsigned integer.
  */
v_uint16
InputDataStream::readUint16()
{
    Uint16Bytes data;
    m_in->read(data.bytes, 2);
    if (m_byteOrder != m_nativeByteOrder)
    {
        swap(data.bytes[0], data.bytes[1]);
    }

    return data.i;
}


/** Read a 16-bit signed integer.
  */
v_int16
InputDataStream::readInt16()
{
    Int16Bytes data;
    m_in->read(data.bytes, 2);
    if (m_byteOrder != m_nativeByteOrder)
    {
        swap(data.bytes[0], data.bytes[1]);
    }

    return data.i;
}


/** Read the specified number of bytes into a buffer. The buffer must
  * be large enough to hold the data.
  */
void
InputDataStream::readData(char* buffer, size_t byteCount)
{
    m_in->read(buffer, byteCount);
}


/** Skip past the specified number of bytes.
  */
void
InputDataStream::skip(size_t byteCount)
{
    m_in->ignore(byteCount);
}


// Determine the native machine byte order
InputDataStream::ByteOrder
InputDataStream::nativeByteOrder() const
{
    Uint32Bytes test;
    test.bytes[0] = 0x12;
    test.bytes[1] = 0x34;
    test.bytes[2] = 0x56;
    test.bytes[3] = 0x78;

    if (test.i == 0x12345678)
    {
        return BigEndian;
    }
    else if (test.i == 0x78563412)
    {
        return LittleEndian;
    }
    else
    {
        assert("Nonstandard byte order" != 0);
    }
    return BigEndian;
}
