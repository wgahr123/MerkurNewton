# MerkurNewton
Just for fun the simulation of mercurys orbit.

Using the following libraries/sources:
* https://github.com/wbhart/mpir
* https://github.com/BrianGladman/mpir
* https://github.com/BrianGladman/mpfr
* https://github.com/jarro2783/cxxopts.

Please note the licenses of these packages.<br/>
My own contribution is licensed here: https://github.com/wgahr123/MerkurNewton/blob/master/LICENSE.md

## Compilation on Windows10
Python >= 3.7.5 and some installed Visual Studio or MSBuild environment is needed.

Call the python script _build_MerkurNewton.py_ to clone and compile all to create the executable _MerkurNewton.exe_.
Download the script with a right click on the following link: 
https://github.com/wgahr123/MerkurNewton/raw/master/build_MerkurNewton.py

#### Help for build_MerkurNewton.py

    PS S:\Projekte\MerkurNewton> python .\build_MerkurNewton.py --help
    usage: build_MerkurNewton.py [-h] [-v] [-d] [-u] [-t] [--clean] [-l] [-c] [-n]
                                 [--repo_author_mpir {BrianGladman,KevinHake}]
                                 [-a ACCURACY]
                                 {all,mpir,mpfr,MerkurNewton}
    
    Create executable MerkurNewton.
    
    positional arguments:
      {all,mpir,mpfr,MerkurNewton}
                            Choose what should be compiled.
    
    optional arguments:
      -h, --help            show this help message and exit
      -v, --verbose         Write more messages.
      -d, --debug           Write debug messages.
      -u, --tune            Tune the package if possible.
      -t, --tests           Perform tests.
      --clean               Clean old directories.
      -l, --lineno          Show linenumber of message call.
      -c, --testcolors      Show examples of colored messages.
      -n, --nocolor         Dont colorize messages.
      --repo_author_mpir {BrianGladman,KevinHake}
                            Choose from list. Default is "KevinHake".
      -a ACCURACY, --accuracy ACCURACY
                            Number of digits used by mpir/mpfr. Default: 200
                            digits

#### How to Use

    PS S:\Projekte\MerkurNewton> python .\build_MerkurNewton.py all
    [INFO] working dir is S:\Projekte\MerkurNewton
    [INFO] preparing build environment
    [INFO] cloning/updating S:\Projekte\MerkurNewton\mpir from https://github.com/KevinHake/mpir.git branch master
    [INFO] compiling lib_mpir_haswell_avx
    [INFO] compiling lib_mpir_cxx
    [INFO] copying includes and libs to S:\Projekte\MerkurNewton\build\mpir
    [INFO] cloning/updating S:\Projekte\MerkurNewton\mpfr from https://github.com/BrianGladman/mpfr.git branch master
    [INFO] compiling lib_mpfr
    [INFO] copying includes and libs to S:\Projekte\MerkurNewton\build\mpfr
    [INFO] cloning/updating S:\Projekte\MerkurNewton\MerkurNewton from D:\Repos\MerkurNewton.git branch master
    [INFO] compiling MerkurNewton
    [INFO] testing MerkurNewton
    compiled with Visual Studio compiler version 192428314 on Jan  6 2020 at 16:24:33
    using lib_mpir (lib_mpir_haswell_avx from https://github.com/KevinHake/mpir.git)
    using lib_mpfr (lib_mpfr from https://github.com/BrianGladman/mpfr.git)
    using mpir and mpfr with 200 digits
    using cxxopts.hpp from https://github.com/jarro2783/cxxopts

    .. script calls _MerkurNewton.exe --help_ to check if it is running:
    MerkurNewton.exe - Simulate the orbit of mercury
    Usage:
      MerkurNewton.exe [OPTION...]
    
      -h, --help                  Print help
      -t, --timestep arg          Set timestep in seconds (default: 1)
      -n, --number_of_orbits arg  Number of orbits to be computed - no limit: 0
                                  (default: 3)
      -m, --method arg            Method for simulation
                                  Method 1: steps with linear simulation
                                  Method 2: steps with quadratic simulation
                                  Method 3: steps with quadratic simulation and
                                            average of accelerations (default: 3)
      -v, --view arg              Print results in range -val..+val timesteps at
                                  the perihel (default: 30)
    
    .. script calls _MerkurNewton.exe_ with defaults to check if the results are correct:
    compiled with Visual Studio compiler version 192428314 on Jan  6 2020 at 16:24:33
    using lib_mpir (lib_mpir_haswell_avx from https://github.com/KevinHake/mpir.git)
    using lib_mpfr (lib_mpfr from https://github.com/BrianGladman/mpfr.git)
    using mpir and mpfr with 200 digits
    using cxxopts.hpp from https://github.com/jarro2783/cxxopts
    
    args.timestep: 1
    timeStep [sec]: 1.00000000000000000000
    orbit 0
    orbit 1 00:00:00.000 [0 days]; 0 deg;
    orbit 1 00:00:02.000 [8 days]; 52.2352 deg;
    orbit 1 00:00:04.000 [17 days]; 94.9302 deg;
    orbit 1 00:00:06.000 [26 days]; 128.661 deg;
    orbit 1 00:00:08.000 [35 days]; 155.219 deg;
    orbit 1 00:00:10.000 [43 days]; 179.723 deg;
    orbit 1 00:00:12.000 [52 days]; -156.294 deg;
    orbit 1 00:00:14.000 [61 days]; -129.618 deg;
    orbit 1 00:00:16.000 [69 days]; -97.9039 deg;
    orbit 1 00:00:18.000 [78 days]; -58.402 deg;
    orbit 1 00:00:20.000 [86 days]; -7.67661 deg;
    orbit 1
    [INFO] test succeeded                                                                                                                 [INFO] S:\Projekte\MerkurNewton\build\MerkurNewton\bin\MerkurNewton.exe is ready to use
    PS S:\Projekte\MerkurNewton>
