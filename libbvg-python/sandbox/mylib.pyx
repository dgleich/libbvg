from libc.stdlib cimport atoi
from libc.stdlib cimport atof
from libc.math cimport sin

cdef int toint(char* s):
    assert s is not NULL
    return atoi(s)

cdef float tofloat(char* s):
    assert s is not NULL
    return atof(s)

cdef double fsin(double x):
    return sin(x*x)

def fib(n):
    a, b = 0, 1
    while b < n:
        print b,
        a, b = b, a + b
    print ''

def s2int(s):
    return toint(s)

def s2float(s):
    return tofloat(s)

def sin2(n):
    print fsin(n)
