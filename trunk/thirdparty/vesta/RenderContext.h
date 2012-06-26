/*
 * $Revision: 653 $ $Date: 2012-02-22 16:13:30 -0800 (Wed, 22 Feb 2012) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_RENDER_CONTEXT_H_
#define _VESTA_RENDER_CONTEXT_H_

#include "VertexArray.h"
#include "PrimitiveBatch.h"
#include "Material.h"
#include "ShaderInfo.h"
#include "PlanarProjection.h"
#include "TextureFont.h"
#include "glhelp/GLVertexBuffer.h"
#include <Eigen/Core>
#include <Eigen/Geometry>


namespace vesta
{

class TextureMap;
class ParticleEmitter;
class ParticleBuffer;
class VertexBuffer;
class GLShaderProgram;
class GLFramebuffer;

/** RenderContext provides an interface for state tracking and shader
  * setup. Vesta classes which need to do rendering should use RenderContext
  * methods rather than calling OpenGL directly. The RenderContext abstracts
  * away most of the differences between the legacy fixed function OpenGL
  * pipeline and the new shader-based programmable pipeline.
  */
class RenderContext
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    /** Graphics shader capability levels:
      *   FixedFunction: legacy mode, no shaders
      *   GLSL1: OpenGL Shading Language version 1
      *   GLSL2: OpenGL Shading Language version 2
      */
    enum ShaderCapability
    {
        FixedFunction = 0,
        GLSL1         = 1,
        GLSL2         = 2,
    };

private:
    // Private constructor; RenderContexts should be created via one
    // of the Create() factory methods.
    RenderContext(ShaderCapability capability);
    bool createGLResources();

public:
    ~RenderContext();

    enum RenderPass
    {
        OpaquePass,
        TranslucentPass
    };

    enum LightType
    {
        DirectionalLight = 0,
        PointLight       = 1,
    };

    struct Light
    {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
        Light() :
            type(DirectionalLight),
            position(Eigen::Vector3f::UnitZ()),
            color(0.0f, 0.0f, 0.0f),
            attenuation(1.0f)
        {
        }

        Light(LightType _type, const Eigen::Vector3f& _position, const Spectrum& _color, float _attenuation) :
            type(_type), position(_position), color(_color), attenuation(_attenuation)
        {
        }

        LightType type;
        Eigen::Vector3f position;
        Spectrum color;
        float attenuation;
    };

    struct ScatteringParameters
    {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

        ScatteringParameters()
        {
        }

        ~ScatteringParameters()
        {
        }

        ScatteringParameters(const ScatteringParameters& other) :
            planetRadius(other.planetRadius),
            atmosphereRadius(other.atmosphereRadius),
            rayleighScaleHeight(other.rayleighScaleHeight),
            rayleighCoeff(other.rayleighCoeff),
            mieAsymmetry(other.mieAsymmetry),
            color(other.color),
            transmittanceTexture(other.transmittanceTexture),
            scatterTexture(other.scatterTexture)
        {
        }

        ScatteringParameters operator=(const ScatteringParameters& other)
        {
            planetRadius = other.planetRadius;
            atmosphereRadius = other.atmosphereRadius;
            rayleighScaleHeight = other.rayleighScaleHeight;
            rayleighCoeff = other.rayleighCoeff;
            mieAsymmetry = other.mieAsymmetry;
            color = other.color;
            transmittanceTexture = other.transmittanceTexture;
            scatterTexture = other.scatterTexture;

            return *this;
        }

        float planetRadius;
        float atmosphereRadius;
        float rayleighScaleHeight;
        Eigen::Vector3f rayleighCoeff;
        float mieAsymmetry;
        Spectrum color;
        counted_ptr<TextureMap> transmittanceTexture;
        counted_ptr<TextureMap> scatterTexture;
    };

    enum
    {
        MaxLights = 4,
        MaxEclipseShadows = 7,
        MaxRingShadows = 1,
    };

    enum RendererOutput
    {
        FragmentColor,
        CameraDistance,
    };

    struct Environment
    {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

        Environment();

        Light m_lights[MaxLights];
        unsigned int m_activeLightCount;
        Spectrum m_ambientLight;

        unsigned int m_shadowMapCount;
        Eigen::Matrix4f m_shadowMapMatrices[MaxLights];
        counted_ptr<GLFramebuffer> m_shadowMaps[MaxLights];

        unsigned int m_omniShadowMapCount;
        counted_ptr<TextureMap> m_omniShadowMaps[MaxLights];

        unsigned int m_eclipseShadowCount;
        Eigen::Matrix4f m_eclipseShadowMatrices[MaxEclipseShadows];
        Eigen::Vector2f m_eclipseShadowSlopes[MaxEclipseShadows];

        unsigned int m_ringShadowCount;
        Eigen::Matrix4f m_ringShadowMatrices[MaxRingShadows];
        Eigen::Vector2f m_ringShadowRadii[MaxRingShadows];
        counted_ptr<TextureMap> m_ringShadowTextures[MaxRingShadows];

        bool m_scatteringEnabled;
        ScatteringParameters m_scattering;

        bool m_sphericalGeometry;

        counted_ptr<TextureMap> m_environmentMap;
    };

    static const unsigned int MaxMatrixStackDepth = 16;

    Eigen::Quaternionf cameraOrientation() const
    {
        return m_cameraOrientation;
    }

    void setCameraOrientation(const Eigen::Quaternionf& cameraOrientation)
    {
        m_cameraOrientation = cameraOrientation;
    }

    /** Return the angular size of a the side of a square pixel. This value
      * value may be used when determining the appropriate amount of detail
      * to display for some object.
      */
    float pixelSize() const
    {
        return m_pixelSize;
    }

    /** Set the pixel size.
      * @param pixelSize the angular size of one side of a square pixel.
      */
    void setPixelSize(float pixelSize);

    /** Set the size of the viewport in pixels.
      */
    void setViewportSize(int width, int height)
    {
        m_viewportWidth = width;
        m_viewportHeight = height;
    }

    /** Get the viewport width in pixels.
      */
    int viewportWidth() const
    {
        return m_viewportWidth;
    }
    
    /** Get the viewport height in pixels.
      */
    int viewportHeight() const
    {
        return m_viewportHeight;
    }
    
    /** Return the current render pass. In order to reduce artifacts from rendering
      * overlapping translucent objects, opaque objects are all rendered before any
      * translucent ones.
      */
    RenderPass pass() const
    {
        return m_renderPass;
    }

    /** Set the current render pass.
      */
    void setPass(RenderPass pass)
    {
        m_renderPass = pass;
    }

    /** Get the current modelview transformation */
    const Eigen::Transform3f& modelview() const
    {
        return m_matrixStack[m_modelViewStackDepth];
    }

    /** Get the current projection transformation */
    const Eigen::Transform3f& projection() const
    {
        return m_projectionStack[m_projectionStackDepth];
    }

    /** Get the current view frustum.
      */
    const Frustum& frustum() const
    {
        return m_frustumStack[m_projectionStackDepth];
    }

    Eigen::Vector3d modelTranslation() const;
    void setModelTranslation(const Eigen::Vector3d& t);

    void pushModelView();
    void translateModelView(const Eigen::Vector3f& v);
    void rotateModelView(const Eigen::Quaternionf& q);
    void scaleModelView(const Eigen::Vector3f& v);
    void identityModelView();
    void setModelView(const Eigen::Matrix4f& m);
    void popModelView();

    void pushProjection();
    void popProjection();
    void setProjection(const PlanarProjection& projection);

    void bindVertexArray(const VertexArray* vertexArray);
    void bindVertexArray(const VertexSpec& spec, const void* vertexData, unsigned int stride);
    void unbindVertexArray();
    void bindVertexBuffer(const VertexSpec& spec, const GLVertexBuffer* vertexBuffer, unsigned int stride);
    void bindVertexBuffer(const VertexSpec& spec, const VertexBuffer* vertexBuffer, unsigned int stride);
    void unbindVertexBuffer();
    void setVertexInfo(const VertexSpec& spec);

    void bindMaterial(const Material* material);
    void enableCustomShader(GLShaderProgram* shaderProgram);
    void disableCustomShader();

    void setActiveLightCount(unsigned int lightCount);
    void setLight(unsigned int index, const Light& light);
    void setAmbientLight(const Spectrum& ambient);

    void setShadowMapCount(unsigned int shadowCount);
    void setShadowMapMatrix(unsigned int index, const Eigen::Matrix4f& shadowMatrix);
    void setShadowMap(unsigned int index, GLFramebuffer* shadowMap);
    void setOmniShadowMapCount(unsigned int shadowCount);
    void setOmniShadowMap(unsigned int index, TextureMap* shadowCubeMap);
    void setEclipseShadowCount(unsigned int shadowCount);
    void setEclipseShadowMatrix(unsigned int index, const Eigen::Matrix4f& shadowMatrix, float umbraSlope, float penumbraSlope);
    void setRingShadowCount(unsigned int shadowCount);
    void setRingShadowMatrix(unsigned int index, const Eigen::Matrix4f& shadowMatrix, float innerRadius);
    void setRingShadowTexture(unsigned int index, TextureMap* texture);

    void setScattering(bool enabled);
    void setScatteringParameters(const ScatteringParameters& scatteringParams);

    void setEnvironmentMap(TextureMap* environmentMap);
    void setSphericalGeometryHint(bool enabled);

    void drawPrimitives(const PrimitiveBatch& batch);
    void drawPrimitives(PrimitiveBatch::PrimitiveType type, unsigned int indexCount, PrimitiveBatch::IndexSize indexSize, const char* indexData);

    void drawBillboard(const Eigen::Vector3f& position, float size);
    void drawText(const Eigen::Vector3f& position, const std::string& text, const TextureFont* font, const Spectrum& color, float opacity = 1.0f);
    void drawEncodedText(const Eigen::Vector3f& position,
                         const std::string& text,
                         const TextureFont* font,
                         TextureFont::Encoding encoding,
                         const Spectrum& color,
                         float opacity = 1.0f);
    void drawCone(float apexAngle, const Eigen::Vector3f& axis,
                  const Spectrum& color, float opacity,
                  unsigned int radialSubdivision, unsigned int axialSubdivision);
    void drawBox(const Eigen::Vector3f& sideLengths, const Spectrum& color);
    void drawCircle(float radius,
                    const Eigen::Vector3f& center,
                    const Eigen::Vector3f& planeNormal,
                    const Spectrum& color,
                    float opacity,
                    unsigned int subdivision);
    void drawParticles(ParticleEmitter* emitter, double clock);

    /** Get the vertex stream buffer for the render context. This is useful
      * for drawing dynamic geometry.
      */
    VertexBuffer* vertexStreamBuffer() const
    {
        return m_vertexStreamBuffer.ptr();
    }

    /** Get the default font. This font is used whenever drawText is called with a NULL font.
      */
    TextureFont* defaultFont()
    {
        return m_defaultFont.ptr();
    }

    void setDefaultFont(TextureFont* font);

    void unbindShader();

    RendererOutput rendererOutput() const;
    void setRendererOutput(RendererOutput output);
            
    ShaderCapability shaderCapability() const
    {
        return m_shaderCapability;
    }

public:
    struct VertexInfo
    {
        VertexInfo() :
            hasNormals(false),
            hasTexCoords(false),
            hasTangents(false),
            hasColors(false)
        {}

        bool hasNormals;
        bool hasTexCoords;
        bool hasTangents;
        bool hasColors;
    };

    static RenderContext* Create();
    static RenderContext* Create(ShaderCapability capability);
    static ShaderCapability GetHardwareCapability();

private:
    void setFixedFunctionMaterial(const Material* material);
    void setShaderMaterial(const Material* material);
    void updateShaderState();
    void updateShaderTransformConstants();
    void invalidateShaderState();
    void invalidateModelViewMatrix()
    {
        m_modelViewMatrixCurrent = false;
    }

private:
    Eigen::Quaternionf m_cameraOrientation;
    float m_pixelSize;
    int m_viewportWidth;
    int m_viewportHeight;
    RenderPass m_renderPass;

    unsigned int m_modelViewStackDepth;
    unsigned int m_projectionStackDepth;
    Eigen::Transform3f m_matrixStack[MaxMatrixStackDepth];
    Eigen::Transform3f m_projectionStack[MaxMatrixStackDepth];
    Frustum m_frustumStack[MaxMatrixStackDepth];
    Eigen::Vector3d m_modelTranslation;

    ParticleBuffer* m_particleBuffer;
    float* m_vertexStream;
    unsigned int m_vertexStreamFloats;

    counted_ptr<VertexBuffer> m_vertexStreamBuffer;

    ShaderCapability m_shaderCapability;
    VertexInfo m_vertexInfo;
    ShaderInfo m_currentShaderInfo;
    GLShaderProgram* m_currentShader;
    counted_ptr<GLShaderProgram> m_customShader;
    Material m_currentMaterial;
    Environment m_environment;

    // Special purpose shaders
    counted_ptr<GLShaderProgram> m_cameraDistanceShader;

    bool m_shaderStateCurrent;
    bool m_modelViewMatrixCurrent;
    RendererOutput m_rendererOutput;

    static bool m_glInitialized;

    counted_ptr<vesta::TextureFont> m_defaultFont;
};

}

#endif // _VESTA_RENDER_CONTEXT_H_
