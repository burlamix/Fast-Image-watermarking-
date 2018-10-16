import numpy as np
import pylab as pl

#different queue
to_print4 = np.loadtxt('pool_lms.txt') 
to_print5 = np.loadtxt('pool_msl.txt') 
to_print6 = np.loadtxt('pool_sml.txt') 






#efficiency
to_print1 = np.loadtxt('e_c.txt') 
to_print2 = np.loadtxt('e_ff.txt') 
to_print3 = np.loadtxt('e_pool.txt') 


#speed up
to_print1 = np.loadtxt('s_c_123.txt') 
to_print2 = np.loadtxt('s_ff_123.txt') 
to_print3 = np.loadtxt('s_pool_123.txt') 


#time
to_print1 = np.loadtxt('c_pof_123.txt') 
to_print2 = np.loadtxt('f_pof_123.txt') 
to_print3 = np.loadtxt('pool_123.txt') 


axe1 = np.loadtxt('axes123.txt') 
axe2 = np.loadtxt('axes64.txt') 

pl.plot(axe1,to_print1, linestyle='-', marker='v',label='C++',color="black")
pl.plot(axe1,to_print2 ,linestyle='--', marker='*',label='Fast flow',color="black")
pl.plot(axe1,to_print3 ,linestyle=':', marker='x',label='pool',color="black")
#pl.plot(axe1,axe1 ,linestyle=':', marker='x',label='ideal',color="black")


pl.legend(loc='upper right')


pl.xlabel('Par. degree', fontsize=10)
pl.ylabel('msec', fontsize=10)


pl.show()