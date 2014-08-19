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

#include "GalleryView.h"
#include <vesta/OGLHeaders.h>
#include <vesta/VertexSpec.h>
#include <vesta/Units.h>
#include <vesta/PlanarProjection.h>
#include <vesta/glhelp/GLShaderProgram.h>
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
    m_scale(1.0f),
    m_cameraFov(float(toRadians(45.0))),
    m_galleryRadius(50.0f),
    m_galleryAngle(float(toRadians(60.0f))),
    m_galleryHeightScale(1.3f),
    m_tileSpacing(0.1f),
    m_selectedTileIndex(-1),
    m_hoverTileIndex(-1),
    m_initialized(false)
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
            float theta = float(toRadians(90.0 * (arc + float(i) / float(arcSegments))));
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

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, vertices[0].position.data());
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, vertices[0].texCoord.data());
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, indices);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    /*
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
    */
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
    const float galleryHeight = viewPlaneWidth * float(m_rows) / float(m_columns) * m_galleryHeightScale;
    double theta = (float(tile.column) / float(m_columns - 1) - 0.5f) * m_galleryAngle - toRadians(90.0);
    float x = float(m_galleryRadius * cos(theta));
    float y = -(float(tile.row) / float(m_rows - 1) - 0.5f) * galleryHeight;
    float z = float(m_galleryRadius * sin(theta));

    return Vector3f(x, y, z);
}


void
GalleryView::renderTile(const Viewport& viewport,
                        const Matrix4f& projectionMat,
                        const Matrix4f& viewMat,
                        const GalleryTile& tile,
                        bool isSelected)
{
    const float viewPlaneWidth = 0.8f * m_galleryRadius * chordLength(m_galleryAngle);
    const float centerOffset = (1.0f / m_scale) * (viewPlaneWidth / 2.0f) / tan(m_cameraFov / 2.0f) - m_galleryRadius * cos(m_galleryAngle / 2.0f);

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

    /*
    glTranslatef(tilePos.x(), tilePos.y(), tilePos.z());
    glRotatef(-(toDegrees(theta) + 90.0f), 0.0f, 1.0f, 0.0f);
    glScalef(scaleFactor, scaleFactor, scaleFactor);
    */
    Transform3f modelTransform = Translation3f(tilePos) * AngleAxisf(-(theta + PI / 2.0f), Vector3f::UnitY()) * Scaling3f(scaleFactor);
    Matrix4f mv = viewMat * modelTransform.matrix();
    Matrix4f mvp = projectionMat * mv;

    m_tileShader->bind();
    m_tileShader->setSampler("baseTex", 0);
    m_tileShader->setConstant("mvp", mvp);
    m_tileShader->setConstant("color", Vector4f(1.0f, 1.0f, 1.0f, alpha));

    if (tile.texture.isValid() && tile.texture->makeResident())
    {
        glBindTexture(GL_TEXTURE_2D, tile.texture->id());
        drawRoundRectangle(2.0f, 2.0f, 0.3f, alpha);
    }

    glUseProgram(0);

#ifndef VESTA_OGLES2
    if (tile.hover > 0.0f)
    {
        Transform3f projectionXform(projectionMat);
        Transform3f cameraXform(Translation3f(Vector3f(0.0f, 0.0f, -centerOffset)));
        Transform3f mvp = projectionXform * cameraXform;
        Vector3f labelPos = tilePos + Vector3f(0.0f, -scaleFactor, 0.0f);
        Vector3f projPosition = mvp * labelPos;
        Vector2f screenPos((projPosition.x() * 0.5f + 0.5f) * viewport.width(),
                           (projPosition.y() * 0.5f + 0.5f) * viewport.height());

#if 0
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, viewport.width(), 0, viewport.height());
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(0.125f, 0.125f, 0);
#endif
        Matrix4f textProjectionMat = PlanarProjection::CreateOrthographic2D(0.0f, viewport.width(), 0.0f, viewport.height()).matrix();
        Matrix4f modelViewMat = Transform3f(Translation3f(Vector3f(0.125f, 0.125f, 0.0f))).matrix();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadMatrixf(textProjectionMat.data());
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadMatrixf(modelViewMat.data());

        if (m_font.isValid())
        {
            glColor4f(1.0f, 1.0f, 1.0f, tile.hover);
            m_font->bind();
            m_font->render(tile.name, screenPos + Vector2f(m_font->textWidth(tile.name) * -0.5f, -15.0f));
        }

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }
#endif
}


void
GalleryView::render(const Viewport& viewport)
{
    if (!m_initialized)
    {
        m_initialized = true;
        initGL();
    }

    if (m_tileShader.isNull())
    {
        // Shader creation failed; nothing we can do
        return;
    }

    const float viewPlaneWidth = 0.8f * m_galleryRadius * chordLength(m_galleryAngle);
    const float centerOffset = (1.0f / m_scale) * (viewPlaneWidth / 2.0f) / tan(m_cameraFov / 2.0f) - m_galleryRadius * cos(m_galleryAngle / 2.0f);

    if (m_opacity <= 0.0f)
    {
        return;
    }

    PlanarProjection projectionMat = PlanarProjection::CreatePerspective(m_cameraFov, viewport.aspectRatio(), 1.0f, 1000.0f);
    Matrix4f viewMat = Transform3f(Translation3f(0.0f, 0.0f, -centerOffset)).matrix();

#ifndef VESTA_OGLES2
    glEnable(GL_TEXTURE_2D);
#endif

    // First, draw all tiles except the selected one
    for (int tileIndex = 0; tileIndex < int(m_tiles.size()); ++tileIndex)
    {
        if (tileIndex != m_selectedTileIndex)
        {
            const GalleryTile& tile = m_tiles[tileIndex];
            renderTile(viewport, projectionMat.matrix(), viewMat, tile, false);
        }
    }

    // Draw the selected tile last: it's in front and should occlude the others.
    // (The z-buffer is disabled, otherwise we'd have to clear it between rendering
    // the solar system and drawing the gallery tiles.)
    if (m_selectedTileIndex >= 0 && m_selectedTileIndex < int(m_tiles.size()))
    {
        renderTile(viewport, projectionMat.matrix(), viewMat, m_tiles[m_selectedTileIndex], true);
    }
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
        m_hoverTileIndex = -1;
        for (int tileIndex = 0; tileIndex < int(m_tiles.size()); ++tileIndex)
        {
            GalleryTile& tile = m_tiles[tileIndex];
            tile.hover = 0.0f;
        }
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
    const float centerOffset = (1.0f / m_scale) * (viewPlaneWidth / 2.0f) / tan(m_cameraFov / 2.0f) - m_galleryRadius * cos(m_galleryAngle / 2.0f);

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
        planeNormal = AngleAxisf(float(-theta), Vector3f::UnitY()) * planeNormal;

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


/** Allocate OpenGL resources required to draw tiles.
 */
void
GalleryView::initGL()
{
    // Simple shader for drawing transformed, textured geometry.
    const char* vertexShaderSource =
#ifndef VESTA_OGLES2
    "#define lowp\n"
    "#define highp\n"
#endif
    "attribute vec4 position;\n"
    "attribute vec2 texCoord;\n"
    "uniform mat4 mvp;\n"
    "uniform vec4 color;\n"
    "varying lowp vec4 vertexColor;\n"
    "varying highp vec2 v_texCoord;\n"
    "void main()\n"
    "{\n"
    "    vertexColor = color;\n"
    "    v_texCoord = texCoord;\n"
    "    gl_Position = mvp * position;\n"
    "}\n";

    const char* fragmentShaderSource =
#ifndef VESTA_OGLES2
    "#define lowp\n"
    "#define highp\n"
#endif
    "varying lowp vec4 vertexColor;\n"
    "varying highp vec2 v_texCoord;\n"
    "uniform sampler2D baseTex;\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = vertexColor * texture2D(baseTex, v_texCoord);\n"
    "}\n";

    GLShader* vertexShader = new GLShader(GLShader::VertexStage);
    if (!vertexShader->compile(vertexShaderSource))
    {
        std::cerr << "Error message(s):\n";
        std::cerr << vertexShader->compileLog().c_str() << std::endl;
        delete vertexShader;
    }

    GLShader* fragmentShader = new GLShader(GLShader::FragmentStage);
    if (!fragmentShader->compile(fragmentShaderSource))
    {
        std::cerr << "Error message(s):\n";
        std::cerr << fragmentShader->compileLog().c_str() << std::endl;
        delete fragmentShader;
    }

    m_tileShader = new GLShaderProgram();
    m_tileShader->addShader(vertexShader);
    m_tileShader->addShader(fragmentShader);

    // Bind vertex attributes
    m_tileShader->bindAttribute("position", 0);
    m_tileShader->bindAttribute("texCoord", 1);

    m_tileShader->link();
}


/** Release OpenGL resources. */
void
GalleryView::finishGL()
{
    m_tileShader = NULL;
    m_textShader = NULL;
}
