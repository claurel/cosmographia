/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "LocalVisualizer.h"
#include "RenderContext.h"
#include "WorldGeometry.h"
#include "Units.h"
#include "Intersect.h"
#include "Debug.h"
#include "GL/glew.h"
#include <vector>
#include <algorithm>

using namespace vesta;
using namespace Eigen;
using namespace std;



/** Create a new LocalVisualizer.
  */
LocalVisualizer::LocalVisualizer(Geometry* geometry) :
    Visualizer(geometry)
{
}


LocalVisualizer::~LocalVisualizer()
{
}


/** \reimp
  */
Eigen::Quaterniond
LocalVisualizer::orientation(const Entity* parent, double t) const
{
    return parent->orientation(t);
}
