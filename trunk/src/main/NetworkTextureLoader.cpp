// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "NetworkTextureLoader.h"
#include "LocalImageLoader.h"
#include <vesta/DataChunk.h>
#include <vesta/DDSLoader.h>
#include <QFileInfo>
#include <QImage>
#include <QStringList>
#include <QThread>
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


/** Construct a new NetworkTextureLoader.
  */
NetworkTextureLoader::NetworkTextureLoader(QObject* parent, bool asynchronous) :
    QObject(parent),
    m_localImageLoader(NULL),
    m_wmsHandler(NULL),
    m_imageLoadThread(NULL),
    m_totalMemoryUsage(0),
    m_textureMemoryLimit(150)
{
    // Construct an ImageLoader and WMSRequester object. Both of these will can in a separate thread
    // so that reading images from disk and decompressing them won't cause the frame rate to stutter.
    // Loading of textures over the network happens in QNetworkAccessManager threads, *not* the disk
    // load/decompression thread.
    //
    // For synchronization, NetworkTextureLoader relies on Qt's queued signals.

    // Create and connect the local image loader
    m_localImageLoader = new LocalImageLoader();
    connect(this, SIGNAL(localTextureRequested(vesta::TextureMap*)), m_localImageLoader, SLOT(loadTexture(vesta::TextureMap*)));
    connect(m_localImageLoader, SIGNAL(ddsTextureLoaded(vesta::TextureMap*, vesta::DataChunk*)),
            this, SLOT(queueTexture(vesta::TextureMap*, vesta::DataChunk*)));
    connect(m_localImageLoader, SIGNAL(textureLoaded(vesta::TextureMap*, const QImage&)),
            this, SLOT(queueTexture(vesta::TextureMap*, const QImage&)));
    connect(m_localImageLoader, SIGNAL(textureLoadFailed(vesta::TextureMap*)),
            this, SLOT(reportTextureLoadFailure(vesta::TextureMap*)));

    // Create and connect the network tile loader
    m_wmsHandler = new WMSRequester(NULL);
    connect(this, SIGNAL(wmsTileRequested(const QString&, const QString&, const QRectF&, unsigned int, vesta::TextureMap*)),
            m_wmsHandler, SLOT(retrieveTile(const QString&, const QString&, const QRectF&, unsigned int, vesta::TextureMap*)));
    connect(m_wmsHandler, SIGNAL(imageCompleted(const QString&, const QImage&)),
            this, SLOT(queueTexture(const QString&, const QImage&)));

    if (asynchronous)
    {
        m_imageLoadThread = new QThread();
        m_wmsHandler->moveToThread(m_imageLoadThread);
        m_localImageLoader->moveToThread(m_imageLoadThread);
        m_imageLoadThread->start();
    }
}


/** Destroy the texture loader and stop all running threads
  * used for loading images.
  */
NetworkTextureLoader::~NetworkTextureLoader()
{
    if (m_imageLoadThread)
    {
        m_imageLoadThread->exit();
        m_imageLoadThread->deleteLater();
    }
    m_wmsHandler->deleteLater();
    m_localImageLoader->deleteLater();
}


std::string
NetworkTextureLoader::searchPath() const
{
    if (m_localImageLoader)
    {
        return std::string(m_localImageLoader->searchPath().toUtf8().data());
    }
    else
    {
        return "";
    }
}


void
NetworkTextureLoader::setSearchPath(const std::string& path)
{
    if (m_localImageLoader)
    {
        m_localImageLoader->setSearchPath(QString::fromUtf8(path.c_str(), path.size()));
    }
}


std::string
NetworkTextureLoader::resolveResourceName(const std::string& resourceName)
{
    bool isWindowsAbsolutePath = false;
#ifdef Q_OS_WIN
    QRegExp driveLetterPattern("[A-Z]:", Qt::CaseInsensitive);
    isWindowsAbsolutePath = driveLetterPattern.indexIn(QString::fromUtf8(resourceName.c_str()), 0) == 0;
#endif

    if (resourceName.length() >= 4 && resourceName.substr(0, 4) == "wms:")
    {
        return resourceName;
    }
    else if (!resourceName.empty() && (resourceName.at(0) == ':' || resourceName.at(0) == '/' || isWindowsAbsolutePath))
    {
        // Either a Qt internal resource (prefix ':') or an absolute path (prefix '/')
        // Don't prepend the search path, just use the resource name
        return resourceName;
    }
    else
    {
        return std::string(m_localImageLoader->searchPath().toUtf8().constData()) + "/" + resourceName;
    }
}


/** Implementation of TextureLoader::makeResident(). The method returns immediately,
  * but the texture will not actually be loaded until the worker thread has completed
  * loading and decompressing the image file.
  */
bool
NetworkTextureLoader::handleMakeResident(TextureMap* texture)
{
    QString textureName = QString::fromUtf8(texture->name().c_str());

    texture->setStatus(TextureMap::Loading);

    // Treat texture names beginning with the string "wms:" as Web Map Server tile requests
    // The names should all have the form:
    //   wms:LAYERNAME:LEVEL:X:Y
    // For example, wms:earth-bmng:3:7:1
    if (textureName.startsWith("wms:"))
    {
        if (m_wmsHandler)
        {
            QString baseName = textureName.remove("wms:");
            WMSRequester::TileAddress tileAddress = WMSRequester::parseTileName(textureName);

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
                emit wmsTileRequested(tileName, tileAddress.surface, tileBox.toRect(), 512, texture);
            }
        }
    }
    else
    {
        emit localTextureRequested(texture);
    }

    return true;
}


/** Stop the image loading thread.
  */
void
NetworkTextureLoader::stop()
{
    if (m_imageLoadThread)
    {
        m_imageLoadThread->quit();
    }
}


/** Apply the texture eviction policy to reduce the amount of memory
  * consumed by textures:
  *
  * Evict textures when the total memory usage is textureMemoryLimit MB. When
  * evicting, eliminate enough textures to get down to 2/3 the memory limit.
  * Don't evict very recently used textures. "Recently" here means within the
  * last 8 frames.
  */
void
NetworkTextureLoader::evictTextures()
{
    const unsigned int meg = 1024*1024;
    const unsigned int limit = m_textureMemoryLimit * meg;
    const unsigned int targetFootprint = limit * 2 / 3;

    if (m_totalMemoryUsage > limit)
    {
        m_totalMemoryUsage = TextureMapLoader::evictTextures(targetFootprint, frameCount() - 8);
        qDebug() << "Evicted textures, frame: " << frameCount();
    }
}


/** Create GL resources for all loaded textures. This method must be called from
  * thread in which a GL context is current (such as the display thread.)
  */
void
NetworkTextureLoader::realizeLoadedTextures()
{
    foreach (LoadedTexture t, m_loadedTextures)
    {
        bool ok = false;
        if (t.ddsImage)
        {
            ok = SetTextureImage(t.texture, t.ddsImage);
            delete t.ddsImage;
        }
        else
        {
            if (t.texImage.format() == QImage::Format_Indexed8)
            {
                // Convert indexed color images to RGB
                ok = SetTextureImage(t.texture, t.texImage.convertToFormat(QImage::Format_RGB32));
            }
            else
            {
                ok = SetTextureImage(t.texture, t.texImage);
            }
        }

        if (!ok)
        {
            t.texture->setStatus(TextureMap::LoadingFailed);
        }
        else
        {
            m_totalMemoryUsage += t.texture->memoryUsage();
        }
    }

    m_loadedTextures.clear();
}


void
NetworkTextureLoader::queueTexture(vesta::TextureMap* texture, const QImage& image)
{
    LoadedTexture t;
    t.texture = texture;
    t.texImage = image;
    t.ddsImage = NULL;

    m_loadedTextures << t;
}


void
NetworkTextureLoader::queueTexture(vesta::TextureMap* texture, vesta::DataChunk* ddsData)
{
    LoadedTexture t;
    t.texture = texture;
    t.ddsImage = ddsData;

    m_loadedTextures << t;
}


void
NetworkTextureLoader::queueTexture(const QString& textureName, const QImage& image)
{
    TextureMap* texture = m_textureTable.take(textureName);
    if (texture)
    {
        queueTexture(texture, image);
    }
}


void
NetworkTextureLoader::reportTextureLoadFailure(vesta::TextureMap* texture)
{
    texture->setStatus(TextureMap::LoadingFailed);
}


QString
NetworkTextureLoader::localSearchPath() const
{
    if (m_localImageLoader)
    {
        return m_localImageLoader->searchPath();
    }
    else
    {
        return "";
    }
}


void
NetworkTextureLoader::setLocalSearchPath(const QString& path)
{
    if (m_localImageLoader)
    {
        m_localImageLoader->setSearchPath(path);
    }
}
