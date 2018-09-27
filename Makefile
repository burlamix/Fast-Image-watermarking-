CXX      = g++
CXX_FLAGS = -O3 -std=c++17 
LD_FLAGS  = -L/usr/X11R6/lib -lm -pthread -lX11
OBJS     =   main pipeline sequential pipe_of_farm ff_pipe_of_farm
FF_FOLD  = -I ../fastflow

all:	$(OBJS)



sequential: clean 
	$(CXX) $(CXX_FLAGS) sequential.cpp -o sequential $(LD_FLAGS) -lstdc++fs

sequential_d: clean 
	$(CXX) $(CXX_FLAGS) sequential.cpp -o sequential $(LD_FLAGS) -lstdc++fs -g -fsanitize=address


c_pipe_farm:  clean
	$(CXX) $(CXX_FLAGS)   c_pipe_farm.cpp -o c_pipe_farm $(LD_FLAGS) -lstdc++fs 

c_pipe_farm_d:  clean
	$(CXX) $(CXX_FLAGS)  c_pipe_farm.cpp -o c_pipe_farm $(LD_FLAGS) -lstdc++fs -g -fsanitize=address -Dcimg_display=0

ff_pipe_farm: clean
	$(CXX) $(CXX_FLAGS) $(FF_FOLD) ff_pipe_farm.cpp -o ff_pipe_farm $(LD_FLAGS) -lstdc++fs 

ff_pipe_farm_d: clean
	$(CXX) $(CXX_FLAGS) $(FF_FOLD) ff_pipe_farm.cpp -o ff_pipe_farm $(LD_FLAGS) -lstdc++fs -g -fsanitize=address -Dcimg_display=0

clean:	
	rm -f $(OBJS)

