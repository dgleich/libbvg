from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext
import numpy

ext_modules = [
    Extension("libbvg",
              ["libbvg.pyx"],
              include_dirs=["../include/", numpy.get_include()],
              library_dirs=["../"],
              libraries=["bvg"])]

setup (
    name = 'Python interface for Libbvg',
    cmdclass = {'build_ext': build_ext},
    ext_modules = ext_modules
)
