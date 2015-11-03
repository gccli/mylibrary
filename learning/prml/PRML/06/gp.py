import sys
import time
import numpy as np
from scipy import stats
from sklearn.gaussian_process import GaussianProcess
from matplotlib import pyplot as plt
from matplotlib import cm

np.set_printoptions(2)

# input train data
X = np.linspace(0, 3*np.pi, 60)
beta = .05

def se(x,y): return np.exp(-1/2 * np.power(abs(x-y), 2))
def kernel(x, y):
    n = x.size
    m = y.size
    K = np.zeros((n,m))
    for i in range(n):
        for j in range(m):
            K[i,j] = se(x[i], y[j])
    return K

def gp_train(x):
    K = kernel(x, x)

    while True:
        try:
            np.random.seed()
            K = K + np.random.normal(0,beta) * np.identity(x.size)
            L = np.linalg.cholesky(K)
            break
        except np.linalg.linalg.LinAlgError as e:
            print 'TRAIN: Compute Cholesky', e
            time.sleep(0.5)

    mu = np.random.randn(x.size)
    y = np.dot(L, mu)
    # y = np.sin(x) + np.random.normal(0,beta) * np.ones((1, x.size))
    print y.shape
    return (K,y)

# targets
K,t = gp_train(X)

def gp_predict(x):
    KKnew = kernel(X, x)
    KnewK = kernel(x, X)

    Knew = kernel(x, x)
    K_inv = np.linalg.inv(K)
    try:
        mu = np.dot(KnewK, K_inv)
        mu = np.dot(mu, t)
        tmp = np.dot(KnewK, K_inv)
        tmp = np.dot(tmp, KKnew)
        cov = Knew - tmp
        L = np.linalg.cholesky(cov)
    except np.linalg.linalg.LinAlgError as e:
        print 'Compute Cholesky for predict:', e
        sys.exit(1)

    u = np.random.randn(x.size)
    y = mu + np.dot(L, u)

    return y

plt.figure(1)
plt.scatter(X, t, edgecolors='k')
plt.plot(X, t, 'g.-.', label=u'$Gaussian\,Process\,\,f(x) $')

# for test
end = 3.5 * np.pi
x = np.linspace(0, end, 40)
y = gp_predict(x)
plt.plot(x, y, 'y', label=u'$posterior\,of\,\mathcal{GP}$')
x = np.linspace(0, end, 40)
y = gp_predict(x)
plt.plot(x, y, 'k', label=u'$posterior\,of\,\mathcal{GP}$')
x = np.linspace(0, end, 40)
y = gp_predict(x)
plt.plot(x, y, 'r', label=u'$posterior\,of\,\mathcal{GP}$')



plt.legend(loc='upper left')
plt.show()
