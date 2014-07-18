from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext
from distutils import log
import os

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
    version = '2.0.1',
    author = 'David F. Gleich, Wei-Yen Day and Yongyang Yu',
    url = 'https://github.com/dgleich/libbvg',
    keywords = 'python interface for decompressing bvgraph',
    cmdclass = {'build_ext': build_ext, 'clean': clean},
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
    ext_modules = ext_modules
)
