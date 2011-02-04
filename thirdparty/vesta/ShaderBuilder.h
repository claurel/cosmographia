/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_SHADER_BUILDER_H_
#define _VESTA_SHADER_BUILDER_H_

#include "ShaderInfo.h"
#include "glhelp/GLShaderProgram.h"
#include <map>


namespace vesta
{

/** ShaderBuilder is used internally in VESTA to construct GLSL
  * shaders for various combinations of surface properties and
  * lighting state.
  */
class ShaderBuilder
{
private:
    ShaderBuilder();
    ~ShaderBuilder();

    /** Get the ShaderBuilder instance that creates GLSL version 1 shaders.
      */
public:
    static ShaderBuilder* GLSL()
    {
        return &s_GLSLBuilder;
    }

    GLShaderProgram* getShader(const ShaderInfo& shaderInfo);

    static const int TangentAttributeLocation = 7;

private:
    GLShaderProgram* generateShader(const ShaderInfo& shaderInfo) const;

private:
    typedef std::map<ShaderInfo, GLShaderProgram*> ShaderCache;
    ShaderCache m_shaderCache;
    static ShaderBuilder s_GLSLBuilder;
};

}

#endif // _VESTA_SHADER_BUILDER_H_
