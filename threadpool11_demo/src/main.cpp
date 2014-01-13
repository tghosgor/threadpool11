/*
Copyright (c) 2013, Tolga HOŞGÖR
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
*/

#include <cstdio>

#include <algorithm>
#include <array>
#include <iostream>
#include <mutex>
#include <thread>

#include "threadpool11/threadpool11.h"

std::mutex coutMutex;

void testFunc(int i) {
  coutMutex.lock();
  std::cout << "\tDoing " << i << " by thread id " << std::this_thread::get_id() << std::endl;
  coutMutex.unlock();
}

void testFunc2()
{
  coutMutex.lock();
  //heavy job that takes 1 second
  std::cout << "Waiting thread id: " << std::this_thread::get_id() << std::endl;
  coutMutex.unlock();
  std::this_thread::sleep_for(std::chrono::seconds(1));
}

void testFunc3()
{
  volatile int i = std::min(5, rand());
}

int main()
{
  threadpool11::Pool pool(5);

  std::cout << "Your machine's hardware concurrency is " << std::thread::hardware_concurrency() << std::endl << std::endl;

  /**
  * Demo #1.
  */
  {
    std::cout << "This demo is about showing how parallelization increases the performance. "
      << "In this test case, work is simulated by making the threads inactive by sleeping for 1 second. "
      << "It will seem like all jobs complete in ~1 second even in a single core machine. "
      << "However, in real life cases, work would keep CPU busy so you would not get any real benefit using "
      << "thread numbers that are higher than your machine's hardware concurrency (threads that are executed "
      << "concurrently) except in some cases like doing file IO." << std::endl << std::endl;
    /*{
      std::cout << "Demo 1\n";
      std::cout << "Executing 5 testFunc2() WITHOUT posting to thread pool:\n";
      auto begin = std::chrono::high_resolution_clock::now();
      testFunc2();
      testFunc2();
      testFunc2();
      testFunc2();
      testFunc2();
      auto end = std::chrono::high_resolution_clock::now();
      std::cout << "\texecution took "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " milliseconds.\n\n";
    }*/

    pool.increaseWorkerCountBy(5);
    {
      std::cout << "Executing 5 testFunc2() WITH posting to thread pool:" << std::endl;
      std::vector<std::future<void>> futures;
      auto begin = std::chrono::high_resolution_clock::now();
      futures.emplace_back(pool.postWork<void>(testFunc2));
      futures.emplace_back(pool.postWork<void>(testFunc2));
      futures.emplace_back(pool.postWork<void>(testFunc2));
      futures.emplace_back(pool.postWork<void>(testFunc2));
      futures.emplace_back(pool.postWork<void>(testFunc2));
      for(auto& it : futures)
        it.get();
      auto end = std::chrono::high_resolution_clock::now();
      std::cout << "\tDemo 1 took "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " milliseconds."
		<< std::endl << std::endl;
    }
  }

  /**
  * Demo #2
  */
  {
    std::cout << "Demo 2" << std::endl;
    pool.decreaseWorkerCountBy(pool.getInactiveWorkerCount() - std::thread::hardware_concurrency());
    std::cout << "Posting 1.000.000 jobs." << std::endl;

    std::vector<std::future<void>> futures;
    auto begin = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 1000000; ++i)
    {
      futures.emplace_back(pool.postWork<void>(testFunc3));
    }
    auto end = std::chrono::high_resolution_clock::now();
    for(auto& it : futures)
      it.get();
    auto end2 = std::chrono::high_resolution_clock::now();

    std::cout << "Demo 2 took " << std::chrono::duration_cast<std::chrono::milliseconds>(end2 - begin).count()
              << " milliseconds. (Posting: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
              << " ms, getting: " << std::chrono::duration_cast<std::chrono::milliseconds>(end2 - end).count() << " ms)"
              << std::endl << std::endl;
  }

  std::cout << "Current worker count is " << pool.getActiveWorkerCount() + pool.getInactiveWorkerCount()
            << " (Active: " << pool.getActiveWorkerCount() << ", Inactive: " << pool.getInactiveWorkerCount()
            << ") . Setting worker count to 5 again... " << std::flush;
  pool.increaseWorkerCountBy(std::min<threadpool11::Pool::WorkerCountType>(5, 5 - pool.getInactiveWorkerCount()));
  std::cout << "The new worker count is " << pool.getInactiveWorkerCount() << "." << std::endl << std::endl;

  /**
  * Demo #3
  * You should always capture by value or use appropriate mutexes for reference access.
  */
  {
    std::cout << "Demo 3" << std::endl;
    std::cout << "Testing lambda copy/modify:" << std::endl;
    for (int i=0; i<20; i++) {
      pool.postWork<void>([=]() { testFunc(i); });
    }
    std::cout << "Done." << std::endl << std::endl;
  }

  /**
  * Demo #4
  * Using futures.
  */
  {
    std::cout << "Demo 4\n";
    std::cout << "WARNING: This test's output may be distorted because no synchronization on std::cout is done.\n";
    std::array<std::future<float>, 20> futures;

    auto begin = std::chrono::high_resolution_clock::now();
    for (int i=0; i<20; i++) {
      futures[i] = pool.postWork<float>([=]() {
        std::cout << "\tExecuted pow(" << i << ", 2) by thread id "
                  << std::this_thread::get_id() << std::endl;
        return pow(i, 2);
      });
    }

    for (int i=0; i < 20; i++) {
      std::cout << "\tfuture[" << i << "] value: " << futures[i].get() << std::endl;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Demo 4 took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
              << " milliseconds." << std::endl << std::endl;
  }

  /**
  * Test case for Issue #1 (fixed): Pool::postWork waiting forever, due to posting work before all threads in pool
  * are properly initialized and waiting.
  */
  /*{
    while(true)
    {
      std::cout << "Loop begin" << std::endl;

      std::vector<std::future<void>> futures;
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " heyyo1" << std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " heyyo2" << std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " heyyo3" << std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " heyyo4" << std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " heyyo5" << std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " heyyo6" << std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " heyyo7" << std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " heyyo8" << std::endl;}));

      //pool[0].setWork([]{});
      std::cout << "waiting 1" << std::endl;
      for(auto& it : futures)
        it.get();
      std::cout << "wait 1 end" << std::endl;
      futures.clear();

      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " 2heyyo1" << std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " 2heyyo2" << std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " 2heyyo3" << std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " 2heyyo4" << std::endl;}));

      std::cout << "waiting 2" << std::endl;
      for(auto& it : futures)
        it.get();
      std::cout << "wait 2 end" << std::endl;
      futures.clear();

      std::cout << "Active thread count: " << pool.getActiveWorkerCount() << std::endl;
      std::cout << "Inactive thread count: " << pool.getInactiveWorkerCount() << std::endl;

      std::cout << "Increasing thread count by 4." << std::endl;
      pool.increaseWorkerCountBy(4);

      std::cout << "Active thread count: " << pool.getActiveWorkerCount() << std::endl;
      std::cout << "Inactive thread count: " << pool.getInactiveWorkerCount() << std::endl;

      std::cout << "Decreasing thread count by 4." << std::endl;
      pool.decreaseWorkerCountBy(4);

      std::cout << "Active thread count: " << pool.getActiveWorkerCount() << std::endl;
      std::cout << "Inactive thread count: " << pool.getInactiveWorkerCount() << std::endl;

      std::cout << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl;

      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  }*/

  std::cout << std::endl << std::endl;

  pool.joinAll();

  getchar();
  return 0;
};
