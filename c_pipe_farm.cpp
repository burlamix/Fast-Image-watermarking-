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

#include "data_struct.cpp"

#include "util.cpp"
#define EOS NULL





//function to load the image from disk
void load_img(CImg<float> * img_mark,int split_degree ,queue<char *> *img_names, queue<task *> *img_to_mark) {



	while(true){

		auto img_n = img_names->try_pop();
	  	if(!img_n.has_value()) {     	// in thre is no value in the queque end             
	  	  return;                               
	  	} 

	  	char * img_name = img_n.value();

		#ifdef PRINTSTATUS
			auto start   = std::chrono::high_resolution_clock::now();
		#endif

	    std::atomic<int> *block_to_do = new std::atomic<int>(split_degree);
		CImg<float>* loaded_img =new CImg<float>();

  		#ifdef PRINTSTATUS
			auto start_real_t   = std::chrono::high_resolution_clock::now();
		#endif
		for (int i=0; i<10;i++){
			try{
				loaded_img->load_jpeg(img_name);				
				break;
			} catch (...) {
				std::cout<<"CImg libary exception captured" << img_name<<std::endl;
			}	
		}		  		
		#ifdef PRINTSTATUS
		auto elapsedx_real_t = std::chrono::high_resolution_clock::now() - start_real_t;
		auto msecx_real_t    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedx_real_t).count();
		#endif

		//array containing split range for each task
		std::vector<std::pair<int,int>>* rf = my_split_n(img_mark,split_degree);

		//creating each task and add to the queue
		for(int i=0;i<split_degree;i++){

			task * t = new task(loaded_img,img_mark,img_name,&rf[0][i].first,&rf[0][i].second,block_to_do,rf);
			img_to_mark->push(t);

		}
		#ifdef PRINTSTATUS
		auto elapsedx = std::chrono::high_resolution_clock::now() - start;
		auto msecx    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedx).count();
		std::cout<<" load "<< msecx <<  " rl "<<msecx_real_t<<  std::endl;
		#endif
	}
	return;
}


void mark_img(queue<task *> *img_to_mark, queue<task *> *img_marked) {

	while(true){


		auto tt = img_to_mark->try_pop();
	  	if(!tt.has_value()) {
                  
	  	  return;                               
	  	} 
  		#ifdef PRINTSTATUS
			auto start   = std::chrono::high_resolution_clock::now();
		#endif
	  	//task containing both the pointer to the image and the mark
	  	auto t = tt.value();

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
			img_marked->push(t);
			delete t->get_v();

		}else {
			delete  t;
		}

		#ifdef PRINTSTATUS
		auto elapsedx = std::chrono::high_resolution_clock::now() - start;
		auto msecx    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedx).count();
		std::cout<<"				 mark "<< msecx <<  " rl "<<msecx_real_t<<  std::endl;
		#endif
	}
	return;
}



void store(char * folder_out, queue<task *> *img_marked) {


	while(true){

		auto tt = img_marked->try_pop();
	  	if(!tt.has_value()) {                  
	  	  return;                               
	  	} 
	  	auto t = tt.value();
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
			} catch (...) {
				std::cout<<"CImg libary exception captured" << t->get_name()<<std::endl;
			}	
		}		  		
		#ifdef PRINTSTATUS

		auto elapsedx_real_t = std::chrono::high_resolution_clock::now() - start_real_t;
		auto msecx_real_t    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedx_real_t).count();
		#endif
		delete [] folder_n;

		free( t->get_name());
		delete t->get_img();
		delete  t->get_atom();
		delete  t;

		#ifdef PRINTSTATUS
		auto elapsedx = std::chrono::high_resolution_clock::now() - start;
		auto msecx    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedx).count();
		std::cout<<" 								store "<< msecx <<  " rl "<<msecx_real_t<<  std::endl;
		#endif
	}

	return;
}


int main(int argc, char * argv[]) {



	if(argc <6 ) {
		std::cout << "Usage is: " << argv[0] << " input_folder mark_name output_folder split_degree (parallel_degree for all / parallel degree for each farm) (fop or pof or barrier) " << std::endl;
		return(0);
	}

	//start total time
  	auto start   = std::chrono::high_resolution_clock::now();



  	// type of models

    CImg<float> *mark = new   CImg<float>(argv[2]);
	int split_degree  = atoi(argv[4]);

	std::string pipe_of_farm = argv[5];
	int parallel_degree  = atoi(argv[6]);
	int n_th_loader;
	int n_th_marker;
	int n_th_storer;

	if(argc < 8){
		pipe_of_farm = argv[6];
		parallel_degree  = atoi(argv[5]);
		n_th_loader= parallel_degree;
		n_th_marker= parallel_degree;
		n_th_storer= parallel_degree;

	}else{
		pipe_of_farm = argv[8];
		n_th_loader= atoi(argv[5]);
		n_th_marker= atoi(argv[6]);
		n_th_storer= atoi(argv[7]);		
	}

	char *	folder = strcat(argv[3],"/");


	// TO DELETE IS OLD
	queue<char *> * img_names = new queue<char *>;

	//make a queue with the name of the image to load
	for(auto& p: std::experimental::filesystem::directory_iterator(argv[1]))
		img_names->push(strdup( p.path().string().c_str()));
	

	if(pipe_of_farm =="pof"){

		//queue of task between stage
		queue<task *> * img_to_mark = new queue<task *>;
		queue<task *> * img_marked = new queue<task *>;

		std::vector< std::thread> th_load;
		std::vector< std::thread> th_mark;
		std::vector< std::thread> th_store;

	  	// create farms of threads for each stage

	    th_load.reserve (n_th_loader); // Reserve memory not to allocate 
	    for (int i = 0; i < n_th_loader; ++i){
	    	th_load.push_back(std::thread(load_img,mark,split_degree,img_names,img_to_mark));
		    cpu_set_t cpuset;
			CPU_ZERO(&cpuset);
			CPU_SET(i, &cpuset);
			int rc = pthread_setaffinity_np(th_load[i].native_handle(),
			                        sizeof(cpu_set_t), &cpuset);
			if (rc != 0) {
				std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
			}
	    }
	    

	    th_mark.reserve (n_th_marker);
	    for (int i = 0; i < n_th_marker; ++i){
	    	th_mark.push_back(std::thread(mark_img,img_to_mark,img_marked));
		    cpu_set_t cpuset;
			CPU_ZERO(&cpuset);
			CPU_SET(i+n_th_loader, &cpuset);
			int rc = pthread_setaffinity_np(th_mark[i].native_handle(),
			                        sizeof(cpu_set_t), &cpuset);
			if (rc != 0) {
				std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
			}	    
		}
	    

	    th_store.reserve (n_th_storer);
	    for (int i = 0; i < n_th_storer; ++i){
	    	th_store.push_back(std::thread(store,folder,img_marked));
		    cpu_set_t cpuset;
			CPU_ZERO(&cpuset);
			CPU_SET(i+n_th_loader+n_th_marker, &cpuset);
			int rc = pthread_setaffinity_np(th_store[i].native_handle(),
			                        sizeof(cpu_set_t), &cpuset);
			if (rc != 0) {
				std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
			}		    
		}


	    //wait that all loader end
		img_names->nomorewriters();
	    for(auto &th : th_load)
			th.join();

	    //wait that all marker end
		img_to_mark->nomorewriters();
		for(auto &th :th_mark)
			th.join();
		delete img_to_mark;

	    //wait that all stored end
		img_marked->nomorewriters();
		for(auto &th : th_store)
			th.join();
	    delete img_marked;


		auto elapsed = std::chrono::high_resolution_clock::now() - start;
  		auto msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

		std::cerr << "Total time " << msec << " msecs. "<< split_degree<<" split "<<(parallel_degree *3)<< " thread " << std::endl;
		std::ofstream outfile;
		outfile.open(get_date("c_"), std::ios_base::app);
		outfile << "Total time " << msec << " msecs. "<< split_degree<<" split "<<(parallel_degree *3)<< " thread " << std::endl;

	}else if(pipe_of_farm =="fop"){


		std::vector<queue<task *> *> img_to_mark; 
		std::vector<queue<task *> *> img_marked; 

		// create queue for each pipeline
	    for (int i = 0; i < parallel_degree; ++i)
	    	img_to_mark.push_back(new queue<task *>);

	    for (int i = 0; i < parallel_degree; ++i)
	    	img_marked.push_back(new queue<task *>);


		std::vector< std::thread> th_load;
		std::vector< std::thread> th_mark;
		std::vector< std::thread> th_store;

		//create vector of pipeline using the previus queue to connect each different pipeline
	    for (int i = 0; i < parallel_degree; ++i){
	    	cpu_set_t cpuset;

	    	th_load.push_back(std::thread(load_img,mark,split_degree,img_names,img_to_mark[i]));
	    	CPU_ZERO(&cpuset);
			CPU_SET((i*3)+1, &cpuset);
			int rc1 = pthread_setaffinity_np(th_store[i].native_handle(), sizeof(cpu_set_t), &cpuset);
	    	th_mark.push_back(std::thread(mark_img,img_to_mark[i],img_marked[i]));
	    	CPU_ZERO(&cpuset);
			CPU_SET((i*3)+2, &cpuset);
			int rc2 = pthread_setaffinity_np(th_store[i].native_handle(), sizeof(cpu_set_t), &cpuset);
	    	th_store.push_back(std::thread(store,folder,img_marked[i]));
	    	CPU_ZERO(&cpuset);
			CPU_SET((i*3)+3, &cpuset);
			int rc3 = pthread_setaffinity_np(th_store[i].native_handle(), sizeof(cpu_set_t), &cpuset);


			if (rc1 != 0 ||rc2 != 0  || rc3 != 0 ) {
				std::cerr << "Error calling pthread_setaffinity_np: " << rc1 << rc2<< rc3 << "\n";
			}	
	    }

		img_names->nomorewriters();

		// as before wait the and of the loader,marker, and store.

		for(auto &th : th_load)
			th.join();

	    for (int i = 0; i < parallel_degree; ++i)
			img_to_mark[i]->nomorewriters();

		for(auto &th :th_mark)
			th.join();

	    for (int i = 0; i < parallel_degree; ++i)
			delete  img_to_mark[i];		

	    for (int i = 0; i < parallel_degree; ++i)
			img_marked[i]->nomorewriters();

		for(auto &th : th_store)
			th.join();	

	    for (int i = 0; i < parallel_degree; ++i)
			delete img_marked[i];	

		auto elapsed = std::chrono::high_resolution_clock::now() - start;
  		auto msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

		std::cout << "Total time " << msec << " msecs. "<< split_degree<<" split "<<(parallel_degree *3)<< " thread " << std::endl;
		std::ofstream outfile;
		outfile.open(get_date("c_"), std::ios_base::app);
		outfile << "Total time " << msec << " msecs. "<< split_degree<<" split "<<(parallel_degree *3)<< " thread " << std::endl;

	}else if(pipe_of_farm =="barrier"){

		queue<task *> * img_to_mark = new queue<task *>;
		queue<task *> * img_marked = new queue<task *>;

		std::vector< std::thread> th_load;
		std::vector< std::thread> th_mark;
		std::vector< std::thread> th_store;


		img_names->nomorewriters();


	  	auto start_load_img   = std::chrono::high_resolution_clock::now();

	  	// create threads load_img
	    th_load.reserve (parallel_degree); // Reserve memory not to allocate 
	    for (int i = 0; i < parallel_degree; ++i){
	    	th_load.push_back(std::thread(load_img,mark,split_degree,img_names,img_to_mark));
	    	cpu_set_t cpuset;
			CPU_ZERO(&cpuset);
			CPU_SET(i, &cpuset);
			int rc = pthread_setaffinity_np(th_load[i].native_handle(),
			                        sizeof(cpu_set_t), &cpuset);
		}
	    //wait threads load_img
	    for(auto &th : th_load)
			th.join();

		auto elapsed_load_img = std::chrono::high_resolution_clock::now() - start_load_img;
		auto msec_load_img    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_load_img).count();



		img_to_mark->nomorewriters();

  		auto start_mark_img   = std::chrono::high_resolution_clock::now();

	  	// create threads mark_img
	    th_mark.reserve (parallel_degree); // Reserve memory not to allocate 
	    for (int i = 0; i < parallel_degree; ++i){
	    	th_mark.push_back(std::thread(mark_img,img_to_mark,img_marked));
	    	cpu_set_t cpuset;
			CPU_ZERO(&cpuset);
			CPU_SET(i+parallel_degree, &cpuset);
			int rc = pthread_setaffinity_np(th_mark[i].native_handle(),
			                        sizeof(cpu_set_t), &cpuset);
	    }

	    //wait threads mark_img
		for(auto &th :th_mark)
			th.join();	  
		delete img_to_mark;

		auto elapsed_mark_img = std::chrono::high_resolution_clock::now() - start_mark_img;
		auto msec_mark_img   = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_mark_img).count();



		img_marked->nomorewriters();

  		auto start_store   = std::chrono::high_resolution_clock::now();

	  	// create threads store
	    th_store.reserve (parallel_degree); // Reserve memory not to allocate 
	    for (int i = 0; i < parallel_degree; ++i){
	    	cpu_set_t cpuset;
			CPU_ZERO(&cpuset);
			CPU_SET(i+parallel_degree+parallel_degree, &cpuset);
			int rc = pthread_setaffinity_np(th_store[i].native_handle(),
			                        sizeof(cpu_set_t), &cpuset);
	    	th_store.push_back(std::thread(store,folder,img_marked));
	    }

	    //wait threads mark_img
		for(auto &th : th_store)
			th.join();
		delete img_marked;

	    auto elapsed_store = std::chrono::high_resolution_clock::now() - start_store;
  		auto msec_store    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_store).count();
		auto elapsed = std::chrono::high_resolution_clock::now() - start;
  		auto msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    	std::cout<<"total "<< msec << " load "<<msec_load_img << " mark "<< msec_mark_img<< 
    			" store "<<  msec_store << split_degree<<" split "<<" with "<<parallel_degree<< " th"<<std::endl;
	std::ofstream outfile;

	outfile.open(get_date("c_"), std::ios_base::app);
	outfile <<"total "<< msec << " load "<<msec_load_img << " mark "<< msec_mark_img<< 
    			" store "<<  msec_store << split_degree<<" split "<<" with "<<parallel_degree<< " th"<<std::endl;


	}else{
		std::cout << "select the type of model between pof, fop, barrier" << std::endl;
	}
	delete mark;

	img_names->delete_work();
 	delete img_names;



  return(0); 
	}
