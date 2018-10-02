#include "../CImg/CImg.h"

#include <iostream>
#include <optional>
#include <deque>
#include <mutex>


using namespace cimg_library;
#define EOS NULL


template <typename T>

class tri_queue
{
private:
  std::mutex              d_mutex;
  std::mutex              out_mutex;
  std::condition_variable d_condition;
  std::vector < std::deque<T>* >          queues;


  std::vector <std::mutex *  >   mutex_a;
  std::vector <std::condition_variable* > condition_a;

  std::vector <int* > jobs_q;
  std::atomic<int> jobs_in;
  std::atomic<int> jobs_out;

  bool                    qend;

  int priority[3];
  std::mutex              in_mutex;


public:


  tri_queue(std::string s) { std::cout << "Created " << s << " queue " << std::endl;  }

  tri_queue(int queue_number,std::string prior):qend(false),jobs_in(0),jobs_out(0) {


      for (int i = 0; i < queue_number; ++i){
        queues.push_back( new std::deque<T>() );
        mutex_a.push_back( new std::mutex());
        condition_a.push_back( new std::condition_variable());
        jobs_q.push_back(new int(0));
      }
      if(prior=="lms"){
        priority[0] = 0;
        priority[1] = 1;
        priority[2] = 2;

      }else if(prior == "sml"){
        priority[0] = 2;
        priority[1] = 1;
        priority[2] = 0;
      }else if(prior == "msl"){
        priority[0] = 1;
        priority[1] = 2;
        priority[2] = 0;
      }else{
        priority[0] = 1;
        priority[1] = 0;
        priority[2] = 2;
      }

  }

  void delete_sunqueue(){
    for (int i = 0; i < queues.size(); ++i){
        delete queues.at(i);
        delete mutex_a.at(i);
        delete condition_a.at(i);
        delete jobs_q.at(i);
      }
  }

  std::atomic<int>* get_jobs_out(){
    return &jobs_out;
  }
  std::atomic<int>* get_jobs_in(){
    return &jobs_in;
  }

  bool  end(){
    std::unique_lock<std::mutex> lockx(this->in_mutex);

    return ((std::all_of(jobs_q.begin(), jobs_q.end(), [](const int *data){return *data==0;}))&& jobs_out==0);
  }


  void decrease_jobs_out(){
    jobs_out--;
  }

  void notify_all_j(){
    this->d_condition.notify_all();
  }

  void nomorewriters() {
    qend = true;
    this->d_condition.notify_all();
    return;
  }

  void push (T const& value){
    this->push(value ,0);
  }

  void push(T const& value,int queue_number) {

    {
    std::unique_lock<std::mutex> lockx(this->in_mutex);

    queues[queue_number]->push_front(value);
    

    *(jobs_q[queue_number])= *(jobs_q[queue_number])+1;

      if(value->get_type()!=0) // we don't decrease the jobs out if there are load!
        jobs_out--;

    }
    this->d_condition.notify_one();
  }


  T pop() {

    int sieze_q =  queues.size();
    int i;
      {
        std::unique_lock<std::mutex> lock(this->in_mutex);

        this->d_condition.wait(lock, [=]{ return ( std::any_of(jobs_q.begin(), jobs_q.end(), [](const int*data){return *data!=0;}) ||
                                                   ((std::all_of(jobs_q.begin(), jobs_q.end(), [](const int *data){return *data==0;})) && jobs_out==0) ); });

        if((std::all_of(jobs_q.begin(), jobs_q.end(), [](const int *data){return *data==0;}))&& jobs_out==0){
            return EOS;
        }

       // for (i = 0; i <= sieze_q; ++i){
        for(const int &i : priority){

          //decrease the tooked jobs
          if(*jobs_q[i]>0){
            *jobs_q[i]=*jobs_q[i]-1;

            // get the task 
            T rc(std::move(this->queues[i]->back()));
            this->queues[i]->pop_back();

            //store are not jobs out that we have to waits
            if(rc->get_type() !=2)
              jobs_out++;
            
            return rc;
          }
        }


      }
    return (EOS);
  }
};


template <typename T>

class queue
{
private:
  std::mutex              d_mutex;
  std::condition_variable d_condition;
  std::deque<T>           d_queue;
  bool                    qend;

public:

  queue(std::string s) { std::cout << "Created " << s << " queue " << std::endl;  }
  queue():qend(false) {}

  void nomorewriters() {
    qend = true;
    this->d_condition.notify_all();
    return;
  }
  void push(T const& value) {
    {
      std::unique_lock<std::mutex> lock(this->d_mutex);
      d_queue.push_front(value);
    }
    this->d_condition.notify_one();
  }
  
  T pop() {
    std::unique_lock<std::mutex> lock(this->d_mutex);

    this->d_condition.wait(lock, [=]{ return !this->d_queue.empty(); });

    T rc(std::move(this->d_queue.back()));
    if (rc != NULL)
      this->d_queue.pop_back();
    
    return rc;
  }

  std::optional<T> try_pop() {
    std::unique_lock<std::mutex> lock(this->d_mutex);
    if(d_queue.empty() && qend) {
        return {};
    } else {
        this->d_condition.wait(lock, [=]{ return (!this->d_queue.empty()||qend); });
        if(this->d_queue.empty()) {
            return {};
        } else {
            T rc(std::move(this->d_queue.back()));
            this->d_queue.pop_back();
            return rc;
        }
    }
  }

};

class task {

  private:
    //integer to decide from wich row/colum start  and end to mark the image
    int *start_block; 
    int *end_block;
    
    //atomic varible to keep the number of the remaning part of the image to compute
    std::atomic<int> *block_to_do;

    char * img_name;
    CImg<float> *mark;
    CImg<float> *img;
    int task_type;

    std::vector<std::pair<int,int>>* v; //needed to keep the pointer to free at the end

  public:

    task(  char* img_n):  img_name(img_n),task_type(0) {}
    task( int* r, int* c, CImg<float> *i, CImg<float> *m): start_block(r), end_block(c), img(i),  mark(m) {}

    task( CImg<float> *i, CImg<float>* m, char* img_n): img(i), mark(m), img_name(img_n) {}
    task( CImg<float> *i, char* img_n): img(i),  img_name(img_n) {}
    task( CImg<float> *i, CImg<float> *m, char* img_n, int* r, int *c ,std::atomic<int> *to_do,std::vector<std::pair<int,int>>*vect): 
             img(i),  mark(m), img_name(img_n), start_block(r), end_block(c), block_to_do(to_do), v(vect),task_type(1) {}


    int *get_start() {return start_block;}
    int *get_end() {return end_block;}
    CImg<float> *get_img()  {return img; }
    CImg<float> * get_img_p()  {return img; }
    CImg<float> *get_mark()  {return mark; }
    std::vector<std::pair<int,int>>* get_v()  {return v; }
    std::atomic<int>  *get_atom()  {return block_to_do; }
    char * get_name()  {return img_name; }
    void set_type(int x){task_type =x;}
    int get_type(){return task_type;}

    void update( CImg<float> *i, CImg<float> *m, int* r, int *c ,std::atomic<int> *to_do,std::vector<std::pair<int,int>>*vect){
        img=i; 
        mark=m; 
        start_block=r;
        end_block=c; 
        block_to_do=to_do; 
        v=vect; 
    }
    

};



