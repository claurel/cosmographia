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

#ifndef NORAD_H
#define NORAD_H 1

/* #define RETAIN_PERTURBATION_VALUES_AT_EPOCH 1 */

/* Two-line-element satellite orbital data */
typedef struct
{
  double epoch, xndt2o, xndd6o, bstar;
  double xincl, xnodeo, eo, omegao, xmo, xno;
  int ephemeris_type;
} tle_t;


      /* 30 Dec 2003:  if we're retaining the perturbation values at the
         epoch,  it'll cost us another five doubles and an integer
         (see 'norad_in.h'):                                        */
#ifdef RETAIN_PERTURBATION_VALUES_AT_EPOCH
#define DEEP_ARG_T_PARAMS     93
#else
#define DEEP_ARG_T_PARAMS     87
#endif

#define N_SGP_PARAMS          11
#define N_SGP4_PARAMS         29
#define N_SGP8_PARAMS         25
#define N_SDP4_PARAMS        (9 + DEEP_ARG_T_PARAMS)
#define N_SDP8_PARAMS        (11 + DEEP_ARG_T_PARAMS)

/* 87 or 93 = size of the 'deep_arg_t' structure,  in 8-byte units */
/* You can use the above constants to minimize the amount of memory used,
   but if you use the following constant,  you can be assured of having
   enough memory for any of the five models: */

#define N_SAT_PARAMS         (11 + DEEP_ARG_T_PARAMS)

/* Byte 63 of the first line of a TLE contains the ephemeris type.  The */
/* following five values are recommended,  but it seems the non-zero    */
/* values are only used internally;  "published" TLEs all have type 0.  */

#define TLE_EPHEMERIS_TYPE_DEFAULT           0
#define TLE_EPHEMERIS_TYPE_SGP               1
#define TLE_EPHEMERIS_TYPE_SGP4              2
#define TLE_EPHEMERIS_TYPE_SDP4              3
#define TLE_EPHEMERIS_TYPE_SGP8              4
#define TLE_EPHEMERIS_TYPE_SDP8              5

/* Funtion prototypes */
/* norad.c */

         /* The Win32 version can be compiled to make a .DLL,  if the     */
         /* functions are declared to be of type __stdcall... _and_ the   */
         /* functions must be declared to be extern "C",  something I     */
         /* overlooked and added 24 Sep 2002.  The DLL_FUNC macro lets    */
         /* this coexist peacefully with other OSes.                      */

#ifdef _WIN32
#define DLL_FUNC __stdcall
extern "C" {
#else
#define DLL_FUNC
#endif

void DLL_FUNC SGP_init( double *params, const tle_t *tle);
void DLL_FUNC SGP(  const double tsince, const tle_t *tle, const double *params,
                                     double *pos, double *vel);

void DLL_FUNC SGP4_init( double *params, const tle_t *tle);
void DLL_FUNC SGP4( const double tsince, const tle_t *tle, const double *params,
                                     double *pos, double *vel);

void DLL_FUNC SGP8_init( double *params, const tle_t *tle);
void DLL_FUNC SGP8( const double tsince, const tle_t *tle, const double *params,
                                     double *pos, double *vel);

void DLL_FUNC SDP4_init( double *params, const tle_t *tle);
void DLL_FUNC SDP4( const double tsince, const tle_t *tle, const double *params,
                                     double *pos, double *vel);

void DLL_FUNC SDP8_init( double *params, const tle_t *tle);
void DLL_FUNC SDP8( const double tsince, const tle_t *tle, const double *params,
                                     double *pos, double *vel);

int DLL_FUNC select_ephemeris( const tle_t *tle);
int DLL_FUNC parse_elements( const char *line1, const char *line2, tle_t *sat);
int DLL_FUNC tle_checksum( const char *buff);

#ifdef _WIN32
}                       /* end of 'extern "C"' section */
#endif

         /* Following are in 'dynamic.cpp',  for C/C++ programs that want  */
         /* to load 'sat_code.dll' and use its functions at runtime.  They */
         /* only make sense in the Win32 world: */
#ifdef _WIN32
int SXPX_init( double *params, const tle_t *tle, const int sxpx_num);
int SXPX( const double tsince, const tle_t *tle, const double *params,
                               double *pos, double *vel, const int sxpx_num);
#endif
#endif
