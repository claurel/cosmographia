// This file is part of Cosmographia.
//
// Copyright (C) 2010-2011 Chris Laurel <claurel@gmail.com>
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

#ifndef _MULTI_WMS_TILED_MAP_H_
#define _MULTI_WMS_TILED_MAP_H_

#include <vesta/HierarchicalTiledMap.h>
#include <QString>
#include <QList>
#include <string>


/** A MultiWMSTiledMap is a HierarchicalTiledMap that supplies texture tiles
  * from one or more sources based on the hiearchy level. Constructors for two
  * configurations are available:
  *
  * Base layer + detail layer
  * Top layer + base layer + detail layer
  *
  * The top layer is a source for the top level of the map (2 x 1 tiles). If possible,
  * the top layer should be on the local filesystem so that some surface texture is visible
  * even when a network connection isn't available or a server is down.
  */
class MultiWMSTiledMap : public vesta::HierarchicalTiledMap
{
public:
    MultiWMSTiledMap(vesta::TextureMapLoader* loader,
                     const QString& baseLayerName,
                     unsigned int baseLayerLevelCount,
                     const QString& detailLayerName,
                     unsigned int detailLayerLevelCount,
                     unsigned int tileSize);

    MultiWMSTiledMap(vesta::TextureMapLoader* loader,
                     const QString& topLayerName,
                     const QString& baseLayerName,
                     unsigned int baseLayerLevelCount,
                     const QString& detailLayerName,
                     unsigned int detailLayerLevelCount,
                     unsigned int tileSize);

    virtual std::string tileResourceIdentifier(unsigned int level, unsigned int column, unsigned int row);
    virtual bool isValidTileAddress(unsigned int level, unsigned int column, unsigned int row);
    virtual bool tileResourceExists(const std::string& resourceId);

private:
    void setupLevelRanges(const QString& topLayerName,
                          const QString& baseLayerName,
                          unsigned int baseLayerMaxLevel,
                          const QString& detailLayerName,
                          unsigned int detailLayerMaxLevel);

private:
    struct TileLevelRange
    {
        QString tileNamePattern;
        unsigned int levelCount;
    };

    QList<TileLevelRange> m_tileLevelRanges;
};

#endif // _MULTI_WMS_TILED_MAP_H_
