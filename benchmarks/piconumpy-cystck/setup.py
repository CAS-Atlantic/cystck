from setuptools import setup, find_packages, Extension
from Cython.Build import cythonize

setup(
    name="piconumpy",
    packages=find_packages(exclude=["bench"]),
    extras_require={
        "dev": ["transonic", "numpy", "pytest", "pythran"],
        # black can't be installed with PyPy3!
        "full": ["black"],
    },
    # you need to manually install hpy.devel from the git repo for now
    
    setup_requires=["hpy>0.0.2"],
    ext_modules=[
        Extension(
            "piconumpy._piconumpy_cpython_capi",
            ["piconumpy/_piconumpy_cpython_capi.c"],
            extra_compile_args=[
                "-Wfatal-errors",  # stop after one error (unrelated to warnings)
                "-Werror",  # turn warnings into errors (all, for now)
            ],
        ),
        *cythonize("piconumpy/_piconumpy_cython.pyx"),
        Extension(
            "piconumpy.piconumpymodule",
            ["../src/stack.c","../src/Cystck_module.c","../src/args.c","../src/Type.c",
            "../src/memory.c","../src/tree.c","../src/Cystck_method.c","./piconumpy/_piconumpy_cystck.c"],
        )
    ],
    hpy_ext_modules=[
        Extension(
            "piconumpy._piconumpy_hpy",
            ["piconumpy/_piconumpy_hpy.c"],
            extra_compile_args=[
                "-Wfatal-errors",
                "-Werror",
            ],
        ),
    ],
)
