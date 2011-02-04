/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_DATA_CHUNK_H_
#define _VESTA_DATA_CHUNK_H_

#include "Object.h"
#include <string>


namespace vesta
{

/** DataChunk is a simple container for a block of raw data. The
  * data contained in a DataChunk may not be modified. An instance
  * of DataChunk stores a copy of the original data, so there's no
  * problem if the original is deleted before the DataChunk.
  */
class DataChunk : public Object
{
public:
    DataChunk(const char* data, unsigned int size);
    DataChunk(const std::string& s);
    ~DataChunk();

    /** Get a pointer to the data. */
    const char* data() const
    {
        return m_data;
    }

    /** Get the size in bytes of the data chunk. */
    unsigned int size() const
    {
        return m_size;
    }

private:
    char* m_data;
    size_t m_size;
};

}

#endif // _VESTA_DATA_CHUNK_H_
