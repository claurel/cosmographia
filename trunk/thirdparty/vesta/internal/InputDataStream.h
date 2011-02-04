/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef VESTA_INTERNAL_INPUT_DATA_STREAM_H_
#define VESTA_INTERNAL_INPUT_DATA_STREAM_H_

#include "../IntegerTypes.h"
#include <iostream>

namespace vesta
{

/** InputDataStream is an internal class used for loading binary data from a stream.
  * It can be set to handle both little- and big- endian data.
  */
class InputDataStream
{
public:
    InputDataStream(std::istream& in);
    InputDataStream(const std::string& s);
    ~InputDataStream();

    enum ByteOrder
    {
        BigEndian    = 0,
        LittleEndian = 1,
    };

    enum StreamStatus
    {
        Good         = 0,
        EndOfFile    = 1,
        StreamError  = 2,
    };

    ByteOrder byteOrder() const
    {
        return m_byteOrder;
    }

    void setByteOrder(ByteOrder byteOrder)
    {
        m_byteOrder = byteOrder;
    }

    StreamStatus status() const;

    v_uint8 readUbyte();
    v_int8 readByte();
    float readFloat();
    v_uint32 readUint32();
    v_int32 readInt32();
    v_uint16 readUint16();
    v_int16 readInt16();

    void readData(char* buffer, size_t byteCount);
    void skip(size_t byteCount);

private:
    ByteOrder nativeByteOrder() const;

private:
    std::istream* m_in;
    ByteOrder m_byteOrder;
    ByteOrder m_nativeByteOrder;
    bool m_ownStream;
};

}

#endif // VESTA_INTERNAL_INPUT_DATA_STREAM_H_
