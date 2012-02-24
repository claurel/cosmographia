// This file is part of Cosmographia.
//
// Copyright (C) 2012 Chris Laurel <claurel@gmail.com>
//
// Eigen is free software; you can redistribute it and/or
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

#ifndef _FEATURE_LABEL_SET_GEOMETRY_H_
#define _FEATURE_LABEL_SET_GEOMETRY_H_

#include <vesta/Geometry.h>
#include <vesta/TextureFont.h>
#include <vesta/AlignedEllipsoid.h>
#include <Eigen/StdVector>
#include <vector>
#include <string>

/** FeatureLabelSetGeometry is a VESTA geometry subclass used for
  * displaying labels of planetary features.
  */
class FeatureLabelSetGeometry : public vesta::Geometry
{
public:
    FeatureLabelSetGeometry();
    virtual ~FeatureLabelSetGeometry();

    void render(vesta::RenderContext& rc, double clock) const;
    float boundingSphereRadius() const;

    virtual bool isOpaque() const
    {
        return false;
    }

    void addFeature(const std::string& label, const Eigen::Vector3f& position, float radius, const vesta::Spectrum& color);

    vesta::TextureFont* font() const
    {
        return m_font.ptr();
    }

    void setFont(vesta::TextureFont* font)
    {
        m_font = font;
    }

    vesta::AlignedEllipsoid occluder() const
    {
        return m_occludingEllipsoid;
    }

    void setOccluder(vesta::AlignedEllipsoid& e)
    {
        m_occludingEllipsoid = e;
    }

    /** Get the value of the global label opacity. This controls the visibility of
      * all feature label sets.
      */
    static float globalOpacity()
    {
        return ms_globalOpacity;
    }

    /** Set the value of the global label opacity. This controls the visibility of
      * all feature label sets. The default opacity is 1 (fully visible labels)
      *
      * \param opacity a value between 0 and 1, with 0 meaning completely transparent labels and 1 meaning fully visible.
      */
    static void setGlobalOpacity(float opacity)
    {
        ms_globalOpacity = opacity;
    }

private:
    struct Feature
    {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
        std::string label;
        Eigen::Vector3f position;
        float size;
        vesta::Spectrum color;
    };

    std::vector<Feature, Eigen::aligned_allocator<Feature> > m_features;
    float m_maxFeatureDistance;

    vesta::counted_ptr<vesta::TextureFont> m_font;
    vesta::AlignedEllipsoid m_occludingEllipsoid;

    static float ms_globalOpacity;
};

#endif // _FEATURE_LABEL_SET_GEOMETRY_H_
