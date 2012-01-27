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

#include "GalleryView.h"
#include <vesta/OGLHeaders.h>
#include <vesta/VertexSpec.h>
#include <vesta/Units.h>
#include <vesta/PlanarProjection.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <cmath>
#include <algorithm>

using namespace vesta;
using namespace Eigen;
using namespace std;


// Gallery parameters:
//
// columns - The number of columns in the tile grid
// rows - The number of rows in the tile grid
// tileSpacing - Space between tiles, as a fraction of the tile size

GalleryView::GalleryView() :
    m_state(Hidden),
    m_time(0.0),
    m_opacity(0.0f),
    m_rows(4),
    m_columns(10),
    m_cameraFov(toRadians(45.0f)),
    m_galleryRadius(50.0f),
    m_galleryAngle(toRadians(60.0f)),
    m_tileSpacing(0.1f),
    m_selectedTileIndex(-1),
    m_hoverTileIndex(-1)
{
}


GalleryView::~GalleryView()
{
}


void
GalleryView::initializeGL()
{
}


struct TileVertex
{
    Eigen::Vector3f position;
    Eigen::Vector2f texCoord;
};


static void
quadIndices(v_uint16* indices, v_uint16 v0, v_uint16 v1, v_uint16 v2, v_uint16 v3)
{
    indices[0] = v0; indices[1] = v1; indices[2] = v2;
    indices[3] = v0; indices[4] = v2; indices[5] = v3;
}


static void
drawRoundRectangle(float width, float height, float radius, float opacity)
{
    const unsigned int arcSegments = 10;
    const unsigned int vertexCount = arcSegments * 4 + 8;

    float inWidth  = width - 2 * radius;
    float inHeight = height - 2 * radius;

    TileVertex vertices[vertexCount];

    // Interior rectangle
    vertices[0].position = Vector3f( inWidth, inHeight, 0.0f) * 0.5f;
    vertices[1].position = Vector3f(-inWidth, inHeight, 0.0f) * 0.5f;
    vertices[2].position = Vector3f(-inWidth, -inHeight, 0.0f) * 0.5f;
    vertices[3].position = Vector3f( inWidth, -inHeight, 0.0f) * 0.5f;

    for (unsigned int arc = 0; arc < 4; ++arc)
    {
        for (unsigned int i = 0; i <= arcSegments; ++i)
        {
            float theta = toRadians(90.0f * (arc + float(i) / float(arcSegments)));
            Vector3f v(radius * cos(theta), radius * sin(theta), 0.0f);

            v -= Vector3f(inWidth * 0.5f, inHeight * 0.5f, 0.0f);
            if (arc == 0 || arc == 3)
            {
                v.x() += inWidth;
            }
            if (arc == 0 || arc == 1)
            {
                v.y() += inHeight;
            }
            vertices[4 + arc * (arcSegments + 1) + i].position = v;
        }
    }

    // Set the texture coordinates
    for (unsigned int i = 0; i < vertexCount; ++i)
    {
        vertices[i].texCoord = Vector2f(vertices[i].position.x() / width + 0.5f,
                                        -vertices[i].position.y() / height + 0.5f);
    }

    const unsigned int triangleCount = 10 + arcSegments * 4;
    const unsigned int indexCount = 3 * triangleCount;
    v_uint16 indices[indexCount];

    // Interior rectangle
    quadIndices(indices + 0, 0, 1, 2, 3);

    // Right rectangle
    quadIndices(indices + 6,  0, 3, 3 + 4 * (arcSegments + 1), 4);

    // Top rectangle
    quadIndices(indices + 12, 1, 0, 3 + 1 * (arcSegments + 1), 4 + 1 * (arcSegments + 1));

    // Left rectangle
    quadIndices(indices + 18, 2, 1, 3 + 2 * (arcSegments + 1), 4 + 2 * (arcSegments + 1));

    // Bottom rectangle
    quadIndices(indices + 24, 3, 2, 3 + 3 * (arcSegments + 1), 4 + 3 * (arcSegments + 1));

    // Corners
    unsigned int baseIndex = 30;
    for (unsigned int arc = 0; arc < 4; ++arc)
    {
        for (unsigned int i = 0; i < arcSegments; ++i)
        {
            indices[baseIndex + 0] = arc;
            indices[baseIndex + 1] = 4 + arc * (arcSegments + 1) + i;
            indices[baseIndex + 2] = 4 + arc * (arcSegments + 1) + i + 1;
            baseIndex += 3;
        }
    }

    unsigned int stride = sizeof(vertices[0]);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, stride, reinterpret_cast<const char*>(vertices));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, stride, reinterpret_cast<const char*>(vertices) + 12);
    glDisableClientState(GL_COLOR_ARRAY);

    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, indices);
    glColor4f(0.0f, 0.0f, 0.0f, opacity);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(2.5f);
    glEnable(GL_LINE_SMOOTH);
    //glDrawArrays(GL_LINE_LOOP, 4, 4 * (arcSegments + 1));
    glLineWidth(1.0f);
    glDisable(GL_LINE_SMOOTH);
    glEnable(GL_TEXTURE_2D);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


PlanarProjection
GalleryView::camera() const
{
    return PlanarProjection::CreatePerspective(m_cameraFov, 1.0f, 1.0f, 1000.0f);
}


// Return the length of the chord between two points on the unit
// circle separated by the angle theta.
static float chordLength(float theta)
{
    return 2.0f * sin(theta * 0.5f);
}


Eigen::Vector3f
GalleryView::tilePosition(const GalleryTile& tile)
{
    const float viewPlaneWidth = 0.8f * m_galleryRadius * chordLength(m_galleryAngle);
    const float galleryHeight = viewPlaneWidth * float(m_rows) / float(m_columns);
    double theta = (float(tile.column) / float(m_columns - 1) - 0.5f) * m_galleryAngle - toRadians(90.0);
    float x = m_galleryRadius * cos(theta);
    float y = (float(tile.row) / float(m_rows - 1) - 0.5f) * galleryHeight;
    float z = m_galleryRadius * sin(theta);

    return Vector3f(x, y, z);
}


void
GalleryView::renderTile(const Viewport& viewport,
                        const Matrix4f& projectionMat,
                        const GalleryTile& tile,
                        bool isSelected)
{
    const float viewPlaneWidth = 0.8f * m_galleryRadius * chordLength(m_galleryAngle);
    const float centerOffset = (viewPlaneWidth / 2.0f) / tan(m_cameraFov / 2.0f) - m_galleryRadius * cos(m_galleryAngle / 2.0f);

    const float tileFraction = 1.0f / (1.0f + m_tileSpacing);
    const float tileScale = 0.5f * tileFraction * m_galleryRadius * chordLength(m_galleryAngle / m_columns);

    double theta = (float(tile.column) / float(m_columns - 1) - 0.5f) * m_galleryAngle - toRadians(90.0);
    float t = float(tile.row + tile.column) / float(m_rows + m_columns) * 1.0f + m_opacity * 2.0f - 1.0f;
    t = max(-1.0f, min(1.0f, t));
    if (!isSelected)
    {
        theta += (t - 1.0f) * toRadians(90.0f);
    }

    float scaleFactor = tileScale;
    float alpha = m_opacity;
    if (isSelected)
    {
        scaleFactor = (1.0f + 3.0f * (1.0f - m_opacity)) * tileScale;
        alpha = min(1.0f, m_opacity * 3.0f);
    }

    Vector3f tilePos = tilePosition(tile);

    glPushMatrix();
    glTranslatef(tilePos.x(), tilePos.y(), tilePos.z());
    glRotatef(-(toDegrees(theta) + 90.0f), 0.0f, 1.0f, 0.0f);
    glScalef(scaleFactor, scaleFactor, scaleFactor);

    if (tile.texture.isValid() && tile.texture->makeResident())
    {
        glBindTexture(GL_TEXTURE_2D, tile.texture->id());
        glColor4f(1.0f, 1.0f, 1.0f, alpha);
        drawRoundRectangle(2.0f, 2.0f, 0.3f, alpha);
    }

    glPopMatrix();

    if (tile.hover > 0.0f)
    {
        Transform3f projectionXform(projectionMat);
        Transform3f cameraXform(Translation3f(Vector3f(0.0f, 0.0f, -centerOffset)));
        Transform3f mvp = projectionXform * cameraXform;
        Vector3f labelPos = tilePos + Vector3f(0.0f, -scaleFactor, 0.0f);
        Vector3f projPosition = mvp * labelPos;
        Vector2f screenPos((projPosition.x() * 0.5f + 0.5f) * viewport.width(),
                           (projPosition.y() * 0.5f + 0.5f) * viewport.height());

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, viewport.width(), 0, viewport.height());
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(0.125f, 0.125f, 0);

        if (m_font.isValid())
        {
            glColor4f(1.0f, 1.0f, 1.0f, tile.hover);
            m_font->bind();
            m_font->render(tile.name, screenPos + Vector2f(m_font->textWidth(tile.name) * -0.5f, 4.0f));
        }

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }
}


void
GalleryView::render(const Viewport& viewport)
{
    const float viewPlaneWidth = 0.8f * m_galleryRadius * chordLength(m_galleryAngle);
    const float centerOffset = (viewPlaneWidth / 2.0f) / tan(m_cameraFov / 2.0f) - m_galleryRadius * cos(m_galleryAngle / 2.0f);

    if (m_opacity <= 0.0f)
    {
        return;
    }

    PlanarProjection projectionMat = PlanarProjection::CreatePerspective(m_cameraFov, viewport.aspectRatio(), 1.0f, 1000.0f);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(projectionMat.matrix().data());
    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -centerOffset);

    glEnable(GL_TEXTURE_2D);

    // First, draw all tiles except the selected one
    for (int tileIndex = 0; tileIndex < int(m_tiles.size()); ++tileIndex)
    {
        if (tileIndex != m_selectedTileIndex)
        {
            const GalleryTile& tile = m_tiles[tileIndex];
            renderTile(viewport, projectionMat.matrix(), tile, false);
        }
    }

    // Draw the selected tile last: it's in front and should occlude the others.
    // (The z-buffer is disabled, otherwise we'd have to clear it between rendering
    // the solar system and drawing the gallery tiles.)
    if (m_selectedTileIndex >= 0 && m_selectedTileIndex < int(m_tiles.size()))
    {
        renderTile(viewport, projectionMat.matrix(), m_tiles[m_selectedTileIndex], true);
    }

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}


void
GalleryView::addTile(TextureMap* texture, const std::string& name)
{
    GalleryTile tile;
    tile.texture = texture;
    tile.name = name;
    tile.row = m_tiles.size() / m_columns;
    tile.column = m_tiles.size() % m_columns;
    tile.hover = 0.0f;
    m_tiles.push_back(tile);
}


std::string
GalleryView::tileName(int tileIndex) const
{
    if (tileIndex < 0 || tileIndex >= int(m_tiles.size()))
    {
        return "";
    }
    else
    {
        return m_tiles[tileIndex].name;
    }
}


void
GalleryView::setVisible(bool visible)
{
    if (visible)
    {
        m_state = Active;
        m_selectedTileIndex = -1;
    }
    else
    {
        m_state = Hidden;
    }
}


bool
GalleryView::isVisible() const
{
    return m_state != Hidden;
}


void
GalleryView::update(double dt)
{
    m_time += dt;

    if (m_state == Active)
    {
        m_opacity = std::min(1.0f, m_opacity + float(dt) * 1.0f);
    }
    else if (m_state == Hidden)
    {
        m_opacity = std::max(0.0f, m_opacity - float(dt) * 1.0f);
    }

    for (int tileIndex = 0; tileIndex < int(m_tiles.size()); ++tileIndex)
    {
        GalleryTile& tile = m_tiles[tileIndex];
        if (tileIndex == m_hoverTileIndex)
        {
            tile.hover = min(1.0f, float(tile.hover + dt * 6));
        }
        else
        {
            tile.hover = max(0.0f, float(tile.hover - dt * 2));
        }
    }
}


bool
GalleryView::mouseReleased(const Viewport& viewport, int x, int y)
{
    if (m_opacity <= 0.0f)
    {
        return false;
    }

    m_selectedTileIndex = pickTile(viewport, x, y);
    if (m_selectedTileIndex >= 0)
    {
        setVisible(false);
        return true;
    }
    else
    {
        return false;
    }
}


void
GalleryView::mouseMoved(const Viewport& viewport, int x, int y)
{
    m_hoverTileIndex = pickTile(viewport, x, y);
}


int
GalleryView::pickTile(const Viewport& viewport, int x, int y)
{
    if (m_opacity <= 0.0f)
    {
        return false;
    }

    int pickedTileIndex = -1;

    Vector2f p = Vector2f((float(x) - viewport.x()) / viewport.width(),
                          -(float(y) - viewport.y()) / viewport.height()) * 2.0f + Vector2f(-1.0f, 1.0f);

    float h = tan(m_cameraFov / 2.0f);
    Vector3f direction = Vector3f(h * viewport.aspectRatio() * p.x(), h * p.y(), -1.0).normalized();

    const float viewPlaneWidth = 0.8f * m_galleryRadius * chordLength(m_galleryAngle);
    const float centerOffset = (viewPlaneWidth / 2.0f) / tan(m_cameraFov / 2.0f) - m_galleryRadius * cos(m_galleryAngle / 2.0f);

    const float tileFraction = 1.0f / (1.0f + m_tileSpacing);
    const float tileScale = 0.5f * tileFraction * m_galleryRadius * chordLength(m_galleryAngle / m_columns);

    Vector3f cameraPosition(0.0f, 0.0f, centerOffset);

    for (int tileIndex = 0; tileIndex < int(m_tiles.size()); ++tileIndex)
    {
        const GalleryTile& tile = m_tiles[tileIndex];

        Vector3f tileCenter = tilePosition(tile);
        double theta = (float(tile.column) / float(m_columns - 1) - 0.5f) * m_galleryAngle - toRadians(90.0);

        float t = float(tile.row + tile.column) / float(m_rows + m_columns) * 1.0f + m_opacity * 2.0f - 1.0f;
        t = max(-1.0f, min(1.0f, t));
        theta += (t - 1.0f) * toRadians(90.0f);

        Vector3f planeNormal = -Vector3f::UnitX();
        planeNormal = AngleAxisf(-theta, Vector3f::UnitY()) * planeNormal;

        Vector3f rayOrigin = cameraPosition - tileCenter;
        float u = -planeNormal.dot(rayOrigin) / planeNormal.dot(direction);
        Vector3f intersection = rayOrigin + direction * u;

        if (abs(intersection.x()) < tileScale && abs(intersection.y()) < tileScale)
        {
            pickedTileIndex = tileIndex;
        }
    }

    return pickedTileIndex;
}

