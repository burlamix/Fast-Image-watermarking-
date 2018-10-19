#include <iostream>
#include <mutex>
#include <algorithm>
#include <thread>
#include <condition_variable>
#include <deque>
#include <vector>
#include <chrono>
#include <cstddef>
#include <math.h>
#include <numeric>
#include <atomic> 
#include <experimental/filesystem>
#include <fstream>

#include "data_struct.cpp"

#include "util.cpp"
#define EOS NULL






void worker_t(char * folder_out,int split_degree, CImg<float> * img_mark, tri_queue<task*> * t_queue) {

	task* t;
	while(true){

  		#ifdef PRINTSTATUS
			auto start_pop   = std::chrono::high_resolution_clock::now();
		#endif
		t = t_queue->pop();
  		#ifdef PRINTSTATUS
			auto elapsedx_pop = std::chrono::high_resolution_clock::now() - start_pop;
			auto msecx_pop    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedx_pop).count();
		#endif

	  	if(t == EOS) {  
	  		#ifdef PRINTSTATUS
	  			std::cout << "EOS" << std::endl;
			#endif
	  		return;                               
	  	} 


		switch(t->get_type())
			{
			case 0: {

		  		#ifdef PRINTSTATUS
					auto start   = std::chrono::high_resolution_clock::now();
				#endif

				char * name_img = new char[strlen(t->get_name())+1]();

				strcpy(name_img,t->get_name());

				std::atomic<int> *block_to_do = new std::atomic<int>(split_degree);


				CImg<float>* loaded_img =new CImg<float>();
				#ifdef PRINTSTATUS
					auto start_real_t   = std::chrono::high_resolution_clock::now();
				#endif
				for (int i=0; i<10;i++){

					try{
						loaded_img->load_jpeg(name_img);				
						break;
					} catch (...) {
						std::cout<<"CImg libary exception captured" << name_img<<std::endl;
					}	
				}
		  		#ifdef PRINTSTATUS
					auto elapsedx_real_t = std::chrono::high_resolution_clock::now() - start_real_t;
					auto msecx_real_t    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedx_real_t).count();
				#endif


				free( t->get_name());

				delete t;


				//array containing split range for each task
				std::vector<std::pair<int,int>>* rf = my_split_n(img_mark,split_degree);

				//creating each task and add to the queue
				for(int i=0;i<split_degree;i++){

					task * t_new = new task(loaded_img,img_mark,name_img,&rf[0][i].first,&rf[0][i].second,block_to_do,rf);
					t_queue->push(t_new,1);

				}


		  		#ifdef PRINTSTATUS
					auto elapsedx = std::chrono::high_resolution_clock::now() - start;
					auto msecx    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedx).count();
					//std::cout<<std::this_thread::get_id()<<" load "<< msecx << "pop" << msecx_pop<<   std::endl;
					std::cout<<" load "<< msecx << " pop " << msecx_pop<<  " rl "<<msecx_real_t<<  std::endl;
				#endif

			}break;
			case 1: {
		  		#ifdef PRINTSTATUS
					auto start   = std::chrono::high_resolution_clock::now();
				#endif
				
		  		#ifdef PRINTSTATUS
					auto start_real_t   = std::chrono::high_resolution_clock::now();
				#endif
				fuse_task_block( t );
		  		#ifdef PRINTSTATUS
					auto elapsedx_real_t = std::chrono::high_resolution_clock::now() - start_real_t;
					auto msecx_real_t    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedx_real_t).count();
				#endif
				std::atomic<int> *block_to_do =	t->get_atom();

				
				// decrease the number of pices to computer for the image
				// if is 0 means that all the image was computed so it can be stored in memory
				if( --(*block_to_do) == 0){ 
					t->set_type(2);
					t_queue->push(t,2);
					delete t->get_v();
				}else {
					delete  t;
				}    

		  		#ifdef PRINTSTATUS
					auto elapsedx = std::chrono::high_resolution_clock::now() - start;
					auto msecx    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedx).count();
					//std::cout<<"				"<<std::this_thread::get_id()<<" mark "<< msecx << "pop" << msecx_pop<<   std::endl;
					std::cout<<"				"<<" mark "<< msecx << " pop " << msecx_pop<<  " rl "<<msecx_real_t<<  std::endl;
				#endif

				}break;
			case 2: {

		  		#ifdef PRINTSTATUS
					auto start   = std::chrono::high_resolution_clock::now();
				#endif

				//merge the name of the image and the path of the output floder
				char * folder_n = new char[strlen(folder_out)+strlen(t->get_name())]();
					strcpy(folder_n,folder_out);
					strcat(folder_n,basename(t->get_name()));

				#ifdef PRINTSTATUS
					auto start_real_t   = std::chrono::high_resolution_clock::now();
				#endif
				for (int i=0; i<10;i++){
					try{

							t->get_img()->save_jpeg(folder_n);


						break;
					} catch (...) {	std::cout<<"CImg libary exception captured" <<std::endl;}	
				}
				#ifdef PRINTSTATUS
					auto elapsedx_real_t = std::chrono::high_resolution_clock::now() - start_real_t;
					auto msecx_real_t    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedx_real_t).count();
				#endif

				delete [] folder_n;
				delete [] t->get_name();
				delete t->get_img();
				delete  t->get_atom();
				delete  t;
   

				if(t_queue->end())
					t_queue->notify_all_j();  

		  		#ifdef PRINTSTATUS
					auto elapsedx = std::chrono::high_resolution_clock::now() - start;
					auto msecx    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedx).count();
					//std::cout<<"								"<<std::this_thread::get_id()<<" save "<< msecx << "pop" << msecx_pop<<   std::endl;
					std::cout<<"								"<<" save "<< msecx << " pop " << msecx_pop<<  " rl "<<msecx_real_t<<  std::endl;
				#endif

			}break;
				default:{std::cout << "something go wrong" << std::endl;}
			}

	}

	return ;
}




int main(int argc, char * argv[]) {



	if(argc <5 ) {
		std::cout << "Usage is: " << argv[0] << " input_folder mark_name output_folder split_degree parallel_degree priority " << std::endl;
		return(0);
	}

	//start total time
  	auto start   = std::chrono::high_resolution_clock::now();


    CImg<float> *mark = new   CImg<float>(argv[2]);
	int split_degree  = atoi(argv[4]);
	int parallel_degree  = atoi(argv[5]);
	std::string priority  = argv[6];


	char *	folder = strcat(argv[3],"/");


	tri_queue<task*> * t_queue = new tri_queue<task*>(3,priority);


	for(auto& p: std::experimental::filesystem::directory_iterator(argv[1])){
		task * t = new task(strdup( p.path().string().c_str()));
		t_queue->push(t,0);
	}



	std::vector< std::thread> th_worker;


    th_worker.reserve (parallel_degree); // Reserve memory not to allocate 
    for (int i = 0; i < parallel_degree; ++i) {
    	th_worker.push_back(std::thread(worker_t,folder,split_degree,mark,t_queue));
		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		CPU_SET(i, &cpuset);
		int rc = pthread_setaffinity_np(th_worker[i].native_handle(),
		                        sizeof(cpu_set_t), &cpuset);
		if (rc != 0) {
			std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
		}
    }



    //wait that all worker 
    for(auto &th : th_worker)
		th.join();

	t_queue->delete_sunqueue();

	delete t_queue;
	delete mark;
	

	auto elapsed = std::chrono::high_resolution_clock::now() - start;
	auto msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

	std::cout << "Total time " << msec << " msecs. "<< split_degree<<" split "<<parallel_degree<< " thread " << std::endl;

	std::ofstream outfile;
	std::string a =get_date("pool_");
	outfile.open(a, std::ios_base::app);
	outfile << "Total time " << msec << " msecs. "<< split_degree<<" split "<<parallel_degree<< " thread " << std::endl;




	return(0); 
}
