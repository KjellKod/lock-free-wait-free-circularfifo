# UPDATE, please see [Q repository](https://github.com/KjellKod/Q) for MPSC, SPSC etc queues. 



3 examples of queues, not only lock-free circular fifo
----------------------------

1. std::atomic aquire/release + relaxed memory ordering and cache line padded

2. std::atomic aquire/release + relaxed memory ordering

3. mutex protected shared_queue as used in g2log and g3log

Build instructions
-------------------
* Unzip 3rdParty/gtest*.zip to 
 
* On Linux
  mkdir build; cd build
  cmake  cmake -DCMAKE_BUILD_TYPE=Release ..
  make

* On Windows
  mkdir build 
  cd build
  cmake -DCMAKE_BUILD_TYPE=Release -G "Visual Studio <put in your version here>" ..
  msbuild CIRCULARFIFO.sln \p:Configuration=Release


This should build three binaries-unit tests that utilizes the 
lock-free,wait-free CircularFifos (+ the broken one) that are 
found in src


Enjoy
Kjell Hedstrom (hedstrom at kjellkod dot cc)

  
