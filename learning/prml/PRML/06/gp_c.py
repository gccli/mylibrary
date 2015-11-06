import sys
import time
import numpy as np
import numpy.linalg as LA # Linear Algebra
from scipy import stats
from sklearn.gaussian_process import GaussianProcess
from matplotlib import pyplot as plt
from sklearn import datasets

np.set_printoptions(2)

iris = datasets.load_iris()
X = iris.data[:, :2]  # only take the first two features.
y = iris.target[iris.target < 2]   # only for two-class classification
X = X[:y.size]
y[ y==0 ] = -1

# input train data
beta = .05

def logit(z): return 1.0/(1+np.exp(-z))
def se(x,y):
    s = 0.0
    for i in range(x.size):
        s += np.power(abs(x[i]-y[i]), 2)
    return np.exp(-s/2)

def kernel(x, y):
    n,_ = x.shape
    m,_ = y.shape
    K = np.zeros((n,m))
    for i in range(n):
        for j in range(m):
            K[i,j] = se(x[i], y[j])
    return K

xx = np.array([1.5, 2.0])
print 'k(x)',kernel(xx, xx)
sys.exit(0)


def gp_train(X, y):
    iteration = 10

    n,_ = X.shape
    I = np.identity(n);
    # Add a small constant, to improve the numerical conditioning of K
    K = kernel(X, X) + 0.01 * I

    f = np.zeros(n)
    W = np.zeros((n,n))
    W_sqrt = np.zeros((n,n))
    t = (y+1)/2

    logp = np.zeros(n)
    while iteration > 0:
        iteration -= 1
        pi = logit(f)
        for i in range(n):
            W[i,i] = pi[i] * (1-pi[i])
            W_sqrt[i,i] = np.sqrt(W[i,i])
        B = I + np.dot(np.dot(W_sqrt, K), W_sqrt) # 3.26
        L = LA.cholesky(B)

        t_pi = t-pi
        b = np.dot(W, f) + t_pi

        tmp = np.dot(np.dot(W_sqrt,K), b)
        tmp = np.dot(LA.inv(L), tmp)
        tmp = np.dot(LA.inv(L.T), tmp)
        tmp = np.dot(W_sqrt, tmp)

        a = b - tmp
        f = np.dot(K,a)
    for i in range(n):
        logp[i] = - np.log(1+np.exp(y[i] * f[i]))

    L_Sum = 0.0
    for i in range(n): L_Sum += np.log(L[i,i])
    logp = logp - 1.0/2*np.dot(a.T, f)
    logp = logp - L_Sum
    #logq = -1./2 * np.dot(np.dot(f.T, LA.inv(K)), f) + logp - 1./2 * np.log(LA.det(B))
    #print '-------- logp:\n',logp
    #print '-------- logq:\n',logq
    return f,logp,K

# targets
hat_f,log_q,K = gp_train(X,y)
def gp_predict(f, X, y, x):
    W = np.zeros((n,n))
    W_sqrt = np.zeros((n,n))

    for i in range(n):
        W[i,i] = pi[i] * (1-pi[i])
        W_sqrt[i,i] = np.sqrt(W[i,i])
    B = I + np.dot(np.dot(W_sqrt, K), W_sqrt) # 3.26

    f_star = kernel(x).T

    pass


plt.figure(1)
#plt.subplot(221)
plt.scatter(X[:,0], X[:,1], c=y)
#plt.plot(X, t, 'g.-.', label=u'$Gaussian\,Process\,\,f(x) $')
#plt.legend(loc='upper left')
plt.show(block=True)
