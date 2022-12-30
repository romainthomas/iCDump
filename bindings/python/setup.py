import os
import sys
import platform
import subprocess
import setuptools
from pathlib import Path
from shutil import copy2

from pkg_resources import get_distribution
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

CURRENT_DIR  = Path(__file__).parent
SOURCE_DIR   = CURRENT_DIR / ".." / ".."
PACKAGE_NAME = "icdump"

_CURRENT_VERSION = "1.1.0"

def report(*args):
    print(*args)

class Distribution(setuptools.Distribution):
    global_options = setuptools.Distribution.global_options + [
        ('ninja', None, 'Use Ninja as build system'),
        ('llvm-dir=', None, 'Path to the LLVM install directory'),
        ('lief-dir=', None, 'Path to the LIEF install directory'),
        ('osx-arch=', None, 'Architecture when cross-compiling for OSX'),
    ]

    def __init__(self, attrs=None):
        self.ninja = False
        self.llvm_dir = None
        self.lief_dir = None
        self.osx_arch = None

        super().__init__(attrs)


class Module(Extension):
    def __init__(self, name, *args, **kwargs):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = SOURCE_DIR


class BuildLibrary(build_ext):
    def run(self):
        try:
            subprocess.check_output(['cmake', '--version'])
        except OSError as os_error:
            raise RuntimeError("CMake must be installed to build the following extensions: " +
                               ", ".join(e.name for e in self.extensions)) from os_error

        if self.distribution.llvm_dir is None:
            report("Please provide the LLVM install directory: '--llvm-dir='")
            sys.exit(1)

        if self.distribution.lief_dir is None:
            report("Please provide the LIEF install directory: '--lief-dir='")
            sys.exit(1)

        for ext in self.extensions:
            self.build_extension(ext)
        self.copy_extensions_to_source()


    @staticmethod
    def has_ninja():
        try:
            subprocess.check_call(['ninja', '--version'])
            return True
        except subprocess.CalledProcessError:
            return False


    def build_extension(self, ext):
        cmake_args = []

        build_temp = Path(self.build_temp).absolute()
        build_temp.mkdir(parents=True, exist_ok=True)

        cmake_library_output_directory = build_temp.parent.absolute()
        cfg                            = 'RelWithDebInfo' if self.debug else 'Release'

        # Ninja ?
        build_with_ninja = False
        if self.has_ninja() and self.distribution.ninja:
            build_with_ninja = True

        if build_with_ninja:
            cmake_args += ["-G", "Ninja"]

        clang_dir = Path(self.distribution.llvm_dir) / ".." / "clang"
        cmake_args += [
            f'-DLIEF_DIR={self.distribution.lief_dir}',
            f'-DLLVM_DIR={self.distribution.llvm_dir}',
            f'-DClang_DIR={clang_dir.as_posix()}',
            '-DICDUMP_LLVM=ON',
            '-DICDUMP_PYTHON_BINDINGS=ON',
            f'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={cmake_library_output_directory}',
            f'-DPython_EXECUTABLE={sys.executable}',
            f'-DCMAKE_BUILD_TYPE={cfg}',
        ]

        if self.distribution.osx_arch is not None:
            cmake_args += [f'-DCMAKE_OSX_ARCHITECTURES={self.distribution.osx_arch}']

        env = os.environ
        cxxflags = os.getenv("CXXFLAGS", None)
        cflags   = os.getenv("CFLAGS", None)

        ccompiler   = os.getenv("CC", None)
        cxxcompiler = os.getenv("CXX", None)

        if cxxflags is not None:
            cmake_args += [
                f'-DCMAKE_CXX_FLAGS={cxxflags}',
            ]

        if cflags is not None:
            cmake_args += [
                f'-DCMAKE_C_FLAGS={cflags}',
            ]

        if ccompiler is not None:
            cmake_args += [
                f'-DCMAKE_C_COMPILER={ccompiler}',
            ]

        if cxxcompiler is not None:
            cmake_args += [
                f'-DCMAKE_CXX_COMPILER={cxxcompiler}',
            ]

        report(f"Platform: {platform.system()}")
        report("Wheel library: {}".format(self.get_ext_fullname(ext.name)))

        # 1. Configure
        configure_cmd = ['cmake', ext.sourcedir.absolute().as_posix()] + cmake_args
        report(" ".join(configure_cmd))
        subprocess.check_call(configure_cmd, cwd=self.build_temp, env=env)

        try:
            subprocess.check_call(['ninja'], cwd=self.build_temp, env=env)
        except subprocess.CalledProcessError as e:
            sys.exit(1)

        py_lib_dst  = Path(self.build_lib) / self.get_ext_filename(self.get_ext_fullname(ext.name))
        libsuffix = py_lib_dst.name.split(".")[-1]

        py_lib_path = cmake_library_output_directory / f"{PACKAGE_NAME}.{libsuffix}"

        Path(self.build_lib).mkdir(exist_ok=True)

        report(f"Copying {py_lib_path} into {py_lib_dst}")
        if not self.dry_run:
            copy2(py_lib_path, py_lib_dst)


# From setuptools-git-version
command       = 'git describe --tags --long --dirty'
git_branch    = 'git rev-parse --abbrev-ref HEAD'
is_tagged_cmd = 'git tag --list --points-at=HEAD'
fmt_dev       = '{tag}.dev0'
fmt_tagged    = '{tag}'

def get_branch():
    try:
        return subprocess.check_output(git_branch.split()).decode('utf-8').strip()
    except Exception:
        return None

def format_version(version: str, fmt: str = fmt_dev, is_dev: bool = False):
    branch = get_branch()
    if branch is not None and branch.startswith("release-"):
        _, version = branch.split("release-")
        return version

    parts = version.split('-')
    assert len(parts) in (3, 4)
    dirty = len(parts) == 4
    tag, count, sha = parts[:3]
    MA, MI, PA = map(int, tag.split(".")) # 0.9.0 -> (0, 9, 0)

    if is_dev:
        tag = f"{MA}.{MI + 1}.{0}"

    if count == '0' and not dirty:
        return tag
    return fmt.format(tag=tag, gitsha=sha.lstrip('g'))


def get_git_version(is_tagged: bool) -> str:
    git_version = subprocess.check_output(command.split()).decode('utf-8').strip()
    if is_tagged:
        return format_version(version=git_version, fmt=fmt_tagged)
    return format_version(version=git_version, fmt=fmt_dev, is_dev=True)

def check_if_tagged() -> bool:
    output = subprocess.check_output(is_tagged_cmd.split()).decode('utf-8').strip()
    return output != ""


def get_pkg_info_version():
    pkg = get_distribution(PACKAGE_NAME)
    return pkg.version

def get_version() -> str:
    if (SOURCE_DIR / ".git").is_dir():
        is_tagged = False
        try:
            is_tagged = check_if_tagged()
        except Exception:
            is_tagged = False

        try:
            return get_git_version(is_tagged)
        except Exception:
            pass

    if (CURRENT_DIR / f"{PACKAGE_NAME}.egg-info" / "PKG-INFO").is_dir():
        return get_pkg_info_version()

    return _CURRENT_VERSION

version = get_version()

report(f"{PACKAGE_NAME} version is {version}")

cmdclass = {
    'build_ext': BuildLibrary,
}

setup(
    distclass=Distribution,
    ext_modules=[Module(PACKAGE_NAME)],
    cmdclass=cmdclass,
    version=version,
)
