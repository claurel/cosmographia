/*
 * $Revision: 654 $ $Date: 2012-02-22 16:22:33 -0800 (Wed, 22 Feb 2012) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "ArrowGeometry.h"
#include "RenderContext.h"
#include "Material.h"
#include "Units.h"
#include "IntegerTypes.h"
#include <Eigen/Core>
#include <cassert>
#include "DataChunk.h"
#include "Debug.h"
#include <fstream>

using namespace vesta;
using namespace Eigen;
using namespace std;


static const unsigned int ARROW_SECTIONS = 20;


ArrowGeometry::ArrowGeometry(float shaftLength,
                             float shaftRadius,
                             float headLength,
                             float headRadius) :
    m_scale(1.0),
    m_opacity(1.0f),
    m_visibleArrows(XAxis),
    m_geometryBoundingRadius(0.0f),
    m_cap(0),
    m_shaft(0),
    m_annulus(0),
    m_point(0),
    m_vertices(0),
    m_font(NULL)
{    
    m_arrowColors[0] = Spectrum(1.0f, 0.0f, 0.0f);
    m_arrowColors[1] = Spectrum(0.0f, 1.0f, 0.0f);
    m_arrowColors[2] = Spectrum(0.0f, 0.0f, 1.0f);
    buildArrowGeometry(shaftLength, shaftRadius, headLength, headRadius);

    m_labels[0] = "X";
    m_labels[1] = "Y";
    m_labels[2] = "Z";
    for(int i = 0; i < 3; i++)
    {
        m_labelsEnabled[i] = false;
    }
}


ArrowGeometry::~ArrowGeometry()
{
    delete m_cap;
    delete m_shaft;
    delete m_annulus;
    delete m_point;
    delete m_vertices;
}


Spectrum
ArrowGeometry::arrowColor(unsigned int which) const
{
    if (which < 3)
    {
        return m_arrowColors[which];
    }
    else
    {
        return Spectrum();
    }
}


/** Set the color of the one of the arrows.
 *
 *  @param which a value between 0 and 2 (inclusive), with x = 0, y = 1, z = 2
 *  @param color the color of the arrow
 */
void
ArrowGeometry::setArrowColor(unsigned int which, const Spectrum& color)
{
    if (which < 3)
    {
        m_arrowColors[which] = color;
    }
}

/** Enables/Disables the drawing of labels for an arrow specified by which
  */
void ArrowGeometry::setLabelEnabled(bool state, unsigned int which)
{
    switch(which)
    {
    case XAxis:
        m_labelsEnabled[0] = state;
        break;
    case YAxis:
        m_labelsEnabled[1] = state;
        break;
    case ZAxis:
        m_labelsEnabled[2] = state;
        break;
    case AllAxes:
        m_labelsEnabled[0] = state;
        m_labelsEnabled[1] = state;
        m_labelsEnabled[2] = state;
        break;
    default:
        break;
    }
}


/** Sets a font for the label text.
  */
void ArrowGeometry::setLabelFont(TextureFont* font)
{
    m_font = font;
}


/** Sets the text of the label for an arrow specified by which
  */
void ArrowGeometry::setLabelText(string text, unsigned int which)
{
    switch(which)
    {
    case XAxis:
        m_labels[0] = text;
        break;
    case YAxis:
        m_labels[1] = text;
        break;
    case ZAxis:
        m_labels[2] = text;
        break;
    case AllAxes:
        m_labels[0] = text;
        m_labels[1] = text;
        m_labels[2] = text;
        break;
    default:
        break;
    }
}

void
ArrowGeometry::render(RenderContext& rc,
                      double /* clock */) const
{
    Material materials[3];
    for (unsigned int i = 0; i < 3; i++)
    {
        materials[i].setEmission(m_arrowColors[i]);
        materials[i].setOpacity(m_opacity);
    }

    rc.bindVertexArray(m_vertices);

    rc.pushModelView();
    rc.scaleModelView(Vector3f::Constant((float) m_scale));

    // Arrows are drawn in the opaque pass if they're completely opaque and the translucent pass
    // otherwise. The anti-aliased fonts used for labels need to be blended with the background
    // for the best appearance, so they're always drawn in the translucent pass.
    bool opaqueArrows = opacity() >= 1.0f;

    // Draw arrows
    if ((rc.pass() == RenderContext::TranslucentPass) ^ opaqueArrows)
    {
        if (visibleArrows() & XAxis)
        {
            rc.pushModelView();
            rc.rotateModelView(Quaternionf(AngleAxisf((float) PI / 2.0f, Vector3f::UnitY())));
            rc.bindMaterial(&materials[0]);
            drawArrow(rc);
            drawLabel(rc, 0);
            rc.popModelView();
        }

        if (visibleArrows() & YAxis)
        {
            rc.pushModelView();
            rc.rotateModelView(Quaternionf(AngleAxisf(-(float) PI / 2.0f, Vector3f::UnitX())));
            rc.bindMaterial(&materials[1]);
            drawArrow(rc);
            drawLabel(rc, 1);
            rc.popModelView();
        }

        if (visibleArrows() & ZAxis)
        {
            rc.pushModelView();
            rc.bindMaterial(&materials[2]);
            drawArrow(rc);
            drawLabel(rc, 2);
            rc.popModelView();
        }
    }

    // Draw labels in the translucent pass
    if (rc.pass() == RenderContext::TranslucentPass)
    {
        if (visibleArrows() & XAxis)
        {
            rc.pushModelView();
            rc.rotateModelView(Quaternionf(AngleAxisf((float) PI / 2.0f, Vector3f::UnitY())));
            rc.bindMaterial(&materials[0]);
            drawLabel(rc, 0);
            rc.popModelView();
        }

        if (visibleArrows() & YAxis)
        {
            rc.pushModelView();
            rc.rotateModelView(Quaternionf(AngleAxisf(-(float) PI / 2.0f, Vector3f::UnitX())));
            rc.bindMaterial(&materials[1]);
            drawLabel(rc, 1);
            rc.popModelView();
        }

        if (visibleArrows() & ZAxis)
        {
            rc.pushModelView();
            rc.bindMaterial(&materials[2]);
            drawLabel(rc, 2);
            rc.popModelView();
        }
    }

    rc.unbindVertexArray();

    rc.popModelView(); // pop scaling transformation    
}


float
ArrowGeometry::boundingSphereRadius() const
{
    return m_geometryBoundingRadius * (float) m_scale;
}


void
ArrowGeometry::drawArrow(RenderContext& rc) const
{
    rc.drawPrimitives(*m_cap);
    rc.drawPrimitives(*m_shaft);
    rc.drawPrimitives(*m_annulus);
    rc.drawPrimitives(*m_point);
}


void
ArrowGeometry::buildArrowGeometry(float shaftLength,
                                  float shaftRadius,
                                  float headLength,
                                  float headRadius)
{
    const unsigned int vertexCount = (ARROW_SECTIONS + 1) * 3 + 2;
    const unsigned int capCenterIndex = vertexCount - 2;
    const unsigned int tipIndex = vertexCount - 1;

    // Vertex data must be allocated as char, not Vector3; Eigen uses a custom allocator for
    // objects.
    unsigned char* byteData = new unsigned char[vertexCount * sizeof(Vector3f)];
    Vector3f* data = reinterpret_cast<Vector3f*>(byteData);//new Vector3f[vertexCount];
    int vertexIndex = 0;

    // End cap
    for (unsigned int i = 0; i <= ARROW_SECTIONS; ++i)
    {
        float theta = (float) i / (float) ARROW_SECTIONS * 2 * (float) PI;
        data[vertexIndex++] = Vector3f(std::cos(theta) * shaftRadius, std::sin(theta) * shaftRadius, 0.0f);
    }

    // Shaft
    for (unsigned int i = 0; i <= ARROW_SECTIONS; ++i)
    {
        float theta = (float) i / (float) ARROW_SECTIONS * 2 * (float) PI;
        data[vertexIndex++] = Vector3f(std::cos(theta) * shaftRadius, std::sin(theta) * shaftRadius, shaftLength);
    }

    // Annulus
    for (unsigned int i = 0; i <= ARROW_SECTIONS; ++i)
    {
        float theta = (float) i / (float) ARROW_SECTIONS * 2 * (float) PI;
        data[vertexIndex++] = Vector3f(std::cos(theta) * headRadius, std::sin(theta) * headRadius, shaftLength);
    }

    data[vertexIndex++] = Vector3f::Zero();
    data[vertexIndex++] = Vector3f(0.0f, 0.0f, shaftLength + headLength);

    // Calculate the bounding radius of the arrow geometry
    m_geometryBoundingRadius = 0.0f;
    for (unsigned i = 0; i < vertexCount; i++)
    {
        m_geometryBoundingRadius = max(m_geometryBoundingRadius, data[i].norm());
    }

    m_vertices = new VertexArray(data, vertexCount, VertexSpec::Position, sizeof(data[0]));

    // End cap of the arrow shaft
    v_uint16* capIndices = new v_uint16[ARROW_SECTIONS + 2];
    capIndices[0] = (v_uint16) capCenterIndex;
    for (unsigned int i = 0; i <= ARROW_SECTIONS; i++)
    {
        capIndices[i + 1] = (v_uint16) (ARROW_SECTIONS - i);
    }
    m_cap = new PrimitiveBatch(PrimitiveBatch::TriangleFan, capIndices, ARROW_SECTIONS);
    delete[] capIndices;

    // Shaft of the arrow
    v_uint16* shaftIndices = new v_uint16[ARROW_SECTIONS * 2 + 2];
    for (unsigned int i = 0; i <= ARROW_SECTIONS; i++)
    {
        shaftIndices[i * 2    ] = (v_uint16) (ARROW_SECTIONS + 1 + i);
        shaftIndices[i * 2 + 1] = (v_uint16) i;
    }
    m_shaft = new PrimitiveBatch(PrimitiveBatch::TriangleStrip, shaftIndices, ARROW_SECTIONS * 2);
    delete[] shaftIndices;

    // Annulus (ring connecting head of arrow to the shaft)
    v_uint16* annulusIndices = new v_uint16[ARROW_SECTIONS * 2 + 2];
    for (unsigned int i = 0; i <= ARROW_SECTIONS; i++)
    {
        annulusIndices[i * 2    ] = (v_uint16) ((ARROW_SECTIONS + 1) * 2 + i);
        annulusIndices[i * 2 + 1] = (v_uint16) ((ARROW_SECTIONS + 1) + i);
    }
    m_annulus = new PrimitiveBatch(PrimitiveBatch::TriangleStrip, annulusIndices, ARROW_SECTIONS * 2);
    delete[] annulusIndices;

    // Point of the arrow
    v_uint16* pointIndices = new v_uint16[ARROW_SECTIONS + 2];
    pointIndices[0] = (v_uint16) tipIndex;
    for (unsigned int i = 0; i <= ARROW_SECTIONS; i++)
    {
        pointIndices[i + 1] = (v_uint16) ((ARROW_SECTIONS + 1) * 2 + i);
    }
    m_point = new PrimitiveBatch(PrimitiveBatch::TriangleFan, pointIndices, ARROW_SECTIONS);
    delete[] pointIndices;
}

void ArrowGeometry::drawLabel(RenderContext& rc, unsigned int which) const
{

    Vector3f arrowOriginScreenSpace;
    Vector3f arrowHeadScreenSpace;
    Vector3f labelPositionScreenSpace;
    float cameraDistance;
    float pixelSize;
    counted_ptr<TextureFont> font = m_font;
    if (font.isNull())
    {
        font = rc.defaultFont();
    }

    if (font.isNull() || !m_labelsEnabled[which])
    {
        return;
    }

    arrowOriginScreenSpace = rc.projection() * rc.modelview() * Vector3f(0.0, 0.0, 0.0);
    arrowHeadScreenSpace = rc.projection() * rc.modelview() * Vector3f(0.0, 0.0, 1.0);

    // note: length of an arrow is 0.5
    labelPositionScreenSpace.x() = 0.5f * ( arrowHeadScreenSpace.x() - arrowOriginScreenSpace.x() ) *
                                   rc.viewportWidth();
    labelPositionScreenSpace.y() = 0.5f * ( arrowHeadScreenSpace.y() - arrowOriginScreenSpace.y() ) *
                                   rc.viewportHeight();
    labelPositionScreenSpace.z() = 0.0f;// no need for a z value


    // move the label to the left, otherwise the label will be drawn on top of the arrow
    Vector3f labelOffset = Vector3f::Zero();
    if(labelPositionScreenSpace.x() < 0)
    {
        labelOffset.x() -= font->textWidth(m_labels[which]);
    }

    // move the label downwards, otherwise the label will be drawn on top of the arrow
    // two times the textWidth of the uppercase character A is sufficent
    if(labelPositionScreenSpace.y() < 0)
    {
        labelOffset.y() -= 2.0f * font->textWidth("A");
    }

    cameraDistance = rc.modelview().translation().norm();
    pixelSize = float(0.5 * m_scale / (rc.pixelSize() * cameraDistance));

    if (pixelSize >= 10.0f)
    {
        rc.pushModelView();
        rc.translateModelView(Vector3f::UnitZ());
        rc.drawText(labelOffset, m_labels[which], font.ptr(), m_arrowColors[which], m_opacity);
        rc.popModelView();
    }
}
