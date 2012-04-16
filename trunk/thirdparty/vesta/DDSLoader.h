/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_DDS_LOADER_H_
#define _VESTA_DDS_LOADER_H_

#include "DataChunk.h"
#include "TextureMap.h"
#include <string>


namespace vesta
{

struct DDSHeader;

/** DDSLoader is a helper class for loading textures stored in Microsoft's DDS
  * (DirectDraw Surface) format. It is designed to be used by a TextureLoader
  * subclass. The TextureLoader should read the entire texture file into a
  * DataChunk, then call DDSLoader.load() to convert the raw data into a
  * texture. The present implementation only handles block compressed textures.
  */
class DDSLoader : public Object
{
public:
    DDSLoader();
    ~DDSLoader();

    bool load(TextureMap* tex, const DataChunk* data);

    std::string errorMessage() const
    {
        return m_errorMessage;
    }

private:
    void reportError(const std::string& message);
    bool loadCompressedTexture(TextureMap* tex, const DataChunk* data, const DDSHeader* header, TextureMap::ImageFormat);

private:
    std::string m_errorMessage;
};

}

#endif // _VESTA_DDS_LOADER_H_
