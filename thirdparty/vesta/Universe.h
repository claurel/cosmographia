/*
 * $Revision: 505 $ $Date: 2010-09-13 16:38:02 -0700 (Mon, 13 Sep 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_UNIVERSE_H_
#define _VESTA_UNIVERSE_H_

#include "Entity.h"
#include "StarCatalog.h"
#include "PickResult.h"
#include <vector>
#include <map>

namespace vesta
{

class SkyLayer;
class PlanarProjection;
class Viewport;
class PickContext;

class Universe : public Object
{
public:
    Universe();
    ~Universe();

    std::vector<Entity*> entities() const;
    void addEntity(Entity* entity);
    void removeEntity(Entity* entity);
    Entity* findFirst(const std::string& name);

    StarCatalog* starCatalog() const;
    void setStarCatalog(StarCatalog* starCatalog);

    bool pickObject(double t,
                    const Eigen::Vector3d& pickOrigin,
                    const Eigen::Vector3d& pickDirection,
                    double pixelAngle,
                    PickResult* result) const;
    bool pickViewportObject(double t,
                            const Eigen::Vector2d& pickPoint,
                            const Eigen::Vector3d& cameraPosition,
                            const Eigen::Quaterniond& cameraOrientation,
                            const PlanarProjection& projection,
                            const Viewport& viewport,
                            PickResult* result) const;
    bool pickObject(PickContext* pc,
                    double t,
                    PickResult* result) const;

    typedef std::map<std::string, counted_ptr<SkyLayer> > SkyLayerTable;
    const SkyLayerTable* layers() const
    {
        return &m_layers;
    }

    void setLayer(const std::string& tag, SkyLayer* layer);
    void removeLayer(const std::string& tag);
    SkyLayer* layer(const std::string& tag) const;
    bool hasLayers() const;
    void clearLayers();

private:
    typedef std::vector<counted_ptr<Entity> > EntityTable;

    EntityTable m_entities;
    counted_ptr<StarCatalog> m_starCatalog;
    SkyLayerTable m_layers;
};

}

#endif // _VESTA_BODY_H_
