/*
 * $Revision: 606 $ $Date: 2011-04-14 22:50:07 -0700 (Thu, 14 Apr 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "TextureMapLoader.h"
#include "Debug.h"
#include <vector>
#include <algorithm>
#include <sstream>

using namespace vesta;
using namespace std;

// Set to 1 to show debugging output for texture eviction
#define DEBUG_EVICTION 0


TextureMapLoader::TextureMapLoader() :
    m_frameCount(0)
{
}


TextureMapLoader::~TextureMapLoader()
{
}


// Convert the name and properties of a texture into a unique key to identify
// the texture.
static string GenerateKey(const string& name, const TextureProperties& properties)
{
    ostringstream key;

    key << name << "|";
    key << (int) properties.addressS << "|";
    key << (int) properties.addressT << "|";

    return key.str();
}


/** Converts a resource name into a unique identifier, possibly based on
  * some state maintained by the TextureMapLoader, such as the current directory.
  * The default implementation just returns the resource name unmodified.
  */
string
TextureMapLoader::resolveResourceName(const std::string& resourceName)
{
    return resourceName;
}


/** Create a new texture object that will be managed by this loader. If a texture with
  * the same resolved resource name and properties already exists, this object is returned.
  * Otherwise, a new texture is created in an unitialized state; the new texture cannot
  * be used for rendering until its makeResident() method is called.
  *
  * The interpretation of the resource name is left to the particular texture loader;
  * typically, it will either be a filename or a URL.
  *
  * Note that the resource name is first resolved by calling resolveResourceName. The
  * default implementation of resolveResourceName returns the resource name unmodified,
  * but a subclass may override the method to generate a string based on the resource
  * name and some internal state of the loader.
  */
TextureMap*
TextureMapLoader::loadTexture(const string& resourceName, const TextureProperties& properties)
{
    string resolvedName = resolveResourceName(resourceName);
    string key = GenerateKey(resolvedName, properties);

    TextureTable::iterator iter = m_textures.find(key);
    if (iter != m_textures.end())
    {
        return iter->second.ptr();
    }
    else
    {
        counted_ptr<TextureMap> texture;
        texture = new TextureMap(resolvedName, this, properties);
        m_textures[key] = texture;
        return texture.ptr();
    }
}


/** Start loading a texture; the texture may not be immediately available to use when
  * rendering if the texture loader is asynchronous.
  *
  * \return true if the texture is ready for rendering
  */
bool
TextureMapLoader::makeResident(TextureMap* texture)
{
    texture->setLastUsed(m_frameCount);
    bool isResident = handleMakeResident(texture);

    return isResident;
}


/** Update the frame count. The frame count is used to track texture usage in order
  * to determine which textures should be evicted first when trimming graphics memory
  * usage.
  */
v_int64
TextureMapLoader::incrementFrameCount()
{
    return ++m_frameCount;
}


class TextureAgePredicate
{
public:
    bool operator() (const TextureMap* t0, const TextureMap* t1) const
    {
        return t0->lastUsed() < t1->lastUsed();
    }
};


/** Evict textures in order to reduce texture memory usage. Textures
  * will be evicted until the total size of textures managed by this
  * texture loader is less than or equal to desired memory. Least recently
  * used textures are evicted first. No texture with a last used value
  * greater than mostRecentAllowed will be evicted, even if it means that
  * the desired memory target can't be reached.
  *
  * Evict textures must be called from a thread in which a GL context
  * is current (typically the display thread.) It can take some time to
  * process all textures, so it shouldn't be called frequently (i.e.
  * once a frame is too often.)
  *
  * \return the total size of all textures remaining
  */
v_uint64
TextureMapLoader::evictTextures(v_uint64 desiredMemory, v_int64 mostRecentAllowed)
{
#if DEBUG_EVICTION
    // Show all textures managed by this loader
    for (TextureTable::const_iterator iter = m_textures.begin(); iter != m_textures.end(); ++iter)
    {
        VESTA_LOG("Texture: %s, mem: %.2f", iter->second.ptr()->name().c_str(), double(iter->second.ptr()->memoryUsage()) / (1024*1024));
    }
#endif

    v_uint64 textureMemory = textureMemoryUsed();

    // Early out if the memory usage target is already met
    if (textureMemory < desiredMemory)
    {
        return textureMemory;
    }

    // Create a list of textures sorted such that least recently used textures are first.
    vector<TextureMap*> sortedTextures;
    for (TextureTable::const_iterator iter = m_textures.begin(); iter != m_textures.end(); ++iter)
    {
        sortedTextures.push_back(iter->second.ptr());
    }
    sort(sortedTextures.begin(), sortedTextures.end(), TextureAgePredicate());

    // Evict textures until we reach the memory target
    for (vector<TextureMap*>::const_iterator iter = sortedTextures.begin();
         iter != sortedTextures.end() && (*iter)->lastUsed() <= mostRecentAllowed && textureMemory > desiredMemory;
         ++iter)
    {
        TextureMap* t = *iter;
        if (t->isResident())
        {
#if DEBUG_EVICTION
            VESTA_LOG("evict %s @ %d", t->name().c_str(), (int) t->lastUsed());
#endif
            textureMemory -= t->memoryUsage();
            t->evict();
        }
    }

    return textureMemory;
}


/** Return the amount of texture memory used for all textures managed by
  * this loader.
  */
v_uint64
TextureMapLoader::textureMemoryUsed() const
{
    // TODO: we should track this rather than counting textures every time

    v_uint64 total = 0;
    for (TextureTable::const_iterator iter = m_textures.begin(); iter != m_textures.end(); ++iter)
    {
        total += iter->second.ptr()->memoryUsage();
    }

    return total;
}
