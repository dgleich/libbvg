"""
hkrelax.py
  
Written by Kyle Kloster and David F. Gleich
"""
## Estimate column c of 
## the matrix exponential
#
#	Requires an input graph G
# returns x, the column c of exp(GD^-1)

import collections
import sys
import math
import time
import bvg

def compute_N(eps):
  epsexp = eps*math.exp(1)
  error = math.exp(1)-1.
  last = 1.
  k = 0.
  while error > epsexp :
    k = k+1.
    last = last/k
    error = error - last
  if k < 1:
    return 1
  return int(k)

def compute_psis(N):
  psis = {}
  psis[N] = 1.
  for i in xrange(N-1,-1,-1):
    psis[i] = psis[i+1]/(float(i+1.))+1.
  return psis    

def compute_threshs(eps, N, psis):
  threshs = {}
  threshs[0] = (math.exp(1)*eps/float(N))/psis[0]
  for j in xrange(1, N+1):
    threshs[j] = threshs[j-1]*psis[j-1]/psis[j]
  return threshs

# Setup parameters, constants:
#
# G is the graph as a dictionary-of-sets, 
# eps is set to stopping tolerance
# N is the degree of the Taylor polynomial
# c is the column to compute

data = '/scratch2/dgleich/data/clueweb12/clueweb12'
start = time.time()
G = bvg.BVGraph(data, 1) # load with offset step = 1 (random access)
end = time.time()
print 'Loading graph takes ' + str(end-start) + 's\n'
print 'BVGraph load suceess!'
eps = 0.000917 # should be user-defined
c = 1000 # should be user-defined
start = time.time()
N = compute_N(eps)
psis = compute_psis(N)
threshs = compute_threshs(eps,N,psis)
## Initialize variables
x = {} # Store x, r as dictionaries
r = {} # initialize residual
Q = collections.deque() # initialize queue
sumresid = 0.    
r[(c,0)] = 1.
Q.append(c)
sumresid += psis[0]
## Main loop
for j in xrange(0, N):
  qsize = len(Q)
  relaxtol = threshs[j]/float(qsize)
  for qi in xrange(0, qsize):
    i = Q.popleft() # v has r[(v,j)] ...
    rij = r[(i,j)]
    if rij < relaxtol:
      continue
    # perform the relax step
    if i not in x: x[i] = 0.
    x[i] += rij
    r[(i,j)] = 0.
    sumresid -= rij*psis[j]
    succ = G.successors(i)
    if len(succ) == 0:
        continue
    update = (rij/(float(j)+1.))/len(succ)
    for u in succ:   # for neighbors of i
      next = (u, j+1)
      if j == N-1: 
          if u not in x: x[u] = 0.
          x[u] += update
      else:
          if next not in r: 
            r[next] = 0.
            Q.append(u)
          r[next] += update
          sumresid += update*psis[j+1]
    # after all neighbors u
    if sumresid < eps: break
  if len(Q) == 0: break
  if sumresid < eps: break
end = time.time()
print 'computation takes ' + str(end-start) + 's\n'
print 'x[c] = ' + str(x[c])
print 'len(x) = ' + str(len(x))
print 'len(r) = ' + str(len(r))
print '#succ = ' + str(len(G.successors(c)))