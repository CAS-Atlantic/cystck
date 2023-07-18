
**Running the Benchmarks**

The directory ```benchmarks/``` contains all benchmarks in the evaluation section of the article. The following sections contain instructions for running each benchmark for the respective APIs.

***KiwiSolver cytsck***
```
sudo python3 -m pip uninstall kiwisolvercystck
python3 -m pip uninstall kiwisolvercystck
pip uninstall kiwisolvercystck
sudo pip uninstall kiwisolvercystck
cd kiwi-cystck
sudo python3 setup.py install
sudo python3 -m scalene --cpu --gpu --memory --cli benchmarks/enaml_like_benchmark.py 
```

***KiwiSolver hpy***
```
sudo python3 -m pip uninstall kiwisolver

python3 -m pip uninstall kiwisolver

pip uninstall kiwisolver

sudo pip uninstall kiwisolver

cd kiwi-hpy

sudo python3 setup.py install

sudo python3 -m scalene --cpu --gpu --memory --cli benchmarks/enaml_like_benchmark.py 
```
***KiwiSolver c api***
```
sudo python3 -m pip uninstall kiwisolver

python3 -m pip uninstall kiwisolver

pip uninstall kiwisolver

sudo pip uninstall kiwisolver

python3 -m pip install kiwisolver

sudo python3 -m scalene --cpu --gpu --memory --cli kiwi-capi/enaml_like_benchmark.py  
```

***ultrajson cystck***
```
sudo python3 -m pip uninstall ujson_cystck

python3 -m pip uninstall ujson_cystck

pip uninstall ujson_cystck

sudo pip uninstall ujson_cystck

cd ultrajson-cystck

sudo python3 setup.py install


sudo python3 -m scalene --cpu --gpu --memory --cli tests/Simple_benchmark.py
```

***ultrajson hpy***
```
sudo python3 -m pip uninstall ujson_cystck

python3 -m pip uninstall ujson_cystck

pip uninstall ujson_hpy

sudo pip uninstall ujson_hpy

cd ultrajson-hpy

sudo python3 setup.py install


sudo python3 -m scalene --cpu --gpu --memory --cli benchmark/main.py
```

***ultrajson c api***
```
sudo python3 -m pip uninstall ujson

python3 -m pip uninstall ujson

pip uninstall ujson

sudo pip uninstall ujson

sudo python3 -m pip install ujson


sudo python3 -m scalene --cpu --gpu --memory --cli ultrajson-capi/bench.py
```


***matplotlib***
```
sudo python3 -m pip uninstall matplotlib

python3 -m pip uninstall matplotlib

pip uninstall matplotlib

sudo pip uninstall matplotlib

cd matplotlib-cystck

sudo python3 -m pip uninstall numpy

python3 -m pip uninstall numpy

pip uninstall numpy

sudo pip uninstall numpy

sudo python3 -m pip install numpy

sudo pip install .


sudo python3 -m scalene --cpu --gpu --memory --profile-all  --cli tools/run_examples.py
```

***matplotlib hpy***
```
sudo python3 -m pip uninstall matplotlib

python3 -m pip uninstall matplotlib

pip uninstall matplotlib

sudo pip uninstall matplotlib

cd matplotlib-hpy

sudo python3 -m pip uninstall numpy

python3 -m pip uninstall numpy

pip uninstall numpy

sudo pip uninstall numpy

sudo python3 -m pip install numpy

sudo pip install .


python3 -m scalene --cpu --gpu --memory --profile-all  --cli tools/run_examples.py
```

***matplotlib c api***
```
sudo python3 -m pip uninstall matplotlib

python3 -m pip uninstall matplotlib

pip uninstall matplotlib

sudo pip uninstall matplotlib

sudo python3 -m pip uninstall numpy

python3 -m pip uninstall numpy

pip uninstall numpy

sudo pip uninstall numpy

sudo python3 -m pip install numpy

sudo python3 -m pip install matplotlib


python3 -m scalene --cpu --gpu --memory --profile-all  --cli matplotlib-hpy/tools/run_examples.py
```

***numpy***
```
sudo python3 -m pip uninstall numpy

python3 -m pip uninstall numpy

pip uninstall numpy

sudo pip uninstall numpy

cd numpy-cystck

sudo python3 setup.py install

sudo python3 -m pip install asv
sudo python3 -m pip install virtualenv


sudo python3 -m scalene --cpu --gpu --memory --profile-all  --cli runtests.py --bench bench_app
```

***numpy hpy***
```
sudo python3 -m pip uninstall numpy

python3 -m pip uninstall numpy

pip uninstall numpy

sudo pip uninstall numpy

cd numpy-hpy

sudo python3 setup.py install

sudo python3 -m pip install asv
sudo python3 -m pip install virtualenv


sudo python3 -m scalene --cpu --gpu --memory --profile-all  --cli runtests.py --bench bench_app
```

***numpy c api***
```
sudo python3 -m pip uninstall numpy

python3 -m pip uninstall numpy

pip uninstall numpy

sudo pip uninstall numpy


sudo python3 -m pip install numpy

sudo python3 -m pip install asv

sudo python3 -m pip install virtualenv

cd numpy-cystck

sudo python3 -m scalene --cpu --gpu --memory --profile-all  --cli runtests.py --bench bench_app
```

***piconumpy***
```
sudo python3 -m pip uninstall piconumpy

python3 -m pip uninstall piconumpy

pip uninstall piconumpy

sudo pip uninstall piconumpy

cd piconumpy-cystck

sudo python3 setup.py install

sudo python3 -m scalene --cpu --gpu --memory --cli bench/bench_cpy_vs_hpy.py
```

***piconumpy hpy***
```
sudo python3 -m pip uninstall piconumpy

python3 -m pip uninstall piconumpy

pip uninstall piconumpy

sudo pip uninstall piconumpy

cd piconumpy-cystck
sudo python3 setup.py install

cd ..

sudo python3 -m scalene --cpu --gpu --memory --cli piconumpy-hpy/bench_cpy_vs_hpy.py
```
***piconumpy c api***
```
sudo python3 -m pip uninstall piconumpy

python3 -m pip uninstall piconumpy

pip uninstall piconumpy

sudo pip uninstall piconumpy

cd piconumpy-cystck

sudo python3 setup.py install

cd ..

sudo python3 -m scalene --cpu --gpu --memory --cli piconumpy-capi/bench_cpy_vs_hpy.py
```

**Migration**

For the migration section, there is a Python script ```Migration/Migration_Script.py``` for converting extensions written using the  Python C API to use the CyStck API.

To convert `module.c` file, use:

```python3 Migration_Script.py module.c```

To upgrade all C and C++ files (``.c``, ``.h``, ``.cc``, ``.cpp``, ``.cxx`` and
``.hpp`` files) in the ``directory/`` directory, use:

```python3 Migration_Script.py directory/```

If a file is modified, a copy of the original file
is created with the ``.old`` suffix.