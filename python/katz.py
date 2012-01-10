#!/usr/bin/python
import libbvg as bvg
import sys
import numpy as np

#def Ax(G):
#    for v in G:
#        src = v.curr

## new an object with graph loaded
data = sys.argv[1]
G1 = bvg.BVGraph(data, 0)  # load with offset step = 0 (iteration)

## set alpha
alpha = 0.85
tol = 0.000001

## get size
n = G1.nverts

## set initial vector
x = np.zeros((n, 1))

## set identity matrix and e vector
I = np.eye(n)
e = np.ones((n,1))

## initialize A matrix
A = np.zeros((n,n))

## set transition matrix
for v in G1.vertices():
    src = v.curr
    deg = v.degree
    for neigh in v:
        dst = neigh
        A[dst, src] = float(1)     # transpose matrix

print np.linalg.norm(A[:,2])

### answer (compute score by inverse)
#M = I - alpha*A
#x = np.dot(np.linalg.inv(M), e)

#x = np.linalg.solve(M,e)

#D,V = np.linalg.eig(alpha*A)
#print D.max()  # ~12

#for xi in x:
#    print xi

sys.exit()

### Katz
## cannot use Jacobi because spectral radius is ~12

### PageRank test (Jacobi)
for i in xrange(0, 1000):
    x_new = e + alpha*np.dot(A, x)  # or x_new = (1-alpha)*e + alpha*np.dot(A,x)
    if np.linalg.norm(x_new - x) < tol:
        break
    x = x_new

for xi in x:
    print xi

print "Completed computation in ", i+1, " steps."

sys.exit()

#A = A.tocsr()
## set (I-alphaA^T)x = alphaA^Te as Mx=f, and perform Richardson
M = I - alpha*A
f = (1-alpha)*e #alpha*np.dot(A, e)
r = f - np.dot(M,x)
for i in xrange(0,1000):
    x = x + r
    r = f - np.dot(M,x)
    for ri in r:
        print ri

#    if r[1] < 0.000001:
#        break
    s = raw_input()

