#include <gtest/gtest.h>
#include <thread>
#include <future>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <exception>
#include <memory>
#include "g2random.h"
#include "g2chrono.h"

#if defined (MEMORY___SEQUENTIAL_CONSISTENT)
#include "circularfifo_memory_sequential_consistent.hpp"
using namespace memory_sequential_consistent;
static const std::string queue_type_message = "CircularFifo_SEQUENTIAL (default sequential) ";


#elif defined (MEMORY___RELAXED_AQUIRE_RELEASE)
#include "circularfifo_memory_relaxed_aquire_release.hpp"
using namespace memory_relaxed_aquire_release;
static const std::string queue_type_message = "CircularFifo_AQUIRE_RELEASE (relaxed/aquire/released) ";


#elif defined (HAZARD)
#include "circularfifo_hazard_platform_dependent.hpp"
using namespace hazard_by_convention;
static const std::string queue_type_message = "CircularFifo_HAZARD (Old School, NOT RECOMMENDED, Highly Platform dependent) ";
#else
#error SEQUENTIAL or AQUIRE_RELEASE or HAZARD should be defined
#endif


void helloFunc() {std::cout << "hello from thread " << std::endl; }

namespace {
  const std::size_t  k_queue_size = 1000; //128;
  const std::size_t  k_amount_to_process = 10*1000*1000; // 10 million
  const g2::Number k_lower_limit = 0;
  const g2::Number k_upper_limit = 10000;

  typedef g2::Number Message;
  typedef CircularFifo<Message, k_queue_size> MessageQueue;

} // anonymous



// Used for debugging and verification,  normally disabled
template<typename Container>
void printValues(const std::string& msg, const Container& values)
{
  using namespace g2;

  std::cout << msg << std::endl;
  std::for_each(values.begin(), values.end(),
                [&](const Number& n) { std::cout << " " << n << " "; });

std::cout << "\n" << std::endl;
}

// Helper for filling a container with random values. Values ranging between low, to high
template<typename Container>
void replaceContentWithRandoms(Container& to_randomize, g2::Number low, g2::Number high)
{
  if(low >= high)
    throw  std::runtime_error("lower input value not lower than higher value");

  using namespace g2;
  auto generator = g2::RandomGenerator(low, high);

  std::for_each(to_randomize.begin(), to_randomize.end(),
                [&](Number& n) { n = generator();});
}


// just to verify that threads c++11 concurrency setup is OK
TEST(Cpp11, stdThreadIsOK)
{
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
std::vector<Message> Producer(MessageQueue& out_fifo, const size_t enough_processed)
{
  std::vector<Message> all_data;
  all_data.resize(enough_processed);
  replaceContentWithRandoms(all_data, k_lower_limit, k_upper_limit);

  std::vector<Message> pushed_data; // for later verification
  for(Message m : all_data)
  {
    while(false == out_fifo.push(m))
    {
      std::this_thread::yield();
    }
    pushed_data.push_back(m);
  }
  return pushed_data;
}


// The consumer functionality (background)
std::vector<Message> Consumer(MessageQueue& in_fifo, const size_t enough_processed)
{
  std::vector<Message> received_data; // for later verification with produced/sent items
  for(size_t count = 0; enough_processed > count; ++count)
  {
    Message m;
    while(false == in_fifo.pop(m))
    {
      std::this_thread::yield();
    }
    received_data.push_back(m);
  }
  return received_data;
}



TEST(ThreadedTest, SingleProducerSingleConsumer)
{
  std::cout << queue_type_message << "\nPrepare to wait a bit, pushing " << k_amount_to_process << " through a circular fifo of size: " << MessageQueue::Capacity-1 << std::endl << std::flush;
  MessageQueue msg_queue;

  g2::StopWatch clock;
  auto future_consumed =
      std::async(std::launch::async, &Consumer, std::ref(msg_queue), k_amount_to_process); // launch::async force actual thread to run

  auto producer_made = Producer(std::ref(msg_queue), k_amount_to_process); // this thread scope
  auto consumer_received = future_consumed.get(); // wait for consumer to finish
  auto elapsed_ms = clock.elapsedMs();


  std::cout << "\n\t#" << k_amount_to_process << " items took #" << elapsed_ms.count() << " milliseconds to shuffle";
  std::cout << std::endl << std::flush;

  ASSERT_TRUE(consumer_received.size() == producer_made.size());
  ASSERT_TRUE(consumer_received.size() == k_amount_to_process);
  //ASSERT_TRUE(std::equal(producer_made.begin(), producer_made.end(), consumer_received.begin()));
  auto getitr = consumer_received.begin();
  auto putitr = producer_made.begin();
  auto idx = 0;
  while(getitr != consumer_received.end())
  {
    if((*getitr) != (*putitr))
    {
      std::cout << "\nC:" << *getitr << "\tP:" << *putitr << "\tidx:" << idx;
    }
    ++idx;
    ++getitr;
    ++putitr;
  }
  ASSERT_TRUE(std::equal(producer_made.begin(), producer_made.end(), consumer_received.begin()));

}

