#!/usr/bin/python
from mylib import fib
from mylib import s2int
from mylib import s2float
from mylib import sin2
import sys

print 'Hello world!'

num = sys.argv[1]
print num

fib(100)
new_num = s2int(num)
print new_num

x = sys.argv[2]
x = s2float(x)
sin2(x)
#print func(num)
