// This file is part of Cosmographia.
// Copyright (C) 2011-2012 Chris Laurel <claurel@gmail.com>
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

#ifndef _VEXT_LOCAL_TILED_MAP_H_
#define _VEXT_LOCAL_TILED_MAP_H_

#include <vesta/HierarchicalTiledMap.h>
#include <QString>
#include <string>


/** LocalTiledMap loads texture tiles from a directory structure on
  * a file system.
  */
class LocalTiledMap : public vesta::HierarchicalTiledMap
{
public:
    LocalTiledMap(vesta::TextureMapLoader* loader, const QString& tileNamePattern, bool flipped, unsigned int tileSize, unsigned int levelCount);
    virtual std::string tileResourceIdentifier(unsigned int level, unsigned int column, unsigned int row);
    virtual bool isValidTileAddress(unsigned int level, unsigned int column, unsigned int row);
    virtual bool tileResourceExists(const std::string& resourceId);

private:
    QString m_tileNamePattern;
    bool m_flipped;
    unsigned int m_levelCount;
};

#endif // _VEXT_LOCAL_TILED_MAP_H_

