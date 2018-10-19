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
#include <experimental/filesystem>
#include <iostream>
#include <ff/pipeline.hpp>
#include <ff/farm.hpp>
using namespace ff;
#include "data_struct.cpp"
#include "util.cpp"


//node to get the file name as a stream
struct getfile_name_node: ff_node_t<char ,char > {

	char * i_folder;

	getfile_name_node( char * i_f): i_folder(i_f) {} 

	char*  svc(char *) {

	for(auto& p: std::experimental::filesystem::directory_iterator(i_folder)){		

		ff_send_out(strdup( p.path().string().c_str()));

	}

	return EOS;
}};

//loader node
struct load_node: ff_node_t<char ,task  > {

	CImg<float>  *img_mark;
	int split_degree ;

	load_node( CImg<float> * mark, int split_d): img_mark(mark) ,split_degree (split_d){} 

	task *svc(char *img_name) {
		#ifdef PRINTSTATUS
			auto start   = std::chrono::high_resolution_clock::now();
		#endif
		CImg<float>* loaded_img =new CImg<float>();



		#ifdef PRINTSTATUS
			auto start_real_t   = std::chrono::high_resolution_clock::now();
		#endif
		for (int i=0; i<10;i++){
			try{
				loaded_img->load_jpeg(img_name);				
				break;
			} catch (...) {
				std::cout<<"CImg libary exception captured" << loaded_img<<std::endl;
			}	
		}
		#ifdef PRINTSTATUS
		auto elapsedx_real_t = std::chrono::high_resolution_clock::now() - start_real_t;
		auto msecx_real_t    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedx_real_t).count();
		#endif

	    std::atomic<int> *block_to_do = new std::atomic<int>(split_degree);

		//array containing split range for each task
		std::vector<std::pair<int,int>>* rf = my_split_n(img_mark,split_degree);

		//creating each task and add to the queue
		for(int i=0;i<split_degree;i++){

			task * t = new task(loaded_img,img_mark,img_name,&rf[0][i].first,&rf[0][i].second,block_to_do,rf);
			ff_send_out(t);
		}
		#ifdef PRINTSTATUS
		auto elapsedx = std::chrono::high_resolution_clock::now() - start;
		auto msecx    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedx).count();
		std::cout<<" load "<< msecx <<  " rl "<<msecx_real_t<<  std::endl;
		#endif
		return GO_ON;
	}
};



//marker node
struct mark_node: ff_node_t<task,task> {

	task *svc(task *t) {

  		#ifdef PRINTSTATUS
			auto start   = std::chrono::high_resolution_clock::now();
		#endif
		std::atomic<int> *block_to_do =	t->get_atom();
  		#ifdef PRINTSTATUS
			auto start_real_t   = std::chrono::high_resolution_clock::now();
		#endif
	  	//task containing both the pointer to the image and the mark
	    fuse_task_block( t );
		#ifdef PRINTSTATUS
		auto elapsedx_real_t = std::chrono::high_resolution_clock::now() - start_real_t;
		auto msecx_real_t    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedx_real_t).count();
		#endif
	  	// decrease the number of pices to computer for the image
		// if is 0 means that all the image was computed so it can be stored in memory
		if( --(*block_to_do) == 0) {
			delete t->get_v();
			#ifdef PRINTSTATUS
			auto elapsedx = std::chrono::high_resolution_clock::now() - start;
			auto msecx    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedx).count();
			std::cout<<"				 mark "<< msecx <<  " rl "<<msecx_real_t<<  std::endl;
			#endif
			return t;
		}else {
			delete  t;
		}
		#ifdef PRINTSTATUS
		auto elapsedx = std::chrono::high_resolution_clock::now() - start;
		auto msecx    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedx).count();
		std::cout<<"				 mark "<< msecx <<  " rl "<<msecx_real_t<<  std::endl;
		#endif
	return GO_ON;
	}
};

struct store_node: ff_node_t<task,int> {

	char *  folder_out;

	store_node( char* folder_o): folder_out(folder_o) {} 

	int *svc(task* t) {
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
				std::cout<<"CImg libary exception captured" << t->get_img()<<std::endl;
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
	return GO_ON;
}};



int main(int argc, char * argv[]) {


	if(argc < 6) {
		std::cout << "Usage is: " << argv[0] << " input_folder mark_name output_folder split_degree parallel_degree (fop or pof) " << std::endl;
		return(0);
	}


	std::string pipe_of_farm = argv[6];
    CImg<float>*mark =   new CImg<float>(argv[2]);
	int split_degree  = atoi(argv[4]);
	int parallel_degree  = atoi(argv[5]);


	char *	folder_in = argv[1];
	char *	folder_out = strcat(argv[3],"/");

  	auto start   = std::chrono::high_resolution_clock::now();

	if(pipe_of_farm =="pof"){

		std::vector<std::unique_ptr<ff_node>> th_load;
		std::vector<std::unique_ptr<ff_node>> th_mark;
		std::vector<std::unique_ptr<ff_node>> th_store;

	  	// create farms of threads for each stage

		for(size_t i=0;i<parallel_degree;++i)
			th_load.push_back(std::unique_ptr<ff_node_t<char,task>> (make_unique<load_node>(mark,split_degree)));

		for(size_t i=0;i<parallel_degree;++i)
			th_mark.push_back(std::unique_ptr<ff_node_t<task,task>> (make_unique<mark_node>()));

		for(size_t i=0;i<parallel_degree;++i)
			th_store.push_back(std::unique_ptr<ff_node_t<task,int>> (make_unique<store_node>(folder_out)));


		ff_Farm<> farm_load(std::move(th_load));
	    ff_Farm<> farm_mark(std::move(th_mark));
	    ff_Farm<> farm_store(std::move(th_store));

	    //remove collector since the store are indipendently and there is no result to merge
		farm_store.remove_collector();
	    
	    getfile_name_node  generator_name(folder_in);

		ff_Pipe<> pipe(generator_name,farm_load,farm_mark,farm_store);

		pipe.run_and_wait_end();

	}else if (pipe_of_farm == "fop"){


		std::vector<std::unique_ptr<ff_node>> farm_o_pipe;

		//create vector of pipeline
		for(size_t i=0;i<parallel_degree;++i)
			farm_o_pipe.push_back( make_unique<ff_Pipe<>> 
				(make_unique<load_node>(mark,split_degree),make_unique<mark_node>(),make_unique<store_node>(folder_out)));

	    ff_Farm<> farm_of_pipe(std::move(farm_o_pipe));
		farm_of_pipe.remove_collector();

	    getfile_name_node  generator_name(folder_in);

	    //create pipeline with the name stream generator
		ff_Pipe<> pipe(generator_name,farm_of_pipe);

		pipe.run_and_wait_end();

	}else{

		std::cout <<" choase between: fop for farm of pipe, pof fo pipe of farm, no other possibility " << std::endl;
		return 0;

	}
	delete mark;

	auto elapsed = std::chrono::high_resolution_clock::now() - start;
	auto msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

	std::cerr << "Total time " << msec << " msecs. "<< split_degree<<" split "<<(parallel_degree *3)<< " thread " << std::endl;
	std::ofstream outfile;
	outfile.open(get_date("ff_"), std::ios_base::app);
	outfile << "Total time " << msec << " msecs. "<< split_degree<<" split "<<(parallel_degree *3)<< " thread " << std::endl;

	return 0;
}
	


/* per il taglio:


la farm per il marker rimane uguale,
divido per righe e mi aggiungo un atomica che conta quante parti 
dell'immagine devono essere ancora eseguite, quando raggiungo lo zero(dentro il makrker)
allora mando l'immagine allo store*/