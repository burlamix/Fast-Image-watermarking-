import numpy as np
import pylab as pl

to_print1 = np.loadtxt('arr.txt') 
to_print2 = np.loadtxt('arr2.txt') 
to_print3 = np.loadtxt('arr3.txt') 
to_print4 = np.loadtxt('arr4.txt') 
to_print5 = np.loadtxt('fast_flow_pof.txt') 

pl.plot(to_print4,to_print1, linestyle='-', marker='v',label='Fast flow',color="black")
pl.plot(to_print4,to_print5 ,linestyle='--', marker='*',label='C++',color="black")
pl.plot(to_print4,to_print4 ,linestyle=':', marker='x',label='theoretical',color="black")
#pl.plot(to_print4,to_print3, linestyle=':', marker='x',label='Store',color="black")


pl.legend(loc='upper right')


pl.xlabel('Par. degree', fontsize=10)
pl.ylabel('msec', fontsize=10)


pl.show()