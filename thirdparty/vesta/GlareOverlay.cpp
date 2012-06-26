/*
 * $Revision: 565 $ $Date: 2011-02-15 16:00:43 -0800 (Tue, 15 Feb 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "GlareOverlay.h"
#include "OGLHeaders.h"
#include "Units.h"
#include "PrimitiveBatch.h"
#include <algorithm>

using namespace vesta;
using namespace Eigen;
using namespace std;


GlareOverlay::GlareOverlay() :
    m_adaptationRate(0.15f),
    m_glareSize(100.0f)
{
}


bool
GlareOverlay::initialize()
{
#ifdef VESTA_OGLES2
    // On OpenGL ES 2.0, we use a simplified glare model that
    // doesn't require occlusion queries. No need to allocate
    // anything, just report success.
    return true;
#else
    if (!GLEW_ARB_occlusion_query)
    {
        // Extension isn't supported
        return false;
    }

    GLint bitsSupported = 0;
    glGetQueryivARB(GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS_ARB, &bitsSupported);
    if (bitsSupported == 0)
    {
        return false;
    }

    m_freeOcclusionQueries.resize(8);
    for (unsigned int i = 0; i < m_freeOcclusionQueries.size(); ++i)
    {
        GLuint id = 0;
        glGenQueriesARB(1, &id);
        m_freeOcclusionQueries[i] = id;
    }

    return true;
#endif
}


GlareOverlay::~GlareOverlay()
{
#ifndef VESTA_OGLES2
    // Clean up free (inactive) queries
    for (unsigned int i = 0; i < m_freeOcclusionQueries.size(); ++i)
    {
        GLuint id = m_freeOcclusionQueries[i];
        if (id != 0)
        {
            glDeleteQueriesARB(1, &id);
        }
    }

    // Clean up active query objects
    for (vector<GlareItem>::iterator iter = m_activeGlareItems.begin(); iter != m_activeGlareItems.end(); ++iter)
    {
        if (iter->m_occlusionQuery != 0)
        {
            glDeleteQueriesARB(1, &iter->m_occlusionQuery);
        }
    }
#endif
}


/** Change the brightness of glare for light sources that have recently changed visibility.
  * Glare from light sources that have recently become occluded will fade, while the glare
  * from newly revealed light sources will increase to full brightness.
  */
void
GlareOverlay::adjustBrightness()
{
#ifndef VESTA_OGLES2
    // The glare overlay uses occlusion queries to detect which light sources
    // are directly visible to the viewer. The result of an occlusion query is
    // not immediately available because the query is queued in the OpenGL command
    // stream. We may have to wait until the next frame (or even the one after that)
    // until the occlusion query has completed. Stalling the CPU until the query is
    // complete can hurt performance dramatically, so we use the result of
    // queries from the previous frame. The result of this is that glare is not switched
    // on and off instantly. There is a short lag between the time a light becomes
    // visible and when its glare reaches full intensity. In order to avoid abrupt
    // flashing of the glare, the code below will interpolate the glare brightness between
    // the on and off state.
    for (vector<GlareItem>::iterator iter = m_activeGlareItems.begin(); iter != m_activeGlareItems.end(); ++iter)
    {
        if (iter->m_occlusionQuery != 0)
        {
            float adjustment = 0.0f;

            // See if the occlusion query result is ready
            GLuint queryId = iter->m_occlusionQuery;
            GLint available = 0;
            glGetQueryObjectivARB(queryId, GL_QUERY_RESULT_AVAILABLE_ARB, &available);

            if (available != 0)
            {
                // Occlusion query is complete; adjust the glare brightness based on the
                // result.
                GLuint sampleCount = 0;
                glGetQueryObjectuivARB(queryId, GL_QUERY_RESULT_ARB, &sampleCount);
                iter->m_occlusionQuery = 0;
                m_freeOcclusionQueries.push_back(queryId);
                if (sampleCount > 0)
                {
                    adjustment = m_adaptationRate;
                }
                else
                {
                    adjustment = -m_adaptationRate;
                }
            }
            else
            {
                // Occlusion query result isn't available yet; don't modify glare
                // intensity up or down.
                adjustment = 0.0f;
            }

            iter->m_brightness = max(0.0f, min(1.0f, iter->m_brightness + adjustment));
        }
    }
#endif
}


void
GlareOverlay::trackGlare(RenderContext& rc, const LightSource* lightSource, const Vector3f& glarePosition, float lightRadius)
{
#ifdef VESTA_OGLES2
    // Occlusion queries aren't supported on OpenGL ES 2.0, so we'll just
    // draw the glare geometry instead of the occlusion test geometry.

    // Enforce the minimum pixel size
    float distance = glarePosition.norm();
    float sizeInPixels = lightRadius * 8.0f / (distance * rc.pixelSize());
    sizeInPixels = max(sizeInPixels, m_glareSize);
    
    Material material;
    material.setDiffuse(Spectrum::Black());
    material.setEmission(Spectrum::White());
    material.setBlendMode(Material::AdditiveBlend);
    material.setOpacity(0.99f);
    material.setBaseTexture(lightSource->glareTexture());
    rc.bindMaterial(&material);
    
    drawGlareGeometry(rc, glarePosition, sizeInPixels * rc.pixelSize() * distance);
#else
    GlareItem* item = NULL;
    bool busy = false;
    for (vector<GlareItem>::iterator iter = m_activeGlareItems.begin(); iter != m_activeGlareItems.end(); ++iter)
    {
        if (iter->m_lightSource.ptr() == lightSource)
        {
            if (iter->m_occlusionQuery != 0)
            {
                busy = true;
            }
            item = &(*iter);
            break;
        }
    }

    if (item == NULL)
    {
        GlareItem newItem;
        m_activeGlareItems.push_back(newItem);
        item = &m_activeGlareItems.back();
    }

    // We need to ensure that when the GPU rasterizes the test geometry, at
    // least one pixel will be drawn. Otherwise, the occlusion query will always
    // fail and no glare will be drawn.
    const float minimumSizeInPixels = 1.5f;

    if (!busy)
    {
        // const cast required because of limitations of counted_ptr
        item->m_lightSource = const_cast<LightSource*>(lightSource);
        item->m_occlusionQuery = getFreeOcclusionQuery();

        // Enforce the minimum pixel size
        float distance = glarePosition.norm();
        float sizeInPixels = lightRadius / (distance * rc.pixelSize());
        sizeInPixels = max(sizeInPixels, minimumSizeInPixels);

        glBeginQueryARB(GL_SAMPLES_PASSED_ARB, item->m_occlusionQuery);
        drawOcclusionTestGeometry(rc, glarePosition, sizeInPixels * rc.pixelSize() * distance);
        glEndQueryARB(GL_SAMPLES_PASSED_ARB);
    }
#endif
}


void
GlareOverlay::renderGlare(RenderContext& rc, const LightSource* lightSource, const Vector3f& glarePosition, float lightRadius)
{
#ifndef VESTA_OGLES2
    GlareItem* item = NULL;
    for (vector<GlareItem>::iterator iter = m_activeGlareItems.begin(); iter != m_activeGlareItems.end(); ++iter)
    {
        if (iter->m_lightSource.ptr() == lightSource)
        {
            item = &(*iter);
            break;
        }
    }

    if (item != NULL)
    {
        TextureMap* glareTexture = lightSource->glareTexture();
        if (glareTexture && glareTexture->makeResident())
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glColor4f(1.0f, 1.0f, 1.0f, item->m_brightness);
            glBindTexture(GL_TEXTURE_2D, glareTexture->id());
            glEnable(GL_TEXTURE_2D);

            // Compute the size of the glare sprite. We want minimum projected radius
            // to be m_glareSize pixels, but it may be larger if the projected size
            // of the light source geometry is bigger.
            float distance = glarePosition.norm();
            float sizeInPixels = lightRadius * 8.0f / (distance * rc.pixelSize());
            sizeInPixels = max(sizeInPixels, m_glareSize);

            drawGlareGeometry(rc, glarePosition, sizeInPixels * rc.pixelSize() * distance);
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_BLEND);
        }
    }
#endif
}


void
GlareOverlay::drawOcclusionTestGeometry(RenderContext& /* rc */, const Vector3f& position, float lightRadius)
{
#ifndef VESTA_OGLES2
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3fv(position.data());
    const unsigned int sliceCount = 30;
    for (unsigned int j = 0; j <= sliceCount; ++j)
    {
        float theta = float(j) / float(sliceCount) * 2.0f * float(PI);
        Vector3f v = position + Vector3f(cos(theta), sin(theta), 0.0f) * lightRadius;
        glVertex3fv(v.data());
    }
    glEnd();
    glDisable(GL_BLEND);
#endif
}


void
GlareOverlay::drawGlareGeometry(RenderContext& rc, const Vector3f& position, float glareRadius)
{
    const unsigned int sliceCount = 30;
#ifdef VESTA_OGLES2
    const unsigned int vertexCount = sliceCount + 2;
    float vertexData[vertexCount * 5];

    vertexData[0] = position.x();
    vertexData[1] = position.y();
    vertexData[2] = position.z();
    vertexData[3] = 0.5f;
    vertexData[4] = 0.5f;
    
    for (unsigned int j = 0; j <= sliceCount; ++j)
    {
        float theta = float(j) / float(sliceCount) * 2.0f * float(PI);
        float s = sin(theta);
        float c = cos(theta);
        Vector3f v = position + Vector3f(c, s, 0.0f) * glareRadius;
        Vector2f texCoord(0.5f + 0.5f * c, 0.5f + 0.5f * s);
        unsigned int baseIndex = (j + 1) * 5;
        vertexData[baseIndex + 0] = v.x();
        vertexData[baseIndex + 1] = v.y();
        vertexData[baseIndex + 2] = v.z();
        vertexData[baseIndex + 3] = texCoord.x();
        vertexData[baseIndex + 4] = texCoord.y();
    }
    
    rc.bindVertexArray(VertexSpec::PositionTex, vertexData, VertexSpec::PositionTex.size());
    rc.drawPrimitives(PrimitiveBatch(PrimitiveBatch::TriangleFan, sliceCount, 0));
#else
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0.5f, 0.5f);
    glVertex3fv(position.data());
    for (unsigned int j = 0; j <= sliceCount; ++j)
    {
        float theta = float(j) / float(sliceCount) * 2.0f * float(PI);
        float s = sin(theta);
        float c = cos(theta);
        Vector3f v = position + Vector3f(c, s, 0.0f) * glareRadius;
        glTexCoord2f(0.5f + 0.5f * c, 0.5f + 0.5f * s);
        glVertex3fv(v.data());
    }
    glEnd();
    glEnable(GL_DEPTH_TEST);
#endif
}


GLuint
GlareOverlay::getFreeOcclusionQuery()
{
    if (m_freeOcclusionQueries.empty())
    {
        return 0;
    }
    else
    {
        GLuint id = m_freeOcclusionQueries.back();
        m_freeOcclusionQueries.pop_back();
        return id;
    }
}
