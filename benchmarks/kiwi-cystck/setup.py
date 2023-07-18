# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2022, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
import os
import sys
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

# Before releasing the version needs to be updated in kiwi/version.h, if the changes
# are not limited to the solver.

# Use the env var KIWI_DISABLE_FH4 to disable linking against VCRUNTIME140_1.dll


ext_modules = [
    Extension(
        "kiwisolvercystck",
        [
            "../src/stack.c",
            "../src/args.c",
            "../src/memory.c",
            "../src/tree.c",
            "../src/Cystck_method.c",
            "../src/Type.c",
            "../src/Cystck_module.c",
            "py/src/kiwisolver.cpp",
            "py/src/constraint.cpp",
            "py/src/expression.cpp",
            "py/src/solver.cpp",
            "py/src/strength.cpp",
            "py/src/term.cpp",
            "py/src/variable.cpp",
        ],
        include_dirs=["."],
        language="c++",
    ),
]


setup(
    name='kiwisolvercystck',
    ext_modules=ext_modules,
)
