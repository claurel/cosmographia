// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Cosmographia is free software; you can redistribute it and/or
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

#ifndef _GALLERY_VIEW_H_
#define _GALLERY_VIEW_H_

#include <vesta/Viewport.h>
#include <vesta/TextureMap.h>
#include <Eigen/Core>
#include <vector>
#include <string>

namespace vesta
{
    class PlanarProjection;
}

class GalleryView
{
public:
    GalleryView();
    ~GalleryView();

    void initializeGL();
    void render(const vesta::Viewport& viewport);

    void addTile(vesta::TextureMap* texture, const std::string& name);

    void setVisible(bool visible);
    bool isVisible() const;

    void update(double dt);

    bool mouseReleased(const vesta::Viewport& viewport, int x, int y);

    /** Return the number of tiles in the gallery. */
    unsigned int tileCount() const
    {
        return m_tiles.size();
    }

    bool isEmpty() const
    {
        return m_tiles.empty();
    }

    /** Get the index of the selected tile (or -1 if no tile is selected.)
      */
    int selectedTile() const
    {
        return m_selectedTileIndex;
    }

    std::string tileName(int tileIndex) const;

private:
    struct GalleryTile
    {
        vesta::counted_ptr<vesta::TextureMap> texture;
        std::string name;
        int row;
        int column;
    };

    enum State
    {
        Hidden,
        Active
    };

    vesta::PlanarProjection camera() const;
    Eigen::Vector3f tilePosition(const GalleryTile& tile);

private:
    std::vector<GalleryTile> m_tiles;
    State m_state;
    double m_time;
    float m_opacity;

    unsigned int m_rows;
    unsigned int m_columns;

    float m_cameraFov;
    float m_galleryRadius;
    float m_galleryAngle;
    float m_tileSpacing;

    int m_selectedTileIndex;
};

#endif // _GALLERY_VIEW_H_
