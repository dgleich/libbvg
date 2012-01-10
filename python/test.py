#!/usr/bin/env python

class MyClass:
  def __init__(self, s):
    self.mysize = s

  def edges(self):
    for i in xrange(self.mysize):
      yield i

t = MyClass(5)
for j in t.edges():
    print j

