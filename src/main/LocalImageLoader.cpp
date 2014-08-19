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

#include "LocalImageLoader.h"
#include <QDebug>
#include <QFileInfo>

using namespace vesta;


LocalImageLoader::LocalImageLoader() :
    m_searchPath(".")
{
}


LocalImageLoader::~LocalImageLoader()
{
}


void
LocalImageLoader::loadTexture(TextureMap* texture)
{
    if (texture)
    {
        QString textureName = QString::fromUtf8(texture->name().c_str());
        QFileInfo info(textureName);

        qDebug() << "loadTexture: " << textureName;

        if (info.suffix() == "dds" || info.suffix() == "dxt5nm")
        {
            // Handle DDS textures
            QFile ddsFile(textureName);
            ddsFile.open(QIODevice::ReadOnly);
            QByteArray data = ddsFile.readAll();

            if (!data.isEmpty())
            {
                emit ddsTextureLoaded(texture, new DataChunk(data.data(), data.size()));
            }
            else
            {
                emit textureLoadFailed(texture);
            }
        }
        else
        {
            // Let Qt handle all file formats other than DDS
            QImage image(textureName);
            if (!image.isNull())
            {
                emit textureLoaded(texture, image);
            }
            else
            {
                emit textureLoadFailed(texture);
            }
        }
    }
}


void
LocalImageLoader::setSearchPath(const QString& path)
{
    m_searchPath = path;
}
