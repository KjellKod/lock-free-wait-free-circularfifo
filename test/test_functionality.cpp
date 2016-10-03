
#include <gtest/gtest.h>
#include <string>
#include <atomic>



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



#if !defined(MEMORY___LOCKED)

  
namespace
{
  const size_t zero = 0;

}

namespace test_of_circular_fifo
{
  size_t calculateIncrement(size_t index, int Capacity)
  {
    return  ((index + 1) % Capacity);
  }

  void doIncrement(std::atomic<size_t>& increment_index, int Capacity)
  {
    auto next_index = calculateIncrement(increment_index.load(), Capacity);
    increment_index.store(next_index);
  }
}

TEST(CircularFifo, OneItemQueueAddRemoveLimits)
{
        bool value_first = true;
        bool value_second = false;

        CircularFifo<bool,1> fifo;
        ASSERT_TRUE(fifo.wasEmpty());
        ASSERT_TRUE(fifo.push(value_first));   // make queue full
        ASSERT_TRUE(fifo.wasFull());
        ASSERT_FALSE(fifo.wasEmpty());
        ASSERT_FALSE(fifo.push(value_second)); // when full, cannot pop

        bool value_to_get = !value_first;     // emptying and verifying empty
        ASSERT_TRUE(fifo.pop(value_to_get));
        ASSERT_FALSE(fifo.wasFull());
        ASSERT_TRUE(value_to_get == value_first); // verify retrieved value
}


TEST(CircularFifo, CannotRemoveFromEmpty)
{
        CircularFifo<double, 10> fifo;
        ASSERT_FALSE(fifo.wasFull());
        ASSERT_TRUE(fifo.wasEmpty());
        double value_to_get;
        ASSERT_FALSE(fifo.pop(value_to_get));
}

// Actually completely redundant with test below
TEST(Namespace, funWithModulus)
{
 // Modulus operator can be confusing if not encountered before
 // Commonly it is used to get the remainder from integer division  (5%2 --> 1 : i.e. 2*2 is 4, leaves 1)
 // or decide whether it is odd or even number ( 4%2 --> 0 : even, 5%2 -->1 : odd)
 // Here we want to verify that it can be used to get "next increment" and later wrap
  //size_t calculateIncrement(const std::atomic<size_t>& unchanged_index, int Capacity)
 using namespace test_of_circular_fifo;
 size_t size = 1; // capacity 1: but size is +1 is used as padding to simplify logic
 size_t capacity = size +1;

 ASSERT_EQ(calculateIncrement(zero, capacity), (zero +1)); // 1st increment
 ASSERT_EQ(calculateIncrement(size, capacity), zero); // wrap
}

// verify calculateIncrement that goes to end of circular fifo index and then wrap around to first index
// somewhat redundant with the test 'funWithModulus' above
TEST(Namespace, calculateIncrementAtomicLoadWithWrapAround)
{
  auto Capacity = 3; // implies Size=2

  std::atomic<size_t> atomic_index(zero);
  size_t expected_index = zero;

  ASSERT_EQ(atomic_index.load(), zero);
  ASSERT_EQ(++expected_index, test_of_circular_fifo::calculateIncrement(atomic_index.load(), Capacity)); // 1 of 2
  ++atomic_index;
  ASSERT_EQ(atomic_index.load(), (zero+1));

  ASSERT_EQ(++expected_index, test_of_circular_fifo::calculateIncrement(atomic_index.load(), Capacity));  // 2 of 2
  ++atomic_index;
  ASSERT_EQ(atomic_index.load(), (zero+2));

  size_t wrapIndex =  test_of_circular_fifo::calculateIncrement(atomic_index.load(), Capacity);
  ASSERT_NE(++expected_index, wrapIndex);
  ASSERT_EQ(zero, wrapIndex); // fifo has wrapped around
}


TEST(Namespace, doIncrementOfAtomicWithWrapAround)
{
  auto Capacity = 2;

  size_t expected_index=zero;
  std::atomic<size_t> atomic_index(zero);

  ASSERT_EQ(atomic_index.load(), zero);
  test_of_circular_fifo::doIncrement(atomic_index, Capacity);
  ASSERT_EQ(++expected_index, atomic_index.load());
  ASSERT_EQ(atomic_index.load(), (zero+1));

  // verify wrap-around
  expected_index = zero;
  test_of_circular_fifo::doIncrement(atomic_index, Capacity);
  ASSERT_EQ(expected_index, atomic_index.load());
}


TEST(CircularFifo, pushUntilFull)
{
   CircularFifo<std::string, 1> circular_fifo;
   std::string one("one");
   std::string two("two");
   ASSERT_TRUE(circular_fifo.push(one));
   ASSERT_FALSE(circular_fifo.push(two));
}

TEST(CircularFifo, popUntilEmpty)
{
  CircularFifo<std::size_t, 2> circular_fifo;
  std::size_t increment = 2;
  const std::size_t start = 100;
  for(std::size_t value = start; value < start+increment; ++value)
  {
    ASSERT_EQ(circular_fifo.push(value), true);
  }

  ASSERT_FALSE(circular_fifo.push(increment)); // already full
  std::size_t read_value = zero;
  for(std::size_t value = start; value < start+increment; ++value)
  {
    ASSERT_TRUE(circular_fifo.pop(read_value));
    ASSERT_EQ(read_value, value);
  }
  ASSERT_FALSE(circular_fifo.pop(read_value)); // already empty
}





TEST(CircularFifo, zeroSizedIsNotWorking)
{
   CircularFifo<std::string, zero> circular_fifo;
   std::string one("one");
   std::string two("two");
   ASSERT_FALSE(circular_fifo.push(one));
   ASSERT_TRUE(circular_fifo.wasFull());
   ASSERT_TRUE(circular_fifo.wasEmpty());
}


#if defined(FIFO_SAFE)
// this test might fail on some platforms - in which case the atomic<size_t>
// is still atomic but not lock-free in the internal implementation
TEST(CircularFifo, IsLockFree)
{
  CircularFifo<bool, 1> fifo;
  ASSERT_TRUE(fifo.isLockFree());

  // advanced type, should not matter
  CircularFifo<std::string, 2000> str_fifo;
  ASSERT_TRUE(str_fifo.isLockFree());
}
#endif
#endif
