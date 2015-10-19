import numpy as np
import matplotlib.pyplot as plt

from sklearn import linear_model

n = 10
x = np.linspace(0, 2*np.pi, n)
y = np.sin(x) + np.rand(n)

clf = linear_model.BayesianRidge()
clf.fit(x, y)
