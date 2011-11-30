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
#include <Eigen/Core>
#include <cmath>

using namespace vesta;
using namespace Eigen;
using namespace std;


GalleryView::GalleryView()
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
drawRoundRectangle(float width, float height, float radius)
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
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


void
GalleryView::render(const Viewport& viewport)
{
    const unsigned int rows = 4;
    const unsigned int columns = 10;
    const float galleryRadius = 50.0f;
    const float galleryHeight = 12.0f;
    const float galleryAngle = toRadians(40.0f);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(45.0, viewport.aspectRatio(), 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 15.0f);

    glEnable(GL_TEXTURE_2D);

    static float deg = 0.0f;
    deg += 1.0f;
    for (unsigned int i = 0; i < rows; ++i)
    {
        float y = (float(i) / float(rows - 1) - 0.5f) * galleryHeight;

        for (unsigned int j = 0; j < columns; ++j)
        {
            double theta = (float(j) / float(columns - 1) - 0.5f) * galleryAngle - toRadians(90.0);

            float x = galleryRadius * cos(theta);
            float z = galleryRadius * sin(theta);

            glPushMatrix();
            glTranslatef(x, y, z);
            glRotatef(-(toDegrees(theta) + 90.0f), 0.0f, 1.0f, 0.0f);
            glScalef(1.5f, 1.5f, 1.5f);

            unsigned int tileIndex = i * columns + j;
            const GalleryTile& tile = m_tiles[tileIndex % m_tiles.size()];

            if (tile.texture.isValid() && tile.texture->makeResident())
            {
                glBindTexture(GL_TEXTURE_2D, tile.texture->id());
            }

            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            /*
            glBegin(GL_QUADS);
            glVertex3f(-1.0f, -1.0f, 0.0f);
            glVertex3f( 1.0f, -1.0f, 0.0f);
            glVertex3f( 1.0f,  1.0f, 0.0f);
            glVertex3f(-1.0f,  1.0f, 0.0f);
            glEnd();
            */
            drawRoundRectangle(2.0f, 2.0f, 0.3f);

            glPopMatrix();
        }
    }

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}


void
GalleryView::addTile(TextureMap* texture)
{
    GalleryTile tile;
    tile.texture = texture;
    m_tiles.push_back(tile);
}
