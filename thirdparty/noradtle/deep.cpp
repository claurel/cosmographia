// -----------------------------------------------------------------------------------------------
//
// This file is part of a free package distributed by Dr TS Kelso, tkelso@grove.net in his web site
// at http://www.grove.net/~tkelso/
//
// This system carries two-line orbital data (*.tle files with the master list being tle.new) 
// and the following key programs:
//
//* spacetrk.zip (LaTeX documentation and FORTRAN source code for the NORAD
//				orbital models needed for using the two-line element sets),
//* sat-db13.zip (a complete database of all payloads launched into orbit),
//* sgp4-pl2.zip (a library of Turbo Pascal units to implement SGP4/SDP4),
//* trakstr2.zip (an implementation of sgp4-pl2 to output satellite ECI
//				position, subpoint, look angles, and right ascension/declination),
//* passupdt.zip (a package for quickly and easily updating two-line element
//				sets), and
//* unzip (a Sun executable for unzipping these programs).
//
// More information on the first five files is available in the file master.dir.
//
//
//- Dr TS Kelso, tkelso@grove.net
//	Adjunct Professor of Space Operations
//	Air Force Institute of Technology
//	SYSOP, Celestial WWW
//				       http://www.grove.net/~tkelso/
//
// -----------------------------------------------------------------------------------------------

#include <math.h>
#include "norad.h"
#include "norad_in.h"

#define zns      1.19459E-5
#define zes      0.01675
#define znl      1.5835218E-4
#define zel      0.05490
#define thdt     4.3752691E-3

      /* Previously,  the integration step was given as two variables:      */
      /* 'stepp' (positive step = +720) and 'stepn' (negative step = -720). */
      /* Exactly why this should be made a variable,  much less _different_ */
      /* variables for positive and negative,  is entirely unclear...       */
      /* (8 Apr 2003) INTEGRATION_STEP is now a maximum integration step.   */
      /* The code in 'dpsec' splits the integration range into equally-sized */
      /* pieces of 720 minutes (half a day) or smaller.                      */
#define INTEGRATION_STEP 720.
#define STEP2 (INTEGRATION_STEP * INTEGRATION_STEP / 2.)

static double ThetaG( double jd);

/* DEEP */
void Deep_dpinit( const tle_t *tle, deep_arg_t *deep_arg)
{
   const double sinq = sin(tle->xnodeo);
   const double cosq = cos(tle->xnodeo);
   const double aqnv = 1/deep_arg->aodp;
   const double c1ss   =  2.9864797E-6;
   const double day = tle->epoch - 2415020.;
                       /* days since 1900 Jan 0.5 = JD 2415020. */
   double zcosi =  0.91744867;
   double zsini =  0.39785416;
   double zsing = -0.98088458;
   double zcosg =  0.1945905;
   double bfact, cc = c1ss, se;
   double ze = zes, zn = zns;
   double sgh, sh, si;
   double zsinh = sinq, zcosh = cosq;
   double sl;
   int iteration;

   deep_arg->thgr = ThetaG( tle->epoch);
   deep_arg->xnq = deep_arg->xnodp;
   deep_arg->xqncl = tle->xincl;
   deep_arg->omegaq = tle->omegao;

   /* If the epoch has changed,  recompute (or initialize) the lunar and */
   /* solar terms... except that now that zcosil, etc. are within the    */
   /* deep_arg structure,  instead of static,  they must _always_ be     */
   /* recomputed.  So I've commented out the 'if' part.  (Revision made  */
   /* 14 May 2005)                                                       */

/* if (day != deep_arg->preep)   */
      {
      const double xnodce = 4.5236020 - 9.2422029E-4 * day;
      const double stem = sin(xnodce);
      const double ctem = cos(xnodce);
      const double c_minus_gam = 0.228027132 * day - 1.1151842;
      const double gam = 5.8351514 + 0.0019443680 * day;
      double zx, zy;

      deep_arg->preep = day;
      deep_arg->zcosil = 0.91375164 - 0.03568096 * ctem;
      deep_arg->zsinil = sqrt(1. - deep_arg->zcosil * deep_arg->zcosil);
      deep_arg->zsinhl = 0.089683511 * stem / deep_arg->zsinil;
      deep_arg->zcoshl = sqrt(1. - deep_arg->zsinhl*deep_arg->zsinhl);
      deep_arg->zmol = FMod2p( c_minus_gam);
      zx = 0.39785416 * stem / deep_arg->zsinil;
      zy = deep_arg->zcoshl * ctem + 0.91744867 * deep_arg->zsinhl * stem;
      zx = atan2( zx, zy) + gam - xnodce;
      deep_arg->zcosgl = cos( zx);
      deep_arg->zsingl = sin( zx);
      deep_arg->zmos = FMod2p( 6.2565837 + 0.017201977 * day);
      } /* End if(day != deep_arg->preep) */

   /* Do solar terms */
   deep_arg->savtsn = 1E20;

   /* There was previously some convoluted logic here,  but it boils    */
   /* down to this:  we compute the solar terms,  then the lunar terms. */
   /* On a second pass,  we recompute the solar terms,  taking advantage */
   /* of the improved data that resulted from computing lunar terms.     */
   for( iteration = 0; iteration < 2; iteration++)
      {
      const double c1l = 4.7968065E-7;
      const double a1 = zcosg * zcosh + zsing * zcosi * zsinh;
      const double a3 = -zsing * zcosh + zcosg * zcosi * zsinh;
      const double a7 = -zcosg * zsinh + zsing * zcosi * zcosh;
      const double a8 = zsing * zsini;
      const double a9 = zsing * zsinh + zcosg * zcosi * zcosh;
      const double a10 = zcosg * zsini;
      const double a2 = deep_arg->cosio * a7 + deep_arg->sinio * a8;
      const double a4 = deep_arg->cosio * a9 + deep_arg->sinio * a10;
      const double a5 = -deep_arg->sinio * a7 + deep_arg->cosio * a8;
      const double a6 = -deep_arg->sinio * a9 + deep_arg->cosio * a10;
      const double x1 = a1 * deep_arg->cosg + a2 * deep_arg->sing;
      const double x2 = a3 * deep_arg->cosg + a4 * deep_arg->sing;
      const double x3 = -a1 * deep_arg->sing + a2 * deep_arg->cosg;
      const double x4 = -a3 * deep_arg->sing + a4 * deep_arg->cosg;
      const double x5 = a5 * deep_arg->sing;
      const double x6 = a6 * deep_arg->sing;
      const double x7 = a5 * deep_arg->cosg;
      const double x8 = a6 * deep_arg->cosg;
      const double z31 = 12 * x1 * x1 - 3 * x3 * x3;
      const double z32 = 24 * x1 * x2 - 6 * x3 * x4;
      const double z33 = 12 * x2 * x2 - 3 * x4 * x4;
      const double z11 = -6 * a1 * a5 + deep_arg->eosq * (-24 * x1 * x7 - 6 * x3 * x5);
      const double z12 = -6 * (a1 * a6 + a3 * a5) +  deep_arg->eosq *
                (-24 * (x2 * x7 + x1 * x8) - 6 * (x3 * x6 + x4 * x5));
      const double z13 = -6 * a3 * a6 + deep_arg->eosq * (-24 * x2 * x8 - 6 * x4 * x6);
      const double z21 = 6 * a2 * a5 + deep_arg->eosq * (24 * x1 * x5 - 6 * x3 * x7);
      const double z22 = 6 * (a4 * a5 + a2 * a6) +  deep_arg->eosq *
                (24 * (x2 * x5 + x1 * x6) - 6 * (x4 * x7 + x3 * x8));
      const double z23 = 6 * a4 * a6 + deep_arg->eosq * (24 * x2 * x6 - 6 * x4 * x8);
      const double s3 = cc / deep_arg->xnq;
      const double s2 = -0.5 * s3 / deep_arg->betao;
      const double s4 = s3 * deep_arg->betao;
      const double s1 = -15 * tle->eo * s4;
      const double s5 = x1 * x3 + x2 * x4;
      const double s6 = x2 * x3 + x1 * x4;
      const double s7 = x2 * x4 - x1 * x3;
      double z1 = 3 * (a1 * a1 + a2 * a2) + z31 * deep_arg->eosq;
      double z2 = 6 * (a1 * a3 + a2 * a4) + z32 * deep_arg->eosq;
      double z3 = 3 * (a3 * a3 + a4 * a4) + z33 * deep_arg->eosq;

      z1 = z1 + z1 + deep_arg->betao2 * z31;
      z2 = z2 + z2 + deep_arg->betao2 * z32;
      z3 = z3 + z3 + deep_arg->betao2 * z33;
      se = s1*zn*s5;
      si = s2*zn*(z11+z13);
      sl = -zn*s3*(z1+z3-14-6*deep_arg->eosq);
      sgh = s4*zn*(z31+z33-6);
      if (deep_arg->xqncl < 5.2359877E-2)
         sh = 0;
      else
         sh = -zn*s2*(z21+z23);
      deep_arg->ee2 = 2*s1*s6;
      deep_arg->e3 = 2*s1*s7;
      deep_arg->xi2 = 2*s2*z12;
      deep_arg->xi3 = 2*s2*(z13-z11);
      deep_arg->xl2 = -2*s3*z2;
      deep_arg->xl3 = -2*s3*(z3-z1);
      deep_arg->xl4 = -2*s3*(-21-9*deep_arg->eosq)*ze;
      deep_arg->xgh2 = 2*s4*z32;
      deep_arg->xgh3 = 2*s4*(z33-z31);
      deep_arg->xgh4 = -18*s4*ze;
      deep_arg->xh2 = -2*s2*z22;
      deep_arg->xh3 = -2*s2*(z23-z21);

      if( !iteration)   /* we compute lunar terms only on the first pass: */
         {
         deep_arg->sse = se;
         deep_arg->ssi = si;
         deep_arg->ssl = sl;
         deep_arg->ssh = sh/deep_arg->sinio;
         deep_arg->ssg = sgh-deep_arg->cosio*deep_arg->ssh;
         deep_arg->se2 = deep_arg->ee2;
         deep_arg->si2 = deep_arg->xi2;
         deep_arg->sl2 = deep_arg->xl2;
         deep_arg->sgh2 = deep_arg->xgh2;
         deep_arg->sh2 = deep_arg->xh2;
         deep_arg->se3 = deep_arg->e3;
         deep_arg->si3 = deep_arg->xi3;
         deep_arg->sl3 = deep_arg->xl3;
         deep_arg->sgh3 = deep_arg->xgh3;
         deep_arg->sh3 = deep_arg->xh3;
         deep_arg->sl4 = deep_arg->xl4;
         deep_arg->sgh4 = deep_arg->xgh4;
         zcosg = deep_arg->zcosgl;
         zsing = deep_arg->zsingl;
         zcosi = deep_arg->zcosil;
         zsini = deep_arg->zsinil;
         zcosh = deep_arg->zcoshl*cosq+deep_arg->zsinhl*sinq;
         zsinh = sinq*deep_arg->zcoshl-cosq*deep_arg->zsinhl;
         zn = znl;
         cc = c1l;
         ze = zel;
         }
      }

   deep_arg->sse += se;
   deep_arg->ssi += si;
   deep_arg->ssl += sl;
   deep_arg->ssg += sgh - deep_arg->cosio / deep_arg->sinio * sh;
   deep_arg->ssh += sh / deep_arg->sinio;

   if( deep_arg->xnq >= 0.00826 && deep_arg->xnq <= 0.00924 && tle->eo >= .5)
      {           /* start of 12-hour orbit, e >.5 section */
      const double root22 = 1.7891679E-6;
      const double root32 = 3.7393792E-7;
      const double root44 = 7.3636953E-9;
      const double root52 = 1.1428639E-7;
      const double root54 = 2.1765803E-9;
      const double g201 = -0.306 - (tle->eo - 0.64) * 0.440;
      const double eoc = tle->eo * deep_arg->eosq;
      const double sini2 = deep_arg->sinio*deep_arg->sinio;
      const double f220 = 0.75*(1+2*deep_arg->cosio+deep_arg->theta2);
      const double f221 = 1.5 * sini2;
      const double f321 = 1.875 * deep_arg->sinio * (1 - 2 *\
               deep_arg->cosio - 3 * deep_arg->theta2);
      const double f322 = -1.875*deep_arg->sinio*(1+2*
               deep_arg->cosio-3*deep_arg->theta2);
      const double f441 = 35 * sini2 * f220;
      const double f442 = 39.3750 * sini2 * sini2;
      const double f522 = 9.84375*deep_arg->sinio*(sini2*(1-2*deep_arg->cosio-5*
                 deep_arg->theta2)+0.33333333*(-2+4*deep_arg->cosio+
                 6*deep_arg->theta2));
      const double f523 = deep_arg->sinio*(4.92187512*sini2*(-2-4*
                 deep_arg->cosio+10*deep_arg->theta2)+6.56250012
                 *(1+2*deep_arg->cosio-3*deep_arg->theta2));
      const double f542 = 29.53125*deep_arg->sinio*(2-8*
                 deep_arg->cosio+deep_arg->theta2*
                 (-12+8*deep_arg->cosio+10*deep_arg->theta2));
      const double f543 = 29.53125*deep_arg->sinio*(-2-8*deep_arg->cosio+
                 deep_arg->theta2*(12+8*deep_arg->cosio-10*
                 deep_arg->theta2));
      double g410, g422, g520, g521, g532, g533;
      double g211, g310, g322;
      double temp, temp1;

      deep_arg->resonance_flag = 1;       /* it _is_ resonant... */
      deep_arg->synchronous_flag = 0;     /* but it's not synchronous */
             /* Geopotential resonance initialization for 12 hour orbits: */
      if (tle->eo <= 0.65)
         {
         g211 = 3.616-13.247*tle->eo+16.290*deep_arg->eosq;
         g310 = -19.302+117.390*tle->eo-228.419*
                    deep_arg->eosq+156.591*eoc;
         g322 = -18.9068+109.7927*tle->eo-214.6334*
                    deep_arg->eosq+146.5816*eoc;
         g410 = -41.122+242.694*tle->eo-471.094*
                    deep_arg->eosq+313.953*eoc;
         g422 = -146.407+841.880*tle->eo-1629.014*
                    deep_arg->eosq+1083.435*eoc;
         g520 = -532.114+3017.977*tle->eo-5740*
                    deep_arg->eosq+3708.276*eoc;
         }
      else
         {
         g211 = -72.099+331.819*tle->eo-508.738*
                    deep_arg->eosq+266.724*eoc;
         g310 = -346.844+1582.851*tle->eo-2415.925*
                    deep_arg->eosq+1246.113*eoc;
         g322 = -342.585+1554.908*tle->eo-2366.899*
                    deep_arg->eosq+1215.972*eoc;
         g410 = -1052.797+4758.686*tle->eo-7193.992*
                    deep_arg->eosq+3651.957*eoc;
         g422 = -3581.69+16178.11*tle->eo-24462.77*
                    deep_arg->eosq+ 12422.52*eoc;
         if (tle->eo <= 0.715)
            g520 = 1464.74-4664.75*tle->eo+3763.64*deep_arg->eosq;
         else
            g520 = -5149.66+29936.92*tle->eo-54087.36*
                         deep_arg->eosq+31324.56*eoc;
         } /* End if (tle->eo <= 0.65) */

      if (tle->eo < 0.7)
         {
         g533 = -919.2277+4988.61*tle->eo-9064.77*
                    deep_arg->eosq+5542.21*eoc;
         g521 = -822.71072+4568.6173*tle->eo-8491.4146*
                    deep_arg->eosq+5337.524*eoc;
         g532 = -853.666+4690.25*tle->eo-8624.77*
                    deep_arg->eosq+ 5341.4*eoc;
         }
      else
         {
         g533 = -37995.78+161616.52*tle->eo-229838.2*
                    deep_arg->eosq+109377.94*eoc;
         g521 = -51752.104+218913.95*tle->eo-309468.16*
                    deep_arg->eosq+146349.42*eoc;
         g532 = -40023.88+170470.89*tle->eo-242699.48*
                    deep_arg->eosq+115605.82*eoc;
         } /* End if (tle->eo <= 0.7) */

      temp1 = 3 * deep_arg->xnq * deep_arg->xnq * aqnv * aqnv;
      temp = temp1*root22;
      deep_arg->d2201 = temp * f220 * g201;
      deep_arg->d2211 = temp * f221 * g211;
      temp1 *= aqnv;
      temp = temp1*root32;
      deep_arg->d3210 = temp * f321 * g310;
      deep_arg->d3222 = temp * f322 * g322;
      temp1 *= aqnv;
      temp = 2*temp1*root44;
      deep_arg->d4410 = temp * f441 * g410;
      deep_arg->d4422 = temp * f442 * g422;
      temp1 *= aqnv;
      temp = temp1*root52;
      deep_arg->d5220 = temp * f522 * g520;
      deep_arg->d5232 = temp * f523 * g532;
      temp = 2*temp1*root54;
      deep_arg->d5421 = temp * f542 * g521;
      deep_arg->d5433 = temp * f543 * g533;
      deep_arg->xlamo = tle->xmo+tle->xnodeo+tle->xnodeo-deep_arg->thgr-deep_arg->thgr;
      bfact = deep_arg->xmdot + deep_arg->xnodot+
                   deep_arg->xnodot - thdt - thdt;
      bfact += deep_arg->ssl + deep_arg->ssh + deep_arg->ssh;
      }        /* end of 12-hour orbit, e >.5 section */
   else if( (deep_arg->xnq < 0.0052359877) && (deep_arg->xnq > 0.0034906585))
      {
      const double q22    =  1.7891679E-6;
      const double q31    =  2.1460748E-6;
      const double q33    =  2.2123015E-7;
      const double cosio_plus_1 = 1. + deep_arg->cosio;
      const double g200 = 1+deep_arg->eosq*(-2.5+0.8125*deep_arg->eosq);
      const double g300 = 1+deep_arg->eosq*(-6+6.60937*deep_arg->eosq);
      const double f311 = 0.9375*deep_arg->sinio*deep_arg->sinio*
             (1+3*deep_arg->cosio)-0.75*cosio_plus_1;
      const double g310 = 1+2*deep_arg->eosq;
      const double f220 = 0.75 * cosio_plus_1 * cosio_plus_1;
      const double f330 = 2.5 * f220 * cosio_plus_1;

      deep_arg->resonance_flag = deep_arg->synchronous_flag = 1;
      /* Synchronous resonance terms initialization */
      deep_arg->del1 = 3*deep_arg->xnq*deep_arg->xnq*aqnv*aqnv;
      deep_arg->del2 = 2*deep_arg->del1*f220*g200*q22;
      deep_arg->del3 = 3*deep_arg->del1*f330*g300*q33*aqnv;
      deep_arg->del1 = deep_arg->del1*f311*g310*q31*aqnv;
      deep_arg->xlamo = tle->xmo+tle->xnodeo+tle->omegao-deep_arg->thgr;
      bfact = deep_arg->xmdot + deep_arg->omgdot + deep_arg->xnodot - thdt;
      bfact = bfact+deep_arg->ssl+deep_arg->ssg+deep_arg->ssh;
      } /* End of geosych case */
   else              /* it's neither a high-e 12-hr orbit nor a geosynch: */
      deep_arg->resonance_flag = deep_arg->synchronous_flag = 0;

   if( deep_arg->resonance_flag)
      {
      deep_arg->xfact = bfact-deep_arg->xnq;

      /* Initialize integrator */
      deep_arg->xli = deep_arg->xlamo;
      deep_arg->xni = deep_arg->xnq;
      deep_arg->atime = 0;
      }
   /* End case dpinit: */
}

void Deep_dpsec( const tle_t *tle, deep_arg_t *deep_arg)
{
   double delt, temp;
   int n_steps;

   deep_arg->xll += deep_arg->ssl*deep_arg->t;
   deep_arg->omgadf += deep_arg->ssg*deep_arg->t;
   deep_arg->xnode += deep_arg->ssh*deep_arg->t;
   deep_arg->em = tle->eo+deep_arg->sse*deep_arg->t;
   deep_arg->xinc = tle->xincl+deep_arg->ssi*deep_arg->t;
   if (deep_arg->xinc < 0)       /* Begin April 1983 errata correction: */
      {
      deep_arg->xinc = -deep_arg->xinc;
      deep_arg->xnode = deep_arg->xnode + pi;
      deep_arg->omgadf = deep_arg->omgadf-pi;
      }                          /* End April 1983 errata correction. */
   if( !deep_arg->resonance_flag ) return;

            /* If we're closer to t=0 than to the currently-stored data
               from the previous call to this function,  then we're
               better off "restarting",  going back to the initial data */
   if( fabs( deep_arg->t) < fabs( deep_arg->t - deep_arg->atime))
      {                                    /* Epoch restart */
      deep_arg->atime = 0;
      deep_arg->xni = deep_arg->xnq;
      deep_arg->xli = deep_arg->xlamo;
      }

               /* How many integration steps does it take to get   */
               /* get from our starting time, deep_arg->atime,     */
               /* to the desired time,  deep_arg->t?:              */
   n_steps = (int)ceil( fabs( deep_arg->t - deep_arg->atime) / INTEGRATION_STEP);
   if( n_steps)
      delt = (deep_arg->t - deep_arg->atime) / (double)n_steps;
   else
      delt = 0.;

   while( n_steps--)
      {
      const double sin_li = sin( deep_arg->xli);
      const double cos_li = cos( deep_arg->xli);
      const double sin_2li = 2. * sin_li * cos_li;
      const double cos_2li = 2. * cos_li * cos_li - 1.;
      double xldot, xnddt, xndot;

                /* Dot terms calculated,  using a lot of trig add/subtract */
                /* identities to reduce the computational load... at the   */
                /* cost of making the code somewhat hard to follow:        */
      if( deep_arg->synchronous_flag )
         {
/*       const double fasx2 = 0.13130908;       */
/*       const double fasx4 = 2.8843198;        */
/*       const double fasx6 = 0.37448087;       */
         const double c_fasx2 =  0.99139134268488593;
         const double s_fasx2 =  0.13093206501640101;
         const double c_2fasx4 =  0.87051638752972937;
         const double s_2fasx4 = -0.49213943048915526;
         const double c_3fasx6 =  0.43258117585763334;
         const double s_3fasx6 =  0.90159499016666422;
         const double sin_3li = sin_2li * cos_li + cos_2li * sin_li;
         const double cos_3li = cos_2li * cos_li - sin_2li * sin_li;

         xndot = deep_arg->del1 * (sin_li  * c_fasx2  - cos_li  * s_fasx2)
               + deep_arg->del2 * (sin_2li * c_2fasx4 - cos_2li * s_2fasx4)
               + deep_arg->del3 * (sin_3li * c_3fasx6 - cos_3li * s_3fasx6);
         xnddt = deep_arg->del1 * (cos_li  * c_fasx2  + sin_li  * s_fasx2)
           + 2 * deep_arg->del2 * (cos_2li * c_2fasx4 + sin_2li * s_2fasx4)
           + 3 * deep_arg->del3 * (cos_3li * c_3fasx6 + sin_3li * s_3fasx6);
         }        /* end of geosynch case */
      else
         {        /* orbit is a 12-hour resonant one: */
/*       const double g22    =  5.7686396;      */
/*       const double g32    =  0.95240898;     */
/*       const double g44    =  1.8014998;      */
/*       const double g52    =  1.0508330;      */
/*       const double g54    =  4.4108898;      */
         const double c_g22 =  0.87051638752972937;
         const double s_g22 = -0.49213943048915526;
         const double c_g32 =  0.57972190187001149;
         const double s_g32 =  0.81481440616389245;
         const double c_g44 = -0.22866241528815548;
         const double s_g44 =  0.97350577801807991;
         const double c_g52 =  0.49684831179884198;
         const double s_g52 =  0.86783740128127729;
         const double c_g54 = -0.29695209575316894;
         const double s_g54 = -0.95489237761529999;
         const double xomi =
                   deep_arg->omegaq + deep_arg->omgdot * deep_arg->atime;
         const double sin_omi = sin( xomi), cos_omi = cos( xomi);
         const double sin_li_m_omi = sin_li * cos_omi - sin_omi * cos_li;
         const double sin_li_p_omi = sin_li * cos_omi + sin_omi * cos_li;
         const double cos_li_m_omi = cos_li * cos_omi + sin_omi * sin_li;
         const double cos_li_p_omi = cos_li * cos_omi - sin_omi * sin_li;
         const double sin_2omi = 2. * sin_omi * cos_omi;
         const double cos_2omi = 2. * cos_omi * cos_omi - 1.;
         const double sin_2li_m_omi = sin_2li * cos_omi - sin_omi * cos_2li;
         const double sin_2li_p_omi = sin_2li * cos_omi + sin_omi * cos_2li;
         const double cos_2li_m_omi = cos_2li * cos_omi + sin_omi * sin_2li;
         const double cos_2li_p_omi = cos_2li * cos_omi - sin_omi * sin_2li;
         const double sin_2li_p_2omi = sin_2li * cos_2omi + sin_2omi * cos_2li;
         const double cos_2li_p_2omi = cos_2li * cos_2omi - sin_2omi * sin_2li;
         const double sin_2omi_p_li = sin_li * cos_2omi + sin_2omi * cos_li;
         const double cos_2omi_p_li = cos_li * cos_2omi - sin_2omi * sin_li;

         xndot = deep_arg->d2201 * (sin_2omi_p_li*c_g22 - cos_2omi_p_li*s_g22)
               + deep_arg->d2211 * (sin_li * c_g22 - cos_li * s_g22)
               + deep_arg->d3210 * (sin_li_p_omi*c_g32 - cos_li_p_omi*s_g32)
               + deep_arg->d3222 * (sin_li_m_omi*c_g32 - cos_li_m_omi*s_g32)
               + deep_arg->d4410 * (sin_2li_p_2omi*c_g44 - cos_2li_p_2omi*s_g44)
               + deep_arg->d4422 * (sin_2li * c_g44 - cos_2li * s_g44)
               + deep_arg->d5220 * (sin_li_p_omi*c_g52 - cos_li_p_omi*s_g52)
               + deep_arg->d5232 * (sin_li_m_omi*c_g52 - cos_li_m_omi*s_g52)
               + deep_arg->d5421 * (sin_2li_p_omi*c_g54 - cos_2li_p_omi*s_g54)
               + deep_arg->d5433 * (sin_2li_m_omi*c_g54 - cos_2li_m_omi*s_g54);
         xnddt = deep_arg->d2201 * (cos_2omi_p_li*c_g22 + sin_2omi_p_li*s_g22)
               + deep_arg->d2211 * (cos_li * c_g22 + sin_li * s_g22)
               + deep_arg->d3210 * (cos_li_p_omi*c_g32 + sin_li_p_omi*s_g32)
               + deep_arg->d3222 * (cos_li_m_omi*c_g32 + sin_li_m_omi*s_g32)
               + deep_arg->d5220 * (cos_li_p_omi*c_g52 + sin_li_p_omi*s_g52)
               + deep_arg->d5232 * (cos_li_m_omi*c_g52 + sin_li_m_omi*s_g52)
            + 2*(deep_arg->d4410 * (cos_2li_p_2omi*c_g44 + sin_2li_p_2omi*s_g44)
               + deep_arg->d4422 * (cos_2li * c_g44 + sin_2li * s_g44)
               + deep_arg->d5421 * (cos_2li_p_omi*c_g54 + sin_2li_p_omi*s_g54)
               + deep_arg->d5433 * (cos_2li_m_omi*c_g54 + sin_2li_m_omi*s_g54));
         } /* End of 12-hr resonant case */

      xldot = deep_arg->xni+deep_arg->xfact;
      xnddt *= xldot;

      deep_arg->xli += delt * (xldot + xndot * delt / 2.);
      deep_arg->xni += delt * (xndot + xnddt * delt / 2.);
      deep_arg->atime += delt;
      }

   deep_arg->xn = deep_arg->xni;

   temp = -deep_arg->xnode + deep_arg->thgr + deep_arg->t * thdt;

   deep_arg->xll = deep_arg->xli + temp
         + (deep_arg->synchronous_flag ? -deep_arg->omgadf : temp);
   /*End case dpsec: */
}

void Deep_dpper( deep_arg_t *deep_arg)
{
               /* If the time didn't change by more than 30 minutes,     */
               /* there's no good reason to recompute the perturbations; */
               /* they don't change enough over so short a time span.    */
      if (fabs(deep_arg->savtsn-deep_arg->t) >= 30.)
         {
         double zf, zm, sinzf, ses, sis, sil, sel, sll, sls;
         double f2, f3, sghl, sghs, shs, sh1;

         deep_arg->savtsn = deep_arg->t;

               /* Update solar perturbations for time T: */
         zm = deep_arg->zmos+zns*deep_arg->t;
         zf = zm+2*zes*sin(zm);
         sinzf = sin(zf);
         f2 = 0.5*sinzf*sinzf-0.25;
         f3 = -0.5*sinzf*cos(zf);
         ses = deep_arg->se2*f2+deep_arg->se3*f3;
         sis = deep_arg->si2*f2+deep_arg->si3*f3;
         sls = deep_arg->sl2*f2+deep_arg->sl3*f3+deep_arg->sl4*sinzf;
         sghs = deep_arg->sgh2*f2+deep_arg->sgh3*f3+deep_arg->sgh4*sinzf;
         shs = deep_arg->sh2*f2+deep_arg->sh3*f3;

               /* Update lunar perturbations for time T: */
         zm = deep_arg->zmol+znl*deep_arg->t;
         zf = zm+2*zel*sin(zm);
         sinzf = sin(zf);
         f2 = 0.5*sinzf*sinzf-0.25;
         f3 = -0.5*sinzf*cos(zf);
         sel = deep_arg->ee2*f2+deep_arg->e3*f3;
         sil = deep_arg->xi2*f2+deep_arg->xi3*f3;
         sll = deep_arg->xl2*f2+deep_arg->xl3*f3+deep_arg->xl4*sinzf;
         sghl = deep_arg->xgh2*f2+deep_arg->xgh3*f3+deep_arg->xgh4*sinzf;
         sh1 = deep_arg->xh2*f2+deep_arg->xh3*f3;

               /* Sum the solar and lunar contributions: */
         deep_arg->pe = ses+sel;
         deep_arg->pinc = sis+sil;
         deep_arg->pl = sls+sll;
         deep_arg->pgh = sghs+sghl;
         deep_arg->ph = shs+sh1;
#ifdef RETAIN_PERTURBATION_VALUES_AT_EPOCH
         if( deep_arg->solar_lunar_init_flag)
            {
            deep_arg->pe0 = deep_arg->pe;
            deep_arg->pinc0 = deep_arg->pinc;
            deep_arg->pl0 = deep_arg->pl;
            deep_arg->pgh0 = deep_arg->pgh;
            deep_arg->ph0 = deep_arg->ph;
            }
         deep_arg->pe  -= deep_arg->pe0;
         deep_arg->pinc -= deep_arg->pinc0;
         deep_arg->pl  -= deep_arg->pl0;
         deep_arg->pgh -= deep_arg->pgh0;
         deep_arg->ph  -= deep_arg->ph0;
         if( deep_arg->solar_lunar_init_flag)
            return;        /* done all we really need to do here... */
#endif
         }

               /* In Spacetrack 3, sinis & cosis were initialized       */
               /* _before_ perturbations were added to xinc.  In        */
               /* Spacetrack 6,  it's the other way around (see below). */
#ifdef SPACETRACK_3
      const double sinis = sin( deep_arg->xinc);
      const double cosis = cos( deep_arg->xinc);
#endif
         /* Add solar/lunar perturbation correction to inclination: */
      deep_arg->xinc += deep_arg->pinc;

         /* Add solar/lunar perturbation correction to eccentricity: */
      deep_arg->em += deep_arg->pe;

      if (deep_arg->xqncl >= 0.2)
         {
                /* Apply periodics directly */
         const double temp_val = deep_arg->ph / deep_arg->sinio;

         deep_arg->omgadf += deep_arg->pgh - deep_arg->cosio * temp_val;
         deep_arg->xnode += temp_val;
         deep_arg->xll += deep_arg->pl;
         }
      else
         {
           /* Apply periodics with Lyddane modification */
         const double sinok = sin(deep_arg->xnode);
         const double cosok = cos(deep_arg->xnode);
               /* Correction from Spacetrack Report #3 to #6:      */
               /* used to be sinis & cosis were computed _before_  */
               /* adding perturbations to XINC.  Now it's _after_: */
#ifndef SPACETRACK_3
         const double sinis = sin(deep_arg->xinc);
         const double cosis = cos(deep_arg->xinc);
#endif
         const double alfdp = deep_arg->ph * cosok
                    + (deep_arg->pinc * cosis + sinis) * sinok;
         const double betdp = - deep_arg->ph * sinok
                    + (deep_arg->pinc * cosis + sinis) * cosok;
         double xls, xnoh;

         deep_arg->xnode = FMod2p(deep_arg->xnode);
         xls = deep_arg->xll + deep_arg->omgadf + cosis * deep_arg->xnode;
         xls += deep_arg->pl + deep_arg->pgh
                            - deep_arg->pinc * deep_arg->xnode * sinis;
         xnoh = deep_arg->xnode;
         deep_arg->xnode = atan2(alfdp,betdp);

          /* This is a patch to Lyddane modification suggested */
          /* by Rob Matson, streamlined very slightly by BJG, to */
          /* keep 'xnode' & 'xnoh' within 180 degrees of each other. */

         if( deep_arg->xnode < xnoh - pi)
            deep_arg->xnode += twopi;
         else if( deep_arg->xnode > xnoh + pi)
            deep_arg->xnode -= twopi;

         deep_arg->xll += deep_arg->pl;
         deep_arg->omgadf = xls - deep_arg->xll - cos(deep_arg->xinc)*
                             deep_arg->xnode;
         } /* End case dpper: */
}

/*------------------------------------------------------------------*/

/* THETAG */
static double ThetaG( double jd)
{
  /* Reference:  The 1992 Astronomical Almanac, page B6. */
  const double omega_E = 1.00273790934;
                   /* Earth rotations per sidereal day (non-constant) */
  const double UT = fmod( jd + .5, 1.);
  double t_cen, GMST, rval;

  t_cen = (jd - UT - 2451545.0) / 36525.;
  GMST = 24110.54841 + t_cen * (8640184.812866 + t_cen *
                           (0.093104 - t_cen * 6.2E-6));
  GMST = fmod( GMST + secday * omega_E * UT, secday);
  if( GMST < 0.)
     GMST += secday;
  rval = twopi * GMST / secday;

  return( rval);
} /*Function thetag*/
