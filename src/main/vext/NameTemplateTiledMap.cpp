// This file is part of Cosmographia.
//
// Copyright (C) 2012 Chris Laurel <claurel@gmail.com>
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

#include "NameTemplateTiledMap.h"
#include <QString>

using namespace vesta;
using namespace std;


NameTemplateTiledMap::NameTemplateTiledMap(TextureMapLoader* loader,
                                         const std::string& templ,
                                         unsigned int tileSize,
                                         unsigned int levelCount) :
    HierarchicalTiledMap(loader, tileSize),
    m_nameTemplate(templ),
    m_levelCount(levelCount)
{
}


string
NameTemplateTiledMap::tileResourceIdentifier(unsigned int level, unsigned int column, unsigned int row)
{
    // Tiles are arranged with north = 0
    unsigned int maxRow = (1u << level) - 1;
    
    /*
    ostringstream tileName;
    tileName << m_pattern << "_" << level << "_" << column << "_" << maxRow - row << ".pvr";

    return tileName.str();
    */

    // The external interface uses C++ strings, but for now use Qt to do the actual
    // substitution. The C++ standard library lacks the functions that would make it easy,
    // especially when dealing with UTF-8 strings.
    QString name = QString::fromUtf8(m_nameTemplate.c_str(), m_nameTemplate.length());
    name.replace("%level", QString::number(level));
    name.replace("%row", QString::number(maxRow - row));
    name.replace("%column", QString::number(column));

    return string(name.toUtf8().data());
}


bool 
NameTemplateTiledMap::isValidTileAddress(unsigned int level, unsigned int column, unsigned int row)
{
    unsigned int rowCount = 1u << level;
    unsigned int columnCount = 1u << (level + 1);
    return level < m_levelCount && row < rowCount && column < columnCount;
}


bool 
NameTemplateTiledMap::tileResourceExists(const std::string& /* resourceId */)
{
    return true;
}




