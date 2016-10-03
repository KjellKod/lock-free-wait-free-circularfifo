# ==================================================================
# 2012 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own
#    risk and comes  with no warranties.
#
# This code is yours to share, use and modify with no strings attached
#   and no restrictions or obligations.
# ===================================================================
#
#
#  WHAT YOU GET
#  Following the instructions below it should build 3 executables
#  It will be unit-test + performance test for each three lock-free
#  queues described at kjellkod.wordpress.com/....***
#  * legacy (not c++ standard and potentially dangerous depending on your platfor)
#  * memory sequential
#  * memory aquire / release
#
# HOW TO BUILD
# Create an empty directory at the same directory level as the
# CMakeLists.txt.
#       cd build
#
# * On Windows
#       cmake -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 11" ..
#       msbuild CIRCULARFIFO.sln /p:Configuration=Release
#
#   On Linux
#   cd build
#   cmake -DCMAKE_BUILD_TYPE=Release ..
#   make
#
#   In your (windows) Release or (Linux) build directory you should
#   now find any of
#   "unit_test_lockfree_{aquire-release}.exe"
# ====================================================================


CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
ENABLE_LANGUAGE(CXX)
SET(CMAKE_BUILD_TYPE Release)



PROJECT (CIRCULARFIFO)


IF(UNIX)
   MESSAGE("-- cmake for *NIX ")
   SET(CMAKE_CXX_FLAGS "-Wall  -Wunused -std=c++11  -Wall -fPIC -Ofast -m64 -D_GLIBCXX_USE_NANOSLEEP")
ENDIF(UNIX)

IF(MSVC)
   MESSAGE("-- cmake for Visual Studio ")
   #VC11 bug: http://code.google.com/p/googletest/issues/detail?id=408
   #add_definition(-D_VARIADIC_MAX=10)
   #https://github.com/anhstudios/swganh/pull/186/files
   ADD_DEFINITIONS (/D_VARIADIC_MAX=10)
   MESSAGE(STATUS "- MSVC: Set variadic max to 10 for MSVC compatibility")
ENDIF(MSVC)


SET(CodeRoot ${CIRCULARFIFO_SOURCE_DIR})
SET(CodeSrcDir ${CodeRoot}/src)
SET(TestSrcDir ${CodeRoot}/test)

INCLUDE_DIRECTORIES(${CodeSrcDir})
INCLUDE_DIRECTORIES(${TestSrcDir})
INCLUDE_DIRECTORIES(${CodeRoot}/g2utilities)


# Google gtest used for unit testing
MESSAGE("Unless already done: Please unzip 3rdParty/gtest-1.7.0)zip to the folder 3rdParty/gtest-1.7.0)")
SET(RootGTest 3rdParty/gtest-1.7.0)
SET(GTestInclude ${RootGTest}/include
                 ${RootGTest}
                 ${RootGTest}/src)
INCLUDE_DIRECTORIES(${GTestInclude})
SET(SrcGTest
   ${RootGTest}/src/gtest-all.cc
   ${RootGTest}/src/gtest_main.cc)
ADD_LiBRARY(gtest_lib ${SrcGTest})
ENABLE_TESTING(true)


# Circular FIFO library
SET (SrcFiles
    ${CodeRoot}/g2utilities/g2random.h
    ${CodeRoot}/g2utilities/g2chrono.h
    ${CodeRoot}/g2utilities/g2chrono.cpp)
ADD_LiBRARY(CircularFifoCompareLibrary  ${SrcFiles})
SET_TARGET_PROPERTIES(CircularFifoCompareLibrary PROPERTIES LINKER_LANGUAGE CXX)
TARGET_LINK_LIBRARIES(CircularFifoCompareLibrary)

SET(CodeLibraries CircularFifoCompareLibrary gtest_lib)


# The actual executable compare *tests*
SET (TestSrcFiles
     ${TestSrcDir}/test_functionality.cpp
     ${TestSrcDir}/test_threaded.cpp
     ${TestSrcDir}/test_main.cpp)


# (default) CircularFifo
ADD_EXECUTABLE(unit_test_lockfree_aquire_release_padded ${TestSrcFiles})
SET_TARGET_PROPERTIES(unit_test_lockfree_aquire_release_padded PROPERTIES COMPILE_DEFINITIONS "GTEST_USE_OWN_TR1_TUPLE=0")
SET_TARGET_PROPERTIES(unit_test_lockfree_aquire_release_padded PROPERTIES COMPILE_DEFINITIONS "MEMORY___RELAXED_AQUIRE_RELEASE_PADDED=1")
TARGET_LINK_LIBRARIES(unit_test_lockfree_aquire_release_padded ${CodeLibraries})



# unit-test for the safe, and atomic refined with std::memory_order_/relaxed/aquire/release
ADD_EXECUTABLE(unit_test_lockfree_aquire_release ${TestSrcFiles})
SET_TARGET_PROPERTIES(unit_test_lockfree_aquire_release PROPERTIES COMPILE_DEFINITIONS "GTEST_USE_OWN_TR1_TUPLE=0")
SET_TARGET_PROPERTIES(unit_test_lockfree_aquire_release PROPERTIES COMPILE_DEFINITIONS "MEMORY___RELAXED_AQUIRE_RELEASE=1")
TARGET_LINK_LIBRARIES(unit_test_lockfree_aquire_release ${CodeLibraries})


# unit-test for the safe, and mutex protected shared_queue
ADD_EXECUTABLE(unit_test_lock_shared ${TestSrcFiles})
SET_TARGET_PROPERTIES(unit_test_lock_shared PROPERTIES COMPILE_DEFINITIONS "GTEST_USE_OWN_TR1_TUPLE=0")
SET_TARGET_PROPERTIES(unit_test_lock_shared PROPERTIES COMPILE_DEFINITIONS "MEMORY___LOCKED=1")
TARGET_LINK_LIBRARIES(unit_test_lock_shared ${CodeLibraries})







