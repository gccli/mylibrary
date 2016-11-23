import sys
import time
import json
import numpy as np
from matplotlib import pyplot as plt
from matplotlib.transforms import Affine2D
import mpl_toolkits.axisartist.floating_axes as floating_axes

x = np.arange(0, 600, 1)
y = np.arange(0, 600, 1)

# http://stackoverflow.com/questions/17411940/matplotlib-scatter-plot-legend
class Coordinate(object):
    flat_coords = []

    def __init__(self, config):
        with open(config) as fp:
            self.coords = json.load(fp)
        for k,v in self.coords.iteritems():
            for x in v:
                x['type'] = k
                self.flat_coords.append(x)


    def plot(self):
        fig, ax = plt.subplots()

        for c in self.flat_coords:
            x = c['coordinate'][0]
            y = c['coordinate'][1]

            if c['type'] == 'player':
                player = c
                if player.get('label'):
                    ax.plot(x, y, 'ro') # others
                else:
                    ax.plot(x, y, 'g*') # out group
            elif c['type'] == 'diamond':
                ax.plot(x, y, 'g^')

        me = self.coords['player'][0]
        ax.annotate('weipin', xy=tuple(me['coordinate']), xycoords='data',
                    xytext=(20, 20), textcoords='offset points',
                    arrowprops=dict(arrowstyle="->"))


#        ax.annotate('weipin', xy=tuple(me['coordinate']), xycoords='data',
#                    xytext=(-40, 20), textcoords='offset points',
#                    arrowprops=dict(arrowstyle="->",
#                                    connectionstyle="arc,angleA=0,armA=30,rad=10"),
#        )

        plt.axis([0, 600, 0, 600])
        ax.grid(True, linewidth=1, which='both')
        plt.show()


if __name__ == '__main__':
    coord = Coordinate('world.json')
    coord.plot()

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
