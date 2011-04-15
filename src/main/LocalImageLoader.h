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

#ifndef _LOCAL_IMAGE_LOADER_H_
#define _LOCAL_IMAGE_LOADER_H_

#include <vesta/DataChunk.h>
#include <vesta/TextureMap.h>
#include <QImage>
#include <QObject>


/** LocalImageLoader handles loading of images from disk. It uses signals and slots
  * to communicate so that it can be run in a separate thread.
  */
class LocalImageLoader : public QObject
{
    Q_OBJECT

public:
    LocalImageLoader();
    ~LocalImageLoader();

    QString searchPath() const
    {
        return m_searchPath;
    }

public slots:
    void loadTexture(vesta::TextureMap* texture);
    void setSearchPath(const QString& path);

signals:
    /** This signal is emitted when a texture is successfully loaded.
      */
    void textureLoaded(vesta::TextureMap* texture, const QImage& image);

    /** This signal is emitted when a DDS format texture is successfully loaded.
      */
    void ddsTextureLoaded(vesta::TextureMap* texture, vesta::DataChunk* ddsData);

    /** This signal is emitted when texture loading fails for any reason.
      */
    void textureLoadFailed(vesta::TextureMap* texture);

private:
    QString m_searchPath;
};

#endif // _LOCAL_IMAGE_LOADER_H_
