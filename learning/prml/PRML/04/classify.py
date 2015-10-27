#!/usr/bin/python
# -*- coding: utf-8 -*-

import time
import numpy as np
import numpy.linalg as linalg
import numpy.matlib
import matplotlib.pyplot as plt
from sklearn import linear_model, datasets


iris = datasets.load_iris()
X = iris.data[:, :2]  # only take the first two features.
Y = iris.target
t = Y[Y < 2]   # only for two-class classification
X = X[:t.size]
mu= [X[:,0].mean(), X[:,1].mean()]
sigma = [X[:,0].std(), X[:,1].std()]
N,M = X.shape

iterations = 10

# (4.59)
def sigmoid(x):
    #x = float(x)
    ret = 1/(1 + np.exp(x))
    return ret

# Gaussian basis function
def phi(x):
    ret = np.matlib.ones((M+1, 1))
    for i in range(1, M+1):
        ret[i] = np.exp( -np.power((x[i-1]-mu[i-1]),2)/(2*np.power(sigma[i-1], 2)) )

    return ret

# (4.87) Posterior probability of class C1
def predict(x, w):
    ret = w.T * phi(x)
    return sigmoid(float(ret))

logreg = linear_model.LogisticRegression(C=1e5,solver='newton-cg')
logreg.fit(X, t)


plt.figure(1)
plt.subplot(311)
plt.scatter(X[:, 0], X[:, 1], c=t, edgecolors='k')

plt.subplot(312)
xx = np.linspace(-5, 5, 100)
plt.plot(xx, sigmoid(xx), 'k-')
plt.title('sigmoid(x)')

plt.show(block=False)

Phi = np.matlib.zeros((N, M+1))
for i in range(N):
    Phi[i] = phi(X[i]).T
print 'Φ:\n',Phi,'\n--------\n',Phi.shape

w0 = np.matlib.zeros((M+1, 1))
w0[0] = 0.02; w0[1] = 0.02
for j in range(iterations):
    R = np.matlib.eye(N, dtype=float)
    y = np.matlib.zeros((N, 1))
    z = np.matlib.zeros((N, 1))
    for i in range(N):
        y_n = predict(X[i], w0)
        R[i,i] = y_n * (1 - y_n)
        y[i] = y_n
    try:
        #print '----\n',R
        Rinv = linalg.inv(R)
        #print '----\n',Rinv,'\n========'
        z = Phi * w0 - Rinv * (y - np.mat(t).T)  # (4.100)
        H = Phi.T * R * Phi                      # (4.97)
        w = linalg.inv(H) * Phi.T * R * z        # (4.99)
        w0 = w
        print 'w^new:', w.A1
        plt.subplot(313)
        plt.scatter(X[:, 0], X[:, 1], c=t, edgecolors='k')

        phi_0 = np.linspace(X[:, 0].min()-2, X[:, 0].max()+1, 100)
        phi_1 = -float(w[0])/float(w[2]) - float(w[1]) * phi_0 /float(w[2])
        plt.plot(phi_0, phi_1, 'k-')
        plt.draw()
    except:
        print "%d'th iterations end of loop" % j
        for i in range(N):
            print 'p(C1|φ,',X[i],'):', predict(X[i], w), ' tn:', t[i], 'logreg', logreg.predict(X[i])
        break



#logreg.fit(X, Y)
#print logreg.predict(X)

plt.show()
