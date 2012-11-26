

3 examples of circular fifo
----------------------------

1. std::atomic default (sequential consistent)

2. std::atomic aquire/release + relaxed memory ordering

3. Broken. Old legacy code pre C++11
that will not work correctly on most platforms


Build instructions
-------------------
* Unzip 3rdParty/gtest*.zip to 
  3rdParty/gtest-1.6.0__stripped
 
* On Linux
  mkdir build; cd build
  cmake ..

* On Windows
  mkdir build 
  cd build
  cmake -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 11" ..
  msbuild CIRCULARFIFO.sln p:Configuration=Release


This should build three binaries-unit tests that utilizes the lock-free,wait-free CircularFifos (+ the broken one) that are found in src


Enjoy
Kjell Hedstrom (hedstrom at kjellkod dot cc)

  
