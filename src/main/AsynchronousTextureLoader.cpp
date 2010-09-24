/*
 * $Revision: 316 $ $Date: 2010-06-29 20:40:02 -0700 (Tue, 29 Jun 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * This file is published under the Astos Solutions Free Public License.
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "AsynchronousTextureLoader.h"
#include "ImageLoaderThread.h"
#include <vesta/DataChunk.h>
#include <vesta/DDSLoader.h>
#include <QFileInfo>
#include <QImage>
#include <QStringList>
#include <QDebug>

using namespace vesta;


static bool SetTextureImage(TextureMap* texture, const QImage& image)
{
    const uchar* bits = image.bits();

    TextureMap::ImageFormat format;
    if (image.depth() == 24)
    {
        format = TextureMap::B8G8R8;
    }
    else if (image.depth() == 32)
    {
        format = TextureMap::B8G8R8A8;
    }
    else
    {
        return false;
    }

    return texture->generate(bits, image.bytesPerLine() * image.height(), image.width(), image.height(), format);
}


static bool SetTextureImage(TextureMap* texture, const DataChunk* ddsFileContents)
{
    DDSLoader ddsLoader;
    bool ok = ddsLoader.load(texture, ddsFileContents);
    if (!ok)
    {
        qDebug() << ddsLoader.errorMessage().c_str();
    }

    return ok;
}


/** Create a new texture loader. The texture loading thread will not start running until
  * the first call to makeResident()
  */
AsynchronousTextureLoader::AsynchronousTextureLoader(QObject* parent) :
    QObject(parent),
    m_loaderThread(NULL),
    m_wmsHandler(NULL),
    m_wmsThread(NULL),
    m_totalMemoryUsage(0)
{
    m_loaderThread = new ImageLoaderThread();

    connect(m_loaderThread, SIGNAL(ddsTextureReady(vesta::TextureMap*, vesta::DataChunk*)),
            this, SLOT(queueTexture(vesta::TextureMap*, vesta::DataChunk*)));
    connect(m_loaderThread, SIGNAL(textureReady(vesta::TextureMap*, const QImage&)),
            this, SLOT(queueTexture(vesta::TextureMap*, const QImage&)));

    m_wmsHandler = new WMSRequester(NULL);
    connect(this, SIGNAL(wmsTileRequested(const QString&, const QString&, const QRectF&, unsigned int)),
            m_wmsHandler, SLOT(retrieveTile(const QString&, const QString&, const QRectF&, unsigned int)));
    connect(m_wmsHandler, SIGNAL(imageCompleted(const QString&, const QImage&)),
            this, SLOT(queueTexture(const QString&, const QImage&)));

    m_wmsThread = new QThread();
    m_wmsHandler->moveToThread(m_wmsThread);
    m_wmsThread->start();
}


/** Destroy a texture loader and halt the texture loading thread.
  */
AsynchronousTextureLoader::~AsynchronousTextureLoader()
{
    //m_textureQueue->deleteLater();
    delete m_loaderThread;
    delete m_wmsThread;
}


/** Implementation of TextureLoader::makeResident(). Since this TextureLoader is
  * asynchronous, it makeResident returns immediately, and the texture will generally
  * not be available immediately.
  */
bool
AsynchronousTextureLoader::handleMakeResident(TextureMap* texture)
{
    QString textureName = texture->name().c_str();

    qDebug() << "handleMakeResident: " << textureName;

    // Treat texture names beginning with the string "wms:" as Web Map Server tile requests
    // The names should all have the form:
    //   wms:LAYERNAME:LEVEL:X:Y
    // For example, wms:earth-bmng:3:7:1
    if (textureName.startsWith("wms:"))
    {
        texture->setStatus(TextureMap::Loading);

        if (m_wmsHandler)
        {
            QString baseName = textureName.remove("wms:");
            WMSRequester::TileAddress tileAddress = WMSRequester::parseTileName(textureName);

            qDebug() << "wms tile " << tileAddress.level << ", " << tileAddress.x << ", " << tileAddress.y;
            if (tileAddress.valid && tileAddress.level < 13)
            {
                double tileExtent = 180.0 / double(1 << tileAddress.level);
                WMSRequester::LatLongBoundingBox tileBox;
                tileBox.west = -180 + tileAddress.x * tileExtent;
                tileBox.south = -90 + tileAddress.y * tileExtent;
                tileBox.east = tileBox.west + tileExtent;
                tileBox.north = tileBox.south + tileExtent;

                QString tileName = baseName;
                m_textureTable[tileName] = texture;
                emit wmsTileRequested(tileName, tileAddress.surface, tileBox.toRect(), 512);
            }
        }
    }
    else
    {
        m_loaderThread->addTexture(texture);
    }

    return true;
}


/** Halt the texture loading thread.
  */
void
AsynchronousTextureLoader::stop()
{
    m_loaderThread->abort();
    m_wmsThread->quit();
}


void
AsynchronousTextureLoader::evictTextures()
{
    // Using hardcoded limits here:
    //   Cleanup textures when the memory usage reaches 150 megs
    //   Eliminate textures until only 100 megs is in use
    //   Don't evict textures used within the last 5 frames
    unsigned int meg = 1024*1024;
    if (m_totalMemoryUsage > 150*meg)
    {
        m_totalMemoryUsage = TextureMapLoader::evictTextures(100*meg, frameCount() - 5);
        qDebug() << "Memory usage after eviction: " << textureMemoryUsed() / double(meg) << ", frame count: " << frameCount();
    }
}


/** Create GL objects for all textures that have been loaded. This must be called
  * in a thread in which the GL context is current. Normally, it will be called in
  * the display method before scene rendering.
  */
void
AsynchronousTextureLoader::processReadyTextures()
{
    foreach (ReadyTexture r, m_readyTextures)
    {
        bool ok = false;
        if (r.ddsImage)
        {
            ok = SetTextureImage(r.texture, r.ddsImage);
            delete r.ddsImage;
        }
        else
        {
            ok = SetTextureImage(r.texture, r.texImage);
        }

        if (!ok)
        {
            r.texture->setStatus(TextureMap::LoadingFailed);
        }
        else
        {
            m_totalMemoryUsage += r.texture->memoryUsage();

            // Report memory usage
            //double meg = 1024.0 * 1024.0;
            //qDebug() << "Texture: " << r.texture->name().c_str() << ", memory used: " << double(m_totalMemoryUsage) / meg << " MB";

        }
    }

    m_readyTextures.clear();
}


void
AsynchronousTextureLoader::queueTexture(vesta::TextureMap* texture, const QImage& image)
{
    ReadyTexture r;
    r.texture = texture;
    r.texImage = image;
    r.ddsImage = NULL;

    m_readyTextures << r;
}


void
AsynchronousTextureLoader::queueTexture(vesta::TextureMap* texture, vesta::DataChunk* ddsData)
{
    ReadyTexture r;
    r.texture = texture;
    r.ddsImage = ddsData;

    m_readyTextures << r;
}


void
AsynchronousTextureLoader::queueTexture(const QString& textureName, const QImage& image)
{
    TextureMap* texture = m_textureTable.take(textureName);
    if (texture)
    {
        queueTexture(texture, image);
    }
}
