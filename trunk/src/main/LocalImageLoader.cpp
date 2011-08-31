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
