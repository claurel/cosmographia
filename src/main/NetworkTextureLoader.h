// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
//
// This file is a part of qtvesta, a set of classes for using
// the VESTA library with the Qt framework.
//
// qtvesta is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// qtvesta is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with qtvesta. If not, see <http://www.gnu.org/licenses/>.

#ifndef _NETWORK_TEXTURE_LOADER_H_
#define _NETWORK_TEXTURE_LOADER_H_

#include "WMSRequester.h"
#include "vext/PathRelativeTextureLoader.h"
#include <vesta/DataChunk.h>

class LocalImageLoader;

class NetworkTextureLoader : public QObject, public PathRelativeTextureLoader
{
Q_OBJECT
public:
    NetworkTextureLoader(QObject* parent, bool asynchronous = true);
    ~NetworkTextureLoader();

    virtual std::string resolveResourceName(const std::string& resourceName);
    bool handleMakeResident(vesta::TextureMap* texture);
    void realizeLoadedTextures();
    void stop();
    void evictTextures();

    WMSRequester* wmsHandler() const
    {
        return m_wmsHandler;
    }

    unsigned int textureMemoryLimit() const
    {
        return m_textureMemoryLimit;
    }

    void setTextureMemoryLimit(unsigned int megs);

    // Required by PathRelativeTextureLoader
    virtual std::string searchPath() const;
    virtual void setSearchPath(const std::string& path);

    QString localSearchPath() const;
    void setLocalSearchPath(const QString& path);

public slots:
    void queueTexture(vesta::TextureMap* texture, const QImage& image);
    void queueTexture(vesta::TextureMap* texture, vesta::DataChunk* ddsData);
    void queueTexture(const QString& textureName, const QImage& image);
    void reportTextureLoadFailure(vesta::TextureMap* texture);

signals:
    void wmsTileRequested(const QString& tileName,
                          const QString& surface,
                          const QRectF& tileBox,
                          unsigned int tileSize,
                          vesta::TextureMap* texture);
    void localTextureRequested(vesta::TextureMap* texture);

private:
    struct LoadedTexture
    {
        LoadedTexture() :
            ddsImage(NULL),
            texture(NULL)
        {
        }

        // The texture data is either a QImage or a DataChunk (for formats not handled
        // by Qt.)
        QImage texImage;
        vesta::DataChunk* ddsImage;
        vesta::TextureMap* texture;
    };

private:
    QList<LoadedTexture> m_loadedTextures;
    QHash<QString, vesta::TextureMap*> m_textureTable;
    LocalImageLoader* m_localImageLoader;
    WMSRequester* m_wmsHandler;
    QThread* m_imageLoadThread;
    unsigned int m_totalMemoryUsage;
    unsigned int m_textureMemoryLimit;
};

#endif // _NETWORK_TEXTURE_LOADER_H_

