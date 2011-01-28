#include <vesta/Units.h>
#include <cmath>

using namespace vesta;
using namespace std;

static const double T0 = J2000;
static const double ASEC2RAD = arcsecToRadians(1.0);
static const double TWOPI = 2.0 * PI;
static const double ASEC360 = 1296000.0;

/******** Code from USNO's NOVAS package. ******/

/********iau2000b */

void iau2000b (double jd_high, double jd_low,

               double *dpsi, double *deps)
/*
------------------------------------------------------------------------

   PURPOSE:
      To compute the forced nutation of the non-rigid Earth based on
      the IAU 2000B precession/nutation model.

   REFERENCES:
      McCarthy, D. and Luzum, B. (2003). "An Abridged Model of the
         Precession & Nutation of the Celestial Pole," Celestial
         Mechanics and Dynamical Astronomy, Volume 85, Issue 1,
         Jan. 2003, p. 37. (IAU 2000B)
      IERS Conventions (2003), Chapter 5.

   INPUT
   ARGUMENTS:
      jd_high (double)
         High-order part of TT Julian date.
      jd_low (double)
         Low-order part of TT Julian date.

   OUTPUT
   ARGUMENTS:
      *dpsi (double)
         Nutation (luni-solar + planetary) in longitude, in radians.
      *deps (double)
         Nutation (luni-solar + planetary) in obliquity, in radians.

   RETURNED
   VALUE:
      None.

   GLOBALS
   USED:
      T0, ASEC2RAD, TWOPI

   FUNCTIONS
   CALLED:
      fmod      math.h
      sin       math.h
      cos       math.h

   VER./DATE/
   PROGRAMMER:
      V1.0/09-03/JAB (USNO/AA)

   NOTES:
      1. IAU 2000B reproduces the IAU 2000A model to a precision of
      1 milliarcsecond in the interval 1995-2020.

------------------------------------------------------------------------
*/
{
   short int i;

/*
   Planetary nutation (arcsec).  These fixed terms account for the
   omission of the long-period planetary terms in the truncated model.
*/

   double dpplan = -0.000135;
   double deplan =  0.000388;

   double t, el, elp, f, d, om, arg, dp, de, sarg, carg, factor, dpsils,
      depsls, dpsipl, depspl;

/*
   Luni-Solar argument multipliers:
       L     L'    F     D     Om
*/

   const short int nals_t[77][5] =
   {
       {  0,    0,    0,    0,    1, },
       {  0,    0,    2,   -2,    2, },
       {  0,    0,    2,    0,    2, },
       {  0,    0,    0,    0,    2, },
       {  0,    1,    0,    0,    0, },
       {  0,    1,    2,   -2,    2, },
       {  1,    0,    0,    0,    0, },
       {  0,    0,    2,    0,    1, },
       {  1,    0,    2,    0,    2, },
       {  0,   -1,    2,   -2,    2, },
       {  0,    0,    2,   -2,    1, },
       { -1,    0,    2,    0,    2, },
       { -1,    0,    0,    2,    0, },
       {  1,    0,    0,    0,    1, },
       { -1,    0,    0,    0,    1, },
       {  1,    0,    2,    2,    2, },
       {  1,    0,    2,    0,    1, },
       { -2,    0,    2,    0,    1, },
       {  0,    0,    0,    2,    0, },
       {  0,    0,    2,    2,    2, },
       {  0,   -2,    2,   -2,    2, },
       { -2,    0,    0,    2,    0, },
       {  2,    0,    2,    0,    2, },
       {  1,    0,    2,   -2,    2, },
       { -1,    0,    2,    0,    1, },
       {  2,    0,    0,    0,    0, },
       {  0,    0,    2,    0,    0, },
       {  0,    1,    0,    0,    1, },
       { -1,    0,    0,    2,    1, },
       {  0,    2,    2,   -2,    2, },
       {  0,    0,   -2,    2,    0, },
       {  1,    0,    0,   -2,    1, },
       {  0,   -1,    0,    0,    1, },
       { -1,    0,    2,    2,    1, },
       {  0,    2,    0,    0,    0, },
       {  1,    0,    2,    2,    2, },
       { -2,    0,    2,    0,    0, },
       {  0,    1,    2,    0,    2, },
       {  0,    0,    2,    2,    1, },
       {  0,   -1,    2,    0,    2, },
       {  0,    0,    0,    2,    1, },
       {  1,    0,    2,   -2,    1, },
       {  2,    0,    2,   -2,    2, },
       { -2,    0,    0,    2,    1, },
       {  2,    0,    2,    0,    1, },
       {  0,   -1,    2,   -2,    1, },
       {  0,    0,    0,   -2,    1, },
       { -1,   -1,    0,    2,    0, },
       {  2,    0,    0,   -2,    1, },
       {  1,    0,    0,    2,    0, },
       {  0,    1,    2,   -2,    1, },
       {  1,   -1,    0,    0,    0, },
       { -2,    0,    2,    0,    2, },
       {  3,    0,    2,    0,    2, },
       {  0,   -1,    0,    2,    0, },
       {  1,   -1,    2,    0,    2, },
       {  0,    0,    0,    1,    0, },
       { -1,   -1,    2,    2,    2, },
       { -1,    0,    2,    0,    0, },
       {  0,   -1,    2,    2,    2, },
       { -2,    0,    0,    0,    1, },
       {  1,    1,    2,    0,    2, },
       {  2,    0,    0,    0,    1, },
       { -1,    1,    0,    1,    0, },
       {  1,    1,    0,    0,    0, },
       {  1,    0,    2,    0,    0, },
       { -1,    0,    2,   -2,    1, },
       {  1,    0,    0,    0,    2, },
       { -1,    0,    0,    1,    0, },
       {  0,    0,    2,    1,    2, },
       { -1,    0,    2,    4,    2, },
       {  1,    1,    0,    1,    1, },
       {  0,   -2,    2,   -2,    1, },
       {  1,    0,    2,    2,    1, },
       { -2,    0,    2,    2,    2, },
       { -1,    0,    0,    0,    2, },
       {  1,    1,    2,   -2,    2 }
   };

/*
   Luni-Solar nutation coefficients, unit 1e-7 arcsec:
   longitude (sin, t*sin, cos), obliquity (cos, t*cos, sin)

   Each row of coefficients in 'cls_t' belongs with the corresponding
   row of fundamental-argument multipliers in 'nals_t'.
*/

   const double cls_t[77][6] =
      {
          { -172064161.0, -174666.0,  33386.0, 92052331.0,  9086.0, 15377.0 },
          { -13170906.0,   -1675.0, -13696.0,  5730336.0, -3015.0, -4587.0 },
          {  -2276413.0,    -234.0,   2796.0,   978459.0,  -485.0,  1374.0 },
          {   2074554.0,     207.0,   -698.0,  -897492.0,   470.0,  -291.0 },
          {   1475877.0,   -3633.0,  11817.0,    73871.0,  -184.0, -1924.0 },
          {   -516821.0,    1226.0,   -524.0,   224386.0,  -677.0,  -174.0 },
          {    711159.0,      73.0,   -872.0,    -6750.0,     0.0,   358.0 },
          {   -387298.0,    -367.0,    380.0,   200728.0,    18.0,   318.0 },
          {   -301461.0,     -36.0,    816.0,   129025.0,   -63.0,   367.0 },
          {    215829.0,    -494.0,    111.0,   -95929.0,   299.0,   132.0 },
          {    128227.0,     137.0,    181.0,   -68982.0,    -9.0,    39.0 },
          {    123457.0,      11.0,     19.0,   -53311.0,    32.0,    -4.0 },
          {    156994.0,      10.0,   -168.0,    -1235.0,     0.0,    82.0 },
          {     63110.0,      63.0,     27.0,   -33228.0,     0.0,    -9.0 },
          {    -57976.0,     -63.0,   -189.0,    31429.0,     0.0,   -75.0 },
          {    -59641.0,     -11.0,    149.0,    25543.0,   -11.0,    66.0 },
          {    -51613.0,     -42.0,    129.0,    26366.0,     0.0,    78.0 },
          {     45893.0,      50.0,     31.0,   -24236.0,   -10.0,    20.0 },
          {     63384.0,      11.0,   -150.0,    -1220.0,     0.0,    29.0 },
          {    -38571.0,      -1.0,    158.0,    16452.0,   -11.0,    68.0 },
          {     32481.0,       0.0,      0.0,   -13870.0,     0.0,     0.0 },
          {    -47722.0,       0.0,    -18.0,      477.0,     0.0,   -25.0 },
          {    -31046.0,      -1.0,    131.0,    13238.0,   -11.0,    59.0 },
          {     28593.0,       0.0,     -1.0,   -12338.0,    10.0,    -3.0 },
          {     20441.0,      21.0,     10.0,   -10758.0,     0.0,    -3.0 },
          {     29243.0,       0.0,    -74.0,     -609.0,     0.0,    13.0 },
          {     25887.0,       0.0,    -66.0,     -550.0,     0.0,    11.0 },
          {    -14053.0,     -25.0,     79.0,     8551.0,    -2.0,   -45.0 },
          {     15164.0,      10.0,     11.0,    -8001.0,     0.0,    -1.0 },
          {    -15794.0,      72.0,    -16.0,     6850.0,   -42.0,    -5.0 },
          {     21783.0,       0.0,     13.0,     -167.0,     0.0,    13.0 },
          {    -12873.0,     -10.0,    -37.0,     6953.0,     0.0,   -14.0 },
          {    -12654.0,      11.0,     63.0,     6415.0,     0.0,    26.0 },
          {    -10204.0,       0.0,     25.0,     5222.0,     0.0,    15.0 },
          {     16707.0,     -85.0,    -10.0,      168.0,    -1.0,    10.0 },
          {     -7691.0,       0.0,     44.0,     3268.0,     0.0,    19.0 },
          {    -11024.0,       0.0,    -14.0,      104.0,     0.0,     2.0 },
          {      7566.0,     -21.0,    -11.0,    -3250.0,     0.0,    -5.0 },
          {     -6637.0,     -11.0,     25.0,     3353.0,     0.0,    14.0 },
          {     -7141.0,      21.0,      8.0,     3070.0,     0.0,     4.0 },
          {     -6302.0,     -11.0,      2.0,     3272.0,     0.0,     4.0 },
          {      5800.0,      10.0,      2.0,    -3045.0,     0.0,    -1.0 },
          {      6443.0,       0.0,     -7.0,    -2768.0,     0.0,    -4.0 },
          {     -5774.0,     -11.0,    -15.0,     3041.0,     0.0,    -5.0 },
          {     -5350.0,       0.0,     21.0,     2695.0,     0.0,    12.0 },
          {     -4752.0,     -11.0,     -3.0,     2719.0,     0.0,    -3.0 },
          {     -4940.0,     -11.0,    -21.0,     2720.0,     0.0,    -9.0 },
          {      7350.0,       0.0,     -8.0,      -51.0,     0.0,     4.0 },
          {      4065.0,       0.0,      6.0,    -2206.0,     0.0,     1.0 },
          {      6579.0,       0.0,    -24.0,     -199.0,     0.0,     2.0 },
          {      3579.0,       0.0,      5.0,    -1900.0,     0.0,     1.0 },
          {      4725.0,       0.0,     -6.0,      -41.0,     0.0,     3.0 },
          {     -3075.0,       0.0,     -2.0,     1313.0,     0.0,    -1.0 },
          {     -2904.0,       0.0,     15.0,     1233.0,     0.0,     7.0 },
          {      4348.0,       0.0,    -10.0,      -81.0,     0.0,     2.0 },
          {     -2878.0,       0.0,      8.0,     1232.0,     0.0,     4.0 },
          {     -4230.0,       0.0,      5.0,      -20.0,     0.0,    -2.0 },
          {     -2819.0,       0.0,      7.0,     1207.0,     0.0,     3.0 },
          {     -4056.0,       0.0,      5.0,       40.0,     0.0,    -2.0 },
          {     -2647.0,       0.0,     11.0,     1129.0,     0.0,     5.0 },
          {     -2294.0,       0.0,    -10.0,     1266.0,     0.0,    -4.0 },
          {      2481.0,       0.0,     -7.0,    -1062.0,     0.0,    -3.0 },
          {      2179.0,       0.0,     -2.0,    -1129.0,     0.0,    -2.0 },
          {      3276.0,       0.0,      1.0,       -9.0,     0.0,     0.0 },
          {     -3389.0,       0.0,      5.0,       35.0,     0.0,    -2.0 },
          {      3339.0,       0.0,    -13.0,     -107.0,     0.0,     1.0 },
          {     -1987.0,       0.0,     -6.0,     1073.0,     0.0,    -2.0 },
          {     -1981.0,       0.0,      0.0,      854.0,     0.0,     0.0 },
          {      4026.0,       0.0,   -353.0,     -553.0,     0.0,  -139.0 },
          {      1660.0,       0.0,     -5.0,     -710.0,     0.0,    -2.0 },
          {     -1521.0,       0.0,      9.0,      647.0,     0.0,     4.0 },
          {      1314.0,       0.0,      0.0,     -700.0,     0.0,     0.0 },
          {     -1283.0,       0.0,      0.0,      672.0,     0.0,     0.0 },
          {     -1331.0,       0.0,      8.0,      663.0,     0.0,     4.0 },
          {      1383.0,       0.0,     -2.0,     -594.0,     0.0,    -2.0 },
          {      1405.0,       0.0,      4.0,     -610.0,     0.0,     2.0 },
          {      1290.0,       0.0,      0.0,     -556.0,     0.0,     0.0 }
 };

/*
   Interval between fundamental epoch J2000.0 and given date.
*/

   t = ((jd_high - T0) + jd_low) / 36525.0;

/*
   ** Luni-solar nutation. **

   Fundamental (Delaunay) arguments from Simon et al. (1994),
   in radians.
*/

/*
   Mean anomaly of the Moon.
*/

   el  = fmod (485868.249036 +
          t * 1717915923.2178, ASEC360) * ASEC2RAD;

/*
   Mean anomaly of the Sun.
*/

   elp = fmod (1287104.79305 +
             t * 129596581.0481, ASEC360) * ASEC2RAD;

/*
   Mean argument of the latitude of the Moon.
*/

   f   = fmod (335779.526232 +
             t * 1739527262.8478, ASEC360) * ASEC2RAD;

/*
   Mean elongation of the Moon from the Sun.
*/

   d   = fmod (1072260.70369 +
             t * 1602961601.2090, ASEC360) * ASEC2RAD;

/*
   Mean longitude of the ascending node of the Moon.
*/

   om  = fmod (450160.398036 -
             t * 6962890.5431, ASEC360) * ASEC2RAD;

/*
  Initialize the nutation values.
*/

   dp = 0.0;
   de = 0.0;

/*
  Summation of luni-solar nutation series (in reverse order).
*/

   for (i = 76; i >= 0; i--)
   {

/*
   Argument and functions.
*/

      arg = fmod ((double) nals_t[i][0] * el  +
                  (double) nals_t[i][1] * elp +
                  (double) nals_t[i][2] * f   +
                  (double) nals_t[i][3] * d   +
                  (double) nals_t[i][4] * om,   TWOPI);

      sarg = sin (arg);
      carg = cos (arg);

/*
   Term.
*/

      dp += (cls_t[i][0] + cls_t[i][1] * t) * sarg
              +   cls_t[i][2] * carg;
      de += (cls_t[i][3] + cls_t[i][4] * t) * carg
              +   cls_t[i][5] * sarg;
   }

/*
  Convert from 0.1 microarcsec units to radians.
*/

   factor = 1.0e-7 * ASEC2RAD;
   dpsils = dp * factor;
   depsls = de * factor;

/*
  ** Planetary nutation. **

  Fixed terms to allow for long-period nutation, in radians.
*/

   dpsipl = dpplan * ASEC2RAD;
   depspl = deplan * ASEC2RAD;

/*
   Total: Add planetary and luni-solar components.
*/

   *dpsi = dpsipl + dpsils;
   *deps = depspl + depsls;

   return;
}
