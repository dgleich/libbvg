# standalone python code to build the library suitable for PyPI

from distutils.core import setup
from distutils.extension import Extension

setup(
    ext_modules = 
        [Extension("bvg", 
            ["python/bvg.c",
             "src/bitfile.c",
             "src/bvgraph.c",
             "src/bvgraph_iterator.c",
             "src/bvgraph_random.c"],
             include_dirs=["include"])]
)
