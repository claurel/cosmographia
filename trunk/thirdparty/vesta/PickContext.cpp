/*
 * $Revision: 609 $ $Date: 2011-04-29 12:30:42 -0700 (Fri, 29 Apr 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "PickContext.h"

using namespace vesta;
using namespace Eigen;


PickContext::PickContext() :
    m_pickOrigin(Vector3d::Zero()),
    m_pickDirection(Vector3d::UnitZ()),
    m_projection(PlanarProjection::CreateOrthographic2D(-1.0f, 1.0f, -1.0f, 1.0f)),
    m_cameraOrientation(Quaterniond::Identity()),
    m_pixelAngle(1.0e-3f)
{
}


PickContext::~PickContext()
{
}


