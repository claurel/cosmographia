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
