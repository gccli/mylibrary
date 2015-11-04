#! /usr/bin/env python

import numpy as np
import numpy.linalg
from pprint import pprint as pp
np.set_printoptions(2)

#x = np.random.randint(1, 9, size=3)

x = np.array([2,1,3])
y = np.array([1,2,3])
z = np.array([2.1, 1.2, 1.5])
print 'x:',x,'y:',y,'z:',z
print 'dot(x,y):', np.dot(x,y)
print 'vdot(x,y):', np.vdot(x,y)
print 'inner(x,y):', np.inner(x,y)
X = np.outer(x,y)
print 'X = outer(x,y):'
pp(X)
print 'X**(1/2):'
pp(X**(0.5))
print 'linalg.matrix_power(X, 2):'
pp(np.linalg.matrix_power(X, 2))

#
print 'linalg.eig(X):'
la, ev = np.linalg.eig(X)
print '  eigenvalue:', la
print '  eigenvector:'
pp(ev)

print 'inv:'
print ev**(-1)
print np.linalg.inv(ev)
# Ax = z, x = A\z

print '\nsolve: Ax = b, x = A\b'
print 'A^{-1} b:', np.dot(np.linalg.inv(ev), z)
x,_,rank,s = np.linalg.lstsq(ev,z)
print 'lstsq(A,b)', x
