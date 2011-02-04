/*
 * $Revision: 505 $ $Date: 2010-09-13 16:38:02 -0700 (Mon, 13 Sep 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "Universe.h"
#include "Geometry.h"
#include "Intersect.h"
#include "Visualizer.h"
#include "SkyLayer.h"
#include <algorithm>
#include <limits>

using namespace vesta;
using namespace Eigen;
using namespace std;


Universe::Universe()
{
}


Universe::~Universe()
{
}


/** Return an array of all entities in the universe.
  */
vector<Entity*>
Universe::entities() const
{
    vector<Entity*> entities;
    for (EntityTable::const_iterator iter = m_entities.begin(); iter != m_entities.end(); ++iter)
    {
        entities.push_back(iter->ptr());
    }
    return entities;
}


/** Add a new entity to the universe. Null entities are ignored.
  * @param entity pointer to an entity.
  */
void
Universe::addEntity(Entity* entity)
{
    m_entities.push_back(counted_ptr<Entity>(entity));
}


/** Remove an entity from the universe.
  */
void
Universe::removeEntity(Entity* entity)
{
    EntityTable::iterator iter = find(m_entities.begin(), m_entities.end(), counted_ptr<Entity>(entity));
    if (iter != m_entities.end())
    {
        m_entities.erase(iter);
    }
}


/** Find the first entity with the specified name.
  *
  * @return a pointer to the an entity with a matching name, or null if
  *    no matching entity is found.
  */
Entity*
Universe::findFirst(const std::string& name)
{
    if (name.empty())
    {
        return NULL;
    }

    for (EntityTable::iterator iter = m_entities.begin(); iter != m_entities.end(); ++iter)
    {
        if (name == (*iter)->name())
        {
            return iter->ptr();
        }
    }

    return NULL;
}


/** Determine the closest object intersected by a ray given by the pick origin and direction.
  * This method returns true if any object was intersected. If the value of result is not
  * null, it will be updated with information about which object was hit by the pick ray.
  *
  * @param t The time given as the number of seconds since 1 Jan 2000 12:00:00 UTC.
  * @param pickOrigin origin of the pick ray
  * @param pickDirection direction of the pick ray (does not need to be normalized)
  * @param pixelAngle angle in radians subtended by a pixel
  * @param result pointer to a PickResult object that will be filled in if the pick ray
  *    hits something.
  */
bool
Universe::pickObject(double t,
                     const Vector3d& pickOrigin,
                     const Vector3d& pickDirection,
                     double pixelAngle,
                     PickResult* result) const
{
    double closest = numeric_limits<double>::infinity();
    PickResult closestResult;

    for (EntityTable::const_iterator iter = m_entities.begin(); iter != m_entities.end(); ++iter)
    {
        Entity* entity = iter->ptr();

        if (entity->geometry() || entity->hasVisualizers())
        {
            if (entity->isVisible() && entity->chronology()->includesTime(t))
            {
                Vector3d position = entity->position(t);

                if (entity->geometry())
                {
                    Geometry* geometry = entity->geometry();
                    double intersectionDistance;
                    if (TestRaySphereIntersection(pickOrigin,
                                                  pickDirection,
                                                  position,
                                                  geometry->boundingSphereRadius(),
                                                  &intersectionDistance))
                    {
                        if (intersectionDistance < closest)
                        {
                            // Transform the pick ray into the local coordinate system of body
                            Matrix3d invRotation = entity->orientation(t).conjugate().toRotationMatrix();
                            Vector3d relativePickOrigin = invRotation * (pickOrigin - position);
                            Vector3d relativePickDirection = invRotation * pickDirection;

                            double distance = intersectionDistance;
                            if (geometry->rayPick(relativePickOrigin, relativePickDirection, t, &distance))
                            {
                                if (distance < closest)
                                {
                                    closest = distance;
                                    closestResult.setHit(entity, distance, pickOrigin + pickDirection * distance);
                                }
                            }
                        }
                    }
                }

                // Visualizers may act as 'pick proxies'
                if (entity->hasVisualizers())
                {
                    Vector3d relativePickOrigin = pickOrigin - position;

                    // Calculate the distance to the plane containing the center of the visualizer
                    // and perpendicular to the pick direction.
                    double distanceToPlane = -pickDirection.dot(relativePickOrigin);

                    if (distanceToPlane > 0.0 && distanceToPlane < closest)
                    {
                        for (Entity::VisualizerTable::const_iterator iter = entity->visualizers()->begin();
                             iter != entity->visualizers()->end(); ++iter)
                        {
                            const Visualizer* visualizer = iter->second.ptr();
                            if (visualizer->isVisible() &&
                                visualizer->rayPick(relativePickOrigin, pickDirection, pixelAngle))
                            {
                                closest = distanceToPlane;
                                closestResult.setHit(entity, distanceToPlane, pickOrigin + pickDirection * distanceToPlane);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    if (closest < numeric_limits<double>::infinity())
    {
        if (result)
        {
            *result = closestResult;
        }
        return true;
    }
    else
    {
        return false;
    }
}


StarCatalog*
Universe::starCatalog() const
{
    return m_starCatalog.ptr();
}


void
Universe::setStarCatalog(StarCatalog* starCatalog)
{
    m_starCatalog = starCatalog;
}


/** Add a new sky layer with a specified tag. If a layer with the
  * same tag already exists, it will be replaced.
  */
void
Universe::setLayer(const std::string& tag, SkyLayer* layer)
{
    m_layers[tag] = counted_ptr<SkyLayer>(layer);
}


/** Remove the sky layer with the specified tag. The method has no
  * effect if the tag is not found.
  */
void
Universe::removeLayer(const std::string& tag)
{
    m_layers.erase(tag);
}


/** Get the sky layer with the specified tag. If no layer with
  * the tag exists, the method returns null.
  */
SkyLayer*
Universe::layer(const std::string& tag) const
{
    SkyLayerTable::const_iterator iter = m_layers.find(tag);
    if (iter == m_layers.end())
    {
        return NULL;
    }
    else
    {
        return iter->second.ptr();
    }
}


/** Return true if there are any sky layers.
  */
bool
Universe::hasLayers() const
{
    return !m_layers.empty();
}


/** Remove all sky layers.
  */
void
Universe::clearLayers()
{
    m_layers.clear();
}
