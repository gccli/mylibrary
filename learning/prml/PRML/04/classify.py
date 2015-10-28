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
N,_ = X.shape
M = 10

iterations = 20

# (4.59)
def sigmoid(x):
    ret = 1/(1 + np.exp(-x))
    return ret

# Gaussian/polynomial basis function
def phi(x):
    ret = np.matlib.ones((M, 1))

    ret[1] = float(x[0])
    ret[2] = float(x[1])
    ret[3] = np.power(ret[1], 2)
    ret[4] = np.power(ret[2], 2)
    ret[5] = ret[1] * ret[2]
    ret[6] = np.power(ret[1], 3)
    ret[7] = np.power(ret[2], 3)
    ret[8] = ret[1] * ret[4]
    ret[9] = ret[2] * ret[3]

    return ret

# (4.87) Posterior probability of class C1
def predict(x, w):
    ret = w.T * phi(x)
    return sigmoid(float(ret))

def vpredict(X, w):
    cols,_ = X.shape
    ret = np.zeros((cols, 1))
    for i in range(cols):
        ret[i] = round(predict(X[i], w))

    return ret

logreg = linear_model.LogisticRegression(C=1e5,solver='newton-cg')
logreg.fit(X, t)

numpy.set_printoptions(precision=2)

plt.figure(1)
plt.subplot(311)
plt.scatter(X[:, 0], X[:, 1], c=t, edgecolors='k')

plt.subplot(312)
xx = np.linspace(-5, 5, 100)
plt.plot(xx, sigmoid(xx), 'k-')
plt.title('sigmoid(x)')

plt.show(block=False)

Phi = np.matlib.zeros((N, M))
for i in range(N):
    Phi[i] = phi(X[i]).T
print 'Φ:\n',Phi,'\n--------\n',Phi.shape

w = np.matlib.zeros((M, 1))

for j in range(iterations):
    R = np.matlib.eye(N, dtype=float)
    y = np.matlib.zeros((N, 1))
    z = np.matlib.zeros((N, 1))
    for i in range(N):
        y_n = predict(X[i], w)
        R[i,i] = y_n * (1 - y_n)
        y[i] = y_n
    try:
        Rinv = linalg.inv(R)
    except:
        print 'Failed to inverse R:\n',R
        R.tofile('R.txt', ',')
        break
    z = Phi * w - Rinv * (y - np.mat(t).T)    # (4.100)
    H = Phi.T * R * Phi                       # (4.97)
    try:
        w_new = linalg.inv(H) * Phi.T * R * z # (4.99)
        w = w_new
        print 'w^new:', w_new.A1
    except:
        print 'Failed to inverse H:\n',H
        break;

    plt.subplot(313)

    h = .02
    x_min, x_max = X[:, 0].min() - .5, X[:, 0].max() + .5
    y_min, y_max = X[:, 1].min() - .5, X[:, 1].max() + .5
    xx, yy = np.meshgrid(np.arange(x_min, x_max, h), np.arange(y_min, y_max, h))

    Z = vpredict(np.c_[xx.ravel(), yy.ravel()], w)
    Z = Z.reshape(xx.shape)
    plt.pcolormesh(xx, yy, Z, cmap=plt.cm.Paired)
    plt.scatter(X[:, 0], X[:, 1], c=t, edgecolors='k')

    plt.draw()

for i in range(N):
    print 'p(C1|φ): %.2f' % predict(X[i], w), \
        'original t_n:', t[i], 'logreg', logreg.predict(X[i])

plt.show()
