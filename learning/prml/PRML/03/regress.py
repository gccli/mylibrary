import numpy as np
import matplotlib.pyplot as plt

# Polynomial regression: extending linear models with basis functions
# http://scikit-learn.org/stable/modules/linear_model.html
from sklearn.preprocessing import PolynomialFeatures
from sklearn import linear_model
from sklearn.linear_model import Ridge
from sklearn.pipeline import Pipeline

degree = 6
sigma = 0.1
n = 100
x = np.linspace(0, 3*np.pi, n)
y = np.sin(x) + np.random.normal(0, sigma, n)
X = x.reshape(x.size,1)

model = Pipeline([('poly', PolynomialFeatures(degree=degree)),
                  ('linear', linear_model.LinearRegression(fit_intercept=False))])
model = model.fit(X, y)
print model.named_steps['linear'].coef_

# Plot outputs
plt.plot(x, np.sin(x), 'b', label="sin(x)")
plt.scatter(x, y, label="training points")
plt.plot(X, model.predict(X), 'g-', label="polynomial of degree %d" % degree)
plt.legend(loc='lower left')
plt.show()
