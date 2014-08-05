#!/usr/bin/env python
try:
    from setuptools import setup
    from setuptools.extension import Extension
except ImportError:
    print("Could't import setuptools. Falling back to distutils")
    from distutils.core import setup
from distutils import log
import os
import sys

# force Cython to be installed before proceeding
from setuptools.dist import Distribution
Distribution(dict(setup_requires='Cython'))

try:
    from Cython.Distutils import build_ext
except ImportError:
    print("Could not import Cython.Distutils. Install `cython` and rerun.")
    sys.exit(1)

# from http://pymssql.googlecode.com/hg/setup.py
from distutils.command.clean import clean as _clean
class clean(_clean):
    """
    Subclass clean so it removes all the Cython generated C files.
    """

    def run(self):
        _clean.run(self)
        for ext in self.distribution.ext_modules:
            cy_sources = [s for s in ext.sources if s.endswith('.pyx')]
            for cy_source in cy_sources:
                c_source = cy_source[:-3] + 'c'
                if os.path.exists(c_source):
                    log.info('removing %s', c_source)
                    os.remove(c_source)
                so_built = cy_source[:-3] + 'so'
                if os.path.exists(so_built):
                    log.info('removing %s', so_built)
                    os.remove(so_built)


ext_modules = [
    Extension("bvg",
              ["bvg.pyx"],
              include_dirs=["../include/"],
              library_dirs=["../"],
              libraries=["bvg"])]

setup (
    name = 'pylibbvg',
    version = '2.0.24',
    author = 'David F. Gleich and Wei-Yen Day and Yongyang Yu',
    cmdclass = {'build_ext': build_ext, 'clean': clean},
    ext_modules = ext_modules
)
