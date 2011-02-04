/*
 * $Revision: 339 $ $Date: 2010-07-08 16:03:01 -0700 (Thu, 08 Jul 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_TEXTURE_MAP_LOADER_H_
#define _VESTA_TEXTURE_MAP_LOADER_H_

#include "Object.h"
#include "TextureMap.h"
#include <string>
#include <map>


namespace vesta
{

class TextureMapLoader : public Object
{
public:
    TextureMapLoader();
    virtual ~TextureMapLoader();

    TextureMap* loadTexture(const std::string& resourceName, const TextureProperties& properties);
    bool makeResident(TextureMap* texture);

    /** Handle a request to make a texture resident. Texture loader subclasses
      * must implement this method to load data from the texture source. It is
      * called when the makeResident() method of a texture is called and the texture
      * isn't already resident.
      */
    virtual bool handleMakeResident(TextureMap* texture) = 0;

    v_uint64 evictTextures(v_uint64 desiredMemory, v_int64 mostRecentAllowed);
    v_uint64 textureMemoryUsed() const;

    /** Get the current frame count for this texture loader. The frame count is
      * used to track texture usage so that least recently used textures can
      * be evicted first.
      */
    v_int64 frameCount() const
    {
        return m_frameCount;
    }

    /** Increment the frame counter. This is typically called once per frame rendered.
      *
      * \return the new value of the frame counter
      */
    v_int64 incrementFrameCount();

private:
    v_int64 m_frameCount;
    typedef std::map<std::string, counted_ptr<TextureMap> > TextureTable;
    TextureTable m_textures;
};

}

#endif // _VESTA_TEXTURE_MAP_H_
