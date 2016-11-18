import sys
import time
import numpy as np
from matplotlib import pyplot as plt
from matplotlib.transforms import Affine2D
import mpl_toolkits.axisartist.floating_axes as floating_axes

x = np.arange(0, 600, 1)
y = np.arange(0, 600, 1)

def get_coordinates():
    coords = []
    with open('coordinates') as fp:
        while True:
            line = fp.readline()
            if not line:
                break
            coord = tuple(line.split())
            coords.append(coord)

    return coords

def check_coord(coord):
    x = int(coord[0])
    y = int(coord[1])
    if (x < 0 or x >= 600) or (y < 0 or y >= 600):
        return False
    return True

coordinates = get_coordinates()

fig = plt.figure()
for coord in coordinates:
    if not check_coord(coord):
        continue
    print coord
    plt.plot(coord[0], coord[1], 'g^')

plt.axis([0, 600, 0, 600])
plt.grid(True, linewidth=1, which='both')
plt.show()


#plot_extents = 0, 10, 0, 10
#transform = Affine2D().rotate_deg(45)
#helper = floating_axes.GridHelperCurveLinear(transform, plot_extents)
#ax = floating_axes.FloatingSubplot(fig, 111, grid_helper=helper)

#fig.add_subplot(ax)
#plt.show()


#ax = fig.gca()
#ax.set_xticks(np.arange(0,1,0.1))
#ax.set_yticks(np.arange(0,1.,0.1))

#plt.scatter(x, y)
#plt.grid()
#plt.show()
