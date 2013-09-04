/*
 * $Revision: 665 $ $Date: 2012-04-20 10:01:29 -0700 (Fri, 20 Apr 2012) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_OGLHEADERS_H_
#define _VESTA_OGLHEADERS_H_

#if defined(VESTA_OGLES2)

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#define VESTA_NO_IMMEDIATE_MODE_3D
#define VESTA_NO_FIXED_FUNCTION_3D

#else

#if !defined(__gl_h_) && !defined(__GL_H__)
#include <GL/glew.h>
#endif

#if 0
#ifdef _WIN32
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif // _WIN32
#endif // 0

#endif // VESTA_OGLES2

// For building with legacy-free OpenGL, the following two macros
// should be defined:
//    VESTA_NO_IMMEDIATE_MODE_3D - prohibit glVertex3f, glNormal3f, etc.
//    VESTA_NO_FIXED_FUNCTION_3D - no deprecated 3D state, including:
//       glShadeModel, glLighting

#endif // _VESTA_OGLHEADERS_H_

