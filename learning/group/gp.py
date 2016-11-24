import os
import sys
import time
import json
import numpy as np
from matplotlib import pyplot as plt
from matplotlib.transforms import Affine2D
import mpl_toolkits.axisartist.floating_axes as floating_axes
from scipy.stats import gaussian_kde


# http://stackoverflow.com/questions/17411940/matplotlib-scatter-plot-legend
class Coordinate(object):
    areas = []
    coords = {}
    flat_coords = []
    color_map = {'diamond': 'green', 'silicon': '0.75', 'player': 'red'}


    def __init__(self):
        files = os.listdir('.')
        for filename in files:
            name = filename.split('.')[0]
            if name in ['silicon', 'diamond', 'player']:
                self.parse_resource(name, filename)

        self.divide()


    def calc_density(self, xy, name=None):
        iv = 75
        count = 0
        for c in self.flat_coords:
            x = c['xy'][0]/iv
            y = c['xy'][1]/iv
            if x==xy[0] and y==xy[1]:
                count=count+1

        return count

    def divide(self):
        x = 75
        for i in range(0, 600, x):
            for j in range(0, 600, x):
                xy = [i/x, j/x]
                subarea = { 'xy': [i/x, j/x], 'count': self.calc_density(xy) }
                self.areas.append(subarea)

                print subarea
        print 'Divided map into {0} areas'.format(len(self.areas))


    # coordinate file
    # for level:  x y [ level ]
    # for player: x y [ name is_bandit ]

    def parse_resource(self, name, filename):
        level = 0
        self.coords[name] = {"color": self.color_map[name], "coords": []}

        fp = open(filename, 'r')
        for line in fp:
            info = line.split()
            x,y = int(info[0]), int(info[1])

            coord = {"xy": [x,y]}
            self.coords[name]['coords'].append(coord)

            coord['type'] = name
            if len(info) > 2:
                if name == 'player':
                    coord['label'] = info[2]
                else:
                    coord['level'] = info[2]
            if len(info) > 3 and name == 'player':
                coord['bandit'] = True
            self.flat_coords.append(coord)

            print coord

    def scatter(self, filt=None):
        for name, v in self.coords.items():
            coords = v['coords']
            if filt and filt != name:
                continue

            x = [ i['xy'][0] for i in coords ]
            y = [ i['xy'][1] for i in coords ]


            xy = np.vstack([x, y])
            z = gaussian_kde(xy)(xy)

            plt.scatter(x, y, c=z, label=name, alpha=0.5, s=100)

            #plt.scatter(x, y, c=self.coords[name]['color'], label=name, alpha=0.5,
            #            edgecolors='black')

        plt.axis([0, 600, 0, 600])
        plt.legend()
        plt.grid(True)
        plt.colorbar()
        plt.show()


    def plot(self):
        fig, ax = plt.subplots()

        for c in self.flat_coords:
            x = c['xy'][0]
            y = c['xy'][1]

            if c['type'] == 'player':
                player = c
                if player.get('bandit'):
                    ax.plot(x, y, 'ro') # others
                else:
                    ax.plot(x, y, 'g*') # out group
            elif c['type'] == 'diamond':
                ax.plot(x, y, 'gd')
            elif c['type'] == 'silicon':
                ax.plot(x, y, '^', c=self.color_map[c['type']])

        #me = self.coords['player'][0]
        #ax.annotate('weipin', xy=tuple(me['coordinate']), xycoords='data',
        #            xytext=(20, 20), textcoords='offset points',
        #            arrowprops=dict(arrowstyle="->"))


#        ax.annotate('weipin', xy=tuple(me['coordinate']), xycoords='data',
#                    xytext=(-40, 20), textcoords='offset points',
#                    arrowprops=dict(arrowstyle="->",
#                                    connectionstyle="arc,angleA=0,armA=30,rad=10"),
#        )

        plt.axis([0, 600, 0, 600])
        ax.grid(True, linewidth=1, which='both')
        plt.show()

if __name__ == '__main__':
    coord = Coordinate()
    coord.plot()
    coord.scatter('silicon')

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
