// SpiceRotationModel.cpp
//
// Copyright (C) 2013 Chris Laurel <claurel@gmail.com>
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met: 
//
//    1. Redistributions of source code must retain the above copyright notice, this
//       list of conditions and the following disclaimer. 
//    2. Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimer in the documentation
//       and/or other materials provided with the distribution. 
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "SpiceRotationModel.h"
#include <SpiceUsr.h>

using namespace vesta;
using namespace Eigen;
using namespace std;


SpiceRotationModel::SpiceRotationModel(const char *fromFrame, const char *toFrame) :
    m_fromFrame(fromFrame),
    m_toFrame(toFrame)
{
}


SpiceRotationModel::~SpiceRotationModel()
{
}


Quaterniond
SpiceRotationModel::orientation(double tdbSec) const
{
    double et = tdbSec;
    SpiceDouble transform[3][3];

    pxform_c(m_fromFrame.c_str(), m_toFrame.c_str(), et, transform);
    if (!failed_c())
    {
        Matrix3d R;
        R << transform[0][0], transform[0][1], transform[0][2],
             transform[1][0], transform[1][1], transform[1][2],
             transform[2][0], transform[2][1], transform[2][2];
        return Quaterniond(R);
    }
    else
    {
        char errorMessage[1024];
        getmsg_c("long", sizeof(errorMessage), errorMessage);
        cerr << errorMessage << std::endl;
        reset_c();
        return Quaterniond::Identity();
    }
}


Vector3d
SpiceRotationModel::angularVelocity(double tdbSec) const
{
    double et = tdbSec;
    SpiceDouble transform[6][6];

    sxform_c(m_fromFrame.c_str(), m_toFrame.c_str(), et, transform);
    if (!failed_c())
    {
        // Extract the angular velocity vector from the state transform matrix
        // Rotation matrix is the top left 3x3 matrix
        Matrix3d R;
        R << transform[0][0], transform[0][1], transform[0][2],
             transform[1][0], transform[1][1], transform[1][2],
             transform[2][0], transform[2][1], transform[2][2];

        // W*R is the lower left 3x3 matrix
        Matrix3d WR;
        WR << transform[3][0], transform[3][1], transform[3][2],
              transform[4][0], transform[4][1], transform[4][2],
              transform[5][0], transform[5][1], transform[5][2];

        // Multiply by inverse of R (= transpose, since R is a rotation) to get
        // the skew-symmetric matrix W*
        Matrix3d W = WR * R.transpose();

        return Vector3d(-W(1, 2), W(0, 2), -W(0, 1));
    }
    else
    {
        char errorMessage[1024];
        getmsg_c("long", sizeof(errorMessage), errorMessage);
        cerr << errorMessage << std::endl;
        reset_c();
        return Vector3d::Zero();
    }
}
