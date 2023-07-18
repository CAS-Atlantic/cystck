try:
    from setuptools import setup, Extension
except ImportError:
    from distutils.core import setup, Extension

from glob import glob
from os import environ, pathsep

CLASSIFIERS = list(filter(None, map(str.strip,
"""
Development Status :: 5 - Production/Stable
Intended Audience :: Developers
License :: OSI Approved :: BSD License
Programming Language :: C
Programming Language :: Python :: 2.6
Programming Language :: Python :: 2.7
Programming Language :: Python :: 3
Programming Language :: Python :: 3.2
Programming Language :: Python :: 3.4
""".splitlines())))
dconv_source_files = glob("./deps/double-conversion/double-conversion/*.cc")
dconv_source_files.append("./lib/dconv_wrapper.cc")

module1 = Extension(
    "ujson_cystck",
    sources=dconv_source_files
    + [
        "./python/ujson.c",
        "./python/objToJSON.c",
        "./python/JSONtoObj.c",
        "./lib/ultrajsonenc.c",
        "./lib/ultrajsondec.c",
        "../src/stack.c",
        "../src/Cystck_module.c",
        "../src/args.c",
        "../src/memory.c",
        "../src/tree.c",
        "../src/Cystck_method.c",
        "../src/Type.c"
    ],
     include_dirs = ['./python', './lib', './deps/double-conversion/double-conversion'],
     extra_compile_args = ['-D_GNU_SOURCE'],
     extra_link_args = ['-lstdc++', '-lm']
)



setup(
    name = 'ujson-cystck',
    version ='1.0',
    description = "Ultra fast JSON encoder and decoder for Python",
    ext_modules = [module1]
)