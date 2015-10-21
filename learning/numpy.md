Array
=====

Array create
------------
[Array create routines](http://docs.scipy.org/doc/numpy/reference/routines.array-creation.html#routines-array-creation)

```
x = np.array([[2, 3],[1, 0]])
x = np.zeros((2, 3))
x = np.arange(10)
x = np.arange(2, 3, 0.1)
x = np.linspace(1., 4., 6)
x = np.indices((3,3))
```

Array manipulation
------------------
1. shape
```
x = np.arange(10)
x = np.reshape(x, (5,2)) # result a 5x2 matrix
x = np.revel(x)          # return a flattened array
```

2. transpose
```
x = np.arange(12).reshape(3,4)
x = np.swapxes(x, 0,1) equiv x.transpose(1,0)
```
