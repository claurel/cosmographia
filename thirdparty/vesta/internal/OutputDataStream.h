/*
 * $Revision: 457 $ $Date: 2010-08-25 08:22:07 -0700 (Wed, 25 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef VESTA_INTERNAL_OUTPUT_DATA_STREAM_H_
#define VESTA_INTERNAL_OUTPUT_DATA_STREAM_H_

#include "../IntegerTypes.h"
#include <iostream>

namespace vesta
{

/** OutputDataStream is an internal class used for saving binary data to a stream.
  * It can be set to handle both little- and big- endian data.
  */
class OutputDataStream
{
public:
    OutputDataStream(std::ostream& out);
    ~OutputDataStream();

    enum ByteOrder
    {
        BigEndian    = 0,
        LittleEndian = 1,
    };

    enum StreamStatus
    {
        Good         = 0,
        StreamError  = 1,
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

    void writeUbyte(v_uint8 data);
    void writeByte(v_int8 data);
    void writeFloat(float data);
    void writeUint32(v_uint32 data);
    void writeInt32(v_int32 data);
    void writeUint16(v_uint16 data);
    void writeInt16(v_int16 data);

    void writeData(const char* buffer, size_t byteCount);

private:
    ByteOrder nativeByteOrder() const;

private:
    std::ostream* m_out;
    ByteOrder m_byteOrder;
    ByteOrder m_nativeByteOrder;
    bool m_ownStream;
};

}

#endif // VESTA_INTERNAL_OUTPUT_DATA_STREAM_H_
