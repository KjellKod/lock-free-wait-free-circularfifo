#ifndef G2_CHRONO_H_ 
#define G2_CHRONO_H_

// http://kjellkod.wordpress.com/2012/02/06/exploring-c11-part-1-time/#more-458
#include <iostream>
#include <chrono>
#include <thread>


namespace g2
{       
  using clock = std::chrono::high_resolution_clock;
  using microseconds = std::chrono::duration<float, std::chrono::microseconds::period>;
  using milliseconds = std::chrono::duration<float, std::chrono::milliseconds::period>;

  clock::time_point now(){return clock::now();}

  microseconds intervalUs(const clock::time_point& t1, const clock::time_point& t0)
  {return microseconds(t1 - t0);}

  milliseconds intervalMs(const clock::time_point& t1,const clock::time_point& t0)
  {return milliseconds(t1 - t0);}


  template<typename Duration>
  void short_sleep(Duration d) // thanks to Anthony Williams for the suggestion of short_sleep
  {                            
    clock::time_point const start=clock::now(), stop=start+d;
    do{
      std::this_thread::yield();
    } while(clock::now()<stop);
  }

  class StopWatch
  {
    clock::time_point start_;
  public:
    StopWatch() : start_(clock::now()){}
    clock::time_point restart()         { start_ = clock::now(); return start_;}
    microseconds elapsedUs()            { return intervalUs(now(), start_);}
    milliseconds elapsedMs()            {return intervalMs(now(), start_);}
  };
} // g2



#endif // G2_CHRONO_H_
