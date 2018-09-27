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


		char * img_name = img_names->pop();

	  	if(img_name == EOS) {     	// in thre is no value in the queque end   
	  		img_to_mark->push(EOS);
	  	  	return;                               
	  	} 


	    std::atomic<int> *block_to_do = new std::atomic<int>(split_degree);

		CImg<float>* loaded_img = new CImg<float>(img_name);

		//array containing split range for each task
		std::vector<std::pair<int,int>>* rf = my_split_n(img_mark,split_degree);

		//creating each task and add to the queue
		for(int i=0;i<split_degree;i++){

			task * t = new task(loaded_img,img_mark,img_name,&rf[0][i].first,&rf[0][i].second,block_to_do,rf);
			img_to_mark->push(t);

		}

	}
	return;
}


void mark_img(queue<task *> *img_to_mark, queue<task *> *img_marked) {

	while(true){

 

		task * t = img_to_mark->pop();
	  	if(t == EOS) {     	// in thre is no value in the queque end   
          img_marked->push(EOS);
	  	  return;                               
	  	} 

	  	fuse_task_block( t );

		std::atomic<int> *block_to_do =	t->get_atom();

	  	// decrease the number of pices to computer for the image
		// if is 0 means that all the image was computed so it can be stored in memory
		if( --(*block_to_do) == 0){ 
			img_marked->push(t);
			delete t->get_v();

		}else {
			delete  t;
		}


	}
	return;
}



void store(char * folder_out, queue<task *> *img_marked) {


	while(true){
		task * t = img_marked->pop();
	  	if(t == EOS) {     	// in thre is no value in the queque end   
	  	  return;                               
	  	} 

	    //merge the name of the image and the path of the output floder
	    char * folder_n = new char[strlen(folder_out)+strlen(t->get_name())]();
	   	strcpy(folder_n,folder_out);
	   	strcat(folder_n,basename(t->get_name()));

		t->get_img()->save(folder_n);

		delete [] folder_n;

		free( t->get_name());
		delete t->get_img();
		delete  t->get_atom();
		delete  t;

	}

	return;
}

void worker (CImg<float> * img_mark, int split_degree ,char * folder_out, queue<char *> *img_names, queue<task *> *img_to_mark, queue<task *> *img_marked) {


	load_img(img_mark, split_degree ,img_names, img_to_mark);
	mark_img(img_to_mark, img_marked);
	store(folder_out, img_marked) ;

}


int main(int argc, char * argv[]) {



	if(argc <6 ) {
		std::cout << "Usage is: " << argv[0] << " input_folder mark_name output_folder split_degree parallel_degree (fop or pof) " << std::endl;
		return(0);
	}

	//start total time
  	auto start   = std::chrono::high_resolution_clock::now();

  	// type of models
	std::string pipe_of_farm = argv[6];

    CImg<float> *mark = new   CImg<float>(argv[2]);
	int split_degree  = atoi(argv[4]);
	int parallel_degree  = atoi(argv[5]);


	char *	folder = strcat(argv[3],"/");



	queue<char *> * img_names = new queue<char *>;

	//make a queue with the name of the image to load
	for(auto& p: std::experimental::filesystem::directory_iterator(argv[1])){
		img_names->push(strdup( p.path().string().c_str()));
	}

	for(int i=0; i<parallel_degree; i++)
	    img_names->push(EOS); 

	if(pipe_of_farm =="pof"){

		//queue of task between stage
		queue<task *> * img_to_mark = new queue<task *>;
		queue<task *> * img_marked = new queue<task *>;

		std::vector< std::thread> th_load;
		std::vector< std::thread> th_mark;
		std::vector< std::thread> th_store;

	  	// create farms of threads for each stage

	    th_load.reserve (parallel_degree); // Reserve memory not to allocate 
	    for (int i = 0; i < parallel_degree; ++i)
	    	th_load.push_back(std::thread(load_img,mark,split_degree,img_names,img_to_mark));
	    

	    th_mark.reserve (parallel_degree);
	    for (int i = 0; i < parallel_degree; ++i)
	    	th_mark.push_back(std::thread(mark_img,img_to_mark,img_marked));
	    

	    th_store.reserve (parallel_degree);
	    for (int i = 0; i < parallel_degree; ++i)
	    	th_store.push_back(std::thread(store,folder,img_marked));


	    //wait that all loader end
		img_names->nomorewriters();
	    for(auto &th : th_load)
			th.join();
		delete img_names;

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

		std::cerr << "Total time " << msec << " msecs. "<< split_degree<<" split "<<parallel_degree<< " thread " << std::endl;

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

	    	th_load.push_back(std::thread(load_img,mark,split_degree,img_names,img_to_mark[i]));
	    	th_mark.push_back(std::thread(mark_img,img_to_mark[i],img_marked[i]));
	    	th_store.push_back(std::thread(store,folder,img_marked[i]));
	    }

		img_names->nomorewriters();

		// as before wait the and of the loader,marker, and store.

		for(auto &th : th_load)
			th.join();
		delete img_names;

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

		std::cerr << "Total time " << msec << " msecs. "<< split_degree<<" split "<<parallel_degree<< " thread " << std::endl;

	}else if(pipe_of_farm =="no"){

		queue<task *> * img_to_mark = new queue<task *>;
		queue<task *> * img_marked = new queue<task *>;

		std::vector< std::thread> th_load;
		std::vector< std::thread> th_mark;
		std::vector< std::thread> th_store;


		img_names->nomorewriters();


	  	auto start_load_img   = std::chrono::high_resolution_clock::now();

	  	// create threads load_img
	    th_load.reserve (parallel_degree); // Reserve memory not to allocate 
	    for (int i = 0; i < parallel_degree; ++i)
	    	th_load.push_back(std::thread(load_img,mark,split_degree,img_names,img_to_mark));
	    //wait threads load_img
	    for(auto &th : th_load)
			th.join();
		delete img_names;

		auto elapsed_load_img = std::chrono::high_resolution_clock::now() - start_load_img;
		auto msec_load_img    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_load_img).count();



		img_to_mark->nomorewriters();

  		auto start_mark_img   = std::chrono::high_resolution_clock::now();

	  	// create threads mark_img
	    th_mark.reserve (parallel_degree); // Reserve memory not to allocate 
	    for (int i = 0; i < parallel_degree; ++i)
	    	th_mark.push_back(std::thread(mark_img,img_to_mark,img_marked));

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
	    for (int i = 0; i < parallel_degree; ++i)
	    	th_store.push_back(std::thread(store,folder,img_marked));

	    //wait threads mark_img
		for(auto &th : th_store)
			th.join();
		delete img_marked;

	    auto elapsed_store = std::chrono::high_resolution_clock::now() - start_store;
  		auto msec_store    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_store).count();
		auto elapsed = std::chrono::high_resolution_clock::now() - start;
  		auto msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    	std::cerr<<"total "<< msec << " load "<<msec_load_img << " mark "<< msec_mark_img<< 
    			" store "<<  msec_store << split_degree<<" split "<<" with "<<parallel_degree<< " th"<<std::endl;

	}else if(pipe_of_farm =="new"){

		//queue of task between stage
		queue<task *> * img_to_mark = new queue<task *>;
		queue<task *> * img_marked = new queue<task *>;

		std::vector< std::thread> th_worker;

//void worker (CImg<float> * img_mark, int split_degree ,char * folder_out, queue<char *> *img_names, queue<task *> *img_to_mark, queue<task *> *img_marked) {

	  	// create farms of threads for each stage

	    th_worker.reserve (parallel_degree); // Reserve memory not to allocate 
	    for (int i = 0; i < parallel_degree; ++i)
	    	th_worker.push_back(std::thread(worker,mark,split_degree,folder,img_names,img_to_mark,img_marked));
	    
	    //wait that all worker end
	    for(auto &th : th_worker)
			th.join();
		
		delete img_names;
		delete img_to_mark;
	    delete img_marked;


		auto elapsed = std::chrono::high_resolution_clock::now() - start;
  		auto msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

		std::cerr << "Total time " << msec << " msecs. "<< split_degree<<" split "<<parallel_degree<< " thread " << std::endl;

	}else{
		std::cout << "Usage is: " << argv[0] << " input_folder mark_name output_folder split_degree parallel_degree (fop or pof) " << std::endl;

		std::cout <<" choase between: fop for farm of pipe, pof fo pipe of farm, no to pipe of farm with barrier. no other possibility " << std::endl;
		return 0;

	}
	delete mark;



  return(0); 
}