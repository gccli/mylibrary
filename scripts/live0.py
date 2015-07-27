#! /usr/bin/ipython
# 
# http://matplotlib.org/examples/
# http://matplotlib.org/examples/pylab_examples/date_demo2.html

from datetime import datetime as dt
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

seconds_of_day = 86400
count = 1
fig, ax = plt.subplots()
line, = ax.plot(np.random.rand(count))

ylimit = 100
xlimit = 60
ydata = [60+10*i for i in np.random.rand(count)]

ax.set_ylim(0, ylimit)
ax.set_xlim(0, xlimit)

def update(data):
    global count
    global xlimit
    count += 1
    line.set_xdata(np.arange(count))
    line.set_ydata(data)
    if (count + 1 > xlimit):
        xlimit += 60
        ax.set_xlim(0,xlimit)

    return line,

def data_gen():
    global ydata
    while True:
        ydata.append(60+10*np.random.rand())
        ret = np.array(ydata)
        yield ret

ani = animation.FuncAnimation(fig, update, data_gen, interval=200)
plt.show()
