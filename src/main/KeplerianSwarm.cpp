// This file is part of Cosmographia.
//
// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
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

#include "KeplerianSwarm.h"
#include <vesta/RenderContext.h>
#include <vesta/Material.h>
#include <vesta/Units.h>
#include <vesta/VertexBuffer.h>
#include <vesta/ShaderBuilder.h>
#include <vesta/glhelp/GLShaderProgram.h>
#include <vesta/Debug.h>
#include <Eigen/Geometry>
#include <algorithm>

using namespace vesta;
using namespace Eigen;
using namespace std;


// Keplerian swarm shader GLSL source
//
// The vertex layout uses the the standard attributes but assignes them
// very unconventional meanings. VESTA should enventually be enhanced with
// support for custom vertex attributes.
//
// position.x : sma
// position.y : eccentricity
// position.z : mean anomaly at epoch
// normal.x   : mean motion
// normal.y   : qw
// normal.z   : qx
// texcoord.s : qy
// texcoord.t : qz

//
// Where (qw, qx, qy, qz) is a quaternion giving the orientation of the orbital
// plane.

#ifdef VESTA_OGLES2

static const char* SwarmVertexShaderSource =
"attribute vec3 vesta_Position;   \n"
"attribute vec3 vesta_Normal;     \n"
"attribute vec3 vesta_TexCoord0;  \n"
"uniform mat4 vesta_ModelViewProjectionMatrix;\n"
"uniform float time;              \n"
"uniform float pointSize;         \n"
"uniform vec4 color;              \n"
"varying lowp vec4 pointColor;    \n"
"\n"
"void main()                      \n"
"{                                \n"
"    float sma = vesta_Position.x;                                                 \n"
"    float ecc = vesta_Position.y;                                                 \n"
"    float M0  = vesta_Position.z;                                                 \n"
"    float nu  = vesta_Normal.x;                                                 \n"
"    vec4 q = vec4(vesta_Normal.z, vesta_TexCoord0.x, vesta_TexCoord0.y, vesta_Normal.y);\n"
"\n"
"    float M = M0 + time * nu;                                                \n"
"    float E = M;                                                             \n"
"    for (int i = 0; i < 4; i += 1)                                           \n"
"        E = M + ecc * sin(E);                                                \n"
"    vec3 position = vec3(sma * (cos(E) - ecc), sma * (sin(E) * sqrt(1.0 - ecc * ecc)), 0.0);\n"
"\n"
"    // Rotate by quaternion q                                                \n"
"    vec3 a = cross(q.xyz, position) + q.w * position;                        \n"
"    position = cross(a, -q.xyz) + dot(q.xyz, position) * q.xyz + q.w * a;    \n"
"\n"
"    float t = time - vesta_TexCoord0.z;                                      \n"
"    if (t < 0.0)                                                             \n"
"        pointColor = vec4(0.0, 0.0, 0.0, 0.0);                               \n"
"    else                                                                     \n"
"        pointColor = mix(vec4(1.0, 1.0, 1.0, 1.0), color, min(t / (86400.0 * 50.0), 1.0));  \n"
"    gl_PointSize = pointSize;\n"
"    gl_Position = vesta_ModelViewProjectionMatrix * vec4(position, 1.0);        \n"
"}                                                                            \n"
;

static const char* SwarmFragmentShaderSource =
"varying lowp vec4 pointColor;                   \n"
"void main()                                     \n"
"{                                               \n"
"    mediump vec2 v = gl_PointCoord - vec2(0.5, 0.5); \n"
"    mediump float opacity = 1.0 - dot(v, v) * 4.0; \n"
"    gl_FragColor = vec4(pointColor.rgb, opacity * pointColor.a);\n"
"}                                               \n"
;

#else

static const char* SwarmVertexShaderSource =
"#version 120                                    \n"
"uniform float time;              \n"
"uniform float pointSize;         \n"
"uniform vec4 color;              \n"
"varying vec4 pointColor;         \n"
"\n"
"void main()                      \n"
"{                                \n"
"    float sma = gl_Vertex.x;                                                 \n"
"    float ecc = gl_Vertex.y;                                                 \n"
"    float M0  = gl_Vertex.z;                                                 \n"
"    float nu  = gl_Normal.x;                                                 \n"
"    vec4 q = vec4(gl_Normal.z, gl_MultiTexCoord0.x, gl_MultiTexCoord0.y, gl_Normal.y);\n"
"\n"
"    float M = M0 + time * nu;                                                \n"
"    float E = M;                                                             \n"
"    for (int i = 0; i < 4; i += 1)                                           \n"
"        E = M + ecc * sin(E);                                                \n"
"    vec3 position = vec3(sma * (cos(E) - ecc), sma * (sin(E) * sqrt(1.0 - ecc * ecc)), 0.0);\n"
"\n"
"    // Rotate by quaternion q                                                \n"
"    vec3 a = cross(q.xyz, position) + q.w * position;                        \n"
"    position = cross(a, -q.xyz) + dot(q.xyz, position) * q.xyz + q.w * a;    \n"
"\n"
"    float t = time - gl_MultiTexCoord0.z;                                    \n"
"    if (t < 0.0)                                                             \n"
"        pointColor = vec4(0.0, 0.0, 0.0, 0.0);                               \n"
"    else                                                                     \n"
"        pointColor = mix(vec4(1.0, 1.0, 1.0, 1.0), color, min(t / (86400.0 * 50.0), 1.0));  \n"
"    gl_PointSize = pointSize;\n"
"    gl_Position = gl_ModelViewProjectionMatrix * vec4(position, 1.0);        \n"
"}                                                                            \n"
;

static const char* SwarmFragmentShaderSource =
"#version 120                                    \n"
"varying vec4 pointColor;                        \n"
"void main()                                     \n"
"{                                               \n"
"    vec2 v = gl_PointCoord - vec2(0.5, 0.5); \n"
"    float opacity = 1.0 - dot(v, v) * 4.0; \n"
"    gl_FragColor = vec4(pointColor.rgb, opacity * pointColor.a);\n"
"}                                               \n"
;

#endif


KeplerianSwarm::KeplerianSwarm() :
    m_vertexSpec(NULL),
    m_epoch(vesta::J2000),
    m_boundingRadius(0.0f),
    m_color(Spectrum(1.0f, 1.0f, 1.0f)),
    m_opacity(1.0f),
    m_pointSize(1.0f),
    m_fadeSize(250.0f),
    m_shaderCompiled(false)
{
#ifndef VESTA_OGLES2
    setClippingPolicy(PreventClipping);
#endif

    VertexAttribute posNormTexAttributes[] = {
        VertexAttribute(VertexAttribute::Position,     VertexAttribute::Float3),
        VertexAttribute(VertexAttribute::Normal,       VertexAttribute::Float3),
        VertexAttribute(VertexAttribute::TextureCoord, VertexAttribute::Float3),
    };

    m_vertexSpec = new VertexSpec(3, posNormTexAttributes);
}


KeplerianSwarm::~KeplerianSwarm()
{
}


void
KeplerianSwarm::render(RenderContext& rc, double clock) const
{
    if (m_objects.empty())
    {
        return;
    }

    // Contents are never treated as opaque; always draw during the translucent pass
    if (rc.pass() != RenderContext::TranslucentPass)
    {
        return;
    }

    float fadeFactor = 1.0f;
    if (m_fadeSize > 0.0f)
    {
        const float sizeFadeStart = m_fadeSize * 4;
        const float sizeFadeEnd = m_fadeSize;
        float pixelSize = boundingSphereRadius() / (rc.modelview().translation().norm() * rc.pixelSize());
        if (pixelSize < sizeFadeStart)
        {
            fadeFactor = std::max(0.0f, (pixelSize - sizeFadeEnd) / (sizeFadeStart - sizeFadeEnd));
        }
    }
    
    if (fadeFactor < 0.001f)
    {
        // Total fade out
        return;
    }
    
    if (m_vertexBuffer.isNull())
    {
        m_vertexBuffer = VertexBuffer::Create(m_objects.size() * sizeof(KeplerianObject), VertexBuffer::StaticDraw, &m_objects[0]);
    }

    if (rc.shaderCapability() != RenderContext::FixedFunction && m_vertexBuffer.isValid())
    {
        // Create the star shaders if they haven't already been compiled
        if (!m_shaderCompiled)
        {
            m_swarmShader = GLShaderProgram::CreateShaderProgram(SwarmVertexShaderSource, SwarmFragmentShaderSource);
            m_shaderCompiled = true;
#ifdef VESTA_OGLES2
            // TODO: Need to make VESTA bind these automatically
            if (m_swarmShader.isValid())
            {
                m_swarmShader->bindAttribute(ShaderBuilder::PositionAttribute, ShaderBuilder::PositionAttributeLocation);
                m_swarmShader->bindAttribute(ShaderBuilder::NormalAttribute, ShaderBuilder::NormalAttributeLocation);
                m_swarmShader->bindAttribute(ShaderBuilder::TexCoordAttribute, ShaderBuilder::TexCoordAttributeLocation);
                m_swarmShader->link();
            }
#endif
        }

        if (m_swarmShader.isValid())
        {
            float effectiveOpacity = fadeFactor * m_opacity;
            
            rc.bindVertexBuffer(*m_vertexSpec, m_vertexBuffer.ptr(), sizeof(KeplerianObject));

            Material material;
            material.setOpacity(std::min(0.99f, effectiveOpacity));
            rc.bindMaterial(&material);

            rc.enableCustomShader(m_swarmShader.ptr());
            m_swarmShader->bind();
            m_swarmShader->setConstant("time", float(clock - m_epoch));
            m_swarmShader->setConstant("pointSize", m_pointSize);
            m_swarmShader->setConstant("color", Vector4f(m_color.red(), m_color.green(), m_color.blue(), effectiveOpacity));
#ifdef VESTA_OGLES2
            m_swarmShader->setConstant("vesta_ModelViewProjectionMatrix", (rc.projection() * rc.modelview()).matrix());                                                             
            rc.drawPrimitives(PrimitiveBatch(PrimitiveBatch::Points, m_objects.size()));
#else
            glEnable(GL_POINT_SPRITE);
            rc.drawPrimitives(PrimitiveBatch(PrimitiveBatch::Points, m_objects.size()));
            glDisable(GL_POINT_SPRITE);
#endif
            rc.unbindVertexBuffer();

            rc.disableCustomShader();
        }
    }
    else
    {
        // TODO: implement non-GPU fallback path. This will be much slower, as the GPU version benefits
        // greatly from fast transcendental functions.
    }
}


float
KeplerianSwarm::boundingSphereRadius() const
{
    return m_boundingRadius;
}


bool
KeplerianSwarm::isOpaque() const
{
    return false;
}


float
KeplerianSwarm::nearPlaneDistance(const Eigen::Vector3f& cameraPosition) const
{
    return max(10000.0f, cameraPosition.norm() - boundingSphereRadius());
}


void
KeplerianSwarm::addObject(const OrbitalElements& elements, double discoveryTime)
{
    Quaterniond orbitOrientation = OrbitalElements::orbitOrientation(elements.inclination,
                                                                     elements.longitudeOfAscendingNode,
                                                                     elements.argumentOfPeriapsis);
    double semiMajorAxis = elements.periapsisDistance / (1.0 - elements.eccentricity);

    // Change of epoch when computing mean anomaly
    double meanAnomaly = elements.meanAnomalyAtEpoch + (m_epoch - elements.epoch) * elements.meanMotion;

    KeplerianObject k;
    k.sma = float(semiMajorAxis);
    k.ecc = float(elements.eccentricity);
    k.meanAnomaly = float(meanAnomaly);
    k.meanMotion = float(elements.meanMotion);
    k.qw = float(orbitOrientation.w());
    k.qx = float(orbitOrientation.x());
    k.qy = float(orbitOrientation.y());
    k.qz = float(orbitOrientation.z());
    k.discoveryDate = discoveryTime - m_epoch;

    m_objects.push_back(k);

    m_boundingRadius = max(m_boundingRadius, float(k.sma * (1.0 + elements.eccentricity)));
}


/** Remove all objects.
  */
void
KeplerianSwarm::clear()
{
    m_boundingRadius = 0.0;
    m_objects.clear();
}
