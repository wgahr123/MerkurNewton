/*
 *  mercury.h
 *
 *  astronomical constants for mercury
 *
 */

#ifndef _MERCURY_H_
#define _MERCURY_H_

#include "real.hpp"

const real Pi = "3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679";

// https://ssd.jpl.nasa.gov/?constants
// last seen: 2019 July 03 21:51 MESZ

const real NewtonianGravitationalConstant = "6.67259E-11"; //  m3/kg s2
                                        // ± 0.00030E-11       m3/kg s2
const real GaussianGravitationalConstant = "0.01720209895"; // AU3/d2

const int JulianDay = 86400; // s
const real LightTimeForOneAU = "499.004783836"; // s
                             // ± 0.00000001       s

const real MassOfSunDivMassOfMercury = "6023600.0"; // Msun/Mmercury
                                       // ± 250.0

const real AstronomicalUnitDistance = "149597870700"; // m
                                            //  ± 3      m

const real HeliocentricGravitationalConstant = "1.32712440018E+20"; // m3 s-2  G*Msun
                                           // ± 0.00000000008E+20      m3 s-2
const real HeliocentricGravitationalConstant_neg = -HeliocentricGravitationalConstant;

// https://solarsystem.nasa.gov/planets/mercury/by-the-numbers/

const real MeanDistanceFromSunAU = "0.38709893"; // AU
const real MeanDistanceFromSun = MeanDistanceFromSunAU * AstronomicalUnitDistance; // 57909175678.24 m
const real MassOfMercury = "3.30104E+23"; // kg
const real YearOfMercury = real("87.97")*real(JulianDay); // s
const real OrbitalEccentricity = "0.20563593";
const real AbsoluteEccentricity = MeanDistanceFromSun * OrbitalEccentricity;

// http://physics.nist.gov/cgi-bin/cuu/Value?c

const real VelocityOfLight = "299792458"; //  m/s 

// derived constants

const real VelocityPerihelion = sqrt( ( ( ( real("1") + OrbitalEccentricity )
                                        / ( real("1") - OrbitalEccentricity )
                                        )
                                      * HeliocentricGravitationalConstant
                                      ) 
                                    / MeanDistanceFromSun
                                    );

const real VelocityAphelion   = sqrt( ( ( ( real("1") - OrbitalEccentricity )
                                        / ( real("1") + OrbitalEccentricity )
                                        )
                                      * HeliocentricGravitationalConstant
                                      ) 
                                    / MeanDistanceFromSun
                                    );

const real DistancePerihelion = ( real("1") - OrbitalEccentricity )*MeanDistanceFromSun;

const real MassOfSun = "1.9885E+30"; // m

#endif // _MERCURY_H_