/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "DataChunk.h"
#include <algorithm>

using namespace vesta;
using namespace std;


/** Create a new DataChunk that will contain a copy of the specified data.
  */
DataChunk::DataChunk(const char* data, unsigned int size) :
    m_data(0),
    m_size(size)
{
    m_data = new char[size];
    copy(data, data + size, m_data);
}


/** Create a new DataChunk that will contain a copy of the contents of
  * the specified string.
  */
DataChunk::DataChunk(const string& s) :
    m_data(0),
    m_size(s.size())
{
    if (m_size > 0)
    {
        m_data = new char[m_size];
        copy(s.c_str(), s.c_str() + m_size, m_data);
    }
}


DataChunk::~DataChunk()
{
    delete[] m_data;
}
