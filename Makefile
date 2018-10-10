CXX      = g++
CXX_FLAGS =  -O3 -std=c++17 
LD_FLAGS  = -L/usr/X11R6/lib -lm -pthread -lX11
OBJS     =    c_pipe_farm sequential ff_pipe_farm  pool_pq 
FF_FOLD  = -I ../fastflow

all:	$(OBJS)



sequential: clean 
	$(CXX) $(CXX_FLAGS) sequential.cpp -o sequential $(LD_FLAGS) -lstdc++fs

sequential_d: clean 
	$(CXX) $(CXX_FLAGS) sequential.cpp -o sequential $(LD_FLAGS) -lstdc++fs -g -fsanitize=address -DDEBUG 


c_pipe_farm:  
	$(CXX) $(CXX_FLAGS)   c_pipe_farm.cpp -o c_pipe_farm $(LD_FLAGS) -lstdc++fs 

c_pipe_farm_d:  clean
	$(CXX) $(CXX_FLAGS)  c_pipe_farm.cpp -o c_pipe_farm $(LD_FLAGS) -lstdc++fs -g -fsanitize=address -Dcimg_display=0 -DDEBUG


pool_pq: 
	$(CXX) $(CXX_FLAGS) $(FF_FOLD) pool_pq.cpp -o pool_pq $(LD_FLAGS) -lstdc++fs 

pool_pq_p: clean
	$(CXX) $(CXX_FLAGS) $(FF_FOLD) pool_pq.cpp -o pool_pq $(LD_FLAGS) -lstdc++fs -g -fsanitize=address -Dcimg_display=0 -DPRINTSTATUS	

pool_pq_d: clean
	$(CXX) $(CXX_FLAGS) $(FF_FOLD) pool_pq.cpp -o pool_pq $(LD_FLAGS) -lstdc++fs -g -fsanitize=address -Dcimg_display=0 

ff_pipe_farm: 
	$(CXX) $(CXX_FLAGS) $(FF_FOLD) ff_pipe_farm.cpp -o ff_pipe_farm $(LD_FLAGS) -lstdc++fs 

ff_pipe_farm_d: clean
	$(CXX) $(CXX_FLAGS) $(FF_FOLD) ff_pipe_farm.cpp -o ff_pipe_farm $(LD_FLAGS) -lstdc++fs -g -fsanitize=address -Dcimg_display=0 -DDEBUG

clean:	
	rm -f $(OBJS)

