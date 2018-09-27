#include "../CImg/CImg.h"
#include <iostream>
#include <chrono>
#include <future>
#include <fstream>
#include <iostream>
#include <string> 
#include <typeinfo>
#include <experimental/filesystem>

#include <vector>
#include <cstdlib>
#include <thread>
#include <optional>
#include <atomic>
#include <climits>

#include "data_struct.cpp"
#include "util.cpp"

namespace fs = std::experimental::filesystem;

using namespace cimg_library;



int main(int argc, char * argv[]) {

	if(argc < 3) {
		std::cout << "Usage is: " << argv[0] << " input_folder mark_name output_folder " << std::endl;
		return(0);
	}
	auto start_total   = std::chrono::high_resolution_clock::now();
	

	CImg<float>* img ;
    CImg<float> *mark = new   CImg<float>(argv[2]);
    char * output_folder =strcat(argv[3],"/");
	char * folder_n;
	char * dir = argv[3];
	std::string file_name;

	for(auto& file: fs::directory_iterator(dir)){

		file_name = file.path().string();

		auto start_load   = std::chrono::high_resolution_clock::now();

		img = new CImg<float>(file_name.c_str());

		auto elapsed_load = std::chrono::high_resolution_clock::now() - start_load;
		auto msec_load    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_load).count();

		auto start_mark   = std::chrono::high_resolution_clock::now();

		fuse_img(img,mark);



		auto elapsed_mark = std::chrono::high_resolution_clock::now() - start_mark;
		auto msec_mark    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_mark).count();


	    folder_n = new char[strlen(output_folder)+strlen(file_name.c_str())]();
	   	strcpy(folder_n,output_folder);
	   	strcat(folder_n,basename(file_name.c_str()));
		auto start_store  = std::chrono::high_resolution_clock::now();

		img->save(folder_n );


		auto elapsed_store = std::chrono::high_resolution_clock::now() - start_store;
		auto msec_store    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_store).count();

    	//std::cerr<< "time to load "<<msec_load << " time to mark "<< msec_mark<< 
    			//" time to store "<<  msec_store<<std::endl;
		delete [] folder_n;
		delete  img;

	}

	delete mark;

	auto elapsed_total = std::chrono::high_resolution_clock::now() - start_total;
	auto msec_total    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_total).count();

	std::cout << "\n total time " << msec_total << " msecs" << std::endl; 

	return 0;
}