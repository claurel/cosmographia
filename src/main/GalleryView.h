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

#ifndef _GALLERY_VIEW_H_
#define _GALLERY_VIEW_H_

#include <vesta/Viewport.h>
#include <vesta/TextureMap.h>
#include <vesta/TextureFont.h>
#include <Eigen/Core>
#include <vector>
#include <string>

namespace vesta
{
    class PlanarProjection;
    class GLShaderProgram;
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
    void mouseMoved(const vesta::Viewport& viewport, int x, int y);

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

    vesta::TextureFont* font() const
    {
        return m_font.ptr();
    }

    void setFont(vesta::TextureFont* font)
    {
        m_font = font;
    }
    
    void setGridSize(unsigned int columns, unsigned int rows)
    {
        m_columns = columns;
        m_rows = rows;
    }

    void setScale(float scale)
    {
        m_scale = scale;
    }
    
    float opacity() const
    {
        return m_opacity;
    }

private:
    struct GalleryTile
    {
        vesta::counted_ptr<vesta::TextureMap> texture;
        std::string name;
        int row;
        int column;
        float hover;
    };

    enum State
    {
        Hidden,
        Active
    };

    vesta::PlanarProjection camera() const;
    Eigen::Vector3f tilePosition(const GalleryTile& tile);
    void renderTile(const vesta::Viewport& viewport,
                    const Eigen::Matrix4f& projectionMat,
                    const Eigen::Matrix4f& viewMat,
                    const GalleryTile& tile,
                    bool isSelected);

    int pickTile(const vesta::Viewport& viewport, int x, int y);

    void initGL();
    void finishGL();

private:
    std::vector<GalleryTile> m_tiles;
    State m_state;
    double m_time;
    float m_opacity;

    unsigned int m_rows;
    unsigned int m_columns;

    float m_scale;
    float m_cameraFov;
    float m_galleryRadius;
    float m_galleryAngle;
    float m_galleryHeightScale;
    float m_tileSpacing;

    int m_selectedTileIndex;
    int m_hoverTileIndex;

    vesta::counted_ptr<vesta::TextureFont> m_font;
    vesta::counted_ptr<vesta::GLShaderProgram> m_tileShader;
    vesta::counted_ptr<vesta::GLShaderProgram> m_textShader;
    bool m_initialized;
};

#endif // _GALLERY_VIEW_H_
