#!/usr/bin/env python
try:
    from setuptools import setup
    from setuptools.extension import Extension
except ImportError:
    print("Could't import setuptools. Falling back to distutils")
    from distutils.core import setup
from distutils import log
from distutils.command.build_clib import build_clib
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

pwd = os.path.dirname(__file__)
includes = [os.path.join(pwd, 'include')]

libcbvg = ('cbvg', {'sources': ["src/bitfile.c", "src/bvgraph.c", "src/bvgraph_iterator.c", "src/bvgraph_random.c", "src/properties.c", "src/util.c"],
                    'include_dirs': includes})

ext_modules = [
    Extension("bvg",
              ["src/bvg.pyx"],
              include_dirs=includes,
              libraries=["cbvg"])]   

setup (
    name = 'pylibbvg',
    version = '2.0.24',
    author = 'David F. Gleich, Wei-Yen Day and Yongyang Yu',
    author_email = 'dgleich@purdue.edu, yu163@purdue.edu',
    url = 'https://github.com/dgleich/libbvg',
    long_description = open('./README.rst').read(),
    description = 'A python interface for decompressing Boldi-Vigna graphs',
    keywords = 'python interface for decompressing bvgraph',
    cmdclass = {'build_clib': build_clib, 'build_ext': build_ext, 'clean': clean},
    classifiers=[
            'Development Status :: 4 - Beta',
            'Intended Audience :: Developers',
            'Programming Language :: Python',
            'Programming Language :: Python :: 2',
            'Programming Language :: Python :: 2.6',
            'Programming Language :: Python :: 2.7',
            'Programming Language :: Python :: 3',
            'Programming Language :: Python :: 3.1',
            'Programming Language :: Python :: 3.2',
            'Programming Language :: Python :: 3.3',
            'Programming Language :: Cython',
        ],
    libraries = [libcbvg],
    ext_modules = ext_modules,
    setup_requires = ['Cython'],
    install_requires = ['Cython']
)
