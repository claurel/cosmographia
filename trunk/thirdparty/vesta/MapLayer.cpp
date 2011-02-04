/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "MapLayer.h"
#include "Units.h"

using namespace vesta;


MapLayer::MapLayer() :
    m_opacity(1.0f)
{
}


MapLayer::~MapLayer()
{
}


/** Create a new bounds object that covers the entire sphere.
  */
MapLayerBounds::MapLayerBounds() :
    m_west(0.0),
    m_south(toRadians(-90.0)),
    m_east(toRadians(360.0)),
    m_north(toRadians(90.0))
{
}


MapLayerBounds::MapLayerBounds(double west, double south, double east, double north) :
    m_west(west),
    m_south(south),
    m_east(east),
    m_north(north)
{
}
