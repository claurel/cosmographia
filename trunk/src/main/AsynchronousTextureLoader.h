/*
 * $Revision: 316 $ $Date: 2010-06-29 20:40:02 -0700 (Tue, 29 Jun 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * This file is published under the Astos Solutions Free Public License.
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _ASYNCHRONOUS_TEXTURE_LOADER_H_
#define _ASYNCHRONOUS_TEXTURE_LOADER_H_

#include "WMSRequester.h"
#include <vesta/TextureMapLoader.h>
#include <vesta/DataChunk.h>

class ImageLoaderThread;


// TextureMapLoader implementation that uses Qt to load an image file
// from disk.
class AsynchronousTextureLoader : public QObject, public vesta::TextureMapLoader
{
Q_OBJECT
public:
    AsynchronousTextureLoader(QObject* parent);
    ~AsynchronousTextureLoader();

    bool handleMakeResident(vesta::TextureMap* texture);
    void processReadyTextures();
    void stop();
    void evictTextures();

    WMSRequester* wmsHandler() const
    {
        return m_wmsHandler;
    }

public slots:
    void queueTexture(vesta::TextureMap* texture, const QImage& image);
    void queueTexture(vesta::TextureMap* texture, vesta::DataChunk* ddsData);
    void queueTexture(const QString& textureName, const QImage& image);

signals:
    void wmsTileRequested(const QString& tileName,
                          const QString& surface,
                          const QRectF& tileBox,
                          unsigned int tileSize);

private:
    struct ReadyTexture
    {
        ReadyTexture() :
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
    QList<ReadyTexture> m_readyTextures;
    QHash<QString, vesta::TextureMap*> m_textureTable;
    ImageLoaderThread* m_loaderThread;
    WMSRequester* m_wmsHandler;
    QThread* m_wmsThread;
    unsigned int m_totalMemoryUsage;
};

#endif // _ASYNCHRONOUS_TEXTURE_LOADER_H_

