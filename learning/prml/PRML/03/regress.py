import numpy as np
import matplotlib.pyplot as plt

# Polynomial regression: extending linear models with basis functions
# http://scikit-learn.org/stable/modules/linear_model.html
from sklearn.preprocessing import PolynomialFeatures
from sklearn.linear_model import LinearRegression,BayesianRidge
from sklearn.pipeline import Pipeline
from sklearn import svm

# Sample data
degree = 7
sigma = 0.1
n = 100
m = 5    # number of pi
i = 4    # number of extra test data
x = np.append(np.linspace(0, 2*np.pi, 2*n/m), np.linspace(3*np.pi, 5*np.pi, 2*n/m))
y = np.sin(x) + np.random.normal(0, sigma, 4*n/m)
x = x.reshape(x.size,1)

model = Pipeline([('poly', PolynomialFeatures(degree=degree)),
                  ('linear', BayesianRidge(fit_intercept=True))])
#model = Pipeline([('poly', PolynomialFeatures(degree=degree)),
#                  ('linear', LinearRegression(fit_intercept=False))])
model = model.fit(x, y)
print 'Coefficient\n', model.named_steps['linear'].coef_

svr = svm.NuSVR(kernel='rbf')
svr.fit(x, y)

# Plot outputs
plt.scatter(x, y, label="training points")
plt.plot(np.arange(0, (m+2)*np.pi, 0.1), np.sin(np.arange(0, (m+2)*np.pi, 0.1)),
         'r', label="sin(x)", linewidth=0.5)
xpred = np.linspace(0, m*np.pi, n)
xpred = xpred.reshape(xpred.size, 1)
plt.plot(xpred, model.predict(xpred), label="polynomial^%d" % degree)
xpred = np.linspace(0, (m+1)*np.pi, n)
xpred = xpred.reshape(xpred.size, 1)
plt.plot(xpred, svr.predict(xpred), 'g-', label="svr")

plt.legend(loc='upper right')
plt.show()
