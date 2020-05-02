/*
* Not any company's property but Public-Domain
* Do with source-code as you will. No requirement to keep this
* header if need to use it/change it/ or do whatever with it
*
* Note that there is No guarantee that this code will work
* and I take no responsibility for this code and any problems you
* might get if using it.
*
* Code & platform dependent issues with it was originally
* published at http://www.kjellkod.cc/threadsafecircularqueue
* 2012-16-19  @author Kjell Hedström, hedstrom@kjellkod.cc */

// should be mentioned the thinking of what goes where
// it is a "controversy" whether what is tail and what is head
// https://en.wikipedia.org/wiki/FIFO_(computing_and_electronics)#Head_or_tail_first

#pragma once

#include <atomic>
#include <cstddef>
#include <thread>

namespace memory_relaxed_aquire_release_padded {


   template<typename Element, size_t Size>
   class CircularFifo {
    public:
      typedef char cache_line[64];
      enum alignas(64) { Capacity = Size + 1 };       // http://en.cppreference.com/w/cpp/types/aligned_storage


      CircularFifo() : _tail(0), _head(0) {}
      virtual ~CircularFifo() {}

      bool push(const Element& item); // pushByMOve?
      bool pop(Element& item);

      bool wasEmpty() const;
      bool wasFull() const;
      bool isLockFree() const;

      unsigned size() const { return 1; }


    private:
      size_t increment(size_t idx) const { return (idx + 1) % Capacity; }

      cache_line _pad_storage;
      /*alignas(64)*/ Element _array[Capacity];
      cache_line _pad_tail;
      /*alignas(64)*/ std::atomic <size_t>  _tail;  
      cache_line  _pad_head;
      /*alignas(64)*/ std::atomic<size_t>   _head; // head(output) index
   };


   template<typename Element, size_t Size>
   bool CircularFifo<Element, Size>::push(const Element& item) {
      const auto current_tail = _tail.load(std::memory_order_relaxed);
      const auto next_tail = increment(current_tail);
      if (next_tail != _head.load(std::memory_order_acquire)) {
         _array[current_tail] = item;
         _tail.store(next_tail, std::memory_order_release);
         return true;
      }

      return false; // full queue

   }


// Pop by Consumer can only update the head (load with relaxed, store with release)
//     the tail must be accessed with at least aquire
   template<typename Element, size_t Size>
   bool CircularFifo<Element, Size>::pop(Element& item) {
      const auto current_head = _head.load(std::memory_order_relaxed);
      if (current_head == _tail.load(std::memory_order_acquire))
         return false; // empty queue

      item = _array[current_head];
      _head.store(increment(current_head), std::memory_order_release);
      return true;
   }

   template<typename Element, size_t Size>
   bool CircularFifo<Element, Size>::wasEmpty() const {
      // snapshot with acceptance of that this comparison operation is not atomic
      return (_head.load(std::memory_order_relaxed) == _tail.load(std::memory_order_relaxed));
   }


// snapshot with acceptance that this comparison is not atomic
   template<typename Element, size_t Size>
   bool CircularFifo<Element, Size>::wasFull() const {
      const auto next_tail = increment(_tail.load(std::memory_order_relaxed)); // aquire, we dont know who call
      return (next_tail == _head.load(std::memory_order_relaxed));
   }


   template<typename Element, size_t Size>
   bool CircularFifo<Element, Size>::isLockFree() const {
      return (_tail.is_lock_free() && _head.is_lock_free());
   }

} // memory_relaxed_aquire_release
