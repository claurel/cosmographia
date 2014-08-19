// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
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

#include "LocalTiledMap.h"
#include <QFileInfo>

using namespace vesta;
using namespace std;


/** \param pattern is a string that will be used to construct a tile name given
  *        the level, column, and row. %1, %2, and %3 in the string will be replaced
  *        with the values of the level, column, and row, respectively.
  *
  * Example pattern: "mars/level%1/tile_%2_%3.png"
  */
LocalTiledMap::LocalTiledMap(TextureMapLoader* loader, const QString& tileNamePattern, bool flipped, unsigned int tileSize, unsigned int levelCount) :
    HierarchicalTiledMap(loader, tileSize),
    m_tileNamePattern(tileNamePattern),
    m_flipped(flipped),
    m_levelCount(levelCount)
{
}


string
LocalTiledMap::tileResourceIdentifier(unsigned int level, unsigned int column, unsigned int row)
{
    // Row may be inverted here if the tiles are arranged so that the northernmost
    // tile in a level is at row 0.
    unsigned int y = m_flipped ? (1 << level) - 1 - row : row;
    QString s = m_tileNamePattern.arg(level).arg(column).arg(y);
    return string(s.toUtf8().data());
}


bool
LocalTiledMap::isValidTileAddress(unsigned int level, unsigned int column, unsigned int row)
{
    return level < m_levelCount && column < (1u << (level + 1)) && row < (1u << level);
}


bool
LocalTiledMap::tileResourceExists(const std::string& resourceId)
{
    if (QString(resourceId.c_str()).startsWith("wms:"))
    {
        return true;//level < levelCount;
    }
    else
    {
        return QFileInfo(resourceId.c_str()).exists();
    }
}
