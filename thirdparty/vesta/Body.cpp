/*
 * $Revision: 530 $ $Date: 2010-10-12 11:26:43 -0700 (Tue, 12 Oct 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "Body.h"

using namespace vesta;
using namespace Eigen;


Body::Body()
{
}


Body::~Body()
{
}


/** Set the geometry for this body. The default geometry
  * is null, indicating that the object has no visibile representation
  * (apart from visualizers.)
  */
void
Body::setGeometry(Geometry* geometry)
{
    m_geometry = geometry;
}
