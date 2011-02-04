/*
 * $Revision: 457 $ $Date: 2010-08-25 08:22:07 -0700 (Wed, 25 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "OutputDataStream.h"
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


/** Wrap a C++ ostream with an OutputDataStream. The OutputDataStream does not
  * take ownership of the ostream, so the caller must take care that the ostream
  * isn't destroyed before the OutputDataStream.
  */
OutputDataStream::OutputDataStream(ostream& out) :
    m_out(&out),
    m_byteOrder(nativeByteOrder()),
    m_nativeByteOrder(nativeByteOrder()),
    m_ownStream(false)
{
}

/** Destructor. */
OutputDataStream::~OutputDataStream()
{
    if (m_ownStream)
    {
        delete m_out;
    }
}


/** Get the status of the stream. It will be one of three values:
  *   Good        - the stream is OK
  *   StreamError - an error other than EOF occurred while reading the file
  */
OutputDataStream::StreamStatus
OutputDataStream::status() const
{
    if (m_out->good())
    {
        return Good;
    }
    else
    {
        return StreamError;
    }
}


/** Write an unsigned byte to the stream.
  */
void
OutputDataStream::writeUbyte(v_uint8 data)
{
    m_out->put(static_cast<char>(data));
}


/** Write a signed byte to the stream.
  */
void
OutputDataStream::writeByte(v_int8 data)
{
    m_out->put(static_cast<char>(data));
}


/** Write an IEEE754 single precision floating point number to the stream.
  */
void
OutputDataStream::writeFloat(float data)
{
    FloatBytes value;

    value.f = data;
    if (m_byteOrder != m_nativeByteOrder)
    {
        swap(value.bytes[0], value.bytes[3]);
        swap(value.bytes[1], value.bytes[2]);
    }
    m_out->write(value.bytes, 4);
}


/** Write a 32-bit unsigned integer.
  */
void
OutputDataStream::writeUint32(v_uint32 data)
{
    Uint32Bytes value;

    value.i = data;
    if (m_byteOrder != m_nativeByteOrder)
    {
        swap(value.bytes[0], value.bytes[3]);
        swap(value.bytes[1], value.bytes[2]);
    }
    m_out->write(value.bytes, 4);
}


/** Write a 32-bit signed integer.
  */
void
OutputDataStream::writeInt32(v_int32 data)
{
    Int32Bytes value;

    value.i = data;
    if (m_byteOrder != m_nativeByteOrder)
    {
        swap(value.bytes[0], value.bytes[3]);
        swap(value.bytes[1], value.bytes[2]);
    }
    m_out->write(value.bytes, 4);
}


/** Write a 16-bit unsigned integer.
  */
void
OutputDataStream::writeUint16(v_uint16 data)
{
    Uint16Bytes value;

    value.i = data;
    if (m_byteOrder != m_nativeByteOrder)
    {
        swap(value.bytes[0], value.bytes[1]);
    }
    m_out->write(value.bytes, 2);
}


/** Write a 16-bit signed integer.
  */
void
OutputDataStream::writeInt16(v_int16 data)
{
    Int16Bytes value;

    value.i = data;
    if (m_byteOrder != m_nativeByteOrder)
    {
        swap(value.bytes[0], value.bytes[1]);
    }
    m_out->write(value.bytes, 2);
}


/** Write the specified number of bytes from a buffer.
  */
void
OutputDataStream::writeData(const char* buffer, size_t byteCount)
{
    m_out->write(buffer, byteCount);
}

// Determine the native machine byte order
OutputDataStream::ByteOrder
OutputDataStream::nativeByteOrder() const
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
