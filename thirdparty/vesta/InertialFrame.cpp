/*
 * $Revision: 490 $ $Date: 2010-09-07 13:34:23 -0700 (Tue, 07 Sep 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "InertialFrame.h"
#include "Units.h"

using namespace vesta;
using namespace Eigen;


// Construct the bias rotation matrix to convert from ICRF to EMEJ2000. The
// two frames differ by less than 1/10 arcsec.
static Matrix3d BiasMatrix()
{
    // Angles from 2003 IERS Conventions, Chapter 5:
    const double xi0  = arcsecToRadians(-0.0166170);
    const double eta0 = arcsecToRadians(-0.0068192);
    const double da0  = arcsecToRadians(-0.01460);

    Matrix3d bias;
    bias << 1.0,  -da0,  xi0,
            da0,   1.0,  eta0,
           -xi0, -eta0,   1.0;

    // Second-order corrections
    bias(0, 0) -= 0.5 * (da0 * da0   + xi0 * xi0);
    bias(1, 1) -= 0.5 * (da0 * da0   + eta0 * eta0);
    bias(2, 2) -= 0.5 * (eta0 * eta0 + xi0 * xi0);

    return bias;
}


// Matrix coefficients from Seidelman, "Explanatory Supplement to the Astronomical Almanac" (1992), p. 312
static double B1950ToJ2000Coeffs[9] =
{
     0.9999256794956877,  0.0111814832391717,   0.0048590037723143,
    -0.0111814832204662,  0.9999374848933135,  -0.0000271702937440,
    -0.0048590038153592, -0.0000271625947142,   0.9999881946023742
};

static Matrix3d B1950ToEMEJ2000Mat(B1950ToJ2000Coeffs);

static Quaterniond ICRFToEMEJ2000(BiasMatrix());
static Quaterniond EMEJ2000ToICRF = ICRFToEMEJ2000.conjugate();
static Quaterniond B1950ToEMEJ2000(B1950ToEMEJ2000Mat);
static Quaterniond B1950ToICRF = EMEJ2000ToICRF * B1950ToEMEJ2000;
static Quaterniond J2000ToB1950 = B1950ToEMEJ2000.conjugate();
static Quaterniond EclipticToEMEJ2000 = Quaterniond(AngleAxisd(toRadians(J2000_OBLIQUITY), Vector3d::UnitX()));

// Fricke offset; this is already included in the B1950 to J2000 matrix. Not needed unless
// we want to create a B1950 frame without the offset. SPICE distinguishes between the B1950
// reference frame and the FK4/B1950 frame. Seidelman (Explanatory Supplement to the Astronomical
// Almanac, chapter 5) uses B1950 to mean the FK4/B1950.0 system.
// static Quaterniond FK4ToB1950 = Quaterniond(AngleAxisd(arcsecToRadians(0.525), Vector3d::UnitZ()));

// Rotations to convert from galactic coordinates to FK4/B1950.0
static Quaterniond GalacticToB1950 = Quaterniond(AngleAxisd(toRadians(282.25), Vector3d::UnitZ())) *
                                     Quaterniond(AngleAxisd(toRadians(62.6),   Vector3d::UnitX())) *
                                     Quaterniond(AngleAxisd(toRadians(327.0),  Vector3d::UnitZ()));

// Standard frames
// The J2000 equator frame is the native frame of VESTA
static InertialFrame* ICRFInstance = new InertialFrame(Quaterniond::Identity());

static InertialFrame* EquatorJ2000Instance = new InertialFrame(EMEJ2000ToICRF);
static InertialFrame* EclipticJ2000Instance = new InertialFrame(EMEJ2000ToICRF * EclipticToEMEJ2000);
static InertialFrame* EquatorB1950Instance = new InertialFrame(B1950ToICRF * EclipticToEMEJ2000);
//static InertialFrame* FK4Instance = new InertialFrame(B1950ToICRF * FK4ToB1950);
static InertialFrame* GalacticInstance = new InertialFrame(B1950ToICRF * GalacticToB1950);

counted_ptr<InertialFrame> InertialFrame::s_ICRF(ICRFInstance);
counted_ptr<InertialFrame> InertialFrame::s_EclipticJ2000(EclipticJ2000Instance);
counted_ptr<InertialFrame> InertialFrame::s_EquatorJ2000(EquatorJ2000Instance);
counted_ptr<InertialFrame> InertialFrame::s_EquatorB1950(EquatorB1950Instance);
counted_ptr<InertialFrame> InertialFrame::s_Galactic(GalacticInstance);

