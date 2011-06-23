// This file is part of Cosmographia.
//
// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
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

#include "KeplerianSwarm.h"
#include <vesta/RenderContext.h>
#include <vesta/Material.h>
#include <vesta/Units.h>
#include <vesta/VertexBuffer.h>
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

static const char* SwarmVertexShaderSource =
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
"varying vec4 pointColor;                        \n"
"void main()                                     \n"
"{                                               \n"
"    gl_FragColor = pointColor;                  \n"
"}                                               \n"
;


KeplerianSwarm::KeplerianSwarm() :
    m_vertexSpec(NULL),
    m_epoch(vesta::J2000),
    m_boundingRadius(0.0f),
    m_color(Spectrum(1.0f, 1.0f, 1.0f)),
    m_opacity(1.0f),
    m_pointSize(1.0f),
    m_shaderCompiled(false)
{
    setClippingPolicy(PreventClipping);

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
        }

        if (m_swarmShader.isValid())
        {
            //rc.bindVertexBuffer(VertexSpec::PositionNormalTex, m_vertexBuffer.ptr(), sizeof(KeplerianObject));
            rc.bindVertexBuffer(*m_vertexSpec, m_vertexBuffer.ptr(), sizeof(KeplerianObject));

            Material material;
            material.setOpacity(m_opacity);
            rc.bindMaterial(&material);

            rc.enableCustomShader(m_swarmShader.ptr());
            m_swarmShader->bind();
            m_swarmShader->setConstant("time", float(clock - m_epoch));
            m_swarmShader->setConstant("pointSize", m_pointSize);
            m_swarmShader->setConstant("color", Vector4f(m_color.red(), m_color.green(), m_color.blue(), m_opacity));

            rc.drawPrimitives(PrimitiveBatch(PrimitiveBatch::Points, m_objects.size()));
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
