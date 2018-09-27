#include "../CImg/CImg.h"

using namespace cimg_library;

bool split_direction = false  ;     //true = in row , false =  in colum



//fucntion that return an vector of pair containing the beginning and the end of the reagion to computer for each task
std::vector<std::pair<int,int>> *my_split_n(CImg<float> * img, int split_degree) {
  
  int len;

  if(split_direction==true)   len = (*img)._width;  
  else                        len = (*img)._height;  

  int block_size = len/split_degree;
  int block_rest = len%split_degree;


  std::vector<std::pair<int,int>> *v = new std::vector<std::pair<int,int>> ();

  int start_block=0;
  int end_block=block_size;

  split_degree--;

  for(int i=0; i<split_degree;i++){
    v->push_back(std::make_pair( start_block, end_block ));
    start_block+=block_size;
    end_block+=block_size;
  }
  end_block+=block_rest;

  //put on the last task the remaing row
  v->push_back(std::make_pair(start_block,end_block ));

  return v;

}




void fuse_task_block(task * t){

  CImg<float> * img = t->get_img();

  CImg<float>* mark = t->get_mark();

  int *sb = t->get_start();
  int *eb = t->get_end() ;
  int np;
  int s_i,e_i,s_j,e_j;

  if(split_direction==true){  // change the direction of splitting
    s_i=0;
    e_i=(*img)._height;
    s_j=*sb;
    e_j=*eb;
  }else{
    s_i= *sb;
    e_i= *eb;
    s_j= 0;
    e_j= (*img)._width;  
  }

  for(int i=s_i ; i< e_i ; i++){

    for(int j=s_j; j<e_j ; j++){

      if((*mark)(i,j) != 255){

        np= round((  (*img)(i,j,0,0)+(*img)(i,j,0,1)+(*img)(i,j,0,2)+85)  /6);

        (*img)(i,j,0,0) = np; 
        (*img)(i,j,0,1) = np;  
        (*img)(i,j,0,2) = np;  
      }
    }
  }
}



//function for the serial version
void fuse_img(CImg<float> * img ,CImg<float>* mark){

  for(int i=0; i<(*img)._height ; i++){

    for(int j=0; j<(*img)._width ; j++){

      if((*mark)(i,j) != 255){
        (*img)(i,j,0,0) = 0;  
        (*img)(i,j,0,1) = 0;  
        (*img)(i,j,0,2) = 0;  
      }
    }
  }
}



