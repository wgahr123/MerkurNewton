#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""This script creates the executable MerkurNewton[.exe]


The used packages mpir, mpfr, and MerkurNewton are downloaded and compiled.
"""

__version__ = '0.1'
__author__ = 'Wolfgang Gahr'
__email__ = 'wgahr@phi-is-c2.de'

import os
import sys
import argparse
import traceback
import colorama
import shutil
import pathlib
import stat
import git
import re
import subprocess
import win32api
import glob
import hashlib

if float('{}.{}'.format(sys.version_info.major, sys.version_info.minor)) < 3.7:
    print("Please use Python version >= 3.7")
    exit(1)

global message
global ROOT_DIR
global BUILD_DIR
global LOGS_DIR

BUILD_SUBDIR = 'build'
LOGS_SUBDIR = 'logs'

# repos for the packages to compile
REPOS = { 'mpir-BrianGladman': { 'repo': 'https://github.com/BrianGladman/mpir.git',
                                 'branch': 'master',
                                 'subdir': ['msvc\\vs19'],
                                 'projects': ['lib_mpir_gc', 'lib_mpir_cxx'],
                                 'patches': {},
                               },
          'mpir-KevinHake': { 'repo': 'https://github.com/KevinHake/mpir.git',
                              'branch': 'master',
                              'subdir': ['build.vc15', 'build.vc16'], # copy
                              'projects': ['lib_mpir_haswell_avx', 'lib_mpir_cxx'],
                              'patches': { '.*\.vcxproj': [
                                                            ( 'prebuild haswell\\avx x64 15',
                                                              'prebuild haswell\\avx x64 16'
                                                            ),
                                                            ( 'postbuild "$(TargetPath)" 15',
                                                              'postbuild "$(TargetPath)" 16'
                                                            ),
                                                            ( 'check_config $(Platform) $(Configuration) 15</Command>',
                                                              'check_config $(Platform) $(Configuration) 16</Command>'
                                                            ),
                                                            ( '$(SolutionDir)$(IntDir)lib_speed.lib',
                                                              '$(SolutionDir)..\\lib_speed\\$(IntDir)lib_speed.lib'
                                                            ),
                                                            ( '$(OutDir)lib_speed.lib',
                                                              '$(SolutionDir)..\\lib_speed\\$(IntDir)lib_speed.lib'
                                                            ),
                                                          ],
                                         }
                            },
          'mpfr': { 'repo': 'https://github.com/BrianGladman/mpfr.git',
                    'branch': 'master',
                    'subdir': ['build.vs19'],
                    'projects': ['lib_mpfr'],
                    'patches': {},
                  },
          'MerkurNewton': { 'repo': 'https://github.com/wgahr123/MerkurNewton.git',
                    'branch': 'master',
                    'subdir': ['.'],
                    'projects': ['MerkurNewton'],
                    'patches': { '.*\.vcxproj': [ ('BUILDWIN', '..\\build'),
                                                ],
                               },
                  },
        }

# scripts to set the compile environment
ENVSCRIPTS = ['VsDevCmd.bat', 'vcvarsall.bat']
ENVSCRIPT_ARGS = { 'VsDevCmd.bat': '-arch=amd64 -no_logo',
                   'vcvarsall.bat': 'x64',
                 }

class Message:
    """
    Class for different kinds of messages.

    Messages are printed with colors,
    and are extended til window width.
    """
    def __init__(self, options):
        self.options = options
        self.errorcount = 0
        self.colors = { # type_ : (background_color, foreground_color)
                        # color := ['BRIGHT+']+name
                        # name := 'RED', 'BLUE', etc
                        'INFO': ('WHITE', 'GREEN'),
                        'VERBOSE': ('BRIGHT+BLACK', 'GREEN'),
                        'DEBUG': ('BRIGHT+BLACK', 'GREEN'),
                        'WARNING': ('BRIGHT+WHITE', 'RED'),
                        'ERROR': ('RED', 'BRIGHT+WHITE'),
                        'ABORT': ('RED', 'BRIGHT+WHITE'),
                      }
        colorama.init()
        if self.options.testcolors:
            self.test()
            exit(0)

    def expand_color(self, msg, type_, inverse):
        color = ''
        #
        bg = self.colors[type_][0]
        if bg.startswith('BRIGHT+'):
            color += colorama.Style.BRIGHT
            bg = bg[len('BRIGHT+'):]
        else:
            color += colorama.Style.NORMAL
        if inverse:
            color += colorama.Fore.__dict__[bg]
        else:
            color += colorama.Back.__dict__[bg]
        #
        fg = self.colors[type_][1]
        if fg.startswith('BRIGHT+'):
            color += colorama.Style.BRIGHT
            fg = fg[len('BRIGHT+'):]
        else:
            color += colorama.Style.NORMAL
        if inverse:
            color += colorama.Back.__dict__[fg]
        else:
            color += colorama.Fore.__dict__[fg]
        #
        return color+msg+colorama.Style.RESET_ALL

    def expand_msg(self, text, type_, inverse):
        # expand text to window width
        (columns, lines) = shutil.get_terminal_size()
        text += ' '*(columns - len(text) - 1)
        # expand colors
        return self.expand_color(text, type_, inverse)

    def message(self, msg, type_, inverse=False):
        if type(msg) is not list:
            msg = msg.splitlines()
        for line in msg:
            if self.options.lineno:
                lineno = traceback.extract_stack()[-3].lineno
                text = "[{}.{:03d}] {}".format(type_, lineno, line)
            else:
                text = "[{}] {}".format(type_, line)
            if self.options.nocolor:
                print(text)
            else:
                print(self.expand_msg(text, type_, inverse))
            #sys.stdout.flush()

    def info(self, msg, inverse=False):
        self.message(msg, 'INFO', inverse)

    def verbose(self, msg):
        if self.options.verbose:
            self.message(msg, 'VERBOSE')

    def debug(self, msg):
        if self.options.debug:
            self.message(msg, 'DEBUG')

    def warning(self, msg):
        self.message(msg, 'WARNING')

    def error(self, msg):
        self.errorcount += 1
        self.message(msg, 'ERROR')

    def abort(self, msg, exit_=True):
        self.message(msg, 'ABORT')
        if exit_:
            exit(1)

    def test(self):

        self.info('info')
        self.warning('warning')
        self.verbose('verbose')
        self.debug('debug')
        self.error('error')
        self.abort('abort')
        self.info('this message is not printed')
        #
        for fg in range(0,10):
            for bg in range(0,10):
                print('\x1B[{0}m{0}\x1B[0m'.format('1;4'+str(bg)+';3'+str(fg)), end='-' )
            print()
        print("\x1B[38;5;206mHELLO\x1B0m")

def handle_rmtree_error(func, path, exc_info):
    """
    Handle error from shutil.rmtree().

    Try to delete file after changing the write permission.
    """
    if not os.access(path, os.W_OK):
       # Try to change the permision of file
       os.chmod(path, stat.S_IWUSR)
       # call the calling function again
       func(path)

def get_repository(repodesc, targetdir):
    """
    Get repository.

    Clone or update working area.
    Show current log message.
    """
    message.info('cloning/updating {} from {} branch {}'.format( targetdir,
                                                                 repodesc['repo'],
                                                                 repodesc['branch'],
                                                               )
                )

    # remove inkonsistent clone
    delete = False
    if os.path.exists(targetdir):
        if not os.path.exists(os.path.join(targetdir, '.git')):
            message.debug('deleting non-git directory')
            delete = True
        else:
            # check for unchanged origin.url
            try:
                old_url = git.Repo(targetdir).config_reader().get_value('remote "origin"', 'url')
                if old_url != repodesc['repo']:
                    message.debug('deleting git directory with different remote')
                    delete = True
            except:
                message.debug('deleting destroyed git directory')
                delete = True

    if delete:
        message.verbose('cleaning {}'.format(targetdir))
        shutil.rmtree(targetdir, onerror=handle_rmtree_error)
        if os.path.exists(targetdir):
            message.abort('could not delete old {}'.format(targetdir))

    # clone or update
    if not os.path.exists(targetdir):
        message.verbose('cloning {} to {}'.format(repodesc['repo'], targetdir))
        os.makedirs(targetdir)
        repo = git.Repo.clone_from( repodesc['repo'],
                                    targetdir,
                                    branch=repodesc['branch'],
                                  )
    else:
        message.verbose('updating {}'.format(targetdir))
        repo = git.Repo(targetdir)
        repo.git.checkout(repodesc['branch'])
        repo.git.pull()

def apply_patches(patches, patchdir):
    """
    Apply patches to files in patchdir.
    """
    message.verbose('apply patches in {}'.format(patchdir))

    number_of_applied_patches = 0
    for selection in patches:
        message.debug('apply patch(es) to filenames {}'.format(selection))

        for root, dirs, files in os.walk(patchdir):
            for filename in files:
                if re.fullmatch(selection, filename):
                    fullname = os.path.join(root, filename)
                    message.debug(fullname)
                    with open(fullname, 'r', encoding='utf-8') as fh:
                        lines = fh.readlines()

                    outlines = []
                    lineno = 0
                    for line in lines:
                        lineno += 1
                        patchno = 0
                        for patch in patches[selection]:
                            patchno += 1
                            if patch[0] in line:
                                number_of_applied_patches += 1
                                # only if not patched already
                                if patch[1] not in line:
                                    message.debug('patching {}:{:03d} from: {}'.format(fullname, lineno, patch[0]))
                                    message.debug('patching {}:{:03d}   to: {}'.format(fullname, lineno, patch[1]))
                                    line = line.replace(patch[0], patch[1])
                        outlines.extend( [line] )

                    with open(fullname, 'w+', encoding='utf-8') as fh:
                        fh.writelines(outlines)

    message.verbose('.. {} patch(es) applied'.format(number_of_applied_patches))

def find_file(filename, rootdir):
    """
    Find all paths for filename below rootdir.
    Return path-list.
    """
    message.debug('searching {} below {}'.format(filename, rootdir))
    paths = []
    for root, dirs, files in os.walk(rootdir):
        if filename.lower() in [f.lower() for f in files]:
            path = os.path.join(root, filename)
            message.debug('found {}'.format(path))
            paths.extend( [path] )
            #print(os.system('cmd /c ""{}" -version"'.format(path)))
    return paths

def get_version_number(filename):
    """
    Get the file version as string.
    From https://stackoverflow.com/questions/580924/python-windows-file-version-attribute

    When there is no version return "0.0.0.0"
    """
    try:
        info = win32api.GetFileVersionInfo(filename, "\\")
        ms = info['FileVersionMS']
        ls = info['FileVersionLS']
        return (win32api.HIWORD(ms), win32api.LOWORD(ms), win32api.HIWORD(ls), win32api.LOWORD(ls))
    except:
        return (0,0,0,0)

def compare_version_numbers(version1, version2):
    """
    Compare two version numbers.

    Return -1 if version1 < version2, 0 if version1 == version2, +1 if version1 > version2
    Version :== (k,l,m,n)
    k, l, m, n := int
    """
    if version1 is None:
        if version2 is None:
            return 0
        else:
            return -1
    elif version2 is None:
        return +1
    else:
        for i in range(0, 4):
            if version1[i] > version2[i]:
                return +1
            elif version1[i] < version2[i]:
                return -1
        return 0

def version_to_string(version):
    """
    Create a version string from tuple.
    """
    return '{}.{}.{}.{}'.format(version[0], version[1], version[2], version[3])

def find_msbuild():
    """
    Find the msbuild.exe to use.
    """
    # find versions of msbuild.exe
    paths = find_file('msbuild.exe', os.environ['ProgramFiles(x86)'])

    # select: only amd64
    paths = [ path for path in paths if '\\amd64\\' in path ]
    # select: newest version of msbuild.exe
    msbuild = None
    latest_version = None
    for path in paths:
        version = get_version_number(path)
        #print(version)
        if compare_version_numbers(version, latest_version) > 0:
            msbuild = path
            latest_version = version
    if msbuild is None:
        message.abort('could not find any msbuild.exe for amd64')
    return (msbuild, version_to_string(latest_version))

def run(command, cwd='.', env=os.environ, stdout=sys.stdout, startupinfo=None):
    """
    Run a command by subprocess.
    """
    if startupinfo is None:
        result = subprocess.run(command, cwd=cwd, env=env, stdout=stdout)
    else:
        result = subprocess.run(command, cwd=cwd, env=env, stdout=stdout, startupinfo=startupinfo)
    if result.stderr is not None:
        for line in result.stderr.splitlines():
            message.warning(line)
    return result.returncode

def get_installpath_from_msbuild(path):
    """
    From path deeper than INSTALLPATH derive the INSTALLPATH.
    """
    for part in ['\\BuildTools\\', '\\Community\\', '0\\']:
        if part in path:
            return path.split(part)[0]+part[:-1]    # strip trailing '\\'
    message.abort('could not find INSTALLPATH')

def get_installpath_visual_studio():
    """
    Get the INSTALLPATH of the latest version of Visual Studio which offers msbuild.
    """
    # find vswhere and ask for the INSTALLPATH
    path = os.path.join(os.environ['ProgramFiles(x86)'], 'Microsoft Visual Studio', 'Installer', 'vswhere.exe')
    path_args = [path]
    path_args.extend( '-latest -requires Microsoft.Component.MSBuild -property installationPath'.split() )
    if os.path.isfile(path):
        result = subprocess.run(path_args, capture_output=True, text=True)
        if result.returncode == 0:
            return result.stdout.strip()

    # if not successful: search for the newest msbuild below ProgramFiles(x86)
    try:
        (msbuild, version) = find_msbuild()
        return get_installpath_from_msbuild(msbuild)
    except:
        message.abort('could not find INSTALLPATH of Visual Studio')

def prepare_build_environment(clean=False):
    """
    Prepare build environment.

    Return dictionary with environment.
    """
    global ROOT_DIR
    global BUILD_DIR
    global LOGS_DIR

    # create (new) directories
    ROOT_DIR = os.path.dirname(os.path.abspath(__file__))
    BUILD_DIR = os.path.join(ROOT_DIR, BUILD_SUBDIR)
    LOGS_DIR = os.path.join(ROOT_DIR, LOGS_SUBDIR)

    if clean:
        clean_all()

    for dir in [BUILD_DIR, LOGS_DIR]:
        if os.path.exists(dir):
            if os.path.isdir(dir):
                message.verbose('using existing directory {}'.format(dir))
            else:
                message.abort('directory {} is blocked by existing file'.format(dir))
        else:
            message.verbose('creating directory {}'.format(dir))
            os.makedirs(dir)

    INSTALLPATH = get_installpath_visual_studio()
    message.verbose('using INSTALLPATH: {}'.format(INSTALLPATH))

    # find script to set environment
    dirname = None
    for scriptname in ENVSCRIPTS:
        paths = find_file(scriptname, INSTALLPATH)
        if len(paths) == 0:
            paths = find_file(scriptname, os.path.dirname(INSTALLPATH))
        if len(paths) > 0:
            dirname = os.path.dirname(paths[0])
            break
    if dirname is None:
        message.abort('could not find script to set the environment for the compiler')
    message.verbose('using environment set by {}'
                    .format(os.path.join(dirname, scriptname))
                   )

    # find available toolsets and choose the newest one
    toolsets_to_check = ['9.0', '14.0']
    msvc_path = os.path.join(INSTALLPATH, 'VC', 'Tools', 'MSVC')
    if os.path.isdir(msvc_path):
        toolsets_to_check.extend( os.listdir(msvc_path) )
    # check
    usable_toolsets = []
    for toolset in toolsets_to_check:
        message.debug('checking toolset {}'.format(toolset))
        command = '@echo off & {} {} {} & cl -?'.format( scriptname,
                                                         ENVSCRIPT_ARGS[scriptname],
                                                         '-vcvars_ver={}'.format(toolset)
                                                       )
        command = ["cmd.exe", "/C", command]
        result = subprocess.run(command, cwd=dirname, capture_output=True)
        message.debug(result.stderr.splitlines())
        if result.returncode == 0:
            usable_toolsets.extend([toolset])
    #
    message.verbose('found toolsets {}'.format(usable_toolsets))
    toolset = usable_toolsets[-1]
    message.verbose('using toolset {}'.format(toolset))

    # create a dict with the environment to compile
    command = '{} {} {}'.format( scriptname,
                                 ENVSCRIPT_ARGS[scriptname],
                                 '-vcvars_ver={}'.format(toolset)
                               )
    message.verbose('get environmanet: {}'.format(command))
    command = ["cmd.exe", "/C", "@echo off & {} & set".format(command)]
    result = subprocess.run(command, cwd=dirname, capture_output=True, text=True)
    if result.returncode != 0:
        message.abort(result.stderr, exit_=False)
        message.abort('could not get environment for the compiler')
    compile_environ = {}
    for line in result.stdout.splitlines():
        message.debug(line)
        parts = line.split('=',1)
        compile_environ[parts[0]] = parts[1]

    # get the path of msbuild.exe
    command = ["cmd.exe", "/C", "where msbuild.exe"]
    result = subprocess.run(command, env=compile_environ, capture_output=True, text=True)
    if result.returncode != 0:
        message.abort(result.stderr, exit_=False)
        message.abort('could not get path of used msbuild.exe')
    for line in result.stdout.splitlines():
        msbuild_path = line
        break

    # get the newest installed Windows SDK Version
    windows_sdk_versions = []
    for elem in os.listdir(os.path.join(os.environ['ProgramFiles(x86)'], 'Windows Kits', '10', 'bin')):
        if elem.startswith('10.0.'):
            windows_sdk_versions.extend( [ elem ])
    if len(windows_sdk_versions) == 0:
        message.abort('could not find any installed Windows SDK Version 10.0')
    message.verbose('found Windows SDK Versions {}'.format(windows_sdk_versions))
    windows_sdk_version = sorted(windows_sdk_versions)[-1]
    message.verbose('using Windows SDK Version {}'.format(windows_sdk_version))

    compile_environ['TOOLSET'] = toolset
    compile_environ['MSBUILD_PATH'] = msbuild_path

    # additional compiler options
    # see https://github.com/MicrosoftDocs/cpp-docs/blob/master/docs/build/reference/cl-environment-variables.md
    compile_environ['CL'] = '/Zp8 /Ob2 /Oi /Ot /Og /GL /Qpar /arch:AVX2 /O2'
    compile_environ['_CL_'] = '/link'

    # compile with /MP and max CPUs, configuration x64/Release, etc
    nop = compile_environ['NUMBER_OF_PROCESSORS']
    msbuild = compile_environ['MSBUILD_PATH']
    compile_command = [ msbuild_path ]
    compile_command.extend( [ '/t:Rebuild' ] )
    compile_command.extend( [ '/nr:false', '/verbosity:d' ] )
    compile_command.extend( [ '/m:{}'.format(nop), '/p:CL_MPCount={}'.format(nop) ] )
    compile_command.extend( [ '/p:Platform=x64', '/p:Configuration=Release' ] )
    compile_command.extend( [ '/p:WindowsTargetPlatformVersion={}'.format(windows_sdk_version) ] )
    compile_command.extend( [ '/p:BuildProjectReferences=false' ] )
    compile_command.extend( [ '/p:ToolsVersion=142' ] )

    return (compile_command, compile_environ)

def compile(projects, workingdir, compile_command, compile_environ=os.environ, logfile=sys.stdout):
    """
    Compile the projects.
    """

    '''
        set "VCTargetsPath=%~1\VC"
        rem   v160\Platforms\x64\PlatformToolsets"
    set "VCTargetsPath=%INSTALLPATH%\MSBuild\Microsoft\VC\v160\"
    echo VCTargetsPath: %VCTargetsPath%
    '''
    message.debug(compile_command)
    for project in projects:
        message.info('compiling {}'.format(project))
        returncode = run( compile_command + [project],
                          cwd=workingdir,
                          env=compile_environ,
                          stdout=logfile
                        )
        if returncode != 0:
            message.abort('could not compile {}'.format(project))

def copy_results(sourcedir, target, quiet=False):
    if not quiet:
        message.info('copying includes and libs to {}'.format(os.path.join(BUILD_DIR, target)))
    includedir = os.path.join(BUILD_DIR, target, 'include')
    libdir = os.path.join(BUILD_DIR, target, 'lib')
    bindir = os.path.join(BUILD_DIR, target, 'bin')
    for filename in os.listdir(sourcedir):
        fullname = os.path.join(sourcedir, filename)
        if os.path.isfile(fullname):
            if filename.endswith('.h'):
                message.verbose('copying {}'.format(filename))
                if not os.path.isdir(includedir):
                    os.makedirs(includedir)
                shutil.copy(fullname, includedir)
            elif filename.endswith('.lib'):
                message.verbose('copying {}'.format(filename))
                if not os.path.isdir(libdir):
                    os.makedirs(libdir)
                shutil.copy(fullname, libdir)
            elif filename.endswith('.exe'):
                message.verbose('copying {}'.format(filename))
                if not os.path.isdir(bindir):
                    os.makedirs(bindir)
                shutil.copy(fullname, bindir)

def get_workingdir(repodir, subdir):
    """
    Get the workingdir from description.
    """
    if len(subdir) > 1:
        sourcedir = os.path.join(repodir, subdir[0])
        targetdir = os.path.join(repodir, subdir[1])
        if not os.path.isdir(targetdir):
            shutil.copytree(sourcedir, targetdir)
        return targetdir
    else:
        return os.path.join(repodir, subdir[0])

def get_md5(fname):
    """
    get MD5 of file.
    """
    hash_md5 = hashlib.md5()
    with open(fname, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_md5.update(chunk)
    return hash_md5.hexdigest()

def clean_all():
    """
    Clean all directories.
    """
    for dir in [BUILD_DIR, LOGS_DIR, 'mpir', 'mpfr', 'MerkurNewton']:
        if os.path.exists(dir):
            message.verbose('cleaning {}'.format(dir))
            shutil.rmtree(dir, onerror=handle_rmtree_error)
            if os.path.exists(dir):
                message.abort('could not delete old {}'.format(dir))

def build_mpir(options, compile_command, compile_environ):
    """
    Build library mpir from repository.
    """
    name = 'mpir'

    repodesc = REPOS[name+'-'+options.repo_author_mpir]
    repodir = os.path.abspath(name)
    resultdir = os.path.join(repodir, 'lib', 'x64', 'Release')

    environ = compile_environ.copy()

    with open(os.path.join(LOGS_DIR, name+'.log'), 'w+') as logfile:

        get_repository(repodesc, repodir)
        workingdir = get_workingdir(repodir, repodesc['subdir'])
        apply_patches(repodesc['patches'], workingdir)
        compile(repodesc['projects'], workingdir, compile_command, environ, logfile)
        copy_results(resultdir, name)
        logfile.flush()

        define_mpir = 'using lib_mpir ({} from {})'.format(repodesc['projects'][0], repodesc['repo'])
        with open(os.path.join(BUILD_DIR, 'using_mpir.h'), 'w+') as fh:
            fh.write(define_mpir)

        if options.tune:
            message.info('tuning (creating gmp_param.h)')

            message.verbose('prepare tuning')
            tuningdir = os.path.join(workingdir, 'mpir-tune', 'tune')

            # patches for new files
            apply_patches( {'test-config.props': [ ( '$(MPLIBS);$(OutDir)\\add-test-lib.lib',
                                                     '$(MPLIBS);..\\..\\mpir-tests\\add-test-lib\\$(IntDir)add-test-lib.lib'
                                                   ),
                                                 ]
                           },
                           workingdir
                         )

            # prepare tuning program
            returncode = run( [sys.executable, 'tune_prebuild.py'],
                              cwd=tuningdir,
                              env=environ,
                              stdout=logfile
                            )
            if returncode != 0:
                message.abort('could not prepare the tuning')
            logfile.flush()

            # compile tuning libs and exes
            tune_projects = [ os.path.join('mpir-tests', 'add-test-lib'),
                              os.path.join('mpir-tune', 'lib_speed'),
                              os.path.join('mpir-tune', 'speed'),
                              os.path.join('mpir-tune', 'try'),
                              os.path.join('mpir-tune', 'tune'),
                            ]

            returncode = run( [sys.executable, 'tune_prebuild.py'],
                              cwd=tuningdir,
                              env=environ,
                              stdout=logfile
                            )
            if returncode != 0:
                message.abort('could not prepare tuning')
            logfile.flush()

            environ['CL'] += ' /I{}'.format(os.path.join(repodir, 'tests'))
            environ['_CL_'] += ' /LIBPATH:"{}"'.format(os.path.join(BUILD_DIR, name, 'lib'))
            environ['_CL_'] += ' /LIBPATH:"{}"'.format(os.path.join(workingdir, 'mpir-tests', 'add-test-lib'))
            environ['_CL_'] += ' /LIBPATH:"{}"'.format(os.path.join(workingdir, 'mpir-tune', 'lib_speed'))

            compile(tune_projects, workingdir, compile_command, environ, logfile)
            logfile.flush()

            # tuning
            ## PS S:\Projekte\MerkurNewton> S:\Projekte\MerkurNewton\mpir\build.vc16\mpir-tune\tune\x64\Release\tune.exe -p 10000
            ## #define GCDEXT_DC_THRESHOLD             382
            ## Fatal error: new reps bad: 1464077704391.00
            ## (old reps 1330989866, unittime 1e-07, precision 1000, t[i] 1e-07)

            message.verbose('tune and write data to gmp-mparam.h')
            filename = os.path.join(workingdir, 'lib_mpir_cxx', 'x64', 'Release', 'gmp-mparam.h')
            # break s sometimes with "Fatal error: new reps bad"
            si = subprocess.STARTUPINFO()
            si.dwflags = subprocess.HIGH_PRIORITY_CLASS
            for n in range(0,5):
                with open(filename, 'w') as fh:
                    returncode = run( ['cmd.exe', '/C', os.path.join(tuningdir, 'x64', 'Release', 'tune.exe')],
                                    cwd=tuningdir,
                                    env=environ,
                                    stdout=fh,
                                    startupinfo=si
                                    )
                    logfile.flush()
                    if returncode != 0:
                        message.verbose('could not create gmp-mparam.h')
                        continue
                break
            if returncode != 0:
                message.abort('could not create gmp-mparam.h')
            message.info('successfully created gmp-mparam.h')

            #copy /Y lib_mpir_cxx\x64\Release\gmp-mparam.h lib_mpir_gc\x64\Release\gmp-mparam.h

            message.verbose('compile again')

            compile( [repodesc['projects'][0]], workingdir, compile_command, environ, logfile)
            copy_results(resultdir, name)
            logfile.flush()

    '''
        if %OPT_TESTS% EQU 0 goto END_MPIR_TESTS

            call :info2 "preparing tests"


            rem command to build tests
            set "_CMD=msbuild.exe %ARGS%"
            set "_CMD=%_CMD% /p:OutDir=%CD%\x64\Release\ /p:SolutionDir=%CD%\"
            rem dont build add-test-lib again and again
            set "_CMD=%_CMD% /p:BuildProjectReferences=false"

            call %_CMD% mpir-tests\add-test-lib\add-test-lib.vcxproj >> %LOGFILE%

            rem build in parallele
            set /A number_of_tests = 0
            for /d %%d in (mpir-tests\*) do (
                if %%d NEQ mpir-tests\add-test-lib (
                    for %%f in (%%d\*.vcxproj) do (
                        set /A number_of_tests += 1
                        echo.> %LOG_DIR%\%%~nf.running
                        start /MIN /LOW cmd /c "%_CMD% %%f > %LOG_DIR%\tests\%%~nf.log & del %LOG_DIR%\%%~nf.running"
                    )
                )
            )
            call :info2 "compiling %number_of_tests% tests"
            :wait
                ping localhost -n 2 > NUL
            if exist %LOG_DIR%\*.running goto wait
            call :info2 "tests ready (logfiles in %LOG_DIR%\tests\)"



            rem 21:56:51,78 bis 22:00:12,88 = 3:21 parallele
            rem 22:04:15,94 bis 22:08:34,03 = 4:18 sequential

            call :info2 "running tests"
            echo %YES% | python run-tests.py | findstr /V subprocess.STARTUPINFO | findstr /V success

        :END_MPIR_TESTS

        call :info2 "installing into %BUILD_DIR%\mpir"
        xcopy /S /Y ..\lib\x64\Release\*.h %BUILD_DIR%\mpir\include\
        xcopy /S /Y ..\lib\x64\Release\*.lib %BUILD_DIR%\mpir\lib\
        xcopy /S /Y ..\lib\x64\Release\*.pdb %BUILD_DIR%\mpir\lib\

        python run-speed.py | findstr /V subprocess.STARTUPINFO | findstr mul


    call :info2 "mpir installed in %BUILD_DIR%\mpir (logfile %LOGFILE%)"
    '''

def build_mpfr(options, compile_command, compile_environ):
    """
    """
    name = 'mpfr'

    repodesc = REPOS[name]
    repodir = os.path.abspath(name)
    workingdir = get_workingdir(repodir, repodesc['subdir'])

    environ = compile_environ.copy()

    addincdir = os.path.join(BUILD_DIR, 'mpir', 'include')
    addlibdir = os.path.join(BUILD_DIR, 'mpir', 'lib')

    environ['CL'] += ' /I"{}"'.format(addincdir)
    environ['_CL_'] += ' /link "mpir.lib" /LIBPATH:"{}"'.format(addlibdir)

    environ['INCLUDE'] = os.path.join(BUILD_DIR, 'mpir', 'include')
    environ['LIB'] = os.path.join(BUILD_DIR, 'mpir', 'lib')
    #compile_command.extend( [ '/p:"VCBuildAdditionalOptions= /useenv"' ] )

    with open(os.path.join(BUILD_DIR, 'using_mpfr.h'), 'w+') as fh:
        fh.write('using lib_mpfr ({} from {})'.format(repodesc['projects'][0], repodesc['repo']))

    with open(os.path.join(LOGS_DIR, name+'.log'), 'w+') as logfile:

        get_repository(repodesc, repodir)
        apply_patches(repodesc['patches'], workingdir)
        compile(repodesc['projects'], workingdir, compile_command, environ, logfile)
        copy_results(os.path.join(repodir, 'lib', 'x64', 'Release'), name)
        copy_results(os.path.join(workingdir, 'lib', 'x64', 'Release'), name, quiet=True)

def build_MerkurNewton(options, compile_command, compile_environ):
    """
    """
    name = 'MerkurNewton'

    repodesc = REPOS[name]
    repodir = os.path.abspath(name)

    # build
    with open(os.path.join(LOGS_DIR, name+'.log'), 'w+') as logfile:

        get_repository(repodesc, repodir)
        workingdir = get_workingdir(repodir, repodesc['subdir'])
        apply_patches(repodesc['patches'], workingdir)

        # get using info for mpir and mpfr
        define_mpir = None
        using_mpir = os.path.join(BUILD_DIR, 'using_mpir.h')
        if os.path.exists(using_mpir):
            with open(using_mpir, 'r') as fh:
                define_mpir = fh.read()
            message.verbose('using mpir: {}'.format(define_mpir))
            apply_patches({'MerkurNewton.cpp': [('"using lib_mpir"', '"{}"'.format(define_mpir))]}, workingdir)

        define_mpfr = None
        using_mpfr = os.path.join(BUILD_DIR, 'using_mpfr.h')
        if os.path.exists(using_mpfr):
            with open(using_mpfr, 'r') as fh:
                define_mpfr = fh.read()
            message.verbose('using mpfr: {}'.format(define_mpfr))
            apply_patches( {'MerkurNewton.cpp': [ ('"using lib_mpfr"', '"{}"'.format(define_mpfr))]}, workingdir)

        compile(repodesc['projects'], workingdir, compile_command, compile_environ, logfile)
        copy_results(os.path.join(workingdir, 'MerkurNewton', 'x64', 'Release'), name, quiet=True)

    # check
    message.info('testing MerkurNewton')

    executable = os.path.join(BUILD_DIR, name, 'bin', 'MerkurNewton.exe')
    command = [executable, '--help']
    result = subprocess.run(command)
    if result.returncode != 0:
        message.abort('could not call MerkurNewton.exe')

    testfile = os.path.join(LOGS_DIR, 'testfile.log')
    with open(testfile, 'w') as tf:
        command = [executable, '-t', '1', '-n', '1', '-m', '3']
        result = subprocess.run(command, stdout=tf)
        if result.returncode != 0:
            message.abort('could not call MerkurNewton.exe')

    # results
    # -t 1 -n 1 -m 3: => 33fa1d843d1bd1cdb320da884207801c
    # -t 5 -n 5 -m 3: => c9284eaa72596095bb2efe9f95afb8e3
    cksum = get_md5(testfile)
    if cksum == '33fa1d843d1bd1cdb320da884207801c':
        message.info('test succeeded', inverse=True)
        message.info('{} is ready to use'.format(executable), inverse=True)
    else:
        message.error('test failed (md5: {})'.format(cksum))

def parse_commandline():
    """
    Parse commandline and return (package, options).
    """
    parser = argparse.ArgumentParser(description='Create executable MerkurNewton.')

    parser.add_argument('package', choices=['all', 'mpir', 'mpfr', 'MerkurNewton'],
                        help='Choose what should be compiled.')

    parser.add_argument('-v', '--verbose', action='store_true', help='Write more messages.')
    parser.add_argument('-d', '--debug', action='store_true', help='Write debug messages.')
    parser.add_argument('-u', '--tune', action='store_true', help='Tune the package if possible.')
    parser.add_argument('-t', '--tests', action='store_true', help='Perform tests.')
    parser.add_argument('--clean', action='store_true', help='Clean old directories.')
    parser.add_argument('-l', '--lineno', action='store_true', help='Show linenumber of message call.')
    parser.add_argument('-c', '--testcolors', action='store_true', help='Show examples of colored messages.')
    parser.add_argument('-n', '--nocolor', action='store_true', help='Dont colorize messages.')
    parser.add_argument('--repo_author_mpir', choices=['BrianGladman', 'KevinHake'], default='KevinHake',
                        help='Choose from list. Default is "KevinHake".')

    args = parser.parse_args()

    return (args.package, args)

def main():
    """
    Get arguments and options from commandline.
    Prepare directories.
    Call build functions.
    """
    global message

    (package, options)= parse_commandline()

    message = Message(options)
    message.debug("package: {}, options: {}".format(package, options))

    path = os.path.dirname(os.path.abspath(__file__))
    os.chdir(path)
    message.info('working dir is {}'.format(path))

    message.info('preparing build environment')
    (compile_command, compile_environ) = prepare_build_environment(clean=options.clean)

    if package == "all" or package == "mpir":
        build_mpir(options, compile_command, compile_environ)
    if package == "all" or package == "mpfr":
        build_mpfr(options, compile_command, compile_environ)
    if package == "all" or package == "MerkurNewton":
        build_MerkurNewton(options, compile_command, compile_environ)

if __name__ == '__main__':
    exit( main() )

# vim: set fileencoding=utf-8 nu sts=4 ts=4 ai et :
