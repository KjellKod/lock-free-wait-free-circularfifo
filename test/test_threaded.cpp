#include <gtest/gtest.h>
#include <thread>
#include <future>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <exception>
#include <memory>
#include <mutex>

#include "g2random.h"
#include "g2chrono.h"

#if defined (MEMORY___RELAXED_AQUIRE_RELEASE_PADDED)
   #include "circularfifo_memory_relaxed_aquire_release_padded.hpp"
   using namespace memory_relaxed_aquire_release_padded;
#elif defined (MEMORY___RELAXED_AQUIRE_RELEASE)
   #include "circularfifo_memory_relaxed_aquire_release.hpp"
   using namespace memory_relaxed_aquire_release;
#elif defined (MEMORY___LOCKED)
#include "shared_queue.hpp"
#else
#error AQUIRE_RELEASE or AQUIRE_RELEASE_PADDED or MEMORY___LOCKED should be defined
#endif


void helloFunc() {std::cout << "hello from thread " << std::endl; }

namespace {
   std::vector<std::string> couts;

   std::mutex gPrintLock;
   void couts_push(std::string str) {
      std::lock_guard<std::mutex> guard(gPrintLock);
      couts.push_back(str);
   }

   void couts_flush() {
      for (const auto& s : couts) {
         std::cout << s << std::endl;
      }
      couts.clear();
   }


   const std::size_t  k_queue_size = 1048576; //2^20
   const std::size_t  k_amount_to_process = 1 * 1000 * 1000; // 1 million
   const g2::Number k_lower_limit = 0;
   const g2::Number k_upper_limit = 1048576;

   typedef g2::Number Message;
#if defined (MEMORY___LOCKED)
   typedef shared_queue<Message> MessageQueue;
#else
   typedef CircularFifo<Message, k_queue_size> MessageQueue;
#endif

} // anonymous











// Helper for filling a container with random values. Values ranging between low, to high
template<typename Container>
void replaceContentWithRandoms(Container& to_randomize, g2::Number low, g2::Number high) {
   if (low >= high)
      throw  std::runtime_error("lower input value not lower than higher value");

   using namespace g2;
   auto generator = g2::RandomGenerator(low, high);

   std::for_each(to_randomize.begin(), to_randomize.end(),
   [&](Number & n) { n = generator();});
}


// just to verify that threads c++11 concurrency setup is OK
TEST(Cpp11, stdThreadIsOK) {
   SCOPED_TRACE("Thread test");  // Scope exit be prepared for destructor failure
   std::thread t1(helloFunc);    // byt ut denna mot en future och std::asynch
   t1.join();
   ASSERT_TRUE(std::thread::hardware_concurrency() > 0);
   SCOPED_TRACE("std::thread::hardware_concurrency");
   std::stringstream ss;
   ss << "std::thread::hardware_concurrency() = " << std::thread::hardware_concurrency();  // Scope exit be prepared for destructor failure
   std::cout << ss.str() << std::endl;
}







// The producer functionality
std::vector<Message> Producer(MessageQueue& out_fifo, const size_t enough_processed) {
   std::vector<Message> all_data;
   all_data.resize(enough_processed);
   replaceContentWithRandoms(all_data, k_lower_limit, k_upper_limit);

   std::vector<Message> pushed_data; // for later verification
   pushed_data.reserve(k_queue_size);
   g2::StopWatch clock;
   bool hit_limit = false;
   for (Message m : all_data) {
      while (false == out_fifo.push(m)) {
         std::this_thread::yield();
         hit_limit = true;
      }
      pushed_data.push_back(m);
   }
   float elapsed_us = clock.elapsedUs().count();
   std::ostringstream outs;
   outs << "\nProducer: #" << k_amount_to_process << " items took #" << elapsed_us / 1000 << " milliseconds to shuffle, average: " << elapsed_us / k_amount_to_process << " us, hit limit?: " << hit_limit << std::endl;
   couts_push(outs.str());
   return pushed_data;
}


// The consumer functionality (background)
std::vector<Message> Consumer(MessageQueue& in_fifo, const size_t enough_processed) {
   std::vector<Message> received_data; // for later verification with produced/sent items
   received_data.reserve(k_queue_size);
   g2::StopWatch clock;
   bool hit_limit = false;
   for (size_t count = 0; enough_processed > count; ++count) {
      Message m;
      while (false == in_fifo.pop(m)) {
         std::this_thread::yield();
         hit_limit = true;
      }
      received_data.push_back(m);
   }

   float elapsed_us = clock.elapsedUs().count();
   std::ostringstream outs;

   outs << "\nConsumer: #" << k_amount_to_process << " items took #" << elapsed_us / 1000 << " milliseconds to shuffle, average: " << elapsed_us / k_amount_to_process << ", hit limit?: " << hit_limit << std::endl;
   couts_push(outs.str());

   return received_data;
}



TEST(ThreadedTest, SingleProducerSingleConsumer) {
#if !defined(MEMORY___LOCKED)
   std::cout << "\nPrepare to wait a bit, pushing " << k_amount_to_process << " through a circular fifo of size: " << MessageQueue::Capacity - 1 << std::endl << std::flush;
#else
   std::cout << "\nPrepare to wait a bit, pushing " << k_amount_to_process << " through a locked dynamic fifo " << std::endl << std::flush;
#endif

   MessageQueue msg_queue;
   g2::StopWatch clock;
   auto future_consumed = std::async(std::launch::async, &Consumer, std::ref(msg_queue), k_amount_to_process); 
   auto producer_made = Producer(std::ref(msg_queue), k_amount_to_process); // this thread scope
   auto consumer_received = future_consumed.get(); // wait for consumer to finish
   float elapsed_ms = clock.elapsedMs().count();
   ASSERT_TRUE(consumer_received.size() == producer_made.size());
   ASSERT_TRUE(consumer_received.size() == k_amount_to_process);
   std::cout << "#" << k_amount_to_process << " items took #" << elapsed_ms << " milliseconds to shuffle";
   std::cout  << std::flush;
   couts_flush();

   ASSERT_TRUE(std::equal(producer_made.begin(), producer_made.end(), consumer_received.begin()));
   auto getitr = consumer_received.begin();
   auto putitr = producer_made.begin();
   auto idx = 0;
   while (getitr != consumer_received.end()) {
      if ((*getitr) != (*putitr)) {
         std::cout << "\nC:" << *getitr << "\tP:" << *putitr << "\tidx:" << idx;
      }
      ++idx;
      ++getitr;
      ++putitr;
   }
   ASSERT_TRUE(std::equal(producer_made.begin(), producer_made.end(), consumer_received.begin()));

}

