/*
 * $Revision: 597 $ $Date: 2011-03-31 09:25:53 -0700 (Thu, 31 Mar 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_LABEL_VISUALIZER_H_
#define _VESTA_LABEL_VISUALIZER_H_

#include "Visualizer.h"
#include "LabelGeometry.h"

namespace vesta
{

class LabelVisualizer : public Visualizer
{
public:
    LabelVisualizer(const std::string& text, TextureFont* font, const Spectrum& color = Spectrum(1.0f, 1.0f, 1.0f), float iconSize = 20.0f);
    ~LabelVisualizer();

    LabelGeometry* label() const
    {
        return m_label.ptr();
    }

protected:
    virtual bool handleRayPick(const PickContext* pc,
                               const Eigen::Vector3d& pickOrigin,
                               double t) const;

private:
    counted_ptr<LabelGeometry> m_label;
};

}

#endif // _VESTA_LABEL_VISUALIZER_H_
