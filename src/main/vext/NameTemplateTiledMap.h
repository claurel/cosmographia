// This file is part of Cosmographia.
//
// Copyright (C) 2012 Chris Laurel <claurel@gmail.com>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Cosmographia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Cosmographia. If not, see <http://www.gnu.org/licenses/>.

#ifndef _NAME_TEMPLATE_TILED_MAP_H_
#define _NAME_PATTERN_TILED_MAP_H_

#include <vesta/HierarchicalTiledMap.h>
#include <vesta/TextureMapLoader.h>
#include <string>

class NameTemplateTiledMap : public vesta::HierarchicalTiledMap
{
public:
    NameTemplateTiledMap(vesta::TextureMapLoader* loader, const std::string& templ, unsigned int tileSize, unsigned int levelCount);
    ~NameTemplateTiledMap() {};
    
    virtual std::string tileResourceIdentifier(unsigned int level, unsigned int column, unsigned int row);
    virtual bool isValidTileAddress(unsigned int level, unsigned int column, unsigned int row);
    virtual bool tileResourceExists(const std::string& resourceId);

private:
    std::string m_nameTemplate;
    unsigned int m_levelCount;
};

#endif // _NAME_PATTERN_TILED_MAP_H_
