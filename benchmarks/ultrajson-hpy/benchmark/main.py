import sys
import time
import json
import ujson

import ujson_hpy



CPYTHON = (sys.implementation.name == "cpython")


def benchmark(mod, fname, N):
    a = time.time()
    for i in range(N):
        with open(fname, 'rb') as f:
            for line in f:
                mod.loads(line)
    b = time.time()
    t = (b-a) * 1000.0 / N
    print('%20s: %6.2f ms per iteration [%2d iterations]' % (mod.__name__, t, N))


def main():
    N = 20
    fname = 'benchmark/2015-01-01-15.json'
    benchmark(ujson_hpy, fname, N)



if __name__ == '__main__':
    main()
