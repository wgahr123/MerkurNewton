/*
 *	MerkurNewton.cpp
 *
 *------------------------------------------------------------
 *  09.12.2019	Wolfgang Gahr	. Erstellung
 *------------------------------------------------------------
 */

#include <iomanip>
#include <iostream>
#include <sys/timeb.h>
#include <time.h>

#include <set>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#include <string>
#include <sysinfoApi.h>
#else
#include <stdlib.h>
#include <string>
#include <fstream>
#include <math.h>
#include <sys/times.h>
#include <unistd.h>
#endif

#include "real.hpp"
#include "vector2.hpp"
#include "mercury.h"
#include "cxxopts.hpp"

const std::string VERSION = "1.0";

const std::string TIMESTEP = "1"; // s
const std::string NUMBER_OF_ORBITS = "3";
const std::string METHOD = "3";
const std::string VIEW = "30"; // timesteps
const std::string PRECISION = "160";

const unsigned int INTERVAL_TO_SAY_HELLO = 2; // s

const real Null = "0";
const real One = "1";
const real Two = "2";
const real Half = "0.5";
const real Degrees = real("180") / Pi;
const real Arcseconds("3600");

#ifdef WIN32
const char FILESEP = '\\';
#else
const char FILESEP = '/';
#endif

typedef struct
{
    std::string timestep = "";      // will be converted to mpfr
    int number_of_orbits = 0;
    int method = 0;
    int view = 0;
    int width_help_line = 0;
    int precision = 0;
} args_t;

std::string get_basename(std::string path)
{
    // std::vector<std::string> strings;
    std::istringstream f(path);
    std::string last;
    while (std::getline(f, last, FILESEP))
        ;
    return last;
}

args_t parse_arguments(int argc, char** argv)
{
    static args_t args;

    try
    {
        cxxopts::Options options(get_basename(argv[0]),
                                 get_basename(argv[0]) + " - Simulate the orbit of mercury");

        options.positional_help("[optional args]")
            .show_positional_help()
            .allow_unrecognised_options()
            .set_width(80)
            .add_options()("h,help", "Print help")(
                "t,timestep", "Set timestep in seconds",
                cxxopts::value<std::string>(args.timestep)->default_value(TIMESTEP))(
                "n,number_of_orbits", "Number of orbits to be computed - no limit: 0",
                cxxopts::value<int>(args.number_of_orbits)->default_value(NUMBER_OF_ORBITS))(
                "m,method",
                "Method for simulation\n"
                "Method 1:\tsteps with linear simulation\n"
                "Method 2:\tsteps with quadratic simulation\n"
                "Method 3:\tsteps with quadratic simulation and\n\t\taverage of accelerations",
                cxxopts::value<int>(args.method)->default_value(METHOD))(
                "v,view", "Print results in range -val..+val timesteps at the perihel",
                cxxopts::value<int>(args.view)->default_value(VIEW))(
                "p,precision", "Set precision for mpir/mpfr in bits (default: 160 bits)",
                cxxopts::value<int>(args.precision)->default_value(PRECISION));

        auto result = options.parse(argc, argv);

        if (result.count("help"))
        {
            std::cout << options.help() << std::endl;
            exit(0);
        }

        // check values

        std::string msg = "";

        if (std::atof(args.timestep.c_str()) <= 0)
            msg = "error: timestep must be greater than 0 seconds";
        if (args.number_of_orbits < 0) msg = "error: number of orbits must be greater or equal 0";
        if (1 > args.method || args.method > 3) msg = "error: method must be 1, 2, or 3";
        if (args.view < 0) msg = "error: view must be greater than 0 arcsecs";
        if (args.precision < 32)
            msg = "error: precision must be greater than 31 bits";

        if (msg.length() > 0)
        {
            std::cerr << msg << std::endl;
            exit(1);
        }

        return args;
    } catch (const cxxopts::OptionException& e)
    {
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }
}

inline void get_angle(const vector2& vec, real& angle)
// angle between +x-axis and vector2, in rad.
{
    angle = atan2(vec.y(), vec.x());
}

inline void get_angle_double(const vector2& vec, double& angle)
// angle between +x-axis and vector2, in rad.
// in double
{
    angle = atan2(double(vec.y()), double(vec.x()));
}

inline void get_accel(const vector2& radius, vector2& accel)
{
    // accel = -(HeliocentricGravitationalConstant / absRadius^3) * radius;

    static real a("0");
    static real b("0");
    static real c("0");

    // a = radius.abs();
    a = radius.x();
    a.sqr_me();
    b = radius.y();
    b.sqr_me();
    a += b;
    a.sqrt_me();

    // b = a^3;
    b = a;
    b *= a;
    b *= a;

    // b = -HeliocentricGravitationalConstant / b;
    b.inv_me();
    b *= HeliocentricGravitationalConstant_neg;

    accel = radius;
    accel *= b;
}

inline void changes(const vector2& acceleration, const real& timeStep, const vector2& velocity,
                    vector2& changeRadius, vector2& changeVelocity)
{
    // delta(v) = a*delta(t)
    changeVelocity = acceleration;

    // delta(r) = v*delta(t) + a*delta(t)*delta(t)/2 = ( v + delta(v)/2 ) * delta(t)
    changeRadius = changeVelocity;
    changeRadius *= Half;
    changeRadius += velocity;

    if (timeStep == One)
        return;

    changeVelocity *= timeStep;
    changeRadius *= timeStep;
}

void method3(const vector2& radius, const vector2& velocity, const real& timeStep,
//    vector2& changeRadius, vector2& changeVelocity)
    vector2& acceleration)
{
    // 1. get the acceleration at radius (acceleration_old)
    // 2. get a new radius with method 2 (radius_try)
    // 3. get the acceleration at radius_try (acceleration_new)
    // 4. now take the mean of these two accelerations (acceleration)
    // 5. get the new radius by this mean acceleration

    //static vector2 acceleration;
    static vector2 acceleration_try;
    static vector2 radius_try;
    static vector2 changeRadius;
    static vector2 changeVelocity;

    get_accel(radius, acceleration);
    changes(acceleration, timeStep, velocity, changeRadius, changeVelocity);

    radius_try = radius;
    radius_try += changeRadius;

    get_accel(radius_try, acceleration_try);

    // acceleration = (acceleration + acceleration_try)/2;
    acceleration += acceleration_try;
    acceleration *= Half;

    //changes(acceleration, timeStep, velocity, changeRadius, changeVelocity);
}

//void method4(vector2& radius, vector2& velocity, const real& timeStep)
//{
//    real durationOfLightSunMercury = radius.abs();
//    durationOfLightSunMercury /= VelocityOfLight;
//
//    real stepsTime = durationOfLightSunMercury;
//    stepsTime /= timeStep;
//    // std::cout << durationOfLightSunMercury.write("%Rf s, ") << stepsTime.write("%Rf steps") <<
//    // std::endl;
//
//    // values before -stepsTime:
//
//    static vector2 radius_left, velocity_left, radius_right, velocity_right;
//
//    vector2 radius_1 = radius;
//    vector2 velocity_1 = velocity;
//
//    int iStepsTime = stepsTime;
//    for (int s = 1; s < iStepsTime; s++)
//    {
//        static real step;
//        if (s == 1) { step = timeStep; }
//        else
//        {
//            step += timeStep;
//        }
//
//        // std::cout << s << step.write(", %Rf s") << std::endl;
//        method3(radius_1, velocity_1, -timeStep);
//    }
//
//    // values after -stepsTime:
//
//    vector2 radius_2 = radius_1;
//    vector2 velocity_2 = velocity_1;
//    method3(radius_2, velocity_2, -timeStep);
//
//    // interpolation
//
//    // radius_old = radius_1 + ((stepsTime % timeStep) / timeStep) * (radius_2 - radius_1);
//    static real c;
//    c = stepsTime;
//    c %= timeStep;
//    c /= timeStep; // 0..1
//    vector2 radius_old = radius_2;
//    radius_old -= radius_1;
//    radius_old *= c;
//    radius_old += radius_1;
//    // std::cout << radius_old.write("old radius %Rf m") << std::endl;
//
//    // now the real step:
//
//    vector2 acceleration_old;
//    vector2 acceleration;
//
//    get_accel(radius_old, acceleration_old);
//    get_accel(radius, acceleration);
//    // std::cout << (acceleration_old - acceleration).write("delta acceleration %Rf m/s^2") <<
//    // std::endl;
//
//    changes(acceleration_old, timeStep, velocity, changeRadius, changeVelocity);
//    radius += changeRadius;
//    velocity += changeVelocity;
//}

inline void distance_to_Perihelion(const vector2& radius, real& distance)
{
    distance = (radius.y().sqr() + (radius.x() - DistancePerihelion).sqr()).sqrt();
}

void computeAndPrintBestParabel(std::vector<vector2>& pointsAtPerihel)
{
    std::cout << pointsAtPerihel.size() << " points (Delta radius x, radius y)" << std::endl;

    std::cout << "index; delta_radius_x_m; radius_y_m; faktor_parabel_1/m;" << std::endl;
    for (int i = 0; i < pointsAtPerihel.size(); i++)
    {
        real x = pointsAtPerihel[i].x() - DistancePerihelion;
        real y = pointsAtPerihel[i].y();
        real a = real("1E+12") * x / y / y;

        std::cout << std::setw(2) << i << "; " << x.write("%20.6RF; ") << y.write("%20.6Rf; ")
             << a.write("%0.10RfE-12") << std::endl;
    }

    // Mittelwert a und Sigme
    real sumValues = 0;
    real sumValuesSquared = 0;
    for (int i = 0; i < pointsAtPerihel.size(); i++)
    {
        real x = pointsAtPerihel[i].x() - DistancePerihelion;
        real y = pointsAtPerihel[i].y();
        real a = x / y / y;

        sumValues += a;
        sumValuesSquared += a.sqr();
    }
    real size = pointsAtPerihel.size();
    real meanA = sumValues / size;
    real sigmaA = ((sumValuesSquared - meanA * meanA * size) / (size - One)).sqrt();

    std::cout << "Delta x = a*y*y;   mean(a): " << meanA.write("%15.10RF  sigma(a): ")
         << sigmaA.write("%15.10Rf") << std::endl;

    // i in arcsecs
    for (int i = -10; i <= 10; i++)
    {
        real phi = real(i) * (Pi / Arcseconds / real(180));
        real sinPhi = sin(phi);
        real cosPhi = cos(phi);
        real checkFor1 = sinPhi.sqr() + cosPhi.sqr() - One;

        std::cout << "phi[\"]: " << std::setw(3) << i << "  phi[2Pi]: " << phi.write(" % 25.20RF")
             << "  sin(phi): " << sinPhi.write(" %25.20RF  cos(phi): ")
             << cosPhi.write(" %25.20Rf  1-sin^2+cos^2: ") << checkFor1.write(" %25.20RG") << std::endl;
    }
}

#ifdef WIN32
bool get_processtime(double& processTime)
{
    FILETIME creationTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;

    const auto SECS = 1E-7;

    const auto mult = (double(MAXUINT64)+1)*SECS; // secs

    if (!GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime))
        return false;

    processTime = double(userTime.dwHighDateTime) * mult + double(userTime.dwLowDateTime)*SECS;
    //processTime += double(kernelTime.dwHighDateTime) * mult + double(kernelTime.dwLowDateTime)*SECS;

    return true;
}
#else
bool get_processtime(double& userTime)
{
    tms buf;

    size_t cps = sysconf(_SC_CLK_TCK);

    if (times(&buf) == -1)
        return false;

    userTime = double(buf.tms_utime + buf.tms_stime)/cps;

    return true;
}
#endif

std::string get_processtime_str()
{
    double processTime;
    std::stringstream sstr;
    size_t hours;
    size_t mins;
    size_t secs;
    size_t msecs;

    if (!get_processtime(processTime)) return std::string();

    msecs = static_cast<size_t>(processTime * 1000) % 1000;
    hours = floor(processTime/3600);
    processTime -= hours*3600;
    mins = floor(processTime/60);
    secs = floor(processTime - mins*60);

    sstr << std::setfill('0') << std::setw(2) << hours << ':'
         << std::setfill('0') << std::setw(2) << mins << ':'
         << std::setfill('0') << std::setw(2) << secs << '.'
         << std::setfill('0') << std::setw(3) << msecs;

    std::string str;
    sstr >> str;

    return str;
}

bool its_time_to_say_hello(unsigned int interval)
{
    // say hello at every INTERVAL secs
    double processTime;
    static double nextProcessTime = 0;

    if (!get_processtime(processTime))
        return false;

    if (processTime < nextProcessTime)
        return false;

    nextProcessTime = processTime + interval;

    return true;
}

int main(int argc, char* argv[])
{
    auto args = parse_arguments(argc, argv);

    std::cerr
#ifdef _MSC_FULL_VER
        << "compiled with Visual Studio compiler version " << _MSC_FULL_VER << " on " << __DATE__ << " at " << __TIME__ << std::endl
#endif
        << "using lib_mpir (lib_mpir_haswell_avx from https://github.com/KevinHake/mpir.git)" << std::endl
        << "using lib_mpfr (lib_mpfr from https://github.com/BrianGladman/mpfr.git)" << std::endl
        << "using mpir and mpfr with " << args.precision << " bits" << std::endl
        << "using cxxopts.hpp from https://github.com/jarro2783/cxxopts" << std::endl
        << std::endl;

    real::set_precision(args.precision);

    vector2 radius(DistancePerihelion, Null);
    vector2 velocity(Null, VelocityPerihelion);
    vector2 acceleration(Null, Null);
    vector2 changeVelocity;
    vector2 changeRadius;

    real factor;
    real factor3;
    real trueAnomaly;
    real angleOfVelocity;
    real absRadius;
    real radius_x;
    real radius_y;
    real velocity_x;
    real velocity_y;
    real distance;
    real durationOfLightSunMercury;
    real stepsTime;

    vector2 acceleration_old;
    vector2 changeVelocity_try;
    vector2 radius_try;
    vector2 velocity_try;
    vector2 acceleration_new;
    vector2 changeVelocity_new;
    vector2 radius_new;
    vector2 velocity_new;

    vector2 radius_1;
    vector2 velocity_1;
    vector2 radius_2;
    vector2 velocity_2;
    vector2 radius_old;

    bool atPerihel;
    int numberOfPointsAtPerihel = 0;
    std::vector<vector2> pointsAtPerihel;

    real timeStep(args.timestep.c_str()); // sec
    real currentTime(0);
    real lastCurrentTime(currentTime);
    std::cerr << "args.timestep: " << args.timestep << std::endl;
    std::cerr << "timeStep [sec]: " << timeStep.write() << std::endl;

    if (timeStep.abs() < real("0.001"))
    {
        std::cerr << "timestep must be > 1 msec" << std::endl;
        exit(1);
    }

    size_t view(args.view);
    if (view < 0)
    {
        std::cerr << "view must be > 0 timesteps" << std::endl;
        exit(1);
    }
    else if (view == 0)
    {
        view = 20; // timesteps
    }
    double view_arc = 0; // [arc] - window for reporting, from view [timesteps]

    size_t days(0);
    time_t startTime = time(0);
    double trueAnomaly_double;
    size_t number_of_orbits = 0;

    while (true)
    {
        days = (size_t)floor((double)currentTime / JulianDay);
        //std::cerr << "currentTime: " << currentTime.write() << "  day: " << days << std::endl;

        get_angle_double(radius, trueAnomaly_double);

        // remember the size of the window for reporting
        if (view > 0)
        {
            --view;
            view_arc = trueAnomaly_double;
            atPerihel = true;
        }
        else
        {
            atPerihel = abs(trueAnomaly_double) < view_arc;
        }

        if (atPerihel)
        {
            if (numberOfPointsAtPerihel == 0)
            {
                std::cerr << "orbit " << number_of_orbits << std::endl;
                std::cout << "orbit;time_s;true_anomaly_deg;distance_m;delta_radius_x_m;radius_y_m;"
                        "velocity_x_m/s;velocity_y_m/"
                        "s;angle_velocity_deg;delta_abs_radius_m;duration_light_s"
                     << std::endl;
                ++number_of_orbits;
            }

            radius_x = radius.x();
            radius_y = radius.y();
            velocity_x = velocity.x();
            velocity_y = velocity.y();
            absRadius = radius.abs();
            durationOfLightSunMercury = absRadius / VelocityOfLight;
            get_angle(radius, trueAnomaly);
            get_angle(velocity, angleOfVelocity);
            distance_to_Perihelion(radius, distance);

            std::cout << real(number_of_orbits).write("%5.0Rf; ") << currentTime.write("%.3Rf") << "; "
                 << (trueAnomaly * Degrees).write() << "; " << distance.write() << "; "
                 << (radius_x - DistancePerihelion).write() << "; " << radius_y.write() << "; "
                 << velocity_x.write() << "; " << velocity_y.write() << "; "
                 << (angleOfVelocity * Degrees).write() << "; "
                 << (absRadius - DistancePerihelion).write()
                 << durationOfLightSunMercury.write("; %.1Rf") << std::endl
                 << std::flush;

            // gathering points at perihel
            //pointsAtPerihel.push_back(radius);
            numberOfPointsAtPerihel += 1;
        }
        else if (numberOfPointsAtPerihel > 0)
        {
            // computeAndPrintBestParabel(pointsAtPerihel);
            numberOfPointsAtPerihel = 0;
            std::cout << std::endl;

            if (args.number_of_orbits != 0 && number_of_orbits > args.number_of_orbits) break;
        }

        if (its_time_to_say_hello(INTERVAL_TO_SAY_HELLO))
        {
            std::cerr << "orbit " << number_of_orbits << " " << get_processtime_str() << " [" << days
                 << " days]; " << trueAnomaly_double * double(Degrees) << " deg;" << std::endl
                 << std::flush;
        }

        // F = G*M*m/r2 = m*a ==> a = -r * GM/abs(r)3

        if (args.method == 3)
        {
            // the acceleration used is the mean between startpoint and endpoint

            method3(radius, velocity, timeStep, acceleration);
            changes(acceleration, timeStep, velocity, changeRadius, changeVelocity);
        }
        else if (args.method == 2)
        {
            // radius is computes with the next item of the taylor series

            get_accel(radius, acceleration);
            changes(acceleration, timeStep, velocity, changeRadius, changeVelocity);
        }
        else if (args.method == 1)
        {
            // linear simulation in timesteps

            get_accel(radius, acceleration);
            changeVelocity = acceleration;
            changeVelocity *= timeStep;
            changeRadius = velocity;
            changeRadius *= timeStep;
        }

        radius += changeRadius;
        velocity += changeVelocity;

        currentTime += timeStep;
    }

    return 0;
}
