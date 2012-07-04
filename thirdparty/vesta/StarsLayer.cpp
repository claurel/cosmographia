/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "StarsLayer.h"
#include "RenderContext.h"
#include "OGLHeaders.h"
#include "ShaderBuilder.h"
#include "Debug.h"
#include "glhelp/GLVertexBuffer.h"
#include "glhelp/GLShaderProgram.h"
#include <string>
#include <cmath>

using namespace vesta;
using namespace Eigen;
using namespace std;


// Star shader GLSL source
//
// Stars are drawn as the sum of a Gaussian and a power function. The Gaussian is the:
// the convolution of the pixel function and the point spread function (both of which
// are themselves modeled as Gaussians.) The glare function gives a halo effect for
// bright stars; the physical cause of this effect is scattering of light in the eye
// or reflections within the optical system of camera.
//
// The point size is computed so that it is just large enough to fit the Gaussian
// disc and glare function for the star. This keeps the number of pixels drawn for
// faint stars very low, which saves fill rate and pixel shader cycles.
//
// Before fragment color is generated, it is mapped from linear to sRGB color space.
// This mapping is unecessary if the EXT_framebuffer_sSRGB extension is enabled.
//
// In order to keep the per-vertex storage at 16 bytes, the following layout is used:
//   x - 32-bit float
//   y - 32-bit float
//   magnitude - 32-bit float
//   color     - 4x8-bit unsigned normalized values
//
// Since the stars lie on a sphere, the z coordinate is computed as sqrt(1-x^2-y^2);
// the sign of z is actually stored in the alpha channel of the color.

#ifdef VESTA_OGLES2
static const char* StarVertexShaderSource =
"attribute vec3 vesta_Position;\n"
"attribute vec4 vesta_Color;\n"
"uniform mat4 vesta_ModelViewProjectionMatrix;\n"
"uniform vec2 viewportSize;       \n"
"uniform vec2 viewportCoord;      \n"
"varying vec2 pointCenter;        \n"
"varying vec4 color;              \n"
"varying float brightness;        \n"
"uniform float Llim;              \n"
"uniform float Lsat;              \n"
"uniform float magScale;          \n"
"uniform float sigma2;            \n"
"uniform highp float glareFalloff;      \n"
"uniform highp float glareBrightness;   \n"
"uniform float exposure;          \n"
"uniform float thresholdBrightness;\n"
"void main()                      \n"
"{                                \n"
"    vec4 position = vec4(vesta_Position, 1.0);                                  \n"
"    float appMag = position.z;                                               \n"
"    position.z = sqrt(1.0 - dot(position.xy, position.xy)) * sign(vesta_Color.a - 0.5);\n"
"    vec4 projectedPosition = vesta_ModelViewProjectionMatrix * position;        \n"
"    vec2 devicePosition = projectedPosition.xy / projectedPosition.w;        \n"
"    pointCenter = (devicePosition * 0.5 + vec2(0.5, 0.5)) * viewportSize + viewportCoord;    \n"
"    color = vesta_Color;                                                        \n"
"    float b = pow(2.512, -appMag * magScale);\n"
"    float r2 = -log(thresholdBrightness / (exposure * b)) * 2.0 * sigma2;          \n"
"    float rGlare2 = (exposure * glareBrightness * b / thresholdBrightness - 1.0) / glareFalloff;     \n"
"    gl_PointSize = 2.0 * sqrt(max(r2, rGlare2));                             \n"

"    brightness = b;                                                          \n"
"    gl_Position = projectedPosition;                                         \n"
"}                                \n";

// Note that most of the uniform, varying, and temporary variables must
// be high precision; otherwise the PowerVR shader compiler aggressively
// reduces precision even in places where it's actually required.
static const char* StarFragmentShaderSource =
"varying lowp vec4 color;                        \n"
"varying highp vec2 pointCenter;                 \n"
"varying highp float brightness;                 \n"
"uniform highp float sigma2;                     \n"
"uniform highp float glareFalloff;             \n"
"uniform highp float glareBrightness;          \n"
"uniform highp float exposure;                   \n"

"void main()                                     \n"
"{                                               \n"
"    highp vec2 offset = gl_FragCoord.xy - pointCenter;          \n"
"    highp float r2 = dot(offset, offset);                       \n"
"    highp float b = exp(-r2 / (2.0 * sigma2));                \n"
"    b += glareBrightness / (glareFalloff * pow(r2, 1.5) + 1.0) * 0.5;     \n"
"    gl_FragColor = vec4(linearToSRGB(b * exposure * color.rgb * brightness), 1.0);   \n"
"}                                                               \n"
;

#else
static const char* StarVertexShaderSource =
"uniform vec2 viewportSize;       \n"
"uniform vec2 viewportCoord;      \n"
"varying vec2 pointCenter;        \n"
"varying vec4 color;              \n"
"varying float brightness;        \n"
"uniform float Llim;              \n"
"uniform float Lsat;              \n"
"uniform float magScale;          \n"
"uniform float sigma2;            \n"
"uniform float glareFalloff;      \n"
"uniform float glareBrightness;   \n"
"uniform float exposure;          \n"
"uniform float thresholdBrightness;\n"
"void main()                      \n"
"{                                \n"
"    vec4 position = gl_Vertex;                                               \n"
"    float appMag = position.z;                                               \n"
"    position.z = sqrt(1.0 - dot(position.xy, position.xy)) * sign(gl_Color.a - 0.5);\n"
"    vec4 projectedPosition = gl_ModelViewProjectionMatrix * position;        \n"
"    vec2 devicePosition = projectedPosition.xy / projectedPosition.w;        \n"
"    pointCenter = (devicePosition * 0.5 + vec2(0.5, 0.5)) * viewportSize + viewportCoord;    \n"
"    color = gl_Color;                                                        \n"
"    float b = pow(2.512, -appMag * magScale);\n"
"    float r2 = -log(thresholdBrightness / (exposure * b)) * 2.0 * sigma2;          \n"
"    float rGlare2 = (exposure * glareBrightness * b / thresholdBrightness - 1.0) / glareFalloff;     \n"
"    gl_PointSize = 2.0 * sqrt(max(r2, max(0.25, rGlare2)));                   \n"

"    brightness = b;                                                          \n"
"    gl_Position = projectedPosition;                                         \n"
"}                                \n"
;

static const char* StarFragmentShaderSource =
"varying vec2 pointCenter;                       \n"
"varying vec4 color;                             \n"
"uniform float sigma2;                           \n"
"uniform float glareFalloff;                     \n"
"uniform float glareBrightness;                  \n"
"uniform float diffSpikeBrightness;              \n"
"uniform float exposure;                         \n"
"varying float brightness;                       \n"

"void main()                                     \n"
"{                                               \n"
"    vec2 offset = gl_FragCoord.xy - pointCenter;                \n"
"    float r2 = dot(offset, offset);                             \n"
"    float b = exp(-r2 / (2.0 * sigma2));                        \n"
"    float spikes = (max(0.0, 1.0 - abs(offset.x + offset.y)) + max(0.0, 1.0 - abs(offset.x - offset.y))) * diffSpikeBrightness;\n"
"    b += glareBrightness / (glareFalloff * pow(r2, 1.5) + 1.0) * (spikes + 0.5);     \n"
"    gl_FragColor = vec4(linearToSRGB(b * exposure * color.rgb * brightness), 1.0);   \n"
"}                                                               \n"
;
#endif

#ifdef VESTA_OGLES2
static const char* LinearToSRGBSource =
"mediump vec3 linearToSRGB(mediump vec3 c)               \n"
"{                                                    \n"
"    mediump vec3 linear = 12.92 * c;                 \n"
"    mediump vec3 nonlinear = (1.0 + 0.055) * pow(c, vec3(1.0 / 2.4)) - vec3(0.055);\n"
"    return mix(linear, nonlinear, step(vec3(0.0031308), c));\n"
"}                                               \n"
;

static const char* PassthroughSRGBSource =
"highp vec3 linearToSRGB(highp vec3 c)          \n"
"{                                               \n"
"    return c;                                   \n"
"}                                               \n"
;
#else
static const char* LinearToSRGBSource =
"vec3 linearToSRGB(vec3 c)                       \n"
"{                                               \n"
"    vec3 linear = 12.92 * c;                    \n"
"    vec3 nonlinear = (1.0 + 0.055) * pow(c, vec3(1.0 / 2.4)) - vec3(0.055);\n"
"    return mix(linear, nonlinear, step(vec3(0.0031308), c));\n"
"}                                               \n"
;

static const char* PassthroughSRGBSource =
"vec3 linearToSRGB(vec3 c)                       \n"
"{                                               \n"
"    return c;                                   \n"
"}                                               \n"
;
#endif




// End star shader

static const float DefaultLimitingMagnitude = 7.0f;

StarsLayer::StarsLayer() :
    m_vertexArray(NULL),
    m_vertexBufferCurrent(false),
    m_starShaderCompiled(false),
    m_style(GaussianStars),
    m_limitingMagnitude(DefaultLimitingMagnitude),
    m_diffractionSpikeBrightness(0.0f)
{
}


StarsLayer::StarsLayer(StarCatalog* starCatalog) :
    m_starCatalog(starCatalog),
    m_vertexArray(NULL),
    m_vertexBufferCurrent(false),
    m_starShaderCompiled(false),
    m_style(GaussianStars),
    m_limitingMagnitude(DefaultLimitingMagnitude),
    m_diffractionSpikeBrightness(0.0f)
{
}


StarsLayer::~StarsLayer()
{
    if (m_vertexArray)
    {
        delete[] m_vertexArray;
    }
}


struct StarsLayerVertex
{
    float x;
    float y;
    float appMag;
    unsigned char color[4];
};

struct StarsLayerVertexFF
{
    float x;
    float y;
    float z;
    unsigned char color[4];
};


// Compute the position of a star on the unit sphere.
static Vector3f StarPositionCartesian(const StarCatalog::StarRecord& star)
{
    float cosDec = cos(star.declination);
    return Vector3f(cosDec * cos(star.RA), cosDec * sin(star.RA), sin(star.declination));
}


static void SpectrumToColor(const Spectrum& s, unsigned char color[])
{
    color[0] = (int) (255.0f * s.red() + 0.5f);
    color[1] = (int) (255.0f * s.green() + 0.5f);
    color[2] = (int) (255.0f * s.blue() + 0.5f);
}


static void SetStarColorSRGB(const StarCatalog::StarRecord& star, unsigned char color[])
{
    Spectrum cieXYZ = StarCatalog::StarColor(star.bvColorIndex);

    Spectrum srgb = Spectrum::XYZtoLinearSRGB(cieXYZ);
    srgb.normalize();
    srgb = Spectrum::LinearSRGBtoSRGB(srgb);
    SpectrumToColor(srgb, color);
}


static void SetStarColorLinearSRGB(const StarCatalog::StarRecord& star, unsigned char color[])
{
    Spectrum cieXYZ = StarCatalog::StarColor(star.bvColorIndex);

    Spectrum srgb = Spectrum::XYZtoLinearSRGB(cieXYZ);
    srgb.normalize();
    SpectrumToColor(srgb, color);
}


static void SetStarBrightness(const StarCatalog::StarRecord& star, float limMag, float satMag, StarsLayerVertexFF& v)
{
    float brightness = std::min(1.0f, std::max(0.0f, (limMag - star.apparentMagnitude) / (limMag - satMag)));
    v.color[3] = (unsigned char) (255.99f * brightness);
}


static char*
CreateStarVertexArrayFF(StarCatalog* starCatalog)
{
    if (starCatalog->size() == 0)
    {
        return NULL;
    }

    StarsLayerVertexFF* va = new StarsLayerVertexFF[starCatalog->size()];
    for (unsigned int i = 0; i < starCatalog->size(); ++i)
    {
        const StarCatalog::StarRecord& star = starCatalog->star(i);
        Vector3f position = StarPositionCartesian(star);
        va[i].x = position.x();
        va[i].y = position.y();
        va[i].z = position.z();
        SetStarColorSRGB(star, va[i].color);
        SetStarBrightness(star, 7.0f, 0.0f, va[i]);
    }

    return reinterpret_cast<char*>(va);
}


static char*
CreateStarVertexArray(StarCatalog* starCatalog)
{
    if (starCatalog->size() == 0)
    {
        return NULL;
    }

    StarsLayerVertex* va = new StarsLayerVertex[starCatalog->size()];
    for (unsigned int i = 0; i < starCatalog->size(); ++i)
    {
        const StarCatalog::StarRecord& star = starCatalog->star(i);
        Vector3f position = StarPositionCartesian(star);
        va[i].x = position.x();
        va[i].y = position.y();

        SetStarColorLinearSRGB(star, va[i].color);
        if (position.z() < 0.0f)
        {
            va[i].color[3] = 0;
        }
        else
        {
            va[i].color[3] = 255;
        }

        va[i].appMag = star.apparentMagnitude;
    }

    return reinterpret_cast<char*>(va);
}


static GLVertexBuffer*
CreateStarVertexBuffer(StarCatalog* starCatalog)
{
    char* buf = CreateStarVertexArray(starCatalog);
    if (buf)
    {
        GLVertexBuffer* vb = new GLVertexBuffer(sizeof(StarsLayerVertex) * starCatalog->size(), GL_STATIC_DRAW, buf);
        delete[] buf;
        return vb;
    }
    else
    {
        return NULL;
    }

}


// Create a star vertex buffer to use for the fixed function OpenGL pipe
static GLVertexBuffer*
CreateStarVertexBufferFF(StarCatalog* starCatalog)
{
    char* buf = CreateStarVertexArrayFF(starCatalog);
    if (buf)
    {
        GLVertexBuffer* vb =new  GLVertexBuffer(sizeof(StarsLayerVertexFF) * starCatalog->size(), GL_STATIC_DRAW, buf);
        delete[] buf;
        return vb;
    }
    else
    {
        return NULL;
    }
}


/** Set the catalog used by this star layer.
  */
void
StarsLayer::setStarCatalog(StarCatalog* starCatalog)
{
    if (m_starCatalog.ptr() != starCatalog)
    {
        m_vertexBufferCurrent = false;
        m_starCatalog = starCatalog;
    }
}


void
StarsLayer::render(RenderContext& rc)
{
    // Create the star shaders if they haven't already been compiled
    if (rc.shaderCapability() != RenderContext::FixedFunction && !m_starShaderCompiled)
    {
        string fragmentShaderSource = string(PassthroughSRGBSource) + StarFragmentShaderSource;
        m_starShader = GLShaderProgram::CreateShaderProgram(StarVertexShaderSource, fragmentShaderSource);

        // Create a version of the shader that does SRGB correction for configurations that don't
        // support the EXT_framebuffer_sRGB extension
        fragmentShaderSource = string(LinearToSRGBSource) + StarFragmentShaderSource;
        m_starShaderSRGB = GLShaderProgram::CreateShaderProgram(StarVertexShaderSource, fragmentShaderSource);

#ifdef VESTA_OGLES2
        if (m_starShaderSRGB.isValid())
        {
            m_starShaderSRGB->bindAttribute(ShaderBuilder::PositionAttribute, ShaderBuilder::PositionAttributeLocation);
            m_starShaderSRGB->bindAttribute(ShaderBuilder::ColorAttribute, ShaderBuilder::ColorAttributeLocation);
            m_starShaderSRGB->link();
        }
#endif
        
        m_starShaderCompiled = true;
    }

    // Update the star vertex buffer (or vertex array memory if vertex buffer objects aren't supported)
    if (!m_vertexBufferCurrent)
    {
        updateVertexBuffer();
    }

    if (m_vertexBuffer.isValid())
    {
        rc.bindVertexBuffer(VertexSpec::PositionColor, m_vertexBuffer.ptr(), VertexSpec::PositionColor.size());
    }
    else if (m_vertexArray)
    {
        rc.bindVertexArray(VertexSpec::PositionColor, m_vertexArray, VertexSpec::PositionColor.size());
    }
    else
    {
        // No valid star data!
        return;
    }

    // Note that vertex buffers are _required_ in order to use the star shader
    // There should be no drivers that support GLSL shaders but not VBs, since
    // the latter is a GL 1.5 feature, while GLSL is GL 2.0.
    bool useStarShader = m_style == GaussianStars &&
                         m_starShader.isValid() &&
                         m_starShaderSRGB.isValid() &&
                         m_vertexBuffer.isValid();
#ifdef VESTA_OGLES2
    bool enableSRGBExt = false;
#else
    bool enableSRGBExt = GLEW_EXT_framebuffer_sRGB == GL_TRUE;
#endif

    Material starMaterial;
    starMaterial.setDiffuse(Spectrum(1.0f, 1.0f, 1.0f));
    starMaterial.setBlendMode(Material::AdditiveBlend);
    rc.bindMaterial(&starMaterial);

    if (useStarShader)
    {
        GLShaderProgram* starShader = NULL;
        if (enableSRGBExt)
        {
            starShader = m_starShader.ptr();
        }
        else
        {
            starShader = m_starShaderSRGB.ptr();
        }

        if (enableSRGBExt)
        {
            rc.enableCustomShader(starShader);
#ifndef VESTA_OGLES2
            glEnable(GL_FRAMEBUFFER_SRGB_EXT);
#endif
        }
        else
        {
            rc.enableCustomShader(starShader);
        }

        starShader->bind();
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        float viewportX = viewport[0];
        float viewportY = viewport[1];
        Vector2f viewportCoord(viewportX, viewportY);
        Vector2f viewportSize(rc.viewportWidth(), rc.viewportHeight());
        starShader->setConstant("viewportSize", viewportSize);
        starShader->setConstant("viewportCoord", viewportCoord);
        starShader->setConstant("sigma2", 0.35f);
        starShader->setConstant("glareFalloff", 1.0f / 15.0f);
        starShader->setConstant("glareBrightness", 0.003f);
        starShader->setConstant("diffSpikeBrightness", m_diffractionSpikeBrightness * 3.0f);

        // Exposure is set such that stars at the limiting magnitude are just
        // visible on screen, i.e. they will be rendered as pixels with
        // value visibilityThreshold when exactly centered. Exposure is calculated
        // so that stars at the saturation magnitude will be rendered as full
        // brightness pixels.
        float visibilityThreshold = 1.0f / 255.0f;
        float logMVisThreshold = log(visibilityThreshold) / log(2.512);
        float saturationMag = m_limitingMagnitude - 4.5f; //+ logMVisThreshold;
        float magScale = (logMVisThreshold) / (saturationMag - m_limitingMagnitude);
        starShader->setConstant("thresholdBrightness", visibilityThreshold);
        starShader->setConstant("exposure", pow(2.512f, magScale * saturationMag));
        starShader->setConstant("magScale", magScale);

#ifdef VESTA_OGLES2
        starShader->setConstant("vesta_ModelViewProjectionMatrix", (rc.projection() * rc.modelview()).matrix());
#endif
        
#ifndef VESTA_OGLES2
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
        if (GLEW_ARB_multisample)
        {
            glDisable(GL_MULTISAMPLE_ARB);
        }
#endif
    }
    else
    {
#ifndef VESTA_OGLES2
        glPointSize(2.0f);
#endif
    }

    rc.drawPrimitives(PrimitiveBatch(PrimitiveBatch::Points, m_starCatalog->size()));

    rc.unbindVertexBuffer();

    if (useStarShader)
    {
        rc.disableCustomShader();
#ifndef VESTA_OGLES2
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
        if (GLEW_ARB_multisample)
        {
            glEnable(GL_MULTISAMPLE_ARB);
        }

        if (enableSRGBExt)
        {
            glDisable(GL_FRAMEBUFFER_SRGB_EXT);
        }
#endif
    }
}


/** Set the style used for star rendering. GaussianStars is more realistic, but
  * is only available on graphics hardware that supports GLSL shaders.
  */
void
StarsLayer::setStyle(StarStyle style)
{
    if (style != m_style)
    {
        m_style = style;
        m_vertexBufferCurrent = false;
    }
}


/** Set the magnitude of the faintest stars visible. Stars at the limiting magnitude will be
  * displayed with the smallest non-zero pixel value (i.e. 1/255 for 8-bit color channels.)
  */
void
StarsLayer::setLimitingMagnitude(float limitingMagnitude)
{
    m_limitingMagnitude = limitingMagnitude;
}


void
StarsLayer::updateVertexBuffer()
{
    bool useStarShader = m_style == GaussianStars && m_starShader.isValid() && m_starShaderSRGB.isValid();

    if (GLVertexBuffer::supported())
    {
        if (useStarShader)
        {
            m_vertexBuffer = CreateStarVertexBuffer(m_starCatalog.ptr());
        }
        else
        {
            m_vertexBuffer = CreateStarVertexBufferFF(m_starCatalog.ptr());
        }
    }
    else
    {
        if (m_vertexArray)
        {
            delete[] m_vertexArray;
            m_vertexArray = NULL;
        }
        m_vertexArray = CreateStarVertexArrayFF(m_starCatalog.ptr());
    }

    m_vertexBufferCurrent = true;
}
